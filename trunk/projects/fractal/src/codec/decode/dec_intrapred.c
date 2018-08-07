/* -*- indent-tabs-mode:nil; c-basic-offset:4 -*- */

#include "dec_intrapred.h"
#include "com_intmath.h"
#include "com_prediction.h"
#include "com_error.h"

#include <string.h>

static bool taa_h264_intra_pred_luma_dc (
  uint8_t *       recon,
  int             stride,
  const uint8_t * left,
  const uint8_t * top,
  bool            left_avail,
  bool            top_avail,
  const int16_t * diff)
{
  int shift = 1;
  int dc = 0;

  /* First, find DC */
  if (left_avail)
  {
    /* Inside MB */
    for (int i = 0; i < 4; i++)
      dc += left [i];
    dc += 2;
    shift++;
  }

  if (top_avail)
  {
    for (int j = 0; j < 4; j++)
      dc += top [j];
    dc += 2;
    shift++;
  }

  if (!left_avail && !top_avail)
  {
    dc = 128;
    shift = 0;
  }
  dc >>= shift;

  /* Second, reconstruct the 4x4 block */
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
      recon[i * stride + j] =
        (uint8_t) max(0, min(255, (diff[i * 4 + j] + dc)));
  }

  return true;
}


static bool taa_h264_intra_pred_luma_vert (
  uint8_t *       recon,
  int             stride,
  const uint8_t * top,
  const int16_t * diff)
{
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      recon[i * stride + j] =
        (uint8_t) max(0, min(255, (diff[i * 4 + j] + top[j])));
    }
  }
  return true;
}


static bool taa_h264_intra_pred_luma_hor (
  uint8_t *       recon,
  int             stride,
  const uint8_t * left,
  const int16_t * diff)
{
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      recon[i * stride + j] =
        (uint8_t) max(0, min(255, (diff[i * 4 + j] + left[i])));
    }
  }
  return true;
}

static void taa_h264_intra_pred_luma_down_left (
  const uint8_t * top_orig,
  bool            top_right_avail,
  uint8_t *       pred)
{
  uint8_t top[8];

  memcpy (top, top_orig, 8 * sizeof (uint8_t));
  if (top_right_avail == false)
  {
    top[4] =  top[5] = top[6] = top[7] = top[3];
  }

  pred[0 * 4 + 0] =
    (uint8_t)((top[0] + 2 * top[1] + top[2] + 2) >> 2);
  pred[0 * 4 + 1] =
    pred[1 * 4 + 0] =
      (uint8_t)((top[1] + 2 * top[2] + top[3] + 2) >> 2);
  pred[0 * 4 + 2] =
    pred[1 * 4 + 1] =
      pred[2 * 4 + 0] =
        (uint8_t)((top[2] + 2 * top[3] + top[4] + 2) >> 2);
  pred[0 * 4 + 3] =
    pred[1 * 4 + 2] =
      pred[2 * 4 + 1] =
        pred[3 * 4 + 0] =
          (uint8_t)((top[3] + 2 * top[4] + top[5] + 2) >> 2);
  pred[1 * 4 + 3] =
    pred[2 * 4 + 2] =
      pred[3 * 4 + 1] =
        (uint8_t)((top[4] + 2 * top[5] + top[6] + 2) >> 2);
  pred[2 * 4 + 3] =
    pred[3 * 4 + 2] =
      (uint8_t)((top[5] + 2 * top[6] + top[7] + 2) >> 2);
  pred[3 * 4 + 3] =
    (uint8_t)((top[6] + 3 * top[7] + 2) >> 2);
}


static void taa_h264_intra_pred_luma_down_right (
  const uint8_t * left,
  const uint8_t * top,
  uint8_t *       pred)
{
  pred[3 * 4 + 0] =
    (uint8_t)((left[3] + 2 * left[2] + left[1] + 2) >> 2);
  pred[2 * 4 + 0] =
    pred[3 * 4 + 1] =
      (uint8_t)((left[2] + 2 * left[1] + left[0] + 2) >> 2);
  pred[1 * 4 + 0] =
    pred[2 * 4 + 1] =
      pred[3 * 4 + 2] =
        (uint8_t)((left[1] + 2 * left[0] + top[-1] + 2) >> 2);
  pred[0 * 4 + 0] =
    pred[1 * 4 + 1] =
      pred[2 * 4 + 2] =
        pred[3 * 4 + 3] =
          (uint8_t)((left[0] + 2 * top[-1] + top[0] + 2) >> 2);
  pred[0 * 4 + 1] =
    pred[1 * 4 + 2] =
      pred[2 * 4 + 3] =
        (uint8_t)((top[-1] + 2 * top[0] + top[1] + 2) >> 2);
  pred[0 * 4 + 2] =
    pred[1 * 4 + 3] =
      (uint8_t)((top[0] + 2 * top[1] + top[2] + 2) >> 2);
  pred[0 * 4 + 3] =
    (uint8_t)((top[1] + 2 * top[2] + top[3] + 2) >> 2);

}


