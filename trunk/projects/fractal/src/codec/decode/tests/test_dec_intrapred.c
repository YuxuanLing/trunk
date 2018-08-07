#include "dec_intrapred.c"
#include "com_mbutils.h"
#include "fractaltest.h"
#include "fractaltest_utils.h"

static char desc[128];

typedef struct
{
  imode4_t mode;
  uint8_t left[4];
  uint8_t top[9];
  bool left_avail;
  bool top_avail;
  bool top_right_avail;
  short residual [4 * 4];
  uint8_t expect[4 * 4];
} test_data_intra_pred_luma;

static void test_intra_4x4_pred_luma_dc (void)
{
  uint8_t rblock [4 * 4];

  test_data_intra_pred_luma test_cases[] = {
    {
      /* Top left corner, no neighbour blocks available. */
      .left_avail = false,
      .top_avail = false,

      .residual = { 100, 100, 100, 100,
                    100, 100, 100, 100,
                    100, 100, 100, 100,
                    100, 100, 100, 100},

      .expect = { 228, 228, 228, 228,
                  228, 228, 228, 228,
                  228, 228, 228, 228,
                  228, 228, 228, 228}
    },
    {
      .left_avail = true,
      .top_avail = true,
      .left = { 101, 102, 103, 105},
      .top = { 1, 2, 3, 4},

      .residual = { 100, 100, 100, 100,
                    100, 100, 100, 100,
                    100, 100, 100, 100,
                    100, 100, 100, 100},

      /* dc = 53 */
      .expect = { 153, 153, 153, 153,
                  153, 153, 153, 153,
                  153, 153, 153, 153,
                  153, 153, 153, 153}
    },
    {
      .left_avail = false,
      .top_avail = true,
      .top = { 10, 20, 25, 5},

      /* dc = 15 */
      .residual = { 100, 100, 100, 100,
                    100, 100, 100, 100,
                    100, 100, 100, 100,
                    100, 100, 100, 100},

      .expect = { 115, 115, 115, 115,
                  115, 115, 115, 115,
                  115, 115, 115, 115,
                  115, 115, 115, 115}
    },
    {
      .left_avail = true,
      .top_avail = false,
      .left = { 10, 20, 25, 5},

      .residual = { 100, 100, 100, 100,
                    100, 100, 100, 100,
                    100, 100, 100, 100,
                    100, 100, 100, 100},

      /* dc = 15 */
      .expect = { 115, 115, 115, 115,
                  115, 115, 115, 115,
                  115, 115, 115, 115,
                  115, 115, 115, 115}
    }
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_intra_pred_luma * test = &test_cases[i];

    taa_h264_intra_pred_luma_dc (rblock, 4,
                                 test->left, test->top,
                                 test->left_avail, test->top_avail,
                                 test->residual);

    int diff = 0;
    COMPARE_2D_ARRAYS_DESC (rblock, test->expect, 4, 4, 4, 4, desc, sizeof (desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}


static void test_intra_4x4_pred_luma_hor (void)
{
  uint8_t rblock [4 * 4];

  test_data_intra_pred_luma test_cases[] = {
    {
      .left     = { 0, 0, 0, 0},

      .residual = { 100, 100, 100, 100,
                    100, 100, 100, 100,
                    100, 100, 100, 100,
                    100, 100, 100, 100},

      .expect   = { 100, 100, 100, 100,
                    100, 100, 100, 100,
                    100, 100, 100, 100,
                    100, 100, 100, 100}
    },
    {
      .left = { 101, 102, 103, 105},

      .residual = { 100, 100, 100, 100,
                    100, 100, 100, 100,
                    100, 100, 100, 100,
                    100, 100, 100, 100},

      .expect = { 201, 201, 201, 201,
                  202, 202, 202, 202,
                  203, 203, 203, 203,
                  205, 205, 205, 205}
    },
    {
      .left = { 200, 200, 200, 200},

      .residual = { 100, 100, 100, 100,
                    100, 100, 100, 100,
                    100, 100, 100, 100,
                    100, 100, 100, 100},

      .expect = { 255, 255, 255, 255,
                  255, 255, 255, 255,
                  255, 255, 255, 255,
                  255, 255, 255, 255}
    },
    {
      .left = { 0, 0, 0, 0},

      .residual = { -100, -100, -100, -100,
                    -100, -100, -100, -100,
                    -100, -100, -100, -100,
                    -100, -100, -100, -100},

      .expect = { 0, 0, 0, 0,
                  0, 0, 0, 0,
                  0, 0, 0, 0,
                  0, 0, 0, 0}
    }
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_intra_pred_luma * test = &test_cases[i];

    taa_h264_intra_pred_luma_hor (rblock, 4, test->left, test->residual);

    int diff = 0;
    COMPARE_2D_ARRAYS_DESC (rblock, test->expect, 4, 4, 4, 4, desc, sizeof (desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}


static void test_intra_4x4_pred_luma_vert (void)
{
  uint8_t rblock [4 * 4];

  test_data_intra_pred_luma test_cases[] = {
    {
      .top     = { 0, 0, 0, 0},

      .residual = { 100, 100, 100, 100,
                    100, 100, 100, 100,
                    100, 100, 100, 100,
                    100, 100, 100, 100},

      .expect   = { 100, 100, 100, 100,
                    100, 100, 100, 100,
                    100, 100, 100, 100,
                    100, 100, 100, 100}
    },
    {
      .top = { 10, 20, 25, 5},

      .residual = { 100, 100, 100, 100,
                    100, 100, 100, 100,
                    100, 100, 100, 100,
                    100, 100, 100, 100},

      .expect = { 110, 120, 125, 105,
                  110, 120, 125, 105,
                  110, 120, 125, 105,
                  110, 120, 125, 105}

    },
    {
      .top = { 200, 200, 200, 200},

      .residual = { 100, 100, 100, 100,
                    100, 100, 100, 100,
                    100, 100, 100, 100,
                    100, 100, 100, 100},

      .expect = { 255, 255, 255, 255,
                  255, 255, 255, 255,
                  255, 255, 255, 255,
                  255, 255, 255, 255}
    },
    {
      .top = { 0, 0, 0, 0},

      .residual = { -100, -100, -100, -100,
                    -100, -100, -100, -100,
                    -100, -100, -100, -100,
                    -100, -100, -100, -100},

      .expect = { 0, 0, 0, 0,
                  0, 0, 0, 0,
                  0, 0, 0, 0,
                  0, 0, 0, 0}
    }
  };


  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_intra_pred_luma * test = &test_cases[i];

    taa_h264_intra_pred_luma_vert (rblock, 4, test->top, test->residual);

    int diff = 0;
    COMPARE_2D_ARRAYS_DESC (rblock, test->expect, 4, 4, 4, 4, desc, sizeof (desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}

static void test_intra_4x4_pred_luma_down_left (void)
{
  uint8_t pred [4 * 4];

  test_data_intra_pred_luma test_cases[] = {
    {
      .top     = { 0, 0, 0, 0, 0, 0, 0, 0},
      .top_right_avail = true,
      .expect   = { 0, 0, 0, 0,
                    0, 0, 0, 0,
                    0, 0, 0, 0,
                    0, 0, 0, 0}
    },
    {
      .top     = { 55,  73, 103,  70, 86,  87,  55,  60},
      .top_right_avail = true,
      .expect   = { 76,   87,   82,   82,
                    87,   82,   82,   79,
                    82,   82,   79,   64,
                    82,   79,   64,   59 }
    },
    {
      .top     = { 42,  68, 106,  96,  96,  96,  96,  96},
      .top_right_avail = false,
      .expect   = { 71,   94,   99,   96,
                    94,   99,   96,   96,
                    99,   96,   96,   96,
                    96,   96,   96,   96 }
    },
  };


  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_intra_pred_luma * test = &test_cases[i];

    taa_h264_intra_pred_luma_down_left (test->top, test->top_right_avail, pred);

    int diff = 0;
    COMPARE_2D_ARRAYS_DESC (pred, test->expect, 4, 4, 4, 4, desc, sizeof (desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}


static void test_intra_4x4_pred_luma_down_right (void)
{
  uint8_t pred [4 * 4];

  test_data_intra_pred_luma test_cases[] = {
    {
      .top     = { 0, 0, 0, 0, 0},
      .left    = { 0, 0, 0, 0},
      .expect   = { 0, 0, 0, 0,
                    0, 0, 0, 0,
                    0, 0, 0, 0,
                    0, 0, 0, 0}
    },
    {
      .top = { 77, 85, 69, 60, 76},
      .left = { 94, 113, 92, 94},
      .expect   = { 83,   79,   71,   66,
                    95,   83,   79,   71,
                    103,   95,   83,   79,
                    98,  103,   95,   83 },
    },
  };


  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_intra_pred_luma * test = &test_cases[i];

    taa_h264_intra_pred_luma_down_right (test->left, &test->top[1], pred);

    int diff = 0;
    COMPARE_2D_ARRAYS_DESC (pred, test->expect, 4, 4, 4, 4, desc, sizeof (desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}


static void test_intra_4x4_pred_luma_vert_right (void)
{
  uint8_t pred [4 * 4];

  test_data_intra_pred_luma test_cases[] = {
    {
      .top     = { 0, 0, 0, 0, 0},
      .left    = { 0, 0, 0, 0},
      .expect   = { 0, 0, 0, 0,
                    0, 0, 0, 0,
                    0, 0, 0, 0,
                    0, 0, 0, 0}
    },
    {
      .top = { 86, 97,  97, 118, 117},
      .left = { 56, 60, 63, 62},
      .expect   = { 92,   97,  108,  118,
                    81,   94,  102,  113,
                    65,   92,   97,  108,
                    60,   81,   94,  102 },
    },
  };


  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_intra_pred_luma * test = &test_cases[i];

    taa_h264_intra_pred_luma_vert_right (test->left, &test->top[1], pred);

    int diff = 0;
    COMPARE_2D_ARRAYS_DESC (pred, test->expect, 4, 4, 4, 4, desc, sizeof (desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}


static void test_intra_4x4_pred_luma_horz_down (void)
{
  uint8_t pred [4 * 4];

  test_data_intra_pred_luma test_cases[] = {
    {
      .top     = { 0, 0, 0, 0, 0},
      .left    = { 0, 0, 0, 0},
      .expect   = { 0, 0, 0, 0,
                    0, 0, 0, 0,
                    0, 0, 0, 0,
                    0, 0, 0, 0}
    },
    {
      .top = { 59, 118,  94,  74,  34},
      .left = { 50, 66, 96, 104},
      .expect = {  55,   72,   97,   95,
                   58,   56,   55,   72,
                   81,   70,   58,   56,
                   100,   91,   81,   70 },
    },
  };


  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_intra_pred_luma * test = &test_cases[i];

    taa_h264_intra_pred_luma_horz_down (test->left, &test->top[1], pred);

    int diff = 0;
    COMPARE_2D_ARRAYS_DESC (pred, test->expect, 4, 4, 4, 4, desc, sizeof (desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}


static void test_intra_4x4_pred_luma_vert_left (void)
{
  uint8_t pred [4 * 4];

  test_data_intra_pred_luma test_cases[] = {
    {
      .top     = { 0, 0, 0, 0, 0, 0, 0, 0},
      .top_right_avail = true,
      .expect   = { 0, 0, 0, 0,
                    0, 0, 0, 0,
                    0, 0, 0, 0,
                    0, 0, 0, 0}
    },
    {
      .top     = { 110, 108,  90, 104, 97,  67,  61,  73},
      .top_right_avail = true,
      .expect   = { 109,   99,   97,  101,
                    104,   98,   99,   91,
                    99,   97,  101,   82,
                    98,   99,   91,   73 }
    },
    {
      .top     = { 32, 52, 64, 113, 0, 0, 0, 0},
      .top_right_avail = false,
      .expect   = { 42,   58,   89,  113,
                    50,   73,  101,  113,
                    58,   89,  113,  113,
                    73,  101,  113,  113}
    },
  };


  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_intra_pred_luma * test = &test_cases[i];

    taa_h264_intra_pred_luma_vert_left (test->top, test->top_right_avail, pred);

    int diff = 0;
    COMPARE_2D_ARRAYS_DESC (pred, test->expect, 4, 4, 4, 4, desc, sizeof (desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}


static void test_intra_4x4_pred_luma_horz_up (void)
{
  uint8_t pred [4 * 4];

  test_data_intra_pred_luma test_cases[] = {
    {
      .left    = { 0, 0, 0, 0},
      .expect   = { 0, 0, 0, 0,
                    0, 0, 0, 0,
                    0, 0, 0, 0,
                    0, 0, 0, 0}
    },
    {
      .left = { 44, 67, 65, 73},
      .expect   = { 56,   61,   66,   68,
                    66,   68,   69,   71,
                    69,   71,   73,   73,
                    73,   73,   73,   73 },
    },
  };


  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_intra_pred_luma * test = &test_cases[i];

    taa_h264_intra_pred_luma_horz_up (test->left, pred);

    int diff = 0;
    COMPARE_2D_ARRAYS_DESC (pred, test->expect, 4, 4, 4, 4, desc, sizeof (desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}


typedef struct
{
  bool left_avail;
  bool up_avail;
  uint8_t left[MB_HEIGHT_Y];
  uint8_t up[MB_WIDTH_Y + 1];
  uint8_t expect[MB_SIZE_Y];
} test_data_intra_pred;


static void test_intra_16x16_pred_luma_dc (void)
{
  uint8_t ipred[MB_SIZE_Y];

  test_data_intra_pred test_cases[] = {
    {
      .left_avail = false,
      .up_avail = false,
      .expect = {
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128
      },
    },
    {
      .left_avail = true,
      .up_avail = true,
      .left = { 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116},
      .up = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16},
      .expect = {
        59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,
        59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,
        59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,
        59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,
        59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,
        59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,
        59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,
        59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,
        59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,
        59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,
        59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,
        59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,
        59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,
        59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,
        59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,
        59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59,  59
      },
    },
    {
      .left_avail = false,
      .up_avail = true,
      .left = { 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116},
      .up = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16},
      .expect = {
        9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
        9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
        9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
        9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
        9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
        9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
        9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
        9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
        9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
        9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
        9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
        9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
        9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
        9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
        9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,
        9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9
      },
    },
    {
      .left_avail = true,
      .up_avail = false,
      .left = { 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116},
      .up = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16},
      .expect = {
        109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109,
        109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109,
        109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109,
        109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109,
        109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109,
        109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109,
        109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109,
        109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109,
        109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109,
        109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109,
        109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109,
        109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109,
        109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109,
        109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109,
        109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109,
        109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109
      },
    },
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_intra_pred * test = &test_cases[i];

    taa_h264_intra_16x16_pred_luma_dc (ipred, test->left, test->up,
                                       test->left_avail, test->up_avail);
    int diff;
    COMPARE_2D_ARRAYS_DESC (ipred, test->expect, 16, 16, MB_WIDTH_Y, 16,
                            desc, sizeof(desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}


static void test_intra_16x16_pred_luma_vert (void)
{
  uint8_t ipred[MB_SIZE_Y];

  test_data_intra_pred test_cases[] = {
    {
      .up = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150},
      .expect = {
        0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150,
        0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150,
        0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150,
        0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150,
        0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150,
        0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150,
        0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150,
        0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150,
        0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150,
        0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150,
        0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150,
        0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150,
        0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150,
        0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150,
        0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150,
        0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150,
      },
    }
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_intra_pred * test = &test_cases[i];

    taa_h264_intra_16x16_pred_luma_vert (ipred, test->up);

    int diff;
    COMPARE_2D_ARRAYS_DESC (ipred, test->expect, 16, 16, MB_WIDTH_Y, 16,
                            desc, sizeof(desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}


static void test_intra_16x16_pred_luma_hor (void)
{
  uint8_t ipred[MB_SIZE_Y];

  test_data_intra_pred test_cases[] = {
    {
      .left = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150},
      .expect = {
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
        20,  20,  20,  20,  20,  20,  20,  20,  20,  20,  20,  20,  20,  20,  20,  20,
        30,  30,  30,  30,  30,  30,  30,  30,  30,  30,  30,  30,  30,  30,  30,  30,
        40,  40,  40,  40,  40,  40,  40,  40,  40,  40,  40,  40,  40,  40,  40,  40,
        50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,
        60,  60,  60,  60,  60,  60,  60,  60,  60,  60,  60,  60,  60,  60,  60,  60,
        70,  70,  70,  70,  70,  70,  70,  70,  70,  70,  70,  70,  70,  70,  70,  70,
        80,  80,  80,  80,  80,  80,  80,  80,  80,  80,  80,  80,  80,  80,  80,  80,
        90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  90,
        100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
        110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110,
        120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120,
        130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130, 130,
        140, 140, 140, 140, 140, 140, 140, 140, 140, 140, 140, 140, 140, 140, 140, 140,
        150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150,
      },
    }
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_intra_pred * test = &test_cases[i];

    taa_h264_intra_16x16_pred_luma_hor (ipred, test->left);

    int diff;
    COMPARE_2D_ARRAYS_DESC (ipred, test->expect, 16, 16, MB_WIDTH_Y, 16,
                            desc, sizeof(desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}



static void test_intra_16x16_pred_luma_plane (void)
{
  uint8_t ipred[MB_SIZE_Y];

  test_data_intra_pred test_cases[] = {
    {
      .up = { 120, 93, 85, 82, 99, 105, 115, 121, 117, 125, 110, 105, 115, 119, 108, 100, 103},
      .left = { 98, 122, 113, 116, 111, 116, 110, 98, 114, 114, 129, 115, 112, 122, 128, 124},
      .expect  = {
        104, 104, 105, 106, 107, 107, 108, 109, 109, 110, 111, 112, 112, 113, 114, 114,
        104, 105, 106, 107, 107, 108, 109, 109, 110, 111, 112, 112, 113, 114, 114, 115,
        105, 106, 106, 107, 108, 109, 109, 110, 111, 112, 112, 113, 114, 114, 115, 116,
        106, 106, 107, 108, 109, 109, 110, 111, 111, 112, 113, 114, 114, 115, 116, 117,
        106, 107, 108, 109, 109, 110, 111, 111, 112, 113, 114, 114, 115, 116, 116, 117,
        107, 108, 109, 109, 110, 111, 111, 112, 113, 114, 114, 115, 116, 116, 117, 118,
        108, 109, 109, 110, 111, 111, 112, 113, 114, 114, 115, 116, 116, 117, 118, 119,
        108, 109, 110, 111, 111, 112, 113, 114, 114, 115, 116, 116, 117, 118, 119, 119,
        109, 110, 111, 111, 112, 113, 113, 114, 115, 116, 116, 117, 118, 119, 119, 120,
        110, 111, 111, 112, 113, 113, 114, 115, 116, 116, 117, 118, 118, 119, 120, 121,
        111, 111, 112, 113, 113, 114, 115, 116, 116, 117, 118, 118, 119, 120, 121, 121,
        111, 112, 113, 113, 114, 115, 116, 116, 117, 118, 118, 119, 120, 121, 121, 122,
        112, 113, 113, 114, 115, 116, 116, 117, 118, 118, 119, 120, 121, 121, 122, 123,
        113, 113, 114, 115, 115, 116, 117, 118, 118, 119, 120, 121, 121, 122, 123, 123,
        113, 114, 115, 115, 116, 117, 118, 118, 119, 120, 120, 121, 122, 123, 123, 124,
        114, 115, 115, 116, 117, 118, 118, 119, 120, 120, 121, 122, 123, 123, 124, 125
      }
    },
  };


  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_intra_pred * test = &test_cases[i];

    taa_h264_intra_16x16_pred_luma_plane (
      ipred, test->left, &test->up[1], test->up[0]);

    int diff;
    COMPARE_2D_ARRAYS_DESC (
      ipred, test->expect, 16, 16, MB_WIDTH_Y, 16,
      desc, sizeof(desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}


typedef struct
{
  uint8_t row[MB_WIDTH_UV + 1];
  uint8_t col[MB_HEIGHT_UV];
  int avail_flags;
  uint8_t expect_pred[MB_SIZE_UV];
} test_data_intra_pred_chroma;

static void test_intra_8x8_pred_chroma_dc (void)
{
  /* We only test one mode and one component, but need size for two
   * because of the expected memory layout. */
  uint8_t ipred[MB_SIZE_UV * 2];

  test_data_intra_pred_chroma test_cases[] = {
    {
      .avail_flags = MB_NONE_,
      .expect_pred =
      { 128,   128,   128,   128,   128,   128,   128,   128,
        128,   128,   128,   128,   128,   128,   128,   128,
        128,   128,   128,   128,   128,   128,   128,   128,
        128,   128,   128,   128,   128,   128,   128,   128,
        128,   128,   128,   128,   128,   128,   128,   128,
        128,   128,   128,   128,   128,   128,   128,   128,
        128,   128,   128,   128,   128,   128,   128,   128,
        128,   128,   128,   128,   128,   128,   128,   128},
    },
    {
      .avail_flags = MB_LEFT_ | MB_UP_,
      .row = { 1, 2, 3, 4, 5, 6, 7, 7},
      .col = { 101, 102, 103, 104, 105, 106, 107, 107},
      /* dc1 = 53 */
      /* dc2 = 6 */
      /* dc3 = 106*/
      /* dc4 = 56 */
      .expect_pred =
      { 53,    53,    53,    53,     6,     6,     6,     6,
        53,    53,    53,    53,     6,     6,     6,     6,
        53,    53,    53,    53,     6,     6,     6,     6,
        53,    53,    53,    53,     6,     6,     6,     6,
        106,   106,   106,   106,    56,    56,    56,    56,
        106,   106,   106,   106,    56,    56,    56,    56,
        106,   106,   106,   106,    56,    56,    56,    56,
        106,   106,   106,   106,    56,    56,    56,    56},
    },
    {
      .avail_flags = MB_UP_,
      .row = { 10, 20, 25, 5, 100, 110, 120, 125},
      /* dc1 = 15 */
      /* dc2 = 114 */
      /* dc3 = 15 */
      /* dc4 = 114 */
      .expect_pred =
      { 15,    15,    15,    15,   114,   114,   114,   114,
        15,    15,    15,    15,   114,   114,   114,   114,
        15,    15,    15,    15,   114,   114,   114,   114,
        15,    15,    15,    15,   114,   114,   114,   114,
        15,    15,    15,    15,   114,   114,   114,   114,
        15,    15,    15,    15,   114,   114,   114,   114,
        15,    15,    15,    15,   114,   114,   114,   114,
        15,    15,    15,    15,   114,   114,   114,   114},
    },
    {
      .avail_flags = MB_LEFT_,
      .col = { 10, 20, 25, 5, 100, 110, 120, 125},
      /* dc1 = 15 */
      /* dc2 = 15 */
      /* dc3 = 114 */
      /* dc4 = 114 */
      .expect_pred =
      { 15,    15,    15,    15,    15,    15,    15,    15,
        15,    15,    15,    15,    15,    15,    15,    15,
        15,    15,    15,    15,    15,    15,    15,    15,
        15,    15,    15,    15,    15,    15,    15,    15,
        114,   114,   114,   114,   114,   114,   114,   114,
        114,   114,   114,   114,   114,   114,   114,   114,
        114,   114,   114,   114,   114,   114,   114,   114,
        114,   114,   114,   114,   114,   114,   114,   114},
    },
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_intra_pred_chroma test = test_cases[i];

    taa_h264_intra_pred_chroma_dc (ipred, test.col, test.row,
                                   test.avail_flags & MB_LEFT_,
                                   test.avail_flags & MB_UP_);

    int diff = 0;
    COMPARE_2D_ARRAYS_DESC (ipred, test.expect_pred, MB_HEIGHT_UV, MB_WIDTH_UV,
                            MB_STRIDE_UV, MB_WIDTH_UV, desc, sizeof(desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}


static void test_intra_8x8_pred_chroma_vert (void)
{
  /* We only test one mode and one component, but need size for two
   * because of the expected memory layout. */
  uint8_t ipred[MB_SIZE_UV * 2];

  test_data_intra_pred_chroma test_cases[] = {
    {
      .row = { 0, 10, 50, 100, 110, 2, 4, 5},
      .expect_pred =
      { 0,    10,    50,   100,   110,     2,     4,     5,
        0,    10,    50,   100,   110,     2,     4,     5,
        0,    10,    50,   100,   110,     2,     4,     5,
        0,    10,    50,   100,   110,     2,     4,     5,
        0,    10,    50,   100,   110,     2,     4,     5,
        0,    10,    50,   100,   110,     2,     4,     5,
        0,    10,    50,   100,   110,     2,     4,     5,
        0,    10,    50,   100,   110,     2,     4,     5},
    },
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_intra_pred_chroma test = test_cases[i];

    taa_h264_intra_pred_chroma_vert (ipred, test.row);

    int diff = 0;
    COMPARE_2D_ARRAYS_DESC (ipred, test.expect_pred, MB_HEIGHT_UV, MB_WIDTH_UV,
                            MB_STRIDE_UV, MB_WIDTH_UV, desc, sizeof(desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}


static void test_intra_8x8_pred_chroma_hor (void)
{
  /* We only test one mode and one component, but need size for two
   * because of the expected memory layout. */
  uint8_t ipred[MB_SIZE_UV * 2];

  test_data_intra_pred_chroma test_cases[] = {
    {
      .col = { 0, 10, 50, 100, 110, 2, 4, 5},
      .expect_pred =
      {   0,     0,     0,     0,     0,     0,     0,     0,
          10,    10,    10,    10,    10,    10,    10,    10,
          50,    50,    50,    50,    50,    50,    50,    50,
          100,   100,   100,   100,   100,   100,   100,   100,
          110,   110,   110,   110,   110,   110,   110,   110,
          2,     2,     2,     2,     2,     2,     2,     2,
          4,     4,     4,     4,     4,     4,     4,     4,
          5,     5,     5,     5,     5,     5,     5,     5},
    },
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_intra_pred_chroma test = test_cases[i];

    taa_h264_intra_pred_chroma_hor (ipred, test.col);

    int diff = 0;
    COMPARE_2D_ARRAYS_DESC (ipred, test.expect_pred, MB_HEIGHT_UV, MB_WIDTH_UV,
                            MB_STRIDE_UV, MB_WIDTH_UV, desc, sizeof(desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}


static void test_intra_8x8_pred_chroma_plane (void)
{
  /* We only test one mode and one component, but need size for two
   * because of the expected memory layout. */
  uint8_t pred[MB_SIZE_UV * 2];

  test_data_intra_pred_chroma test_cases[] = {
    {
      .row = { 96, 112, 104, 115, 106, 105, 105, 105, 105},
      .col = { 101, 101, 101, 101, 115, 127, 133, 127},
      .expect_pred  = {
        101, 102, 102, 102, 102, 102, 102, 102,
        106, 106, 106, 107, 107, 107, 107, 107,
        111, 111, 111, 111, 111, 112, 112, 112,
        116, 116, 116, 116, 116, 116, 116, 117,
        120, 121, 121, 121, 121, 121, 121, 121,
        125, 125, 125, 126, 126, 126, 126, 126,
        130, 130, 130, 130, 130, 131, 131, 131,
        135, 135, 135, 135, 135, 135, 135, 136
      }
    },
  };


  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_intra_pred_chroma * test = &test_cases[i];

    taa_h264_intra_pred_chroma_plane (
      pred, test->col, &test->row[1], test->row[0]);

    int diff;
    COMPARE_2D_ARRAYS_DESC (pred, test->expect_pred, MB_HEIGHT_UV, MB_WIDTH_UV,
                            MB_STRIDE_UV, MB_WIDTH_UV, desc, sizeof(desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}



void test_dec_intrapred (void)
{
  test_intra_4x4_pred_luma_dc ();
  test_intra_4x4_pred_luma_vert ();
  test_intra_4x4_pred_luma_hor ();
  test_intra_4x4_pred_luma_down_left ();
  test_intra_4x4_pred_luma_down_right ();
  test_intra_4x4_pred_luma_vert_right ();
  test_intra_4x4_pred_luma_horz_down ();
  test_intra_4x4_pred_luma_vert_left ();
  test_intra_4x4_pred_luma_horz_up ();

  test_intra_16x16_pred_luma_dc ();
  test_intra_16x16_pred_luma_vert ();
  test_intra_16x16_pred_luma_hor ();
  test_intra_16x16_pred_luma_plane ();

  test_intra_8x8_pred_chroma_dc ();
  test_intra_8x8_pred_chroma_vert ();
  test_intra_8x8_pred_chroma_hor ();
  test_intra_8x8_pred_chroma_plane ();
}
