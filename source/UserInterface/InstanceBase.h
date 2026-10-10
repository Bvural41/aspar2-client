#pragma once
#include "../GameLib/RaceData.h"
#include "../GameLib/ActorInstance.h"
#ifdef ENABLE_SASH_SYSTEM
#include "../EterLib/GrpObjectInstance.h"
#endif
#include "GameType.h"
#include "AffectFlagContainer.h"

class CInstanceBase
{	
	public:
		struct SCreateData
		{
			BYTE	m_bType;
			DWORD	m_dwStateFlags;
			DWORD	m_dwEmpireID;
			DWORD	m_dwGuildID;
			DWORD	m_dwLevel;
#ifdef ENABLE_CONQUEROR_LEVEL
			DWORD	m_dwConquerorLevel;
#endif
#if defined(ENABLE_SHOW_MOB_INFO)
			DWORD	m_dwAIFlag;
#endif
			DWORD	m_dwVID;
			DWORD	m_dwRace;
			DWORD	m_dwMovSpd;
			DWORD	m_dwAtkSpd;
			LONG	m_lPosX;
			LONG	m_lPosY;
			FLOAT	m_fRot;
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
			DWORD	m_dwMountVnum;

			short	m_sAlignment;
			BYTE	m_byPKMode;
#ifdef ENABLE_BATTLE_FIELD
			BYTE	combat_zone_rank;
			DWORD	combat_zone_points;
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
			CAffectFlagContainer	m_kAffectFlags;

			std::string m_stName;

			bool	m_isMain;
		};

	public:
		typedef DWORD TType;

		enum EMobAIFlags
		{
			AIFLAG_AGGRESSIVE		= (1 <<  0),
			AIFLAG_NOMOVE			= (1 <<  1),
			AIFLAG_COWARD			= (1 <<  2),
			AIFLAG_NOATTACKSHINSU	= (1 <<  3),
			AIFLAG_NOATTACKJINNO	= (1 <<  4),
			AIFLAG_NOATTACKCHUNJO	= (1 <<  5),
			AIFLAG_ATTACKMOB		= (1 <<  6),
			AIFLAG_BERSERK			= (1 <<  7),
			AIFLAG_STONESKIN		= (1 <<  8),
			AIFLAG_GODSPEED			= (1 <<  9),
			AIFLAG_DEATHBLOW		= (1 << 10),
			AIFLAG_REVIVE			= (1 << 11),
#ifdef ENABLE_SUNG_MAHI_TOWER
			AIFLAG_NOATTACK			= (1 << 12),
#endif
#ifdef ENABLE_TREASURE_EVENT
			AIFLAG_NORECOVERY		= (1 << 13),
#endif
		};

		enum EDirection
		{
			DIR_NORTH,
			DIR_NORTHEAST,
			DIR_EAST,
			DIR_SOUTHEAST,
			DIR_SOUTH,
			DIR_SOUTHWEST,
			DIR_WEST,
			DIR_NORTHWEST,
			DIR_MAX_NUM,
		};

		enum
		{
			FUNC_WAIT,
			FUNC_MOVE,
			FUNC_ATTACK,
			FUNC_COMBO,
			FUNC_MOB_SKILL,
			FUNC_EMOTION,
			FUNC_SKILL = 0x80,
		};

		enum
		{
			AFFECT_YMIR,						//0
			AFFECT_INVISIBILITY,				//1
			AFFECT_SPAWN,						//2
			AFFECT_POISON,						//3
			AFFECT_SLOW,						//4
			AFFECT_STUN,						//5
			AFFECT_DUNGEON_READY,				//6
			AFFECT_SHOW_ALWAYS,					//7
			AFFECT_BUILDING_CONSTRUCTION_SMALL,	//8
			AFFECT_BUILDING_CONSTRUCTION_LARGE,	//9
			AFFECT_BUILDING_UPGRADE,			//10
			AFFECT_MOV_SPEED_POTION,			//11
			AFFECT_ATT_SPEED_POTION,			//12
			AFFECT_FISH_MIND,					//13
			AFFECT_JEOKRANG,					//14
			AFFECT_CHEONGRANG,					//15
			AFFECT_JEONGWI,						//16
			AFFECT_GEOMGYEONG,					//17
			AFFECT_CHEONGEUN,					//18
			AFFECT_GYEONGGONG,					//19
			AFFECT_EUNHYEONG,					//20
			AFFECT_GWIGEOM,						//21
			AFFECT_GONGPO,						//22
			AFFECT_JUMAGAP,						//23
			AFFECT_HOSIN,						//24
			AFFECT_BOHO,						//25
			AFFECT_KWAESOK,						//26
			AFFECT_HEUKSIN,						//27
			AFFECT_MUYEONG,						//28
			AFFECT_REVIVE_INVISIBILITY,			//29
			AFFECT_FIRE,						//30
			AFFECT_GICHEON,						//31
			AFFECT_JEUNGRYEOK,					//32
			AFFECT_DASH,						//33
			AFFECT_PABEOP,						//34
			AFFECT_FALLEN_CHEONGEUN,			//35
			AFFECT_POLYMORPH,					//36
			AFFECT_WAR_FLAG1,					//37
			AFFECT_WAR_FLAG2,					//38
			AFFECT_WAR_FLAG3,					//39
			AFFECT_CHINA_FIREWORK,				//40
			AFFECT_HAIR,						//41
			AFFECT_GERMANY,						//42
			AFFECT_BLEEDING,					//43
#ifdef ENABLE_MELEY_LAIR_DUNGEON
			AFFECT_STATUE1,						//44
			AFFECT_STATUE2,						//45
			AFFECT_STATUE3,						//46
			AFFECT_STATUE4,						//47
#endif
#ifdef ENABLE_MAX_RED_BUFF_EFFECT
			AFFECT_RED_BUFF,					//48
#endif
#ifdef ENABLE_SOUL_SYSTEM
			AFFECT_SOUL_RED,					//49
			AFFECT_SOUL_BLUE,					//50
			AFFECT_SOUL_MIX,					//51
#endif
#ifdef ENABLE_CONQUEROR_LEVEL
			AFFECT_CHEONUN,						//52
			AFFECT_CHUNWOON,					//53
#endif
#ifdef ENABLE_BATTLE_ROYALE
			AFFECT_BATTLE_ROYALE_CROWN,			//54
			AFFECT_BATTLE_ROYALE_SLOW_1,		//55
			AFFECT_BATTLE_ROYALE_SLOW_2,		//56
			AFFECT_BATTLE_ROYALE_SLOW_3,		//57
#endif
#ifdef ENABLE_MOONLIGHT_VALLEY
			AFFECT_MOONLIGHT_TORNADO,			//58
#endif
#ifdef ENABLE_GREEDY_ROOM
			AFFECT_TORNADO,						//59
			AFFECT_AAMON_DEFENCE,				//60
			AFFECT_FIRE_WAVE,					//61
			AFFECT_SPEAR_WARNING,				//62
			AFFECT_SPEAR_FIRE,					//63
			AFFECT_AAMON_LASTSTAND,				//64
			AFFECT_AAMON_LASTSTAND_PC,			//65
#endif
			AFFECT_NUM = 66,					//Max 64
		};

		enum
		{
			NEW_AFFECT_MOV_SPEED					= 200,
			NEW_AFFECT_ATT_SPEED,
			NEW_AFFECT_ATT_GRADE,
			NEW_AFFECT_INVISIBILITY,
			NEW_AFFECT_STR,
			NEW_AFFECT_DEX,							// 205
			NEW_AFFECT_CON,
			NEW_AFFECT_INT,
			NEW_AFFECT_FISH_MIND_PILL,

			NEW_AFFECT_POISON,
			NEW_AFFECT_STUN,						// 210
			NEW_AFFECT_SLOW,
			NEW_AFFECT_DUNGEON_READY,
			NEW_AFFECT_DUNGEON_UNIQUE,

			NEW_AFFECT_BUILDING,
			NEW_AFFECT_REVIVE_INVISIBLE,			// 215
			NEW_AFFECT_FIRE,
			NEW_AFFECT_CAST_SPEED,
			NEW_AFFECT_HP_RECOVER_CONTINUE,
			NEW_AFFECT_SP_RECOVER_CONTINUE, 

			NEW_AFFECT_POLYMORPH,					// 220
			NEW_AFFECT_MOUNT,

			NEW_AFFECT_WAR_FLAG,					// 222

			NEW_AFFECT_BLOCK_CHAT,					// 223
			NEW_AFFECT_CHINA_FIREWORK,

			NEW_AFFECT_BOW_DISTANCE,				// 225

