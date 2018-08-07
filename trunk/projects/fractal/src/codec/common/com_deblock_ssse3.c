#include "com_error.h"
#include "com_deblock.h"
#include "com_mbutils.h"
#include "com_common.h"

#pragma warning (disable:981)

static const uint8_t qp_scale_chroma[52] = {
  0,  1,  2,  3,  4,  5,  6,  7,
  8,  9, 10, 11, 12, 13, 14, 15,
  16, 17, 18, 19, 20, 21, 22, 23,
  24, 25, 26, 27, 28, 29, 29, 30,
  31, 32, 32, 33, 34, 34, 35, 35,
  36, 36, 37, 37, 37, 38, 38, 38,
  39, 39, 39, 39
};

static int taa_h264_mv_diff (
  const mv_t * a,
  const mv_t * b)
{
  bool differ = abs (a->x - b->x) >= 4;
  differ |= abs (a->y - b->y) >= 4;
  differ |= a->ref_num != b->ref_num;
  return differ ? 1 : 0;
}


static uint32_t taa_h264_mb_mv_diff (
  const mv_t * curr,
  const mv_t * left,
  const mv_t * top)
{
  uint32_t vert = 0x0;
  uint32_t horz = 0x0;

  for (int i = 0; i < NUM_4x4_BLOCKS_Y; i++)
  {
    const mv_t * mv_left = (i % 4) ? &curr[i - 1] : &left[i + 3];
    const mv_t * mv_top = (i / 4) ? &curr[i - 4] : &top[i + 12];
    int v = taa_h264_mv_diff (&curr[i], mv_left);
    int h = taa_h264_mv_diff (&curr[i], mv_top);
    vert |= v << i;
    horz |= h << i;
  }

  return (horz << 16) | vert;
}


