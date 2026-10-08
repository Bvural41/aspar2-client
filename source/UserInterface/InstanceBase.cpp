#include "StdAfx.h"
#include "InstanceBase.h"
#include "PythonTitleManager.h"
#include "PythonBackground.h"
#include "PythonNonPlayer.h"
#include "PythonPlayer.h"
#include "PythonCharacterManager.h"
#include "AbstractPlayer.h"
#include "AbstractApplication.h"
#include "packet.h"
#include "Locale_inc.h"
#include "PythonSystem.h"
#include "../eterlib/StateManager.h"
#include "../gamelib/ItemManager.h"
#include "../gamelib/RaceManager.h"
#ifdef ENABLE_GRAPHIC_ON_OFF
#include "PythonSystem.h"
#endif
#ifdef ENABLE_LEFT_SEAT
#include "PythonNetworkStream.h"
#endif
#ifdef ENABLE_BATTLE_ROYALE
#include "PythonBattleRoyaleManager.h"
#endif

BOOL HAIR_COLOR_ENABLE=FALSE;
BOOL USE_ARMOR_SPECULAR=FALSE;
BOOL RIDE_HORSE_ENABLE=TRUE;
const float c_fDefaultRotationSpeed = 1200.0f;
const float c_fDefaultHorseRotationSpeed = 1000.0f;


bool IsWall(unsigned race)
{
	switch (race)
	{
		case 14201:
		case 14202:
		case 14203:
		case 14204:
			return true;
			break;
	}
	return false;
}

#ifdef ENABLE_OBJ_SCALLING
typedef struct
{
	uint32_t	dwCodNPC;
	float		dwDimensiune;
} TDimensiuneNpc;

static const std::vector<TDimensiuneNpc> vectorDimensiuneNpc =
{
	{ 20201 , 50.0f },
	{ 20205 , 50.0f },
	{ 20209 , 50.0f },
	{ 20202 , 50.0f },
	{ 20206 , 50.0f },
	{ 20210 , 50.0f },
	{ 20203 , 50.0f },
	{ 20207 , 50.0f },
	{ 20211 , 50.0f },
	{ 20204 , 50.0f },
	{ 20208 , 50.0f },
	{ 20212 , 50.0f },
	{ 20226 , 50.0f },
	{ 20227 , 50.0f },
	{ 20114 , 50.0f },
	{ 20115 , 50.0f },
	{ 20116 , 50.0f },
	{ 20117 , 50.0f },
	{ 20118 , 50.0f },
	{ 20227 , 50.0f },
	{ 20226 , 50.0f },
	{ 20222 , 50.0f },
	{ 20221 , 50.0f },
	{ 20110 , 50.0f },
	{ 20111 , 50.0f },
	{ 20112 , 50.0f },
	{ 20113 , 50.0f },
	{ 20232 , 50.0f },
	{ 20231 , 50.0f },
	{ 20120 , 50.0f },
	{ 20121 , 50.0f },
	{ 20122 , 50.0f },
	{ 20123 , 50.0f },
	{ 20124 , 50.0f },
	{ 20125 , 50.0f },
	{ 20219 , 50.0f },
	{ 20220 , 50.0f },
	{ 20233 , 50.0f },
	{ 20234 , 50.0f },
	{ 20235 , 50.0f },
	{ 20236 , 50.0f },
	{ 20237 , 50.0f },
	{ 20238 , 50.0f },
	{ 20243 , 50.0f },
	{ 20244 , 50.0f },
	{ 20239 , 50.0f },
	{ 20240 , 50.0f },
	{ 20241 , 50.0f },
	{ 20245 , 50.0f },
	{ 20246 , 50.0f },
	{ 20247 , 50.0f },
	{ 20250 , 50.0f },
	{ 20251 , 50.0f },
	{ 20252 , 50.0f },
	{ 20253 , 50.0f },
	{ 20254 , 50.0f },
	{ 20255 , 50.0f },
	{ 20256 , 50.0f },
	{ 20257 , 50.0f },
	{ 20258 , 50.0f },
	{ 20259 , 50.0f },
	{ 20260 , 50.0f },
	{ 20261 , 50.0f },
	{ 20262 , 50.0f },
	{ 20263 , 50.0f },
	{ 20264 , 50.0f },
	{ 20248 , 50.0f },
	{ 20249 , 50.0f },
	{ 20265 , 50.0f },
	{ 20266 , 50.0f },
	{ 20267 , 50.0f },
	{ 20268 , 50.0f },
	{ 20269 , 50.0f },
	{ 20270 , 50.0f },
	{ 20271 , 50.0f },
	{ 20272 , 50.0f },
	{ 20273 , 50.0f },
	{ 20278 , 50.0f },
	{ 20279 , 50.0f },
};

inline float DimeniuneNPC(const DWORD dwRaceIndex)
{
	for (const auto & it : vectorDimensiuneNpc)
	{
		if (it.dwCodNPC == dwRaceIndex)
			return it.dwDimensiune;
	}

	return 0.0f;
}
#endif

CInstanceBase::SHORSE::SHORSE()
{
	__Initialize();
}

CInstanceBase::SHORSE::~SHORSE()
{
	assert(m_pkActor==NULL);
}

void CInstanceBase::SHORSE::__Initialize()
{
	m_isMounting=false;
	m_pkActor=NULL;
}

void CInstanceBase::SHORSE::SetAttackSpeed(UINT uAtkSpd)
{
	if (!IsMounting())
		return;

	CActorInstance& rkActor=GetActorRef();
	rkActor.SetAttackSpeed(uAtkSpd/100.0f);	
}

void CInstanceBase::SHORSE::SetMoveSpeed(UINT uMovSpd)
{	
	if (!IsMounting())
		return;

	CActorInstance& rkActor=GetActorRef();
	rkActor.SetMoveSpeed(uMovSpd/100.0f);
}

void CInstanceBase::SHORSE::Create(const TPixelPosition& c_rkPPos, UINT eRace, UINT eHitEffect)
{
	assert(NULL==m_pkActor && "CInstanceBase::SHORSE::Create - ALREADY MOUNT");

	m_pkActor=new CActorInstance;

	CActorInstance& rkActor=GetActorRef();
	rkActor.SetEventHandler(CActorInstance::IEventHandler::GetEmptyPtr());
	if (!rkActor.SetRace(eRace))
	{
		delete m_pkActor;
		m_pkActor=NULL;
		return;
	}

	rkActor.SetShape(0);
	rkActor.SetBattleHitEffect(eHitEffect);
	rkActor.SetAlphaValue(0.0f);
	rkActor.BlendAlphaValue(1.0f, 0.5f);
	rkActor.SetMoveSpeed(1.0f);
	rkActor.SetAttackSpeed(1.0f);
	rkActor.SetMotionMode(CRaceMotionData::MODE_GENERAL);
	rkActor.Stop();
	rkActor.RefreshActorInstance();

	rkActor.SetCurPixelPosition(c_rkPPos);

	m_isMounting=true;
}

void CInstanceBase::SHORSE::Destroy()
{
	if (m_pkActor)
	{
		m_pkActor->Destroy();
		delete m_pkActor;	
	}	

	__Initialize();
}

CActorInstance& CInstanceBase::SHORSE::GetActorRef()
{
	assert(NULL!=m_pkActor && "CInstanceBase::SHORSE::GetActorRef");
	return *m_pkActor;
}

CActorInstance* CInstanceBase::SHORSE::GetActorPtr()
{
	return m_pkActor;
}

UINT CInstanceBase::SHORSE::GetLevel()
{
	if (m_pkActor)
	{
		DWORD mount = m_pkActor->GetRace();
		switch (mount)
		{
			case 20101:
			case 20102:
			case 20103:
				return 1;
			case 20104:
			case 20105:
			case 20106:
				return 2;
			case 20107:
			case 20108:
			case 20109:
			case 20110:
			case 20111:
			case 20112:
			case 20113:
			case 20114:
			case 20115:
			case 20116:
			case 20117:
			case 20118:
			case 20120:
			case 20121:
			case 20122:
			case 20123:
			case 20124:
			case 20125:
			case 20226:
			case 20227:
			case 20119:
			case 20219:
			case 20220:
			case 20221:
			case 20222:
			case 20224:
			case 20225:
			case 20229:
			case 20230:
			case 20231:
			case 20232:
			case 20233:
			case 20234:
			case 20235:
			case 20236:
			case 20237:
			case 20238:
			case 20239:
			case 20240:
			case 20241:
			case 20242:
			case 20243:
			case 20244:
			case 20245:
			case 20246:
			case 20247:
			case 20248:
			case 20249:
			case 20250:
			case 20251:
			case 20252:
			case 20253:
			case 20254:
			case 20255:
			case 20256:
			case 20257:
			case 20258:
			case 20259:
			case 20260:
			case 20261:
			case 20262:
			case 20263:
			case 20264:
			case 20265:
			case 20266:
			case 20267:
			case 20268:
			case 20269:
			case 20270:
			case 20271:
			case 20272:
			case 20273:
			case 20274:
			case 20275:
			case 20276:
			case 20277:
			case 20278:
			case 20279:
#ifdef ENABLE_STANDING_MOUNT
			case 20280:
			case 20281:
			case 20282:
			case 20283:
			case 20284:
#endif
#ifdef ENABLE_RIDING_EXTENDED
			case 20149:
			case 20150:
			case 20151:
#endif
				return 3;
		}

		{
			if ((20205 <= mount && 20208 >= mount) ||
				(20214 == mount) || (20217 == mount) ||
				(20224 == mount) || (20229 == mount) ||
				(20234 == mount) || (20237 == mount)
				)
				return 2;

			if ((20209 <= mount &&  20212 >= mount) || 
				(20215 == mount) || (20218 == mount) ||
				(20225 == mount) || (20230 == mount) ||
				(20235 == mount) || (20238 == mount)
				)
				return 3;
		}
	}
	return 0;
}

bool CInstanceBase::SHORSE::IsNewMount()
{
	if (!m_pkActor)
		return false;
	DWORD mount = m_pkActor->GetRace();

	if ((20205 <= mount &&  20208 >= mount) ||
		(20214 == mount) || (20217 == mount) ||
		(20224 == mount) || (20229 == mount) ||
		(20234 == mount) || (20237 == mount)
		)
		return true;

	// 고급 탈것
	if ((20209 <= mount && 20212 >= mount) ||
		(20215 == mount) || (20218 == mount) ||
		(20225 == mount) || (20230 == mount) ||
		(20235 == mount) || (20238 == mount)
		)
		return true;

		switch (mount)
		{
			case 20107:
			case 20108:
			case 20109:
			case 20110:
			case 20111:
			case 20112:
			case 20113:
			case 20114:
			case 20115:
			case 20116:
			case 20117:
			case 20118:
			case 20120:
			case 20121:
			case 20122:
			case 20123:
			case 20124:
			case 20125:
			case 20226:
			case 20227:
			case 20119:
			case 20219:
			case 20220:
			case 20221:
			case 20222:
			case 20224:
			case 20225:
			case 20229:
			case 20230:
			case 20231:
			case 20232:
			case 20233:
			case 20234:
			case 20235:
			case 20236:
			case 20237:
			case 20238:
			case 20239:
			case 20240:
			case 20241:
			case 20242:
			case 20243:
			case 20244:
			case 20245:
			case 20246:
			case 20247:
			case 20248:
			case 20249:
			case 20250:
			case 20251:
			case 20252:
			case 20253:
			case 20254:
			case 20255:
			case 20256:
			case 20257:
			case 20258:
			case 20259:
			case 20260:
			case 20261:
			case 20262:
			case 20263:
			case 20264:
			case 20265:
			case 20266:
			case 20267:
			case 20268:
			case 20269:
			case 20270:
			case 20271:
			case 20272:
			case 20273:
			case 20274:
			case 20275:
			case 20276:
			case 20277:
			case 20278:
			case 20279:
#ifdef ENABLE_STANDING_MOUNT
			case 20280:
			case 20281:
			case 20282:
			case 20283:
			case 20284:
#endif
#ifdef ENABLE_RIDING_EXTENDED
			case 20149:
			case 20150:
			case 20151:
#endif
				return true;
		}

	return false;
}

bool CInstanceBase::IsUnderAttackByOtherPlayer() const
{
	IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();
	CInstanceBase* pMainActor = CPythonPlayer::Instance().NEW_GetMainActorPtr();
	if (!pMainActor)
		return false; // E?er oyuncunun ana karakteri yoksa, kontrol yapmaya gerek yok.

	uint32_t myVID = pMainActor->GetVirtualID();

	// E?er son saldıran ben de?ilsem ve son saldırı yeni olduysa, hedef ba?ka biri tarafından vuruluyor demektir.
	return (m_lastAttackerVID != 0 && m_lastAttackerVID != myVID && (GetCurrentTime() - m_lastAttackTime) < 3000);
}

#ifdef ENABLE_STANDING_MOUNT
bool CInstanceBase::SHORSE::IsHoverBoard()
{
	if (!m_pkActor)
		return false;

	return (m_pkActor->GetRace() == SURFBOARD || 
			m_pkActor->GetRace() == WUKONG_1 || 
			m_pkActor->GetRace() == WUKONG_2 || 
			m_pkActor->GetRace() == DRAKKAR_1 || 
			m_pkActor->GetRace() == DRAKKAR_2);
}
#endif

bool CInstanceBase::SHORSE::CanUseSkill()
{
	if (IsMounting())
		return 2 < GetLevel();

	return true;
}

bool CInstanceBase::SHORSE::CanAttack()
{
	if (IsMounting())
		if (GetLevel()<=1)
			return false;

	return true;
}
			
bool CInstanceBase::SHORSE::IsMounting()
{
	return m_isMounting;
}

void CInstanceBase::SHORSE::Deform()
{
	if (!IsMounting())
		return;

	CActorInstance& rkActor=GetActorRef();
	rkActor.INSTANCEBASE_Deform();
}

void CInstanceBase::SHORSE::Render()
{
	if (!IsMounting())
		return;

	CActorInstance& rkActor=GetActorRef();
	rkActor.Render();
}

void CInstanceBase::__AttachHorseSaddle()
{
	if (!IsMountingHorse())
		return;
	m_kHorse.m_pkActor->AttachModelInstance(CRaceData::PART_MAIN, "saddle", m_GraphicThingInstance, CRaceData::PART_MAIN);
}

void CInstanceBase::__DetachHorseSaddle()
{
	if (!IsMountingHorse())
		return;
	m_kHorse.m_pkActor->DetachModelInstance(CRaceData::PART_MAIN, m_GraphicThingInstance, CRaceData::PART_MAIN);
}

void CInstanceBase::BlockMovement()
{
	m_GraphicThingInstance.BlockMovement();
}

bool CInstanceBase::IsBlockObject(const CGraphicObjectInstance& c_rkBGObj)
{
	return m_GraphicThingInstance.IsBlockObject(c_rkBGObj);
}

bool CInstanceBase::AvoidObject(const CGraphicObjectInstance& c_rkBGObj)
{
	return m_GraphicThingInstance.AvoidObject(c_rkBGObj);
}

bool __ArmorVnumToShape(int iVnum, DWORD * pdwShape)
{
	*pdwShape = iVnum;

	if (0 == iVnum || 1 == iVnum)
		return false;

	if (!USE_ARMOR_SPECULAR)
		return false;

	CItemData * pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(iVnum, &pItemData))
		return false;

	enum
	{
		SHAPE_VALUE_SLOT_INDEX = 3,
	};

	*pdwShape = pItemData->GetValue(SHAPE_VALUE_SLOT_INDEX);

	return true;
}

class CActorInstanceBackground : public IBackground
{
	public:
		CActorInstanceBackground() {}
		virtual ~CActorInstanceBackground() {}
		bool IsBlock(int x, int y)
		{
			CPythonBackground& rkBG=CPythonBackground::Instance();
			return rkBG.isAttrOn(x, y, CTerrainImpl::ATTRIBUTE_BLOCK);
		}
};

static CActorInstanceBackground gs_kActorInstBG;

bool CInstanceBase::LessRenderOrder(CInstanceBase* pkInst)
{
	int nMainAlpha=(__GetAlphaValue() < 1.0f) ? 1 : 0;
	int nTestAlpha=(pkInst->__GetAlphaValue() < 1.0f) ? 1 : 0;
	if (nMainAlpha < nTestAlpha)
		return true;
	if (nMainAlpha > nTestAlpha)
		return false;

	if (GetRace()<pkInst->GetRace())
		return true;
	if (GetRace()>pkInst->GetRace())
		return false;

	if (GetShape()<pkInst->GetShape())
		return true;

	if (GetShape()>pkInst->GetShape())
		return false;

	UINT uLeftLODLevel=__LessRenderOrder_GetLODLevel();
	UINT uRightLODLevel=pkInst->__LessRenderOrder_GetLODLevel();
	if (uLeftLODLevel<uRightLODLevel)
		return true;
	if (uLeftLODLevel>uRightLODLevel)
		return false;

	if (m_awPart[CRaceData::PART_WEAPON]<pkInst->m_awPart[CRaceData::PART_WEAPON])
		return true;

	return false;
}

UINT CInstanceBase::__LessRenderOrder_GetLODLevel()
{
	CGrannyLODController* pLODCtrl=m_GraphicThingInstance.GetLODControllerPointer(0);
	if (!pLODCtrl)
		return 0;

	return pLODCtrl->GetLODLevel();
}

bool CInstanceBase::__Background_GetWaterHeight(const TPixelPosition& c_rkPPos, float* pfHeight)
{
	long lHeight;
	if (!CPythonBackground::Instance().GetWaterHeight(int(c_rkPPos.x), int(c_rkPPos.y), &lHeight))
		return false;

	*pfHeight = float(lHeight);

	return true;
}

bool CInstanceBase::__Background_IsWaterPixelPosition(const TPixelPosition& c_rkPPos)
{
	return CPythonBackground::Instance().isAttrOn(c_rkPPos.x, c_rkPPos.y, CTerrainImpl::ATTRIBUTE_WATER);
}

const float PC_DUST_RANGE = 2000.0f;
const float NPC_DUST_RANGE = 1000.0f;

DWORD CInstanceBase::ms_dwUpdateCounter=0;
DWORD CInstanceBase::ms_dwRenderCounter=0;
DWORD CInstanceBase::ms_dwDeformCounter=0;

CDynamicPool<CInstanceBase> CInstanceBase::ms_kPool;

bool CInstanceBase::__IsInDustRange()
{
	if (!__IsExistMainInstance())
		return false;

	CInstanceBase* pkInstMain=__GetMainInstancePtr();

	float fDistance=NEW_GetDistanceFromDestInstance(*pkInstMain);

	if (IsPC())
	{
		if (fDistance<=PC_DUST_RANGE)
			return true;
	}

	if (fDistance<=NPC_DUST_RANGE)
		return true;

	return false;
}

