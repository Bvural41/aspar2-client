#pragma once

#include <stdint.h>

#if defined(_WIN32)
#include <windows.h>
#else
#ifndef DWORD
typedef uint32_t DWORD;
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

	#define TEA_KEY_LENGTH 16

    int tea_encrypt(DWORD *dest, const DWORD *src, const DWORD *key, int size);
    int tea_decrypt(DWORD *dest, const DWORD *src, const DWORD *key, int size);

#ifdef __cplusplus
};

#if defined(_WIN32)
// On Windows MSVC, uint32_t is unsigned int and DWORD is unsigned long.
// Provide inline overloads so code passing uint32_t* works without casts.
inline int tea_encrypt(uint32_t *dest, const uint32_t *src, const uint32_t *key, int size)
{
	return tea_encrypt(reinterpret_cast<DWORD*>(dest), reinterpret_cast<const DWORD*>(src), reinterpret_cast<const DWORD*>(key), size);
}

inline int tea_decrypt(uint32_t *dest, const uint32_t *src, const uint32_t *key, int size)
{
	return tea_decrypt(reinterpret_cast<DWORD*>(dest), reinterpret_cast<const DWORD*>(src), reinterpret_cast<const DWORD*>(key), size);
}
#endif

#endif
