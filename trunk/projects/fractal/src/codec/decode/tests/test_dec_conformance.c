#include "fractaltest.h"
#include "md5.h"
#include "annexb.h"
#include "taah264dec.h"

#include <stdio.h>
#include <string.h>

// Disable warning to allow assert with descriptive text, eg FASSERT(!"foo")
#pragma diag_suppress 279

typedef struct  
{
  md5_state_t * md5_state;
  bool expect_no_errors;
} context;

static void report_error_cb (
  void *           ctx,
  taa_h264_error_t level,
  const char *     error_file,
  int              error_line,
  const char *     msg)
{
  context * c = (context *) ctx;
  if (c->expect_no_errors)
  {
    printf ("%s:%d %s\n", error_file, error_line, msg);
    FASSERT (!"Should never see decoder error/warning in conformance tests");
  }
}

static void report_frame_complete_cb (
  void *                           ctx,
  const taa_h264_decoded_frame_t * frame)
{
  md5_state_t * state = ((context *) ctx)->md5_state;

  int i;
  const uint8_t * y = frame->y;
  const uint8_t * u = frame->u;
  const uint8_t * v = frame->v;
  const int stride  = frame->stride;
  const int cstride = frame->stride_chroma;
  int rows    = frame->height;
  int cols    = frame->width;
  int crows   = frame->height / 2;
  int ccols   = frame->width / 2;

  if (frame->crop_left || frame->crop_right || frame->crop_top || frame->crop_bottom)
  {
    rows = frame->height - frame->crop_top - frame->crop_bottom;
    cols = frame->width - frame->crop_left - frame->crop_right;
    crows = rows / 2;
    ccols = cols / 2;

    y += stride * frame->crop_top;
    u += cstride * frame->crop_top / 2;
    v += cstride * frame->crop_top / 2;

    y += frame->crop_left;
    u += frame->crop_left / 2;
    v += frame->crop_left / 2;
  }

  for (i = 0; i < rows; i++)
    md5_append (state, &y[i * stride], cols);

  for (i = 0; i < crows; i++)
    md5_append (state, &u[i * cstride], ccols);

  for (i = 0; i < crows; i++)
    md5_append (state, &v[i * cstride], ccols);
}



static void decode_file (
  FILE * file,
  bool expect_no_errors,
  char * md5,
  int md5_maxlen)
{
  const int max_width = 1280;
  const int max_height = 720;
  taa_h264_dec_handle * decoder;
  annexb_context annexb;
  context cb_context;

  md5_state_t state;
  md5_byte_t digest[16];

  annexb_init (&annexb, file);
  md5_init(&state);

  cb_context.md5_state = &state;
  cb_context.expect_no_errors = expect_no_errors;

  taa_h264_dec_create_params cparams;
  cparams.width = max_width;
  cparams.height = max_height;
  cparams.callbacks.context = &cb_context;
  cparams.callbacks.report_error = report_error_cb;
  cparams.callbacks.report_frame_complete = report_frame_complete_cb;
  cparams.callbacks.received_sei_user_data = NULL;
  cparams.callbacks.reference_ack = NULL;

  decoder = taa_h264_dec_create (&cparams);

  while (extract_nalu (&annexb))
  {
    int happy;
    int nalu_ok;
    taa_h264_dec_exec_params eparams;
    eparams.nalu_buffer = annexb.nalu_start;
    eparams.nalu_size = annexb.nalu_size;

    nalu_ok = taa_h264_dec_execute (decoder, &eparams, &happy);
    if (expect_no_errors)
    {
      FASSERT (nalu_ok);
      FASSERT (happy);
    }
  }
  
  md5_finish (&state, digest);
  FASSERT (md5_maxlen > 32);
  for (int i = 0; i < 16; i++)
    sprintf(&md5[i*2], "%02x", digest[i]);

  taa_h264_dec_destroy (decoder);
  annexb_free (&annexb);
}

typedef struct
{
  const char * filename;
  const char * reference_md5;
  bool expect_no_errors;
} test_file;


