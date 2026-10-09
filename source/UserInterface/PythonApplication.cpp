#include "StdAfx.h"
#include "../eterBase/Error.h"
#include "../eterlib/Camera.h"
#include "../eterlib/AttributeInstance.h"
#include "../gamelib/AreaTerrain.h"
#include "../EterGrnLib/Material.h"
#if !defined(__ANDROID__) && !defined(__APPLE__)
#ifdef CEF_BROWSER
#include "CefWebBrowser.h"
#else
#include "../CWebBrowser/CWebBrowser.h"
#endif
#endif


#include "resource.h"
#include "PythonApplication.h"
#include "PythonCharacterManager.h"

#include "ProcessScanner.h"

#include "HackShield.h"
#include "NProtectGameGuard.h"
#include "WiseLogicXTrap.h"
#include "CheckLatestFiles.h"
#ifdef ENABLE_SWITCHBOT
#include "PythonSwitchbot.h"
#endif
#ifdef EVENT_HANDLER_MASTER
#include "EventHandler.h"
#endif

extern void GrannyCreateSharedDeformBuffer();
extern void GrannyDestroySharedDeformBuffer();

float MIN_FOG = 2400.0f;
double g_specularSpd=0.007f;

CPythonApplication * CPythonApplication::ms_pInstance;

float c_fDefaultCameraRotateSpeed = 1.5f;
float c_fDefaultCameraPitchSpeed = 1.5f;
float c_fDefaultCameraZoomSpeed = 0.05f;

CPythonApplication::CPythonApplication() :
	m_bCursorVisible(TRUE),
	m_bLiarCursorOn(false),
	m_iCursorMode(CURSOR_MODE_HARDWARE),
	m_isWindowed(false),
	m_isFrameSkipDisable(false),
	m_poMouseHandler(NULL),
	m_dwUpdateFPS(0),
	m_dwRenderFPS(0),
	m_fAveRenderTime(0.0f),
	m_dwFaceCount(0),
	m_fGlobalTime(0.0f),
	m_fGlobalElapsedTime(0.0f),
	m_dwLButtonDownTime(0),
	m_dwLastIdleTime(0),
	m_isExit(false)
{
#ifndef _DEBUG
	SetEterExceptionHandler();
#endif

#if !defined(__ANDROID__) && !defined(__APPLE__)
	CTimer::Instance().UseCustomTime();
#endif

	m_dwWidth = 800;
	m_dwHeight = 600;

	ms_pInstance = this;
	m_isWindowFullScreenEnable = FALSE;

	m_v3CenterPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_dwStartLocalTime = ELTimer_GetMSec();
	m_tServerTime = 0;
	m_tLocalStartTime = 0;

	m_iPort = 0;
	m_iFPS = 60;

	m_isActivateWnd = false;
	m_isMinimizedWnd = true;

	m_fRotationSpeed = 0.0f;
	m_fPitchSpeed = 0.0f;
	m_fZoomSpeed = 0.0f;

#ifdef ENABLE_SHAKE_CAMERA
	m_fShakeCameraEndTime = 0.0f;
	m_fShakeCameraNextTime = 0.0f;
	m_iShakeCameraLevel = 0;
	m_v3ShakeCameraOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
#endif

	m_fFaceSpd = 0.0f;

	m_dwFaceAccCount = 0;
	m_dwFaceAccTime = 0;

	m_dwFaceSpdSum = 0;
	m_dwFaceSpdCount = 0;

	m_FlyingManager.SetMapManagerPtr(&m_pyBackground);

	m_iCursorNum = CURSOR_SHAPE_NORMAL;
	m_iContinuousCursorNum = CURSOR_SHAPE_NORMAL;

	m_isSpecialCameraMode = FALSE;
	m_fCameraRotateSpeed = c_fDefaultCameraRotateSpeed;
	m_fCameraPitchSpeed = c_fDefaultCameraPitchSpeed;
	m_fCameraZoomSpeed = c_fDefaultCameraZoomSpeed;

	m_iCameraMode = CAMERA_MODE_NORMAL;
	m_fBlendCameraStartTime = 0.0f;
	m_fBlendCameraBlendTime = 0.0f;

	m_iForceSightRange = -1;

#ifdef ENABLE_AUTO_SYSTEM
	m_bUseAutoLogin = false;
#endif

	CCameraManager::Instance().AddCamera(EVENT_CAMERA_NUMBER);
}

CPythonApplication::~CPythonApplication()
{
}

void CPythonApplication::GetMousePosition(POINT* ppt)
{
	CMSApplication::GetMousePosition(ppt);
}

void CPythonApplication::SetMinFog(float fMinFog)
{
	MIN_FOG = fMinFog;
}

void CPythonApplication::SetFrameSkip(bool isEnable)
{
	if (isEnable)
		m_isFrameSkipDisable=false;
	else
		m_isFrameSkipDisable=true;
}

void CPythonApplication::NotifyHack(const char* c_szFormat, ...)
{
	char szBuf[1024];

	va_list args;
	va_start(args, c_szFormat);	
	_vsnprintf(szBuf, sizeof(szBuf), c_szFormat, args);
	va_end(args);
	m_pyNetworkStream.NotifyHack(szBuf);
}

void CPythonApplication::GetInfo(UINT eInfo, std::string* pstInfo)
{
	switch (eInfo)
	{
	case INFO_ACTOR:
		m_kChrMgr.GetInfo(pstInfo);
		break;
	case INFO_EFFECT:
		m_kEftMgr.GetInfo(pstInfo);
		break;
	case INFO_ITEM:
		m_pyItem.GetInfo(pstInfo);
		break;
	case INFO_TEXTTAIL:
		m_pyTextTail.GetInfo(pstInfo);
		break;
	}
}

#if defined(__ANDROID__)
extern "C" void Android_ExitApp();
#endif

void CPythonApplication::Abort()
{
	TraceError("============================================================================================================");
	TraceError("Abort!!!!\n\n");

	PyThreadState* tstate = PyThreadState_GET();
	if (tstate)
	{
		for (PyFrameObject* frame = tstate->frame; frame; frame = frame->f_back)
		{
			PyCodeObject* f_code = frame->f_code;
			if (!f_code || !f_code->co_filename || !f_code->co_name)
				continue;

			const char* filename = PyString_AsString(f_code->co_filename);
			const char* funcname = PyString_AsString(f_code->co_name);
			int line = PyFrame_GetLineNumber(frame);
			TraceError("filename=%s, name=%s, line=%d", filename, funcname, line);
		}
	}

	PostQuitMessage(0);
#if defined(__ANDROID__)
	Android_ExitApp();
#endif
}

void CPythonApplication::Exit()
{
	m_isExit = true;
#if defined(__ANDROID__)
	Android_ExitApp();
#else
	PostQuitMessage(0);
#endif
}

bool PERF_CHECKER_RENDER_GAME = false;

void CPythonApplication::RenderGame()
{
#if defined(USE_OPENGL_ES)
	CGraphicBase::SetDungeonMapMode(m_pyBackground.IsDungeonMap());
#endif

	if (!PERF_CHECKER_RENDER_GAME)
	{
		m_kRenderTargetManager.RenderBackgrounds();

#ifdef ENABLE_TITLE_SYSTEM
		CPythonTitleManager::Instance().RenderBackground();
#endif
#ifdef ENABLE_INGAME_WIKI
		if (CPythonWikiRenderTarget::instance().CanRenderWikiModules()) {
			m_pyWikiModelViewManager.RenderBackgrounds();
		}
#endif
		float fAspect=m_kWndMgr.GetAspect();
		float fFarClip=m_pyBackground.GetFarClip();

#ifdef ENABLE_FOV_OPTION
		if (CPythonSystem::instance().IsExtendedFOV())
			m_pyGraphic.SetPerspective(CAMERA_PERSPECTIVE * 2.5, fAspect, 100.0, fFarClip);
		else
			m_pyGraphic.SetPerspective(CAMERA_PERSPECTIVE, fAspect, 100.0, fFarClip);
#else
		m_pyGraphic.SetPerspective(CAMERA_PERSPECTIVE, fAspect, 100.0, fFarClip);
#endif

		CCullingManager::Instance().Process();

		m_kChrMgr.Deform();
	
		m_kRenderTargetManager.DeformModels();
#ifdef ENABLE_INGAME_WIKI
		if (CPythonWikiRenderTarget::instance().CanRenderWikiModules()) {
			m_pyWikiModelViewManager.DeformModels();
		}
#endif
	
		m_pyBackground.RenderCharacterShadowToTexture();
	
		m_pyGraphic.SetGameRenderState();
		m_pyGraphic.PushState();
	
		{
			long lx, ly;
			m_kWndMgr.GetMousePosition(lx, ly);
			m_pyGraphic.SetCursorPosition(lx, ly);
		}
	
		m_pyBackground.RenderSky();
	
		m_pyBackground.RenderBeforeLensFlare();
	
		m_pyBackground.RenderCloud();
	
		m_pyBackground.BeginEnvironment();
		m_pyBackground.Render();
	
		m_pyBackground.SetCharacterDirLight();
		m_kChrMgr.Render();
	
		m_kRenderTargetManager.RenderModels();

#ifdef ENABLE_TITLE_SYSTEM
		CPythonTitleManager::Instance().RenderModel();
#endif
#ifdef ENABLE_INGAME_WIKI
		if (CPythonWikiRenderTarget::instance().CanRenderWikiModules()) {
			m_pyWikiModelViewManager.RenderModels();
		}
#endif
	
		m_pyBackground.SetBackgroundDirLight();
		m_pyBackground.RenderWater();
		m_pyBackground.RenderSnow();
		m_pyBackground.RenderEffect();
	
		m_pyBackground.EndEnvironment();
	
		m_kEftMgr.Render();
	
		m_pyItem.Render();
	
		m_FlyingManager.Render();
	
		m_pyBackground.BeginEnvironment();
		m_pyBackground.RenderPCBlocker();
		m_pyBackground.EndEnvironment();
	
		m_pyBackground.RenderAfterLensFlare();
	}
}

