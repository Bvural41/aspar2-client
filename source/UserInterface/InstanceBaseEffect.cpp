#include "StdAfx.h"
#include "InstanceBase.h"
#include "PythonTextTail.h"
#include "AbstractApplication.h"
#include "AbstractPlayer.h"
#include "PythonPlayer.h"
#include "PythonSystem.h"
#include "PythonNonPlayer.h"

#include "../EffectLib/EffectManager.h"
#include "../EffectLib/ParticleSystemData.h"
#include "../EterLib/Camera.h"
#ifdef ENABLE_BATTLE_ROYALE
#include "PythonBackground.h"
#include "PythonBattleRoyaleManager.h"
#endif
#ifdef ENABLE_TITLE_SYSTEM
#include "PythonTitleManager.h"
#endif

float CInstanceBase::ms_fDustGap;
float CInstanceBase::ms_fHorseDustGap;
DWORD CInstanceBase::ms_adwCRCAffectEffect[CInstanceBase::EFFECT_NUM];
std::string CInstanceBase::ms_astAffectEffectAttachBone[EFFECT_NUM];

#define BYTE_COLOR_TO_D3DX_COLOR(r, g, b) D3DXCOLOR(float(r)/255.0f, float(g)/255.0f, float(b)/255.0f, 1.0f)

/*
D3DXCOLOR CInstanceBase::ms_kD3DXClrPC(0xFFFFD84D);//1.0f, 0.8470f, 0.3f, 1.0f
D3DXCOLOR CInstanceBase::ms_kD3DXClrNPC(0xFF7BE85E);//0.4823f, 0.9098f, 0.3686f, 1.0f
D3DXCOLOR CInstanceBase::ms_kD3DXClrMOB(0xFFEC170a);//0.9254f, 0.0901f, 0.0392f, 1.0f
D3DXCOLOR CInstanceBase::ms_kD3DXClrPVP(0xFF8532D9);
D3DXCOLOR CInstanceBase::ms_kD3DXClrPVPSelf(0xFFEE36DF);
D3DXCOLOR CInstanceBase::ms_kD3DXClrKiller = BYTE_COLOR_TO_D3DX_COLOR(180, 100, 0);
D3DXCOLOR CInstanceBase::ms_kD3DXClrTitle[CInstanceBase::TITLE_MAX_NUM] =
{
	BYTE_COLOR_TO_D3DX_COLOR(  0, 204, 255),
	BYTE_COLOR_TO_D3DX_COLOR(  0, 144, 255),
	BYTE_COLOR_TO_D3DX_COLOR( 92, 110, 255),
	BYTE_COLOR_TO_D3DX_COLOR(155, 155, 255),
	0xFFFFFFFF, // None
	BYTE_COLOR_TO_D3DX_COLOR(207, 117,   0),
	BYTE_COLOR_TO_D3DX_COLOR(235,  83,   0),
	BYTE_COLOR_TO_D3DX_COLOR(227,   0,   0),
	BYTE_COLOR_TO_D3DX_COLOR(255,   0,   0),
};
*/

D3DXCOLOR g_akD3DXClrTitle[CInstanceBase::TITLE_NUM];
D3DXCOLOR g_akD3DXClrName[CInstanceBase::NAMECOLOR_NUM];

std::map<int, std::string> g_TitleNameMap;
std::set<DWORD> g_kSet_dwPVPReadyKey;
std::set<DWORD> g_kSet_dwPVPKey;
std::set<DWORD> g_kSet_dwGVGKey;
std::set<DWORD> g_kSet_dwDUELKey;

#ifdef ENABLE_LEFT_SEAT
std::string g_LeftSeatText;
#endif

bool g_isEmpireNameMode=false;

void  CInstanceBase::SetEmpireNameMode(bool isEnable)
{
	g_isEmpireNameMode=isEnable;

	if (isEnable)
	{
		g_akD3DXClrName[NAMECOLOR_MOB]=g_akD3DXClrName[NAMECOLOR_EMPIRE_MOB];
		g_akD3DXClrName[NAMECOLOR_NPC]=g_akD3DXClrName[NAMECOLOR_EMPIRE_NPC];
		g_akD3DXClrName[NAMECOLOR_PC]=g_akD3DXClrName[NAMECOLOR_NORMAL_PC];

		for (UINT uEmpire=1; uEmpire<EMPIRE_NUM; ++uEmpire)
			g_akD3DXClrName[NAMECOLOR_PC+uEmpire]=g_akD3DXClrName[NAMECOLOR_EMPIRE_PC+uEmpire];
		
	}
	else
	{
		g_akD3DXClrName[NAMECOLOR_MOB]=g_akD3DXClrName[NAMECOLOR_NORMAL_MOB];
		g_akD3DXClrName[NAMECOLOR_NPC]=g_akD3DXClrName[NAMECOLOR_NORMAL_NPC];

		for (UINT uEmpire=0; uEmpire<EMPIRE_NUM; ++uEmpire)
			g_akD3DXClrName[NAMECOLOR_PC+uEmpire]=g_akD3DXClrName[NAMECOLOR_NORMAL_PC];
	}
}

const D3DXCOLOR& CInstanceBase::GetIndexedNameColor(UINT eNameColor)
{
	if (eNameColor>=NAMECOLOR_NUM)
	{
		static D3DXCOLOR s_kD3DXClrNameDefault(0xffffffff);
		return s_kD3DXClrNameDefault;
	}

	return g_akD3DXClrName[eNameColor];
}

void CInstanceBase::AddDamageEffect(DWORD damage,BYTE flag,BOOL bSelf,BOOL bTarget)
{
	if(CPythonSystem::Instance().IsShowDamage())
	{		
		SEffectDamage sDamage;
		sDamage.bSelf = bSelf;
		sDamage.bTarget = bTarget;
		sDamage.damage = damage;
		sDamage.flag = flag;
#ifdef ENABLE_DAMAGE_QUEUE_FIX
		m_DamageQueue.push(sDamage);
		if ( m_DamageQueue.size ( ) > 20 ) 
			m_DamageQueue.pop();
#else
		m_DamageQueue.push_back(sDamage);
#endif
	}
}