static void taa_h264_mb_compute_strength (
  const mbinfo_store_t * mb,
  const mv_t *           motion_vectors_ptr,
  int                    mbxmax,
  mb_deblock_t *         deb)
{
  uint32_t do_filter = 0xffffffff;
  int vert_strength1 = 0x0;
  int vert_strength2 = 0x0;
  int horz_strength1 = 0x0;
  int horz_strength2 = 0x0;

  bool left_avail = MB_LEFT (mb->flags);
  bool top_avail = MB_UP (mb->flags);

  const mbinfo_store_t no_deblock = {
    .cbp_blk = 0x0,
    .flags = 0x0,
    .qp = 0,
  };
  const mbinfo_store_t * curr = mb;
  const mbinfo_store_t * left = left_avail ? &mb[-1] : &no_deblock;
  const mbinfo_store_t * top = top_avail ? &mb[-mbxmax] : &no_deblock;
  const mv_t * mv_curr = motion_vectors_ptr;
  const mv_t * mv_left = &motion_vectors_ptr[-NUM_4x4_BLOCKS_Y];
  const mv_t * mv_top = &motion_vectors_ptr[-(mbxmax * NUM_4x4_BLOCKS_Y)];

  if (!left_avail || (left->flags & MB_TYPE_INTRA_))
    mv_left = mv_curr;
  if (!top_avail || (top->flags & MB_TYPE_INTRA_))
    mv_top = mv_curr;

  if (!(curr->flags & MB_TYPE_INTRA_))
  {
    vert_strength1 = curr->cbp_blk;
    vert_strength1 |= (curr->cbp_blk << 1) & 0xeeee;
    vert_strength1 |= (left->cbp_blk >> 3) & 0x1111;
    do_filter = vert_strength1;

    horz_strength1 = curr->cbp_blk;
    horz_strength1 |= (curr->cbp_blk << 4);
    horz_strength1 |= (top->cbp_blk >> 12) & 0x000f;
    do_filter |= horz_strength1 << 16;

    if ((left->mbtype <= P_16x16) & (top->mbtype <= P_16x16) & (mb->mbtype <= P_16x16))
    {
      uint32_t mv_diff = 0x0;
      if ((abs(mv_curr[0].x -  mv_top[0].x) >= 4) | (abs(mv_curr[0].y - mv_top[0].y)  >= 4) | (mv_curr[0].ref_num !=  mv_top[0].ref_num)) mv_diff |= 0x000f0000;
      if ((abs(mv_curr[0].x - mv_left[0].x) >= 4) | (abs(mv_curr[0].y - mv_left[0].y) >= 4) | (mv_curr[0].ref_num != mv_left[0].ref_num)) mv_diff |= 0x00001111;
      do_filter |= mv_diff;
    }
    else if ((left->mbtype < P_8x8) & (top->mbtype < P_8x8) & (mb->mbtype < P_8x8))
    {
      uint32_t mv_diff = 0x0;
      if ((abs(mv_curr[ 0].x - mv_left[ 3].x) >= 4) | (abs(mv_curr[ 0].y - mv_left [3].y) >= 4) | (mv_curr[ 0].ref_num != mv_left[ 3].ref_num)) mv_diff |= 0x00000011;
      if ((abs(mv_curr[ 2].x - mv_curr[ 1].x) >= 4) | (abs(mv_curr[ 2].y - mv_curr [1].y) >= 4) | (mv_curr[ 2].ref_num != mv_curr[ 1].ref_num)) mv_diff |= 0x00000044;
      if ((abs(mv_curr[ 8].x - mv_left[11].x) >= 4) | (abs(mv_curr[ 8].y - mv_left[11].y) >= 4) | (mv_curr[ 8].ref_num != mv_left[11].ref_num)) mv_diff |= 0x00001100;
      if ((abs(mv_curr[10].x - mv_curr[ 9].x) >= 4) | (abs(mv_curr[10].y - mv_curr [9].y) >= 4) | (mv_curr[10].ref_num != mv_curr[ 9].ref_num)) mv_diff |= 0x00004400;
      if ((abs(mv_curr[ 0].x - mv_top [12].x) >= 4) | (abs(mv_curr[ 0].y - mv_top [12].y) >= 4) | (mv_curr[ 0].ref_num != mv_top [12].ref_num)) mv_diff |= 0x00030000;
      if ((abs(mv_curr[ 2].x - mv_top [14].x) >= 4) | (abs(mv_curr[ 2].y - mv_top [14].y) >= 4) | (mv_curr[ 2].ref_num != mv_top [14].ref_num)) mv_diff |= 0x000C0000;
      if ((abs(mv_curr[ 8].x - mv_curr[ 4].x) >= 4) | (abs(mv_curr[ 8].y - mv_curr [4].y) >= 4) | (mv_curr[ 8].ref_num != mv_curr[ 4].ref_num)) mv_diff |= 0x03000000;
      if ((abs(mv_curr[10].x - mv_curr[ 6].x) >= 4) | (abs(mv_curr[10].y - mv_curr [6].y) >= 4) | (mv_curr[10].ref_num != mv_curr[ 6].ref_num)) mv_diff |= 0x0C000000;
      do_filter |= mv_diff;
    }
    else
    {
      do_filter |= taa_h264_mb_mv_diff (mv_curr, mv_left, mv_top);
    }

    if ((left->flags) & MB_TYPE_INTRA_)
    {
      do_filter |= 0x00001111;
      vert_strength2 = 0x1111;
      vert_strength1 &= 0xeeee;
    }

    if ((top->flags) & MB_TYPE_INTRA_)
    {
      do_filter |= 0x000f0000;
      horz_strength2 = 0x000f;
      horz_strength1 &= 0xfff0;
    }

    if (!left_avail)
      do_filter &= 0xffffeeee;
    if (!top_avail)
      do_filter &= 0xfff0ffff;
  }
  else
  {
    if (!left_avail)
      do_filter &= 0xffffeeee;
    if (!top_avail)
      do_filter &= 0xfff0ffff;

    vert_strength2 = do_filter & 0xffff;
    horz_strength2 = do_filter >> 16;
  }

  deb->do_filter = do_filter;
  deb->vert_strength1 = (uint16_t) vert_strength1;
  deb->vert_strength2 = (uint16_t) vert_strength2;
  deb->horz_strength1 = (uint16_t) horz_strength1;
  deb->horz_strength2 = (uint16_t) horz_strength2;

  deb->filter_offset_a = mb->filter_offset_a;
  deb->filter_offset_b = mb->filter_offset_b;

  deb->curr_qp = curr->qp;
  deb->left_qp = (uint8_t)((left->qp + curr->qp + 1) >> 1);
  deb->top_qp =  (uint8_t)((top->qp + curr->qp + 1) >> 1);
  deb->curr_qpc = qp_scale_chroma[curr->qp];
  deb->left_qpc = (uint8_t)((qp_scale_chroma[left->qp] + qp_scale_chroma[curr->qp] + 1) >> 1);
  deb->top_qpc = (uint8_t)((qp_scale_chroma[top->qp] + qp_scale_chroma[curr->qp] + 1) >> 1);
}


