#include "StdAfx.h"
#include "PythonBackground.h"
#include "PythonPlayerEventHandler.h"
#include "PythonApplication.h"
#include "PythonItem.h"
#include "../eterbase/Timer.h"
#include "AbstractPlayer.h"
using std::make_pair;
#ifdef ENABLE_AUTO_PICKUP_SYSTEM
#include "PythonSystem.h"
#endif

#ifndef ENABLE_INGAME_WIKI
enum
{
	MAIN_RACE_WARRIOR_M,
	MAIN_RACE_ASSASSIN_W,
	MAIN_RACE_SURA_M,
	MAIN_RACE_SHAMAN_W,
	MAIN_RACE_WARRIOR_W,
	MAIN_RACE_ASSASSIN_M,
	MAIN_RACE_SURA_W,
	MAIN_RACE_SHAMAN_M,
	MAIN_RACE_WOLFMAN_M,
	MAIN_RACE_MAX_NUM,
};
#endif

const long long POINT_MAGIC_NUMBER = 0xe73ac1da;

void CPythonPlayer::SPlayerStatus::SetPoint(UINT ePoint, long long lPoint)
{
	m_alPoint[ePoint]=lPoint ^ POINT_MAGIC_NUMBER;
}

long long CPythonPlayer::SPlayerStatus::GetPoint(UINT ePoint)
{
	return m_alPoint[ePoint] ^ POINT_MAGIC_NUMBER;
}

bool CPythonPlayer::AffectIndexToSkillIndex(DWORD dwAffectIndex, DWORD * pdwSkillIndex)
{
	if (m_kMap_dwAffectIndexToSkillIndex.end() == m_kMap_dwAffectIndexToSkillIndex.find(dwAffectIndex))
		return false;

	*pdwSkillIndex = m_kMap_dwAffectIndexToSkillIndex[dwAffectIndex];
	return true;
}

bool CPythonPlayer::AffectIndexToSkillSlotIndex(UINT uAffect, DWORD* pdwSkillSlotIndex)
{
	DWORD dwSkillIndex=m_kMap_dwAffectIndexToSkillIndex[uAffect];

	return GetSkillSlotIndex(dwSkillIndex, pdwSkillSlotIndex);
}

bool CPythonPlayer::__GetPickedActorPtr(CInstanceBase** ppkInstPicked)
{
	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	CInstanceBase* pkInstPicked=rkChrMgr.OLD_GetPickedInstancePtr();
	if (!pkInstPicked)
		return false;

	*ppkInstPicked=pkInstPicked;
	return true;
}

bool CPythonPlayer::__GetPickedActorID(DWORD* pdwActorID)
{
	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	return rkChrMgr.OLD_GetPickedInstanceVID(pdwActorID);
}

bool CPythonPlayer::__GetPickedItemID(DWORD* pdwItemID)
{
	CPythonItem& rkItemMgr=CPythonItem::Instance();
	return rkItemMgr.GetPickedItemID(pdwItemID);
}

bool CPythonPlayer::__GetPickedGroundPos(TPixelPosition* pkPPosPicked)
{
	CPythonBackground& rkBG=CPythonBackground::Instance();

	TPixelPosition kPPosPicked;
	if (rkBG.GetPickingPoint(pkPPosPicked))
	{
		pkPPosPicked->y=-pkPPosPicked->y;
		return true;
	}

	return false;
}

void CPythonPlayer::NEW_GetMainActorPosition(TPixelPosition* pkPPosActor)
{
	TPixelPosition kPPosMainActor;

	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	CInstanceBase * pInstance = rkPlayer.NEW_GetMainActorPtr();
	if (pInstance)
	{
		pInstance->NEW_GetPixelPosition(pkPPosActor);
	}
	else
	{
		CPythonApplication::Instance().GetCenterPosition(pkPPosActor);
	}
}



bool CPythonPlayer::RegisterEffect(DWORD dwEID, const char* c_szFileName, bool isCache)
{
	if (dwEID>=EFFECT_NUM)
		return false;

	CEffectManager& rkEftMgr=CEffectManager::Instance();
	rkEftMgr.RegisterEffect2(c_szFileName, &m_adwEffect[dwEID], isCache);
	return true;
}

void CPythonPlayer::NEW_ShowEffect(int dwEID, TPixelPosition kPPosDst)
{
	if (dwEID>=EFFECT_NUM)
		return;

	D3DXVECTOR3 kD3DVt3Pos(kPPosDst.x, -kPPosDst.y, kPPosDst.z);
	D3DXVECTOR3 kD3DVt3Dir(0.0f, 0.0f, 1.0f);

	CEffectManager& rkEftMgr=CEffectManager::Instance();
	rkEftMgr.CreateEffect(m_adwEffect[dwEID], kD3DVt3Pos, kD3DVt3Dir);
}

CInstanceBase* CPythonPlayer::NEW_FindActorPtr(DWORD dwVID)
{
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	return rkChrMgr.GetInstancePtr(dwVID);
}

CInstanceBase* CPythonPlayer::NEW_GetMainActorPtr()
{
	return NEW_FindActorPtr(m_dwMainCharacterIndex);
}

///////////////////////////////////////////////////////////////////////////////////////////

void CPythonPlayer::Update()
{
	NEW_RefreshMouseWalkingDirection();

#ifdef ENABLE_AUTO_QUQUE_ATTACK
	AutoFarmLoop();
#endif

#ifdef ENABLE_BALATHOR_DUNGEON
	UpdateBalathorEffect();
#endif

	CPythonPlayerEventHandler& rkPlayerEventHandler=CPythonPlayerEventHandler::GetSingleton();
	rkPlayerEventHandler.FlushVictimList();

	if (m_isDestPosition)
	{
		CInstanceBase * pInstance = NEW_GetMainActorPtr();
		if (pInstance)
		{
			TPixelPosition PixelPosition;
			pInstance->NEW_GetPixelPosition(&PixelPosition);

			if (abs(int(PixelPosition.x) - m_ixDestPos) + abs(int(PixelPosition.y) - m_iyDestPos) < 10000)
			{
				m_isDestPosition = FALSE;
			}
			else
			{
				if (CTimer::Instance().GetCurrentMillisecond() - m_iLastAlarmTime > 20000)
				{
					AlarmHaveToGo();
				}
			}
		}
	}

	if (m_isConsumingStamina)
	{
		float fElapsedTime = CTimer::Instance().GetElapsedSecond();
		m_fCurrentStamina -= (fElapsedTime * m_fConsumeStaminaPerSec);

		SetStatus(POINT_STAMINA, DWORD(m_fCurrentStamina));

		PyCallClassMemberFunc(m_ppyGameWindow, "RefreshStamina", Py_BuildValue("()"));
	}

#ifdef ENABLE_AUTO_PICKUP_SYSTEM
	static DWORD s_dwNextTCPAutoPickTime = 0;
	DWORD dwCurAutoTime = ELTimer_GetMSec();
	if (CPythonSystem::instance().IsAutoPickup() == true)
	{
		if (dwCurAutoTime >= s_dwNextTCPAutoPickTime)
		{
			s_dwNextTCPAutoPickTime = dwCurAutoTime + 500;
			PickCloseItem();
		}
	}
#endif

	__Update_AutoAttack();
	__Update_NotifyGuildAreaEvent();

#ifdef ENABLE_AUTO_SYSTEM
	if (m_dwAutoHuntRangeEffectID != 0)
	{
		CInstanceBase* pMainInstance = NEW_GetMainActorPtr();
		if (pMainInstance)
		{
			TPixelPosition kPPos;
			pMainInstance->NEW_GetPixelPosition(&kPPos);

			D3DXMATRIX mat;
			D3DXMatrixTranslation(&mat, kPPos.x, -kPPos.y, kPPos.z);

			CEffectManager& rkEftMgr = CEffectManager::Instance();
			if (rkEftMgr.SelectEffectInstance(m_dwAutoHuntRangeEffectID))
			{
				rkEftMgr.SetEffectInstanceGlobalMatrix(mat);
			}
		}
	}

	if (AutoStatus())
		UpdateAuto();
#endif
}

bool CPythonPlayer::__IsUsingChargeSkill()
{
	CInstanceBase * pkInstMain = NEW_GetMainActorPtr();
	if (!pkInstMain)
		return false;

	if (__CheckDashAffect(*pkInstMain))
		return true;

	if (MODE_USE_SKILL != m_eReservedMode)
		return false;

	if (m_dwSkillSlotIndexReserved >= SKILL_MAX_NUM)
		return false;

	TSkillInstance & rkSkillInst = m_playerStatus.aSkill[m_dwSkillSlotIndexReserved];

	CPythonSkill::TSkillData * pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(rkSkillInst.dwIndex, &pSkillData))
		return false;

	return pSkillData->IsChargeSkill() ? true : false;
}

void CPythonPlayer::__Update_AutoAttack()
{
	if (0 == m_dwAutoAttackTargetVID)
		return;

	if (ELTimer_GetMSec() < AutoHuntDodgeTime)
		return;

	CInstanceBase * pkInstMain = NEW_GetMainActorPtr();
	if (!pkInstMain)
		return;

	// 탄환격 쓰고 달려가는 �중�는 스킵
	if (__IsUsingChargeSkill())
		return;

	CInstanceBase* pkInstVictim = NEW_FindActorPtr(m_dwAutoAttackTargetVID);
	if (!pkInstVictim)
		__ClearAutoAttackTargetActorID();
	else
	{
		if (pkInstVictim->IsDead())
			__ClearAutoAttackTargetActorID();
#ifdef ENABLE_AUTO_SYSTEM
		else if (AutoStatus() && !GetAutoAttackOnOff())
		{
			__ClearAutoAttackTargetActorID();
			return;
		}
#endif
		else if (pkInstMain->IsMountingHorse() && !pkInstMain->CanAttackHorseLevel())
			__ClearAutoAttackTargetActorID();
		else if (pkInstMain->IsAttackableInstance(*pkInstVictim))
		{
			if (pkInstMain->IsSleep())
			{
				//TraceError("SKIP_AUTO_ATTACK_IN_SLEEPING");
			}
			else
				__ReserveClickActor(m_dwAutoAttackTargetVID);
		}
	}
}

void CPythonPlayer::__Update_NotifyGuildAreaEvent()
{
	CInstanceBase * pkInstMain = NEW_GetMainActorPtr();
	if (pkInstMain)
	{
		TPixelPosition kPixelPosition;
		pkInstMain->NEW_GetPixelPosition(&kPixelPosition);

		DWORD dwAreaID = CPythonMiniMap::Instance().GetGuildAreaID(
			ULONG(kPixelPosition.x), ULONG(kPixelPosition.y));

		if (dwAreaID != m_inGuildAreaID)
		{
			if (0xffffffff != dwAreaID)
			{
				PyCallClassMemberFunc(m_ppyGameWindow, "BINARY_Guild_EnterGuildArea", Py_BuildValue("(i)", dwAreaID));
			}
			else
			{
				PyCallClassMemberFunc(m_ppyGameWindow, "BINARY_Guild_ExitGuildArea", Py_BuildValue("(i)", dwAreaID));
			}

			m_inGuildAreaID = dwAreaID;
		}
	}
}

void CPythonPlayer::SetMainCharacterIndex(int iIndex)
{
	m_dwMainCharacterIndex = iIndex;

	CInstanceBase* pkInstMain=NEW_GetMainActorPtr();
	if (pkInstMain)
	{
		CPythonPlayerEventHandler& rkPlayerEventHandler=CPythonPlayerEventHandler::GetSingleton();
		pkInstMain->SetEventHandler(&rkPlayerEventHandler);
	}
}

DWORD CPythonPlayer::GetMainCharacterIndex()
{
	return m_dwMainCharacterIndex;
}

bool CPythonPlayer::IsMainCharacterIndex(DWORD dwIndex)
{
	return (m_dwMainCharacterIndex == dwIndex);
}

DWORD CPythonPlayer::GetGuildID()
{
	CInstanceBase* pkInstMain=NEW_GetMainActorPtr();
	if (!pkInstMain)
		return 0xffffffff;

	return pkInstMain->GetGuildID();
}

void CPythonPlayer::SetWeaponPower(DWORD dwMinPower, DWORD dwMaxPower, DWORD dwMinMagicPower, DWORD dwMaxMagicPower, DWORD dwAddPower)
{
	m_dwWeaponMinPower=dwMinPower;
	m_dwWeaponMaxPower=dwMaxPower;
	m_dwWeaponMinMagicPower=dwMinMagicPower;
	m_dwWeaponMaxMagicPower=dwMaxMagicPower;
	m_dwWeaponAddPower=dwAddPower;

	__UpdateBattleStatus();	
}

void CPythonPlayer::SetRace(DWORD dwRace)
{
	m_dwRace=dwRace;
}

DWORD CPythonPlayer::GetRace()
{
	return m_dwRace;
}

DWORD CPythonPlayer::__GetRaceStat()
{
	switch (GetRace())
	{
		case MAIN_RACE_WARRIOR_M:
		case MAIN_RACE_WARRIOR_W:
			return GetStatus(POINT_ST);
			break;

		case MAIN_RACE_ASSASSIN_M:
		case MAIN_RACE_ASSASSIN_W:
			return GetStatus(POINT_DX);
			break;

		case MAIN_RACE_SURA_M:
		case MAIN_RACE_SURA_W:
			return GetStatus(POINT_ST);
			break;

		case MAIN_RACE_SHAMAN_M:
		case MAIN_RACE_SHAMAN_W:
			return GetStatus(POINT_IQ);
			break;

		case MAIN_RACE_WOLFMAN_M:
			return GetStatus(POINT_ST);
			break;
	}

	return GetStatus(POINT_ST);
}

DWORD CPythonPlayer::__GetLevelAtk()
{
	return 2*GetStatus(POINT_LEVEL);
}

DWORD CPythonPlayer::__GetStatAtk()
{
	return (4*GetStatus(POINT_ST)+2*__GetRaceStat())/3;
}

DWORD CPythonPlayer::__GetWeaponAtk(DWORD dwWeaponPower)
{
	return 2*dwWeaponPower;
}

DWORD CPythonPlayer::__GetTotalAtk(DWORD dwWeaponPower, DWORD dwRefineBonus)
{
	DWORD dwLvAtk=__GetLevelAtk();
	DWORD dwStAtk=__GetStatAtk();

	/////

	DWORD dwWepAtk;
	DWORD dwTotalAtk;	

	if (LocaleService_IsCHEONMA())
	{
		dwWepAtk = __GetWeaponAtk(dwWeaponPower+dwRefineBonus);
		dwTotalAtk = dwLvAtk+(dwStAtk+dwWepAtk)*(GetStatus(POINT_DX)+210)/300;		
	}
	else
	{
		int hr = __GetHitRate();
		dwWepAtk = __GetWeaponAtk(dwWeaponPower+dwRefineBonus);
		dwTotalAtk = dwLvAtk+(dwStAtk+dwWepAtk)*hr/100;	
	}

	return dwTotalAtk;
}

