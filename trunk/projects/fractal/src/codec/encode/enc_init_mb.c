#include "enc_config.h"
#include "encoder.h"
#include "enc_init_mb.h"
#include "enc_motionsearch.h"
#include "com_mbutils.h"
#include "com_intmath.h"
#include "com_prediction.h"
#include "com_interpolation.h"
#include "enc_config.h"
#include "enc_mb.h"


// borrowed from widget, roughly 2*1.12^qp
static const int thr_table[52] = 
{
  /* 0*/    2,    2,    3,    3,    3,    4,    4,    4,
  /* 8*/    5,    6,    6,    7,    8,    9,   10,   11,
  /*16*/   12,   14,   15,   17,   19,   22,   24,   27,
  /*24*/   30,   34,   38,   43,   48,   53,   60,   67,
  /*32*/   75,   84,   94,  106,  118,  132,  148,  166,
  /*40*/  186,  208,  233,  261,  293,  328,  367,  411,
  /*48*/  461,  516,  578,  647
};

#include <stdlib.h>
#include <emmintrin.h>
#include <limits.h>
#define NUM_MVS_MB 16

/* QP2QUANT is calculated with the formula:
 *
 * for qp in range(52):
 *   print round (64 * (2 ** ((qp-12) / 6.0)) * C)
 *
 * with C=1 at the moment. Coding efficiency can be tweaked further
 * by finding the best C.
 *
 * TODO: Improve compression by adjusting the scaling factor C. */

#pragma warning (disable:981)

static const int taa_h264_qp2quant[52] = {
  16, 18, 20, 23, 25, 29, 32, 36,
  40, 45, 51, 57, 64, 72, 81, 91,
  102, 114, 128, 144, 161, 181, 203, 228,
  256, 287, 323, 362, 406, 456, 512, 575,
  645, 724, 813, 912, 1024, 1149, 1290, 1448,
  1625, 1825, 2048, 2299, 2580, 2896, 3251, 3649,
  4096, 4598, 5161, 5793,
};

#include "fast_object_motion.h"

/* Threshold for accepting early skip */
static const int threshold_tab[6] = { 10, 11, 13, 14, 16, 18};

static const int qp_scale_chroma[52] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
  12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
  28, 29, 29, 30, 31, 32, 32, 33, 34, 34, 35, 35, 36, 36, 37, 37,
  37, 38, 38, 38, 39, 39, 39, 39
};


#define EARLY_SKIP_FLAG            (1 << 1)
#define EARLY_SKIP_INTRA_FLAG      (1 << 2)
#define EARLY_SKIP_CBP_LUMA_FLAG   (1 << 3)
#define EARLY_SKIP_CBP_CHROMA_FLAG (1 << 4)
#define EARLY_SKIP_CBP_MASK        (EARLY_SKIP_CBP_LUMA_FLAG | EARLY_SKIP_CBP_CHROMA_FLAG)


#ifdef HAVE_SSSE3
//==================================================================================
//                             SSSE3 version of earlyskip
//==================================================================================
# include <tmmintrin.h>
# define taa_h264_check_early_skip taa_h264_check_early_skip_ssse3
# include "enc_earlyskip_template.h"
# undef taa_h264_check_early_skip
#else
//==================================================================================
//                             SSE2 version of earlyskip
//==================================================================================
# define _mm_abs_epi16(a)    (_mm_max_epi16((a), _mm_sub_epi16(_mm_setzero_si128(), (a))))
# define _mm_hadd_epi16(a, b) (_mm_add_epi16(_mm_packs_epi32(_mm_srai_epi32((a), 16), _mm_srai_epi32((b), 16)), _mm_packs_epi32(_mm_srai_epi32(_mm_slli_si128((a), 2), 16), _mm_srai_epi32(_mm_slli_si128((b), 2), 16))))
# define _mm_hsub_epi16(a, b) (_mm_sub_epi16(_mm_packs_epi32(_mm_srai_epi32((a), 16), _mm_srai_epi32((b), 16)), _mm_packs_epi32(_mm_srai_epi32(_mm_slli_si128((a), 2), 16), _mm_srai_epi32(_mm_slli_si128((b), 2), 16))))
# define taa_h264_check_early_skip taa_h264_check_early_skip_sse2
# include "enc_earlyskip_template.h"
# undef taa_h264_check_early_skip
# undef _mm_hsub_epi16
# undef _mm_hadd_epi16
# undef _mm_abs_epi16
#endif

static void taa_h264_candidate_motion_vectors (
  const int       avail_flags,
  const int       mvs_stride,
  const int       ref_num,
  const mv_t      motion_vectors_curr[],
  mv_t *  pred,
  mv_t *  skip
  )
{
  const mv_t mv_null = { 0, 0, .ref_num = -1};

  bool a_avail = MB_LEFT     (avail_flags);
  bool b_avail = MB_UP       (avail_flags);
  bool c_avail = MB_UP_RIGHT (avail_flags);
  bool d_avail = MB_UP_LEFT  (avail_flags);

  /*
   -------------------------------------
     5 |   0  1  4  5 |
     7 |   2  3  6  7 |
    13 |   8  9 12 13 |
    15 |  10 11 14 15 |  10 11 14 15 |
   -------------------------------------
     5 |   0  1  4  5 |
     7 |   2  3  6  7 |
    13 |   8  9 12 13 |
    15 |  10 11 14 15 |
   -------------------------------------

       D  |  B  |  C  |
   -------------------------------------
       A  | cur |

   */

  int offset_a = -13;   // reach to 5
  int offset_b = -mvs_stride + 12;
  int offset_c = -mvs_stride + 16 + 12;

  if (c_avail == false)
  {
    c_avail = d_avail;
    offset_c = -mvs_stride - 1;
  }

  const mv_t * mv_a = a_avail ? &motion_vectors_curr [offset_a] : &mv_null;
  const mv_t * mv_b = b_avail ? &motion_vectors_curr [offset_b] : &mv_null;
  const mv_t * mv_c = c_avail ? &motion_vectors_curr [offset_c] : &mv_null;

  /* This guess for predicted MV is similar to the MV prediction for P16x16 */
  bool left    = a_avail && (mv_a->ref_num == ref_num);
  bool up      = b_avail && (mv_b->ref_num == ref_num);
  bool upright = c_avail && (mv_c->ref_num == ref_num);

  int num_equal_refs = (left == true) + (up == true) + (upright == true);
  if (num_equal_refs == 1)
  {
    /* Only one block has same ref, return that MV */
    if (left)
      *pred = *mv_a;
    else if (up)
      *pred = *mv_b;
    else
      *pred = *mv_c;
  }
  else if (!b_avail && !c_avail
           && a_avail && mv_a->ref_num >= 0)
  {
    /* If only block_a_available, mv_a is set as predictor no matter the reference. */
    *pred = *mv_a;
  }
  else
  {
    pred->y = (int16_t) taa_h264_median (mv_a->y, mv_b->y, mv_c->y);
    pred->x = (int16_t) taa_h264_median (mv_a->x, mv_b->x, mv_c->x);
    pred->ref_num = (int8_t) ref_num;
  }

  taa_h264_compute_skip_motion_vector (
    pred, mv_a, mv_b, a_avail, b_avail, skip);

}