void CInstanceBase::ProcessDamage()
{
	if(m_DamageQueue.empty())
		return;

	SEffectDamage sDamage = m_DamageQueue.front();

#ifdef ENABLE_DAMAGE_QUEUE_FIX
	sDamage = m_DamageQueue.front();
	m_DamageQueue.pop();
#else
	sDamage = m_DamageQueue.front();
	m_DamageQueue.pop_front();
#endif

	DWORD damage = sDamage.damage;
	BYTE flag = sDamage.flag;
	BOOL bSelf = sDamage.bSelf;
	BOOL bTarget = sDamage.bTarget;

	CCamera * pCamera = CCameraManager::Instance().GetCurrentCamera();	
	float cameraAngle = GetDegreeFromPosition2(pCamera->GetTarget().x,pCamera->GetTarget().y,pCamera->GetEye().x,pCamera->GetEye().y);

	DWORD FONT_WIDTH = 30;
	
	CEffectManager& rkEftMgr=CEffectManager::Instance();

	D3DXVECTOR3 v3Pos = m_GraphicThingInstance.GetPosition();
	v3Pos.z += float(m_GraphicThingInstance.GetHeight());

	D3DXVECTOR3 v3Rot = D3DXVECTOR3(0.0f, 0.0f, cameraAngle);

	if ( (flag & DAMAGE_DODGE) || (flag & DAMAGE_BLOCK) )
	{
		if(bSelf)
			rkEftMgr.CreateEffect(ms_adwCRCAffectEffect[EFFECT_DAMAGE_MISS],v3Pos,v3Rot);
		else
			rkEftMgr.CreateEffect(ms_adwCRCAffectEffect[EFFECT_DAMAGE_TARGETMISS],v3Pos,v3Rot);
		//__AttachEffect(EFFECT_DAMAGE_MISS);
		return;
	}
	else if (flag & DAMAGE_CRITICAL)
	{
		//rkEftMgr.CreateEffect(ms_adwCRCAffectEffect[EFFECT_DAMAGE_CRITICAL],v3Pos,v3Rot);
		//return;
	}

	string strDamageType;
	DWORD rdwCRCEft = 0;
	{
		if(bSelf)
		{
			strDamageType = "damage_";
			if(m_bDamageEffectType==0)
				rdwCRCEft = EFFECT_DAMAGE_SELFDAMAGE;
			else
				rdwCRCEft = EFFECT_DAMAGE_SELFDAMAGE2;
			m_bDamageEffectType = !m_bDamageEffectType;
		}
#ifdef ENABLE_CRITICAL_VALUE
		else if(bTarget == false)
		{
			strDamageType = "nontarget_";
			rdwCRCEft = EFFECT_DAMAGE_NOT_TARGET;
			return;
		}
		else
		{
			if (CPythonSystem::Instance().IsEnableDamageBar())
				strDamageType = (flag & DAMAGE_CRITICAL) ? "target_crit_" : "target_";
			else
				strDamageType = "target_";
			rdwCRCEft = EFFECT_DAMAGE_TARGET;
		}
#else
		else if(bTarget == false)
		{
			strDamageType = "nontarget_";
			rdwCRCEft = EFFECT_DAMAGE_NOT_TARGET;
			return;
		}
		else
		{
			strDamageType = "target_";
			rdwCRCEft = EFFECT_DAMAGE_TARGET;
		}
#endif
	}
	
	DWORD index = 0;
	DWORD num = 0;
	std::vector<std::string> textures;
#ifdef ENABLE_DAMAGE_BAR
	BYTE idxMultiplier = 0;
	BYTE valIdx = 0;

	if (!CPythonSystem::Instance().IsEnableDamageBar() && !bSelf && bTarget)
	{
		bool bCritical  = (flag & DAMAGE_CRITICAL)  != 0;
		bool bPenetrate = (flag & DAMAGE_PENETRATE) != 0;

		// .mse effect dosyalarini bir kez register et, CRC cachele (RegisterEffect2 ile CRC al)
		static DWORD s_dwCRCCritical    = 0;
		static DWORD s_dwCRCPenetration = 0;
		static DWORD s_dwCRCMix         = 0;
		if (!s_dwCRCCritical)
			rkEftMgr.RegisterEffect2("d:/ymir work/effect/affect/damagevalue/damage_critical.mse",    &s_dwCRCCritical);
		if (!s_dwCRCPenetration)
			rkEftMgr.RegisterEffect2("d:/ymir work/effect/affect/damagevalue/damage_penetration.mse", &s_dwCRCPenetration);
		if (!s_dwCRCMix)
			rkEftMgr.RegisterEffect2("d:/ymir work/effect/affect/damagevalue/damage_mix.mse",         &s_dwCRCMix);

		DWORD dwIconCRC = 0;
		if (bCritical && bPenetrate)  dwIconCRC = s_dwCRCMix;
		else if (bCritical)           dwIconCRC = s_dwCRCCritical;
		else if (bPenetrate)          dwIconCRC = s_dwCRCPenetration;

		if (dwIconCRC)
		{
			DWORD numDigits = 0;
			DWORD tmpDmg = damage;
			do { tmpDmg /= 10; numDigits++; } while (tmpDmg > 0);

			const DWORD iconPos = FONT_WIDTH * numDigits + 20;

			D3DXMATRIX iconMatrix, iconTrans;
			D3DXMatrixIdentity(&iconMatrix);
			D3DXMatrixTranslation(&iconMatrix, v3Pos.x, v3Pos.y, v3Pos.z);
			D3DXMatrixMultiply(&iconMatrix, &pCamera->GetInverseViewMatrix(), &iconMatrix);
			D3DXMatrixTranslation(&iconTrans, (float)iconPos, 0, 0);
			iconTrans._41 = -iconTrans._41;
			iconMatrix = iconTrans * iconMatrix;
			D3DXMatrixMultiply(&iconMatrix, &pCamera->GetViewMatrix(), &iconMatrix);

			rkEftMgr.CreateEffect(dwIconCRC,
				D3DXVECTOR3(iconMatrix._41, iconMatrix._42, iconMatrix._43), v3Rot);
		}
	}
#endif
	while(damage>0)
	{
#ifdef ENABLE_DAMAGE_LIMIT_RENEWAL
		if (index > 35)
#else
		if (index > 7)
#endif
		{
			TraceError("ProcessDamage Possibility of endless loop");
			break;
		}
		num = damage%10;
		damage /= 10;
		char numBuf[MAX_PATH];
		sprintf(numBuf,"%d.dds",num);
		textures.push_back("d:/ymir work/effect/affect/damagevalue/"+strDamageType+numBuf);

		rkEftMgr.SetEffectTextures(ms_adwCRCAffectEffect[rdwCRCEft],textures);

		D3DXMATRIX matrix,matTrans;
		D3DXMatrixIdentity(&matrix);
		matrix._41 = v3Pos.x;
		matrix._42 = v3Pos.y;
		matrix._43 = v3Pos.z;
		D3DXMatrixTranslation(&matrix,v3Pos.x,v3Pos.y,v3Pos.z);
		D3DXMatrixMultiply(&matrix,&pCamera->GetInverseViewMatrix(),&matrix);
#ifdef ENABLE_DAMAGE_BAR
		if (CPythonSystem::Instance().IsEnableDamageBar())
			D3DXMatrixTranslation(&matTrans, FONT_WIDTH*idxMultiplier, 0, 0);
		else
			D3DXMatrixTranslation(&matTrans,FONT_WIDTH*index,0,0);
#else
		D3DXMatrixTranslation(&matTrans,FONT_WIDTH*index,0,0);
#endif
		matTrans._41 = -matTrans._41;
		matrix = matTrans*matrix;
		D3DXMatrixMultiply(&matrix,&pCamera->GetViewMatrix(),&matrix);

		rkEftMgr.CreateEffect(ms_adwCRCAffectEffect[rdwCRCEft],D3DXVECTOR3(matrix._41,matrix._42,matrix._43)
			,v3Rot);

		textures.clear();

		index++;
#ifdef ENABLE_DAMAGE_BAR
		if (CPythonSystem::Instance().IsEnableDamageBar())
		{
			idxMultiplier++;
			if (!bSelf)
			{
				if (damage > 0 && (index % 3) == 0){
					if (flag & DAMAGE_CRITICAL)
					{
						if (valIdx > 0)
						{
							if (valIdx > 1)
								textures.push_back("d:/ymir work/effect/affect/damagevalue/damage_crit_t.dds");
							else
								textures.push_back("d:/ymir work/effect/affect/damagevalue/damage_crit_m.dds");
						}
						else
						{
							textures.push_back("d:/ymir work/effect/affect/damagevalue/damage_crit_k.dds");
						}
					}
					else
					{
						if (valIdx > 0)
						{
							if (valIdx > 1)
								textures.push_back("d:/ymir work/effect/affect/damagevalue/damage_t.dds");
							else
								textures.push_back("d:/ymir work/effect/affect/damagevalue/damage_m.dds");
						}
						else
						{
							textures.push_back("d:/ymir work/effect/affect/damagevalue/damage_k.dds");
						}
					}
					rkEftMgr.SetEffectTextures(ms_adwCRCAffectEffect[rdwCRCEft], textures);
					D3DXMATRIX matrix, matTrans;
					D3DXMatrixIdentity(&matrix);
					matrix._41 = v3Pos.x;
					matrix._42 = v3Pos.y;
					matrix._43 = v3Pos.z;
					D3DXMatrixTranslation(&matrix, v3Pos.x, v3Pos.y, v3Pos.z);
					D3DXMatrixMultiply(&matrix, &pCamera->GetInverseViewMatrix(), &matrix);
					D3DXMatrixTranslation(&matTrans, FONT_WIDTH*idxMultiplier, 0, 0);
					matTrans._41 = -matTrans._41;
					matrix = matTrans*matrix;
					D3DXMatrixMultiply(&matrix, &pCamera->GetViewMatrix(), &matrix);
					rkEftMgr.CreateEffect(ms_adwCRCAffectEffect[rdwCRCEft], D3DXVECTOR3(matrix._41, matrix._42, matrix._43), v3Rot);
					textures.clear();
					idxMultiplier++;
					if (!bSelf)
						valIdx++;
				}
			}
			else
			{
				if (damage > 0 && (index % 3) == 0){
					{
						if (valIdx > 0)
						{
							if (valIdx > 1)
								textures.push_back("d:/ymir work/effect/affect/damagevalue/damage_you_t.dds");
							else
								textures.push_back("d:/ymir work/effect/affect/damagevalue/damage_you_m.dds");
						}
						else
						{
							textures.push_back("d:/ymir work/effect/affect/damagevalue/damage_you_k.dds");
						}
					}
					rkEftMgr.SetEffectTextures(ms_adwCRCAffectEffect[rdwCRCEft], textures);
					D3DXMATRIX matrix, matTrans;
					D3DXMatrixIdentity(&matrix);
					matrix._41 = v3Pos.x;
					matrix._42 = v3Pos.y;
					matrix._43 = v3Pos.z;
					D3DXMatrixTranslation(&matrix, v3Pos.x, v3Pos.y, v3Pos.z);
					D3DXMatrixMultiply(&matrix, &pCamera->GetInverseViewMatrix(), &matrix);
					D3DXMatrixTranslation(&matTrans, FONT_WIDTH*idxMultiplier, 0, 0);
					matTrans._41 = -matTrans._41;
					matrix = matTrans*matrix;
					D3DXMatrixMultiply(&matrix, &pCamera->GetViewMatrix(), &matrix);
					rkEftMgr.CreateEffect(ms_adwCRCAffectEffect[rdwCRCEft], D3DXVECTOR3(matrix._41, matrix._42, matrix._43), v3Rot);
					textures.clear();
					idxMultiplier++;
					if (!bSelf)
						valIdx++;
				}
			}
		}
#endif
	}
}

