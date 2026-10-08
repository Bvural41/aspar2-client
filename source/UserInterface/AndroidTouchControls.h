#pragma once

#include "../eterlib/StdAfx.h"
#include <SDL2/SDL_touch.h>

class CAndroidTouchControls
{
public:
	CAndroidTouchControls();
	~CAndroidTouchControls();

	void UpdateScreenSize(int width, int height);

	bool OnFingerDown(SDL_FingerID fingerId, float x, float y);
	bool OnFingerUp(SDL_FingerID fingerId, float x, float y);
	bool OnFingerMotion(SDL_FingerID fingerId, float x, float y, float dx, float dy);

	void Render();
	void Reset();

	bool IsInGame() const;

private:
	void DrawFilledCircle(float cx, float cy, float radius, DWORD color, int steps = 28);
	void DrawCircleRing(float cx, float cy, float radius, float thickness, DWORD color, int steps = 28);
	void DrawFilledRect(float sx, float sy, float ex, float ey, DWORD color);
	void DrawRectOutline(float sx, float sy, float ex, float ey, float thickness, DWORD color);
	void DrawThickLine(float sx, float sy, float ex, float ey, float thickness, DWORD color);

private:
	int m_screenWidth;
	int m_screenHeight;

	// --- Joystick ---
	SDL_FingerID m_joyFingerId;
	bool m_bJoyActive;
	float m_joyDefaultBaseX;
	float m_joyDefaultBaseY;
	float m_joyBaseX;
	float m_joyBaseY;
	float m_joyThumbX;
	float m_joyThumbY;
	float m_joyBaseRadius;
	float m_joyThumbRadius;
	float m_joyDeadZone;

	// --- Attack Button ---
	SDL_FingerID m_attackFingerId;
	bool m_bAttackPressed;
	float m_attackX;
	float m_attackY;
	float m_attackRadius;

	// --- Zoom Slider ---
	SDL_FingerID m_zoomFingerId;
	bool m_bZoomActive;
	float m_zoomBarX;
	float m_zoomBarY;
	float m_zoomBarW;
	float m_zoomBarH;
	float m_zoomThumbY;
	float m_zoomLastTouchY;

	// --- Camera Rotation & Tap ---
	SDL_FingerID m_cameraFingerId;
	bool m_bCameraDragging;
	float m_camStartX;
	float m_camStartY;
	float m_camLastX;
	float m_camLastY;
	uint32_t m_camStartTime;
};
