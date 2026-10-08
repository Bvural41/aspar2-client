#include "StdAfx.h"
#include "MsWindow.h"
#ifndef SDL_MAIN_HANDLED
#define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include "GrpBase.h"


#include <windowsx.h>
#define DISABLE_TASKBAR_GROUPING
#ifdef DISABLE_TASKBAR_GROUPING
#include <Shobjidl.h>
#endif

HINSTANCE CMSWindow::ms_hInstance = NULL;

static WNDPROC s_pOldSDLWndProc = NULL;

LRESULT CALLBACK MSWindowProcedure(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{	
	CMSWindow * pWnd = (CMSWindow *) GetWindowLong(hWnd, GWL_USERDATA);

	if (pWnd)
		return pWnd->WindowProcedure(hWnd, uiMsg, wParam, lParam);	

#if !defined(__ANDROID__) && !defined(__APPLE__)
	if (s_pOldSDLWndProc)
		return CallWindowProc(s_pOldSDLWndProc, hWnd, uiMsg, wParam, lParam);
#endif

	return DefWindowProc(hWnd, uiMsg, wParam, lParam);
}

LRESULT CMSWindow::WindowProcedure(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uiMsg)
	{
		case WM_SIZE:
			OnSize(wParam, lParam);
			break;

		case WM_ACTIVATEAPP:
			m_isActive = (wParam == WA_ACTIVE) || (wParam == WA_CLICKACTIVE);
			break;
	}

#if !defined(__ANDROID__) && !defined(__APPLE__)
	if (s_pOldSDLWndProc)
		return CallWindowProc(s_pOldSDLWndProc, hWnd, uiMsg, wParam, lParam);
#endif

	return DefWindowProc(hWnd, uiMsg, wParam, lParam);
}

void CMSWindow::OnSize(WPARAM wParam, LPARAM /*lParam*/)
{
	if (wParam == SIZE_MINIMIZED) 
	{
		InvalidateRect(m_hWnd, NULL, true);
		m_isActive = false;        
		m_isVisible = false;
	}
	else
	{
		m_isActive = true;
		m_isVisible = true;
	}
}

void CMSWindow::Destroy()
{
	if (m_pSDLWindow)
	{
		if (m_hWnd && s_pOldSDLWndProc)
		{
			SetWindowLong(m_hWnd, GWL_WNDPROC, (LONG)s_pOldSDLWndProc);
			s_pOldSDLWndProc = NULL;
		}
		SDL_DestroyWindow(m_pSDLWindow);
		m_pSDLWindow = NULL;
	}
	else if (m_hWnd)
	{
		if (IsWindow(m_hWnd))
			DestroyWindow(m_hWnd);
	}

	m_hWnd = NULL;
	m_isVisible = false;
	m_isActive = false;
}

