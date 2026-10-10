#include "StdAfx.h"
#include "StateManager.h"
#include "GrpBase.h"
#include "Camera.h"
#include "../eterBase/Timer.h"

//#define StateManager_Assert(a) if (!(a)) puts("assert"#a)
#define StateManager_Assert(a) assert(a)

struct SLightData
{
	enum
	{
		LIGHT_NUM = 8,
	};
	D3DLIGHT9 m_akD3DLight[LIGHT_NUM];
} m_kLightData;

#if defined(USE_OPENGL_ES)
static uint32_t s_uCurrentEnabledAttribMask = 0;

static inline void SetEnabledAttribMask(uint32_t targetMask)
{
	if (s_uCurrentEnabledAttribMask == targetMask)
		return;

	for (int i = 0; i < 4; ++i)
	{
		uint32_t bit = (1u << i);
		bool targetEnabled = (targetMask & bit) != 0;
		bool currentEnabled = (s_uCurrentEnabledAttribMask & bit) != 0;
		if (targetEnabled != currentEnabled)
		{
			if (targetEnabled)
				glEnableVertexAttribArray(i);
			else
				glDisableVertexAttribArray(i);
		}
	}
	s_uCurrentEnabledAttribMask = targetMask;
}
#endif

void CStateManager::SetLight(DWORD index, CONST D3DLIGHT9* pLight) const
{
	assert(index<SLightData::LIGHT_NUM);
	m_kLightData.m_akD3DLight[index]=*pLight;

	if (m_lpD3DDev) m_lpD3DDev->SetLight(index, pLight);
#if defined(USE_OPENGL_ES)
	if (index == 0 && pLight && pLight->Type == D3DLIGHT_DIRECTIONAL)
	{
		const_cast<CStateManager*>(this)->SetSkySunData(-pLight->Direction.x, -pLight->Direction.y, -pLight->Direction.z, 1.0f);
	}
#endif
}

void CStateManager::GetLight(DWORD index, D3DLIGHT9* pLight) const
{
	assert(index<8);
	*pLight=m_kLightData.m_akD3DLight[index];
}

bool CStateManager::BeginScene()
{
	m_bScene=true;

#if defined(USE_OPENGL_ES)
	return true;
#else
	D3DXMATRIX m4Proj;
	D3DXMATRIX m4View;
	D3DXMATRIX m4World;
	GetTransform(D3DTS_WORLD, &m4World);
	GetTransform(D3DTS_PROJECTION, &m4Proj);
	GetTransform(D3DTS_VIEW, &m4View);
	SetTransform(D3DTS_WORLD, &m4World);
	SetTransform(D3DTS_PROJECTION, &m4Proj);
	SetTransform(D3DTS_VIEW, &m4View);

	if (FAILED(m_lpD3DDev->BeginScene()))
		return false;
	return true;
#endif
}

void CStateManager::EndScene()
{
#if defined(USE_OPENGL_ES)
	m_bScene=false;
#else
	if (m_lpD3DDev)
		m_lpD3DDev->EndScene();
	m_bScene=false;
#endif
}

CStateManager::CStateManager(LPDIRECT3DDEVICE9 lpDevice) : m_lpD3DDev(nullptr)
{
	m_bScene = false;
	m_dwBestMinFilter = D3DTEXF_LINEAR;
	m_dwBestMagFilter = D3DTEXF_LINEAR;
	SetDevice(lpDevice);
}

CStateManager::~CStateManager()
{
	if (m_lpD3DDev)
	{
		m_lpD3DDev->Release();
		m_lpD3DDev = nullptr;
	}
}

void CStateManager::SetDevice(LPDIRECT3DDEVICE9 lpDevice)
{
#if defined(USE_OPENGL_ES)
	m_lpD3DDev = nullptr;
	SetDefaultState();
#else
	StateManager_Assert(lpDevice);
	lpDevice->AddRef();

	if (m_lpD3DDev)
	{
		m_lpD3DDev->Release();
		m_lpD3DDev = nullptr;
	}

	m_lpD3DDev = lpDevice;

	SetDefaultState();
#endif
}

