#include "dec_dpb.h"

static void long_term_is_unusable (decoded_picture_t * pic);

bool taa_h264_dec_new_dpb_needed (
  const int dpb_maxw, 
  const int dpb_maxh, 
  const int dpb_refframes,
  const int frameinfo_w, 
  const int frameinfo_h,
  const int frameinfo_refframes)
{
  if
  (
       (frameinfo_w > dpb_maxw)
    || (frameinfo_h > dpb_maxh)
    || (dpb_maxw <= 0)
    || (dpb_maxh <= 0)
    || (dpb_refframes != frameinfo_refframes)
    || (frameinfo_w * frameinfo_h > dpb_maxw * dpb_maxh)
    || (taa_h264_dec_dpb_size(frameinfo_w, frameinfo_h, frameinfo_refframes)
          > taa_h264_dec_dpb_size(dpb_maxw, dpb_maxh, dpb_refframes))
  )
    return true;
  return false;
}

int taa_h264_dec_dpb_size (
  const int width, 
  const int height, 
  const int num_ref_frames)
{
  int padded_width  = width  + PAD_HORZ_Y  * 2;
  int padded_height = height + PAD_VERT_Y * 2;
  int buf_size_one  = (padded_width * padded_height * 3 / 2) * sizeof (uint8_t);
  return buf_size_one * (num_ref_frames + 1);
}

/* Initializes the DPB meta information. Setting up pointers and sets default
 * values. */
void taa_h264_dec_dpb_init_metainfo (
  decoded_picture_buffer_t * dpb,
  ref_meta_t *               refmeta)
{
  for (int i = 0; i < dpb->max_ref_frames + 1; i++)
  {
    dpb->pictures[i].meta = &refmeta[i];
    refmeta[i].in_sync = false;
    refmeta[i].usable_ref = false;
  }
}

/* Updates the meta information of current frame. */
bool taa_h264_dec_dpb_update_metainfo (
  decoded_picture_buffer_t * dpb,
  int                        frame_num,
  bool                       all_mbs_decoded,
  bool                       ref_is_in_sync)
{
  ref_meta_t * meta = (ref_meta_t *) dpb->pictures[dpb->curr_idx].meta;
  meta->in_sync = all_mbs_decoded && ref_is_in_sync;
  meta->original_frame_num = frame_num;
  // TODO: Filled frames due to gap should not be usable as reference
  meta->usable_ref = true;
  return meta->in_sync;
}

/**
 * Fills a copy of the structure suitable for concealment source into PREVIOUS.
 * Returns FALSE if no such picture exists.
 */
bool taa_h264_dpb_get_conceal_src (
  decoded_picture_buffer_t * dpb,
  framebuf_t *               src)
{
  if (dpb->conceal_idx == TAA_H264_DPB_NO_MATCH)
    return false;

  *src = dpb->pictures[dpb->conceal_idx].ipbuf.full;
  return true;
}


/* Returns true if there exists a reference frame at INDEX */
bool taa_h264_dec_dpb_ref_exist (
  const decoded_picture_buffer_t * dpb,
  unsigned                         index)
{
  return (index < dpb->total_ref_frames) &&
         dpb->ref_list[index]->is_used &&
         ((ref_meta_t*)dpb->ref_list[index]->meta)->usable_ref;
}


void taa_h264_dec_dpb_fill_gap_in_frame_num (
  decoded_picture_buffer_t * dpb,
  int                        prev_frame_num,
  int                        curr_frame_num)
{
  int fill_frame_num = prev_frame_num + 1;
  if (fill_frame_num >= dpb->max_frame_num)
    fill_frame_num -= dpb->max_frame_num;

  const bool is_ref = true;
  const bool is_idr = false;
  const bool is_idr_long_term = false;
  const bool adaptive_marking_flag = false;

  while (fill_frame_num != curr_frame_num)
  {
    // The fill frames should not be used as a reference. If they are it means
    // we have lost frames we should not have.
    decoded_picture_t * pic = &dpb->pictures[dpb->curr_idx];
    ref_meta_t * meta = (ref_meta_t *) pic->meta;
    meta->in_sync = false;
    meta->usable_ref = false;

    taa_h264_dpb_update_picnums (dpb, fill_frame_num);
    taa_h264_dpb_update (dpb, &fill_frame_num, is_ref, is_idr,
                         is_idr_long_term, adaptive_marking_flag, NULL);

    fill_frame_num++;
    if (fill_frame_num >= dpb->max_frame_num)
      fill_frame_num -= dpb->max_frame_num;
  }
}


/* Returns true if reference frame with INDEX is in sync */
bool taa_h264_dec_dpb_ref_in_sync (
  const decoded_picture_buffer_t * dpb,
  unsigned                         index)
{
  if (index < dpb->total_ref_frames)
  {
    ref_meta_t * meta = (ref_meta_t *) dpb->ref_list[index]->meta;
    return meta->in_sync;
  }
  return false;
}


