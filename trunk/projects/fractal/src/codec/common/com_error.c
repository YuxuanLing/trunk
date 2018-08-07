#include "com_error.h"

#include <stdio.h>

#ifndef NDEBUG
# include <stdarg.h>

static int trace_levels = TAA_H264_TRACE_NONE;
static FILE * tracefile = NULL;

void taa_h264_init_trace_ (
  int    level_flags,
  FILE * file)
{
  trace_levels = level_flags;
  tracefile = file;
}

void taa_h264_trace_bitstream_ (
  int          level,
  const char * name,
  uint32_t     symbol,
  int          len)
{
  if (level & trace_levels)
  {
    char bitstring[33];
    for (int i = len - 1; i >= 0; i--)
      bitstring[i] = (uint8_t)(((symbol >> i) & 0x1) ? '1' : '0');
    bitstring[len] = '\0';
    fprintf (tracefile, "%-50s %10u [%32s]\n", name, symbol, bitstring);
  }
}

void taa_h264_trace_value_ (
  int          level,
  const char * name,
  int          value)
{
  if (level & trace_levels)
    fprintf (tracefile, "%-50s %10d\n", name, value);
}

void taa_h264_trace_ (
  int          level,
  const char * msg,
  ...)
{
  if (level & trace_levels)
  {
    va_list args;
    va_start (args, msg);
    vfprintf (tracefile, msg, args);
    va_end (args);
    fflush (tracefile);
  }
}

bool taa_h264_trace_level_enabled_ (int level)
{
  return (level & trace_levels) ? true : false;
}

#endif

/* NOTE: Be sure to keep the enums in error.h and this table in sync. */
static const char * error_code_strings[] =
{
  "TAA_H264_NO_ERROR",
  "TAA_H264_INVALID_ARGUMENT",
  "TAA_H264_ALLOCATION_ERROR",
  "TAA_H264_UNEXPECTED_EOS",
  "TAA_H264_INVALID_ZERO_BIT",
  "TAA_H264_INVALID_SPS_REFERENCED",
  "TAA_H264_INVALID_PPS_REFERENCED",
  "TAA_H264_INVALID_REFERENCE_FRAME",
  "TAA_H264_INVALID_QP",
  "TAA_H264_INVALID_MBTYPE",
  "TAA_H264_INVALID_CBP",
  "TAA_H264_INVALID_INTRA_MODE",
  "TAA_H264_GAP_IN_FRAME_NUM_NOT_ALLOWED",
  "TAA_H264_DPB_INVALID_RPLR",
  "TAA_H264_DPB_INVALID_MMCO",
  "TAA_H264_DPB_INVALID_SHORT_TERM_PICNUM",
  "TAA_H264_DPB_INVALID_LONG_TERM_FRAME_INDEX",
  "TAA_H264_DPB_INTERNAL_ERROR",
  "TAA_H264_UNSUPPORTED_RESOLUTION",
  "TAA_H264_UNSUPPORTED_NALU_TYPE",
  "TAA_H264_UNSUPPORTED_SEI_TYPE",
  "TAA_H264_UNKNOWN_SEI_UUID",
  "TAA_H264_INVALID_NUM_REFERENCE_FRAMES",
  "TAA_H264_ALLOCATION_ERROR_NUM_COEFFS_Y",
  "TAA_H264_ALLOCATION_ERROR_NUM_COEFFS_UV",
  "TAA_H264_ALLOCATION_ERROR_INTRA_MODES",
  "TAA_H264_ALLOCATION_ERROR_MBTYPES",
  "TAA_H264_ALLOCATION_ERROR_MOTION_VECTORS",
  "TAA_H264_ALLOCATION_ERROR_MOTION_VECTORS_CURR",
  "TAA_H264_ALLOCATION_ERROR_MOTION_VECTORS_PREV",
  "TAA_H264_ALLOCATION_ERROR_MOTION_VECTORS_CURR_16X16",
  "TAA_H264_ALLOCATION_ERROR_MOTION_VECTORS_PREV_16X16",
  "TAA_H264_ALLOCATION_ERROR_MBINFO_STORED",
  "TAA_H264_ALLOCATION_ERROR_MEINFO",
  "TAA_H264_ALLOCATION_ERROR_DPB_MEM_BASE",
  "TAA_H264_ALLOCATION_ERROR_FRAMEINFO_CREATE",
  "TAA_H264_ALLOCATION_ERROR_BITWRITER_CREATE",
  "TAA_H264_ALLOCATION_ERROR_RESIZER_CREATE",
  "TAA_H264_ALLOCATION_ERROR_CONTROL_CREATE"
};

void taa_h264_error_ (
  error_handler_t * handler,
  const char *      filename,
  int               line,
  int               event,
  taa_h264_error_t  type)
{
  handler->error_detected |= (type > TAA_H264_ERR_WARNING);
  if (handler->func)
  {
    handler->func (handler->context, type, filename, line,
                   error_code_strings[event]);
  }
}