			NEW_AFFECT_EXP_BONUS					= 500,
			NEW_AFFECT_ITEM_BONUS					= 501,
			NEW_AFFECT_SAFEBOX						= 502,
			NEW_AFFECT_AUTOLOOT						= 503,
			NEW_AFFECT_FISH_MIND					= 504,
			NEW_AFFECT_MARRIAGE_FAST				= 505,
			NEW_AFFECT_GOLD_BONUS					= 506,

			NEW_AFFECT_MALL							= 510,
			NEW_AFFECT_NO_DEATH_PENALTY				= 511,
			NEW_AFFECT_SKILL_BOOK_BONUS				= 512,
			NEW_AFFECT_SKILL_BOOK_NO_DELAY			= 513,

			NEW_AFFECT_EXP_BONUS_EURO_FREE			= 516,
			NEW_AFFECT_EXP_BONUS_EURO_FREE_UNDER_15	= 517,

			NEW_AFFECT_AUTO_HP_RECOVERY				= 534,
			NEW_AFFECT_AUTO_SP_RECOVERY				= 535,

			NEW_AFFECT_DRAGON_SOUL_QUALIFIED		= 540, 
			NEW_AFFECT_DRAGON_SOUL_DECK1			= 541,
			NEW_AFFECT_DRAGON_SOUL_DECK2			= 542,
#ifdef ENABLE_DS_SET
			NEW_AFFECT_DS_SET						= 543,
#endif
			NEW_AFFECT_RAMADAN_ABILITY				= 300,
			NEW_AFFECT_RAMADAN_RING					= 301,

			NEW_AFFECT_NOG_POCKET_ABILITY			= 302,
#ifdef ENABLE_MAX_RED_BUFF_EFFECT
			NEW_AFFECT_BUFF							= 316,
#endif
#ifdef ENABLE_NEW_AFFECT_POTION
			AFFECT_POTION_BLEND_1					= 613,
			AFFECT_POTION_BLEND_2					= 614,
			AFFECT_POTION_BLEND_3					= 615,
			AFFECT_POTION_BLEND_4					= 616,
			AFFECT_POTION_BLEND_5					= 617,
			AFFECT_POTION_BLEND_6					= 618,
			AFFECT_POTION_BLEND_7					= 619,

			AFFECT_POTION_BLEND_8					= 640,
			AFFECT_POTION_BLEND_9					= 641,
			AFFECT_POTION_BLEND_10					= 642,
			AFFECT_POTION_BLEND_11					= 643,
			AFFECT_POTION_BLEND_12					= 644,
			AFFECT_POTION_BLEND_13					= 645,
			AFFECT_POTION_BLEND_14					= 646,
			AFFECT_POTION_BLEND_15					= 647,
			AFFECT_POTION_BLEND_16					= 648,
			AFFECT_POTION_BLEND_17					= 649,
			AFFECT_POTION_BLEND_18					= 650,
			AFFECT_POTION_BLEND_19					= 651,
			AFFECT_POTION_BLEND_20					= 652,
			AFFECT_POTION_BLEND_21					= 653,
			AFFECT_POTION_BLEND_22					= 654,
			AFFECT_POTION_BLEND_23					= 655,
			AFFECT_POTION_BLEND_24					= 656,
			AFFECT_POTION_BLEND_25					= 657,
			AFFECT_POTION_BLEND_26					= 658,
			AFFECT_POTION_BLEND_27					= 659,
			AFFECT_POTION_BLEND_28					= 660,
			AFFECT_POTION_BLEND_29					= 661,
			AFFECT_POTION_BLEND_30					= 662,
			AFFECT_POTION_BLEND_31					= 663,
			AFFECT_POTION_BLEND_32					= 664,

			AFFECT_POTION_1							= 625,
			AFFECT_POTION_2							= 626,
			AFFECT_POTION_3							= 627,
			AFFECT_POTION_4							= 628,
			AFFECT_POTION_5							= 629,
			AFFECT_POTION_6							= 630,
			AFFECT_POTION_7							= 631,
			AFFECT_POTION_8							= 632,
			AFFECT_POTION_9							= 633,
#endif
#ifdef ENABLE_SUNGMA_PREMIUM_BUFF
			AFFECT_SUNGMA_PREMIUM_BUFF				= 634,
#endif
#ifdef ENABLE_ATTRACT_RANGER_SYSYTEM
			AFFECT_POTION_10						= 635,
#endif
#ifdef ENABLE_MELEY_LAIR_DUNGEON
			AFFECT_STATUE							= 700,
#endif
#ifdef ENABLE_SOUL_SYSTEM
			NEW_AFFECT_SOUL_RED						= 720,
			NEW_AFFECT_SOUL_BLUE					= 721,
			NEW_AFFECT_SOUL_MIX						= 723,
#endif
#ifdef ENABLE_MOONLIGHT_VALLEY
			NEW_AFFECT_MOONLIGHT_TORNADO			= 724,
#endif
#ifdef ENABLE_GREEDY_ROOM
			NEW_AFFECT_TORNADO						= 725,
			NEW_AFFECT_AAMON_DEFENCE				= 726,
			NEW_AFFECT_FIRE_WAVE					= 727,
			NEW_AFFECT_SPEAR_WARNING				= 728,
			NEW_AFFECT_SPEAR_FIRE					= 729,
			NEW_AFFECT_AAMON_LASTSTAND				= 730,
			NEW_AFFECT_AAMON_LASTSTAND_PC			= 731,
#endif
			NEW_AFFECT_BLEEDING						= 304,
#ifdef ENABLE_OFFLINESHOP_SYSTEM
			NEW_AFFECT_DECORATION					= 703,
#endif
#ifdef ENABLE_ELEMENTAL_WORLD
			NEW_AFFECT_PROTECTION_OF_ELEMENTAL		= 595,
#endif
#ifdef ENABLE_AUTO_SYSTEM
			NEW_AFFECT_AUTO							= 814,
#endif
#ifdef ENABLE_CONQUEROR_LEVEL
			AFFECT_SUNGMA_STR						= 904,
			AFFECT_SUNGMA_VIT						= 905,
			AFFECT_SUNGMA_RES						= 906,
			AFFECT_SUNGMA_INT						= 907,
#endif
#ifdef ENABLE_SET_ITEM
			NEW_AFFECT_SET_ITEM						= 550,
			NEW_AFFECT_SET_ITEM_SET_VALUE_1			= 601,
			NEW_AFFECT_SET_ITEM_SET_VALUE_2			= 602,
			NEW_AFFECT_SET_ITEM_SET_VALUE_3			= 603,
			NEW_AFFECT_SET_ITEM_SET_VALUE_4			= 604,
			NEW_AFFECT_SET_ITEM_SET_VALUE_5			= 605,
#endif
#ifdef ENABLE_BATTLE_ROYALE
			NEW_AFFECT_BATTLE_ROYALE_CROWN			= 688,
			NEW_AFFECT_BATTLE_ROYALE_FIELD_DAMAGE	= 689,
			NEW_AFFECT_BATTLE_ROYALE_SLOW			= 690,
			NEW_AFFECT_BATTLE_ROYALE_MOVE_SPEED		= 691,
			NEW_AFFECT_BATTLE_ROYALE_INFINITE_STAMINA	= 692,
#endif
#ifdef ENABLE_SUNG_MAHI_TOWER
			NEW_AFFECT_SUNG_MAHI_BP					= 701,
			NEW_AFFECT_SUNG_MAHI_CURSE				= 702,
#endif
#if defined(ENABLE_DEFENSE_WAVE)
			NEW_AFFECT_DEFENSEWAVE_LASER			= 573,
#endif
#ifdef ENABLE_COSTUME_SET_ITEM
			NEW_AFFECT_COSTUME_SET_ITEM				= 574,
#endif
#ifdef ENABLE_MULTI_FARM_BLOCK
			NEW_AFFECT_MULTI_FARM					= 704,
#endif
#ifdef ENABLE_FLOWER_EVENT
			AFFECT_FLOWER_EVENT						= 570,
#endif
#ifdef ENABLE_TREASURE_EVENT
			NEW_AFFECT_TREASURE_EVENT_AFFECT_1		= 668,
			NEW_AFFECT_TREASURE_EVENT_AFFECT_2		= 669,
#endif
#ifdef ENABLE_RIDING_EXTENDED
			NEW_AFFECT_SKILL_MOUNT_UPGRADE_NIMBLE	= 739,
			NEW_AFFECT_SKILL_MOUNT_UPGRADE_EXP		= 740,
			NEW_AFFECT_SKILL_MOUNT_UPGRADE_SPEED	= 741,
			NEW_AFFECT_SKILL_MOUNT_UPGRADE_GYENONGGONG	= 742,
			NEW_AFFECT_SKILL_MOUNT_UPGRADE_INVINCIBLITIY	= 743,
			NEW_AFFECT_SKILL_MOUNT_UPGRADE_KNOCKBACK	= 744,
			NEW_AFFECT_SKILL_MOUNT_UPGRADE_TO_STRONG	= 745,
			NEW_AFFECT_SKILL_MOUNT_UPGRADE_EFFECT_UP	= 746,
			NEW_AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_STR	= 747,
			NEW_AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_HP	= 748,
			NEW_AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_MOVE	= 749,
			NEW_AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_IMMUNE	= 750,
			NEW_AFFECT_SKILL_MOUNT_UPGRADE_HIT_PCT	= 751,
#endif
#ifdef ENABLE_VOTE4BUFF
			NEW_AFFECT_VOTE4BUFF					= 999,
#endif