void taa_h264_dec_dpb_set_all_short_term_out_of_sync (
  decoded_picture_buffer_t * dpb)
{
  decoded_picture_t * picbuf = dpb->pictures;
  for (int i = 0; i < dpb->max_ref_frames + 1; i++)
  {
    if (picbuf[i].is_used && !picbuf[i].is_longterm)
    {
      ref_meta_t * meta = (ref_meta_t *) picbuf[i].meta;
      meta->in_sync = false;
    }
  }
}


/* If an IDR frame exist in DPB, mark it as unusable for reference. */
void taa_h264_dec_dpb_set_idr_as_unusable_reference (
  decoded_picture_buffer_t * dpb)
{
  // FIXME: An IDR does not need to be at position 0. Make an IDR flag in meta.
  // printf ("-- checking for IDRs in ref buf --\n");
  decoded_picture_t * pic = taa_h264_dpb_find_picture (dpb, 0, true);
  if (pic && ((ref_meta_t *)pic->meta)->original_frame_num == 0)
  {
    // printf ("-- setting existing IDR as unusable --\n");
    long_term_is_unusable(pic);
  }
}

/* A LT picture with this index exists. From this point this picture
 * considered wrong. Mark it as out of sync. We shouldn't mark it as
 * unused since the reference list would then be out of sync with regards
 * to the number of ST and LT frames. However, we should never use it as
 * reference since using a significantly different reference frame than
 * intended has serious visual effects. */
static void long_term_is_unusable (
  decoded_picture_t * pic)
{
  ref_meta_t * meta = (ref_meta_t *) pic->meta;
  meta->in_sync = false;
  meta->usable_ref = false;
}

#pragma warning(disable : 869)

/* No previous picture with this LTFI exists. Ideally we should insert a fake
 * long term picture in order to keep the reference buffer in sync. */
static void long_term_is_missing (
  decoded_picture_buffer_t * dpb,
  int                        lt_idx)
{
  // TODO: Implement fake long term pictures
}


static void long_term_is_unusable_if_exists (
  decoded_picture_buffer_t * dpb,
  int                        lt_idx)
{
  decoded_picture_t * pic = taa_h264_dpb_find_picture (dpb, lt_idx, true);
  if (pic)
    long_term_is_unusable (pic);
  else
    long_term_is_missing (dpb, lt_idx);
}


/* This function handles MMCO repetions as used in Flux, not on general basis.
 * If another encoder sends RPM repetition the behavior is unknown. */
static void ref_pic_marking_repetition_mmco (
  decoded_picture_buffer_t * dpb,
  const mmco_t *             mmco,
  int                        original_frame_num,
  callbacks_t *              cb)
{
  int i = 0;

  while (mmco[i].op != 0)
  {
    switch (mmco[i].op)
    {
    case 1:
    {
      /* Mark short-term as unused for reference. */
      int picnum = original_frame_num - (mmco[i].val1 + 1);
      if (original_frame_num > dpb->last_seen_frame_num)
        picnum -= dpb->max_frame_num;
      taa_h264_dpb_set_short_term_unused (dpb, picnum);
      break;
    }
    case 2:
    {
      /* Mark long-term as unused for reference. Assumes that no MMCOs later
       * than original_frame_num has used this LTFI. */
      int lt_idx = mmco[i].val1;
      taa_h264_dpb_set_long_term_unused (dpb, lt_idx);
      break;
    }
    case 3:
    {
      /* Mark short-term as long-term */
      decoded_picture_t * pic;
      bool ack = true;
      int ack_frame_num;
      int picnum = original_frame_num - (mmco[i].val1 + 1);
      int lt_idx = mmco[i].val2;

      if (original_frame_num > dpb->last_seen_frame_num)
        picnum -= dpb->max_frame_num;
      pic = taa_h264_dpb_find_picture (dpb, picnum, false);
      if (pic)
      {
        // Short term picture exists. Mark it as long term and ACK/NACK.
        taa_h264_dpb_set_long_term_unused (dpb, lt_idx);
        taa_h264_dpb_set_long_term (dpb, pic, lt_idx);

        ref_meta_t * meta = (ref_meta_t *) pic->meta;
        ack_frame_num = meta->original_frame_num;
        ack = meta->in_sync;
      }
      else
      {
        // Short term picture does not exists.
        // Ack the frame_num of the missing frame. We might have wrapping.
        if (picnum < 0)
          picnum += dpb->max_frame_num;
        ack_frame_num = picnum;
        ack = false;
        long_term_is_unusable_if_exists (dpb, lt_idx);
      }
      if (cb->reference_ack)
        cb->reference_ack (cb->context, ack_frame_num, ack, false);
      break;
    }
    case 4:
    {
      /* Set the max number of long-term frames. Only expand the buffer if
       * necessary. If MMCO says "shrink it", don't do it because we might
       * have processed later MMCOs which expanded the buffer and we don't want
       * to mark those LT frames as unused. */
      int new_max = mmco[i].val1;
      if (new_max > dpb->max_lt_frames)
        dpb->max_lt_frames = (uint8_t) new_max;

      break;
    }
    case 5:
    {
      /* We should mark all as unused and set max long term frames to zero,
       * but do we want to in this case? */
      break;
    }
    case 6:
    {
      /* Mark picture with original_frame_num as long term. */
      bool ack = true;
      int lt_idx = mmco[i].val1;
      decoded_picture_t * pic = taa_h264_dpb_find_picture (dpb, lt_idx, true);
      if (pic)
      {
        ref_meta_t * meta = (ref_meta_t *) pic->meta;
        if (original_frame_num != meta->original_frame_num || !meta->in_sync)
        {
          long_term_is_unusable (pic);
          ack = false;
        }
      }
      else
      {
        long_term_is_missing (dpb, lt_idx);
        ack = false;
      }

      if (cb->reference_ack)
        cb->reference_ack (cb->context, original_frame_num, ack, false);

      break;
    }
    default:
    {
      TAA_H264_WARNING (
        dpb->error_handler, TAA_H264_DPB_INVALID_MMCO);
      TAA_H264_DEBUG_ASSERT (false);
    }
    }
    i++;
  }
}