void CPythonApplication::UpdateGame()
{
	DWORD t1 = ELTimer_GetMSec();
	POINT ptMouse;
	GetMousePosition(&ptMouse);

	CGraphicTextInstance::Hyperlink_UpdateMousePos(ptMouse.x, ptMouse.y);

#ifdef ENABLE_TRANSLATOR_GOOGLE_SYSTEM
	CGraphicTextInstance::UpdateMousePos(ptMouse.x, ptMouse.y);
#endif

	DWORD t2 = ELTimer_GetMSec();

	// !@# Alt+Tab Áß SetTransfor ¿¡¼­ Æ¨±è Çö»ó ÇØ°áÀ» À§ÇØ - [levites]
	//if (m_isActivateWnd)
	{
		CScreen s;
		float fAspect = UI::CWindowManager::Instance().GetAspect();
		float fFarClip = CPythonBackground::Instance().GetFarClip();

#ifdef ENABLE_FOV_OPTION
		if (CPythonSystem::instance().IsExtendedFOV())
			m_pyGraphic.SetPerspective(CAMERA_PERSPECTIVE * 2.5, fAspect, 100.0, fFarClip);
		else
			m_pyGraphic.SetPerspective(CAMERA_PERSPECTIVE, fAspect, 100.0, fFarClip);
#else
		m_pyGraphic.SetPerspective(CAMERA_PERSPECTIVE, fAspect, 100.0, fFarClip);
#endif
		s.BuildViewFrustum();
	}

	DWORD t3 = ELTimer_GetMSec();
	TPixelPosition kPPosMainActor;
	m_pyPlayer.NEW_GetMainActorPosition(&kPPosMainActor);

	DWORD t4 = ELTimer_GetMSec();
	m_pyBackground.Update(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);

	DWORD t5 = ELTimer_GetMSec();
	m_GameEventManager.SetCenterPosition(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);
	m_GameEventManager.Update();

	DWORD t6 = ELTimer_GetMSec();
	m_kChrMgr.Update();
	m_kRenderTargetManager.UpdateModels();

#ifdef ENABLE_TITLE_SYSTEM
	CPythonTitleManager::Instance().UpdateModel();
#endif

#ifdef ENABLE_INGAME_WIKI
	if (CPythonWikiRenderTarget::instance().CanRenderWikiModules()) {
		m_pyWikiModelViewManager.UpdateModels();
	}
#endif

	DWORD t7 = ELTimer_GetMSec();
#ifdef ENABLE_MINIMIZED_EFFECT_FIX
	static DWORD s_dwLastEffectUpdate = ELTimer_GetMSec();
	DWORD dwNow = ELTimer_GetMSec();

	DWORD dwInterval = 0;

	DWORD dwDelta = dwNow - s_dwLastEffectUpdate;
	if (dwDelta > 100)
		dwDelta = dwInterval; // spike koruması

	if (dwDelta >= dwInterval)
	{
		s_dwLastEffectUpdate = dwNow;

		m_kEftMgr.Update();
		m_kEftMgr.UpdateSound();
	}
#else
	m_kEftMgr.Update();
	m_kEftMgr.UpdateSound();
#endif
	DWORD t8 = ELTimer_GetMSec();
	m_FlyingManager.Update();
	DWORD t9 = ELTimer_GetMSec();
	m_pyItem.Update(ptMouse);
	DWORD t10 = ELTimer_GetMSec();
	m_pyPlayer.Update();
	DWORD t11 = ELTimer_GetMSec();

	m_pyPlayer.NEW_GetMainActorPosition(&kPPosMainActor);
	SetCenterPosition(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);
	DWORD t12 = ELTimer_GetMSec();

	if (PERF_CHECKER_RENDER_GAME)
	{
		if (t12 - t1 > 5)
		{
			static FILE* fp = fopen("perf_game_update.txt", "w");

			fprintf(fp, "GU.Total %d (Time %d)\n", t12 - t1, ELTimer_GetMSec());
			fprintf(fp, "GU.GMP %d\n", t2 - t1);
			fprintf(fp, "GU.SCR %d\n", t3 - t2);
			fprintf(fp, "GU.MPS %d\n", t4 - t3);
			fprintf(fp, "GU.BG %d\n", t5 - t4);
			fprintf(fp, "GU.GEM %d\n", t6 - t5);
			fprintf(fp, "GU.CHR %d\n", t7 - t6);
			fprintf(fp, "GU.EFT %d\n", t8 - t7);
			fprintf(fp, "GU.FLY %d\n", t9 - t8);
			fprintf(fp, "GU.ITM %d\n", t10 - t9);
			fprintf(fp, "GU.PLR %d\n", t11 - t10);
			fprintf(fp, "GU.POS %d\n", t12 - t11);
			fflush(fp);
		}
	}

#ifdef ENABLE_BATTLE_ROYALE
	PythonBattleRoyaleManager::Instance().Update();
#endif

}

void CPythonApplication::SkipRenderBuffering(DWORD dwSleepMSec)
{
	m_dwBufSleepSkipTime=ELTimer_GetMSec()+dwSleepMSec;
}

