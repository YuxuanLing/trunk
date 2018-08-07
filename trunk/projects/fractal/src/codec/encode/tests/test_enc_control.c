#include "taah264enc.h"
#include "com_compatibility.h"
#include "fractaltest.h"
#include <memory.h>

#define MAX_NAL_REF_IDC                               3
#define PARSE_FBIT_FROM_NALU(nalu_header)             ((nalu_header & 0x80) == 0x80)
#define PARSE_NRI_FROM_NALU(nalu_header)              ((nalu_header >> 5) & 0x03)
#define PARSE_TYPE_FROM_NALU(nalu_header)             (nalu_header & 0x1F)
#define PARSE_TEMPORAL_ID_FROM_NALU(nalu_header)      \
  (MAX_NAL_REF_IDC - PARSE_NRI_FROM_NALU (nalu_header))

typedef struct
{
  int active_levels;
  int counter;
} test_context;

#pragma warning (disable:869)
static bool nal_cb (
  unsigned char * buf,
  unsigned        size,
  unsigned char * total_buffer,
  unsigned        total_size,
  unsigned        lt_idx,
  unsigned        frame_num,
  bool            last_nalu_in_frame,
  void *          context)
{
  static uint8_t expect_start_code[] = {0, 0, 0, 1};
  FASSERT (buf != NULL);
  FASSERT (memcmp (buf, expect_start_code, sizeof expect_start_code) == 0);
  buf += sizeof expect_start_code;

  int type = PARSE_TYPE_FROM_NALU (*buf);
  int temporal_id = PARSE_TEMPORAL_ID_FROM_NALU (*buf);

  test_context * ctx = (test_context *) context;
  if (ctx->active_levels >= 0)
    FASSERT (temporal_id < ctx->active_levels);
  if ((type == 1 || type == 5) && last_nalu_in_frame)
  {
    ctx->counter++;
  }

  return false;
}
#pragma warning (default:869)

void test_skip_temporal_levels (void)
{
  taa_h264_enc_create_params cparams;
  taa_h264_enc_exec_params eparams;
  taa_h264_enc_handle * enc;
  const int max_nalu_size = 1024;
  uint8_t nalu[max_nalu_size];
  test_context context;

  const int width = 16;
  const int height = 16;
  TAA_H264_ALIGN(64) uint8_t input[width * height * 3 / 2];

  taa_h264_enc_default_create_params (&cparams);
  taa_h264_enc_default_exec_params (&eparams);

  cparams.height = height;
  cparams.width = width;
  cparams.callbacks.output_nalu = nal_cb;
  cparams.callbacks.context = &context;

  // GOP structure:  0 2 1 2 0 ....
  cparams.num_ref_frames = 3;
  cparams.temporal_levels = 3;
  eparams.gop_size = 4;

  eparams.bitrate = 100;
  eparams.max_nalu_size = max_nalu_size;
  eparams.raw_buffer = input;
  eparams.output_buffer = nalu;

  const int frames = 9;
  enc = taa_h264_enc_create (&cparams);
  FASSERT (enc != NULL);

  // Everything should be encoded
  context.active_levels = eparams.active_temporal_levels = -1;
  context.counter = 0;
  for (int i = 0; i < frames; i++)
  {
    eparams.force_idr = (i == 0);
    taa_h264_enc_execute (enc, &eparams);
  }
  FASSERT (context.counter == 9);

  // All frames should be encoded
  context.active_levels = eparams.active_temporal_levels = 3;
  context.counter = 0;
  for (int i = 0; i < frames; i++)
  {
    eparams.force_idr = (i == 0);
    taa_h264_enc_execute (enc, &eparams);
  }
  FASSERT (context.counter == frames);

  // Temporal levels 0 and 1 should be encoded
  context.active_levels = eparams.active_temporal_levels = 2;
  context.counter = 0;
  for (int i = 0; i < frames; i++)
  {
    eparams.force_idr = (i == 0);
    taa_h264_enc_execute (enc, &eparams);
  }
  FASSERT (context.counter == 5);

  // Temporal level 0 should be encoded
  context.active_levels = eparams.active_temporal_levels = 1;
  context.counter = 0;
  for (int i = 0; i < frames; i++)
  {
    eparams.force_idr = (i == 0);
    taa_h264_enc_execute (enc, &eparams);
  }
  FASSERT (context.counter == 3);

  // Nothing should be encoded
  context.active_levels = eparams.active_temporal_levels = 0;
  context.counter = 0;
  for (int i = 0; i < frames; i++)
  {
    eparams.force_idr = (i == 0);
    taa_h264_enc_execute (enc, &eparams);
  }
  FASSERT (context.counter == 0);

  // Switch active levels during a GOP
  context.active_levels = eparams.active_temporal_levels = 3;
  context.counter = 0;
  for (int i = 0; i < frames; i++)
  {
    if (i == 3)
    {
      context.active_levels = eparams.active_temporal_levels = 1;
    }
    eparams.force_idr = (i == 0);
    taa_h264_enc_execute (enc, &eparams);
  }
  FASSERT (context.counter == 5);

  taa_h264_enc_destroy (enc);
}


void test_enc_control (void)
{
  test_skip_temporal_levels ();
}
