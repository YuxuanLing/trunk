/* ================================================================================= */
/*                                                                                   */
/*                               Copyright Notice                                    */
/*                                                                                   */
/*  All material herein: Copyright © 2009 TANDBERG Telecom AS. All Rights Reserved.  */
/*  This product is protected under US patents. Other patents pending.               */
/*  Further information can be found at http://www.tandberg.com/tandberg_pm.jsp.     */
/*                                                                                   */
/*  The source code is owned by TANDBERG Telecom AS and is protected by copyright    */
/*  laws and international copyright treaties, as well as other intellectual         */
/*  property laws and treaties. All right, title and interest in the source code     */
/*  are owned TANDBERG Telecom AS. Therefore, you must treat the source code like    */
/*  any other copyrighted material.                                                  */
/*                                                                                   */
/* ================================================================================= */
#include "enc_config.h"
#include "enc_mb.h"
#include "com_mbutils.h"
#include "com_intmath.h"
#include "com_prediction.h"
#include <limits.h>
#include <emmintrin.h>

#pragma warning (disable:981)


/********************* QUANTIZATION *****************************/

static const int16_t TAA_H264_ALIGN(64) quant_coef[6][16] =
{
  /* Listed in zig-zag scan order, (0.0) (0.1) (1.0) (2.0) (1.1) ... */
  { 13107, 8066, 8066, 13107, 5243, 13107, 8066, 8066, 8066, 8066, 5243, 13107, 5243, 8066, 8066, 5243},
  { 11916, 7490, 7490, 11916, 4660, 11916, 7490, 7490, 7490, 7490, 4660, 11916, 4660, 7490, 7490, 4660},
  { 10082, 6554, 6554, 10082, 4194, 10082, 6554, 6554, 6554, 6554, 4194, 10082, 4194, 6554, 6554, 4194},
  { 9362, 5825, 5825,  9362, 3647,  9362, 5825, 5825, 5825, 5825, 3647,  9362, 3647, 5825, 5825, 3647},
  { 8192, 5243, 5243,  8192, 3355,  8192, 5243, 5243, 5243, 5243, 3355,  8192, 3355, 5243, 5243, 3355},
  { 7282, 4559, 4559,  7282, 2893,  7282, 4559, 4559, 4559, 4559, 2893,  7282, 2893, 4559, 4559, 2893}
};

static const int16_t TAA_H264_ALIGN(64) dequant_coef[6][16] =
{
  { 10, 13, 13, 10, 16, 10, 13, 13, 13, 13, 16, 10, 16, 13, 13, 16},
  { 11, 14, 14, 11, 18, 11, 14, 14, 14, 14, 18, 11, 18, 14, 14, 18},
  { 13, 16, 16, 13, 20, 13, 16, 16, 16, 16, 20, 13, 20, 16, 16, 20},
  { 14, 18, 18, 14, 23, 14, 18, 18, 18, 18, 23, 14, 23, 18, 18, 23},
  { 16, 20, 20, 16, 25, 16, 20, 20, 20, 20, 25, 16, 25, 20, 20, 25},
  { 18, 23, 23, 18, 29, 18, 23, 23, 23, 23, 29, 18, 29, 23, 23, 29}
};

static const int qp_scale_chroma[52] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
  12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
  28, 29, 29, 30, 31, 32, 32, 33, 34, 34, 35, 35, 36, 36, 37, 37,
  37, 38, 38, 38, 39, 39, 39, 39
};

static const int Q_BITS = 15;
/* static const int MAX_QP = 51; */
static const int MAX_Q_OFFSET = 2796202;    /* (1 << (Q_BITS + (MAX_QP/6))) / 3 */
static const int MAX_Q_OFFSET_DC = 5592404; /* MAX_Q_OFFSET << 1; */
#define COEFF_COST_LUMA_8X8_TH 4
#define COEFF_COST_LUMA_16X16_TH 5
#define COEFF_COST_CHROMA_TH 3
#define COEFF_COST_MAX 99999

static int16_t const TAA_H264_ALIGN(64) quant[48] =
{
  13107, 8066, 13107, 8066, 8066, 5243, 8066, 5243,
  11916, 7490, 11916, 7490, 7490, 4660, 7490, 4660,
  10082, 6554, 10082, 6554, 6554, 4194, 6554, 4194,
  9362, 5825,  9362, 5825, 5825, 3647, 5825, 3647,
  8192, 5243,  8192, 5243, 5243, 3355, 5243, 3355,
  7282, 4559,  7282, 4559, 4559, 2893, 4559, 2893,
};

static int16_t const TAA_H264_ALIGN(64) de_quant[48] =
{
  10, 13, 10, 13, 13, 16, 13, 16,
  11, 14, 11, 14, 14, 18, 14, 18,
  13, 16, 13, 16, 16, 20, 16, 20,
  14, 18, 14, 18, 18, 23, 18, 23,
  16, 20, 16, 20, 20, 25, 20, 25,
  18, 23, 18, 23, 23, 29, 23, 29,
};

// zigzag to raster
static int8_t const TAA_H264_ALIGN(64) zagzig[16] =
{
  0, 1, 4, 8, 5, 2, 3, 6, 9, 12, 13, 10,  7, 11, 14, 15
};

static const int16_t TAA_H264_ALIGN(64) b[4][8] =
{
  { 1,  1,  1,  1,  2,  1,  2,  1},
  { 1,  1,  1,  1, -1, -2, -1, -2},
  {-1,  1, -1,  1,  1, -2,  1, -2},
  { 1, -1,  1, -1,  2, -1,  2, -1}
};

static const int16_t TAA_H264_ALIGN(64) x[8][8] =
{
  { 1,  1,  1,  1,  1,  1,  1,  1},
  { 1,  1,  1,  1,  1,  1,  1,  1},
  { 2,  1,  2,  1,  2,  1,  2,  1},
  {-1, -2, -1, -2, -1, -2, -1, -2},
  { 1, -1,  1, -1,  1, -1,  1, -1},
  {-1,  1, -1,  1, -1,  1, -1,  1},
  { 1, -2,  1, -2,  1, -2,  1, -2},
  { 2, -1,  2, -1,  2, -1,  2, -1}
};

#include "enc_quant_template.h"

static void
taa_h264_transform_4x4_x86_64 ( __m128i dct[2] )
{
  __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;

  xmm0 = _mm_shuffle_epi32(dct[0], _MM_SHUFFLE(2, 0, 3, 1));
  xmm1 = _mm_shuffle_epi32(dct[0], _MM_SHUFFLE(3, 1, 2, 0));
  xmm2 = _mm_madd_epi16(xmm0, (*(__m128i *) b[0]));
  xmm3 = _mm_madd_epi16(xmm1, (*(__m128i *) b[1]));
  xmm4 = _mm_madd_epi16(xmm0, (*(__m128i *) b[2]));
  xmm5 = _mm_madd_epi16(xmm1, (*(__m128i *) b[3]));
  xmm6 = _mm_packs_epi32(_mm_add_epi32(xmm2, xmm3), _mm_add_epi32(xmm4, xmm5));

  xmm0 =  _mm_shuffle_epi32(dct[1], _MM_SHUFFLE(2, 0, 3, 1));
  xmm1 =  _mm_shuffle_epi32(dct[1], _MM_SHUFFLE(3, 1, 2, 0));
  xmm2 = _mm_madd_epi16(xmm0, (*(__m128i *) b[0]));
  xmm3 = _mm_madd_epi16(xmm1, (*(__m128i *) b[1]));
  xmm4 = _mm_madd_epi16(xmm0, (*(__m128i *) b[2]));
  xmm5 = _mm_madd_epi16(xmm1, (*(__m128i *) b[3]));
  xmm7 = _mm_packs_epi32(_mm_add_epi32(xmm2, xmm3), _mm_add_epi32(xmm4, xmm5));

  xmm1 = _mm_madd_epi16(xmm6, (*(__m128i *) x[0]));
  xmm2 = _mm_madd_epi16(xmm7, (*(__m128i *) x[1]));
  xmm3 = _mm_madd_epi16(xmm6, (*(__m128i *) x[2]));
  xmm4 = _mm_madd_epi16(xmm7, (*(__m128i *) x[3]));
  xmm5 = _mm_packs_epi32(_mm_add_epi32(xmm1, xmm2), _mm_add_epi32(xmm3, xmm4));
  _mm_store_si128(&dct[0], xmm5);

  xmm1 = _mm_madd_epi16(xmm6, (*(__m128i *) x[4]));
  xmm2 = _mm_madd_epi16(xmm7, (*(__m128i *) x[5]));
  xmm3 = _mm_madd_epi16(xmm6, (*(__m128i *) x[6]));
  xmm4 = _mm_madd_epi16(xmm7, (*(__m128i *) x[7]));
  xmm5 = _mm_packs_epi32(_mm_add_epi32(xmm1, xmm2), _mm_add_epi32(xmm3, xmm4));
  _mm_store_si128(&dct[1], xmm5);
}

static const int16_t TAA_H264_ALIGN(64) c[8][8] =
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
static const int16_t TAA_H264_ALIGN(64) offset[8] =
{
  32, 32, 32, 32, 32, 32, 32, 32
};

static void
taa_h264_inverse_transform_4x4_x86_64 ( __m128i dct[2] )
{
  __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;

  xmm0 = _mm_shuffle_epi32(dct[0], _MM_SHUFFLE(2, 0, 3, 1));
  xmm1 = _mm_shuffle_epi32(dct[0], _MM_SHUFFLE(3, 1, 2, 0));
  xmm2 = _mm_srai_epi32(_mm_madd_epi16(xmm0, (*(__m128i *) c[0])), 1);
  xmm3 = _mm_madd_epi16(xmm1, (*(__m128i *) c[1]));
  xmm4 = _mm_madd_epi16(xmm0, (*(__m128i *) c[2]));
  xmm5 = _mm_srai_epi32(_mm_add_epi32(_mm_madd_epi16(xmm1, (*(__m128i *) c[3])), (*(__m128i *) l)), 1);
  xmm6 = _mm_packs_epi32(_mm_add_epi32(xmm2, xmm3), _mm_add_epi32(xmm4, xmm5));

  xmm0 =  _mm_shuffle_epi32(dct[1], _MM_SHUFFLE(2, 0, 3, 1));
  xmm1 =  _mm_shuffle_epi32(dct[1], _MM_SHUFFLE(3, 1, 2, 0));
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
  _mm_store_si128(&dct[0], xmm5);

  xmm1 = _mm_srai_epi32(_mm_add_epi32(_mm_madd_epi16(xmm6, (*(__m128i *) c[3])), (*(__m128i *) l)), 1);
  xmm2 = _mm_madd_epi16(xmm7, (*(__m128i *) c[6]));
  xmm3 = _mm_madd_epi16(xmm6, (*(__m128i *) c[7]));
  xmm4 = _mm_srai_epi32(_mm_add_epi32(_mm_madd_epi16(xmm7, (*(__m128i *) c[3])), (*(__m128i *) l)), 1);
  xmm5 = _mm_packs_epi32(_mm_add_epi32(xmm1, xmm2), _mm_add_epi32(xmm3, xmm4));
  xmm5 = _mm_srai_epi16(_mm_add_epi16(xmm5, (*(__m128i *) offset)), 6);
  _mm_store_si128(&dct[1], xmm5);
}

