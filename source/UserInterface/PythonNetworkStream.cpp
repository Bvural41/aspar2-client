#include "StdAfx.h"
#include "../EterLib/NetPacketHeaderMap.h"

#ifdef __ANDROID__
#include <android/log.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "Metin2Mobile", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "Metin2Mobile", __VA_ARGS__)
#endif

#include "PythonNetworkStream.h"
#include "AbstractApplication.h"
#include "Packet.h"
#include "NetworkActorManager.h"
#include "GuildMarkDownloader.h"
#include "GuildMarkUploader.h"
#include "MarkManager.h"
#include "ProcessCRC.h"
#ifdef ENABLE_BATTLEPASS
#include "BattlePassUtils.hpp"
#endif

// MARK_BUG_FIX
static DWORD gs_nextDownloadMarkTime = 0;
// END_OF_MARK_BUG_FIX

// Packet ---------------------------------------------------------------------------
class CMainPacketHeaderMap : public CNetworkPacketHeaderMap
{
	public:
		enum
		{
			STATIC_SIZE_PACKET = false,
			DYNAMIC_SIZE_PACKET = true,
		};

	public:
		CMainPacketHeaderMap()
		{
			Set(HEADER_GC_EMPIRE,				CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCEmpire), STATIC_SIZE_PACKET));
			Set(HEADER_GC_WARP,					CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCWarp), STATIC_SIZE_PACKET));
			Set(HEADER_GC_SKILL_COOLTIME_END,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSkillCoolTimeEnd), STATIC_SIZE_PACKET));
			Set(HEADER_GC_QUEST_INFO,			CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCQuestInfo), DYNAMIC_SIZE_PACKET));
			Set(HEADER_GC_REQUEST_MAKE_GUILD,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCBlank), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PVP,					CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPVP), STATIC_SIZE_PACKET));
			Set(HEADER_GC_DUEL_START,			CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCDuelStart), DYNAMIC_SIZE_PACKET));
			Set(HEADER_GC_CHARACTER_ADD,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCCharacterAdd), STATIC_SIZE_PACKET));
			Set(HEADER_GC_CHAR_ADDITIONAL_INFO,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCCharacterAdditionalInfo), STATIC_SIZE_PACKET));

			Set(HEADER_GC_CHARACTER_UPDATE,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCCharacterUpdate), STATIC_SIZE_PACKET));

			Set(HEADER_GC_CHARACTER_DEL,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCCharacterDelete), STATIC_SIZE_PACKET));
			Set(HEADER_GC_CHARACTER_MOVE,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMove), STATIC_SIZE_PACKET));
			Set(HEADER_GC_CHAT,					CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCChat), DYNAMIC_SIZE_PACKET));

			Set(HEADER_GC_SYNC_POSITION,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCC2C), DYNAMIC_SIZE_PACKET));

			Set(HEADER_GC_LOGIN_SUCCESS3,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCLoginSuccess3), STATIC_SIZE_PACKET));
			Set(HEADER_GC_LOGIN_SUCCESS4,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCLoginSuccess4), STATIC_SIZE_PACKET));
			Set(HEADER_GC_LOGIN_FAILURE,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCLoginFailure), STATIC_SIZE_PACKET));

			Set(HEADER_GC_PLAYER_CREATE_SUCCESS,		 CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPlayerCreateSuccess), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PLAYER_CREATE_FAILURE,		 CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCCreateFailure), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PLAYER_DELETE_SUCCESS,		 CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCBlank), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PLAYER_DELETE_WRONG_SOCIAL_ID, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCBlank), STATIC_SIZE_PACKET));

			Set(HEADER_GC_STUN,					CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCStun), STATIC_SIZE_PACKET));
			Set(HEADER_GC_DEAD,					CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCDead), STATIC_SIZE_PACKET));
			
			Set(HEADER_GC_MAIN_CHARACTER,			CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMainCharacter), STATIC_SIZE_PACKET));

			// SUPPORT_BGM
			Set(HEADER_GC_MAIN_CHARACTER2_EMPIRE,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMainCharacter2_EMPIRE), STATIC_SIZE_PACKET));
			Set(HEADER_GC_MAIN_CHARACTER3_BGM,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMainCharacter3_BGM), STATIC_SIZE_PACKET));
			Set(HEADER_GC_MAIN_CHARACTER4_BGM_VOL,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMainCharacter4_BGM_VOL), STATIC_SIZE_PACKET));
			// END_OFSUPPORT_BGM

			Set(HEADER_GC_PLAYER_POINTS,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPoints), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PLAYER_POINT_CHANGE,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPointChange), STATIC_SIZE_PACKET));

			Set(HEADER_GC_ITEM_SET,				CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemSet), STATIC_SIZE_PACKET));
			Set(HEADER_GC_ITEM_SET2,			CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemSet2), STATIC_SIZE_PACKET));

			Set(HEADER_GC_ITEM_USE,				CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemUse), STATIC_SIZE_PACKET));
			Set(HEADER_GC_ITEM_UPDATE,			CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemUpdate), STATIC_SIZE_PACKET));

			Set(HEADER_GC_ITEM_GROUND_ADD,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemGroundAdd), STATIC_SIZE_PACKET));
			Set(HEADER_GC_ITEM_GROUND_DEL,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemGroundDel), STATIC_SIZE_PACKET));
			Set(HEADER_GC_ITEM_OWNERSHIP,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemOwnership), STATIC_SIZE_PACKET));

			Set(HEADER_GC_QUICKSLOT_ADD,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCQuickSlotAdd), STATIC_SIZE_PACKET));
			Set(HEADER_GC_QUICKSLOT_DEL,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCQuickSlotDel), STATIC_SIZE_PACKET));
			Set(HEADER_GC_QUICKSLOT_SWAP,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCQuickSlotSwap), STATIC_SIZE_PACKET));

			Set(HEADER_GC_WHISPER,				CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCWhisper), STATIC_SIZE_PACKET));

			Set(HEADER_GC_CHARACTER_POSITION,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPosition), STATIC_SIZE_PACKET));
			Set(HEADER_GC_MOTION,				CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMotion), STATIC_SIZE_PACKET));

			Set(HEADER_GC_SHOP,			CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCShop), DYNAMIC_SIZE_PACKET));
			Set(HEADER_GC_SHOP_SIGN,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCShopSign), STATIC_SIZE_PACKET));
			Set(HEADER_GC_EXCHANGE,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCExchange), STATIC_SIZE_PACKET));

			Set(HEADER_GC_PING,			CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPing), STATIC_SIZE_PACKET));

			Set(HEADER_GC_SCRIPT,			CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCScript), DYNAMIC_SIZE_PACKET));
			Set(HEADER_GC_QUEST_CONFIRM,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCQuestConfirm), STATIC_SIZE_PACKET));

			Set(HEADER_GC_TARGET,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCTarget), STATIC_SIZE_PACKET));

			Set(HEADER_GC_MOUNT,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMount), STATIC_SIZE_PACKET));

			Set(HEADER_GC_CHANGE_SPEED,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCChangeSpeed), STATIC_SIZE_PACKET));

			Set(HEADER_GC_HANDSHAKE,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCHandshake), STATIC_SIZE_PACKET));
			Set(HEADER_GC_HANDSHAKE_OK,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCBlank), STATIC_SIZE_PACKET));
			Set(HEADER_GC_BINDUDP,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCBindUDP), STATIC_SIZE_PACKET));
			Set(HEADER_GC_OWNERSHIP,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCOwnership), STATIC_SIZE_PACKET));
			Set(HEADER_GC_CREATE_FLY,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCCreateFly), STATIC_SIZE_PACKET));

			Set(HEADER_GC_ADD_FLY_TARGETING, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCFlyTargeting), STATIC_SIZE_PACKET));
			Set(HEADER_GC_FLY_TARGETING, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCFlyTargeting), STATIC_SIZE_PACKET));

			Set(HEADER_GC_PHASE,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPhase), STATIC_SIZE_PACKET));
			Set(HEADER_GC_SKILL_LEVEL,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSkillLevel), STATIC_SIZE_PACKET));
			Set(HEADER_GC_SKILL_LEVEL_NEW,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSkillLevelNew), STATIC_SIZE_PACKET));

			Set(HEADER_GC_MESSENGER,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMessenger), DYNAMIC_SIZE_PACKET));
			Set(HEADER_GC_GUILD,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCGuild), DYNAMIC_SIZE_PACKET));

			Set(HEADER_GC_PARTY_INVITE,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPartyInvite), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PARTY_ADD,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPartyAdd), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PARTY_UPDATE,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPartyUpdate), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PARTY_REMOVE,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPartyRemove), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PARTY_LINK,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPartyLink), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PARTY_UNLINK,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPartyUnlink), STATIC_SIZE_PACKET));
			Set(HEADER_GC_PARTY_PARAMETER,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPartyParameter), STATIC_SIZE_PACKET));

			Set(HEADER_GC_SAFEBOX_SET,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemSet), STATIC_SIZE_PACKET));
			Set(HEADER_GC_SAFEBOX_DEL,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemDel), STATIC_SIZE_PACKET));
			Set(HEADER_GC_SAFEBOX_WRONG_PASSWORD,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSafeboxWrongPassword), STATIC_SIZE_PACKET));
			Set(HEADER_GC_SAFEBOX_SIZE,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSafeboxSize), STATIC_SIZE_PACKET));
			Set(HEADER_GC_SAFEBOX_MONEY_CHANGE,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSafeboxMoneyChange), STATIC_SIZE_PACKET));

			Set(HEADER_GC_FISHING,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCFishing), STATIC_SIZE_PACKET));
			Set(HEADER_GC_DUNGEON, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCDungeon), DYNAMIC_SIZE_PACKET));

			Set(HEADER_GC_TIME, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCTime), STATIC_SIZE_PACKET));
			Set(HEADER_GC_WALK_MODE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCWalkMode), STATIC_SIZE_PACKET));
			Set(HEADER_GC_CHANGE_SKILL_GROUP, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCChangeSkillGroup), STATIC_SIZE_PACKET));
			Set(HEADER_GC_REFINE_INFORMATION, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCRefineInformation), STATIC_SIZE_PACKET));
			Set(HEADER_GC_REFINE_INFORMATION_NEW, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCRefineInformationNew), STATIC_SIZE_PACKET));
			Set(HEADER_GC_OBSERVER_ADD, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCObserverAdd), STATIC_SIZE_PACKET));
			Set(HEADER_GC_OBSERVER_REMOVE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCObserverRemove), STATIC_SIZE_PACKET));
			Set(HEADER_GC_OBSERVER_MOVE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCObserverMove), STATIC_SIZE_PACKET));
			Set(HEADER_GC_SEPCIAL_EFFECT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSpecialEffect), STATIC_SIZE_PACKET));
			Set(HEADER_GC_NPC_POSITION, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCNPCPosition), DYNAMIC_SIZE_PACKET));
			Set(HEADER_GC_CHANGE_NAME, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCChangeName), STATIC_SIZE_PACKET));

			Set(HEADER_GC_CHINA_MATRIX_CARD, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCChinaMatrixCard), STATIC_SIZE_PACKET));
			Set(HEADER_GC_RUNUP_MATRIX_QUIZ, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCRunupMatrixQuiz), STATIC_SIZE_PACKET));
			Set(HEADER_GC_NEWCIBN_PASSPOD_REQUEST, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCNEWCIBNPasspodRequest), STATIC_SIZE_PACKET));
			Set(HEADER_GC_NEWCIBN_PASSPOD_FAILURE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCNEWCIBNPasspodFailure), STATIC_SIZE_PACKET));
			Set(HEADER_GC_LOGIN_KEY, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCLoginKey), STATIC_SIZE_PACKET));

			Set(HEADER_GC_AUTH_SUCCESS, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCAuthSuccess), STATIC_SIZE_PACKET));
			Set(HEADER_GC_AUTH_SUCCESS_OPENID, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCAuthSuccessOpenID), STATIC_SIZE_PACKET));
			Set(HEADER_GC_CHANNEL, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCChannel), STATIC_SIZE_PACKET));
			Set(HEADER_GC_LAND_LIST, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCLandList), DYNAMIC_SIZE_PACKET));

			Set(HEADER_GC_TARGET_UPDATE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCTargetUpdate), STATIC_SIZE_PACKET));
			Set(HEADER_GC_TARGET_DELETE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCTargetDelete), STATIC_SIZE_PACKET));
			Set(HEADER_GC_TARGET_CREATE_NEW, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCTargetCreateNew), STATIC_SIZE_PACKET));

			Set(HEADER_GC_AFFECT_ADD, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCAffectAdd), STATIC_SIZE_PACKET));
			Set(HEADER_GC_AFFECT_REMOVE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCAffectRemove), STATIC_SIZE_PACKET));

			Set(HEADER_GC_MALL_OPEN, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMallOpen), STATIC_SIZE_PACKET));
			Set(HEADER_GC_MALL_SET, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemSet), STATIC_SIZE_PACKET));
			Set(HEADER_GC_MALL_DEL, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCItemDel), STATIC_SIZE_PACKET));

			Set(HEADER_GC_LOVER_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCLoverInfo), STATIC_SIZE_PACKET));
			Set(HEADER_GC_LOVE_POINT_UPDATE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCLovePointUpdate), STATIC_SIZE_PACKET));

			Set(HEADER_GC_DIG_MOTION, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCDigMotion), STATIC_SIZE_PACKET));
			Set(HEADER_GC_DAMAGE_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCDamageInfo), STATIC_SIZE_PACKET));

			Set(HEADER_GC_HYBRIDCRYPT_KEYS, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCHybridCryptKeys), DYNAMIC_SIZE_PACKET));
			Set(HEADER_GC_HYBRIDCRYPT_SDB, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCHybridSDB), DYNAMIC_SIZE_PACKET));
			Set(HEADER_GC_SPECIFIC_EFFECT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSpecificEffect), STATIC_SIZE_PACKET));
			Set(HEADER_GC_DRAGON_SOUL_REFINE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCDragonSoulRefine), STATIC_SIZE_PACKET));
