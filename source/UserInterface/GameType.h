#pragma once
#include "Locale_inc.h"
#include <cstdint>

#include "../GameLib/ItemData.h"

struct SAffects
{
	enum
	{
		AFFECT_MAX_NUM = 32,
	};

	SAffects() : dwAffects(0) {}
	SAffects(const DWORD & c_rAffects)
	{
		__SetAffects(c_rAffects);
	}
	int operator = (const DWORD & c_rAffects)
	{
		__SetAffects(c_rAffects);
	}

	BOOL IsAffect(BYTE byIndex)
	{
		return dwAffects & (1 << byIndex);
	}

	void __SetAffects(const DWORD & c_rAffects)
	{
		dwAffects = c_rAffects;
	}

	DWORD dwAffects;
};

extern std::string g_strGuildSymbolPathName;

const DWORD c_Name_Max_Length = 64;
const DWORD c_FileName_Max_Length = 128;
const DWORD c_Short_Name_Max_Length = 32;

const DWORD c_Inventory_Page_Size = 5*9;
const DWORD c_Inventory_Page_Count = 4;

#ifdef ENABLE_SPLIT_INVENTORY_SYSTEM
const DWORD c_Special_Inventory_Page_Size = 5*9;
const DWORD c_Special_Inventory_Page_Count = 4;
const DWORD c_Special_ItemSlot_Count = c_Special_Inventory_Page_Size * c_Special_Inventory_Page_Count;
#endif

#ifdef ENABLE_SOUL_ROULETTE_SYSTEM
const int ROULETTE_ITEM_MAX = 20;
#endif

const DWORD c_ItemSlot_Count = c_Inventory_Page_Size * c_Inventory_Page_Count;
const DWORD c_Equipment_Count = 12;

const DWORD c_Equipment_Start = c_ItemSlot_Count;

const DWORD c_Equipment_Body	= c_Equipment_Start + 0;
const DWORD c_Equipment_Head	= c_Equipment_Start + 1;
const DWORD c_Equipment_Shoes	= c_Equipment_Start + 2;
const DWORD c_Equipment_Wrist	= c_Equipment_Start + 3;
const DWORD c_Equipment_Weapon	= c_Equipment_Start + 4;
const DWORD c_Equipment_Neck	= c_Equipment_Start + 5;
const DWORD c_Equipment_Ear		= c_Equipment_Start + 6;
const DWORD c_Equipment_Unique1	= c_Equipment_Start + 7;
const DWORD c_Equipment_Unique2	= c_Equipment_Start + 8;
const DWORD c_Equipment_Arrow	= c_Equipment_Start + 9;
const DWORD c_Equipment_Shield	= c_Equipment_Start + 10;

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
const DWORD c_New_Equipment_Start = c_Equipment_Start + 25;
const DWORD c_Equipment_Ring1 = c_New_Equipment_Start + 0;
const DWORD c_Equipment_Ring2 = c_New_Equipment_Start + 1;
const DWORD c_Equipment_Belt  = c_New_Equipment_Start + 2;;
#ifdef ENABLE_PENDANT
const DWORD c_Equipment_Pendant  = c_New_Equipment_Start + 3;
#endif
#ifdef ENABLE_GLOVE_SYSTEM
const DWORD c_Equipment_Glove  = c_New_Equipment_Start + 4;
#endif
#ifdef ENABLE_EXTENDED_PET_SYSTEM
const DWORD c_Equipment_Pet  = c_New_Equipment_Start + 5;
#endif
#ifdef ENABLE_PASSIVE_SYSTEM
const DWORD c_Equipment_Passive  = c_New_Equipment_Start + 6;
#endif
#ifdef ENABLE_NEW_RING_EQUIPMENT
const DWORD c_Equipment_New_Glove  = c_New_Equipment_Start + 7;
const DWORD c_Equipment_New_Exp  = c_New_Equipment_Start + 8;
#endif
const DWORD c_New_Equipment_Count = 9;
#endif
#ifdef ENABLE_RIDING_EXTENDED
const BYTE HORSE_MAX_LEVEL = 40;
#endif

enum EDragonSoulDeckType
{
	DS_DECK_1,
	DS_DECK_2,
	DS_DECK_MAX_NUM = 2,
};

