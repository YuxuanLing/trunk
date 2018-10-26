#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <xmmintrin.h>
#include "com_compatibility.h"


#if defined(__INTEL_COMPILER)
typedef unsigned __int8  uint8_t;
#else
typedef unsigned char uint8_t;
#endif

static const int ker_size = 8;
static const float kernel = 0.01562459f;

static const float C1 = 6.5025f;
static const float C2 = 58.522495f;

static const int vert_step = 4;
static const int horis_step = 4;

// Calculates the ssim for a pixel using SIMD instructions.
//
// The code which do not use SIMD is at the bottom of this method for reference.
float 
ssim_pixel(const float *original, 
           const float *encoded, 
           const float *orig_sq, 
           const float *enc_sq,
           const float *orig_enc, 
           const int width, 
           const int height)
{
  __assume_aligned(original, 64);
  __assume_aligned(encoded, 64);
  __assume_aligned(orig_sq, 64);
  __assume_aligned(enc_sq, 64);
  __assume_aligned(orig_enc, 64);

  int row, col;
  float mu_x, mu_y, mu_xy, sigma_x, sigma_y, sigma_xy;
  __m128 mm_mu_x, mm_mu_y, mm_sig_x, mm_sig_y, mm_sig_xy;
  TAA_H264_ALIGN(64) float *conv;

  mm_mu_x = _mm_set_ps1(0.0f);
  mm_mu_y = _mm_set_ps1(0.0f);
  mm_sig_x = _mm_set_ps1(0.0f);
  mm_sig_y = _mm_set_ps1(0.0f);
  mm_sig_xy = _mm_set_ps1(0.0f);

  for(row = 0; row < ker_size; row++) {
    _mm_prefetch((const char*) (&original[row*width]), _MM_HINT_T0);
    for(col = 0; col < ker_size; col += 4) {
      mm_mu_x = _mm_add_ps(mm_mu_x, _mm_load_ps(&original[row*width + col]));
    }

    _mm_prefetch((const char*) (&encoded[row*width]), _MM_HINT_T0);
    for(col = 0; col < ker_size; col += 4) {
      mm_mu_y = _mm_add_ps(mm_mu_y, _mm_load_ps(&encoded[row*width + col]));
    }

    _mm_prefetch((const char*) (&orig_sq[row*width]), _MM_HINT_T0);
    for(col = 0; col < ker_size; col += 4) {
      mm_sig_x = _mm_add_ps(mm_sig_x, _mm_load_ps(&orig_sq[row*width + col]));
    }

    _mm_prefetch((const char*) (&enc_sq[row*width]), _MM_HINT_T0);
    for(col = 0; col < ker_size; col += 4) {
      mm_sig_y = _mm_add_ps(mm_sig_y, _mm_load_ps(&enc_sq[row*width + col]));
    }

    _mm_prefetch((const char*) (&orig_enc[row*width]), _MM_HINT_T0);
    for(col = 0; col < ker_size; col += 4) {
      mm_sig_xy = _mm_add_ps(mm_sig_xy, _mm_load_ps(&orig_enc[row*width + col]));
    }
  }

  conv = (float*) &mm_mu_x;
  mu_x = conv[0] + conv[1] + conv[2] + conv[3];
  conv = (float*) &mm_mu_y;
  mu_y = conv[0] + conv[1] + conv[2] + conv[3];

  conv = (float*) &mm_sig_x;
  sigma_x = conv[0] + conv[1] + conv[2] + conv[3];
  conv = (float*) &mm_sig_y;
  sigma_y = conv[0] + conv[1] + conv[2] + conv[3];
  conv = (float*) &mm_sig_xy;
  sigma_xy = conv[0] + conv[1] + conv[2] + conv[3];

  mu_xy = mu_x*mu_y;
  mu_x *= mu_x;
  mu_y *= mu_y;

  sigma_x -= mu_x;
  sigma_y -= mu_y;
  sigma_xy -= mu_xy;

  return ((2*mu_xy + C1)*(2*sigma_xy + C2))/((mu_x + mu_y + C1)*(sigma_x + sigma_y + C2));

  /**************************************************************************************************
  // This is the code which is done using SIMD instructions above. This is just left as a reference.
  for(row = 0; row < ker_size; row++) {
  for(col = 0; col < ker_size; col++) {
  mu_x += original[row*width + col];
  mu_y += encoded[row*width + col];
  sigma_x += orig_sq[row*width + col];
  sigma_y += enc_sq[row*width + col];
  sigma_xy += orig_enc[row*width + col];
  }
  }

  mu_xy = mu_x*mu_y;
  mu_x *= mu_x;
  mu_y *= mu_y;

  sigma_x -= mu_x;
  sigma_y -= mu_y;
  sigma_xy -= mu_xy;

  sigma_x = sqrtf(sigma_x);
  sigma_y = sqrtf(sigma_y);

  return ((2*mu_xy + C1)*(2*sigma_xy + C2))/((mu_x + mu_y + C1)*(sigma_x*sigma_x + sigma_y*sigma_y + C2));
  ********************************************************************************************************/
}

