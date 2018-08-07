#include "enc_mb.c"
#include "com_mbutils.h"
#include "fractaltest.h"
#include "fractaltest_utils.h"

static char desc[128];


typedef struct
{
  bool left_avail;
  bool up_avail;
  uint8_t left[4];
  uint8_t up[4];
  uint8_t expect[4 * 4];
  unsigned expect_sad;
} test_data_intra_pred;

static const imode4_t pred_mode = DC_PRED;
static const int lambda = 0;

static void test_intra_4x4_pred_luma_dc (void)
{
  uint8_t ipred[NUM_I4x4_MODES * 16];
  uint8_t block[MB_SIZE_Y];

  /* We always use the same input block, just for convenience */
  uint8_t block_in[16][16] =
  { {  0,  10,  20,  30, },
    { 40,  50,  60,  70, },
    { 80,  85,  90,  95, },
    { 100, 128, 200, 255, }};

  test_data_intra_pred test_cases[] = {
    {
      .left_avail = false,
      .up_avail = false,
      .expect = { 128, 128, 128, 128,
                  128, 128, 128, 128,
                  128, 128, 128, 128,
                  128, 128, 128, 128},
    },
    {
      .left_avail = true,
      .up_avail = true,
      .left = { 101, 102, 103, 105},
      .up = { 1, 2, 3, 4},
      /* dc = 53 */
      .expect = { 53,  53,  53,  53,
                  53,  53,  53,  53,
                  53,  53,  53,  53,
                  53,  53,  53,  53},
    },
    {
      .left_avail = false,
      .up_avail = true,
      .up = { 10, 20, 25, 5},
      /* dc = 15 */
      .expect = { 15,  15,  15,  15,
                  15,  15,  15,  15,
                  15,  15,  15,  15,
                  15,  15,  15,  15},
    },
    {
      .left_avail = true,
      .up_avail = false,
      .left = { 10, 20, 25, 5},
      /* dc = 15 */
      .expect = { 15,  15,  15,  15,
                  15,  15,  15,  15,
                  15,  15,  15,  15,
                  15,  15,  15,  15},
    },
  };

  FILL_ARRAY_FROM_MATRIX (block, block_in, 4, 4, MB_WIDTH_Y);

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_intra_pred * test = &test_cases[i];

    imode4_t best_mode;
    int coding_options = TAA_H264_LQ_DISABLE_HIGH_INTRA_4x4_MODES;
    uint8_t upleft = 0;
    bool upleft_avail = false;
    bool upright_avail = false;
    taa_h264_intra_4x4_pred_luma (
      block, test->left, test->up, upleft, test->left_avail, test->up_avail, upleft_avail,
      upright_avail, pred_mode, lambda, coding_options, &best_mode, ipred);

    int diff = 0;
    COMPARE_2D_ARRAYS_DESC (&ipred[DC_PRED * 16], test->expect,
                            4, 4, 4, 4, desc, sizeof(desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}




/* TODO: Add test cases to confirm that vertical and horizontal prediction is
 * not used where not allowed */
static void test_intra_4x4_pred_luma_vert (void)
{
  uint8_t ipred[NUM_I4x4_MODES * 16];
  uint8_t block[MB_SIZE_Y];

  /* We always use the same input block, just for convenience */
  uint8_t block_in[16][16] =
  { {  0,  10,  20,  30, },
    { 40,  50,  60,  70, },
    { 80,  85,  90,  95, },
    { 100, 128, 200, 255, }};

  test_data_intra_pred test_cases[] = {
    {
      /* Top left corner of MB, up and left MB available */
      .left_avail = true,
      .up_avail = true,
      .left = { 101, 102, 103, 105},
      .up = { 1, 2, 3, 4},
      .expect = { 1, 2, 3, 4,
                  1, 2, 3, 4,
                  1, 2, 3, 4,
                  1, 2, 3, 4},
    },
    {
      /* Top left corner of MB, only upper MB is available */
      .left_avail = false,
      .up_avail = true,
      .up = { 10, 20, 25, 5},
      .expect = { 10, 20, 25, 5,
                  10, 20, 25, 5,
                  10, 20, 25, 5,
                  10, 20, 25, 5},
    },
  };

  FILL_ARRAY_FROM_MATRIX (block, block_in, 4, 4, MB_WIDTH_Y);

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_intra_pred * test = &test_cases[i];

    imode4_t best_mode;
    int coding_options = TAA_H264_LQ_DISABLE_HIGH_INTRA_4x4_MODES;
    uint8_t upleft = 0;
    bool upleft_avail = false;
    bool upright_avail = false;
    taa_h264_intra_4x4_pred_luma (
      block, test->left, test->up, upleft, test->left_avail, test->up_avail, upleft_avail,
      upright_avail, pred_mode, lambda, coding_options, &best_mode, ipred);

    int diff = 0;
    COMPARE_2D_ARRAYS_DESC (&ipred[VERT_PRED * 16], test->expect,
                            4, 4, 4, 4, desc, sizeof(desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}


static void test_intra_4x4_pred_luma_hor (void)
{
  uint8_t ipred[NUM_I4x4_MODES * 16];
  uint8_t block[MB_SIZE_Y];

  /* We always use the same input block, just for convenience. Also, we
   * always pass the upper left block as input no matter what y8, x8, y4 and
   * x4 are. This is not really correct, but should not matter when it comes
   * to the behavior of intra_4x4_pred_luma(). */
  uint8_t block_in[16][16] =
  { {  0,  10,  20,  30, },
    { 40,  50,  60,  70, },
    { 80,  85,  90,  95, },
    { 100, 128, 200, 255, }};

  test_data_intra_pred test_cases[] = {
    {
      /* Top left corner of MB, up and left MB available */
      .left_avail = true,
      .up_avail = true,
      .left = { 101, 102, 103, 105},
      .up = { 1, 2, 3, 4},
      .expect = { 101, 101, 101, 101,
                  102, 102, 102, 102,
                  103, 103, 103, 103,
                  105, 105, 105, 105},
    },
    {
      /* Top left corner of MB, only left MB is available */
      .left_avail = true,
      .up_avail = false,
      .left = { 10, 20, 25, 5},
      .expect = { 10, 10, 10, 10,
                  20, 20, 20, 20,
                  25, 25, 25, 25,
                  5,  5,  5,  5},
    }
  };

  FILL_ARRAY_FROM_MATRIX (block, block_in, 4, 4, MB_WIDTH_Y);

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_intra_pred * test = &test_cases[i];

    imode4_t best_mode;
    int coding_options = TAA_H264_LQ_DISABLE_HIGH_INTRA_4x4_MODES;
    uint8_t upleft = 0;
    bool upleft_avail = false;
    bool upright_avail = false;
    taa_h264_intra_4x4_pred_luma (
      block, test->left, test->up, upleft, test->left_avail, test->up_avail, upleft_avail,
      upright_avail, pred_mode, lambda, coding_options, &best_mode, ipred);

    int diff = 0;
    COMPARE_2D_ARRAYS_DESC (&ipred[HOR_PRED * 16], test->expect,
                            4, 4, 4, 4, desc, sizeof(desc), &diff);
    FASSERT2 (diff == 0, desc);
  }
}

void test_enc_intrapred (void)
{
  test_intra_4x4_pred_luma_dc ();
  test_intra_4x4_pred_luma_vert ();
  test_intra_4x4_pred_luma_hor ();
}
