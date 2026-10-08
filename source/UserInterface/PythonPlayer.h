#pragma once

#include "AbstractPlayer.h"
#include "Packet.h"
#include "PythonSkill.h"

#ifdef ENABLE_INGAME_WIKI
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
#ifdef ENABLE_WOLFMAN_CHARACTER
	MAIN_RACE_WOLFMAN_M,
#endif
	MAIN_RACE_MAX_NUM,
};
#endif

class CInstanceBase;

/*
 *	ë©”ì?¸ ìº?ë¦­í„° (ì??ì‹ ì?´ ì¡°ì •í•˜ëŠ” ìº?ë¦­í„°) ê°€ ê°€ì§„ ì •ë³´ë“¤ì?„ ê´€ë¦¬í•œë‹¤.
 *
 * 2003-01-12 Levites	ë³¸ë?˜ëŠ” CPythonCharacterê°€ ê°€ì§€ê³  ì?ˆì—ˆì§€ë§Œ ê·œëª¨ê°€ ë„ˆë¬´ ì»¤ì ¸ ë²„ë¦°ë?°ë‹¤
 *						ìœ„ì¹˜ë?„ ì• ë§¤í•´ì„œ ë”°ë¡œ ë¶„ë¦¬
 * 2003-07-19 Levites	ë©”ì?¸ ìº?ë¦­í„°ì?˜ ì?´ë?™ ì²˜ë¦¬ CharacterInstanceì—?ì„œ ë–¼ì–´ë‹¤ ë¶™ì?„
 *						ê¸°ì¡´ì?˜ ë?°ì?´íƒ€ ë³´ì¡´ì?˜ ì—­í• ì—?ì„œ ì™„ë²½í•œ ë©”ì?¸ í”Œë ˆì?´ì–´ ì œì–´ í?´ë?˜ìŠ¤ë¡œ
 *						íƒˆë°”ê¿ˆ í•¨.
 */

class CPythonPlayer : public CSingleton<CPythonPlayer>, public IAbstractPlayer
{
	public:
		enum
		{
			CATEGORY_NONE		= 0,
			CATEGORY_ACTIVE		= 1,
			CATEGORY_PASSIVE	= 2,
			CATEGORY_MAX_NUM	= 3,

			STATUS_INDEX_ST = 1,
			STATUS_INDEX_DX = 2,
			STATUS_INDEX_IQ = 3,
			STATUS_INDEX_HT = 4,
		};

		enum
		{
			MBT_LEFT,
			MBT_RIGHT,
			MBT_MIDDLE,
			MBT_NUM,
		};

		enum
		{
			MBF_SMART,
			MBF_MOVE,
			MBF_CAMERA,
			MBF_ATTACK,
			MBF_SKILL,
			MBF_AUTO,
#ifdef ENABLE_AUTO_QUQUE_ATTACK
			MBF_QUEUE_ON,
#endif
		};

		enum
		{
			MBS_CLICK,
			MBS_PRESS,
		};

		enum EMode
		{
			MODE_NONE,
			MODE_CLICK_POSITION,
			MODE_CLICK_ITEM,
			MODE_CLICK_ACTOR,
			MODE_USE_SKILL,
		};

		enum EEffect
		{
			EFFECT_PICK,
			EFFECT_NUM,
		};

		enum EMetinSocketType
		{
			METIN_SOCKET_TYPE_NONE,
			METIN_SOCKET_TYPE_SILVER,
			METIN_SOCKET_TYPE_GOLD,
		};

		typedef struct SSkillInstance
		{
			DWORD dwIndex;
			int iType;
			int iGrade;
			int iLevel;
			float fcurEfficientPercentage;
			float fnextEfficientPercentage;
			BOOL isCoolTime;

			float fCoolTime;			// NOTE : ì¿¨íƒ€ì?„ ì¤‘ì?¸ ìŠ¤í‚¬ ìŠ¬ë¡¯ì?„
			float fLastUsedTime;		//        í€µì°½ì—? ë“±ë¡?í•  ë•Œ ì‚¬ìš©í•˜ëŠ” ë³€ìˆ˜
			BOOL bActive;
		} TSkillInstance;

		enum EKeyBoard_UD
		{
			KEYBOARD_UD_NONE,
			KEYBOARD_UD_UP,
			KEYBOARD_UD_DOWN,
		};

		enum EKeyBoard_LR
		{
			KEYBOARD_LR_NONE,
			KEYBOARD_LR_LEFT,
			KEYBOARD_LR_RIGHT,
		};

		enum
		{
			DIR_UP,
			DIR_DOWN,
			DIR_LEFT,
			DIR_RIGHT,
		};

		typedef struct SPlayerStatus
		{
			TItemData			aItem[c_Inventory_Count];
			TItemData			aDSItem[c_DragonSoul_Inventory_Count];
#ifdef ENABLE_SWITCHBOT
			TItemData			aSwitchbotItem[SWITCHBOT_SLOT_COUNT];
#endif
			TQuickSlot			aQuickSlot[QUICKSLOT_MAX_NUM];
			TSkillInstance		aSkill[SKILL_MAX_NUM];
			long long			m_alPoint[POINT_MAX_NUM];
			long				lQuickPageIndex;
			
			void SetPoint(UINT ePoint, long long lPoint);
			long long GetPoint(UINT ePoint);
#ifdef ENABLE_AUTO_SYSTEM
			TAutoSlot aAutoSlot[AUTO_POSITINO_SLOT_MAX];
#endif
		} TPlayerStatus;

