#include "com_dpb.h"
#include "com_intmath.h"
#include "com_compatibility.h"

#include <limits.h>
#include <string.h>
#include <stdio.h>

void taa_h264_unpad_framebuf (
  framebuf_t * buf)
{
  int unpad_offset = PAD_VERT_Y * buf->stride + PAD_HORZ_Y;
  buf->y = &buf->y[unpad_offset];
  buf->rows = buf->rows - 2 * PAD_VERT_Y;
  buf->cols = buf->cols - 2 * PAD_HORZ_Y;

  unpad_offset = PAD_VERT_UV * buf->cstride + PAD_HORZ_UV;
  buf->u = &buf->u[unpad_offset];
  buf->v = &buf->v[unpad_offset];
  buf->crows = buf->crows - 2 * PAD_VERT_UV;
  buf->ccols = buf->ccols - 2 * PAD_HORZ_UV;
}

static int taa_h264_init_framebuf (
  framebuf_t * buf,
  int          cols,
  int          rows,
  uint8_t *    mem)
{
  int size_y = cols * rows;
  int size_cr = size_y / 4;

  /* The buffers don't own the memory */
  buf->data = NULL;

  buf->y = mem;
  buf->u = buf->y + size_y;
  buf->v = buf->u + size_cr;

  buf->cols = cols;
  buf->rows = rows;
  buf->ccols = cols / 2;
  buf->crows = rows / 2;
  buf->stride = buf->cols;
  buf->cstride = buf->ccols;

  return size_y + 2 * size_cr;
}


/**
 * Initializes the decoded picture buffer. WIDTH and HEIGHT gives the
 * dimensions of the buffers in the storage. DPB_HAS_INTERPOLATED is
 * true if the dpb also contains interpolated versions of each frame.
 * */
bool taa_h264_dpb_init (
  decoded_picture_buffer_t * dpb,
  int                        max_ref_frames,
  int                        width,
  int                        height,
  int                        max_frame_num,
  bool                       dpb_has_interpolated,
  error_handler_t *          error_handler)
{
  TAA_H264_DEBUG_ASSERT (max_ref_frames < MAX_NUM_DPB_PICS);
  max_ref_frames = min (max_ref_frames, MAX_NUM_DPB_PICS - 1);

  if (width * height > dpb->max_width * dpb->max_height)
  {
    TAA_H264_ERROR (error_handler, TAA_H264_UNSUPPORTED_RESOLUTION);
    return false;
  }

  const int num_pics       = max_ref_frames + 1;
  const int padded_width   = width  + PAD_HORZ_Y  * 2;
  const int padded_height  = height + PAD_VERT_Y * 2;

  uint8_t * memptr = dpb->mem_base;

  for (int i = 0; i < num_pics; i++)
  {
    decoded_picture_t * pic = &dpb->pictures[i];
    frameipbuf_t * ipbuf = &pic->ipbuf;

    pic->is_used = false;
    pic->is_longterm = false;
    pic->picid = 0;

    memptr += taa_h264_init_framebuf (&ipbuf->full,
                                      padded_width, padded_height,
                                      memptr);

    if (dpb_has_interpolated)
    {
      memptr += taa_h264_init_framebuf (&ipbuf->half_horz,
                                        padded_width, padded_height,
                                        memptr);
      memptr += taa_h264_init_framebuf (&ipbuf->half_vert,
                                        padded_width, padded_height,
                                        memptr);
      memptr += taa_h264_init_framebuf (&ipbuf->half_diag,
                                        padded_width, padded_height,
                                        memptr);
    }
    else
    {
      taa_h264_init_framebuf (&ipbuf->half_horz, 0, 0, NULL);
      taa_h264_init_framebuf (&ipbuf->half_vert, 0, 0, NULL);
      taa_h264_init_framebuf (&ipbuf->half_diag, 0, 0, NULL);
    }
  }

  dpb->max_ref_frames = (uint8_t) max_ref_frames;
  dpb->total_ref_frames = 0;
  dpb->curr_idx = 0;
  dpb->output_idx = TAA_H264_DPB_NO_MATCH;
  dpb->conceal_idx = TAA_H264_DPB_NO_MATCH;
  dpb->max_lt_frames = 0;
  dpb->num_lt_frames = 0;

  dpb->max_frame_num = max_frame_num;
  dpb->last_seen_frame_num = 0;

  dpb->error_handler = error_handler;

  return true;
}

