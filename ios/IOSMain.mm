#include "UserInterface/StdAfx.h"
#include "PythonApplication.h"
#include "PythonPlayer.h"
#include "PythonNetworkStream.h"
#include "../eterlib/Camera.h"
#include "../MilesLib/SoundBase.h"
#include "../MilesLib/SoundManager.h"
#include "PythonIME.h"
#include "../eterPack/EterPackManager.h"
#include "../eterBase/lzo.h"
#include "EventHandler.h"
#include "../ScriptLib/PythonLauncher.h"
#include "PythonExceptionSender.h"

#ifdef interface
#undef interface
#endif
#ifdef now
#undef now
#endif

#import "IOSMain.h"
#import "GameViewController.h"

#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

extern "C" void IOS_WriteSysErr(const char* msg) {
	@autoreleasepool {
		NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
		NSString *docsPath = paths.firstObject;
		if (docsPath && msg) {
			NSString *syserrPath = [docsPath stringByAppendingPathComponent:@"syserr.txt"];
			FILE* f = fopen([syserrPath UTF8String], "a");
			if (f) {
				time_t tNow = time(0);
				struct tm tmNow = *localtime(&tNow);
				fprintf(f, "%02d%02d %02d:%02d:%02d :: %s\n",
						tmNow.tm_mon + 1, tmNow.tm_mday, tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec,
						msg);
				fflush(f);
				fclose(f);
			}
		}
	}
}

extern void initpack();
extern void initdbg();
extern void initime();
extern void initgrp();
extern void initgrpImage();
extern void initgrpText();
extern void initwndMgr();
extern void initudp();
extern void initapp();
extern void initsystem();
extern void initchr();
extern void initchrmgr();
extern void initPlayer();
extern void initItem();
extern void initNonPlayer();
extern void initTrade();
extern void initChat();
extern void initTextTail();
extern void initnet();
extern void initMiniMap();
extern void initProfiler();
extern void initEvent();
extern void initeffect();
extern void initfly();
extern void initsnd();
extern void initeventmgr();
extern void initshop();
extern void initquest();
extern void initskill();
extern void initBackground();
extern void initMessenger();
extern void initsafebox();
extern void initguild();
extern void initServerStateChecker();
extern void initRenderTarget();

#ifdef ENABLE_MESSENGER_RENEWAL
extern void initCommunity();
#endif
#ifdef ENABLE_INGAME_WIKI
extern void initWiki();
#endif
#ifdef ENABLE_VOTE4BUFF
extern void initM2WebSocket();
#endif
#ifdef ENABLE_BATTLEPASS
extern void initBattlePass();
#endif
#ifdef ENABLE_BATTLE_FIELD
extern void initCombatZoneSystem();
#endif
#ifdef ENABLE_TITLE_SYSTEM
extern void initTitleSystem();
#endif
#ifdef ENABLE_TREASURE_EVENT
extern void inittreasure_event();
#endif
#ifdef ENABLE_BATTLE_ROYALE
extern void initbattleRoyaleMgr();
#endif
#ifdef ENABLE_SWITCHBOT
extern void initSwitchbot();
#endif
#ifdef ENABLE_EVENT_SYSTEM
extern void initGameEvents();
#endif
#ifdef ENABLE_CUBE_RENEWAL
extern void intcuberenewal();
#endif
#ifdef ENABLE_SASH_SYSTEM
extern void initSash();
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
extern void initChangeLook();
#endif
#ifdef ENABLE_AURA_SYSTEM
extern void initAura();
#endif
#ifdef ENABLE_SHOP_SEARCH_SYSTEM
extern void initprivateShopSearch();
#endif
#ifdef ENABLE_RIDING_EXTENDED
extern void initmountupgrade();
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
extern void initskillpet();
#endif

