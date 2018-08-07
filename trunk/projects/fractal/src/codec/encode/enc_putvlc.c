#include "enc_writebits.h"
#include "enc_putvlc.h"
#include "enc_tables_cavlc.h"
#include "com_mbutils.h"
#include "com_prediction.h"

/* Worst case when considering number of code symbols for a 4x4 block is a
 * sequence like "0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2", which will produce 1
 * coeff_token, 15 levels, 1 tot_zeros and 15 run_befores.
 */
static const int MAX_CAVLC_CODES_4x4 = 32;
static const int MAX_CAVLC_CODES_2x2 = 8;

static unsigned taa_h264_info_len (
  unsigned code)
{
  return code == 0 ? 0 : _bit_scan_reverse (code);
}


unsigned taa_h264_quote_uvlc(
  unsigned v)
{
  unsigned code = v + 1;
  unsigned len = 1 + 2 * taa_h264_info_len (code);
  return len;
}

unsigned taa_h264_put_uvlc (
  bitwriter_t * writer,
  unsigned          v)
{
  unsigned code = v + 1;
  unsigned len = 1 + 2 * taa_h264_info_len (code);

  taa_h264_write_bits (writer, len, code);
  return len;
}

unsigned taa_h264_quote_level (
  int v)
{
  int code = abs (v) * 2;
  if (v > 0)
    code--;
  return taa_h264_quote_uvlc (code);
}

unsigned taa_h264_put_level (
  bitwriter_t * writer,
  int           v)
{
  int code = abs (v) * 2;
  if (v > 0)
    code--;
  return taa_h264_put_uvlc (writer, code);
}

unsigned taa_h264_quote_cbp (
  int cbp_luma,
  int cbp_chroma,
  int inter)
{
  int cbp = cbp_luma | (cbp_chroma << 4);
  return cbp_code_tab[cbp][inter];
}

static unsigned taa_h264_put_coeff_token (
  bitwriter_t * writer,
  int           total_coeffs,
  int           trailing_ones,
  int           vlc_num)
{
  unsigned len;
  unsigned code;
  if (vlc_num == 3)
  {
    /* Fixed length code */
    len = 6;
    if (total_coeffs == 0)
      code = 3;
    else
      code = ((total_coeffs - 1) << 2) | trailing_ones;
  }
  else
  {
    len  = coeff_token_length_tab [vlc_num][trailing_ones][total_coeffs];
    code = coeff_token_code_tab   [vlc_num][trailing_ones][total_coeffs];
  }
  taa_h264_write_bits (writer, len, code);
  return len;
}

static unsigned taa_h264_put_coeff_token_chroma_dc (
  bitwriter_t * writer,
  int           total_coeffs,
  int           trailing_ones)
{
  unsigned len  = coeff_token_chroma_dc_length_tab [trailing_ones][total_coeffs];
  unsigned code = coeff_token_chroma_dc_code_tab   [trailing_ones][total_coeffs];
  taa_h264_write_bits (writer, len, code);
  return len;
}

static unsigned taa_h264_put_level_vlc_0 (
  bitwriter_t * writer,
  unsigned          abs_level,
  int           sign)
{
  unsigned len;
  unsigned code;

  if (abs_level < 8)
  {
    code = 1;
    len = (abs_level << 1) - !sign;
    /* leading zeros maximum 13 */
  }
  else if (abs_level < 16)
  {
    code = (1 << 4) | ((abs_level - 8) << 1) | sign;
    len = 19;
    /* leading zeros are 14 */
  }
  else
  {
    /*  */
    code = (1 << 12) | ((abs_level - 16) << 1) | sign;
    len = 28;
    /* leading zeros are 15*/
  }
  taa_h264_write_bits (writer, len, code);
  return len;
}

static unsigned taa_h264_put_level_vlc_n (
  bitwriter_t * writer,
  unsigned          abs_level,
  int           sign,
  int           vlc_num)
{
  unsigned len;
  unsigned code;

  int shift = vlc_num - 1;
  unsigned escape = 15 << shift;

  abs_level--;
  if (abs_level < escape)
  {
    int suffix = abs_level & (~((0xffffffff) << shift));
    len = (abs_level >> shift) + vlc_num + 1;
    code = (1 << (shift + 1)) | (suffix << 1) | sign;
  }
  else
  {
    len = 28;
    code = (1 << 12) | ((abs_level - escape) << 1) | sign;
    /* leading zeros are 15 */
  }
  taa_h264_write_bits (writer, len, code);
  return len;
}