void CInstanceBase::__EnableSkipCollision()
{
	if (__IsMainInstance())
	{
		TraceError("CInstanceBase::__EnableSkipCollision - 자신은 충돌검사스킵이 되면 안된다!!");
		return;
	}
	m_GraphicThingInstance.EnableSkipCollision();
}

void CInstanceBase::__DisableSkipCollision()
{
	m_GraphicThingInstance.DisableSkipCollision();
}

DWORD CInstanceBase::__GetShadowMapColor(float x, float y)
{
	CPythonBackground& rkBG=CPythonBackground::Instance();
	return rkBG.GetShadowMapColor(x, y);
}

float CInstanceBase::__GetBackgroundHeight(float x, float y)
{
	CPythonBackground& rkBG=CPythonBackground::Instance();
	return rkBG.GetHeight(x, y);
}

#ifdef __MOVIE_MODE__
BOOL CInstanceBase::IsMovieMode()
{
	if (IsAffect(AFFECT_INVISIBILITY))
		return true;

	return false;
}

#endif

#ifdef ENABLE_NINJA_MAP_FIX
BOOL CInstanceBase::IsInvisibility()
{
	if (IsAffect(AFFECT_INVISIBILITY) || IsAffect(AFFECT_EUNHYEONG))
		return true;
	return false;
}
#else
BOOL CInstanceBase::IsInvisibility()
{
	if (IsAffect(AFFECT_INVISIBILITY))
		return true;

	return false;
}
#endif

BOOL CInstanceBase::IsStealth()
{
	if (IsAffect(AFFECT_EUNHYEONG))
		return true;

	return false;
}

BOOL CInstanceBase::IsParalysis()
{
	return m_GraphicThingInstance.IsParalysis();
}

BOOL CInstanceBase::IsGameMaster()
{
	if (m_kAffectFlagContainer.IsSet(AFFECT_YMIR))
		return true;
	return false;
}

BOOL CInstanceBase::IsSameEmpire(CInstanceBase& rkInstDst)
{
	if (0 == rkInstDst.m_dwEmpireID)
		return TRUE;

#ifdef ENABLE_BATTLE_FIELD
	if (IsCombatZoneMap() || rkInstDst.IsCombatZoneMap())
		return FALSE;
#endif

	if (IsGameMaster())
		return TRUE;

	if (rkInstDst.IsGameMaster())
		return TRUE;

	if (rkInstDst.m_dwEmpireID==m_dwEmpireID)
		return TRUE;

	return FALSE;
}

DWORD CInstanceBase::GetEmpireID()
{
	return m_dwEmpireID;
}

#if defined(ENABLE_SHOW_MOB_INFO)
DWORD CInstanceBase::GetAIFlag()
{
	return m_dwAIFlag;
}
#endif

DWORD CInstanceBase::GetGuildID()
{
	return m_dwGuildID;
}

int CInstanceBase::GetAlignment()
{
	return m_sAlignment;
}

UINT CInstanceBase::GetAlignmentGrade()
{
	if (m_sAlignment >= 12000)
		return 0;
	else if (m_sAlignment >= 8000)
		return 1;
	else if (m_sAlignment >= 4000)
		return 2;
	else if (m_sAlignment >= 1000)
		return 3;
	else if (m_sAlignment >= 0)
		return 4;
	else if (m_sAlignment > -4000)
		return 5;
	else if (m_sAlignment > -8000)
		return 6;
	else if (m_sAlignment > -12000)
		return 7;

	return 8;
}

int CInstanceBase::GetAlignmentType()
{
	switch (GetAlignmentGrade())
	{
	case 0:
	case 1:
	case 2:
	case 3:
	{
		return ALIGNMENT_TYPE_WHITE;
		break;
	}

	case 5:
	case 6:
	case 7:
	case 8:
	{
		return ALIGNMENT_TYPE_DARK;
		break;
	}
	}

	return ALIGNMENT_TYPE_NORMAL;
}

BYTE CInstanceBase::GetPKMode()
{
	return m_byPKMode;
}

bool CInstanceBase::IsKiller()
{
	return m_isKiller;
}

bool CInstanceBase::IsPartyMember()
{
	return m_isPartyMember;
}

BOOL CInstanceBase::IsInSafe()
{
	const TPixelPosition& c_rkPPosCur=m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
	if (CPythonBackground::Instance().isAttrOn(c_rkPPosCur.x, c_rkPPosCur.y, CTerrainImpl::ATTRIBUTE_BANPK))
		return TRUE;

	return FALSE;
}

float CInstanceBase::CalculateDistanceSq3d(const TPixelPosition& c_rkPPosDst)
{
	const TPixelPosition& c_rkPPosSrc=m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
	return SPixelPosition_CalculateDistanceSq3d(c_rkPPosSrc, c_rkPPosDst);
}

void CInstanceBase::OnSelected()
{
#ifdef __MOVIE_MODE__
	if (!__IsExistMainInstance())
		return;
#endif

	if (IsStoneDoor())
		return;

	if (IsDead())
		return;

	__AttachSelectEffect();
}

void CInstanceBase::OnUnselected()
{
	__DetachSelectEffect();
}

void CInstanceBase::OnTargeted()
{
#ifdef __MOVIE_MODE__
	if (!__IsExistMainInstance())
		return;
#endif

	if (IsStoneDoor())
		return;

	if (IsDead())
		return;

	__AttachTargetEffect();
}

void CInstanceBase::OnUntargeted()
{
	__DetachTargetEffect();
}

void CInstanceBase::DestroySystem()
{
	ms_kPool.Clear();
}

void CInstanceBase::CreateSystem(UINT uCapacity)
{
	ms_kPool.Create(uCapacity);

	memset(ms_adwCRCAffectEffect, 0, sizeof(ms_adwCRCAffectEffect));

	ms_fDustGap=250.0f;
	ms_fHorseDustGap=500.0f;
}

CInstanceBase* CInstanceBase::New()
{
	return ms_kPool.Alloc();
}

void CInstanceBase::Delete(CInstanceBase* pkInst)
{
	pkInst->Destroy();
	ms_kPool.Free(pkInst);
}

void CInstanceBase::SetMainInstance()
{
	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();

	DWORD dwVID=GetVirtualID();
	rkChrMgr.SetMainInstance(dwVID);

	m_GraphicThingInstance.SetMainInstance();
}

CInstanceBase* CInstanceBase::__GetMainInstancePtr()
{
	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	return rkChrMgr.GetMainInstancePtr();
}

void CInstanceBase::__ClearMainInstance()
{
	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	rkChrMgr.ClearMainInstance();
}

bool CInstanceBase::__IsMainInstance()
{
	if (this==__GetMainInstancePtr())
		return true;

	return false;
}

bool CInstanceBase::__IsExistMainInstance()
{
	if(__GetMainInstancePtr())
		return true;
	else
		return false;
}

bool CInstanceBase::__MainCanSeeHiddenThing()
{
	return false;
//	CInstanceBase * pInstance = __GetMainInstancePtr();
//	return pInstance->IsAffect(AFFECT_GAMJI);
}

float CInstanceBase::__GetBowRange()
{
	float fRange = 2500.0f - 100.0f;

	if (__IsMainInstance())
	{
		IAbstractPlayer& rPlayer=IAbstractPlayer::GetSingleton();
		fRange += float(rPlayer.GetStatus(POINT_BOW_DISTANCE));
	}

	return fRange;
}

CInstanceBase* CInstanceBase::__FindInstancePtr(DWORD dwVID)
{
	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	return rkChrMgr.GetInstancePtr(dwVID);
}

bool CInstanceBase::__FindRaceType(DWORD dwRace, BYTE* pbType)
{
	CPythonNonPlayer& rkNonPlayer=CPythonNonPlayer::Instance();
	return rkNonPlayer.GetInstanceType(dwRace, pbType);
}

#ifdef ENABLE_INGAME_WIKI
bool CInstanceBase::Create(const SCreateData& c_rkCreateData, bool wikiPreview)
#else
bool CInstanceBase::Create(const SCreateData& c_rkCreateData)
#endif
{
	IAbstractApplication::GetSingleton().SkipRenderBuffering(300);

	SetInstanceType(c_rkCreateData.m_bType);

	if (!SetRace(c_rkCreateData.m_dwRace))
		return false;

#ifdef ENABLE_SUPPORT_SYSTEM
	int i=0;
	if(c_rkCreateData.m_dwRace == 9000)
	{
		SetInstanceType(CActorInstance::TYPE_PC);
		SetRace(3);
		i=1;
	}
#endif

	SetVirtualID(c_rkCreateData.m_dwVID);

	if (c_rkCreateData.m_isMain)
	{
		SetMainInstance();
#ifdef ENABLE_LEFT_SEAT
		CPythonPlayer::Instance().LoadLeftSeatData();
#endif
	}

	if (IsGuildWall())
	{
		unsigned center_x;
		unsigned center_y;

		c_rkCreateData.m_kAffectFlags.ConvertToPosition(&center_x, &center_y);
		
		float center_z = __GetBackgroundHeight(center_x, center_y);
		NEW_SetPixelPosition(TPixelPosition(float(c_rkCreateData.m_lPosX), float(c_rkCreateData.m_lPosY), center_z));
	}
	else
	{
		SCRIPT_SetPixelPosition(float(c_rkCreateData.m_lPosX), float(c_rkCreateData.m_lPosY));
	}	

	if (0 != c_rkCreateData.m_dwMountVnum)
		MountHorse(c_rkCreateData.m_dwMountVnum);

	SetArmor(c_rkCreateData.m_dwArmor);

#ifdef ENABLE_SUPPORT_SYSTEM
	if (IsPC() || i == 1 )
#else
	if (IsPC())
#endif
	{
		SetHair(c_rkCreateData.m_dwHair);
#ifdef ENABLE_BATTLE_FIELD
		SetCombatZoneRank(c_rkCreateData.combat_zone_rank);
		SetCombatZonePoints(c_rkCreateData.combat_zone_points);
#endif
#ifdef ENABLE_SASH_SYSTEM
		SetSash(c_rkCreateData.m_dwSash);
#endif
#ifdef ENABLE_REFINE_ELEMENT
		SetRefineElementType(c_rkCreateData.m_bRefineElementType);
#endif
#ifdef ENABLE_NEW_ARROW_SYSTEM
		SetWeapon(c_rkCreateData.m_dwWeapon, c_rkCreateData.m_dwArrowType);
#else
		SetWeapon(c_rkCreateData.m_dwWeapon);
#endif
#ifdef ENABLE_AURA_SYSTEM
		SetAura(c_rkCreateData.m_dwAura);
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		SetLanguage(c_rkCreateData.m_bLanguage);
#endif
	}

#ifdef ENABLE_SUPPORT_SYSTEM
	if (i==1)
		SetSupportShaman(c_rkCreateData.is_support_shaman);
#endif

	__Create_SetName(c_rkCreateData);
#ifdef ENABLE_CONQUEROR_LEVEL
	if(IsPC())
		m_dwConquerorLevel = c_rkCreateData.m_dwConquerorLevel;
#endif
	m_dwLevel = c_rkCreateData.m_dwLevel;
#if defined(ENABLE_SHOW_MOB_INFO)
	m_dwAIFlag = c_rkCreateData.m_dwAIFlag;
#endif
	m_dwGuildID = c_rkCreateData.m_dwGuildID;
	m_dwEmpireID = c_rkCreateData.m_dwEmpireID;
	SetVirtualNumber(c_rkCreateData.m_dwRace);
	SetRotation(c_rkCreateData.m_fRot);
	SetAlignment(c_rkCreateData.m_sAlignment);
	SetLevelText(c_rkCreateData.m_dwLevel);
	SetPKMode(c_rkCreateData.m_byPKMode);
	SetMoveSpeed(c_rkCreateData.m_dwMovSpd);
	SetAttackSpeed(c_rkCreateData.m_dwAtkSpd);

#ifdef ENABLE_BOSS_EFFECT_SYSTEM
	if (m_dwRace == 191 ||
		m_dwRace == 192 ||
		m_dwRace == 193 ||
		m_dwRace == 194 ||
		m_dwRace == 491 ||
		m_dwRace == 492 ||
		m_dwRace == 493 ||
		m_dwRace == 494 ||
		m_dwRace == 531 ||
		m_dwRace == 532 ||
		m_dwRace == 533 ||
		m_dwRace == 534 ||
		m_dwRace == 591 ||
		m_dwRace == 691 ||
		m_dwRace == 791 ||
		m_dwRace == 792 ||
		m_dwRace == 1091 ||
		m_dwRace == 1092 ||
		m_dwRace == 1093 ||
		m_dwRace == 1192 ||
		m_dwRace == 1304 ||
		m_dwRace == 1901 ||
		m_dwRace == 2091 ||
		m_dwRace == 2206 ||
		m_dwRace == 2307 ||
		m_dwRace == 2306 ||
		m_dwRace == 2491 ||
		m_dwRace == 2492 ||
		m_dwRace == 2494 ||
		m_dwRace == 2495 ||
		m_dwRace == 2597 ||
		m_dwRace == 2598 ||
		m_dwRace == 3190 ||
		m_dwRace == 3191 ||
		m_dwRace == 3290 ||
		m_dwRace == 3291 ||
		m_dwRace == 3390 ||
		m_dwRace == 3391 ||
		m_dwRace == 3490 ||
		m_dwRace == 3491 ||
		m_dwRace == 3590 ||
		m_dwRace == 3591 ||
		m_dwRace == 3690 ||
		m_dwRace == 3691 ||
		m_dwRace == 3790 ||
		m_dwRace == 3791 ||
		m_dwRace == 3890 ||
		m_dwRace == 3891 ||
		m_dwRace == 6390 ||
		m_dwRace == 6391 ||
		m_dwRace == 3090 ||
		m_dwRace == 3091 ||
		m_dwRace == 2191 ||
		m_dwRace == 6421 ||
		m_dwRace == 4204 ||
		m_dwRace == 4209 ||
		m_dwRace == 4210 ||
		m_dwRace == 3596 ||
		m_dwRace == 1095 ||
		m_dwRace == 1191
	)
		__AttachEffectBoss();
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
	if (IsGrowthPet())
		SetPetScale(c_rkCreateData.m_dwLevel);
#endif

	if (!IsWearingDress())
	{
		m_GraphicThingInstance.SetAlphaValue(0.0f);
		m_GraphicThingInstance.BlendAlphaValue(1.0f, 0.5f);
	}

	if (!IsGuildWall())
	{
		SetAffectFlagContainer(c_rkCreateData.m_kAffectFlags);
	}

#ifdef ENABLE_BATTLE_ROYALE
	if (!CPythonBackground::Instance().IsBattleRoyaleMap() || !PythonBattleRoyaleManager::Instance().IsPCTextTailHided() || !IsPC())
	{
		AttachTextTail();
		RefreshTextTail();
	}
#else
	AttachTextTail();
	RefreshTextTail();
#endif

	if (c_rkCreateData.m_dwStateFlags & ADD_CHARACTER_STATE_SPAWN) 
	{
		// if (IsAffect(AFFECT_SPAWN))
			// __AttachEffect(EFFECT_SPAWN_APPEAR); //Bvural41 Slot Spawn Fix

		if (IsPC())
		{
			Refresh(CRaceMotionData::NAME_WAIT, true);
		}
		else
		{
			Refresh(CRaceMotionData::NAME_SPAWN, false);
		}
	}
	else
	{
		Refresh(CRaceMotionData::NAME_WAIT, true);
	}

	__AttachEmpireEffect(c_rkCreateData.m_dwEmpireID);

	RegisterBoundingSphere();

	if (c_rkCreateData.m_dwStateFlags & ADD_CHARACTER_STATE_DEAD)
		m_GraphicThingInstance.DieEnd();

	SetStateFlags(c_rkCreateData.m_dwStateFlags);

	m_GraphicThingInstance.SetBattleHitEffect(ms_adwCRCAffectEffect[EFFECT_HIT]);

#ifdef ENABLE_OFFLINESHOP_SYSTEM
	if (IsShop())
	{
		if (m_myShop == 0 && strstr(c_rkCreateData.m_stName.c_str(), CPythonPlayer::Instance().GetName()))
			__AttachEffect(EFFECT_REFINED + EFFECT_MYOFFSHOP);
	}
#endif

	if (!IsPC()
#ifdef ENABLE_INGAME_WIKI
		|| wikiPreview
#endif
	)
	{
		DWORD dwBodyColor = CPythonNonPlayer::Instance().GetMonsterColor(c_rkCreateData.m_dwRace);
		if (0 != dwBodyColor)
		{
			SetModulateRenderMode();
			SetAddColor(dwBodyColor);
		}
	}

	__AttachHorseSaddle();

#ifdef ENABLE_BALATHOR_DUNGEON
	if (c_rkCreateData.m_dwRace == 6896 || c_rkCreateData.m_dwRace == 6897)
		m_GraphicThingInstance.SetForceShow(true);
	CPythonPlayer::Instance().CheckBalathorEffect(this);
#endif

#ifdef ENABLE_OBJ_SCALLING
	float fScale = DimeniuneNPC(GetRace());
	if (fScale)
	{
		fScale /= 100.0f;
		m_GraphicThingInstance.SetScale(fScale, fScale, fScale, true);
	}
#endif

	const int c_iGuildSymbolRace = 14200;
	if (c_iGuildSymbolRace == GetRace())
	{
		std::string strFileName = GetGuildSymbolFileName(m_dwGuildID);
		if (IsFile(strFileName.c_str()))
			m_GraphicThingInstance.ChangeMaterial(strFileName.c_str());
	}

#ifdef ENABLE_WHITE_DRAGON
	if (GetRace() == 6790 || GetRace() == 6791)
		m_GraphicThingInstance.SetForceShow(true);
#endif

#ifdef ENABLE_GREEDY_ROOM
	if (GetRace() == 6939 || GetRace() == 6943)
		m_GraphicThingInstance.SetForceShow(true);
#endif

	return true;
}


void CInstanceBase::__Create_SetName(const SCreateData& c_rkCreateData)
{
	if (IsGoto())
	{
		SetNameString("", 0);
		return;
	}
	if (IsWarp())
	{
		__Create_SetWarpName(c_rkCreateData);
		return;
	}

	SetNameString(c_rkCreateData.m_stName.c_str(), c_rkCreateData.m_stName.length());
}

void CInstanceBase::__Create_SetWarpName(const SCreateData& c_rkCreateData)
{
	const char * c_szName;
	if (CPythonNonPlayer::Instance().GetName(c_rkCreateData.m_dwRace, &c_szName))
	{
		std::string strName = c_szName;
		int iFindingPos = strName.find_first_of(" ", 0);
		if (iFindingPos > 0)
		{
			strName.resize(iFindingPos);
		}
		SetNameString(strName.c_str(), strName.length());
	}
	else
	{
		SetNameString(c_rkCreateData.m_stName.c_str(), c_rkCreateData.m_stName.length());
	}
}

