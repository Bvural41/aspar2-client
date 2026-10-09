#include "StdAfx.h"
#ifdef __ANDROID__
#include <android/log.h>
#endif

#include <time.h>
#include <stdio.h>
#include "Debug.h"
#include "Singleton.h"
#include "Timer.h"

const DWORD DEBUG_STRING_MAX_LEN = 4096;

static int isLogFile = false;
HWND g_PopupHwnd = NULL;

class CLogFile : public CSingleton<CLogFile>
{
	public:
		CLogFile() : m_fp(NULL)
		{
		}

		virtual ~CLogFile()
		{
			if (m_fp)
				fclose(m_fp);

			m_fp = NULL;
		}

		void Initialize()
		{
			m_fp = fopen("log.txt", "w");
		}

		void Write(const char * c_pszMsg)
		{
			if (!m_fp)
				return;

			time_t ct = time(0);
			struct tm ctm = *localtime(&ct);

			fprintf(m_fp, "%02d%02d %02d:%02d:%05d :: %s", 
				ctm.tm_mon + 1, 
				ctm.tm_mday,
				ctm.tm_hour,
				ctm.tm_min,
				ELTimer_GetMSec() % 60000,
				c_pszMsg);

			fflush(m_fp);
		}

	protected:
		FILE *	m_fp;
};

static CLogFile gs_logfile;

static UINT gs_uLevel=0;

void SetLogLevel(UINT uLevel)
{
	gs_uLevel=uLevel;
}

void Log(UINT uLevel, const char* c_szMsg)
{
	if (uLevel>=gs_uLevel)
		Trace(c_szMsg);
}

void Logn(UINT uLevel, const char* c_szMsg)
{
	if (uLevel>=gs_uLevel)
		Tracen(c_szMsg);
}

void Logf(UINT uLevel, const char* c_szFormat, ...)
{
	if (uLevel<gs_uLevel)
		return;
	
	char szBuf[DEBUG_STRING_MAX_LEN+1];

	va_list args;
	va_start(args, c_szFormat);
	_vsnprintf(szBuf, sizeof(szBuf), c_szFormat, args);
	va_end(args);
#ifdef _DEBUG
	OutputDebugString(szBuf);	
	fputs(szBuf, stdout);
#endif

	if (isLogFile)
		LogFile(szBuf);
}

void Lognf(UINT uLevel, const char* c_szFormat, ...)
{
	if (uLevel<gs_uLevel)
		return;

	va_list args;
	va_start(args, c_szFormat);

	char szBuf[DEBUG_STRING_MAX_LEN+2];
	int len = vsnprintf(szBuf, sizeof(szBuf)-2, c_szFormat, args);
	if (len < 0 || len >= (int)(sizeof(szBuf) - 2))
		len = sizeof(szBuf) - 2;

	szBuf[len] = '\n';
	szBuf[len + 1] = '\0';
	va_end(args);
#ifdef __ANDROID__
	__android_log_print(ANDROID_LOG_INFO, "Metin2Trace", "%s", szBuf);
#endif
#ifdef _DEBUG
	OutputDebugString(szBuf);	
	puts(szBuf);
#endif

	if (isLogFile)
		LogFile(szBuf);
}


void Trace(const char * c_szMsg)
{
#ifdef __ANDROID__
	__android_log_print(ANDROID_LOG_INFO, "Metin2Trace", "%s", c_szMsg);
#endif
#ifdef _DEBUG
	OutputDebugString(c_szMsg);
	printf("%s", c_szMsg);
#endif

	if (isLogFile)
		LogFile(c_szMsg);
}

void Tracen(const char* c_szMsg)
{
#ifdef __ANDROID__
	__android_log_print(ANDROID_LOG_INFO, "Metin2Trace", "%s", c_szMsg);
#endif
#ifdef _DEBUG
	char szBuf[DEBUG_STRING_MAX_LEN+1];
	_snprintf(szBuf, sizeof(szBuf), "%s\n", c_szMsg);
	OutputDebugString(szBuf);
	puts(c_szMsg);

	if (isLogFile)
		LogFile(szBuf);

	puts(c_szMsg);
	putc('\n', stdout);
#else
	if (isLogFile)
	{
		LogFile(c_szMsg);
		LogFile("\n");
	}
#endif
}

void Tracenf(const char* c_szFormat, ...)
{
	va_list args;
	va_start(args, c_szFormat);

	char szBuf[DEBUG_STRING_MAX_LEN+2];
	int len = vsnprintf(szBuf, sizeof(szBuf)-2, c_szFormat, args);
	if (len < 0 || len >= (int)(sizeof(szBuf) - 2))
		len = sizeof(szBuf) - 2;

	szBuf[len] = '\n';
	szBuf[len + 1] = '\0';
	va_end(args);
#ifdef _DEBUG
	OutputDebugString(szBuf);	
	printf("%s", szBuf);
#endif

	if (isLogFile)
		LogFile(szBuf);
}

void Tracef(const char* c_szFormat, ...)
{
	char szBuf[DEBUG_STRING_MAX_LEN+1];

	va_list args;
	va_start(args, c_szFormat);
	_vsnprintf(szBuf, sizeof(szBuf), c_szFormat, args);
	va_end(args);

#ifdef _DEBUG
	OutputDebugString(szBuf);	
	fputs(szBuf, stdout);
#endif

	if (isLogFile)
		LogFile(szBuf);
}

