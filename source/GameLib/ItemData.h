#pragma once
#include "../EterLib/GrpSubImage.h"
#include "../EterGrnLib/Thing.h"
#include "../UserInterface/Locale_inc.h"
#ifdef ENABLE_INGAME_WIKI
#include "InGameWiki.h"
#endif

class CItemData
{
	public:
		enum
		{
			ITEM_NAME_MAX_LEN = 64,
			ITEM_LIMIT_MAX_NUM = 2,
			ITEM_VALUES_MAX_NUM = 6,
			ITEM_SMALL_DESCR_MAX_LEN = 256,
#if defined(ENABLE_ITEM_APPLY4)
			ITEM_APPLY_MAX_NUM = 4,
#else
			ITEM_APPLY_MAX_NUM = 3,
#endif
			ITEM_SOCKET_MAX_NUM = 6,
#ifdef ENABLE_GLOVE_SYSTEM
			GLOVE_ATTR_MAX_NUM = 7,
#endif
		};

		enum EItemType
		{
			ITEM_TYPE_NONE,					//0
			ITEM_TYPE_WEAPON,				//1
			ITEM_TYPE_ARMOR,				//2
			ITEM_TYPE_USE,					//3
			ITEM_TYPE_AUTOUSE,				//4
			ITEM_TYPE_MATERIAL,				//5
			ITEM_TYPE_SPECIAL,				//6
			ITEM_TYPE_TOOL,					//7
			ITEM_TYPE_LOTTERY,				//8
			ITEM_TYPE_ELK,					//9
			ITEM_TYPE_METIN,				//10
			ITEM_TYPE_CONTAINER,			//11
			ITEM_TYPE_FISH,					//12
			ITEM_TYPE_ROD,					//13
			ITEM_TYPE_RESOURCE,				//14
			ITEM_TYPE_CAMPFIRE,				//15
			ITEM_TYPE_UNIQUE,				//16
			ITEM_TYPE_SKILLBOOK,			//17
			ITEM_TYPE_QUEST,				//18
			ITEM_TYPE_POLYMORPH,			//19
			ITEM_TYPE_TREASURE_BOX,			//20
			ITEM_TYPE_TREASURE_KEY,			//21
			ITEM_TYPE_SKILLFORGET,			//22
			ITEM_TYPE_GIFTBOX,				//23
			ITEM_TYPE_PICK,					//24
			ITEM_TYPE_HAIR,					//25
			ITEM_TYPE_TOTEM,				//26
			ITEM_TYPE_BLEND,				//27
			ITEM_TYPE_COSTUME,				//28
			ITEM_TYPE_DS,					//29
			ITEM_TYPE_SPECIAL_DS,			//30
			ITEM_TYPE_EXTRACT,				//31
			ITEM_TYPE_SECONDARY_COIN,		//32
			ITEM_TYPE_RING,					//33
			ITEM_TYPE_BELT,					//34
#ifdef ENABLE_CHEQUE_SYSTEM
			ITEM_TYPE_WON,					//35
#endif
			ITEM_TYPE_TRANSFER_SCROLL,		//36
#ifdef ENABLE_ITEM_TYPE_GACHA
			ITEM_TYPE_GACHA,				//37
#endif
#ifdef ENABLE_SOUL_SYSTEM
			ITEM_TYPE_SOUL,					//38 soul
#endif
#ifdef ENABLE_EXTENDED_PET_SYSTEM
			ITEM_TYPE_PET_OLD,				//39 pet
#endif
#ifdef ENABLE_PASSIVE_SYSTEM
			ITEM_TYPE_PASSIVE,				//40 passive
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
			ITEM_TYPE_PET,
#endif
			ITEM_TYPE_MAX_NUM,
		};

		enum EWeaponSubTypes
		{
			WEAPON_SWORD,
			WEAPON_DAGGER,
			WEAPON_BOW,
			WEAPON_TWO_HANDED,
			WEAPON_BELL,
			WEAPON_FAN,
			WEAPON_ARROW,
#ifdef ENABLE_NEW_ARROW_SYSTEM
			WEAPON_UNLIMITED_ARROW,
#endif
			WEAPON_MOUNT_SPEAR,
			WEAPON_CLAW,
			WEAPON_NUM_TYPES,
			WEAPON_NONE = WEAPON_NUM_TYPES+1,
		};

		enum EMaterialSubTypes
		{
			MATERIAL_LEATHER,
			MATERIAL_BLOOD,
			MATERIAL_ROOT,
			MATERIAL_NEEDLE,
			MATERIAL_JEWEL,
			MATERIAL_DS_REFINE_NORMAL, 
			MATERIAL_DS_REFINE_BLESSED, 
			MATERIAL_DS_REFINE_HOLLY,
		};