#ifdef ENABLE_DS_GRADE_MYTH
enum EDragonSoulGradeTypes
{
	DRAGON_SOUL_GRADE_NORMAL,
	DRAGON_SOUL_GRADE_BRILLIANT,
	DRAGON_SOUL_GRADE_RARE,
	DRAGON_SOUL_GRADE_ANCIENT,
	DRAGON_SOUL_GRADE_LEGENDARY,
	DRAGON_SOUL_GRADE_MYTH,
	DRAGON_SOUL_GRADE_MAX,

};
#else
enum EDragonSoulGradeTypes
{
	DRAGON_SOUL_GRADE_NORMAL,
	DRAGON_SOUL_GRADE_BRILLIANT,
	DRAGON_SOUL_GRADE_RARE,
	DRAGON_SOUL_GRADE_ANCIENT,
	DRAGON_SOUL_GRADE_LEGENDARY,
	DRAGON_SOUL_GRADE_MAX,

};
#endif

enum EDragonSoulStepTypes
{
	DRAGON_SOUL_STEP_LOWEST,
	DRAGON_SOUL_STEP_LOW,
	DRAGON_SOUL_STEP_MID,
	DRAGON_SOUL_STEP_HIGH,
	DRAGON_SOUL_STEP_HIGHEST,
	DRAGON_SOUL_STEP_MAX,
};

#ifdef ENABLE_SKILLBOOK_COMB_SYSTEM
const DWORD c_SkillBook_Comb_Size = 10;
enum ESkillBookComb
{
	SKILLBOOK_COMB_SLOT_MAX = c_SkillBook_Comb_Size
};
#endif

#ifdef ENABLE_COSTUME_SYSTEM
	const DWORD c_Costume_Slot_Start	= c_Equipment_Start + 19;
	const DWORD c_Costume_Slot_Body		= c_Costume_Slot_Start + 0;
	const DWORD c_Costume_Slot_Hair		= c_Costume_Slot_Start + 1;
#ifdef ENABLE_SASH_SYSTEM
	const DWORD c_Costume_Slot_Sash		= c_Costume_Slot_Start + 2;
#endif
#ifdef ENABLE_COSTUME_WEAPON_SYSTEM
	const DWORD c_Costume_Slot_Weapon	= c_Costume_Slot_Start + 3;
#endif
	const DWORD c_Costume_Slot_Mount = c_Costume_Slot_Start + 4;
#ifdef ENABLE_AURA_SYSTEM
	const DWORD c_Costume_Slot_Aura	= c_Costume_Slot_Start + 5;
#endif
	const DWORD c_Costume_Slot_Count	= 6;
	const DWORD c_Costume_Slot_End		= c_Costume_Slot_Start + c_Costume_Slot_Count;
#endif

const DWORD c_Wear_Max = 34;

const DWORD c_DragonSoul_Equip_Start = c_ItemSlot_Count + c_Wear_Max;
#ifdef ENABLE_DRAGON_SOUL_7_SLOT
const DWORD c_DragonSoul_Equip_Slot_Max = 7;
#else
const DWORD c_DragonSoul_Equip_Slot_Max = 6;
#endif
const DWORD c_DragonSoul_Equip_End = c_DragonSoul_Equip_Start + c_DragonSoul_Equip_Slot_Max * DS_DECK_MAX_NUM;
const DWORD c_DragonSoul_Equip_Reserved_Count = c_DragonSoul_Equip_Slot_Max * 3;

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
	const DWORD c_Belt_Inventory_Slot_Start = c_DragonSoul_Equip_End + c_DragonSoul_Equip_Reserved_Count;
#ifdef ENABLE_BELT_INVENTORY_RENEWAL
	const DWORD c_Belt_Inventory_Width = 6;
	const DWORD c_Belt_Inventory_Height= 6;
#else
	const DWORD c_Belt_Inventory_Width = 4;
	const DWORD c_Belt_Inventory_Height= 4;
#endif
	const DWORD c_Belt_Inventory_Slot_Count = c_Belt_Inventory_Width * c_Belt_Inventory_Height;
	const DWORD c_Belt_Inventory_Slot_End = c_Belt_Inventory_Slot_Start + c_Belt_Inventory_Slot_Count;
#endif

