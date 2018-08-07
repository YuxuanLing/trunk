#include "dec_getvlc.c"
#include "fractaltest.h"
#include "fractaltest_utils.h"

#include <stdio.h>
#include <string.h>

#ifdef WIN32
# define NULL_FILENAME "nul"
#else
# define NULL_FILENAME "/dev/null"
#endif


static FILE * nullfile;
FILE * tracefile;

static char desc[64];
static uint8_t big_buf[128];
#define TRAILING_BITS "1"

static void init_reader (
  bitreader_t * r,
  const char *  bitstring_in)
{

  // Always put a trailing bit at the end of the string to avoid the
  // preprocessing process to remove 0 bytes from our test
  int n = strlen (bitstring_in);
  char bitstring[n + strlen (TRAILING_BITS) + 1];
  strcpy (bitstring, bitstring_in);
  strcat (bitstring, TRAILING_BITS);
  n = strlen (bitstring);

  /* Transform the string to bits */
  for (int i = 0; i < n; i++)
  {
    unsigned bit = (bitstring[i] == '1');
    unsigned bitpos = i % 8;
    unsigned bytepos = i / 8;
    bit <<= 7 - bitpos;
    if (bitpos == 0)
      big_buf[bytepos] = (uint8_t) bit;
    else
      big_buf[bytepos] = (uint8_t)(big_buf[bytepos] | bit);
  }

  taa_h264_load_buffer (r, big_buf, (n + 7) / 8);
}


static unsigned get_num_bits_read (
  bitreader_t * r)
{
  unsigned num_bytes = r->bufptr - big_buf - 1;
  unsigned num_bits = (32 - r->bitrest) % 8;
  return num_bytes * 8 + num_bits;
}


typedef struct
{
  char * bitstring;
  int expect_output;
} test_data_exp_golomb;

static void test_exp_golomb_unsigned (void)
{
  bitreader_t reader;
  test_data_exp_golomb test_cases[] = {
    { "1", 0},
    { "010", 1},
    { "011", 2},
    { "00100", 3},
    { "000011111", 30},
    { "00000100000", 31},
    { "0000001000000", 63}
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    init_reader (&reader, test_cases[i].bitstring);
    int result = taa_h264_get_uvlc (&reader);

    snprintf (desc, sizeof(desc), "bits %s expects %d, got %d",
              test_cases[i].bitstring, test_cases[i].expect_output, result);
    FASSERT2 (test_cases[i].expect_output == result, desc);
    unsigned num_bits = get_num_bits_read (&reader);
    FASSERT (num_bits == strlen (test_cases[i].bitstring));
  }
}


static void test_exp_golomb_signed (void)
{
  bitreader_t reader;
  test_data_exp_golomb test_cases[] = {
    { "1", 0},
    { "010", 1},
    { "011", -1},
    { "00100", 2},
    { "00101", -2},
    { "000011111", -15},
    { "00000100000", 16},
    { "0000001000000", 32}
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    init_reader (&reader, test_cases[i].bitstring);
    int result = taa_h264_get_level (&reader);

    snprintf (desc, sizeof(desc), "bits %s expects %d, got %d",
              test_cases[i].bitstring, test_cases[i].expect_output, result);
    FASSERT2 (test_cases[i].expect_output == result, desc);
    unsigned num_bits = get_num_bits_read (&reader);
    FASSERT (num_bits == strlen (test_cases[i].bitstring));
  }
}

typedef struct
{
  char *  bitstring;
  int vlc_num;
  int expect_total_coeffs;
  int expect_trailing_ones;
  int expect_bits_advanced;
} test_data_coeff_token;


