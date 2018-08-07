#include "com_prediction.h"
#include "com_mbutils.h"
#include "fractaltest.h"
#include "fractaltest_utils.h"

#include <stdio.h>
#include <stdlib.h>


static char desc[128];
static const int NUM_MVS_MB = 16;


/* Init the motion vectors to a fixed pattern of values so that it is easy to
 * know the expected output. Not good for testing coverage though. Sets up 3x3
 * MBs.
 */
static mv_t * alloc_mvs_default_values (
  int * stride,
  int   mbymax,
  int   mbxmax)
{
  const int offset_per_mb = 20;
  const int x_offset = 1000;

  *stride = mbymax * NUM_MVS_MB;

  mv_t * motion_vectors = malloc (mbymax * mbxmax * NUM_MVS_MB * sizeof (mv_t));
  mv_t * pmv = motion_vectors;

  for (int i = 0; i < mbymax; i++)
  {
    for (int j = 0; j < mbxmax; j++)
    {
      int mb_offset = (i * mbymax + j) * offset_per_mb;
      for (int k = 0; k < NUM_MVS_MB; k++)
      {
        pmv->y = (uint16_t)(mb_offset + k);
        pmv->x = (uint16_t)(mb_offset + k + x_offset);
        pmv->ref_num = 0;
        pmv++;
      }
    }
  }
  return motion_vectors;
}

typedef struct
{
  mv_t pred;
  mv_t left;
  mv_t up;
  int mb_avail_flags;
  mv_t expect_mv;
  bool expect_ret;
} test_data_skip_mv;

static void test_skip_mv (void)
{
  int stride;
  const int mbymax = 3;
  const int mbxmax = 3;
  mv_t * mvs = alloc_mvs_default_values (&stride, mbymax, mbxmax);

  test_data_skip_mv test_cases[] = {
    {
      /* Up not available */
      .pred = { .x = 10, .y = -20, .ref_num = 1},
      .left = { .x =  1, .y =   1, .ref_num = 0},
      .up   = { .x =  1, .y =   1, .ref_num = 0},
      .mb_avail_flags = MB_LEFT_,
      .expect_mv = { .x =  0, .y =   0, .ref_num = 0},
      .expect_ret = true
    },
    {
      /* Left not available */
      .pred = { .x = 10, .y = -20, .ref_num = 1},
      .left = { .x =  1, .y =   1, .ref_num = 0},
      .up   = { .x =  1, .y =   1, .ref_num = 0},
      .mb_avail_flags = MB_UP_,
      .expect_mv = { .x =  0, .y =   0, .ref_num = 0},
      .expect_ret = true
    },
    {
      /* Both available, left is zero */
      .pred = { .x = 10, .y = -20, .ref_num = 1},
      .left = { .x =  0, .y =   0, .ref_num = 0},
      .up   = { .x =  1, .y =   1, .ref_num = 0},
      .mb_avail_flags = MB_LEFT_ | MB_UP_,
      .expect_mv = { .x =  0, .y =   0, .ref_num = 0},
      .expect_ret = true
    },
    {
      /* Both available, up is zero */
      .pred = { .x = 10, .y = -20, .ref_num = 1},
      .left = { .x =  1, .y =   1, .ref_num = 0},
      .up   = { .x =  0, .y =   0, .ref_num = 0},
      .mb_avail_flags = MB_LEFT_ | MB_UP_,
      .expect_mv = { .x =  0, .y =   0, .ref_num = 0},
      .expect_ret = true
    },
    {
      /* Both available, none is zero */
      .pred = { .x = 10, .y = -20, .ref_num = 1},
      .left = { .x =  0, .y =   0, .ref_num = 1},
      .up   = { .x =  1, .y =   0, .ref_num = 0},
      .mb_avail_flags = MB_LEFT_ | MB_UP_,
      .expect_mv = { .x =  10, .y = -20, .ref_num = 0},
      .expect_ret = false
    },
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    /* Always use mb (1, 1) for simplicity. Flag decides availability */
    const int mby = 1;
    const int mbx = 1;
    test_data_skip_mv test = test_cases[i];

    /* Override the default MVs with the ones given in the test case.
     * Only on the positions we know is checked. */
    int mbpos = mby * mbxmax + mbx;
    mvs[(mbpos - mbxmax) * NUM_MVS_MB + 12] = test.up;
    mvs[(mbpos - 1) * NUM_MVS_MB + 3] = test.left;

    // An unnecessary awkward test setup due to refactoring in code.
    // TODO: Make time to clean up test
    mv_t mv;
    int mvs_offset = (mby * mbxmax + mbx) * NUM_MVS_MB;
    bool avail_a = MB_LEFT (test.mb_avail_flags);
    bool avail_b = MB_UP (test.mb_avail_flags);
    int offset_a = avail_a ? -13 : 0;
    int offset_b = avail_b ? -(mbxmax * NUM_MVS_MB) + 12 : 0;
    const mv_t * mv_a = &mvs[mvs_offset + offset_a];
    const mv_t * mv_b = &mvs[mvs_offset + offset_b];

    bool ret = taa_h264_compute_skip_motion_vector (
      &test.pred, mv_a, mv_b, avail_a, avail_b, &mv);

    //bool ret = taa_h264_find_skip_motion_vector (
    //  &mvs[mvs_offset],
    //  stride,
    //  &test.pred,
    //  test.mb_avail_flags,
    //  &mv);
    FASSERT (ret == test.expect_ret);
    FASSERT (mv.x == test.expect_mv.x);
    FASSERT (mv.y == test.expect_mv.y);
    FASSERT (mv.ref_num == test.expect_mv.ref_num);
  }
  free (mvs);
}


