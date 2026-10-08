#pragma once

class CPythonSystem : public CSingleton<CPythonSystem>
{
	public:
		enum EWindow
		{
			WINDOW_STATUS,
			WINDOW_INVENTORY,
			WINDOW_ABILITY,
			WINDOW_SOCIETY,
			WINDOW_JOURNAL,
			WINDOW_COMMAND,

			WINDOW_QUICK,
			WINDOW_GAUGE,
			WINDOW_MINIMAP,
			WINDOW_CHAT,

			WINDOW_MAX_NUM,
		};

		enum
		{
			FREQUENCY_MAX_NUM  = 30,
			RESOLUTION_MAX_NUM = 100
		};

		typedef struct SResolution
		{
			DWORD	width;
			DWORD	height;
			DWORD	bpp;		// bits per pixel (high-color = 16bpp, true-color = 32bpp)

			DWORD	frequency[20];
			BYTE	frequency_count;
		} TResolution;

		typedef struct SWindowStatus
		{
			int		isVisible;
			int		isMinimized;

			int		ixPosition;
			int		iyPosition;
			int		iHeight;
		} TWindowStatus;

		typedef struct SConfig
		{
#ifdef ENABLE_GPU_CONFIG
			std::string		sGPU;
#endif
			DWORD			width;
			DWORD			height;
			DWORD			bpp;
			DWORD			frequency;

			bool			is_software_cursor;
			bool			is_object_culling;
			int				iDistance;
#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
			int				iShadowTargetLevel;
			int				iShadowQualityLevel;
#else
			int				iShadowLevel;
#endif

			FLOAT			music_volume;
			BYTE			voice_volume;

			int				gamma;

			int				isSaveID;
			char			SaveID[20];

#ifdef ENABLE_TRANSLATOR_GOOGLE_SYSTEM
			char			TransLangKey[5];
#endif

			bool			bWindowed;
			bool			bDecompressDDS;
			bool			bNoSoundCard;
			bool			bUseDefaultIME;
			BYTE			bSoftwareTiling;
			bool			bViewChat;
#ifdef ENABLE_REFINE_RENEWAL
			bool			bRefineStatus;
			bool			bRefineStatusFlag;
#endif
			bool			bAlwaysShowName;
			bool			bShowDamage;
			bool			bShowSalesText;
			bool			bSnowTexturesMode;
#ifdef ENABLE_AUTO_SYSTEM
			bool			bAutoHuntStone;
			bool			bAutoHuntPawn;
			bool			bAutoHuntSPawn;
			bool			bAutoHuntKnight;
			bool			bAutoHuntSKnight;
			bool			bAutoHuntBoss;
			bool			bAutoHuntKing;
			int				iAutoHuntLevelMin;
			int				iAutoHuntLevelMax;
#endif
#ifdef ENABLE_PREMIUM_AFFECT_SYSTEM
			bool			bShowPremiumAffect;
#endif
#ifdef ENABLE_DAMAGE_BAR
			bool			bShowDamageBar;
#endif
#ifdef ENABLE_TIME_SYSTEM
			bool			bShowTimeSystem;
#endif
#ifdef ENABLE_INVENTORY_ADDITION
			bool			bShowInventoryAddition;
#endif
#ifdef ENABLE_PACKET_INFO_SYSTEM
			bool			bShowInfoWindow;
#endif
#ifdef ENABLE_FOG_FIX
			bool			bShowFogMode;
#endif
#ifdef ENABLE_DICE_SYSTEM
			bool			bDiceFlag;
#endif
#if defined(ENABLE_SHOW_MOB_INFO)
			bool			bShowMobLevel;
			bool			bShowMobAIFlag;
#endif
#ifdef ENABLE_TITLE_SYSTEM
			bool			bShowTitle;
#endif
#ifdef ENABLE_AUTO_PICKUP_SYSTEM
			bool			bAutoPickup;
#endif
#ifdef ENABLE_FOV_OPTION
			bool			bExtendedFOV;
#endif
#if defined(ENABLE_ENVIRONMENT_EFFECT_OPTION)
			bool			bShowNightMode;
			bool			bShowSnowMode;
			bool			bShowSnowTextureMode;
#endif
#ifdef ENABLE_GRAPHIC_ON_OFF
			int				iEffectLevel;
			// int				iPrivateShopLevel;
			int				iDropItemLevel;

			bool			bPetStatus;
			bool			bNpcNameStatus;
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
			bool			bShowFlag;
#endif
			bool			bShowHighlight;
#ifdef ENABLE_OFFLINESHOP_SYSTEM
			BYTE			shopnames_range;
#endif
#ifdef ENABLE_SAVE_CAMERA_MODE
			BYTE			bCameraMode;
#endif
#ifdef ENABLE_ANIMATION_OPTIMIZATION
			bool			bShowOtherCharAttacked;
#endif
#ifdef ENABLE_FAST_ITEM_DELETE_SYSTEM
			bool			bAutoSellStatus;
#endif
		} TConfig;

