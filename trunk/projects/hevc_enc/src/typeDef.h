/** \file     TypeDef.h
\brief    Define macros, basic types, new types and enumerations
*/

#ifndef __TYPEDEF__
#define __TYPEDEF__

#ifndef __COMMONDEF__
#error Include CommonDef.h not TypeDef.h
#endif


#if _MSC_VER > 1000
#pragma once
#endif


#include <vector>
#include <utility>

// ====================================================================================================================
// Basic type redefinition
// ====================================================================================================================

typedef       void                Void;
typedef       bool                Bool;

typedef       char                TChar; // Used for text/characters
typedef       signed char         SChar; // Signed 8-bit values
typedef       unsigned char       UChar; // Unsigned 8-bit values
typedef       unsigned char       UINT8; // Unsigned 8-bit values
typedef       short               Short;
typedef       short               INT16;
typedef       unsigned short      UShort;
typedef       unsigned short      UINT16;
typedef       int                 INT32;
typedef       unsigned int        UINT32;
typedef       double              Double;
typedef       float               Float;


#endif