void taa_h264_dpb_reset (
  decoded_picture_buffer_t * dpb)
{
  for (int i = 0; i < MAX_NUM_DPB_PICS; i++)
  {
    decoded_picture_t * pic = &dpb->pictures[i];
    pic->is_used = false;
    pic->is_longterm = false;
    pic->picid = 0;
  }

  dpb->total_ref_frames = 0;
  dpb->max_lt_frames = 0;
  dpb->num_lt_frames = 0;
}

/**
 * Returns the first unused position in DPB. */
static int taa_h264_dpb_find_unused (
  decoded_picture_buffer_t * dpb)
{
  int idx = TAA_H264_DPB_NO_MATCH;
  const int max_frames = dpb->max_ref_frames + 1;

  for (int i = 0; i < max_frames; i++)
  {
    decoded_picture_t * pic = &dpb->pictures[i];
    if (pic->is_used == false)
    {
      idx = i;
      break;
    }
  }

  return idx;
}


decoded_picture_t * taa_h264_dpb_find_picture (
  decoded_picture_buffer_t * dpb,
  int                        picid,
  bool                       longterm)
{
  const int max_frames = dpb->max_ref_frames + 1;

  for (int i = 0; i < max_frames; i++)
  {
    decoded_picture_t * pic = &dpb->pictures[i];
    if (pic->is_used == true
        && pic->picid == picid
        && pic->is_longterm == longterm)
    {
      return pic;
    }
  }

  return NULL;
}

static void taa_h264_dpb_set_picture_unused (
  decoded_picture_buffer_t * dpb,
  decoded_picture_t *        pic)
{
  pic->is_used = false;
  dpb->total_ref_frames--;
  if (pic->is_longterm)
  {
    pic->is_longterm = false;
    dpb->num_lt_frames--;
  }
}


/* Returns the index of the oldest short therm picture or TAA_H264_DPB_NO_MATCH
 * if none is found. */
static int taa_h264_dpb_find_oldest_short_term_ (
  decoded_picture_buffer_t * dpb)
{
  int oldest_idx = TAA_H264_DPB_NO_MATCH;
  int oldest_picnum = INT_MAX;
  const int max_frames = dpb->max_ref_frames + 1;

  for (int i = 0; i < max_frames; i++)
  {
    decoded_picture_t * pic = &dpb->pictures[i];
    if (pic->is_used == true
        && pic->is_longterm == false
        && pic->picid < oldest_picnum)
    {
      oldest_idx = i;
      oldest_picnum = pic->picid;
    }
  }
  return oldest_idx;
}

decoded_picture_t * taa_h264_dpb_find_oldest_short_term (
  decoded_picture_buffer_t * dpb)
{
  int idx = taa_h264_dpb_find_oldest_short_term_ (dpb);
  if (idx != TAA_H264_DPB_NO_MATCH)
    return &dpb->pictures[idx];
  else
    return NULL;
}

/**
 * Find the oldest picture and set it as unused. Returns the index of
 * the oldest picture that was found. */
static int taa_h264_dpb_remove_oldest_short_term (
  decoded_picture_buffer_t * dpb)
{
  int idx = taa_h264_dpb_find_oldest_short_term_(dpb);
  if (idx != TAA_H264_DPB_NO_MATCH)
    taa_h264_dpb_set_picture_unused (dpb, &dpb->pictures[idx]);
  return idx;
}


