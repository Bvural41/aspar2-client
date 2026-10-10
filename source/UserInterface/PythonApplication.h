#pragma once
#include "../EterLib/MSApplication.h"
#include "../EterLib/SDLInputMapping.h"
#include "../EterLib/Input.h"
#include "../EterLib/Profiler.h"
#include "../EterLib/GrpDevice.h"
#include "../EterLib/NetDevice.h"
#include "../EterLib/GrpLightManager.h"
#include "../EffectLib/EffectManager.h"
#include "../GameLib/RaceManager.h"
#include "../GameLib/ItemManager.h"
#include "../GameLib/FlyingObjectManager.h"
#include "../GameLib/GameEventManager.h"
#include "../MilesLib/SoundManager.h"
#include "PythonEventManager.h"
#include "PythonPlayer.h"
#include "PythonNonPlayer.h"
#include "PythonMiniMap.h"
#include "PythonIME.h"
#include "PythonItem.h"
#include "PythonShop.h"
#include "PythonExchange.h"
#include "PythonChat.h"
#include "PythonTextTail.h"
#include "PythonSkill.h"
#include "PythonSystem.h"
//#include "PythonNetworkDatagram.h"
#include "PythonNetworkStream.h"
#include "PythonCharacterManager.h"
#include "PythonQuest.h"
#include "PythonMessenger.h"
#ifdef ENABLE_MESSENGER_RENEWAL
#include "PythonCommunity.h"
#endif
#include "PythonSafeBox.h"
#include "PythonGuild.h"
#include "GuildMarkDownloader.h"
#include "GuildMarkUploader.h"
#include "AccountConnector.h"
#include "ServerStateChecker.h"
#if defined(__ANDROID__)
#include "AndroidTouchControls.h"
#endif
#include "AbstractApplication.h"

#include "../EterLib/CRenderTarget.h"
#include "../EterLib/CRenderTargetManager.h"

#ifdef ENABLE_SASH_SYSTEM
#include "PythonSash.h"
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
#include "PythonChangeLook.h"
#endif
#ifdef ENABLE_AURA_SYSTEM
#include "PythonAura.h"
#endif
#ifdef ENABLE_CUBE_RENEWAL
#include "PythonCubeRenewal.h"
#endif
#ifdef ENABLE_EVENT_SYSTEM
#include "PythonGameEvents.h"
#endif
#ifdef ENABLE_SHOP_SEARCH_SYSTEM
#include "PythonPrivateShopSearch.h"
#endif
#ifdef ENABLE_RIDING_EXTENDED
#include "PythonMountUpGrade.h"
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
#include "PythonSkillPet.h"
#endif
#ifdef ENABLE_BATTLEPASS
#include "BattlePassSystem.hpp"
#endif
#ifdef ENABLE_BATTLE_ROYALE
#include "PythonBattleRoyaleManager.h"
#endif
#ifdef ENABLE_TREASURE_EVENT
#include "PythonTreasureEvent.h"
#endif
#ifdef ENABLE_TITLE_SYSTEM
#include "PythonTitleManager.h"
#endif
#ifdef ENABLE_INGAME_WIKI
#include "PythonWikiRenderTarget.h"
#include "PythonWiki.h"
#endif

#define CAMERA_PERSPECTIVE 30.0f // 30.0f // 75.0f

class CPythonApplication : public CMSApplication, public CInputKeyboard, public IAbstractApplication
{
	public:
		enum EDeviceState
		{
			DEVICE_STATE_FALSE,
			DEVICE_STATE_SKIP,
			DEVICE_STATE_OK,
		};

#ifdef ENABLE_OFFICAL_FEATURES
		enum EVersion
		{
			VERSION_NUMBER = 49,
		};

		enum ELoginInfo
		{
			LOGIN_TYPE_NONE,
			LOGIN_TYPE_STEAM,
			LOGIN_TYPE_TNT
		};
#endif

		enum ECursorMode
		{
			CURSOR_MODE_HARDWARE,
			CURSOR_MODE_SOFTWARE,
		};

		enum ECursorShape
		{
			CURSOR_SHAPE_NORMAL,
			CURSOR_SHAPE_ATTACK,
			CURSOR_SHAPE_TARGET,
			CURSOR_SHAPE_TALK,
			CURSOR_SHAPE_CANT_GO,
			CURSOR_SHAPE_PICK,