static void test_cavlc_get_coeff_token (void)
{
  bitreader_t reader;

  test_data_coeff_token test_cases[] = {
    /* vlc_num == 0 */
    { "1", 0, 0, 0, 1},
    { "000001100", 0, 3, 1, 8},
    { "000001101", 0, 3, 1, 8},
    { "00000000001010", 0, 10, 1, 14},
    { "000000000000001", 0, 13, 1, 15},
    { "0000000000000111", 0, 15, 0, 16},
    { "0000000000001000", 0, 16, 3, 16},
    /* vlc_num == 1 */
    { "11", 1, 0, 0, 2},
    { "0000001110", 1, 6, 0, 9},
    { "0000001111", 1, 6, 0, 9},
    { "00000000001001", 1, 15, 0, 14},
    { "00000000001000", 1, 15, 1, 14},
    /* vlc_num == 2 */
    { "1111", 2, 0, 0, 4},
    { "1001", 2, 6, 3, 4},
    { "000001111", 2, 10, 0, 9},
    /* vlc_num == 3 */
    { "000011", 3, 0, 0, 6},
    { "000000", 3, 1, 0, 6},
    { "101110", 3, 12, 2, 6}
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    int total_coeffs;
    int trailing_ones;
    init_reader (&reader, test_cases[i].bitstring);
    taa_h264_get_coeff_token (&reader, test_cases[i].vlc_num, &total_coeffs, &trailing_ones);

    snprintf (desc, sizeof(desc), "bits %s expects total_coeffs %d, got %d",
              test_cases[i].bitstring, test_cases[i].expect_total_coeffs, total_coeffs);
    FASSERT2 (test_cases[i].expect_total_coeffs == total_coeffs, desc);

    snprintf (desc, sizeof(desc), "bits %s expects trailing_ones %d, got %d",
              test_cases[i].bitstring, test_cases[i].expect_trailing_ones, trailing_ones);
    FASSERT2 (test_cases[i].expect_trailing_ones == trailing_ones, desc);
    unsigned num_bits = get_num_bits_read (&reader);
    FASSERT (num_bits == test_cases[i].expect_bits_advanced);
  }
}


static void test_cavlc_get_coeff_token_chroma_dc (void)
{
  bitreader_t reader;

  test_data_coeff_token test_cases[] = {
    { "01", -1, 0, 0, 2},
    { "0111", -1, 0, 0, 2},
    { "01001", -1, 0, 0, 2},
    { "1", -1, 1, 1, 1},
    { "00000010", -1, 4, 2, 8},
    { "0000000", -1, 4, 3, 7}
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    int total_coeffs;
    int trailing_ones;
    init_reader (&reader, test_cases[i].bitstring);
    taa_h264_get_coeff_token_chroma_dc (&reader, &total_coeffs, &trailing_ones);

    snprintf (desc, sizeof(desc), "bits %s expects total_coeffs %d, got %d",
              test_cases[i].bitstring, test_cases[i].expect_total_coeffs, total_coeffs);
    FASSERT2 (test_cases[i].expect_total_coeffs == total_coeffs, desc);

    snprintf (desc, sizeof(desc), "bits %s expects trailing_ones %d, got %d",
              test_cases[i].bitstring, test_cases[i].expect_trailing_ones, trailing_ones);
    FASSERT2 (test_cases[i].expect_trailing_ones == trailing_ones, desc);
    unsigned num_bits = get_num_bits_read (&reader);
    FASSERT (num_bits == test_cases[i].expect_bits_advanced);
  }
}



typedef struct
{
  char * bitstring;
  int vlc_num;
  int expect_level;
} test_data_get_level;

static void test_cavlc_get_level_vlc_0 (void)
{
  bitreader_t reader;

  test_data_get_level test_cases[] = {
    { "1", 0, 1},
    { "01", 0, -1},
    { "001", 0, 2},
    { "0000000000001", 0, 7},
    { "00000000000001", 0, -7},
    { "0000000000000010000", 0, 8},
    { "0000000000000010001", 0, -8},
    { "0000000000000011110", 0, 15},
    { "0000000000000001111111111110", 0, 2047 + 16}
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    init_reader (&reader, test_cases[i].bitstring);
    int level = taa_h264_get_level_vlc_0 (&reader);

    snprintf (desc, sizeof(desc), "bits %s expects level %d, got %d",
              test_cases[i].bitstring, test_cases[i].expect_level, level);
    FASSERT2 (test_cases[i].expect_level == level, desc);
    unsigned num_bits = get_num_bits_read (&reader);
    FASSERT (num_bits == strlen (test_cases[i].bitstring));
  }
}


