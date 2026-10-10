#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "Packet.h"

#include "PythonGuild.h"
#include "PythonCharacterManager.h"
#include "PythonPlayer.h"
#include "PythonBackground.h"
#include "PythonMiniMap.h"
#include "PythonTextTail.h"
#include "PythonItem.h"
#include "PythonChat.h"
#include "PythonShop.h"
#include "PythonExchange.h"
#include "PythonQuest.h"
#include "PythonEventManager.h"
#include "PythonMessenger.h"
#include "PythonApplication.h"
#include "../EterPack/EterPackManager.h"
#include "../GameLib/ItemManager.h"
#include "AbstractApplication.h"
#include "AbstractCharacterManager.h"
#include "InstanceBase.h"
#include "ProcessCRC.h"
#ifdef ENABLE_BATTLE_FIELD
#include "PythonCombatZone.h"
#endif
#ifdef ENABLE_CUBE_RENEWAL
#include "PythonCubeRenewal.h"
#endif
#ifdef ENABLE_EVENT_SYSTEM
#include "PythonGameEvents.h"
#endif
#ifdef ENABLE_RIDING_EXTENDED
#include "PythonMountUpGrade.h"
#endif
#ifdef ENABLE_TITLE_SYSTEM
#include "PythonTitleManager.h"
#endif
#ifdef ENABLE_MESSENGER_RENEWAL
#include "PythonCommunity.h"
#endif

BOOL gs_bEmpireLanuageEnable = TRUE;

#ifdef ENABLE_AUTO_RESTART_EVENT
#include "EventHandler.h"
void SendAutoHuntRestartPauseBot()
{
	const auto& hndl = EventHandler::Instance().GetHandler("AutoHuntRestartPauseBot");
	if (!hndl)
		return;
	int count = hndl->get()->bind; // how many called
	if (count)
	{
		CPythonPlayer::Instance().SetAutoPause(false);
	}
}

void SendAutoHuntRestart()
{
	const auto& hndl = EventHandler::Instance().GetHandler("AutoHuntRestart");
	if (!hndl)
		return;
	int count = hndl->get()->bind; // how many called
	if (count)
	{
		CPythonPlayer::Instance().SetAutoPause(true);

		EventHandler::Instance().AddEvent("AutoHuntRestartPauseBot", std::bind(&SendAutoHuntRestartPauseBot), 15, 1);

		CPythonNetworkStream::Instance().SendChatPacket("/restart_auto");
	}
}
#endif

void CPythonNetworkStream::__RefreshAlignmentWindow()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshAlignment", Py_BuildValue("()"));
}

void CPythonNetworkStream::__RefreshTargetBoardByVID(DWORD dwVID)
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshTargetBoardByVID", Py_BuildValue("(i)", dwVID));
}

void CPythonNetworkStream::__RefreshTargetBoardByName(const char * c_szName)
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshTargetBoardByName", Py_BuildValue("(s)", c_szName));
}

void CPythonNetworkStream::__RefreshTargetBoard()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshTargetBoard", Py_BuildValue("()"));
}

void CPythonNetworkStream::__RefreshGuildWindowGradePage()
{
	m_isRefreshGuildWndGradePage=true;
}

void CPythonNetworkStream::__RefreshGuildWindowSkillPage()
{
	m_isRefreshGuildWndSkillPage=true;
}

void CPythonNetworkStream::__RefreshGuildWindowMemberPageGradeComboBox()
{
	m_isRefreshGuildWndMemberPageGradeComboBox=true;
}

void CPythonNetworkStream::__RefreshGuildWindowMemberPage()
{
	m_isRefreshGuildWndMemberPage=true;
}

void CPythonNetworkStream::__RefreshGuildWindowBoardPage()
{
	m_isRefreshGuildWndBoardPage=true;
}

void CPythonNetworkStream::__RefreshGuildWindowInfoPage()
{
	m_isRefreshGuildWndInfoPage=true;
}

void CPythonNetworkStream::__RefreshMessengerWindow()
{
	m_isRefreshMessengerWnd=true;
}

void CPythonNetworkStream::__RefreshSafeboxWindow()
{
	m_isRefreshSafeboxWnd=true;
}

void CPythonNetworkStream::__RefreshMallWindow()
{
	m_isRefreshMallWnd=true;
}

void CPythonNetworkStream::__RefreshSkillWindow()
{
	m_isRefreshSkillWnd=true;
}

void CPythonNetworkStream::__RefreshExchangeWindow()
{
	m_isRefreshExchangeWnd=true;
}

void CPythonNetworkStream::__RefreshStatus()
{
	m_isRefreshStatus=true;
}

void CPythonNetworkStream::__RefreshCharacterWindow()
{
	m_isRefreshCharacterWnd=true;
}

void CPythonNetworkStream::__RefreshInventoryWindow()
{
	m_isRefreshInventoryWnd=true;
}

void CPythonNetworkStream::__RefreshEquipmentWindow()
{
	m_isRefreshEquipmentWnd=true;
}

#ifdef ENABLE_SHOP_SEARCH_SYSTEM
void CPythonNetworkStream::__RefreshShopSearchWindow()
{
	m_isRefreshShopSearchWnd = true;
}
#endif

void CPythonNetworkStream::__SetGuildID(DWORD id)
{
	if (m_dwGuildID != id)
	{
		m_dwGuildID = id;
		IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();

		for (int i = 0; i < PLAYER_PER_ACCOUNT4; ++i)
			if (!strncmp(m_akSimplePlayerInfo[i].szName, rkPlayer.GetName(), CHARACTER_NAME_MAX_LEN))
			{
				m_adwGuildID[i] = id;

				std::string  guildName;
				if (CPythonGuild::Instance().GetGuildName(id, &guildName))
				{
					m_astrGuildName[i] = guildName;
				}
				else
				{
					m_astrGuildName[i] = "";
				}
			}
	}
}

struct PERF_PacketInfo
{
	DWORD dwCount;
	DWORD dwTime;

	PERF_PacketInfo()
	{
		dwCount=0;
		dwTime=0;
	}
};


#ifdef __PERFORMANCE_CHECK__

class PERF_PacketTimeAnalyzer
{
	public:
		~PERF_PacketTimeAnalyzer()
		{
			FILE* fp=fopen("perf_dispatch_packet_result.txt", "w");		

			for (std::map<DWORD, PERF_PacketInfo>::iterator i=m_kMap_kPacketInfo.begin(); i!=m_kMap_kPacketInfo.end(); ++i)
			{
				if (i->second.dwTime>0)
					fprintf(fp, "header %d: count %d, time %d, tpc %d\n", i->first, i->second.dwCount, i->second.dwTime, i->second.dwTime/i->second.dwCount);
			}
			fclose(fp);
		}

	public:
		std::map<DWORD, PERF_PacketInfo> m_kMap_kPacketInfo;
};

PERF_PacketTimeAnalyzer gs_kPacketTimeAnalyzer;

#endif

// Game Phase ---------------------------------------------------------------------------
void CPythonNetworkStream::GamePhase()
{
	if (!m_kQue_stHack.empty())
	{
		__SendHack(m_kQue_stHack.front().c_str());
		m_kQue_stHack.pop_front();
	}

	TPacketHeader header = 0;
	bool ret = true;

#ifdef __PERFORMANCE_CHECK__
	DWORD timeBeginDispatch=timeGetTime();

	static std::map<DWORD, PERF_PacketInfo> kMap_kPacketInfo;
	kMap_kPacketInfo.clear();
#endif

	const DWORD MAX_RECV_COUNT = 32; //28-02-25 Bvural41 Fix
	const DWORD SAFE_RECV_BUFSIZE = 8192;
	DWORD dwRecvCount = 0;

	while (ret)
	{
		if(dwRecvCount++ >= MAX_RECV_COUNT-1 && GetRecvBufferSize() < SAFE_RECV_BUFSIZE
			&& m_strPhase == "Game") //phase_game ì´ ì•„ë‹ˆì–´ë„ ì—¬ê¸°ë¡œ ë“¤ì–´ì˜¤ëŠ” ê²½ìš°ê°€ ìˆë‹¤.
			break;

		if (!CheckPacket(&header))
			break;

#ifdef __PERFORMANCE_CHECK__
		DWORD timeBeginPacket=timeGetTime();
#endif

		switch (header)
		{
			case HEADER_GC_OBSERVER_ADD:
				ret = RecvObserverAddPacket();
				break;

			case HEADER_GC_OBSERVER_REMOVE:
				ret = RecvObserverRemovePacket();
					break;

			case HEADER_GC_OBSERVER_MOVE:
				ret = RecvObserverMovePacket();
				break;

			case HEADER_GC_WARP:
				ret = RecvWarpPacket();
				break;

			case HEADER_GC_PHASE:
				ret = RecvPhasePacket();
				return; // ë„ì¤‘ì— Phase ê°€ ë°”ë€Œë©´ ì¼ë‹¨ ë¬´ì¡°ê±´ GamePhase íƒˆì¶œ - [levites]
				break;

			case HEADER_GC_PVP:
				ret = RecvPVPPacket();
				break;

			case HEADER_GC_DUEL_START:
				ret = RecvDuelStartPacket();
				break;

			case HEADER_GC_CHARACTER_ADD:
 				ret = RecvCharacterAppendPacket();
				break;

			case HEADER_GC_CHAR_ADDITIONAL_INFO:
				ret = RecvCharacterAdditionalInfo();
				break;

			case HEADER_GC_CHARACTER_UPDATE:
				ret = RecvCharacterUpdatePacket();
				break;

			case HEADER_GC_CHARACTER_DEL:
				ret = RecvCharacterDeletePacket();
				break;

			case HEADER_GC_CHAT:
				ret = RecvChatPacket();
				break;

			case HEADER_GC_SYNC_POSITION:
				ret = RecvSyncPositionPacket();
				break;

			case HEADER_GC_OWNERSHIP:
				ret = RecvOwnerShipPacket();
				break;

			case HEADER_GC_WHISPER:
				ret = RecvWhisperPacket();
				break;

			case HEADER_GC_CHARACTER_MOVE:
				ret = RecvCharacterMovePacket();
				break;

			// Position
			case HEADER_GC_CHARACTER_POSITION:
				ret = RecvCharacterPositionPacket();
				break;

			// Battle Packet
			case HEADER_GC_STUN:
				ret = RecvStunPacket();
				break;

			case HEADER_GC_DEAD:
				ret = RecvDeadPacket();
				break;

			case HEADER_GC_PLAYER_POINT_CHANGE:
				ret = RecvPointChange();
				break;

			// item packet.
			case HEADER_GC_ITEM_SET:
				ret = RecvItemSetPacket();
				break;

			case HEADER_GC_ITEM_SET2:
				ret = RecvItemSetPacket2();
				break;

			case HEADER_GC_ITEM_USE:
				ret = RecvItemUsePacket();
				break;

			case HEADER_GC_ITEM_UPDATE:
				ret = RecvItemUpdatePacket();
				break;

			case HEADER_GC_ITEM_GROUND_ADD:
				ret = RecvItemGroundAddPacket();
				break;

			case HEADER_GC_ITEM_GROUND_DEL:
				ret = RecvItemGroundDelPacket();
				break;

			case HEADER_GC_ITEM_OWNERSHIP:
				ret = RecvItemOwnership();
				break;

			case HEADER_GC_QUICKSLOT_ADD:
				ret = RecvQuickSlotAddPacket();
				break;

			case HEADER_GC_QUICKSLOT_DEL:
				ret = RecvQuickSlotDelPacket();
				break;

			case HEADER_GC_QUICKSLOT_SWAP:
				ret = RecvQuickSlotMovePacket();
				break;

			case HEADER_GC_MOTION:
				ret = RecvMotionPacket();
				break;

			case HEADER_GC_SHOP:
				ret = RecvShopPacket();
				break;
				
			case HEADER_GC_SHOP_SIGN:
				ret = RecvShopSignPacket();
				break;

			case HEADER_GC_EXCHANGE:
				ret = RecvExchangePacket();
				break;

			case HEADER_GC_QUEST_INFO:
				ret = RecvQuestInfoPacket();
				break;

			case HEADER_GC_REQUEST_MAKE_GUILD:
				ret = RecvRequestMakeGuild();
				break;

			case HEADER_GC_PING:
				ret = RecvPingPacket();
				break;

			case HEADER_GC_SCRIPT:
				ret = RecvScriptPacket();
				break;

			case HEADER_GC_QUEST_CONFIRM:
				ret = RecvQuestConfirmPacket();
				break;

			case HEADER_GC_TARGET:
				ret = RecvTargetPacket();
				break;

			case HEADER_GC_DAMAGE_INFO:
				ret = RecvDamageInfoPacket();
				break;

			case HEADER_GC_MOUNT:
				ret = RecvMountPacket();
				break;

			case HEADER_GC_CHANGE_SPEED:
				ret = RecvChangeSpeedPacket();
				break;

			case HEADER_GC_PLAYER_POINTS:
				ret = __RecvPlayerPoints();
				break;

			case HEADER_GC_CREATE_FLY:
				ret = RecvCreateFlyPacket();
				break;

			case HEADER_GC_FLY_TARGETING:
				ret = RecvFlyTargetingPacket();
				break;

			case HEADER_GC_ADD_FLY_TARGETING:
				ret = RecvAddFlyTargetingPacket();
				break;

			case HEADER_GC_SKILL_LEVEL:
				ret = RecvSkillLevel();
				break;

			case HEADER_GC_CHANGE_NAME:
				ret = __RecvChangeName();
				break;

#ifdef ENABLE_BATTLE_ROYALE
			case HEADER_GC_BATTLE_ROYALE:
				ret = RecvBattleRoyale();
				break;
#endif

#ifdef ENABLE_ATTR_6TH_7TH_SYSTEM
			case HEADER_GC_OPEN_67_ATTR:
				ret = Recv67AttrOpenPacket();
				break;
#endif

#ifdef ENABLE_KILL_STATISTICS
			case HEADER_GC_KILL_STATISTICS:
				ret = RecvKillStatistics();
				break;
#endif

#ifdef ENABLE_TARGET_INFORMATION_SYSTEM
			case HEADER_GC_TARGET_INFO:
				ret = RecvTargetInfoPacket();
				break;
#endif

#ifdef ENABLE_SOUL_ROULETTE_SYSTEM
			case HEADER_GC_SOUL_ROULETTE:
				ret = RecvSoulRoulette();
				break;
#endif

#ifdef ENABLE_BATTLE_FIELD
			case HEADER_GC_RANKING_COMBAT_ZONE:
				ret = __RecvCombatZoneRankingPacket();
				break;
				
			case HEADER_GC_SEND_COMBAT_ZONE:
				ret = __RecvCombatZonePacket();
				break;
#endif

#ifdef ENABLE_MINI_GAME_CATCH_KING
			case HEADER_GC_MINI_GAME_CATCH_KING:
				ret = RecvMiniGameCatchKingPacket();
				break;
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
			case HEADER_GC_REQUEST_CHANGE_LANGUAGE:
				ret = RecvRequestChangeLanguage();
				break;
#endif

#ifdef ENABLE_MULTI_LANGUAGE_WHISPER_DETAILS
			case HEADER_GC_WHISPER_DETAILS:
				ret = RecvWhisperDetails();
				break;
#endif

#ifdef ENABLE_TITLE_SYSTEM
			case HEADER_GC_TITLE_DATA:
				ret = RecvTitleDataPacket();
				break;

			case HEADER_GC_TITLE_UPDATE:
				ret = RecvTitleUpdatePacket();
				break;
#endif

			case HEADER_GC_SKILL_LEVEL_NEW:
				ret = RecvSkillLevelNew();
				break;

			case HEADER_GC_MESSENGER:
				ret = RecvMessenger();
				break;

			case HEADER_GC_GUILD:
				ret = RecvGuild();
				break;

			case HEADER_GC_PARTY_INVITE:
				ret = RecvPartyInvite();
				break;

			case HEADER_GC_PARTY_ADD:
				ret = RecvPartyAdd();
				break;

			case HEADER_GC_PARTY_UPDATE:
				ret = RecvPartyUpdate();
				break;

			case HEADER_GC_PARTY_REMOVE:
				ret = RecvPartyRemove();
				break;

			case HEADER_GC_PARTY_LINK:
				ret = RecvPartyLink();
				break;

			case HEADER_GC_PARTY_UNLINK:
				ret = RecvPartyUnlink();
				break;

			case HEADER_GC_PARTY_PARAMETER:
				ret = RecvPartyParameter();
				break;

			case HEADER_GC_SAFEBOX_SET:
				ret = RecvSafeBoxSetPacket();
				break;

			case HEADER_GC_SAFEBOX_DEL:
				ret = RecvSafeBoxDelPacket();
				break;

			case HEADER_GC_SAFEBOX_WRONG_PASSWORD:
				ret = RecvSafeBoxWrongPasswordPacket();
				break;

			case HEADER_GC_SAFEBOX_SIZE:
				ret = RecvSafeBoxSizePacket();
				break;

			case HEADER_GC_SAFEBOX_MONEY_CHANGE:
				ret = RecvSafeBoxMoneyChangePacket();
				break;

			case HEADER_GC_FISHING:
				ret = RecvFishing();
				break;

			case HEADER_GC_DUNGEON:
				ret = RecvDungeon();
				break;

			case HEADER_GC_TIME:
				ret = RecvTimePacket();
				break;

			case HEADER_GC_WALK_MODE:
				ret = RecvWalkModePacket();
				break;

			case HEADER_GC_CHANGE_SKILL_GROUP:
				ret = RecvChangeSkillGroupPacket();
				break;

			case HEADER_GC_REFINE_INFORMATION:
				ret = RecvRefineInformationPacket();
				break;

			case HEADER_GC_REFINE_INFORMATION_NEW:
				ret = RecvRefineInformationPacketNew();
				break;

			case HEADER_GC_SEPCIAL_EFFECT:
				ret = RecvSpecialEffect();
				break;

			case HEADER_GC_NPC_POSITION:
				ret = RecvNPCList();
				break;

			case HEADER_GC_CHANNEL:
				ret = RecvChannelPacket();
				break;

			case HEADER_GC_LAND_LIST:
				ret = RecvLandPacket();
				break;

			//case HEADER_GC_TARGET_CREATE:
			//	ret = RecvTargetCreatePacket();
			//	break;

			case HEADER_GC_TARGET_CREATE_NEW:
				ret = RecvTargetCreatePacketNew();
				break;

			case HEADER_GC_TARGET_UPDATE:
				ret = RecvTargetUpdatePacket();
				break;

			case HEADER_GC_TARGET_DELETE:
				ret = RecvTargetDeletePacket();
				break;

			case HEADER_GC_AFFECT_ADD:
				ret = RecvAffectAddPacket();
				break;

			case HEADER_GC_AFFECT_REMOVE:
				ret = RecvAffectRemovePacket();
				break;

			case HEADER_GC_MALL_OPEN:
				ret = RecvMallOpenPacket();
				break;

			case HEADER_GC_MALL_SET:
				ret = RecvMallItemSetPacket();
				break;

			case HEADER_GC_MALL_DEL:
				ret = RecvMallItemDelPacket();
				break;

			case HEADER_GC_LOVER_INFO:
				ret = RecvLoverInfoPacket();
				break;

			case HEADER_GC_LOVE_POINT_UPDATE:
				ret = RecvLovePointUpdatePacket();
				break;

			case HEADER_GC_DIG_MOTION:
				ret = RecvDigMotionPacket();
				break;

			case HEADER_GC_HANDSHAKE:
				RecvHandshakePacket();
				return;
				break;

			case HEADER_GC_HANDSHAKE_OK:
				RecvHandshakeOKPacket();
				return;
				break;
			
			case HEADER_GC_HYBRIDCRYPT_KEYS:
				RecvHybridCryptKeyPacket();
				return;
				break;

			case HEADER_GC_HYBRIDCRYPT_SDB:
				RecvHybridCryptSDBPacket();
				return;
				break;

			case HEADER_GC_SPECIFIC_EFFECT:
				ret = RecvSpecificEffect();
				break;

			case HEADER_GC_DRAGON_SOUL_REFINE:
				ret = RecvDragonSoulRefine();
				break;

#ifdef ENABLE_BATTLEPASS
			case net_battlepass::HEADER_GC_BATTLEPASS:
				ret = BattlePassManager::RecvBattlePassPacket();
				break;
#endif

#ifdef ENABLE_FISH_EVENT_SYSTEM
			case HEADER_GC_FISH_EVENT_INFO:
				ret = RecvFishEventInfo();
				break;
#endif

#ifdef ENABLE_DS_SET
			case HEADER_GC_DS_TABLE:
				ret = RecvDSTablePacket();
				break;
#endif

#ifdef ENABLE_REFINE_ELEMENT
			case HEADER_GC_REFINE_ELEMENT:
				ret = RecvRefineElementPacket();
				break;
#endif

#ifdef ENABLE_SWITCHBOT
			case HEADER_GC_SWITCHBOT:
				ret = RecvSwitchbotPacket();
				break;
#endif

#ifdef ENABLE_SHOP_SEARCH_SYSTEM
			case HEADER_GC_SHOPSEARCH_SET:
				ret = RecvShopSearchSet();
				break;
#endif

#ifdef ENABLE_OFFLINESHOP_SYSTEM
			case HEADER_GC_OFFLINE_SHOP:
				ret = RecvOfflineShopPacket();
				break;
	
			case HEADER_GC_OFFLINE_SHOP_SIGN:
				ret = RecvOfflineShopSignPacket();
				break;
#endif

#ifdef ENABLE_SASH_SYSTEM
			case HEADER_GC_SASH:
				ret = RecvSashPacket();
				break;
#endif

#ifdef ENABLE_SUPPORT_SYSTEM
			case HEADER_GC_SUPPORT_SHAMAN_SKILL:
				ret = RecvSupportShamanUseSkill();
				break;
#endif

#ifdef ENABLE_EVENT_SYSTEM
			case HEADER_GC_EVENT_INFO:
				ret = RecvEventInfo();
				break;
#endif

#ifdef ENABLE_SELECT_REWARD_BOX
			case HEADER_GC_REWARD_INFO:
				ret = RecvSelectRewardInfo();
				break;


			case HEADER_GC_REWARD_RESULT:
				if (!RecvSelectRewardResultPacket())
					return;
				break;
#endif

#ifdef ENABLE_MAILBOX_SYSTEM
			case HEADER_GC_MAILBOX_RECEIVE:
				ret = RecvMailBox();
				break;
#endif

#ifdef ENABLE_EXTEND_INVEN_SYSTEM
			case HEADER_GC_EXTEND_INVENTORY:
				ret = RecvExtendInventoryPacket();
				break;
#endif

#ifdef ENABLE_CHANGELOOK_SYSTEM
			case HEADER_GC_CL:
				ret = RecvChangeLookPacket();
				break;
#endif

#ifdef ENABLE_12ZI
			case HEADER_GC_SEPCIAL_ZODIAC_EFFECT:
				ret = RecvSpecialZodiacEffect();
				break;
#endif

#ifdef ENABLE_VIEW_EQUIPMENT_SYSTEM
			case HEADER_GC_VIEW_EQUIP:
				ret = RecvViewEquipPacket();
				break;
#endif

#ifdef ENABLE_CUBE_RENEWAL
			case HEADER_GC_CUBE_RENEWAL:
				ret = RecvCubeRenewalPacket();
				break;	
#endif

#ifdef ENABLE_SHOW_CHEST_DROP
			case HEADER_GC_CHEST_DROP_INFO:
				ret = RecvChestDropInfo();
				break;
#endif

#ifdef _IMPROVED_PACKET_ENCRYPTION_
			case HEADER_GC_KEY_AGREEMENT:
				RecvKeyAgreementPacket();
				return;
				break;

			case HEADER_GC_KEY_AGREEMENT_COMPLETED:
				RecvKeyAgreementCompletedPacket();
				return;
				break;
#endif

#ifdef ENABLE_AURA_SYSTEM
			case HEADER_GC_AURA:
				ret = RecvAuraPacket();
				break;
#endif
#ifdef ENABLE_GIFTBOX_MULTI_OPEN
			case HEADER_GC_REFRESH_INVENTORY:
				ret = RecvRefreshInventoryPacket();
				break;
#endif
#ifdef ENABLE_RIDING_EXTENDED
			case HEADER_GC_MOUNT_UP_GRADE:
				ret = RecvMountUpGrade();
				break;

			case HEADER_GC_MOUNT_UP_GRADE_CHAT:
				ret = RecvMountUpGradeChat();
				break;
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
			case HEADER_GC_GROWTH_PET_INFO:
				ret = RecvGrowthPetInformation();
				break;
#endif
#if defined(ENABLE_CLIENT_TIMER)
			case HEADER_GC_CLIENT_TIMER:
				ret = RecvClientTimer();
				break;
#endif
#ifdef ENABLE_TREASURE_EVENT
			case HEADER_GC_TREASURE_EVENT:
				ret = RecvTreasureEvent();
				break;
#endif
			default:
				ret = RecvDefaultPacket(header);
				break;
		}
#ifdef __PERFORMANCE_CHECK__
		DWORD timeEndPacket=timeGetTime();

		{
			PERF_PacketInfo& rkPacketInfo=kMap_kPacketInfo[header];
			rkPacketInfo.dwCount++;
			rkPacketInfo.dwTime+=timeEndPacket-timeBeginPacket;			
		}

		{
			PERF_PacketInfo& rkPacketInfo=gs_kPacketTimeAnalyzer.m_kMap_kPacketInfo[header];
			rkPacketInfo.dwCount++;
			rkPacketInfo.dwTime+=timeEndPacket-timeBeginPacket;			
		}
#endif
	}

#ifdef __PERFORMANCE_CHECK__
	DWORD timeEndDispatch=timeGetTime();
	
	if (timeEndDispatch-timeBeginDispatch>2)
	{
		static FILE* fp=fopen("perf_dispatch_packet.txt", "w");		

		fprintf(fp, "delay %d\n", timeEndDispatch-timeBeginDispatch);
		for (std::map<DWORD, PERF_PacketInfo>::iterator i=kMap_kPacketInfo.begin(); i!=kMap_kPacketInfo.end(); ++i)
		{
			if (i->second.dwTime>0)
				fprintf(fp, "header %d: count %d, time %d\n", i->first, i->second.dwCount, i->second.dwTime);
		}
		fputs("=====================================================\n", fp);
		fflush(fp);
	}
#endif

	if (!ret)
		RecvErrorPacket(header);

	static DWORD s_nextRefreshTime = ELTimer_GetMSec();

	DWORD curTime = ELTimer_GetMSec();
	if (s_nextRefreshTime > curTime)
		return;	
	
	

	if (m_isRefreshCharacterWnd)
	{
		m_isRefreshCharacterWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshCharacter", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 500;
	}

	if (m_isRefreshEquipmentWnd)
	{
		m_isRefreshEquipmentWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshEquipment", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 500;
	}

	if (m_isRefreshInventoryWnd)
	{
		m_isRefreshInventoryWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshInventory", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 500;
	}

	if (m_isRefreshExchangeWnd)
	{
		m_isRefreshExchangeWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshExchange", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 500;
	}

	if (m_isRefreshSkillWnd)
	{
		m_isRefreshSkillWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSkill", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 500;
	}

	if (m_isRefreshSafeboxWnd)
	{
		m_isRefreshSafeboxWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSafebox", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 500;
	}

	if (m_isRefreshMallWnd)
	{
		m_isRefreshMallWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshMall", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 500;
	}

	if (m_isRefreshStatus)
	{
		m_isRefreshStatus=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshStatus", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 500;
	}

	if (m_isRefreshMessengerWnd)
	{
		m_isRefreshMessengerWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshMessenger", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 500;
	}

	if (m_isRefreshGuildWndInfoPage)
	{
		m_isRefreshGuildWndInfoPage=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildInfoPage", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 500;
	}

	if (m_isRefreshGuildWndBoardPage)
	{
		m_isRefreshGuildWndBoardPage=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildBoardPage", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 500;
	}

	if (m_isRefreshGuildWndMemberPage)
	{
		m_isRefreshGuildWndMemberPage=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildMemberPage", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 500;
	}

	if (m_isRefreshGuildWndMemberPageGradeComboBox)
	{
		m_isRefreshGuildWndMemberPageGradeComboBox=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildMemberPageGradeComboBox", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 500;
	}

	if (m_isRefreshGuildWndSkillPage)
	{
		m_isRefreshGuildWndSkillPage=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildSkillPage", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 500;
	}

	if (m_isRefreshGuildWndGradePage)
	{
		m_isRefreshGuildWndGradePage=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildGradePage", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 500;
	}

#ifdef ENABLE_SHOP_SEARCH_SYSTEM
	if (m_isRefreshShopSearchWnd)
	{
		m_isRefreshShopSearchWnd = false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshShopSearch", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 500;
	}
#endif

}

void CPythonNetworkStream::__InitializeGamePhase()
{
	__ServerTimeSync_Initialize();

	m_isRefreshStatus=false;
	m_isRefreshCharacterWnd=false;
	m_isRefreshEquipmentWnd=false;
	m_isRefreshInventoryWnd=false;
	m_isRefreshExchangeWnd=false;
	m_isRefreshSkillWnd=false;
	m_isRefreshSafeboxWnd=false;
	m_isRefreshMallWnd=false;
	m_isRefreshMessengerWnd=false;
	m_isRefreshGuildWndInfoPage=false;
	m_isRefreshGuildWndBoardPage=false;
	m_isRefreshGuildWndMemberPage=false;
	m_isRefreshGuildWndMemberPageGradeComboBox=false;
	m_isRefreshGuildWndSkillPage=false;
	m_isRefreshGuildWndGradePage=false;

	m_EmoticonStringVector.clear();

	m_pInstTarget = NULL;
}

void CPythonNetworkStream::Warp(LONG lGlobalX, LONG lGlobalY)
{
	CPythonBackground& rkBgMgr=CPythonBackground::Instance();
	rkBgMgr.Destroy();
	rkBgMgr.Create();
	rkBgMgr.Warp(lGlobalX, lGlobalY);
#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
	rkBgMgr.RefreshShadowTargetLevel();
	rkBgMgr.RefreshShadowQualityLevel();
#else
	rkBgMgr.RefreshShadowLevel();
#endif

	// NOTE : Warp í–ˆì„ë•Œ CenterPositionì˜ Heightê°€ 0ì´ê¸° ë•Œë¬¸ì— ì¹´ë©”ë¼ê°€ ë•…ë°”ë‹¥ì— ë°•í˜€ìˆê²Œ ë¨
	//        ì›€ì§ì¼ë•Œë§ˆë‹¤ Heightê°€ ê°±ì‹  ë˜ê¸° ë•Œë¬¸ì´ë¯€ë¡œ ë§µì„ ì´ë™í•˜ë©´ Positionì„ ê°•ì œë¡œ í•œë²ˆ
	//        ì…‹íŒ…í•´ì¤€ë‹¤ - [levites]
	LONG lLocalX = lGlobalX;
	LONG lLocalY = lGlobalY;
	__GlobalPositionToLocalPosition(lLocalX, lLocalY);
	float fHeight = CPythonBackground::Instance().GetHeight(float(lLocalX), float(lLocalY));

	IAbstractApplication& rkApp=IAbstractApplication::GetSingleton();
	rkApp.SetCenterPosition(float(lLocalX), float(lLocalY), fHeight);

	__ShowMapName(lLocalX, lLocalY);
}

void CPythonNetworkStream::__ShowMapName(LONG lLocalX, LONG lLocalY)
{
	const std::string & c_rstrMapFileName = CPythonBackground::Instance().GetWarpMapName();
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ShowMapName", Py_BuildValue("(sii)", c_rstrMapFileName.c_str(), lLocalX, lLocalY));
}

void CPythonNetworkStream::__LeaveGamePhase()
{
	CInstanceBase::ClearPVPKeySystem();

	__ClearNetworkActorManager();

	m_bComboSkillFlag = FALSE;

	IAbstractCharacterManager& rkChrMgr=IAbstractCharacterManager::GetSingleton();
	rkChrMgr.Destroy();

	CPythonItem& rkItemMgr=CPythonItem::Instance();
	rkItemMgr.Destroy();
}

void CPythonNetworkStream::SetGamePhase()
{
	if ("Game"!=m_strPhase)
		m_phaseLeaveFunc.Run();

	Tracen("");
	Tracen("## Network - Game Phase ##");
	Tracen("");

	m_strPhase = "Game";

	m_dwChangingPhaseTime = ELTimer_GetMSec();
	m_phaseProcessFunc.Set(this, &CPythonNetworkStream::GamePhase);
	m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveGamePhase);

	// Main Character ë“±ë¡O

	IAbstractPlayer & rkPlayer = IAbstractPlayer::GetSingleton();
	rkPlayer.SetMainCharacterIndex(GetMainActorVID());

	__RefreshStatus();
}

bool CPythonNetworkStream::RecvObserverAddPacket()
{
	TPacketGCObserverAdd kObserverAddPacket;
	if (!Recv(sizeof(kObserverAddPacket), &kObserverAddPacket))
		return false;

	CPythonMiniMap::Instance().AddObserver(
		kObserverAddPacket.vid, 
		kObserverAddPacket.x*100.0f, 
		kObserverAddPacket.y*100.0f);

	return true;
}

