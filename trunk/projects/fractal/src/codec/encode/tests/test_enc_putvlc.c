#include "enc_putvlc.c"
#include "enc_writebits.c"
#include "fractaltest.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static bitwriter_t writer;

typedef struct
{
  uint16_t word;
  uint16_t len;
} code;

typedef struct
{
  int value;
  int vlc_num;
  code expect_code;
} test_data_vlc_simple;


static void test_exp_golomb_unsigned (void)
{
  test_data_vlc_simple test_cases[] = {
    {
      .value = 0,
      .expect_code = { .word = 1, .len = 1}
    },
    {
      .value = 1,
      .expect_code = { .word = 2, .len = 3}
    },
    {
      .value = 2,
      .expect_code = { .word = 3, .len = 3}
    },
    {
      .value = 3,
      .expect_code = { .word = 4, .len = 5}
    },
    {
      .value = 6,
      .expect_code = { .word = 7, .len = 5}
    },
    {
      .value = 7,
      .expect_code = { .word = 8, .len = 7}
    },
    {
      .value = 14,
      .expect_code = { .word = 15, .len = 7}
    },
    {
      .value = 31,
      .expect_code = { .word = 32, .len = 11}
    },
    {
      .value = 62,
      .expect_code = { .word = 63, .len = 11}
    }
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_vlc_simple test = test_cases[i];
/*        bitwriter_t writer; */
    taa_h264_reset_writer (&writer, NULL, false);

    unsigned len = taa_h264_put_uvlc (&writer, (unsigned) test.value);

    int codeword = writer.code_buf_base[0] >> 16;
    int codelen = writer.code_buf_base[0] & 0xff;

    FASSERT (codeword == test.expect_code.word);
    FASSERT (codelen == test.expect_code.len);
    FASSERT (len == codelen);
  }
}

static void test_exp_golomb_signed (void)
{
  test_data_vlc_simple test_cases[] = {
    {
      .value = 0,
      .expect_code = { .word = 1, .len = 1}
    },
    {
      .value = 1,
      .expect_code = { .word = 2, .len = 3}
    },
    {
      .value = -1,
      .expect_code = { .word = 3, .len = 3}
    },
    {
      .value = 2,
      .expect_code = { .word = 4, .len = 5}
    },
    {
      .value = -2,
      .expect_code = { .word = 5, .len = 5}
    },
    {
      .value = 3,
      .expect_code = { .word = 6, .len = 5}
    },
    {
      .value = -3,
      .expect_code = { .word = 7, .len = 5}
    },
    {
      .value = -127,
      .expect_code = { .word = 255, .len = 15}
    },
    {
      .value = 128,
      .expect_code = { .word = 256, .len = 17}
    }
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_vlc_simple test = test_cases[i];
    taa_h264_reset_writer (&writer, NULL, false);

    unsigned len = taa_h264_put_level (&writer, test.value);

    int codeword = writer.code_buf_base[0] >> 16;
    int codelen = writer.code_buf_base[0] & 0xff;

    FASSERT (codeword == test.expect_code.word);
    FASSERT (codelen == test.expect_code.len);
    FASSERT (len == codelen);
  }
}


typedef struct
{
  int total_coeffs;
  int trailing_ones;
  int vlc_num;
  code expect_code;
} test_data_coeff_token;

static void test_cavlc_put_coeff_token (void)
{
  test_data_coeff_token test_cases[] = {
    {
      .total_coeffs = 5,
      .trailing_ones = 3,
      .vlc_num = 0,
      .expect_code = { .word = 4, .len = 7},
    },
    {
      .total_coeffs = 5,
      .trailing_ones = 1,
      .vlc_num = 0,
      .expect_code = { .word = 6, .len = 10},
    },
    {
      .total_coeffs = 15,
      .trailing_ones = 0,
      .vlc_num = 0,
      .expect_code = { .word = 7, .len = 16},
    },
    {
      .total_coeffs = 0,
      .trailing_ones = 0,
      .vlc_num = 3,
      .expect_code = { .word = 3, .len = 6},
    },
    {
      .total_coeffs = 1,
      .trailing_ones = 1,
      .vlc_num = 3,
      .expect_code = { .word = 1, .len = 6},
    },
    {
      .total_coeffs = 4,
      .trailing_ones = 1,
      .vlc_num = 3,
      .expect_code = { .word = 13, .len = 6},
    },
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_coeff_token test = test_cases[i];
/*        bitwriter_t writer; */
    taa_h264_reset_writer (&writer, NULL, false);

    unsigned ret = taa_h264_put_coeff_token (&writer, test.total_coeffs,
                                         test.trailing_ones, test.vlc_num);

    int codeword = writer.code_buf_base[0] >> 16;
    int codelen = writer.code_buf_base[0] & 0xff;

    FASSERT (codeword == test.expect_code.word);
    FASSERT (codelen == test.expect_code.len);
    FASSERT (ret == codelen);
  }
}



