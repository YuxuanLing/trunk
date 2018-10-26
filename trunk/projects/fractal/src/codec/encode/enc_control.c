#include "enc_control.h"
#include "enc_gop.h"
#include "enc_ratecontrol.h"
#include "com_intmath.h"

#include <stdlib.h>
#include <stdio.h>


control_t * taa_h264_control_create (
  uint8_t  num_layers,
  uint16_t num_mbs)
{
  control_t * ctrl = TAA_H264_MALLOC (sizeof (control_t));
  if (!ctrl)
    return NULL;

  ctrl->num_layers = num_layers;

  ctrl->ratecontrol = taa_h264_rate_control_create (num_mbs);
  if (!ctrl->ratecontrol)
  {
    TAA_H264_FREE (ctrl);
    ctrl = NULL;
  }

  return ctrl;
}

void taa_h264_control_init (
  control_t * ctrl,
  uint8_t     gop_size,
  uint8_t     framerate,
  uint16_t    bitrate,
  uint8_t     initial_qp,
  uint8_t     min_qp,
  uint8_t     max_qp,
  uint16_t    num_mbs)
{
  ctrl->enable_frame_skip = false;
  ctrl->skip_frames = 0;
  ctrl->fixed_qp = false;
  ctrl->intra_period = 0;
  ctrl->frames_since_intra = 0;

  taa_h264_reset_gop (&ctrl->gop, gop_size, ctrl->num_layers);
  taa_h264_rate_control_init (
    ctrl->ratecontrol, framerate, bitrate, initial_qp, min_qp, max_qp, num_mbs);
}

void taa_h264_control_delete (control_t * ctrl)
{
  if (ctrl)
  {
    taa_h264_rate_control_delete (ctrl->ratecontrol);
    TAA_H264_FREE (ctrl);
  }
}

/**
 * Decides the frame type, that is whether this frame is of intra or inter type
 * and its temporal id.
 *
 * Returns the temporal id of the frame. SKIP returns TRUE if frame should be
 * skipped according to rate control. INTRA returns TRUE if frame should be
 * coded as INTRA. */
uint8_t taa_h264_control_frame_type (
  control_t * ctrl,
  bool        enable_frame_skip,
  bool        request_intra,
  int         new_intra_period,
  uint8_t     new_gop_size,
  int         active_temporal_levels,
  bool *      skip,
  bool *      intra)
{
  uint8_t temporal_id;

  *intra = false;
  *skip = false;
  ctrl->intra_period = new_intra_period;

  if (ctrl->skip_frames > 0 && enable_frame_skip)
  {
    *skip = true;
    ctrl->skip_frames--;
    return 0;
  }
  else if (ctrl->intra_period > 0 &&
           ctrl->frames_since_intra >= ctrl->intra_period)
  {
    *intra = true;
  }
  else
  {
    *intra = request_intra;
  }

  if (*intra)
    ctrl->frames_since_intra = 1;
  else
    ctrl->frames_since_intra++;

  /* If GOP is not equal (meaning a different frequency between P-frames of
   * the temporal layers), reset and start from the beginning of the new
   * GOP. */
  if (*intra || !taa_h264_gop_is_equal (&ctrl->gop, new_gop_size))
  {
    temporal_id = 0;
    taa_h264_reset_gop (&ctrl->gop, new_gop_size, ctrl->num_layers);
  }
  else
  {
    /* Find the temporal id for this frame according to the (previously) defined
     * GOP */
    temporal_id = taa_h264_update_gop (&ctrl->gop);
  }

  /* Negative values means feature is disabled */
  if ((active_temporal_levels >= 0) &&
      (active_temporal_levels <= temporal_id))
    {
      *skip = true;
    }


  return temporal_id;
}

/**
 * Returns the adjusted target bitrate for this frame, taking into account the
 * GOP structure and current temporal id.
 */