void CInstanceBase::SetNameString(const char* c_szName, int len)
{
	m_stName.assign(c_szName, len);
}

bool CInstanceBase::SetRace(DWORD eRace)
{
	m_dwRace = eRace;

	if (!m_GraphicThingInstance.SetRace(eRace))
		return false;

	if (!__FindRaceType(m_dwRace, &m_eRaceType))
		m_eRaceType=CActorInstance::TYPE_PC;

	return true;
}

BOOL CInstanceBase::__IsChangableWeapon(int iWeaponID)
{
	if (IsWearingDress())
	{
		const int c_iBouquets[] =
		{
			50201,
			50202,
			50203,
			50204,
			0,
		};

		for (int i = 0; c_iBouquets[i] != 0; ++i)
			if (iWeaponID == c_iBouquets[i])
				return true;

		return false;
	}
	else
		return true;
}

BOOL CInstanceBase::IsWearingDress()
{
	const int c_iWeddingDressShape = 201;
	return c_iWeddingDressShape == m_eShape;
}

BOOL CInstanceBase::IsHoldingPickAxe()
{
	const int c_iPickAxeStart = 29101;
	const int c_iPickAxeEnd = 29110;
	return m_awPart[CRaceData::PART_WEAPON] >= c_iPickAxeStart && m_awPart[CRaceData::PART_WEAPON] <= c_iPickAxeEnd;
}

BOOL CInstanceBase::IsNewMount()
{
	return m_kHorse.IsNewMount();
}

BOOL CInstanceBase::IsMountingHorse()
{
	return m_kHorse.IsMounting();
}

#ifdef ENABLE_STANDING_MOUNT
BOOL CInstanceBase::IsMountingHoverBoard()
{
	return m_kHorse.IsHoverBoard();
}
#endif

void CInstanceBase::MountHorse(UINT eRace)
{
	m_kHorse.Destroy();
	m_kHorse.Create(m_GraphicThingInstance.NEW_GetCurPixelPositionRef(), eRace, ms_adwCRCAffectEffect[EFFECT_HIT]);

#ifdef ENABLE_STANDING_MOUNT
	if (eRace == SURFBOARD || eRace == WUKONG_1 || eRace == WUKONG_2 || eRace == DRAKKAR_1 || eRace == DRAKKAR_2)
	{
		SetMotionMode(CRaceMotionData::MODE_HORSE_STAND);
	}
	else
#endif
	{
		SetMotionMode(CRaceMotionData::MODE_HORSE);
	}

	SetRotationSpeed(c_fDefaultHorseRotationSpeed);

	m_GraphicThingInstance.MountHorse(m_kHorse.GetActorPtr());
	m_GraphicThingInstance.Stop();
	m_GraphicThingInstance.RefreshActorInstance();

#ifdef ENABLE_TITLE_SYSTEM
	CPythonTitleManager::Instance().RefreshTitleByVID(GetVirtualID());
#endif
}

void CInstanceBase::DismountHorse()
{
	m_kHorse.Destroy();

#ifdef ENABLE_TITLE_SYSTEM
	CPythonTitleManager::Instance().RefreshTitleByVID(GetVirtualID());
#endif
}

void CInstanceBase::GetInfo(std::string* pstInfo)
{
	char szInfo[256];
	sprintf(szInfo, "Inst - UC %d, RC %d Pool - %d ", 
		ms_dwUpdateCounter, 
		ms_dwRenderCounter,
		ms_kPool.GetCapacity()
	);

	pstInfo->append(szInfo);
}

void CInstanceBase::ResetPerformanceCounter()
{
	ms_dwUpdateCounter=0;
	ms_dwRenderCounter=0;
	ms_dwDeformCounter=0;
}

bool CInstanceBase::NEW_IsLastPixelPosition()
{
	return m_GraphicThingInstance.IsPushing();
}

const TPixelPosition& CInstanceBase::NEW_GetLastPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetLastPixelPositionRef();
}

void CInstanceBase::NEW_SetDstPixelPositionZ(FLOAT z)
{
	m_GraphicThingInstance.NEW_SetDstPixelPositionZ(z);
}

void CInstanceBase::NEW_SetDstPixelPosition(const TPixelPosition& c_rkPPosDst)
{
	m_GraphicThingInstance.NEW_SetDstPixelPosition(c_rkPPosDst);
}

void CInstanceBase::NEW_SetSrcPixelPosition(const TPixelPosition& c_rkPPosSrc)
{
	m_GraphicThingInstance.NEW_SetSrcPixelPosition(c_rkPPosSrc);
}

const TPixelPosition& CInstanceBase::NEW_GetCurPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetCurPixelPositionRef();	
}

const TPixelPosition& CInstanceBase::NEW_GetDstPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetDstPixelPositionRef();
}

const TPixelPosition& CInstanceBase::NEW_GetSrcPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetSrcPixelPositionRef();
}

void CInstanceBase::OnSyncing()
{
	m_GraphicThingInstance.__OnSyncing();
}

void CInstanceBase::OnWaiting()
{
	m_GraphicThingInstance.__OnWaiting();
}

void CInstanceBase::OnMoving()
{
	m_GraphicThingInstance.__OnMoving();
}

void CInstanceBase::ChangeGuild(DWORD dwGuildID)
{
	m_dwGuildID=dwGuildID;

	DetachTextTail();
#ifdef ENABLE_BATTLE_ROYALE
	if (!CPythonBackground::Instance().IsBattleRoyaleMap() || !PythonBattleRoyaleManager::Instance().IsPCTextTailHided() || !IsPC())
	{
		AttachTextTail();
		RefreshTextTail();
	}
#else
	AttachTextTail();
	RefreshTextTail();
#endif
}

DWORD CInstanceBase::GetPart(CRaceData::EParts part)
{
	assert(part >= 0 && part < CRaceData::PART_MAX_NUM);
	return m_awPart[part];
}

DWORD CInstanceBase::GetShape()
{
	return m_eShape;
}

bool CInstanceBase::CanAct()
{
	return m_GraphicThingInstance.CanAct();
}

bool CInstanceBase::CanMove()
{
#ifdef ENABLE_MOONLIGHT_VALLEY
	if (IsAffect(AFFECT_MOONLIGHT_TORNADO))
		return false;
#endif

	return m_GraphicThingInstance.CanMove();
}

bool CInstanceBase::CanUseSkill()
{
	if (IsPoly())
		return false;

	if (IsWearingDress())
		return false;

	if (IsHoldingPickAxe())
		return false;

	if (!m_kHorse.CanUseSkill())
		return false;

	if (!m_GraphicThingInstance.CanUseSkill())
		return false;

	return true;
}

bool CInstanceBase::CanAttack()
{
	if (!m_kHorse.CanAttack())
		return false;

	if (IsWearingDress())
		return false;

	if (IsHoldingPickAxe())
		return false;
	
	return m_GraphicThingInstance.CanAttack();
}

bool CInstanceBase::CanFishing()
{
	return m_GraphicThingInstance.CanFishing();
}

BOOL CInstanceBase::IsBowMode()
{
	return m_GraphicThingInstance.IsBowMode();
}

BOOL CInstanceBase::IsHandMode()
{
	return m_GraphicThingInstance.IsHandMode();
}

BOOL CInstanceBase::IsFishingMode()
{
	if (CRaceMotionData::MODE_FISHING == m_GraphicThingInstance.GetMotionMode()
#ifdef ENABLE_STANDING_MOUNT
		|| CRaceMotionData::MODE_HORSE_STAND_FISHING == m_GraphicThingInstance.GetMotionMode()
#endif
	)
		return true;

	return false;
}

BOOL CInstanceBase::IsFishing()
{
	return m_GraphicThingInstance.IsFishing();
}

BOOL CInstanceBase::IsDead()
{
	return m_GraphicThingInstance.IsDead();
}

BOOL CInstanceBase::IsStun()
{
#ifdef ENABLE_MOONLIGHT_VALLEY
	return m_GraphicThingInstance.IsStun() || IsAffect(AFFECT_MOONLIGHT_TORNADO);
#else
	return m_GraphicThingInstance.IsStun();
#endif
}

BOOL CInstanceBase::IsSleep()
{
	return m_GraphicThingInstance.IsSleep();
}


BOOL CInstanceBase::__IsSyncing()
{
	return m_GraphicThingInstance.__IsSyncing();
}

void CInstanceBase::NEW_SetOwner(DWORD dwVIDOwner)
{
	m_GraphicThingInstance.SetOwner(dwVIDOwner);
}

float CInstanceBase::GetLocalTime()
{
	return m_GraphicThingInstance.GetLocalTime();
}

void CInstanceBase::PushUDPState(DWORD dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg)
{
}

DWORD	ELTimer_GetServerFrameMSec();

void CInstanceBase::PushTCPStateExpanded(DWORD dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg, UINT uTargetVID)
{
	SCommand kCmdNew;
	kCmdNew.m_kPPosDst = c_rkPPosDst;
	kCmdNew.m_dwChkTime = dwCmdTime+100;
	kCmdNew.m_dwCmdTime = dwCmdTime;
	kCmdNew.m_fDstRot = fDstRot;
	kCmdNew.m_eFunc = eFunc;
	kCmdNew.m_uArg = uArg;
	kCmdNew.m_uTargetVID = uTargetVID;
	m_kQue_kCmdNew.push_back(kCmdNew);
}

void CInstanceBase::PushTCPState(DWORD dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg)
{	
	/*
	if (__IsMainInstance())
	{
		//assert(!"CInstanceBase::PushTCPState 플레이어 자신에게 이동패킷은 오면 안된다!");
		TraceError("CInstanceBase::PushTCPState 플레이어 자신에게 이동패킷은 오면 안된다!");
		return;
	}
	//Bvural41 Fix Combat Zone Syserr
	*/

	int nNetworkGap=ELTimer_GetServerFrameMSec()-dwCmdTime;
	
	m_nAverageNetworkGap=(m_nAverageNetworkGap*70+nNetworkGap*30)/100;
	
	/*
	if (m_dwBaseCmdTime == 0)
	{
		m_dwBaseChkTime = ELTimer_GetFrameMSec()-nNetworkGap;
		m_dwBaseCmdTime = dwCmdTime;

		Tracenf("VID[%d] 네트웍갭 [%d]", GetVirtualID(), nNetworkGap);
	}
	*/

	//m_dwBaseChkTime-m_dwBaseCmdTime+ELTimer_GetServerMSec();

	SCommand kCmdNew;
	kCmdNew.m_kPPosDst = c_rkPPosDst;
	kCmdNew.m_dwChkTime = dwCmdTime+m_nAverageNetworkGap;//m_dwBaseChkTime + (dwCmdTime - m_dwBaseCmdTime);// + nNetworkGap;
	kCmdNew.m_dwCmdTime = dwCmdTime;
	kCmdNew.m_fDstRot = fDstRot;
	kCmdNew.m_eFunc = eFunc;
	kCmdNew.m_uArg = uArg;
	m_kQue_kCmdNew.push_back(kCmdNew);

	//int nApplyGap=kCmdNew.m_dwChkTime-ELTimer_GetServerFrameMSec();

	//if (nApplyGap<-500 || nApplyGap>500)
	//	Tracenf("VID[%d] NAME[%s] 네트웍갭 [cur:%d ave:%d] 작동시간 (%d)", GetVirtualID(), GetNameString(), nNetworkGap, m_nAverageNetworkGap, nApplyGap);
}

BOOL CInstanceBase::__CanProcessNetworkStatePacket()
{
	if (m_GraphicThingInstance.IsDead())
		return FALSE;
	if (m_GraphicThingInstance.IsKnockDown())
		return FALSE;
	if (m_GraphicThingInstance.IsUsingSkill())
		if (!m_GraphicThingInstance.CanCancelSkill())
			return FALSE;

	return TRUE;
}

BOOL CInstanceBase::__IsEnableTCPProcess(UINT eCurFunc)
{
	if (m_GraphicThingInstance.IsActEmotion())
	{
		return FALSE;
	}

	if (!m_bEnableTCPState)
	{
		if (FUNC_EMOTION != eCurFunc)
		{
			return FALSE;
		}
	}

	return TRUE;
}

void CInstanceBase::StateProcess()
{	
	while (1)
	{
		if (m_kQue_kCmdNew.empty())
			return;	

		DWORD dwDstChkTime = m_kQue_kCmdNew.front().m_dwChkTime;
		DWORD dwCurChkTime = ELTimer_GetServerFrameMSec();	

		if (dwCurChkTime < dwDstChkTime)
			return;

		SCommand kCmdTop = m_kQue_kCmdNew.front();
		m_kQue_kCmdNew.pop_front();	

		TPixelPosition kPPosDst = kCmdTop.m_kPPosDst;
		//DWORD dwCmdTime = kCmdTop.m_dwCmdTime;
		FLOAT fRotDst = kCmdTop.m_fDstRot;
		UINT eFunc = kCmdTop.m_eFunc;
		UINT uArg = kCmdTop.m_uArg;
		UINT uVID = GetVirtualID();	
		UINT uTargetVID = kCmdTop.m_uTargetVID;

		TPixelPosition kPPosCur;
		NEW_GetPixelPosition(&kPPosCur);

		/*
		if (IsPC())
			Tracenf("%d cmd: vid=%d[%s] func=%d arg=%d  curPos=(%f, %f) dstPos=(%f, %f) rot=%f (time %d)", 
			ELTimer_GetMSec(),
			uVID, m_stName.c_str(), eFunc, uArg, 
			kPPosCur.x, kPPosCur.y,
			kPPosDst.x, kPPosDst.y, fRotDst, dwCmdTime-m_dwBaseCmdTime);
		*/

		TPixelPosition kPPosDir = kPPosDst - kPPosCur;
		float fDirLen = (float)sqrt(kPPosDir.x * kPPosDir.x + kPPosDir.y * kPPosDir.y);

		if (!__CanProcessNetworkStatePacket())
		{
			Lognf(0, "vid=%d 움직일 수 없는 상태라 스킵 IsDead=%d, IsKnockDown=%d", uVID, m_GraphicThingInstance.IsDead(), m_GraphicThingInstance.IsKnockDown());
			return;
		}

		if (!__IsEnableTCPProcess(eFunc))
		{
			return;
		}

		switch (eFunc)
		{
			case FUNC_WAIT:
			{
				if (uArg == 1) // Spawn animasyonu
				{
					m_isGoing = FALSE;
					if (!IsWaiting())
						EndWalking();
			
					SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
					SetAdvancingRotation(fRotDst);
					SetRotation(fRotDst);
			
					m_GraphicThingInstance.InterceptOnceMotion(CRaceMotionData::NAME_SPAWN);
				}
				else // Normal wait.msa
				{
					if (fDirLen > 1.0f)
					{
						NEW_SetSrcPixelPosition(kPPosCur);
						NEW_SetDstPixelPosition(kPPosDst);
			
						__EnableSkipCollision();
			
						m_fDstRot = fRotDst;
						m_isGoing = TRUE;
			
						m_kMovAfterFunc.eFunc = FUNC_WAIT;
			
						if (!IsWalking())
							StartWalking();
					}
					else
					{
						m_isGoing = FALSE;
						if (!IsWaiting())
							EndWalking();
			
						SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
						SetAdvancingRotation(fRotDst);
						SetRotation(fRotDst);
			
						// m_GraphicThingInstance.InterceptOnceMotion(CRaceMotionData::NAME_WAIT);
					}
				}
				break;
			}


			case FUNC_MOVE:
			{
				//NEW_GetSrcPixelPositionRef() = kPPosCur;
				//NEW_GetDstPixelPositionRef() = kPPosDst;
				NEW_SetSrcPixelPosition(kPPosCur);
				NEW_SetDstPixelPosition(kPPosDst);
				m_fDstRot = fRotDst;
				m_isGoing = TRUE;
				__EnableSkipCollision();
				//m_isSyncMov = TRUE;

				m_kMovAfterFunc.eFunc = FUNC_MOVE;

				if (!IsWalking())
				{
					//Tracen("걷고 있지 않아 걷기 시작");
					StartWalking();
				}
				else
				{
					//Tracen("이미 걷는중 ");
				}
				break;
			}

			case FUNC_COMBO:
			{
				if (fDirLen >= 50.0f)
				{
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);
					m_fDstRot=fRotDst;
					m_isGoing = TRUE;
					__EnableSkipCollision();

					m_kMovAfterFunc.eFunc = FUNC_COMBO;
					m_kMovAfterFunc.uArg = uArg;

					if (!IsWalking())
						StartWalking();
				}
				else
				{
					m_isGoing = FALSE;

					if (IsWalking())
						EndWalking();

					SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
					RunComboAttack(fRotDst, uArg);
				}
				break;
			}

			case FUNC_ATTACK:
			{
				if (fDirLen>=50.0f)
				{
					//NEW_GetSrcPixelPositionRef() = kPPosCur;
					//NEW_GetDstPixelPositionRef() = kPPosDst;
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);
					m_fDstRot = fRotDst;
					m_isGoing = TRUE;
					__EnableSkipCollision();
					//m_isSyncMov = TRUE;

					m_kMovAfterFunc.eFunc = FUNC_ATTACK;

					if (!IsWalking())
						StartWalking();
				}
				else
				{
					m_isGoing = FALSE;

					if (IsWalking())
						EndWalking();

					SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
					BlendRotation(fRotDst);

					RunNormalAttack(fRotDst);
				}
				break;
			}

			case FUNC_MOB_SKILL:
			{
				if (fDirLen >= 50.0f)
				{
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);
					m_fDstRot = fRotDst;
					m_isGoing = TRUE;
					__EnableSkipCollision();

					m_kMovAfterFunc.eFunc = FUNC_MOB_SKILL;
					m_kMovAfterFunc.uArg = uArg;

					if (!IsWalking())
						StartWalking();
				}
				else
				{
					m_isGoing = FALSE;

					if (IsWalking())
						EndWalking();

					SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
					BlendRotation(fRotDst);

					m_GraphicThingInstance.InterceptOnceMotion(CRaceMotionData::NAME_SPECIAL_1 + uArg);
				}
				break;
			}

			case FUNC_EMOTION:
			{
				if (fDirLen>100.0f)
				{
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);
					m_fDstRot = fRotDst;
					m_isGoing = TRUE;

					if (__IsMainInstance())
						__EnableSkipCollision();

					m_kMovAfterFunc.eFunc = FUNC_EMOTION;
					m_kMovAfterFunc.uArg = uArg;
					m_kMovAfterFunc.uArgExpanded = uTargetVID;
					m_kMovAfterFunc.kPosDst = kPPosDst;

					if (!IsWalking())
						StartWalking();
				}
				else
				{
					__ProcessFunctionEmotion(uArg, uTargetVID, kPPosDst);
				}
				break;
			}

			default:
			{
				if (eFunc & FUNC_SKILL)
				{
					if (fDirLen >= 50.0f)
					{
						//NEW_GetSrcPixelPositionRef() = kPPosCur;
						//NEW_GetDstPixelPositionRef() = kPPosDst;
						NEW_SetSrcPixelPosition(kPPosCur);
						NEW_SetDstPixelPosition(kPPosDst);
						m_fDstRot = fRotDst;
						m_isGoing = TRUE;
						//m_isSyncMov = TRUE;
						__EnableSkipCollision();

						m_kMovAfterFunc.eFunc = eFunc;
						m_kMovAfterFunc.uArg = uArg;

						if (!IsWalking())
							StartWalking();

					}
					else
					{
						m_isGoing = FALSE;

						if (IsWalking())
							EndWalking();

						SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
						SetAdvancingRotation(fRotDst);
						SetRotation(fRotDst);

						NEW_UseSkill(0, eFunc & 0x7f, uArg&0x0f, (uArg>>4) ? true : false);
					}
				}
				break;
			}
		}
	}
}


