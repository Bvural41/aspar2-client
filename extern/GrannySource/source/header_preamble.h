// Note missing header guards, we want this to include every time...

// ========================================================================
// $File: //jeffr/granny_29/rt/header_preamble.h $
// $DateTime: 2011/12/06 12:28:06 $
// $Change: 35917 $
// $Revision: #1 $
//
// (C) Copyright 1999-2011 by RAD Game Tools, All Rights Reserved.
// ========================================================================

#if !defined(GRANNY_COMPILER_H)
#include "granny_compiler.h"
#endif

// Structure packing explicitly set to 4 (32-bit) or 8 (64-bit)
#if COMPILER_MSVC || defined(__x86_64__) || defined(__aarch64__) || defined(__arm64__) || defined(_WIN64)

#pragma pack(push)
#if defined(__x86_64__) || defined(__aarch64__) || defined(__arm64__) || defined(_WIN64) || defined(__RAD64__)
#pragma pack(8)
#else
#pragma pack(4)
#endif

#endif
