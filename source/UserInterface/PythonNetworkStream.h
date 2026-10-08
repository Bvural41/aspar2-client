#pragma once

#include "../eterLib/FuncObject.h"
#include "../eterlib/NetStream.h"
#include "../eterLib/NetPacketHeaderMap.h"
#include "InsultChecker.h"
#include "packet.h"
#ifdef ENABLE_LOADING_TIP_INFO
#include <unordered_map>
#endif
#ifdef ENABLE_SWITCHBOT
#include "PythonSwitchbot.h"
#endif

class CInstanceBase;
class CNetworkActorManager;
struct SNetworkActorData;
struct SNetworkUpdateActorData;

class CPythonNetworkStream : public CNetworkStream, public CSingleton<CPythonNetworkStream>
{
	public:
		enum
		{
			SERVER_COMMAND_LOG_OUT = 0,
			SERVER_COMMAND_RETURN_TO_SELECT_CHARACTER = 1,
			SERVER_COMMAND_QUIT = 2,

			MAX_ACCOUNT_PLAYER
		};
		
		enum
		{
			ERROR_NONE,
			ERROR_UNKNOWN,
			ERROR_CONNECT_MARK_SERVER,			
			ERROR_LOAD_MARK,
			ERROR_MARK_WIDTH,
			ERROR_MARK_HEIGHT,

			// MARK_BUG_FIX
			ERROR_MARK_UPLOAD_NEED_RECONNECT,
			ERROR_MARK_CHECK_NEED_RECONNECT,
			// END_OF_MARK_BUG_FIX
		};

		enum
		{
			ACCOUNT_CHARACTER_SLOT_ID,
			ACCOUNT_CHARACTER_SLOT_NAME,
			ACCOUNT_CHARACTER_SLOT_RACE,
			ACCOUNT_CHARACTER_SLOT_LEVEL,
			ACCOUNT_CHARACTER_SLOT_STR,
			ACCOUNT_CHARACTER_SLOT_DEX,
			ACCOUNT_CHARACTER_SLOT_HTH,
			ACCOUNT_CHARACTER_SLOT_INT,
			ACCOUNT_CHARACTER_SLOT_PLAYTIME,
			ACCOUNT_CHARACTER_SLOT_FORM,
			ACCOUNT_CHARACTER_SLOT_ADDR,
			ACCOUNT_CHARACTER_SLOT_PORT,
			ACCOUNT_CHARACTER_SLOT_GUILD_ID,
			ACCOUNT_CHARACTER_SLOT_GUILD_NAME,
			ACCOUNT_CHARACTER_SLOT_CHANGE_NAME_FLAG,
			ACCOUNT_CHARACTER_SLOT_HAIR,
#ifdef ENABLE_SASH_SYSTEM
			ACCOUNT_CHARACTER_SLOT_SASH,
#endif
#ifdef ENABLE_AURA_SYSTEM
			ACCOUNT_CHARACTER_SLOT_AURA,
#endif
			ACCOUNT_CHARACTER_SLOT_CONQUEROR_LEVEL,

			ACCOUNT_CHARACTER_SLOT_SUNGMA_ST,
			ACCOUNT_CHARACTER_SLOT_SUNGMA_HP,
			ACCOUNT_CHARACTER_SLOT_SUNGMA_MOVE,
			ACCOUNT_CHARACTER_SLOT_SUNGMA_INMUNE,
		};

		enum
		{
			PHASE_WINDOW_LOGO,
			PHASE_WINDOW_LOGIN,
			PHASE_WINDOW_SELECT,
			PHASE_WINDOW_CREATE,
			PHASE_WINDOW_LOAD,
			PHASE_WINDOW_GAME,
			PHASE_WINDOW_EMPIRE,
			PHASE_WINDOW_NUM,
		};

	public:
		CPythonNetworkStream();
		virtual ~CPythonNetworkStream();
		
		bool SendSpecial(int nLen, void * pvBuf);

		void StartGame();
		void Warp(LONG lGlobalX, LONG lGlobalY);
		
		void NotifyHack(const char* c_szMsg);		
		void SetWaitFlag();

		void SendEmoticon(UINT eEmoticon);

		void ExitApplication();
		void ExitGame();
		void LogOutGame();
		void AbsoluteExitGame();
		void AbsoluteExitApplication();

		void EnableChatInsultFilter(bool isEnable);		
		bool IsChatInsultIn(const char* c_szMsg);
		bool IsInsultIn(const char* c_szMsg);

		DWORD GetGuildID();

		UINT UploadMark(const char* c_szImageFileName);
		UINT UploadSymbol(const char* c_szImageFileName);

		void RefreshGuildMark();

		bool LoadInsultList(const char* c_szInsultListFileName);
		bool LoadConvertTable(DWORD dwEmpireID, const char* c_szFileName);

		UINT		GetAccountCharacterSlotDatau(UINT iSlot, UINT eType);
		const char* GetAccountCharacterSlotDataz(UINT iSlot, UINT eType);

		// SUPPORT_BGM
		const char*		GetFieldMusicFileName();
		float			GetFieldMusicVolume();
		// END_OF_SUPPORT_BGM

		bool IsSelectedEmpire();

		void ToggleGameDebugInfo();
#ifdef ENABLE_INGAME_WIKI
		void ToggleWikiWindow();
		void SendPythonFunc(PyObject* poArgs, const char* c_szFuncName);
		void SendPythonFuncWithoutParameter(const char* c_szFuncName);
#endif

		void SetMarkServer(const char* c_szAddr, UINT uPort);
		void ConnectLoginServer(const char* c_szAddr, UINT uPort);
		void ConnectGameServer(UINT iChrSlot);