static const int coeff_cost_tab[16] = {3, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static int8_t
taa_h264_transform_quant_dequant_run_level(
  const int   qp_div_6,
  const int   qp_mod_6,
  const int   q_bits,
  const int   q_offset,
  __m128i     dct[2],                                     /* In: Transformed coeffs. Out: Dequantized trans coeffs. */
  coeffs4_t * coeffs4,
  int *       cost)                                 /* Out: The cost of encoding the coeffisients. */
{
  TAA_H264_ALIGN(64) uint8_t qcoeff[16];
  TAA_H264_ALIGN(64) int8_t ssign[16];
  taa_h264_transform_4x4_x86_64(dct);
  int max_level = 0;
  int lost = 0;
  int idx = 0;
  int cidx = 0;
#ifdef HAVE_SSSE3
  int flag = quant_dequant_ssse3(qp_div_6, qp_mod_6, q_bits, q_offset, dct, qcoeff, ssign);
  int mask = flag;
  while (mask)
  {
    const int run = _bit_scan_forward(mask);
    idx = idx + run + 1;
    mask = mask >> (run+1);
    coeffs4->level[cidx] = qcoeff[idx-1];
    coeffs4->run[cidx]   = (uint8_t) (run);
    coeffs4->sign[cidx]  = (int8_t) (-ssign[idx-1]);
    cidx++;
    max_level = max (qcoeff[idx-1], max_level);
    lost += coeff_cost_tab[run];
  }
#else
  int flag = quant_dequant_sse2(qp_div_6, qp_mod_6, q_bits, q_offset, dct, qcoeff, ssign);
  int mask = flag;
  int run = 0;
  int raster_idx = -1;
  int zigzag_idx, last_zigzag_idx = -1;
  while (mask)
  {
    const int raster_run = _bit_scan_forward(mask);
    raster_idx += raster_run + 1;
    mask = mask >> (raster_run + 1);
    zigzag_idx = zigzag[raster_idx];
    run = zigzag_idx - (last_zigzag_idx + 1);
    last_zigzag_idx = zigzag_idx;
    if (run < 0) break;
    coeffs4->level[cidx] = qcoeff[raster_idx];
    coeffs4->run[cidx]   = (uint8_t) run;
    coeffs4->sign[cidx]  = (int8_t) (-ssign[raster_idx]);
    cidx++;
    max_level = max (qcoeff[raster_idx], max_level);
    lost += coeff_cost_tab[run];
  }
  if (run<0) // fallback
  {
    run = 0;
    lost = 0;
    idx = 0;
    cidx = 0;
    int cnt = _popcnt32(flag);

    while (cnt)
    {
      const int block_idx = zagzig[idx++];
      int16_t level = qcoeff[block_idx];

      if (level)
      {
        coeffs4->level[cidx] = level;
        coeffs4->run[cidx]   = (uint8_t) run;
        coeffs4->sign[cidx]  = (int8_t) (-ssign[block_idx]);
        cidx++;
        max_level = max (level, max_level);
        lost += coeff_cost_tab[run];
        run = -1;
        cnt--;
      }
      run++;
    }
  }
#endif

  coeffs4->num_nonzero = cidx;
  if (max_level > 1)
    lost = COEFF_COST_MAX;
  *cost = lost;
  return((int8_t)((flag == 0) ? 0 : -1));
}

/**
 * Calculates the runs and levels the quantized coeffisient in a 2x2
 * block (chroma dc). Cost is not calculated since it is not used for
 * chroma dc. */
static void taa_h264_run_level_encode_2x2 (
  const int16_t * qcoeffs,
  coeffs2_t *     coeffs2)
{
  int run = 0;
  int max_level = 0;
  int nnz = 0;

  for (int c = 0; c < 4; c++)
  {
    const int coeff = qcoeffs[c];

    if (coeff)
    {
      int abslevel = abs (coeff);
      int sign = coeff < 0;

      coeffs2->level[nnz] = (int16_t) abslevel;
      coeffs2->run[nnz] = (uint8_t) run;
      coeffs2->sign[nnz] = (uint8_t) sign;
      nnz++;

      max_level = max (abslevel, max_level);
      run = 0;
    }
    else
    {
      run++;
    }
  }
  coeffs2->num_nonzero = nnz;
}

/* Performs quantization and dequantization of transformed coeffisient in TCOEFF.  */
static int taa_h264_quant_dequant_4x4_noskip(
  const int   qp_div_6,
  const int   qp_mod_6,
  const int   qp_bits,
  const int   qp_offset,
  int16_t *   tcoeff,                                     /* In: Transformed coeffs. Out: Dequantized trans coeffs. */
  coeffs4_t * coeffs4,
  int *       cost)                                 /* Out: The cost of encoding the coeffisients. */
{
  TAA_H264_ALIGN(64) uint8_t qcoeff[16];
  TAA_H264_ALIGN(64) int8_t ssign[16];
  int max_level = 0;
  int lost = 0;
  int idx = 0;
  int cidx = 0;
#ifdef HAVE_SSSE3
  int flag = quant_dequant_ssse3(qp_div_6, qp_mod_6, qp_bits, qp_offset, (__m128i *)tcoeff, qcoeff, ssign);
  int mask = flag;
  while (mask)
  {
    const int run = _bit_scan_forward(mask);
    idx = idx + run + 1;
    mask = mask >> (run+1);
    coeffs4->level[cidx] = qcoeff[idx-1];
    coeffs4->run[cidx]   = (uint8_t) (run);
    coeffs4->sign[cidx]  = (int8_t) (-ssign[idx-1]);
    cidx++;
    max_level = max (qcoeff[idx-1], max_level);
    lost += coeff_cost_tab[run];
  }
#else
  int flag = quant_dequant_sse2(qp_div_6, qp_mod_6, qp_bits, qp_offset, (__m128i *)tcoeff, qcoeff, ssign);
  int mask = flag;
  int run = 0;
  int raster_idx = -1;
  int zigzag_idx, last_zigzag_idx = -1;
  while (mask)
  {
    const int raster_run = _bit_scan_forward(mask);
    raster_idx += raster_run + 1;
    mask = mask >> (raster_run + 1);
    zigzag_idx = zigzag[raster_idx];
    run = zigzag_idx - (last_zigzag_idx + 1);
    last_zigzag_idx = zigzag_idx;
    if (run < 0) break;
    coeffs4->level[cidx] = qcoeff[raster_idx];
    coeffs4->run[cidx]   = (uint8_t) run;
    coeffs4->sign[cidx]  = (int8_t) (-ssign[raster_idx]);
    cidx++;
    max_level = max (qcoeff[raster_idx], max_level);
    lost += coeff_cost_tab[run];
  }
  if (run<0) // fallback
  {
    run = 0;
    lost = 0;
    idx = 0;
    cidx = 0;
    int cnt = _popcnt32(flag);

    while (cnt)
    {
      const int block_idx = zagzig[idx++];
      int16_t level = qcoeff[block_idx];

      if (level)
      {
        coeffs4->level[cidx] = level;
        coeffs4->run[cidx]   = (uint8_t) run;
        coeffs4->sign[cidx]  = (int8_t) (-ssign[block_idx]);
        cidx++;
        max_level = max (level, max_level);
        lost += coeff_cost_tab[run];
        run = -1;
        cnt--;
      }
      run++;
    }
  }
#endif

  coeffs4->num_nonzero = cidx;
  if (max_level > 1)
    lost = COEFF_COST_MAX;
  *cost = lost;
  return (flag == 0) ? 0 : 1;
}

/* Performs quantization and dequantization of transformed coeffisient in TCOEFF.  */
static int taa_h264_quant_dequant_4x4_skip(
  const int   qp_div_6,
  const int   qp_mod_6,
  const int   qp_bits,
  const int   qp_offset,
  int16_t *   tcoeff,                                     /* In: Transformed coeffs. Out: Dequantized trans coeffs. */
  coeffs4_t * coeffs4,
  int *       cost)                                 /* Out: The cost of encoding the coeffisients. */
{
  TAA_H264_ALIGN(64) uint8_t qcoeff[16];
  TAA_H264_ALIGN(64) int8_t ssign[16];
  int16_t save = tcoeff[0];
  tcoeff[0] = 0;
  int lost = 0;
  int max_level = 0;
  int cidx = 1;

#ifdef HAVE_SSSE3
  int flag = quant_dequant_ssse3(qp_div_6, qp_mod_6, qp_bits, qp_offset, (__m128i *)tcoeff, qcoeff, ssign);
  tcoeff[0] = save;
  int idx = 0;
  int mask = flag >> 1;
  while (mask)
  {
    const int run = _bit_scan_forward(mask);
    idx = idx + run + 1;
    mask = mask >> (run+1);
    coeffs4->level[cidx] = qcoeff[idx];
    coeffs4->run[cidx]   = (uint8_t) (run);
    coeffs4->sign[cidx]  = (int8_t) (-ssign[idx]);
    cidx++;
    max_level = max (qcoeff[idx], max_level);
    lost += coeff_cost_tab[run];
  }
#else
  int flag = quant_dequant_sse2(qp_div_6, qp_mod_6, qp_bits, qp_offset, (__m128i *)tcoeff, qcoeff, ssign);
  tcoeff[0] = save;
  int mask = flag >> 1;
  int run = 0;
  int raster_idx = 0;
  int zigzag_idx, last_zigzag_idx = 0;
  while (mask)
  {
    const int raster_run = _bit_scan_forward(mask);
    raster_idx += raster_run + 1;
    mask = mask >> (raster_run+1);
    zigzag_idx = zigzag[raster_idx];
    run = zigzag_idx - (last_zigzag_idx + 1);
    last_zigzag_idx = zigzag_idx;
    if (run<0) break;
    coeffs4->level[cidx] = qcoeff[raster_idx];
    coeffs4->run[cidx]   = (uint8_t) (run);
    coeffs4->sign[cidx]  = (int8_t) (-ssign[raster_idx]);
    cidx++;
    max_level = max (qcoeff[raster_idx], max_level);
    lost += coeff_cost_tab[run];
  }
  if (run<0) // fallback
  {
    int run = 0;
    int idx = 1;
    int cnt = _popcnt32(flag >> 1);
    lost = 0;
    cidx = 1;
    while (cnt)
    {
      const int block_idx = zagzig[idx++];
      int16_t level = qcoeff[block_idx];

      if (level)
      {
        coeffs4->level[cidx] = level;
        coeffs4->run[cidx]   = (uint8_t) run;
        coeffs4->sign[cidx]  = (int8_t) (-ssign[block_idx]);
        cidx++;
        max_level = max (level, max_level);
        lost += coeff_cost_tab[run];
        run = -1;
        cnt--;
      }
      run++;
    }
  }
#endif

  coeffs4->num_nonzero = cidx - 1;
  if (max_level > 1)
    lost = COEFF_COST_MAX;
  *cost = lost;
  return (flag == 0) ? 0 : 1;
}

static int taa_h264_quant_dequant_2x2_dc_chroma (
  const int   qp_div_6,
  const int   qp_mod_6,
  const int   qp_bits,
  const int   qp_offset,
  int16_t *   tcoeff,
  coeffs2_t * coeffs2)
{
  int16_t qcoeff[4];
  const int qc = quant_coef[qp_mod_6][0];
  const int dqc = dequant_coef[qp_mod_6][0];

  int flag = 0;

  for (int i = 0; i < 4; i++)
  {
    /* Forward quantization */
    int idx = i * 16;                     /* zagzig index and raster index is both 0 */
    int sign = tcoeff[idx] > 0;
    int level = (abs (tcoeff[idx]) * qc + qp_offset) >> (qp_bits + 1);
    qcoeff[i] = (int16_t)(sign ? level : -level);

    /* Inverse quantization */
    tcoeff[idx] = (int16_t)((qcoeff[i] * dqc) << qp_div_6);

    flag |= level;
  }

  taa_h264_run_level_encode_2x2 (qcoeff, coeffs2);

  return (flag == 0) ? 0 : 1;
}

/* TODO: Is it possible to merge this with chroma DC?
 *
 * Need to handle the inverse quantization the same way in order to do
 * that. */
static int taa_h264_quant_4x4_dc_luma (
  const int   qp_mod_6,
  const int   q_bits,
  const int   q_offset,
  int16_t *   tcoeff,
  coeffs4_t * coeffs4)
{
  const int16_t qc = quant_coef[qp_mod_6][0];
  TAA_H264_ALIGN(64) uint8_t qcoeff[16];
  TAA_H264_ALIGN(64) int8_t ssign[16];
  int max_level = 0;
  int idx = 0;
  int cidx = 0;
#ifdef HAVE_SSSE3
  int flag = quant_ssse3(qc, q_bits+1, q_offset, tcoeff, qcoeff, ssign);
  int mask = flag;
  while (mask)
  {
    const int run = _bit_scan_forward(mask);
    idx = idx + run + 1;
    mask = mask >> (run+1);
    coeffs4->level[cidx] = qcoeff[idx-1];
    coeffs4->run[cidx]   = (uint8_t) (run);
    coeffs4->sign[cidx]  = (int8_t) (-ssign[idx-1]);
    cidx++;
    max_level = max (qcoeff[idx-1], max_level);
  }
#else
  int flag = quant_sse2(qc, q_bits+1, q_offset, tcoeff, qcoeff, ssign);
  int mask = flag;
  int run = 0;
  int raster_idx = -1;
  int zigzag_idx, last_zigzag_idx = -1;
  while (mask)
  {
    const int raster_run = _bit_scan_forward(mask);
    raster_idx += raster_run + 1;
    mask = mask >> (raster_run + 1);
    zigzag_idx = zigzag[raster_idx];
    run = zigzag_idx - (last_zigzag_idx + 1);
    last_zigzag_idx = zigzag_idx;
    if (run < 0) break;
    coeffs4->level[cidx] = qcoeff[raster_idx];
    coeffs4->run[cidx]   = (uint8_t) run;
    coeffs4->sign[cidx]  = (int8_t) (-ssign[raster_idx]);
    cidx++;
  }
  if (run<0) // fallback
  {
    run = 0;
    idx = 0;
    cidx = 0;
    int cnt = _popcnt32(flag);

    while (cnt)
    {
      const int block_idx = zagzig[idx++];
      int16_t level = qcoeff[block_idx];

      if (level)
      {
        coeffs4->level[cidx] = level;
        coeffs4->run[cidx]   = (uint8_t) run;
        coeffs4->sign[cidx]  = (int8_t) (-ssign[block_idx]);
        cidx++;
        max_level = max (level, max_level);
        run = -1;
        cnt--;
      }
      run++;
    }
  }
#endif

  coeffs4->num_nonzero = cidx;
  return (flag == 0) ? 0 : 1;
}

/* TODO: Check if there is performance gain by doing the dequant operation in
 * the inverse transform function. */
static void taa_h264_dequant_4x4_dc_luma (
  const int          qp_div_6,
  const int          qp_mod_6,
  int16_t *  qcoeff)
{
  const int dqc = (int)(dequant_coef[qp_mod_6][0] << qp_div_6);
  TAA_H264_ALIGN (16) int temp[16];
#pragma ivdep
#pragma vector aligned
#pragma unroll(4)
  for (int i = 0; i < 16; i++)
    temp[i] = (int)((qcoeff[i] * dqc + 2) >> 2);
#pragma ivdep
#pragma vector aligned
#pragma unroll(2)
  for (int i = 0; i < 16; i++)
    qcoeff[i] = (int16_t)((temp[i] > 32767) ? 32767 : (temp[i] < -32768) ? -32768 : temp[i]);
}

#if 0
static void taa_h264_transform_4x4 (
  int16_t * block,
  int16_t * coeff)
{
  int tmp0, tmp1, tmp2, tmp3;

  /* Horizontal transform */
  for (int i = 0; i < 4; i++)
  {
    tmp0 = block[i * 4 + 0] + block[i * 4 + 3];
    tmp1 = block[i * 4 + 1] + block[i * 4 + 2];
    tmp2 = block[i * 4 + 1] - block[i * 4 + 2];
    tmp3 = block[i * 4 + 0] - block[i * 4 + 3];

    coeff[i * 4 + 0] = (int16_t) (tmp0 + tmp1);
    coeff[i * 4 + 1] = (int16_t) ((tmp3 << 1) + tmp2);
    coeff[i * 4 + 2] = (int16_t) (tmp0 - tmp1);
    coeff[i * 4 + 3] = (int16_t) (tmp3 - (tmp2 << 1));
  }

  /* Vertical transform */
  for (int i = 0; i < 4; i++)
  {
    tmp0 = coeff[0 * 4 + i] + coeff[3 * 4 + i];
    tmp1 = coeff[1 * 4 + i] + coeff[2 * 4 + i];
    tmp2 = coeff[1 * 4 + i] - coeff[2 * 4 + i];
    tmp3 = coeff[0 * 4 + i] - coeff[3 * 4 + i];

    coeff[0 * 4 + i] = (int16_t) (tmp0 + tmp1);
    coeff[1 * 4 + i] = (int16_t) ((tmp3 << 1) + tmp2);
    coeff[2 * 4 + i] = (int16_t) (tmp0 - tmp1);
    coeff[3 * 4 + i] = (int16_t) (tmp3 - (tmp2 << 1));
  }
}


static void taa_h264_inverse_transform_4x4 (
  int16_t * coeff,
  int16_t * block)
{
  /* Section 8.5.10 */
  /* Horizontal transform */
  int tmp0, tmp1, tmp2, tmp3;
  for (int i = 0; i < 4; i++)
  {
    tmp0 =  coeff[i * 4 + 0]       +  coeff[i * 4 + 2];
    tmp1 =  coeff[i * 4 + 0]       -  coeff[i * 4 + 2];
    tmp2 = (coeff[i * 4 + 1] >> 1) -  coeff[i * 4 + 3];
    tmp3 =  coeff[i * 4 + 1]       + (coeff[i * 4 + 3] >> 1);

    block[i * 4 + 0] = (int16_t)(tmp0 + tmp3);
    block[i * 4 + 1] = (int16_t)(tmp1 + tmp2);
    block[i * 4 + 2] = (int16_t)(tmp1 - tmp2);
    block[i * 4 + 3] = (int16_t)(tmp0 - tmp3);
  }

  /* Vertical transform */
  for (int i = 0; i < 4; i++)
  {
    tmp0 =  block[0 * 4 + i]       +  block[2 * 4 + i];
    tmp1 =  block[0 * 4 + i]       -  block[2 * 4 + i];
    tmp2 = (block[1 * 4 + i] >> 1) -  block[3 * 4 + i];
    tmp3 =  block[1 * 4 + i]       + (block[3 * 4 + i] >> 1);

    block[0 * 4 + i] = (int16_t)((tmp0 + tmp3 + 32) >> 6);
    block[1 * 4 + i] = (int16_t)((tmp1 + tmp2 + 32) >> 6);
    block[2 * 4 + i] = (int16_t)((tmp1 - tmp2 + 32) >> 6);
    block[3 * 4 + i] = (int16_t)((tmp0 - tmp3 + 32) >> 6);
  }
}
#endif

static void taa_h264_hadamard_4x4 (
  int16_t * block,
  int16_t * coeff)
{
  int tmp0, tmp1, tmp2, tmp3;
  /* Horizontal transform */
  for (int i = 0; i < 4; i++)
  {
    tmp0 = block[i * 4 + 0] + block[i * 4 + 3];
    tmp1 = block[i * 4 + 1] + block[i * 4 + 2];
    tmp2 = block[i * 4 + 1] - block[i * 4 + 2];
    tmp3 = block[i * 4 + 0] - block[i * 4 + 3];

    coeff[i * 4 + 0] = (int16_t)(tmp0 + tmp1);
    coeff[i * 4 + 1] = (int16_t)(tmp3 + tmp2);
    coeff[i * 4 + 2] = (int16_t)(tmp0 - tmp1);
    coeff[i * 4 + 3] = (int16_t)(tmp3 - tmp2);
  }

  /* Vertical transform */
  for (int i = 0; i < 4; i++)
  {
    tmp0 = coeff[0 * 4 + i] + coeff[3 * 4 + i];
    tmp1 = coeff[1 * 4 + i] + coeff[2 * 4 + i];
    tmp2 = coeff[1 * 4 + i] - coeff[2 * 4 + i];
    tmp3 = coeff[0 * 4 + i] - coeff[3 * 4 + i];

    coeff[0 * 4 + i] = (int16_t)((tmp0 + tmp1) >> 1);
    coeff[1 * 4 + i] = (int16_t)((tmp3 + tmp2) >> 1);
    coeff[2 * 4 + i] = (int16_t)((tmp0 - tmp1) >> 1);
    coeff[3 * 4 + i] = (int16_t)((tmp3 - tmp2) >> 1);
  }
}




static void taa_h264_inverse_hadamard_4x4 (
  int16_t * coeff,
  int16_t * block)
{
  int tmp0, tmp1, tmp2, tmp3;
  /* Horizontal transform */
  for (int i = 0; i < 4; i++)
  {
    tmp0 = coeff[i * 4 + 0] + coeff[i * 4 + 2];
    tmp1 = coeff[i * 4 + 0] - coeff[i * 4 + 2];
    tmp2 = coeff[i * 4 + 1] - coeff[i * 4 + 3];
    tmp3 = coeff[i * 4 + 1] + coeff[i * 4 + 3];

    block[i * 4 + 0] = (int16_t)(tmp0 + tmp3);
    block[i * 4 + 1] = (int16_t)(tmp1 + tmp2);
    block[i * 4 + 2] = (int16_t)(tmp1 - tmp2);
    block[i * 4 + 3] = (int16_t)(tmp0 - tmp3);
  }
  /* Vertical transform and dequantization */
  for (int i = 0; i < 4; i++)
  {
    tmp0 = block[0 * 4 + i] + block[2 * 4 + i];
    tmp1 = block[0 * 4 + i] - block[2 * 4 + i];
    tmp2 = block[1 * 4 + i] - block[3 * 4 + i];
    tmp3 = block[1 * 4 + i] + block[3 * 4 + i];

    block[0 * 4 + i] = (int16_t)(tmp0 + tmp3);
    block[1 * 4 + i] = (int16_t)(tmp1 + tmp2);
    block[2 * 4 + i] = (int16_t)(tmp1 - tmp2);
    block[3 * 4 + i] = (int16_t)(tmp0 - tmp3);
  }
}

static void taa_h264_hadamard_2x2 (int16_t * coeffs)
{
  int c1 = coeffs [0 * 16];
  int c2 = coeffs [1 * 16];
  int c3 = coeffs [2 * 16];
  int c4 = coeffs [3 * 16];

  coeffs[0 * 16] = (int16_t)(c1 + c2 + c3 + c4);
  coeffs[1 * 16] = (int16_t)(c1 - c2 + c3 - c4);
  coeffs[2 * 16] = (int16_t)(c1 + c2 - c3 - c4);
  coeffs[3 * 16] = (int16_t)(c1 - c2 - c3 + c4);
}


static void taa_h264_inverse_hadamard_2x2 (int16_t * coeffs)
{
  int c1 = coeffs [0 * 16];
  int c2 = coeffs [1 * 16];
  int c3 = coeffs [2 * 16];
  int c4 = coeffs [3 * 16];

  coeffs[0 * 16] = (int16_t)(c1 + c2 + c3 + c4);
  coeffs[1 * 16] = (int16_t)(c1 - c2 + c3 - c4);
  coeffs[2 * 16] = (int16_t)(c1 + c2 - c3 - c4);
  coeffs[3 * 16] = (int16_t)(c1 - c2 - c3 + c4);

  coeffs[0 * 16] >>= 1;
  coeffs[1 * 16] >>= 1;
  coeffs[2 * 16] >>= 1;
  coeffs[3 * 16] >>= 1;
}

/************************** ENCODE MB **********************************/



/* TODO: 4x4 intra prediction may be done in common/prediction.c. Might reduce
 * performance of encoder and/or decoder a bit. */

static int taa_h264_sad_sub (
  const uint8_t * b1,
  const uint8_t * b2,
  int             rows,               /* Rows to iterate */
  int             cols,               /* Columns to iterate */
  int             stride1,            /* Stride between rows of block 1*/
  int             stride2)            /* Stride between rows of block 2 */
{
  int sad = 0;
  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      sad += abs (b1[i * stride1 + j] - b2[i * stride2 + j]);
    }
  }
  return sad;
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
  const uint8_t   topleft,
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
    (uint8_t)((left[1] + 2 * left[0] + topleft + 2) >> 2);
  pred[0 * 4 + 0] =
    pred[1 * 4 + 1] =
    pred[2 * 4 + 2] =
    pred[3 * 4 + 3] =
    (uint8_t)((left[0] + 2 * topleft + top[0] + 2) >> 2);
  pred[0 * 4 + 1] =
    pred[1 * 4 + 2] =
    pred[2 * 4 + 3] =
    (uint8_t)((topleft + 2 * top[0] + top[1] + 2) >> 2);
  pred[0 * 4 + 2] =
    pred[1 * 4 + 3] =
    (uint8_t)((top[0] + 2 * top[1] + top[2] + 2) >> 2);
  pred[0 * 4 + 3] =
    (uint8_t)((top[1] + 2 * top[2] + top[3] + 2) >> 2);

}


