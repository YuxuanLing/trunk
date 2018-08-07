#ifndef _FAST_OBJECT_MOTION_H_
#define _FAST_OBJECT_MOTION_H_

#include "com_intmath.h"
#include "com_common.h"

#include <emmintrin.h>

typedef struct
{
  mv_t vector_outer;
  mv_t vector;
} fast_t;

#ifndef DISABLE_FAST_OBJECT_MOTION
static
fast_t taa_h264_fast_object_motion(
  const int     yp,
  const int     height,
  const int     stride,
  const int16_t motion_vector_x[],
  const int16_t motion_vector_y[])
{
  fast_t fast = {0, };
  int16_t vmax_inner = 16;   // We like motion vectors in the rane 16....64
  int16_t vmax_outer = 64;   // We like motion vectors in the rane 64...128
  //const int ww = 8;
  const int wh = 4;
  const int ts = max(-yp>>4, -wh);
  const int bs = min((height-yp-16)>>4, wh);
  //const int ls = max(-xp>>4, -ww);
  //const int rs = min((width-xp-16)>>4, ww);

  for(int y = ts; y<=bs; y++)
  {
    int16_t xpos[8], ypos[8], xvec[8], yvec[8], xdif[8], ydif[8], outer_cmp[8], inner_cmp[8];
    TAA_H264_ALIGN(64) int16_t vlen_inner[8] = {0, };
    TAA_H264_ALIGN(64) int16_t vlen_outer[8] = {0, };

    for(int x = 0; x<8; x++)
    {
      xpos[x] = (int16_t) ((x-4) <<6);
      ypos[x] = (int16_t) ((y  ) <<6);
      xvec[x] = motion_vector_x[y*stride-4+x];
      yvec[x] = motion_vector_y[y*stride-4+x];
    }
    for(int x = 0; x<8; x++)
    {
      xdif[x] = (int16_t)(xvec[x] - xpos[x]);
      ydif[x] = (int16_t)(yvec[x] - ypos[x]);
    }
    for(int x = 0; x<8; x++)
    {
      xdif[x] = (int16_t) abs(xdif[x]);
      ydif[x] = (int16_t) abs(ydif[x]);
    }
    for(int x = 0; x<8; x++)
    {
      inner_cmp[x] = (int16_t)((xdif[x] < ( 64+16)) & (ydif[x] < ( 64+16)));
      outer_cmp[x] = (int16_t)((xdif[x] < (128+32)) & (ydif[x] < (128+32)));
    }
    for(int x = 0; x<8; x++)
    {
      if (outer_cmp[x])
        vlen_outer[x] = (int16_t) (abs(xvec[x]) + abs(yvec[x]));
      if (inner_cmp[x])
        vlen_inner[x] = (int16_t) (abs(xvec[x]) + abs(yvec[x]));
    }
    uint16_t mask = (uint16_t)_mm_movemask_epi8(_mm_packs_epi16(_mm_cmpgt_epi16(*(__m128i *)vlen_outer, _mm_set1_epi16(vmax_outer)), _mm_cmpgt_epi16(*(__m128i *)vlen_inner, _mm_set1_epi16(vmax_inner))));
    uint16_t mask_outer = (uint16_t)(mask & 0xff);
    int idx_outer = 0;
    while(mask_outer)
    {
      const int run = _bit_scan_forward(mask_outer);
      idx_outer = idx_outer + run + 1;
      mask_outer = (uint16_t) (mask_outer >> (run+1));
      if (vlen_outer[idx_outer-1]>vmax_outer)
      {
        vmax_outer = vlen_outer[idx_outer-1];
        fast.vector_outer.x = xvec[idx_outer-1];
        fast.vector_outer.y = yvec[idx_outer-1];
      }
    }

    uint16_t mask_inner = (uint16_t)(mask >> 8);
    int idx_inner = 0;
    while(mask_inner)
    {
      const int run = _bit_scan_forward(mask_inner);
      idx_inner = idx_inner + run + 1;
      mask_inner = (uint16_t) (mask_inner >> (run+1));
      if (vlen_inner[idx_inner-1]>vmax_inner)
      {
        vmax_inner = vlen_inner[idx_inner-1];
        fast.vector.x = xvec[idx_inner-1];
        fast.vector.y = yvec[idx_inner-1];
      }
    }
  }
  return(fast);
}
#endif

#endif
