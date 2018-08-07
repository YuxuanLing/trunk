#include "taah264stdtypes.h"
#include "encoder.h"
#include "enc_load_mb.h"
#include "com_cpuinfo.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <emmintrin.h>

#ifdef _WIN32
# include <windows.h>
#else
static bool IsDebuggerPresent ()
{
  return false;
}
#endif

//#define DEBUG_PADDING
#define ROUND_UP_16(x) (((x) + 15) & (~0xF))

typedef struct
{
  int width_in;
  int width_out;
  int height_in;
  int height_out;
  const char * filename_in;
  const char * filename_out;
} args_t;

static bool parse_args (int      argc,
                        char **  argv,
                        args_t * args)
{
  if (argc < 6)
  {
    const char * progname = argv[0];
    printf ("Usage: %s width_in height_in width_out height_out file_in file_out\n",
            progname);
    return false;
  }
  args->width_in = atoi (argv[1]);
  args->height_in = atoi (argv[2]);
  args->width_out = atoi (argv[3]);
  args->height_out = atoi (argv[4]);
  args->filename_in = argv[5];
  args->filename_out = argv[6];

  return true;
}

static bool validate_args (const args_t * args)
{
  if ((args->width_out > args->width_in) ||
      (args->height_out > args->height_in))
  {
    printf ("Only downscaling is supported\n");
    return false;
  }

  return true;
}

static int get_file_size (FILE * f)
{
  fpos_t fpos[1];
  long pos1, pos2;

  fgetpos (f, fpos);
  pos1 = ftell (f);
  fseek (f, 0, SEEK_END);
  pos2 = ftell (f);
  fsetpos (f, fpos);
  return pos2 - pos1;
}

static void save_mb (
  const uint8_t * mb_y,
  const uint8_t * mb_uv,
  int             width,
  int             height,
  int             xpos,
  int             ypos,
  uint8_t *       framedst)
{
  bool crop_x = false;
  bool crop_y = false;

#ifdef DEBUG_PADDING
  width = ROUND_UP_16 (width);
  height = ROUND_UP_16 (height);
#endif

  uint8_t * mbdst_y = framedst + ypos * width + xpos;
  uint8_t * mbdst_u = framedst + height * width + (ypos/2) * (width/2) + xpos/2;
  uint8_t * mbdst_v = framedst + height * width * 5 / 4 + (ypos/2) * (width/2) + xpos/2;

  if (xpos + MB_WIDTH_Y > width)
    crop_x = true;
  if (ypos + MB_HEIGHT_Y > height)
    crop_y = true;

  if (!crop_x && !crop_y)
  {
    for (int i = 0; i < MB_HEIGHT_Y; i++)
    {
      memcpy (&mbdst_y[i * width], &mb_y[MB_WIDTH_Y * i], MB_WIDTH_Y);
    }

    for (int i = 0; i < MB_HEIGHT_UV; i++)
    {
      memcpy (&mbdst_u[i * width / 2], &mb_uv[MB_STRIDE_UV * i              ], MB_WIDTH_UV);
      memcpy (&mbdst_v[i * width / 2], &mb_uv[MB_STRIDE_UV * i + MB_WIDTH_UV], MB_WIDTH_UV);
    }
  }
  else
  {
    const int cropsize_x = (crop_x) ? (xpos + MB_WIDTH_Y)  - width  : 0;
    const int cropsize_y = (crop_y) ? (ypos + MB_HEIGHT_Y) - height : 0;

    for (int i = 0; i < MB_HEIGHT_Y - cropsize_y; i++)
    {
      memcpy (&mbdst_y[i * width], &mb_y[MB_WIDTH_Y * i], MB_WIDTH_Y - cropsize_x);
    }

    for (int i = 0; i < MB_HEIGHT_UV - cropsize_y/2; i++)
    {
      memcpy (&mbdst_u[i * width / 2], &mb_uv[MB_STRIDE_UV * i              ], MB_WIDTH_UV - cropsize_x/2);
      memcpy (&mbdst_v[i * width / 2], &mb_uv[MB_STRIDE_UV * i + MB_WIDTH_UV], MB_WIDTH_UV - cropsize_x/2);
    }
  }
}