DWORD CPythonPlayer::__GetHitRate()
{
	int src = 0;

	if (LocaleService_IsCHEONMA())
	{
		src = GetStatus(POINT_DX);
	}
	else
	{
		src = (GetStatus(POINT_DX) * 4 + GetStatus(POINT_LEVEL) * 2)/6;
	}

	return 100*(min(90, src)+210)/300;
}

DWORD CPythonPlayer::__GetEvadeRate()
{
	return 30*(2*GetStatus(POINT_DX)+5)/(GetStatus(POINT_DX)+95);
} 

void CPythonPlayer::__UpdateBattleStatus()
{
	m_playerStatus.SetPoint(POINT_NONE, 0);
	m_playerStatus.SetPoint(POINT_EVADE_RATE, __GetEvadeRate());
	m_playerStatus.SetPoint(POINT_HIT_RATE, __GetHitRate());
	m_playerStatus.SetPoint(POINT_MIN_WEP, m_dwWeaponMinPower+m_dwWeaponAddPower);
	m_playerStatus.SetPoint(POINT_MAX_WEP, m_dwWeaponMaxPower+m_dwWeaponAddPower);
	m_playerStatus.SetPoint(POINT_MIN_MAGIC_WEP, m_dwWeaponMinMagicPower+m_dwWeaponAddPower);
	m_playerStatus.SetPoint(POINT_MAX_MAGIC_WEP, m_dwWeaponMaxMagicPower+m_dwWeaponAddPower);
	m_playerStatus.SetPoint(POINT_MIN_ATK, __GetTotalAtk(m_dwWeaponMinPower, m_dwWeaponAddPower));
	m_playerStatus.SetPoint(POINT_MAX_ATK, __GetTotalAtk(m_dwWeaponMaxPower, m_dwWeaponAddPower));	
}

void CPythonPlayer::SetStatus(DWORD dwType, long long lValue)
{
	if (dwType >= POINT_MAX_NUM)
	{
		assert(!" CPythonPlayer::SetStatus - Strange Status Type!");
		Tracef("CPythonPlayer::SetStatus - Set Status Type Error\n");
		return;
	}

	if (dwType == POINT_LEVEL)
	{
		CInstanceBase* pkPlayer = NEW_GetMainActorPtr();

		if (pkPlayer)
		{
#ifdef ENABLE_TEXT_LEVEL_REFRESH
			pkPlayer->SetLevel(lValue);
#endif
			pkPlayer->UpdateTextTailLevel(lValue);
		}
	}
#ifdef ENABLE_CONQUEROR_LEVEL
	else if (dwType == POINT_CONQUEROR_LEVEL)
	{
		CInstanceBase* pkPlayer = NEW_GetMainActorPtr();

		if (pkPlayer)
		{
#ifdef ENABLE_TEXT_LEVEL_REFRESH
			pkPlayer->SetLevel(lValue);
#endif
			pkPlayer->UpdateTextTailConquerorLevel(lValue);
		}
	}
#endif
	switch (dwType)
	{
		case POINT_MIN_WEP:
		case POINT_MAX_WEP:
		case POINT_MIN_ATK:
		case POINT_MAX_ATK:
		case POINT_HIT_RATE:
		case POINT_EVADE_RATE:
		case POINT_LEVEL:
		case POINT_ST:
		case POINT_DX:
		case POINT_IQ:
			m_playerStatus.SetPoint(dwType, lValue);
			__UpdateBattleStatus();
			break;
		default:
			m_playerStatus.SetPoint(dwType, lValue);
			break;
	}		
}

long long CPythonPlayer::GetStatus(DWORD dwType)
{
	if (dwType >= POINT_MAX_NUM)
	{
		assert(!" CPythonPlayer::GetStatus - Strange Status Type!");
		Tracef("CPythonPlayer::GetStatus - Get Status Type Error\n");
		return 0;
	}

	return m_playerStatus.GetPoint(dwType);
}

const char* CPythonPlayer::GetName()
{
	return m_stName.c_str();
}

void CPythonPlayer::SetName(const char *name)
{
	m_stName = name;
}

void CPythonPlayer::NotifyDeletingCharacterInstance(DWORD dwVID)
{
	if (m_dwMainCharacterIndex == dwVID)
		m_dwMainCharacterIndex = 0;
}

void CPythonPlayer::NotifyCharacterDead(DWORD dwVID)
{
	if (__IsSameTargetVID(dwVID))
	{
		SetTarget(0);
	}
}

void CPythonPlayer::NotifyCharacterUpdate(DWORD dwVID)
{
	if (__IsSameTargetVID(dwVID))
	{
		CInstanceBase * pMainInstance = NEW_GetMainActorPtr();
		CInstanceBase * pTargetInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwVID);
		if (pMainInstance && pTargetInstance)
		{
			if (!pMainInstance->IsTargetableInstance(*pTargetInstance))
			{
				SetTarget(0);
				PyCallClassMemberFunc(m_ppyGameWindow, "CloseTargetBoard", Py_BuildValue("()"));
			}
			else
			{
				PyCallClassMemberFunc(m_ppyGameWindow, "RefreshTargetBoardByVID", Py_BuildValue("(i)", dwVID));
			}
		}
	}
}

void CPythonPlayer::NotifyDeadMainCharacter()
{
	__ClearAutoAttackTargetActorID();
}

void CPythonPlayer::NotifyChangePKMode()
{
	PyCallClassMemberFunc(m_ppyGameWindow, "OnChangePKMode", Py_BuildValue("()"));
}


void CPythonPlayer::MoveItemData(TItemPos SrcCell, TItemPos DstCell)
{
	if (!SrcCell.IsValidCell() || !DstCell.IsValidCell())
		return;

	TItemData src_item(*GetItemData(SrcCell));
	TItemData dst_item(*GetItemData(DstCell));
	SetItemData(DstCell, src_item);
	SetItemData(SrcCell, dst_item);
}

const TItemData * CPythonPlayer::GetItemData(TItemPos Cell) const
{
	if (!Cell.IsValidCell())
		return NULL;

	switch (Cell.window_type)
	{
		case INVENTORY:
		case EQUIPMENT:
			return &m_playerStatus.aItem[Cell.cell];

		case DRAGON_SOUL_INVENTORY:
			return &m_playerStatus.aDSItem[Cell.cell];

#ifdef ENABLE_SWITCHBOT
		case SWITCHBOT:
			return &m_playerStatus.aSwitchbotItem[Cell.cell];
#endif

		default:
			return NULL;
	}
}

void CPythonPlayer::SetItemData(TItemPos Cell, const TItemData & c_rkItemInst)
{
	if (!Cell.IsValidCell())
		return;

	if (c_rkItemInst.vnum != 0)
	{
		CItemData * pItemData;
		if (!CItemManager::Instance().GetItemDataPointer(c_rkItemInst.vnum, &pItemData))
		{
			TraceError("CPythonPlayer::SetItemData(window_type : %d, dwSlotIndex=%d, itemIndex=%d) - Failed to item data\n", Cell.window_type, Cell.cell, c_rkItemInst.vnum);
			return;
		}
	}

	switch (Cell.window_type)
	{
		case INVENTORY:
		case EQUIPMENT:
			m_playerStatus.aItem[Cell.cell] = c_rkItemInst;
			break;

		case DRAGON_SOUL_INVENTORY:
			m_playerStatus.aDSItem[Cell.cell] = c_rkItemInst;
			break;

#ifdef ENABLE_SWITCHBOT
		case SWITCHBOT:
			m_playerStatus.aSwitchbotItem[Cell.cell] = c_rkItemInst;
			break;
#endif
	}
}

DWORD CPythonPlayer::GetItemIndex(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;

	return GetItemData(Cell)->vnum;
}

DWORD CPythonPlayer::GetItemFlags(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;
	const TItemData * pItem = GetItemData(Cell);
	assert (pItem != NULL);
	return pItem->flags;
}

DWORD CPythonPlayer::GetItemAntiFlags(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;

	const TItemData * pItem = GetItemData(Cell);
	assert(pItem != NULL);
	return pItem->anti_flags;
}

BYTE CPythonPlayer::GetItemTypeBySlot(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;

	CItemData * pItemDataPtr = NULL;
	if (CItemManager::Instance().GetItemDataPointer(GetItemIndex(Cell), &pItemDataPtr))
		return pItemDataPtr->GetType();
	else
	{
		TraceError("FAILED\t\tCPythonPlayer::GetItemTypeBySlot()\t\tCell(%d, %d) item is null", Cell.window_type, Cell.cell);
		return 0;
	}
}

BYTE CPythonPlayer::GetItemSubTypeBySlot(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;

	CItemData * pItemDataPtr = NULL;
	if (CItemManager::Instance().GetItemDataPointer(GetItemIndex(Cell), &pItemDataPtr))
		return pItemDataPtr->GetSubType();
	else
	{
		TraceError("FAILED\t\tCPythonPlayer::GetItemSubTypeBySlot()\t\tCell(%d, %d) item is null", Cell.window_type, Cell.cell);
		return 0;
	}
}

DWORD CPythonPlayer::GetItemCount(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;
	const TItemData * pItem = GetItemData(Cell);
	if (pItem == NULL)
		return 0;
	else
		return pItem->count;
}

#if defined(ENABLE_CUBE_RENEWAL) && defined(ENABLE_SET_ITEM)
DWORD CPythonPlayer::GetItemCountByVnum(DWORD dwVnum, bool bIgnoreSetValue)
#else
DWORD CPythonPlayer::GetItemCountByVnum(DWORD dwVnum)
#endif
{
	DWORD dwCount = 0;

	for (int i = 0; i < c_Inventory_Count; ++i)
	{

		const TItemData & c_rItemData = m_playerStatus.aItem[i];

#if defined(ENABLE_CUBE_RENEWAL) && defined(ENABLE_SET_ITEM)
		if (bIgnoreSetValue && c_rItemData.set_value)
			continue;
#endif

		if (c_rItemData.vnum == dwVnum)
		{
			dwCount += c_rItemData.count;
		}
	}

	return dwCount;
}

DWORD CPythonPlayer::GetItemMetinSocket(TItemPos Cell, DWORD dwMetinSocketIndex)
{
	if (!Cell.IsValidCell())
		return 0;

	if (dwMetinSocketIndex >= ITEM_SOCKET_SLOT_MAX_NUM)
		return 0;

	return GetItemData(Cell)->alSockets[dwMetinSocketIndex];
}

#ifdef ENABLE_GLOVE_SYSTEM
void CPythonPlayer::GetItemApplyRandom(TItemPos Cell, DWORD dwApplySlotIndex, BYTE* pbyType, short* psValue)
{
	*pbyType = 0;
	*psValue = 0;

	if (!Cell.IsValidCell())
		return;

	if (dwApplySlotIndex >= ITEM_APPLY_RANDOM_SLOT_MAX_NUM)
		return;

	*pbyType = GetItemData(Cell)->aApplyRandom[dwApplySlotIndex].bType;
	*psValue = GetItemData(Cell)->aApplyRandom[dwApplySlotIndex].sValue;
}
#endif

void CPythonPlayer::GetItemAttribute(TItemPos Cell, DWORD dwAttrSlotIndex, BYTE * pbyType, short * psValue)
{
	*pbyType = 0;
	*psValue = 0;

	if (!Cell.IsValidCell())
		return;

	if (dwAttrSlotIndex >= ITEM_ATTRIBUTE_SLOT_MAX_NUM)
		return;

	*pbyType = GetItemData(Cell)->aAttr[dwAttrSlotIndex].bType;
	*psValue = GetItemData(Cell)->aAttr[dwAttrSlotIndex].sValue;
}

#ifdef ENABLE_EXTENDED_ITEM_COUNT
void CPythonPlayer::SetItemCount(TItemPos Cell, WORD byCount, bool uPacketMi)
#else
void CPythonPlayer::SetItemCount(TItemPos Cell, BYTE byCount)
#endif
{
	if (!Cell.IsValidCell())
		return;

	(const_cast <TItemData *>(GetItemData(Cell)))->count = byCount;
	if (!uPacketMi){ PyCallClassMemberFunc(m_ppyGameWindow, "RefreshInventory", Py_BuildValue("()")); }
}

#ifdef ENABLE_CHANGELOOK_SYSTEM
void CPythonPlayer::SetItemTransmutation(TItemPos Cell, DWORD dwVnum)
{
	if (!Cell.IsValidCell())
		return;
	
	(const_cast <TItemData *>(GetItemData(Cell)))->transmutation = dwVnum;
	PyCallClassMemberFunc(m_ppyGameWindow, "RefreshInventory", Py_BuildValue("()"));
}

DWORD CPythonPlayer::GetItemTransmutation(TItemPos Cell)
{
	if (Cell.IsValidCell())
	{
		const TItemData * pkItem = GetItemData(Cell);
		if (pkItem)
			return pkItem->transmutation;
	}
	
	return 0;
}
#endif

#ifdef ENABLE_SOULBIND_SYSTEM
void CPythonPlayer::SetItemBind(TItemPos Cell, long lBindTime)
{
	if (!Cell.IsValidCell())
		return;
	
	(const_cast <TItemData *>(GetItemData(Cell)))->bind = lBindTime;
	PyCallClassMemberFunc(m_ppyGameWindow, "RefreshInventory", Py_BuildValue("()"));
}

long CPythonPlayer::GetItemBind(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;
	
	const TItemData * pItem = GetItemData(Cell);
	if (pItem == NULL)
		return 0;
	else
		return pItem->bind;
}
#endif

void CPythonPlayer::SetItemMetinSocket(TItemPos Cell, DWORD dwMetinSocketIndex, DWORD dwMetinNumber)
{
	if (!Cell.IsValidCell())
		return;
	if (dwMetinSocketIndex >= ITEM_SOCKET_SLOT_MAX_NUM)
		return;

	(const_cast <TItemData *>(GetItemData(Cell)))->alSockets[dwMetinSocketIndex] = dwMetinNumber;
}

#ifdef ENABLE_GLOVE_SYSTEM
void CPythonPlayer::SetItemApplyRandom(TItemPos Cell, DWORD dwApplyIndex, BYTE byType, short sValue)
{
	if (!Cell.IsValidCell())
		return;

	if (dwApplyIndex >= ITEM_APPLY_RANDOM_SLOT_MAX_NUM)
		return;

	(const_cast <TItemData*>(GetItemData(Cell)))->aApplyRandom[dwApplyIndex].bType = byType;
	(const_cast <TItemData*>(GetItemData(Cell)))->aApplyRandom[dwApplyIndex].sValue = sValue;
}
#endif

void CPythonPlayer::SetItemAttribute(TItemPos Cell, DWORD dwAttrIndex, BYTE byType, short sValue)
{
	if (!Cell.IsValidCell())
		return;
	if (dwAttrIndex >= ITEM_ATTRIBUTE_SLOT_MAX_NUM)
		return;

	(const_cast <TItemData *>(GetItemData(Cell)))->aAttr[dwAttrIndex].bType = byType;
	(const_cast <TItemData *>(GetItemData(Cell)))->aAttr[dwAttrIndex].sValue = sValue;
}