static void taa_h264_intra_pred_luma_vert_right (
  const uint8_t * left,
  const uint8_t * top,
  const uint8_t   topleft,
  uint8_t *       pred)
{
  pred[0 * 4 + 0] =
    pred[2 * 4 + 1] =
    (uint8_t)((topleft + top[0] + 1) >> 1);
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
    (uint8_t)((left[0] + 2 * topleft + top[0] + 2) >> 2);
  pred[1 * 4 + 1] =
    pred[3 * 4 + 2] =
    (uint8_t)((topleft + 2 * top[0] + top[1] + 2) >> 2);
  pred[1 * 4 + 2] =
    pred[3 * 4 + 3] =
    (uint8_t)((top[0] + 2 * top[1] + top[2] + 2) >> 2);
  pred[1 * 4 + 3] =
    (uint8_t)((top[1] + 2 * top[2] + top[3] + 2) >> 2);
  pred[2 * 4 + 0] =
    (uint8_t)((topleft + 2 * left[0] + left[1] + 2) >> 2);
  pred[3 * 4 + 0] =
    (uint8_t)((left[0] + 2 * left[1] + left[2] + 2) >> 2);

}


static void taa_h264_intra_pred_luma_horz_down (
  const uint8_t * left,
  const uint8_t * top,
  const uint8_t   topleft,
  uint8_t *       pred)
{
  pred[0 * 4 + 0] =
    pred[1 * 4 + 2] =
    (uint8_t)((topleft + left[0] + 1) >> 1);
  pred[0 * 4 + 1] =
    pred[1 * 4 + 3] =
    (uint8_t)((left[0] + 2 * topleft + top[0] + 2) >> 2);
  pred[0 * 4 + 2] =
    (uint8_t)((topleft + 2 * top[0] + top[1] + 2) >> 2);
  pred[0 * 4 + 3] =
    (uint8_t)((top[0] + 2 * top[1] + top[2] + 2) >> 2);
  pred[1 * 4 + 0] =
    pred[2 * 4 + 2] =
    (uint8_t)((left[0] + left[1] + 1) >> 1);
  pred[1 * 4 + 1] =
    pred[2 * 4 + 3] =
    (uint8_t)((topleft + 2 * left[0] + left[1] + 2) >> 2);
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


static unsigned taa_h264_intra_4x4_pred_luma (
  const uint8_t *    curr,                               /* pointer to input 4x4 block */
  const uint8_t *    left,                               /* column left of 4x4 block */
  const uint8_t *    up,                                 /* row above 4x4 block */
  const uint8_t      upleft,                             /* value top left of 4x4 block */
  const bool         left_avail,                         /* whether left block is available */
  const bool         up_avail,                           /* whether above block is available */
  const bool         upleft_avail,
  const bool         upright_avail,
  const imode4_t     pred_mode,
  const int          lambda,
  const unsigned     coding_options,
  imode4_t *         mode_out,                           /* out: best intra mode */
  uint8_t *  ipred                               /* out: pointer for storage of pred blocks */
  )
{
  unsigned min_cost = UINT_MAX;
  imode4_t best_mode;
  uint8_t * ipred_ptr;
  const int pred_cost = (1 * lambda) >> LAMBDA_SHIFT;
  const int nopred_cost = (4 * lambda) >> LAMBDA_SHIFT;

  /* Vertical prediction */
  if (up_avail)
  {
    ipred_ptr = &ipred[VERT_PRED * 16];
    for (int i = 0; i < 4; i++)
    {
      ipred_ptr[i * 4 + 0] = up[0];
      ipred_ptr[i * 4 + 1] = up[1];
      ipred_ptr[i * 4 + 2] = up[2];
      ipred_ptr[i * 4 + 3] = up[3];
    }
    unsigned cost = (pred_mode != VERT_PRED) ? nopred_cost : pred_cost;
    cost += taa_h264_sad_sub (ipred_ptr, curr, 4, 4, 4, MB_WIDTH_Y);
    if (cost < min_cost)
    {
      min_cost = cost;
      best_mode = VERT_PRED;
    }
  }

  /* Horizontal prediction */
  if (left_avail)
  {
    ipred_ptr = &ipred[HOR_PRED * 16];
    for (int j = 0; j < 4; j++)
    {
      ipred_ptr[0 * 4 + j] = left[0];
      ipred_ptr[1 * 4 + j] = left[1];
      ipred_ptr[2 * 4 + j] = left[2];
      ipred_ptr[3 * 4 + j] = left[3];
    }
    unsigned cost = (pred_mode != HOR_PRED) ? nopred_cost : pred_cost;
    cost += taa_h264_sad_sub (ipred_ptr, curr, 4, 4, 4, MB_WIDTH_Y);
    if (cost < min_cost)
    {
      min_cost = cost;
      best_mode = HOR_PRED;
    }
  }


  /* DC prediction is always available */
  int shift = 1;
  int sum = 0;
  int sum_left = left[0] + left[1] + left[2] + left[3];
  int sum_up   = up[0] + up[1] + up[2] + up[3];
  if (left_avail)
  {
    sum += sum_left + 2;
    shift++;
  }
  if (up_avail)
  {
    sum += sum_up + 2;
    shift++;
  }
  if (shift == 1)
  {
    sum = 128;
    shift = 0;
  }
  uint8_t dc = (uint8_t)(sum >> shift);
  ipred_ptr = &ipred[DC_PRED * 16];
  for (int i = 0; i < 16; i++)
    ipred_ptr[i] = dc;

  unsigned cost = (pred_mode != DC_PRED) ? nopred_cost : pred_cost;
  cost += taa_h264_sad_sub (ipred_ptr, curr, 4, 4, 4, MB_WIDTH_Y);
  if (cost < min_cost)
  {
    min_cost = cost;
    best_mode = DC_PRED;
  }

  const int max_mode = (coding_options & TAA_H264_LQ_DISABLE_HIGH_INTRA_4x4_MODES) ? DC_PRED : HOR_UP_PRED;
  for (int mode = DIAG_DOWN_LEFT_PRED; mode <= max_mode; mode++)
  {
    ipred_ptr = &ipred[mode * 16];
    bool valid = false;

    switch (mode)
    {
    case DIAG_DOWN_LEFT_PRED:
      if (up_avail)
      {
        taa_h264_intra_pred_luma_down_left (
          up, upright_avail, ipred_ptr);
        valid = true;
      }
      break;
    case DIAG_DOWN_RIGHT_PRED:
      if (left_avail && up_avail && upleft_avail)
      {
        taa_h264_intra_pred_luma_down_right (
          left, up, upleft, ipred_ptr);
        valid = true;
      }
      break;
    case VERT_RIGHT_PRED:
      if (left_avail && up_avail && upleft_avail)
      {
        taa_h264_intra_pred_luma_vert_right (
          left, up, upleft, ipred_ptr);
        valid = true;
      }
      break;
    case HOR_DOWN_PRED:
      if (left_avail && up_avail && upleft_avail)
      {
        taa_h264_intra_pred_luma_horz_down (
          left, up, upleft, ipred_ptr);
        valid = true;
      }
      break;
    case VERT_LEFT_PRED:
      if (up_avail)
      {
        taa_h264_intra_pred_luma_vert_left (
          up, upright_avail, ipred_ptr);
        valid = true;
      }
      break;
    case HOR_UP_PRED:
      if (left_avail)
      {
        taa_h264_intra_pred_luma_horz_up (
          left, ipred_ptr);
        valid = true;
      }
      break;
    default:
      TAA_H264_DEBUG_ASSERT (false);
      break;
    }

    if (valid)
    {
      unsigned cost = (pred_mode != mode) ? nopred_cost : pred_cost;
      cost += taa_h264_sad_sub (ipred_ptr, curr, 4, 4, 4, MB_WIDTH_Y);
      if (cost < min_cost)
      {
        min_cost = cost;
        best_mode = (imode4_t) mode;
      }
    }
  }

  *mode_out = best_mode;
  return min_cost;
}

static uint8_t const TAA_H264_ALIGN (16) zigzag_8x8[16] =
{
  0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15
};


/* TODO: Should remove mbxmax and pred_intra_modes and move call to
 * taa_h264_predict_intra_4x4_mode else where. */
/* Returns the cost of encoding this MB as intra 4x4. */
static unsigned taa_h264_encode_i4x4_luma_ (
  const bool            intra,
  const int             qp,
  const int             avail_flags,
  const int             mbxmax,
  const int             lambda,
  luma_t *              luma,
  const uint8_t *       recon_left_y,
  const uint8_t *       recon_top_y,
  const uint8_t         recon_topleft_y,
  const unsigned        coding_options,
  imode4_t *    pred_intra_modes,
  uint8_t *     intra_modes_ptr,
  mbcoeffs_t *  mbcoeffs)
{
  const int qp_div_6 = TAA_H264_DIV6 (qp);
  const int qp_mod_6 = qp - qp_div_6 * 6;
  const int qp_bits = Q_BITS + qp_div_6;
  const int shift_q_offset = 8 - qp_div_6 + (intra ? 0 : 1);
  const int qp_offset = MAX_Q_OFFSET >> shift_q_offset;

  const bool mb_left = MB_LEFT (avail_flags);
  const bool mb_top = MB_UP (avail_flags);
  const bool mb_top_left = MB_UP_LEFT (avail_flags);
  const bool mb_top_right = MB_UP_RIGHT (avail_flags);

  int left_flags;
  int top_flags;
  int top_right_flags;
  int top_left_flags;

  taa_h264_set_4x4_blocks_neighbor_flags (
    mb_left, mb_top, mb_top_right, mb_top_left,
    &left_flags, &top_flags, &top_right_flags, &top_left_flags);

  unsigned cost = 0;

  for (int m = 0; m < 16; m += 8)
  {
    for (int n = 0; n < 16; n += 8)
    {
      for (int i = 0; i < 8; i += 4)
      {
        for (int j = 0; j < 8; j += 4)
        {
          uint8_t TAA_H264_ALIGN(64) predbuf[16 * NUM_I4x4_MODES];
          int16_t TAA_H264_ALIGN(64) transbuf[16];

          const int index_4x4 = m + n / 2 + i / 2 + j / 4;
          const int raster_idx = m + n / 4 + i + j / 4;
          const int offset_pos = (m + i) * MB_WIDTH_Y + n + j;
          const bool first_col = (n | j) == 0;
          const bool first_row = (m | i) == 0;
          const bool left_avail = NEIGHBOR_BLOCK_AVAILABLE (left_flags, raster_idx);
          const bool up_avail = NEIGHBOR_BLOCK_AVAILABLE (top_flags, raster_idx);
          const bool upright_avail = NEIGHBOR_BLOCK_AVAILABLE (top_right_flags, raster_idx);
          const bool upleft_avail = NEIGHBOR_BLOCK_AVAILABLE (top_left_flags, raster_idx);

          const uint8_t * input_ptr = &luma->input[offset_pos];
          uint8_t *  recon_ptr = &luma->recon[offset_pos];

          imode4_t imode;

          /* predict intra 4x4 mode */
          imode4_t pred_mode =
            taa_h264_predict_intra_4x4_mode (&intra_modes_ptr [raster_idx],
                                             mbxmax * NUM_4x4_BLOCKS_Y,
                                             !first_col,
                                             !first_row,
                                             mb_left,
                                             mb_top);
          pred_intra_modes [raster_idx] = pred_mode;

          /* intra 4x4 prediction */
          // TODO: Clean up, could probably find a better memory layout to avoid
          // so many if tests.
          uint8_t recon_left[4];
          const uint8_t * left_ptr = (first_col) ? &recon_left_y[m + i] : recon_left;
          const uint8_t * up_ptr   = (first_row) ? &recon_top_y[n + j] : &recon_ptr[-MB_WIDTH_Y];
          uint8_t upleft;
          
          if (first_col && first_row)
            upleft = recon_topleft_y;
          else if (first_row)
            upleft = up_ptr[-1];
          else if (first_col)
            upleft = left_ptr[-1];
          else
            upleft = recon_ptr[-MB_WIDTH_Y-1];

          if (!first_col)
          {
            recon_left[0] = recon_ptr[               - 1];
            recon_left[1] = recon_ptr[MB_WIDTH_Y     - 1];
            recon_left[2] = recon_ptr[MB_WIDTH_Y * 2 - 1];
            recon_left[3] = recon_ptr[MB_WIDTH_Y * 3 - 1];
          }

          cost += taa_h264_intra_4x4_pred_luma (input_ptr,
                                        left_ptr,
                                        up_ptr,
                                        upleft,
                                        left_avail,
                                        up_avail,
                                        upleft_avail,
                                        upright_avail,
                                        pred_mode,
                                        lambda,
                                        coding_options,
                                        &imode,
                                        predbuf);
          intra_modes_ptr [raster_idx] = (uint8_t) imode;

          /* residual */
          const uint8_t * predptr = &predbuf[imode * 16];
          for (int k = 0; k < 4; k++)
          {
            for (int l = 0; l < 4; l++)
            {
              transbuf[k * 4 + l] = (int16_t)(input_ptr [k * MB_WIDTH_Y + l] - predptr [k * 4 + l]);
            }
          }

          /* 4x4 transform */
          taa_h264_transform_4x4_x86_64((__m128i *) transbuf);

          /* 4x4 quantization */
          int dummy_cost;
          int cbpflag = taa_h264_quant_dequant_4x4_noskip (
            qp_div_6,
            qp_mod_6,
            qp_bits,
            qp_offset,
            transbuf,
            &mbcoeffs->luma_ac[index_4x4],
            &dummy_cost);

          mbcoeffs->luma_cbp = (int16_t)(mbcoeffs->luma_cbp | (cbpflag << zigzag_8x8[index_4x4]));

          if (cbpflag)
          {
            taa_h264_inverse_transform_4x4_x86_64((__m128i *) transbuf);
          }
          else
          {
            for (int k = 0; k < 4 * 4; k++)
            {
              transbuf[k] = 0;
            }
          }
          /* 4x4 reconstruction */
          for (int k = 0; k < 4; k++)
          {
            for (int l = 0; l < 4; l++)
            {
              recon_ptr[k * MB_WIDTH_Y + l] = (uint8_t)
                max(0, min(255, (transbuf[k * 4 + l] + predptr[k * 4 + l])));
            }
          }
        }
      }
    }
  }

  return cost;
}

#ifdef UNIT_TEST
static
#endif
#ifdef HAVE_SSSE3
unsigned taa_h264_encode_i4x4_luma_ssse3 (
#else
unsigned taa_h264_encode_i4x4_luma (
#endif
  const bool intra,
  mbinfo_t * mb,
  uint8_t * intra_modes_ptr,
  const unsigned coding_options)
{
  mb->mbcoeffs.luma_cbp = 0;
  return taa_h264_encode_i4x4_luma_ (
    intra,
    mb->mquant,
    mb->avail_flags,
    mb->mbxmax,
    mb->lambda,
    &mb->luma,
    mb->recon_left_y,
    mb->recon_top_y,
    mb->recon_topleft_y,
    coding_options,
    mb->pred_intra_modes,
    intra_modes_ptr,
    &mb->mbcoeffs);
}

static void taa_h264_encode_intra_16x16_luma (
  const bool            intra,
  const int             qp,
  const imode16_t       best_i16x16_mode,
  luma_t *              luma,
  const uint8_t         ipred_y[MB_SIZE_Y],
  mbcoeffs_t *  mbcoeffs)
{
  const int qp_div_6 = TAA_H264_DIV6 (qp);
  const int qp_mod_6 = qp - (qp_div_6*6);
  const int qp_bits = Q_BITS + qp_div_6;
  const int shift_q_offset = 8 - qp_div_6 + (intra ? 0 : 1);
  const int qp_offset = MAX_Q_OFFSET_DC >> shift_q_offset;

  TAA_H264_ALIGN(64) int16_t dcbuf [4 * 4];
  int offset_imode = best_i16x16_mode * MB_SIZE_Y;
  int num_coeffs = 0;
  int cbpflag[16];
  const int num_dc_coeffs = 16;
  const int num_ac_coeffs_per_block = 15;
  __m128i predbuf[16];
  __m128i transbuf[32];

  for (int i = 0; i < 4; i++)
  {
    //=================================================================
    //                   4 x 4 transpose luma->input
    //=================================================================
    __m128 row0, row1, row2, row3, tmp0, tmp1, tmp2, tmp3;
    row0 = _mm_load_ps((float *) &luma->input[(4*i+0) * MB_WIDTH_Y]);
    row1 = _mm_load_ps((float *) &luma->input[(4*i+1) * MB_WIDTH_Y]);
    row2 = _mm_load_ps((float *) &luma->input[(4*i+2) * MB_WIDTH_Y]);
    row3 = _mm_load_ps((float *) &luma->input[(4*i+3) * MB_WIDTH_Y]);
    tmp0 = _mm_shuffle_ps((row0), (row1), 0x44);
    tmp2 = _mm_shuffle_ps((row0), (row1), 0xEE);
    tmp1 = _mm_shuffle_ps((row2), (row3), 0x44);
    tmp3 = _mm_shuffle_ps((row2), (row3), 0xEE);
    __m128i const input[4] =
    {
      _mm_castps_si128(_mm_shuffle_ps(tmp0, tmp1, 0x88)),
      _mm_castps_si128(_mm_shuffle_ps(tmp0, tmp1, 0xDD)),
      _mm_castps_si128(_mm_shuffle_ps(tmp2, tmp3, 0x88)),
      _mm_castps_si128(_mm_shuffle_ps(tmp2, tmp3, 0xDD))
    };
    //=================================================================
    //                   4 x 4 transpose ipred_y
    //=================================================================
    row0 = _mm_load_ps((float *) &ipred_y[offset_imode + (4*i+0) * MB_WIDTH_Y]);
    row1 = _mm_load_ps((float *) &ipred_y[offset_imode + (4*i+1) * MB_WIDTH_Y]);
    row2 = _mm_load_ps((float *) &ipred_y[offset_imode + (4*i+2) * MB_WIDTH_Y]);
    row3 = _mm_load_ps((float *) &ipred_y[offset_imode + (4*i+3) * MB_WIDTH_Y]);
    tmp0 = _mm_shuffle_ps((row0), (row1), 0x44);
    tmp2 = _mm_shuffle_ps((row0), (row1), 0xEE);
    tmp1 = _mm_shuffle_ps((row2), (row3), 0x44);
    tmp3 = _mm_shuffle_ps((row2), (row3), 0xEE);

    predbuf[4*i+0] = _mm_castps_si128(_mm_shuffle_ps(tmp0, tmp1, 0x88));
    predbuf[4*i+1] = _mm_castps_si128(_mm_shuffle_ps(tmp0, tmp1, 0xDD));
    predbuf[4*i+2] = _mm_castps_si128(_mm_shuffle_ps(tmp2, tmp3, 0x88));
    predbuf[4*i+3] = _mm_castps_si128(_mm_shuffle_ps(tmp2, tmp3, 0xDD));



    //=================================================================
    //               4 x 4 transform luma->input - ipred_y
    //=================================================================
    for (int j = 0; j < 4; j++)
    {
      const int zz = zigzag_8x8[4*i+j];
      __m128i O = _mm_setzero_si128();
      transbuf[8*i+2*j+0] = _mm_sub_epi16(_mm_unpacklo_epi8(input[j], O), _mm_unpacklo_epi8(predbuf[4*i+j], O));
      transbuf[8*i+2*j+1] = _mm_sub_epi16(_mm_unpackhi_epi8(input[j], O), _mm_unpackhi_epi8(predbuf[4*i+j], O));

      taa_h264_transform_4x4_x86_64((__m128i * ) &transbuf[8*i+2*j+0]);
      memcpy(&dcbuf[4*i+j], &transbuf[8*i+2*j+0], 2);

      int dummy_cost;
      cbpflag[zz] = taa_h264_quant_dequant_4x4_skip (
        qp_div_6,
        qp_mod_6,
        qp_bits,
        qp_offset>>1,
        (int16_t *) &transbuf[8*i+2*j+0],
        &mbcoeffs->luma_ac[zz],
        &dummy_cost);
      mbcoeffs->luma_cbp = (int16_t)(mbcoeffs->luma_cbp | (cbpflag[zz] << (4*i+j)));
      num_coeffs += num_ac_coeffs_per_block;
    }
  }

  taa_h264_hadamard_4x4 (dcbuf, dcbuf);
  int dcflag = taa_h264_quant_4x4_dc_luma (
    qp_mod_6,
    qp_bits,
    qp_offset,
    dcbuf,
    &mbcoeffs->luma_dc);

  taa_h264_inverse_hadamard_4x4 (dcbuf, dcbuf);
  taa_h264_dequant_4x4_dc_luma (
    qp_div_6,
    qp_mod_6,
    dcbuf);
  num_coeffs += num_dc_coeffs;

  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      const int zz = zigzag_8x8[4*i + j];
      memcpy(&transbuf[8*i+2*j], &dcbuf[4*i + j], 2);
      if (dcflag || cbpflag[zz])
      {
        __m128i O = _mm_setzero_si128();
        taa_h264_inverse_transform_4x4_x86_64(&transbuf[8*i+2*j]);
        predbuf[4*i+j] = _mm_packus_epi16(_mm_add_epi16(transbuf[8*i+2*j+0], _mm_unpacklo_epi8(predbuf[4*i + j], O)),
                                          _mm_add_epi16(transbuf[8*i+2*j+1], _mm_unpackhi_epi8(predbuf[4*i + j], O)));

      }
    }
    __m128 row0 = _mm_castsi128_ps(predbuf[4*i+0]);
    __m128 row1 = _mm_castsi128_ps(predbuf[4*i+1]);
    __m128 row2 = _mm_castsi128_ps(predbuf[4*i+2]);
    __m128 row3 = _mm_castsi128_ps(predbuf[4*i+3]);
    __m128 tmp0 = _mm_shuffle_ps(row0, row1, 0x44);
    __m128 tmp2 = _mm_shuffle_ps(row0, row1, 0xEE);
    __m128 tmp1 = _mm_shuffle_ps(row2, row3, 0x44);
    __m128 tmp3 = _mm_shuffle_ps(row2, row3, 0xEE);
    _mm_store_ps((float *) &luma->recon[64*i +  0], _mm_shuffle_ps(tmp0, tmp1, 0x88));
    _mm_store_ps((float *) &luma->recon[64*i + 16], _mm_shuffle_ps(tmp0, tmp1, 0xDD));
    _mm_store_ps((float *) &luma->recon[64*i + 32], _mm_shuffle_ps(tmp2, tmp3, 0x88));
    _mm_store_ps((float *) &luma->recon[64*i + 48], _mm_shuffle_ps(tmp2, tmp3, 0xDD));
  }
}