static
uint32_t taa_h264_intra_16x16_sad (
  mbinfo_t * mb)
{
  bool const left_avail = MB_LEFT (mb->avail_flags);
  bool const up_avail = MB_UP (mb->avail_flags);
  bool const leftup_avail = MB_UP_LEFT (mb->avail_flags);

  uint8_t * ipred        = mb->pred_i16x16_y;
  uint8_t const * curr   = mb->luma.input;
  uint8_t const * left   = mb->recon_left_y;
  uint8_t const * up     = mb->recon_top_y;
  uint8_t const upleft   = mb->recon_topleft_y;
  imode16_t * mode_out   = &mb->best_i16x16_mode;

  imode16_t best_mode    = DC_PRED_16;
  uint32_t min_sad   = UINT_MAX;
  uint32_t plane_sad = UINT_MAX;
  uint32_t hor_sad   = UINT_MAX;
  uint32_t vert_sad  = UINT_MAX;
  uint32_t dc_sad    = UINT_MAX;

  //======================================================================
  //                              DC prediction
  //======================================================================
  {
    TAA_H264_ALIGN(64) uint8_t zero[16] = {0};
    uint8_t *  pred = &ipred[DC_PRED_16 * MB_SIZE_Y];
    int shift = 3;
    uint32_t sum = 0;
    if (left_avail)
    {
      int32_t sad = 0;
#pragma ivdep
#pragma vector always
#pragma vector aligned
      for (int i = 0; i < MB_HEIGHT_Y; i++)
        sad += abs(left[i]-zero[i]);
      sum += sad + 8;
      shift++;
    }
    if (up_avail)
    {
      int32_t sad = 0;
#pragma ivdep
#pragma vector always
#pragma vector aligned
      for (int i = 0; i < MB_WIDTH_Y; i++)
        sad += abs(up[i]-zero[i]);
      sum += sad + 8;
      shift++;
    }
    if (shift == 3)
    {
      sum = 128;
      shift = 0;
    }
    uint8_t dc = (uint8_t)(sum >> shift);
#pragma ivdep
#pragma vector always
#pragma vector aligned
    for (int i = 0; i < MB_SIZE_Y; i++)
      pred[i] = dc;


    dc_sad = 0;
    for (int i = 0; i < 16; i++)
    {
      uint32_t sad = 0;
#pragma ivdep
#pragma vector always
#pragma vector aligned
      for (int j = 0; j < 16; j++)
      {
        sad += abs (pred[i * MB_WIDTH_Y + j] - curr[i * MB_WIDTH_Y + j]);
      }
      dc_sad += sad;
    }
  }
  //======================================================================
  //                        Vertical prediction
  //======================================================================
  if (up_avail)
  {
    uint8_t *  pred = &ipred[VERT_PRED_16 * MB_SIZE_Y];
    vert_sad = 0;
    for (int i = 0; i < 16; i++)
    {
      uint32_t sad = 0;
#pragma ivdep
#pragma vector always
#pragma vector aligned
      for (int j = 0; j < 16; j++)
      {
        pred[i * MB_WIDTH_Y + j] = up[j];
        sad += abs (pred[i * MB_WIDTH_Y + j] - curr[i * MB_WIDTH_Y + j]);
      }
      vert_sad += sad;
    }
  }
  //======================================================================
  //                           Left prediction
  //======================================================================
  if (left_avail)
  {
    uint8_t *  pred = &ipred[HOR_PRED_16 * MB_SIZE_Y];

    hor_sad = 0;
    for (int i = 0; i < 16; i++)
    {
      uint32_t sad = 0;
#pragma ivdep
#pragma vector always
#pragma vector aligned
      for (int j = 0; j < 16; j++)
      {
        pred [i * MB_WIDTH_Y + j] = left[i];
        sad += abs (pred[i * MB_WIDTH_Y + j] - curr[i * MB_WIDTH_Y + j]);
      }
      hor_sad += sad;
    }
  }
  //======================================================================
  //                        Plane prediction
  //======================================================================
  if (left_avail && up_avail && leftup_avail)
  {
    uint8_t *  pred = &ipred[PLANE_PRED_16 * MB_SIZE_Y];
    int h = 0;
    int v = 0;

    for (int i = 1; i <= 7; i++)
    {
      h += i * (up[7 + i] - up[7 - i]);
      v += i * (left[7 + i] - left[7 - i]);
    }

    h += 8 * (up[15] - upleft);
    v += 8 * (left[15] - upleft);

    int16_t const a = (int16_t) (16 * (up[15] + left[15]));
    int16_t const b = (int16_t) ((5 * h + 32) >> 6);
    int16_t const c = (int16_t) ((5 * v + 32) >> 6);
#if defined(_M_X64) || defined (__x86_64__)
    plane_sad = 0;
    for (int i = 0; i < 16; i++)
    {
#pragma ivdep
#pragma vector always
#pragma vector aligned
      for (int j = 0; j < 16; j++)
      {
        int16_t temp = (int16_t) (a+(j-7)*b +(i-7)*c + 16);
        pred[i * 16 + j] = (uint8_t) (max(0,min(255,temp>>5)));
      }
      uint32_t sad = 0;
#pragma ivdep
#pragma vector always
#pragma vector aligned
      for (int j = 0; j < 16; j++)
      {
        sad += abs (pred[i * 16 + j] - curr[i * 16 + j]);
      }
      plane_sad += sad;
    }
#else
    plane_sad = 0;
    for (int i = 0; i < 16; i++)
    {
      TAA_H264_ALIGN(64) int16_t temp[16];
#pragma ivdep
#pragma unroll(2)
#pragma vector always
#pragma vector aligned
      for (int j = 0; j < 16; j++)
      {
        temp[j] = (int16_t) (a+(j-7)*b +(i-7)*c + 16);
        temp[j] = (int16_t) (temp[j] >> 5);
      }
      uint32_t sad = 0;
#pragma ivdep
#pragma vector always
#pragma vector aligned
      for (int j = 0; j < 16; j++)
      {
        pred[i * 16 + j] = (uint8_t) ((temp[j] > 255) ? 255 : (temp[j] < 0) ? 0 : temp[j]);
        sad += abs (pred[i * 16 + j] - curr[i * 16 + j]);
      }
      plane_sad += sad;
    }
#endif
  }
  //======================================================================
  //                           Decide best mode
  //======================================================================
  min_sad = plane_sad;
  best_mode = (imode16_t) PLANE_PRED_16;
  if (hor_sad < min_sad)
  {
    min_sad = hor_sad;
    best_mode = (imode16_t) HOR_PRED_16;
  }
  if (vert_sad < min_sad)
  {
    min_sad = vert_sad;
    best_mode = (imode16_t) VERT_PRED_16;
  }
  if (dc_sad < min_sad)
  {
    min_sad = dc_sad;
    best_mode = (imode16_t) DC_PRED_16;
  }
  *mode_out = best_mode;
  return min_sad;
}


