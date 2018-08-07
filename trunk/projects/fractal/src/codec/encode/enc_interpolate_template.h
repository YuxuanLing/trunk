/* ================================================================================= */
/* */
/*                               Copyright Notice */
/* */
/*  All material herein: Copyright © 2009 TANDBERG Telecom AS. All Rights Reserved. */
/*  This product is protected under US patents. Other patents pending. */
/*  Further information can be found at http://www.tandberg.com/tandberg_pm.jsp. */
/* */
/*  The source code is owned by TANDBERG Telecom AS and is protected by copyright */
/*  laws and international copyright treaties, as well as other intellectual */
/*  property laws and treaties. All right, title and interest in the source code */
/*  are owned TANDBERG Telecom AS. Therefore, you must treat the source code like */
/*  any other copyrighted material. */
/* */
/* ================================================================================= */

#include "enc_config.h"

#ifndef AVG_HPEL

#include "com_compatibility.h"
#include <string.h>

/*
 *   +--------------------------------------------------+
 *   |                                                  |
 *   |C                             padding, 32 pixels  |
 *   |                                                  |
 *   |A       +-------------------------------+         |
 *   |        |                               |         |
 *   |        |                               |         |
 *   |        |                               |         |
 *   |        |     picture                   |         |
 *   |        |                               |         |
 *   |        |                               |         |
 *   |        |                               |         |
 *   |        |                               |         |
 *   |        +-------------------------------+        B|
 *   |                                                  |
 *   |                                                 D|
 *   |                                                  |
 *   +--------------------------------------------------+
 *
 * Input arguments:
 * - ph = A
 * - pv = C (3 pixels above A)
 * - qh = pos A in half hor
 * - qv = pos C in half vert
 */