bool CPythonApplication::Process()
{
#if defined(CHECK_LATEST_DATA_FILES)
	if (CheckLatestFiles_PollEvent())
		return false;
#endif
#ifdef USE_AHNLAB_HACKSHIELD
	if (HackShield_PollEvent())
		return false;
#endif
#ifdef XTRAP_CLIENT_ENABLE
	XTrap_PollEvent();
#endif
	ELTimer_SetFrameMSec();

	// 	m_Profiler.Clear();
	DWORD dwStart = ELTimer_GetMSec();

	///////////////////////////////////////////////////////////////////////////////////////////////////
	static DWORD	s_dwUpdateFrameCount = 0;
	static DWORD	s_dwRenderFrameCount = 0;
	static DWORD	s_dwFaceCount = 0;
	static UINT		s_uiLoad = 0;
	static DWORD	s_dwCheckTime = ELTimer_GetMSec();

	if (ELTimer_GetMSec() - s_dwCheckTime > 1000)
	{
		m_dwUpdateFPS		= s_dwUpdateFrameCount;
		m_dwRenderFPS		= s_dwRenderFrameCount;
		m_dwLoad			= s_uiLoad;

		m_dwFaceCount		= s_dwFaceCount / max(1, s_dwRenderFrameCount);

		s_dwCheckTime		= ELTimer_GetMSec();

		s_uiLoad = s_dwFaceCount = s_dwUpdateFrameCount = s_dwRenderFrameCount = 0;
#ifdef EVENT_HANDLER_MASTER
		EventHandler::Instance().Proccess();
#endif
	}

	// Update Time
	static BOOL s_bFrameSkip = false;
	static UINT s_uiNextFrameTime = ELTimer_GetMSec();

#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime1=ELTimer_GetMSec();
#endif
	CTimer& rkTimer=CTimer::Instance();
	rkTimer.Advance();

	m_fGlobalTime = rkTimer.GetCurrentSecond();
	m_fGlobalElapsedTime = rkTimer.GetElapsedSecond();

	UINT uiFrameTime = max(1u, (UINT)rkTimer.GetElapsedMilliecond());
	s_uiNextFrameTime += uiFrameTime;	//17 - 1초당 60fps기준.

	DWORD updatestart = ELTimer_GetMSec();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime2=ELTimer_GetMSec();
#endif
	// Network I/O	
	m_pyNetworkStream.Process();	
	//m_pyNetworkDatagram.Process();

	m_kGuildMarkUploader.Process();

#ifdef USE_NPROTECT_GAMEGUARD
	if (GameGuard_IsError())
		return false;
#endif

	m_kGuildMarkDownloader.Process();
	m_kAccountConnector.Process();

#ifdef __PERFORMANCE_CHECK__		
	DWORD dwUpdateTime3=ELTimer_GetMSec();
#endif
	//////////////////////
	// Input Process
	// Keyboard
	UpdateKeyboard();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime4=ELTimer_GetMSec();
#endif
	// Mouse
#if !defined(__ANDROID__) && !defined(__APPLE__)
	int mouseX = 0, mouseY = 0;

	if (m_pSDLWindow)
	{
		SDL_GetMouseState(&mouseX, &mouseY);
		OnMouseMove(mouseX, mouseY);
	}
	else
	{
		POINT Point;
		if (GetCursorPos(&Point))
		{
			ScreenToClient(m_hWnd, &Point);
			OnMouseMove(Point.x, Point.y);		
		}
	}
#endif
	//////////////////////
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime5=ELTimer_GetMSec();
#endif
	//!@# Alt+Tab 중 SetTransfor 에서 튕김 현상 해결을 위해 - [levites]
	//if (m_isActivateWnd)
	__UpdateCamera();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime6=ELTimer_GetMSec();
#endif
	// Update Game Playing
	CResourceManager::Instance().Update();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime7=ELTimer_GetMSec();
#endif
	OnCameraUpdate();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime8=ELTimer_GetMSec();
#endif
	OnMouseUpdate();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime9=ELTimer_GetMSec();
#endif
	OnUIUpdate();

#ifdef __PERFORMANCE_CHECK__		
	DWORD dwUpdateTime10=ELTimer_GetMSec();

	if (dwUpdateTime10-dwUpdateTime1>10)
	{			
		static FILE* fp=fopen("perf_app_update.txt", "w");

		fprintf(fp, "AU.Total %d (Time %d)\n", dwUpdateTime9-dwUpdateTime1, ELTimer_GetMSec());
		fprintf(fp, "AU.TU %d\n", dwUpdateTime2-dwUpdateTime1);
		fprintf(fp, "AU.NU %d\n", dwUpdateTime3-dwUpdateTime2);
		fprintf(fp, "AU.KU %d\n", dwUpdateTime4-dwUpdateTime3);
		fprintf(fp, "AU.MP %d\n", dwUpdateTime5-dwUpdateTime4);
		fprintf(fp, "AU.CP %d\n", dwUpdateTime6-dwUpdateTime5);
		fprintf(fp, "AU.RU %d\n", dwUpdateTime7-dwUpdateTime6);
		fprintf(fp, "AU.CU %d\n", dwUpdateTime8-dwUpdateTime7);
		fprintf(fp, "AU.MU %d\n", dwUpdateTime9-dwUpdateTime8);
		fprintf(fp, "AU.UU %d\n", dwUpdateTime10-dwUpdateTime9);			
		fprintf(fp, "----------------------------------\n");
		fflush(fp);
	}		
#endif

	//Update하는데 걸린시간.delta값
	m_dwCurUpdateTime = ELTimer_GetMSec() - updatestart;

	DWORD dwCurrentTime = ELTimer_GetMSec();
	BOOL  bCurrentLateUpdate = FALSE;

	s_bFrameSkip = false;

	if (dwCurrentTime > s_uiNextFrameTime)
	{
		int dt = dwCurrentTime - s_uiNextFrameTime;
		int nAdjustTime = (uiFrameTime > 0) ? (int)(((float)dt / (float)uiFrameTime) * uiFrameTime) : dt;

		if ( dt >= 500 )
		{
			s_uiNextFrameTime += nAdjustTime; 
			printf("FrameSkip 보정 %d\n",nAdjustTime);
			CTimer::Instance().Adjust(nAdjustTime);
		}

		s_bFrameSkip = true;
		bCurrentLateUpdate = TRUE;
	}

	//s_bFrameSkip = false;

	//if (dwCurrentTime > s_uiNextFrameTime)
	//{
	//	int dt = dwCurrentTime - s_uiNextFrameTime;

	//	//너무 늦었을 경우 따라잡는다.
	//	//그리고 m_dwCurUpdateTime는 delta인데 delta랑 absolute time이랑 비교하면 어쩌자는겨?
	//	//if (dt >= 500 || m_dwCurUpdateTime > s_uiNextFrameTime)

	//	//기존코드대로 하면 0.5초 이하 차이난 상태로 update가 지속되면 계속 rendering frame skip발생
	//	if (dt >= 500 || m_dwCurUpdateTime > s_uiNextFrameTime)
	//	{
	//		s_uiNextFrameTime += dt / uiFrameTime * uiFrameTime; 
	//		printf("FrameSkip 보정 %d\n", dt / uiFrameTime * uiFrameTime);
	//		CTimer::Instance().Adjust((dt / uiFrameTime) * uiFrameTime);
	//		s_bFrameSkip = true;
	//	}
	//}

	if (m_isFrameSkipDisable)
		s_bFrameSkip = false;

#if defined(__VTUNE__) || defined(__ANDROID__) || defined(__APPLE__) || defined(__IOS__)
	s_bFrameSkip = false;
#endif
	/*
	static bool s_isPrevFrameSkip=false;
	static DWORD s_dwFrameSkipCount=0;
	static DWORD s_dwFrameSkipEndTime=0;

	static DWORD ERROR_FRAME_SKIP_COUNT = 60*5;
	static DWORD ERROR_FRAME_SKIP_TIME = ERROR_FRAME_SKIP_COUNT*18;

	//static DWORD MAX_FRAME_SKIP=0;

	if (IsActive())
	{
	DWORD dwFrameSkipCurTime=ELTimer_GetMSec();

	if (s_bFrameSkip)
	{
	// 이전 프레임도 스킵이라면..
	if (s_isPrevFrameSkip)
	{
	if (s_dwFrameSkipEndTime==0)
	{
	s_dwFrameSkipCount=0; // 프레임 체크는 로딩 대비
	s_dwFrameSkipEndTime=dwFrameSkipCurTime+ERROR_FRAME_SKIP_TIME; // 시간 체크는 로딩후 프레임 스킵 체크

	//printf("FrameSkipCheck Start\n");
	}
	++s_dwFrameSkipCount;

	//if (MAX_FRAME_SKIP<s_dwFrameSkipCount)
	//	MAX_FRAME_SKIP=s_dwFrameSkipCount;

	//printf("u %d c %d/%d t %d\n", 
	//	dwUpdateTime9-dwUpdateTime1,
	//	s_dwFrameSkipCount, 
	//	MAX_FRAME_SKIP,
	//	s_dwFrameSkipEndTime);

	//#ifndef _DEBUG
	// 일정 시간동안 계속 프레임 스킵만 한다면...
	if (s_dwFrameSkipCount>ERROR_FRAME_SKIP_COUNT && s_dwFrameSkipEndTime<dwFrameSkipCurTime)
	{
	s_isPrevFrameSkip=false;
	s_dwFrameSkipEndTime=0;
	s_dwFrameSkipCount=0;

	//m_pyNetworkStream.AbsoluteExitGame();

	/*
	TraceError("무한 프레임 스킵으로 접속을 종료합니다");

	{
	FILE* fp=fopen("errorlog.txt", "w");
	if (fp)
	{
	fprintf(fp, "FRAMESKIP\n");
	fprintf(fp, "Total %d\n",		dwUpdateTime9-dwUpdateTime1);
	fprintf(fp, "Timer %d\n",		dwUpdateTime2-dwUpdateTime1);
	fprintf(fp, "Network %d\n",		dwUpdateTime3-dwUpdateTime2);
	fprintf(fp, "Keyboard %d\n",	dwUpdateTime4-dwUpdateTime3);
	fprintf(fp, "Controll %d\n",	dwUpdateTime5-dwUpdateTime4);
	fprintf(fp, "Resource %d\n",	dwUpdateTime6-dwUpdateTime5);
	fprintf(fp, "Camera %d\n",		dwUpdateTime7-dwUpdateTime6);
	fprintf(fp, "Mouse %d\n",		dwUpdateTime8-dwUpdateTime7);
	fprintf(fp, "UI %d\n",			dwUpdateTime9-dwUpdateTime8);
	fclose(fp);

	WinExec("errorlog.exe", SW_SHOW);
	}
	}
	}
	}

	s_isPrevFrameSkip=true;
	}
	else
	{
	s_isPrevFrameSkip=false;
	s_dwFrameSkipCount=0;
	s_dwFrameSkipEndTime=0;
	}
	}
	else
	{
	s_isPrevFrameSkip=false;
	s_dwFrameSkipCount=0;
	s_dwFrameSkipEndTime=0;
	}
	*/
	if (!s_bFrameSkip)
	{
		CGrannyMaterial::TranslateSpecularMatrix(g_specularSpd, g_specularSpd, 0.0f);
		DWORD dwRenderStartTime = ELTimer_GetMSec();
		bool canRender = true;

#if defined(__ANDROID__) || defined(__APPLE__) || defined(__IOS__)
		canRender = true;
#else
		if (m_isMinimizedWnd || IsIconic(GetWindowHandle()))
		{
			canRender = false;
		}
		else
		{
			if (DEVICE_STATE_OK != CheckDeviceState())
			{
				canRender = false;
			}
		}

		if (!IsActive())
		{
			SkipRenderBuffering(3000);
		}

		if (!canRender)
		{
			//RenderSpecial();	//09-12-19 21:24 Bvural41
			SkipRenderBuffering(1500);	//16-10-25 Bvural41 Efekt Birikme Fix std: 3000
		}
		else
#endif
		{
			// RestoreLostDevice
			CCullingManager::Instance().Update();
			if (m_pyGraphic.Begin())
			{

				m_pyGraphic.ClearDepthBuffer();

#if defined(_DEBUG) || defined(__ANDROID__) || defined(__APPLE__) || defined(__IOS__)
				m_pyGraphic.SetClearColor(0.0f, 0.0f, 0.0f);
				m_pyGraphic.Clear();
#endif

				/////////////////////
				// Interface
				m_pyGraphic.SetInterfaceRenderState();

				OnUIRender();
				OnMouseRender();
#if defined(__ANDROID__)
				m_kTouchControls.Render();
#endif
				/////////////////////

				m_pyGraphic.End();

				//DWORD t1 = ELTimer_GetMSec();
				m_pyGraphic.Show();
				//DWORD t2 = ELTimer_GetMSec();

				DWORD dwRenderEndTime = ELTimer_GetMSec();

				static DWORD s_dwRenderCheckTime = dwRenderEndTime;
				static DWORD s_dwRenderRangeTime = 0;
				static DWORD s_dwRenderRangeFrame = 0;

				m_dwCurRenderTime = dwRenderEndTime - dwRenderStartTime;
				s_dwRenderRangeTime += m_dwCurRenderTime;
				++s_dwRenderRangeFrame;

				if (dwRenderEndTime - s_dwRenderCheckTime > 1000)
				{
					m_fAveRenderTime = (s_dwRenderRangeFrame > 0) ? float(double(s_dwRenderRangeTime) / double(s_dwRenderRangeFrame)) : 0.0f;

					s_dwRenderCheckTime = ELTimer_GetMSec();
					s_dwRenderRangeTime = 0;
					s_dwRenderRangeFrame = 0;
				}

				DWORD dwCurFaceCount = m_pyGraphic.GetFaceCount();
				m_pyGraphic.ResetFaceCount();
				s_dwFaceCount += dwCurFaceCount;

				if (dwCurFaceCount > 5000)
				{
					// ÇÁ·¹ÀÓ ¿ÏÃæ Ã³¸®
					if (dwRenderEndTime > m_dwBufSleepSkipTime)
					{
						static float s_fBufRenderTime = 0.0f;

						float fCurRenderTime = m_dwCurRenderTime;

						if (fCurRenderTime > s_fBufRenderTime)
						{
							float fRatio = fMAX(0.5f, (fCurRenderTime - s_fBufRenderTime) / 30.0f);
							s_fBufRenderTime = (s_fBufRenderTime * (100.0f - fRatio) + (fCurRenderTime + 5) * fRatio) / 100.0f;
						}
						else
						{
							float fRatio = 0.5f;
							s_fBufRenderTime = (s_fBufRenderTime * (100.0f - fRatio) + fCurRenderTime * fRatio) / 100.0f;
						}

						// ÇÑ°èÄ¡¸¦ Á¤ÇÑ´Ù
						if (s_fBufRenderTime > 100.0f)
							s_fBufRenderTime = 100.0f;

						DWORD dwBufRenderTime = s_fBufRenderTime;

						if (m_isWindowed)
						{
							if (dwBufRenderTime > 58)
								dwBufRenderTime = 64;
							else if (dwBufRenderTime > 42)
								dwBufRenderTime = 48;
							else if (dwBufRenderTime > 26)
								dwBufRenderTime = 32;
							else if (dwBufRenderTime > 10)
								dwBufRenderTime = 16;
							else
								dwBufRenderTime = 8;
						}

						// ÀÏÁ¤ ÇÁ·¹ÀÓ ¼Óµµ¿¡ ¸ÂÃß¾îÁÖ´ÂÂÊ¿¡ ´«¿¡ ÆíÇÏ´Ù
						// ¾Æ·¡¿¡¼­ ÇÑ¹ø ÇÏ¸é ‰ç´?
						//if (m_dwCurRenderTime < dwBufRenderTime)
						//	Sleep(dwBufRenderTime - m_dwCurRenderTime);

						m_fAveRenderTime = s_fBufRenderTime;
					}

					m_dwFaceAccCount += dwCurFaceCount;
					m_dwFaceAccTime += m_dwCurRenderTime;

					m_fFaceSpd = (m_dwFaceAccTime > 0) ? (float)m_dwFaceAccCount / (float)m_dwFaceAccTime : 0.0f;

#if defined(__ANDROID__) || defined(__APPLE__) || defined(__IOS__)
					m_pyBackground.SetViewDistanceSet(0, 20000.0f);
#else
					// 거리 자동 조절
					if (-1 == m_iForceSightRange)
					{
						static float s_fAveRenderTime = 16.0f;
						float fRatio = 0.3f;
						s_fAveRenderTime = (s_fAveRenderTime * (100.0f - fRatio) + max(16.0f, m_dwCurRenderTime) * fRatio) / 100.0f;

						float fFar = 25600.0f;
						float fNear = MIN_FOG;
						double dbAvePow = double(1000.0f / s_fAveRenderTime);
						double dbMaxPow = 60.0;
						float fDistance = max(fNear + (fFar - fNear) * (dbAvePow) / dbMaxPow, fNear);
						m_pyBackground.SetViewDistanceSet(0, fDistance);
					}
					// 거리 강제 설정시
					else
					{
						m_pyBackground.SetViewDistanceSet(0, float(m_iForceSightRange));
					}
#endif
				}
				else
				{
					// 10000 폴리곤 보다 적을때는 가장 멀리 보이게 한다
					m_pyBackground.SetViewDistanceSet(0, 25600.0f);
				}

				++s_dwRenderFrameCount;
			}
		}
	}

#if !defined(__ANDROID__) && !defined(__APPLE__)
	int rest = s_uiNextFrameTime - ELTimer_GetMSec();


	if (rest > 0 && !bCurrentLateUpdate)
	{
		s_uiLoad -= rest; // 쉰 시간은 로드에서 뺀다..
		Sleep(rest);
	}
#endif

	++s_dwUpdateFrameCount;

	s_uiLoad += ELTimer_GetMSec() - dwStart;
	//m_Profiler.ProfileByScreen();
	return true;
}

