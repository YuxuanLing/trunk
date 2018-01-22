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


#define HEVC_MALLOC(type, count)    (type*)hevc_enc_malloc(sizeof(type) * (count))
#define HEVC_FREE(ptr)              hevc_enc_free(ptr)
#define HEVC_FREE_ZERO(ptr)         hevc_enc_free(ptr); (ptr) = NULL


#endif