static unsigned taa_h264_put_total_zeros (
  bitwriter_t * writer,
  int           total_zeros,
  int           total_coeffs)
{
  unsigned len  = total_zeros_length_tab [total_coeffs - 1][total_zeros];
  unsigned code = total_zeros_code_tab   [total_coeffs - 1][total_zeros];
  taa_h264_write_bits (writer, len, code);
  return len;
}


static unsigned taa_h264_put_total_zeros_chroma_dc (
  bitwriter_t * writer,
  int           total_zeros,
  int           total_coeffs)
{
  unsigned len  = total_zeros_chroma_dc_length_tab [total_coeffs - 1][total_zeros];
  unsigned code = total_zeros_chroma_dc_code_tab   [total_coeffs - 1][total_zeros];
  taa_h264_write_bits (writer, len, code);
  return len;
}


/* cavlc for run_before. zeros_left must be greater than 0 */
static unsigned taa_h264_put_run_before (
  bitwriter_t * writer,
  int           run,
  int           zeros_left)
{
  unsigned len;
  unsigned code;
  if (zeros_left < 8)
  {
    len  = run_before_length_tab [zeros_left - 1][run];
    code = run_before_code_tab   [zeros_left - 1][run];
  }
  else
  {
    /* Use the last entry in the table */
    len  = run_before_length_tab [6][run];
    code = run_before_code_tab   [6][run];
  }
  taa_h264_write_bits (writer, len, code);
  return len;
}


static unsigned taa_h264_encode_coeffs_chroma_dc (
  bitwriter_t * writer,
  coeffs2_t *   coeffs2)
{
  const uint8_t * run = coeffs2->run;
  const int8_t * sign = coeffs2->sign;
  const int16_t * level = coeffs2->level;
  const int nnz = coeffs2->num_nonzero;

  int trailing_ones = 0;
  int total_zeros = 0;
  int total_coeffs = 0;
  int num_of_bits = 0;

  TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "*** chroma dc ***\n");

  // Find total zeros (excluding the trailing zeros)
#pragma novector
  for (int i = 0; i < nnz; i++)
    total_zeros += run[i];

  // Find the number of trailing ones
  for (int i = nnz-1; i >= 0 && level[i] == 1 && trailing_ones < 3; i--)
    trailing_ones++;

  total_coeffs = nnz;

  /* Encode coeff_token (the number of coeffisients and trailing
   * ones) */
  num_of_bits += taa_h264_put_coeff_token_chroma_dc (writer, total_coeffs, trailing_ones);
  TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "total_coeffs = %d\n", total_coeffs);
  TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "trailing_ones = %d\n", trailing_ones);
  TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "num_of_bits = %d\n", num_of_bits);

  if (total_coeffs == 0)
    return num_of_bits;

  /* Encode sign of trailing ones with highest frequency first */
  for (int x = 0; x < trailing_ones; x++)
  {
    const int idx = nnz - 1 - x;
    TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "sign %d\n", sign[idx]);
    taa_h264_write_bits (writer, 1, sign[idx]);
    num_of_bits++;
  }
  TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "num_of_bits = %d\n", num_of_bits);


  /* Encode the first level. It's handled a bit different than the rest, so */
  /* do it outside the loop. */
  int vlc_num = 0;
  if (trailing_ones < total_coeffs)
  {
    const int idx = nnz - 1 - trailing_ones;
    int abslev = level[idx];

    TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "level %d  (vlc 0)\n", abslev * (sign[idx] ? -1 : 1));
    if (trailing_ones < 3)
    {
      /* If less than three trailing ones, then the first "high" level can't */
      /* be 1 or -1, so we decrease in order to save bits. */
      abslev--;
      TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "abs level adjusted to %d\n", abslev);
    }
    /* total_coeffs < 10, so always vlc 0 */
    num_of_bits += taa_h264_put_level_vlc_0 (writer, abslev, sign[idx]);
    TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "num_of_bits = %d\n", num_of_bits);

    /* Update VLC table */
    if (level[idx] > 3)
      vlc_num = 2;
    else
      vlc_num = 1;
  }


  /* Encode the rest of the levels */
  for (int idx = nnz - 1 - trailing_ones - 1; idx >= 0; idx--)
  {
    const int abslev = level[idx];

    TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "level %d  (vlc %d)\n", abslev * (sign[idx] ? -1 : 1), vlc_num);
    /* vlc_num is never 0 after the first level */
    num_of_bits += taa_h264_put_level_vlc_n (writer, abslev, sign[idx], vlc_num);
    TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "num_of_bits = %d\n", num_of_bits);

    if (abslev > vlc_threshold_tab[vlc_num])
      vlc_num++;
  }

  /* Encode the number of zero coeffisient */
  TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "total_zeros = %d\n", total_zeros);
  if (total_coeffs < 4)
  {
    num_of_bits += taa_h264_put_total_zeros_chroma_dc (writer, total_zeros, total_coeffs);
  }
  TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "num_of_bits = %d\n", num_of_bits);

  /* Encode zero runs */
  int zeros_left = total_zeros;
  for (int idx = nnz - 1; idx > 0 && zeros_left; idx--)
  {
    TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "zeros left %d - run_before %d\n", zeros_left, run[idx]);
    num_of_bits += taa_h264_put_run_before (writer, run[idx], zeros_left);
    zeros_left -= run[idx];
    TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "num_of_bits = %d\n", num_of_bits);
  }

  return num_of_bits;

}