void CPythonApplication::UpdateClientRect()
{
	RECT rcApp;
	GetClientRect(&rcApp);
	OnSizeChange(rcApp.right - rcApp.left, rcApp.bottom - rcApp.top);
}

void CPythonApplication::SetMouseHandler(PyObject* poMouseHandler)
{	
	m_poMouseHandler = poMouseHandler;
}

int CPythonApplication::CheckDeviceState()
{
	CGraphicDevice::EDeviceState e_deviceState = m_grpDevice.GetDeviceState();

	switch (e_deviceState)
	{
	case CGraphicDevice::DEVICESTATE_NULL:
		return DEVICE_STATE_FALSE;

	case CGraphicDevice::DEVICESTATE_BROKEN:
		return DEVICE_STATE_SKIP;

	case CGraphicDevice::DEVICESTATE_NEEDS_RESET:
		{
			CPythonBackground& rkBG = CPythonBackground::Instance();
			rkBG.ReleaseCharacterShadowTexture();

#ifdef ENABLE_RENDER_TARGET
			CRenderTargetManager::Instance().ReleaseRenderTargetTextures();
#endif
#ifdef ENABLE_INGAME_WIKI
			CWikiRenderTargetManager::Instance().ReleaseRenderTargetTextures();
#endif

			Trace("DEVICESTATE_NEEDS_RESET - attempting");
			if (!m_grpDevice.Reset())
			{
				return DEVICE_STATE_SKIP;
			}

#ifdef ENABLE_RENDER_TARGET
			CRenderTargetManager::Instance().CreateRenderTargetTextures();
#endif
#ifdef ENABLE_INGAME_WIKI
			CWikiRenderTargetManager::Instance().CreateRenderTargetTextures();
#endif
			rkBG.CreateCharacterShadowTexture();
		}
		break;

	case CGraphicDevice::DEVICESTATE_OK:
	default:
		break;
	}

	return DEVICE_STATE_OK;
}

