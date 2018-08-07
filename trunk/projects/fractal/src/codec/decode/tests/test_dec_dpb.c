#include "dec_dpb.h"
#include "test_com_helpers.h"
#include "fractaltest.h"
#include <string.h>

#define MAX_RPM_REPEATS 8
#define MAX_MMCO_IN_RPM 5
#define MAX_ACKS (MAX_RPM_REPEATS * MAX_MMCO_IN_RPM)

typedef struct
{
  int original_frame_num;
  bool original_idr_flag;
  bool idr_is_long_term;
  bool adaptive_marking_flag;
  mmco_t mmco[MAX_MMCO_IN_RPM];
} rpm_t;

typedef struct
{
  bool is_longterm;
  int st_id;
  int lt_id;
  int original_frame_num;
  bool in_sync;
  bool usable_ref;
} testpic_t;


#define MAGIC_NUM_REST_UNUSED (-123456789)

static const testpic_t rest_of_pics_are_unused = {
  .original_frame_num = MAGIC_NUM_REST_UNUSED
};

static bool is_rest_of_pics_unused (
                                    testpic_t * pic)
{
  return pic->original_frame_num == MAGIC_NUM_REST_UNUSED;
}


static void init_dpb_from_test_pictures (
  testpic_t                  pictures[MAX_NUM_DPB_PICS],
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
    meta[i].in_sync = pictures[i].in_sync;
    meta[i].usable_ref = pictures[i].usable_ref;
    meta[i].original_frame_num = pictures[i].original_frame_num;
    pic.meta = &meta[i];
    dpb->last_seen_frame_num = pictures[i].original_frame_num;
    helper_dpb_add_picture (dpb, &pic);
    i++;
  }
}



// Global variables used for storing the acks/nacks for one test.
// Global due to lazyness
static unsigned ack_result[MAX_ACKS];
static unsigned nack_result[MAX_ACKS];
static int num_acks;
static int num_nacks;

#define ACK_NOT_SET (~0x0)
#pragma warning (disable:869)
void ack_cb (void *   context,
             unsigned frame_num,
             bool     ack,
             bool     idr)
{
  if (ack)
    ack_result[num_acks++] = frame_num;
  else
    nack_result[num_nacks++] = frame_num;
}
#pragma warning (default:869)

void init_acks_for_test (void)
{
  memset (ack_result, 0xff, sizeof (ack_result));
  memset (nack_result, 0xff, sizeof (nack_result));
  num_acks = 0;
  num_nacks = 0;
}

static bool has_correct_acks (
  unsigned * expected_acks,
  int        expected_num_acks,
  unsigned * expected_nacks,
  int        expected_num_nacks)
{
  bool ok = true;

  ok &= expected_num_acks == num_acks;
  ok &= expected_num_nacks == num_nacks;

  for (int i = 0; i < num_acks; i++)
    ok &= expected_acks[i] == ack_result[i];
  for (int i = 0; i < num_nacks; i++)
    ok &= expected_nacks[i] == nack_result[i];

  return ok;
}

typedef struct
{
  testpic_t test_pictures [MAX_NUM_DPB_PICS];
  rpm_t rpm_repeat_list [MAX_RPM_REPEATS];
  int rpm_repeat_count;
  bool expect_noop;
  testpic_t expect_pictures[MAX_NUM_DPB_PICS];
  int expect_num_acks;
  int expect_num_nacks;
  unsigned expect_acks[MAX_ACKS];
  unsigned expect_nacks[MAX_ACKS];
} test_data_rpm_repeat;

