#ifndef _DEC_SEI_H_
#define _DEC_SEI_H_

#include "dec_typedefs.h"
#include "com_error.h"

bool taa_h264_read_sei (
  bitreader_t * r,
  decoder_t *   decoder);

#endif
