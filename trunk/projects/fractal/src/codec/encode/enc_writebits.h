#ifndef _WRITEBITS_H_
#define _WRITEBITS_H_

#include "taah264enc.h"
#include "enc_typedefs.h"

bitwriter_t * taa_h264_bitwriter_create (
  void);

void taa_h264_bitwriter_delete (
  bitwriter_t * w);

void taa_h264_reset_writer (
  bitwriter_t * w,
  uint8_t *     bytebuf,
  bool          aggregate);

void taa_h264_save_writer_state (
  bitwriter_t * w);

void taa_h264_load_writer_state (
  bitwriter_t * w);

unsigned taa_h264_write_bits (
  bitwriter_t * w,
  unsigned      n,
  uint32_t      val);

void taa_h264_flush_code_buffer (
  bitwriter_t * w);

void taa_h264_flush_code_buffer_if_needed (
  bitwriter_t * w,
  int           need);

int taa_h264_available_codes_before_flush (
  bitwriter_t * w);

void taa_h264_send_slice (
  bitwriter_t * w,
  nal_cb_t      func,
  unsigned      longterm_idx,
  unsigned      frame_num,
  bool          last_nalu_in_frame,
  void *        context,
  unsigned      max_packet_size,
  unsigned *    max_nalu_size,
  unsigned *    current_nal_size);

void taa_h264_send_nonslice (
  bitwriter_t * w,
  nal_cb_t      func,
  void *        context,
  unsigned      max_packet_size,
  unsigned *    max_nalu_size);

void taa_h264_write_byte_no_check (
  bitwriter_t * w,
  uint8_t       byte);

uint32_t * taa_h264_reserve_code (
  bitwriter_t * w);

void taa_h264_write_reserved_code (
  uint32_t * code_buf_ptr,
  unsigned   n,
  uint32_t   val);

void taa_h264_code_buffer_to_string_ (
  const bitwriter_t * w,
  char bitstring[],
  int maxlen);

int taa_h264_bitwriter_get_bitrest(bitwriter_t *w);

#endif
