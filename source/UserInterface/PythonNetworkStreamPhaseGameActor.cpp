#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "NetworkActorManager.h"
#include "PythonBackground.h"
#include "PythonApplication.h"
#include "AbstractPlayer.h"
#include "../GameLib/ActorInstance.h"
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
#include "InstanceBase.h"
#endif

void CPythonNetworkStream::__GlobalPositionToLocalPosition(LONG& rGlobalX, LONG& rGlobalY)
{
	CPythonBackground&rkBgMgr=CPythonBackground::Instance();
	rkBgMgr.GlobalPositionToLocalPosition(rGlobalX, rGlobalY);
}

void CPythonNetworkStream::__LocalPositionToGlobalPosition(LONG& rLocalX, LONG& rLocalY)
{
	CPythonBackground&rkBgMgr=CPythonBackground::Instance();
	rkBgMgr.LocalPositionToGlobalPosition(rLocalX, rLocalY);
}

bool CPythonNetworkStream::__CanActMainInstance()
{
	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	CInstanceBase* pkInstMain=rkChrMgr.GetMainInstancePtr();
	if (!pkInstMain)
		return false;

	return pkInstMain->CanAct();
}

void CPythonNetworkStream::__ClearNetworkActorManager()
{
	m_rokNetActorMgr->Destroy();
}

void __SetWeaponPower(IAbstractPlayer& rkPlayer, DWORD dwWeaponID)
{
	DWORD minPower=0;
	DWORD maxPower=0;
	DWORD minMagicPower=0;
	DWORD maxMagicPower=0;
	DWORD addPower=0;

	CItemData* pkWeapon;
#if defined(ENABLE_COSTUME_WEAPON_SYSTEM) || defined(ENABLE_CHANGELOOK_SYSTEM)
	if (CItemManager::Instance().GetItemDataPointer(CPythonPlayer::Instance().GetItemIndex(TItemPos(INVENTORY, c_Equipment_Weapon)), &pkWeapon))
#else
	if (CItemManager::Instance().GetItemDataPointer(dwWeaponID, &pkWeapon))
#endif
	{
		if (pkWeapon->GetType()==CItemData::ITEM_TYPE_WEAPON)
		{
			minPower=pkWeapon->GetValue(3);
			maxPower=pkWeapon->GetValue(4);
			minMagicPower=pkWeapon->GetValue(1);
			maxMagicPower=pkWeapon->GetValue(2);
			addPower=pkWeapon->GetValue(5);
		}
	}

#ifdef ENABLE_COSTUME_WEAPON_SYSTEM
	CItemData * pItemData;
	if (CItemManager::Instance().GetItemDataPointer(CPythonPlayer::Instance().GetItemIndex(TItemPos(INVENTORY, c_Equipment_Weapon)), &pItemData))
	{
		minPower=pItemData->GetValue(3);
		maxPower=pItemData->GetValue(4);
		minMagicPower=pItemData->GetValue(1);
		maxMagicPower=pItemData->GetValue(2);
		addPower=pItemData->GetValue(5);
	}
#endif

	rkPlayer.SetWeaponPower(minPower, maxPower, minMagicPower, maxMagicPower, addPower);
}

//테이블에서 이름이 "." 인 것들
//차후에 서버에서 보내주지 않게 되면 없어질 함수..(서버님께 꼭!!협박; )
bool IsInvisibleRace(WORD raceNum)
{
	switch(raceNum)
	{
	case 20025:
	case 20038:
	case 20039:
		return true;
	default:
		return false;
	}
}

static SNetworkActorData s_kNetActorData;