void CInstanceBase::AttachSpecialEffect(DWORD effect)
{
	__AttachEffect(effect);
}

#ifdef ENABLE_12ZI
void CInstanceBase::AttachSpecialZodiacEffect(UINT eEftType, long GetX, long GetY)
{
	const D3DXMATRIX & c_rmatGlobal = m_GraphicThingInstance.GetTransform();

	D3DXMATRIX matrix;
	D3DXMatrixIdentity(&matrix);
	matrix._41 = float(GetX);
	matrix._42 = -float(GetY);
	matrix._43 = c_rmatGlobal._43;

	DWORD dwEffectCRC = ms_adwCRCAffectEffect[eEftType];
	DWORD dwEffectIndex = CEffectManager::Instance().GetEmptyIndex();
	CEffectManager::Instance().CreateEffectInstance(dwEffectIndex, dwEffectCRC);
	CEffectManager::Instance().SelectEffectInstance(dwEffectIndex);
	CEffectManager::Instance().SetEffectInstanceGlobalMatrix(matrix);
}
#endif

void CInstanceBase::LevelUp()
{
	__AttachEffect(EFFECT_LEVELUP);
}

void CInstanceBase::SkillUp()
{
	__AttachEffect(EFFECT_SKILLUP);
}

void CInstanceBase::CreateSpecialEffect(DWORD iEffectIndex)
{
	D3DXMATRIX matGlobal = m_GraphicThingInstance.GetTransform();

	DWORD dwEffectIndex = CEffectManager::Instance().GetEmptyIndex();
	DWORD dwEffectCRC = ms_adwCRCAffectEffect[iEffectIndex];

	CEffectManager::Instance().CreateEffectInstance(dwEffectIndex, dwEffectCRC);
	CEffectManager::Instance().SelectEffectInstance(dwEffectIndex);
	CEffectManager::Instance().SetEffectInstanceGlobalMatrix(matGlobal);
}


#ifdef ENABLE_GRAPHIC_ON_OFF
void CInstanceBase::__EffectContainer_Continue()
{
	SEffectContainer::Dict& rkDctEftID = __EffectContainer_GetDict();

	SEffectContainer::Dict::iterator i;
	for (i = rkDctEftID.begin(); i != rkDctEftID.end(); ++i)
		__AttachEffect(i->second);
}

void CInstanceBase::__EffectContainer_Suspend()
{
	SEffectContainer::Dict& rkDctEftID = __EffectContainer_GetDict();

	SEffectContainer::Dict::iterator i;
	for (i = rkDctEftID.begin(); i != rkDctEftID.end(); ++i)
		__DetachEffect(i->second);
}
#endif

void CInstanceBase::__EffectContainer_Destroy()
{
	SEffectContainer::Dict& rkDctEftID=__EffectContainer_GetDict();

	SEffectContainer::Dict::iterator i;
	for (i=rkDctEftID.begin(); i!=rkDctEftID.end(); ++i)
		__DetachEffect(i->second);

	rkDctEftID.clear();
}

void CInstanceBase::__EffectContainer_Initialize()
{
	SEffectContainer::Dict& rkDctEftID=__EffectContainer_GetDict();
	rkDctEftID.clear();	
}

CInstanceBase::SEffectContainer::Dict& CInstanceBase::__EffectContainer_GetDict()
{
	return m_kEffectContainer.m_kDct_dwEftID;
}

// Return value �� boolean ���� ID �� �ٲߴϴ�
DWORD CInstanceBase::__EffectContainer_AttachEffect(DWORD dwEftKey)
{
	SEffectContainer::Dict& rkDctEftID=__EffectContainer_GetDict();
	SEffectContainer::Dict::iterator f=rkDctEftID.find(dwEftKey);
	if (rkDctEftID.end()!=f)
		return 0;

	DWORD dwEftID=__AttachEffect(dwEftKey);
	rkDctEftID.insert(SEffectContainer::Dict::value_type(dwEftKey, dwEftID));
	return dwEftID;
}


void CInstanceBase::__EffectContainer_DetachEffect(DWORD dwEftKey)
{
	SEffectContainer::Dict& rkDctEftID=__EffectContainer_GetDict();
	SEffectContainer::Dict::iterator f=rkDctEftID.find(dwEftKey);
	if (rkDctEftID.end()==f)
		return;

	__DetachEffect(f->second);

	rkDctEftID.erase(f);
}

void CInstanceBase::__AttachEmpireEffect(DWORD eEmpire)
{
	if (!__IsExistMainInstance())
		return;

#ifdef ENABLE_BATTLE_ROYALE
	if (CPythonBackground::Instance().IsBattleRoyaleMap() && PythonBattleRoyaleManager::Instance().IsPCTextTailHided())
		return;
#endif

	CInstanceBase* pkInstMain=__GetMainInstancePtr();

	if (IsWarp())
		return;
	if (IsObject())
		return;
	if (IsFlag())
		return;
	if (IsResource())
		return;
	if (IsNPC())
		return;
	if (IsPet())
		return;
#ifdef ENABLE_BATTLE_FIELD
	if (IsCombatZoneMap())
		return;
#endif
#ifdef ENABLE_EVENT_BANNER_FLAG
	if (IsBannerFlag())
		return;
#endif
	if (pkInstMain->IsGameMaster())
	{
	}
	else
	{
		if (pkInstMain->IsSameEmpire(*this))
			return;

		// HIDE_OTHER_EMPIRE_EUNHYEONG_ASSASSIN
		if (IsAffect(AFFECT_EUNHYEONG))
			return;
		// END_OF_HIDE_OTHER_EMPIRE_EUNHYEONG_ASSASSIN
	}

	if (IsGameMaster())
		return;

	__EffectContainer_AttachEffect(EFFECT_EMPIRE+eEmpire);
}

