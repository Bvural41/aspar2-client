#pragma once

#include "GrpImage.h"
#include "GrpIndexBuffer.h"
#include "GrpVertexBufferDynamic.h"
#include "Pool.h"
#include "../UserInterface/Locale_inc.h"

class CGraphicImageInstance
{
	public:
		static DWORD Type();
		BOOL IsType(DWORD dwType);

	public:
		CGraphicImageInstance();
		virtual ~CGraphicImageInstance();

		void Destroy();

#ifdef ENABLE_USE_CLIP_MASK
		void Render(RECT* pClipRect = nullptr);
#else
		void Render();
#endif
#ifdef ENABLE_USE_CLIP_MASK
		void RenderCoolTime(float fCoolTime, RECT* pClipRect = nullptr);
#else
		void RenderCoolTime(float fCoolTime);
#endif

		void SetDiffuseColor(float fr, float fg, float fb, float fa);
		void SetPosition(float fx, float fy);
		void SetScale(float sx, float sy);
		void SetImagePointer(CGraphicImage* pImage);
		void ReloadImagePointer(CGraphicImage* pImage);
		bool IsEmpty() const;

		int GetWidth();
		int GetHeight();

		CGraphicTexture * GetTexturePointer();
		const CGraphicTexture &	GetTextureReference() const;
		CGraphicImage * GetGraphicImagePointer();

		bool operator == (const CGraphicImageInstance & rhs) const;

		D3DXCOLOR GetPixelColor(int x, int y);

	protected:
		void Initialize();

#ifdef ENABLE_USE_CLIP_MASK
		virtual void OnRender(RECT* pClipRect);
		virtual void OnRenderCoolTime(float fCoolTime, RECT* pClipRect);
#else
		virtual void OnRender();
		virtual void OnRenderCoolTime(float fCoolTime);
#endif

		virtual void OnSetImagePointer();

		virtual BOOL OnIsType(DWORD dwType);

	protected:
		D3DXCOLOR m_DiffuseColor;
		D3DXVECTOR2 m_v2Position;
		D3DXVECTOR2 m_v2Scale;

		CGraphicImage::TRef m_roImage;

	public:
		static void CreateSystem(UINT uCapacity);
		static void DestroySystem();

		static CGraphicImageInstance* New();
		static void Delete(CGraphicImageInstance* pkImgInst);

		static CDynamicPool<CGraphicImageInstance>		ms_kPool;
};
