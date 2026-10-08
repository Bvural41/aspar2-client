#pragma once
#include <stdint.h>
#include "windows.h"

typedef uint32_t MMRESULT;
#define TIMERR_NOERROR 0

typedef void* HWAVEIN;
typedef void* HWAVEOUT;
typedef HWAVEOUT* LPHWAVEOUT;
typedef void* HMIDIOUT;
typedef HMIDIOUT* LPHMIDIOUT;
typedef void* HMIXER;

typedef struct wavehdr_tag {
    LPSTR      lpData;
    DWORD      dwBufferLength;
    DWORD      dwBytesRecorded;
    DWORD_PTR  dwUser;
    DWORD      dwFlags;
    DWORD      dwLoops;
    struct wavehdr_tag *lpNext;
    DWORD_PTR  reserved;
} WAVEHDR, *PWAVEHDR, *LPWAVEHDR;

typedef struct tagWAVEFORMAT {
    WORD        wFormatTag;
    WORD        nChannels;
    DWORD       nSamplesPerSec;
    DWORD       nAvgBytesPerSec;
    WORD        nBlockAlign;
} WAVEFORMAT, *PWAVEFORMAT, *LPWAVEFORMAT;

typedef struct tagPCMWAVEFORMAT {
    WAVEFORMAT  wf;
    WORD        wBitsPerSample;
} PCMWAVEFORMAT, *PPCMWAVEFORMAT, *LPPCMWAVEFORMAT;

typedef struct tagWAVEFORMATEX {
    WORD        wFormatTag;
    WORD        nChannels;
    DWORD       nSamplesPerSec;
    DWORD       nAvgBytesPerSec;
    WORD        nBlockAlign;
    WORD        wBitsPerSample;
    WORD        cbSize;
} WAVEFORMATEX, *PWAVEFORMATEX, *LPWAVEFORMATEX;

typedef struct midihdr_tag {
    LPSTR       lpData;
    DWORD       dwBufferLength;
    DWORD       dwBytesRecorded;
    DWORD_PTR   dwUser;
    DWORD       dwFlags;
    struct midihdr_tag *lpNext;
    DWORD_PTR   reserved;
    DWORD       dwOffset;
    DWORD_PTR   dwReserved[8];
} MIDIHDR, *PMIDIHDR, *LPMIDIHDR;
