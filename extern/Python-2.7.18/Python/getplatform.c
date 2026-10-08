
#include "Python.h"

#ifndef PLATFORM
#if defined(__APPLE__)
#define PLATFORM "darwin"
#elif defined(__ANDROID__)
#define PLATFORM "linux2"
#else
#define PLATFORM "unknown"
#endif
#endif

const char *
Py_GetPlatform(void)
{
	return PLATFORM;
}
