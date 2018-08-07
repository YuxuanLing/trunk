#ifndef _ENC_GOP_H_
#define _ENC_GOP_H_

#include "enc_dpb.h"
#include "taah264enc.h"

#include "taah264stdtypes.h"

#define MAX_TEMPORAL_LEVELS 8
struct gop_s
{
  uint8_t levels;
  uint8_t period[MAX_TEMPORAL_LEVELS];
  uint8_t count[MAX_TEMPORAL_LEVELS];
};

struct ref_control_s
{
  /* The ref_idx of frame to use as reference. */
  int ref_idx;
  /* The long term frame index to set for current frame (if any). */
  int longterm_idx;
};

void taa_h264_reset_gop (
  gop_t * gop,
  uint8_t size,
  uint8_t levels);

uint8_t taa_h264_update_gop (
  gop_t * gop);

bool taa_h264_gop_is_equal (
  const gop_t * gop,
  uint8_t       size);

bool taa_h264_hierarchical_reference_control (
  /* TODO: Make this const */
  ref_meta_t * *  refframes_meta,
  uint8_t         total_ref_frames,
  uint8_t         temporal_id,
  uint8_t         num_temporal_levels,
  ref_control_t * refcontrol);

bool taa_h264_flux_super_p_control (
  ref_meta_t * *  refframes_meta,
  uint8_t         total_ref_frames,
  uint8_t         max_ref_frames,
  ref_control_t * refcontrol,
  const flux_t *  flux);

bool  taa_h264_flux_decide_refframes (
  ref_meta_t * *  refframes_meta,
  uint8_t         total_ref_frames,
  uint8_t         max_ref_frames,
  ref_control_t * refcontrol,
  const flux_t *  flux,
  bool            is_super_p);

#endif