			CURSOR_SHAPE_DOOR,
			CURSOR_SHAPE_CHAIR,
			CURSOR_SHAPE_MAGIC,				// Magic
			CURSOR_SHAPE_BUY,				// Buy
			CURSOR_SHAPE_SELL,				// Sell

			CURSOR_SHAPE_CAMERA_ROTATE,		// Camera Rotate
			CURSOR_SHAPE_HSIZE,				// Horizontal Size
			CURSOR_SHAPE_VSIZE,				// Vertical Size
			CURSOR_SHAPE_HVSIZE,			// Horizontal & Vertical Size
#ifdef ENABLE_TRANSLATOR_GOOGLE_SYSTEM
			CURSOR_SHAPE_TRANS,				// Translator Size
#endif
#ifdef ENABLE_FISHING_RENEWAL
			CURSOR_SHAPE_FISH,
#endif
			CURSOR_SHAPE_COUNT,

			// 안정적인 네이밍 변환을 위한 임시 enumerate
			NORMAL = CURSOR_SHAPE_NORMAL,
			ATTACK = CURSOR_SHAPE_ATTACK,
			TARGET = CURSOR_SHAPE_TARGET,
			CAMERA_ROTATE = CURSOR_SHAPE_CAMERA_ROTATE,
			CURSOR_COUNT = CURSOR_SHAPE_COUNT,
		};

		enum EInfo
		{
			INFO_ACTOR,
			INFO_EFFECT,
			INFO_ITEM,
			INFO_TEXTTAIL,
		};

		enum ECameraControlDirection
		{
			CAMERA_TO_POSITIVE = 1,
			CAMERA_TO_NEGITIVE = -1,
			CAMERA_STOP = 0,
		};

		enum
		{
			CAMERA_MODE_NORMAL = 0,
			CAMERA_MODE_STAND = 1,
			CAMERA_MODE_BLEND = 2,

			EVENT_CAMERA_NUMBER = 101,
		};

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		enum ELocales
		{
			LOCALE_DEFAULT,
			LOCALE_EN, // United Kingdom
			LOCALE_PT, // Portugal
			LOCALE_ES, // Spain
			LOCALE_FR, // France
			LOCALE_DE, // Germany
			LOCALE_RO, // Romania
			LOCALE_PL, // Poland
			LOCALE_IT, // Italy
			LOCALE_CZ, // Czech
			LOCALE_HU, // Hungary
			LOCALE_TR, // Turkey
			LOCALE_MAX_NUM,
		};
#endif

		struct SCameraSpeed
		{
			float m_fUpDir;
			float m_fViewDir;
			float m_fCrossDir;

			SCameraSpeed() : m_fUpDir(0.0f), m_fViewDir(0.0f), m_fCrossDir(0.0f) {}
		};

	public:
		CPythonApplication();
		virtual ~CPythonApplication();

	public:
		void ShowWebPage(const char* c_szURL, const RECT& c_rcWebPage);
		void MoveWebPage(const RECT& c_rcWebPage);
		void HideWebPage();

		bool IsWebPageMode();

	public:
		void NotifyHack(const char* c_szFormat, ...);
		void GetInfo(UINT eInfo, std::string* pstInfo);
		void GetMousePosition(POINT* ppt);
		
		static CPythonApplication& Instance()
		{
			assert(ms_pInstance != NULL);
			return *ms_pInstance;
		}

		void Loop();
		void Destroy();
		void Clear();
		void Exit();
		void Abort();
		bool IsExit() const { return m_isExit; }

		void SetMinFog(float fMinFog);
		void SetFrameSkip(bool isEnable);
		void SkipRenderBuffering(DWORD dwSleepMSec);

		bool Create(PyObject* poSelf, const char* c_szName, int width, int height, int Windowed);
#ifdef ENABLE_GPU_CONFIG
		bool CreateDevice(int width, int height, int Windowed, int bit = 32, int frequency = 0, const std::string& gpu = "");
#else
		bool CreateDevice(int width, int height, int Windowed, int bit = 32, int frequency = 0);
#endif

		void UpdateGame();
		void RenderGame();

		bool Process();
		void OnMouseMove(int x, int y);
		void OnMouseLeftButtonDown(int x, int y);
		void OnMouseLeftButtonUp(int x, int y);

		void UpdateClientRect();

		bool CreateCursors();
		void DestroyCursors();