#ifdef ENABLE_SPLIT_INVENTORY_SYSTEM
	const DWORD c_Skill_Book_Inventory_Slot_Start = c_Belt_Inventory_Slot_End;
	const DWORD c_Skill_Book_Inventory_Slot_Count = c_Special_ItemSlot_Count;
	const DWORD c_Skill_Book_Inventory_Slot_End = c_Skill_Book_Inventory_Slot_Start + c_Skill_Book_Inventory_Slot_Count;
	
	const DWORD c_Upgrade_Items_Inventory_Slot_Start = c_Skill_Book_Inventory_Slot_End;
	const DWORD c_Upgrade_Items_Inventory_Slot_Count = c_Special_ItemSlot_Count;
	const DWORD c_Upgrade_Items_Inventory_Slot_End = c_Upgrade_Items_Inventory_Slot_Start + c_Upgrade_Items_Inventory_Slot_Count;
	
	const DWORD c_Stone_Inventory_Slot_Start = c_Upgrade_Items_Inventory_Slot_End;
	const DWORD c_Stone_Inventory_Slot_Count = c_Special_ItemSlot_Count;
	const DWORD c_Stone_Inventory_Slot_End = c_Stone_Inventory_Slot_Start + c_Stone_Inventory_Slot_Count;
	
	const DWORD c_Box_Inventory_Slot_Start = c_Stone_Inventory_Slot_End;
	const DWORD c_Box_Inventory_Slot_Count = c_Special_ItemSlot_Count;
	const DWORD c_Box_Inventory_Slot_End = c_Box_Inventory_Slot_Start + c_Box_Inventory_Slot_Count;
	
	const DWORD c_Efsun_Inventory_Slot_Start = c_Box_Inventory_Slot_End;
	const DWORD c_Efsun_Inventory_Slot_Count = c_Special_ItemSlot_Count;
	const DWORD c_Efsun_Inventory_Slot_End = c_Efsun_Inventory_Slot_Start + c_Efsun_Inventory_Slot_Count;
	
	const DWORD c_Cicek_Inventory_Slot_Start = c_Efsun_Inventory_Slot_End;
	const DWORD c_Cicek_Inventory_Slot_Count = c_Special_ItemSlot_Count;
	const DWORD c_Cicek_Inventory_Slot_End = c_Cicek_Inventory_Slot_Start + c_Cicek_Inventory_Slot_Count;

#ifdef ENABLE_SPLIT_INVENTORY_SYSTEM
	const DWORD c_Inventory_Count = c_Cicek_Inventory_Slot_End;
#else
	const DWORD c_Inventory_Count = c_Belt_Inventory_Slot_End;
#endif
#else
	const DWORD c_Inventory_Count = c_DragonSoul_Equip_End;
#endif

const DWORD c_DragonSoul_Inventory_Start = 0;
const DWORD c_DragonSoul_Inventory_Box_Size = 32;
const DWORD c_DragonSoul_Inventory_Count = CItemData::DS_SLOT_NUM_TYPES * DRAGON_SOUL_GRADE_MAX * c_DragonSoul_Inventory_Box_Size;
const DWORD c_DragonSoul_Inventory_End = c_DragonSoul_Inventory_Start + c_DragonSoul_Inventory_Count;

enum ESlotType
{
	SLOT_TYPE_NONE,
	SLOT_TYPE_INVENTORY,
	SLOT_TYPE_SKILL,
	SLOT_TYPE_EMOTION,
	SLOT_TYPE_SHOP,
	SLOT_TYPE_EXCHANGE_OWNER,
	SLOT_TYPE_EXCHANGE_TARGET,
	SLOT_TYPE_QUICK_SLOT,
	SLOT_TYPE_SAFEBOX,
	SLOT_TYPE_PRIVATE_SHOP,
	SLOT_TYPE_MALL,
	SLOT_TYPE_DRAGON_SOUL_INVENTORY,
	SLOT_TYPE_BELT_INVENTORY,
#ifdef ENABLE_SPLIT_INVENTORY_SYSTEM
	SLOT_TYPE_SKILL_BOOK_INVENTORY,
	SLOT_TYPE_UPGRADE_ITEMS_INVENTORY,
	SLOT_TYPE_STONE_INVENTORY,
	SLOT_TYPE_BOX_INVENTORY,
	SLOT_TYPE_EFSUN_INVENTORY,
	SLOT_TYPE_CICEK_INVENTORY,
#endif
#ifdef ENABLE_SWITCHBOT
	SLOT_TYPE_SWITCHBOT,
#endif
#ifdef ENABLE_FISH_EVENT_SYSTEM
	SLOT_TYPE_FISH_EVENT,
#endif
#ifdef ENABLE_AUTO_SYSTEM
	SLOT_TYPE_AUTO,
#endif
	SLOT_TYPE_MAX,
};