static void taa_h264_encode_inter_luma (
  const bool            intra,
  const int             qp,
  luma_t *              luma,
  mbcoeffs_t *  mbcoeffs)
{
  const int qp_div_6 = TAA_H264_DIV6 (qp);
  const int qp_mod_6 = qp - qp_div_6 * 6;
  const int qp_bits = Q_BITS + qp_div_6;
  const int shift_q_offset = 8 - qp_div_6 + (intra ? 0 : 1);
  const int qp_offset = MAX_Q_OFFSET >> shift_q_offset;

  __m128i predbuf[16];
  __m128i transbuf[32];

  int cost[16];
  TAA_H264_ALIGN (16) int8_t cbpflag[16]={0,};
  for (int i = 0; i < 4; i++)
  {
    //=================================================================
    //                   4 x 4 transpose luma->input
    //=================================================================
    __m128 row0, row1, row2, row3, tmp0, tmp1, tmp2, tmp3;
    row0 = _mm_load_ps((float *) &luma->input[(4*i+0) * MB_WIDTH_Y]);
    row1 = _mm_load_ps((float *) &luma->input[(4*i+1) * MB_WIDTH_Y]);
    row2 = _mm_load_ps((float *) &luma->input[(4*i+2) * MB_WIDTH_Y]);
    row3 = _mm_load_ps((float *) &luma->input[(4*i+3) * MB_WIDTH_Y]);
    tmp0 = _mm_shuffle_ps((row0), (row1), 0x44);
    tmp2 = _mm_shuffle_ps((row0), (row1), 0xEE);
    tmp1 = _mm_shuffle_ps((row2), (row3), 0x44);
    tmp3 = _mm_shuffle_ps((row2), (row3), 0xEE);
    __m128i const input[4] =
    {
      _mm_castps_si128(_mm_shuffle_ps(tmp0, tmp1, 0x88)),
      _mm_castps_si128(_mm_shuffle_ps(tmp0, tmp1, 0xDD)),
      _mm_castps_si128(_mm_shuffle_ps(tmp2, tmp3, 0x88)),
      _mm_castps_si128(_mm_shuffle_ps(tmp2, tmp3, 0xDD))
    };
    //=================================================================
    //                   4 x 4 transpose luma->recon
    //=================================================================
    row0 = _mm_load_ps((float *) &luma->recon[(4*i+0) * MB_WIDTH_Y]);
    row1 = _mm_load_ps((float *) &luma->recon[(4*i+1) * MB_WIDTH_Y]);
    row2 = _mm_load_ps((float *) &luma->recon[(4*i+2) * MB_WIDTH_Y]);
    row3 = _mm_load_ps((float *) &luma->recon[(4*i+3) * MB_WIDTH_Y]);
    tmp0 = _mm_shuffle_ps((row0), (row1), 0x44);
    tmp2 = _mm_shuffle_ps((row0), (row1), 0xEE);
    tmp1 = _mm_shuffle_ps((row2), (row3), 0x44);
    tmp3 = _mm_shuffle_ps((row2), (row3), 0xEE);

    predbuf[4*i+0] = _mm_castps_si128(_mm_shuffle_ps(tmp0, tmp1, 0x88));
    predbuf[4*i+1] = _mm_castps_si128(_mm_shuffle_ps(tmp0, tmp1, 0xDD));
    predbuf[4*i+2] = _mm_castps_si128(_mm_shuffle_ps(tmp2, tmp3, 0x88));
    predbuf[4*i+3] = _mm_castps_si128(_mm_shuffle_ps(tmp2, tmp3, 0xDD));

    for (int j = 0; j < 4; j++)
    {
      const int zz = zigzag_8x8[4*i + j];

      //=================================================================
      //                  Difference luma->input - luma->recon
      //=================================================================
      __m128i O = _mm_setzero_si128();
      transbuf[8*i+2*j+0] = _mm_sub_epi16(_mm_unpacklo_epi8(input[j], O), _mm_unpacklo_epi8(predbuf[4*i+j], O));
      transbuf[8*i+2*j+1] = _mm_sub_epi16(_mm_unpackhi_epi8(input[j], O), _mm_unpackhi_epi8(predbuf[4*i+j], O));

      //=================================================================
      //          4x4 transform + quant + dequant + run + level
      //=================================================================
      if (luma->do_4x4_enc[4*i + j])
      {
        cbpflag[4*i + j] = taa_h264_transform_quant_dequant_run_level (
          qp_div_6,
          qp_mod_6,
          qp_bits,
          qp_offset,
          &transbuf[8*i+2*j+0],
          &mbcoeffs->luma_ac[zz],
          &cost[zz]);
      }
      else
      {
        cost[zz] = 0;
      }


    }
  }
  int coeff_cost_8x8_1 =  cost[0] +  cost[1] +  cost[2] +  cost[3];
  int coeff_cost_8x8_2 =  cost[4] +  cost[5] +  cost[6] +  cost[7];
  int coeff_cost_8x8_3 =  cost[8] +  cost[9] + cost[10] + cost[11];
  int coeff_cost_8x8_4 = cost[12] + cost[13] + cost[14] + cost[15];

  if (coeff_cost_8x8_1 < COEFF_COST_LUMA_8X8_TH)
  {
    coeff_cost_8x8_1 = 0;
    cbpflag[0] = 0;
    cbpflag[1] = 0;
    cbpflag[4] = 0;
    cbpflag[5] = 0;
  }
  if (coeff_cost_8x8_2 < COEFF_COST_LUMA_8X8_TH)
  {
    coeff_cost_8x8_2 = 0;
    cbpflag[2] = 0;
    cbpflag[3] = 0;
    cbpflag[6] = 0;
    cbpflag[7] = 0;
  }
  if (coeff_cost_8x8_3 < COEFF_COST_LUMA_8X8_TH)
  {
    coeff_cost_8x8_3 = 0;
    cbpflag[8] = 0;
    cbpflag[9] = 0;
    cbpflag[12] = 0;
    cbpflag[13] = 0;
  }
  if (coeff_cost_8x8_4 < COEFF_COST_LUMA_8X8_TH)
  {
    coeff_cost_8x8_4 = 0;
    cbpflag[10] = 0;
    cbpflag[11] = 0;
    cbpflag[14] = 0;
    cbpflag[15] = 0;
  }
  const int sum_coeff_cost = coeff_cost_8x8_1 + coeff_cost_8x8_2 + coeff_cost_8x8_3 + coeff_cost_8x8_4;

  if (sum_coeff_cost < COEFF_COST_LUMA_16X16_TH)
  {
    mbcoeffs->luma_cbp = 0;
  }
  else
  {
    mbcoeffs->luma_cbp = (int16_t) _mm_movemask_epi8(*(__m128i *)cbpflag);
  }
  if (mbcoeffs->luma_cbp)
  {
    for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 4; j++)
      {
        if (cbpflag[4*i+j])
        {
          __m128i O = _mm_setzero_si128();
          taa_h264_inverse_transform_4x4_x86_64(&transbuf[8*i+2*j]);
          predbuf[4*i+j] = _mm_packus_epi16(_mm_add_epi16(transbuf[8*i+2*j+0], _mm_unpacklo_epi8(predbuf[4*i + j], O)),
                                            _mm_add_epi16(transbuf[8*i+2*j+1], _mm_unpackhi_epi8(predbuf[4*i + j], O)));
        }
      }
      __m128 row0 = _mm_castsi128_ps(predbuf[4*i+0]);
      __m128 row1 = _mm_castsi128_ps(predbuf[4*i+1]);
      __m128 row2 = _mm_castsi128_ps(predbuf[4*i+2]);
      __m128 row3 = _mm_castsi128_ps(predbuf[4*i+3]);
      __m128 tmp0 = _mm_shuffle_ps(row0, row1, 0x44);
      __m128 tmp2 = _mm_shuffle_ps(row0, row1, 0xEE);
      __m128 tmp1 = _mm_shuffle_ps(row2, row3, 0x44);
      __m128 tmp3 = _mm_shuffle_ps(row2, row3, 0xEE);
      _mm_store_ps((float *) &luma->recon[64*i +  0], _mm_shuffle_ps(tmp0, tmp1, 0x88));
      _mm_store_ps((float *) &luma->recon[64*i + 16], _mm_shuffle_ps(tmp0, tmp1, 0xDD));
      _mm_store_ps((float *) &luma->recon[64*i + 32], _mm_shuffle_ps(tmp2, tmp3, 0x88));
      _mm_store_ps((float *) &luma->recon[64*i + 48], _mm_shuffle_ps(tmp2, tmp3, 0xDD));
    }
  }
}