#ifdef ENABLE_REFINE_ELEMENT
void CPythonPlayer::SetItemRefineElement(TItemPos Cell, DWORD dwRefineElement)
{
	if (!Cell.IsValidCell())
		return;
	
	(const_cast <TItemData *>(GetItemData(Cell)))->dwRefineElement = dwRefineElement;
	PyCallClassMemberFunc(m_ppyGameWindow, "RefreshInventory", Py_BuildValue("()"));
}

DWORD CPythonPlayer::GetItemRefineElement(TItemPos Cell)
{
	if (Cell.IsValidCell())
	{
		const TItemData * pkItem = GetItemData(Cell);
		if (pkItem)
			return pkItem->dwRefineElement;
	}
	
	return 0;
}
#endif

int CPythonPlayer::GetQuickPage()
{
	return m_playerStatus.lQuickPageIndex;
}

void CPythonPlayer::SetQuickPage(int nQuickPageIndex)
{
	if (nQuickPageIndex<0)
		m_playerStatus.lQuickPageIndex=QUICKSLOT_MAX_LINE+nQuickPageIndex;	
	else if (nQuickPageIndex>=QUICKSLOT_MAX_LINE)
		m_playerStatus.lQuickPageIndex=nQuickPageIndex%QUICKSLOT_MAX_LINE;	
	else
		m_playerStatus.lQuickPageIndex=nQuickPageIndex;	

	PyCallClassMemberFunc(m_ppyGameWindow, "RefreshInventory", Py_BuildValue("()"));
}

DWORD	CPythonPlayer::LocalQuickSlotIndexToGlobalQuickSlotIndex(DWORD dwLocalSlotIndex)
{
	return m_playerStatus.lQuickPageIndex*QUICKSLOT_MAX_COUNT_PER_LINE+dwLocalSlotIndex;	
}

void	CPythonPlayer::GetGlobalQuickSlotData(DWORD dwGlobalSlotIndex, DWORD* pdwWndType, DWORD* pdwWndItemPos)
{
	TQuickSlot& rkQuickSlot=__RefGlobalQuickSlot(dwGlobalSlotIndex);
	*pdwWndType=rkQuickSlot.Type;
	*pdwWndItemPos=rkQuickSlot.Position;
}

void	CPythonPlayer::GetLocalQuickSlotData(DWORD dwSlotPos, DWORD* pdwWndType, DWORD* pdwWndItemPos)
{
	TQuickSlot& rkQuickSlot=__RefLocalQuickSlot(dwSlotPos);
	*pdwWndType=rkQuickSlot.Type;
	*pdwWndItemPos=rkQuickSlot.Position;
}

TQuickSlot & CPythonPlayer::__RefLocalQuickSlot(int SlotIndex)
{
	return __RefGlobalQuickSlot(LocalQuickSlotIndexToGlobalQuickSlotIndex(SlotIndex));
}

TQuickSlot & CPythonPlayer::__RefGlobalQuickSlot(int SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= QUICKSLOT_MAX_NUM)
	{
		static TQuickSlot s_kQuickSlot;
		s_kQuickSlot.Type = 0;
		s_kQuickSlot.Position = 0;
		return s_kQuickSlot;
	}

	return m_playerStatus.aQuickSlot[SlotIndex];
}

void CPythonPlayer::RemoveQuickSlotByValue(int iType, int iPosition)
{
	for (UINT i = 0; i < QUICKSLOT_MAX_NUM; ++i)
	{
		if (iType == m_playerStatus.aQuickSlot[i].Type)
			if (iPosition == m_playerStatus.aQuickSlot[i].Position)
				CPythonNetworkStream::Instance().SendQuickSlotDelPacket(i);
	}
}

char CPythonPlayer::IsItem(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;

	return 0 != GetItemData(Cell)->vnum;
}

void CPythonPlayer::RequestMoveGlobalQuickSlotToLocalQuickSlot(DWORD dwGlobalSrcSlotIndex, DWORD dwLocalDstSlotIndex)
{
	//DWORD dwGlobalSrcSlotIndex=LocalQuickSlotIndexToGlobalQuickSlotIndex(dwLocalSrcSlotIndex);
	DWORD dwGlobalDstSlotIndex=LocalQuickSlotIndexToGlobalQuickSlotIndex(dwLocalDstSlotIndex);

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendQuickSlotMovePacket((UINT) dwGlobalSrcSlotIndex, (UINT)dwGlobalDstSlotIndex);
}

void CPythonPlayer::RequestAddLocalQuickSlot(DWORD dwLocalSlotIndex, DWORD dwWndType, DWORD dwWndItemPos)
{
	if (dwLocalSlotIndex>=QUICKSLOT_MAX_COUNT_PER_LINE)
		return;

	DWORD dwGlobalSlotIndex=LocalQuickSlotIndexToGlobalQuickSlotIndex(dwLocalSlotIndex);

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendQuickSlotAddPacket((UINT)dwGlobalSlotIndex, (BYTE)dwWndType, (UINT)dwWndItemPos);
}

void CPythonPlayer::RequestAddToEmptyLocalQuickSlot(DWORD dwWndType, DWORD dwWndItemPos)
{
    for (int i = 0; i < QUICKSLOT_MAX_COUNT_PER_LINE; ++i)
    {
        TQuickSlot& rkQuickSlot=__RefLocalQuickSlot(i);

        if (0 == rkQuickSlot.Type)
        {
            DWORD dwGlobalQuickSlotIndex=LocalQuickSlotIndexToGlobalQuickSlotIndex(i);
            CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
            rkNetStream.SendQuickSlotAddPacket((BYTE)dwGlobalQuickSlotIndex, (BYTE)dwWndType, (BYTE)dwWndItemPos);
            return;
        }
    }

}

void CPythonPlayer::RequestDeleteGlobalQuickSlot(DWORD dwGlobalSlotIndex)
{
	if (dwGlobalSlotIndex>=QUICKSLOT_MAX_COUNT)
		return;

	//if (dwLocalSlotIndex>=QUICKSLOT_MAX_SLOT_PER_LINE)
	//	return;

	//DWORD dwGlobalSlotIndex=LocalQuickSlotIndexToGlobalQuickSlotIndex(dwLocalSlotIndex);

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendQuickSlotDelPacket((UINT)dwGlobalSlotIndex);
}

void CPythonPlayer::RequestUseLocalQuickSlot(DWORD dwLocalSlotIndex)
{
	if (dwLocalSlotIndex>=QUICKSLOT_MAX_COUNT_PER_LINE)
		return;

	DWORD dwRegisteredType;
	DWORD dwRegisteredItemPos;
	GetLocalQuickSlotData(dwLocalSlotIndex, &dwRegisteredType, &dwRegisteredItemPos);

	switch (dwRegisteredType)
	{
		case SLOT_TYPE_INVENTORY:
		{
			CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
			rkNetStream.SendItemUsePacket(TItemPos(INVENTORY, (WORD)dwRegisteredItemPos));
			break;
		}
		case SLOT_TYPE_SKILL:
		{
			ClickSkillSlot(dwRegisteredItemPos);
			break;
		}
		case SLOT_TYPE_EMOTION:
		{
			PyCallClassMemberFunc(m_ppyGameWindow, "BINARY_ActEmotion", Py_BuildValue("(i)", dwRegisteredItemPos));
			break;
		}
	}
}

void CPythonPlayer::AddQuickSlot(int QuickSlotIndex, char IconType, INT IconPosition)
{
	if (QuickSlotIndex < 0 || QuickSlotIndex >= QUICKSLOT_MAX_NUM)
		return;

	m_playerStatus.aQuickSlot[QuickSlotIndex].Type = IconType;
	m_playerStatus.aQuickSlot[QuickSlotIndex].Position = IconPosition;
}

void CPythonPlayer::DeleteQuickSlot(int QuickSlotIndex)
{
	if (QuickSlotIndex < 0 || QuickSlotIndex >= QUICKSLOT_MAX_NUM)
		return;

	m_playerStatus.aQuickSlot[QuickSlotIndex].Type = 0;
	m_playerStatus.aQuickSlot[QuickSlotIndex].Position = 0;
}

void CPythonPlayer::MoveQuickSlot(int Source, int Target)
{
	if (Source < 0 || Source >= QUICKSLOT_MAX_NUM)
		return;

	if (Target < 0 || Target >= QUICKSLOT_MAX_NUM)
		return;

	TQuickSlot& rkSrcSlot=__RefGlobalQuickSlot(Source);
	TQuickSlot& rkDstSlot=__RefGlobalQuickSlot(Target);

	std::swap(rkSrcSlot, rkDstSlot);
}

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
bool CPythonPlayer::IsBeltInventorySlot(TItemPos Cell)
{
	return Cell.IsBeltInventoryCell();
}
#endif

#ifdef ENABLE_SPLIT_INVENTORY_SYSTEM
bool CPythonPlayer::IsSkillBookInventorySlot(TItemPos Cell)
{
	return Cell.IsSkillBookInventoryCell();
}

bool CPythonPlayer::IsUpgradeItemsInventorySlot(TItemPos Cell)
{
	return Cell.IsUpgradeItemsInventoryCell();
}

bool CPythonPlayer::IsStoneInventorySlot(TItemPos Cell)
{
	return Cell.IsStoneInventoryCell();
}

bool CPythonPlayer::IsBoxInventorySlot(TItemPos Cell)
{
	return Cell.IsBoxInventoryCell();
}

bool CPythonPlayer::IsEfsunInventorySlot(TItemPos Cell)
{
	return Cell.IsEfsunInventoryCell();
}

bool CPythonPlayer::IsCicekInventorySlot(TItemPos Cell)
{
	return Cell.IsCicekInventoryCell();
}
#endif

bool CPythonPlayer::IsInventorySlot(TItemPos Cell)
{
	return !Cell.IsEquipCell() && Cell.IsValidCell();
}

bool CPythonPlayer::IsEquipmentSlot(TItemPos Cell)
{
	return Cell.IsEquipCell();
}

bool CPythonPlayer::IsEquipItemInSlot(TItemPos Cell)
{
	if (!Cell.IsEquipCell())
	{
		return false;
	}

	const TItemData * pData = GetItemData(Cell);
	
	if (NULL == pData)
	{
		return false;
	}

	DWORD dwItemIndex = pData->vnum;

	CItemManager::Instance().SelectItemData(dwItemIndex);
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
	{
		TraceError("Failed to find ItemData - CPythonPlayer::IsEquipItem(window_type=%d, iSlotindex=%d)\n", Cell.window_type, Cell.cell);
		return false;
	}

	return pItemData->IsEquipment() ? true : false;
}


void CPythonPlayer::SetSkill(DWORD dwSlotIndex, DWORD dwSkillIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return;

	m_playerStatus.aSkill[dwSlotIndex].dwIndex = dwSkillIndex;
	m_skillSlotDict[dwSkillIndex] = dwSlotIndex;
}

int CPythonPlayer::GetSkillIndex(DWORD dwSlotIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return 0;

	return m_playerStatus.aSkill[dwSlotIndex].dwIndex;
}

bool CPythonPlayer::GetSkillSlotIndex(DWORD dwSkillIndex, DWORD* pdwSlotIndex)
{
	std::map<DWORD, DWORD>::iterator f=m_skillSlotDict.find(dwSkillIndex);
	if (m_skillSlotDict.end()==f)
	{
		return false;
	}

	*pdwSlotIndex=f->second;

	return true;
}

int CPythonPlayer::GetSkillGrade(DWORD dwSlotIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return 0;

	return m_playerStatus.aSkill[dwSlotIndex].iGrade;
}

int CPythonPlayer::GetSkillLevel(DWORD dwSlotIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return 0;

	return m_playerStatus.aSkill[dwSlotIndex].iLevel;
}

float CPythonPlayer::GetSkillCurrentEfficientPercentage(DWORD dwSlotIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return 0;

	return m_playerStatus.aSkill[dwSlotIndex].fcurEfficientPercentage;
}

float CPythonPlayer::GetSkillNextEfficientPercentage(DWORD dwSlotIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return 0;

	return m_playerStatus.aSkill[dwSlotIndex].fnextEfficientPercentage;
}

void CPythonPlayer::SetSkillLevel(DWORD dwSlotIndex, DWORD dwSkillLevel)
{
	assert(!"CPythonPlayer::SetSkillLevel - 사용하지 않는 함수");
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return;

	m_playerStatus.aSkill[dwSlotIndex].iGrade = -1;
	m_playerStatus.aSkill[dwSlotIndex].iLevel = dwSkillLevel;
}

void CPythonPlayer::SetSkillLevel_(DWORD dwSkillIndex, DWORD dwSkillGrade, DWORD dwSkillLevel)
{
	DWORD dwSlotIndex;
	if (!GetSkillSlotIndex(dwSkillIndex, &dwSlotIndex))
	{
#if defined(ENABLE_RIDING_EXTENDED)
		if (dwSkillIndex >= 285 && dwSkillIndex <= 311)
		{
			SetSkill(dwSkillIndex, dwSkillIndex);
			dwSlotIndex = dwSkillIndex;
		}
		else
#endif
			return;
	}

	if (dwSlotIndex >= SKILL_MAX_NUM)
		return;

	switch (dwSkillGrade)
	{
		case 0:
			m_playerStatus.aSkill[dwSlotIndex].iGrade = dwSkillGrade;
			m_playerStatus.aSkill[dwSlotIndex].iLevel = dwSkillLevel;
			break;
		case 1:
			m_playerStatus.aSkill[dwSlotIndex].iGrade = dwSkillGrade;
			m_playerStatus.aSkill[dwSlotIndex].iLevel = dwSkillLevel-20+1;
			break;
		case 2:
			m_playerStatus.aSkill[dwSlotIndex].iGrade = dwSkillGrade;
			m_playerStatus.aSkill[dwSlotIndex].iLevel = dwSkillLevel-30+1;
			break;
		case 3:
			m_playerStatus.aSkill[dwSlotIndex].iGrade = dwSkillGrade;
			m_playerStatus.aSkill[dwSlotIndex].iLevel = dwSkillLevel-40+1;
			break;
	}

	const DWORD SKILL_MAX_LEVEL = 40;

	if (dwSkillLevel>SKILL_MAX_LEVEL)
	{
		m_playerStatus.aSkill[dwSlotIndex].fcurEfficientPercentage = 0.0f;
		m_playerStatus.aSkill[dwSlotIndex].fnextEfficientPercentage = 0.0f;

		TraceError("CPythonPlayer::SetSkillLevel(SlotIndex=%d, SkillLevel=%d)", dwSlotIndex, dwSkillLevel);
		return;
	}

	m_playerStatus.aSkill[dwSlotIndex].fcurEfficientPercentage	= LocaleService_GetSkillPower(dwSkillLevel)/100.0f;
	m_playerStatus.aSkill[dwSlotIndex].fnextEfficientPercentage = LocaleService_GetSkillPower(dwSkillLevel+1)/100.0f;

}

