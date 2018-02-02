#ifndef HEVC_ENC_INTRAPRED_H
#define HEVC_ENC_INTRAPRED_H
#include "commonDef.h"
#include "constants.h"


template<int width>
void intra_pred_dc_c(Pixel* dst, INT32 dstStride, const Pixel* srcPix, int /*dirMode*/, int bFilter);

template<int log2Size>
void intra_planar_pred_c(Pixel* dst, INT32 dstStride, const Pixel* srcPix, int /*dirMode*/, int /*bFilter*/);


template<int width>
void intra_pred_ang_c(Pixel* dst, INT32 dstStride, const Pixel *srcPix0, int dirMode, int bFilter);

template<int log2Size>
void all_angs_pred_c(Pixel *dest, Pixel *refPix, Pixel *filtPix, int bLuma);


#endif