void CStateManager::SetBestFiltering(DWORD dwStage)
{
	SetSamplerState(dwStage, D3DSAMP_MINFILTER, m_dwBestMinFilter);
	SetSamplerState(dwStage, D3DSAMP_MAGFILTER, m_dwBestMagFilter);
	SetSamplerState(dwStage, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
}

void CStateManager::Restore()
{
	int i, j;

	m_bForce = true;

	for (i = 0; i < STATEMANAGER_MAX_RENDERSTATES; ++i)
		SetRenderState(D3DRENDERSTATETYPE(i), m_CurrentState.m_RenderStates[i]);

	for (i = 0; i < STATEMANAGER_MAX_STAGES; ++i)
		for (j = 0; j < STATEMANAGER_MAX_TEXTURESTATES; ++j)
			SetTextureStageState(i, D3DTEXTURESTAGESTATETYPE(j), m_CurrentState.m_TextureStates[i][j]);

	for (i = 0; i < STATEMANAGER_MAX_STAGES; ++i)
		for (j = 0; j < STATEMANAGER_MAX_SAMPLERSTATES; ++j)
			SetSamplerState(i, D3DSAMPLERSTATETYPE(j), m_CurrentState.m_SamplerStates[i][j]);

	for (i = 0; i < STATEMANAGER_MAX_STAGES; ++i)
		SetTexture(i, m_CurrentState.m_Textures[i]);

	m_bForce = false;
}

void CStateManager::SetDefaultState()
{
#if defined(USE_OPENGL_ES)
	m_dwBestMagFilter = D3DTEXF_LINEAR;
	m_dwBestMinFilter = D3DTEXF_LINEAR;
	SetEnabledAttribMask(0);
#else
	D3DCAPS9 d3dCaps;
	m_lpD3DDev->GetDeviceCaps(&d3dCaps);

	if (d3dCaps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC)
		m_dwBestMagFilter = D3DTEXF_ANISOTROPIC;
	else
		m_dwBestMagFilter = D3DTEXF_LINEAR;

	if (d3dCaps.TextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC)
		m_dwBestMinFilter = D3DTEXF_ANISOTROPIC;
	else
		m_dwBestMinFilter = D3DTEXF_LINEAR;

	DWORD dwMax = d3dCaps.MaxAnisotropy;
	dwMax = dwMax < 4 ? dwMax : 4;

	for (int i = 0; i < 8; ++i)
		m_lpD3DDev->SetSamplerState(i, D3DSAMP_MAXANISOTROPY, dwMax);
	// @fixme039 END
#endif

	m_CurrentState.ResetState();
	m_CopyState.ResetState();
	m_ChipState.ResetState();

	m_bScene = false;
	m_bForce = true;

	D3DXMATRIX Identity;
	D3DXMatrixIdentity(&Identity);

	SetTransform(D3DTS_WORLD, &Identity);
	SetTransform(D3DTS_VIEW, &Identity);
	SetTransform(D3DTS_PROJECTION, &Identity);

	D3DMATERIAL9 DefaultMat;
	ZeroMemory(&DefaultMat, sizeof(D3DMATERIAL9));

	DefaultMat.Diffuse.r = 1.0f;
	DefaultMat.Diffuse.g = 1.0f;
	DefaultMat.Diffuse.b = 1.0f;
	DefaultMat.Diffuse.a = 1.0f;
	DefaultMat.Ambient.r = 1.0f;
	DefaultMat.Ambient.g = 1.0f;
	DefaultMat.Ambient.b = 1.0f;
	DefaultMat.Ambient.a = 1.0f;
	DefaultMat.Emissive.r = 0.0f;
	DefaultMat.Emissive.g = 0.0f;
	DefaultMat.Emissive.b = 0.0f;
	DefaultMat.Emissive.a = 0.0f;
	DefaultMat.Specular.r = 0.0f;
	DefaultMat.Specular.g = 0.0f;
	DefaultMat.Specular.b = 0.0f;
	DefaultMat.Specular.a = 0.0f;
	DefaultMat.Power = 0.0f;

	SetMaterial(&DefaultMat);

	SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
	SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);
	SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
	SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);

	SetRenderState(D3DRS_LASTPIXEL, FALSE);
	SetRenderState(D3DRS_ALPHAREF, 1);
	SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	SetRenderState(D3DRS_FOGSTART, 0);
	SetRenderState(D3DRS_FOGEND, 0);
	SetRenderState(D3DRS_FOGDENSITY, 0);
	SetRenderState(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
	SetRenderState(D3DRS_AMBIENT, 0x00000000);
	SetRenderState(D3DRS_LOCALVIEWER, FALSE);
	SetRenderState(D3DRS_NORMALIZENORMALS, FALSE);
	SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
	SetRenderState(D3DRS_CLIPPLANEENABLE, 0);
	if (m_lpD3DDev)
		m_lpD3DDev->SetSoftwareVertexProcessing(false);
	SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
	SetRenderState(D3DRS_MULTISAMPLEMASK, 0xFFFFFFFF);
	SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
	SetRenderState(D3DRS_COLORWRITEENABLE, 0xFFFFFFFF);
	SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
#if defined(USE_OPENGL_ES)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
	SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	SetRenderState(D3DRS_FOGENABLE, FALSE);
	SetRenderState(D3DRS_FOGCOLOR, 0xFF000000);
	SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
	SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
	SetRenderState(D3DRS_RANGEFOGENABLE, FALSE);
	SetRenderState(D3DRS_ZENABLE, TRUE);
	SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	SetRenderState(D3DRS_DITHERENABLE, TRUE);
	SetRenderState(D3DRS_STENCILENABLE, FALSE);
	SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	SetRenderState(D3DRS_CLIPPING, TRUE);
	SetRenderState(D3DRS_LIGHTING, FALSE);
	SetRenderState(D3DRS_SPECULARENABLE, FALSE);
	SetRenderState(D3DRS_COLORVERTEX, FALSE);
	SetRenderState(D3DRS_WRAP0, 0);
	SetRenderState(D3DRS_WRAP1, 0);
	SetRenderState(D3DRS_WRAP2, 0);
	SetRenderState(D3DRS_WRAP3, 0);
	SetRenderState(D3DRS_WRAP4, 0);
	SetRenderState(D3DRS_WRAP5, 0);
	SetRenderState(D3DRS_WRAP6, 0);
	SetRenderState(D3DRS_WRAP7, 0);

	SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
	SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
	SetTextureStageState(2, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	SetTextureStageState(2, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	SetTextureStageState(2, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	SetTextureStageState(2, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	SetTextureStageState(3, D3DTSS_COLOROP, D3DTOP_DISABLE);
	SetTextureStageState(3, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	SetTextureStageState(3, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	SetTextureStageState(3, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	SetTextureStageState(3, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	SetTextureStageState(3, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	SetTextureStageState(4, D3DTSS_COLOROP, D3DTOP_DISABLE);
	SetTextureStageState(4, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	SetTextureStageState(4, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	SetTextureStageState(4, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	SetTextureStageState(4, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	SetTextureStageState(4, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	SetTextureStageState(5, D3DTSS_COLOROP, D3DTOP_DISABLE);
	SetTextureStageState(5, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	SetTextureStageState(5, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	SetTextureStageState(5, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	SetTextureStageState(5, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	SetTextureStageState(5, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	SetTextureStageState(6, D3DTSS_COLOROP, D3DTOP_DISABLE);
	SetTextureStageState(6, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	SetTextureStageState(6, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	SetTextureStageState(6, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	SetTextureStageState(6, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	SetTextureStageState(6, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	SetTextureStageState(7, D3DTSS_COLOROP, D3DTOP_DISABLE);
	SetTextureStageState(7, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	SetTextureStageState(7, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	SetTextureStageState(7, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	SetTextureStageState(7, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	SetTextureStageState(7, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
	SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 2);
	SetTextureStageState(3, D3DTSS_TEXCOORDINDEX, 3);
	SetTextureStageState(4, D3DTSS_TEXCOORDINDEX, 4);
	SetTextureStageState(5, D3DTSS_TEXCOORDINDEX, 5);
	SetTextureStageState(6, D3DTSS_TEXCOORDINDEX, 6);
	SetTextureStageState(7, D3DTSS_TEXCOORDINDEX, 7);

	for (DWORD i = 0; i < 8; ++i)
	{
		SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);
		SetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		SetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		SetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
		SetTexture(i, nullptr);
	}

	SetVertexShader(nullptr);
	SetVertexDeclaration(nullptr);
	SetPixelShader(0);
	SetFVF(D3DFVF_XYZ);

	D3DXVECTOR4 av4Null[STATEMANAGER_MAX_VCONSTANTS]{};
	SetVertexShaderConstant(0, av4Null, STATEMANAGER_MAX_VCONSTANTS);
	SetPixelShaderConstant(0, av4Null, STATEMANAGER_MAX_PCONSTANTS);

	m_bForce = false;

#ifdef _DEBUG
	int i, j;
	for (i = 0; i < STATEMANAGER_MAX_RENDERSTATES; i++)
		m_bRenderStateSavingFlag[i] = FALSE;

	for (j = 0; j < STATEMANAGER_MAX_TRANSFORMSTATES; j++)
		m_bTransformSavingFlag[j] = FALSE;

	for (j = 0; j < STATEMANAGER_MAX_STAGES; ++j)
		for (i = 0; i < STATEMANAGER_MAX_TEXTURESTATES; ++i)
			m_bTextureStageStateSavingFlag[j][i] = FALSE;
	for (j = 0; j < STATEMANAGER_MAX_STAGES; ++j)
		for (i = 0; i < STATEMANAGER_MAX_SAMPLERSTATES; ++i)
			m_bSamplerStateSavingFlag[j][i] = FALSE;
#endif _DEBUG
}

// Material
void CStateManager::SaveMaterial()
{
	m_CopyState.m_D3DMaterial = m_CurrentState.m_D3DMaterial;
}

void CStateManager::SaveMaterial(const D3DMATERIAL9 * pMaterial)
{
	// Check that we have set this up before, if not, the default is this.
	m_CopyState.m_D3DMaterial = m_CurrentState.m_D3DMaterial;
	SetMaterial(pMaterial);
}

void CStateManager::RestoreMaterial()
{
	SetMaterial(&m_CopyState.m_D3DMaterial);
}

void CStateManager::SetMaterial(const D3DMATERIAL9 * pMaterial)
{
	if (m_lpD3DDev) m_lpD3DDev->SetMaterial(pMaterial);
	m_CurrentState.m_D3DMaterial = *pMaterial;
}

void CStateManager::GetMaterial(D3DMATERIAL9 * pMaterial) const
{
	// Set the renderstate and remember it.
	*pMaterial = m_CurrentState.m_D3DMaterial;
}

// Renderstates
DWORD CStateManager::GetRenderState(D3DRENDERSTATETYPE Type) const
{
	return m_CurrentState.m_RenderStates[Type];
}

void CStateManager::SaveRenderState(D3DRENDERSTATETYPE Type, DWORD dwValue)
{
#ifdef _DEBUG
	if (m_bRenderStateSavingFlag[Type])
	{
		Tracef(" CStateManager::SaveRenderState - This render state is already saved [%d, %d]\n", Type, dwValue);
		StateManager_Assert(!" This render state is already saved!");
	}
	m_bRenderStateSavingFlag[Type] = TRUE;
#endif _DEBUG

	// Check that we have set this up before, if not, the default is this.
	m_CopyState.m_RenderStates[Type] = m_CurrentState.m_RenderStates[Type];
	SetRenderState(Type, dwValue);
}

void CStateManager::RestoreRenderState(D3DRENDERSTATETYPE Type)
{
#ifdef _DEBUG
	if (!m_bRenderStateSavingFlag[Type])
	{
		Tracef(" CStateManager::SaveRenderState - This render state was not saved [%d, %d]\n", Type);
		StateManager_Assert(!" This render state was not saved!");
	}
	m_bRenderStateSavingFlag[Type] = FALSE;
#endif _DEBUG

	SetRenderState(Type, m_CopyState.m_RenderStates[Type]);
}

#if defined(USE_OPENGL_ES)
static void ApplyCullMode(DWORD cullMode)
{
	if (cullMode == D3DCULL_NONE)
	{
		glDisable(GL_CULL_FACE);
	}
	else
	{
		glEnable(GL_CULL_FACE);
		if (cullMode == D3DCULL_CW)
			glCullFace(GL_BACK);
		else
			glCullFace(GL_FRONT);
	}
}
#endif

void CStateManager::SetRenderState(D3DRENDERSTATETYPE Type, DWORD Value)
{
	if (m_CurrentState.m_RenderStates[Type] == Value)
		return;

#if defined(USE_OPENGL_ES)
	switch (Type)
	{
		case D3DRS_ALPHABLENDENABLE:
			if (Value) glEnable(GL_BLEND); else glDisable(GL_BLEND);
			break;
		case D3DRS_ZENABLE:
			if (Value) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
			break;
		case D3DRS_ZWRITEENABLE:
			glDepthMask(Value ? GL_TRUE : GL_FALSE);
			break;
		case D3DRS_CULLMODE:
			ApplyCullMode(Value);
			break;
	}
#endif

	if (m_lpD3DDev)
		m_lpD3DDev->SetRenderState(Type, Value);
	m_CurrentState.m_RenderStates[Type] = Value;
}

void CStateManager::GetRenderState(D3DRENDERSTATETYPE Type, DWORD * pdwValue) const
{
	*pdwValue = m_CurrentState.m_RenderStates[Type];
}

// Textures
void CStateManager::SaveTexture(DWORD dwStage, LPDIRECT3DBASETEXTURE9 pTexture)
{
	// Check that we have set this up before, if not, the default is this.
	m_CopyState.m_Textures[dwStage] = m_CurrentState.m_Textures[dwStage];
	SetTexture(dwStage, pTexture);
}

void CStateManager::RestoreTexture(DWORD dwStage)
{
	SetTexture(dwStage, m_CopyState.m_Textures[dwStage]);
}

void CStateManager::SetTexture(DWORD dwStage, LPDIRECT3DBASETEXTURE9 pTexture)
{
#if defined(USE_OPENGL_ES)
	GLuint glTex = (GLuint)(uintptr_t)pTexture;
	glActiveTexture(GL_TEXTURE0 + dwStage);
	glBindTexture(GL_TEXTURE_2D, glTex);
	m_CurrentState.m_Textures[dwStage] = pTexture;
	return;
#else
	if (pTexture == m_CurrentState.m_Textures[dwStage])
		return;

	if (m_lpD3DDev) m_lpD3DDev->SetTexture(dwStage, pTexture);
	m_CurrentState.m_Textures[dwStage] = pTexture;
#endif
}

void CStateManager::GetTexture(DWORD dwStage, LPDIRECT3DBASETEXTURE9 * ppTexture) const
{
	*ppTexture = m_CurrentState.m_Textures[dwStage];
}

// Texture stage states
void CStateManager::SaveTextureStageState(DWORD dwStage,D3DTEXTURESTAGESTATETYPE Type, DWORD dwValue)
{
	// Check that we have set this up before, if not, the default is this.
#ifdef _DEBUG
	if (m_bTextureStageStateSavingFlag[dwStage][Type])
	{
		Tracef(" CStateManager::SaveTextureStageState - This texture stage state is already saved [%d, %d]\n", dwStage, Type);
		StateManager_Assert(!" This texture stage state is already saved!");
	}
	m_bTextureStageStateSavingFlag[dwStage][Type] = TRUE;
#endif _DEBUG
	m_CopyState.m_TextureStates[dwStage][Type] = m_CurrentState.m_TextureStates[dwStage][Type];
	SetTextureStageState(dwStage, Type, dwValue);
}

void CStateManager::RestoreTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE Type)
{
#ifdef _DEBUG
	if (!m_bTextureStageStateSavingFlag[dwStage][Type])
	{
		Tracef(" CStateManager::RestoreTextureStageState - This texture stage state was not saved [%d, %d]\n", dwStage, Type);
		StateManager_Assert(!" This texture stage state was not saved!");
	}
	m_bTextureStageStateSavingFlag[dwStage][Type] = FALSE;
#endif _DEBUG
	SetTextureStageState(dwStage, Type, m_CopyState.m_TextureStates[dwStage][Type]);
}

void CStateManager::SetTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE Type, DWORD dwValue)
{
	if (m_CurrentState.m_TextureStates[dwStage][Type] == dwValue)
		return;

	if (m_lpD3DDev) m_lpD3DDev->SetTextureStageState(dwStage, Type, dwValue);
	m_CurrentState.m_TextureStates[dwStage][Type] = dwValue;
}

void CStateManager::GetTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE Type, DWORD * pdwValue) const
{
	*pdwValue = m_CurrentState.m_TextureStates[dwStage][Type];
}

////Converting2DirectX9
void CStateManager::SaveSamplerState(DWORD dwStage, D3DSAMPLERSTATETYPE Type, DWORD dwValue)
{
	// Check that we have set this up before, if not, the default is this.
#ifdef _DEBUG
	if (m_bSamplerStateSavingFlag[dwStage][Type])
	{
	}
	m_bSamplerStateSavingFlag[dwStage][Type] = TRUE;
#endif _DEBUG
	m_CopyState.m_SamplerStates[dwStage][Type] = m_CurrentState.m_SamplerStates[dwStage][Type];
	SetSamplerState(dwStage, Type, dwValue);
}

void CStateManager::RestoreSamplerState(DWORD dwStage, D3DSAMPLERSTATETYPE Type)
{
#ifdef _DEBUG
	if (!m_bSamplerStateSavingFlag[dwStage][Type])
	{
	}
	m_bSamplerStateSavingFlag[dwStage][Type] = FALSE;
#endif _DEBUG
	SetSamplerState(dwStage, Type, m_CopyState.m_SamplerStates[dwStage][Type]);
}

void CStateManager::SetSamplerState(DWORD dwStage, D3DSAMPLERSTATETYPE Type, DWORD dwValue)
{
	if (m_CurrentState.m_SamplerStates[dwStage][Type] == dwValue)
		return;

#if !defined(USE_OPENGL_ES)
	if (m_lpD3DDev) m_lpD3DDev->SetSamplerState(dwStage, Type, dwValue);
#endif

	m_CurrentState.m_SamplerStates[dwStage][Type] = dwValue;
}

void CStateManager::GetSamplerState(DWORD dwStage, D3DSAMPLERSTATETYPE Type, DWORD* pdwValue)
{
	*pdwValue = m_CurrentState.m_SamplerStates[dwStage][Type];
}

// Vertex Shader
void CStateManager::SaveVertexShader(LPDIRECT3DVERTEXSHADER9 dwShader)
{
	m_CopyState.m_dwVertexShader = m_CurrentState.m_dwVertexShader;
	SetVertexShader(dwShader);
}

void CStateManager::RestoreVertexShader()
{
	SetVertexShader(m_CopyState.m_dwVertexShader);
}

void CStateManager::SetVertexShader(LPDIRECT3DVERTEXSHADER9 dwShader)
{
	if (m_CurrentState.m_dwVertexShader != dwShader)
	{
		m_CurrentState.m_dwVertexShader = dwShader;
		if (m_lpD3DDev) m_lpD3DDev->SetVertexShader(dwShader);
	}
}

void CStateManager::GetVertexShader(LPDIRECT3DVERTEXSHADER9 * pdwShader) const
{
	*pdwShader = m_CurrentState.m_dwVertexShader;
}

// Vertex Declaration
void CStateManager::SaveVertexDeclaration(LPDIRECT3DVERTEXDECLARATION9 dwShader)
{
	m_CopyState.m_dwVertexDeclaration = m_CurrentState.m_dwVertexDeclaration;
	SetVertexDeclaration(dwShader);
}

void CStateManager::RestoreVertexDeclaration()
{
	SetVertexDeclaration(m_CopyState.m_dwVertexDeclaration);
}

void CStateManager::SetVertexDeclaration(LPDIRECT3DVERTEXDECLARATION9 dwShader)
{
	if (m_lpD3DDev) m_lpD3DDev->SetVertexDeclaration(dwShader);
	m_CurrentState.m_dwVertexDeclaration = dwShader;
	m_CurrentState.m_dwFVF = 0;
}

void CStateManager::GetVertexDeclaration(LPDIRECT3DVERTEXDECLARATION9* pdwShader)
{
	*pdwShader = m_CurrentState.m_dwVertexDeclaration;
}

// Pixel Shader
void CStateManager::SavePixelShader(LPDIRECT3DPIXELSHADER9 dwShader)
{
	m_CopyState.m_dwPixelShader = m_CurrentState.m_dwPixelShader;
	SetPixelShader(dwShader);
}

void CStateManager::RestorePixelShader()
{
	SetPixelShader(m_CopyState.m_dwPixelShader);
}

void CStateManager::SetPixelShader(LPDIRECT3DPIXELSHADER9 dwShader)
{
	if (m_CurrentState.m_dwPixelShader != dwShader)
	{
		m_CurrentState.m_dwPixelShader = dwShader;
		if (m_lpD3DDev) m_lpD3DDev->SetPixelShader(dwShader);
	}
}

void CStateManager::GetPixelShader(LPDIRECT3DPIXELSHADER9 * pdwShader) const
{
	*pdwShader = m_CurrentState.m_dwPixelShader;
}

// FVF
void CStateManager::SaveFVF(DWORD dwFVF)
{
	m_CopyState.m_dwFVF = m_CurrentState.m_dwFVF;
	SetFVF(dwFVF);
}

void CStateManager::RestoreFVF()
{
	SetFVF(m_CopyState.m_dwFVF);
}

void CStateManager::SetFVF(DWORD dwFVF)
{
	if (m_lpD3DDev) m_lpD3DDev->SetFVF(dwFVF);
	m_CurrentState.m_dwFVF = dwFVF;
	m_CurrentState.m_dwVertexDeclaration = nullptr;
}

void CStateManager::GetFVF(DWORD* dwFVF)
{
	*dwFVF = m_CurrentState.m_dwFVF;
}

// *** These states are cached, but not protected from multiple sends of the same value.
// Transform
void CStateManager::SaveTransform(D3DTRANSFORMSTATETYPE Type, const D3DMATRIX* pMatrix)
{
#ifdef _DEBUG
	if (m_bTransformSavingFlag[Type])
	{
		Tracef(" CStateManager::SaveTransform - This transform is already saved [%d]\n", Type);
		StateManager_Assert(!" This trasform is already saved!");
	}
	m_bTransformSavingFlag[Type] = TRUE;
#endif _DEBUG

	m_CopyState.m_Matrices[Type] = m_CurrentState.m_Matrices[Type];
	SetTransform(Type, (D3DXMATRIX *)pMatrix);
}

void CStateManager::RestoreTransform(D3DTRANSFORMSTATETYPE Type)
{
#ifdef _DEBUG
	if (!m_bTransformSavingFlag[Type])
	{
		Tracef(" CStateManager::RestoreTransform - This transform was not saved [%d]\n", Type);
		StateManager_Assert(!" This render state was not saved!");
	}
	m_bTransformSavingFlag[Type] = FALSE;
#endif _DEBUG

	SetTransform(Type, &m_CopyState.m_Matrices[Type]);
}

// Don't cache-check the transform.  To much to do
void CStateManager::SetTransform(D3DTRANSFORMSTATETYPE Type, const D3DMATRIX* pMatrix)
{
	if (m_bScene)
	{
		if (m_lpD3DDev) m_lpD3DDev->SetTransform(Type, pMatrix);
	}
	else
	{
		assert(D3DTS_VIEW==Type || D3DTS_PROJECTION==Type || D3DTS_WORLD==Type);
	}

	m_CurrentState.m_Matrices[Type] = *pMatrix;
}

void CStateManager::GetTransform(D3DTRANSFORMSTATETYPE Type, D3DMATRIX * pMatrix) const
{
	*pMatrix = m_CurrentState.m_Matrices[Type];
}

// SetVertexShaderConstant
void CStateManager::SaveVertexShaderConstant(DWORD dwRegister,CONST void* pConstantData,DWORD dwConstantCount)
{
	DWORD i;

	for (i = 0; i < dwConstantCount; i++)
	{
		StateManager_Assert((dwRegister + i) < STATEMANAGER_MAX_VCONSTANTS);
		m_CopyState.m_VertexShaderConstants[dwRegister + i] = m_CurrentState.m_VertexShaderConstants[dwRegister + i];
	}

	SetVertexShaderConstant(dwRegister, pConstantData, dwConstantCount);
}

void CStateManager::RestoreVertexShaderConstant(DWORD dwRegister, DWORD dwConstantCount)
{
	SetVertexShaderConstant(dwRegister, &m_CopyState.m_VertexShaderConstants[dwRegister], dwConstantCount);
}

void CStateManager::SetVertexShaderConstant(DWORD dwRegister,CONST void* pConstantData,DWORD dwConstantCount)
{
	if (m_lpD3DDev)
		m_lpD3DDev->SetVertexShaderConstantF(dwRegister, (const float*)pConstantData, dwConstantCount);  // Converting2DirectX9

	// Set the renderstate and remember it.
	for (DWORD i = 0; i < dwConstantCount; i++)
	{
		StateManager_Assert((dwRegister + i) < STATEMANAGER_MAX_VCONSTANTS);
		m_CurrentState.m_VertexShaderConstants[dwRegister + i] = *(((D3DXVECTOR4*)pConstantData) + i);
	}
}

// SetPixelShaderConstant
void CStateManager::SavePixelShaderConstant(DWORD dwRegister,CONST void* pConstantData,DWORD dwConstantCount)
{
	DWORD i;

	for (i = 0; i < dwConstantCount; i++)
	{
		StateManager_Assert((dwRegister + i) < STATEMANAGER_MAX_VCONSTANTS);
		m_CopyState.m_PixelShaderConstants[dwRegister + i] = *(((D3DXVECTOR4*)pConstantData) + i);
	}

	SetPixelShaderConstant(dwRegister, pConstantData, dwConstantCount);
}

void CStateManager::RestorePixelShaderConstant(DWORD dwRegister, DWORD dwConstantCount)
{
	SetPixelShaderConstant(dwRegister, &m_CopyState.m_PixelShaderConstants[dwRegister], dwConstantCount);
}

void CStateManager::SetPixelShaderConstant(DWORD dwRegister,CONST void* pConstantData,DWORD dwConstantCount)
{
	if (m_lpD3DDev)
		m_lpD3DDev->SetPixelShaderConstantF(dwRegister, (const float*)pConstantData, dwConstantCount);  // Converting2DirectX9

	// Set the renderstate and remember it.
	for (DWORD i = 0; i < dwConstantCount; i++)
	{
		StateManager_Assert((dwRegister + i) < STATEMANAGER_MAX_VCONSTANTS);
		m_CurrentState.m_PixelShaderConstants[dwRegister + i] = *(((D3DXVECTOR4*)pConstantData) + i);
	}
}

void CStateManager::SaveStreamSource(UINT StreamNumber, LPDIRECT3DVERTEXBUFFER9 pStreamData,UINT Stride)
{
	// Check that we have set this up before, if not, the default is this.
	m_CopyState.m_StreamData[StreamNumber] = m_CurrentState.m_StreamData[StreamNumber];
	SetStreamSource(StreamNumber, pStreamData, Stride);
}

void CStateManager::RestoreStreamSource(UINT StreamNumber)
{
	SetStreamSource(StreamNumber,
					m_CopyState.m_StreamData[StreamNumber].m_lpStreamData,
					m_CopyState.m_StreamData[StreamNumber].m_Stride);
}

void CStateManager::SetStreamSource(UINT StreamNumber, LPDIRECT3DVERTEXBUFFER9 pStreamData, UINT Stride)
{
	const CStreamData kStreamData(pStreamData, Stride);
	if (m_CurrentState.m_StreamData[StreamNumber] == kStreamData)
		return;

#if defined(USE_OPENGL_ES)
	GLuint vbo = (GLuint)(uintptr_t)pStreamData;
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
#else
	if (m_lpD3DDev) m_lpD3DDev->SetStreamSource(StreamNumber, pStreamData, 0, Stride);
#endif
	m_CurrentState.m_StreamData[StreamNumber] = kStreamData;
}

void CStateManager::SaveIndices(LPDIRECT3DINDEXBUFFER9 pIndexData, UINT BaseVertexIndex)
{
	m_CopyState.m_IndexData = m_CurrentState.m_IndexData;
	SetIndices(pIndexData, BaseVertexIndex);
}

void CStateManager::RestoreIndices()
{
	SetIndices(m_CopyState.m_IndexData.m_lpIndexData, m_CopyState.m_IndexData.m_BaseVertexIndex);
}

void CStateManager::SetIndices(LPDIRECT3DINDEXBUFFER9 pIndexData, UINT BaseVertexIndex)
{
	const CIndexData kIndexData(pIndexData, BaseVertexIndex);

	if (m_CurrentState.m_IndexData == kIndexData)
		return;

#if defined(USE_OPENGL_ES)
	GLuint ebo = (GLuint)(uintptr_t)pIndexData;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
#else
	if (m_lpD3DDev) m_lpD3DDev->SetIndices(pIndexData);
#endif
	m_CurrentState.m_IndexData = kIndexData;
}

#if defined(USE_OPENGL_ES)
bool g_bRenderingCharacterShadow = false;
static float s_fSkySunDir[3] = { 0.0f, -0.5f, 0.5f };
static float s_fSkySunIntensity = 1.0f;
static GLenum D3DToGLBlend(DWORD d3dBlend);

static void SetupMeshShaderAndStates(const CStateManagerState& state)
{
	glUseProgram(CGraphicBase::ms_uMeshShaderProgram);

	if (state.m_RenderStates[D3DRS_ZENABLE] && state.m_RenderStates[D3DRS_ZENABLE] != 0x7FFFFFFF)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}

	if (state.m_RenderStates[D3DRS_ZWRITEENABLE] && state.m_RenderStates[D3DRS_ZWRITEENABLE] != 0x7FFFFFFF)
		glDepthMask(GL_TRUE);
	else
		glDepthMask(GL_FALSE);

	ApplyCullMode(state.m_RenderStates[D3DRS_CULLMODE]);

	bool isShadowPass = (state.m_RenderStates[D3DRS_SRCBLEND] == D3DBLEND_ZERO && state.m_RenderStates[D3DRS_DESTBLEND] == D3DBLEND_SRCCOLOR);
	bool isPCBlockerPass = (!isShadowPass &&
		state.m_RenderStates[D3DRS_ALPHABLENDENABLE] &&
		state.m_RenderStates[D3DRS_ALPHABLENDENABLE] != 0x7FFFFFFF &&
		state.m_Textures[1] != nullptr &&
		state.m_TextureStates[1][D3DTSS_TEXCOORDINDEX] == D3DTSS_TCI_CAMERASPACEPOSITION &&
		state.m_TextureStates[1][D3DTSS_ALPHAOP] != D3DTOP_DISABLE);

	if (CGraphicBase::ms_iLocMeshIsShadowPass != -1)
		glUniform1i(CGraphicBase::ms_iLocMeshIsShadowPass, isShadowPass ? 1 : 0);
	if (CGraphicBase::ms_iLocMeshIsPCBlockerPass != -1)
		glUniform1i(CGraphicBase::ms_iLocMeshIsPCBlockerPass, isPCBlockerPass ? 1 : 0);

	if (isShadowPass)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ZERO, GL_SRC_COLOR);
		glDepthMask(GL_FALSE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(-1.0f, -1.0f);
	}
	else if (isPCBlockerPass)
	{
		glDisable(GL_POLYGON_OFFSET_FILL);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	}
	else
	{
		glDisable(GL_POLYGON_OFFSET_FILL);
		if (state.m_RenderStates[D3DRS_ALPHABLENDENABLE] && state.m_RenderStates[D3DRS_ALPHABLENDENABLE] != 0x7FFFFFFF)
		{
			glEnable(GL_BLEND);
			GLenum glSrc = D3DToGLBlend(state.m_RenderStates[D3DRS_SRCBLEND]);
			GLenum glDst = D3DToGLBlend(state.m_RenderStates[D3DRS_DESTBLEND]);
			glBlendFunc(glSrc, glDst);
		}
		else
		{
			glDisable(GL_BLEND);
		}
	}

	// Calculate WVP matrix
	static D3DXMATRIX s_matWVP;
	D3DXMatrixMultiply(&s_matWVP, &state.m_Matrices[D3DTS_WORLD], &state.m_Matrices[D3DTS_VIEW]);
	D3DXMatrixMultiply(&s_matWVP, &s_matWVP, &state.m_Matrices[D3DTS_PROJECTION]);
	glUniformMatrix4fv(CGraphicBase::ms_iLocMeshWVP, 1, GL_FALSE, (const float*)&s_matWVP);
	glUniformMatrix4fv(CGraphicBase::ms_iLocMeshWorld, 1, GL_FALSE, (const float*)&state.m_Matrices[D3DTS_WORLD]);

	if (CGraphicBase::ms_iLocMeshView != -1)
		glUniformMatrix4fv(CGraphicBase::ms_iLocMeshView, 1, GL_FALSE, (const float*)&state.m_Matrices[D3DTS_VIEW]);
	if (CGraphicBase::ms_iLocMeshTexMat1 != -1)
		glUniformMatrix4fv(CGraphicBase::ms_iLocMeshTexMat1, 1, GL_FALSE, (const float*)&state.m_Matrices[D3DTS_TEXTURE1]);

	int alphaTest = (state.m_RenderStates[D3DRS_ALPHATESTENABLE] && state.m_RenderStates[D3DRS_ALPHATESTENABLE] != 0x7FFFFFFF) ? 1 : 0;
	float alphaRef = 0.5f;
	if (state.m_RenderStates[D3DRS_ALPHAREF] != 0x7FFFFFFF && state.m_RenderStates[D3DRS_ALPHAREF] > 0)
		alphaRef = float(state.m_RenderStates[D3DRS_ALPHAREF] & 0xFF) / 255.0f;
	glUniform1i(CGraphicBase::ms_iLocMeshAlphaTest, alphaTest);
	glUniform1f(CGraphicBase::ms_iLocMeshAlphaRef, alphaRef);

	if (g_bRenderingCharacterShadow)
	{
		glDisable(GL_BLEND);
		glUniform3f(CGraphicBase::ms_iLocMeshLightDir, 0.0f, 0.0f, 1.0f);
		glUniform3f(CGraphicBase::ms_iLocMeshLightColor, 0.0f, 0.0f, 0.0f);
		glUniform3f(CGraphicBase::ms_iLocMeshAmbientColor, 0.5f, 0.5f, 0.5f);
		glUniform1i(CGraphicBase::ms_iLocMeshFogEnable, 0);
	}
	else
	{
		// Lighting parameters:
		// Directional light from front-top-right
		glUniform3f(CGraphicBase::ms_iLocMeshLightDir, 0.4f, 0.7f, 0.5f);
		glUniform3f(CGraphicBase::ms_iLocMeshLightColor, 0.9f, 0.9f, 0.9f);
		glUniform3f(CGraphicBase::ms_iLocMeshAmbientColor, 0.55f, 0.55f, 0.55f);

		// Fog
		int meshFogEnable = (state.m_RenderStates[D3DRS_FOGENABLE] && state.m_RenderStates[D3DRS_FOGENABLE] != 0x7FFFFFFF) ? 1 : 0;
		glUniform1i(CGraphicBase::ms_iLocMeshFogEnable, meshFogEnable);

		DWORD dwMeshFogColor = (state.m_RenderStates[D3DRS_FOGCOLOR] != 0x7FFFFFFF) ? state.m_RenderStates[D3DRS_FOGCOLOR] : 0xFFFFFFFF;
		float meshFogR = float((dwMeshFogColor >> 16) & 0xFF) / 255.0f;
		float meshFogG = float((dwMeshFogColor >> 8) & 0xFF) / 255.0f;
		float meshFogB = float(dwMeshFogColor & 0xFF) / 255.0f;
		float meshFogA = float((dwMeshFogColor >> 24) & 0xFF) / 255.0f;
		if (meshFogA < 0.001f) meshFogA = 1.0f;
		glUniform4f(CGraphicBase::ms_iLocMeshFogColor, meshFogR, meshFogG, meshFogB, meshFogA);

		float meshFogStart = 3500.0f;
		float meshFogEnd = 18500.0f;
		if (state.m_RenderStates[D3DRS_FOGSTART] != 0x7FFFFFFF && state.m_RenderStates[D3DRS_FOGSTART] != 0)
		{
			float fs = *((const float*)&state.m_RenderStates[D3DRS_FOGSTART]);
			if (fs > 10.0f && fs < 200000.0f)
				meshFogStart = fs;
		}
		if (state.m_RenderStates[D3DRS_FOGEND] != 0x7FFFFFFF && state.m_RenderStates[D3DRS_FOGEND] != 0)
		{
			float fe = *((const float*)&state.m_RenderStates[D3DRS_FOGEND]);
			if (fe > meshFogStart && fe < 200000.0f)
				meshFogEnd = fe;
		}
		glUniform2f(CGraphicBase::ms_iLocMeshFogRange, meshFogStart, meshFogEnd);

		CCamera* pCam = CCameraManager::Instance().GetCurrentCamera();
		D3DXVECTOR3 camPos(0.0f, 0.0f, 0.0f);
		if (pCam)
		{
			camPos = pCam->GetEye();
		}
		glUniform3f(CGraphicBase::ms_iLocMeshCameraPos, camPos.x, camPos.y, camPos.z);
		glUniform3f(CGraphicBase::ms_iLocMeshSunDir, s_fSkySunDir[0], s_fSkySunDir[1], s_fSkySunDir[2]);
	}

	if (CGraphicBase::ms_iLocMeshIsDungeonMap != -1)
		glUniform1i(CGraphicBase::ms_iLocMeshIsDungeonMap, CGraphicBase::IsDungeonMapMode() ? 1 : 0);

	// Texture stage 0
	int useTex = (state.m_Textures[0] != nullptr) ? 1 : 0;
	glUniform1i(CGraphicBase::ms_iLocMeshUseTexture, useTex);
	glUniform1i(CGraphicBase::ms_iLocMeshTexture, 0);

	if (useTex)
	{
		static GLuint s_lastMeshTex = 0;
		GLuint tex0 = (GLuint)(uintptr_t)state.m_Textures[0];
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex0);
		if (s_lastMeshTex != tex0)
		{
			s_lastMeshTex = tex0;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
	}

	// Texture stage 1 (Lightmap for TPNT2Vertex OR Dynamic Shadow map for isShadowPass)
	UINT stride = state.m_StreamData[0].m_Stride;
	int useTex2 = (!isPCBlockerPass && stride == sizeof(TPNT2Vertex) && state.m_Textures[1] != nullptr) ? 1 : 0;
	glUniform1i(CGraphicBase::ms_iLocMeshUseTexture2, useTex2);
	glUniform1i(CGraphicBase::ms_iLocMeshTexture2, 1);

	if (useTex2 || isPCBlockerPass || (isShadowPass && state.m_Textures[1] != nullptr))
	{
		static GLuint s_lastMeshTex1 = 0;
		GLuint tex1 = (GLuint)(uintptr_t)state.m_Textures[1];
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, tex1);
		if (s_lastMeshTex1 != tex1 || isShadowPass || isPCBlockerPass)
		{
			s_lastMeshTex1 = tex1;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		glActiveTexture(GL_TEXTURE0);
	}
}

static void SetupTerrainShaderAndStates(const CStateManagerState& state)
{
	glUseProgram(CGraphicBase::ms_uTerrainShaderProgram);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_CULL_FACE);

	static D3DXMATRIX s_matWVP;
	D3DXMatrixMultiply(&s_matWVP, &state.m_Matrices[D3DTS_WORLD], &state.m_Matrices[D3DTS_VIEW]);
	D3DXMatrixMultiply(&s_matWVP, &s_matWVP, &state.m_Matrices[D3DTS_PROJECTION]);
	glUniformMatrix4fv(CGraphicBase::ms_iLocTerrainWVP, 1, GL_FALSE, (const float*)&s_matWVP);
	glUniformMatrix4fv(CGraphicBase::ms_iLocTerrainView, 1, GL_FALSE, (const float*)&state.m_Matrices[D3DTS_VIEW]);
	glUniformMatrix4fv(CGraphicBase::ms_iLocTerrainWorld, 1, GL_FALSE, (const float*)&state.m_Matrices[D3DTS_WORLD]);
	glUniformMatrix4fv(CGraphicBase::ms_iLocTerrainTexMat0, 1, GL_FALSE, (const float*)&state.m_Matrices[D3DTS_TEXTURE0]);
	glUniformMatrix4fv(CGraphicBase::ms_iLocTerrainTexMat1, 1, GL_FALSE, (const float*)&state.m_Matrices[D3DTS_TEXTURE1]);

	int useTile = (state.m_Textures[0] != nullptr) ? 1 : 0;
	glUniform1i(CGraphicBase::ms_iLocTerrainUseTileTexture, useTile);

	static bool s_terrainSamplerInit = false;
	if (!s_terrainSamplerInit)
	{
		glUniform1i(CGraphicBase::ms_iLocTerrainTileTexture, 0);
		glUniform1i(CGraphicBase::ms_iLocTerrainSplatTexture, 1);
		s_terrainSamplerInit = true;
	}

	bool isShadowPass = (state.m_RenderStates[D3DRS_SRCBLEND] == D3DBLEND_ZERO && state.m_RenderStates[D3DRS_DESTBLEND] == D3DBLEND_SRCCOLOR);
	glUniform1i(CGraphicBase::ms_iLocTerrainIsShadowPass, isShadowPass ? 1 : 0);

	if (CGraphicBase::ms_iLocTerrainIsDungeonMap != -1)
		glUniform1i(CGraphicBase::ms_iLocTerrainIsDungeonMap, CGraphicBase::IsDungeonMapMode() ? 1 : 0);

	static GLuint s_lastTex0 = 0;
	static bool s_lastTex0IsShadow = false;
	static GLuint s_lastTex1 = 0;

	if (useTile)
	{
		GLuint tex0 = (GLuint)(uintptr_t)state.m_Textures[0];
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex0);
		if (s_lastTex0 != tex0 || s_lastTex0IsShadow != isShadowPass)
		{
			s_lastTex0 = tex0;
			s_lastTex0IsShadow = isShadowPass;
			if (isShadowPass)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
		}
	}

	bool useSplatAlpha = (state.m_TextureStates[1][D3DTSS_ALPHAOP] != D3DTOP_DISABLE && state.m_Textures[1] != nullptr);
	glUniform1i(CGraphicBase::ms_iLocTerrainUseSplatAlpha, useSplatAlpha ? 1 : 0);
	if (state.m_Textures[1] != nullptr)
	{
		GLuint tex1 = (GLuint)(uintptr_t)state.m_Textures[1];
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, tex1);
		if (s_lastTex1 != tex1)
		{
			s_lastTex1 = tex1;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
	}

	if (isShadowPass)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ZERO, GL_SRC_COLOR);
		glDepthMask(GL_FALSE);
		glUniform1i(CGraphicBase::ms_iLocTerrainUseSplatAlpha, (state.m_Textures[1] != nullptr) ? 1 : 0);
	}
	else if (useSplatAlpha)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE);
	}
	else
	{
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
	}

	static bool s_terrainLightInit = false;
	if (!s_terrainLightInit)
	{
		glUniform3f(CGraphicBase::ms_iLocTerrainLightDir, 0.4f, 0.7f, 0.5f);
		glUniform3f(CGraphicBase::ms_iLocTerrainLightColor, 0.0f, 0.0f, 0.0f);
		glUniform3f(CGraphicBase::ms_iLocTerrainAmbientColor, 1.0f, 1.0f, 1.0f);
		s_terrainLightInit = true;
	}

	// Fog
	int terrainFogEnable = (state.m_RenderStates[D3DRS_FOGENABLE] && state.m_RenderStates[D3DRS_FOGENABLE] != 0x7FFFFFFF) ? 1 : 0;
	glUniform1i(CGraphicBase::ms_iLocTerrainFogEnable, terrainFogEnable);

	DWORD dwTerrainFogColor = (state.m_RenderStates[D3DRS_FOGCOLOR] != 0x7FFFFFFF) ? state.m_RenderStates[D3DRS_FOGCOLOR] : 0xFFFFFFFF;
	float terrainFogR = float((dwTerrainFogColor >> 16) & 0xFF) / 255.0f;
	float terrainFogG = float((dwTerrainFogColor >> 8) & 0xFF) / 255.0f;
	float terrainFogB = float(dwTerrainFogColor & 0xFF) / 255.0f;
	float terrainFogA = float((dwTerrainFogColor >> 24) & 0xFF) / 255.0f;
	if (terrainFogA < 0.001f) terrainFogA = 1.0f;
	glUniform4f(CGraphicBase::ms_iLocTerrainFogColor, terrainFogR, terrainFogG, terrainFogB, terrainFogA);

	float terrainFogStart = 3500.0f;
	float terrainFogEnd = 18500.0f;
	if (state.m_RenderStates[D3DRS_FOGSTART] != 0x7FFFFFFF && state.m_RenderStates[D3DRS_FOGSTART] != 0)
	{
		float fs = *((const float*)&state.m_RenderStates[D3DRS_FOGSTART]);
		if (fs > 10.0f && fs < 200000.0f)
			terrainFogStart = fs;
	}
	if (state.m_RenderStates[D3DRS_FOGEND] != 0x7FFFFFFF && state.m_RenderStates[D3DRS_FOGEND] != 0)
	{
		float fe = *((const float*)&state.m_RenderStates[D3DRS_FOGEND]);
		if (fe > terrainFogStart && fe < 200000.0f)
			terrainFogEnd = fe;
	}
	glUniform2f(CGraphicBase::ms_iLocTerrainFogRange, terrainFogStart, terrainFogEnd);

	CCamera* pCamTerrain = CCameraManager::Instance().GetCurrentCamera();
	D3DXVECTOR3 camPosTerrain(0.0f, 0.0f, 0.0f);
	if (pCamTerrain)
	{
		camPosTerrain = pCamTerrain->GetEye();
	}
	glUniform3f(CGraphicBase::ms_iLocTerrainCameraPos, camPosTerrain.x, camPosTerrain.y, camPosTerrain.z);
	glUniform3f(CGraphicBase::ms_iLocTerrainSunDir, s_fSkySunDir[0], s_fSkySunDir[1], s_fSkySunDir[2]);

	glActiveTexture(GL_TEXTURE0);
}

static GLenum D3DToGLBlend(DWORD d3dBlend)
{
	switch (d3dBlend)
	{
		case D3DBLEND_ZERO: return GL_ZERO;
		case D3DBLEND_ONE: return GL_ONE;
		case D3DBLEND_SRCCOLOR: return GL_SRC_COLOR;
		case D3DBLEND_INVSRCCOLOR: return GL_ONE_MINUS_SRC_COLOR;
		case D3DBLEND_SRCALPHA: return GL_SRC_ALPHA;
		case D3DBLEND_INVSRCALPHA: return GL_ONE_MINUS_SRC_ALPHA;
		case D3DBLEND_DESTALPHA: return GL_DST_ALPHA;
		case D3DBLEND_INVDESTALPHA: return GL_ONE_MINUS_DST_ALPHA;
		case D3DBLEND_DESTCOLOR: return GL_DST_COLOR;
		case D3DBLEND_INVDESTCOLOR: return GL_ONE_MINUS_DST_COLOR;
		default: return GL_SRC_ALPHA;
	}
}

static void SetupParticleShaderAndStates(const CStateManagerState& state, bool hasVertexColor)
{
	glUseProgram(CGraphicBase::ms_uParticleShaderProgram);

	if (state.m_RenderStates[D3DRS_ZENABLE] && state.m_RenderStates[D3DRS_ZENABLE] != 0x7FFFFFFF)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}

	if (state.m_RenderStates[D3DRS_ZWRITEENABLE] && state.m_RenderStates[D3DRS_ZWRITEENABLE] != 0x7FFFFFFF)
		glDepthMask(GL_TRUE);
	else
		glDepthMask(GL_FALSE);

	glDisable(GL_CULL_FACE);

	if (state.m_RenderStates[D3DRS_ALPHABLENDENABLE] && state.m_RenderStates[D3DRS_ALPHABLENDENABLE] != 0x7FFFFFFF)
	{
		glEnable(GL_BLEND);
		GLenum glSrc = D3DToGLBlend(state.m_RenderStates[D3DRS_SRCBLEND]);
		GLenum glDst = D3DToGLBlend(state.m_RenderStates[D3DRS_DESTBLEND]);
		glBlendFunc(glSrc, glDst);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	static D3DXMATRIX s_matWVP;
	D3DXMatrixMultiply(&s_matWVP, &state.m_Matrices[D3DTS_WORLD], &state.m_Matrices[D3DTS_VIEW]);
	D3DXMatrixMultiply(&s_matWVP, &s_matWVP, &state.m_Matrices[D3DTS_PROJECTION]);
	glUniformMatrix4fv(CGraphicBase::ms_iLocParticleWVP, 1, GL_FALSE, (const float*)&s_matWVP);

	DWORD tfactor = state.m_RenderStates[D3DRS_TEXTUREFACTOR];
	if (tfactor == 0 || tfactor == 0x7FFFFFFF)
		tfactor = 0xFFFFFFFF;
	float a = float((tfactor >> 24) & 0xFF) / 255.0f;
	float r = float((tfactor >> 16) & 0xFF) / 255.0f;
	float g = float((tfactor >> 8) & 0xFF) / 255.0f;
	float b = float(tfactor & 0xFF) / 255.0f;
	glUniform4f(CGraphicBase::ms_iLocParticleColor, r, g, b, a);
	glUniform1i(CGraphicBase::ms_iLocParticleUseVertexColor, hasVertexColor ? 1 : 0);

	DWORD colorOp = state.m_TextureStates[0][D3DTSS_COLOROP];
	glUniform1i(CGraphicBase::ms_iLocParticleColorOp, (int)colorOp);

	int useTex = (state.m_Textures[0] != nullptr && colorOp != D3DTOP_DISABLE) ? 1 : 0;
	glUniform1i(CGraphicBase::ms_iLocParticleUseTexture, useTex);
	glUniform1i(CGraphicBase::ms_iLocParticleTexture, 0);
	if (useTex)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, (GLuint)(uintptr_t)state.m_Textures[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
}

#if defined(USE_OPENGL_ES)
// Static data set by CSkyBox before rendering
static float s_fSkyGradientColors[16 * 4] = {};
static int s_iSkyGradientCount = 0;
static int s_iSkyMode = 0; // 0=gradient, 1=texture, 2=cloud

void CStateManager::SetSkyGradientData(const float* pColors, int count)
{
	s_iSkyGradientCount = (count > 16) ? 16 : count;
	if (pColors && s_iSkyGradientCount > 0)
		memcpy(s_fSkyGradientColors, pColors, s_iSkyGradientCount * 4 * sizeof(float));
}

void CStateManager::SetSkySunData(float dx, float dy, float dz, float intensity)
{
	s_fSkySunDir[0] = dx;
	s_fSkySunDir[1] = dy;
	s_fSkySunDir[2] = dz;
	s_fSkySunIntensity = intensity;
}

void CStateManager::SetSkyMode(int mode)
{
	s_iSkyMode = mode;
}

static void SetupSkyShaderAndStates(const CStateManagerState& state)
{
	glUseProgram(CGraphicBase::ms_uSkyShaderProgram);

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);

	bool isCloud = CGraphicBase::IsSkyCloudMode();
	if (isCloud || (state.m_RenderStates[D3DRS_ALPHABLENDENABLE] && state.m_RenderStates[D3DRS_ALPHABLENDENABLE] != 0x7FFFFFFF))
	{
		glEnable(GL_BLEND);
		GLenum glSrc = D3DToGLBlend(state.m_RenderStates[D3DRS_SRCBLEND]);
		GLenum glDst = D3DToGLBlend(state.m_RenderStates[D3DRS_DESTBLEND]);
		glBlendFunc(glSrc, glDst);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	static D3DXMATRIX s_matWVP;
	D3DXMatrixMultiply(&s_matWVP, &state.m_Matrices[D3DTS_WORLD], &state.m_Matrices[D3DTS_VIEW]);
	D3DXMatrixMultiply(&s_matWVP, &s_matWVP, &state.m_Matrices[D3DTS_PROJECTION]);
	glUniformMatrix4fv(CGraphicBase::ms_iLocSkyWVP, 1, GL_FALSE, (const float*)&s_matWVP);
	glUniformMatrix4fv(CGraphicBase::ms_iLocSkyWorld, 1, GL_FALSE, (const float*)&state.m_Matrices[D3DTS_WORLD]);
	glUniformMatrix4fv(CGraphicBase::ms_iLocSkyTexMat0, 1, GL_FALSE, (const float*)&state.m_Matrices[D3DTS_TEXTURE0]);

	glUniform1i(CGraphicBase::ms_iLocSkyIsCloud, isCloud ? 1 : 0);

	float curTime = CTimer::Instance().GetCurrentMillisecond() * 0.001f;
	glUniform1f(CGraphicBase::ms_iLocSkyTime, curTime);

	CCamera* pCam = CCameraManager::Instance().GetCurrentCamera();
	if (pCam)
	{
		const D3DXVECTOR3& eye = pCam->GetEye();
		glUniform3f(CGraphicBase::ms_iLocSkyCameraPos, eye.x, eye.y, eye.z);
	}

	// Set sky mode and gradient/sun data
	int skyMode = s_iSkyMode;
	if (isCloud)
		skyMode = 2;

	glUniform1i(CGraphicBase::ms_iLocSkyMode, skyMode);

	if (skyMode == 0 && s_iSkyGradientCount > 0)
	{
		glUniform4fv(CGraphicBase::ms_iLocSkyColors, s_iSkyGradientCount, s_fSkyGradientColors);
		glUniform1i(CGraphicBase::ms_iLocSkyColorCount, s_iSkyGradientCount);
	}
	else
	{
		glUniform1i(CGraphicBase::ms_iLocSkyColorCount, 0);
	}

	// Sun data for cloud lighting
	glUniform3fv(CGraphicBase::ms_iLocSkySunDir, 1, s_fSkySunDir);
	glUniform1f(CGraphicBase::ms_iLocSkySunIntensity, s_fSkySunIntensity);

	int useTex = 0;
	if (isCloud)
	{
		useTex = 1;
	}
	else if (state.m_TextureStates[0][D3DTSS_COLOROP] == D3DTOP_SELECTARG1 && state.m_Textures[0] != nullptr)
	{
		useTex = 1;
	}

	glUniform1i(CGraphicBase::ms_iLocSkyUseTexture, useTex);
	glUniform1i(CGraphicBase::ms_iLocSkyTexture, 0);
	if (useTex && state.m_Textures[0])
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, (GLuint)(uintptr_t)state.m_Textures[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, isCloud ? GL_REPEAT : GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, isCloud ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	}
}

static void SetupWaterShaderAndStates(const CStateManagerState& state)
{
	glUseProgram(CGraphicBase::ms_uWaterShaderProgram);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	static D3DXMATRIX s_matWVP;
	D3DXMatrixMultiply(&s_matWVP, &state.m_Matrices[D3DTS_WORLD], &state.m_Matrices[D3DTS_VIEW]);
	D3DXMatrixMultiply(&s_matWVP, &s_matWVP, &state.m_Matrices[D3DTS_PROJECTION]);
	glUniformMatrix4fv(CGraphicBase::ms_iLocWaterWVP, 1, GL_FALSE, (const float*)&s_matWVP);
	glUniformMatrix4fv(CGraphicBase::ms_iLocWaterWorld, 1, GL_FALSE, (const float*)&state.m_Matrices[D3DTS_WORLD]);

	glUniform1f(CGraphicBase::ms_iLocWaterScale, 0.00125f);

	CCamera* pCam = CCameraManager::Instance().GetCurrentCamera();
	if (pCam)
	{
		const D3DXVECTOR3& eye = pCam->GetEye();
		glUniform3f(CGraphicBase::ms_iLocWaterCameraPos, eye.x, eye.y, eye.z);
	}

	int fogEnable = (state.m_RenderStates[D3DRS_FOGENABLE] && state.m_RenderStates[D3DRS_FOGENABLE] != 0x7FFFFFFF) ? 1 : 0;
	DWORD fogColor = state.m_RenderStates[D3DRS_FOGCOLOR];
	float fcA = float((fogColor >> 24) & 0xFF) / 255.0f;
	float fcR = float((fogColor >> 16) & 0xFF) / 255.0f;
	float fcG = float((fogColor >> 8) & 0xFF) / 255.0f;
	float fcB = float(fogColor & 0xFF) / 255.0f;
	glUniform4f(CGraphicBase::ms_iLocWaterFogColor, fcR, fcG, fcB, fcA);

	float fogNear = *(float*)&state.m_RenderStates[D3DRS_FOGSTART];
	float fogFar = *(float*)&state.m_RenderStates[D3DRS_FOGEND];
	glUniform2f(CGraphicBase::ms_iLocWaterFogRange, fogNear, fogFar);
	glUniform1i(CGraphicBase::ms_iLocWaterFogEnable, fogEnable);

	float curTime = CTimer::Instance().GetCurrentMillisecond() * 0.001f;
	glUniform1f(CGraphicBase::ms_iLocWaterTime, curTime);

	glUniform1i(CGraphicBase::ms_iLocWaterTexture, 0);
	if (state.m_Textures[0])
	{
		static GLuint s_lastWaterTex = 0;
		GLuint tex0 = (GLuint)(uintptr_t)state.m_Textures[0];
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex0);
		if (s_lastWaterTex != tex0)
		{
			s_lastWaterTex = tex0;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
	}
}
#endif

static void SetupUIShaderAndStates(const CStateManagerState& state)
{
	glUseProgram(CGraphicBase::ms_uUIShaderProgram);

	if (state.m_RenderStates[D3DRS_ZENABLE] && state.m_RenderStates[D3DRS_ZENABLE] != 0x7FFFFFFF)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}

	if (state.m_RenderStates[D3DRS_ZWRITEENABLE] && state.m_RenderStates[D3DRS_ZWRITEENABLE] != 0x7FFFFFFF)
		glDepthMask(GL_TRUE);
	else
		glDepthMask(GL_FALSE);

	glDisable(GL_CULL_FACE);

	if (state.m_RenderStates[D3DRS_ALPHABLENDENABLE] && state.m_RenderStates[D3DRS_ALPHABLENDENABLE] != 0x7FFFFFFF)
	{
		glEnable(GL_BLEND);

		DWORD srcBlend = state.m_RenderStates[D3DRS_SRCBLEND];
		DWORD destBlend = state.m_RenderStates[D3DRS_DESTBLEND];
		GLenum glSrc = GL_SRC_ALPHA;
		GLenum glDst = GL_ONE_MINUS_SRC_ALPHA;
		if (srcBlend == D3DBLEND_INVDESTCOLOR && destBlend == D3DBLEND_ONE)
		{
			glSrc = GL_ONE_MINUS_DST_COLOR;
			glDst = GL_ONE;
		}
		else if (srcBlend == D3DBLEND_ZERO && destBlend == D3DBLEND_SRCCOLOR)
		{
			glSrc = GL_ZERO;
			glDst = GL_SRC_COLOR;
		}
		else if (srcBlend == D3DBLEND_ONE && destBlend == D3DBLEND_ONE)
		{
			glSrc = GL_ONE;
			glDst = GL_ONE;
		}
		glBlendFunc(glSrc, glDst);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	const float* pMatrixToUpload = nullptr;
	static D3DXMATRIX s_matWVP;

	bool isIdentity = (fabs(state.m_Matrices[D3DTS_PROJECTION]._11 - 1.0f) < 0.001f &&
	                   fabs(state.m_Matrices[D3DTS_PROJECTION]._22 - 1.0f) < 0.001f &&
	                   fabs(state.m_Matrices[D3DTS_PROJECTION]._33 - 1.0f) < 0.001f &&
	                   fabs(state.m_Matrices[D3DTS_PROJECTION]._41) < 0.001f);

	if (!isIdentity && (state.m_Matrices[D3DTS_PROJECTION]._11 != 0.0f || state.m_Matrices[D3DTS_PROJECTION]._22 != 0.0f))
	{
		D3DXMatrixMultiply(&s_matWVP, &state.m_Matrices[D3DTS_WORLD], &state.m_Matrices[D3DTS_VIEW]);
		D3DXMatrixMultiply(&s_matWVP, &s_matWVP, &state.m_Matrices[D3DTS_PROJECTION]);
		pMatrixToUpload = (const float*)&s_matWVP;
	}
	else
	{
		UINT w = 1024, h = 768;
		CGraphicBase::GetBackBufferSize(&w, &h);
		if (w == 0) w = 1024;
		if (h == 0) h = 768;
		D3DXMATRIX matOrtho;
		D3DXMatrixOrthoOffCenterRH(&matOrtho, 0.0f, (float)w, (float)h, 0.0f, 0.0f, 1000.0f);
		D3DXMatrixMultiply(&s_matWVP, &state.m_Matrices[D3DTS_WORLD], &state.m_Matrices[D3DTS_VIEW]);
		D3DXMatrixMultiply(&s_matWVP, &s_matWVP, &matOrtho);
		pMatrixToUpload = (const float*)&s_matWVP;
	}
	glUniformMatrix4fv(CGraphicBase::ms_iLocProjMatrix, 1, GL_FALSE, pMatrixToUpload);
	glUniformMatrix4fv(CGraphicBase::ms_iLocUIWorld, 1, GL_FALSE, (const float*)&state.m_Matrices[D3DTS_WORLD]);
	glUniformMatrix4fv(CGraphicBase::ms_iLocUITexMat1, 1, GL_FALSE, (const float*)&state.m_Matrices[D3DTS_TEXTURE1]);

	int useTex = (state.m_Textures[0] != nullptr && state.m_TextureStates[0][D3DTSS_COLOROP] != D3DTOP_DISABLE) ? 1 : 0;
	glUniform1i(CGraphicBase::ms_iLocUseTexture, useTex);
	glUniform1i(CGraphicBase::ms_iLocTexture, 0);


	if (useTex)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, (GLuint)(uintptr_t)state.m_Textures[0]);

		DWORD mag = state.m_SamplerStates[0][D3DSAMP_MAGFILTER];
		DWORD min = state.m_SamplerStates[0][D3DSAMP_MINFILTER];
		GLint glMag = (mag == D3DTEXF_LINEAR) ? GL_LINEAR : GL_NEAREST;
		GLint glMin = (min == D3DTEXF_LINEAR) ? GL_LINEAR : GL_NEAREST;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glMin);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glMag);
	}

	int useMask = (state.m_Textures[1] != nullptr && state.m_TextureStates[1][D3DTSS_COLOROP] != D3DTOP_DISABLE) ? 1 : 0;
	glUniform1i(CGraphicBase::ms_iLocUIUseMask, useMask);
	glUniform1i(CGraphicBase::ms_iLocUIMaskTexture, 1);
	if (useMask)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, (GLuint)(uintptr_t)state.m_Textures[1]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	DWORD colorArg1 = state.m_TextureStates[0][D3DTSS_COLORARG1];
	DWORD colorArg2 = state.m_TextureStates[0][D3DTSS_COLORARG2];
	DWORD colorOp = state.m_TextureStates[0][D3DTSS_COLOROP];
	DWORD alphaArg1 = state.m_TextureStates[0][D3DTSS_ALPHAARG1];
	DWORD alphaArg2 = state.m_TextureStates[0][D3DTSS_ALPHAARG2];
	DWORD alphaOp = state.m_TextureStates[0][D3DTSS_ALPHAOP];

	int useTFactor = 0;
	if (colorArg1 == D3DTA_TFACTOR || colorArg2 == D3DTA_TFACTOR)
	{
		if ((colorOp == D3DTOP_SELECTARG1 && colorArg1 == D3DTA_TFACTOR) ||
		    (colorOp == D3DTOP_SELECTARG2 && colorArg2 == D3DTA_TFACTOR))
		{
			useTFactor = 2;
		}
		else if (colorOp == D3DTOP_ADD)
		{
			useTFactor = 3;
		}
		else
		{
			useTFactor = 1;
		}
	}

	DWORD tfactor = state.m_RenderStates[D3DRS_TEXTUREFACTOR];
	if (tfactor == 0x7FFFFFFF)
		tfactor = 0xFFFFFFFF;

	float tfA = float((tfactor >> 24) & 0xFF) / 255.0f;
	float tfR = float((tfactor >> 16) & 0xFF) / 255.0f;
	float tfG = float((tfactor >> 8) & 0xFF) / 255.0f;
	float tfB = float(tfactor & 0xFF) / 255.0f;

	if (useTFactor != 0)
	{
		if (alphaOp == D3DTOP_SELECTARG1 && alphaArg1 != D3DTA_TFACTOR)
			tfA = 1.0f;
		else if (alphaOp == D3DTOP_SELECTARG2 && alphaArg2 != D3DTA_TFACTOR)
			tfA = 1.0f;
		else if (alphaArg1 != D3DTA_TFACTOR && alphaArg2 != D3DTA_TFACTOR)
			tfA = 1.0f;
	}
	else
	{
		tfA = 1.0f;
	}

	glUniform1i(CGraphicBase::ms_iLocUIUseTFactor, useTFactor);
	glUniform4f(CGraphicBase::ms_iLocUITFactorColor, tfR, tfG, tfB, tfA);

	glActiveTexture(GL_TEXTURE0);
}

static void SetupSpeedTreeShaderAndStates(const CStateManagerState& state)
{
	glUseProgram(CGraphicBase::ms_uSpeedTreeShaderProgram);

	if (state.m_RenderStates[D3DRS_ZENABLE] && state.m_RenderStates[D3DRS_ZENABLE] != 0x7FFFFFFF)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}

	if (state.m_RenderStates[D3DRS_ZWRITEENABLE] && state.m_RenderStates[D3DRS_ZWRITEENABLE] != 0x7FFFFFFF)
		glDepthMask(GL_TRUE);
	else
		glDepthMask(GL_FALSE);

	glDisable(GL_CULL_FACE);

	if (state.m_RenderStates[D3DRS_ALPHABLENDENABLE] && state.m_RenderStates[D3DRS_ALPHABLENDENABLE] != 0x7FFFFFFF)
	{
		glEnable(GL_BLEND);
		GLenum glSrc = D3DToGLBlend(state.m_RenderStates[D3DRS_SRCBLEND]);
		GLenum glDst = D3DToGLBlend(state.m_RenderStates[D3DRS_DESTBLEND]);
		glBlendFunc(glSrc, glDst);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	static D3DXMATRIX s_matWVP;
	D3DXMatrixMultiply(&s_matWVP, &state.m_Matrices[D3DTS_WORLD], &state.m_Matrices[D3DTS_VIEW]);
	D3DXMatrixMultiply(&s_matWVP, &s_matWVP, &state.m_Matrices[D3DTS_PROJECTION]);
	glUniformMatrix4fv(CGraphicBase::ms_iLocSpeedTreeWVP, 1, GL_FALSE, (const float*)&s_matWVP);
	glUniformMatrix4fv(CGraphicBase::ms_iLocSpeedTreeWorld, 1, GL_FALSE, (const float*)&state.m_Matrices[D3DTS_WORLD]);

	int alphaTest = (state.m_RenderStates[D3DRS_ALPHATESTENABLE] && state.m_RenderStates[D3DRS_ALPHATESTENABLE] != 0x7FFFFFFF) ? 1 : 0;
	float alphaRef = 0.33f;
	if (state.m_RenderStates[D3DRS_ALPHAREF] != 0x7FFFFFFF)
	{
		DWORD dwRef = state.m_RenderStates[D3DRS_ALPHAREF];
		if (dwRef >= 254)
			alphaRef = 1.1f;
		else
			alphaRef = float(dwRef & 0xFF) / 255.0f;
	}
	glUniform1i(CGraphicBase::ms_iLocSpeedTreeAlphaTest, alphaTest);
	glUniform1f(CGraphicBase::ms_iLocSpeedTreeAlphaRef, alphaRef);

	int fogEnable = (state.m_RenderStates[D3DRS_FOGENABLE] && state.m_RenderStates[D3DRS_FOGENABLE] != 0x7FFFFFFF) ? 1 : 0;
	glUniform1i(CGraphicBase::ms_iLocSpeedTreeFogEnable, fogEnable);

	DWORD dwFogColor = (state.m_RenderStates[D3DRS_FOGCOLOR] != 0x7FFFFFFF) ? state.m_RenderStates[D3DRS_FOGCOLOR] : 0xFFFFFFFF;
	float fogR = float((dwFogColor >> 16) & 0xFF) / 255.0f;
	float fogG = float((dwFogColor >> 8) & 0xFF) / 255.0f;
	float fogB = float(dwFogColor & 0xFF) / 255.0f;
	float fogA = float((dwFogColor >> 24) & 0xFF) / 255.0f;
	if (fogA < 0.001f) fogA = 1.0f;
	glUniform4f(CGraphicBase::ms_iLocSpeedTreeFogColor, fogR, fogG, fogB, fogA);

	float fogStart = 3500.0f;
	float fogEnd = 18500.0f;
	if (state.m_RenderStates[D3DRS_FOGSTART] != 0x7FFFFFFF && state.m_RenderStates[D3DRS_FOGSTART] != 0)
	{
		float fs = *((const float*)&state.m_RenderStates[D3DRS_FOGSTART]);
		if (fs > 10.0f && fs < 200000.0f)
			fogStart = fs;
	}
	if (state.m_RenderStates[D3DRS_FOGEND] != 0x7FFFFFFF && state.m_RenderStates[D3DRS_FOGEND] != 0)
	{
		float fe = *((const float*)&state.m_RenderStates[D3DRS_FOGEND]);
		if (fe > fogStart && fe < 200000.0f)
			fogEnd = fe;
	}
	glUniform2f(CGraphicBase::ms_iLocSpeedTreeFogRange, fogStart, fogEnd);

	CCamera* pCam = CCameraManager::Instance().GetCurrentCamera();
	D3DXVECTOR3 camPos(0.0f, 0.0f, 0.0f);
	if (pCam)
		camPos = pCam->GetEye();
	glUniform3f(CGraphicBase::ms_iLocSpeedTreeCameraPos, camPos.x, camPos.y, camPos.z);
	glUniform3f(CGraphicBase::ms_iLocSpeedTreeSunDir, s_fSkySunDir[0], s_fSkySunDir[1], s_fSkySunDir[2]);

	glUniform1i(CGraphicBase::ms_iLocSpeedTreeTexture, 0);
	glActiveTexture(GL_TEXTURE0);
	if (state.m_Textures[0] != nullptr)
	{
		glBindTexture(GL_TEXTURE_2D, (GLuint)(uintptr_t)state.m_Textures[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
}
#endif

HRESULT CStateManager::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) const
{
#if defined(USE_OPENGL_ES)
	UINT stride = m_CurrentState.m_StreamData[0].m_Stride;
	bool isOrtho = (fabs(m_CurrentState.m_Matrices[D3DTS_PROJECTION]._34) < 0.001f);
	if ((stride == sizeof(TPNTVertex) || stride == sizeof(TPNT2Vertex)) && CGraphicBase::ms_uMeshShaderProgram != 0)
	{
		SetupMeshShaderAndStates(m_CurrentState);

		if (m_CurrentState.m_StreamData[0].m_lpStreamData)
			glBindBuffer(GL_ARRAY_BUFFER, (GLuint)(uintptr_t)m_CurrentState.m_StreamData[0].m_lpStreamData);

		if (stride == sizeof(TPNT2Vertex))
		{
			SetEnabledAttribMask((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3));
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)12);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)24);
			glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, (void*)32);
		}
		else
		{
			SetEnabledAttribMask((1 << 0) | (1 << 1) | (1 << 2));
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)12);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)24);
		}

		GLenum mode = GL_TRIANGLES;
		GLsizei count = PrimitiveCount * 3;
		if (PrimitiveType == D3DPT_TRIANGLESTRIP)
		{
			mode = GL_TRIANGLE_STRIP;
			count = PrimitiveCount + 2;
		}
		else if (PrimitiveType == D3DPT_TRIANGLEFAN)
		{
			mode = GL_TRIANGLE_FAN;
			count = PrimitiveCount + 2;
		}
		else if (PrimitiveType == D3DPT_LINELIST)
		{
			mode = GL_LINES;
			count = PrimitiveCount * 2;
		}
		else if (PrimitiveType == D3DPT_LINESTRIP)
		{
			mode = GL_LINE_STRIP;
			count = PrimitiveCount + 1;
		}
		else if (PrimitiveType == D3DPT_POINTLIST)
		{
			mode = GL_POINTS;
			count = PrimitiveCount;
		}

		glDrawArrays(mode, StartVertex, count);
	}
	else if ((CGraphicBase::IsWaterMode() || stride == 16) && CGraphicBase::ms_uWaterShaderProgram != 0)
	{
		SetupWaterShaderAndStates(m_CurrentState);

		if (m_CurrentState.m_StreamData[0].m_lpStreamData)
			glBindBuffer(GL_ARRAY_BUFFER, (GLuint)(uintptr_t)m_CurrentState.m_StreamData[0].m_lpStreamData);

		if (stride == 0)
			stride = 16;

		SetEnabledAttribMask((1 << 0) | (1 << 1));
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (void*)12);

		GLenum mode = GL_TRIANGLES;
		GLsizei count = PrimitiveCount * 3;
		if (PrimitiveType == D3DPT_TRIANGLESTRIP)
		{
			mode = GL_TRIANGLE_STRIP;
			count = PrimitiveCount + 2;
		}
		else if (PrimitiveType == D3DPT_TRIANGLEFAN)
		{
			mode = GL_TRIANGLE_FAN;
			count = PrimitiveCount + 2;
		}

		glDrawArrays(mode, StartVertex, count);
	}
	else if (CGraphicBase::IsSkyBoxMode() && CGraphicBase::ms_uSkyShaderProgram != 0)
	{
		SetupSkyShaderAndStates(m_CurrentState);

		if (m_CurrentState.m_StreamData[0].m_lpStreamData)
			glBindBuffer(GL_ARRAY_BUFFER, (GLuint)(uintptr_t)m_CurrentState.m_StreamData[0].m_lpStreamData);
		else
			glBindBuffer(GL_ARRAY_BUFFER, CGraphicBase::ms_uPDTVBO);

		if (stride == 0)
			stride = sizeof(TPDTVertex);

		SetEnabledAttribMask((1 << 0) | (1 << 1) | (1 << 2));
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (void*)12);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)16);

		GLenum mode = GL_TRIANGLES;
		GLsizei count = PrimitiveCount * 3;
		if (PrimitiveType == D3DPT_TRIANGLESTRIP)
		{
			mode = GL_TRIANGLE_STRIP;
			count = PrimitiveCount + 2;
		}
		else if (PrimitiveType == D3DPT_TRIANGLEFAN)
		{
			mode = GL_TRIANGLE_FAN;
			count = PrimitiveCount + 2;
		}

		glDrawArrays(mode, StartVertex, count);
	}
	else if (CGraphicBase::IsSpeedTreeMode() && CGraphicBase::ms_uSpeedTreeShaderProgram != 0)
	{
		SetupSpeedTreeShaderAndStates(m_CurrentState);

		if (m_CurrentState.m_StreamData[0].m_lpStreamData)
			glBindBuffer(GL_ARRAY_BUFFER, (GLuint)(uintptr_t)m_CurrentState.m_StreamData[0].m_lpStreamData);

		if (stride == 0)
			stride = 24;

		SetEnabledAttribMask((1 << 0) | (1 << 1) | (1 << 2));
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (void*)12);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)16);

		GLenum mode = GL_TRIANGLES;
		GLsizei count = PrimitiveCount * 3;
		if (PrimitiveType == D3DPT_TRIANGLESTRIP)
		{
			mode = GL_TRIANGLE_STRIP;
			count = PrimitiveCount + 2;
		}
		else if (PrimitiveType == D3DPT_TRIANGLEFAN)
		{
			mode = GL_TRIANGLE_FAN;
			count = PrimitiveCount + 2;
		}
		else if (PrimitiveType == D3DPT_LINELIST)
		{
			mode = GL_LINES;
			count = PrimitiveCount * 2;
		}

		glDrawArrays(mode, StartVertex, count);
	}
	else if (!isOrtho && CGraphicBase::ms_uParticleShaderProgram != 0)
	{
		bool hasColor = (stride >= 24);
		SetupParticleShaderAndStates(m_CurrentState, hasColor);

		if (m_CurrentState.m_StreamData[0].m_lpStreamData)
			glBindBuffer(GL_ARRAY_BUFFER, (GLuint)(uintptr_t)m_CurrentState.m_StreamData[0].m_lpStreamData);
		else
			glBindBuffer(GL_ARRAY_BUFFER, CGraphicBase::ms_uPDTVBO);

		if (stride == 0)
			stride = sizeof(TPDTVertex);

		if (hasColor)
		{
			SetEnabledAttribMask((1 << 0) | (1 << 1) | (1 << 2));
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
			glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (void*)12);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)16);
		}
		else
		{
			SetEnabledAttribMask((1 << 0) | (1 << 2));
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)12);
		}

		GLenum mode = GL_TRIANGLES;
		GLsizei count = PrimitiveCount * 3;
		if (PrimitiveType == D3DPT_TRIANGLESTRIP)
		{
			mode = GL_TRIANGLE_STRIP;
			count = PrimitiveCount + 2;
		}
		else if (PrimitiveType == D3DPT_TRIANGLEFAN)
		{
			mode = GL_TRIANGLE_FAN;
			count = PrimitiveCount + 2;
		}
		else if (PrimitiveType == D3DPT_LINELIST)
		{
			mode = GL_LINES;
			count = PrimitiveCount * 2;
		}
		else if (PrimitiveType == D3DPT_LINESTRIP)
		{
			mode = GL_LINE_STRIP;
			count = PrimitiveCount + 1;
		}
		else if (PrimitiveType == D3DPT_POINTLIST)
		{
			mode = GL_POINTS;
			count = PrimitiveCount;
		}

		glDrawArrays(mode, StartVertex, count);
	}
	else if (CGraphicBase::ms_uUIShaderProgram != 0)
	{
		SetupUIShaderAndStates(m_CurrentState);

		if (m_CurrentState.m_StreamData[0].m_lpStreamData)
			glBindBuffer(GL_ARRAY_BUFFER, (GLuint)(uintptr_t)m_CurrentState.m_StreamData[0].m_lpStreamData);
		else
			glBindBuffer(GL_ARRAY_BUFFER, CGraphicBase::ms_uPDTVBO);

		if (stride == 0)
			stride = sizeof(TPDTVertex);

		if (stride == 20)
		{
			SetEnabledAttribMask((1 << 0) | (1 << 2));
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
			glVertexAttrib4f(1, 1.0f, 1.0f, 1.0f, 1.0f);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)12);
		}
		else
		{
			SetEnabledAttribMask((1 << 0) | (1 << 1) | (1 << 2));
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
			glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (void*)12);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)16);
		}

		GLenum mode = GL_TRIANGLES;
		GLsizei count = PrimitiveCount * 3;
		if (PrimitiveType == D3DPT_TRIANGLESTRIP)
		{
			mode = GL_TRIANGLE_STRIP;
			count = PrimitiveCount + 2;
		}
		else if (PrimitiveType == D3DPT_TRIANGLEFAN)
		{
			mode = GL_TRIANGLE_FAN;
			count = PrimitiveCount + 2;
		}
		else if (PrimitiveType == D3DPT_LINELIST)
		{
			mode = GL_LINES;
			count = PrimitiveCount * 2;
		}
		else if (PrimitiveType == D3DPT_LINESTRIP)
		{
			mode = GL_LINE_STRIP;
			count = PrimitiveCount + 1;
		}
		else if (PrimitiveType == D3DPT_POINTLIST)
		{
			mode = GL_POINTS;
			count = PrimitiveCount;
		}

		glDrawArrays(mode, StartVertex, count);
	}
	return S_OK;
