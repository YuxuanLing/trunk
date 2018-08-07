#include "fractaltest.h"
#include "dec.c"
#include "test_com_helpers.h"


typedef struct
{
  sliceinfo_t curr_slice;
  sliceinfo_t prev_slice;
  bool frame_complete;
  bool expect;
} test_data_is_new_frame;

static void test_is_new_frame (void)
{
  test_data_is_new_frame test_cases[] = {
    /* Change in frame num */
    {
      .curr_slice = { .frame_num = 0 },
      .prev_slice = { .frame_num = 0 },
      .frame_complete = false,
      .expect = false
    },
    {
      .curr_slice = { .frame_num = 0 },
      .prev_slice = { .frame_num = 1 },
      .frame_complete = false,
      .expect = true
    },
    {
      .curr_slice = { .frame_num = 15 },
      .prev_slice = { .frame_num = 14 },
      .frame_complete = false,
      .expect = true
    },
    /* Change in picture parameter set id */
    {
      .curr_slice = { .pic_parset_id = 0, },
      .prev_slice = { .pic_parset_id = 0, },
      .frame_complete = false,
      .expect = false
    },
    {
      .curr_slice = { .pic_parset_id = 0, },
      .prev_slice = { .pic_parset_id = 1, },
      .frame_complete = false,
      .expect = true
    },
    /* New frame due to nalref idc */
    {
      .curr_slice = { .nalref_idc = NALU_PRIORITY_DISPOSABLE },
      .prev_slice = { .nalref_idc = NALU_PRIORITY_DISPOSABLE },
      .frame_complete = false,
      .expect = false
    },
    {
      .curr_slice = { .nalref_idc = NALU_PRIORITY_LOW },
      .prev_slice = { .nalref_idc = NALU_PRIORITY_DISPOSABLE },
      .frame_complete = false,
      .expect = true
    },
    {
      .curr_slice = { .nalref_idc = NALU_PRIORITY_DISPOSABLE },
      .prev_slice = { .nalref_idc = NALU_PRIORITY_LOW },
      .frame_complete = false,
      .expect = true
    },
    {
      .curr_slice = { .nalref_idc = NALU_PRIORITY_LOW },
      .prev_slice = { .nalref_idc = NALU_PRIORITY_HIGH },
      .frame_complete = false,
      .expect = false
    },
    /* New frame due to picture order count */
    {
      .curr_slice = { .poc = { .type = 0, .lsb = 1 } },
      .prev_slice = { .poc = { .type = 0, .lsb = 2 } },
      .frame_complete = false,
      .expect = true
    },
    {
      .curr_slice = { .poc = { .type = 0, .delta_bottom = 1 } },
      .prev_slice = { .poc = { .type = 0, .delta_bottom = 2 } },
      .frame_complete = false,
      .expect = true
    },
    {
      .curr_slice = { .poc = { .type = 1, .delta_0 = 1 } },
      .prev_slice = { .poc = { .type = 1, .delta_0 = 2 } },
      .frame_complete = false,
      .expect = true
    },
    {
      .curr_slice = { .poc = { .type = 1, .delta_1 = 1 } },
      .prev_slice = { .poc = { .type = 1, .delta_1 = 2 } },
      .frame_complete = false,
      .expect = true
    },
    /* New frame due to IDR */
    {
      .curr_slice = { .idr_pic_id = IDR_PIC_ID_NOT_AVAILABLE },
      .prev_slice = { .idr_pic_id = IDR_PIC_ID_NOT_AVAILABLE },
      .frame_complete = false,
      .expect = false
    },
    {
      .curr_slice = { .idr_pic_id = IDR_PIC_ID_NOT_AVAILABLE },
      .prev_slice = { .idr_pic_id = 0 },
      .frame_complete = false,
      .expect = true
    },
    {
      .curr_slice = { .idr_pic_id = 0 },
      .prev_slice = { .idr_pic_id = IDR_PIC_ID_NOT_AVAILABLE },
      .frame_complete = false,
      .expect = true
    },
    {
      .curr_slice = { .idr_pic_id = 2 },
      .prev_slice = { .idr_pic_id = 1 },
      .frame_complete = false,
      .expect = true
    },
    {
      .curr_slice = { .idr_pic_id = 2 },
      .prev_slice = { .idr_pic_id = 2 },
      .frame_complete = false,
      .expect = false
    },
    {
      /* Not according to standard, but test for interoperability */
      .curr_slice = { .idr_pic_id = 0 },
      .prev_slice = { .idr_pic_id = 0 },
      .frame_complete = true,
      .expect = true
    }
  };

  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_is_new_frame * test = &test_cases[i];
    bool new_frame = taa_h264_is_new_frame (
      &test->curr_slice, &test->prev_slice, test->frame_complete);

    FASSERT (new_frame == test->expect);
  }
}