#ifdef ENABLE_TARGET_INFORMATION_SYSTEM
bool CPythonNetworkStream::SendTargetInfoLoadPacket(DWORD dwVID)
{
	TPacketCGTargetInfoLoad TargetInfoLoadPacket;
	TargetInfoLoadPacket.header = HEADER_CG_TARGET_INFO_LOAD;
	TargetInfoLoadPacket.dwVID = dwVID;

	if (!Send(sizeof(TargetInfoLoadPacket), &TargetInfoLoadPacket))
		return false;

	return SendSequence();
}
#endif

#ifdef ENABLE_EXTEND_INVEN_SYSTEM
bool CPythonNetworkStream::SendExtendInvenUpgrade()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExtendInventory	ExInvUpgrade;

	ExInvUpgrade.header = HEADER_CG_EXTEND_INVENTORY;
	ExInvUpgrade.sub_header = 0;

	if (!Send(sizeof(ExInvUpgrade), &ExInvUpgrade))
	{
		Tracef("SendExtendInvenUpgrade Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::SendExtendInvenButtonClick(int index)
{
	TPacketCGExtendInventory ExtendButtonClick;
	ExtendButtonClick.header = HEADER_CG_EXTEND_INVENTORY;
	ExtendButtonClick.sub_header = 1;

	if (!Send(sizeof(ExtendButtonClick), &ExtendButtonClick))
		return false;

	return true;
}

bool CPythonNetworkStream::RecvExtendInventoryPacket()
{
	TPacketGCExtendInventory ExtendInvpacket;

	if (!Peek(sizeof(ExtendInvpacket), &ExtendInvpacket))
	{
		Tracen("Recv ExtendInventory Info Packet Error #1");
		return false;
	}

	switch (ExtendInvpacket.subheader)
	{
		case SET_STAGE_EXTEND_INVEN:
		{
			if (!Recv(sizeof(ExtendInvpacket), &ExtendInvpacket))
				return false;

			CPythonPlayer::instance().SetExtendInvenStage(ExtendInvpacket.dwExtend_inventory_stage);
			// TraceError("CPythonPlayer::SetExtendInvenStage(%u) - PACKET RECEIVED SUCESS", CPythonPlayer::instance().GetExtendInvenStage());
			break;
		}
		case NOTIFY_EXTEND_INVEN_ITEM_USE:
		{
			if (!Recv(sizeof(ExtendInvpacket), &ExtendInvpacket))
				return false;

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ExInvenItemUseMsg", Py_BuildValue("(iii)", ExtendInvpacket.id, ExtendInvpacket.id1, ExtendInvpacket.id2));
			break;
		}
		case SET_MAX_EXTEND_INVENTORY:
		{
			if (!Recv(sizeof(ExtendInvpacket), &ExtendInvpacket))
				return false;

			CPythonPlayer::instance().SetExtendInvenMax(ExtendInvpacket.dwExtend_inventory_max);

			// TraceError("CPythonPlayer::SetExtendInvenMax(%u) - PACKET RECEIVED SUCESS", CPythonPlayer::instance().GetExtendInvenMax());
			break;
		}
	}

	__RefreshStatus();
	__RefreshInventoryWindow();

	return true;
}
#endif

bool CPythonNetworkStream::RecvObserverRemovePacket()
{
	TPacketGCObserverAdd kObserverRemovePacket;
	if (!Recv(sizeof(kObserverRemovePacket), &kObserverRemovePacket))
		return false;

	CPythonMiniMap::Instance().RemoveObserver(
		kObserverRemovePacket.vid
	);

	return true;
}

bool CPythonNetworkStream::RecvObserverMovePacket()
{
	TPacketGCObserverMove kObserverMovePacket;
	if (!Recv(sizeof(kObserverMovePacket), &kObserverMovePacket))
		return false;

	CPythonMiniMap::Instance().MoveObserver(
		kObserverMovePacket.vid, 
		kObserverMovePacket.x*100.0f, 
		kObserverMovePacket.y*100.0f);

	return true;
}


bool CPythonNetworkStream::RecvWarpPacket()
{
	TPacketGCWarp kWarpPacket;

	if (!Recv(sizeof(kWarpPacket), &kWarpPacket))
		return false;
#ifdef ENABLE_LOADING_TIP_INFO
	CPythonBackground::Instance().SetWarpMapInfo(kWarpPacket.l_MapIndex);
#endif
	__DirectEnterMode_Set(m_dwSelectedCharacterIndex);
	
	CNetworkStream::Connect((DWORD)kWarpPacket.lAddr, kWarpPacket.wPort);

	return true;
}

bool CPythonNetworkStream::RecvDuelStartPacket()
{
	TPacketGCDuelStart kDuelStartPacket;
	if (!Recv(sizeof(kDuelStartPacket), &kDuelStartPacket))
		return false;
	
	DWORD count = (kDuelStartPacket.wSize - sizeof(kDuelStartPacket))/sizeof(DWORD);

	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();

	CInstanceBase* pkInstMain=rkChrMgr.GetMainInstancePtr();
	if (!pkInstMain)
	{
		TraceError("CPythonNetworkStream::RecvDuelStartPacket - MainCharacter is NULL");
		return false;
	}
	DWORD dwVIDSrc = pkInstMain->GetVirtualID();
	DWORD dwVIDDest;

	for ( DWORD i = 0; i < count; i++)
	{
		Recv(sizeof(dwVIDDest),&dwVIDDest);
		CInstanceBase::InsertDUELKey(dwVIDSrc,dwVIDDest);
	}
	
	if(count == 0)
		pkInstMain->SetDuelMode(CInstanceBase::DUEL_CANNOTATTACK);
	else
		pkInstMain->SetDuelMode(CInstanceBase::DUEL_START);
	
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard", Py_BuildValue("()"));
	
	rkChrMgr.RefreshAllPCTextTail();

	return true;
}

bool CPythonNetworkStream::RecvPVPPacket()
{
	TPacketGCPVP kPVPPacket;
	if (!Recv(sizeof(kPVPPacket), &kPVPPacket))
		return false;

	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();
	CPythonPlayer & rkPlayer = CPythonPlayer::Instance();

	switch (kPVPPacket.bMode)
	{
		case PVP_MODE_AGREE:
			rkChrMgr.RemovePVPKey(kPVPPacket.dwVIDSrc, kPVPPacket.dwVIDDst);

			// ìƒëŒ€ê°€ ë‚˜(Dst)ì—ê²Œ ë™ì˜ë¥¼ êµ¬í–ˆì„ë•Œ
			if (rkPlayer.IsMainCharacterIndex(kPVPPacket.dwVIDDst))
				rkPlayer.RememberChallengeInstance(kPVPPacket.dwVIDSrc);

			// ìƒëŒ€ì—ê²Œ ë™ì˜ë¥¼ êµ¬í•œ ë™ì•ˆì—ëŠ” ëŒ€ê²° ë¶ˆëŠ¥
			if (rkPlayer.IsMainCharacterIndex(kPVPPacket.dwVIDSrc))
				rkPlayer.RememberCantFightInstance(kPVPPacket.dwVIDDst);
			break;
		case PVP_MODE_REVENGE:
		{
			rkChrMgr.RemovePVPKey(kPVPPacket.dwVIDSrc, kPVPPacket.dwVIDDst);

			DWORD dwKiller = kPVPPacket.dwVIDSrc;
			DWORD dwVictim = kPVPPacket.dwVIDDst;

			// ë‚´(victim)ê°€ ìƒëŒ€ì—ê²Œ ë³µìˆ˜í•  ìˆ˜ ìˆì„ë•Œ
			if (rkPlayer.IsMainCharacterIndex(dwVictim))
				rkPlayer.RememberRevengeInstance(dwKiller);

			// ìƒëŒ€(victim)ê°€ ë‚˜ì—ê²Œ ë³µìˆ˜í•˜ëŠ” ë™ì•ˆì—ëŠ” ëŒ€ê²° ë¶ˆëŠ¥
			if (rkPlayer.IsMainCharacterIndex(dwKiller))
				rkPlayer.RememberCantFightInstance(dwVictim);
			break;
		}

		case PVP_MODE_FIGHT:
			rkChrMgr.InsertPVPKey(kPVPPacket.dwVIDSrc, kPVPPacket.dwVIDDst);
			rkPlayer.ForgetInstance(kPVPPacket.dwVIDSrc);
			rkPlayer.ForgetInstance(kPVPPacket.dwVIDDst);
			break;
		case PVP_MODE_NONE:
			rkChrMgr.RemovePVPKey(kPVPPacket.dwVIDSrc, kPVPPacket.dwVIDDst);
			rkPlayer.ForgetInstance(kPVPPacket.dwVIDSrc);
			rkPlayer.ForgetInstance(kPVPPacket.dwVIDDst);
			break;
	}

	// NOTE : PVP í† ê¸€ì‹œ TargetBoard ë¥¼ ì—…ë°ì´íŠ¸ í•©ë‹ˆë‹¤.
	__RefreshTargetBoardByVID(kPVPPacket.dwVIDSrc);
	__RefreshTargetBoardByVID(kPVPPacket.dwVIDDst);

	return true;
}

// DELETEME
/*
void CPythonNetworkStream::__SendWarpPacket()
{
	TPacketCGWarp kWarpPacket;
	kWarpPacket.bHeader=HEADER_GC_WARP;
	if (!Send(sizeof(kWarpPacket), &kWarpPacket))
	{
		return;
	}
}
*/
void CPythonNetworkStream::NotifyHack(const char* c_szMsg)
{
	if (!m_kQue_stHack.empty())
		if (c_szMsg==m_kQue_stHack.back())
			return;

	m_kQue_stHack.push_back(c_szMsg);	
}

bool CPythonNetworkStream::__SendHack(const char* c_szMsg)
{
	Tracen(c_szMsg);
	
	TPacketCGHack kPacketHack;
	kPacketHack.bHeader=HEADER_CG_HACK;
	strncpy(kPacketHack.szBuf, c_szMsg, sizeof(kPacketHack.szBuf)-1);

	if (!Send(sizeof(kPacketHack), &kPacketHack))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendMessengerAddByVIDPacket(DWORD vid)
{
	TPacketCGMessenger packet;
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_ADD_BY_VID;
	if (!Send(sizeof(packet), &packet))
		return false;
	if (!Send(sizeof(vid), &vid))
		return false;
	return SendSequence();
}

bool CPythonNetworkStream::SendMessengerAddByNamePacket(const char * c_szName)
{
	TPacketCGMessenger packet;
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_ADD_BY_NAME;
	if (!Send(sizeof(packet), &packet))
		return false;
	char szName[CHARACTER_NAME_MAX_LEN];
	strncpy(szName, c_szName, CHARACTER_NAME_MAX_LEN-1);
	szName[CHARACTER_NAME_MAX_LEN-1] = '\0'; // #720: ë©”ì‹ ì € ì´ë¦„ ê´€ë ¨ ë²„í¼ ì˜¤ë²„í”Œë¡œìš° ë²„ê·¸ ìˆ˜ì •

	if (!Send(sizeof(szName), &szName))
		return false;
	Tracef(" SendMessengerAddByNamePacket : %s\n", c_szName);
	return SendSequence();
}

bool CPythonNetworkStream::SendMessengerRemovePacket(const char * c_szKey, const char * c_szName)
{
	TPacketCGMessenger packet;
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_REMOVE;
	if (!Send(sizeof(packet), &packet))
		return false;
	char szKey[CHARACTER_NAME_MAX_LEN];
	strncpy(szKey, c_szKey, CHARACTER_NAME_MAX_LEN-1);
	if (!Send(sizeof(szKey), &szKey))
		return false;
	__RefreshTargetBoardByName(c_szName);
	return SendSequence();
}

#ifdef ENABLE_MESSENGER_BLOCK
bool CPythonNetworkStream::SendMessengerAddBlockByVIDPacket(DWORD vid)
{
	TPacketCGMessenger packet;
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_ADD_BLOCK_BY_VID;
	if (!Send(sizeof(packet), &packet))
		return false;
	if (!Send(sizeof(vid), &vid))
		return false;
	return SendSequence();
}

bool CPythonNetworkStream::SendMessengerAddBlockByNamePacket(const char * c_szName)
{
	TPacketCGMessenger packet;
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_ADD_BLOCK_BY_NAME;
	if (!Send(sizeof(packet), &packet))
		return false;
	char szName[CHARACTER_NAME_MAX_LEN];
	strncpy(szName, c_szName, CHARACTER_NAME_MAX_LEN-1);
	szName[CHARACTER_NAME_MAX_LEN-1] = '\0'; // #720: ??? ?? ?? ?? ????? ?? ??

	if (!Send(sizeof(szName), &szName))
		return false;
	Tracef(" SendMessengerAddBlockByNamePacket : %s\n", c_szName);
	return SendSequence();
}

bool CPythonNetworkStream::SendMessengerRemoveBlockPacket(const char * c_szKey, const char * c_szName)
{
	TPacketCGMessenger packet;
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_REMOVE_BLOCK;
	if (!Send(sizeof(packet), &packet))
		return false;
	char szKey[CHARACTER_NAME_MAX_LEN];
	strncpy(szKey, c_szKey, CHARACTER_NAME_MAX_LEN-1);
	if (!Send(sizeof(szKey), &szKey))
		return false;
	__RefreshTargetBoardByName(c_szName);
	return SendSequence();
}
#endif

bool CPythonNetworkStream::SendCharacterStatePacket(const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg)
{
	NANOBEGIN
	if (!__CanActMainInstance())
		return true;

	if (fDstRot < 0.0f)
		fDstRot = 360 + fDstRot;
	else if (fDstRot > 360.0f)
		fDstRot = fmodf(fDstRot, 360.0f);

	// TODO: ë‚˜ì¤‘ì— íŒ¨í‚·ì´ë¦„ì„ ë°”ê¾¸ì
	TPacketCGMove kStatePacket;
	kStatePacket.bHeader = HEADER_CG_CHARACTER_MOVE;
	kStatePacket.bFunc = eFunc;
	kStatePacket.bArg = uArg;
	kStatePacket.bRot = fDstRot/5.0f;
	kStatePacket.lX = long(c_rkPPosDst.x);
	kStatePacket.lY = long(c_rkPPosDst.y);

#ifdef ENABLE_SVSIDE_ANTI_CHEAT
	{
		CInstanceBase * pkInst = CPythonCharacterManager::Instance().GetMainInstancePtr();
		if (eFunc == 3 && uArg > 21){ return true; }
		 
		static UINT c_packet = 0;
		static DWORD t_packet = 0;
		DWORD t = timeGetTime();
		if (t_packet + 500 < t) {
			c_packet = 0;
			t_packet = t;
		}
		c_packet++;
		if (c_packet > 8){ return true; }
		 
		static UINT l_func = 0;
		static DWORD t_f = 0;
		 
		static float f_x = 0.0f;
		static float f_y = 0.0f;
		const D3DXVECTOR3 & cpos = pkInst->GetGraphicThingInstancePtr()->GetPosition();
		if (f_x == cpos.x && f_y == cpos.y && t - t_f < 1000 && l_func == eFunc && (eFunc < 3)) {
			return true;
		}
		f_x = cpos.x;
		f_y = cpos.y;
		 
		if (l_func == 0 && eFunc == 0 && t_f + 10 > t){ return true; }
		l_func = eFunc;
		t_f = t;
		if (cpos.x == 0.0f && cpos.y == -0.0f) {
			kStatePacket.lX = long(c_rkPPosDst.x);
			kStatePacket.lY = long(c_rkPPosDst.y);
		}else{
			kStatePacket.lX = long(cpos.x);
			kStatePacket.lY = long(cpos.y < 0.0f ? cpos.y * -1.0f : cpos.y);
		}
	}
#endif

	kStatePacket.dwTime = ELTimer_GetServerMSec();
	
	assert(kStatePacket.lX >= 0 && kStatePacket.lX < 204800);

	__LocalPositionToGlobalPosition(kStatePacket.lX, kStatePacket.lY);

	if (!Send(sizeof(kStatePacket), &kStatePacket))
	{
		Tracenf("CPythonNetworkStream::SendCharacterStatePacket(dwCmdTime=%u, fDstPos=(%f, %f), fDstRot=%f, eFunc=%d uArg=%d) - PACKET SEND ERROR",
			kStatePacket.dwTime,
			float(kStatePacket.lX),
			float(kStatePacket.lY),
			fDstRot,
			kStatePacket.bFunc,
			kStatePacket.bArg);
		return false;
	}
	NANOEND
	return SendSequence();
}

// NOTE : SlotIndexëŠ” ì„ì‹œ
bool CPythonNetworkStream::SendUseSkillPacket(DWORD dwSkillIndex, DWORD dwTargetVID)
{
	TPacketCGUseSkill UseSkillPacket;
	UseSkillPacket.bHeader = HEADER_CG_USE_SKILL;
	UseSkillPacket.dwVnum = dwSkillIndex;
	UseSkillPacket.dwTargetVID = dwTargetVID;
	if (!Send(sizeof(TPacketCGUseSkill), &UseSkillPacket))
	{
		Tracen("CPythonNetworkStream::SendUseSkillPacket - SEND PACKET ERROR");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendChatPacket(const char * c_szChat, BYTE byType)
{
	if (m_strPhase != "Game")
		return true; //22-02-25 Bvural41 Fix

	if (strlen(c_szChat) == 0)
		return true;

	if (strlen(c_szChat) >= 512)
		return true;

	if (c_szChat[0] == '/')
	{
		if (1 == strlen(c_szChat))
		{
			if (!m_strLastCommand.empty())
				c_szChat = m_strLastCommand.c_str();
		}
		else
		{
			m_strLastCommand = c_szChat;
		}
	}

	if (ClientCommand(c_szChat))
		return true;

	int iTextLen = strlen(c_szChat) + 1;
	TPacketCGChat ChatPacket;
	ChatPacket.header = HEADER_CG_CHAT;
	ChatPacket.length = sizeof(ChatPacket) + iTextLen;
	ChatPacket.type = byType;

	if (!Send(sizeof(ChatPacket), &ChatPacket))
		return false;

	if (!Send(iTextLen, c_szChat))
		return false;

	return SendSequence();
}

//////////////////////////////////////////////////////////////////////////
// Emoticon
void CPythonNetworkStream::RegisterEmoticonString(const char * pcEmoticonString)
{
	if (m_EmoticonStringVector.size() >= CInstanceBase::EMOTICON_NUM)
	{
		TraceError("Can't register emoticon string... vector is full (size:%d)", m_EmoticonStringVector.size() );
		return;
	}
	m_EmoticonStringVector.push_back(pcEmoticonString);
}

bool CPythonNetworkStream::ParseEmoticon(const char * pChatMsg, DWORD * pdwEmoticon)
{
	for (DWORD dwEmoticonIndex = 0; dwEmoticonIndex < m_EmoticonStringVector.size() ; ++dwEmoticonIndex)
	{
		if (strlen(pChatMsg) > m_EmoticonStringVector[dwEmoticonIndex].size())
			continue;

		const char * pcFind = strstr(pChatMsg, m_EmoticonStringVector[dwEmoticonIndex].c_str());

		if (pcFind != pChatMsg)
			continue;

		*pdwEmoticon = dwEmoticonIndex;

		return true;
	}

	return false;
}
// Emoticon
//////////////////////////////////////////////////////////////////////////

void CPythonNetworkStream::__ConvertEmpireText(DWORD dwEmpireID, char* szText)
{
	if (dwEmpireID<1 || dwEmpireID>3)
		return;

	UINT uHanPos;

	STextConvertTable& rkTextConvTable=m_aTextConvTable[dwEmpireID-1];

	BYTE* pbText=(BYTE*)szText;
	while (*pbText)
	{
		if (*pbText & 0x80)
		{
			if (pbText[0]>=0xb0 && pbText[0]<=0xc8 && pbText[1]>=0xa1 && pbText[1]<=0xfe)
			{
				uHanPos=(pbText[0]-0xb0)*(0xfe-0xa1+1)+(pbText[1]-0xa1);
				pbText[0]=rkTextConvTable.aacHan[uHanPos][0];
				pbText[1]=rkTextConvTable.aacHan[uHanPos][1];
			}
			pbText+=2;
		}
		else
		{
			if (*pbText>='a' && *pbText<='z')
			{
				*pbText=rkTextConvTable.acLower[*pbText-'a'];
			}
			else if (*pbText>='A' && *pbText<='Z')
			{
				*pbText=rkTextConvTable.acUpper[*pbText-'A'];
			}
			pbText++;
		}
	}
}

bool CPythonNetworkStream::RecvChatPacket()
{
	TPacketGCChat kChat;
    char buf[1024 + 1];
	char line[1024 + 1];

	if (!Recv(sizeof(kChat), &kChat))
		return false;

	UINT uChatSize=kChat.size - sizeof(kChat);

	if (!Recv(uChatSize, buf))
		return false;

	buf[uChatSize]='\0';
	
	// ìœ ëŸ½ ì•„ë ë²„ì „ ì²˜ë¦¬
	// "ì´ë¦„: ë‚´ìš©" ì…ë ¥ì„ "ë‚´ìš©: ì´ë¦„" ìˆœì„œë¡œ ì¶œë ¥í•˜ê¸° ìœ„í•´ íƒ­(0x08)ì„ ë„£ìŒ
	// íƒ­ì„ ì•„ëì–´ ê¸°í˜¸ë¡œ ì²˜ë¦¬í•´ (ì˜ì–´1) : (ì˜ì–´2) ë¡œ ì…ë ¥ë˜ì–´ë„ (ì˜ì–´2) : (ì˜ì–´1) ë¡œ ì¶œë ¥í•˜ê²Œ ë§Œë“ ë‹¤
	if (LocaleService_IsEUROPE() && GetDefaultCodePage() == 1256)
	{
		char * p = strchr(buf, ':'); 
		if (p && p[1] == ' ')
			p[1] = 0x08;
	}

	if (kChat.type >= CHAT_TYPE_MAX_NUM)
		return true;

	if (CHAT_TYPE_COMMAND == kChat.type)
	{
		ServerCommand(buf);
		return true;
	}

	if (kChat.dwVID != 0)
	{
		CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
		CInstanceBase * pkInstChatter = rkChrMgr.GetInstancePtr(kChat.dwVID);
		if (NULL == pkInstChatter)
			return true;
		
		switch (kChat.type)
		{
		case CHAT_TYPE_TALKING:  /* ê·¸ëƒ¥ ì±„íŒ… */
		case CHAT_TYPE_PARTY:    /* íŒŒí‹°ë§ */
		case CHAT_TYPE_GUILD:    /* ê¸¸ë“œë§ */
		case CHAT_TYPE_SHOUT:	/* ì™¸ì¹˜ê¸° */
		case CHAT_TYPE_WHISPER:	// ì„œë²„ì™€ëŠ” ì—°ë™ë˜ì§€ ì•ŠëŠ” Only Client Enum
			{
				char * p = strchr(buf, ':');

				if (p)
					p += 2;
				else
					p = buf;

				DWORD dwEmoticon;

				if (ParseEmoticon(p, &dwEmoticon))
				{
					pkInstChatter->SetEmoticon(dwEmoticon);
					return true;
				}
				else
				{
					if (gs_bEmpireLanuageEnable)
					{
						CInstanceBase* pkInstMain=rkChrMgr.GetMainInstancePtr();
						if (pkInstMain)
							if (!pkInstMain->IsSameEmpire(*pkInstChatter))
								__ConvertEmpireText(pkInstChatter->GetEmpireID(), p);
					}

					if (m_isEnableChatInsultFilter)
					{
						if (false == pkInstChatter->IsNPC() && false == pkInstChatter->IsEnemy())
						{
							__FilterInsult(p, strlen(p));
						}
					}

					_snprintf(line, sizeof(line), "%s", p);
				}
			}
			break;
		case CHAT_TYPE_COMMAND:
		case CHAT_TYPE_INFO:
		case CHAT_TYPE_NOTICE:
		case CHAT_TYPE_BIG_NOTICE:
#ifdef ENABLE_OX_RENEWAL
		case CHAT_TYPE_CONTROL_NOTICE:
#endif
#ifdef ENABLE_DICE_SYSTEM
		case CHAT_TYPE_DICE_INFO:
#endif
#ifdef ENABLE_12ZI
		case CHAT_TYPE_ZODIAC_NOTICE:
		case CHAT_TYPE_ZODIAC_SUB_NOTICE:
		case CHAT_TYPE_ZODIAC_NOTICE_CLEAR:
#endif
#ifdef ENABLE_CHAT_SETTINGS_EXTEND
		case CHAT_TYPE_EXP_INFO:
		case CHAT_TYPE_ITEM_INFO:
		case CHAT_TYPE_MONEY_INFO:
#endif
		case CHAT_TYPE_MAX_NUM:
		default:
			_snprintf(line, sizeof(line), "%s", buf);
			break;
		}

		if (CHAT_TYPE_SHOUT != kChat.type)
		{
			CPythonTextTail::Instance().RegisterChatTail(kChat.dwVID, line);
		}

#ifdef ENABLE_12ZI
		if (pkInstChatter->IsPC() && (CHAT_TYPE_ZODIAC_NOTICE != kChat.type && CHAT_TYPE_ZODIAC_SUB_NOTICE != kChat.type && CHAT_TYPE_ZODIAC_NOTICE_CLEAR != kChat.type))
			CPythonChat::Instance().AppendChat(kChat.type, buf);
#else
		if (pkInstChatter->IsPC())
			CPythonChat::Instance().AppendChat(kChat.type, buf);
#endif
	}
	else
	{
		if (CHAT_TYPE_NOTICE == kChat.type)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetTipMessage", Py_BuildValue("(s)", buf));
		}
		else if (CHAT_TYPE_BIG_NOTICE == kChat.type)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetBigMessage", Py_BuildValue("(s)", buf));
		}
		else if (CHAT_TYPE_SHOUT == kChat.type)
		{
			char * p = strchr(buf, ':');

			if (p)
			{
				if (m_isEnableChatInsultFilter)
					__FilterInsult(p, strlen(p));
			}
		}
#ifdef ENABLE_OX_RENEWAL
		else if (CHAT_TYPE_CONTROL_NOTICE == kChat.type)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetBigControlMessage", Py_BuildValue("(s)", buf));
		}
#endif
#ifdef ENABLE_12ZI
		else if (CHAT_TYPE_ZODIAC_NOTICE == kChat.type)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetMissionMessage", Py_BuildValue("(s)", buf));
			return true;
		}
		else if (CHAT_TYPE_ZODIAC_SUB_NOTICE == kChat.type)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetSubMissionMessage", Py_BuildValue("(s)", buf));
			return true;
		}
		else if (CHAT_TYPE_ZODIAC_NOTICE_CLEAR == kChat.type)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_CleanMissionMessage", Py_BuildValue("()"));
			return true;
		}
#endif

		CPythonChat::Instance().AppendChat(kChat.type, buf);
	}
	return true;
}

bool CPythonNetworkStream::RecvWhisperPacket()
{
	TPacketGCWhisper whisperPacket;
	char buf[512 + 1];

	if (!Recv(sizeof(whisperPacket), &whisperPacket))
		return false;

	assert(whisperPacket.wSize - sizeof(whisperPacket) < 512);

	if (!Recv(whisperPacket.wSize - sizeof(whisperPacket), &buf))
		return false;

	buf[whisperPacket.wSize - sizeof(whisperPacket)] = '\0';

#ifdef ENABLE_WHISPER_RENEWAL
	#include "PythonWhisper.h"
	if (std::string(buf).find("|?whisper_renewal>|") != std::string::npos) {
		CPythonWhisper::Instance().AddName(whisperPacket.szNameFrom, CPythonWhisper::TARGET);
		return true;
	}
	else if (std::string(buf).find("|?whisper_renewal<|") != std::string::npos) {
		CPythonWhisper::Instance().DeleteName(whisperPacket.szNameFrom, CPythonWhisper::TARGET);
		return true;
	}
#endif

	static char line[256];
	if (CPythonChat::WHISPER_TYPE_CHAT == whisperPacket.bType || CPythonChat::WHISPER_TYPE_GM == whisperPacket.bType)
	{
		_snprintf(line, sizeof(line), "%s : %s", whisperPacket.szNameFrom, buf);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnRecvWhisper", Py_BuildValue("(iss)", (int) whisperPacket.bType, whisperPacket.szNameFrom, line));
	}
	else if (CPythonChat::WHISPER_TYPE_SYSTEM == whisperPacket.bType || CPythonChat::WHISPER_TYPE_ERROR == whisperPacket.bType)
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnRecvWhisperSystemMessage", Py_BuildValue("(iss)", (int) whisperPacket.bType, whisperPacket.szNameFrom, buf));
	}
	else
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnRecvWhisperError", Py_BuildValue("(iss)", (int) whisperPacket.bType, whisperPacket.szNameFrom, buf));
	}

	return true;
}

bool CPythonNetworkStream::SendWhisperPacket(const char * name, const char * c_szChat)
{
	if (strlen(c_szChat) >= 255)
		return true;

	int iTextLen = strlen(c_szChat) + 1;
	TPacketCGWhisper WhisperPacket;
	WhisperPacket.bHeader = HEADER_CG_WHISPER;
	WhisperPacket.wSize = sizeof(WhisperPacket) + iTextLen;

	strncpy(WhisperPacket.szNameTo, name, sizeof(WhisperPacket.szNameTo) - 1);

	if (!Send(sizeof(WhisperPacket), &WhisperPacket))
		return false;

	if (!Send(iTextLen, c_szChat))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendMobileMessagePacket(const char * name, const char * c_szChat)
{
	int iTextLen = strlen(c_szChat) + 1;
	TPacketCGSMS SMSPacket;
	SMSPacket.bHeader = HEADER_CG_SMS;
	SMSPacket.wSize = sizeof(SMSPacket) + iTextLen;

	strncpy(SMSPacket.szNameTo, name, sizeof(SMSPacket.szNameTo) - 1);

	if (!Send(sizeof(SMSPacket), &SMSPacket))
		return false;

	if (!Send(iTextLen, c_szChat))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvPointChange()
{
	TPacketGCPointChange PointChange;

	if (!Recv(sizeof(TPacketGCPointChange), &PointChange))
	{
		Tracen("Recv Point Change Packet Error");
		return false;
	}

	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	rkChrMgr.ShowPointEffect(PointChange.Type, PointChange.dwVID);

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();

	if (pInstance)
	if (PointChange.dwVID == pInstance->GetVirtualID())
	{
		CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
		rkPlayer.SetStatus(PointChange.Type, PointChange.value);

		switch (PointChange.Type)
		{
			case POINT_STAT_RESET_COUNT:
				__RefreshStatus();
				break;
#ifdef ENABLE_IMPROVED_LOGOUT_POINTS
			case POINT_PLAYTIME:
				m_akSimplePlayerInfo[m_dwSelectedCharacterIndex].dwPlayMinutes = PointChange.value;
				break;
#endif
			case POINT_LEVEL:
#ifdef ENABLE_IMPROVED_LOGOUT_POINTS
				m_akSimplePlayerInfo[m_dwSelectedCharacterIndex].byLevel = PointChange.value;
				__RefreshStatus();
				__RefreshSkillWindow();
				break;
#endif
			case POINT_ST:
#ifdef ENABLE_IMPROVED_LOGOUT_POINTS
				m_akSimplePlayerInfo[m_dwSelectedCharacterIndex].byST = PointChange.value;
				__RefreshStatus();
				__RefreshSkillWindow();
				break;
#endif
			case POINT_DX:
#ifdef ENABLE_IMPROVED_LOGOUT_POINTS
				m_akSimplePlayerInfo[m_dwSelectedCharacterIndex].byDX = PointChange.value;
				__RefreshStatus();
				__RefreshSkillWindow();
				break;
#endif
			case POINT_HT:
#ifdef ENABLE_IMPROVED_LOGOUT_POINTS
				m_akSimplePlayerInfo[m_dwSelectedCharacterIndex].byHT = PointChange.value;
				__RefreshStatus();
				__RefreshSkillWindow();
				break;
#endif
			case POINT_IQ:
#ifdef ENABLE_IMPROVED_LOGOUT_POINTS
				m_akSimplePlayerInfo[m_dwSelectedCharacterIndex].byIQ = PointChange.value;
				__RefreshStatus();
				__RefreshSkillWindow();
				break;
#else
				__RefreshStatus();
				__RefreshSkillWindow();
				break;
#endif
#ifdef ENABLE_CONQUEROR_LEVEL
			case POINT_CONQUEROR_LEVEL:
			case POINT_SUNGMA_STR:
			case POINT_SUNGMA_HP:
			case POINT_SUNGMA_MOVE:
			case POINT_SUNGMA_INMUNE:
#endif
			case POINT_SKILL:
			case POINT_SUB_SKILL:
			case POINT_HORSE_SKILL:
				__RefreshSkillWindow();
				break;
			case POINT_ENERGY:
				if (PointChange.value == 0)
				{
					rkPlayer.SetStatus(POINT_ENERGY_END_TIME, 0);
				}
				__RefreshStatus();
				break;
			default:
				__RefreshStatus();
				break;
		}

		if (POINT_GOLD == PointChange.Type)
		{
			if (PointChange.amount > 0)
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnPickMoney", Py_BuildValue("(i)", PointChange.amount));
			}
		}

#ifdef ENABLE_CHEQUE_SYSTEM
		else if (POINT_CHEQUE == PointChange.Type)
		{
			if (PointChange.amount > 0)
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnPickCheque", Py_BuildValue("(i)", PointChange.amount));
			}
		}
#endif
#ifdef ENABLE_GAYA_SYSTEM
		else if (POINT_GEM == PointChange.Type)
		{
			if (PointChange.amount > 0)
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnPickGem", Py_BuildValue("(i)", PointChange.amount));
			}
		}
#endif
#ifdef ENABLE_STONE_POINT_SYSTEM
		else if (POINT_STONE == PointChange.Type)
		{
			if (PointChange.amount > 0)
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnPickStonePoint", Py_BuildValue("(i)", PointChange.amount));
			}
		}
