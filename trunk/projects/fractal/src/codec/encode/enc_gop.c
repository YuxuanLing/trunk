#include "enc_gop.h"
#include "com_intmath.h"

#include <limits.h>
#include <stdio.h>


static bool find_vacant_slot (
  ref_meta_t * *  refframes_meta,
  uint8_t         total_ref_frames,
  uint8_t         max_ref_frames,
  ref_control_t * refcontrol);

static bool find_nacked_slot (
  ref_meta_t * *  refframes_meta,
  uint8_t         total_ref_frames,
  ref_control_t * refcontrol,
  const flux_t *  flux);

static bool find_overwriteable_acked_slot (
  ref_meta_t * *  refframes_meta,
  uint8_t         total_ref_frames,
  ref_control_t * refcontrol,
  const flux_t *  flux);

static bool find_overwriteable_unconfirmed_slot (
  ref_meta_t * *  refframes_meta,
  uint8_t         total_ref_frames,
  ref_control_t * refcontrol,
  const flux_t *  flux);

static bool find_outdated_unknown_slot (
  ref_meta_t * *  refframes_meta,
  uint8_t         total_ref_frames,
  ref_control_t * refcontrol,
  const flux_t *  flux);

static bool find_oldest_slot (
  ref_meta_t * *  refframes_meta,
  uint8_t         total_ref_frames,
  ref_control_t * refcontrol);


/**
 * Resets the gop structure and initializes the period for the given
 * levels. */
void taa_h264_reset_gop (
  gop_t * gop,
  uint8_t size,
  uint8_t levels)
{
  /* TODO: Assert that there are enough reference frames */
  uint8_t period = 1;

  gop->levels = (uint8_t) TAA_H264_CLIP (levels, 1, MAX_TEMPORAL_LEVELS);

  if (gop->levels == 1)
    size = 1;

  /* All levels will have a period that is a power of two. The
   * highest level will have a period of 1, then 2, 4, etc.. Except
   * from level 0 which have the same period as the gop size (which
   * normally also is a power of 2). This works well for normal
   * hierarchical prediction structures. */

  int i = gop->levels - 1;
  while (i > 0 && period < size)
  {
    gop->count[i] = period;
    gop->period[i] = period;
    period *= 2;
    i--;
  }

  for (; i >= 0; i--)
  {
    gop->count[i] = size;
    gop->period[i] = size;
  }
}

/**
 * Updates the GOP structure and returns the temporal ID of the
 * current frame. */
uint8_t taa_h264_update_gop (
  gop_t * gop)
{
  int tid = gop->levels - 1;
  for (int i = 0; i < gop->levels; i++)
  {
    gop->count[i]--;
    if (gop->count[i] == 0)
    {
      gop->count[i] = gop->period[i];
      tid = min (i, tid);
    }
  }

  return (uint8_t) tid;
}

/**
 * Checks if GOP matches the given gop size and number of temporal
 * levels.*/
bool taa_h264_gop_is_equal (
  const gop_t * gop,
  uint8_t       size)
{
  return gop->period[0] == size;
}

/**
 * This function implements the hierarchical reference scheme, i.e. it finds the
 * most recent frame of same or lower temporal id. Returns TRUE if a suitable
 * reference frame was found. */
bool taa_h264_hierarchical_reference_control (
  ref_meta_t * *  refframes_meta,
  uint8_t         total_ref_frames,
  uint8_t         temporal_id,
  uint8_t         num_temporal_levels,
  ref_control_t * refcontrol)
{
  int min_age = INT_MAX;
  int ref_idx = -1;

  for (int i = 0; i < total_ref_frames; i++)
  {
    const ref_meta_t * curr = refframes_meta[i];
    if (curr->temporal_id <= temporal_id)
    {
      if (curr->age < min_age)
      {
        ref_idx = i;
        min_age = curr->age;
      }
    }
  }
  refcontrol->ref_idx = ref_idx;

  /* The highest temporal ids never need to be stored as long
   * term frames with this hierarchical reference scheme. For
   * convenience we set all other frames as long term. This
   * might not be necessary for all cases (depending on the
   * number of temporal levels, gop size and number of reference
   * frames), but it's the easiest. */
  if (temporal_id < num_temporal_levels - 1)
    refcontrol->longterm_idx = temporal_id;
  else
    refcontrol->longterm_idx = -1;

  return ref_idx >= 0;
}



bool taa_h264_flux_super_p_control (
  ref_meta_t * *  refframes_meta,
  uint8_t         total_ref_frames,
  uint8_t         max_ref_frames,
  ref_control_t * refcontrol,
  const flux_t *  flux)
{
  bool ret = false;
  int num_acks = 0;

  /* check if we have ANY ack'ed frames, if not return false. */
  for (int k = 0; k<total_ref_frames; k++)
  {
    const ref_meta_t * curr = refframes_meta[k];
    if (curr->long_term_idx >= 0)
    {
      if (flux->ack_status[curr->long_term_idx] == 1)
        num_acks++;
    }
  }

  if (num_acks == 0)
    return false;

  ret = taa_h264_flux_decide_refframes (refframes_meta, total_ref_frames,
                                        max_ref_frames, refcontrol, flux, true);

  //printf ("---- selected SUPER P frame with: lt_idx %d ref_idx %d ----\n \n",
  //  refcontrol->longterm_idx, refcontrol->ref_idx);

  return ret;
}

