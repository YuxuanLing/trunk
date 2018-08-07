#include "com_interpolation_ref.h"
#include "com_common.h"
#include "com_intmath.h"
#include <stdlib.h>
#include <string.h>


/**
 * A bit naive and straightforward implementation of the interpolation. Based
 * on Langley's H.264 RCDO implementation, but modified to H.264.
 */

#pragma warning (disable: 981)

typedef uint8_t (*taa_h264_get_pixel_fn)(const uint8_t * src, int ss);

static uint8_t average_up(
  taa_h264_get_pixel_fn a,
  taa_h264_get_pixel_fn b,
  const uint8_t *       src,
  int                   ss)
{
  return (uint8_t) max(0, min(255, ((a(src, ss) + b(src, ss) + 1) >> 1)));
}

/* static uint8_t average_down(taa_h264_get_pixel_fn a, taa_h264_get_pixel_fn b, const uint8_t *src, int ss) */
/* { */
/*     return clip((a(src, ss) + b(src,ss)) >> 1); */
/* } */

static uint8_t get_whole_pixel(
  int             dx,
  int             dy,
  const uint8_t * src,
  int             ss)
{
  return src[dy * ss + dx];
}

static uint8_t rl_A(
  const uint8_t * src,
  int             ss)
{
  return get_whole_pixel( 0, -2, src, ss);
}
/* static uint8_t rl_B(const uint8_t *src, int ss) { return get_whole_pixel( 1, -2, src, ss); } */
static uint8_t rl_C(
  const uint8_t * src,
  int             ss)
{
  return get_whole_pixel( 0, -1, src, ss);
}
/* static uint8_t rl_D(const uint8_t *src, int ss) { return get_whole_pixel( 1, -1, src, ss); } */
static uint8_t rl_E(
  const uint8_t * src,
  int             ss)
{
  return get_whole_pixel(-2,  0, src, ss);
}
static uint8_t rl_F(
  const uint8_t * src,
  int             ss)
{
  return get_whole_pixel(-1,  0, src, ss);
}
static uint8_t rl_G(
  const uint8_t * src,
  int             ss)
{
  return get_whole_pixel( 0,  0, src, ss);
}
static uint8_t rl_H(
  const uint8_t * src,
  int             ss)
{
  return get_whole_pixel( 1,  0, src, ss);
}
static uint8_t rl_I(
  const uint8_t * src,
  int             ss)
{
  return get_whole_pixel( 2,  0, src, ss);
}
static uint8_t rl_J(
  const uint8_t * src,
  int             ss)
{
  return get_whole_pixel( 3,  0, src, ss);
}
/* static uint8_t rl_K(const uint8_t *src, int ss) { return get_whole_pixel(-2,  1, src, ss); } */
/* static uint8_t rl_L(const uint8_t *src, int ss) { return get_whole_pixel(-1,  1, src, ss); } */
static uint8_t rl_M(
  const uint8_t * src,
  int             ss)
{
  return get_whole_pixel( 0,  1, src, ss);
}
/* static uint8_t rl_N(const uint8_t *src, int ss) { return get_whole_pixel( 1,  1, src, ss); } */
/* static uint8_t rl_P(const uint8_t *src, int ss) { return get_whole_pixel( 2,  1, src, ss); } */
/* static uint8_t rl_Q(const uint8_t *src, int ss) { return get_whole_pixel( 3,  1, src, ss); } */
static uint8_t rl_R(
  const uint8_t * src,
  int             ss)
{
  return get_whole_pixel( 0,  2, src, ss);
}
/* static uint8_t rl_S(const uint8_t *src, int ss) { return get_whole_pixel( 1,  2, src, ss); } */
static uint8_t rl_T(
  const uint8_t * src,
  int             ss)
{
  return get_whole_pixel( 0,  3, src, ss);
}
/* static uint8_t rl_U(const uint8_t *src, int ss) { return get_whole_pixel( 1,  3, src, ss); } */

static int rl_b1(
  const uint8_t * src,
  int             ss);
static int rl_h1(
  const uint8_t * src,
  int             ss);
static int rl_m1(
  const uint8_t * src,
  int             ss);
static int rl_s1(
  const uint8_t * src,
  int             ss);

static uint8_t rl_a(
  const uint8_t * src,
  int             ss);
static uint8_t rl_b(
  const uint8_t * src,
  int             ss);
static uint8_t rl_c(
  const uint8_t * src,
  int             ss);
static uint8_t rl_d(
  const uint8_t * src,
  int             ss);
static uint8_t rl_e(
  const uint8_t * src,
  int             ss);
static uint8_t rl_f(
  const uint8_t * src,
  int             ss);