void CPythonPlayer::SetSkillCoolTime(DWORD dwSkillIndex)
{
	DWORD dwSlotIndex;
	if (!GetSkillSlotIndex(dwSkillIndex, &dwSlotIndex))
	{
		Tracenf("CPythonPlayer::SetSkillCoolTime(dwSkillIndex=%d) - FIND SLOT ERROR", dwSkillIndex);
		return;
	}

	if (dwSlotIndex>=SKILL_MAX_NUM)
	{
		Tracenf("CPythonPlayer::SetSkillCoolTime(dwSkillIndex=%d) - dwSlotIndex=%d/%d OUT OF RANGE", dwSkillIndex, dwSlotIndex, SKILL_MAX_NUM);
		return;
	}

	m_playerStatus.aSkill[dwSlotIndex].isCoolTime=true;
}

void CPythonPlayer::EndSkillCoolTime(DWORD dwSkillIndex)
{
	DWORD dwSlotIndex;
	if (!GetSkillSlotIndex(dwSkillIndex, &dwSlotIndex))
	{
		Tracenf("CPythonPlayer::EndSkillCoolTime(dwSkillIndex=%d) - FIND SLOT ERROR", dwSkillIndex);
		return;
	}

	if (dwSlotIndex>=SKILL_MAX_NUM)
	{
		Tracenf("CPythonPlayer::EndSkillCoolTime(dwSkillIndex=%d) - dwSlotIndex=%d/%d OUT OF RANGE", dwSkillIndex, dwSlotIndex, SKILL_MAX_NUM);
		return;
	}

	m_playerStatus.aSkill[dwSlotIndex].isCoolTime=false;
}

float CPythonPlayer::GetSkillCoolTime(DWORD dwSlotIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return 0.0f;

	return m_playerStatus.aSkill[dwSlotIndex].fCoolTime;
}

float CPythonPlayer::GetSkillElapsedCoolTime(DWORD dwSlotIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return 0.0f;

	return CTimer::Instance().GetCurrentSecond() - m_playerStatus.aSkill[dwSlotIndex].fLastUsedTime;
}

void CPythonPlayer::__ActivateSkillSlot(DWORD dwSlotIndex)
{
	if (dwSlotIndex>=SKILL_MAX_NUM)
	{
		Tracenf("CPythonPlayer::ActivavteSkill(dwSlotIndex=%d/%d) - OUT OF RANGE", dwSlotIndex, SKILL_MAX_NUM);
		return;
	}

	m_playerStatus.aSkill[dwSlotIndex].bActive = TRUE;
	PyCallClassMemberFunc(m_ppyGameWindow, "ActivateSkillSlot", Py_BuildValue("(i)", dwSlotIndex));
}

void CPythonPlayer::__DeactivateSkillSlot(DWORD dwSlotIndex)
{
	if (dwSlotIndex>=SKILL_MAX_NUM)
	{
		Tracenf("CPythonPlayer::DeactivavteSkill(dwSlotIndex=%d/%d) - OUT OF RANGE", dwSlotIndex, SKILL_MAX_NUM);
		return;
	}

	m_playerStatus.aSkill[dwSlotIndex].bActive = FALSE;
	PyCallClassMemberFunc(m_ppyGameWindow, "DeactivateSkillSlot", Py_BuildValue("(i)", dwSlotIndex));
}

BOOL CPythonPlayer::IsSkillCoolTime(DWORD dwSlotIndex)
{
	if (!__CheckRestSkillCoolTime(dwSlotIndex))
		return FALSE;

	return TRUE;
}

BOOL CPythonPlayer::IsSkillActive(DWORD dwSlotIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return FALSE;

	return m_playerStatus.aSkill[dwSlotIndex].bActive;
}

BOOL CPythonPlayer::IsToggleSkill(DWORD dwSlotIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return FALSE;

	DWORD dwSkillIndex = m_playerStatus.aSkill[dwSlotIndex].dwIndex;

	CPythonSkill::TSkillData * pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(dwSkillIndex, &pSkillData))
		return FALSE;

	return pSkillData->IsToggleSkill();
}

void CPythonPlayer::SetPlayTime(DWORD dwPlayTime)
{
	m_dwPlayTime = dwPlayTime;
}

DWORD CPythonPlayer::GetPlayTime()
{
	return m_dwPlayTime;
}

void CPythonPlayer::SendClickItemPacket(DWORD dwIID)
{
	if (IsObserverMode())
		return;

	static DWORD s_dwNextTCPTime = 0;

	DWORD dwCurTime = ELTimer_GetMSec();

#ifndef ENABLE_INSTANT_PICKUP_SYSTEM
	if (dwCurTime >= s_dwNextTCPTime)
#endif
	{
#ifndef ENABLE_INSTANT_PICKUP_SYSTEM
		s_dwNextTCPTime = dwCurTime + 500;
#endif

		const char* c_szOwnerName;
		if (!CPythonItem::Instance().GetOwnership(dwIID, &c_szOwnerName))
			return;

		if (strlen(c_szOwnerName) > 0)
			if (0 != strcmp(c_szOwnerName, GetName()))
			{
				CItemData* pItemData;
				if (!CItemManager::Instance().GetItemDataPointer(CPythonItem::Instance().GetVirtualNumberOfGroundItem(dwIID), &pItemData))
				{
					Tracenf("CPythonPlayer::SendClickItemPacket(dwIID=%d) : Non-exist item.", dwIID);
					return;
				}
				if (!IsPartyMemberByName(c_szOwnerName) || pItemData->IsAntiFlag(CItemData::ITEM_ANTIFLAG_DROP | CItemData::ITEM_ANTIFLAG_GIVE))
				{
					PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotPickItem", Py_BuildValue("()"));
					return;
				}
			}

		CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
		rkNetStream.SendItemPickUpPacket(dwIID);
	}
}

void CPythonPlayer::__SendClickActorPacket(CInstanceBase& rkInstVictim)
{
	// �� 타고 광산� �는 것� 대한 예외 처리
	CInstanceBase* pkInstMain=NEW_GetMainActorPtr();
	if (pkInstMain)
	if (pkInstMain->IsHoldingPickAxe())
	#ifdef ENABLE_STANDING_MOUNT
	if (pkInstMain->IsMountingHorse() && !pkInstMain->IsMountingHoverBoard())
#else
	if (pkInstMain->IsMountingHorse())
#endif
	if (rkInstVictim.IsResource())
	{
		PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotMining", Py_BuildValue("()"));
		return;
	}

	static DWORD s_dwNextTCPTime = 0;

	DWORD dwCurTime=ELTimer_GetMSec();

	if (dwCurTime >= s_dwNextTCPTime)
	{
		s_dwNextTCPTime=dwCurTime+1000;

		CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();

		DWORD dwVictimVID=rkInstVictim.GetVirtualID();
		rkNetStream.SendOnClickPacket(dwVictimVID);
	}
}

void CPythonPlayer::ActEmotion(DWORD dwEmotionID)
{
	CInstanceBase * pkInstTarget = __GetAliveTargetInstancePtr();
	if (!pkInstTarget)
	{
		PyCallClassMemberFunc(m_ppyGameWindow, "OnCannotShotError", Py_BuildValue("(is)", GetMainCharacterIndex(), "NEED_TARGET"));
		return;
	}

	CPythonNetworkStream::Instance().SendChatPacket(_getf("/kiss %s", pkInstTarget->GetNameString()));
}

void CPythonPlayer::StartEmotionProcess()
{
	__ClearReservedAction();
	__ClearAutoAttackTargetActorID();

	m_bisProcessingEmotion = TRUE;
}

void CPythonPlayer::EndEmotionProcess()
{
	m_bisProcessingEmotion = FALSE;
}

BOOL CPythonPlayer::__IsProcessingEmotion()
{
	return m_bisProcessingEmotion;
}

// Dungeon
void CPythonPlayer::SetDungeonDestinationPosition(int ix, int iy)
{
	m_isDestPosition = TRUE;
	m_ixDestPos = ix;
	m_iyDestPos = iy;

	AlarmHaveToGo();
}

void CPythonPlayer::AlarmHaveToGo()
{
	m_iLastAlarmTime = CTimer::Instance().GetCurrentMillisecond();

	/////

	CInstanceBase * pInstance = NEW_GetMainActorPtr();
	if (!pInstance)
		return;

	TPixelPosition PixelPosition;
	pInstance->NEW_GetPixelPosition(&PixelPosition);

	float fAngle = GetDegreeFromPosition2(PixelPosition.x, PixelPosition.y, float(m_ixDestPos), float(m_iyDestPos));
	fAngle = fmod(540.0f - fAngle, 360.0f);
	D3DXVECTOR3 v3Rotation(0.0f, 0.0f, fAngle);

	PixelPosition.y *= -1.0f;

	CEffectManager::Instance().RegisterEffect("d:/ymir work/effect/etc/compass/appear_middle.mse");
	CEffectManager::Instance().CreateEffect("d:/ymir work/effect/etc/compass/appear_middle.mse", PixelPosition, v3Rotation);
}

// Party
void CPythonPlayer::ExitParty()
{
	m_PartyMemberMap.clear();

	CPythonCharacterManager::Instance().RefreshAllPCTextTail();
}

void CPythonPlayer::AppendPartyMember(DWORD dwPID, const char * c_szName)
{
	m_PartyMemberMap.insert(make_pair(dwPID, TPartyMemberInfo(dwPID, c_szName)));
}

void CPythonPlayer::LinkPartyMember(DWORD dwPID, DWORD dwVID)
{
	TPartyMemberInfo * pPartyMemberInfo;
	if (!GetPartyMemberPtr(dwPID, &pPartyMemberInfo))
	{
		TraceError(" CPythonPlayer::LinkPartyMember(dwPID=%d, dwVID=%d) - Failed to find party member", dwPID, dwVID);
		return;
	}

	pPartyMemberInfo->dwVID = dwVID;

	CInstanceBase * pInstance = NEW_FindActorPtr(dwVID);
	if (pInstance)
		pInstance->RefreshTextTail();
}

void CPythonPlayer::UnlinkPartyMember(DWORD dwPID)
{
	TPartyMemberInfo * pPartyMemberInfo;
	if (!GetPartyMemberPtr(dwPID, &pPartyMemberInfo))
	{
		TraceError(" CPythonPlayer::UnlinkPartyMember(dwPID=%d) - Failed to find party member", dwPID);
		return;
	}

	pPartyMemberInfo->dwVID = 0;
}

void CPythonPlayer::UpdatePartyMemberInfo(DWORD dwPID, BYTE byState, BYTE byHPPercentage)
{
	TPartyMemberInfo * pPartyMemberInfo;
	if (!GetPartyMemberPtr(dwPID, &pPartyMemberInfo))
	{
		TraceError(" CPythonPlayer::UpdatePartyMemberInfo(dwPID=%d, byState=%d, byHPPercentage=%d) - Failed to find character", dwPID, byState, byHPPercentage);
		return;
	}

	pPartyMemberInfo->byState = byState;
	pPartyMemberInfo->byHPPercentage = byHPPercentage;
}

void CPythonPlayer::UpdatePartyMemberAffect(DWORD dwPID, BYTE byAffectSlotIndex, short sAffectNumber)
{
	if (byAffectSlotIndex >= PARTY_AFFECT_SLOT_MAX_NUM)
	{
		TraceError(" CPythonPlayer::UpdatePartyMemberAffect(dwPID=%d, byAffectSlotIndex=%d, sAffectNumber=%d) - Strange affect slot index", dwPID, byAffectSlotIndex, sAffectNumber);
		return;
	}

	TPartyMemberInfo * pPartyMemberInfo;
	if (!GetPartyMemberPtr(dwPID, &pPartyMemberInfo))
	{
		TraceError(" CPythonPlayer::UpdatePartyMemberAffect(dwPID=%d, byAffectSlotIndex=%d, sAffectNumber=%d) - Failed to find character", dwPID, byAffectSlotIndex, sAffectNumber);
		return;
	}

	pPartyMemberInfo->sAffects[byAffectSlotIndex] = sAffectNumber;
}

void CPythonPlayer::RemovePartyMember(DWORD dwPID)
{
	DWORD dwVID = 0;
	TPartyMemberInfo * pPartyMemberInfo;
	if (GetPartyMemberPtr(dwPID, &pPartyMemberInfo))
	{
		dwVID = pPartyMemberInfo->dwVID;
	}

	m_PartyMemberMap.erase(dwPID);

	if (dwVID > 0)
	{
		CInstanceBase * pInstance = NEW_FindActorPtr(dwVID);
		if (pInstance)
			pInstance->RefreshTextTail();
	}
}

bool CPythonPlayer::IsPartyMemberByVID(DWORD dwVID)
{
	std::map<DWORD, TPartyMemberInfo>::iterator itor = m_PartyMemberMap.begin();
	for (; itor != m_PartyMemberMap.end(); ++itor)
	{
		TPartyMemberInfo & rPartyMemberInfo = itor->second;
		if (dwVID == rPartyMemberInfo.dwVID)
			return true;
	}

	return false;
}

bool CPythonPlayer::IsPartyMemberByName(const char * c_szName)
{
	std::map<DWORD, TPartyMemberInfo>::iterator itor = m_PartyMemberMap.begin();
	for (; itor != m_PartyMemberMap.end(); ++itor)
	{
		TPartyMemberInfo & rPartyMemberInfo = itor->second;
		if (0 == rPartyMemberInfo.strName.compare(c_szName))
			return true;
	}

	return false;
}

bool CPythonPlayer::GetPartyMemberPtr(DWORD dwPID, TPartyMemberInfo ** ppPartyMemberInfo)
{
	std::map<DWORD, TPartyMemberInfo>::iterator itor = m_PartyMemberMap.find(dwPID);

	if (m_PartyMemberMap.end() == itor)
		return false;

	*ppPartyMemberInfo = &(itor->second);

	return true;
}

bool CPythonPlayer::PartyMemberPIDToVID(DWORD dwPID, DWORD * pdwVID)
{
	std::map<DWORD, TPartyMemberInfo>::iterator itor = m_PartyMemberMap.find(dwPID);

	if (m_PartyMemberMap.end() == itor)
		return false;

	const TPartyMemberInfo & c_rPartyMemberInfo = itor->second;
	*pdwVID = c_rPartyMemberInfo.dwVID;

	return true;
}

bool CPythonPlayer::PartyMemberVIDToPID(DWORD dwVID, DWORD * pdwPID)
{
	std::map<DWORD, TPartyMemberInfo>::iterator itor = m_PartyMemberMap.begin();
	for (; itor != m_PartyMemberMap.end(); ++itor)
	{
		TPartyMemberInfo & rPartyMemberInfo = itor->second;
		if (dwVID == rPartyMemberInfo.dwVID)
		{
			*pdwPID = rPartyMemberInfo.dwPID;
			return true;
		}
	}

	return false;
}

bool CPythonPlayer::IsSamePartyMember(DWORD dwVID1, DWORD dwVID2)
{
	return (IsPartyMemberByVID(dwVID1) && IsPartyMemberByVID(dwVID2));
}

// PVP
void CPythonPlayer::RememberChallengeInstance(DWORD dwVID)
{
	m_RevengeInstanceSet.erase(dwVID);
	m_ChallengeInstanceSet.insert(dwVID);
}
void CPythonPlayer::RememberRevengeInstance(DWORD dwVID)
{
	m_ChallengeInstanceSet.erase(dwVID);
	m_RevengeInstanceSet.insert(dwVID);
}
void CPythonPlayer::RememberCantFightInstance(DWORD dwVID)
{
	m_CantFightInstanceSet.insert(dwVID);
}
void CPythonPlayer::ForgetInstance(DWORD dwVID)
{
	m_ChallengeInstanceSet.erase(dwVID);
	m_RevengeInstanceSet.erase(dwVID);
	m_CantFightInstanceSet.erase(dwVID);
}

