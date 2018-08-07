#include "enc_ratecontrol.h"
#include "com_intmath.h"

/*
 * TODO: adjust frame statistics for overhead information to achieve exact
 * numbers after re-encoding
 *
 * TODO: add error functions instead of fprintf stderr
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "com_compatibility.h"

/* TODO: How are this numbers calculated? */
static const uint32_t rc_qp_scale_tab[52] = {
  8,     9,  10,   11,   13,   14,   16,   18,   20,   23,  25,   29,    32,
  36,   40,  45,   51,   57,   64,   72,   81,   91,  102,  114,  128,  144,
  161, 181, 203,  228,  256,  287,  323,  362,  406,  456,  512,  575,  645,
  724, 813, 912, 1024, 1149, 1290, 1448, 1625, 1825, 2048, 2299, 2580, 2896
};




void taa_h264_rate_control_init (
  ratecontrol_t * rc,
  uint8_t         framerate,                        /* fps */
  uint16_t        bitrate,                         /* kbps */
  uint8_t         initial_qp,
  uint8_t         min_qp,
  uint8_t         max_qp,
  uint16_t        num_mbs)
{
  uint32_t bits_per_mb;
  uint32_t bits_per_frame;
  int i;

  rc->min_qp = min_qp;
  rc->max_qp = max_qp;
  rc->framerate = framerate;
  rc->bitrate = bitrate;
  rc->num_mbs = num_mbs;

  if (framerate == 0)
    framerate = 1;
  if (bitrate == 0)
    bitrate = 1;

  bits_per_frame = (bitrate * 1000) / framerate;
  bits_per_mb = bits_per_frame / num_mbs;

  // For optimize the frame skipping algorithm.
  for (i = 0; i < MAX_PREVIOUS_FRMAE_INFO_NUM; i++)
  {
    rc->previous_frame_length[i] = bits_per_frame;
  }
  rc->pre_frame_length_sum = bits_per_frame * MAX_PREVIOUS_FRMAE_INFO_NUM;
  rc->previous_frame_pos = 0;
  rc->length_compensate_ratio = 0;

  rc->target_bits = bits_per_frame;
  rc->bits_this_frame = bits_per_frame;
  rc->bits_quant_this_frame = bits_per_frame * rc_qp_scale_tab[initial_qp];
  rc->bits_sliding_frame = rc->bits_this_frame;
  rc->bits_quant_sliding_frame = rc->bits_quant_this_frame;
  rc->quant_average = initial_qp * 1024;
  rc->overprod_bits = 0;
  rc->corr_factor = 1 << 16;

  for (int i = 0; i < num_mbs; i++)
  {
    rc->prev_frame_mb_bits[i] = bits_per_mb;
    rc->prev_frame_mb_qp[i] = initial_qp;
  }
}


/* Updates the target bit rate without overwriting what we learned from past
 * frames. Is in practice a no-op function if rate controlling parameters are
 * not changed. */
void taa_h264_rate_control_update_target (
  ratecontrol_t * rc,
  uint8_t         framerate,                                 /* fps */
  uint16_t        bitrate,                                  /* kbps */
  uint8_t         min_qp,
  uint8_t         max_qp,
  uint32_t        max_mbps,
  uint32_t        max_static_mbps)
{
  rc->min_qp = min_qp;
  rc->max_qp = max_qp;
  rc->max_mbps = max_mbps;
  rc->max_static_mbps = max_static_mbps;

  if (rc->framerate != framerate || rc->bitrate != bitrate)
  {
    int i;
    rc->framerate = framerate;
    rc->bitrate = bitrate;

    if (framerate == 0)
      framerate = 1;
    if (bitrate == 0)
      bitrate = 1;

    rc->target_bits = (bitrate * 1000) / framerate;

    /* TODO: overprod bits should probably also be updated */

    // For optimize the frame skipping algorithm.
    for (i = 0; i < MAX_PREVIOUS_FRMAE_INFO_NUM; i++)
    {
      rc->previous_frame_length[i] = rc->target_bits;
    }

    rc->pre_frame_length_sum = rc->target_bits * MAX_PREVIOUS_FRMAE_INFO_NUM;
    rc->previous_frame_pos = 0;
    rc->length_compensate_ratio = 0;
  }
}

