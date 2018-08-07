#ifndef _INTERPOLATION_H_
#define _INTERPOLATION_H_
#include "taah264stdtypes.h"
#include "com_compatibility.h"
#include <emmintrin.h>

void taa_h264_save_best_chroma_8x8(
  int const          xfrac,
  int const          yfrac,
  int const          cstride,
  uint8_t const *    full_u,
  uint8_t const *    full_v,
  uint8_t *          best_uv
  );

void taa_h264_save_best_chroma_8x4(
  int const          xfrac,
  int const          yfrac,
  int const          cstride,
  uint8_t const *    full_u,
  uint8_t const *    full_v,
  uint8_t *          best_uv
  );

void taa_h264_save_best_chroma_4x8(
  int const          xfrac,
  int const          yfrac,
  int const          cstride,
  uint8_t const *    full_u,
  uint8_t const *    full_v,
  uint8_t *           best_uv
  );

void taa_h264_save_best_16x16(
  const int          stride,
  int const          zfrac,
  uint8_t const *    fpel,
  uint8_t *         best
  );

void taa_h264_save_best_16x8(
  const int          stride,
  int const          zfrac,
  uint8_t const *    fpel,
  uint8_t *          best
  );

void taa_h264_save_best_8x16(
  const int          stride,
  int const          zfrac,
  uint8_t const *    fpel,
  uint8_t *          best
  );

void taa_h264_save_best_8x8(
  const int          stride,
  int const          zfrac,
  uint8_t const *    fpel,
  uint8_t *           best
  );

#endif
