#pragma once

#if !defined(__ANDROID__) && !defined(__APPLE__)
  #error "This windows.h is only for Android and Apple iOS!"
#endif

// Complete stub for Windows Win32, D3D and COM types for Android Clang build.

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#ifdef __cplusplus
#define EXTERN_C extern "C"
extern "C" {

#else
#define EXTERN_C extern
#endif

extern int g_nAndroidScreenWidth;
extern int g_nAndroidScreenHeight;
extern int g_nAndroidMouseX;
extern int g_nAndroidMouseY;

#define WIN32_LEAN_AND_MEAN

#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#ifndef _MAX_PATH
#define _MAX_PATH 260
#endif

#ifndef LF_FACESIZE
#define LF_FACESIZE 32
#endif

#ifndef CONST
#define CONST const
#endif

#ifndef VOID
#define VOID void
#endif

#ifndef FAR
#define FAR
#endif

#ifndef NEAR
#define NEAR
#endif

#ifndef DECLSPEC_UUID
#define DECLSPEC_UUID(x)
#endif

#ifndef KL_NAMELENGTH
#define KL_NAMELENGTH 9
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef OPTIONAL
#define OPTIONAL
#endif

#ifndef stricmp
#define stricmp strcasecmp
#endif
#ifndef strnicmp
#define strnicmp strncasecmp
#endif
#ifndef _stricmp
#define _stricmp strcasecmp
#endif
#ifndef _strnicmp
#define _strnicmp strncasecmp
#endif
#ifndef _snprintf
#define _snprintf snprintf
#endif
#ifndef _vsnprintf
#define _vsnprintf vsnprintf
#endif
#define PostQuitMessage(exitCode) ((void)(exitCode))

#define ZeroMemory(Destination,Length) memset((Destination),0,(Length))
#define CopyMemory(Destination,Source,Length) memcpy((Destination),(Source),(Length))
#define MoveMemory(Destination,Source,Length) memmove((Destination),(Source),(Length))
#define FillMemory(Destination,Length,Fill) memset((Destination),(Fill),(Length))

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#define BLACK_BRUSH 4
#define WS_OVERLAPPED       0x00000000L
#define WS_POPUP            0x80000000L
#define WS_CHILD            0x40000000L
#define WS_MINIMIZE         0x20000000L
#define WS_VISIBLE          0x10000000L
#define WS_DISABLED         0x08000000L
#define WS_CLIPSIBLINGS     0x04000000L
#define WS_CLIPCHILDREN     0x02000000L
#define WS_MAXIMIZE         0x01000000L
#define WS_CAPTION          0x00C00000L
#define WS_BORDER           0x00800000L
#define WS_DLGFRAME         0x00400000L
#define WS_VSCROLL          0x00200000L
#define WS_HSCROLL          0x00100000L
#define WS_SYSMENU          0x00080000L
#define WS_THICKFRAME       0x00040000L
#define WS_GROUP            0x00020000L
#define WS_TABSTOP          0x00010000L
#define WS_MINIMIZEBOX      0x00020000L
#define WS_MAXIMIZEBOX      0x00010000L
#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)

#define HWND_TOP            ((HWND)0)
#define SWP_SHOWWINDOW      0x0040
#define WM_SETICON          0x0080
#define ICON_SMALL          0
#define ICON_BIG            1

#define SM_CXBORDER         5
#define SM_CYBORDER         6
#define SM_CXDLGFRAME       7
#define SM_CYDLGFRAME       8
#define SM_CXFRAME          32
#define SM_CYFRAME          33
#define SM_CYCAPTION        4

#define SPI_GETWORKAREA     0x0030
#define SPI_GETSTICKYKEYS   0x003A
#define SPI_SETSTICKYKEYS   0x003B
#define SKF_AVAILABLE       0x00000002
#define SKF_HOTKEYACTIVE    0x00000004

#define FLASHW_ALL          0x00000003
#define FLASHW_TIMERNOFG    0x0000000C

#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010

#ifndef strcmpi
#define strcmpi strcasecmp
#endif
#ifndef _strcmpi
#define _strcmpi strcasecmp
#endif

#define MAKEINTRESOURCE(i) ((LPSTR)((uintptr_t)((WORD)(i))))
#define MAKEINTRESOURCEA(i) ((LPSTR)((uintptr_t)((WORD)(i))))
#define MAKEWORD(a, b)      ((WORD)(((BYTE)(((uintptr_t)(a)) & 0xff)) | ((WORD)((BYTE)(((uintptr_t)(b)) & 0xff))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)(((uintptr_t)(a)) & 0xffff)) | ((DWORD)((WORD)(((uintptr_t)(b)) & 0xffff))) << 16))
#define LOWORD(l)           ((WORD)(((uintptr_t)(l)) & 0xffff))
#define HIWORD(l)           ((WORD)((((uintptr_t)(l)) >> 16) & 0xffff))
#define LOBYTE(w)           ((BYTE)(((uintptr_t)(w)) & 0xff))
#define MAKEFOURCC(ch0, ch1, ch2, ch3) ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) | ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24))
#define MessageBox(hWnd, lpText, lpCaption, uType) ((int)0)

#include <wchar.h>
#ifndef __min
#define __min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef __max
#define __max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#define _PC_24 0
#define _MCW_PC 0
#define _controlfp(x, y) (0)

#define VK_LEFT 0x25
#define VK_UP 0x26
#define VK_RIGHT 0x27
#define VK_DOWN 0x28
#define VK_HOME 0x24
#define VK_END 0x23
#define VK_DELETE 0x2E
#define VK_INSERT 0x2D
#define VK_PRIOR 0x21
#define VK_NEXT 0x22
#define VK_RETURN 0x0D
#define VK_SPACE 0x20
#define VK_ESCAPE 0x1B
#define VK_TAB 0x09
#define VK_BACK 0x08

typedef char CHAR;
typedef char TCHAR;
typedef wchar_t WCHAR;
#define _T(x) x
#define TEXT(x) x

#if defined(__APPLE__)
#include <objc/objc.h>
#else
typedef int BOOL;
#endif
typedef int16_t SHORT;
typedef int32_t LONG;
typedef uint32_t ULONG;
typedef uint32_t DWORD;
typedef size_t SIZE_T;
typedef size_t* PSIZE_T;
typedef intptr_t SSIZE_T;
typedef uint16_t WORD;
typedef WORD ATOM;
typedef uint8_t BYTE;
typedef void* HANDLE;
typedef void* LPVOID;
typedef void* PVOID;
typedef const void* LPCVOID;
typedef const void* PCVOID;
typedef uint32_t UINT;
typedef uint32_t UINT32;
typedef uint64_t UINT64;
typedef uint16_t USHORT;
typedef uint8_t UCHAR;
typedef double DOUBLE;
typedef int32_t INT;
typedef float FLOAT;

typedef struct _LUID {
    DWORD LowPart;
    LONG HighPart;
} LUID, *PLUID;

typedef struct tagTEXTMETRICW {
    LONG tmHeight;
    LONG tmAscent;
    LONG tmDescent;
    LONG tmInternalLeading;
    LONG tmExternalLeading;
    LONG tmAveCharWidth;
    LONG tmMaxCharWidth;
    LONG tmWeight;
    LONG tmOverhang;
    LONG tmDigitizedAspectX;
    LONG tmDigitizedAspectY;
    WCHAR tmFirstChar;
    WCHAR tmLastChar;
    WCHAR tmDefaultChar;
    WCHAR tmBreakChar;
    BYTE tmItalic;
    BYTE tmUnderlined;
    BYTE tmStruckOut;
    BYTE tmPitchAndFamily;
    BYTE tmCharSet;
} TEXTMETRICW, *PTEXTMETRICW, *LPTEXTMETRICW;

typedef struct tagTEXTMETRICA {
    LONG tmHeight;
    LONG tmAscent;
    LONG tmDescent;
    LONG tmInternalLeading;
    LONG tmExternalLeading;
    LONG tmAveCharWidth;
    LONG tmMaxCharWidth;
    LONG tmWeight;
    LONG tmOverhang;
    LONG tmDigitizedAspectX;
    LONG tmDigitizedAspectY;
    BYTE tmFirstChar;
    BYTE tmLastChar;
    BYTE tmDefaultChar;
    BYTE tmBreakChar;
    BYTE tmItalic;
    BYTE tmUnderlined;
    BYTE tmStruckOut;
    BYTE tmPitchAndFamily;
    BYTE tmCharSet;
} TEXTMETRICA, *PTEXTMETRICA, *LPTEXTMETRICA;
typedef void* HWND;
typedef void* HDC;
typedef void* HGLRC;
typedef void* HMODULE;
typedef void* HINSTANCE;
typedef void* HFONT;
typedef void* HBITMAP;
typedef void* HPALETTE;
typedef void* HRGN;
typedef void* HKL;
typedef void* HHOOK;
typedef void* HMENU;
typedef void* HICON;
typedef void* HCURSOR;
typedef void* HBRUSH;
typedef void* HPEN;
typedef void* HWINEVENTHOOK;
typedef uint16_t LANGID;
typedef uint32_t LCID;
typedef long LRESULT;
typedef long HRESULT;

typedef int (*FARPROC)();
typedef int (*NEARPROC)();
typedef int (*PROC)();

#define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name

typedef BYTE* LPBYTE;
typedef BYTE* PBYTE;
typedef DWORD* LPDWORD;
typedef DWORD* PDWORD;
typedef WORD* LPWORD;
typedef WORD* PWORD;
typedef BOOL* LPBOOL;
typedef BOOL* PBOOL;
typedef INT* LPINT;
typedef INT* PINT;
typedef UINT* PUINT;
typedef UINT* LPUINT;
typedef LONG* LPLONG;
typedef LONG* PLONG;

typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

typedef struct _WIN32_FIND_DATAA {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    CHAR  cFileName[MAX_PATH];
    CHAR  cAlternateFileName[14];
} WIN32_FIND_DATAA, *PWIN32_FIND_DATAA, *LPWIN32_FIND_DATAA;
typedef WIN32_FIND_DATAA WIN32_FIND_DATA;

typedef struct _OSVERSIONINFOA {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    CHAR  szCSDVersion[128];
} OSVERSIONINFOA, *POSVERSIONINFOA, *LPOSVERSIONINFOA;
typedef OSVERSIONINFOA OSVERSIONINFO;

typedef struct tagSTICKYKEYS {
    DWORD cbSize;
    DWORD dwFlags;
} STICKYKEYS, *LPSTICKYKEYS;

typedef struct tagFLASHWINFO {
    UINT  cbSize;
    HWND  hwnd;
    DWORD dwFlags;
    UINT  uCount;
    DWORD dwTimeout;
} FLASHWINFO, *PFLASHWINFO;

typedef struct _SECURITY_ATTRIBUTES {
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

typedef const char* LPCSTR;
typedef const char* PCSTR;
typedef char* LPSTR;
typedef char* PCHAR;
typedef const char* LPCTSTR;
typedef char* LPTSTR;
typedef const wchar_t* LPCWSTR;
typedef wchar_t* LPWSTR;

typedef uintptr_t UINT_PTR;
typedef intptr_t INT_PTR;
typedef uintptr_t DWORD_PTR;
typedef intptr_t LONG_PTR;

typedef LONG_PTR LPARAM;
typedef UINT_PTR WPARAM;

typedef LRESULT (*WNDPROC)(HWND, UINT, WPARAM, LPARAM);

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define S_OK 0
#define S_FALSE 1
#define E_FAIL -1
#define E_NOTIMPL -2
#define E_OUTOFMEMORY -3
#define E_INVALIDARG -4
#define E_POINTER -5
#define E_HANDLE -6
#define E_ABORT -7
#define E_ACCESSDENIED -8
#define E_PENDING -9

#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)

#ifndef MAKE_HRESULT
#define MAKE_HRESULT(sev,fac,code) \
    ((HRESULT) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))) )
