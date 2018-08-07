#include "taah264enc.h"
#include "enc.h"
#include "snr.h"
#include "testsequences.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include <omp.h>

#define CODING_OPTIONS (0)

bool encode (
  FILE *   file,
  int      width,
  int      height,
  int      frames,
  int      fps,
  int      qp,
  double * psnr,
  double * bitrate);

static bool count_nalu_bytes (
  unsigned char * buf,
  size_t          size,
  unsigned char * total_buffer,
  size_t          total_size,
  unsigned int    lt_idx,
  unsigned int    frame_num,
  bool            last_nalu_in_frame,
  void *          context);

static void report_error (
  void *           context,
  taa_h264_error_t level,
  const char *     error_file,
  int              error_line,
  const char *     msg);

#define TRACE(...) printf (__VA_ARGS__)
//#define TRACE(...)

typedef struct
{
  FILE * errout;
  int total_bytes;
} context_t;


int main (int argc, char ** argv)
{
  const TestSequence * sequences;
  int sequence_table = 0;
  int num_seqs = 0;

  if (argc > 1) {
    sequence_table = atoi (argv[1]);
  }
  switch (sequence_table)
  {
  case 1:
    puts ("Using sequences: playtime");
    sequences = sequences_playtime;
    num_seqs = sizeof (sequences_playtime) / sizeof (TestSequence);
    break;
  case 0:
  default:
    puts ("Using sequences: benchmark v2");
    sequences = sequences_benchmark_v2;
    num_seqs = sizeof (sequences_benchmark_v2) / sizeof (TestSequence);
    break;
  }

  const int qp_base = 28;
  const int qp_step = 4;
  double bdr_values[num_seqs];

  for (int i = 0; i < num_seqs; i++) {
    const TestSequence * seq = &sequences[i];
    double psnr [4];
    double bitrate [4];

    #pragma omp parallel for schedule(dynamic, 1)
    for (int j = 0; j < 4; j++) {

      //int th_id = omp_get_thread_num();
      //int th_num = omp_get_num_threads ();
      //TRACE ("hello from th_id %d (th_num %d), i=%d, j=%d\n", th_id, th_num, i, j);

      FILE * file;
      if (!(file = fopen(seq->path, "rb"))) {
        fprintf(stderr, "Could not open input file `%s'\n", seq->path);
        assert (false);
      }

      int qp = qp_base + j * qp_step;

      bool ok = encode (file, seq->width, seq->height, seq->frames, seq->fps, qp,
                        &psnr[j], &bitrate[j]);
      if (!ok) {
        fprintf (stderr, "ERROR WHILE ENCODING!!!\n");
        assert (ok);
      }
      fclose (file);
    }
    //for (int k = 0; k < 4; k++)
    //  printf ("%.3f %.3f\n", psnr[k], bitrate[k]);

    bdr_values[i] = bdr (seq->psnr_ref, psnr, seq->bitrate_ref, bitrate);
    printf ("%-50s %.3f\n", seq->path, bdr_values[i]);
  }
  
  double sum = 0;
  for (int i = 0; i < num_seqs; i++)
    sum += bdr_values[i];
  double avg = sum / num_seqs;
  puts (  "=========================================================");
  printf ("%-50s %.3f\n", "TOTAL AVERAGE", avg);
  puts (  "=========================================================");

  return 0;
}


bool encode (
  FILE *   file,
  int      width,
  int      height,
  int      frames,
  int      fps,
  int      qp,
  double * psnr,
  double * bitrate)
{
  int ysize, csize;
  float accsnr_y = 0;
  static const int MAX_NALU_SIZE = 1450;

  taa_h264_enc_create_params cparams;
  taa_h264_enc_default_create_params (&cparams);
  cparams.width = width;
  cparams.height = height;
  cparams.callbacks.report_error = report_error;
  cparams.callbacks.output_nalu = count_nalu_bytes;
  context_t context = {.errout = stderr, .total_bytes = 0};
  cparams.callbacks.context = &context;
  taa_h264_enc_handle * encoder = taa_h264_enc_create (&cparams);

  ysize = width * height;
  csize = ysize / 4;

  int framesize = ysize + 2 * csize;
  unsigned char * framebuf = _mm_malloc(framesize, 16);
  if (!framebuf)
  {
    fprintf (stderr, "Error allocating memory for all frames\n");
    return false;
  }

  const int raw_mb_size = 16 * 16 + 8 * 8 * 2;
  uint8_t * outbuf = _mm_malloc ((MAX_NALU_SIZE + raw_mb_size) * sizeof (uint8_t), 64);
  if (!outbuf)
  {
    fprintf (stderr, "Error allocation memory for output NALU buffer\n");
    return false;
  }

  taa_h264_enc_exec_params eparams;
  taa_h264_enc_default_exec_params (&eparams);
  eparams.raw_buffer = framebuf;
  eparams.input_width = width;
  eparams.input_height = height;
  eparams.qp = qp;
  eparams.framerate = fps;
  eparams.max_nalu_size = MAX_NALU_SIZE;
  eparams.coding_options = CODING_OPTIONS;
  eparams.output_buffer = outbuf;

  for (int k = 0; k < frames; k++)
  {
    if (fread(framebuf, sizeof(unsigned char), framesize, file) != framesize)
    {
      fprintf (stderr, "Error reading raw input from file\n");
      return false;
    }

    eparams.raw_buffer = framebuf;
    eparams.force_idr = (k == 0);
    int ret = taa_h264_enc_execute (encoder, &eparams);
    if (ret & TAA_H264_FRAME_SKIPPED)
    {
      fprintf(stderr, "%5d skipped, should not happen with current config\n", k);
      continue;
    }

    /* HACK: This is to access the internals of the codec */
    encoder_t * enc_priv = (encoder_t *) encoder->priv;
    framebuf_t * recon = &enc_priv->frameinfo.recon;
    accsnr_y += snr (framebuf, recon->y, width, height, recon->stride);
  }

  float kbits = (float ) context.total_bytes * 8 / 1000;
  float kbps = (kbits * fps) / frames;

  taa_h264_enc_destroy (encoder);
  _mm_free (framebuf);
  _mm_free (outbuf);

  *psnr = accsnr_y / frames;
  *bitrate = kbps;

  return true;
}

#pragma warning (disable:869)

static bool count_nalu_bytes (
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
  c->total_bytes += size;

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