		typedef struct SPartyMemberInfo
		{
			SPartyMemberInfo(DWORD _dwPID, const char * c_szName) : dwPID(_dwPID), strName(c_szName), dwVID(0) {}

			DWORD dwVID;
			DWORD dwPID;
			std::string strName;
			BYTE byState;
			BYTE byHPPercentage;
			short sAffects[PARTY_AFFECT_SLOT_MAX_NUM];
		} TPartyMemberInfo;

		enum EPartyRole
		{
			PARTY_ROLE_NORMAL,
			PARTY_ROLE_LEADER,
			PARTY_ROLE_ATTACKER,
			PARTY_ROLE_TANKER,
			PARTY_ROLE_BUFFER,
			PARTY_ROLE_SKILL_MASTER,
			PARTY_ROLE_BERSERKER,
			PARTY_ROLE_DEFENDER,
			PARTY_ROLE_MAX_NUM,
		};

		enum
		{
			SKILL_NORMAL,
			SKILL_MASTER,
			SKILL_GRAND_MASTER,
			SKILL_PERFECT_MASTER,
		};

		// ì??ë?™ë¬¼ì•½ ìƒ?íƒœ ê´€ë ¨ íŠ¹í™” êµ¬ì¡°ì²´.. ì?´ëŸ°ì‹?ì?˜ íŠ¹í™” ì²˜ë¦¬ ì?‘ì—…ì?„ ì•ˆ í•˜ë ¤ê³  ìµœëŒ€í•œ ë…¸ë ¥í–ˆì§€ë§Œ ì‹¤íŒ¨í•˜ê³  ê²°êµ­ íŠ¹í™”ì²˜ë¦¬.
		struct SAutoPotionInfo
		{
			SAutoPotionInfo() : bActivated(false), totalAmount(0), currentAmount(0) {}

			bool bActivated;					// í™œì„±í™” ë?˜ì—ˆëŠ”ê°€?			
			long currentAmount;					// í˜„ì?¬ ë‚¨ì?€ ì–‘
			long totalAmount;					// ì „ì²´ ì–‘
			long inventorySlotIndex;			// ì‚¬ìš©ì¤‘ì?¸ ì•„ì?´í…œì?˜ ì?¸ë²¤í† ë¦¬ìƒ? ìŠ¬ë¡¯ ì?¸ë?±ìŠ¤
		};

		enum EAutoPotionType
		{
			AUTO_POTION_TYPE_HP = 0,
			AUTO_POTION_TYPE_SP = 1,
			AUTO_POTION_TYPE_NUM
		};

	public:
		CPythonPlayer(void);
		virtual ~CPythonPlayer(void);

		void	PickCloseMoney();
		void	PickCloseItem();

		void	SetGameWindow(PyObject * ppyObject);
		void	OnMobileJoystick(int action, float x, float y);

		void	SetObserverMode(bool isEnable);
		bool	IsObserverMode();

		void	SetQuickCameraMode(bool isEnable);

		void	SetAttackKeyState(bool isPress);
		void	SetMobileMoveDirection(float fDirRot);

		void	NEW_GetMainActorPosition(TPixelPosition* pkPPosActor);

		bool	RegisterEffect(DWORD dwEID, const char* c_szEftFileName, bool isCache);

		bool	NEW_SetMouseState(int eMBType, int eMBState);
		bool	NEW_SetMouseFunc(int eMBType, int eMBFunc);
		int		NEW_GetMouseFunc(int eMBT);
		void	NEW_SetMouseMiddleButtonState(int eMBState);

		void	NEW_SetAutoCameraRotationSpeed(float fRotSpd);
		void	NEW_ResetCameraRotation();

		void	NEW_SetSingleDirKeyState(int eDirKey, bool isPress);
		void	NEW_SetSingleDIKKeyState(int eDIKKey, bool isPress);
		void	NEW_SetMultiDirKeyState(bool isLeft, bool isRight, bool isUp, bool isDown);

		void	NEW_Attack();
		void	NEW_Fishing();
		bool	NEW_CancelFishing();

		void	NEW_LookAtFocusActor();
		bool	NEW_IsAttackableDistanceFocusActor();


		bool	NEW_MoveToDestPixelPositionDirection(const TPixelPosition& c_rkPPosDst);
		bool	NEW_MoveToMousePickedDirection();
		bool	NEW_MoveToMouseScreenDirection();
		bool	NEW_MoveToDirection(float fDirRot);
		void	NEW_Stop();


		// Reserved
		bool	NEW_IsEmptyReservedDelayTime(float fElapsedtime);	// ë„¤ì?´ë°? êµ?ì • ë…¼ì?˜ í•„ìš” - [levites]


		// Dungeon
		void	SetDungeonDestinationPosition(int ix, int iy);
		void	AlarmHaveToGo();


		CInstanceBase* NEW_FindActorPtr(DWORD dwVID);
		CInstanceBase* NEW_GetMainActorPtr();