/**
 * Mark short term reference picture mathcing PICNUM as unused.
 * Returns true if a mathcing picture was found and removed. */
bool taa_h264_dpb_set_short_term_unused (
  decoded_picture_buffer_t * dpb,
  int                        picnum)
{
  const int longterm = false;
  decoded_picture_t * pic = taa_h264_dpb_find_picture (dpb, picnum, longterm);
  bool ret = true;

  if (pic)
    taa_h264_dpb_set_picture_unused (dpb, pic);
  else
    ret = false;

  return ret;
}


/**
 * Mark long term reference picture mathcing PICNUM as unused.
 * Returns true if a mathcing picture was found and removed. */
bool taa_h264_dpb_set_long_term_unused (
  decoded_picture_buffer_t * dpb,
  int                        lt_index)
{
  const int longterm = true;
  decoded_picture_t * pic = taa_h264_dpb_find_picture (dpb, lt_index, longterm);
  bool ret = true;
  if (pic)
    taa_h264_dpb_set_picture_unused (dpb, pic);
  else
    ret = false;

  return ret;
}

/**
 * Set picture as short term, assuming it was not used before */
static void taa_h264_dpb_set_short_term (
  decoded_picture_buffer_t * dpb,
  decoded_picture_t *        pic,
  int                        picnum)
{
  TAA_H264_DEBUG_ASSERT (pic->is_used == false);

  pic->picid = picnum;
  pic->is_used = true;
  pic->is_longterm = false;
  dpb->total_ref_frames++;
}

/**
 * Assign a long term index to a short term reference frame */
void taa_h264_dpb_set_long_term (
  decoded_picture_buffer_t * dpb,
  decoded_picture_t *        pic,
  int                        lt_index)
{
  TAA_H264_DEBUG_ASSERT (dpb->num_lt_frames < dpb->max_lt_frames);
  TAA_H264_DEBUG_ASSERT (lt_index < dpb->max_lt_frames);
  TAA_H264_DEBUG_ASSERT (pic->is_used == true && pic->is_longterm == false);

  /* If we discover that the DPB is in some strange state, we try to guess how
   * it should be and do our best and hope the we're lucky enough that the
   * encoder don't use whatever we're missing (assume Flux behavior on
   * encoder). */
  if (lt_index >= dpb->max_lt_frames)
  {
    /* We probably missed a command for extending the LT buffer */
    dpb->max_lt_frames = (uint8_t) (lt_index + 1);
  }

  pic->is_longterm = true;
  pic->picid = lt_index;
  dpb->num_lt_frames++;
}


/**
 * Mark all the long term frames that has LTFI above the new max index
 * as unused. */
static void taa_h264_dpb_shrink_long_term_buffer (
  decoded_picture_buffer_t * dpb,
  int                        new_max)
{
  const int max_frames = dpb->max_ref_frames + 1;
  for (int i = 0; i < max_frames; i++)
  {
    decoded_picture_t * pic = &dpb->pictures[i];
    if (pic->is_used == true
        && pic->is_longterm == true
        && pic->picid >= new_max)
    {
      taa_h264_dpb_set_picture_unused (dpb, pic);
    }
  }
  dpb->max_lt_frames = (uint8_t) new_max;
}



static int taa_h264_dpb_sliding_window (
  decoded_picture_buffer_t * dpb)
{
  int new_idx;

  if (dpb->total_ref_frames > dpb->max_ref_frames)
    new_idx = taa_h264_dpb_remove_oldest_short_term (dpb);
  else
    new_idx = taa_h264_dpb_find_unused (dpb);

  if (new_idx == TAA_H264_DPB_NO_MATCH)
  {
    TAA_H264_WARNING (dpb->error_handler, TAA_H264_DPB_INTERNAL_ERROR);
    new_idx = 0;
  }

  return new_idx;
}