static void test_cavlc_get_level_vlc_n (void)
{
  bitreader_t reader;

  /* TODO: add more test cases. */
  test_data_get_level test_cases[] = {
    { "0010", 1, 3},
    { "00010", 1, 4},
    { "11", 1, -1},
    { "111", 2, -2},
    { "01010", 3, 6},
    { "00000000110", 2, 18}
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    init_reader (&reader, test_cases[i].bitstring);
    int level = taa_h264_get_level_vlc_n (&reader, test_cases[i].vlc_num);

    snprintf (desc, sizeof(desc), "bits %s expects level %d, got %d",
              test_cases[i].bitstring, test_cases[i].expect_level, level);
    FASSERT2 (test_cases[i].expect_level == level, desc);
    unsigned num_bits = get_num_bits_read (&reader);
    FASSERT (num_bits == strlen (test_cases[i].bitstring));
  }
}


typedef struct
{
  char * bitstring;
  int total_coeffs;
  int expect_total_zeros;
  int expect_advanced_bits;
} test_data_total_zeros;

static void test_cavlc_get_total_zeros (void)
{
  bitreader_t reader;

  test_data_total_zeros test_cases [] = {
    { "1", 1, 0, 1},
    { "000000001", 1, 15, 9},
    { "0011", 5, 2, 4},
    { "111", 5, 3, 3},
    { "000000", 8, 8, 6},
    { "1", 15, 1, 1},
    { "00", 14, 0, 2}
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    init_reader (&reader, test_cases[i].bitstring);
    int tz = taa_h264_get_total_zeros (&reader, test_cases[i].total_coeffs);

    snprintf (desc, sizeof(desc), "bits %s expects total_zeros %d, got %d",
              test_cases[i].bitstring, test_cases[i].expect_total_zeros, tz);
    FASSERT2 (test_cases[i].expect_total_zeros == tz, desc);
    unsigned num_bits = get_num_bits_read (&reader);
    FASSERT (num_bits == test_cases[i].expect_advanced_bits);
  }
}


static void test_cavlc_get_total_zeros_chroma_dc (void)
{
  bitreader_t reader;

  test_data_total_zeros test_cases [] = {
    { "1", 1, 0, 1},
    { "1", 2, 0, 1},
    { "1", 3, 0, 1},
    { "001", 1, 2, 3},
    { "0010", 1, 2, 3},
    { "000",  1, 3, 3},
    { "0001", 1, 3, 3},
    { "0000", 1, 3, 3},
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    init_reader (&reader, test_cases[i].bitstring);
    int tz = taa_h264_get_total_zeros_chroma_dc (&reader, test_cases[i].total_coeffs);

    snprintf (desc, sizeof(desc), "bits %s expects total_zeros %d, got %d",
              test_cases[i].bitstring, test_cases[i].expect_total_zeros, tz);
    FASSERT2 (test_cases[i].expect_total_zeros == tz, desc);
    unsigned num_bits = get_num_bits_read (&reader);
    FASSERT (num_bits == test_cases[i].expect_advanced_bits);
  }
}


typedef struct
{
  char * bitstring;
  int zeros_left;
  int expect_run;
} test_data_run_before;

static void test_cavlc_get_run_before (void)
{
  bitreader_t reader;

  test_data_run_before test_cases[] = {
    { "1", 1, 0},
    { "0", 1, 1},
    { "1", 2, 0},
    { "101", 6, 5},
    { "101", 7, 2},
    { "00000000001", 13, 14}
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    init_reader (&reader, test_cases[i].bitstring);
    int run = taa_h264_get_run_before (&reader, test_cases[i].zeros_left);

    snprintf (desc, sizeof(desc), "bits %s expects run_before %d, got %d",
              test_cases[i].bitstring, test_cases[i].expect_run, run);
    FASSERT2 (test_cases[i].expect_run == run, desc);
    unsigned num_bits = get_num_bits_read (&reader);
    FASSERT (num_bits == strlen (test_cases[i].bitstring));
  }
}