#ifdef ENABLE_GPU_CONFIG
bool CPythonApplication::CreateDevice(int width, int height, int Windowed, int bit /* = 32*/, int frequency /* = 0*/, const std::string& gpu)
#else
bool CPythonApplication::CreateDevice(int width, int height, int Windowed, int bit /* = 32*/, int frequency /* = 0*/)
#endif
{
	int iRet;

	m_grpDevice.InitBackBufferCount(2);
	m_grpDevice.RegisterWarningString(CGraphicDevice::CREATE_BAD_DRIVER, ApplicationStringTable_GetStringz(IDS_WARN_BAD_DRIVER, "WARN_BAD_DRIVER"));
	m_grpDevice.RegisterWarningString(CGraphicDevice::CREATE_NO_TNL, ApplicationStringTable_GetStringz(IDS_WARN_NO_TNL, "WARN_NO_TNL"));

#ifdef ENABLE_GPU_CONFIG
	iRet = m_grpDevice.Create(GetWindowHandle(), width, height, Windowed ? true : false, bit, frequency, gpu);
#else
	iRet = m_grpDevice.Create(GetWindowHandle(), width, height, Windowed ? true : false, bit,frequency);
#endif

	switch (iRet)
	{
	case CGraphicDevice::CREATE_OK:
		return true;

	case CGraphicDevice::CREATE_REFRESHRATE:
		return true;

	case CGraphicDevice::CREATE_ENUM:
	case CGraphicDevice::CREATE_DETECT:
		SET_EXCEPTION(CREATE_NO_APPROPRIATE_DEVICE);
		TraceError("CreateDevice: Enum & Detect failed");
		return false;

	case CGraphicDevice::CREATE_NO_DIRECTX:
		//PyErr_SetString(PyExc_RuntimeError, "DirectX 8.1 or greater required to run game");
		SET_EXCEPTION(CREATE_NO_DIRECTX);
		TraceError("CreateDevice: DirectX 8.1 or greater required to run game");
		return false;

	case CGraphicDevice::CREATE_DEVICE:
		//PyErr_SetString(PyExc_RuntimeError, "GraphicDevice create failed");
		SET_EXCEPTION(CREATE_DEVICE);
		TraceError("CreateDevice: GraphicDevice create failed");
		return false;

	case CGraphicDevice::CREATE_FORMAT:
		SET_EXCEPTION(CREATE_FORMAT);
		TraceError("CreateDevice: Change the screen format");
		return false;

		/*case CGraphicDevice::CREATE_GET_ADAPTER_DISPLAY_MODE:
		//PyErr_SetString(PyExc_RuntimeError, "GetAdapterDisplayMode failed");
		SET_EXCEPTION(CREATE_GET_ADAPTER_DISPLAY_MODE);
		TraceError("CreateDevice: GetAdapterDisplayMode failed");
		return false;*/

	case CGraphicDevice::CREATE_GET_DEVICE_CAPS:
		PyErr_SetString(PyExc_RuntimeError, "GetDevCaps failed");
		TraceError("CreateDevice: GetDevCaps failed");
		return false;

	case CGraphicDevice::CREATE_GET_DEVICE_CAPS2:
		PyErr_SetString(PyExc_RuntimeError, "GetDevCaps2 failed");
		TraceError("CreateDevice: GetDevCaps2 failed");
		return false;

	default:
		if (iRet & CGraphicDevice::CREATE_OK)
		{
			//if (iRet & CGraphicDevice::CREATE_BAD_DRIVER)
			//{
			//	LogBox(ApplicationStringTable_GetStringz(IDS_WARN_BAD_DRIVER), NULL, GetWindowHandle());
			//}
			if (iRet & CGraphicDevice::CREATE_NO_TNL)
			{
				CGrannyLODController::SetMinLODMode(true);
				//LogBox(ApplicationStringTable_GetStringz(IDS_WARN_NO_TNL), NULL, GetWindowHandle());
			}
			return true;
		}

		//PyErr_SetString(PyExc_RuntimeError, "Unknown Error!");
		SET_EXCEPTION(UNKNOWN_ERROR);
		TraceError("CreateDevice: Unknown Error!");
		return false;
	}
}