		void SetLoginInfo(const char* c_szID, const char* c_szPassword);
#ifdef ENABLE_OFFICAL_CHARACTER_SCREEN
		std::string GetLoginID() const { return m_stID; }
#endif
		void SetLoginKey(DWORD dwLoginKey);
		void ClearLoginInfo( void );

		void SetHandler(PyObject* poHandler);
		void SetPhaseWindow(UINT ePhaseWnd, PyObject* poPhaseWnd);
		void ClearPhaseWindow(UINT ePhaseWnd, PyObject* poPhaseWnd);
		void SetServerCommandParserWindow(PyObject* poPhaseWnd);
		bool IsGamePhase() const { return m_strPhase == "Game"; }
		const std::string& GetPhaseName() const { return m_strPhase; }

		bool SendSyncPositionElementPacket(DWORD dwVictimVID, DWORD dwVictimX, DWORD dwVictimY);

		bool SendAttackPacket(UINT uMotAttack, DWORD dwVIDVictim);
		bool SendCharacterStatePacket(const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg);
		bool SendUseSkillPacket(DWORD dwSkillIndex, DWORD dwTargetVID=0);
		bool SendTargetPacket(DWORD dwVID);

		// OLDCODE:
		bool SendCharacterStartWalkingPacket(float fRotation, long lx, long ly);
		bool SendCharacterEndWalkingPacket(float fRotation, long lx, long ly);
		bool SendCharacterCheckWalkingPacket(float fRotation, long lx, long ly);

		bool SendCharacterPositionPacket(BYTE iPosition);
		bool SendItemUsePacket(TItemPos pos
#ifdef ENABLE_GIFTBOX_MULTI_OPEN
			, ICOUNT open_count = 0
#endif
		);

		bool SendItemUseToItemPacket(TItemPos source_pos, TItemPos target_pos);
		bool SendItemDestroyPacket(TItemPos pos);
		bool SendItemSellPacket(TItemPos pos);
#ifdef ENABLE_CHEQUE_SYSTEM
		bool SendItemDropPacket(TItemPos pos, DWORD elk, DWORD won);
		bool SendItemDropPacketNew(TItemPos pos, DWORD elk, DWORD won, DWORD count);
#else
		bool SendItemDropPacket(TItemPos pos, DWORD elk);
		bool SendItemDropPacketNew(TItemPos pos, DWORD elk, DWORD count);
#endif
#ifdef ENABLE_EXTENDED_ITEM_COUNT
		bool SendItemMovePacket(TItemPos pos, TItemPos change_pos, WORD num);
#else
		bool SendItemMovePacket(TItemPos pos, TItemPos change_pos, BYTE num);
#endif
		bool SendItemPickUpPacket(DWORD vid);
		bool SendQuickSlotAddPacket(UINT wpos, BYTE type, UINT pos);
		bool SendQuickSlotDelPacket(UINT wpos);
		bool SendQuickSlotMovePacket(UINT wpos, UINT change_pos);

		// PointReset 개 임시
		bool SendPointResetPacket();

#ifdef ENABLE_BATTLE_ROYALE
		bool SendBattleRoyaleApplication();
		bool SendBattleRoyaleApplicationCancel();
		bool SendBattleRoyaleExit();
		bool SendBattleRoyaleRanking();
		bool SendBattleRoyaleStart(bool bUseSpecialItem);
		bool SendBattleRoyaleClaimReward();
#endif

		// Shop
		bool SendShopEndPacket();
		bool SendShopBuyPacket(BYTE byCount);
		bool SendShopSellPacket(BYTE bySlot);
#ifdef ENABLE_EXTENDED_ITEM_COUNT
		bool SendShopSellPacketNew(WORD wSlot, WORD byCount, BYTE byType);
#else
		bool SendShopSellPacketNew(WORD wSlot, BYTE byCount, BYTE byType);
#endif

#ifdef ENABLE_FISH_EVENT_SYSTEM
		bool SendFishBoxUse(BYTE bWindow, WORD wCell);
		bool SendFishShapeAdd(BYTE bPos);
#endif

#ifdef ENABLE_SOUL_ROULETTE_SYSTEM
		bool SoulRoulette(const BYTE option);
		bool RecvSoulRoulette();
#endif

#ifdef ENABLE_WON_EXCHANGE_WINDOW
		// Won Exchange
		bool SendWonExchangeSellPacket(WORD wValue);
		bool SendWonExchangeBuyPacket(WORD wValue);
#endif

#ifdef ENABLE_OFFLINESHOP_SYSTEM
		bool SendOfflineShopEndPacket();
		bool SendOfflineShopBuyPacket(DWORD vid, BYTE pos);
		bool SendAddOfflineShopItem(TItemPos bDisplayPos, BYTE bPos, long long lPrice, int lPrice_Cheque);
		bool SendRemoveOfflineShopItem(BYTE bPos);
		bool SendOpenShopSlot(BYTE bPos);
		bool SendGetBackItems();
		bool SendAddTime();
		bool SendDestroyOfflineShop();
		bool SendTakeOfflineMoney();
		bool SendTakeOfflineMoneyCheque();
		bool SendOfflineShopChangeTitle(const char* title);
		bool SendOfflineShopCheck();
		bool SendOfflineShopButton();
		bool SendOfflineOpenWithVID(DWORD vid);
		bool SendOfflineShopChangeDecoration(const char* sign, DWORD vnum, DWORD type);
#endif

		// Exchange
		bool SendExchangeStartPacket(DWORD vid);
#ifdef ENABLE_ITEM_CHECKINOUT_UPDATE
		bool SendExchangeItemAddPacket(TItemPos ItemPos, BYTE byDisplayPos, bool SelectPosAuto);
#else
		bool SendExchangeItemAddPacket(TItemPos ItemPos, BYTE byDisplayPos);
#endif
		bool SendExchangeElkAddPacket(long long elk);
		bool SendExchangeItemDelPacket(BYTE pos);
		bool SendExchangeAcceptPacket();
		bool SendExchangeExitPacket();