#endif

typedef int64_t LONGLONG;
typedef uint64_t ULONGLONG;
typedef void* RPC_IF_HANDLE;
struct IRpcStubBuffer;
struct IRpcChannelBuffer;
typedef void* PRPC_MESSAGE;

#define WINAPI
#define CALLBACK
#define __stdcall
#define __RPC_USER
#define __RPC_STUB
#define STDMETHODCALLTYPE
#define STDAPI EXTERN_C HRESULT
#define MIDL_INTERFACE(x) struct

typedef struct _CRITICAL_SECTION {
    void* DebugInfo;
    LONG LockCount;
    LONG RecursionCount;
    HANDLE OwningThread;
    HANDLE LockSemaphore;
    uintptr_t SpinCount;
} CRITICAL_SECTION, *PCRITICAL_SECTION, *LPCRITICAL_SECTION;

typedef union _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG HighPart;
    };
    struct {
        DWORD LowPart;
        LONG HighPart;
    } u;
    int64_t QuadPart;
} LARGE_INTEGER;

typedef union _ULARGE_INTEGER {
    struct {
        DWORD LowPart;
        DWORD HighPart;
    };
    struct {
        DWORD LowPart;
        DWORD HighPart;
    } u;
    uint64_t QuadPart;
} ULARGE_INTEGER;

typedef struct _GUID {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
} GUID;

typedef GUID IID;
typedef GUID CLSID;
typedef GUID* LPGUID;
typedef const GUID* LPCGUID;
typedef const GUID& REFGUID;
typedef const IID& REFIID;
typedef const CLSID& REFCLSID;

#ifndef DEFINE_GUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    static const GUID name = { l, w1, w2, { (unsigned char)(b1), (unsigned char)(b2), (unsigned char)(b3), (unsigned char)(b4), (unsigned char)(b5), (unsigned char)(b6), (unsigned char)(b7), (unsigned char)(b8) } }
#endif
#ifndef interface
#define interface struct
#endif
#define PURE = 0
#define THIS_
#define THIS void
#define STDMETHOD(method) virtual HRESULT STDMETHODCALLTYPE method
#define STDMETHOD_(type,method) virtual type STDMETHODCALLTYPE method
#define DECLARE_INTERFACE(iface) struct iface
#define DECLARE_INTERFACE_(iface, baseiface) struct iface : public baseiface

#ifndef __IUnknown_INTERFACE_DEFINED__
#define __IUnknown_INTERFACE_DEFINED__
struct IUnknown {
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) = 0;
    virtual ULONG STDMETHODCALLTYPE AddRef(void) = 0;
    virtual ULONG STDMETHODCALLTYPE Release(void) = 0;
};
#endif

struct IStream;

