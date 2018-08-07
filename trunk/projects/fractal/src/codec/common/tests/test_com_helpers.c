#include "test_com_helpers.h"
#include "com_intmath.h"
#include "com_dpb.h"


void helper_dpb_init (
  decoded_picture_buffer_t * dpb,
  int max_ref_frames,
  int max_frame_num)
{
  const int width = 0;
  const int height = 0;
  dpb->max_width = width;
  dpb->max_height = height;

  // Init with max number of ref frames to initialize everything, so that we can
  // easily add one and one picture, increment dpb counters and don't care
  // about something being uninitialized.
  taa_h264_dpb_init (dpb, MAX_NUM_REF_FRAMES, width, height, max_frame_num,
    false, NULL);
  dpb->max_ref_frames = (uint8_t) max_ref_frames;
}

void helper_dpb_add_picture (
  decoded_picture_buffer_t * dpb,
  const decoded_picture_t *  pic)
{
  dpb->pictures[dpb->curr_idx] = *pic;
  dpb->curr_idx++;

  if (pic->is_used)
  {
    dpb->total_ref_frames++;
    dpb->max_ref_frames = (uint8_t) max (dpb->max_ref_frames, dpb->total_ref_frames);

    if (pic->is_longterm)
    {
      dpb->num_lt_frames++;
      dpb->max_lt_frames++;
    }
  }
}

