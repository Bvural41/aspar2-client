#include "StdAfx.h"
#include "AndroidTouchControls.h"
#include "PythonApplication.h"
#include "PythonPlayer.h"
#include "../eterlib/Camera.h"
#include "../eterlib/StateManager.h"
#include "../eterpythonlib/PythonGraphic.h"
#include <SDL2/SDL.h>
#include <cmath>
#include <vector>

CAndroidTouchControls::CAndroidTouchControls()
	: m_screenWidth(0)
	, m_screenHeight(0)
	, m_joyFingerId(-1)
	, m_bJoyActive(false)
	, m_joyDefaultBaseX(140.0f)
	, m_joyDefaultBaseY(580.0f)
	, m_joyBaseX(140.0f)
	, m_joyBaseY(580.0f)
	, m_joyThumbX(140.0f)
	, m_joyThumbY(580.0f)
	, m_joyBaseRadius(90.0f)
	, m_joyThumbRadius(40.0f)
	, m_joyDeadZone(14.0f)
	, m_attackFingerId(-1)
	, m_bAttackPressed(false)
	, m_attackX(1180.0f)
	, m_attackY(600.0f)
	, m_attackRadius(60.0f)
	, m_zoomFingerId(-1)
	, m_bZoomActive(false)
	, m_zoomBarX(25.0f)
	, m_zoomBarY(180.0f)
	, m_zoomBarW(32.0f)
	, m_zoomBarH(160.0f)
	, m_zoomThumbY(260.0f)
	, m_zoomLastTouchY(0.0f)
	, m_cameraFingerId(-1)
	, m_bCameraDragging(false)
	, m_camStartX(0.0f)
	, m_camStartY(0.0f)
	, m_camLastX(0.0f)
	, m_camLastY(0.0f)
	, m_camStartTime(0)
{
}

CAndroidTouchControls::~CAndroidTouchControls()
{
}

bool CAndroidTouchControls::IsInGame() const
{
	return false;
}

void CAndroidTouchControls::UpdateScreenSize(int width, int height)
{
	if (width <= 0 || height <= 0)
		return;

	m_screenWidth = width;
	m_screenHeight = height;

	float minDim = (float)min(width, height);
	float scale = minDim / 720.0f;
	if (scale < 0.65f) scale = 0.65f;

	// Joystick: bottom-left
	m_joyBaseRadius = 88.0f * scale;
	m_joyThumbRadius = 38.0f * scale;
	m_joyDeadZone = 12.0f * scale;
	m_joyDefaultBaseX = 135.0f * scale;
	m_joyDefaultBaseY = (float)height - 135.0f * scale;
	m_joyBaseX = m_joyDefaultBaseX;
	m_joyBaseY = m_joyDefaultBaseY;
	m_joyThumbX = m_joyBaseX;
	m_joyThumbY = m_joyBaseY;

	// Attack button: bottom-right
	m_attackRadius = 56.0f * scale;
	m_attackX = (float)width - 96.0f * scale;
	m_attackY = (float)height - 96.0f * scale;

	// Zoom slider: far left, above joystick
	m_zoomBarW = 30.0f * scale;
	m_zoomBarH = 150.0f * scale;
	m_zoomBarX = 24.0f * scale;
	m_zoomBarY = (float)height * 0.22f;
	m_zoomThumbY = m_zoomBarY + m_zoomBarH * 0.5f;
}

void CAndroidTouchControls::Reset()
{
	if (m_bJoyActive)
	{
		CPythonPlayer::Instance().NEW_Stop();
	}
	if (m_bAttackPressed)
	{
		CPythonPlayer::Instance().SetAttackKeyState(false);
	}
	m_joyFingerId = -1;
	m_bJoyActive = false;
	m_attackFingerId = -1;
	m_bAttackPressed = false;
	m_zoomFingerId = -1;
	m_bZoomActive = false;
	m_cameraFingerId = -1;
	m_bCameraDragging = false;
	m_joyThumbX = m_joyBaseX = m_joyDefaultBaseX;
	m_joyThumbY = m_joyBaseY = m_joyDefaultBaseY;
	m_zoomThumbY = m_zoomBarY + m_zoomBarH * 0.5f;
}

