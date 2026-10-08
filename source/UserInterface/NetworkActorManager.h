#pragma once

#include "InstanceBase.h"

struct SNetworkActorData
{
	std::string m_stName;

	CAffectFlagContainer	m_kAffectFlags;

	BYTE	m_bType;
	DWORD	m_dwVID;
	DWORD	m_dwStateFlags;
	DWORD	m_dwEmpireID;
	DWORD	m_dwRace;
	DWORD	m_dwMovSpd;
	DWORD	m_dwAtkSpd;
	FLOAT	m_fRot;
	LONG	m_lCurX;
	LONG	m_lCurY;
	LONG	m_lSrcX;
	LONG	m_lSrcY;
	LONG	m_lDstX;
	LONG	m_lDstY;

	DWORD	m_dwServerSrcTime;
	DWORD	m_dwClientSrcTime;
	DWORD	m_dwDuration;

	DWORD	m_dwArmor;
	DWORD	m_dwWeapon;
	DWORD	m_dwHair;
#ifdef ENABLE_REFINE_ELEMENT
	BYTE	m_bRefineElementType;
#endif
#ifdef ENABLE_SASH_SYSTEM
	DWORD	m_dwSash;
#endif
#ifdef ENABLE_NEW_ARROW_SYSTEM
	DWORD	m_dwArrowType;
#endif

	DWORD	m_dwOwnerVID;

	short	m_sAlignment;
	BYTE	m_byPKMode;
#if defined(ENABLE_BATTLE_FIELD)
	BYTE	combat_zone_rank;
	DWORD	combat_zone_points;
#endif
	DWORD	m_dwMountVnum;

	DWORD	m_dwGuildID;
	DWORD	m_dwLevel;
#ifdef ENABLE_CONQUEROR_LEVEL
	DWORD	m_dwConquerorLevel;
#endif
#if defined(ENABLE_SHOW_MOB_INFO)
	DWORD	m_dwAIFlag;
#endif
#ifdef ENABLE_AURA_SYSTEM
	DWORD	m_dwAura;
#endif
#ifdef ENABLE_SUPPORT_SYSTEM
	bool	is_support_shaman;
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	BYTE	m_bLanguage;
#endif
#ifdef ENABLE_LEFT_SEAT
	bool m_bLeftSeat;
#endif

	SNetworkActorData();

	void SetDstPosition(DWORD dwServerTime, LONG lDstX, LONG lDstY, DWORD dwDuration);
	void SetPosition(LONG lPosX, LONG lPosY);
	void UpdatePosition();	

	// NETWORK_ACTOR_DATA_COPY
	SNetworkActorData(const SNetworkActorData& src);
	void operator=(const SNetworkActorData& src);
	void __copy__(const SNetworkActorData& src);	
	// END_OF_NETWORK_ACTOR_DATA_COPY
};

struct SNetworkMoveActorData
{
	DWORD	m_dwVID;
	DWORD	m_dwTime;
	LONG	m_lPosX;
	LONG	m_lPosY;
	float	m_fRot;
	DWORD	m_dwFunc;
	DWORD	m_dwArg;
	DWORD	m_dwDuration;

	SNetworkMoveActorData()
	{
		m_dwVID=0;
		m_dwTime=0;
		m_fRot=0.0f;
		m_lPosX=0;
		m_lPosY=0;
		m_dwFunc=0;
		m_dwArg=0;
		m_dwDuration=0;
	}
};