		// Quest
		bool SendScriptAnswerPacket(int iAnswer);
		bool SendScriptButtonPacket(unsigned int iIndex);
		bool SendAnswerMakeGuildPacket(const char * c_szName);
		bool SendQuestInputStringPacket(const char * c_szString);

		bool SendQuestConfirmPacket(BYTE byAnswer, DWORD dwPID);

		// Event
		bool SendOnClickPacket(DWORD vid);

#ifdef ENABLE_FLOWER_EVENT
		bool SendFlowerEventExchange(uint8_t id);
#endif

		// Fly
		bool SendFlyTargetingPacket(DWORD dwTargetVID, const TPixelPosition& kPPosTarget);
		bool SendAddFlyTargetingPacket(DWORD dwTargetVID, const TPixelPosition& kPPosTarget);
		bool SendShootPacket(UINT uSkill);

		// Command
		bool ClientCommand(const char * c_szCommand);
		void ServerCommand(char * c_szCommand);

		// Emoticon
		void RegisterEmoticonString(const char * pcEmoticonString);

		// Party
		bool SendPartyInvitePacket(DWORD dwVID);
		bool SendPartyInviteAnswerPacket(DWORD dwLeaderVID, BYTE byAccept);
		bool SendPartyRemovePacket(DWORD dwPID);
		bool SendPartySetStatePacket(DWORD dwVID, BYTE byState, BYTE byFlag);
		bool SendPartyUseSkillPacket(BYTE bySkillIndex, DWORD dwVID);
		bool SendPartyParameterPacket(BYTE byDistributeMode);

		// SafeBox
		bool SendSafeBoxMoneyPacket(BYTE byState, DWORD dwMoney);
		bool SendSafeBoxCheckinPacket(TItemPos InventoryPos, BYTE bySafeBoxPos);
		bool SendSafeBoxCheckoutPacket(BYTE bySafeBoxPos, TItemPos InventoryPos);
#ifdef ENABLE_EXTENDED_ITEM_COUNT
		bool SendSafeBoxItemMovePacket(BYTE bySourcePos, BYTE byTargetPos, WORD byCount);
#else
		bool SendSafeBoxItemMovePacket(BYTE bySourcePos, BYTE byTargetPos, BYTE byCount);
#endif

		// Mall
		bool SendMallCheckoutPacket(BYTE byMallPos, TItemPos InventoryPos);

		// Guild
		bool SendGuildAddMemberPacket(DWORD dwVID);
		bool SendGuildRemoveMemberPacket(DWORD dwPID);
		bool SendGuildChangeGradeNamePacket(BYTE byGradeNumber, const char * c_szName);
		bool SendGuildChangeGradeAuthorityPacket(BYTE byGradeNumber, BYTE byAuthority);
		bool SendGuildOfferPacket(DWORD dwExperience);
		bool SendGuildPostCommentPacket(const char * c_szMessage);
		bool SendGuildDeleteCommentPacket(DWORD dwIndex);
		bool SendGuildRefreshCommentsPacket(DWORD dwHighestIndex);
		bool SendGuildChangeMemberGradePacket(DWORD dwPID, BYTE byGrade);
		bool SendGuildUseSkillPacket(DWORD dwSkillID, DWORD dwTargetVID);
		bool SendGuildChangeMemberGeneralPacket(DWORD dwPID, BYTE byFlag);
		bool SendGuildInvitePacket(DWORD dwVID);
		bool SendGuildInviteAnswerPacket(DWORD dwGuildID, BYTE byAnswer);
		bool SendGuildChargeGSPPacket(DWORD dwMoney);
		bool SendGuildDepositMoneyPacket(DWORD dwMoney);
		bool SendGuildWithdrawMoneyPacket(DWORD dwMoney);

#ifdef ENABLE_MAILBOX_SYSTEM
		bool SendMailBoxClose();
#ifdef ENABLE_CHEQUE_SYSTEM
		bool SendMailBoxMail(const char* nombre_pj, const char* asunto, const char* descrip, int slot_item, int yang, int won);
#else
		bool SendMailBoxMail(const char* nombre_pj, const char* asunto, const char* descrip, int slot_item, int yang);
#endif
		bool SendMailBoxNameCheck(const char* nombre_pj);
		bool SendMailBoxAcceptMail(int index_mail);
		bool SendMailBoxDeleteMail(int index_mail);
		bool SendMailBoxAcceptAllMail();
		bool SendMailBoxDeleteAllMail();
		bool SendMailBoxViewMail(int index_mail);
#endif

		// Mall
		bool RecvMallOpenPacket();
		bool RecvMallItemSetPacket();
		bool RecvMallItemDelPacket();

#ifdef ENABLE_AUTO_SYSTEM
		void SendAutoCoolTime(int slotIndex, int iValue);
#endif

#ifdef ENABLE_KILL_STATISTICS
		bool RecvKillStatistics();
#endif

		// Lover
		bool RecvLoverInfoPacket();
		bool RecvLovePointUpdatePacket();

		// Dig
		bool RecvDigMotionPacket();

#ifdef ENABLE_TREASURE_EVENT
		bool RecvTreasureEvent();
#endif

		// Fishing
		bool SendFishingPacket(int iRotation);
		bool SendGiveItemPacket(DWORD dwTargetVID, TItemPos ItemPos, int iItemCount);

#ifdef ENABLE_OFFLINESHOP_SYSTEM
		bool SendBuildOfflineShopPacket(const char* c_szName, const std::vector<TOfflineShopItemTable>& c_rSellingItemStock, DWORD shopVnum, BYTE shopTitle);
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		bool SendChangeLanguagePacket(BYTE bLanguage);
		bool SendChangeLanguage(const char* lang);