#endif
#ifdef ENABLE_GUILD_DONATE_SYSTEM
		else if (POINT_MEDAL_HONOR == PointChange.Type)
		{
			if (PointChange.amount > 0)
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnPickMedalHonor", Py_BuildValue("(i)", PointChange.amount));
			}
		}
#endif
		else if (POINT_EXP == PointChange.Type)
		{
			if (PointChange.amount > 0)
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnPickExp", Py_BuildValue("(i)", PointChange.amount));
			}
		}
#ifdef ENABLE_CONQUEROR_LEVEL
		else if (POINT_CONQUEROR_EXP == PointChange.Type)
		{
			if (PointChange.amount > 0)
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnPickExpConqueror", Py_BuildValue("(i)", PointChange.amount));
			}
		}
#endif
	}
#ifdef ENABLE_TEXT_LEVEL_REFRESH
	else
	{
		pInstance = CPythonCharacterManager::Instance().GetInstancePtr(PointChange.dwVID);
		if (pInstance && PointChange.Type == POINT_LEVEL)
		{
			pInstance->SetLevel(PointChange.value);
			pInstance->UpdateTextTailLevel(PointChange.value);
		}
#ifdef ENABLE_CONQUEROR_LEVEL
		else if (pInstance && PointChange.Type == POINT_CONQUEROR_LEVEL)
		{
			pInstance->SetLevel(PointChange.value);
			pInstance->UpdateTextTailConquerorLevel(PointChange.value);
		}
#endif
	}
#endif
	return true;
}

bool CPythonNetworkStream::RecvStunPacket()
{
	TPacketGCStun StunPacket;

	if (!Recv(sizeof(StunPacket), &StunPacket))
	{
		Tracen("CPythonNetworkStream::RecvStunPacket Error");
		return false;
	}

	//Tracef("RecvStunPacket %d\n", StunPacket.vid);

	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	CInstanceBase * pkInstSel = rkChrMgr.GetInstancePtr(StunPacket.vid);

	if (pkInstSel)
	{
		if (CPythonCharacterManager::Instance().GetMainInstancePtr()==pkInstSel)
			pkInstSel->Die();
		else
			pkInstSel->Stun();
	}

	return true;
}

bool CPythonNetworkStream::RecvDeadPacket()
{
	TPacketGCDead DeadPacket;
	if (!Recv(sizeof(DeadPacket), &DeadPacket))
	{
		Tracen("CPythonNetworkStream::RecvDeadPacket Error");
		return false;
	}

	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	CInstanceBase * pkChrInstSel = rkChrMgr.GetInstancePtr(DeadPacket.vid);
	if (pkChrInstSel)
	{
		CInstanceBase* pkInstMain=rkChrMgr.GetMainInstancePtr();
		if (pkInstMain==pkChrInstSel)
		{
			Tracenf("ì£¼ì¸ê³µ ì‚¬ë§");
#ifdef ENABLE_AUTO_SYSTEM
			if (CPythonPlayer::Instance().AutoStatus())
			{
#ifdef ENABLE_AUTO_RESTART_EVENT
				if (CPythonPlayer::Instance().GetAutoRestart())
				{
					EventHandler::Instance().AddEvent("AutoHuntRestart", std::bind(&SendAutoHuntRestart), 10, 1); // 60-> 1 min, -1 ->endless event
				}
#endif
			}
#endif
			if (false == pkInstMain->GetDuelMode())
			{
#ifdef ENABLE_ELEMENTAL_WORLD
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnGameOver", Py_BuildValue("(i)", DeadPacket.IMapIdx));
#else
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnGameOver", Py_BuildValue("()"));
#endif
			}
			CPythonPlayer::Instance().NotifyDeadMainCharacter();
		}

		pkChrInstSel->Die();
	}

	return true;
}

bool CPythonNetworkStream::SendCharacterPositionPacket(BYTE iPosition)
{
	TPacketCGPosition PositionPacket;

	PositionPacket.header = HEADER_CG_CHARACTER_POSITION;
	PositionPacket.position = iPosition;

	if (!Send(sizeof(TPacketCGPosition), &PositionPacket))
	{
		Tracen("Send Character Position Packet Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendOnClickPacket(DWORD vid)
{
	TPacketCGOnClick OnClickPacket;
	OnClickPacket.header	= HEADER_CG_ON_CLICK;
	OnClickPacket.vid		= vid;

	if (!Send(sizeof(OnClickPacket), &OnClickPacket))
	{
		Tracen("Send On_Click Packet Error");
		return false;
	}

	Tracef("SendOnClickPacket\n");
	return SendSequence();
}

bool CPythonNetworkStream::RecvCharacterPositionPacket()
{
	TPacketGCPosition PositionPacket;

	if (!Recv(sizeof(TPacketGCPosition), &PositionPacket))
		return false;

	CInstanceBase * pChrInstance = CPythonCharacterManager::Instance().GetInstancePtr(PositionPacket.vid);

	if (!pChrInstance)
		return true;

	//pChrInstance->UpdatePosition(PositionPacket.position);

	return true;
}

bool CPythonNetworkStream::RecvMotionPacket()
{
	TPacketGCMotion MotionPacket;

	if (!Recv(sizeof(TPacketGCMotion), &MotionPacket))
		return false;

	CInstanceBase * pMainInstance = CPythonCharacterManager::Instance().GetInstancePtr(MotionPacket.vid);
	CInstanceBase * pVictimInstance = NULL;

	if (0 != MotionPacket.victim_vid)
		pVictimInstance = CPythonCharacterManager::Instance().GetInstancePtr(MotionPacket.victim_vid);

	if (!pMainInstance)
		return false;

#ifdef ENABLE_BALATHOR_DUNGEON
	if (pVictimInstance)
		pVictimInstance->PushOnceMotion(MotionPacket.motion);
#endif

	return true;
}

bool CPythonNetworkStream::RecvShopPacket()
{
	std::vector<char> vecBuffer;
	vecBuffer.clear();

	TPacketGCShop  packet_shop;
	if (!Recv(sizeof(packet_shop), &packet_shop))
		return false;

	int iSize = packet_shop.size - sizeof(packet_shop);
	if (iSize > 0)
	{
		vecBuffer.resize(iSize);
		if (!Recv(iSize, &vecBuffer[0]))
			return false;
	}

	switch (packet_shop.subheader)
	{
		case SHOP_SUBHEADER_GC_START:
			{
				CPythonShop::Instance().Clear();

				TPacketGCShopStart * pShopStartPacket = (TPacketGCShopStart *)&vecBuffer[0];
				DWORD dwVID = pShopStartPacket->owner_vid;

				for (BYTE iItemIndex = 0; iItemIndex < SHOP_HOST_ITEM_MAX_NUM; ++iItemIndex)
				{
					CPythonShop::Instance().SetItemData(iItemIndex, pShopStartPacket->items[iItemIndex]);
				}

#ifdef ENABLE_TYPE_SHOPEX_SYSTEM
				CPythonShop::Instance().SetPriceType(pShopStartPacket->price_type);
#endif

#ifdef ENABLE_12ZI
				CPythonShop::Instance().SetLimitedItemShop(pShopStartPacket->islimiteditemshop);
#endif

#ifdef ENABLE_STONE_POINT_SYSTEM
				CPythonShop::Instance().SetStonePointShop(pShopStartPacket->isstonepointshop);
#endif

#ifdef ENABLE_GUILD_DONATE_SYSTEM
				CPythonShop::Instance().SetMedalHonorShop(pShopStartPacket->ismedalhonorshop);
#endif

#ifdef ENABLE_CHEQUE_DESK_SYSTEM
				CPythonShop::Instance().SetChequeDeskShop(pShopStartPacket->ischequedesknpc);
#endif

#ifdef ENABLE_BATTLE_FIELD
				CPythonShop::instance().SetTabCoinType(0, SHOP_COIN_TYPE_GOLD);
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartShop", Py_BuildValue("(iiii)", dwVID, 0, 0, 0));
#else
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartShop", Py_BuildValue("(i)", dwVID));
#endif
				__RefreshInventoryWindow();
			}
			break;

		case SHOP_SUBHEADER_GC_START_EX:
			{
				CPythonShop::Instance().Clear();

				TPacketGCShopStartEx * pShopStartPacket = (TPacketGCShopStartEx *)&vecBuffer[0];
				size_t read_point = sizeof(TPacketGCShopStartEx);

				DWORD dwVID = pShopStartPacket->owner_vid;
				BYTE shop_tab_count = pShopStartPacket->shop_tab_count;
#ifdef ENABLE_BATTLE_FIELD
				DWORD dwPoints = pShopStartPacket->points;
				DWORD dwCurLimit = pShopStartPacket->curLimit;
				DWORD dwMaxLimit = pShopStartPacket->maxLimit;
#endif

				CPythonShop::instance().SetTabCount(shop_tab_count);
				
				for (size_t i = 0; i < shop_tab_count; i++)
				{
					TPacketGCShopStartEx::TSubPacketShopTab* pPackTab = (TPacketGCShopStartEx::TSubPacketShopTab*)&vecBuffer[read_point];
					read_point += sizeof(TPacketGCShopStartEx::TSubPacketShopTab);
					
					CPythonShop::instance().SetTabCoinType(i, pPackTab->coin_type);
					CPythonShop::instance().SetTabName(i, pPackTab->name);

					struct packet_shop_item* item = &pPackTab->items[0];
					
					for (BYTE j = 0; j < SHOP_HOST_ITEM_MAX_NUM; j++)
					{
						TShopItemData* itemData = (item + j);
						CPythonShop::Instance().SetItemData(i, j, *itemData);
					}
				}

#ifdef ENABLE_BATTLE_FIELD
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartShop", Py_BuildValue("(iiii)", dwVID, dwPoints, dwCurLimit, dwMaxLimit));
#else
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartShop", Py_BuildValue("(i)", dwVID));
#endif
			}
			break;


		case SHOP_SUBHEADER_GC_END:
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "EndShop", Py_BuildValue("()"));
				__RefreshInventoryWindow();
			}
			break;

		case SHOP_SUBHEADER_GC_UPDATE_ITEM:
			{
				TPacketGCShopUpdateItem * pShopUpdateItemPacket = (TPacketGCShopUpdateItem *)&vecBuffer[0];
				CPythonShop::Instance().SetItemData(pShopUpdateItemPacket->pos, pShopUpdateItemPacket->item);
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshShop", Py_BuildValue("()"));
			}
			break;

		case SHOP_SUBHEADER_GC_UPDATE_PRICE:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetShopSellingPrice", Py_BuildValue("(i)", *(int *)&vecBuffer[0]));
			break;
			
		case SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_MONEY"));
			break;

		case SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY_EX:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_MONEY_EX"));
			break;

#ifdef ENABLE_BATTLE_FIELD
		case SHOP_SUBHEADER_GC_NOT_ENOUGH_POINTS:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_POINTS"));
			break;
			
		case SHOP_SUBHEADER_GC_MAX_LIMIT_POINTS:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "MAX_LIMIT_POINTS"));
			break;
			
		case SHOP_SUBHEADER_GC_OVERFLOW_LIMIT_POINTS:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "OVERFLOW_LIMIT_POINTS"));
			break;
#endif

		case SHOP_SUBHEADER_GC_SOLDOUT:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "SOLDOUT"));
			break;

		case SHOP_SUBHEADER_GC_INVENTORY_FULL:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "INVENTORY_FULL"));
			break;

		case SHOP_SUBHEADER_GC_INVALID_POS:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "INVALID_POS"));
			break;

#ifdef ENABLE_CHEQUE_SYSTEM
		case SHOP_SUBHEADER_GC_NOT_ENOUGH_CHEQUE:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_CHEQUE"));
			break;

		case SHOP_SUBHEADER_GC_NOT_ENOUGH_CHEQUE_MONEY:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_CHEQUE_MONEY"));
			break;
#endif

#ifdef ENABLE_BUY_WITH_ITEM
		case SHOP_SUBHEADER_GC_NOT_ENOUGH_ITEM:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_ITEM"));
			break;
#endif

#ifdef ENABLE_12ZI
		case SHOP_SUBHEADER_GC_NOT_ENOUGH_ZODIAC_ITEM:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_ZODIAC_ITEMS"));
			break;
#endif

#ifdef ENABLE_CHEQUE_DESK_SYSTEM
		case SHOP_SUBHEADER_GC_NOT_ITEM_LIMIT:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "SHOP_SUBHEADER_GC_NOT_ITEM_LIMIT"));
			break;
			
		case SHOP_SUBHEADER_GC_NOT_PLAYER_ITEM_LIMIT:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "SHOP_SUBHEADER_GC_NOT_PLAYER_ITEM_LIMIT"));
			break;
			
		case SHOP_SUBHEADER_GC_NOT_GLOBAL_ITEM_LIMIT:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "SHOP_SUBHEADER_GC_NOT_GLOBAL_ITEM_LIMIT"));
			break;
			
		case SHOP_SUBHEADER_GC_NO_EVENT_TIME:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "SHOP_SUBHEADER_GC_NO_EVENT_TIME"));
			break;
			
		case SHOP_SUBHEADER_GC_NOT_LEVEL:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "SHOP_SUBHEADER_GC_NOT_LEVEL"));
			break;
#endif

#ifdef ENABLE_STONE_POINT_SYSTEM
		case SHOP_SUBHEADER_GC_NOT_ENOUGH_STONE_POINT:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_STONE_POINT"));
			break;
#endif

#ifdef ENABLE_GUILD_DONATE_SYSTEM
		case SHOP_SUBHEADER_GC_NOT_ENOUGH_MEDAL_HONOR_POINT:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_MEDAL_HONOR_POINT"));
			break;
			
		case SHOP_SUBHEADER_GC_NOT_ENOUGH_MEDAL_HONOR_POINT_LIMIT:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_MEDAL_HONOR_POINT_LIMIT"));
			break;
#endif

		default:
			TraceError("CPythonNetworkStream::RecvShopPacket: Unknown subheader\n");
			break;
	}

	return true;
}

bool CPythonNetworkStream::RecvExchangePacket()
{
	TPacketGCExchange exchange_packet;

	if (!Recv(sizeof(exchange_packet), &exchange_packet))
		return false;

	switch (exchange_packet.subheader)
	{
		case EXCHANGE_SUBHEADER_GC_START:
			CPythonExchange::Instance().Clear();
			CPythonExchange::Instance().Start();
			CPythonExchange::Instance().SetSelfName(CPythonPlayer::Instance().GetName());
			{
				CInstanceBase * pCharacterInstance = CPythonCharacterManager::Instance().GetInstancePtr(exchange_packet.arg1);

				if (pCharacterInstance)
				{
					CPythonExchange::Instance().SetTargetName(pCharacterInstance->GetNameString());
				}
			}

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartExchange", Py_BuildValue("()"));
			__RefreshInventoryWindow();
			break;

		case EXCHANGE_SUBHEADER_GC_ITEM_ADD:
			if (exchange_packet.is_me)
			{
				int iSlotIndex = exchange_packet.arg2.cell;
#ifdef ENABLE_EXTENDED_ITEM_COUNT
				CPythonExchange::Instance().SetItemToSelf(iSlotIndex, exchange_packet.arg1, (WORD)exchange_packet.arg3);
#else
				CPythonExchange::Instance().SetItemToSelf(iSlotIndex, exchange_packet.arg1, (BYTE)exchange_packet.arg3);
#endif
				for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
					CPythonExchange::Instance().SetItemMetinSocketToSelf(iSlotIndex, i, exchange_packet.alValues[i]);
#ifdef ENABLE_GLOVE_SYSTEM
				for (int j = 0; j < ITEM_APPLY_RANDOM_SLOT_MAX_NUM; ++j)
					CPythonExchange::Instance().SetItemApplyRandomToSelf(iSlotIndex, j, exchange_packet.aApplyRandom[j].bType, exchange_packet.aApplyRandom[j].sValue);
#endif
				for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
					CPythonExchange::Instance().SetItemAttributeToSelf(iSlotIndex, j, exchange_packet.aAttr[j].bType, exchange_packet.aAttr[j].sValue);
#ifdef ENABLE_CHANGELOOK_SYSTEM
				CPythonExchange::Instance().SetItemTransmutation(iSlotIndex, exchange_packet.dwTransmutation, true);
#endif
#ifdef ENABLE_REFINE_ELEMENT
				CPythonExchange::Instance().SetItemRefineElementToSelf(iSlotIndex, exchange_packet.dwRefineElement);
#endif
#ifdef ENABLE_SET_ITEM
				CPythonExchange::Instance().SetItemSetValue(iSlotIndex, exchange_packet.set_value, true);
#endif
#ifdef ENABLE_TRADABLE_ICON
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AddExchangeItemSlotIndex", Py_BuildValue("(i)", exchange_packet.arg4.cell));
#endif
			}
			else
			{
				int iSlotIndex = exchange_packet.arg2.cell;
#ifdef ENABLE_EXTENDED_ITEM_COUNT
				CPythonExchange::Instance().SetItemToTarget(iSlotIndex, exchange_packet.arg1, (WORD)exchange_packet.arg3);
#else
				CPythonExchange::Instance().SetItemToTarget(iSlotIndex, exchange_packet.arg1, (BYTE)exchange_packet.arg3);
#endif
				for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
					CPythonExchange::Instance().SetItemMetinSocketToTarget(iSlotIndex, i, exchange_packet.alValues[i]);
#ifdef ENABLE_GLOVE_SYSTEM
				for (int j = 0; j < ITEM_APPLY_RANDOM_SLOT_MAX_NUM; ++j)
					CPythonExchange::Instance().SetItemApplyRandomToTarget(iSlotIndex, j, exchange_packet.aApplyRandom[j].bType, exchange_packet.aApplyRandom[j].sValue);
#endif
				for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
					CPythonExchange::Instance().SetItemAttributeToTarget(iSlotIndex, j, exchange_packet.aAttr[j].bType, exchange_packet.aAttr[j].sValue);
#ifdef ENABLE_CHANGELOOK_SYSTEM
				CPythonExchange::Instance().SetItemTransmutation(iSlotIndex, exchange_packet.dwTransmutation, true);
#endif
#ifdef ENABLE_REFINE_ELEMENT
				CPythonExchange::Instance().SetItemRefineElementToTarget(iSlotIndex, exchange_packet.dwRefineElement);
#endif
#ifdef ENABLE_SET_ITEM
				CPythonExchange::Instance().SetItemSetValue(iSlotIndex, exchange_packet.set_value, false);
#endif
			}

			__RefreshExchangeWindow();
			__RefreshInventoryWindow();
			break;

		case EXCHANGE_SUBHEADER_GC_ITEM_DEL:
			if (exchange_packet.is_me)
			{
#ifdef ENABLE_EXTENDED_ITEM_COUNT
				CPythonExchange::Instance().DelItemOfSelf((WORD)exchange_packet.arg1);
#else
				CPythonExchange::Instance().DelItemOfSelf((BYTE)exchange_packet.arg1);
#endif
			}
			else
			{
#ifdef ENABLE_EXTENDED_ITEM_COUNT
				CPythonExchange::Instance().DelItemOfTarget((WORD)exchange_packet.arg1);
#else
				CPythonExchange::Instance().DelItemOfTarget((BYTE)exchange_packet.arg1);
#endif
			}
			__RefreshExchangeWindow();
			__RefreshInventoryWindow();
			break;

		case EXCHANGE_SUBHEADER_GC_ELK_ADD:
			if (exchange_packet.is_me)
				CPythonExchange::Instance().SetElkToSelf(exchange_packet.arg1);
			else
				CPythonExchange::Instance().SetElkToTarget(exchange_packet.arg1);

			__RefreshExchangeWindow();
			break;

#ifdef ENABLE_CHEQUE_SYSTEM
		case EXCHANGE_SUBHEADER_GC_WON_ADD:
			if (exchange_packet.is_me)
				CPythonExchange::Instance().SetWonToSelf(exchange_packet.arg1);
			else
				CPythonExchange::Instance().SetWonToTarget(exchange_packet.arg1);

			__RefreshExchangeWindow();
			break;
#endif

		case EXCHANGE_SUBHEADER_GC_ACCEPT:
			if (exchange_packet.is_me)
			{
				CPythonExchange::Instance().SetAcceptToSelf((BYTE) exchange_packet.arg1);
			}
			else
			{
				CPythonExchange::Instance().SetAcceptToTarget((BYTE) exchange_packet.arg1);
			}
			__RefreshExchangeWindow();
			break;

		case EXCHANGE_SUBHEADER_GC_END:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "EndExchange", Py_BuildValue("()"));
			__RefreshInventoryWindow();
			CPythonExchange::Instance().End();
			break;

		case EXCHANGE_SUBHEADER_GC_ALREADY:
			Tracef("trade_already");
			break;

		case EXCHANGE_SUBHEADER_GC_LESS_ELK:
			Tracef("trade_less_elk");
			break;
	};

	return true;
}

bool CPythonNetworkStream::RecvQuestInfoPacket()
{
	TPacketGCQuestInfo QuestInfo;

	if (!Peek(sizeof(TPacketGCQuestInfo), &QuestInfo))
	{
		Tracen("Recv Quest Info Packet Error #1");
		return false;
	}

	if (!Peek(QuestInfo.size))
	{
		Tracen("Recv Quest Info Packet Error #2");
		return false;
	}

	Recv(sizeof(TPacketGCQuestInfo));

	const BYTE & c_rFlag = QuestInfo.flag;

	enum
	{
		QUEST_PACKET_TYPE_NONE,
		QUEST_PACKET_TYPE_BEGIN,
		QUEST_PACKET_TYPE_UPDATE,
		QUEST_PACKET_TYPE_END,
	};

	BYTE byQuestPacketType = QUEST_PACKET_TYPE_NONE;

	if (0 != (c_rFlag & QUEST_SEND_IS_BEGIN))
	{
		BYTE isBegin;
		if (!Recv(sizeof(isBegin), &isBegin))
			return false;

		if (isBegin)
			byQuestPacketType = QUEST_PACKET_TYPE_BEGIN;
		else
			byQuestPacketType = QUEST_PACKET_TYPE_END;
	}
	else
	{
		byQuestPacketType = QUEST_PACKET_TYPE_UPDATE;
	}

	// Recv Data Start
	char szTitle[30 + 1] = "";
	char szClockName[16 + 1] = "";
	int iClockValue = 0;
	char szCounterName[16 + 1] = "";
	int iCounterValue = 0;
	char szIconFileName[24 + 1] = "";

	if (0 != (c_rFlag & QUEST_SEND_TITLE))
	{
		if (!Recv(sizeof(szTitle), &szTitle))
			return false;

		szTitle[30]='\0';
	}
	if (0 != (c_rFlag & QUEST_SEND_CLOCK_NAME))
	{
		if (!Recv(sizeof(szClockName), &szClockName))
			return false;

		szClockName[16]='\0';
	}
	if (0 != (c_rFlag & QUEST_SEND_CLOCK_VALUE))
	{
		if (!Recv(sizeof(iClockValue), &iClockValue))
			return false;
	}
	if (0 != (c_rFlag & QUEST_SEND_COUNTER_NAME))
	{
		if (!Recv(sizeof(szCounterName), &szCounterName))
			return false;

		szCounterName[16]='\0';
	}
	if (0 != (c_rFlag & QUEST_SEND_COUNTER_VALUE))
	{
		if (!Recv(sizeof(iCounterValue), &iCounterValue))
			return false;
	}
	if (0 != (c_rFlag & QUEST_SEND_ICON_FILE))
	{
		if (!Recv(sizeof(szIconFileName), &szIconFileName))
			return false;

		szIconFileName[24]='\0';
	}
	// Recv Data End

	CPythonQuest& rkQuest=CPythonQuest::Instance();

	// Process Start
	if (QUEST_PACKET_TYPE_END == byQuestPacketType)
	{
		rkQuest.DeleteQuestInstance(QuestInfo.index);
	}
	else if (QUEST_PACKET_TYPE_UPDATE == byQuestPacketType)
	{
		if (!rkQuest.IsQuest(QuestInfo.index))
		{
#ifdef ENABLE_QUEST_RENEWAL
			rkQuest.MakeQuest(QuestInfo.index, QuestInfo.c_index);
#else
			rkQuest.MakeQuest(QuestInfo.index);
#endif
		}

		if (strlen(szTitle) > 0)
			rkQuest.SetQuestTitle(QuestInfo.index, szTitle);
		if (strlen(szClockName) > 0)
			rkQuest.SetQuestClockName(QuestInfo.index, szClockName);
		if (strlen(szCounterName) > 0)
			rkQuest.SetQuestCounterName(QuestInfo.index, szCounterName);
		if (strlen(szIconFileName) > 0)
			rkQuest.SetQuestIconFileName(QuestInfo.index, szIconFileName);

		if (c_rFlag & QUEST_SEND_CLOCK_VALUE)
			rkQuest.SetQuestClockValue(QuestInfo.index, iClockValue);
		if (c_rFlag & QUEST_SEND_COUNTER_VALUE)
			rkQuest.SetQuestCounterValue(QuestInfo.index, iCounterValue);
	}
	else if (QUEST_PACKET_TYPE_BEGIN == byQuestPacketType)
	{
		CPythonQuest::SQuestInstance QuestInstance;
		QuestInstance.dwIndex = QuestInfo.index;
		QuestInstance.strTitle = szTitle;
		QuestInstance.strClockName = szClockName;
		QuestInstance.iClockValue = iClockValue;
		QuestInstance.strCounterName = szCounterName;
		QuestInstance.iCounterValue = iCounterValue;
		QuestInstance.strIconFileName = szIconFileName;
		CPythonQuest::Instance().RegisterQuestInstance(QuestInstance);
	}
	// Process Start End

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshQuest", Py_BuildValue("()"));
	return true;
}

bool CPythonNetworkStream::RecvQuestConfirmPacket()
{
	TPacketGCQuestConfirm kQuestConfirmPacket;
	if (!Recv(sizeof(kQuestConfirmPacket), &kQuestConfirmPacket))
	{
		Tracen("RecvQuestConfirmPacket Error");
		return false;
	}

	PyObject * poArg = Py_BuildValue("(sii)", kQuestConfirmPacket.msg, kQuestConfirmPacket.timeout, kQuestConfirmPacket.requestPID);
 	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OnQuestConfirm", poArg);
	return true;
}

bool CPythonNetworkStream::RecvRequestMakeGuild()
{
	TPacketGCBlank blank;
	if (!Recv(sizeof(blank), &blank))
	{
		Tracen("RecvRequestMakeGuild Packet Error");
		return false;
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AskGuildName", Py_BuildValue("()"));

	return true;
}

void CPythonNetworkStream::ToggleGameDebugInfo()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ToggleDebugInfo", Py_BuildValue("()"));
}

#ifdef ENABLE_SOUL_ROULETTE_SYSTEM
bool CPythonNetworkStream::RecvSoulRoulette()
{
	TPacketGCSoulRoulette Packet;
	if (!Recv(sizeof(Packet), &Packet)) {
		Tracen("RecvSoulRoulette Error");
		return false;
	}

	enum { OPEN, CLOSE, TURN };
	switch (Packet.option) {
	case OPEN:
		for (int i = 0; i < ROULETTE_ITEM_MAX; i++)
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ROULETTE_ICON", Py_BuildValue("(iii)", i, Packet.ItemInfo[i].vnum, Packet.ItemInfo[i].count));
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ROULETTE_OPEN", Py_BuildValue("(ii)", Packet.yang, Packet.soul));
		break;
	case CLOSE:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ROULETTE_CLOSE", Py_BuildValue("()"));
		break;
	case TURN:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ROULETTE_TURN", Py_BuildValue("(ii)", Packet.ItemInfo[0].vnum, Packet.ItemInfo[0].count));
		break;
	}

	return true;
}

bool CPythonNetworkStream::SoulRoulette(const BYTE option)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGSoulRoulette p;
	p.header = HEADER_CG_SOUL_ROULETTE;
	p.option = option;

	if (!Send(sizeof(p), &p)) {
		Tracen("Error SoulRoulette");
		return false;
	}

	return SendSequence();
}
#endif

bool CPythonNetworkStream::SendExchangeStartPacket(DWORD vid)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange	packet;

	packet.header		= HEADER_CG_EXCHANGE;
	packet.subheader	= EXCHANGE_SUBHEADER_CG_START;
	packet.arg1			= vid;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_start_packet Error\n");
		return false;
	}

	Tracef("send_trade_start_packet   vid %d \n", vid);
	return SendSequence();
}

bool CPythonNetworkStream::SendExchangeElkAddPacket(long long elk)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange	packet;

	packet.header		= HEADER_CG_EXCHANGE;
	packet.subheader	= EXCHANGE_SUBHEADER_CG_ELK_ADD;
	packet.arg1			= elk;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_elk_add_packet Error\n");
		return false;
	}

	return SendSequence();
}

#ifdef ENABLE_CHEQUE_SYSTEM
bool CPythonNetworkStream::SendExchangeWonAddPacket(DWORD won)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange	packet;

	packet.header		= HEADER_CG_EXCHANGE;
	packet.subheader	= EXCHANGE_SUBHEADER_CG_WON_ADD;
	packet.arg1			= won;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_elk_add_packet Error\n");
		return false;
	}

	return SendSequence();
}
#endif

#ifdef ENABLE_ITEM_CHECKINOUT_UPDATE
bool CPythonNetworkStream::SendExchangeItemAddPacket(TItemPos ItemPos, BYTE byDisplayPos, bool SelectPosAuto)
#else
bool CPythonNetworkStream::SendExchangeItemAddPacket(TItemPos ItemPos, BYTE byDisplayPos)
#endif
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange	packet;

	packet.header		= HEADER_CG_EXCHANGE;
	packet.subheader	= EXCHANGE_SUBHEADER_CG_ITEM_ADD;
	packet.Pos			= ItemPos;
	packet.arg2			= byDisplayPos;
#ifdef ENABLE_ITEM_CHECKINOUT_UPDATE
	packet.SelectPosAuto = SelectPosAuto;
#endif

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_item_add_packet Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendExchangeItemDelPacket(BYTE pos)
{
	assert(!"Can't be called function - CPythonNetworkStream::SendExchangeItemDelPacket");
	return true;

	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange	packet;

	packet.header		= HEADER_CG_EXCHANGE;
	packet.subheader	= EXCHANGE_SUBHEADER_CG_ITEM_DEL;
	packet.arg1			= pos;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_item_del_packet Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendExchangeAcceptPacket()
{
	if (!__CanActMainInstance())
		return true;
	
	TPacketCGExchange	packet;

	packet.header		= HEADER_CG_EXCHANGE;
	packet.subheader	= EXCHANGE_SUBHEADER_CG_ACCEPT;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_accept_packet Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendExchangeExitPacket()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange	packet;

	packet.header		= HEADER_CG_EXCHANGE;
	packet.subheader	= EXCHANGE_SUBHEADER_CG_CANCEL;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_exit_packet Error\n");
		return false;
	}

	return SendSequence();
}

#ifdef ENABLE_FLOWER_EVENT
bool CPythonNetworkStream::SendFlowerEventExchange(uint8_t id)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGFlowerEventSend packet{};
	packet.header = HEADER_CG_FLOWER_EVENT_EXCHANGE;
	packet.id = id;

	if (!Send(sizeof(TPacketCGFlowerEventSend), &packet))
	{
		Tracef("SendFlowerEventExchange Error\n");
		return false;
	}

	return SendSequence();
}
#endif

#ifdef ENABLE_MAILBOX_SYSTEM
bool CPythonNetworkStream::SendMailBoxClose()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGMailBoxSend packet;
	packet.header = HEADER_CG_MAILBOX_SEND;
	packet.subheader = MAILBOX_SUB_HEADER_CLOSE;

	if (!Send(sizeof(TPacketCGMailBoxSend), &packet))
	{
		Tracef("SendMailBoxClose Error\n");
		return false;
	}

	return SendSequence();
}

#ifdef ENABLE_CHEQUE_SYSTEM
bool CPythonNetworkStream::SendMailBoxMail(const char* nombre_pj, const char* asunto, const char* descrip, int slot_item, int yang, int won)
#else
bool CPythonNetworkStream::SendMailBoxMail(const char* nombre_pj, const char* asunto, const char* descrip, int slot_item, int yang)
#endif
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGMailBoxSend packet;
	packet.header = HEADER_CG_MAILBOX_SEND;
	packet.subheader = MAILBOX_SUB_HEADER_SEND_MAIL;

	strncpy(packet.nombre_pj, nombre_pj, 12);
	strncpy(packet.asunto, asunto, 24);
	strncpy(packet.descrip, descrip, 100);
	packet.slot_item = slot_item;
	packet.yang = yang;
