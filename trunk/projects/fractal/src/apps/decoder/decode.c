#include "taah264dec.h"

#include "dec.h"
#include "annexb.h"


#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#include "getopt.h"                     /* included last since it redefines stuff */

#ifdef WIN32
# define CATCH_UNHANDLED_EXCEPTIONS
#endif

#define EXIT_ON_ERROR 0

typedef struct
{
  taa_h264_decoded_frame_t dp;
  bool has_new_picture;
  FILE * errout;
} context_t;

static FILE * outfile = NULL;

static char * DEFAULT_FILENAME_LOG = "decoder.log";

static void help (
  const char * progname)
{
  printf ("Usage: %s [options] INFILE OUTFILE\n"
          "  Options:\n"
          "    -h, --help              show this help message\n"
          "    -r FILE,\n"
          "    --reference FILE        calculate PSNR with FILE as reference\n"
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
          "                              - 32 DEBLOCK\n"
          "    -e,\n"
          "    --handle-exceptions     catch unhandled exceptions, Windows only\n"
          , progname);
}

static void usage(
  const char * progname)
{
  fprintf(stderr,
          "Summary: %s [-help] [options] INFILE OUTFILE\n",
          progname);
}


static void write_frame (
  const taa_h264_decoded_frame_t * output,
  FILE *                           f)
{
  int i;
  const uint8_t * y = output->y;
  const uint8_t * u = output->u;
  const uint8_t * v = output->v;
  const int stride  = output->stride;
  const int cstride = output->stride_chroma;

  int rows    = output->height;
  int cols    = output->width;
  int crows   = output->height / 2;
  int ccols   = output->width / 2;

  bool cropping = output->crop_left
                  || output->crop_right
                  || output->crop_top
                  || output->crop_bottom;

  if (cropping)
  {
    /* Typically for 1080p, but could also be for any other case if
     * the encoder signals cropping. */

    rows = output->height - output->crop_top - output->crop_bottom;
    cols = output->width - output->crop_left - output->crop_right;
    crows = rows / 2;
    ccols = cols / 2;

    y += stride * output->crop_top;
    u += cstride * output->crop_top / 2;
    v += cstride * output->crop_top / 2;

    y += output->crop_left;
    u += output->crop_left / 2;
    v += output->crop_left / 2;
  }

  for (i = 0; i < rows; i++)
    fwrite (&y[i * stride], sizeof(uint8_t), cols, f);

  for (i = 0; i < crows; i++)
    fwrite (&u[i * cstride], sizeof(uint8_t), ccols, f);

  for (i = 0; i < crows; i++)
    fwrite (&v[i * cstride], sizeof(uint8_t), ccols, f);
}


static double snr (
  const uint8_t * data1,
  const uint8_t * data2,
  int             width,
  int             height,
  int             stride1,
  int             stride2)
{
  double psnr;
  uint64_t sumsqr = 0;

  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      int ival = abs (data1[i * stride1 + j] - data2[i * stride2 + j]);
      sumsqr += (ival * ival);
    }
  }
  psnr = ((double) sumsqr) / (65025.0 * width * height);
  psnr = -10 * log10(psnr);

  return psnr;
}


