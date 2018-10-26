
//=================================================================================
//
//                               Copyright Notice
//
//  All material herein: Copyright © 2009 TANDBERG Telecom AS. All Rights Reserved.
//  This product is protected under US patents. Other patents pending.
//  Further information can be found at http://www.tandberg.com/tandberg_pm.jsp.
//
//  The source code is owned by TANDBERG Telecom AS and is protected by copyright
//  laws and international copyright treaties, as well as other intellectual
//  property laws and treaties. All right, title and interest in the source code
//  are owned TANDBERG Telecom AS. Therefore, you must treat the source code like
//  any other copyrighted material.
//
//=================================================================================
#include "enc_load_mb.h"
#include "encoder.h"
#include "com_intmath.h"
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <emmintrin.h>
#include <tmmintrin.h>

#pragma warning (disable:981)

struct resizer_s
{
  TAA_H264_ALIGN(64) int8_t filter[64][8];
  TAA_H264_ALIGN(64) int8_t coeffs[64][64];
  uint64_t cbf[64];
  int fact;
};

#define RESIZE_MB_CUBIC_FUNC resize_mb_cubic_ssse3
#define RESIZE_MB_OCTAVE_FUNC resize_mb_octave_ssse3
#include "enc_resize_template.h"
#undef RESIZE_MB_CUBIC_FUNC
#undef RESIZE_MB_OCTAVE_FUNC

#define RESIZE_MB_CUBIC_FUNC resize_mb_cubic_sse2
#define RESIZE_MB_OCTAVE_FUNC resize_mb_octave_sse2
static uint8_t const TAA_H264_ALIGN(64) M[16] =
{
  0xff, 0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff, 0
};
#define _mm_maddubs_epi16(a, b) _mm_adds_epi16(_mm_mullo_epi16(_mm_srli_epi16((a), 8), _mm_srai_epi16((b), 8)), _mm_mullo_epi16(_mm_and_si128((a), *(__m128i *)M), _mm_srai_epi16(_mm_slli_si128((b), 1), 8)))
#define _mm_hadds_epi16(a, b) _mm_adds_epi16(_mm_packs_epi32(_mm_srai_epi32((a), 16), _mm_srai_epi32((b), 16)), _mm_packs_epi32(_mm_srai_epi32(_mm_slli_si128((a), 2), 16), _mm_srai_epi32(_mm_slli_si128((b), 2), 16)))
#include "enc_resize_template.h"
#undef RESIZE_MB_CUBIC_FUNC
#undef RESIZE_MB_OCTAVE_FUNC
#undef _mm_maddubs_epi16
#undef _mm_hadds_epi16