void CInstanceBase::MovementProcess()
{
	TPixelPosition kPPosCur;
	NEW_GetPixelPosition(&kPPosCur);

	TPixelPosition kPPosNext;
	{
		const D3DXVECTOR3 & c_rkV3Mov = m_GraphicThingInstance.GetMovementVectorRef();

		kPPosNext.x = kPPosCur.x + (+c_rkV3Mov.x);
		kPPosNext.y = kPPosCur.y + (-c_rkV3Mov.y);
		kPPosNext.z = kPPosCur.z + (+c_rkV3Mov.z);
	}

	TPixelPosition kPPosDeltaSC = kPPosCur - NEW_GetSrcPixelPositionRef();
	TPixelPosition kPPosDeltaSN = kPPosNext - NEW_GetSrcPixelPositionRef();
	TPixelPosition kPPosDeltaSD = NEW_GetDstPixelPositionRef() - NEW_GetSrcPixelPositionRef();

	float fCurLen = sqrtf(kPPosDeltaSC.x * kPPosDeltaSC.x + kPPosDeltaSC.y * kPPosDeltaSC.y);
	float fNextLen = sqrtf(kPPosDeltaSN.x * kPPosDeltaSN.x + kPPosDeltaSN.y * kPPosDeltaSN.y);
	float fTotalLen = sqrtf(kPPosDeltaSD.x * kPPosDeltaSD.x + kPPosDeltaSD.y * kPPosDeltaSD.y);
	float fRestLen = fTotalLen - fCurLen;

	if (__IsMainInstance())
	{
		if (m_isGoing && IsWalking())
		{
			float fDstRot = NEW_GetAdvancingRotationFromPixelPosition(NEW_GetSrcPixelPositionRef(), NEW_GetDstPixelPositionRef());

			SetAdvancingRotation(fDstRot);

			if (fRestLen<=0.0)
			{
				if (IsWalking())
					EndWalking();

				m_isGoing = FALSE;

				BlockMovement();

				if (FUNC_EMOTION == m_kMovAfterFunc.eFunc)
				{
					DWORD dwMotionNumber = m_kMovAfterFunc.uArg;
					DWORD dwTargetVID = m_kMovAfterFunc.uArgExpanded;
					__ProcessFunctionEmotion(dwMotionNumber, dwTargetVID, m_kMovAfterFunc.kPosDst);
					m_kMovAfterFunc.eFunc = FUNC_WAIT;
					return;
				}
			}
		}
	}
	else
	{
		if (m_isGoing && IsWalking())
		{
			float fDstRot = NEW_GetAdvancingRotationFromPixelPosition(NEW_GetSrcPixelPositionRef(), NEW_GetDstPixelPositionRef());

			SetAdvancingRotation(fDstRot);

			if (fRestLen < -100.0f)
			{
				NEW_SetSrcPixelPosition(kPPosCur);

				float fDstRot = NEW_GetAdvancingRotationFromPixelPosition(kPPosCur, NEW_GetDstPixelPositionRef());
				SetAdvancingRotation(fDstRot);

				if (FUNC_MOVE == m_kMovAfterFunc.eFunc)
				{
					m_kMovAfterFunc.eFunc = FUNC_WAIT;
				}
			}
			else if (fCurLen <= fTotalLen && fTotalLen <= fNextLen)
			{
				if (m_GraphicThingInstance.IsDead() || m_GraphicThingInstance.IsKnockDown())
				{
					__DisableSkipCollision();

					m_isGoing = FALSE;
				}
				else
				{
					switch (m_kMovAfterFunc.eFunc)
					{
						case FUNC_ATTACK:
						{
							if (IsWalking())
								EndWalking();

							__DisableSkipCollision();
							m_isGoing = FALSE;

							BlockMovement();
							SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
							SetAdvancingRotation(m_fDstRot);
							SetRotation(m_fDstRot);

							RunNormalAttack(m_fDstRot);
							break;
						}

						case FUNC_COMBO:
						{
							if (IsWalking())
								EndWalking();

							__DisableSkipCollision();
							m_isGoing = FALSE;

							BlockMovement();
							SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
							RunComboAttack(m_fDstRot, m_kMovAfterFunc.uArg);
							break;
						}

						case FUNC_EMOTION:
						{
							m_isGoing = FALSE;
							m_kMovAfterFunc.eFunc = FUNC_WAIT;
							__DisableSkipCollision();
							BlockMovement();

							DWORD dwMotionNumber = m_kMovAfterFunc.uArg;
							DWORD dwTargetVID = m_kMovAfterFunc.uArgExpanded;
							__ProcessFunctionEmotion(dwMotionNumber, dwTargetVID, m_kMovAfterFunc.kPosDst);
							break;
						}

						case FUNC_MOVE:
						{
							break;
						}

						case FUNC_MOB_SKILL:
						{
							if (IsWalking())
								EndWalking();

							__DisableSkipCollision();
							m_isGoing = FALSE;

							BlockMovement();
							SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
							SetAdvancingRotation(m_fDstRot);
							SetRotation(m_fDstRot);

							m_GraphicThingInstance.InterceptOnceMotion(CRaceMotionData::NAME_SPECIAL_1 + m_kMovAfterFunc.uArg);
							break;
						}

						default:
						{
							if (m_kMovAfterFunc.eFunc & FUNC_SKILL)
							{
								SetAdvancingRotation(m_fDstRot);
								BlendRotation(m_fDstRot);
								NEW_UseSkill(0, m_kMovAfterFunc.eFunc & 0x7f, m_kMovAfterFunc.uArg&0x0f, (m_kMovAfterFunc.uArg>>4) ? true : false);
							}
							else
							{
								//Tracenf("VID %d 스킬 공격 (%f, %f) rot %f", GetVirtualID(), NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y, m_fDstRot);

								__DisableSkipCollision();
								m_isGoing = FALSE;

								BlockMovement();
								SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
								SetAdvancingRotation(m_fDstRot);
								BlendRotation(m_fDstRot);
								if (!IsWaiting())
								{
									EndWalking();
								}

								//Tracenf("VID %d 정지 (%f, %f) rot %f IsWalking %d", GetVirtualID(), NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y, m_fDstRot, IsWalking());
							}
							break;
						}
					}

				}
			}

		}
	}

	if (IsWalking() || m_GraphicThingInstance.IsUsingMovingSkill())
	{
		float fRotation = m_GraphicThingInstance.GetRotation();
		float fAdvancingRotation = m_GraphicThingInstance.GetAdvancingRotation();
		int iDirection = GetRotatingDirection(fRotation, fAdvancingRotation);

		if (DEGREE_DIRECTION_SAME != m_iRotatingDirection)
		{
			if (DEGREE_DIRECTION_LEFT == iDirection)
			{
				fRotation = fmodf(fRotation + m_fRotSpd*m_GraphicThingInstance.GetSecondElapsed(), 360.0f);
			}
			else if (DEGREE_DIRECTION_RIGHT == iDirection)
			{
				fRotation = fmodf(fRotation - m_fRotSpd*m_GraphicThingInstance.GetSecondElapsed() + 360.0f, 360.0f);
			}

			if (m_iRotatingDirection != GetRotatingDirection(fRotation, fAdvancingRotation))
			{
				m_iRotatingDirection = DEGREE_DIRECTION_SAME;
				fRotation = fAdvancingRotation;
			}

			m_GraphicThingInstance.SetRotation(fRotation);
		}

		if (__IsInDustRange())
		{ 
			float fDustDistance = NEW_GetDistanceFromDestPixelPosition(m_kPPosDust);
			if (IsMountingHorse())
			{
				if (fDustDistance > ms_fHorseDustGap)
				{
					NEW_GetPixelPosition(&m_kPPosDust);
					__AttachEffect(EFFECT_HORSE_DUST);
				}
			}
			else
			{
				if (fDustDistance > ms_fDustGap)
				{
					NEW_GetPixelPosition(&m_kPPosDust);
					__AttachEffect(EFFECT_DUST);
				}
			}
		}
	}
}