/* =============================================================================== */
/* */
/* (C) Copyright 2008 TANDBERG Telecom AS */
/* */
/* =============================================================================== */
#include <emmintrin.h>


/* #if ( __INTEL_COMPILER == 1000 ) */
/* #define  */
/* #endif */

/* Padded with 12 duplicate entries before and after Table 8-14. This is
 * because filter offset can be ±12. Thus we avoid clipping. */

static const uint8_t alpha_tab[52 + 24] = {
  0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0,  0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   4,   4,
  5,   6,   7,   8,   9,  10,  12,  13,  15,
  17,  20,  22,  25,  28,  32,  36,  40,  45,
  50,  56,  63,  71,  80,  90, 101, 113, 127,
  144, 162, 182, 203, 226, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};

static const uint8_t beta_tab[52 + 24]  = {
  0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0,  0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   2,   2,
  2,   3,   3,   3,   3,   4,   4,   4,   6,
  6,   7,   7,   8,   8,   9,   9,   10,  10,
  11,  11,  12,  12,  13,  13,  14,   14,  15,
  15,  16,  16,  17,  17,  18,  18,
  18,  18,  18,  18,  18,  18,  18,  18,  18, 18, 18, 18
};

static const int tc0_tab[3 * (52 + 24)] = {
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,

  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00010001,
  0x00000000, 0x00000000, 0x00010001, 0x00000000, 0x00000000, 0x00010001,
  0x00000000, 0x00000000, 0x00010001, 0x00000000, 0x00010001, 0x00010001,
  0x00000000, 0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00010001,
  0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00010001,
  0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00020002,
  0x00010001, 0x00010001, 0x00020002, 0x00010001, 0x00010001, 0x00020002,
  0x00010001, 0x00010001, 0x00020002, 0x00010001, 0x00020002, 0x00030003,
  0x00010001, 0x00020002, 0x00030003, 0x00020002, 0x00020002, 0x00030003,
  0x00020002, 0x00020002, 0x00040004, 0x00020002, 0x00030003, 0x00040004,
  0x00020002, 0x00030003, 0x00040004, 0x00030003, 0x00030003, 0x00050005,
  0x00030003, 0x00040004, 0x00060006, 0x00030003, 0x00040004, 0x00060006,
  0x00040004, 0x00050005, 0x00070007, 0x00040004, 0x00050005, 0x00080008,
  0x00040004, 0x00060006, 0x00090009, 0x00050005, 0x00070007, 0x000a000a,
  0x00060006, 0x00080008, 0x000b000b, 0x00060006, 0x00080008, 0x000d000d,
  0x00070007, 0x000a000a, 0x000e000e, 0x00080008, 0x000b000b, 0x00100010,
  0x00090009, 0x000c000c, 0x00120012, 0x000a000a, 0x000d000d, 0x00140014,
  0x000b000b, 0x000f000f, 0x00170017, 0x000d000d, 0x00110011, 0x00190019,

  0x000d000d, 0x00110011, 0x00190019, 0x000d000d, 0x00110011, 0x00190019,
  0x000d000d, 0x00110011, 0x00190019, 0x000d000d, 0x00110011, 0x00190019,
  0x000d000d, 0x00110011, 0x00190019, 0x000d000d, 0x00110011, 0x00190019,
  0x000d000d, 0x00110011, 0x00190019, 0x000d000d, 0x00110011, 0x00190019,
  0x000d000d, 0x00110011, 0x00190019, 0x000d000d, 0x00110011, 0x00190019,
  0x000d000d, 0x00110011, 0x00190019, 0x000d000d, 0x00110011, 0x00190019
};