// Calculate the ssim for the whole frame. We actually only computes the ssim
// for every eight pixel, and take the mean value of theese. This might affect
// the precicion a bit, but the worst observed is in the fourth digit after the
// comma, and then just by 1. The speedup of computing only every eigth is also
// much greater, and we get a speedup of 2.5x.
float
ssim(const uint8_t *original, 
     const uint8_t *encoded, 
     const int width, 
     const int height, 
     const int stride)
{
  __assume_aligned(original, 64);
  __assume_aligned(encoded, 64);

  float mssim = 0.0;

  int row, col;
  float *orig;
  float *enc;
  float *orig_sq; 
  float *enc_sq;
  float *orig_enc;
  __m128 *mm_orig, *mm_enc, *mm_orig_sq, *mm_enc_sq, *mm_orig_enc;
  __m128 mm_ker, mm_tmp1;

  // Floating point arrays to be used in the calculation.
  orig = _mm_malloc(width*height*sizeof(float), 64);
  enc = _mm_malloc(width*height*sizeof(float), 64);
  orig_sq = _mm_malloc(width*height*sizeof(float), 64);
  enc_sq = _mm_malloc(width*height*sizeof(float), 64);
  orig_enc = _mm_malloc(width*height*sizeof(float), 64);

  // Convert to floating point numbers, and pre-square and multiply with kernel.
  for(row = 0; row < height; row++) {
    _mm_prefetch((const char*) (&original[row*width]), _MM_HINT_T0);
#pragma unroll(8)
#pragma vector aligned
    for(col = 0; col < width; col++) {
      orig[row*width + col] = (float) original[row*width + col];
    }

    _mm_prefetch((const char*) (&encoded[row*stride]), _MM_HINT_T0);
#pragma unroll(8)
#pragma vector aligned
    for(col = 0; col < width; col++) {
      enc[row*width + col] = (float) encoded[row*stride + col];
    }
  }

  mm_ker = _mm_set_ps1(kernel);

  // Pre-calculate the squared and original*encoded to just have to do this
  // once instead of for each pixel, save a lot of time. Also pre-multiplies
  // with the kernel.
  for(row = 0; row < height; row++) {
    _mm_prefetch((const char*) (&orig[row*width]), _MM_HINT_T0);
    for(col = 0; col < width; col += 4) {
      mm_orig = (__m128*) &orig[row*width + col];
      mm_orig_sq = (__m128*) &orig_sq[row*width + col];
      mm_orig_enc = (__m128*) &orig_enc[row*width + col];

      mm_tmp1 = _mm_mul_ps(*mm_orig, mm_ker);
      *mm_orig_sq = _mm_mul_ps(mm_tmp1, *mm_orig);
      *mm_orig = mm_tmp1;
      *mm_orig_enc = mm_tmp1;
    }

    _mm_prefetch((const char*) (&enc[row*width]), _MM_HINT_T0);
    for(col = 0; col < width; col += 4) {
      mm_enc = (__m128*) &enc[row*width + col];
      mm_enc_sq = (__m128*) &enc_sq[row*width + col];
      mm_orig_enc = (__m128*) &orig_enc[row*width + col];

      mm_tmp1 = _mm_mul_ps(*mm_enc, mm_ker);

      *mm_enc_sq = _mm_mul_ps(mm_tmp1, *mm_enc);
      *mm_orig_enc = _mm_mul_ps(*mm_orig_enc, *mm_enc);
      *mm_enc = mm_tmp1;
    }
  }

  // Calculate the ssim for each pixel. Change the second loop to increment
  // by 1 instead of 4 to change from SIMD to normal.
  for(row = 0; row < height - ker_size; row += vert_step) {
    for(col = 0; col < width - ker_size; col += horis_step) {
      mssim += ssim_pixel(&orig[row*width + col],
        &enc[row*width + col],
        &orig_sq[row*width + col],
        &enc_sq[row*width + col],
        &orig_enc[row*width + col],
        width, height);
    }
  }

  _mm_free(orig);
  _mm_free(enc);
  _mm_free(orig_sq);
  _mm_free(enc_sq);
  _mm_free(orig_enc);

  return vert_step*horis_step*mssim / ((width - ker_size)*(height - ker_size));
}