void CPythonApplication::ProcessSDLEvent(const SDL_Event& event)
{
	switch (event.type)
	{
		case SDL_KEYDOWN:
		{
			BYTE dik = SDL_ScancodeToDIK(event.key.keysym.scancode);
			if (dik != 0 && !IsPressed(dik))
			{
				KeyDown(dik);
			}
			break;
		}

		case SDL_KEYUP:
		{
			BYTE dik = SDL_ScancodeToDIK(event.key.keysym.scancode);
			if (dik != 0 && IsPressed(dik))
			{
#ifdef _WIN32
				// Metin2 Space/Movement Lock: If the window is in the background (user clicked another window or Alt+Tabbed),
				// ignore SDL2's synthetic focus-loss keyup for attack and movement keys so the character continues auto-attacking/walking.
				if (GetForegroundWindow() != m_hWnd || SDL_GetKeyboardFocus() != m_pSDLWindow)
				{
					if (dik == DIK_SPACE || dik == DIK_W || dik == DIK_S || dik == DIK_A || dik == DIK_D ||
						dik == DIK_UP || dik == DIK_DOWN || dik == DIK_LEFT || dik == DIK_RIGHT)
					{
						break;
					}
				}
#endif
				KeyUp(dik);
			}
			break;
		}

		case SDL_WINDOWEVENT:
		{
			switch (event.window.event)
			{
				case SDL_WINDOWEVENT_RESIZED:
				case SDL_WINDOWEVENT_SIZE_CHANGED:
				{
					int w = event.window.data1;
					int h = event.window.data2;
					if (w > 0 && h > 0)
					{
						m_dwWidth = w;
						m_dwHeight = h;
						m_grpDevice.ResizeBackBuffer(w, h);
						OnSizeChange(w, h);
#if defined(__ANDROID__)
						m_kTouchControls.UpdateScreenSize(w, h);
#endif
					}
					break;
				}
				case SDL_WINDOWEVENT_MINIMIZED:
					m_isMinimizedWnd = true;
					m_isActive = false;
					break;
				case SDL_WINDOWEVENT_RESTORED:
				case SDL_WINDOWEVENT_MAXIMIZED:
					m_isMinimizedWnd = false;
					m_isActive = true;
					break;
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					m_isActivateWnd = true;
					m_SoundManager.RestoreVolume();
					break;
				case SDL_WINDOWEVENT_FOCUS_LOST:
					m_isActivateWnd = false;
					m_SoundManager.SaveVolume();
					break;
			}
			break;
		}

		case SDL_FINGERDOWN:
		{
			int x = (int)(event.tfinger.x * m_dwWidth);
			int y = (int)(event.tfinger.y * m_dwHeight);
#if defined(__ANDROID__)
			if (!m_kTouchControls.OnFingerDown(event.tfinger.fingerId, (float)x, (float)y))
			{
				OnMouseLeftButtonDown(x, y);
			}
#else
			OnMouseLeftButtonDown(x, y);
#endif
			break;
		}

		case SDL_FINGERUP:
		{
			int x = (int)(event.tfinger.x * m_dwWidth);
			int y = (int)(event.tfinger.y * m_dwHeight);
#if defined(__ANDROID__)
			if (!m_kTouchControls.OnFingerUp(event.tfinger.fingerId, (float)x, (float)y))
			{
				OnMouseLeftButtonUp(x, y);
			}
#else
			OnMouseLeftButtonUp(x, y);
#endif
			break;
		}

		case SDL_FINGERMOTION:
		{
			int x = (int)(event.tfinger.x * m_dwWidth);
			int y = (int)(event.tfinger.y * m_dwHeight);
			float dx = event.tfinger.dx * (float)m_dwWidth;
			float dy = event.tfinger.dy * (float)m_dwHeight;
#if defined(__ANDROID__)
			if (!m_kTouchControls.OnFingerMotion(event.tfinger.fingerId, (float)x, (float)y, dx, dy))
			{
				OnMouseMove(x, y);
			}
#else
			OnMouseMove(x, y);
#endif
			break;
		}

		case SDL_MOUSEBUTTONDOWN:
		{
			if (event.button.which == SDL_TOUCH_MOUSEID)
				break;

			if (event.button.button == SDL_BUTTON_LEFT)
				OnMouseLeftButtonDown(event.button.x, event.button.y);
			else if (event.button.button == SDL_BUTTON_RIGHT)
				OnMouseRightButtonDown(event.button.x, event.button.y);
			else if (event.button.button == SDL_BUTTON_MIDDLE)
				OnMouseMiddleButtonDown(event.button.x, event.button.y);
			break;
		}

		case SDL_MOUSEBUTTONUP:
		{
			if (event.button.which == SDL_TOUCH_MOUSEID)
				break;

			if (event.button.button == SDL_BUTTON_LEFT)
				OnMouseLeftButtonUp(event.button.x, event.button.y);
			else if (event.button.button == SDL_BUTTON_RIGHT)
				OnMouseRightButtonUp(event.button.x, event.button.y);
			else if (event.button.button == SDL_BUTTON_MIDDLE)
				OnMouseMiddleButtonUp(event.button.x, event.button.y);
			break;
		}

		case SDL_MOUSEMOTION:
		{
			if (event.motion.which == SDL_TOUCH_MOUSEID)
				break;

			OnMouseMove(event.motion.x, event.motion.y);
			break;
		}

		case SDL_MOUSEWHEEL:
		{
			int wheelDelta = event.wheel.y;
			if (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
				wheelDelta *= -1;

			int mouseX = 0, mouseY = 0;
			SDL_GetMouseState(&mouseX, &mouseY);
			short zDelta = (short)(wheelDelta * 120);

#ifdef ENABLE_MOUSEWHEEL_EVENT
			if (OnMouseWheelScroll(mouseX, mouseY, zDelta))
				break;
#endif

			OnMouseWheel(zDelta);
			break;
		}

		default:
			break;
	}
}

void CPythonApplication::Loop()
{	
	SDL_Event event;
	while (!m_isExit)
	{	
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				m_isExit = true;
				break;
			}

			ProcessSDLEvent(event);
		}

		if (m_isExit)
			break;

		MSG msg;
		if (PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE))
		{
			m_isExit = true;
			break;
		}

		if (!Process())
			break;

		m_dwLastIdleTime = ELTimer_GetMSec();
	}
}

// SUPPORT_NEW_KOREA_SERVER
bool LoadLocaleData(const char* localePath)
{
	NANOBEGIN
		CPythonNonPlayer& rkNPCMgr = CPythonNonPlayer::Instance();
	CItemManager& rkItemMgr = CItemManager::Instance();
	CPythonSkill& rkSkillMgr = CPythonSkill::Instance();
#ifdef ENABLE_GROWTH_PET_SYSTEM
	CPythonSkillPet&	rkSkillPetMgr = CPythonSkillPet::Instance();
#endif
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();

	char szItemList[256];
	char szItemProto[256];
	char szItemDesc[256];
	char szMobProto[256];
	char szSkillDescFileName[256];
	char szSkillTableFileName[256];
	char szInsultList[256];
#ifdef ENABLE_GROWTH_PET_SYSTEM
	char szSkillPetFileName[256];
#endif
	snprintf(szItemList, sizeof(szItemList), "locale/common/item_list.txt");
	snprintf(szItemProto,	sizeof(szItemProto),	"%s/item_proto",	localePath);
	snprintf(szItemDesc,	sizeof(szItemDesc),	"%s/itemdesc.txt",	localePath);	
	snprintf(szMobProto,	sizeof(szMobProto),	"%s/mob_proto",		localePath);	
	snprintf(szSkillDescFileName, sizeof(szSkillDescFileName),	"%s/SkillDesc.txt", localePath);
	snprintf(szSkillTableFileName, sizeof(szSkillTableFileName), "locale/common/SkillTable.txt");
	snprintf(szInsultList,	sizeof(szInsultList),	"%s/insult.txt", localePath);
#ifdef ENABLE_GROWTH_PET_SYSTEM
	snprintf(szSkillPetFileName, sizeof(szSkillPetFileName), "%s/pet_skill.txt", localePath);
	rkSkillPetMgr.Destroy();
#endif
	rkNPCMgr.Destroy();
	rkItemMgr.Destroy();
	rkSkillMgr.Destroy();

	if (!rkItemMgr.LoadItemList(szItemList))
	{
		TraceError("LoadLocaleData - LoadItemList(%s) Error", szItemList);
	}	

	if (!rkItemMgr.LoadItemTable(szItemProto))
	{
		TraceError("LoadLocaleData - LoadItemProto(%s) Error", szItemProto);
		return false;
	}

	if (!rkItemMgr.LoadItemDesc(szItemDesc))
	{
		Tracenf("LoadLocaleData - LoadItemDesc(%s) Error", szItemDesc);	
	}

	if (!rkNPCMgr.LoadNonPlayerData(szMobProto))
	{
		TraceError("LoadLocaleData - LoadMobProto(%s) Error", szMobProto);
		return false;
	}

	if (!rkSkillMgr.RegisterSkillDesc(szSkillDescFileName))
	{
		TraceError("LoadLocaleData - RegisterSkillDesc(%s) Error", szMobProto);
		return false;
	}

	if (!rkSkillMgr.RegisterSkillTable(szSkillTableFileName))
	{
		TraceError("LoadLocaleData - RegisterSkillTable(%s) Error", szMobProto);
		return false;
	}

#ifdef ENABLE_LOADING_TIP_INFO
	char szTip[256];
	snprintf(szTip, sizeof(szTip), "%s/loading_tip_vnum.txt", localePath);
	if (!rkNetStream.LoadTipVnum(szTip)) {
		TraceError("LoadLocaleData - LoadTipVnum(%s) Error", szTip);
		return false;
	}
	snprintf(szTip, sizeof(szTip), "locale/common/loading_tip_list.txt", localePath);
	if (!rkNetStream.LoadTipList(szTip)) {
		TraceError("LoadLocaleData - LoadTipList(%s) Error", szTip);
		return false;
	}
#endif

	if (!rkNetStream.LoadInsultList(szInsultList))
	{
		Tracenf("CPythonApplication - CPythonNetworkStream::LoadInsultList(%s)", szInsultList);				
	}

#ifdef ENABLE_GROWTH_PET_SYSTEM
	if (!rkSkillPetMgr.RegisterSkillPet(szSkillPetFileName))
	{
		TraceError("LoadLocaleData - RegisterSkillPet(%s) Error", szSkillPetFileName);
		return false;
	}
#endif

	if (LocaleService_IsYMIR())
	{	
		char szEmpireTextConvFile[256];
		for (DWORD dwEmpireID=1; dwEmpireID<=3; ++dwEmpireID)
		{			
			sprintf(szEmpireTextConvFile, "%s/lang%d.cvt", localePath, dwEmpireID);
			if (!rkNetStream.LoadConvertTable(dwEmpireID, szEmpireTextConvFile))
			{
				TraceError("LoadLocaleData - CPythonNetworkStream::LoadConvertTable(%d, %s) FAILURE", dwEmpireID, szEmpireTextConvFile);			
			}
		}
	}

#ifdef ENABLE_SASH_SYSTEM
	char szItemScale[256];
	snprintf(szItemScale, sizeof(szItemScale), "locale/common/item_scale.txt", localePath);
	if (!rkItemMgr.LoadItemScale(szItemScale))
		Tracenf("LoadLocaleData: error while loading %s.", szItemScale);
#endif
#ifdef ENABLE_BATTLEPASS
	BattlePassManager::Instance().LoadBattlePass();
#endif

#ifdef ENABLE_TITLE_SYSTEM
	CPythonTitleManager& rkInTitleResourceList = CPythonTitleManager::Instance();
	char szLoadingTitleResourceList[256];
	snprintf(szLoadingTitleResourceList, sizeof(szLoadingTitleResourceList), "locale/common/title_resource_list.txt", localePath);
	if (!rkInTitleResourceList.LoadTitleResourceList(szLoadingTitleResourceList))
	{
		TraceError("LoadLocaleData - LoadTitleResourceList(%s) Error", szLoadingTitleResourceList);
		return false;
	}
#endif

#ifdef ENABLE_INGAME_WIKI
	CPythonWiki& rkWiki = CPythonWiki::Instance();
	if (!rkWiki.LoadJsonFiles()) {
		TraceError("LoadLocaleData - Wiki LoadJsonFiles() Warning (Files not yet extracted or missing)");
	}
#endif

	NANOEND
		return true;
}
// END_OF_SUPPORT_NEW_KOREA_SERVER

