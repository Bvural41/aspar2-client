#pragma once
#ifdef ENABLE_INGAME_WIKI
#include "../GameLib/InGameWiki.h"
#include <memory>
#endif

class CPythonNonPlayer : public CSingleton<CPythonNonPlayer>
{
	public:
#ifdef ENABLE_INGAME_WIKI
		enum EMobTypes
		{
			MONSTER,
			NPC,
			STONE,
			WARP,
			DOOR,
			BUILDING,
			AUTO_NPC,
		};
#endif

		enum  EClickEvent
		{
			ON_CLICK_EVENT_NONE		= 0,
			ON_CLICK_EVENT_BATTLE	= 1,
			ON_CLICK_EVENT_SHOP		= 2,
			ON_CLICK_EVENT_TALK		= 3,
			ON_CLICK_EVENT_VEHICLE	= 4,

			ON_CLICK_EVENT_MAX_NUM,
		};

		enum EMobEnchants
		{   
			MOB_ENCHANT_CURSE,
			MOB_ENCHANT_SLOW,   
			MOB_ENCHANT_POISON,
			MOB_ENCHANT_STUN,   
			MOB_ENCHANT_CRITICAL,
			MOB_ENCHANT_PENETRATE,
			MOB_ENCHANTS_MAX_NUM
		};
		enum EMobResists
		{
			MOB_RESIST_SWORD,
			MOB_RESIST_TWOHAND,
			MOB_RESIST_DAGGER,
			MOB_RESIST_BELL,
			MOB_RESIST_FAN,
			MOB_RESIST_BOW,
			MOB_RESIST_FIRE,
			MOB_RESIST_ELECT,
			MOB_RESIST_MAGIC,
			MOB_RESIST_WIND,
			MOB_RESIST_POISON,
			MOB_RESIST_CLAW,
			MOB_RESISTS_MAX_NUM 
		};

#ifdef ENABLE_ELEMENT_TARGET
		enum EMobElements
		{
			MOB_ELEMENT_FIRE,
			MOB_ELEMENT_ICE,
			MOB_ELEMENT_EARTH,
			MOB_ELEMENT_WIND,
			MOB_ELEMENT_DARK,
			MOB_ELEMENT_ELECT,
			MOB_ELEMENT_MAX_NUM
		};
#endif

#define MOB_ATTRIBUTE_MAX_NUM	12
#define MOB_SKILL_MAX_NUM		5

#pragma pack(push)
#pragma pack(1)
		typedef struct SMobSkillLevel
		{
			DWORD       dwVnum;
			BYTE        bLevel;
		} TMobSkillLevel;

		typedef struct SMobTable
		{
			DWORD       dwVnum;
			// char		szName[CHARACTER_NAME_MAX_LEN + 1]; 
			// char		szLocaleName[CHARACTER_NAME_MAX_LEN + 1];
			char		szName[24+1];
			char		szLocaleName[24+1];

			BYTE        bType;                  // Monster, NPC
			BYTE        bRank;                  // PAWN, KNIGHT, KING
			BYTE        bBattleType;            // MELEE, etc..
			BYTE        bLevel;                 // Level
			BYTE        bSize;

			DWORD       dwGoldMin;
			DWORD       dwGoldMax;
			DWORD       dwExp;
			DWORD       dwSungMaExp;
#ifdef ENABLE_MOB_HP_EXTENDED
			long long       dwMaxHP;
#else
			DWORD       dwMaxHP;
#endif
			BYTE        bRegenCycle;
			BYTE        bRegenPercent;
			WORD        wDef;

			DWORD       dwAIFlag;
			DWORD       dwRaceFlag;
			DWORD       dwImmuneFlag;

			BYTE        bStr, bDex, bCon, bInt;
			DWORD       dwDamageRange[2];

			short       sAttackSpeed;
			short       sMovingSpeed;
			BYTE        bAggresiveHPPct;
			WORD        wAggressiveSight;
			WORD        wAttackRange;

			char        cEnchants[MOB_ENCHANTS_MAX_NUM];
			char        cResists[MOB_RESISTS_MAX_NUM];
#ifdef ENABLE_ELEMENT_TARGET
			char        cElements[MOB_ELEMENT_MAX_NUM];
#endif
			DWORD       dwResurrectionVnum;
			DWORD       dwDropItemVnum;

			BYTE        bMountCapacity;
			BYTE        bOnClickType;

			BYTE        bEmpire;
			char        szFolder[64 + 1];
			float       fDamMultiply;
			DWORD       dwSummonVnum;
			DWORD       dwDrainSP;
			DWORD		dwMonsterColor;
		    DWORD       dwPolymorphItemVnum;

			TMobSkillLevel	Skills[MOB_SKILL_MAX_NUM];

		    BYTE		bBerserkPoint;
			BYTE		bStoneSkinPoint;
			BYTE		bGodSpeedPoint;
			BYTE		bDeathBlowPoint;
			BYTE		bRevivePoint;
		} TMobTable;
#pragma pack(pop)

#ifdef ENABLE_INGAME_WIKI
		typedef struct SWikiInfoTable
		{
			~SWikiInfoTable() = default;
			SWikiInfoTable() {
				isSet = false;
				isFiltered = false;
				
				dropList.clear();
				mobTable = nullptr;
			}
			
			bool isSet;
			bool isFiltered;
			
			std::vector<CommonWikiData::TWikiMobDropInfo> dropList;
			std::unique_ptr<TMobTable> mobTable;
		} TWikiInfoTable;
		