typedef struct
{
  char * bitstring;
  int expect_coeffs[16];
} test_data_cavlc;


static void test_cavlc_get_coeffs_luma (void)
{
  bitreader_t reader;
  int16_t coeffs_out[16];

  test_data_cavlc test_cases[] = {
    {
      .bitstring = "000010001110010111101101",
      .expect_coeffs = { 0, 3, 0, 1, -1, -1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    {
      .bitstring = "000000011010001001000010111001100",
      .expect_coeffs = { -2, 4, 3, -3, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    {
      .bitstring = "000000000010100011010100110011000001000111000000001001010100111110",
      .expect_coeffs = { 17, -6, 0, 6, -3, -2, 1, 1, 0, 1, 0, -2, 0, 1, 0, 0}
    },
    {
      .bitstring = "1",
      .expect_coeffs = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    {
      .bitstring = "00000000000010000001101010101010101010101010",
      .expect_coeffs = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    }
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_cavlc * test = &test_cases[i];

    init_reader (&reader, test->bitstring);
    taa_h264_get_coeffs_4x4_cavlc (&reader, coeffs_out, LUMA_4x4, 0);

    int diff;
    COMPARE_ARRAYS_DESC (coeffs_out, test->expect_coeffs, 16, desc, sizeof(desc), &diff);
    FASSERT2 (diff == 0, desc);

    unsigned num_bits = get_num_bits_read (&reader);
    FASSERT (num_bits == strlen (test->bitstring));
  }
}


static void test_cavlc_get_coeffs_chroma_ac (void)
{
  bitreader_t reader;
  int16_t coeffs_out[16];
  const int16_t dc = 999;

  test_data_cavlc test_cases[] = {
    {
      .bitstring = "000010001110010111101101",
      .expect_coeffs = { dc, 0, 3, 0, 1, -1, -1, 0, 1, 0, 0, 0, 0, 0, 0, 0}
    },
    {
      .bitstring = "000000011010001001000010111001100",
      .expect_coeffs = { dc, -2, 4, 3, -3, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    {
      .bitstring = "000000000010100011010100110011000001000111000000001001010100111110",
      .expect_coeffs = { dc, 17, -6, 0, 6, -3, -2, 1, 1, 0, 1, 0, -2, 0, 1, 0}
    },
    {
      .bitstring = "1",
      .expect_coeffs = { dc, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    {
      .bitstring = "000000000000110000011010101010101010101010",
      .expect_coeffs = { dc, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
    }
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_cavlc * test = &test_cases[i];

    init_reader (&reader, test->bitstring);
    coeffs_out[0] = dc;
    taa_h264_get_coeffs_4x4_cavlc (&reader, coeffs_out, CHROMA_AC, 0);

    int diff;
    COMPARE_ARRAYS_DESC (coeffs_out, test->expect_coeffs, 16, desc, sizeof(desc), &diff);
    FASSERT2 (diff == 0, desc);

    unsigned num_bits = get_num_bits_read (&reader);
    FASSERT (num_bits == strlen (test->bitstring));
  }
}

static void setup (void)
{
  nullfile = fopen (NULL_FILENAME, "w");
  if (nullfile == NULL) {
    fprintf (stderr, "Failed to open '%s', used for testing\n", NULL_FILENAME);
  }
  tracefile = nullfile;
}

void test_dec_getvlc (void)
{
  setup ();

  test_exp_golomb_unsigned ();
  test_exp_golomb_signed ();

  test_cavlc_get_coeff_token ();
  test_cavlc_get_coeff_token_chroma_dc ();
  test_cavlc_get_level_vlc_0 ();
  test_cavlc_get_level_vlc_n ();
  test_cavlc_get_total_zeros ();
  test_cavlc_get_total_zeros_chroma_dc ();
  test_cavlc_get_run_before ();
  test_cavlc_get_coeffs_luma ();
  test_cavlc_get_coeffs_chroma_ac ();
}
