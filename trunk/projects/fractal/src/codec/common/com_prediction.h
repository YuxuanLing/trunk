/* -*- indent-tabs-mode:nil; c-basic-offset:4 -*- */

#ifndef _PREDICTION_H_
#define _PREDICTION_H_

#include "com_common.h"
#include "com_intmath.h"
#include <memory.h>

/**
 * INTRA PREDICTION
 * ************************************************************/
static __inline
void taa_h264_intra_pred_chroma_dc (
  uint8_t *       pred,
  const uint8_t * left,
  const uint8_t * top,
  bool            left_avail,
  bool            top_avail)

{
  int sum_top1 = 0;
  int sum_top2 = 0;
  int sum_left1 = 0;
  int sum_left2 = 0;
  int dc1, dc2, dc3, dc4;

  if (top_avail)
  {
    for (int k = 0; k < 4; k++)
    {
      sum_top1 += top[k];
      sum_top2 += top[k + 4];
    }
    sum_top1 += 2;
    sum_top2 += 2;
  }
  if (left_avail)
  {
    for (int k = 0; k < 4; k++)
    {
      sum_left1 += left[k];
      sum_left2 += left[k + 4];
    }
    sum_left1 += 2;
    sum_left2 += 2;
  }

  if (top_avail && left_avail)
  {
    dc1 = (sum_top1 + sum_left1) / 8;
    dc2 = (sum_top2) / 4;
    dc3 = (sum_left2) / 4;
    dc4 = (sum_top2 + sum_left2) / 8;
  }
  else if (top_avail)
  {
    dc1 = dc3 = (sum_top1) / 4;
    dc2 = dc4 = (sum_top2) / 4;
  }
  else if (left_avail)
  {
    dc1 = dc2 = (sum_left1) / 4;
    dc3 = dc4 = (sum_left2) / 4;
  }
  else
  {
    dc1 = dc2 = dc3 = dc4 = 128;
  }

  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      pred[(i    ) * MB_STRIDE_UV + j    ] =  (uint8_t) dc1;
      pred[(i    ) * MB_STRIDE_UV + j + 4] =  (uint8_t) dc2;
      pred[(i + 4) * MB_STRIDE_UV + j    ] =  (uint8_t) dc3;
      pred[(i + 4) * MB_STRIDE_UV + j + 4] =  (uint8_t) dc4;
    }
  }
}

static __inline
void taa_h264_intra_pred_chroma_vert (
  uint8_t *       pred,
  const uint8_t * top)
{
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      pred[i * MB_STRIDE_UV + j] = top[j];
    }
  }
}

static __inline
void taa_h264_intra_pred_chroma_hor (
  uint8_t *       pred,
  const uint8_t * left)
{
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      pred[i * MB_STRIDE_UV + j] = left[i];
    }
  }
}




/**
 * MODE PREDICTION
 * ************************************************************/
static __inline
imode4_t taa_h264_predict_intra_4x4_mode (
  uint8_t * intra_modes,
  int       stride,
  bool      block_left,                                   /* true if 4x4 to left is within same mb */
  bool      block_top,                                     /* true if 4x4 above is within same mb */
  bool      mb_left,                                   /* true if left mb is available and I4x4 */
  bool      mb_top)                                     /* true if top mb is available and I4x4 */
{
  bool dc_only    = !(block_top   || mb_top)  || !(block_left || mb_left);
  int imode_left = dc_only ? DC_PRED : intra_modes [block_left ? -1 : -13];
  int imode_top   = dc_only ? DC_PRED : intra_modes [block_top   ? -4 : -stride + 12];
  return (imode4_t) min (imode_left, imode_top);
}

mv_t taa_h264_predict_motion_vector (
  mv_t * mvs,
  int    mvs_stride,
  int    block_num,
  int    partshape_y,
  int    partshape_x,
  int    ref_num,
  bool   avail_left,
  bool   avail_top,
  bool   avail_topright,
  bool   avail_topleft);

/**
 * NUMBER OF COEFFSIENTS PREDICTION
 * ************************************************************/
static __inline
int taa_h264_predict_nonzero_coeffs (
  const uint8_t * num_coeffs,
  int             stride,
  bool            first_col,
  bool            first_row,
  bool            left_available,
  bool            up_available,
  bool            chroma)
{
  int left_offset;
  int top_offset;
  int pred_nnz = 0;

  if (chroma)
  {
    left_offset = first_col ? -7 : -1;
    top_offset  = first_row ? -stride + 2 : -2;
  }
  else
  {
    left_offset = first_col ? -13 : -1;
    top_offset  = first_row ? -stride + 12 : -4;
  }

  if (left_available)
    pred_nnz += num_coeffs[left_offset];

  if (up_available)
    pred_nnz += num_coeffs[top_offset];

  if (left_available && up_available)
    pred_nnz = (pred_nnz + 1) >> 1;

  return pred_nnz;
}



#endif