		// flying target set
		void	Clear();
		void	ClearSkillDict(); // ì—†ì–´ì§€ê±°ë‚˜ ClearGame ìª½ìœ¼ë¡œ í?¬í•¨ë?  í•¨ìˆ˜
		void	NEW_ClearSkillData(bool bAll = false);

		void	Update();


		// Play Time
		DWORD	GetPlayTime();
		void	SetPlayTime(DWORD dwPlayTime);

		// System
		void	SetMainCharacterIndex(int iIndex);

		DWORD	GetMainCharacterIndex();
		bool	IsMainCharacterIndex(DWORD dwIndex);
		DWORD	GetGuildID();
		void	NotifyDeletingCharacterInstance(DWORD dwVID);
		void	NotifyCharacterDead(DWORD dwVID);
		void	NotifyCharacterUpdate(DWORD dwVID);
		void	NotifyDeadMainCharacter();
		void	NotifyChangePKMode();


		// Player Status
		const char *	GetName();
		void	SetName(const char *name);
		
		void	SetRace(DWORD dwRace);
		DWORD	GetRace();

		void	SetWeaponPower(DWORD dwMinPower, DWORD dwMaxPower, DWORD dwMinMagicPower, DWORD dwMaxMagicPower, DWORD dwAddPower);
		void	SetStatus(DWORD dwType, long long lValue);
		long long	GetStatus(DWORD dwType);


		// Item
		void	MoveItemData(TItemPos SrcCell, TItemPos DstCell);
		void	SetItemData(TItemPos Cell, const TItemData & c_rkItemInst);
		const	TItemData * GetItemData(TItemPos Cell) const;
#ifdef ENABLE_EXTENDED_ITEM_COUNT
		void	SetItemCount(TItemPos Cell, WORD byCount, bool uPacketMi = false);
#else
		void	SetItemCount(TItemPos Cell, BYTE byCount);
#endif
#ifdef ENABLE_SOULBIND_SYSTEM
		void	SetItemBind(TItemPos Cell, long lBindTime);
		long	GetItemBind(TItemPos Cell);
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
		void	SetItemTransmutation(TItemPos itemPos, DWORD dwVnum);
		DWORD	GetItemTransmutation(TItemPos itemPos);
#endif
#ifdef ENABLE_SET_ITEM
		void	SetItemSetValue(TItemPos Cell, uint8_t set_value);
		uint8_t	GetItemSetValue(TItemPos Cell);
#endif
		void	SetItemMetinSocket(TItemPos Cell, DWORD dwMetinSocketIndex, DWORD dwMetinNumber);
		void	SetItemAttribute(TItemPos Cell, DWORD dwAttrIndex, BYTE byType, short sValue);
#ifdef ENABLE_GLOVE_SYSTEM
		void	SetItemApplyRandom(TItemPos Cell, DWORD dwApplyIndex, BYTE byType, short sValue);
#endif
		DWORD	GetItemIndex(TItemPos Cell);
		DWORD	GetItemFlags(TItemPos Cell);
		DWORD	GetItemAntiFlags(TItemPos Cell);
		BYTE	GetItemTypeBySlot(TItemPos Cell);
		BYTE	GetItemSubTypeBySlot(TItemPos Cell);
		DWORD	GetItemCount(TItemPos Cell);
#if defined(ENABLE_CUBE_RENEWAL) && defined(ENABLE_SET_ITEM)
		DWORD GetItemCountByVnum(DWORD dwVnum, bool bIgnoreSetValue);
#else
		DWORD GetItemCountByVnum(DWORD dwVnum);
#endif
		DWORD	GetItemMetinSocket(TItemPos Cell, DWORD dwMetinSocketIndex);
		void	GetItemAttribute(TItemPos Cell, DWORD dwAttrSlotIndex, BYTE * pbyType, short * psValue);
#ifdef ENABLE_GLOVE_SYSTEM
		void	GetItemApplyRandom(TItemPos Cell, DWORD dwApplySlotIndex, BYTE* pbyType, short* psValue);
#endif
		void	SendClickItemPacket(DWORD dwIID);

#ifdef ENABLE_REFINE_ELEMENT
		void	SetItemRefineElement(TItemPos itemPos, DWORD dwRefineElement);
		DWORD	GetItemRefineElement(TItemPos itemPos);
#endif

		void	RequestAddLocalQuickSlot(DWORD dwLocalSlotIndex, DWORD dwWndType, DWORD dwWndItemPos);
		void	RequestAddToEmptyLocalQuickSlot(DWORD dwWndType, DWORD dwWndItemPos);
		void	RequestMoveGlobalQuickSlotToLocalQuickSlot(DWORD dwGlobalSrcSlotIndex, DWORD dwLocalDstSlotIndex);
		void	RequestDeleteGlobalQuickSlot(DWORD dwGlobalSlotIndex);
		void	RequestUseLocalQuickSlot(DWORD dwLocalSlotIndex);
		DWORD	LocalQuickSlotIndexToGlobalQuickSlotIndex(DWORD dwLocalSlotIndex);