/* Encodes a 4x4 block with CAVLC. Returns the number of bits written. */
static unsigned taa_h264_encode_coeffs_4x4_cavlc (
  bitwriter_t *     writer,
  const coeffs4_t * coeffs4,
  blocktype_t       type,
  unsigned              total_coeffs_pred,
  unsigned *            total_coeffs_out)
{
  /* See section 7.3.5.3.1 and 9.2 */

  int trailing_ones = 0;
  int total_zeros = 0;
  int total_coeffs = 0;
  int num_of_bits = 0;

  int max_num_coeffs;
  int skip_coeffs;


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

  const uint8_t * run = &coeffs4->run[skip_coeffs];
  const int8_t * sign = &coeffs4->sign[skip_coeffs];
  const int16_t * level = &coeffs4->level[skip_coeffs];
  const int nnz = coeffs4->num_nonzero;


  TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "**********************************\n");
  TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "block type %u\n", type );

  // Find total zeros (excluding the trailing zeros)
#pragma novector
  for (int i = 0; i < nnz; i++)
    total_zeros += run[i];

  // Find the number of trailing ones
  for (int i = nnz-1; i >= 0 && level[i] == 1 && trailing_ones < 3; i--)
    trailing_ones++;

  total_coeffs = nnz;
  *total_coeffs_out = total_coeffs;

  /* Find correct VLC table */
  int vlc_num;
  if (total_coeffs_pred < 2)
    vlc_num = 0;
  else if (total_coeffs_pred < 4)
    vlc_num = 1;
  else if (total_coeffs_pred < 8)
    vlc_num = 2;
  else
    vlc_num = 3;


  /* Encode coeff_token, */
  /* that is number of coeffisients and trailing ones */
  num_of_bits += taa_h264_put_coeff_token (writer, total_coeffs, trailing_ones, vlc_num);
  TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "total_coeffs = %d\n", total_coeffs);
  TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "trailing_ones = %d\n", trailing_ones);
  TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "num_of_bits = %d (vlc_num %d)\n", num_of_bits, vlc_num);

  if (total_coeffs == 0)
    return num_of_bits;

  /* Encode sign of trailing ones with highest frequency first */
  for (int x = 0; x < trailing_ones; x++)
  {
    const int idx = nnz - 1 - x;
    TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "sign %d\n", sign[idx]);
    taa_h264_write_bits (writer, 1, sign[idx]);
    num_of_bits++;
  }
  TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "num_of_bits = %d\n", num_of_bits);


  /* Encode the first level. It's handled a bit different than the rest, so */
  /* do it outside the loop. */
  if (trailing_ones < total_coeffs)
  {
    const int idx = nnz - 1 - trailing_ones;
    int abslev = level[idx];

    if (trailing_ones < 3)
    {
      /* If less than three trailing ones, then the first "high" level can't */
      /* be 1 or -1, so we decrease in order to save bits. */
      abslev--;
      TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "abs level adjusted to %d\n", abslev);
    }

    if (total_coeffs > 10 && trailing_ones < 3)
    {
      vlc_num = 1;
      num_of_bits += taa_h264_put_level_vlc_n (writer, abslev, sign[idx], vlc_num);
    }
    else
    {
      vlc_num = 0;
      num_of_bits += taa_h264_put_level_vlc_0 (writer, abslev, sign[idx]);
    }
    TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "level %d  (vlc %d)\n", level[idx] *(sign[idx] ? -1 : 1), vlc_num);


    /* Update VLC table */
    if (level[idx] > vlc_threshold_tab[vlc_num])
      vlc_num++;

    if (level[idx] > 3)
      vlc_num = 2;
  }

  /* Encode the rest of the levels */
  for (int idx = nnz - 1 - trailing_ones - 1; idx >= 0; idx--)
  {
    const int abslev = level[idx];

    TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "level %d  (vlc %d)\n", abslev * (sign[idx] ? -1 : 1), vlc_num);
    /* vlc_num is never 0 after the first level */
    num_of_bits += taa_h264_put_level_vlc_n (writer, abslev, sign[idx], vlc_num);
    TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "num_of_bits = %d\n", num_of_bits);

    if (abslev > vlc_threshold_tab[vlc_num])
      vlc_num++;
  }

  /* Send the number of zero coeffisient */
  TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "total_zeros = %d\n", total_zeros);
  if (total_coeffs < max_num_coeffs)
  {
    num_of_bits += taa_h264_put_total_zeros (writer, total_zeros, total_coeffs);
  }
  TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "num_of_bits = %d\n", num_of_bits);

  /* Send zero runs */
  int zeros_left = total_zeros;
  for (int idx = nnz - 1; idx > 0 && zeros_left; idx--)
  {
    TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "zeros left %d - run_before %d\n", zeros_left, run[idx]);
    num_of_bits += taa_h264_put_run_before (writer, run[idx], zeros_left);
    zeros_left -= run[idx];
    TAA_H264_TRACE (TAA_H264_TRACE_CAVLC, "num_of_bits = %d\n", num_of_bits);
  }

  return num_of_bits;
}