			NEW_AFFECT_QUEST_START_IDX				= 1000,
		};

		enum
		{
			STONE_SMOKE1 = 0,	// 99%
			STONE_SMOKE2 = 1,	// 85%
			STONE_SMOKE3 = 2,	// 80%
			STONE_SMOKE4 = 3,	// 60%
			STONE_SMOKE5 = 4,	// 45%
			STONE_SMOKE6 = 5,	// 40%
			STONE_SMOKE7 = 6,	// 20%
			STONE_SMOKE8 = 7,	// 10%
			STONE_SMOKE_NUM = 4,
		};

		enum EBuildingAffect
		{
			BUILDING_CONSTRUCTION_SMALL = 0,
			BUILDING_CONSTRUCTION_LARGE = 1,
			BUILDING_UPGRADE = 2,
		};

		enum
		{
			WEAPON_DUALHAND,
			WEAPON_ONEHAND,
			WEAPON_TWOHAND,
			WEAPON_NUM,
		};

		enum
		{
			EMPIRE_NONE,
			EMPIRE_A,
			EMPIRE_B,
			EMPIRE_C,
			EMPIRE_NUM,
		};

		enum
		{
			NAMECOLOR_MOB,
			NAMECOLOR_NPC,
			NAMECOLOR_PC,
			NAMECOLOR_PC_END = NAMECOLOR_PC + EMPIRE_NUM,
			NAMECOLOR_NORMAL_MOB,
			NAMECOLOR_NORMAL_NPC,
			NAMECOLOR_NORMAL_PC,
			NAMECOLOR_NORMAL_PC_END = NAMECOLOR_NORMAL_PC + EMPIRE_NUM,
			NAMECOLOR_EMPIRE_MOB,
			NAMECOLOR_EMPIRE_NPC,
			NAMECOLOR_EMPIRE_PC,
			NAMECOLOR_EMPIRE_PC_END = NAMECOLOR_EMPIRE_PC + EMPIRE_NUM,
			NAMECOLOR_FUNC,
			NAMECOLOR_PK,
			NAMECOLOR_PVP,
			NAMECOLOR_PARTY,
			NAMECOLOR_WARP,
			NAMECOLOR_WAYPOINT,
			NAMECOLOR_METIN,
#ifdef ENABLE_SUPPORT_SYSTEM
			NAMECOLOR_SUPPORT,
#endif
#ifdef ENABLE_CONQUEROR_LEVEL
			NAMECOLOR_CONQUEROR,
#endif
#ifdef ENABLE_OFFLINESHOP_SYSTEM
			NAMECOLOR_SHOP,
#endif
			NAMECOLOR_EXTRA = NAMECOLOR_FUNC + 10,
			NAMECOLOR_NUM = NAMECOLOR_EXTRA + 10,
		};
				
		enum
		{
			ALIGNMENT_TYPE_WHITE,
			ALIGNMENT_TYPE_NORMAL,
			ALIGNMENT_TYPE_DARK,
		};

		enum
		{
			EMOTICON_EXCLAMATION	= 1,
			EMOTICON_FISH			= 11,
			EMOTICON_NUM			= 128,

			TITLE_NUM 				= 9,
			TITLE_NONE 				= 4,
		};

		enum
		{
			EFFECT_REFINED_NONE,

			EFFECT_SWORD_REFINED7,
			EFFECT_SWORD_REFINED8,
			EFFECT_SWORD_REFINED9,

			EFFECT_BOW_REFINED7,
			EFFECT_BOW_REFINED8,
			EFFECT_BOW_REFINED9,

			EFFECT_FANBELL_REFINED7,
			EFFECT_FANBELL_REFINED8,
			EFFECT_FANBELL_REFINED9,

			EFFECT_SMALLSWORD_REFINED7,
			EFFECT_SMALLSWORD_REFINED8,
			EFFECT_SMALLSWORD_REFINED9,

			EFFECT_SMALLSWORD_REFINED7_LEFT,
			EFFECT_SMALLSWORD_REFINED8_LEFT,
			EFFECT_SMALLSWORD_REFINED9_LEFT,

			EFFECT_BODYARMOR_REFINED7,
			EFFECT_BODYARMOR_REFINED8,
			EFFECT_BODYARMOR_REFINED9,

			EFFECT_CLAW_REFINED7,
			EFFECT_CLAW_REFINED8,
			EFFECT_CLAW_REFINED9,

			EFFECT_CLAW_REFINED7_LEFT,
			EFFECT_CLAW_REFINED8_LEFT,
			EFFECT_CLAW_REFINED9_LEFT,

			EFFECT_BODYARMOR_SPECIAL,
			EFFECT_BODYARMOR_SPECIAL2,

#ifdef VERSION_162_ENABLED
			EFFECT_BODYARMOR_SPECIAL3,
#endif

			EFFECT_BODYARMOR_SPECIAL4,

#ifdef ENABLE_SASH_SYSTEM
			EFFECT_SASH,
#endif

#ifdef ENABLE_OFFICIAL_19_3_ITEMS
			EFFECT_BODYARMOR_SPECIAL_19_3,
			EFFECT_WEAPON_SWORD_SPECIAL_19_3,
			EFFECT_WEAPON_DAGGER_SPECIAL_19_3,
			EFFECT_WEAPON_BOW_SPECIAL_19_3,
			EFFECT_WEAPON_TWO_HANDED_SPECIAL_19_3,
			EFFECT_WEAPON_BELL_SPECIAL_19_3,
			EFFECT_WEAPON_FAN_SPECIAL_19_3,
			EFFECT_WEAPON_CLAW_SPECIAL_19_3,
			EFFECT_WEAPON_DAGGER_SPECIAL_19_3_LEFT,
			EFFECT_WEAPON_CLAW_SPECIAL_19_3_LEFT,
#endif

#ifdef ENABLE_OFFLINESHOP_SYSTEM
			EFFECT_MYOFFSHOP,
#endif

#ifdef ENABLE_GREEDY_ROOM
			EFFECT_WEAPON_MOONLIGHT_SWORD = 45,
			EFFECT_WEAPON_MOONLIGHT_DAGGER_RIGHT,
			EFFECT_WEAPON_MOONLIGHT_DAGGER_LEFT,
			EFFECT_WEAPON_MOONLIGHT_BOW,
			EFFECT_WEAPON_MOONLIGHT_TWO_HANDED,
			EFFECT_WEAPON_MOONLIGHT_BELL,
			EFFECT_WEAPON_MOONLIGHT_FAN,
			EFFECT_WEAPON_MOONLIGHT_CLAW_RIGHT,
			EFFECT_WEAPON_MOONLIGHT_CLAW_LEFT,
#endif
			EFFECT_REFINED_NUM,
		};
		
		enum DamageFlag
		{
			DAMAGE_NORMAL	= (1<<0),
			DAMAGE_POISON	= (1<<1),
			DAMAGE_DODGE	= (1<<2),
			DAMAGE_BLOCK	= (1<<3),
			DAMAGE_PENETRATE= (1<<4),
			DAMAGE_CRITICAL = (1<<5),
		};

#ifdef ENABLE_REFINE_ELEMENT
		enum ERefineElementWeapon
		{
			EFFECT_REFINE_ELEMENT_WEAPON_SWORD,
			EFFECT_REFINE_ELEMENT_WEAPON_BOW,
			EFFECT_REFINE_ELEMENT_WEAPON_FAN,
			EFFECT_REFINE_ELEMENT_WEAPON_DAGGER,
			EFFECT_REFINE_ELEMENT_WEAPON_DAGGER_LEFT,
#ifdef ENABLE_WOLFMAN_CHARACTER
			EFFECT_REFINE_ELEMENT_WEAPON_CLAW,
			EFFECT_REFINE_ELEMENT_WEAPON_CLAW_LEFT,
#endif
			EFFECT_REFINE_ELEMENT_WEAPON_MAX,
		};

