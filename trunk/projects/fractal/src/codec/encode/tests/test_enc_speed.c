#include "test_enc_speed.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
# include <windows.h>
#endif

#ifdef __INTEL_COMPILER
# include <ia32intrin.h>
#else
//# include <intrin.h>
//# pragma intrinsic(__rdtsc)
#endif

#pragma warning (disable : 981)

static double speed = 1.0;

static void set_cpu_speed ()
{
    #ifdef WIN32
  __int64 pf, p1, p2, ticks1, ticks2;
  QueryPerformanceFrequency((LARGE_INTEGER *) &pf);
  QueryPerformanceCounter((LARGE_INTEGER *) &p1);
  ticks1 = __rdtsc();
  for (volatile long i = 0; i < 1000000; ++i) ;
  ticks2 = __rdtsc();
  QueryPerformanceCounter((LARGE_INTEGER *) &p2);
  speed = (double)(pf * (ticks2 - ticks1)) / ((p2 - p1) * 1000000000);
  printf("\n\nCPU Speed = %0.2f GHz\n\n", (double) speed);
    #else
  speed = 2.4;
    #endif
}

void print_result (
  const char * name,
  clock_t      start,
  clock_t      finish)
{
  double time_diff = (double)(finish - start);
  printf("%-50s:\t\t %14.1f\t\t %8.1f\n", name,
         time_diff * 1000.0 * speed,
         (1000.0 * CLOCKS_PER_SEC) / time_diff);
}


#ifndef WIN32
# include <sys/resource.h>
static void unlimit_stack (
  void)
{

  struct rlimit rlim = { RLIM_INFINITY, RLIM_INFINITY };

  if (setrlimit(RLIMIT_STACK, &rlim) == -1 ) {
    perror("setrlimit error");
    exit(1);
  }
}
#endif

int main ()
{
  set_cpu_speed ();
    #ifndef WIN32
  unlimit_stack ();
    #endif
  test_enc_me_speed();
  test_enc_deblock_speed();
  test_enc_interpolate_speed();

  return 0;
}