typedef struct
{
  int partshape_y;
  int partshape_x;
  int block_num;
  int ref_num;
  int mby;
  int mbx;
  int mb_avail_flags;
  mv_t expect;
  mv_t expect_upper;
  mv_t expect_lower;
  mv_t expect_left;
  mv_t expect_right;
} test_data_mv_pred;

static void test_mv_pred_16x16 (void)
{
  int stride;
  mv_t * mvs = alloc_mvs_default_values (&stride, 3, 3);

  test_data_mv_pred test_cases[] = {
    {
      /* No MBs available */
      .partshape_y = 16,
      .partshape_x = 16,
      .ref_num = 0,
      .mby = 0,
      .mbx = 0,
      .mb_avail_flags = MB_NONE_,
      .expect = { .y = 0, .x = 0, .ref_num = 0}
    },
    {
      /* Only left MB is available */
      .partshape_y = 16,
      .partshape_x = 16,
      .ref_num = 0,
      .mby = 0,
      .mbx = 1,
      .mb_avail_flags = MB_LEFT_,
      .expect = { .y = 3, .x = 1003, .ref_num = 0}
    },
    {
      /* Only up MB is available */
      .partshape_y = 16,
      .partshape_x = 16,
      .ref_num = 0,
      .mby = 1,
      .mbx = 0,
      .mb_avail_flags = MB_UP_,
      .expect = { .y = 12, .x = 1012, .ref_num = 0}
    },
    {
      /* Up left and up right is available */
      .partshape_y = 16,
      .partshape_x = 16,
      .ref_num = 0,
      .mby = 1,
      .mbx = 1,
      .mb_avail_flags = MB_UPRIGHT_ | MB_UPLEFT_,
      /* Should choose only up right */
      .expect = { .y = 52, .x = 1052, .ref_num = 0}
    },
    {
      /* Only up left is available */
      .partshape_y = 16,
      .partshape_x = 16,
      .ref_num = 0,
      .mby = 1,
      .mbx = 1,
      .mb_avail_flags = MB_UPLEFT_,
      .expect = { .y = 15, .x = 1015, .ref_num = 0}
    },
    {
      /* Up and left is available */
      .partshape_y = 16,
      .partshape_x = 16,
      .ref_num = 0,
      .mby = 1,
      .mbx = 1,
      .mb_avail_flags = MB_UP_ | MB_LEFT_,
      .expect = { .y = 32, .x = 1032, .ref_num = 0}
    },
    {
      /* Up, up right and left is available */
      .partshape_y = 16,
      .partshape_x = 16,
      .ref_num = 0,
      .mby = 1,
      .mbx = 1,
      .mb_avail_flags = MB_UP_ | MB_LEFT_ | MB_UPRIGHT_,
      .expect = { .y = 52, .x = 1052, .ref_num = 0}
    },
  };


  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_mv_pred test = test_cases[i];
    int mvs_offset = test.mby * stride + test.mbx * NUM_MVS_MB;
    bool block_left    = test.mb_avail_flags & MB_LEFT_;
    bool block_up      = test.mb_avail_flags & MB_UP_;
    bool block_upright = test.mb_avail_flags & MB_UPRIGHT_;
    bool block_upleft  = test.mb_avail_flags & MB_UPLEFT_;
    int block_num = 0;
    mv_t res = taa_h264_predict_motion_vector (
      &mvs[mvs_offset], stride,
      block_num,
      test.partshape_y,
      test.partshape_x,
      test.ref_num,
      block_left,
      block_up,
      block_upright,
      block_upleft);

    FASSERT (res.y == test.expect.y);
    FASSERT (res.x == test.expect.x);
    FASSERT (res.ref_num == test.expect.ref_num);
  }

  free(mvs);
}