#ifdef ENABLE_CHEQUE_SYSTEM
	packet.won = won;
#endif

	if (!Send(sizeof(TPacketCGMailBoxSend), &packet))
	{
		Tracef("SendMailBoxMail Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendMailBoxNameCheck(const char* nombre_pj)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGMailBoxSend packet;
	packet.header = HEADER_CG_MAILBOX_SEND;
	packet.subheader = MAILBOX_SUB_HEADER_CHECK_NAME;
	strncpy(packet.nombre_pj, nombre_pj, 12);

	if (!Send(sizeof(TPacketCGMailBoxSend), &packet))
	{
		Tracef("SendMailBoxNameCheck Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendMailBoxAcceptMail(int index_mail)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGMailBoxSend packet;
	packet.header = HEADER_CG_MAILBOX_SEND;
	packet.subheader = MAILBOX_SUB_HEADER_ACCEPT_MAIL;
	packet.id_mail = index_mail;

	if (!Send(sizeof(TPacketCGMailBoxSend), &packet))
	{
		Tracef("SendMailBoxAcceptMail Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendMailBoxDeleteMail(int index_mail)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGMailBoxSend packet;
	packet.header = HEADER_CG_MAILBOX_SEND;
	packet.subheader = MAILBOX_SUB_HEADER_DELETE_MAIL;
	packet.id_mail = index_mail;

	if (!Send(sizeof(TPacketCGMailBoxSend), &packet))
	{
		Tracef("SendMailBoxDeleteMail Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendMailBoxAcceptAllMail()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGMailBoxSend packet;
	packet.header = HEADER_CG_MAILBOX_SEND;
	packet.subheader = MAILBOX_SUB_HEADER_ACCEPT_ALL_MAIL;

	if (!Send(sizeof(TPacketCGMailBoxSend), &packet))
	{
		Tracef("SendMailBoxAcceptAllMail Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendMailBoxDeleteAllMail()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGMailBoxSend packet;
	packet.header = HEADER_CG_MAILBOX_SEND;
	packet.subheader = MAILBOX_SUB_HEADER_DELETE_ALL_MAIL;

	if (!Send(sizeof(TPacketCGMailBoxSend), &packet))
	{
		Tracef("SendMailBoxDeleteAllMail Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendMailBoxViewMail(int index_mail)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGMailBoxSend packet;
	packet.header = HEADER_CG_MAILBOX_SEND;
	packet.subheader = MAILBOX_SUB_HEADER_VIEW_MAIL;
	packet.id_mail = index_mail;

	if (!Send(sizeof(TPacketCGMailBoxSend), &packet))
	{
		Tracef("SendMailBoxViewMail Error\n");
		return false;
	}

	return SendSequence();
}
#endif

// PointReset ê°œì„ì‹œ
bool CPythonNetworkStream::SendPointResetPacket()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartPointReset", Py_BuildValue("()"));
	return true;
}

bool CPythonNetworkStream::__IsPlayerAttacking()
{
	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	CInstanceBase* pkInstMain=rkChrMgr.GetMainInstancePtr();
	if (!pkInstMain)
		return false;

	if (!pkInstMain->IsAttacking())
		return false;

	return true;
}

bool CPythonNetworkStream::RecvScriptPacket()
{
	TPacketGCScript ScriptPacket;

	if (!Recv(sizeof(TPacketGCScript), &ScriptPacket))
	{
		TraceError("RecvScriptPacket_RecvError");
		return false;
	}

	if (ScriptPacket.size < sizeof(TPacketGCScript))
	{
		TraceError("RecvScriptPacket_SizeError");
		return false;
	}

	ScriptPacket.size -= sizeof(TPacketGCScript);
	
	static string str;
	str = "";
	str.resize(ScriptPacket.size+1);

	if (!Recv(ScriptPacket.size, &str[0]))
		return false;

	str[str.size()-1] = '\0';

	if (str.compare(0, 13, "[DESTROY_ALL]") == 0)
	{
		CPythonNetworkStream::Instance().HideQuestWindows();
		return true;
	}

	int iIndex = CPythonEventManager::Instance().RegisterEventSetFromString(str);

	if (-1 != iIndex)
	{
		CPythonEventManager::Instance().SetVisibleLineCount(iIndex, 30);
		CPythonNetworkStream::Instance().OnScriptEventStart(ScriptPacket.skin,iIndex);
	}

	return true;
}

bool CPythonNetworkStream::SendScriptAnswerPacket(int iAnswer)
{
	TPacketCGScriptAnswer ScriptAnswer;

	ScriptAnswer.header = HEADER_CG_SCRIPT_ANSWER;
	ScriptAnswer.answer = (BYTE) iAnswer;
	if (!Send(sizeof(TPacketCGScriptAnswer), &ScriptAnswer))
	{
		Tracen("Send Script Answer Packet Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendScriptButtonPacket(unsigned int iIndex)
{
	TPacketCGScriptButton ScriptButton;

	ScriptButton.header = HEADER_CG_SCRIPT_BUTTON;
	ScriptButton.idx = iIndex;
	if (!Send(sizeof(TPacketCGScriptButton), &ScriptButton))
	{
		Tracen("Send Script Button Packet Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendAnswerMakeGuildPacket(const char * c_szName)
{
	TPacketCGAnswerMakeGuild Packet;

	Packet.header = HEADER_CG_ANSWER_MAKE_GUILD;
	strncpy(Packet.guild_name, c_szName, GUILD_NAME_MAX_LEN);
	Packet.guild_name[GUILD_NAME_MAX_LEN] = '\0';

	if (!Send(sizeof(Packet), &Packet))
	{
		Tracen("SendAnswerMakeGuild Packet Error");
		return false;
	}

// 	Tracef(" SendAnswerMakeGuildPacket : %s", c_szName);
	return SendSequence();
}

bool CPythonNetworkStream::SendQuestInputStringPacket(const char * c_szString)
{
	TPacketCGQuestInputString Packet;
	Packet.bHeader = HEADER_CG_QUEST_INPUT_STRING;
	strncpy(Packet.szString, c_szString, QUEST_INPUT_STRING_MAX_NUM);

	if (!Send(sizeof(Packet), &Packet))
	{
		Tracen("SendQuestInputStringPacket Error");
		return false;
	}

	return SendSequence();
}

#ifdef ENABLE_OX_RENEWAL
bool CPythonNetworkStream::SendQuestInputStringLongPacket(const char * c_szString)
{
	TPacketCGQuestInputLongString Packet;
	Packet.bHeader = HEADER_CG_QUEST_INPUT_LONG_STRING;
	strncpy(Packet.szString, c_szString, QUEST_INPUT_LONG_STRING_MAX_NUM);

	if (!Send(sizeof(Packet), &Packet))
	{
		Tracen("SendQuestInputLongStringPacket Error");
		return false;
	}

	return SendSequence();
}
#endif

bool CPythonNetworkStream::SendQuestConfirmPacket(BYTE byAnswer, DWORD dwPID)
{
	TPacketCGQuestConfirm kPacket;
	kPacket.header = HEADER_CG_QUEST_CONFIRM;
	kPacket.answer = byAnswer;
	kPacket.requestPID = dwPID;

	if (!Send(sizeof(kPacket), &kPacket))
	{
		Tracen("SendQuestConfirmPacket Error");
		return false;
	}

	Tracenf(" SendQuestConfirmPacket : %d, %d", byAnswer, dwPID);
	return SendSequence();
}

bool CPythonNetworkStream::RecvSkillCoolTimeEnd()
{
	TPacketGCSkillCoolTimeEnd kPacketSkillCoolTimeEnd;
	if (!Recv(sizeof(kPacketSkillCoolTimeEnd), &kPacketSkillCoolTimeEnd))
	{
		Tracen("CPythonNetworkStream::RecvSkillCoolTimeEnd - RecvError");
		return false;
	}

	CPythonPlayer::Instance().EndSkillCoolTime(kPacketSkillCoolTimeEnd.bSkill);

	return true;
}

bool CPythonNetworkStream::RecvSkillLevel()
{
	assert(!"CPythonNetworkStream::RecvSkillLevel - ì‚¬ìš©í•˜ì§€ ì•ŠëŠ” í•¨ìˆ˜");
	TPacketGCSkillLevel packet;
	if (!Recv(sizeof(TPacketGCSkillLevel), &packet))
	{
		Tracen("CPythonNetworkStream::RecvSkillLevel - RecvError");
		return false;
	}

	DWORD dwSlotIndex;

	CPythonPlayer& rkPlayer=CPythonPlayer::Instance();
	for (int i = 0; i < SKILL_MAX_NUM; ++i)
	{
		if (rkPlayer.GetSkillSlotIndex(i, &dwSlotIndex))
			rkPlayer.SetSkillLevel(dwSlotIndex, packet.abSkillLevels[i]);
	}

	__RefreshSkillWindow();
	__RefreshStatus();
	Tracef(" >> RecvSkillLevel\n");
	return true;
}

bool CPythonNetworkStream::RecvSkillLevelNew()
{
	TPacketGCSkillLevelNew packet;
	if (!Recv(sizeof(TPacketGCSkillLevelNew), &packet))
	{
		Tracen("CPythonNetworkStream::RecvSkillLevelNew - RecvError");
		return false;
	}

	CPythonPlayer& rkPlayer=CPythonPlayer::Instance();
#ifdef ENABLE_7AND8TH_SKILLS
	BYTE SKILL_ANTI_PALBANG = 221;
	BYTE SKILL_ANTI_BYEURAK = 228;
	BYTE SKILL_ANTI_SALPOONG = 229;
	BYTE SKILL_HELP_PALBANG = 236;
	BYTE SKILL_HELP_BYEURAK = 243;
	BYTE SKILL_HELP_SALPOONG = 244;

	rkPlayer.SetSkill(7, 0);
	rkPlayer.SetSkill(8, 0);
#endif
	for (int i = 0; i < SKILL_MAX_NUM; ++i)
	{
		TPlayerSkill & rPlayerSkill = packet.skills[i];
#ifdef ENABLE_7AND8TH_SKILLS
		if (i >= SKILL_ANTI_PALBANG && i <= SKILL_ANTI_SALPOONG)
		{
			if (rPlayerSkill.bLevel >= 1)
			{
				rkPlayer.SetSkill(7, i);
				rkPlayer.SetSkillLevel_(i, rPlayerSkill.bMasterType, rPlayerSkill.bLevel);
			}
		}
		else if (i >= SKILL_HELP_PALBANG && i <= SKILL_HELP_SALPOONG)
		{
			if (rPlayerSkill.bLevel >= 1)
			{
				rkPlayer.SetSkill(8, i);
				rkPlayer.SetSkillLevel_(i, rPlayerSkill.bMasterType, rPlayerSkill.bLevel);
			}
		}
		else
			rkPlayer.SetSkillLevel_(i, rPlayerSkill.bMasterType, rPlayerSkill.bLevel);
#else
		rkPlayer.SetSkillLevel_(i, rPlayerSkill.bMasterType, rPlayerSkill.bLevel);
#endif
	}

	__RefreshSkillWindow();
	__RefreshStatus();
	return true;
}


bool CPythonNetworkStream::RecvDamageInfoPacket()
{
	TPacketGCDamageInfo DamageInfoPacket;

	if (!Recv(sizeof(TPacketGCDamageInfo), &DamageInfoPacket))
	{
		Tracen("Recv Target Packet Error");
		return false;
	}
	
	CInstanceBase * pInstTarget = CPythonCharacterManager::Instance().GetInstancePtr(DamageInfoPacket.dwVID);
	bool bSelf = (pInstTarget == CPythonCharacterManager::Instance().GetMainInstancePtr());
	bool bTarget = (pInstTarget==m_pInstTarget);
	if (pInstTarget)
	{
		if(DamageInfoPacket.damage >= 0)
			pInstTarget->AddDamageEffect(DamageInfoPacket.damage,DamageInfoPacket.flag,bSelf,bTarget);
		//else
			//TraceError("Damage is equal or below 0.");
	}

	return true;
}

bool CPythonNetworkStream::RecvTargetPacket()
{
	TPacketGCTarget TargetPacket;

	if (!Recv(sizeof(TPacketGCTarget), &TargetPacket))
	{
		Tracen("Recv Target Packet Error");
		return false;
	}

#if defined(ENABLE_DEFENSE_WAVE)
	if (TargetPacket.bAlliance)
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetHPAllianceTargetBoard",
			Py_BuildValue("(iLL)", TargetPacket.dwVID, TargetPacket.iMinHP, TargetPacket.iMaxHP));
		return true;
	}
#endif

	CInstanceBase* pInstPlayer = CPythonCharacterManager::Instance().GetMainInstancePtr();
	CInstanceBase* pInstTarget = CPythonCharacterManager::Instance().GetInstancePtr(TargetPacket.dwVID);

	bool bHideBoard = false;
#ifdef ENABLE_BATTLE_ROYALE
	if (CPythonBackground::Instance().IsBattleRoyaleMap() && pInstTarget && pInstTarget->IsPC())
		bHideBoard = true;
#endif

	if (pInstPlayer && pInstTarget)
	{
		if (!pInstTarget->IsDead())
		{
			if (!bHideBoard)
			{
#ifdef ENABLE_VIEW_TARGET_PLAYER_HP
				if (pInstTarget->IsBuilding())
#else
				if (pInstTarget->IsPC() || pInstTarget->IsBuilding())
#endif
				{
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoardIfDifferent", Py_BuildValue("(i)", TargetPacket.dwVID));
				}
				else if (pInstPlayer->CanViewTargetHP(*pInstTarget))
				{
#ifdef ENABLE_VIEW_TARGET_DECIMAL_HP
#ifdef ENABLE_MOB_HP_EXTENDED
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
						"SetHPTargetBoard",
						Py_BuildValue("(iiLL)", TargetPacket.dwVID,
												TargetPacket.bHPPercent,
												TargetPacket.iMinHP,
												TargetPacket.iMaxHP));
#else
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
						"SetHPTargetBoard",
						Py_BuildValue("(iiii)", TargetPacket.dwVID,
												TargetPacket.bHPPercent,
												TargetPacket.iMinHP,
												TargetPacket.iMaxHP));
#endif
#else
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
						"SetHPTargetBoard",
						Py_BuildValue("(ii)", TargetPacket.dwVID,
												TargetPacket.bHPPercent));
#endif

#ifdef ENABLE_ELEMENT_TARGET
					PyObject* poDict = PyDict_New();
					for (uint8_t uiElementIndex = 0; uiElementIndex < CPythonNonPlayer::MOB_ELEMENT_MAX_NUM; ++uiElementIndex)
						PyDict_SetItem(poDict, Py_BuildValue("i", uiElementIndex), Py_BuildValue("i", TargetPacket.bElement[uiElementIndex]));
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ShowTargetElementEnchant", Py_BuildValue("(O)", poDict));
#endif
				}
				else
				{
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard", Py_BuildValue("()"));
				}
			}

			m_pInstTarget = pInstTarget;
		}
	}
	else
	{
		if (!bHideBoard)
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard", Py_BuildValue("()"));
	}

	return true;
}

#ifdef ENABLE_TARGET_INFORMATION_SYSTEM
bool CPythonNetworkStream::RecvTargetInfoPacket()
{
	TPacketGCTargetInfo pInfoTargetPacket;

	if (!Recv(sizeof(TPacketGCTargetInfo), &pInfoTargetPacket))
	{
		Tracen("Recv Info Target Packet Error");
		return false;
	}

	CInstanceBase * pInstPlayer = CPythonCharacterManager::Instance().GetMainInstancePtr();
	CInstanceBase * pInstTarget = CPythonCharacterManager::Instance().GetInstancePtr(pInfoTargetPacket.dwVID);
	if (pInstPlayer && pInstTarget)
	{
		if (!pInstTarget->IsDead())
		{
			if (pInstTarget->IsEnemy() || pInstTarget->IsStone())
			{
				// Item bilgisini ekle (race, vnum, count, exp, sungmaexp)
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AddTargetMonsterDropInfo", 
#ifdef ENABLE_CONQUEROR_LEVEL
				Py_BuildValue("(iiiii)", pInfoTargetPacket.race, pInfoTargetPacket.dwVnum, pInfoTargetPacket.count, pInfoTargetPacket.dwExp, pInfoTargetPacket.dwSungMaExp));
#else
				Py_BuildValue("(iiii)", pInfoTargetPacket.race, pInfoTargetPacket.dwVnum, pInfoTargetPacket.count, pInfoTargetPacket.dwExp));
#endif
				// Listeyi yenile
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_RefreshTargetMonsterDropInfo", Py_BuildValue("(i)", pInfoTargetPacket.race));
			}
			else
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard", Py_BuildValue("()"));

			// m_pInstTarget = pInstTarget;
		}
	}
	else
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard", Py_BuildValue("()"));
	}

	return true;
}
#endif

bool CPythonNetworkStream::RecvMountPacket()
{
	TPacketGCMount MountPacket;

	if (!Recv(sizeof(TPacketGCMount), &MountPacket))
	{
		Tracen("Recv Mount Packet Error");
		return false;
	}

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(MountPacket.vid);

	if (pInstance)
	{
		// Mount
		if (0 != MountPacket.mount_vid)
		{
//			pInstance->Ride(MountPacket.pos, MountPacket.mount_vid);
		}
		// Unmount
		else
		{
//			pInstance->Unride(MountPacket.pos, MountPacket.x, MountPacket.y);
		}
	}

	if (CPythonPlayer::Instance().IsMainCharacterIndex(MountPacket.vid))
	{
//		CPythonPlayer::Instance().SetRidingVehicleIndex(MountPacket.mount_vid);
	}

	return true;
}

bool CPythonNetworkStream::RecvChangeSpeedPacket()
{
	TPacketGCChangeSpeed SpeedPacket;

	if (!Recv(sizeof(TPacketGCChangeSpeed), &SpeedPacket))
	{
		Tracen("Recv Speed Packet Error");
		return false;
	}

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(SpeedPacket.vid);

	if (!pInstance)
		return true;

//	pInstance->SetWalkSpeed(SpeedPacket.walking_speed);
//	pInstance->SetRunSpeed(SpeedPacket.running_speed);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Recv

bool CPythonNetworkStream::SendAttackPacket(UINT uMotAttack, DWORD dwVIDVictim)
{
	if (!__CanActMainInstance())
		return true;

#ifdef ATTACK_TIME_LOG
	static DWORD prevTime = timeGetTime();
	DWORD curTime = timeGetTime();
	TraceError("TIME: %.4f(%.4f) ATTACK_PACKET: %d TARGET: %d", curTime/1000.0f, (curTime-prevTime)/1000.0f, uMotAttack, dwVIDVictim);
	prevTime = curTime;
#endif

	TPacketCGAttack kPacketAtk;

	kPacketAtk.header = HEADER_CG_ATTACK;
	kPacketAtk.bType = uMotAttack;
	kPacketAtk.dwVictimVID = dwVIDVictim;

#ifdef ENABLE_AUTO_SYSTEM
	CPythonPlayer::Instance().SetLastAttackTime(CTimer::Instance().GetCurrentSecond());
#endif

	if (!SendSpecial(sizeof(kPacketAtk), &kPacketAtk))
	{
		Tracen("Send Battle Attack Packet Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendSpecial(int nLen, void * pvBuf)
{
	BYTE bHeader = *(BYTE *) pvBuf;

	switch (bHeader)
	{
		case HEADER_CG_ATTACK:
			{
				TPacketCGAttack * pkPacketAtk = (TPacketCGAttack *) pvBuf;
				pkPacketAtk->bCRCMagicCubeProcPiece = GetProcessCRCMagicCubePiece();
				pkPacketAtk->bCRCMagicCubeFilePiece = GetProcessCRCMagicCubePiece();
				return Send(nLen, pvBuf);
			}
			break;
	}

	return Send(nLen, pvBuf);
}

bool CPythonNetworkStream::RecvAddFlyTargetingPacket()
{
	TPacketGCFlyTargeting kPacket;
	if (!Recv(sizeof(kPacket), &kPacket))
		return false;

	__GlobalPositionToLocalPosition(kPacket.lX, kPacket.lY);

	Tracef("VID [%d]ê°€ íƒ€ê²Ÿì„ ì¶”ê°€ ì„¤ì •\n",kPacket.dwShooterVID);

	CPythonCharacterManager & rpcm = CPythonCharacterManager::Instance();

	CInstanceBase * pShooter = rpcm.GetInstancePtr(kPacket.dwShooterVID);

	if (!pShooter)
	{
#ifndef _DEBUG		
		TraceError("CPythonNetworkStream::RecvFlyTargetingPacket() - dwShooterVID[%d] NOT EXIST", kPacket.dwShooterVID);
#endif
		return true;
	}

	CInstanceBase * pTarget = rpcm.GetInstancePtr(kPacket.dwTargetVID);

	if (kPacket.dwTargetVID && pTarget)
	{
		pShooter->GetGraphicThingInstancePtr()->AddFlyTarget(pTarget->GetGraphicThingInstancePtr());
	}
	else
	{
		float h = CPythonBackground::Instance().GetHeight(kPacket.lX,kPacket.lY) + 60.0f; // TEMPORARY HEIGHT
		pShooter->GetGraphicThingInstancePtr()->AddFlyTarget(D3DXVECTOR3(kPacket.lX,kPacket.lY,h));
		//pShooter->GetGraphicThingInstancePtr()->SetFlyTarget(kPacket.kPPosTarget.x,kPacket.kPPosTarget.y,);
	}

	return true;
}

bool CPythonNetworkStream::RecvFlyTargetingPacket()
{
	TPacketGCFlyTargeting kPacket;
	if (!Recv(sizeof(kPacket), &kPacket))
		return false;

	__GlobalPositionToLocalPosition(kPacket.lX, kPacket.lY);

	//Tracef("CPythonNetworkStream::RecvFlyTargetingPacket - VID [%d]\n",kPacket.dwShooterVID);

	CPythonCharacterManager & rpcm = CPythonCharacterManager::Instance();

	CInstanceBase * pShooter = rpcm.GetInstancePtr(kPacket.dwShooterVID);

	if (!pShooter)
	{
#ifdef _DEBUG
		TraceError("CPythonNetworkStream::RecvFlyTargetingPacket() - dwShooterVID[%d] NOT EXIST", kPacket.dwShooterVID);
#endif
		return true;
	}

	CInstanceBase * pTarget = rpcm.GetInstancePtr(kPacket.dwTargetVID);

	if (kPacket.dwTargetVID && pTarget)
	{
		pShooter->GetGraphicThingInstancePtr()->SetFlyTarget(pTarget->GetGraphicThingInstancePtr());
	}
	else
	{
		float h = CPythonBackground::Instance().GetHeight(kPacket.lX, kPacket.lY) + 60.0f; // TEMPORARY HEIGHT
		pShooter->GetGraphicThingInstancePtr()->SetFlyTarget(D3DXVECTOR3(kPacket.lX,kPacket.lY,h));
		//pShooter->GetGraphicThingInstancePtr()->SetFlyTarget(kPacket.kPPosTarget.x,kPacket.kPPosTarget.y,);
	}

	return true;
}

bool CPythonNetworkStream::SendShootPacket(UINT uSkill)
{
	TPacketCGShoot kPacketShoot;
	kPacketShoot.bHeader=HEADER_CG_SHOOT;
	kPacketShoot.bType=uSkill;

	if (!Send(sizeof(kPacketShoot), &kPacketShoot))
	{
		Tracen("SendShootPacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendAddFlyTargetingPacket(DWORD dwTargetVID, const TPixelPosition & kPPosTarget)
{
	TPacketCGFlyTargeting packet;

	//CPythonCharacterManager & rpcm = CPythonCharacterManager::Instance();

	packet.bHeader	= HEADER_CG_ADD_FLY_TARGETING;
	packet.dwTargetVID = dwTargetVID;
	packet.lX = kPPosTarget.x;
	packet.lY = kPPosTarget.y;

	__LocalPositionToGlobalPosition(packet.lX, packet.lY);
	
	if (!Send(sizeof(packet), &packet))
	{
		Tracen("Send FlyTargeting Packet Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendFlyTargetingPacket(DWORD dwTargetVID, const TPixelPosition & kPPosTarget)
{
	TPacketCGFlyTargeting packet;

	packet.bHeader	= HEADER_CG_FLY_TARGETING;
	packet.dwTargetVID = dwTargetVID;
	packet.lX = kPPosTarget.x;
	packet.lY = kPPosTarget.y;

	__LocalPositionToGlobalPosition(packet.lX, packet.lY);

	if (!Send(sizeof(packet), &packet))
	{
		Tracen("Send FlyTargeting Packet Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvCreateFlyPacket()
{
	TPacketGCCreateFly kPacket;
	if (!Recv(sizeof(TPacketGCCreateFly), &kPacket))
		return false;

	CFlyingManager& rkFlyMgr = CFlyingManager::Instance();
	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();

	CInstanceBase * pkStartInst = rkChrMgr.GetInstancePtr(kPacket.dwStartVID);
	CInstanceBase * pkEndInst = rkChrMgr.GetInstancePtr(kPacket.dwEndVID);
	if (!pkStartInst || !pkEndInst)
		return true;

	rkFlyMgr.CreateIndexedFly(kPacket.bType, pkStartInst->GetGraphicThingInstancePtr(), pkEndInst->GetGraphicThingInstancePtr());

	return true;
}

bool CPythonNetworkStream::SendTargetPacket(DWORD dwVID)
{
	TPacketCGTarget packet;
	packet.header = HEADER_CG_TARGET;
	packet.dwVID = dwVID;

	if (!Send(sizeof(packet), &packet))
	{
		Tracen("Send Target Packet Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendSyncPositionElementPacket(DWORD dwVictimVID, DWORD dwVictimX, DWORD dwVictimY)
{
	TPacketCGSyncPositionElement kSyncPos;
	kSyncPos.dwVID=dwVictimVID;
	kSyncPos.lX=dwVictimX;
	kSyncPos.lY=dwVictimY;

	__LocalPositionToGlobalPosition(kSyncPos.lX, kSyncPos.lY);

	if (!Send(sizeof(kSyncPos), &kSyncPos))
	{
		Tracen("CPythonNetworkStream::SendSyncPositionElementPacket - ERROR");
		return false;
	}

	return true;
}

#ifdef ENABLE_MESSENGER_RENEWAL
void CPythonNetworkStream::RequestMessengerRefresh()
{
	__RefreshMessengerWindow();
}

bool CPythonNetworkStream::SendMessengerSetConnectionStatePacket(BYTE bConnectionState)
{
	if (bConnectionState > MESSENGER_CONNECTION_STATE_DISCONNECT)
		return false;

	TPacketCGMessenger Packet;
	Packet.header = HEADER_CG_MESSENGER;
	Packet.subheader = MESSENGER_SUBHEADER_CG_SET_CONNECTION_STATE;
	if (!Send(sizeof(Packet), &Packet))
		return false;

	TPacketCGMessengerSetConnectionState StatePacket;
	StatePacket.bConnectionState = bConnectionState;
	if (!Send(sizeof(StatePacket), &StatePacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendMessengerSetStatusMessagePacket(const char* c_szMessage)
{
	if (!c_szMessage || !*c_szMessage)
		return false;

	size_t len = strlen(c_szMessage);
	if (len == 0 || len > MESSENGER_STATUS_MESSAGE_MAX_LEN)
		return false;

	TPacketCGMessenger Packet;
	Packet.header = HEADER_CG_MESSENGER;
	Packet.subheader = MESSENGER_SUBHEADER_CG_SET_STATUS_MESSAGE;
	if (!Send(sizeof(Packet), &Packet))
		return false;

	TPacketCGMessengerSetStatusMessage StatusPacket;
	memset(&StatusPacket, 0, sizeof(StatusPacket));
	strncpy(StatusPacket.szStatusMessage, c_szMessage, MESSENGER_STATUS_MESSAGE_MAX_LEN);
	StatusPacket.szStatusMessage[MESSENGER_STATUS_MESSAGE_MAX_LEN] = '\0';
	if (!Send(sizeof(StatusPacket), &StatusPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendMessengerDeleteStatusMessagePacket()
{
	TPacketCGMessenger Packet;
	Packet.header = HEADER_CG_MESSENGER;
	Packet.subheader = MESSENGER_SUBHEADER_CG_DELETE_STATUS_MESSAGE;
	if (!Send(sizeof(Packet), &Packet))
		return false;

	return SendSequence();
}
#endif

bool CPythonNetworkStream::RecvMessenger()
{
	TPacketGCMessenger p;
	if (!Recv(sizeof(p), &p))
		return false;

	int iSize = p.size - sizeof(p);
	char char_name[24+1];

	switch (p.subheader)
	{
		case MESSENGER_SUBHEADER_GC_LIST:
		{
			TPacketGCMessengerListOnline on;
			while(iSize)
			{
				if (!Recv(sizeof(TPacketGCMessengerListOffline),&on))
					return false;

				if (!Recv(on.length, char_name))
					return false;

				char_name[on.length] = 0;

				if (on.connected & MESSENGER_CONNECTED_STATE_ONLINE)
					CPythonMessenger::Instance().OnFriendLogin(char_name);
				else
					CPythonMessenger::Instance().OnFriendLogout(char_name);

#ifdef ENABLE_MESSENGER_RENEWAL
				if (on.bLevel > 0)
				{
					CPythonCommunity::Instance().OnFriendRenewalDetails(char_name, on.bLevel, on.bIsConqueror, on.bChannel, on.lMapIndex);
				}
				CPythonCommunity::Instance().OnFriendConnectionState(char_name, on.bConnectionState);
				CPythonCommunity::Instance().OnFriendStatusMessage(char_name, on.szStatusMessage);
#endif

				if (on.connected & MESSENGER_CONNECTED_STATE_MOBILE)
					CPythonMessenger::Instance().SetMobile(char_name, TRUE);

				iSize -= sizeof(TPacketGCMessengerListOffline);
				iSize -= on.length;
			}
			break;
		}
#ifdef ENABLE_MESSENGER_BLOCK
		case MESSENGER_SUBHEADER_GC_BLOCK_LIST:
		{
			TPacketGCMessengerBlockListOnline onn;
			while(iSize)
			{
				if (!Recv(sizeof(TPacketGCMessengerBlockListOffline),&onn))
					return false;

				if (!Recv(onn.length, char_name))
					return false;

				char_name[onn.length] = 0;

				if (onn.connected & MESSENGER_CONNECTED_STATE_ONLINE)
					CPythonMessenger::Instance().OnBlockLogin(char_name);
				else
					CPythonMessenger::Instance().OnBlockLogout(char_name);

				iSize -= sizeof(TPacketGCMessengerBlockListOffline);
				iSize -= onn.length;
			}
			break;
		}
		
		case MESSENGER_SUBHEADER_GC_BLOCK_LOGIN:
		{
			TPacketGCMessengerLogin pp;
			if (!Recv(sizeof(pp),&pp))
				return false;
			if (!Recv(pp.length, char_name))
				return false;
			char_name[pp.length] = 0;
			CPythonMessenger::Instance().OnBlockLogin(char_name);
			__RefreshTargetBoardByName(char_name);
			break;
		}

		case MESSENGER_SUBHEADER_GC_BLOCK_LOGOUT:
		{
			TPacketGCMessengerLogout logout2;
			if (!Recv(sizeof(logout2),&logout2))
				return false;
			if (!Recv(logout2.length, char_name))
				return false;
			char_name[logout2.length] = 0;
			CPythonMessenger::Instance().OnBlockLogout(char_name);
			break;
		}
#endif
		case MESSENGER_SUBHEADER_GC_LOGIN:
		{
			TPacketGCMessengerLogin p;
			if (!Recv(sizeof(p),&p))
				return false;
			if (!Recv(p.length, char_name))
				return false;
			char_name[p.length] = 0;
			CPythonMessenger::Instance().OnFriendLogin(char_name);
			__RefreshTargetBoardByName(char_name);
			break;
		}

		case MESSENGER_SUBHEADER_GC_LOGOUT:
		{
			TPacketGCMessengerLogout logout;
			if (!Recv(sizeof(logout),&logout))
				return false;
			if (!Recv(logout.length, char_name))
				return false;
			char_name[logout.length] = 0;
			CPythonMessenger::Instance().OnFriendLogout(char_name);
			break;
		}

#ifdef ENABLE_MESSENGER_RENEWAL
		case MESSENGER_SUBHEADER_GC_MY_STATUS_MESSAGE:
		{
			TPacketGCMessengerMyStatusMessage statusPacket;
			if (!Recv(sizeof(statusPacket), &statusPacket))
				return false;

			CPythonCommunity::Instance().NotifyMyStatusMessage(statusPacket.szStatusMessage);
			break;
		}

		case MESSENGER_SUBHEADER_GC_STATUS_MESSAGE:
		{
			TPacketGCMessengerStatusMessage statusPacket;
			if (!Recv(sizeof(statusPacket), &statusPacket))
				return false;

			CPythonCommunity::Instance().OnFriendStatusMessage(statusPacket.szName, statusPacket.szStatusMessage);
			break;
		}

		case MESSENGER_SUBHEADER_GC_CONNECTION_STATE:
		{
			TPacketGCMessengerListOnline connPacket;
			if (!Recv(sizeof(connPacket), &connPacket))
				return false;

			if (!Recv(connPacket.length, char_name))
				return false;

			char_name[connPacket.length] = 0;

			if (connPacket.connected & MESSENGER_CONNECTED_STATE_ONLINE)
			{
				CPythonMessenger::Instance().OnFriendLogin(char_name);
				CPythonCommunity::Instance().OnFriendRenewalDetails(char_name, connPacket.bLevel, connPacket.bIsConqueror, connPacket.bChannel, connPacket.lMapIndex);
			}
			else
			{
				CPythonMessenger::Instance().OnFriendLogout(char_name);
			}

			CPythonCommunity::Instance().OnFriendConnectionState(char_name, connPacket.bConnectionState);
			CPythonCommunity::Instance().OnFriendStatusMessage(char_name, connPacket.szStatusMessage);
			break;
		}
#endif

		case MESSENGER_SUBHEADER_GC_MOBILE:
		{
			BYTE byState; // ??? ??? ????? ???
			BYTE byLength;
			if (!Recv(sizeof(byState), &byState))
				return false;
			if (!Recv(sizeof(byLength), &byLength))
				return false;
			if (!Recv(byLength, char_name))
				return false;
			char_name[byLength] = 0;
			CPythonMessenger::Instance().SetMobile(char_name, byState);
			break;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
// Party

bool CPythonNetworkStream::SendPartyInvitePacket(DWORD dwVID)
{
	TPacketCGPartyInvite kPartyInvitePacket;
	kPartyInvitePacket.header = HEADER_CG_PARTY_INVITE;
	kPartyInvitePacket.vid = dwVID;

	if (!Send(sizeof(kPartyInvitePacket), &kPartyInvitePacket))
	{
		Tracenf("CPythonNetworkStream::SendPartyInvitePacket [%ud] - PACKET SEND ERROR", dwVID);
		return false;
	}

	Tracef(" << SendPartyInvitePacket : %d\n", dwVID);
	return SendSequence();
}

bool CPythonNetworkStream::SendPartyInviteAnswerPacket(DWORD dwLeaderVID, BYTE byAnswer)
{
	TPacketCGPartyInviteAnswer kPartyInviteAnswerPacket;
	kPartyInviteAnswerPacket.header = HEADER_CG_PARTY_INVITE_ANSWER;
	kPartyInviteAnswerPacket.leader_pid = dwLeaderVID;
	kPartyInviteAnswerPacket.accept = byAnswer;

	if (!Send(sizeof(kPartyInviteAnswerPacket), &kPartyInviteAnswerPacket))
	{
		Tracenf("CPythonNetworkStream::SendPartyInviteAnswerPacket [%ud %ud] - PACKET SEND ERROR", dwLeaderVID, byAnswer);
		return false;
	}

	Tracef(" << SendPartyInviteAnswerPacket : %d, %d\n", dwLeaderVID, byAnswer);
	return SendSequence();
}

bool CPythonNetworkStream::SendPartyRemovePacket(DWORD dwPID)
{
	TPacketCGPartyRemove kPartyInviteRemove;
	kPartyInviteRemove.header = HEADER_CG_PARTY_REMOVE;
	kPartyInviteRemove.pid = dwPID;

	if (!Send(sizeof(kPartyInviteRemove), &kPartyInviteRemove))
	{
		Tracenf("CPythonNetworkStream::SendPartyRemovePacket [%ud] - PACKET SEND ERROR", dwPID);
		return false;
	}

	Tracef(" << SendPartyRemovePacket : %d\n", dwPID);
	return SendSequence();
}

bool CPythonNetworkStream::SendPartySetStatePacket(DWORD dwVID, BYTE byState, BYTE byFlag)
{
	TPacketCGPartySetState kPartySetState;
	kPartySetState.byHeader = HEADER_CG_PARTY_SET_STATE;
	kPartySetState.dwVID = dwVID;
	kPartySetState.byState = byState;
	kPartySetState.byFlag = byFlag;

	if (!Send(sizeof(kPartySetState), &kPartySetState))
	{
		Tracenf("CPythonNetworkStream::SendPartySetStatePacket(%ud, %ud) - PACKET SEND ERROR", dwVID, byState);
		return false;
	}

	Tracef(" << SendPartySetStatePacket : %d, %d, %d\n", dwVID, byState, byFlag);
	return SendSequence();
}

bool CPythonNetworkStream::SendPartyUseSkillPacket(BYTE bySkillIndex, DWORD dwVID)
{
	TPacketCGPartyUseSkill kPartyUseSkill;
	kPartyUseSkill.byHeader = HEADER_CG_PARTY_USE_SKILL;
	kPartyUseSkill.bySkillIndex = bySkillIndex;
	kPartyUseSkill.dwTargetVID = dwVID;

	if (!Send(sizeof(kPartyUseSkill), &kPartyUseSkill))
	{
		Tracenf("CPythonNetworkStream::SendPartyUseSkillPacket(%ud, %ud) - PACKET SEND ERROR", bySkillIndex, dwVID);
		return false;
	}

	Tracef(" << SendPartyUseSkillPacket : %d, %d\n", bySkillIndex, dwVID);
	return SendSequence();
}

bool CPythonNetworkStream::SendPartyParameterPacket(BYTE byDistributeMode)
{
	TPacketCGPartyParameter kPartyParameter;
	kPartyParameter.bHeader = HEADER_CG_PARTY_PARAMETER;
	kPartyParameter.bDistributeMode = byDistributeMode;

	if (!Send(sizeof(kPartyParameter), &kPartyParameter))
	{
		Tracenf("CPythonNetworkStream::SendPartyParameterPacket(%d) - PACKET SEND ERROR", byDistributeMode);
		return false;
	}

	Tracef(" << SendPartyParameterPacket : %d\n", byDistributeMode);
	return SendSequence();
}

bool CPythonNetworkStream::RecvPartyInvite()
{
	TPacketGCPartyInvite kPartyInvitePacket;
	if (!Recv(sizeof(kPartyInvitePacket), &kPartyInvitePacket))
		return false;

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(kPartyInvitePacket.leader_pid);
	if (!pInstance)
	{
		TraceError(" CPythonNetworkStream::RecvPartyInvite - Failed to find leader instance [%d]\n", kPartyInvitePacket.leader_pid);
		return true;
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RecvPartyInviteQuestion", Py_BuildValue("(is)", kPartyInvitePacket.leader_pid, pInstance->GetNameString()));
	Tracef(" >> RecvPartyInvite : %d, %s\n", kPartyInvitePacket.leader_pid, pInstance->GetNameString());

	return true;
}

bool CPythonNetworkStream::RecvPartyAdd()
{
	TPacketGCPartyAdd kPartyAddPacket;
	if (!Recv(sizeof(kPartyAddPacket), &kPartyAddPacket))
		return false;

	CPythonPlayer::Instance().AppendPartyMember(kPartyAddPacket.pid, kPartyAddPacket.name);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AddPartyMember", Py_BuildValue("(is)", kPartyAddPacket.pid, kPartyAddPacket.name));

	Tracef(" >> RecvPartyAdd : %d, %s\n", kPartyAddPacket.pid, kPartyAddPacket.name);

	return true;
}

bool CPythonNetworkStream::RecvPartyUpdate()
{
	TPacketGCPartyUpdate kPartyUpdatePacket;
	if (!Recv(sizeof(kPartyUpdatePacket), &kPartyUpdatePacket))
		return false;

	CPythonPlayer::TPartyMemberInfo * pPartyMemberInfo;
	if (!CPythonPlayer::Instance().GetPartyMemberPtr(kPartyUpdatePacket.pid, &pPartyMemberInfo))
		return true;

	BYTE byOldState = pPartyMemberInfo->byState;

	CPythonPlayer::Instance().UpdatePartyMemberInfo(kPartyUpdatePacket.pid, kPartyUpdatePacket.state, kPartyUpdatePacket.percent_hp);
	for (int i = 0; i < PARTY_AFFECT_SLOT_MAX_NUM; ++i)
	{
		CPythonPlayer::Instance().UpdatePartyMemberAffect(kPartyUpdatePacket.pid, i, kPartyUpdatePacket.affects[i]);
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "UpdatePartyMemberInfo", Py_BuildValue("(i)", kPartyUpdatePacket.pid));

	// ë§Œì•½ ë¦¬ë”ê°€ ë°”ë€Œì—ˆë‹¤ë©´, TargetBoard ì˜ ë²„íŠ¼ì„ ì—…ë°ì´íŠ¸ í•œë‹¤.
	DWORD dwVID;
	if (CPythonPlayer::Instance().PartyMemberPIDToVID(kPartyUpdatePacket.pid, &dwVID))
	if (byOldState != kPartyUpdatePacket.state)
	{
		__RefreshTargetBoardByVID(dwVID);
	}

// 	Tracef(" >> RecvPartyUpdate : %d, %d, %d\n", kPartyUpdatePacket.pid, kPartyUpdatePacket.state, kPartyUpdatePacket.percent_hp);

	return true;
}

bool CPythonNetworkStream::RecvPartyRemove()
{
	TPacketGCPartyRemove kPartyRemovePacket;
	if (!Recv(sizeof(kPartyRemovePacket), &kPartyRemovePacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RemovePartyMember", Py_BuildValue("(i)", kPartyRemovePacket.pid));
	Tracef(" >> RecvPartyRemove : %d\n", kPartyRemovePacket.pid);

	return true;
}

bool CPythonNetworkStream::RecvPartyLink()
{
	TPacketGCPartyLink kPartyLinkPacket;
	if (!Recv(sizeof(kPartyLinkPacket), &kPartyLinkPacket))
		return false;

	CPythonPlayer::Instance().LinkPartyMember(kPartyLinkPacket.pid, kPartyLinkPacket.vid);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "LinkPartyMember", Py_BuildValue("(ii)", kPartyLinkPacket.pid, kPartyLinkPacket.vid));	

	Tracef(" >> RecvPartyLink : %d, %d\n", kPartyLinkPacket.pid, kPartyLinkPacket.vid);

	return true;
}

bool CPythonNetworkStream::RecvPartyUnlink()
{
	TPacketGCPartyUnlink kPartyUnlinkPacket;
	if (!Recv(sizeof(kPartyUnlinkPacket), &kPartyUnlinkPacket))
		return false;

	CPythonPlayer::Instance().UnlinkPartyMember(kPartyUnlinkPacket.pid);

	if (CPythonPlayer::Instance().IsMainCharacterIndex(kPartyUnlinkPacket.vid))
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "UnlinkAllPartyMember", Py_BuildValue("()"));
	}
	else
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "UnlinkPartyMember", Py_BuildValue("(i)", kPartyUnlinkPacket.pid));
	}

	Tracef(" >> RecvPartyUnlink : %d, %d\n", kPartyUnlinkPacket.pid, kPartyUnlinkPacket.vid);

	return true;
}

bool CPythonNetworkStream::RecvPartyParameter()
{
	TPacketGCPartyParameter kPartyParameterPacket;
	if (!Recv(sizeof(kPartyParameterPacket), &kPartyParameterPacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ChangePartyParameter", Py_BuildValue("(i)", kPartyParameterPacket.bDistributeMode));
	Tracef(" >> RecvPartyParameter : %d\n", kPartyParameterPacket.bDistributeMode);

	return true;
}

// Party
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// Guild

bool CPythonNetworkStream::SendGuildAddMemberPacket(DWORD dwVID)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_ADD_MEMBER;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(dwVID), &dwVID))
		return false;

	Tracef(" SendGuildAddMemberPacket\n", dwVID);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildRemoveMemberPacket(DWORD dwPID)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_REMOVE_MEMBER;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(dwPID), &dwPID))
		return false;

	Tracef(" SendGuildRemoveMemberPacket %d\n", dwPID);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildChangeGradeNamePacket(BYTE byGradeNumber, const char * c_szName)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_CHANGE_GRADE_NAME;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(byGradeNumber), &byGradeNumber))
		return false;

	char szName[GUILD_GRADE_NAME_MAX_LEN+1];
	strncpy(szName, c_szName, GUILD_GRADE_NAME_MAX_LEN);
	szName[GUILD_GRADE_NAME_MAX_LEN] = '\0';

	if (!Send(sizeof(szName), &szName))
		return false;

	Tracef(" SendGuildChangeGradeNamePacket %d, %s\n", byGradeNumber, c_szName);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildChangeGradeAuthorityPacket(BYTE byGradeNumber, BYTE byAuthority)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_CHANGE_GRADE_AUTHORITY;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(byGradeNumber), &byGradeNumber))
		return false;
	if (!Send(sizeof(byAuthority), &byAuthority))
		return false;

	Tracef(" SendGuildChangeGradeAuthorityPacket %d, %d\n", byGradeNumber, byAuthority);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildOfferPacket(DWORD dwExperience)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_OFFER;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(dwExperience), &dwExperience))
		return false;

	Tracef(" SendGuildOfferPacket %d\n", dwExperience);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildPostCommentPacket(const char * c_szMessage)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_POST_COMMENT;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	BYTE bySize = BYTE(strlen(c_szMessage)) + 1;
	if (!Send(sizeof(bySize), &bySize))
		return false;
	if (!Send(bySize, c_szMessage))
		return false;

	Tracef(" SendGuildPostCommentPacket %d, %s\n", bySize, c_szMessage);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildDeleteCommentPacket(DWORD dwIndex)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_DELETE_COMMENT;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwIndex), &dwIndex))
		return false;

	Tracef(" SendGuildDeleteCommentPacket %d\n", dwIndex);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildRefreshCommentsPacket(DWORD dwHighestIndex)
{
	static DWORD s_LastTime = timeGetTime() - 1001;

	if (timeGetTime() - s_LastTime < 1000)
		return true;
	s_LastTime = timeGetTime();

	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_REFRESH_COMMENT;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	Tracef(" SendGuildRefreshCommentPacket %d\n", dwHighestIndex);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildChangeMemberGradePacket(DWORD dwPID, BYTE byGrade)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_CHANGE_MEMBER_GRADE;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwPID), &dwPID))
		return false;
	if (!Send(sizeof(byGrade), &byGrade))
		return false;

	Tracef(" SendGuildChangeMemberGradePacket %d, %d\n", dwPID, byGrade);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildUseSkillPacket(DWORD dwSkillID, DWORD dwTargetVID)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_USE_SKILL;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwSkillID), &dwSkillID))
		return false;
	if (!Send(sizeof(dwTargetVID), &dwTargetVID))
		return false;

	Tracef(" SendGuildUseSkillPacket %d, %d\n", dwSkillID, dwTargetVID);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildChangeMemberGeneralPacket(DWORD dwPID, BYTE byFlag)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_CHANGE_MEMBER_GENERAL;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwPID), &dwPID))
		return false;
	if (!Send(sizeof(byFlag), &byFlag))
		return false;

	Tracef(" SendGuildChangeMemberGeneralFlagPacket %d, %d\n", dwPID, byFlag);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildInviteAnswerPacket(DWORD dwGuildID, BYTE byAnswer)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwGuildID), &dwGuildID))
		return false;
	if (!Send(sizeof(byAnswer), &byAnswer))
		return false;

	Tracef(" SendGuildInviteAnswerPacket %d, %d\n", dwGuildID, byAnswer);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildChargeGSPPacket(DWORD dwMoney)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_CHARGE_GSP;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwMoney), &dwMoney))
		return false;

	Tracef(" SendGuildChargeGSPPacket %d\n", dwMoney);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildDepositMoneyPacket(DWORD dwMoney)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_DEPOSIT_MONEY;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(dwMoney), &dwMoney))
		return false;

	Tracef(" SendGuildDepositMoneyPacket %d\n", dwMoney);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildWithdrawMoneyPacket(DWORD dwMoney)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_WITHDRAW_MONEY;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(dwMoney), &dwMoney))
		return false;

	Tracef(" SendGuildWithdrawMoneyPacket %d\n", dwMoney);
	return SendSequence();
}

bool CPythonNetworkStream::RecvGuild()
{
    TPacketGCGuild GuildPacket;
	if (!Recv(sizeof(GuildPacket), &GuildPacket))
		return false;

	switch(GuildPacket.subheader)
	{
		case GUILD_SUBHEADER_GC_LOGIN:
		{
			DWORD dwPID;
			if (!Recv(sizeof(DWORD), &dwPID))
				return false;

			// Messenger
			CPythonGuild::TGuildMemberData * pGuildMemberData;
			if (CPythonGuild::Instance().GetMemberDataPtrByPID(dwPID, &pGuildMemberData))
				if (0 != pGuildMemberData->strName.compare(CPythonPlayer::Instance().GetName()))
					CPythonMessenger::Instance().LoginGuildMember(pGuildMemberData->strName.c_str());

			//Tracef(" <Login> %d\n", dwPID);
			break;
		}
		case GUILD_SUBHEADER_GC_LOGOUT:
		{
			DWORD dwPID;
			if (!Recv(sizeof(DWORD), &dwPID))
				return false;

			// Messenger
			CPythonGuild::TGuildMemberData * pGuildMemberData;
			if (CPythonGuild::Instance().GetMemberDataPtrByPID(dwPID, &pGuildMemberData))
				if (0 != pGuildMemberData->strName.compare(CPythonPlayer::Instance().GetName()))
					CPythonMessenger::Instance().LogoutGuildMember(pGuildMemberData->strName.c_str());

			//Tracef(" <Logout> %d\n", dwPID);
			break;
		}
		case GUILD_SUBHEADER_GC_REMOVE:
		{
			DWORD dwPID;
			if (!Recv(sizeof(dwPID), &dwPID))
				return false;

			// Main Player ì¼ ê²½ìš° DeleteGuild
			if (CPythonGuild::Instance().IsMainPlayer(dwPID))
			{
				CPythonGuild::Instance().Destroy();
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "DeleteGuild", Py_BuildValue("()"));
				CPythonMessenger::Instance().RemoveAllGuildMember();
				__SetGuildID(0);
				__RefreshMessengerWindow();
				__RefreshTargetBoard();
				__RefreshCharacterWindow();
			}
			else
			{
				// Get Member Name
				std::string strMemberName = "";
				CPythonGuild::TGuildMemberData * pData;
				if (CPythonGuild::Instance().GetMemberDataPtrByPID(dwPID, &pData))
				{
					strMemberName = pData->strName;
					CPythonMessenger::Instance().RemoveGuildMember(pData->strName.c_str());
				}

				CPythonGuild::Instance().RemoveMember(dwPID);

				// Refresh
				__RefreshTargetBoardByName(strMemberName.c_str());
				__RefreshGuildWindowMemberPage();
			}

			Tracef(" <Remove> %d\n", dwPID);
			break;
		}
		case GUILD_SUBHEADER_GC_LIST:
		{
			int iPacketSize = int(GuildPacket.size) - sizeof(GuildPacket);

			for (; iPacketSize > 0;)
			{
				TPacketGCGuildSubMember memberPacket;
				if (!Recv(sizeof(memberPacket), &memberPacket))
					return false;

				char szName[CHARACTER_NAME_MAX_LEN+1] = "";
				if (memberPacket.byNameFlag)
				{
					if (!Recv(sizeof(szName), &szName))
						return false;

					iPacketSize -= CHARACTER_NAME_MAX_LEN+1;
				}
				else
				{
					CPythonGuild::TGuildMemberData * pMemberData;
					if (CPythonGuild::Instance().GetMemberDataPtrByPID(memberPacket.pid, &pMemberData))
					{
						strncpy(szName, pMemberData->strName.c_str(), CHARACTER_NAME_MAX_LEN);
					}
				}

				//Tracef(" <List> %d : %s, %d (%d, %d, %d)\n", memberPacket.pid, szName, memberPacket.byGrade, memberPacket.byJob, memberPacket.byLevel, memberPacket.dwOffer);

				CPythonGuild::SGuildMemberData GuildMemberData;
				GuildMemberData.dwPID = memberPacket.pid;
				GuildMemberData.byGrade = memberPacket.byGrade;
				GuildMemberData.strName = szName;
				GuildMemberData.byJob = memberPacket.byJob;
				GuildMemberData.byLevel = memberPacket.byLevel;
				GuildMemberData.dwOffer = memberPacket.dwOffer;
				GuildMemberData.byGeneralFlag = memberPacket.byIsGeneral;
				CPythonGuild::Instance().RegisterMember(GuildMemberData);

				// Messenger
				if (strcmp(szName, CPythonPlayer::Instance().GetName()))
					CPythonMessenger::Instance().AppendGuildMember(szName);

				__RefreshTargetBoardByName(szName);

				iPacketSize -= sizeof(memberPacket);
			}

			__RefreshGuildWindowInfoPage();
			__RefreshGuildWindowMemberPage();
			__RefreshMessengerWindow();
			__RefreshCharacterWindow();
			break;
		}
		case GUILD_SUBHEADER_GC_GRADE:
		{
			BYTE byCount;
			if (!Recv(sizeof(byCount), &byCount))
				return false;

			for (BYTE i = 0; i < byCount; ++ i)
			{
				BYTE byIndex;
				if (!Recv(sizeof(byCount), &byIndex))
					return false;
				TPacketGCGuildSubGrade GradePacket;
				if (!Recv(sizeof(GradePacket), &GradePacket))
					return false;

				CPythonGuild::Instance().SetGradeData(byIndex, CPythonGuild::SGuildGradeData(GradePacket.auth_flag, GradePacket.grade_name));
				//Tracef(" <Grade> [%d/%d] : %s, %d\n", byIndex, byCount, GradePacket.grade_name, GradePacket.auth_flag);
			}
			__RefreshGuildWindowGradePage();
			__RefreshGuildWindowMemberPageGradeComboBox();
			break;
		}
		case GUILD_SUBHEADER_GC_GRADE_NAME:
		{
			BYTE byGradeNumber;
			if (!Recv(sizeof(byGradeNumber), &byGradeNumber))
				return false;

			char szGradeName[GUILD_GRADE_NAME_MAX_LEN+1] = "";
			if (!Recv(sizeof(szGradeName), &szGradeName))
				return false;

			CPythonGuild::Instance().SetGradeName(byGradeNumber, szGradeName);
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildGrade", Py_BuildValue("()"));

			Tracef(" <Change Grade Name> %d, %s\n", byGradeNumber, szGradeName);
			__RefreshGuildWindowGradePage();
			__RefreshGuildWindowMemberPageGradeComboBox();
			break;
		}
		case GUILD_SUBHEADER_GC_GRADE_AUTH:
		{
			BYTE byGradeNumber;
			if (!Recv(sizeof(byGradeNumber), &byGradeNumber))
				return false;
			BYTE byAuthorityFlag;
			if (!Recv(sizeof(byAuthorityFlag), &byAuthorityFlag))
				return false;

			CPythonGuild::Instance().SetGradeAuthority(byGradeNumber, byAuthorityFlag);
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildGrade", Py_BuildValue("()"));

			Tracef(" <Change Grade Authority> %d, %d\n", byGradeNumber, byAuthorityFlag);
			__RefreshGuildWindowGradePage();
			break;
		}
		case GUILD_SUBHEADER_GC_INFO:
		{
			TPacketGCGuildInfo GuildInfo;
			if (!Recv(sizeof(GuildInfo), &GuildInfo))
				return false;

			CPythonGuild::Instance().EnableGuild();
			CPythonGuild::TGuildInfo & rGuildInfo = CPythonGuild::Instance().GetGuildInfoRef();
			strncpy(rGuildInfo.szGuildName, GuildInfo.name, GUILD_NAME_MAX_LEN);
			rGuildInfo.szGuildName[GUILD_NAME_MAX_LEN] = '\0';

			rGuildInfo.dwGuildID = GuildInfo.guild_id;
			rGuildInfo.dwMasterPID = GuildInfo.master_pid;
			rGuildInfo.dwGuildLevel = GuildInfo.level;
			rGuildInfo.dwCurrentExperience = GuildInfo.exp;
			rGuildInfo.dwCurrentMemberCount = GuildInfo.member_count;
			rGuildInfo.dwMaxMemberCount = GuildInfo.max_member_count;
			rGuildInfo.dwGuildMoney = GuildInfo.gold;
			rGuildInfo.bHasLand = GuildInfo.hasLand;

			//Tracef(" <Info> %s, %d, %d : %d\n", GuildInfo.name, GuildInfo.master_pid, GuildInfo.level, rGuildInfo.bHasLand);
			__RefreshGuildWindowInfoPage();
			break;
		}
		case GUILD_SUBHEADER_GC_COMMENTS:
		{
			BYTE byCount;
			if (!Recv(sizeof(byCount), &byCount))
				return false;

			CPythonGuild::Instance().ClearComment();
			//Tracef(" >>> Comments Count : %d\n", byCount);

			for (BYTE i = 0; i < byCount; ++i)
			{
				DWORD dwCommentID;
				if (!Recv(sizeof(dwCommentID), &dwCommentID))
					return false;

				char szName[CHARACTER_NAME_MAX_LEN+1] = "";
				if (!Recv(sizeof(szName), &szName))
					return false;

				char szComment[GULID_COMMENT_MAX_LEN+1] = "";
				if (!Recv(sizeof(szComment), &szComment))
					return false;

				//Tracef(" [Comment-%d] : %s, %s\n", dwCommentID, szName, szComment);
				CPythonGuild::Instance().RegisterComment(dwCommentID, szName, szComment);
			}

			__RefreshGuildWindowBoardPage();
			break;
		}
		case GUILD_SUBHEADER_GC_CHANGE_EXP:
		{
			BYTE byLevel;
			if (!Recv(sizeof(byLevel), &byLevel))
				return false;
			DWORD dwEXP;
			if (!Recv(sizeof(dwEXP), &dwEXP))
				return false;
			CPythonGuild::Instance().SetGuildEXP(byLevel, dwEXP);
			Tracef(" <ChangeEXP> %d, %d\n", byLevel, dwEXP);
			__RefreshGuildWindowInfoPage();
			break;
		}
		case GUILD_SUBHEADER_GC_CHANGE_MEMBER_GRADE:
		{
			DWORD dwPID;
			if (!Recv(sizeof(dwPID), &dwPID))
				return false;
			BYTE byGrade;
			if (!Recv(sizeof(byGrade), &byGrade))
				return false;
			CPythonGuild::Instance().ChangeGuildMemberGrade(dwPID, byGrade);
			Tracef(" <ChangeMemberGrade> %d, %d\n", dwPID, byGrade);
			__RefreshGuildWindowMemberPage();
			break;
		}
		case GUILD_SUBHEADER_GC_SKILL_INFO:
		{
			CPythonGuild::TGuildSkillData & rSkillData = CPythonGuild::Instance().GetGuildSkillDataRef();
			if (!Recv(sizeof(rSkillData.bySkillPoint), &rSkillData.bySkillPoint))
				return false;
			if (!Recv(sizeof(rSkillData.bySkillLevel), rSkillData.bySkillLevel))
				return false;
			if (!Recv(sizeof(rSkillData.wGuildPoint), &rSkillData.wGuildPoint))
				return false;
			if (!Recv(sizeof(rSkillData.wMaxGuildPoint), &rSkillData.wMaxGuildPoint))
				return false;

			Tracef(" <SkillInfo> %d / %d, %d\n", rSkillData.bySkillPoint, rSkillData.wGuildPoint, rSkillData.wMaxGuildPoint);
			__RefreshGuildWindowSkillPage();
			break;
		}
		case GUILD_SUBHEADER_GC_CHANGE_MEMBER_GENERAL:
		{
			DWORD dwPID;
			if (!Recv(sizeof(dwPID), &dwPID))
				return false;
			BYTE byFlag;
			if (!Recv(sizeof(byFlag), &byFlag))
				return false;

			CPythonGuild::Instance().ChangeGuildMemberGeneralFlag(dwPID, byFlag);
			Tracef(" <ChangeMemberGeneralFlag> %d, %d\n", dwPID, byFlag);
			__RefreshGuildWindowMemberPage();
			break;
		}
		case GUILD_SUBHEADER_GC_GUILD_INVITE:
		{
			DWORD dwGuildID;
			if (!Recv(sizeof(dwGuildID), &dwGuildID))
				return false;
			char szGuildName[GUILD_NAME_MAX_LEN+1];
			if (!Recv(GUILD_NAME_MAX_LEN, &szGuildName))
				return false;

			szGuildName[GUILD_NAME_MAX_LEN] = 0;

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RecvGuildInviteQuestion", Py_BuildValue("(is)", dwGuildID, szGuildName));
			Tracef(" <Guild Invite> %d, %s\n", dwGuildID, szGuildName);
			break;
		}
		case GUILD_SUBHEADER_GC_WAR:
		{
			TPacketGCGuildWar kGuildWar;
			if (!Recv(sizeof(kGuildWar), &kGuildWar))
				return false;

			switch (kGuildWar.bWarState)
			{
				case GUILD_WAR_SEND_DECLARE:
					Tracef(" >> GUILD_SUBHEADER_GC_WAR : GUILD_WAR_SEND_DECLARE\n");
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], 
						"BINARY_GuildWar_OnSendDeclare", 
						Py_BuildValue("(i)", kGuildWar.dwGuildOpp)
					);
					break;
				case GUILD_WAR_RECV_DECLARE:
					Tracef(" >> GUILD_SUBHEADER_GC_WAR : GUILD_WAR_RECV_DECLARE\n");
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], 
						"BINARY_GuildWar_OnRecvDeclare", 
						Py_BuildValue("(ii)", kGuildWar.dwGuildOpp, kGuildWar.bType)
					);
					break;
				case GUILD_WAR_ON_WAR:
					Tracef(" >> GUILD_SUBHEADER_GC_WAR : GUILD_WAR_ON_WAR : %d, %d\n", kGuildWar.dwGuildSelf, kGuildWar.dwGuildOpp);
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], 
						"BINARY_GuildWar_OnStart", 
						Py_BuildValue("(ii)", kGuildWar.dwGuildSelf, kGuildWar.dwGuildOpp)
					);
					CPythonGuild::Instance().StartGuildWar(kGuildWar.dwGuildOpp);
					break;
				case GUILD_WAR_END:
					Tracef(" >> GUILD_SUBHEADER_GC_WAR : GUILD_WAR_END\n");
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], 
						"BINARY_GuildWar_OnEnd", 
						Py_BuildValue("(ii)", kGuildWar.dwGuildSelf, kGuildWar.dwGuildOpp)
					);
					CPythonGuild::Instance().EndGuildWar(kGuildWar.dwGuildOpp);
					break;
			}
			break;
		}
		case GUILD_SUBHEADER_GC_GUILD_NAME:
		{
			DWORD dwID;
			char szGuildName[GUILD_NAME_MAX_LEN+1];

			int iPacketSize = int(GuildPacket.size) - sizeof(GuildPacket);

			int nItemSize = sizeof(dwID) + GUILD_NAME_MAX_LEN;

			assert(iPacketSize%nItemSize==0 && "GUILD_SUBHEADER_GC_GUILD_NAME");

			for (; iPacketSize > 0;)
			{
				if (!Recv(sizeof(dwID), &dwID))
					return false;
				
				if (!Recv(GUILD_NAME_MAX_LEN, &szGuildName))
					return false;

				szGuildName[GUILD_NAME_MAX_LEN] = 0;

				//Tracef(" >> GulidName [%d : %s]\n", dwID, szGuildName);
				CPythonGuild::Instance().RegisterGuildName(dwID, szGuildName);
				iPacketSize -= nItemSize;
			}
			break;
		}
		case GUILD_SUBHEADER_GC_GUILD_WAR_LIST:
		{
			DWORD dwSrcGuildID;
			DWORD dwDstGuildID;

			int iPacketSize = int(GuildPacket.size) - sizeof(GuildPacket);
			int nItemSize = sizeof(dwSrcGuildID) + sizeof(dwDstGuildID);

			assert(iPacketSize%nItemSize==0 && "GUILD_SUBHEADER_GC_GUILD_WAR_LIST");

			for (; iPacketSize > 0;)
			{				
				if (!Recv(sizeof(dwSrcGuildID), &dwSrcGuildID))
					return false;
				
				if (!Recv(sizeof(dwDstGuildID), &dwDstGuildID))
					return false;

				Tracef(" >> GulidWarList [%d vs %d]\n", dwSrcGuildID, dwDstGuildID);
				CInstanceBase::InsertGVGKey(dwSrcGuildID, dwDstGuildID);
				CPythonCharacterManager::Instance().ChangeGVG(dwSrcGuildID, dwDstGuildID);
				iPacketSize -= nItemSize;
			}
			break;
		}
		case GUILD_SUBHEADER_GC_GUILD_WAR_END_LIST:
		{
			DWORD dwSrcGuildID;
			DWORD dwDstGuildID;

			int iPacketSize = int(GuildPacket.size) - sizeof(GuildPacket);
			int nItemSize = sizeof(dwSrcGuildID) + sizeof(dwDstGuildID);

			assert(iPacketSize%nItemSize==0 && "GUILD_SUBHEADER_GC_GUILD_WAR_END_LIST");

			for (; iPacketSize > 0;)
			{
				
				if (!Recv(sizeof(dwSrcGuildID), &dwSrcGuildID))
					return false;
				
				if (!Recv(sizeof(dwDstGuildID), &dwDstGuildID))
					return false;

				Tracef(" >> GulidWarEndList [%d vs %d]\n", dwSrcGuildID, dwDstGuildID);
				CInstanceBase::RemoveGVGKey(dwSrcGuildID, dwDstGuildID);
				CPythonCharacterManager::Instance().ChangeGVG(dwSrcGuildID, dwDstGuildID);
				iPacketSize -= nItemSize;
			}
			break;
		}
		case GUILD_SUBHEADER_GC_WAR_POINT:
		{
			TPacketGuildWarPoint GuildWarPoint;
			if (!Recv(sizeof(GuildWarPoint), &GuildWarPoint))
				return false;

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], 
				"BINARY_GuildWar_OnRecvPoint", 
				Py_BuildValue("(iii)", GuildWarPoint.dwGainGuildID, GuildWarPoint.dwOpponentGuildID, GuildWarPoint.lPoint)
			);
			break;
		}
		case GUILD_SUBHEADER_GC_MONEY_CHANGE:
		{
			DWORD dwMoney;
			if (!Recv(sizeof(dwMoney), &dwMoney))
				return false;

			CPythonGuild::Instance().SetGuildMoney(dwMoney);

			__RefreshGuildWindowInfoPage();
			Tracef(" >> Guild Money Change : %d\n", dwMoney);
			break;
		}