#else
	if (m_lpD3DDev) return (m_lpD3DDev->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount)); return S_OK;
#endif
}

HRESULT CStateManager::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, const void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	m_CurrentState.m_StreamData[0] = nullptr;
#if defined(USE_OPENGL_ES)
	if (pVertexStreamZeroData != nullptr)
	{
		const char* ptr = (const char*)pVertexStreamZeroData;
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		bool isOrtho = (fabs(m_CurrentState.m_Matrices[D3DTS_PROJECTION]._34) < 0.001f);

		if (CGraphicBase::IsSpeedTreeMode() && CGraphicBase::ms_uSpeedTreeShaderProgram != 0)
		{
			SetupSpeedTreeShaderAndStates(m_CurrentState);

			SetEnabledAttribMask((1 << 0) | (1 << 1) | (1 << 2));
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VertexStreamZeroStride, ptr);
			glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, VertexStreamZeroStride, ptr + 12);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, VertexStreamZeroStride, ptr + 16);
		}
		else if (!isOrtho && CGraphicBase::ms_uParticleShaderProgram != 0)
		{
			bool hasColor = (VertexStreamZeroStride >= 24);
			SetupParticleShaderAndStates(m_CurrentState, hasColor);

			if (hasColor)
			{
				SetEnabledAttribMask((1 << 0) | (1 << 1) | (1 << 2));
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VertexStreamZeroStride, ptr);
				glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, VertexStreamZeroStride, ptr + 12);
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, VertexStreamZeroStride, ptr + 16);
			}
			else
			{
				SetEnabledAttribMask((1 << 0) | (1 << 2));
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VertexStreamZeroStride, ptr);
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, VertexStreamZeroStride, ptr + 12);
			}
		}
		else if (CGraphicBase::ms_uUIShaderProgram != 0)
		{
			SetupUIShaderAndStates(m_CurrentState);

			if (VertexStreamZeroStride == 20)
			{
				SetEnabledAttribMask((1 << 0) | (1 << 2));
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VertexStreamZeroStride, ptr);
				glVertexAttrib4f(1, 1.0f, 1.0f, 1.0f, 1.0f);
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, VertexStreamZeroStride, ptr + 12);
			}
			else
			{
				SetEnabledAttribMask((1 << 0) | (1 << 1) | (1 << 2));
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VertexStreamZeroStride, ptr);
				glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, VertexStreamZeroStride, ptr + 12);
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, VertexStreamZeroStride, ptr + 16);
			}
		}

		GLenum mode = GL_TRIANGLES;
		GLsizei count = PrimitiveCount * 3;
		if (PrimitiveType == D3DPT_TRIANGLESTRIP)
		{
			mode = GL_TRIANGLE_STRIP;
			count = PrimitiveCount + 2;
		}
		else if (PrimitiveType == D3DPT_TRIANGLEFAN)
		{
			mode = GL_TRIANGLE_FAN;
			count = PrimitiveCount + 2;
		}
		else if (PrimitiveType == D3DPT_LINELIST)
		{
			mode = GL_LINES;
			count = PrimitiveCount * 2;
		}
		else if (PrimitiveType == D3DPT_LINESTRIP)
		{
			mode = GL_LINE_STRIP;
			count = PrimitiveCount + 1;
		}
		else if (PrimitiveType == D3DPT_POINTLIST)
		{
			mode = GL_POINTS;
			count = PrimitiveCount;
		}

		glDrawArrays(mode, 0, count);
	}
	return S_OK;
