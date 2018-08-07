#if HAVE_CONFIG_H
#include "config.h"
#endif
#include "com_cpuinfo.h"
#include <string.h>

//#if __INTEL_COMPILER
#if 1

void taa_h264_get_cpuinfo (
  cpuinfo_t * cpuinfo)
{
  const int mmx_flag    = 1 << 23;
  const int sse_flag    = 1 << 25;      /* Safe to use -QxK/-xK */
  const int sse2_flag   = 1 << 26;      /* Safe to use -QxN/-xN */
  const int sse3_flag   = 1 <<  0;      /* Safe to use -QxP/-xP */
  const int ssse3_flag  = 1 <<  9;      /* Safe to use -QxT/-xT */
  const int sse4_1_flag = 1 << 19;
  const int sse4_2_flag = 1 << 20;

  int info[4] = { -1};
  unsigned int vendor[4] = {0, 0, 0, 0};

  __cpuid(info, 0);
  vendor[0] = info[1];
  vendor[1] = info[3];
  vendor[2] = info[2];

  __cpuid(info, 1);

  if (strncmp ((char *) vendor, "GenuineIntel", 12) != 0 &&
      strncmp ((char *) vendor, "AuthenticAMD", 12) != 0)
  {
    info[2] = 0;
  }

  cpuinfo->have_mmx    = (info[3] & mmx_flag   ) > 0;
  cpuinfo->have_sse    = (info[3] & sse_flag   ) > 0;
  cpuinfo->have_sse2   = (info[3] & sse2_flag  ) > 0;
  cpuinfo->have_sse3   = (info[2] & sse3_flag  ) > 0;
  cpuinfo->have_ssse3  = (info[2] & ssse3_flag ) > 0;
  cpuinfo->have_sse4_1 = (info[2] & sse4_1_flag) > 0;
  cpuinfo->have_sse4_2 = (info[2] & sse4_2_flag) > 0;
}

#else

#if HAVE_CPUID_H
# include <cpuid.h>

void taa_h264_get_cpuinfo (
  cpuinfo_t * cpuinfo)
{
  unsigned int eax = 0;
  unsigned int ebx = 0;
  unsigned int ecx = 0;
  unsigned int edx = 0;

  unsigned int vendor[4] = {0, 0, 0, 0};

  __get_cpuid (0, &eax, &ebx, &ecx, &edx);
  vendor[0] = ebx;
  vendor[1] = edx;
  vendor[2] = ecx;

  if (!__get_cpuid (1, &eax, &ebx, &ecx, &edx))
  {
    ecx = 0;
    edx = 0;
  }

  if (strncmp ((char *) vendor, "GenuineIntel", 12) != 0 &&
      strncmp ((char *) vendor, "AuthenticAMD", 12) != 0)
  {
    ecx = 0;
  }

  cpuinfo->have_mmx    = (edx & bit_MMX   ) > 0;
  cpuinfo->have_sse    = (edx & bit_SSE   ) > 0;
  cpuinfo->have_sse2   = (edx & bit_SSE2  ) > 0;
  cpuinfo->have_sse3   = (ecx & bit_SSE3  ) > 0;
  cpuinfo->have_ssse3  = (ecx & bit_SSSE3 ) > 0;
  cpuinfo->have_sse4_1 = (ecx & bit_SSE4_1) > 0;
  cpuinfo->have_sse4_2 = (ecx & bit_SSE4_2) > 0;
}
#else  /* HAVE_CPUID_H */

# if ENV_DARWIN

#  include <mach/mach_types.h>
#  include <sys/types.h>
#  include <sys/sysctl.h>

#define _Bit(n)			(1ULL << n)
#define _HBit(n)		(1ULL << ((n)+32))
#define quad(hi,lo)	(((uint64_t)(hi)) << 32 | (lo))

/*
 * The CPUID_FEATURE_XXX values define 64-bit values
 * returned in %ecx:%edx to a CPUID request with %eax of 1: 
 */
#define	CPUID_FEATURE_FPU     _Bit(0)	/* Floating point unit on-chip */
#define	CPUID_FEATURE_VME     _Bit(1)	/* Virtual Mode Extension */
#define	CPUID_FEATURE_DE      _Bit(2)	/* Debugging Extension */
#define	CPUID_FEATURE_PSE     _Bit(3)	/* Page Size Extension */
#define	CPUID_FEATURE_TSC     _Bit(4)	/* Time Stamp Counter */
#define	CPUID_FEATURE_MSR     _Bit(5)	/* Model Specific Registers */
#define CPUID_FEATURE_PAE     _Bit(6)	/* Physical Address Extension */
#define	CPUID_FEATURE_MCE     _Bit(7)	/* Machine Check Exception */
#define	CPUID_FEATURE_CX8     _Bit(8)	/* CMPXCHG8B */
#define	CPUID_FEATURE_APIC    _Bit(9)	/* On-chip APIC */
#define CPUID_FEATURE_SEP     _Bit(11)	/* Fast System Call */
#define	CPUID_FEATURE_MTRR    _Bit(12)	/* Memory Type Range Register */
#define	CPUID_FEATURE_PGE     _Bit(13)	/* Page Global Enable */
#define	CPUID_FEATURE_MCA     _Bit(14)	/* Machine Check Architecture */
#define	CPUID_FEATURE_CMOV    _Bit(15)	/* Conditional Move Instruction */
#define CPUID_FEATURE_PAT     _Bit(16)	/* Page Attribute Table */
#define CPUID_FEATURE_PSE36   _Bit(17)	/* 36-bit Page Size Extension */
#define CPUID_FEATURE_PSN     _Bit(18)	/* Processor Serial Number */
#define CPUID_FEATURE_CLFSH   _Bit(19)	/* CLFLUSH Instruction supported */
#define CPUID_FEATURE_DS      _Bit(21)	/* Debug Store */
#define CPUID_FEATURE_ACPI    _Bit(22)	/* Thermal monitor and Clock Ctrl */
#define CPUID_FEATURE_MMX     _Bit(23)	/* MMX supported */
#define CPUID_FEATURE_FXSR    _Bit(24)	/* Fast floating pt save/restore */
#define CPUID_FEATURE_SSE     _Bit(25)	/* Streaming SIMD extensions */
#define CPUID_FEATURE_SSE2    _Bit(26)	/* Streaming SIMD extensions 2 */
#define CPUID_FEATURE_SS      _Bit(27)	/* Self-Snoop */
#define CPUID_FEATURE_HTT     _Bit(28)	/* Hyper-Threading Technology */
#define CPUID_FEATURE_TM      _Bit(29)	/* Thermal Monitor (TM1) */
#define CPUID_FEATURE_PBE     _Bit(31)	/* Pend Break Enable */

