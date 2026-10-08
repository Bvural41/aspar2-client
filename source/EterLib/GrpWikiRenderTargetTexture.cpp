#include "StdAfx.h"

#if defined(USE_OPENGL_ES)
static const float c_fHalfPixel = 0.0f;
#else
static const float c_fHalfPixel = 0.5f;
#endif
#ifdef ENABLE_INGAME_WIKI
#include "../EterBase/Stl.h"
#include "../EterBase/Utils.h"

#include "StateManager.h"
#include "GrpWikiRenderTargetTexture.h"

CGraphicWikiRenderTargetTexture::~CGraphicWikiRenderTargetTexture() { Reset(); }
CGraphicWikiRenderTargetTexture::CGraphicWikiRenderTargetTexture() :
	m_lpd3dRenderTexture(nullptr),
	m_lpd3dRenderTargetSurface(nullptr),
	m_lpd3dDepthSurface(nullptr),
	m_lpd3dOriginalRenderTarget(nullptr),
	m_lpd3dOldDepthBufferSurface(nullptr),
	m_d3dFormat(D3DFMT_UNKNOWN),
	m_depthStencilFormat(D3DFMT_UNKNOWN)
#if defined(USE_OPENGL_ES)
	  , m_fbo(0), m_depthRbo(0), m_prevFBO(0)
#endif
{
	Initialize();
	
	memset(&m_renderRect, 0, sizeof(m_renderRect));
	memset(&m_renderBox, 0, sizeof(m_renderBox));
#if defined(USE_OPENGL_ES)
	memset(m_prevViewport, 0, sizeof(m_prevViewport));
#endif
}

bool CGraphicWikiRenderTargetTexture::Create(const int width, const int height, const D3DFORMAT texFormat, const D3DFORMAT depthFormat)
{
#if defined(USE_OPENGL_ES)
	Reset();
	m_height = height;
	m_width = width;
	m_d3dFormat = texFormat;
	m_depthStencilFormat = depthFormat;

	GLuint glTex = 0;
	glGenTextures(1, &glTex);
	glBindTexture(GL_TEXTURE_2D, glTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	m_lpd3dRenderTexture = (LPDIRECT3DTEXTURE9)(uintptr_t)glTex;

	glGenRenderbuffers(1, &m_depthRbo);
	glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);

	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glTex, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRbo);

	GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		TraceError("CGraphicWikiRenderTargetTexture::Create FBO incomplete: 0x%x", fboStatus);
		ReleaseTextures();
		return false;
	}

	return true;
#else
	Reset();
	
	m_height = height;
	m_width = width;
	
	if (!CreateRenderTexture(width, height, texFormat))
		return false;
	
	if (!CreateRenderDepthStencil(width, height, depthFormat))
		return false;
	
	return true;
#endif
}

int CGraphicWikiRenderTargetTexture::GetWidth() {
	return m_width;
}

int CGraphicWikiRenderTargetTexture::GetHeight() {
	return m_height;
}

void CGraphicWikiRenderTargetTexture::CreateTextures()
{
#if defined(USE_OPENGL_ES)
	if (m_width > 0 && m_height > 0)
		Create(m_width, m_height, m_d3dFormat, m_depthStencilFormat);
#else
	if (CreateRenderTexture(m_width, m_height, m_d3dFormat))
		CreateRenderDepthStencil(m_width, m_height, m_depthStencilFormat);
#endif
}

bool CGraphicWikiRenderTargetTexture::CreateRenderTexture(const int width, const int height, const D3DFORMAT format)
{
#if defined(USE_OPENGL_ES)
	return true;
#else
	m_d3dFormat = format;
	
	if (FAILED(ms_lpd3dDevice->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, m_d3dFormat, D3DPOOL_DEFAULT, &m_lpd3dRenderTexture, nullptr)))
		return false;
	
	if (FAILED(m_lpd3dRenderTexture->GetSurfaceLevel(0, &m_lpd3dRenderTargetSurface)))
		return false;
	
	return true;
#endif
}

void CGraphicWikiRenderTargetTexture::ReleaseTextures()
{
#if defined(USE_OPENGL_ES)
	if (m_fbo)
	{
		glDeleteFramebuffers(1, &m_fbo);
		m_fbo = 0;
	}
	if (m_depthRbo)
	{
		glDeleteRenderbuffers(1, &m_depthRbo);
		m_depthRbo = 0;
	}
	if (m_lpd3dRenderTexture)
	{
		GLuint glTex = (GLuint)(uintptr_t)m_lpd3dRenderTexture;
		if (glTex)
			glDeleteTextures(1, &glTex);
		m_lpd3dRenderTexture = nullptr;
	}
#else
	SAFE_RELEASE(m_lpd3dRenderTexture);
	SAFE_RELEASE(m_lpd3dRenderTargetSurface);
	SAFE_RELEASE(m_lpd3dDepthSurface);
	SAFE_RELEASE(m_lpd3dOriginalRenderTarget);
	SAFE_RELEASE(m_lpd3dOldDepthBufferSurface);
#endif
}

LPDIRECT3DTEXTURE9 CGraphicWikiRenderTargetTexture::GetD3DRenderTargetTexture() const
{
	return m_lpd3dRenderTexture;
}

bool CGraphicWikiRenderTargetTexture::CreateRenderDepthStencil(const int width, const int height, const D3DFORMAT format)
{
#if defined(USE_OPENGL_ES)
	return true;
#else
	m_depthStencilFormat = format;
	
	return SUCCEEDED(ms_lpd3dDevice->CreateDepthStencilSurface(width, height, m_depthStencilFormat, D3DMULTISAMPLE_NONE, 0, TRUE, &m_lpd3dDepthSurface, nullptr));
#endif
}

