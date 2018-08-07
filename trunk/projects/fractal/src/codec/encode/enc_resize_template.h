#if 1
// TODO: Low hanging optimizations opportunities present in this function.
static void TAA_H264_NOINLINE RESIZE_MB_OCTAVE_FUNC(
  const int READ,
  int8_t filter[64][8],
  int8_t coeffs[64][64],
  const uint8_t *in_y,
  const uint8_t *in_u,
  const uint8_t *in_v,
  uint8_t * mb_y,
  uint8_t * mb_uv,
  const int stride,
  const int RSZ,
  const unsigned int fineH,
  const unsigned int fineV)
{
  __m128i *xmm = (__m128i *)malloc(sizeof(__m128i)*READ);
  const int cstride = stride >> 1;

  __assume_aligned(mb_y, 16);
  __assume_aligned(mb_uv, 16);

  const unsigned int xx0 =  fineH + 2*RSZ*0;
  const unsigned int xx1 =  fineH + 2*RSZ*1;
  const unsigned int xx2 =  fineH + 2*RSZ*2;
  const unsigned int xx3 =  fineH + 2*RSZ*3;
  const unsigned int xx4 =  fineH + 2*RSZ*4;
  const unsigned int xx5 =  fineH + 2*RSZ*5;
  const unsigned int xx6 =  fineH + 2*RSZ*6;
  const unsigned int xx7 =  fineH + 2*RSZ*7;
  const unsigned int xx8 =  fineH + 2*RSZ*8;
  const unsigned int xx9 =  fineH + 2*RSZ*9;
  const unsigned int xxA =  fineH + 2*RSZ*10;
  const unsigned int xxB =  fineH + 2*RSZ*11;
  const unsigned int xxC =  fineH + 2*RSZ*12;
  const unsigned int xxD =  fineH + 2*RSZ*13;
  const unsigned int xxE =  fineH + 2*RSZ*14;
  const unsigned int xxF =  fineH + 2*RSZ*15;

  const int ix0 = (xx0 >> 20) - (fineH >> 20);
  const int ix1 = (xx1 >> 20) - (fineH >> 20);
  const int ix2 = (xx2 >> 20) - (fineH >> 20);
  const int ix3 = (xx3 >> 20) - (fineH >> 20);
  const int ix4 = (xx4 >> 20) - (fineH >> 20);
  const int ix5 = (xx5 >> 20) - (fineH >> 20);
  const int ix6 = (xx6 >> 20) - (fineH >> 20);
  const int ix7 = (xx7 >> 20) - (fineH >> 20);
  const int ix8 = (xx8 >> 20) - (fineH >> 20);
  const int ix9 = (xx9 >> 20) - (fineH >> 20);
  const int ixA = (xxA >> 20) - (fineH >> 20);
  const int ixB = (xxB >> 20) - (fineH >> 20);
  const int ixC = (xxC >> 20) - (fineH >> 20);
  const int ixD = (xxD >> 20) - (fineH >> 20);
  const int ixE = (xxE >> 20) - (fineH >> 20);
  const int ixF = (xxF >> 20) - (fineH >> 20);

  __m128i c01 = _mm_castpd_si128(_mm_set_pd(*(double *)filter[(xx1>>14)&63], *(double *)filter[(xx0>>14)&63]));
  __m128i c23 = _mm_castpd_si128(_mm_set_pd(*(double *)filter[(xx3>>14)&63], *(double *)filter[(xx2>>14)&63]));
  __m128i c45 = _mm_castpd_si128(_mm_set_pd(*(double *)filter[(xx5>>14)&63], *(double *)filter[(xx4>>14)&63]));
  __m128i c67 = _mm_castpd_si128(_mm_set_pd(*(double *)filter[(xx7>>14)&63], *(double *)filter[(xx6>>14)&63]));
  __m128i c89 = _mm_castpd_si128(_mm_set_pd(*(double *)filter[(xx9>>14)&63], *(double *)filter[(xx8>>14)&63]));
  __m128i cAB = _mm_castpd_si128(_mm_set_pd(*(double *)filter[(xxB>>14)&63], *(double *)filter[(xxA>>14)&63]));
  __m128i cCD = _mm_castpd_si128(_mm_set_pd(*(double *)filter[(xxD>>14)&63], *(double *)filter[(xxC>>14)&63]));
  __m128i cEF = _mm_castpd_si128(_mm_set_pd(*(double *)filter[(xxF>>14)&63], *(double *)filter[(xxE>>14)&63]));

  // Scale Y
  int ys = 0;
  for (int y = 0; y<16; y++)
  {
    const unsigned int yy =  fineV + 2*RSZ*y;
    const int iy = (yy >> 20) - (fineV >> 20);
    for(int n = 0; n<iy-ys+8; n++)
    {
      __m128i xmm0 = _mm_maddubs_epi16(_mm_castpd_si128(_mm_set_pd(*(double *)&in_y[(n+ys-3)*stride+ix1-3], *(double *)&in_y[(n+ys-3)*stride+ix0-3])), c01);
      __m128i xmm1 = _mm_maddubs_epi16(_mm_castpd_si128(_mm_set_pd(*(double *)&in_y[(n+ys-3)*stride+ix3-3], *(double *)&in_y[(n+ys-3)*stride+ix2-3])), c23); xmm0 = _mm_hadds_epi16 (xmm0, xmm1);
      __m128i xmm2 = _mm_maddubs_epi16(_mm_castpd_si128(_mm_set_pd(*(double *)&in_y[(n+ys-3)*stride+ix5-3], *(double *)&in_y[(n+ys-3)*stride+ix4-3])), c45);
      __m128i xmm3 = _mm_maddubs_epi16(_mm_castpd_si128(_mm_set_pd(*(double *)&in_y[(n+ys-3)*stride+ix7-3], *(double *)&in_y[(n+ys-3)*stride+ix6-3])), c67); xmm2 = _mm_hadds_epi16 (xmm2, xmm3);
      __m128i xmm4 = _mm_maddubs_epi16(_mm_castpd_si128(_mm_set_pd(*(double *)&in_y[(n+ys-3)*stride+ix9-3], *(double *)&in_y[(n+ys-3)*stride+ix8-3])), c89);
      __m128i xmm5 = _mm_maddubs_epi16(_mm_castpd_si128(_mm_set_pd(*(double *)&in_y[(n+ys-3)*stride+ixB-3], *(double *)&in_y[(n+ys-3)*stride+ixA-3])), cAB); xmm4 = _mm_hadds_epi16 (xmm4, xmm5);
      __m128i xmm6 = _mm_maddubs_epi16(_mm_castpd_si128(_mm_set_pd(*(double *)&in_y[(n+ys-3)*stride+ixD-3], *(double *)&in_y[(n+ys-3)*stride+ixC-3])), cCD);
      __m128i xmm7 = _mm_maddubs_epi16(_mm_castpd_si128(_mm_set_pd(*(double *)&in_y[(n+ys-3)*stride+ixF-3], *(double *)&in_y[(n+ys-3)*stride+ixE-3])), cEF); xmm6 = _mm_hadds_epi16 (xmm6, xmm7);
      xmm[n+ys] = _mm_packus_epi16(_mm_srai_epi16(_mm_adds_epi16(_mm_hadds_epi16 (xmm0, xmm2), _mm_set1_epi16(64)), 7), _mm_srai_epi16(_mm_adds_epi16(_mm_hadds_epi16 (xmm4, xmm6), _mm_set1_epi16(64)), 7));
    }

    __m128i xmm0 = _mm_maddubs_epi16(_mm_unpacklo_epi8(xmm[iy+0], xmm[iy+1]), *(__m128i*)&coeffs[(yy>>14)&63][ 0]);
    __m128i xmm1 = _mm_maddubs_epi16(_mm_unpacklo_epi8(xmm[iy+2], xmm[iy+3]), *(__m128i*)&coeffs[(yy>>14)&63][16]); xmm0 = _mm_adds_epi16(xmm0, xmm1);
    __m128i xmm2 = _mm_maddubs_epi16(_mm_unpacklo_epi8(xmm[iy+4], xmm[iy+5]), *(__m128i*)&coeffs[(yy>>14)&63][32]);
    __m128i xmm3 = _mm_maddubs_epi16(_mm_unpacklo_epi8(xmm[iy+6], xmm[iy+7]), *(__m128i*)&coeffs[(yy>>14)&63][48]); xmm2 = _mm_adds_epi16(xmm2, xmm3);
    __m128i xmm4 = _mm_maddubs_epi16(_mm_unpackhi_epi8(xmm[iy+0], xmm[iy+1]), *(__m128i*)&coeffs[(yy>>14)&63][ 0]);
    __m128i xmm5 = _mm_maddubs_epi16(_mm_unpackhi_epi8(xmm[iy+2], xmm[iy+3]), *(__m128i*)&coeffs[(yy>>14)&63][16]); xmm4 = _mm_adds_epi16(xmm4, xmm5);
    __m128i xmm6 = _mm_maddubs_epi16(_mm_unpackhi_epi8(xmm[iy+4], xmm[iy+5]), *(__m128i*)&coeffs[(yy>>14)&63][32]);
    __m128i xmm7 = _mm_maddubs_epi16(_mm_unpackhi_epi8(xmm[iy+6], xmm[iy+7]), *(__m128i*)&coeffs[(yy>>14)&63][48]); xmm6 = _mm_adds_epi16(xmm6, xmm7);
    _mm_store_si128((__m128i *)&mb_y[16*y], _mm_packus_epi16(_mm_srai_epi16(_mm_adds_epi16(_mm_adds_epi16(xmm0, xmm2), _mm_set1_epi16(64)), 7), _mm_srai_epi16(_mm_adds_epi16(_mm_adds_epi16(xmm4, xmm6), _mm_set1_epi16(64)), 7)));
    ys = iy + 8;
  }
  // Scale UV
  ys = 0;
  for (int y = 0; y<8; y++)
  {
    const unsigned int yy =  fineV + 2*RSZ*y;
    const int iy = (yy >> 20) - (fineV >> 20);
    for(int n = 0; n<iy-ys+8; n++)
    {
      __m128i xmm0 = _mm_maddubs_epi16(_mm_castpd_si128(_mm_set_pd(*(double *)&in_u[(n+ys-3)*cstride+ix1-3], *(double *)&in_u[(n+ys-3)*cstride+ix0-3])), c01);
      __m128i xmm1 = _mm_maddubs_epi16(_mm_castpd_si128(_mm_set_pd(*(double *)&in_u[(n+ys-3)*cstride+ix3-3], *(double *)&in_u[(n+ys-3)*cstride+ix2-3])), c23); xmm0 = _mm_hadds_epi16 (xmm0, xmm1);
      __m128i xmm2 = _mm_maddubs_epi16(_mm_castpd_si128(_mm_set_pd(*(double *)&in_u[(n+ys-3)*cstride+ix5-3], *(double *)&in_u[(n+ys-3)*cstride+ix4-3])), c45);
      __m128i xmm3 = _mm_maddubs_epi16(_mm_castpd_si128(_mm_set_pd(*(double *)&in_u[(n+ys-3)*cstride+ix7-3], *(double *)&in_u[(n+ys-3)*cstride+ix6-3])), c67); xmm2 = _mm_hadds_epi16 (xmm2, xmm3);
      __m128i xmm4 = _mm_maddubs_epi16(_mm_castpd_si128(_mm_set_pd(*(double *)&in_v[(n+ys-3)*cstride+ix1-3], *(double *)&in_v[(n+ys-3)*cstride+ix0-3])), c01);
      __m128i xmm5 = _mm_maddubs_epi16(_mm_castpd_si128(_mm_set_pd(*(double *)&in_v[(n+ys-3)*cstride+ix3-3], *(double *)&in_v[(n+ys-3)*cstride+ix2-3])), c23); xmm4 = _mm_hadds_epi16 (xmm4, xmm5);
      __m128i xmm6 = _mm_maddubs_epi16(_mm_castpd_si128(_mm_set_pd(*(double *)&in_v[(n+ys-3)*cstride+ix5-3], *(double *)&in_v[(n+ys-3)*cstride+ix4-3])), c45);
      __m128i xmm7 = _mm_maddubs_epi16(_mm_castpd_si128(_mm_set_pd(*(double *)&in_v[(n+ys-3)*cstride+ix7-3], *(double *)&in_v[(n+ys-3)*cstride+ix6-3])), c67); xmm6 = _mm_hadds_epi16 (xmm6, xmm7);
      xmm[n+ys] = _mm_packus_epi16(_mm_srai_epi16(_mm_adds_epi16(_mm_hadds_epi16 (xmm0, xmm2), _mm_set1_epi16(64)), 7), _mm_srai_epi16(_mm_adds_epi16(_mm_hadds_epi16 (xmm4, xmm6), _mm_set1_epi16(64)), 7));
    }
    __m128i xmm0 = _mm_maddubs_epi16(_mm_unpacklo_epi8(xmm[iy+0], xmm[iy+1]), *(__m128i*)&coeffs[(yy>>14)&63][ 0]);
    __m128i xmm1 = _mm_maddubs_epi16(_mm_unpacklo_epi8(xmm[iy+2], xmm[iy+3]), *(__m128i*)&coeffs[(yy>>14)&63][16]); xmm0 = _mm_adds_epi16(xmm0, xmm1);
    __m128i xmm2 = _mm_maddubs_epi16(_mm_unpacklo_epi8(xmm[iy+4], xmm[iy+5]), *(__m128i*)&coeffs[(yy>>14)&63][32]);
    __m128i xmm3 = _mm_maddubs_epi16(_mm_unpacklo_epi8(xmm[iy+6], xmm[iy+7]), *(__m128i*)&coeffs[(yy>>14)&63][48]); xmm2 = _mm_adds_epi16(xmm2, xmm3);
    __m128i xmm4 = _mm_maddubs_epi16(_mm_unpackhi_epi8(xmm[iy+0], xmm[iy+1]), *(__m128i*)&coeffs[(yy>>14)&63][ 0]);
    __m128i xmm5 = _mm_maddubs_epi16(_mm_unpackhi_epi8(xmm[iy+2], xmm[iy+3]), *(__m128i*)&coeffs[(yy>>14)&63][16]); xmm4 = _mm_adds_epi16(xmm4, xmm5);
    __m128i xmm6 = _mm_maddubs_epi16(_mm_unpackhi_epi8(xmm[iy+4], xmm[iy+5]), *(__m128i*)&coeffs[(yy>>14)&63][32]);
    __m128i xmm7 = _mm_maddubs_epi16(_mm_unpackhi_epi8(xmm[iy+6], xmm[iy+7]), *(__m128i*)&coeffs[(yy>>14)&63][48]); xmm6 = _mm_adds_epi16(xmm6, xmm7);

    _mm_store_si128((__m128i *)&mb_uv[16*y], _mm_packus_epi16(_mm_srai_epi16(_mm_adds_epi16(_mm_adds_epi16(xmm0, xmm2), _mm_set1_epi16(64)), 7), _mm_srai_epi16(_mm_adds_epi16(_mm_adds_epi16(xmm4, xmm6), _mm_set1_epi16(64)), 7)));
    ys = iy + 8;
  }

  free(xmm);
}
#else
static void __declspec(noinline) taa_h264_scale_mb_NxN(
  int8_t filter[64][8],
  int8_t coeffs[64][64],
  uint8_t in_y[N  ][N  ],
  uint8_t in_u[N/2][N/2],
  uint8_t in_v[N/2][N/2],
  uint8_t * mb_uv,
  const int RSZ,
  const int fineH,
  const int fineV)
{
  __declspec(align(16)) int16_t yt[16][N] = {0, };
  __declspec(align(16)) int16_t ut[8][N/2] = {0, };
  __declspec(align(16)) int16_t vt[8][N/2] = {0, };

  __assume_aligned(mb_y, 16);
  __assume_aligned(mb_uv, 16);

  // Vertical + Horizontal scaling Y
  int ys = 0;
  for (int y = 0; y<16; y++)
  {
    const int yy =  fineV + 2*RSZ*y;
    const int iy = (yy >> 20) - (fineV >> 20);
    const char * cj = filter[(yy>>14)&63];
    for(int x = 0; x<16; x++)
    {
      const int xx =  fineH + 2*RSZ*x;
      const int ix = (xx >> 20) - (fineH >> 20);
      const char * ci = filter[(xx>>14)&63];
      for(int n = 0; n<iy-ys+8; n++)
        for(int k = 0; k<8; k++)
          yt[x][n+ys] += ci[k] * in_y[n+ys][k+ix];
    }
    int16_t sj[16] = {0, };
    for(int x = 0; x<16; x++)
      for(int n = 0; n<8; n++)
        sj[x] += cj[n] * min(max((yt[x][n+iy]+64)>>7, 0), 255);
    for(int x = 0; x<16; x++)
      mb_y[16*y+x] = min(max((sj[x]+64)>>7, 0), 255);
    ys = iy + 8;
  }
  ys = 0;
  for (int y = 0; y<8; y++)
  {
    const int yy =  fineV + 2*RSZ*y;
    const int iy = (yy >> 20) - (fineV >> 20);
    const char * cj = filter[(yy>>14)&63];
    for(int x = 0; x<8; x++)
    {
      const int xx =  fineH + 2*RSZ*x;
      const int ix = (xx >> 20) - (fineH >> 20);
      const char * ci = filter[(xx>>14)&63];
      for(int n = 0; n<iy-ys+8; n++)
        for(int k = 0; k<8; k++)
          ut[x][n+ys] += ci[k] * in_u[n+ys][k+ix];
    }
    int16_t sj[8] = {0, };
    for(int x = 0; x<8; x++)
      for(int n = 0; n<8; n++)
        sj[x] += cj[n] * min(max((ut[x][n+iy]+64)>>7, 0), 255);
    for(int x = 0; x<8; x++)
      mb_u[8*y+x] = min(max((sj[x]+64)>>7, 0), 255);
    ys = iy + 8;
  }
  ys = 0;
  for (int y = 0; y<8; y++)
  {
    const int yy =  fineV + 2*RSZ*y;
    const int iy = (yy >> 20) - (fineV >> 20);
    const char * cj = filter[(yy>>14)&63];
    for(int x = 0; x<8; x++)
    {
      const int xx =  fineH + 2*RSZ*x;
      const int ix = (xx >> 20) - (fineH >> 20);
      const char * ci = filter[(xx>>14)&63];
      for(int n = 0; n<iy-ys+8; n++)
        for(int k = 0; k<8; k++)
          vt[x][n+ys] += ci[k] * in_v[n+ys][k+ix];
    }
    int16_t sj[8] = {0, };
    for(int x = 0; x<8; x++)
      for(int n = 0; n<8; n++)
        sj[x] += cj[n] * min(max((vt[x][n+iy]+64)>>7, 0), 255);
    for(int x = 0; x<8; x++)
      mb_v[8*y+x] = min(max((sj[x]+64)>>7, 0), 255);
    ys = iy + 8;
  }
}