#ifdef ENABLE_TREASURE_EVENT
			Set(HEADER_GC_TREASURE_EVENT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCTreasureEvent), DYNAMIC_SIZE_PACKET));
#endif
#ifdef ENABLE_BATTLE_ROYALE
			Set(HEADER_GC_BATTLE_ROYALE,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCBattleRoyale), DYNAMIC_SIZE_PACKET));
#endif
#ifdef ENABLE_BATTLEPASS
			Set(net_battlepass::HEADER_GC_BATTLEPASS, TPacketType(sizeof(net_battlepass::packet_battlepass), DYNAMIC_SIZE_PACKET));
#endif
#ifdef ENABLE_FISH_EVENT_SYSTEM
			Set(HEADER_GC_FISH_EVENT_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCFishEventInfo), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_DS_SET
			Set(HEADER_GC_DS_TABLE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketDSTable), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_ATTR_6TH_7TH_SYSTEM
			Set(HEADER_GC_OPEN_67_ATTR, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacket67AttrOpenClose), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_KILL_STATISTICS
			Set(HEADER_GC_KILL_STATISTICS, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCKillStatistics), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_REFINE_ELEMENT
			Set(HEADER_GC_REFINE_ELEMENT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCRefineElement), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_SOUL_ROULETTE_SYSTEM
			Set(HEADER_GC_SOUL_ROULETTE,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSoulRoulette), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_SWITCHBOT
			Set(HEADER_GC_SWITCHBOT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSwitchbot), DYNAMIC_SIZE_PACKET));