bool CPythonNetworkStream::RecvCharacterAppendPacket()
{
	TPacketGCCharacterAdd chrAddPacket;
	if (!Recv(sizeof(chrAddPacket), &chrAddPacket))
		return false;

	__GlobalPositionToLocalPosition(chrAddPacket.x, chrAddPacket.y);

	SNetworkActorData kNetActorData;
	kNetActorData.m_bType=chrAddPacket.bType;
	kNetActorData.m_dwMovSpd=chrAddPacket.bMovingSpeed;
	kNetActorData.m_dwAtkSpd=chrAddPacket.bAttackSpeed;
	kNetActorData.m_dwRace=chrAddPacket.wRaceNum;
	
	kNetActorData.m_dwStateFlags=chrAddPacket.bStateFlag;
	kNetActorData.m_dwVID=chrAddPacket.dwVID;
	kNetActorData.m_fRot=chrAddPacket.angle;

	kNetActorData.m_stName="";

	kNetActorData.m_stName="";
	kNetActorData.m_kAffectFlags.CopyData(0, sizeof(chrAddPacket.dwAffectFlag[0]), &chrAddPacket.dwAffectFlag[0]);
	kNetActorData.m_kAffectFlags.CopyData(32, sizeof(chrAddPacket.dwAffectFlag[1]), &chrAddPacket.dwAffectFlag[1]);
#ifdef ENABLE_EXTENDED_AFFECT_FLAG
	kNetActorData.m_kAffectFlags.CopyData(64, sizeof(chrAddPacket.dwAffectFlag[2]), &chrAddPacket.dwAffectFlag[2]);
#endif
	kNetActorData.SetPosition(chrAddPacket.x, chrAddPacket.y);

	kNetActorData.m_sAlignment=0;/*chrAddPacket.sAlignment*/;	
	kNetActorData.m_byPKMode=0;/*chrAddPacket.bPKMode*/;	
	kNetActorData.m_dwGuildID=0;/*chrAddPacket.dwGuild*/;
	kNetActorData.m_dwEmpireID=0;/*chrAddPacket.bEmpire*/;
	kNetActorData.m_dwArmor=0;/*chrAddPacket.awPart[CHR_EQUIPPART_ARMOR]*/;
	kNetActorData.m_dwWeapon=0;/*chrAddPacket.awPart[CHR_EQUIPPART_WEAPON]*/;
	kNetActorData.m_dwHair=0;/*chrAddPacket.awPart[CHR_EQUIPPART_HAIR]*/;
	kNetActorData.m_dwMountVnum=0;/*chrAddPacket.dwMountVnum*/;	
#ifdef ENABLE_REFINE_ELEMENT
	kNetActorData.m_bRefineElementType = 0;
#endif
#ifdef ENABLE_SASH_SYSTEM
	kNetActorData.m_dwSash = 0;
#endif
#ifdef ENABLE_NEW_ARROW_SYSTEM
	kNetActorData.m_dwArrowType = 0;
#endif
#if defined(ENABLE_SHOW_MOB_INFO)
	kNetActorData.m_dwLevel = chrAddPacket.dwLevel;
	kNetActorData.m_dwAIFlag = chrAddPacket.dwAIFlag;
#else
	kNetActorData.m_dwLevel = 0; // 몬스터 레벨 표시 안함
#endif
#ifdef ENABLE_CONQUEROR_LEVEL
	kNetActorData.m_dwConquerorLevel = 0;
#endif
#ifdef ENABLE_AURA_SYSTEM
	kNetActorData.m_dwAura = 0;
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	kNetActorData.m_bLanguage = 0;
#endif

	if(kNetActorData.m_bType != CActorInstance::TYPE_PC && 
		kNetActorData.m_bType != CActorInstance::TYPE_NPC)
	{
		const char * c_szName;
		CPythonNonPlayer& rkNonPlayer=CPythonNonPlayer::Instance();
		if (rkNonPlayer.GetName(kNetActorData.m_dwRace, &c_szName))
			kNetActorData.m_stName = c_szName;
		//else
		//	kNetActorData.m_stName=chrAddPacket.name;

		__RecvCharacterAppendPacket(&kNetActorData);
	}
	else
	{
		s_kNetActorData = kNetActorData;
	}

	return true;
}