#if defined(ENABLE_COMMUNITY_GUILD_RENEWAL)
		case GUILD_SUBHEADER_GC_MEMBER_LOCATION:
		{
			TPacketGCGuildMemberLocation locPacket;
			if (!Recv(sizeof(locPacket), &locPacket))
				return false;

			CPythonCommunity::Instance().OnGuildMemberLocation(locPacket.dwPID, locPacket.bChannel, locPacket.lMapIndex);
			break;
		}
#endif
	}

	return true;
}

// Guild
//////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
// Fishing

bool CPythonNetworkStream::SendFishingPacket(int iRotation)
{
	BYTE byHeader = HEADER_CG_FISHING;
	if (!Send(sizeof(byHeader), &byHeader))
		return false;
	BYTE byPacketRotation = iRotation / 5;
	if (!Send(sizeof(BYTE), &byPacketRotation))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendGiveItemPacket(DWORD dwTargetVID, TItemPos ItemPos, int iItemCount)
{
	TPacketCGGiveItem GiveItemPacket;
	GiveItemPacket.byHeader = HEADER_CG_GIVE_ITEM;
	GiveItemPacket.dwTargetVID = dwTargetVID;
	GiveItemPacket.ItemPos = ItemPos;
	GiveItemPacket.byItemCount = iItemCount;

	if (!Send(sizeof(GiveItemPacket), &GiveItemPacket))
		return false;

	return SendSequence();
}

#ifdef ENABLE_BATTLE_FIELD
bool CPythonNetworkStream::__RecvCombatZoneRankingPacket()
{
	TPacketGCCombatZoneRankingData p;
	if (!Recv(sizeof(TPacketGCCombatZoneRankingData), &p))
	{
		Tracen("Recv PacketGCCombatZoneRankingData Error");
		return false;
	}

	CPythonCombatZone::instance()->Initialize(p);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_CombatZone_Manager", Py_BuildValue("(s)", "RegisterRank"));
	return true;
}

bool CPythonNetworkStream::SendCombatZoneRequestActionPacket(int iAction, int iValue)
{
	TPacketCGCombatZoneRequestAction p;
	p.header = HEADER_CG_COMBAT_ZONE_REQUEST_ACTION;
	p.action = iAction;
	p.value	 = iValue;

	if (!Send(sizeof(p), &p))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::__RecvCombatZonePacket()
{
	TPacketGCSendCombatZone p;

	if (!Recv(sizeof(TPacketGCSendCombatZone), &p))
	{
		Tracen("Recv CombatZone Packet Error");
		return false;
	}

	switch (p.sub_header)
	{
		/*
		case COMBAT_ZONE_SUB_HEADER_ADD_LEAVING_TARGET:
			CPythonBackground::Instance().CreateCombatZoneTargetsLeaving(p.m_pInfoData[0], p.m_pInfoData[1]);
			break;
		
		case COMBAT_ZONE_SUB_HEADER_REMOVE_LEAVING_TARGET:
			CPythonBackground::Instance().DeleteCombatZoneTargetsLeaving(0);
			break;
		*/
			
		case COMBAT_ZONE_SUB_HEADER_FLASH_ON_MINIMAP:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_CombatZone_Manager", Py_BuildValue("(s)", "StartFlashing"));
			break;		
			
		case COMBAT_ZONE_SUB_HEADER_OPEN_RANKING:
		{
			DWORD dwPoints = p.m_pInfoData[0];
			DWORD dwTimeRemaining = p.m_pInfoData[1];
			DWORD dwCurMobsKills = p.m_pInfoData[2];
			DWORD dwMaxMobsKills = p.m_pInfoData[3];
			
			CPythonCombatZone::instance()->SendDataDays(p.m_pDataDays, p.isRunning);
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_CombatZone_Manager", Py_BuildValue("(siiii)", "OpenWindow", dwPoints, dwTimeRemaining, dwCurMobsKills, dwMaxMobsKills));
		}
		break;	

		case COMBAT_ZONE_SUB_HEADER_REFRESH_SHOP:
		{
			DWORD dwPoints = p.m_pInfoData[0];
			DWORD dwCurLimit = p.m_pInfoData[1];
			DWORD dwMaxLimit = p.m_pInfoData[2];
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_CombatZone_Manager", Py_BuildValue("(siii)", "RefreshShop", dwPoints, dwCurLimit, dwMaxLimit));
		}
		break;	
		default:
			return false;
	}
	return true;
}
#endif

bool CPythonNetworkStream::RecvFishing()
{
	TPacketGCFishing FishingPacket;
	if (!Recv(sizeof(FishingPacket), &FishingPacket))
		return false;

	CInstanceBase * pFishingInstance = NULL;
	if (FISHING_SUBHEADER_GC_FISH != FishingPacket.subheader)
	{
		pFishingInstance = CPythonCharacterManager::Instance().GetInstancePtr(FishingPacket.info);
		if (!pFishingInstance)
			return true;
	}

	switch (FishingPacket.subheader)
	{
		case FISHING_SUBHEADER_GC_START:
			pFishingInstance->StartFishing(float(FishingPacket.dir) * 5.0f);
			break;
		case FISHING_SUBHEADER_GC_STOP:
			if (pFishingInstance->IsFishing())
				pFishingInstance->StopFishing();
			break;
		case FISHING_SUBHEADER_GC_REACT:
			if (pFishingInstance->IsFishing())
			{
				pFishingInstance->SetFishEmoticon(); // Fish Emoticon
				pFishingInstance->ReactFishing();
			}
			break;
		case FISHING_SUBHEADER_GC_SUCCESS:
			pFishingInstance->CatchSuccess();
			break;
		case FISHING_SUBHEADER_GC_FAIL:
			pFishingInstance->CatchFail();
			if (pFishingInstance == CPythonCharacterManager::Instance().GetMainInstancePtr())
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingFailure", Py_BuildValue("()"));
			}
			break;
		case FISHING_SUBHEADER_GC_FISH:
		{
			DWORD dwFishID = FishingPacket.info;

			if (0 == FishingPacket.info)
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingNotifyUnknown", Py_BuildValue("()"));
				return true;
			}

			CItemData * pItemData;
			if (!CItemManager::Instance().GetItemDataPointer(dwFishID, &pItemData))
				return true;

			CInstanceBase * pMainInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
			if (!pMainInstance)
				return true;

			if (pMainInstance->IsFishing())
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingNotify", Py_BuildValue("(is)", CItemData::ITEM_TYPE_FISH == pItemData->GetType(), pItemData->GetName()));
			}
			else
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingSuccess", Py_BuildValue("(is)", CItemData::ITEM_TYPE_FISH == pItemData->GetType(), pItemData->GetName()));
			}
			break;
		}
	}

	return true;
}
// Fishing
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// Dungeon
bool CPythonNetworkStream::RecvDungeon()
{
	TPacketGCDungeon DungeonPacket;
	if (!Recv(sizeof(DungeonPacket), &DungeonPacket))
		return false;

	switch (DungeonPacket.subheader)
	{
		case DUNGEON_SUBHEADER_GC_TIME_ATTACK_START:
		{
			break;
		}
		case DUNGEON_SUBHEADER_GC_DESTINATION_POSITION:
		{
			unsigned long ulx, uly;
			if (!Recv(sizeof(ulx), &ulx))
				return false;
			if (!Recv(sizeof(uly), &uly))
				return false;

			CPythonPlayer::Instance().SetDungeonDestinationPosition(ulx, uly);
			break;
		}
	}

	return true;
}
// Dungeon
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// MyShop
bool CPythonNetworkStream::SendBuildPrivateShopPacket(const char * c_szName, const std::vector<TShopItemTable> & c_rSellingItemStock)
{
	TPacketCGMyShop packet;
	packet.bHeader = HEADER_CG_MYSHOP;
	strncpy(packet.szSign, c_szName, SHOP_SIGN_MAX_LEN);
	packet.bCount = c_rSellingItemStock.size();
	if (!Send(sizeof(packet), &packet))
		return false;

	for (std::vector<TShopItemTable>::const_iterator itor = c_rSellingItemStock.begin(); itor < c_rSellingItemStock.end(); ++itor)
	{
		const TShopItemTable & c_rItem = *itor;
		if (!Send(sizeof(c_rItem), &c_rItem))
			return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvShopSignPacket()
{
	TPacketGCShopSign p;
	if (!Recv(sizeof(TPacketGCShopSign), &p))
		return false;

	CPythonPlayer& rkPlayer=CPythonPlayer::Instance();
	
	if (0 == strlen(p.szSign))
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], 
			"BINARY_PrivateShop_Disappear", 
			Py_BuildValue("(i)", p.dwVID)
		);

		if (rkPlayer.IsMainCharacterIndex(p.dwVID))
			rkPlayer.ClosePrivateShop();
	}
	else
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], 
			"BINARY_PrivateShop_Appear", 
			Py_BuildValue("(is)", p.dwVID, p.szSign)
		);

		if (rkPlayer.IsMainCharacterIndex(p.dwVID))
			rkPlayer.OpenPrivateShop();
	}

	return true;
}
/////////////////////////////////////////////////////////////////////////

