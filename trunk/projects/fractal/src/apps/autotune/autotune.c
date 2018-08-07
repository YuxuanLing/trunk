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
#define DEBUG(...) printf (__VA_ARGS__)

typedef struct
{
  FILE * errout;
  int total_bytes;
} context_t;

#define num_tuning_values 4
static int curr_state [num_tuning_values] = {0,};
static int best_state [num_tuning_values] = {0,};
static int base_state [num_tuning_values] = {0,};

// In order to make the program compile when tuning multipliers are removed from
// the core parts of the program is disabled
// #define AUTOTUNE_ENABLED

#ifdef AUTOTUNE_ENABLED
extern float tunefactor_lambda;
extern float tunefactor_lambda_intra;
extern int tunefactor_middleskip;
extern int tunefactor_i4window;
#else
static float tunefactor_lambda;
static float tunefactor_lambda_intra;
static int tunefactor_middleskip;
static int tunefactor_i4window;
#endif

static float one_iteration (const TestSequence * seqs, int num_seqs)
{
  const int qp_base = 28;
  const int qp_step = 4;
  double bdr_values[num_seqs];

  //update_variables_from_state ();
//  iterations_since_best++;

  //#pragma omp parallel for schedule(dynamic, 1)
  for (int i = 0; i < num_seqs; i++) {
    const TestSequence * seq = &seqs[i];
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
        exit (1);
      }

      int qp = qp_base + j * qp_step;
      bool ok = encode (file, seq->width, seq->height, seq->frames, seq->fps, qp,
                        &psnr[j], &bitrate[j]);
      if (!ok)
        fprintf (stderr, "ERROR WHILE ENCODING!!!\n");

      fclose (file);
    }
    bdr_values[i] = bdr (seq->psnr_ref, psnr, seq->bitrate_ref, bitrate);
    //printf ("bdr %.3f\n", bdr_values[i]);
  }
    
  double sum = 0;
  for (int i = 0; i < num_seqs; i++)
    sum += bdr_values[i];

  return  sum / num_seqs;
}


static void random_state (int * base, int * curr, int n)
{
  // TODO: Decrease max_change with decreased temperature?
  const int max_change = 5;
  for (int i = 0; i < n; i++)
    curr[i] = base[i] + (rand () % (2*max_change+1)) - max_change;
}

static void update_extern_variables (int * curr, int n)
{
  assert (n == num_tuning_values);

  // 0.849, 1.061
  // When tuning a variable for the first time the resolution should be larger than 1/1000
  tunefactor_lambda       = 0.95 + curr[0] / 100.0f;
  tunefactor_lambda_intra = 0.95 + curr[1] / 100.0f;
  tunefactor_middleskip   = curr[2];
  tunefactor_i4window     = curr[3];

  TRACE ("tuning lambda       with multiplier %.5f\n", tunefactor_lambda);
  TRACE ("tuning lambda intra with multiplier %.5f\n", tunefactor_lambda_intra);
  TRACE ("tuning middleskip   with offset     %d\n",   tunefactor_middleskip);
  TRACE ("tuning i4window     with offset     %d\n",   tunefactor_i4window);
}


