#include "taah264enc.h"
#include "ssim.h"

#include "enc.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* included last since it redefines getopt-stuff */
#include "getopt.h"

#ifdef WIN32
# define CATCH_UNHANDLED_EXCEPTIONS
#endif

static char * DEFAULT_FILENAME_LOG = "encoder.log";
static const int MAX_NALU_SIZE = 1450;

typedef struct context_t
{
  FILE * nalout;
  FILE * errout;
  int framebytes;
} context_t;

static void help (
  const char * progname)
{
  printf ("Usage: %s [options] INFILE OUTFILE FRAMES\n"
          "  Options:\n"
          "    -h, --help              show this help message\n"
          "    -q QP,\n"
          "    --qp QP                 encode with quantization parameter QP\n"
          "    -b RATE,\n"
          "    --bitrate RATE          encode with bitrate RATE (kbit/s) (N/A)\n"
          "    -x RES,\n"
          "    --resolution RES        the encoded resolution.\n"
          "                            RES is WIDTHxHEIGHT, e.g. 1280x720, or a special number:\n"
          "                              - 2  CIF\n"
          "                              - 3  720p\n"
          "    --input-width WIDTH     width of input file if different from encoded width\n"
          "    --input-height HEGIHT   height of input file if different from encoded height\n"
          "    -f FPS\n"
          "    --framerate FPS         frame rate used for rate control/calculation\n"
          "    -i N\n"
          "    --intra-period N        force an intra frame every N frames\n"
          "                            N = 0 disables periodic intra frames\n"
          "    --gop-size SIZE         distance between two frames of lowest temporal level\n"
          "    --t-levels N            number of temporal levels\n"
          "    --max-ref-frames N      number of allocated reference frames\n"
          "                            N = 0 tries to guess the number based on temporal levels\n"
          "    -c FLAGS\n"
          "    --coding-options FLAGS  set FLAGS to enable/disable coding algorithms, see header file\n"
          "    -r FILE,\n"
          "    --reconstruct FILE      write reconstructed frames to FILE\n"
          "    -l FILE,\n"
          "    --log FILE              write log/trace to FILE\n"
          "    -t MASK,\n"
          "    --trace MASK            print traces matching MASK to log-file\n"
          "                            MASK is a mask of the following values:\n"
          "                              - 0  NONE\n"
          "                              - 1  KEY_VALUES\n"
          "                              - 2  HEADERS\n"
          "                              - 4  CAVLC\n"
          "                              - 8  MV\n"
          "                              - 16 DPB\n"
          "    --no-psnr               don't calculate PSNR\n"
          "    -e,\n"
          "    --handle-exceptions     catch unhandled exceptions, Windows only\n"
          "\n"
          "For maximum speed, parameter `--no-psnr' should be specified and\n"
          "`-r' should not.\n",
          progname);
}

static void usage(
  const char * progname)
{
  fprintf(stderr,
          "Summary: %s [-help] [options] INFILE OUTFILE FRAMES\n",
          progname);
}

static void write_frame (
  framebuf_t * buf,
  FILE *       f)
{
  int i;
  uint8_t * y = buf->y;
  uint8_t * u = buf->u;
  uint8_t * v = buf->v;
  int rows    = buf->rows;
  int cols    = buf->cols;
  int crows   = buf->crows;
  int ccols   = buf->ccols;
  int stride  = buf->stride;
  int cstride = buf->cstride;

  for (i = 0; i < rows; i++)
  {
    if (fwrite (&y[i * stride], sizeof(uint8_t), cols, f) != cols)
      fprintf (stderr, "Error writing Y to file\n");
  }

  for (i = 0; i < crows; i++)
  {
    if (fwrite (&u[i * cstride], sizeof(uint8_t), ccols, f) != ccols)
      fprintf (stderr, "Error writing U to file\n");
  }

  for (i = 0; i < crows; i++)
  {
    if (fwrite (&v[i * cstride], sizeof(uint8_t), ccols, f) != ccols)
      fprintf (stderr, "Error writing V to file\n");
  }
}