		enum EArmorSubTypes
		{
			ARMOR_BODY,
			ARMOR_HEAD,
			ARMOR_SHIELD,
			ARMOR_WRIST,
			ARMOR_FOOTS,
			ARMOR_NECK,
			ARMOR_EAR,
#ifdef ENABLE_PENDANT
			ARMOR_PENDANT,
#endif
#ifdef ENABLE_GLOVE_SYSTEM
			ARMOR_GLOVE,
#endif
			ARMOR_NUM_TYPES
		};

		enum ECostumeSubTypes
		{
			COSTUME_BODY,				//0	갑옷(main look)
			COSTUME_HAIR,				//1	헤어(탈착가능)
#ifdef ENABLE_SASH_SYSTEM
			COSTUME_SASH,
#endif
#ifdef ENABLE_COSTUME_WEAPON_SYSTEM
			COSTUME_WEAPON,
#endif
			COSTUME_MOUNT,
#ifdef ENABLE_AURA_SYSTEM
			COSTUME_AURA,
#endif
			COSTUME_NUM_TYPES,
		};

		enum EPetSubTypes
		{
#ifdef ENABLE_EXTENDED_PET_SYSTEM
			PET_NONE,
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
			PET_EGG,
			PET_SEAL,
			PET_BOOK,
			PET_FEED,
			PET_ATTR,
			PET_REVIVE,
			PET_BOX,
#endif
			PET_NUM_TYPES,
		};

		enum EUseSubTypes
		{
			USE_POTION,					// 0
			USE_TALISMAN,
			USE_TUNING,
			USE_MOVE,
			USE_TREASURE_BOX,
			USE_MONEYBAG,
			USE_BAIT,
			USE_ABILITY_UP,
			USE_AFFECT,
			USE_CREATE_STONE,
			USE_SPECIAL,				// 10
			USE_POTION_NODELAY,
			USE_CLEAR,
			USE_INVISIBILITY,
			USE_DETACHMENT,
			USE_BUCKET,
			USE_POTION_CONTINUE,
			USE_CLEAN_SOCKET,
			USE_CHANGE_ATTRIBUTE,
			USE_ADD_ATTRIBUTE,
			USE_ADD_ACCESSORY_SOCKET,	// 20
			USE_PUT_INTO_ACCESSORY_SOCKET,
			USE_ADD_ATTRIBUTE2,
			USE_RECIPE,
			USE_CHANGE_ATTRIBUTE2,
			USE_TIME_CHARGE_PER,
			USE_TIME_CHARGE_FIX,				// 28
			USE_PUT_INTO_BELT_SOCKET,			// 29 벨트 소켓에 사용할 수 있는 아이템 
			USE_PUT_INTO_RING_SOCKET,			// 30 반지 소켓에 사용할 수 있는 아이템 (유니크 반지 말고, 새로 추가된 반지 슬롯)
#ifdef ENABLE_SOULBIND_SYSTEM
			USE_BIND,
			USE_UNBIND,
#endif
			USE_COSTUME_ENCHANT,
			USE_COSTUME_TRANSFORM,
#ifdef ENABLE_REFINE_ELEMENT
			USE_ELEMENT_UPGRADE,
			USE_ELEMENT_DOWNGRADE,
			USE_ELEMENT_CHANGE,
#endif
#ifdef ENABLE_SELECT_REWARD_BOX
			USE_SELECT_BOX,
#endif
#ifdef ENABLE_TITLE_SYSTEM
			USE_TITLE,
#endif
		};

#ifdef ENABLE_SOUL_SYSTEM
		enum ESoulSubType
		{
			RED_SOUL,
			BLUE_SOUL,
		};
#endif

		enum EDragonSoulSubType
		{
			DS_SLOT1,
			DS_SLOT2,
			DS_SLOT3,
			DS_SLOT4,
			DS_SLOT5,
			DS_SLOT6,
#ifdef ENABLE_DRAGON_SOUL_7_SLOT
			DS_SLOT7,
#endif
#ifdef ENABLE_DRAGON_SOUL_7_SLOT
			DS_SLOT_NUM_TYPES = 7,
#else
			DS_SLOT_NUM_TYPES = 6,
#endif
		};

		enum EMetinSubTypes
		{
			METIN_NORMAL,
			METIN_GOLD,
#ifdef ENABLE_GLOVE_SYSTEM
			METIN_SUNGMA,
#endif
#ifdef ENABLE_METIN_ELEMENTAL
			METIN_ELEMENTAL,
#endif
		};

