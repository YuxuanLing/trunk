#ifndef _ENC_MB_H_
#define _ENC_MB_H_

#include "encoder.h"

void taa_h264_encode_mb (
  frameinfo_t *  frameinfo,
  mbinfo_t *     currmb,
  const bool     intra,
  const unsigned coding_options);

void taa_h264_encode_mb_ssse3 (
  frameinfo_t * frameinfo,
  mbinfo_t *    currmb,
  const bool     intra,
  const unsigned coding_options);

unsigned taa_h264_encode_i4x4_luma (
  const bool intra,
  mbinfo_t * mb,
  uint8_t * intra_modes_ptr,
  const unsigned coding_options);

unsigned taa_h264_encode_i4x4_luma_ssse3 (
  const bool intra,
  mbinfo_t * mb,
  uint8_t * intra_modes_ptr,
  const unsigned coding_options);

#endif