bool CAndroidTouchControls::OnFingerDown(SDL_FingerID fingerId, float x, float y)
{
	if (!IsInGame())
		return false;

	// 1. Attack Button (Bottom-Right)
	float atkDx = x - m_attackX;
	float atkDy = y - m_attackY;
	if ((atkDx * atkDx + atkDy * atkDy) <= (m_attackRadius * 1.35f) * (m_attackRadius * 1.35f))
	{
		m_attackFingerId = fingerId;
		m_bAttackPressed = true;
		CPythonPlayer::Instance().SetAttackKeyState(true);
		return true;
	}

	// 2. Zoom Slider (Far Left, above joystick)
	if (x >= (m_zoomBarX - 25.0f) && x <= (m_zoomBarX + m_zoomBarW + 25.0f) &&
		y >= (m_zoomBarY - 20.0f) && y <= (m_zoomBarY + m_zoomBarH + 20.0f))
	{
		m_zoomFingerId = fingerId;
		m_bZoomActive = true;
		m_zoomLastTouchY = y;
		m_zoomThumbY = y;
		if (m_zoomThumbY < m_zoomBarY + 15.0f) m_zoomThumbY = m_zoomBarY + 15.0f;
		if (m_zoomThumbY > m_zoomBarY + m_zoomBarH - 15.0f) m_zoomThumbY = m_zoomBarY + m_zoomBarH - 15.0f;

		CCamera* pCamera = CCameraManager::Instance().GetCurrentCamera();
		if (pCamera)
		{
			if (y < m_zoomBarY + 35.0f)
				pCamera->Zoom(0.85f);
			else if (y > m_zoomBarY + m_zoomBarH - 35.0f)
				pCamera->Zoom(1.15f);
		}
		return true;
	}

	// 3. Virtual Joystick (Bottom-Left zone)
	if (x < (float)m_screenWidth * 0.38f && y > (float)m_screenHeight * 0.42f)
	{
		m_joyFingerId = fingerId;
		m_bJoyActive = true;

		float minMarginX = m_joyBaseRadius + 15.0f;
		float maxMarginX = (float)m_screenWidth * 0.38f - m_joyBaseRadius;
		float minMarginY = (float)m_screenHeight * 0.42f + m_joyBaseRadius;
		float maxMarginY = (float)m_screenHeight - m_joyBaseRadius - 15.0f;

		m_joyBaseX = fmaxf(minMarginX, fminf(x, maxMarginX));
		m_joyBaseY = fmaxf(minMarginY, fminf(y, maxMarginY));
		m_joyThumbX = x;
		m_joyThumbY = y;

		float dx = x - m_joyBaseX;
		float dy = y - m_joyBaseY;
		float dist = sqrtf(dx * dx + dy * dy);
		if (dist > m_joyDeadZone)
		{
			float screen_dy = -dy; // Up is positive
			float angle = atan2f(dx, screen_dy) * (180.0f / D3DX_PI);
			if (angle < 0.0f) angle += 360.0f;
			CPythonPlayer::Instance().NEW_MoveToDirection(angle);
		}
		return true;
	}

	// 4. Middle / Camera & World Tap zone
	if (m_cameraFingerId == -1)
	{
		m_cameraFingerId = fingerId;
		m_camStartX = m_camLastX = x;
		m_camStartY = m_camLastY = y;
		m_camStartTime = ELTimer_GetMSec();
		m_bCameraDragging = false;
		return true;
	}

	return false;
}