static void taa_h264_encode_chroma (
  const bool            intra,    // in
  const int             qp,       // in
  chroma_t *            chroma,
  mbcoeffs_t *  mbcoeffs  // out
  )
{
  const int qpc = qp_scale_chroma[qp];
  const int qp_div_6 = TAA_H264_DIV6 (qpc);
  const int qp_mod_6 = qpc - qp_div_6 * 6;
  const int qp_bits = Q_BITS + qp_div_6;
  const int shift_q_offset = 8 - qp_div_6 + (intra ? 0 : 1);
  const int qp_offset = MAX_Q_OFFSET_DC >> shift_q_offset;

  int16_t TAA_H264_ALIGN(64) transbuf[MB_SIZE_UV * 2];

  int num_coeffs = 0;
  const int num_dc_coeffs = 4;
  const int num_ac_coeffs_per_block = 15;

  int dcflag = 0;
  int cword = 0;

  __m128i predbuf_m128i[16];
  __m128i * transbuf_m128i = (__m128i *) transbuf;

  /* Find residual and do forward transform for both U and V */
  for (int i = 0; i < 2; i++)
  {
    __m128 row0, row1, row2, row3, tmp0, tmp1, tmp2, tmp3;
    row0 = _mm_load_ps((float *) &chroma->input[(4*i+0) * MB_STRIDE_UV]);
    row1 = _mm_load_ps((float *) &chroma->input[(4*i+1) * MB_STRIDE_UV]);
    row2 = _mm_load_ps((float *) &chroma->input[(4*i+2) * MB_STRIDE_UV]);
    row3 = _mm_load_ps((float *) &chroma->input[(4*i+3) * MB_STRIDE_UV]);
    tmp0 = _mm_shuffle_ps((row0), (row1), 0x44);
    tmp2 = _mm_shuffle_ps((row0), (row1), 0xEE);
    tmp1 = _mm_shuffle_ps((row2), (row3), 0x44);
    tmp3 = _mm_shuffle_ps((row2), (row3), 0xEE);
    __m128i const input[4] =
    {
      _mm_castps_si128(_mm_shuffle_ps(tmp0, tmp1, 0x88)),
      _mm_castps_si128(_mm_shuffle_ps(tmp0, tmp1, 0xDD)),
      _mm_castps_si128(_mm_shuffle_ps(tmp2, tmp3, 0x88)),
      _mm_castps_si128(_mm_shuffle_ps(tmp2, tmp3, 0xDD))
    };
    //=================================================================
    //                   4 x 4 transpose ipred_y
    //=================================================================
    row0 = _mm_load_ps((float *) &chroma->recon[(4*i+0) * MB_STRIDE_UV]);
    row1 = _mm_load_ps((float *) &chroma->recon[(4*i+1) * MB_STRIDE_UV]);
    row2 = _mm_load_ps((float *) &chroma->recon[(4*i+2) * MB_STRIDE_UV]);
    row3 = _mm_load_ps((float *) &chroma->recon[(4*i+3) * MB_STRIDE_UV]);
    tmp0 = _mm_shuffle_ps((row0), (row1), 0x44);
    tmp2 = _mm_shuffle_ps((row0), (row1), 0xEE);
    tmp1 = _mm_shuffle_ps((row2), (row3), 0x44);
    tmp3 = _mm_shuffle_ps((row2), (row3), 0xEE);

    predbuf_m128i[4*i+0] = _mm_castps_si128(_mm_shuffle_ps(tmp0, tmp1, 0x88));
    predbuf_m128i[4*i+1] = _mm_castps_si128(_mm_shuffle_ps(tmp0, tmp1, 0xDD));
    predbuf_m128i[4*i+2] = _mm_castps_si128(_mm_shuffle_ps(tmp2, tmp3, 0x88));
    predbuf_m128i[4*i+3] = _mm_castps_si128(_mm_shuffle_ps(tmp2, tmp3, 0xDD));

    __m128i O = _mm_setzero_si128();
    transbuf_m128i[4*i+ 0] = _mm_sub_epi16(_mm_unpacklo_epi8(input[0], O), _mm_unpacklo_epi8(predbuf_m128i[4*i+0], O));
    transbuf_m128i[4*i+ 1] = _mm_sub_epi16(_mm_unpackhi_epi8(input[0], O), _mm_unpackhi_epi8(predbuf_m128i[4*i+0], O));
    taa_h264_transform_4x4_x86_64(&transbuf_m128i[4*i+ 0]);
    transbuf_m128i[4*i+ 2] = _mm_sub_epi16(_mm_unpacklo_epi8(input[1], O), _mm_unpacklo_epi8(predbuf_m128i[4*i+1], O));
    transbuf_m128i[4*i+ 3] = _mm_sub_epi16(_mm_unpackhi_epi8(input[1], O), _mm_unpackhi_epi8(predbuf_m128i[4*i+1], O));
    taa_h264_transform_4x4_x86_64(&transbuf_m128i[4*i+ 2]);
    transbuf_m128i[4*i+ 8] = _mm_sub_epi16(_mm_unpacklo_epi8(input[2], O), _mm_unpacklo_epi8(predbuf_m128i[4*i+2], O));
    transbuf_m128i[4*i+ 9] = _mm_sub_epi16(_mm_unpackhi_epi8(input[2], O), _mm_unpackhi_epi8(predbuf_m128i[4*i+2], O));
    taa_h264_transform_4x4_x86_64(&transbuf_m128i[4*i+ 8]);
    transbuf_m128i[4*i+10] = _mm_sub_epi16(_mm_unpacklo_epi8(input[3], O), _mm_unpacklo_epi8(predbuf_m128i[4*i+3], O));
    transbuf_m128i[4*i+11] = _mm_sub_epi16(_mm_unpackhi_epi8(input[3], O), _mm_unpackhi_epi8(predbuf_m128i[4*i+3], O));
    taa_h264_transform_4x4_x86_64(&transbuf_m128i[4*i+10]);
  }

  /* Handle the DC coefficients */
  for (int uv = 0; uv < 2; uv++)
  {
    int16_t * transbuf_uv = &transbuf[MB_SIZE_UV * uv];

    taa_h264_hadamard_2x2 (transbuf_uv);

    dcflag |= taa_h264_quant_dequant_2x2_dc_chroma (
      qp_div_6,
      qp_mod_6,
      qp_bits,
      qp_offset,
      transbuf_uv,
      &mbcoeffs->chroma_dc[uv]);

    taa_h264_inverse_hadamard_2x2 (transbuf_uv);

    num_coeffs += num_dc_coeffs;
  }


  /* Quantize the AC coefficients */
  int coeff_cost = 0;
  int count = 2 * NUM_4x4_BLOCKS_UV - 1;

  for (int i = 0; i < 2; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      int cost;
      int cbpflag = taa_h264_quant_dequant_4x4_skip (
        qp_div_6,
        qp_mod_6,
        qp_bits,
        qp_offset>>1,
        &transbuf[MB_SIZE_UV * i + j * 4 * 4],
        &mbcoeffs->chroma_ac[i * 4 + j],
        &cost);

      coeff_cost += cost;

      cword |= (cbpflag << count);
      count--;
      num_coeffs += num_ac_coeffs_per_block;
    }
  }

  if (cword && coeff_cost <= COEFF_COST_CHROMA_TH)
  {
    cword = 0;
    /* TODO: For now, make sure all all coeff are zero. Should create
     * a inverse transform dc only function.*/
    for (int k = 0; k < NUM_4x4_BLOCKS_UV * 2; k++)
      for (int l = 1; l < 16; l++)
        transbuf[k * 16 + l] = 0;
  }

  count = 2 * NUM_4x4_BLOCKS_UV - 1;

  const int count_a[8] = {7, 6, 3, 2, 5, 4,  1, 0};
  for (int i = 0; i < 2; i++)
  {
    bool cbp_this_block;

    cbp_this_block = (cword >> count_a[4*i+0]) & 0x01;
    if (dcflag || cbp_this_block)
    {
      __m128i O = _mm_setzero_si128();
      taa_h264_inverse_transform_4x4_x86_64(&transbuf_m128i[4*i+0]);
      predbuf_m128i[4*i+0] = _mm_packus_epi16(_mm_add_epi16(transbuf_m128i[4*i+0], _mm_unpacklo_epi8(predbuf_m128i[4*i+0], O)),
                                              _mm_add_epi16(transbuf_m128i[4*i+1], _mm_unpackhi_epi8(predbuf_m128i[4*i+0], O)));
    }
    cbp_this_block = (cword >> count_a[4*i+1]) & 0x01;
    if (dcflag || cbp_this_block)
    {
      __m128i O = _mm_setzero_si128();
      taa_h264_inverse_transform_4x4_x86_64(&transbuf_m128i[4*i+2]);
      predbuf_m128i[4*i+1] = _mm_packus_epi16(_mm_add_epi16(transbuf_m128i[4*i+2], _mm_unpacklo_epi8(predbuf_m128i[4*i+1], O)),
                                              _mm_add_epi16(transbuf_m128i[4*i+3], _mm_unpackhi_epi8(predbuf_m128i[4*i+1], O)));
    }
    cbp_this_block = (cword >> count_a[4*i+2]) & 0x01;
    if (dcflag || cbp_this_block)
    {
      __m128i O = _mm_setzero_si128();
      taa_h264_inverse_transform_4x4_x86_64(&transbuf_m128i[4*i+8]);
      predbuf_m128i[4*i+2] = _mm_packus_epi16(_mm_add_epi16(transbuf_m128i[4*i+8], _mm_unpacklo_epi8(predbuf_m128i[4*i+2], O)),
                                              _mm_add_epi16(transbuf_m128i[4*i+9], _mm_unpackhi_epi8(predbuf_m128i[4*i+2], O)));
    }
    cbp_this_block = (cword >> count_a[4*i+3]) & 0x01;
    if (dcflag || cbp_this_block)
    {
      __m128i O = _mm_setzero_si128();
      taa_h264_inverse_transform_4x4_x86_64(&transbuf_m128i[4*i+10]);
      predbuf_m128i[4*i+3] = _mm_packus_epi16(_mm_add_epi16(transbuf_m128i[4*i+10], _mm_unpacklo_epi8(predbuf_m128i[4*i+3], O)),
                                              _mm_add_epi16(transbuf_m128i[4*i+11], _mm_unpackhi_epi8(predbuf_m128i[4*i+3], O)));
    }

    __m128 row0 = _mm_castsi128_ps(predbuf_m128i[4*i+0]);
    __m128 row1 = _mm_castsi128_ps(predbuf_m128i[4*i+1]);
    __m128 row2 = _mm_castsi128_ps(predbuf_m128i[4*i+2]);
    __m128 row3 = _mm_castsi128_ps(predbuf_m128i[4*i+3]);
    __m128 tmp0 = _mm_shuffle_ps(row0, row1, 0x44);
    __m128 tmp2 = _mm_shuffle_ps(row0, row1, 0xEE);
    __m128 tmp1 = _mm_shuffle_ps(row2, row3, 0x44);
    __m128 tmp3 = _mm_shuffle_ps(row2, row3, 0xEE);
    _mm_store_ps((float *) &chroma->recon[64*i +  0], _mm_shuffle_ps(tmp0, tmp1, 0x88));
    _mm_store_ps((float *) &chroma->recon[64*i + 16], _mm_shuffle_ps(tmp0, tmp1, 0xDD));
    _mm_store_ps((float *) &chroma->recon[64*i + 32], _mm_shuffle_ps(tmp2, tmp3, 0x88));
    _mm_store_ps((float *) &chroma->recon[64*i + 48], _mm_shuffle_ps(tmp2, tmp3, 0xDD));
  }

  mbcoeffs->chroma_cbp = (int16_t)((dcflag != 0) | ((cword != 0) << 1));
}

