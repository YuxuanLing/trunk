#ifndef TEST_COM_HELPERS_H
#define TEST_COM_HELPERS_H

#include "com_typedefs.h"

void helper_dpb_init (
  decoded_picture_buffer_t * dpb,
  int max_ref_frames,
  int max_frame_num);

void helper_dpb_add_picture (
  decoded_picture_buffer_t * dpb,
  const decoded_picture_t *  pic);

#endif