int main (int     argc,
          char ** argv)
{
  args_t args;
  FILE * file_in;
  FILE * file_out;

  if (!parse_args (argc, argv, &args))
  {
    fprintf (stderr, "Invalid arguments\n");
    return 1;
  }

  if (!validate_args (&args))
  {
    fprintf (stderr, "Invalid arguments\n");
    return 2;
  }


  /* Open files */
  if (!(file_in = fopen(args.filename_in, "rb")))
  {
    fprintf(stderr, "Could not open input file `%s'\n", args.filename_in);
    return 1;
  }
  if (!(file_out = fopen(args.filename_out, "wb")))
  {
    fprintf(stderr, "Could not open output file `%s'\n", args.filename_out);
    return 1;
  }
  __declspec(align(16)) uint8_t mb_y[256] = {0, };
  __declspec(align(16)) uint8_t mb_uv[128] = {0, };

  const int filesize = get_file_size (file_in);
  const int framesize_in  = args.width_in  * args.height_in  * 3 / 2;
#ifndef DEBUG_PADDING
  const int framesize_out = args.width_out * args.height_out * 3 / 2;
#else
  const int framesize_out =
    ROUND_UP_16 (args.width_out) * ROUND_UP_16 (args.height_out) * 3 / 2;
#endif
  const int max_frames = filesize / framesize_in;
  const int max_mbx = (args.width_out + 15) / 16;
  const int max_mby = (args.height_out + 15) / 16;

  //printf ("Input:  %-50s %4d x %-4d   %4d frames\n",
  //        args.filename_in, args.width_in, args.height_in, max_frames);
  //printf ("Output: %-50s %4d x %-4d\n",
  //        args.filename_out, args.width_out, args.height_out);

  clock_t start, finish;
  float duration;

  // Load the whole sequence at once
  unsigned char * sequence_in  = _mm_malloc (framesize_in  * max_frames, 16);
  unsigned char * sequence_out = _mm_malloc (framesize_out * max_frames, 16);
  fread (sequence_in, 1, framesize_in * max_frames, file_in);

  const int width_in   = args.width_in;
  const int height_in  = args.height_in;
  const int width_out  = args.width_out;
  const int height_out = args.height_out;
  const int ysize_in   = width_in * height_in;
  const int csize_in   = ysize_in / 4;
  const int cwidth_in  = width_in / 2;

//========================================================
//  Setup scale factors and calculate filter coefficients
//========================================================
  const int fact_x = width_in   * RSA / width_out;
  const int fact_y = height_in  * RSA / height_out;
  const int fact   = height_in  * RSA / height_out;
  const int MBX_IN = fact_x>>(RSA_SHFT-5);
  const int MBY_IN = fact_y>>(RSA_SHFT-5);

  if (fact_x != fact)
  {
    printf("Error: cannot alter aspect ratio!\n");
    return(0);
  }

  resizer_t * resizer = taa_h264_resizer_create ();
  taa_h264_resizer_init (resizer, fact);
  cpuinfo_t cpuinfo;
  taa_h264_get_cpuinfo (&cpuinfo);

//========================================================

  start = clock ();
  for (int frame = 0; frame < max_frames; frame++)
  {
    const uint8_t * input = &sequence_in[frame * framesize_in];
    uint8_t * dst = &sequence_out[frame * framesize_out];

    for (int mby = 0; mby < max_mby; mby++)
    {
      const int fineV = 2 * (16 * mby) * fact - RSA + fact;
      const int ypos_in  = (MBY_IN==64) ? 64*mby : (fineV >> RSA_SHFT);
      const int cypos_in = ypos_in >> 1;

      for (int mbx = 0; mbx < max_mbx; mbx++)
      {
        const int fineH = 2 * (16 * mbx) * fact - RSA + fact;
        const int xpos_in  = (MBX_IN==64) ? 64*mbx : (fineH >> RSA_SHFT);
        const int cxpos_in = xpos_in >> 1;

        const uint8_t * in_y = input + ypos_in * width_in + xpos_in;
        const uint8_t * in_u = input + ysize_in + cypos_in * cwidth_in + cxpos_in;
        const uint8_t * in_v = input + ysize_in + csize_in + cypos_in * cwidth_in + cxpos_in;

        const int xpos_out = mbx * MB_WIDTH_Y;
        const int ypos_out = mby * MB_HEIGHT_Y;

        taa_h264_load_resized_mb (
          resizer, &cpuinfo, in_y, in_u, in_v, mb_y, mb_uv, width_in, height_in,
          xpos_in, ypos_in, fineH, fineV);
        save_mb (mb_y, mb_uv, width_out, height_out, xpos_out, ypos_out, dst);
      }
    }
  }
  finish = clock ();
  duration = ((float) (finish - start)) / CLOCKS_PER_SEC;
  printf ("Time used: %.3f sec (%d frames, %dx%d -> %dx%d)\n",
          duration, max_frames, width_in, height_in, width_out, height_out);

  // Write scaled sequence to file
  fwrite (sequence_out, sizeof (uint8_t), max_frames * framesize_out, file_out);

  fclose (file_in);
  fclose (file_out);
  taa_h264_resizer_delete(resizer);

  if (IsDebuggerPresent())
  {
    printf ("Press ENTER to continue.\n");
    getc (stdin);
  }

  return 0;
}
