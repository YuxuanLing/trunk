#ifndef _ERROR_H_
#define _ERROR_H_

#include "com_typedefs.h"

typedef enum
{
  TAA_H264_NO_ERROR = 0,
  TAA_H264_INVALID_ARGUMENT,
  TAA_H264_ALLOCATION_ERROR,
  TAA_H264_UNEXPECTED_EOS,
  TAA_H264_INVALID_ZERO_BIT,
  TAA_H264_INVALID_SPS_REFERENCED,
  TAA_H264_INVALID_PPS_REFERENCED,
  TAA_H264_INVALID_REFERENCE_FRAME,
  TAA_H264_INVALID_QP,
  TAA_H264_INVALID_MBTYPE,
  TAA_H264_INVALID_CBP,
  TAA_H264_INVALID_INTRA_MODE,
  TAA_H264_GAP_IN_FRAME_NUM_NOT_ALLOWED,
  TAA_H264_DPB_INVALID_RPLR,
  TAA_H264_DPB_INVALID_MMCO,
  TAA_H264_DPB_INVALID_SHORT_TERM_PICNUM,
  TAA_H264_DPB_INVALID_LONG_TERM_FRAME_INDEX,
  TAA_H264_DPB_INTERNAL_ERROR,
  TAA_H264_UNSUPPORTED_RESOLUTION,
  TAA_H264_UNSUPPORTED_NALU_TYPE,
  TAA_H264_UNSUPPORTED_SEI_TYPE,
  TAA_H264_UNKNOWN_SEI_UUID,
  TAA_H264_INVALID_NUM_REFERENCE_FRAMES,
  TAA_H264_ALLOCATION_ERROR_NUM_COEFFS_Y,
  TAA_H264_ALLOCATION_ERROR_NUM_COEFFS_UV,
  TAA_H264_ALLOCATION_ERROR_INTRA_MODES,
  TAA_H264_ALLOCATION_ERROR_MBTYPES,
  TAA_H264_ALLOCATION_ERROR_MOTION_VECTORS,
  TAA_H264_ALLOCATION_ERROR_MOTION_VECTORS_CURR,
  TAA_H264_ALLOCATION_ERROR_MOTION_VECTORS_PREV,
  TAA_H264_ALLOCATION_ERROR_MOTION_VECTORS_CURR_16X16,
  TAA_H264_ALLOCATION_ERROR_MOTION_VECTORS_PREV_16X16,
  TAA_H264_ALLOCATION_ERROR_MBINFO_STORED,
  TAA_H264_ALLOCATION_ERROR_MEINFO,
  TAA_H264_ALLOCATION_ERROR_DPB_MEM_BASE,
  TAA_H264_ALLOCATION_ERROR_FRAMEINFO_CREATE,
  TAA_H264_ALLOCATION_ERROR_BITWRITER_CREATE,
  TAA_H264_ALLOCATION_ERROR_RESIZER_CREATE,
  TAA_H264_ALLOCATION_ERROR_CONTROL_CREATE,
  TAA_H264_ALLOCATION_ERROR_MB_DEBLOCK_INFO
} error_codes_t;

typedef struct
{
  bool error_detected;
  error_cb_t func;
  void * context;
} error_handler_t;

void taa_h264_error_ (
  error_handler_t * handler,
  const char *      filename,
  int               line,
  int               event,
  taa_h264_error_t  type);

#define TAA_H264_CRITICAL(handler, event) taa_h264_error_ (handler, __FILE__, __LINE__, event, TAA_H264_ERR_CRITICAL)
#define TAA_H264_ERROR(handler, event)    taa_h264_error_ (handler, __FILE__, __LINE__, event, TAA_H264_ERR_ERROR)
#define TAA_H264_WARNING(handler, event)  taa_h264_error_ (handler, __FILE__, __LINE__, event, TAA_H264_ERR_WARNING)

#ifndef NDEBUG

# include "taah264stdtypes.h"
# include <stdio.h>

# define MYDEBUG(...) TAA_H264_TRACE(0xFF, __VA_ARGS__)

# define TAA_H264_TRACE_ENABLED
//# define TAA_H264_ASSERTS_ENABLED

typedef enum {
  TAA_H264_TRACE_NONE       = 0,
  TAA_H264_TRACE_KEY_VALUES = 1,
  TAA_H264_TRACE_HEADERS    = 2,
  TAA_H264_TRACE_CAVLC      = 4,
  TAA_H264_TRACE_MV         = 8,
  TAA_H264_TRACE_DPB        = 16,
  TAA_H264_TRACE_DEBLOCK    = 32
} trace_level;

void taa_h264_init_trace_ (
  int    level_flags,
  FILE * tracefile);
void taa_h264_trace_bitstream_ (
  int          level,
  const char * name,
  uint32_t     bits,
  int          len);
void taa_h264_trace_value_ (
  int          level,
  const char * name,
  int          value);
void taa_h264_trace_ (
  int          level,
  const char * msg,
  ...);
bool taa_h264_trace_level_enabled_ (int level);

# define TAA_H264_TRACE_BITSTREAM(level, name, symbol, len) taa_h264_trace_bitstream_ (level, name, symbol, len)
# define TAA_H264_TRACE_VALUE(level, name, value)           taa_h264_trace_value_ (level, name, value)
# define TAA_H264_TRACE(level, ...)                         taa_h264_trace_ (level, __VA_ARGS__)
# define TAA_H264_INIT_TRACE(flags, tracefile)              taa_h264_init_trace_ (flags, tracefile)
# define TAA_H264_TRACE_LEVEL_ENABLED(level)                taa_h264_trace_level_enabled_ (level)


#else

/* Make all debug functions no-ops */

# define MYDEBUG(...)

# define TAA_H264_INIT_TRACE(flags, tracefile)
# define TAA_H264_TRACE_BITSTREAM(level, name, symbol, len)
# define TAA_H264_TRACE_VALUE(level, name, value)
# define TAA_H264_TRACE(level, ...)
# define TAA_H264_TRACE_LEVEL_ENABLED(level)                false

#endif


#ifdef TAA_H264_ASSERTS_ENABLED
# include <assert.h>
# define TAA_H264_DEBUG_ASSERT(expr)                        assert (expr)
#else
# define TAA_H264_DEBUG_ASSERT(expr)
#endif


#endif
