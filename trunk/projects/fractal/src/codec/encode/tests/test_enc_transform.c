#include "enc_mb.c"
#include "fractaltest.h"
#include "fractaltest_utils.h"

#include <stdio.h>
#include <stdlib.h>


static char desc[128];

/* NOTE: Hinv * diag{0.25, 0.2, 0.25, 0.2} * H = I, not Hinv * H = I */

typedef struct
{
  TAA_H264_ALIGN(16) int16_t block[16];
  TAA_H264_ALIGN(16) int16_t expect[16];
} test_data_transform_4x4;

static void test_transform_4x4 (void)
{
  test_data_transform_4x4 test_cases [] = {
    {
      .block =  {
        0,     0,     0,     0,
        0,     0,     0,     0,
        0,     0,     0,     0,
        0,     0,     0,     0
      },
      .expect = {
        0,     0,     0,     0,
        0,     0,     0,     0,
        0,     0,     0,     0,
        0,     0,     0,     0
      }
    },
    {
      .block =  {
        1,     1,     1,     1,
        1,     1,     1,     1,
        1,     1,     1,     1,
        1,     1,     1,     1
      },
      .expect = {
        16,     0,     0,     0,
        0,     0,     0,     0,
        0,     0,     0,     0,
        0,     0,     0,     0
      }
    },
    {
      .block =  {
        255,  255,  255,  255,
        255,  255,  255,  255,
        255,  255,  255,  255,
        255,  255,  255,  255
      },
      .expect = {
        4080,    0,    0,    0,
        0,    0,    0,    0,
        0,    0,    0,    0,
        0,    0,    0,    0
      }
    },
    {
      .block = {
        0,   255,     0,   255,
        255,     0,   255,     0,
        0,   255,     0,   255,
        255,     0,   255,     0
      },
      .expect = {
        2040,     0,     0,     0,
        0,  -510,     0, -1530,
        0,     0,     0,     0,
        0, -1530,     0, -4590
      }
    },
    {
      .block = {
        55,   158,    85,   108,
        19,   220,   112,   189,
        50,   121,   218,   218,
        202,   236,   139,   195
      },
      .expect = {
        2325,  -587,  -253,  -746,
        -799,   -87,  -257,  -436,
        31,   743,   137,   -26,
        -232,  -546,     4,   812
      }
    },
    {
      .block = {
        31,   201,   251,   130,
        170,   211,     5,   129,
        150,    14,    37,   249,
        236,    18,    17,   129
      },
      .expect = {
        1978,    34,   470,  -318,
        491,  -417, -1507,  -526,
        48,  -100,  -392,   530,
        83, -1481,   -91,   532
      }
    },
    {
      .block = {
        0,   1,   1,   0,
        2,   0,   0,   0,
        0,   0,   0,   0,
        0,   0,   0,   0
      },
      .expect = {
        4,   4,   0,   2,
        6,   4,  -2,   2,
        0,  -4,  -4,  -2,
        -2,  -8,  -6,  -4
      }
    },
    {
      .block = {
        -17,   -16,   -16,   -13,
        -18,   -17,   -13,    -9,
        -16,   -15,   -12,   -10,
        -16,   -15,   -14,   -14
      },
      .expect = {
        -231,   -50,     5,    -5,
        -10,   -13,     8,    -9,
        -11,    24,    -3,    -3,
        5,    11,    -1,    -2
      }
    }
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_transform_4x4 test = test_cases[i];
    taa_h264_transform_4x4_x86_64 ((__m128i *)test.block);
    int diff;
    COMPARE_ARRAYS_DESC (test.block, test.expect, 16, desc, sizeof (desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}




static void test_hadamard_4x4 (void)
{
  test_data_transform_4x4 test_cases [] = {
    {
      .block =  {
        0,     0,     0,     0,
        0,     0,     0,     0,
        0,     0,     0,     0,
        0,     0,     0,     0
      },
      .expect = {
        0,     0,     0,     0,
        0,     0,     0,     0,
        0,     0,     0,     0,
        0,     0,     0,     0
      }
    },
    {
      .block =  {
        1,     1,     1,     1,
        1,     1,     1,     1,
        1,     1,     1,     1,
        1,     1,     1,     1
      },
      .expect = {
        8,     0,     0,     0,
        0,     0,     0,     0,
        0,     0,     0,     0,
        0,     0,     0,     0
      }
    },
    {
      .block =  {
        -1,    -1,    -1,    -1,
        -1,    -1,    -1,    -1,
        -1,    -1,    -1,    -1,
        -1,    -1,    -1,    -1
      },
      .expect = {
        -8,     0,     0,     0,
        0,     0,     0,     0,
        0,     0,     0,     0,
        0,     0,     0,     0
      }
    },
    {
      .block =  {
        1000, 1000, 1000, 1000,
        1000, 1000, 1000, 1000,
        1000, 1000, 1000, 1000,
        1000, 1000, 1000, 1000
      },
      .expect = {
        8000,    0,    0,    0,
        0,    0,    0,    0,
        0,    0,    0,    0,
        0,    0,    0,    0
      }
    },
    {
      .block =  {
        179,   17, -199, -188,
        137,   10, -199, -188,
        -14,  -66, -199, -175,
        -1,   -5, -205, -175
      },
      .expect = {
        -636,  892,  210,  134,
        204,  224,  100,  132,
        58,   64,   -4,  -10,
        -10,  -16,   38,   44
      }
    }
  };

  int16_t coeff[16];

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_transform_4x4 test = test_cases[i];
    taa_h264_hadamard_4x4 (test.block, coeff);
    int diff;
    COMPARE_ARRAYS_DESC (coeff, test.expect, 16, desc, sizeof (desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}

typedef struct
{
  int16_t TAA_H264_ALIGN(64) coeffs[MB_SIZE_Y];
  int16_t expect[16];
} test_data_itransform_4x4;


static void test_inverse_transform_4x4 (void)
{
  test_data_itransform_4x4 test_cases [] = {
    {
      /* All zeros */
      .coeffs = {
        0,     0,     0,     0,
        0,     0,     0,     0,
        0,     0,     0,     0,
        0,     0,     0,     0
      },
      .expect = {
        0,     0,     0,     0,
        0,     0,     0,     0,
        0,     0,     0,     0,
        0,     0,     0,     0
      }
    },
    {
      /* Positive DC, negative AC */
      .coeffs = {
        136,   -28,     0,    -4,
        -112,     0,     0,     0,
        0,     0,     0,     0,
        -16,     0,     0,     0
      },
      .expect = {
        0,     0,     0,     1,
        1,     1,     2,     2,
        2,     3,     3,     3,
        4,     4,     4,     4
      }
    },
    {
      /* Some random numbers */
      .coeffs = {
        -920,  -156,    20,     0,
        -26,   -32,    26,   -32,
        -40,    78,     0,     0,
        0,    32,     0,     0
      },
      .expect = {
        -16,   -16,   -16,   -13,
        -18,   -16,   -13,    -9,
        -16,   -15,   -12,   -11,
        -15,   -15,   -14,   -14
      }
    },
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_itransform_4x4 test = test_cases[i];

    taa_h264_inverse_transform_4x4_x86_64 ((__m128i *)test.coeffs);

    int diff;
    COMPARE_2D_ARRAYS_DESC (test.coeffs, test.expect, 4, 4, 4, 4, desc, sizeof (desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}

static void test_inverse_hadamard_4x4 (void)
{
  test_data_itransform_4x4 test_cases [] = {
    {
      .coeffs = {
        0,     0,     0,     0,
        0,     0,     0,     0,
        0,     0,     0,     0,
        0,     0,     0,     0
      },
      .expect = {
        0,     0,     0,     0,
        0,     0,     0,     0,
        0,     0,     0,     0,
        0,     0,     0,     0
      }
    },
    {
      .coeffs =  {
        1,     1,     1,     1,
        1,     1,     1,     1,
        1,     1,     1,     1,
        1,     1,     1,     1
      },
      .expect = {
        16,     0,     0,     0,
        0,     0,     0,     0,
        0,     0,     0,     0,
        0,     0,     0,     0
      }
    },
    {
      .coeffs =  {
        -1,    -1,    -1,    -1,
        -1,    -1,    -1,    -1,
        -1,    -1,    -1,    -1,
        -1,    -1,    -1,    -1
      },
      .expect = {
        -16,     0,     0,     0,
        0,     0,     0,     0,
        0,     0,     0,     0,
        0,     0,     0,     0
      }
    },
    {
      .coeffs = {
        -11,    14,    -4,     4,
        6,    13,    -3,     5,
        6,     3,    -1,     0,
        -1,     1,    -1,     1
      },
      .expect = {
        32,    30,   -12,   -50,
        16,    10,   -20,   -46,
        -26,   -24,   -22,   -24,
        -10,    -4,   -14,   -12
      }
    }
  };

  int16_t block[16];

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_itransform_4x4 test = test_cases[i];

    taa_h264_inverse_hadamard_4x4 (test.coeffs, block);

    int diff;
    COMPARE_2D_ARRAYS_DESC (block, test.expect, 4, 4, 4, 4, desc, sizeof (desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}


/* TODO: Write tests to tests the dynamic range in inverse transform */

void test_enc_transform (void)
{
  test_transform_4x4 ();
  test_inverse_transform_4x4 ();
  test_hadamard_4x4 ();
  test_inverse_hadamard_4x4 ();
}
