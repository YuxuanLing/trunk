#include "enc_writebits.c"
#include "fractaltest.h"

#include <string.h>

#define MAX_CODES_IN_TEST 16
typedef struct
{
  int num_codes;
  uint32_t codes[MAX_CODES_IN_TEST];
  uint32_t lengths[MAX_CODES_IN_TEST];
} test_data_code_buf;

static bitwriter_t writer;

static void test_code_buffer (void)
{
  test_data_code_buf test_cases[] = {
    {
      .num_codes = 1,
      .codes = { 0},
      .lengths = { 1},
    },
    {
      .num_codes = 1,
      .codes = { 1},
      .lengths = { 1},
    },
    {
      .num_codes = 1,
      .codes = { 8},
      .lengths = { 10},
    },
    {
      .num_codes = 4,
      .codes = { 0, 4, 1, 65535},
      .lengths = { 6, 4, 4, 16},
    }
  };

  int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_code_buf test = test_cases[i];
    uint8_t dummybuf[MAX_CODES_IN_TEST * 32];
    taa_h264_reset_writer (&writer, dummybuf, false);

    for (int j = 0; j < test.num_codes; j++)
      taa_h264_write_bits (&writer, test.lengths[j], test.codes[j]);

    for (int j = 0; j < test.num_codes; j++)
    {
      FASSERT ((writer.code_buf_base[j] & 0xFFFF) == test.lengths[j]);
      FASSERT ((writer.code_buf_base[j] >> 16) == test.codes[j]);
    }

    int num = writer.code_buf_ptr - writer.code_buf_base;
    FASSERT (num == test.num_codes);
  }
}


static void test_reserved_code_buffer (void)
{
  bitwriter_t writer;
  bitwriter_t * w = &writer;

  uint8_t dummybuf[MAX_CODES_IN_TEST * 32];
  taa_h264_reset_writer (w, dummybuf, false);

  taa_h264_write_bits (w, 4, 0xC);
  uint32_t * reserved_code = taa_h264_reserve_code (w);
  taa_h264_write_bits (w, 3, 0x6);
  taa_h264_write_reserved_code (reserved_code, 16, 0xBABE);

  FASSERT (w->code_buf_ptr - w->code_buf_base == 3);

  FASSERT ((w->code_buf_base[0] & 0xFFFF) == 4);
  FASSERT ((w->code_buf_base[1] & 0xFFFF) == 16);
  FASSERT ((w->code_buf_base[2] & 0xFFFF) == 3);

  FASSERT ((w->code_buf_base[0] >> 16) == 0xC);
  FASSERT ((w->code_buf_base[1] >> 16) == 0xBABE);
  FASSERT ((w->code_buf_base[2] >> 16) == 0x6);
}

typedef struct
{
  int num_codes;
  uint32_t codes[MAX_CODES_IN_TEST];
  uint32_t lengths[MAX_CODES_IN_TEST];
  uint8_t expect_buf [MAX_CODES_IN_TEST * 32];
  int expect_bytecount;
  int expect_bitrest;
} test_data_flush;


static void test_flush_code_buffer (void)
{
  test_data_flush test_cases[] = {
    {
      .num_codes = 1,
      .codes = { 1},
      .lengths = { 1},
      .expect_buf = { 0x80},
      .expect_bytecount = 1,
      .expect_bitrest = 31
    },
    {
      .num_codes = 5,
      .codes = { 1, 1, 1, 1, 1},
      .lengths = { 8, 8, 8, 8, 8},
      .expect_buf = { 0x01, 0x01, 0x01, 0x01, 0x01},
      .expect_bytecount = 5,
      .expect_bitrest = 24
    },
    {
      .num_codes = 4,
      .codes = { 1, 1, 1, 1},
      .lengths = { 8, 8, 8, 8},
      .expect_buf = { 0x01, 0x01, 0x01, 0x01},
      .expect_bytecount = 4,
      .expect_bitrest = 0
    },
    {
      .num_codes = 5,
      .codes = { 1, 1, 1, 1, 1},
      .lengths = { 8, 8, 8, 8, 7},
      .expect_buf = { 0x01, 0x01, 0x01, 0x01, 0x02},
      .expect_bytecount = 5,
      .expect_bitrest = 25
    },
    {
      .num_codes = 7,
      .codes = { 0, 1, 7, 66, 1, 0, 0},
      .lengths = { 1, 2, 5, 8, 1, 1, 1},
      .expect_buf = { 0x27, 0x42, 0x80},
      .expect_bytecount = 3,
      .expect_bitrest = 13
    },
    {
      .num_codes = 5,
      .codes =   { 2, 2, 3, 3, 2},
      .lengths = { 3, 2, 2, 2, 3},
      .expect_buf = { 0x57, 0xA0},
      .expect_bytecount = 2,
      .expect_bitrest = 20
    },
  };


  int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_flush test = test_cases[i];
    uint8_t buf[MAX_CODES_IN_TEST * 32];
    taa_h264_reset_writer (&writer, buf, false);

    for (int j = 0; j < test.num_codes; j++)
      taa_h264_write_bits (&writer, test.lengths[j], test.codes[j]);

    taa_h264_flush_code_buffer (&writer);

    FASSERT (writer.bytecount == test.expect_bytecount);
    FASSERT (writer.bitrest == test.expect_bitrest);
    FASSERT (writer.code_buf_base == writer.code_buf_ptr);
    FASSERT (memcmp (writer.bytebuf, test.expect_buf, writer.bytecount) == 0);

    /* Test to ensure that variables are set so that flushing twice
     * without adding new code symbols does not change the buffer. */
    taa_h264_flush_code_buffer (&writer);

    FASSERT (writer.bytecount == test.expect_bytecount);
    FASSERT (writer.bitrest == test.expect_bitrest);
    FASSERT (writer.code_buf_base == writer.code_buf_ptr);
    FASSERT (memcmp (writer.bytebuf, test.expect_buf, writer.bytecount) == 0);

  }
}

