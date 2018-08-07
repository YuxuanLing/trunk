/* -*- indent-tabs-mode:nil; c-basic-offset:4 -*- */
#ifndef _ANNEXB_H_
#define _ANNEXB_H_

#include <stdio.h>

typedef struct
{
  unsigned char * buf_start;
  unsigned char * nalu_start;
  size_t buf_size;
  size_t nalu_size;
  FILE * file;
} annexb_context;

void annexb_init (
  annexb_context * c,
  FILE *           file);
void annexb_free (
  annexb_context * c);
unsigned char * extract_nalu (
  annexb_context * c);


#endif