		enum ERefineElementType
		{
			REFINE_ELEMENT_CATEGORY_ELECT,
			REFINE_ELEMENT_CATEGORY_FIRE,
			REFINE_ELEMENT_CATEGORY_ICE,
			REFINE_ELEMENT_CATEGORY_WIND,
			REFINE_ELEMENT_CATEGORY_EARTH,
			REFINE_ELEMENT_CATEGORY_DARK,
			REFINE_ELEMENT_CATEGORY_MAX,
		};
#endif

		enum
		{
			EFFECT_DUST,
			EFFECT_STUN,
			EFFECT_HIT,
			EFFECT_FLAME_ATTACK,
			EFFECT_FLAME_HIT,
			EFFECT_FLAME_ATTACH,
			EFFECT_ELECTRIC_ATTACK,
			EFFECT_ELECTRIC_HIT,
			EFFECT_ELECTRIC_ATTACH,
			EFFECT_SPAWN_APPEAR,
			EFFECT_SPAWN_DISAPPEAR,
			EFFECT_LEVELUP,
			EFFECT_SKILLUP,
			EFFECT_HPUP_RED,
			EFFECT_SPUP_BLUE,
			EFFECT_SPEEDUP_GREEN,
			EFFECT_DXUP_PURPLE,
			EFFECT_CRITICAL,
			EFFECT_PENETRATE,
			EFFECT_BLOCK,
			EFFECT_DODGE,
			EFFECT_FIRECRACKER,
			EFFECT_SPIN_TOP,
			EFFECT_WEAPON,
			EFFECT_WEAPON_END = EFFECT_WEAPON + WEAPON_NUM,
			EFFECT_AFFECT,
			EFFECT_AFFECT_GYEONGGONG = EFFECT_AFFECT + AFFECT_GYEONGGONG,
			EFFECT_AFFECT_KWAESOK = EFFECT_AFFECT + AFFECT_KWAESOK,
			EFFECT_AFFECT_END = EFFECT_AFFECT + AFFECT_NUM,
			EFFECT_EMOTICON,
			EFFECT_EMOTICON_END = EFFECT_EMOTICON + EMOTICON_NUM,
			EFFECT_SELECT,
			EFFECT_TARGET,
			EFFECT_EMPIRE,
			EFFECT_EMPIRE_END = EFFECT_EMPIRE + EMPIRE_NUM,
			EFFECT_HORSE_DUST,
			EFFECT_REFINED,
			EFFECT_REFINED_END = EFFECT_REFINED + EFFECT_REFINED_NUM,
			EFFECT_DAMAGE_TARGET,
			EFFECT_DAMAGE_NOT_TARGET,
			EFFECT_DAMAGE_SELFDAMAGE,
			EFFECT_DAMAGE_SELFDAMAGE2,
			EFFECT_DAMAGE_POISON,
			EFFECT_DAMAGE_MISS,
			EFFECT_DAMAGE_TARGETMISS,
			EFFECT_DAMAGE_CRITICAL,
			EFFECT_SUCCESS,
			EFFECT_FAIL,
			EFFECT_FR_SUCCESS,
			EFFECT_LEVELUP_ON_14_FOR_GERMANY,
			EFFECT_LEVELUP_UNDER_15_FOR_GERMANY,
			EFFECT_PERCENT_DAMAGE1,
			EFFECT_PERCENT_DAMAGE2,
			EFFECT_PERCENT_DAMAGE3,
			EFFECT_AUTO_HPUP,
			EFFECT_AUTO_SPUP,
			EFFECT_RAMADAN_RING_EQUIP,
			EFFECT_HALLOWEEN_CANDY_EQUIP,
			EFFECT_HAPPINESS_RING_EQUIP,
			EFFECT_LOVE_PENDANT_EQUIP,
			EFFECT_TEMP,

#ifdef ENABLE_BATTLE_FIELD
			EFFECT_COMBAT_ZONE_POTION,
#endif

#ifdef ENABLE_SASH_SYSTEM
			EFFECT_SASH_SUCCEDED,
			EFFECT_SASH_EQUIP,
#endif

#ifdef VERSION_162_ENABLED
			EFFECT_HEALER,
#endif

#ifdef ENABLE_12ZI
			EFFECT_SKILL_DAMAGE_ZONE,
			EFFECT_SKILL_DAMAGE_ZONE_BUYUK,
			EFFECT_SKILL_DAMAGE_ZONE_ORTA,
			EFFECT_SKILL_DAMAGE_ZONE_KUCUK,
			EFFECT_SKILL_SAFE_ZONE,
			EFFECT_SKILL_SAFE_ZONE_BUYUK,
			EFFECT_SKILL_SAFE_ZONE_ORTA,
			EFFECT_SKILL_SAFE_ZONE_KUCUK,
			EFFECT_METEOR,
			EFFECT_BEAD_RAIN,
			EFFECT_FALL_ROCK,
			EFFECT_ARROW_RAIN,
			EFFECT_HORSE_DROP,
			EFFECT_EGG_DROP,
			EFFECT_DEAPO_BOOM,
#endif
#ifdef ENABLE_NEW_GYEONGGONG_SKILL
			EFFECT_GYEONGGONG_BOOM,
#endif
#ifdef ENABLE_AGGREGATE_MONSTER_EFFECT
			EFFECT_AGGREGATE_MONSTER,
#endif
#ifdef ENABLE_BOSS_EFFECT_SYSTEM
			EFFECT_BOSS,
#endif
#ifdef ENABLE_PASSIVE_SYSTEM
			EFFECT_PASSIVE,
#endif
#ifdef ENABLE_EQUIPMENT_AFFECT
			EFFECT_EQUIPMENT_1,
			EFFECT_EQUIPMENT_2,
			EFFECT_EQUIPMENT_3,
			EFFECT_EQUIPMENT_4,
			EFFECT_EQUIPMENT_5,
			EFFECT_EQUIPMENT_6,
#endif
#ifdef ENABLE_REFINE_ELEMENT
			EFFECT_REFINE_ELEMENT,
			EFFECT_REFINE_ELEMENT_END = EFFECT_REFINE_ELEMENT + (EFFECT_REFINE_ELEMENT_WEAPON_MAX * REFINE_ELEMENT_CATEGORY_MAX),
#endif
#ifdef ENABLE_QUEEN_NETHIS
			EFFECT_SNAKE_REGEN,
#endif
#ifdef ENABLE_AUTO_QUQUE_ATTACK
			EFFECT_QUQUE_ATTACK,
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
			EFFECT_PET_SKILL_RESTAURATION,
			EFFECT_PET_SKILL_IMMORTAL,
			EFFECT_PET_SKILL_PANACEA,
			EFFECT_PET_SKILL_FEATHERLIGHT,
			EFFECT_PET_CHANGE_ATTR,
#endif
#if defined(ENABLE_DEFENSE_WAVE)
			EFFECT_DEFENSE_WAVE_LASER,
#endif
#ifdef ENABLE_FLOWER_EVENT
			EFFECT_FLOWER_EVENT,
#endif
#ifdef ENABLE_TITLE_SYSTEM
			EFFECT_TITLE_01,
			EFFECT_TITLE_02,
			EFFECT_TITLE_03,
			EFFECT_TITLE_04,
			EFFECT_TITLE_05,
			EFFECT_TITLE_06,
			EFFECT_TITLE_07,
			EFFECT_TITLE_08,
#endif
			EFFECT_NUM,
		};

#ifdef ENABLE_MELEY_LAIR_DUNGEON
		enum
		{
			EFFECT_DRAGONLAIR_STONE_UNBEATABLE_1 = EFFECT_AFFECT + AFFECT_STATUE1,
			EFFECT_DRAGONLAIR_STONE_UNBEATABLE_2 = EFFECT_AFFECT + AFFECT_STATUE2,
			EFFECT_DRAGONLAIR_STONE_UNBEATABLE_3 = EFFECT_AFFECT + AFFECT_STATUE3,
			EFFECT_DRAGONLAIR_STONE_UNBEATABLE_4 = EFFECT_AFFECT + AFFECT_STATUE4,
		};
#endif

		enum
		{
			DUEL_NONE,
			DUEL_CANNOTATTACK,
			DUEL_START,
		};