static void test_cavlc_put_coeff_token_chroma_dc (void)
{
  test_data_coeff_token test_cases[] = {
    {
      .total_coeffs = 0,
      .trailing_ones = 0,
      .expect_code = { .word = 1, .len = 2},
    },
    {
      .total_coeffs = 1,
      .trailing_ones = 1,
      .expect_code = { .word = 1, .len = 1},
    },
    {
      .total_coeffs = 4,
      .trailing_ones = 2,
      .expect_code = { .word = 2, .len = 8},
    },
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_coeff_token test = test_cases[i];
/*        bitwriter_t writer; */
    taa_h264_reset_writer (&writer, NULL, false);

    unsigned ret = taa_h264_put_coeff_token_chroma_dc (&writer, test.total_coeffs, test.trailing_ones);

    int codeword = writer.code_buf_base[0] >> 16;
    int codelen = writer.code_buf_base[0] & 0xff;

    FASSERT (codeword == test.expect_code.word);
    FASSERT (codelen == test.expect_code.len);
    FASSERT (ret == codelen);
  }
}

static void test_cavlc_put_level_vlc_0 (void)
{
  test_data_vlc_simple test_cases[] = {
    {
      .value = 1,
      .expect_code = { .word = 1, .len = 1}
    },
    {
      .value = -2,
      .expect_code = { .word = 1, .len = 4}
    }
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_vlc_simple test = test_cases[i];
    taa_h264_reset_writer (&writer, NULL, false);

    unsigned ret = taa_h264_put_level_vlc_0 (&writer, abs (test.value), test.value < 0);

    int codeword = writer.code_buf_base[0] >> 16;
    int codelen = writer.code_buf_base[0] & 0xff;

    FASSERT (codeword == test.expect_code.word);
    FASSERT (codelen == test.expect_code.len);
    FASSERT (ret == codelen);
  }
}


static void test_cavlc_put_level_vlc_n (void)
{
  test_data_vlc_simple test_cases[] = {
    {
      .value = 3,
      .vlc_num = 1,
      .expect_code = { .word = 2, .len = 4}
    },
    {
      .value = 4,
      .vlc_num = 1,
      .expect_code = { .word = 2, .len = 5}
    },
    {
      .value = -46,
      .vlc_num = 2,
      .expect_code = { .word = 4127, .len = 28}
    },

  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_vlc_simple test = test_cases[i];
    taa_h264_reset_writer (&writer, NULL, false);

    unsigned ret = taa_h264_put_level_vlc_n (
      &writer, abs (test.value), test.value < 0, test.vlc_num);

    int codeword = writer.code_buf_base[0] >> 16;
    int codelen = writer.code_buf_base[0] & 0xff;

    FASSERT (codeword == test.expect_code.word);
    FASSERT (codelen == test.expect_code.len);
    FASSERT (ret == codelen);
  }
}


typedef struct
{
  coeffs4_t coeffs4;
  char * expect_code;
  int expect_total_coeffs;
} test_data_cavlc;

