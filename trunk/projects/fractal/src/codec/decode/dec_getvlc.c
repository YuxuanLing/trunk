#include "dec_getvlc.h"
#include "dec_tables_cavlc.h"
#include "com_intmath.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>


int taa_h264_get_uvlc (
  bitreader_t * reader)
{
  unsigned int lead = 0;
  unsigned int cn = 0;
  int max_bits_left = 32;

  // count down to make sure we don't have an infinite loop on invalid streams
  while (max_bits_left--)
  {
    if (!taa_h264_show_bits (reader, 1))
    {
      lead++;
      taa_h264_advance_bits (reader, 1);
    }
    else
    {
      cn = taa_h264_read_bits (reader, lead + 1) - 1;
      max_bits_left = 0;
    }
  }
  return cn;
}


int taa_h264_get_level(
  bitreader_t * reader)
{
  int code, sign, coeff;

  code = taa_h264_get_uvlc(reader);
  if (code % 2)
  {
    sign = 0;
    code += 1;
  }
  else
  {
    sign = 1;
  }
  coeff = code >> 1;
  if (sign)
    coeff = -coeff;
  return coeff;
}


int taa_h264_inverse_cbp_code (
  int  val,
  bool inter)
{
  return inverse_cbp_code_tab[val][inter ? 1 : 0];
}


int taa_h264_inverse_cbp_code_i16 (
  int i16mode_num)
{
  int cbpy = (i16mode_num < 12) ? 0 : 0x0F;
  if (cbpy)
    i16mode_num -= 12;
  int cbpuv = (i16mode_num / 4);

  return (cbpuv << 4) | cbpy;
}


static int taa_h264_leading_zeros_32 (
  uint32_t a)
{
  int i = 0;
  uint32_t mask = 0x80000000;

  while((a & mask) == 0)
  {
    mask >>= 1;
    i++;
    if(i == 32)
      return(32);
  }
  return i;
}

/* gives you the next n bits from bit position pos */
static uint32_t taa_h264_next_bits_32 (
  uint32_t bits,
  int      pos,
  int      n)
{
  return (bits << pos) >> (32 - n);
}



static void taa_h264_get_coeff_token (
  bitreader_t * reader,
  int           vlc_num,
  int *         total_coeffs,
  int *         trailing_ones)
{
  if (vlc_num == 3)
  {
    /* Fixed length code */
    int code = taa_h264_read_bits (reader, 6);
    if (code == 0x3)
    {
      /* Special case for 0 coeff case */
      *trailing_ones = 0;
      *total_coeffs = 0;
    }
    else
    {
      *trailing_ones = code & 0x3;
      *total_coeffs = (code >> 2) + 1;
    }
  }
  else
  {
    /*
     * No code is more than 16 bits, but need to get minimum 18 bits since
     * a code may have up to 14 leading zeros and we read the three next
     * bits after the first '1'. Show 32 is more efficient?
     */
    uint32_t bits =  (uint32_t) taa_h264_show_bits (reader, 32);
    int lead_zeros = taa_h264_leading_zeros_32 (bits);
    int next3 = taa_h264_next_bits_32 (bits, lead_zeros + 1, 3);
    int index = lead_zeros * 8 + next3;

    /* Value is 16 bits on format:
     * [ trailing ones (2)  | num coeffs (5) | nothing (4) | code length (5) ]
     */
    uint16_t val =  inverse_coeff_token_tab[vlc_num * 128 + index];
    *trailing_ones = val >> 14;
    *total_coeffs = (val >> 9) & 0x1f;
    int codelen = val & 0x1f;

    taa_h264_advance_bits (reader, codelen);
  }
}


static void taa_h264_get_coeff_token_chroma_dc (
  bitreader_t * reader,
  int *         total_coeffs,
  int *         trailing_ones)
{
  /*
   * No code is more than 8 bits, but need to get minimum 10 bits since
   * a code may have up to 6 leading zeros and we read the three next
   * bits after the first '1'. Show 32 is more efficient?
   */
  uint32_t bits =  (uint32_t) taa_h264_show_bits (reader, 32);
  int lead_zeros = taa_h264_leading_zeros_32 (bits);
  /* Caution: some codes are all 0's. */
  lead_zeros = min (9 - 1, lead_zeros);
  int next2 = taa_h264_next_bits_32 (bits, lead_zeros + 1, 2);


  /* value is 16 bits on format
   * [ trailing ones (2)  | num coeffs (5) | nothing (5) | code length (4)] */
  uint16_t val = inverse_coeff_token_chroma_dc_tab [lead_zeros * 4 + next2];
  *trailing_ones = val >> 14;
  *total_coeffs = (val >> 9) & 0x1f;
  int codelen = val & 0xf;

  taa_h264_advance_bits (reader, codelen);
}