	public:
		static void DestroySystem();
		static void CreateSystem(UINT uCapacity);
		static bool RegisterEffect(UINT eEftType, const char* c_szEftAttachBone, const char* c_szEftName, bool isCache);
		static void RegisterTitleName(int iIndex, const char * c_szTitleName);
		static bool RegisterNameColor(UINT uIndex, UINT r, UINT g, UINT b);
		static bool RegisterTitleColor(UINT uIndex, UINT r, UINT g, UINT b);
		static bool ChangeEffectTexture(UINT eEftType, const char* c_szSrcFileName, const char* c_szDstFileName);

		static void SetDustGap(float fDustGap);
		static void SetHorseDustGap(float fDustGap);

		static void SetEmpireNameMode(bool isEnable);
		static const D3DXCOLOR& GetIndexedNameColor(UINT eNameColor);

#ifdef ENABLE_LEFT_SEAT
		static void RegisterLeftSeatText(const char* c_szLeftSeatText);
#endif

	public:
		void SetMainInstance();

		void OnSelected();
		void OnUnselected();
		void OnTargeted();
		void OnUntargeted();
#ifdef ENABLE_AUTO_QUQUE_ATTACK
		void SetAutoFarmAffect(const bool bStatus);
#endif
#ifdef ENABLE_AUTO_SYSTEM
		void				SetAutoTarget(bool bFlag) { m_isAutoTarget = bFlag; RefreshTextTail(); }
		bool				IsAutoTarget() const { return m_isAutoTarget; }
#endif

	protected:
		bool __IsExistMainInstance();
		bool __IsMainInstance();
		bool __MainCanSeeHiddenThing();
		float __GetBowRange();

	protected:
		DWORD	__AttachEffect(UINT eEftType);
		DWORD	__AttachEffect(char filename[128]);
		void	__DetachEffect(DWORD dwEID);

#ifdef ENABLE_TITLE_SYSTEM
		DWORD	__AttachTitleEffect(UINT eEftType, float fScale = 1.0f);
#endif

	public:
		void CreateSpecialEffect(DWORD iEffectIndex);
		void AttachSpecialEffect(DWORD effect);

#ifdef ENABLE_TITLE_SYSTEM
		void AttachTitleEffectByPath(const char* szEffectPath);
		void DetachTitleEffect();
#endif

#ifdef ENABLE_12ZI
		void AttachSpecialZodiacEffect(UINT eEftType, long GetX, long GetY);
#endif

	protected:
		static std::string ms_astAffectEffectAttachBone[EFFECT_NUM];
		static DWORD ms_adwCRCAffectEffect[EFFECT_NUM];
		static float ms_fDustGap;
		static float ms_fHorseDustGap;

	public:
		CInstanceBase();
		virtual ~CInstanceBase();

		bool LessRenderOrder(CInstanceBase* pkInst);

		void MountHorse(UINT eRace);
		void DismountHorse();

		void SCRIPT_SetAffect(UINT eAffect, bool isVisible); 

		float CalculateDistanceSq3d(const TPixelPosition& c_rkPPosDst);

		bool IsFlyTargetObject();
		void ClearFlyTargetInstance();
		void SetFlyTargetInstance(CInstanceBase& rkInstDst);
		void AddFlyTargetInstance(CInstanceBase& rkInstDst);
		void AddFlyTargetPosition(const TPixelPosition& c_rkPPosDst);

		float GetFlyTargetDistance();

		void SetAlpha(float fAlpha);

		void DeleteBlendOut();

		void					AttachTextTail();
		void					DetachTextTail();
		void					UpdateTextTailLevel(DWORD level);
#ifdef ENABLE_CONQUEROR_LEVEL
		void					UpdateTextTailConquerorLevel(DWORD level);
#endif
		void					RefreshTextTail();
		void					RefreshTextTailTitle();

#ifdef ENABLE_INGAME_WIKI
		bool					Create(const SCreateData& c_rkCreateData, bool wikiPreview = false);
#else
		bool					Create(const SCreateData& c_rkCreateData);
#endif

		bool					CreateDeviceObjects();
		void					DestroyDeviceObjects();

		void					Destroy();

		void					Update();
		bool					UpdateDeleting();

		void					Transform();
		void					Deform();
		void					Render();
		void					RenderTrace();
		void					RenderToShadowMap();
		void					RenderCollision();
		void					RegisterBoundingSphere();

		void					GetBoundBox(D3DXVECTOR3 * vtMin, D3DXVECTOR3 * vtMax);

		void					SetNameString(const char* c_szName, int len);
		bool					SetRace(DWORD dwRaceIndex);

		void					SetVirtualID(DWORD wVirtualNumber);
		void					SetVirtualNumber(DWORD dwVirtualNumber);
		void					SetInstanceType(int iInstanceType);
		void					SetAlignment(short sAlignment);
		void					SetLevelText(int mLevel);
#ifdef ENABLE_TEXT_LEVEL_REFRESH
		void					SetLevel(DWORD dwLevel);
#endif
#ifdef ENABLE_CONQUEROR_LEVEL
		void					SetConquerorLevelText(int iLevel);
#endif
#ifdef ENABLE_LEFT_SEAT
		void					UpdateTextTailLeftSeat(const std::string& c_rstrText);
#endif
		void					SetPKMode(BYTE byPKMode);
		void					SetKiller(bool bFlag);
		void					SetPartyMemberFlag(bool bFlag);
		void					SetStateFlags(DWORD dwStateFlags);

#ifdef ENABLE_LEFT_SEAT
		void					SetLeftSeat(bool bLeftSeat);
		bool					GetLeftSeat() const { return m_bLeftSeat; }
#endif


		void					SetArmor(DWORD dwArmor);
		void					SetShape(DWORD eShape, float fSpecular=0.0f);
		void					SetHair(DWORD eHair);

#ifdef ENABLE_SASH_SYSTEM
		void					SetSash(DWORD dwSash);
		void					ChangeSash(DWORD dwSash);
#endif

#ifdef ENABLE_AURA_SYSTEM
		bool					SetAura(DWORD eAura);
		void					ChangeAura(DWORD eAura);
#endif

#ifdef ENABLE_REFINE_ELEMENT
		void					SetRefineElementType(BYTE bRefineElementType) { m_bRefineElementType = bRefineElementType; }
		BYTE					GetRefineElementType() const { return m_bRefineElementType; }
#endif

#ifdef ENABLE_NEW_ARROW_SYSTEM
		bool					SetWeapon(DWORD eWeapon, DWORD eArrow = 0);
#else
		bool					SetWeapon(DWORD eWeapon);
#endif

		void					SetCostumEvo(DWORD dwCostumEvo);

		bool					ChangeArmor(DWORD dwArmor);

#ifdef ENABLE_NEW_ARROW_SYSTEM
		void					ChangeWeapon(DWORD eWeapon, DWORD eArrow = 0);
#else
		void					ChangeWeapon(DWORD eWeapon);
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		void					SetLanguage(BYTE bLang) { m_bLanguage = bLang; }
		BYTE					GetLanguage() const { return m_bLanguage; }
#endif

		void					ChangeHair(DWORD eHair);
		void					ChangeGuild(DWORD dwGuildID);
		DWORD					GetWeaponType();

		void					SetComboType(UINT uComboType);
		void					SetAttackSpeed(UINT uAtkSpd);
		void					SetMoveSpeed(UINT uMovSpd);
		void					SetRotationSpeed(float fRotSpd);

		const char *			GetNameString();

#ifdef ENABLE_BATTLE_FIELD
		bool					IsCombatZoneMap();
		void					SetCombatZonePoints(DWORD dwValue);
		DWORD					GetCombatZonePoints();
		void					SetCombatZoneRank(BYTE bValue);
		BYTE					GetCombatZoneRank();
#endif

#ifdef ENABLE_SUPPORT_SYSTEM
		void					SetSupportShaman(bool bTrue);
		bool					IsSupportShaman();
#endif

		// int						GetLevel();
		int						GetInstanceType();

#ifdef ENABLE_NEW_EXCHANGE_WINDOW
		DWORD					GetLevel();
#endif

		DWORD					GetPart(CRaceData::EParts part);
		DWORD					GetShape();
		DWORD					GetRace();
		DWORD					GetVirtualID();
		DWORD					GetVirtualNumber();
		DWORD					GetEmpireID();

#ifdef ENABLE_SHOW_MOB_INFO
		DWORD					GetAIFlag();
#endif

		DWORD					GetGuildID();
		int						GetAlignment();
		UINT					GetAlignmentGrade();
		int						GetAlignmentType();
		BYTE					GetPKMode();
		bool					IsKiller();
		bool					IsPartyMember();

		void					ActDualEmotion(CInstanceBase & rkDstInst, WORD dwMotionNumber1, WORD dwMotionNumber2);
		void					ActEmotion(DWORD dwMotionNumber);
		void					LevelUp();
		void					SkillUp();
		void					UseSpinTop();
		void					Revive();
		void					Stun();
		void					Die();
		void					Hide();
		void					Show();