enum EWindows
{
	RESERVED_WINDOW,
	INVENTORY,
	EQUIPMENT,
	SAFEBOX,
	MALL,
	DRAGON_SOUL_INVENTORY,
	BELT_INVENTORY,
#ifdef ENABLE_SWITCHBOT
	SWITCHBOT,
#endif
#ifdef ENABLE_SPLIT_INVENTORY_SYSTEM
	SKILL_BOOK_INVENTORY,
	UPGRADE_ITEMS_INVENTORY,
	STONE_INVENTORY,
	BOX_INVENTORY,
	EFSUN_INVENTORY,
	CICEK_INVENTORY,
#endif
	GROUND,
	WINDOW_TYPE_MAX,
};

#ifdef ENABLE_FISH_EVENT_SYSTEM
enum EFishEventInfo
{
	FISH_EVENT_SHAPE_NONE,
	FISH_EVENT_SHAPE_1,
	FISH_EVENT_SHAPE_2,
	FISH_EVENT_SHAPE_3,
	FISH_EVENT_SHAPE_4,
	FISH_EVENT_SHAPE_5,
	FISH_EVENT_SHAPE_6,
	FISH_EVENT_SHAPE_7,
	FISH_EVENT_SHAPE_MAX_NUM,
};
#endif

#ifdef ENABLE_LEFT_SEAT
enum ELeftSeatTime
{
	LEFT_SEAT_TIME_10_MIN,
	LEFT_SEAT_TIME_30_MIN,
	LEFT_SEAT_TIME_90_MIN,
	LEFT_SEAT_TIME_MAX,
};

enum ELeftSeatLogoutTime
{
	LEFT_SEAT_LOGOUT_TIME_30_MIN,
	LEFT_SEAT_LOGOUT_TIME_60_MIN,
	LEFT_SEAT_LOGOUT_TIME_120_MIN,
	LEFT_SEAT_LOGOUT_TIME_180_MIN,
	LEFT_SEAT_LOGOUT_TIME_OFF,
	LEFT_SEAT_LOGOUT_TIME_MAX,
};
#endif

#ifdef ENABLE_TRADABLE_ICON
enum ETopWindowTypes
{
	ON_TOP_WND_NONE,
	ON_TOP_WND_SHOP,
	ON_TOP_WND_EXCHANGE,
	ON_TOP_WND_SAFEBOX,
	ON_TOP_WND_PRIVATE_SHOP,
	ON_TOP_WND_ITEM_COMB,
	ON_TOP_WND_PET_FEED,
#ifdef ENABLE_OFFLINESHOP_SYSTEM
	ON_TOP_WND_OFFLINE_SHOP,
	ON_TOP_WND_OFFLINE_SHOP_MANAGER,
#endif
#ifdef ENABLE_MAILBOX_SYSTEM
	ON_TOP_WND_MAILBOX,
#endif
#ifdef ENABLE_ATTR_6TH_7TH_SYSTEM
	ON_TOP_WND_ATTR_67,
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
	ON_TOP_WND_PET_FEED_LIFE,
	ON_TOP_WND_PET_FEED_EQUIP,
	ON_TOP_WND_PET_PAGE_ATTR,
	ON_TOP_WND_PET_PAGE_REVIVE_1,
	ON_TOP_WND_PET_PAGE_REVIVE_2,
#endif
	ON_TOP_WND_MAX,
};
#endif

enum EDSInventoryMaxNum
{
	DS_INVENTORY_MAX_NUM = c_DragonSoul_Inventory_Count,
	DS_REFINE_WINDOW_MAX_NUM = 15,
};

#pragma pack (push, 1)
#define WORD_MAX 0xffff

#ifdef ENABLE_SWITCHBOT
enum ESwitchbotValues
{
	SWITCHBOT_SLOT_COUNT = 5,
	SWITCHBOT_ALTERNATIVE_COUNT = 2,
	MAX_NORM_ATTR_NUM = 5,
};