static int taa_h264_get_level_vlc_0 (
  bitreader_t * reader)
{
  uint32_t bits = taa_h264_show_bits (reader, 32);
  int lead_zeros = taa_h264_leading_zeros_32 (bits);

  int code;
  int sign;
  int level;
  int len;

  if (lead_zeros < 14)
  {
    sign = lead_zeros & 0x1;
    level = (lead_zeros >> 1) + 1;
    len = lead_zeros + 1;
  }
  else if (lead_zeros == 14)
  {
    len = 19;
    code = bits >> (32 - len);
    sign = code & 0x1;
    level = ((code >> 1) & 0x7) + 8;
  }
  else
  {
    len = 28;
    code = bits >> (32 - len);
    sign = code & 0x1;
    level = ((code >> 1) & 2047) + 16;
  }
  taa_h264_advance_bits (reader, len);
  return (sign ? -level : level);
}



static int taa_h264_get_level_vlc_n (
  bitreader_t * reader,
  int           vlc_num)
{
  uint32_t bits = taa_h264_show_bits (reader, 32);
  int lead_zeros = taa_h264_leading_zeros_32 (bits);
  int shift = vlc_num - 1;

  int code;
  int sign;
  int level;
  int len;

  if (lead_zeros < 15)
  {
    len = lead_zeros + vlc_num + 1;
    code = bits >> (32 - len);
    sign = code & 1;
    level = (lead_zeros << shift) + 1;
    level += (code >> 1) & (~(0xffffffff << shift));
  }
  else
  {
    int escape = 15 << shift;

    len = 28;
    code = bits >> (32 - len);
    sign =  code & 0x1;
    level = ((code >> 1) & 2047) + escape + 1;
  }
  taa_h264_advance_bits (reader, len);

  return (sign ? -level : level);
}


static int taa_h264_get_total_zeros (
  bitreader_t * reader,
  int           total_coeffs)
{
  uint32_t bits = taa_h264_show_bits (reader, 32);

  int lead_zeros = taa_h264_leading_zeros_32 (bits);
  /* Caution: some code are only 0's */
  lead_zeros = min (10 - 1, lead_zeros);
  int next2 = taa_h264_next_bits_32 (bits, lead_zeros + 1, 2);

  /*
   * Index is
   * [(total_coeffs-1) * 40 + lead_zeros * 4 + 'next two bits after first 1']
   *
   * Value is 8 bits on format
   * [ total_zeros (4) | code length (4) ]
   */
  int32_t index = (lead_zeros << 2 ) + next2;
  uint8_t val = inverse_total_zeros_tab[ (total_coeffs - 1) * 40 + index];

  int total_zeros = val >> 4;
  int len = val & 0xf;

  taa_h264_advance_bits (reader, len);

  return total_zeros;
}

/* TODO: Some functions should perhaps be merged into one, taking more input
* parameters. Especially total_zeros functions. Only constants are differ.*/

static int taa_h264_get_total_zeros_chroma_dc (
  bitreader_t * reader,
  int           total_coeffs)
{
  uint32_t bits = taa_h264_show_bits (reader, 32);

  int lead_zeros = taa_h264_leading_zeros_32 (bits);
  /* Caution: some code are only 0's */
  lead_zeros = min (4 - 1, lead_zeros);
  int next1 = taa_h264_next_bits_32 (bits, lead_zeros + 1, 1);


  /*
   * Index is
   * [(total_coeffs-1) * 40 + lead_zeros * 2 + 'next one bit after first 1']
   *
   * Value is 8 bits on format
   * [ total_zeros (4) | code length (4) ]
   */
  int index = lead_zeros * 2 + next1;
  uint8_t val = inverse_total_zeros_chroma_dc_tab[ (total_coeffs - 1) * 8 + index];

  int total_zeros = val >> 4;
  int len = val & 0xf;

  taa_h264_advance_bits (reader, len);

  return total_zeros;
}