bool CMSWindow::Create(const char* c_szName, int brush, DWORD cs, DWORD ws, HICON hIcon, int iCursorResource, int width, int height)
{
#if defined(__ANDROID__) || defined(__APPLE__)
	m_hWnd = (HWND)1;
	m_isVisible = true;
	m_isActive = true;
	return true;
#else
	Destroy();

	if (!SDL_WasInit(SDL_INIT_VIDEO))
	{
		SDL_SetMainReady();
		if (SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
		{
			TraceError("SDL_InitSubSystem failed: %s", SDL_GetError());
			return false;
		}
	}

	Uint32 flags = SDL_WINDOW_HIDDEN;
	if (ws & WS_MAXIMIZE)
		flags |= SDL_WINDOW_MAXIMIZED;
	if (ws & WS_MINIMIZE)
		flags |= SDL_WINDOW_MINIMIZED;
	if (ws & WS_THICKFRAME)
		flags |= SDL_WINDOW_RESIZABLE;
	if (ws == WS_POPUP || ((ws & WS_POPUP) && !(ws & WS_CAPTION)))
		flags |= SDL_WINDOW_BORDERLESS;

#if defined(USE_OPENGL_ES)
	flags |= SDL_WINDOW_OPENGL;
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#endif

	// Convert ANSI / CP1254 to UTF-8 for SDL
	wchar_t wszName[1024];
	MultiByteToWideChar(CP_ACP, 0, c_szName, -1, wszName, 1024);
	char szUtf8Name[2048];
	WideCharToMultiByte(CP_UTF8, 0, wszName, -1, szUtf8Name, 2048, NULL, NULL);

	m_pSDLWindow = SDL_CreateWindow(
		szUtf8Name,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		width,
		height,
		flags
	);

	if (!m_pSDLWindow)
	{
		TraceError("SDL_CreateWindow failed: %s", SDL_GetError());
		return false;
	}

#if defined(USE_OPENGL_ES)
	CGraphicBase::SetSDLWindow(m_pSDLWindow);
#endif

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	if (!SDL_GetWindowWMInfo(m_pSDLWindow, &wmInfo))
	{
		TraceError("SDL_GetWindowWMInfo failed: %s", SDL_GetError());
		SDL_DestroyWindow(m_pSDLWindow);
		m_pSDLWindow = NULL;
		return false;
	}

	m_hWnd = wmInfo.info.win.window;

	// Enforce exact Win32 window style on m_hWnd so caption and borders are properly created by DWM
	SetWindowLong(m_hWnd, GWL_STYLE, ws);
	SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

	// Explicitly set the window text using Win32 ANSI API for 100% Turkish character compatibility
	SetWindowTextA(m_hWnd, c_szName);

	if (hIcon)
	{
		SendMessage(m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		SendMessage(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
	}

#ifdef DISABLE_TASKBAR_GROUPING
	OSVERSIONINFO v;
	v.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&v);
	if (v.dwMajorVersion == 6 && v.dwMinorVersion >= 1 || v.dwMajorVersion > 6)
	{
		WCHAR myAppID[128];
		swprintf(myAppID, sizeof(myAppID) / sizeof(myAppID[0]), L"MyMetin2AppID%u", GetCurrentProcessId());
		HRESULT hr = SetCurrentProcessExplicitAppUserModelID(myAppID);
		if (!SUCCEEDED(hr))
			return false;
	}
#endif

	SetWindowLong(m_hWnd, GWL_USERDATA, (DWORD) this);
	s_pOldSDLWndProc = (WNDPROC)SetWindowLong(m_hWnd, GWL_WNDPROC, (LONG)MSWindowProcedure);

	return true;
#endif
}

void CMSWindow::SetVisibleMode(bool isVisible)
{
	m_isVisible = isVisible;

	if (m_isVisible)
		Show();
	else
		Hide();
}

void CMSWindow::Show()
{
	m_isVisible = true;
	if (m_pSDLWindow)
		SDL_ShowWindow(m_pSDLWindow);
	else if (m_hWnd)
		ShowWindow(m_hWnd, SW_SHOW);
}

void CMSWindow::Hide()
{
	m_isVisible = false;
	if (m_pSDLWindow)
		SDL_HideWindow(m_pSDLWindow);
	else if (m_hWnd)
		ShowWindow(m_hWnd, SW_HIDE);
}

bool CMSWindow::IsVisible()
{
	return m_isVisible;
}

bool CMSWindow::IsActive()
{
	return m_isActive;
}

HINSTANCE CMSWindow::GetInstance()
{
	return ms_hInstance;
}

HWND CMSWindow::GetWindowHandle()
{
	return m_hWnd;
}

int	CMSWindow::GetScreenWidth()
{
	return GetSystemMetrics(SM_CXSCREEN);
}

int	CMSWindow::GetScreenHeight()
{
	return GetSystemMetrics(SM_CYSCREEN);
}

void CMSWindow::GetWindowRect(RECT* prc)
{
	::GetWindowRect(m_hWnd, prc);
}


void CMSWindow::GetClientRect(RECT* prc)
{
	::GetClientRect(m_hWnd, prc);
}

void CMSWindow::GetMousePosition(POINT* ppt)
{
	GetCursorPos(ppt);
	ScreenToClient(m_hWnd, ppt);
}

void CMSWindow::SetPosition(int x, int y)
{
	if (m_hWnd)
		SetWindowPos(m_hWnd, NULL, x, y, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
	else if (m_pSDLWindow)
		SDL_SetWindowPosition(m_pSDLWindow, x, y);
}

void CMSWindow::SetCenterPosition()
{
	RECT rc;

	GetClientRect(&rc);

	int windowWidth = rc.right - rc.left;
	int windowHeight = rc.bottom - rc.top;

	SetPosition((GetScreenWidth()-windowWidth)/2, (GetScreenHeight()-windowHeight)/2);
}

void CMSWindow::AdjustSize(int width, int height)
{
	SetRect(&m_rect, 0, 0, width, height);

	if (m_hWnd)
	{
		AdjustWindowRectEx(&m_rect,
							GetWindowStyle(m_hWnd),     
							GetMenu(m_hWnd ) != NULL,    
							GetWindowExStyle(m_hWnd ) ); 

		MoveWindow
		( 
			m_hWnd, 
			0, 
			0, 
			m_rect.right - m_rect.left, 
			m_rect.bottom - m_rect.top, 
			FALSE
		);
	}
	else if (m_pSDLWindow)
	{
		SDL_SetWindowSize(m_pSDLWindow, width, height);
	}
}

void CMSWindow::SetText(const char* c_szText)
{
	if (m_hWnd)
		SetWindowTextA(m_hWnd, c_szText);

	if (m_pSDLWindow)
	{
		wchar_t wszText[1024];
		MultiByteToWideChar(CP_ACP, 0, c_szText, -1, wszText, 1024);
		char szUtf8[2048];
		WideCharToMultiByte(CP_UTF8, 0, wszText, -1, szUtf8, 2048, NULL, NULL);
		SDL_SetWindowTitle(m_pSDLWindow, szUtf8);
	}
}

void CMSWindow::SetSize(int width, int height)
{	
	if (m_hWnd)
		SetWindowPos(m_hWnd, NULL, 0, 0, width, height, SWP_NOZORDER|SWP_NOMOVE);
	else if (m_pSDLWindow)
		SDL_SetWindowSize(m_pSDLWindow, width, height);
}


CMSWindow::CMSWindow()
{
	m_hWnd = NULL;
	m_pSDLWindow = NULL;
	m_isActive = false;
	m_isVisible = false;
}

CMSWindow::~CMSWindow()
{
}