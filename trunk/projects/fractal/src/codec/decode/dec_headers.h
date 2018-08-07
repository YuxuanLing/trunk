#ifndef _HEADERS_H_
#define _HEADERS_H_

#include "dec_typedefs.h"
#include "dec_readbits.h"

void taa_h264_read_nalu_header (
  bitreader_t *     reader,
  nalref_t *        idc,
  nalutype_t *      type,
  error_handler_t * eh);

int taa_h264_read_sequence_parameter_set (
  bitreader_t *  r,
  seq_parset_t * sps_array);

int taa_h264_read_picture_parameter_set (
  bitreader_t *  r,
  pic_parset_t * pps_array);

void taa_h264_read_slice_header (
  bitreader_t *        r,
  nalref_t             idc,
  nalutype_t           type,
  const seq_parset_t * sps_array,
  const pic_parset_t * pps_array,
  sliceinfo_t *        slice,
  error_handler_t *    error_handler);

int taa_h264_read_mb_header (
  frameinfo_t * frameinfo,
  mbinfo_t *    mb,
  bool          islice,
  bitreader_t * r,
  unsigned *    prev_qp);

bool taa_h264_read_prefix (
  bitreader_t * r,
  nalref_t      idc,
  int *         layer_index);

bool taa_h264_read_mmco (
  mmco_t *      mmco,
  bitreader_t * r);

# if REF_FRAME_CHECKSUM

void taa_h264_read_checksum (
  bitreader_t * r,
  uint32_t *    checksum,
  int *         long_term_frame_idx,
  int *         original_frame_num);

# endif

#endif