void TraceError(const char* c_szFormat, ...)
{
	char szBuf[DEBUG_STRING_MAX_LEN+2];

	strncpy(szBuf, "SYSERR: ", 9);
	int prefixLen = 8;

	va_list args;
	va_start(args, c_szFormat);
	int written = vsnprintf(szBuf + prefixLen, sizeof(szBuf) - (prefixLen + 2), c_szFormat, args);
	va_end(args);

	int totalLen = prefixLen;
	if (written < 0 || written >= (int)(sizeof(szBuf) - (prefixLen + 2)))
	{
		totalLen = sizeof(szBuf) - 2;
	}
	else
	{
		totalLen = prefixLen + written;
	}

	szBuf[totalLen] = '\n';
	szBuf[totalLen + 1] = '\0';

#if defined(__ANDROID__) || defined(__APPLE__) || defined(__IOS__)
#if defined(__ANDROID__)
	__android_log_print(ANDROID_LOG_ERROR, "Metin2SysErr", "%s", szBuf);
#elif defined(__OBJC__)
	NSLog(@"[Aspar2 iOS SYSERR] %s", szBuf);
#endif
	static FILE* s_pSysErrFile = nullptr;
	static bool s_bSysErrInit = false;
	if (!s_bSysErrInit)
	{
		s_bSysErrInit = true;
#ifdef __OBJC__
		NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
		NSString *docsPath = paths.firstObject;
		if (docsPath) {
			NSString *syserrPath = [docsPath stringByAppendingPathComponent:@"syserr.txt"];
			s_pSysErrFile = fopen([syserrPath UTF8String], "a");
		}
#endif
		if (!s_pSysErrFile)
			s_pSysErrFile = fopen("syserr.txt", "a");
		if (!s_pSysErrFile)
			s_pSysErrFile = fopen("/sdcard/aspar2/syserr.txt", "a");
		if (!s_pSysErrFile)
			s_pSysErrFile = fopen("/storage/emulated/0/aspar2/syserr.txt", "a");
	}
	if (s_pSysErrFile)
	{
		time_t tNow = time(0);
		struct tm tmNow = *localtime(&tNow);
		fprintf(s_pSysErrFile, "%02d%02d %02d:%02d:%05d :: %s",
				tmNow.tm_mon + 1, tmNow.tm_mday, tmNow.tm_hour, tmNow.tm_min,
				ELTimer_GetMSec() % 60000, szBuf + 8);
		fflush(s_pSysErrFile);
	}
#endif

	time_t ct = time(0);
	struct tm ctm = *localtime(&ct);

	fprintf(stderr, "%02d%02d %02d:%02d:%05d :: %s", 
					ctm.tm_mon + 1, 
					ctm.tm_mday,
					ctm.tm_hour,
					ctm.tm_min,
					ELTimer_GetMSec() % 60000,
					szBuf + 8);
	fflush(stderr);
	
#ifdef _DEBUG
	OutputDebugString(szBuf);
	fputs(szBuf, stdout);
#endif

	if (isLogFile)
		LogFile(szBuf);
}

void TraceErrorWithoutEnter(const char* c_szFormat, ...)
{
	char szBuf[DEBUG_STRING_MAX_LEN];

	va_list args;
	va_start(args, c_szFormat);
	_vsnprintf(szBuf, sizeof(szBuf), c_szFormat, args);
	va_end(args);

#ifdef __ANDROID__
	__android_log_print(ANDROID_LOG_ERROR, "Metin2SysErr", "%s", szBuf);
#endif

	time_t ct = time(0);
	struct tm ctm = *localtime(&ct);

	fprintf(stderr, "%02d%02d %02d:%02d:%05d :: %s", 
					ctm.tm_mon + 1, 
					ctm.tm_mday,
					ctm.tm_hour,
					ctm.tm_min,
					ELTimer_GetMSec() % 60000,
					szBuf + 8);
	fflush(stderr);

#ifdef _DEBUG
	OutputDebugString(szBuf);
	fputs(szBuf, stdout);
#endif

	if (isLogFile)
		LogFile(szBuf);
}

void LogBoxf(const char* c_szFormat, ...)
{
	va_list args;
	va_start(args, c_szFormat);

	char szBuf[2048];
	_vsnprintf(szBuf, sizeof(szBuf), c_szFormat, args);

	LogBox(szBuf);
}

void LogBox(const char* c_szMsg, const char * c_szCaption, HWND hWnd)
{
	if (!hWnd)
		hWnd = g_PopupHwnd;

	MessageBox(hWnd, c_szMsg, c_szCaption ? c_szCaption : "LOG", MB_OK);
	Tracen(c_szMsg);
}

void LogFile(const char * c_szMsg)
{
	CLogFile::Instance().Write(c_szMsg);
}

void LogFilef(const char * c_szMessage, ...)
{
	va_list args;
	va_start(args, c_szMessage);
	char szBuf[DEBUG_STRING_MAX_LEN+1];
	_vsnprintf(szBuf, sizeof(szBuf), c_szMessage, args);

	CLogFile::Instance().Write(szBuf);
}

void OpenLogFile(bool bUseLogFIle)
{
	const char* c_szLogDir = "syserr";
	CreateDirectory(c_szLogDir, NULL);

	time_t rawtime;
	struct tm* timeinfo;
	char buffer[80];
	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, sizeof(buffer), "./syserr/%Y%m%d.%H%M%S.txt", timeinfo);
	const char* path(buffer);

#ifndef _DISTRIBUTE 
	freopen(path, "w", stderr);

	if (bUseLogFIle)
	{
		isLogFile = true;
		CLogFile::Instance().Initialize();
	}
#endif
}

void OpenConsoleWindow()
{
#if !defined(__ANDROID__) && !defined(__APPLE__)
	AllocConsole();

	freopen("CONOUT$", "a", stdout);
	freopen("CONIN$", "r", stdin);
#endif

}
