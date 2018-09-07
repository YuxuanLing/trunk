#ifndef _ENC_PUTVLC_H_
#define _ENC_PUTVLC_H_

#include "encoder.h"
#include "enc_writebits.h"              /* because of the  functions */


#ifdef TAA_H264_TRACE_ENABLED
# define TAA_H264_WRITE_U(w, val, len, str) taa_h264_write_u_trace (w, val, len, str)
# define TAA_H264_WRITE_UE(w, val, str)     taa_h264_write_ue_trace (w, val, str)
# define TAA_H264_WRITE_SE(w, val, str)     taa_h264_write_se_trace (w, val, str)
#else
# define TAA_H264_WRITE_U(w, val, len, str) taa_h264_write_bits (w, len, val)
# define TAA_H264_WRITE_UE(w, val, str)     taa_h264_put_uvlc (w, val)
# define TAA_H264_WRITE_SE(w, val, str)     taa_h264_put_level (w, val)
#endif

unsigned taa_h264_encode_coeffs_luma_cavlc (
  mbinfo_t *    mb,
  uint8_t *     num_coeffs,
  bitwriter_t * writer);

unsigned taa_h264_encode_coeffs_chroma_cavlc (
  mbinfo_t *    mb,
  uint8_t *     num_coeffs,
  bitwriter_t * writer,
  int           mode);

unsigned taa_h264_quote_uvlc(
  unsigned cn);

unsigned taa_h264_quote_level(
  int level);

unsigned taa_h264_quote_cbp (
  int cbp_luma,
  int cbp_chroma,
  int inter);

unsigned taa_h264_put_uvlc(
  bitwriter_t * writer,
  unsigned          cn);

unsigned taa_h264_put_level(
  bitwriter_t * writer,
  int           level);



#ifdef TAA_H264_TRACE_ENABLED

static __inline unsigned taa_h264_write_u_trace (
  bitwriter_t * w,
  uint32_t      val,
  int           len,
  const char *  str)
{
  taa_h264_write_bits (w, len, val);
  TAA_H264_TRACE_VALUE (TAA_H264_TRACE_HEADERS, str, val);
  return len;
}

static __inline unsigned taa_h264_write_ue_trace (
  bitwriter_t * w,
  uint32_t      val,
  const char *  str)
{
  int len = taa_h264_put_uvlc (w, val);
  TAA_H264_TRACE_VALUE (TAA_H264_TRACE_HEADERS, str, val);
  return len;
}

static __inline unsigned taa_h264_write_se_trace (
  bitwriter_t * w,
  int32_t       val,
  const char *  str)
{
  int len = taa_h264_put_level (w, val);
  TAA_H264_TRACE_VALUE (TAA_H264_TRACE_HEADERS, str, val);
  return len;
}
#endif

#endif