static int taa_h264_get_run_before (
  bitreader_t * reader,
  int           zeros_left)
{
  int len, run;
  uint32_t bits = taa_h264_show_bits (reader, 32);
  int zeros_left_minus_1 = (zeros_left < 7) ? zeros_left - 1 : 6;
  int first3 = bits >> (32 - 3);
  uint8_t val = inverse_run_before_tab [zeros_left_minus_1 * 8 + first3];
  if (val == 0)
  {
    int lead_zeros = taa_h264_leading_zeros_32 (bits);
    len = lead_zeros + 1;
    run = lead_zeros + 4;
  }
  else
  {
    len = val & 0xf;
    run = val >> 4;
  }

  taa_h264_advance_bits (reader, len);

  return run;
}

/* Reads the coeffisients from the bitstream and stores them in TCOEFF in zig
 * zag order. */
static int taa_h264_get_coeffs_4x4_cavlc (
  bitreader_t * reader,
  int16_t *     tcoeff,
  blocktype_t   type,
  int           total_coeffs_pred)
{
  int total_coeffs;
  int trailing_ones;
  int level[16];

  int max_num_coeffs;
  int skip_coeffs;

  TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "**********************************\n");

  switch (type)
  {
  case LUMA_4x4:
  case LUMA_16x16_DC:
    max_num_coeffs = 16;
    skip_coeffs = 0;
    break;
  case LUMA_16x16_AC:
  case CHROMA_AC:
    max_num_coeffs = 15;
    skip_coeffs = 1;
    break;
  default:
    /* Not implemented */
    max_num_coeffs = 0;
    skip_coeffs = 0;
    TAA_H264_DEBUG_ASSERT (false);
    break;
  }

  int vlc_num = 0;
  if (total_coeffs_pred < 2)
    vlc_num = 0;
  else if (total_coeffs_pred < 4)
    vlc_num = 1;
  else if (total_coeffs_pred < 8)
    vlc_num = 2;
  else
    vlc_num = 3;

  taa_h264_get_coeff_token (reader, vlc_num, &total_coeffs, &trailing_ones);

  TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "total_coeffs = %d\n", total_coeffs);
  TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "trailing_ones = %d\n", trailing_ones);
  TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "used vlc_num = %d\n", vlc_num);

  TAA_H264_DEBUG_ASSERT (total_coeffs <= max_num_coeffs);
  TAA_H264_DEBUG_ASSERT (trailing_ones < 4);

  /* To be safe in case of corrupt bit stream */
  total_coeffs = min (total_coeffs, max_num_coeffs);
  trailing_ones = min (trailing_ones, 3);

  if (total_coeffs == 0)
  {
    int last_coeff = max_num_coeffs + skip_coeffs;
    for (int x = skip_coeffs; x < last_coeff; x++)
    {
      tcoeff[x] = 0;
      TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "coeff = 0\n");
    }
    return 0;
  }


  int read_coeffs = 0;
  for (; read_coeffs < trailing_ones; read_coeffs++)
  {
    level[read_coeffs] = (taa_h264_read_bits (reader, 1) == 0) ? 1 : -1;
    TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "sign %d\n", level[read_coeffs]);
  }

  if (total_coeffs > trailing_ones) {
    /* Let's handle the first big coeffisient outside the loop since it's a */
    /* bit special. */
    int lev;

    if (total_coeffs > 10 && trailing_ones < 3)
    {
      vlc_num = 1;
      lev = taa_h264_get_level_vlc_n (reader, vlc_num);
    }
    else
    {
      vlc_num = 0;
      lev = taa_h264_get_level_vlc_0 (reader);
    }

    TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "level %d (vlc_num %d)\n", lev, vlc_num);

    if (trailing_ones < 3)
    {
      if (lev < 0)
        lev--;
      else
        lev++;
      TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "level ajusted %d\n", lev);
    }

    level[read_coeffs] = lev;
    read_coeffs++;

    /* Update VLC table */
    if (abs (lev) > vlc_threshold_tab[vlc_num])
      vlc_num++;

    if (abs (lev) > 3)
      vlc_num = 2;

    /* Read the rest of the big coeffs */
    for (; read_coeffs < total_coeffs; read_coeffs++)
    {
      /* vlc_num can't be zero for second coeff */
      level[read_coeffs] = taa_h264_get_level_vlc_n (reader, vlc_num);
      TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "level %d (vlc %d)\n", level[read_coeffs], vlc_num);

      if (abs (level[read_coeffs]) > vlc_threshold_tab[vlc_num])
        vlc_num++;
    }
  }

  if (total_coeffs < max_num_coeffs)
  {
    /* Read total zeros and runs and fill inn zeros */
    int total_zeros = taa_h264_get_total_zeros (reader, total_coeffs);
    int scan_num = total_coeffs + total_zeros - 1 + skip_coeffs;
    int zeros_left = total_zeros;
    TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "total_zeros %d\n", total_zeros);

    TAA_H264_DEBUG_ASSERT (total_zeros + total_coeffs <= max_num_coeffs);

    /* reset all */
    int last_coeff = max_num_coeffs + skip_coeffs;
    for (int x = skip_coeffs; x < last_coeff; x++)
    {
      tcoeff[x] = 0;
    }



    /* Checking for scan_num >= 0 to avoid to avoid accessing negative array
     * index if the stream is corrupt. Do not report the error (since such check
     * would be slow at this level), instead assume the error will trigger
     * another one at header level that will be reported. (Very high probability
     * for this.)
     */

    /* To be safe in case of corrupt bit stream we check scan_num */
    scan_num = min (scan_num, 15);
    for (int x = 0; x < total_coeffs && scan_num >= 0; x++)
    {
      tcoeff[scan_num--] = (int16_t) level[x];
      if (zeros_left && x < total_coeffs - 1)
      {
        /* read run of zeros before next coeff */
        int run_before = taa_h264_get_run_before (reader, zeros_left);
        TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "zeros_left %d   ", zeros_left);
        TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "run %d\n", run_before);

        scan_num -= run_before;
        zeros_left -= run_before;
      }
    }
  }
  else
  {
    /* No zeros, just fill in the coeff matrix */
    int scan_num = max_num_coeffs + skip_coeffs;
    int x = 0;
    while (scan_num != skip_coeffs)
    {
      tcoeff[--scan_num] = (int16_t) level[x++];
    }
  }