static bool taa_h264_dpb_adaptive_marking (
  decoded_picture_buffer_t * dpb,
  const mmco_t *             mmco,
  int                        curr_picnum)
{
  int i = 0;
  bool has_mmco5 = false;

  while (mmco[i].op != 0)
  {
    switch (mmco[i].op)
    {
    case 1:
    {
      /* Mark short-term as unused for reference */
      int picnum = curr_picnum - (mmco[i].val1 + 1);
      if (!taa_h264_dpb_set_short_term_unused (dpb, picnum))
      {
        TAA_H264_WARNING (
          dpb->error_handler, TAA_H264_DPB_INVALID_SHORT_TERM_PICNUM);
      }
      break;
    }
    case 2:
    {
      /* Mark long-term as unused for reference */
      int lt_idx = mmco[i].val1;
      if (!taa_h264_dpb_set_long_term_unused (dpb, lt_idx))
      {
        TAA_H264_WARNING (dpb->error_handler,
                          TAA_H264_DPB_INVALID_LONG_TERM_FRAME_INDEX);
      }

      break;
    }
    case 3:
    {
      /* Mark short-term as long-term */
      decoded_picture_t * pic;
      int picnum = curr_picnum - (mmco[i].val1 + 1);
      int lt_idx = mmco[i].val2;

      taa_h264_dpb_set_long_term_unused (dpb, lt_idx);
      pic = taa_h264_dpb_find_picture (dpb, picnum, false);
      if (pic)
        taa_h264_dpb_set_long_term (dpb, pic, lt_idx);
      else
        TAA_H264_WARNING (
          dpb->error_handler, TAA_H264_DPB_INVALID_SHORT_TERM_PICNUM);

      break;
    }
    case 4:
    {
      /* Set the max number of long-term frames and set as
       * unused those that have higher index */
      int new_max = mmco[i].val1;

      if (new_max < dpb->max_lt_frames)
        taa_h264_dpb_shrink_long_term_buffer (dpb, new_max);
      else
        dpb->max_lt_frames = (uint8_t) new_max;

      break;
    }
    case 5:
    {
      /* Mark all as unused and set max long term frames to zero */
      decoded_picture_t * pic = &dpb->pictures[dpb->curr_idx];
      taa_h264_dpb_reset (dpb);
      taa_h264_dpb_set_short_term (dpb, pic, 0);
      has_mmco5 = true;
      break;
    }
    case 6:
    {
      /* Mark current picture as long term */
      /* Mark short-term as long-term */
      decoded_picture_t * pic = &dpb->pictures[dpb->curr_idx];
      int lt_idx = mmco[i].val1;

      taa_h264_dpb_set_long_term_unused (dpb, lt_idx);
      taa_h264_dpb_set_long_term (dpb, pic, lt_idx);
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

  return has_mmco5;
}

/**
 *  Update the DPB after a frame has been decoded. Prepares the buffer
 *  for the next frame to come. */
bool taa_h264_dpb_update (
  decoded_picture_buffer_t * dpb,
  int *                      frame_num,
  bool                       is_ref,
  bool                       is_idr,
  bool                       is_idr_long_term,
  bool                       adaptive_marking_flag,
  const mmco_t *             mmco)
{
  decoded_picture_t * curr_pic = &dpb->pictures[dpb->curr_idx];
  int new_idx;
  bool success = true;

  if (is_idr)
  {
    taa_h264_dpb_reset (dpb);
    taa_h264_dpb_set_short_term (dpb, curr_pic, *frame_num);

    if (is_idr_long_term)
    {
      dpb->max_lt_frames = 1;
      taa_h264_dpb_set_long_term (dpb, curr_pic, 0);
    }

    new_idx = taa_h264_dpb_find_unused (dpb);
    dpb->conceal_idx = dpb->curr_idx;
  }
  else if (is_ref)
  {
    taa_h264_dpb_set_short_term (dpb, curr_pic, *frame_num);

    if (adaptive_marking_flag)
    {
      bool has_mmco5 = taa_h264_dpb_adaptive_marking (dpb, mmco, curr_pic->picid);
      if (has_mmco5)
        *frame_num = 0;
    }

    new_idx = taa_h264_dpb_sliding_window (dpb);
    dpb->conceal_idx = dpb->curr_idx;
  }
  else
  {
    /* Disposable frame: Set the index of next frame to the same as the current
     * frame. This assumes that the frame is output before anything from the
     * next frame is decoded (and overwrites this memory). Works for now. */

    /* Don't update conceal_idx since the current frame is disposable and won't
     * be available for concealment on the next frame */

    curr_pic->picid = *frame_num;
    curr_pic->is_used = false;
    curr_pic->is_longterm = false;

    new_idx = dpb->curr_idx;
 }

  /* Update index on where to write the next frame */
  dpb->last_seen_frame_num = *frame_num;
  dpb->output_idx = dpb->curr_idx;

  TAA_H264_DEBUG_ASSERT (new_idx != TAA_H264_DPB_NO_MATCH);
  if (new_idx != TAA_H264_DPB_NO_MATCH)
  {
    dpb->curr_idx = (uint8_t) new_idx;
  }
  else
  {
    /* Something went wrong. We need to report the error and set the DPB to a
     * non-crashing state */
    dpb->curr_idx = 0;
    TAA_H264_ERROR (dpb->error_handler, TAA_H264_DPB_INTERNAL_ERROR);
    success = false;
  }

  return success;
}


/**
 * Updates the picnums of the pictures in the DPB in order to
 * compensate for frame_num wrapping. */
void taa_h264_dpb_update_picnums (
  decoded_picture_buffer_t * dpb,
  int                        frame_num)
{
  const int max_frames = dpb->max_ref_frames + 1;
  for (int i = 0; i < max_frames; i++)
  {
    if (dpb->pictures[i].is_used
        && dpb->pictures[i].is_longterm == false
        && dpb->pictures[i].picid > frame_num)
    {
      dpb->pictures[i].picid -= dpb->max_frame_num;
    }
  }
}


/**
 * Sorts the list of decoded pictures as specififed in 8.2.4.2. That
 * is, decreasing PicNum order for short-term frames and increasing of
 * long-term frames. Long-term frames are put at the end of the
 * ref_list. */
static void taa_h264_dpb_default_ref_list (
  decoded_picture_buffer_t * dpb)
{
  decoded_picture_t * picbuf = dpb->pictures;
  decoded_picture_t * * reflist = dpb->ref_list;

  memset (reflist, 0x0, MAX_NUM_DPB_PICS * sizeof (reflist[0]));

  int refpos = 0;
  const int max_frames = dpb->max_ref_frames + 1;
  for (int i = 0; i < max_frames; i++)
  {
    if (picbuf[i].is_used && !picbuf[i].is_longterm)
    {
      int k = refpos;
      reflist[refpos++] = &picbuf[i];
      while (k > 0 && reflist[k - 1]->picid < reflist[k]->picid)
      {
        decoded_picture_t * tmp = reflist[k];
        reflist[k] = reflist[k - 1];
        reflist[k - 1] = tmp;
        k--;
      }
    }
  }
  TAA_H264_DEBUG_ASSERT (refpos == dpb->total_ref_frames - dpb->num_lt_frames);

  int num_st = refpos;
  for (int i = 0; i < dpb->max_ref_frames + 1; i++)
  {
    if (picbuf[i].is_used && picbuf[i].is_longterm)
    {
      int k = refpos;
      reflist[refpos++] = &picbuf[i];
      while (k > num_st && reflist[k - 1]->picid > reflist[k]->picid)
      {
        decoded_picture_t * tmp = reflist[k];
        reflist[k] = reflist[k - 1];
        reflist[k - 1] = tmp;
        k--;
      }
    }
  }

  TAA_H264_DEBUG_ASSERT (refpos == dpb->total_ref_frames);

#ifdef TAA_H264_TRACE_ENABLED
  TAA_H264_TRACE (TAA_H264_TRACE_DPB, "dpb_default_list:\n");
  for (int i = 0; i < dpb->total_ref_frames; i++)
    TAA_H264_TRACE (TAA_H264_TRACE_DPB, "\t [%2d] picid %3d, lt %2d\n",
                    i, reflist[i]->picid, reflist[i]->is_longterm);
#endif
}

/**
 * Reorders the reference list according to given Reference Picture
 * List Reordering commands RPLR. */
static bool taa_h264_dpb_reorder_ref_list (
  decoded_picture_buffer_t * dpb,
  const rplr_t *             rplr,
  int                        frame_num)
{
  /* Error handling: If the reorder command is not valid (probably because
   * we have lost important RPM info), we can't risk using this index as a
   * reference. If insert another reference frame in this position instead,
   * we have no idea how similar this frame is to the missing frame. Using
   * a radicaly different frame as reference is really bad because you can
   * end up with a "flashback frame".
   *
   * We could mark only this frame as unusable as reference and still
   * reconstruct MBs that do not refer to this one. But in our normal
   * scenario there is not point in doing this, so let's keep things simple.
   * Return failure and trust normal error handling/concealment. */

  int refidx = 0;
  int idc = rplr[0].idc;
  int picnum_pred = frame_num;
  decoded_picture_t * * reflist = dpb->ref_list;

  while (idc != 3)
  {
    int i;
    if (idc < 2)
    {
      int absdiff = rplr[refidx].val + 1;
      int picnum_nowrap, picnum;
      if (idc == 0)
      {
        picnum_nowrap = picnum_pred - absdiff;
        if (picnum_nowrap < 0)
          picnum_nowrap += dpb->max_frame_num;
      }
      else
      {
        picnum_nowrap = picnum_pred + absdiff;
        if (picnum_nowrap >= dpb->max_frame_num)
          picnum_nowrap -= dpb->max_frame_num;

      }

      picnum = picnum_nowrap;
      if (picnum > frame_num)
        picnum -= dpb->max_frame_num;

      picnum_pred = picnum_nowrap;

      /* Picture with "picnum" should be placed at position
       * "refidx" in the reflist. */

      /* Find short term picture with given picnum */
      for (i = refidx; i < dpb->total_ref_frames; i++)
      {
        if (reflist[i]->picid == picnum
            && reflist[i]->is_longterm == false)
          break;
      }

      if (i == dpb->total_ref_frames)
      {
        TAA_H264_WARNING (
          dpb->error_handler, TAA_H264_DPB_INVALID_SHORT_TERM_PICNUM);
        TAA_H264_DEBUG_ASSERT (i == dpb->total_ref_frames);
        return false;
      }
    }
    else
    {
      int ltfi = rplr[refidx].val;

      /* Find long term picture with given long term frame index */
      for (i = refidx; i < dpb->total_ref_frames; i++)
      {
        if (reflist[i]->picid == ltfi
            && reflist[i]->is_longterm == true)
          break;
      }
      if (i == dpb->total_ref_frames)
      {
        TAA_H264_WARNING (
          dpb->error_handler, TAA_H264_DPB_INVALID_LONG_TERM_FRAME_INDEX);
        TAA_H264_DEBUG_ASSERT (false);
        return false;
      }
    }

    /* Shift affected pictures one index later in the list */
    decoded_picture_t * curr_refpic = reflist[i];
    for (; i > refidx; i--)
      reflist[i] = reflist[i - 1];
    reflist[refidx] = curr_refpic;

    refidx++;
    idc = rplr[refidx].idc;
  }

#ifdef TAA_H264_TRACE_ENABLED
  TAA_H264_TRACE (TAA_H264_TRACE_DPB, "dpb_reordered_list:\n");
  for (int i = 0; i < dpb->total_ref_frames; i++)
  {

    if (reflist[i] != NULL)
    {
      TAA_H264_TRACE (TAA_H264_TRACE_DPB, "\t [%2d] picid %3d, lt %2d\n",
                      i, reflist[i]->picid, reflist[i]->is_longterm);
    }
    else
    {
      TAA_H264_DEBUG_ASSERT (reflist[i] != NULL);
    }
  }
#endif

  return true;
}


/**
 * Setup of the reference list according to default ordering and signaled reordering.
 *
 * Returns the total number of available reference frames and the reference
 * frame buffers (the data) in REF_BUFFERS. Returns a negative value if there
 * was an error on reference buffer is not to be trusted. */
int taa_h264_dpb_set_ref_list (
  decoded_picture_buffer_t * dpb,
  bool                       reordering_flag,
  const rplr_t *             rplr,
  int                        frame_num,
  frameipbuf_t * *           ref_buffers,
  void * *                   ref_metas)
{
  bool success = true;
  const int failure_retval = -1;

  // Should never happen unless there is bit corruption
  if (dpb->max_ref_frames < dpb->total_ref_frames)
    return failure_retval;

  taa_h264_dpb_default_ref_list (dpb);
  if (reordering_flag)
    success = taa_h264_dpb_reorder_ref_list (dpb, rplr, frame_num);

  /* According to standard we only need num_active_ref_frames entries
   * in the buffer, but our encoder needs to see all available
   * reference frames in order to take the reference frame decision.
   * This is correct, but slightly more work. */
  for (int i = 0; i < dpb->total_ref_frames; i++)
  {
    ref_buffers[i] = &dpb->ref_list[i]->ipbuf;
    if (ref_metas != NULL)
      ref_metas[i] = dpb->ref_list[i]->meta;
  }

  return success ? dpb->total_ref_frames : failure_retval;
}


/**
 * Fills a copy of the structure for current picture into CURRENT
 */
void taa_h264_dpb_get_current_framebuf (
  decoded_picture_buffer_t * dpb,
  framebuf_t *               current)
{
  *current = dpb->pictures[dpb->curr_idx].ipbuf.full;
}


#if 0
void taa_h264_dump_reference_buffer (
  decoded_picture_buffer_t * dpb,
  FILE *                     file)
{
  for (int k = 0; k < dpb->total_ref_frames; k++)
  {
    framebuf_t buf = dpb->ref_list[k]->ipbuf.full;
    taa_h264_unpad_framebuf (&buf);

    for (int i = 0; i < buf.rows; i++)
      fwrite (&buf.y[i * buf.stride], sizeof(uint8_t), buf.cols, file);

    for (int i = 0; i < buf.crows; i++)
      fwrite (&buf.u[i * buf.cstride], sizeof(uint8_t), buf.ccols, file);

    for (int i = 0; i < buf.crows; i++)
      fwrite (&buf.v[i * buf.cstride], sizeof(uint8_t), buf.ccols, file);
  }
}
#endif

#if REF_FRAME_CHECKSUM

void taa_h264_dpb_set_checksum (
  decoded_picture_buffer_t * dpb)
{
  decoded_picture_t * dp = &dpb->pictures[dpb->curr_idx];
  uint8_t * y = dp->ipbuf.full.y;
  const int width = dp->ipbuf.full.cols - 2 * PAD_HORZ_Y;
  const int height = dp->ipbuf.full.rows - 2 * PAD_VERT_Y;
  const int stride = dp->ipbuf.full.stride;
  uint32_t checksum = 0;

  for (int i = 0; i < height; i++)
    for (int j = 0; j < width; j++)
      checksum += y[i*stride + j];

  dp->checksum = checksum;
}

#endif