bool CPythonPlayer::IsChallengeInstance(DWORD dwVID)
{
	return m_ChallengeInstanceSet.end() != m_ChallengeInstanceSet.find(dwVID);
}
bool CPythonPlayer::IsRevengeInstance(DWORD dwVID)
{
	return m_RevengeInstanceSet.end() != m_RevengeInstanceSet.find(dwVID);
}
bool CPythonPlayer::IsCantFightInstance(DWORD dwVID)
{
	return m_CantFightInstanceSet.end() != m_CantFightInstanceSet.find(dwVID);
}

void CPythonPlayer::OpenPrivateShop()
{
	m_isOpenPrivateShop = TRUE;
}
void CPythonPlayer::ClosePrivateShop()
{
	m_isOpenPrivateShop = FALSE;
}

bool CPythonPlayer::IsOpenPrivateShop()
{
	return m_isOpenPrivateShop;
}

bool CPythonPlayer::IsDead()
{
	CInstanceBase* pMainInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
	if (!pMainInstance)
		return false;

	return pMainInstance->IsDead();
}

bool CPythonPlayer::IsPoly()
{
	CInstanceBase* pMainInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
	if (!pMainInstance)
		return false;

	return pMainInstance->IsPoly();
}

void CPythonPlayer::SetObserverMode(bool isEnable)
{
	m_isObserverMode=isEnable;
}

bool CPythonPlayer::IsObserverMode()
{
	return m_isObserverMode;
}


BOOL CPythonPlayer::__ToggleCoolTime()
{
	m_sysIsCoolTime = 1 - m_sysIsCoolTime;
	return m_sysIsCoolTime;
}

BOOL CPythonPlayer::__ToggleLevelLimit()
{
	m_sysIsLevelLimit = 1 - m_sysIsLevelLimit;
	return m_sysIsLevelLimit;
}

void CPythonPlayer::StartStaminaConsume(DWORD dwConsumePerSec, DWORD dwCurrentStamina)
{
	m_isConsumingStamina = TRUE;
	m_fConsumeStaminaPerSec = float(dwConsumePerSec);
	m_fCurrentStamina = float(dwCurrentStamina);

	SetStatus(POINT_STAMINA, dwCurrentStamina);
}

void CPythonPlayer::StopStaminaConsume(DWORD dwCurrentStamina)
{
	m_isConsumingStamina = FALSE;
	m_fConsumeStaminaPerSec = 0.0f;
	m_fCurrentStamina = float(dwCurrentStamina);

	SetStatus(POINT_STAMINA, dwCurrentStamina);
}

DWORD CPythonPlayer::GetPKMode()
{
	CInstanceBase * pInstance = NEW_GetMainActorPtr();
	if (!pInstance)
		return 0;

	return pInstance->GetPKMode();
}

void CPythonPlayer::SetMobileFlag(BOOL bFlag)
{
	m_bMobileFlag = bFlag;
	PyCallClassMemberFunc(m_ppyGameWindow, "RefreshMobile", Py_BuildValue("()"));
}

BOOL CPythonPlayer::HasMobilePhoneNumber()
{
	return m_bMobileFlag;
}

void CPythonPlayer::SetGameWindow(PyObject * ppyObject)
{
	m_ppyGameWindow = ppyObject;
}

void CPythonPlayer::OnMobileJoystick(int action, float x, float y)
{
	if (m_ppyGameWindow)
	{
		PyCallClassMemberFunc(m_ppyGameWindow, "OnMobileJoystick", Py_BuildValue("(iff)", action, x, y));
	}
}

void CPythonPlayer::NEW_ClearSkillData(bool bAll)
{
#ifdef ENABLE_BALATHOR_DUNGEON
	RemoveBalathorEffect();
#endif

	std::map<DWORD, DWORD>::iterator it;

	for (it = m_skillSlotDict.begin(); it != m_skillSlotDict.end();)
	{
		if (bAll || __GetSkillType(it->first) == CPythonSkill::SKILL_TYPE_ACTIVE)
			it = m_skillSlotDict.erase(it);
		else
			++it;
	}

	for (int i = 0; i < SKILL_MAX_NUM; ++i)
	{
		ZeroMemory(&m_playerStatus.aSkill[i], sizeof(TSkillInstance));
	}

	for (int j = 0; j < SKILL_MAX_NUM; ++j)
	{
		// 2004.09.30.myevan.스킬갱신시 스킬 ���트업[+] 버튼� 안나와 처리
		m_playerStatus.aSkill[j].iGrade = 0;
		m_playerStatus.aSkill[j].fcurEfficientPercentage=0.0f;
		m_playerStatus.aSkill[j].fnextEfficientPercentage=0.05f;
	}

	if (m_ppyGameWindow)
		PyCallClassMemberFunc(m_ppyGameWindow, "BINARY_CheckGameButton", Py_BuildNone());
}

#ifdef ENABLE_EXTEND_INVEN_SYSTEM
int CPythonPlayer::GetExtendInvenStage()
{
	return m_exInvenStage;
}

void CPythonPlayer::SetExtendInvenStage(short inven_stage)
{
	m_exInvenStage = inven_stage;
}

int CPythonPlayer::GetExtendInvenMax()
{
	return m_exInvenMax;
}

void CPythonPlayer::SetExtendInvenMax(short inven_max)
{
	m_exInvenMax = inven_max;
}
#endif

void CPythonPlayer::ClearSkillDict()
{
	// ClearSkillDict
	m_skillSlotDict.clear();

#ifdef ENABLE_AUTO_QUQUE_ATTACK
	bTotalQuqueAutoAttack = 3;
	m_vecQuqueAutoAttack.clear();
#endif

	// Game End - Player Data Reset
	m_isOpenPrivateShop = false;
	m_isObserverMode = false;

	m_isConsumingStamina = FALSE;
	m_fConsumeStaminaPerSec = 0.0f;
	m_fCurrentStamina = 0.0f;

	m_bMobileFlag = FALSE;

	__ClearAutoAttackTargetActorID();
}

#ifdef ENABLE_KEYBOARD_SETTINGS_SYSTEM
void CPythonPlayer::OpenKeyChangeWindow()
{
	PyCallClassMemberFunc(m_ppyGameWindow, "OpenKeyChangeWindow", Py_BuildValue("()"));
}
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
void CPythonPlayer::OpenLanguageChangeWindow()
{
	PyCallClassMemberFunc(m_ppyGameWindow, "OpenLanguageChangeWindow", Py_BuildValue("()"));
}
#endif

void CPythonPlayer::Clear()
{
#ifdef ENABLE_QUICKSLOT_IMPROVE
	int iSavedQuickPage = m_playerStatus.lQuickPageIndex;
	memset(&m_playerStatus, 0, sizeof(m_playerStatus));
	m_playerStatus.lQuickPageIndex = iSavedQuickPage;
#else
	memset(&m_playerStatus, 0, sizeof(m_playerStatus));
#endif

	NEW_ClearSkillData(true);

	m_bisProcessingEmotion = FALSE;

	m_dwSendingTargetVID = 0;
	m_fTargetUpdateTime = 0.0f;

// #ifdef ENABLE_BATTLE_ROYALE
	// m_bIsBattleRoyaleEnabled = false;
// #endif

	// Test Code for Status Interface
	m_stName = "";
	m_dwMainCharacterIndex = 0;
	m_dwRace = 0;
	m_dwWeaponMinPower = 0;
	m_dwWeaponMaxPower = 0;
	m_dwWeaponMinMagicPower = 0;
	m_dwWeaponMaxMagicPower = 0;
	m_dwWeaponAddPower = 0;

	/////
	m_MovingCursorPosition = TPixelPosition(0, 0, 0);
	m_fMovingCursorSettingTime = 0.0f;

	m_eReservedMode = MODE_NONE;
	m_fReservedDelayTime = 0.0f;
	m_kPPosReserved = TPixelPosition(0, 0, 0);
	m_dwVIDReserved = 0;
	m_dwIIDReserved = 0;
	m_dwSkillSlotIndexReserved = 0;
	m_dwSkillRangeReserved = 0;

	m_isUp = false;
	m_isDown = false;
	m_isLeft = false;
	m_isRight = false;
	m_isSmtMov = false;
	m_isDirMov = false;
	m_isDirKey = false;
	m_isAtkKey = false;
	m_bMobileDirActive = false;
	m_fMobileDirRot = 0.0f;

	m_isCmrRot = true;
	m_fCmrRotSpd = 20.0f;

	m_iComboOld = 0;

	m_dwVIDPicked=0;
	m_dwIIDPicked=0;

	m_dwcurSkillSlotIndex = DWORD(-1);

	m_dwTargetVID = 0;
	m_dwTargetEndTime = 0;

	m_PartyMemberMap.clear();

	m_ChallengeInstanceSet.clear();
	m_RevengeInstanceSet.clear();

	m_isOpenPrivateShop = false;
	m_isObserverMode = false;

	m_isConsumingStamina = FALSE;
	m_fConsumeStaminaPerSec = 0.0f;
	m_fCurrentStamina = 0.0f;

	m_inGuildAreaID = 0xffffffff;

	m_bMobileFlag = FALSE;

#ifdef ENABLE_OFFICAL_FEATURES
	m_isOpenSafeBox = false;
	m_isOpenMall = false;
#endif

#ifdef ENABLE_AUTO_SYSTEM
	memset(&m_playerStatus, 0, sizeof(m_playerStatus));
	autoStatus = false;
	autoStart = true;	//false
	autoPause = false;
	autohuntStartLocation = TPixelPosition(0, 0, 0);
	findTargetMs = 0;

	zLastSec = 0;
	kpLastMs = 0;
	mpLastMs = 0;
	auto_HP = false;
	autp_MP = false;
	affect_control = 0;

	autoAttackOnOff = false;
	autoSkillOnOff = false;
	autoPositionsOnOff = false;
	autoRangeOnOff = false;
	AutoRestart = false;
	AutoHuntDodgeTime = 0;

	sonHedefBulMs = 0;
	m_mapAffectData.clear();
#endif


	__ClearAutoAttackTargetActorID();
}

#ifdef ENABLE_LEFT_SEAT
void CPythonPlayer::LoadLeftSeatData()
{
	PyCallClassMemberFunc(m_ppyGameWindow, "LoadLeftSeatWaitTimeIndexData", Py_BuildValue("()"));
}
#endif

CPythonPlayer::CPythonPlayer(void)
{
	SetMovableGroundDistance(40.0f);

	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_JEONGWI), 3));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_GEOMGYEONG), 4));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_CHEONGEUN), 19));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_GYEONGGONG), 49));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_EUNHYEONG), 34));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_GONGPO), 64));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_JUMAGAP), 65));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_HOSIN), 94));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_BOHO), 95));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_KWAESOK), 110));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_GICHEON), 96));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_JEUNGRYEOK), 111));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_PABEOP), 66));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_FALLEN_CHEONGEUN), 19));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_GWIGEOM), 63));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_MUYEONG), 78));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_HEUKSIN), 79));
#ifdef ENABLE_WOLFMAN_CHARACTER
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_JEOKRANG), 174));
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_CHEONGRANG), 175));
#endif
#ifdef ENABLE_CONQUEROR_LEVEL
	m_kMap_dwAffectIndexToSkillIndex.insert(make_pair(int(CInstanceBase::AFFECT_CHEONUN), 182));
#endif
	m_ppyGameWindow = NULL;

	m_sysIsCoolTime = TRUE;
	m_sysIsLevelLimit = TRUE;
	m_dwPlayTime = 0;
	#if defined(__ANDROID__) || defined(__APPLE__) || defined(__IOS__)
	m_aeMBFButton[MBT_LEFT]=CPythonPlayer::MBF_AUTO;
#else
	m_aeMBFButton[MBT_LEFT]=CPythonPlayer::MBF_SMART;
#endif
	m_aeMBFButton[MBT_RIGHT]=CPythonPlayer::MBF_CAMERA;
	m_aeMBFButton[MBT_MIDDLE]=CPythonPlayer::MBF_CAMERA;

	memset(m_adwEffect, 0, sizeof(m_adwEffect));

	m_isDestPosition = FALSE;
	m_ixDestPos = 0;
	m_iyDestPos = 0;
	m_iLastAlarmTime = 0;

	Clear();
}

CPythonPlayer::~CPythonPlayer(void)
{
}

#ifdef ENABLE_AUTO_SYSTEM
void CPythonPlayer::AddAffect(DWORD dwType, TPacketAffectElement kElem)
{
	int iAffIndex = GetAffectDataIndex(dwType, kElem.bPointIdxApplyOn);
	if (iAffIndex != -1)
	{
		m_mapAffectData.at(iAffIndex) = kElem;
	}
	else
	{
		m_mapAffectData.push_back(kElem);
	}
}

void CPythonPlayer::RemoveAffect(DWORD dwType, BYTE bApplyOn)
{
	for (TAffectDataVector::iterator it = m_mapAffectData.begin(); it != m_mapAffectData.end(); ++it)
	{
		TPacketAffectElement elem = *it;
		if (elem.dwType == dwType && (bApplyOn == 0 || bApplyOn == elem.bPointIdxApplyOn))
		{
			m_mapAffectData.erase(it);
			break;
		}
	}
}

int CPythonPlayer::GetAffectDataIndex(DWORD dwType, BYTE bApplyOn)
{
	int ret = -1, i = 0;
	for (TAffectDataVector::iterator it = m_mapAffectData.begin(); it != m_mapAffectData.end(); ++it, ++i)
	{
		TPacketAffectElement elem = *it;
		if (elem.dwType == dwType && (bApplyOn == 0 || bApplyOn == elem.bPointIdxApplyOn))
		{
			ret = i;
			break;
		}
	}
	return ret;
}

TPacketAffectElement CPythonPlayer::GetAffectData(DWORD dwType, BYTE bApplyOn)
{
	TPacketAffectElement ret;
	memset(&ret, 0, sizeof(TPacketAffectElement));
	for (TAffectDataVector::iterator it = m_mapAffectData.begin(); it != m_mapAffectData.end(); ++it)
	{
		TPacketAffectElement elem = *it;
		if (elem.dwType == dwType && (bApplyOn == 0 || bApplyOn == elem.bPointIdxApplyOn))
		{
			ret = elem;
			break;
		}
	}
	return ret;
}

int CPythonPlayer::GetAffectDuration(uint32_t dwType)
{
	int ret = 0, i = 0;
	for (TAffectDataVector::iterator it = m_mapAffectData.begin(); it != m_mapAffectData.end(); ++it, ++i)
	{
		if (it->dwType == dwType)
		{
			ret = it->lDuration;
			break;
		}
	}

	return ret;
}