static uint16_t taa_h264_adjust_target_bitrate (
  control_t * ctrl,
  uint8_t     temporal_id,
  uint16_t    original_bitrate)
{
  /* TODO: Research and properly distribute bits between temporal layers. This
   * should be optimized for a few cases, but also general enough to work decent
   * with other GOP structures. */

  const int gop_size = ctrl->gop.period[0];

  if (ctrl->num_layers == 1 || gop_size == 1)
    return original_bitrate;

  /* A (too) simple formula to calculate how much extra bits will be used on the
   * base frame rate. We add extra bits to the base layer, for the other frames
   * we subtract a some. We calucate a factor to the _average_ bits per frame.
   */
  const float max_factor0 = 3.0;
  const float magic = 7.0;
  float factor0 = min (1 + (gop_size - 1) / magic, max_factor0);
  float factor;
  if (temporal_id == 0)
  {
    factor = factor0;
  }
  else
  {
    /* We adjust the factor for the other frames so that
     *   1 * B0 + (GOPSIZE-1) * Bn = GOPSIZE * Bavg
     * resulting in equal bitrate for all other layers than the base layer.
     */
    factor = (gop_size - factor0) / (gop_size - 1.0f);

  }
  return (uint16_t) (original_bitrate * factor);
}

/**
 * Prepares the controller for encoding of a new frame and possibly update the
 * target encoding values.
 */
void taa_h264_control_start_frame (
  control_t * ctrl,
  uint8_t     temporal_id,
  uint8_t     base_qp,
  uint8_t     framerate,                                /* fps */
  uint16_t    bitrate,                                  /* kbps */
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
  )
{
  ctrl->fixed_qp = (base_qp != TAA_H264_QP_NOT_SPECIFIED);

  if (ctrl->fixed_qp)
  {
    /* TODO: Auto-tune this QP offset table.
     * These numbers are just picked out of the blue. */
    const int qp_offset[MAX_TEMPORAL_LEVELS] = {0, 3, 4, 5, 6, 7, 8, 9};
    int idx = min (temporal_id, MAX_TEMPORAL_LEVELS-1);
    ctrl->frame_qp = (uint8_t) (base_qp + qp_offset[idx]);
#ifdef TAA_SAVE_264_MEINFO
    if(debug_flag) *data_position += sprintf(data_need_send  + *data_position,"    Fixed_qp=%d ",ctrl->frame_qp);
#endif
  }
  else
  {
    bitrate = taa_h264_adjust_target_bitrate (ctrl, temporal_id, bitrate);
#ifdef TAA_SAVE_264_MEINFO
    if(debug_flag) *data_position += sprintf(data_need_send  + *data_position,  "Calculated_bitrate=%d ",bitrate);
#endif

    taa_h264_rate_control_update_target (
      ctrl->ratecontrol, framerate, bitrate, min_qp, max_qp, max_mbps,
      max_static_mbps);

#ifdef TAA_SAVE_264_MEINFO
    if(debug_flag) *data_position += sprintf(data_need_send  + *data_position,
            "min_qp=%d max_qp=%d framerate=%d bitrate=%d ",
            ctrl->ratecontrol->min_qp,
            ctrl->ratecontrol->max_qp,
            ctrl->ratecontrol->framerate,
            ctrl->ratecontrol->bitrate);
#endif
    taa_h264_rate_control_start_frame (ctrl->ratecontrol);
  }
}

/**
 * Notifies the controller about the results after encoding a frame, so that the
 * controller can prepare for next frame.
 */
void taa_h264_control_end_frame (
  control_t * ctrl,
  uint32_t    num_static_mbs
#ifdef TAA_SAVE_264_MEINFO
  ,int *skip_frame
  ,int *skip_mbps
  ,int *skip_bits
  ,int *length_compensate_ratio
#endif
)
{
  if (!ctrl->fixed_qp)
  {
#ifdef TAA_SAVE_264_MEINFO
    ctrl->skip_frames = taa_h264_rate_contol_end_frame (ctrl->ratecontrol, num_static_mbs, skip_mbps, skip_bits, length_compensate_ratio);
    *skip_frame = ctrl->skip_frames;
#else
    ctrl->skip_frames = taa_h264_rate_contol_end_frame (ctrl->ratecontrol, num_static_mbs);
#endif
  }
}


uint8_t taa_h264_control_mb (
  control_t * ctrl,
  uint8_t     prev_qp,                         /* previus mb's qp */
  uint32_t    prev_bits,                       /* previous mb's used bits */
  uint16_t    mbpos
  )
{
  uint8_t qp;
  if (ctrl->fixed_qp)
  {
    qp = ctrl->frame_qp;
  }
  else
  {
    qp = taa_h264_rate_control_mb (ctrl->ratecontrol, prev_qp, prev_bits, mbpos);
  }

  return qp;
}