		void	GetGlobalQuickSlotData(DWORD dwGlobalSlotIndex, DWORD* pdwWndType, DWORD* pdwWndItemPos);
		void	GetLocalQuickSlotData(DWORD dwSlotPos, DWORD* pdwWndType, DWORD* pdwWndItemPos);
		void	RemoveQuickSlotByValue(int iType, int iPosition);

		char	IsItem(TItemPos SlotIndex);

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
		bool	IsBeltInventorySlot(TItemPos Cell);
#endif
		bool	IsInventorySlot(TItemPos SlotIndex);
		bool	IsEquipmentSlot(TItemPos SlotIndex);
#ifdef ENABLE_SPLIT_INVENTORY_SYSTEM
		bool	IsSkillBookInventorySlot(TItemPos Cell);
		bool	IsUpgradeItemsInventorySlot(TItemPos Cell);
		bool	IsStoneInventorySlot(TItemPos Cell);
		bool	IsBoxInventorySlot(TItemPos Cell);
		bool	IsEfsunInventorySlot(TItemPos Cell);
		bool	IsCicekInventorySlot(TItemPos Cell);
#endif
		bool	IsEquipItemInSlot(TItemPos iSlotIndex);

#ifdef ENABLE_EXTEND_INVEN_SYSTEM
		int		GetExtendInvenStage();
		int		GetExtendInvenMax();
		void	SetExtendInvenStage(short inven_stage);
		void	SetExtendInvenMax(short inven_max);
#endif

		// Quickslot
		int		GetQuickPage();
		void	SetQuickPage(int nPageIndex);
		void	AddQuickSlot(int QuickslotIndex, char IconType, INT IconPosition);
		void	DeleteQuickSlot(int QuickslotIndex);
		void	MoveQuickSlot(int Source, int Target);


		// Skill
		void	SetSkill(DWORD dwSlotIndex, DWORD dwSkillIndex);
		bool	GetSkillSlotIndex(DWORD dwSkillIndex, DWORD* pdwSlotIndex);
		int		GetSkillIndex(DWORD dwSlotIndex);
		int		GetSkillGrade(DWORD dwSlotIndex);
		int		GetSkillLevel(DWORD dwSlotIndex);
		float	GetSkillCurrentEfficientPercentage(DWORD dwSlotIndex);
		float	GetSkillNextEfficientPercentage(DWORD dwSlotIndex);
		void	SetSkillLevel(DWORD dwSlotIndex, DWORD dwSkillLevel);
		void	SetSkillLevel_(DWORD dwSkillIndex, DWORD dwSkillGrade, DWORD dwSkillLevel);
		BOOL	IsToggleSkill(DWORD dwSlotIndex);
		void	ClickSkillSlot(DWORD dwSlotIndex);
		void	ChangeCurrentSkillNumberOnly(DWORD dwSlotIndex);
		bool	FindSkillSlotIndexBySkillIndex(DWORD dwSkillIndex, DWORD * pdwSkillSlotIndex);

		void	SetSkillCoolTime(DWORD dwSkillIndex);
		void	EndSkillCoolTime(DWORD dwSkillIndex);

		float	GetSkillCoolTime(DWORD dwSlotIndex);
		float	GetSkillElapsedCoolTime(DWORD dwSlotIndex);
		BOOL	IsSkillActive(DWORD dwSlotIndex);
		BOOL	IsSkillCoolTime(DWORD dwSlotIndex);
		void	UseGuildSkill(DWORD dwSkillSlotIndex);
		bool	AffectIndexToSkillSlotIndex(UINT uAffect, DWORD* pdwSkillSlotIndex);
		bool	AffectIndexToSkillIndex(DWORD dwAffectIndex, DWORD * pdwSkillIndex);

		void	SetAffect(UINT uAffect);
		void	ResetAffect(UINT uAffect);
		void	ClearAffects();


		// Target
		void	SetTarget(DWORD dwVID, BOOL bForceChange = TRUE);
		void	OpenCharacterMenu(DWORD dwVictimActorID);
		DWORD	GetTargetVID();

#ifdef ENABLE_KEYBOARD_SETTINGS_SYSTEM
		void	OpenKeyChangeWindow();
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		void	OpenLanguageChangeWindow();
#endif

		// Party
		void	ExitParty();
		void	AppendPartyMember(DWORD dwPID, const char * c_szName);
		void	LinkPartyMember(DWORD dwPID, DWORD dwVID);
		void	UnlinkPartyMember(DWORD dwPID);
		void	UpdatePartyMemberInfo(DWORD dwPID, BYTE byState, BYTE byHPPercentage);
		void	UpdatePartyMemberAffect(DWORD dwPID, BYTE byAffectSlotIndex, short sAffectNumber);
		void	RemovePartyMember(DWORD dwPID);
		bool	IsPartyMemberByVID(DWORD dwVID);
		bool	IsPartyMemberByName(const char * c_szName);
		bool	GetPartyMemberPtr(DWORD dwPID, TPartyMemberInfo ** ppPartyMemberInfo);
		bool	PartyMemberPIDToVID(DWORD dwPID, DWORD * pdwVID);
		bool	PartyMemberVIDToPID(DWORD dwVID, DWORD * pdwPID);
		bool	IsSamePartyMember(DWORD dwVID1, DWORD dwVID2);

