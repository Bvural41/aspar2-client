#include "StdAfx.h"
#include "../eterBase/Stl.h"
#include "GrpTexture.h"
#include "StateManager.h"

void CGraphicTexture::DestroyDeviceObjects()
{
#if defined(USE_OPENGL_ES)
	if (m_uGLTexture)
	{
		glDeleteTextures(1, &m_uGLTexture);
		m_uGLTexture = 0;
	}
#else
	safe_release(m_lpd3dTexture);
#endif
}

void CGraphicTexture::Destroy()
{
	DestroyDeviceObjects();

	Initialize();
}

void CGraphicTexture::Initialize()
{
	m_lpd3dTexture = nullptr;
#if defined(USE_OPENGL_ES)
	m_uGLTexture = 0;
#endif
	m_width = 0;
	m_height = 0;
	m_bEmpty = true;
}

bool CGraphicTexture::IsEmpty() const
{
	return m_bEmpty;
}

void CGraphicTexture::SetTextureStage(int stage) const
{
#if defined(USE_OPENGL_ES)
	if (m_uGLTexture)
	{
		// Update STATEMANAGER so SetupUIShaderAndStates sees the correct texture
		STATEMANAGER.SetTexture(stage, (LPDIRECT3DBASETEXTURE9)(uintptr_t)m_uGLTexture);
	}
#else
	assert(ms_lpd3dDevice != nullptr);
	STATEMANAGER.SetTexture(stage, m_lpd3dTexture);
#endif
}

LPDIRECT3DTEXTURE9 CGraphicTexture::GetD3DTexture() const
{
#if defined(USE_OPENGL_ES)
	return (LPDIRECT3DTEXTURE9)(uintptr_t)m_uGLTexture;
#else
	return m_lpd3dTexture;
#endif
}

int CGraphicTexture::GetWidth() const
{
	return m_width;
}

int CGraphicTexture::GetHeight() const
{
	return m_height;
}

CGraphicTexture::CGraphicTexture()
{
	Initialize();
}

CGraphicTexture::~CGraphicTexture()
{
}