static bool RunMainScript(CPythonLauncher& pyLauncher, const char* lpCmdLine) {
	initpack();
	initdbg();
	initime();
	initgrp();
	initgrpImage();
	initgrpText();
	initwndMgr();
	initudp();
	initapp();
	initsystem();
	initchr();
	initchrmgr();
	initPlayer();
	initItem();
	initNonPlayer();
	initTrade();
	initChat();
	initTextTail();
	initnet();
	initMiniMap();
	initProfiler();
	initEvent();
	initeffect();
	initfly();
	initsnd();
	initeventmgr();
	initshop();
	initskill();
	initquest();
	initBackground();
	initMessenger();
	initsafebox();
	initguild();
	initServerStateChecker();
	initRenderTarget();

#ifdef ENABLE_MESSENGER_RENEWAL
	initCommunity();
#endif
#ifdef ENABLE_INGAME_WIKI
	initWiki();
#endif
#ifdef ENABLE_VOTE4BUFF
	initM2WebSocket();
#endif
#ifdef ENABLE_BATTLEPASS
	initBattlePass();
#endif
#ifdef ENABLE_BATTLE_FIELD
	initCombatZoneSystem();
#endif
#ifdef ENABLE_TITLE_SYSTEM
	initTitleSystem();
#endif
#ifdef ENABLE_TREASURE_EVENT
	inittreasure_event();
#endif
#ifdef ENABLE_BATTLE_ROYALE
	initbattleRoyaleMgr();
#endif
#ifdef ENABLE_SWITCHBOT
	initSwitchbot();
#endif
#ifdef ENABLE_EVENT_SYSTEM
	initGameEvents();
#endif
#ifdef ENABLE_CUBE_RENEWAL
	intcuberenewal();
#endif
#ifdef ENABLE_SASH_SYSTEM
	initSash();
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
	initChangeLook();
#endif
#ifdef ENABLE_AURA_SYSTEM
	initAura();
#endif
#ifdef ENABLE_SHOP_SEARCH_SYSTEM
	initprivateShopSearch();
#endif
#ifdef ENABLE_RIDING_EXTENDED
	initmountupgrade();
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
	initskillpet();
#endif

	pyLauncher.RunLine("import sys");
	pyLauncher.RunLine("sys.path.append('lib')");
	pyLauncher.RunLine("__DEBUG__ = 1");
	pyLauncher.RunLine("import sys, types");
	pyLauncher.RunLine("if '_locale' not in sys.modules:\n    m = types.ModuleType('_locale')\n    m.Error = Exception\n    m.CHAR_MAX = 127\n    m.LC_ALL, m.LC_COLLATE, m.LC_CTYPE = 0, 1, 2\n    m.LC_MONETARY, m.LC_NUMERIC, m.LC_TIME = 3, 4, 5\n    m.setlocale = lambda *a, **k: 'C'\n    m._getdefaultlocale = lambda *a, **k: ('tr_TR', 'cp1254')\n    m.localeconv = lambda *a, **k: {'decimal_point': '.', 'thousands_sep': ','}\n    m.strcoll = lambda a, b: (a > b) - (a < b)\n    m.strxfrm = lambda s: s\n    sys.modules['_locale'] = m\n");

	return pyLauncher.RunFile("system.py");
}

static std::string g_strBundlePath = "";
static std::string g_strDocsPath = "";