		void SetLanguage(BYTE bLanguage) { m_bLanguage = bLanguage; }
		int GetLanguage() const { return m_bLanguage; }

	protected:
		BYTE m_bLanguage;

	public:
#endif

		// Private Shop
		bool SendBuildPrivateShopPacket(const char * c_szName, const std::vector<TShopItemTable> & c_rSellingItemStock);

		// Refine
		bool SendRefinePacket(UINT byPos, BYTE byType);

		bool SendSelectItemPacket(DWORD dwItemPos);

		// Client Version
		bool SendClientVersionPacket();

		// CRC Report
		bool __SendCRCReportPacket();

		// 용홍석 강화
		bool SendDragonSoulRefinePacket(BYTE bRefineType, TItemPos* pos);

		// Handshake
		bool RecvHandshakePacket();
		bool RecvHandshakeOKPacket();

		bool RecvHybridCryptKeyPacket();
		bool RecvHybridCryptSDBPacket();
#ifdef _IMPROVED_PACKET_ENCRYPTION_
		bool RecvKeyAgreementPacket();
		bool RecvKeyAgreementCompletedPacket();

#endif
		// ETC
		DWORD GetMainActorVID();
		DWORD GetMainActorRace();
		DWORD GetMainActorEmpire();
		DWORD GetMainActorSkillGroup();
		void SetEmpireID(DWORD dwEmpireID);
		DWORD GetEmpireID();
		void __TEST_SetSkillGroupFake(int iIndex);
#ifdef ENABLE_ATTR_6TH_7TH_SYSTEM
		bool Send67AttrPacket(int iMaterialCount, int iSupportCount, int iSupportPos, int iItemPos);
		bool Send67AttrClosePacket();
		bool Recv67AttrOpenPacket();
#endif
#ifdef ENABLE_OX_RENEWAL
		bool SendQuestInputStringLongPacket(const char * c_szString);
#endif
#ifdef ENABLE_SELECT_REWARD_BOX
		bool 	SendSelectRewardInfo(WORD wInventoryCell);
		bool 	RecvSelectRewardInfo();
		bool	SendSelectRewardSelectPacket(int iInventoryPos, DWORD dwItemVnum, BYTE bItemCount);
		bool	RecvSelectRewardResultPacket();
#endif
#ifdef ENABLE_CHEQUE_SYSTEM
		bool SendExchangeWonAddPacket(DWORD won);
#endif
#ifdef ENABLE_MINI_GAME_CATCH_KING
		bool SendMiniGameCatchKing(BYTE bSubHeader, BYTE bSubArgument);
#endif
#ifdef ENABLE_SHOW_CHEST_DROP
		bool 	SendChestDropInfo(WORD wInventoryCell);
		bool 	SendChestDropInfoByVnum(DWORD dwItemVnum);
		bool 	RecvChestDropInfo();
#endif
#ifdef ENABLE_CUBE_RENEWAL
		bool CubeRenewalMakeItem(int index_item, int count_item, int index_item_improve);
		bool CubeRenewalClose();
		bool RecvCubeRenewalPacket();
#endif
#ifdef ENABLE_EVENT_SYSTEM
		bool RecvEventInfo();
#endif
#ifdef ENABLE_AURA_SYSTEM
		bool RecvAuraPacket(bool bReturn = false);
		bool SendAuraClosePacket();
		bool SendAuraAddPacket(TItemPos tPos, BYTE bPos);
		bool SendAuraRemovePacket(BYTE bPos);
		bool SendAuraRefinePacket();
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
		bool	SendClClosePacket();
		bool	SendClAddPacket(TItemPos tPos, BYTE bPos);
		bool	SendClRemovePacket(BYTE bPos);
		bool	SendClRefinePacket();
#endif
#ifdef ENABLE_SASH_SYSTEM
		bool SendSashClosePacket();
		bool SendSashAddPacket(TItemPos tPos, BYTE bPos);
		bool SendSashRemovePacket(BYTE bPos);
		bool SendSashRefinePacket();
#endif
#ifdef ENABLE_EXTEND_INVEN_SYSTEM
		bool SendExtendInvenUpgrade();
		bool SendExtendInvenButtonClick(int index);
#endif

#ifdef ENABLE_MOVE_CHANNEL
		void SetMapIndex(int iMapIndex);
		int GetMapIndex();

		void SetChannelNumber(BYTE bChannelNumber);
		BYTE GetChannelNumber();

	protected:
		BYTE m_bChannelNumber;
		int m_iMapIndex;
#endif

	//////////////////////////////////////////////////////////////////////////
	// Phase 관련
	//////////////////////////////////////////////////////////////////////////
	public:
		void SetOffLinePhase();
		void SetHandShakePhase();
		void SetLoginPhase();
		void SetSelectPhase();
		void SetLoadingPhase();
		void SetGamePhase();
		void ClosePhase();

		// Login Phase
		bool SendLoginPacket(const char * c_szName, const char * c_szPassword);
		bool SendLoginPacketNew(const char * c_szName, const char * c_szPassword);
		bool SendChinaMatrixCardPacket(const char * c_szMatrixCardString);
		bool SendRunupMatrixAnswerPacket(const char * c_szMatrixCardString);
		bool SendNEWCIBNPasspodAnswerPacket(const char * answer);
		bool SendDirectEnterPacket(const char * c_szName, const char * c_szPassword, UINT uChrSlot);

		bool SendEnterGame();