static uint8_t rl_g(
  const uint8_t * src,
  int             ss);
static uint8_t rl_h(
  const uint8_t * src,
  int             ss);
static uint8_t rl_i(
  const uint8_t * src,
  int             ss);
static uint8_t rl_j(
  const uint8_t * src,
  int             ss);
static uint8_t rl_k(
  const uint8_t * src,
  int             ss);
static uint8_t rl_m(
  const uint8_t * src,
  int             ss);
static uint8_t rl_n(
  const uint8_t * src,
  int             ss);
static uint8_t rl_p(
  const uint8_t * src,
  int             ss);
static uint8_t rl_q(
  const uint8_t * src,
  int             ss);
static uint8_t rl_r(
  const uint8_t * src,
  int             ss);
static uint8_t rl_s(
  const uint8_t * src,
  int             ss);

static int rl_cc(
  const uint8_t * src,
  int             ss);
static int rl_dd(
  const uint8_t * src,
  int             ss);
static int rl_ee(
  const uint8_t * src,
  int             ss);
static int rl_ff(
  const uint8_t * src,
  int             ss);

/* static int rl_aa(const uint8_t *src, int ss); */
/* static int rl_bb(const uint8_t *src, int ss); */
/* static int rl_gg(const uint8_t *src, int ss); */
/* static int rl_hh(const uint8_t *src, int ss); */


static int rl_b1(
  const uint8_t * src,
  int             ss)
{
  return rl_E(src, ss)
         -  5 * rl_F(src, ss)
         + 20 * rl_G(src, ss)
         + 20 * rl_H(src, ss)
         -  5 * rl_I(src, ss)
         +      rl_J(src, ss);
}

static int rl_h1(
  const uint8_t * src,
  int             ss)
{
  return rl_A(src, ss)
         -  5 * rl_C(src, ss)
         + 20 * rl_G(src, ss)
         + 20 * rl_M(src, ss)
         -  5 * rl_R(src, ss)
         +      rl_T(src, ss);
}

static int rl_m1(
  const uint8_t * src,
  int             ss)
{
  return rl_h1 (src + 1, ss);
}

static int rl_s1(
  const uint8_t * src,
  int             ss)
{
  return rl_b1 (src + ss, ss);
}

static uint8_t rl_b(
  const uint8_t * src,
  int             ss)
{
  return (uint8_t) max(0, min(255, ((rl_b1 (src, ss) + 16) >> 5)));
}

static uint8_t rl_h(
  const uint8_t * src,
  int             ss)
{
  return (uint8_t) max(0, min(255, ((rl_h1 (src, ss) + 16) >> 5)));
}

static uint8_t rl_m(
  const uint8_t * src,
  int             ss)
{
  return (uint8_t) max(0, min(255, ((rl_m1 (src, ss) + 16) >> 5)));
}

static uint8_t rl_s(
  const uint8_t * src,
  int             ss)
{
  return (uint8_t) (uint8_t) max(0, min(255, ((rl_s1 (src, ss) + 16) >> 5)));
}

static uint8_t rl_j(
  const uint8_t * src,
  int             ss)
{
  int j1 = rl_cc (src, ss)
           - 5  * rl_dd (src, ss)
           + 20 * rl_h1 (src, ss)
           + 20 * rl_m1 (src, ss)
           - 5  * rl_ee (src, ss)
           +      rl_ff (src, ss);
  return (uint8_t) max(0, min(255, ((j1 + 512) >> 10)));
}

static uint8_t rl_a(
  const uint8_t * src,
  int             ss)
{
  return average_up(rl_G, rl_b, src, ss);
}
static uint8_t rl_c(
  const uint8_t * src,
  int             ss)
{
  return average_up(rl_H, rl_b, src, ss);
}
static uint8_t rl_d(
  const uint8_t * src,
  int             ss)
{
  return average_up(rl_G, rl_h, src, ss);
}
static uint8_t rl_n(
  const uint8_t * src,
  int             ss)
{
  return average_up(rl_M, rl_h, src, ss);
}

static uint8_t rl_e(
  const uint8_t * src,
  int             ss)
{
  return average_up(rl_b, rl_h, src, ss);
}
static uint8_t rl_g(
  const uint8_t * src,
  int             ss)
{
  return average_up(rl_b, rl_m, src, ss);
}
static uint8_t rl_p(
  const uint8_t * src,
  int             ss)
{
  return average_up(rl_h, rl_s, src, ss);
}
static uint8_t rl_r(
  const uint8_t * src,
  int             ss)
{
  return average_up(rl_m, rl_s, src, ss);
}