enum EAttributeSet
{
	ATTRIBUTE_SET_WEAPON,
	ATTRIBUTE_SET_BODY,
	ATTRIBUTE_SET_WRIST,
	ATTRIBUTE_SET_FOOTS,
	ATTRIBUTE_SET_NECK,
	ATTRIBUTE_SET_HEAD,
	ATTRIBUTE_SET_SHIELD,
	ATTRIBUTE_SET_EAR,
#ifdef ENABLE_ITEM_ATTR_COSTUME
	ATTRIBUTE_SET_COSTUME_BODY,
	ATTRIBUTE_SET_COSTUME_HAIR,
	ATTRIBUTE_SET_COSTUME_WEAPON,
#endif
#ifdef ENABLE_PENDANT
	ATTRIBUTE_SET_PENDANT,
#endif
#ifdef ENABLE_GLOVE_SYSTEM
	ATTRIBUTE_SET_GLOVE,
#endif
	ATTRIBUTE_SET_MAX_NUM,
};
#endif

#ifdef ENABLE_REFINE_ELEMENT
enum ERefineElement
{
	REFINE_ELEMENT_MAX = 3,
	ELEMENT_MIN_REFINE_LEVEL = 7,
	REFINE_ELEMENT_UPGRADE_YANG = 3000000,
	REFINE_ELEMENT_DOWNGRADE_YANG = 10000000,
	REFINE_ELEMENT_CHANGE_YANG = 10000000,
	
	REFINE_ELEMENT_TYPE_UPGRADE = 0,
	REFINE_ELEMENT_TYPE_DOWNGRADE = 1,
	REFINE_ELEMENT_TYPE_CHANGE = 2,
	REFINE_ELEMENT_TYPE_UPGRADE_SUCCES = 10,
	REFINE_ELEMENT_TYPE_UPGRADE_FAIL = 11,
	REFINE_ELEMENT_TYPE_DOWNGRADE_SUCCES = 12,
	REFINE_ELEMENT_TYPE_CHANGE_SUCCES = 13,

	REFINE_ELEMENT_RANDOM_VALUE_MIN = 1,
	REFINE_ELEMENT_RANDOM_VALUE_MAX = 8,
	REFINE_ELEMENT_RANDOM_BONUS_VALUE_MIN = 2,
	REFINE_ELEMENT_RANDOM_BONUS_VALUE_MAX = 12,
};
#endif

#ifdef ENABLE_SET_ITEM
enum ESetItemType
{
	SET_ITEM_SET_VALUE_NONE,
	SET_ITEM_SET_VALUE_1,
	SET_ITEM_SET_VALUE_2,
	SET_ITEM_SET_VALUE_3,
	SET_ITEM_SET_VALUE_4,
	SET_ITEM_SET_VALUE_5,
	SET_ITEM_SET_VALUE_MAX
};
#endif

#ifdef ENABLE_AUTO_SYSTEM
enum EAutoSlots
{
	AUTO_SKILL_SLOT_MAX = 12,
	AUTO_POSITINO_SLOT_MAX = 12 + 12 + 1,
};

const int auto_red_potions[] = { 27001, 27002, 27003, 27007, 70390 };
const int auto_blue_potions[] = { 27004, 27005, 27006, 27008, 70391 };

const float AUTO_MAX_FOCUS_DISTANCE = 10000.0f;
const BYTE AUTO_MAX_KILL_SECOND = 10;

typedef struct SAutoSlot
{
	DWORD slotPos;
	DWORD fillingTime;
	int32_t nextUsage;
} TAutoSlot;
#endif

