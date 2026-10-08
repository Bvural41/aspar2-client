#pragma once

#include "GrpBase.h"

class CGraphicTexture : public CGraphicBase
{
	public:
		virtual bool IsEmpty() const;

		int GetWidth() const;
		int GetHeight() const;

		void SetTextureStage(int stage) const;
		LPDIRECT3DTEXTURE9 GetD3DTexture() const;

#if defined(USE_OPENGL_ES)
		inline unsigned int GetGLTexture() const { return m_uGLTexture; }
#endif

		void DestroyDeviceObjects();

	protected:
		CGraphicTexture();
		virtual	~CGraphicTexture();

		void Destroy();
		void Initialize();

	protected:
		bool m_bEmpty;

		int m_width;
		int m_height;

		LPDIRECT3DTEXTURE9 m_lpd3dTexture;
#if defined(USE_OPENGL_ES)
		unsigned int m_uGLTexture;
#endif
};