static void TAA_H264_NOINLINE taa_h264_pad_mb (
  const int N,
  const uint8_t *in_y,
  const uint8_t *in_u,
  const uint8_t *in_v,
  //uint8_t   ld_y[N][N],
  //uint8_t   ld_u[N/2][N/2],
  //uint8_t   ld_v[N/2][N/2],
  uint8_t   *ld_y,
  uint8_t   *ld_u,
  uint8_t   *ld_v,

  const int stride,
  int lpad,
  int rpad,
  int tpad,
  int bpad
  )
{
  const int cstride = stride >> 1;

  __assume_aligned(ld_y, 16);
  __assume_aligned(ld_u, 16);
  __assume_aligned(ld_v, 16);

  // Luma y
  for (int j = tpad; j < N-bpad; j++)
    for (int i = lpad; i < N-rpad; i++)
		ld_y[j*N + i] = in_y[(j - 3)*stride + i - 3];
      //ld_y[j][i] = in_y[(j-3)*stride+i-3];

  // Chroma u
  for (int j = tpad; j <(N-bpad)/2; j++)
    for (int i = lpad; i < (N-rpad)/2; i++)
		ld_u[j*N/2 + i] = in_u[(j - 3)*cstride + i - 3];
      //ld_u[j][i] = in_u[(j-3)*cstride+i-3];

  // Chroma v
  for (int j = tpad; j <(N-bpad)/2; j++)
    for (int i = lpad; i < (N-rpad)/2; i++)
		ld_v[j*N/2 + i] = in_v[(j - 3)*cstride + i - 3];
      //ld_v[j][i] = in_v[(j-3)*cstride+i-3];

  // Pad top
  if (tpad)
  {
    for (int j = 0; j < 3; j++)
      for (int i = 0; i < N; i++)
		  ld_y[j*N + i] = ld_y[3*N/2 + i];
        //ld_y[j][i] = ld_y[3][i];
    for (int j = 0; j < 3; j++)
      for (int i = 0; i < N/2; i++)
		  ld_u[j*N/2 + i] = ld_u[3*N/2 + i];
        //ld_u[j][i] = ld_u[3][i];
    for (int j = 0; j < 3; j++)
      for (int i = 0; i < N/2; i++)
		  ld_v[j*N/2 + i] = ld_v[3*N/2 + i];
        //ld_v[j][i] = ld_v[3][i];
  }
  // Pad left
  if (lpad)
  {
    for (int j = 0; j < N; j++)
      for (int i = 0; i < 3; i++)
		  ld_y[j*N + i] = ld_y[j*N + 3];
        //ld_y[j][i] = ld_y[j][3];
    for (int j = 0; j < N/2; j++)
      for (int i = 0; i < 3; i++)
		  ld_u[j*N/2 + i] = ld_u[j*N/2 + 3];
        //ld_u[j][i] = ld_u[j][3];
    for (int j = 0; j < N/2; j++)
      for (int i = 0; i < 3; i++)
		  ld_v[j*N/2 + i] = ld_v[j*N/2 + 3];
        //ld_v[j][i] = ld_v[j][3];
  }
  // Pad bottom
  if (bpad)
  {
    for (int j = N-bpad; j < N; j++)
      for (int i = 0; i < N; i++)
		  ld_y[j*N + i] = ld_y[(N - bpad - 1)*N + i];
        //ld_y[j][i] = ld_y[N-bpad-1][i];
    for (int j = (N-bpad)/2; j < N/2; j++)
      for (int i = 0; i < N/2; i++)
		  ld_u[j*N/2 + i] = ld_u[((N - bpad) / 2 - 1)*N/2 + i];
        //ld_u[j][i] = ld_u[(N-bpad)/2-1][i];
    for (int j = (N-bpad)/2; j < N/2; j++)
      for (int i = 0; i < N/2; i++)
		  ld_v[j*N/2 + i] = ld_v[((N - bpad) / 2 - 1)*N/2 + i];
        //ld_v[j][i] = ld_v[(N-bpad)/2-1][i];
  }
  // Pad right
  if (rpad)
  {
    for (int j = 0; j < N; j++)
      for (int i = N-rpad; i < N; i++)
		  ld_y[j*N + i] = ld_y[j*N + (N - rpad - 1)];
        //ld_y[j][i] = ld_y[j][N-rpad-1];
    for (int j = 0; j < N/2; j++)
      for (int i = (N-rpad)/2; i < N/2; i++)
		  ld_u[j*N/2 + i] = ld_u[j*N/2 + ((N - rpad) / 2 - 1)];
        //ld_u[j][i] = ld_u[j][(N-rpad)/2-1];
    for (int j = 0; j < N/2; j++)
      for (int i = (N-rpad)/2; i < N/2; i++)
		  ld_v[j*N/2 + i] = ld_v[j*N/2 + ((N - rpad) / 2 - 1)];
        //ld_v[j][i] = ld_v[j][(N-rpad)/2-1];
  }
}

#define SCALE_FIXED_COEFF_SHIFT 6

static int nearest(float a)
{
  int r = (int)(a*1024.0f);
  r = (r+512)>>10;
  return(r);
}

static uint64_t make_coeffs(
  float a,
  float b,
  float c,
  float d)
{
  float f[4];
  int r[4];
  f[0] = a;
  f[1] = b;
  f[2] = c;
  f[3] = d;

  int sum = 0;
  int i;

  for (i = 0; i<4; i++)
  {
    r[i] = nearest((float)(1<<SCALE_FIXED_COEFF_SHIFT)*f[i]);
    sum += r[i];
  }

  int error = sum-(1<<SCALE_FIXED_COEFF_SHIFT);

  while (error!=0)
  {
    int g = (error>0) ? -1 : 1;
    int min_i = 0;
    float min_e = 10000.0f;
    for (i = 0; i<4; i++)
    {
      float e = f[i]*(1<<SCALE_FIXED_COEFF_SHIFT)-r[i]-g;
      if (e<0.0f) e = -e;

      if (e<min_e)
      {
        min_e = e;
        min_i = i;
      }
    }
    r[min_i] += g;
    error += g;
  }

  int8_t c0 = (int8_t)(r[3]);
  int8_t c1 = (int8_t)(r[2]);
  int8_t c2 = (int8_t)(r[1]);
  int8_t c3 = (int8_t)(r[0]);
  uint64_t cc;
  _mm_storel_epi64((__m128i *)&cc, _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, c0, c1, c2, c3, c0, c1, c2, c3));

  return(cc);
}