#endif



static TAA_H264_NOINLINE void RESIZE_MB_CUBIC_FUNC(
  const uint8_t *    ld_y,
  const uint8_t *    ld_u,
  const uint8_t *    ld_v,
  uint8_t *  mb_y,
  uint8_t *  mb_uv,
  uint64_t           cbf[64],
  const int          stride,
  const int          fact)
{
  const int cstride = stride >> 1;

  TAA_H264_ALIGN(16) int32_t s[256];
  TAA_H264_ALIGN(16) int16_t t[256];
  for (int y = 0; y<16; y++)
  {
    const int yy  = y*fact-(1<<15);
    const int iy0 = (yy>>16)+0;
    const int iy1 = (yy>>16)+1;
    const int16_t bf = (int16_t)((yy&0xffff)>>8);
    const int16_t tf = (int16_t)(255-bf);
    const __m128i b = _mm_set_epi16(bf, tf, bf, tf, bf, tf, bf, tf);
    for (int x = 0; x<16; x += 4)
    {
      const int xx0 = (x+0)*fact-(1<<15);
      const int xx1 = (x+1)*fact-(1<<15);
      const int xx2 = (x+2)*fact-(1<<15);
      const int xx3 = (x+3)*fact-(1<<15);

      const int ix0 = (xx0>>16)-1;
      const int ix1 = (xx1>>16)-1;
      const int ix2 = (xx2>>16)-1;
      const int ix3 = (xx3>>16)-1;

      __m128i v0 = _mm_set_epi32(*(int*)&ld_y[iy1*stride+ix1], *(int*)&ld_y[iy0*stride+ix1], *(int*)&ld_y[iy1*stride+ix0], *(int*)&ld_y[iy0*stride+ix0]);
      __m128i v1 = _mm_set_epi32(*(int*)&ld_y[iy1*stride+ix3], *(int*)&ld_y[iy0*stride+ix3], *(int*)&ld_y[iy1*stride+ix2], *(int*)&ld_y[iy0*stride+ix2]);
      __m128i z0 = _mm_castpd_si128(_mm_set_pd(*(double*)&cbf[(xx1>>10)&63], *(double*)&cbf[(xx0>>10)&63]));
      __m128i z1 = _mm_castpd_si128(_mm_set_pd(*(double*)&cbf[(xx3>>10)&63], *(double*)&cbf[(xx2>>10)&63]));

      _mm_store_si128((__m128i *)&s[16*y+x], _mm_madd_epi16(_mm_hadds_epi16 (_mm_maddubs_epi16(v0, z0), _mm_maddubs_epi16(v1, z1)), b));
    }
  }
  __assume_aligned(mb_y, 16);
  for (int x = 0; x<256; x++)
    s[x] = (s[x]+8192)>>14;
  for (int x = 0; x<256; x++)
    t[x] = (int16_t) min(max(s[x], -32768), 32767);
  for (int x = 0; x<256; x++)
    mb_y[x] = (uint8_t) min(max(t[x], 0), 255);

  for (int y = 0; y<8; y++)
  {
    const int yy  = y*fact-(1<<15);
    const int iy0 = (yy>>16)+0;
    const int iy1 = (yy>>16)+1;
    const int16_t bf = (int16_t)((yy&0xffff)>>8);
    const int16_t tf = (int16_t)(255-bf);
    const __m128i b = _mm_set_epi16(bf, tf, bf, tf, bf, tf, bf, tf);
    for (int x = 0; x<8; x += 4)
    {
      const int xx0 = (x+0)*fact-(1<<15);
      const int xx1 = (x+1)*fact-(1<<15);
      const int xx2 = (x+2)*fact-(1<<15);
      const int xx3 = (x+3)*fact-(1<<15);

      const int ix0 = (xx0>>16)-1;
      const int ix1 = (xx1>>16)-1;
      const int ix2 = (xx2>>16)-1;
      const int ix3 = (xx3>>16)-1;

      __m128i u_v0 = _mm_set_epi32(*(int*)&ld_u[iy1*cstride+ix1], *(int*)&ld_u[iy0*cstride+ix1], *(int*)&ld_u[iy1*cstride+ix0], *(int*)&ld_u[iy0*cstride+ix0]);
      __m128i u_v1 = _mm_set_epi32(*(int*)&ld_u[iy1*cstride+ix3], *(int*)&ld_u[iy0*cstride+ix3], *(int*)&ld_u[iy1*cstride+ix2], *(int*)&ld_u[iy0*cstride+ix2]);
      __m128i u_z0 = _mm_castpd_si128(_mm_set_pd(*(double*)&cbf[(xx1>>10)&63], *(double*)&cbf[(xx0>>10)&63]));
      __m128i u_z1 = _mm_castpd_si128(_mm_set_pd(*(double*)&cbf[(xx3>>10)&63], *(double*)&cbf[(xx2>>10)&63]));

      __m128i v_v0 = _mm_set_epi32(*(int*)&ld_v[iy1*cstride+ix1], *(int*)&ld_v[iy0*cstride+ix1], *(int*)&ld_v[iy1*cstride+ix0], *(int*)&ld_v[iy0*cstride+ix0]);
      __m128i v_v1 = _mm_set_epi32(*(int*)&ld_v[iy1*cstride+ix3], *(int*)&ld_v[iy0*cstride+ix3], *(int*)&ld_v[iy1*cstride+ix2], *(int*)&ld_v[iy0*cstride+ix2]);
      __m128i v_z0 = _mm_castpd_si128(_mm_set_pd(*(double*)&cbf[(xx1>>10)&63], *(double*)&cbf[(xx0>>10)&63]));
      __m128i v_z1 = _mm_castpd_si128(_mm_set_pd(*(double*)&cbf[(xx3>>10)&63], *(double*)&cbf[(xx2>>10)&63]));

      _mm_store_si128((__m128i *)&s[16*y+x+0], _mm_madd_epi16(_mm_hadds_epi16 (_mm_maddubs_epi16(u_v0, u_z0), _mm_maddubs_epi16(u_v1, u_z1)), b));
      _mm_store_si128((__m128i *)&s[16*y+x+8], _mm_madd_epi16(_mm_hadds_epi16 (_mm_maddubs_epi16(v_v0, v_z0), _mm_maddubs_epi16(v_v1, v_z1)), b));
    }
  }
  __assume_aligned(mb_uv, 16);
  for (int x = 0; x<128; x++)
    s[x] = (s[x]+8192)>>14;
  for (int x = 0; x<128; x++)
    t[x] = (int16_t) min(max(s[x], -32768), 32767);
  for (int x = 0; x<128; x++)
    mb_uv[x] = (uint8_t) min(max(t[x], 0), 255);
}
