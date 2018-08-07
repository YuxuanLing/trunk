#ifndef _ENC_INIT_MB_H_
#define _ENC_INIT_MB_H_

void taa_h264_init_mb (
  const int            stride,
  const int            cstride,
  const int            quant,
  const int            xpos,
  const int            ypos,
  const int            cxpos,
  const int            cypos,
  const int            mbxmax,
  const int            mbymax,
  const int            height,
  const int            width,
  bool                 intra,
  frameinfo_t *        frameinfo,
  mbinfo_t *           mb,
  int                  mby,
  int                  mbx,
  int                  first_mb_in_slice,
  mv_t *               motion_vectors_curr,
  const mv_t *         motion_vectors_prev,
  fastpos_t *          motion_vectors_curr_16x16,
  meinfo_t *           meinfo,
  const unsigned       coding_options);

void taa_h264_init_mb_ssse3 (
  const int            stride,
  const int            cstride,
  const int            quant,
  const int            xpos,
  const int            ypos,
  const int            cxpos,
  const int            cypos,
  const int            mbxmax,
  const int            mbymax,
  const int            height,
  const int            width,
  bool                 intra,
  frameinfo_t *        frameinfo,
  mbinfo_t *           mb,
  int                  mby,
  int                  mbx,
  int                  first_mb_in_slice,
  mv_t *               motion_vectors_curr,
  const mv_t *         motion_vectors_prev,
  fastpos_t *          motion_vectors_curr_16x16,
  meinfo_t *           meinfo,
  const unsigned       coding_options);

#endif