/**
 * Finds a suitable reference frame and decides if it should be a long term
 * frame based on the Flux scheme. Return value is passed in REFCONTROL.
 **/
bool taa_h264_flux_decide_refframes (
  ref_meta_t * *  refframes_meta,
  uint8_t         total_ref_frames,
  uint8_t         max_ref_frames,
  ref_control_t * refcontrol,
  const flux_t *  flux,
  bool            is_super_p)
{
  int min_age = INT_MAX;

  // first find useful ref_idx:
  // --> just find the youngest frame in buffer.
  // --> if this is a super p frame, make sure the ref frame
  //     is also ACK'ed.

  for (int k = 0; k<total_ref_frames; k++)
  {
    const ref_meta_t * curr = refframes_meta[k];

    if (is_super_p)
    {
      if ((curr->long_term_idx >= 0) &&
          (flux->ack_status[curr->long_term_idx] == 1) &&
          (curr->age < min_age))
      {
        refcontrol->ref_idx = k;
        min_age = curr->age;
      }
    }
    else
    {
      if (curr->age < min_age)
      {
        refcontrol->ref_idx = k;
        min_age = curr->age;
      }
    }
    //printf (" ref_idx=%d   \t lt_idx=%d   \t age=%d   \t ack=%d   \t temp_id=%d \n", k,
    //  curr->long_term_idx, curr->age, flux->ack_status[curr->long_term_idx], curr->temporal_id);
  }

  // next: find the destination longterm_idx.
  // --> check possibilities in prioritized order:

  /* [1] check if there is a vacant slot in the LT buffer first */
  if (find_vacant_slot (refframes_meta, total_ref_frames, max_ref_frames, refcontrol))
    return true;

  /* [2] no vacant slot found , try to overwrite any NACK'ed slots */
  if (find_nacked_slot (refframes_meta, total_ref_frames, refcontrol, flux))
    return true;

  /* [3] no NACKS, see if there are any old (outdated) frames without feedback information */
  if (find_outdated_unknown_slot (refframes_meta, total_ref_frames, refcontrol, flux))
    return true;

  /* [4] no NACKS and none outdated, overwrite old ACKed frame if we have multiple ACKS */
  if (find_overwriteable_acked_slot (refframes_meta, total_ref_frames, refcontrol, flux))
    return true;

  /* [5] no NACKS, none outdated, no old ACKS, look for non-acked (unknown) slots older than an ACKed slot */
  if (find_overwriteable_unconfirmed_slot (refframes_meta, total_ref_frames, refcontrol, flux))
    return true;

  /* [6] no suitable location found, default to overwriting the oldest LT slot in buffer */
  return find_oldest_slot (refframes_meta, total_ref_frames, refcontrol);
}


static bool
find_vacant_slot (
  ref_meta_t * *  refframes_meta,
  uint8_t         total_ref_frames,
  uint8_t         max_ref_frames,
  ref_control_t * refcontrol)
{
  int highest_used_lt_idx = -1;

  for (int k = 0; k<total_ref_frames; k++)
  {
    const ref_meta_t * curr = refframes_meta[k];
    // TODO: if (curr->long_term_idx > highest_used_lt_idx)
    if ((curr->long_term_idx >= 0) && (curr->long_term_idx > highest_used_lt_idx))
      highest_used_lt_idx = curr->long_term_idx;
  }

  if ((highest_used_lt_idx != -1) && (highest_used_lt_idx < (max_ref_frames -2)))
  {
    refcontrol->longterm_idx = (uint8_t)(highest_used_lt_idx + 1);
    return true;
  }
  else
    return false;
}


static bool
find_nacked_slot (
  ref_meta_t * *  refframes_meta,
  uint8_t         total_ref_frames,
  ref_control_t * refcontrol,
  const flux_t *  flux)
{
  int nacked_lt_idx = -1;
  int nacked_lt_idx_age = -1;

  // TODO: Pick the first NACKed frame we see, we don't care about the age
  for (int k = 0; k<total_ref_frames; k++)
  {
    const ref_meta_t * curr = refframes_meta[k];
    if ((curr->long_term_idx >= 0) &&
        (flux->ack_status[curr->long_term_idx] == -1) &&
        (curr->age > nacked_lt_idx_age))
    {
      nacked_lt_idx = curr->long_term_idx;
      nacked_lt_idx_age = curr->age;
    }
  }

  if (nacked_lt_idx != -1)
  {
    refcontrol->longterm_idx = (uint8_t)nacked_lt_idx;
    return true;
  }
  else
    return false;
}

