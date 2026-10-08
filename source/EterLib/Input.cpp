#include "StdAfx.h"
#include "Input.h"

HWND CInputKeyboard::ms_hWnd = NULL;
bool CInputKeyboard::ms_bPressedKey[256];

CInputKeyboard::CInputKeyboard()
{
	ResetKeyboard();
}

CInputKeyboard::~CInputKeyboard()
{
}

void CInputKeyboard::ResetKeyboard()
{
	for (int i = 0; i < 256; ++i)
	{
		if (ms_bPressedKey[i])
			KeyUp(i);
	}
	memset(ms_bPressedKey, 0, sizeof(ms_bPressedKey));
}

bool CInputKeyboard::InitializeKeyboard(HWND hWnd)
{
	ms_hWnd = hWnd;
	ResetKeyboard();
	return true;
}

void CInputKeyboard::UpdateKeyboard()
{
#ifdef _WIN32
	HWND hWndForeground = GetForegroundWindow();

	// When Metin2 is the foreground active window, synchronize key state with physical hardware.
	// Any key (Space, W, A, S, D, etc.) that is NOT physically held down will be released immediately!
	// When Metin2 is in the background, we do NOT release keys, preserving background auto-attack/walking.
	if (hWndForeground && hWndForeground == ms_hWnd)
	{
		for (int i = 0; i < 256; ++i)
		{
			if (ms_bPressedKey[i])
			{
				int vk = 0;
				if (i == DIK_SPACE) vk = VK_SPACE;
				else if (i == DIK_UP) vk = VK_UP;
				else if (i == DIK_DOWN) vk = VK_DOWN;
				else if (i == DIK_LEFT) vk = VK_LEFT;
				else if (i == DIK_RIGHT) vk = VK_RIGHT;
				else if (i == DIK_LMENU || i == DIK_RMENU) vk = VK_MENU;
				else if (i == DIK_LCONTROL || i == DIK_RCONTROL) vk = VK_CONTROL;
				else if (i == DIK_LSHIFT || i == DIK_RSHIFT) vk = VK_SHIFT;
				else if (i == DIK_LWIN) vk = VK_LWIN;
				else if (i == DIK_RWIN) vk = VK_RWIN;
				else vk = MapVirtualKeyA(i & 0x7F, 1);

				if (vk > 0 && (GetAsyncKeyState(vk) & 0x8000) == 0)
				{
					KeyUp(i);
				}
			}
		}
	}
#endif
}

void CInputKeyboard::KeyDown(int iIndex)
{
	if (iIndex < 0 || iIndex >= 256)
		return;

	ms_bPressedKey[iIndex] = true;
	OnKeyDown(iIndex);
}

void CInputKeyboard::KeyUp(int iIndex)
{
	if (iIndex < 0 || iIndex >= 256)
		return;

	ms_bPressedKey[iIndex] = false;
	OnKeyUp(iIndex);
}

bool CInputKeyboard::IsPressed(int iIndex)
{
	if (iIndex < 0 || iIndex >= 256)
		return false;

	return ms_bPressedKey[iIndex];
}