static void TAA_H264_HPEL_INTERPOLATION(
#if !defined(H) && !defined(W)
  const int     H,
  const int     W,
#endif
  const uint8_t ph[],
  const uint8_t pv[],
  uint8_t       qd[],
  uint8_t       qh[],
  uint8_t       qv[])
{
  uint8_t C[16];
  int16_t temp[16];
  __assume_aligned(qd, 16);
  __assume_aligned(qh, 16);
  __assume_aligned(qv, 16);
  //=======================================================================
  //                            HORIZONTAL VPEL
  //=======================================================================
  {
    const int a[6] = { -2, -1, 0, 1, 2, 3};
    const uint8_t * p0 = ph - 2;
    const uint8_t * p1 = ph + a[1];
    const uint8_t * p2 = ph + a[2];
    const uint8_t * p3 = ph + a[3];
    const uint8_t * p4 = ph + a[4];
    const uint8_t * p5 = ph + a[5];

    /* Horizontal interpolation from point A to B */
    for (int i = 0; i < H * W; i += 16)
    {
#pragma unroll(2)
      for(int k = 0; k < 16; k++)
        temp[k] = (int16_t)(20 * (p2[i + k] + p3[i + k]) + (-5) * (p1[i + k] + p4[i + k]) + p0[i + k] + p5[i + k] + 16);
#pragma unroll(2)
      for(int k = 0; k < 16; k++)
        temp[k] = (int16_t)(temp[k] >> 5);
      for(int k = 0; k < 16; k++)
        qh[i + k] = (uint8_t)((temp[k] > 255) ? 255 : (temp[k] < 0) ? 0 : temp[k]);
    }
    /* Overwriting the leftmost and rightmost interpolated values, since they
     * give wrong result in the above loop (they are using values from next/previous line
     * instead of padding). */
    for (int i = 0; i < H; i++)
    {
      for(int k = 0; k < 16; k++)
        C[k] = qh[(i + 0) * W + 7  ];
      memcpy(qh + (i + 0) * W, C, 8);
      for(int k = 0; k < 16; k++)
        C[k] = qh[(i + 1) * W - 7 - 1];
      memcpy(qh + (i + 1) * W - 7, C, 8);
    }
    /* Padding top and bottom. */
    for(int i = 0; i < PAD_VERT_Y; i++)
    {
      for(int j = 0; j < W; j += 16)
      {
        for(int k = 0; k < 16; k++)
        {
          qh[j + (i - PAD_VERT_Y) * W + k] = qh[j + k];
          qh[j + (i + H) * W + k]  = qh[j + (H - 1) * W + k];
        }
      }
    }
  }

  {
    /* Vertical interpolation from point C to D */
    for (int i = 0; i < H + 6; i++)
    {
      //=======================================================================
      //                            VERTICAL HPEL
      //=======================================================================
      TAA_H264_ALIGN(64) int32_t buff[16];
      TAA_H264_ALIGN(64) int16_t vpel[W+32];

      const int a[6] = { -2, -1, 0, 1, 2, 3};
      const int16_t * p0 = &vpel[16] - 2;
      const int16_t * p1 = &vpel[16] + a[1];
      const int16_t * p2 = &vpel[16] + a[2];
      const int16_t * p3 = &vpel[16] + a[3];
      const int16_t * p4 = &vpel[16] + a[4];
      const int16_t * p5 = &vpel[16] + a[5];

      for (int j = 0; j < W; j += 16)
      {
#pragma unroll(2)
        for(int k = 0; k < 16; k++)
          temp[k] = (int16_t)(20 * (pv[(i + 0) * W + j + k] + pv[(i + 1) * W + j + k]) + (-5) * (pv[(i - 1) * W + j + k] + pv[(i + 2) * W + j + k]) + pv[(i - 2) * W + j + k] + pv[(i + 3) * W + j + k]);
#pragma unroll(2)
        for(int k = 0; k < 16; k++)
          vpel[k+j+16] = temp[k];
#pragma unroll(2)
        for(int k = 0; k < 16; k++)
          temp[k] = (int16_t)(temp[k] + 16);
#pragma unroll(2)
        for(int k = 0; k < 16; k++)
          temp[k] = (int16_t)(temp[k] >> 5);
        for(int k = 0; k < 16; k++)
          qv[i * W + j + k] = (uint8_t)((temp[k] > 255) ? 255 : (temp[k] < 0) ? 0 : temp[k]);
      }
      /* Overwriting leftmost and rightmost values. Same reason as for
       * horizontal interpolation. */
      for(int k = 0; k < 16; k++)
        C[k] = qv[(i + 0) * W + 7  ];
      memcpy(qv + (i + 0) * W, C, 8);
      for(int k = 0; k < 16; k++)
        C[k] = qv[(i + 1) * W - 7 - 1];
      memcpy(qv + (i + 1) * W - 7, C, 8);

      //=======================================================================
      //                            DIAGONAL HPEL
      //=======================================================================
      for(int k = 0; k < 16; k++)
        vpel[k] = vpel[16];
      for(int k = 0; k < 16; k++)
        vpel[W+16+k] = vpel[W+15];
      for (int j = 0; j < W; j += 16)
      {
#pragma unroll(4)
        for(int k = 0; k < 16; k++)
          buff[k]  = p0[j + k];
#pragma unroll(4)
        for(int k = 0; k < 16; k++)
          buff[k] += p1[j + k] * (-5);
#pragma unroll(4)
        for(int k = 0; k < 16; k++)
          buff[k] += p2[j + k] * 20;
#pragma unroll(4)
        for(int k = 0; k < 16; k++)
          buff[k] += p3[j + k] * 20;
#pragma unroll(4)
        for(int k = 0; k < 16; k++)
          buff[k] += p4[j + k] * (-5);
#pragma unroll(4)
        for(int k = 0; k < 16; k++)
          buff[k] += p5[j + k];
#pragma unroll(4)
        for(int k = 0; k < 16; k++)
          buff[k] += 512;
#pragma unroll(4)
        for(int k = 0; k < 16; k++)
          buff[k] = buff[k] >> 10;
#pragma unroll(2)
        for(int k = 0; k < 16; k++)
          temp[k] = (int16_t) ((buff[k] > 32767) ? 32767 : (buff[k] < -32768) ? -32768 : buff[k]);
        for(int k = 0; k < 16; k++)
          qd[i * W + j + k] = (uint8_t)((temp[k] > 255) ? 255 : (temp[k] < 0) ? 0 : temp[k]);
      }
      for(int k = 0; k < 16; k++)
        C[k] = qd[(i + 0) * W + 7  ];
      memcpy(qd + (i + 0) * W, C, 8);
      for(int k = 0; k < 16; k++)
        C[k] = qd[(i + 1) * W - 7 - 1];
      memcpy(qd + (i + 1) * W - 7, C, 8);
    }
    /* Padding the rest of the top and bottom (PAD_VERT_Y-3 rows are remaining).*/
    for(int i = 0; i < PAD_VERT_Y - 3; i++)
    {
      for(int j = 0; j < W; j += 16)
      {
        for(int k = 0; k < 16; k++)
        {
          qv[j + i * W - (PAD_VERT_Y - 3) * W + k] = qv[j + k];
          qv[j + i * W + 6 * W + H * W + k]  = qv[j + (H - 1) * W + 6 * W + k];
        }
      }
    }
    for(int i = 0; i < PAD_VERT_Y - 3; i++)
    {
      for(int j = 0; j < W; j += 16)
      {
        for(int k = 0; k < 16; k++)
        {
          qd[j + i * W - (PAD_VERT_Y - 3) * W + k] = qd[j + k];
          qd[j + i * W + 6 * W + H * W + k]  = qd[j + (H - 1) * W + 6 * W + k];
        }
      }
    }
  }
}

#endif
