#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#include "com_compatibility.h"
#include <emmintrin.h>

#if 0
static
void taa_h264_inverse_transform_4x4_dec_C (
  const short * coeff,
  short *       block)
{
  /* Section 8.5.10 */
  /* Horizontal transform */
  int tmp0, tmp1, tmp2, tmp3;
  for (int i = 0; i < 4; i++)
  {
    tmp0 =  coeff[i * 4 + 0]       + coeff[i * 4 + 2];
    tmp1 =  coeff[i * 4 + 0]       - coeff[i * 4 + 2];
    tmp2 = (coeff[i * 4 + 1] >> 1) - coeff[i * 4 + 3];
    tmp3 =  coeff[i * 4 + 1]       + (coeff[i * 4 + 3] >> 1);

    block[i * 4 + 0] = (short) (tmp0 + tmp3);
    block[i * 4 + 1] = (short) (tmp1 + tmp2);
    block[i * 4 + 2] = (short) (tmp1 - tmp2);
    block[i * 4 + 3] = (short) (tmp0 - tmp3);
  }

  /* Vertical transform */
  for (int i = 0; i < 4; i++)
  {
    tmp0 =  block[0 * 4 + i]       + block[2 * 4 + i];
    tmp1 =  block[0 * 4 + i]       - block[2 * 4 + i];
    tmp2 = (block[1 * 4 + i] >> 1) - block[3 * 4 + i];
    tmp3 =  block[1 * 4 + i]       + (block[3 * 4 + i] >> 1);

    block[0 * 4 + i] = (short) ((tmp0 + tmp3 + 32) >> 6);
    block[1 * 4 + i] = (short) ((tmp1 + tmp2 + 32) >> 6);
    block[2 * 4 + i] = (short) ((tmp1 - tmp2 + 32) >> 6);
    block[3 * 4 + i] = (short) ((tmp0 - tmp3 + 32) >> 6);
  }
}
#endif

static const short TAA_H264_ALIGN(64) c[8][8] =
{
  { 2,  1,  2,  1,  2,  1,  2,  1},
  { 1,  1,  1,  1, -1, -1, -1, -1},
  {-1,  1, -1,  1,  1, -1,  1, -1},
  { 2, -1,  2, -1,  2, -1,  2, -1},
  { 1,  1,  1,  1,  1,  1,  1,  1},
  {-1, -1, -1, -1, -1, -1, -1, -1},
  {-1,  1, -1,  1, -1,  1, -1,  1},
  { 1, -1,  1, -1,  1, -1,  1, -1}
};

static const int TAA_H264_ALIGN(64) l[4] =
{
  1, 1, 1, 1
};

static const short TAA_H264_ALIGN(64) offset[8] =
{
  32, 32, 32, 32, 32, 32, 32, 32
};

static
void taa_h264_inverse_transform_4x4_dec (
  const short * coeff,
  short *       block)
{
  __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;

  xmm0 = _mm_shuffle_epi32(*(__m128i *)&coeff[0], _MM_SHUFFLE(2, 0, 3, 1));
  xmm1 = _mm_shuffle_epi32(*(__m128i *)&coeff[0], _MM_SHUFFLE(3, 1, 2, 0));
  xmm2 = _mm_srai_epi32(_mm_madd_epi16(xmm0, (*(__m128i *) c[0])), 1);
  xmm3 = _mm_madd_epi16(xmm1, (*(__m128i *) c[1]));
  xmm4 = _mm_madd_epi16(xmm0, (*(__m128i *) c[2]));
  xmm5 = _mm_srai_epi32(_mm_add_epi32(_mm_madd_epi16(xmm1, (*(__m128i *) c[3])), (*(__m128i *) l)), 1);
  xmm6 = _mm_packs_epi32(_mm_add_epi32(xmm2, xmm3), _mm_add_epi32(xmm4, xmm5));

  xmm0 =  _mm_shuffle_epi32(*(__m128i *)&coeff[8], _MM_SHUFFLE(2, 0, 3, 1));
  xmm1 =  _mm_shuffle_epi32(*(__m128i *)&coeff[8], _MM_SHUFFLE(3, 1, 2, 0));
  xmm2 = _mm_srai_epi32(_mm_madd_epi16(xmm0, (*(__m128i *) c[0])), 1);
  xmm3 = _mm_madd_epi16(xmm1, (*(__m128i *) c[1]));
  xmm4 = _mm_madd_epi16(xmm0, (*(__m128i *) c[2]));
  xmm5 = _mm_srai_epi32(_mm_add_epi32(_mm_madd_epi16(xmm1, (*(__m128i *) c[3])), (*(__m128i *) l)), 1);
  xmm7 = _mm_packs_epi32(_mm_add_epi32(xmm2, xmm3), _mm_add_epi32(xmm4, xmm5));

  xmm1 = _mm_madd_epi16(xmm6, (*(__m128i *) c[4]));
  xmm2 = _mm_srai_epi32(_mm_madd_epi16(xmm7, (*(__m128i *) c[0])), 1);
  xmm3 = _mm_srai_epi32(_mm_madd_epi16(xmm6, (*(__m128i *) c[0])), 1);
  xmm4 = _mm_madd_epi16(xmm7, (*(__m128i *) c[5]));
  xmm5 = _mm_packs_epi32(_mm_add_epi32(xmm1, xmm2), _mm_add_epi32(xmm3, xmm4));
  xmm5 = _mm_srai_epi16(_mm_add_epi16(xmm5, (*(__m128i *) offset)), 6);
  _mm_store_si128((__m128i *)&block[0], xmm5);

  xmm1 = _mm_srai_epi32(_mm_add_epi32(_mm_madd_epi16(xmm6, (*(__m128i *) c[3])), (*(__m128i *) l)), 1);
  xmm2 = _mm_madd_epi16(xmm7, (*(__m128i *) c[6]));
  xmm3 = _mm_madd_epi16(xmm6, (*(__m128i *) c[7]));
  xmm4 = _mm_srai_epi32(_mm_add_epi32(_mm_madd_epi16(xmm7, (*(__m128i *) c[3])), (*(__m128i *) l)), 1);
  xmm5 = _mm_packs_epi32(_mm_add_epi32(xmm1, xmm2), _mm_add_epi32(xmm3, xmm4));
  xmm5 = _mm_srai_epi16(_mm_add_epi16(xmm5, (*(__m128i *) offset)), 6);
  _mm_store_si128((__m128i *)&block[8], xmm5);
}