typedef struct
{
  bool is_longterm;
  int st_id;
  int lt_id;
  int original_frame_num;
} testpic_t;


#define MAGIC_NUM_REST_UNUSED (-123456789)

static const testpic_t rest_of_pics_are_unused = {
  .original_frame_num = MAGIC_NUM_REST_UNUSED
};

static bool is_rest_of_pics_unused (
  const testpic_t * pic)
{
  return pic->original_frame_num == MAGIC_NUM_REST_UNUSED;
}

static void init_dpb_from_test_pictures (
  const testpic_t            pictures[MAX_NUM_DPB_PICS],
  decoded_picture_buffer_t * dpb,
  ref_meta_t                 meta[MAX_NUM_DPB_PICS])
{
  helper_dpb_init (dpb, 0, 16);

  int i = 0;
  while (!is_rest_of_pics_unused (&pictures[i]))
  {
    decoded_picture_t pic;
    pic.is_used = true;
    pic.is_longterm = pictures[i].is_longterm;
    pic.picid = pic.is_longterm ? pictures[i].lt_id : pictures[i].st_id;
    meta[i].in_sync = true;
    meta[i].usable_ref = true;
    meta[i].original_frame_num = pictures[i].original_frame_num;
    pic.meta = &meta[i];
    helper_dpb_add_picture (dpb, &pic);
    i++;
  }
}

typedef struct
{
  int prev_frame_num;
  int curr_frame_num;
  bool prev_is_disposable;
  bool curr_is_disposable;
  bool curr_is_long_term;
  bool curr_reference_lt_frame;
  nalutype_t curr_nalu_type;
  bool gap_propagation_from_prev;
  bool expect_out_of_sync;
  bool expect_ghost_frame;
  bool expect_gap_propagation;
  int expect_dpb_status;
} test_data_frame_num_gap;

enum
{
  ALL_IS_GOOD         = 0x0,
  SHORT_TERM_UNSYNC   = 0x1,
  LONG_TERM_UNSYNC    = 0x2,
  IDR_UNSYNC          = 0x4,
  SHORT_TERM_UNUSABLE = 0x8,
  LONG_TERM_UNUSABLE  = 0x10,
  IDR_UNUSABLE        = 0x20,

};


// Go through the DPB and check if we have any IDRs, long term frames or short
// term frames that are out of sync / unusable for reference.
static int get_dpb_status (decoded_picture_buffer_t * dpb)
{
  int status = ALL_IS_GOOD;

  for (int i = 0; i < dpb->max_frame_num + 1; i++)
  {
    decoded_picture_t * pic = &dpb->pictures[i];
    ref_meta_t * meta = (ref_meta_t *) pic->meta;
    if (pic->is_used)
    {
      if (!meta->in_sync)
      {
        if (meta->original_frame_num == 0)
          status |= IDR_UNSYNC;
        else if (pic->is_longterm)
          status |= LONG_TERM_UNSYNC;
        else
          status |= SHORT_TERM_UNSYNC;
      }

      if (!meta->usable_ref)
      {
        if (meta->original_frame_num == 0)
          status |= IDR_UNUSABLE;
        else if (pic->is_longterm)
          status |= LONG_TERM_UNUSABLE;
        else
          status |= SHORT_TERM_UNUSABLE;
      }
    }
  }

  return status;
}