		enum ELimitTypes
		{
			LIMIT_NONE,

			LIMIT_LEVEL,
			LIMIT_STR,
			LIMIT_DEX,
			LIMIT_INT,
			LIMIT_CON,
			LIMIT_PCBANG,
			LIMIT_REAL_TIME,
			LIMIT_REAL_TIME_START_FIRST_USE,
			LIMIT_TIMER_BASED_ON_WEAR,
#ifdef ENABLE_NEWWORLD_LEVEL
			LIMIT_NEWWORLD_LEVEL,
#endif
			LIMIT_MAX_NUM
		};

		enum EItemAntiFlag
		{
			ITEM_ANTIFLAG_FEMALE = (1 << 0),
			ITEM_ANTIFLAG_MALE = (1 << 1),
			ITEM_ANTIFLAG_WARRIOR = (1 << 2),
			ITEM_ANTIFLAG_ASSASSIN = (1 << 3),
			ITEM_ANTIFLAG_SURA = (1 << 4),
			ITEM_ANTIFLAG_SHAMAN = (1 << 5),
			ITEM_ANTIFLAG_GET = (1 << 6),
			ITEM_ANTIFLAG_DROP = (1 << 7),
			ITEM_ANTIFLAG_SELL = (1 << 8),
			ITEM_ANTIFLAG_EMPIRE_A = (1 << 9),
			ITEM_ANTIFLAG_EMPIRE_B = (1 << 10),
			ITEM_ANTIFLAG_EMPIRE_R = (1 << 11),
			ITEM_ANTIFLAG_SAVE = (1 << 12),
			ITEM_ANTIFLAG_GIVE = (1 << 13),
			ITEM_ANTIFLAG_PKDROP = (1 << 14),
			ITEM_ANTIFLAG_STACK = (1 << 15),
			ITEM_ANTIFLAG_MYSHOP = (1 << 16),
			ITEM_ANTIFLAG_SAFEBOX = (1 << 17),
			ITEM_ANTIFLAG_WOLFMAN = (1 << 18),
#ifdef ENABLE_SOULBIND_SYSTEM
			ITEM_ANTIFLAG_BIND = (1 << 19),
#endif
		};

		enum EItemFlag
		{
			ITEM_FLAG_REFINEABLE        = (1 << 0),		// 개량 가능
			ITEM_FLAG_SAVE              = (1 << 1),
			ITEM_FLAG_STACKABLE         = (1 << 2),     // 여러개 합칠 수 있음
			ITEM_FLAG_COUNT_PER_1GOLD   = (1 << 3),		// 가격이 개수 / 가격으로 변함
			ITEM_FLAG_SLOW_QUERY        = (1 << 4),		// 게임 종료시에만 SQL에 쿼리함
			ITEM_FLAG_RARE              = (1 << 5),
			ITEM_FLAG_UNIQUE            = (1 << 6),
			ITEM_FLAG_MAKECOUNT			= (1 << 7),
			ITEM_FLAG_IRREMOVABLE		= (1 << 8),
			ITEM_FLAG_CONFIRM_WHEN_USE	= (1 << 9),
			ITEM_FLAG_QUEST_USE         = (1 << 10),    // 퀘스트 스크립트 돌리는지?
			ITEM_FLAG_QUEST_USE_MULTIPLE= (1 << 11),    // 퀘스트 스크립트 돌리는지?
			ITEM_FLAG_UNUSED03          = (1 << 12),    // UNUSED03
			ITEM_FLAG_LOG               = (1 << 13),    // 사용시 로그를 남기는 아이템인가?
			ITEM_FLAG_APPLICABLE		= (1 << 14),
		};

