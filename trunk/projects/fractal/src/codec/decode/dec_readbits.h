#ifndef _READBITS_H_
#define _READBITS_H_

#include "decoder.h"

#include <stddef.h>

void taa_h264_load_buffer (
  bitreader_t * r,
  uint8_t *     buf,
  unsigned      size);
bool taa_h264_reader_has_more_data (
  bitreader_t * r);
int taa_h264_reader_bytes_left (
  bitreader_t * r);
uint32_t taa_h264_show_32bits (
  bitreader_t * r);
uint32_t taa_h264_show_bits (
  bitreader_t * r,
  unsigned      n);
uint32_t taa_h264_read_bits (
  bitreader_t * r,
  unsigned      n);
uint8_t taa_h264_read_byte_aligned (
  bitreader_t * r);
void taa_h264_advance_bits (
  bitreader_t * r,
  unsigned      n);
void taa_h264_advance_to_byte (
  bitreader_t * r);
void taa_h264_advance_rbsp_trailing_bits (
  bitreader_t * r);



#endif