unsigned __GetWindowMode(bool windowed)
{
	if (windowed)
		return WS_OVERLAPPED | WS_CAPTION |   WS_SYSMENU | WS_MINIMIZEBOX;

	return WS_POPUP;
}

bool CPythonApplication::Create(PyObject * poSelf, const char * c_szName, int width, int height, int Windowed)
{
	NANOBEGIN
		Windowed = CPythonSystem::Instance().IsWindowed() ? 1 : 0;

	bool bAnotherWindow = false;

	if (FindWindow(NULL, c_szName))
		bAnotherWindow = true;

	m_dwWidth = width;
	m_dwHeight = height;

	// Window
	UINT WindowMode = __GetWindowMode(Windowed ? true : false);
	WindowMode |= WS_CLIPCHILDREN; // CEF alt pencereleri (nesne market vb.) ile DirectX'in çakışmasını engeller

	if (!CMSWindow::Create(c_szName, 4, 0, WindowMode, ::LoadIcon( GetInstance(), MAKEINTRESOURCE( IDI_METIN2 ) ), IDC_CURSOR_NORMAL, width, height))
	{
		//PyErr_SetString(PyExc_RuntimeError, "CMSWindow::Create failed");
		TraceError("CMSWindow::Create failed");
		SET_EXCEPTION(CREATE_WINDOW);
		return false;
	}

#ifdef USE_NPROTECT_GAMEGUARD
	if (!GameGuard_Run(CMSWindow::GetWindowHandle()))
		return false;
#endif
#ifdef XTRAP_CLIENT_ENABLE
	if (!XTrap_CheckInit())
		return false;
#endif

	if (m_pySystem.IsUseDefaultIME())
	{
		CPythonIME::Instance().UseDefaultIME();
	}

	// 풀스크린 모드이고
	// 디폴트 IME 를 사용하거나 유럽 버전이면
	// 윈도우 풀스크린 모드를 사용한다
	if (!m_pySystem.IsWindowed() && (m_pySystem.IsUseDefaultIME() || LocaleService_IsEUROPE()))
	{
		m_isWindowed = false;
		m_isWindowFullScreenEnable = TRUE;
		__SetFullScreenWindow(GetWindowHandle(), width, height, m_pySystem.GetBPP());

		Windowed = true;
	}
	else
	{
		AdjustSize(m_pySystem.GetWidth(), m_pySystem.GetHeight());

		if (Windowed)
		{
			m_isWindowed = true;

			RECT workArea;
			SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

			const UINT workAreaWidth = (workArea.right - workArea.left);
			const UINT workAreaHeight = (workArea.bottom - workArea.top);

			const UINT windowWidth = m_pySystem.GetWidth() + GetSystemMetrics(SM_CXBORDER) * 2 + GetSystemMetrics(SM_CXDLGFRAME) * 2 + GetSystemMetrics(SM_CXFRAME) * 2;
			const UINT windowHeight = m_pySystem.GetHeight() + GetSystemMetrics(SM_CYBORDER) * 2 + GetSystemMetrics(SM_CYDLGFRAME) * 2 + GetSystemMetrics(SM_CYFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION);

			const UINT x = workAreaWidth / 2 - windowWidth / 2;
			const UINT y = workAreaHeight / 2 - windowHeight / 2;

			SetPosition(x, y);
		}
		else
		{
			m_isWindowed = false;
			SetPosition(0, 0);
		}
	}

	NANOEND
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// Cursor
		if (!CreateCursors())
		{
			//PyErr_SetString(PyExc_RuntimeError, "CMSWindow::Cursors Create Error");
			TraceError("CMSWindow::Cursors Create Error");
			SET_EXCEPTION("CREATE_CURSOR");
			return false;
		}

		if (!m_pySystem.IsNoSoundCard())
		{
			// Sound
			if (!m_SoundManager.Create())
			{
				// NOTE : 중국측의 요청으로 생략
				//		LogBox(ApplicationStringTable_GetStringz(IDS_WARN_NO_SOUND_DEVICE));
			}
		}

		extern bool GRAPHICS_CAPS_SOFTWARE_TILING;

		if (!m_pySystem.IsAutoTiling())
			GRAPHICS_CAPS_SOFTWARE_TILING = m_pySystem.IsSoftwareTiling();

		// Device
#if defined(__ANDROID__) || defined(__APPLE__) || defined(__IOS__)
		if (!CreateDevice(width, height, Windowed, 32, 60))
			return false;
		m_pySystem.SetResolution(width, height);
#if defined(__ANDROID__)
		m_kTouchControls.UpdateScreenSize(width, height);
#endif
#elif defined(ENABLE_GPU_CONFIG)
		if (!CreateDevice(m_pySystem.GetWidth(), m_pySystem.GetHeight(), Windowed, m_pySystem.GetBPP(), m_pySystem.GetFrequency(), m_pySystem.GetGPU()))
			return false;
#else
		if (!CreateDevice(m_pySystem.GetWidth(), m_pySystem.GetHeight(), Windowed, m_pySystem.GetBPP(), m_pySystem.GetFrequency()))
			return false;
#endif

#ifdef ENABLE_GPU_CONFIG
		if (std::string(m_pySystem.GetGPU()).empty())
		{
			const char* detectedGPU = CGraphicDevice::GetCurrentAdapterDescription();
			if (detectedGPU && *detectedGPU)
			{
				m_pySystem.SetGPU(detectedGPU);
				m_pySystem.SaveConfig();
			}
		}
#endif

		GrannyCreateSharedDeformBuffer();

		if (m_pySystem.IsAutoTiling())
		{
			if (m_grpDevice.IsFastTNL())
			{
				m_pyBackground.ReserveSoftwareTilingEnable(false);
			}
			else
			{
				m_pyBackground.ReserveSoftwareTilingEnable(true);
			}
		}
		else
		{
			m_pyBackground.ReserveSoftwareTilingEnable(m_pySystem.IsSoftwareTiling());
		}

		SetVisibleMode(true);
#if defined(__ANDROID__) || defined(__APPLE__) || defined(__IOS__)
		m_isActive = true;
		m_isActivateWnd = true;
#endif

		if (m_isWindowFullScreenEnable) //m_pySystem.IsUseDefaultIME() && !m_pySystem.IsWindowed())
		{
			SetWindowPos(GetWindowHandle(), HWND_TOP, 0, 0, width, height, SWP_SHOWWINDOW);
		}

		if (!InitializeKeyboard(GetWindowHandle()))
			return false;

		m_pySystem.GetDisplaySettings();

		// Mouse
		if (m_pySystem.IsSoftwareCursor())
			SetCursorMode(CURSOR_MODE_SOFTWARE);
		else
			SetCursorMode(CURSOR_MODE_HARDWARE);

		// Network
		if (!m_netDevice.Create())
		{
			//PyErr_SetString(PyExc_RuntimeError, "NetDevice::Create failed");
			TraceError("NetDevice::Create failed");
			SET_EXCEPTION("CREATE_NETWORK");
			return false;
		}

		if (!m_grpDevice.IsFastTNL())
			CGrannyLODController::SetMinLODMode(true);

		m_pyItem.Create();

		// Other Modules
		DefaultFont_Startup();

		CPythonIME::Instance().Create(GetWindowHandle());
		CPythonIME::Instance().SetText("", 0);
		CPythonTextTail::Instance().Initialize();

		// Light Manager
		m_LightManager.Initialize();

		CGraphicImageInstance::CreateSystem(32);

		// 백업
		STICKYKEYS sStickKeys;
		memset(&sStickKeys, 0, sizeof(sStickKeys));
		sStickKeys.cbSize = sizeof(sStickKeys);
		SystemParametersInfo( SPI_GETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0 );
		m_dwStickyKeysFlag = sStickKeys.dwFlags;

		// 설정
		sStickKeys.dwFlags &= ~(SKF_AVAILABLE|SKF_HOTKEYACTIVE);
		SystemParametersInfo( SPI_SETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0 );

		// SphereMap
		CGrannyMaterial::CreateSphereMap(0, "d:/ymir work/special/spheremap.jpg");
		CGrannyMaterial::CreateSphereMap(1, "d:/ymir work/special/spheremap01.jpg");
		return true;
}

