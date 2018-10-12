#include "enc_writebits.h"
#include "com_error.h"
#include "com_compatibility.h"

#include <stdlib.h>

#define MAX_ZERO_BYTES 2
#define CODE_BUF_SIZE 256

struct bitwriter_s
{
  uint32_t code_buf_base [CODE_BUF_SIZE];
  uint32_t * code_buf_ptr;
  uint8_t * bytebuf;
  int bytecount;
  uint8_t * latest_nalu_start;
  int bitbuf;
  int bitrest;
  int skip_escape;
  int saved_bytecount;
  int saved_bitbuf;
  int saved_bitrest;
};


/* Adds the RBSP trailing bits to the buffer. Should be performed in the
 * preprocessing step after all codes are flushed to the buffer. */
static unsigned taa_h264_write_rbsp_trailing_bits (
  uint8_t * buf,
  unsigned  len,
  int       bitrest)
{
  int mod8 = bitrest % 8;
  bool new_byte = mod8 == 0;

  if (new_byte)
    buf[len++] = 0x80;
  else
    buf[len - 1] = (uint8_t)(buf[len - 1] | (0x1 << (mod8 - 1)));

  return len;
}


/* Performs emulation prevention on buffer, i.e. escpapes illegal byte
* sequence in buf. Returns number of bytes in buf after processing. */
static int taa_h264_escape_byte_sequnce (
  uint8_t * buf,
  int       len,
  int       skip)
{
  /* Borrowed from langley codec */

  int count = 0;
  int num = 0;

  //TODO-Bo:
  // This function is not efficient. It must be optimized.

  /* 1st pass, check if any bytes needs to be escaped */
  for(int i = skip; i < len; i++)
  {
    uint8_t b = buf[i];

    if(count == MAX_ZERO_BYTES && !(b & 0xfc))
    {
      num++;
      count = 0;
    }

    if(b == 0x00)
      count++;
    else
      count = 0;
  }

  if (num == 0)
    return len;

  /* 2nd pass, escape illegal sequences */
  count = 0;
  for(int i = skip; i < len; i++)
  {
    uint8_t b = buf[i];

    if(count == MAX_ZERO_BYTES && !(b & 0xfc))
    {
      for(int j = len - 1; j >= i; j = j - 1)
        buf[j + 1] = buf[j];

      buf[i] = 0x03;
      count = 0;
      len++;
      continue;
    }

    if(b == 0x00)
      count++;
    else
      count = 0;
  }

  return len;
}

static void taa_h264_reset_bit_buffer (
  bitwriter_t * w)
{
  w->bitbuf = 0;
  w->bitrest = 32;
}

static int addCabacZeroWords(uint8_t * buf, int stuffing_bytes)
{
	int i;
	for (i = 0; i < stuffing_bytes; i += 3)
	{
		*buf++ = 0x00; // CABAC zero word
		*buf++ = 0x00;
		*buf++ = 0x03;
	}
	return i;
}


int  cabacZeroWords_need_added(int pic_bin_count, int pic_size_in_mbs, int bytes_in_picture)
{

	int stuffing_bytes = 0;
	int i = 0;

	int RawMbBits = 256 * 8 + 2 * 8 * 8 * 8;
	int min_num_bytes = ((96 * pic_bin_count) - (RawMbBits * (int)pic_size_in_mbs * 3) + 1023) / 1024;

	if (min_num_bytes > bytes_in_picture)
	{

		stuffing_bytes = min_num_bytes - bytes_in_picture;
		//printf("Inserting %d/%d cabac_zero_word syntax elements/bytes (Clause 7.4.2.10)\n", ((stuffing_bytes + 2) / 3), stuffing_bytes);
		return stuffing_bytes;
		//for (i = 0; i < stuffing_bytes; i += 3)
		//{
		//	*buf++ = 0x00; // CABAC zero word
		//	*buf++ = 0x00;
		//	*buf++ = 0x03;
		//}
		//cur_stats->bit_use_stuffing_bits[p_Vid->type] += (i << 3);
		//nalu->len += i;
	}

	return 0;
}


/* Post processes the output buffer. */
static int taa_h264_postprocess_output_buffer (
  bitwriter_t * w, int pic_bin_count, int frame_size, int pic_size_in_mbs)
{
  int latest_nalu_size = (w->bytebuf + w->bytecount) - w->latest_nalu_start;
  latest_nalu_size = taa_h264_write_rbsp_trailing_bits (w->latest_nalu_start, latest_nalu_size, w->bitrest);
  latest_nalu_size = taa_h264_escape_byte_sequnce (w->latest_nalu_start, latest_nalu_size, w->skip_escape);

 if (pic_bin_count != 0)
 {
	 int stuffing_bytes = 0, latest_frame_size = frame_size;
	 latest_frame_size += latest_nalu_size;
	 stuffing_bytes = cabacZeroWords_need_added(pic_bin_count, pic_size_in_mbs, latest_frame_size);
	 if (stuffing_bytes)
	 {
		 assert(0);
		 latest_nalu_size += addCabacZeroWords((w->latest_nalu_start + latest_nalu_size), stuffing_bytes);
	 }
 }
  w->bytecount = (w->latest_nalu_start + latest_nalu_size) - w->bytebuf;

  taa_h264_reset_bit_buffer (w);

  return latest_nalu_size;
}

