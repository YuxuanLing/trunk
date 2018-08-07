#include "dec.h"
#include "dec_transform.h"
#include "fractaltest.h"
#include "fractaltest_utils.h"

#include <stdio.h>
#include <stdlib.h>

static char desc[128];

/* TODO: Write tests to tests the dynamic range in inverse transform */

typedef struct
{
  TAA_H264_ALIGN(16) short coeffs[16];
  short expect[16];
  int qp;
} test_data_transform_4x4;


static void test_inverse_transform_4x4 (void)
{
  test_data_transform_4x4 test_cases [] = {
    {
      .coeffs = {
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
      },
      .expect = {
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
      }
    },
    {
      .coeffs = {
        136, -28,  0, -4,
        -112,   0,  0,  0,
        0,   0,  0,  0,
        -16,   0,  0,  0
      },
      .expect = {
        0, 0, 0, 1,
        1, 1, 2, 2,
        2, 3, 3, 3,
        4, 4, 4, 4
      }
    },
    {
      .coeffs = {
        -920, -156, 20,   0,
        -26,  -32, 26, -32,
        -40,   78,  0,   0,
        0,   32,  0,   0
      },
      .expect = {
        -16, -16, -16, -13,
        -18, -16, -13, -9,
        -16, -15, -12, -11,
        -15, -15, -14, -14
      }
    },
  };

  TAA_H264_ALIGN(64) short block[16];

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_transform_4x4 * test = &test_cases[i];
    taa_h264_inverse_transform_4x4_dec (test->coeffs, block);
    int diff;
    COMPARE_ARRAYS_DESC (block, test->expect, 16, desc, sizeof (desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}

typedef struct
{
  short coeffs[16];
  short expect[16];
  int qp;
} test_data_transform_4x4_int;

static void test_inverse_hadamard_4x4 (void)
{
  test_data_transform_4x4_int test_cases [] = {
    {
      .coeffs = {
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
      },
      .expect = {
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
      },
      .qp = 26
    },
    {
      .coeffs = {
        -1, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
      },
      .expect = {
        -64, -64, -64, -64,
        -64, -64, -64, -64,
        -64, -64, -64, -64,
        -64, -64, -64, -64
      },
      .qp = 28
    },
    {
      .coeffs = {
        1,   -1,   -2,    0,
        5,   -1,    0,    0,
        0,    0,    0,    0,
        2,    0,    0,    0
      },
      .expect = {
        256,   512,  768,  512,
        0,   256,  512,  256,
        -256,     0,    0, -256,
        -512,  -256, -256, -512
      },
      .qp = 28
    },
    {
      .coeffs = {
        1, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
      },
      .expect = {
        896, 896, 896, 896,
        896, 896, 896, 896,
        896, 896, 896, 896,
        896, 896, 896, 896
      },
      .qp = 51
    },
    {
      .coeffs = {
        0,    0,    0,    0,
        -2,    0,    0,    0,
        0,    1,    0,    0,
        0,    1,    0,    0
      },
      .expect = {
        0,     0, -3584, -3584,
        -3584, -3584,     0,     0,
        1792,  1792,  1792,  1792,
        1792,  1792,  1792,  1792
      },
      .qp = 51
    },
    {
      .coeffs = {
        0,   -1,    0,    0,
        1,    0,    0,    0,
        0,    0,    1,    0,
        0,    0,    0,    0
      },
      .expect = {
        896,  -896,   896, 2688,
        -896,   896,  2688,  896,
        -2688,  -896,   896, -896,
        -896, -2688,  -896,  896
      },
      .qp = 51
    },
    {
      .coeffs = {
        5,   -1,    0,   -1,
        2,    0,    0,    0,
        1,    0,    0,    0,
        0,    0,    0,    0
      },
      .expect = {
        33,   44,   44,   55,
        22,   33,   33,   44,
        0,   11,   11,   22,
        11,   22,   22,   33
      },
      .qp = 7
    }
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_transform_4x4_int * test = &test_cases[i];
    short * dcbuf = test->coeffs;
    taa_h264_inverse_hadamard_and_dequant_4x4 (dcbuf, test->qp);
    int diff;
    COMPARE_ARRAYS_DESC (dcbuf, test->expect, 16, desc, sizeof (desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}

void test_dec_transform (void)
{
  test_inverse_transform_4x4 ();
  test_inverse_hadamard_4x4 ();
}