		void SafeSetCapture();
		void SafeReleaseCapture();

		BOOL SetCursorNum(int iCursorNum);
		void SetCursorVisible(BOOL bFlag, bool bLiarCursorOn = false);
		BOOL GetCursorVisible();
		bool GetLiarCursorOn();
		void SetCursorMode(int iMode);
		int GetCursorMode();
		int GetCursorNum() { return m_iCursorNum; }

		void SetMouseHandler(PyObject * poMouseHandler);

		int GetWidth();
		int GetHeight();

		void SetGlobalCenterPosition(LONG x, LONG y);
		void SetCenterPosition(float fx, float fy, float fz);
		void GetCenterPosition(TPixelPosition * pPixelPosition);
		void SetCamera(float Distance, float Pitch, float Rotation, float fDestinationHeight);
#ifdef ENABLE_TITLE_SYSTEM
		void SetTitleCameraPosition();
		void TitleCreate();
#endif
		void GetCamera(float * Distance, float * Pitch, float * Rotation, float * DestinationHeight);
		void RotateCamera(int iDirection);
		void PitchCamera(int iDirection);
		void ZoomCamera(int iDirection);
		void MovieRotateCamera(int iDirection);
		void MoviePitchCamera(int iDirection);
		void MovieZoomCamera(int iDirection);
		void MovieResetCamera();
		void SetViewDirCameraSpeed(float fSpeed);
		void SetCrossDirCameraSpeed(float fSpeed);
		void SetUpDirCameraSpeed(float fSpeed);
		float GetRotation();
		float GetPitch();

#ifdef ENABLE_SHAKE_CAMERA
		void SetShakeCamera(float fDuration, int iLevel);
#endif

		void SetFPS(int iFPS);
		void RenderSpecial();
		void SetServerTime(time_t tTime);
		time_t GetServerTime();
		time_t GetServerTimeStamp();
		float GetGlobalTime();
		float GetGlobalElapsedTime();

		float GetFaceSpeed()		{ return m_fFaceSpd; }
		float GetAveRenderTime()	{ return m_fAveRenderTime; }
		DWORD GetCurRenderTime()	{ return m_dwCurRenderTime; }
		DWORD GetCurUpdateTime()	{ return m_dwCurUpdateTime; }
		DWORD GetUpdateFPS()		{ return m_dwUpdateFPS; }
		DWORD GetRenderFPS()		{ return m_dwRenderFPS; }
		DWORD GetLoad()			{ return m_dwLoad; }
		DWORD GetFaceCount()	{ return m_dwFaceCount; }

		void SetConnectData(const char * c_szIP, int iPort);
		void GetConnectData(std::string & rstIP, int & riPort);

		void RunIMEUpdate();
		void RunIMETabEvent();
		void RunIMEReturnEvent();
		void RunPressExitKey();

		void RunIMEChangeCodePage();
		void RunIMEOpenCandidateListEvent();
		void RunIMECloseCandidateListEvent();
		void RunIMEOpenReadingWndEvent();
		void RunIMECloseReadingWndEvent();

		void EnableSpecialCameraMode();
		void SetCameraSpeed(int iPercentage);

		bool IsLockCurrentCamera();
		void SetEventCamera(const SCameraSetting & c_rCameraSetting);
		void BlendEventCamera(const SCameraSetting & c_rCameraSetting, float fBlendTime);
		void SetDefaultCamera();

		void SetCameraSetting(const SCameraSetting & c_rCameraSetting);
		void GetCameraSetting(SCameraSetting * pCameraSetting);
		void SaveCameraSetting(const char * c_szFileName);
		bool LoadCameraSetting(const char * c_szFileName);

		void SetForceSightRange(int iRange);

#ifdef ENABLE_WINDOW_MESSAGE
		void FlashApplication();
#endif

#ifdef ENABLE_MULTI_FARM_BLOCK
		void MultiFarmBlockIcon(BYTE bStatus);
#endif

	public:
		int OnLogoOpen(char* szName);
		int OnLogoUpdate();
		void OnLogoRender();
		void OnLogoClose();
		void SetTitle(const char* szTitle);

#ifdef ENABLE_AUTO_SYSTEM
		void SetAutoLogin(bool bVal) { m_bUseAutoLogin = bVal; }
		bool IsAutoLogin() { return m_bUseAutoLogin; }
#endif