		// Select Phase
		bool SendSelectEmpirePacket(DWORD dwEmpireID);
		bool SendSelectCharacterPacket(BYTE account_Index);
		bool SendChangeNamePacket(BYTE index, const char *name);
		bool SendCreateCharacterPacket(BYTE index, const char *name, BYTE job, BYTE shape, BYTE byStat1, BYTE byStat2, BYTE byStat3, BYTE byStat4);
		bool SendDestroyCharacterPacket(BYTE index, const char * szPrivateCode);

		// Main Game Phase
		bool SendC2CPacket(DWORD dwSize, void * pData);
		bool SendChatPacket(const char * c_szChat, BYTE byType = CHAT_TYPE_TALKING);
		bool SendWhisperPacket(const char * name, const char * c_szChat);
#ifdef ENABLE_ADMIN_BAN_MANAGER
		bool SendAdminBanManager(int action, const char* c_szUserName, const char* c_szReason, int duration);
#endif
#ifdef ENABLE_BATTLE_FIELD
		bool SendCombatZoneRequestActionPacket(int action, int value);
#endif
#ifdef ENABLE_WHISPER_ADMIN_SYSTEM
		bool SendWhisperAdminPacket(const char* c_szText, const char* c_szLang, int color);
#endif
		bool SendMobileMessagePacket(const char * name, const char * c_szChat);
		bool SendMessengerAddByVIDPacket(DWORD vid);
		bool SendMessengerAddByNamePacket(const char * c_szName);
		bool SendMessengerRemovePacket(const char * c_szKey, const char * c_szName);
#ifdef ENABLE_MESSENGER_RENEWAL
		void RequestMessengerRefresh();
		bool SendMessengerSetConnectionStatePacket(BYTE bConnectionState);
		bool SendMessengerSetStatusMessagePacket(const char* c_szMessage);
		bool SendMessengerDeleteStatusMessagePacket();
#endif

#ifdef ENABLE_MESSENGER_BLOCK
		bool SendMessengerAddBlockByVIDPacket(DWORD vid);
		bool SendMessengerAddBlockByNamePacket(const char * c_szName);
		bool SendMessengerRemoveBlockPacket(const char * c_szKey, const char * c_szName);
#endif

#ifdef ENABLE_OFFLINESHOP_SYSTEM
		bool RecvOfflineShopPacket();
		bool RecvOfflineShopSignPacket();
#endif

#ifdef ENABLE_SHOP_SEARCH_SYSTEM
		bool SendPrivateShopSearchInfo(TPacketCGShopSearch* p);
		bool RecvShopSearchSet();
#endif

		bool __IsNotPing();

		void __DownloadMark();
		void __DownloadSymbol(const std::vector<DWORD> & c_rkVec_dwGuildID);

		void __PlayInventoryItemUseSound(TItemPos uSlotPos);
		void __PlayInventoryItemDropSound(TItemPos uSlotPos);
		void __PlaySafeBoxItemDropSound(UINT uSlotPos);
		void __PlayMallItemDropSound(UINT uSlotPos);

		bool __CanActMainInstance();

#ifdef ENABLE_LEFT_SEAT
	public:
		bool SendLeftSeatWaitTimeIndexPacket(BYTE bIndex);
		bool SendLeftSeatOffPacket(BYTE bIndex);
		bool DisableLeftSeatLogOutState();
#endif

	public:
		void __RefreshInventoryWindow();
		void __RefreshSafeboxWindow();
		enum REFRESH_WINDOW_TYPE
		{
			RefreshStatus = (1 << 0),
			RefreshAlignmentWindow = (1 << 1),
			RefreshCharacterWindow = (1 << 2),
			RefreshEquipmentWindow = (1 << 3), 
			RefreshInventoryWindow = (1 << 4),
			RefreshExchangeWindow = (1 << 5),
			RefreshSkillWindow = (1 << 6),
			RefreshSafeboxWindow  = (1 << 7),
			RefreshMessengerWindow = (1 << 8),
			RefreshGuildWindowInfoPage = (1 << 9),
			RefreshGuildWindowBoardPage = (1 << 10),
			RefreshGuildWindowMemberPage = (1 << 11), 
			RefreshGuildWindowMemberPageGradeComboBox = (1 << 12),
			RefreshGuildWindowSkillPage = (1 << 13),
			RefreshGuildWindowGradePage = (1 << 14),
			RefreshTargetBoard = (1 << 15),
			RefreshMallWindow = (1 << 16),
		};

	protected:
		bool OnProcess();	// State들을 실제로 실행한다.
		void OffLinePhase();
		void HandShakePhase();
		void LoginPhase();
		void SelectPhase();
		void LoadingPhase();
		void GamePhase();
		void __RefreshStatus();
		void __RefreshAlignmentWindow();
		void __RefreshCharacterWindow();
		void __RefreshEquipmentWindow();
		void __RefreshExchangeWindow();
		void __RefreshSkillWindow();
		void __RefreshMessengerWindow();
		void __RefreshGuildWindowInfoPage();
		void __RefreshGuildWindowBoardPage();
		void __RefreshGuildWindowMemberPage();
		void __RefreshGuildWindowMemberPageGradeComboBox();
		void __RefreshGuildWindowSkillPage();
		void __RefreshGuildWindowGradePage();
		void __RefreshTargetBoardByVID(DWORD dwVID);
		void __RefreshTargetBoardByName(const char * c_szName);
		void __RefreshTargetBoard();
		void __RefreshMallWindow();
		bool __SendHack(const char* c_szMsg);
#ifdef ENABLE_SHOP_SEARCH_SYSTEM
		void __RefreshShopSearchWindow();
#endif

	protected:
		bool RecvObserverAddPacket();
		bool RecvObserverRemovePacket();
		bool RecvObserverMovePacket();

