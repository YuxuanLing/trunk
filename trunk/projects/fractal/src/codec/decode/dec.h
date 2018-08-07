#ifndef _TNTMDEC_H_
#define _TNTMDEC_H_

#include <stdio.h>
#include "decoder.h"
#include "dec_readbits.h"

bool taa_h264_process_nalu (
  decoder_t * decoder,
  uint8_t *   input_buffer,
  unsigned    input_buffer_size);

bool taa_h264_dec_init (
  decoder_t * decoder,
  int         width,
  int         height,
  int         num_ref_frames);

void taa_h264_dec_free (
  decoder_t * decoder);
#endif