#endif
#ifdef ENABLE_MAILBOX_SYSTEM
			Set(HEADER_GC_MAILBOX_RECEIVE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMailBoxReceive), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_VIEW_EQUIPMENT_SYSTEM
			Set(HEADER_GC_VIEW_EQUIP, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCViewEquip), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_TARGET_INFORMATION_SYSTEM
			Set(HEADER_GC_TARGET_INFO,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCTargetInfo), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_BATTLE_FIELD
			Set(HEADER_GC_RANKING_COMBAT_ZONE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCCombatZoneRankingData), STATIC_SIZE_PACKET));
			Set(HEADER_GC_SEND_COMBAT_ZONE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSendCombatZone), STATIC_SIZE_PACKET));
#endif
#ifdef _IMPROVED_PACKET_ENCRYPTION_
			Set(HEADER_GC_KEY_AGREEMENT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketKeyAgreement), STATIC_SIZE_PACKET));
			Set(HEADER_GC_KEY_AGREEMENT_COMPLETED, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketKeyAgreementCompleted), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_MINI_GAME_CATCH_KING
			Set(HEADER_GC_MINI_GAME_CATCH_KING, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMiniGameCatchKing), DYNAMIC_SIZE_PACKET));
#endif
#ifdef ENABLE_SHOW_CHEST_DROP
			Set(HEADER_GC_CHEST_DROP_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCChestDropInfo), DYNAMIC_SIZE_PACKET));
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
			Set(HEADER_GC_CL, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketChangeLook), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_GIFTBOX_MULTI_OPEN
			Set(HEADER_GC_REFRESH_INVENTORY, CNetworkPacketHeaderMap::TPacketType(sizeof(BYTE), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_SASH_SYSTEM
			Set(HEADER_GC_SASH, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketSash), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_SUPPORT_SYSTEM
			Set(HEADER_GC_SUPPORT_SHAMAN_SKILL, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSupportShamanSkill), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_EVENT_SYSTEM
			Set(HEADER_GC_EVENT_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCEventInfo), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_12ZI
			Set(HEADER_GC_SEPCIAL_ZODIAC_EFFECT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSpecialZodiacEffect), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_EXTEND_INVEN_SYSTEM
			Set(HEADER_GC_EXTEND_INVENTORY,					CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCExtendInventory),			false));
#endif
#ifdef ENABLE_AURA_SYSTEM
			Set(HEADER_GC_AURA, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketAura), false));
