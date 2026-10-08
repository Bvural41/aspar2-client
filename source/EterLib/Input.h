#pragma once

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)		{ if (p) { (p)->Release(); (p)=NULL; } }
#endif

class CInputDevice
{
	public:
		CInputDevice() {}
		virtual ~CInputDevice() {}

		bool CreateDevice(HWND hWnd = NULL) { return true; }
};

class CInputKeyboard : public CInputDevice
{
	public:
		CInputKeyboard();
		virtual ~CInputKeyboard();

		bool			InitializeKeyboard(HWND hWnd = NULL);
		void			UpdateKeyboard();
		void			ResetKeyboard();

		bool			IsPressed(int iIndex);
		void			KeyDown(int iIndex);
		void			KeyUp(int iIndex);

	protected:
		virtual void	OnKeyDown(int iIndex) = 0;
		virtual void	OnKeyUp(int iIndex) = 0;

	protected:
		static bool		ms_bPressedKey[256];
		static HWND		ms_hWnd;
};