static uint8_t rl_f(
  const uint8_t * src,
  int             ss)
{
  return average_up(rl_b, rl_j, src, ss);
}
static uint8_t rl_i(
  const uint8_t * src,
  int             ss)
{
  return average_up(rl_h, rl_j, src, ss);
}
static uint8_t rl_k(
  const uint8_t * src,
  int             ss)
{
  return average_up(rl_m, rl_j, src, ss);
}
static uint8_t rl_q(
  const uint8_t * src,
  int             ss)
{
  return average_up(rl_s, rl_j, src, ss);
}

static int rl_cc(
  const uint8_t * src,
  int             ss)
{
  return rl_h1(src - 2, ss);
}
static int rl_dd(
  const uint8_t * src,
  int             ss)
{
  return rl_h1(src - 1, ss);
}
static int rl_ee(
  const uint8_t * src,
  int             ss)
{
  return rl_h1(src + 2, ss);
}
static int rl_ff(
  const uint8_t * src,
  int             ss)
{
  return rl_h1(src + 3, ss);
}

/* static int rl_aa(const uint8_t *src, int ss) { return rl_b1(src - 2 * ss, ss); } */
/* static int rl_bb(const uint8_t *src, int ss) { return rl_b1(src - 1 * ss, ss); } */
/* static int rl_gg(const uint8_t *src, int ss) { return rl_b1(src + 2 * ss, ss); } */
/* static int rl_hh(const uint8_t *src, int ss) { return rl_b1(src + 3 * ss, ss); } */


static const taa_h264_get_pixel_fn get_luma_pixel_fns[4][4] =
{
  { rl_G, rl_a, rl_b, rl_c, },
  { rl_d, rl_e, rl_f, rl_g, },
  { rl_h, rl_i, rl_j, rl_k, },
  { rl_n, rl_p, rl_q, rl_r, },
};

void taa_h264_get_block_luma_ref (
  const uint8_t * src,
  int             src_stride,
  int             yvec,
  int             xvec,
  uint8_t *       dest_ptr,
  int             dest_stride,
  int             block_height,
  int             block_width)
{
  int dx = xvec & 3;
  int dy = yvec & 3;

  int xint = xvec >> 2;
  int yint = yvec >> 2;
  int xint_ref = xint + PAD_HORZ_Y;
  int yint_ref = yint + PAD_VERT_Y;

  const uint8_t * src_ptr = &src[yint_ref * src_stride + xint_ref];

  for (int y = 0; y < block_height; ++y)
  {
    const uint8_t * src_row  = &src_ptr[y * src_stride];
    uint8_t * dest_row = &dest_ptr[y * dest_stride];
    for (int x = 0; x < block_width; ++x)
    {
      dest_row[x] = get_luma_pixel_fns[dy][dx](&src_row[x], src_stride);
    }
  }
}

void taa_h264_get_block_chroma_ref (
  const uint8_t * full_pels,
  const int       stride,
  uint8_t *       block,                         /* out: predicted block,
                                                  * interpolated if necessary */
  const int       block_size_y,
  const int       block_size_x,
  const int       block_stride,
  int             cypos,
  int             cxpos,
  int16_t         mvy,
  int16_t         mvx)
{
  int yint = cypos + (mvy >> 3);
  int xint = cxpos + (mvx >> 3);
  int yint_ref = yint + PAD_VERT_UV;
  int xint_ref = xint + PAD_HORZ_UV;

  int yfrac = mvy & 7;
  int xfrac = mvx & 7;
  const uint8_t * ref_ptr = &full_pels[yint_ref * stride + xint_ref];

  if (yfrac == 0 && xfrac == 0)
  {
    /* TODO: Remove separate branch? */
    for (int i = 0; i < block_size_y; i++)
    {
#pragma novector
      for (int j = 0; j < block_size_x; j++)
      {
        block[i * block_stride + j] = ref_ptr[i * stride + j];
      }
    }
  }
  else
  {

    int coef_a = (8 - xfrac) * (8 - yfrac);
    int coef_b = (xfrac    ) * (8 - yfrac);
    int coef_c = (8 - xfrac) * (yfrac    );
    int coef_d = (xfrac    ) * (yfrac    );

    for (int i = 0; i < block_size_y; i++)
    {
      for (int j = 0; j < block_size_x; j++)
      {
        block[i * block_stride + j] = (uint8_t)(
          (coef_a * ref_ptr[i       * stride + j    ] +
           coef_b * ref_ptr[i       * stride + j + 1] +
           coef_c * ref_ptr[(i + 1) * stride + j    ] +
           coef_d * ref_ptr[(i + 1) * stride + j + 1] + 32) >> 6);
      }
    }
  }
}

#pragma warning (default:981)