bool CAndroidTouchControls::OnFingerMotion(SDL_FingerID fingerId, float x, float y, float dx, float dy)
{
	if (!IsInGame())
		return false;

	// Joystick
	if (fingerId == m_joyFingerId)
	{
		float jdx = x - m_joyBaseX;
		float jdy = y - m_joyBaseY;
		float dist = sqrtf(jdx * jdx + jdy * jdy);

		if (dist > m_joyBaseRadius)
		{
			float ratio = m_joyBaseRadius / dist;
			m_joyThumbX = m_joyBaseX + jdx * ratio;
			m_joyThumbY = m_joyBaseY + jdy * ratio;
		}
		else
		{
			m_joyThumbX = x;
			m_joyThumbY = y;
		}

		if (dist > m_joyDeadZone)
		{
			float screen_dy = -jdy;
			float angle = atan2f(jdx, screen_dy) * (180.0f / D3DX_PI);
			if (angle < 0.0f) angle += 360.0f;
			CPythonPlayer::Instance().NEW_MoveToDirection(angle);
		}
		else
		{
			CPythonPlayer::Instance().NEW_Stop();
		}
		return true;
	}

	// Attack Button
	if (fingerId == m_attackFingerId)
	{
		float atkDx = x - m_attackX;
		float atkDy = y - m_attackY;
		if ((atkDx * atkDx + atkDy * atkDy) > (m_attackRadius * 2.2f) * (m_attackRadius * 2.2f))
		{
			m_attackFingerId = -1;
			m_bAttackPressed = false;
			CPythonPlayer::Instance().SetAttackKeyState(false);
		}
		return true;
	}

	// Zoom Slider
	if (fingerId == m_zoomFingerId)
	{
		float deltaY = y - m_zoomLastTouchY;
		m_zoomLastTouchY = y;

		m_zoomThumbY = y;
		if (m_zoomThumbY < m_zoomBarY + 15.0f) m_zoomThumbY = m_zoomBarY + 15.0f;
		if (m_zoomThumbY > m_zoomBarY + m_zoomBarH - 15.0f) m_zoomThumbY = m_zoomBarY + m_zoomBarH - 15.0f;

		CCamera* pCamera = CCameraManager::Instance().GetCurrentCamera();
		if (pCamera)
		{
			if (deltaY < -1.5f)
				pCamera->Zoom(0.96f);
			else if (deltaY > 1.5f)
				pCamera->Zoom(1.04f);
		}
		return true;
	}

	// Camera Swipe
	if (fingerId == m_cameraFingerId)
	{
		float moveDx = x - m_camStartX;
		float moveDy = y - m_camStartY;
		float moveDist = sqrtf(moveDx * moveDx + moveDy * moveDy);

		if (moveDist > 10.0f)
		{
			m_bCameraDragging = true;
			float stepDx = x - m_camLastX;
			float stepDy = y - m_camLastY;
			m_camLastX = x;
			m_camLastY = y;

			CCamera* pCamera = CCameraManager::Instance().GetCurrentCamera();
			if (pCamera)
			{
				float fRollDegree = stepDx * 0.32f;
				float fPitchDegree = -stepDy * 0.28f;
				pCamera->RotateEyeAroundTarget(fPitchDegree, fRollDegree);
			}
		}
		return true;
	}

	return false;
}

bool CAndroidTouchControls::OnFingerUp(SDL_FingerID fingerId, float x, float y)
{
	if (!IsInGame())
		return false;

	// Joystick
	if (fingerId == m_joyFingerId)
	{
		m_joyFingerId = -1;
		m_bJoyActive = false;
		m_joyBaseX = m_joyDefaultBaseX;
		m_joyBaseY = m_joyDefaultBaseY;
		m_joyThumbX = m_joyDefaultBaseX;
		m_joyThumbY = m_joyDefaultBaseY;
		CPythonPlayer::Instance().NEW_Stop();
		return true;
	}

	// Attack Button
	if (fingerId == m_attackFingerId)
	{
		m_attackFingerId = -1;
		m_bAttackPressed = false;
		CPythonPlayer::Instance().SetAttackKeyState(false);
		return true;
	}

	// Zoom Slider
	if (fingerId == m_zoomFingerId)
	{
		m_zoomFingerId = -1;
		m_bZoomActive = false;
		m_zoomThumbY = m_zoomBarY + m_zoomBarH * 0.5f;
		return true;
	}

	// Camera Swipe vs Tap
	if (fingerId == m_cameraFingerId)
	{
		m_cameraFingerId = -1;
		if (!m_bCameraDragging)
		{
			CPythonApplication::Instance().OnMouseLeftButtonDown((int)x, (int)y);
			CPythonApplication::Instance().OnMouseLeftButtonUp((int)x, (int)y);
		}
		m_bCameraDragging = false;
		return true;
	}

	return false;
}

void CAndroidTouchControls::DrawFilledCircle(float cx, float cy, float radius, DWORD color, int steps)
{
	if (steps < 3) steps = 3;
	std::vector<TPDTVertex> vertices;
	vertices.resize(steps + 2);

	vertices[0].position = D3DXVECTOR3(cx, cy, 0.0f);
	vertices[0].diffuse = color;
	vertices[0].texCoord = D3DXVECTOR2(0.0f, 0.0f);

	float angleStep = 2.0f * D3DX_PI / float(steps);
	for (int i = 0; i <= steps; ++i)
	{
		float a = i * angleStep;
		vertices[i + 1].position = D3DXVECTOR3(cx + radius * cosf(a), cy + radius * sinf(a), 0.0f);
		vertices[i + 1].diffuse = color;
		vertices[i + 1].texCoord = D3DXVECTOR2(0.0f, 0.0f);
	}

	if (CGraphicBase::SetPDTStream(&vertices[0], vertices.size()))
	{
		STATEMANAGER.SetTexture(0, NULL);
		STATEMANAGER.SetTexture(1, NULL);
		STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLEFAN, 0, steps);
	}
}

