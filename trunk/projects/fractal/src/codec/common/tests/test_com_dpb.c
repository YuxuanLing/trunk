#include "com_dpb.c"
#include "test_com_helpers.h"
#include "fractaltest.h"

typedef struct
{
  int curr_picnum;
  bool is_ref;
  int max_ref_frames;
  int num_pictures;
  decoded_picture_t pictures[MAX_NUM_DPB_PICS];
  bool expect_used[MAX_NUM_DPB_PICS];
  int expect_picnums[MAX_NUM_DPB_PICS];
} test_data_sliding_window;


static void test_sliding_window (void)
{
  test_data_sliding_window test_cases[] = {
    {
      .max_ref_frames = 1,
      .num_pictures = 0,
      .pictures = {0},
      .curr_picnum = 10,
      .is_ref = true,
      .expect_used = { true, false, },
      .expect_picnums = { 10, 0, }
    },
    {
      .max_ref_frames = MAX_NUM_REF_FRAMES,
      .num_pictures = 0,
      .pictures = {0},
      .curr_picnum = 10,
      .is_ref = true,
      .expect_used = { true, false, },
      .expect_picnums = { 10, 0, }
    },
    {
      .max_ref_frames = 3,
      .num_pictures = 2,
      .pictures = { { .is_used = true, .picid = 4},
                    { .is_used = true, .picid = 2} },
      .curr_picnum = 10,
      .is_ref = true,
      .expect_used = { true, true, true, false, },
      .expect_picnums = { 4, 2, 10, 0, }
    },
    {
      .max_ref_frames = 2,
      .num_pictures = 2,
      .pictures = { { .is_used = true, .picid = 4},
                    { .is_used = true, .picid = 2} },
      .curr_picnum = 10,
      .is_ref = true,
      .expect_used = { true, false, true, false, },
      .expect_picnums = { 4, 0, 10, 0, }
    },
    {
      .max_ref_frames = 3,
      .num_pictures = 2,
      .pictures = { { .is_used = true, .picid = 4},
                    { .is_used = true, .picid = 2} },
      .curr_picnum = 10,
      .is_ref = false,
      .expect_used = { true, true, false, false, },
      .expect_picnums = { 4, 2, 0, 0, }
    },
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_sliding_window * test = &test_cases[i];
    const mmco_t * mmco = NULL;
    const bool adaptive_marking_flag = false;

    decoded_picture_buffer_t dpb;
    helper_dpb_init (&dpb, test->max_ref_frames, 16);
    for (int j = 0; j < test->num_pictures; j++)
      helper_dpb_add_picture (&dpb, &test->pictures[j]);

    taa_h264_dpb_update (&dpb, &test->curr_picnum, test->is_ref,
                         false, false, adaptive_marking_flag, mmco);

    for (int j = 0; j < dpb.max_ref_frames+1; j++)
    {
      FASSERT (test->expect_used[j] == dpb.pictures[j].is_used);
      if (dpb.pictures[j].is_used)
        FASSERT (test->expect_picnums[j] == dpb.pictures[j].picid);
    }
  }
}

