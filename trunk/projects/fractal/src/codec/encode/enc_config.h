#ifndef _ENC_CONFIG_H_
#define _ENC_CONFIG_H_

/* Compile with x86-64 optimized functions */
#define OPTIMIZE_X86_64


/* The number of reference frames to store in the reference buffer
 * (aka DPB). 1 <= n <= 16 */
#define NUM_REF_FRAMES 16

/* Log2 of (maximum frame number + 1). Must be 4 <= n <= 16. */
#define LOG2_MAX_FRAME_NUM 15

/* Calculate half pel interpolation on the fly during motion search
 * using bilinear average */
#define AVG_HPEL

/* Wether diagonal half pels and the depending quarter pels should be
 * enabled during motion search */
#define HPEL_DIAG_ENABLED 0

/* Disable fast object motion. Likely in video conferencing */
#define DISABLE_FAST_OBJECT_MOTION

/* Detection and removal of trailing artifacts
 * 0: disable
 * 1: intra 16x16 only
 * 2: intra 16x16 and 4x4 */
#define TRAILING_ARTIFACT_REMOVAL 0

/* If earlyskip detects that LUMA residual is small with skip mv, don't do
 * motion search */
#define EARLY_SKIP_MV_SEARCH 1

/* If we should check early after motion search whether to encode CBP. */
#define MIDDLE_SKIP_CBP_CHECK 1

/* The size of the averaging window for the extra cost of approximate I4x4 */
#define INTRA_4x4_APPROX_AVG_WINDOW 16



/**
 * Below we set some convenience macros based on the definitions
 * above. These should not be set manually. */

/* Debug the interpolation functions if we're in debug mode and we're
 * not using average hpel. Average hpel would be an approximation and
 * not yield the same result. */
#define SANITY_INTERPOLATION !defined(NDEBUG) && !defined(AVG_HPEL)
#define SANITY_MV_SEARCH SANITY_INTERPOLATION && (_MSC_VER >= 1500 && _MSC_VER < 1600)

// Probably not the most correct place to put this constant, but I'm feeling lazy
#define LAMBDA_SHIFT 6

#endif