static float _r(float x)
{
  float p0, p1, p2, p3;
  p0 = x+2.0f;
  p1 = x+1.0f;
  p2 = x+0.0f;
  p3 = x-1.0f;
  p0 = (p0<0.0f) ? 0.0f : (p0*p0*p0);
  p1 = (p1<0.0f) ? 0.0f : (p1*p1*p1);
  p2 = (p2<0.0f) ? 0.0f : (p2*p2*p2);
  p3 = (p3<0.0f) ? 0.0f : (p3*p3*p3);
  return((p0-4.0f*p1+6.0f*p2-4.0f*p3)/6.0f);
}

static void scale_generate_filter_interp_coeffs(uint64_t cbf[64])
{
  memset(cbf, 0, 64*sizeof(uint64_t));
  int i;
  float F, x0, x1, x2, x3, t;

  for (i = 0; i<64; i++)
  {
    F = (0.0f+i)/64.0f;
    x0 = _r(-1.0f-F);
    x1 = _r(0.0f-F);
    x2 = _r(1.0f-F);
    x3 = _r(2.0f-F);
    t = x0+x1+x2+x3;
    x0 /= t;
    x1 /= t;
    x2 /= t;
    x3 /= t;
    cbf[i] = make_coeffs(x0, x1, x2, x3);
  }
}

static void TAA_H264_NOINLINE taa_h264_scale_mb_h (
  const uint8_t * mbsrc_y,
  const uint8_t * mbsrc_u,
  const uint8_t * mbsrc_v,
  uint8_t * mb_y,
  uint8_t * mb_uv,
  const int stride)
{
  const int cstride = stride >> 1;
  TAA_H264_ALIGN(64) uint8_t avg[32*16];

  // Luma y
  for (int i = 0; i < 16; i++)
  {
#pragma vector aligned
    for (int j = 0; j < 32; j++)
      avg[32*i+j] = (uint8_t)((mbsrc_y[(2*i+0)*stride+j] + mbsrc_y[(2*i+1)*stride+j] + 1) >> 1);
  }
#pragma vector aligned
  for (int i = 0; i < 256; i++)
    mb_y[i] = (uint8_t)((avg[2*i] + avg[2*i+1] + 1) >> 1);

  // Chroma u + v
  for (int i = 0; i < 8; i++)
  {
#pragma vector aligned
    for (int j = 0; j < 16; j++)
    {
      avg[32*i+j+ 0] = (uint8_t)((mbsrc_u[(2*i+0)*cstride+j] + mbsrc_u[(2*i+1)*cstride+j] + 1) >> 1);
      avg[32*i+j+16] = (uint8_t)((mbsrc_v[(2*i+0)*cstride+j] + mbsrc_v[(2*i+1)*cstride+j] + 1) >> 1);
    }
  }
#pragma vector aligned
  for (int i = 0; i < 128; i++)
    mb_uv[i] = (uint8_t)((avg[2*i] + avg[2*i+1] + 1) >> 1);
}

static void TAA_H264_NOINLINE taa_h264_scale_mb_q (
  const uint8_t * mbsrc_y,
  const uint8_t * mbsrc_u,
  const uint8_t * mbsrc_v,
  uint8_t * mb_y,
  uint8_t * mb_uv,
  const int stride)
{
  const int cstride = stride >> 1;
  TAA_H264_ALIGN(64) uint8_t avg[64*32];
  TAA_H264_ALIGN(64) uint8_t y[32*32];
  TAA_H264_ALIGN(64) uint8_t u[16*16];
  TAA_H264_ALIGN(64) uint8_t v[16*16];

  // Luma y
  for (int i = 0; i < 32; i++)
  {
#pragma vector aligned
    for (int j = 0; j < 64; j++)
      avg[64*i+j] = (uint8_t)((mbsrc_y[2*i*stride+j] + mbsrc_y[(2*i+1)*stride+j] + 1) >> 1);
  }
#pragma vector aligned
  for (int i = 0; i < 32*32; i++)
    y[i] = (uint8_t)((avg[2*i] + avg[2*i+1] + 1) >> 1);

  for (int i = 0; i < 16; i++)
  {
#pragma vector aligned
    for (int j = 0; j < 32; j++)
      avg[32*i+j] = (uint8_t)((y[2*i*32+j] + y[(2*i+1)*32+j] + 1) >> 1);
  }
#pragma vector aligned
  for (int i = 0; i < 16*16; i++)
    mb_y[i] = (uint8_t)((avg[2*i] + avg[2*i+1] + 1) >> 1);

  // Chroma u
  for (int i = 0; i < 16; i++)
  {
#pragma vector aligned
    for (int j = 0; j < 32; j++)
      avg[32*i+j] = (uint8_t)((mbsrc_u[2*i*cstride+j] + mbsrc_u[(2*i+1)*cstride+j] + 1) >> 1);
  }
#pragma vector aligned
  for (int i = 0; i < 16*16; i++)
    u[i] = (uint8_t)((avg[2*i] + avg[2*i+1] + 1) >> 1);

  // Chroma v
  for (int i = 0; i < 16; i++)
  {
#pragma vector aligned
    for (int j = 0; j < 32; j++)
      avg[32*i+j] = (uint8_t)((mbsrc_v[2*i*cstride+j] + mbsrc_v[(2*i+1)*cstride+j] + 1) >> 1);
  }
#pragma vector aligned
  for (int i = 0; i < 16*16; i++)
    v[i] = (uint8_t)((avg[2*i] + avg[2*i+1] + 1) >> 1);

  // Chroma u + v
  for (int i = 0; i < 8; i++)
  {
#pragma vector aligned
    for (int j = 0; j < 16; j++)
    {
      avg[32*i+j+ 0] = (uint8_t)((u[(2*i+0)*16+j] + u[(2*i+1)*16+j] + 1) >> 1);
      avg[32*i+j+16] = (uint8_t)((v[(2*i+0)*16+j] + v[(2*i+1)*16+j] + 1) >> 1);
    }
  }
#pragma vector aligned
  for (int i = 0; i < 128; i++)
    mb_uv[i] = (uint8_t)((avg[2*i] + avg[2*i+1] + 1) >> 1);
}

