#include "com_deblock.c"
#include "fractaltest.h"

#define NUM_MVS_MB 16

typedef struct
{
  mbinfo_store_t mb_curr;
  mbinfo_store_t mb_left;
  mbinfo_store_t mb_top;
  mv_t motion_vectors[NUM_MVS_MB * 4];
  mb_deblock_t expect;
} test_data_compute_strength;

static void test_compute_strength_intra (void)
{
  test_data_compute_strength test_cases[] = {
    {
      .mb_curr = {
        .flags = MB_NONE_ | MB_TYPE_INTRA_,
        .cbp_blk = 0,
        .qp = 28,
      },
      .expect = {
        .vert_strength1 = 0x0,
        .vert_strength2 = 0xeeee,
        .horz_strength1 = 0x0,
        .horz_strength2 = 0xfff0,
        .do_filter = 0xfff0eeee,
        .curr_qp = 28,
        .curr_qpc = 28,
      }
    },
    {
      .mb_curr = {
        .flags = MB_NONE_ | MB_TYPE_INTRA_,
        .cbp_blk = 0xcafe,
        .qp = 51,
      },
      .expect = {
        .vert_strength1 = 0x0,
        .vert_strength2 = 0xeeee,
        .horz_strength1 = 0x0,
        .horz_strength2 = 0xfff0,
        .do_filter = 0xfff0eeee,
        .curr_qp = 51,
        .curr_qpc = 39,
      }
    },
    {
      .mb_curr = {
        .flags = MB_LEFT_,
        .cbp_blk = 0x0,
        .qp = 51,
      },
      .mb_left = {
        .flags = MB_NONE_ | MB_TYPE_INTRA_,
        .cbp_blk = 0x0,
        .qp = 51,
      },
      .expect = {
        .vert_strength1 = 0x0,
        .vert_strength2 = 0x1111,
        .horz_strength1 = 0x0,
        .horz_strength2 = 0x0,
        .do_filter = 0x00001111,
        .curr_qp = 51,
        .curr_qpc = 39,
      }
    },
    {
      .mb_curr = {
        .flags = MB_UP_,
        .cbp_blk = 0x0,
        .qp = 51,
      },
      .mb_top = {
        .flags = MB_NONE_ | MB_TYPE_INTRA_,
        .cbp_blk = 0x0,
        .qp = 51,
      },
      .expect = {
        .vert_strength1 = 0x0,
        .vert_strength2 = 0x0,
        .horz_strength1 = 0x0,
        .horz_strength2 = 0x000f,
        .do_filter = 0x000f0000,
        .curr_qp = 51,
        .curr_qpc = 39,
      }
    },
  };

  const int mbxmax = 2;
  const int mbymax = 2;
  mbinfo_store_t mb_stored[mbxmax * mbymax];

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_compute_strength * test = &test_cases[i];
    mb_deblock_t result;

    mb_stored[0 * mbxmax + 1] = test->mb_top;
    mb_stored[1 * mbxmax + 0] = test->mb_left;
    mb_stored[1 * mbxmax + 1] = test->mb_curr;

    mbinfo_store_t * curr_mb_stored = &mb_stored[1 * mbxmax + 1];
    mv_t * motion_vectors_ptr = &test->motion_vectors[(1 * mbxmax + 1) * NUM_MVS_MB];

    taa_h264_mb_compute_strength (curr_mb_stored, motion_vectors_ptr,  mbxmax, &result);

#if 0
    fprintf (stderr, "---\n");
    fprintf (stderr, "vert1: 0x%04x\n", result.vert_strength1);
    fprintf (stderr, "vert2: 0x%04x\n", result.vert_strength2);
    fprintf (stderr, "horz1: 0x%04x\n", result.horz_strength1);
    fprintf (stderr, "horz2: 0x%04x\n", result.horz_strength2);
    fprintf (stderr, "filte: 0x%08x\n", result.do_filter);
#endif

    FASSERT (result.do_filter == test->expect.do_filter);
    FASSERT (result.vert_strength1 == test->expect.vert_strength1);
    FASSERT (result.vert_strength2 == test->expect.vert_strength2);
    FASSERT (result.horz_strength1 == test->expect.horz_strength1);
    FASSERT (result.horz_strength2 == test->expect.horz_strength2);
    FASSERT (result.curr_qp == test->expect.curr_qp);
    FASSERT (result.curr_qpc == test->expect.curr_qpc);
  }
}