#else
	if (m_lpD3DDev) return (m_lpD3DDev->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride)); return S_OK;
#endif
}

HRESULT CStateManager::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT minIndex, UINT NumVertices, UINT startIndex, UINT primCount, INT baseVertexIndex) const
{
#if defined(USE_OPENGL_ES)
	if (primCount == 0)
		return S_OK;

	UINT stride = m_CurrentState.m_StreamData[0].m_Stride;
	bool isOrtho = (fabs(m_CurrentState.m_Matrices[D3DTS_PROJECTION]._34) < 0.001f);
	if ((stride == sizeof(TPNTVertex) || stride == sizeof(TPNT2Vertex)) && CGraphicBase::ms_uMeshShaderProgram != 0)
	{
		if (!m_CurrentState.m_StreamData[0].m_lpStreamData || !m_CurrentState.m_IndexData.m_lpIndexData)
			return S_OK;

		SetupMeshShaderAndStates(m_CurrentState);

		glBindBuffer(GL_ARRAY_BUFFER, (GLuint)(uintptr_t)m_CurrentState.m_StreamData[0].m_lpStreamData);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)(uintptr_t)m_CurrentState.m_IndexData.m_lpIndexData);

		uintptr_t baseOffset = (uintptr_t)(baseVertexIndex * (int)stride);

		if (stride == sizeof(TPNT2Vertex))
		{
			SetEnabledAttribMask((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3));
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)(baseOffset + 0));
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(baseOffset + 12));
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(baseOffset + 24));
			glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, (void*)(baseOffset + 32));
		}
		else
		{
			SetEnabledAttribMask((1 << 0) | (1 << 1) | (1 << 2));
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)(baseOffset + 0));
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(baseOffset + 12));
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(baseOffset + 24));
		}

		GLenum mode = GL_TRIANGLES;
		GLsizei count = primCount * 3;
		if (PrimitiveType == D3DPT_LINELIST)
		{
			mode = GL_LINES;
			count = primCount * 2;
		}
		else if (PrimitiveType == D3DPT_TRIANGLESTRIP)
		{
			mode = GL_TRIANGLE_STRIP;
			count = primCount + 2;
		}

		glDrawElements(mode, count, GL_UNSIGNED_SHORT, (void*)(startIndex * sizeof(WORD)));
	}
	else if (stride == 24 && (m_CurrentState.m_dwFVF & D3DFVF_NORMAL) != 0 && CGraphicBase::ms_uTerrainShaderProgram != 0)
	{
		if (!m_CurrentState.m_StreamData[0].m_lpStreamData || !m_CurrentState.m_IndexData.m_lpIndexData)
			return S_OK;

		SetupTerrainShaderAndStates(m_CurrentState);

		glBindBuffer(GL_ARRAY_BUFFER, (GLuint)(uintptr_t)m_CurrentState.m_StreamData[0].m_lpStreamData);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)(uintptr_t)m_CurrentState.m_IndexData.m_lpIndexData);

		uintptr_t baseOffset = (uintptr_t)(baseVertexIndex * (int)stride);

		SetEnabledAttribMask((1 << 0) | (1 << 1));
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)(baseOffset + 0));
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(baseOffset + 12));

		GLenum mode = GL_TRIANGLES;
		GLsizei count = primCount * 3;
		if (PrimitiveType == D3DPT_LINELIST)
		{
			mode = GL_LINES;
			count = primCount * 2;
		}
		else if (PrimitiveType == D3DPT_TRIANGLESTRIP)
		{
			mode = GL_TRIANGLE_STRIP;
			count = primCount + 2;
		}

		glDrawElements(mode, count, GL_UNSIGNED_SHORT, (void*)(startIndex * sizeof(WORD)));
	}
	else if (CGraphicBase::IsSpeedTreeMode() && CGraphicBase::ms_uSpeedTreeShaderProgram != 0)
	{
		if (!m_CurrentState.m_StreamData[0].m_lpStreamData || !m_CurrentState.m_IndexData.m_lpIndexData)
			return S_OK;

		SetupSpeedTreeShaderAndStates(m_CurrentState);

		glBindBuffer(GL_ARRAY_BUFFER, (GLuint)(uintptr_t)m_CurrentState.m_StreamData[0].m_lpStreamData);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)(uintptr_t)m_CurrentState.m_IndexData.m_lpIndexData);

		if (stride == 0)
			stride = (m_CurrentState.m_StreamData[0].m_Stride != 0) ? m_CurrentState.m_StreamData[0].m_Stride : 32;

		uintptr_t baseOffset = (uintptr_t)(baseVertexIndex * (int)stride);

		SetEnabledAttribMask((1 << 0) | (1 << 1) | (1 << 2));
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)(baseOffset + 0));
		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (void*)(baseOffset + 12));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(baseOffset + 16));

		GLenum mode = GL_TRIANGLES;
		GLsizei count = primCount * 3;
		if (PrimitiveType == D3DPT_LINELIST)
		{
			mode = GL_LINES;
			count = primCount * 2;
		}
		else if (PrimitiveType == D3DPT_TRIANGLESTRIP)
		{
			mode = GL_TRIANGLE_STRIP;
			count = primCount + 2;
		}

		glDrawElements(mode, count, GL_UNSIGNED_SHORT, (void*)(startIndex * sizeof(WORD)));
	}
	else if (!isOrtho && CGraphicBase::ms_uParticleShaderProgram != 0)
	{
		bool hasColor = (stride >= 24);
		SetupParticleShaderAndStates(m_CurrentState, hasColor);

		if (m_CurrentState.m_StreamData[0].m_lpStreamData)
			glBindBuffer(GL_ARRAY_BUFFER, (GLuint)(uintptr_t)m_CurrentState.m_StreamData[0].m_lpStreamData);
		else
			glBindBuffer(GL_ARRAY_BUFFER, CGraphicBase::ms_uPDTVBO);

		if (stride == 0)
			stride = sizeof(TPDTVertex);

		if (m_CurrentState.m_IndexData.m_lpIndexData)
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)(uintptr_t)m_CurrentState.m_IndexData.m_lpIndexData);
		else if (CGraphicBase::ms_auDefEBO[CGraphicBase::DEFAULT_IB_FILL_RECT] != 0)
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CGraphicBase::ms_auDefEBO[CGraphicBase::DEFAULT_IB_FILL_RECT]);

		uintptr_t baseOffset = (uintptr_t)(baseVertexIndex * (int)stride);

		if (hasColor)
		{
			SetEnabledAttribMask((1 << 0) | (1 << 1) | (1 << 2));
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)(baseOffset + 0));
			glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (void*)(baseOffset + 12));
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(baseOffset + 16));
		}
		else
		{
			SetEnabledAttribMask((1 << 0) | (1 << 2));
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)(baseOffset + 0));
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(baseOffset + 12));
		}

		GLenum mode = GL_TRIANGLES;
		GLsizei count = primCount * 3;
		if (PrimitiveType == D3DPT_LINELIST)
		{
			mode = GL_LINES;
			count = primCount * 2;
		}
		else if (PrimitiveType == D3DPT_TRIANGLESTRIP)
		{
			mode = GL_TRIANGLE_STRIP;
			count = primCount + 2;
		}

		glDrawElements(mode, count, GL_UNSIGNED_SHORT, (void*)(startIndex * sizeof(WORD)));
	}
	else if (CGraphicBase::ms_uUIShaderProgram != 0)
	{
		SetupUIShaderAndStates(m_CurrentState);

		if (m_CurrentState.m_StreamData[0].m_lpStreamData)
			glBindBuffer(GL_ARRAY_BUFFER, (GLuint)(uintptr_t)m_CurrentState.m_StreamData[0].m_lpStreamData);
		else
			glBindBuffer(GL_ARRAY_BUFFER, CGraphicBase::ms_uPDTVBO);

		if (stride == 0)
			stride = sizeof(TPDTVertex);

		if (m_CurrentState.m_IndexData.m_lpIndexData)
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)(uintptr_t)m_CurrentState.m_IndexData.m_lpIndexData);
		else if (CGraphicBase::ms_auDefEBO[CGraphicBase::DEFAULT_IB_FILL_RECT] != 0)
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CGraphicBase::ms_auDefEBO[CGraphicBase::DEFAULT_IB_FILL_RECT]);

		uintptr_t baseOffset = (uintptr_t)(baseVertexIndex * (int)stride);

		if (stride == 20)
		{
			SetEnabledAttribMask((1 << 0) | (1 << 2));
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)(baseOffset + 0));
			glVertexAttrib4f(1, 1.0f, 1.0f, 1.0f, 1.0f);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(baseOffset + 12));
		}
		else
		{
			SetEnabledAttribMask((1 << 0) | (1 << 1) | (1 << 2));
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)(baseOffset + 0));
			glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (void*)(baseOffset + 12));
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(baseOffset + 16));
		}

		GLenum mode = GL_TRIANGLES;
		GLsizei count = primCount * 3;
		if (PrimitiveType == D3DPT_LINELIST)
		{
			mode = GL_LINES;
			count = primCount * 2;
		}
		else if (PrimitiveType == D3DPT_TRIANGLESTRIP)
		{
			mode = GL_TRIANGLE_STRIP;
			count = primCount + 2;
		}

		glDrawElements(mode, count, GL_UNSIGNED_SHORT, (void*)(startIndex * sizeof(WORD)));
	}
	return S_OK;