static float snr (
  uint8_t * data1,
  uint8_t * data2,
  const int width,
  const int height,
  const int stride)
{
  float psnr;
  float sumsqr = 0;

  for (int i = 0; i < height; i++)
  {
#pragma unroll(8)
#pragma ivdep
#pragma vector aligned
    for (int j = 0; j < width; j++)
    {
      float ival = (float) (data1[i * width + j] - data2[i * stride + j]);
      sumsqr += (ival * ival);
    }
  }
  psnr = ((float) sumsqr) / (65025.0f * width * height);
  psnr = -10.0f * log10f(psnr);

  return psnr;
}


#ifdef CATCH_UNHANDLED_EXCEPTIONS
# include <windows.h>
LONG __stdcall unhandled_exceptions_filter (
  EXCEPTION_POINTERS * pep )
{
  fprintf (stderr, "An unhandled exception occured. Fatal!\n");
  return EXCEPTION_EXECUTE_HANDLER;
}
#endif

static bool is_numeric(char x)
{
  return (x >= '0') && (x <= '9');
}

static bool infer_size(
  const char * a,
  int *        width,
  int *        height)
{
  char arg[16];
  strcpy (arg, a);
  int len = strlen(arg);
  int i;
  for(i = len-1; i>=1; i = i-1)
  {
    if ((arg[i]=='x') && is_numeric(arg[i-1]) && is_numeric(arg[i+1]))
      break;
  }
  if (i == 0)
    return false;

  arg[i] = '\0';
  *width = atoi (&arg[0]);
  *height = atoi (&arg[i+1]);

  return true;
}

static const int NO_VALUE = -999;