void CInstanceBase::__ProcessFunctionEmotion(DWORD dwMotionNumber, DWORD dwTargetVID, const TPixelPosition & c_rkPosDst)
{
	if (IsWalking())
		EndWalkingWithoutBlending();

	__EnableChangingTCPState();
	SCRIPT_SetPixelPosition(c_rkPosDst.x, c_rkPosDst.y);

	CInstanceBase * pTargetInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwTargetVID);
	if (pTargetInstance)
	{
		pTargetInstance->__EnableChangingTCPState();

		if (pTargetInstance->IsWalking())
			pTargetInstance->EndWalkingWithoutBlending();

		WORD wMotionNumber1 = HIWORD(dwMotionNumber);
		WORD wMotionNumber2 = LOWORD(dwMotionNumber);

		int src_job = RaceToJob(GetRace());
		int dst_job = RaceToJob(pTargetInstance->GetRace());

		NEW_LookAtDestInstance(*pTargetInstance);
		m_GraphicThingInstance.InterceptOnceMotion(wMotionNumber1 + dst_job);
		m_GraphicThingInstance.SetRotation(m_GraphicThingInstance.GetTargetRotation());
		m_GraphicThingInstance.SetAdvancingRotation(m_GraphicThingInstance.GetTargetRotation());

		pTargetInstance->NEW_LookAtDestInstance(*this);
		pTargetInstance->m_GraphicThingInstance.InterceptOnceMotion(wMotionNumber2 + src_job);
		pTargetInstance->m_GraphicThingInstance.SetRotation(pTargetInstance->m_GraphicThingInstance.GetTargetRotation());
		pTargetInstance->m_GraphicThingInstance.SetAdvancingRotation(pTargetInstance->m_GraphicThingInstance.GetTargetRotation());

		if (pTargetInstance->__IsMainInstance())
		{
			IAbstractPlayer & rPlayer=IAbstractPlayer::GetSingleton();
			rPlayer.EndEmotionProcess();
		}
	}

	if (__IsMainInstance())
	{
		IAbstractPlayer & rPlayer=IAbstractPlayer::GetSingleton();
		rPlayer.EndEmotionProcess();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Update & Deform & Render

int g_iAccumulationTime = 0;

void CInstanceBase::Update()
{
	++ms_dwUpdateCounter;	

	StateProcess();
	m_GraphicThingInstance.PhysicsProcess();
	m_GraphicThingInstance.RotationProcess();
	m_GraphicThingInstance.ComboProcess();
	m_GraphicThingInstance.AccumulationMovement();

	if (m_GraphicThingInstance.IsMovement())
	{
		TPixelPosition kPPosCur;
		NEW_GetPixelPosition(&kPPosCur);

		DWORD dwCurTime=ELTimer_GetFrameMSec();
		//if (m_dwNextUpdateHeightTime<dwCurTime)
		{
			m_dwNextUpdateHeightTime=dwCurTime;
			kPPosCur.z = __GetBackgroundHeight(kPPosCur.x, kPPosCur.y);
			NEW_SetPixelPosition(kPPosCur);
		}

		// SetMaterialColor
		{
			DWORD dwMtrlColor=__GetShadowMapColor(kPPosCur.x, kPPosCur.y);
			m_GraphicThingInstance.SetMaterialColor(dwMtrlColor);
		}
	}

	m_GraphicThingInstance.UpdateAdvancingPointInstance();

#ifdef ENABLE_ANIMATION_OPTIMIZATION
	if (CPythonSystem::Instance().IsShowOtherCharAttacked())
		AttackProcess();
	else
		AttackProcessOnlySelf();
#else
	AttackProcess();
#endif
	MovementProcess();

	m_GraphicThingInstance.MotionProcess(IsPC());

#ifdef ENABLE_GRAPHIC_ON_OFF
	if (CPythonSystem::instance().GetEffectLevel() == 4)
	{
		m_GraphicThingInstance.SetDeactiveAllAttachingEffect();
	}
	else if (CPythonSystem::instance().GetEffectLevel() == 3)
	{
		if (!__IsMainInstance())
			m_GraphicThingInstance.SetDeactiveAllAttachingEffect();
		else
			if (!IsInvisibility() && !IsStealth())
				m_GraphicThingInstance.SetActiveAllAttachingEffect();
	}
	else if (CPythonSystem::instance().GetEffectLevel() == 2)
	{
		if (!IsPC())
			m_GraphicThingInstance.SetDeactiveAllAttachingEffect();
		else
			if (!IsInvisibility() && !IsStealth())
				m_GraphicThingInstance.SetActiveAllAttachingEffect();
	}
	else if (CPythonSystem::instance().GetEffectLevel() == 1)
	{
		if (!__IsMainInstance() || !IsEnemy() && !IsNPC())
			m_GraphicThingInstance.SetDeactiveAllAttachingEffect();
		else
			if (!IsInvisibility() && !IsStealth())
				m_GraphicThingInstance.SetActiveAllAttachingEffect();
	}
	else
	{
		if (!IsInvisibility() && !IsStealth())
			m_GraphicThingInstance.SetActiveAllAttachingEffect();
	}

	if (IsPet())
	{
		if (CPythonSystem::instance().IsPetStatus() == 1)
		{
			if (!IsAffect(AFFECT_INVISIBILITY))
			{
				__SetAffect(AFFECT_INVISIBILITY, true);
				m_kAffectFlagContainer.Set(AFFECT_INVISIBILITY, true);
			}
		}
		else
		{
			if (IsAffect(AFFECT_INVISIBILITY))
			{
				__SetAffect(AFFECT_INVISIBILITY, false);
				m_kAffectFlagContainer.Set(AFFECT_INVISIBILITY, false);
			}
		}
	}

	// if (IsShop())
	// {
		// if (CPythonSystem::instance().GetPrivateShopLevel() >= 3)
		// {
			// if (!IsAffect(AFFECT_INVISIBILITY))
			// {
				// __SetAffect(AFFECT_INVISIBILITY, true);
				// m_kAffectFlagContainer.Set(AFFECT_INVISIBILITY, true);
			// }
		// }
		// else
		// {
			// if (IsAffect(AFFECT_INVISIBILITY))
			// {
				// __SetAffect(AFFECT_INVISIBILITY, false);
				// m_kAffectFlagContainer.Set(AFFECT_INVISIBILITY, false);
			// }
		// }
	// }
#endif

#ifdef ENABLE_NEW_EMOTION
	if (m_GraphicThingInstance.__GetCurrentMotionIndex() < CRaceMotionData::NAME_CLAP || m_GraphicThingInstance.__GetCurrentMotionIndex() == CRaceMotionData::NAME_DIG)
	{
		if (m_GraphicThingInstance.GetPartItemID(CRaceData::PART_WEAPON) != m_awPart[CRaceData::PART_WEAPON])
		{
			m_GraphicThingInstance.AttachWeapon(m_awPart[CRaceData::PART_WEAPON]);
			CItemData* pItemData;
			if (IsAffect(AFFECT_GEOMGYEONG))
				__Warrior_SetGeomgyeongAffect(true);
			if (IsAffect(AFFECT_GWIGEOM))
				__AttachEffect(EFFECT_AFFECT + AFFECT_GWIGEOM);
		}
	}
	else if (m_GraphicThingInstance.GetPartItemID(CRaceData::PART_WEAPON))
	{
		m_GraphicThingInstance.AttachWeapon(0);
		__ClearWeaponRefineEffect();
		if (IsAffect(AFFECT_GEOMGYEONG))
			__Warrior_SetGeomgyeongAffect(false);
		if (IsAffect(AFFECT_GWIGEOM))
			__DetachEffect(EFFECT_AFFECT + AFFECT_GWIGEOM);
	}
#endif
	if (IsMountingHorse())
	{
		m_kHorse.m_pkActor->HORSE_MotionProcess(FALSE);
	}

	__ComboProcess();

	ProcessDamage();
}

void CInstanceBase::Transform()
{
	if (__IsSyncing())
	{
		//OnSyncing();
	}
	else
	{
		if (IsWalking() || m_GraphicThingInstance.IsUsingMovingSkill())
		{
			const D3DXVECTOR3& c_rv3Movment=m_GraphicThingInstance.GetMovementVectorRef();

			float len=(c_rv3Movment.x*c_rv3Movment.x)+(c_rv3Movment.y*c_rv3Movment.y);
			if (len>1.0f)
				OnMoving();
			else
				OnWaiting();
		}
	}

	if (IsAffect(AFFECT_TORNADO))
	{
		TPixelPosition kPPos;
		NEW_GetPixelPosition(&kPPos);

#ifdef ENABLE_GREEDY_ROOM
		if (m_dwFireWaveEffectID != 0)
		{
			CEffectManager::Instance().DestroyEffectInstance(m_dwFireWaveEffectID);
			m_dwFireWaveEffectID = 0;
		}

		if (m_dwGreedyTornadoEffectID2 == 0)
		{
			static DWORD dwGreedyTornadoCRC2 = 0;
			if (dwGreedyTornadoCRC2 == 0)
				CEffectManager::Instance().RegisterEffect2("d:/ymir work/effect/monster2/greedy_room_ground_01_tornado.mse", &dwGreedyTornadoCRC2);

			m_dwGreedyTornadoEffectID2 = CEffectManager::Instance().GetEmptyIndex();
			CEffectManager::Instance().CreateEffectInstance(m_dwGreedyTornadoEffectID2, dwGreedyTornadoCRC2);
		}

		if (m_dwGreedyTornadoEffectID2 != 0)
		{
			D3DXMATRIX matGlobal;
			D3DXMatrixTranslation(&matGlobal, kPPos.x, -kPPos.y, kPPos.z);

			CEffectManager::Instance().SelectEffectInstance(m_dwGreedyTornadoEffectID2);
			CEffectManager::Instance().SetEffectInstanceGlobalMatrix(matGlobal);
		}
#endif
	}
	else if (IsAffect(AFFECT_MOONLIGHT_TORNADO))
	{
		if (m_GraphicThingInstance.IsMoving())
		{
			m_GraphicThingInstance.Stop();
		}

		TPixelPosition kPPos;
		NEW_GetPixelPosition(&kPPos);

		float fCurTime = ELTimer_GetFrameMSec() / 1000.0f;
		float fSpin = fmod(fCurTime * (360.0f / 0.5f), 360.0f); 

		m_GraphicThingInstance.SetCurPixelPosition(TPixelPosition(kPPos.x, kPPos.y, kPPos.z + 150.0f));
		m_GraphicThingInstance.SetRotation(fSpin); 

		m_GraphicThingInstance.INSTANCEBASE_Transform();

#ifdef ENABLE_MOONLIGHT_VALLEY
		if (m_dwGreedyTornadoEffectID == 0)
		{
			static DWORD dwGreedyTornadoCRC = 0;
			if (dwGreedyTornadoCRC == 0)
				CEffectManager::Instance().RegisterEffect2("d:/ymir work/effect/monster2/golden_chicken_tornado.mse", &dwGreedyTornadoCRC);

			m_dwGreedyTornadoEffectID = CEffectManager::Instance().GetEmptyIndex();
			CEffectManager::Instance().CreateEffectInstance(m_dwGreedyTornadoEffectID, dwGreedyTornadoCRC);
		}

		if (m_dwGreedyTornadoEffectID != 0)
		{
			D3DXMATRIX matGlobal;
			D3DXMatrixTranslation(&matGlobal, kPPos.x, -kPPos.y, kPPos.z);

			CEffectManager::Instance().SelectEffectInstance(m_dwGreedyTornadoEffectID);
			CEffectManager::Instance().SetEffectInstanceGlobalMatrix(matGlobal);
		}
#endif
		m_GraphicThingInstance.SetCurPixelPosition(kPPos);
	}
#ifdef ENABLE_GREEDY_ROOM
	else if (IsAffect(AFFECT_FIRE_WAVE))
	{
		if (m_dwGreedyTornadoEffectID != 0)
		{
			CEffectManager::Instance().DestroyEffectInstance(m_dwGreedyTornadoEffectID);
			m_dwGreedyTornadoEffectID = 0;
		}
		if (m_dwGreedyTornadoEffectID2 != 0)
		{
			CEffectManager::Instance().DestroyEffectInstance(m_dwGreedyTornadoEffectID2);
			m_dwGreedyTornadoEffectID2 = 0;
		}

		TPixelPosition kPPos;
		NEW_GetPixelPosition(&kPPos);

		if (m_dwFireWaveEffectID == 0)
		{
			static DWORD dwFireWaveCRC = 0;
			if (dwFireWaveCRC == 0)
				CEffectManager::Instance().RegisterEffect2("d:/ymir work/effect/monster2/greedy_room_ground_03.mse", &dwFireWaveCRC);

			m_dwFireWaveEffectID = CEffectManager::Instance().GetEmptyIndex();
			CEffectManager::Instance().CreateEffectInstance(m_dwFireWaveEffectID, dwFireWaveCRC);
		}

		if (m_dwFireWaveEffectID != 0)
		{
			D3DXMATRIX matRot, matTranslation, matGlobal;
			D3DXMatrixRotationZ(&matRot, D3DXToRadian(-GetRotation()));
			D3DXMatrixTranslation(&matTranslation, kPPos.x, -kPPos.y, kPPos.z + 100.0f);
			D3DXMatrixMultiply(&matGlobal, &matRot, &matTranslation);

			CEffectManager::Instance().SelectEffectInstance(m_dwFireWaveEffectID);
			CEffectManager::Instance().SetEffectInstanceGlobalMatrix(matGlobal);
		}
	}
	else if (IsAffect(AFFECT_SPEAR_WARNING))
	{
		TPixelPosition kPPos;
		NEW_GetPixelPosition(&kPPos);

		if (m_dwSpearWarningEffectID == 0)
		{
			static DWORD dwSpearWarningCRC = 0;
			if (dwSpearWarningCRC == 0)
				CEffectManager::Instance().RegisterEffect2("d:/ymir work/effect/monster2/greedy_room_ground_02_arrow.mse", &dwSpearWarningCRC);

			m_dwSpearWarningEffectID = CEffectManager::Instance().GetEmptyIndex();
			CEffectManager::Instance().CreateEffectInstance(m_dwSpearWarningEffectID, dwSpearWarningCRC);
		}

		if (m_dwSpearWarningEffectID != 0)
		{
			D3DXMATRIX matRot, matTranslation, matGlobal;
			D3DXMatrixRotationZ(&matRot, D3DXToRadian(-GetRotation()));
			D3DXMatrixTranslation(&matTranslation, kPPos.x, -kPPos.y, kPPos.z + 100.0f);
			D3DXMatrixMultiply(&matGlobal, &matRot, &matTranslation);

			CEffectManager::Instance().SelectEffectInstance(m_dwSpearWarningEffectID);
			CEffectManager::Instance().SetEffectInstanceGlobalMatrix(matGlobal);
		}
	}
	else if (IsAffect(AFFECT_SPEAR_FIRE))
	{
		TPixelPosition kPPos;
		NEW_GetPixelPosition(&kPPos);

		if (m_dwSpearFireEffectID == 0)
		{
			static DWORD dwSpearFireCRC = 0;
			if (dwSpearFireCRC == 0)
				CEffectManager::Instance().RegisterEffect2("d:/ymir work/effect/monster2/greedy_room_ground_02_arrow_spear.mse", &dwSpearFireCRC);

			m_dwSpearFireEffectID = CEffectManager::Instance().GetEmptyIndex();
			CEffectManager::Instance().CreateEffectInstance(m_dwSpearFireEffectID, dwSpearFireCRC);
		}

		if (m_dwSpearFireEffectID != 0)
		{
			D3DXMATRIX matRot, matTranslation, matGlobal;
			D3DXMatrixRotationZ(&matRot, D3DXToRadian(-GetRotation()));
			D3DXMatrixTranslation(&matTranslation, kPPos.x, -kPPos.y, kPPos.z + 100.0f);
			D3DXMatrixMultiply(&matGlobal, &matRot, &matTranslation);

			CEffectManager::Instance().SelectEffectInstance(m_dwSpearFireEffectID);
			CEffectManager::Instance().SetEffectInstanceGlobalMatrix(matGlobal);
		}
	}
#endif
	else
	{
#ifdef ENABLE_GREEDY_ROOM
		if (m_dwGreedyTornadoEffectID != 0)
		{
			CEffectManager::Instance().DestroyEffectInstance(m_dwGreedyTornadoEffectID);
			m_dwGreedyTornadoEffectID = 0;
		}
		if (m_dwGreedyTornadoEffectID2 != 0)
		{
			CEffectManager::Instance().DestroyEffectInstance(m_dwGreedyTornadoEffectID2);
			m_dwGreedyTornadoEffectID2 = 0;
		}
		if (m_dwFireWaveEffectID != 0)
		{
			CEffectManager::Instance().DestroyEffectInstance(m_dwFireWaveEffectID);
			m_dwFireWaveEffectID = 0;
		}
		if (m_dwSpearWarningEffectID != 0)
		{
			CEffectManager::Instance().DestroyEffectInstance(m_dwSpearWarningEffectID);
			m_dwSpearWarningEffectID = 0;
		}
		if (m_dwSpearFireEffectID != 0)
		{
			CEffectManager::Instance().DestroyEffectInstance(m_dwSpearFireEffectID);
			m_dwSpearFireEffectID = 0;
		}
#endif
		m_GraphicThingInstance.INSTANCEBASE_Transform();
	}
}

void CInstanceBase::Deform()
{
	if (!__CanRender())
		return;

	++ms_dwDeformCounter;

	m_GraphicThingInstance.INSTANCEBASE_Deform();

	m_kHorse.Deform();
}

void CInstanceBase::RenderTrace()
{
	if (!__CanRender())
		return;

	m_GraphicThingInstance.RenderTrace();
}

void CInstanceBase::Render()
{
	if (!__CanRender())
		return;

	++ms_dwRenderCounter;

	m_kHorse.Render();
	m_GraphicThingInstance.Render();

#ifdef ENABLE_NINJA_MAP_FIX
	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();

	for (auto ptr = rkChrMgr.CharacterInstanceBegin(); ptr != rkChrMgr.CharacterInstanceEnd(); ++ptr)
	{
		CInstanceBase * pkInstEach = *ptr;

		if (pkInstEach)
		{
			if (pkInstEach->IsAffect(AFFECT_EUNHYEONG) || pkInstEach->IsAffect(AFFECT_INVISIBILITY))
			{
				if (CPythonPlayer::Instance().IsMainCharacterIndex(pkInstEach->GetVirtualID()))
					continue;

				pkInstEach->m_GraphicThingInstance.HideAllAttachingEffect();
			}
		}
	}
#endif

	if (CActorInstance::IsDirLine())
	{	
		if (NEW_GetDstPixelPositionRef().x != 0.0f)
		{
			static CScreen s_kScreen;

			STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_DIFFUSE);
			STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_SELECTARG1);
			STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_DISABLE);	
			STATEMANAGER.SaveRenderState(D3DRS_ZENABLE, FALSE);
			STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);
			STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
			
			TPixelPosition px;
			m_GraphicThingInstance.GetPixelPosition(&px);
			D3DXVECTOR3 kD3DVt3Cur(px.x, px.y, px.z);
			D3DXVECTOR3 kD3DVt3Dest(NEW_GetDstPixelPositionRef().x, -NEW_GetDstPixelPositionRef().y, NEW_GetDstPixelPositionRef().z);

			//printf("%s %f\n", GetNameString(), kD3DVt3Cur.y - kD3DVt3Dest.y);
			//float fdx = NEW_GetDstPixelPositionRef().x - NEW_GetSrcPixelPositionRef().x;
			//float fdy = NEW_GetDstPixelPositionRef().y - NEW_GetSrcPixelPositionRef().y;

			s_kScreen.SetDiffuseColor(0.0f, 0.0f, 1.0f);
			s_kScreen.RenderLine3d(kD3DVt3Cur.x, kD3DVt3Cur.y, px.z, kD3DVt3Dest.x, kD3DVt3Dest.y, px.z);
			STATEMANAGER.RestoreRenderState(D3DRS_ZENABLE);
			STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, TRUE);
			STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);
		}
	}	
}

void CInstanceBase::RenderToShadowMap()
{
	if (IsDoor())
		return;

	if (IsBuilding())
		return;

	if (!__CanRender())
		return;

	if (!__IsExistMainInstance())
		return;

	CInstanceBase * pkInstMain = __GetMainInstancePtr();

#ifndef ENABLE_SHADOW_RENDER_QUALITY_OPTION
	const float SHADOW_APPLY_DISTANCE = 2500.0f;
#endif

	float fDistance = NEW_GetDistanceFromDestInstance(*pkInstMain);
#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
	float fDistanceMax = CPythonBackground::Instance().GetShadowDistance();
	if (fDistance >= fDistanceMax)
#else
	if (fDistance>=SHADOW_APPLY_DISTANCE)
#endif
		return;

	m_GraphicThingInstance.RenderToShadowMap();
}

