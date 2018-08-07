#include "test_enc_speed.h"

#include "../../common/com_deblock.c"

//======================================================================================================
// Compile test using:
// icl -Qip -O2 -QxL -Qansi_alias -Qalias_args- -GX- -Qrestrict -TP /D UNIT_TEST /I api deblock.c /link /stack:100000000
// cl /Ox /Ob2 /Oi /Ot /Oy /GT /GL /GS- /GR- /arch:SSE2 /fp:fast -TP /D UNIT_TEST /I api -wd4068 deblock.c /link /stack:100000000
//======================================================================================================
#include <stdio.h>

#define SIZE (1280 * 720 * 16 * 4)


#define NUM_MBS_Y 45
#define NUM_MBS_X 80

int test_enc_deblock_speed()
{
  int num_mbs_x[] = {
    0, 8, 11, 22, 22, 32, 36, 40, 44, 48, 44, 50, 64, 64, 80
  };
  int num_mbs_y[] = {
    0, 6, 9, 15, 18, 18, 28, 30, 30, 28, 36, 38, 36, 48, 45
  };

  char * res_x[] = {
    "RES_SQCIF",
    "RES_QCIF",
    "RES_SIF",
    "RES_CIF",
    "RES_WCIF",
    "RES_25CIF",
    "RES_VGA",
    "RES_4SIF",
    "RES_W3CIF",
    "RES_4CIF",
    "RES_SVGA",
    "RES_W4CIF",
    "RES_XGA",
    "RES_720P",
    "RES_UNKNOWN"
  };

  int i, j, k;
  clock_t start, finish;
  const mb_deblock_t deblock_mb_strong =
  { .do_filter = 0xffffffff,
    .vert_strength1 = 0,
    .vert_strength2 = 0xffff,
    .horz_strength1 = 0,
    .horz_strength2 = 0xffff,
    .filter_offset_a = 0,
    .filter_offset_b = 0,
    .curr_qp = 26,
    .left_qp = 13,
    .top_qp = 23,
    .curr_qpc = 26,
    .left_qpc = 13,
    .top_qpc = 23};
  const mb_deblock_t deblock_mb_weak =
  { .do_filter = 0xffffeeee,
    .vert_strength1 = 0xeeee,
    .vert_strength2 = 0,
    .horz_strength1 = 0xbea9,
    .horz_strength2 = 0,
    .filter_offset_a = 0,
    .filter_offset_b = 0,
    .curr_qp = 26,
    .left_qp = 32,
    .top_qp = 12,
    .curr_qpc = 26,
    .left_qpc = 31,
    .top_qpc = 12};

  mb_deblock_t deblock_frame[NUM_MBS_X * NUM_MBS_Y];
  for(i = 0; i < NUM_MBS_X * NUM_MBS_Y; i += 2)
  {
    deblock_frame[i + 0] = deblock_mb_strong;
    deblock_frame[i + 1] = deblock_mb_weak;
  }

  TAA_H264_ALIGN (16) uint8_t src_y[SIZE];
  TAA_H264_ALIGN (16) uint8_t src_uv[SIZE];

  for(i = 0; i < SIZE; i++)
  {
    src_y[i] = (unsigned char)((255 * rand()) / RAND_MAX);
    src_uv[i] = (unsigned char)((255 * rand()) / RAND_MAX);
  }

  //============ LOOPFILTER SSSE3 ====================
  for (k = 1; k < 15; k++)
  {
    char name[64];
    //int sum = 0;
    start = clock();
    for (i = 0; i < 10; i++) {
      for (j = 0; j < 100; j++) {
        taa_h264_deblocking_filter (
          deblock_frame,
          src_y + 65536,
          src_uv + 65536,
          num_mbs_x[k],
          num_mbs_y[k],
          k);
        //sum = sum + src_y[10] + src_uv[10];
      }
    }
    finish = clock();
    snprintf (name, sizeof(name),
              "taa_h264_deblocking_filter_ssse3 %s", res_x[k - 1]);
    print_result (name, start, finish);
  }
  return(0);
}