		enum EWearPositions
		{
			WEAR_BODY,				// 0
			WEAR_HEAD,				// 1
			WEAR_FOOTS,				// 2
			WEAR_WRIST,				// 3
			WEAR_WEAPON,			// 4
			WEAR_NECK,				// 5
			WEAR_EAR,				// 6
			WEAR_UNIQUE1,			// 7
			WEAR_UNIQUE2,			// 8
			WEAR_ARROW,				// 9
			WEAR_SHIELD,			// 10
			WEAR_ABILITY1,			// 11
			WEAR_ABILITY2,			// 12
			WEAR_ABILITY3,			// 13
			WEAR_ABILITY4,			// 14
			WEAR_ABILITY5,			// 15
			WEAR_ABILITY6,			// 16
			WEAR_ABILITY7,			// 17
			WEAR_ABILITY8,			// 18
			WEAR_COSTUME_BODY,		// 19
			WEAR_COSTUME_HAIR,		// 20
#ifdef ENABLE_SASH_SYSTEM
			WEAR_COSTUME_SASH,		// 21
#endif
#ifdef ENABLE_COSTUME_WEAPON_SYSTEM
			WEAR_COSTUME_WEAPON,	// 22
#endif
			WEAR_COSTUME_MOUNT,		// 23
			WEAR_COSTUME_AURA,		// 24
			WEAR_RING1,				// 25
			WEAR_RING2,				// 26
			WEAR_BELT,				// 27
#ifdef ENABLE_PENDANT
			WEAR_PENDANT,			// 28
#endif
#ifdef ENABLE_GLOVE_SYSTEM
			WEAR_GLOVE,				// 29
#endif
#ifdef ENABLE_EXTENDED_PET_SYSTEM
			WEAR_PET,				// 30
#endif
#ifdef ENABLE_PASSIVE_SYSTEM
			WEAR_PASSIVE,			// 31
#endif
			WEAR_MAX_NUM,
		};

		enum EItemWearableFlag
		{
			WEARABLE_BODY			= (1 << 0),
			WEARABLE_HEAD			= (1 << 1),
			WEARABLE_FOOTS			= (1 << 2),
			WEARABLE_WRIST			= (1 << 3),
			WEARABLE_WEAPON			= (1 << 4),
			WEARABLE_NECK			= (1 << 5),
			WEARABLE_EAR			= (1 << 6),
			WEARABLE_UNIQUE			= (1 << 7),
			WEARABLE_SHIELD			= (1 << 8),
			WEARABLE_ARROW			= (1 << 9),
			WEARABLE_HAIR			= (1 << 10),
			WEARABLE_ABILITY		= (1 << 11),
			WEARABLE_COSTUME_BODY	= (1 << 12),
			WEARABLE_COSTUME_HAIR	= (1 << 13),
			WEARABLE_COSTUME_SASH	= (1 << 14),
			WEARABLE_COSTUME_WEAPON	= (1 << 15),
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
			WEARABLE_COSTUME_MOUNT	= (1 << 16),
#endif
#ifdef ENABLE_PENDANT
			WEARABLE_PENDANT		= (1 << 17),
#endif
#ifdef ENABLE_GLOVE_SYSTEM
			WEARABLE_GLOVE			= (1 << 18),
#endif
#ifdef ENABLE_EXTENDED_PET_SYSTEM
			WEARABLE_PET			= (1 << 19),
#endif
#ifdef ENABLE_NEW_RING_EQUIPMENT
			WEARABLE_NEW_GLOVE		= (1 << 20),
			WEARABLE_NEW_EXP		= (1 << 21),
#endif
		};