CPythonPlayer::TAffectDataVector CPythonPlayer::GetAffectDataVector(DWORD dwType)
{
	TAffectDataVector vAffect;
	TAffectDataVector::iterator it = m_vecAffectData.begin();
	for (; it != m_vecAffectData.end(); ++it)
	{
		const TPacketAffectElement elem = *it;
		if (elem.dwType == dwType)
			vAffect.push_back(elem);
	}
	return vAffect;
}

void CPythonPlayer::UpdateAuto()
{
	CInstanceBase* pkInstMain = NEW_GetMainActorPtr();
	if (!pkInstMain)
	{
		CInstanceBase* pkOldTarget = CPythonCharacterManager::Instance().GetInstancePtr(m_dwAutoAttackTargetVID);
		if (pkOldTarget)
			pkOldTarget->SetAutoTarget(false);

		__ClearTarget();
		m_dwAutoAttackTargetVID = 0;
		return;
	}

	if (autoStatus)
		pkInstMain->SetAutoAffect(true);

	if (GetAutoPause())
	{
		CInstanceBase* pkOldTarget = CPythonCharacterManager::Instance().GetInstancePtr(m_dwAutoAttackTargetVID);
		if (pkOldTarget)
			pkOldTarget->SetAutoTarget(false);

		__ClearTarget();
		m_dwAutoAttackTargetVID = 0;
		return;
	}

	if (!CanStartAuto())
	{
		CInstanceBase* pkOldTarget = CPythonCharacterManager::Instance().GetInstancePtr(m_dwAutoAttackTargetVID);
		if (pkOldTarget)
			pkOldTarget->SetAutoTarget(false);

		AutoStartOnOff(false);
		CPythonChat::Instance().AppendChat(1, "<Otomatik Av> Premium item kullan�lmad�.");
		return;
	}

	//--------------------------------
	// Sabitler
	//--------------------------------
	const DWORD STUCK_CHECK_TIME = 3000;
	const float STUCK_DIST_SQR = 40000.0f;
	const float DODGE_DIST = 1200.0f;
	const float PROBE_DIST = 400.0f;
	const float PROBE_STEP = 100.0f;
	const DWORD ATTACK_TIMEOUT = 15000;

	//--------------------------------
	// Timerlar
	//--------------------------------
	static DWORD lastAttackTime = ELTimer_GetMSec();
	static DWORD dwLastCheckTime = 0;
	static TPixelPosition kLastPos = {0, 0, 0};
	static int iStuckCount = 0;
	static bool isReturningStart = false;

bool isPass = false;

	if (findTargetMs != 0 &&
		((findTargetMs + AUTO_MAX_KILL_SECOND) <= CTimer::Instance().GetCurrentSecond()))
	{
		isPass = true;
		findTargetMs = 0;
	}

	if (GetAutoSkillOnOff())
		AutoHuntTimer(true);

	if (GetAutoPositionOnOff())
	{
		AutoHuntTimer(false);
		AutoReserveUse();
	}

	//--------------------------------
	// STUCK KONTROL
	//--------------------------------
	if ((m_dwAutoAttackTargetVID != 0 || isReturningStart) && !pkInstMain->IsAttacking())
	{
		DWORD dwCurTime = ELTimer_GetMSec();

		if (dwCurTime - dwLastCheckTime > STUCK_CHECK_TIME)
		{
			TPixelPosition kCurPos;
			pkInstMain->NEW_GetPixelPosition(&kCurPos);

			if (isReturningStart)
			{
				float distToStart = pkInstMain->NEW_GetDistanceFromDestPixelPosition(AutoHuntGetStartLocation());
				if (distToStart < 150.0f)
				{
					isReturningStart = false;
					iStuckCount = 0;
					kLastPos = kCurPos;
					dwLastCheckTime = dwCurTime;
				}
			}

			float fDistSqr =
				(kCurPos.x - kLastPos.x) * (kCurPos.x - kLastPos.x) +
				(kCurPos.y - kLastPos.y) * (kCurPos.y - kLastPos.y);

			if (fDistSqr < STUCK_DIST_SQR)
				iStuckCount++;
			else
				iStuckCount = 0;

			if (iStuckCount == 1)
			{
				TPixelPosition targetPos;
				bool bHasTarget = false;

				if (isReturningStart)
				{
					targetPos = AutoHuntGetStartLocation();
					bHasTarget = true;
				}
				else
				{
					CInstanceBase* pTarget = CPythonCharacterManager::Instance().GetInstancePtr(m_dwAutoAttackTargetVID);
					if (pTarget)
					{
						pTarget->NEW_GetPixelPosition(&targetPos);
						bHasTarget = true;
					}
				}

				__ClearReservedAction();

				if (bHasTarget)
				{

					float dx = targetPos.x - kCurPos.x;
					float dy = targetPos.y - kCurPos.y;
					float dist = sqrtf(dx * dx + dy * dy);

					if (dist > 0.0f)
					{
						const int DIR_COUNT = 8;
						const float PI_VAL = 3.14159265f;

						float baseAngle = atan2f(dy, dx);
						int scanSign = (iStuckCount % 2 == 1) ? 1 : -1;

						bool foundPath = false;
						TPixelPosition dodgePos = kCurPos;

						for (int i = 1; i <= DIR_COUNT; ++i)
						{
							float angle = baseAngle + scanSign * (i * (PI_VAL / 4.0f));

							float dirX = cosf(angle);
							float dirY = sinf(angle);

							bool pathClear = true;

							int steps = PROBE_DIST / PROBE_STEP;

							for (int s = 1; s <= steps; ++s)
							{
								float px = kCurPos.x + dirX * (PROBE_STEP * s);
								float py = kCurPos.y + dirY * (PROBE_STEP * s);

								if (CPythonBackground::Instance().isAttrOn(px, py, (1 << 0)))
								{
									pathClear = false;
									break;
								}
							}

							if (pathClear)
							{
								dodgePos.x = kCurPos.x + dirX * DODGE_DIST;
								dodgePos.y = kCurPos.y + dirY * DODGE_DIST;
								dodgePos.z = kCurPos.z;

								foundPath = true;
								break;
							}
						}

						if (!foundPath)
						{
							dodgePos.x = kCurPos.x - (dx / dist) * DODGE_DIST;
							dodgePos.y = kCurPos.y - (dy / dist) * DODGE_DIST;
							dodgePos.z = kCurPos.z;
						}

						pkInstMain->NEW_MoveToDestPixelPositionDirection(dodgePos);
						AutoHuntDodgeTime = ELTimer_GetMSec() + 1200;
					}
				}
			}

			else if (iStuckCount == 2)
			{
				TPixelPosition targetPos;
				bool bHasTarget = false;

				__ClearReservedAction();

				if (isReturningStart)
				{
					targetPos = AutoHuntGetStartLocation();
					bHasTarget = true;
					isReturningStart = false;
				}
				else
				{
					CInstanceBase* pTarget = CPythonCharacterManager::Instance().GetInstancePtr(m_dwAutoAttackTargetVID);
					if (pTarget)
					{
						pTarget->NEW_GetPixelPosition(&targetPos);
						bHasTarget = true;
					}
				}

				if (bHasTarget)
				{

					pkInstMain->SCRIPT_SetPixelPosition(targetPos.x, targetPos.y);
					CPythonNetworkStream::Instance().SendCharacterStatePacket(targetPos, pkInstMain->GetRotation(), CInstanceBase::FUNC_WAIT, 0);
				}
			}

			if (iStuckCount >= 3)
			{
				CInstanceBase* pkOldTarget =
					CPythonCharacterManager::Instance().GetInstancePtr(m_dwAutoAttackTargetVID);

				if (pkOldTarget)
					pkOldTarget->SetAutoTarget(false);

				__ClearTarget();
				m_dwAutoAttackTargetVID = 0;

				iStuckCount = 0;
				isPass = true;

				if (dwCurTime > 1000)
					sonHedefBulMs = dwCurTime - 1000;
				else
					sonHedefBulMs = 1;
			}

			kLastPos = kCurPos;
			dwLastCheckTime = dwCurTime;
		}
	}
	else
	{
		iStuckCount = 0;
	}

	//--------------------------------
	// RANGE KONTROL
	//--------------------------------
	if (GetAutoRangeOnOff() && m_dwAutoAttackTargetVID != 0)
	{
		CInstanceBase* pTarget =
			CPythonCharacterManager::Instance().GetInstancePtr(m_dwAutoAttackTargetVID);

		if (pTarget)
		{
			TPixelPosition startPos = AutoHuntGetStartLocation();

			float dist =
				pTarget->NEW_GetDistanceFromDestPixelPosition(startPos);

			if (dist > GetAutoHuntRange())
			{
				pTarget->SetAutoTarget(false);

				__ClearTarget();
				m_dwAutoAttackTargetVID = 0;

				findTargetMs = 0;
				sonHedefBulMs = 0;
				lastAttackTime = ELTimer_GetMSec();

				pkInstMain->NEW_MoveToDestPixelPositionDirection(startPos);
				isReturningStart = true;
			}
		}
	}

	//--------------------------------
	// HEDEF BULMA
	//--------------------------------
	if ((m_dwAutoAttackTargetVID == 0 && GetAutoAttackOnOff()) || isPass)
	{
		CInstanceBase* pkOldTarget =
			CPythonCharacterManager::Instance().GetInstancePtr(m_dwAutoAttackTargetVID);

		if (pkOldTarget)
			pkOldTarget->SetAutoTarget(false);

		CInstanceBase* pkTarget =
			CPythonCharacterManager::Instance().AutoHuntingGetMob(pkInstMain, isPass);

		if (pkTarget)
		{
			pkTarget->SetAutoTarget(true);

			SetTarget(pkTarget->GetVirtualID());
			m_dwAutoAttackTargetVID = pkTarget->GetVirtualID();

			sonHedefBulMs = ELTimer_GetMSec() + 10000;
			lastAttackTime = ELTimer_GetMSec();
			isReturningStart = false;
		}
		else
		{
			if (sonHedefBulMs == 0)
				sonHedefBulMs = ELTimer_GetMSec() + 10000;

			if (sonHedefBulMs < ELTimer_GetMSec())
			{
				pkInstMain->NEW_MoveToDestPixelPositionDirection(
					AutoHuntGetStartLocation());

				isReturningStart = true;

				sonHedefBulMs = 0;
			}
		}
	}

	//--------------------------------
	// ATTACK TIMEOUT
	//--------------------------------
	if (m_dwAutoAttackTargetVID != 0 &&
		(ELTimer_GetMSec() - lastAttackTime >= ATTACK_TIMEOUT) &&
		!pkInstMain->IsAttacking())
	{
		CInstanceBase* pTarget =
			CPythonCharacterManager::Instance().GetInstancePtr(m_dwAutoAttackTargetVID);

		if (pTarget)
			pTarget->SetAutoTarget(false);

		__ClearTarget();
		m_dwAutoAttackTargetVID = 0;

		findTargetMs = 0;
		sonHedefBulMs = 0;
		lastAttackTime = ELTimer_GetMSec();

		pkInstMain->NEW_MoveToDestPixelPositionDirection(
			AutoHuntGetStartLocation());
		isReturningStart = true;
	}
}

void CPythonPlayer::AutoStartOnOff(bool gelenDurum)
{
	CInstanceBase* pkInstMain = NEW_GetMainActorPtr();
	if (!pkInstMain)
		return;

	findTargetMs = 0;
	autoStatus = gelenDurum;

#ifdef ENABLE_MESSENGER_RENEWAL
	if (gelenDurum)
		CPythonNetworkStream::Instance().SendMessengerSetConnectionStatePacket(2);
	else
		CPythonNetworkStream::Instance().SendMessengerSetConnectionStatePacket(0);
#endif

	if (!gelenDurum)
	{
		CInstanceBase* pkInstOldTarget = CPythonCharacterManager::Instance().GetInstancePtr(m_dwAutoAttackTargetVID);
		if (pkInstOldTarget)
		{
			pkInstOldTarget->SetAutoTarget(false);
		}

		__ClearReservedAction();
		__ClearAutoAttackTargetActorID();
		pkInstMain->SetAutoAffect(false);
	}
	else
	{
		autohuntStartLocation = TPixelPosition(0, 0, 0);
		pkInstMain->NEW_GetPixelPosition(&autohuntStartLocation);
		pkInstMain->SetAutoAffect(true);
	}

	auto_HP = false;
	autp_MP = false;
}

void CPythonPlayer::AutoHuntTimer(bool beceriSlot)
{
	CInstanceBase* pkInstMain = NEW_GetMainActorPtr();
	DWORD suanTime = CTimer::Instance().GetCurrentSecond();
	int maxMesafe = 3000;

	if (beceriSlot)
	{
		if (pkInstMain->IsUsingSkill())
			return;

		for (BYTE i = 0; i < AUTO_SKILL_SLOT_MAX; i++)
		{
			if (m_playerStatus.aAutoSlot[i].slotPos && m_playerStatus.aAutoSlot[i].fillingTime)
			{
				DWORD sPos = m_playerStatus.aAutoSlot[i].slotPos;
				DWORD sDS = m_playerStatus.aAutoSlot[i].fillingTime;
				DWORD sSK = m_playerStatus.aAutoSlot[i].nextUsage;

				if (sSK <= suanTime)
				{
					CPythonSkill::SSkillData* c_pSkillData;
					if (!CPythonSkill::Instance().GetSkillData(GetSkillIndex(sPos), &c_pSkillData))
					{
						TraceError("otoAv - Failed to find skill by %d", sPos);
						return;
					}

					if (c_pSkillData->IsAttackSkill() && zLastSec != suanTime)
					{
						if (m_dwAutoAttackTargetVID != 0 && __GetAliveTargetInstancePtr() && !IsSkillCoolTime(sPos))
						{	//Eger %1000000 emin degilsen burayi elleme
							CInstanceBase* pInstance = __GetAliveTargetInstancePtr();
							if (!pInstance)
							{
								TraceError("AutoHuntTimer - hedef yok!");
								return;
							}

							maxMesafe = c_pSkillData->IsNeedBow() ? 2500 : 250;

							if ((int)pkInstMain->GetDistance(pInstance) <= maxMesafe && !pInstance->IsDead() && !pInstance->IsStun())
							{
								if (!GetTargetVID() || !m_dwAutoAttackTargetVID)
									return;

								UseAutoSkill(sPos, suanTime, i);
							}
						}
					}
					else
					{
						if (!IsSkillCoolTime(sPos))
						{
							if (IsToggleSkill(sPos) && IsSkillActive(sPos))
								continue;

							UseAutoSkill(sPos, suanTime, i);
						}
					}
				}
			}
		}
	}
	else
	{
		for (int i = (int)AUTO_SKILL_SLOT_MAX; i < AUTO_POSITINO_SLOT_MAX; i++)
		{
			if (m_playerStatus.aAutoSlot[i].slotPos && m_playerStatus.aAutoSlot[i].fillingTime)
			{
				DWORD iPos = m_playerStatus.aAutoSlot[i].slotPos;
				DWORD iDS = m_playerStatus.aAutoSlot[i].fillingTime;
				DWORD iSK = m_playerStatus.aAutoSlot[i].nextUsage;
				int iVnum = CPythonPlayer::Instance().GetItemIndex(TItemPos(INVENTORY, iPos));

				CItemManager::Instance().SelectItemData(iVnum);
				CItemData* pItem = CItemManager::Instance().GetSelectedItemDataPointer();
				if (!pItem)
				{
					// TraceError("Not Used 1", iVnum);
					TraceError("Otomatik av item bilgisi alinamadi. itemVnum=%d", iVnum);
					continue;
				}

				bool kirmiziMi = false, maviMi = false;
				int iType = pItem->GetType(), iSubType = pItem->GetSubType();
				if (iType == pItem->ITEM_TYPE_USE)
				{
					TItemPos itemPos;
					itemPos.cell = iPos;	//#19.01.2019 06:02 - Test et.

					if (iSubType == pItem->USE_POTION)
					{
						for (int k = 0; k < sizeof(auto_red_potions) / sizeof(auto_red_potions[0]); k++)
						{
							if (iVnum == auto_red_potions[k])
							{
								kirmiziMi = true;
								break;
							}
						}

						for (int m = 0; m < sizeof(auto_blue_potions) / sizeof(auto_blue_potions[0]); m++)
						{
							if (iVnum == auto_blue_potions[m])
							{
								maviMi = true;
								break;
							}
						}

						if (kirmiziMi)
						{
							BYTE hpYuzde = MINMAX(0, GetStatus(POINT_HP) * 100 / GetStatus(POINT_MAX_HP), 100);
							if (hpYuzde <= iDS)
								auto_HP = true;
						}

						if (maviMi)
						{
							BYTE spYuzde = MINMAX(0, GetStatus(POINT_SP) * 100 / GetStatus(POINT_MAX_SP), 100);
							if (spYuzde <= iDS)
								autp_MP = true;
						}
					}
					else if (iSubType == pItem->USE_ABILITY_UP || iSubType == pItem->USE_AFFECT)
					{
						if (iSK <= suanTime)
						{
							CPythonNetworkStream::Instance().SendItemUsePacket(itemPos);
							m_playerStatus.aAutoSlot[i].nextUsage = m_playerStatus.aAutoSlot[i].fillingTime + suanTime;
						}
					}
				}
				else if (iType == pItem->ITEM_TYPE_BLEND)
				{
					TItemPos itemPos;
					itemPos.cell = iPos;
					if (iSK <= suanTime)
					{
						CPythonNetworkStream::Instance().SendItemUsePacket(itemPos);
						m_playerStatus.aAutoSlot[i].nextUsage = m_playerStatus.aAutoSlot[i].fillingTime + suanTime;
					}
				}
			}
		}
	}
}