	protected:
		LRESULT WindowProcedure(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);
		void ProcessSDLEvent(const SDL_Event& event);

		void OnCameraUpdate();

		void OnUIUpdate();
		void OnUIRender();

		void OnMouseUpdate();
		void OnMouseRender();

		void OnMouseWheel(int nLen);
		void OnMouseMiddleButtonDown(int x, int y);
		void OnMouseMiddleButtonUp(int x, int y);
		void OnMouseLeftButtonDoubleClick(int x, int y);
		void OnMouseRightButtonDown(int x, int y);
		void OnMouseRightButtonUp(int x, int y);
#ifdef ENABLE_MOUSEWHEEL_EVENT
		bool OnMouseWheelScroll(long x, long y , short wDelta);
#endif
		void OnSizeChange(int width, int height);
		void OnKeyDown(int iIndex);
		void OnKeyUp(int iIndex);
		void OnIMEKeyDown(int iIndex);

		int CheckDeviceState();

		BOOL __IsContinuousChangeTypeCursor(int iCursorNum);

		void __UpdateCamera();

		void __SetFullScreenWindow(HWND hWnd, DWORD dwWidth, DWORD dwHeight, DWORD dwBPP);
		void __MinimizeFullScreenWindow(HWND hWnd, DWORD dwWidth, DWORD dwHeight);


	protected:
		CTimer m_timer;

		CLightManager				m_LightManager;
		CSoundManager				m_SoundManager;
		CFlyingManager				m_FlyingManager;
		CRaceManager				m_RaceManager;
		CGameEventManager			m_GameEventManager;
		CItemManager				m_kItemMgr;

		UI::CWindowManager			m_kWndMgr;
		CEffectManager				m_kEftMgr;
		CPythonCharacterManager		m_kChrMgr;

		CServerStateChecker			m_kServerStateChecker;
		CPythonGraphic				m_pyGraphic;
		CPythonNetworkStream		m_pyNetworkStream;
		//CPythonNetworkDatagram		m_pyNetworkDatagram;
		CPythonPlayer				m_pyPlayer;
		CPythonIME					m_pyIme;
		CPythonItem					m_pyItem;
		CPythonShop					m_pyShop;
		CPythonExchange				m_pyExchange;
		CPythonChat					m_pyChat;
		CPythonTextTail				m_pyTextTail;
		CPythonNonPlayer			m_pyNonPlayer;
		CPythonMiniMap				m_pyMiniMap;
		CPythonEventManager			m_pyEventManager;
		CPythonBackground			m_pyBackground;
		CPythonSkill				m_pySkill;
		CPythonResource				m_pyRes;
		CPythonQuest				m_pyQuest;
		CPythonMessenger			m_pyManager;
#ifdef ENABLE_MESSENGER_RENEWAL
		CPythonCommunity			m_pyCommunity;
#endif
		CPythonSafeBox				m_pySafeBox;
		CPythonGuild				m_pyGuild;
#ifdef ENABLE_BATTLE_ROYALE
		PythonBattleRoyaleManager	m_pyBattleRoyaleManager;
#endif
#ifdef ENABLE_RIDING_EXTENDED
		CPythonMountUpGrade			m_pyMountUpGrade;
#endif
#ifdef ENABLE_SWITCHBOT
		CPythonSwitchbot			m_pySwitchbot;
#endif
#ifdef ENABLE_SASH_SYSTEM
		CPythonSash					m_pySash;
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
		CPythonChangeLook	m_pyChangeLook;
#endif
#ifdef ENABLE_AURA_SYSTEM
		CPythonAura					m_pyAura;
#endif
#ifdef ENABLE_CUBE_RENEWAL
		CPythonCubeRenewal 			m_pyCubeRenewal;
#endif
#ifdef ENABLE_EVENT_SYSTEM
		CPythonGameEvents			m_pyGameEvents;
#endif
#ifdef ENABLE_SHOP_SEARCH_SYSTEM
		CPythonPrivateShopSearch	m_pyPrivateShopSearch;
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
		CPythonSkillPet				m_pySkillPet;
#endif
#ifdef ENABLE_BATTLEPASS
		BattlePassManager			m_BattlePassMgr;
#endif
		CRenderTargetManager		m_kRenderTargetManager;
		CGuildMarkManager			m_kGuildMarkManager;
		CGuildMarkDownloader		m_kGuildMarkDownloader;
		CGuildMarkUploader			m_kGuildMarkUploader;
		CAccountConnector			m_kAccountConnector;
		CGraphicDevice				m_grpDevice;
		CNetworkDevice				m_netDevice;
#ifdef ENABLE_TREASURE_EVENT
		CPythonTreasureEvent		m_kTreasureEvent;
#endif
		CPythonSystem				m_pySystem;
#ifdef ENABLE_TITLE_SYSTEM
		CPythonTitleManager			m_pyTitleSystem;
#endif
#ifdef ENABLE_INGAME_WIKI
		CGraphicWikiRenderTargetTexture		m_pyWikiRenderTargetTexture;
		CWikiRenderTargetManager			m_pyWikiModelViewManager;
		CPythonWikiRenderTarget				m_pyWikiRenderTarget;
		CPythonWiki							m_pyWikiLoader;
#endif