/* Initializes the rate control for a new frame. Returns wether this frame
 * should be coded or not (skipped due to rate control). */
bool taa_h264_rate_control_start_frame (
  ratecontrol_t * rc)
{
  const float scaled_target_bits = rc->target_bits * 65535.0f;
  const float bits_this_frame = rc->bits_this_frame * 1.0f + 0.001f;
  const float corr_factor = scaled_target_bits / bits_this_frame;

  /* Calculate correction factor for this frame */
  rc->corr_factor = (int64_t)(corr_factor);

  /* Initialize sliding frame statistics to frame statistics from previous frame */
  /* This ensures exact resync at the beginning of each frame */
  rc->bits_sliding_frame = rc->bits_this_frame;
  rc->bits_quant_sliding_frame = rc->bits_quant_this_frame;
  if (rc->bits_sliding_frame > 0)
    rc->quant_average = (rc->bits_quant_sliding_frame << 10) / rc->bits_sliding_frame;
  else
    rc->bits_sliding_frame = 0;

  /* Initialize frame statistics for this frame */
  rc->bits_this_frame = 0;
  rc->bits_quant_this_frame = 0;

  return true;
}


/* Adjusts QP on a macroblock according to TANDBERG rate control algorithm,
 * the same algorithm that is used in other TANDBERG codecs.
 *
 * Returns the new QP for current MB. */
uint8_t taa_h264_rate_control_mb (
  ratecontrol_t * rc,
  uint8_t         prev_qp,                         /* previus mb's qp */
  uint32_t        prev_bits,                       /* previous mb's used bits */
  uint16_t        mbpos)
{
  int32_t target_bits_current;
  uint32_t new_qp;
  uint32_t prev_qp_scaled;
  uint32_t last_qp_scaled;
  uint32_t new_qp_scaled;
  uint32_t last_bits;

  /* extract and update current and last bits and quants */
  last_bits = rc->prev_frame_mb_bits[mbpos];
  last_qp_scaled = rc_qp_scale_tab[rc->prev_frame_mb_qp[mbpos]];
  prev_qp_scaled = rc_qp_scale_tab[prev_qp];

  /* Update parameters for previous frame */
  rc->prev_frame_mb_bits[mbpos] = prev_bits;
  rc->prev_frame_mb_qp[mbpos] = prev_qp;

  /* Update sliding frame statistics */
  rc->bits_sliding_frame += (prev_bits - last_bits);
  rc->bits_quant_sliding_frame += (prev_bits * prev_qp_scaled - last_bits * last_qp_scaled);

  /* Update frame statistics */
  rc->bits_this_frame += prev_bits;
  rc->bits_quant_this_frame += prev_bits * prev_qp_scaled;

  /* Set target number of bits */
  target_bits_current = rc->target_bits - rc->overprod_bits;

  /* This shouldn't be necessary if overprod_bits is clipped as below */
  if (target_bits_current <= 0)
    target_bits_current = 1;

  /* Set Quantizer */
  new_qp_scaled = rc->bits_quant_sliding_frame / target_bits_current;
  new_qp = rc->min_qp;
  while ((rc_qp_scale_tab[new_qp] < (new_qp_scaled - (new_qp_scaled / 16)))
         && (new_qp < rc->max_qp))
  {
    new_qp++;
  }

  /* Update sliding average and overproduced bits before next MB */
  if(rc->bits_sliding_frame > 0)
    rc->quant_average = (rc->bits_quant_sliding_frame << 10) / rc->bits_sliding_frame;
  else
    rc->quant_average = 0;

  int64_t curr_bits = (rc->corr_factor * last_bits) >> 16;
  rc->overprod_bits += prev_bits - (int32_t)curr_bits;
  if (rc->overprod_bits > (rc->target_bits >> 1))
    rc->overprod_bits = rc->target_bits >> 1;
  if (rc->overprod_bits < -(rc->target_bits >> 3))
    rc->overprod_bits = -(rc->target_bits >> 3);

  return (uint8_t) new_qp;
}