#endif
#ifdef ENABLE_CUBE_RENEWAL
			Set(HEADER_GC_CUBE_RENEWAL,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCCubeRenewalReceive), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
			Set(HEADER_GC_REQUEST_CHANGE_LANGUAGE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketChangeLanguage), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_MULTI_LANGUAGE_WHISPER_DETAILS
		Set(HEADER_GC_WHISPER_DETAILS, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCWhisperDetails), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_OFFLINESHOP_SYSTEM
			Set(HEADER_GC_OFFLINE_SHOP, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCShop), DYNAMIC_SIZE_PACKET));
			Set(HEADER_GC_OFFLINE_SHOP_SIGN, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCShopSign), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_SHOP_SEARCH_SYSTEM
			Set(HEADER_GC_SHOPSEARCH_SET, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCShopSearchItemSet), DYNAMIC_SIZE_PACKET));
#endif
#ifdef ENABLE_RIDING_EXTENDED
			Set(HEADER_GC_MOUNT_UP_GRADE,		CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMountUpGrade), STATIC_SIZE_PACKET));
			Set(HEADER_GC_MOUNT_UP_GRADE_CHAT,	CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMountUpGradeChat), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
			Set(HEADER_GC_GROWTH_PET_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TGrowthPetInfoPacket), STATIC_SIZE_PACKET));
#endif
#if defined(ENABLE_CLIENT_TIMER)
			Set(HEADER_GC_CLIENT_TIMER, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCClientTimer), STATIC_SIZE_PACKET));
#endif
#ifdef ENABLE_SELECT_REWARD_BOX
			Set(HEADER_GC_REWARD_INFO, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSelectRewardInfo), DYNAMIC_SIZE_PACKET));
			Set(HEADER_GC_REWARD_RESULT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCSelectRewardResult)));
#endif
#ifdef ENABLE_TITLE_SYSTEM
			Set(HEADER_GC_TITLE_DATA, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCTitleData), DYNAMIC_SIZE_PACKET));
			Set(HEADER_GC_TITLE_UPDATE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCTitleUpdate), STATIC_SIZE_PACKET));
#endif
		}
};

int g_iLastPacket[2] = { 0, 0 };

void CPythonNetworkStream::ExitApplication()
{
	if (__IsNotPing())
	{
		AbsoluteExitApplication();
	}
	else
	{
		SendChatPacket("/quit");
	}
}

void CPythonNetworkStream::ExitGame()
{
	if (__IsNotPing())
	{
		LogOutGame();
	}
	else
	{
		SendChatPacket("/phase_select");
	}
}


void CPythonNetworkStream::LogOutGame()
{
	if (__IsNotPing())
	{		
		AbsoluteExitGame();
	}	
	else
	{
		SendChatPacket("/logout");
	}
}

void CPythonNetworkStream::AbsoluteExitGame()
{
	if (!IsOnline())
		return;

	OnRemoteDisconnect();		
	Disconnect();
}

void CPythonNetworkStream::AbsoluteExitApplication()
{
	IAbstractApplication::GetSingleton().Exit();
}

bool CPythonNetworkStream::__IsNotPing()
{
	return false;
}

DWORD CPythonNetworkStream::GetGuildID()
{
	return m_dwGuildID;
}

UINT CPythonNetworkStream::UploadMark(const char * c_szImageFileName)
{
	if (0 == m_dwGuildID)
		return ERROR_MARK_UPLOAD_NEED_RECONNECT;

	gs_nextDownloadMarkTime = 0;

	UINT uError=ERROR_UNKNOWN;
	CGuildMarkUploader& rkGuildMarkUploader=CGuildMarkUploader::Instance();
	if (!rkGuildMarkUploader.Connect(m_kMarkAuth.m_kNetAddr, m_kMarkAuth.m_dwHandle, m_kMarkAuth.m_dwRandomKey, m_dwGuildID, c_szImageFileName, &uError))
	{
		switch (uError)
		{
			case CGuildMarkUploader::ERROR_CONNECT:
				return ERROR_CONNECT_MARK_SERVER;
				break;
			case CGuildMarkUploader::ERROR_LOAD:
				return ERROR_LOAD_MARK;
				break;
			case CGuildMarkUploader::ERROR_WIDTH:
				return ERROR_MARK_WIDTH;
				break;
			case CGuildMarkUploader::ERROR_HEIGHT:
				return ERROR_MARK_HEIGHT;
				break;
			default:
				return ERROR_UNKNOWN;
		}
	}

	__DownloadMark();
	
	if (CGuildMarkManager::INVALID_MARK_ID == CGuildMarkManager::Instance().GetMarkID(m_dwGuildID))
		return ERROR_MARK_CHECK_NEED_RECONNECT;

	return ERROR_NONE;
}

UINT CPythonNetworkStream::UploadSymbol(const char* c_szImageFileName)
{
	UINT uError=ERROR_UNKNOWN;
	CGuildMarkUploader& rkGuildMarkUploader=CGuildMarkUploader::Instance();
	if (!rkGuildMarkUploader.ConnectToSendSymbol(m_kMarkAuth.m_kNetAddr, m_kMarkAuth.m_dwHandle, m_kMarkAuth.m_dwRandomKey, m_dwGuildID, c_szImageFileName, &uError))
	{
		switch (uError)
		{
			case CGuildMarkUploader::ERROR_CONNECT:
				return ERROR_CONNECT_MARK_SERVER;
				break;
			case CGuildMarkUploader::ERROR_LOAD:
				return ERROR_LOAD_MARK;
				break;
			case CGuildMarkUploader::ERROR_WIDTH:
				return ERROR_MARK_WIDTH;
				break;
			case CGuildMarkUploader::ERROR_HEIGHT:
				return ERROR_MARK_HEIGHT;
				break;
			default:
				return ERROR_UNKNOWN;
		}
	}

	return ERROR_NONE;
}

void CPythonNetworkStream::__DownloadMark()
{
	DWORD curTime = ELTimer_GetMSec();

	if (curTime < gs_nextDownloadMarkTime)
		return;

	gs_nextDownloadMarkTime = curTime + 60000 * 3; // 3분

	CGuildMarkDownloader& rkGuildMarkDownloader = CGuildMarkDownloader::Instance();
	rkGuildMarkDownloader.Connect(m_kMarkAuth.m_kNetAddr, m_kMarkAuth.m_dwHandle, m_kMarkAuth.m_dwRandomKey);
}

