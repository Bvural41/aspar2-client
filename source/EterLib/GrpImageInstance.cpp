#include "StdAfx.h"

#if defined(USE_OPENGL_ES)
static const float c_fHalfPixel = 0.0f;
#else
static const float c_fHalfPixel = 0.5f;
#endif
#include "GrpImageInstance.h"
#include "StateManager.h"
#include "../UserInterface/Locale_inc.h"
#include "../eterBase/CRC32.h"
//STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
//STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
//STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
//STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);

CDynamicPool<CGraphicImageInstance>		CGraphicImageInstance::ms_kPool;

void CGraphicImageInstance::CreateSystem(UINT uCapacity)
{
	ms_kPool.Create(uCapacity);
}

void CGraphicImageInstance::DestroySystem()
{
	ms_kPool.Destroy();
}

CGraphicImageInstance* CGraphicImageInstance::New()
{
	return ms_kPool.Alloc();
}

void CGraphicImageInstance::Delete(CGraphicImageInstance* pkImgInst)
{
	pkImgInst->Destroy();
	ms_kPool.Free(pkImgInst);
}

#ifdef ENABLE_USE_CLIP_MASK
void CGraphicImageInstance::Render(RECT* pClipRect)
#else
void CGraphicImageInstance::Render()
#endif
{
	if (IsEmpty())
		return;

	assert(!IsEmpty());

#ifdef ENABLE_USE_CLIP_MASK
	OnRender(pClipRect);
#else
	OnRender();
#endif
}

#ifdef ENABLE_USE_CLIP_MASK
void CGraphicImageInstance::OnRender(RECT* pClipRect)
#else
void CGraphicImageInstance::OnRender()
#endif
{
#ifdef USE_D3D9
	D3DPERF_BeginEvent(D3DCOLOR_ARGB(255, 50, 50, 0), L"** CGraphicImageInstance::OnRender **");
#endif

	if (IsEmpty() || !m_roImage.GetPointer())
		return;

	const auto pTexture = m_roImage->GetTexturePointer();
	if (!pTexture)
		return;

	const auto fimgWidth = m_roImage->GetWidth() * m_v2Scale.x;
	const auto fimgHeight = m_roImage->GetHeight() * m_v2Scale.y;

	const auto& c_rRect = m_roImage->GetRectReference();
	const auto texReverseWidth = 1.0f / float(pTexture->GetWidth());
	const auto texReverseHeight = 1.0f / float(pTexture->GetHeight());

	auto su = c_rRect.left * texReverseWidth;
	auto sv = c_rRect.top * texReverseHeight;
	auto eu = (c_rRect.left + (c_rRect.right - c_rRect.left)) * texReverseWidth;
	auto ev = (c_rRect.top + (c_rRect.bottom - c_rRect.top)) * texReverseHeight;

	TPDTVertex vertices[4];

#ifdef ENABLE_USE_CLIP_MASK
	if (pClipRect)
	{
		auto sx = m_v2Position.x - c_fHalfPixel;
		auto sy = m_v2Position.y - c_fHalfPixel;
		auto ex = m_v2Position.x + fimgWidth - c_fHalfPixel;
		auto ey = m_v2Position.y + fimgHeight - c_fHalfPixel;

		const auto width = ex - sx;
		const auto height = ey - sy;
		const auto uDiff = eu - su;
		const auto vDiff = ev - sv;

		if (ex < pClipRect->left)
		{
			return;
		}

		if (ey < pClipRect->top)
		{
			return;
		}

		if (sx > pClipRect->right)
		{
			return;
		}

		if (sy > pClipRect->bottom)
		{
			return;
		}

		if (sx < pClipRect->left)
		{
			su += (pClipRect->left - sx) / width * uDiff;
			sx = pClipRect->left;
		}

		if (sy < pClipRect->top)
		{
			sv += (pClipRect->top - sy) / height * vDiff;
			sy = pClipRect->top;
		}

		if (ex > pClipRect->right)
		{
			eu -= (ex - pClipRect->right) / width * uDiff;
			ex = pClipRect->right;
		}

		if (ey > pClipRect->bottom)
		{
			ev -= (ey - pClipRect->bottom) / height * vDiff;
			ey = pClipRect->bottom;
		}

		vertices[0].position.x = sx;
		vertices[0].position.y = sy;
		vertices[0].position.z = 0.0f;
		vertices[0].texCoord = TTextureCoordinate(su, sv);
		vertices[0].diffuse = m_DiffuseColor;

		vertices[1].position.x = ex;
		vertices[1].position.y = sy;
		vertices[1].position.z = 0.0f;
		vertices[1].texCoord = TTextureCoordinate(eu, sv);
		vertices[1].diffuse = m_DiffuseColor;

		vertices[2].position.x = sx;
		vertices[2].position.y = ey;
		vertices[2].position.z = 0.0f;
		vertices[2].texCoord = TTextureCoordinate(su, ev);
		vertices[2].diffuse = m_DiffuseColor;

		vertices[3].position.x = ex;
		vertices[3].position.y = ey;
		vertices[3].position.z = 0.0f;
		vertices[3].texCoord = TTextureCoordinate(eu, ev);
		vertices[3].diffuse = m_DiffuseColor;
	}
	else
	{
#endif
	vertices[0].position.x = m_v2Position.x - c_fHalfPixel;
	vertices[0].position.y = m_v2Position.y - c_fHalfPixel;
	vertices[0].position.z = 0.0f;
	vertices[0].texCoord = TTextureCoordinate(su, sv);
	vertices[0].diffuse = m_DiffuseColor;

	vertices[1].position.x = m_v2Position.x + fimgWidth - c_fHalfPixel;
	vertices[1].position.y = m_v2Position.y - c_fHalfPixel;
	vertices[1].position.z = 0.0f;
	vertices[1].texCoord = TTextureCoordinate(eu, sv);
	vertices[1].diffuse = m_DiffuseColor;

	vertices[2].position.x = m_v2Position.x - c_fHalfPixel;
	vertices[2].position.y = m_v2Position.y + fimgHeight - c_fHalfPixel;
	vertices[2].position.z = 0.0f;
	vertices[2].texCoord = TTextureCoordinate(su, ev);
	vertices[2].diffuse = m_DiffuseColor;

	vertices[3].position.x = m_v2Position.x + fimgWidth - c_fHalfPixel;
	vertices[3].position.y = m_v2Position.y + fimgHeight - c_fHalfPixel;
	vertices[3].position.z = 0.0f;
	vertices[3].texCoord = TTextureCoordinate(eu, ev);
	vertices[3].diffuse = m_DiffuseColor;
#ifdef ENABLE_USE_CLIP_MASK
	}
#endif

	if (CGraphicBase::SetPDTStream(vertices, 4))
	{
		CGraphicBase::SetDefaultIndexBuffer(CGraphicBase::DEFAULT_IB_FILL_RECT);

		STATEMANAGER.SetTexture(0, pTexture->GetD3DTexture());
		STATEMANAGER.SetTexture(1, NULL);

#ifdef USE_D3D9
		STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
#else
		STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
#endif

		STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2);
	}

