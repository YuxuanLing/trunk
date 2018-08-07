#include "enc_headers.h"
#include "enc_writebits.h"
#include "com_mbutils.h"
#include "fractaltest.h"
#include <stdlib.h>
#include <string.h>

static char desc[128];

typedef struct
{
  mbtype_t mbtype;
  uint8_t last_quant;
  bool islice;
  int mbrun;
  int mquant;
  uint16_t ycbp;
  bool acflag_uv;
  bool dcflag_uv;
  char * expect_code;
} test_data_mb_header;

static void test_mb_header_intra_4x4_all_pred (void)
{
  /* TAA_H264_INIT_TRACE (TAA_H264_TRACE_HEADERS, stderr); */

  const int mbymax = 2;
  const int mbxmax = 2;
  mbinfo_t mbmap[mbymax * mbxmax];
  bitwriter_t * writer = taa_h264_bitwriter_create();

  test_data_mb_header test_cases [] = {
    {
      .mbtype = I_4x4,
      .last_quant = 28,
      .islice = true,
      .mquant = 28,
      .ycbp = 0,
      .acflag_uv = false,
      .dcflag_uv = false,
      .expect_code = "11111111111111111100100"
    },
    {
      .mbtype = I_4x4,
      .last_quant = 28,
      .islice = true,
      .mquant = 28,
      .ycbp = 0x0020,
      .acflag_uv = false,
      .dcflag_uv = false,
      .expect_code = "1111111111111111110000111101"
    },
    {
      .mbtype = I_4x4,
      .last_quant = 28,
      .islice = true,
      .mquant = 28,
      .ycbp = 0x0020,
      .acflag_uv = true,
      .dcflag_uv = false,
      .expect_code = "111111111111111111000001010111"
    },
    {
      .mbtype = I_4x4,
      .last_quant = 28,
      .islice = true,
      .mquant = 28,
      .ycbp = 0x2020,
      .acflag_uv = false,
      .dcflag_uv = true,
      .expect_code = "1111111111111111110000101111"
    },
    {
      /* not i-slice, meaing mbtype will be different, and also
       * mb_skip_run will be a part of the code */
      .mbtype = I_4x4,
      .last_quant = 28,
      .islice = false,
      .mbrun = 0,
      .mquant = 28,
      .ycbp = 0,
      .acflag_uv = false,
      .dcflag_uv = false,
      .expect_code = "1001101111111111111111100100"
    },
    {
      /* test if mb_skip_run is written */
      .mbtype = I_4x4,
      .last_quant = 28,
      .islice = false,
      .mbrun = 10,
      .mquant = 28,
      .ycbp = 0,
      .acflag_uv = false,
      .dcflag_uv = false,
      .expect_code = "0001011001101111111111111111100100"
    },
  };

  uint8_t intra_modes[16];
  const int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    test_data_mb_header test = test_cases[i];

    taa_h264_reset_writer (writer, NULL, false);

    bool islice = test.islice;
    int mbrun = test.mbrun;

    mbmap[0].mquant = test.mquant;
    mbmap[0].intra_mode_chroma = DC_PRED_8;
    mbmap[0].mbcoeffs.luma_cbp = test.ycbp;
    mbmap[0].mbcoeffs.chroma_cbp = (uint16_t) (test.dcflag_uv | (test.acflag_uv << 1));
    mbmap[0].mbtype = test.mbtype;
    mbmap[0].mbpos = 0;

    for (int k = 0; k < NUM_4x4_BLOCKS_Y; k++)
      intra_modes[k] = (uint8_t) (mbmap[0].pred_intra_modes[k] = DC_PRED);
    mbmap[0].best_i8x8_mode_chroma = DC_PRED_8;

    taa_h264_set_mb_availability (0, 0, mbxmax, mbymax, 0, NULL, false, &mbmap[0].avail_flags);
    int num_bits = taa_h264_write_mb_header (&mbmap[0], writer, intra_modes, mbrun,
                                             islice, &test.last_quant);

    char result_code[64];
    taa_h264_code_buffer_to_string_ (
      writer, result_code, sizeof result_code / sizeof *result_code);

    snprintf (desc, sizeof(desc), "%s != %s", test.expect_code, result_code);
    FASSERT (num_bits == strlen (test.expect_code));
    FASSERT2 (strcmp (result_code, test.expect_code) == 0, desc);
  }

  taa_h264_bitwriter_delete (writer);
}


void test_enc_headers (void)
{
  test_mb_header_intra_4x4_all_pred ();
}