struct SNetworkUpdateActorData
{
	DWORD m_dwVID;
	DWORD m_dwGuildID;
	DWORD m_dwArmor;
	DWORD m_dwWeapon;
	DWORD m_dwHair;
	DWORD m_dwMovSpd;
	DWORD m_dwAtkSpd;
	short m_sAlignment;
	BYTE m_byPKMode;
	DWORD m_dwLevel;
#ifdef ENABLE_REFINE_ELEMENT
	BYTE m_bRefineElementType;
#endif
#ifdef ENABLE_CONQUEROR_LEVEL
	DWORD	m_dwConquerorLevel;
#endif
#if defined(ENABLE_BATTLE_FIELD)
	DWORD combat_zone_points;
#endif
#ifdef ENABLE_AURA_SYSTEM
	DWORD m_dwAura;
#endif
#ifdef ENABLE_SASH_SYSTEM
	DWORD	m_dwSash;
#endif
#ifdef ENABLE_NEW_ARROW_SYSTEM
	DWORD m_dwArrowType;
#endif
#ifdef ENABLE_SUPPORT_SYSTEM
	bool is_support_shaman;
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	BYTE m_bLanguage;
#endif
#ifdef ENABLE_LEFT_SEAT
	bool m_bLeftSeat;
#endif

	DWORD m_dwMountVnum;
	DWORD m_dwStateFlags;
	CAffectFlagContainer m_kAffectFlags;

	SNetworkUpdateActorData()
	{
		m_dwGuildID=0;
		m_dwVID=0;
		m_dwArmor=0;
		m_dwWeapon=0;
		m_dwHair=0;
		m_dwMovSpd=0;
		m_dwAtkSpd=0;
		m_sAlignment=0;
		m_byPKMode=0;
		m_dwMountVnum=0;
		m_dwStateFlags=0;
#ifdef ENABLE_REFINE_ELEMENT
		m_bRefineElementType = 0;
#endif
#ifdef ENABLE_CONQUEROR_LEVEL
		m_dwConquerorLevel = 0;
#endif
#ifdef ENABLE_SASH_SYSTEM
		m_dwSash = 0;
#endif
#ifdef ENABLE_NEW_ARROW_SYSTEM
		m_dwArrowType=0;
#endif
#if defined(ENABLE_BATTLE_FIELD)
		combat_zone_points = 0;
#endif
#ifdef ENABLE_AURA_SYSTEM
		m_dwAura = 0;
#endif
#ifdef ENABLE_SUPPORT_SYSTEM
		is_support_shaman = false;
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		m_bLanguage = 0;
#endif
#ifdef ENABLE_LEFT_SEAT
		m_bLeftSeat = false;
#endif
		m_kAffectFlags.Clear();
	}
};

class CPythonCharacterManager;

class CNetworkActorManager : public CReferenceObject
{
	public:
		CNetworkActorManager();
		virtual ~CNetworkActorManager();

		void Destroy();

		void SetMainActorVID(DWORD dwVID);

		void RemoveActor(DWORD dwVID);
		void AppendActor(const SNetworkActorData& c_rkNetActorData);
		void UpdateActor(const SNetworkUpdateActorData& c_rkNetUpdateActorData);
		void MoveActor(const SNetworkMoveActorData& c_rkNetMoveActorData);

		void SyncActor(DWORD dwVID, LONG lPosX, LONG lPosY);
		void SetActorOwner(DWORD dwOwnerVID, DWORD dwVictimVID);

		void Update();

	protected:
		void __OLD_Update();

		void __UpdateMainActor();

		bool __IsVisiblePos(LONG lPosX, LONG lPosY);
		bool __IsVisibleActor(const SNetworkActorData& c_rkNetActorData);
		bool __IsMainActorVID(DWORD dwVID);

		void __RemoveAllGroundItems();
		void __RemoveAllActors();
		void __RemoveDynamicActors();
		void __RemoveCharacterManagerActor(SNetworkActorData& rkNetActorData);

		SNetworkActorData* __FindActorData(DWORD dwVID);

		CInstanceBase* __AppendCharacterManagerActor(SNetworkActorData& rkNetActorData);
		CInstanceBase* __FindActor(SNetworkActorData& rkNetActorData);
		CInstanceBase* __FindActor(SNetworkActorData& rkNetActorData, LONG lDstX, LONG lDstY);

		CPythonCharacterManager& __GetCharacterManager();

	protected:
		DWORD m_dwMainVID;

		LONG m_lMainPosX;
		LONG m_lMainPosY;

		std::map<DWORD, SNetworkActorData> m_kNetActorDict;
};
