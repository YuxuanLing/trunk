/**********************************************************************************************************************
*	$Id$
*
*	DESCRIPTION:
*	Collection of macro/function declarations and template implementations of useful math operations.
*
**********************************************************************************************************************/

#ifndef	MATH_OPS
#define	MATH_OPS					"        MATH_OPS >>>    "
/**	MATH_OPS
 */

#include	"cutil.h"



#ifdef	__cplusplus
	extern	"C"
	{
#endif

/**	SECTION - miscellaneous macros/functions implemented in "src/cm_util.c"
 */
	#define	__math_dft	INFINITY32

	/* Pixel bits depth expansion, e.g. PelExp(pel, 8, 2) will expand from 8b to 10b */
	#define	PelExp(pel, b, eb)		(((pel) << (eb)) + ((pel) >> ((b) - (eb))))
	#define	UniExp(pel, b, e)		(((b) < (e)) ? PelExp(pel, b, (e) - (b)) : UNSGDES(pel, (b) - (e)))

	/* Map [0,255] to [0,256] */
	#define	Map255to256(pel)							PelExp(pel, 7, 0)

	/* Clip to 0~255, or to -128~127 then add 128 */
	extern	UNSG8	*Clp255, *Clp127Add128;

	/* Clip to -128~127, or to 0~255 then sub 128 */
	extern	SIGN8	*Clp127, *Clp255Sub128;

	/* Commonly used clipping methods */
	#define	Clp(pel, lmt)								(	((lmt) == __math_dft)	?	(pel)					:	\
															((lmt) == 255)		?	Clp255[(SIGN32)(pel)]		:	\
															((lmt) == 127)		?	Clp255Sub128[(SIGN32)(pel)]	:	\
															((lmt) == 0)		?	Clp127Add128[(SIGN32)(pel)]	:	\
															((lmt) == - 128)	?	Clp127[(SIGN32)(pel)]		:	\
															((lmt) >= 0)		?	SATURATE(pel, 0, lmt)		:	\
																				SATURATE(pel, lmt, - (lmt) - 1)		\
																		)
	/* Commonly used clipping histogram methods */
	#define	HistogramClp(tbl, pel, lmt)					do{	if((lmt) == __math_dft) break;							\
															else if(((lmt) == 255) || ((lmt) == 127))				\
																HistogramSat(tbl, (SIGN32)(pel), 0, 255);			\
															else if(((lmt) == 0) || ((lmt) == - 128))				\
																HistogramSat(tbl, (SIGN32)(pel), - 128, 127);		\
															else if((lmt) >= 0) HistogramSat(tbl, pel, 0, lmt);		\
															else HistogramSat(tbl, pel, lmt, - (lmt) - 1);			\
																		}while(0)
	/* Commonly used reverse-abs and clipping histogram methods */
	#define	HistogramABS(tbl, abs, pel, lmt)			do{	if(lmt)													\
																if((pel) < 0) {										\
																	HistogramTbl(tbl, (abs) > - (lmt) ? 1 : 0);		\
																	(abs) = - MIN(abs, - (lmt));					\
																			}										\
																else {												\
																	HistogramTbl(tbl, (abs) > - (lmt) - 1 ? 2 : 0);	\
																	(abs) = MIN(abs, - (lmt) - 1);					\
																			}										\
															else if((pel) < 0) (abs) = - (abs);						\
																		}while(0)
	/* Calculate SAD with constant */
	#define	ABSN(sum, a, x, n)							do{	SIGN32 in, d;											\
															for(in = 0; in < (n); in ++) {							\
																d = (a)[in]; d -= (x); (sum) += ABS(d);				\
																	}												\
																		}while(0)
	/* Calculate SAD */
	#define	SADN(sad, a, b, n)							do{	SIGN32 in, d;											\
															for(in = 0; in < (n); in ++) {							\
																d = (a)[in]; d -= (b)[in]; (sad) += ABS(d);			\
																	}												\
																		}while(0)
	/* Calculate SSD */
	#define	SSDN(ssd, a, b, n)							do{	SIGN32 in, d;											\
															for(in = 0; in < (n); in ++) {							\
																d = (a)[in]; d -= (b)[in]; (ssd) += d * d;			\
																	}												\
																		}while(0)

	/******************************************************************************************************************
	*	Function: GreatCommDiv
	*	Description: Calculate greatest common divisor.
	*	Return:			UNSG32						-	GCD
	******************************************************************************************************************/
	UNSG32	GreatCommDiv(
						SIGN32	s32a,					/*!	1st integer !*/
						SIGN32	s32b					/*!	2nd integer !*/
						);

	/******************************************************************************************************************
	*	Function: LeastCommMtp
	*	Description: Calculate least common multiple.
	*	Return:			UNSG32						-	LCM
	******************************************************************************************************************/
	UNSG32	LeastCommMtp(
						SIGN32	s32a,					/*!	1st integer !*/
						SIGN32	s32b					/*!	2nd integer !*/
						);

	/******************************************************************************************************************
	*	Function: CoPrime
	*	Description: Reduce 2 integers to co-prime numbers.
	******************************************************************************************************************/
	void	CoPrime(	SIGN32	*ps32a,					/*!	1st integer !*/
						SIGN32	*ps32b					/*!	2nd integer !*/
						);
/**	ENDOFSECTION
 */