static void taa_h264_intra_pred_luma_vert_right (
  const uint8_t * left,
  const uint8_t * top,
  uint8_t *       pred)
{
  pred[0 * 4 + 0] =
    pred[2 * 4 + 1] =
      (uint8_t)((top[-1] + top[0] + 1) >> 1);
  pred[0 * 4 + 1] =
    pred[2 * 4 + 2] =
      (uint8_t)((top[0] + top[1] + 1) >> 1);
  pred[0 * 4 + 2] =
    pred[2 * 4 + 3] =
      (uint8_t)((top[1] + top[2] + 1) >> 1);
  pred[0 * 4 + 3] =
    (uint8_t)((top[2] + top[3] + 1) >> 1);
  pred[1 * 4 + 0] =
    pred[3 * 4 + 1] =
      (uint8_t)((left[0] + 2 * top[-1] + top[0] + 2) >> 2);
  pred[1 * 4 + 1] =
    pred[3 * 4 + 2] =
      (uint8_t)((top[-1] + 2 * top[0] + top[1] + 2) >> 2);
  pred[1 * 4 + 2] =
    pred[3 * 4 + 3] =
      (uint8_t)((top[0] + 2 * top[1] + top[2] + 2) >> 2);
  pred[1 * 4 + 3] =
    (uint8_t)((top[1] + 2 * top[2] + top[3] + 2) >> 2);
  pred[2 * 4 + 0] =
    (uint8_t)((top[-1] + 2 * left[0] + left[1] + 2) >> 2);
  pred[3 * 4 + 0] =
    (uint8_t)((left[0] + 2 * left[1] + left[2] + 2) >> 2);

}


static void taa_h264_intra_pred_luma_horz_down (
  const uint8_t * left,
  const uint8_t * top,
  uint8_t *       pred)
{
  pred[0 * 4 + 0] =
    pred[1 * 4 + 2] =
      (uint8_t)((top[-1] + left[0] + 1) >> 1);
  pred[0 * 4 + 1] =
    pred[1 * 4 + 3] =
      (uint8_t)((left[0] + 2 * top[-1] + top[0] + 2) >> 2);
  pred[0 * 4 + 2] =
    (uint8_t)((top[-1] + 2 * top[0] + top[1] + 2) >> 2);
  pred[0 * 4 + 3] =
    (uint8_t)((top[0] + 2 * top[1] + top[2] + 2) >> 2);
  pred[1 * 4 + 0] =
    pred[2 * 4 + 2] =
      (uint8_t)((left[0] + left[1] + 1) >> 1);
  pred[1 * 4 + 1] =
    pred[2 * 4 + 3] =
      (uint8_t)((top[-1] + 2 * left[0] + left[1] + 2) >> 2);
  pred[2 * 4 + 0] =
    pred[3 * 4 + 2] =
      (uint8_t)((left[1] + left[2] + 1) >> 1);
  pred[2 * 4 + 1] =
    pred[3 * 4 + 3] =
      (uint8_t)((left[0] + 2 * left[1] + left[2] + 2) >> 2);
  pred[3 * 4 + 0] =
    (uint8_t)((left[2] + left[3] + 1) >> 1);
  pred[3 * 4 + 1] =
    (uint8_t)((left[1] + 2 * left[2] + left[3] + 2) >> 2);
}