extern "C" {

int g_nAndroidScreenWidth = 1067;
int g_nAndroidScreenHeight = 600;
int g_nAndroidMouseX = 0;
int g_nAndroidMouseY = 0;

bool IOS_Init(const char* bundlePath, const char* docsPath, int width, int height) {
	NSLog(@"[Aspar2 iOS] Initializing Engine (%dx%d)", width, height);

	if ([GameViewController sharedInstance]) {
		[[GameViewController sharedInstance] makeCurrentGLContext];
	}

	if (bundlePath) g_strBundlePath = bundlePath;
	if (docsPath) g_strDocsPath = docsPath;

	g_nAndroidScreenWidth = width;
	g_nAndroidScreenHeight = height;
	g_nAndroidMouseX = width / 2;
	g_nAndroidMouseY = height / 2;

	if (!g_strBundlePath.empty()) {
		chdir(g_strBundlePath.c_str());
		NSLog(@"[Aspar2 iOS] Working directory set to: %s", g_strBundlePath.c_str());
	}

	static CLZO lzo;
	static CEterPackManager EterPackManager;
	static EventHandler eventHandler;

	CEterPackManager::Instance().SetRelativePathMode();
	CEterPackManager::Instance().SetCacheMode();
	CEterPackManager::Instance().SetSearchMode(true);

	auto regPack = [&](const char* name, const char* dirPrefix = "*") {
		std::string baseName = name;
		size_t slashPos = baseName.find_last_of('/');
		if (slashPos != std::string::npos) baseName = baseName.substr(slashPos + 1);

		// 1. Check Documents directory (downloaded patches / otopatcher)
		if (!g_strDocsPath.empty()) {
			std::string docPack = g_strDocsPath + "/pack/" + baseName;
			if (access((docPack + ".index").c_str(), R_OK) == 0) {
				bool ok = CEterPackManager::Instance().RegisterPack(docPack.c_str(), dirPrefix);
				NSLog(@"[Aspar2 iOS] RegisterPack (DOCS) %s: %s", docPack.c_str(), ok ? "OK" : "FAILED");
				if (ok) return true;
			}
			std::string docAspar2Pack = g_strDocsPath + "/aspar2/pack/" + baseName;
			if (access((docAspar2Pack + ".index").c_str(), R_OK) == 0) {
				bool ok = CEterPackManager::Instance().RegisterPack(docAspar2Pack.c_str(), dirPrefix);
				NSLog(@"[Aspar2 iOS] RegisterPack (ASPAR2 DOCS) %s: %s", docAspar2Pack.c_str(), ok ? "OK" : "FAILED");
				if (ok) return true;
			}
		}

		// 2. Fallback to app bundle
		bool ok = CEterPackManager::Instance().RegisterPack(name, dirPrefix);
		NSLog(@"[Aspar2 iOS] RegisterPack (BUNDLE) %s: %s", name, ok ? "OK" : "FAILED");
		return ok;
	};

	// Register root pack
	if (!g_strDocsPath.empty() && access((g_strDocsPath + "/pack/root.index").c_str(), R_OK) == 0) {
		std::string docRoot = g_strDocsPath + "/pack/root";
		CEterPackManager::Instance().RegisterRootPack(docRoot.c_str());
		NSLog(@"[Aspar2 iOS] RegisterRootPack (DOCS): OK");
	} else if (!g_strDocsPath.empty() && access((g_strDocsPath + "/aspar2/pack/root.index").c_str(), R_OK) == 0) {
		std::string docRoot = g_strDocsPath + "/aspar2/pack/root";
		CEterPackManager::Instance().RegisterRootPack(docRoot.c_str());
		NSLog(@"[Aspar2 iOS] RegisterRootPack (ASPAR2 DOCS): OK");
	} else {
		CEterPackManager::Instance().RegisterRootPack("pack/root");
		NSLog(@"[Aspar2 iOS] RegisterRootPack (BUNDLE): OK");
	}

	struct PackEntry { const char* dir; const char* name; };
	static const PackEntry s_packEntries[] = {
		{ "pack/", "metin2_patch_maps" },
		{ "pack/", "metin2_patch_snow" },
		{ "pack/", "metin2_patch_w20_sound" },
		{ "pack/", "metin2_patch_party" },
		{ "pack/", "metin2_patch_dss" },
		{ "pack/", "metin2_patch_dragon_rock" },
		{ "pack/", "metin2_patch_dawnmistwood" },
		{ "pack/", "metin2_patch_5th_armor" },
		{ "pack/", "metin2_patch_easter_2017" },
		{ "pack/", "metin2_patch_ramadan" },
		{ "pack/", "metin2_patch_costume" },
		{ "pack/", "metin2_patch_etc" },
		{ "pack/", "metin2_patch_halloween" },
		{ "pack/", "metin2_patch_pet" },
		{ "pack/", "metin2_patch_mount" },
		{ "season1/", "season1" },
		{ "season2/", "season2" },
		{ "sound/monster/", "sound" },
		{ "sound/npc/", "sound" },
		{ "sound/pc/", "sound" },
		{ "sound/ui/", "sound" },
		{ "sound/ambience/", "sound_m" },
		{ "sound/common/", "sound_m" },
		{ "sound/effect/", "sound_m" },
		{ "sound/monster/", "sound_m" },
		{ "sound/npc/", "sound_m" },
		{ "sound/pc/", "sound_m" },
		{ "sound/ui/", "sound_m" },
		{ "sound/monster2/", "sound2" },
		{ "sound/pc2/", "sound2" },
		{ "bgm/", "bgm" },
		{ "d:/ymir work/special/", "etc" },
		{ "d:/ymir work/environment/", "etc" },
		{ "locale/tr/", "locale" },
		{ "uiscript/", "uiscript" },
		{ "d:/ymir work/ui/", "etc" },
	};

	for (const auto& entry : s_packEntries) {
		std::string packTarget = std::string("pack/") + entry.name;
		regPack(packTarget.c_str(), entry.dir);
	}

	regPack("pack/locale", "*");
	regPack("pack/etc", "*");
	regPack("pack/tree", "*");
	regPack("pack/property", "*");
	regPack("pack/terrain", "*");
	regPack("pack/zone", "*");

	LocaleService_LoadConfig("locale.cfg");
	if (strcmp(LocaleService_GetLocaleName(), "ymir") == 0 || strlen(LocaleService_GetLocaleName()) == 0) {
		LocaleService_ForceSetLocale("tr", "locale/tr");
	}
	NSLog(@"[Aspar2 iOS] Locale: %s (%s)", LocaleService_GetLocaleName(), LocaleService_GetLocalePath());

	static CPythonApplication app;

	if ([GameViewController sharedInstance]) {
		[[GameViewController sharedInstance] makeCurrentGLContext];
	}

	if (!app.Create(NULL, "Aspar2", width, height, 1)) {
		NSLog(@"[Aspar2 iOS] CPythonApplication::Create failed!");
		TraceError("IOS_Init: CPythonApplication::Create failed!");
		return false;
	}

	UI::CWindowManager::Instance().SetScreenSize(width, height);
	if (CPythonSystem::InstancePtr()) {
		CPythonSystem::Instance().GetConfig()->width = width;
		CPythonSystem::Instance().GetConfig()->height = height;
	}

	static CPythonLauncher pyLauncher;
	static CPythonExceptionSender pyExceptionSender;
	SetExceptionSender(&pyExceptionSender);

	bool scriptSuccess = false;
	if (pyLauncher.Create()) {
		NSLog(@"[Aspar2 iOS] Running system.py...");
		if (!RunMainScript(pyLauncher, "")) {
			NSLog(@"[Aspar2 iOS] RunMainScript failed!");
			TraceError("IOS_Init: RunMainScript(system.py) failed!");
		} else {
			NSLog(@"[Aspar2 iOS] RunMainScript completed successfully!");
			scriptSuccess = true;
		}
	} else {
		NSLog(@"[Aspar2 iOS] pyLauncher.Create() failed!");
		TraceError("IOS_Init: pyLauncher.Create() failed!");
	}

	NSLog(@"[Aspar2 iOS] Metin2 Engine Initialized Result: %s", scriptSuccess ? "SUCCESS" : "FAILED");
	return scriptSuccess;
}

void IOS_Render() {
	if (CPythonApplication::Instance().IsExit()) {
		return;
	}
	CPythonApplication::Instance().Process();
}

static bool s_bActionIsUI = false;
static bool s_bActionIsWorld = false;
static bool s_bActionConverted = false;
static int s_nActionLastX = 0;
static int s_nActionLastY = 0;

static float s_fJoystickX = 0.0f;
static float s_fJoystickY = -1.0f;
static float s_fJoystickW = 194.0f;
static float s_fJoystickH = 194.0f;

static bool IOS_IsJoystickZone(float x, float y) {
	UI::CWindowManager & rkWndMgr = UI::CWindowManager::Instance();
	long sh = rkWndMgr.GetScreenHeight();
	float jx = s_fJoystickX;
	float jy = (s_fJoystickY >= 0.0f) ? s_fJoystickY : (float)(sh - s_fJoystickH);
	float jw = s_fJoystickW;
	float jh = s_fJoystickH;
	return (x >= jx && x <= jx + jw && y >= jy && y <= jy + jh);
}

static bool IOS_IsHudZone(long x, long y) {
	UI::CWindowManager & rkWndMgr = UI::CWindowManager::Instance();
	long sw = rkWndMgr.GetScreenWidth();
	long sh = rkWndMgr.GetScreenHeight();
	return IOS_IsJoystickZone((float)x, (float)y) || (x > sw - 370 && y > sh - 320) || (y < 65);
}

static void IOS_RotateCamera(float deltaX, float deltaY) {
	CCamera* pkCmrCur = CCameraManager::Instance().GetCurrentCamera();
	if (pkCmrCur && !pkCmrCur->IsLock()) {
		float fRollDelta = deltaX * 0.30f;
		float fPitchDelta = deltaY * 0.20f;
		pkCmrCur->RotateEyeAroundTarget(fPitchDelta, fRollDelta);
	}
}

void IOS_JoystickTouch(int action, float x, float y) {
	CPythonPlayer::Instance().OnMobileJoystick(action, x, y);
}

void IOS_ActionTouch(int action, float x, float y, float deltaX, float deltaY, bool isDrag) {
	int ix = static_cast<int>(x);
	int iy = static_cast<int>(y);

	if (action == 0) { // ACTION_DOWN
		bool bIsUI = UI::CWindowManager::Instance().IsUIWindowAt(ix, iy);
		s_bActionConverted = false;
		s_nActionLastX = ix;
		s_nActionLastY = iy;
		if (bIsUI) {
			s_bActionIsUI = true;
			s_bActionIsWorld = UI::CWindowManager::Instance().IsGameWorldAt(ix, iy) && !IOS_IsHudZone(ix, iy);
			CPythonApplication::Instance().OnMouseMove(ix, iy);
			CPythonApplication::Instance().OnMouseLeftButtonDown(ix, iy);
		} else {
			s_bActionIsUI = false;
			s_bActionIsWorld = false;
		}
	} else if (action == 2) { // ACTION_MOVE
		s_nActionLastX = ix;
		s_nActionLastY = iy;
		if (s_bActionConverted) {
			IOS_RotateCamera(deltaX, deltaY);
		} else if (s_bActionIsUI) {
			CPythonApplication::Instance().OnMouseMove(ix, iy);
		} else {
			IOS_RotateCamera(deltaX, deltaY);
		}
	} else if (action == 1) { // ACTION_UP
		if (!s_bActionConverted && s_bActionIsUI) {
			CPythonApplication::Instance().OnMouseMove(ix, iy);
			CPythonApplication::Instance().OnMouseLeftButtonUp(ix, iy);
		}
		s_bActionIsUI = false;
		s_bActionIsWorld = false;
		s_bActionConverted = false;
	}
}

void IOS_SecondaryTouch(int action, float x, float y, float deltaX, float deltaY) {
	if (action == 2) {
		float fRollDelta = deltaX * 0.30f;
		float fPitchDelta = -deltaY * 0.20f;
		CCamera* pkCmrCur = CCameraManager::Instance().GetCurrentCamera();
		if (pkCmrCur && !pkCmrCur->IsLock()) {
			pkCmrCur->RotateEyeAroundTarget(fPitchDelta, fRollDelta);
		}
	}
}

void IOS_OnKeyboardText(const char* text) {
	if (!text) return;
	CPythonIME::Instance().SetText(text, strlen(text));
	CPythonApplication::Instance().RunIMEUpdate();
}

void IOS_OnKeyboardEnter() {
	CPythonApplication::Instance().RunIMEReturnEvent();
}

void IOS_ShowKeyboard(const char* initialText) {
	[[GameViewController sharedInstance] showKeyboard:initialText];
}

void IOS_HideKeyboard() {
	[[GameViewController sharedInstance] hideKeyboard];
}

void IOS_ShowWebPage(const char* url) {
	[[GameViewController sharedInstance] showWebPage:url];
}

void IOS_HideWebPage() {
	[[GameViewController sharedInstance] hideWebPage];
}

bool IOS_IsWebShowing() {
	return [[GameViewController sharedInstance] isWebShowing];
}

bool IOS_IsGamePhase() {
	if (CPythonNetworkStream::InstancePtr()) {
		return CPythonNetworkStream::Instance().IsGamePhase();
	}
	return false;
}

void IOS_PauseAudio() {
	if (CSoundManager::InstancePtr()) {
		CSoundManager::Instance().SaveVolume();
	}
	CSoundBase::PauseAudio();
}

void IOS_ResumeAudio() {
	CSoundBase::ResumeAudio();
	if (CSoundManager::InstancePtr()) {
		CSoundManager::Instance().RestoreVolume();
	}
}

void IOS_SaveConfig() {
	if (CPythonSystem::InstancePtr()) {
		CPythonSystem::Instance().SaveConfig();
		CPythonSystem::Instance().SaveInterfaceStatus();
	}
}

} // extern "C"