		// Fight
		void	RememberChallengeInstance(DWORD dwVID);
		void	RememberRevengeInstance(DWORD dwVID);
		void	RememberCantFightInstance(DWORD dwVID);
		void	ForgetInstance(DWORD dwVID);
		bool	IsChallengeInstance(DWORD dwVID);
		bool	IsRevengeInstance(DWORD dwVID);
		bool	IsCantFightInstance(DWORD dwVID);

		// Private Shop
		void	OpenPrivateShop();
		void	ClosePrivateShop();
		bool	IsOpenPrivateShop();

		bool	IsDead();
		bool	IsPoly();

		// Stamina
		void	StartStaminaConsume(DWORD dwConsumePerSec, DWORD dwCurrentStamina);
		void	StopStaminaConsume(DWORD dwCurrentStamina);

		// PK Mode
		DWORD	GetPKMode();

		// Mobile
		void	SetMobileFlag(BOOL bFlag);
		BOOL	HasMobilePhoneNumber();

		// Combo
		void	SetComboSkillFlag(BOOL bFlag);

		// System
		void	SetMovableGroundDistance(float fDistance);

		// Emotion
		void	ActEmotion(DWORD dwEmotionID);
		void	StartEmotionProcess();
		void	EndEmotionProcess();

		// Function Only For Console System
		BOOL	__ToggleCoolTime();
		BOOL	__ToggleLevelLimit();

		__inline const	SAutoPotionInfo& GetAutoPotionInfo(int type) const	{ return m_kAutoPotionInfo[type]; }
		__inline		SAutoPotionInfo& GetAutoPotionInfo(int type)		{ return m_kAutoPotionInfo[type]; }
		__inline void					 SetAutoPotionInfo(int type, const SAutoPotionInfo& info)	{ m_kAutoPotionInfo[type] = info; }		

	protected:
		TQuickSlot &	__RefLocalQuickSlot(int SlotIndex);
		TQuickSlot &	__RefGlobalQuickSlot(int SlotIndex);

		DWORD	__GetLevelAtk();
		DWORD	__GetStatAtk();
		DWORD	__GetWeaponAtk(DWORD dwWeaponPower);		
		DWORD	__GetTotalAtk(DWORD dwWeaponPower, DWORD dwRefineBonus);
		DWORD	__GetRaceStat();		
		DWORD	__GetHitRate();
		DWORD	__GetEvadeRate();

		void	__UpdateBattleStatus();

		void	__DeactivateSkillSlot(DWORD dwSlotIndex);
		void	__ActivateSkillSlot(DWORD dwSlotIndex);

		void	__OnPressSmart(CInstanceBase& rkInstMain, bool isAuto);
		void	__OnClickSmart(CInstanceBase& rkInstMain, bool isAuto);

		void	__OnPressItem(CInstanceBase& rkInstMain, DWORD dwPickedItemID);
		void	__OnPressActor(CInstanceBase& rkInstMain, DWORD dwPickedActorID, bool isAuto);
		void	__OnPressGround(CInstanceBase& rkInstMain, const TPixelPosition& c_rkPPosPickedGround);
		void	__OnPressScreen(CInstanceBase& rkInstMain);

		void	__OnClickActor(CInstanceBase& rkInstMain, DWORD dwPickedActorID, bool isAuto);
		void	__OnClickItem(CInstanceBase& rkInstMain, DWORD dwPickedItemID);
		void	__OnClickGround(CInstanceBase& rkInstMain, const TPixelPosition& c_rkPPosPickedGround);

		bool	__IsMovableGroundDistance(CInstanceBase& rkInstMain, const TPixelPosition& c_rkPPosPickedGround);

		bool	__GetPickedActorPtr(CInstanceBase** pkInstPicked);

		bool	__GetPickedActorID(DWORD* pdwActorID);
		bool	__GetPickedItemID(DWORD* pdwItemID);
		bool	__GetPickedGroundPos(TPixelPosition* pkPPosPicked);

		void	__ClearReservedAction();
		void	__ReserveClickItem(DWORD dwItemID);
		void	__ReserveClickActor(DWORD dwActorID);
		void	__ReserveClickGround(const TPixelPosition& c_rkPPosPickedGround);
		void	__ReserveUseSkill(DWORD dwActorID, DWORD dwSkillSlotIndex, DWORD dwRange);

		void	__ReserveProcess_ClickActor();

		void	__ShowPickedEffect(const TPixelPosition& c_rkPPosPickedGround);
		void	__SendClickActorPacket(CInstanceBase& rkInstVictim);

		void	__ClearAutoAttackTargetActorID();
		void	__SetAutoAttackTargetActorID(DWORD dwActorID);

		void	NEW_ShowEffect(int dwEID, TPixelPosition kPPosDst);

		void	NEW_SetMouseSmartState(int eMBS, bool isAuto);
		void	NEW_SetMouseMoveState(int eMBS);
		void	NEW_SetMouseCameraState(int eMBS);
#ifdef ENABLE_AUTO_QUQUE_ATTACK
		void	NEW_SetMouseCameraStateQuque(int eMBS);
#endif
		void	NEW_GetMouseDirRotation(float fScrX, float fScrY, float* pfDirRot);
		void	NEW_GetMultiKeyDirRotation(bool isLeft, bool isRight, bool isUp, bool isDown, float* pfDirRot);

