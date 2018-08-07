#ifndef _MBUTILS_H_
#define _MBUTILS_H_

#include "com_common.h"
#include "com_intmath.h"

#define MB_NONE_          0
#define MB_LEFT_          (0x1 << 0)
#define MB_UP_            (0x1 << 1)
#define MB_UPLEFT_        (0x1 << 2)
#define MB_UPRIGHT_       (0x1 << 3)
#define MB_LEFT_INTRA_    (0x1 << 4)
#define MB_UP_INTRA_      (0x1 << 5)
#define MB_UPLEFT_INTRA_  (0x1 << 6)
#define MB_UPRIGHT_INTRA_ (0x1 << 7)
#define MB_TYPE_INTRA_    (0x1 << 8)
#define MB_LEFT_INSIDE_   (0x1 << 9)
#define MB_UP_INSIDE_     (0x1 << 10)
#define MB_RIGHT_INSIDE_  (0x1 << 11)
#define MB_BOTTOM_INSIDE_ (0x1 << 12)
#define MB_DECODED_       (0x1 << 15)

#define MB_ALL_AVAIL (   \
    MB_LEFT_           | \
    MB_UP_             | \
    MB_UPLEFT_         | \
    MB_UPRIGHT_        | \
    MB_LEFT_INTRA_     | \
    MB_UP_INTRA_       | \
    MB_UPLEFT_INTRA_   | \
    MB_UPRIGHT_INTRA_  | \
    MB_LEFT_INSIDE_    | \
    MB_UP_INSIDE_      | \
    MB_RIGHT_INSIDE_   | \
    MB_BOTTOM_INSIDE_  )

#define MB_LEFT_INSIDE(flags)    ((flags) & MB_LEFT_INSIDE_)
#define MB_UP_INSIDE(flags)      ((flags) & MB_UP_INSIDE_)
#define MB_RIGHT_INSIDE(flags)   ((flags) & MB_RIGHT_INSIDE_)
#define MB_BOTTOM_INSIDE(flags)  ((flags) & MB_BOTTOM_INSIDE_)
#define MB_LEFT(flags)           ((flags) & MB_LEFT_)
#define MB_UP(flags)             ((flags) & MB_UP_)
#define MB_UP_LEFT(flags)        ((flags) & MB_UPLEFT_)
#define MB_UP_RIGHT(flags)       ((flags) & MB_UPRIGHT_)
#define MB_LEFT_INTRA(flags)     ((flags) & MB_LEFT_INTRA_)
#define MB_UP_INTRA(flags)       ((flags) & MB_UP_INTRA_)
#define MB_UP_LEFT_INTRA(flags)  ((flags) & MB_UPLEFT_INTRA_)
#define MB_UP_RIGHT_INTRA(flags) ((flags) & MB_UPRIGHT_INTRA_)
#define NEIGHBOR_BLOCK_AVAILABLE(flags, blocknum) (((flags) >> (blocknum)) & 0x1)
#define MB_TYPE_IS_INTRA(type)   ((type) == I_4x4 || (type) == I_16x16 || (type) == I_PCM)
#define MB_IS_DECODED(flags)     ((flags) & MB_DECODED_)
#define MB_SET_DECODED(flags)    ((flags) | MB_DECODED_)
#define MB_UNSET_DECODED(flags)  ((flags) ^ MB_DECODED_)

/**
 * Computes the skip motion vector when all dependent variables are
 * known (input arguments). Returns true if the skip MV is a zero
 * vector. */
static __inline
bool taa_h264_compute_skip_motion_vector (
  const mv_t * pred,
  const mv_t * mv_a,
  const mv_t * mv_b,
  bool         avail_a,
  bool         avail_b,
  mv_t *       skip_mv)
{
  /* Section 8.4.1.1 */

  bool zero_mv = false;
  skip_mv->ref_num = 0;

  if (!avail_a || !avail_b
      || ((mv_a->x | mv_a->y | mv_a->ref_num) == 0)
      || ((mv_b->x | mv_b->y | mv_b->ref_num) == 0))
  {
    skip_mv->x = 0;
    skip_mv->y = 0;
    zero_mv = true;
  }
  else
  {
    skip_mv->x = pred->x;
    skip_mv->y = pred->y;
  }

  return zero_mv;
}



