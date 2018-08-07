#include "taah264stdtypes.h"

#ifdef HAVE_SSSE3
# include <tmmintrin.h>
static __inline
int quant_ssse3(
  const int16_t qc,
  const int     q_bits,
  const int     q_offset,
  int16_t       tcoeff[16],
  uint8_t       qcoeff[16],
  int8_t        sign[16]
  )
{
  __m128i xmm0 = (*(__m128i *) &tcoeff[0]);
  __m128i xmm1 = (*(__m128i *) &tcoeff[8]);
  __m128i xmm2 = _mm_mullo_epi16(xmm0, _mm_set1_epi16(qc));
  __m128i xmm3 = _mm_mulhi_epi16(xmm0, _mm_set1_epi16(qc));
  __m128i xmm4 = _mm_mullo_epi16(xmm1, _mm_set1_epi16(qc));
  __m128i xmm5 = _mm_mulhi_epi16(xmm1, _mm_set1_epi16(qc));
  __m128i xmm6 = _mm_unpacklo_epi16(xmm2, xmm3);
  __m128i xmm7 = _mm_unpackhi_epi16(xmm2, xmm3);
  __m128i xmm8 = _mm_unpacklo_epi16(xmm4, xmm5);
  __m128i xmm9 = _mm_unpackhi_epi16(xmm4, xmm5);
  xmm6 = _mm_add_epi32(xmm6, _mm_sign_epi32(_mm_set1_epi32(q_offset), xmm6));
  xmm7 = _mm_add_epi32(xmm7, _mm_sign_epi32(_mm_set1_epi32(q_offset), xmm7));
  xmm8 = _mm_add_epi32(xmm8, _mm_sign_epi32(_mm_set1_epi32(q_offset), xmm8));
  xmm9 = _mm_add_epi32(xmm9, _mm_sign_epi32(_mm_set1_epi32(q_offset), xmm9));
  xmm6 = _mm_srai_epi32(xmm6, q_bits);
  xmm7 = _mm_srai_epi32(xmm7, q_bits);
  xmm8 = _mm_srai_epi32(xmm8, q_bits);
  xmm9 = _mm_srai_epi32(xmm9, q_bits);
  xmm0 = _mm_cmplt_epi16(xmm0, _mm_setzero_si128());
  xmm1 = _mm_cmplt_epi16(xmm1, _mm_setzero_si128());
  // reduce from signed 16 bits to 9 bits (sign array + unsigned 8 bits)
  // this is exact for most cases, but an approximation for low QP
  __m128i xmm10 = _mm_max_epi16(_mm_min_epi16(_mm_sub_epi16(_mm_packs_epi32(xmm6, xmm7), xmm0), _mm_set1_epi16(255)), _mm_set1_epi16(-255));
  __m128i xmm11 = _mm_max_epi16(_mm_min_epi16(_mm_sub_epi16(_mm_packs_epi32(xmm8, xmm9), xmm1), _mm_set1_epi16(255)), _mm_set1_epi16(-255));
  _mm_store_si128((__m128i *)&tcoeff[0], xmm10);
  _mm_store_si128((__m128i *)&tcoeff[8], xmm11);
  __m128i xmm14 = _mm_shuffle_epi8(_mm_packs_epi16( xmm0,  xmm1), (*(__m128i *)zagzig));
  __m128i xmm15 = _mm_shuffle_epi8(_mm_packus_epi16(_mm_abs_epi16(xmm10), _mm_abs_epi16(xmm11)), (*(__m128i *)zagzig));
  _mm_store_si128((__m128i *)sign, xmm14);
  _mm_store_si128((__m128i *)qcoeff, xmm15);
  // return flags for coeffs not equal zero
  return(_mm_movemask_epi8(_mm_xor_si128(_mm_cmpeq_epi8(xmm15, _mm_setzero_si128()), _mm_cmpeq_epi16(xmm15, xmm15))));
}
static __inline
int quant_dequant_ssse3(
  const int qp_div_6,
  const int qp_mod_6,
  const int q_bits,
  const int q_offset,
  __m128i   dct[2],
  uint8_t   qcoeff[16],
  int8_t    sign[16]
  )
{
  __m128i xmm0 = dct[0];
  __m128i xmm1 = dct[1];
  __m128i xmm2 = _mm_mullo_epi16(xmm0, (*(__m128i *)&quant[qp_mod_6<<3]));
  __m128i xmm3 = _mm_mulhi_epi16(xmm0, (*(__m128i *)&quant[qp_mod_6<<3]));
  __m128i xmm4 = _mm_mullo_epi16(xmm1, (*(__m128i *)&quant[qp_mod_6<<3]));
  __m128i xmm5 = _mm_mulhi_epi16(xmm1, (*(__m128i *)&quant[qp_mod_6<<3]));
  __m128i xmm6 = _mm_unpacklo_epi16(xmm2, xmm3);
  __m128i xmm7 = _mm_unpackhi_epi16(xmm2, xmm3);
  __m128i xmm8 = _mm_unpacklo_epi16(xmm4, xmm5);
  __m128i xmm9 = _mm_unpackhi_epi16(xmm4, xmm5);
  xmm6 = _mm_add_epi32(xmm6, _mm_sign_epi32(_mm_set1_epi32(q_offset), xmm6));
  xmm7 = _mm_add_epi32(xmm7, _mm_sign_epi32(_mm_set1_epi32(q_offset), xmm7));
  xmm8 = _mm_add_epi32(xmm8, _mm_sign_epi32(_mm_set1_epi32(q_offset), xmm8));
  xmm9 = _mm_add_epi32(xmm9, _mm_sign_epi32(_mm_set1_epi32(q_offset), xmm9));
  xmm6 = _mm_srai_epi32(xmm6, q_bits);
  xmm7 = _mm_srai_epi32(xmm7, q_bits);
  xmm8 = _mm_srai_epi32(xmm8, q_bits);
  xmm9 = _mm_srai_epi32(xmm9, q_bits);
  xmm0 = _mm_cmplt_epi16(xmm0, _mm_setzero_si128());
  xmm1 = _mm_cmplt_epi16(xmm1, _mm_setzero_si128());
  // reduce from signed 16 bits to 9 bits (sign array + unsigned 8 bits)
  // this is exact for most cases, but an approximation for low QP
  __m128i xmm10 = _mm_max_epi16(_mm_min_epi16(_mm_sub_epi16(_mm_packs_epi32(xmm6, xmm7), xmm0), _mm_set1_epi16(255)), _mm_set1_epi16(-255));
  __m128i xmm11 = _mm_max_epi16(_mm_min_epi16(_mm_sub_epi16(_mm_packs_epi32(xmm8, xmm9), xmm1), _mm_set1_epi16(255)), _mm_set1_epi16(-255));
  __m128i xmm12 = _mm_mullo_epi16(xmm10, (*(__m128i *)&de_quant[qp_mod_6<<3]));
  __m128i xmm13 = _mm_mullo_epi16(xmm11, (*(__m128i *)&de_quant[qp_mod_6<<3]));
  __m128i xmm14 = _mm_shuffle_epi8(_mm_packs_epi16(xmm0,   xmm1), (*(__m128i *)zagzig));
  __m128i xmm15 = _mm_shuffle_epi8(_mm_packus_epi16(_mm_abs_epi16(xmm10), _mm_abs_epi16(xmm11)), (*(__m128i *)zagzig));
  _mm_store_si128((__m128i *)sign, xmm14);
  _mm_store_si128((__m128i *)qcoeff, xmm15);
  dct[0] = _mm_slli_epi16(xmm12, qp_div_6);
  dct[1] = _mm_slli_epi16(xmm13, qp_div_6);
  // return flags for coeffs not equal zero
  return(_mm_movemask_epi8(_mm_xor_si128(_mm_cmpeq_epi8(xmm15, _mm_setzero_si128()), _mm_cmpeq_epi16(xmm15, xmm15))));
}
#else
# include <emmintrin.h>

