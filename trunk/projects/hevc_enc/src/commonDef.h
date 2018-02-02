/** \file     CommonDef.h
\brief    Defines version information, constants and small in-line functions
*/

#ifndef __COMMONDEF__
#define __COMMONDEF__


#include <algorithm>
#include <iostream>
#include <assert.h>
#include <limits>

#include "typeDef.h"

#if _MSC_VER > 1000
#pragma once
#endif


#define NUM_INTRA_MODE 35


#if HIGH_BIT_DEPTH
typedef UINT16 Pixel;
typedef UINT32 Sum_t;
typedef UINT64 Sum2_t;
typedef UINT64 Pixel4;
typedef INT64  Ssum2_t;
#else
typedef UINT8  Pixel;
typedef UINT16 Sum_t;
typedef UINT32 Sum2_t;
typedef UINT32 Pixel4;
typedef INT32  Ssum2_t; // Signed sum
#endif // if HIGH_BIT_DEPTH



#define HEVC_MALLOC(type, count)    (type*)hevc_enc_malloc(sizeof(type) * (count))
#define HEVC_FREE(ptr)              hevc_enc_free(ptr)
#define HEVC_FREE_ZERO(ptr)         hevc_enc_free(ptr); (ptr) = NULL






#endif
