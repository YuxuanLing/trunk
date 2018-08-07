#ifndef _ENC_CONTROL_H_
#define _ENC_CONTROL_H_

#include "enc_typedefs.h"
#include "enc_gop.h"
#include "encoder.h"

struct control_s
{
  uint8_t num_layers;
  int skip_frames;
  bool enable_frame_skip;
  gop_t gop;
  bool fixed_qp;
  uint8_t frame_qp;
  int intra_period;
  int frames_since_intra;
  ratecontrol_t * ratecontrol;
};


control_t * taa_h264_control_create (
  uint8_t  num_layers,
  uint16_t num_mbs);

void taa_h264_control_init (
  control_t * ctrl,
  uint8_t     gop_size,
  uint8_t     framerate,
  uint16_t    bitrate,
  uint8_t     initial_qp,
  uint8_t     min_qp,
  uint8_t     max_qp,
  uint16_t    num_mbs);


void taa_h264_control_delete (
  control_t * ctrl);

void taa_h264_control_start_frame (
  control_t * ctrl,
  uint8_t     temporal_id,
  uint8_t     base_qp,
  uint8_t     framerate,
  uint16_t    bitrate,
  uint8_t     min_qp,
  uint8_t     max_qp,
  uint32_t    max_mbps,
  uint32_t    max_static_mbps
#ifdef TAA_SAVE_264_MEINFO
  ,unsigned char debug_flag
  ,FILE *fp_meta_info
  , uint8_t *data_need_send
  , unsigned *data_position
#endif
  );



void taa_h264_control_end_frame (
  control_t * ctrl,
  uint32_t    num_static_mbs
#ifdef TAA_SAVE_264_MEINFO
  ,int *skip_frame
  ,int *skip_mbps
  ,int *skip_bits
  ,int *length_compensate_ratio
  ,unsigned char debug_detail_flag
#endif
);

uint8_t taa_h264_control_mb (
  control_t * ctrl,
  uint8_t     prev_qp,
  uint32_t    prev_bits,
  uint16_t    mbpos);

uint8_t taa_h264_control_frame_type (
  control_t * ctrl,
  bool        enable_frame_skip,
  bool        request_intra,
  int         new_intra_period,
  uint8_t     new_gop_size,
  int         active_temporal_level,
  bool *      skip,
  bool *      intra);

#endif