/* Estimates the SAD for intra coding this macroblock. The current original
 * input MB is used for intra prediction instead of the reconstructed blocks
 * (reconstructed blocks are not available since quantization and transform is
 * not performed), hence only an estimated SAD is achieved and not the exact
 * value.*/
static unsigned taa_h264_intra_4x4_sad_fast (
  mbinfo_t * mb,
  uint8_t *  intra_modes_ptr)
{
  unsigned mb_cost = 0;
  bool const mb_up   = MB_UP (mb->avail_flags);
  bool const mb_left = MB_LEFT (mb->avail_flags);
  const int pred_cost = (1 * mb->lambda) >> LAMBDA_SHIFT;
  const int nopred_cost = (4 * mb->lambda) >> LAMBDA_SHIFT;

  //======================================================================
  // This is just an estimation since the prediction for one 4x4 block use
  // the reconstructed values of the previous blocks which are not available
  // yet. We don't perform transform, quantization and the inverse in this
  // step.
  //======================================================================
  for (int i = 0; i < MB_HEIGHT_Y; i += 4)
  {
    //======================================================================
    //                          4 x 4 Transpose
    //======================================================================
    __m128 const row0 = _mm_load_ps((float *) &mb->luma.input[(i+0) * MB_WIDTH_Y]);
    __m128 const row1 = _mm_load_ps((float *) &mb->luma.input[(i+1) * MB_WIDTH_Y]);
    __m128 const row2 = _mm_load_ps((float *) &mb->luma.input[(i+2) * MB_WIDTH_Y]);
    __m128 const row3 = _mm_load_ps((float *) &mb->luma.input[(i+3) * MB_WIDTH_Y]);
    __m128 const tmp0 = _mm_shuffle_ps((row0), (row1), 0x44);
    __m128 const tmp2 = _mm_shuffle_ps((row0), (row1), 0xEE);
    __m128 const tmp1 = _mm_shuffle_ps((row2), (row3), 0x44);
    __m128 const tmp3 = _mm_shuffle_ps((row2), (row3), 0xEE);
    __m128i const xmm[4] =
    {
      _mm_castps_si128(_mm_shuffle_ps(tmp0, tmp1, 0x88)),
      _mm_castps_si128(_mm_shuffle_ps(tmp0, tmp1, 0xDD)),
      _mm_castps_si128(_mm_shuffle_ps(tmp2, tmp3, 0x88)),
      _mm_castps_si128(_mm_shuffle_ps(tmp2, tmp3, 0xDD))
    };
    for (int j = 0, k = 0; j < MB_WIDTH_Y; j += 4, k++)
    {
      uint32_t min_cost = UINT_MAX;
      uint8_t best_mode = DC_PRED;
      const int raster_idx = i + j/4;

      // TODO: Verify if this has any positive effect. Error due to approximation
      // is probably much more than the cost of missing pred mode.
      imode4_t pred_mode = taa_h264_predict_intra_4x4_mode (
        &intra_modes_ptr[raster_idx],
        mb->mbxmax * NUM_4x4_BLOCKS_Y,
        j != 0,
        i != 0,
        mb_left,
        mb_up);

      //======================================================================
      //                        Vertical prediction
      //======================================================================
      bool up   = (i != 0) || mb_up;
      const uint8_t * up_ptr = (i == 0) ? &mb->recon_top_y[j] : &mb->luma.input[(i - 1) * MB_WIDTH_Y + j];
      if (up)
      {
        __m128i const xmm0 = _mm_set1_epi32(*(int *)(up_ptr));
        __m128i const xmm1 = _mm_sad_epu8(xmm[k], xmm0);
        uint32_t cost = (pred_mode != VERT_PRED) ? nopred_cost : pred_cost;
        cost += _mm_cvtsi128_si32(_mm_add_epi32(xmm1, _mm_srli_si128(xmm1, 8)));
        if (cost < min_cost)
        {
          min_cost = cost;
          best_mode = VERT_PRED;
        }
      }

      //======================================================================
      //                          Horizontal prediction
      //======================================================================
      bool const left = (j != 0) || mb_left;
      uint8_t const TAA_H264_ALIGN(64) input_left[4] =
      {
        mb->luma.input[(i+0) * MB_WIDTH_Y + j - 1],
        mb->luma.input[(i+1) * MB_WIDTH_Y + j - 1],
        mb->luma.input[(i+2) * MB_WIDTH_Y + j - 1],
        mb->luma.input[(i+3) * MB_WIDTH_Y + j - 1],
      };
      uint8_t const * left_ptr = (j == 0) ? &mb->recon_left_y[i] : input_left;
      if (left)
      {
        __m128i xmm0 = _mm_cvtsi32_si128(*(int *)left_ptr);
        xmm0 = _mm_unpacklo_epi8(xmm0, xmm0);
        xmm0 = _mm_unpacklo_epi8(xmm0, xmm0);
        __m128i const xmm1 = _mm_sad_epu8(xmm[k], xmm0);
        uint32_t cost = (pred_mode != HOR_PRED) ? nopred_cost : pred_cost;
        cost += _mm_cvtsi128_si32(_mm_add_epi32(xmm1, _mm_srli_si128(xmm1, 8)));
        if (cost < min_cost)
        {
          min_cost = cost;
          best_mode = HOR_PRED;
        }
      }
      //======================================================================
      //                    DC prediction is always available
      //======================================================================
      int shift = 1;
      int sum = 0;
      int sum_left = left_ptr[0] + left_ptr[1] + left_ptr[2] + left_ptr[3];
      int sum_up   = up_ptr[0] + up_ptr[1] + up_ptr[2] + up_ptr[3];
      if (left)
      {
        sum += sum_left + 2;
        shift++;
      }
      if (up)
      {
        sum += sum_up + 2;
        shift++;
      }
      if (shift == 1)
      {
        sum = 128;
        shift = 0;
      }
      uint8_t const dc = (uint8_t)(sum >> shift);
      __m128i const xmm0 = _mm_set1_epi8(dc);
      __m128i const xmm1 = _mm_sad_epu8(xmm[k], xmm0);
      uint32_t cost = (pred_mode != DC_PRED) ? nopred_cost : pred_cost;
      cost += _mm_cvtsi128_si32(_mm_add_epi32(xmm1, _mm_srli_si128(xmm1, 8)));
      if (cost < min_cost)
      {
        min_cost = cost;
        best_mode = DC_PRED;
      }
      mb_cost += min_cost;
      intra_modes_ptr[raster_idx] = (uint8_t) best_mode;
    }
  }
  // This is not the final decision. Reset any stored intra modes.
  for (int i = 0; i < NUM_4x4_BLOCKS_Y; i++)
    intra_modes_ptr[i] = DC_PRED;

  return mb_cost;
}