/* Call this function to resync statistics for this frame. */
/* FIXME: This should be called, probably in rc_end_frame(), but isn't. */
//static void taa_h264_rate_control_resync_this_frame (
//  ratecontrol_t * rc,
//  uint32_t        real_bits_this_frame,
//  uint8_t         qp)
//{
//  rc->bits_this_frame = real_bits_this_frame;
//  rc->bits_quant_this_frame = real_bits_this_frame * rc_qp_scale_tab[qp];
//}

/**
 * Should be run at the end of a coded frame. Returns how many of the next
 * frames should be skipped. */
int taa_h264_rate_contol_end_frame (
  ratecontrol_t * rc,
  const int       num_static_mbs /* Total number of skipped MBs in frame */
#ifdef TAA_SAVE_264_MEINFO
  ,int *skip_mbps
  ,int *skip_bits
  ,int *length_compensate_ratio
#endif
)
{
  int skip_frames = 0;

  //TODO2: Only needs to be calculated if frame skipping is enabled

  const float max_overprod = 1.1;
  const float max_mbps = (float) rc->max_mbps;
  const float max_static_mbps = (float) rc->max_static_mbps;
  const float num_mbs = (float) rc->num_mbs;
  int frameskip_mbps = 0;
  int frameskip_bits = 0;
  uint32_t avg_bits_this_frame;

  if (max_mbps > 0 && max_static_mbps > 0)
  {
    /* Find skipping based on max (static) mbps and produced mbs. */
    const float nonstatic_factor = (num_mbs - num_static_mbs) / max_mbps;
    const float static_factor = num_static_mbs / max_static_mbps;
    frameskip_mbps = ceilf (rc->framerate * (nonstatic_factor + static_factor) - 1);
  }

  // For optimize the frame skipping algorithm.
  rc->pre_frame_length_sum -= rc->previous_frame_length[rc->previous_frame_pos];
  rc->pre_frame_length_sum += rc->bits_this_frame;
  rc->previous_frame_length[rc->previous_frame_pos] = rc->bits_this_frame;
  rc->previous_frame_pos += 1;
  rc->previous_frame_pos = rc->previous_frame_pos & (MAX_PREVIOUS_FRMAE_INFO_NUM - 1);

  avg_bits_this_frame = rc->pre_frame_length_sum / MAX_PREVIOUS_FRMAE_INFO_NUM;

  /* Always consider overproduction by rc algorithm */
  {
    int tmp;
    float val = avg_bits_this_frame / (rc->target_bits * max_overprod) - 1;
    if(val > 0)
    {
      val -= rc->length_compensate_ratio / 100.0;
      tmp = ceilf(val);
      rc->length_compensate_ratio = (int)((tmp - val) * 100);
    }
    else
    {
      tmp = 0;
      rc->length_compensate_ratio = 0;
    }

#ifdef TAA_SAVE_264_MEINFO
    *length_compensate_ratio = rc->length_compensate_ratio;
#endif

    frameskip_bits = tmp;
  }

#ifdef TAA_SAVE_264_MEINFO
  *skip_mbps = frameskip_mbps;
  *skip_bits = frameskip_bits;
#endif

  skip_frames = (uint8_t) max (frameskip_mbps, frameskip_bits);

  return skip_frames;
}


ratecontrol_t * taa_h264_rate_control_create (
  int num_mbs)
{
  ratecontrol_t * rc = TAA_H264_MALLOC (sizeof (ratecontrol_t));
  if (!rc)
  {
    return NULL;
  }
  rc->prev_frame_mb_qp = NULL;
  rc->prev_frame_mb_bits = NULL;

  rc->prev_frame_mb_qp = TAA_H264_MALLOC (num_mbs * sizeof (uint8_t));
  if (!rc->prev_frame_mb_qp)
  {
    TAA_H264_FREE (rc);
    return NULL;
  }

  rc->prev_frame_mb_bits = TAA_H264_MALLOC (num_mbs * sizeof (uint32_t));
  if (!rc->prev_frame_mb_bits)
  {
    TAA_H264_FREE (rc->prev_frame_mb_qp);
    TAA_H264_FREE (rc);
    return NULL;
  }

  return rc;
}

void taa_h264_rate_control_delete (
  ratecontrol_t * rc)
{
  if (rc == NULL)
    return;

  TAA_H264_FREE (rc->prev_frame_mb_qp);
  TAA_H264_FREE (rc->prev_frame_mb_bits);
  TAA_H264_FREE (rc);
}