static void TAA_H264_NOINLINE taa_h264_scale_pad_mb_q (
  const uint8_t * mbsrc_y,
  const uint8_t * mbsrc_u,
  const uint8_t * mbsrc_v,
  uint8_t * mb_y,
  uint8_t * mb_uv,
  int width,
  int height,
  int xpos,
  int ypos,
  bool pad_x,
  bool pad_y)
{
  TAA_H264_ALIGN(64) uint8_t buf_y[64*64];
  TAA_H264_ALIGN(64) uint8_t buf_u[32*32];
  TAA_H264_ALIGN(64) uint8_t buf_v[32*32];

  memset(buf_y, 127, 64*64);
  memset(buf_u, 127, 32*32);
  memset(buf_v, 127, 32*32);

  const int read_x = pad_x ? width - xpos : 64;
  const int read_y = pad_y ? height - ypos : 64;
  const int cread_x = read_x >> 1;
  const int cread_y = read_y >> 1;
  const int cwidth = width >> 1;

  for (int i = 0; i < read_y; i++)
  {
#pragma vector aligned
    for (int j = 0; j < read_x; j++)
      buf_y[64*i+j] = mbsrc_y[i*width+j];
  }

  for (int i = 0; i < cread_y; i++)
  {
#pragma vector aligned
    for (int j = 0; j < cread_x; j++)
      buf_u[32*i+j] = mbsrc_u[i*cwidth+j];
  }

  for (int i = 0; i < cread_y; i++)
  {
#pragma vector aligned
    for (int j = 0; j < cread_x; j++)
      buf_v[32*i+j] = mbsrc_v[i*cwidth+j];
  }

  taa_h264_scale_mb_q (buf_y, buf_u, buf_v, mb_y, mb_uv, 64);
}




static void TAA_H264_NOINLINE taa_h264_scale_pad_mb_h (
  const uint8_t * mbsrc_y,
  const uint8_t * mbsrc_u,
  const uint8_t * mbsrc_v,
  uint8_t * mb_y,
  uint8_t * mb_uv,
  int width,
  int height,
  int xpos,
  int ypos,
  bool pad_x,
  bool pad_y)
{
  TAA_H264_ALIGN(64) uint8_t buf_y[32*32];
  TAA_H264_ALIGN(64) uint8_t buf_u[16*16];
  TAA_H264_ALIGN(64) uint8_t buf_v[16*16];

  memset(buf_y, 127, 32*32);
  memset(buf_u, 127, 16*16);
  memset(buf_v, 127, 16*16);

  const int read_x = pad_x ? width - xpos : 32;
  const int read_y = pad_y ? height - ypos : 32;
  const int cread_x = read_x >> 1;
  const int cread_y = read_y >> 1;
  const int cwidth = width >> 1;

  for (int i = 0; i < read_y; i++)
  {
#pragma vector aligned
    for (int j = 0; j < read_x; j++)
      buf_y[32*i+j] = mbsrc_y[i*width+j];
  }

  for (int i = 0; i < cread_y; i++)
  {
#pragma vector aligned
    for (int j = 0; j < cread_x; j++)
      buf_u[16*i+j] = mbsrc_u[i*cwidth+j];
  }

  for (int i = 0; i < cread_y; i++)
  {
#pragma vector aligned
    for (int j = 0; j < cread_x; j++)
      buf_v[16*i+j] = mbsrc_v[i*cwidth+j];
  }

  taa_h264_scale_mb_h (buf_y, buf_u, buf_v, mb_y, mb_uv, 32);
}