		float	GetDegreeFromDirection(int iUD, int iLR);
		float	GetDegreeFromPosition(int ix, int iy, int iHalfWidth, int iHalfHeight);

		bool	CheckCategory(int iCategory);
		bool	CheckAbilitySlot(int iSlotIndex);

		void	RefreshKeyWalkingDirection();
		void	NEW_RefreshMouseWalkingDirection();


		// Instances
		void	RefreshInstances();

		bool	__CanShot(CInstanceBase& rkInstMain, CInstanceBase& rkInstTarget);
		bool	__CanUseSkill();

		bool	__CanMove();
		
		bool	__CanAttack();
		bool	__CanChangeTarget();

		bool	__CheckSkillUsable(DWORD dwSlotIndex);
		void	__UseCurrentSkill();
		void	__UseChargeSkill(DWORD dwSkillSlotIndex);
		bool	__UseSkill(DWORD dwSlotIndex);
		bool	__CheckSpecialSkill(DWORD dwSkillIndex);

		bool	__CheckRestSkillCoolTime(DWORD dwSkillSlotIndex);
		bool	__CheckShortLife(TSkillInstance & rkSkillInst, CPythonSkill::TSkillData& rkSkillData);
		bool	__CheckShortMana(TSkillInstance & rkSkillInst, CPythonSkill::TSkillData& rkSkillData);
		bool	__CheckShortArrow(TSkillInstance & rkSkillInst, CPythonSkill::TSkillData& rkSkillData);
		bool	__CheckDashAffect(CInstanceBase& rkInstMain);

		void	__SendUseSkill(DWORD dwSkillSlotIndex, DWORD dwTargetVID);
		void	__RunCoolTime(DWORD dwSkillSlotIndex);

		BYTE	__GetSkillType(DWORD dwSkillSlotIndex);

		bool	__IsReservedUseSkill(DWORD dwSkillSlotIndex);
		bool	__IsMeleeSkill(CPythonSkill::TSkillData& rkSkillData);
		bool	__IsChargeSkill(CPythonSkill::TSkillData& rkSkillData);
		DWORD	__GetSkillTargetRange(CPythonSkill::TSkillData& rkSkillData);
		bool	__SearchNearTarget();
		bool	__IsUsingChargeSkill();

		bool	__ProcessEnemySkillTargetRange(CInstanceBase& rkInstMain, CInstanceBase& rkInstTarget, CPythonSkill::TSkillData& rkSkillData, DWORD dwSkillSlotIndex);

		// Item
		bool	__HasEnoughArrow();
		bool	__HasItem(DWORD dwItemID);
		DWORD	__GetPickableDistance();

		// Target
		CInstanceBase*		__GetTargetActorPtr();
		void				__ClearTarget();
		DWORD				__GetTargetVID();
		void				__SetTargetVID(DWORD dwVID);
		bool				__IsSameTargetVID(DWORD dwVID);
		bool				__IsTarget();
		bool				__ChangeTargetToPickedInstance();

		CInstanceBase *		__GetSkillTargetInstancePtr(CPythonSkill::TSkillData& rkSkillData);
		CInstanceBase *		__GetAliveTargetInstancePtr();
		CInstanceBase *		__GetDeadTargetInstancePtr();

		BOOL				__IsRightButtonSkillMode();

#ifdef ENABLE_AUTO_QUQUE_ATTACK
		bool				__IsLeftButtonQueueMode();
#endif

		// Update
		void				__Update_AutoAttack();
		void				__Update_NotifyGuildAreaEvent();

		// Emotion
		BOOL				__IsProcessingEmotion();

	protected:
		PyObject *				m_ppyGameWindow;

		// Client Player Data
		std::map<DWORD, DWORD>	m_skillSlotDict;
		std::string				m_stName;
		DWORD					m_dwMainCharacterIndex;		
		DWORD					m_dwRace;
		DWORD					m_dwWeaponMinPower;
		DWORD					m_dwWeaponMaxPower;
		DWORD					m_dwWeaponMinMagicPower;
		DWORD					m_dwWeaponMaxMagicPower;
		DWORD					m_dwWeaponAddPower;

		// Todo
		DWORD					m_dwSendingTargetVID;
		float					m_fTargetUpdateTime;

		// Attack
		DWORD					m_dwAutoAttackTargetVID;

		// NEW_Move
		EMode					m_eReservedMode;
		float					m_fReservedDelayTime;

		float					m_fMovDirRot;

		bool					m_isUp;
		bool					m_isDown;
		bool					m_isLeft;
		bool					m_isRight;
		bool					m_isAtkKey;
		bool					m_isDirKey;
		bool					m_bMobileDirActive;
		float					m_fMobileDirRot;
		bool					m_isCmrRot;
		bool					m_isSmtMov;
		bool					m_isDirMov;

		float					m_fCmrRotSpd;

		TPlayerStatus			m_playerStatus;

		UINT					m_iComboOld;
		DWORD					m_dwVIDReserved;
		DWORD					m_dwIIDReserved;

