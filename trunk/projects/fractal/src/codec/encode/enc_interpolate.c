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
#include <memory.h>
#include "enc_interpolate.h"
#include "com_common.h"
#include "enc_config.h"

#ifndef AVG_HPEL

/* res_SQCIF */
# define W (128 + PAD_HORZ_Y * 2)
# define H 96
# define TAA_H264_HPEL_INTERPOLATION TAA_H264_SQCIF_HPEL_INTERPOLATION
# include "enc_interpolate_template.h"
# undef W
# undef H
# undef TAA_H264_HPEL_INTERPOLATION

/* res_QCIF */
# define W (176 + PAD_HORZ_Y * 2)
# define H 144
# define TAA_H264_HPEL_INTERPOLATION TAA_H264_QCIF_HPEL_INTERPOLATION
# include "enc_interpolate_template.h"
# undef W
# undef H
# undef TAA_H264_HPEL_INTERPOLATION

/* res_SIF */
# define W (352 + PAD_HORZ_Y * 2)
# define H 240
# define TAA_H264_HPEL_INTERPOLATION TAA_H264_SIF_HPEL_INTERPOLATION
# include "enc_interpolate_template.h"
# undef W
# undef H
# undef TAA_H264_HPEL_INTERPOLATION

/* res_CIF */
# define W (352 + PAD_HORZ_Y * 2)
# define H 288
# define TAA_H264_HPEL_INTERPOLATION TAA_H264_CIF_HPEL_INTERPOLATION
# include "enc_interpolate_template.h"
# undef W
# undef H
# undef TAA_H264_HPEL_INTERPOLATION

/* res_WCIF */
# define W (512 + PAD_HORZ_Y * 2)
# define H 288
# define TAA_H264_HPEL_INTERPOLATION TAA_H264_WCIF_HPEL_INTERPOLATION
# include "enc_interpolate_template.h"
# undef W
# undef H
# undef TAA_H264_HPEL_INTERPOLATION

/* res_25CIF */
# define W (576 + PAD_HORZ_Y * 2)
# define H 448
# define TAA_H264_HPEL_INTERPOLATION TAA_H264_25CIF_HPEL_INTERPOLATION
# include "enc_interpolate_template.h"
# undef W
# undef H
# undef TAA_H264_HPEL_INTERPOLATION

/* res_VGA */
# define W (640 + PAD_HORZ_Y * 2)
# define H 480
# define TAA_H264_HPEL_INTERPOLATION TAA_H264_VGA_HPEL_INTERPOLATION
# include "enc_interpolate_template.h"
# undef W
# undef H
# undef TAA_H264_HPEL_INTERPOLATION

/* res_4SIF */
# define W (704 + PAD_HORZ_Y * 2)
# define H 480
# define TAA_H264_HPEL_INTERPOLATION TAA_H264_4SIF_HPEL_INTERPOLATION
# include "enc_interpolate_template.h"
# undef W
# undef H
# undef TAA_H264_HPEL_INTERPOLATION

/* res_W3CIF */
# define W (768 + PAD_HORZ_Y * 2)
# define H 448
# define TAA_H264_HPEL_INTERPOLATION TAA_H264_W3CIF_HPEL_INTERPOLATION
# include "enc_interpolate_template.h"
# undef W
# undef H
# undef TAA_H264_HPEL_INTERPOLATION

/* res_4CIF */
# define W (704 + PAD_HORZ_Y * 2)
# define H 576
# define TAA_H264_HPEL_INTERPOLATION TAA_H264_4CIF_HPEL_INTERPOLATION
# include "enc_interpolate_template.h"
# undef W
# undef H
# undef TAA_H264_HPEL_INTERPOLATION

/* res_SVGA */
# define W (800 + PAD_HORZ_Y * 2)
# define H 600
# define TAA_H264_HPEL_INTERPOLATION TAA_H264_SVGA_HPEL_INTERPOLATION
# include "enc_interpolate_template.h"
# undef W
# undef H
# undef TAA_H264_HPEL_INTERPOLATION

/* res_W4CIF */
# define W (1024 + PAD_HORZ_Y * 2)
# define H 576
# define TAA_H264_HPEL_INTERPOLATION TAA_H264_W4CIF_HPEL_INTERPOLATION
# include "enc_interpolate_template.h"
# undef W
# undef H
# undef TAA_H264_HPEL_INTERPOLATION

/* res_XGA */
# define W (1024 + PAD_HORZ_Y * 2)
# define H 768
# define TAA_H264_HPEL_INTERPOLATION TAA_H264_XGA_HPEL_INTERPOLATION
# include "enc_interpolate_template.h"
# undef W
# undef H
# undef TAA_H264_HPEL_INTERPOLATION

/* res_720P */
# define W (1280 + PAD_HORZ_Y * 2)
# define H 720
# define TAA_H264_HPEL_INTERPOLATION TAA_H264_720P_HPEL_INTERPOLATION
# include "enc_interpolate_template.h"
# undef W
# undef H
# undef TAA_H264_HPEL_INTERPOLATION