void CPythonApplication::SetGlobalCenterPosition(LONG x, LONG y)
{
	CPythonBackground& rkBG=CPythonBackground::Instance();
	rkBG.GlobalPositionToLocalPosition(x, y);

	float z = CPythonBackground::Instance().GetHeight(x, y);

	CPythonApplication::Instance().SetCenterPosition(x, y, z);
}

void CPythonApplication::SetCenterPosition(float fx, float fy, float fz)
{
	m_v3CenterPosition.x = +fx;
	m_v3CenterPosition.y = -fy;
	m_v3CenterPosition.z = +fz;
}

void CPythonApplication::GetCenterPosition(TPixelPosition * pPixelPosition)
{
	pPixelPosition->x = +m_v3CenterPosition.x;
	pPixelPosition->y = -m_v3CenterPosition.y;
	pPixelPosition->z = +m_v3CenterPosition.z;
}


void CPythonApplication::SetServerTime(time_t tTime)
{
	m_dwStartLocalTime	= ELTimer_GetMSec();
	m_tServerTime		= tTime;
	m_tLocalStartTime	= time(0);
}

time_t CPythonApplication::GetServerTime()
{
	return (ELTimer_GetMSec() - m_dwStartLocalTime) + m_tServerTime;
}

// 2005.03.28 - MALL 아이템에 들어있는 시간의 단위가 서버에서 time(0) 으로 만들어지는
//              값이기 때문에 단위를 맞추기 위해 시간 관련 처리를 별도로 추가
time_t CPythonApplication::GetServerTimeStamp()
{
	return (time(0) - m_tLocalStartTime) + m_tServerTime;
}

float CPythonApplication::GetGlobalTime()
{
	return m_fGlobalTime;
}

float CPythonApplication::GetGlobalElapsedTime()
{
	return m_fGlobalElapsedTime;
}

void CPythonApplication::SetFPS(int iFPS)
{
	m_iFPS = iFPS;
}

int CPythonApplication::GetWidth()
{
	return m_dwWidth;
}

int CPythonApplication::GetHeight()
{
	return m_dwHeight;
}

void CPythonApplication::SetConnectData(const char * c_szIP, int iPort)
{
	m_strIP = c_szIP;
	m_iPort = iPort;
}

void CPythonApplication::GetConnectData(std::string & rstIP, int & riPort)
{
	rstIP	= m_strIP;
	riPort	= m_iPort;
}

void CPythonApplication::EnableSpecialCameraMode()
{
	m_isSpecialCameraMode = TRUE;
}

void CPythonApplication::SetCameraSpeed(int iPercentage)
{
	m_fCameraRotateSpeed = c_fDefaultCameraRotateSpeed * float(iPercentage) / 100.0f;
	m_fCameraPitchSpeed = c_fDefaultCameraPitchSpeed * float(iPercentage) / 100.0f;
	m_fCameraZoomSpeed = c_fDefaultCameraZoomSpeed * float(iPercentage) / 100.0f;
}

#ifdef ENABLE_SHAKE_CAMERA
void CPythonApplication::SetShakeCamera(float fDuration, int iLevel)
{
	if (fDuration <= 0.0f || iLevel <= 0)
	{
		m_fShakeCameraEndTime = 0.0f;
		m_fShakeCameraNextTime = 0.0f;
		m_iShakeCameraLevel = 0;
		m_v3ShakeCameraOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		return;
	}

	m_iShakeCameraLevel = MINMAX(1, iLevel, 10);
	m_fShakeCameraEndTime = CTimer::Instance().GetCurrentSecond() + fDuration;
	m_fShakeCameraNextTime = 0.0f;
	m_v3ShakeCameraOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
}
#endif

void CPythonApplication::SetForceSightRange(int iRange)
{
	m_iForceSightRange = iRange;
}

void CPythonApplication::SetTitle(const char* szTitle)
{
	CMSWindow::SetText(szTitle);
}

void CPythonApplication::Clear()
{
	m_pySystem.Clear();
}

#ifdef ENABLE_WINDOW_MESSAGE
#include <Windows.h>
void CPythonApplication::FlashApplication()
{
	HWND hWnd = GetWindowHandle();
	FLASHWINFO fi;
	fi.cbSize = sizeof(FLASHWINFO);
	fi.hwnd = hWnd;
	fi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
	fi.uCount = 0;
	fi.dwTimeout = 0;
	FlashWindowEx(&fi);
}
#endif

void CPythonApplication::Destroy()
{
#if !defined(__ANDROID__) && !defined(__APPLE__)
#ifdef CEF_BROWSER

	CefWebBrowser_Destroy();
#else
	WebBrowser_Destroy();
#endif
#endif

	// SphereMap
	CGrannyMaterial::DestroySphereMap();

	m_kWndMgr.Destroy();

	CPythonSystem::Instance().SaveConfig();

#ifdef ENABLE_INGAME_WIKI
	m_pyWikiModelViewManager.InitializeData();
#endif

	m_kRenderTargetManager.Destroy();

#ifdef ENABLE_TITLE_SYSTEM
	m_pyTitleSystem.Destroy();
#endif

	DestroyCollisionInstanceSystem();

	m_pySystem.SaveInterfaceStatus();

	m_pyEventManager.Destroy();	
	m_FlyingManager.Destroy();

	m_pyMiniMap.Destroy();

	m_pyTextTail.Destroy();
	m_pyChat.Destroy();	
	m_kChrMgr.Destroy();
	m_RaceManager.Destroy();

	m_pyItem.Destroy();
	m_kItemMgr.Destroy();

	m_pyBackground.Destroy();

	m_kEftMgr.Destroy();
	m_LightManager.Destroy();

	// DEFAULT_FONT
	DefaultFont_Cleanup();
	// END_OF_DEFAULT_FONT

	GrannyDestroySharedDeformBuffer();

	m_pyGraphic.Destroy();
	//m_pyNetworkDatagram.Destroy();	

	m_pyRes.Destroy();

	m_kGuildMarkDownloader.Disconnect();

	CGrannyModelInstance::DestroySystem();
	CGraphicImageInstance::DestroySystem();


	m_SoundManager.Destroy();
	m_grpDevice.Destroy();

	// FIXME : 만들어져 있지 않음 - [levites]
	//CSpeedTreeForestDirectX8::Instance().Clear();

	CAttributeInstance::DestroySystem();
	CTextFileLoader::DestroySystem();
	DestroyCursors();

	CMSApplication::Destroy();

	STICKYKEYS sStickKeys;
	memset(&sStickKeys, 0, sizeof(sStickKeys));
	sStickKeys.cbSize = sizeof(sStickKeys);
	sStickKeys.dwFlags = m_dwStickyKeysFlag;
	SystemParametersInfo( SPI_SETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0 );
}

#ifdef ENABLE_MULTI_FARM_BLOCK
void CPythonApplication::MultiFarmBlockIcon(BYTE bStatus)
{
	HICON exeIcon = bStatus ? LoadIcon(ms_hInstance, MAKEINTRESOURCE(IDI_METIN2)) : LoadIcon(ms_hInstance, MAKEINTRESOURCE(BLOCK_METIN2));
	SendMessage(GetWindowHandle(), WM_SETICON, ICON_BIG, (LPARAM)exeIcon);
}
#endif

#ifdef ENABLE_TITLE_SYSTEM
void CPythonApplication::TitleCreate()
{
	CPythonTitleManager::Instance().Create(m_dwWidth, m_dwHeight);
}
#endif