		// Common
		bool RecvErrorPacket(int header);
		bool RecvPingPacket();
		bool RecvDefaultPacket(int header);
		bool RecvPhasePacket();

#ifdef ENABLE_PACKET_DESYNC_LOG
		void LogPacketTrace(BYTE bHeader, int iSize, const void* pData = NULL);
		void DumpPacketDesync(int iBadHeader, const char* szPhase);
#endif

		// Login Phase
		bool __RecvLoginSuccessPacket3();
		bool __RecvLoginSuccessPacket4();
		bool __RecvLoginFailurePacket();
		bool __RecvEmpirePacket();
		bool __RecvChinaMatrixCardPacket();
		bool __RecvRunupMatrixQuizPacket();
		bool __RecvNEWCIBNPasspodRequestPacket();
		bool __RecvNEWCIBNPasspodFailurePacket();
		bool __RecvLoginKeyPacket();

		// Select Phase
		bool __RecvPlayerCreateSuccessPacket();
		bool __RecvPlayerCreateFailurePacket();
		bool __RecvPlayerDestroySuccessPacket();
		bool __RecvPlayerDestroyFailurePacket();
		bool __RecvPreserveItemPacket();
		bool __RecvPlayerPoints();
		bool __RecvChangeName();
#ifdef ENABLE_BATTLE_FIELD
		bool __RecvCombatZoneRankingPacket();
		bool __RecvCombatZonePacket();
#endif
		// Loading Phase
		bool RecvMainCharacter();		
		bool RecvMainCharacter2_EMPIRE();
		bool RecvMainCharacter3_BGM();
		bool RecvMainCharacter4_BGM_VOL();

		void __SetFieldMusicFileName(const char* musicName);
		void __SetFieldMusicFileInfo(const char* musicName, float vol);
		// END_OF_SUPPORT_BGM

		// Main Game Phase
		bool RecvWarpPacket();
		bool RecvPVPPacket();
		bool RecvDuelStartPacket();
		bool RecvGlobalTimePacket();
		bool RecvCharacterAppendPacket();
		bool RecvCharacterAdditionalInfo();

		bool RecvCharacterUpdatePacket();

		bool RecvCharacterDeletePacket();
		bool RecvChatPacket();
		bool RecvOwnerShipPacket();
		bool RecvSyncPositionPacket();
		bool RecvWhisperPacket();
		bool RecvPointChange();						// Alarm to python
		bool RecvChangeSpeedPacket();

		bool RecvStunPacket();
		bool RecvDeadPacket();
		bool RecvCharacterMovePacket();

		bool RecvItemSetPacket();					// Alarm to python
		bool RecvItemSetPacket2();					// Alarm to python
		bool RecvItemUsePacket();					// Alarm to python
		bool RecvItemUpdatePacket();				// Alarm to python
		bool RecvItemGroundAddPacket();
		bool RecvItemGroundDelPacket();
		bool RecvItemOwnership();

		bool RecvQuickSlotAddPacket();				// Alarm to python
		bool RecvQuickSlotDelPacket();				// Alarm to python
		bool RecvQuickSlotMovePacket();				// Alarm to python

		bool RecvCharacterPositionPacket();
		bool RecvMotionPacket();

		bool RecvShopPacket();
		bool RecvShopSignPacket();
		bool RecvExchangePacket();

		// Quest
		bool RecvScriptPacket();
		bool RecvQuestInfoPacket();
		bool RecvQuestConfirmPacket();
		bool RecvRequestMakeGuild();

		// Skill
		bool RecvSkillLevel();
		bool RecvSkillLevelNew();
		bool RecvSkillCoolTimeEnd();

		// Target
		bool RecvTargetPacket();
		bool RecvViewEquipPacket();
		bool RecvDamageInfoPacket();
#ifdef ENABLE_SUPPORT_SYSTEM
		bool RecvSupportShamanUseSkill();
#endif
#ifdef ENABLE_TARGET_INFORMATION_SYSTEM
		bool RecvTargetInfoPacket();

		public:
			bool SendTargetInfoLoadPacket(DWORD dwVID);

		protected:
#endif

		// Mount
		bool RecvMountPacket();

		// Fly
		bool RecvCreateFlyPacket();
		bool RecvFlyTargetingPacket();
		bool RecvAddFlyTargetingPacket();

		// Messenger
		bool RecvMessenger();

		// Guild
		bool RecvGuild();

		// Party
		bool RecvPartyInvite();
		bool RecvPartyAdd();
		bool RecvPartyUpdate();
		bool RecvPartyRemove();
		bool RecvPartyLink();
		bool RecvPartyUnlink();
		bool RecvPartyParameter();

		// SafeBox
		bool RecvSafeBoxSetPacket();
		bool RecvSafeBoxDelPacket();
		bool RecvSafeBoxWrongPasswordPacket();
		bool RecvSafeBoxSizePacket();
		bool RecvSafeBoxMoneyChangePacket();

#ifdef ENABLE_MINI_GAME_CATCH_KING
		bool RecvMiniGameCatchKingPacket();
#endif

#ifdef ENABLE_FISH_EVENT_SYSTEM
		bool RecvFishEventInfo();
#endif

		// Fishing
		bool RecvFishing();

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		// Multi Language
		bool RecvRequestChangeLanguage();
#endif

		// Dungeon
		bool RecvDungeon();

		// Time
		bool RecvTimePacket();

		// WalkMode
		bool RecvWalkModePacket();

		// ChangeSkillGroup
		bool RecvChangeSkillGroupPacket();

		// Refine
		bool RecvRefineInformationPacket();
		bool RecvRefineInformationPacketNew();

		// Use Potion
		bool RecvSpecialEffect();

#ifdef ENABLE_12ZI
		bool RecvSpecialZodiacEffect();
#endif

		// 서버에서 지정한 이팩트 발동 패킷.
		bool RecvSpecificEffect();
		
