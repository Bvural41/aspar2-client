#include "StdAfx.h"
#include "PythonApplication.h"
#include "PythonPlayer.h"
#include "PythonNetworkStream.h"
#include "../eterlib/Camera.h"
#include "../MilesLib/SoundBase.h"
#include "../MilesLib/SoundManager.h"

#ifdef __ANDROID__
#include "PythonIME.h"
#include <jni.h>
#include <unistd.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#define LOG_TAG "Metin2Mobile"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#include <signal.h>
#include <unwind.h>
#include <dlfcn.h>
#include <dirent.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <lzo-2.03/lzo1x.h>

struct BacktraceState {
    void** current;
    void** end;
};

static _Unwind_Reason_Code unwind_callback(struct _Unwind_Context* context, void* arg) {
    BacktraceState* state = (BacktraceState*)arg;
    uintptr_t pc = _Unwind_GetIP(context);
    if (pc) {
        if (state->current == state->end) return _URC_END_OF_STACK;
        *state->current++ = (void*)pc;
    }
    return _URC_NO_REASON;
}

static void sigsegv_handler(int sig, siginfo_t* info, void* ucontext) {
    __android_log_print(ANDROID_LOG_FATAL, "Metin2Crash",
        "=== SIGSEGV at addr=%p, si_code=%d ===", info->si_addr, info->si_code);

    void* buffer[64];
    BacktraceState state = {buffer, buffer + 64};
    _Unwind_Backtrace(unwind_callback, &state);
    int count = (int)(state.current - buffer);

    for (int i = 0; i < count; i++) {
        Dl_info dli;
        if (dladdr(buffer[i], &dli)) {
            __android_log_print(ANDROID_LOG_FATAL, "Metin2Crash",
                "#%02d pc %p  %s (%s+0x%lx)",
                i, (void*)((uintptr_t)buffer[i] - (uintptr_t)dli.dli_fbase),
                dli.dli_fname ? dli.dli_fname : "???",
                dli.dli_sname ? dli.dli_sname : "???",
                dli.dli_sname ? (unsigned long)((uintptr_t)buffer[i] - (uintptr_t)dli.dli_saddr) : 0UL);
        } else {
            __android_log_print(ANDROID_LOG_FATAL, "Metin2Crash", "#%02d pc %p  ???", i, buffer[i]);
        }
    }

    // Stack memory scan to uncover callers when unwinder halts at hand-written asm (memmove)
    ucontext_t* uc = (ucontext_t*)ucontext;
#if defined(__x86_64__)
    uintptr_t* sp = (uintptr_t*)uc->uc_mcontext.gregs[REG_RSP];
    __android_log_print(ANDROID_LOG_FATAL, "Metin2Crash", "RIP=%p, RSP=%p", (void*)uc->uc_mcontext.gregs[REG_RIP], (void*)sp);
#elif defined(__aarch64__)
    uintptr_t* sp = (uintptr_t*)uc->uc_mcontext.sp;
    __android_log_print(ANDROID_LOG_FATAL, "Metin2Crash", "PC=%p, SP=%p", (void*)uc->uc_mcontext.pc, (void*)sp);
#else
    uintptr_t* sp = nullptr;
#endif
    if (sp) {
        for (int s = 0; s < 64; s++) {
            uintptr_t val = sp[s];
            Dl_info dli;
            if (dladdr((void*)val, &dli) && dli.dli_fname) {
                __android_log_print(ANDROID_LOG_FATAL, "Metin2Crash",
                    "STACK[%02d]=%p pc=%p in %s (%s+0x%lx)", s, (void*)val,
                    (void*)(val - (uintptr_t)dli.dli_fbase),
                    dli.dli_fname, dli.dli_sname ? dli.dli_sname : "???",
                    dli.dli_sname ? (unsigned long)(val - (uintptr_t)dli.dli_saddr) : 0UL);
            }
        }
    }

    // Re-raise to get the default crash dump
    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGSEGV, &sa, NULL);
    raise(SIGSEGV);
}

static void install_sigsegv_handler() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = sigsegv_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGSEGV, &sa, NULL);
    __android_log_print(ANDROID_LOG_INFO, "Metin2Crash", "Custom SIGSEGV handler installed");
}

#include "../eterPack/EterPackManager.h"
#include "../eterBase/lzo.h"
#include "EventHandler.h"
#include "../ScriptLib/PythonLauncher.h"
#include "PythonExceptionSender.h"

AAssetManager* g_pAssetManager = nullptr;

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
extern void initskill();
extern void initquest();
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

