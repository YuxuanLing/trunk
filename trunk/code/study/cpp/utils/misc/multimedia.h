/**********************************************************************************************************************
*	$Log: vsys#comm#cmodel#inc#multimedia.h_1,v $
*	Revision 1.4  2006-11-08 17:19:36-08  lsha
*	Intra DC mode added
*
*	Revision 1.3  2006-05-31 14:41:49-07  lsha
*	...No comments entered during checkin...
*
*
*	DESCRIPTION:
*	Common constants, definitions and useful macros for multimedia related applications.
*
**********************************************************************************************************************/

#ifndef	MULTIMEDIA
#define	MULTIMEDIA					"      MULTIMEDIA >>>    "
/**	MULTIMEDIA
 */

#include	"math_ops.h"
#include	"mm_tbl.h"



#ifdef	__cplusplus
	extern	"C"
	{
#endif

/**	SECTION - common video constants
 */
		typedef enum {

		VOL_eos						= 0,				/* End Of Video Sequence */
		VOL_seq						= 1,				/* Layer sequence */
		VOL_gop						= 2,				/* Layer group of picture */
		VOL_vop						= 3,				/* Layer picture object */
		VOL_slice					= 4,				/* Layer slice/GOB */
		VOL_macroblock				= 5,				/* Layer macroblock */

		} EVideoLayerObject;
#ifndef	__CODE_LINK__
		static	char	*epvol[]	= {	"End Of Sequence",
										"Sequence",
										"Group Of Picture",
										"Picture Object",
										"Slice",
										"Macroblock"
											};
#else
		#define	epvol				garrps8
#endif
		typedef enum {

		VOP_I						= 0,				/* I-picture */
		VOP_P						= 1,				/* P-picture */
		VOP_B						= 2,				/* B-picture */

		} EVideoPictureObject;
#ifndef	__CODE_LINK__
		static	char	*epvop[]	= {	"I",
										"P",
										"B"
											};
#else
		#define	epvop				garrps8
#endif
		typedef enum {

		VES_H261					= 0x61000000,		/* H.261 */
		VES_H263					= 0x63000000,		/* H.263 */
		VES_H264					= 0x64000000,		/* H.264 */
		VES_H264bp					= 0x64100000,		/* H.264 baseline profile */
		VES_H264xp					= 0x64200000,		/* H.264 extended profile */
		VES_H264mp					= 0x64300000,		/* H.264 main profile */
		VES_H264hp					= 0x64400000,		/* H.264 high profile */

		VES_MPG1					= 0x21000000,		/* MPEG1 */
		VES_MPG2					= 0x22000000,		/* MPEG2 */
		VES_MPG4					= 0x24000000,		/* MPEG4 */

		VES_WMV9					= 0x49000000,		/* Microsoft WMV9 */
		VES_WMV9sp					= 0x49100000,		/* Microsoft WMV9 simple profile */
		VES_WMV9mp					= 0x49200000,		/* Microsoft WMV9 main profile */
		VES_WMV9ap					= 0x49300000,		/* Microsoft WMV9 advanced profile */

		VES_AVSV					= 0x11000000,		/* AVS video */

		VES_MJPG					= 0x01000000,		/* Motion JPEG */
		VES_SNDV					= 0x02000000,		/* Sony DV */

		VES_						= 0xFF000000,		/* 'EVideoElementStream' mask */

		} EVideoElementStream;

		typedef enum {

		__scan_prog					= 0,				/* Picture scan: progressive */
		__scan_top					= 1,				/* Picture scan: interlace top field */
		__scan_btm					= 2,				/* Picture scan: interlace bottom field */
		__scan_weaved				= 3,				/* Picture scan: interlace weaved frame */

		} EFrameFieldScan;

		typedef enum {

		__blkp_1x1					= 0,				/* (Macro)block partition: no split (16x16/8x8) */
		__blkp_1x2					= 1,				/* (Macro)block partition: vertical split (16x8/8x4) */
		__blkp_2x1					= 2,				/* (Macro)block partition: horizontal split (8x16/4x8) */
		__blkp_2x2					= 3,				/* (Macro)block partition: bi-split (8x8/4x4) */

		} EAnyblockPartition;

		typedef enum {

		__mbpd_intra				= 0,				/* Intra macroblock */
		__mbpd_forward				= 1,				/* Motion: forward prediction */
		__mbpd_backward				= 2,				/* Motion: backward prediction */
		__mbpd_bipred				= 3,				/* Motion: bi-direction prediction */

		__mbpd_skip					= 4,				/* Skip mode */
		__mbpd_dp_direct			= 12,				/* Direct prediction (or MPEG2 dual-prime) */

		} EMacroblockPredictionDirection;

/**	ENDOFSECTION
 */



/**	SECTION - common data structures
 */
		typedef enum {

		H264_i16_vertical			= 0,				/* Intra luma 16x16 vertical prediction */
		H264_i16_horizontal			= 1,				/* Intra luma 16x16 horizontal prediction */
		H264_i16_dc					= 2,				/* Intra luma 16x16 DC prediction */
		H264_i16_plane				= 3,				/* Intra luma 16x16 plane prediction */

		} Eh264Intra16x16;
#ifndef	__CODE_LINK__
		static	char	*epi16[]	= {	"intra luma 16x16 vertical prediction",
										"intra luma 16x16 horizontal prediction",
										"intra luma 16x16 DC prediction",
										"intra luma 16x16 plane prediction",
											};
#else
		#define	epi16				garrps8
#endif
		typedef enum {

		H264_iUV_dc					= 0,				/* Intra chroma DC prediction */
		H264_iUV_horizontal			= 1,				/* Intra chroma horizontal prediction */
		H264_iUV_vertical			= 2,				/* Intra chroma vertical prediction */
		H264_iUV_plane				= 3,				/* Intra chroma plane prediction */

		} Eh264IntraChroma;
#ifndef	__CODE_LINK__
		static	char	*epiuv[]	= {	"intra chroma DC prediction",
										"intra chroma horizontal prediction",
										"intra chroma vertical prediction",
										"intra chroma plane prediction",
											};
#else
		#define	epiuv				garrps8
#endif
		typedef enum {

		H264_NxN_vertical			= 0,				/* Intra luma 4x4/8x8 vertical prediction */
		H264_NxN_horizontal			= 1,				/* Intra luma 4x4/8x8 horizontal prediction */
		H264_NxN_dc					= 2,				/* Intra luma 4x4/8x8 dc prediction */
		H264_NxN_downLeft			= 3,				/* Intra luma 4x4/8x8 diagonal down left prediction */
		H264_NxN_downRight			= 4,				/* Intra luma 4x4/8x8 diagonal down right prediction */
		H264_NxN_verticalRight		= 5,				/* Intra luma 4x4/8x8 vertical right prediction */
		H264_NxN_horizontalDown		= 6,				/* Intra luma 4x4/8x8 horizontal Down prediction */
		H264_NxN_verticalLeft		= 7,				/* Intra luma 4x4/8x8 vertical left prediction */
		H264_NxN_horizontalUp		= 8,				/* Intra luma 4x4/8x8 horizontal up prediction */

		} Eh264IntraNxN;
#ifndef	__CODE_LINK__
		static	char	*epiNxN[]	= {	"intra luma 4x4/8x8 vertical prediction",
										"intra luma 4x4/8x8 horizontal prediction",
										"intra luma 4x4/8x8 DC prediction",
										"intra luma 4x4/8x8 diagonal down left prediction",
										"intra luma 4x4/8x8 diagonal down right prediction",
										"intra luma 4x4/8x8 vertical right prediction",
										"intra luma 4x4/8x8 horizontal Down prediction",
										"intra luma 4x4/8x8 vertical left prediction",
										"intra luma 4x4/8x8 horizontal up prediction",
											};
#else
		#define	epiNxN				garrps8
#endif

/**	ENDOFSECTION
 */



/**	SECTION - H.264 specific constants
 */
		typedef struct {

		SIGN32	s32cols;								/* Width in horizontal sample count */
		SIGN32	s32rows;								/* Height in vertical sample count */
		void	*ptr;									/* Data pointer */
		SIGN32	s32pitch;								/* Picture stride in horizontal sample count */

		} Sptr2d;

/**	ENDOFSECTION
 */



/**	SECTION - common used functions
 */
	/* Macro to locate a 2D pointer */
		#define	ptr2d(p, x, y, pitch)		((p) + (x) + (y) * (pitch))

	/* Macro to copy between 2 2d pointers (pa<-pb) */
		#define	ptr2dCpy(cols, rows, Ta, pa, sa, Tb, pb, sb, cmp, pc, lmt, h)										\
											do{	Ta *a = (Ta*)(pa); Tb *b = (Tb*)(pb), *c = (Tb*)(pc), v;			\
												SIGN32 ri, ci;														\
												for(ri = 0; ri < rows; ri ++, a += (sa), b += (sb))					\
													for(ci = 0; ci < cols; ci ++) {									\
														v = b[ci]; if(cmp) v += *c ++;								\
														HistogramClp(h, v, lmt); a[ci] = (Ta)Clp(v, lmt);			\
															}														\
																}while(0)

	/* Macro to copy region by 2d modulo tiling */
		#define	mod2dCpy(T, pa, xa, ya, sa, pb, xb, yb, sb)															\
											do{	T *a = (T*)(pa), *b; SIGN32 ai, aj, bi, bj;							\
												for(ai = bi = 0; ai < (ya); ai ++) {								\
													if(bi == 0) b = (Tb*)(pb);										\
													for(aj = bj = 0; aj < (xa); aj ++, ModInc(bj, 1, xb))			\
														a[aj] = b[bj];												\
													ModInc(bi, 1, yb); a += (sa); b += (sb);						\
															}														\
																}while(0)
	/* Macro to calculate coding efficiency score */
		#define	EfficiencyScore(area, fps, bps, psnr)																\
											(100 * .25 * (area) * (fps) / (bps) *									\
																pow(2., ((psnr) - 40.) / (10. * log10(2.))))
/**	ENDOFSECTION
 */

#ifdef	__cplusplus
	}
#endif



/**	MULTIMEDIA
 */
#endif

/**	ENDOFFILE: multimedia.h *******************************************************************************************
 */