static void taa_h264_save_motion_vectors (
  const int   pos,
  mv_t *      motion_vectors,
  fastpos_t * motion_vectors_16x16,
  mv_t *      mv1,
  mv_t *      mv2,
  mbtype_t    mbtype)
{
  switch (mbtype)
  {
  case P_SKIP:
  case P_16x16:
  {
    motion_vectors_16x16->x[pos] = (int16_t) mv1->x;
    motion_vectors_16x16->y[pos] = (int16_t) mv1->y;
    for (int k = 0; k < NUM_MVS_MB; k++)
    {
      motion_vectors[k].y = (int16_t) mv1->y;
      motion_vectors[k].x = (int16_t) mv1->x;
      motion_vectors[k].ref_num = mv1->ref_num;
    }
    break;
  }
  case P_16x8:
  {
    motion_vectors_16x16->x[pos] = 0;
    motion_vectors_16x16->y[pos] = 0;

    for (int k = 0; k < 8; k++)
    {
      motion_vectors[k].y = (int16_t) mv1->y;
      motion_vectors[k].x = (int16_t) mv1->x;
      motion_vectors[k].ref_num = mv1->ref_num;

      motion_vectors[k + 8].y = (int16_t) mv2->y;
      motion_vectors[k + 8].x = (int16_t) mv2->x;
      motion_vectors[k + 8].ref_num = mv2->ref_num;
    }
    break;
  }
  case P_8x16:
  {
    motion_vectors_16x16->x[pos] = 0;
    motion_vectors_16x16->y[pos] = 0;

    for (int k = 0; k < NUM_MVS_MB; k += 4)
    {
      motion_vectors[k].y = (int16_t) mv1->y;
      motion_vectors[k].x = (int16_t) mv1->x;
      motion_vectors[k].ref_num = mv1->ref_num;

      motion_vectors[k + 1].y = (int16_t) mv1->y;
      motion_vectors[k + 1].x = (int16_t) mv1->x;
      motion_vectors[k + 1].ref_num = mv1->ref_num;

      motion_vectors[k + 2].y = (int16_t) mv2->y;
      motion_vectors[k + 2].x = (int16_t) mv2->x;
      motion_vectors[k + 2].ref_num = mv2->ref_num;

      motion_vectors[k + 3].y = (int16_t) mv2->y;
      motion_vectors[k + 3].x = (int16_t) mv2->x;
      motion_vectors[k + 3].ref_num = mv2->ref_num;
    }
    break;
  }
  default:
  {
    break;
  }
  }
}

static void taa_h264_set_predicted_motion_vectors (
  mbinfo_t * mb,
  mv_t *     motion_vectors,
  int        mvs_stride)
{
  bool mb_left = MB_LEFT (mb->avail_flags);
  bool mb_up = MB_UP (mb->avail_flags);
  bool mb_upright = MB_UP_RIGHT (mb->avail_flags);
  bool mb_upleft = MB_UP_LEFT (mb->avail_flags);

  bool block_left = mb_left;
  bool block_up = mb_up;
  bool block_upright = mb_upright;
  bool block_upleft = mb_upleft;

  switch (mb->mbtype)
  {
  case P_SKIP:
  case P_16x16:
  {
    mb->pred1 = taa_h264_predict_motion_vector (motion_vectors,
                                                mvs_stride,
                                                0, 16, 16,
                                                mb->mv1.ref_num,
                                                block_left,
                                                block_up,
                                                block_upright,
                                                block_upleft);
    break;
  }
  case P_16x8:
  {
    mb->pred1 = taa_h264_predict_motion_vector (motion_vectors,
                                                mvs_stride,
                                                0, 8, 16,
                                                mb->mv1.ref_num,
                                                block_left,
                                                block_up,
                                                block_upright,
                                                block_upleft);

    block_up = true;
    block_upright = false;
    block_upleft = mb_left;
    mb->pred2 = taa_h264_predict_motion_vector (motion_vectors,
                                                mvs_stride,
                                                8, 8, 16,
                                                mb->mv2.ref_num,
                                                block_left,
                                                block_up,
                                                block_upright,
                                                block_upleft);
    break;
  }
  case P_8x16:
  {
    block_upright = mb_up;
    mb->pred1 = taa_h264_predict_motion_vector (motion_vectors,
                                                mvs_stride,
                                                0, 16, 8,
                                                mb->mv1.ref_num,
                                                block_left,
                                                block_up,
                                                block_upright,
                                                block_upleft);
    block_left = true;
    block_upright = mb_upright;
    block_upleft = mb_up;
    mb->pred2 = taa_h264_predict_motion_vector (motion_vectors,
                                                mvs_stride,
                                                2, 16, 8,
                                                mb->mv2.ref_num,
                                                block_left,
                                                block_up,
                                                block_upright,
                                                block_upleft);
    break;
  }
  default:
  {
    TAA_H264_DEBUG_ASSERT (false);
    break;
  }
  }
}

static void taa_h264_set_motion_vectors_not_avail (
  const int   pos,
  mv_t        motion_vectors_curr[],
  fastpos_t * motion_vectors_curr_16x16
  )
{
  motion_vectors_curr_16x16->x[pos] = 0;
  motion_vectors_curr_16x16->y[pos] = 0;

  for (int i = 0; i < NUM_MVS_MB; i++)
  {
    motion_vectors_curr[i].x = 0;
    motion_vectors_curr[i].y = 0;
    motion_vectors_curr[i].ref_num = -1;
  }
}


#define INTRA_NORMAL 1
#define INTRA_SKIP   2