void CPythonNetworkStream::__DownloadSymbol(const std::vector<DWORD> & c_rkVec_dwGuildID)
{
	CGuildMarkDownloader& rkGuildMarkDownloader=CGuildMarkDownloader::Instance();
	rkGuildMarkDownloader.ConnectToRecvSymbol(m_kMarkAuth.m_kNetAddr, m_kMarkAuth.m_dwHandle, m_kMarkAuth.m_dwRandomKey, c_rkVec_dwGuildID);
}

void CPythonNetworkStream::SetPhaseWindow(UINT ePhaseWnd, PyObject* poPhaseWnd)
{
	if (ePhaseWnd>=PHASE_WINDOW_NUM)
		return;

	m_apoPhaseWnd[ePhaseWnd]=poPhaseWnd;
}

void CPythonNetworkStream::ClearPhaseWindow(UINT ePhaseWnd, PyObject* poPhaseWnd)
{
	if (ePhaseWnd>=PHASE_WINDOW_NUM)
		return;

	if (poPhaseWnd != m_apoPhaseWnd[ePhaseWnd])
		return;

	m_apoPhaseWnd[ePhaseWnd]=0;
}

void CPythonNetworkStream::SetServerCommandParserWindow(PyObject* poWnd)
{
	m_poSerCommandParserWnd = poWnd;
}

bool CPythonNetworkStream::IsSelectedEmpire()
{
	if (m_dwEmpireID)
		return true;
	
	return false;
}

UINT CPythonNetworkStream::GetAccountCharacterSlotDatau(UINT iSlot, UINT eType)
{
	if (iSlot >= PLAYER_PER_ACCOUNT4)
		return 0;
		
	TSimplePlayerInformation&	rkSimplePlayerInfo=m_akSimplePlayerInfo[iSlot];
	
	switch (eType)
	{
		case ACCOUNT_CHARACTER_SLOT_ID:
			return rkSimplePlayerInfo.dwID;
		case ACCOUNT_CHARACTER_SLOT_RACE:
			return rkSimplePlayerInfo.byJob;
		case ACCOUNT_CHARACTER_SLOT_LEVEL:
			return rkSimplePlayerInfo.byLevel;
		case ACCOUNT_CHARACTER_SLOT_STR:
			return rkSimplePlayerInfo.byST;
		case ACCOUNT_CHARACTER_SLOT_DEX:
			return rkSimplePlayerInfo.byDX;
		case ACCOUNT_CHARACTER_SLOT_HTH:
			return rkSimplePlayerInfo.byHT;
		case ACCOUNT_CHARACTER_SLOT_INT:
			return rkSimplePlayerInfo.byIQ;
		case ACCOUNT_CHARACTER_SLOT_PLAYTIME:
			return rkSimplePlayerInfo.dwPlayMinutes;
		case ACCOUNT_CHARACTER_SLOT_FORM:
			return rkSimplePlayerInfo.wMainPart;
		case ACCOUNT_CHARACTER_SLOT_PORT:
			return rkSimplePlayerInfo.wPort;
		case ACCOUNT_CHARACTER_SLOT_GUILD_ID:
			return m_adwGuildID[iSlot];
			break;
		case ACCOUNT_CHARACTER_SLOT_CHANGE_NAME_FLAG:
			return rkSimplePlayerInfo.bChangeName;
			break;
		case ACCOUNT_CHARACTER_SLOT_HAIR:
			return rkSimplePlayerInfo.wHairPart;
			break;
#ifdef ENABLE_SASH_SYSTEM
		case ACCOUNT_CHARACTER_SLOT_SASH:
			return rkSimplePlayerInfo.wSashPart;
			break;
#endif
#ifdef ENABLE_AURA_SYSTEM
		case ACCOUNT_CHARACTER_SLOT_AURA:
			return rkSimplePlayerInfo.wAuraPart;
			break;
#endif
		case ACCOUNT_CHARACTER_SLOT_CONQUEROR_LEVEL:
			return rkSimplePlayerInfo.byConquerorLevel;
			break;
		case ACCOUNT_CHARACTER_SLOT_SUNGMA_ST:
			return rkSimplePlayerInfo.bySungmaST;
			break;
		case ACCOUNT_CHARACTER_SLOT_SUNGMA_HP:
			return rkSimplePlayerInfo.bySungmaHP;
			break;
		case ACCOUNT_CHARACTER_SLOT_SUNGMA_MOVE:
			return rkSimplePlayerInfo.bySungmaMV;
			break;
		case ACCOUNT_CHARACTER_SLOT_SUNGMA_INMUNE:
			return rkSimplePlayerInfo.bySungmaINM;
			break;

	}
	return 0;
}