void CAndroidTouchControls::DrawCircleRing(float cx, float cy, float radius, float thickness, DWORD color, int steps)
{
	if (steps < 3) steps = 3;
	std::vector<TPDTVertex> vertices;
	vertices.resize((steps + 1) * 2);

	float angleStep = 2.0f * D3DX_PI / float(steps);
	float rOuter = radius + thickness * 0.5f;
	float rInner = radius - thickness * 0.5f;

	for (int i = 0; i <= steps; ++i)
	{
		float a = i * angleStep;
		float cosA = cosf(a);
		float sinA = sinf(a);

		vertices[i * 2].position = D3DXVECTOR3(cx + rOuter * cosA, cy + rOuter * sinA, 0.0f);
		vertices[i * 2].diffuse = color;
		vertices[i * 2].texCoord = D3DXVECTOR2(0.0f, 0.0f);

		vertices[i * 2 + 1].position = D3DXVECTOR3(cx + rInner * cosA, cy + rInner * sinA, 0.0f);
		vertices[i * 2 + 1].diffuse = color;
		vertices[i * 2 + 1].texCoord = D3DXVECTOR2(0.0f, 0.0f);
	}

	if (CGraphicBase::SetPDTStream(&vertices[0], vertices.size()))
	{
		STATEMANAGER.SetTexture(0, NULL);
		STATEMANAGER.SetTexture(1, NULL);
		STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, steps * 2);
	}
}

void CAndroidTouchControls::DrawFilledRect(float sx, float sy, float ex, float ey, DWORD color)
{
	TPDTVertex vertices[4];
	vertices[0].position = D3DXVECTOR3(sx, sy, 0.0f); vertices[0].diffuse = color; vertices[0].texCoord = D3DXVECTOR2(0.0f, 0.0f);
	vertices[1].position = D3DXVECTOR3(sx, ey, 0.0f); vertices[1].diffuse = color; vertices[1].texCoord = D3DXVECTOR2(0.0f, 0.0f);
	vertices[2].position = D3DXVECTOR3(ex, sy, 0.0f); vertices[2].diffuse = color; vertices[2].texCoord = D3DXVECTOR2(0.0f, 0.0f);
	vertices[3].position = D3DXVECTOR3(ex, ey, 0.0f); vertices[3].diffuse = color; vertices[3].texCoord = D3DXVECTOR2(0.0f, 0.0f);

	if (CGraphicBase::SetPDTStream(vertices, 4))
	{
		STATEMANAGER.SetTexture(0, NULL);
		STATEMANAGER.SetTexture(1, NULL);
		STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	}
}

void CAndroidTouchControls::DrawRectOutline(float sx, float sy, float ex, float ey, float thickness, DWORD color)
{
	DrawFilledRect(sx, sy, ex, sy + thickness, color);
	DrawFilledRect(sx, ey - thickness, ex, ey, color);
	DrawFilledRect(sx, sy, sx + thickness, ey, color);
	DrawFilledRect(ex - thickness, sy, ex, ey, color);
}

void CAndroidTouchControls::DrawThickLine(float sx, float sy, float ex, float ey, float thickness, DWORD color)
{
	float dx = ex - sx;
	float dy = ey - sy;
	float len = sqrtf(dx * dx + dy * dy);
	if (len < 0.001f) return;

	float nx = -dy / len * (thickness * 0.5f);
	float ny = dx / len * (thickness * 0.5f);

	TPDTVertex vertices[4];
	vertices[0].position = D3DXVECTOR3(sx + nx, sy + ny, 0.0f); vertices[0].diffuse = color; vertices[0].texCoord = D3DXVECTOR2(0.0f, 0.0f);
	vertices[1].position = D3DXVECTOR3(sx - nx, sy - ny, 0.0f); vertices[1].diffuse = color; vertices[1].texCoord = D3DXVECTOR2(0.0f, 0.0f);
	vertices[2].position = D3DXVECTOR3(ex + nx, ey + ny, 0.0f); vertices[2].diffuse = color; vertices[2].texCoord = D3DXVECTOR2(0.0f, 0.0f);
	vertices[3].position = D3DXVECTOR3(ex - nx, ey - ny, 0.0f); vertices[3].diffuse = color; vertices[3].texCoord = D3DXVECTOR2(0.0f, 0.0f);

	if (CGraphicBase::SetPDTStream(vertices, 4))
	{
		STATEMANAGER.SetTexture(0, NULL);
		STATEMANAGER.SetTexture(1, NULL);
		STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	}
}