static void taa_h264_mb_mode_decision (
  const int     stride,
  const int     cstride,
  const int     pos,
  const int     xpos,
  const int     ypos,
  const int     cxpos,
  const int     cypos,
  mbinfo_t *    currmb,
  frameinfo_t * frameinfo,
  int           intra,
  mv_t *        motion_vectors_curr,
  fastpos_t *   motion_vectors_curr_16x16,
  mv_t          pred,
  mv_t          cand[4],
  const unsigned coding_options)
{
  mbtype_t best_mbtype = I_4x4;
  unsigned cost;
  unsigned min_cost = UINT_MAX;

  if (!intra)
  {
    mbtype_t best_mode;
    cost = taa_h264_motion_estimate (
      stride,
      frameinfo->meinfo,
      currmb->mquant,
      &currmb->mv1,
      &currmb->mv2,
      &best_mode,
      pred,
      cand,
      currmb->luma.input,
      currmb->luma.recon,
      currmb->chroma.recon
      );

    int count_8x8_enc = 4;
    if (frameinfo->meinfo->sad8x8[0] < currmb->thr * 4)
    {
      currmb->luma.do_4x4_enc[0] = 0;
      currmb->luma.do_4x4_enc[1] = 0;
      currmb->luma.do_4x4_enc[4] = 0;
      currmb->luma.do_4x4_enc[5] = 0;
      count_8x8_enc--;
    }
    if (frameinfo->meinfo->sad8x8[1] < currmb->thr * 4)
    {
      currmb->luma.do_4x4_enc[2] = 0;
      currmb->luma.do_4x4_enc[3] = 0;
      currmb->luma.do_4x4_enc[6] = 0;
      currmb->luma.do_4x4_enc[7] = 0;
      count_8x8_enc--;
    }
    if (frameinfo->meinfo->sad8x8[2] < currmb->thr * 4)
    {
      currmb->luma.do_4x4_enc[8]  = 0;
      currmb->luma.do_4x4_enc[9]  = 0;
      currmb->luma.do_4x4_enc[12] = 0;
      currmb->luma.do_4x4_enc[13] = 0;
      count_8x8_enc--;
    }
    if (frameinfo->meinfo->sad8x8[3] < currmb->thr * 4)
    {
      currmb->luma.do_4x4_enc[10] = 0;
      currmb->luma.do_4x4_enc[11] = 0;
      currmb->luma.do_4x4_enc[14] = 0;
      currmb->luma.do_4x4_enc[15] = 0;
      count_8x8_enc--;
    }

    if (cost < min_cost)
    {
      min_cost = cost;
      best_mbtype = best_mode;
    }

    // Check if we should do skip instead of P_16x16
    // threshold multiplier found by auto tuning
    int thr = (currmb->thr * (160-9) + 8) >> 4;
    if (best_mbtype == P_16x16 &&
      currmb->mv1.vec == currmb->mvskip.vec &&
      count_8x8_enc == 0 &&
      min_cost < thr)
    {
      best_mbtype = P_SKIP;
    }
  }

  if (best_mbtype != P_SKIP)
  {
    const framebuf_t * recon = &frameinfo->recon;
    //===============================================================
    //                     Intra prediction
    //===============================================================
    if (MB_UP (currmb->avail_flags))
    {
      const uint8_t * recon_y = &recon->y [(ypos - 1) * stride + xpos];
      const uint8_t * recon_u = &recon->u [(cypos - 1) * cstride + cxpos];
      const uint8_t * recon_v = &recon->v [(cypos - 1) * cstride + cxpos];
      uint8_t *  top_y = currmb->recon_top_y;
      uint8_t *  top_u = currmb->recon_top_u;
      uint8_t *  top_v = currmb->recon_top_v;

      // In addition to left and top, some 4x4 intra modes use values from top
      // left and top right MB. No availability tests needed because recon is padded.
#pragma vector aligned
      for (int i = 0; i < MB_WIDTH_Y+4; i++)
        top_y[i] = recon_y [i];

#pragma vector aligned
      for (int i = 0; i < MB_WIDTH_UV; i++)
      {
        top_u[i] = recon_u[i];
        top_v[i] = recon_v[i];
      }
    }
    if (MB_LEFT (currmb->avail_flags))
    {
      const uint8_t * recon_y = &recon->y [ypos * stride + xpos - 1];
      const uint8_t * recon_u = &recon->u [cypos * cstride + cxpos - 1];
      const uint8_t * recon_v = &recon->v [cypos * cstride + cxpos - 1];
      uint8_t *  left_y = currmb->recon_left_y;
      uint8_t *  left_u = currmb->recon_left_u;
      uint8_t *  left_v = currmb->recon_left_v;

      for (int i = 0; i < MB_HEIGHT_Y; i++)
        left_y[i] = recon_y[i * stride];

      for (int i = 0; i < MB_HEIGHT_UV; i++)
      {
        left_u[i] = recon_u[i * cstride];
        left_v[i] = recon_v[i * cstride];
      }
    }
    if (MB_UP_LEFT (currmb->avail_flags))
    {
      currmb->recon_topleft_y = recon->y[(ypos - 1) * stride + xpos - 1];
    }

    // No bias ("cost penalty") for intra 16x16
    cost = taa_h264_intra_16x16_sad (currmb);

    if (cost < min_cost)
    {
      min_cost = cost;
      best_mbtype = I_16x16;
    }

#if TRAILING_ARTIFACT_REMOVAL == 1
    if (intra == INTRA_NORMAL)
#endif
    {
      uint8_t * intra_modes_ptr = &frameinfo->intra_modes[currmb->mbpos * NUM_4x4_BLOCKS_Y];
      if (!intra)
      {
        currmb->approx_i4x4_cost = taa_h264_intra_4x4_sad_fast (currmb, intra_modes_ptr);
        cost = currmb->approx_i4x4_cost + frameinfo->approx_i4x4_penalty + ((currmb->lambda * 24) >> LAMBDA_SHIFT);
        if (cost < min_cost)
        {
          min_cost = cost;
          best_mbtype = I_4x4;
        }
      }
      else
      {
        // On intra frames we do a better i4x4 decision, doing the full transform
        // and quantization. We set/store variabels so that we don't have to do that
        // again later
#ifdef HAVE_SSSE3
        currmb->approx_i4x4_cost = taa_h264_encode_i4x4_luma_ssse3 (intra, currmb, intra_modes_ptr, coding_options);
#else
        currmb->approx_i4x4_cost = taa_h264_encode_i4x4_luma (intra, currmb, intra_modes_ptr, coding_options);
#endif
        cost = currmb->approx_i4x4_cost + ((currmb->lambda * 24) >> LAMBDA_SHIFT);
        if (cost < min_cost)
        {
          min_cost = cost;
          best_mbtype = I_4x4;
        }
        else
        {
          // Reset variables modified by the i4x4 encode function
          for (int i = 0; i < NUM_4x4_BLOCKS_Y; i++)
            intra_modes_ptr[i] = DC_PRED;
        }
      }
    }
  }

  currmb->mbtype = best_mbtype;

  if (!MB_TYPE_IS_INTRA (currmb->mbtype))
  {
    taa_h264_save_motion_vectors (
      pos,
      motion_vectors_curr,
      motion_vectors_curr_16x16,
      &currmb->mv1,
      &currmb->mv2,
      currmb->mbtype);

    taa_h264_set_predicted_motion_vectors (currmb, motion_vectors_curr,
                                           currmb->mbxmax * NUM_MVS_MB);
  }
  else
  {
    taa_h264_set_motion_vectors_not_avail (pos, motion_vectors_curr, motion_vectors_curr_16x16);
  }
}




