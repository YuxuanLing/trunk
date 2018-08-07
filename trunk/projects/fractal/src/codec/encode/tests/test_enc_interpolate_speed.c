#include "test_enc_speed.h"

#include "../enc_interpolate.c"

/* ====================================================================================================== */
/* Compile test using: */
/* icl -Qip -O2 -QxL -Qansi_alias -Qalias_args- -GX- -Qrestrict -TP /D UNIT_TEST /I api interpolate.c /link /stack:64000000 */
/* cl /Ox /Ob2 /Oi /Ot /Oy /GT /GL /GS- /GR- /arch:SSE2 /fp:fast -TP /D UNIT_TEST /I api -wd4068 interpolate.c /link /stack:64000000 */
/* ====================================================================================================== */
#include <stdlib.h>

#define SIZE (1280 * 720 * 16)


int test_enc_interpolate_speed()
{
#if 0
  int i, j;
  clock_t start, finish;
  TAA_H264_ALIGN (16) unsigned char A_src1[SIZE];
  TAA_H264_ALIGN (16) unsigned char B_dst1[SIZE];
  TAA_H264_ALIGN (16) unsigned char A_src2[SIZE];
  TAA_H264_ALIGN (16) unsigned char B_dst2[SIZE];
  for(i = 0; i < SIZE; i++)
  {
    A_src1[i] = (unsigned char)((255 * rand()) / RAND_MAX);
  }
  // =================== TAA_H264_SQCIF_HPEL_INTERPOLATION ================
  start = clock();
  for(i = 0; i < 10; i++)
    for(j = 0; j < 100; j++)
      TAA_H264_SQCIF_HPEL_INTERPOLATION (
        A_src1 + 65536, B_dst1 + 65536, A_src2 + 65536, B_dst2 + 65536);
  finish = clock();
  print_result ("TAA_H264_SQCIF_HPEL_INTERPOLATION", start, finish);

  // =================== TAA_H264_QCIF_HPEL_INTERPOLATION =================
  start = clock();
  for(i = 0; i < 10; i++)
    for(j = 0; j < 100; j++)
      TAA_H264_QCIF_HPEL_INTERPOLATION (
        A_src1 + 65536, B_dst1 + 65536, A_src2 + 65536, B_dst2 + 65536);
  finish = clock();
  print_result ("TAA_H264_QCIF_HPEL_INTERPOLATION", start, finish);

  // =================== TAA_H264_SIF_HPEL_INTERPOLATION ==================
  start = clock();
  for(i = 0; i < 10; i++)
    for(j = 0; j < 100; j++)
      TAA_H264_SIF_HPEL_INTERPOLATION (
        A_src1 + 65536, B_dst1 + 65536, A_src2 + 65536, B_dst2 + 65536);
  finish = clock();
  print_result ("TAA_H264_SIF_HPEL_INTERPOLATION", start, finish);

  // =================== TAA_H264_CIF_HPEL_INTERPOLATION ==================
  start = clock();
  for(i = 0; i < 10; i++)
    for(j = 0; j < 100; j++)
      TAA_H264_CIF_HPEL_INTERPOLATION (
        A_src1 + 65536, B_dst1 + 65536, A_src2 + 65536, B_dst2 + 65536);
  finish = clock();
  print_result ("TAA_H264_CIF_HPEL_INTERPOLATION", start, finish);

  // =================== TAA_H264_WCIF_HPEL_INTERPOLATION =================
  start = clock();
  for(i = 0; i < 10; i++)
    for(j = 0; j < 100; j++)
      TAA_H264_WCIF_HPEL_INTERPOLATION (
        A_src1 + 65536, B_dst1 + 65536, A_src2 + 65536, B_dst2 + 65536);
  finish = clock();
  print_result ("TAA_H264_WCIF_HPEL_INTERPOLATION", start, finish);

  // =================== TAA_H264_25CIF_HPEL_INTERPOLATION ================
  start = clock();
  for(i = 0; i < 10; i++)
    for(j = 0; j < 100; j++)
      TAA_H264_25CIF_HPEL_INTERPOLATION (
        A_src1 + 65536, B_dst1 + 65536, A_src2 + 65536, B_dst2 + 65536);
  finish = clock();
  print_result ("TAA_H264_25CIF_HPEL_INTERPOLATION", start, finish);

  // =================== TAA_H264_VGA_HPEL_INTERPOLATION ==================
  start = clock();
  for(i = 0; i < 10; i++)
    for(j = 0; j < 100; j++)
      TAA_H264_VGA_HPEL_INTERPOLATION (
        A_src1 + 65536, B_dst1 + 65536, A_src2 + 65536, B_dst2 + 65536);
  finish = clock();
  print_result ("TAA_H264_VGA_HPEL_INTERPOLATION", start, finish);

  // =================== TAA_H264_4SIF_HPEL_INTERPOLATION =================
  start = clock();
  for(i = 0; i < 10; i++)
    for(j = 0; j < 100; j++)
      TAA_H264_4SIF_HPEL_INTERPOLATION (
        A_src1 + 65536, B_dst1 + 65536, A_src2 + 65536, B_dst2 + 65536);
  finish = clock();
  print_result ("TAA_H264_4SIF_HPEL_INTERPOLATION", start, finish);

  // =================== TAA_H264_W3CIF_HPEL_INTERPOLATION ================
  start = clock();
  for(i = 0; i < 10; i++)
    for(j = 0; j < 100; j++)
      TAA_H264_W3CIF_HPEL_INTERPOLATION (
        A_src1 + 65536, B_dst1 + 65536, A_src2 + 65536, B_dst2 + 65536);
  finish = clock();
  print_result ("TAA_H264_W3CIF_HPEL_INTERPOLATION", start, finish);

  // =================== TAA_H264_4CIF_HPEL_INTERPOLATION =================
  start = clock();
  for(i = 0; i < 10; i++)
    for(j = 0; j < 100; j++)
      TAA_H264_4CIF_HPEL_INTERPOLATION (
        A_src1 + 65536, B_dst1 + 65536, A_src2 + 65536, B_dst2 + 65536);
  finish = clock();
  print_result ("TAA_H264_4CIF_HPEL_INTERPOLATION", start, finish);

  // =================== TAA_H264_SVGA_HPEL_INTERPOLATION =================
  start = clock();
  for(i = 0; i < 10; i++)
    for(j = 0; j < 100; j++)
      TAA_H264_SVGA_HPEL_INTERPOLATION (
        A_src1 + 65536, B_dst1 + 65536, A_src2 + 65536, B_dst2 + 65536);
  finish = clock();
  print_result ("TAA_H264_SVGA_HPEL_INTERPOLATION", start, finish);

  // =================== TAA_H264_W4CIF_HPEL_INTERPOLATION ================
  start = clock();
  for(i = 0; i < 10; i++)
    for(j = 0; j < 100; j++)
      TAA_H264_W4CIF_HPEL_INTERPOLATION (
        A_src1 + 65536, B_dst1 + 65536, A_src2 + 65536, B_dst2 + 65536);
  finish = clock();
  print_result ("TAA_H264_W4CIF_HPEL_INTERPOLATION", start, finish);

  // =================== TAA_H264_XGA_HPEL_INTERPOLATION ==================
  start = clock();
  for(i = 0; i < 10; i++)
    for(j = 0; j < 100; j++)
      TAA_H264_XGA_HPEL_INTERPOLATION (
        A_src1 + 65536, B_dst1 + 65536, A_src2 + 65536, B_dst2 + 65536);
  finish = clock();
  print_result ("TAA_H264_XGA_HPEL_INTERPOLATION", start, finish);

  // =================== TAA_H264_720P_HPEL_INTERPOLATION =================
  start = clock();
  for(i = 0; i < 10; i++)
    for(j = 0; j < 100; j++)
      TAA_H264_720P_HPEL_INTERPOLATION (
        A_src1 + 65536, B_dst1 + 65536, A_src2 + 65536, B_dst2 + 65536);
  finish = clock();
  print_result ("TAA_H264_720P_HPEL_INTERPOLATION", start, finish);

  // =================== TAA_H264_UNKNOWN_HPEL_INTERPOLATION ================
  start = clock();
  for(i = 0; i < 10; i++)
    for(j = 0; j < 100; j++)
      TAA_H264_UNKNOWN_HPEL_INTERPOLATION (
        600, 800, A_src1 + 65536, B_dst1 + 65536, A_src2 + 65536, B_dst2 + 65536);
  finish = clock();
  print_result ("TAA_H264_UNKNOWN_HPEL_INTERPOLATION", start, finish);
#endif
  return(0);
}