void CInstanceBase::RenderCollision()
{
	m_GraphicThingInstance.RenderCollisionData();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Setting & Getting Data

void CInstanceBase::SetVirtualID(DWORD dwVirtualID)
{
	m_GraphicThingInstance.SetVirtualID(dwVirtualID);
}

void CInstanceBase::SetVirtualNumber(DWORD dwVirtualNumber)
{
	m_dwVirtualNumber = dwVirtualNumber;
}

void CInstanceBase::SetInstanceType(int iInstanceType)
{
	m_GraphicThingInstance.SetActorType(iInstanceType);
}

void CInstanceBase::SetAlignment(short sAlignment)
{
	m_sAlignment = sAlignment;
	RefreshTextTailTitle();
}

#ifdef ENABLE_SUPPORT_SYSTEM
void CInstanceBase::SetSupportShaman(bool bTrue)
{
	is_support_shaman = bTrue;
}

bool CInstanceBase::IsSupportShaman()
{
	return is_support_shaman;
}
#endif

#ifdef ENABLE_TEXT_LEVEL_REFRESH
void CInstanceBase::SetLevel(DWORD dwLevel)
{
	m_dwLevel = dwLevel;
}
#endif

void CInstanceBase::SetLevelText(int sLevel)
{
	m_dwLevel = sLevel;
	UpdateTextTailLevel(sLevel);
}

#ifdef ENABLE_CONQUEROR_LEVEL
void CInstanceBase::SetConquerorLevelText(int sLevel)
{
	m_dwConquerorLevel = sLevel;
	UpdateTextTailConquerorLevel(sLevel);
}
#endif

void CInstanceBase::SetPKMode(BYTE byPKMode)
{
	if (m_byPKMode == byPKMode)
		return;

	m_byPKMode = byPKMode;

	if (__IsMainInstance())
	{
		IAbstractPlayer& rPlayer=IAbstractPlayer::GetSingleton();
		rPlayer.NotifyChangePKMode();
	}	
}

void CInstanceBase::SetKiller(bool bFlag)
{
	if (m_isKiller == bFlag)
		return;

	m_isKiller = bFlag;
	RefreshTextTail();
}

void CInstanceBase::SetPartyMemberFlag(bool bFlag)
{
	m_isPartyMember = bFlag;
}

void CInstanceBase::SetStateFlags(DWORD dwStateFlags)
{
	if (dwStateFlags & ADD_CHARACTER_STATE_KILLER)
		SetKiller(TRUE);
	else
		SetKiller(FALSE);

	if (dwStateFlags & ADD_CHARACTER_STATE_PARTY)
		SetPartyMemberFlag(TRUE);
	else
		SetPartyMemberFlag(FALSE);
}

#ifdef ENABLE_LEFT_SEAT
void CInstanceBase::SetLeftSeat(bool bLeftSeat)
{
	m_bLeftSeat = bLeftSeat;

	CPythonNetworkStream::Instance().SetLeftSeatLogOutState(bLeftSeat);
}
#endif

void CInstanceBase::SetComboType(UINT uComboType)
{
	m_GraphicThingInstance.SetComboType(uComboType);
}

const char * CInstanceBase::GetNameString()
{
	return m_stName.c_str();
}

#ifdef ENABLE_BATTLE_FIELD
bool CInstanceBase::IsCombatZoneMap()
{
	if (!strcmp(CPythonBackground::Instance().GetWarpMapName(), "metin2_map_battlefied"))
		return true;
	return false;
}

void CInstanceBase::SetCombatZonePoints(DWORD dwValue)
{
	combat_zone_points = dwValue;
}

DWORD CInstanceBase::GetCombatZonePoints()
{
	return combat_zone_points;
}

void CInstanceBase::SetCombatZoneRank(BYTE bValue)
{
	combat_zone_rank = bValue;
}

BYTE CInstanceBase::GetCombatZoneRank()
{
	return combat_zone_rank;
}
#endif

DWORD CInstanceBase::GetRace()
{
	return m_dwRace;
}

#ifdef ENABLE_NEW_EXCHANGE_WINDOW
DWORD CInstanceBase::GetLevel()
{
	return m_dwLevel;
}
#endif

bool CInstanceBase::IsConflictAlignmentInstance(CInstanceBase& rkInstVictim)
{
	if (PK_MODE_PROTECT == rkInstVictim.GetPKMode())
		return false;

	switch (GetAlignmentType())
	{
		case ALIGNMENT_TYPE_NORMAL:
		case ALIGNMENT_TYPE_WHITE:
			if (ALIGNMENT_TYPE_DARK == rkInstVictim.GetAlignmentType())
				return true;
			break;
		case ALIGNMENT_TYPE_DARK:
			if (GetAlignmentType() != rkInstVictim.GetAlignmentType())
				return true;
			break;
	}

	return false;
}

void CInstanceBase::SetDuelMode(DWORD type)
{
	m_dwDuelMode = type;
}

DWORD CInstanceBase::GetDuelMode()
{
	return m_dwDuelMode;
}

bool CInstanceBase::IsAttackableInstance(CInstanceBase& rkInstVictim)
{
	if (__IsMainInstance())
	{		
		CPythonPlayer& rkPlayer=CPythonPlayer::Instance();
		if(rkPlayer.IsObserverMode())
			return false;
	}

	if (GetVirtualID() == rkInstVictim.GetVirtualID())
		return false;

	if (IsStone())
	{
		if (rkInstVictim.IsPC())
			return true;
	}
	else if (IsPC())
	{

#ifdef ENABLE_SUPPORT_SYSTEM
		if (rkInstVictim.IsSupportShaman())
			return false;
#endif

		if (rkInstVictim.IsStone())
			return true;

		if (rkInstVictim.IsPC())
		{
			if (GetDuelMode())
			{
				switch(GetDuelMode())
				{
				case DUEL_CANNOTATTACK:
					return false;
				case DUEL_START:
					if(__FindDUELKey(GetVirtualID(),rkInstVictim.GetVirtualID()))
						return true;
					else
						return false;
				}
			}
			if (PK_MODE_GUILD == GetPKMode())
				if (GetGuildID() == rkInstVictim.GetGuildID())
					return false;

			if (rkInstVictim.IsKiller())
				if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
					return true;

			if (PK_MODE_PROTECT != GetPKMode())
			{
				if (PK_MODE_FREE == GetPKMode())
				{
					if (PK_MODE_PROTECT != rkInstVictim.GetPKMode())
						if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
							return true;
				}
				if (PK_MODE_GUILD == GetPKMode())
				{
					if (PK_MODE_PROTECT != rkInstVictim.GetPKMode())
						if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
							if (GetGuildID() != rkInstVictim.GetGuildID())
								return true;
				}
			}

			if (IsSameEmpire(rkInstVictim))	
			{
				if (IsPVPInstance(rkInstVictim))
					return true;

				if (PK_MODE_REVENGE == GetPKMode())
					if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
						if (IsConflictAlignmentInstance(rkInstVictim))
							return true;
			}
			else
			{
				return true;
			}
		}

		if (rkInstVictim.IsEnemy())
			return true;

		if (rkInstVictim.IsWoodenDoor())
			return true;
	}
	else if (IsEnemy())
	{
		if (rkInstVictim.IsPC())
			return true;

		if (rkInstVictim.IsBuilding())
			return true;

#ifdef ENABLE_SUPPORT_SYSTEM
		if (rkInstVictim.IsSupportShaman())
			return false;
#endif
	}
	else if (IsPoly())
	{
		if (rkInstVictim.IsPC())
			return true;

		if (rkInstVictim.IsEnemy())
			return true;

#ifdef ENABLE_SUPPORT_SYSTEM
		if (rkInstVictim.IsSupportShaman())
			return false;
#endif

	}
	return false;
}

bool CInstanceBase::IsTargetableInstance(CInstanceBase& rkInstVictim)
{
	return rkInstVictim.CanPickInstance();
}

bool CInstanceBase::CanChangeTarget()
{
	return m_GraphicThingInstance.CanChangeTarget();
}

bool CInstanceBase::CanPickInstance()
{
	if (!__IsInViewFrustum())
		return false;

	if (IsDoor())
	{
		if (IsDead())
			return false;
	}

	if (IsPC())
	{
		if (IsAffect(AFFECT_EUNHYEONG))
		{
			if (!__MainCanSeeHiddenThing())
				return false;
		}
		if (IsAffect(AFFECT_REVIVE_INVISIBILITY))
			return false;
		if (IsAffect(AFFECT_INVISIBILITY))
			return false;
	}

#ifdef ENABLE_BALATHOR_DUNGEON
	if (IsDead() && GetRace() != 20493)
#else
	if (IsDead())
#endif
		return false;

	return true;
}

#ifdef ENABLE_POISON_GAUGE_SYSTEM
bool CInstanceBase::IsPoisoned()
{
	if (IsAffect(AFFECT_POISON) && !IsStone())
		return true;
	else
		return false;
}
#endif

bool CInstanceBase::CanViewTargetHP(CInstanceBase& rkInstVictim)
{
	if (rkInstVictim.IsStone())
		return true;
	if (rkInstVictim.IsWoodenDoor())
		return true;
	if (rkInstVictim.IsEnemy())
		return true;
#ifdef ENABLE_SUPPORT_SYSTEM
	if (rkInstVictim.IsSupportShaman())
		return false;
#endif
#ifdef ENABLE_VIEW_TARGET_PLAYER_HP
	if (rkInstVictim.IsPC())
		return true;
#endif
	return false;
}

BOOL CInstanceBase::IsPoly()
{
	return m_GraphicThingInstance.IsPoly();
}

#ifdef ENABLE_OFFLINESHOP_SYSTEM
BOOL CInstanceBase::IsShop()
{
	return m_GraphicThingInstance.IsShop();
}
#endif

BOOL CInstanceBase::IsPC()
{
	return m_GraphicThingInstance.IsPC();
}

BOOL CInstanceBase::IsNPC()
{
	return m_GraphicThingInstance.IsNPC();
}

#ifdef ENABLE_GROWTH_PET_SYSTEM
BOOL CInstanceBase::IsGrowthPet()
{
	return m_GraphicThingInstance.IsGrowthPet();
}

void CInstanceBase::SetPetScale(int iLevel)
{
	if (!IsGrowthPet())
		return;

	m_dwLevel = iLevel;
	
	float fScale = 1.0f;

	const int TABLE_LEN = 15;

	struct SPetScaleTable {
		DWORD dwMinLevel;
		DWORD dwMaxLevel;
		float fScale;
	};
	const SPetScaleTable PetScaleTable[TABLE_LEN] =
	{
		{  0,  9, 0.60f },
		{ 10, 19, 0.70f },
		{ 20, 29, 0.75f },
		{ 30, 39, 0.80f },
		{ 40, 49, 0.90f },
		{ 50, 59, 0.95f },
		{ 60, 69, 1.00f },
		{ 70, 79, 1.10f },
		{ 80, 89, 1.20f },
		{ 90, 98, 1.25f },
		{ 99, 104, 1.30f },
		{ 105, 105, 1.50f },
	};

	for (int i = 0; i < TABLE_LEN; ++i) {
		if (m_dwLevel >= PetScaleTable[i].dwMinLevel && m_dwLevel <= PetScaleTable[i].dwMaxLevel) {
			fScale = PetScaleTable[i].fScale;
			break;
		}
	}

	if (m_dwConquerorLevel > 0 && m_dwLevel >= 105)
	{
		float add = 0.02f * float(m_dwConquerorLevel / 5);
		float newScale = fScale + add;
		if (newScale > 1.60f)
			newScale = 1.60f;
		fScale = newScale;
	}

	m_GraphicThingInstance.SetScale(fScale, fScale, fScale, true);
}
#endif

BOOL CInstanceBase::IsEnemy()
{
	return m_GraphicThingInstance.IsEnemy();
}

BOOL CInstanceBase::IsStone()
{
	return m_GraphicThingInstance.IsStone();
}

BOOL CInstanceBase::IsPet()
{
	if (GetRace() >= 34001 && GetRace() <= 34999)
		return true;
	return false;
}

// BOOL CInstanceBase::IsShop()
// {
	// if (GetRace() >= 30000 && GetRace() <= 30009)
		// return true;
	// return false;
// }

BOOL CInstanceBase::IsGuildWall()
{
	return IsWall(m_dwRace);
}

BOOL CInstanceBase::IsResource()
{
	switch (m_dwVirtualNumber)
	{
		case 20047:
		case 20048:
		case 20049:
		case 20050:
		case 20051:
		case 20052:
		case 20053:
		case 20054:
		case 20055:
		case 20056:
		case 20057:
		case 20058:
		case 20059:
		case 30301:
		case 30302:
		case 30303:
		case 30304:
		case 30305:
		case 30306:
		case 30309:
		case 30310:
		case 30311:
		case 30312:
		case 30313:
			return TRUE;
	}

	return FALSE;
}

BOOL CInstanceBase::IsWarp()
{
	return m_GraphicThingInstance.IsWarp();
}

BOOL CInstanceBase::IsGoto()
{
	return m_GraphicThingInstance.IsGoto();
}

BOOL CInstanceBase::IsObject()
{
	return m_GraphicThingInstance.IsObject();
}

BOOL CInstanceBase::IsBuilding()
{
	return m_GraphicThingInstance.IsBuilding();
}

BOOL CInstanceBase::IsDoor()
{
	return m_GraphicThingInstance.IsDoor();
}

BOOL CInstanceBase::IsWoodenDoor()
{
	if (m_GraphicThingInstance.IsDoor())
	{
		int vnum = GetVirtualNumber();
		if (vnum == 13000) // 나무문
			return true;
		else if (vnum >= 30111 && vnum <= 30119)
			return true;
		else
			return false;
	}
	else
	{
		return false;
	}
}

BOOL CInstanceBase::IsStoneDoor()
{
	return m_GraphicThingInstance.IsDoor() && 13001 == GetVirtualNumber();
}

BOOL CInstanceBase::IsFlag()
{
	if (GetRace() == 20035)
		return TRUE;
	if (GetRace() == 20036)
		return TRUE;
	if (GetRace() == 20037)
		return TRUE;

	return FALSE;
}

BOOL CInstanceBase::IsForceVisible()
{
	if (IsAffect(AFFECT_SHOW_ALWAYS))
		return TRUE;

	if (IsObject() || IsBuilding() || IsDoor() )
		return TRUE;

	return FALSE;
}

#ifdef ENABLE_EVENT_BANNER_FLAG
BOOL CInstanceBase::IsBannerFlag()
{
	if (GetRace() >= 20127 && GetRace() <= 20143)
		return TRUE;

	return FALSE;
}
#endif

int	CInstanceBase::GetInstanceType()
{
	return m_GraphicThingInstance.GetActorType();
}

DWORD CInstanceBase::GetVirtualID()
{
	return m_GraphicThingInstance.GetVirtualID();
}

DWORD CInstanceBase::GetVirtualNumber()
{
	return m_dwVirtualNumber;
}

bool CInstanceBase::__IsInViewFrustum()
{
#if defined(__ANDROID__)
	return true;
#else
	return m_GraphicThingInstance.isShow();
#endif
}

bool CInstanceBase::__CanRender()
{
	if (IsAlwaysRender())
	{
		return true;
	}

#if !defined(__ANDROID__)
	if (!__IsInViewFrustum())
		return false;
#endif

	if (IsAffect(AFFECT_INVISIBILITY))
		return false;

#ifdef ENABLE_OFFLINESHOP_SYSTEM
	CPythonSystem& pythonSystem = CPythonSystem::Instance();
	CInstanceBase* pkInstMain = __GetMainInstancePtr();
	if (pkInstMain != NULL && pkInstMain != this)
	{
		const float fDistance = NEW_GetDistanceFromDestInstance(*pkInstMain);
		if (IsShop())
		{
			const WORD shopRange[] = { 10000,6000,4000,2000,1000 };
			BYTE shopRangeIndex = pythonSystem.GetShopNamesRange();
			if (shopRangeIndex >= 5)
				shopRangeIndex = 0;
			if (fDistance > shopRange[shopRangeIndex])
			{
				m_GraphicThingInstance.SetDeactiveAllAttachingEffect();
				return false;
			}
		}
	}
#endif

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Graphic Control

bool CInstanceBase::IntersectBoundingBox()
{
	float u, v, t;
	return m_GraphicThingInstance.Intersect(&u, &v, &t);
}

bool CInstanceBase::IntersectDefendingSphere()
{
	return m_GraphicThingInstance.IntersectDefendingSphere();
}

float CInstanceBase::GetDistance(CInstanceBase * pkTargetInst)
{
	TPixelPosition TargetPixelPosition;
	pkTargetInst->m_GraphicThingInstance.GetPixelPosition(&TargetPixelPosition);
	return GetDistance(TargetPixelPosition);
}

float CInstanceBase::GetDistance(const TPixelPosition & c_rPixelPosition)
{
	TPixelPosition PixelPosition;
	m_GraphicThingInstance.GetPixelPosition(&PixelPosition);

	float fdx = PixelPosition.x - c_rPixelPosition.x;
	float fdy = PixelPosition.y - c_rPixelPosition.y;

	return sqrtf((fdx*fdx) + (fdy*fdy));
}

CActorInstance& CInstanceBase::GetGraphicThingInstanceRef()
{
	return m_GraphicThingInstance;
}

CActorInstance* CInstanceBase::GetGraphicThingInstancePtr()
{
	return &m_GraphicThingInstance;
}

void CInstanceBase::RefreshActorInstance()
{
	m_GraphicThingInstance.RefreshActorInstance();
}

void CInstanceBase::Refresh(DWORD dwMotIndex, bool isLoop)
{
	RefreshState(dwMotIndex, isLoop);
}

void CInstanceBase::RestoreRenderMode()
{
	m_GraphicThingInstance.RestoreRenderMode();
}

void CInstanceBase::SetAddRenderMode()
{
	m_GraphicThingInstance.SetAddRenderMode();
}

void CInstanceBase::SetModulateRenderMode()
{
	m_GraphicThingInstance.SetModulateRenderMode();
}

void CInstanceBase::SetRenderMode(int iRenderMode)
{
	m_GraphicThingInstance.SetRenderMode(iRenderMode);
}

void CInstanceBase::SetAddColor(const D3DXCOLOR & c_rColor)
{
	m_GraphicThingInstance.SetAddColor(c_rColor);
}

void CInstanceBase::__SetBlendRenderingMode()
{
	m_GraphicThingInstance.SetBlendRenderMode();
}

void CInstanceBase::__SetAlphaValue(float fAlpha)
{
	m_GraphicThingInstance.SetAlphaValue(fAlpha);
}

float CInstanceBase::__GetAlphaValue()
{
	return m_GraphicThingInstance.GetAlphaValue();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Part

void CInstanceBase::SetHair(DWORD eHair)
{
	if (!HAIR_COLOR_ENABLE)
		return;

	if (IsPC()==false)
		return;
	m_awPart[CRaceData::PART_HAIR] = eHair;
	m_GraphicThingInstance.SetHair(eHair);
}

void CInstanceBase::ChangeHair(DWORD eHair)
{
	if (!HAIR_COLOR_ENABLE)
		return;

	if (IsPC()==false)
		return;

	if (GetPart(CRaceData::PART_HAIR)==eHair)
		return;

	SetHair(eHair);

	//int type = m_GraphicThingInstance.GetMotionMode();

	RefreshState(CRaceMotionData::NAME_WAIT, true);
	//RefreshState(type, true);
}

void CInstanceBase::SetArmor(DWORD dwArmor)
{
	DWORD dwShape;
	if (__ArmorVnumToShape(dwArmor, &dwShape))
	{
		CItemData * pItemData;
		if (CItemManager::Instance().GetItemDataPointer(dwArmor, &pItemData))
		{
			float fSpecularPower=pItemData->GetSpecularPowerf();
			SetShape(dwShape, fSpecularPower);
			__GetRefinedEffect(pItemData);
			return;
		}
		else
			__ClearArmorRefineEffect();
	}

	SetShape(dwArmor);
}

#ifdef ENABLE_AURA_SYSTEM
bool CInstanceBase::SetAura(DWORD eAura)
{
	if (!IsPC() || IsPoly() || __IsShapeAnimalWear())
		return false;

	// TraceError("DEBUG_INSTANCE_AURA: name:%s vnum:%u", GetNameString(), eAura);

	//m_GraphicThingInstance.ChangePart(CRaceData::PART_AURA, eAura);
	if (!eAura)
	{
		if (m_auraRefineEffect)
		{
			__DetachEffect(m_auraRefineEffect);
			m_auraRefineEffect = 0;
		}
		m_awPart[CRaceData::PART_AURA] = 0;
		return true;
	}

	CItemData* pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(eAura, &pItemData))
	{
		TraceError("DEBUG_INSTANCE_AURA_FAIL: item data not found for %u", eAura);
		if (m_auraRefineEffect)
		{
			__DetachEffect(m_auraRefineEffect);
			m_auraRefineEffect = 0;
		}
		m_awPart[CRaceData::PART_AURA] = 0;
		return true;
	}
	
	// TraceError("DEBUG_INSTANCE_AURA_SUCCESS: item:%u effect:%u", eAura, pItemData->GetAuraEffectID());
	
	ClearAuraEffect();

	BYTE byRace = (BYTE)GetRace();
	BYTE byJob = (BYTE)RaceToJob(byRace);
	BYTE bySex = (BYTE)RaceToSex(byRace);

	byJob += 1;
	if (bySex == 0)
		byJob += 5;

	m_GraphicThingInstance.AttachAura(pItemData);

	float fScaleX, fScaleY, fScaleZ, fPositionX, fPositionY, fPositionZ;
	if (pItemData->GetItemScale(byJob, fScaleX, fScaleY, fScaleZ, fPositionX, fPositionY, fPositionZ))
	{
		if (m_kHorse.IsMounting() && byJob != NRaceData::JOB_WOLFMAN)
			fPositionZ += 15.0f;

		// D3DXVECTOR3 * sendPosition;
		// sendPosition->x = fPositionX;
		// sendPosition->y = fPositionY;
		// sendPosition->z = fPositionZ;
	}

	m_auraRefineEffect = m_GraphicThingInstance.AttachEffectByID(NULL, "Bip01 Spine2", pItemData->GetAuraEffectID(), NULL);
	m_awPart[CRaceData::PART_AURA] = eAura;
	//__EffectContainer_AttachEffect(m_auraRefineEffect);
	
	return true;
}

void CInstanceBase::ChangeAura(DWORD eAura)
{
	if (!IsPC())
		return;
	
	SetAura(eAura);
}

void CInstanceBase::ClearAuraEffect()
{
	if (!m_auraRefineEffect)
		return;
	
	__DetachEffect(m_auraRefineEffect);
	m_auraRefineEffect = 0;
}
#endif

#ifdef ENABLE_SASH_SYSTEM
void CInstanceBase::SetSash(DWORD dwSash)
{
	if (!IsPC())
		return;
	
	if (IsPoly())
		return;
	
	if (dwSash == 0) {
		m_awPart[CRaceData::PART_SASH] = 0;
		m_GraphicThingInstance.AttachSash(0, 0.0f);
		ClearSashEffect();
		return;
	}

	dwSash += 85000;
	ClearSashEffect();
	
	float fSpecular = 65.0f;
	if (dwSash > 87000)
	{
		dwSash -= 2000;
		fSpecular += 35;
		
		m_dwSashEffect = EFFECT_REFINED + EFFECT_SASH;
		__EffectContainer_AttachEffect(m_dwSashEffect);
	}
	
	fSpecular /= 100.0f;
	m_awPart[CRaceData::PART_SASH] = dwSash;
	
	CItemData * pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(dwSash, &pItemData))
		return;
	
	m_GraphicThingInstance.AttachSash(pItemData, fSpecular);

#ifdef ENABLE_OBJ_SCALLING
	DWORD dwRace = GetRace(), dwPos = RaceToJob(dwRace), dwSex = RaceToSex(dwRace);
	dwPos += 1;
	if (dwSex == 0)
		dwPos += 5;

	float fScaleX, fScaleY, fScaleZ, fPositionX, fPositionY, fPositionZ;
	if (pItemData && pItemData->GetItemScale(dwPos, fScaleX, fScaleY, fScaleZ, fPositionX, fPositionY, fPositionZ))
	{
		m_GraphicThingInstance.SetScale(fScaleX, fScaleY, fScaleZ, true);
		if (m_kHorse.IsMounting())
			fPositionZ += 10.0f;

		m_GraphicThingInstance.SetScalePosition(fPositionX, fPositionY, fPositionZ);
	}
#endif
}

void CInstanceBase::ChangeSash(DWORD dwSash)
{
	if (!IsPC())
		return;
	
	SetSash(dwSash);
}

void CInstanceBase::ClearSashEffect()
{
	if (!m_dwSashEffect)
		return;
	
	__EffectContainer_DetachEffect(m_dwSashEffect);
	m_dwSashEffect = 0;
}
#endif

void CInstanceBase::SetShape(DWORD eShape, float fSpecular)
{
	if (IsPoly())
	{
		m_GraphicThingInstance.SetShape(0);
	}
	else
	{
		m_GraphicThingInstance.SetShape(eShape, fSpecular);
	}

	m_eShape = eShape;
}

DWORD CInstanceBase::GetWeaponType()
{
	DWORD dwWeapon = GetPart(CRaceData::PART_WEAPON);
	CItemData * pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(dwWeapon, &pItemData))
		return CItemData::WEAPON_NONE;

#ifdef ENABLE_COSTUME_WEAPON_SYSTEM
	if (pItemData->GetType() == CItemData::ITEM_TYPE_COSTUME)
		return pItemData->GetValue(3);
#endif

	return pItemData->GetWeaponType();
}

void CInstanceBase::__ClearWeaponRefineEffect()
{
	if (m_swordRefineEffectRight)
	{
		__DetachEffect(m_swordRefineEffectRight);
		m_swordRefineEffectRight = 0;
	}
	if (m_swordRefineEffectLeft)
	{
		__DetachEffect(m_swordRefineEffectLeft);
		m_swordRefineEffectLeft = 0;
	}
}

void CInstanceBase::__ClearArmorRefineEffect()
{
	if (m_armorRefineEffect)
	{
		__DetachEffect(m_armorRefineEffect);
		m_armorRefineEffect = 0;
	}
}

