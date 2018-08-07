#ifndef _ENC_SEI_H_
#define _ENC_SEI_H_

#include "enc_typedefs.h"

int taa_h264_write_user_data_sei (
  bitwriter_t *   w,
  const uint8_t * data,
  int        data_size);

int taa_h264_write_ref_pic_marking_sei (
  bitwriter_t * w,
  ref_meta_t ** repeat_list,
  int           repeat_count,
  int           temporal_id);

#endif