static bool
find_outdated_unknown_slot (
  ref_meta_t * *  refframes_meta,
  uint8_t         total_ref_frames,
  ref_control_t * refcontrol,
  const flux_t *  flux)
{
  int oldest_lt_idx = -1;
  int oldest_lt_age = -1;

  for (int k = 0; k<total_ref_frames; k++)
  {
    const ref_meta_t * curr = refframes_meta[k];

    if ((curr->long_term_idx >=0) &&
        (curr->age > oldest_lt_age) &&
        (flux->ack_status[curr->long_term_idx] == 0))
    {
      oldest_lt_idx = curr->long_term_idx;
      oldest_lt_age = curr->age;
    }
  }

  // throw out anything older than 100 frames (3-4 seconds..)
  if (oldest_lt_age > 100)
  {
    refcontrol->longterm_idx = oldest_lt_idx;
    return true;
  }
  else
    return false;
}



static bool
find_overwriteable_acked_slot (
  ref_meta_t * *  refframes_meta,
  uint8_t         total_ref_frames,
  ref_control_t * refcontrol,
  const flux_t *  flux)
{
  int youngest_acked_lt_age = INT_MAX;
  int num_acks = 0;

  int candidate_lt_idx = -1;
  int candidate_lt_age = -1;

  // TODO: Simplify/merge if tests. Why do we find the youngest first, why not
  // directly find the oldest?

  /* first find the youngest acked frame (if any) */
  for (int i = 0; i < total_ref_frames; i++)
  {
    const ref_meta_t * curr = refframes_meta[i];
    if ((curr->long_term_idx >= 0) &&
        (flux->ack_status[curr->long_term_idx] == 1) &&
        (curr->age < youngest_acked_lt_age))
    {
      youngest_acked_lt_age = curr->age;
    }
    if ((curr->long_term_idx >= 0) && 
      flux->ack_status[curr->long_term_idx] == 1)
      num_acks++;
  }

  /* if we only have one acked slot, return (no solution) */
  if (num_acks <= 1)
    return false;

  candidate_lt_age = youngest_acked_lt_age;

  /* now find the oldest acked slot */
  for (int i = 0; i < total_ref_frames; i++)
  {
    const ref_meta_t * curr = refframes_meta[i];
    if ((curr->long_term_idx >= 0) &&
        (flux->ack_status[curr->long_term_idx] == 1) &&
        (curr->age > candidate_lt_age))
    {
      candidate_lt_age = curr->age;
      candidate_lt_idx = curr->long_term_idx;
    }
  }

  if (candidate_lt_idx != -1)
  {
    refcontrol->longterm_idx = candidate_lt_idx;
    return true;
  }
  else
    return false;
}


static bool
find_overwriteable_unconfirmed_slot (
  ref_meta_t * *  refframes_meta,
  uint8_t         total_ref_frames,
  ref_control_t * refcontrol,
  const flux_t *  flux)
{
  int youngest_acked_lt_age = INT_MAX;
  int num_acks = 0;

  int candidate_lt_idx = -1;
  int candidate_lt_age = -1;

  // TODO: Simplify/merge if tests. Why do we find the youngest first, why not
  // directly find the oldest?

  /* first find the youngest acked frame (if any) */
  for (int i = 0; i < total_ref_frames; i++)
  {
    const ref_meta_t * curr = refframes_meta[i];
    if ((curr->long_term_idx >= 0) &&
        (flux->ack_status[curr->long_term_idx] == 1) &&
        (curr->age < youngest_acked_lt_age))
    {
      youngest_acked_lt_age = curr->age;
    }
    if ((curr->long_term_idx >= 0) &&
      flux->ack_status[curr->long_term_idx] == 1)
      num_acks++;
  }

  /* if we only have one acked slot, return (no solution) */
  if (num_acks <= 1)
    return false;

  /* if all frames are acked, return (no solution) */
  if (num_acks == (total_ref_frames - 1))
    return false;

  candidate_lt_age = youngest_acked_lt_age;

  /* now find the oldest unconfirmed (ack_status == 0) slot */
  for (int i = 0; i < total_ref_frames; i++)
  {
    const ref_meta_t * curr = refframes_meta[i];
    if ((curr->long_term_idx >= 0) &&
        (flux->ack_status[curr->long_term_idx] == 0) &&
        (curr->age > candidate_lt_age))
    {
      candidate_lt_age = curr->age;
      candidate_lt_idx = curr->long_term_idx;
    }
  }

  if (candidate_lt_age != youngest_acked_lt_age)
  {
    refcontrol->longterm_idx = candidate_lt_idx;
    return true;
  }
  else
    return false;
}



static bool
find_oldest_slot (
  ref_meta_t * *  refframes_meta,
  uint8_t         total_ref_frames,
  ref_control_t * refcontrol)
{
  bool ret = false;
  int oldest_lt_idx = -1;
  int oldest_lt_age = -1;

  for (int k = 0; k<total_ref_frames; k++)
  {
    const ref_meta_t * curr = refframes_meta[k];

    if ((curr->long_term_idx >=0) &&
        curr->age > oldest_lt_age)
    {
      oldest_lt_idx = curr->long_term_idx;
      oldest_lt_age = curr->age;
    }
  }
  if (oldest_lt_idx != -1)
  {
    refcontrol->longterm_idx = oldest_lt_idx;
    ret = true;
  }
  return ret;
}