static void taa_h264_send_output_buffer (
  bitwriter_t * w,
  nal_cb_t      func,
  unsigned int  longterm_idx,
  unsigned int  frame_num,
  bool          last_nalu_in_frame,
  void *        context,
  unsigned      max_packet_size,
  unsigned *    max_nalu_size,
  unsigned *    current_nal_size,
  int pic_bin_count, int frame_size, int pic_size_in_mbs)
{
  taa_h264_flush_code_buffer (w);
  int latest_nalu_size = taa_h264_postprocess_output_buffer (w, pic_bin_count, frame_size, pic_size_in_mbs);
  bool aggregate = func (w->latest_nalu_start,
                         latest_nalu_size,
                         w->bytebuf,
                         w->bytecount,
                         longterm_idx,
                         frame_num,
                         (int)last_nalu_in_frame,
                         context);

  *current_nal_size = w->bytecount;

  // Make writer ready for next nalu
  taa_h264_reset_writer (w, w->bytebuf, aggregate);
  if (aggregate)
    *max_nalu_size -= latest_nalu_size;
  else
    *max_nalu_size = max_packet_size;
}


#ifdef UNIT_TEST
static
#endif
unsigned taa_h264_send_slice (
  bitwriter_t * w,
  nal_cb_t      func,
  unsigned int  longterm_idx,
  unsigned int  frame_num,
  bool          last_nalu_in_frame,
  void *        context,
  unsigned      max_packet_size,
  unsigned *    max_nalu_size,
  unsigned *    current_nal_size,
  int pic_bin_count, int frame_size, int pic_size_in_mbs)
{
  taa_h264_send_output_buffer (
    w, func, longterm_idx, frame_num, last_nalu_in_frame, context,
    max_packet_size, max_nalu_size, current_nal_size, pic_bin_count, frame_size, pic_size_in_mbs);
  return current_nal_size;
}


#ifdef UNIT_TEST
static
#endif
unsigned taa_h264_send_nonslice (
  bitwriter_t * w,
  nal_cb_t      func,
  void *        context,
  unsigned      max_packet_size,
  unsigned *    max_nalu_size)
{
  unsigned current_nal_size;
  taa_h264_send_output_buffer (w, func, 0, 0, false, context, max_packet_size, max_nalu_size, &current_nal_size, 0, 0, 0);
  return current_nal_size;
}

#ifdef UNIT_TEST
static
#endif
void taa_h264_flush_code_buffer (
  bitwriter_t * w)
{
  static const unsigned mask[33] = {
    0x00000000, 0x00000001, 0x00000003, 0x00000007,
    0x0000000f, 0x0000001f, 0x0000003f, 0x0000007f,
    0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff,
    0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff,
    0x0000ffff, 0x0001ffff, 0x0003ffff, 0x0007ffff,
    0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff,
    0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff,
    0x0fffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff,
    0xffffffff
  };

  int num_codes = w->code_buf_ptr - w->code_buf_base;
  int bitrest = w->bitrest;
  uint32_t bitbuf = w->bitbuf;
  int bytecount = w->bytecount;
  uint8_t * bytebuf = w->bytebuf;

  TAA_H264_DEBUG_ASSERT (num_codes <= CODE_BUF_SIZE);

  /* Adjust for partly flushed bitbuf */
  if (bitrest != 32)
    bytecount -= (32 + 7 - bitrest) / 8;

  for (int i = 0; i < num_codes; i++)
  {
    uint32_t code_and_len = w->code_buf_base[i];
    int code = code_and_len >> 16;
    int len = code_and_len & 0xFFFF;

    if (len <= bitrest)
    {
      bitbuf |= (code & mask[len]) << (bitrest - len);
      bitrest -= len;
    }
    else
    {
      int rest = len - bitrest;
      bitbuf |= (code >> rest) & mask [len - rest];
      bytebuf[bytecount++] = (uint8_t)((bitbuf >> 24) & 0xff);
      bytebuf[bytecount++] = (uint8_t)((bitbuf >> 16) & 0xff);
      bytebuf[bytecount++] = (uint8_t)((bitbuf >>  8) & 0xff);
      bytebuf[bytecount++] = (uint8_t)((bitbuf >>  0) & 0xff);
      bitbuf = (code & mask[rest]) << (32 - rest);
      bitrest = 32 - rest;
    }
  }

  /* Flush the rest of bits in bitbuf without updating bitrest */
  if (bitrest != 32)
  {
    int bytes_minus1 = (32 - bitrest - 1) / 8;
    for (int i = 24; i >= 24 - bytes_minus1 * 8; i -= 8)
      bytebuf [bytecount++] = (uint8_t)((bitbuf >> i) & 0xff);
  }

  w->bitrest = bitrest;
  w->bitbuf = bitbuf;
  w->bytecount = bytecount;
  w->code_buf_ptr = w->code_buf_base;
}


