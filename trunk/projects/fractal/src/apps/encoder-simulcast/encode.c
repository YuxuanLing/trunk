#include "taah264enc.h"

#include "enc.h"
#include "resize.h"

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
static const int MAX_NALU_SIZE = 1500;
static const int NUM_ENCODERS = 2;

typedef struct
{
  FILE * outfile;
  FILE * reconfile;
  FILE * tracefile;
  FILE * errfile;
} files_t;


static void help (
  const char * progname)
{
  printf ("Usage: %s [options] INFILE OUTFILE FRAMES\n"
          "  Options:\n"
          "    -h, --help              show this help message\n"
          "    -q QP,\n"
          "    --qp QP                 encode with quantiazation paramater QP\n"
          "    -b RATE,\n"
          "    --bitrate RATE          encode with bitrate RATE (kbit/s) (N/A)\n"
          "    -x RES,\n"
          "    --resolution RES        the resolution of the input file.\n"
          "                            RES is WIDTHxHEIGHT, e.g. 1280x720, or a special number:\n"
          "                              - 2  CIF\n"
          "                              - 3  720p\n"
          "    -f FPS\n"
          "    --framerate FPS         frame rate used for rate control/calculation\n"
          "    -i N\n"
          "    --intra-period N        force an intra frame every N frames\n"
          "                            N = 0 disables periodic intra frames\n"
          "    --gop-size SIZE         distiance between two frames of lowest temporal level\n"
          "    --t-levels N            number of temporal levels\n"
          "    --max-ref-frames N      number of allocated reference frames\n"
          "                            N = 0 tries to guess the number based on temporal levels\n"
          "    -r FILE,\n"
          "    --reconstruct FILE      write reconstruced frames to FILE\n"
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
  int *        framerate,
  int *        bitrate,
  int *        qp,
  int *        intra_period,
  int *        gop_size,
  int *        t_levels,
  int *        max_ref_frames,
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
    { "help",           no_argument,       0,    'h' },
    { "resolution",     required_argument, 0,    'x' },
    { "framerate",      required_argument, 0,    'f' },
    { "bitrate",        required_argument, 0,    'b' },
    { "qp",             required_argument, 0,    'q' },
    { "intra-period",   required_argument, 0,    'i' },
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
  *framerate = 30;
  *bitrate = NO_VALUE;
  *qp = NO_VALUE;
  *intra_period = 0;
  *gop_size = 1;
  *t_levels = 1;
  *max_ref_frames = 0;
  *trace = 0;

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

  if (width < 16 || width > 1920 || height < 16 || height > 1080)
  {
    fprintf(stderr, "resolution %d x %d is not supported\n", width, height);
    return false;
  }

  return true;
}

#pragma warning (disable:869)
/* Callback function used by encoder to ouput encoded data.
 * Writes the NALU to file. */
static bool write_nalu (
  unsigned char * buf,
  size_t          size,
  unsigned char * total_buf,
  size_t          total_size,
  unsigned int    lt_idx,
  unsigned int    frame_num,
  bool            last_nalu_in_frame,
  void *          context)
{
  FILE * outfile = ((files_t *) context)->outfile;

  int n = fwrite (buf, sizeof (unsigned char), size, outfile);
  if (size != n)
  {
    fprintf (stderr, "Warning: Error while writing NALU to file.");
  }

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
  FILE * errfile = ((files_t *) context)->errfile;

  fprintf (errfile, "%s(%d), L%u: %s\n", error_file, error_line, level, msg);

  if (level > 1)
    exit (1);
}

/* Splits the filename into a basename and extension. Warning, original filename is not preserved. */
static void filename_split (
  char *   filename,
  char * * base,
  char * * ext)
{
  int i;
  int len = strlen (filename);
  for (i = 0; i < len; i++)
  {
    if (filename[i] == '.')
      break;
  }

  *base = filename;
  if (i < len)
  {
    (*base)[i] = '\0';
    *ext = &(*base)[i+1];
  }
  else
  {
    *ext = '\0';
  }
}


int main(
  int      argc,
  char * * argv)
{
  int k;
  unsigned int ysize, csize;
  float psnr_y, psnr_u, psnr_v;
  float accsnr_y, accsnr_u, accsnr_v;
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
  int frames;
  bool do_psnr;
  char * filename_in = NULL;
  char * filename_out = NULL;
  char * filename_rec = NULL;
  char * filename_log = NULL;

  files_t files[NUM_ENCODERS];

  FILE * infile = NULL;
  FILE * tracefile = NULL;

  const char * progname = argv[0];

  if (!parse_options (progname,
                      argc,
                      argv,
                      &do_psnr,
                      &width,
                      &height,
                      &framerate,
                      &bitrate,
                      &qp,
                      &intra_period,
                      &gop_size,
                      &t_levels,
                      &max_ref_frames,
                      &trace,
                      &filename_log,
                      &filename_rec,
                      &filename_in,
                      &filename_out,
                      &frames))
  {
    return 1;
  }

  if (!validate_options (width, height, &bitrate, &qp))
  {
    return 1;
  }

  /* Open files */

  if (!(infile = fopen(filename_in, "rb")))
  {
    fprintf(stderr, "Could not open input file `%s'\n", filename_in);
    return 1;
  }

  char * base_out;
  char * ext_out;
  char * base_rec;
  char * ext_rec;
  filename_split (filename_out, &base_out, &ext_out);
  if (filename_rec)
  {
    filename_split (filename_rec, &base_rec, &ext_rec);
    write_reconstructed = true;
  }

  for (int i = 0; i < NUM_ENCODERS; i++)
  {
    char fn[64];
    snprintf (fn, sizeof (fn), "%s-%d.%s", base_out, i, ext_out);
    printf ("FILENAME %s\n", fn);
    if (!(files[i].outfile = fopen(fn, "wb")))
    {
      fprintf(stderr, "Could not open ouput file `%s'\n", fn);
      return 1;
    }

    if (write_reconstructed)
    {
      snprintf (fn, sizeof (fn), "%s-%d.%s", base_rec, i, ext_rec);
      printf ("FILENAME %s\n", fn);
      if (!(files[i].reconfile = fopen(fn, "wb")))
      {
        fprintf(stderr, "Could not open recon file `%s'\n", fn);
        return 1;
      }
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

  taa_h264_enc_create_params cparams[NUM_ENCODERS];
  taa_h264_enc_handle * enc_handles[NUM_ENCODERS];

  for (int i = 0; i < NUM_ENCODERS; i++)
  {
    cparams[i].width = width >> i;
    cparams[i].height = height >> i;
    cparams[i].num_ref_frames = max_ref_frames;
    cparams[i].temporal_levels = t_levels;
    cparams[i].callbacks.report_error = report_error;
    cparams[i].callbacks.output_nalu = write_nalu;
    cparams[i].callbacks.context = &files[i];
    enc_handles[i] = taa_h264_enc_create (&cparams[i]);
  }

  ysize = width * height;
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
  fprintf(stdout, "------------------------------------------------------------------------------\n");

  if (do_psnr)
  {
    accsnr_y = 0;
    accsnr_u = 0;
    accsnr_v = 0;
    fprintf(stdout, " Frame  Av.qp  SNRY      SNRU      SNRV  \n");
    fprintf(stdout, "------------------------------------------------------------------------------\n");
  }

  int framesize = ysize + 2 * csize;
  unsigned char * framebuf = _mm_malloc(framesize * frames, 16);
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

  /* Initilize input params with default values. Update what's necessary for
   * each frame. */
  taa_h264_enc_exec_params eparams[NUM_ENCODERS];
  for (int i = 0; i < NUM_ENCODERS; i++)
  {
    taa_h264_enc_default_exec_params (&eparams[i]);
    eparams[i].intra_period = intra_period;
    eparams[i].gop_size = gop_size;
    eparams[i].qp = qp;
    eparams[i].bitrate = bitrate;
    eparams[i].framerate = framerate;
    /* Same outbuf for all handlers since we don't have
     * parallelization */
    eparams[i].output_buffer = outbuf;
    eparams[i].max_nalu_size = MAX_NALU_SIZE;
    if (i > 0)
    {
      int scaled_framesize = cparams[i].width * cparams[i].height * 3 / 2;
      eparams[i].raw_buffer = _mm_malloc (scaled_framesize, 16);
      if (eparams[i].raw_buffer == NULL)
      {
        fprintf (stderr, "Failed allocating buffers for scaled input\n");
        exit (1);
      }
    }
    else
    {
      /* The unscaled camera input will be used. We don't need to
       * allocate memory for this one. */
      eparams[i].raw_buffer = NULL;
    }
  }

  if (!do_psnr && !write_reconstructed)
  {
    /* Fast and minimal encoding loop. */
    start = clock();
    for (k = 0; k < frames; k++)
    {
      for (int i = 0; i < NUM_ENCODERS; i++)
      {
        eparams[i].force_idr = (k == 0);
        if (i == 0)
          eparams[i].raw_buffer = framebuf + k * framesize;
        else
          resize (framebuf + k * framesize, eparams[i].raw_buffer, width, height, i + 1);

        taa_h264_enc_execute (enc_handles[i], &eparams[i]);
      }
    }
    finish = clock();
    duration = (float) (finish - start) / CLOCKS_PER_SEC;
  }
  else
  {
    /* General encoding loop. */
    start = clock();
    for (k = 0; k < frames; k++)
    {
      for (int i = 0; i < NUM_ENCODERS; i++)
      {
        eparams[i].force_idr = (k == 0);
        if (i == 0)
          eparams[i].raw_buffer = framebuf + k * framesize;
        else
          resize (framebuf + k * framesize, eparams[i].raw_buffer, width, height, i + 1);

        taa_h264_enc_execute (enc_handles[i], &eparams[i]);
      }

      if (do_psnr)
      {
        /* HACK: This is to access the internals of the codec */
        /* FIXME: Only does psnr for the first encoder/layer. */
        encoder_t * enc_priv = (encoder_t *) enc_handles[0]->priv;
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

        fprintf(stdout, "%5d%6d%10.5f%10.5f%10.5f\n",
                k, qp, psnr_y, psnr_u, psnr_v);
        fflush (stdout);

        accsnr_y += psnr_y;
        accsnr_u += psnr_u;
        accsnr_v += psnr_v;
      }

      if (write_reconstructed)
      {
        for (int i = 0; i < NUM_ENCODERS; i++)
        {
          /* HACK: This is to access the internals of the codec and to reconstruct the frame. */
          encoder_t * enc_priv = (encoder_t *) enc_handles[i]->priv;
          framebuf_t recon_cropped = enc_priv->frameinfo.recon;
          recon_cropped.cols = enc_priv->configured_width;
          recon_cropped.ccols = recon_cropped.cols / 2;
          recon_cropped.rows = enc_priv->configured_height;
          recon_cropped.crows = recon_cropped.rows / 2;
          write_frame (&recon_cropped, files[i].reconfile);
        }
      }
    }
    finish = clock();
    duration = (float) (finish - start) / CLOCKS_PER_SEC;
  }

  if (do_psnr)
  {
    fprintf(stdout, "------------------- Average data for all frames ------------------------------\n");
    fprintf(stdout, "PSNR Y          : %10.3f\n", accsnr_y / frames);
    fprintf(stdout, "PSNR U          : %10.3f\n", accsnr_u / frames);
    fprintf(stdout, "PSNR V          : %10.3f\n", accsnr_v / frames);
  }

  fprintf(stdout, "------------------------------------------------------------------------------\n");

  long bytes = ftell(files[0].outfile);
  float kbits = (float ) bytes * 8 / 1000;

  /* NOTE: The format of this output (even the number of whitespaces)
   * has effect on the regression test script. Should be fixed with a
   * more general regexp. */
  fprintf(stdout, "Time used %.3f sec\n", duration);
  fprintf(stdout, "Framerate %.3f fps\n", (frames * 1.0) / duration);
  fprintf(stdout, "Frames encoded  :  %d\n", frames);
  fprintf(stdout, "File size       :  %ld   Bytes\n", bytes);
  fprintf(stdout, "Average Bitrate :  %5.2f kbit/s (@ %d fps)\n",
          (kbits * framerate) / frames, framerate);


  _mm_free (framebuf);
  _mm_free (outbuf);

  if (infile) fclose(infile);
  if (tracefile) fclose(tracefile);

  for (int i = 0; i < NUM_ENCODERS; i++)
  {
    taa_h264_enc_destroy (enc_handles[i]);
    enc_handles[i] = NULL;

    if (i > 0) _mm_free (eparams[i].raw_buffer);
    if (files[i].outfile) fclose(files[i].outfile);
    if (files[i].reconfile) fclose(files[i].reconfile);
  }

  return 0;
}