static void test_compute_strength_cbp_blk (void)
{
  test_data_compute_strength test_cases[] = {
    {
      .mb_curr = {
        .flags = MB_NONE_,
        .cbp_blk = 0xffff,
        .qp = 28,
      },
      .expect = {
        .vert_strength1 = 0xeeee,
        .vert_strength2 = 0x0,
        .horz_strength1 = 0xfff0,
        .horz_strength2 = 0x0,
        .do_filter = 0xfff0eeee,
        .curr_qp = 28,
        .curr_qpc = 28,
      }
    },
    {
      .mb_curr = {
        .flags = MB_NONE_,
        .cbp_blk = 0x0001,
        .qp = 28,
      },
      .expect = {
        .vert_strength1 = 0x0002,
        .vert_strength2 = 0x0,
        .horz_strength1 = 0x0010,
        .horz_strength2 = 0x0,
        .do_filter = 0x00100002,
        .curr_qp = 28,
        .curr_qpc = 28,
      }
    },
    {
      .mb_curr = {
        .flags = MB_NONE_,
        .cbp_blk = 0x8421,
        .qp = 28,
      },
      .expect = {
        .vert_strength1 = 0x8c62,
        .vert_strength2 = 0x0,
        .horz_strength1 = 0xc630,
        .horz_strength2 = 0x0,
        .do_filter = 0xc6308c62,
        .curr_qp = 28,
        .curr_qpc = 28,
      }
    },
    {
      .mb_curr = {
        .flags = MB_LEFT_,
        .cbp_blk = 0x0,
        .qp = 28,
      },
      .mb_left = {
        .flags = MB_NONE_,
        .cbp_blk = 0x0808,
        .qp = 28,
      },
      .expect = {
        .vert_strength1 = 0x0101,
        .vert_strength2 = 0x0,
        .horz_strength1 = 0x0,
        .horz_strength2 = 0x0,
        .do_filter = 0x00000101,
        .curr_qp = 28,
        .curr_qpc = 28,
      }
    },
    {
      .mb_curr = {
        .flags = MB_UP_,
        .cbp_blk = 0x0,
        .qp = 28,
      },
      .mb_top = {
        .flags = MB_NONE_,
        .cbp_blk = 0xa000,
        .qp = 28,
      },
      .expect = {
        .vert_strength1 = 0x0,
        .vert_strength2 = 0x0,
        .horz_strength1 = 0x000a,
        .horz_strength2 = 0x0,
        .do_filter = 0x000a0000,
        .curr_qp = 28,
        .curr_qpc = 28,
      }
    },
    {
      .mb_curr = {
        .flags = MB_UP_,
        .cbp_blk = 0x0,
        .qp = 28,
      },
      .mb_top = {
        .flags = MB_NONE_ | MB_TYPE_INTRA_,
        .cbp_blk = 0xa000,
        .qp = 28,
      },
      .expect = {
        .vert_strength1 = 0x0,
        .vert_strength2 = 0x0,
        .horz_strength1 = 0x0000,
        .horz_strength2 = 0x000f,
        .do_filter = 0x000f0000,
        .curr_qp = 28,
        .curr_qpc = 28,
      }
    },
  };

  const int mbxmax = 2;
  const int mbymax = 2;
  mbinfo_store_t mb_stored[mbxmax * mbymax];

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_compute_strength * test = &test_cases[i];
    mb_deblock_t result;

    mb_stored[0 * mbxmax + 1] = test->mb_top;
    mb_stored[1 * mbxmax + 0] = test->mb_left;
    mb_stored[1 * mbxmax + 1] = test->mb_curr;

    mbinfo_store_t * curr_mb_stored = &mb_stored[1 * mbxmax + 1];
    mv_t * motion_vectors_ptr = &test->motion_vectors[(1 * mbxmax + 1) * NUM_MVS_MB];

    taa_h264_mb_compute_strength (curr_mb_stored, motion_vectors_ptr, mbxmax, &result);

#if 0
    fprintf (stderr, "---\n");
    fprintf (stderr, "vert1: 0x%04x\n", result.vert_strength1);
    fprintf (stderr, "vert2: 0x%04x\n", result.vert_strength2);
    fprintf (stderr, "horz1: 0x%04x\n", result.horz_strength1);
    fprintf (stderr, "horz2: 0x%04x\n", result.horz_strength2);
    fprintf (stderr, "filte: 0x%08x\n", result.do_filter);
#endif

// Macros to check only the blocks that are filtered according to do_filter
#define EQUAL_VERT_STRENGTH(s1, s2, m) ((s1 & (m & 0xffff)) == (s2 & (m & 0xffff)))
#define EQUAL_HORZ_STRENGTH(s1, s2, m) ((s1 & (m >> 16)) == (s2 & (m >> 16)))

    FASSERT (result.do_filter == test->expect.do_filter);
    FASSERT (EQUAL_VERT_STRENGTH(result.vert_strength1, test->expect.vert_strength1, result.do_filter));
    FASSERT (EQUAL_VERT_STRENGTH(result.vert_strength2, test->expect.vert_strength2, result.do_filter));
    FASSERT (EQUAL_HORZ_STRENGTH(result.horz_strength1, test->expect.horz_strength1, result.do_filter));
    FASSERT (EQUAL_HORZ_STRENGTH(result.horz_strength2, test->expect.horz_strength2, result.do_filter));
    FASSERT (result.curr_qp == test->expect.curr_qp);
    FASSERT (result.curr_qpc == test->expect.curr_qpc);
  }
}