		bool					CanAct();
		bool					CanMove();
		bool					CanAttack();
		bool					CanUseSkill();
		bool					CanFishing();
		bool					IsConflictAlignmentInstance(CInstanceBase& rkInstVictim);
		bool					IsAttackableInstance(CInstanceBase& rkInstVictim);
		bool					IsTargetableInstance(CInstanceBase& rkInstVictim);
		bool					IsPVPInstance(CInstanceBase& rkInstVictim);
		bool					CanChangeTarget();
		bool					CanPickInstance();
		bool					CanViewTargetHP(CInstanceBase& rkInstVictim);
#ifdef ENABLE_POISON_GAUGE_SYSTEM
		bool					IsPoisoned();
#endif
#ifdef ENABLE_AUTO_SYSTEM
		void					SetAutoAffect(bool val);
#endif
		BOOL					IsGoing();
		bool					NEW_Goto(const TPixelPosition& c_rkPPosDst, float fDstRot);
		void					EndGoing();

		void					SetRunMode();
		void					SetWalkMode();

		bool					IsAffect(UINT uAffect);
		BOOL					IsInvisibility();
		BOOL					IsStealth();
		BOOL					IsParalysis();
		BOOL					IsGameMaster();
		BOOL					IsSameEmpire(CInstanceBase& rkInstDst);
		BOOL					IsBowMode();
		BOOL					IsHandMode();
		BOOL					IsFishingMode();
		BOOL					IsFishing();

		BOOL					IsWearingDress();
		BOOL					IsHoldingPickAxe();
		BOOL					IsMountingHorse();
#ifdef ENABLE_STANDING_MOUNT
		BOOL					IsMountingHoverBoard();
#endif
		BOOL					IsNewMount();
		BOOL					IsForceVisible();
		BOOL					IsInSafe();

#ifdef ENABLE_EVENT_BANNER_FLAG
		BOOL					IsBannerFlag();
#endif

		BOOL					IsEnemy();
		BOOL					IsStone();
		BOOL					IsResource();
		BOOL					IsNPC();
#ifdef ENABLE_GROWTH_PET_SYSTEM
		BOOL					IsGrowthPet();
		void					SetPetScale(int iLevel);
#endif
		BOOL					IsPC();
		BOOL					IsPoly();
		BOOL					IsWarp();
		BOOL					IsGoto();
		BOOL					IsObject();
		BOOL					IsDoor();
		BOOL					IsBuilding();
		BOOL					IsWoodenDoor();
		BOOL					IsStoneDoor();
		BOOL					IsFlag();
		BOOL					IsGuildWall();
#ifdef ENABLE_OFFLINESHOP_SYSTEM
		BOOL					IsShop();
#endif
		BOOL					IsPet();

		BOOL					IsDead();
		BOOL					IsStun();
		BOOL					IsSleep();
		BOOL					__IsSyncing();
		BOOL					IsWaiting();
		BOOL					IsWalking();
		BOOL					IsPushing();
		BOOL					IsAttacking();
		BOOL					IsActingEmotion();
		BOOL					IsAttacked();
		BOOL					IsKnockDown();
		BOOL					IsUsingSkill();
		BOOL					IsUsingMovingSkill();
		BOOL					CanCancelSkill();
		BOOL					CanAttackHorseLevel();

#ifdef __MOVIE_MODE__
		BOOL					IsMovieMode();
#endif
		bool					NEW_CanMoveToDestPixelPosition(const TPixelPosition& c_rkPPosDst);

		void					NEW_SetAdvancingRotationFromPixelPosition(const TPixelPosition& c_rkPPosSrc, const TPixelPosition& c_rkPPosDst);
		void					NEW_SetAdvancingRotationFromDirPixelPosition(const TPixelPosition& c_rkPPosDir);
		bool					NEW_SetAdvancingRotationFromDestPixelPosition(const TPixelPosition& c_rkPPosDst);
		void					SetAdvancingRotation(float fRotation);

		void					EndWalking(float fBlendingTime=0.15f);
		void					EndWalkingWithoutBlending();

		void					SetEventHandler(CActorInstance::IEventHandler* pkEventHandler);

		void					PushUDPState(DWORD dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg);
		void					PushTCPState(DWORD dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg);
		void					PushTCPStateExpanded(DWORD dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg, UINT uTargetVID);

		void					NEW_Stop();

		bool					NEW_UseSkill(UINT uSkill, UINT uMot, UINT uMotLoopCount, bool isMovingSkill);
		void					NEW_Attack();
		void					NEW_Attack(float fDirRot);
		void					NEW_AttackToDestPixelPositionDirection(const TPixelPosition& c_rkPPosDst);
		bool					NEW_AttackToDestInstanceDirection(CInstanceBase& rkInstDst, IFlyEventHandler* pkFlyHandler);
		bool					NEW_AttackToDestInstanceDirection(CInstanceBase& rkInstDst);

		bool					NEW_MoveToDestPixelPositionDirection(const TPixelPosition& c_rkPPosDst);
		void					NEW_MoveToDestInstanceDirection(CInstanceBase& rkInstDst);
		void					NEW_MoveToDirection(float fDirRot);

		float					NEW_GetDistanceFromDirPixelPosition(const TPixelPosition& c_rkPPosDir);
		float					NEW_GetDistanceFromDestPixelPosition(const TPixelPosition& c_rkPPosDst);
		float					NEW_GetDistanceFromDestInstance(CInstanceBase& rkInstDst);

		float					NEW_GetRotation();
		float					NEW_GetRotationFromDestPixelPosition(const TPixelPosition& c_rkPPosDst);
		float					NEW_GetRotationFromDirPixelPosition(const TPixelPosition& c_rkPPosDir);
		float					NEW_GetRotationFromDestInstance(CInstanceBase& rkInstDst);

		float					NEW_GetAdvancingRotationFromDirPixelPosition(const TPixelPosition& c_rkPPosDir);
		float					NEW_GetAdvancingRotationFromDestPixelPosition(const TPixelPosition& c_rkPPosDst);
		float					NEW_GetAdvancingRotationFromPixelPosition(const TPixelPosition& c_rkPPosSrc, const TPixelPosition& c_rkPPosDst);

		BOOL					NEW_IsClickableDistanceDestPixelPosition(const TPixelPosition& c_rkPPosDst);
		BOOL					NEW_IsClickableDistanceDestInstance(CInstanceBase& rkInstDst);

		bool					NEW_GetFrontInstance(CInstanceBase ** ppoutTargetInstance, float fDistance);
		void					NEW_GetRandomPositionInFanRange(CInstanceBase& rkInstTarget, TPixelPosition* pkPPosDst);
		bool					NEW_GetInstanceVectorInFanRange(float fSkillDistance, CInstanceBase& rkInstTarget, std::vector<CInstanceBase*>* pkVct_pkInst);
		bool					NEW_GetInstanceVectorInCircleRange(float fSkillDistance, std::vector<CInstanceBase*>* pkVct_pkInst);

		void					NEW_SetOwner(DWORD dwOwnerVID);
		void					NEW_SyncPixelPosition(LONG & nPPosX, LONG & nPPosY);
		void					NEW_SyncCurrentPixelPosition();

		void					NEW_SetPixelPosition(const TPixelPosition& c_rkPPosDst);

		bool					NEW_IsLastPixelPosition();
		const TPixelPosition&	NEW_GetLastPixelPositionRef();

		BOOL					isNormalAttacking();
		BOOL					isComboAttacking();
		MOTION_KEY				GetNormalAttackIndex();
		DWORD					GetComboIndex();
		float					GetAttackingElapsedTime();
		void					InputNormalAttack(float fAtkDirRot);
		void					InputComboAttack(float fAtkDirRot);

		void					RunNormalAttack(float fAtkDirRot);
		void					RunComboAttack(float fAtkDirRot, DWORD wMotionIndex);

		CInstanceBase*			FindNearestVictim();
		BOOL					CheckAdvancing();


		bool					AvoidObject(const CGraphicObjectInstance& c_rkBGObj);		
		bool					IsBlockObject(const CGraphicObjectInstance& c_rkBGObj);
		void					BlockMovement();

	public:
		BOOL					CheckAttacking(CInstanceBase& rkInstVictim);
		void					ProcessHitting(DWORD dwMotionKey, CInstanceBase * pVictimInstance);
		void					ProcessHitting(DWORD dwMotionKey, BYTE byEventIndex, CInstanceBase * pVictimInstance);
		void					GetBlendingPosition(TPixelPosition * pPixelPosition);
		void					SetBlendingPosition(const TPixelPosition & c_rPixelPosition);