static void TAA_H264_NOINLINE taa_h264_load_mb (
  const uint8_t * mbsrc_y,
  const uint8_t * mbsrc_u,
  const uint8_t * mbsrc_v,
  uint8_t * mb_y,
  uint8_t * mb_uv,
  const int stride)
{
  const int cstride = stride >> 1;
  // Load Luma Y block
  for(int i = 0; i < 16; i++)
  {
    _mm_prefetch((const char*)(&mb_y[(i+8)*stride]), _MM_HINT_T0);
    _mm_store_pd((double *)(mb_y +  i*16), _mm_load_pd((double *)(mbsrc_y +  i*stride)));    
  }

  // Load Chroma U and V block
  for(int i = 0; i < 8; i++)
  {
    _mm_store_pd((double *)(mb_uv + i*16),
		 _mm_set_pd (*((double *)(mbsrc_v + cstride*i)),
			     *((double *)(mbsrc_u + cstride*i))));
  }
}

static void TAA_H264_NOINLINE taa_h264_load_mb_with_padding (
  const uint8_t * mbsrc_y,
  const uint8_t * mbsrc_u,
  const uint8_t * mbsrc_v,
  uint8_t * mb_y,
  uint8_t * mb_uv,
  const int width,
  const int read_y)
{
  TAA_H264_ALIGN(64) uint8_t buf_y[16*16];
  TAA_H264_ALIGN(64) uint8_t buf_u[8*8];
  TAA_H264_ALIGN(64) uint8_t buf_v[8*8];

  memset(buf_y, 127, sizeof buf_y);
  memset(buf_u, 127, sizeof buf_u);
  memset(buf_v, 127, sizeof buf_v);

  const int read_x = 16;
  const int cread_x = read_x / 2;
  const int cread_y = read_y / 2;
  const int cwidth = width / 2;

  for (int i = 0; i < read_y; i++)
  {
#pragma vector aligned
    for (int j = 0; j < read_x; j++)
      buf_y[16*i+j] = mbsrc_y[i*width+j];
  }

  for (int i = 0; i < cread_y; i++)
  {
#pragma vector aligned
    for (int j = 0; j < cread_x; j++)
      buf_u[8*i+j] = mbsrc_u[i*cwidth+j];
  }

  for (int i = 0; i < cread_y; i++)
  {
#pragma vector aligned
    for (int j = 0; j < cread_x; j++)
      buf_v[8*i+j] = mbsrc_v[i*cwidth+j];
  }

  taa_h264_load_mb(buf_y, buf_u, buf_v, mb_y, mb_uv, 16);
}

#define RSZ_1 (1 * RSA)
#define RSZ_2 (2 * RSA)
#define RSZ_4 (4 * RSA)