void CInstanceBase::__AttachSelectEffect()
{
	__EffectContainer_AttachEffect(EFFECT_SELECT);
}

void CInstanceBase::__DetachSelectEffect()
{
	__EffectContainer_DetachEffect(EFFECT_SELECT);
}

void CInstanceBase::__AttachTargetEffect()
{
	__EffectContainer_AttachEffect(EFFECT_TARGET);
}

void CInstanceBase::__DetachTargetEffect()
{
	__EffectContainer_DetachEffect(EFFECT_TARGET);
}


void CInstanceBase::__StoneSmoke_Inialize()
{
	m_kStoneSmoke.m_dwEftID=0;
}

void CInstanceBase::__StoneSmoke_Destroy()
{
	if (!m_kStoneSmoke.m_dwEftID)
		return;

	__DetachEffect(m_kStoneSmoke.m_dwEftID);
	m_kStoneSmoke.m_dwEftID=0;
}

void CInstanceBase::__StoneSmoke_Create(DWORD eSmoke)
{
	m_kStoneSmoke.m_dwEftID=m_GraphicThingInstance.AttachSmokeEffect(eSmoke);
}

void CInstanceBase::SetAlpha(float fAlpha)
{
	__SetBlendRenderingMode();
	__SetAlphaValue(fAlpha);
}

bool CInstanceBase::UpdateDeleting()
{
	Update();
	Transform();

	IAbstractApplication& rApp=IAbstractApplication::GetSingleton();

	float fAlpha = __GetAlphaValue() - (rApp.GetGlobalElapsedTime() * 1.5f);
	__SetAlphaValue(fAlpha);

	if (fAlpha < 0.0f)
		return false;

	return true;
}

void CInstanceBase::DeleteBlendOut()
{
	__SetBlendRenderingMode();
	__SetAlphaValue(1.0f);
	DetachTextTail();

	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	rkPlayer.NotifyDeletingCharacterInstance(GetVirtualID());
}

void CInstanceBase::ClearPVPKeySystem()
{
	g_kSet_dwPVPReadyKey.clear();
	g_kSet_dwPVPKey.clear();
	g_kSet_dwGVGKey.clear();
	g_kSet_dwDUELKey.clear();
}

void CInstanceBase::InsertPVPKey(DWORD dwVIDSrc, DWORD dwVIDDst)
{
	DWORD dwPVPKey=__GetPVPKey(dwVIDSrc, dwVIDDst);

	g_kSet_dwPVPKey.insert(dwPVPKey);
}

void CInstanceBase::InsertPVPReadyKey(DWORD dwVIDSrc, DWORD dwVIDDst)
{
	DWORD dwPVPReadyKey=__GetPVPKey(dwVIDSrc, dwVIDDst);

	g_kSet_dwPVPKey.insert(dwPVPReadyKey);
}

void CInstanceBase::RemovePVPKey(DWORD dwVIDSrc, DWORD dwVIDDst)
{
	DWORD dwPVPKey=__GetPVPKey(dwVIDSrc, dwVIDDst);

	g_kSet_dwPVPKey.erase(dwPVPKey);
}

void CInstanceBase::InsertGVGKey(DWORD dwSrcGuildVID, DWORD dwDstGuildVID)
{
	DWORD dwGVGKey = __GetPVPKey(dwSrcGuildVID, dwDstGuildVID);
	g_kSet_dwGVGKey.insert(dwGVGKey);
}

void CInstanceBase::RemoveGVGKey(DWORD dwSrcGuildVID, DWORD dwDstGuildVID)
{
	DWORD dwGVGKey = __GetPVPKey(dwSrcGuildVID, dwDstGuildVID);
	g_kSet_dwGVGKey.erase(dwGVGKey);
}

void CInstanceBase::InsertDUELKey(DWORD dwVIDSrc, DWORD dwVIDDst)
{
	DWORD dwPVPKey=__GetPVPKey(dwVIDSrc, dwVIDDst);

	g_kSet_dwDUELKey.insert(dwPVPKey);
}

DWORD CInstanceBase::__GetPVPKey(DWORD dwVIDSrc, DWORD dwVIDDst)
{
	if (dwVIDSrc>dwVIDDst)
		std::swap(dwVIDSrc, dwVIDDst);

	DWORD awSrc[2];
	awSrc[0]=dwVIDSrc;
	awSrc[1]=dwVIDDst;

    const BYTE * s = (const BYTE *) awSrc;
    const BYTE * end = s + sizeof(awSrc);
    unsigned long h = 0;

    while (s < end)
    {
        h *= 16777619;
        h ^= (BYTE) *(BYTE *) (s++);
    }

    return h;
}

bool CInstanceBase::__FindPVPKey(DWORD dwVIDSrc, DWORD dwVIDDst)
{
	DWORD dwPVPKey=__GetPVPKey(dwVIDSrc, dwVIDDst);

	if (g_kSet_dwPVPKey.end()==g_kSet_dwPVPKey.find(dwPVPKey))
		return false;

	return true;
}

bool CInstanceBase::__FindPVPReadyKey(DWORD dwVIDSrc, DWORD dwVIDDst)
{
	DWORD dwPVPKey=__GetPVPKey(dwVIDSrc, dwVIDDst);

	if (g_kSet_dwPVPReadyKey.end()==g_kSet_dwPVPReadyKey.find(dwPVPKey))
		return false;

	return true;
}
//������� ��� ������� Ȯ���Ҷ�.
bool CInstanceBase::__FindGVGKey(DWORD dwSrcGuildID, DWORD dwDstGuildID)
{
	DWORD dwGVGKey=__GetPVPKey(dwSrcGuildID, dwDstGuildID);

	if (g_kSet_dwGVGKey.end()==g_kSet_dwGVGKey.find(dwGVGKey))
		return false;

	return true;
}
//��� ��忡���� ��� ��븸 ������ �� �ִ�.
bool CInstanceBase::__FindDUELKey(DWORD dwVIDSrc, DWORD dwVIDDst)
{
	DWORD dwDUELKey=__GetPVPKey(dwVIDSrc, dwVIDDst);

	if (g_kSet_dwDUELKey.end()==g_kSet_dwDUELKey.find(dwDUELKey))
		return false;

	return true;
}

bool CInstanceBase::IsPVPInstance(CInstanceBase& rkInstSel)
{
	DWORD dwVIDSrc=GetVirtualID();
	DWORD dwVIDDst=rkInstSel.GetVirtualID();

	DWORD dwGuildIDSrc=GetGuildID();
	DWORD dwGuildIDDst=rkInstSel.GetGuildID();

	if (GetDuelMode())	//��� ����϶��� ~_~
		return true;	

	return __FindPVPKey(dwVIDSrc, dwVIDDst) || __FindGVGKey(dwGuildIDSrc, dwGuildIDDst);
											//__FindDUELKey(dwVIDSrc, dwVIDDst);
}

const D3DXCOLOR& CInstanceBase::GetNameColor()
{
#ifdef ENABLE_AUTO_SYSTEM
	if (IsAutoTarget())
	{
		static D3DXCOLOR s_kD3DXClrAutoTarget(0xFF00FF00);
		return s_kD3DXClrAutoTarget;
	}
#endif

	return GetIndexedNameColor(GetNameColorIndex());
}