static void simulated_annealing (float temperature, float temperature_end, float cooling,
                                 int max_iterations, const TestSequence * seqs, int num_seqs)
{
  // Intital state is set by the global values at start up
  double best_bdr = 999999.9;
  double prev_bdr = best_bdr;
  int iterations = 0;
  int iterations_since_best = 0;
  
  while (temperature > temperature_end) {
    iterations++;
    iterations_since_best++;    
    DEBUG ("D: [%d] temp=%.5f\n", iterations, temperature);
    
    random_state (base_state, curr_state, num_tuning_values);
    update_extern_variables (curr_state, num_tuning_values);
    
    double new_bdr = one_iteration (seqs, num_seqs);
    double delta = new_bdr - prev_bdr;

    TRACE ("[%d] new avg bdr %.3f, best bdr %.3f, since last best %d \n",
           iterations, new_bdr, best_bdr, iterations_since_best);

    if (new_bdr < best_bdr) {
      // We have a new best result, store it
      best_bdr = new_bdr;
      iterations_since_best = 0;
      TRACE ("**********************************************\n");
      TRACE ("new best state (new best bdr %.3f): ", best_bdr);
      for (int i = 0; i < num_tuning_values; i++) {
        best_state[i] = curr_state[i];
        TRACE ("%d ", best_state[i]);
      }
      TRACE ("\n");
      TRACE ("**********************************************\n");      
    }

    if (delta < 0) {
      // Always move to better state than the previous one
      TRACE ("Moving to better state (new_bdr=%.3f, prev_bdr=%.3f)\n", new_bdr, prev_bdr);
      prev_bdr = new_bdr;
      for (int i = 0; i < num_tuning_values; i++)
        base_state[i] = curr_state[i];
    } else {
      // Although the current state is worse than the previous, we
      // still might choose it as the base state based on a probablity
      // that decreases with time
      float rnd = rand() / (float) RAND_MAX;
      float prob = exp (-delta / temperature);
      DEBUG ("D: Worse state, check for moving prob=%.3f, delta=%.3f, temp=%.3f\n", prob, delta, temperature);
      if (rnd < prob) {
        TRACE ("Worse result but updating base state anyway (prob=%.3f, delta=%.3f, temp=%.3f)\n",
               prob, delta, temperature);
        prev_bdr = new_bdr;
        for (int i = 0; i < num_tuning_values; i++)
          base_state[i] = curr_state[i];
      }
    }
    TRACE ("\n");
    
    temperature *= cooling;

    if (iterations_since_best > 50) {
      TRACE ("\n\n%d iterations since last best, reset \n\n\n", iterations_since_best);
      // Seems like we've reached a dead end, reset to the best state
      prev_bdr = best_bdr;
      for (int i = 0; i < num_tuning_values; i++)
        base_state[i] = best_state[i];
    }

    if (iterations % 16 == 0) {
      printf ("Best state vector so far (%.3f): [", best_bdr);
      for (int i = 0; i < num_tuning_values; i++) {
        printf ("%d, ", best_state [i]);
      }
      printf ("]\n\n");
    }

    if (iterations >= max_iterations)
      break;
  }
}


int main (int argc, char ** argv)
{
#ifndef AUTOTUNE_ENABLED
  fprintf (stderr, "Autotune is disabled. To enable define the macro "
    "AUTOTUNE_ENABLED and add tuning multiplier in the core codec\n");
  exit (-1);
#endif

#if 1
  const TestSequence * sequences = sequences_benchmark_v2;
  const int num_seqs = sizeof (sequences_benchmark_v2) / sizeof (TestSequence);
#else
  const TestSequence sequences [] = {
    { .path = "BQSquare_416x240_60.yuv",
      .width=416, .height=240, .fps=60, .frames=600,
      .psnr_ref={34.393, 31.285, 28.352, 25.767},
      .bitrate_ref={2456.631, 1163.662, 450.088, 195.990}},

    { .path = "BlowingBubbles_416x240_50.yuv",
      .width=416, .height=240, .fps=50, .frames=500,
      .psnr_ref={34.118, 31.060, 28.361, 25.992},
      .bitrate_ref={1612.095, 787.644, 378.472, 183.525}},

    { .path = "vidyo3_1280x720_60.yuv",
      .width=1280, .height=720, .fps=60, .frames=600,
      .psnr_ref={40.061, 37.578, 35.003, 32.530},
      .bitrate_ref={1745.342, 878.304, 493.830, 291.434}},
  };
  const int num_seqs = sizeof (sequences) / sizeof (TestSequence);
#endif

  srand ((unsigned int)(time (NULL)));

  float temperature = 0.5; // high (relative to expected delta) temeprature accepts more worse results as base
  float temperature_end = 0.0001;
  float cooling = 0.98; // low value means faster decrease in temperature
  int max_iterations = 10000;
  simulated_annealing (temperature, temperature_end, cooling, max_iterations, sequences, num_seqs);

  TRACE ("**********************************************\n");
  printf ("Best state vector: [");
  for (int i = 0; i < num_tuning_values; i++) {
    printf ("%d, ", best_state [i]);
  }
  printf ("]\n");
  TRACE ("**********************************************\n");  
  

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
  double accsnr_y = 0;
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
  eparams.coding_options = 0;
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