typedef struct SItemPos
{
	BYTE window_type;
	WORD cell;
	SItemPos ()
	{
		window_type = INVENTORY;
		cell = WORD_MAX;
	}
	SItemPos (BYTE _window_type, WORD _cell)
	{
		window_type = _window_type;
		cell = _cell;
	}

	bool IsValidCell()
	{
		switch (window_type)
		{
		case INVENTORY:
			return cell < c_Inventory_Count;
			break;
		case EQUIPMENT:
			return cell < c_DragonSoul_Equip_End;
			break;
		case DRAGON_SOUL_INVENTORY:
			return cell < (DS_INVENTORY_MAX_NUM);
			break;
#ifdef ENABLE_SWITCHBOT
		case SWITCHBOT:
			return cell < SWITCHBOT_SLOT_COUNT;
			break;
#endif
		default:
			return false;
		}
	}
	bool IsEquipCell()
	{
		switch (window_type)
		{
		case INVENTORY:
		case EQUIPMENT:
			return (c_Equipment_Start + c_Wear_Max > cell) && (c_Equipment_Start <= cell);
			break;

		case BELT_INVENTORY:
		case DRAGON_SOUL_INVENTORY:
			return false;
			break;

		default:
			return false;
		}
	}

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
	bool IsBeltInventoryCell()
	{
		bool bResult = c_Belt_Inventory_Slot_Start <= cell && c_Belt_Inventory_Slot_End > cell;
		return bResult;
	}
#endif

#ifdef ENABLE_SPLIT_INVENTORY_SYSTEM
	bool IsSkillBookInventoryCell()
	{
		bool bResult = c_Skill_Book_Inventory_Slot_Start <= cell && c_Skill_Book_Inventory_Slot_End > cell;
		return bResult;
	}
	
	bool IsUpgradeItemsInventoryCell()
	{
		bool bResult = c_Upgrade_Items_Inventory_Slot_Start <= cell && c_Upgrade_Items_Inventory_Slot_End > cell;
		return bResult;
	}
	
	bool IsStoneInventoryCell()
	{
		bool bResult = c_Stone_Inventory_Slot_Start <= cell && c_Stone_Inventory_Slot_End > cell;
		return bResult;
	}
	
	bool IsBoxInventoryCell()
	{
		bool bResult = c_Box_Inventory_Slot_Start <= cell && c_Box_Inventory_Slot_End > cell;
		return bResult;
	}
	
	bool IsEfsunInventoryCell()
	{
		bool bResult = c_Efsun_Inventory_Slot_Start <= cell && c_Efsun_Inventory_Slot_End > cell;
		return bResult;
	}
	
	bool IsCicekInventoryCell()
	{
		bool bResult = c_Cicek_Inventory_Slot_Start <= cell && c_Cicek_Inventory_Slot_End > cell;
		return bResult;
	}
#endif

	bool operator==(const struct SItemPos& rhs) const
	{
		return (window_type == rhs.window_type) && (cell == rhs.cell);
	}

	bool operator<(const struct SItemPos& rhs) const
	{
		return (window_type < rhs.window_type) || ((window_type == rhs.window_type) && (cell < rhs.cell));
	}
} TItemPos;
#pragma pack(pop)

const DWORD c_QuickBar_Line_Count = 3;
const DWORD c_QuickBar_Slot_Count = 12;

const float c_Idle_WaitTime = 5.0f;

const int c_Monster_Race_Start_Number = 6;
const int c_Monster_Model_Start_Number = 20001;

const float c_fAttack_Delay_Time = 0.2f;
const float c_fHit_Delay_Time = 0.1f;
const float c_fCrash_Wave_Time = 0.2f;
const float c_fCrash_Wave_Distance = 3.0f;

const float c_fHeight_Step_Distance = 50.0f;

enum
{
	DISTANCE_TYPE_FOUR_WAY,
	DISTANCE_TYPE_EIGHT_WAY,
	DISTANCE_TYPE_ONE_WAY,
	DISTANCE_TYPE_MAX_NUM,
};

const float c_fMagic_Script_Version = 1.0f;
const float c_fSkill_Script_Version = 1.0f;
const float c_fMagicSoundInformation_Version = 1.0f;
const float c_fBattleCommand_Script_Version = 1.0f;
const float c_fEmotionCommand_Script_Version = 1.0f;
const float c_fActive_Script_Version = 1.0f;
const float c_fPassive_Script_Version = 1.0f;

// Used by PushMove
#ifdef ENABLE_FISH_EVENT_SYSTEM
const float c_fWalkDistance = 175.0f;
const float c_fRunDistance = 310.0f;
#endif

#define FILE_MAX_LEN 128

enum
{
	ITEM_SOCKET_SLOT_MAX_NUM		= 6,
	ITEM_ATTRIBUTE_SLOT_MAX_NUM		= 15,
#ifdef ENABLE_GLOVE_SYSTEM
	ITEM_APPLY_RANDOM_SLOT_MAX_NUM = 4,
#endif
};