static void taa_h264_intra_pred_luma_vert_left (
  const uint8_t * top_orig,
  bool            top_right_avail,
  uint8_t *       pred)
{
  uint8_t top[8];

  memcpy (top, top_orig, 8 * sizeof (uint8_t));
  if (top_right_avail == false)
  {
    top[4] =  top[5] = top[6] = top[7] = top[3];
  }

  pred[0 * 4 + 0] =
    (uint8_t)((top[0] + top[1] + 1) >> 1);
  pred[0 * 4 + 1] =
    pred[2 * 4 + 0] =
      (uint8_t)((top[1] + top[2] + 1) >> 1);
  pred[0 * 4 + 2] =
    pred[2 * 4 + 1] =
      (uint8_t)((top[2] + top[3] + 1) >> 1);
  pred[0 * 4 + 3] =
    pred[2 * 4 + 2] =
      (uint8_t)((top[3] + top[4] + 1) >> 1);
  pred[2 * 4 + 3] =
    (uint8_t)((top[4] + top[5] + 1) >> 1);
  pred[1 * 4 + 0] =
    (uint8_t)((top[0] + 2 * top[1] + top[2] + 2) >> 2);
  pred[1 * 4 + 1] =
    pred[3 * 4 + 0] =
      (uint8_t)((top[1] + 2 * top[2] + top[3] + 2) >> 2);
  pred[1 * 4 + 2] =
    pred[3 * 4 + 1] =
      (uint8_t)((top[2] + 2 * top[3] + top[4] + 2) >> 2);
  pred[1 * 4 + 3] =
    pred[3 * 4 + 2] =
      (uint8_t)((top[3] + 2 * top[4] + top[5] + 2) >> 2);
  pred[3 * 4 + 3] =
    (uint8_t)((top[4] + 2 * top[5] + top[6] + 2) >> 2);
}


static void taa_h264_intra_pred_luma_horz_up (
  const uint8_t * left,
  uint8_t *       pred)
{
  pred[0 * 4 + 0] =
    (uint8_t)((left[0] + left[1] + 1) >> 1);
  pred[0 * 4 + 1] =
    (uint8_t)((left[0] + 2 * left[1] + left[2] + 2) >> 2);
  pred[0 * 4 + 2] =
    pred[1 * 4 + 0] =
      (uint8_t)((left[1] + left[2] + 1) >> 1);
  pred[0 * 4 + 3] =
    pred[1 * 4 + 1] =
      (uint8_t)((left[1] + 2 * left[2] + left[3] + 2) >> 2);
  pred[1 * 4 + 2] =
    pred[2 * 4 + 0] =
      (uint8_t)((left[2] + left[3] + 1) >> 1);
  pred[1 * 4 + 3] =
    pred[2 * 4 + 1] =
      (uint8_t)((left[2] + 3 * left[3] + 2) >> 2);
  pred[2 * 4 + 3] =
    pred[3 * 4 + 1] =
      pred[3 * 4 + 0] =
        pred[2 * 4 + 2] =
          pred[3 * 4 + 2] =
            pred[3 * 4 + 3] =
              (uint8_t) left[3];
}


bool taa_h264_intra_4x4_pred_and_recon_luma (
  imode4_t        pred_mode,
  uint8_t *       recon,                                      /* pointer to y for this 4x4 block */
  int             stride,
  const uint8_t * left,
  const uint8_t * top,
  bool            left_avail,
  bool            top_avail,
  bool            top_left_avail,
  bool            top_right_avail,
  const int16_t * diff)
{
  bool ret = false;
  if (pred_mode <= DC_PRED)
  {
    switch (pred_mode)
    {
    case VERT_PRED:
      if (top_avail)
        ret = taa_h264_intra_pred_luma_vert (
          recon, stride, top, diff);
      break;
    case HOR_PRED:
      if (left_avail)
        ret = taa_h264_intra_pred_luma_hor (
          recon, stride, left, diff);
      break;
    case DC_PRED:
      ret = taa_h264_intra_pred_luma_dc (
        recon, stride, left, top, left_avail, top_avail, diff);
      break;
    default:
      TAA_H264_DEBUG_ASSERT (false);
      break;
    }
  }
  else
  {
    uint8_t pred[4 * 4];

    switch (pred_mode)
    {
    case DIAG_DOWN_LEFT_PRED:
      if (top_avail)
      {
        taa_h264_intra_pred_luma_down_left (
          top, top_right_avail, pred);
        ret = true;
      }
      break;
    case DIAG_DOWN_RIGHT_PRED:
      if (left_avail && top_avail && top_left_avail)
      {
        taa_h264_intra_pred_luma_down_right (
          left, top, pred);
        ret = true;
      }
      break;
    case VERT_RIGHT_PRED:
      if (left_avail && top_avail && top_left_avail)
      {
        taa_h264_intra_pred_luma_vert_right (
          left, top, pred);
        ret = true;
      }
      break;
    case HOR_DOWN_PRED:
      if (left_avail && top_avail && top_left_avail)
      {
        taa_h264_intra_pred_luma_horz_down (
          left, top, pred);
        ret = true;
      }
      break;
    case VERT_LEFT_PRED:
      if (top_avail)
      {
        taa_h264_intra_pred_luma_vert_left (
          top, top_right_avail, pred);
        ret = true;
      }
      break;
    case HOR_UP_PRED:
      if (left_avail)
      {
        taa_h264_intra_pred_luma_horz_up (
          left, pred);
        ret = true;
      }
      break;
    default:
      TAA_H264_DEBUG_ASSERT (false);
      break;
    }

    for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 4; j++)
      {
        recon[i * stride + j] =
          (uint8_t) max(0, min(255, (diff[i * 4 + j] + pred[i * 4 + j])));
      }
    }
  }
  return ret;
}