bool CPythonNetworkStream::RecvCharacterAdditionalInfo()
{
	TPacketGCCharacterAdditionalInfo chrInfoPacket;
	if (!Recv(sizeof(chrInfoPacket), &chrInfoPacket))
		return false;

	
	SNetworkActorData kNetActorData = s_kNetActorData;
	if (IsInvisibleRace(kNetActorData.m_dwRace))
		return true;

	if (kNetActorData.m_dwVID == chrInfoPacket.dwVID)
	{
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		const char* c_szNpcClientName;

		bool bIsMount = (kNetActorData.m_dwRace >= 20101 && kNetActorData.m_dwRace <= 20284);
		bool bIsPet = (kNetActorData.m_dwRace >= 34001 && kNetActorData.m_dwRace <= 34999);
		bool bIsBuffi = (strstr(chrInfoPacket.name, " - Buffi") != NULL) || (kNetActorData.m_dwRace == 9000);
		if (!bIsBuffi && CPythonNonPlayer::Instance().GetName(kNetActorData.m_dwRace, &c_szNpcClientName))
		{
			if (strstr(c_szNpcClientName, "Şaman - Destek") != NULL)
				bIsBuffi = true;
		}

		if (!bIsMount && !bIsPet && !bIsBuffi)
		{
			if (kNetActorData.m_bType == CActorInstance::TYPE_NPC
				&& !(kNetActorData.m_dwRace >= 30000 && kNetActorData.m_dwRace <= 30007)
				&& !(kNetActorData.m_dwRace >= 34036 && kNetActorData.m_dwRace <= 34054)
				&& !(kNetActorData.m_dwRace >= 34064 && kNetActorData.m_dwRace <= 34065)
				&& !(kNetActorData.m_dwRace >= 34073 && kNetActorData.m_dwRace <= 34082)
				&& !(kNetActorData.m_dwRace >= 34165 && kNetActorData.m_dwRace <= 34166)
				&& !(kNetActorData.m_dwRace >= 34229 && kNetActorData.m_dwRace <= 34231)
				&& (strstr(chrInfoPacket.name, "'s ") == NULL))
			{
				if (CPythonNonPlayer::Instance().GetName(kNetActorData.m_dwRace, &c_szNpcClientName))
					kNetActorData.m_stName = c_szNpcClientName;
				else
					kNetActorData.m_stName = chrInfoPacket.name;
			}
			else
				kNetActorData.m_stName = chrInfoPacket.name;
		}
		else
		{
			kNetActorData.m_stName = chrInfoPacket.name;
		}
#else
		kNetActorData.m_stName = chrInfoPacket.name;
#endif

		kNetActorData.m_dwGuildID = chrInfoPacket.dwGuildID;
#ifdef ENABLE_BATTLE_FIELD
		kNetActorData.combat_zone_rank = chrInfoPacket.combat_zone_rank;
#endif
		kNetActorData.m_dwLevel = chrInfoPacket.dwLevel;
#ifdef ENABLE_CONQUEROR_LEVEL
		kNetActorData.m_dwConquerorLevel = chrInfoPacket.dwConquerorLevel;
#endif
		kNetActorData.m_sAlignment=chrInfoPacket.sAlignment;
		kNetActorData.m_byPKMode=chrInfoPacket.bPKMode;
		kNetActorData.m_dwGuildID=chrInfoPacket.dwGuildID;
		kNetActorData.m_dwEmpireID=chrInfoPacket.bEmpire;
		kNetActorData.m_dwArmor=chrInfoPacket.awPart[CHR_EQUIPPART_ARMOR];
		kNetActorData.m_dwWeapon=chrInfoPacket.awPart[CHR_EQUIPPART_WEAPON];
		kNetActorData.m_dwHair=chrInfoPacket.awPart[CHR_EQUIPPART_HAIR];
#ifdef ENABLE_REFINE_ELEMENT
		kNetActorData.m_bRefineElementType = chrInfoPacket.bRefineElementType;
#endif
#ifdef ENABLE_SASH_SYSTEM
		kNetActorData.m_dwSash = chrInfoPacket.awPart[CHR_EQUIPPART_SASH];
#endif
#ifdef ENABLE_NEW_ARROW_SYSTEM
		kNetActorData.m_dwArrowType = chrInfoPacket.awPart[CHR_PART_ARROW_TYPE];
#endif
#ifdef ENABLE_AURA_SYSTEM
		kNetActorData.m_dwAura = chrInfoPacket.awPart[CHR_EQUIPPART_AURA];
#endif
#ifdef ENABLE_SUPPORT_SYSTEM
		kNetActorData.is_support_shaman = chrInfoPacket.bIsSupportShaman;
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		kNetActorData.m_bLanguage = chrInfoPacket.bLanguage;
#endif
		kNetActorData.m_dwMountVnum=chrInfoPacket.dwMountVnum;

		__RecvCharacterAppendPacket(&kNetActorData);
	}
	else
	{
		TraceError("TPacketGCCharacterAdditionalInfo name=%s vid=%d race=%d Error",chrInfoPacket.name,chrInfoPacket.dwVID,kNetActorData.m_dwRace);
	}
	return true;
}

