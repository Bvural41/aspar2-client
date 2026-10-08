#include "StdAfx.h"
#include "PythonApplication.h"

#if defined(__ANDROID__) || defined(__APPLE__)
extern "C" {
#ifdef __ANDROID__
void Android_ShowWebPage(const char* url);
void Android_HideWebPage();
bool Android_IsWebShowing();
#define Platform_ShowWebPage Android_ShowWebPage
#define Platform_HideWebPage Android_HideWebPage
#define Platform_IsWebShowing Android_IsWebShowing
#else
void IOS_ShowWebPage(const char* url);
void IOS_HideWebPage();
bool IOS_IsWebShowing();
#define Platform_ShowWebPage IOS_ShowWebPage
#define Platform_HideWebPage IOS_HideWebPage
#define Platform_IsWebShowing IOS_IsWebShowing
#endif
}

bool CPythonApplication::IsWebPageMode()
{
	return Platform_IsWebShowing();
}

void CPythonApplication::ShowWebPage(const char* c_szURL, const RECT& c_rcWebPage)
{
	Platform_ShowWebPage(c_szURL);
}

void CPythonApplication::MoveWebPage(const RECT& c_rcWebPage)
{
}

void CPythonApplication::HideWebPage()
{
	Platform_HideWebPage();
}
#else
#ifdef CEF_BROWSER
#include "CefWebBrowser.h"
#else
#include "../CWebBrowser/CWebBrowser.h"
#endif

bool CPythonApplication::IsWebPageMode()
{
#ifdef CEF_BROWSER
	return CefWebBrowser_IsVisible() ? true : false;
#else
	return WebBrowser_IsVisible() ? true : false;
#endif
}

void CPythonApplication::ShowWebPage(const char* c_szURL, const RECT& c_rcWebPage)
{
#ifdef CEF_BROWSER
	if (CefWebBrowser_IsVisible())
		return;
#else
	if (WebBrowser_IsVisible())
		return;
#endif

#ifndef CEF_BROWSER
	m_grpDevice.EnableWebBrowserMode(c_rcWebPage);
#endif

#ifdef CEF_BROWSER
	if (!CefWebBrowser_Show(GetWindowHandle(), c_szURL, &c_rcWebPage))
#else
	if (!WebBrowser_Show(GetWindowHandle(), c_szURL, &c_rcWebPage))
#endif
	{
		// TraceError("CREATE_WEBBROWSER_ERROR:%d", GetLastError());
	}

	SetCursorMode(CURSOR_MODE_HARDWARE);
}

void CPythonApplication::MoveWebPage(const RECT& c_rcWebPage)
{
#ifdef CEF_BROWSER
	if (CefWebBrowser_IsVisible())
#else
	if (WebBrowser_IsVisible())
#endif
	{
#ifndef CEF_BROWSER
		m_grpDevice.MoveWebBrowserRect(c_rcWebPage);
#endif

#ifdef CEF_BROWSER
		CefWebBrowser_Move(&c_rcWebPage);
#else
		WebBrowser_Move(&c_rcWebPage);
#endif
	}
}

void CPythonApplication::HideWebPage()
{
#ifdef CEF_BROWSER
	if (CefWebBrowser_IsVisible())
	{
		CefWebBrowser_Hide();
#else
	if (WebBrowser_IsVisible())
	{
		WebBrowser_Hide();
#endif

#ifndef CEF_BROWSER
		m_grpDevice.DisableWebBrowserMode();	
#endif

		if (m_pySystem.IsSoftwareCursor())
			SetCursorMode(CURSOR_MODE_SOFTWARE);
		else
			SetCursorMode(CURSOR_MODE_HARDWARE);
	}
}
#endif
