#ifndef _DEC_DPB_H_
#define _DEC_DPB_H_

#include "com_dpb.h"
#include "dec_typedefs.h"
#include "taah264dec.h"

struct ref_meta_dec_s
{
  /* Whether this ref frame is assumed to be in sync (correct) with encoder */
  bool in_sync;
  /* Whether this ref frame is good enough to be used as reference. */
  bool usable_ref;
  /* The frame_num that was signaled with the frame */
  int original_frame_num;
};

bool taa_h264_dec_new_dpb_needed (
  const int dpb_maxw, 
  const int dpb_maxh, 
  const int dpb_refframes,
  const int frameinfo_w, 
  const int frameinfo_h,
  const int frameinfo_refframes);

int taa_h264_dec_dpb_size (
  const int width, 
  const int height, 
  const int num_ref_frames);

void taa_h264_dec_dpb_init_metainfo (
  decoded_picture_buffer_t * dpb,
  ref_meta_t *               refmeta);

bool taa_h264_dec_dpb_update_metainfo (
  decoded_picture_buffer_t * dpb,
  int                        frame_num,
  bool                       all_mbs_decoded,
  bool                       ref_is_in_sync);


bool taa_h264_dpb_get_conceal_src (
  decoded_picture_buffer_t * dpb,
  framebuf_t *               previous);

bool taa_h264_dec_dpb_ref_exist (
  const decoded_picture_buffer_t * dpb,
  unsigned                         index);

bool taa_h264_dec_dpb_ref_in_sync (
  const decoded_picture_buffer_t * dpb,
  unsigned                         index);

void taa_h264_dec_dpb_set_all_short_term_out_of_sync (
  decoded_picture_buffer_t * dpb);

void taa_h264_dec_dpb_set_idr_as_unusable_reference (
  decoded_picture_buffer_t * dpb);

void taa_h264_dec_dpb_fill_gap_in_frame_num (
  decoded_picture_buffer_t * dpb,
  int                        prev_frame_num,
  int                        curr_frame_num);

void taa_h264_dec_dpb_rpm_repetition (
  decoded_picture_buffer_t * dpb,
  int                        original_frame_num,
  bool                       original_idr_flag,
  bool                       idr_is_long_term,
  bool                       adaptive_marking_flag,
  const mmco_t *             mmco,
  callbacks_t *              cb);

void taa_h264_dec_dpb_rpm_ack (
  decoded_picture_buffer_t * dpb,
  unsigned                   frame_num,
  bool                       is_ref,
  bool                       is_idr,
  bool                       is_idr_long_term,
  bool                       adaptive_marking_flag,
  const mmco_t *             mmco,
  callbacks_t *              cb);


# if REF_FRAME_CHECKSUM

void taa_h264_dpb_assert_checksum (
  decoded_picture_buffer_t * dpb,
  uint32_t checksum,
  int long_term_frame_idx,
  int original_frame_num);

# endif

#endif