#ifdef UNIT_TEST
static
#endif
void taa_h264_flush_code_buffer_if_needed (
  bitwriter_t * w,
  int           need)
{
  int num_codes = w->code_buf_ptr - w->code_buf_base;
  if (num_codes + need > CODE_BUF_SIZE)
  {
    taa_h264_flush_code_buffer (w);
  }
}


#ifdef UNIT_TEST
static
#endif
int taa_h264_available_codes_before_flush (
  bitwriter_t * w)
{
  int num_codes = w->code_buf_ptr - w->code_buf_base;
  return CODE_BUF_SIZE - num_codes;
}


#ifdef UNIT_TEST
static
#endif
/* This function must be called before code buffer is flushed */
void taa_h264_write_byte_no_check (
  bitwriter_t * w,
  uint8_t       byte)
{
  TAA_H264_DEBUG_ASSERT (w->bytecount == w->skip_escape);

  w->bytebuf [w->bytecount++] = byte;
  w->skip_escape++;
}


#ifdef UNIT_TEST
static
#endif
/* Save the state of the writer so we can return to this state later. */
void taa_h264_save_writer_state (
  bitwriter_t * w)
{
  /* We don't save the code buffer, just ensure that it's empty. */
  taa_h264_flush_code_buffer (w);
  w->saved_bytecount = w->bytecount;
  w->saved_bitrest = w->bitrest;
  w->saved_bitbuf = w->bitbuf;
}


#ifdef UNIT_TEST
static
#endif
/* Load the writer state that was previously saved */
void taa_h264_load_writer_state (
  bitwriter_t * w)
{
  w->code_buf_ptr = w->code_buf_base;
  w->bytecount = w->saved_bytecount;
  w->bitrest = w->saved_bitrest;
  w->bitbuf = w->saved_bitbuf;

  int mask = ((0xFF << (w->bitrest % 8)) & 0xFF);
  w->bytebuf[w->bytecount - 1] = (uint8_t)(w->bytebuf[w->bytecount - 1] & mask);
}


#ifdef UNIT_TEST
static
#endif
unsigned taa_h264_write_bits (
  bitwriter_t * w,
  unsigned      n,
  uint32_t      val)
{
  TAA_H264_DEBUG_ASSERT ((w->code_buf_ptr - w->code_buf_base < CODE_BUF_SIZE) &&
                         (w->code_buf_ptr >= w->code_buf_base));
  *w->code_buf_ptr = (val << 16) | n;
  w->code_buf_ptr++;

  return n;
}


#ifdef UNIT_TEST
static
#endif
/**
 * Reserve a position in the code buffer for writing a code at a later
 * point. Returns the pointer that is to be passed into a writing
 * function.. */
uint32_t * taa_h264_reserve_code (bitwriter_t * w)
{
  *w->code_buf_ptr = (0 << 16) | 0;
  w->code_buf_ptr++;
  return &w->code_buf_ptr[-1];;
}

#ifdef UNIT_TEST
static
#endif
void taa_h264_write_reserved_code (
  uint32_t * code_buf_ptr,
  unsigned   n,
  uint32_t   val)
{
  *code_buf_ptr = (val << 16) | n;
}


#ifdef UNIT_TEST
static
#endif
void taa_h264_reset_writer (
  bitwriter_t * w,
  uint8_t *     bytebuf,
  bool          aggregate)
{
  if (!aggregate)
  {
    // Next NALU is not added to the existing NALUs. Start from the beginning.
    w->bytebuf = bytebuf;
    w->bytecount = 0;
  }
  w->code_buf_ptr = w->code_buf_base;
  w->latest_nalu_start = w->bytebuf + w->bytecount;
  w->bitrest = 32;
  w->bitbuf = 0;
  w->skip_escape = 0;

  w->saved_bytecount = w->bytecount;
  w->saved_bitrest = w->bitrest;
  w->saved_bitbuf = w->bitbuf;
}

int taa_h264_bitwriter_get_bitrest(bitwriter_t *w)
{
	return w->bitrest;
}

#ifdef UNIT_TEST
static
#endif
bitwriter_t * taa_h264_bitwriter_create (
  void)
{
  return TAA_H264_MALLOC (sizeof (bitwriter_t));
}


#ifdef UNIT_TEST
static
#endif
void taa_h264_bitwriter_delete (
  bitwriter_t * w)
{
  TAA_H264_FREE (w);
}


#ifdef UNIT_TEST
static
#endif
/* Functions for help with testing. */
void taa_h264_code_buffer_to_string_ (
  const bitwriter_t * w,
  char                bitstring[],
  int                 maxlen)
{
  int pos = 0;
  int num_codes = w->code_buf_ptr - w->code_buf_base;

  for (int i = 0; i < num_codes; i++)
  {
    int codeword = w->code_buf_base[i] >> 16;
    int codelen = w->code_buf_base[i] & 0xFFFF;
    for (int j = codelen - 1; j >= 0 && pos < maxlen; j--)
      bitstring[pos++] = (uint8_t)(((codeword >> j) & 0x1) ? '1' : '0');
  }
  bitstring[pos] = '\0';
}