static
void TAA_H264_16X16_TRANSPOSE(
  __m128i X[16],
  __m128i Y[16])
{
  __m128i x3, x7, x11, x15, x19, x23, x27, x31, x4, x8, x12, x16, x20, x24, x28, x32;
  __m128i y17, y19, y21, y23, y25, y27, y29, y31, y18, y20, y22, y24, y26, y28, y30, y32;
  __m128i z17, z19, z21, z23, z25, z27, z29, z31, z18, z20, z22, z24, z26, z28, z30, z32;
  x3  = _mm_unpacklo_epi8(X[ 0], X[ 1]);  x4 = _mm_unpackhi_epi8(X[ 0], X[ 1]);
  x7  = _mm_unpacklo_epi8(X[ 2], X[ 3]);  x8 = _mm_unpackhi_epi8(X[ 2], X[ 3]);
  x11 = _mm_unpacklo_epi8(X[ 4], X[ 5]); x12 = _mm_unpackhi_epi8(X[ 4], X[ 5]);
  x15 = _mm_unpacklo_epi8(X[ 6], X[ 7]); x16 = _mm_unpackhi_epi8(X[ 6], X[ 7]);
  x19 = _mm_unpacklo_epi8(X[ 8], X[ 9]); x20 = _mm_unpackhi_epi8(X[ 8], X[ 9]);
  x23 = _mm_unpacklo_epi8(X[10], X[11]); x24 = _mm_unpackhi_epi8(X[10], X[11]);
  x27 = _mm_unpacklo_epi8(X[12], X[13]); x28 = _mm_unpackhi_epi8(X[12], X[13]);
  x31 = _mm_unpacklo_epi8(X[14], X[15]); x32 = _mm_unpackhi_epi8(X[14], X[15]);
  y17 = _mm_unpacklo_epi16( x3,  x7);    y18 = _mm_unpackhi_epi16( x3, x7);
  y19 = _mm_unpacklo_epi16( x4,  x8);    y20 = _mm_unpackhi_epi16( x4, x8);
  y21 = _mm_unpacklo_epi16(x11, x15);    y22 = _mm_unpackhi_epi16(x11, x15);
  y23 = _mm_unpacklo_epi16(x12, x16);    y24 = _mm_unpackhi_epi16(x12, x16);
  y25 = _mm_unpacklo_epi16(x19, x23);    y26 = _mm_unpackhi_epi16(x19, x23);
  y27 = _mm_unpacklo_epi16(x20, x24);    y28 = _mm_unpackhi_epi16(x20, x24);
  y29 = _mm_unpacklo_epi16(x27, x31);    y30 = _mm_unpackhi_epi16(x27, x31);
  y31 = _mm_unpacklo_epi16(x28, x32);    y32 = _mm_unpackhi_epi16(x28, x32);
  z17 = _mm_unpacklo_epi32(y17, y21);    z18 = _mm_unpackhi_epi32(y17, y21);
  z19 = _mm_unpacklo_epi32(y18, y22);    z20 = _mm_unpackhi_epi32(y18, y22);
  z21 = _mm_unpacklo_epi32(y19, y23);    z22 = _mm_unpackhi_epi32(y19, y23);
  z23 = _mm_unpacklo_epi32(y20, y24);    z24 = _mm_unpackhi_epi32(y20, y24);
  z25 = _mm_unpacklo_epi32(y25, y29);    z26 = _mm_unpackhi_epi32(y25, y29);
  z27 = _mm_unpacklo_epi32(y26, y30);    z28 = _mm_unpackhi_epi32(y26, y30);
  z29 = _mm_unpacklo_epi32(y27, y31);    z30 = _mm_unpackhi_epi32(y27, y31);
  z31 = _mm_unpacklo_epi32(y28, y32);    z32 = _mm_unpackhi_epi32(y28, y32);
  Y[ 0] = _mm_unpacklo_epi64(z17, z25); Y[ 1] = _mm_unpackhi_epi64(z17, z25);
  Y[ 2] = _mm_unpacklo_epi64(z18, z26); Y[ 3] = _mm_unpackhi_epi64(z18, z26);
  Y[ 4] = _mm_unpacklo_epi64(z19, z27); Y[ 5] = _mm_unpackhi_epi64(z19, z27);
  Y[ 6] = _mm_unpacklo_epi64(z20, z28); Y[ 7] = _mm_unpackhi_epi64(z20, z28);
  Y[ 8] = _mm_unpacklo_epi64(z21, z29); Y[ 9] = _mm_unpackhi_epi64(z21, z29);
  Y[10] = _mm_unpacklo_epi64(z22, z30); Y[11] = _mm_unpackhi_epi64(z22, z30);
  Y[12] = _mm_unpacklo_epi64(z23, z31); Y[13] = _mm_unpackhi_epi64(z23, z31);
  Y[14] = _mm_unpacklo_epi64(z24, z32); Y[15] = _mm_unpackhi_epi64(z24, z32);
}

