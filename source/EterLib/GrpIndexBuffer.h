#pragma once

#include "GrpBase.h"

class CGraphicIndexBuffer : public CGraphicBase
{
	public:
		CGraphicIndexBuffer();
		virtual ~CGraphicIndexBuffer();

		void Destroy();
		bool Create(int idxCount, D3DFORMAT d3dFmt);
		bool Create(int faceCount, TFace* faces);

		bool CreateDeviceObjects();
		void DestroyDeviceObjects();

		bool Copy(int bufSize, const void* srcIndices) const;

		bool Lock(void** pretIndices) const;
		void Unlock() const;

		bool Lock(void** pretIndices);
		void Unlock();

		void SetIndices(int startIndex=0) const;

		LPDIRECT3DINDEXBUFFER9 GetD3DIndexBuffer() const;
#if defined(USE_OPENGL_ES)
		inline unsigned int GetEBO() const { return m_uEBO; }
#endif

		int GetIndexCount() const {return m_iidxCount;}

	protected:
		void Initialize();

	protected:
		LPDIRECT3DINDEXBUFFER9	m_lpd3dIdxBuf;
#if defined(USE_OPENGL_ES)
		unsigned int			m_uEBO;
		mutable void*			m_pData;
#endif
		DWORD					m_dwBufferSize;
		D3DFORMAT				m_d3dFmt;
		int						m_iidxCount;
};
