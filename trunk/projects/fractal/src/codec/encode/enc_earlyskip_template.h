static int taa_h264_check_early_skip (
  const luma_t *   luma,
  const chroma_t * chroma,
  const int        qp)
{
  __m128i const O = _mm_setzero_si128();
  __m128i const M1 = _mm_set_epi16(0x0000, 0xffff, 0x0000, 0xffff, 0x0000, 0xffff, 0x0000, 0xffff);
  __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
  __m128 row0, row1, row2, row3, tmp0, tmp1, tmp2, tmp3;
  __m128i inp0, inp1, inp2, inp3, rec0, rec1, rec2, rec3;

  int luma_noskip = 0;
  int chroma_noskip = 0;

  const int qpc = qp_scale_chroma[qp];
  const int qp_div_6 = TAA_H264_DIV6 (qp);
  const int qp_mod_6 = qp - qp_div_6 * 6;
  const int qpc_div_6 = TAA_H264_DIV6 (qpc);
  const int qpc_mod_6 = qpc - qpc_div_6 * 6;
  const int thr_y  = threshold_tab[qp_mod_6] * (1 << (qp_div_6 - 2));
  const int thr_cr = threshold_tab[qpc_mod_6] * (1 << (qpc_div_6 - 2));

  //==================================================================================
  //                                 CHECK SKIP Y
  //==================================================================================
  for (int i = 0; i < 4; i++)
  {
    __m128 ymm[16], zmm[16];
    for (int k = 0; k<4; k++)
    {
      zmm[k+ 0] = _mm_load_ps ((float *) &luma->input[0+(4*i+k) * MB_WIDTH_Y]);
      zmm[k+ 4] = _mm_loadu_ps((float *) &luma->input[2+(4*i+k) * MB_WIDTH_Y]);
      zmm[k+ 8] = _mm_load_ps ((float *) &luma->recon[0+(4*i+k) * MB_WIDTH_Y]);
      zmm[k+12] = _mm_loadu_ps((float *) &luma->recon[2+(4*i+k) * MB_WIDTH_Y]);
    }

    for (int k = 0; k<8; k++)
    {
      ymm[k+0] = _mm_shuffle_ps((zmm[2*k]), (zmm[2*k+1]), 0x44);
      ymm[k+8] = _mm_shuffle_ps((zmm[2*k]), (zmm[2*k+1]), 0xEE);
    }

    xmm0 = _mm_castps_si128(_mm_shuffle_ps(ymm[0+0], ymm[1+0], 0x88));
    xmm4 = _mm_castps_si128(_mm_shuffle_ps(ymm[0+0], ymm[1+0], 0xDD));
    xmm2 = _mm_castps_si128(_mm_shuffle_ps(ymm[4+0], ymm[5+0], 0x88));
    xmm6 = _mm_castps_si128(_mm_shuffle_ps(ymm[4+0], ymm[5+0], 0xDD));

    xmm1 = _mm_sad_epu8(_mm_and_si128(M1, _mm_castps_si128(_mm_shuffle_ps(ymm[2+0], ymm[3+0], 0x88))), O);
    xmm5 = _mm_sad_epu8(_mm_and_si128(M1, _mm_castps_si128(_mm_shuffle_ps(ymm[2+0], ymm[3+0], 0xDD))), O);
    xmm3 = _mm_sad_epu8(_mm_and_si128(M1, _mm_castps_si128(_mm_shuffle_ps(ymm[6+0], ymm[7+0], 0x88))), O);
    xmm7 = _mm_sad_epu8(_mm_and_si128(M1, _mm_castps_si128(_mm_shuffle_ps(ymm[6+0], ymm[7+0], 0xDD))), O);

    xmm0 = _mm_packs_epi32(_mm_packs_epi32(_mm_sad_epu8(xmm0, O), _mm_sub_epi64(_mm_sad_epu8(_mm_and_si128(M1, xmm0), O), xmm1)), _mm_packs_epi32(_mm_sad_epu8(xmm4, O), _mm_sub_epi64(_mm_sad_epu8(_mm_and_si128(M1, xmm4), O), xmm5)));
    xmm1 = _mm_packs_epi32(_mm_packs_epi32(_mm_sad_epu8(xmm2, O), _mm_sub_epi64(_mm_sad_epu8(_mm_and_si128(M1, xmm2), O), xmm3)), _mm_packs_epi32(_mm_sad_epu8(xmm6, O), _mm_sub_epi64(_mm_sad_epu8(_mm_and_si128(M1, xmm6), O), xmm7)));
    xmm2 = _mm_hadd_epi16(xmm0, xmm1);
    xmm3 = _mm_hsub_epi16(xmm0, xmm1);

    luma_noskip |= _mm_movemask_epi8(_mm_cmpgt_epi16(_mm_abs_epi16(_mm_sub_epi16(_mm_unpacklo_epi64(xmm2, xmm3), _mm_unpackhi_epi64(xmm2, xmm3))), _mm_set1_epi16((int16_t)(thr_y))));
    if  (luma_noskip)
      break;

    xmm0 = _mm_castps_si128(_mm_shuffle_ps(ymm[0+8], ymm[1+8], 0x88));
    xmm4 = _mm_castps_si128(_mm_shuffle_ps(ymm[0+8], ymm[1+8], 0xDD));
    xmm2 = _mm_castps_si128(_mm_shuffle_ps(ymm[4+8], ymm[5+8], 0x88));
    xmm6 = _mm_castps_si128(_mm_shuffle_ps(ymm[4+8], ymm[5+8], 0xDD));

    xmm1 = _mm_sad_epu8(_mm_and_si128(M1, _mm_castps_si128(_mm_shuffle_ps(ymm[2+8], ymm[3+8], 0x88))), O);
    xmm5 = _mm_sad_epu8(_mm_and_si128(M1, _mm_castps_si128(_mm_shuffle_ps(ymm[2+8], ymm[3+8], 0xDD))), O);
    xmm3 = _mm_sad_epu8(_mm_and_si128(M1, _mm_castps_si128(_mm_shuffle_ps(ymm[6+8], ymm[7+8], 0x88))), O);
    xmm7 = _mm_sad_epu8(_mm_and_si128(M1, _mm_castps_si128(_mm_shuffle_ps(ymm[6+8], ymm[7+8], 0xDD))), O);

    xmm0 = _mm_packs_epi32(_mm_packs_epi32(_mm_sad_epu8(xmm0, O), _mm_sub_epi64(_mm_sad_epu8(_mm_and_si128(M1, xmm0), O), xmm1)), _mm_packs_epi32(_mm_sad_epu8(xmm4, O), _mm_sub_epi64(_mm_sad_epu8(_mm_and_si128(M1, xmm4), O), xmm5)));
    xmm1 = _mm_packs_epi32(_mm_packs_epi32(_mm_sad_epu8(xmm2, O), _mm_sub_epi64(_mm_sad_epu8(_mm_and_si128(M1, xmm2), O), xmm3)), _mm_packs_epi32(_mm_sad_epu8(xmm6, O), _mm_sub_epi64(_mm_sad_epu8(_mm_and_si128(M1, xmm6), O), xmm7)));
    xmm2 = _mm_hadd_epi16(xmm0, xmm1);
    xmm3 = _mm_hsub_epi16(xmm0, xmm1);

    luma_noskip |= _mm_movemask_epi8(_mm_cmpgt_epi16(_mm_abs_epi16(_mm_sub_epi16(_mm_unpacklo_epi64(xmm2, xmm3), _mm_unpackhi_epi64(xmm2, xmm3))), _mm_set1_epi16((int16_t)(thr_y))));
    if  (luma_noskip)
      break;
  }

  row0 = _mm_load_ps((float *) &chroma->input[(4*0+0) * MB_STRIDE_UV]);
  row1 = _mm_load_ps((float *) &chroma->input[(4*0+1) * MB_STRIDE_UV]);
  row2 = _mm_load_ps((float *) &chroma->input[(4*0+2) * MB_STRIDE_UV]);
  row3 = _mm_load_ps((float *) &chroma->input[(4*0+3) * MB_STRIDE_UV]);
  tmp0 = _mm_shuffle_ps((row0), (row1), 0x44);
  tmp2 = _mm_shuffle_ps((row0), (row1), 0xEE);
  tmp1 = _mm_shuffle_ps((row2), (row3), 0x44);
  tmp3 = _mm_shuffle_ps((row2), (row3), 0xEE);
  inp0 = _mm_castps_si128(_mm_shuffle_ps(tmp0, tmp1, 0x88));
  inp1 = _mm_castps_si128(_mm_shuffle_ps(tmp0, tmp1, 0xDD));
  inp2 = _mm_castps_si128(_mm_shuffle_ps(tmp2, tmp3, 0x88));
  inp3 = _mm_castps_si128(_mm_shuffle_ps(tmp2, tmp3, 0xDD));

  row0 = _mm_load_ps((float *) &chroma->recon[(4*0+0) * MB_STRIDE_UV]);
  row1 = _mm_load_ps((float *) &chroma->recon[(4*0+1) * MB_STRIDE_UV]);
  row2 = _mm_load_ps((float *) &chroma->recon[(4*0+2) * MB_STRIDE_UV]);
  row3 = _mm_load_ps((float *) &chroma->recon[(4*0+3) * MB_STRIDE_UV]);
  tmp0 = _mm_shuffle_ps((row0), (row1), 0x44);
  tmp2 = _mm_shuffle_ps((row0), (row1), 0xEE);
  tmp1 = _mm_shuffle_ps((row2), (row3), 0x44);
  tmp3 = _mm_shuffle_ps((row2), (row3), 0xEE);
  rec0 = _mm_castps_si128(_mm_shuffle_ps(tmp0, tmp1, 0x88));
  rec1 = _mm_castps_si128(_mm_shuffle_ps(tmp0, tmp1, 0xDD));
  rec2 = _mm_castps_si128(_mm_shuffle_ps(tmp2, tmp3, 0x88));
  rec3 = _mm_castps_si128(_mm_shuffle_ps(tmp2, tmp3, 0xDD));

  xmm0 = _mm_packs_epi32(_mm_packs_epi32(_mm_sad_epu8(inp0, O), _mm_sad_epu8(rec0, O)), _mm_packs_epi32(_mm_sad_epu8(inp1, O), _mm_sad_epu8(rec1, O)));
  xmm1 = _mm_packs_epi32(_mm_packs_epi32(_mm_sad_epu8(inp2, O), _mm_sad_epu8(rec2, O)), _mm_packs_epi32(_mm_sad_epu8(inp3, O), _mm_sad_epu8(rec3, O)));

  row0 = _mm_load_ps((float *) &chroma->input[(4*1+0) * MB_STRIDE_UV]);
  row1 = _mm_load_ps((float *) &chroma->input[(4*1+1) * MB_STRIDE_UV]);
  row2 = _mm_load_ps((float *) &chroma->input[(4*1+2) * MB_STRIDE_UV]);
  row3 = _mm_load_ps((float *) &chroma->input[(4*1+3) * MB_STRIDE_UV]);
  tmp0 = _mm_shuffle_ps((row0), (row1), 0x44);
  tmp2 = _mm_shuffle_ps((row0), (row1), 0xEE);
  tmp1 = _mm_shuffle_ps((row2), (row3), 0x44);
  tmp3 = _mm_shuffle_ps((row2), (row3), 0xEE);
  inp0 = _mm_castps_si128(_mm_shuffle_ps(tmp0, tmp1, 0x88));
  inp1 = _mm_castps_si128(_mm_shuffle_ps(tmp0, tmp1, 0xDD));
  inp2 = _mm_castps_si128(_mm_shuffle_ps(tmp2, tmp3, 0x88));
  inp3 = _mm_castps_si128(_mm_shuffle_ps(tmp2, tmp3, 0xDD));

  row0 = _mm_load_ps((float *) &chroma->recon[(4*1+0) * MB_STRIDE_UV]);
  row1 = _mm_load_ps((float *) &chroma->recon[(4*1+1) * MB_STRIDE_UV]);
  row2 = _mm_load_ps((float *) &chroma->recon[(4*1+2) * MB_STRIDE_UV]);
  row3 = _mm_load_ps((float *) &chroma->recon[(4*1+3) * MB_STRIDE_UV]);
  tmp0 = _mm_shuffle_ps((row0), (row1), 0x44);
  tmp2 = _mm_shuffle_ps((row0), (row1), 0xEE);
  tmp1 = _mm_shuffle_ps((row2), (row3), 0x44);
  tmp3 = _mm_shuffle_ps((row2), (row3), 0xEE);
  rec0 = _mm_castps_si128(_mm_shuffle_ps(tmp0, tmp1, 0x88));
  rec1 = _mm_castps_si128(_mm_shuffle_ps(tmp0, tmp1, 0xDD));
  rec2 = _mm_castps_si128(_mm_shuffle_ps(tmp2, tmp3, 0x88));
  rec3 = _mm_castps_si128(_mm_shuffle_ps(tmp2, tmp3, 0xDD));

  xmm2 = _mm_packs_epi32(_mm_packs_epi32(_mm_sad_epu8(inp0, O), _mm_sad_epu8(rec0, O)), _mm_packs_epi32(_mm_sad_epu8(inp1, O), _mm_sad_epu8(rec1, O)));
  xmm3 = _mm_packs_epi32(_mm_packs_epi32(_mm_sad_epu8(inp2, O), _mm_sad_epu8(rec2, O)), _mm_packs_epi32(_mm_sad_epu8(inp3, O), _mm_sad_epu8(rec3, O)));

  xmm0 = _mm_hsub_epi16(_mm_hadd_epi16(xmm0, xmm2), _mm_hadd_epi16(xmm1, xmm3));
  xmm1 = _mm_hadd_epi16(xmm0, O);
  xmm2 = _mm_hsub_epi16(xmm0, O);

  chroma_noskip = _mm_movemask_epi8(_mm_packs_epi16(
                                      _mm_cmpgt_epi16(_mm_abs_epi16(_mm_hadd_epi16(xmm1, xmm2)), _mm_set1_epi16((int16_t)(thr_cr))),
                                      _mm_cmpgt_epi16(_mm_abs_epi16(_mm_hsub_epi16(xmm1, xmm2)), _mm_set1_epi16((int16_t)(thr_cr)))));

  int flags = 0;
  if (luma_noskip == 0)
    flags |= EARLY_SKIP_CBP_LUMA_FLAG;
  if (chroma_noskip == 0)
    flags |= EARLY_SKIP_CBP_CHROMA_FLAG;
  if ((flags & EARLY_SKIP_CBP_MASK) == EARLY_SKIP_CBP_MASK)
    flags |= EARLY_SKIP_FLAG;

  return flags;
}