static __inline
void taa_h264_set_mb_availability (
  int              mbx,
  int              mby,
  int              mbxmax,
  int              mbymax,
  int              first_mb_in_slice,
  mbinfo_store_t * mbinfo_stored,
  bool             constrained_intra_pred,
  int *            flags)
{
  int avail = 0;
  int mbpos = mby * mbxmax + mbx;

  bool left    = (mbpos - 1) >= first_mb_in_slice;
  bool up      = (mbpos - mbxmax) >= first_mb_in_slice;
  bool upright = (mbpos - mbxmax + 1) >= first_mb_in_slice;
  bool upleft  = (mbpos - mbxmax - 1) >= first_mb_in_slice;

  left    &= (mbx != 0);
  upleft  &= (mbx != 0);
  upright &= (mbx + 1 != mbxmax);

  avail |= left ? MB_LEFT_ : 0;
  avail |= up ? MB_UP_ : 0;
  avail |= upright ? MB_UPRIGHT_ : 0;
  avail |= upleft ? MB_UPLEFT_ : 0;

  /* Availability for intra encoding is handles separately since constrained
   * intra prediction may give different availability than normal.*/
  bool left_intra = left;
  bool up_intra = up;
  bool upleft_intra = upleft;
  bool upright_intra = upright;

  if (constrained_intra_pred)
  {
    left_intra = left && (mbinfo_stored[-1].flags & MB_TYPE_INTRA_);
    up_intra = up && (mbinfo_stored[-mbxmax].flags & MB_TYPE_INTRA_);
    upleft_intra = upleft && (mbinfo_stored[-mbxmax - 1].flags & MB_TYPE_INTRA_);
    upright_intra = upright && (mbinfo_stored[-mbxmax + 1].flags & MB_TYPE_INTRA_);
  }

  avail |= left_intra ? MB_LEFT_INTRA_ : 0;
  avail |= up_intra ? MB_UP_INTRA_ : 0;
  avail |= upright_intra ? MB_UPRIGHT_INTRA_ : 0;
  avail |= upleft_intra ? MB_UPLEFT_INTRA_ : 0;

  bool left_inside   = mbx > 0;
  bool up_inside     = mby > 0;
  bool rigth_inside  = mbx < (mbxmax - 1);
  bool bottom_inside = mby < (mbymax - 1);

  avail |= left_inside ? MB_LEFT_INSIDE_ : 0;
  avail |= up_inside ? MB_UP_INSIDE_ : 0;
  avail |= rigth_inside ? MB_RIGHT_INSIDE_ : 0;
  avail |= bottom_inside ? MB_BOTTOM_INSIDE_ : 0;

  *flags = avail;
}


/* Setting availability flags for all 4x4 blocks (raster order). Block
 * number 0 is bit 0 (least significant bit). */
static __inline
void taa_h264_set_4x4_blocks_neighbor_flags (
  bool  mb_left,
  bool  mb_up,
  bool  mb_upright,
  bool  mb_upleft,
  int * left_flags,
  int * up_flags,
  int * upright_flags,
  int * upleft_flags)
{

  *left_flags = 0xffff;
  *up_flags = 0xffff;
  *upright_flags = mb_upright ? 0x575f : 0x5757;
  *upleft_flags = mb_upleft ? 0xffff : 0xfffe;
  if (!mb_up)
  {
    *up_flags &= 0xfff0;
    *upright_flags &= 0xfff8;
    *upleft_flags &= 0xfff1;
  }
  if (!mb_left)
  {
    *left_flags &= 0xeeee;
    *upleft_flags &= 0xeeef;
  }
}

static __inline
void taa_h264_store_mbinfo (
  const int        mbxmax,
  const int        mbx,
  const int        mby,
  const mbtype_t   mbtype,
  const int        flags,
  const uint16_t   luma_cbp,
  const int        qp_in,
  const uint8_t    deblock_disable,
  const uint8_t    filter_offset_a,
  const uint8_t    filter_offset_b,
  const mv_t *     motion_vectors,
#ifdef TAA_SAVE_264_MEINFO
  const unsigned   num_bits_current_mb,
#endif
  mbinfo_store_t * mb)
{
  mb->skip = (uint8_t) (mbtype == P_SKIP);
  mb->mbtype = mbtype;
  const bool left_avail = MB_LEFT (flags);
  const bool top_avail = MB_UP (flags);
  if (left_avail && top_avail)
  {
    const mbinfo_store_t * mb_left = mb - 1;
    const mbinfo_store_t * mb_top =  mb - mbxmax;
    if (mb->skip & mb_left->skip & mb_top->skip)
    {
      const mv_t * mv_curr = motion_vectors;
      const mv_t * mv_left = motion_vectors - NUM_4x4_BLOCKS_Y;
      const mv_t * mv_up   = motion_vectors - NUM_4x4_BLOCKS_Y * mbxmax;
      if ((abs(mv_curr->x - mv_left->x) < 4) & (abs(mv_curr->y - mv_left->y) < 4)
          && (abs(mv_curr->x - mv_up->x) < 4) & (abs(mv_curr->y - mv_up->y)  < 4))
      {
        mb->skip = 0x3;
        mb->flags = flags;
        mb->cbp_blk = 0;
        mb->qp = (uint8_t) qp_in;
        return;
      }
    }
  }

  uint16_t cbp_blk = luma_cbp;
  int qp = qp_in;
  int new_flags = flags;

  if (MB_TYPE_IS_INTRA (mbtype))
    new_flags |= MB_TYPE_INTRA_;

  /* HACK: TODO: Do proper initialization of ycbp for skip mbs. */
  if (mbtype == P_SKIP)
    cbp_blk = 0;

  if (deblock_disable == 0)
  {
    /* Only disable out picture boundaries */
    if (mbx > 0)
      new_flags |= MB_LEFT_;
    if (mby > 0)
      new_flags |= MB_UP_;
  }
  else if (deblock_disable == 1)
  {
    /* Disable for whole slice */
    /* Reset the availability flags, not the rest */
    new_flags &= ~MB_ALL_AVAIL;
    qp = 0;
    cbp_blk = 0;
  }
  else
  {
    /* deblock_disable == 2 */
    /* Disable at slice boundaries */
  }

  mb->flags = new_flags;
  mb->qp = (uint8_t) qp;
  mb->cbp_blk = (uint16_t) cbp_blk;

  /* mb->deblock_disable = deblock_disable; */
  mb->filter_offset_a = filter_offset_a;
  mb->filter_offset_b = filter_offset_b;


  //
#ifdef TAA_SAVE_264_MEINFO
  mb->consumed_bits_num = num_bits_current_mb;
#endif
}

#endif