static
void TAA_H264_2X8X8_TRANSPOSE(
  __m128i X[8],
  __m128i Y[8])
{
  __m128i x3, x4, x7, x8, x11, x12, x15, x16, y17, y18, y19, y20, y21, y22, y23, y24, z17, z18, z19, z20, z21, z22, z23, z24;
  x3  = _mm_unpacklo_epi8(X[0], X[1]);   x4 = _mm_unpackhi_epi8(X[0], X[1]);
  x7  = _mm_unpacklo_epi8(X[2], X[3]);   x8 = _mm_unpackhi_epi8(X[2], X[3]);
  x11 = _mm_unpacklo_epi8(X[4], X[5]);  x12 = _mm_unpackhi_epi8(X[4], X[5]);
  x15 = _mm_unpacklo_epi8(X[6], X[7]);  x16 = _mm_unpackhi_epi8(X[6], X[7]);
  y17 = _mm_unpacklo_epi16( x3,  x7);   y18 = _mm_unpackhi_epi16( x3, x7);
  y19 = _mm_unpacklo_epi16( x4,  x8);   y20 = _mm_unpackhi_epi16( x4, x8);
  y21 = _mm_unpacklo_epi16(x11, x15);   y22 = _mm_unpackhi_epi16(x11, x15);
  y23 = _mm_unpacklo_epi16(x12, x16);   y24 = _mm_unpackhi_epi16(x12, x16);
  z17 = _mm_unpacklo_epi32(y17, y21);   z18 = _mm_unpackhi_epi32(y17, y21);
  z19 = _mm_unpacklo_epi32(y18, y22);   z20 = _mm_unpackhi_epi32(y18, y22);
  z21 = _mm_unpacklo_epi32(y19, y23);   z22 = _mm_unpackhi_epi32(y19, y23);
  z23 = _mm_unpacklo_epi32(y20, y24);   z24 = _mm_unpackhi_epi32(y20, y24);
  Y[0] = _mm_unpacklo_epi64(z17, z21); Y[1] = _mm_unpackhi_epi64(z17, z21);
  Y[2] = _mm_unpacklo_epi64(z18, z22); Y[3] = _mm_unpackhi_epi64(z18, z22);
  Y[4] = _mm_unpacklo_epi64(z19, z23); Y[5] = _mm_unpackhi_epi64(z19, z23);
  Y[6] = _mm_unpacklo_epi64(z20, z24); Y[7] = _mm_unpackhi_epi64(z20, z24);
}

#ifdef HAVE_SSSE3
/* ================================================================================== */
/*                               SSSE3 _mm_abs_epi16 */
/* ================================================================================== */
# include <tmmintrin.h>