static
void taa_h264_intra_16x16_pred_luma_dc (
  uint8_t *       pred,
  const uint8_t * left,
  const uint8_t * top,
  bool            left_avail,
  bool            top_avail)
{
  int shift = 3;
  int sum = 0;
  if (left_avail)
  {
    for (int i = 0; i < MB_HEIGHT_Y; i++)
      sum += left[i];
    sum += 8;
    shift++;
  }
  if (top_avail)
  {
    for (int i = 0; i < MB_WIDTH_Y; i++)
      sum += top[i];
    sum += 8;
    shift++;
  }
  if (shift == 3)
  {
    sum = 128;
    shift = 0;
  }
  uint8_t dc = (uint8_t)(sum >> shift);
  for (int i = 0; i < MB_SIZE_Y; i++)
    pred[i] = dc;
}

static
void taa_h264_intra_16x16_pred_luma_vert (
  uint8_t *       pred,
  const uint8_t * top)
{
  for (int i = 0; i < MB_HEIGHT_Y; i++)
  {
    memcpy (&pred[i * MB_WIDTH_Y], top, MB_WIDTH_Y * sizeof(uint8_t));
  }
}

static
void taa_h264_intra_16x16_pred_luma_hor (
  uint8_t *       pred,
  const uint8_t * left)
{
  for (int i = 0; i < MB_HEIGHT_Y; i++)
  {
    for (int j = 0; j < MB_WIDTH_Y; j++)
      pred [i * MB_WIDTH_Y + j] = left[i];
  }
}

static
void taa_h264_intra_16x16_pred_luma_plane (
  uint8_t *       pred,
  const uint8_t * left,
  const uint8_t * top,
  uint8_t         topleft)
{
  int h = 0;
  int v = 0;

  for (int i = 1; i <= 7; i++)
  {
    h += i * (top[7 + i] - top[7 - i]);
    v += i * (left[7 + i] - left[7 - i]);
  }

  h += 8 * (top[15] - topleft);
  v += 8 * (left[15] - topleft);

  int a = 16 * (top[15] + left[15]);
  int b = (5 * h + 32) >> 6;
  int c = (5 * v + 32) >> 6;

  int t0 = a - 7 * (b + c) + 16;
  for (int i = 0; i < MB_HEIGHT_Y; i++)
  {
    int t = t0;
    for (int j = 0; j < MB_WIDTH_Y; j++)
    {
      int16_t temp = (int16_t) (t >> 5);
      pred[i * MB_WIDTH_Y + j] = (uint8_t)((temp > 255) ? 255 : (temp < 0) ? 0 : temp);
      t += b;
    }
    t0 += c;
  }
}


bool taa_h264_intra_16x16_pred_and_recon_luma (
  imode16_t       pred_mode,
  uint8_t *       recon,
  int             stride,
  bool            left_avail,
  bool            top_avail,
  bool            topleft_avail,
  const int16_t * residual)
{
  uint8_t pred[MB_SIZE_Y];
  uint8_t left[MB_HEIGHT_Y];
  uint8_t * top = NULL;
  uint8_t topleft = 0;

  bool ret = false;

  if (top_avail)
  {
    top = &recon [-stride];
  }
  if (topleft_avail)
  {
    topleft = recon [-stride - 1];
  }
  if (left_avail)
  {
    int left_pos = -1;
    for (int j = 0; j < MB_HEIGHT_Y; j++)
    {
      left [j] = recon [left_pos];
      left_pos += stride;
    }
  }

  switch (pred_mode)
  {
  case HOR_PRED_16:
    if (left_avail)
    {
      taa_h264_intra_16x16_pred_luma_hor (pred, left);
      ret = true;
    }
    break;
  case VERT_PRED_16:
    if (top_avail)
    {
      taa_h264_intra_16x16_pred_luma_vert (pred, top);
      ret = true;
    }
    break;
  case DC_PRED_16:
    taa_h264_intra_16x16_pred_luma_dc (pred, left, top,
                                       left_avail, top_avail);
    ret = true;
    break;
  case PLANE_PRED_16:
    if (left_avail && top_avail && topleft_avail)
    {
      taa_h264_intra_16x16_pred_luma_plane (pred,
                                            left, top, topleft);
      ret = true;
    }
    break;
  default:
    TAA_H264_DEBUG_ASSERT (false);
  }

  /* Reconstruction */
  /* TODO: The pred, diff and recon have different layouts, so this is kind
   * of clumsy. Reconsider the memory layout */
  for (int k = 0; k < NUM_4x4_BLOCKS_Y; k++)
  {
    const int block_size = 4 * 4;
    int y = (k / 4) * 4;
    int x = (k % 4) * 4;

    uint8_t * recon_ptr = &recon[y * stride + x];
    const uint8_t * pred_ptr  = &pred[y * MB_WIDTH_Y + x];
    const int16_t * residual_ptr  = &residual [k * block_size];

    for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 4; j++)
        recon_ptr[i * stride + j] =
          (uint8_t) max(0, min(255, (residual_ptr[i * 4 + j] + pred_ptr[i * MB_WIDTH_Y + j])));
    }
  }

  return ret;
}