		enum EApplyTypes
		{
			APPLY_NONE,//1
			APPLY_MAX_HP,//2
			APPLY_MAX_SP,//3
			APPLY_CON,//4
			APPLY_INT,//5
			APPLY_STR,//6
			APPLY_DEX,//7
			APPLY_ATT_SPEED,//8
			APPLY_MOV_SPEED,//9
			APPLY_CAST_SPEED,//10
			APPLY_HP_REGEN,//11
			APPLY_SP_REGEN,//12
			APPLY_POISON_PCT,//13
			APPLY_STUN_PCT,//14
			APPLY_SLOW_PCT,//15
			APPLY_CRITICAL_PCT,//16
			APPLY_PENETRATE_PCT,//17
			APPLY_ATTBONUS_HUMAN,//18
			APPLY_ATTBONUS_ANIMAL,//19
			APPLY_ATTBONUS_ORC,//20
			APPLY_ATTBONUS_MILGYO,//21
			APPLY_ATTBONUS_UNDEAD,//22
			APPLY_ATTBONUS_DEVIL,//23
			APPLY_STEAL_HP,//24
			APPLY_STEAL_SP,//25
			APPLY_MANA_BURN_PCT,//26
			APPLY_DAMAGE_SP_RECOVER,//27
			APPLY_BLOCK,//28
			APPLY_DODGE,//29
			APPLY_RESIST_SWORD,//30
			APPLY_RESIST_TWOHAND,//31
			APPLY_RESIST_DAGGER,//32
			APPLY_RESIST_BELL,//33
			APPLY_RESIST_FAN,//34
			APPLY_RESIST_BOW,//35
			APPLY_RESIST_FIRE,//36
			APPLY_RESIST_ELEC,//
			APPLY_RESIST_MAGIC,//
			APPLY_RESIST_WIND,//
			APPLY_REFLECT_MELEE,//
			APPLY_REFLECT_CURSE,//
			APPLY_POISON_REDUCE,//
			APPLY_KILL_SP_RECOVER,//
			APPLY_EXP_DOUBLE_BONUS,//
			APPLY_GOLD_DOUBLE_BONUS,//
			APPLY_ITEM_DROP_BONUS,//
			APPLY_POTION_BONUS,//
			APPLY_KILL_HP_RECOVER,//
			APPLY_IMMUNE_STUN,//
			APPLY_IMMUNE_SLOW,//
			APPLY_IMMUNE_FALL,//
			APPLY_SKILL,//
			APPLY_BOW_DISTANCE,//
			APPLY_ATT_GRADE_BONUS,//
			APPLY_DEF_GRADE_BONUS,//
			APPLY_MAGIC_ATT_GRADE,//
			APPLY_MAGIC_DEF_GRADE,//
			APPLY_CURSE_PCT,//
			APPLY_MAX_STAMINA,//
			APPLY_ATT_BONUS_TO_WARRIOR,//
			APPLY_ATT_BONUS_TO_ASSASSIN,//
			APPLY_ATT_BONUS_TO_SURA,//
			APPLY_ATT_BONUS_TO_SHAMAN,//
			APPLY_ATT_BONUS_TO_MONSTER,//
			APPLY_MALL_ATTBONUS,//
			APPLY_MALL_DEFBONUS,//
			APPLY_MALL_EXPBONUS,//
			APPLY_MALL_ITEMBONUS,//
			APPLY_MALL_GOLDBONUS,//
			APPLY_MAX_HP_PCT,//
			APPLY_MAX_SP_PCT,//
			APPLY_SKILL_DAMAGE_BONUS,//
			APPLY_NORMAL_HIT_DAMAGE_BONUS,//
			APPLY_SKILL_DEFEND_BONUS,//
			APPLY_NORMAL_HIT_DEFEND_BONUS,//
			APPLY_PC_BANG_EXP_BONUS,//
			APPLY_PC_BANG_DROP_BONUS,//
			APPLY_EXTRACT_HP_PCT,///
			APPLY_RESIST_WARRIOR,//
			APPLY_RESIST_ASSASSIN,//
			APPLY_RESIST_SURA,//
			APPLY_RESIST_SHAMAN,//
			APPLY_ENERGY,//
			APPLY_DEF_GRADE,//
			APPLY_COSTUME_ATTR_BONUS,//
			APPLY_MAGIC_ATTBONUS_PER,//
			APPLY_MELEE_MAGIC_ATTBONUS_PER,//
			APPLY_RESIST_ICE,//
			APPLY_RESIST_EARTH,//
			APPLY_RESIST_DARK,//
			APPLY_ANTI_CRITICAL_PCT,
			APPLY_ANTI_PENETRATE_PCT,
			APPLY_BLEEDING_PCT,
			APPLY_BLEEDING_REDUCE,
#ifdef ENABLE_WOLFMAN_CHARACTER
			APPLY_ATTBONUS_WOLFMAN,
			APPLY_RESIST_WOLFMAN,
			APPLY_RESIST_CLAW,
#endif
#ifdef ENABLE_ANTI_RESIST_MAGIC_BONUS_SYSTEM
			APPLY_ANTI_RESIST_MAGIC,	// 97
#endif
#ifdef ENABLE_ELEMENT_NEW_BONUSES
			APPLY_ENCHANT_DARK,
			APPLY_ENCHANT_EARTH,
			APPLY_ENCHANT_ELECT,
			APPLY_ENCHANT_FIRE,
			APPLY_ENCHANT_ICE,
			APPLY_ENCHANT_WIND,
#endif
#ifdef ENABLE_PENDANT
			APPLY_ATTBONUS_SWORD,	//104
			APPLY_ATTBONUS_TWOHAND,
			APPLY_ATTBONUS_DAGGER,
			APPLY_ATTBONUS_BELL,
			APPLY_ATTBONUS_FAN,
			APPLY_ATTBONUS_BOW,
			APPLY_ATTBONUS_CLAW,
			APPLY_RESIST_HUMAN,
			APPLY_ATTBONUS_BOCEK,
			APPLY_ATTBONUS_COL,
			APPLY_ATTBONUS_CZ,
			APPLY_RESIST_DUSUS,
#endif
#ifdef ENABLE_STONE_BOSS_BONUS
			APPLY_ATTBONUS_BOSS,	//116
			APPLY_ATTBONUS_METIN,
#endif
#ifdef ENABLE_CONQUEROR_LEVEL
			APPLY_SUNGMA_STR,//118
			APPLY_SUNGMA_HP,//119
			APPLY_SUNGMA_MOVE,//120
			APPLY_SUNGMA_INMUNE,//121
			APPLY_CONQUEROR_POINT,//122
#endif
#ifdef ENABLE_GLOVE_SYSTEM
			APPLY_RANDOM,//123
#endif
#ifdef ENABLE_ELEMENT_ALL_BONUSES
			APPLY_ELEMENT_ALL,//124
#endif
#ifdef ENABLE_PRECISION
			APPLY_HIT_PCT,//125
#endif
			APPLY_RESIST_MOUNT_FALL,
			APPLY_NORMAL_HIT_DEFEND_BONUS_BOSS_OR_MORE,
			APPLY_SKILL_DEFEND_BONUS_BOSS_OR_MORE,
			APPLY_NORMAL_HIT_DAMAGE_BONUS_BOSS_OR_MORE,
			APPLY_SKILL_DAMAGE_BONUS_BOSS_OR_MORE,
			APPLY_HIT_BUFF_ENCHANT_FIRE,
			APPLY_HIT_BUFF_ENCHANT_ICE,
			APPLY_HIT_BUFF_ENCHANT_ELEC,
			APPLY_HIT_BUFF_ENCHANT_WIND,
			APPLY_HIT_BUFF_ENCHANT_DARK,
			APPLY_HIT_BUFF_ENCHANT_EARTH,
			APPLY_HIT_BUFF_RESIST_FIRE,
			APPLY_HIT_BUFF_RESIST_ICE,
			APPLY_HIT_BUFF_RESIST_ELEC,
			APPLY_HIT_BUFF_RESIST_WIND,
			APPLY_HIT_BUFF_RESIST_DARK,
			APPLY_HIT_BUFF_RESIST_EARTH,
			APPLY_SKILL_DURATION_INCREASE_EUNHYUNG,
			APPLY_SKILL_DURATION_INCREASE_GYEONGGONG,
			APPLY_SKILL_DURATION_INCREASE_GEOMKYUNG,
			APPLY_SKILL_DURATION_INCREASE_JEOKRANG,
			APPLY_USE_SKILL_AMSEOP_HP_ABSORB,
			APPLY_USE_SKILL_PABEOB_STUN,
			APPLY_DAMAGE_HP_RECOVERY,
			APPLY_DAMAGE_SP_RECOVERY,
			APPLY_ALIGNMENT_DAMAGE_BONUS,
			APPLY_NORMAL_DAMAGE_GUARD,
			APPLY_MORE_THEN_HP90_DAMAGE_REDUCE,
			APPLY_ATTBONUS_PER_HUMAN,
			APPLY_ATTBONUS_PER_ANIMAL,
			APPLY_ATTBONUS_PER_ORC,
			APPLY_ATTBONUS_PER_MILGYO,
			APPLY_ATTBONUS_PER_UNDEAD,
			APPLY_ATTBONUS_PER_DEVIL,
			APPLY_ENCHANT_PER_ELECT,
			APPLY_ENCHANT_PER_FIRE,
			APPLY_ENCHANT_PER_ICE,
			APPLY_ENCHANT_PER_WIND,
			APPLY_ENCHANT_PER_EARTH,
			APPLY_ENCHANT_PER_DARK,
			APPLY_ATTBONUS_PER_CZ,
			APPLY_ATTBONUS_PER_INSECT,
			APPLY_ATTBONUS_PER_DESERT,
			APPLY_ATTBONUS_PER_STONE,
			APPLY_ATTBONUS_PER_MONSTER,
			APPLY_RESIST_PER_HUMAN,
			APPLY_RESIST_PER_ICE,
			APPLY_RESIST_PER_DARK,
			APPLY_RESIST_PER_EARTH,
			APPLY_RESIST_PER_FIRE,
			APPLY_RESIST_PER_ELEC,
			APPLY_RESIST_PER_MAGIC,
			APPLY_RESIST_PER_WIND,
			APPLY_MOB_HIT_MOB_AGGRESSIVE,
			APPLY_MOUNT_NO_KNOCKBACK,
			APPLY_SUNGMA_PER_STR,
			APPLY_SUNGMA_PER_HP,
			APPLY_SUNGMA_PER_MOVE,
			APPLY_SUNGMA_PER_IMMUNE,
#ifdef ENABLE_MYSTERY_DUNGEON
			APPLY_MYSTERY,
#endif
#ifdef ENABLE_GREEDY_ROOM
			APPLY_ATTBONUS_MOON,
#endif
			MAX_APPLY_NUM,
		};
		