/* ================================================================================== */
/*                           SSSE3 OPTIMIZED LOOPFILTER CIF */
/* ================================================================================== */
# define NUM_MBS_Y 18
# define NUM_MBS_X 22
# define TAA_H264_DEBLOCKING_FILTER_CHROMA TAA_H264_DEBLOCKING_FILTER_CHROMA_CIF_SSSE3
# define TAA_H264_DEBLOCKING_FILTER_LUMA TAA_H264_DEBLOCKING_FILTER_LUMA_CIF_SSSE3
# include "com_deblock_template.h"
# undef TAA_H264_DEBLOCKING_FILTER_CHROMA
# undef TAA_H264_DEBLOCKING_FILTER_LUMA
# undef NUM_MBS_Y
# undef NUM_MBS_X

/* ================================================================================== */
/*                           SSSE3 OPTIMIZED LOOPFILTER VGA */
/* ================================================================================== */
# define NUM_MBS_Y 30
# define NUM_MBS_X 40
# define TAA_H264_DEBLOCKING_FILTER_CHROMA TAA_H264_DEBLOCKING_FILTER_CHROMA_VGA_SSSE3
# define TAA_H264_DEBLOCKING_FILTER_LUMA TAA_H264_DEBLOCKING_FILTER_LUMA_VGA_SSSE3
# include "com_deblock_template.h"
# undef TAA_H264_DEBLOCKING_FILTER_CHROMA
# undef TAA_H264_DEBLOCKING_FILTER_LUMA
# undef NUM_MBS_Y
# undef NUM_MBS_X

/* ================================================================================== */
/*                           SSSE3 OPTIMIZED LOOPFILTER 720P */
/* ================================================================================== */
# define NUM_MBS_Y 45
# define NUM_MBS_X 80
# define TAA_H264_DEBLOCKING_FILTER_CHROMA TAA_H264_DEBLOCKING_FILTER_CHROMA_720P_SSSE3
# define TAA_H264_DEBLOCKING_FILTER_LUMA TAA_H264_DEBLOCKING_FILTER_LUMA_720P_SSSE3
# include "com_deblock_template.h"
# undef TAA_H264_DEBLOCKING_FILTER_CHROMA
# undef TAA_H264_DEBLOCKING_FILTER_LUMA
# undef NUM_MBS_Y
# undef NUM_MBS_X

/* ================================================================================== */
/*                          SSSE3 OPTIMIZED LOOPFILTER 1080P */
/* ================================================================================== */
# define NUM_MBS_Y 68                   /* 1080 is rounded up to 1088 */
# define NUM_MBS_X 120
# define TAA_H264_DEBLOCKING_FILTER_CHROMA TAA_H264_DEBLOCKING_FILTER_CHROMA_1080P_SSSE3
# define TAA_H264_DEBLOCKING_FILTER_LUMA TAA_H264_DEBLOCKING_FILTER_LUMA_1080P_SSSE3
# include "com_deblock_template.h"
# undef TAA_H264_DEBLOCKING_FILTER_CHROMA
# undef TAA_H264_DEBLOCKING_FILTER_LUMA
# undef NUM_MBS_Y
# undef NUM_MBS_X

# include "com_deblock_template.h"