/**	SECTION - 3x3 matrix operations
 */
	/* Calculate 2x2 determinant */
		#define	DET2x2(a, b, c, d)			((a) * (d) - (b) * (c))

	/* Calculate inverse of 2x2 matrix */
		#define	INV2x2(t, m)				do{	t[0][0] =   m[1][1]; t[0][1] = - m[1][0];							\
												t[1][0] = - m[0][1]; t[1][1] =   m[0][0];							\
																}while(0)
	/* Calculate 3x3 determinant */
		#define	DET3x3(m)					(	+ m[0][0] * DET2x2(m[1][1], m[1][2], m[2][1], m[2][2])				\
													- m[1][0] * DET2x2(m[0][1], m[0][2], m[2][1], m[2][2])			\
														+ m[2][0] * DET2x2(m[0][1], m[0][2], m[1][1], m[1][2])		\
																)
	/* Calculate inverse of 3x3 matrix */
		#define	INV3x3(t, m)				do{	t[0][0] = + DET2x2(m[1][1], m[1][2], m[2][1], m[2][2]);				\
													t[0][1] = - DET2x2(m[0][1], m[0][2], m[2][1], m[2][2]);			\
														t[0][2] = + DET2x2(m[0][1], m[0][2], m[1][1], m[1][2]);		\
												t[1][0] = - DET2x2(m[1][0], m[1][2], m[2][0], m[2][2]);				\
													t[1][1] = + DET2x2(m[0][0], m[0][2], m[2][0], m[2][2]);			\
														t[1][2] = - DET2x2(m[0][0], m[0][2], m[1][0], m[1][2]);		\
												t[2][0] = + DET2x2(m[1][0], m[1][1], m[2][0], m[2][1]);				\
													t[2][1] = - DET2x2(m[0][0], m[0][1], m[2][0], m[2][1]);			\
														t[2][2] = + DET2x2(m[0][0], m[0][1], m[1][0], m[1][1]);		\
																}while(0)
	/* 3x1 vector add */
		#define	SUM3x1(t, m, n, j)			do{	t[0][j] = m[0][j] + n[0][j];										\
												t[1][j] = m[1][j] + n[1][j];										\
												t[2][j] = m[2][j] + n[2][j];										\
																}while(0)
	/* 3x3 matrix add */
		#define	SUM3x3(t, m, n)				do{	SUM3x1(t, m, n, 0); SUM3x1(t, m, n, 1); SUM3x1(t, m, n, 2); }while(0)

	/* 3x1 vector substraction */
		#define	DIF3x1(t, m, n, j)			do{	t[0][j] = m[0][j] - n[0][j];										\
												t[1][j] = m[1][j] - n[1][j];										\
												t[2][j] = m[2][j] - n[2][j];										\
																}while(0)
	/* 3x3 matrix substraction */
		#define	DIF3x3(t, m, n)				do{	DIF3x1(t, m, n, 0); DIF3x1(t, m, n, 1); DIF3x1(t, m, n, 2); }while(0)

	/* Multiply a 1x3 vector with a 3x1 vector */
		#define	CNV3ij(m, n, i, j)			(m[i][0] * n[0][j] + m[i][1] * n[1][j] + m[i][2] * n[2][j])

	/* Multiply a 3x3 matrix with a 3x1 vector */
		#define	CNV3x1(t, m, n, j)			do{	t[0][j] = CNV3ij(m, n, 0, j);										\
												t[1][j] = CNV3ij(m, n, 1, j);										\
												t[2][j] = CNV3ij(m, n, 2, j);										\
																}while(0)
	/* Multiply a 3x3 matrix with a 3x3 matrix */
		#define	CNV3x3(t, m, n)				do{	CNV3x1(t, m, n, 0); CNV3x1(t, m, n, 1); CNV3x1(t, m, n, 2); }while(0)

	/* Add a 3x1 vector with a variable */
		#define	ADD3x1(t, m, x, j)			do{	t[0][j] = m[0][j] + (x);											\
												t[1][j] = m[1][j] + (x);											\
												t[2][j] = m[2][j] + (x);											\
																}while(0)
	/* Add a 3x3 matrix with a variable */
		#define	ADD3x3(t, m, x)				do{	ADD3x1(t, m, x, 0); ADD3x1(t, m, x, 1); ADD3x1(t, m, x, 2); }while(0)

	/* Multiply a 3x1 vector with a variable */
		#define	MUL3x1(t, m, x, j)			do{	t[0][j] = m[0][j] * (x);											\
												t[1][j] = m[1][j] * (x);											\
												t[2][j] = m[2][j] * (x);											\
																}while(0)
	/* Multiply a 3x3 matrix with a variable */
		#define	MUL3x3(t, m, x)				do{	MUL3x1(t, m, x, 0); MUL3x1(t, m, x, 1); MUL3x1(t, m, x, 2); }while(0)

	/* Divide a 3x1 integer vector with a variable */
		#define	DIV3x1(t, m, k, j)			do{	t[0][j] = SIGNDIV(m[0][j], k);										\
												t[1][j] = SIGNDIV(m[1][j], k);										\
												t[2][j] = SIGNDIV(m[2][j], k);										\
																}while(0)
	/* Divide a 3x3 integer matrix with a variable */
		#define	DIV3x3(t, m, k)				do{	DIV3x1(t, m, k, 0); DIV3x1(t, m, k, 1); DIV3x1(t, m, k, 2); }while(0)

	/* Scale a 3x1 integer vector with a variable */
		#define	SCL3x1(t, m, k, j)			do{	t[0][j] = UNSGSCL(m[0][j], k);										\
												t[1][j] = UNSGSCL(m[1][j], k);										\
												t[2][j] = UNSGSCL(m[2][j], k);										\
																}while(0)
	/* Scale a 3x3 integer matrix with a variable */
		#define	SCL3x3(t, m, k)				do{	SCL3x1(t, m, k, 0); SCL3x1(t, m, k, 1); SCL3x1(t, m, k, 2); }while(0)

	/* Set a 3x1 vector from an array */
		#define	SET3x1(m, v, d, j)			do{	m[0][j] = v[d ? j : j * 3];											\
												m[1][j] = v[d ? j + 3 : j * 3 + 1];									\
												m[2][j] = v[d ? j + 6 : j * 3 + 2];									\
																}while(0)
	/* Set a 3x3 matrix from an array */
		#define	SET3x3(m, v, d)				do{	SET3x1(m, v, d, 0); SET3x1(m, v, d, 1); SET3x1(m, v, d, 2); }while(0)

	/* Get an array from a 3x1 vector */
		#define	GET3x1(m, v, d, j)			do{	v[d ? j : j * 3] = m[0][j];											\
												v[d ? j + 3 : j * 3 + 1] = m[1][j];									\
												v[d ? j + 6 : j * 3 + 2] = m[2][j];									\
																}while(0)
	/* Get an array from a 3x3 matrix */
		#define	GET3x3(m, v, d)				do{	GET3x1(m, v, d, 0); GET3x1(m, v, d, 1); GET3x1(m, v, d, 2); }while(0)

	/* 3x3 matrix transpose */
		#define	TSP3x3(t, m)				do{	t[0][0] = m[0][0]; t[0][1] = m[1][0]; t[0][2] = m[2][0];			\
												t[1][0] = m[0][1]; t[1][1] = m[1][1]; t[1][2] = m[2][1];			\
												t[2][0] = m[0][2]; t[2][1] = m[1][2]; t[2][2] = m[2][2];			\
																}while(0)
	/* Calculate inverse of 3x3 matrix (integer) and normalize to the given bits */
		#define	INV3x3NORM(t, tb, m, mb)	do{	SIGN32 n[3][3], det;												\
												SCL3x3(n, m, tb - mb); det = UNSGDES(DET3x3(n), 4); INV3x3(t, n);	\
												SCL3x3(n, t, tb * 2 - 4); DIV3x3(t, n, det);						\
																}while(0)