static void test_default_ref_list (void)
{
  test_data_sliding_window test_cases[] = {
    {
      .max_ref_frames = 2,
      .num_pictures = 2,
      .pictures = { { .is_used = true, .picid = 1, .is_longterm = false},
                    { .is_used = true, .picid = 2, .is_longterm = false} }
    },
    {
      .max_ref_frames = 2,
      .num_pictures = 2,
      .pictures = { { .is_used = true, .picid = 2, .is_longterm = false},
                    { .is_used = true, .picid = 1, .is_longterm = false} }
    },
    {
      .max_ref_frames = 2,
      .num_pictures = 1,
      .pictures = { { .is_used = true, .picid = 2, .is_longterm = false} }
    },
    {
      .max_ref_frames = MAX_NUM_REF_FRAMES,
      .num_pictures = 7,
      .pictures = { { .is_used = true,  .picid = 2, .is_longterm = false},
                    { .is_used = false, .picid = 3, .is_longterm = false},
                    { .is_used = true,  .picid = 6, .is_longterm = false},
                    { .is_used = false, .picid = 7, .is_longterm = false},
                    { .is_used = true,  .picid = 6, .is_longterm = false},
                    { .is_used = true,  .picid = 9, .is_longterm = false},
                    { .is_used = true,  .picid = 8, .is_longterm = false} }
    },
    {
      .max_ref_frames = 2,
      .num_pictures = 2,
      .pictures = { { .is_used = true, .picid = 2, .is_longterm = true},
                    { .is_used = true, .picid = 1, .is_longterm = true} }
    },
    {
      .max_ref_frames = 3,
      .num_pictures = 3,
      .pictures = { { .is_used = true, .picid = 2, .is_longterm = true},
                    { .is_used = true, .picid = 0, .is_longterm = false},
                    { .is_used = true, .picid = 1, .is_longterm = true} }
    },
    {
      .max_ref_frames = 3,
      .num_pictures = 3,
      .pictures = { { .is_used = true, .picid = 0, .is_longterm = true},
                    { .is_used = true, .picid = 8, .is_longterm = false},
                    { .is_used = true, .picid = 9, .is_longterm = false} }
    },
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_sliding_window * test = &test_cases[i];

    decoded_picture_buffer_t dpb;
    helper_dpb_init (&dpb, test->max_ref_frames, 16);
    for (int j = 0; j < test->num_pictures; j++)
      helper_dpb_add_picture (&dpb, &test->pictures[j]);

    taa_h264_dpb_default_ref_list (&dpb);

    int last_picid = INT_MAX;
    bool short_term_done = false;
    for (int j = 0; j < dpb.total_ref_frames; j++)
    {
      decoded_picture_t * pic = dpb.ref_list[j];

      if (short_term_done == false && pic->is_longterm)
      {
        short_term_done = true;
        last_picid = -1;
      }

      if (short_term_done == false)
      {
        FASSERT (pic->is_used == true);
        FASSERT (pic->is_longterm == false);
        FASSERT (pic->picid <= last_picid);
        last_picid = pic->picid;
      }
      else
      {
        FASSERT (pic->is_used == true);
        FASSERT (pic->is_longterm == true);
        FASSERT (pic->picid > last_picid);
        last_picid = pic->picid;
      }
    }
  }
}


typedef struct
{
  int curr_picnum;
  int max_frame_num;
  int num_pictures;
  decoded_picture_t pictures[MAX_NUM_DPB_PICS];
  rplr_t rplr[MAX_NUM_REF_FRAMES];
  int expect_picnums[MAX_NUM_DPB_PICS];
} test_data_reorder_ref_list;

