#include "dec_readbits.h"
#include "fractaltest.h"

#include <string.h>
#include <stdlib.h>

static uint8_t * big_buf = NULL;
static char desc[128];

static void init_reader (
  bitreader_t * r,
  const char *  bitstring,
  unsigned      offset)
{
  unsigned n = strlen (bitstring);
  int numbits = offset + n;
  int buf_size = numbits / 8;
  if (numbits % 8 != 0)
    buf_size += 1;

  if (big_buf)
    TAA_H264_FREE (big_buf);
  big_buf = TAA_H264_MALLOC (buf_size);

  /* Clear the offset memory */
  for (unsigned i = 0; i < (offset + 7) / 8; i++)
    big_buf[i] = 0;

  /* Put the strings into memory */
  for (unsigned i = 0; i < n; i++)
  {
    unsigned bit = (bitstring[i] == '1');
    unsigned bitpos = (i + offset) % 8;
    unsigned bytepos = (i + offset) / 8;
    bit <<= 7 - bitpos;
    if (bitpos == 0)
      big_buf[bytepos] = (uint8_t) bit;
    else
      big_buf[bytepos] = (uint8_t)(big_buf[bytepos] | bit);
  }

  taa_h264_load_buffer (r, big_buf, buf_size);
  taa_h264_advance_bits (r, offset);
}


static void init_reader_simple (
  bitreader_t * r,
  int           bytes_to_load)
{
  if (big_buf)
    TAA_H264_FREE (big_buf);
  big_buf = TAA_H264_MALLOC (bytes_to_load);

  /* Clear the offset memory */
  for (int i = 0; i < bytes_to_load; i++)
    big_buf[i] = 0xBA;

  taa_h264_load_buffer (r, big_buf, bytes_to_load);
}



typedef struct
{
  char * buffer_bits;
  int buffer_offset;                                    /* How many bits to simulate before
                                                         * the "buffer_bits" */
  int num_bits_to_show;
  uint32_t expect_value;
} test_data_show_bits;

static void test_show_bits (void)
{
  bitreader_t r;
  test_data_show_bits test_cases [] = {
    { "0", 0, 0, 0},
    { "0", 0, 1, 0},
    { "1", 0, 1, 1},
    { "11", 0, 1, 1},
    { "11", 0, 2, 3},
    { "11", 5, 2, 3},
    { "010101", 30, 4, 5}
  };

  int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_show_bits test = test_cases[i];

    init_reader (&r, test.buffer_bits, test.buffer_offset);
    uint32_t result = taa_h264_show_bits (&r, test.num_bits_to_show);

    snprintf (desc, sizeof (desc), "show %d bits from %s expects value %u, got %u",
              test.num_bits_to_show, test.buffer_bits + test.buffer_offset,
              test.expect_value, result);
    FASSERT2 (result == test.expect_value, desc);
  }
}


#define READS_PER_TEST 3
typedef struct
{
  char * buffer_bits;
  int num_bits[READS_PER_TEST];
  uint32_t expect_value[READS_PER_TEST];
} test_data_read_bits;


static void test_read_bits (void)
{
  test_data_read_bits test_cases [] = {
    { "101", { 1, 1, 1}, { 0x1, 0x0, 0x1}},
    { "100000000000000000000000000000010001000000000000000000000000000000000000",
      { 31, 4, 32}, { 0x40000000, 0x8, 0x80000000}}
  };

  bitreader_t r;

  int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_read_bits test = test_cases[i];

    init_reader (&r, test.buffer_bits, 0);

    for (int j = 0; j < READS_PER_TEST; j++)
    {
      int num_bits = test.num_bits[j];
      uint32_t expect_value = test.expect_value[j];
      uint32_t result = taa_h264_read_bits (&r, num_bits);

      snprintf (desc, sizeof (desc),
                "read # %d from bits %s expects value %u, got %u",
                j, test.buffer_bits, expect_value, result);
      FASSERT2 (result == expect_value, desc);
    }
  }
}

#if 0
typedef struct
{
  uint8_t * buf;
  size_t size;
  uint8_t * expect;
} test_data_preprocess_buffer;