/* Creates cbp for luma 8x8 blocks out of cbp for 4x4 blocks. Assumes that
 * the cbp of the 4x4 is in the following bit position pattern:
 *
 *  15 14 11 10
 *  13 12  9  8
 *   7  6  3  2
 *   5  4  1  0
 *
 * */
int taa_h264_get_cbp_luma (
  mbinfo_t * mb)
{
  int cbp = 0;
  const int ycbp = mb->mbcoeffs.luma_cbp;

  if (mb->mbtype == I_16x16)
  {
    if (ycbp != 0)
      cbp = 0xFF;
  }
  else
  {
    cbp  = ((ycbp & 0x0033) ? 1 : 0) << 0; // 0x0033 =         00110011
    cbp |= ((ycbp & 0x00CC) ? 1 : 0) << 1; // 0x00CC =         11001100
    cbp |= ((ycbp & 0x3300) ? 1 : 0) << 2; // 0x3300 = 0011001100000000
    cbp |= ((ycbp & 0xCC00) ? 1 : 0) << 3; // 0xCC00 = 1100110000000000
  }
  return cbp;
}


/* Creates cbp for 8x8 chroma based on table 7-12 */
int taa_h264_get_cbp_chroma (
  mbinfo_t * mb)
{
  const int acmask = 0x2;
  const int cr_cbp = mb->mbcoeffs.chroma_cbp;
  int ret = 2;

  if (cr_cbp == 0)
    ret = 0;
  else if ((cr_cbp & acmask) == 0)
    ret = 1;
  return ret;
}