/* res_1080P */
# define W (1920 + PAD_HORZ_Y * 2)
# define H 1088
# define TAA_H264_HPEL_INTERPOLATION TAA_H264_1080P_HPEL_INTERPOLATION
# include "enc_interpolate_template.h"
# undef W
# undef H
# undef TAA_H264_HPEL_INTERPOLATION

/* res_UNKNOWN */
# define TAA_H264_HPEL_INTERPOLATION TAA_H264_UNKNOWN_HPEL_INTERPOLATION
# include "enc_interpolate_template.h"
# undef TAA_H264_HPEL_INTERPOLATION


/* =============================================================================== */
/* */
/* This function implements the 6-tap FIR filter in the H.264 and ISO/IEC */
/* International Standard 14 496-10 (MPEG-4 part 10) for Advanced Video Coding (AVC). */
/* */
/*     T. Wiegand, G. Sullivan, G. Bjontegaard, and A. */
/*     Luthra. "Overview of the H.264/AVC Video Coding */
/*     Standard," IEEE Transactions on Circuits and Systems for */
/*     Video Technology, Vol. 13, No. 7, July 2003. */
/* */
/*     http://ip.hhi.de/imagecom_G1/assets/pdfs/csvt_overview_0305.pdf */
/* */
/* The particular implmentation given here is adapted to Intel Architecture to */
/* achieve: */
/* */
/*     1. Maximum SIMD performance, achieved by automatic vectorization. */
/*     2. Maximum ILP performance, achievd by pragma unrolling. */
/*     3. Maximum MEMORY performance, achived by resolution dependent filtering. */
/* */
/* The filter has been implemented using the little-known usage-pattern of the */
/* C preprocessor known as "X-Macros". */
/* */
/*     http://en.wikipedia.org/wiki/C_preprocessor#X-Macros */
/* */
/* This pattern ensures that loop count is fixed for each resolution and */
/* maximum MEMORY performance results as all strides are known compile time. */
/* */
/* The correct resolution is selected using a SWITCH statement in C that */
/* is compiled to a JUMP-table, with the advantage that it is compact and that */
/* the resolution is correctly predicted in 100% of the cases. */
/* */
/*      SWITCH statement  http://en.wikipedia.org/wiki/Switch_statement */
/*      JUMP table        http://en.wikipedia.org/wiki/Branch_table */
/* */
/* ============================================================================== */
void taa_h264_hpel_interpolation (
  unsigned char *    ph,
  unsigned char *    pv,
  unsigned char *    qd,  // diagonal    hpel
  unsigned char *    qh,  // horizontal  hpel
  unsigned char *    qv,  // vertical    hpel
  const int          h,
  const int          w,
  const source_res_t resolution)
{
  switch (resolution)
  {
  case   RES_SQCIF: TAA_H264_SQCIF_HPEL_INTERPOLATION    (ph, pv, qd, qh, qv); break;
  case    RES_QCIF: TAA_H264_QCIF_HPEL_INTERPOLATION     (ph, pv, qd, qh, qv); break;
  case     RES_SIF: TAA_H264_SIF_HPEL_INTERPOLATION      (ph, pv, qd, qh, qv); break;
  case     RES_CIF: TAA_H264_CIF_HPEL_INTERPOLATION      (ph, pv, qd, qh, qv); break;
  case    RES_WCIF: TAA_H264_WCIF_HPEL_INTERPOLATION     (ph, pv, qd, qh, qv); break;
  case   RES_25CIF: TAA_H264_25CIF_HPEL_INTERPOLATION    (ph, pv, qd, qh, qv); break;
  case     RES_VGA: TAA_H264_VGA_HPEL_INTERPOLATION      (ph, pv, qd, qh, qv); break;
  case    RES_4SIF: TAA_H264_4SIF_HPEL_INTERPOLATION     (ph, pv, qd, qh, qv); break;
  case   RES_W3CIF: TAA_H264_W3CIF_HPEL_INTERPOLATION    (ph, pv, qd, qh, qv); break;
  case    RES_4CIF: TAA_H264_4CIF_HPEL_INTERPOLATION     (ph, pv, qd, qh, qv); break;
  case    RES_SVGA: TAA_H264_SVGA_HPEL_INTERPOLATION     (ph, pv, qd, qh, qv); break;
  case   RES_W4CIF: TAA_H264_W4CIF_HPEL_INTERPOLATION    (ph, pv, qd, qh, qv); break;
  case     RES_XGA: TAA_H264_XGA_HPEL_INTERPOLATION      (ph, pv, qd, qh, qv); break;
  case    RES_720P: TAA_H264_720P_HPEL_INTERPOLATION     (ph, pv, qd, qh, qv); break;
  case   RES_1080P: TAA_H264_1080P_HPEL_INTERPOLATION    (ph, pv, qd, qh, qv); break;
  case RES_UNKNOWN: TAA_H264_UNKNOWN_HPEL_INTERPOLATION  (h, w, ph, pv, qd, qh, qv); break;
  }
}
#endif
