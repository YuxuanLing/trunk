#ifndef _INTERPOLATE_REF_H_
#define _INTERPOLATE_REF_H_

#include "taah264stdtypes.h"

/**
 * These functions are slow reference implementations of the interpolation.
 */

void taa_h264_get_block_luma_ref (
  const uint8_t * src,
  int             src_stride,
  int             yvec,
  int             xvec,
  uint8_t *       dest,
  int             dest_stride,
  int             block_height,
  int             block_width);

void taa_h264_get_block_chroma_ref (
  const uint8_t * full_pels,
  const int       stride,
  uint8_t *       block,
  const int       block_size_y,
  const int       block_size_x,
  const int       block_stride,
  int             cypos,
  int             cxpos,
  int16_t         mvy,
  int16_t         mvx);

#endif
