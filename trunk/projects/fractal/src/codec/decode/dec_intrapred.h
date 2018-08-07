/* -*- indent-tabs-mode:nil; c-basic-offset:4 -*- */

#ifndef _INTRAPRED_H_
#define _INTRAPRED_H_

#include "com_common.h"


bool taa_h264_intra_4x4_pred_and_recon_luma (
  imode4_t        pred_mode,
  uint8_t *       recon,
  int             stride,
  const uint8_t * left,
  const uint8_t * top,
  bool            left_avail,
  bool            top_avail,
  bool            top_left_avail,
  bool            top_right_avail,
  const int16_t * diff);

bool taa_h264_intra_16x16_pred_and_recon_luma (
  imode16_t       pred_mode,
  uint8_t *       recon,
  int             stride,
  bool            left_avail,
  bool            top_avail,
  bool            topleft_avail,
  const int16_t * residual);


bool taa_h264_intra_pred_and_recon_chroma_8x8 (
  imode8_t        pred_mode,
  uint8_t *       recon,
  int             stride,
  bool            left_avail,
  bool            top_avail,
  bool            top_left_avail,
  const int16_t * diff);
#endif