static bool parse_options (
  const char * progname,
  int          argc,
  char *       argv[],
  bool *       do_psnr,
  int *        width,
  int *        height,
  int *        input_width,
  int *        input_height,
  int *        framerate,
  int *        bitrate,
  int *        qp,
  int *        intra_period,
  int *        gop_size,
  int *        t_levels,
  int *        max_ref_frames,
  unsigned *   coding_options,
  int *        trace,
  char * *     filename_log_ptr,
  char * *     filename_rec_ptr,
  char * *     filename_in_ptr,
  char * *     filename_out_ptr,
  int *        num_frames)
{
  char * shortopts = "hq:b:x:r:l:t:e";
  struct option longopts[] =
  {
    /* name,        has_arg,           flag, val */         /* longind */
    { "no-psnr",        no_argument,       0,     0  },         /*       0 */
    { "gop-size",       required_argument, 0,     0  },         /*       1 */
    { "t-levels",       required_argument, 0,     0  },         /*       2 */
    { "max-ref-frames", required_argument, 0,     0  },         /*       3 */
    { "input-width",    required_argument, 0,     0  },         /*       4 */
    { "input-height",   required_argument, 0,     0  },         /*       5 */
    { "help",           no_argument,       0,    'h' },
    { "resolution",     required_argument, 0,    'x' },
    { "framerate",      required_argument, 0,    'f' },
    { "bitrate",        required_argument, 0,    'b' },
    { "qp",             required_argument, 0,    'q' },
    { "intra-period",   required_argument, 0,    'i' },
    { "coding-options", required_argument, 0,    'c' },
    { "trace",          required_argument, 0,    't' },
    { "reconstruct",    required_argument, 0,    'r' },
    { "log",            required_argument, 0,    'l' },
    { "handle-exceptions", no_argument,    0,    'e' },
    /* end-of-list marker */
    { 0, 0, 0, 0 }
  };
  int opt;
  int longind = 0;

  bool logfile = false;
  bool recfile = false;

  *do_psnr = true;
  *width = 0;
  *height = 0;
  *input_width = 0;
  *input_height = 0;
  *framerate = 30;
  *bitrate = NO_VALUE;
  *qp = NO_VALUE;
  *intra_period = 0;
  *gop_size = 1;
  *t_levels = 1;
  *max_ref_frames = 0;
  *trace = 0;
  *coding_options = 0;

  /* Parse optional arguments */
  while ((opt = getopt_long_only(argc, argv, shortopts, longopts, &longind)) != -1)
  {
    switch (opt)
    {
    case 0:         /* a long option without an equivalent short option */
      switch (longind)
      {
      case 0:                  /* -no-psnr */
        *do_psnr = false;
        break;
      case 1:                  /* -gop-size */
      {
        char ignored;
        if (sscanf(optarg, "%d%c", gop_size, &ignored) != 1)
        {
          fprintf(stderr, "%s: gop-size `%s' is not a number\n",
                  progname, optarg);
          usage(progname);
          return false;
        }
        break;
      }
      case 2:                  /* t-levels */
      {
        char ignored;
        if (sscanf(optarg, "%d%c", t_levels, &ignored) != 1)
        {
          fprintf(stderr, "%s: t-levels `%s' is not a number\n",
                  progname, optarg);
          usage(progname);
          return false;
        }
        break;
      }
      case 3:                  /* max-ref-frames */
      {
        char ignored;
        if (sscanf(optarg, "%d%c", max_ref_frames, &ignored) != 1)
        {
          fprintf(stderr, "%s: max-ref-frames `%s' is not a number\n",
                  progname, optarg);
          usage(progname);
          return false;
        }
        break;
      }
      case 4:                  /* input-width */
      {
        char ignored;
        if (sscanf(optarg, "%d%c", input_width, &ignored) != 1)
        {
          fprintf(stderr, "%s: input-width `%s' is not a number\n",
                  progname, optarg);
          usage(progname);
          return false;
        }
        break;
      }
      case 5:                  /* input-height */
      {
        char ignored;
        if (sscanf(optarg, "%d%c", input_height, &ignored) != 1)
        {
          fprintf(stderr, "%s: input-height `%s' is not a number\n",
                  progname, optarg);
          usage(progname);
          return false;
        }
        break;
      }
      default:               /* something unexpected has happened */
        fprintf(stderr,
                "%s: "
                "getopt_long_only unexpectedly returned %d for `--%s'\n",
                progname,
                opt,
                longopts[longind].name);
        return false;
      }
      break;
    case 'h':         /* -help */
      help(progname);
      return false;
    case 'x':         /* -resolution=RES */
    {
      char ignored;
      if (!infer_size (optarg, width, height))
      {
        int resolution;
        if (sscanf(optarg, "%d%c", &resolution, &ignored) == 1)
        {
          if (resolution == 2)
          {
            *width = 352;
            *height = 288;
          }
          else if (resolution == 3)
          {
            *width = 1280;
            *height = 720;
          }
          else
          {
            fprintf (stderr, "could not resolve resolution\n");
            usage (progname);
            return false;
          }
        }
        else
        {
          fprintf (stderr, "could not resolve resolution\n");
          usage (progname);
          return false;
        }
      }
      break;
    }
    case 'f':         /* -framerate=FPS */
    {
      char ignored;
      if (sscanf(optarg, "%d%c", framerate, &ignored) != 1)
      {
        fprintf(stderr, "%s: framerate `%s' is not a number\n",
                progname, optarg);
        usage(progname);
        return false;
      }
      break;
    }
    case 'b':         /* -bitrate=RATE */
    {
      char ignored;
      if (sscanf(optarg, "%d%c", bitrate, &ignored) != 1)
      {
        fprintf(stderr, "%s: bitrate `%s' is not a number\n",
                progname, optarg);
        usage(progname);
        return false;
      }
      break;
    }
    case 'q':         /* -qp=QP */
    {
      char ignored;
      if (sscanf(optarg, "%d%c", qp, &ignored) != 1)
      {
        fprintf(stderr, "%s: qp `%s' is not a number\n",
                progname, optarg);
        usage(progname);
        return false;
      }
      break;
    }
    case 'i':         /* -intraperiod=N */
    {
      char ignored;
      if (sscanf(optarg, "%d%c", intra_period, &ignored) != 1)
      {
        fprintf(stderr, "%s: intraperiod `%s' is not a number\n",
                progname, optarg);
        usage(progname);
        return false;
      }
      break;
    }
    case 'c':         /* -coding-options=FLAGS */
      {
        char ignored;
        if (sscanf(optarg, "%u%c", coding_options, &ignored) != 1)
        {
          fprintf(stderr, "%s: coding_options `%s' is not a number\n",
            progname, optarg);
          usage(progname);
          return false;
        }
        break;
      }
    case 't':         /* -trace=MASK */
    {
      char ignored;
      if (sscanf(optarg, "%d%c", trace, &ignored) != 1)
      {
        fprintf(stderr, "%s: trace `%s' is not a number\n",
                progname, optarg);
        usage(progname);
        return false;
      }
      break;
    }
    case 'r':         /* -reconstruct=FILE */
    {
      *filename_rec_ptr = optarg;
      recfile = true;
      break;
    }
    case 'l':         /* -log=FILE */
    {
      *filename_log_ptr = optarg;
      logfile = true;
      break;
    }
    case 'e':         /* -handle-exceptions */
    {
            #ifdef CATCH_UNHANDLED_EXCEPTIONS
      SetUnhandledExceptionFilter(unhandled_exceptions_filter);
            #endif
      break;
    }
    case '?':         /* getopt_long_only noticed an error */
      usage(progname);
      return false;
    default:         /* something unexpected has happened */
      fprintf(stderr, "%s: getopt_long_only returned an unexpected value (%d)\n",
              progname, opt);
      return false;
    }
  }

  if (!recfile)
    *filename_rec_ptr = NULL;
  if (!logfile)
    *filename_log_ptr = NULL;

  if (*max_ref_frames == 0)
    *max_ref_frames = *t_levels;

  /* Parse mandatory arguments */
  if (argc - optind != 3)
  {
    usage (progname);
    return false;
  }

  *filename_in_ptr = argv[optind++];
  *filename_out_ptr = argv[optind++];
  *num_frames = atoi (argv[optind++]);

  return true;
}