bool CPythonNetworkStream::RecvCharacterUpdatePacket()
{
	TPacketGCCharacterUpdate chrUpdatePacket;
	if (!Recv(sizeof(chrUpdatePacket), &chrUpdatePacket))
		return false;

	SNetworkUpdateActorData kNetUpdateActorData;
	kNetUpdateActorData.m_dwGuildID=chrUpdatePacket.dwGuildID;
#ifdef ENABLE_BATTLE_FIELD
	kNetUpdateActorData.combat_zone_points = chrUpdatePacket.combat_zone_points;
#endif
	kNetUpdateActorData.m_dwMovSpd=chrUpdatePacket.bMovingSpeed;
	kNetUpdateActorData.m_dwAtkSpd=chrUpdatePacket.bAttackSpeed;
	kNetUpdateActorData.m_dwArmor=chrUpdatePacket.awPart[CHR_EQUIPPART_ARMOR];
	kNetUpdateActorData.m_dwWeapon=chrUpdatePacket.awPart[CHR_EQUIPPART_WEAPON];
	kNetUpdateActorData.m_dwHair=chrUpdatePacket.awPart[CHR_EQUIPPART_HAIR];
#ifdef ENABLE_REFINE_ELEMENT
	kNetUpdateActorData.m_bRefineElementType = chrUpdatePacket.bRefineElementType;
#endif
	kNetUpdateActorData.m_dwVID=chrUpdatePacket.dwVID;	
	kNetUpdateActorData.m_kAffectFlags.CopyData(0, sizeof(chrUpdatePacket.dwAffectFlag[0]), &chrUpdatePacket.dwAffectFlag[0]);
	kNetUpdateActorData.m_kAffectFlags.CopyData(32, sizeof(chrUpdatePacket.dwAffectFlag[1]), &chrUpdatePacket.dwAffectFlag[1]);
#ifdef ENABLE_EXTENDED_AFFECT_FLAG
	kNetUpdateActorData.m_kAffectFlags.CopyData(64, sizeof(chrUpdatePacket.dwAffectFlag[2]), &chrUpdatePacket.dwAffectFlag[2]);
#endif
	kNetUpdateActorData.m_sAlignment=chrUpdatePacket.sAlignment;
	kNetUpdateActorData.m_byPKMode=chrUpdatePacket.bPKMode;
	kNetUpdateActorData.m_dwStateFlags=chrUpdatePacket.bStateFlag;
	kNetUpdateActorData.m_dwMountVnum=chrUpdatePacket.dwMountVnum;
	kNetUpdateActorData.m_dwLevel=chrUpdatePacket.dwLevel;
#ifdef ENABLE_CONQUEROR_LEVEL
	kNetUpdateActorData.m_dwConquerorLevel=chrUpdatePacket.dwConquerorLevel;
#endif
#ifdef ENABLE_SASH_SYSTEM
	kNetUpdateActorData.m_dwSash = chrUpdatePacket.awPart[CHR_EQUIPPART_SASH];
#endif
#ifdef ENABLE_NEW_ARROW_SYSTEM
	kNetUpdateActorData.m_dwArrowType = chrUpdatePacket.awPart[CHR_PART_ARROW_TYPE];
#endif
#ifdef ENABLE_AURA_SYSTEM
	kNetUpdateActorData.m_dwAura = chrUpdatePacket.awPart[CHR_EQUIPPART_AURA];
#endif
#ifdef ENABLE_SUPPORT_SYSTEM
	kNetUpdateActorData.is_support_shaman = chrUpdatePacket.bIsSupportShaman;
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	kNetUpdateActorData.m_bLanguage = chrUpdatePacket.bLanguage;
#endif
#ifdef ENABLE_LEFT_SEAT
	kNetUpdateActorData.m_bLeftSeat = chrUpdatePacket.bLeftSeat;
#endif
	__RecvCharacterUpdatePacket(&kNetUpdateActorData);

	return true;
}