static void taa_h264_deblocking_filter (
  mb_deblock_t *  deblock_frame,
  uint8_t *       src_y,
  uint8_t *       src_uv,
  const unsigned int      num_mbs_x,
  const unsigned int      num_mbs_y,
  const source_res_t      resolution)
{
  switch (resolution)
  {
  case RES_CIF:
    TAA_H264_DEBLOCKING_FILTER_LUMA_CIF_SSSE3  (deblock_frame, src_y);
    TAA_H264_DEBLOCKING_FILTER_CHROMA_CIF_SSSE3(deblock_frame, src_uv);
    break;
  case RES_VGA:
    TAA_H264_DEBLOCKING_FILTER_LUMA_VGA_SSSE3  (deblock_frame, src_y);
    TAA_H264_DEBLOCKING_FILTER_CHROMA_VGA_SSSE3(deblock_frame, src_uv);
    break;
  case RES_720P:
    TAA_H264_DEBLOCKING_FILTER_LUMA_720P_SSSE3  (deblock_frame, src_y);
    TAA_H264_DEBLOCKING_FILTER_CHROMA_720P_SSSE3(deblock_frame, src_uv);
    break;
  case RES_1080P:
    TAA_H264_DEBLOCKING_FILTER_LUMA_1080P_SSSE3  (deblock_frame, src_y);
    TAA_H264_DEBLOCKING_FILTER_CHROMA_1080P_SSSE3(deblock_frame, src_uv);
    break;
  default:
    TAA_H264_DEBLOCKING_FILTER_LUMA  (deblock_frame, src_y, num_mbs_x, num_mbs_y);
    TAA_H264_DEBLOCKING_FILTER_CHROMA (deblock_frame, src_uv, num_mbs_x, num_mbs_y);
    break;
  }
}
#else
# define _mm_abs_epi16(a) (_mm_max_epi16((a), _mm_sub_epi16(_mm_setzero_si128(), (a))))
/* ================================================================================== */
/*                           SSE2 OPTIMIZED LOOPFILTER CIF */
/* ================================================================================== */
# define NUM_MBS_Y 18
# define NUM_MBS_X 22
# define TAA_H264_DEBLOCKING_FILTER_CHROMA TAA_H264_DEBLOCKING_FILTER_CHROMA_CIF_SSE2
# define TAA_H264_DEBLOCKING_FILTER_LUMA TAA_H264_DEBLOCKING_FILTER_LUMA_CIF_SSE2
# include "com_deblock_template.h"
# undef TAA_H264_DEBLOCKING_FILTER_CHROMA
# undef TAA_H264_DEBLOCKING_FILTER_LUMA
# undef NUM_MBS_Y
# undef NUM_MBS_X

/* ================================================================================== */
/*                           SSE2 OPTIMIZED LOOPFILTER VGA */
/* ================================================================================== */
# define NUM_MBS_Y 30
# define NUM_MBS_X 40
# define TAA_H264_DEBLOCKING_FILTER_CHROMA TAA_H264_DEBLOCKING_FILTER_CHROMA_VGA_SSE2
# define TAA_H264_DEBLOCKING_FILTER_LUMA TAA_H264_DEBLOCKING_FILTER_LUMA_VGA_SSE2
# include "com_deblock_template.h"
# undef TAA_H264_DEBLOCKING_FILTER_CHROMA
# undef TAA_H264_DEBLOCKING_FILTER_LUMA
# undef NUM_MBS_Y
# undef NUM_MBS_X

/* ================================================================================== */
/*                           SSE2 OPTIMIZED LOOPFILTER 720P */
/* ================================================================================== */
# define NUM_MBS_Y 45
# define NUM_MBS_X 80
# define TAA_H264_DEBLOCKING_FILTER_CHROMA TAA_H264_DEBLOCKING_FILTER_CHROMA_720P_SSE2
# define TAA_H264_DEBLOCKING_FILTER_LUMA TAA_H264_DEBLOCKING_FILTER_LUMA_720P_SSE2
# include "com_deblock_template.h"
# undef TAA_H264_DEBLOCKING_FILTER_CHROMA
# undef TAA_H264_DEBLOCKING_FILTER_LUMA
# undef NUM_MBS_Y
# undef NUM_MBS_X

/* ================================================================================== */
/*                          SSE2 OPTIMIZED LOOPFILTER 1080P */
/* ================================================================================== */
# define NUM_MBS_Y 68                   /* 1080 is rounded up to 1088 */
# define NUM_MBS_X 120
# define TAA_H264_DEBLOCKING_FILTER_CHROMA TAA_H264_DEBLOCKING_FILTER_CHROMA_1080P_SSE2
# define TAA_H264_DEBLOCKING_FILTER_LUMA TAA_H264_DEBLOCKING_FILTER_LUMA_1080P_SSE2
# include "com_deblock_template.h"
# undef TAA_H264_DEBLOCKING_FILTER_CHROMA
# undef TAA_H264_DEBLOCKING_FILTER_LUMA
# undef NUM_MBS_Y
# undef NUM_MBS_X

# include "com_deblock_template.h"