#ifdef USE_D3D9
	D3DPERF_EndEvent();
#endif
}

const CGraphicTexture & CGraphicImageInstance::GetTextureReference() const
{
	return m_roImage->GetTextureReference();
}

CGraphicTexture * CGraphicImageInstance::GetTexturePointer()
{
	CGraphicImage* pkImage = m_roImage.GetPointer();
	return pkImage ? pkImage->GetTexturePointer() : NULL;
}

CGraphicImage * CGraphicImageInstance::GetGraphicImagePointer()
{
	return m_roImage.GetPointer();
}

int CGraphicImageInstance::GetWidth()
{
	if (IsEmpty())
		return 0;
	
	return m_roImage->GetWidth();
}

int CGraphicImageInstance::GetHeight()
{
	if (IsEmpty())
		return 0;
	
	return m_roImage->GetHeight();
}

void CGraphicImageInstance::SetDiffuseColor(float fr, float fg, float fb, float fa)
{
	m_DiffuseColor.r = fr;
	m_DiffuseColor.g = fg;
	m_DiffuseColor.b = fb;
	m_DiffuseColor.a = fa;
}
void CGraphicImageInstance::SetPosition(float fx, float fy)
{
	m_v2Position.x = fx;
	m_v2Position.y = fy;
}

void CGraphicImageInstance::SetScale(float fx, float fy)
{
	m_v2Scale.x = fx;
	m_v2Scale.y = fy;
}

void CGraphicImageInstance::SetImagePointer(CGraphicImage * pImage)
{
	m_roImage.SetPointer(pImage);

	OnSetImagePointer();
}

void CGraphicImageInstance::ReloadImagePointer(CGraphicImage * pImage)
{
	if (m_roImage.IsNull())
	{
		SetImagePointer(pImage);
		return;
	}

	CGraphicImage * pkImage = m_roImage.GetPointer();

	if (pkImage)
		pkImage->Reload();
}

bool CGraphicImageInstance::IsEmpty() const
{
	if (!m_roImage.IsNull() && !m_roImage->IsEmpty())
		return false;

	return true;
}

bool CGraphicImageInstance::operator == (const CGraphicImageInstance & rhs) const
{
	return (m_roImage.GetPointer() == rhs.m_roImage.GetPointer());
}