#pragma pack(push)
#pragma pack(1)

typedef struct SQuickSlot
{
	BYTE Type;
	UINT Position;
} TQuickSlot;

typedef struct TPlayerItemAttribute
{
	BYTE		bType;
	short		sValue;
#ifdef ENABLE_GLOVE_SYSTEM
	BYTE bPath;
#endif
} TPlayerItemAttribute;

typedef struct packet_item
{
	DWORD		vnum;
#ifdef ENABLE_EXTENDED_ITEM_COUNT
	WORD		count;
#else
	BYTE		count;
#endif
#ifdef ENABLE_SOULBIND_SYSTEM
	int32_t		bind;
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
	DWORD		transmutation;
#endif
#ifdef ENABLE_REFINE_ELEMENT
	DWORD		dwRefineElement;
#endif
#ifdef ENABLE_SET_ITEM
	uint8_t		set_value;
#endif
	DWORD		flags;
	DWORD		anti_flags;
	int32_t		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#ifdef ENABLE_GLOVE_SYSTEM
	TPlayerItemAttribute aApplyRandom[ITEM_APPLY_RANDOM_SLOT_MAX_NUM];
#endif
} TItemData;

typedef struct packet_shop_item
{
	DWORD		vnum;
	DWORD		price;
#ifdef ENABLE_CHEQUE_SYSTEM
	DWORD		price_cheque;
#endif
#ifdef ENABLE_BUY_WITH_ITEM
	DWORD		witemVnum;
#endif
#ifdef ENABLE_EXTENDED_ITEM_COUNT
	WORD		count;
#else
	BYTE		count;
#endif
	BYTE		display_pos;
	int32_t		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#ifdef ENABLE_GLOVE_SYSTEM
	TPlayerItemAttribute aApplyRandom[ITEM_APPLY_RANDOM_SLOT_MAX_NUM];
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
	DWORD		transmutation;
#endif
#ifdef ENABLE_REFINE_ELEMENT
	DWORD		dwRefineElement;
#endif
#ifdef ENABLE_12ZI
	DWORD		getLimitedCount;
	DWORD		getLimitedPurchaseCount;
#endif
#ifdef ENABLE_CHEQUE_DESK_SYSTEM
	DWORD		getMyStok;
#endif
} TShopItemData;
#ifdef ENABLE_BALATHOR_DUNGEON
enum
{
	BALATHOR_EFF_AREA = 0,
	BALATHOR_EFF_GROUND_DROP,
	BALATHOR_EFF_GROUND_ICE,
	BALATHOR_EFF_EGG_ARROW,
	BALATHOR_EFF_EGG_FLOOD,
	BALATHOR_EFF_EGG_GUARD,
	BALATHOR_EFF_EGG_MOOJUK,
	BALATHOR_EFF_RED_ARROW,
	BALATHOR_EFF_POSITION,
	BALATHOR_EFF_MAX,
};
typedef struct SBalathorEffect
{
	DWORD dwID;
	DWORD dwVID;
	BYTE bEffIndex;
	float fX;
	float fY;
	float fZ;
	float fRotation;
	DWORD dwDuration;
	bool bIsMain;
	DWORD dwEffectIndex;
}TBalathorEffect;
#endif

#pragma pack(pop)

inline float GetSqrtDistance(int ix1, int iy1, int ix2, int iy2) // By sqrt
{
	float dx, dy;

	dx = float(ix1 - ix2);
	dy = float(iy1 - iy2);

	return sqrtf(dx*dx + dy*dy);
}

// DEFAULT_FONT
void DefaultFont_Startup();
void DefaultFont_Cleanup();
void DefaultFont_SetName(const char * c_szFontName);
CResource* DefaultFont_GetResource();
#ifdef ENABLE_FISH_EVENT_SYSTEM
CResource* DefaultItalicFont_GetResource();
#endif
// END_OF_DEFAULT_FONT

void SetGuildSymbolPath(const char * c_szPathName);
const char * GetGuildSymbolFileName(DWORD dwGuildID);
BYTE SlotTypeToInvenType(BYTE bSlotType);
BYTE ApplyTypeToPointType(BYTE bApplyType);
#ifdef ENABLE_FLOWER_EVENT
uint16_t PointTypeToApplyType(uint16_t wPointType);	//@fixme436
#endif