typedef struct _RECT {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECT, *PRECT, *LPRECT;

typedef struct _POINT {
  LONG x;
  LONG y;
} POINT, *PPOINT, *LPPOINT;

typedef struct _SIZE {
  LONG cx;
  LONG cy;
} SIZE, *PSIZE, *LPSIZE;

typedef struct tagMSG {
    HWND   hwnd;
    UINT   message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD  time;
    POINT  pt;
} MSG, *PMSG, *NPMSG, *LPMSG;

typedef struct _RGNDATAHEADER {
    DWORD dwSize;
    DWORD iType;
    DWORD nCount;
    DWORD nRgnSize;
    RECT  rcBound;
} RGNDATAHEADER;

typedef struct _RGNDATA {
    RGNDATAHEADER rdh;
    char          Buffer[1];
} RGNDATA;

typedef struct _PALETTEENTRY {
    BYTE peRed;
    BYTE peGreen;
    BYTE peBlue;
    BYTE peFlags;
} PALETTEENTRY;

typedef struct tagLOGFONTA {
    LONG lfHeight;
    LONG lfWidth;
    LONG lfEscapement;
    LONG lfOrientation;
    LONG lfWeight;
    BYTE lfItalic;
    BYTE lfUnderline;
    BYTE lfStrikeOut;
    BYTE lfCharSet;
    BYTE lfOutPrecision;
    BYTE lfClipPrecision;
    BYTE lfQuality;
    BYTE lfPitchAndFamily;
    char lfFaceName[32];
} LOGFONTA, *PLOGFONTA, *LPLOGFONTA;

typedef LOGFONTA LOGFONT;

typedef struct tagLOGFONTW {
    LONG lfHeight;
    LONG lfWidth;
    LONG lfEscapement;
    LONG lfOrientation;
    LONG lfWeight;
    BYTE lfItalic;
    BYTE lfUnderline;
    BYTE lfStrikeOut;
    BYTE lfCharSet;
    BYTE lfOutPrecision;
    BYTE lfClipPrecision;
    BYTE lfQuality;
    BYTE lfPitchAndFamily;
    WCHAR lfFaceName[32];
} LOGFONTW, *PLOGFONTW, *LPLOGFONTW;



typedef TEXTMETRICA TEXTMETRIC;

typedef struct _POINTFLOAT {
    FLOAT x;
    FLOAT y;
} POINTFLOAT;

typedef struct _GLYPHMETRICSFLOAT {
    FLOAT      gmfBlackBoxX;
    FLOAT      gmfBlackBoxY;
    POINTFLOAT gmfptGlyphOrigin;
    FLOAT      gmfCellIncX;
    FLOAT      gmfCellIncY;
} GLYPHMETRICSFLOAT, *PGLYPHMETRICSFLOAT, *LPGLYPHMETRICSFLOAT;

typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG  biWidth;
    LONG  biHeight;
    WORD  biPlanes;
    WORD  biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG  biXPelsPerMeter;
    LONG  biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER, *LPBITMAPINFOHEADER;

typedef struct tagRGBQUAD {
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiColors[1];
} BITMAPINFO, *PBITMAPINFO, *LPBITMAPINFO;

typedef uint32_t D3DCOLOR;

typedef struct IDirect3DDevice8 *LPDIRECT3DDEVICE8;
typedef struct IDirect3DTexture8 *LPDIRECT3DTEXTURE8;

#ifdef __cplusplus
inline DWORD GetTickCount() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (DWORD)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}
#ifndef GetCurrentTime
#define GetCurrentTime() GetTickCount()
#endif
#ifndef timeGetTime
#define timeGetTime() GetTickCount()
#endif

inline void Sleep(DWORD dwMilliseconds) { usleep(dwMilliseconds * 1000); }
inline void timeBeginPeriod(UINT) {}
inline void timeEndPeriod(UINT) {}

inline void InitializeCriticalSection(LPCRITICAL_SECTION) {}
inline void DeleteCriticalSection(LPCRITICAL_SECTION) {}
inline void EnterCriticalSection(LPCRITICAL_SECTION) {}
inline void LeaveCriticalSection(LPCRITICAL_SECTION) {}
inline HRESULT CoInitialize(void*) { return S_OK; }
inline void CoUninitialize(void) {}
inline HRESULT CoCreateInstance(REFIID, struct IUnknown*, DWORD, REFIID, void**) { return E_NOTIMPL; }
inline HWND FindWindow(LPCSTR, LPCSTR) { return NULL; }
inline HICON LoadIcon(HINSTANCE, LPCSTR) { return NULL; }
inline BOOL SystemParametersInfo(UINT, UINT, PVOID, UINT) { return TRUE; }

#ifndef SM_CXSCREEN
#define SM_CXSCREEN 0
#define SM_CYSCREEN 1
#endif
#ifndef SM_CXFULLSCREEN
#define SM_CXFULLSCREEN 16
#define SM_CYFULLSCREEN 17
#endif

inline int GetSystemMetrics(int nIndex) {
    if (nIndex == SM_CXSCREEN || nIndex == SM_CXFULLSCREEN) return (g_nAndroidScreenWidth > 0) ? g_nAndroidScreenWidth : 2400;
    if (nIndex == SM_CYSCREEN || nIndex == SM_CYFULLSCREEN) return (g_nAndroidScreenHeight > 0) ? g_nAndroidScreenHeight : 1080;
    return 0;
}

#define GENERIC_READ        0x80000000L
#define GENERIC_WRITE       0x40000000L
#define GENERIC_EXECUTE     0x20000000L
#define GENERIC_ALL         0x10000000L

#define FILE_SHARE_READ     0x00000001
#define FILE_SHARE_WRITE    0x00000002
#define FILE_SHARE_DELETE   0x00000004

#define CREATE_NEW          1
#define CREATE_ALWAYS       2
#define OPEN_EXISTING       3
#define OPEN_ALWAYS         4
#define TRUNCATE_EXISTING   5

#define FILE_ATTRIBUTE_NORMAL 0x00000080

#define FILE_BEGIN          0
#define FILE_CURRENT        1
#define FILE_END            2

inline HANDLE FindFirstFile(LPCSTR, LPWIN32_FIND_DATAA) { return INVALID_HANDLE_VALUE; }
inline BOOL FindNextFile(HANDLE, LPWIN32_FIND_DATAA) { return FALSE; }
inline BOOL FindClose(HANDLE) { return TRUE; }
inline DWORD GetFileAttributesA(LPCSTR) { return INVALID_FILE_ATTRIBUTES; }
#define GetFileAttributes GetFileAttributesA
#define _unlink unlink
inline SHORT GetAsyncKeyState(int) { return 0; }
inline BOOL MoveFile(LPCSTR, LPCSTR) { return TRUE; }
inline BOOL CopyFile(LPCSTR, LPCSTR, BOOL) { return TRUE; }
inline BOOL DeleteFile(LPCSTR) { return TRUE; }
inline DWORD GetCurrentDirectory(DWORD, LPSTR) { return 0; }
inline int lstrlen(LPCSTR s) { return s ? (int)strlen(s) : 0; }
inline DWORD GetPrivateProfileString(LPCSTR, LPCSTR, LPCSTR, LPSTR lpReturnedString, DWORD nSize, LPCSTR) { if (nSize) lpReturnedString[0] = 0; return 0; }
inline int LoadString(HINSTANCE, UINT, LPSTR lpBuffer, int nBufferMax) { if (nBufferMax) lpBuffer[0] = 0; return 0; }
inline UINT GetDriveType(LPCSTR) { return 0; }
inline HANDLE GetCurrentProcess() { return (HANDLE)1; }
inline BOOL SetProcessWorkingSetSize(HANDLE, SIZE_T, SIZE_T) { return TRUE; }

inline BOOL SetWindowPos(HWND, HWND, int, int, int, int, UINT) { return TRUE; }
inline LRESULT SendMessage(HWND, UINT, WPARAM, LPARAM) { return 0; }
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <pthread.h>
#ifdef __ANDROID__
#include <android/log.h>
#endif

#define INVALID_FILE_SIZE   ((DWORD)0xFFFFFFFF)
#define INVALID_SET_FILE_POINTER ((DWORD)-1)

struct PackCacheEntry {
    char path[256];
    int fd;
    DWORD fileSize;
};

inline PackCacheEntry g_packCache[128] = {};
inline int g_packCacheCount = 0;
inline pthread_mutex_t g_packCacheMutex = PTHREAD_MUTEX_INITIALIZER;

inline bool IsPackDataFile(const char* path) {
    if (!path) return false;
    size_t len = strlen(path);
    if (len < 4) return false;
    const char* ext = path + len - 4;
    if (strcasecmp(ext, ".epk") == 0 || strcasecmp(ext, ".eix") == 0) return true;
    if (len >= 5) {
        const char* ext5 = path + len - 5;
        if (strcasecmp(ext5, ".data") == 0) return true;
    }
    if (len >= 6) {
        const char* ext6 = path + len - 6;
        if (strcasecmp(ext6, ".index") == 0) return true;
    }
    return false;
}

inline int FindCachedPackFd(const char* path, DWORD* outSize = nullptr) {
    pthread_mutex_lock(&g_packCacheMutex);
    for (int i = 0; i < g_packCacheCount; ++i) {
        if (strcasecmp(g_packCache[i].path, path) == 0) {
            int fd = g_packCache[i].fd;
            if (outSize) *outSize = g_packCache[i].fileSize;
            pthread_mutex_unlock(&g_packCacheMutex);
            return fd;
        }
    }
    pthread_mutex_unlock(&g_packCacheMutex);
    return -1;
}

