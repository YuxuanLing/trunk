#include "enc_dpb.h"
#include "enc_config.h"
#include <string.h>


static void taa_h264_enc_dpb_init_metainfo (
  decoded_picture_buffer_t * dpb,
  ref_meta_t *               refmeta)
{
  for (int i = 0; i < dpb->max_ref_frames + 1; i++)
  {
    dpb->pictures[i].meta = &refmeta[i];
    refmeta[i].age = 0;
    refmeta[i].temporal_id = 0;
  }
}


void taa_h264_enc_dpb_init (
  decoded_picture_buffer_t * dpb,
  ref_meta_t *               refmeta,
  uint8_t                    max_ref_frames,
  int                        width,
  int                        height,
  bool                       has_interpolated,
  error_handler_t *          error_handler)
{
  taa_h264_dpb_init (dpb, max_ref_frames, width, height,
                     1 << LOG2_MAX_FRAME_NUM, has_interpolated, error_handler);
  taa_h264_enc_dpb_init_metainfo (dpb, refmeta);
}

static void sort_rpm_repeats (
  ref_meta_t * list[MAX_RPM_REPEATS],
  int          size)
{
  // We want to resend in correct order. Sort RPMs based on age, oldest first.
  for (int i = size - 2; i >= 0; i--)
  {
    for(int j = 0; j <= i; j++)
    {
      if (list[j]->age < list[j+1]->age)
      {
        ref_meta_t * t = list[j];
        list[j] = list[j+1];
        list[j+1] = t;
      }
    }
  }
}

/**
 * Creates a list of dec_ref_pic_marking_repetition based on flux information.
 * ACK_STATUS is an array where the index is the long term index for a frame 
 * in the reference buffer. If ACK_STATUS[i] == 0 it means that the long term 
 * frame with long term frame index i has unknown status and should be 
 * repeated. Returns the number of repetitions. RPM_LIST is an array of rpms and
 * the index is the long term frame index this the rpm generates/modifies.
 *
 * NOTE: It reads the dpb->ref_list so this should be updated before calling
 * this function. */
int taa_h264_ref_pic_marking_repetition (
  decoded_picture_buffer_t * dpb,
  int *                      ack_status,
  ref_meta_t *               rpm_list[MAX_NUM_REF_FRAMES],
  ref_meta_t *               repeat_list[MAX_RPM_REPEATS])
{
  // Repeat the ref pic marking for all frames that has not been ACK/NACK'ed
  int repeats = 0;

  // Find all long term frames with unknown status and their corresponding rpm
  for (int i = 0; i < dpb->total_ref_frames; i++)
  {
    decoded_picture_t * pic = dpb->ref_list[i];
    if (pic->is_longterm && ack_status[pic->picid] == 0)
    {
      /* Only repeat the RPM if there exist one for this long term frame. It 
         might be that the RPM is postponed to a later frame and has not been 
         generated/sent yet. */
      if (rpm_list[pic->picid])
        repeat_list[repeats++] = rpm_list[pic->picid];
    }
  }

  sort_rpm_repeats (repeat_list, repeats);
  return repeats;
}

/* Repeat all RPMs that were sent on a long term frame, e.g. "mark current
 * frame as long term" is repeated, but not necessarily "mark short term as
 * long term" (unless the RPM was sent during a LT frame).
 * Returns the number of repetitions. RPM_LIST is an array of rpms and
 * the index is the long term frame index this the rpm generates/modifies. */
int taa_h264_ref_pic_marking_repetition_all_lt_frames (
  decoded_picture_buffer_t * dpb,
  ref_meta_t *               rpm_list[MAX_NUM_REF_FRAMES],
  ref_meta_t *               repeat_list[MAX_RPM_REPEATS])
{
  int repeats = 0;

  for (int i = 0; i < dpb->total_ref_frames; i++)
  {
    decoded_picture_t * pic = dpb->ref_list[i];
    if (pic->is_longterm && rpm_list[pic->picid])
      repeat_list[repeats++] = rpm_list[pic->picid];
  }

  sort_rpm_repeats (repeat_list, repeats);
  return repeats;
}

/**
 * IMPORTANT: When calling this function, make sure that
 * taa_h264_dpb_update has already been called. This is
 * to make sure that dpb->prev_idx contains the correct id.
 */