DWORD CGraphicImageInstance::Type()
{
	static DWORD s_dwType = GetCRC32("CGraphicImageInstance", strlen("CGraphicImageInstance"));
	return (s_dwType);
}

BOOL CGraphicImageInstance::IsType(DWORD dwType)
{
	return OnIsType(dwType);
}

BOOL CGraphicImageInstance::OnIsType(DWORD dwType)
{
	if (CGraphicImageInstance::Type() == dwType)
		return TRUE;

	return FALSE;
}

void CGraphicImageInstance::OnSetImagePointer()
{
}

void CGraphicImageInstance::Initialize()
{
	m_DiffuseColor.r = m_DiffuseColor.g = m_DiffuseColor.b = m_DiffuseColor.a = 1.0f;
	m_v2Position.x = m_v2Position.y = 0.0f;
	m_v2Scale.x = m_v2Scale.y = 1.0f;
}

void CGraphicImageInstance::Destroy()
{
	m_roImage.SetPointer(NULL); // CRef 에서 레퍼런스 카운트가 떨어져야 함.
	Initialize();
}

CGraphicImageInstance::CGraphicImageInstance()
{
	Initialize();
}

CGraphicImageInstance::~CGraphicImageInstance()
{
	Destroy();
}

D3DXCOLOR CGraphicImageInstance::GetPixelColor(int x, int y)
{
	// we first need the d3d texture, but its the "shortest" way to get it
	D3DXCOLOR dxClr = D3DXCOLOR(0, 0, 0, 0);
	CGraphicImage * pImage = m_roImage.GetPointer();
	if (!pImage)
		return dxClr;
	CGraphicTexture * pTexture = pImage->GetTexturePointer();
	if (!pTexture)
		return dxClr;

	LPDIRECT3DTEXTURE9 d3dTexture = pTexture->GetD3DTexture();
	if (!d3dTexture)
		return dxClr;

	IDirect3DSurface9* surface;
	D3DSURFACE_DESC desc;
	D3DLOCKED_RECT rect;
	RECT rc;

	// we want just want to lock only one pixel
	rc.left = x;
	rc.right = x + 1;
	rc.top = y;
	rc.bottom = y + 1;

	if (FAILED(d3dTexture->GetSurfaceLevel(0, &surface))) // get the top surface of the image (it contains the whole image)
		return dxClr;
	if (FAILED(surface->GetDesc(&desc)))
		return dxClr;
	if (FAILED(surface->LockRect(&rect, &rc, D3DLOCK_READONLY | D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_NOSYSLOCK))) // lock the pixel
		return dxClr;

	PBYTE dwTexel = (PBYTE)rect.pBits;

	switch (desc.Format)
	{
	// there are several possible image formats, but its the most common one and as I saw its more than enough
	case D3DFMT_A8R8G8B8:
		dxClr.a = dwTexel[3];
		dxClr.r = dwTexel[2];
		dxClr.g = dwTexel[1];
		dxClr.b = dwTexel[0];
		break;
	}
	surface->UnlockRect(); // unlock the pixel for further using (like render)

	return dxClr;
}

#ifdef ENABLE_USE_CLIP_MASK
void CGraphicImageInstance::RenderCoolTime(float fCoolTime, RECT* pClipRect)
#else
void CGraphicImageInstance::RenderCoolTime(float fCoolTime)
#endif
{
	if (IsEmpty())
		return;

	assert(!IsEmpty());

#ifdef ENABLE_USE_CLIP_MASK
    OnRenderCoolTime(fCoolTime, pClipRect);
#else
    OnRenderCoolTime(fCoolTime);
#endif
}

