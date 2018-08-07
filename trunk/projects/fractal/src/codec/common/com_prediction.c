#include "com_prediction.h"
#include "com_mbutils.h"
#include "com_intmath.h"

#include <string.h>

/* TODO: This function really need some cleanup. Would be nice with less * if-tests. */
mv_t taa_h264_predict_motion_vector (
  mv_t * mvs,                                       /* Pointer to position in array of motion vectors for this partition. */
  int    mvs_stride,                                    /* Stride of the mvs array */
  int    block_num,                                     /* 4x4 block number of top left block */
  int    partshape_y,                                   /* The partition height */
  int    partshape_x,                                   /* The partition width */
  int    ref_num,                                       /* The reference number for frame to predict from */
  bool   block_left,
  bool   block_top,
  bool   block_topright,
  bool   block_topleft)
{
  mv_t mv_null = { 0, 0, .ref_num = -1};
  mv_t median_mv;
  mv_t * ret = NULL;

  bool block_a_available = block_left;
  bool block_b_available = block_top;
  bool block_c_available = block_topright;
  bool block_d_available = block_topleft;

  int offset_a;
  int offset_b;
  int offset_c;
  int offset_d;

  if (block_num % 4)
    offset_a = -1;
  else
    offset_a = -13;

  if (block_num / 4)
    offset_b = -4;
  else
    offset_b = -mvs_stride + 12;

  if (block_num % 4)
    offset_d = -5;
  else
    offset_d = -17;

  if (block_num / 4 == 0)
    offset_d = offset_b - 1;
  if (block_num == 0)
    offset_d = -mvs_stride - 1;

  int block_num2 = block_num + partshape_x / 4 - 1;
  if (block_num2 / 4)
    offset_c = -3;
  else
    offset_c = -mvs_stride + 13;
  if (block_num2 == 3)
    offset_c = -mvs_stride + 16 + 9;

  offset_c += block_num2 - block_num;

  if (block_c_available == false)
  {
    block_c_available = block_d_available;
    offset_c = offset_d;
  }

  mv_t * mv_a = block_a_available ? &mvs[block_num + offset_a] : &mv_null;
  mv_t * mv_b = block_b_available ? &mvs[block_num + offset_b] : &mv_null;
  mv_t * mv_c = block_c_available ? &mvs[block_num + offset_c] : &mv_null;

  /* Check if ref num is the same. This also checks if block is nonintra
   * since ref_num == -1 for intra blocks */
  bool avail_left = block_a_available && (mv_a->ref_num == ref_num);
  bool avail_top = block_b_available && (mv_b->ref_num == ref_num);
  bool avail_topright = block_c_available && (mv_c->ref_num == ref_num);

  int num_equal_refs =
    (avail_left == true) +
    (avail_top == true) +
    (avail_topright == true);

  if (num_equal_refs == 1)
  {
    /* Only one block has same ref, return that MV */
    if (avail_left)
      ret = mv_a;
    else if (avail_top)
      ret = mv_b;
    else
      ret = mv_c;
  }
  else if(partshape_x == 8 && partshape_y == 16)
  {
    if (block_num == 0)
    {
      if (avail_left)
        ret = mv_a;
    }
    else
    {
      if (avail_topright)
        ret = mv_c;
    }
  }
  else if(partshape_x == 16 && partshape_y == 8)
  {
    if (block_num == 0)
    {
      if (avail_top)
        ret = mv_b;
    }
    else
    {
      if (avail_left)
        ret = mv_a;
    }
  }

  if (block_b_available == false && block_c_available == false
      && block_a_available == true && mv_a->ref_num >= 0 && ret == NULL)
  {
    /* If only block_a_available, mv_a is set as predictor no matter the reference. */
    ret = mv_a;
  }

  if (ret == NULL)
  {
    /* Median filtering */
    ret = &median_mv;
    ret->y = (int16_t) taa_h264_median (mv_a->y, mv_b->y, mv_c->y);
    ret->x = (int16_t) taa_h264_median (mv_a->x, mv_b->x, mv_c->x);
    ret->ref_num = (int8_t) ref_num;
  }

  return *ret;
}
