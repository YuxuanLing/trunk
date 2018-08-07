static __declspec(noinline) void taa_h264_scale_cubic(
  const uint8_t *    src,
  uint8_t * restrict dst,
  uint64_t           cbf[64],
  const int          stride,
  const int          fact)
{
  __declspec(align(16)) int32_t s[N*N];
  __declspec(align(16)) int16_t t[N*N];
  for (int y = 0; y<N; y++)
  {
    const int yy  = y*fact-(1<<15);
    const int iy0 = (yy>>16)+0;
    const int iy1 = (yy>>16)+1;
    const int16_t bf = (int16_t)((yy&0xffff)>>8);
    const int16_t tf = (int16_t)(255-bf);
    const __m128i b = _mm_set_epi16(bf, tf, bf, tf, bf, tf, bf, tf);
    for (int x = 0; x<N; x += 4)
    {
      const int xx0 = (x+0)*fact-(1<<15);
      const int xx1 = (x+1)*fact-(1<<15);
      const int xx2 = (x+2)*fact-(1<<15);
      const int xx3 = (x+3)*fact-(1<<15);

      const int ix0 = (xx0>>16)-1;
      const int ix1 = (xx1>>16)-1;
      const int ix2 = (xx2>>16)-1;
      const int ix3 = (xx3>>16)-1;

      __m128i v0 = _mm_set_epi32(*(int*)&src[iy1*stride+ix1], *(int*)&src[iy0*stride+ix1], *(int*)&src[iy1*stride+ix0], *(int*)&src[iy0*stride+ix0]);
      __m128i v1 = _mm_set_epi32(*(int*)&src[iy1*stride+ix3], *(int*)&src[iy0*stride+ix3], *(int*)&src[iy1*stride+ix2], *(int*)&src[iy0*stride+ix2]);
      __m128i z0 = _mm_castpd_si128(_mm_set_pd(*(double*)&cbf[(xx1>>10)&63], *(double*)&cbf[(xx0>>10)&63]));
      __m128i z1 = _mm_castpd_si128(_mm_set_pd(*(double*)&cbf[(xx3>>10)&63], *(double*)&cbf[(xx2>>10)&63]));

      _mm_store_si128((__m128i *)&s[N*y+x], _mm_madd_epi16(_mm_hadds_epi16 (_mm_maddubs_epi16(v0, z0), _mm_maddubs_epi16(v1, z1)), b));
    }
  }
  __assume_aligned(dst, 16);
  for (int x = 0; x<N*N; x++)
    s[x] = (s[x]+8192)>>14;
  for (int x = 0; x<N*N; x++)
    t[x] = (int16_t) min(max(s[x], -32768), 32767);
  for (int x = 0; x<N*N; x++)
    dst[x] = (uint8_t) min(max(t[x], 0), 255);
}