		typedef std::list<TMobTable*> TMobTableList;
		typedef std::map<DWORD, TWikiInfoTable> TNonPlayerDataMap;
#else
		typedef std::list<TMobTable *> TMobTableList;
		typedef std::map<DWORD, TMobTable *> TNonPlayerDataMap;
#endif

	public:
		CPythonNonPlayer(void);
		virtual ~CPythonNonPlayer(void);

		void Clear();
		void Destroy();

		bool				LoadNonPlayerData(const char * c_szFileName);

		const TMobTable *	GetTable(DWORD dwVnum);
		bool				GetName(DWORD dwVnum, const char ** c_pszName);
		bool				GetInstanceType(DWORD dwVnum, BYTE* pbType);
		BYTE				GetEventType(DWORD dwVnum);
		BYTE				GetEventTypeByVID(DWORD dwVID);
		DWORD				GetMonsterColor(DWORD dwVnum);
		const char*			GetMonsterName(DWORD dwVnum);

#ifdef ENABLE_INGAME_WIKI
		TWikiInfoTable*						GetWikiTable(DWORD dwVnum);
		static bool							CanRenderMonsterModel(DWORD dwMonsterVnum);
		size_t								WikiLoadClassMobs(BYTE bType, unsigned short fromLvl, unsigned short toLvl);
		void								WikiSetBlacklisted(DWORD vnum);
		void								BuildWikiSearchList();
		std::tuple<const char*, int>		GetMonsterDataByNamePart(const char* namePart);
		std::vector<DWORD>*					WikiGetLastMobs() { return &m_vecTempMob; }
#endif

#ifdef ENABLE_TARGET_INFORMATION_SYSTEM
#ifdef ENABLE_MOB_HP_EXTENDED
		long long			GetMonsterMaxHP(DWORD dwVnum);
#else
		DWORD				GetMonsterMaxHP(DWORD dwVnum);
#endif
		DWORD				GetMonsterRaceFlag(DWORD dwVnum);
		DWORD				GetMonsterLevel(DWORD dwVnum);
		DWORD				GetMonsterDamage1(DWORD dwVnum);
		DWORD				GetMonsterDamage2(DWORD dwVnum);
		DWORD				GetMonsterExp(DWORD dwVnum);
#ifdef ENABLE_CONQUEROR_LEVEL
		DWORD				GetMonsterConquerorExp(DWORD dwVnum);
#endif
		float				GetMonsterDamageMultiply(DWORD dwVnum);
		DWORD				GetMonsterST(DWORD dwVnum);
		DWORD				GetMonsterDX(DWORD dwVnum);
		bool				IsMonsterStone(DWORD dwVnum);
#endif
		// Function for outer
		void				GetMatchableMobList(int iLevel, int iInterval, TMobTableList * pMobTableList);

	protected:
		TNonPlayerDataMap	m_NonPlayerDataMap;

#ifdef ENABLE_INGAME_WIKI
		void								SortMobDataName();

		std::vector<DWORD>					m_vecTempMob;
		std::vector<TMobTable*>				m_vecWikiNameSort;
#endif
};