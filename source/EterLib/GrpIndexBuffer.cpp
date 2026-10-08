#include "StdAfx.h"
#include "../eterBase/Stl.h"
#include "GrpIndexBuffer.h"
#include "StateManager.h"

LPDIRECT3DINDEXBUFFER9 CGraphicIndexBuffer::GetD3DIndexBuffer() const
{
#if defined(USE_OPENGL_ES)
	return (LPDIRECT3DINDEXBUFFER9)(uintptr_t)m_uEBO;
#else
	return m_lpd3dIdxBuf;
#endif
}

void CGraphicIndexBuffer::SetIndices(int startIndex) const
{
#if defined(USE_OPENGL_ES)
	if (m_uEBO)
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uEBO);
#else
	assert(ms_lpd3dDevice!=nullptr);
	STATEMANAGER.SetIndices(m_lpd3dIdxBuf, startIndex);
#endif
}

bool CGraphicIndexBuffer::Lock(void** pretIndices) const
{
#if defined(USE_OPENGL_ES)
	if (!m_uEBO)
		return false;
	if (!m_pData && m_dwBufferSize > 0)
		m_pData = malloc(m_dwBufferSize);
	*pretIndices = m_pData;
	return (m_pData != nullptr);
#else
	assert(m_lpd3dIdxBuf!=nullptr);

	if (FAILED(m_lpd3dIdxBuf->Lock(0, 0, pretIndices, 0)))
		return false;

	return true;
#endif
}

void CGraphicIndexBuffer::Unlock() const
{
#if defined(USE_OPENGL_ES)
	if (!m_uEBO || !m_pData)
		return;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uEBO);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_dwBufferSize, m_pData);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#else
	assert(m_lpd3dIdxBuf!=nullptr);

	m_lpd3dIdxBuf->Unlock();
#endif
}

bool CGraphicIndexBuffer::Lock(void** pretIndices)
{
#if defined(USE_OPENGL_ES)
	if (!m_uEBO)
		return false;
	if (!m_pData && m_dwBufferSize > 0)
		m_pData = malloc(m_dwBufferSize);
	*pretIndices = m_pData;
	return (m_pData != nullptr);
#else
	assert(m_lpd3dIdxBuf!=nullptr);

	if (FAILED(m_lpd3dIdxBuf->Lock(0, 0, pretIndices, 0)))
		return false;

	return true;
#endif
}

void CGraphicIndexBuffer::Unlock()
{
#if defined(USE_OPENGL_ES)
	if (!m_uEBO || !m_pData)
		return;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uEBO);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_dwBufferSize, m_pData);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#else
	assert(m_lpd3dIdxBuf!=nullptr);

	m_lpd3dIdxBuf->Unlock();
#endif
}

bool CGraphicIndexBuffer::Copy(int bufSize, const void* srcIndices) const
{
#if defined(USE_OPENGL_ES)
	void* dstIndices;
	if (!Lock(&dstIndices))
		return false;

	memcpy(dstIndices, srcIndices, bufSize);

	Unlock();
	return true;
#else
	assert(m_lpd3dIdxBuf!=nullptr);

	BYTE* dstIndices;
	if (FAILED(m_lpd3dIdxBuf->Lock(0, 0, (void**)&dstIndices, 0)))
		return false;

	memcpy(dstIndices, srcIndices, bufSize);

	m_lpd3dIdxBuf->Unlock();

	return true;
#endif
}

bool CGraphicIndexBuffer::Create(int faceCount, TFace* faces)
{
	const int idxCount = faceCount * 3;
	m_iidxCount = idxCount;
	if (!Create(idxCount, D3DFMT_INDEX16))
		return false;

#if defined(USE_OPENGL_ES)
	WORD* dstIndices;
	if (!Lock((void**)&dstIndices))
		return false;

	for (int i = 0; i < faceCount; ++i, dstIndices += 3)
	{
		const TFace* curFace = faces + i;
		dstIndices[0] = curFace->indices[0];
		dstIndices[1] = curFace->indices[1];
		dstIndices[2] = curFace->indices[2];
	}

	Unlock();
	return true;
#else
	WORD* dstIndices;
	if (FAILED(m_lpd3dIdxBuf->Lock(0, 0, (void**)&dstIndices, 0)))
		return false;

	for (int i = 0; i<faceCount; ++i, dstIndices+=3)
	{
		const TFace * curFace=faces+i;
		dstIndices[0]=curFace->indices[0];
		dstIndices[1]=curFace->indices[1];
		dstIndices[2]=curFace->indices[2];
	}

	m_lpd3dIdxBuf->Unlock();
	return true;
#endif
}

bool CGraphicIndexBuffer::CreateDeviceObjects()
{
#if defined(USE_OPENGL_ES)
	if (m_uEBO != 0)
		return true;

	glGenBuffers(1, &m_uEBO);
	if (!m_uEBO)
		return false;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_dwBufferSize, nullptr, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	if (!m_pData && m_dwBufferSize > 0)
		m_pData = malloc(m_dwBufferSize);

	return true;
#else
	assert(ms_lpd3dDevice != nullptr);
	assert(m_lpd3dIdxBuf == nullptr);

	if (FAILED(
		ms_lpd3dDevice->CreateIndexBuffer(
			m_dwBufferSize,
			D3DUSAGE_WRITEONLY,
			m_d3dFmt,
			D3DPOOL_DEFAULT,
			&m_lpd3dIdxBuf,
			nullptr)
	))
		return false;

	return true;
#endif
}

void CGraphicIndexBuffer::DestroyDeviceObjects()
{
#if defined(USE_OPENGL_ES)
	if (m_uEBO)
	{
		glDeleteBuffers(1, &m_uEBO);
		m_uEBO = 0;
	}
	if (m_pData)
	{
		free(m_pData);
		m_pData = nullptr;
	}
#else
	safe_release(m_lpd3dIdxBuf);
#endif
}

bool CGraphicIndexBuffer::Create(int idxCount, D3DFORMAT d3dFmt)
{
#if !defined(USE_OPENGL_ES)
	assert(ms_lpd3dDevice!=nullptr);
#endif
	assert(idxCount > 0);

	Destroy();

	m_iidxCount = idxCount;
	m_dwBufferSize = sizeof(WORD) * idxCount;
	m_d3dFmt = d3dFmt;

	return CreateDeviceObjects();
}

void CGraphicIndexBuffer::Destroy()
{
	DestroyDeviceObjects();
}

void CGraphicIndexBuffer::Initialize()
{
	m_lpd3dIdxBuf = nullptr;
#if defined(USE_OPENGL_ES)
	m_uEBO = 0;
	m_pData = nullptr;
#endif
	m_iidxCount = 0;
	m_dwBufferSize = 0;
	m_d3dFmt = D3DFMT_UNKNOWN;
}

CGraphicIndexBuffer::CGraphicIndexBuffer()
{
	Initialize();
}

CGraphicIndexBuffer::~CGraphicIndexBuffer()
{
	Destroy();
}