static bool validate_options (
  int   width,
  int   height,
  int * input_width,
  int * input_height,
  int * bitrate,
  int * qp)
{
  /* One and only one of qp and bitrate must be specified */
  if ((*qp == NO_VALUE && *bitrate == NO_VALUE)
      || (*qp != NO_VALUE && *bitrate != NO_VALUE))
  {
    fprintf (stderr, "One and only one of qp and bitrate must be specified\n");
    return false;
  }

  if (*qp != NO_VALUE)
  {
    const int max_qp = 51;
    const int min_qp = 0;
    if (*qp < min_qp || *qp > max_qp)
    {
      fprintf (stderr, "QP `%d' is out of range [%d, %d]\n",
               *qp, min_qp, max_qp);
      return false;
    }
  } else
  {
    *qp = TAA_H264_QP_NOT_SPECIFIED;
  }

  if (*bitrate != NO_VALUE)
  {
    if (*bitrate < 1)
    {
      fprintf (stderr, "BITRATE `%d' must be a positive value\n", *bitrate);
      return false;
    }
  }

  if (width < 16 || height < 16)
  {
    fprintf(stderr, "output resolution %d x %d is not supported\n", width, height);
    return false;
  }

  if (*input_width == 0)
    *input_width = width;

  if (*input_height == 0)
    *input_height = height;

  if (*input_width % 16)
  {
    fprintf (stderr, "input width must be a multiple of 16\n");
    return false;
  }

  if (*input_width < width || *input_height < height)
  {
    fprintf (stderr, "output resolution must be same or less as input\n");
    return false;
  }

#define AR_SHIFT 16
  if (((*input_width << AR_SHIFT) / *input_height) != ((width << AR_SHIFT) / height))
  {
    fprintf (stderr, "Resolution has different aspect ratio (%dx%d -> %dx%d)",
             *input_width, *input_height, width, height);
    return false;
  }

  return true;
}