void taa_h264_load_resized_mb(
  resizer_t *        resizer,
  cpuinfo_t *        cpuinfo,
  const uint8_t *    in_y,
  const uint8_t *    in_u,
  const uint8_t *    in_v,
  uint8_t *  mb_y,
  uint8_t *  mb_uv,
  int                width_in,
  int                height_in,
  int                xpos_in,
  int                ypos_in,
  unsigned int       fineH,
  unsigned int       fineV)
{
  const int RSZ = (resizer->fact);
  // size: number of pixels in original picture that corresponds to 16px in new
  const int SIZE = (RSZ >> (SHFT - 4));
  // read: how many pixels do we need to read for resizing 16px
  // assuming 8-tap filter and rounding up to nearest modulo 32 for efficiency
  const int READ = (((SIZE+31+7)>>5)<<5); // should it be plus 7 (3 before block, 4 after)?
  // The padding is calculated with the respect to read pixels from the input
  int lpad = ((xpos_in - 3) < 0) ? 3 : 0;
  int rpad = ((xpos_in + READ) > width_in) ? (xpos_in + READ) - width_in : 0;
  int tpad = ((ypos_in - 3) < 0) ? 3 : 0;
  int bpad = ((ypos_in + READ) > height_in) ? (ypos_in + READ) - height_in : 0;
  bool pad = lpad | rpad | tpad | bpad;

  if (!pad)
  {
    switch(RSZ)
    {
    case RSZ_1: // ratio 1:1
      taa_h264_load_mb(in_y, in_u, in_v, mb_y, mb_uv, width_in);
      break;
    case RSZ_2: // ratio 1:2
      taa_h264_scale_mb_h(in_y, in_u, in_v, mb_y, mb_uv, width_in);
      break;
    case RSZ_4: // ratio 1:4
      taa_h264_scale_mb_q(in_y, in_u, in_v, mb_y, mb_uv, width_in);
      break;
    default: // any ratio
      switch (READ)
      {
      case 32:
        if (cpuinfo->have_ssse3)
          resize_mb_cubic_ssse3(in_y, in_u, in_v, mb_y, mb_uv, resizer->cbf, width_in, RSZ>>3);
        else
          resize_mb_cubic_sse2(in_y, in_u, in_v, mb_y, mb_uv, resizer->cbf, width_in, RSZ>>3);
        break;
      default:
        if (cpuinfo->have_ssse3)
          resize_mb_octave_ssse3 (READ, resizer->filter, resizer->coeffs, in_y, in_u, in_v, mb_y, mb_uv, width_in, RSZ, fineH, fineV);
        else
          resize_mb_octave_sse2 (READ, resizer->filter, resizer->coeffs, in_y, in_u, in_v, mb_y, mb_uv, width_in, RSZ, fineH, fineV);
        break;
      }
      break;
    }
  }
  else
  {
    switch(RSZ)
    {
    case RSZ_1: // ratio 1:1
      /* Since input width is modulo 16 and this is a 1:1 mapping there is no
       * horizontal padding. bpad has to be recalculated since the previous value
       * was based on a 8-tap filter.*/
      bpad = ((ypos_in + 16) > height_in) ? (ypos_in + 16) - height_in : 0;
      if (bpad)
        taa_h264_load_mb_with_padding (in_y, in_u, in_v, mb_y, mb_uv, width_in, 16 - bpad);
      else
        taa_h264_load_mb(in_y, in_u, in_v, mb_y, mb_uv, width_in);
      break;
    case RSZ_2: // ratio 1:2
      // The previous rpad/bpad was assuming 8-tap filter. The 2x downscaling only
      // uses 2 pixel average, so we might not need padding after all.
      rpad = ((xpos_in + 32) > width_in) ? (xpos_in + 32) - width_in : 0;
      bpad = ((ypos_in + 32) > height_in) ? (ypos_in + 32) - height_in : 0;
      if (rpad | bpad)
        taa_h264_scale_pad_mb_h(in_y, in_u, in_v, mb_y, mb_uv, width_in, height_in, xpos_in, ypos_in, rpad, bpad);
      else
        taa_h264_scale_mb_h(in_y, in_u, in_v, mb_y, mb_uv, width_in);
      break;
    case RSZ_4: // ratio 1:4
      // The previous rpad/bpad was assuming 8-tap filter. The 4x downscaling
      // uses 4 pixel average, so we might not need padding after all.
      rpad = ((xpos_in + 64) > width_in) ? (xpos_in + 64) - width_in : 0;
      bpad = ((ypos_in + 64) > height_in) ? (ypos_in + 64) - height_in : 0;
      if (rpad | bpad)
        taa_h264_scale_pad_mb_q(in_y, in_u, in_v, mb_y, mb_uv, width_in, height_in, xpos_in, ypos_in, rpad, bpad);
      else
        taa_h264_scale_mb_q(in_y, in_u, in_v, mb_y, mb_uv, width_in);
      break;
    default: // any ratio
    {
	  TAA_H264_ALIGN(64) uint8_t *ld_y = (uint8_t *)malloc(READ*READ);               // [READ][READ];
	  TAA_H264_ALIGN(64) uint8_t *ld_u = (uint8_t *)malloc((READ/2)*(READ/2));       //[READ / 2][READ / 2];
	  TAA_H264_ALIGN(64) uint8_t *ld_v = (uint8_t *)malloc((READ / 2)*(READ / 2));   //[READ / 2][READ / 2];
      switch (READ)
      {
      case 32:
        taa_h264_pad_mb (READ, in_y, in_u, in_v, ld_y, ld_u, ld_v, width_in, lpad, rpad, tpad, bpad);
        if (cpuinfo->have_ssse3)
          resize_mb_cubic_ssse3(&ld_y[3 * READ + 3], &ld_u[3 * (READ/2) + 3], &ld_v[3 * (READ / 2) + 3], mb_y, mb_uv, resizer->cbf, READ, RSZ>>3);
        else
          resize_mb_cubic_sse2(&ld_y[3 * READ + 3], &ld_u[3 * (READ / 2) + 3], &ld_v[3 * (READ / 2) + 3], mb_y, mb_uv, resizer->cbf, READ, RSZ>>3);
	  
	    free(ld_y);
		free(ld_u);
		free(ld_v);
        break;
      default:
        // FIXME: There seems to be a bug when READ is 128 (e.g. 1080p -> 180p)
        taa_h264_pad_mb (READ, in_y, in_u, in_v, ld_y, ld_u, ld_v, width_in, lpad, rpad, tpad, bpad);
        if (cpuinfo->have_ssse3)
          resize_mb_octave_ssse3 (READ, resizer->filter, resizer->coeffs, &ld_y[3 * READ + 3], &ld_u[3 * (READ / 2) + 3], &ld_v[3 * (READ / 2) + 3], mb_y, mb_uv, READ, RSZ, fineH, fineV);
        else
          resize_mb_octave_sse2 (READ, resizer->filter, resizer->coeffs, &ld_y[3 * READ + 3], &ld_u[3 * (READ / 2) + 3], &ld_v[3 * (READ / 2) + 3], mb_y, mb_uv, READ, RSZ, fineH, fineV);
	  
	    free(ld_y);
		free(ld_u);
		free(ld_v);
        break;
      }


      break;
    }
    }
  }
}