static void taa_h264_split_sad_aligned_4x8x8(
  const uint8_t * cur,
  const uint8_t * ref,
  uint16_t        sad[4]
  )
{
  __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
  __m128i sum0, sum1;
  sum0 = _mm_setzero_si128();
  sum1 = _mm_setzero_si128();

  xmm0 = _mm_sad_epu8(*(__m128i *)(&cur[0 * MB_WIDTH_Y]), *(__m128i *)(&ref[0 * MB_WIDTH_Y]));
  xmm1 = _mm_sad_epu8(*(__m128i *)(&cur[1 * MB_WIDTH_Y]), *(__m128i *)(&ref[1 * MB_WIDTH_Y]));
  xmm2 = _mm_sad_epu8(*(__m128i *)(&cur[2 * MB_WIDTH_Y]), *(__m128i *)(&ref[2 * MB_WIDTH_Y]));
  xmm3 = _mm_sad_epu8(*(__m128i *)(&cur[3 * MB_WIDTH_Y]), *(__m128i *)(&ref[3 * MB_WIDTH_Y]));
  xmm4 = _mm_sad_epu8(*(__m128i *)(&cur[4 * MB_WIDTH_Y]), *(__m128i *)(&ref[4 * MB_WIDTH_Y]));
  xmm5 = _mm_sad_epu8(*(__m128i *)(&cur[5 * MB_WIDTH_Y]), *(__m128i *)(&ref[5 * MB_WIDTH_Y]));
  xmm6 = _mm_sad_epu8(*(__m128i *)(&cur[6 * MB_WIDTH_Y]), *(__m128i *)(&ref[6 * MB_WIDTH_Y]));
  xmm7 = _mm_sad_epu8(*(__m128i *)(&cur[7 * MB_WIDTH_Y]), *(__m128i *)(&ref[7 * MB_WIDTH_Y]));

  sum0 = _mm_add_epi16(sum0, xmm0);
  sum0 = _mm_add_epi16(sum0, xmm1);
  sum0 = _mm_add_epi16(sum0, xmm2);
  sum0 = _mm_add_epi16(sum0, xmm3);
  sum0 = _mm_add_epi16(sum0, xmm4);
  sum0 = _mm_add_epi16(sum0, xmm5);
  sum0 = _mm_add_epi16(sum0, xmm6);
  sum0 = _mm_add_epi16(sum0, xmm7);

  xmm8  = _mm_sad_epu8(*(__m128i *)(&cur[ 8 * MB_WIDTH_Y]), *(__m128i *)(&ref[ 8 * MB_WIDTH_Y]));
  xmm9  = _mm_sad_epu8(*(__m128i *)(&cur[ 9 * MB_WIDTH_Y]), *(__m128i *)(&ref[ 9 * MB_WIDTH_Y]));
  xmm10 = _mm_sad_epu8(*(__m128i *)(&cur[10 * MB_WIDTH_Y]), *(__m128i *)(&ref[10 * MB_WIDTH_Y]));
  xmm11 = _mm_sad_epu8(*(__m128i *)(&cur[11 * MB_WIDTH_Y]), *(__m128i *)(&ref[11 * MB_WIDTH_Y]));
  xmm12 = _mm_sad_epu8(*(__m128i *)(&cur[12 * MB_WIDTH_Y]), *(__m128i *)(&ref[12 * MB_WIDTH_Y]));
  xmm13 = _mm_sad_epu8(*(__m128i *)(&cur[13 * MB_WIDTH_Y]), *(__m128i *)(&ref[13 * MB_WIDTH_Y]));
  xmm14 = _mm_sad_epu8(*(__m128i *)(&cur[14 * MB_WIDTH_Y]), *(__m128i *)(&ref[14 * MB_WIDTH_Y]));
  xmm15 = _mm_sad_epu8(*(__m128i *)(&cur[15 * MB_WIDTH_Y]), *(__m128i *)(&ref[15 * MB_WIDTH_Y]));

  sum1 = _mm_add_epi16(sum1, xmm8);
  sum1 = _mm_add_epi16(sum1, xmm9);
  sum1 = _mm_add_epi16(sum1, xmm10);
  sum1 = _mm_add_epi16(sum1, xmm11);
  sum1 = _mm_add_epi16(sum1, xmm12);
  sum1 = _mm_add_epi16(sum1, xmm13);
  sum1 = _mm_add_epi16(sum1, xmm14);
  sum1 = _mm_add_epi16(sum1, xmm15);

  sum0 = _mm_packs_epi32(sum0, sum1);
  sum0 = _mm_packs_epi32(sum0, sum0);
  _mm_storel_epi64((__m128i *) sad, sum0);
}