bool RunMainScript(CPythonLauncher& pyLauncher, const char* lpCmdLine) {
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
	pyLauncher.RunLine("__COMMAND_LINE__ = \"\"");

	if (!pyLauncher.RunFile("system.py")) {
		LOGE("RunFile('system.py') Error: %s", pyLauncher.GetError());
		return false;
	}

	LOGI("system.py executed successfully!");
	return true;
}

std::string g_strAndroidExternalPath = "";

extern "C" {

int g_nAndroidScreenWidth = 1067;
int g_nAndroidScreenHeight = 600;
int g_nAndroidMouseX = 0;
int g_nAndroidMouseY = 0;
static JavaVM* s_pJavaVM = nullptr;

JNIEXPORT void JNICALL Java_com_metin2_client_NativeLib_init(JNIEnv* env, jobject obj, jobject assetManager, jstring internalPath, jstring externalPath, jint width, jint height) {
	if (env) {
		env->GetJavaVM(&s_pJavaVM);
	}
	LOGI("Initializing Metin2 Mobile Engine (%dx%d)", width, height);
	install_sigsegv_handler();

	// Dynamically disable Android fdsan (File Descriptor Sanitizer) to prevent abort on close()
	typedef void (*set_fdsan_fn)(int);
	set_fdsan_fn set_fdsan = (set_fdsan_fn)dlsym(RTLD_DEFAULT, "android_fdsan_set_error_level");
	if (set_fdsan) {
		set_fdsan(0); // ANDROID_FDSAN_ERROR_LEVEL_DISABLED
		LOGI("android_fdsan disabled dynamically");
	}

	g_nAndroidScreenWidth = width;
	g_nAndroidScreenHeight = height;
	g_nAndroidMouseX = width / 2;
	g_nAndroidMouseY = height / 2;

	// Change current working directory to the app's internal files folder
	if (internalPath) {
		const char* pathStr = env->GetStringUTFChars(internalPath, nullptr);
		if (pathStr) {
			LOGI("Setting working directory to: %s", pathStr);
			chdir(pathStr);
			env->ReleaseStringUTFChars(internalPath, pathStr);
		}
	}

	std::string extPackDir = "";
	if (externalPath) {
		const char* extStr = env->GetStringUTFChars(externalPath, nullptr);
		if (extStr) {
			g_strAndroidExternalPath = extStr;
			extPackDir = g_strAndroidExternalPath + "/pack/";
			LOGI("External pack directory set to: %s", extPackDir.c_str());
			env->ReleaseStringUTFChars(externalPath, extStr);
		}
	}

	// Store AAssetManager globally for asset/pack loading
	g_pAssetManager = AAssetManager_fromJava(env, assetManager);

	static CLZO lzo;
	static CEterPackManager EterPackManager;
	static EventHandler eventHandler;

	CEterPackManager::Instance().SetRelativePathMode();
	CEterPackManager::Instance().SetCacheMode();
	CEterPackManager::Instance().SetSearchMode(true); // SEARCH_PACK_FIRST

	auto regPack = [&](const char* name, const char* dirPrefix = "*") {
		std::string baseName = name;
		size_t slashPos = baseName.find_last_of('/');
		if (slashPos != std::string::npos) baseName = baseName.substr(slashPos + 1);

		// 1. Check /sdcard/aspar2/pack/ and /storage/emulated/0/aspar2/pack/ first
		std::string asparSdcard = "/sdcard/aspar2/pack/" + baseName;
		if (access((asparSdcard + ".index").c_str(), R_OK) == 0) {
			bool ok = CEterPackManager::Instance().RegisterPack(asparSdcard.c_str(), dirPrefix);
			LOGI("RegisterPack (SDCARD-ASPAR2) %s [%s]: %s", asparSdcard.c_str(), dirPrefix, ok ? "OK" : "FAILED");
			if (ok) return true;
		}

		std::string asparEmulated = "/storage/emulated/0/aspar2/pack/" + baseName;
		if (access((asparEmulated + ".index").c_str(), R_OK) == 0) {
			bool ok = CEterPackManager::Instance().RegisterPack(asparEmulated.c_str(), dirPrefix);
			LOGI("RegisterPack (EMULATED-ASPAR2) %s [%s]: %s", asparEmulated.c_str(), dirPrefix, ok ? "OK" : "FAILED");
			if (ok) return true;
		}

		// 1b. Fallback to /sdcard/metin2/pack/
		std::string sdcardPath = "/sdcard/metin2/pack/" + baseName;
		if (access((sdcardPath + ".index").c_str(), R_OK) == 0) {
			bool ok = CEterPackManager::Instance().RegisterPack(sdcardPath.c_str(), dirPrefix);
			LOGI("RegisterPack (SDCARD) %s [%s]: %s", sdcardPath.c_str(), dirPrefix, ok ? "OK" : "FAILED");
			if (ok) return true;
		}

		// 1c. Fallback to /storage/emulated/0/metin2/pack/
		std::string emulatedPath = "/storage/emulated/0/metin2/pack/" + baseName;
		if (access((emulatedPath + ".index").c_str(), R_OK) == 0) {
			bool ok = CEterPackManager::Instance().RegisterPack(emulatedPath.c_str(), dirPrefix);
			LOGI("RegisterPack (EMULATED) %s [%s]: %s", emulatedPath.c_str(), dirPrefix, ok ? "OK" : "FAILED");
			if (ok) return true;
		}

		// 2. Check external storage
		if (!extPackDir.empty()) {
			std::string fullExt = extPackDir + baseName;
			if (access((fullExt + ".index").c_str(), R_OK) == 0) {
				bool ok = CEterPackManager::Instance().RegisterPack(fullExt.c_str(), dirPrefix);
				LOGI("RegisterPack (EXTERNAL) %s [%s]: %s", fullExt.c_str(), dirPrefix, ok ? "OK" : "FAILED");
				if (ok) return true;
			}
		}

		// 3. Fallback to internal storage
		bool ok = CEterPackManager::Instance().RegisterPack(name, dirPrefix);
		LOGI("RegisterPack (INTERNAL) %s [%s]: %s", name, dirPrefix, ok ? "OK" : "FAILED");
		return ok;
	};

	// Register root pack (check aspar2 sdcard/emulated, then legacy metin2, external, internal)
	if (access("/sdcard/aspar2/pack/root.index", R_OK) == 0) {
		CEterPackManager::Instance().RegisterRootPack("/sdcard/aspar2/pack/root");
		LOGI("RegisterRootPack (SDCARD-ASPAR2) /sdcard/aspar2/pack/root: OK");
	} else if (access("/storage/emulated/0/aspar2/pack/root.index", R_OK) == 0) {
		CEterPackManager::Instance().RegisterRootPack("/storage/emulated/0/aspar2/pack/root");
		LOGI("RegisterRootPack (EMULATED-ASPAR2) /storage/emulated/0/aspar2/pack/root: OK");
	} else if (access("/sdcard/metin2/pack/root.index", R_OK) == 0) {
		CEterPackManager::Instance().RegisterRootPack("/sdcard/metin2/pack/root");
		LOGI("RegisterRootPack (SDCARD) /sdcard/metin2/pack/root: OK");
	} else if (access("/storage/emulated/0/metin2/pack/root.index", R_OK) == 0) {
		CEterPackManager::Instance().RegisterRootPack("/storage/emulated/0/metin2/pack/root");
		LOGI("RegisterRootPack (EMULATED) /storage/emulated/0/metin2/pack/root: OK");
	} else if (!extPackDir.empty() && access((extPackDir + "root.index").c_str(), R_OK) == 0) {
		std::string extRoot = extPackDir + "root";
		CEterPackManager::Instance().RegisterRootPack(extRoot.c_str());
		LOGI("RegisterRootPack (EXTERNAL) %s: OK", extRoot.c_str());
	} else {
		CEterPackManager::Instance().RegisterRootPack("pack/root");
		LOGI("RegisterRootPack (INTERNAL) pack/root: OK");
	}

	struct PackEntry { const char* dir; const char* name; };
	static const PackEntry s_packEntries[] = {
		{ "pack/", "metin2_patch_maps" },
		{ "pack/", "metin2_patch_map_smhdungeon" },
		{ "pack/", "metin2_patch_map_greedy" },
		{ "pack/", "metin2_patch_guild_pve" },
		{ "pack/", "metin2_patch_aggregate" },
		{ "pack/", "metin2_patch_arena" },
		{ "pack/", "metin2_patch_ts_snow_dungeon" },
		{ "pack/", "metin2_patch_private_search" },
		{ "pack/", "metin2_patch_catch_king" },
		{ "pack/", "metin2_patch_labyrinth" },
		{ "pack/", "metin2_patch_crystal_dungeon" },
		{ "pack/", "metin2_patch_mushroom" },
		{ "pack/", "metin2_patch_ork" },
		{ "pack/", "metin2_patch_dungeon_info" },
		{ "pack/", "metin2_patch_flag" },
		{ "pack/", "metin2_patch_12zi_mob" },
		{ "pack/", "metin2_patch_sould" },
		{ "pack/", "metin2_patch_myshop_deco" },
		{ "pack/", "metin2_patch_new_emotions" },
		{ "pack/", "metin2_patch_12zi_obj" },
		{ "pack/", "metin2_patch_12zi_map" },
		{ "pack/", "metin2_patch_luck" },
		{ "pack/", "metin2_patch_battlefied" },
		{ "pack/", "metin2_patch_arch_box" },
		{ "pack/", "metin2_patch_gem" },
		{ "pack/", "metin2_patch_new_pet" },
		{ "pack/", "metin2_patch_new_ui" },
		{ "pack/", "metin2_patch_pet_mount" },
		{ "pack/", "metin2_patch_heald" },
		{ "d:/ymir work/pc/", "pc" },
		{ "d:/ymir work/pc2/", "pc2" },
		{ "d:/ymir work/pc3/", "pc3" },
		{ "d:/ymir work/monster/", "monster" },
		{ "d:/ymir work/monster2/", "monster2" },
		{ "d:/ymir work/effect/", "effect" },
		{ "d:/ymir work/zone/", "zone" },
		{ "d:/ymir work/terrainmaps/", "terrain" },
		{ "d:/ymir work/npc/", "npc" },
		{ "d:/ymir work/npc2/", "npc2" },
		{ "d:/ymir work/tree/", "tree" },
		{ "d:/ymir work/guild/", "guild" },
		{ "d:/ymir work/item/", "item" },
		{ "textureset/", "textureset" },
		{ "property/", "property" },
		{ "icon/", "icon" },
		{ "sound/ambience/", "sound" },
		{ "sound/common/", "sound" },
		{ "sound/effect/", "sound" },
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

	// Also register common packs with wildcard
	regPack("pack/locale", "*");
	regPack("pack/etc", "*");
	regPack("pack/tree", "*");
	regPack("pack/property", "*");
	regPack("pack/terrain", "*");
	regPack("pack/zone", "*");

	// Auto-scan and register all additional packs and patches from all locations
	auto scanDir = [](const std::string& dirPath) {
		DIR* pPackDir = opendir(dirPath.c_str());
		if (pPackDir) {
			std::string prefix = dirPath;
			if (prefix.empty() || prefix.back() != '/') prefix += '/';
			struct dirent* entry;
			while ((entry = readdir(pPackDir)) != nullptr) {
				std::string fname = entry->d_name;
				if (fname.size() > 6 && fname.substr(fname.size() - 6) == ".index") {
					std::string packBase = fname.substr(0, fname.size() - 6);
					if (packBase != "root") {
						std::string fullPath = prefix + packBase;
						bool ok = CEterPackManager::Instance().RegisterPack(fullPath.c_str(), "*");
						LOGI("AutoRegisterPack %s: %s", fullPath.c_str(), ok ? "OK" : "FAILED");
					}
				}
			}
			closedir(pPackDir);
		}
	};

	if (access("/sdcard/aspar2/pack", R_OK) == 0) {
		scanDir("/sdcard/aspar2/pack");
	} else if (access("/storage/emulated/0/aspar2/pack", R_OK) == 0) {
		scanDir("/storage/emulated/0/aspar2/pack");
	} else if (access("/sdcard/metin2/pack", R_OK) == 0) {
		scanDir("/sdcard/metin2/pack");
	} else if (access("/storage/emulated/0/metin2/pack", R_OK) == 0) {
		scanDir("/storage/emulated/0/metin2/pack");
	}
	if (!extPackDir.empty()) {
		scanDir(extPackDir);
	}

	LocaleService_LoadConfig("locale.cfg");
	if (strcmp(LocaleService_GetLocaleName(), "ymir") == 0 || strlen(LocaleService_GetLocaleName()) == 0) {
		LocaleService_ForceSetLocale("tr", "locale/tr");
	}
	LOGI("Metin2 Locale initialized: %s (%s)", LocaleService_GetLocaleName(), LocaleService_GetLocalePath());

	static CPythonApplication app;

	if (!app.Create(NULL, "Metin2Mobile", width, height, 1)) {
		LOGE("CPythonApplication::Create failed on Android!");
		return;
	}

	UI::CWindowManager::Instance().SetScreenSize(width, height);
	if (CPythonSystem::InstancePtr()) {
		CPythonSystem::Instance().GetConfig()->width = width;
		CPythonSystem::Instance().GetConfig()->height = height;
	}

	static CPythonLauncher pyLauncher;
	static CPythonExceptionSender pyExceptionSender;
	SetExceptionSender(&pyExceptionSender);

	if (pyLauncher.Create()) {
		LOGI("Running Metin2 Python Main Script (system.py)...");
		if (!RunMainScript(pyLauncher, "")) {
			LOGE("RunMainScript failed!");
		} else {
			LOGI("RunMainScript completed successfully!");
		}
	} else {
		LOGE("pyLauncher.Create() failed!");
	}

	LOGI("Metin2 Mobile Engine Initialized Successfully");
}

void Android_ExitApp();

JNIEXPORT void JNICALL Java_com_metin2_client_NativeLib_render(JNIEnv* env, jobject obj) {
	if (CPythonApplication::Instance().IsExit()) {
		return;
	}
	CPythonApplication::Instance().Process();
}

JNIEXPORT void JNICALL Java_com_metin2_client_NativeLib_touchEvent(JNIEnv* env, jobject obj, jint action, jfloat x, jfloat y) {
	int ix = static_cast<int>(x);
	int iy = static_cast<int>(y);
	g_nAndroidMouseX = ix;
	g_nAndroidMouseY = iy;

	if (action == 0 || action == 5) { // ACTION_DOWN or ACTION_POINTER_DOWN
		CPythonApplication::Instance().OnMouseMove(ix, iy);
		CPythonApplication::Instance().OnMouseLeftButtonDown(ix, iy);
	} else if (action == 1 || action == 3 || action == 6) { // ACTION_UP, ACTION_CANCEL, ACTION_POINTER_UP
		CPythonApplication::Instance().OnMouseMove(ix, iy);
		CPythonApplication::Instance().OnMouseLeftButtonUp(ix, iy);
	} else if (action == 2) { // ACTION_MOVE
		CPythonApplication::Instance().OnMouseMove(ix, iy);
	}
}


JNIEXPORT void JNICALL Java_com_metin2_client_NativeLib_joystickTouch(JNIEnv* env, jobject obj, jint action, jfloat x, jfloat y) {
	CPythonPlayer::Instance().OnMobileJoystick(action, x, y);
}

static bool s_bActionIsUI = false;      // primary finger is routed through the python mouse
static bool s_bActionIsWorld = false;   // ...and it started on the 3D world ("game" window, camera drag / tap)
static bool s_bActionConverted = false; // primary world finger was converted to a native camera drag
static int  s_nActionLastX = 0;
static int  s_nActionLastY = 0;

static float s_fJoystickX = 0.0f;
static float s_fJoystickY = -1.0f;
static float s_fJoystickW = 194.0f;
static float s_fJoystickH = 194.0f;

bool Android_IsJoystickZone(float x, float y) {
	UI::CWindowManager & rkWndMgr = UI::CWindowManager::Instance();
	long sh = rkWndMgr.GetScreenHeight();
	float jx = s_fJoystickX;
	float jy = (s_fJoystickY >= 0.0f) ? s_fJoystickY : (float)(sh - s_fJoystickH);
	float jw = s_fJoystickW;
	float jh = s_fJoystickH;
	return (x >= jx && x <= jx + jw && y >= jy && y <= jy + jh);
}

// Same HUD zones that game.py uses to decide between "camera touch" and "HUD touch".
static bool Android_IsHudZone(long x, long y) {
	UI::CWindowManager & rkWndMgr = UI::CWindowManager::Instance();
	long sw = rkWndMgr.GetScreenWidth();
	long sh = rkWndMgr.GetScreenHeight();
	return Android_IsJoystickZone((float)x, (float)y) || (x > sw - 370 && y > sh - 320) || (y < 65);
}

static void Android_RotateCamera(float deltaX, float deltaY) {
	CCamera* pkCmrCur = CCameraManager::Instance().GetCurrentCamera();
	if (pkCmrCur && !pkCmrCur->IsLock()) {
		float fRollDelta = deltaX * 0.30f;
		float fPitchDelta = deltaY * 0.20f;
		pkCmrCur->RotateEyeAroundTarget(fPitchDelta, fRollDelta);
	}
}

JNIEXPORT void JNICALL Java_com_metin2_client_NativeLib_actionTouch(JNIEnv* env, jobject obj, jint action, jfloat x, jfloat y, jfloat deltaX, jfloat deltaY, jboolean isDrag) {
	int ix = static_cast<int>(x);
	int iy = static_cast<int>(y);

	if (action == 0 || action == 5) { // ACTION_DOWN / ACTION_POINTER_DOWN
		bool bIsUI = UI::CWindowManager::Instance().IsUIWindowAt(ix, iy);
		s_bActionConverted = false;
		s_nActionLastX = ix;
		s_nActionLastY = iy;
		if (bIsUI) {
			s_bActionIsUI = true;
			s_bActionIsWorld = UI::CWindowManager::Instance().IsGameWorldAt(ix, iy) && !Android_IsHudZone(ix, iy);
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
			// World drag that gave the python mouse away to a HUD button: rotate natively.
			Android_RotateCamera(deltaX, deltaY);
		} else if (s_bActionIsUI) {
			CPythonApplication::Instance().OnMouseMove(ix, iy);
		} else {
			Android_RotateCamera(deltaX, deltaY);
		}
	} else if (action == 1 || action == 3 || action == 6) { // ACTION_UP / ACTION_CANCEL / ACTION_POINTER_UP
		if (s_bActionConverted) {
			// Mouse already belongs to a HUD button (e.g. attack), do not touch it.
			s_bActionConverted = false;
			s_bActionIsUI = false;
			s_bActionIsWorld = false;
		} else if (s_bActionIsUI) {
			CPythonApplication::Instance().OnMouseMove(ix, iy);
			CPythonApplication::Instance().OnMouseLeftButtonUp(ix, iy);
			s_bActionIsUI = false;
			s_bActionIsWorld = false;
		} else {
			if (!isDrag) {
				CPythonApplication::Instance().OnMouseMove(ix, iy);
				CPythonApplication::Instance().OnMouseLeftButtonDown(ix, iy);
				CPythonApplication::Instance().OnMouseLeftButtonUp(ix, iy);
			}
		}
	}
}

// Extra finger while the primary "action" finger is busy.
//  - over the 3D world         -> camera drag (native)
//  - over a HUD button         -> normal button press. If the primary finger is a camera drag
//                                 (python mouse owner), it is converted to a native camera drag first,
//                                 so the HUD button (e.g. attack) can take the mouse.
static int s_nSecondaryMode = 0; // 0 = none, 1 = UI press (python mouse), 2 = camera drag, 3 = UI press (independent 2nd capture)

JNIEXPORT void JNICALL Java_com_metin2_client_NativeLib_secondaryTouch(JNIEnv* env, jobject obj, jint action, jfloat x, jfloat y, jfloat deltaX, jfloat deltaY) {
	int ix = static_cast<int>(x);
	int iy = static_cast<int>(y);

	if (action == 0 || action == 5) { // DOWN / POINTER_DOWN
		UI::CWindowManager & rkWndMgr = UI::CWindowManager::Instance();
		if (rkWndMgr.IsGameWorldAt(ix, iy) && !Android_IsHudZone(ix, iy)) {
			s_nSecondaryMode = 2; // camera drag
		} else if (rkWndMgr.IsUIWindowAt(ix, iy)) {
			if (s_bActionIsUI && s_bActionIsWorld && !s_bActionConverted) {
				// Release the primary world touch from the python mouse (this ends game.py's camera drag
				// cleanly) and keep rotating the camera natively.
				CPythonApplication::Instance().OnMouseMove(s_nActionLastX, s_nActionLastY);
				CPythonApplication::Instance().OnMouseLeftButtonUp(s_nActionLastX, s_nActionLastY);
				s_bActionConverted = true;
			}
			if (!s_bActionIsUI || s_bActionConverted) {
				s_nSecondaryMode = 1;
				CPythonApplication::Instance().OnMouseMove(ix, iy);
				CPythonApplication::Instance().OnMouseLeftButtonDown(ix, iy);
			} else {
				// Primary finger holds a real UI element (e.g. attack button): give this finger its own capture.
				s_nSecondaryMode = 3;
				rkWndMgr.RunSecondaryTouchDown(ix, iy);
			}
		} else {
			s_nSecondaryMode = 0;
		}
	} else if (action == 2) { // MOVE
		if (s_nSecondaryMode == 1) {
			CPythonApplication::Instance().OnMouseMove(ix, iy);
		} else if (s_nSecondaryMode == 2) {
			Android_RotateCamera(deltaX, deltaY);
		} else if (s_nSecondaryMode == 3) {
			UI::CWindowManager::Instance().RunSecondaryTouchMove(ix, iy);
		}
	} else if (action == 1 || action == 3 || action == 6) { // UP / CANCEL / POINTER_UP
		if (s_nSecondaryMode == 1) {
			CPythonApplication::Instance().OnMouseMove(ix, iy);
			CPythonApplication::Instance().OnMouseLeftButtonUp(ix, iy);
		} else if (s_nSecondaryMode == 3) {
			UI::CWindowManager::Instance().RunSecondaryTouchUp(ix, iy);
		}
		s_nSecondaryMode = 0;
	}
}

JNIEXPORT void JNICALL Java_com_metin2_client_NativeLib_cameraRotate(JNIEnv* env, jobject obj, jfloat deltaX, jfloat deltaY) {
	CCamera* pkCmrCur = CCameraManager::Instance().GetCurrentCamera();
	if (pkCmrCur && !pkCmrCur->IsLock()) {
		float fRollDelta = deltaX * 0.30f;
		float fPitchDelta = -deltaY * 0.20f;
		pkCmrCur->RotateEyeAroundTarget(fPitchDelta, fRollDelta);
	}
}

static JNIEnv* Android_GetJNIEnv() {
	if (!s_pJavaVM) return nullptr;
	JNIEnv* env = nullptr;
	jint res = s_pJavaVM->GetEnv((void**)&env, JNI_VERSION_1_6);
	if (res == JNI_EDETACHED) {
		if (s_pJavaVM->AttachCurrentThread(&env, nullptr) != JNI_OK) {
			return nullptr;
		}
	}
	return env;
}

void Android_SetJoystickZone(float x, float y, float width, float height) {
	s_fJoystickX = x;
	s_fJoystickY = y;
	s_fJoystickW = width;
	s_fJoystickH = height;

	JNIEnv* env = Android_GetJNIEnv();
	if (!env) return;
	jclass clazz = env->FindClass("com/metin2/client/NativeLib");
	if (!clazz) return;
	jmethodID mid = env->GetStaticMethodID(clazz, "updateJoystickZone", "(FFFF)V");
	if (mid) {
		env->CallStaticVoidMethod(clazz, mid, (jfloat)x, (jfloat)y, (jfloat)width, (jfloat)height);
	}
	env->DeleteLocalRef(clazz);
}

void Android_ShowKeyboard(const char* initialText) {
	JNIEnv* env = Android_GetJNIEnv();
	if (!env) return;
	jclass clazz = env->FindClass("com/metin2/client/NativeLib");
	if (!clazz) return;
	jmethodID mid = env->GetStaticMethodID(clazz, "showKeyboard", "(Ljava/lang/String;)V");
	if (mid) {
		jstring jstr = env->NewStringUTF(initialText ? initialText : "");
		env->CallStaticVoidMethod(clazz, mid, jstr);
		env->DeleteLocalRef(jstr);
	}
	env->DeleteLocalRef(clazz);
}

void Android_HideKeyboard() {
	JNIEnv* env = Android_GetJNIEnv();
	if (!env) return;
	jclass clazz = env->FindClass("com/metin2/client/NativeLib");
	if (!clazz) return;
	jmethodID mid = env->GetStaticMethodID(clazz, "hideKeyboard", "()V");
	if (mid) {
		env->CallStaticVoidMethod(clazz, mid);
	}
	env->DeleteLocalRef(clazz);
}

void Android_ShowWebPage(const char* url) {
	JNIEnv* env = Android_GetJNIEnv();
	if (!env) return;
	jclass clazz = env->FindClass("com/metin2/client/NativeLib");
	if (!clazz) return;
	jmethodID mid = env->GetStaticMethodID(clazz, "showWebPage", "(Ljava/lang/String;)V");
	if (mid) {
		jstring jstr = env->NewStringUTF(url ? url : "");
		env->CallStaticVoidMethod(clazz, mid, jstr);
		env->DeleteLocalRef(jstr);
	}
	env->DeleteLocalRef(clazz);
}

void Android_HideWebPage() {
	JNIEnv* env = Android_GetJNIEnv();
	if (!env) return;
	jclass clazz = env->FindClass("com/metin2/client/NativeLib");
	if (!clazz) return;
	jmethodID mid = env->GetStaticMethodID(clazz, "hideWebPage", "()V");
	if (mid) {
		env->CallStaticVoidMethod(clazz, mid);
	}
	env->DeleteLocalRef(clazz);
}

bool Android_IsWebShowing() {
	JNIEnv* env = Android_GetJNIEnv();
	if (!env) return false;
	jclass clazz = env->FindClass("com/metin2/client/NativeLib");
	if (!clazz) return false;
	jmethodID mid = env->GetStaticMethodID(clazz, "isWebShowing", "()Z");
	jboolean res = JNI_FALSE;
	if (mid) {
		res = env->CallStaticBooleanMethod(clazz, mid);
	}
	env->DeleteLocalRef(clazz);
	return (res == JNI_TRUE);
}

void Android_ExitApp() {
	LOGI("Android_ExitApp() called");
	if (CPythonSystem::InstancePtr()) {
		CPythonSystem::Instance().SaveConfig();
		CPythonSystem::Instance().SaveInterfaceStatus();
	}
	JNIEnv* env = Android_GetJNIEnv();
	if (!env) return;
	jclass clazz = env->FindClass("com/metin2/client/NativeLib");
	if (!clazz) return;
	jmethodID mid = env->GetStaticMethodID(clazz, "exitApp", "()V");
	if (mid) {
		env->CallStaticVoidMethod(clazz, mid);
	}
	env->DeleteLocalRef(clazz);
}

JNIEXPORT void JNICALL Java_com_metin2_client_NativeLib_onKeyboardText(JNIEnv* env, jclass clazz, jstring text) {
	if (!text) return;
	const char* str = env->GetStringUTFChars(text, nullptr);
	if (str) {
		CPythonIME::Instance().SetText(str, strlen(str));
		CPythonApplication::Instance().RunIMEUpdate();
		env->ReleaseStringUTFChars(text, str);
	}
}

JNIEXPORT void JNICALL Java_com_metin2_client_NativeLib_onKeyboardEnter(JNIEnv* env, jclass clazz) {
	CPythonApplication::Instance().RunIMEReturnEvent();
}

JNIEXPORT void JNICALL Java_com_metin2_client_NativeLib_pauseAudio(JNIEnv* env, jclass clazz) {
	LOGI("NativeLib_pauseAudio() called");
	if (CSoundManager::InstancePtr()) {
		CSoundManager::Instance().SaveVolume();
	}
	CSoundBase::PauseAudio();
}

JNIEXPORT void JNICALL Java_com_metin2_client_NativeLib_resumeAudio(JNIEnv* env, jclass clazz) {
	LOGI("NativeLib_resumeAudio() called");
	CSoundBase::ResumeAudio();
	if (CSoundManager::InstancePtr()) {
		CSoundManager::Instance().RestoreVolume();
	}
}

JNIEXPORT void JNICALL Java_com_metin2_client_NativeLib_saveConfig(JNIEnv* env, jclass clazz) {
	LOGI("NativeLib_saveConfig() called");
	if (CPythonSystem::InstancePtr()) {
		CPythonSystem::Instance().SaveConfig();
		CPythonSystem::Instance().SaveInterfaceStatus();
	}
}

JNIEXPORT jboolean JNICALL Java_com_metin2_client_NativeLib_isGamePhase(JNIEnv* env, jclass clazz) {
	if (CPythonNetworkStream::InstancePtr()) {
		return CPythonNetworkStream::Instance().IsGamePhase() ? JNI_TRUE : JNI_FALSE;
	}
	return JNI_FALSE;
}

// Otopatch: [DWORD gercek boyut][ham LZO1X akisi] formatindaki .lz dosyasini acar (PC patcher ile ayni format).
// mmap kullanir, buyuk pack dosyalari icin Java heap'ine yuklenmez. 0 = basarili, negatif = hata.
JNIEXPORT jint JNICALL Java_com_metin2_client_NativeLib_lzDecompressFile(JNIEnv* env, jclass clazz, jstring jsrc, jstring jdst) {
	static bool s_lzoReady = (lzo_init() == LZO_E_OK);
	if (!s_lzoReady) return -1;

	const char* src = env->GetStringUTFChars(jsrc, nullptr);
	const char* dst = env->GetStringUTFChars(jdst, nullptr);

	jint result = 0;
	int fdIn = -1, fdOut = -1;
	void* pIn = MAP_FAILED;
	void* pOut = MAP_FAILED;
	size_t inSize = 0, outSize = 0;

	do {
		fdIn = open(src, O_RDONLY);
		if (fdIn < 0) { result = -2; break; }
		off_t len = lseek(fdIn, 0, SEEK_END);
		if (len <= 5) { result = -3; break; }
		inSize = (size_t)len;
		pIn = mmap(nullptr, inSize, PROT_READ, MAP_PRIVATE, fdIn, 0);
		if (pIn == MAP_FAILED) { result = -4; break; }

		uint32_t realSize = 0;
		memcpy(&realSize, pIn, sizeof(realSize));
		outSize = (size_t)realSize;

		fdOut = open(dst, O_RDWR | O_CREAT | O_TRUNC, 0644);
		if (fdOut < 0) { result = -5; break; }
		if (outSize > 0) {
			if (ftruncate(fdOut, (off_t)outSize) != 0) { result = -6; break; }
			pOut = mmap(nullptr, outSize, PROT_READ | PROT_WRITE, MAP_SHARED, fdOut, 0);
			if (pOut == MAP_FAILED) { result = -7; break; }

			lzo_uint outLen = (lzo_uint)outSize;
			int r = lzo1x_decompress_safe((const lzo_bytep)pIn + sizeof(uint32_t), (lzo_uint)(inSize - sizeof(uint32_t)),
			                              (lzo_bytep)pOut, &outLen, nullptr);
			if (r != LZO_E_OK || outLen != (lzo_uint)outSize) { result = -8; break; }
			msync(pOut, outSize, MS_SYNC);
		}
	} while (0);

	if (pOut != MAP_FAILED) munmap(pOut, outSize);
	if (pIn != MAP_FAILED) munmap(pIn, inSize);
	if (fdOut >= 0) close(fdOut);
	if (fdIn >= 0) close(fdIn);
	env->ReleaseStringUTFChars(jsrc, src);
	env->ReleaseStringUTFChars(jdst, dst);
	return result;
}

} // extern "C"
#endif