void CPythonPlayer::UseAutoSkill(DWORD dwSlotIndex, long iTime, int slotIndex)
{
	if (dwSlotIndex >= SKILL_MAX_NUM)
		return;

	TSkillInstance& rkSkillInst = m_playerStatus.aSkill[dwSlotIndex];
	CPythonSkill::TSkillData* pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(rkSkillInst.dwIndex, &pSkillData))
		return;

	if (CPythonSkill::SKILL_TYPE_GUILD == pSkillData->byType)
	{
		UseGuildSkill(dwSlotIndex);
		return;
	}

	if (!pSkillData->IsCanUseSkill())
		return;

	if (IsSkillCoolTime(dwSlotIndex))
		return;

	if (pSkillData->IsStandingSkill())
	{
		if (pSkillData->IsToggleSkill())
		{
			if (!IsSkillActive(dwSlotIndex))
			{
				CInstanceBase* pkInstMain = NEW_GetMainActorPtr();
				if (!pkInstMain)
					return;

				if (pkInstMain->IsUsingSkill())
					return;

				//CPythonNetworkStream::Instance().SendUseSkillPacket(rkSkillInst.dwIndex);
				if (__UseSkill(dwSlotIndex))
				{
					m_playerStatus.aAutoSlot[slotIndex].nextUsage = m_playerStatus.aAutoSlot[slotIndex].fillingTime + iTime + 1;
					zLastSec = iTime;
				}
			}
		}
		else
		{
			if (__UseSkill(dwSlotIndex))
			{
				m_playerStatus.aAutoSlot[slotIndex].nextUsage = m_playerStatus.aAutoSlot[slotIndex].fillingTime + iTime + 1;
				zLastSec = iTime;
			}
		}
	}
	else if (m_dwcurSkillSlotIndex == dwSlotIndex)
	{
		if (__UseSkill(m_dwcurSkillSlotIndex))
		{
			m_playerStatus.aAutoSlot[slotIndex].nextUsage = m_playerStatus.aAutoSlot[slotIndex].fillingTime + iTime + 1;
			zLastSec = iTime;
		}
	}

	if (!__IsRightButtonSkillMode())
	{
		if (__UseSkill(dwSlotIndex))
		{
			m_playerStatus.aAutoSlot[slotIndex].nextUsage = m_playerStatus.aAutoSlot[slotIndex].fillingTime + iTime + 1;
			zLastSec = iTime;
		}
	}
	else
	{
		m_dwcurSkillSlotIndex = dwSlotIndex;
		PyCallClassMemberFunc(m_ppyGameWindow, "ChangeCurrentSkill", Py_BuildValue("(i)", dwSlotIndex));
	}
}

void CPythonPlayer::AutoReserveUse()
{
	if (auto_HP)
	{
		TItemPos itemPos;
		BYTE otoAvSlotIndex;
		bool pVar = false;

		for (BYTE i = (BYTE)AUTO_SKILL_SLOT_MAX; i < AUTO_POSITINO_SLOT_MAX; i++)
		{
			if (m_playerStatus.aAutoSlot[i].slotPos)
			{
				DWORD iPos = m_playerStatus.aAutoSlot[i].slotPos;
				int iVnum = CPythonPlayer::Instance().GetItemIndex(TItemPos(INVENTORY, iPos));

				CItemManager::Instance().SelectItemData(iVnum);
				CItemData* pItem = CItemManager::Instance().GetSelectedItemDataPointer();
				if (pItem)
				{
					for (int x = 0; x < sizeof(auto_red_potions) / sizeof(auto_red_potions[0]); x++)
					{
						if (iVnum == auto_red_potions[x])
						{
							pVar = true;
							itemPos.cell = iPos;
							otoAvSlotIndex = i;
							break;
						}
					}
				}
			}
		}

		if (pVar)
		{
			if (GetStatus(POINT_HP) + GetStatus(POINT_HP_RECOVERY) < GetStatus(POINT_MAX_HP))
			{
				if (kpLastMs <= CTimer::Instance().GetCurrentMillisecond())
				{
					CPythonNetworkStream::Instance().SendItemUsePacket(itemPos);
					kpLastMs = CTimer::Instance().GetCurrentMillisecond() + 400;
					AutoSlotControl(otoAvSlotIndex);
				}
			}
			else
				auto_HP = false;
		}
	}

	if (autp_MP)
	{
		TItemPos itemPos;
		BYTE autoSlotIndex;
		bool pVar = false;

		for (BYTE i = (BYTE)AUTO_SKILL_SLOT_MAX; i < AUTO_POSITINO_SLOT_MAX; i++)
		{
			if (m_playerStatus.aAutoSlot[i].slotPos)
			{
				DWORD iPos = m_playerStatus.aAutoSlot[i].slotPos;
				int iVnum = CPythonPlayer::Instance().GetItemIndex(TItemPos(INVENTORY, iPos));

				CItemManager::Instance().SelectItemData(iVnum);
				CItemData* pItem = CItemManager::Instance().GetSelectedItemDataPointer();
				if (pItem)
				{
					for (int x = 0; x < sizeof(auto_blue_potions) / sizeof(auto_blue_potions[0]); x++)
					{
						if (iVnum == auto_blue_potions[x])
						{
							pVar = true;
							itemPos.cell = iPos;
							autoSlotIndex = i;
							break;
						}
					}
				}
			}
		}

		if (pVar)
		{
			if (GetStatus(POINT_SP) + GetStatus(POINT_SP_RECOVERY) < GetStatus(POINT_MAX_SP))
			{
				if (mpLastMs <= CTimer::Instance().GetCurrentMillisecond())
				{
					CPythonNetworkStream::Instance().SendItemUsePacket(itemPos);
					mpLastMs = CTimer::Instance().GetCurrentMillisecond() + 400;
					AutoSlotControl(autoSlotIndex);
				}
			}
			else
				autp_MP = false;
		}
	}
}

bool CPythonPlayer::AutoBarrierCheck(CInstanceBase* pTarget, CInstanceBase* pInstance)
{
	return 0;//empty function
}

void CPythonPlayer::AutoSlotControl(DWORD slotIndex)
{
	if (slotIndex < 0 || slotIndex >= AUTO_POSITINO_SLOT_MAX)
		return;

	if (slotIndex >= AUTO_SKILL_SLOT_MAX)
	{
		DWORD iPos = m_playerStatus.aAutoSlot[slotIndex].slotPos;
		if (CPythonPlayer::Instance().GetItemCount(TItemPos(INVENTORY, iPos)) < 1)
			ClearAutoPositionSlot(slotIndex, true);

		int iVnum = CPythonPlayer::Instance().GetItemIndex(TItemPos(INVENTORY, iPos));
		if (!iVnum)
			ClearAutoPositionSlot(slotIndex, true);
	}

	PyCallClassMemberFunc(m_ppyGameWindow, "AutoSlotRefresh", Py_BuildValue("()"));
}

void CPythonPlayer::SetAutoSkillSlotIndex(int iSlotIndex, DWORD dwIndex)
{
	if (iSlotIndex < 0 || iSlotIndex >= AUTO_SKILL_SLOT_MAX)
		return;

	memset(&m_playerStatus.aAutoSlot[iSlotIndex], 0, sizeof(m_playerStatus.aAutoSlot[iSlotIndex]));

	if (iSlotIndex >= 0 && iSlotIndex < AUTO_SKILL_SLOT_MAX)
	{
		int skillIndex = GetSkillIndex(dwIndex);
		if (skillIndex)
		{
			int skillPuani = GetSkillGrade(dwIndex);
			float skillDerece = GetSkillCurrentEfficientPercentage(dwIndex);
			// TraceError("skillPuani:%d,skillIndex:%d,skillDerece:%d", skillPuani, skillIndex, skillDerece);

			CPythonSkill::SSkillData* c_pSkillData;
			if (!CPythonSkill::Instance().GetSkillData(skillIndex, &c_pSkillData))
				return;

			int skillDS = (int)c_pSkillData->GetSkillCoolTime(skillDerece);
			if (skillDS == 0)
				skillDS = (int)c_pSkillData->GetDuration(skillDerece);

			if (!skillDS)
				return;

			for (BYTE i = 0; i < AUTO_SKILL_SLOT_MAX; i++)
			{
				if (m_playerStatus.aAutoSlot[i].slotPos == dwIndex)
					ClearAutoSKillSlot();
			}

			m_playerStatus.aAutoSlot[iSlotIndex].slotPos = dwIndex;
			m_playerStatus.aAutoSlot[iSlotIndex].fillingTime = skillDS;
			CPythonNetworkStream::Instance().SendAutoCoolTime(iSlotIndex, skillDS);
		}
	}
}

void CPythonPlayer::SetAutoPositionSlotIndex(int iSlotIndex, DWORD dwIndex)
{
	if (iSlotIndex < AUTO_SKILL_SLOT_MAX || iSlotIndex >= AUTO_POSITINO_SLOT_MAX)
		return;

	memset(&m_playerStatus.aAutoSlot[iSlotIndex], 0, sizeof(m_playerStatus.aAutoSlot[iSlotIndex]));


	if (iSlotIndex < AUTO_POSITINO_SLOT_MAX)
	{
		if (dwIndex == 0)
		{
			// CPythonChat::Instance().AppendChat(1, "Not used");
			return;
		}

		int iVnum = GetItemIndex(TItemPos(INVENTORY, dwIndex));
		CItemManager::Instance().SelectItemData(iVnum);
		CItemData* pItem = CItemManager::Instance().GetSelectedItemDataPointer();

		int iType = pItem->GetType();
		int iSubType = pItem->GetSubType();
		int rtDs = 0;

		if (iType == pItem->ITEM_TYPE_USE)
		{
			if (iSubType == pItem->USE_ABILITY_UP)
				rtDs = pItem->GetValue(1);

			if (iSubType == pItem->USE_AFFECT)
				rtDs = pItem->GetValue(3);

			/*if (iSubType == pItem->USE_POTION && (iVnum == 27003 || iVnum == 27006))
				rtDs = 50;*/

			if (iSubType == pItem->USE_POTION)
			{
				for (int k = 0; k < sizeof(auto_red_potions) / sizeof(auto_red_potions[0]); k++)
				{
					if (iVnum == auto_red_potions[k])
					{
						rtDs = 50;
						break;
					}
				}

				for (int m = 0; m < sizeof(auto_blue_potions) / sizeof(auto_blue_potions[0]); m++)
				{
					if (iVnum == auto_blue_potions[m])
					{
						rtDs = 50;
						break;
					}
				}
			}
		}
		else if (iType == pItem->ITEM_TYPE_BLEND)
		{
			rtDs = 600;
		}

		if (rtDs > 0)
		{
			for (auto i = (int)AUTO_SKILL_SLOT_MAX; i < AUTO_POSITINO_SLOT_MAX; i++)
			{
				if (m_playerStatus.aAutoSlot[i].slotPos == dwIndex)
				{
					//TraceError("m_playerStatus.aAutoSlot[i].slotPos:%d, %d,%d", m_playerStatus.aAutoSlot[i].slotPos, m_playerStatus.aAutoSlot[i].fillingTime, m_playerStatus.aAutoSlot[i].nextUsage);
					ClearAutoPositionSlot();
				}
			}

			m_playerStatus.aAutoSlot[iSlotIndex].slotPos = dwIndex;
			m_playerStatus.aAutoSlot[iSlotIndex].fillingTime = rtDs;
			CPythonNetworkStream::Instance().SendAutoCoolTime(iSlotIndex, rtDs);
		}
	}
}

void CPythonPlayer::SetAutoSlotCoolTime(int otoAvSlotIndex, DWORD dSuresi)
{
	if (otoAvSlotIndex < 0 || otoAvSlotIndex >= AUTO_POSITINO_SLOT_MAX)
		return;

	m_playerStatus.aAutoSlot[otoAvSlotIndex].fillingTime = dSuresi;
}

void CPythonPlayer::ClearAutoSKillSlot()
{
	for (int i = 0; i < AUTO_SKILL_SLOT_MAX; ++i)
	{
		m_playerStatus.aAutoSlot[i].slotPos = 0;
		m_playerStatus.aAutoSlot[i].fillingTime = 0;
		m_playerStatus.aAutoSlot[i].nextUsage = 0;
	}
}

void CPythonPlayer::ClearAutoPositionSlot(int autoSlotIndex, bool manual)
{
	if (manual)
	{
		m_playerStatus.aAutoSlot[autoSlotIndex].slotPos = 0;
		m_playerStatus.aAutoSlot[autoSlotIndex].fillingTime = 0;
		m_playerStatus.aAutoSlot[autoSlotIndex].nextUsage = 0;
	}
	else
	{
		for (int i = AUTO_SKILL_SLOT_MAX; i < autoSlotIndex; ++i)
		{
			m_playerStatus.aAutoSlot[i].slotPos = 0;
			m_playerStatus.aAutoSlot[i].fillingTime = 0;
			m_playerStatus.aAutoSlot[i].nextUsage = 0;
		}
	}
}