		DWORD					m_dwcurSkillSlotIndex;
		DWORD					m_dwSkillSlotIndexReserved;
		DWORD					m_dwSkillRangeReserved;

		TPixelPosition			m_kPPosInstPrev;
		TPixelPosition			m_kPPosReserved;

		// Emotion
		BOOL					m_bisProcessingEmotion;

		// Dungeon
		BOOL					m_isDestPosition;
		int						m_ixDestPos;
		int						m_iyDestPos;
		int						m_iLastAlarmTime;

		// Party
		std::map<DWORD, TPartyMemberInfo>	m_PartyMemberMap;

		// PVP
		std::set<DWORD>			m_ChallengeInstanceSet;
		std::set<DWORD>			m_RevengeInstanceSet;
		std::set<DWORD>			m_CantFightInstanceSet;

		// Private Shop
		bool					m_isOpenPrivateShop;
		bool					m_isObserverMode;

		// Stamina
		BOOL					m_isConsumingStamina;
		float					m_fCurrentStamina;
		float					m_fConsumeStaminaPerSec;

		// Guild
		DWORD					m_inGuildAreaID;

		// Mobile
		BOOL					m_bMobileFlag;

		// System
		BOOL					m_sysIsCoolTime;
		BOOL					m_sysIsLevelLimit;

	protected:
		// Game Cursor Data
		TPixelPosition			m_MovingCursorPosition;
		float					m_fMovingCursorSettingTime;
		DWORD					m_adwEffect[EFFECT_NUM];

		DWORD					m_dwVIDPicked;
		DWORD					m_dwIIDPicked;
		int						m_aeMBFButton[MBT_NUM];

		DWORD					m_dwTargetVID;
		DWORD					m_dwTargetEndTime;
		DWORD					m_dwPlayTime;

		long long				m_llCurrentOfflineShopMoney;
		DWORD					m_dwCurrentOfflineShopCheque;

#ifdef ENABLE_EXTEND_INVEN_SYSTEM
		DWORD					m_exInvenStage;
		DWORD					m_exInvenMax;
#endif

		SAutoPotionInfo			m_kAutoPotionInfo[AUTO_POTION_TYPE_NUM];

	protected:
		float					MOVABLE_GROUND_DISTANCE;

	private:
		std::map<DWORD, DWORD> m_kMap_dwAffectIndexToSkillIndex;

#ifdef ENABLE_AUTO_SYSTEM
	protected:
		typedef std::vector<TPacketAffectElement> TAffectDataVector;
		TAffectDataVector	m_mapAffectData;

	public:
		void	AddAffect(DWORD dwType, TPacketAffectElement kElem);
		void	RemoveAffect(DWORD dwType, BYTE bApplyOn);
		int		GetAffectDataIndex(DWORD dwType, BYTE bApplyOn);
		TPacketAffectElement GetAffectData(DWORD dwType, BYTE bApplyOn);
		int GetAffectDuration(uint32_t dwType);

		using TAffectDataVector = std::vector<TPacketAffectElement>;
		TAffectDataVector GetAffectDataVector(DWORD dwType);

	protected:
		TAffectDataVector m_vecAffectData;

	public:
		TAutoSlot& AutoSlotData(int iSlotIndex);
	
		void SetAutoSlotCoolTime(int otoAvSlotIndex, DWORD dSuresi);
		void GetAutoSlotIndex(DWORD dwSlotPos, DWORD* dwVnum, DWORD* fillingTime);
		void SetLastAttackTime(DWORD dwTime) { findTargetMs = dwTime; }
	
		void SetAutoSkillSlotIndex(int iSlotIndex, DWORD dwIndex);
		void SetAutoPositionSlotIndex(int iSlotIndex, DWORD dwIndex);
	
		int CheckSkillSlotCoolTime(uint8_t bIndex, int iSlotIndex, int iCoolTime);
		int CheckPositionSlotCoolTime(uint8_t bIndex, int iSlotIndex, int iCoolTime);
	
		bool AutoStatus() { return autoStatus; }
		void StartAuto(bool gelenDurum) { autoStart = gelenDurum; }
		bool CanStartAuto() { return autoStart; }
	
		void SetAutoPause(bool val) { autoPause = val; }
		bool GetAutoPause() { return autoPause; }
	
		TPixelPosition AutoHuntGetStartLocation() { return autohuntStartLocation; }
	
		void AutoHuntTimer(bool beceriSlot);
		void UseAutoSkill(DWORD dwSlotIndex, long iTime, int slotIndex);
		void AutoReserveUse();/*0 red 1 blue*/
		bool AutoBarrierCheck(CInstanceBase* pTarget, CInstanceBase* pInstance);
		void AutoSlotControl(DWORD slotIndex);
	
		void ClearAutoSKillSlot();
		void ClearAutoPositionSlot(int autoSlotIndex = AUTO_POSITINO_SLOT_MAX, bool manual = false);
		void ClearAutoAllSlot();
	
		void AutoStartOnOff(bool gelenDurum);
		void AutoAttackOnOff(bool gelenDurum) { autoAttackOnOff = gelenDurum; }
		bool GetAutoAttackOnOff() { return autoAttackOnOff; }
	