static void test_mv_pred_16x8 (void)
{
  int stride;
  mv_t * mvs = alloc_mvs_default_values (&stride, 3, 3);

  test_data_mv_pred test_cases[] = {
    {
      /* No neighbor MBs available */
      .partshape_y = 8,
      .partshape_x = 16,
      .ref_num = 0,
      .mby = 0,
      .mbx = 0,
      .mb_avail_flags = MB_NONE_,
      .expect_upper = { .y = 0, .x = 0, .ref_num = 0},
      .expect_lower = { .y = 4, .x = 1004, .ref_num = 0}
    },
    {
      /* Only left MB is available */
      .partshape_y = 8,
      .partshape_x = 16,
      .ref_num = 0,
      .mby = 0,
      .mbx = 1,
      .mb_avail_flags = MB_LEFT_,
      .expect_upper = { .y =  3, .x = 1003, .ref_num = 0},
      .expect_lower = { .y = 11, .x = 1011, .ref_num = 0}
    },
    {
      /* Only up MB is available */
      .partshape_y = 8,
      .partshape_x = 16,
      .ref_num = 0,
      .mby = 1,
      .mbx = 0,
      .mb_avail_flags = MB_UP_,
      .expect_upper = { .y = 12, .x = 1012, .ref_num = 0},
      .expect_lower = { .y = 64, .x = 1064, .ref_num = 0}
    },
    {
      /* Up left and up right is available */
      .partshape_y = 8,
      .partshape_x = 16,
      .ref_num = 0,
      .mby = 1,
      .mbx = 1,
      .mb_avail_flags = MB_UPRIGHT_ | MB_UPLEFT_,
      .expect_upper = { .y = 52, .x = 1052, .ref_num = 0},
      .expect_lower = { .y = 84, .x = 1084, .ref_num = 0},
    },
    {
      /* Only up left is available */
      .partshape_y = 8,
      .partshape_x = 16,
      .ref_num = 0,
      .mby = 1,
      .mbx = 1,
      .mb_avail_flags = MB_UPLEFT_,
      .expect_upper = { .y = 15, .x = 1015, .ref_num = 0},
      .expect_lower = { .y = 84, .x = 1084, .ref_num = 0},
    },
    {
      /* Up and left is available */
      .partshape_y = 8,
      .partshape_x = 16,
      .ref_num = 0,
      .mby = 1,
      .mbx = 1,
      .mb_avail_flags = MB_UP_ | MB_LEFT_,
      .expect_upper = { .y = 32, .x = 1032, .ref_num = 0},
      .expect_lower = { .y = 71, .x = 1071, .ref_num = 0}
    },
    {
      /* Up, up right and left is available */
      .partshape_y = 8,
      .partshape_x = 16,
      .ref_num = 0,
      .mby = 1,
      .mbx = 1,
      .mb_avail_flags = MB_UP_ | MB_LEFT_ | MB_UPRIGHT_,
      .expect_upper = { .y = 32, .x = 1032, .ref_num = 0},
      .expect_lower = { .y = 71, .x = 1071, .ref_num = 0}
    },
  };


  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_mv_pred test = test_cases[i];
    int mvs_offset = test.mby * stride + test.mbx * NUM_MVS_MB;
    bool mb_left    = test.mb_avail_flags & MB_LEFT_;
    bool mb_up      = test.mb_avail_flags & MB_UP_;
    bool mb_upright = test.mb_avail_flags & MB_UPRIGHT_;
    bool mb_upleft  = test.mb_avail_flags & MB_UPLEFT_;

    bool block_left = mb_left;
    bool block_up = mb_up;
    bool block_upright = mb_upright;
    bool block_upleft = mb_upleft;
    int block_num = 0;

    mv_t res_upper = taa_h264_predict_motion_vector (
      &mvs[mvs_offset], stride,
      block_num,
      test.partshape_y,
      test.partshape_x,
      test.ref_num,
      block_left,
      block_up,
      block_upright,
      block_upleft);

    block_num = 8;
    block_left = mb_left;
    block_up = true;
    block_upright = false;
    block_upleft = mb_left;

    mv_t res_lower = taa_h264_predict_motion_vector (
      &mvs[mvs_offset], stride,
      block_num,
      test.partshape_y,
      test.partshape_x,
      test.ref_num,
      block_left,
      block_up,
      block_upright,
      block_upleft);

    FASSERT (res_upper.y == test.expect_upper.y);
    FASSERT (res_upper.x == test.expect_upper.x);
    FASSERT (res_upper.ref_num == test.expect_upper.ref_num);

    FASSERT (res_lower.y == test.expect_lower.y);
    FASSERT (res_lower.x == test.expect_lower.x);
    FASSERT (res_lower.ref_num == test.expect_lower.ref_num);
  }

  free(mvs);
}