static void taa_h264_deblocking_filter (
  mb_deblock_t *  deblock_frame,
  uint8_t *       src_y,
  uint8_t *       src_uv,
  const unsigned int      num_mbs_x,
  const unsigned int      num_mbs_y,
  const source_res_t      resolution)
{
  switch (resolution)
  {
  case RES_CIF:
    TAA_H264_DEBLOCKING_FILTER_LUMA_CIF_SSE2  (deblock_frame, src_y);
    TAA_H264_DEBLOCKING_FILTER_CHROMA_CIF_SSE2(deblock_frame, src_uv);
    break;
  case RES_VGA:
    TAA_H264_DEBLOCKING_FILTER_LUMA_VGA_SSE2  (deblock_frame, src_y);
    TAA_H264_DEBLOCKING_FILTER_CHROMA_VGA_SSE2(deblock_frame, src_uv);
    break;
  case RES_720P:
    TAA_H264_DEBLOCKING_FILTER_LUMA_720P_SSE2  (deblock_frame, src_y);
    TAA_H264_DEBLOCKING_FILTER_CHROMA_720P_SSE2(deblock_frame, src_uv);
    break;
  case RES_1080P:
    TAA_H264_DEBLOCKING_FILTER_LUMA_1080P_SSE2  (deblock_frame, src_y);
    TAA_H264_DEBLOCKING_FILTER_CHROMA_1080P_SSE2(deblock_frame, src_uv);
    break;
  default:
    TAA_H264_DEBLOCKING_FILTER_LUMA  (deblock_frame, src_y, num_mbs_x, num_mbs_y);
    TAA_H264_DEBLOCKING_FILTER_CHROMA (deblock_frame, src_uv, num_mbs_x, num_mbs_y);
    break;
  }
}
# undef _mm_abs_epi16

#endif

#ifdef HAVE_SSSE3
void taa_h264_deblock_frame_ssse3 (
#else
void taa_h264_deblock_frame (
#endif
  framebuf_t *     recon,
  mbinfo_store_t * mbinfo_stored,
  mb_deblock_t   * deblock_info,
  mv_t *           motion_vectors,
  int mbymax,
  int mbxmax,
  source_res_t resolution)
{
  const int mbmax = mbxmax * mbymax;

  for (int i = 0; i < mbmax; i++)
  {
    if (mbinfo_stored[i].skip == 0x3)
    {
      deblock_info[i].do_filter = 0;
    }
    else
    {
      taa_h264_mb_compute_strength (
        &mbinfo_stored[i],
        &motion_vectors[i * NUM_4x4_BLOCKS_Y],
        mbxmax,
        &deblock_info[i]);
    }
  }

#ifdef TAA_H264_TRACE_ENABLED
  if (TAA_H264_TRACE_LEVEL_ENABLED(TAA_H264_TRACE_DEBLOCK))
  {
    for (int i = 0; i < mbmax; i++)
    {
      int vert[16] = {0, };
      int horz[16] = {0, };
      mb_deblock_t * curr = &deblock_info[i];
      for (int j = 0; j < 16; j++)
      {
        if ((curr->do_filter >> (j+16)) & 0x1)
          horz[j] = 1 + ((curr->horz_strength1 >> j) & 0x1) + 2 * ((curr->horz_strength2 >> j) & 0x1);
      }

      for (int j = 0; j < 16; j++)
      {
        if ((curr->do_filter >> j) & 0x1)
          vert[j] = 1 + ((curr->vert_strength1 >> j) & 0x1) + 2 * ((curr->vert_strength2 >> j) & 0x1);
      }

      TAA_H264_TRACE (TAA_H264_TRACE_DEBLOCK, "#%04d horizontal:\t", i);
      for (int j = 0; j < 16; j++)
        TAA_H264_TRACE (TAA_H264_TRACE_DEBLOCK, " %d", horz[j]);
      TAA_H264_TRACE (TAA_H264_TRACE_DEBLOCK, "\n#%04d vertical:  \t", i);
      for (int j = 0; j < 16; j++)
        TAA_H264_TRACE (TAA_H264_TRACE_DEBLOCK, " %d", vert[j]);
      TAA_H264_TRACE (TAA_H264_TRACE_DEBLOCK, "\n");
    }
  }
#endif

  taa_h264_deblocking_filter (deblock_info, recon->y, recon->u, mbxmax, mbymax, resolution);
}