static void test_preprocess_buffer (
  void)
{
  unittest_context ("preprocess_buffer");

  test_data_preprocess_buffer test_cases [] = {
    { .buf = &{ (uint8_t) 0xBA}, .size = 1, .expect = &{ (uint8_t) 0xBA}},
  };

  int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_read_bits test = test_cases[i];

    preprocess_buffer (test.buf, test.size);
    FASSERT (memcmp (test.buf, test.expect, test.size) == 0);
  }
}
#endif


typedef struct
{
  int bytes_to_load;
  int bits_to_advance;
  int expected_bytes_left;
} test_data_bytes_left;

static void test_bytes_left (void)
{
  bitreader_t r;
  test_data_bytes_left test_cases [] = {
    {
      .bytes_to_load = 1,
      .bits_to_advance = 0,
      .expected_bytes_left = 1
    },
    {
      .bytes_to_load = 1,
      .bits_to_advance = 1,
      .expected_bytes_left = 0
    },
    {
      .bytes_to_load = 1,
      .bits_to_advance = 8,
      .expected_bytes_left = 0
    },
    {
      .bytes_to_load = 3,
      .bits_to_advance = 8,
      .expected_bytes_left = 2
    },
    {
      .bytes_to_load = 10,
      .bits_to_advance = 32,
      .expected_bytes_left = 6
    },
    {
      .bytes_to_load = 10,
      .bits_to_advance = 25,
      .expected_bytes_left = 6
    },
  };

  int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_bytes_left * test = &test_cases[i];
    init_reader_simple (&r, test->bytes_to_load);
    taa_h264_advance_bits (&r, test->bits_to_advance);
    FASSERT (test->expected_bytes_left == taa_h264_reader_bytes_left (&r));
  }

  // A custom test, advancing between each check

  init_reader (&r, "000000001111111100000000111111110000000011111111", 0);

  // byte aligned
  FASSERT (6 == taa_h264_reader_bytes_left (&r));

  // advance one byte, still byte aligned
  taa_h264_advance_bits (&r, 8);
  FASSERT (5 == taa_h264_reader_bytes_left (&r));

  // one bit under byte aligned
  taa_h264_advance_bits (&r, 1);
  FASSERT (4 == taa_h264_reader_bytes_left (&r));

  // advance a lot (one bit under byte aligned)
  taa_h264_advance_bits (&r, 24);
  FASSERT (1 == taa_h264_reader_bytes_left (&r));

  // byte aligned
  taa_h264_advance_bits (&r, 7);
  FASSERT (1 == taa_h264_reader_bytes_left (&r));

  // one bit under byte aligned
  taa_h264_advance_bits (&r, 1);
  FASSERT (0 == taa_h264_reader_bytes_left (&r));
}


static void test_has_more_data (void)
{
  bitreader_t r;

  // A valid bit sequence must end with a one (trailing rbsp bit)
  init_reader (&r, "1", 0);
  FASSERT (false == taa_h264_reader_has_more_data (&r));

  init_reader (&r, "01", 0);
  FASSERT (true == taa_h264_reader_has_more_data (&r));
  taa_h264_advance_bits (&r, 1);
  FASSERT (false == taa_h264_reader_has_more_data (&r));

  init_reader (&r, "00000001", 0);
  taa_h264_advance_bits (&r, 6);
  FASSERT (true == taa_h264_reader_has_more_data (&r));
  taa_h264_advance_bits (&r, 1);
  FASSERT (false == taa_h264_reader_has_more_data (&r));

  init_reader (&r, "00000000111111110000000011111111000000001", 0);
  taa_h264_advance_bits (&r, 32);
  FASSERT (true == taa_h264_reader_has_more_data (&r));
  taa_h264_advance_bits (&r, 8);
  FASSERT (false == taa_h264_reader_has_more_data (&r));

  init_reader (&r, "111111110000000011111111000000001", 0);
  taa_h264_advance_bits (&r, 25);
  FASSERT (true == taa_h264_reader_has_more_data (&r));
  taa_h264_advance_bits (&r, 7);
  FASSERT (false == taa_h264_reader_has_more_data (&r));
}


void test_dec_readbits (void)
{
  test_show_bits ();
  test_read_bits ();
  test_bytes_left ();
  test_has_more_data ();
  if (big_buf)
    TAA_H264_FREE (big_buf);
}