static void test_ref_pic_marking_repetition (void)
{
  test_data_rpm_repeat test_cases[] = {
    {
      /* Repeated RPM contains nothing */
      .test_pictures = {
        {.is_longterm = false, .st_id = 0, .original_frame_num = 1, .in_sync = true, .usable_ref = true},
        {.is_longterm = true,  .lt_id = 0, .original_frame_num = 0, .in_sync = true, .usable_ref = true},
        rest_of_pics_are_unused,
      },
      .rpm_repeat_count = 2,
      .rpm_repeat_list = {
        {
          .original_frame_num = 0,
          .original_idr_flag = true,
          .idr_is_long_term = false,
          .adaptive_marking_flag = false,
        },
        {
          .original_frame_num = 1,
          .original_idr_flag = false,
          .adaptive_marking_flag = true,
          .mmco = {{.op = 0}}
        },
      },
      .expect_noop = true,
      .expect_num_acks = 0,
      .expect_num_nacks = 0,
    },

    {
      /* Mark a currpic as long term with a MMCO that has already been processed. */
      .test_pictures = {
        {.is_longterm = false, .st_id = 2, .original_frame_num = 2, .in_sync = true, .usable_ref = true},
        {.is_longterm = true,  .lt_id = 1, .original_frame_num = 5, .in_sync = true, .usable_ref = true},
        rest_of_pics_are_unused,
      },
      .rpm_repeat_count = 1,
      .rpm_repeat_list = {
        {
          .original_frame_num = 5,
          .original_idr_flag = false,
          .adaptive_marking_flag = true,
          .mmco = {{.op = 6, .val1 = 1}, {.op = 0}}
        }
      },
      .expect_noop = true,
      .expect_num_acks = 1,
      .expect_num_nacks = 0,
      .expect_acks = {5, }
    },

    {
      /* Mark IDR as long. Already processed so no change should be made */
      .test_pictures = {
        {.is_longterm = false, .st_id = 2, .original_frame_num = 2, .in_sync = true, .usable_ref = true},
        {.is_longterm = true,  .lt_id = 0, .original_frame_num = 0, .in_sync = true, .usable_ref = true},
        rest_of_pics_are_unused,
      },
      .rpm_repeat_count = 1,
      .rpm_repeat_list = {
        {
          .original_frame_num = 0,
          .original_idr_flag = true,
          .idr_is_long_term = true,
        }
      },
      .expect_noop = true,
      .expect_num_acks = 1,
      .expect_num_nacks = 0,
      .expect_acks = {0, }
    },

    {
      /* LTFI exists, but has different original_frame_num */
      .test_pictures = {
        {.is_longterm = false, .st_id = 2, .original_frame_num = 2, .in_sync = true, .usable_ref = true},
        {.is_longterm = true,  .lt_id = 1, .original_frame_num = 0, .in_sync = true, .usable_ref = true},
        rest_of_pics_are_unused,
      },
      .rpm_repeat_count = 1,
      .rpm_repeat_list = {
        {
          .original_frame_num = 42,
          .original_idr_flag = false,
          .adaptive_marking_flag = true,
          .mmco = {{.op = 6, .val1 = 1}, {.op = 0}}
        }
      },
      .expect_noop = false,
      .expect_pictures = {
        {.is_longterm = false, .st_id = 2, .original_frame_num = 2, .in_sync = true, .usable_ref = true},
        {.is_longterm = true,  .lt_id = 1, .original_frame_num = 0, .in_sync = false, .usable_ref = false},
        rest_of_pics_are_unused,
      },
      .expect_num_acks = 0,
      .expect_num_nacks = 1,
      .expect_nacks = {42, }
    },

    {
      /* LTFI does not exists when sending "mark current as long term" which
       * means we don't have the original frame. */
      .test_pictures = {
        {.is_longterm = false, .st_id = 2, .original_frame_num = 2, .in_sync = true, .usable_ref = true},
        {.is_longterm = true,  .lt_id = 0, .original_frame_num = 0, .in_sync = true, .usable_ref = true},
        rest_of_pics_are_unused,
      },
      .rpm_repeat_count = 1,
      .rpm_repeat_list = {
        {
          .original_frame_num = 0,
          .original_idr_flag = false,
          .adaptive_marking_flag = true,
          .mmco = {{.op = 6, .val1 = 1}, {.op = 0}}
        },
      },
      .expect_noop = false,
      .expect_pictures = {
        {.is_longterm = false, .st_id = 2, .original_frame_num = 2, .in_sync = true, .usable_ref = true},
        {.is_longterm = true,  .lt_id = 0, .original_frame_num = 0, .in_sync = true, .usable_ref = true},
        rest_of_pics_are_unused,
      },
      .expect_num_acks = 0,
      .expect_num_nacks = 1,
      .expect_nacks = {0, }
    },

    {
      /* MMCO triggers multiple acks and nacks. */
      .test_pictures = {
        {.is_longterm = true,  .lt_id = 0, .original_frame_num = 10, .in_sync = true, .usable_ref = true},
        {.is_longterm = true,  .lt_id = 2, .original_frame_num = 30, .in_sync = true, .usable_ref = true},
        {.is_longterm = true,  .lt_id = 3, .original_frame_num = 40, .in_sync = true, .usable_ref = true},
        rest_of_pics_are_unused,
      },
      .rpm_repeat_count = 6,
      .rpm_repeat_list = {
        {
          // Triggers NACK and marks the existing LTFI 0 as wrong
          .original_frame_num = 0,
          .original_idr_flag = true,
          .idr_is_long_term = true,
          .adaptive_marking_flag = false,
        },
        {
          // Triggers NACK
          .original_frame_num = 20,
          .original_idr_flag = false,
          .adaptive_marking_flag = true,
          .mmco = {{.op = 6, .val1 = 1}, {.op = 0}}
        },
        {
          // Triggers ACK
          .original_frame_num = 30,
          .original_idr_flag = false,
          .adaptive_marking_flag = true,
          .mmco = {{.op = 6, .val1 = 2}, {.op = 0}}
        },
        {
          // Triggers ACK
          .original_frame_num = 40,
          .original_idr_flag = false,
          .adaptive_marking_flag = true,
          .mmco = {{.op = 6, .val1 = 3}, {.op = 0}}
        },
        {
          // Triggers NACK
          .original_frame_num = 50,
          .original_idr_flag = false,
          .adaptive_marking_flag = true,
          .mmco = {{.op = 6, .val1 = 4}, {.op = 0}}
        },
      },
      .expect_noop = false,
      .expect_pictures = {
        {.is_longterm = true,  .lt_id = 0, .original_frame_num = 10, .in_sync = false, .usable_ref = false},
        {.is_longterm = true,  .lt_id = 2, .original_frame_num = 30, .in_sync = true, .usable_ref = true},
        {.is_longterm = true,  .lt_id = 3, .original_frame_num = 40, .in_sync = true, .usable_ref = true},
        rest_of_pics_are_unused,
      },
      .expect_num_acks = 2,
      .expect_num_nacks = 3,
      .expect_acks = {30, 40},
      .expect_nacks = {0, 20, 50},
    },

    {
      /* MMCO to expand long term buffer.
       * The content of the buffer should not be changed */
      .test_pictures = {
        {.is_longterm = false,  .st_id = 0, .original_frame_num = 0, .in_sync = true, .usable_ref = true},
        {.is_longterm = false,  .st_id = 1, .original_frame_num = 1, .in_sync = true, .usable_ref = true},
        {.is_longterm = true,   .lt_id = 0, .original_frame_num = 2, .in_sync = true, .usable_ref = true},
        rest_of_pics_are_unused,
      },
      .rpm_repeat_count = 1,
      .rpm_repeat_list = {
        {
          // Expand to two long term frames
          .original_frame_num = 0,
          .original_idr_flag = false,
          .adaptive_marking_flag = true,
          .mmco = {{.op = 4, .val1 = 2}, {.op = 0}}
        },
      },
      .expect_noop = true,
      .expect_num_acks = 0,
      .expect_num_nacks = 0,
    },

    {
      /* MMCO to shrink long term buffer. We don't want to free the long term
       * frames since they might have been added _after_ this repeated MMCO
       * was sent the first time. */
      .test_pictures = {
        {.is_longterm = false,  .st_id = 0, .original_frame_num = 0, .in_sync = true, .usable_ref = true},
        {.is_longterm = false,  .st_id = 1, .original_frame_num = 1, .in_sync = true, .usable_ref = true},
        {.is_longterm = true,   .lt_id = 1, .original_frame_num = 2, .in_sync = true, .usable_ref = true},
        rest_of_pics_are_unused,
      },
      .rpm_repeat_count = 1,
      .rpm_repeat_list = {
        {
          // Shrink to zero long term frames
          .original_frame_num = 0,
          .original_idr_flag = false,
          .adaptive_marking_flag = true,
          .mmco = {{.op = 4, .val1 = 0}, {.op = 0}}
        },
      },
      .expect_noop = true,
      .expect_num_acks = 0,
      .expect_num_nacks = 0,
    },

    {
      /* Moving multiple short term frames to the long term buffer. First,
       * expand the LT buffer, then move to LT buffer where one of the
       * previous LT frames have the same LTFI and will be overwritten. */
      .test_pictures = {
        {.is_longterm = false, .st_id = 1, .original_frame_num = 1, .in_sync = true, .usable_ref = true},
        {.is_longterm = false, .st_id = 2, .original_frame_num = 2, .in_sync = true, .usable_ref = true},
        {.is_longterm = true,  .lt_id = 0, .original_frame_num = 0, .in_sync = false, .usable_ref = false},
        {.is_longterm = true,  .lt_id = 1, .original_frame_num = 5, .in_sync = false, .usable_ref = true},
        rest_of_pics_are_unused,
      },
      .rpm_repeat_count = 1,
      .rpm_repeat_list = {
        {
          .original_frame_num = 3,
          .original_idr_flag = false,
          .adaptive_marking_flag = true,
          .mmco = {
            {.op = 4, .val1 = 3}, // expand to three LT frames
            {.op = 3, .val1 = 0, .val2 = 2}, // move ST 2 to LTFI 2, ACK
            {.op = 3, .val1 = 1, .val2 = 0}, // move ST 1 to LTFI 0, overwriting existing LTFO 0, ACK
            {.op = 3, .val1 = 2, .val2 = 1}, // move ST 0 to LTFI 1, ST 0 does not exist, mark exsisting LTFI 2 as out of sync, NACK
            {.op = 0}
          }
        },
      },
      .expect_noop = false,
      .expect_pictures = {
        {.is_longterm = true,  .lt_id = 0, .original_frame_num = 1, .in_sync = true, .usable_ref = true},
        {.is_longterm = true,  .lt_id = 2, .original_frame_num = 2, .in_sync = true, .usable_ref = true},
        {.is_longterm = true,  .lt_id = 1, .original_frame_num = 5, .in_sync = false, .usable_ref = false},
        rest_of_pics_are_unused,
      },
      .expect_num_acks = 2,
      .expect_num_nacks = 1,
      .expect_acks = {2, 1, },
      .expect_nacks = {0, }
    },

    {
      /* frame_num is wrapped between current frame and the rpm's
       * original_frame_num. The test data assumes max_frame_num is 16, thus
       * after wrapping ST_ID 15 => -1, 14 => -2 etc.
       *
       * Mark short term frames as unused. */
      .test_pictures = {
        {.is_longterm = false, .st_id = -3, .original_frame_num = 13, .in_sync = true, .usable_ref = true},
        {.is_longterm = false, .st_id = -2, .original_frame_num = 14, .in_sync = true, .usable_ref = true},
        {.is_longterm = false, .st_id = -1, .original_frame_num = 15, .in_sync = true, .usable_ref = true},
        {.is_longterm = false,  .st_id = 0, .original_frame_num = 0,  .in_sync = true, .usable_ref = true},
        {.is_longterm = false,  .st_id = 1, .original_frame_num = 1,  .in_sync = true, .usable_ref = true},
        rest_of_pics_are_unused,
      },
      .rpm_repeat_count = 2,
      .rpm_repeat_list = {
        {
          .original_frame_num = 15,
          .original_idr_flag = false,
          .adaptive_marking_flag = true,
          .mmco = {
            {.op = 1, .val1 = 0}, // mark 14 as unused
            {.op = 3, .val1 = 1, .val2 = 0}, // mark 13 as long term (LTFI 0)
            {.op = 0}
          }
        },
        {
          .original_frame_num = 1,
          .original_idr_flag = false,
          .adaptive_marking_flag = true,
          .mmco = {
            {.op = 1, .val1 = 1}, // mark 15 as unused
            {.op = 1, .val1 = 0}, // mark 0 as unused
            {.op = 0}
          }
        },
      },
      .expect_noop = false,
      .expect_pictures = {
        {.is_longterm = true,  .lt_id = 0, .original_frame_num = 13, .in_sync = true, .usable_ref = true},
        {.is_longterm = false, .st_id = 1, .original_frame_num =  1, .in_sync = true, .usable_ref = true},
        rest_of_pics_are_unused,
      },
      .expect_num_acks = 1,
      .expect_num_nacks = 0,
      .expect_acks = {13, },
      .expect_nacks = {0, }
    },
  };

  callbacks_t cb = {.context = NULL, .reference_ack = ack_cb};

  int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    decoded_picture_buffer_t dpb;
    decoded_picture_buffer_t expect_dpb;
    ref_meta_t meta[MAX_NUM_DPB_PICS];
    ref_meta_t expect_meta[MAX_NUM_DPB_PICS];
    test_data_rpm_repeat * test = &test_cases[i];

    /* Initialize the initial state. */
    init_dpb_from_test_pictures (test->test_pictures, &dpb, meta);
    init_acks_for_test ();
    /* Initialize the expected state. */
    if (test->expect_noop)
      init_dpb_from_test_pictures (test->test_pictures, &expect_dpb, expect_meta);
    else
      init_dpb_from_test_pictures (test->expect_pictures, &expect_dpb, expect_meta);

    for (int j = 0; j < test->rpm_repeat_count; j++)
    {
      rpm_t * rpm = &test->rpm_repeat_list[j];
      taa_h264_dec_dpb_rpm_repetition (
        &dpb,
        rpm->original_frame_num,
        rpm->original_idr_flag,
        rpm->idr_is_long_term,
        rpm->adaptive_marking_flag,
        rpm->mmco,
        &cb);
    }

    // manual assert on expected values
    // memcmp does not work since pointers (which we have not set) differs
    FASSERT (expect_dpb.total_ref_frames == dpb.total_ref_frames);
    FASSERT (expect_dpb.num_lt_frames == dpb.num_lt_frames);
    int j = 0, idx1 = 0, idx2 = 0;
    while (j < dpb.total_ref_frames)
    {
      decoded_picture_t * expect_pic = &expect_dpb.pictures[idx2];
      decoded_picture_t * pic = &dpb.pictures[idx1];
      ref_meta_t * expect_meta = (ref_meta_t *) expect_pic->meta;
      ref_meta_t * meta = (ref_meta_t *) pic->meta;

      // Skip pictures that are not used
      if (!pic->is_used)
        idx1++;
      if (!expect_pic->is_used)
        idx2++;
      if (!(pic->is_used && expect_pic->is_used))
        continue;

      FASSERT (expect_pic->is_used == pic->is_used);
      FASSERT (expect_pic->is_longterm == pic->is_longterm);
      FASSERT (expect_pic->picid == pic->picid);
      FASSERT (expect_meta->in_sync == meta->in_sync);
      FASSERT (expect_meta->original_frame_num == meta->original_frame_num);
      FASSERT (expect_meta->usable_ref == meta->usable_ref);

      j++;
      idx1++;
      idx2++;
    }

    FASSERT (has_correct_acks (test->expect_acks,
                               test->expect_num_acks,
                               test->expect_nacks,
                               test->expect_num_nacks));
  }
}


void test_dec_dpb (void)
{
  test_ref_pic_marking_repetition ();
}