#else
	if (m_lpD3DDev) return (m_lpD3DDev->DrawIndexedPrimitive(PrimitiveType, baseVertexIndex, minIndex, NumVertices, startIndex, primCount)); return S_OK;
#endif
}

HRESULT CStateManager::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertexIndices, UINT PrimitiveCount, CONST void * pIndexData, D3DFORMAT IndexDataFormat, CONST void * pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	m_CurrentState.m_IndexData = nullptr;
	m_CurrentState.m_StreamData[0] = nullptr;
#if defined(USE_OPENGL_ES)
	if (CGraphicBase::ms_uUIShaderProgram != 0 && pIndexData != nullptr && pVertexStreamZeroData != nullptr)
	{
		SetupUIShaderAndStates(m_CurrentState);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		const char* ptr = (const char*)pVertexStreamZeroData;
		SetEnabledAttribMask((1 << 0) | (1 << 1) | (1 << 2));
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VertexStreamZeroStride, ptr);
		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, VertexStreamZeroStride, ptr + 12);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, VertexStreamZeroStride, ptr + 16);

		GLenum mode = GL_TRIANGLES;
		GLsizei count = PrimitiveCount * 3;
		if (PrimitiveType == D3DPT_LINELIST)
		{
			mode = GL_LINES;
			count = PrimitiveCount * 2;
		}

		glDrawElements(mode, count, GL_UNSIGNED_SHORT, pIndexData);
	}
	return S_OK;
#else
	if (m_lpD3DDev) return (m_lpD3DDev->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertexIndices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride)); return S_OK;
#endif
}

