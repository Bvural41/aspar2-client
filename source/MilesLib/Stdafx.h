#ifndef __INC_MILESLIB_STDAFX_H__
#define __INC_MILESLIB_STDAFX_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable:4786)
#pragma warning(disable:4100)
#pragma warning(disable:4201)

#include <windows.h>
#include <miniaudio.h>

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif

#include "../eterBase/CRC32.h"
#include "../eterBase/Utils.h"
#include "../eterBase/Debug.h"


#endif