bool CPythonNetworkStream::RecvTimePacket()
{
	TPacketGCTime TimePacket;
	if (!Recv(sizeof(TimePacket), &TimePacket))
		return false;

	IAbstractApplication& rkApp=IAbstractApplication::GetSingleton();
	rkApp.SetServerTime(TimePacket.time);

	return true;
}

bool CPythonNetworkStream::RecvWalkModePacket()
{
	TPacketGCWalkMode WalkModePacket;
	if (!Recv(sizeof(WalkModePacket), &WalkModePacket))
		return false;

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(WalkModePacket.vid);
	if (pInstance)
	{
		if (WALKMODE_RUN == WalkModePacket.mode)
		{
			pInstance->SetRunMode();
		}
		else
		{
			pInstance->SetWalkMode();
		}
	}

	return true;
}

bool CPythonNetworkStream::RecvChangeSkillGroupPacket()
{
	TPacketGCChangeSkillGroup ChangeSkillGroup;
	if (!Recv(sizeof(ChangeSkillGroup), &ChangeSkillGroup))
		return false;

	m_dwMainActorSkillGroup = ChangeSkillGroup.skill_group;

	CPythonPlayer::Instance().NEW_ClearSkillData();
	__RefreshCharacterWindow();
	return true;
}

void CPythonNetworkStream::__TEST_SetSkillGroupFake(int iIndex)
{
	m_dwMainActorSkillGroup = DWORD(iIndex);

	CPythonPlayer::Instance().NEW_ClearSkillData();
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshCharacter", Py_BuildValue("()"));
}

#ifdef ENABLE_WHISPER_ADMIN_SYSTEM
bool CPythonNetworkStream::SendWhisperAdminPacket(const char* c_szText, const char* c_szLang, int color)
{
	TPacketCGWhisperAdmin p;
	p.header = HEADER_CG_WHISPER_ADMIN;
	strncpy(p.szText, c_szText, sizeof(p.szText));
	strncpy(p.szLang, c_szLang, sizeof(p.szLang));
	p.color	 = color;	

	if (!Send(sizeof(p), &p))
		return false;

	return SendSequence();
}
#endif

#ifdef ENABLE_ADMIN_BAN_MANAGER
bool CPythonNetworkStream::SendAdminBanManager(int action, const char* c_szUserName, const char* c_szReason, int duration)
{
	TPacketCGAdminBanManger p;
	p.header = HEADER_CG_ADMIN_BAN_MANAGER;
	p.action = action;
	strncpy(p.user_name, c_szUserName, sizeof(p.user_name));
	strncpy(p.reason, c_szReason, sizeof(p.reason));
	p.duration = duration;

	if (!Send(sizeof(p), &p))
		return false;

	return SendSequence();
}
#endif

bool CPythonNetworkStream::SendRefinePacket(UINT byPos, BYTE byType)
{
	TPacketCGRefine kRefinePacket;
	kRefinePacket.header = HEADER_CG_REFINE;
	kRefinePacket.pos = byPos;
	kRefinePacket.type = byType;

	if (!Send(sizeof(kRefinePacket), &kRefinePacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendSelectItemPacket(DWORD dwItemPos)
{
	TPacketCGScriptSelectItem kScriptSelectItem;
	kScriptSelectItem.header = HEADER_CG_SCRIPT_SELECT_ITEM;
	kScriptSelectItem.selection = dwItemPos;

	if (!Send(sizeof(kScriptSelectItem), &kScriptSelectItem))
		return false;

	return SendSequence();
}


bool CPythonNetworkStream::RecvRefineInformationPacket()
{
	return RecvRefineInformationPacketNew();
}

bool CPythonNetworkStream::RecvRefineInformationPacketNew()
{
	TPacketGCRefineInformationNew kRefineInfoPacket;
	if (!Recv(sizeof(kRefineInfoPacket), &kRefineInfoPacket))
		return false;

	TRefineTable& rkRefineTable = kRefineInfoPacket.refine_table;
#ifdef ENABLE_GLOVE_SYSTEM
	PyObject* poList = PyList_New(0);
	for (uint8_t uiApplySlot = 0; uiApplySlot < ITEM_APPLY_RANDOM_SLOT_MAX_NUM; ++uiApplySlot)
	{
		PyList_Append(poList, Py_BuildValue("(ii)", kRefineInfoPacket.aApplyRandom[uiApplySlot].bType, kRefineInfoPacket.aApplyRandom[uiApplySlot].sValue));
	}
#endif

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
		"OpenRefineDialog",
#ifdef ENABLE_GLOVE_SYSTEM
		Py_BuildValue("(iiiiiO)",
			kRefineInfoPacket.pos,
			kRefineInfoPacket.refine_table.result_vnum,
			rkRefineTable.cost,
			rkRefineTable.prob,
			kRefineInfoPacket.type,
			poList)
#else
		Py_BuildValue("(iiiii)",
			kRefineInfoPacket.pos,
			kRefineInfoPacket.refine_table.result_vnum,
			rkRefineTable.cost,
			rkRefineTable.prob,
			kRefineInfoPacket.type)
#endif
	);

	for (int i = 0; i < rkRefineTable.material_count; ++i)
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AppendMaterialToRefineDialog", Py_BuildValue("(ii)", rkRefineTable.materials[i].vnum, rkRefineTable.materials[i].count));

#ifdef _DEBUG
	Tracef(" >> RecvRefineInformationPacketNew(pos=%d, result_vnum=%d, cost=%d, prob=%d, type=%d, src_vnum=%d)\n",
		kRefineInfoPacket.pos,
		kRefineInfoPacket.refine_table.result_vnum,
		rkRefineTable.cost,
		rkRefineTable.prob,
		kRefineInfoPacket.type
// #ifdef ENABLE_GLOVE_SYSTEM
//		, 0
//#endif
	);
#endif

	return true;
}

bool CPythonNetworkStream::RecvNPCList()
{
	TPacketGCNPCPosition kNPCPosition;
	if (!Recv(sizeof(kNPCPosition), &kNPCPosition))
		return false;

	assert(int(kNPCPosition.size)-sizeof(kNPCPosition) == kNPCPosition.count*sizeof(TNPCPosition) && "HEADER_GC_NPC_POSITION");

	CPythonMiniMap::Instance().ClearAtlasMarkInfo();

	for (int i = 0; i < kNPCPosition.count; ++i)
	{
		TNPCPosition NPCPosition;
		if (!Recv(sizeof(TNPCPosition), &NPCPosition))
			return false;

		CPythonMiniMap::Instance().RegisterAtlasMark(NPCPosition.bType, NPCPosition.name, NPCPosition.x, NPCPosition.y);
	}

	return true;
}

bool CPythonNetworkStream::__SendCRCReportPacket()
{
	/*
	DWORD dwProcessCRC = 0;
	DWORD dwFileCRC = 0;
	CFilename exeFileName;
	//LPCVOID c_pvBaseAddress = NULL;

	GetExeCRC(dwProcessCRC, dwFileCRC);

	CFilename strRootPackFileName = CEterPackManager::Instance().GetRootPacketFileName();
	strRootPackFileName.ChangeDosPath();

	TPacketCGCRCReport kReportPacket;

	kReportPacket.header = HEADER_CG_CRC_REPORT;
	kReportPacket.byPackMode = CEterPackManager::Instance().GetSearchMode();
	kReportPacket.dwBinaryCRC32 = dwFileCRC;
	kReportPacket.dwProcessCRC32 = dwProcessCRC;
	kReportPacket.dwRootPackCRC32 = GetFileCRC32(strRootPackFileName.c_str());

	if (!Send(sizeof(kReportPacket), &kReportPacket))
		Tracef("SendClientReportPacket Error");

	return SendSequence();
	*/	
	return true;
}

bool CPythonNetworkStream::SendClientVersionPacket()
{
	std::string filename;

	GetExcutedFileName(filename);

	filename = CFileNameHelper::NoPath(filename);
	CFileNameHelper::ChangeDosPath(filename);

	if (LocaleService_IsEUROPE() && false == LocaleService_IsYMIR())
	{
		TPacketCGClientVersion2 kVersionPacket;
		kVersionPacket.header = HEADER_CG_CLIENT_VERSION2;
		strncpy(kVersionPacket.filename, filename.c_str(), sizeof(kVersionPacket.filename)-1);
		strncpy(kVersionPacket.timestamp, "1215955205", sizeof(kVersionPacket.timestamp)-1); // # python time.time ì•ìë¦¬
		//strncpy(kVersionPacket.timestamp, __TIMESTAMP__, sizeof(kVersionPacket.timestamp)-1); // old_string_ver
		//strncpy(kVersionPacket.timestamp, "1218055205", sizeof(kVersionPacket.timestamp)-1); // new_future
		//strncpy(kVersionPacket.timestamp, "1214055205", sizeof(kVersionPacket.timestamp)-1); // old_past

		if (!Send(sizeof(kVersionPacket), &kVersionPacket))
			Tracef("SendClientReportPacket Error");
	}
	else
	{
		TPacketCGClientVersion kVersionPacket;
		kVersionPacket.header = HEADER_CG_CLIENT_VERSION;
		strncpy(kVersionPacket.filename, filename.c_str(), sizeof(kVersionPacket.filename)-1);
		strncpy(kVersionPacket.timestamp, __TIMESTAMP__, sizeof(kVersionPacket.timestamp)-1);

		if (!Send(sizeof(kVersionPacket), &kVersionPacket))
			Tracef("SendClientReportPacket Error");
	}
	return SendSequence();
}

bool CPythonNetworkStream::RecvAffectAddPacket()
{
	TPacketGCAffectAdd kAffectAdd;
	if (!Recv(sizeof(kAffectAdd), &kAffectAdd))
		return false;

	TPacketAffectElement & rkElement = kAffectAdd.elem;

	if (rkElement.bPointIdxApplyOn == POINT_ENERGY)
	{
		CPythonPlayer::instance().SetStatus (POINT_ENERGY_END_TIME, CPythonApplication::Instance().GetServerTimeStamp() + rkElement.lDuration);
		__RefreshStatus();
	}

#ifdef ENABLE_BATTLE_ROYALE
	if (rkElement.dwType == CInstanceBase::NEW_AFFECT_BATTLE_ROYALE_FIELD_DAMAGE)
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenElectricFieldOnScreenImage", Py_BuildValue("(b)", true));
	}
#endif

#ifdef ENABLE_AUTO_QUQUE_ATTACK
	CPythonPlayer::Instance().SetTotalAutoFarmCount(3);
#endif

#ifdef ENABLE_FLOWER_EVENT
	CPythonPlayer::Instance().AddAffect(rkElement.dwType, kAffectAdd.elem);
#endif

#ifdef ENABLE_AUTO_SYSTEM
	if (rkElement.dwType == CInstanceBase::NEW_AFFECT_AUTO)
	{
		// CPythonCharacterManager::Instance().GetInstancePtrByName(CPythonPlayer::Instance().GetName())->SetAutoAffect(true);
		CPythonCharacterManager::Instance().SetAutoOnOff(true);
		CPythonPlayer::instance().AddAffect(rkElement.dwType, kAffectAdd.elem);
	}
#endif

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_NEW_AddAffect", Py_BuildValue("(iiii)", rkElement.dwType, rkElement.bPointIdxApplyOn, rkElement.lApplyValue, rkElement.lDuration));

	return true;
}

bool CPythonNetworkStream::RecvAffectRemovePacket()
{
	TPacketGCAffectRemove kAffectRemove;
	if (!Recv(sizeof(kAffectRemove), &kAffectRemove))
		return false;

#ifdef ENABLE_BATTLE_ROYALE
	if (kAffectRemove.dwType == CInstanceBase::NEW_AFFECT_BATTLE_ROYALE_FIELD_DAMAGE)
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenElectricFieldOnScreenImage", Py_BuildValue("(b)", false));
	}
#endif

#ifdef ENABLE_AUTO_QUQUE_ATTACK
	CPythonPlayer::Instance().SetTotalAutoFarmCount(3);
#endif

#ifdef ENABLE_FLOWER_EVENT
	CPythonPlayer::Instance().RemoveAffect(kAffectRemove.dwType, kAffectRemove.bApplyOn);
#endif

#ifdef ENABLE_AUTO_SYSTEM
	if (kAffectRemove.dwType == CInstanceBase::NEW_AFFECT_AUTO)
	{
		// CPythonCharacterManager::Instance().GetInstancePtrByName(CPythonPlayer::Instance().GetName())->SetAutoAffect(false);
		CPythonCharacterManager::Instance().SetAutoOnOff(false);
	}
#endif

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_NEW_RemoveAffect", Py_BuildValue("(ii)", kAffectRemove.dwType, kAffectRemove.bApplyOn));

	return true;
}