#ifdef TAA_H264_TRACE_ENABLED
  for (int x = max_num_coeffs + skip_coeffs - 1; x >= skip_coeffs; x--)
  {
    TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "coeff = %d\n", tcoeff[x]);
  }
#endif

  return total_coeffs;
}


int taa_h264_get_coeffs_luma_cavlc (
  bitreader_t * reader,
  int16_t *     tcoeff,
  int           total_coeffs_pred)
{
  return taa_h264_get_coeffs_4x4_cavlc (
           reader, tcoeff, LUMA_4x4, total_coeffs_pred);
}


int taa_h264_get_coeffs_luma_dc_cavlc (
  bitreader_t * reader,
  int16_t *     dcblock,
  int           total_coeffs_pred)
{
  /* TODO: This is a incostistent since we don't do inverse zig zag in other
   * cavlc functions. */
  static const int inverse_zig_zag_4x4[16][2] = {
    { 0, 0}, { 0, 1}, { 1, 0}, { 2, 0},
    { 1, 1}, { 0, 2}, { 0, 3}, { 1, 2},
    { 2, 1}, { 3, 0}, { 3, 1}, { 2, 2},
    { 1, 3}, { 2, 3}, { 3, 2}, { 3, 3}
  };
  int16_t dccoeffs [16];

  int ret = taa_h264_get_coeffs_4x4_cavlc (
    reader, dccoeffs, LUMA_16x16_DC, total_coeffs_pred);

  /* Need to do the inverse zig zag somewhere, since the dequantization is
   * different for luma DC. */
  for (int k = 0; k < 16; k++)
  {
    int i = inverse_zig_zag_4x4[k][0];
    int j = inverse_zig_zag_4x4[k][1];
    dcblock [i * 4 + j] = dccoeffs[k];
  }

  return ret;
}

int taa_h264_get_coeffs_luma_ac_cavlc (
  bitreader_t * reader,
  int16_t *     tcoeff,
  int           total_coeffs_pred)
{
  return taa_h264_get_coeffs_4x4_cavlc (
           reader, tcoeff, LUMA_16x16_AC, total_coeffs_pred);
}


int taa_h264_get_coeffs_chroma_ac_cavlc (
  bitreader_t * reader,
  int16_t *     tcoeff,
  int           total_coeffs_pred)
{
  return taa_h264_get_coeffs_4x4_cavlc (
           reader, tcoeff, CHROMA_AC, total_coeffs_pred);
}

