#include <stdio.h>
#include <stdlib.h>

#include "test_enc_speed.h"
#include "com_mbutils.h"
#include "com_resolutions.h"
#include "enc_motionsearch.c"
#include "com_interpolation.c"
#include "enc_interpolate.c"


#pragma warning (disable : 188)
#include "test_enc_me_speed_inc.h"
//#include "N:\\TANDBERG\\h264seq\\raw_video\\test_enc_me_speed_inc.h"

int test_enc_me_speed()
{
  mv_t best_mv1 = { 0, 0, 0};
  mv_t best_mv2 = { 0, 0, 0};
  mbtype_t best_mode;
  int luma_size;
  int chroma_size;
  int data_size;

  const int frame_width = 1280;
  const int frame_height = 720;
  const int padded_width = frame_width + 2 * PAD_HORZ_Y;
  const int padded_height = frame_height + 2 * PAD_VERT_Y;
  const source_res_t frameinfo_resolution = RES_720P;

  framebuf_t cam;
  framebuf_t ref_full;
  framebuf_t ref_half_diag;
  framebuf_t ref_half_horz;
  framebuf_t ref_half_vert;

  //============================================================/
  //                  Create CAM frame buffers
  //============================================================/
  cam.rows    = frame_height;
  cam.cols    = frame_width;
  cam.stride  = frame_width;
  cam.crows   = cam.rows / 2;
  cam.ccols   = cam.cols / 2;
  cam.cstride = cam.stride / 2;
  luma_size = cam.rows * cam.stride;
  chroma_size = cam.crows * cam.cstride;
  data_size = luma_size + 2 * chroma_size;
  cam.data = _mm_malloc(data_size, 64);
  cam.y = cam.data;
  cam.u = cam.data + luma_size;
  cam.v = cam.data + luma_size + chroma_size;
  FILE * fd;
  if (!(fd = fopen("cam_speed.yuv", "rb")))
  {
    perror ("Error opening camera file");
    exit (1);
  }

  if (fread(cam.y, 1, luma_size, fd) != luma_size)
  {
    perror ("Error reading camera Y");
    exit (1);
  }
  if (fread(cam.u, 1, chroma_size, fd) != chroma_size)
  {
    perror ("Error reading camera U");
    exit (1);
  }
  if (fread(cam.v, 1, chroma_size, fd) != chroma_size)
  {
    perror ("Error reading camera V");
    exit (1);
  }
  fclose(fd);

  //============================================================/
  //                  Create REF frame buffers
  //============================================================/
  ref_full.rows    = padded_height;
  ref_full.cols    = padded_width;
  ref_full.stride  = padded_width;
  ref_full.crows   = ref_full.rows / 2;
  ref_full.ccols   = ref_full.cols / 2;
  ref_full.cstride = ref_full.stride / 2;
  luma_size = ref_full.rows * ref_full.stride;
  chroma_size = ref_full.crows * ref_full.cstride;
  data_size = luma_size + 2 * chroma_size;
  ref_full.data = _mm_malloc (data_size, 64);
  ref_full.y = ref_full.data;
  ref_full.u = ref_full.data + luma_size;
  ref_full.v = ref_full.data + luma_size + chroma_size;
  if (!(fd = fopen("ref_speed.yuv", "rb")))
  {
    perror ("Error opening rebuf file");
    exit (1);
  }

  if (fread(ref_full.y, 1, luma_size, fd) != luma_size)
  {
    perror ("Error reading refbuf Y");
    exit (1);
  }
  if (fread(ref_full.u, 1, chroma_size, fd) != chroma_size)
  {
    perror ("Error reading refbuf U");
    exit (1);
  }
  if (fread(ref_full.v, 1, chroma_size, fd) != chroma_size)
  {
    perror ("Error reading refbuf V");
    exit (1);
  }
  fclose(fd);

  //============================================================/
  //                  Create halv pixel buffers
  //============================================================/
  ref_half_vert = ref_half_horz = ref_full;

  ref_half_horz.data = _mm_malloc(data_size, 64);
  ref_half_horz.y = ref_half_horz.data;
  ref_half_horz.u = ref_half_horz.data + luma_size;
  ref_half_horz.v = ref_half_horz.data + luma_size + chroma_size;

  ref_half_vert.data = _mm_malloc(data_size, 64);
  ref_half_vert.y = ref_half_vert.data;
  ref_half_vert.u = ref_half_vert.data + luma_size;
  ref_half_vert.v = ref_half_vert.data + luma_size + chroma_size;

  ref_half_diag.data = _mm_malloc(data_size, 64);
  ref_half_diag.y = ref_half_diag.data;
  ref_half_diag.u = ref_half_diag.data + luma_size;
  ref_half_diag.v = ref_half_diag.data + luma_size + chroma_size;

  //============================================================/
  //                  Create ME buffers
  //============================================================/
  meinfo_t * me = taa_h264_meinfo_create();

  //============================================================/
  //                          Initialization
  //============================================================/
  {
    int offset_full = ref_full.stride * PAD_VERT_Y + PAD_HORZ_Y;
    int offset_hor  = ref_half_horz.stride * PAD_VERT_Y + PAD_HORZ_Y;
    int offset_vert = ref_half_vert.stride * PAD_VERT_Y + PAD_HORZ_Y;
    //int padded_width = frameinfo_width + PAD_HORZ_Y * 2;

    taa_h264_hpel_interpolation (
      &ref_full.y [offset_full - PAD_HORZ_Y],
      &ref_full.y [offset_full - 3 * padded_width - PAD_HORZ_Y],
      &ref_half_diag.y [offset_vert - 3 * padded_width - PAD_HORZ_Y],
      &ref_half_horz.y [offset_hor - PAD_HORZ_Y],
      &ref_half_vert.y [offset_vert - 3 * padded_width - PAD_HORZ_Y],
      frame_height,
      padded_width,
      frameinfo_resolution);

    taa_h264_meinfo_init_frame (
      me,
      frame_width,
      frame_height,
      &ref_full,
      &ref_half_diag,
      &ref_half_horz,
      &ref_half_vert,
      0);
  }


  //============================================================/
  //                      Do motion search
  //============================================================/
  clock_t start, finish;
  start = clock();
    #ifndef NDEBUG
  for (int ii = 0; ii < 1; ii++)
    #else
  for (int ii = 0; ii < 1000; ii++)
    #endif
  {
    for (int ypos = 0; ypos < 720; ypos += 16)
    {
      for (int xpos = 0; xpos < 1280; xpos += 16)
      {
        int mbpos = xpos / 16 + 80 * ypos / 16;

        TAA_H264_ALIGN (64) uint8_t input_mb_base[2*256 + 4 * 64];
        uint8_t * input_mb_y = &input_mb_base[256+128];
        uint8_t * input_mb_u = &input_mb_base[2*256+128];
        uint8_t skip_y[256];
        uint8_t skip_u[64];

        uint8_t * mby = &cam.y[cam.stride  * ypos     + xpos  ];
        uint8_t * mbu = &cam.u[cam.cstride * ypos / 2 + xpos / 2];
        uint8_t * mbv = &cam.v[cam.cstride * ypos / 2 + xpos / 2];

        for (int i = 0; i < MB_HEIGHT_Y; i++)
        {
          for (int j = 0; j < MB_WIDTH_Y; j++)
            input_mb_y[i * MB_WIDTH_Y + j] = mby[i * cam.stride + j];
        }
        for (int i = 0; i < MB_HEIGHT_UV; i++)
        {
          for (int j = 0; j < MB_WIDTH_UV; j++)
          {
            input_mb_u[i * MB_STRIDE_UV + j] = mbu[i * cam.cstride + j];
            input_mb_u[i * MB_STRIDE_UV + j] = mbv[i * cam.cstride + j];
          }
        }

        if (motion_vectors_curr[mbpos * 16].ref_num != -1)
        {
          mv_t pred = {0, };
          mv_t cand[4] = {0, };
                    #ifndef NDEBUG
          bool failed = false;
                    #endif
          int quant = 26;
          taa_h264_motion_estimate(
            ref_full.stride,
            me,
            quant,
            &best_mv1,
            &best_mv2,
            &best_mode,
            pred,
            cand,
            input_mb_y,
            skip_y,
            skip_u
            );
                    #ifndef NDEBUG
          if (best_mode != mb_types[mbpos])
          {
            printf ("mbtype at pos %4d differ, %d != %d\n",
                    mbpos, best_mode, mb_types[mbpos]);
            failed = true;
          }

          if (best_mode > P_SKIP)
          {
            mv_t correct_mv1 = motion_vectors_curr[mbpos * 16];
            if (correct_mv1.x != best_mv1.x
                || correct_mv1.y != best_mv1.y)
            {
              printf ("mv1 at pos %4d differ (mode %d, correct (%d,%d), got (%d,%d))\n",
                      mbpos, best_mode, correct_mv1.x, correct_mv1.y, best_mv1.x, best_mv1.y);
              failed = true;
            }
          }

          if (best_mode > P_16x16)
          {
            int offset = best_mode == P_8x16 ? 2 : 8;
            mv_t correct_mv2 = motion_vectors_curr[mbpos * 16 + offset];

            if (correct_mv2.x != best_mv2.x
                || correct_mv2.y != best_mv2.y)
            {
              printf ("mv2 at pos %4d differ (mode %d, correct (%d,%d), got (%d,%d))\n",
                      mbpos, best_mode, correct_mv2.x, correct_mv2.y, best_mv2.x, best_mv2.y);
              failed = true;
            }
          }

          if (failed)
          {
            printf ("exiting me_speed...\n");
            return 1;
          }
                    #endif
        }
      }
    }
  }
  finish = clock();
  print_result ("ME search", start, finish);

  taa_h264_meinfo_delete(me);

  _mm_free (cam.data);
  _mm_free (ref_full.data);
  _mm_free (ref_half_horz.data);
  _mm_free (ref_half_vert.data);

  return(0);
}