static void test_compute_strength_motion_vectors (void)
{
  test_data_compute_strength test_cases[] = {
    {
      .mb_curr = {
        .mbtype = P_8x8,
        .flags = MB_LEFT_ | MB_UP_,
        .cbp_blk = 0x0,
        .qp = 28,
      },
      .motion_vectors = {
        /* mb top left not important */
        { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0},
        { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0},
        { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0},
        { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0},

        /* mb top, bottom mv row important */
        { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0},
        { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0},
        { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0},
        { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0},

        /* mb left, right mv column important */
        { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0},
        { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0},
        { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0},
        { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 4, .y = 4, .ref_num = 1},

        /* mb curr */
        { .x = 4, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0},
        { .x = 4, .y = 4, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0},
        { .x = 4, .y = 4, .ref_num = 1}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0},
        { .x = 4, .y = 4, .ref_num = 1}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0}, { .x = 0, .y = 0, .ref_num = 0},
      },
      .expect = {
        .vert_strength1 = 0x0,
        .vert_strength2 = 0x0,
        .horz_strength1 = 0x0,
        .horz_strength2 = 0x0,
        .do_filter = 0xccc48880,
        .do_filter = 0x01112333,
        .curr_qp = 28,
        .curr_qpc = 28,
      }
    },
  };

  const int mbxmax = 2;
  const int mbymax = 2;
  mbinfo_store_t mb_stored[mbxmax * mbymax];

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_compute_strength * test = &test_cases[i];
    mb_deblock_t result;

    mb_stored[0 * mbxmax + 1] = test->mb_top;
    mb_stored[1 * mbxmax + 0] = test->mb_left;
    mb_stored[1 * mbxmax + 1] = test->mb_curr;

    mbinfo_store_t * curr_mb_stored = &mb_stored[1 * mbxmax + 1];
    mv_t * motion_vectors_ptr = &test->motion_vectors[(1 * mbxmax + 1) * NUM_MVS_MB];

    taa_h264_mb_compute_strength (curr_mb_stored, motion_vectors_ptr, mbxmax, &result);

#if 0
    fprintf (stderr, "---\n");
    fprintf (stderr, "vert1: 0x%04x\n", result.vert_strength1);
    fprintf (stderr, "vert2: 0x%04x\n", result.vert_strength2);
    fprintf (stderr, "horz1: 0x%04x\n", result.horz_strength1);
    fprintf (stderr, "horz2: 0x%04x\n", result.horz_strength2);
    fprintf (stderr, "filte: 0x%08x\n", result.do_filter);
#endif

    FASSERT (result.do_filter == test->expect.do_filter);
    FASSERT (result.vert_strength1 == test->expect.vert_strength1);
    FASSERT (result.vert_strength2 == test->expect.vert_strength2);
    FASSERT (result.horz_strength1 == test->expect.horz_strength1);
    FASSERT (result.horz_strength2 == test->expect.horz_strength2);
    FASSERT (result.curr_qp == test->expect.curr_qp);
    FASSERT (result.curr_qpc == test->expect.curr_qpc);
  }
}


void test_com_deblock (void)
{
  test_compute_strength_intra ();
  test_compute_strength_cbp_blk ();
  test_compute_strength_motion_vectors ();
}