UINT CInstanceBase::GetNameColorIndex()
{
	if (IsPC())
	{

#ifdef ENABLE_BATTLE_FIELD
		if (IsCombatZoneMap())
			return NAMECOLOR_PVP;
#endif

		if (m_isKiller)
		{
			return NAMECOLOR_PK;
		}

		if (__IsExistMainInstance() && !__IsMainInstance())
		{			
			CInstanceBase* pkInstMain=__GetMainInstancePtr();
			if (!pkInstMain)
			{
				TraceError("CInstanceBase::GetNameColorIndex - MainInstance is NULL");
				return NAMECOLOR_PC;
			}
			DWORD dwVIDMain=pkInstMain->GetVirtualID();
			DWORD dwVIDSelf=GetVirtualID();

			if (pkInstMain->GetDuelMode())
			{
				switch(pkInstMain->GetDuelMode())
				{
				case DUEL_CANNOTATTACK:
					return NAMECOLOR_PC + GetEmpireID();
				case DUEL_START:
					if(__FindDUELKey(dwVIDMain, dwVIDSelf))
						return NAMECOLOR_PVP;
					else
						return NAMECOLOR_PC + GetEmpireID();
				}
			}

			if (pkInstMain->IsSameEmpire(*this))
			{
				if (__FindPVPKey(dwVIDMain, dwVIDSelf))
				{
					return NAMECOLOR_PVP;
				}

				DWORD dwGuildIDMain=pkInstMain->GetGuildID();
				DWORD dwGuildIDSelf=GetGuildID();
				if (__FindGVGKey(dwGuildIDMain, dwGuildIDSelf))
				{
					return NAMECOLOR_PVP;
				}
				/*
				if (__FindDUELKey(dwVIDMain, dwVIDSelf))
				{
					return NAMECOLOR_PVP;
				}
				*/
			}
			else
			{
				return NAMECOLOR_PVP;
			}
		}

		IAbstractPlayer& rPlayer=IAbstractPlayer::GetSingleton();
		if (rPlayer.IsPartyMemberByVID(GetVirtualID()))
			return NAMECOLOR_PARTY;

		return NAMECOLOR_PC + GetEmpireID();
		
	}
	else if (IsNPC())
	{
#ifdef ENABLE_OFFLINESHOP_SYSTEM
		if (IsShop())
			return NAMECOLOR_SHOP;
#endif
		return NAMECOLOR_NPC;
	}
	else if (IsEnemy())
	{
		return NAMECOLOR_MOB;
	}
	else if (IsPoly())
	{
		return NAMECOLOR_MOB;
	}
	else if (IsStone())
	{
		return NAMECOLOR_METIN;
	}
#ifdef ENABLE_SUPPORT_SYSTEM
	else if (IsSupportShaman())
	{
		return NAMECOLOR_PC;
	}
#endif
	return D3DXCOLOR(0xffffffff);
}

const D3DXCOLOR& CInstanceBase::GetTitleColor()
{
#ifdef ENABLE_AUTO_SYSTEM
	if (IsAutoTarget())
	{
		static D3DXCOLOR s_kD3DXClrAutoTarget(0xFF00FF00);
		return s_kD3DXClrAutoTarget;
	}
#endif

	UINT uGrade = GetAlignmentGrade();
	if ( uGrade >= TITLE_NUM)
	{
		static D3DXCOLOR s_kD3DXClrTitleDefault(0xffffffff);
		return s_kD3DXClrTitleDefault;
	}

	return g_akD3DXClrTitle[uGrade];
}

void CInstanceBase::AttachTextTail()
{
	if (m_isTextTail)
	{
		TraceError("CInstanceBase::AttachTextTail - VID [%d] ALREADY EXIST", GetVirtualID());
		return;
	}
		//Bvural41 Fix

	m_isTextTail=true;

	DWORD dwVID=GetVirtualID();

	float fTextTailHeight = IsMountingHorse() ? 110.0f : 10.0f;

	static D3DXCOLOR s_kD3DXClrTextTail=D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	CPythonTextTail::Instance().RegisterCharacterTextTail(m_dwGuildID, dwVID, s_kD3DXClrTextTail, fTextTailHeight);

#ifdef ENABLE_TITLE_SYSTEM
	CPythonTitleManager::Instance().RefreshTitleByVID(dwVID);
#endif

	// CHARACTER_LEVEL
#ifdef ENABLE_CONQUEROR_LEVEL
	if (m_dwConquerorLevel)
	{
		UpdateTextTailConquerorLevel(m_dwConquerorLevel);
	}
	else if (m_dwLevel)
	{
		UpdateTextTailLevel(m_dwLevel);
	}
#else
	if (m_dwLevel)
	{
		UpdateTextTailLevel(m_dwLevel);
	}
#endif

#ifdef ENABLE_LEFT_SEAT
	if (m_bLeftSeat)
		UpdateTextTailLeftSeat(g_LeftSeatText);
#endif
}

void CInstanceBase::DetachTextTail()
{
	if (!m_isTextTail)
		return;

	m_isTextTail=false;
	CPythonTextTail::Instance().DeleteCharacterTextTail(GetVirtualID());
}

void CInstanceBase::UpdateTextTailLevel(DWORD level)
{
#ifdef ENABLE_BATTLE_ROYALE
	if (CPythonBackground::Instance().IsBattleRoyaleMap() && PythonBattleRoyaleManager::Instance().IsPCTextTailHided() && IsPC())
		return;
#endif

	static D3DXCOLOR s_kPlayerLevelColor = D3DXCOLOR(152.0f/255.0f, 255.0f/255.0f, 51.0f/255.0f, 1.0f);
#ifdef ENABLE_SHOW_MOB_INFO
	static D3DXCOLOR s_kMobLevelColor = D3DXCOLOR(119.0f/255.0f, 246.0f/255.0f, 168.0f/255.0f, 1.0f);
#endif

	char szText[256];
	sprintf(szText, "Lv %d ", level);

	if (IsPC())
	{
		CPythonTextTail::Instance().AttachLevel(GetVirtualID(), szText, s_kPlayerLevelColor);
	}
#ifdef ENABLE_SHOW_MOB_INFO
	else if (IsEnemy() || IsStone())
	{
		sprintf(szText, "Lv %d ", level);
		CPythonTextTail::Instance().AttachLevel(GetVirtualID(), szText, s_kMobLevelColor);
	}
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
	else if (IsGrowthPet()) {
		sprintf(szText, "Lv %d ", level);
		CPythonTextTail::Instance().AttachLevel(GetVirtualID(), szText, s_kPlayerLevelColor);
	}
#endif
}

#ifdef ENABLE_CONQUEROR_LEVEL
void CInstanceBase::UpdateTextTailConquerorLevel(DWORD level)
{
	char szText[256];
	sprintf(szText, "Lv %d ", level);
	CPythonTextTail::Instance().AttachConquerorLevel(GetVirtualID(), szText);
}
#endif

#ifdef ENABLE_LEFT_SEAT
void CInstanceBase::UpdateTextTailLeftSeat(const std::string& c_rstrText)
{
	CPythonTextTail::Instance().AttachLeftSeatText(GetVirtualID(), c_rstrText);
}
#endif

void CInstanceBase::RefreshTextTail()
{
	CPythonTextTail::Instance().SetCharacterTextTailColor(GetVirtualID(), GetNameColor());

	int iAlignmentGrade = GetAlignmentGrade();
	if (TITLE_NONE == iAlignmentGrade)
	{
		CPythonTextTail::Instance().DetachTitle(GetVirtualID());
	}
	else
	{
		std::map<int, std::string>::iterator itor = g_TitleNameMap.find(iAlignmentGrade);
		if (g_TitleNameMap.end() != itor)
		{
			const std::string & c_rstrTitleName = itor->second;
			CPythonTextTail::Instance().AttachTitle(GetVirtualID(), c_rstrTitleName.c_str(), GetTitleColor());
		}
	}
}

