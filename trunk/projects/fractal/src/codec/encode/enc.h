#ifndef _TNTMENC_H_
#define _TNTMENC_H_

#include "taah264enc.h"
#include "encoder.h"

bool taa_h264_enc_init (
  encoder_t * encoder,
  int         width,
  int         height,
  uint16_t    sar_width,
  uint16_t    sar_height,
  int         num_ref_frames,
  int         temporal_levels,
  int         profile_level
#ifdef TAA_SAVE_264_MEINFO
  ,unsigned char debug_flag
  ,uint8_t     *key_word
  ,uint8_t    meta_detail_flag
  ,char debug_ip_addr[32]
  ,unsigned int debug_port
#endif
#ifdef TAA_DEBUG_READ_YUV_FILE
  ,unsigned used_yuvfile_flag
  ,FILE *fp
  ,unsigned total_frame
  ,unsigned yuv_w
  ,unsigned yuv_h
#endif
   );

/* TODO: Move this elsewhere */
int taa_h264_get_cbp_luma(mbinfo_t * mb);
int taa_h264_get_cbp_chroma(mbinfo_t * mb);

void taa_h264_enc_free (
  encoder_t * encoder);

int taa_h264_process_frame (
  encoder_t *        encoder,
  const uint8_t *    input,
  const int          input_width,
  const int          input_height,
  const unsigned     intra_period,
  const uint8_t      gop_size,
  bool               idr_frame,
  const uint8_t      min_qp,
  const uint8_t      max_qp,
  const uint8_t      max_qpi,
  const uint8_t      pic_qp,
  const uint16_t     bitrate,
  const uint8_t      framerate,
  const uint32_t     max_mbps,
  const uint32_t     max_static_mbps,
  const bool         enable_frame_skip,
  const int          active_temporal_levels,
  const flux_t *     flux,
  const uint32_t     max_nalu_size,
  const unsigned     coding_options,
  const unsigned     use_high_profile,
  uint8_t *          outbuf);
#endif