void CPythonPlayer::ClearAutoAllSlot()
{
	ClearAutoSKillSlot();
	ClearAutoPositionSlot();
}

TAutoSlot& CPythonPlayer::AutoSlotData(int iSlotIndex)
{
	if (iSlotIndex < 0 || iSlotIndex >= AUTO_POSITINO_SLOT_MAX)
	{
		static TAutoSlot s_kOtoAvSlot;
		s_kOtoAvSlot.slotPos = 0;
		s_kOtoAvSlot.fillingTime = 0;
		return s_kOtoAvSlot;
	}

	return m_playerStatus.aAutoSlot[iSlotIndex];
}

void CPythonPlayer::GetAutoSlotIndex(DWORD dwSlotPos, DWORD* dwVnum, DWORD* fillingTime)
{
	TAutoSlot& rkOtoAvSlot = AutoSlotData(dwSlotPos);

	if (dwSlotPos >= AUTO_POSITINO_SLOT_MAX)
	{
		*dwVnum = 0;
		return;
	}

	*dwVnum = rkOtoAvSlot.slotPos;
	*fillingTime = rkOtoAvSlot.fillingTime;
}

int CPythonPlayer::CheckSkillSlotCoolTime(uint8_t bIndex, int iSlotIndex, int iCoolTime)
{
	DWORD dwVnum, fillingTime;

	CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
	rkPlayer.GetAutoSlotIndex(bIndex, &dwVnum, &fillingTime);
	if (dwVnum && dwVnum == iSlotIndex)
	{
		if (bIndex >= 0 && bIndex < AUTO_SKILL_SLOT_MAX)
		{
			int skillIndex = rkPlayer.GetSkillIndex(dwVnum);
			if (skillIndex)
			{
				int skillPuani = rkPlayer.GetSkillGrade(dwVnum);
				float skillDerece = rkPlayer.GetSkillCurrentEfficientPercentage(dwVnum);
				// TraceError("skillPuani:%d,skillIndex:%d,skillDerece:%d", skillPuani, skillIndex, skillDerece);

				CPythonSkill::SSkillData* c_pSkillData;
				if (!CPythonSkill::Instance().GetSkillData(skillIndex, &c_pSkillData))
				{
					TraceError("slotIndex:%d", dwVnum);
					return 0;
				}

				int skillDS = (int)c_pSkillData->GetSkillCoolTime(skillDerece);
				if (skillDS == 0)
				{
					skillDS = (int)c_pSkillData->GetDuration(skillDerece);
				}

				if (!skillDS)
					return 0;

				return iCoolTime < skillDS ? skillDS : iCoolTime;
			}
		}
	}

	return 0;
}

int CPythonPlayer::CheckPositionSlotCoolTime(uint8_t bIndex, int iSlotIndex, int iCoolTime)
{
	DWORD dwVnum, fillingTime;

	CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
	rkPlayer.GetAutoSlotIndex(bIndex, &dwVnum, &fillingTime);
	if (dwVnum && dwVnum == iSlotIndex)
	{
		if (bIndex >= AUTO_SKILL_SLOT_MAX && bIndex < AUTO_POSITINO_SLOT_MAX)
		{
			int iVnum = CPythonPlayer::Instance().GetItemIndex(TItemPos(INVENTORY, dwVnum));
			CItemManager::Instance().SelectItemData(iVnum);
			CItemData* pItem = CItemManager::Instance().GetSelectedItemDataPointer();

			int iType = pItem->GetType(), iSubType = pItem->GetSubType(), rtDs = 0;
			if (iType == pItem->ITEM_TYPE_USE)
			{
				if (iSubType == pItem->USE_ABILITY_UP)
					rtDs = pItem->GetValue(1);
				if (iSubType == pItem->USE_AFFECT)
					rtDs = pItem->GetValue(3);
				if (iSubType == pItem->USE_POTION)
				{
					for (int k = 0; k < sizeof(auto_red_potions) / sizeof(auto_red_potions[0]); k++)
					{
						if (iVnum == auto_red_potions[k])
						{
							rtDs = 50;
							break;
						}
					}
					for (int m = 0; m < sizeof(auto_blue_potions) / sizeof(auto_blue_potions[0]); m++)
					{
						if (iVnum == auto_blue_potions[m])
						{
							rtDs = 50;
							break;
						}
					}

				}
			}
			return rtDs < iCoolTime ? iCoolTime : rtDs;
		}
	}

	return 0;
}
#endif

#ifdef ENABLE_SET_ITEM
void CPythonPlayer::SetItemSetValue(TItemPos Cell, uint8_t set_value)
{
	if (!Cell.IsValidCell())
		return;

	(const_cast <TItemData*>(GetItemData(Cell)))->set_value = set_value;
	PyCallClassMemberFunc(m_ppyGameWindow, "RefreshInventory", Py_BuildValue("()"));
}

uint8_t CPythonPlayer::GetItemSetValue(TItemPos Cell)
{
	if (!Cell.IsValidCell())
		return 0;

	const TItemData* pItem = GetItemData(Cell);
	if (pItem == NULL)
		return 0;

	return pItem->set_value;
}
#endif

#ifdef ENABLE_AUTO_SYSTEM
void CPythonPlayer::SetAutoHuntRange(float fRange)
{
	m_fAutoHuntRange = fRange;
}
void CPythonPlayer::SetAutoHuntRangeEffect(float fRange)
{
	CEffectManager& rkEftMgr = CEffectManager::Instance();
	if (fRange <= 0.0f)
	{
		if (m_dwAutoHuntRangeEffectID != 0)
		{
			rkEftMgr.DestroyEffectInstance(m_dwAutoHuntRangeEffectID);
			m_dwAutoHuntRangeEffectID = 0;
		}
		return;
	}
	if (m_dwAutoHuntRangeEffectID == 0)
	{
		DWORD dwEffectCRC = 0;
		if (rkEftMgr.RegisterEffect2("d:/ymir work/effect/etc/auto_hunt/auto_hunt_range_01.mse", &dwEffectCRC))
		{
			CInstanceBase* pMainInstance = NEW_GetMainActorPtr();
			if (pMainInstance)
			{
				TPixelPosition kPPos;
				pMainInstance->NEW_GetPixelPosition(&kPPos);
				D3DXVECTOR3 kD3DVt3Pos(kPPos.x, -kPPos.y, kPPos.z);
				D3DXVECTOR3 kD3DVt3Dir(0.0f, 0.0f, 1.0f);
				m_dwAutoHuntRangeEffectID = rkEftMgr.CreateEffect(dwEffectCRC, kD3DVt3Pos, kD3DVt3Dir);
			}
		}
	}
	if (m_dwAutoHuntRangeEffectID != 0)
	{
		if (rkEftMgr.SelectEffectInstance(m_dwAutoHuntRangeEffectID))
		{
			float fScale = fRange / 10000.0f; 
			rkEftMgr.SetEffectInstanceScale(fScale);
		}
	}
}
#endif

#ifdef ENABLE_BALATHOR_DUNGEON
const char* CPythonPlayer::GetBalathorEffectFileName(BYTE bEffIndex)
{
	switch (bEffIndex)
	{
	case BALATHOR_EFF_AREA:
		return "d:/ymir work/effect/monster2/guild_whitedragon_boss_s2_area.mse";
	case BALATHOR_EFF_GROUND_ICE:
		return "d:/ymir work/effect/monster2/whitedragon_ground_ice.mse";
	case BALATHOR_EFF_GROUND_DROP:
		return "d:/ymir work/effect/monster2/guild_whitedragon_ground_drop.mse";
	case BALATHOR_EFF_EGG_ARROW:
		return "d:/ymir work/effect/monster2/guild_whitedragon_egg_arrow.mse";
	case BALATHOR_EFF_EGG_FLOOD:
		return "d:/ymir work/effect/monster2/guild_whitedragon_egg_flood.mse";
	case BALATHOR_EFF_EGG_GUARD:
		return "d:/ymir work/effect/monster2/guild_whitedragon_egg_guard.mse";
	case BALATHOR_EFF_EGG_MOOJUK:
		// return "d:/ymir work/effect/monster2/guild_whitedragon_egg_guard.mse";
		return "d:/ymir work/effect/monster2/guild_whitedragon_egg_moojuk.mse";
	case BALATHOR_EFF_RED_ARROW:
		return "d:/ymir work/effect/etc/direction/direction_land.mse";
	case BALATHOR_EFF_POSITION:
		return "d:/ymir work/effect/etc/npc_location/npc_location_direction.mse";
	}
	return NULL;
}
const TBalathorEffect* CPythonPlayer::GetBalathorEffect(DWORD dwID)
{
	for (const auto& eff : m_vecEffects)
	{
		if (eff.dwID == dwID)
			return &eff;
	}
	return NULL;
}
void CPythonPlayer::CheckBalathorEffect(CInstanceBase* pInstance)
{
	DWORD dwVID = pInstance->GetVirtualID();
	for (const auto& effInfo : m_vecEffects)
	{
		if (effInfo.dwVID == dwVID)
			pInstance->AddBalathorEffect(&effInfo);
	}
}

void CPythonPlayer::UpdateBalathorEffect()
{
	if (m_vecEffects.empty())
		return;

	for (const auto& effInfo : m_vecEffects)
	{
		if (effInfo.bEffIndex == BALATHOR_EFF_POSITION && effInfo.dwVID == 0 && effInfo.bIsMain && effInfo.dwEffectIndex != 0)
		{
			CInstanceBase* pInstance = NEW_GetMainActorPtr();
			if (!pInstance)
				return;

			CEffectManager& effMngr = CEffectManager::Instance();

			TPixelPosition mainPosition;
			pInstance->NEW_GetPixelPosition(&mainPosition);

			float fAngle = GetDegreeFromPosition2(effInfo.fX, effInfo.fY, mainPosition.x, mainPosition.y);

			fAngle = fmod(540.0f - fAngle, 360.0f);

			fAngle += 180.0f;

			mainPosition.y *= -1.0f;

			D3DXMATRIX mat;
			D3DXMatrixIdentity(&mat);
			D3DXMatrixRotationZ(&mat, D3DXToRadian(fAngle));

			mat._41 = mainPosition.x;
			mat._42 = mainPosition.y;
			mat._43 = mainPosition.z;

			effMngr.SelectEffectInstance(effInfo.dwEffectIndex);
			effMngr.SetEffectInstanceGlobalMatrix(mat);
		}
	}
}

void CPythonPlayer::RemoveBalathorEffect(DWORD dwID)
{
	auto it_balathor = m_vecEffects.begin();
	while (it_balathor != m_vecEffects.end())
	{
		if (dwID != 0 && it_balathor->dwID != dwID)
		{
			++it_balathor;
			continue;
		}

		if (it_balathor->dwVID == 0)
		{
			if (it_balathor->dwEffectIndex != 0)
				CEffectManager::Instance().DestroyEffectInstance(it_balathor->dwEffectIndex);
		}
		else
		{
			CInstanceBase* pInstance = NEW_FindActorPtr(it_balathor->dwVID);
			if (pInstance)
				pInstance->RemoveBalathorEffect(&*it_balathor);
		}
		it_balathor = m_vecEffects.erase(it_balathor);
	}
}
void CPythonPlayer::AddBalathorEffect(DWORD dwID, DWORD dwVID, BYTE bEffIndex, float fX, float fY, float fZ, float fRotation, bool bIsMain)
{
	TBalathorEffect effectInfo;
	effectInfo.dwID = dwID;
	effectInfo.dwVID = dwVID;
	effectInfo.bEffIndex = bEffIndex;
	effectInfo.fX = fX * 100.0f;
	effectInfo.fY = fY * 100.0f;
	effectInfo.fZ = fZ;
	effectInfo.fRotation = fRotation;
	effectInfo.bIsMain = bIsMain;

	if (effectInfo.dwVID == 0)
	{
		const char* szFileName = CPythonPlayer::Instance().GetBalathorEffectFileName(effectInfo.bEffIndex);
		if (!szFileName)
			return;

		CInstanceBase* pInstance = NEW_GetMainActorPtr();
		if (!pInstance)
			return;

		D3DXVECTOR3 mainPosition = pInstance->GetActorInstance()->GetPosition();

		D3DXVECTOR3 v3Pos(effectInfo.fX, effectInfo.fY, mainPosition.z + effectInfo.fZ);
		v3Pos.y *= -1.0f;

		D3DXVECTOR3 v3Rot(0.0f, 0.0f, effectInfo.fRotation);

		CEffectManager& effMngr = CEffectManager::Instance();
		effMngr.RegisterEffect(szFileName, false, false);
		effectInfo.dwEffectIndex = effMngr.CreateEffect(szFileName, v3Pos, v3Rot);
	}
	else
	{
		effectInfo.dwEffectIndex = 0;

		CInstanceBase* pInstance = NEW_FindActorPtr(effectInfo.dwVID);
		if (pInstance)
			pInstance->AddBalathorEffect(&effectInfo);
	}

	m_vecEffects.emplace_back(effectInfo);
}
#endif

#ifdef ENABLE_QUICKSLOT_IMPROVE
DWORD CPythonPlayer::QuickPageLocalIndexToGlobalQuickSlotIndex(DWORD dwPageIndex, DWORD dwLocalSlotIndex)
{
	if (dwPageIndex >= QUICKSLOT_MAX_LINE)
		return QUICKSLOT_MAX_NUM;

	if (dwLocalSlotIndex >= QUICKSLOT_MAX_COUNT_PER_LINE)
		return QUICKSLOT_MAX_NUM;

	return dwPageIndex * QUICKSLOT_MAX_COUNT_PER_LINE + dwLocalSlotIndex;
}

void CPythonPlayer::RequestAddGlobalQuickSlot(DWORD dwGlobalSlotIndex, BYTE byWndType, WORD wWndItemPos)
{
	if (dwGlobalSlotIndex >= QUICKSLOT_MAX_NUM)
		return;

	CPythonNetworkStream::Instance().SendQuickSlotAddPacket(
		static_cast<WORD>(dwGlobalSlotIndex),
		byWndType,
		wWndItemPos
	);
}

void CPythonPlayer::RequestMoveGlobalQuickSlot(DWORD dwGlobalSrcSlotIndex, DWORD dwGlobalDstSlotIndex)
{
	if (dwGlobalSrcSlotIndex >= QUICKSLOT_MAX_NUM)
		return;

	if (dwGlobalDstSlotIndex >= QUICKSLOT_MAX_NUM)
		return;

	if (dwGlobalSrcSlotIndex == dwGlobalDstSlotIndex)
		return;

	CPythonNetworkStream::Instance().SendQuickSlotMovePacket(
		static_cast<WORD>(dwGlobalSrcSlotIndex),
		static_cast<WORD>(dwGlobalDstSlotIndex)
	);
}
#endif