static void test_cavlc_luma (void)
{
  /* Some of these are examples from http://vcodex.com/files/h264_vlc.pdf */
  test_data_cavlc test_cases [] = {
    {
      .coeffs4 = {
        .num_nonzero = 5,
        .level = {3, 1, 1, 1, 1, },
        .sign  = {0, 0, 1, 1, 0, },
        .run   = {1, 1, 0, 0, 1, },
      },
      .expect_code = "000010001110010111101101",
      .expect_total_coeffs = 5
    },
    {
      .coeffs4 = {
        .num_nonzero = 5,
        .level = {2, 4, 3, 3, 1, },
        .sign  = {1, 0, 0, 1, 1, },
        .run   = {0, 0, 0, 0, 2, },
      },
      .expect_code = "000000011010001001000010111001100",
      .expect_total_coeffs = 5
    },
    {
      .coeffs4 = {
        .num_nonzero = 10,
        .level = { 17, 6, 6, 3, 2, 1, 1, 1, 2, 1, },
        .sign  = {  0, 1, 0, 1, 1, 0, 0, 0, 1, 0, },
        .run   = {  0, 0, 1, 0, 0, 0, 0, 1, 1, 1, },
      },
      .expect_code = "000000000010100011010100110011000001000111000000001001010100111110",
      .expect_total_coeffs = 10
    },
    {
      .coeffs4 = {
        .num_nonzero = 0,
        .level = { 0, },
        .sign  = { 0, },
        .run   = { 0, },
      },
      .expect_code = "1",
      .expect_total_coeffs = 0
    },
    {
      .coeffs4 = {
        .num_nonzero = 16,
        .level = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        .sign  = {  0, },
        .run   = {  0, },
      },
      .expect_code = "00000000000010000001101010101010101010101010",
      .expect_total_coeffs = 16
    },
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_cavlc test = test_cases[i];
    char result_code[256];
    taa_h264_reset_writer (&writer, NULL, false);

    int len;
    unsigned total_coeffs_out;

    len = taa_h264_encode_coeffs_4x4_cavlc (&writer, &test.coeffs4, LUMA_4x4, 0, &total_coeffs_out);
    taa_h264_code_buffer_to_string_ (
      &writer, result_code, sizeof result_code / sizeof *result_code);

    FASSERT (len == strlen (test.expect_code));
    FASSERT (strcmp (result_code, test.expect_code) == 0);
    FASSERT (total_coeffs_out == test.expect_total_coeffs);
  }
}

static void test_cavlc_chroma_ac (void)
{
  /* Some of these are examples from http://vcodex.com/files/h264_vlc.pdf */
  test_data_cavlc test_cases [] = {
    {
      .coeffs4 = {
        .num_nonzero = 5,
        .level = {99, 3, 1, 1, 1, 1, },
        .sign  = {99, 0, 0, 1, 1, 0, },
        .run   = {99, 1, 1, 0, 0, 1, },
      },
      .expect_code = "000010001110010111101101",
      .expect_total_coeffs = 5
    },
    {
      .coeffs4 = {
        .num_nonzero = 5,
        .level = {99, 2, 4, 3, 3, 1, },
        .sign  = {99, 1, 0, 0, 1, 1, },
        .run   = {99, 0, 0, 0, 0, 2, },
      },
      .expect_code = "000000011010001001000010111001100",
      .expect_total_coeffs = 5
    },
    {
      .coeffs4 = {
        .num_nonzero = 10,
        .level = { 99, 17, 6, 6, 3, 2, 1, 1, 1, 2, 1, },
        .sign  = { 99,  0, 1, 0, 1, 1, 0, 0, 0, 1, 0, },
        .run   = { 99,  0, 0, 1, 0, 0, 0, 0, 1, 1, 1, },
      },
      .expect_code = "000000000010100011010100110011000001000111000000001001010100111110",
      .expect_total_coeffs = 10
    },
    {
      .coeffs4 = {
        .num_nonzero = 0,
        .level = { 99, 0, },
        .sign  = { 99, 0, },
        .run   = { 99, 0, },
      },
      .expect_code = "1",
      .expect_total_coeffs = 0
    },
    {
      .coeffs4 = {
        .num_nonzero = 15,
        .level = { 99, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        .sign  = {  0, },
        .run   = {  0, },
      },
      .expect_code = "000000000000110000011010101010101010101010",
      .expect_total_coeffs = 15
    },
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_cavlc test = test_cases[i];
    char result_code[256];
    taa_h264_reset_writer (&writer, NULL, false);

    int len;
    unsigned total_coeffs_out;

    len = taa_h264_encode_coeffs_4x4_cavlc (&writer, &test.coeffs4, CHROMA_AC, 0, &total_coeffs_out);
    taa_h264_code_buffer_to_string_ (
      &writer, result_code, sizeof result_code / sizeof *result_code);

    FASSERT (len == strlen (test.expect_code));
    FASSERT (strcmp (result_code, test.expect_code) == 0);
    FASSERT (total_coeffs_out == test.expect_total_coeffs);
  }
}

void test_enc_putvlc (void)
{
  test_exp_golomb_unsigned ();
  test_exp_golomb_signed ();

  test_cavlc_luma ();
  test_cavlc_chroma_ac ();
  /* TODO: Add tests for chroma dc full examples and ac examples. */
  test_cavlc_put_coeff_token ();
  test_cavlc_put_coeff_token_chroma_dc ();
  test_cavlc_put_level_vlc_0 ();
  test_cavlc_put_level_vlc_n ();
}
