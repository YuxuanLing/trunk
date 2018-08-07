#ifndef _MOTIONSEARCH_H_
#define _MOTIONSEARCH_H_

#include "com_common.h"
#include "enc_typedefs.h"

struct meinfo_s
{
  int xpos;
  int ypos;
  int width;
  int height;

  int best_cost;
  mbtype_t best_mode;
  mvpos best_mv[2];

  int lambda;
  int thr;

  framebuf_t * ref_full;
  framebuf_t * ref_half_diag;
  framebuf_t * ref_half_horz;
  framebuf_t * ref_half_vert;
  int ref_num;
  uint16_t sad8x8[4];
};

meinfo_t * taa_h264_meinfo_create (
  void);

void taa_h264_meinfo_delete (
  meinfo_t * me);


void taa_h264_meinfo_init_frame (
  meinfo_t *   me,
  int          width,
  int          height,
  framebuf_t * ref_full,
  framebuf_t * ref_half_diag,
  framebuf_t * ref_half_horz,
  framebuf_t * ref_half_vert,
  int          ref_num);

uint32_t taa_h264_motion_estimate (
  const int stride,
  meinfo_t *    me,
  const int qp,
  mv_t *        best_mv1,
  mv_t *        best_mv2,
  mbtype_t *    best_mode,
  const mv_t pred,
  const mv_t cand[4],
  const uint8_t mblock_y[256],
  uint8_t recon_y[256],
  uint8_t recon_uv[64]
  );

#endif