static void taa_h264_encode_intra_chroma (
  const bool            intra,
  const int             avail_flags,
  const int             qp,
  chroma_t *            chroma,
  const uint8_t *       recon_top_u,
  const uint8_t *       recon_top_v,
  const uint8_t *       recon_left_u,
  const uint8_t *       recon_left_v,
  imode8_t *    best_i8x8_mode_chroma,
  mbcoeffs_t *  mbcoeffs)
{
  TAA_H264_ALIGN(64) uint8_t predbuf_uv[MB_SIZE_UV * 2 * NUM_IMODES_UV];

  imode8_t best_mode = DC_PRED_8;
  uint8_t *       ipred_uv   = predbuf_uv;
  const uint8_t * top_u      = recon_top_u;
  const uint8_t * top_v      = recon_top_v;
  const uint8_t * left_u     = recon_left_u;
  const uint8_t * left_v     = recon_left_v;
  bool left_avail = MB_LEFT (avail_flags);
  bool up_avail = MB_UP (avail_flags);
  unsigned min_sad = UINT_MAX;

  int mode_avail_mask = 0x0;

  const int mode_size = MB_SIZE_UV * 2;
  uint8_t * pred_u = &ipred_uv [0];
  uint8_t * pred_v = &ipred_uv [MB_WIDTH_UV];


  taa_h264_intra_pred_chroma_dc (
    &pred_u[DC_PRED_8 * mode_size], left_u, top_u, left_avail, up_avail);
  taa_h264_intra_pred_chroma_dc (
    &pred_v[DC_PRED_8 * mode_size], left_v, top_v, left_avail, up_avail);
  mode_avail_mask |= 1 << DC_PRED_8;

  if (up_avail)
  {
    taa_h264_intra_pred_chroma_vert (
      &pred_u[VERT_PRED_8 * mode_size], top_u);
    taa_h264_intra_pred_chroma_vert (
      &pred_v[VERT_PRED_8 * mode_size], top_v);
    mode_avail_mask |= 1 << VERT_PRED_8;
  }

  if (left_avail)
  {
    taa_h264_intra_pred_chroma_hor (
      &pred_u[HOR_PRED_8 * mode_size], left_u);
    taa_h264_intra_pred_chroma_hor (
      &pred_v[HOR_PRED_8 * mode_size], left_v);
    mode_avail_mask |= 1 << HOR_PRED_8;
  }

  for (int mode = 0; mode < NUM_IMODES_UV; mode++)
  {
    if ((mode_avail_mask >> mode) & 0x1)
    {
      unsigned sad = taa_h264_sad_sub (&pred_u[mode * mode_size],
                                   chroma->input, MB_HEIGHT_UV, MB_WIDTH_UV * 2,
                                   MB_STRIDE_UV, MB_STRIDE_UV);
      if (sad < min_sad)
      {
        min_sad = sad;
        best_mode = (imode8_t) mode;
      }
    }
  }
  _mm_store_pd((double *)(&chroma->recon[0*MB_STRIDE_UV]), _mm_load_pd((double *)(&predbuf_uv[best_mode * MB_SIZE_UV * 2] +  0*MB_STRIDE_UV)));
  _mm_store_pd((double *)(&chroma->recon[1*MB_STRIDE_UV]), _mm_load_pd((double *)(&predbuf_uv[best_mode * MB_SIZE_UV * 2] +  1*MB_STRIDE_UV)));
  _mm_store_pd((double *)(&chroma->recon[2*MB_STRIDE_UV]), _mm_load_pd((double *)(&predbuf_uv[best_mode * MB_SIZE_UV * 2] +  2*MB_STRIDE_UV)));
  _mm_store_pd((double *)(&chroma->recon[3*MB_STRIDE_UV]), _mm_load_pd((double *)(&predbuf_uv[best_mode * MB_SIZE_UV * 2] +  3*MB_STRIDE_UV)));
  _mm_store_pd((double *)(&chroma->recon[4*MB_STRIDE_UV]), _mm_load_pd((double *)(&predbuf_uv[best_mode * MB_SIZE_UV * 2] +  4*MB_STRIDE_UV)));
  _mm_store_pd((double *)(&chroma->recon[5*MB_STRIDE_UV]), _mm_load_pd((double *)(&predbuf_uv[best_mode * MB_SIZE_UV * 2] +  5*MB_STRIDE_UV)));
  _mm_store_pd((double *)(&chroma->recon[6*MB_STRIDE_UV]), _mm_load_pd((double *)(&predbuf_uv[best_mode * MB_SIZE_UV * 2] +  6*MB_STRIDE_UV)));
  _mm_store_pd((double *)(&chroma->recon[7*MB_STRIDE_UV]), _mm_load_pd((double *)(&predbuf_uv[best_mode * MB_SIZE_UV * 2] +  7*MB_STRIDE_UV)));


  taa_h264_encode_chroma (
    intra,
    qp,
    chroma,
    mbcoeffs);

  *best_i8x8_mode_chroma = best_mode;
}

