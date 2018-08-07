#include "fractaltest.h"
#include "fractaltest_utils.h"
#include "enc_mb.c"
#define HAVE_SSSE3
#include "enc_quant_template.h"

#include <stdlib.h>

static char desc[64];
#define NO_VAL 1000

static void coeffs4_to_array (
  const coeffs4_t * coeffs4,
  int16_t *         a,
  int               skip,
  int               max_coeffs)
{
  int n = 0;
  int idx = skip;
  for (int i = 0; i < skip; i++)
    a[n++] = NO_VAL;
  for (int i = 0; i < coeffs4->num_nonzero; i++)
  {
    for (int j = 0; j < coeffs4->run[idx]; j++)
      a[n++] = 0;
    a[n++] = (int16_t) (coeffs4->level[idx] * (coeffs4->sign[idx] ? -1 : 1));
    idx++;
  }
  while (n < max_coeffs + skip)
    a[n++] = 0;
}


typedef struct
{
  int16_t TAA_H264_ALIGN(64) tcoeff[16];
  int qp;
  bool inter;
  int16_t TAA_H264_ALIGN(64) expect_tcoeff[16];
  int16_t TAA_H264_ALIGN(64) expect_qcoeff[16];
} test_data_quantize_4x4;

static void test_quant_dequant_4x4_luma (void)
{
  cpuinfo_t cpuinfo;
  taa_h264_get_cpuinfo (&cpuinfo);

  test_data_quantize_4x4 test_cases[] = {
    {
      .tcoeff = {   36,   -320,     -8,   -140,
                    575,   -538,    -93,   -159,
                    -198,    204,     86,     62,
                    -575,    416,    141,    113 },
      .qp = 23,
      .inter = true,
      .expect_tcoeff = {    144,   -920,      0,   -368,
                            1840,  -1392,   -184,   -232,
                            -720,    552,    288,    184,
                            -1840,    928,    368,    232 },
      .expect_qcoeff = {  1,     -5,     10,     -5,
                          -6,      0,     -2,     -1,
                          3,    -10,      4,      2,
                          -1,      1,      2,      1}
    },
    {
      .tcoeff = { -461,   -113,     -3,      6,
                  -514,    -63,     16,     21,
                  327,    -87,      5,     24,
                  603,   -134,      3,      3},
      .qp = 23,
      .inter = true,
      .expect_tcoeff = { -1728,   -368,      0,      0,
                         -1656,      0,      0,      0,
                         1296,   -184,      0,      0,
                         1840,   -232,      0,      0},
      .expect_qcoeff = {-12,     -2,     -9,      9,
                        0,      0,      0,      0,
                        -1,     10,     -1,      0,
                        0,      0,      0,      0},
    }
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_quantize_4x4 test = test_cases[i];
    int16_t qcoeff[16];
    coeffs4_t coeffs4;
    int dummy_cost;
    const int qp_div_6 = TAA_H264_DIV6 (test.qp);
    const int qp_mod_6 = test.qp - qp_div_6 * 6;
    const int qp_bits = Q_BITS + qp_div_6;
    const int shift_q_offset = 8 - qp_div_6 + (test.inter ? 1 : 0);
    const int qp_offset = MAX_Q_OFFSET >> shift_q_offset;

    taa_h264_quant_dequant_4x4_noskip (qp_div_6, qp_mod_6, qp_bits, qp_offset,
                                       test.tcoeff, &coeffs4, &dummy_cost);
    coeffs4_to_array (&coeffs4, qcoeff, 0, 16);

    int diff;
    /* Check scanned and quantized values */
    COMPARE_ARRAYS_DESC (qcoeff, test.expect_qcoeff, 16, desc, sizeof(desc), &diff);
    FASSERT2 (diff == 0, desc);

    /* Check dequantized values */
    COMPARE_ARRAYS_DESC (test.tcoeff, test.expect_tcoeff, 16, desc, sizeof(desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}


static void test_quant_dequant_16x16_luma_ac (void)
{
  cpuinfo_t cpuinfo;
  taa_h264_get_cpuinfo (&cpuinfo);

  test_data_quantize_4x4 test_cases[] = {
    {
      .tcoeff = { NO_VAL,     16,     -9,     23,
                  93,     36,    -39,    -47,
                  -19,    -32,    -59,    -11,
                  -66,   -142,    -42,    -16 },
      .qp = 26,
      .inter = false,
      .expect_tcoeff = { NO_VAL,      0,      0,      0,
                         256,      0,      0,      0,
                         0,      0,   -208,      0,
                         -256,   -320,      0,      0},
      .expect_qcoeff = { NO_VAL,      0,      1,      0,
                         0,      0,      0,      0,
                         0,     -1,     -1,     -1,
                         0,      0,      0,      0}
    },
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_quantize_4x4 test = test_cases[i];
    int16_t qcoeff[16];
    coeffs4_t coeffs4;
    int dummy_cost;
    const int qp_div_6 = TAA_H264_DIV6 (test.qp);
    const int qp_mod_6 = test.qp - qp_div_6 * 6;
    const int qp_bits = Q_BITS + qp_div_6;
    const int shift_q_offset = 8 - qp_div_6;
    const int qp_offset = MAX_Q_OFFSET >> shift_q_offset;

    taa_h264_quant_dequant_4x4_skip (qp_div_6, qp_mod_6, qp_bits, qp_offset,
                                     test.tcoeff, &coeffs4, &dummy_cost);

    coeffs4_to_array (&coeffs4, qcoeff, 1, 15);

    int diff;
    /* Check scanned and quantized values */
    COMPARE_ARRAYS_DESC (&qcoeff[1], &test.expect_qcoeff[1], 15, desc, sizeof(desc), &diff);
    FASSERT2 (diff == 0, desc);

    /* Check dequantized values */
    COMPARE_ARRAYS_DESC (&test.tcoeff[1], &test.expect_tcoeff[1],
                         15, desc, sizeof(desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}

static void test_quant_dc4x4_sanity (void)
{
  /* Testing sanity for low level quantization functions. */
  cpuinfo_t cpuinfo;
  taa_h264_get_cpuinfo (&cpuinfo);

  // Check dynamic range.
  for (int i = INT16_MIN; i <= INT16_MAX; i += 10)
  {
    // We use the minimum allowed QP do have the highest range on quantized values
    const int qp = 0;
    const bool intra = true;
    const int qp_div_6 = TAA_H264_DIV6 (qp);
    const int qp_bits = Q_BITS + qp_div_6;
    const int shift_q_offset = 8 - qp_div_6 + (intra ? 0 : 1);
    const int qp_offset = MAX_Q_OFFSET_DC >> shift_q_offset;

    TAA_H264_ALIGN(16) int16_t tcoeff_in[16] = {0, };
    TAA_H264_ALIGN(16) uint8_t qcoeff[16];
    TAA_H264_ALIGN(16) int8_t sign[16];
    int flag;

    // Set up transform coeffs with some randomly picked values.
    tcoeff_in[0] = (int16_t) i;
    tcoeff_in[1] = (int16_t) (-i * 2 / 3);
    tcoeff_in[3] = (int16_t) i;
    tcoeff_in[15] = (int16_t) (-i/2);

    TAA_H264_ALIGN(16) int16_t tcoeff[16];
    if (cpuinfo.have_sse2)
    {
      for (int j = 0; j < 16; j++)
        tcoeff[j] = tcoeff_in[j];
      flag = quant_sse2 (quant_coef[qp % 6][0], qp_bits+1, qp_offset, tcoeff, qcoeff, sign);
    }
    else
    {
      FASSERT (false);
    }

    if (cpuinfo.have_ssse3)
    {
      TAA_H264_ALIGN(16) int16_t tcoeff2[16];
      TAA_H264_ALIGN(16) uint8_t qcoeff2[16];
      TAA_H264_ALIGN(16) int8_t sign2[16];
      for (int j = 0; j < 16; j++)
        tcoeff2[j] = tcoeff_in[j];
      int flag2 = quant_ssse3 (quant_coef[qp % 6][0], qp_bits+1, qp_offset, tcoeff2, qcoeff2, sign2);
      /* Make sure SSE2 and SSSE3 yields same results. SSE2 functions returns
       * value in raster scan order, while SSSE3 in zigzag  order, so we can't
       * compare directly. */
      for (int j = 0; j < 16; j++)
      {
        // compare the coeffs in raster scan order
        FASSERT (((flag >> j) & 0x1) == ((flag2 >> zigzag[j]) & 0x1));
        FASSERT (qcoeff[j] == qcoeff2[zigzag[j]]);
        FASSERT (sign[j] == sign2[zigzag[j]]);
        FASSERT (tcoeff[j] == tcoeff2[j]);
      }
    }

    for (int n = 0; n < 16; n++)
    {
      // Check that each flag and sign is correct.
      bool nonzero_flag = (flag >> n) & 0x1;
      FASSERT (nonzero_flag == (qcoeff[n] != 0));
      if (nonzero_flag)
      {
        if (tcoeff_in[n] < 0)
          FASSERT (sign[n] != 0);
        else
          FASSERT (sign[n] == 0);
      }

      // Check that signaled coeffs and internal coeffs are equal
      FASSERT (tcoeff[n] == (sign[n] ? -qcoeff[n] : qcoeff[n]));
    }
  }
}

static int dequant (int qcoeff,
                    int zigzag_idx,
                    int quant_div_6,
                    int quant_mod_6)
{
  static const int dequant_coef[6][16] = {
    { 10, 13, 13, 10, 16, 10, 13, 13, 13, 13, 16, 10, 16, 13, 13, 16},
    { 11, 14, 14, 11, 18, 11, 14, 14, 14, 14, 18, 11, 18, 14, 14, 18},
    { 13, 16, 16, 13, 20, 13, 16, 16, 16, 16, 20, 13, 20, 16, 16, 20},
    { 14, 18, 18, 14, 23, 14, 18, 18, 18, 18, 23, 14, 23, 18, 18, 23},
    { 16, 20, 20, 16, 25, 16, 20, 20, 20, 20, 25, 16, 25, 20, 20, 25},
    { 18, 23, 23, 18, 29, 18, 23, 23, 23, 23, 29, 18, 29, 23, 23, 29}
  };

  return (qcoeff * dequant_coef[quant_mod_6][zigzag_idx]) << quant_div_6;
}

static void test_quant_4x4_sanity (void)
{
  /* Testing sanity for low level quantization/dequantization functions. */
  cpuinfo_t cpuinfo;
  taa_h264_get_cpuinfo (&cpuinfo);

  // Check dynamic range.
  for (int i = INT16_MIN; i <= INT16_MAX; i += 10)
  {
    // We use the minimum allowed QP do have the highest range on quantized values
    const int qp = 0;
    const bool intra = true;
    const int qp_div_6 = TAA_H264_DIV6 (qp);
    const int qp_mod_6 = qp - (qp_div_6*6);
    const int qp_bits = Q_BITS + qp_div_6;
    const int shift_q_offset = 8 - qp_div_6 + (intra ? 0 : 1);
    const int qp_offset = MAX_Q_OFFSET_DC >> shift_q_offset;

    TAA_H264_ALIGN(16) int16_t tcoeff_in[16] = {0, };
    TAA_H264_ALIGN(16) uint8_t qcoeff[16];
    TAA_H264_ALIGN(16) int8_t sign[16];
    int flag;

    // Set up transform coeffs with some randomly picked values.
    tcoeff_in[0] = (int16_t) i;
    tcoeff_in[1] = (int16_t) (-i * 2 / 3);
    tcoeff_in[3] = (int16_t) i;
    tcoeff_in[15] = (int16_t) (-i/2);

    TAA_H264_ALIGN(16) int16_t tcoeff[16];
    if (cpuinfo.have_sse2)
    {
      for (int j = 0; j < 16; j++)
        tcoeff[j] = tcoeff_in[j];
      flag = quant_dequant_sse2 (qp_div_6, qp_mod_6, qp_bits, qp_offset, (__m128i*)tcoeff, qcoeff, sign);
    }
    else
    {
      FASSERT (false);
    }

    if (cpuinfo.have_ssse3)
    {
      TAA_H264_ALIGN(16) int16_t tcoeff2[16];
      TAA_H264_ALIGN(16) uint8_t qcoeff2[16];
      TAA_H264_ALIGN(16) int8_t sign2[16];
      for (int j = 0; j < 16; j++)
        tcoeff2[j] = tcoeff_in[j];
      int flag2 = quant_dequant_ssse3 (qp_div_6, qp_mod_6, qp_bits, qp_offset, (__m128i*)tcoeff2, qcoeff2, sign2);
      /* Make sure SSE2 and SSSE3 yields same results. SSE2 functions returns
       * value in raster scan order, while SSSE3 in zigzag  order, so we can't
       * compare directly. */
      for (int j = 0; j < 16; j++)
      {
        // compare the coeffs in raster scan order
        FASSERT (((flag >> j) & 0x1) == ((flag2 >> zigzag[j]) & 0x1));
        FASSERT (qcoeff[j] == qcoeff2[zigzag[j]]);
        FASSERT (sign[j] == sign2[zigzag[j]]);
        FASSERT (tcoeff[j] == tcoeff2[j]);
      }
    }

    for (int n = 0; n < 16; n++)
    {
      // Check that each flag and sign is correct.
      bool nonzero_flag = (flag >> n) & 0x1;
      FASSERT (nonzero_flag == (qcoeff[n] != 0));
      if (nonzero_flag)
      {
        if (tcoeff_in[n] < 0)
          FASSERT (sign[n] != 0);
        else
          FASSERT (sign[n] == 0);
      }

      // Check that signaled coeffs and internal coeffs are equal
      // The returned coeffs are dequantized
      int dq = dequant (qcoeff[n], zigzag[n], 0, 0);
      FASSERT (tcoeff[n] == ((sign[n] ? -dq : dq)));
    }
  }
}


void test_enc_quant (void)
{
  test_quant_dequant_4x4_luma ();
  test_quant_dequant_16x16_luma_ac ();

  test_quant_dc4x4_sanity ();
  test_quant_4x4_sanity ();
}