static void calculate_psnr (
  taa_h264_decoded_frame_t * output,
  FILE *                     ref,
  double *                   psnr_y,
  double *                   psnr_u,
  double *                   psnr_v)
{
  const uint8_t * y = output->y;
  const uint8_t * u = output->u;
  const uint8_t * v = output->v;
  int rows    = output->height;
  int cols    = output->width;
  int crows   = output->height / 2;
  int ccols   = output->width / 2;
  int stride  = output->stride;
  int cstride = output->stride_chroma;

  int ysize = rows * cols;
  int csize = crows * ccols;

  uint8_t refbuf [ysize + 2 * csize];
  uint8_t * ref_y = refbuf;
  uint8_t * ref_u = ref_y + ysize;
  uint8_t * ref_v = ref_u + csize;

  /* Read in the original frame for snr calculation */
  if (fread(ref_y, sizeof(uint8_t), ysize, ref) != ysize)
  {
    fprintf (stderr, "Error reading Y from reference file");
    exit (1);
  }
  if (fread(ref_u, sizeof(uint8_t), csize, ref) != csize)
  {
    fprintf (stderr, "Error reading U from reference file");
    exit (1);
  }
  if (fread(ref_v, sizeof(uint8_t), csize, ref) != csize)
  {
    fprintf (stderr, "Error reading V from reference file");
    exit (1);
  }

  *psnr_y = snr (ref_y, y, cols, rows, cols, stride);
  *psnr_u = snr (ref_u, u, ccols, crows, ccols, cstride);
  *psnr_v = snr (ref_v, v, ccols, crows, ccols, cstride);
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


static bool parse_options (
  const char * progname,
  int          argc,
  char *       argv[],
  int *        trace,
  char * *     filename_log_ptr,
  char * *     filename_ref_ptr,
  char * *     filename_in_ptr,
  char * *     filename_out_ptr)
{
  char * shortopts = "hq:b:x:r:l:t:e";
  struct option longopts[] =
  {
    /* name,        has_arg,           flag, val */         /* longind */
    { "help",       no_argument,        0,    'h' },         /*       1 */
    { "trace",      required_argument,  0,    't' },         /*       2 */
    { "reference",  required_argument,  0,    'r' },         /*       3 */
    { "log",        required_argument,  0,    'l' },         /*       4 */
    { "handle-exceptions", no_argument, 0,    'e' },         /*       5 */
    /* end-of-list marker */
    { 0, 0, 0, 0 }
  };
  int opt;
  int longind = 0;

  bool logfile = false;
  bool reffile = false;

  *trace = 0;

  /* Parse optional arguments */
  while ((opt = getopt_long_only(argc, argv, shortopts, longopts, &longind)) != -1)
  {
    switch (opt)
    {
    case 'h':         /* -help */
      help(progname);
      return false;
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
    case 'r':         /* -reference=FILE */
    {
      *filename_ref_ptr = optarg;
      reffile = true;
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

  if (!reffile)
    *filename_ref_ptr = NULL;
  if (!logfile)
    *filename_log_ptr = NULL;

  /* Parse mandatory arguments */
  if (argc - optind != 2)
  {
    usage (progname);
    return false;
  }

  *filename_in_ptr = argv[optind++];
  *filename_out_ptr = argv[optind++];

  return true;
}

static void report_error_cb (
  void *           context,
  taa_h264_error_t level,
  const char *     error_file,
  int              error_line,
  const char *     msg)
{
  FILE * errfile = ((context_t *) context)->errout;
  fprintf (errfile, "%s(%d), L%u: %s\n", error_file, error_line, level, msg);

#if EXIT_ON_ERROR
  if (level > 1)
    exit (1);
#endif
}

static void report_frame_complete_cb (
  void *                           context,
  const taa_h264_decoded_frame_t * dp)
{
  /* We don't write the output here, since we don't want it to be a part of
   * the decoding time measurement. */
  static int counter = 0;
  //printf ("complete frame %04d\n", counter++);
  context_t * ctx = (context_t *) context;
  ctx->has_new_picture = true;
  ctx->dp = *dp;

  //write_frame (dp, outfile);
}

int main(
  int      argc,
  char * * argv)
{
  bool do_psnr = false;

  int trace;
  char * filename_in = NULL;
  char * filename_out = NULL;
  char * filename_ref = NULL;
  char * filename_log = NULL;

  FILE * infile = NULL;
//  FILE * outfile = NULL;
  FILE * reffile = NULL;
  FILE * tracefile = NULL;

  const char * progname = argv[0];

  if (!parse_options (progname,
                      argc,
                      argv,
                      &trace,
                      &filename_log,
                      &filename_ref,
                      &filename_in,
                      &filename_out))
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
    fprintf(stderr, "Could not open ouput file `%s'\n", filename_out);
    return 1;
  }

  if (filename_ref)
  {
    do_psnr = true;
    if (!(reffile = fopen(filename_ref, "rb")))
    {
      fprintf(stderr, "Could not open reference file `%s'\n", filename_ref);
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


  taa_h264_dec_handle * decoder;
  annexb_context ctx;
  annexb_init (&ctx, infile);

  context_t context = { .has_new_picture = false, .errout = stderr};
  taa_h264_dec_create_params cparams;

  // width, height, and num_ref_frames are set to zero so that
  // the dpb is initialized when the first SPS is activated.
  cparams.width  = 0; 
  cparams.height = 0; 
  cparams.num_ref_frames = 0; 
  cparams.callbacks.context = &context;
  cparams.callbacks.report_error = report_error_cb;
  cparams.callbacks.report_frame_complete = report_frame_complete_cb;
  cparams.callbacks.received_sei_user_data = NULL;
  cparams.callbacks.reference_ack = NULL;

  decoder = taa_h264_dec_create (&cparams);
  if (!decoder)
  {
    fprintf(stderr, "Could not create decoder.\n");
    return 1;
  }

  /* Report statistics. First a header */
  fprintf(stdout, "------------------------------- h264 decoder ---------------------------\n");
  fprintf(stdout, "Input h264 file         : %s\n", filename_in);
  fprintf(stdout, "Output yuv file         : %s\n", filename_out);
  if (do_psnr)
    fprintf(stdout, "Reference yuv file      : %s\n", filename_ref);
  if (trace != 0)
    fprintf(stdout, "Log file                : %s\n", filename_log);
  fprintf(stdout, "-----------------------------------------------------------------------\n");


  double psnr_y;
  double psnr_u;
  double psnr_v;
  double accsnr_y = 0;
  double accsnr_u = 0;
  double accsnr_v = 0;
  clock_t clock1;
  clock_t clock2;
  int this_time;
  int acc_time = 0;
  int frames = 0;

  if (do_psnr)
  {
    fprintf(stdout, " Frame     SNRY      SNRU      SNRV  Time(ms) \n");
    fprintf(stdout, "-----------------------------------------------------------------------\n");
  }

  while (extract_nalu (&ctx))
  {
    int happy;
    taa_h264_dec_exec_params eparams;
    eparams.nalu_buffer = ctx.nalu_start;
    eparams.nalu_size = ctx.nalu_size;

    clock1 = clock();
    /* TODO: Have a return value that indicate if an error occured, true/false */
    taa_h264_dec_execute (decoder, &eparams, &happy);
    clock2 = clock();

    this_time = (int)((clock2 - clock1) * 1000.0 / CLOCKS_PER_SEC);
    acc_time += this_time;

    if (context.has_new_picture)
    {
      write_frame (&context.dp, outfile);

      if (do_psnr)
      {
        calculate_psnr (&context.dp, reffile,
                        &psnr_y, &psnr_u, &psnr_v);

        fprintf(stdout, "%5d%12.5f%10.5f%10.5f%7d\n",
                frames, psnr_y, psnr_u, psnr_v, this_time);
        fflush(stdout);

        accsnr_y += psnr_y;
        accsnr_u += psnr_u;
        accsnr_v += psnr_v;
      }

      context.has_new_picture = false;
      frames++;
    }
  }

  taa_h264_dec_destroy (decoder);
  annexb_free (&ctx);

  fprintf(stdout, "------------------- Average data for all frames ----------------------\n");
  fprintf(stdout, "PSNR Y:         %10.3f\n", accsnr_y / frames);
  fprintf(stdout, "PSNR U:         %10.3f\n", accsnr_u / frames);
  fprintf(stdout, "PSNR V:         %10.3f\n", accsnr_v / frames);
  fprintf(stdout, "Frames decoded: %10d\n", frames);
  fprintf(stdout, "Total time:     %10.3f\n", acc_time/1000.0);
  fprintf(stdout, "Fps:            %10.3f\n", frames / (acc_time/1000.0));
  fprintf(stdout, "----------------------------------------------------------------------\n");

  if (infile) fclose(infile);
  if (outfile) fclose(outfile);
  if (reffile) fclose(reffile);
  if (tracefile) fclose(tracefile);

#if 0
  if (IsDebuggerPresent())
  {
    printf ("Press ENTER to continue.");
    getchar ();
  }
#endif

  return 0;
}