// raster to zigzag
static int const TAA_H264_ALIGN(16) zigzag[16] =
{
  0, 1, 5, 6, 2, 4, 7, 12, 3, 8, 11, 13, 9, 10, 14, 15
};

static __m128i __inline _mm_abs_epi16_sse2(__m128i a)
{
  __m128i negative = _mm_cmplt_epi16(a, _mm_setzero_si128());
  return _mm_add_epi16(_mm_xor_si128(a, negative), _mm_srli_epi16(negative, 15));
}
static __m128i __inline _mm_sign_epi32_sse2(
  __m128i a,
  __m128i b)
{
  __m128i ap, an, c, d, one;
  d  = _mm_cmpgt_epi32(b, _mm_setzero_si128());
  ap = _mm_and_si128(a, d);
  c   = _mm_cmplt_epi32(b, _mm_setzero_si128());
  one = _mm_set1_epi32(1);
  an  = _mm_and_si128(a, c);
  an  = _mm_xor_si128(an, c);
  one = _mm_and_si128(one, c);
  an  = _mm_add_epi8(an, one);
  return(_mm_or_si128(an, ap));
}

static int __inline
quant_sse2(
  const int16_t qc,
  const int     q_bits,
  const int     q_offset,
  int16_t       tcoeff[16],
  uint8_t       qcoeff[16],
  int8_t        sign[16]
  )
{
  __m128i xmm0 = (*(__m128i *) &tcoeff[0]);
  __m128i xmm1 = (*(__m128i *) &tcoeff[8]);
  __m128i xmm2 = _mm_mullo_epi16(xmm0, _mm_set1_epi16(qc));
  __m128i xmm3 = _mm_mulhi_epi16(xmm0, _mm_set1_epi16(qc));
  __m128i xmm4 = _mm_mullo_epi16(xmm1, _mm_set1_epi16(qc));
  __m128i xmm5 = _mm_mulhi_epi16(xmm1, _mm_set1_epi16(qc));
  __m128i xmm6 = _mm_unpacklo_epi16(xmm2, xmm3);
  __m128i xmm7 = _mm_unpackhi_epi16(xmm2, xmm3);
  __m128i xmm8 = _mm_unpacklo_epi16(xmm4, xmm5);
  __m128i xmm9 = _mm_unpackhi_epi16(xmm4, xmm5);
  xmm6 = _mm_add_epi32(xmm6, _mm_sign_epi32_sse2(_mm_set1_epi32(q_offset), xmm6));
  xmm7 = _mm_add_epi32(xmm7, _mm_sign_epi32_sse2(_mm_set1_epi32(q_offset), xmm7));
  xmm8 = _mm_add_epi32(xmm8, _mm_sign_epi32_sse2(_mm_set1_epi32(q_offset), xmm8));
  xmm9 = _mm_add_epi32(xmm9, _mm_sign_epi32_sse2(_mm_set1_epi32(q_offset), xmm9));
  xmm6 = _mm_srai_epi32(xmm6, q_bits);
  xmm7 = _mm_srai_epi32(xmm7, q_bits);
  xmm8 = _mm_srai_epi32(xmm8, q_bits);
  xmm9 = _mm_srai_epi32(xmm9, q_bits);
  xmm0 = _mm_cmplt_epi16(xmm0, _mm_setzero_si128());
  xmm1 = _mm_cmplt_epi16(xmm1, _mm_setzero_si128());
  // reduce from signed 16 bits to 9 bits (sign array + unsigned 8 bits)
  // this is exact for most cases, but an approximation for low QP
  __m128i xmm10 = _mm_max_epi16(_mm_min_epi16(_mm_sub_epi16(_mm_packs_epi32(xmm6, xmm7), xmm0), _mm_set1_epi16(255)), _mm_set1_epi16(-255));
  __m128i xmm11 = _mm_max_epi16(_mm_min_epi16(_mm_sub_epi16(_mm_packs_epi32(xmm8, xmm9), xmm1), _mm_set1_epi16(255)), _mm_set1_epi16(-255));
  _mm_store_si128((__m128i *)&tcoeff[0], xmm10);
  _mm_store_si128((__m128i *)&tcoeff[8], xmm11);
  __m128i xmm14 = _mm_packs_epi16( xmm0,  xmm1);
  __m128i xmm15 = _mm_packus_epi16(_mm_abs_epi16_sse2(xmm10), _mm_abs_epi16_sse2(xmm11));
  _mm_store_si128((__m128i *)sign, xmm14);
  _mm_store_si128((__m128i *)qcoeff, xmm15);
  // return flags for coeffs not equal zero
  return(_mm_movemask_epi8(_mm_xor_si128(_mm_cmpeq_epi8(xmm15, _mm_setzero_si128()), _mm_cmpeq_epi16(xmm15, xmm15))));
}
static int __inline
quant_dequant_sse2(
  const int qp_div_6,
  const int qp_mod_6,
  const int q_bits,
  const int q_offset,
  __m128i   dct[2],
  uint8_t   qcoeff[16],
  int8_t    sign[16]
  )
{
  __m128i xmm0 = dct[0];
  __m128i xmm1 = dct[1];
  __m128i xmm2 = _mm_mullo_epi16(xmm0, (*(__m128i *)&quant[qp_mod_6<<3]));
  __m128i xmm3 = _mm_mulhi_epi16(xmm0, (*(__m128i *)&quant[qp_mod_6<<3]));
  __m128i xmm4 = _mm_mullo_epi16(xmm1, (*(__m128i *)&quant[qp_mod_6<<3]));
  __m128i xmm5 = _mm_mulhi_epi16(xmm1, (*(__m128i *)&quant[qp_mod_6<<3]));
  __m128i xmm6 = _mm_unpacklo_epi16(xmm2, xmm3);
  __m128i xmm7 = _mm_unpackhi_epi16(xmm2, xmm3);
  __m128i xmm8 = _mm_unpacklo_epi16(xmm4, xmm5);
  __m128i xmm9 = _mm_unpackhi_epi16(xmm4, xmm5);
  xmm6 = _mm_add_epi32(xmm6, _mm_sign_epi32_sse2(_mm_set1_epi32(q_offset), xmm6));
  xmm7 = _mm_add_epi32(xmm7, _mm_sign_epi32_sse2(_mm_set1_epi32(q_offset), xmm7));
  xmm8 = _mm_add_epi32(xmm8, _mm_sign_epi32_sse2(_mm_set1_epi32(q_offset), xmm8));
  xmm9 = _mm_add_epi32(xmm9, _mm_sign_epi32_sse2(_mm_set1_epi32(q_offset), xmm9));
  xmm6 = _mm_srai_epi32(xmm6, q_bits);
  xmm7 = _mm_srai_epi32(xmm7, q_bits);
  xmm8 = _mm_srai_epi32(xmm8, q_bits);
  xmm9 = _mm_srai_epi32(xmm9, q_bits);
  xmm0 = _mm_cmplt_epi16(xmm0, _mm_setzero_si128());
  xmm1 = _mm_cmplt_epi16(xmm1, _mm_setzero_si128());
  // reduce from signed 16 bits to 9 bits (sign array + unsigned 8 bits)
  // this is exact for most cases, but an approximation for low QP
  __m128i xmm10 = _mm_max_epi16(_mm_min_epi16(_mm_sub_epi16(_mm_packs_epi32(xmm6, xmm7), xmm0), _mm_set1_epi16(255)), _mm_set1_epi16(-255));
  __m128i xmm11 = _mm_max_epi16(_mm_min_epi16(_mm_sub_epi16(_mm_packs_epi32(xmm8, xmm9), xmm1), _mm_set1_epi16(255)), _mm_set1_epi16(-255));
  __m128i xmm12 = _mm_mullo_epi16(xmm10, (*(__m128i *)&de_quant[qp_mod_6<<3]));
  __m128i xmm13 = _mm_mullo_epi16(xmm11, (*(__m128i *)&de_quant[qp_mod_6<<3]));
  __m128i xmm14 = _mm_packs_epi16(xmm0, xmm1);
  __m128i xmm15 = _mm_packus_epi16(_mm_abs_epi16_sse2(xmm10), _mm_abs_epi16_sse2(xmm11));
  _mm_store_si128((__m128i *)sign, xmm14);
  _mm_store_si128((__m128i *)qcoeff, xmm15);
  dct[0] = _mm_slli_epi16(xmm12, qp_div_6);
  dct[1] = _mm_slli_epi16(xmm13, qp_div_6);
  // return flags for coeffs not equal zero
  return(_mm_movemask_epi8(_mm_xor_si128(_mm_cmpeq_epi8(xmm15, _mm_setzero_si128()), _mm_cmpeq_epi16(xmm15, xmm15))));
}
#endif