void CPythonNetworkStream::__RecvCharacterAppendPacket(SNetworkActorData * pkNetActorData)
{
	// NOTE : 카메라가 땅에 묻히는 문제의 해결을 위해 메인 캐릭터가 지형에 올려지기
	//        전에 맵을 업데이트 해 높이를 구할 수 있도록 해놓아야 합니다.
	//        단, 게임이 들어갈때가 아닌 이미 캐릭터가 추가 된 이후에만 합니다.
	//        헌데 이동인데 왜 Move로 안하고 Append로 하는지..? - [levites]
	IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();
	if (rkPlayer.IsMainCharacterIndex(pkNetActorData->m_dwVID))
	{
		rkPlayer.SetRace(pkNetActorData->m_dwRace);

		__SetWeaponPower(rkPlayer, pkNetActorData->m_dwWeapon);

		if (rkPlayer.NEW_GetMainActorPtr())
		{
			CPythonBackground::Instance().Update(pkNetActorData->m_lCurX, pkNetActorData->m_lCurY, 0.0f);
			CPythonCharacterManager::Instance().Update();

			// NOTE : 사귀 타워일 경우 GOTO 로 이동시에도 맵 이름을 출력하도록 처리
			{
				std::string strMapName = CPythonBackground::Instance().GetWarpMapName();
				if (strMapName == "metin2_map_deviltower1")
					__ShowMapName(pkNetActorData->m_lCurX, pkNetActorData->m_lCurY);
			}
		}
		else
		{
			__ShowMapName(pkNetActorData->m_lCurX, pkNetActorData->m_lCurY);
		}
	}

	m_rokNetActorMgr->AppendActor(*pkNetActorData);

	if (GetMainActorVID()==pkNetActorData->m_dwVID)
	{
		rkPlayer.SetTarget(0);
		if (m_bComboSkillFlag)
			rkPlayer.SetComboSkillFlag(m_bComboSkillFlag);

		__SetGuildID(pkNetActorData->m_dwGuildID);
		//CPythonApplication::Instance().SkipRenderBuffering(10000);
	}
}

void CPythonNetworkStream::__RecvCharacterUpdatePacket(SNetworkUpdateActorData * pkNetUpdateActorData)
{
	m_rokNetActorMgr->UpdateActor(*pkNetUpdateActorData);

	IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();
	if (rkPlayer.IsMainCharacterIndex(pkNetUpdateActorData->m_dwVID))
	{
		__SetGuildID(pkNetUpdateActorData->m_dwGuildID);
		__SetWeaponPower(rkPlayer, pkNetUpdateActorData->m_dwWeapon);

		__RefreshStatus();
		__RefreshAlignmentWindow();
		__RefreshEquipmentWindow();
		__RefreshInventoryWindow();
#ifdef ENABLE_IMPROVED_LOGOUT_POINTS
		m_akSimplePlayerInfo[m_dwSelectedCharacterIndex].wHairPart = pkNetUpdateActorData->m_dwHair;
		m_akSimplePlayerInfo[m_dwSelectedCharacterIndex].wMainPart = pkNetUpdateActorData->m_dwArmor;
#ifdef ENABLE_SASH_SYSTEM
		m_akSimplePlayerInfo[m_dwSelectedCharacterIndex].wSashPart = pkNetUpdateActorData->m_dwSash;
#endif
#ifdef ENABLE_AURA_SYSTEM
		m_akSimplePlayerInfo[m_dwSelectedCharacterIndex].wAuraPart = pkNetUpdateActorData->m_dwAura;
#endif
#endif
	}
	else
	{
		rkPlayer.NotifyCharacterUpdate(pkNetUpdateActorData->m_dwVID);
	}
}

bool CPythonNetworkStream::RecvCharacterDeletePacket()
{
	TPacketGCCharacterDelete chrDelPacket;

	if (!Recv(sizeof(chrDelPacket), &chrDelPacket))
	{
		TraceError("CPythonNetworkStream::RecvCharacterDeletePacket - Recv Error");
		return false;
	}

	m_rokNetActorMgr->RemoveActor(chrDelPacket.dwVID);

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], 
		"BINARY_PrivateShop_Disappear", 
		Py_BuildValue("(i)", chrDelPacket.dwVID)
	);

	return true;
}