const char* CPythonNetworkStream::GetAccountCharacterSlotDataz(UINT iSlot, UINT eType)
{
	static const char* sc_szEmpty="";

	if (iSlot >= PLAYER_PER_ACCOUNT4)
		return sc_szEmpty;
		
	TSimplePlayerInformation&	rkSimplePlayerInfo=m_akSimplePlayerInfo[iSlot];
	
	switch (eType)
	{
		case ACCOUNT_CHARACTER_SLOT_ADDR:
			{				
				BYTE ip[4];

				const int LEN = 4;
				for (int i = 0; i < LEN; i++)
				{
					ip[i] = BYTE(rkSimplePlayerInfo.lAddr&0xff);
					rkSimplePlayerInfo.lAddr>>=8;
				}


				static char s_szAddr[256];
				sprintf(s_szAddr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
				return s_szAddr;
			}
			break;
		case ACCOUNT_CHARACTER_SLOT_NAME:
			return rkSimplePlayerInfo.szName;
			break;		
		case ACCOUNT_CHARACTER_SLOT_GUILD_NAME:
			return m_astrGuildName[iSlot].c_str();
			break;
	}
	return sc_szEmpty;
}

void CPythonNetworkStream::ConnectLoginServer(const char* c_szAddr, UINT uPort)
{
	CNetworkStream::Connect(c_szAddr, uPort);		
}

void CPythonNetworkStream::SetMarkServer(const char* c_szAddr, UINT uPort)
{
	m_kMarkAuth.m_kNetAddr.Set(c_szAddr, uPort);
}

void CPythonNetworkStream::ConnectGameServer(UINT iChrSlot)
{
	if (iChrSlot >= PLAYER_PER_ACCOUNT4)
		return;

	m_dwSelectedCharacterIndex = iChrSlot;

	__DirectEnterMode_Set(iChrSlot);

	TSimplePlayerInformation&	rkSimplePlayerInfo=m_akSimplePlayerInfo[iChrSlot];	
	CNetworkStream::Connect((DWORD)rkSimplePlayerInfo.lAddr, rkSimplePlayerInfo.wPort);
}

void CPythonNetworkStream::SetLoginInfo(const char* c_szID, const char* c_szPassword)
{
	m_stID=c_szID;
	m_stPassword=c_szPassword;
}

void CPythonNetworkStream::ClearLoginInfo( void )
{
	m_stPassword = "";
}

void CPythonNetworkStream::SetLoginKey(DWORD dwLoginKey)
{
	m_dwLoginKey = dwLoginKey;
}

bool CPythonNetworkStream::CheckPacket(TPacketHeader * pRetHeader)
{
	*pRetHeader = 0;

	static CMainPacketHeaderMap s_packetHeaderMap;

	TPacketHeader header;

	if (!Peek(sizeof(TPacketHeader), &header))
		return false;

	if (0 == header)
	{
		if (!Recv(sizeof(TPacketHeader), &header))
			return false;
		
		while (Peek(sizeof(TPacketHeader), &header))
		{
			if (0 == header)
			{
				if (!Recv(sizeof(TPacketHeader), &header))
					return false;
			}
			else
			{
				break;
			}
		}

		if (0 == header)
			return false;
	}

	CNetworkPacketHeaderMap::TPacketType PacketType;

	if (!s_packetHeaderMap.Get(header, &PacketType))
	{
		TraceError("Unknown packet header: %d, last: %d %d", header, g_iLastPacket[0], g_iLastPacket[1]);
#ifdef ENABLE_PACKET_DESYNC_LOG
		DumpPacketDesync(header, m_strPhase.c_str());
#endif
		ClearRecvBuffer();

		PostQuitMessage(0);
		return false;
	}

	// Code for dynamic size packet
	if (PacketType.isDynamicSizePacket)
	{
		TDynamicSizePacketHeader DynamicSizePacketHeader;

		if (!Peek(sizeof(TDynamicSizePacketHeader), &DynamicSizePacketHeader))
			return false;

		if (!Peek(DynamicSizePacketHeader.size))
		{
			Tracef("CPythonNetworkStream::CheckPacket - Not enough dynamic packet size: header %d packet size: %d\n", 
				DynamicSizePacketHeader.header,
				DynamicSizePacketHeader.size);
			return false;
		}
	}
	else
	{
		if (!Peek(PacketType.iPacketSize))
		{
			return false;
		}
	}

	if (!header)
		return false;

	*pRetHeader = header;	

	g_iLastPacket[0] = g_iLastPacket[1];
	g_iLastPacket[1] = header;
#ifdef ENABLE_PACKET_DESYNC_LOG
	int packetSize = PacketType.isDynamicSizePacket ? 0 : PacketType.iPacketSize;
	LogPacketTrace(header, packetSize, NULL);
#endif
	//Tracenf("header %d size %d", header, PacketType.iPacketSize);
	//Tracenf("header %d size %d outputpos[%d] security %u", header, PacketType.iPacketSize, m_recvBufOutputPos, IsSecurityMode());
	return true;
}

#ifdef ENABLE_PACKET_DESYNC_LOG
void CPythonNetworkStream::LogPacketTrace(BYTE bHeader, int iSize, const void* pData)
{
	m_iPacketHistoryIndex = (m_iPacketHistoryIndex + 1) % 64;
	SPacketTraceEntry& entry = m_aPacketHistory[m_iPacketHistoryIndex];
	entry.dwTime = ELTimer_GetMSec();
	entry.bHeader = bHeader;
	entry.iSize = iSize;
	memset(entry.abPreview, 0, sizeof(entry.abPreview));
	if (pData && iSize > 0)
	{
		int copySize = (iSize < 16) ? iSize : 16;
		memcpy(entry.abPreview, pData, copySize);
	}
}

void CPythonNetworkStream::DumpPacketDesync(int iBadHeader, const char* szPhase)
{
	FILE* fp = fopen("packet_desync_debug.log", "a+");
	if (fp)
	{
		time_t rawtime;
		time(&rawtime);
		struct tm* timeinfo = localtime(&rawtime);
		char timeStr[64];
		if (timeinfo)
			strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);
		else
			strcpy(timeStr, "UNKNOWN_TIME");

		fprintf(fp, "====================================================================\n");
		fprintf(fp, "[%s] PACKET DESYNC DETECTED! Phase: %s | Bad Header: %d (0x%02X)\n",
				timeStr, szPhase ? szPhase : "UNKNOWN", iBadHeader, (BYTE)iBadHeader);
		fprintf(fp, "Recv Buffer Left Size: %d bytes\n", GetRecvBufferSize());
		fprintf(fp, "--- RECV PACKET HISTORY (Last 32 Packets, Oldest -> Newest) ---\n");

		for (int i = 0; i < 32; ++i)
		{
			int idx = (m_iPacketHistoryIndex - 31 + i + 64) % 64;
			const SPacketTraceEntry& e = m_aPacketHistory[idx];
			if (e.bHeader != 0 || e.iSize != 0)
			{
				fprintf(fp, "[%02d] Time: %u ms | Header: %3d (0x%02X) | Expected Size: %4d bytes | Hex: ",
						i + 1, e.dwTime, e.bHeader, e.bHeader, e.iSize);
				for (int k = 0; k < 16; ++k)
					fprintf(fp, "%02X ", e.abPreview[k]);
				fprintf(fp, "\n");
			}
		}
		fprintf(fp, "====================================================================\n\n");
		fclose(fp);
	}

	TraceError("[PACKET_DESYNC] Corrupted header %d (0x%02X) in phase %s! Check packet_desync_debug.log for full history.",
		iBadHeader, (BYTE)iBadHeader, szPhase ? szPhase : "UNKNOWN");

	TPacketCGPacketDesyncReport report;
	memset(&report, 0, sizeof(report));
	report.header = HEADER_CG_PACKET_DESYNC_REPORT;
	report.bad_header = (BYTE)iBadHeader;
	if (szPhase)
		strncpy(report.szPhase, szPhase, sizeof(report.szPhase) - 1);
	else
		strncpy(report.szPhase, "UNKNOWN", sizeof(report.szPhase) - 1);
	report.dwBufferLeft = (DWORD)GetRecvBufferSize();
	report.bHistoryCount = 32;
	for (int i = 0; i < 32; ++i)
	{
		int idx = (m_iPacketHistoryIndex - 31 + i + 64) % 64;
		report.aHistory[i] = m_aPacketHistory[idx];
	}
	Send(sizeof(report), &report);
}
#endif