#pragma warning (disable:869)

/* Callback function used by encoder to output encoded data.
 * Writes the NALU to file. */
static bool write_nalu (
  unsigned char * buf,
  size_t          size,
  unsigned char * total_buffer,
  size_t          total_size,
  unsigned int    lt_idx,
  unsigned int    frame_num,
  bool            last_nalu_in_frame,
  void *          context)
{
  context_t * c = (context_t *) context;
  FILE * outfile = c->nalout;

  int n = fwrite (buf, sizeof (unsigned char), size, outfile);
  if (size != n)
  {
    fprintf (stderr, "Warning: Error while writing NALU to file.");
  }
  c->framebytes += size;

  return false;
}
#pragma warning (default:869)


static void report_error (
  void *           context,
  taa_h264_error_t level,
  const char *     error_file,
  int              error_line,
  const char *     msg)
{
  FILE * errfile = ((context_t *) context)->errout;

  fprintf (errfile, "%s(%d), L%u: %s\n", error_file, error_line, level, msg);

  if (level > 1)
    exit (1);
}




int main(
  int      argc,
  char * * argv)
{
  int k;
  unsigned int ysize, csize;
  float psnr_y, psnr_u, psnr_v, mssim;
  float accsnr_y, accsnr_u, accsnr_v, accmssim;
  int framerate;
  clock_t start, finish;
  float duration;

  bool write_reconstructed = false;

  int trace;
  int qp;
  int bitrate;
  int intra_period;
  int gop_size;
  int t_levels;
  int max_ref_frames;
  int width;
  int height;
  int input_width;
  int input_height;
  int frames;
  unsigned coding_options;
  bool do_psnr;
  char * filename_in = NULL;
  char * filename_out = NULL;
  char * filename_rec = NULL;
  char * filename_log = NULL;

  FILE * infile = NULL;
  FILE * outfile = NULL;
  FILE * reconfile = NULL;
  FILE * tracefile = NULL;

  const char * progname = argv[0];
  printf("------start encoder------\n");
  //getchar();

  if (!parse_options (progname,
                      argc,
                      argv,
                      &do_psnr,
                      &width,
                      &height,
                      &input_width,
                      &input_height,
                      &framerate,
                      &bitrate,
                      &qp,
                      &intra_period,
                      &gop_size,
                      &t_levels,
                      &max_ref_frames,
                      &coding_options,
                      &trace,
                      &filename_log,
                      &filename_rec,
                      &filename_in,
                      &filename_out,
                      &frames))
  {
    return 1;
  }

  if (!validate_options (width, height, &input_width, &input_height, &bitrate, &qp))
  {
    return 1;
  }

  /* Open files */

  if (!(infile = fopen(filename_in, "rb")))
  {
    fprintf(stderr, "Could not open input file `%s'\n", filename_in);
    return 1;
  }
  if (!(outfile = fopen(filename_out, "wb")))
  {
    fprintf(stderr, "Could not open output file `%s'\n", filename_out);
    return 1;
  }

  if (filename_rec)
  {
    write_reconstructed = true;
    if (!(reconfile = fopen(filename_rec, "wb")))
    {
      fprintf(stderr, "Could not open recon file `%s'\n", filename_rec);
      return 1;
    }
  }

  if (trace != 0)
  {
    if (!filename_log)
      filename_log = DEFAULT_FILENAME_LOG;

    if (!(tracefile = fopen(filename_log, "w")))
    {
      fprintf(stderr, "Could not open log file `%s'\n", filename_log);
      return 1;
    }

    TAA_H264_INIT_TRACE (trace, tracefile);
  }

  taa_h264_enc_create_params cparams;
  taa_h264_enc_default_create_params (&cparams);
  cparams.width = width;
  cparams.height = height;
  cparams.num_ref_frames = max_ref_frames;
  cparams.temporal_levels = t_levels;
  cparams.callbacks.report_error = report_error;
  cparams.callbacks.output_nalu = write_nalu;
  context_t context = { .nalout = outfile, .errout = stderr, .framebytes = 0};
  cparams.callbacks.context = &context;
  taa_h264_enc_handle * encoder = taa_h264_enc_create (&cparams);
  if (!encoder)
  {
    fprintf(stderr, "Could not create encoder.\n");
    return 1;
  }

  ysize = input_width * input_height;
  csize = ysize / 4;

  /* Report statistics. First a header */
  fprintf(stdout, "---------------------------- h264 encoder -------------------------------------\n");
  fprintf(stdout, "Input yuv file          : %s\n", filename_in);
  fprintf(stdout, "Output bitstream file   : %s\n", filename_out);
  if (write_reconstructed)
    fprintf(stdout, "Output yuv file         : %s\n", filename_rec);
  if (trace != 0)
    fprintf(stdout, "Log file                : %s\n", filename_log);
  fprintf(stdout, "Frame width             : %d\n", width);
  fprintf(stdout, "Frame height            : %d\n", height);
  fprintf(stdout, "Frames to be encoded    : %d\n", frames);
  fprintf(stdout, "Quantizer start value   : %d\n", qp);
  fprintf(stdout, "-------------------------------------------------------------------------------\n");

  if (do_psnr)
  {
    accsnr_y = 0;
    accsnr_u = 0;
    accsnr_v = 0;
    accmssim = 0;
    fprintf(stdout, " Frame  Av.qp     SNRY      SNRU      SNRV       SSIM\n");
    fprintf(stdout, "-------------------------------------------------------------------------------\n");
  }

  int framesize = ysize + 2 * csize;
  unsigned char * framebuf = _mm_malloc(framesize * frames, 64);
  if (!framebuf)
  {
    fprintf (stderr, "Error allocating memory for all frames\n");
    return 1;
  }

  /* Read all frames into memory. Good for speed. */
  for (k = 0; k < frames; k++)
  {
    if (fread( framebuf + k * framesize, sizeof(unsigned char), framesize, infile) != framesize)
    {
      fprintf (stderr, "Error reading raw input from file\n");
      return 1;
    }
  }

  const int raw_mb_size = 16 * 16 + 8 * 8 * 2;
  uint8_t * outbuf = _mm_malloc ((MAX_NALU_SIZE + raw_mb_size) * sizeof (uint8_t), 64);
  if (!outbuf)
  {
    fprintf (stderr, "Error allocation memory for output NALU buffer\n");
    return 1;
  }

  /* Initialize input params with default values. Update what's necessary for
   * each frame. */
  taa_h264_enc_exec_params eparams;
  taa_h264_enc_default_exec_params (&eparams);
  eparams.raw_buffer = framebuf;
  eparams.input_width = input_width;
  eparams.input_height = input_height;
  eparams.intra_period = intra_period;
  eparams.gop_size = gop_size;
  eparams.qp = qp;
  eparams.bitrate = bitrate;
  eparams.framerate = framerate;
  eparams.max_nalu_size = MAX_NALU_SIZE;
  eparams.coding_options = coding_options;
  //eparams.coding_options = TAA_H264_HQ_DISABLE_EARLY_SKIP;
  eparams.output_buffer = outbuf;

  /* Config for MXP1700 compatibility */
  /* eparams.max_mbps = 35000; */
  /* eparams.max_static_mbps = 395500; */
  /* eparams.enable_frame_skip = true; */

  if (!do_psnr && !write_reconstructed)
  {
    /* Fast and minimal encoding loop. */
    start = clock();
    for (k = 0; k < frames; k++)
    {
      eparams.raw_buffer = framebuf + k * framesize;
      eparams.force_idr = (k == 0);
      taa_h264_enc_execute (encoder, &eparams);
    }
    finish = clock();
    duration = ((float) (finish - start)) / CLOCKS_PER_SEC;
  }
  else
  {
    /* General encoding loop. */
    start = clock();
    for (k = 0; k < frames; k++)
    {
      eparams.raw_buffer = framebuf + k * framesize;
      eparams.force_idr = (k == 0);
      int ret = taa_h264_enc_execute (encoder, &eparams);
      if (ret & TAA_H264_FRAME_SKIPPED)
      {
        fprintf(stdout, "%5d skipped\n", k);
        fflush (stdout);
        continue;
      }

      if (do_psnr)
      {
        /* HACK: This is to access the internals of the codec */
        encoder_t * enc_priv = (encoder_t *) encoder->priv;
        framebuf_t * recon = &enc_priv->frameinfo.recon;
        const int stride = recon->stride;
        const int cstride = recon->cstride;
        psnr_y = snr (&framebuf[0 + k * framesize],
                      recon->y, width, height,
                      stride);
        psnr_u = snr (&framebuf[ysize + k * framesize],
                      recon->u, width / 2, height / 2,
                      cstride);
        psnr_v = snr (&framebuf[ysize + csize + k * framesize],
                      recon->v, width / 2, height / 2,
                      cstride);
        mssim = ssim (&framebuf[0 + k*framesize], 
          recon->y, width, height, 
          stride);

        fprintf(stdout, "%5d%6d bytes     %10.5f%10.5f%10.5f%10.5f\n",
                k, context.framebytes, psnr_y, psnr_u, psnr_v, mssim);
        fflush (stdout);

        context.framebytes = 0;
        accsnr_y += psnr_y;
        accsnr_u += psnr_u;
        accsnr_v += psnr_v;
        accmssim += mssim;
      }

      if (write_reconstructed)
      {
        /* HACK: This is to access the internals of the codec and to reconstruct the frame. */
        encoder_t * enc_priv = (encoder_t *) encoder->priv;
        framebuf_t recon_cropped = enc_priv->frameinfo.recon;
        recon_cropped.cols = width;
        recon_cropped.ccols = width / 2;
        recon_cropped.rows = height;
        recon_cropped.crows = height / 2;
        write_frame (&recon_cropped, reconfile);
      }
    }
    finish = clock();
    duration = ((float) (finish - start)) / CLOCKS_PER_SEC;
  }

  if (do_psnr)
  {
    fprintf(stdout, "----------------------- Average data for all frames ---------------------------\n");
    fprintf(stdout, "PSNR Y          : %10.3f\n", accsnr_y / frames);
    fprintf(stdout, "PSNR U          : %10.3f\n", accsnr_u / frames);
    fprintf(stdout, "PSNR V          : %10.3f\n", accsnr_v / frames);
    fprintf(stdout, "SSIM            : %10.3f\n", accmssim / frames);
  }

  fprintf(stdout, "-------------------------------------------------------------------------------\n");

  long bytes = ftell(outfile);
  float kbits = (float ) bytes * 8 / 1000;

  /* NOTE: The format of this output has effect on the regression test
   * script. Make sure to update the regexp in the regtest if you change here.*/
  fprintf(stdout, "Time used %.3f sec\n", duration);
  if (duration > 0.01)
    fprintf(stdout, "Framerate %.3f fps\n", (frames * 1.0) / duration);
  else
    fprintf(stdout, "Framerate %.3f fps\n", (frames * 1.0) / 0.01);
  fprintf(stdout, "Frames encoded  :  %d\n", frames);
  fprintf(stdout, "File size       :  %ld   Bytes\n", bytes);
  fprintf(stdout, "Average Bitrate :  %5.2f kbit/s (@ %d fps)\n",
          (kbits * framerate) / frames, framerate);

  taa_h264_enc_destroy (encoder);
  _mm_free (framebuf);
  _mm_free (outbuf);

  if (infile) fclose(infile);
  if (outfile) fclose(outfile);
  if (reconfile) fclose(reconfile);
  if (tracefile) fclose(tracefile);

  return 0;
}