UINT CInstanceBase::__GetRefinedEffect(CItemData* pItem)
{
	DWORD refine = pItem->GetRefine();
	DWORD vnum = pItem->GetIndex();
	bool bIsCustomEffect = false;
#ifdef ENABLE_REFINE_ELEMENT
	BYTE bRefineElement = GetRefineElementType();
#endif
	switch (pItem->GetType())
	{
	case CItemData::ITEM_TYPE_WEAPON:
		__ClearWeaponRefineEffect();
#ifdef ENABLE_OFFICIAL_19_3_ITEMS
		if ((vnum >= 330 && vnum <= 335 ) || (vnum >= 350 && vnum <= 355) || (vnum >= 370 && vnum <= 375) || (vnum >= 390 && vnum <= 395))
		{
			m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_WEAPON_SWORD_SPECIAL_19_3;
			bIsCustomEffect = true;
		}
		else if ((vnum >= 1200 && vnum <= 1205 ) || (vnum >= 1220 && vnum <= 1225))
		{
			m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_WEAPON_DAGGER_SPECIAL_19_3; //BI?K
			m_swordRefineEffectLeft = EFFECT_REFINED+EFFECT_WEAPON_DAGGER_SPECIAL_19_3_LEFT; //BI?K
			bIsCustomEffect = true;
		}
		else if ((vnum >= 2220 && vnum <= 2225 ) || (vnum >= 2240 && vnum <= 2245))
		{
			m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_WEAPON_BOW_SPECIAL_19_3; //YAY
			bIsCustomEffect = true;
		}
		else if ((vnum >= 3240 && vnum <= 3245) || (vnum >= 3260 && vnum <= 3265))
		{
			m_swordRefineEffectLeft = EFFECT_REFINED+EFFECT_WEAPON_TWO_HANDED_SPECIAL_19_3; //?ft el
			bIsCustomEffect = true;
		}
		else if ((vnum >= 5180 && vnum <= 5185) || (vnum >= 5210 && vnum <= 5215))
		{
			m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_WEAPON_BELL_SPECIAL_19_3; //?N
			bIsCustomEffect = true;
		}
		else if ((vnum >= 7320 && vnum <= 7325 ) || (vnum >= 7340 && vnum <= 7345))
		{
			m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_WEAPON_FAN_SPECIAL_19_3; //YELPAZAE
			bIsCustomEffect = true;
		}
		else if ((vnum >= 6140 && vnum <= 6145 ) || (vnum >= 6160 && vnum <= 6165 ))
		{
			m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_WEAPON_CLAW_SPECIAL_19_3; //PENCE
			m_swordRefineEffectLeft = EFFECT_REFINED+EFFECT_WEAPON_CLAW_SPECIAL_19_3_LEFT; //PENCE
			bIsCustomEffect = true;
		}
#endif

#ifdef ENABLE_GREEDY_ROOM
		if ((vnum >= 410 && vnum <= 415) || (vnum >= 430 && vnum <= 435))
		{
			m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_WEAPON_MOONLIGHT_SWORD;
			bIsCustomEffect = true;
		}
		else if (vnum >= 1240 && vnum <= 1245)
		{
			m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_WEAPON_MOONLIGHT_DAGGER_RIGHT;
			m_swordRefineEffectLeft = EFFECT_REFINED+EFFECT_WEAPON_MOONLIGHT_DAGGER_LEFT;
			bIsCustomEffect = true;
		}
		else if (vnum >= 2260 && vnum <= 2265)
		{
			m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_WEAPON_MOONLIGHT_BOW;
			bIsCustomEffect = true;
		}
		else if (vnum >= 3280 && vnum <= 3285)
		{
			m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_WEAPON_MOONLIGHT_TWO_HANDED;
			bIsCustomEffect = true;
		}
		else if (vnum >= 5230 && vnum <= 5235)
		{
			m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_WEAPON_MOONLIGHT_BELL;
			bIsCustomEffect = true;
		}
		else if (vnum >= 7360 && vnum <= 7365)
		{
			m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_WEAPON_MOONLIGHT_FAN;
			bIsCustomEffect = true;
		}
		else if (vnum >= 6180 && vnum <= 6185)
		{
			m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_WEAPON_MOONLIGHT_CLAW_RIGHT;
			m_swordRefineEffectLeft = EFFECT_REFINED+EFFECT_WEAPON_MOONLIGHT_CLAW_LEFT;
			bIsCustomEffect = true;
		}
#endif

#ifdef ENABLE_REFINE_ELEMENT
		if (refine < 7 && !bRefineElement && !bIsCustomEffect)
#else
		if (refine < 7 && !bIsCustomEffect)
#endif
			return 0;

#ifdef ENABLE_REFINE_ELEMENT
		if(bRefineElement)
		{
			BYTE bRealRefineElement = bRefineElement - 1;
			switch(pItem->GetSubType())
			{
				case CItemData::WEAPON_DAGGER:
					m_swordRefineEffectRight = EFFECT_REFINE_ELEMENT + ((bRealRefineElement * EFFECT_REFINE_ELEMENT_WEAPON_MAX) + EFFECT_REFINE_ELEMENT_WEAPON_DAGGER);
					m_swordRefineEffectLeft = EFFECT_REFINE_ELEMENT + ((bRealRefineElement * EFFECT_REFINE_ELEMENT_WEAPON_MAX) + EFFECT_REFINE_ELEMENT_WEAPON_DAGGER_LEFT);
					break;
					
				case CItemData::WEAPON_FAN:
					m_swordRefineEffectRight = EFFECT_REFINE_ELEMENT + ((bRealRefineElement * EFFECT_REFINE_ELEMENT_WEAPON_MAX) + EFFECT_REFINE_ELEMENT_WEAPON_FAN);
					break;
					
				case CItemData::WEAPON_ARROW:
				case CItemData::WEAPON_BELL:
					m_swordRefineEffectRight = EFFECT_REFINE_ELEMENT + ((bRealRefineElement * EFFECT_REFINE_ELEMENT_WEAPON_MAX) + EFFECT_REFINE_ELEMENT_WEAPON_DAGGER);
					break;
				case CItemData::WEAPON_BOW:
					m_swordRefineEffectRight = EFFECT_REFINE_ELEMENT + ((bRealRefineElement * EFFECT_REFINE_ELEMENT_WEAPON_MAX) + EFFECT_REFINE_ELEMENT_WEAPON_BOW);
					break;
#ifdef ENABLE_WOLFMAN_CHARACTER
				case CItemData::WEAPON_CLAW:
					m_swordRefineEffectRight = EFFECT_REFINE_ELEMENT + ((bRealRefineElement * EFFECT_REFINE_ELEMENT_WEAPON_MAX) + EFFECT_REFINE_ELEMENT_WEAPON_CLAW);
					m_swordRefineEffectLeft = EFFECT_REFINE_ELEMENT + ((bRealRefineElement * EFFECT_REFINE_ELEMENT_WEAPON_MAX) + EFFECT_REFINE_ELEMENT_WEAPON_CLAW_LEFT);
					break;
#endif
				default:
					m_swordRefineEffectRight = EFFECT_REFINE_ELEMENT + ((bRealRefineElement * EFFECT_REFINE_ELEMENT_WEAPON_MAX) + EFFECT_REFINE_ELEMENT_WEAPON_SWORD);
					break;
			}
		}
		else
		{
#endif
			if (!bIsCustomEffect)
			{
				switch(pItem->GetSubType())
				{
					case CItemData::WEAPON_DAGGER:
						m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_SMALLSWORD_REFINED7+refine-7;
						m_swordRefineEffectLeft = EFFECT_REFINED+EFFECT_SMALLSWORD_REFINED7_LEFT+refine-7;
						break;
					case CItemData::WEAPON_FAN:
						m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_FANBELL_REFINED7+refine-7;
						break;
					case CItemData::WEAPON_ARROW:
					case CItemData::WEAPON_BELL:
						m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_SMALLSWORD_REFINED7+refine-7;
						break;
					case CItemData::WEAPON_BOW:
						m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_BOW_REFINED7+refine-7;
						break;
#ifdef ENABLE_WOLFMAN_CHARACTER
					case CItemData::WEAPON_CLAW:
						m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_CLAW_REFINED7+refine-7;
						m_swordRefineEffectLeft = EFFECT_REFINED+EFFECT_CLAW_REFINED7_LEFT+refine-7;
						break;
#endif
					default:
						m_swordRefineEffectRight = EFFECT_REFINED+EFFECT_SWORD_REFINED7+refine-7;
						break;
				}
			}
#ifdef ENABLE_REFINE_ELEMENT
		}
#endif
		if (m_swordRefineEffectRight)
			m_swordRefineEffectRight = __AttachEffect(m_swordRefineEffectRight);
		if (m_swordRefineEffectLeft)
			m_swordRefineEffectLeft = __AttachEffect(m_swordRefineEffectLeft);
		break;

		case CItemData::ITEM_TYPE_ARMOR:
			
			if (pItem->GetSubType() == CItemData::ARMOR_BODY)
			{
				DWORD vnum = pItem->GetIndex();
				if ((12010 <= vnum && vnum <= 12049 ) || (21080 <= vnum && vnum <= 21089))
				{
					__AttachEffect(EFFECT_REFINED+EFFECT_BODYARMOR_SPECIAL);
					__AttachEffect(EFFECT_REFINED+EFFECT_BODYARMOR_SPECIAL2);
				}
#ifdef VERSION_162_ENABLED
				if (vnum >= 20760 && vnum <= 20959)
				{
					__AttachEffect(EFFECT_REFINED + EFFECT_BODYARMOR_SPECIAL3);
					break;
				}
#endif
				if (vnum >= 20010 && vnum <= 20019)
				{
					__AttachEffect(EFFECT_REFINED + EFFECT_BODYARMOR_SPECIAL4);
					break;
				}

#ifdef ENABLE_OFFICIAL_19_3_ITEMS
				if (vnum >= 21220 && vnum <= 21225
				|| vnum >= 21240 && vnum <= 21245
				|| vnum >= 21260 && vnum <= 21265
				|| vnum >= 21280 && vnum <= 21285
				|| vnum >= 21300 && vnum <= 21305
				
				|| vnum >= 21320 && vnum <= 21325
				|| vnum >= 21340 && vnum <= 21345
				|| vnum >= 21360 && vnum <= 21365
				|| vnum >= 21380 && vnum <= 21385
				|| vnum >= 21400 && vnum <= 21405)
				{
					__AttachEffect(EFFECT_REFINED + EFFECT_BODYARMOR_SPECIAL_19_3);
					break;
				}
#endif
			}

			if (refine < 7)
				return 0;

			if (pItem->GetSubType() == CItemData::ARMOR_BODY)
			{
				m_armorRefineEffect = EFFECT_REFINED+EFFECT_BODYARMOR_REFINED7+refine-7;
				__AttachEffect(m_armorRefineEffect);
			}
			break;
	}

	return 0;
}

#ifdef ENABLE_NEW_ARROW_SYSTEM
bool CInstanceBase::SetWeapon(DWORD eWeapon, DWORD eArrow)
#else
bool CInstanceBase::SetWeapon(DWORD eWeapon)
#endif
{
	if (IsPoly())
		return false;
	
	if (__IsShapeAnimalWear())
		return false;
	
	if (__IsChangableWeapon(eWeapon) == false)
		eWeapon = 0;

	m_GraphicThingInstance.AttachWeapon(eWeapon);
	m_awPart[CRaceData::PART_WEAPON] = eWeapon;
#ifdef ENABLE_NEW_ARROW_SYSTEM
	m_awPart[CRaceData::PART_ARROW_TYPE] = eArrow;
#endif
	
	CItemData * pItemData;
	if (CItemManager::Instance().GetItemDataPointer(eWeapon, &pItemData))
	{
#ifdef ENABLE_COSTUME_WEAPON_SYSTEM
		if (pItemData->GetType() == CItemData::ITEM_TYPE_COSTUME)
			__ClearWeaponRefineEffect();
#endif
		__GetRefinedEffect(pItemData);
	}
	else
		__ClearWeaponRefineEffect();

	return true;
}

#ifdef ENABLE_NEW_ARROW_SYSTEM
void CInstanceBase::ChangeWeapon(DWORD eWeapon, DWORD eArrow)
#else
void CInstanceBase::ChangeWeapon(DWORD eWeapon)
#endif
{
#ifdef ENABLE_NEW_ARROW_SYSTEM
	CItemData * pItemData;
	m_awPart[CRaceData::PART_ARROW_TYPE] = eArrow;
	
	if (CItemManager::Instance().GetItemDataPointer(eWeapon, &pItemData))
	{
		if (!m_kHorse.IsMounting() && pItemData->GetSubType() == CItemData::WEAPON_BOW)
		{
			if (eArrow == CItemData::WEAPON_UNLIMITED_ARROW)
				SetMotionMode(CRaceMotionData::MODE_BOW_SPECIAL);
			else
				SetMotionMode(CRaceMotionData::MODE_BOW);
		}
	}
#endif
	if (eWeapon == m_GraphicThingInstance.GetPartItemID(CRaceData::PART_WEAPON))
		return;
#ifdef ENABLE_NEW_ARROW_SYSTEM
	if (SetWeapon(eWeapon, eArrow))
#else
	if (SetWeapon(eWeapon))
#endif
		RefreshState(CRaceMotionData::NAME_WAIT, true);
}

bool CInstanceBase::ChangeArmor(DWORD dwArmor)
{
	DWORD eShape;
	__ArmorVnumToShape(dwArmor, &eShape);

	if (GetShape()==eShape)
		return false;

	CAffectFlagContainer kAffectFlagContainer;
	kAffectFlagContainer.CopyInstance(m_kAffectFlagContainer);

	DWORD dwVID = GetVirtualID();
	DWORD dwRace = GetRace();
	DWORD eHair = GetPart(CRaceData::PART_HAIR);
#ifdef ENABLE_SASH_SYSTEM
	DWORD dwSash = GetPart(CRaceData::PART_SASH);
#endif
	DWORD eWeapon = GetPart(CRaceData::PART_WEAPON);
#ifdef ENABLE_NEW_ARROW_SYSTEM
	DWORD eArrow = GetPart(CRaceData::PART_ARROW_TYPE);
#endif
#ifdef ENABLE_AURA_SYSTEM
	DWORD eAura = GetPart(CRaceData::PART_AURA);
#endif
	float fRot = GetRotation();
	float fAdvRot = GetAdvancingRotation();

	if (IsWalking())
		EndWalking();

	__ClearAffects();

	if (!SetRace(dwRace))
	{
		TraceError("CPythonCharacterManager::ChangeArmor - SetRace VID[%d] Race[%d] ERROR", dwVID, dwRace);
		return false;
	}

	SetArmor(dwArmor);
	SetHair(eHair);
#ifdef ENABLE_SASH_SYSTEM
	SetSash(dwSash);
#endif
#ifdef ENABLE_NEW_ARROW_SYSTEM
	SetWeapon(eWeapon, eArrow);
#else
	SetWeapon(eWeapon);
#endif
#ifdef ENABLE_AURA_SYSTEM
	SetAura(eAura);
#endif

	SetRotation(fRot);
	SetAdvancingRotation(fAdvRot);

	__AttachHorseSaddle();

	RefreshState(CRaceMotionData::NAME_WAIT, TRUE);

	SetAffectFlagContainer(kAffectFlagContainer);

	CActorInstance::IEventHandler& rkEventHandler=GetEventHandlerRef();
	rkEventHandler.OnChangeShape();

	return true;
}

bool CInstanceBase::__IsShapeAnimalWear()
{
	if (100 == GetShape() ||
		101 == GetShape() ||
		102 == GetShape() ||
		103 == GetShape())
		return true;

	return false;
}

DWORD CInstanceBase::__GetRaceType()
{
	return m_eRaceType;
}