bool CPythonNetworkStream::RecvErrorPacket(int header)
{
	TraceError("Phase %s does not handle this header (header: %d, last: %d, %d)",
		m_strPhase.c_str(), header, g_iLastPacket[0], g_iLastPacket[1]);

#ifdef ENABLE_PACKET_DESYNC_LOG
	DumpPacketDesync(header, m_strPhase.c_str());
#endif

	ClearRecvBuffer();
	return true;
}

bool CPythonNetworkStream::RecvPhasePacket()
{
	TPacketGCPhase packet_phase;

	if (!Recv(sizeof(TPacketGCPhase), &packet_phase))
		return false;

#ifdef __ANDROID__
	LOGI("[NET] RecvPhasePacket: transitioning to phase=%d (current=%s)", packet_phase.phase, m_strPhase.c_str());
#endif

	switch (packet_phase.phase)
	{
		case PHASE_CLOSE:				// 끊기는 상태 (또는 끊기 전 상태)
			ClosePhase();
			break;

		case PHASE_HANDSHAKE:			// 악수..;;
			SetHandShakePhase();
			break;

		case PHASE_LOGIN:				// 로그인 중
			SetLoginPhase();
			break;

		case PHASE_SELECT:				// 캐릭터 선택 화면
			SetSelectPhase();

			BuildProcessCRC();
	
			// MARK_BUG_FIX
			__DownloadMark();
			// END_OF_MARK_BUG_FIX
			break;

		case PHASE_LOADING:				// 선택 후 로딩 화면
			SetLoadingPhase();
			break;

		case PHASE_GAME:				// 게임 화면
			SetGamePhase();
			break;

		case PHASE_DEAD:				// 죽었을 때.. (게임 안에 있는 것일 수도..)
			break;
	}

	return true;
}

bool CPythonNetworkStream::RecvPingPacket()
{
	Tracef("recv ping packet. (securitymode %u)\n", IsSecurityMode());

	TPacketGCPing kPacketPing;

	if (!Recv(sizeof(TPacketGCPing), &kPacketPing))
		return false;

	m_dwLastGamePingTime = ELTimer_GetMSec();

	TPacketCGPong kPacketPong;
	kPacketPong.bHeader = HEADER_CG_PONG;

	if (!Send(sizeof(TPacketCGPong), &kPacketPong))
		return false;

	if (IsSecurityMode())
		return SendSequence();
	else
		return true;
}

bool CPythonNetworkStream::RecvDefaultPacket(int header)
{
	if (!header)
		return true;

	TraceError("Default packet handler called for header %d, state %s\n", header, m_strPhase.c_str());
#ifdef ENABLE_PACKET_DESYNC_LOG
	DumpPacketDesync(header, m_strPhase.c_str());
#endif
	ClearRecvBuffer();
	return true;
}

bool CPythonNetworkStream::OnProcess()
{
	if (m_isStartGame)
	{
		m_isStartGame = FALSE;

		PyCallClassMemberFunc(m_poHandler, "SetGamePhase", Py_BuildValue("()"));
//		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartGame", Py_BuildValue("()"));
	}
	
	m_rokNetActorMgr->Update();

	if (m_phaseProcessFunc.IsEmpty())
		return true;

	//TPacketHeader header;
	//while(CheckPacket(&header))
	{
		m_phaseProcessFunc.Run();
	}

	return true;
}


// Set
void CPythonNetworkStream::SetOffLinePhase()
{
	if ("OffLine" != m_strPhase)
		m_phaseLeaveFunc.Run();

	m_strPhase = "OffLine";

	Tracen("");
	Tracen("## Network - OffLine Phase ##");	
	Tracen("");

	m_dwChangingPhaseTime = ELTimer_GetMSec();
	m_phaseProcessFunc.Set(this, &CPythonNetworkStream::OffLinePhase);
	m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveOfflinePhase);

	SetGameOffline();

	m_dwSelectedCharacterIndex = 0;

	__DirectEnterMode_Initialize();
	__BettingGuildWar_Initialize();
}


void CPythonNetworkStream::ClosePhase()
{
	PyCallClassMemberFunc(m_poHandler, "SetLoginPhase", Py_BuildValue("()"));
}

// Game Online
void CPythonNetworkStream::SetGameOnline()
{
	m_isGameOnline = TRUE;
}

void CPythonNetworkStream::SetGameOffline()
{
	m_isGameOnline = FALSE;
}

BOOL CPythonNetworkStream::IsGameOnline()
{
	return m_isGameOnline;
}

// Handler
void CPythonNetworkStream::SetHandler(PyObject* poHandler)
{
	m_poHandler = poHandler;
}

// ETC
DWORD CPythonNetworkStream::GetMainActorVID()
{
	return m_dwMainActorVID;
}

DWORD CPythonNetworkStream::GetMainActorRace()
{
	return m_dwMainActorRace;
}

DWORD CPythonNetworkStream::GetMainActorEmpire()
{
	return m_dwMainActorEmpire;
}

DWORD CPythonNetworkStream::GetMainActorSkillGroup()
{
	return m_dwMainActorSkillGroup;
}

void CPythonNetworkStream::SetEmpireID(DWORD dwEmpireID)
{
	m_dwEmpireID = dwEmpireID;
}

DWORD CPythonNetworkStream::GetEmpireID()
{
	return m_dwEmpireID;
}

