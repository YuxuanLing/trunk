#ifndef _RESIZE_H_
#define _RESIZE_H_

#include "taah264stdtypes.h"

void resize (
  const uint8_t * src,
  uint8_t *       dst,
  const int       src_width,
  const int       src_height,
  const int       factor);

#endif