inline bool IsCachedPackFd(int fd) {
    if (fd < 0) return false;
    pthread_mutex_lock(&g_packCacheMutex);
    for (int i = 0; i < g_packCacheCount; ++i) {
        if (g_packCache[i].fd == fd) {
            pthread_mutex_unlock(&g_packCacheMutex);
            return true;
        }
    }
    pthread_mutex_unlock(&g_packCacheMutex);
    return false;
}

inline DWORD GetCachedPackFileSize(int fd) {
    if (fd < 0) return INVALID_FILE_SIZE;
    pthread_mutex_lock(&g_packCacheMutex);
    for (int i = 0; i < g_packCacheCount; ++i) {
        if (g_packCache[i].fd == fd) {
            DWORD sz = g_packCache[i].fileSize;
            pthread_mutex_unlock(&g_packCacheMutex);
            return sz;
        }
    }
    pthread_mutex_unlock(&g_packCacheMutex);
    return INVALID_FILE_SIZE;
}

inline void AddCachedPackFd(const char* path, int fd, DWORD fileSize) {
    pthread_mutex_lock(&g_packCacheMutex);
    if (g_packCacheCount < 128) {
        strncpy(g_packCache[g_packCacheCount].path, path, sizeof(g_packCache[g_packCacheCount].path) - 1);
        g_packCache[g_packCacheCount].path[sizeof(g_packCache[g_packCacheCount].path) - 1] = '\0';
        g_packCache[g_packCacheCount].fd = fd;
        g_packCache[g_packCacheCount].fileSize = fileSize;
        g_packCacheCount++;
    }
    pthread_mutex_unlock(&g_packCacheMutex);
}

inline HANDLE CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
    if (!lpFileName) return INVALID_HANDLE_VALUE;

    char cleanPathBuf[512];
    strncpy(cleanPathBuf, lpFileName, sizeof(cleanPathBuf) - 1);
    cleanPathBuf[sizeof(cleanPathBuf) - 1] = '\0';
    for (char* p = cleanPathBuf; *p; ++p) {
        if (*p == '\\') *p = '/';
    }
    const char* lpCleanName = cleanPathBuf;

    int flags = 0;
    if ((dwDesiredAccess & (GENERIC_READ | GENERIC_WRITE)) == (GENERIC_READ | GENERIC_WRITE)) {
        flags = O_RDWR;
    } else if (dwDesiredAccess & GENERIC_WRITE) {
        flags = O_WRONLY;
    } else {
        flags = O_RDONLY;
    }

    if (dwCreationDisposition == CREATE_ALWAYS || dwCreationDisposition == OPEN_ALWAYS) {
        flags |= O_CREAT;
    }
    if (dwCreationDisposition == CREATE_ALWAYS) {
        flags |= O_TRUNC;
    }

    if (flags == O_RDONLY && IsPackDataFile(lpCleanName)) {
        int cachedFd = FindCachedPackFd(lpCleanName);
        if (cachedFd >= 0) {
            return (HANDLE)(intptr_t)(cachedFd + 1);
        }
    }

    int fd = open(lpCleanName, flags, 0666);
    if (fd < 0) {
        return INVALID_HANDLE_VALUE;
    }

    if (flags == O_RDONLY && IsPackDataFile(lpCleanName)) {
        struct stat st;
        DWORD fsz = 0;
        if (fstat(fd, &st) == 0) {
            fsz = (DWORD)(st.st_size & 0xFFFFFFFF);
        }
        AddCachedPackFd(lpCleanName, fd, fsz);
    }

    return (HANDLE)(intptr_t)(fd + 1);
}
#define CreateFile CreateFileA

inline BOOL CloseHandle(HANDLE hObject) {
    if (hObject == NULL || hObject == INVALID_HANDLE_VALUE) return TRUE;
    int fd = (int)(intptr_t)hObject - 1;
    if (fd >= 0) {
        if (!IsCachedPackFd(fd)) {
            close(fd);
        }
    }
    return TRUE;
}

inline BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, void* lpOverlapped) {
    if (hFile == NULL || hFile == INVALID_HANDLE_VALUE || !lpBuffer) return FALSE;
    int fd = (int)(intptr_t)hFile - 1;
    ssize_t bytes = read(fd, lpBuffer, nNumberOfBytesToRead);
    if (bytes >= 0) {
        if (lpNumberOfBytesRead) *lpNumberOfBytesRead = (DWORD)bytes;
        return TRUE;
    }
    if (lpNumberOfBytesRead) *lpNumberOfBytesRead = 0;
    return FALSE;
}

inline BOOL WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, void* lpOverlapped) {
    if (hFile == NULL || hFile == INVALID_HANDLE_VALUE || !lpBuffer) return FALSE;
    int fd = (int)(intptr_t)hFile - 1;
    ssize_t bytes = write(fd, lpBuffer, nNumberOfBytesToWrite);
    if (bytes >= 0) {
        if (lpNumberOfBytesWritten) *lpNumberOfBytesWritten = (DWORD)bytes;
        return TRUE;
    }
    if (lpNumberOfBytesWritten) *lpNumberOfBytesWritten = 0;
    return FALSE;
}

inline DWORD GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh) {
    if (hFile == NULL || hFile == INVALID_HANDLE_VALUE) return INVALID_FILE_SIZE;
    int fd = (int)(intptr_t)hFile - 1;
    DWORD cachedSize = GetCachedPackFileSize(fd);
    if (cachedSize != INVALID_FILE_SIZE) {
        if (lpFileSizeHigh) *lpFileSizeHigh = 0;
        return cachedSize;
    }
    struct stat st;
    if (fstat(fd, &st) == 0) {
        if (lpFileSizeHigh) *lpFileSizeHigh = (DWORD)(st.st_size >> 32);
        return (DWORD)(st.st_size & 0xFFFFFFFF);
    }
    return INVALID_FILE_SIZE;
}

inline DWORD SetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod) {
    if (hFile == NULL || hFile == INVALID_HANDLE_VALUE) return INVALID_SET_FILE_POINTER;
    int fd = (int)(intptr_t)hFile - 1;
    int whence = SEEK_SET;
    if (dwMoveMethod == FILE_CURRENT) whence = SEEK_CUR;
    else if (dwMoveMethod == FILE_END) whence = SEEK_END;

    off_t res = lseek(fd, lDistanceToMove, whence);
    if (res == (off_t)-1) return INVALID_SET_FILE_POINTER;
    return (DWORD)res;
}

#ifndef SUCCEEDED
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#endif
#ifndef FAILED
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#endif

#define MB_OK               0x00000000L
#define MB_TOPMOST          0x00040000L
#define HWND_TOPMOST        ((HWND)-1)

inline HDC GetDC(HWND) { return NULL; }
inline int ReleaseDC(HWND, HDC) { return 1; }

#define sprintf_s snprintf
#define VK_CONTROL          0x11
#define VK_SHIFT            0x10
#define VK_MENU             0x12

inline BOOL QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount) {
    if (lpPerformanceCount) lpPerformanceCount->QuadPart = GetTickCount();
    return TRUE;
}
inline BOOL QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency) {
    if (lpFrequency) lpFrequency->QuadPart = 1000;
    return TRUE;
}

#define PAGE_READONLY       0x02
#define FILE_MAP_READ       0x0004
#define FILE_MAP_WRITE      0x0002
#define GMEM_FIXED          0x0000
#define GMEM_ZEROINIT       0x0040
#define _S_IWRITE           0000200

typedef uint8_t BOOLEAN;
typedef unsigned char* PUCHAR;
typedef void* HGLOBAL;