static void test_mv_pred_8x16 (void)
{
  int stride;
  mv_t * mvs = alloc_mvs_default_values (&stride, 3, 3);

  test_data_mv_pred test_cases[] = {
    {
      /* No neighbor MBs available */
      .partshape_y = 16,
      .partshape_x = 8,
      .ref_num = 0,
      .mby = 0,
      .mbx = 0,
      .mb_avail_flags = MB_NONE_,
      .expect_left  = { .y = 0, .x = 0, .ref_num = 0},
      .expect_right = { .y = 1, .x = 1001, .ref_num = 0}
    },
    {
      /* Only left MB is available */
      .partshape_y = 16,
      .partshape_x = 8,
      .ref_num = 0,
      .mby = 0,
      .mbx = 1,
      .mb_avail_flags = MB_LEFT_,
      .expect_left  = { .y =  3, .x = 1003, .ref_num = 0},
      .expect_right = { .y = 21, .x = 1021, .ref_num = 0}
    },
    {
      /* Only up MB is available */
      .partshape_y = 16,
      .partshape_x = 8,
      .ref_num = 0,
      .mby = 1,
      .mbx = 0,
      .mb_avail_flags = MB_UP_,
      .expect_left  = { .y = 12, .x = 1012, .ref_num = 0},
      .expect_right = { .y = 13, .x = 1013, .ref_num = 0}
    },
    {
      /* Up left and up right is available */
      .partshape_y = 16,
      .partshape_x = 8,
      .ref_num = 0,
      .mby = 1,
      .mbx = 1,
      .mb_avail_flags = MB_UPRIGHT_ | MB_UPLEFT_,
      .expect_left  = { .y = 15, .x = 1015, .ref_num = 0},
      .expect_right = { .y = 52, .x = 1052, .ref_num = 0},
    },
    {
      /* Only up left is available */
      .partshape_y = 16,
      .partshape_x = 8,
      .ref_num = 0,
      .mby = 1,
      .mbx = 1,
      .mb_avail_flags = MB_UPLEFT_,
      .expect_left  = { .y = 15, .x = 1015, .ref_num = 0},
      .expect_right = { .y = 81, .x = 1081, .ref_num = 0},
    },
    {
      /* Up and left is available */
      .partshape_y = 16,
      .partshape_x = 8,
      .ref_num = 0,
      .mby = 1,
      .mbx = 1,
      .mb_avail_flags = MB_UP_ | MB_LEFT_,
      .expect_left  = { .y = 63, .x = 1063, .ref_num = 0},
      .expect_right = { .y = 33, .x = 1033, .ref_num = 0}
    },
    {
      /* Up, up right and left is available */
      .partshape_y = 16,
      .partshape_x = 8,
      .ref_num = 0,
      .mby = 1,
      .mbx = 1,
      .mb_avail_flags = MB_UP_ | MB_LEFT_ | MB_UPRIGHT_,
      .expect_left  = { .y = 63, .x = 1063, .ref_num = 0},
      .expect_right = { .y = 52, .x = 1052, .ref_num = 0}
    },
  };


  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_mv_pred test = test_cases[i];
    int mvs_offset = test.mby * stride + test.mbx * NUM_MVS_MB;
    bool mb_left    = test.mb_avail_flags & MB_LEFT_;
    bool mb_up      = test.mb_avail_flags & MB_UP_;
    bool mb_upright = test.mb_avail_flags & MB_UPRIGHT_;
    bool mb_upleft  = test.mb_avail_flags & MB_UPLEFT_;

    bool block_left = mb_left;
    bool block_up = mb_up;
    bool block_upright = mb_up;
    bool block_upleft = mb_upleft;
    int block_num = 0;

    mv_t res_left  = taa_h264_predict_motion_vector (
      &mvs[mvs_offset], stride,
      block_num,
      test.partshape_y,
      test.partshape_x,
      test.ref_num,
      block_left,
      block_up,
      block_upright,
      block_upleft);

    block_num = 2;
    block_left = true;
    block_up = mb_up;
    block_upright = mb_upright;
    block_upleft = mb_up;

    mv_t res_right = taa_h264_predict_motion_vector (
      &mvs[mvs_offset], stride,
      block_num,
      test.partshape_y,
      test.partshape_x,
      test.ref_num,
      block_left,
      block_up,
      block_upright,
      block_upleft);

    FASSERT (res_left.y == test.expect_left.y);
    FASSERT (res_left.x == test.expect_left.x);
    FASSERT (res_left.ref_num == test.expect_left.ref_num);

    FASSERT (res_right.y == test.expect_right.y);
    FASSERT (res_right.x == test.expect_right.x);
    FASSERT (res_right.ref_num == test.expect_right.ref_num);
  }


  /* Case not possible to test with default mvs values */
  /* Up and up right is available and in same MB. Median becomes upright. */
  mv_t tmp = mvs[14];
  mv_t expect = { .x = 1, .y = 2, .ref_num = 0};
  mvs[14] = expect;
  mv_t res = taa_h264_predict_motion_vector (
    &mvs[3 * NUM_MVS_MB], stride,
    0,
    16,
    8,
    0,
    false,
    true,
    true,
    false);
  FASSERT (res.y == expect.y);
  FASSERT (res.x == expect.x);
  FASSERT (res.ref_num == expect.ref_num);
  mvs[14] = tmp;

  free(mvs);
}