#ifdef ENABLE_MOVE_CHANNEL
void CPythonNetworkStream::SetMapIndex(int iMapIndex)
{
	m_iMapIndex = iMapIndex;
}

int CPythonNetworkStream::GetMapIndex()
{
	return m_iMapIndex;
}

void CPythonNetworkStream::SetChannelNumber(BYTE bChannelNumber)
{
	m_bChannelNumber = bChannelNumber;
}

BYTE CPythonNetworkStream::GetChannelNumber()
{
	return m_bChannelNumber;
}
#endif

void CPythonNetworkStream::__ClearSelectCharacterData()
{
	NANOBEGIN
	memset(&m_akSimplePlayerInfo, 0, sizeof(m_akSimplePlayerInfo));

	for (int i = 0; i < PLAYER_PER_ACCOUNT4; ++i)
	{
		m_adwGuildID[i] = 0;
		m_astrGuildName[i] = "";
	}
	NANOEND
}

void CPythonNetworkStream::__DirectEnterMode_Initialize()
{
	m_kDirectEnterMode.m_isSet=false;
	m_kDirectEnterMode.m_dwChrSlotIndex=0;	
}

void CPythonNetworkStream::__DirectEnterMode_Set(UINT uChrSlotIndex)
{
	m_kDirectEnterMode.m_isSet=true;
	m_kDirectEnterMode.m_dwChrSlotIndex=uChrSlotIndex;
}

bool CPythonNetworkStream::__DirectEnterMode_IsSet()
{
	return m_kDirectEnterMode.m_isSet;
}

void CPythonNetworkStream::__InitializeMarkAuth()
{
	m_kMarkAuth.m_dwHandle=0;
	m_kMarkAuth.m_dwRandomKey=0;
}

void CPythonNetworkStream::__BettingGuildWar_Initialize()
{
	m_kBettingGuildWar.m_dwBettingMoney=0;
	m_kBettingGuildWar.m_dwObserverCount=0;
}

void CPythonNetworkStream::__BettingGuildWar_SetObserverCount(UINT uObserverCount)
{
	m_kBettingGuildWar.m_dwObserverCount=uObserverCount;
}

void CPythonNetworkStream::__BettingGuildWar_SetBettingMoney(UINT uBettingMoney)
{
	m_kBettingGuildWar.m_dwBettingMoney=uBettingMoney;
}

DWORD CPythonNetworkStream::EXPORT_GetBettingGuildWarValue(const char* c_szValueName)
{
	if (stricmp(c_szValueName, "OBSERVER_COUNT") == 0)
		return m_kBettingGuildWar.m_dwObserverCount;

	if (stricmp(c_szValueName, "BETTING_MONEY") == 0)
		return m_kBettingGuildWar.m_dwBettingMoney;

	return 0;
}

void CPythonNetworkStream::__ServerTimeSync_Initialize()
{
	m_kServerTimeSync.m_dwChangeClientTime=0;
	m_kServerTimeSync.m_dwChangeServerTime=0;
}

void CPythonNetworkStream::SetWaitFlag()
{
	m_isWaitLoginKey = TRUE;
}

void CPythonNetworkStream::SendEmoticon(UINT eEmoticon)
{
	if(eEmoticon < m_EmoticonStringVector.size())
		SendChatPacket(m_EmoticonStringVector[eEmoticon].c_str());
	else
		assert(false && "SendEmoticon Error");
}

CPythonNetworkStream::CPythonNetworkStream()
{
	m_rokNetActorMgr=new CNetworkActorManager;

	memset(m_akSimplePlayerInfo, 0, sizeof(m_akSimplePlayerInfo));

#ifdef ENABLE_PACKET_DESYNC_LOG
	memset(m_aPacketHistory, 0, sizeof(m_aPacketHistory));
	m_iPacketHistoryIndex = 0;
#endif

	m_phaseProcessFunc.Clear();

	m_dwEmpireID = 0;
	m_dwGuildID = 0;

	m_dwMainActorVID = 0;
	m_dwMainActorRace = 0;
	m_dwMainActorEmpire = 0;
	m_dwMainActorSkillGroup = 0;
	m_poHandler = NULL;

	m_dwLastGamePingTime = 0;

	m_dwLoginKey = 0;
	m_isWaitLoginKey = FALSE;
	m_isStartGame = FALSE;
	m_isEnableChatInsultFilter = FALSE;
	m_bComboSkillFlag = FALSE;
	m_strPhase = "OffLine";

#ifdef ENABLE_MOVE_CHANNEL
	m_bChannelNumber = 0;
	m_iMapIndex = 0;
#endif

#ifdef ENABLE_LEFT_SEAT
	m_bIsLeftSeatLogOutState = false;
#endif

	__InitializeGamePhase();
	__InitializeMarkAuth();

	__DirectEnterMode_Initialize();
	__BettingGuildWar_Initialize();

	std::fill(m_apoPhaseWnd, m_apoPhaseWnd+PHASE_WINDOW_NUM, (PyObject*)NULL);
	m_poSerCommandParserWnd = NULL;

	SetOffLinePhase();
}

CPythonNetworkStream::~CPythonNetworkStream()
{
	Tracen("PythonNetworkMainStream Clear");
#ifdef ENABLE_LOADING_TIP_INFO
	m_TipVnum.clear();
	m_TipList.clear();
#endif
}

void CPythonNetworkStream::HideQuestWindows()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "HideAllQuestWindow", Py_BuildValue("()"));
}

#ifdef ENABLE_INGAME_WIKI
void CPythonNetworkStream::ToggleWikiWindow()
{
	if (m_apoPhaseWnd[PHASE_WINDOW_GAME])
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ToggleWikiWindow", Py_BuildValue("()"));
}

void CPythonNetworkStream::SendPythonFunc(PyObject* poArgs, const char* c_szFuncName)
{
	if (m_apoPhaseWnd[PHASE_WINDOW_GAME])
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], (char*)c_szFuncName, poArgs);
}

void CPythonNetworkStream::SendPythonFuncWithoutParameter(const char* c_szFuncName)
{
	if (m_apoPhaseWnd[PHASE_WINDOW_GAME])
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], (char*)c_szFuncName, Py_BuildValue("()"));
}
#endif