static void taa_h264_late_skip_check (
  mbinfo_t * currmb)
{
  if (currmb->mbtype == P_16x16
      && !(currmb->mbcoeffs.luma_cbp || currmb->mbcoeffs.chroma_cbp)
      && currmb->mv1.x == currmb->mvskip.x
      && currmb->mv1.y == currmb->mvskip.y
      && currmb->mv1.ref_num == currmb->mvskip.ref_num)
  {
    currmb->mbtype = P_SKIP;
  }
}

#ifdef UNIT_TEST
static
#endif
#ifdef HAVE_SSSE3
void taa_h264_encode_mb_ssse3 (
#else
void taa_h264_encode_mb (
#endif
  frameinfo_t *  frameinfo,
  mbinfo_t *     currmb,
  const bool     intra,
  const unsigned coding_options)
{
  const bool luma_done = intra && currmb->mbtype == I_4x4;
  if (!luma_done)
    currmb->mbcoeffs.luma_cbp = 0;
  currmb->mbcoeffs.chroma_cbp = 0;

  if (currmb->mbtype == I_4x4)      // Intra 4x4
  {
    if (!luma_done)
    {
      const int imodes_offset = currmb->mbpos * NUM_4x4_BLOCKS_Y;
      unsigned i4x4cost = taa_h264_encode_i4x4_luma_ (
        intra,
        currmb->mquant,
        currmb->avail_flags,
        currmb->mbxmax,
        currmb->lambda,
        &currmb->luma,
        currmb->recon_left_y,
        currmb->recon_top_y,
        currmb->recon_topleft_y,
        coding_options,
        currmb->pred_intra_modes,
        &frameinfo->intra_modes [imodes_offset],
        &currmb->mbcoeffs);

      // The shift was found by auto tuning
      const int shift = 1;
      const int diff_approx_vs_real = abs (currmb->approx_i4x4_cost - i4x4cost);
      frameinfo->approx_i4x4_penalty += (diff_approx_vs_real - frameinfo->approx_i4x4_penalty) >> shift;
    }

    taa_h264_encode_intra_chroma (
      intra,
      currmb->avail_flags,
      currmb->mquant,
      &currmb->chroma,
      currmb->recon_top_u,
      currmb->recon_top_v,
      currmb->recon_left_u,
      currmb->recon_left_v,
      &currmb->best_i8x8_mode_chroma,
      &currmb->mbcoeffs);

  }
  else if (currmb->mbtype == I_16x16)  // Intra 16x16
  {
    taa_h264_encode_intra_16x16_luma (
      intra,
      currmb->mquant,
      currmb->best_i16x16_mode,
      &currmb->luma,
      currmb->pred_i16x16_y,
      &currmb->mbcoeffs);

    taa_h264_encode_intra_chroma (
      intra,
      currmb->avail_flags,
      currmb->mquant,
      &currmb->chroma,
      currmb->recon_top_u,
      currmb->recon_top_v,
      currmb->recon_left_u,
      currmb->recon_left_v,
      &currmb->best_i8x8_mode_chroma,
      &currmb->mbcoeffs);
  }
  else
  {
    if (currmb->encode_luma)
    {
      taa_h264_encode_inter_luma (
        intra,
        currmb->mquant,
        &currmb->luma,
        &currmb->mbcoeffs);
    }
    if (currmb->encode_chroma)
    {
      taa_h264_encode_chroma (
        intra,
        currmb->mquant,
        &currmb->chroma,
        &currmb->mbcoeffs);
    }
  }

  taa_h264_late_skip_check (currmb);
}