		void					StartFishing(float frot);
		void					StopFishing();
		void					ReactFishing();
		void					CatchSuccess();
		void					CatchFail();
		BOOL					GetFishingRot(int * pirot);

		void					RestoreRenderMode();
		void					SetAddRenderMode();
		void					SetModulateRenderMode();
		void					SetRenderMode(int iRenderMode);
		void					SetAddColor(const D3DXCOLOR & c_rColor);

		void					SCRIPT_SetPixelPosition(float fx, float fy);
		void					NEW_GetPixelPosition(TPixelPosition * pPixelPosition);

		void					NEW_LookAtFlyTarget();
		void					NEW_LookAtDestInstance(CInstanceBase& rkInstDst);
		void					NEW_LookAtDestPixelPosition(const TPixelPosition& c_rkPPosDst);

		float					GetRotation();
		float					GetAdvancingRotation();
		void					SetRotation(float fRotation);
		void					BlendRotation(float fRotation, float fBlendTime = 0.1f);

		void					SetDirection(int dir);
		void					BlendDirection(int dir, float blendTime);
		float					GetDegreeFromDirection(int dir);

		BOOL					isLock();

		void					SetMotionMode(int iMotionMode);
		int						GetMotionMode(DWORD dwMotionIndex);

		void					ResetLocalTime();
		void					SetLoopMotion(WORD wMotion, float fBlendTime=0.1f, float fSpeedRatio=1.0f);
		void					PushOnceMotion(WORD wMotion, float fBlendTime=0.1f, float fSpeedRatio=1.0f);
		void					PushLoopMotion(WORD wMotion, float fBlendTime=0.1f, float fSpeedRatio=1.0f);
		void					SetEndStopMotion();

		bool					IntersectDefendingSphere();
		bool					IntersectBoundingBox();

		void					Refresh(DWORD dwMotIndex, bool isLoop);

		float					GetDistance(CInstanceBase * pkTargetInst);
		float					GetDistance(const TPixelPosition & c_rPixelPosition);

		CActorInstance&			GetGraphicThingInstanceRef();
		CActorInstance*			GetGraphicThingInstancePtr();		
		
		bool __Background_IsWaterPixelPosition(const TPixelPosition& c_rkPPos);
		bool __Background_GetWaterHeight(const TPixelPosition& c_rkPPos, float* pfHeight);

		void __ClearAffectFlagContainer();
		void __ClearAffects();

		void __SetAffect(UINT eAffect, bool isVisible);
		
		void SetAffectFlagContainer(const CAffectFlagContainer& c_rkAffectFlagContainer);

		void __SetNormalAffectFlagContainer(const CAffectFlagContainer& c_rkAffectFlagContainer);
		void __SetStoneSmokeFlagContainer(const CAffectFlagContainer& c_rkAffectFlagContainer);

		void SetEmoticon(UINT eEmoticon);
		void SetFishEmoticon();
		bool IsPossibleEmoticon();

	protected:
		UINT					__LessRenderOrder_GetLODLevel();
		void					__Initialize();
		void					__InitializeRotationSpeed();

		void					__Create_SetName(const SCreateData& c_rkCreateData);
		void					__Create_SetWarpName(const SCreateData& c_rkCreateData);

		CInstanceBase*			__GetMainInstancePtr();
		CInstanceBase*			__FindInstancePtr(DWORD dwVID);

		bool  __FindRaceType(DWORD dwRace, BYTE* pbType);
		DWORD __GetRaceType();

		bool __IsShapeAnimalWear();
		BOOL __IsChangableWeapon(int iWeaponID);

		void __EnableSkipCollision();
		void __DisableSkipCollision();

		void __ClearMainInstance();

		void __Shaman_SetParalysis(bool isParalysis);
		void __Warrior_SetGeomgyeongAffect(bool isVisible);
		void __Assassin_SetEunhyeongAffect(bool isVisible);
		void __SetReviveInvisibilityAffect(bool isVisible);

#ifdef ENABLE_NEW_GYEONGGONG_SKILL
		void __Assassin_SetGyeongGongAffect(bool isVisible);
#endif

		BOOL __CanProcessNetworkStatePacket();
		
		bool __IsInDustRange();

		void __ProcessFunctionEmotion(DWORD dwMotionNumber, DWORD dwTargetVID, const TPixelPosition & c_rkPosDst);
		void __EnableChangingTCPState();
		void __DisableChangingTCPState();
		BOOL __IsEnableTCPProcess(UINT eCurFunc);

		bool __CanRender();
		bool __IsInViewFrustum();

		void __AttachHorseSaddle();
		void __DetachHorseSaddle();
		
		struct SHORSE
		{
			bool m_isMounting;
			CActorInstance* m_pkActor;
			
			SHORSE();
			~SHORSE();
			
			void Destroy();
			void Create(const TPixelPosition& c_rkPPos, UINT eRace, UINT eHitEffect);
			
			void SetAttackSpeed(UINT uAtkSpd);
			void SetMoveSpeed(UINT uMovSpd);
			void Deform();
			void Render();
			CActorInstance& GetActorRef();
			CActorInstance* GetActorPtr();

			bool IsMounting();
			bool CanAttack();
			bool CanUseSkill();

			UINT GetLevel();
			bool IsNewMount();
#ifdef ENABLE_STANDING_MOUNT
			bool IsHoverBoard();
#endif
			void __Initialize();
		} m_kHorse;


	protected:
		void					__SetBlendRenderingMode();
		void					__SetAlphaValue(float fAlpha);
		float					__GetAlphaValue();

		void					__ComboProcess();
		void					MovementProcess();
		void					TodoProcess();
		void					StateProcess();
		void					AttackProcess();

#ifdef ENABLE_ANIMATION_OPTIMIZATION
		bool					IsMainInstance() const;
		void					AttackProcessOnlySelf();
#endif

		void					StartWalking();
		float					GetLocalTime();

		void					RefreshState(DWORD dwMotIndex, bool isLoop);
		void					RefreshActorInstance();

	protected:
		void					OnSyncing();
		void					OnWaiting();
		void					OnMoving();

		void					NEW_SetCurPixelPosition(const TPixelPosition& c_rkPPosDst);
		void					NEW_SetSrcPixelPosition(const TPixelPosition& c_rkPPosDst);
		void					NEW_SetDstPixelPosition(const TPixelPosition& c_rkPPosDst);
		void					NEW_SetDstPixelPositionZ(FLOAT z);

		const TPixelPosition&	NEW_GetCurPixelPositionRef();
		const TPixelPosition&	NEW_GetSrcPixelPositionRef();

	public:
		const TPixelPosition&	NEW_GetDstPixelPositionRef();
		
	protected:
		BOOL m_isTextTail;		

		std::string				m_stName;

		DWORD					m_awPart[CRaceData::PART_MAX_NUM];
		DWORD					m_dwLevel;
#ifdef ENABLE_CONQUEROR_LEVEL
		DWORD					m_dwConquerorLevel;
#endif
#ifdef ENABLE_REFINE_ELEMENT
		BYTE					m_bRefineElementType;
#endif
#if defined(ENABLE_SHOW_MOB_INFO)
		DWORD					m_dwAIFlag;
#endif
		DWORD					m_dwEmpireID;
		DWORD					m_dwGuildID;

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		BYTE					m_bLanguage;
#endif

#ifdef ENABLE_LEFT_SEAT
		bool					m_bLeftSeat;
#endif

	protected:
		CAffectFlagContainer	m_kAffectFlagContainer;
		DWORD					m_adwCRCAffectEffect[AFFECT_NUM];
		UINT	__GetRefinedEffect(CItemData* pItem);
		void	__ClearWeaponRefineEffect();
		void	__ClearArmorRefineEffect();

#ifdef ENABLE_SASH_SYSTEM
		void	ClearSashEffect();
#endif

#ifdef ENABLE_AURA_SYSTEM
		void	ClearAuraEffect();
#endif

	protected:
		void __AttachSelectEffect();
		void __DetachSelectEffect();

		void __AttachTargetEffect();
		void __DetachTargetEffect();

		void __AttachEmpireEffect(DWORD eEmpire);

	protected:
		struct SEffectContainer
		{
			typedef std::map<DWORD, DWORD> Dict;
			Dict m_kDct_dwEftID;
		} m_kEffectContainer;

		void __EffectContainer_Initialize();
		void __EffectContainer_Destroy();
#ifdef ENABLE_GRAPHIC_ON_OFF
		void __EffectContainer_Suspend();
		void __EffectContainer_Continue();
#endif
#ifdef ENABLE_BOSS_EFFECT_SYSTEM
		void __AttachEffectBoss();
#endif

