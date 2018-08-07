#ifndef _DEC_GETVLC_H_
#define _DEC_GETVLC_H_

#include "dec_readbits.h"
#include "decoder.h"

#include "com_error.h"

#ifdef TAA_H264_TRACE_ENABLED
# define TAA_H264_READ_U(r, len, str) taa_h264_read_u_trace (r, len, str)
# define TAA_H264_READ_UE(r, str)     taa_h264_read_ue_trace (r, str)
# define TAA_H264_READ_SE(r, str)     taa_h264_read_se_trace (r, str)
#else
# define TAA_H264_READ_U(r, len, str) taa_h264_read_bits (r, len)
# define TAA_H264_READ_UE(r, str)     taa_h264_get_uvlc (r)
# define TAA_H264_READ_SE(r, str)     taa_h264_get_level (r)
#endif


int taa_h264_get_uvlc (
  bitreader_t * r);
int taa_h264_get_level (
  bitreader_t * r);
int taa_h264_inverse_cbp_code (
  int  val,
  bool inter);
int taa_h264_inverse_cbp_code_i16 (
  int i16mode_num);

int taa_h264_get_coeffs_luma_cavlc (
  bitreader_t * r,
  int16_t *     tcoeff,
  int           total_coeffs_pred);
int taa_h264_get_coeffs_luma_dc_cavlc (
  bitreader_t * r,
  int16_t *     tcoeff,
  int           total_coeffs_pred);
int taa_h264_get_coeffs_luma_ac_cavlc (
  bitreader_t * r,
  int16_t *     tcoeff,
  int           total_coeffs_pred);
int taa_h264_get_coeffs_chroma_ac_cavlc (
  bitreader_t * r,
  int16_t *     tcoeff,
  int           total_coeffs_pred);
int taa_h264_get_coeffs_chroma_dc_cavlc (
  bitreader_t * r,
  int16_t *     coeffs);

#ifdef TAA_H264_TRACE_ENABLED

static __inline int taa_h264_read_u_trace (
  bitreader_t * r,
  int           len,
  const char *  str)
{
  int val = taa_h264_read_bits (r, len);
  TAA_H264_TRACE_BITSTREAM (TAA_H264_TRACE_HEADERS, str, val, len);
  return val;
}

static __inline int taa_h264_read_ue_trace (
  bitreader_t * r,
  const char *  str)
{
  int val = taa_h264_get_uvlc (r);
  TAA_H264_TRACE_VALUE (TAA_H264_TRACE_HEADERS, str, val);
  return val;
}

static __inline int taa_h264_read_se_trace (
  bitreader_t * r,
  const char *  str)
{
  int val = taa_h264_get_level (r);
  TAA_H264_TRACE_VALUE (TAA_H264_TRACE_HEADERS, str, val);
  return val;
}
#endif

#endif