bool CPythonNetworkStream::RecvCharacterMovePacket()
{
	TPacketGCMove kMovePacket;
	if (!Recv(sizeof(TPacketGCMove), &kMovePacket))
	{
		Tracen("CPythonNetworkStream::RecvCharacterMovePacket - PACKET READ ERROR");
		return false;
	}

	__GlobalPositionToLocalPosition(kMovePacket.lX, kMovePacket.lY);

	SNetworkMoveActorData kNetMoveActorData;
	kNetMoveActorData.m_dwArg=kMovePacket.bArg;
	kNetMoveActorData.m_dwFunc=kMovePacket.bFunc;
	kNetMoveActorData.m_dwTime=kMovePacket.dwTime;
	kNetMoveActorData.m_dwVID=kMovePacket.dwVID;
	kNetMoveActorData.m_fRot=kMovePacket.bRot*5.0f;
	kNetMoveActorData.m_lPosX=kMovePacket.lX;
	kNetMoveActorData.m_lPosY=kMovePacket.lY;
	kNetMoveActorData.m_dwDuration=kMovePacket.dwDuration;

	m_rokNetActorMgr->MoveActor(kNetMoveActorData);

	return true;
}

bool CPythonNetworkStream::RecvOwnerShipPacket()
{
	TPacketGCOwnership kPacketOwnership;

	if (!Recv(sizeof(kPacketOwnership), &kPacketOwnership))
		return false;

	m_rokNetActorMgr->SetActorOwner(kPacketOwnership.dwOwnerVID, kPacketOwnership.dwVictimVID);

	return true;
}

bool CPythonNetworkStream::RecvSyncPositionPacket()
{
	TPacketGCSyncPosition kPacketSyncPos;
	if (!Recv(sizeof(kPacketSyncPos), &kPacketSyncPos))
		return false;

	TPacketGCSyncPositionElement kSyncPos;

	UINT uSyncPosCount=(kPacketSyncPos.wSize-sizeof(kPacketSyncPos))/sizeof(kSyncPos);
	for (UINT iSyncPos=0; iSyncPos<uSyncPosCount; ++iSyncPos)
	{		
		if (!Recv(sizeof(TPacketGCSyncPositionElement), &kSyncPos))
			return false;

#ifdef __MOVIE_MODE__
		return true;
#endif __MOVIE_MODE__

		//Tracenf("CPythonNetworkStream::RecvSyncPositionPacket %d (%d, %d)", kSyncPos.dwVID, kSyncPos.lX, kSyncPos.lY);

		__GlobalPositionToLocalPosition(kSyncPos.lX, kSyncPos.lY);
		m_rokNetActorMgr->SyncActor(kSyncPos.dwVID, kSyncPos.lX, kSyncPos.lY);

		/*
		CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();
		CInstanceBase * pkChrInst = rkChrMgr.GetInstancePtr(kSyncPos.dwVID);

		if (pkChrInst)
		{			
			pkChrInst->NEW_SyncPixelPosition(kSyncPos.lX, kSyncPos.lY);		
		}
		*/
}

	return true;
}

#ifdef ENABLE_SUPPORT_SYSTEM
bool CPythonNetworkStream::RecvSupportShamanUseSkill()
{
	TPacketGCSupportShamanSkill packet;
	//TraceError("Skill0");
	if (!Recv(sizeof(TPacketGCSupportShamanSkill), &packet))
	{
		Tracen("CPythonNetworkStream::RecvCharacterMovePacket - PACKET READ ERROR");
		return false;
	}
	CInstanceBase* pkInstClone = CPythonCharacterManager::Instance().GetInstancePtr(packet.dwVid);
	if (!pkInstClone)
		return false;
	DWORD dwSkillIndex = packet.dwVnum;
	CPythonSkill::TSkillData * pSkillData;
	if (!CPythonSkill::Instance().GetSkillData(dwSkillIndex, &pSkillData))
		return false;
	DWORD value;
	if(packet.dwLevel <=17)
		value = 0;
	else if(packet.dwLevel <=30 && packet.dwLevel >17)
		value = 1;
	else if(packet.dwLevel > 30 && packet.dwLevel <= 39)
		value = 2;
	else if(packet.dwLevel >= 40)
		value = 3;
	
	DWORD dwMotionIndex = pSkillData->GetSkillMotionIndex(value);
	if (!dwMotionIndex)
		return false;
	if (!pkInstClone->NEW_UseSkill(dwSkillIndex, dwMotionIndex, 1, false))
	{
		Tracenf("CPythonPlayer::UseGuildSkill(%d) - pkInstMain->NEW_UseSkill - ERROR", dwSkillIndex);
	}
	
	return true;
}
#endif

