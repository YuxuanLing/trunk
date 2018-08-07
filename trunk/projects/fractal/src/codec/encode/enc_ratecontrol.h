#ifndef _RATECONTROL_H_
#define _RATECONTROL_H_

#include "taah264stdtypes.h"
#include "enc_typedefs.h"

ratecontrol_t * taa_h264_rate_control_create (
  int num_mbs);

void taa_h264_rate_control_delete (
  ratecontrol_t * rc);

void taa_h264_rate_control_init (
  ratecontrol_t * rc,
  uint8_t         framerate,                       /* fps */
  uint16_t        bitrate,                         /* kbps */
  uint8_t         initial_qp,
  uint8_t         min_qp,
  uint8_t         max_qp,
  uint16_t        num_mbs);

bool taa_h264_rate_control_start_frame (
  ratecontrol_t * rc);

void taa_h264_rate_control_update_target (
  ratecontrol_t * rc,
  uint8_t         framerate,                                /* fps */
  uint16_t        bitrate,                                  /* kbps */
  uint8_t         min_qp,
  uint8_t         max_qp,
  uint32_t        max_mbps,
  uint32_t        max_static_mbps);

uint8_t taa_h264_rate_control_mb (
  ratecontrol_t * rc,
  uint8_t         curr_qp,
  uint32_t        curr_bits,
  uint16_t        mbpos);

int taa_h264_rate_contol_end_frame (
  ratecontrol_t * rc,
  const int       num_static_mbs /* Total number of skipped MBs in frame */
#ifdef TAA_SAVE_264_MEINFO
  ,int *skip_mbps
  ,int *skip_bits
  ,int *length_compensate_ratio
#endif
);

#endif