		void AutoSkillOnOff(bool gelenDurum) { autoSkillOnOff = gelenDurum; }
		bool GetAutoSkillOnOff() { return autoSkillOnOff; }
	
		void AutoPositionOnOff(bool gelenDurum) { autoPositionsOnOff = gelenDurum; }
		bool GetAutoPositionOnOff() { return autoPositionsOnOff; }
	
		void AutoRangeOnOff(bool gelenDurum) { autoRangeOnOff = gelenDurum; }
		bool GetAutoRangeOnOff() { return autoRangeOnOff; }
	
		void SetAutoRestart(bool gelenDurum) { AutoRestart = gelenDurum; }
		bool GetAutoRestart() { return AutoRestart; }
		long	sonHedefBulMs;

		void SetAutoHuntRange(float fRange);
		void SetAutoHuntRangeEffect(float fRange);
		float GetAutoHuntRange() const { return m_fAutoHuntRange; }

	protected:
		void UpdateAuto();
	
		bool autoStatus;
		bool autoStart;
		bool autoPause;
		DWORD findTargetMs;
		TPixelPosition autohuntStartLocation;
	
		long zLastSec;
		long kpLastMs;
		long mpLastMs;
		bool auto_HP;
		bool autp_MP;
		long affect_control;
	
		bool autoAttackOnOff;
		bool autoSkillOnOff;
		bool autoPositionsOnOff;
		bool autoRangeOnOff;
		bool AutoRestart;
		DWORD AutoHuntDodgeTime;

	private:
		float m_fAutoHuntRange = 5000.0f;
		DWORD m_dwAutoHuntRangeEffectID = 0;
#endif

#ifdef ENABLE_BATTLE_ROYALE
	public:
		void SetBattleRoyaleEnable(bool enable) { m_bIsBattleRoyaleEnabled = enable; }
		bool GetBattleRoyaleEnable() const { return m_bIsBattleRoyaleEnabled; }

	private:
		bool					m_bIsBattleRoyaleEnabled;
#endif

#ifdef ENABLE_BALATHOR_DUNGEON
	public:
		const char* GetBalathorEffectFileName(const BYTE bEffIndex);
		void CheckBalathorEffect(CInstanceBase* pInstance);
		void AddBalathorEffect(DWORD dwID, DWORD dwVID, BYTE bEffIndex, float fX, float fY, float fZ, float fRotation, bool bIsMain);
		void RemoveBalathorEffect(DWORD dwID = 0);
		void UpdateBalathorEffect();
		const TBalathorEffect* GetBalathorEffect(DWORD dwID);
	protected:
		std::vector<TBalathorEffect> m_vecEffects;
#endif

#ifdef ENABLE_OFFICAL_FEATURES
	protected:
		bool m_isOpenSafeBox; // Safebox
		bool m_isOpenMall; // Mall
	
	public:
		// Safebox
		void SetOpenSafeBox(bool isOpen) { m_isOpenSafeBox = isOpen; };
		bool IsOpenSafeBox() const { return m_isOpenSafeBox; };
	
		// Mall
		void SetOpenMall(bool isOpen) { m_isOpenMall = isOpen; };
		bool IsOpenMall() const { return m_isOpenMall; };
#endif

#ifdef ENABLE_AUTO_QUQUE_ATTACK
	public:
		void		AutoFarmLoop();
		bool		AutoFarmQuqueSet(const bool isAdd, const DWORD dwVirtualID);
		DWORD		GetAutoFarmTarget();
		void		SetTotalAutoFarmCount(const BYTE bCount) { bTotalQuqueAutoAttack = bCount; }
		size_t		GetAutoFarmQueueSize() const { return m_vecQuqueAutoAttack.size(); }
		bool		IsAutoFarmQueued(const DWORD dwVirtualID) const {
			return std::find(m_vecQuqueAutoAttack.begin(), m_vecQuqueAutoAttack.end(), dwVirtualID) != m_vecQuqueAutoAttack.end();
		}
		void		ClearAutoFarmQueue() { AutoFarmQuqueSet(false, 0); }
	protected:
		BYTE		bTotalQuqueAutoAttack;
		std::vector<DWORD> m_vecQuqueAutoAttack;
#endif

#ifdef ENABLE_LEFT_SEAT
	public:
		void LoadLeftSeatData();
#endif

#ifdef ENABLE_QUICKSLOT_IMPROVE
	public:
		void RequestAddGlobalQuickSlot(DWORD dwGlobalSlotIndex, BYTE byWndType, WORD wWndItemPos);
		void RequestMoveGlobalQuickSlot(DWORD dwGlobalSrcSlotIndex, DWORD dwGlobalDstSlotIndex);
		DWORD QuickPageLocalIndexToGlobalQuickSlotIndex(DWORD dwPageIndex, DWORD dwLocalSlotIndex);
#endif
};

extern const int c_iFastestSendingCount;
extern const int c_iSlowestSendingCount;
extern const float c_fFastestSendingDelay;
extern const float c_fSlowestSendingDelay;
extern const float c_fRotatingStepTime;

extern const float c_fComboDistance;
extern const float c_fPickupDistance;
extern const float c_fClickDistance;