static void taa_h264_enc_dpb_update_metainfo (
  decoded_picture_buffer_t * dpb,
  uint8_t                    temporal_id)
{
  for (int i = 0; i < dpb->max_ref_frames + 1; i++)
    ((ref_meta_t *) dpb->pictures[i].meta)->age++;

  decoded_picture_t * pic = &dpb->pictures[dpb->output_idx];
  ref_meta_t * meta = (ref_meta_t *) pic->meta;
  meta->age = 0;
  meta->temporal_id = temporal_id;
  meta->long_term_idx = pic->is_longterm ? pic->picid : -1;
}

/* Returns the ref pic marking commands of the current picture */
ref_meta_t * taa_h264_enc_dpb_curr_ref_meta (decoded_picture_buffer_t * dpb)
{
  return dpb->pictures[dpb->curr_idx].meta;
}

void taa_h264_enc_dpb_update (
  decoded_picture_buffer_t * dpb,
  int *                      frame_num,
  uint8_t                    temporal_id)
{
  const bool is_ref = true;
  const rpm_t * rpm = &taa_h264_enc_dpb_curr_ref_meta(dpb)->rpm;
  const bool has_mmco = rpm->mmco[0].op != 0;
  const bool is_idr_long_term = rpm->idr_flag && has_mmco;

  dpb->pictures[dpb->curr_idx].original_frame_num = *frame_num;

  taa_h264_dpb_update_picnums (dpb, *frame_num);

  taa_h264_dpb_update (dpb, frame_num, is_ref, rpm->idr_flag,
                       is_idr_long_term, has_mmco, rpm->mmco);

  taa_h264_enc_dpb_update_metainfo (dpb, temporal_id);
}

/**
 * Generate reference picture list reordering commands that moves
 * reference frame with REF_IDX to position 0.
 *
 * IMPORTANT: The reference frames are not actually reordered, so the
 * new index should only be used for signaling and the old one must
 * still be used when looking up the frame. */
void taa_h264_generate_rplr (
  const decoded_picture_buffer_t * dpb,
  int                              frame_num,
  uint8_t                          ref_idx,
  rplr_t *                         rplr)
{
  decoded_picture_t * ref = dpb->ref_list[ref_idx];
  if (ref->is_longterm)
  {
    rplr[0].idc = 2;
    rplr[0].val = ref->picid;
  }
  else
  {
    int diff = frame_num - ref->picid;
    if (diff <= 0)
      diff += (1 << LOG2_MAX_FRAME_NUM);
    rplr[0].idc = 0;
    rplr[0].val = diff - 1;
  }
  rplr[1].idc = 3;
}

/* Generates a command for expanding the long term buffer if needed. Returns 
 * the number of MMCO commands generated. */
static int mmco_expand_lt_buffer_if_needed (
  decoded_picture_buffer_t * dpb,
  uint8_t                    ltfi,
  mmco_t *                   mmco)
{
  int i = 0;
  const bool expand_lt_buffer = ltfi >= dpb->max_lt_frames;
  const bool buffer_is_full = dpb->max_ref_frames == dpb->total_ref_frames;

  if (expand_lt_buffer && buffer_is_full)
  {
    // Remove a short term to make room for the new long term index
    decoded_picture_t * pic = taa_h264_dpb_find_oldest_short_term (dpb);
    if (pic)
    {
      mmco[i].op = 1;
      mmco[i].val1 = ((ref_meta_t *) pic->meta)->age;
      i++;
    }
  }

  if (expand_lt_buffer)
  {
    mmco[i].op = 4;
    mmco[i].val1 = ltfi + 1;
    i++;
  }

  return i;
}

/** Generate memory management control operation commands in order to mark a
  * frame as long term frame with index LTFI. If CURRENT is true the current 
  * frame is marked as long term, if false the previous will be marked as long 
  * term. Returns the resulting mmco commands in MMCO. */
void taa_h264_generate_mmco_curr_to_long (
  decoded_picture_buffer_t * dpb,
  uint8_t                    ltfi,
  mmco_t *                   mmco)
{
  int i = mmco_expand_lt_buffer_if_needed (dpb, ltfi, mmco);

  mmco[i].op = 6;
  mmco[i].val1 = ltfi;
  i++;

  mmco[i].op = 0;
}