void CInstanceBase::RefreshTextTailTitle()
{
	RefreshTextTail();
}

// 2004.07.25.myevan.����Ʈ �� �ٴ� ���� �ذ�
/////////////////////////////////////////////////
void CInstanceBase::__ClearAffectFlagContainer()
{
	m_kAffectFlagContainer.Clear();
}

void CInstanceBase::__ClearAffects()
{
#ifdef ENABLE_MELEY_LAIR_DUNGEON
	if ((IsStone()) && (GetVirtualNumber() != MELEY_LAIR_DUNGEON_STATUE))
#else
	if (IsStone())
#endif
	{
		__StoneSmoke_Destroy();
	}
	else
	{
		for (int iAffect=0; iAffect<AFFECT_NUM; ++iAffect)
		{
			__DetachEffect(m_adwCRCAffectEffect[iAffect]);
			m_adwCRCAffectEffect[iAffect]=0;
		}

		__ClearAffectFlagContainer();
	}

	m_GraphicThingInstance.__OnClearAffects();
}

/////////////////////////////////////////////////

void CInstanceBase::__SetNormalAffectFlagContainer(const CAffectFlagContainer& c_rkAffectFlagContainer)
{
	for (int i=0; i<CAffectFlagContainer::BIT_SIZE; ++i)
	{
		bool isOldSet=m_kAffectFlagContainer.IsSet(i);
		bool isNewSet=c_rkAffectFlagContainer.IsSet(i);

		if (isOldSet != isNewSet)
		{
			__SetAffect(i, isNewSet);

			if (isNewSet)
				m_GraphicThingInstance.__OnSetAffect(i);
			else
				m_GraphicThingInstance.__OnResetAffect(i);
		}
	}

	m_kAffectFlagContainer.CopyInstance(c_rkAffectFlagContainer);
}

void CInstanceBase::__SetStoneSmokeFlagContainer(const CAffectFlagContainer& c_rkAffectFlagContainer)
{
	m_kAffectFlagContainer.CopyInstance(c_rkAffectFlagContainer);

	DWORD eSmoke;
	if (m_kAffectFlagContainer.IsSet(STONE_SMOKE8))
		eSmoke=3;
	else if (m_kAffectFlagContainer.IsSet(STONE_SMOKE5)|m_kAffectFlagContainer.IsSet(STONE_SMOKE6)|m_kAffectFlagContainer.IsSet(STONE_SMOKE7))
		eSmoke=2;
	else if (m_kAffectFlagContainer.IsSet(STONE_SMOKE2)|m_kAffectFlagContainer.IsSet(STONE_SMOKE3)|m_kAffectFlagContainer.IsSet(STONE_SMOKE4))
		eSmoke=1;
	else
		eSmoke=0;

	__StoneSmoke_Destroy();
	__StoneSmoke_Create(eSmoke);
}

void CInstanceBase::SetAffectFlagContainer(const CAffectFlagContainer& c_rkAffectFlagContainer)
{
	if (IsBuilding())
	{
		return;		
	}
#ifdef ENABLE_MELEY_LAIR_DUNGEON
	else if ((IsStone()) && (GetVirtualNumber() != MELEY_LAIR_DUNGEON_STATUE))
#else
	else if (IsStone())
#endif
	{
		__SetStoneSmokeFlagContainer(c_rkAffectFlagContainer);
	}
	else
	{
		__SetNormalAffectFlagContainer(c_rkAffectFlagContainer);
	}
}


void CInstanceBase::SCRIPT_SetAffect(UINT eAffect, bool isVisible)
{
	__SetAffect(eAffect, isVisible);
}

void CInstanceBase::__SetReviveInvisibilityAffect(bool isVisible)
{
	if (isVisible)
	{
		// NOTE : Dress �� �԰� ������ Alpha �� ���� �ʴ´�.
		if (IsWearingDress())
			return;

		m_GraphicThingInstance.BlendAlphaValue(0.5f, 1.0f);
	}
	else
	{
		m_GraphicThingInstance.BlendAlphaValue(1.0f, 1.0f);	
	}
}

void CInstanceBase::__Assassin_SetEunhyeongAffect(bool isVisible)
{
	if (isVisible)
	{
		// NOTE : Dress �� �԰� ������ Alpha �� ���� �ʴ´�.
		if (IsWearingDress())
			return;

		if (__IsMainInstance() || __MainCanSeeHiddenThing())
		{
			m_GraphicThingInstance.BlendAlphaValue(0.5f, 1.0f);
		}
		else
		{
			// 2004.10.16.myevan.������ ���� ����
			m_GraphicThingInstance.BlendAlphaValue(0.0f, 1.0f);
#if defined(ENABLE_GRAPHIC_ON_OFF) || defined(ENABLE_EFFECT_CAMERA_VIEW_FIX)
			m_GraphicThingInstance.SetDeactiveAllAttachingEffect();
			// __EffectContainer_Suspend();
#else
			m_GraphicThingInstance.HideAllAttachingEffect();
#endif
		}
	}
	else
	{
		m_GraphicThingInstance.BlendAlphaValue(1.0f, 1.0f);
#if defined(ENABLE_GRAPHIC_ON_OFF) || defined(ENABLE_EFFECT_CAMERA_VIEW_FIX)
		m_GraphicThingInstance.SetActiveAllAttachingEffect();
		// __EffectContainer_Continue();
#else
		m_GraphicThingInstance.ShowAllAttachingEffect();
#endif
	}
}

void CInstanceBase::__Shaman_SetParalysis(bool isParalysis)
{
	m_GraphicThingInstance.SetParalysis(isParalysis);
}



void CInstanceBase::__Warrior_SetGeomgyeongAffect(bool isVisible)
{
	if (isVisible)
	{
		if (IsWearingDress())
			return;

		if (m_kWarrior.m_dwGeomgyeongEffect)
			__DetachEffect(m_kWarrior.m_dwGeomgyeongEffect);

		m_GraphicThingInstance.SetReachScale(1.5f);
		if (m_GraphicThingInstance.IsTwoHandMode())
			m_kWarrior.m_dwGeomgyeongEffect=__AttachEffect(EFFECT_WEAPON+WEAPON_TWOHAND);
		else
			m_kWarrior.m_dwGeomgyeongEffect=__AttachEffect(EFFECT_WEAPON+WEAPON_ONEHAND);
	}
	else
	{
		m_GraphicThingInstance.SetReachScale(1.0f);

		__DetachEffect(m_kWarrior.m_dwGeomgyeongEffect);
		m_kWarrior.m_dwGeomgyeongEffect=0;
	}
}

#ifdef ENABLE_NEW_GYEONGGONG_SKILL
void CInstanceBase::__Assassin_SetGyeongGongAffect(bool isVisible)
{
	if (isVisible)
	{
		if (IsWearingDress())
			return;

		if (m_kAssassin.m_dwGyeongGongEffect)
			__DetachEffect(m_kAssassin.m_dwGyeongGongEffect);

		m_GraphicThingInstance.SetReachScale(1.5f);
		m_kAssassin.m_dwGyeongGongEffect = __AttachEffect(EFFECT_GYEONGGONG_BOOM);
	}
	else
	{
		m_GraphicThingInstance.SetReachScale(1.0f);

		__DetachEffect(m_kAssassin.m_dwGyeongGongEffect);
		m_kAssassin.m_dwGyeongGongEffect = 0;
	}
}
#endif