/**	ENDOFSECTION
 */

#ifdef	__cplusplus
	}
#endif



#ifdef	__cplusplus
/**	__cplusplus
 */

/**	SECTION - template functions for segmented multiplication, PSNR, vector convolutions and matrix operations
 */
	/* Multiply-shift with a bit field */
	#define	_fldMtp(x, c, i, mask, roof)				UNIVSCL((x) * GetField(c, i, (mask) << i), i - (roof))

	/******************************************************************************************************************
	*	Function: _segMtp
	*	Description: Segmented multiplication.
	******************************************************************************************************************/
	template <class T, SIGN32 b, SIGN32 n>
	INLINE	T		_segMtp(
						T		x,						/*!	Input !*/
						T		c,						/*!	Multiplication factor !*/
						SIGN32	s32roof					/*!	Descaling bits to avoid overflow !*/
						)
	{
	SIGN32	i = 0, j;
	T	mtp = 0, mask = NSETMASK(b - 1, 0);
	for(j = 0; j < n; j ++, i += b) mtp += _fldMtp(x, c, i, mask, s32roof);

	return mtp;
	/**	ENDOFFUNCTION: _segMtp **/
	}



	/******************************************************************************************************************
	*	DEFINITION - universal type process template
	******************************************************************************************************************/
	template <class T> INLINE void _tout(T v)			{ xdbg("%9d", v); }
	template <> INLINE void _tout<UNSG32>(UNSG32 v)		{ xdbg("%9X", v); }
	template <> INLINE void _tout<REAL32>(REAL32 v)		{ xdbg("%9.3f", v); }
	template <> INLINE void _tout<REAL64>(REAL64 v)		{ xdbg("%9.3f", v); }

	template <class T> INLINE T SCL(T v, SIGN32 scl)					{ return UNIVSCL(v, scl); }
	template <> INLINE REAL32 SCL<REAL32>(REAL32 v, SIGN32 scl)			{ return REALSCL(v, scl); }
	template <> INLINE REAL64 SCL<REAL64>(REAL64 v, SIGN32 scl)			{ return REALSCL(v, scl); }

	template <class T> INLINE T DIV(T v, T div)							{ return SIGNDIV(v, div); }
	template <> INLINE REAL32 DIV<REAL32>(REAL32 v, REAL32 div)			{ return v / div; }
	template <> INLINE REAL64 DIV<REAL64>(REAL64 v, REAL64 div)			{ return v / div; }

	template <class T> INLINE T AVG(T x1, T x2, T a, T sum)				{ return (sum == 256)
																				? Average256(x1, x2, a)
																				: Average(x1, x2, a, sum); }
	template <> INLINE REAL32 AVG(REAL32 x1, REAL32 x2, REAL32 a, REAL32 sum)
																		{ return (x1 * a + x2 * (sum - a)) / sum; }
	template <> INLINE REAL64 AVG(REAL64 x1, REAL64 x2, REAL64 a, REAL64 sum)
																		{ return (x1 * a + x2 * (sum - a)) / sum; }
