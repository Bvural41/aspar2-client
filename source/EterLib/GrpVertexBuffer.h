#pragma once

#include "GrpBase.h"

class CGraphicVertexBuffer : public CGraphicBase
{
	public:
		CGraphicVertexBuffer();
		virtual ~CGraphicVertexBuffer();

		void	Destroy();
		virtual bool	Create(int vtxCount, DWORD fvf, DWORD usage, D3DPOOL d3dPool);

		bool	CreateDeviceObjects();
		void	DestroyDeviceObjects();

		bool	Copy(int bufSize, const void* srcVertices);

		bool	LockRange(unsigned count, void** pretVertices) const;
		bool	Lock(void** pretVertices) const;
		bool	Unlock() const;

		bool	LockDynamic(void** pretVertices) const;
		virtual bool	Lock(void** pretVertices);
		bool	Unlock();

		void	SetStream(int stride, int layer=0) const;

		int		GetVertexCount() const;
		int		GetVertexStride() const;
		DWORD	GetFlexibleVertexFormat() const;

#if defined(USE_OPENGL_ES)
		inline	LPDIRECT3DVERTEXBUFFER9 GetD3DVertexBuffer() const	{ return (LPDIRECT3DVERTEXBUFFER9)(uintptr_t)m_uVBO; }
#else
		inline	LPDIRECT3DVERTEXBUFFER9 GetD3DVertexBuffer() const	{ return m_lpd3dVB; }
#endif
		inline	DWORD GetBufferSize() const	{ return m_dwBufferSize; }

#if defined(USE_OPENGL_ES)
		inline	unsigned int GetVBO() const { return m_uVBO; }
#endif

		bool	IsEmpty() const;

	protected:
		void	Initialize();

	protected:
		LPDIRECT3DVERTEXBUFFER9 m_lpd3dVB;
#if defined(USE_OPENGL_ES)
		unsigned int			m_uVBO;
		mutable void*			m_pData;
#endif

		DWORD					m_dwBufferSize;
		DWORD					m_dwFVF;
		DWORD					m_dwUsage;
		D3DPOOL					m_d3dPool;
		int						m_vtxCount;
		DWORD					m_dwLockFlag;
};