	public:
		CPythonSystem();
		virtual ~CPythonSystem();

		void Clear();
		void SetInterfaceHandler(PyObject * poHandler);
		void DestroyInterfaceHandler();

		// Config
		void							SetDefaultConfig();
		bool							LoadConfig();
		bool							SaveConfig();
		void							ApplyConfig();
		void							SetConfig(TConfig * set_config);
		TConfig *						GetConfig();
		void							ChangeSystem();

		// Interface
		bool							LoadInterfaceStatus();
		void							SaveInterfaceStatus();
		bool							isInterfaceConfig();
		const TWindowStatus &			GetWindowStatusReference(int iIndex);

		DWORD							GetWidth();
		DWORD							GetHeight();
		void							SetResolution(DWORD width, DWORD height) { m_Config.width = width; m_Config.height = height; }
		DWORD							GetBPP();
		DWORD							GetFrequency();
		bool							IsSoftwareCursor();
		bool							IsWindowed();
		bool							IsViewChat();
		bool							IsAlwaysShowName();
		bool							IsShowDamage();
		bool							IsShowSalesText();
#ifdef ENABLE_REFINE_RENEWAL	
		bool							IsRefineStatusShow();
		void							SetRefineStatus(int iFlag);
#endif
		bool							IsUseDefaultIME();
		bool							IsNoSoundCard();
		bool							IsAutoTiling();
		bool							IsSoftwareTiling();
		void							SetSoftwareTiling(bool isEnable);
		void							SetViewChatFlag(int iFlag);
		void							SetAlwaysShowNameFlag(int iFlag);
		void							SetShowDamageFlag(int iFlag);
		void							SetShowSalesTextFlag(int iFlag);
		bool							IsSnowTexturesMode();
#ifdef ENABLE_FOG_FIX
		void							SetFogModeOption(int iFlag);
		bool							GetFogModeOption();
#endif
#ifdef ENABLE_AUTO_SYSTEM
		bool							IsAutoHuntStone();
		bool							SetAutoHuntStoneFlag(int iFlag);			// returns false if locked by another client
		bool							IsAutoHuntPawn();
		void							SetAutoHuntPawnFlag(int iFlag);
		bool							IsAutoHuntSPawn();
		void							SetAutoHuntSPawnFlag(int iFlag);
		bool							IsAutoHuntKnight();
		void							SetAutoHuntKnightFlag(int iFlag);
		bool							IsAutoHuntSKnight();
		void							SetAutoHuntSKnightFlag(int iFlag);
		bool							IsAutoHuntBoss();
		void							SetAutoHuntBossFlag(int iFlag);
		bool							IsAutoHuntKing();
		void							SetAutoHuntKingFlag(int iFlag);
		int								GetAutoHuntLevelMin();
		void							SetAutoHuntLevelMin(int iLevel);
		int								GetAutoHuntLevelMax();
		void							SetAutoHuntLevelMax(int iLevel);
#endif
#ifdef ENABLE_PREMIUM_AFFECT_SYSTEM
		void							SetPremiumAffect(int iFlag);
		bool							IsEnablePremiumAffect();
#endif
#ifdef ENABLE_DAMAGE_BAR
		void							SetDamageBar(int iFlag);
		bool							IsEnableDamageBar();
#endif
#ifdef ENABLE_TIME_SYSTEM
		void							SetTimeSystem(int iFlag);
		bool							IsEnableTimeSystem();
#endif
#ifdef ENABLE_INVENTORY_ADDITION
		void							SetInventoryAddition(int iFlag);
		bool							IsEnableInventoryAddition();
#endif
#ifdef ENABLE_PACKET_INFO_SYSTEM
		void							SetInfoWindow(int iFlag);
		bool							IsEnableInfoWindow();
#endif
#if defined(ENABLE_SHOW_MOB_INFO)
		void							SetShowMobAIFlag(int iFlag);
		bool							IsShowMobAIFlag();
		void							SetShowMobLevel(int iFlag);
		bool							IsShowMobLevel();
#endif
#ifdef ENABLE_TITLE_SYSTEM
		bool							IsShowTitle();
		void							SetShowTitle(bool bFlag);
#endif
#ifdef ENABLE_AUTO_PICKUP_SYSTEM
		void							SetAutoPickup(int iFlag);
		bool							IsAutoPickup();
#endif
#ifdef ENABLE_FOV_OPTION
		bool							IsExtendedFOV();
		void							SetExtendedFOV(int iFlag);
#endif
#if defined(ENABLE_ENVIRONMENT_EFFECT_OPTION)
		void							SetNightModeOption(int iFlag);
		bool							GetNightModeOption();
		void							SetSnowModeOption(int iFlag);
		bool							GetSnowModeOption();
		void							SetSnowTextureModeOption(int iFlag);
		bool							GetSnowTextureModeOption();
#endif
#ifdef ENABLE_GRAPHIC_ON_OFF
		int								GetEffectLevel();
		void							SetEffectLevel(unsigned int level);
		int								GetDropItemLevel();
		void							SetDropItemLevel(unsigned int level);
		bool							IsPetStatus();
		void							SetPetStatusFlag(int iFlag);
		bool							IsNpcNameStatus();
		void							SetNpcNameStatusFlag(int iFlag);
#endif
#ifdef ENABLE_DICE_SYSTEM
		void							SetDiceChatShow(int iFlag);
		bool							IsDiceChatShow();
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		bool							IsShowFlag();
		void							SetShowFlag(int iFlag);
#endif
		void							SetShowHighlightFlag(int iFlag);