static void mv_pred_loop_tests (
  test_data_mv_pred * tests,
  int                 num_tests,
  mv_t *              mvs,
  int                 mvs_stride)
{
  for (int i = 0; i < num_tests; i++)
  {
    test_data_mv_pred * test = &tests[i];
    int mvs_offset = test->mby * mvs_stride + test->mbx * NUM_MVS_MB;
    bool mb_left    = test->mb_avail_flags & MB_LEFT_;
    bool mb_up      = test->mb_avail_flags & MB_UP_;
    bool mb_upright = test->mb_avail_flags & MB_UPRIGHT_;
    bool mb_upleft  = test->mb_avail_flags & MB_UPLEFT_;

    int left_flags;
    int up_flags;
    int upright_flags;
    int upleft_flags;

    int blocknum_top_left = test->block_num;
    int blocknum_top_right = blocknum_top_left + test->partshape_x / 4 - 1;

    taa_h264_set_4x4_blocks_neighbor_flags (
      mb_left, mb_up, mb_upright, mb_upleft,
      &left_flags, &up_flags, &upright_flags, &upleft_flags);

    bool block_left = NEIGHBOR_BLOCK_AVAILABLE (left_flags, blocknum_top_left);
    bool block_up = NEIGHBOR_BLOCK_AVAILABLE (up_flags, blocknum_top_left);
    bool block_upright = NEIGHBOR_BLOCK_AVAILABLE (upright_flags, blocknum_top_right);
    bool block_upleft = NEIGHBOR_BLOCK_AVAILABLE (upleft_flags, blocknum_top_left);

    mv_t res_left  = taa_h264_predict_motion_vector (
      &mvs[mvs_offset],
      mvs_stride,
      test->block_num,
      test->partshape_y,
      test->partshape_x,
      test->ref_num,
      block_left,
      block_up,
      block_upright,
      block_upleft);

    FASSERT (res_left.y == test->expect.y);
    FASSERT (res_left.x == test->expect.x);
    FASSERT (res_left.ref_num == test->expect.ref_num);
  }
}

static void test_mv_pred_4x4_internal_blocks (void)
{
  int stride;
  mv_t * mvs = alloc_mvs_default_values (&stride, 3, 3);

  test_data_mv_pred test_cases[] = {
    {
      /* No neighbor MBs available, no blocks avail */
      .partshape_y = 4,
      .partshape_x = 4,
      .block_num = 0,
      .ref_num = 0,
      .mby = 0,
      .mbx = 0,
      .mb_avail_flags = MB_NONE_,
      .expect  = { .y = 0, .x = 0, .ref_num = 0},
    },
    {
      /* No neighbor MBs available, left block avail */
      .partshape_y = 4,
      .partshape_x = 4,
      .block_num = 1,
      .ref_num = 0,
      .mby = 0,
      .mbx = 0,
      .mb_avail_flags = MB_NONE_,
      .expect  = { .y = 0, .x = 1000, .ref_num = 0},
    },
    {
      /* No neighbor MBs available, left block avail */
      .partshape_y = 4,
      .partshape_x = 4,
      .block_num = 3,
      .ref_num = 0,
      .mby = 0,
      .mbx = 0,
      .mb_avail_flags = MB_NONE_,
      .expect  = { .y = 2, .x = 1002, .ref_num = 0},
    },
    {
      /* No neighbor MBs available, top and upright blocks avail */
      .partshape_y = 4,
      .partshape_x = 4,
      .block_num = 4,
      .ref_num = 0,
      .mby = 0,
      .mbx = 0,
      .mb_avail_flags = MB_NONE_,
      .expect  = { .y = 0, .x = 1000, .ref_num = 0},
    },
    {
      /* No neighbor MBs available, top and left blocks avail */
      .partshape_y = 4,
      .partshape_x = 4,
      .block_num = 5,
      .ref_num = 0,
      .mby = 0,
      .mbx = 0,
      .mb_avail_flags = MB_NONE_,
      .expect  = { .y = 1, .x = 1001, .ref_num = 0},
    },
    {
      /* No neighbor MBs available, all blocks avail */
      .partshape_y = 4,
      .partshape_x = 4,
      .block_num = 9,
      .ref_num = 0,
      .mby = 0,
      .mbx = 0,
      .mb_avail_flags = MB_NONE_,
      .expect  = { .y = 6, .x = 1006, .ref_num = 0},
    },
  };


  size_t n = sizeof(test_cases) / sizeof(test_cases[0]);
  mv_pred_loop_tests (test_cases, n, mvs, stride);

  free(mvs);
}