bool CPythonNetworkStream::RecvChannelPacket()
{
	TPacketGCChannel kChannelPacket;
	if (!Recv(sizeof(kChannelPacket), &kChannelPacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ReceiveChannel", Py_BuildValue("(i)", kChannelPacket.channel));

	return true;
}

#ifdef ENABLE_VIEW_EQUIPMENT_SYSTEM
bool CPythonNetworkStream::RecvViewEquipPacket()
{
	TPacketGCViewEquip kViewEquipPacket;
	if (!Recv(sizeof(kViewEquipPacket), &kViewEquipPacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenEquipmentDialog", Py_BuildValue("(i)", kViewEquipPacket.dwVID));

	for (int i = 0; i < 23; ++i)
	{
		TEquipmentItemSet & rItemSet = kViewEquipPacket.equips[i];
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetEquipmentDialogItem", Py_BuildValue("(iiiiiii)", kViewEquipPacket.dwVID, i, rItemSet.vnum, rItemSet.count, rItemSet.transmutation, rItemSet.dwRefineElement, rItemSet.set_value));

		for (int j = 0; j < ITEM_SOCKET_SLOT_MAX_NUM; ++j)
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetEquipmentDialogSocket", Py_BuildValue("(iiii)", kViewEquipPacket.dwVID, i, j, rItemSet.alSockets[j]));

		for (int k = 0; k < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++k)
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetEquipmentDialogAttr", Py_BuildValue("(iiiii)", kViewEquipPacket.dwVID, i, k, rItemSet.aAttr[k].bType, rItemSet.aAttr[k].sValue));
#ifdef ENABLE_GLOVE_SYSTEM
		for (int k = 0; k < ITEM_APPLY_RANDOM_SLOT_MAX_NUM; ++k)
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetEquipmentDialogRandomAttr", Py_BuildValue("(iiiii)", kViewEquipPacket.dwVID, i, k, rItemSet.aApplyRandom[k].bType, rItemSet.aApplyRandom[k].sValue));
#endif
	}

	return true;
}
#endif

bool CPythonNetworkStream::RecvLandPacket()
{
	TPacketGCLandList kLandList;
	if (!Recv(sizeof(kLandList), &kLandList))
		return false;
	std::vector<DWORD> kVec_dwGuildID;
	CPythonMiniMap & rkMiniMap = CPythonMiniMap::Instance();
	CPythonBackground & rkBG = CPythonBackground::Instance();
	CInstanceBase * pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
	rkMiniMap.ClearGuildArea();
	rkBG.ClearGuildArea();
	int iPacketSize = (kLandList.size - sizeof(TPacketGCLandList));
	for (; iPacketSize > 0; iPacketSize-=sizeof(TLandPacketElement))
	{
		TLandPacketElement kElement;
		if (!Recv(sizeof(TLandPacketElement), &kElement))
			return false;
		rkMiniMap.RegisterGuildArea(kElement.dwID,
									kElement.dwGuildID,
									kElement.x,
									kElement.y,
									kElement.width,
									kElement.height);
		if (pMainInstance)
		if (kElement.dwGuildID == pMainInstance->GetGuildID())
		{
			rkBG.RegisterGuildArea(kElement.x,
								   kElement.y,
								   kElement.x+kElement.width,
								   kElement.y+kElement.height);
		}
		if (0 != kElement.dwGuildID)
			kVec_dwGuildID.push_back(kElement.dwGuildID);
	}
	return true;
}

bool CPythonNetworkStream::RecvTargetCreatePacket()
{
	TPacketGCTargetCreate kTargetCreate;
	if (!Recv(sizeof(kTargetCreate), &kTargetCreate))
		return false;

	CPythonMiniMap & rkpyMiniMap = CPythonMiniMap::Instance();
	rkpyMiniMap.CreateTarget(kTargetCreate.lID, kTargetCreate.szTargetName);

//#ifdef _DEBUG
//	char szBuf[256+1];
//	_snprintf(szBuf, sizeof(szBuf), "íƒ€ê²Ÿì´ ìƒì„± ë˜ì—ˆìŠµë‹ˆë‹¤ [%d:%s]", kTargetCreate.lID, kTargetCreate.szTargetName);
//	CPythonChat::Instance().AppendChat(CHAT_TYPE_NOTICE, szBuf);
//	Tracef(" >> RecvTargetCreatePacket %d : %s\n", kTargetCreate.lID, kTargetCreate.szTargetName);
//#endif

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OpenAtlasWindow", Py_BuildValue("()"));
	return true;
}

bool CPythonNetworkStream::RecvTargetCreatePacketNew()
{
	TPacketGCTargetCreateNew kTargetCreate;
	if (!Recv(sizeof(kTargetCreate), &kTargetCreate))
		return false;

	CPythonMiniMap & rkpyMiniMap = CPythonMiniMap::Instance();
	CPythonBackground & rkpyBG = CPythonBackground::Instance();
	if (CREATE_TARGET_TYPE_LOCATION == kTargetCreate.byType)
	{
		rkpyMiniMap.CreateTarget(kTargetCreate.lID, kTargetCreate.szTargetName);
	}
#ifdef ENABLE_BATTLE_FIELD
	else if (CREATE_TARGET_TYPE_COMBAT_ZONE == kTargetCreate.byType)
	{
		rkpyBG.CreateCombatZoneTargetsLeaving(kTargetCreate.lID, kTargetCreate.dwVID);
		rkpyMiniMap.CreateTarget(kTargetCreate.lID,
			kTargetCreate.szTargetName
			,kTargetCreate.dwVID);
	}
#endif
	else
	{
		rkpyMiniMap.CreateTarget(kTargetCreate.lID, kTargetCreate.szTargetName, kTargetCreate.dwVID);
		rkpyBG.CreateTargetEffect(kTargetCreate.lID, kTargetCreate.dwVID);
	}

//#ifdef _DEBUG
//	char szBuf[256+1];
//	_snprintf(szBuf, sizeof(szBuf), "ìºë¦­í„° íƒ€ê²Ÿì´ ìƒì„± ë˜ì—ˆìŠµë‹ˆë‹¤ [%d:%s:%d]", kTargetCreate.lID, kTargetCreate.szTargetName, kTargetCreate.dwVID);
//	CPythonChat::Instance().AppendChat(CHAT_TYPE_NOTICE, szBuf);
//	Tracef(" >> RecvTargetCreatePacketNew %d : %d/%d\n", kTargetCreate.lID, kTargetCreate.byType, kTargetCreate.dwVID);
//#endif

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OpenAtlasWindow", Py_BuildValue("()"));
	return true;
}

bool CPythonNetworkStream::RecvTargetUpdatePacket()
{
	TPacketGCTargetUpdate kTargetUpdate;
	if (!Recv(sizeof(kTargetUpdate), &kTargetUpdate))
		return false;

	CPythonMiniMap & rkpyMiniMap = CPythonMiniMap::Instance();
	rkpyMiniMap.UpdateTarget(kTargetUpdate.lID, kTargetUpdate.lX, kTargetUpdate.lY);

	CPythonBackground & rkpyBG = CPythonBackground::Instance();
	rkpyBG.CreateTargetEffect(kTargetUpdate.lID, kTargetUpdate.lX, kTargetUpdate.lY);

//#ifdef _DEBUG
//	char szBuf[256+1];
//	_snprintf(szBuf, sizeof(szBuf), "íƒ€ê²Ÿì˜ ìœ„ì¹˜ê°€ ê°±ì‹  ë˜ì—ˆìŠµë‹ˆë‹¤ [%d:%d/%d]", kTargetUpdate.lID, kTargetUpdate.lX, kTargetUpdate.lY);
//	CPythonChat::Instance().AppendChat(CHAT_TYPE_NOTICE, szBuf);
//	Tracef(" >> RecvTargetUpdatePacket %d : %d, %d\n", kTargetUpdate.lID, kTargetUpdate.lX, kTargetUpdate.lY);
//#endif

	return true;
}

bool CPythonNetworkStream::RecvTargetDeletePacket()
{
	TPacketGCTargetDelete kTargetDelete;
	if (!Recv(sizeof(kTargetDelete), &kTargetDelete))
		return false;

	CPythonMiniMap & rkpyMiniMap = CPythonMiniMap::Instance();
	rkpyMiniMap.DeleteTarget(kTargetDelete.lID);

	CPythonBackground & rkpyBG = CPythonBackground::Instance();
	rkpyBG.DeleteTargetEffect(kTargetDelete.lID);

//#ifdef _DEBUG
//	Tracef(" >> RecvTargetDeletePacket %d\n", kTargetDelete.lID);
//#endif

	return true;
}

bool CPythonNetworkStream::RecvLoverInfoPacket()
{
	TPacketGCLoverInfo kLoverInfo;
	if (!Recv(sizeof(kLoverInfo), &kLoverInfo))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_LoverInfo", Py_BuildValue("(si)", kLoverInfo.szName, kLoverInfo.byLovePoint));
#ifdef _DEBUG
	Tracef("RECV LOVER INFO : %s, %d\n", kLoverInfo.szName, kLoverInfo.byLovePoint);
#endif
	return true;
}

bool CPythonNetworkStream::RecvLovePointUpdatePacket()
{
	TPacketGCLovePointUpdate kLovePointUpdate;
	if (!Recv(sizeof(kLovePointUpdate), &kLovePointUpdate))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_UpdateLovePoint", Py_BuildValue("(i)", kLovePointUpdate.byLovePoint));
#ifdef _DEBUG
	Tracef("RECV LOVE POINT UPDATE : %d\n", kLovePointUpdate.byLovePoint);
#endif
	return true;
}

bool CPythonNetworkStream::RecvDigMotionPacket()
{
	TPacketGCDigMotion kDigMotion;
	if (!Recv(sizeof(kDigMotion), &kDigMotion))
		return false;

#ifdef _DEBUG
	Tracef(" Dig Motion [%d/%d]\n", kDigMotion.vid, kDigMotion.count);
#endif

	IAbstractCharacterManager& rkChrMgr=IAbstractCharacterManager::GetSingleton();
	CInstanceBase * pkInstMain = rkChrMgr.GetInstancePtr(kDigMotion.vid);
	CInstanceBase * pkInstTarget = rkChrMgr.GetInstancePtr(kDigMotion.target_vid);
	if (NULL == pkInstMain)
		return true;

	if (pkInstTarget)
		pkInstMain->NEW_LookAtDestInstance(*pkInstTarget);

	for (int i = 0; i < kDigMotion.count; ++i)
		pkInstMain->PushOnceMotion(CRaceMotionData::NAME_DIG);

	return true;
}

bool CPythonNetworkStream::SendDragonSoulRefinePacket(BYTE bRefineType, TItemPos* pos)
{
	TPacketCGDragonSoulRefine pk;
	pk.header = HEADER_CG_DRAGON_SOUL_REFINE;
	pk.bSubType = bRefineType;
	memcpy (pk.ItemGrid, pos, sizeof (TItemPos) * DS_REFINE_WINDOW_MAX_NUM);
	if (!Send(sizeof (pk), &pk))
	{
		return false;
	}
	return true;
}

#ifdef ENABLE_SKILLBOOK_COMB_SYSTEM
bool CPythonNetworkStream::SendSkillBookCombinationPacket(TItemPos* pPos, BYTE bAction)
{
	TPacketCGSkillBookCombination SkillBookCombCGPacket;
	SkillBookCombCGPacket.bHeader = HEADER_CG_SKILLBOOK_COMB;
	SkillBookCombCGPacket.bAction = bAction;
	memcpy(SkillBookCombCGPacket.CombItemGrid, pPos, sizeof(TItemPos) * SKILLBOOK_COMB_SLOT_MAX);

	if (!Send(sizeof(SkillBookCombCGPacket), &SkillBookCombCGPacket))
		return false;

	return true;
}
#endif

#ifdef ENABLE_SASH_SYSTEM
bool CPythonNetworkStream::RecvSashPacket(bool bReturn)
{
	TPacketSash sPacket;
	if (!Recv(sizeof(sPacket), &sPacket))
		return bReturn;
	
	bReturn = true;
	switch (sPacket.subheader)
	{
		case SASH_SUBHEADER_GC_OPEN:
			{
				CPythonSash::Instance().Clear();
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActSash", Py_BuildValue("(ib)", 1, sPacket.bWindow));
			}
			break;
		case SASH_SUBHEADER_GC_CLOSE:
			{
				CPythonSash::Instance().Clear();
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActSash", Py_BuildValue("(ib)", 2, sPacket.bWindow));
			}
			break;
		case SASH_SUBHEADER_GC_ADDED:
			{
				CPythonSash::Instance().AddMaterial(sPacket.dwPrice, sPacket.bPos, sPacket.tPos);
				if (sPacket.bPos == 1)
				{
					CPythonSash::Instance().AddResult(sPacket.dwItemVnum, sPacket.dwMinAbs, sPacket.dwMaxAbs);
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AlertSash", Py_BuildValue("(b)", sPacket.bWindow));
				}
				
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActSash", Py_BuildValue("(ib)", 3, sPacket.bWindow));
			}
			break;
		case SASH_SUBHEADER_GC_REMOVED:
			{
				CPythonSash::Instance().RemoveMaterial(sPacket.dwPrice, sPacket.bPos);
				if (sPacket.bPos == 0)
					CPythonSash::Instance().RemoveMaterial(sPacket.dwPrice, 1);
				
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActSash", Py_BuildValue("(ib)", 4, sPacket.bWindow));
			}
			break;
		case SASH_SUBHEADER_CG_REFINED:
			{
				if (sPacket.dwMaxAbs == 0)
					CPythonSash::Instance().RemoveMaterial(sPacket.dwPrice, 1);
				else
				{
					CPythonSash::Instance().RemoveMaterial(sPacket.dwPrice, 0);
					CPythonSash::Instance().RemoveMaterial(sPacket.dwPrice, 1);
				}
				
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActSash", Py_BuildValue("(ib)", 4, sPacket.bWindow));
			}
			break;
		default:
			TraceError("CPythonNetworkStream::RecvSashPacket: unknown subheader %d\n.", sPacket.subheader);
			break;
	}
	
	return bReturn;
}

bool CPythonNetworkStream::SendSashClosePacket()
{
	if (!__CanActMainInstance())
		return true;
	
	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;
	
	TPacketSash sPacket;
	sPacket.header = HEADER_CG_SASH;
	sPacket.subheader = SASH_SUBHEADER_CG_CLOSE;
	sPacket.dwPrice = 0;
	sPacket.bPos = 0;
	sPacket.tPos = tPos;
	sPacket.dwItemVnum = 0;
	sPacket.dwMinAbs = 0;
	sPacket.dwMaxAbs = 0;
	if (!Send(sizeof(sPacket), &sPacket))
		return false;
	
	return SendSequence();
}

bool CPythonNetworkStream::SendSashAddPacket(TItemPos tPos, BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;
	
	TPacketSash sPacket;
	sPacket.header = HEADER_CG_SASH;
	sPacket.subheader = SASH_SUBHEADER_CG_ADD;
	sPacket.dwPrice = 0;
	sPacket.bPos = bPos;
	sPacket.tPos = tPos;
	sPacket.dwItemVnum = 0;
	sPacket.dwMinAbs = 0;
	sPacket.dwMaxAbs = 0;
	if (!Send(sizeof(sPacket), &sPacket))
		return false;
	
	return SendSequence();
}

bool CPythonNetworkStream::SendSashRemovePacket(BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;
	
	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;
	
	TPacketSash sPacket;
	sPacket.header = HEADER_CG_SASH;
	sPacket.subheader = SASH_SUBHEADER_CG_REMOVE;
	sPacket.dwPrice = 0;
	sPacket.bPos = bPos;
	sPacket.tPos = tPos;
	sPacket.dwItemVnum = 0;
	sPacket.dwMinAbs = 0;
	sPacket.dwMaxAbs = 0;
	if (!Send(sizeof(sPacket), &sPacket))
		return false;
	
	return SendSequence();
}

bool CPythonNetworkStream::SendSashRefinePacket()
{
	if (!__CanActMainInstance())
		return true;
	
	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;
	
	TPacketSash sPacket;
	sPacket.header = HEADER_CG_SASH;
	sPacket.subheader = SASH_SUBHEADER_CG_REFINE;
	sPacket.dwPrice = 0;
	sPacket.bPos = 0;
	sPacket.tPos = tPos;
	sPacket.dwItemVnum = 0;
	sPacket.dwMinAbs = 0;
	sPacket.dwMaxAbs = 0;
	if (!Send(sizeof(sPacket), &sPacket))
		return false;
	
	return SendSequence();
}
#endif

#ifdef ENABLE_CHANGELOOK_SYSTEM
bool CPythonNetworkStream::RecvChangeLookPacket()
{
	TPacketChangeLook sPacket;
	if (!Recv(sizeof(sPacket), &sPacket))
		return false;
	
	switch (sPacket.subheader)
	{
		case CL_SUBHEADER_OPEN:
			{
				CPythonChangeLook::Instance().Clear();
				CPythonChangeLook::Instance().SetCost(sPacket.dwCost);
				#ifdef ENABLE_MOUNT_CHANGELOOK_SYSTEM
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActChangeLook", Py_BuildValue("(ii)", 1, sPacket.bMount));
				#else
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActChangeLook", Py_BuildValue("(i)", 1));
				#endif
			}
			break;
		case CL_SUBHEADER_CLOSE:
			{
				CPythonChangeLook::Instance().Clear();
				#ifdef ENABLE_MOUNT_CHANGELOOK_SYSTEM
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActChangeLook", Py_BuildValue("(ii)", 2, 0));
				#else
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActChangeLook", Py_BuildValue("(i)", 2));
				#endif
			}
			break;
		case CL_SUBHEADER_ADD:
			{
				CPythonChangeLook::Instance().AddMaterial(sPacket.bPos, sPacket.tPos);
				#ifdef ENABLE_MOUNT_CHANGELOOK_SYSTEM
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActChangeLook", Py_BuildValue("(ii)", 3, 0));
				#else
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActChangeLook", Py_BuildValue("(i)", 3));
				#endif
				if (sPacket.bPos == 1)
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AlertChangeLook", Py_BuildValue("()"));
			}
			break;
		case CL_SUBHEADER_REMOVE:
			{
				if (sPacket.bPos == 1 || sPacket.bPos == 2)
					CPythonChangeLook::Instance().RemoveMaterial(sPacket.bPos);
				else
					CPythonChangeLook::Instance().RemoveAllMaterials();

				#ifdef ENABLE_MOUNT_CHANGELOOK_SYSTEM
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActChangeLook", Py_BuildValue("(ii)", 4, 0));
				#else
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActChangeLook", Py_BuildValue("(i)", 4));
				#endif
			}
			break;
		case CL_SUBHEADER_REFINE:
			{
				CPythonChangeLook::Instance().RemoveAllMaterials();
				#ifdef ENABLE_MOUNT_CHANGELOOK_SYSTEM
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActChangeLook", Py_BuildValue("(ii)", 4, 0));
				#else
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActChangeLook", Py_BuildValue("(i)", 4));
				#endif
			}
			break;
		default:
			TraceError("CPythonNetworkStream::RecvChangeLookPacket: unknown subheader %d\n.", sPacket.subheader);
			break;
	}
	
	return true;
}

bool CPythonNetworkStream::SendClClosePacket()
{
	if (!__CanActMainInstance())
		return true;
	
	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;
	
	TPacketChangeLook sPacket;
	sPacket.header = HEADER_CG_CL;
	sPacket.subheader = CL_SUBHEADER_CLOSE;
	sPacket.dwCost = 0;
	sPacket.bPos = 0;
	sPacket.tPos = tPos;
	if (!Send(sizeof(sPacket), &sPacket))
		return false;
	
	return SendSequence();
}

bool CPythonNetworkStream::SendClAddPacket(TItemPos tPos, BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;
	
	TPacketChangeLook sPacket;
	sPacket.header = HEADER_CG_CL;
	sPacket.subheader = CL_SUBHEADER_ADD;
	sPacket.dwCost = 0;
	sPacket.bPos = bPos;
	sPacket.tPos = tPos;
	if (!Send(sizeof(sPacket), &sPacket))
		return false;
	
	return SendSequence();
}

bool CPythonNetworkStream::SendClRemovePacket(BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;
	
	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;
	
	TPacketChangeLook sPacket;
	sPacket.header = HEADER_CG_CL;
	sPacket.subheader = CL_SUBHEADER_REMOVE;
	sPacket.dwCost = 0;
	sPacket.bPos = bPos;
	sPacket.tPos = tPos;
	if (!Send(sizeof(sPacket), &sPacket))
		return false;
	
	return SendSequence();
}

bool CPythonNetworkStream::SendClRefinePacket()
{
	if (!__CanActMainInstance())
		return true;
	
	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;
	
	TPacketChangeLook sPacket;
	sPacket.header = HEADER_CG_CL;
	sPacket.subheader = CL_SUBHEADER_REFINE;
	sPacket.dwCost = 0;
	sPacket.bPos = 0;
	sPacket.tPos = tPos;
	if (!Send(sizeof(sPacket), &sPacket))
		return false;
	
	return SendSequence();
}
#endif

#ifdef ENABLE_SELECT_REWARD_BOX
bool CPythonNetworkStream::SendSelectRewardInfo(WORD wInventoryCell)
{
	TPacketCGSelectRewardInfo packet;
	packet.header = HEADER_CG_REWARD_INFO;
	packet.wInventoryCell = wInventoryCell;
	if (!Send(sizeof(packet), &packet))
		return false;
	return SendSequence();
}

bool CPythonNetworkStream::RecvSelectRewardInfo()
{
	TPacketGCSelectRewardInfo packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	packet.wSize -= sizeof(packet);
	while (packet.wSize > 0)
	{
		TChestSelectRewardTable kTab;
		if (!Recv(sizeof(kTab), &kTab))
			return false;

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AddSelectRewardInfo", Py_BuildValue("(iiiii)", packet.dwBoxVnum, kTab.bPageIndex, kTab.bSlotIndex, kTab.dwItemVnum, kTab.bItemCount));

		packet.wSize -= sizeof(kTab);
	}
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_RefreshSelectRewardInfo", Py_BuildValue("(i)", packet.dwBoxVnum));

	return true;
}

bool CPythonNetworkStream::SendSelectRewardSelectPacket(int iInventoryPos, DWORD dwItemVnum, BYTE bItemCount)
{
	if (!__CanActMainInstance())
		return false;
	TPacketCGSelectRewardSelect packet;
	packet.header = HEADER_CG_REWARD_SELECT;
	packet.wInventoryCell = iInventoryPos;
	packet.dwSelectedItemVnum = dwItemVnum;
	packet.bSelectedItemCount = bItemCount;
	if (!Send(sizeof(packet), &packet))
		return false;
	return SendSequence();
}
#endif

#ifdef ENABLE_SHOW_CHEST_DROP
bool CPythonNetworkStream::SendChestDropInfo(WORD wInventoryCell)
{
	TPacketCGChestDropInfo packet;
	packet.header = HEADER_CG_CHEST_DROP_INFO;
	packet.wInventoryCell = wInventoryCell;
	packet.dwItemVnum = 0;
	
	if (!Send(sizeof(packet), &packet))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendChestDropInfoByVnum(DWORD dwItemVnum)
{
	TPacketCGChestDropInfo packet;
	packet.header = HEADER_CG_CHEST_DROP_INFO;
	packet.wInventoryCell = 0xFFFF;
	packet.dwItemVnum = dwItemVnum;
	
	if (!Send(sizeof(packet), &packet))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvChestDropInfo()
{
	TPacketGCChestDropInfo packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	packet.wSize -= sizeof(packet);
	while (packet.wSize > 0)
	{
		TChestDropInfoTable kTab;
		if (!Recv(sizeof(kTab), &kTab))
			return false;

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AddChestDropInfo", Py_BuildValue("(iiiii)", packet.dwChestVnum, kTab.bPageIndex, kTab.bSlotIndex, kTab.dwItemVnum, kTab.bItemCount));

		packet.wSize -= sizeof(kTab);
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_RefreshChestDropInfo", Py_BuildValue("(i)", packet.dwChestVnum));

	return true;
}
#endif

#ifdef ENABLE_MINI_GAME_CATCH_KING
bool CPythonNetworkStream::SendMiniGameCatchKing(BYTE bSubHeader, BYTE bSubArgument)
{
	if (!__CanActMainInstance())
		return true;
	
	TPacketCGMiniGameCatchKing packet;
	packet.bHeader = HEADER_CG_MINI_GAME_CATCH_KING;
	packet.bSubheader = bSubHeader;
	packet.bSubArgument = bSubArgument;

	if (!Send(sizeof(TPacketCGMiniGameCatchKing), &packet))
	{
		Tracef("SendMiniGameCatchKing Send Packet Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvMiniGameCatchKingPacket()
{
	std::vector<char> vecBuffer;
	vecBuffer.clear();

	TPacketGCMiniGameCatchKing packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	int iSize = packet.wSize - sizeof(packet);
	if (iSize > 0)
	{
		vecBuffer.resize(iSize);
		if (!Recv(iSize, &vecBuffer[0]))
			return false;
	}

	switch (packet.bSubheader)
	{
		case SUBHEADER_GC_CATCH_KING_EVENT_INFO:
			{
				bool bIsEnable = *(bool *)&vecBuffer[0];
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameCatchKingEvent", Py_BuildValue("(b)", bIsEnable));
			}
			break;
			
		case SUBHEADER_GC_CATCH_KING_START:
			{
				DWORD dwBigScore = *(DWORD *)&vecBuffer[0];
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameCatchKingEventStart", Py_BuildValue("(i)", dwBigScore));
			}
			break;
			
		case SUBHEADER_GC_CATCH_KING_SET_CARD:
			{
				BYTE bCardNumber = *(BYTE *)&vecBuffer[0];
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameCatchKingSetHandCard", Py_BuildValue("(i)", bCardNumber));
			}
			break;
			
		case SUBHEADER_GC_CATCH_KING_RESULT_FIELD:
			{
				TPacketGCMiniGameCatchKingResult * packSecond = (TPacketGCMiniGameCatchKingResult *)&vecBuffer[0];
				
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameCatchKingResultField", Py_BuildValue("(iiiibbbb)", 
					packSecond->dwPoints, packSecond->bRowType, packSecond->bCardPos, packSecond->bCardValue, 
					packSecond->bKeepFieldCard, packSecond->bDestroyHandCard, packSecond->bGetReward, packSecond->bIsFiveNearBy));
			}
			break;
			
		case SUBHEADER_GC_CATCH_KING_SET_END_CARD:
			{
				TPacketGCMiniGameCatchKingSetEndCard * packSecond = (TPacketGCMiniGameCatchKingSetEndCard *)&vecBuffer[0];
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameCatchKingSetEndCard", Py_BuildValue("(ii)", packSecond->bCardPos, packSecond->bCardValue));
			}
			break;
			
		case SUBHEADER_GC_CATCH_KING_REWARD:
			{
				BYTE bReturnCode = *(BYTE *)&vecBuffer[0];
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameCatchKingReward", Py_BuildValue("(i)", bReturnCode));
			}
			break;
			
		default:
			TraceError("CPythonNetworkStream::RecvMiniGameCatchKingPacket: Unknown subheader\n");
			break;
	}

	return true;
}
#endif

#ifdef ENABLE_AURA_SYSTEM
bool CPythonNetworkStream::RecvAuraPacket(bool bReturn)
{
	TPacketAura sPacket;
	if (!Recv(sizeof(sPacket), &sPacket))
		return bReturn;
	
	bReturn = true;
	switch (sPacket.subheader)
	{
		case AURA_SUBHEADER_GC_OPEN:
			{
				CPythonAura::Instance().Clear();
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActAura", Py_BuildValue("(ii)", 1, (int)sPacket.bWindow));
			}
			break;
		case AURA_SUBHEADER_GC_CLOSE:
			{
				CPythonAura::Instance().Clear();
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActAura", Py_BuildValue("(ii)", 2, (int)sPacket.bWindow));
			}
			break;
		case AURA_SUBHEADER_GC_ADDED:
			{
				CPythonAura::Instance().AddMaterial(sPacket.dwPrice, sPacket.bPos, sPacket.tPos);
				if (sPacket.bPos == 1)
				{
					CPythonAura::Instance().AddResult(sPacket.dwItemVnum, sPacket.dwMinAbs, sPacket.dwMaxAbs);
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AlertAura", Py_BuildValue("(i)", (int)sPacket.bWindow));
				}
				
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActAura", Py_BuildValue("(ii)", 3, (int)sPacket.bWindow));
			}
			break;
		case AURA_SUBHEADER_GC_REMOVED:
			{
				CPythonAura::Instance().RemoveMaterial(sPacket.dwPrice, sPacket.bPos);
				if (sPacket.bPos == 0)
					CPythonAura::Instance().RemoveMaterial(sPacket.dwPrice, 1);
				
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActAura", Py_BuildValue("(ii)", 4, (int)sPacket.bWindow));
			}
			break;
		case AURA_SUBHEADER_GC_REFINED:
			{
				if (sPacket.bPos != 255)
				{
					if (sPacket.dwMaxAbs == 0)
						CPythonAura::Instance().RemoveMaterial(sPacket.dwPrice, 1);
					else
					{
						CPythonAura::Instance().RemoveMaterial(sPacket.dwPrice, 0);
						CPythonAura::Instance().RemoveMaterial(sPacket.dwPrice, 1);
					}
				}
				
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActAura", Py_BuildValue("(ib)", 4, sPacket.bWindow));
			}
			break;
		default:
			TraceError("CPythonNetworkStream::RecvAuraPacket: unknown subheader %d\n.", sPacket.subheader);
			break;
	}
	
	return bReturn;
}

bool CPythonNetworkStream::SendAuraClosePacket()
{
	if (!__CanActMainInstance())
		return true;
	
	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;
	
	TPacketAura sPacket;
	sPacket.header = HEADER_CG_AURA;
	sPacket.subheader = AURA_SUBHEADER_CG_CLOSE;
	sPacket.dwPrice = 0;
	sPacket.bPos = 0;
	sPacket.tPos = tPos;
	sPacket.dwItemVnum = 0;
	sPacket.dwMinAbs = 0;
	sPacket.dwMaxAbs = 0;
	if (!Send(sizeof(sPacket), &sPacket))
		return false;
	
	return SendSequence();
}

bool CPythonNetworkStream::SendAuraAddPacket(TItemPos tPos, BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;
	
	TPacketAura sPacket;
	sPacket.header = HEADER_CG_AURA;
	sPacket.subheader = AURA_SUBHEADER_CG_ADD;
	sPacket.dwPrice = 0;
	sPacket.bPos = bPos;
	sPacket.tPos = tPos;
	sPacket.dwItemVnum = 0;
	sPacket.dwMinAbs = 0;
	sPacket.dwMaxAbs = 0;
	if (!Send(sizeof(sPacket), &sPacket))
		return false;
	
	return SendSequence();
}

bool CPythonNetworkStream::SendAuraRemovePacket(BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;
	
	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;
	
	TPacketAura sPacket;
	sPacket.header = HEADER_CG_AURA;
	sPacket.subheader = AURA_SUBHEADER_CG_REMOVE;
	sPacket.dwPrice = 0;
	sPacket.bPos = bPos;
	sPacket.tPos = tPos;
	sPacket.dwItemVnum = 0;
	sPacket.dwMinAbs = 0;
	sPacket.dwMaxAbs = 0;
	if (!Send(sizeof(sPacket), &sPacket))
		return false;
	
	return SendSequence();
}

bool CPythonNetworkStream::SendAuraRefinePacket()
{
	if (!__CanActMainInstance())
		return true;
	
	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;
	
	TPacketAura sPacket;
	sPacket.header = HEADER_CG_AURA;
	sPacket.subheader = AURA_SUBHEADER_CG_REFINE;
	sPacket.dwPrice = 0;
	sPacket.bPos = 0;
	sPacket.tPos = tPos;
	sPacket.dwItemVnum = 0;
	sPacket.dwMinAbs = 0;
	sPacket.dwMaxAbs = 0;
	if (!Send(sizeof(sPacket), &sPacket))
		return false;
	
	return SendSequence();
}
#endif

#ifdef ENABLE_CUBE_RENEWAL
bool CPythonNetworkStream::CubeRenewalMakeItem(int index_item, int count_item, int index_item_improve)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGCubeRenewalSend	packet;

	packet.header		= HEADER_CG_CUBE_RENEWAL;
	packet.subheader	= CUBE_RENEWAL_SUB_HEADER_MAKE_ITEM;
	packet.index_item 	= index_item;
	packet.count_item   = count_item;
	packet.index_item_improve	= index_item_improve;

	if (!Send(sizeof(TPacketCGCubeRenewalSend), &packet))
	{
		Tracef("CPythonNetworkStream::CubeRenewalMakeItem Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::CubeRenewalClose()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGCubeRenewalSend	packet;

	packet.header		= HEADER_CG_CUBE_RENEWAL;
	packet.subheader	= CUBE_RENEWAL_SUB_HEADER_CLOSE;

	if (!Send(sizeof(TPacketCGCubeRenewalSend), &packet))
	{
		Tracef("CPythonNetworkStream::CubeRenewalClose Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvCubeRenewalPacket()
{

	TPacketGCCubeRenewalReceive CubeRenewalPacket;

	if (!Recv(sizeof(CubeRenewalPacket), &CubeRenewalPacket))
		return false;

	switch (CubeRenewalPacket.subheader)
	{

		case CUBE_RENEWAL_SUB_HEADER_OPEN_RECEIVE:
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_CUBE_RENEWAL_OPEN", Py_BuildValue("()"));
		}
		break;

		case CUBE_RENEWAL_SUB_HEADER_DATES_RECEIVE:
		{
			CPythonCubeRenewal::Instance().ReceiveList(CubeRenewalPacket.date_cube_renewal);
		}
		break;

		case CUBE_RENEWAL_SUB_HEADER_DATES_LOADING:
		{
			CPythonCubeRenewal::Instance().LoadingList();
		}
		break;

		case CUBE_RENEWAL_SUB_HEADER_CLEAR_DATES_RECEIVE:
		{
			CPythonCubeRenewal::Instance().ClearList();
		}
		break;
	}

	return true;
}
#endif

#ifdef ENABLE_SWITCHBOT
bool CPythonNetworkStream::RecvSwitchbotPacket()
{
	TPacketGCSwitchbot pack;
	if (!Recv(sizeof(pack), &pack))
		return false;

	size_t packet_size = int(pack.size) - sizeof(TPacketGCSwitchbot);
	if (pack.subheader == SUBHEADER_GC_SWITCHBOT_UPDATE)
	{
		if (packet_size != sizeof(CPythonSwitchbot::TSwitchbotTable))
			return false;

		CPythonSwitchbot::TSwitchbotTable table;
		if (!Recv(sizeof(table), &table))
			return false;

		CPythonSwitchbot::Instance().Update(table);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSwitchbotWindow", Py_BuildValue("()"));
	}
	else if (pack.subheader == SUBHEADER_GC_SWITCHBOT_UPDATE_ITEM)
	{
		if (packet_size != sizeof(TSwitchbotUpdateItem))
			return false;

		TSwitchbotUpdateItem update;
		if (!Recv(sizeof(update), &update))
			return false;

		TItemPos pos(SWITCHBOT, update.slot);

		IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();
		rkPlayer.SetItemCount(pos, update.count);

		for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
		{
			rkPlayer.SetItemMetinSocket(pos, i, update.alSockets[i]);
		}

		for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
		{
			rkPlayer.SetItemAttribute(pos, j, update.aAttr[j].bType, update.aAttr[j].sValue);
		}

#ifdef ENABLE_GLOVE_SYSTEM
		for (int j = 0; j < ITEM_APPLY_RANDOM_SLOT_MAX_NUM; ++j)
		{
			rkPlayer.SetItemApplyRandom(pos, j, update.aApplyRandom[j].bType, update.aApplyRandom[j].sValue);
		}
#endif

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSwitchbotItem", Py_BuildValue("(i)", update.slot));
		return true;
	}
	else if (pack.subheader == SUBHEADER_GC_SWITCHBOT_SEND_ATTRIBUTE_INFORMATION)
	{
		CPythonSwitchbot::Instance().ClearAttributeMap();

		size_t table_size = sizeof(CPythonSwitchbot::TSwitchbottAttributeTable);
		while (packet_size >= table_size)
		{
			CPythonSwitchbot::TSwitchbottAttributeTable table;
			if (!Recv(table_size, &table))
				return false;


			CPythonSwitchbot::Instance().AddAttributeToMap(table);
			packet_size -= table_size;
		}
	}

	return true;
}

bool CPythonNetworkStream::SendSwitchbotStartPacket(BYTE slot, std::vector<CPythonSwitchbot::TSwitchbotAttributeAlternativeTable> alternatives)
{
	TPacketCGSwitchbot pack;
	pack.header = HEADER_CG_SWITCHBOT;
	pack.subheader = SUBHEADER_CG_SWITCHBOT_START;
	pack.size = sizeof(TPacketCGSwitchbot) + sizeof(CPythonSwitchbot::TSwitchbotAttributeAlternativeTable) * SWITCHBOT_ALTERNATIVE_COUNT;
	pack.slot = slot;

	if (!Send(sizeof(pack), &pack))
		return false;

	for (const auto& it : alternatives)
		if (!Send(sizeof(it), &it))
			return false;

	return true;
}

bool CPythonNetworkStream::SendSwitchbotStopPacket(BYTE slot)
{
	TPacketCGSwitchbot pack;
	pack.header = HEADER_CG_SWITCHBOT;
	pack.subheader = SUBHEADER_CG_SWITCHBOT_STOP;
	pack.size = sizeof(TPacketCGSwitchbot);
	pack.slot = slot;

	if (!Send(sizeof(pack), &pack))
		return false;

	return true;
}
#endif

#ifdef ENABLE_WON_EXCHANGE_WINDOW
bool CPythonNetworkStream::SendWonExchangeSellPacket(WORD wValue)
{
	TPacketCGWonExchange kWonExchangePacket;
	kWonExchangePacket.bSubHeader = WON_EXCHANGE_CG_SUBHEADER_SELL;
	kWonExchangePacket.wValue = wValue;

	if (!Send(sizeof(TPacketCGWonExchange), &kWonExchangePacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendWonExchangeBuyPacket(WORD wValue)
{
	TPacketCGWonExchange kWonExchangePacket;
	kWonExchangePacket.bSubHeader = WON_EXCHANGE_CG_SUBHEADER_BUY;
	kWonExchangePacket.wValue = wValue;

	if (!Send(sizeof(TPacketCGWonExchange), &kWonExchangePacket))
		return false;

	return SendSequence();
}
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
bool CPythonNetworkStream::SendChangeLanguagePacket(BYTE bLanguage)
{
	TPacketChangeLanguage packet;
	packet.bHeader = HEADER_CG_CHANGE_LANGUAGE;
	packet.bLanguage = bLanguage;

	if (!Send(sizeof(packet), &packet))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvRequestChangeLanguage()
{
	TPacketChangeLanguage packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	CInstanceBase* pkInstMain = rkChrMgr.GetMainInstancePtr();

	if (!pkInstMain)
	{
		TraceError("CPythonNetworkStream::RecvRequestChangeLanguage - MainCharacter is NULL");
		return false;
	}

	pkInstMain->SetLanguage(packet.bLanguage);
	pkInstMain->Update();

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_RequestChangeLanguage", Py_BuildValue("(i)", packet.bLanguage));

	return true;
}
#endif

#ifdef ENABLE_MULTI_LANGUAGE_WHISPER_DETAILS
bool CPythonNetworkStream::SendWhisperDetails(const char* name)
{
	TPacketCGWhisperDetails packet{};
	packet.header = HEADER_CG_WHISPER_DETAILS;
	strncpy(packet.name, name, sizeof(packet.name) - 1);
	if (!Send(sizeof(packet), &packet))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvWhisperDetails()
{
	TPacketGCWhisperDetails packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_RecieveWhisperDetails", Py_BuildValue("(si)", packet.name, packet.bLanguage));
#else
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_RecieveWhisperDetails", Py_BuildValue("(s)", packet.name));
#endif

	return true;
}
#endif

#ifdef ENABLE_EVENT_SYSTEM
bool CPythonNetworkStream::RecvEventInfo()
{
	TPacketGCEventInfo	infoPacket;
	if (!Recv(sizeof(infoPacket), &infoPacket))
		return false;
	CPythonGameEvents::instance().SetActivateEvent(infoPacket.isActivate, infoPacket.event_id);
	CPythonGameEvents::instance().SetEventTime(infoPacket.event_id, infoPacket.event_time);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnRecvEventInformation", Py_BuildValue("()"));
	return true;
}
#endif

#ifdef ENABLE_REFINE_ELEMENT
bool CPythonNetworkStream::RecvRefineElementPacket()
{
	TPacketGCRefineElement packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], 
		"BINARY_RefineElementProcess", 
		Py_BuildValue("(iii)", 
		packet.bType, packet.wSrcCell, packet.wDstCell));
	
	return true;
}

bool CPythonNetworkStream::SendRefineElementPacket(BYTE bArg)
{
	if (!__CanActMainInstance())
		return true;
	
	TPacketCGRefineElement packet;
	packet.bHeader = HEADER_CG_REFINE_ELEMENT;
	packet.bArg = bArg;

	if (!Send(sizeof(TPacketCGRefineElement), &packet))
	{
		TraceError("SendRefineElementPacket Error - Arg %d", bArg);
		return false;
	}

	return true;
}
#endif

#ifdef ENABLE_KILL_STATISTICS
bool CPythonNetworkStream::RecvKillStatistics()
{
	TPacketGCKillStatistics KStatisticsGC;

	if (!Recv(sizeof(TPacketGCKillStatistics), &KStatisticsGC))
	{
		Tracen("Recv KStatisticsGC Packet Error");
		return false;
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ReceiveKillStatisticsPacket", Py_BuildValue("(iiiiiiiii)", 
		KStatisticsGC.iJinnoKills, KStatisticsGC.iShinsooKills, KStatisticsGC.iChunjoKills, KStatisticsGC.iTotalKills, KStatisticsGC.iTotalDeaths,
		KStatisticsGC.iDuelsWon, KStatisticsGC.iDuelsLost, KStatisticsGC.iBossesKills, KStatisticsGC.iStonesKills));
	return true;
}
#endif

#ifdef ENABLE_OFFLINESHOP_SYSTEM
bool CPythonNetworkStream::SendBuildOfflineShopPacket(const char* c_szName, const std::vector<TOfflineShopItemTable>& c_rSellingItemStock, DWORD shopVnum, BYTE shopTitle)
{
	TPacketCGMyOfflineShop packet;
	packet.bHeader = HEADER_CG_MY_OFFLINE_SHOP;
	strncpy(packet.szSign, c_szName, SHOP_SIGN_MAX_LEN);
	packet.bCount = c_rSellingItemStock.size();
	packet.shopVnum = shopVnum;
	packet.shopTitle = shopTitle;
	if (!Send(sizeof(packet), &packet))
		return false;

	for (std::vector<TOfflineShopItemTable>::const_iterator itor = c_rSellingItemStock.begin(); itor < c_rSellingItemStock.end(); ++itor)
	{
		const TOfflineShopItemTable& c_rItem = *itor;
		if (!Send(sizeof(c_rItem), &c_rItem))
			return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvOfflineShopSignPacket()
{
	TPacketGCShopSign p;
	if (!Recv(sizeof(TPacketGCShopSign), &p))
		return false;
	CInstanceBase* pInstance = CPythonCharacterManager::instance().GetInstancePtr(p.dwVID);
	if (pInstance)
	{
		if (pInstance->IsShop())
		{
			if (0 == strlen(p.szSign))
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OfflineShop_Disappear", Py_BuildValue("(i)", p.dwVID));
			else
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OfflineShop_Appear", Py_BuildValue("(is)", p.dwVID, p.szSign));
		}
	}
	return true;
}

bool CPythonNetworkStream::RecvOfflineShopPacket()
{
	TPacketGCShop  packet_shop;
	if (!Recv(sizeof(packet_shop), &packet_shop))
		return false;
	std::vector<char> vecBuffer;
	vecBuffer.clear();
	int iSize = packet_shop.size - sizeof(packet_shop);

	switch (packet_shop.subheader)
	{
	case SHOP_SUBHEADER_GC_REFRESH_MONEY:
	case SHOP_SUBHEADER_GC_REFRESH_DISPLAY_COUNT:
	case SHOP_SUBHEADER_GC_CHECK_RESULT:
	case SHOP_SUBHEADER_GC_REFRESH_COUNT:
	case SHOP_SUBHEADER_GC_REALWATCHER_COUNT:
	case SHOP_SUBHEADER_GC_CHANGE_TITLE:
	case SHOP_SUBHEADER_GC_REFRESH_MONEY_CHEQUE:
		break;
	default:
		if (iSize > 0)
		{
			vecBuffer.resize(iSize);
			if (!Recv(iSize, &vecBuffer[0]))
				return false;
		}
	}

	switch (packet_shop.subheader)
	{
	case SHOP_SUBHEADER_GC_START:
	{
		CPythonShop::Instance().Clear();
		TPacketGCOfflineShopStart* pShopStartPacket = (TPacketGCOfflineShopStart*)vecBuffer.data();
		for (BYTE iItemIndex = 0; iItemIndex < OFFLINE_SHOP_HOST_ITEM_MAX_NUM; ++iItemIndex)
			CPythonShop::Instance().SetOfflineShopItemData(iItemIndex, pShopStartPacket->items[iItemIndex]);
		CPythonShop::Instance().SetShopDisplayedCount(pShopStartPacket->m_dwDisplayedCount);
		CPythonShop::Instance().SetRealWatcherCount(pShopStartPacket->m_dwRealWatcherCount);
		CPythonShop::Instance().SetCurrentOfflineShopMoney(pShopStartPacket->price);
		CPythonShop::Instance().SetCurrentOfflineShopMoneyCheque(pShopStartPacket->price_cheque);
		CPythonShop::Instance().SetShopSign(pShopStartPacket->title);
		CPythonShop::Instance().SetShopFlag(pShopStartPacket->flag);
		CPythonShop::Instance().SetShopType(pShopStartPacket->type);
		CPythonShop::Instance().SetShopTime(pShopStartPacket->time);

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartOfflineShop", Py_BuildValue("(ii)", pShopStartPacket->owner_vid, pShopStartPacket->IsOwner == true ? 1 : 0));
	}
	break;

	case SHOP_SUBHEADER_GC_END:
	{
		CPythonShop::instance().Clear();
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "EndOfflineShop", Py_BuildValue("()"));
	}
	break;

	case SHOP_SUBHEADER_GC_UPDATE_ITEM:
	{
		TPacketGCOfflineShopUpdateItem* pShopUpdateItemPacket = (TPacketGCOfflineShopUpdateItem*)vecBuffer.data();
		CPythonShop::Instance().SetShopDisplayedCount(pShopUpdateItemPacket->m_dwDisplayedCount);
		CPythonShop::Instance().SetRealWatcherCount(pShopUpdateItemPacket->m_dwRealWatcherCount);
		CPythonShop::Instance().SetShopFlag(pShopUpdateItemPacket->flag);
		CPythonShop::Instance().SetShopTime(pShopUpdateItemPacket->time);
		CPythonShop::Instance().SetShopSign(pShopUpdateItemPacket->title);
		CPythonShop::Instance().SetShopType(pShopUpdateItemPacket->type);
		CPythonShop::Instance().SetOfflineShopItemData(pShopUpdateItemPacket->pos, pShopUpdateItemPacket->item);
		CPythonShop::Instance().SetCurrentOfflineShopMoney(pShopUpdateItemPacket->price);
		CPythonShop::Instance().SetCurrentOfflineShopMoneyCheque(pShopUpdateItemPacket->price_cheque);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshOfflineShop", Py_BuildValue("()"));
	}
	break;

	case SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_MONEY"));
		break;

	case SHOP_SUBHEADER_GC_NOT_ENOUGH_CHEQUE:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_CHEQUE"));
		break;

	case SHOP_SUBHEADER_GC_SOLD_OUT:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "SOLDOUT"));
		break;

	case SHOP_SUBHEADER_GC_OK:
		//PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "SOLDOUT"));
		break;

	case SHOP_SUBHEADER_GC_INVENTORY_FULL:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "INVENTORY_FULL"));
		break;

	case SHOP_SUBHEADER_GC_INVALID_POS:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "INVALID_POS"));
		break;

	case SHOP_SUBHEADER_GC_REFRESH_MONEY:
	{
		long long llMoney = 0;
		if (!Recv(sizeof(long long), &llMoney))
			return false;
		CPythonShop::Instance().SetCurrentOfflineShopMoney(llMoney);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshOfflineShop", Py_BuildValue("()"));
	}
	break;

#ifdef ENABLE_CHEQUE_SYSTEM
	case SHOP_SUBHEADER_GC_REFRESH_MONEY_CHEQUE:
	{
		DWORD dwCheque = 0;
		if (!Recv(sizeof(int), &dwCheque))
			return false;
		CPythonShop::Instance().SetCurrentOfflineShopMoneyCheque(dwCheque);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshOfflineShop", Py_BuildValue("()"));
	}
	break;
#endif

	case SHOP_SUBHEADER_GC_CHECK_RESULT:
	{
		BYTE bHasOfflineShop = 0;
		if (!Recv(sizeof(BYTE), &bHasOfflineShop))
			return false;
		CPythonShop::instance().SetHasOfflineShop(bHasOfflineShop == 1 ? true : false);
	}
	break;

	case SHOP_SUBHEADER_GC_REFRESH_DISPLAY_COUNT:
	{
		DWORD displayCount = 0;
		if (!Recv(sizeof(DWORD), &displayCount))
			return false;
		CPythonShop::Instance().SetShopDisplayedCount(displayCount);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshOfflineShop", Py_BuildValue("()"));
	}
	break;
	case SHOP_SUBHEADER_GC_REALWATCHER_COUNT:
	{
		DWORD realWatcher = 0;
		if (!Recv(sizeof(DWORD), &realWatcher))
			return false;
		CPythonShop::Instance().SetRealWatcherCount(realWatcher);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshOfflineShop", Py_BuildValue("()"));
	}
	break;

	case SHOP_SUBHEADER_GC_REFRESH_COUNT:
	{
		DWORD count[2] = { 0,0 };
		if (!Recv(sizeof(count), &count))
			return false;
		CPythonShop::Instance().SetShopDisplayedCount(count[0]);
		CPythonShop::Instance().SetRealWatcherCount(count[1]);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshOfflineShop", Py_BuildValue("()"));
	}
	break;

	case SHOP_SUBHEADER_GC_CHANGE_TITLE:
	{
		char sign[SHOP_SIGN_MAX_LEN + 1];
		if (!Recv(sizeof(sign), &sign))
			return false;
		CPythonShop::Instance().SetShopSign(sign);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshOfflineShop", Py_BuildValue("()"));
	}
	break;

	default:
		TraceError("CPythonNetworkStream::RecvOfflineShopPacket: Unknown subheader %d\n", packet_shop.subheader);
		break;
	}

	return true;
}
#endif

#ifdef ENABLE_SHOP_SEARCH_SYSTEM
bool CPythonNetworkStream::SendPrivateShopSearchInfo(TPacketCGShopSearch* p)
{
	if (!__CanActMainInstance())
		return true;
	if (!Send(sizeof(TPacketCGShopSearch), p))
	{
		return false;
	}

	return true;
}

bool CPythonNetworkStream::RecvShopSearchSet()
{
	TPacketGCShopSearchItemSet packet_item_set;
	if (!Recv(sizeof(TPacketGCShopSearchItemSet), &packet_item_set))
		return false;
	WORD itemCount;
	if (!Recv(sizeof(itemCount), &itemCount))
		return false;
	std::vector<TOfflineShopItemData> m_vec;
	m_vec.resize(itemCount);
	if (!Recv(itemCount * sizeof(TOfflineShopItemData), m_vec.data()))
		return false;
	for (WORD j = 0; j < itemCount; j++)
		CPythonPrivateShopSearch::Instance().AddItemData(m_vec[j]);
	__RefreshShopSearchWindow();
	return true;
}
#endif

#ifdef ENABLE_AUTO_SYSTEM
void CPythonNetworkStream::SendAutoCoolTime(int slotIndex, int iValue)
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetAutoCoolTime", Py_BuildValue("(ii)", slotIndex, iValue));
}
#endif

#ifdef ENABLE_ATTR_6TH_7TH_SYSTEM
bool CPythonNetworkStream::Send67AttrPacket(int iMaterialCount, int iSupportCount, int iSupportPos, int iItemPos)
{
	TPacketCG67Attr p =
	{
		static_cast<BYTE>(HEADER_CG_67_ATTR),
		static_cast<BYTE>(iMaterialCount),
		static_cast<BYTE>(iSupportCount),
		static_cast<short>(iSupportPos),
		static_cast<short>(iItemPos),
	};

	if (!Send(sizeof(p), &p))
	{
		Tracef("Send67AttrPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::Send67AttrClosePacket()
{
	TPacket67AttrOpenClose p;
	p.bHeader = HEADER_CG_CLOSE_67_ATTR;

	if (!Send(sizeof(p), &p))
	{
		Tracef("Send67AttrClosePacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::Recv67AttrOpenPacket()
{
	TPacket67AttrOpenClose p;

	if (!Recv(sizeof(p), &p))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenAttr67AddDlg", Py_BuildValue("()"));

	return true;
}
#endif

#ifdef ENABLE_GIFTBOX_MULTI_OPEN
bool CPythonNetworkStream::RecvRefreshInventoryPacket()
{
	BYTE packet;

	if (!Recv(sizeof(packet), &packet))
		return false;

	__RefreshInventoryWindow();

	return true;
}
#endif

#ifdef ENABLE_RIDING_EXTENDED
bool CPythonNetworkStream::MountUpGrade(const uint8_t iSubHeader, const uint32_t arg)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGMountUpGrade p =
	{
		static_cast<uint8_t>(HEADER_CG_MOUNT_UP_GRADE),
		static_cast<uint8_t>(iSubHeader),
		static_cast<uint32_t>(arg),
	};

	if (!Send(sizeof(p), &p))
	{
		Tracen("CPythonNetworkStream::MountUpGrade Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvMountUpGrade()
{
	TPacketGCMountUpGrade p;
	if (!Recv(sizeof(p), &p))
	{
		Tracen("RecvOpenMountUpGrade Error");
		return false;
	}

	if (!CPythonMountUpGrade::Instance().GetHandler())
	{
		if (m_apoPhaseWnd[PHASE_WINDOW_GAME])
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenMountUpGrade", Py_BuildValue("()"));
	}

	CPythonMountUpGrade::Instance().Process(p.subheader, p.horse_level, p.is_fail, p.existing_exp);

	return true;
}

bool CPythonNetworkStream::RecvMountUpGradeChat()
{
	TPacketGCMountUpGradeChat p;
	if (!Recv(sizeof(p), &p))
	{
		Tracen("RecvMountUpGradeChat Error");
		return false;
	}

	CPythonMountUpGrade::Instance().Chat(p.type, p.value);

	return true;
}
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
bool CPythonNetworkStream::SendBornPet(const char* petname)
{
	TPacketCGBornPet packet;
	packet.byHeader = HEADER_CG_PET_BORN;
	strncpy(packet.petname, petname, 12);
	packet.petname[12] = '\0';

	if (!Send(sizeof(packet), &packet))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendRenewPetName(const char* petname, int itempos1, int itempos2)
{
	TPacketCGRenewPetName packet;
	packet.byHeader = HEADER_CG_PET_RENEW_NAME;
	strncpy(packet.petname, petname, 12);
	packet.petname[12] = '\0';
	packet.itempos1 = itempos1;
	packet.itempos2 = itempos2;

	if (!Send(sizeof(packet), &packet))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvGrowthPetInformation()
{
	TGrowthPetInfoPacket pack;
	if (!Recv(sizeof(pack), &pack))
		return false;

	if (pack.dwSealItemVnum == 0) {
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_GrowthPetUnsummon", Py_BuildValue("()"));
	}
	else { 
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_GrowthPetSetInfo", Py_BuildValue("(iisiiiiiiiiiiiiiiiiiiiiiiiiii)", 
			pack.dwSealItemVnum, pack.dwSealSlot,
			pack.szPetName,
			pack.dwAge, pack.dwBirthday, pack.dwItemDuration, pack.dwBornDuration,
			pack.dwLevel, pack.dwEvo, pack.bCanEvolution,
			pack.dwExpMob, pack.dwExpItem, pack.dwNextExp[0], pack.dwNextExp[1],
			pack.dwBonus[0], pack.dwBonus[1], pack.dwBonus[2],
			pack.dwSkillInfo[0][0], pack.dwSkillInfo[0][1], pack.dwSkillInfo[1][0], pack.dwSkillInfo[1][1], pack.dwSkillInfo[2][0], pack.dwSkillInfo[2][1],
			pack.dwChampLevel, pack.dwChampExp, pack.dwChampNextExp, pack.dwChampRes, pack.dwChampMetin, pack.dwChampHit
		));
	}

	return true;
}
#endif

#ifdef ENABLE_LEFT_SEAT
bool CPythonNetworkStream::SendLeftSeatWaitTimeIndexPacket(BYTE bIndex)
{
	TPacketCGLeftSeat Packet;
	Packet.bHeader = HEADER_CG_LEFT_SEAT;
	Packet.bSubHeader = LEFT_SEAT_SET_WAIT_TIME_INDEX;
	Packet.bIndex = bIndex;

	if (!Send(sizeof(Packet), &Packet))
	{
		Tracef("SendLeftSeatWaitTimeIndexPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendLeftSeatOffPacket(BYTE bIndex)
{
	TPacketCGLeftSeat Packet;
	Packet.bHeader = HEADER_CG_LEFT_SEAT;
	Packet.bSubHeader = LEFT_SEAT_SET_LOGOUT_TIME_INDEX;
	Packet.bIndex = bIndex;

	if (!Send(sizeof(Packet), &Packet))
	{
		Tracef("SendLeftSeatOffPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::DisableLeftSeatLogOutState()
{
	TPacketCGLeftSeat Packet;
	Packet.bHeader = HEADER_CG_LEFT_SEAT;
	Packet.bSubHeader = LEFT_SEAT_DISABLE_LOGOUT_STATE;
	Packet.bIndex = 0;

	if (!Send(sizeof(Packet), &Packet))
	{
		Tracef("DisableLeftSeatLogOutState Error\n");
		return false;
	}

	return SendSequence();
}
#endif

#ifdef ENABLE_BATTLE_ROYALE
bool CPythonNetworkStream::RecvBattleRoyale()
{
	TPacketGCBattleRoyale p;

	if (!Peek(sizeof(p), &p))
		return false;

	if (!Peek(p.size))
		return false;

	Recv(sizeof(p));

	switch (p.subheader)
	{
	case PythonBattleRoyaleManager::BATTLE_ROYALE_GC_INSERT_APPLICANT:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BattleRoyaleProcess", Py_BuildValue("(ii)", PythonBattleRoyaleManager::BATTLE_ROYALE_GC_PROCESS_CATEGORY_MINI_GAME, p.subheader));
		return true;
	case PythonBattleRoyaleManager::BATTLE_ROYALE_GC_REMOVE_APPLICANT:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BattleRoyaleProcess", Py_BuildValue("(ii)", PythonBattleRoyaleManager::BATTLE_ROYALE_GC_PROCESS_CATEGORY_MINI_GAME, p.subheader));
		return true;
	case PythonBattleRoyaleManager::BATTLE_ROYALE_GC_STARTABLE:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BattleRoyaleProcess", Py_BuildValue("(ii)", PythonBattleRoyaleManager::BATTLE_ROYALE_GC_PROCESS_CATEGORY_MINI_GAME, p.subheader));
		return true;
	case PythonBattleRoyaleManager::BATTLE_ROYALE_GC_ENTRY_CLOSED:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BattleRoyaleProcess", Py_BuildValue("(ii)", PythonBattleRoyaleManager::BATTLE_ROYALE_GC_PROCESS_CATEGORY_MINI_GAME, p.subheader));
		return true;
	case PythonBattleRoyaleManager::BATTLE_ROYALE_GC_START:
		PythonBattleRoyaleManager::Instance().SetPlaying(true);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BattleRoyaleProcess", Py_BuildValue("(ii)", PythonBattleRoyaleManager::BATTLE_ROYALE_GC_PROCESS_CATEGORY_MINI_GAME, p.subheader));
		return true;
	case PythonBattleRoyaleManager::BATTLE_ROYALE_GC_EXIT:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BattleRoyaleProcess", Py_BuildValue("(ii)", PythonBattleRoyaleManager::BATTLE_ROYALE_GC_PROCESS_CATEGORY_MINI_GAME, p.subheader));
		PythonBattleRoyaleManager::Instance().BATTLE_ROYALE_Unknown_6();
		return true;
	case PythonBattleRoyaleManager::BATTLE_ROYALE_GC_OPEN_KILL_INFO:
	{
		bool bOpen;
		if (!Recv(sizeof(bOpen), &bOpen))
			return false;

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BattleRoyaleProcess", Py_BuildValue("(ii(b))", PythonBattleRoyaleManager::BATTLE_ROYALE_GC_PROCESS_CATEGORY_BATTLE_ROYALE_INTERFACE, p.subheader, bOpen));
		return true;
	}
	case PythonBattleRoyaleManager::BATTLE_ROYALE_GC_KILL_INFO:
	{
		uint32_t uAliveCount;
		if (!Recv(sizeof(uAliveCount), &uAliveCount))
			return false;

		uint32_t uKillCount;
		if (!Recv(sizeof(uKillCount), &uKillCount))
			return false;

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BattleRoyaleProcess", Py_BuildValue("(ii(ii))", PythonBattleRoyaleManager::BATTLE_ROYALE_GC_PROCESS_CATEGORY_BATTLE_ROYALE_INTERFACE, p.subheader, uAliveCount, uKillCount));
		return true;
	}
	case PythonBattleRoyaleManager::BATTLE_ROYALE_GC_DEAD:
	{
		uint8_t bPersonalRank;
		if (!Recv(sizeof(bPersonalRank), &bPersonalRank))
			return false;
		
		char szKillerName[CHARACTER_NAME_MAX_LEN + 1];
		if (!Recv(sizeof(szKillerName), &szKillerName))
			return false;
		
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BattleRoyaleProcess", Py_BuildValue("(ii(si))", PythonBattleRoyaleManager::BATTLE_ROYALE_GC_PROCESS_CATEGORY_BATTLE_ROYALE_INTERFACE, p.subheader, szKillerName, bPersonalRank));
		return true;
	}
	case PythonBattleRoyaleManager::BATTLE_ROYALE_GC_WHITE_CIRCLE:
	{
		int iX;
		if (!Recv(sizeof(iX), &iX))
			return false;

		int iY;
		if (!Recv(sizeof(iY), &iY))
			return false;

		int iRadius;
		if (!Recv(sizeof(iRadius), &iRadius))
			return false;

		PythonBattleRoyaleManager::Instance().SetWhiteCirclePos(iX, iY, iRadius);
		return true;
	}
	case PythonBattleRoyaleManager::BATTLE_ROYALE_GC_RED_CIRCLE:
	{
		int iX;
		if (!Recv(sizeof(iX), &iX))
			return false;

		int iY;
		if (!Recv(sizeof(iY), &iY))
			return false;

		int iRadius;
		if (!Recv(sizeof(iRadius), &iRadius))
			return false;

		PythonBattleRoyaleManager::Instance().SetRedCirclePos(iX, iY, iRadius);
		return true;
	}
	case PythonBattleRoyaleManager::BATTLE_ROYALE_GC_SHRINK:
	{
		int iTime;
		if (!Recv(sizeof(iTime), &iTime))
			return false;

		PythonBattleRoyaleManager::Instance().Shrink(iTime);
		return true;
	}
	case PythonBattleRoyaleManager::BATTLE_ROYALE_GC_UNKNOWN:
		PythonBattleRoyaleManager::Instance().BATTLE_ROYALE_Unknown_5();
		return true;
	case PythonBattleRoyaleManager::BATTLE_ROYALE_GC_WINNER:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BattleRoyaleProcess", Py_BuildValue("(ii)", PythonBattleRoyaleManager::BATTLE_ROYALE_GC_PROCESS_CATEGORY_BATTLE_ROYALE_INTERFACE, p.subheader));
		return true;
	case PythonBattleRoyaleManager::BATTLE_ROYALE_GC_HAS_REWARD:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BattleRoyaleProcess", Py_BuildValue("(ii)", PythonBattleRoyaleManager::BATTLE_ROYALE_GC_PROCESS_CATEGORY_MINI_GAME, p.subheader));
		return true;
	}

	return false;
}

bool CPythonNetworkStream::SendBattleRoyaleApplication()
{
	TPacketCGBattleRoyale p;
	p.header = HEADER_CG_BATTLE_ROYALE;
	p.subheader = PythonBattleRoyaleManager::BATTLE_ROYALE_CG_APPLICATION;
	p.arg = false;

	if (!Send(sizeof(p), &p))
	{
		Tracen("CPythonNetworkStream::SendBattleRoyaleApplication Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendBattleRoyaleApplicationCancel()
{
	TPacketCGBattleRoyale p;
	p.header = HEADER_CG_BATTLE_ROYALE;
	p.subheader = PythonBattleRoyaleManager::BATTLE_ROYALE_CG_APPLICATION_CANCEL;
	p.arg = false;

	if (!Send(sizeof(p), &p))
	{
		Tracen("CPythonNetworkStream::SendBattleRoyaleApplicationCancel Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendBattleRoyaleExit()
{
	TPacketCGBattleRoyale p;
	p.header = HEADER_CG_BATTLE_ROYALE;
	p.subheader = PythonBattleRoyaleManager::BATTLE_ROYALE_CG_EXIT;
	p.arg = false;

	if (!Send(sizeof(p), &p))
	{
		Tracen("CPythonNetworkStream::SendBattleRoyaleExit Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendBattleRoyaleRanking()
{
	TPacketCGBattleRoyale p;
	p.header = HEADER_CG_BATTLE_ROYALE;
	p.subheader = PythonBattleRoyaleManager::BATTLE_ROYALE_CG_RANKING;
	p.arg = false;

	if (!Send(sizeof(p), &p))
	{
		Tracen("CPythonNetworkStream::SendBattleRoyaleRanking Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendBattleRoyaleStart(bool bUseSpecialItem)
{
	TPacketCGBattleRoyale p;
	p.header = HEADER_CG_BATTLE_ROYALE;
	p.subheader = PythonBattleRoyaleManager::BATTLE_ROYALE_CG_START;
	p.arg = bUseSpecialItem;

	if (!Send(sizeof(p), &p))
	{
		Tracen("CPythonNetworkStream::SendBattleRoyaleStart Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendBattleRoyaleClaimReward()
{
	TPacketCGBattleRoyale p;
	p.header = HEADER_CG_BATTLE_ROYALE;
	p.subheader = PythonBattleRoyaleManager::BATTLE_ROYALE_CG_CLAIM_REWARD;
	p.arg = 0;

	if (!Send(sizeof(p), &p))
	{
		TraceError("CPythonNetworkStream::SendBattleRoyaleClaimReward Error");
		return false;
	}

	return SendSequence();
}
#endif

#if defined(ENABLE_CLIENT_TIMER)
bool CPythonNetworkStream::RecvClientTimer()
{
	TPacketGCClientTimer Packet;
	if (!Recv(sizeof(TPacketGCClientTimer), &Packet))
		return false;

	switch (Packet.bSubHeader)
	{
		case CLIENT_TIMER_SUBHEADER_GC_SET:
		{
			PyObject* poDict = PyDict_New();
			if (poDict == nullptr)
				return false;

			for (BYTE bDataIndex = 0; bDataIndex < ECLIENT_TIMER_MAX; ++bDataIndex)
				PyDict_SetItem(poDict, Py_BuildValue("i", bDataIndex), Py_BuildValue("i", Packet.dwData[bDataIndex]));

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ClientTimerProcess",
				Py_BuildValue("iO", CLIENT_TIMER_SUBHEADER_GC_SET, poDict));

			Py_DECREF(poDict);
		}
		break;

		case CLIENT_TIMER_SUBHEADER_GC_DELETE:
		default:
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ClientTimerProcess",
				Py_BuildValue("ii", CLIENT_TIMER_SUBHEADER_GC_DELETE, 0));
		}
		break;
	}

	return true;
}
#endif

#ifdef ENABLE_TREASURE_EVENT
#include "PythonTreasureEvent.h"
bool CPythonNetworkStream::RecvTreasureEvent()
{
	TPacketGCTreasureEvent pack;
	if (!Recv(sizeof(TPacketGCTreasureEvent), &pack))
	{
		Tracen("Recv RecvTreasureEvent Packet Error");
		return false;
	}

	CPythonTreasureEvent& tmap_mngr = CPythonTreasureEvent::Instance();

	if (pack.bSubHeader == TREASURE_EVENT_GC_SUBHEADER_PLAYER)
	{
		int iAccumulatedIndex;
		if (!Recv(sizeof(int), &iAccumulatedIndex))
			return false;

		int iRewardIndex;
		if (!Recv(sizeof(int), &iRewardIndex))
			return false;

		int iDoubloon;
		if (!Recv(sizeof(int), &iDoubloon))
			return false;

		tmap_mngr.SetDoubloon(iDoubloon);
		tmap_mngr.SetAccumulatedIndex(iAccumulatedIndex);
		tmap_mngr.SetRewardFlag(iRewardIndex);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_TreasureEventUpdate", Py_BuildValue("(i)", 0));
	}
	else if (pack.bSubHeader == TREASURE_EVENT_GC_SUBHEADER_GAUGE)
	{
		int iHP;
		if (!Recv(sizeof(int), &iHP))
			return false;
		int iMaxHP;
		if (!Recv(sizeof(int), &iMaxHP))
			return false;
		BYTE bBuff[4] = {0, 0, 0, 0};
		for (int j = 0; j < 4; ++j)
		{
			if (!Recv(sizeof(BYTE), &bBuff[j]))
				return false;
		}
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetTreasureMapGauge",
			Py_BuildValue("(iiiiii)", iHP, iMaxHP, (int)bBuff[0], (int)bBuff[1], (int)bBuff[2], (int)bBuff[3]));
	}
	else if (pack.bSubHeader == TREASURE_EVENT_GC_SUBHEADER_CLOSE)
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_TreasureEventUpdate", Py_BuildValue("(i)", 1));
	}
	else if (pack.bSubHeader == TREASURE_EVENT_GC_SUBHEADER_GLOBAL_VALUE)
	{
		int iAccumulatedIndex;
		if (!Recv(sizeof(int), &iAccumulatedIndex))
			return false;
		DWORD dwAffType;
		if (!Recv(sizeof(DWORD), &dwAffType))
			return false;
		int iCount;
		if (!Recv(sizeof(int), &iCount))
			return false;
		for (BYTE j = 0; j < TREASURE_EVENT_ACCUMULATED_REWARDS_MAX_COUNT; ++j)
		{
			TTreasureAffectReward* pAffect = tmap_mngr.GetAffectReward(iAccumulatedIndex, j);
			if (pAffect && pAffect->affect_type == dwAffType)
			{
				pAffect->global_used = iCount;
				break;
			}
		}
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_TreasureEventUpdate", Py_BuildValue("(i)", 0));
	}
	else if (pack.bSubHeader == TREASURE_EVENT_GC_SUBHEADER_RANK)
	{
		BYTE bRankCount;
		if (!Recv(sizeof(BYTE), &bRankCount))
			return false;
		std::vector<TTreasureRank>& rank_vector = tmap_mngr.GetRankVector();
		rank_vector.resize(bRankCount);
		if (bRankCount > 0)
		{
			if (!Recv(bRankCount * sizeof(TTreasureRank), &rank_vector[0]))
				return false;
		}
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_TreasureEventUpdate", Py_BuildValue("(i)", 0));
	}
	else if (pack.bSubHeader == TREASURE_EVENT_GC_SUBHEADER_LOAD)
	{
		tmap_mngr.Clear();

		int iAccumulatedIndex;
		if (!Recv(sizeof(int), &iAccumulatedIndex))
			return false;

		int iRewardIndex;
		if (!Recv(sizeof(int), &iRewardIndex))
			return false;

		int iDoubloon;
		if (!Recv(sizeof(int), &iDoubloon))
			return false;

		tmap_mngr.SetDoubloon(iDoubloon);
		tmap_mngr.SetAccumulatedIndex(iAccumulatedIndex);
		tmap_mngr.SetRewardFlag(iRewardIndex);


		BYTE bRankCount;
		if (!Recv(sizeof(BYTE), &bRankCount))
			return false;
		std::vector<TTreasureRank>& rank_vector = tmap_mngr.GetRankVector();
		rank_vector.resize(bRankCount);
		if (bRankCount > 0)
		{
			if (!Recv(bRankCount * sizeof(TTreasureRank), &rank_vector[0]))
				return false;
		}

		BYTE bEventRewardsSize;
		if (!Recv(sizeof(BYTE), &bEventRewardsSize))
			return false;

		std::vector<TTreasureSimpleReward>& event_rewards = tmap_mngr.GetEventRewardsVector();
		event_rewards.resize(bEventRewardsSize);
		if (bEventRewardsSize > 0)
		{
			if (!Recv(bEventRewardsSize * sizeof(TTreasureSimpleReward), &event_rewards[0]))
				return false;
		}

		std::map<BYTE, STreasureAccumulatedReward>& accumulated_rewards = tmap_mngr.GetAccumulatedRewardsMap();
		accumulated_rewards.clear();

		BYTE bAccumulatedRewardsSize;
		if (!Recv(sizeof(BYTE), &bAccumulatedRewardsSize))
			return false;

		for (BYTE j = 0; j < bAccumulatedRewardsSize; ++j)
		{
			BYTE bIndex;
			if (!Recv(sizeof(BYTE), &bIndex))
				return false;

			STreasureAccumulatedReward item;

			BYTE bRandomRewardsSize;
			if (!Recv(sizeof(BYTE), &bRandomRewardsSize))
				return false;

			item.random_rewards.resize(bRandomRewardsSize);
			if (bRandomRewardsSize > 0)
			{
				if (!Recv(bRandomRewardsSize * sizeof(TTreasureRandomReward), &item.random_rewards[0]))
					return false;
			}

			BYTE bAffectRewardsSize;
			if (!Recv(sizeof(BYTE), &bAffectRewardsSize))
				return false;

			item.affect_rewards.resize(bAffectRewardsSize);
			if (bAffectRewardsSize > 0)
			{
				if (!Recv(bAffectRewardsSize * sizeof(TTreasureAffectReward), &item.affect_rewards[0]))
					return false;
			}
			accumulated_rewards.emplace(bIndex, item);
		}
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_TreasureEventUpdate", Py_BuildValue("(i)", 0));
	}
	return true;
}
#endif

#ifdef ENABLE_TITLE_SYSTEM
bool CPythonNetworkStream::SendTitleEquipPacket(DWORD dwTitleID)
{
	TPacketCGTitleEquip kPacket;
	kPacket.bHeader = HEADER_CG_TITLE_EQUIP;
	kPacket.dwTitleID = dwTitleID;

	if (!Send(sizeof(kPacket), &kPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvTitleDataPacket()
{
	TPacketGCTitleData kPacket;
	if (!Recv(sizeof(kPacket), &kPacket))
		return false;

	CPythonTitleManager::Instance().Clear();

	int iEntryCount = (kPacket.wSize - sizeof(kPacket)) / sizeof(TTitleEntry);
	for (int i = 0; i < iEntryCount; ++i)
	{
		TTitleEntry kEntry;
		if (!Recv(sizeof(kEntry), &kEntry))
			return false;

		bool bIsEquip = (kPacket.dwEquippedTitleID == kEntry.dwTitleID);
		CPythonTitleManager::Instance().SetPlayerTitleData(kEntry.dwTitleID, kEntry.iEndTime, bIsEquip, true);
	}

	return true;
}

bool CPythonNetworkStream::RecvTitleUpdatePacket()
{
	TPacketGCTitleUpdate kPacket;
	if (!Recv(sizeof(kPacket), &kPacket))
		return false;

	if (kPacket.bSubHeader == 0)
	{
		CPythonTitleManager::Instance().SetPlayerTitleData(kPacket.dwTitleID, kPacket.iEndTime, false, true);
	}
	else if (kPacket.bSubHeader == 1)
	{
		CPythonTitleManager::Instance().RemovePlayerTitleData(kPacket.dwTitleID);
	}

	return true;
}
#endif