void CInstanceBase::__SetAffect(UINT eAffect, bool isVisible)
{
	switch (eAffect)
	{
		case AFFECT_YMIR:
			if (IsAffect(AFFECT_INVISIBILITY))
				return;
			break;
/*
		case AFFECT_GWIGEOM: // ���� �Ӽ� �������� �ٲ� ����
			if (isVisible)
			{
				m_GraphicThingInstance.SetBattleHitEffect(ms_adwCRCAffectEffect[EFFECT_ELECTRIC_HIT]);
				m_GraphicThingInstance.SetBattleAttachEffect(ms_adwCRCAffectEffect[EFFECT_ELECTRIC_ATTACH]);
			}
			else
			{
				m_GraphicThingInstance.SetBattleHitEffect(ms_adwCRCAffectEffect[EFFECT_HIT]);
				m_GraphicThingInstance.SetBattleAttachEffect(0);
			}
			return;
			break;
		case AFFECT_HWAYEOM: // ȭ�� �Ӽ� �������� �ٲ� ����
			if (isVisible)
			{
				m_GraphicThingInstance.SetBattleHitEffect(ms_adwCRCAffectEffect[EFFECT_FLAME_HIT]);
				m_GraphicThingInstance.SetBattleAttachEffect(ms_adwCRCAffectEffect[EFFECT_FLAME_ATTACH]);
			}
			else
			{
				m_GraphicThingInstance.SetBattleHitEffect(ms_adwCRCAffectEffect[EFFECT_HIT]);
				m_GraphicThingInstance.SetBattleAttachEffect(0);
			}
			// ȭ������ ������ ���� �Ͻ������� Visible �մϴ�.
			return;
			break;
*/
		case AFFECT_CHEONGEUN:
			m_GraphicThingInstance.SetResistFallen(isVisible);
			break;
		// case AFFECT_JEOKRANG:
		// 	m_GraphicThingInstance.SetResistFallen(isVisible);
		// 	break;
		case AFFECT_GEOMGYEONG:
			__Warrior_SetGeomgyeongAffect(isVisible);
			return;
			break;
		case AFFECT_REVIVE_INVISIBILITY:
			__Assassin_SetEunhyeongAffect(isVisible);
			break;
		case AFFECT_EUNHYEONG:
			__Assassin_SetEunhyeongAffect(isVisible);
			break;
#ifdef ENABLE_NEW_GYEONGGONG_SKILL
		case AFFECT_GYEONGGONG:
			if (isVisible)
			{
				__Assassin_SetGyeongGongAffect(isVisible);
				if (!IsWalking())
					return;
			}
			break;
		case AFFECT_KWAESOK:
			if (isVisible)
				if (!IsWalking())
					return;
			break;
#else
		case AFFECT_GYEONGGONG:
		case AFFECT_KWAESOK:
			if (isVisible)
				if (!IsWalking())
					return;
			break;
#endif

#ifdef ENABLE_NINJA_MAP_FIX
		case AFFECT_INVISIBILITY:
			if (isVisible)
			{
				// m_GraphicThingInstance.ClearAttachingEffect();
				// __EffectContainer_Destroy();
				// DetachTextTail();
				m_GraphicThingInstance.HideAllAttachingEffect();
			}
			else
			{
				m_GraphicThingInstance.BlendAlphaValue(1.0f, 1.0f);
				m_GraphicThingInstance.ShowAllAttachingEffect();

				// AttachTextTail();
				// RefreshTextTail();
			}
			return;
			break;
#else
		case AFFECT_INVISIBILITY:
			if (isVisible)
			{
				m_GraphicThingInstance.ClearAttachingEffect();
				__EffectContainer_Destroy();
				DetachTextTail();
			}
			else
			{
				m_GraphicThingInstance.BlendAlphaValue(1.0f, 1.0f);
				AttachTextTail();
				RefreshTextTail();
			}
			return;
			break;
#endif

//		case AFFECT_FAINT:
//			m_GraphicThingInstance.SetFaint(isVisible);
//			break;
//		case AFFECT_SLEEP:
//			m_GraphicThingInstance.SetSleep(isVisible);
//			break;
		case AFFECT_STUN:
			m_GraphicThingInstance.SetSleep(isVisible);
			break;

#ifdef ENABLE_GREEDY_ROOM
		case AFFECT_AAMON_DEFENCE:
			if (GetRace() == 6939)
			{
				if (isVisible)
				{
					m_GraphicThingInstance.SetOverrideLoopMotion(40);
					SetLoopMotion(40);
				}
				else
				{
					m_GraphicThingInstance.SetOverrideLoopMotion(0);
					SetLoopMotion(CRaceMotionData::NAME_WAIT);
				}
			}
			break;

		case AFFECT_AAMON_LASTSTAND:
			if (GetRace() == 6939)
			{
				if (isVisible)
				{
					m_GraphicThingInstance.SetOverrideLoopMotion(CRaceMotionData::NAME_ETC_5);
					SetLoopMotion(CRaceMotionData::NAME_ETC_5);
				}
				else
				{
					m_GraphicThingInstance.SetOverrideLoopMotion(0);
					SetLoopMotion(CRaceMotionData::NAME_WAIT);
				}
			}
			break;

		case AFFECT_AAMON_LASTSTAND_PC:
			if (isVisible)
			{
				DWORD dwCRC = 0;
				CEffectManager::Instance().RegisterEffect2("d:/ymir work/effect/monster2/greedy_room_pc_board.mse", &dwCRC, false);
				m_adwCRCAffectEffect[AFFECT_AAMON_LASTSTAND_PC] = m_GraphicThingInstance.AttachEffectByID(0, NULL, dwCRC);
			}
			else
			{
				__DetachEffect(m_adwCRCAffectEffect[AFFECT_AAMON_LASTSTAND_PC]);
				m_adwCRCAffectEffect[AFFECT_AAMON_LASTSTAND_PC] = 0;
			}
			break;
#endif

	}

	if (eAffect>=AFFECT_NUM)
	{
		TraceError("CInstanceBase[VID:%d]::SetAffect(eAffect:%d<AFFECT_NUM:%d, isVisible=%d)", GetVirtualID(), eAffect, isVisible);
		return;
	}

	if (isVisible)
	{
		if (!m_adwCRCAffectEffect[eAffect])
		{
			m_adwCRCAffectEffect[eAffect]=__AttachEffect(EFFECT_AFFECT+eAffect);
		}
	}
	else
	{
		if (m_adwCRCAffectEffect[eAffect])
		{
			__DetachEffect(m_adwCRCAffectEffect[eAffect]);
			m_adwCRCAffectEffect[eAffect]=0;
		}
	}
}

bool CInstanceBase::IsPossibleEmoticon()
{
	CEffectManager& rkEftMgr=CEffectManager::Instance();
	for(DWORD eEmoticon = 0; eEmoticon < EMOTICON_NUM; eEmoticon++)
	{
		DWORD effectID = ms_adwCRCAffectEffect[EFFECT_EMOTICON+eEmoticon];
		if( effectID &&	rkEftMgr.IsAliveEffect(effectID) )
			return false;
	}

	if(ELTimer_GetMSec() - m_dwEmoticonTime < 1000)
	{
		//TraceError("ELTimer_GetMSec() - m_dwEmoticonTime");
		return false;
	}

	return true;
}

void CInstanceBase::SetFishEmoticon()
{
	SetEmoticon(EMOTICON_FISH);
}