typedef struct _SYSTEM_INFO {
    DWORD dwPageSize;
    LPVOID lpMinimumApplicationAddress;
    LPVOID lpMaximumApplicationAddress;
    DWORD_PTR dwActiveProcessorMask;
    DWORD dwNumberOfProcessors;
    DWORD dwProcessorType;
    DWORD dwAllocationGranularity;
    WORD wProcessorLevel;
    WORD wProcessorRevision;
} SYSTEM_INFO, *LPSYSTEM_INFO;

inline void GetSystemInfo(LPSYSTEM_INFO lpSystemInfo) {
    if (lpSystemInfo) {
        memset(lpSystemInfo, 0, sizeof(SYSTEM_INFO));
        lpSystemInfo->dwPageSize = 4096;
        lpSystemInfo->dwAllocationGranularity = 65536;
        lpSystemInfo->dwNumberOfProcessors = 4;
    }
}

inline DWORD GetLastError() { return 0; }
inline BOOL UnmapViewOfFile(LPCVOID lpBaseAddress) {
    if (!lpBaseAddress) return TRUE;
    free((void*)lpBaseAddress);  // MapViewOfFile now uses malloc+pread
    return TRUE;
}

inline UINT RegisterClipboardFormatA(LPCSTR) { return 0; }
inline BOOL OpenClipboard(HWND) { return FALSE; }
inline BOOL CloseClipboard() { return TRUE; }
inline HANDLE SetClipboardData(UINT, HANDLE) { return NULL; }
inline HANDLE GetClipboardData(UINT) { return NULL; }

#define GMEM_MOVEABLE       0x0002
inline HGLOBAL GlobalAlloc(UINT, SIZE_T) { return NULL; }
inline LPVOID GlobalLock(HGLOBAL h) { return h; }
inline BOOL GlobalUnlock(HGLOBAL) { return TRUE; }
inline HGLOBAL GlobalFree(HGLOBAL) { return NULL; }

inline HANDLE CreateFileMapping(HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR lpName) {
    if (hFile == NULL || hFile == INVALID_HANDLE_VALUE) return INVALID_HANDLE_VALUE;
    int fd = (int)(intptr_t)hFile - 1;
    if (IsCachedPackFd(fd)) {
        return hFile;
    }
    int dupFd = dup(fd);
    if (dupFd < 0) return INVALID_HANDLE_VALUE;
    return (HANDLE)(intptr_t)(dupFd + 1);
}
inline HANDLE CreateFileMappingA(HANDLE h, LPSECURITY_ATTRIBUTES sa, DWORD p, DWORD ms, DWORD ls, LPCSTR n) {
    return CreateFileMapping(h, sa, p, ms, ls, n);
}

inline LPVOID MapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap) {
    if (hFileMappingObject == NULL || hFileMappingObject == INVALID_HANDLE_VALUE) return NULL;
    int fd = (int)(intptr_t)hFileMappingObject - 1;

    size_t len = (dwNumberOfBytesToMap > 0) ? dwNumberOfBytesToMap : 0;
    off_t offset = (off_t)dwFileOffsetLow;
    if (len == 0) {
        DWORD cachedSize = GetCachedPackFileSize(fd);
        if (cachedSize != INVALID_FILE_SIZE) {
            len = (size_t)cachedSize;
        } else {
            struct stat st;
            if (fstat(fd, &st) != 0) {
                return NULL;
            }
            len = (size_t)st.st_size;
        }
        if (offset + (off_t)len > (off_t)len) {
            if ((off_t)len > offset) {
                len = (size_t)(len - offset);
            }
        }
    }
    if (len == 0) len = 4096;

    // Android FUSE on /sdcard does NOT support copy-on-write mmap (MAP_PRIVATE+PROT_WRITE).
    // Even though mmap() succeeds, the kernel raises SEGV_ACCERR when writing to such pages.
    // Solution: allocate heap memory and read the file contents into it.
    void* buf = malloc(len);
    if (!buf) {
        return NULL;
    }

    size_t totalRead = 0;
    while (totalRead < len) {
        ssize_t n = pread(fd, (char*)buf + totalRead, len - totalRead, offset + totalRead);
        if (n <= 0) {
            if (n < 0 && errno == EINTR) continue;
            // Short read or error - zero-fill the rest
            memset((char*)buf + totalRead, 0, len - totalRead);
            break;
        }
        totalRead += (size_t)n;
    }

    return buf;
}
inline void OutputDebugString(LPCSTR) {}
inline void OutputDebugStringA(LPCSTR) {}

inline DWORD GetTempPath(DWORD, LPSTR) { return 0; }
inline UINT GetTempFileName(LPCSTR, LPCSTR, UINT, LPSTR) { return 0; }
inline BOOL CreateDirectory(LPCSTR, LPSECURITY_ATTRIBUTES) { return TRUE; }
inline BOOL RemoveDirectory(LPCSTR) { return TRUE; }
inline DWORD GetModuleFileName(HMODULE, LPSTR lpFilename, DWORD) { if (lpFilename) lpFilename[0] = 0; return 0; }
inline char* _ecvt(double val, int count, int* dec, int* sign) {
    static char buf[64];
    snprintf(buf, sizeof(buf), "%.*e", count, val);
    if (dec) *dec = 0;
    if (sign) *sign = (val < 0) ? 1 : 0;
    return buf;
}

#define CP_ACP              0
#define CP_OEMCP            1
#define CP_MACCP            2
#define CP_THREAD_ACP       3
#define CP_SYMBOL           42
#define CP_UTF7             65000
#define CP_UTF8             65001

#define PM_NOREMOVE         0x0000
#define PM_REMOVE           0x0001
#define PM_NOYIELD          0x0002

#define WM_CLOSE            0x0010
#define WM_QUIT             0x0012
#define WM_DESTROY          0x0002

inline BOOL PeekMessage(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) { return FALSE; }
inline BOOL GetMessage(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax) { return FALSE; }
inline BOOL TranslateMessage(const MSG* lpMsg) { return TRUE; }
inline LRESULT DispatchMessage(const MSG* lpMsg) { return 0; }
inline HMODULE GetModuleHandle(LPCSTR) { return (HMODULE)1; }
inline HMODULE GetModuleHandleA(LPCSTR) { return (HMODULE)1; }
inline HANDLE GetCurrentThread() { return (HANDLE)2; }

#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>

inline char* _strupr(char* s) {
    if (!s) return s;
    for (char* p = s; *p; ++p) *p = (char)toupper((unsigned char)*p);
    return s;
}
#ifndef strupr
#define strupr _strupr
#endif

#define ANSI_CHARSET        0
#define DEFAULT_CHARSET     1
#define SYMBOL_CHARSET      2
#define SHIFTJIS_CHARSET    128
#define HANGUL_CHARSET      129
#define GB2312_CHARSET      134
#define CHINESEBIG5_CHARSET 136
#define OEM_CHARSET         255
#define JOHAB_CHARSET       130
#define HEBREW_CHARSET      177
#define ARABIC_CHARSET      178
#define GREEK_CHARSET       161
#define TURKISH_CHARSET     162
#define VIETNAMESE_CHARSET  163
#define THAI_CHARSET        222
#define EASTEUROPE_CHARSET  238
#define RUSSIAN_CHARSET     204
#define BALTIC_CHARSET      186

typedef int (CALLBACK* FONTENUMPROC)(const LOGFONT*, const TEXTMETRIC*, DWORD, LPARAM);
typedef LOGFONT* LPLOGFONT;
inline int EnumFontFamilies(HDC, LPCSTR, FONTENUMPROC, LPARAM) { return 0; }
inline int EnumFontFamiliesEx(HDC, LOGFONT*, FONTENUMPROC, LPARAM, DWORD) { return 0; }
inline int EnumFontFamiliesExA(HDC hdc, LOGFONT* lf, FONTENUMPROC proc, LPARAM lp, DWORD dw) { return EnumFontFamiliesEx(hdc, lf, proc, lp, dw); }

#define THREAD_PRIORITY_NORMAL 0
inline BOOL SetThreadPriority(HANDLE, int) { return TRUE; }
#define INFINITE 0xFFFFFFFF
#define WAIT_OBJECT_0 0
#define WAIT_TIMEOUT 258
#define WAIT_FAILED 0xFFFFFFFF