		DWORD __EffectContainer_AttachEffect(DWORD eEffect);
		void __EffectContainer_DetachEffect(DWORD eEffect);

		SEffectContainer::Dict& __EffectContainer_GetDict();

	protected:
		struct SStoneSmoke 
		{
			DWORD m_dwEftID;
		} m_kStoneSmoke;

		void __StoneSmoke_Inialize();
		void __StoneSmoke_Destroy();
		void __StoneSmoke_Create(DWORD eSmoke);


	protected:
		BYTE					m_eType;
		BYTE					m_eRaceType;
		DWORD					m_eShape;
		DWORD					m_dwRace;
		DWORD					m_dwVirtualNumber;
		short					m_sAlignment;
		BYTE					m_byPKMode;

#ifdef ENABLE_SUPPORT_SYSTEM
		bool					is_support_shaman;
#endif

#ifdef ENABLE_BATTLE_FIELD
		BYTE					combat_zone_rank;
		DWORD					combat_zone_points;
#endif

		bool					m_isKiller;
		bool					m_isPartyMember;

#ifdef ENABLE_AUTO_SYSTEM
		bool					m_isAutoTarget;
#endif

		int						m_iRotatingDirection;

		DWORD					m_dwAdvActorVID;
		DWORD					m_dwLastDmgActorVID;

		LONG					m_nAverageNetworkGap;
		DWORD					m_dwNextUpdateHeightTime;

		bool					m_isGoing;

		TPixelPosition			m_kPPosDust;

		DWORD					m_dwLastComboIndex;

		DWORD					m_swordRefineEffectRight;
		DWORD					m_swordRefineEffectLeft;
		DWORD					m_armorRefineEffect;

#ifdef ENABLE_OFFLINESHOP_SYSTEM
		DWORD					m_myShop;
#endif

#ifdef ENABLE_SASH_SYSTEM
		DWORD					m_dwSashEffect;
#endif

#ifdef ENABLE_TITLE_SYSTEM
		DWORD					m_dwTitleEffect;
#endif

#ifdef ENABLE_AURA_SYSTEM
		DWORD					m_auraRefineEffect;
#endif

#ifdef ENABLE_GREEDY_ROOM
		DWORD					m_dwGreedyTornadoEffectID;
		DWORD					m_dwGreedyTornadoEffectID2;
		DWORD					m_dwFireWaveEffectID;
		DWORD					m_dwSpearWarningEffectID;
		DWORD					m_dwSpearFireEffectID;
#endif

		struct SMoveAfterFunc
		{
			UINT eFunc;
			UINT uArg;

			UINT uArgExpanded;
			TPixelPosition kPosDst;
		};

		SMoveAfterFunc m_kMovAfterFunc;

		float m_fDstRot;
		float m_fAtkPosTime;
		float m_fRotSpd;
		float m_fMaxRotSpd;

		BOOL m_bEnableTCPState;

		CActorInstance m_GraphicThingInstance;

	private:
		uint32_t m_lastAttackerVID; // Son saldıran oyuncunun VID'si
		uint32_t m_lastAttackTime;  // Son saldırı zamanı (opsiyonel)
	
	public:
		void SetLastAttacker(uint32_t attackerVID)
		{
			m_lastAttackerVID = attackerVID;
			m_lastAttackTime = GetCurrentTime(); // Saldırı zamanını kaydet
		}
	
		uint32_t GetLastAttacker() const { return m_lastAttackerVID; }

		bool IsUnderAttackByOtherPlayer() const;

	protected:
		struct SCommand
		{
			DWORD	m_dwChkTime;
			DWORD	m_dwCmdTime;
			float	m_fDstRot;
			UINT 	m_eFunc;
			UINT 	m_uArg;
			UINT	m_uTargetVID;
			TPixelPosition m_kPPosDst;
		};

		typedef std::list<SCommand> CommandQueue;

		DWORD		m_dwBaseChkTime;
		DWORD		m_dwBaseCmdTime;

		DWORD		m_dwSkipTime;

		CommandQueue m_kQue_kCmdNew;

		BOOL		m_bDamageEffectType;

		struct SEffectDamage
		{
			DWORD damage;
			BYTE flag;
			BOOL bSelf;
			BOOL bTarget;
		};

#ifdef ENABLE_DAMAGE_QUEUE_FIX
		typedef std :: queue < SEffectDamage > CommandDamageQueue ;
#else
		typedef std::list<SEffectDamage> CommandDamageQueue;
#endif
		CommandDamageQueue m_DamageQueue;

		void ProcessDamage();

	public:
		void AddDamageEffect(DWORD damage,BYTE flag,BOOL bSelf,BOOL bTarget);

	protected:
		struct SWarrior
		{
			DWORD m_dwGeomgyeongEffect;
		};

		SWarrior m_kWarrior;

		void __Warrior_Initialize();

#ifdef ENABLE_NEW_GYEONGGONG_SKILL
	protected:
		struct SAssassin
		{
			DWORD m_dwGyeongGongEffect;
		};

		SAssassin m_kAssassin;

		void __Assassin_Initialize();
#endif

	public:
		static void ClearPVPKeySystem();

		static void InsertPVPKey(DWORD dwSrcVID, DWORD dwDstVID);
		static void InsertPVPReadyKey(DWORD dwSrcVID, DWORD dwDstVID);
		static void RemovePVPKey(DWORD dwSrcVID, DWORD dwDstVID);

		static void InsertGVGKey(DWORD dwSrcGuildVID, DWORD dwDstGuildVID);
		static void RemoveGVGKey(DWORD dwSrcGuildVID, DWORD dwDstGuildVID);

		static void InsertDUELKey(DWORD dwSrcVID, DWORD dwDstVID);

		UINT GetNameColorIndex();

		const D3DXCOLOR& GetNameColor();
		const D3DXCOLOR& GetTitleColor();

	protected:
		static DWORD __GetPVPKey(DWORD dwSrcVID, DWORD dwDstVID);
		static bool __FindPVPKey(DWORD dwSrcVID, DWORD dwDstVID);
		static bool __FindPVPReadyKey(DWORD dwSrcVID, DWORD dwDstVID);
		static bool __FindGVGKey(DWORD dwSrcGuildID, DWORD dwDstGuildID);
		static bool __FindDUELKey(DWORD dwSrcGuildID, DWORD dwDstGuildID);

	protected:
		CActorInstance::IEventHandler* GetEventHandlerPtr();
		CActorInstance::IEventHandler& GetEventHandlerRef();

	protected:
		static float __GetBackgroundHeight(float x, float y);
		static DWORD __GetShadowMapColor(float x, float y);

	public:
		static void ResetPerformanceCounter();
		static void GetInfo(std::string* pstInfo);

	public:
		static CInstanceBase* New();
		static void Delete(CInstanceBase* pkInst);

		static CDynamicPool<CInstanceBase>	ms_kPool;

#ifdef ENABLE_BALATHOR_DUNGEON
	public:
		CActorInstance* GetActorInstance() { return &m_GraphicThingInstance; }
		void RemoveBalathorEffect(const TBalathorEffect* pEffect);
		void AddBalathorEffect(const TBalathorEffect* pEffect);
	protected:
		std::map<DWORD, DWORD> m_mapBalathorEffects;
#endif

	protected:
		static DWORD ms_dwUpdateCounter;
		static DWORD ms_dwRenderCounter;
		static DWORD ms_dwDeformCounter;

	public:		
		DWORD					GetDuelMode();
		void					SetDuelMode(DWORD type);
	protected:
		DWORD					m_dwDuelMode;
		DWORD					m_dwEmoticonTime;

	protected:
		bool m_IsAlwaysRender;

	public:
		void SetQueueEntity(DWORD state) { m_GraphicThingInstance.SetQueueElement(state); }

	public:
		bool IsAlwaysRender();
		void SetAlwaysRender(bool val);

#ifdef ENABLE_MINIMIZED_EFFECT_FIX
	public:
		DWORD m_dwLastUpdateTime = 0;
#endif
};

inline int RaceToJob(int race)
{
	switch (race)
	{
		case 0:
		case 4:
			return 0;
		case 1:
		case 5:
			return 1;
		case 2:
		case 6:
			return 2;
		case 3:
		case 7:
			return 3;
		case 8:
			return 4;
		default:
			return 0;
	}
	return 0;
}

inline int RaceToSex(int race)
{
	switch (race)
	{
		case 0:
		case 2:
		case 5:
		case 7:
		case 8:
			return 1;
		case 1:
		case 3:
		case 4:
		case 6:
			return 0;

	}
	return 0;
}