/**
 * Handle a dec_ref_pic_marking_repetition. If we discover something that's
 * missing, mark the affected pictures as out of sync, unusable .*/
void taa_h264_dec_dpb_rpm_repetition (
  decoded_picture_buffer_t * dpb,
  int                        original_frame_num,
  bool                       original_idr_flag,
  bool                       idr_is_long_term,
  bool                       adaptive_marking_flag,
  const mmco_t *             mmco,
  callbacks_t *              cb)
{
  if (original_idr_flag)
  {
    if (idr_is_long_term)
    {
      /* Find any picture with LTFI 0 and check if it's the expected picture. */
      decoded_picture_t * pic = taa_h264_dpb_find_picture (dpb, 0, true);
      bool ack = true;
      if (pic)
      {
        ref_meta_t * meta = (ref_meta_t *) pic->meta;
        if (original_frame_num != meta->original_frame_num || !meta->in_sync)
        {
          long_term_is_unusable (pic);
          ack = false;
        }
      }
      else
      {
        long_term_is_missing (dpb, 0);
        ack = false;
      }
      // printf ("-- rpm repetition for IDR, ack=%d --\n", ack);
      if (cb->reference_ack)
        cb->reference_ack (cb->context, original_frame_num, ack, true);
    }
  }
  else if (adaptive_marking_flag)
  {
    ref_pic_marking_repetition_mmco (dpb, mmco, original_frame_num, cb);
  }
}


/* Goes through the reference picture marking information for current frame 
 * and calls ACK or NACK for the long term frames. */
void taa_h264_dec_dpb_rpm_ack (
  decoded_picture_buffer_t * dpb,
  unsigned                   frame_num,
  bool                       is_ref,
  bool                       is_idr,
  bool                       is_idr_long_term,
  bool                       adaptive_marking_flag,
  const mmco_t *             mmco,
  callbacks_t *              cb)
{
  if (!cb->reference_ack)
    return;

  if (is_idr && is_idr_long_term)
  {
    ref_meta_t * meta = dpb->pictures[dpb->curr_idx].meta;
    cb->reference_ack (cb->context, frame_num, meta->in_sync, true);
  }
  else if (is_ref && adaptive_marking_flag)
  {
    int i = 0;
    while (mmco[i].op != 0)
    {
      switch (mmco[i].op)
      {
      case 3:
      {
        // FIXME: Check if short term frame is idr and set in callback
        // Mark short-term as long-term
        // If the referred short-term exist and is in sync, send ACK else NACK
        decoded_picture_t * pic;
        int picnum = frame_num - (mmco[i].val1 + 1);
        pic = taa_h264_dpb_find_picture (dpb, picnum, false);
        // Handle wrapping before sending ACK/NACK
        if (picnum < 0)
          picnum += dpb->max_frame_num;
        if (pic)
        {
          ref_meta_t * meta = pic->meta;
          cb->reference_ack (cb->context, picnum, meta->in_sync, false); 
        }
        else
        {
          cb->reference_ack (cb->context, picnum, false, false);
        }

        break;
      }
      case 6:
      {
        // Mark current picture as long term
        // Current picture obviously exists, but we need to check if it's in sync
        ref_meta_t * meta = dpb->pictures[dpb->curr_idx].meta;
        cb->reference_ack (cb->context, frame_num, meta->in_sync, false);
        break;
      }
      default:
        // No ACK/NACK for the other operations as they don't create long term frames
        break;
      }
      i++;
    }
  }
}


#if REF_FRAME_CHECKSUM

void taa_h264_dpb_assert_checksum (
  decoded_picture_buffer_t * dpb,
  uint32_t checksum,
  int long_term_frame_idx,
  int original_frame_num)
{
  decoded_picture_t * pic = taa_h264_dpb_find_picture (dpb, long_term_frame_idx, true);
  ref_meta_t * meta = (ref_meta_t *) pic->meta;

  if (pic->checksum != checksum || original_frame_num != meta->original_frame_num)
  {
    // Minor trick since assert() is no-op in release
    __asm {int 3};
  }
}
#endif