#if defined(__APPLE__)
#include <dispatch/dispatch.h>

#if defined(__OBJC__)
inline HANDLE CreateSemaphore(LPSECURITY_ATTRIBUTES, LONG lInitialCount, LONG lMaximumCount, LPCSTR) {
    dispatch_semaphore_t sem = dispatch_semaphore_create(lInitialCount);
    return (__bridge_retained HANDLE)sem;
}
inline BOOL ReleaseSemaphore(HANDLE hSemaphore, LONG lReleaseCount, LPLONG lpPreviousCount) {
    if (hSemaphore) {
        dispatch_semaphore_t sem = (__bridge dispatch_semaphore_t)hSemaphore;
        for (LONG i = 0; i < lReleaseCount; ++i) {
            dispatch_semaphore_signal(sem);
        }
        return TRUE;
    }
    return FALSE;
}
inline DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds) {
    if (!hHandle) return WAIT_FAILED;
    dispatch_semaphore_t sem = (__bridge dispatch_semaphore_t)hHandle;
    dispatch_time_t timeout = (dwMilliseconds == INFINITE) ? DISPATCH_TIME_FOREVER : dispatch_time(DISPATCH_TIME_NOW, (int64_t)dwMilliseconds * NSEC_PER_MSEC);
    intptr_t res = dispatch_semaphore_wait(sem, timeout);
    return (res == 0) ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
}
#else
inline HANDLE CreateSemaphore(LPSECURITY_ATTRIBUTES, LONG lInitialCount, LONG lMaximumCount, LPCSTR) {
    dispatch_semaphore_t sem = dispatch_semaphore_create(lInitialCount);
    return (HANDLE)sem;
}
inline BOOL ReleaseSemaphore(HANDLE hSemaphore, LONG lReleaseCount, LPLONG lpPreviousCount) {
    if (hSemaphore) {
        dispatch_semaphore_t sem = (dispatch_semaphore_t)hSemaphore;
        for (LONG i = 0; i < lReleaseCount; ++i) {
            dispatch_semaphore_signal(sem);
        }
        return TRUE;
    }
    return FALSE;
}
inline DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds) {
    if (!hHandle) return WAIT_FAILED;
    dispatch_semaphore_t sem = (dispatch_semaphore_t)hHandle;
    dispatch_time_t timeout = (dwMilliseconds == INFINITE) ? DISPATCH_TIME_FOREVER : dispatch_time(DISPATCH_TIME_NOW, (int64_t)dwMilliseconds * NSEC_PER_MSEC);
    intptr_t res = dispatch_semaphore_wait(sem, timeout);
    return (res == 0) ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
}
#endif

#else
inline HANDLE CreateSemaphore(LPSECURITY_ATTRIBUTES, LONG lInitialCount, LONG lMaximumCount, LPCSTR) {
    sem_t* sem = (sem_t*)malloc(sizeof(sem_t));
    if (sem) sem_init(sem, 0, (unsigned int)lInitialCount);
    return (HANDLE)sem;
}
inline BOOL ReleaseSemaphore(HANDLE hSemaphore, LONG lReleaseCount, LPLONG lpPreviousCount) {
    if (hSemaphore) {
        for (LONG i = 0; i < lReleaseCount; ++i) sem_post((sem_t*)hSemaphore);
        return TRUE;
    }
    return FALSE;
}
inline DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds) {
    if (!hHandle) return WAIT_FAILED;
    if (dwMilliseconds == INFINITE) {
        sem_wait((sem_t*)hHandle);
        return WAIT_OBJECT_0;
    } else {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += dwMilliseconds / 1000;
        ts.tv_nsec += (dwMilliseconds % 1000) * 1000000;
        if (ts.tv_nsec >= 1000000000) { ts.tv_sec++; ts.tv_nsec -= 1000000000; }
        if (sem_timedwait((sem_t*)hHandle, &ts) == 0) return WAIT_OBJECT_0;
        return WAIT_TIMEOUT;
    }
}
#endif

typedef unsigned (__stdcall *PTHREAD_START_ROUTINE)(void*);
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;

struct AndroidThreadData {
    PTHREAD_START_ROUTINE func;
    void* arg;
};

inline void* AndroidThreadFunc(void* p) {
    AndroidThreadData* d = (AndroidThreadData*)p;
    PTHREAD_START_ROUTINE func = d->func;
    void* arg = d->arg;
    free(d);
    func(arg);
    return NULL;
}

inline uintptr_t _beginthreadex(void* security, unsigned stack_size, PTHREAD_START_ROUTINE start_address, void* arglist, unsigned initflag, unsigned* thrdaddr) {
    pthread_t thread;
    AndroidThreadData* d = (AndroidThreadData*)malloc(sizeof(AndroidThreadData));
    d->func = start_address;
    d->arg = arglist;
    pthread_create(&thread, NULL, AndroidThreadFunc, d);
    if (thrdaddr) *thrdaddr = (unsigned)(uintptr_t)thread;
    return (uintptr_t)thread;
}

inline int MultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar) {
    if (!lpMultiByteStr) return 0;
    if (cbMultiByte < 0) cbMultiByte = (int)strlen(lpMultiByteStr);
    if (cchWideChar == 0) return cbMultiByte;
    int count = (cbMultiByte < cchWideChar) ? cbMultiByte : cchWideChar;
    if (lpWideCharStr) {
        for (int i = 0; i < count; ++i) {
            unsigned char c = (unsigned char)lpMultiByteStr[i];
            wchar_t wc = c;
            switch (c) {
                case 0xD0: wc = 0x011E; break; // Ğ
                case 0xF0: wc = 0x011F; break; // ğ
                case 0xDD: wc = 0x0130; break; // İ
                case 0xFD: wc = 0x0131; break; // ı
                case 0xDE: wc = 0x015E; break; // Ş
                case 0xFE: wc = 0x015F; break; // ş
                default: wc = c; break;
            }
            lpWideCharStr[i] = wc;
        }
        if (count < cchWideChar && cbMultiByte == count) lpWideCharStr[count] = 0;
    }
    return count;
}

inline int WideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar) {
    if (!lpWideCharStr) return 0;
    if (cchWideChar < 0) cchWideChar = (int)wcslen(lpWideCharStr);
    if (cbMultiByte == 0) return cchWideChar;
    int count = (cchWideChar < cbMultiByte) ? cchWideChar : cbMultiByte;
    if (lpMultiByteStr) {
        for (int i = 0; i < count; ++i) {
            wchar_t wc = lpWideCharStr[i];
            char c = (char)(wc & 0xff);
            switch (wc) {
                case 0x011E: c = (char)0xD0; break; // Ğ
                case 0x011F: c = (char)0xF0; break; // ğ
                case 0x0130: c = (char)0xDD; break; // İ
                case 0x0131: c = (char)0xFD; break; // ı
                case 0x015E: c = (char)0xDE; break; // Ş
                case 0x015F: c = (char)0xFE; break; // ş
                default: c = (char)(wc & 0xff); break;
            }
            lpMultiByteStr[i] = c;
        }
        if (count < cbMultiByte && cchWideChar == count) lpMultiByteStr[count] = 0;
    }
    return count;
}

#define BI_RGB 0
#define DIB_RGB_COLORS 0
#define FW_NORMAL 400
#define FW_BOLD 700
#define OUT_DEFAULT_PRECIS 0
#define CLIP_DEFAULT_PRECIS 0
#define ANTIALIASED_QUALITY 4
#define DEFAULT_PITCH 0
#define ERROR_ALREADY_EXISTS 183L
#ifndef _countof
#define _countof(a) (sizeof(a)/sizeof(*(a)))
#endif

#ifndef COLORREF
typedef DWORD COLORREF;
#endif

#ifndef RGB
#define RGB(r,g,b) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#endif

typedef void* HGDIOBJ;
typedef void* HFONT;
typedef void* HBITMAP;
typedef void* HRGN;

