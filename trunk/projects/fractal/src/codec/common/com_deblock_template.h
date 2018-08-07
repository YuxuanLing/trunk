#include "com_common.h"
#include <string.h>
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
#pragma warning( disable : 810 )
#pragma warning( disable : 981 )
#pragma warning( disable : 1418 )
#pragma warning( disable : 2259 )

#define STRIDE_Y (NUM_MBS_X * 16 + 2 * PAD_HORZ_Y)
#define STRIDE_UV (NUM_MBS_X * 8 + 2 * PAD_HORZ_UV)


static void TAA_H264_DEBLOCKING_FILTER_LUMA (
  mb_deblock_t *            deblock_frame,
  uint8_t *  src_ext
#if !defined(NUM_MBS_X) && !defined(NUM_MBS_Y)
  ,
  const int                         NUM_MBS_X
  ,
  const int                         NUM_MBS_Y
#endif
  )
{
  __m128i X[20], Y[20];
  int mbx, mby;
  for ( mby = 0; mby < NUM_MBS_Y; mby++ )
  {
    mb_deblock_t * mb_deblock = deblock_frame + mby * NUM_MBS_X;
    for (mbx = 0; mbx < NUM_MBS_X; mbx++)
    {
      /********************************************************/
      /*              Load 16x16 Luma Macroblock              */
      /********************************************************/
      uint8_t *  p = src_ext + mbx * 16 + 16 * STRIDE_Y * mby;
      X[ 0] = _mm_load_si128((__m128i *)(p - STRIDE_Y *  4));
      X[ 1] = _mm_load_si128((__m128i *)(p - STRIDE_Y *  3));
      X[ 2] = _mm_load_si128((__m128i *)(p - STRIDE_Y *  2));
      X[ 3] = _mm_load_si128((__m128i *)(p - STRIDE_Y *  1));
      X[ 4] = _mm_load_si128((__m128i *)(p + STRIDE_Y *  0));
      X[ 5] = _mm_load_si128((__m128i *)(p + STRIDE_Y *  1));
      X[ 6] = _mm_load_si128((__m128i *)(p + STRIDE_Y *  2));
      X[ 7] = _mm_load_si128((__m128i *)(p + STRIDE_Y *  3));
      X[ 8] = _mm_load_si128((__m128i *)(p + STRIDE_Y *  4));
      X[ 9] = _mm_load_si128((__m128i *)(p + STRIDE_Y *  5));
      X[10] = _mm_load_si128((__m128i *)(p + STRIDE_Y *  6));
      X[11] = _mm_load_si128((__m128i *)(p + STRIDE_Y *  7));
      X[12] = _mm_load_si128((__m128i *)(p + STRIDE_Y *  8));
      X[13] = _mm_load_si128((__m128i *)(p + STRIDE_Y *  9));
      X[14] = _mm_load_si128((__m128i *)(p + STRIDE_Y * 10));
      X[15] = _mm_load_si128((__m128i *)(p + STRIDE_Y * 11));
      X[16] = _mm_load_si128((__m128i *)(p + STRIDE_Y * 12));
      X[17] = _mm_load_si128((__m128i *)(p + STRIDE_Y * 13));
      X[18] = _mm_load_si128((__m128i *)(p + STRIDE_Y * 14));
      X[19] = _mm_load_si128((__m128i *)(p + STRIDE_Y * 15));

      if (mb_deblock->do_filter)
      {
        int edge, edge_start_x, edge_end_x, edge_start_y, edge_end_y;
        int do_filter = mb_deblock->do_filter;
        const int vert_strength1 = mb_deblock->vert_strength1;
        const int vert_strength2 = mb_deblock->vert_strength2;
        const int horz_strength1 = mb_deblock->horz_strength1;
        const int horz_strength2 = mb_deblock->horz_strength2;

        /********************************************************/
        /*            16x16 matrix transpose X -> Y             */
        /********************************************************/
        TAA_H264_16X16_TRANSPOSE(&X[4], &Y[4]);

        edge_start_y = !mbx || !(do_filter & 0x1111);
        edge_end_y   = (do_filter & 0xeeee) ? 4 : 1;

        for (edge = edge_start_y; edge < edge_end_y; edge++)
        {
          const int curr_quant = (edge) ? mb_deblock->curr_qp : mb_deblock->left_qp;
          const int offset = curr_quant * 3 + 3 * (12 + mb_deblock->filter_offset_a);
          __m128i _Alpha = _mm_set1_epi16(alpha_tab[12 + mb_deblock->filter_offset_a + curr_quant]);
          __m128i _Beta = _mm_set1_epi16(beta_tab[12 + mb_deblock->filter_offset_b + curr_quant]);

          /* ===================================== */
          /*     VERTICAL LUMA STRONG FILTER */
          /* ===================================== */
          if (edge == 0 && (vert_strength2 & 0x1))
          {
            __m128i O = _mm_setzero_si128();
            __m128i _L3_lo, _L2_lo, _L1_lo, _L0_lo, _R0_lo, _R1_lo, _R2_lo, _R3_lo;
            __m128i _L3_hi, _L2_hi, _L1_hi, _L0_hi, _R0_hi, _R1_hi, _R2_hi, _R3_hi;
            __m128i _doFilter_lo, _bSmallGap_lo, _bFilterL1_lo, _bFilterR1_lo, _rfilter_lo, _lfilter_lo, _l0_lo, _l2_lo, _r2_lo, _l1_lo, _r2_hi, _l1_hi, _r0_hi, _r1_hi;
            __m128i _doFilter_hi, _bSmallGap_hi, _bFilterL1_hi, _bFilterR1_hi, _rfilter_hi, _lfilter_hi, _l0_hi, _r0_lo, _r1_lo, _l2_hi;
            __m128i _iDelta_lo, _ucAbsDelta_lo, _ucDeltap1_lo, _ucDeltaq1_lo;
            __m128i _iDelta_hi, _ucAbsDelta_hi, _ucDeltap1_hi, _ucDeltaq1_hi;

            _L3_lo = _mm_unpacklo_epi8(Y[0], O);
            _L2_lo = _mm_unpacklo_epi8(Y[1], O);
            _L1_lo = _mm_unpacklo_epi8(Y[2], O);
            _L0_lo = _mm_unpacklo_epi8(Y[3], O);
            _R0_lo = _mm_unpacklo_epi8(Y[4], O);
            _R1_lo = _mm_unpacklo_epi8(Y[5], O);
            _R2_lo = _mm_unpacklo_epi8(Y[6], O);
            _R3_lo = _mm_unpacklo_epi8(Y[7], O);

            _iDelta_lo = _mm_sub_epi16(_R0_lo, _L0_lo);
            _ucAbsDelta_lo = _mm_abs_epi16(_iDelta_lo);
            _ucDeltap1_lo = _mm_abs_epi16(_mm_sub_epi16(_L0_lo, _L1_lo));
            _ucDeltaq1_lo = _mm_abs_epi16(_mm_sub_epi16(_R0_lo, _R1_lo));

            _doFilter_lo  = _mm_and_si128(_mm_and_si128(_mm_cmplt_epi16(_ucDeltap1_lo, _Beta), _mm_cmplt_epi16(_ucDeltaq1_lo, _Beta)), _mm_cmplt_epi16(_ucAbsDelta_lo, _Alpha));
            _bSmallGap_lo = _mm_cmplt_epi16(_ucAbsDelta_lo, _mm_add_epi16(_mm_srai_epi16(_Alpha, 2), _mm_set1_epi16(2)));
            _bFilterL1_lo = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_L0_lo, _L2_lo)), _Beta );
            _bFilterR1_lo = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_lo, _R2_lo)), _Beta );

            _lfilter_lo = _mm_and_si128(_bFilterL1_lo, _bSmallGap_lo);
            _rfilter_lo = _mm_and_si128(_bFilterR1_lo, _bSmallGap_lo);

            _l2_lo = _mm_slli_epi16(_L3_lo, 1);
            _l2_lo = _mm_add_epi16(_l2_lo, _L2_lo);
            _l2_lo = _mm_add_epi16(_l2_lo, _L2_lo);
            _l2_lo = _mm_add_epi16(_l2_lo, _L2_lo);
            _l2_lo = _mm_add_epi16(_l2_lo, _L1_lo);
            _l2_lo = _mm_add_epi16(_l2_lo, _L0_lo);
            _l2_lo = _mm_add_epi16(_l2_lo, _R0_lo);
            _l2_lo = _mm_add_epi16(_l2_lo, _mm_set1_epi16(4));
            _l2_lo = _mm_srai_epi16(_l2_lo, 3);
            _l2_lo = _mm_or_si128(_mm_and_si128(_lfilter_lo, _l2_lo), _mm_andnot_si128(_lfilter_lo, _L2_lo));

            _r2_lo = _mm_slli_epi16(_R3_lo, 1);
            _r2_lo = _mm_add_epi16(_r2_lo, _R2_lo);
            _r2_lo = _mm_add_epi16(_r2_lo, _R2_lo);
            _r2_lo = _mm_add_epi16(_r2_lo, _R2_lo);
            _r2_lo = _mm_add_epi16(_r2_lo, _R1_lo);
            _r2_lo = _mm_add_epi16(_r2_lo, _R0_lo);
            _r2_lo = _mm_add_epi16(_r2_lo, _L0_lo);
            _r2_lo = _mm_add_epi16(_r2_lo, _mm_set1_epi16(4));
            _r2_lo = _mm_srai_epi16(_r2_lo, 3);
            _r2_lo = _mm_or_si128(_mm_and_si128(_rfilter_lo, _r2_lo), _mm_andnot_si128(_rfilter_lo, _R2_lo));

            /* strange bug _l2_lo = _mm_or_si128(_mm_and_si128(_lfilter_lo,_mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_mm_slli_epi16(_L3_lo,1),_L2_lo),_mm_add_epi16(_L2_lo,_L2_lo)),_mm_add_epi16(_mm_add_epi16(_L1_lo,_R0_lo),_mm_add_epi16(_R0_lo,_mm_set1_epi16(4)))),3)),_mm_andnot_si128(_lfilter_lo,_L2_lo)); */
            _l1_lo = _mm_or_si128(_mm_and_si128(_lfilter_lo, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_L2_lo, _L1_lo), _mm_add_epi16(_L0_lo, _R0_lo)), _mm_set1_epi16(2)), 2)), _mm_andnot_si128(_lfilter_lo, _L1_lo));
            _l0_lo = _mm_or_si128(_mm_and_si128(_lfilter_lo, _mm_srai_epi16(_mm_add_epi16(_L2_lo, _mm_add_epi16(_mm_slli_epi16(_mm_add_epi16(_mm_add_epi16(_L1_lo, _L0_lo), _R0_lo), 1), _mm_add_epi16(_R1_lo, _mm_set1_epi16(4)))), 3)), _mm_andnot_si128(_lfilter_lo, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_L1_lo, _L1_lo), _mm_add_epi16(_L0_lo, _R1_lo)), _mm_set1_epi16(2)), 2)));
            _r0_lo = _mm_or_si128(_mm_and_si128(_rfilter_lo, _mm_srai_epi16(_mm_add_epi16(_R2_lo, _mm_add_epi16(_mm_slli_epi16(_mm_add_epi16(_mm_add_epi16(_R1_lo, _R0_lo), _L0_lo), 1), _mm_add_epi16(_L1_lo, _mm_set1_epi16(4)))), 3)), _mm_andnot_si128(_rfilter_lo, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_R1_lo, _R1_lo), _mm_add_epi16(_R0_lo, _L1_lo)), _mm_set1_epi16(2)), 2)));
            _r1_lo = _mm_or_si128(_mm_and_si128(_rfilter_lo, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_R2_lo, _R1_lo), _mm_add_epi16(_R0_lo, _L0_lo)), _mm_set1_epi16(2)), 2)), _mm_andnot_si128(_rfilter_lo, _R1_lo));
            /* strange bug _r2_lo = _mm_or_si128(_mm_and_si128(_rfilter_lo,_mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_mm_slli_epi16(_R3_lo,1),_R2_lo),_mm_add_epi16(_R2_lo,_R2_lo)),_mm_add_epi16(_mm_add_epi16(_R1_lo,_L0_lo),_mm_add_epi16(_L0_lo,_mm_set1_epi16(4)))),3)),_mm_andnot_si128(_rfilter_lo,_R2_lo)); */

            _L3_hi = _mm_unpackhi_epi8(Y[0], O);
            _L2_hi = _mm_unpackhi_epi8(Y[1], O);
            _L1_hi = _mm_unpackhi_epi8(Y[2], O);
            _L0_hi = _mm_unpackhi_epi8(Y[3], O);
            _R0_hi = _mm_unpackhi_epi8(Y[4], O);
            _R1_hi = _mm_unpackhi_epi8(Y[5], O);
            _R2_hi = _mm_unpackhi_epi8(Y[6], O);
            _R3_hi = _mm_unpackhi_epi8(Y[7], O);

            _iDelta_hi = _mm_sub_epi16(_R0_hi, _L0_hi);
            _ucAbsDelta_hi = _mm_abs_epi16(_iDelta_hi);
            _ucDeltap1_hi = _mm_abs_epi16(_mm_sub_epi16(_L0_hi, _L1_hi));
            _ucDeltaq1_hi = _mm_abs_epi16(_mm_sub_epi16(_R0_hi, _R1_hi));

            _doFilter_hi  = _mm_and_si128(_mm_and_si128(_mm_cmplt_epi16(_ucDeltap1_hi, _Beta), _mm_cmplt_epi16(_ucDeltaq1_hi, _Beta)), _mm_cmplt_epi16(_ucAbsDelta_hi, _Alpha));
            _bSmallGap_hi = _mm_cmplt_epi16(_ucAbsDelta_hi, _mm_add_epi16(_mm_srai_epi16(_Alpha, 2), _mm_set1_epi16(2)));
            _bFilterL1_hi = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_L0_hi, _L2_hi)), _Beta );
            _bFilterR1_hi = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_hi, _R2_hi)), _Beta );

            _lfilter_hi = _mm_and_si128(_bFilterL1_hi, _bSmallGap_hi);
            _rfilter_hi = _mm_and_si128(_bFilterR1_hi, _bSmallGap_hi);

            _l2_hi = _mm_slli_epi16(_L3_hi, 1);
            _l2_hi = _mm_add_epi16(_l2_hi, _L2_hi);
            _l2_hi = _mm_add_epi16(_l2_hi, _L2_hi);
            _l2_hi = _mm_add_epi16(_l2_hi, _L2_hi);
            _l2_hi = _mm_add_epi16(_l2_hi, _L1_hi);
            _l2_hi = _mm_add_epi16(_l2_hi, _L0_hi);
            _l2_hi = _mm_add_epi16(_l2_hi, _R0_hi);
            _l2_hi = _mm_add_epi16(_l2_hi, _mm_set1_epi16(4));
            _l2_hi = _mm_srai_epi16(_l2_hi, 3);
            _l2_hi = _mm_or_si128(_mm_and_si128(_lfilter_hi, _l2_hi), _mm_andnot_si128(_lfilter_hi, _L2_hi));

            _r2_hi = _mm_slli_epi16(_R3_hi, 1);
            _r2_hi = _mm_add_epi16(_r2_hi, _R2_hi);
            _r2_hi = _mm_add_epi16(_r2_hi, _R2_hi);
            _r2_hi = _mm_add_epi16(_r2_hi, _R2_hi);
            _r2_hi = _mm_add_epi16(_r2_hi, _R1_hi);
            _r2_hi = _mm_add_epi16(_r2_hi, _R0_hi);
            _r2_hi = _mm_add_epi16(_r2_hi, _L0_hi);
            _r2_hi = _mm_add_epi16(_r2_hi, _mm_set1_epi16(4));
            _r2_hi = _mm_srai_epi16(_r2_hi, 3);
            _r2_hi = _mm_or_si128(_mm_and_si128(_rfilter_hi, _r2_hi), _mm_andnot_si128(_rfilter_hi, _R2_hi));

            /* strange bug _l2_hi = _mm_or_si128(_mm_and_si128(_lfilter_hi,_mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_mm_slli_epi16(_L3_hi,1),_L2_hi),_mm_add_epi16(_L2_hi,_L2_hi)),_mm_add_epi16(_mm_add_epi16(_L1_hi,_R0_hi),_mm_add_epi16(_R0_hi,_mm_set1_epi16(4)))),3)),_mm_andnot_si128(_lfilter_hi,_L2_hi)); */
            _l1_hi = _mm_or_si128(_mm_and_si128(_lfilter_hi, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_L2_hi, _L1_hi), _mm_add_epi16(_L0_hi, _R0_hi)), _mm_set1_epi16(2)), 2)), _mm_andnot_si128(_lfilter_hi, _L1_hi));
            _l0_hi = _mm_or_si128(_mm_and_si128(_lfilter_hi, _mm_srai_epi16(_mm_add_epi16(_L2_hi, _mm_add_epi16(_mm_slli_epi16(_mm_add_epi16(_mm_add_epi16(_L1_hi, _L0_hi), _R0_hi), 1), _mm_add_epi16(_R1_hi, _mm_set1_epi16(4)))), 3)), _mm_andnot_si128(_lfilter_hi, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_L1_hi, _L1_hi), _mm_add_epi16(_L0_hi, _R1_hi)), _mm_set1_epi16(2)), 2)));
            _r0_hi = _mm_or_si128(_mm_and_si128(_rfilter_hi, _mm_srai_epi16(_mm_add_epi16(_R2_hi, _mm_add_epi16(_mm_slli_epi16(_mm_add_epi16(_mm_add_epi16(_R1_hi, _R0_hi), _L0_hi), 1), _mm_add_epi16(_L1_hi, _mm_set1_epi16(4)))), 3)), _mm_andnot_si128(_rfilter_hi, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_R1_hi, _R1_hi), _mm_add_epi16(_R0_hi, _L1_hi)), _mm_set1_epi16(2)), 2)));
            _r1_hi = _mm_or_si128(_mm_and_si128(_rfilter_hi, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_R2_hi, _R1_hi), _mm_add_epi16(_R0_hi, _L0_hi)), _mm_set1_epi16(2)), 2)), _mm_andnot_si128(_rfilter_hi, _R1_hi));
            /* strange bug _r2_hi = _mm_or_si128(_mm_and_si128(_rfilter_hi,_mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_mm_slli_epi16(_R3_hi,1),_R2_hi),_mm_add_epi16(_R2_hi,_R2_hi)),_mm_add_epi16(_mm_add_epi16(_R1_hi,_L0_hi),_mm_add_epi16(_L0_hi,_mm_set1_epi16(4)))),3)),_mm_andnot_si128(_rfilter_hi,_R2_hi)); */

            Y[1] = _mm_packus_epi16(_mm_or_si128(_mm_and_si128(_doFilter_lo, _l2_lo), _mm_andnot_si128(_doFilter_lo, _L2_lo)), _mm_or_si128(_mm_and_si128(_doFilter_hi, _l2_hi), _mm_andnot_si128(_doFilter_hi, _L2_hi)));
            Y[2] = _mm_packus_epi16(_mm_or_si128(_mm_and_si128(_doFilter_lo, _l1_lo), _mm_andnot_si128(_doFilter_lo, _L1_lo)), _mm_or_si128(_mm_and_si128(_doFilter_hi, _l1_hi), _mm_andnot_si128(_doFilter_hi, _L1_hi)));
            Y[3] = _mm_packus_epi16(_mm_or_si128(_mm_and_si128(_doFilter_lo, _l0_lo), _mm_andnot_si128(_doFilter_lo, _L0_lo)), _mm_or_si128(_mm_and_si128(_doFilter_hi, _l0_hi), _mm_andnot_si128(_doFilter_hi, _L0_hi)));
            Y[4] = _mm_packus_epi16(_mm_or_si128(_mm_and_si128(_doFilter_lo, _r0_lo), _mm_andnot_si128(_doFilter_lo, _R0_lo)), _mm_or_si128(_mm_and_si128(_doFilter_hi, _r0_hi), _mm_andnot_si128(_doFilter_hi, _R0_hi)));
            Y[5] = _mm_packus_epi16(_mm_or_si128(_mm_and_si128(_doFilter_lo, _r1_lo), _mm_andnot_si128(_doFilter_lo, _R1_lo)), _mm_or_si128(_mm_and_si128(_doFilter_hi, _r1_hi), _mm_andnot_si128(_doFilter_hi, _R1_hi)));
            Y[6] = _mm_packus_epi16(_mm_or_si128(_mm_and_si128(_doFilter_lo, _r2_lo), _mm_andnot_si128(_doFilter_lo, _R2_lo)), _mm_or_si128(_mm_and_si128(_doFilter_hi, _r2_hi), _mm_andnot_si128(_doFilter_hi, _R2_hi)));
          }

          /* ===================================== */
          /*    VERTICAL LUMA STANDARD FILTER */
          /* ===================================== */
          else
          {
            uint16_t do_filter_curr = 0x1111 & (do_filter >> edge);
            if (do_filter_curr)
            {
              TAA_H264_ALIGN(64) int16_t do_curr[4] = { -((do_filter_curr >> 0) & 1),
                                                        -((do_filter_curr >> 4) & 1),
                                                        -((do_filter_curr >> 8) & 1),
                                                        -((do_filter_curr >> 12) & 1)};

              const int a = tc0_tab[(offset + ((vert_strength2>>(edge+ 0))&0x1) * 2 + ((vert_strength1>>(edge+ 0))&0x1))];
              const int b = tc0_tab[(offset + ((vert_strength2>>(edge+ 4))&0x1) * 2 + ((vert_strength1>>(edge+ 4))&0x1))];
              const int c = tc0_tab[(offset + ((vert_strength2>>(edge+ 8))&0x1) * 2 + ((vert_strength1>>(edge+ 8))&0x1))];
              const int d = tc0_tab[(offset + ((vert_strength2>>(edge+12))&0x1) * 2 + ((vert_strength1>>(edge+12))&0x1))];

              {
                __m128i O = _mm_setzero_si128();
                __m128i _const_4 = _mm_set1_epi16(0x0004);
                __m128i _C0_lo = _mm_set_epi32(b, b, a, a);
                __m128i _C0_hi = _mm_set_epi32(d, d, c, c);
                __m128i _dofilter_lo = _mm_set_epi16(do_curr[1], do_curr[1], do_curr[1], do_curr[1],
                                                     do_curr[0], do_curr[0], do_curr[0], do_curr[0]);
                __m128i _dofilter_hi = _mm_set_epi16(do_curr[3], do_curr[3], do_curr[3], do_curr[3],
                                                     do_curr[2], do_curr[2], do_curr[2], do_curr[2]);
                __m128i _L2_lo, _L1_lo, _L0_lo, _R0_lo, _R1_lo, _R2_lo, _dif_lo, _delta0_lo, _delta1_lo, _RL0_lo, _Delta_lo, _aq_lo, _ap_lo, _t0_lo, _t1_lo, _t2_lo, _c0_lo, _nc0_lo, _filter_lo;
                __m128i _L2_hi, _L1_hi, _L0_hi, _R0_hi, _R1_hi, _R2_hi, _dif_hi, _delta0_hi, _delta1_hi, _RL0_hi, _Delta_hi, _aq_hi, _ap_hi, _t0_hi, _t1_hi, _t2_hi, _c0_hi, _nc0_hi, _filter_hi;

                _L2_lo = _mm_unpacklo_epi8(Y[4 * edge + 1], O);
                _L1_lo = _mm_unpacklo_epi8(Y[4 * edge + 2], O);
                _L0_lo = _mm_unpacklo_epi8(Y[4 * edge + 3], O);
                _R0_lo = _mm_unpacklo_epi8(Y[4 * edge + 4], O);
                _R1_lo = _mm_unpacklo_epi8(Y[4 * edge + 5], O);
                _R2_lo = _mm_unpacklo_epi8(Y[4 * edge + 6], O);

                _ap_lo  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_L0_lo, _L2_lo)), _Beta );
                _aq_lo  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_lo, _R2_lo)), _Beta );
                _t1_lo  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_lo, _R1_lo)), _Beta );
                _t2_lo  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_L0_lo, _L1_lo)), _Beta );
                _t0_lo  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_lo, _L0_lo)), _Alpha);

                _RL0_lo = _mm_avg_epu16(_L0_lo, _R0_lo);
                _Delta_lo = _mm_sub_epi16(_R0_lo, _L0_lo);

                _c0_lo  = _mm_sub_epi16(_mm_sub_epi16(_C0_lo, _ap_lo), _aq_lo);
                _nc0_lo = _mm_sub_epi16(O, _c0_lo);
                _dif_lo = _mm_sub_epi16(_L1_lo, _R1_lo);
                _dif_lo = _mm_add_epi16(_dif_lo, _mm_slli_epi16(_Delta_lo, 2));
                _dif_lo = _mm_add_epi16(_dif_lo, _const_4);
                _dif_lo = _mm_srai_epi16(_dif_lo, 3);
                _dif_lo = _mm_min_epi16(_c0_lo, _mm_max_epi16(_nc0_lo, _dif_lo));

                _filter_lo = _mm_and_si128(_mm_and_si128(_mm_and_si128(_t0_lo, _t1_lo), _t2_lo), _dofilter_lo);
                _delta0_lo = _mm_add_epi16(_L2_lo, _RL0_lo);
                _delta1_lo = _mm_add_epi16(_R2_lo, _RL0_lo);

                _delta0_lo = _mm_srai_epi16(_mm_sub_epi16(_delta0_lo, _mm_add_epi16(_L1_lo, _L1_lo)), 1);
                _delta1_lo = _mm_srai_epi16(_mm_sub_epi16(_delta1_lo, _mm_add_epi16(_R1_lo, _R1_lo)), 1);
                _delta0_lo = _mm_min_epi16(_C0_lo, _mm_max_epi16(_mm_sub_epi16(O, _C0_lo), _delta0_lo));
                _delta1_lo = _mm_min_epi16(_C0_lo, _mm_max_epi16(_mm_sub_epi16(O, _C0_lo), _delta1_lo));

                _dif_lo    = _mm_and_si128(_dif_lo, _filter_lo);
                _delta0_lo = _mm_and_si128(_delta0_lo, _filter_lo);
                _delta0_lo = _mm_and_si128(_delta0_lo, _ap_lo);
                _delta1_lo = _mm_and_si128(_delta1_lo, _filter_lo);
                _delta1_lo = _mm_and_si128(_delta1_lo, _aq_lo);

                _L2_hi = _mm_unpackhi_epi8(Y[4 * edge + 1], O);
                _L1_hi = _mm_unpackhi_epi8(Y[4 * edge + 2], O);
                _L0_hi = _mm_unpackhi_epi8(Y[4 * edge + 3], O);
                _R0_hi = _mm_unpackhi_epi8(Y[4 * edge + 4], O);
                _R1_hi = _mm_unpackhi_epi8(Y[4 * edge + 5], O);
                _R2_hi = _mm_unpackhi_epi8(Y[4 * edge + 6], O);

                _ap_hi  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_L0_hi, _L2_hi)), _Beta );
                _aq_hi  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_hi, _R2_hi)), _Beta );
                _t1_hi  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_hi, _R1_hi)), _Beta );
                _t2_hi  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_L0_hi, _L1_hi)), _Beta );
                _t0_hi  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_hi, _L0_hi)), _Alpha);

                _RL0_hi = _mm_avg_epu16(_L0_hi, _R0_hi);
                _Delta_hi = _mm_sub_epi16(_R0_hi, _L0_hi);

                _c0_hi  = _mm_sub_epi16(_mm_sub_epi16(_C0_hi, _ap_hi), _aq_hi);
                _nc0_hi = _mm_sub_epi16(O, _c0_hi);
                _dif_hi = _mm_sub_epi16(_L1_hi, _R1_hi);
                _dif_hi = _mm_add_epi16(_dif_hi, _mm_slli_epi16(_Delta_hi, 2));
                _dif_hi = _mm_add_epi16(_dif_hi, _const_4);
                _dif_hi = _mm_srai_epi16(_dif_hi, 3);
                _dif_hi = _mm_min_epi16(_c0_hi, _mm_max_epi16(_nc0_hi, _dif_hi));

                _filter_hi = _mm_and_si128(_mm_and_si128(_mm_and_si128(_t0_hi, _t1_hi), _t2_hi), _dofilter_hi);
                _delta0_hi = _mm_add_epi16(_L2_hi, _RL0_hi);
                _delta1_hi = _mm_add_epi16(_R2_hi, _RL0_hi);

                _delta0_hi = _mm_srai_epi16(_mm_sub_epi16(_delta0_hi, _mm_add_epi16(_L1_hi, _L1_hi)), 1);
                _delta1_hi = _mm_srai_epi16(_mm_sub_epi16(_delta1_hi, _mm_add_epi16(_R1_hi, _R1_hi)), 1);
                _delta0_hi = _mm_min_epi16(_C0_hi, _mm_max_epi16(_mm_sub_epi16(O, _C0_hi), _delta0_hi));
                _delta1_hi = _mm_min_epi16(_C0_hi, _mm_max_epi16(_mm_sub_epi16(O, _C0_hi), _delta1_hi));

                _dif_hi    = _mm_and_si128(_dif_hi, _filter_hi);
                _delta0_hi = _mm_and_si128(_delta0_hi, _filter_hi);
                _delta0_hi = _mm_and_si128(_delta0_hi, _ap_hi);
                _delta1_hi = _mm_and_si128(_delta1_hi, _filter_hi);
                _delta1_hi = _mm_and_si128(_delta1_hi, _aq_hi);

                Y[4 * edge + 2] = _mm_packus_epi16(_mm_add_epi16(_L1_lo, _delta0_lo), _mm_add_epi16(_L1_hi, _delta0_hi));
                Y[4 * edge + 3] = _mm_packus_epi16(_mm_add_epi16(_L0_lo, _dif_lo),    _mm_add_epi16(_L0_hi, _dif_hi)   );
                Y[4 * edge + 4] = _mm_packus_epi16(_mm_sub_epi16(_R0_lo, _dif_lo),    _mm_sub_epi16(_R0_hi, _dif_hi)   );
                Y[4 * edge + 5] = _mm_packus_epi16(_mm_add_epi16(_R1_lo, _delta1_lo), _mm_add_epi16(_R1_hi, _delta1_hi));
              }

            }
          }
        }

        /********************************************************/
        /*            16x16 matrix transpose Y -> X             */
        /********************************************************/
        TAA_H264_16X16_TRANSPOSE(&Y[4], &X[4]);

        /* horz edge */
        do_filter >>= 16;

        edge_start_x = !mby || !(do_filter & 0xf);
        edge_end_x   = (do_filter & 0xfff0) ? 4 : 1;

        for (edge = edge_start_x; edge < edge_end_x; edge++)
        {
          const int curr_quant = (edge) ? mb_deblock->curr_qp : mb_deblock->top_qp;
          const int offset = curr_quant * 3 + 3 * (12 + mb_deblock->filter_offset_a);
          __m128i _Alpha = _mm_set1_epi16(alpha_tab[12 + mb_deblock->filter_offset_a + curr_quant]);
          __m128i _Beta = _mm_set1_epi16(beta_tab[12 + mb_deblock->filter_offset_b + curr_quant]);

          /* ===================================== */
          /*    HORIZONTAL LUMA STRONG FILTER */
          /* ===================================== */
          if (edge == 0 && (horz_strength2 & 0x1))
          {
            __m128i O = _mm_setzero_si128();
            __m128i _L3_lo, _L2_lo, _L1_lo, _L0_lo, _R0_lo, _R1_lo, _R2_lo, _R3_lo;
            __m128i _L3_hi, _L2_hi, _L1_hi, _L0_hi, _R0_hi, _R1_hi, _R2_hi, _R3_hi;
            __m128i _doFilter_lo, _bSmallGap_lo, _bFilterL1_lo, _bFilterR1_lo, _rfilter_lo, _lfilter_lo, _l0_lo, _l1_lo, _l2_lo, _r0_lo, _r1_lo, _r2_lo;
            __m128i _doFilter_hi, _bSmallGap_hi, _bFilterL1_hi, _bFilterR1_hi, _rfilter_hi, _lfilter_hi, _l0_hi, _l1_hi, _l2_hi, _r0_hi, _r1_hi, _r2_hi;
            __m128i _iDelta_lo, _ucAbsDelta_lo, _ucDeltap1_lo, _ucDeltaq1_lo;
            __m128i _iDelta_hi, _ucAbsDelta_hi, _ucDeltap1_hi, _ucDeltaq1_hi;

            _L3_lo = _mm_unpacklo_epi8(X[0], O);
            _L2_lo = _mm_unpacklo_epi8(X[1], O);
            _L1_lo = _mm_unpacklo_epi8(X[2], O);
            _L0_lo = _mm_unpacklo_epi8(X[3], O);
            _R0_lo = _mm_unpacklo_epi8(X[4], O);
            _R1_lo = _mm_unpacklo_epi8(X[5], O);
            _R2_lo = _mm_unpacklo_epi8(X[6], O);
            _R3_lo = _mm_unpacklo_epi8(X[7], O);

            _iDelta_lo = _mm_sub_epi16(_R0_lo, _L0_lo);
            _ucAbsDelta_lo = _mm_abs_epi16(_iDelta_lo);
            _ucDeltap1_lo = _mm_abs_epi16(_mm_sub_epi16(_L0_lo, _L1_lo));
            _ucDeltaq1_lo = _mm_abs_epi16(_mm_sub_epi16(_R0_lo, _R1_lo));

            _doFilter_lo  = _mm_and_si128(_mm_and_si128(_mm_cmplt_epi16(_ucDeltap1_lo, _Beta), _mm_cmplt_epi16(_ucDeltaq1_lo, _Beta)), _mm_cmplt_epi16(_ucAbsDelta_lo, _Alpha));
            _bSmallGap_lo = _mm_cmplt_epi16(_ucAbsDelta_lo, _mm_add_epi16(_mm_srai_epi16(_Alpha, 2), _mm_set1_epi16(2)));
            _bFilterL1_lo = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_L0_lo, _L2_lo)), _Beta );
            _bFilterR1_lo = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_lo, _R2_lo)), _Beta );

            _lfilter_lo = _mm_and_si128(_bFilterL1_lo, _bSmallGap_lo);
            _rfilter_lo = _mm_and_si128(_bFilterR1_lo, _bSmallGap_lo);

            _l2_lo = _mm_slli_epi16(_L3_lo, 1);
            _l2_lo = _mm_add_epi16(_l2_lo, _L2_lo);
            _l2_lo = _mm_add_epi16(_l2_lo, _L2_lo);
            _l2_lo = _mm_add_epi16(_l2_lo, _L2_lo);
            _l2_lo = _mm_add_epi16(_l2_lo, _L1_lo);
            _l2_lo = _mm_add_epi16(_l2_lo, _L0_lo);
            _l2_lo = _mm_add_epi16(_l2_lo, _R0_lo);
            _l2_lo = _mm_add_epi16(_l2_lo, _mm_set1_epi16(4));
            _l2_lo = _mm_srai_epi16(_l2_lo, 3);
            _l2_lo = _mm_or_si128(_mm_and_si128(_lfilter_lo, _l2_lo), _mm_andnot_si128(_lfilter_lo, _L2_lo));

            _r2_lo = _mm_slli_epi16(_R3_lo, 1);
            _r2_lo = _mm_add_epi16(_r2_lo, _R2_lo);
            _r2_lo = _mm_add_epi16(_r2_lo, _R2_lo);
            _r2_lo = _mm_add_epi16(_r2_lo, _R2_lo);
            _r2_lo = _mm_add_epi16(_r2_lo, _R1_lo);
            _r2_lo = _mm_add_epi16(_r2_lo, _R0_lo);
            _r2_lo = _mm_add_epi16(_r2_lo, _L0_lo);
            _r2_lo = _mm_add_epi16(_r2_lo, _mm_set1_epi16(4));
            _r2_lo = _mm_srai_epi16(_r2_lo, 3);
            _r2_lo = _mm_or_si128(_mm_and_si128(_rfilter_lo, _r2_lo), _mm_andnot_si128(_rfilter_lo, _R2_lo));

            /* strange bug _l2_lo = _mm_or_si128(_mm_and_si128(_lfilter_lo,_mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_mm_slli_epi16(_L3_lo,1),_L2_lo),_mm_add_epi16(_L2_lo,_L2_lo)),_mm_add_epi16(_mm_add_epi16(_L1_lo,_R0_lo),_mm_add_epi16(_R0_lo,_mm_set1_epi16(4)))),3)),_mm_andnot_si128(_lfilter_lo,_L2_lo)); */
            _l1_lo = _mm_or_si128(_mm_and_si128(_lfilter_lo, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_L2_lo, _L1_lo), _mm_add_epi16(_L0_lo, _R0_lo)), _mm_set1_epi16(2)), 2)), _mm_andnot_si128(_lfilter_lo, _L1_lo));
            _l0_lo = _mm_or_si128(_mm_and_si128(_lfilter_lo, _mm_srai_epi16(_mm_add_epi16(_L2_lo, _mm_add_epi16(_mm_slli_epi16(_mm_add_epi16(_mm_add_epi16(_L1_lo, _L0_lo), _R0_lo), 1), _mm_add_epi16(_R1_lo, _mm_set1_epi16(4)))), 3)), _mm_andnot_si128(_lfilter_lo, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_L1_lo, _L1_lo), _mm_add_epi16(_L0_lo, _R1_lo)), _mm_set1_epi16(2)), 2)));
            _r0_lo = _mm_or_si128(_mm_and_si128(_rfilter_lo, _mm_srai_epi16(_mm_add_epi16(_R2_lo, _mm_add_epi16(_mm_slli_epi16(_mm_add_epi16(_mm_add_epi16(_R1_lo, _R0_lo), _L0_lo), 1), _mm_add_epi16(_L1_lo, _mm_set1_epi16(4)))), 3)), _mm_andnot_si128(_rfilter_lo, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_R1_lo, _R1_lo), _mm_add_epi16(_R0_lo, _L1_lo)), _mm_set1_epi16(2)), 2)));
            _r1_lo = _mm_or_si128(_mm_and_si128(_rfilter_lo, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_R2_lo, _R1_lo), _mm_add_epi16(_R0_lo, _L0_lo)), _mm_set1_epi16(2)), 2)), _mm_andnot_si128(_rfilter_lo, _R1_lo));
            /* strange bug _r2_lo = _mm_or_si128(_mm_and_si128(_rfilter_lo,_mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_mm_slli_epi16(_R3_lo,1),_R2_lo),_mm_add_epi16(_R2_lo,_R2_lo)),_mm_add_epi16(_mm_add_epi16(_R1_lo,_L0_lo),_mm_add_epi16(_L0_lo,_mm_set1_epi16(4)))),3)),_mm_andnot_si128(_rfilter_lo,_R2_lo)); */

            _L3_hi = _mm_unpackhi_epi8(X[0], O);
            _L2_hi = _mm_unpackhi_epi8(X[1], O);
            _L1_hi = _mm_unpackhi_epi8(X[2], O);
            _L0_hi = _mm_unpackhi_epi8(X[3], O);
            _R0_hi = _mm_unpackhi_epi8(X[4], O);
            _R1_hi = _mm_unpackhi_epi8(X[5], O);
            _R2_hi = _mm_unpackhi_epi8(X[6], O);
            _R3_hi = _mm_unpackhi_epi8(X[7], O);

            _iDelta_hi = _mm_sub_epi16(_R0_hi, _L0_hi);
            _ucAbsDelta_hi = _mm_abs_epi16(_iDelta_hi);
            _ucDeltap1_hi = _mm_abs_epi16(_mm_sub_epi16(_L0_hi, _L1_hi));
            _ucDeltaq1_hi = _mm_abs_epi16(_mm_sub_epi16(_R0_hi, _R1_hi));

            _doFilter_hi  = _mm_and_si128(_mm_and_si128(_mm_cmplt_epi16(_ucDeltap1_hi, _Beta), _mm_cmplt_epi16(_ucDeltaq1_hi, _Beta)), _mm_cmplt_epi16(_ucAbsDelta_hi, _Alpha));
            _bSmallGap_hi = _mm_cmplt_epi16(_ucAbsDelta_hi, _mm_add_epi16(_mm_srai_epi16(_Alpha, 2), _mm_set1_epi16(2)));
            _bFilterL1_hi = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_L0_hi, _L2_hi)), _Beta );
            _bFilterR1_hi = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_hi, _R2_hi)), _Beta );

            _lfilter_hi = _mm_and_si128(_bFilterL1_hi, _bSmallGap_hi);
            _rfilter_hi = _mm_and_si128(_bFilterR1_hi, _bSmallGap_hi);

            _l2_hi = _mm_slli_epi16(_L3_hi, 1);
            _l2_hi = _mm_add_epi16(_l2_hi, _L2_hi);
            _l2_hi = _mm_add_epi16(_l2_hi, _L2_hi);
            _l2_hi = _mm_add_epi16(_l2_hi, _L2_hi);
            _l2_hi = _mm_add_epi16(_l2_hi, _L1_hi);
            _l2_hi = _mm_add_epi16(_l2_hi, _L0_hi);
            _l2_hi = _mm_add_epi16(_l2_hi, _R0_hi);
            _l2_hi = _mm_add_epi16(_l2_hi, _mm_set1_epi16(4));
            _l2_hi = _mm_srai_epi16(_l2_hi, 3);
            _l2_hi = _mm_or_si128(_mm_and_si128(_lfilter_hi, _l2_hi), _mm_andnot_si128(_lfilter_hi, _L2_hi));

            _r2_hi = _mm_slli_epi16(_R3_hi, 1);
            _r2_hi = _mm_add_epi16(_r2_hi, _R2_hi);
            _r2_hi = _mm_add_epi16(_r2_hi, _R2_hi);
            _r2_hi = _mm_add_epi16(_r2_hi, _R2_hi);
            _r2_hi = _mm_add_epi16(_r2_hi, _R1_hi);
            _r2_hi = _mm_add_epi16(_r2_hi, _R0_hi);
            _r2_hi = _mm_add_epi16(_r2_hi, _L0_hi);
            _r2_hi = _mm_add_epi16(_r2_hi, _mm_set1_epi16(4));
            _r2_hi = _mm_srai_epi16(_r2_hi, 3);
            _r2_hi = _mm_or_si128(_mm_and_si128(_rfilter_hi, _r2_hi), _mm_andnot_si128(_rfilter_hi, _R2_hi));

            /* strange bug _l2_hi = _mm_or_si128(_mm_and_si128(_lfilter_hi,_mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_mm_slli_epi16(_L3_hi,1),_L2_hi),_mm_add_epi16(_L2_hi,_L2_hi)),_mm_add_epi16(_mm_add_epi16(_L1_hi,_R0_hi),_mm_add_epi16(_R0_hi,_mm_set1_epi16(4)))),3)),_mm_andnot_si128(_lfilter_hi,_L2_hi)); */
            _l1_hi = _mm_or_si128(_mm_and_si128(_lfilter_hi, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_L2_hi, _L1_hi), _mm_add_epi16(_L0_hi, _R0_hi)), _mm_set1_epi16(2)), 2)), _mm_andnot_si128(_lfilter_hi, _L1_hi));
            _l0_hi = _mm_or_si128(_mm_and_si128(_lfilter_hi, _mm_srai_epi16(_mm_add_epi16(_L2_hi, _mm_add_epi16(_mm_slli_epi16(_mm_add_epi16(_mm_add_epi16(_L1_hi, _L0_hi), _R0_hi), 1), _mm_add_epi16(_R1_hi, _mm_set1_epi16(4)))), 3)), _mm_andnot_si128(_lfilter_hi, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_L1_hi, _L1_hi), _mm_add_epi16(_L0_hi, _R1_hi)), _mm_set1_epi16(2)), 2)));
            _r0_hi = _mm_or_si128(_mm_and_si128(_rfilter_hi, _mm_srai_epi16(_mm_add_epi16(_R2_hi, _mm_add_epi16(_mm_slli_epi16(_mm_add_epi16(_mm_add_epi16(_R1_hi, _R0_hi), _L0_hi), 1), _mm_add_epi16(_L1_hi, _mm_set1_epi16(4)))), 3)), _mm_andnot_si128(_rfilter_hi, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_R1_hi, _R1_hi), _mm_add_epi16(_R0_hi, _L1_hi)), _mm_set1_epi16(2)), 2)));
            _r1_hi = _mm_or_si128(_mm_and_si128(_rfilter_hi, _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_R2_hi, _R1_hi), _mm_add_epi16(_R0_hi, _L0_hi)), _mm_set1_epi16(2)), 2)), _mm_andnot_si128(_rfilter_hi, _R1_hi));
            /* strange bug _r2_hi = _mm_or_si128(_mm_and_si128(_rfilter_hi,_mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_mm_slli_epi16(_R3_hi,1),_R2_hi),_mm_add_epi16(_R2_hi,_R2_hi)),_mm_add_epi16(_mm_add_epi16(_R1_hi,_L0_hi),_mm_add_epi16(_L0_hi,_mm_set1_epi16(4)))),3)),_mm_andnot_si128(_rfilter_hi,_R2_hi)); */

            X[1] = _mm_packus_epi16(_mm_or_si128(_mm_and_si128(_doFilter_lo, _l2_lo), _mm_andnot_si128(_doFilter_lo, _L2_lo)), _mm_or_si128(_mm_and_si128(_doFilter_hi, _l2_hi), _mm_andnot_si128(_doFilter_hi, _L2_hi)));
            X[2] = _mm_packus_epi16(_mm_or_si128(_mm_and_si128(_doFilter_lo, _l1_lo), _mm_andnot_si128(_doFilter_lo, _L1_lo)), _mm_or_si128(_mm_and_si128(_doFilter_hi, _l1_hi), _mm_andnot_si128(_doFilter_hi, _L1_hi)));
            X[3] = _mm_packus_epi16(_mm_or_si128(_mm_and_si128(_doFilter_lo, _l0_lo), _mm_andnot_si128(_doFilter_lo, _L0_lo)), _mm_or_si128(_mm_and_si128(_doFilter_hi, _l0_hi), _mm_andnot_si128(_doFilter_hi, _L0_hi)));
            X[4] = _mm_packus_epi16(_mm_or_si128(_mm_and_si128(_doFilter_lo, _r0_lo), _mm_andnot_si128(_doFilter_lo, _R0_lo)), _mm_or_si128(_mm_and_si128(_doFilter_hi, _r0_hi), _mm_andnot_si128(_doFilter_hi, _R0_hi)));
            X[5] = _mm_packus_epi16(_mm_or_si128(_mm_and_si128(_doFilter_lo, _r1_lo), _mm_andnot_si128(_doFilter_lo, _R1_lo)), _mm_or_si128(_mm_and_si128(_doFilter_hi, _r1_hi), _mm_andnot_si128(_doFilter_hi, _R1_hi)));
            X[6] = _mm_packus_epi16(_mm_or_si128(_mm_and_si128(_doFilter_lo, _r2_lo), _mm_andnot_si128(_doFilter_lo, _R2_lo)), _mm_or_si128(_mm_and_si128(_doFilter_hi, _r2_hi), _mm_andnot_si128(_doFilter_hi, _R2_hi)));
          }

          /* ===================================== */
          /*   HORIZONTAL LUMA STANDARD FILTER */
          /* ===================================== */
          else
          {
            uint16_t do_filter_curr = 0xf & (do_filter >> (edge * 4));

            if (do_filter_curr)
            {
              TAA_H264_ALIGN(64) int16_t do_curr[4] = { -((do_filter_curr >> 0) & 1),
                                                        -((do_filter_curr >> 1) & 1),
                                                        -((do_filter_curr >> 2) & 1),
                                                        -((do_filter_curr >> 3) & 1)};

              const int a = tc0_tab[(offset + ((horz_strength2>>(4*edge + 0))&0x1) * 2 + ((horz_strength1>>(4*edge + 0))&0x1))];
              const int b = tc0_tab[(offset + ((horz_strength2>>(4*edge + 1))&0x1) * 2 + ((horz_strength1>>(4*edge + 1))&0x1))];
              const int c = tc0_tab[(offset + ((horz_strength2>>(4*edge + 2))&0x1) * 2 + ((horz_strength1>>(4*edge + 2))&0x1))];
              const int d = tc0_tab[(offset + ((horz_strength2>>(4*edge + 3))&0x1) * 2 + ((horz_strength1>>(4*edge + 3))&0x1))];

              {
                __m128i O = _mm_setzero_si128();
                __m128i _const_4 = _mm_set1_epi16(0x0004);
                __m128i _C0_lo = _mm_set_epi32(b, b, a, a);
                __m128i _C0_hi = _mm_set_epi32(d, d, c, c);
                __m128i _dofilter_lo = _mm_set_epi16(do_curr[1], do_curr[1], do_curr[1], do_curr[1],
                                                     do_curr[0], do_curr[0], do_curr[0], do_curr[0]);
                __m128i _dofilter_hi = _mm_set_epi16(do_curr[3], do_curr[3], do_curr[3], do_curr[3],
                                                     do_curr[2], do_curr[2], do_curr[2], do_curr[2]);
                __m128i _L2_lo, _L1_lo, _L0_lo, _R0_lo, _R1_lo, _R2_lo, _dif_lo, _delta0_lo, _delta1_lo, _RL0_lo, _Delta_lo, _aq_lo, _ap_lo, _t0_lo, _t1_lo, _t2_lo, _c0_lo, _nc0_lo, _filter_lo;
                __m128i _L2_hi, _L1_hi, _L0_hi, _R0_hi, _R1_hi, _R2_hi, _dif_hi, _delta0_hi, _delta1_hi, _RL0_hi, _Delta_hi, _aq_hi, _ap_hi, _t0_hi, _t1_hi, _t2_hi, _c0_hi, _nc0_hi, _filter_hi;

                _L2_lo = _mm_unpacklo_epi8(X[4 * edge + 1], O);
                _L1_lo = _mm_unpacklo_epi8(X[4 * edge + 2], O);
                _L0_lo = _mm_unpacklo_epi8(X[4 * edge + 3], O);
                _R0_lo = _mm_unpacklo_epi8(X[4 * edge + 4], O);
                _R1_lo = _mm_unpacklo_epi8(X[4 * edge + 5], O);
                _R2_lo = _mm_unpacklo_epi8(X[4 * edge + 6], O);

                _ap_lo  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_L0_lo, _L2_lo)), _Beta );
                _aq_lo  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_lo, _R2_lo)), _Beta );
                _t1_lo  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_lo, _R1_lo)), _Beta );
                _t2_lo  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_L0_lo, _L1_lo)), _Beta );
                _t0_lo  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_lo, _L0_lo)), _Alpha);

                _RL0_lo = _mm_avg_epu16(_L0_lo, _R0_lo);
                _Delta_lo = _mm_sub_epi16(_R0_lo, _L0_lo);

                _c0_lo  = _mm_sub_epi16(_mm_sub_epi16(_C0_lo, _ap_lo), _aq_lo);
                _nc0_lo = _mm_sub_epi16(O, _c0_lo);
                _dif_lo = _mm_sub_epi16(_L1_lo, _R1_lo);
                _dif_lo = _mm_add_epi16(_dif_lo, _mm_slli_epi16(_Delta_lo, 2));
                _dif_lo = _mm_add_epi16(_dif_lo, _const_4);
                _dif_lo = _mm_srai_epi16(_dif_lo, 3);
                _dif_lo = _mm_min_epi16(_c0_lo, _mm_max_epi16(_nc0_lo, _dif_lo));

                _filter_lo = _mm_and_si128(_mm_and_si128(_mm_and_si128(_t0_lo, _t1_lo), _t2_lo), _dofilter_lo);
                _delta0_lo = _mm_add_epi16(_L2_lo, _RL0_lo);
                _delta1_lo = _mm_add_epi16(_R2_lo, _RL0_lo);

                _delta0_lo = _mm_srai_epi16(_mm_sub_epi16(_delta0_lo, _mm_add_epi16(_L1_lo, _L1_lo)), 1);
                _delta1_lo = _mm_srai_epi16(_mm_sub_epi16(_delta1_lo, _mm_add_epi16(_R1_lo, _R1_lo)), 1);
                _delta0_lo = _mm_min_epi16(_C0_lo, _mm_max_epi16(_mm_sub_epi16(O, _C0_lo), _delta0_lo));
                _delta1_lo = _mm_min_epi16(_C0_lo, _mm_max_epi16(_mm_sub_epi16(O, _C0_lo), _delta1_lo));

                _dif_lo    = _mm_and_si128(_dif_lo, _filter_lo);
                _delta0_lo = _mm_and_si128(_delta0_lo, _filter_lo);
                _delta0_lo = _mm_and_si128(_delta0_lo, _ap_lo);
                _delta1_lo = _mm_and_si128(_delta1_lo, _filter_lo);
                _delta1_lo = _mm_and_si128(_delta1_lo, _aq_lo);

                _L2_hi = _mm_unpackhi_epi8(X[4 * edge + 1], O);
                _L1_hi = _mm_unpackhi_epi8(X[4 * edge + 2], O);
                _L0_hi = _mm_unpackhi_epi8(X[4 * edge + 3], O);
                _R0_hi = _mm_unpackhi_epi8(X[4 * edge + 4], O);
                _R1_hi = _mm_unpackhi_epi8(X[4 * edge + 5], O);
                _R2_hi = _mm_unpackhi_epi8(X[4 * edge + 6], O);

                _ap_hi  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_L0_hi, _L2_hi)), _Beta );
                _aq_hi  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_hi, _R2_hi)), _Beta );
                _t1_hi  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_hi, _R1_hi)), _Beta );
                _t2_hi  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_L0_hi, _L1_hi)), _Beta );
                _t0_hi  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_hi, _L0_hi)), _Alpha);

                _RL0_hi = _mm_avg_epu16(_L0_hi, _R0_hi);
                _Delta_hi = _mm_sub_epi16(_R0_hi, _L0_hi);

                _c0_hi  = _mm_sub_epi16(_mm_sub_epi16(_C0_hi, _ap_hi), _aq_hi);
                _nc0_hi = _mm_sub_epi16(O, _c0_hi);
                _dif_hi = _mm_sub_epi16(_L1_hi, _R1_hi);
                _dif_hi = _mm_add_epi16(_dif_hi, _mm_slli_epi16(_Delta_hi, 2));
                _dif_hi = _mm_add_epi16(_dif_hi, _const_4);
                _dif_hi = _mm_srai_epi16(_dif_hi, 3);
                _dif_hi = _mm_min_epi16(_c0_hi, _mm_max_epi16(_nc0_hi, _dif_hi));

                _filter_hi = _mm_and_si128(_mm_and_si128(_mm_and_si128(_t0_hi, _t1_hi), _t2_hi), _dofilter_hi);
                _delta0_hi = _mm_add_epi16(_L2_hi, _RL0_hi);
                _delta1_hi = _mm_add_epi16(_R2_hi, _RL0_hi);

                _delta0_hi = _mm_srai_epi16(_mm_sub_epi16(_delta0_hi, _mm_add_epi16(_L1_hi, _L1_hi)), 1);
                _delta1_hi = _mm_srai_epi16(_mm_sub_epi16(_delta1_hi, _mm_add_epi16(_R1_hi, _R1_hi)), 1);
                _delta0_hi = _mm_min_epi16(_C0_hi, _mm_max_epi16(_mm_sub_epi16(O, _C0_hi), _delta0_hi));
                _delta1_hi = _mm_min_epi16(_C0_hi, _mm_max_epi16(_mm_sub_epi16(O, _C0_hi), _delta1_hi));

                _dif_hi    = _mm_and_si128(_dif_hi, _filter_hi);
                _delta0_hi = _mm_and_si128(_delta0_hi, _filter_hi);
                _delta0_hi = _mm_and_si128(_delta0_hi, _ap_hi);
                _delta1_hi = _mm_and_si128(_delta1_hi, _filter_hi);
                _delta1_hi = _mm_and_si128(_delta1_hi, _aq_hi);

                X[4 * edge + 2] = _mm_packus_epi16(_mm_add_epi16(_L1_lo, _delta0_lo), _mm_add_epi16(_L1_hi, _delta0_hi));
                X[4 * edge + 3] = _mm_packus_epi16(_mm_add_epi16(_L0_lo, _dif_lo),    _mm_add_epi16(_L0_hi, _dif_hi)   );
                X[4 * edge + 4] = _mm_packus_epi16(_mm_sub_epi16(_R0_lo, _dif_lo),    _mm_sub_epi16(_R0_hi, _dif_hi)   );
                X[4 * edge + 5] = _mm_packus_epi16(_mm_add_epi16(_R1_lo, _delta1_lo), _mm_add_epi16(_R1_hi, _delta1_hi));
              }

            }
          }
        }

        if (edge_start_y == 0)
        {
          TAA_H264_ALIGN(64) int patch[16];

          _mm_store_si128((__m128i *) &patch[0], _mm_unpacklo_epi16(_mm_unpacklo_epi8(Y[0], Y[1]), _mm_unpacklo_epi8(Y[2], Y[3])));
          _mm_store_si128((__m128i *) &patch[4], _mm_unpackhi_epi16(_mm_unpacklo_epi8(Y[0], Y[1]), _mm_unpacklo_epi8(Y[2], Y[3])));
          _mm_store_si128((__m128i *) &patch[8], _mm_unpacklo_epi16(_mm_unpackhi_epi8(Y[0], Y[1]), _mm_unpackhi_epi8(Y[2], Y[3])));
          _mm_store_si128((__m128i *) &patch[12], _mm_unpackhi_epi16(_mm_unpackhi_epi8(Y[0], Y[1]), _mm_unpackhi_epi8(Y[2], Y[3])));

          *(int *)(p + STRIDE_Y * 0 - 4)  = patch[0];
          *(int *)(p + STRIDE_Y * 1 - 4)  = patch[1];
          *(int *)(p + STRIDE_Y * 2 - 4)  = patch[2];
          *(int *)(p + STRIDE_Y * 3 - 4)  = patch[3];
          *(int *)(p + STRIDE_Y * 4 - 4)  = patch[4];
          *(int *)(p + STRIDE_Y * 5 - 4)  = patch[5];
          *(int *)(p + STRIDE_Y * 6 - 4)  = patch[6];
          *(int *)(p + STRIDE_Y * 7 - 4)  = patch[7];
          *(int *)(p + STRIDE_Y * 8 - 4)  = patch[8];
          *(int *)(p + STRIDE_Y * 9 - 4)  = patch[9];
          *(int *)(p + STRIDE_Y * 10 - 4) = patch[10];
          *(int *)(p + STRIDE_Y * 11 - 4) = patch[11];
          *(int *)(p + STRIDE_Y * 12 - 4) = patch[12];
          *(int *)(p + STRIDE_Y * 13 - 4) = patch[13];
          *(int *)(p + STRIDE_Y * 14 - 4) = patch[14];
          *(int *)(p + STRIDE_Y * 15 - 4) = patch[15];
        }
        _mm_store_si128((__m128i *)(p - STRIDE_Y * 4), X[ 0]);
        _mm_store_si128((__m128i *)(p - STRIDE_Y * 3), X[ 1]);
        _mm_store_si128((__m128i *)(p - STRIDE_Y * 2), X[ 2]);
        _mm_store_si128((__m128i *)(p - STRIDE_Y * 1), X[ 3]);
        _mm_store_si128((__m128i *)(p + STRIDE_Y *  0), X[ 4]);
        _mm_store_si128((__m128i *)(p + STRIDE_Y *  1), X[ 5]);
        _mm_store_si128((__m128i *)(p + STRIDE_Y *  2), X[ 6]);
        _mm_store_si128((__m128i *)(p + STRIDE_Y *  3), X[ 7]);
        _mm_store_si128((__m128i *)(p + STRIDE_Y *  4), X[ 8]);
        _mm_store_si128((__m128i *)(p + STRIDE_Y *  5), X[ 9]);
        _mm_store_si128((__m128i *)(p + STRIDE_Y *  6), X[10]);
        _mm_store_si128((__m128i *)(p + STRIDE_Y *  7), X[11]);
        _mm_store_si128((__m128i *)(p + STRIDE_Y *  8), X[12]);
        _mm_store_si128((__m128i *)(p + STRIDE_Y *  9), X[13]);
        _mm_store_si128((__m128i *)(p + STRIDE_Y * 10), X[14]);
        _mm_store_si128((__m128i *)(p + STRIDE_Y * 11), X[15]);
        _mm_store_si128((__m128i *)(p + STRIDE_Y * 12), X[16]);
        _mm_store_si128((__m128i *)(p + STRIDE_Y * 13), X[17]);
        _mm_store_si128((__m128i *)(p + STRIDE_Y * 14), X[18]);
        _mm_store_si128((__m128i *)(p + STRIDE_Y * 15), X[19]);
      }
      {
        __m128i x4, x8, x12, x16, x20, x24, x28, x32, y20, y24, y28, y32, z23, z31, z24, z32;
        x4  = _mm_unpackhi_epi8(X[ 4], X[ 5]);   x20 = _mm_unpackhi_epi8(X[12], X[13]);
        x8  = _mm_unpackhi_epi8(X[ 6], X[ 7]);   x24 = _mm_unpackhi_epi8(X[14], X[15]);
        x12 = _mm_unpackhi_epi8(X[ 8], X[ 9]);   x28 = _mm_unpackhi_epi8(X[16], X[17]);
        x16 = _mm_unpackhi_epi8(X[10], X[11]);   x32 = _mm_unpackhi_epi8(X[18], X[19]);

        y20 = _mm_unpackhi_epi16( x4, x8);       y28 = _mm_unpackhi_epi16(x20, x24);
        y24 = _mm_unpackhi_epi16(x12, x16);      y32 = _mm_unpackhi_epi16(x28, x32);
        z23 = _mm_unpacklo_epi32(y20, y24);      z24 = _mm_unpackhi_epi32(y20, y24);
        z31 = _mm_unpacklo_epi32(y28, y32);      z32 = _mm_unpackhi_epi32(y28, y32);

        Y[0] = _mm_unpacklo_epi64(z23, z31);    Y[1] = _mm_unpackhi_epi64(z23, z31);
        Y[2] = _mm_unpacklo_epi64(z24, z32);    Y[3] = _mm_unpackhi_epi64(z24, z32);
      }
      mb_deblock++;
    }
  }


  /* ============================================ */
  /* extend left right */
  /* ============================================ */
  TAA_H264_DEBUG_ASSERT (PAD_HORZ_Y == 32);
  for ( mby = 0; mby < NUM_MBS_Y; mby++ )
  {
    int i;
    uint8_t  *  src_left  = src_ext + 16 * STRIDE_Y * mby - PAD_HORZ_Y;
    uint8_t  *  src_right = src_left + STRIDE_Y - PAD_HORZ_Y;
    for ( i = 0; i < 16; i++ )
    {
      _mm_store_si128((__m128i *) &src_left[i * STRIDE_Y], _mm_set1_epi8(src_left[i * STRIDE_Y + PAD_HORZ_Y]));
      _mm_store_si128((__m128i *) &src_left[i * STRIDE_Y + PAD_HORZ_Y / 2], _mm_set1_epi8(src_left[i * STRIDE_Y + PAD_HORZ_Y]));
      _mm_store_si128((__m128i *) &src_right[i * STRIDE_Y], _mm_set1_epi8(src_right[i * STRIDE_Y - 1]));
      _mm_store_si128((__m128i *) &src_right[i * STRIDE_Y + PAD_HORZ_Y / 2], _mm_set1_epi8(src_right[i * STRIDE_Y - 1]));
    }
  }

  /* ============================================ */
  /* extend top bottom */
  /* ============================================ */
  {
    int i, j;
    uint8_t *  src_top = src_ext - PAD_HORZ_Y;
    uint8_t *  dst_top = src_ext - STRIDE_Y * PAD_VERT_Y - PAD_HORZ_Y;
    for(i = 0; i < PAD_VERT_Y; i++)
    {
            #pragma vector aligned
      for(j = 0; j < STRIDE_Y; j++)
      {
        dst_top[i * STRIDE_Y + j] = src_top[j];
      }
    }
  }
  src_ext  += 16 * STRIDE_Y * NUM_MBS_Y;
  {
    int i, j;
    uint8_t *  src_bottom = src_ext - STRIDE_Y - PAD_HORZ_Y;
    uint8_t *  dst_bottom = src_ext - PAD_HORZ_Y;
    for(i = 0; i < PAD_VERT_Y; i++)
    {
            #pragma vector aligned
      for(j = 0; j < STRIDE_Y; j++)
      {
        dst_bottom[i * STRIDE_Y + j] = src_bottom[j];
      }
    }
  }
}

