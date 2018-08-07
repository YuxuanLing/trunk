#ifndef _TAA_H264_TYPES_H_
#define _TAA_H264_TYPES_H_

#include <stddef.h>

#if (defined(_MSC_VER) && (_MSC_VER >= 1600)) /* VS2010 or later */

#if _MSC_VER < 1900
  #include <VC/include/stdint.h> /* to simulate #include_next on windows */
#else
  #include <stdint.h>
#endif

  #define bool int
  #define true 1
  #define false 0
  #define __bool_true_false_are_defined 1

#elif defined(_MSC_VER)

  /* Need to define the types for Windows compatibility */
  typedef __int8           int8_t;
  typedef __int16          int16_t;
  typedef __int32          int32_t;
  typedef __int64          int64_t;
  typedef unsigned __int8  uint8_t;
  typedef unsigned __int16 uint16_t;
  typedef unsigned __int32 uint32_t;
  typedef unsigned __int64 uint64_t;

  #define bool int
  #define true 1
  #define false 0
  #define __bool_true_false_are_defined 1

  /* Normally the limits for the above types are defined in stdint.h, but windows
   * has decided to put it in limits.h */
  #include <limits.h>

  #define INT8_MIN  _I8_MIN
  #define INT8_MAX  _I8_MAX
  #define UINT8_MAX _UI8_MAX

  #define INT16_MIN  _I16_MIN
  #define INT16_MAX  _I16_MAX
  #define UINT16_MAX _UI16_MAX

  #define INT32_MIN  _I32_MIN
  #define INT32_MAX  _I32_MAX
  #define UINT32_MAX _UI32_MAX

#else

  #include <stdint.h>
  #include <stdbool.h>

#endif

/**
 * WARNING:  Recoverable error, output video probably has artifacts
 * ERROR:    Recoverable error, but no video out before it's fixed
 * CRITICAL: Non-recoverable error
 */
typedef enum
{
  TAA_H264_ERR_WARNING,
  TAA_H264_ERR_ERROR,
  TAA_H264_ERR_CRITICAL
} taa_h264_error_t;

typedef void (*taa_h264_report_error_func)(void *           context,
                                           taa_h264_error_t error_type,
                                           const char *     error_file,
                                           int              error_line,
                                           const char *     msg);

#endif
