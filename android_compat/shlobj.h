#pragma once
#include "windows.h"

#define CSIDL_PERSONAL 0x0005
#define CSIDL_MYDOCUMENTS 0x000c
#define SHGFP_TYPE_CURRENT 0

inline HRESULT SHGetFolderPath(HWND, int, HANDLE, DWORD, LPSTR pszPath) { if (pszPath) pszPath[0] = 0; return 0; }
inline HRESULT SHGetFolderPathA(HWND h, int c, HANDLE t, DWORD f, LPSTR p) { return SHGetFolderPath(h, c, t, f, p); }

inline BOOL SHGetSpecialFolderPath(HWND, LPSTR pszPath, int, BOOL) { if (pszPath) pszPath[0] = 0; return TRUE; }
inline BOOL SHGetSpecialFolderPathA(HWND h, LPSTR p, int c, BOOL f) { return SHGetSpecialFolderPath(h, p, c, f); }