static void test_mv_pred_4x4_left_blocks (void)
{
  int stride;
  mv_t * mvs = alloc_mvs_default_values (&stride, 3, 3);

  test_data_mv_pred test_cases[] = {
    {
      /* Left MB available */
      .partshape_y = 4,
      .partshape_x = 4,
      .block_num = 0,
      .ref_num = 0,
      .mby = 0,
      .mbx = 1,
      .mb_avail_flags = MB_LEFT_,
      .expect  = { .y = 3, .x = 1003, .ref_num = 0},
    },
    {
      /* Left MB available, up and upright block avail */
      .partshape_y = 4,
      .partshape_x = 4,
      .block_num = 8,
      .ref_num = 0,
      .mby = 0,
      .mbx = 1,
      .mb_avail_flags = MB_LEFT_,
      .expect  = { .y = 24, .x = 1024, .ref_num = 0},
    },
    {
      /* Top-left MB available */
      .partshape_y = 4,
      .partshape_x = 4,
      .block_num = 0,
      .ref_num = 0,
      .mby = 1,
      .mbx = 1,
      .mb_avail_flags = MB_UPLEFT_,
      .expect  = { .y = 15, .x = 1015, .ref_num = 0},
    },
    {
      /* Top-left and top MB available */
      .partshape_y = 4,
      .partshape_x = 4,
      .block_num = 0,
      .ref_num = 0,
      .mby = 1,
      .mbx = 1,
      .mb_avail_flags = MB_UPLEFT_ | MB_UP_,
      .expect  = { .y = 32, .x = 1032, .ref_num = 0},
    },
  };

  size_t n = sizeof(test_cases) / sizeof(test_cases[0]);
  mv_pred_loop_tests (test_cases, n, mvs, stride);
  free (mvs);
}

static void test_mv_pred_4x4_top_blocks (void)
{
  int stride;
  mv_t * mvs = alloc_mvs_default_values (&stride, 3, 3);

  test_data_mv_pred test_cases[] = {
    {
      /* Top MB available */
      .partshape_y = 4,
      .partshape_x = 4,
      .block_num = 0,
      .ref_num = 0,
      .mby = 1,
      .mbx = 0,
      .mb_avail_flags = MB_UP_,
      .expect  = { .y = 12, .x = 1012, .ref_num = 0},
    },
    {
      /* Top MB available, left block avail */
      .partshape_y = 4,
      .partshape_x = 4,
      .block_num = 1,
      .ref_num = 0,
      .mby = 1,
      .mbx = 0,
      .mb_avail_flags = MB_UP_,
      .expect  = { .y = 14, .x = 1014, .ref_num = 0},
    },
    {
      /* Top MB available, left block avail, upright not */
      .partshape_y = 4,
      .partshape_x = 4,
      .block_num = 3,
      .ref_num = 0,
      .mby = 1,
      .mbx = 0,
      .mb_avail_flags = MB_UP_,
      .expect  = { .y = 15, .x = 1015, .ref_num = 0},
    },
    {
      /* Top and top-right MB available, left block avail */
      .partshape_y = 4,
      .partshape_x = 4,
      .block_num = 3,
      .ref_num = 0,
      .mby = 1,
      .mbx = 0,
      .mb_avail_flags = MB_UP_ | MB_UPRIGHT_,
      .expect  = { .y = 32, .x = 1032, .ref_num = 0},
    },
    {
      /* Top-right MB available, left block avail */
      .partshape_y = 4,
      .partshape_x = 4,
      .block_num = 3,
      .ref_num = 0,
      .mby = 1,
      .mbx = 0,
      .mb_avail_flags = MB_UPRIGHT_,
      .expect  = { .y = 32, .x = 1032, .ref_num = 0},
    },
  };

  size_t n = sizeof(test_cases) / sizeof(test_cases[0]);
  mv_pred_loop_tests (test_cases, n, mvs, stride);
  free (mvs);
}


static void test_mv_pred_4x4_right_blocks (void)
{
  int stride;
  mv_t * mvs = alloc_mvs_default_values (&stride, 3, 3);

  test_data_mv_pred test_cases[] = {
    {
      /* No neighbor MBs available, left and top blocks avail */
      .partshape_y = 4,
      .partshape_x = 4,
      .block_num = 7,
      .ref_num = 0,
      .mby = 1,
      .mbx = 1,
      .mb_avail_flags = MB_NONE_,
      .expect  = { .y = 83, .x = 1083, .ref_num = 0},
    },
    {
      /* All neighbor MBs available, left and top blocks avail */
      .partshape_y = 4,
      .partshape_x = 4,
      .block_num = 7,
      .ref_num = 0,
      .mby = 1,
      .mbx = 1,
      .mb_avail_flags = MB_LEFT_ | MB_UP_ | MB_UPRIGHT_ | MB_UPLEFT_,
      .expect  = { .y = 83, .x = 1083, .ref_num = 0},
    },
  };

  size_t n = sizeof(test_cases) / sizeof(test_cases[0]);
  mv_pred_loop_tests (test_cases, n, mvs, stride);
  free (mvs);
}