#include <stdlib.h>
#define _lmbd(a, b) (((b) == (0)) ? (32) : (31 - _bit_scan_reverse(b)))
static inline int  ABS (int a)
{
  return abs(a);
}
static inline uint32_t NORM(int a)
{
  int signbit = (a >> 31); return(_lmbd(1, abs(a + signbit))-1);
}
static inline uint32_t ISPOW2(uint32_t a)
{
  return ( (a > 0) && ((a & (a - 1)) == 0) );
}
static inline int  MPYQ24(int a,
                          int b)
{
  int64_t r64 = ((int64_t)(a)*(int64_t)(b) + 0x800000) >> 24;  return((int)(r64));
}

/*****************************************************************************
 * Author        : MAH
 * Description   : Return normalized filter tap value at position x, both in Q24.
 ****************************************************************************/
static inline int tap_value_normal(unsigned x)
{
  uint32_t
  C_0_5 = 0x00800000, C_1_0 = 0x01000000,
  C_2_0 = 0x02000000, C_2_5 = 0x02800000,
  x2, x3, f, g, h, k = 0x01965FEA*0;

  x2 = MPYQ24(x, x);
  x3 = MPYQ24(x2, x = ABS(x));

  f = (x3 + (x3 >> 1)) - MPYQ24(C_2_5, x2 ) + C_1_0;

  g = C_2_0 + MPYQ24(-C_0_5, x3) +
      MPYQ24( C_2_5, x2) - (x << 2);

  h = ((f & (x < C_1_0 ? ~0 : 0)) + (g & (x > C_1_0 ? ~0 : 0)))
      & (x < C_2_0 ? ~0 : 0);

  return MPYQ24(h, C_1_0 + MPYQ24(k, x2));
}

/*****************************************************************************
 * Author        :  MAH
 * Description   : Auto-generate resize coefficients for arbitrary resize
 *                 factors and filters of arbitrary lengths.
 ****************************************************************************/