		enum EImmuneFlags
		{
			IMMUNE_PARA         = (1 << 0),
			IMMUNE_CURSE        = (1 << 1),
			IMMUNE_STUN         = (1 << 2),
			IMMUNE_SLEEP        = (1 << 3),
			IMMUNE_SLOW         = (1 << 4),
			IMMUNE_POISON       = (1 << 5),
			IMMUNE_TERROR       = (1 << 6),
#ifdef ENABLE_INGAME_WIKI
			IMMUNE_FLAG_MAX_NUM = 7,
#endif
		};

#pragma pack(push)
#pragma pack(1)
		typedef struct SItemLimit
		{
			BYTE        bType;
			int32_t     lValue;
		} TItemLimit;

		typedef struct SItemApply
		{
			BYTE        bType;
			int32_t     lValue;
		} TItemApply;

		typedef struct SItemTable
		{
			DWORD       dwVnum;
			DWORD       dwVnumRange;
			char        szName[ITEM_NAME_MAX_LEN + 1];
			char        szLocaleName[ITEM_NAME_MAX_LEN + 1];
			BYTE        bType;
			BYTE        bSubType;
			
			BYTE        bWeight;
			BYTE        bSize;
			
			DWORD       dwAntiFlags;
			DWORD       dwFlags;
			DWORD       dwWearFlags;
			DWORD       dwImmuneFlag;
						
			DWORD       dwIBuyItemPrice;
			DWORD		llISellItemPrice;
			
			TItemLimit  aLimits[ITEM_LIMIT_MAX_NUM];
			TItemApply  aApplies[ITEM_APPLY_MAX_NUM];
			int32_t     alValues[ITEM_VALUES_MAX_NUM];
			int32_t     alSockets[ITEM_SOCKET_MAX_NUM];
			DWORD       dwRefinedVnum;
			WORD		wRefineSet;
			BYTE        bAlterToMagicItemPct;
			BYTE		bSpecular;
			BYTE        bGainSocketPct;
		} TItemTable;

#ifdef ENABLE_SASH_SYSTEM
		struct SScaleInfo
		{
			float	fScaleX, fScaleY, fScaleZ;
			float	fPositionX, fPositionY, fPositionZ;
		};