static
void taa_h264_intra_pred_chroma_plane (
  uint8_t *       pred,
  const uint8_t * left,
  const uint8_t * top,
  uint8_t         topleft)
{
  int h = 0;
  int v = 0;

  for (int i = 1; i <= 3; i++)
  {
    h += i * (top[3 + i] - top[3 - i]);
    v += i * (left[3 + i] - left[3 - i]);
  }

  h += 4 * (top[7] - topleft);
  v += 4 * (left[7] - topleft);

  int a = 16 * (top[7] + left[7]);
  int b = (17 * h + 16) >> 5;
  int c = (17 * v + 16) >> 5;

  int t0 = a - 3 * (b + c) + 16;
  for (int i = 0; i < MB_HEIGHT_UV; i++)
  {
    int t = t0;
    for (int j = 0; j < MB_WIDTH_UV; j++)
    {
      pred[i * MB_STRIDE_UV + j] = (uint8_t) max(min((t >> 5), 255), 0);
      t += b;
    }
    t0 += c;
  }
}

bool taa_h264_intra_pred_and_recon_chroma_8x8 (
  imode8_t        pred_mode,
  uint8_t *       recon,
  int             stride,
  bool            left_avail,
  bool            top_avail,
  bool            top_left_avail,
  const int16_t * diff)
{
  /* PRED array is allocated for both u and v, even though we only
   * process component in this function. This is because the (common)
   * intra prediction functions are optimized for the encoder which
   * uses combined U and V. */
  uint8_t pred[MB_SIZE_UV * 2];
  uint8_t left[MB_HEIGHT_UV];
  uint8_t * top = NULL;

  bool ret = false;

  if (top_avail)
  {
    top = &recon [-stride];
  }
  if (left_avail)
  {
    int left_pos = -1;
    for (int j = 0; j < MB_HEIGHT_UV; j++)
    {
      left [j] = recon [left_pos];
      left_pos += stride;
    }
  }

  switch (pred_mode)
  {
  case DC_PRED_8:
    taa_h264_intra_pred_chroma_dc (pred, left, top,
                                   left_avail, top_avail);
    ret = true;
    break;
  case HOR_PRED_8:
    if (left_avail)
    {
      taa_h264_intra_pred_chroma_hor (pred, left);
      ret = true;
    }
    break;
  case VERT_PRED_8:
    if (top_avail)
    {
      taa_h264_intra_pred_chroma_vert (pred, top);
      ret = true;
    }
    break;
  case PLANE_PRED_8:
    if (left_avail && top_avail && top_left_avail)
    {
      taa_h264_intra_pred_chroma_plane (pred, left, top, top[-1]);
      ret = true;
    }
    break;
  default:
    TAA_H264_DEBUG_ASSERT (false);
  }

  /* Reconstruction */
  /* TODO: The pred, diff and recon have different layouts, so this is kind
   * of clumsy */
  for (int k = 0; k < NUM_4x4_BLOCKS_UV; k++)
  {
    int y = (k / 2) * 4;
    int x = (k % 2) * 4;
    const int block_size = 4 * 4;
    uint8_t * recon_ptr = &recon[y * stride + x];
    uint8_t * pred_ptr  = &pred[y * MB_STRIDE_UV + x];
    const int16_t * diff_ptr  = &diff [k * block_size];

    for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 4; j++)
        recon_ptr[i * stride + j] =
          (uint8_t) max(0, min(255, (diff_ptr[i * 4 + j] + pred_ptr[i * MB_STRIDE_UV + j])));
    }
  }

  return ret;
}
