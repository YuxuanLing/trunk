#include "dec_readbits.h"
#include <stdio.h>

#define MAX_ZERO_BYTES 2

#if 0
static void dump_bitreader (
  bitreader_t * r)
{
  int level = 0xFF;
  uint8_t * b;

  TAA_H264_TRACE (level, "----\n");
  b =  r->bufptr;
  uint32_t tmp = (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
  TAA_H264_TRACE (level, "\tbufptr ptr: %p val: 0x%08x\n",
                  b, tmp);
  TAA_H264_TRACE (level, "\tbitbuf 0x%08x 0x%08x\n", r->bitbuf[0], r->bitbuf[1]);
  TAA_H264_TRACE (level, "\tbitrest = %d\n", r->bitrest);
  TAA_H264_TRACE (level, "----\n");
}
#endif

/* Loads 32 bits in the bitbuffer. If there isn't 4 bytes left in the
 * buffer the bit buffer will be padded with 0 bits. */
static void load_bitbuf (bitreader_t * r)
{
  if (r->bufptr + 3 <= r->bufend)
  {
    r->bitbuf[0] = r->bitbuf[1];
    r->bitbuf[1] = (r->bufptr[0] << 24) | (r->bufptr[1] << 16) | (r->bufptr[2] << 8) | r->bufptr[3];
    r->bufptr += 4;
    r->bitrest += 32;
  }
  else
  {
    // To make sure we don't read outside our memory we copy to a local array
    uint8_t buf[4] = {0, };
    uint8_t * ptr = buf;
    while (r->bufptr <= r->bufend)
      *ptr++ = *r->bufptr++;

    r->bitbuf[0] = r->bitbuf[1];
    r->bitbuf[1] = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
    r->bitrest += 32;
  }
}

/* Preprocess the buffer by removing emulation_prevention_three_byte when
 * present. The given buffer is modified in place. Returns the size of the
 * new buffer. */
static int taa_h264_preprocess_buffer (
  uint8_t * buf,
  unsigned  size)
{
  int read_pos = 0;
  int write_pos = 0;
  int num_zero_bytes = 0;

  while (size--)
  {
    uint8_t byte = buf[read_pos];

    if ((num_zero_bytes == MAX_ZERO_BYTES) && (byte == 0x03))
    {
      /* Current byte is emulation_prevention_three_byte, skip it */
      num_zero_bytes = 0;
	  /* Since size and read_pos move in lockstep fashion with size+read_pos always equal 
	     to intial size, if size is 0 we can be sure that read_pos will be at initial_size, 
		 so buf[read_pos] will be an invalid read when size is 0, hence we need to break. 
		 Also we will encounter this condition only if last 3 bytes of nal are 00 00 03. 
		 Although it makes no difference to break before read_pos++ or after, but in spirit
		 of keeping the semantics correct, read_pos should reflect number of bytes actually 
		 read from buf, hence we break before read_pos++.*/
	  if(size == 0) 
          break;    
      read_pos++;
      byte = buf[read_pos];
      size--;
    }

    if (byte == 0x00)
      num_zero_bytes++;
    else
      num_zero_bytes = 0;

    buf[write_pos] = byte;
    write_pos++;
    read_pos++;
  }

  // In case there are one or more "zero bytes" at the after the RBSP's trailing
  // bits. Not sure if it's allowed but some encoders produce it.
  size = write_pos;
  while (size > 1 && buf[size-1] == 0x0)
    size--;

  return size;
}


void taa_h264_load_buffer (
  bitreader_t * r,
  uint8_t *     buf,
  unsigned      size)
{
  int new_size = taa_h264_preprocess_buffer (buf, size);
  r->bufptr = buf;
  r->bufend = buf + new_size - 1;
  r->bitrest = -32;
  r->bitsleft = new_size * 8;
  load_bitbuf (r);
  load_bitbuf (r);
}

/* Returns the number of whole bytes left in buffer rounded down  */
int taa_h264_reader_bytes_left (
  bitreader_t * r)
{
  return r->bitsleft / 8;
}



bool taa_h264_reader_has_more_data (
  bitreader_t * r)
{
  if (r->bitsleft > 8)
    return true;
  else if (r->bitsleft <= 0)
    // Safety; should never happen on streams with trailing rbsp bit
    return false;

  int bitpos = (8 - r->bitrest % 8) % 8;
  bool only_trail_bits_left = ((*r->bufend << bitpos) & 0xFF) == 0x80;

  return !only_trail_bits_left;
}



void taa_h264_advance_bits (
  bitreader_t * r,
  unsigned      n)
{
  r->bitrest -= n;
  r->bitsleft -= n;
  if (r->bitrest <= 0)
    load_bitbuf (r);
}

uint32_t taa_h264_show_32bits (
  bitreader_t * r)
{
  /* bitrest is <= 32 */
  int k = r->bitrest - 32;
  if (k == 0)
    return r->bitbuf[0];
  else
    return (r->bitbuf[0] << (-k)) | (r->bitbuf[1] >> r->bitrest);
}

uint32_t taa_h264_show_bits (
  bitreader_t * r,
  unsigned      n)
{
  uint32_t tmp = taa_h264_show_32bits (r);
  return (tmp >> (32 - n));
}

uint32_t taa_h264_read_bits (
  bitreader_t * r,
  unsigned      n)
{
  uint32_t tmp = taa_h264_show_bits (r, n);
  taa_h264_advance_bits (r, n);
  return tmp;
}


uint8_t taa_h264_read_byte_aligned (
  bitreader_t * r)
{
  /* This may be done way more efficient if necessary since we reading one
   * aligned byte. Optimizing is postponed since this function currently is
   * only used when decoding PCM, which is very rare for video conferencing.
   */
  return (uint8_t) taa_h264_read_bits (r, 8);
}


/* Advanced to next byte boundary */
void taa_h264_advance_to_byte (
  bitreader_t * r)
{
  taa_h264_advance_bits (r, r->bitrest % 8);
}

void taa_h264_advance_rbsp_trailing_bits (
  bitreader_t * r)
{
  taa_h264_read_bits (r, 1);
  taa_h264_advance_to_byte (r);
}
