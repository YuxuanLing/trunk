#include "resize.h"


/**
 * Downscales SRC and puts the results in DST. Resulting resolution is
 * orig / FACTOR.
 */
void resize (
  const uint8_t * src,
  uint8_t *       dst,
  const int       src_width,
  const int       src_height,
  const int       factor)
{
  const int dst_width = src_width / factor;
  const int dst_height = src_height / factor;

  uint8_t * dst_y = dst;
  uint8_t * dst_u = &dst_y[dst_width * dst_height];
  uint8_t * dst_v = &dst_u[dst_width * dst_height / 4];

  const uint8_t * src_y = src;
  const uint8_t * src_u = &src_y[src_width * src_height];
  const uint8_t * src_v = &src_u[src_width * src_height / 4];

  int y;
  int x;

  for (y = 0; y < dst_height; y++)
  {
    for (x = 0; x < dst_width; x++)
    {
      dst_y[y * dst_width + x] =
        src_y[y * factor * src_width + x * factor];
    }
  }

  for (y = 0; y < dst_height/2; y++)
  {
    for (x = 0; x < dst_width/2; x++)
    {
      dst_u[y * dst_width/2 + x] =
        src_u[y * factor * src_width/2 + x * factor];
    }
  }

  for (y = 0; y < dst_height/2; y++)
  {
    for (x = 0; x < dst_width/2; x++)
    {
      dst_v[y * dst_width/2 + x] =
        src_v[y * factor * src_width/2 + x * factor];
    }
  }
}