#ifdef ENABLE_USE_CLIP_MASK
void CGraphicImageInstance::OnRenderCoolTime(float fCoolTime, RECT* pClipRect)
#else
void CGraphicImageInstance::OnRenderCoolTime(float fCoolTime)
#endif
{
	if (fCoolTime >= 1.0f)
		fCoolTime = 1.0f;

	CGraphicImage * pImage = m_roImage.GetPointer();
	CGraphicTexture * pTexture = pImage->GetTexturePointer();

	float fimgWidth = pImage->GetWidth() * m_v2Scale.x;
	float fimgHeight = pImage->GetHeight() * m_v2Scale.y;
	float fimgWidthHalf = fimgWidth * 0.5f;
	float fimgHeightHalf = fimgHeight * 0.5f;

	const RECT& c_rRect = pImage->GetRectReference();
	float texReverseWidth = 1.0f / float(pTexture->GetWidth());
	float texReverseHeight = 1.0f / float(pTexture->GetHeight());
	float su = c_rRect.left * texReverseWidth;
	float sv = c_rRect.top * texReverseHeight;
	float eu = c_rRect.right * texReverseWidth;
	float ev = c_rRect.bottom * texReverseHeight;
	float euh = eu * 0.5f;
	float evh = ev * 0.5f;
	float fxCenter = m_v2Position.x + fimgWidthHalf - c_fHalfPixel;
	float fyCenter = m_v2Position.y + fimgHeightHalf - c_fHalfPixel;

	if (fCoolTime < 1.0f)
	{
		if (fCoolTime < 0.0)
			fCoolTime = 0.0;

		const int c_iTriangleCountPerBox = 8;
		static D3DXVECTOR2 s_v2BoxPos[c_iTriangleCountPerBox] =
		{
			D3DXVECTOR2(-1.0f, -1.0f),
			D3DXVECTOR2(-1.0f,  0.0f),
			D3DXVECTOR2(-1.0f, +1.0f),
			D3DXVECTOR2( 0.0f, +1.0f),
			D3DXVECTOR2(+1.0f, +1.0f),
			D3DXVECTOR2(+1.0f,  0.0f),
			D3DXVECTOR2(+1.0f, -1.0f),
			D3DXVECTOR2( 0.0f, -1.0f),
		};
		 
		D3DXVECTOR2 v2TexPos[c_iTriangleCountPerBox] =
		{
			D3DXVECTOR2( su,  sv),
			D3DXVECTOR2( su, evh),
			D3DXVECTOR2( su,  ev),
			D3DXVECTOR2(euh,  ev),
			D3DXVECTOR2( eu,  ev),
			D3DXVECTOR2( eu, evh),
			D3DXVECTOR2( eu,  sv),
			D3DXVECTOR2(euh,  sv),
		};

		int iTriCount = int(8.0f - 8.0f * fCoolTime);
		float fLastPercentage = (8.0f - 8.0f * fCoolTime) - iTriCount;

		std::vector<TPDTVertex> vertices;
		TPDTVertex vertex;
		vertex.position = TPosition(fxCenter, fyCenter, 0.0f);
		vertex.texCoord = TTextureCoordinate(euh, evh);
		vertex.diffuse = m_DiffuseColor;
		vertices.push_back(vertex);

		vertex.position = TPosition(fxCenter, fyCenter - fimgHeightHalf - c_fHalfPixel, 0.0f);
		vertex.texCoord = TTextureCoordinate(euh, sv);
		vertex.diffuse = m_DiffuseColor;
		vertices.push_back(vertex);

		if (iTriCount > 0)
		{
			for (int j = 0; j < iTriCount; ++j)
			{
				vertex.position = TPosition(fxCenter + (s_v2BoxPos[j].x * fimgWidthHalf) - c_fHalfPixel,
											fyCenter + (s_v2BoxPos[j].y * fimgHeightHalf) - c_fHalfPixel,
											0.0f);
				vertex.texCoord = TTextureCoordinate(v2TexPos[j & (c_iTriangleCountPerBox - 1)].x,
													 v2TexPos[j & (c_iTriangleCountPerBox - 1)].y);
				vertex.diffuse = m_DiffuseColor;
				vertices.push_back(vertex);
			}
		}

		if (fLastPercentage > 0.0f)
		{
			D3DXVECTOR2 * pv2Pos;
			D3DXVECTOR2 * pv2LastPos;
			assert((iTriCount+8)%8>=0&&(iTriCount+8)%8<8);
			assert((iTriCount+7)%8>=0&&(iTriCount+7)%8<8);
			pv2LastPos=&s_v2BoxPos[(iTriCount+8)%8];
			pv2Pos=&s_v2BoxPos[(iTriCount+7)%8];
			float fxShit = (pv2LastPos->x - pv2Pos->x) * fLastPercentage + pv2Pos->x;
			float fyShit = (pv2LastPos->y - pv2Pos->y) * fLastPercentage + pv2Pos->y;
			vertex.position = TPosition(fimgWidthHalf * fxShit + fxCenter - c_fHalfPixel,
										fimgHeightHalf * fyShit + fyCenter - c_fHalfPixel,
										0.0f);
			vertex.texCoord = TTextureCoordinate(euh * fxShit + euh,
												 evh * fyShit + evh);
			vertex.diffuse = m_DiffuseColor;
			vertices.push_back(vertex);
			++iTriCount;
		}

		if (vertices.empty())
			return;

		STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		if (CGraphicBase::SetPDTStream(&vertices[0], vertices.size()))
		{
			CGraphicBase::SetDefaultIndexBuffer(CGraphicBase::DEFAULT_IB_FILL_TRI);
			STATEMANAGER.SetTexture(0, pTexture->GetD3DTexture());
			STATEMANAGER.SetTexture(1, NULL);
#ifdef ENABLE_D3DX9
			STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
#else
			STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
#endif
			STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLEFAN, 0, iTriCount);
		}
		STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	}
}
