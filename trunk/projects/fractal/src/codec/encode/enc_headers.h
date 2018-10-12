#ifndef _ENC_HEADERS_H_
#define _ENC_HEADERS_H_

#include "enc.h"
#include <stdio.h>

unsigned taa_h264_write_nalu_header (
  bitwriter_t * writer,
  nalref_t      idc,
  nalutype_t    type,
  bool          long_start_code);

unsigned taa_h264_write_sequence_parameter_set (
  bitwriter_t *      writer,
  const sequence_t * sequence,
  unsigned       width,
  unsigned       height);

unsigned taa_h264_write_picture_parameter_set (
  bitwriter_t *      writer,
  const sequence_t * sequence);

unsigned taa_h264_write_slice_header (
  bitwriter_t *      writer,
  const sequence_t * sequence,
  nalutype_t         type,
  bool               islice,
  int                frame_num,
  int                first_mb,
  int                slice_qp,
  int                temporal_id,
  const rpm_t *      rpm,
  rplr_t *           rplr,
  slice_enc_t  *     curr_slice);

unsigned taa_h264_write_mb_header (
  mbinfo_t *    mb,
  bitwriter_t * writer,
  uint8_t *     intra_modes,
  int           mbrun,
  bool          islice,
  uint8_t *     last_coded_qp);

int taa_h264_write_ref_pic_marking (
  bitwriter_t * writer,
  const rpm_t * rpm);

nalref_t taa_h264_get_priority (
  nalutype_t type,
  int        temporal_id);

# if REF_FRAME_CHECKSUM

void taa_h264_write_checksum (
  bitwriter_t * w,
  uint32_t      checksum,
  int           long_term_frame_idx,
  int           original_frame_num);

# endif

#endif
