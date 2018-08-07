#ifndef _DEBLOCK_H_
#define _DEBLOCK_H_

#include "com_resolutions.h"
#include "com_intmath.h"
#include "com_common.h"

void taa_h264_deblock_frame (
  framebuf_t *     framebuf_ext,
  mbinfo_store_t * mbinfo_stored,
  mb_deblock_t   * deblock_info,
  mv_t *           motion_vectors,
  int              mbymax,
  int              mbxmax,
  source_res_t     resolution);

void taa_h264_deblock_frame_ssse3 (
  framebuf_t *     framebuf_ext,
  mbinfo_store_t * mbinfo_stored,
  mb_deblock_t   * deblock_info,
  mv_t *           motion_vectors,
  int              mbymax,
  int              mbxmax,
  source_res_t     resolution);

#endif