void CInstanceBase::RefreshState(DWORD dwMotIndex, bool isLoop)
{
	DWORD dwPartItemID = m_GraphicThingInstance.GetPartItemID(CRaceData::PART_WEAPON);

	BYTE byItemType = 0xff;
	BYTE bySubType = 0xff;

	CItemManager & rkItemMgr = CItemManager::Instance();
	CItemData * pItemData;
	if (rkItemMgr.GetItemDataPointer(dwPartItemID, &pItemData))
	{
		byItemType = pItemData->GetType();
		bySubType = pItemData->GetWeaponType();
	}

	if (IsPoly())
	{
		SetMotionMode(CRaceMotionData::MODE_GENERAL);
	}
	else if (IsWearingDress())
	{
		SetMotionMode(CRaceMotionData::MODE_WEDDING_DRESS);
	}
	else if (IsHoldingPickAxe())
	{
		if (m_kHorse.IsMounting())
		{
#ifdef ENABLE_STANDING_MOUNT
			if (m_kHorse.IsHoverBoard())
			{
				SetMotionMode(CRaceMotionData::MODE_HORSE_STAND);
			}
			else
				SetMotionMode(CRaceMotionData::MODE_HORSE);
#else
			SetMotionMode(CRaceMotionData::MODE_HORSE);
#endif
		}
		else
		{
			SetMotionMode(CRaceMotionData::MODE_GENERAL);
		}
	}
	else if (CItemData::ITEM_TYPE_ROD == byItemType)
	{
#ifdef ENABLE_STANDING_MOUNT
		if (m_kHorse.IsMounting())
		{
			if (m_kHorse.IsHoverBoard())
				SetMotionMode(CRaceMotionData::MODE_HORSE_STAND_FISHING);
			else
				SetMotionMode(CRaceMotionData::MODE_HORSE);
		}
		else
		{
			SetMotionMode(CRaceMotionData::MODE_FISHING);
		}
#else
		if (m_kHorse.IsMounting())
		{
			SetMotionMode(CRaceMotionData::MODE_HORSE);
		}
		else
		{
			SetMotionMode(CRaceMotionData::MODE_FISHING);
		}
#endif
	}
#ifdef ENABLE_COSTUME_WEAPON_SYSTEM
	else if (byItemType == CItemData::ITEM_TYPE_COSTUME)
	{
#ifdef ENABLE_STANDING_MOUNT
		if (m_kHorse.IsHoverBoard())
			switch (pItemData->GetValue(3))
			{
				case CItemData::WEAPON_SWORD:
						SetMotionMode(CRaceMotionData::MODE_HORSE_STAND_SWORD);
					break;
				case CItemData::WEAPON_DAGGER:
						SetMotionMode(CRaceMotionData::MODE_HORSE_STAND_DUALHAND_SWORD);
					break;
				case CItemData::WEAPON_BOW:
						SetMotionMode(CRaceMotionData::MODE_HORSE_STAND_BOW);
					break;
				case CItemData::WEAPON_TWO_HANDED:
						SetMotionMode(CRaceMotionData::MODE_HORSE_STAND_TWOHAND_SWORD);
					break;
				case CItemData::WEAPON_BELL:
						SetMotionMode(CRaceMotionData::MODE_HORSE_STAND_BELL);
					break;
				case CItemData::WEAPON_FAN:
						SetMotionMode(CRaceMotionData::MODE_HORSE_STAND_FAN);
					break;
				case CItemData::WEAPON_CLAW:
						SetMotionMode(CRaceMotionData::MODE_HORSE_STAND_CLAW);
					break;
				default:
						SetMotionMode(CRaceMotionData::MODE_HORSE_STAND);
					break;
			}
		else
			switch (pItemData->GetValue(3))
			{
				case CItemData::WEAPON_SWORD:
					if (m_kHorse.IsMounting())
						SetMotionMode(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD);
					else
						SetMotionMode(CRaceMotionData::MODE_ONEHAND_SWORD);
					break;
				case CItemData::WEAPON_DAGGER:
					if (m_kHorse.IsMounting())
						SetMotionMode(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD);
					else
						SetMotionMode(CRaceMotionData::MODE_DUALHAND_SWORD);
					break;
				case CItemData::WEAPON_BOW:
					if (m_kHorse.IsMounting())
						SetMotionMode(CRaceMotionData::MODE_HORSE_BOW);
					else
#ifdef ENABLE_NEW_ARROW_SYSTEM
					{
						if (m_awPart[CRaceData::PART_ARROW_TYPE] == CItemData::WEAPON_UNLIMITED_ARROW)
							SetMotionMode(CRaceMotionData::MODE_BOW_SPECIAL);
						else
							SetMotionMode(CRaceMotionData::MODE_BOW);
					}
#else
					SetMotionMode(CRaceMotionData::MODE_BOW);
#endif
					break;
				case CItemData::WEAPON_TWO_HANDED:
					if (m_kHorse.IsMounting())
						SetMotionMode(CRaceMotionData::MODE_HORSE_TWOHAND_SWORD);
					else
						SetMotionMode(CRaceMotionData::MODE_TWOHAND_SWORD);
					break;
				case CItemData::WEAPON_BELL:
					if (m_kHorse.IsMounting())
						SetMotionMode(CRaceMotionData::MODE_HORSE_BELL);
					else
						SetMotionMode(CRaceMotionData::MODE_BELL);
					break;
				case CItemData::WEAPON_FAN:
					if (m_kHorse.IsMounting())
						SetMotionMode(CRaceMotionData::MODE_HORSE_BELL);
					else
						SetMotionMode(CRaceMotionData::MODE_FAN);
					break;
				case CItemData::WEAPON_CLAW:
					if (m_kHorse.IsMounting())
						SetMotionMode(CRaceMotionData::MODE_HORSE_CLAW);
					else
						SetMotionMode(CRaceMotionData::MODE_CLAW);
					break;
				default:
						if (m_kHorse.IsMounting())
							SetMotionMode(CRaceMotionData::MODE_HORSE);
						else
							SetMotionMode(CRaceMotionData::MODE_GENERAL);
					break;
			}
#else
		switch (pItemData->GetValue(3))
		{
			case CItemData::WEAPON_SWORD:
				if (m_kHorse.IsMounting())
					SetMotionMode(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD);
				else
					SetMotionMode(CRaceMotionData::MODE_ONEHAND_SWORD);
				break;
			case CItemData::WEAPON_DAGGER:
				if (m_kHorse.IsMounting())
					SetMotionMode(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD);
				else
					SetMotionMode(CRaceMotionData::MODE_DUALHAND_SWORD);
				break;
			case CItemData::WEAPON_BOW:
				if (m_kHorse.IsMounting())
					SetMotionMode(CRaceMotionData::MODE_HORSE_BOW);
				else
#ifdef ENABLE_NEW_ARROW_SYSTEM
				{
					if (m_awPart[CRaceData::PART_ARROW_TYPE] == CItemData::WEAPON_UNLIMITED_ARROW)
						SetMotionMode(CRaceMotionData::MODE_BOW_SPECIAL);
					else
						SetMotionMode(CRaceMotionData::MODE_BOW);
				}
#else
					SetMotionMode(CRaceMotionData::MODE_BOW);
#endif
				break;
			case CItemData::WEAPON_TWO_HANDED:
				if (m_kHorse.IsMounting())
					SetMotionMode(CRaceMotionData::MODE_HORSE_TWOHAND_SWORD);
				else
					SetMotionMode(CRaceMotionData::MODE_TWOHAND_SWORD);
				break;
			case CItemData::WEAPON_BELL:
				if (m_kHorse.IsMounting())
					SetMotionMode(CRaceMotionData::MODE_HORSE_BELL);
				else
					SetMotionMode(CRaceMotionData::MODE_BELL);
				break;
			case CItemData::WEAPON_FAN:
				if (m_kHorse.IsMounting())
					SetMotionMode(CRaceMotionData::MODE_HORSE_BELL);
				else
					SetMotionMode(CRaceMotionData::MODE_FAN);
				break;
			case CItemData::WEAPON_CLAW:
				if (m_kHorse.IsMounting())
					SetMotionMode(CRaceMotionData::MODE_HORSE_CLAW);
				else
					SetMotionMode(CRaceMotionData::MODE_CLAW);
				break;
			default:
					if (m_kHorse.IsMounting())
						SetMotionMode(CRaceMotionData::MODE_HORSE);
					else
						SetMotionMode(CRaceMotionData::MODE_GENERAL);
				break;
		}
#endif
	}
#endif
#ifdef ENABLE_STANDING_MOUNT
	else if (m_kHorse.IsMounting() && !m_kHorse.IsHoverBoard())
#else
	else if (m_kHorse.IsMounting())
#endif
	{
		switch (bySubType)
		{
			case CItemData::WEAPON_SWORD:
				SetMotionMode(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD);
				break;

			case CItemData::WEAPON_TWO_HANDED:
				SetMotionMode(CRaceMotionData::MODE_HORSE_TWOHAND_SWORD);
				break;

			case CItemData::WEAPON_DAGGER:
				SetMotionMode(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD);
				break;

			case CItemData::WEAPON_FAN:
				SetMotionMode(CRaceMotionData::MODE_HORSE_BELL);
				break;

			case CItemData::WEAPON_BELL:
				SetMotionMode(CRaceMotionData::MODE_HORSE_BELL);
				break;

			case CItemData::WEAPON_BOW:
				SetMotionMode(CRaceMotionData::MODE_HORSE_BOW);
				break;

			case CItemData::WEAPON_CLAW:
				SetMotionMode(CRaceMotionData::MODE_HORSE_CLAW);
				break;

			default:
				SetMotionMode(CRaceMotionData::MODE_HORSE);
				break;
		}
	}
#ifdef ENABLE_STANDING_MOUNT
	else if (m_kHorse.IsMounting() && m_kHorse.IsHoverBoard())
	{
		switch (bySubType)
		{
			case CItemData::WEAPON_SWORD:
				SetMotionMode(CRaceMotionData::MODE_HORSE_STAND_SWORD); // Warrior & Sura & Assassin
				break;

			case CItemData::WEAPON_TWO_HANDED:
				SetMotionMode(CRaceMotionData::MODE_HORSE_STAND_TWOHAND_SWORD); // Only Warrior
				break;

			case CItemData::WEAPON_DAGGER:
				SetMotionMode(CRaceMotionData::MODE_HORSE_STAND_DUALHAND_SWORD); // Only Assassin
				break;

			case CItemData::WEAPON_FAN:
				SetMotionMode(CRaceMotionData::MODE_HORSE_STAND_FAN); // Only Shaman
				break;

			case CItemData::WEAPON_BELL:
				SetMotionMode(CRaceMotionData::MODE_HORSE_STAND_BELL); // Only Shaman
				break;

			case CItemData::WEAPON_BOW:
				SetMotionMode(CRaceMotionData::MODE_HORSE_STAND_BOW); // Only Assassin
				break;
#ifdef ENABLE_WOLFMAN_CHARACTER
			case CItemData::WEAPON_CLAW:
				SetMotionMode(CRaceMotionData::MODE_HORSE_STAND_CLAW); // Only Wolfman
				break;
#endif
			default:
				SetMotionMode(CRaceMotionData::MODE_HORSE_STAND);
				break;
		}
	}
#endif
	else
	{
		switch (bySubType)
		{
			case CItemData::WEAPON_SWORD:
				SetMotionMode(CRaceMotionData::MODE_ONEHAND_SWORD);
				break;

			case CItemData::WEAPON_TWO_HANDED:
				SetMotionMode(CRaceMotionData::MODE_TWOHAND_SWORD);
				break;

			case CItemData::WEAPON_DAGGER:
				SetMotionMode(CRaceMotionData::MODE_DUALHAND_SWORD);
				break;

			case CItemData::WEAPON_BOW:
#ifdef ENABLE_NEW_ARROW_SYSTEM
				if (m_awPart[CRaceData::PART_ARROW_TYPE] == CItemData::WEAPON_UNLIMITED_ARROW)
					SetMotionMode(CRaceMotionData::MODE_BOW_SPECIAL);
				else
					SetMotionMode(CRaceMotionData::MODE_BOW);
#else
				SetMotionMode(CRaceMotionData::MODE_BOW);
#endif
				break;

			case CItemData::WEAPON_FAN:
				SetMotionMode(CRaceMotionData::MODE_FAN);
				break;

			case CItemData::WEAPON_BELL:
				SetMotionMode(CRaceMotionData::MODE_BELL);
				break;
#ifdef ENABLE_WOLFMAN_CHARACTER
			case CItemData::WEAPON_CLAW:
				SetMotionMode(CRaceMotionData::MODE_CLAW);
				break;
#endif
			case CItemData::WEAPON_ARROW:
#ifdef ENABLE_NEW_ARROW_SYSTEM
			case CItemData::WEAPON_UNLIMITED_ARROW:
#endif
			default:
				SetMotionMode(CRaceMotionData::MODE_GENERAL);
				break;
		}
	}

#ifdef ENABLE_GREEDY_ROOM
	if (GetRace() == 6939)
	{
		if (IsAffect(AFFECT_AAMON_DEFENCE))
		{
			dwMotIndex = 40;
			isLoop = true;
		}
		else if (IsAffect(AFFECT_AAMON_LASTSTAND))
		{
			dwMotIndex = CRaceMotionData::NAME_ETC_5;
			isLoop = true;
		}
	}
#endif

	if (isLoop)
		m_GraphicThingInstance.InterceptLoopMotion(dwMotIndex);
	else
		m_GraphicThingInstance.InterceptOnceMotion(dwMotIndex);

	RefreshActorInstance();
}

void CInstanceBase::RegisterBoundingSphere()
{
	if (!IsStone())
	{
		m_GraphicThingInstance.DeformNoSkin();
	}

	m_GraphicThingInstance.RegisterBoundingSphere();
}

bool CInstanceBase::CreateDeviceObjects()
{
	return m_GraphicThingInstance.CreateDeviceObjects();
}

void CInstanceBase::DestroyDeviceObjects()
{
	m_GraphicThingInstance.DestroyDeviceObjects();
}

void CInstanceBase::Destroy()
{
	DetachTextTail();

#ifdef ENABLE_TITLE_SYSTEM
	DetachTitleEffect();
#endif

	DismountHorse();

	m_kQue_kCmdNew.clear();

#ifdef ENABLE_BALATHOR_DUNGEON
	for (auto it = m_mapBalathorEffects.begin(); it != m_mapBalathorEffects.end(); ++it)
	{
		DWORD id = it->first;
		DWORD eff_id = it->second;

		const TBalathorEffect* balathor_effect = CPythonPlayer::Instance().GetBalathorEffect(id);
		if (balathor_effect)
		{
			if (balathor_effect->dwVID != 0)
				__DetachEffect(eff_id);
			//else
			//	CEffectManager::Instance().DestroyEffectInstance(eff_id);
		}
	}
	m_mapBalathorEffects.clear();
#endif

#ifdef ENABLE_OFFLINESHOP_SYSTEM
	if (m_myShop)
	{
		__DetachEffect(m_myShop);
		m_myShop = 0;
	}
#endif

#ifdef ENABLE_GREEDY_ROOM
	if (m_dwGreedyTornadoEffectID != 0)
	{
		CEffectManager::Instance().DestroyEffectInstance(m_dwGreedyTornadoEffectID);
		m_dwGreedyTornadoEffectID = 0;
	}
	if (m_dwGreedyTornadoEffectID2 != 0)
	{
		CEffectManager::Instance().DestroyEffectInstance(m_dwGreedyTornadoEffectID2);
		m_dwGreedyTornadoEffectID2 = 0;
	}
	if (m_dwFireWaveEffectID != 0)
	{
		CEffectManager::Instance().DestroyEffectInstance(m_dwFireWaveEffectID);
		m_dwFireWaveEffectID = 0;
	}
	if (m_dwSpearWarningEffectID != 0)
	{
		CEffectManager::Instance().DestroyEffectInstance(m_dwSpearWarningEffectID);
		m_dwSpearWarningEffectID = 0;
	}
	if (m_dwSpearFireEffectID != 0)
	{
		CEffectManager::Instance().DestroyEffectInstance(m_dwSpearFireEffectID);
		m_dwSpearFireEffectID = 0;
	}
#endif

	__EffectContainer_Destroy();
	__StoneSmoke_Destroy();

	if (__IsMainInstance())
		__ClearMainInstance();	
	
	m_GraphicThingInstance.Destroy();
	
	__Initialize();
}

void CInstanceBase::__InitializeRotationSpeed()
{
	SetRotationSpeed(c_fDefaultRotationSpeed);
}

void CInstanceBase::__Warrior_Initialize()
{
	m_kWarrior.m_dwGeomgyeongEffect=0;
}

#ifdef ENABLE_NEW_GYEONGGONG_SKILL
void CInstanceBase::__Assassin_Initialize()
{
	m_kAssassin.m_dwGyeongGongEffect = 0;
}
#endif

void CInstanceBase::__Initialize()
{
	__Warrior_Initialize();
	__StoneSmoke_Inialize();
	__EffectContainer_Initialize();
	__InitializeRotationSpeed();
#ifdef ENABLE_NEW_GYEONGGONG_SKILL
	__Assassin_Initialize();
#endif

	SetEventHandler(CActorInstance::IEventHandler::GetEmptyPtr());

#ifdef ENABLE_OFFLINESHOP_SYSTEM
	m_myShop=0;
#endif

	m_kAffectFlagContainer.Clear();

	m_dwLevel = 0;
#ifdef ENABLE_CONQUEROR_LEVEL
	m_dwConquerorLevel = 0;
#endif
#if defined(ENABLE_SHOW_MOB_INFO)
	m_dwAIFlag = 0;
#endif
	m_dwGuildID = 0;
	m_dwEmpireID = 0;

	m_eType = 0;
	m_eRaceType = 0;
	m_eShape = 0;
	m_dwRace = 0;
	m_dwVirtualNumber = 0;

	m_dwBaseCmdTime=0;
	m_dwBaseChkTime=0;
	m_dwSkipTime=0;

	m_GraphicThingInstance.Initialize();

	m_dwAdvActorVID=0;
	m_dwLastDmgActorVID=0;

	m_nAverageNetworkGap=0;
	m_dwNextUpdateHeightTime=0;

	m_iRotatingDirection = DEGREE_DIRECTION_SAME;

	m_isTextTail = FALSE;
	m_isGoing = FALSE;
	NEW_SetSrcPixelPosition(TPixelPosition(0, 0, 0));
	NEW_SetDstPixelPosition(TPixelPosition(0, 0, 0));

	m_kPPosDust = TPixelPosition(0, 0, 0);

	m_kQue_kCmdNew.clear();

	m_dwLastComboIndex = 0;

	m_swordRefineEffectRight = 0;
	m_swordRefineEffectLeft = 0;
	m_armorRefineEffect = 0;

#ifdef ENABLE_SASH_SYSTEM
	m_dwSashEffect = 0;
#endif

#ifdef ENABLE_TITLE_SYSTEM
	m_dwTitleEffect = 0;
#endif

#ifdef ENABLE_AURA_SYSTEM
	m_auraRefineEffect = 0;
#endif

#ifdef ENABLE_GREEDY_ROOM
	m_dwGreedyTornadoEffectID = 0;
	m_dwGreedyTornadoEffectID2 = 0;
	m_dwFireWaveEffectID = 0;
	m_dwSpearWarningEffectID = 0;
	m_dwSpearFireEffectID = 0;
#endif

#ifdef ENABLE_SUPPORT_SYSTEM
	is_support_shaman = false;
#endif

#ifdef ENABLE_BATTLE_FIELD
	combat_zone_rank = 0;
	combat_zone_points = 0;
#endif

#ifdef ENABLE_LEFT_SEAT
	m_bLeftSeat = false;
#endif

	m_sAlignment = 0;
	m_byPKMode = 0;
	m_isKiller = false;
	m_isPartyMember = false;

#ifdef ENABLE_AUTO_SYSTEM
	m_isAutoTarget = false;
#endif

	m_bEnableTCPState = TRUE;

	m_stName = "";

	memset(m_awPart, 0, sizeof(m_awPart));
	memset(m_adwCRCAffectEffect, 0, sizeof(m_adwCRCAffectEffect));
	memset(&m_kMovAfterFunc, 0, sizeof(m_kMovAfterFunc));

	m_bDamageEffectType = false;
	m_dwDuelMode = DUEL_NONE;
	m_dwEmoticonTime = 0;
	m_IsAlwaysRender = false;

}

CInstanceBase::CInstanceBase()
{
	__Initialize();
}

CInstanceBase::~CInstanceBase()
{
	Destroy();
}

bool CInstanceBase::IsAlwaysRender()
{
	return m_IsAlwaysRender;
}

void CInstanceBase::SetAlwaysRender(bool val)
{
	m_IsAlwaysRender = val;
}

void CInstanceBase::GetBoundBox(D3DXVECTOR3 * vtMin, D3DXVECTOR3 * vtMax)
{
	m_GraphicThingInstance.GetBoundBox(vtMin, vtMax);
}

#ifdef ENABLE_BALATHOR_DUNGEON
void CInstanceBase::RemoveBalathorEffect(const TBalathorEffect* pEffect)
{
	const auto it = m_mapBalathorEffects.find(pEffect->dwID);
	if (it == m_mapBalathorEffects.end())
		return;
	__DetachEffect(it->second);
	m_mapBalathorEffects.erase(it);
}

void CInstanceBase::AddBalathorEffect(const TBalathorEffect* pEffect)
{
	const auto it = m_mapBalathorEffects.find(pEffect->dwID);
	if (it != m_mapBalathorEffects.end())
		return;

	const char* szFileName = CPythonPlayer::Instance().GetBalathorEffectFileName(pEffect->bEffIndex);
	if (!szFileName)
		return;

	DWORD dwCRC = 0;
	if (!CEffectManager::Instance().RegisterEffect2(szFileName, &dwCRC, false))
		return;

	DWORD dwResult = m_GraphicThingInstance.AttachEffectByID(NULL, "Bip01", dwCRC);
	if (dwResult != 0)
		m_mapBalathorEffects.emplace_hint(it, pEffect->dwID, dwResult);
}
#endif