static void test_dec_frame_num_gap (void)
{
  test_data_frame_num_gap test_cases[] = {
    {
      // no gap, current is short term
      .prev_frame_num = 0,
      .curr_frame_num = 1,
      .prev_is_disposable = false,
      .curr_is_disposable = false,
      .curr_is_long_term = false,
      .curr_reference_lt_frame = false,
      .curr_nalu_type = NALU_TYPE_SLICE,
      .gap_propagation_from_prev = false,
      .expect_out_of_sync = false,
      .expect_ghost_frame = false,
      .expect_gap_propagation = false,
      .expect_dpb_status = ALL_IS_GOOD
    },
    {
      // no gap, current is long term
      .prev_frame_num = 0,
      .curr_frame_num = 1,
      .prev_is_disposable = false,
      .curr_is_disposable = false,
      .curr_is_long_term = true,
      .curr_reference_lt_frame = false,
      .curr_nalu_type = NALU_TYPE_SLICE,
      .gap_propagation_from_prev = false,
      .expect_out_of_sync = false,
      .expect_ghost_frame = false,
      .expect_gap_propagation = false,
      .expect_dpb_status = ALL_IS_GOOD
    },
    {
      // no gap, current is super p
      .prev_frame_num = 0,
      .curr_frame_num = 1,
      .prev_is_disposable = false,
      .curr_is_disposable = false,
      .curr_is_long_term = true,
      .curr_reference_lt_frame = false,
      .curr_nalu_type = NALU_TYPE_SLICE,
      .gap_propagation_from_prev = false,
      .expect_out_of_sync = false,
      .expect_ghost_frame = false,
      .expect_gap_propagation = false,
      .expect_dpb_status = ALL_IS_GOOD
    },
    {
      // gap, current is short term
      .prev_frame_num = 0,
      .curr_frame_num = 2,
      .prev_is_disposable = false,
      .curr_is_disposable = false,
      .curr_is_long_term = false,
      .curr_reference_lt_frame = false,
      .curr_nalu_type = NALU_TYPE_SLICE,
      .gap_propagation_from_prev = false,
      .expect_out_of_sync = true,
      .expect_ghost_frame = false,
      .expect_gap_propagation = false,
      .expect_dpb_status = SHORT_TERM_UNSYNC
    },
    {
      // gap, might have missing IDR
      // Potentially missing IDR(s) must be invalidate all IDRs in buffer, so
      // so that RPM repetition on a newer and missing IDR does not get falsely ACKed.
      .prev_frame_num = 99,
      .curr_frame_num = 1,
      .prev_is_disposable = false,
      .curr_is_disposable = false,
      .curr_is_long_term = false,
      .curr_reference_lt_frame = false,
      .curr_nalu_type = NALU_TYPE_SLICE,
      .gap_propagation_from_prev = false,
      .expect_out_of_sync = true,
      .expect_ghost_frame = false,
      .expect_gap_propagation = false,
      .expect_dpb_status = SHORT_TERM_UNSYNC | IDR_UNSYNC | IDR_UNUSABLE
    },
    {
      // gap, short term referencing lt frame
      // If we have a gap and current frame reference a LT frame, we might use
      // wrong LT frame (with same index) for reference, causing a ghost frame.
      .prev_frame_num = 0,
      .curr_frame_num = 2,
      .prev_is_disposable = false,
      .curr_is_disposable = false,
      .curr_is_long_term = false,
      .curr_reference_lt_frame = true,
      .curr_nalu_type = NALU_TYPE_SLICE,
      .gap_propagation_from_prev = false,
      .expect_out_of_sync = true,
      .expect_ghost_frame = true,
      .expect_gap_propagation = false,
      .expect_dpb_status = SHORT_TERM_UNSYNC
    },
    {
      // gap, long term referencing lt frame (super p)
      // Although there is a possibility for a ghost frame, we don't treat it as
      // such since the encoder should only used ACKed reference frames for super p.
      .prev_frame_num = 0,
      .curr_frame_num = 2,
      .prev_is_disposable = false,
      .curr_is_disposable = false,
      .curr_is_long_term = true,
      .curr_reference_lt_frame = true,
      .curr_nalu_type = NALU_TYPE_SLICE,
      .gap_propagation_from_prev = false,
      .expect_out_of_sync = true,
      .expect_ghost_frame = false,
      .expect_gap_propagation = false,
      .expect_dpb_status = SHORT_TERM_UNSYNC,
    },
    {
      // gap, current is disposable so error must propagate to next frame
      .prev_frame_num = 0,
      .curr_frame_num = 2,
      .prev_is_disposable = false,
      .curr_is_disposable = true,
      .curr_is_long_term = true,
      .curr_reference_lt_frame = true,
      .curr_nalu_type = NALU_TYPE_SLICE,
      .gap_propagation_from_prev = false,
      .expect_out_of_sync = true,
      .expect_ghost_frame = false,
      .expect_gap_propagation = true,
      .expect_dpb_status = SHORT_TERM_UNSYNC,
    },
    {
      // no gap, previous is disposable
      .prev_frame_num = 0,
      .curr_frame_num = 0,
      .prev_is_disposable = true,
      .curr_is_disposable = false,
      .curr_is_long_term = false,
      .curr_reference_lt_frame = false,
      .curr_nalu_type = NALU_TYPE_SLICE,
      .gap_propagation_from_prev = false,
      .expect_out_of_sync = false,
      .expect_ghost_frame = false,
      .expect_gap_propagation = false,
      .expect_dpb_status = ALL_IS_GOOD,
    },
    {
      // gap, previous is disposable
      .prev_frame_num = 0,
      .curr_frame_num = 1,
      .prev_is_disposable = true,
      .curr_is_disposable = false,
      .curr_is_long_term = false,
      .curr_reference_lt_frame = false,
      .curr_nalu_type = NALU_TYPE_SLICE,
      .gap_propagation_from_prev = false,
      .expect_out_of_sync = true,
      .expect_ghost_frame = false,
      .expect_gap_propagation = false,
      .expect_dpb_status = SHORT_TERM_UNSYNC,
    },
    {
      // gap, both are disposable
      .prev_frame_num = 0,
      .curr_frame_num = 1,
      .prev_is_disposable = true,
      .curr_is_disposable = true,
      .curr_is_long_term = false,
      .curr_reference_lt_frame = false,
      .curr_nalu_type = NALU_TYPE_SLICE,
      .gap_propagation_from_prev = false,
      .expect_out_of_sync = true,
      .expect_ghost_frame = false,
      .expect_gap_propagation = true,
      .expect_dpb_status = SHORT_TERM_UNSYNC,
    },
    {
      // gap, current is disposable and reference long term
      .prev_frame_num = 0,
      .curr_frame_num = 2,
      .prev_is_disposable = false,
      .curr_is_disposable = true,
      .curr_is_long_term = false,
      .curr_reference_lt_frame = true,
      .curr_nalu_type = NALU_TYPE_SLICE,
      .gap_propagation_from_prev = false,
      .expect_out_of_sync = true,
      .expect_ghost_frame = true,
      .expect_gap_propagation = true,
      .expect_dpb_status = SHORT_TERM_UNSYNC,
    },
    {
      // gap, but current is idr
      .prev_frame_num = 99,
      .curr_frame_num = 0,
      .prev_is_disposable = false,
      .curr_is_disposable = false,
      .curr_is_long_term = false,
      .curr_reference_lt_frame = false,
      .curr_nalu_type = NALU_TYPE_IDR,
      .gap_propagation_from_prev = false,
      .expect_out_of_sync = false,
      .expect_ghost_frame = false,
      .expect_gap_propagation = false,
      .expect_dpb_status = ALL_IS_GOOD,
    },
    {
      // gap, but current is idr and long term
      .prev_frame_num = 99,
      .curr_frame_num = 0,
      .prev_is_disposable = false,
      .curr_is_disposable = false,
      .curr_is_long_term = true,
      .curr_reference_lt_frame = false,
      .curr_nalu_type = NALU_TYPE_IDR,
      .gap_propagation_from_prev = false,
      .expect_out_of_sync = false,
      .expect_ghost_frame = false,
      .expect_gap_propagation = false,
      .expect_dpb_status = ALL_IS_GOOD,
    },
    {
      // no gap, but gap is propagated from prev frame
      .prev_frame_num = 1,
      .curr_frame_num = 2,
      .prev_is_disposable = false,
      .curr_is_disposable = false,
      .curr_is_long_term = false,
      .curr_reference_lt_frame = false,
      .curr_nalu_type = NALU_TYPE_SLICE,
      .gap_propagation_from_prev = true,
      .expect_out_of_sync = true,
      .expect_ghost_frame = false,
      .expect_gap_propagation = false,
      .expect_dpb_status = SHORT_TERM_UNSYNC,
    },
  };

  // Use the same initial DPB for all test cases. The DPB is not really important,
  // but is used to verify that the correct operations has been performed on the DPB
  const testpic_t initial_dpb_pictures[] = {
    {.is_longterm = false, .st_id = 5, .original_frame_num = 5},
    {.is_longterm = true,  .lt_id = 0, .original_frame_num = 0},
    {.is_longterm = true,  .lt_id = 1, .original_frame_num = 4},
    rest_of_pics_are_unused,
  };

  int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_frame_num_gap * t = &test_cases[i];
    frameinfo_t frameinfo = {0,};
    sliceinfo_t sliceinfo = {0,};

    frameinfo.frame_num = t->prev_frame_num;
    frameinfo.is_ref = t->prev_is_disposable ? false : true;
    sliceinfo.frame_num = t->curr_frame_num;
    sliceinfo.nalutype = t->curr_nalu_type;
    sliceinfo.nalref_idc = t->curr_is_disposable ? NALU_PRIORITY_DISPOSABLE : NALU_PRIORITY_NORMAL;
    sliceinfo.adaptive_marking_flag = t->curr_is_long_term;
    sliceinfo.reordering_flag = t->curr_reference_lt_frame;

    decoded_picture_buffer_t dpb;
    ref_meta_t meta[MAX_NUM_DPB_PICS];
    const bool gaps_in_frame_num_allowed = false;
    bool out_of_sync = false;
    bool ghost_frame = false;
    error_handler_t eh = {0,};

    init_dpb_from_test_pictures (initial_dpb_pictures, &dpb, meta);

    bool gap_propagation = check_and_handle_frame_num_gap (
      &frameinfo,
      &sliceinfo,
      &dpb,
      gaps_in_frame_num_allowed,
      t->gap_propagation_from_prev,
      &out_of_sync,
      &ghost_frame,
      &eh);

    FASSERT (gap_propagation == t->expect_gap_propagation);
    FASSERT (out_of_sync == t->expect_out_of_sync);
    FASSERT (ghost_frame == t->expect_ghost_frame);

    int dpb_status = get_dpb_status (&dpb);
    FASSERT (dpb_status == t->expect_dpb_status);
  }
}



void test_dec_dec (void)
{
  test_is_new_frame ();
  test_dec_frame_num_gap ();
}