void CInstanceBase::SetEmoticon(UINT eEmoticon)
{
	if (eEmoticon>=EMOTICON_NUM)
	{
		TraceError("CInstanceBase[VID:%d]::SetEmoticon(eEmoticon:%d<EMOTICON_NUM:%d, isVisible=%d)",
			GetVirtualID(), eEmoticon);
		return;
	}

#ifdef ENABLE_OFFLINESHOP_SYSTEM
	if (IsShop())
		return;
#endif

	if (IsPossibleEmoticon())
	{
		D3DXVECTOR3 v3Pos = m_GraphicThingInstance.GetPosition();
		v3Pos.z += float(m_GraphicThingInstance.GetHeight());

		//CEffectManager& rkEftMgr=CEffectManager::Instance();
		CCamera * pCamera = CCameraManager::Instance().GetCurrentCamera();
		
		D3DXVECTOR3 v3Dir = (pCamera->GetEye()-v3Pos)*9/10;	
		v3Pos = pCamera->GetEye()-v3Dir;

		v3Pos = D3DXVECTOR3(0,0,0);
		v3Pos.z += float(m_GraphicThingInstance.GetHeight());

		//rkEftMgr.CreateEffect(ms_adwCRCAffectEffect[EFFECT_EMOTICON+eEmoticon],v3Pos,D3DXVECTOR3(0,0,0));
		m_GraphicThingInstance.AttachEffectByID(0, NULL, ms_adwCRCAffectEffect[EFFECT_EMOTICON+eEmoticon],&v3Pos);
		m_dwEmoticonTime = ELTimer_GetMSec();
	}
}

void CInstanceBase::SetDustGap(float fDustGap)
{
	ms_fDustGap=fDustGap;
}

void CInstanceBase::SetHorseDustGap(float fDustGap)
{
	ms_fHorseDustGap=fDustGap;
}

void CInstanceBase::__DetachEffect(DWORD dwEID)
{
	m_GraphicThingInstance.DettachEffect(dwEID);
}

DWORD CInstanceBase::__AttachEffect(UINT eEftType)
{
	// 2004.07.17.levites.isShow�� ViewFrustumCheck�� ����
	if (IsAffect(AFFECT_INVISIBILITY))
		return 0;

	if (eEftType>=EFFECT_NUM)
		return 0;

	if (ms_astAffectEffectAttachBone[eEftType].empty())
	{
		return m_GraphicThingInstance.AttachEffectByID(0, NULL, ms_adwCRCAffectEffect[eEftType]);
	}
	else
	{
		std::string & rstrBoneName = ms_astAffectEffectAttachBone[eEftType];
		const char * c_szBoneName;
		// ��տ� ���� �� ����Ѵ�.
		// �̷� ���� ���� ó���� �س��� ���� ĳ���� ���� Equip �� Bone Name �� �ٸ��� ����.
		if (0 == rstrBoneName.compare("PART_WEAPON"))
		{
			if (m_GraphicThingInstance.GetAttachingBoneName(CRaceData::PART_WEAPON, &c_szBoneName))
			{
				return m_GraphicThingInstance.AttachEffectByID(0, c_szBoneName, ms_adwCRCAffectEffect[eEftType]);
			}
		}
		else if (0 == rstrBoneName.compare("PART_WEAPON_LEFT"))
		{
			if (m_GraphicThingInstance.GetAttachingBoneName(CRaceData::PART_WEAPON_LEFT, &c_szBoneName))
			{
				return m_GraphicThingInstance.AttachEffectByID(0, c_szBoneName, ms_adwCRCAffectEffect[eEftType]);
			}
		}
		else
		{
			return m_GraphicThingInstance.AttachEffectByID(0, rstrBoneName.c_str(), ms_adwCRCAffectEffect[eEftType]);
		}
	}

	return 0;
}

void CInstanceBase::__ComboProcess()
{
	/*
	DWORD dwcurComboIndex = m_GraphicThingInstance.GetComboIndex();

	if (0 != dwcurComboIndex)
	{
		if (m_dwLastComboIndex != m_GraphicThingInstance.GetComboIndex())
		{
			if (!m_GraphicThingInstance.IsHandMode() & IsAffect(AFFECT_HWAYEOM))
			{
				__AttachEffect(EFFECT_FLAME_ATTACK);
			}
		}
	}

	m_dwLastComboIndex = dwcurComboIndex;
	*/
}

bool CInstanceBase::RegisterEffect(UINT eEftType, const char* c_szEftAttachBone, const char* c_szEftName, bool isCache)
{
	if (eEftType>=EFFECT_NUM)
		return false;

	ms_astAffectEffectAttachBone[eEftType]=c_szEftAttachBone;

	DWORD& rdwCRCEft=ms_adwCRCAffectEffect[eEftType];
	if (!CEffectManager::Instance().RegisterEffect2(c_szEftName, &rdwCRCEft, isCache))
	{
		TraceError("CInstanceBase::RegisterEffect(eEftType=%d, c_szEftAttachBone=%s, c_szEftName=%s, isCache=%d) - Error", eEftType, c_szEftAttachBone, c_szEftName, isCache);
		rdwCRCEft=0;
		return false;
	}

	return true;
}

void CInstanceBase::RegisterTitleName(int iIndex, const char * c_szTitleName)
{
	g_TitleNameMap.insert(std::make_pair(iIndex, c_szTitleName));
}

#ifdef ENABLE_LEFT_SEAT
void CInstanceBase::RegisterLeftSeatText(const char* c_szLeftSeatText)
{
	g_LeftSeatText = c_szLeftSeatText;
}
#endif

D3DXCOLOR __RGBToD3DXColoru(UINT r, UINT g, UINT b)
{
	DWORD dwColor=0xff;dwColor<<=8;
	dwColor|=r;dwColor<<=8;
	dwColor|=g;dwColor<<=8;
	dwColor|=b;

	return D3DXCOLOR(dwColor);
}

bool CInstanceBase::RegisterNameColor(UINT uIndex, UINT r, UINT g, UINT b)
{
	if (uIndex>=NAMECOLOR_NUM)
		return false;

	g_akD3DXClrName[uIndex]=__RGBToD3DXColoru(r, g, b);
	return true;
}

bool CInstanceBase::RegisterTitleColor(UINT uIndex, UINT r, UINT g, UINT b)
{
	if (uIndex>=TITLE_NUM)
		return false;

	g_akD3DXClrTitle[uIndex]=__RGBToD3DXColoru(r, g, b);
	return true;	
}

#ifdef ENABLE_BOSS_EFFECT_SYSTEM
void CInstanceBase::__AttachEffectBoss()
{
	if (!__IsExistMainInstance())
		return;

	CInstanceBase* pkInstMain = __GetMainInstancePtr();

	if (IsWarp())
		return;
	if (IsObject())
		return;
	if (IsFlag())
		return;
	if (IsResource())
		return;

	__EffectContainer_AttachEffect(EFFECT_BOSS);
}
#endif

#ifdef ENABLE_AUTO_QUQUE_ATTACK
void CInstanceBase::SetAutoFarmAffect(const bool bStatus)
{
	if (bStatus)
		__EffectContainer_AttachEffect(EFFECT_QUQUE_ATTACK);
	else
		__EffectContainer_DetachEffect(EFFECT_QUQUE_ATTACK);
}
#endif

#ifdef ENABLE_TITLE_SYSTEM
void CInstanceBase::DetachTitleEffect()
{
	if (m_dwTitleEffect)
	{
		__DetachEffect(m_dwTitleEffect);
		m_dwTitleEffect = 0;
	}
}

void CInstanceBase::AttachTitleEffectByPath(const char* szEffectPath)
{
	DetachTitleEffect();

	if (!szEffectPath || !*szEffectPath)
		return;

	DWORD dwEffectID = 0;
	if (!CEffectManager::Instance().RegisterEffect2(szEffectPath, &dwEffectID))
		return;

	// Position effect above character using model height + text tail offset + extra offset
	float fBaseHeight = m_GraphicThingInstance.GetHeight();
	float fTextTailHeight = IsMountingHorse() ? 110.0f : 10.0f;
	float fHeight = fBaseHeight + fTextTailHeight + 90.0f;
	D3DXVECTOR3 v3Offset(0.0f, 0.0f, fHeight);

	m_dwTitleEffect = m_GraphicThingInstance.AttachEffectByID(0, NULL, dwEffectID, &v3Offset);

}
#endif