		PyObject *					m_poMouseHandler;
		D3DXVECTOR3					m_v3CenterPosition;

		unsigned int				m_iFPS;
		float						m_fAveRenderTime;
		DWORD						m_dwCurRenderTime;
		DWORD						m_dwCurUpdateTime;
		DWORD						m_dwLoad;
		DWORD						m_dwWidth;
		DWORD						m_dwHeight;

	protected:
		// Time
		DWORD						m_dwLastIdleTime;
		DWORD						m_dwStartLocalTime;
		time_t						m_tServerTime;
		time_t						m_tLocalStartTime;
		float						m_fGlobalTime;
		float						m_fGlobalElapsedTime;

		/////////////////////////////////////////////////////////////
		// Camera
		SCameraSetting				m_DefaultCameraSetting;
		SCameraSetting				m_kEventCameraSetting;

		int							m_iCameraMode;
		float						m_fBlendCameraStartTime;
		float						m_fBlendCameraBlendTime;
		SCameraSetting				m_kEndBlendCameraSetting;

		float						m_fRotationSpeed;
		float						m_fPitchSpeed;
		float						m_fZoomSpeed;
		float						m_fCameraRotateSpeed;
		float						m_fCameraPitchSpeed;
		float						m_fCameraZoomSpeed;

#ifdef ENABLE_SHAKE_CAMERA
		float						m_fShakeCameraEndTime;
		float						m_fShakeCameraNextTime;
		int							m_iShakeCameraLevel;
		D3DXVECTOR3					m_v3ShakeCameraOffset;
#endif

		SCameraPos					m_kCmrPos;
		SCameraSpeed				m_kCmrSpd;

		BOOL						m_isSpecialCameraMode;
		// Camera
		/////////////////////////////////////////////////////////////

		float						m_fFaceSpd;
		DWORD						m_dwFaceSpdSum;
		DWORD						m_dwFaceSpdCount;

		DWORD						m_dwFaceAccCount;
		DWORD						m_dwFaceAccTime;

		DWORD						m_dwUpdateFPS;
		DWORD						m_dwRenderFPS;
		DWORD						m_dwFaceCount;
		DWORD						m_dwLButtonDownTime;
		DWORD						m_dwLButtonUpTime;

		typedef std::map<int, HANDLE>		TCursorHandleMap;
		TCursorHandleMap			m_CursorHandleMap;
		HANDLE						m_hCurrentCursor;

		BOOL						m_bCursorVisible;
		bool						m_bLiarCursorOn;
		int							m_iCursorMode;
		bool						m_isWindowed;
		bool						m_isFrameSkipDisable;

		// Connect Data
		std::string					m_strIP;
		int							m_iPort;

		static CPythonApplication*	ms_pInstance;

		bool						m_isMinimizedWnd;
		bool						m_isActivateWnd;
		BOOL						m_isWindowFullScreenEnable;
		bool						m_isExit;

		DWORD						m_dwStickyKeysFlag;
		DWORD						m_dwBufSleepSkipTime;
		int							m_iForceSightRange;
#ifdef ENABLE_AUTO_SYSTEM
		bool						m_bUseAutoLogin;
#endif
	protected:
		int m_iCursorNum;
		int m_iContinuousCursorNum;

#if defined(__ANDROID__)
	public:
		CAndroidTouchControls& GetTouchControls() { return m_kTouchControls; }
	protected:
		CAndroidTouchControls m_kTouchControls;
#endif
};