typedef struct _ABCFLOAT {
    FLOAT abcfA;
    FLOAT abcfB;
    FLOAT abcfC;
} ABCFLOAT, *PABCFLOAT, *LPABCFLOAT;

#define TRANSPARENT 1
#define OPAQUE 2

inline BOOL DeleteObject(HGDIOBJ) { return TRUE; }
inline BOOL DeleteDC(HDC) { return TRUE; }
inline HDC CreateCompatibleDC(HDC) { return (HDC)1; }
inline HGDIOBJ SelectObject(HDC, HGDIOBJ) { return NULL; }
inline COLORREF SetTextColor(HDC, COLORREF) { return 0; }
inline int SetBkMode(HDC, int) { return 1; }
inline COLORREF SetBkColor(HDC, COLORREF) { return 0; }
inline BOOL TextOut(HDC, int, int, LPCSTR, int) { return TRUE; }
inline BOOL TextOutA(HDC, int, int, LPCSTR, int) { return TRUE; }
inline BOOL TextOutW(HDC, int, int, LPCWSTR, int) { return TRUE; }
inline HFONT CreateFontIndirect(const LOGFONT*) { return (HFONT)1; }
inline HFONT CreateFontIndirectA(const LOGFONT*) { return (HFONT)1; }
inline BOOL GetTextExtentPoint32(HDC, LPCSTR, int, LPSIZE lpSize) { if (lpSize) { lpSize->cx = 8; lpSize->cy = 16; } return TRUE; }
inline BOOL GetTextExtentPoint32A(HDC h, LPCSTR s, int c, LPSIZE l) { return GetTextExtentPoint32(h, s, c, l); }
inline BOOL GetTextExtentPoint32W(HDC, LPCWSTR, int, LPSIZE lpSize) { if (lpSize) { lpSize->cx = 8; lpSize->cy = 16; } return TRUE; }
inline BOOL GetCharABCWidthsFloatW(HDC, UINT, UINT, LPABCFLOAT) { return TRUE; }
inline LPSTR CharNextExA(WORD CodePage, LPCSTR lpCurrentChar, DWORD dwFlags) { return (LPSTR)(lpCurrentChar ? lpCurrentChar + 1 : NULL); }
inline LPSTR CharPrevExA(WORD CodePage, LPCSTR lpStart, LPCSTR lpCurrentChar, DWORD dwFlags) { return (LPSTR)(lpCurrentChar > lpStart ? lpCurrentChar - 1 : lpStart); }
inline HBITMAP CreateDIBSection(HDC, const BITMAPINFO*, UINT, void** ppvBits, HANDLE, DWORD) {
    if (ppvBits) *ppvBits = calloc(1, 1024*1024*4);
    return (HBITMAP)1;
}

inline HANDLE CreateMutex(LPSECURITY_ATTRIBUTES, BOOL, LPCSTR) { return (HANDLE)1; }
inline HANDLE CreateMutexA(LPSECURITY_ATTRIBUTES sa, BOOL b, LPCSTR n) { return CreateMutex(sa, b, n); }
inline BOOL ReleaseMutex(HANDLE) { return TRUE; }
#define SM_CXFULLSCREEN 16
#define SM_CYFULLSCREEN 17

inline BOOL IsIconic(HWND) { return FALSE; }
inline int SetDIBitsToDevice(HDC, int, int, DWORD, DWORD, int, int, UINT, UINT, const void*, const BITMAPINFO*, UINT) { return 0; }

inline int memcpy_s(void* dst, size_t dstsz, const void* src, size_t count) {
    if (dst && src && count > 0) memcpy(dst, src, count);
    return 0;
}

#define GWL_USERDATA (-21)
#define GWL_EXSTYLE (-20)
#define GWL_STYLE (-16)
#define GWL_WNDPROC (-4)
#define WM_SIZE 0x0005
#define WM_ACTIVATEAPP 0x001C
#define WA_INACTIVE 0
#define WA_ACTIVE 1
#define WA_CLICKACTIVE 2
#define SIZE_MINIMIZED 1
#define SIZE_RESTORED 0
#define SIZE_MAXIMIZED 2
#define SW_HIDE 0
#define SW_SHOW 5
#define SW_MINIMIZE 6
#define SW_RESTORE 9

typedef const wchar_t* PCWSTR;
typedef OSVERSIONINFOA OSVERSIONINFO;
typedef OSVERSIONINFOA* LPOSVERSIONINFO;

inline LRESULT DefWindowProc(HWND, UINT, WPARAM, LPARAM) { return 0; }
inline LRESULT DefWindowProcA(HWND h, UINT m, WPARAM w, LPARAM l) { return DefWindowProc(h, m, w, l); }
inline BOOL InvalidateRect(HWND, const RECT*, BOOL) { return TRUE; }
inline BOOL IsWindow(HWND) { return TRUE; }
inline BOOL DestroyWindow(HWND) { return TRUE; }
inline HWND CreateWindow(LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID) { return (HWND)1; }
inline HWND CreateWindowA(LPCSTR cn, LPCSTR wn, DWORD ws, int x, int y, int w, int h, HWND p, HMENU m, HINSTANCE i, LPVOID pv) { return (HWND)1; }
inline BOOL GetVersionEx(LPOSVERSIONINFO) { return TRUE; }
inline BOOL GetVersionExA(LPOSVERSIONINFOA) { return TRUE; }
inline DWORD GetCurrentProcessId() { return (DWORD)getpid(); }
inline HRESULT SetCurrentProcessExplicitAppUserModelID(PCWSTR) { return 0; }
inline LONG_PTR SetWindowLongPtr(HWND, int, LONG_PTR) { return 0; }
inline LONG_PTR SetWindowLongPtrA(HWND h, int n, LONG_PTR l) { return SetWindowLongPtr(h, n, l); }
inline LONG_PTR GetWindowLongPtr(HWND, int) { return 0; }
inline LONG_PTR GetWindowLongPtrA(HWND h, int n) { return GetWindowLongPtr(h, n); }
inline LONG SetWindowLong(HWND, int, LONG) { return 0; }
inline LONG SetWindowLongA(HWND h, int n, LONG l) { return SetWindowLong(h, n, l); }
inline LONG GetWindowLong(HWND, int) { return 0; }
inline LONG GetWindowLongA(HWND h, int n) { return GetWindowLong(h, n); }
inline BOOL ShowWindow(HWND, int) { return TRUE; }
inline BOOL UpdateWindow(HWND) { return TRUE; }
inline BOOL MoveWindow(HWND, int, int, int, int, BOOL) { return TRUE; }
inline BOOL GetClientRect(HWND, LPRECT lpRect) {
    if (lpRect) {
        lpRect->left = 0;
        lpRect->top = 0;
        lpRect->right = (g_nAndroidScreenWidth > 0) ? g_nAndroidScreenWidth : 2400;
        lpRect->bottom = (g_nAndroidScreenHeight > 0) ? g_nAndroidScreenHeight : 1080;
    }
    return TRUE;
}
inline BOOL GetWindowRect(HWND, LPRECT lpRect) { return GetClientRect(NULL, lpRect); }
inline BOOL AdjustWindowRect(LPRECT, DWORD, BOOL) { return TRUE; }
inline BOOL AdjustWindowRectEx(LPRECT, DWORD, BOOL, DWORD) { return TRUE; }

#define SM_CXSCREEN 0
#define SM_CYSCREEN 1
#define SWP_NOSIZE 0x0001
#define SWP_NOMOVE 0x0002
#define SWP_NOZORDER 0x0004
#define SWP_FRAMECHANGED 0x0020
#define IDI_APPLICATION ((LPCSTR)32512)
#define IDC_ARROW ((LPCSTR)32512)

typedef struct tagWNDCLASSA {
    UINT style;
    WNDPROC lpfnWndProc;
    int cbClsExtra;
    int cbWndExtra;
    HINSTANCE hInstance;
    HICON hIcon;
    HCURSOR hCursor;
    HBRUSH hbrBackground;
    LPCSTR lpszMenuName;
    LPCSTR lpszClassName;
} WNDCLASSA, *PWNDCLASSA, *LPWNDCLASSA;
typedef WNDCLASSA WNDCLASS;

