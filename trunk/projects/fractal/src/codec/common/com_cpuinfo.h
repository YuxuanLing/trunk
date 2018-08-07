#ifndef _CPUINFO_H_
#define _CPUINFO_H_

#include "com_typedefs.h"

struct cpuinfo_s
{
  bool have_mmx;
  bool have_sse;
  bool have_sse2;
  bool have_sse3;
  bool have_ssse3;
  bool have_sse4_1;
  bool have_sse4_2;
};

void taa_h264_get_cpuinfo (
  cpuinfo_t * cpuinfo);

#endif