unsigned taa_h264_encode_coeffs_chroma_cavlc (
  mbinfo_t *    mb,
  uint8_t *     num_coeffs,                                     /* pointer to num_coeffs_uv for mb  */
  bitwriter_t * writer,
  int           mode)                               /* Mode to say wether DC/AC is to be coded */
{
  int mb_left = MB_LEFT (mb->avail_flags);
  int mb_top  = MB_UP (mb->avail_flags);

  int bitcount = 0;
  int ncoeffs = 0;
  const int num_dc_coeffs = 4;
  const int num_ac_coeffs_per_block = 15;

  if (mode == 0)
    return bitcount;

  for (int uv = 0; uv < 2; uv++)
  {
    taa_h264_flush_code_buffer_if_needed (writer, MAX_CAVLC_CODES_2x2);

    int len = taa_h264_encode_coeffs_chroma_dc (writer,
                                                &mb->mbcoeffs.chroma_dc[uv]);
    bitcount += len;
    ncoeffs += num_dc_coeffs;
  }

  if (mode == 2)
  {
    int offset_nc = 0;
    int stride_nc = mb->mbxmax * NUM_4x4_BLOCKS_UV * 2;

    for (int uv = 0; uv < 2; uv++)
    {
      for (int k = 0; k < 4; k++)
      {
        unsigned coeffs_out;
        bool first_col = k % 2 == 0;
        bool first_row = k / 2 == 0;
        bool left_avail = mb_left || !first_col;
        bool up_avail = mb_top || !first_row;

        int coeffs_pred = taa_h264_predict_nonzero_coeffs (
          &num_coeffs[offset_nc], stride_nc,
          first_col, first_row, left_avail, up_avail, true);

        taa_h264_flush_code_buffer_if_needed (writer, MAX_CAVLC_CODES_4x4);

        int len = taa_h264_encode_coeffs_4x4_cavlc (writer,
                                                    &mb->mbcoeffs.chroma_ac[uv * 4 + k],
                                                    CHROMA_AC,
                                                    coeffs_pred,
                                                    &coeffs_out);
        num_coeffs[offset_nc] = (uint8_t) coeffs_out;

        bitcount += len;
        offset_nc += 1;
        ncoeffs += num_ac_coeffs_per_block;
      }
    }
  }

  return bitcount;
}


unsigned taa_h264_encode_coeffs_luma_cavlc (
  mbinfo_t *    mb,
  uint8_t *     num_coeffs,                                    /* pointer to num_coeffs for this MB */
  bitwriter_t * writer)
{
  unsigned numbits = 0;
  int offset = 0;
  int cbp = taa_h264_get_cbp_luma (mb);
  int stride_nc = mb->mbxmax * NUM_4x4_BLOCKS_Y;

  int mb_left = MB_LEFT (mb->avail_flags);
  int mb_top  = MB_UP (mb->avail_flags);

  int num_ac_coeffs_per_block = 16;
  blocktype_t blocktype = LUMA_4x4;

  if (mb->mbtype == I_16x16)
  {
    const int num_dc_coeffs = 16;
    num_ac_coeffs_per_block = 15;

    blocktype = LUMA_16x16_DC;

    /* The prediction of number of nonzero coeffs for DC when I_16x16 is
     * the like being the top left "normal" 4x4 block. */
    unsigned coeffs_out_dummy;
    unsigned coeffs_pred = taa_h264_predict_nonzero_coeffs (&num_coeffs[0],
                                                        stride_nc,
                                                        true,
                                                        true,
                                                        mb_left,
                                                        mb_top,
                                                        false);

    taa_h264_flush_code_buffer_if_needed (writer, MAX_CAVLC_CODES_4x4);
    numbits += taa_h264_encode_coeffs_4x4_cavlc (writer,
                                                 &mb->mbcoeffs.luma_dc,
                                                 blocktype,
                                                 coeffs_pred,
                                                 &coeffs_out_dummy);
    offset += num_dc_coeffs;
    blocktype = LUMA_16x16_AC;
  }


  for (int i8x8 = 0; i8x8 < 4; i8x8++)
  {
    int raster_idx = (i8x8 / 2) * 8 + (i8x8 % 2) * 2;
    for (int i4x4 = 0; i4x4 < 4; i4x4++)
    {
      if (cbp & (1 << i8x8))
      {
        bool first_col = raster_idx % 4 == 0;
        bool first_row = raster_idx / 4 == 0;
        bool left_avail = mb_left || !first_col;
        bool up_avail = mb_top || !first_row;

        uint8_t * num_coeffs_ptr = &num_coeffs[raster_idx];

        unsigned coeffs_out;
        unsigned coeffs_pred = taa_h264_predict_nonzero_coeffs (
          num_coeffs_ptr, stride_nc,
          first_col, first_row, left_avail, up_avail, false);

        taa_h264_flush_code_buffer_if_needed (writer, MAX_CAVLC_CODES_4x4);
        numbits += taa_h264_encode_coeffs_4x4_cavlc (writer,
                                                     &mb->mbcoeffs.luma_ac[i8x8 * 4 + i4x4],
                                                     blocktype,
                                                     coeffs_pred,
                                                     &coeffs_out);
        *num_coeffs_ptr = (uint8_t) coeffs_out;
      }
      offset += num_ac_coeffs_per_block;
      raster_idx += (i4x4 == 1) ? 3 : 1;
    }
  }
  return numbits;
}