inline BOOL SetRect(LPRECT lprc, int xLeft, int yTop, int xRight, int yBottom) {
    if (lprc) { lprc->left = xLeft; lprc->top = yTop; lprc->right = xRight; lprc->bottom = yBottom; }
    return TRUE;
}
inline DWORD GetWindowStyle(HWND) { return 0; }
inline DWORD GetWindowExStyle(HWND) { return 0; }
inline HMENU GetMenu(HWND) { return NULL; }
inline BOOL SetWindowText(HWND, LPCSTR) { return TRUE; }
inline BOOL SetWindowTextA(HWND h, LPCSTR s) { return SetWindowText(h, s); }
inline HCURSOR LoadCursor(HINSTANCE, LPCSTR) { return (HCURSOR)1; }
inline HCURSOR LoadCursorA(HINSTANCE h, LPCSTR s) { return LoadCursor(h, s); }
inline HGDIOBJ GetStockObject(int) { return (HGDIOBJ)1; }
inline ATOM RegisterClass(const WNDCLASS*) { return 1; }
#define IMAGE_CURSOR 2
#define LR_VGACOLOR 0x0080
#define VK_SCROLL 0x91

inline BOOL DestroyCursor(HCURSOR) { return TRUE; }
inline int ShowCursor(BOOL bShow) {
	static int s_cursorCount = 0;
	return bShow ? ++s_cursorCount : --s_cursorCount;
}
inline HCURSOR SetCursor(HCURSOR) { return (HCURSOR)1; }
inline BOOL ClientToScreen(HWND, LPPOINT) { return TRUE; }
inline BOOL SetCursorPos(int, int) { return TRUE; }
inline BOOL SetFileAttributes(LPCSTR, DWORD) { return TRUE; }
inline BOOL SetFileAttributesA(LPCSTR f, DWORD a) { return SetFileAttributes(f, a); }
inline HANDLE LoadImage(HINSTANCE, LPCSTR, UINT, int, int, UINT) { return (HANDLE)1; }
inline HANDLE LoadImageA(HINSTANCE h, LPCSTR n, UINT t, int cx, int cy, UINT f) { return LoadImage(h, n, t, cx, cy, f); }

inline HMODULE LoadLibraryA(LPCSTR) { return (HMODULE)NULL; }
inline HMODULE LoadLibrary(LPCSTR) { return (HMODULE)NULL; }
inline FARPROC GetProcAddress(HMODULE, LPCSTR) { return (FARPROC)NULL; }
inline BOOL FreeLibrary(HMODULE) { return TRUE; }
inline HINSTANCE ShellExecuteA(HWND, LPCSTR, LPCSTR, LPCSTR, LPCSTR, int) { return (HINSTANCE)33; }
inline HINSTANCE ShellExecute(HWND, LPCSTR, LPCSTR, LPCSTR, LPCSTR, int) { return (HINSTANCE)33; }
inline SHORT GetKeyState(int) { return 0; }

#define DM_BITSPERPEL 0x00040000L
#define DM_PELSWIDTH 0x00080000L
#define DM_PELSHEIGHT 0x00100000L
#define CDS_FULLSCREEN 0x00000004
#define DISP_CHANGE_SUCCESSFUL 0
#define DISP_CHANGE_RESTART 1
#define DISP_CHANGE_FAILED -1

#define WM_KEYDOWN 0x0100
#define WM_KEYUP 0x0101
#define WM_CHAR 0x0102
#define WM_SYSKEYDOWN 0x0104
#define WM_SYSKEYUP 0x0105
#define WM_MOUSEMOVE 0x0200
#define WM_LBUTTONDOWN 0x0201
#define WM_LBUTTONUP 0x0202
#define WM_LBUTTONDBLCLK 0x0203
#define WM_RBUTTONDOWN 0x0204
#define WM_RBUTTONUP 0x0205
#define WM_RBUTTONDBLCLK 0x0206
#define WM_MBUTTONDOWN 0x0207
#define WM_MBUTTONUP 0x0208
#define WM_MBUTTONDBLCLK 0x0209
#define WM_MOUSEWHEEL 0x020A
#define WM_EXITSIZEMOVE 0x0232
#define WM_SETCURSOR 0x0020
#define WM_KILLFOCUS 0x0008
#define WM_SETFOCUS 0x0007
#define WM_ACTIVATE 0x0006
#define WM_PAINT 0x000F
#define WM_ERASEBKGND 0x0014
#define WM_COMMAND 0x0111
#define WM_SYSCOMMAND 0x0112
#define WM_TIMER 0x0113
#define WM_HSCROLL 0x0114
#define WM_VSCROLL 0x0115

#define VK_F1 0x70
#define VK_F2 0x71
#define VK_F3 0x72
#define VK_F4 0x73
#define VK_F5 0x74
#define VK_F6 0x75
#define VK_F7 0x76
#define VK_F8 0x77
#define VK_F9 0x78
#define VK_F10 0x79
#define VK_F11 0x7A
#define VK_F12 0x7B

#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#define GET_WHEEL_DELTA_WPARAM(wParam) ((short)HIWORD(wParam))

inline HWND GetCapture() { return (HWND)0; }

#define WM_INPUTLANGCHANGE 0x0051
#define WM_IME_STARTCOMPOSITION 0x010D
#define WM_IME_ENDCOMPOSITION 0x010E
#define WM_IME_COMPOSITION 0x010F
#define WM_IME_SETCONTEXT 0x0281
#define WM_IME_NOTIFY 0x0282
#define WM_IME_CONTROL 0x0283
#define WM_IME_COMPOSITIONFULL 0x0284
#define WM_IME_SELECT 0x0285
#define WM_IME_CHAR 0x0286
#define WM_IME_REQUEST 0x0288
#define WM_IME_KEYDOWN 0x0290
#define WM_IME_KEYUP 0x0291

#define ISC_SHOWUICOMPOSITIONWINDOW 0x80000000
#define ISC_SHOWUIALLCANDIDATEWINDOW 0x0000000F

typedef struct _devicemodeA {
    BYTE dmDeviceName[32];
    WORD dmSpecVersion;
    WORD dmDriverVersion;
    WORD dmSize;
    WORD dmDriverExtra;
    DWORD dmFields;
    short dmOrientation;
    short dmPaperSize;
    short dmPaperLength;
    short dmPaperWidth;
    short dmScale;
    short dmCopies;
    short dmDefaultSource;
    short dmPrintQuality;
    short dmColor;
    short dmDuplex;
    short dmYResolution;
    short dmTTOption;
    short dmCollate;
    BYTE dmFormName[32];
    WORD dmLogPixels;
    DWORD dmBitsPerPel;
    DWORD dmPelsWidth;
    DWORD dmPelsHeight;
    DWORD dmDisplayFlags;
    DWORD dmDisplayFrequency;
    DWORD dmICMMethod;
    DWORD dmICMIntent;
    DWORD dmMediaType;
    DWORD dmDitherType;
    DWORD dmReserved1;
    DWORD dmReserved2;
    DWORD dmPanningWidth;
    DWORD dmPanningHeight;
} DEVMODEA, *PDEVMODEA, *LPDEVMODEA;
typedef DEVMODEA DEVMODE;

inline HWND SetCapture(HWND) { return (HWND)1; }
inline BOOL ReleaseCapture() { return TRUE; }
inline LONG ChangeDisplaySettings(const DEVMODE*, DWORD) { return DISP_CHANGE_SUCCESSFUL; }
inline BOOL GetCursorPos(LPPOINT lpPoint) {
    if (lpPoint) {
        lpPoint->x = g_nAndroidMouseX;
        lpPoint->y = g_nAndroidMouseY;
    }
    return TRUE;
}
inline BOOL ScreenToClient(HWND, LPPOINT) { return TRUE; }
inline BOOL FlashWindowEx(FLASHWINFO*) { return TRUE; }
#include "ddraw.h"
#endif

#ifdef __cplusplus
}
#endif