		// Window
		void							SaveWindowStatus(int iIndex, int iVisible, int iMinimized, int ix, int iy, int iHeight);

		// SaveID
		int								IsSaveID();
		const char *					GetSaveID();
		void							SetSaveID(int iValue, const char * c_szSaveID);

#ifdef ENABLE_TRANSLATOR_GOOGLE_SYSTEM
		const char *					GetTransLangKey();
		void							SetTransLangKey(const char * c_szTransLangKey);
#endif

		/// Display
		void							GetDisplaySettings();

		int								GetResolutionCount();
		int								GetFrequencyCount(int index);
		bool							GetResolution(int index, OUT DWORD *width, OUT DWORD *height, OUT DWORD *bpp);
		bool							GetFrequency(int index, int freq_index, OUT DWORD *frequncy);
		int								GetResolutionIndex(DWORD width, DWORD height, DWORD bpp);
		int								GetFrequencyIndex(int res_index, DWORD frequency);
		bool							isViewCulling();

		// Sound
		float							GetMusicVolume();
		int								GetSoundVolume();
		void							SetMusicVolume(float fVolume);
		void							SetSoundVolumef(float fVolume);

		int								GetDistance();
#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
		int								GetShadowTargetLevel();
		void							SetShadowTargetLevel(unsigned int level);
		int								GetShadowQualityLevel();
		void							SetShadowQualityLevel(unsigned int level);
#else
		int								GetShadowLevel() const;
		void							SetShadowLevel(unsigned int level);
#endif

#ifdef ENABLE_OFFLINESHOP_SYSTEM
		void							SetShopNamesRange(BYTE iIndex) { m_Config.shopnames_range = iIndex; }
		BYTE							GetShopNamesRange() { return m_Config.shopnames_range; }
#endif

#ifdef ENABLE_SAVE_CAMERA_MODE
		void							SetCameraMode(BYTE bMode);
		BYTE							GetCameraMode() const;
#endif

#ifdef ENABLE_ANIMATION_OPTIMIZATION
		void							SetShowOtherCharAttacked(bool bEnable);
		bool							IsShowOtherCharAttacked() const;
#endif

#ifdef ENABLE_FAST_ITEM_DELETE_SYSTEM
		bool							IsAutoSellStatus();
		void							SetAutoSellStatus(int iFlag);
#endif

	protected:
#ifdef ENABLE_MULTI_FARM_BLOCK
		HANDLE							m_hAutoStoneMutex;
#endif
		TResolution						m_ResolutionList[RESOLUTION_MAX_NUM];
		int								m_ResolutionCount;

		TConfig							m_Config;
		TConfig							m_OldConfig;

		bool							m_isInterfaceConfig;
		PyObject *						m_poInterfaceHandler;
		TWindowStatus					m_WindowStatus[WINDOW_MAX_NUM];

#ifdef ENABLE_GPU_CONFIG
	public:
		const std::string&				GetGPU() const;
		void							SetGPU(const std::string& gpu);
#endif
};