#ifndef	__CODE_LINK__
	/*!	@brief
	*	Calculate PSNR from SSD.
	*	@return			REAL64						:	PSNR, 999.999 if identical
	******************************************************************************************************************/
	static	REAL64	PSNR(
						SIGN32	area,				///<i>	picture area size
						REAL64	ssd,				///<i>	sum of squared distance
						SIGN32	absdmax				///<i>	maximum pixel difference
								= 255
						)
	{
	if((ssd /= area) < .0000001) return 999.999;

	return 10. * log10(absdmax * absdmax / ssd);
	//*	ENDOFFUNCTION: PSNR
	}

	/*!	@brief
	*	Calculate PSNR for 2 pictures.
	*	@param[in]		Ta		pixel type of picture.a
	*	@param[in]		Tb		pixel type of picture.b
	*	@return			REAL64						:	PSNR, 999.999 if identical
	******************************************************************************************************************/
	template <class Ta, class Tb>
	REAL64	PSNR(		SIGN32	width,				///<i>	horizontal size
						SIGN32	height,				///<i>	vertical size
						Ta		*pa,				///<i>	1st picture
						SIGN32	apitch,				///<i>	1st picture stride in pixels
						Tb		*pb,				///<i>	2nd picture
						SIGN32	bpitch,				///<i>	2nd picture stride in pixels
						SIGN32	absdmax				///<i>	maximum pixel difference
								= 255
						)
	{
	SIGN32 i; REAL64 err = 0;
	for(i = 0; i < height; i ++, pa += apitch, pb += bpitch)
		SSDN(err, pa, pb, width);

	return PSNR(width * height, err, absdmax);
	//*	ENDOFFUNCTION: PSNR
	}

	/*!	@def	PSNRYUV
	*	Calculate average PSNR for YUV.
	*	@param[in]		area	picture area size
	*	@param[in]		ssdY	SSD of Y
	*	@param[in]		ssdU	SSD of U
	*	@param[in]		ssdV	SSD of V
	*	@return			floating					:	averaged PSNR
	******************************************************************************************************************/
	#define	PSNRYUV(area, ssdY, ssdU, ssdV)			\
			PSNR((area) * 5 / 4, ssdY + (ssdU + ssdV) / 2)

	/*!	@def	CES
	*	Compression-Efficiency-Score.
	*	@param[in]		area	picture area size
	*	@param[in]		fps		video frame rate
	*	@param[in]		bps		video bit-rate
	*	@param[in]		psnr	video PSNR
	*	@return			floating					:	compression efficiency score
	******************************************************************************************************************/
	#define CES(area, fps, bps, psnr)				\
			(100 * 0.25 * area * fps / bps * pow(2, (psnr - 40) / (10 * log10(2.))))