static void test_decoder_conformance (void)
{
  test_file test_cases[] =
  {
    /* Generate */
    {"AUD_MW_E.264",                         "e96fe5054de0329a8868d06003375cdb", true},
    {"BA1_FT_C.264",                         "4f2da01d1d1ae7b99bea3fe1fb9e8ef4", true},
    {"BA1_Sony_D.jsv",                       "114d1cf94a2fcaffda0cf1b49964bf3d", true},
    {"BA2_Sony_F.jsv",                       "124d28307b05702812a374cfc9aad615", true},
    {"BA_MW_D.264",                          "7d5d351ad061640294bf43a43150fbca", true},
    {"BAMQ1_JVC_C.264",                      "bad372deef52c08fc1e384ecd1a43137", true},
    {"BAMQ2_JVC_C.264",                      "e3f5d5b0774b55370745f2d04f009575", true},
    {"BANM_MW_D.264",                        "e637d38ed004df3540218e3d84b43e42", true},
    {"BASQP1_Sony_C.jsv",                    "9e9c06cfc882a3f618b6ad40811c1331", true},
    {"CI1_FT_B.264",                         "6832762976b6d48719bb6cb603acd988", true},
    {"CI_MW_D.264",                          "037becca5bc836b869aba825293d39a3", true},
    {"CVFC1_Sony_C.jsv",                     "9fdb17e17d332b5d9752362c9c7ff9b0", true},
    {"CVPCMNL1_SVA_C.264",                   "5c1fd0f68e875200711febf1d683e58f", true},
    {"CVPCMNL2_SVA_C.264",                   "af1f1dbe1dd6569cb02271f053217d88", true},
    {"FM1_BT_B.h264",                        "f21ad956409cfa237099f7ac28390614", false},
    {"FM1_FT_E.264",                         "bf2d60e2a0140368e087580a8565fc99", false},
    {"FM2_SVA_B.264",                        "bd0ed007d1cba3746073cb82974a1191", false},
    {"FM2_SVA_C.264",                        "2ca2331ff02689d0cc036935a7f0d4bf", false},
    {"HCBP1_HHI_A.264",                      "13022e7970d78f1de4aaf1f7bd0e440b", false},
    {"HCBP2_HHI_A.264",                      "6c689d1541f97dcc1a17f5bdb6569cf1", false},
    {"LS_SVA_D.264",                         "9c53be4b1dedcd4598d30293f01c7bfe", true},
    {"MIDR_MW_D.264",                        "d87bff88b2c5b96ccb291ef68a45bbc2", true},
    {"MPS_MW_A.264",                         "88bb5a513bd7f3cc8190c7c03688ab22", true},
    {"MR1_BT_A.h264",                        "6ea31a214aadd8bdc8e7d37195d91c81", true},
    {"MR1_MW_A.264",                         "8c03b4a5b27a6f594d917d6fee1d86e6", true},
    {"MR2_MW_A.264",                         "20e66bac06e537fb1d2fa949b28046cd", true},
    {"MR2_TANDBERG_E.264",                   "d154bf9264960fecc6d2cf72be4cf8cc", true},
    {"MR3_TANDBERG_B.264",                   "dd1091f75ac15d3ae70485295b0a116e", true},
    {"MR4_TANDBERG_C.264",                   "5f5f14beca549172487bb74c4d3ba8f9", false},
    {"MR5_TANDBERG_C.264",                   "5f5f14beca549172487bb74c4d3ba8f9", false},
    {"NL1_Sony_D.jsv",                       "d4bb8d980c1377ee45515763ae7989fd", true},
    {"NL2_Sony_H.jsv",                       "48d8380cdb7eff52116c1aaddbc583f5", true},
    {"NLMQ1_JVC_C.264",                      "5c4a2f6b39385805f480a3a4432873b2", true},
    {"NLMQ2_JVC_C.264",                      "90b70fbaa5ca679ec9bf5e011ddba8f9", true},
    {"NRF_MW_E.264",                         "a8635615b50c5a16decc555a3c6c81c8", true},
    {"SVA_BA1_B.264",                        "dab92aa2145ab44abab2beb2868dd326", true},
    {"SVA_BA2_D.264",                        "66130b14295574bf35b725a8eaded3ae", true},
    {"SVA_Base_B.264",                       "180dda3234bcbe57fc45587dac7d43fb", true},
    {"SVA_CL1_E.264",                        "5723a1518de9fadca7499c5ba34da7c4", true},
    {"SVA_FM1_E.264",                        "7f7eaf6107852b871a3894a950e3647e", true},
    {"SVA_NL1_B.264",                        "b5626983ac0877497fff9a4b10d2f1d4", true},
    {"SVA_NL2_E.264",                        "b47e932d436288013b8453d9a1d0f60d", true},
    {"UNOFFICIAL_C20_PADDING.264",           "095078d34b5a9a080253c92a7dbe0b17", true},
    {"UNOFFICIAL_EX90_DISPOSABLE.264",       "882b3b3935b9b696ad856bcad713226e", true},
    {"UNOFFICIAL_MXP3000.264",               "95fc58311d25fa3d8729ad1ddae9d7a5", true},
  };

  int n = sizeof(test_cases) / sizeof(test_cases[0]);
  for (int i = 0; i < n; i++)
  {
    FILE * file;
    char result_md5[32+1];
    test_file * test = &test_cases[i];

    file = fopen (test->filename, "rb");
    if (!file)
      perror (test->filename);
    FASSERT(file != NULL);

    decode_file (file, test->expect_no_errors, result_md5, sizeof result_md5);

    // Ideally every sequence would be decoded perfectly, but it's not.
    // A couple of features are not implemented, such as FMO and 
    // "encoding order != display order". Assert that decoding the sequences has
    // the expected outcome, even if it means a different md5.
    if (test->expect_no_errors)
      FASSERT (strcmp (result_md5, test->reference_md5) == 0);
    else
      FASSERT (strcmp (result_md5, test->reference_md5) != 0);

    fclose (file);
  }
}

void test_dec_conformance (void)
{
  test_decoder_conformance ();
}