void CGraphicWikiRenderTargetTexture::SetRenderTarget()
{
#if defined(USE_OPENGL_ES)
	if (!m_fbo) return;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_prevFBO);
	glGetIntegerv(GL_VIEWPORT, m_prevViewport);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glViewport(0, 0, m_width, m_height);
#else
	if (!ms_lpd3dDevice) return;
	ms_lpd3dDevice->GetRenderTarget(0, &m_lpd3dOriginalRenderTarget);
	ms_lpd3dDevice->GetDepthStencilSurface(&m_lpd3dOldDepthBufferSurface);
	ms_lpd3dDevice->SetRenderTarget(0, m_lpd3dRenderTargetSurface);
	ms_lpd3dDevice->SetDepthStencilSurface(m_lpd3dDepthSurface);
#endif
}

void CGraphicWikiRenderTargetTexture::ResetRenderTarget()
{
#if defined(USE_OPENGL_ES)
	glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)m_prevFBO);
	glViewport(m_prevViewport[0], m_prevViewport[1], m_prevViewport[2], m_prevViewport[3]);
#else
	if (!ms_lpd3dDevice) return;
	ms_lpd3dDevice->SetRenderTarget(0, m_lpd3dOriginalRenderTarget);
	ms_lpd3dDevice->SetDepthStencilSurface(m_lpd3dOldDepthBufferSurface);
	
	SAFE_RELEASE(m_lpd3dOriginalRenderTarget);
	SAFE_RELEASE(m_lpd3dOldDepthBufferSurface);
#endif
}

void CGraphicWikiRenderTargetTexture::SetRenderingRect(RECT* rect)
{
	m_renderRect = *rect;
}

RECT* CGraphicWikiRenderTargetTexture::GetRenderingRect()
{
	return &m_renderRect;
}

void CGraphicWikiRenderTargetTexture::SetRenderingBox(RECT* renderBox)
{
	m_renderBox = *renderBox;
}

RECT* CGraphicWikiRenderTargetTexture::GetRenderingBox()
{
	return &m_renderBox;
}

void CGraphicWikiRenderTargetTexture::Clear()
{
#if defined(USE_OPENGL_ES)
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
	if (!ms_lpd3dDevice) return;
	ms_lpd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
#endif
}

void CGraphicWikiRenderTargetTexture::Render() const
{
	const float sx = static_cast<float>(m_renderRect.left) - c_fHalfPixel + static_cast<float>(m_renderBox.left);
	const float sy = static_cast<float>(m_renderRect.top) - c_fHalfPixel + static_cast<float>(m_renderBox.top);
	const float ex = static_cast<float>(m_renderRect.left) + (static_cast<float>(m_renderRect.right) - static_cast<float>(m_renderRect.left)) - c_fHalfPixel - static_cast<float>(m_renderBox.right);
	const float ey = static_cast<float>(m_renderRect.top) + (static_cast<float>(m_renderRect.bottom) - static_cast<float>(m_renderRect.top)) - c_fHalfPixel - static_cast<float>(m_renderBox.bottom);
	
	const float texReverseWidth = 1.0f / (static_cast<float>(m_renderRect.right) - static_cast<float>(m_renderRect.left));
	const float texReverseHeight = 1.0f / (static_cast<float>(m_renderRect.bottom) - static_cast<float>(m_renderRect.top));
	
	const float su = m_renderBox.left * texReverseWidth;
	const float euCalc = ((m_renderRect.right - m_renderRect.left) - m_renderBox.right) * texReverseWidth;
	const float evCalc = ((m_renderRect.bottom - m_renderRect.top) - m_renderBox.bottom) * texReverseHeight;
#if defined(USE_OPENGL_ES)
	// OpenGL FBO renders upside-down, flip V coords
	const float sv = evCalc;
	const float eu = euCalc;
	const float ev = m_renderBox.top * texReverseHeight;
#else
	const float sv = m_renderBox.top * texReverseHeight;
	const float eu = euCalc;
	const float ev = evCalc;
#endif
	
	TPDTVertex pVertices[4];
	pVertices[0].position = TPosition(sx, sy, 0.0f);
	pVertices[0].texCoord = TTextureCoordinate(su, sv);
	pVertices[0].diffuse = 0xFFFFFFFF;
	
	pVertices[1].position = TPosition(ex, sy, 0.0f);
	pVertices[1].texCoord = TTextureCoordinate(eu, sv);
	pVertices[1].diffuse = 0xFFFFFFFF;
	
	pVertices[2].position = TPosition(sx, ey, 0.0f);
	pVertices[2].texCoord = TTextureCoordinate(su, ev);
	pVertices[2].diffuse = 0xFFFFFFFF;
	
	pVertices[3].position = TPosition(ex, ey, 0.0f);
	pVertices[3].texCoord = TTextureCoordinate(eu, ev);
	pVertices[3].diffuse = 0xFFFFFFFF;
	
	if (SetPDTStream(pVertices, 4))
	{
		CGraphicBase::SetDefaultIndexBuffer(CGraphicBase::DEFAULT_IB_FILL_RECT);
		
		STATEMANAGER.SetTexture(0, GetD3DRenderTargetTexture());
		STATEMANAGER.SetTexture(1, NULL);
		STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_DIFFUSE);
		STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2);
	}
}

void CGraphicWikiRenderTargetTexture::Reset()
{
	Destroy();
	ReleaseTextures();
	
	m_d3dFormat = D3DFMT_UNKNOWN;
	m_depthStencilFormat = D3DFMT_UNKNOWN;
}
#endif
