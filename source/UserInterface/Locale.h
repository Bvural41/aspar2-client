#pragma once

#if defined(__ANDROID__) || defined(__linux__) || defined(__GNUC__) || defined(__clang__)
#include_next <locale.h>
#endif

#ifdef __cplusplus

#if defined(__APPLE__)
#include <objc/objc.h>
#ifndef LPCSTR
typedef const char* LPCSTR;
#endif
#ifndef BYTE
typedef unsigned char BYTE;
#endif
#ifndef HINSTANCE
typedef void* HINSTANCE;
#endif
#elif defined(__ANDROID__) || !defined(_WIN32)
#ifndef BOOL
typedef int BOOL;
#endif
#ifndef LPCSTR
typedef const char* LPCSTR;
#endif
#ifndef BYTE
typedef unsigned char BYTE;
#endif
#ifndef HINSTANCE
typedef void* HINSTANCE;
#endif
#endif
#include "Locale_inc.h"

bool		LocaleService_IsYMIR();
bool		LocaleService_IsJAPAN();
bool		LocaleService_IsENGLISH();
bool		LocaleService_IsHONGKONG();
bool		LocaleService_IsTAIWAN();
bool		LocaleService_IsNEWCIBN();
bool		LocaleService_IsEUROPE();
bool		LocaleService_IsWorldEdition();

unsigned	LocaleService_GetCodePage();
const char* LocaleService_GetName();
const char*	LocaleService_GetLocaleName();
const char*	LocaleService_GetLocalePath();
const char*	LocaleService_GetSecurityKey();
BOOL		LocaleService_IsLeadByte( const char chByte );
int			LocaleService_StringCompareCI( LPCSTR szStringLeft, LPCSTR szStringRight, size_t sizeLength );

void		LocaleService_ForceSetLocale(const char* name, const char* localePath);
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
BYTE LocaleService_GetLocaID();
bool LocaleService_SaveLoca(int iCodePage, const char* szLocale);
const char* LocaleService_GetLoca();
#endif
void		LocaleService_LoadConfig(const char* fileName);
bool		LocaleService_LoadGlobal(HINSTANCE hInstance);
unsigned	LocaleService_GetLastExp(int level);
int			LocaleService_GetSkillPower(unsigned level);

// CHEONMA
void		LocaleService_SetCHEONMA(bool isEnable);
bool		LocaleService_IsCHEONMA();
// END_OF_CHEONMA

#ifdef USE_OPENID
void LocaleService_SetOpenIDAuthKey(const char *authKey);
const char*	LocaleService_GetOpenIDAuthKey();
#endif

#endif /* __cplusplus */