static void test_mv_pred_8x4 (void)
{
  int stride;
  mv_t * mvs = alloc_mvs_default_values (&stride, 3, 3);

  test_data_mv_pred test_cases[] = {
    {
      /* Top MB available, no blocks avail */
      .partshape_y = 4,
      .partshape_x = 8,
      .block_num = 0,
      .ref_num = 0,
      .mby = 1,
      .mbx = 1,
      .mb_avail_flags = MB_UP_,
      .expect  = { .y = 32, .x = 1032, .ref_num = 0},
    },
    {
      /* Top and top-left MB available, no blocks avail */
      /* Top left should not matter since top right is available in top MB */
      .partshape_y = 4,
      .partshape_x = 8,
      .block_num = 0,
      .ref_num = 0,
      .mby = 1,
      .mbx = 1,
      .mb_avail_flags = MB_UP_ | MB_UPLEFT_,
      .expect  = { .y = 32, .x = 1032, .ref_num = 0},
    },
    {
      /* Left and top MB available, no blocks avail */
      .partshape_y = 4,
      .partshape_x = 8,
      .block_num = 0,
      .ref_num = 0,
      .mby = 1,
      .mbx = 1,
      .mb_avail_flags = MB_LEFT_ | MB_UP_,
      .expect  = { .y = 34, .x = 1034, .ref_num = 0},
    },
    {
      /* Top MB available, left block avail */
      .partshape_y = 4,
      .partshape_x = 8,
      .block_num = 2,
      .ref_num = 0,
      .mby = 1,
      .mbx = 0,
      .mb_avail_flags = MB_UP_,
      .expect  = { .y = 14, .x = 1014, .ref_num = 0},
    },
    {
      /* Top and top-right MB available, left block avail */
      .partshape_y = 4,
      .partshape_x = 8,
      .block_num = 2,
      .ref_num = 0,
      .mby = 1,
      .mbx = 1,
      .mb_avail_flags = MB_UP_ | MB_UPRIGHT_,
      .expect  = { .y = 52, .x = 1052, .ref_num = 0},
    },
    {
      /* No MB avail, top and top-left and top left */
      .partshape_y = 4,
      .partshape_x = 8,
      .block_num = 10,
      .ref_num = 0,
      .mby = 0,
      .mbx = 0,
      .mb_avail_flags = MB_NONE_,
      .expect  = { .y = 6, .x = 1006, .ref_num = 0},
    },
  };

  size_t n = sizeof(test_cases) / sizeof(test_cases[0]);
  mv_pred_loop_tests (test_cases, n, mvs, stride);
  free (mvs);
}



typedef struct
{
  int mby;
  int mbx;
  int by;
  int bx;
  int expect;
  int expect_u;
  int expect_v;
} test_data_pred_nnz;

static void test_predict_nonzero_coeffs_luma (void)
{
  /* We use the same input array for all the tests, but change the
   * "position" of where we predict the nonzero coeffs from. */
  const int mbxmax = 2;
  const int mbymax = 2;
  uint8_t num_coeffs[mbxmax * mbymax * 16];

  for (int i = 0; i < mbymax; i++)
  {
    for (int j = 0; j < mbxmax; j++)
    {
      for (int k = 0; k < 16; k++)
      {
        /* Offset of 100 per row and offset of 20 per column for the */
        /* value. */
        int value = i * 100 + j * 20 + k;
        int index = i * 16 * mbxmax + j * 16 + k;
        num_coeffs[index] = (uint8_t) value;
      }
    }
  }

  test_data_pred_nnz test_cases[] = {
    {
      .mby = 0,
      .mbx = 0,
      .by = 0,
      .bx = 0,
      .expect = 0
    },
    {
      .mby = 0,
      .mbx = 0,
      .by = 0,
      .bx = 2,
      .expect = 1
    },
    {
      .mby = 0,
      .mbx = 0,
      .by = 2,
      .bx = 0,
      .expect = 4
    },
    {
      .mby = 0,
      .mbx = 0,
      .by = 1,
      .bx = 1,
      .expect = (1 + 4 + 1) / 2
    },
    {
      .mby = 0,
      .mbx = 0,
      .by = 3,
      .bx = 3,
      .expect = (11 + 14 + 1) / 2
    },
    {
      .mby = 0,
      .mbx = 1,
      .by = 0,
      .bx = 0,
      .expect = 3
    },
    {
      .mby = 0,
      .mbx = 1,
      .by = 1,
      .bx = 0,
      .expect = (7 + 20 + 1) / 2
    },
    {
      .mby = 1,
      .mbx = 0,
      .by = 0,
      .bx = 0,
      .expect = 12
    },
    {
      .mby = 1,
      .mbx = 0,
      .by = 0,
      .bx = 3,
      .expect = (102 + 15 + 1) / 2
    },
    {
      .mby = 1,
      .mbx = 1,
      .by = 0,
      .bx = 0,
      .expect = (103 + 32 + 1) / 2
    },
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_pred_nnz * test = &test_cases[i];

    int nc_stride = mbxmax * NUM_4x4_BLOCKS_Y;
    int mb_offset = (test->mby * mbxmax + test->mbx) * NUM_4x4_BLOCKS_Y;
    int raster_idx = test->by * 4 + test->bx;
    uint8_t * num_coeffs_ptr = &num_coeffs [mb_offset + raster_idx];

    bool first_col = test->bx == 0;
    bool first_row = test->by == 0;
    bool mb_left = test->mbx != 0;
    bool mb_top  = test->mby != 0;
    int result = taa_h264_predict_nonzero_coeffs (
      num_coeffs_ptr,
      nc_stride,
      first_col,
      first_row,
      !first_col || mb_left,
      !first_row || mb_top,
      false);
    FASSERT (result == test->expect);
  }
}