		typedef struct SScaleTable
		{
			SScaleInfo	tInfo[10];
		} TScaleTable;
#endif

//		typedef struct SItemTable
//		{
//			DWORD       dwVnum;
//			char        szItemName[ITEM_NAME_MAX_LEN + 1];
//			BYTE        bType;
//			BYTE        bSubType;
//			BYTE        bSize;
//			DWORD       dwAntiFlags;
//			DWORD       dwFlags;
//			DWORD       dwWearFlags;
//			DWORD       dwIBuyItemPrice;
//			DWORD		dwISellItemPrice;
//			TItemLimit  aLimits[ITEM_LIMIT_MAX_NUM];
//			TItemApply  aApplies[ITEM_APPLY_MAX_NUM];
//			long        alValues[ITEM_VALUES_MAX_NUM];
//			long        alSockets[ITEM_SOCKET_MAX_NUM];
//			DWORD       dwRefinedVnum;
//			BYTE		bSpecular;
//			DWORD		dwIconNumber;
//		} TItemTable;
#pragma pack(pop)

	public:
		CItemData();
		virtual ~CItemData();

		void Clear();
		void SetSummary(const std::string& c_rstSumm);
		void SetDescription(const std::string& c_rstDesc);

		CGraphicThing * GetModelThing();
		CGraphicThing * GetSubModelThing();
		CGraphicThing * GetDropModelThing();
		CGraphicSubImage * GetIconImage();

		DWORD GetLODModelThingCount();
		BOOL GetLODModelThingPointer(DWORD dwIndex, CGraphicThing ** ppModelThing);

		DWORD GetAttachingDataCount();
		BOOL GetCollisionDataPointer(DWORD dwIndex, const NRaceData::TAttachingData ** c_ppAttachingData);
		BOOL GetAttachingDataPointer(DWORD dwIndex, const NRaceData::TAttachingData ** c_ppAttachingData);

		/////
		const TItemTable*	GetTable() const;
		DWORD GetIndex() const;
		const char * GetName() const;
		const char * GetDescription() const;
		const char * GetSummary() const;
		BYTE GetType() const;
		BYTE GetSubType() const;
		
