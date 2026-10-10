#include "StdAfx.h"
#include "../EterBase/Stl.h"
#include "GrpVertexBuffer.h"
#include "StateManager.h"

int	CGraphicVertexBuffer::GetVertexStride() const
{
	const int retSize = D3DXGetFVFVertexSize(m_dwFVF);
	return retSize;
}

DWORD CGraphicVertexBuffer::GetFlexibleVertexFormat() const
{
	return m_dwFVF;
}

int CGraphicVertexBuffer::GetVertexCount() const
{
	return m_vtxCount;
}

void CGraphicVertexBuffer::SetStream(int stride, int layer) const
{
#if defined(USE_OPENGL_ES)
	if (m_uVBO)
		glBindBuffer(GL_ARRAY_BUFFER, m_uVBO);
#else
	assert(ms_lpd3dDevice != nullptr);
	STATEMANAGER.SetStreamSource(layer, m_lpd3dVB, stride);
#endif
}

bool CGraphicVertexBuffer::LockRange(unsigned count, void** pretVertices) const
{
#if defined(USE_OPENGL_ES)
	return Lock(pretVertices);
#else
	if (!m_lpd3dVB)
		return false;

	const DWORD dwLockSize=GetVertexStride() * count;
	if (FAILED(m_lpd3dVB->Lock(0, dwLockSize, pretVertices, m_dwLockFlag)))
		return false;

	return true;
#endif
}

bool CGraphicVertexBuffer::Lock(void ** pretVertices) const
{
#if defined(USE_OPENGL_ES)
	if (!m_uVBO)
		return false;
	if (!m_pData && m_dwBufferSize > 0)
		m_pData = malloc(m_dwBufferSize);
	*pretVertices = m_pData;
	return (m_pData != nullptr);
#else
	if (!m_lpd3dVB)
		return false;

	const DWORD dwLockSize=GetVertexStride()*GetVertexCount();
	if (FAILED(m_lpd3dVB->Lock(0, dwLockSize, pretVertices, m_dwLockFlag)))
		return false;

	return true;
#endif
}

bool CGraphicVertexBuffer::Unlock() const
{
#if defined(USE_OPENGL_ES)
	if (!m_uVBO || !m_pData)
		return false;

	glBindBuffer(GL_ARRAY_BUFFER, m_uVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_dwBufferSize, m_pData);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
#else
	if (!m_lpd3dVB)
		return false;

	if ( FAILED(m_lpd3dVB->Unlock()) )
		return false;
	return true;
#endif
}

bool CGraphicVertexBuffer::IsEmpty() const
{
#if defined(USE_OPENGL_ES)
	return (m_uVBO != 0);
#else
	if (m_lpd3dVB)
		return true;
	else
		return false;
#endif
}

bool CGraphicVertexBuffer::LockDynamic(void** pretVertices) const
{
#if defined(USE_OPENGL_ES)
	return Lock(pretVertices);
#else
	if (!m_lpd3dVB)
		return false;

	if (FAILED(m_lpd3dVB->Lock(0, 0, pretVertices, 0)))
		return false;

	return true;
#endif
}

bool CGraphicVertexBuffer::Lock(void ** pretVertices)
{
#if defined(USE_OPENGL_ES)
	if (!m_uVBO)
		return false;
	if (!m_pData && m_dwBufferSize > 0)
		m_pData = malloc(m_dwBufferSize);
	*pretVertices = m_pData;
	return (m_pData != nullptr);
#else
	if (!m_lpd3dVB)
		return false;

	if (FAILED(m_lpd3dVB->Lock(0, 0, pretVertices, m_dwLockFlag)))
		return false;

	return true;
#endif
}

bool CGraphicVertexBuffer::Unlock()
{
#if defined(USE_OPENGL_ES)
	if (!m_uVBO || !m_pData)
		return false;

	glBindBuffer(GL_ARRAY_BUFFER, m_uVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_dwBufferSize, m_pData);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
#else
	if (!m_lpd3dVB)
		return false;

	if ( FAILED(m_lpd3dVB->Unlock()) )
		return false;
	return true;
#endif
}

bool CGraphicVertexBuffer::Copy(int bufSize, const void* srcVertices)
{
	void * dstVertices;

	if (!Lock(&dstVertices))
		return false;

	memcpy(dstVertices, srcVertices, bufSize);

	Unlock();
	return true;
}

bool CGraphicVertexBuffer::CreateDeviceObjects()
{
#if defined(USE_OPENGL_ES)
	if (m_uVBO != 0)
		return true;

	glGenBuffers(1, &m_uVBO);
	if (!m_uVBO)
		return false;

	glBindBuffer(GL_ARRAY_BUFFER, m_uVBO);
	glBufferData(GL_ARRAY_BUFFER, m_dwBufferSize, nullptr, (m_dwUsage & D3DUSAGE_DYNAMIC) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (!m_pData && m_dwBufferSize > 0)
		m_pData = malloc(m_dwBufferSize);

	return true;
#else
	assert(ms_lpd3dDevice != nullptr);
	assert(m_lpd3dVB == nullptr);

	if (FAILED(
		ms_lpd3dDevice->CreateVertexBuffer(
		m_dwBufferSize,
		m_dwUsage,
		m_dwFVF,
		m_d3dPool,
		&m_lpd3dVB, nullptr)
		))
		return false;

	return true;
#endif
}

void CGraphicVertexBuffer::DestroyDeviceObjects()
{
#if defined(USE_OPENGL_ES)
	if (m_uVBO)
	{
		glDeleteBuffers(1, &m_uVBO);
		m_uVBO = 0;
	}
	if (m_pData)
	{
		free(m_pData);
		m_pData = nullptr;
	}
#else
	safe_release(m_lpd3dVB);
#endif
}

bool CGraphicVertexBuffer::Create(int vtxCount, DWORD fvf, DWORD usage, D3DPOOL d3dPool)
{
#if !defined(USE_OPENGL_ES)
	assert(ms_lpd3dDevice != nullptr);
#endif
	assert(vtxCount > 0);

	Destroy();

	m_vtxCount = vtxCount;
	m_dwBufferSize = D3DXGetFVFVertexSize(fvf) * m_vtxCount;
	m_d3dPool = d3dPool;
	m_dwUsage = usage;
	m_dwFVF = fvf;

	if (usage == D3DUSAGE_WRITEONLY || usage == D3DUSAGE_DYNAMIC)
		m_dwLockFlag = 0;
	else
		m_dwLockFlag = D3DLOCK_READONLY;

	return CreateDeviceObjects();
}

void CGraphicVertexBuffer::Destroy()
{
	DestroyDeviceObjects();
}

void CGraphicVertexBuffer::Initialize()
{
	m_lpd3dVB = nullptr;
#if defined(USE_OPENGL_ES)
	m_uVBO = 0;
	m_pData = nullptr;
#endif
	m_vtxCount = 0;
	m_dwBufferSize = 0;
}

CGraphicVertexBuffer::CGraphicVertexBuffer()
{
	Initialize();
}

CGraphicVertexBuffer::~CGraphicVertexBuffer()
{
	Destroy();
}
