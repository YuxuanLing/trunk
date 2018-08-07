#include "enc_sei.h"
#include "enc_writebits.h"
#include "com_sei.h"
#include "fractaltest.h"

#include <memory.h>

const uint8_t expect_header[] = {
  0x00, 0x00, 0x00, 0x01, // start code
  0x06 // nalu header
};

const uint8_t fixed_payload_byte = 0xab;

typedef struct
{
  int user_data_len;

} test_result;

#pragma warning (disable:869)
static bool assert_user_data (
  unsigned char * buf,
  unsigned        size,
  unsigned char * total_buf,
  unsigned        total_size,
  unsigned        lt_idx,
  unsigned        frame_num,
  bool            last_nalu_in_frame,
  void *          context)
{
  int original_datalen = *((int*) context);
  const uint8_t * bufend = buf + size;

  FASSERT (memcmp (buf, expect_header, sizeof expect_header) == 0);
  buf += sizeof expect_header;

  // Payload type (assume less than 255 bytes)
  FASSERT (*buf == 0x05);
  buf++;

  // Payload size (assume less than 255 bytes)
  int payload_size = 0;
  while (*buf == 0xff)
  {
    payload_size += 255;
    buf++;
  }
  payload_size += *buf;
  buf++;

  FASSERT (payload_size - sizeof uuid == original_datalen);

  FASSERT (memcmp (buf, uuid, sizeof uuid) == 0);
  for (int i = sizeof uuid; i < payload_size; i++)
    FASSERT (buf[i] == fixed_payload_byte);
  buf += payload_size;

  FASSERT (*buf == 0x80);

  return false;
}

static bool assert_escaped_user_data (
  unsigned char * buf,
  unsigned        size,
  unsigned char * total_buf,
  unsigned        total_size,
  unsigned        lt_idx,
  unsigned        frame_num,
  bool            last_nalu_in_frame,
  void *          context)
{
  int expected_escape_bytes = *((int*) context);
  const uint8_t * bufend = buf + size;

  FASSERT (memcmp (buf, expect_header, sizeof expect_header) == 0);
  buf += sizeof expect_header;

  // Payload type (assume less than 255 bytes)
  FASSERT (*buf == 0x05);
  buf++;

  // Payload size (assume less than 255 bytes)
  int payload_size = 0;
  while (*buf == 0xff)
  {
    payload_size += 255;
    buf++;
  }
  payload_size += *buf;
  buf++;

  // Count number of emulation prevention bytes
  int num_escapes = 0;
  for (int i = 0; buf + i + 2< bufend; i++)
  {
    if (buf[i] == 0 && buf[i+1] == 0 && buf[i+2] == 3)
      num_escapes++;
  }
  FASSERT (num_escapes == expected_escape_bytes);
  // We don't care about the payload values in this test
  buf += payload_size + num_escapes;

  FASSERT (*buf == 0x80);

  return false;
}
#pragma warning (default:869)

static void test_user_data (size_t datalen)
{
  uint8_t data[datalen];
  uint8_t nalu[datalen + 1000];
  unsigned max_packet_size = 1344;
  unsigned max_nalu_size = max_packet_size;

  bitwriter_t * writer = taa_h264_bitwriter_create();
  taa_h264_reset_writer (writer, nalu, false);

  memset (data, fixed_payload_byte, sizeof data);

  taa_h264_write_user_data_sei (writer, data, datalen);
  taa_h264_send_nonslice (writer, assert_user_data, &datalen, max_packet_size,
    &max_nalu_size);

  taa_h264_bitwriter_delete (writer);
}


static void test_user_data_escape (void)
{
  // data array that contains a byte sequence that needs to be escaped
  uint8_t data[] = {0xab, 0x00, 0x00, 0x02, 0x78};
  uint8_t nalu[64];
  int expect_escape_bytes = 1;
  unsigned max_packet_size = 1344;
  unsigned max_nalu_size = max_packet_size;

  bitwriter_t * writer = taa_h264_bitwriter_create();
  taa_h264_reset_writer (writer, nalu, false);

  taa_h264_write_user_data_sei (writer, data, sizeof data);
  taa_h264_send_nonslice (writer, assert_escaped_user_data, &expect_escape_bytes,
    max_packet_size, &max_nalu_size);

  taa_h264_bitwriter_delete (writer);
}


void test_enc_sei (void)
{
  test_user_data (20);
  test_user_data (1000);
  test_user_data_escape ();
}