static void test_predict_nonzero_coeffs_chroma (void)
{
  /* We use the same input array for all the tests, but the "position" of
   * where we predict the nonzero coeffs from. */
  const int mbxmax = 2;
  const int mbymax = 2;
  uint8_t num_coeffs[mbxmax * mbymax * NUM_4x4_BLOCKS_UV * 2];

  for (int i = 0; i < mbymax; i++)
  {
    for (int j = 0; j < mbxmax; j++)
    {
      for (int uv = 0; uv < 2; uv++)
      {
        for (int k = 0; k < NUM_4x4_BLOCKS_UV; k++)
        {
          /* Offset of 20 per row and offset of 10 per column for the */
          /* value. U start from 0 and up, V from 100 */
          int value = uv * 100 + i * 20 + j * 10 + k;
          int index = i * NUM_4x4_BLOCKS_UV * 2 * mbxmax +
                      j * NUM_4x4_BLOCKS_UV * 2 +
                      uv * NUM_4x4_BLOCKS_UV + k;
          num_coeffs[index] = (uint8_t) value;
        }
      }
    }
  }

  test_data_pred_nnz test_cases[] = {
    {
      .mby = 0,
      .mbx = 0,
      .by = 0,
      .bx = 0,
      .expect_u = 0,
      .expect_v = 0,
    },
    {
      .mby = 0,
      .mbx = 0,
      .by = 0,
      .bx = 1,
      .expect_u = 0,
      .expect_v = 100,
    },
    {
      .mby = 0,
      .mbx = 0,
      .by = 1,
      .bx = 0,
      .expect_u = 0,
      .expect_v = 100
    },
    {
      .mby = 0,
      .mbx = 0,
      .by = 1,
      .bx = 1,
      .expect_u = (1 + 2 + 1) / 2,
      .expect_v = (101 + 102 + 1) / 2
    },
    {
      .mby = 0,
      .mbx = 1,
      .by = 0,
      .bx = 0,
      .expect_u = 1,
      .expect_v = 101,
    },
    {
      .mby = 0,
      .mbx = 1,
      .by = 1,
      .bx = 0,
      .expect_u = (3 + 10 + 1) / 2,
      .expect_v = (103 + 110 + 1) / 2
    },
    {
      .mby = 1,
      .mbx = 0,
      .by = 0,
      .bx = 0,
      .expect_u = 2,
      .expect_v = 102
    },
    {
      .mby = 1,
      .mbx = 0,
      .by = 0,
      .bx = 1,
      .expect_u = (20 + 3 + 1) / 2,
      .expect_v = (120 + 103 + 1) / 2
    },
    {
      .mby = 1,
      .mbx = 1,
      .by = 0,
      .bx = 0,
      .expect_u = (21 + 12 + 1) / 2,
      .expect_v = (121 + 112 + 1) / 2
    },
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_pred_nnz * test = &test_cases[i];

    int mb_offset = (test->mby * mbxmax + test->mbx) * NUM_4x4_BLOCKS_UV * 2;
    int raster_idx = test->by * 2 + test->bx;
    int block_offset_u = raster_idx;
    int block_offset_v = block_offset_u + NUM_4x4_BLOCKS_UV;

    int nc_stride = mbxmax * NUM_4x4_BLOCKS_UV * 2;
    uint8_t * num_coeffs_u = &num_coeffs [mb_offset + block_offset_u];
    uint8_t * num_coeffs_v = &num_coeffs [mb_offset + block_offset_v];

    bool first_col = test->bx == 0;
    bool first_row = test->by == 0;
    bool mb_left = test->mbx != 0;
    bool mb_top  = test->mby != 0;

    int result_u = taa_h264_predict_nonzero_coeffs (num_coeffs_u,
                                                    nc_stride,
                                                    first_col,
                                                    first_row,
                                                    mb_left || !first_col,
                                                    mb_top || !first_row,
                                                    true);

    int result_v = taa_h264_predict_nonzero_coeffs (num_coeffs_v,
                                                    nc_stride,
                                                    first_col,
                                                    first_row,
                                                    mb_left || !first_col,
                                                    mb_top || !first_row,
                                                    true);

    FASSERT (result_u == test->expect_u);
    FASSERT (result_v == test->expect_v);
  }
}

void test_com_prediction (void)
{
  test_predict_nonzero_coeffs_luma();
  test_predict_nonzero_coeffs_chroma();

  /* Don't bother with testing 8x8 and 4x8 since 8x8 will give the same
   * results as 8x4 and 4x8 yields the same as 4x4. */
  test_skip_mv ();
  test_mv_pred_16x16 ();
  test_mv_pred_16x8 ();
  test_mv_pred_8x16 ();
  test_mv_pred_8x4 ();
  test_mv_pred_4x4_internal_blocks ();
  test_mv_pred_4x4_left_blocks ();
  test_mv_pred_4x4_top_blocks ();
  test_mv_pred_4x4_right_blocks ();
}