static void test_reorder_ref_list (void)
{
  test_data_reorder_ref_list test_cases[] = {
    {
      .curr_picnum = 3,
      .max_frame_num = 32,
      .num_pictures = 2,
      .pictures = { { .is_used = true, .picid = 2, .is_longterm = false},
                    { .is_used = true, .picid = 1, .is_longterm = false} },
      .rplr = { { .idc = 3}, },
      .expect_picnums = { 2, 1}
    },
    {
      .curr_picnum = 3,
      .max_frame_num = 32,
      .num_pictures = 3,
      .pictures = { { .is_used = true, .picid = 0, .is_longterm = false},
                    { .is_used = true, .picid = 1, .is_longterm = false},
                    { .is_used = true, .picid = 2, .is_longterm = false} },
      .rplr = { { .idc = 0, .val = 2},
                { .idc = 1, .val = 0},
                { .idc = 3}, },
      .expect_picnums = { 0, 1, 2}
    },
    {
      .curr_picnum = 3,
      .max_frame_num = 32,
      .num_pictures = 3,
      .pictures = { { .is_used = true, .picid = 0, .is_longterm = false},
                    { .is_used = true, .picid = 10, .is_longterm = true},
                    { .is_used = true, .picid = 2, .is_longterm = false} },
      .rplr = { { .idc = 2, .val = 10},
                { .idc = 0, .val = 2},
                { .idc = 3}, },
      .expect_picnums = { 10, 0, 2}
    },
    {
      .curr_picnum = 21,
      .max_frame_num = 32,
      .num_pictures = 7,
      .pictures = { { .is_used = true, .picid =  0, .is_longterm = true},
                    { .is_used = true, .picid =  3, .is_longterm = false},
                    { .is_used = true, .picid = 10, .is_longterm = false},
                    { .is_used = true, .picid = 14, .is_longterm = false},
                    { .is_used = true, .picid = 18, .is_longterm = false},
                    { .is_used = true, .picid = 19, .is_longterm = false},
                    { .is_used = true, .picid = 20, .is_longterm = false} },
      .rplr = { { .idc = 2, .val = 0},
                { .idc = 1, .val = 13},
                { .idc = 0, .val = 14},
                { .idc = 0, .val = 5},
                { .idc = 0, .val = 3},
                { .idc = 3}, },
      .expect_picnums = { 0, 3, 20, 14, 10, 19, 18}
    },
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_reorder_ref_list * test = &test_cases[i];

    decoded_picture_buffer_t dpb;
    helper_dpb_init (&dpb, 0, test->max_frame_num);
    for (int j = 0; j < test->num_pictures; j++)
      helper_dpb_add_picture (&dpb, &test->pictures[j]);

    taa_h264_dpb_default_ref_list (&dpb);
    taa_h264_dpb_reorder_ref_list (&dpb, test->rplr, test->curr_picnum);

    for (int j = 0; j < dpb.total_ref_frames; j++)
    {
      FASSERT (dpb.ref_list[j]->is_used == true);
      FASSERT (dpb.ref_list[j]->picid == test->expect_picnums[j]);
    }
  }
}


typedef struct
{
  int frame_num;
  int max_frame_num;
  int num_pictures;
  decoded_picture_t pictures[MAX_NUM_DPB_PICS];
  int expect_picnums[MAX_NUM_DPB_PICS];
} test_data_update_picnums;

static void test_update_picnums (void)
{
  test_data_update_picnums test_cases[] = {
    {
      .frame_num = 0,
      .max_frame_num = 16,
      .num_pictures = 3,
      .pictures = { { .is_used = true, .picid = 2, .is_longterm = false},
                    { .is_used = true, .picid = 0, .is_longterm = false},
                    { .is_used = true, .picid = 1, .is_longterm = false} },
      .expect_picnums = { 2 - 16, 0, 1 - 16}
    },
    {
      .frame_num = 0,
      .max_frame_num = 16,
      .num_pictures = 3,
      .pictures = { { .is_used = true, .picid = 2, .is_longterm = false},
                    { .is_used = true, .picid = 0, .is_longterm = false},
                    { .is_used = true, .picid = 1, .is_longterm = true} },
      .expect_picnums = { 2 - 16, 0, 1}
    },
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_update_picnums * test = &test_cases[i];

    decoded_picture_buffer_t dpb;
    helper_dpb_init (&dpb, 0, test->max_frame_num);
    for (int j = 0; j < test->num_pictures; j++)
      helper_dpb_add_picture (&dpb, &test->pictures[j]);

    taa_h264_dpb_update_picnums (&dpb, test->frame_num);

    for (int j = 0; j < MAX_NUM_DPB_PICS; j++)
    {
      if (dpb.pictures[j].is_used)
        FASSERT (dpb.pictures[j].picid == test->expect_picnums[j]);
    }
  }
}

/* TODO: Write tests for adaptive marking */

void test_com_dpb (void)
{
  test_sliding_window ();
  test_default_ref_list ();
  test_reorder_ref_list ();
  test_update_picnums ();
}