int taa_h264_get_coeffs_chroma_dc_cavlc (
  bitreader_t * reader,
  int16_t *     coeffs)
{
  int total_coeffs;
  int trailing_ones;
  int level[4];

  const int max_num_coeffs = 4;

  TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "**********chroma dc ***************\n");

  taa_h264_get_coeff_token_chroma_dc (reader, &total_coeffs, &trailing_ones);

  TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "total_coeffs = %d\n", total_coeffs);
  TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "trailing_ones = %d\n", trailing_ones);

  TAA_H264_DEBUG_ASSERT (total_coeffs <= max_num_coeffs);
  TAA_H264_DEBUG_ASSERT (trailing_ones < 4);

  /* To be safe in case of corrupt bit stream */
  total_coeffs = min (total_coeffs, max_num_coeffs);
  trailing_ones = min (trailing_ones, 3);

  if (total_coeffs == 0)
  {
    for (int x = 0; x < max_num_coeffs; x++)
    {
      coeffs[x] = 0;
      TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "coeff = 0\n");
    }
    return 0;
  }


  int read_coeffs = 0;
  for (; read_coeffs < trailing_ones; read_coeffs++)
  {
    level[read_coeffs] = (taa_h264_read_bits (reader, 1) == 0) ? 1 : -1;
    TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "sign %d\n", level[read_coeffs]);
  }

  int vlc_num;
  if (total_coeffs > trailing_ones) {
    /* Let's handle the first big coeffisient outside the loop since it's a */
    /* bit special. */
    int lev = taa_h264_get_level_vlc_0 (reader);

    TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "level %d\n", lev);

    if (trailing_ones < 3)
    {
      if (lev < 0)
        lev--;
      else
        lev++;
      TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "level ajusted %d\n", lev);
    }
    level[read_coeffs] = lev;
    read_coeffs++;

    /* Update VLC table */
    if (abs (lev) > 3)
      vlc_num = 2;
    else
      vlc_num = 1;

    /* Read the rest of the big coeffs */
    for (; read_coeffs < total_coeffs; read_coeffs++)
    {
      /* vlc_num can't be zero for second coeff */
      level[read_coeffs] = taa_h264_get_level_vlc_n (reader, vlc_num);
      TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "level %d (vlc %d)\n", level[read_coeffs], vlc_num);

      if (abs (level[read_coeffs]) > vlc_threshold_tab[vlc_num])
        vlc_num++;
    }
  }

  if (total_coeffs < max_num_coeffs)
  {
    /* Read total zeros and runs and fill inn zeros */
    int total_zeros = taa_h264_get_total_zeros_chroma_dc (reader, total_coeffs);

    int scan_num = total_coeffs + total_zeros - 1;
    int zeros_left = total_zeros;

    TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "total_zeros %d\n", total_zeros);

    TAA_H264_DEBUG_ASSERT (total_zeros + total_coeffs <= max_num_coeffs);

    for (int x = 0; x < max_num_coeffs; x++)
      coeffs[x] = 0;

    /* To be safe in case of corrupt bit stream */
    scan_num = min (scan_num, 3);
    for (int x = 0; x < total_coeffs && scan_num >= 0; x++)
    {
      TAA_H264_DEBUG_ASSERT (scan_num >= 0 && scan_num < 4);
      coeffs[scan_num] = (int16_t) level[x];
      scan_num--;

      if (zeros_left && x < total_coeffs - 1)
      {
        /* read run of zeros before next coeff */
        int run_before = taa_h264_get_run_before (reader, zeros_left);
        TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "zeros_left %d   ", zeros_left);
        TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "run %d    ", run_before);

        scan_num -= run_before;
        zeros_left -= run_before;
      }
    }
  }
  else
  {
    /* No zeros, just fill in the coeff matrix */
    int scan_num = max_num_coeffs;
    int x = 0;
    while (scan_num != 0)
    {
      scan_num--;
      coeffs[scan_num] = (int16_t) level[x];
      x++;
    }
  }

#ifdef TAA_H264_TRACE_ENABLED
  for (int x = 3; x >= 0; x--)
  {
    TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "coeff = %d\n", coeffs[x]);
  }
#endif

  return total_coeffs;
}
