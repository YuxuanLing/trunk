#include "enc_gop.h"
#include "fractaltest.h"

typedef struct
{
  uint8_t levels;
  uint8_t size;
  gop_t expect_gop;
} test_data_reset_gop;

static void test_reset_gop (void)
{
  test_data_reset_gop test_cases [] = {
    {
      .levels = 1,
      .size = 1,
      .expect_gop = {
        .levels = 1,
        .period = { 1},
        .count = { 1}
      },
    },
    {
      .levels = 2,
      .size = 2,
      .expect_gop = {
        .levels = 2,
        .period = { 2, 1},
        .count = { 2, 1}
      },
    },
    {
      .levels = 3,
      .size = 4,
      .expect_gop = {
        .levels = 3,
        .period = { 4, 2, 1},
        .count = { 4, 2, 1}
      },
    },
    {
      .levels = 8,
      .size = 1 << (8 - 1),
      .expect_gop = {
        .levels = 8,
        .period = { 128, 64, 32, 16, 8, 4, 2, 1},
        .count = { 128, 64, 32, 16, 8, 4, 2, 1}
      },
    },
    {
      .levels = 3,
      .size = 6,
      .expect_gop = {
        .levels = 3,
        .period = { 6, 2, 1},
        .count = { 6, 2, 1}
      },
    },
    {
      .levels = 3,
      .size = 3,
      .expect_gop = {
        .levels = 3,
        .period = { 3, 2, 1},
        .count = { 3, 2, 1}
      },
    },
    {
      /* Invalid case, exceeds maximum number of levels */
      .levels = 9,
      .size = 200,
      .expect_gop = {
        .levels = 8,
        .period = { 200, 64, 32, 16, 8, 4, 2, 1},
        .count = { 200, 64, 32, 16, 8, 4, 2, 1}
      }
    },
    {
      /* Invalid case, exceeds minimum number of levels */
      .levels = 0,
      .size = 1,
      .expect_gop = {
        .levels = 1,
        .period = { 1},
        .count = { 1}
      },
    },
    {
      /* Invalid case, size doesn't make sence */
      .levels = 1,
      .size = 7,
      .expect_gop = {
        .levels = 1,
        .period = { 1},
        .count = { 1}
      },
    }
  };

  int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_reset_gop * test = &test_cases[i];
    gop_t gop;

    taa_h264_reset_gop (&gop, test->size, test->levels);

    FASSERT (gop.levels == test->expect_gop.levels);
    for (int j = 0; j < gop.levels; j++)
    {
      FASSERT (gop.period[j] == test->expect_gop.period[j]);
      FASSERT (gop.count[j] == test->expect_gop.count[j]);
    }
  }
}


typedef struct
{
  gop_t gop;
  int expect_tid[16];
} test_data_update_gop;

static void test_update_gop (void)
{
  test_data_update_gop test_cases [] = {
    {
      .gop = {
        .levels = 1,
        .period = { 1},
        .count = { 1}
      },
      .expect_tid = { 0, 0, 0, 0, -1},
    },
    {
      .gop = {
        .levels = 3,
        .period = { 4, 2, 1},
        .count = { 4, 2, 1}
      },
      .expect_tid = { 2, 1, 2, 0, 2, 1, 2, 0, -1},
    },
  };

  int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_update_gop * test = &test_cases[i];

    for (int j = 0; test->expect_tid[j] != -1; j++)
    {
      int tid = taa_h264_update_gop (&test->gop);
      FASSERT (tid == test->expect_tid[j]);
    }
  }
}

void test_enc_gop (void)
{
  test_reset_gop ();
  test_update_gop ();
}