		// 용혼석 관련
		bool RecvDragonSoulRefine();

		// MiniMap Info
		bool RecvNPCList();
		bool RecvLandPacket();
		bool RecvTargetCreatePacket();
		bool RecvTargetCreatePacketNew();
		bool RecvTargetUpdatePacket();
		bool RecvTargetDeletePacket();

		// Affect
		bool RecvAffectAddPacket();
		bool RecvAffectRemovePacket();

#ifdef ENABLE_EXTEND_INVEN_SYSTEM
		bool RecvExtendInventoryPacket();
#endif

		// Channel
		bool RecvChannelPacket();
#ifdef ENABLE_BATTLE_ROYALE
		bool RecvBattleRoyale();
#endif
#ifdef ENABLE_SASH_SYSTEM
		bool RecvSashPacket(bool bReturn = false);
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
		bool	RecvChangeLookPacket();
#endif
#ifdef ENABLE_MAILBOX_SYSTEM
		bool	RecvMailBox();
#endif

		//Security
		bool RecvHSCheckRequest();
		bool RecvXTrapVerifyRequest();

#ifdef ENABLE_DS_SET
		bool	RecvDSTablePacket();
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
	public:
		bool SendBornPet(const char* petname);
		bool SendRenewPetName(const char* petname, int itempos1, int itempos2);
		bool RecvGrowthPetInformation();
#endif

	protected:
		// 이모티콘
		bool ParseEmoticon(const char * pChatMsg, DWORD * pdwEmoticon);

		// 파이썬으로 보내는 콜들
		void OnConnectFailure();
		void OnScriptEventStart(int iSkin, int iIndex);
		void HideQuestWindows();
		
		void OnRemoteDisconnect();
		void OnDisconnect();

		void SetGameOnline();
		void SetGameOffline();
		BOOL IsGameOnline();

#ifdef ENABLE_SKILLBOOK_COMB_SYSTEM
	public:
		bool SendSkillBookCombinationPacket(TItemPos* pPos, BYTE bAction);
#endif

#ifdef ENABLE_REFINE_ELEMENT
	public:
		bool SendRefineElementPacket(BYTE);
	protected:
		bool RecvRefineElementPacket();
#endif

	protected:
		bool CheckPacket(TPacketHeader * pRetHeader);
		
		void __InitializeGamePhase();
		void __InitializeMarkAuth();
		void __GlobalPositionToLocalPosition(LONG& rGlobalX, LONG& rGlobalY);
		void __LocalPositionToGlobalPosition(LONG& rLocalX, LONG& rLocalY);
#if defined(_WIN32)
		inline void __GlobalPositionToLocalPosition(int& rGlobalX, int& rGlobalY) {
			LONG x = (LONG)rGlobalX, y = (LONG)rGlobalY;
			__GlobalPositionToLocalPosition(x, y);
			rGlobalX = (int)x; rGlobalY = (int)y;
		}
		inline void __LocalPositionToGlobalPosition(int& rLocalX, int& rLocalY) {
			LONG x = (LONG)rLocalX, y = (LONG)rLocalY;
			__LocalPositionToGlobalPosition(x, y);
			rLocalX = (int)x; rLocalY = (int)y;
		}
#elif defined(__ANDROID__) || defined(__linux__)
		inline void __GlobalPositionToLocalPosition(long& rGlobalX, long& rGlobalY) {
			LONG x = (LONG)rGlobalX, y = (LONG)rGlobalY;
			__GlobalPositionToLocalPosition(x, y);
			rGlobalX = x; rGlobalY = y;
		}
		inline void __LocalPositionToGlobalPosition(long& rLocalX, long& rLocalY) {
			LONG x = (LONG)rLocalX, y = (LONG)rLocalY;
			__LocalPositionToGlobalPosition(x, y);
			rLocalX = x; rLocalY = y;
		}
#endif

		bool __IsPlayerAttacking();
		bool __IsEquipItemInSlot(TItemPos Cell);

		void __ShowMapName(LONG lLocalX, LONG lLocalY);

		void __LeaveOfflinePhase() {}
		void __LeaveHandshakePhase() {}
		void __LeaveLoginPhase() {}
		void __LeaveSelectPhase() {}
		void __LeaveLoadingPhase() {}
		void __LeaveGamePhase();

		void __ClearNetworkActorManager();

		void __ClearSelectCharacterData();

		// DELETEME
		//void __SendWarpPacket();

		void __ConvertEmpireText(DWORD dwEmpireID, char* szText);

		void __RecvCharacterAppendPacket(SNetworkActorData * pkNetActorData);
		void __RecvCharacterUpdatePacket(SNetworkUpdateActorData * pkNetUpdateActorData);

		void __FilterInsult(char* szLine, UINT uLineLen);

		void __SetGuildID(DWORD id);

	protected:
		TPacketGCHandshake m_HandshakeData;
		DWORD m_dwChangingPhaseTime;
		DWORD m_dwBindupRetryCount;
		DWORD m_dwMainActorVID;
		DWORD m_dwMainActorRace;
		DWORD m_dwMainActorEmpire;
		DWORD m_dwMainActorSkillGroup;
		BOOL m_isGameOnline;
		BOOL m_isStartGame;

		DWORD m_dwGuildID;
		DWORD m_dwEmpireID;
		
		struct SServerTimeSync
		{
			DWORD m_dwChangeServerTime;
			DWORD m_dwChangeClientTime;
		} m_kServerTimeSync;

		void __ServerTimeSync_Initialize();
		//DWORD m_dwBaseServerTime;
		//DWORD m_dwBaseClientTime;

		DWORD m_dwLastGamePingTime;