int generate_coeffs(
  int8_t ret[64][8],                     /* output buffer of nt * np signed chars     */
  int    nt,                             /* number of taps, must be even              */
  int    np,                             /* number of phases, must be a power of two  */
  int    rsz,                            /* resize factor, typically RSA*srclen/dstlen */
  int    csum                            /* output coefficient sum, a power of two    */
  )
{
  int8_t res[8 * 64];
  int fval[MAX_NPxNT], i, ph, sum, isum, tsum, R, r, sf, seed, ntt;
  int lcs = _lmbd(0, csum);
  int downscaling = (rsz < RSA) ? 0 : 1;

  if(!downscaling) rsz = RSA;  /* get the old upscale coeffs */
  if((np * nt) > MAX_NPxNT) return -1;  /* np * nt too large */
  if(!ISPOW2(np)) return -1;           /* # phases not 2^n  */
  if(nt & 1) return -1;                /* # taps not even   */

  /* == If nt not a power of two, roundup to nearest power of two == */
  ntt = nt;
  if(!ISPOW2(nt))
  {
    nt = (nt << NORM(nt) & 0x40000000) >> (NORM(nt) - 1);
  }

  seed = ((1 << 30) / ((rsz >> 0) * np >> 14)) << 5;
  sf = MPYQ24(seed, (1 << (24 + 1)) - (MPYQ24((rsz >> 5) * np, seed) << 4)) >> 7;

  for(ph = 0; ph < np >> 1; ph++)
    for(i = 0; i<nt; i++)
      fval[ph * nt + i] = tap_value_normal((np * (2 * (1 + i) - nt) - 2 * ph - 1) * sf);

  /* == If nt not a power of two, shift values and zero out extra taps == */
  if(nt > ntt)
  {
    for(ph = 0; ph < np >> 1; ph++)
      for(i = 0; i<ntt; i++)
        fval[ph * nt + i] = fval[ph * nt + i + ((nt - ntt) >> 1)];

    for(ph = 0; ph < np >> 1; ph++)
      for(i = ntt; i<nt; i++)
        fval[ph * nt + i] = 0;
  }

  for(ph = 0; ph < np >> 1; ph++)
  {
    int nri = 3, ns;

    for(i = 0, sum = 0; i < nt; i++)
      sum += fval[ph * nt + i];

    /* === first guess for 1/sum is 1/rsz ==== */
    ns = NORM(sum);

    // Should not happen, but avoid a potential divide by zero
    if (sum == 0)
      sum = 1;

    isum = 0x7FFFFFFF / (sum << ns >> 16) << (1 + ns);

    /* === improve guess for 1/sum using Newton-Raphson ==== */
    while(nri--)
      isum = MPYQ24(isum, (1 << (24 + 1)) - MPYQ24(sum, isum));

    /* === scale to make sum[fval] (roughly) equal to unity ==== */
    for(i = 0; i < nt; i++)
      fval[ph * nt + i] = MPYQ24(isum, fval[ph * nt + i]);


    /* ==== try to improve rounding, but don't spend too much time... ==== */
    R = r = 1 << (lcs - 8);

    do
    {
      for(i = 0, tsum = 0; i < nt; i++)
      {
        int t = (fval[ph * nt + i] + R) >> (lcs + 1 - 8);

        tsum += res[ph * nt + i] = (int8_t) (t > 127 ? 127 : t);
      }

      R -= r = r >> 1;

    } while (tsum > csum && r);


    /* ==== ... remaining bits here will be taken care of here  ==== */
    for(r = 0; r < csum - tsum; r++)
    {
      int idx = 0, m = 0, d, t;

      for(i = 0; i < nt; i++)
      {
        d = fval[ph*nt + i] -
            ((t = res[ph*nt + i]) << (lcs + 1 - 8));

        if(d > m && t < 127) { m = d; idx = i; }
      }
      ++res[ph * nt + idx];
    }
  }
  /* ==== symmetrical phases ==== */
  for(i = 0; i < nt * np >> 1; i++)
    res[(nt * np >> 1) + i] = res[(nt * np >> 1) -1 - i];

  /* == If nt not a power of two, shift values and zero out extra taps == */
  if(nt > ntt)
  {
    for(ph = np >> 1; ph < np; ph++)
      for(i = 0; i<ntt; i++)
        res[ph * nt + i] = res[ph * nt + i + (nt - ntt)];

    for(ph = np >> 1; ph < np; ph++)
      for(i = ntt; i<nt; i++)
        res[ph * nt + i] = 0;
  }
  memcpy(ret, res, 8*64);
  return 0;
}

resizer_t * taa_h264_resizer_create (void)
{
  resizer_t * r = TAA_H264_MALLOC (sizeof (resizer_t));
  if (!r)
    return NULL;
  r->fact = -1;
  return r;
}

void taa_h264_resizer_delete (resizer_t * r)
{
  TAA_H264_FREE (r);
}

void taa_h264_resizer_init (resizer_t * r,
                            int         fact)
{
  if (fact == r->fact)
    return;

  const int vtaps = 8;
  r->fact = fact;
  // 8-tap coefficients
  generate_coeffs(r->filter, vtaps, NTxNP / vtaps, fact, 128);
  // cubic coefficients
  scale_generate_filter_interp_coeffs(r->cbf);

  // Prepare data for smart instruction
  for(int i = 0; i < 64; i++)
  {
    for(int j = 0; j < 8; j++)
    {
      r->coeffs[i][2*j+0+ 0] = r->filter[i][0];
      r->coeffs[i][2*j+1+ 0] = r->filter[i][1];
      r->coeffs[i][2*j+0+16] = r->filter[i][2];
      r->coeffs[i][2*j+1+16] = r->filter[i][3];
      r->coeffs[i][2*j+0+32] = r->filter[i][4];
      r->coeffs[i][2*j+1+32] = r->filter[i][5];
      r->coeffs[i][2*j+0+48] = r->filter[i][6];
      r->coeffs[i][2*j+1+48] = r->filter[i][7];
    }
  }
}