#endif



	/******************************************************************************************************************
	*	Function: _errupdate
	*	Description: Update total and max L1/L2 errors.
	******************************************************************************************************************/
	template <class T, class Te>
	void	_errupdate(	T		*pa,					/*!	Input A !*/
						T		*pb,					/*!	Input B !*/
						Te		*pe,					/*!	Non-zero to store current errors !*/
						Te		*perr,					/*!	Non-zero to store accumulated errors !*/
						Te		*pabs,					/*!	Non-zero to store max absolute errors !*/
						Te		*perr2,					/*!	Non-zero to store accumulated L2 errors !*/
						Te		&errMax,				/*!	Non-zero to store all time max error !*/
						Te		&err2Max,				/*!	Non-zero to store all time max L2 error !*/
						SIGN32	s32n					/*!	Number of samples !*/
						)
	{
	SIGN32	i;
	Te e, e2 = 0;
	for(i = 0; i < s32n; i ++) {
		e = (Te)(pa[i]) - (Te)(pb[i]); if(pe) pe[i] = e; if(perr) perr[i] += e;
		e = ABS(e); if(pabs) pabs[i] = MAX(pabs[i], e); errMax = MAX(errMax, e);
		e *= e; if(perr2) perr2[i] += e; e2 += e;
			}
	err2Max = MAX(err2Max, e2);
	/**	ENDOFFUNCTION: _errupdate **/
	}



	/* Typical (while not always!) pair of 'T' & 'Tcnv' */
	#define	__SIGN8										SIGN8 , SIGN32
	#define	__UNSG8										UNSG8 , SIGN32
	#define	__SIGN16									SIGN16, SIGN32
	#define	__UNSG16									UNSG16, SIGN32
	#define	__SIGN32									SIGN32, SIGN32
	#define	__UNSG32									UNSG32, SIGN32
	#define	__REAL32									REAL32, REAL32
	#define	__REAL64									REAL64, REAL64

	#define	__NO_SCL									0, 0, NULL

	/******************************************************************************************************************
	*	Function: _cnv
	*	Description: Template of vector convolution operation.
	*		Note: 'T' is specified temporal storage type.
	*	Return:			Tcnv						-	Any type of specified convolution result
	******************************************************************************************************************/
	template <SIGN32 lmt, class T, class Tcnv, class Tlft, class Trht>
	INLINE	Tcnv	_cnv(
						SIGN32	s32vec,					/*!	Vector size !*/
						Tlft	*plft,					/*!	Input: left vector !*/
						SIGN32	s32lft,					/*!	Input: left vector incremental step !*/
						Trht	*prht,					/*!	Input: right vector !*/
						SIGN32	s32rht,					/*!	Input: right vector incremental step !*/
						T		rnd,					/*!	Result rounding !*/
						SIGN32	s32des,					/*!	Result de-scaling !*/
						SIGN32	*htbl					/*!	Non-zero for histogram clipping table !*/
						)
	{
	while(s32vec -- > 0) {
		rnd += (T)*plft * (T)*prht; plft += s32lft; prht += s32rht;
			}
	rnd = SCL<T>(rnd, - s32des); HistogramClp(htbl, rnd, lmt); return (Tcnv)Clp(rnd, lmt);
	/**	ENDOFFUNCTION: _cnv **/
	}



	/******************************************************************************************************************
	*	Function: _cnvN
	*	Description: Template of generic convolution of 2 sets of vectors.
	******************************************************************************************************************/
	template <SIGN32 lmt, class T, class Tcnv, class Tlft, class Trht>
	INLINE	void	_cnvN(
						SIGN32	s32vec,					/*!	Vector size !*/
						Tcnv	*pcnv,					/*!	Output result vector array !*/
						SIGN32	s32cnvinc,				/*!	Output result vector pointer incremental !*/

						Tlft	**ppl,					/*!	Input: left vector array !*/
						SIGN32	s32lft,					/*!	Input: left vector incremental step !*/
						SIGN32	s32lftinc,				/*!	Input: left vector pointer incremental
															__math_dft to use vector array
															!*/
						Trht	**ppr,					/*!	Input: right vector array !*/
						SIGN32	s32rht,					/*!	Input: right vector incremental step !*/
						SIGN32	s32rhtinc,				/*!	Input: right vector pointer incremental
															__math_dft to use vector array
															!*/
						SIGN32	s32N,					/*!	Number of vectors !*/
						T		rnd,					/*!	Result rounding !*/
						SIGN32	s32des,					/*!	Result de-scaling !*/
						SIGN32	*htbl					/*!	Non-zero for histogram clipping table !*/
						)
	{
	Tlft *plft = *ppl; Trht *prht = *ppr;
	while(s32N -- > 0) {
		*pcnv = _cnv<lmt, T, Tcnv, Tlft, Trht>
				(s32vec, plft, s32lft, prht, s32rht, rnd, s32des, htbl);
		if(s32lftinc != __math_dft) plft += s32lftinc; else plft = *(++ ppl);
		if(s32rhtinc != __math_dft) prht += s32rhtinc; else prht = *(++ ppr);
		pcnv += s32cnvinc;
			}
	/**	ENDOFFUNCTION: _cnvN **/
	}



	/******************************************************************************************************************
	*	Function: _cnvNxM
	*	Description: Template of generic convolution of 2-dimensional sets of vectors.
	******************************************************************************************************************/
	template <SIGN32 lmt, class T, class Tcnv, class Tlft, class Trht>
	INLINE	void	_cnvNxM(
						SIGN32	s32vec,					/*!	Vector size !*/
						Tcnv	*pcnv,					/*!	Output result vector 2-dimensional array !*/
						SIGN32	s32cnvinc,				/*!	Output result vector pointer incremental !*/
						SIGN32	s32cnvpitch,			/*!	Output result vector pointer line stride !*/

						Tlft	**ppl,					/*!	Input: left vector 2-dimensional array !*/
						SIGN32	s32lft,					/*!	Input: left vector incremental step !*/
						SIGN32	s32lftinc,				/*!	Input: left vector pointer incremental
															__math_dft to use vector array
															!*/
						SIGN32	s32lftpitch,			/*!	Input: left vector pointer/array line stride !*/

						Trht	**ppr,					/*!	Input: right vector 2-dimensional array !*/
						SIGN32	s32rht,					/*!	Input: right vector incremental step !*/
						SIGN32	s32rhtinc,				/*!	Input: right vector pointer incremental
															__math_dft to use vector array
															!*/
						SIGN32	s32rhtpitch,			/*!	Input: right vector pointer/array line stride !*/

						SIGN32	s32N,					/*!	Number of vectors in 1st dimension !*/
						SIGN32	s32M,					/*!	Number of vectors in 2nd dimension !*/
						T		rnd,					/*!	Result rounding !*/
						SIGN32	s32des,					/*!	Result de-scaling !*/
						SIGN32	*htbl					/*!	Non-zero for histogram clipping table !*/
						)
	{
	Tlft *plft = *ppl; Trht *prht = *ppr;
	if(s32lftinc != __math_dft) ppl = &plft;
	if(s32rhtinc != __math_dft) ppr = &prht;

	while(s32M -- > 0) {
		_cnvN<lmt, T, Tcnv, Tlft, Trht>
			(s32vec, pcnv, s32cnvinc,
				ppl, s32lft, s32lftinc, ppr, s32rht, s32rhtinc,
					s32N, rnd, s32des, htbl);
		if(s32lftinc != __math_dft) (*ppl) += s32lftpitch; else ppl += s32lftpitch;
		if(s32rhtinc != __math_dft) (*ppr) += s32rhtpitch; else ppr += s32rhtpitch;
		pcnv += s32cnvpitch;
			}
	/**	ENDOFFUNCTION: _cnvNxM **/
	}



	/******************************************************************************************************************
	*	Function: _cnv_mtx
	*	Description: NxM = KxM * NxK (generic matrix convolution multiplication with optional de-scaling).
	******************************************************************************************************************/
	template <SIGN32 lmt, class T, class Tnm, class Tkm, class Tnk>
	INLINE	void	_cnv_mtx(
						SIGN32	s32N,					/*!	Width of output and right input matrix !*/
						SIGN32	s32M,					/*!	Height of output and left input matrix !*/
						SIGN32	s32K,					/*!	Matrix width/height for left/right input !*/

						Tnm		*pnm,					/*!	Output matrix !*/
						SIGN32	s32nmpitch,				/*!	Output matrix line stride !*/
						Tkm		*pkm,					/*!	Input: left matrix !*/
						SIGN32	s32kmpitch,				/*!	Input: left matrix line stride !*/
						Tnk		*pnk,					/*!	Input: right matrix !*/
						SIGN32	s32nkpitch,				/*!	Input: right matrix line stride !*/

						T		rnd,					/*!	Result rounding !*/
						SIGN32	s32des,					/*!	Result de-scaling !*/
						SIGN32	*htbl					/*!	Non-zero for histogram clipping table !*/
						)
	{
	_cnvNxM<lmt, T, Tnm, Tkm, Tnk>
		(s32K, pnm, 1, s32nmpitch, &pkm, 1, 0, s32kmpitch, &pnk, s32nkpitch, 1, 0, s32N, s32M, rnd, s32des, htbl);
	/**	ENDOFFUNCTION: _cnv_mtx **/
	}

	/* NxM = KxM * NxK (any unified type matrix multiplication) */
	template <class T>
	INLINE	void	_mtx_cnv(SIGN32 N, SIGN32 M, SIGN32 K, T *pnm, T *pkm, T *pnk)
														{	_cnv_mtx<__math_dft, T, T, T, T>
																(N, M, K, pnm, N, pkm, K, pnk, N, 0, 0, NULL);
																		}



	/******************************************************************************************************************
	*	CONSTANTS - Operation modes
	******************************************************************************************************************/
	typedef enum {

	__opmd_add			= 0,							/* Operation: a + b */
	__opmd_sub			= 1,							/* Operation: a - b */
	__opmd_mul			= 2,							/* Operation: a * b */
	__opmd_div			= 3,							/* Operation: a / b */

	__opmd				= 0x0F							/* Mask for operation */

	} EMath_OperationMode;

	/* Get value from array or non-array variable/constant */
	template <class T, class pT> INLINE T _opGet(pT p, SIGN32 i)				{ return (T)(p[i]); }
	template <> INLINE UNSG8  _opGet<UNSG8 , UNSG8 >(UNSG8  v, SIGN32 i)		{ return v; }
	template <> INLINE SIGN8  _opGet<SIGN8 , SIGN8 >(SIGN8  v, SIGN32 i)		{ return v; }
	template <> INLINE UNSG16 _opGet<UNSG16, UNSG16>(UNSG16 v, SIGN32 i)		{ return v; }
	template <> INLINE SIGN16 _opGet<SIGN16, SIGN16>(SIGN16 v, SIGN32 i)		{ return v; }
	template <> INLINE UNSG32 _opGet<UNSG32, UNSG32>(UNSG32 v, SIGN32 i)		{ return v; }
	template <> INLINE SIGN32 _opGet<SIGN32, SIGN32>(SIGN32 v, SIGN32 i)		{ return v; }
	template <> INLINE REAL32 _opGet<REAL32, REAL32>(REAL32 v, SIGN32 i)		{ return v; }
	template <> INLINE REAL64 _opGet<REAL64, REAL64>(REAL64 v, SIGN32 i)		{ return v; }

	/* Template of generic (+,-,*,/) operation */
	template <SIGN32 op, class T, class pTa, class pTb>
	INLINE	T		_opCal(pTa pa, SIGN32 ia, pTb pb, SIGN32 ib)
														{	T left = _opGet<T, pTa> (pa, ia);
															T right = _opGet<T, pTb> (pb, ib);
															switch(op & __opmd) {
																case __opmd_add	: return left + right;
																case __opmd_sub	: return left - right;
																case __opmd_mul	: return left * right;
																case __opmd_div	: return DIV<T>(left, right);
																default			: return 0;
																	}
																		}

	/******************************************************************************************************************
	*	Function: _mtx_opt
	*	Description: Template of matrix (+,-,*,/) operation.
	******************************************************************************************************************/
	template <SIGN32 lmt, class T, class Tc, class pTa, class pTb, SIGN32 op>
	INLINE	void	_mtx_opt(
						Tc		*p,						/*!	Output destination matrix !*/
						SIGN32	s32pitch,				/*!	Output destination matrix line stride !*/
						pTa		pa,						/*!	Input: left matrix !*/
						pTb		pb,						/*!	Input: right matrix !*/

						SIGN32	s32N,					/*!	NxM matrix width !*/
						SIGN32	s32M,					/*!	NxM matrix height !*/
						SIGN32	s32ainc,				/*!	Matrix horizontal incremental of left input !*/
						SIGN32	s32apitch,				/*!	Matrix line stride of left input !*/
						SIGN32	s32binc,				/*!	Matrix horizontal incremental of right input !*/
						SIGN32	s32bpitch,				/*!	Matrix line stride of right input !*/

						T		rnd,					/*!	Result rounding !*/
						SIGN32	s32des,					/*!	Result de-scaling !*/
						SIGN32	*htbl					/*!	Non-zero for histogram clipping table !*/
						)
	{
	SIGN32	i, a, b, s32a = 0, s32b = 0;
	while(s32M -- > 0) {
		for(i = 0, a = s32a, b = s32b; i < s32N; i ++, a += s32ainc, b += s32binc) {
			T c = rnd + _opCal<op, T, pTa, pTb>(pa, a, pb, b);
			c = SCL<T>(c, - s32des);
			HistogramClp(htbl, c, lmt);
			p[i] = (Tc)Clp(c, lmt);
				}
		p += s32pitch; s32a += s32apitch; s32b += s32bpitch;
			}
	/**	ENDOFFUNCTION: _mtx_opt **/
	}

	/* '_opt_mtx' template argument shortcuts */
	#define	__mtx_add_fix(Ta, Tb)						Ta *, Tb  , __opmd_add
	#define	__mtx_add_mtx(Ta, Tb)						Ta *, Tb *, __opmd_add
	#define	__fix_sub_mtx(Ta, Tb)						Ta  , Tb *, __opmd_sub
	#define	__mtx_sub_fix(Ta, Tb)						Ta *, Tb  , __opmd_sub
	#define	__mtx_sub_mtx(Ta, Tb)						Ta *, Tb *, __opmd_sub
	#define	__mtx_mul_fix(Ta, Tb)						Ta *, Tb  , __opmd_mul
	#define	__mtx_mul_mtx(Ta, Tb)						Ta *, Tb *, __opmd_mul
	#define	__fix_div_mtx(Ta, Tb)						Ta  , Tb *, __opmd_div
	#define	__mtx_div_fix(Ta, Tb)						Ta *, Tb  , __opmd_div
	#define	__mtx_div_mtx(Ta, Tb)						Ta *, Tb *, __opmd_div
	#define	__set_fix(Ta, T)							T, T, Ta  , T , __opmd_add
	#define	__set_mtx(Ta, T)							T, T, Ta *, T , __opmd_add

	/* '_opt_mtx' funtion argument shortcuts */
	#define	__NxM(pitch)								1, pitch
	#define	__1xM(pitch)								0, pitch
	#define	__Nx1(pitch)								1, 0
	#define	__NxM_NxM(n, m)								n, m, __NxM(n), __NxM(n)
	#define	__1xM_NxM(n, m)								n, m, __1xM(n), __NxM(n)
	#define	__Nx1_NxM(n, m)								n, m, __Nx1(n), __NxM(n)
	#define	__NxM_1xM(n, m)								n, m, __NxM(n), __1xM(n)
	#define	__1xM_1xM(n, m)								n, m, __1xM(n), __1xM(n)
	#define	__Nx1_1xM(n, m)								n, m, __Nx1(n), __1xM(n)
	#define	__NxM_Nx1(n, m)								n, m, __NxM(n), __Nx1(n)
	#define	__1xM_Nx1(n, m)								n, m, __1xM(n), __Nx1(n)
	#define	__Nx1_Nx1(n, m)								n, m, __Nx1(n), __Nx1(n)

	/* Matrix scaling */
	template <class Ta, class Tb>
	INLINE	void	_mtx_scl(SIGN32 N, SIGN32 M, Ta *pa, SIGN32 apitch, Tb *pb, SIGN32 bpitch, Ta rnd, SIGN32 scl)
														{	_mtx_opt<__math_dft, __set_mtx(Tb, Ta)>
																(pa, apitch, pb, 0, N, M, __NxM(bpitch), __NxM(0),
																	rnd, - scl, NULL);
																		}
	/* Matrix copy */
	template <class Ta, class Tb>
	INLINE	void	_mtx_cpy(SIGN32 N, SIGN32 M, Ta *pa, SIGN32 apitch, Tb *pb, SIGN32 bpitch)
														{	_mtx_scl<Ta, Tb>(N, M, pa, apitch, pb, bpitch, 0, 0);
																		}
	/* Matrix assign to a value */
	template <class T>
	INLINE	void	_mtx_set(SIGN32 N, SIGN32 M, T *p, SIGN32 pitch, T v)
														{	_mtx_opt<__math_dft, __set_fix(T, T)>
																(p, pitch, v, 0, __NxM_NxM(N, M), __NO_SCL);
																		}



	/******************************************************************************************************************
	*	Function: _mtx_tsp
	*	Description: Template to transpose a matrix (MxN <- NxM).
	******************************************************************************************************************/
	template <SIGN32 lmt, class T, class Ttsp, class Torg>
	INLINE	void	_mtx_tsp(
						SIGN32	s32N,					/*!	NxM matrix width !*/
						SIGN32	s32M,					/*!	NxM matrix height !*/

						Ttsp	*ptsp,					/*!	Transposed matrix (MxN) !*/
						SIGN32	s32tspinc,				/*!	Transposed matrix horizontal incremental !*/
						SIGN32	s32tsppitch,			/*!	Transposed matrix line stride !*/
						Torg	*porg,					/*!	Original matrix (NxM) !*/
						SIGN32	s32orginc,				/*!	Original matrix horizontal incremental !*/
						SIGN32	s32orgpitch,			/*!	Original matrix line stride !*/

						T		rnd,					/*!	Result rounding !*/
						SIGN32	s32des,					/*!	Result de-scaling !*/
						SIGN32	*htbl					/*!	Non-zero for histogram clipping table !*/
						)
	{
	SIGN32	i, x, y;
	while(s32M -- > 0) {
		for(i = 0, x = y = 0; i < s32N; i ++, x += s32orginc, y += s32tsppitch) {
			T t = rnd + porg[x];
			t = SCL<T>(t, - s32des); HistogramClp(htbl, t, lmt); ptsp[y] = (Ttsp)Clp(t, lmt);
				}
		ptsp += s32tspinc; porg += s32orgpitch;
			}
	/**	ENDOFFUNCTION: _mtx_tsp **/
	}



	/******************************************************************************************************************
	*	Function: _mtx_det
	*	Description: Template to calulate KxK determinant recursively.
	*	Return:			T							-	KxK determinant
	******************************************************************************************************************/
	template <class T, class Tm>
	T		_mtx_det(	Tm		*p,						/*!	Matrix buffer !*/
						SIGN32	s32K,					/*!	KxK matrix !*/
						Tm		*ptmp					/*!	Temporal buffer:
															sizeof(Tm) * [K*(K-1)*(2K-1)/6]
															NULL to use default global buffer
															!*/
								= NULL
						)
	{
	if(s32K == 1) return (T)*p; s32K --;
	if(!ptmp) ptmp = (Tm*)garru32; Tm *pnxt = ptmp + s32K * s32K;
	T dk, det = 0;

	_mtx_cpy<Tm, Tm>(s32K, s32K, ptmp, s32K, p + s32K + 2, s32K + 1);
	for(SIGN32 i = 0; i <= s32K; i ++) {
		if(i > 0)
			_mtx_cpy<Tm, Tm>(1, s32K, ptmp + i - 1, s32K, p + s32K + i, s32K + 1);
		dk = (T)(p[i]) * _mtx_det<T, Tm>(ptmp, s32K, ptmp + s32K * s32K); if(i & 1) det -= dk; else det += dk;
			}
	return det;
	/**	ENDOFFUNCTION: _mtx_det **/
	}



	/******************************************************************************************************************
	*	Function: _mtx_inv
	*	Description: Template to inverse a KxK matrix and normalize to given bits.
	*	Return:			T							-	KxK determinant, or 0 if normalization disabled
	******************************************************************************************************************/
	template <SIGN32 lmt, class T, class Ti, class Tm>
	T		_mtx_inv(	Ti		*pinv,					/*!	Inversed matrix buffer !*/
						SIGN32	s32inormb,				/*!	Inversed matrix normalized bits !*/
						Tm		*pmtx,					/*!	Original matrix buffer !*/
						SIGN32	s32mnormb,				/*!	Original matrix normalized bits !*/
						SIGN32	s32K,					/*!	KxK matrix !*/
						T		detmax					/*!	Maximum safe determinant, 0 to disable !*/
								= 0,
						Tm		*ptmp					/*!	Temporal buffer:
															sizeof(Tm) * [K*K*(2K+3)/6] + sizof(T) * [K*K]
															NULL to use default global buffer
															!*/
								= NULL
						)
	{
	if(!ptmp) ptmp = (Tm*)garru32;
	T det, *p = (T*)ptmp; ptmp = (Tm*)(p + s32K * s32K);

	if(s32K == 1) *p = 1;
	else {
		Tm *pwin = ptmp + s32K * (s32K - 1), *pnxt = pwin + (s32K - 1) * (s32K - 1);
		_mtx_cpy<Tm, Tm>(s32K, s32K - 1, ptmp, s32K, pmtx + s32K, s32K);
		for(SIGN32 i = 0; i < s32K; i ++) {
			if(i > 0)
				_mtx_cpy<Tm, Tm>(s32K, 1, ptmp + (i - 1) * s32K, s32K, pmtx + (i - 1) * s32K, s32K);
			_mtx_cpy<Tm, Tm>(s32K - 1, s32K - 1, pwin, s32K - 1, ptmp + 1, s32K);
			for(SIGN32 j = 0; j < s32K; j ++) {
				if(j > 0)
					_mtx_cpy<Tm, Tm>(1, s32K - 1, pwin + j - 1, s32K - 1, ptmp + j - 1, s32K);
				det = _mtx_det<T, Tm>(pwin, s32K - 1, pnxt); p[j * s32K + i] = ((i + j) & 1) ? - det : det;
					}
						}
							}
	if((s32inormb += s32mnormb) <= 0)
		det = 0;
	else {
		det = _mtx_det<T, Tm>(pmtx, s32K, ptmp); s32mnormb = 0; T d = ABS(det);
		if(detmax > 0) while(d > detmax) { detmax *= 2; s32mnormb ++; }
		if(s32mnormb) d = DIV<T>(d, (T)(1 << s32mnormb));
		_mtx_scl<T, T>(s32K, s32K, p, s32K, p, s32K, 0, s32inormb - s32mnormb);
		_mtx_opt<__math_dft, T, T, __mtx_div_fix(T, T)>(p, s32K, p, d, __NxM_NxM(s32K, s32K), __NO_SCL);
		if(det < 0) _mtx_opt<__math_dft, T, T, __fix_sub_mtx(T, T)>(p, s32K, 0, p, __NxM_NxM(s32K, s32K), __NO_SCL);
			}
	_mtx_cpy<Ti, T>(s32K, s32K, pinv, s32K, p, s32K); return det;
	/**	ENDOFFUNCTION: _mtx_inv **/
	}



	/******************************************************************************************************************
	*	Function: _mtx_out
	*	Description: Matrix print out.
	******************************************************************************************************************/
	template <class T>
	void	_mtx_out(	T		*p,						/*!	Matrix buffer !*/
						SIGN32	s32N,					/*!	NxM matrix width !*/
						SIGN32	s32M,					/*!	NxM matrix height !*/
						SIGN32	s32inc,					/*!	Matrix horizontal incremental !*/
						SIGN32	s32pitch,				/*!	Matrix line stride !*/
						char	*ps8lnpfx				/*!	Prefix to output at the beginning of each lines !*/
						)
	{
	SIGN32	i, j;
	for(i = 0; i < s32M; i ++, p += s32pitch) {
		xdbg("%s{ ", ps8lnpfx); for(j = 0; j < s32N; j ++) _tout<T>(p[j * s32inc]); xdbg("    }\n");
			}
	/**	ENDOFFUNCTION: _mtx_out **/
	}

/**	ENDOFSECTION
 */

/**	__cplusplus
 */
#endif



/**	MATH_OPS
 */
#endif

/**	ENDOFFILE: math_ops.h *********************************************************************************************
 */

