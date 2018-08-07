#include "dec.h"
#include "dec_quant.c"
#include "fractaltest.h"
#include "fractaltest_utils.h"

static char desc[64];

/* TODO: Add more test cases */
static void test_dequantize_4x4_no_runs_low_qp (void)
{
  const int qp = 6;
  const int chroma = 0;
  const int ac = 0;
  int diff;

  __declspec(align(16)) int16_t dqcoeff[16];
  __declspec(align(16)) int16_t qcoeff[16] = { -34, 16, 7, 14, 5, 4, 5, -2, -8, 4, -1, -5, 1, -1, -1, -1 };
  int expect[16] = { -680, 416, 80, 130, 182, 160, -52, 32, 280, -208, -100, -26, 104, -32, -26, -32 };

  taa_h264_dequantize_4x4 (qcoeff, dqcoeff, qp, chroma, ac, 0);
  COMPARE_ARRAYS_DESC (dqcoeff, expect, 16, desc, sizeof(desc), &diff);
  FASSERT2 (diff == 0, desc);
}

void test_dec_quant (void)
{
  test_dequantize_4x4_no_runs_low_qp ();
}