#define CPUID_FEATURE_SSE3    _HBit(0)	/* Streaming SIMD extensions 3 */
#define CPUID_FEATURE_MONITOR _HBit(3)	/* Monitor/mwait */
#define CPUID_FEATURE_DSCPL   _HBit(4)	/* Debug Store CPL */
#define CPUID_FEATURE_VMX     _HBit(5)	/* VMX */
#define CPUID_FEATURE_SMX     _HBit(6)	/* SMX */
#define CPUID_FEATURE_EST     _HBit(7)	/* Enhanced SpeedsTep (GV3) */
#define CPUID_FEATURE_TM2     _HBit(8)	/* Thermal Monitor 2 */
#define CPUID_FEATURE_SSSE3   _HBit(9)	/* Supplemental SSE3 instructions */
#define CPUID_FEATURE_CID     _HBit(10)	/* L1 Context ID */
#define CPUID_FEATURE_CX16    _HBit(13)	/* CmpXchg16b instruction */
#define CPUID_FEATURE_xTPR    _HBit(14)	/* Send Task PRiority msgs */
#define CPUID_FEATURE_PDCM    _HBit(15)	/* Perf/Debug Capability MSR */
#define CPUID_FEATURE_DCA     _HBit(18)	/* Direct Cache Access */
#define CPUID_FEATURE_SSE4_1  _HBit(19)	/* Streaming SIMD extensions 4.1 */
#define CPUID_FEATURE_SSE4_2  _HBit(20)	/* Streaming SIMD extensions 4.2 */
#define CPUID_FEATURE_xAPIC   _HBit(21)	/* Extended APIC Mode */
#define CPUID_FEATURE_POPCNT  _HBit(23)	/* POPCNT instruction */
#define CPUID_FEATURE_VMM     _HBit(31)	/* VMM (Hypervisor) present */

/*
 * The CPUID_EXTFEATURE_XXX values define 64-bit values
 * returned in %ecx:%edx to a CPUID request with %eax of 0x80000001: 
 */
#define CPUID_EXTFEATURE_SYSCALL   _Bit(11)	/* SYSCALL/sysret */
#define CPUID_EXTFEATURE_XD	   _Bit(20)	/* eXecute Disable */
#define CPUID_EXTFEATURE_RDTSCP	   _Bit(27)	/* RDTSCP */
#define CPUID_EXTFEATURE_EM64T	   _Bit(29)	/* Extended Mem 64 Technology */

#define CPUID_EXTFEATURE_LAHF	   _HBit(20)	/* LAFH/SAHF instructions */

/*
 * The CPUID_EXTFEATURE_XXX values define 64-bit values
 * returned in %ecx:%edx to a CPUID request with %eax of 0x80000007: 
 */
#define CPUID_EXTFEATURE_TSCI      _Bit(8)	/* TSC Invariant */

#define	CPUID_CACHE_SIZE	16	/* Number of descriptor values */

#define CPUID_MWAIT_EXTENSION	_Bit(0)	/* enumeration of WMAIT extensions */
#define CPUID_MWAIT_BREAK	_Bit(1)	/* interrupts are break events	   */

#define CPUID_MODEL_YONAH	14
#define CPUID_MODEL_MEROM	15
#define CPUID_MODEL_PENRYN	23
#define CPUID_MODEL_NEHALEM	26

void taa_h264_get_cpuinfo (
  cpuinfo_t * cpuinfo)
{
  const char* name = "machdep.cpu.feature_bits";
  int ret;
  size_t result_size = 8;  /* 64-bit integer result */
  uint64_t result;
  
  /* all Intel Macs we use are 64-bit core-2 or better. they all have
  mmx, sse, sse2 and sse3. */
  cpuinfo->have_mmx    = true;
  cpuinfo->have_sse    = true;
  cpuinfo->have_sse2   = true;
  cpuinfo->have_sse3   = true;

  ret = sysctlbyname(name, &result, &result_size, NULL, 0);
  if (ret != 0) {
    return;
  }

  cpuinfo->have_ssse3  = (result & CPUID_FEATURE_SSSE3 ) > 0;
  cpuinfo->have_sse4_1 = (result & CPUID_FEATURE_SSE4_1) > 0;
  cpuinfo->have_sse4_2 = (result & CPUID_FEATURE_SSE4_2) > 0;
}
# else /* ENV_DARWIN */
# error "cpuid information is missing. Please implement taa_h264_get_cpuinfo."
#endif
#endif /* HAVE_CPUID_H */
#endif  /* HAVE_CPUID_H */
