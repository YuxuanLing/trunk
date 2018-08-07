#ifndef _ENC_DPB_H_
#define _ENC_DPB_H_

#include "com_dpb.h"
#include "enc_typedefs.h"

#define MAX_RPM_REPEATS MAX_NUM_REF_FRAMES

/* Info for ref_pic_marking */
struct rpm_s
{
  bool idr_flag;
  int frame_num;
  mmco_t mmco[MAX_NUM_MMCO_CMDS];
};

/* Meta information associated with a reference picture */
struct ref_meta_enc_s
{
  rpm_t rpm;
  uint8_t age;
  uint8_t temporal_id;
  int long_term_idx;
};


void taa_h264_enc_dpb_init (
  decoded_picture_buffer_t * dpb,
  ref_meta_t *               refmeta,
  uint8_t                    max_ref_frames,
  int                        width,
  int                        height,
  bool                       dpb_has_interpolated,
  error_handler_t *          error_handler);

void taa_h264_enc_dpb_update (
  decoded_picture_buffer_t * dpb,
  int *                      frame_num,
  uint8_t                    temporal_id);

void taa_h264_generate_rplr (
  const decoded_picture_buffer_t * dpb,
  int                              frame_num,
  uint8_t                          ref_idx,
  rplr_t *                         rplr);

void taa_h264_generate_mmco_curr_to_long (
  decoded_picture_buffer_t * dpb,
  uint8_t                    ltfi,
  mmco_t *                   mmco);
  
int taa_h264_ref_pic_marking_repetition (
  decoded_picture_buffer_t * dpb,
  int * ack_status,
  ref_meta_t * rpm_list[MAX_NUM_REF_FRAMES],
  ref_meta_t * repeat_list[MAX_NUM_DPB_PICS]);

int taa_h264_ref_pic_marking_repetition_all_lt_frames (
    decoded_picture_buffer_t * dpb,
    ref_meta_t *               rpm_list[MAX_NUM_REF_FRAMES],
    ref_meta_t *               repeat_list[MAX_RPM_REPEATS]);

ref_meta_t * taa_h264_enc_dpb_curr_ref_meta (
  decoded_picture_buffer_t * dpb);

#endif
