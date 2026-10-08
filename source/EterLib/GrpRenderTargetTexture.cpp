#include "StdAfx.h"

#if defined(USE_OPENGL_ES)
static const float c_fHalfPixel = 0.0f;
#else
static const float c_fHalfPixel = 0.5f;
#endif
#include "../EterBase/Stl.h"
#include "GrpRenderTargetTexture.h"
#include "StateManager.h"

CGraphicRenderTargetTexture::CGraphicRenderTargetTexture()
	: m_lpd3dRenderTexture(nullptr),
	  m_lpd3dRenderTargetSurface(nullptr),
	  m_lpd3dDepthSurface(nullptr),
	  m_lpd3dOriginalRenderTarget(nullptr),
	  m_lpd3dOldDepthBufferSurface(nullptr),
	  m_d3dFormat{D3DFMT_UNKNOWN}, m_depthStencilFormat{D3DFMT_UNKNOWN}
#if defined(USE_OPENGL_ES)
	  , m_fbo(0), m_depthRbo(0), m_prevFBO(0)
#endif
{
	Initialize();
	m_renderRect.left = 0;
	m_renderRect.top = 0;
	m_renderRect.right = 0;
	m_renderRect.bottom = 0;
#if defined(USE_OPENGL_ES)
	memset(m_prevViewport, 0, sizeof(m_prevViewport));
#endif
}

CGraphicRenderTargetTexture::~CGraphicRenderTargetTexture()
{
	Reset();
}

void CGraphicRenderTargetTexture::ReleaseTextures()
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
	safe_release(m_lpd3dRenderTexture);
	safe_release(m_lpd3dRenderTargetSurface);
	safe_release(m_lpd3dDepthSurface);
	safe_release(m_lpd3dOriginalRenderTarget);
	safe_release(m_lpd3dOldDepthBufferSurface);
#endif
}

bool CGraphicRenderTargetTexture::Create(const int width, const int height, const D3DFORMAT texFormat, const D3DFORMAT depthFormat)
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
		TraceError("CGraphicRenderTargetTexture::Create FBO incomplete: 0x%x", fboStatus);
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

void CGraphicRenderTargetTexture::CreateTextures()
{
#if defined(USE_OPENGL_ES)
	if (m_width > 0 && m_height > 0)
		Create(m_width, m_height, m_d3dFormat, m_depthStencilFormat);
#else
	if (CreateRenderTexture(m_width, m_height, m_d3dFormat))
	{
		CreateRenderDepthStencil(m_width, m_height, m_depthStencilFormat);
	}
#endif
}

bool CGraphicRenderTargetTexture::CreateRenderTexture(const int width, const int height, const D3DFORMAT format)
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

bool CGraphicRenderTargetTexture::CreateRenderDepthStencil(const int width, const int height, const D3DFORMAT format)
{
#if defined(USE_OPENGL_ES)
	return true;
#else
	m_depthStencilFormat = format;

	return SUCCEEDED(ms_lpd3dDevice->CreateDepthStencilSurface(width, height, m_depthStencilFormat, D3DMULTISAMPLE_NONE, 0, TRUE, &m_lpd3dDepthSurface, nullptr));
#endif
}

void CGraphicRenderTargetTexture::SetRenderTarget()
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

void CGraphicRenderTargetTexture::ResetRenderTarget()
{
#if defined(USE_OPENGL_ES)
	glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)m_prevFBO);
	glViewport(m_prevViewport[0], m_prevViewport[1], m_prevViewport[2], m_prevViewport[3]);
#else
	if (!ms_lpd3dDevice) return;
	ms_lpd3dDevice->SetRenderTarget(0, m_lpd3dOriginalRenderTarget);
	ms_lpd3dDevice->SetDepthStencilSurface(m_lpd3dOldDepthBufferSurface);

	safe_release(m_lpd3dOriginalRenderTarget);
	safe_release(m_lpd3dOldDepthBufferSurface);
#endif
}

void CGraphicRenderTargetTexture::Clear()
{
#if defined(USE_OPENGL_ES)
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
	if (!ms_lpd3dDevice) return;
	ms_lpd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
#endif
}

void CGraphicRenderTargetTexture::SetRenderingRect(RECT* rect)
{
	m_renderRect = *rect;
}

RECT* CGraphicRenderTargetTexture::GetRenderingRect()
{
	return &m_renderRect;
}

LPDIRECT3DTEXTURE9 CGraphicRenderTargetTexture::GetRenderTargetTexture() const
{
	return m_lpd3dRenderTexture;
}

void CGraphicRenderTargetTexture::Render() const
{
	const auto imgWidth = static_cast<float>(m_renderRect.right - m_renderRect.left);
	const auto imgHeight = static_cast<float>(m_renderRect.bottom - m_renderRect.top);

#if defined(USE_OPENGL_ES)
	const auto eu = (m_width > 0) ? (imgWidth / static_cast<float>(m_width)) : 1.0f;
	const auto evRaw = (m_height > 0) ? (imgHeight / static_cast<float>(m_height)) : 1.0f;
#else
	D3DSURFACE_DESC desc;
	if (m_lpd3dRenderTargetSurface)
		m_lpd3dRenderTargetSurface->GetDesc(&desc);
	else
		return;

	const auto eu = imgWidth / static_cast<float>(desc.Width);
	const auto ev = imgHeight / static_cast<float>(desc.Height);
#endif

	const auto su = 0.0f;
#if defined(USE_OPENGL_ES)
	// OpenGL FBO renders upside-down, flip V coords
	const auto sv = evRaw;
	const auto ev = 0.0f;
#else
	const auto sv = 0.0f;
#endif

	TPDTVertex pVertices[4];

	pVertices[0].position = TPosition(m_renderRect.left - c_fHalfPixel, m_renderRect.top - c_fHalfPixel, 0.0f);
	pVertices[0].texCoord = TTextureCoordinate(su, sv);
	pVertices[0].diffuse = 0xffffffff;

	pVertices[1].position = TPosition(m_renderRect.right - c_fHalfPixel, m_renderRect.top - c_fHalfPixel, 0.0f);
	pVertices[1].texCoord = TTextureCoordinate(eu, sv);
	pVertices[1].diffuse = 0xffffffff;

	pVertices[2].position = TPosition(m_renderRect.left - c_fHalfPixel, m_renderRect.bottom - c_fHalfPixel, 0.0f);
	pVertices[2].texCoord = TTextureCoordinate(su, ev);
	pVertices[2].diffuse = 0xffffffff;

	pVertices[3].position = TPosition(m_renderRect.right - c_fHalfPixel, m_renderRect.bottom - c_fHalfPixel, 0.0f);
	pVertices[3].texCoord = TTextureCoordinate(eu, ev);
	pVertices[3].diffuse = 0xffffffff;

	if (SetPDTStream(pVertices, 4))
	{
		SetDefaultIndexBuffer(DEFAULT_IB_FILL_RECT);

		STATEMANAGER.SetTexture(0, GetRenderTargetTexture());
		STATEMANAGER.SetTexture(1, nullptr);
		STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_DIFFUSE);
		STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2);
	}
}

void CGraphicRenderTargetTexture::Reset()
{
	Destroy();
	ReleaseTextures();

	m_d3dFormat = D3DFMT_UNKNOWN;
	m_depthStencilFormat = D3DFMT_UNKNOWN;
}