		UINT GetRefine() const;
		const char* GetUseTypeString() const;
		DWORD GetWeaponType() const;
		BYTE GetSize() const;
		BOOL IsAntiFlag(DWORD dwFlag) const;
		BOOL IsFlag(DWORD dwFlag) const;
		BOOL IsWearableFlag(DWORD dwFlag) const;
		BOOL HasNextGrade() const;
		DWORD GetWearFlags() const;
		DWORD GetIBuyItemPrice() const;
		long long GetISellItemPrice() const;
		BOOL GetLimit(BYTE byIndex, TItemLimit * pItemLimit) const;
		BOOL GetApply(BYTE byIndex, TItemApply * pItemApply) const;
		long GetValue(BYTE byIndex) const;
		long GetSocket(BYTE byIndex) const;
		long SetSocket(BYTE byIndex,DWORD value);
		int GetSocketCount() const;
		DWORD GetIconNumber() const;
#ifdef ENABLE_INGAME_WIKI
		DWORD GetRefinedVnum() const { return m_ItemTable.dwRefinedVnum; }
		WORD	GetRefineSet() const { return m_ItemTable.wRefineSet; }
#endif

		UINT	GetSpecularPoweru() const;
		float	GetSpecularPowerf() const;
	
		/////

		BOOL IsEquipment() const;

		/////

		//BOOL LoadItemData(const char * c_szFileName);
		void SetDefaultItemData(const char * c_szIconFileName, const char * c_szModelFileName  = NULL);
		void SetItemTableData(TItemTable * pItemTable);
	
#ifdef ENABLE_AURA_SYSTEM
	protected:
		DWORD m_dwAuraEffectID;
	public:
		void SetAuraEffectID(const char* szAuraEffectPath);
		DWORD GetAuraEffectID() const { return m_dwAuraEffectID; }
		
		enum EAuraMisc
		{
			AURA_GRADE_MAX_NUM = 6,
		};
#endif
	
#ifdef ENABLE_SASH_SYSTEM
		void SetItemScale(const std::string strJob, const std::string strSex, const std::string strScaleX, const std::string strScaleY, const std::string strScaleZ, const std::string strPositionX, const std::string strPositionY, const std::string strPositionZ);
		bool GetItemScale(DWORD dwPos, float & fScaleX, float & fScaleY, float & fScaleZ, float & fPositionX, float & fPositionY, float & fPositionZ);
#endif

#ifdef ENABLE_INGAME_WIKI
	public:
		typedef struct SWikiItemInfo
		{
			~SWikiItemInfo() = default;
			SWikiItemInfo() {
				isSet = false;
				hasData = false;
				bIsCommon = false;

				dwOrigin = 0;
				maxRefineLevel = CommonWikiData::MAX_REFINE_COUNT;

				pRefineData.clear();
				pChestInfo.clear();
				pOriginInfo.clear();
			}

			bool isSet;
			bool hasData;
			bool bIsCommon;
			DWORD dwOrigin;
			int maxRefineLevel;
			std::vector<CommonWikiData::TWikiRefineInfo> pRefineData;
			std::vector<CommonWikiData::TWikiChestInfo> pChestInfo;
			std::vector<CommonWikiData::TWikiItemOriginInfo> pOriginInfo;
		} TWikiItemInfo;

		bool IsValidImage() { return m_isValidImage; }
		std::string GetIconFileName() { return m_strIconFileName; }
		TWikiItemInfo* GetWikiTable() { return &m_wikiInfo; }
		bool IsBlacklisted() { return m_isBlacklisted; }

		void ValidateImage(bool isValidImage) { m_isValidImage = isValidImage; }
		void SetBlacklisted(bool val) { m_isBlacklisted = val; }

	protected:
		bool m_isValidImage;
		bool m_isBlacklisted;

	private:
		TWikiItemInfo m_wikiInfo;
#endif

	protected:
		void __LoadFiles();
		void __SetIconImage(const char * c_szFileName);

	protected:
		std::string m_strModelFileName;
		std::string m_strSubModelFileName;
		std::string m_strDropModelFileName;
		std::string m_strIconFileName;
		std::string m_strDescription;
		std::string m_strSummary;
		std::vector<std::string> m_strLODModelFileNameVector;

		CGraphicThing * m_pModelThing;
		CGraphicThing * m_pSubModelThing;
		CGraphicThing * m_pDropModelThing;
		CGraphicSubImage * m_pIconImage;
		std::vector<CGraphicThing *> m_pLODModelThingVector;

		NRaceData::TAttachingDataVector m_AttachingDataVector;
		DWORD		m_dwVnum;
		TItemTable m_ItemTable;
#ifdef ENABLE_SASH_SYSTEM
		TScaleTable m_ScaleTable;
#endif

	public:
		static void DestroySystem();

		static CItemData* New();
		static void Delete(CItemData* pkItemData);

		static CDynamicPool<CItemData>		ms_kPool;
};