#ifdef HAVE_SSSE3
void taa_h264_init_mb_ssse3 (
#else
void taa_h264_init_mb (
#endif
  const int stride,
  const int cstride,
  const int quant,
  const int xpos,
  const int ypos,
  const int cxpos,
  const int cypos,
  const int mbxmax,
  const int mbymax,
  const int height,
  const int width,
  bool islice,
  frameinfo_t *        frameinfo,
  mbinfo_t *           mb,
  int mby,
  int mbx,
  int first_mb_in_slice,
  mv_t *  motion_vectors_curr,
  const mv_t *         motion_vectors_prev,
  fastpos_t *  motion_vectors_curr_16x16,
  meinfo_t *           me,
  const unsigned coding_options)
{
  const int mvs_stride = mbxmax * NUM_MVS_MB;
  const int pos = mby * mbxmax + mbx;
  mv_t pred;
  int intra = islice ? INTRA_NORMAL : 0;

  mb->mby = mby;
  mb->mbx = mbx;
  mb->mbpos = mby * mbxmax + mbx;
  mb->mbxmax = mbxmax;
  
  memset(&(mb->mbcoeffs), 0, sizeof(mb->mbcoeffs));
  memset(&(mb->mvd[0][0][0]), 0, sizeof(mb->mvd));

  taa_h264_set_mb_availability (
    mbx,
    mby,
    mbxmax,
    mbymax,
    first_mb_in_slice,
    0x0,
    false,
    &mb->avail_flags);

  mb->mbtype = I_4x4;
  mb->mv1.x = 0;
  mb->mv1.y = 0;
  mb->mv2.x = 0;
  mb->mv2.y = 0;
  mb->mvskip.x = 0;
  mb->mvskip.y = 0;

  /* TODO: We don't respect the max mv's according to H.264 Level yet (Table A-1) */
  mb->max_mv_top_left.x = (int16_t)((-xpos - PAD_HORZ_Y) * 4);
  mb->max_mv_top_left.y = (int16_t)((-ypos - PAD_VERT_Y) * 4);
  mb->max_mv_bottom_right.x = (int16_t)((width + PAD_HORZ_Y - xpos - MB_WIDTH_Y) * 4);
  mb->max_mv_bottom_right.y = (int16_t)((height + PAD_VERT_Y - ypos - MB_HEIGHT_Y) * 4);

  mb->mquant = quant;
  mb->lambda = taa_h264_qp2quant [quant];
  // For I slices we set lambda to weight in favor of better sad compared to bits used
  // 0.625 multiplier found by auto tuning
  if (islice)
    mb->lambda =  (mb->lambda * 80) >> 7;

  mb->thr = thr_table[quant];
  if (coding_options & TAA_H264_HQ_DECREASED_THRESHOLD)
    mb->thr /= 2;

  mv_t * skip_mv = &mb->mvskip;

  const int16_t cand_xmin = (int16_t)max(((-xpos - 19) * 4), -500);
  const int16_t cand_ymin = (int16_t)max(((-ypos - 19) * 4), -500);
  const int16_t cand_xmax = (int16_t)min(((me->width - xpos - MB_WIDTH_Y + 18) * 4), 500);
  const int16_t cand_ymax = (int16_t)min(((me->height - ypos - MB_HEIGHT_Y + 18) * 4), 500);

  bool can_skip = false;

  //===============================================================
  //                   Prepare for motion search
  //===============================================================
  if (!intra)
  {
    me->xpos = xpos;
    me->ypos = ypos;
    me->lambda = mb->lambda;
    me->thr = mb->thr * 6;

    taa_h264_candidate_motion_vectors (
      mb->avail_flags,
      mbxmax * NUM_MVS_MB,
      me->ref_num,
      motion_vectors_curr,
      &pred,
      skip_mv
      );

    //===============================================================
    //               Store skip block if skip possible
    //===============================================================

    bool vector_outside_padding = (skip_mv->x < cand_xmin)
                                  || (skip_mv->y < cand_ymin)
                                  || (skip_mv->x + ((skip_mv->x & 3) == 3) > cand_xmax)
                                  || (skip_mv->y + ((skip_mv->y & 3) == 3) > cand_ymax);

    if (!vector_outside_padding)
    {
      /* TODO: Less concervative clipping of motion vector so that we
       * can get more skip at edge boundaries. */
      /* const int xref  = taa_h264_clip (xpos + (skip_mv->x >> 2) + PAD_HORZ_Y, -PAD_HORZ_Y + 3, PAD_HORZ_Y - 3); */
      /* const int yref  = taa_h264_clip (ypos + (skip_mv->y >> 2) + PAD_VERT_Y, -PAD_VERT_Y + 3, PAD_VERT_Y - 3); */
      const int xref  = xpos + (skip_mv->x >> 2) + PAD_HORZ_Y;
      const int yref  = ypos + (skip_mv->y >> 2) + PAD_VERT_Y;
      const int zfrac = 4 * (skip_mv->y & 3) + (skip_mv->x & 3);

      can_skip = (frameinfo->meinfo->ref_num == 0);

      if (can_skip)
      {
        const int cxref = xref >> 1;
        const int cyref = yref >> 1;
        const int xfrac = skip_mv->x & 7;
        const int yfrac = skip_mv->y & 7;
        taa_h264_save_best_chroma_8x8(
          xfrac,
          yfrac,
          cstride,
          &me->ref_full->u[cyref*cstride + cxref],
          &me->ref_full->v[cyref*cstride + cxref],
          mb->chroma.recon);

        taa_h264_save_best_16x16(
          stride,
          zfrac,
          &me->ref_full->y[yref * stride + xref],
          mb->luma.recon);
      }
    }
  }

#ifdef TAA_DEBUG_SAVE_MEINFO
  saved_avail_flags[poss] = currmb.avail_flags;
#endif

  bool early_skip = false;
  bool do_motion_search = true;
  mb->encode_luma = true;
  mb->encode_chroma = true;

  if (can_skip && !(coding_options & TAA_H264_XQ_DISABLE_EARLY_SKIP))
  {
    unsigned early_flags = 0;
    // TODO: Only do early skip for skip vector == 0?
#ifdef HAVE_SSSE3
    early_flags = taa_h264_check_early_skip_ssse3 (
      &mb->luma,
      &mb->chroma,
      quant);
#else
    early_flags = taa_h264_check_early_skip_sse2 (
      &mb->luma,
      &mb->chroma,
      quant);
#endif

    if (early_flags & EARLY_SKIP_INTRA_FLAG)
    {
      intra = INTRA_SKIP;
    }
    else
    {
      /* We use information for early skip routine to determine if it's worth
      * encoding coefficient for luma or chroma */
      early_skip        = early_flags & EARLY_SKIP_FLAG            ? true : false;
      mb->encode_luma   = early_flags & EARLY_SKIP_CBP_LUMA_FLAG   ? false : true;
      mb->encode_chroma = early_flags & EARLY_SKIP_CBP_CHROMA_FLAG ? false : true;

#if EARLY_SKIP_MV_SEARCH
      if (early_skip == false && mb->encode_luma == false &&
        !(coding_options & TAA_H264_HQ_DISABLE_EARLY_SKIP_MV_SEARCH))
      {
        /* If the early skip test returns that luma could be skipped but we
        * don't have a early skip MB (because of chroma), check if it's ok to
        * skip motion search */
        uint16_t sad8x8[4];
        taa_h264_split_sad_aligned_4x8x8 (mb->luma.input, mb->luma.recon, sad8x8);
        const int max_cost_8x8 = max (max (sad8x8[0], sad8x8[1]), max (sad8x8[2], sad8x8[3]));
        if (max_cost_8x8 < mb->thr * 6)
          do_motion_search = false;

      }
#endif
    }
  }

  if (early_skip)
  {
    mb->mv1 = mb->mvskip;
    mb->mv2 = mb->mvskip;
    mb->mbtype = P_SKIP;
  }
  else if (do_motion_search == false)
  {
    //========================================================
    //                      LUMA SKIP
    //========================================================
    mb->mbtype = P_16x16;
    mb->mv1 = mb->mvskip;
    mb->mv2 = mb->mvskip;

    taa_h264_save_motion_vectors (pos,
                                  motion_vectors_curr,
                                  motion_vectors_curr_16x16,
                                  &mb->mv1,
                                  &mb->mv2,
                                  mb->mbtype);

    taa_h264_set_predicted_motion_vectors (mb, motion_vectors_curr,
                                           mb->mbxmax * NUM_MVS_MB);
  }
  else
  {
    mv_t cand[4] = {0, };

    if (!intra)
    {
      TAA_H264_ALIGN (16) int16_t cand_x[16];
      TAA_H264_ALIGN (16) int16_t cand_y[16];

      const mv_t mv_null = { 0, 0, .ref_num = -1};

      const bool left_avail    = MB_LEFT (mb->avail_flags);
      const bool up_avail      = MB_UP   (mb->avail_flags);
      const bool left_inside   = MB_LEFT_INSIDE    (mb->avail_flags);
      const bool up_inside     = MB_UP_INSIDE      (mb->avail_flags);
      const bool rigth_inside  = MB_RIGHT_INSIDE   (mb->avail_flags);
      const bool bottom_inside = MB_BOTTOM_INSIDE  (mb->avail_flags);

      const int left_offset = -13;
      const int up_offset = -mvs_stride + 12;

      const int _left   = left_inside ? left_offset : 0;
      const int _up     = up_inside ? up_offset : 0;
      const int _right  = rigth_inside ? 16 : 0;
      const int _bottom = bottom_inside ? mvs_stride : 0;

      const mv_t centre = pred;
      const mv_t mva = left_avail ? motion_vectors_curr [left_offset] : mv_null;
      const mv_t mvb = up_avail ? motion_vectors_curr [up_offset] : mv_null;
      const mv_t skp = *skip_mv;

      //====================
      // FAST OBJECT MOTION
      //====================
      fast_t fast = {0, };
#ifndef DISABLE_FAST_OBJECT_MOTION
      // TODO: This does not compile anymore. Fix it or delete it
      fast = taa_h264_fast_object_motion(
        ypos,
        height,
        mbxmax,
        &motion_vectors_prev_16x16->x[pos],
        &motion_vectors_prev_16x16->y[pos]);
#endif

      cand_x[0]  = (int16_t)(centre.x);                       cand_y[0]  = (int16_t)(centre.y);
      cand_x[1]  = (int16_t)(skp.x);                          cand_y[1]  = (int16_t)(skp.y);
      cand_x[2]  = (int16_t)(mva.x);                          cand_y[2]  = (int16_t)(mva.y);
      cand_x[3]  = (int16_t)(mvb.x);                          cand_y[3]  = (int16_t)(mvb.y);
      cand_x[4]  = (int16_t)(fast.vector.x);                  cand_y[4]  = (int16_t)(fast.vector.y);
      cand_x[5]  = (int16_t)(fast.vector_outer.x);            cand_y[5]  = (int16_t)(fast.vector_outer.y);
      cand_x[6]  = (int16_t)(motion_vectors_prev[0].x);       cand_y[6]  = (int16_t)(motion_vectors_prev[0].y);
      cand_x[7]  = (int16_t)(motion_vectors_prev[_right].x);  cand_y[7]  = (int16_t)(motion_vectors_prev[_right].y);
      cand_x[8]  = (int16_t)(motion_vectors_prev[_bottom].x); cand_y[8]  = (int16_t)(motion_vectors_prev[_bottom].y);
      cand_x[9]  = (int16_t)(motion_vectors_prev[_left].x);   cand_y[9]  = (int16_t)(motion_vectors_prev[_left].y);
      cand_x[10] = (int16_t)(motion_vectors_prev[_up].x);     cand_y[10] = (int16_t)(motion_vectors_prev[_up].y);
      cand_x[11] = (int16_t)(centre.x + 0);                   cand_y[11] = (int16_t)(centre.y - 4);
      cand_x[12] = (int16_t)(centre.x + 0);                   cand_y[12] = (int16_t)(centre.y + 4);
      cand_x[13] = (int16_t)(centre.x - 4);                   cand_y[13] = (int16_t)(centre.y + 0);
      cand_x[14] = (int16_t)(centre.x + 4);                   cand_y[14] = (int16_t)(centre.y + 0);
      cand_x[15] = (int16_t)(centre.x + 4);                   cand_y[15] = (int16_t)(centre.y + 4);

#pragma ivdep
#pragma vector aligned
      for(int i = 0; i<16; i++)
      {
        cand_x[i] = (int16_t)(min(max(cand_xmin, cand_x[i]), cand_xmax));
        cand_y[i] = (int16_t)(min(max(cand_ymin, cand_y[i]), cand_ymax));
      }

      int nv;
      int nn = 0;
      uint32_t check_mask = 0;
      for(nv = 0; nv < 16; nv++)
      {
        int16_t vx = (int16_t)((cand_x[nv]+2) & 0xfffc);
        int16_t vy = (int16_t)((cand_y[nv]+2) & 0xfffc);

        if ((vx | vy) != 0)     // Do not store (0,0)
        {
          uint32_t m = (uint32_t)(1 << (((vx) ^ (vy >> 2)) & 31));
          int ok = ((m & check_mask) == 0) ? 1 : 0;
          cand[nn].x = vx;
          cand[nn].y = vy;
          nn += ok;
          check_mask |= m;
          if (nn == 4) break;
        }
      }
    }

    TAA_H264_TRACE (TAA_H264_TRACE_KEY_VALUES, "mbpos = %d, qp = %d\n", mb->mbpos, quant);
    //========================================================
    //                      MODE DECISION
    //========================================================
    taa_h264_mb_mode_decision (
      stride,
      cstride,
      pos,
      xpos,
      ypos,
      cxpos,
      cypos,
      mb,
      frameinfo,
      intra,
      motion_vectors_curr,
      motion_vectors_curr_16x16,
      pred,
      cand,
      coding_options);
    TAA_H264_TRACE (TAA_H264_TRACE_KEY_VALUES, "mbtype %d\n", mb->mbtype);

    const bool inter_type = !MB_TYPE_IS_INTRA (mb->mbtype);
    const bool early_skip_test_done = can_skip;
    const bool mv_is_skipmv =
      mb->mv1.x == mb->mvskip.x
      && mb->mv1.y == mb->mvskip.y
      && mb->mv2.x == mb->mvskip.x
      && mb->mv2.y == mb->mvskip.y;

    if (inter_type && !(early_skip_test_done && mv_is_skipmv))
    {
#if MIDDLE_SKIP_CBP_CHECK
      int early_flags;
      //========================================================
      //                      MIDDLE SKIP
      //========================================================
# ifdef HAVE_SSSE3
      early_flags = taa_h264_check_early_skip_ssse3 (
        &mb->luma,
        &mb->chroma,
        quant);
# else
      early_flags = taa_h264_check_early_skip_sse2 (
        &mb->luma,
        &mb->chroma,
        quant);
# endif

      mb->encode_luma   = early_flags & EARLY_SKIP_CBP_LUMA_FLAG   ? false : true;
      mb->encode_chroma = early_flags & EARLY_SKIP_CBP_CHROMA_FLAG ? false : true;
#endif
    }
  }
}