		std::string	m_stID;
		std::string	m_stPassword;
		std::string	m_strLastCommand;
		std::string	m_strPhase;
		DWORD m_dwLoginKey;
		BOOL m_isWaitLoginKey;

#ifdef ENABLE_PACKET_DESYNC_LOG
		SPacketTraceEntry m_aPacketHistory[64];
		int m_iPacketHistoryIndex;
#endif

		std::string m_stMarkIP;

		CFuncObject<CPythonNetworkStream>	m_phaseProcessFunc;
		CFuncObject<CPythonNetworkStream>	m_phaseLeaveFunc;

		PyObject*							m_poHandler;
		PyObject*							m_apoPhaseWnd[PHASE_WINDOW_NUM];
		PyObject*							m_poSerCommandParserWnd;

		TSimplePlayerInformation			m_akSimplePlayerInfo[PLAYER_PER_ACCOUNT4];
		DWORD								m_adwGuildID[PLAYER_PER_ACCOUNT4];
		std::string							m_astrGuildName[PLAYER_PER_ACCOUNT4];
		bool m_bSimplePlayerInfo;

		CRef<CNetworkActorManager>			m_rokNetActorMgr;

		bool m_isRefreshStatus;
		bool m_isRefreshCharacterWnd;
		bool m_isRefreshEquipmentWnd;
		bool m_isRefreshInventoryWnd;
		bool m_isRefreshExchangeWnd;
		bool m_isRefreshSkillWnd;
		bool m_isRefreshSafeboxWnd;
		bool m_isRefreshMallWnd;
		bool m_isRefreshMessengerWnd;
		bool m_isRefreshGuildWndInfoPage;
		bool m_isRefreshGuildWndBoardPage;
		bool m_isRefreshGuildWndMemberPage;
		bool m_isRefreshGuildWndMemberPageGradeComboBox;
		bool m_isRefreshGuildWndSkillPage;
		bool m_isRefreshGuildWndGradePage;
#ifdef ENABLE_SHOP_SEARCH_SYSTEM
		bool m_isRefreshShopSearchWnd;
#endif

		// Emoticon
		std::vector<std::string> m_EmoticonStringVector;

		struct STextConvertTable 
		{
			char acUpper[26];
			char acLower[26];
			BYTE aacHan[5000][2];
		} m_aTextConvTable[3];



		struct SMarkAuth
		{
			CNetworkAddress m_kNetAddr;
			DWORD m_dwHandle;
			DWORD m_dwRandomKey;
		} m_kMarkAuth;



		DWORD m_dwSelectedCharacterIndex;

		CInsultChecker m_kInsultChecker;

		bool m_isEnableChatInsultFilter;
		bool m_bComboSkillFlag;

		std::deque<std::string> m_kQue_stHack;

#ifdef ENABLE_LEFT_SEAT
	public:
		void SetLeftSeatLogOutState(bool bLeftSeatLogOutState) { m_bIsLeftSeatLogOutState = bLeftSeatLogOutState; }
		bool IsLeftSeatLogOutState() const { return m_bIsLeftSeatLogOutState; }
	
	protected:
		bool m_bIsLeftSeatLogOutState;
#endif

	private:
		struct SDirectEnterMode
		{
			bool m_isSet;
			DWORD m_dwChrSlotIndex;
		} m_kDirectEnterMode;

		void __DirectEnterMode_Initialize();
		void __DirectEnterMode_Set(UINT uChrSlotIndex);
		bool __DirectEnterMode_IsSet();
#ifdef ENABLE_LOADING_TIP_INFO
		std::unordered_map<int, std::string> m_TipVnum; // vnum id, string
		std::vector<std::pair<std::vector<long>, std::vector<int>>> m_TipList; // map index, vnum id
#endif

	public:
		DWORD EXPORT_GetBettingGuildWarValue(const char* c_szValueName);
#ifdef ENABLE_LOADING_TIP_INFO
		bool LoadTipVnum(const char* FileName);
		bool LoadTipList(const char* FileName);
		const decltype(m_TipVnum)& GetTipMap() const { return m_TipVnum; };
		const decltype(m_TipList)& GetTipList() const { return m_TipList; };
#endif

	private:
		struct SBettingGuildWar
		{
			DWORD m_dwBettingMoney;
			DWORD m_dwObserverCount;
		} m_kBettingGuildWar;

		CInstanceBase * m_pInstTarget;

		void __BettingGuildWar_Initialize();
		void __BettingGuildWar_SetObserverCount(UINT uObserverCount);
		void __BettingGuildWar_SetBettingMoney(UINT uBettingMoney);

#ifdef ENABLE_GIFTBOX_MULTI_OPEN
	protected:
		bool RecvRefreshInventoryPacket();
#endif

#ifdef ENABLE_SWITCHBOT
	public:
		bool RecvSwitchbotPacket();

		bool SendSwitchbotStartPacket(BYTE slot, std::vector<CPythonSwitchbot::TSwitchbotAttributeAlternativeTable> alternatives);
		bool SendSwitchbotStopPacket(BYTE slot);
#endif

#ifdef ENABLE_MULTI_LANGUAGE_WHISPER_DETAILS
	public:
		bool SendWhisperDetails(const char* name);
		bool RecvWhisperDetails();
#endif

#ifdef ENABLE_RIDING_EXTENDED
	public:
		bool MountUpGrade(const uint8_t iSubHeader, const uint32_t arg = 0);
		bool RecvMountUpGrade();
		bool RecvMountUpGradeChat();
#endif

#if defined(ENABLE_CLIENT_TIMER)
	public:
		bool RecvClientTimer();
#endif

#ifdef ENABLE_TITLE_SYSTEM
	public:
		bool SendTitleEquipPacket(DWORD dwTitleID);
	protected:
		bool RecvTitleDataPacket();
		bool RecvTitleUpdatePacket();
#endif
};