static
void taa_h264_inverse_hadamard_and_dequant_4x4 (
  short * block,
  int     qp)
{
  int tmp0, tmp1, tmp2, tmp3;

  /* These entries correspond to the dequant table in quant.c for the DC
   * position and shifted with "qp / 6" */
  static const int dequant_coef_dc[52] = {
    10 << 0,  11 << 0,  13 << 0,  14 << 0,  16 << 0,  18 << 0,
    10 << 1,  11 << 1,  13 << 1,  14 << 1,  16 << 1,  18 << 1,
    10 << 2,  11 << 2,  13 << 2,  14 << 2,  16 << 2,  18 << 2,
    10 << 3,  11 << 3,  13 << 3,  14 << 3,  16 << 3,  18 << 3,
    10 << 4,  11 << 4,  13 << 4,  14 << 4,  16 << 4,  18 << 4,
    10 << 5,  11 << 5,  13 << 5,  14 << 5,  16 << 5,  18 << 5,
    10 << 6,  11 << 6,  13 << 6,  14 << 6,  16 << 6,  18 << 6,
    10 << 7,  11 << 7,  13 << 7,  14 << 7,  16 << 7,  18 << 7,
    10 << 8,  11 << 8,  13 << 8,  14 << 8
  };

  /* int qp_div_6 = qp / 6; */
  /* int qp_mod_6 = qp % 6; */
  /* int dqc = dequant_coef[qp_mod_6][0] << qp_div_6; */
  int dqc = dequant_coef_dc[qp];

  /* Horizontal transform */
  for (int i = 0; i < 4; i++)
  {
    tmp0 = block[i * 4 + 0] + block[i * 4 + 2];
    tmp1 = block[i * 4 + 0] - block[i * 4 + 2];
    tmp2 = block[i * 4 + 1] - block[i * 4 + 3];
    tmp3 = block[i * 4 + 1] + block[i * 4 + 3];

    block[i * 4 + 0] = (short) (tmp0 + tmp3);
    block[i * 4 + 1] = (short) (tmp1 + tmp2);
    block[i * 4 + 2] = (short) (tmp1 - tmp2);
    block[i * 4 + 3] = (short) (tmp0 - tmp3);
  }

  /* Vertical transform and dequantization */
  for (int i = 0; i < 4; i++)
  {
    tmp0 = block[0 * 4 + i] + block[2 * 4 + i];
    tmp1 = block[0 * 4 + i] - block[2 * 4 + i];
    tmp2 = block[1 * 4 + i] - block[3 * 4 + i];
    tmp3 = block[1 * 4 + i] + block[3 * 4 + i];

    block[0 * 4 + i] = (short) (((tmp0 + tmp3) * dqc + 2) >> 2);
    block[1 * 4 + i] = (short) (((tmp1 + tmp2) * dqc + 2) >> 2);
    block[2 * 4 + i] = (short) (((tmp1 - tmp2) * dqc + 2) >> 2);
    block[3 * 4 + i] = (short) (((tmp0 - tmp3) * dqc + 2) >> 2);
  }
}

static
void taa_h264_inverse_hadamard_2x2_dec (
  short * coeff)
{
  const int block_size = 4 * 4;

  int c1 = coeff[block_size * 0];
  int c2 = coeff[block_size * 1];
  int c3 = coeff[block_size * 2];
  int c4 = coeff[block_size * 3];

  coeff [block_size * 0] = (short) ((c1 + c2 + c3 + c4) >> 1);
  coeff [block_size * 1] = (short) ((c1 - c2 + c3 - c4) >> 1);
  coeff [block_size * 2] = (short) ((c1 + c2 - c3 - c4) >> 1);
  coeff [block_size * 3] = (short) ((c1 - c2 - c3 + c4) >> 1);
}


#endif
