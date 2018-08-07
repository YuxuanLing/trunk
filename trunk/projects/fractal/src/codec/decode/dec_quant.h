/* -*- indent-tabs-mode:nil; c-basic-offset:4 -*- */
#ifndef _QUANT_H_
#define _QUANT_H_

#include "com_common.h"

static const short dequant_coef[6][16] = {
  { 10, 13, 13, 10, 16, 10, 13, 13, 13, 13, 16, 10, 16, 13, 13, 16},
  { 11, 14, 14, 11, 18, 11, 14, 14, 14, 14, 18, 11, 18, 14, 14, 18},
  { 13, 16, 16, 13, 20, 13, 16, 16, 16, 16, 20, 13, 20, 16, 16, 20},
  { 14, 18, 18, 14, 23, 14, 18, 18, 18, 18, 23, 14, 23, 18, 18, 23},
  { 16, 20, 20, 16, 25, 16, 20, 20, 20, 20, 25, 16, 25, 20, 20, 25},
  { 18, 23, 23, 18, 29, 18, 23, 23, 23, 23, 29, 18, 29, 23, 23, 29}
};

static const short zz_dequant_coef[6][16] = {
  { 10, 13, 10, 13, 13, 16, 13, 16, 10, 13, 10, 13, 13, 16, 13, 16},
  { 11, 14, 11, 14, 14, 18, 14, 18, 11, 14, 11, 14, 14, 18, 14, 18},
  { 13, 16, 13, 16, 16, 20, 16, 20, 13, 16, 13, 16, 16, 20, 16, 20},
  { 14, 18, 14, 18, 18, 23, 18, 23, 14, 18, 14, 18, 18, 23, 18, 23},
  { 16, 20, 16, 20, 20, 25, 20, 25, 16, 20, 16, 20, 20, 25, 20, 25},
  { 18, 23, 18, 23, 23, 29, 23, 29, 18, 23, 18, 23, 23, 29, 23, 29}
};

static const int qp_scale_chroma[52] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
  12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
  28, 29, 29, 30, 31, 32, 32, 33, 34, 34, 35, 35, 36, 36, 37, 37,
  37, 38, 38, 38, 39, 39, 39, 39
};


static int const zagzig[16] =
{
  0, 1, 5, 6, 2, 4, 7, 12, 3, 8, 11, 13, 9, 10, 14, 15
};

static
void taa_h264_dequantize_4x4 (
  const short *    qcoeff,                         /* Quantiz ed coeffs in zig-zag order */
  short * restrict dqcoeff,                         /* Out: Dequantized coeffs in raster scan order */
  int              qpy,                              /* QP for luma */
  int              chroma,                           /* True if called for chroma */
  int              ac,                               /* True if called for AC components. */
  uint8_t          chroma_qp_index_offset)
{
  TAA_H264_ALIGN(64) short zz_qcoeff[16];

  const int qp = chroma ? qp_scale_chroma[qpy] + chroma_qp_index_offset: qpy;
  const int quant_div_6 = (qp * 171) >> 10;
  const int quant_mod_6 = qp - quant_div_6 * 6;
  const short save = dqcoeff[0];

#pragma unroll(16)
  for (int x = 0; x < 16; x++)
    zz_qcoeff[x] = qcoeff[zagzig[x]];

#pragma ivdep
#pragma vector aligned
  for (int x = 0; x < 16; x++)
    dqcoeff[x] = (short) ((zz_qcoeff[x] * zz_dequant_coef[quant_mod_6][x])<< quant_div_6);

  if (ac) dqcoeff[0] = save;
}

static
void taa_h264_dequantize_chroma_dc (
  const short *    qcoeff,
  short * restrict dqcoeff,
  int              qpy,
  uint8_t          chroma_qp_index_offset)
{
  int qpc = qp_scale_chroma[qpy] + chroma_qp_index_offset;
  int qp_div_6 = (qpc * 171) >> 10;
  int qp_mod_6 = qpc - qp_div_6 * 6;
  int dqc = dequant_coef[qp_mod_6][0];

  dqcoeff [16 * 0] = (short) ((qcoeff[0] * dqc) << qp_div_6);
  dqcoeff [16 * 1] = (short) ((qcoeff[1] * dqc) << qp_div_6);
  dqcoeff [16 * 2] = (short) ((qcoeff[2] * dqc) << qp_div_6);
  dqcoeff [16 * 3] = (short) ((qcoeff[3] * dqc) << qp_div_6);
}

#endif