static void TAA_H264_DEBLOCKING_FILTER_CHROMA (
  mb_deblock_t  *           deblock_frame,
  uint8_t *  src_uv_ext
#if !defined(NUM_MBS_X) && !defined(NUM_MBS_Y)
  ,
  const int                         NUM_MBS_X
  ,
  const int                         NUM_MBS_Y
#endif
  )
{
  __m128i X[10], Y[10];
  int mbx, mby;
  for ( mby = 0; mby < NUM_MBS_Y; mby++ )
  {
    mb_deblock_t * mb_deblock = deblock_frame + mby * NUM_MBS_X;
    for (mbx = 0; mbx < NUM_MBS_X; mbx++)
    {
      /********************************************************/
      /*            Load two 8x8 Chroma Macroblocks           */
      /********************************************************/
      uint8_t *  pU = src_uv_ext + mbx * 8 + 8 * STRIDE_UV * mby;
      uint8_t *  pV = src_uv_ext + mbx * 8 + 8 * STRIDE_UV * mby + (NUM_MBS_X + 4) * (NUM_MBS_Y + 4) * 8 * 8;

      X[0] = _mm_unpacklo_epi64(_mm_loadl_epi64((__m128i *)(pV - STRIDE_UV * 2)), _mm_loadl_epi64(((__m128i *)(pU - STRIDE_UV * 2))));
      X[1] = _mm_unpacklo_epi64(_mm_loadl_epi64((__m128i *)(pV - STRIDE_UV * 1)), _mm_loadl_epi64(((__m128i *)(pU - STRIDE_UV * 1))));
      X[2] = _mm_unpacklo_epi64(_mm_loadl_epi64((__m128i *)(pV + STRIDE_UV * 0)), _mm_loadl_epi64(((__m128i *)(pU + STRIDE_UV * 0))));
      X[3] = _mm_unpacklo_epi64(_mm_loadl_epi64((__m128i *)(pV + STRIDE_UV * 1)), _mm_loadl_epi64(((__m128i *)(pU + STRIDE_UV * 1))));
      X[4] = _mm_unpacklo_epi64(_mm_loadl_epi64((__m128i *)(pV + STRIDE_UV * 2)), _mm_loadl_epi64(((__m128i *)(pU + STRIDE_UV * 2))));
      X[5] = _mm_unpacklo_epi64(_mm_loadl_epi64((__m128i *)(pV + STRIDE_UV * 3)), _mm_loadl_epi64(((__m128i *)(pU + STRIDE_UV * 3))));
      X[6] = _mm_unpacklo_epi64(_mm_loadl_epi64((__m128i *)(pV + STRIDE_UV * 4)), _mm_loadl_epi64(((__m128i *)(pU + STRIDE_UV * 4))));
      X[7] = _mm_unpacklo_epi64(_mm_loadl_epi64((__m128i *)(pV + STRIDE_UV * 5)), _mm_loadl_epi64(((__m128i *)(pU + STRIDE_UV * 5))));
      X[8] = _mm_unpacklo_epi64(_mm_loadl_epi64((__m128i *)(pV + STRIDE_UV * 6)), _mm_loadl_epi64(((__m128i *)(pU + STRIDE_UV * 6))));
      X[9] = _mm_unpacklo_epi64(_mm_loadl_epi64((__m128i *)(pV + STRIDE_UV * 7)), _mm_loadl_epi64(((__m128i *)(pU + STRIDE_UV * 7))));

      if (mb_deblock->do_filter)
      {
        int edge, edge_start_x, edge_start_y, edge_end_x, edge_end_y;
        int do_filter = mb_deblock->do_filter;
        const int vert_strength1 = mb_deblock->vert_strength1;
        const int vert_strength2 = mb_deblock->vert_strength2;
        const int horz_strength1 = mb_deblock->horz_strength1;
        const int horz_strength2 = mb_deblock->horz_strength2;

        /********************************************************/
        /*            2x8x8 matrix transpose X -> Y             */
        /********************************************************/
        TAA_H264_2X8X8_TRANSPOSE(&X[2], &Y[2]);

        do_filter = mb_deblock->do_filter;

        /* vert edge */
        edge_start_y = (!mbx || !(do_filter & 0x1111)) << 1;
        edge_end_y   = (do_filter & 0x4444) ? 4 : 1;

        for (edge = edge_start_y; edge < edge_end_y; edge += 2)
        {
          const int curr_quant = (edge) ? mb_deblock->curr_qpc : mb_deblock->left_qpc;
          const int offset = curr_quant * 3 + 3 * (12 + mb_deblock->filter_offset_a);
          __m128i _Alpha = _mm_set1_epi16(alpha_tab[12 + mb_deblock->filter_offset_a + curr_quant]);
          __m128i _Beta = _mm_set1_epi16(beta_tab[12 + mb_deblock->filter_offset_b + curr_quant]);

          /* ===================================== */
          /*    VERTICAL CHROMA STRONG FILTER */
          /* ===================================== */
          if (edge == 0 && (vert_strength2 & 0x1))           /* strong */
          {
            __m128i O = _mm_setzero_si128();
            __m128i _l0_U, _r0_U, _L1_U, _L0_U, _R0_U, _R1_U, _t0_U, _t1_U, _t2_U, _filter_U;
            __m128i _l0_V, _r0_V, _L1_V, _L0_V, _R0_V, _R1_V, _t0_V, _t1_V, _t2_V, _filter_V;

            _L1_V = _mm_unpacklo_epi8(Y[0], O);
            _L0_V = _mm_unpacklo_epi8(Y[1], O);
            _R0_V = _mm_unpacklo_epi8(Y[2], O);
            _R1_V = _mm_unpacklo_epi8(Y[3], O);
            _L1_U = _mm_unpackhi_epi8(Y[0], O);
            _L0_U = _mm_unpackhi_epi8(Y[1], O);
            _R0_U = _mm_unpackhi_epi8(Y[2], O);
            _R1_U = _mm_unpackhi_epi8(Y[3], O);

            _t1_V  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_V, _R1_V)), _Beta );
            _t2_V  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_L0_V, _L1_V)), _Beta );
            _t0_V  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_V, _L0_V)), _Alpha);
            _t1_U  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_U, _R1_U)), _Beta );
            _t2_U  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_L0_U, _L1_U)), _Beta );
            _t0_U  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_U, _L0_U)), _Alpha);

            _filter_V = _mm_and_si128(_mm_and_si128(_t0_V, _t1_V), _t2_V);
            _filter_U = _mm_and_si128(_mm_and_si128(_t0_U, _t1_U), _t2_U);

            _l0_V = _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_L1_V, _L1_V), _L0_V), _R1_V), _mm_set1_epi16(2)), 2);
            _r0_V = _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_R1_V, _R1_V), _R0_V), _L1_V), _mm_set1_epi16(2)), 2);
            _l0_U = _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_L1_U, _L1_U), _L0_U), _R1_U), _mm_set1_epi16(2)), 2);
            _r0_U = _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_R1_U, _R1_U), _R0_U), _L1_U), _mm_set1_epi16(2)), 2);

            Y[1] = _mm_packus_epi16(_mm_or_si128(_mm_and_si128(_filter_V, _l0_V), _mm_andnot_si128(_filter_V, _L0_V)), _mm_or_si128(_mm_and_si128(_filter_U, _l0_U), _mm_andnot_si128(_filter_U, _L0_U)));
            Y[2] = _mm_packus_epi16(_mm_or_si128(_mm_and_si128(_filter_V, _r0_V), _mm_andnot_si128(_filter_V, _R0_V)), _mm_or_si128(_mm_and_si128(_filter_U, _r0_U), _mm_andnot_si128(_filter_U, _R0_U)));
          }

          /* ===================================== */
          /*   VERTICAL CHROMA STANDARD FILTER */
          /* ===================================== */
          else
          {
            uint16_t do_filter_curr = 0x1111 & (do_filter >> (edge));

            if (do_filter_curr)
            {
              TAA_H264_ALIGN(64) int16_t do_curr[4] = { -((do_filter_curr >> 0) & 1),
                                                        -((do_filter_curr >> 4) & 1),
                                                        -((do_filter_curr >> 8) & 1),
                                                        -((do_filter_curr >> 12) & 1)};

              const int a = tc0_tab[(offset + ((vert_strength2>>(edge +  0))&0x1) * 2 + ((vert_strength1>>(edge +  0))&0x1))];
              const int b = tc0_tab[(offset + ((vert_strength2>>(edge +  4))&0x1) * 2 + ((vert_strength1>>(edge +  4))&0x1))];
              const int c = tc0_tab[(offset + ((vert_strength2>>(edge +  8))&0x1) * 2 + ((vert_strength1>>(edge +  8))&0x1))];
              const int d = tc0_tab[(offset + ((vert_strength2>>(edge + 12))&0x1) * 2 + ((vert_strength1>>(edge + 12))&0x1))];

              {
                __m128i O = _mm_setzero_si128();
                __m128i _const_4 = _mm_set1_epi16(0x0004);
                __m128i _C0 =  _mm_sub_epi16(_mm_set_epi32(d, c, b, a), _mm_cmpeq_epi16(O, O));
                __m128i _doFilter = _mm_set_epi16(do_curr[3], do_curr[3], do_curr[2], do_curr[2],
                                                  do_curr[1], do_curr[1], do_curr[0], do_curr[0]);
                __m128i _L1_V, _L0_V, _R0_V, _R1_V, _Delta_V, _t0_V, _t1_V, _t2_V, _dif_V, _filter_V;
                __m128i _L1_U, _L0_U, _R0_U, _R1_U, _Delta_U, _t0_U, _t1_U, _t2_U, _dif_U, _filter_U;

                _L1_V = _mm_unpacklo_epi8(Y[2 * edge + 0], O);
                _L0_V = _mm_unpacklo_epi8(Y[2 * edge + 1], O);
                _R0_V = _mm_unpacklo_epi8(Y[2 * edge + 2], O);
                _R1_V = _mm_unpacklo_epi8(Y[2 * edge + 3], O);
                _L1_U = _mm_unpackhi_epi8(Y[2 * edge + 0], O);
                _L0_U = _mm_unpackhi_epi8(Y[2 * edge + 1], O);
                _R0_U = _mm_unpackhi_epi8(Y[2 * edge + 2], O);
                _R1_U = _mm_unpackhi_epi8(Y[2 * edge + 3], O);

                _t1_V  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_V, _R1_V)), _Beta );
                _t2_V  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_L0_V, _L1_V)), _Beta );
                _t0_V  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_V, _L0_V)), _Alpha);
                _t1_U  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_U, _R1_U)), _Beta );
                _t2_U  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_L0_U, _L1_U)), _Beta );
                _t0_U  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_U, _L0_U)), _Alpha);

                _filter_V = _mm_and_si128(_mm_and_si128(_mm_and_si128(_t0_V, _t1_V), _t2_V), _doFilter);
                _filter_U = _mm_and_si128(_mm_and_si128(_mm_and_si128(_t0_U, _t1_U), _t2_U), _doFilter);
                _Delta_V = _mm_sub_epi16(_R0_V, _L0_V);
                _Delta_U = _mm_sub_epi16(_R0_U, _L0_U);

                _dif_V = _mm_sub_epi16(_L1_V, _R1_V);
                _dif_V = _mm_add_epi16(_dif_V, _mm_slli_epi16(_Delta_V, 2));
                _dif_V = _mm_add_epi16(_dif_V, _const_4);
                _dif_V = _mm_srai_epi16(_dif_V, 3);
                _dif_V = _mm_min_epi16(_C0, _mm_max_epi16(_mm_sub_epi16(O, _C0), _dif_V));
                _dif_V = _mm_and_si128(_dif_V, _filter_V);
                _dif_U = _mm_sub_epi16(_L1_U, _R1_U);
                _dif_U = _mm_add_epi16(_dif_U, _mm_slli_epi16(_Delta_U, 2));
                _dif_U = _mm_add_epi16(_dif_U, _const_4);
                _dif_U = _mm_srai_epi16(_dif_U, 3);
                _dif_U = _mm_min_epi16(_C0, _mm_max_epi16(_mm_sub_epi16(O, _C0), _dif_U));
                _dif_U = _mm_and_si128(_dif_U, _filter_U);

                Y[2 * edge + 1] = _mm_packus_epi16(_mm_add_epi16(_L0_V, _dif_V), _mm_add_epi16(_L0_U, _dif_U));
                Y[2 * edge + 2] = _mm_packus_epi16(_mm_sub_epi16(_R0_V, _dif_V), _mm_sub_epi16(_R0_U, _dif_U));
              }

            }
          }
        }
        /********************************************************/
        /*            2x8x8 matrix transpose Y -> X             */
        /********************************************************/
        TAA_H264_2X8X8_TRANSPOSE(&Y[2], &X[2]);

        /* horz edge */
        do_filter >>= 16;

        edge_start_x = (!mby || !(do_filter & 0xf)) << 1;
        edge_end_x   = (do_filter & 0x0f00) ? 4 : 1;

        for (edge = edge_start_x; edge < edge_end_x; edge += 2)
        {
          const int curr_quant = (edge) ? mb_deblock->curr_qpc : mb_deblock->top_qpc;
          const int offset = curr_quant * 3 + 3 * (12 + mb_deblock->filter_offset_a);
          __m128i _Alpha = _mm_set1_epi16(alpha_tab[12 + mb_deblock->filter_offset_a + curr_quant]);
          __m128i _Beta = _mm_set1_epi16(beta_tab[12 + mb_deblock->filter_offset_b + curr_quant]);

          /* ===================================== */
          /*   HORIZONTAL CHROMA STRONG FILTER */
          /* ===================================== */
          if (edge == 0 && (horz_strength2 & 0x1))           /* strong */
          {
            __m128i O = _mm_setzero_si128();
            __m128i _l0_U, _r0_U, _L1_U, _L0_U, _R0_U, _R1_U, _t0_U, _t1_U, _t2_U, _filter_U;
            __m128i _l0_V, _r0_V, _L1_V, _L0_V, _R0_V, _R1_V, _t0_V, _t1_V, _t2_V, _filter_V;

            _L1_V = _mm_unpacklo_epi8(X[0], O);
            _L0_V = _mm_unpacklo_epi8(X[1], O);
            _R0_V = _mm_unpacklo_epi8(X[2], O);
            _R1_V = _mm_unpacklo_epi8(X[3], O);
            _L1_U = _mm_unpackhi_epi8(X[0], O);
            _L0_U = _mm_unpackhi_epi8(X[1], O);
            _R0_U = _mm_unpackhi_epi8(X[2], O);
            _R1_U = _mm_unpackhi_epi8(X[3], O);

            _t1_V  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_V, _R1_V)), _Beta );
            _t2_V  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_L0_V, _L1_V)), _Beta );
            _t0_V  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_V, _L0_V)), _Alpha);
            _t1_U  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_U, _R1_U)), _Beta );
            _t2_U  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_L0_U, _L1_U)), _Beta );
            _t0_U  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_U, _L0_U)), _Alpha);

            _filter_V = _mm_and_si128(_mm_and_si128(_t0_V, _t1_V), _t2_V);
            _filter_U = _mm_and_si128(_mm_and_si128(_t0_U, _t1_U), _t2_U);

            _l0_V = _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_L1_V, _L1_V), _L0_V), _R1_V), _mm_set1_epi16(2)), 2);
            _r0_V = _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_R1_V, _R1_V), _R0_V), _L1_V), _mm_set1_epi16(2)), 2);
            _l0_U = _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_L1_U, _L1_U), _L0_U), _R1_U), _mm_set1_epi16(2)), 2);
            _r0_U = _mm_srai_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_mm_add_epi16(_R1_U, _R1_U), _R0_U), _L1_U), _mm_set1_epi16(2)), 2);

            X[1] = _mm_packus_epi16(_mm_or_si128(_mm_and_si128(_filter_V, _l0_V), _mm_andnot_si128(_filter_V, _L0_V)), _mm_or_si128(_mm_and_si128(_filter_U, _l0_U), _mm_andnot_si128(_filter_U, _L0_U)));
            X[2] = _mm_packus_epi16(_mm_or_si128(_mm_and_si128(_filter_V, _r0_V), _mm_andnot_si128(_filter_V, _R0_V)), _mm_or_si128(_mm_and_si128(_filter_U, _r0_U), _mm_andnot_si128(_filter_U, _R0_U)));
          }

          /* ===================================== */
          /*  HORIZONTAL CHROMA STANDARD FILTER */
          /* ===================================== */
          else
          {
            uint16_t do_filter_curr = 0xf & (do_filter >> (edge * 4));

            if (do_filter_curr)
            {
              TAA_H264_ALIGN(64) int16_t do_curr[4] = { -((do_filter_curr >> 0) & 1),
                                                        -((do_filter_curr >> 1) & 1),
                                                        -((do_filter_curr >> 2) & 1),
                                                        -((do_filter_curr >> 3) & 1)};

              const int a = tc0_tab[(offset + ((horz_strength2>>(4*edge + 0))&0x1) * 2 + ((horz_strength1>>(4*edge + 0))&0x1))];
              const int b = tc0_tab[(offset + ((horz_strength2>>(4*edge + 1))&0x1) * 2 + ((horz_strength1>>(4*edge + 1))&0x1))];
              const int c = tc0_tab[(offset + ((horz_strength2>>(4*edge + 2))&0x1) * 2 + ((horz_strength1>>(4*edge + 2))&0x1))];
              const int d = tc0_tab[(offset + ((horz_strength2>>(4*edge + 3))&0x1) * 2 + ((horz_strength1>>(4*edge + 3))&0x1))];

              {
                __m128i O = _mm_setzero_si128();
                __m128i _const_4 = _mm_set1_epi16(0x0004);
                __m128i _C0 =  _mm_sub_epi16(_mm_set_epi32(d, c, b, a), _mm_cmpeq_epi16(O, O));
                __m128i _doFilter = _mm_set_epi16(do_curr[3], do_curr[3], do_curr[2], do_curr[2],
                                                  do_curr[1], do_curr[1], do_curr[0], do_curr[0]);
                __m128i _L1_V, _L0_V, _R0_V, _R1_V, _Delta_V, _t0_V, _t1_V, _t2_V, _dif_V, _filter_V;
                __m128i _L1_U, _L0_U, _R0_U, _R1_U, _Delta_U, _t0_U, _t1_U, _t2_U, _dif_U, _filter_U;

                _L1_V = _mm_unpacklo_epi8(X[2 * edge + 0], O);
                _L0_V = _mm_unpacklo_epi8(X[2 * edge + 1], O);
                _R0_V = _mm_unpacklo_epi8(X[2 * edge + 2], O);
                _R1_V = _mm_unpacklo_epi8(X[2 * edge + 3], O);
                _L1_U = _mm_unpackhi_epi8(X[2 * edge + 0], O);
                _L0_U = _mm_unpackhi_epi8(X[2 * edge + 1], O);
                _R0_U = _mm_unpackhi_epi8(X[2 * edge + 2], O);
                _R1_U = _mm_unpackhi_epi8(X[2 * edge + 3], O);

                _t1_V  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_V, _R1_V)), _Beta );
                _t2_V  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_L0_V, _L1_V)), _Beta );
                _t0_V  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_V, _L0_V)), _Alpha);
                _t1_U  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_U, _R1_U)), _Beta );
                _t2_U  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_L0_U, _L1_U)), _Beta );
                _t0_U  = _mm_cmplt_epi16( _mm_abs_epi16(_mm_sub_epi16(_R0_U, _L0_U)), _Alpha);

                _filter_V = _mm_and_si128(_mm_and_si128(_mm_and_si128(_t0_V, _t1_V), _t2_V), _doFilter);
                _filter_U = _mm_and_si128(_mm_and_si128(_mm_and_si128(_t0_U, _t1_U), _t2_U), _doFilter);
                _Delta_V = _mm_sub_epi16(_R0_V, _L0_V);
                _Delta_U = _mm_sub_epi16(_R0_U, _L0_U);

                _dif_V = _mm_sub_epi16(_L1_V, _R1_V);
                _dif_V = _mm_add_epi16(_dif_V, _mm_slli_epi16(_Delta_V, 2));
                _dif_V = _mm_add_epi16(_dif_V, _const_4);
                _dif_V = _mm_srai_epi16(_dif_V, 3);
                _dif_V = _mm_min_epi16(_C0, _mm_max_epi16(_mm_sub_epi16(O, _C0), _dif_V));
                _dif_V = _mm_and_si128(_dif_V, _filter_V);
                _dif_U = _mm_sub_epi16(_L1_U, _R1_U);
                _dif_U = _mm_add_epi16(_dif_U, _mm_slli_epi16(_Delta_U, 2));
                _dif_U = _mm_add_epi16(_dif_U, _const_4);
                _dif_U = _mm_srai_epi16(_dif_U, 3);
                _dif_U = _mm_min_epi16(_C0, _mm_max_epi16(_mm_sub_epi16(O, _C0), _dif_U));
                _dif_U = _mm_and_si128(_dif_U, _filter_U);

                X[2 * edge + 1] = _mm_packus_epi16(_mm_add_epi16(_L0_V, _dif_V), _mm_add_epi16(_L0_U, _dif_U));
                X[2 * edge + 2] = _mm_packus_epi16(_mm_sub_epi16(_R0_V, _dif_V), _mm_sub_epi16(_R0_U, _dif_U));
              }

            }
          }
        }
        if (edge_start_y == 0)
        {
          TAA_H264_ALIGN(64) int16_t patch[16];

          _mm_store_si128((__m128i *) &patch[0], _mm_unpacklo_epi8(Y[0], Y[1]));
          _mm_store_si128((__m128i *) &patch[8], _mm_unpackhi_epi8(Y[0], Y[1]));
          *(int16_t *)(pV + STRIDE_UV * 0 - 2)  = patch[0];
          *(int16_t *)(pV + STRIDE_UV * 1 - 2)  = patch[1];
          *(int16_t *)(pV + STRIDE_UV * 2 - 2)  = patch[2];
          *(int16_t *)(pV + STRIDE_UV * 3 - 2)  = patch[3];
          *(int16_t *)(pV + STRIDE_UV * 4 - 2)  = patch[4];
          *(int16_t *)(pV + STRIDE_UV * 5 - 2)  = patch[5];
          *(int16_t *)(pV + STRIDE_UV * 6 - 2)  = patch[6];
          *(int16_t *)(pV + STRIDE_UV * 7 - 2)  = patch[7];
          *(int16_t *)(pU + STRIDE_UV * 0 - 2)  = patch[8];
          *(int16_t *)(pU + STRIDE_UV * 1 - 2)  = patch[9];
          *(int16_t *)(pU + STRIDE_UV * 2 - 2)  = patch[10];
          *(int16_t *)(pU + STRIDE_UV * 3 - 2)  = patch[11];
          *(int16_t *)(pU + STRIDE_UV * 4 - 2)  = patch[12];
          *(int16_t *)(pU + STRIDE_UV * 5 - 2)  = patch[13];
          *(int16_t *)(pU + STRIDE_UV * 6 - 2)  = patch[14];
          *(int16_t *)(pU + STRIDE_UV * 7 - 2)  = patch[15];
        }
        _mm_storel_epi64((__m128i *)(pV - STRIDE_UV * 2), X[0]);
        _mm_storel_epi64((__m128i *)(pV - STRIDE_UV * 1), X[1]);
        _mm_storel_epi64((__m128i *)(pV + STRIDE_UV * 0), X[2]);
        _mm_storel_epi64((__m128i *)(pV + STRIDE_UV * 1), X[3]);
        _mm_storel_epi64((__m128i *)(pV + STRIDE_UV * 2), X[4]);
        _mm_storel_epi64((__m128i *)(pV + STRIDE_UV * 3), X[5]);
        _mm_storel_epi64((__m128i *)(pV + STRIDE_UV * 4), X[6]);
        _mm_storel_epi64((__m128i *)(pV + STRIDE_UV * 5), X[7]);
        _mm_storel_epi64((__m128i *)(pV + STRIDE_UV * 6), X[8]);
        _mm_storel_epi64((__m128i *)(pV + STRIDE_UV * 7), X[9]);

        _mm_storel_epi64((__m128i *)(pU - STRIDE_UV * 2), _mm_srli_si128(X[0], 8));
        _mm_storel_epi64((__m128i *)(pU - STRIDE_UV * 1), _mm_srli_si128(X[1], 8));
        _mm_storel_epi64((__m128i *)(pU + STRIDE_UV * 0), _mm_srli_si128(X[2], 8));
        _mm_storel_epi64((__m128i *)(pU + STRIDE_UV * 1), _mm_srli_si128(X[3], 8));
        _mm_storel_epi64((__m128i *)(pU + STRIDE_UV * 2), _mm_srli_si128(X[4], 8));
        _mm_storel_epi64((__m128i *)(pU + STRIDE_UV * 3), _mm_srli_si128(X[5], 8));
        _mm_storel_epi64((__m128i *)(pU + STRIDE_UV * 4), _mm_srli_si128(X[6], 8));
        _mm_storel_epi64((__m128i *)(pU + STRIDE_UV * 5), _mm_srli_si128(X[7], 8));
        _mm_storel_epi64((__m128i *)(pU + STRIDE_UV * 6), _mm_srli_si128(X[8], 8));
        _mm_storel_epi64((__m128i *)(pU + STRIDE_UV * 7), _mm_srli_si128(X[9], 8));
      }
      {
        __m128i x3, x4, x7, x8, x11, x12, x15, x16, y18, y20, y22, y24, z20, z24;
        x3  = _mm_unpacklo_epi8(X[2], X[3]);   x4 = _mm_unpackhi_epi8(X[2], X[3]);
        x7  = _mm_unpacklo_epi8(X[4], X[5]);   x8 = _mm_unpackhi_epi8(X[4], X[5]);
        x11 = _mm_unpacklo_epi8(X[6], X[7]);  x12 = _mm_unpackhi_epi8(X[6], X[7]);
        x15 = _mm_unpacklo_epi8(X[8], X[9]);  x16 = _mm_unpackhi_epi8(X[8], X[9]);
        y18 = _mm_unpackhi_epi16( x3, x7);
        y20 = _mm_unpackhi_epi16( x4, x8);
        y22 = _mm_unpackhi_epi16(x11, x15);
        y24 = _mm_unpackhi_epi16(x12, x16);
        z20 = _mm_unpackhi_epi32(y18, y22);
        z24 = _mm_unpackhi_epi32(y20, y24);
        Y[0] = _mm_unpacklo_epi64(z20, z24); Y[1] = _mm_unpackhi_epi64(z20, z24);
      }
      mb_deblock++;
    }
  }


  /* ============================================ */
  /* extend left right */
  /* ============================================ */
  TAA_H264_DEBUG_ASSERT (PAD_HORZ_UV == 16);
  {
    if (STRIDE_UV % 16 == 0)
    {
      /* Every row is 16 bytes aligned */
      for ( mby = 0; mby < NUM_MBS_Y; mby++ )
      {
        int i;
        uint8_t  *  src_left_u  = src_uv_ext + 8 * STRIDE_UV * mby - PAD_HORZ_UV;
        uint8_t  *  src_left_v  = src_uv_ext + 8 * STRIDE_UV * mby - PAD_HORZ_UV + STRIDE_UV * (NUM_MBS_Y * MB_HEIGHT_UV + 2 * PAD_VERT_UV);
        uint8_t  *  src_right_u = src_left_u + STRIDE_UV - PAD_HORZ_UV;
        uint8_t  *  src_right_v = src_left_v + STRIDE_UV - PAD_HORZ_UV;

        for ( i = 0; i < MB_HEIGHT_UV; i++ )
        {
          _mm_store_si128((__m128i *) &src_left_u[i * STRIDE_UV], _mm_set1_epi8(src_left_u[i * STRIDE_UV + PAD_HORZ_UV]));
          _mm_store_si128((__m128i *) &src_left_v[i * STRIDE_UV], _mm_set1_epi8(src_left_v[i * STRIDE_UV + PAD_HORZ_UV]));
          _mm_store_si128((__m128i *) &src_right_u[i * STRIDE_UV], _mm_set1_epi8(src_right_u[i * STRIDE_UV - 1]));
          _mm_store_si128((__m128i *) &src_right_v[i * STRIDE_UV], _mm_set1_epi8(src_right_v[i * STRIDE_UV - 1]));
        }
      }
    }
    else
    {
      /* Only 8 byte alignement is guranteed */
      for ( mby = 0; mby < NUM_MBS_Y; mby++ )
      {
        int i;
        uint8_t  *  src_left_u  = src_uv_ext + 8 * STRIDE_UV * mby - PAD_HORZ_UV;
        uint8_t  *  src_left_v  = src_uv_ext + 8 * STRIDE_UV * mby - PAD_HORZ_UV + STRIDE_UV * (NUM_MBS_Y * MB_HEIGHT_UV + 2 * PAD_VERT_UV);
        uint8_t  *  src_right_u = src_left_u + STRIDE_UV - PAD_HORZ_UV;
        uint8_t  *  src_right_v = src_left_v + STRIDE_UV - PAD_HORZ_UV;

        for ( i = 0; i < MB_HEIGHT_UV; i++ )
        {
          memset (&src_left_u[i * STRIDE_UV], src_left_u[i * STRIDE_UV + PAD_HORZ_UV], PAD_HORZ_UV);
          memset (&src_left_v[i * STRIDE_UV], src_left_v[i * STRIDE_UV + PAD_HORZ_UV], PAD_HORZ_UV);
          memset (&src_right_u[i * STRIDE_UV], src_right_u[i * STRIDE_UV - 1], PAD_HORZ_UV);
          memset (&src_right_v[i * STRIDE_UV], src_right_v[i * STRIDE_UV - 1], PAD_HORZ_UV);
        }
      }
    }
  }


  /* ============================================ */
  /* extend top */
  /* ============================================ */
  {
    int i, j;
    uint8_t *  src_u = src_uv_ext - PAD_HORZ_UV;
    uint8_t *  src_v = src_uv_ext - PAD_HORZ_UV + (NUM_MBS_X + 4) * (NUM_MBS_Y + 4) * 8 * 8;
    uint8_t *  dst_u = src_uv_ext - STRIDE_UV * PAD_VERT_UV - PAD_HORZ_UV;
    uint8_t *  dst_v = src_uv_ext - STRIDE_UV * PAD_VERT_UV - PAD_HORZ_UV + (NUM_MBS_X + 4) * (NUM_MBS_Y + 4) * 8 * 8;
    for(i = 0; i < PAD_VERT_UV - 1; i++)
    {
      /* The following loop may write 8 bytes into next stride if (luma) width
       * is not modulo 32, but that's ok since the next 8 bytes belongs to this
       * will be overwritten on next iteration. Last stride handled separately
       * to avoid memory overwriting. */
      /* #pragma vector aligned */
      for(j = 0; j < STRIDE_UV; j += 16)
      {
        _mm_storeu_si128((__m128i *) &dst_u[i * STRIDE_UV + j], _mm_loadu_si128((__m128i *) &src_u[j]));
        _mm_storeu_si128((__m128i *) &dst_v[i * STRIDE_UV + j], _mm_loadu_si128((__m128i *) &src_v[j]));
      }
    }
    /* Last stride, make sure we dont't do memory overwrite outside the frame
     * buffer. */
    for(j = 0; j < STRIDE_UV - 16; j += 16)
    {
      _mm_storeu_si128((__m128i *) &dst_u[(PAD_VERT_UV - 1) * STRIDE_UV + j], _mm_loadu_si128((__m128i *) &src_u[j]));
      _mm_storeu_si128((__m128i *) &dst_v[(PAD_VERT_UV - 1) * STRIDE_UV + j], _mm_loadu_si128((__m128i *) &src_v[j]));
    }
    /* Handle last one/two macroblocks seperately. */
    if (STRIDE_UV % 16 == 0)
    {
      _mm_storeu_si128((__m128i *) &dst_u[(PAD_VERT_UV - 1) * STRIDE_UV + STRIDE_UV - 16], _mm_loadu_si128((__m128i *) &src_u[STRIDE_UV - 16]));
      _mm_storeu_si128((__m128i *) &dst_v[(PAD_VERT_UV - 1) * STRIDE_UV + STRIDE_UV - 16], _mm_loadu_si128((__m128i *) &src_v[STRIDE_UV - 16]));
    }
    else
    {
      /* Stride is modulo 8. Only write 8 bytes */
      memcpy (&dst_u[(PAD_VERT_UV - 1) * STRIDE_UV + STRIDE_UV - 8], &src_u[STRIDE_UV - 8], 8);
      memcpy (&dst_v[(PAD_VERT_UV - 1) * STRIDE_UV + STRIDE_UV - 8], &src_v[STRIDE_UV - 8], 8);
    }
  }

  /* ============================================ */
  /* extend bottom */
  /* ============================================ */

  src_uv_ext += 8 * STRIDE_UV * NUM_MBS_Y;
  {
    int i, j;
    uint8_t *  src_u = src_uv_ext - STRIDE_UV - PAD_HORZ_UV;
    uint8_t *  src_v = src_uv_ext - STRIDE_UV - PAD_HORZ_UV + (NUM_MBS_X + 4) * (NUM_MBS_Y + 4) * 8 * 8;
    uint8_t *  dst_u = src_uv_ext - PAD_HORZ_UV;
    uint8_t *  dst_v = src_uv_ext - PAD_HORZ_UV + (NUM_MBS_X + 4) * (NUM_MBS_Y + 4) * 8 * 8;
    for(i = 0; i < PAD_VERT_UV - 1; i++)
    {
      /* The following loop may write 8 bytes into next stride if (luma) width
       * is not modulo 32, but that's ok since the next 8 bytes belongs to this
       * will be overwritten on next iteration. Last stride handled separately
       * to avoid memory overwriting. */
      /* #pragma vector aligned */
      for(j = 0; j < STRIDE_UV; j += 16)
      {
        _mm_storeu_si128((__m128i *) &dst_u[i * STRIDE_UV + j], _mm_loadu_si128((__m128i *) &src_u[j]));
        _mm_storeu_si128((__m128i *) &dst_v[i * STRIDE_UV + j], _mm_loadu_si128((__m128i *) &src_v[j]));
      }
    }
    /* Last stride, make sure we dont't do memory overwrite outside the frame
     * buffer. */
    for(j = 0; j < STRIDE_UV - 16; j += 16)
    {
      _mm_storeu_si128((__m128i *) &dst_u[(PAD_VERT_UV - 1) * STRIDE_UV + j], _mm_loadu_si128((__m128i *) &src_u[j]));
      _mm_storeu_si128((__m128i *) &dst_v[(PAD_VERT_UV - 1) * STRIDE_UV + j], _mm_loadu_si128((__m128i *) &src_v[j]));
    }
    /* Handle last one/two macroblocks seperately. */
    if (STRIDE_UV % 16 == 0)
    {
      _mm_storeu_si128((__m128i *) &dst_u[(PAD_VERT_UV - 1) * STRIDE_UV + STRIDE_UV - 16], _mm_loadu_si128((__m128i *) &src_u[STRIDE_UV - 16]));
      _mm_storeu_si128((__m128i *) &dst_v[(PAD_VERT_UV - 1) * STRIDE_UV + STRIDE_UV - 16], _mm_loadu_si128((__m128i *) &src_v[STRIDE_UV - 16]));
    }
    else
    {
      /* Stride is modulo 8. Only write 8 bytes */
      memcpy (&dst_u[(PAD_VERT_UV - 1) * STRIDE_UV + STRIDE_UV - 8], &src_u[STRIDE_UV - 8], 8);
      memcpy (&dst_v[(PAD_VERT_UV - 1) * STRIDE_UV + STRIDE_UV - 8], &src_v[STRIDE_UV - 8], 8);
    }
  }
}

#undef STRIDE_Y
#undef STRUDE_UV

#pragma warning( default : 810 )
#pragma warning( default : 981 )
#pragma warning( default : 1418 )
#pragma warning( default : 2259 )