#define MAX_BUF_LEN  16
typedef struct
{
  uint8_t buf[MAX_BUF_LEN];
  int bitrest;
  int bytecount;
  int skip;
  uint8_t expect_buf[MAX_BUF_LEN];
  int expect_bytecount;
} test_data_postprocess;


static void test_trailing_bits (void)
{
  test_data_postprocess test_cases[] = {
    {
      .buf = { 0x01},
      .bytecount = 1,
      .bitrest = 24,
      .expect_buf = { 0x01, 0x80},
      .expect_bytecount = 2
    },
    {
      .buf = { 0x01, 0xAB, 0xCD, 0xEF},
      .bytecount = 4,
      .bitrest = 0,
      .expect_buf = { 0x01, 0xAB, 0xCD, 0xEF, 0x80},
      .expect_bytecount = 5
    },
    {
      .buf = { 0x02},
      .bytecount = 1,
      .bitrest = 25,
      .expect_buf = { 0x03},
      .expect_bytecount = 1
    },
    {
      .buf = { 0x80},
      .bytecount = 1,
      .bitrest = 31,
      .expect_buf = { 0xC0},
      .expect_bytecount = 1
    },
  };


  int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_postprocess test = test_cases[i];

    size_t ret = taa_h264_write_rbsp_trailing_bits (test.buf, test.bytecount, test.bitrest);

    FASSERT (ret == test.expect_bytecount);
    FASSERT (memcmp (test.buf, test.expect_buf, ret) == 0);
  }
}

static void test_escape_byte_sequence (void)
{
  test_data_postprocess test_cases[] = {
    {
      /* No escape needed */
      .buf = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06},
      .bytecount = 6,
      .skip = 0,
      .expect_buf = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06},
      .expect_bytecount = 6
    },
    {
      /* No escape needed */
      .buf = { 0x00, 0x00, 0x04, 0x00, 0x00, 0x05},
      .bytecount = 6,
      .skip = 0,
      .expect_buf = { 0x00, 0x00, 0x04, 0x00, 0x00, 0x05},
      .expect_bytecount = 6
    },
    {
      /* Escape needed */
      .buf = { 0x00, 0x00, 0x01, 0x00, 0x00, 0x02, 0x00, 0x00, 0x03},
      .bytecount = 9,
      .skip = 0,
      .expect_buf = { 0x00, 0x00, 0x03, 0x01, 0x00, 0x00, 0x03, 0x02, 0x00, 0x00, 0x03, 0x03},
      .expect_bytecount = 12
    },
    {
      /* Escape needed twice */
      .buf = { 0x00, 0x00, 0x00, 0x00, 0x00},
      .bytecount = 5,
      .skip = 0,
      .expect_buf = { 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00},
      .expect_bytecount = 7
    },
    {
      /* Escape not needed because of skip */
      .buf = { 0x00, 0x00, 0x01, 0x02, 0x03},
      .bytecount = 5,
      .skip = 3,
      .expect_buf = { 0x00, 0x00, 0x01, 0x02, 0x03},
      .expect_bytecount = 5
    },

  };


  int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_postprocess test = test_cases[i];

    int ret = taa_h264_escape_byte_sequnce (test.buf, test.bytecount, test.skip);

    FASSERT (ret == test.expect_bytecount);
    FASSERT (memcmp (test.buf, test.expect_buf, ret) == 0);
  }
}


void test_enc_writebits (void)
{
  test_code_buffer ();
  test_flush_code_buffer ();
  test_escape_byte_sequence ();
  test_trailing_bits ();
  test_reserved_code_buffer ();
}