void CAndroidTouchControls::Render()
{
	if (!IsInGame())
		return;

	CPythonGraphic::Instance().SetInterfaceRenderState();

	float minDim = (float)min(m_screenWidth, m_screenHeight);
	float scale = minDim / 720.0f;
	if (scale < 0.65f) scale = 0.65f;

	// 1. --- Render Virtual Joystick ---
	// Base circle background
	DrawFilledCircle(m_joyBaseX, m_joyBaseY, m_joyBaseRadius, D3DCOLOR_ARGB(70, 15, 20, 30));
	// Base outer ring
	DrawCircleRing(m_joyBaseX, m_joyBaseY, m_joyBaseRadius, 2.5f * scale, D3DCOLOR_ARGB(140, 180, 210, 255));
	// Center anchor dot
	DrawFilledCircle(m_joyBaseX, m_joyBaseY, 5.0f * scale, D3DCOLOR_ARGB(120, 180, 210, 255));

	// Thumbstick knob
	DWORD thumbBg = m_bJoyActive ? D3DCOLOR_ARGB(180, 60, 130, 220) : D3DCOLOR_ARGB(120, 50, 75, 115);
	DWORD thumbRing = m_bJoyActive ? D3DCOLOR_ARGB(250, 255, 255, 255) : D3DCOLOR_ARGB(180, 200, 225, 255);
	DrawFilledCircle(m_joyThumbX, m_joyThumbY, m_joyThumbRadius, thumbBg);
	DrawCircleRing(m_joyThumbX, m_joyThumbY, m_joyThumbRadius, 2.5f * scale, thumbRing);

	// 2. --- Render Attack Button ---
	DWORD atkBg = m_bAttackPressed ? D3DCOLOR_ARGB(200, 220, 50, 50) : D3DCOLOR_ARGB(90, 120, 25, 25);
	DWORD atkRing = m_bAttackPressed ? D3DCOLOR_ARGB(255, 255, 220, 80) : D3DCOLOR_ARGB(170, 230, 80, 80);
	DrawFilledCircle(m_attackX, m_attackY, m_attackRadius, atkBg);
	DrawCircleRing(m_attackX, m_attackY, m_attackRadius, 3.5f * scale, atkRing);

	// Sword / Attack cross symbol in center
	float iconSize = 16.0f * scale;
	DWORD iconColor = D3DCOLOR_ARGB(240, 255, 255, 255);
	DrawThickLine(m_attackX - iconSize, m_attackY - iconSize, m_attackX + iconSize, m_attackY + iconSize, 3.0f * scale, iconColor);
	DrawThickLine(m_attackX + iconSize, m_attackY - iconSize, m_attackX - iconSize, m_attackY + iconSize, 3.0f * scale, iconColor);

	// 3. --- Render Zoom Slider ---
	DrawFilledRect(m_zoomBarX, m_zoomBarY, m_zoomBarX + m_zoomBarW, m_zoomBarY + m_zoomBarH, D3DCOLOR_ARGB(65, 15, 20, 30));
	DrawRectOutline(m_zoomBarX, m_zoomBarY, m_zoomBarX + m_zoomBarW, m_zoomBarY + m_zoomBarH, 1.5f * scale, D3DCOLOR_ARGB(120, 160, 190, 230));

	// Top '+' symbol
	float topCenterY = m_zoomBarY + 16.0f * scale;
	float midX = m_zoomBarX + m_zoomBarW * 0.5f;
	DrawThickLine(midX - 7.0f * scale, topCenterY, midX + 7.0f * scale, topCenterY, 2.0f * scale, D3DCOLOR_ARGB(220, 220, 240, 255));
	DrawThickLine(midX, topCenterY - 7.0f * scale, midX, topCenterY + 7.0f * scale, 2.0f * scale, D3DCOLOR_ARGB(220, 220, 240, 255));

	// Bottom '-' symbol
	float botCenterY = m_zoomBarY + m_zoomBarH - 16.0f * scale;
	DrawThickLine(midX - 7.0f * scale, botCenterY, midX + 7.0f * scale, botCenterY, 2.0f * scale, D3DCOLOR_ARGB(220, 220, 240, 255));

	// Slider handle thumb
	DWORD zoomThumbColor = m_bZoomActive ? D3DCOLOR_ARGB(240, 100, 170, 255) : D3DCOLOR_ARGB(160, 180, 210, 255);
	DrawFilledCircle(midX, m_zoomThumbY, 10.0f * scale, zoomThumbColor);
	DrawCircleRing(midX, m_zoomThumbY, 10.0f * scale, 1.5f * scale, D3DCOLOR_ARGB(220, 255, 255, 255));
}
