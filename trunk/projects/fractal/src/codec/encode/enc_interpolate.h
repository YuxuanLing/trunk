/* ================================================================================= */
/* */
/*                               Copyright Notice */
/* */
/*  All material herein: Copyright © 2009 TANDBERG Telecom AS. All Rights Reserved. */
/*  This product is protected under US patents. Other patents pending. */
/*  Further information can be found at http://www.tandberg.com/tandberg_pm.jsp. */
/* */
/*  The source code is owned by TANDBERG Telecom AS and is protected by copyright */
/*  laws and international copyright treaties, as well as other intellectual */
/*  property laws and treaties. All right, title and interest in the source code */
/*  are owned TANDBERG Telecom AS. Therefore, you must treat the source code like */
/*  any other copyrighted material. */
/* */
/* ================================================================================= */

#include "enc_config.h"
#include "com_resolutions.h"

#ifndef AVG_HPEL

void taa_h264_hpel_interpolation (
  unsigned char *    ph,
  unsigned char *    pv,
  unsigned char *    qd,  // diagonal    hpel
  unsigned char *    qh,  // horizontal  hpel
  unsigned char *    qv,  // vertical    hpel
  const int          h,
  const int          w,
  const source_res_t resolution);

#endif
