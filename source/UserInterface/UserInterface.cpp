#include "StdAfx.h"
#include "PythonApplication.h"
#include "ProcessScanner.h"
#include "PythonExceptionSender.h"
#include "resource.h"
#include "Version.h"

#ifdef _DEBUG
#include <crtdbg.h>
#endif

#include "../eterPack/EterPackManager.h"
#include "../eterLib/Util.h"
#ifdef CEF_BROWSER
#include "CefWebBrowser.h"
#else
#include "../CWebBrowser/CWebBrowser.h"
#endif
#include "../eterBase/CPostIt.h"

#include "CheckLatestFiles.h"

#include "Hackshield.h"
#include "NProtectGameGuard.h"
#include "WiseLogicXTrap.h"
#ifdef EVENT_HANDLER_MASTER
#include "EventHandler.h"
#endif
#ifdef ENABLE_VOTE4BUFF
#include "M2WebSocket.h"
#endif

using namespace std;

extern "C" {  
extern int _fltused;  
volatile int _AVOID_FLOATING_POINT_LIBRARY_BUG = _fltused;  
};
#if _MSC_VER >= 1900
extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }
#endif

#pragma comment(linker, "/NODEFAULTLIB:libci.lib")
#pragma comment( lib, "version.lib" )
#pragma comment( lib, "python27.lib" )
#pragma comment( lib, "imagehlp.lib" )
//#pragma comment( lib, "devil.lib" ) // replaced by stb_image
#if GrannyProductMinorVersion==4
#pragma comment( lib, "granny2.4.0.10.lib" )
#elif GrannyProductMinorVersion==7
#pragma comment( lib, "granny2.7.0.30.lib" )
#elif GrannyProductMinorVersion==8
#pragma comment( lib, "granny2.8.49.0.lib" )
#elif GrannyProductMinorVersion==9
#pragma comment( lib, "granny2.9.12.0.lib" )
#elif GrannyProductMinorVersion==11
#pragma comment( lib, "granny2.11.8.0.lib" )
#else
#error "unknown granny version"
#endif
//#pragma comment( lib, "mss32.lib" ) // replaced by miniaudio
#pragma comment( lib, "winmm.lib" )
#pragma comment( lib, "imm32.lib" )
#pragma comment( lib, "oldnames.lib" )
#pragma comment( lib, "SpeedTreeRT.lib" )
//#pragma comment( lib, "dinput8.lib" ) // replaced by SDL2
#pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "ws2_32.lib" )
//#pragma comment( lib, "wsock32.lib" )
#include <stdlib.h>
#include <cryptopp/cryptoppLibLink.h>
bool __IS_TEST_SERVER_MODE__=false;

extern bool SetDefaultCodePage(DWORD codePage);

#ifdef USE_OPENID
extern int openid_test;
#endif

static const char * sc_apszPythonLibraryFilenames[] =
{
	"UserDict.pyc",
	"__future__.pyc",
	"copy_reg.pyc",
	"linecache.pyc",
	"ntpath.pyc",
	"os.pyc",
	"site.pyc",
	"stat.pyc",
	"string.pyc",
	"traceback.pyc",
	"types.pyc",
	"\n",
};

#ifdef ENABLE_FIX_D_YMIR_WORK
#include <windows.h>
#include <string>
bool WrongPathYmir(const std::string& dirName_in)
{
    DWORD type = GetFileAttributesA(dirName_in.c_str());

    if (type == INVALID_FILE_ATTRIBUTES)
        return false;

    if (type & FILE_ATTRIBUTE_DIRECTORY)
        return true;

    return false;
}
#endif

char gs_szErrorString[512] = "";

void ApplicationSetErrorString(const char* szErrorString)
{
	strcpy(gs_szErrorString, szErrorString);
}

bool CheckPythonLibraryFilenames()
{
	for (int i = 0; *sc_apszPythonLibraryFilenames[i] != '\n'; ++i)
	{
		std::string stFilename = "lib\\";
		stFilename += sc_apszPythonLibraryFilenames[i];

		if (_access(stFilename.c_str(), 0) != 0)
		{
			return false;
		}

		MoveFile(stFilename.c_str(), stFilename.c_str());
	}

	return true;
}

struct ApplicationStringTable 
{
	HINSTANCE m_hInstance;
	std::map<DWORD, std::string> m_kMap_dwID_stLocale;
} gs_kAppStrTable;

void ApplicationStringTable_Initialize(HINSTANCE hInstance)
{
	gs_kAppStrTable.m_hInstance=hInstance;
}

const std::string& ApplicationStringTable_GetString(DWORD dwID, LPCSTR szKey)
{
	char szBuffer[512];
	char szIniFileName[256];
	char szLocale[256];

	::GetCurrentDirectory(sizeof(szIniFileName), szIniFileName);
	if(szIniFileName[lstrlen(szIniFileName)-1] != '\\')
		strcat(szIniFileName, "\\");
	strcat(szIniFileName, "metin2client.dat");

	strcpy(szLocale, LocaleService_GetLocalePath());
	if(strnicmp(szLocale, "locale/", strlen("locale/")) == 0)
		strcpy(szLocale, LocaleService_GetLocalePath() + strlen("locale/"));
	::GetPrivateProfileString(szLocale, szKey, NULL, szBuffer, sizeof(szBuffer)-1, szIniFileName);
	if(szBuffer[0] == '\0')
		LoadString(gs_kAppStrTable.m_hInstance, dwID, szBuffer, sizeof(szBuffer)-1);
	if(szBuffer[0] == '\0')
		::GetPrivateProfileString("en", szKey, NULL, szBuffer, sizeof(szBuffer)-1, szIniFileName);
	if(szBuffer[0] == '\0')
		strcpy(szBuffer, szKey);

	std::string& rstLocale=gs_kAppStrTable.m_kMap_dwID_stLocale[dwID];
	rstLocale=szBuffer;

	return rstLocale;
}

const std::string& ApplicationStringTable_GetString(DWORD dwID)
{
	char szBuffer[512];

	LoadString(gs_kAppStrTable.m_hInstance, dwID, szBuffer, sizeof(szBuffer)-1);
	std::string& rstLocale=gs_kAppStrTable.m_kMap_dwID_stLocale[dwID];
	rstLocale=szBuffer;

	return rstLocale;
}

const char* ApplicationStringTable_GetStringz(DWORD dwID, LPCSTR szKey)
{
	return ApplicationStringTable_GetString(dwID, szKey).c_str();
}

const char* ApplicationStringTable_GetStringz(DWORD dwID)
{
	return ApplicationStringTable_GetString(dwID).c_str();
}

////////////////////////////////////////////

int Setup(LPSTR lpCmdLine); // Internal function forward

#ifdef ENABLE_INDEX_FILE
bool PackInitialize(const char* c_pszFolder)
{
	std::vector<std::vector<std::string>> indexVec
	{
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

	if ((UINT)GetDriveType((LPCSTR)L"D:\\") == 3)
	{
		std::string szYmirWorkPath;
		szYmirWorkPath == "D:\\ymir work";

		struct stat st;
		if (stat("D:\\ymir work", &st) == 0)
		{
			LogBoxf("Something is wrong delete the file from D (ymir work)");
			return true;
		}
	}

	if (_access(c_pszFolder, 0) != 0)
		return true;

	std::string stFolder(c_pszFolder);
	stFolder += "/";

#ifdef YMIR_WORK_FPS_DROP_FIX
	bool bPackFirst = true;
#else
	const bool bPackFirst = true;
#endif
// Bvural41 27-11-23 sonradan düzeltildi

#if defined(USE_RELATIVE_PATH)
	CEterPackManager::Instance().SetRelativePathMode();
#endif
	CEterPackManager::Instance().SetCacheMode();
	CEterPackManager::Instance().SetSearchMode(bPackFirst);

	CSoundData::SetPackMode();

	std::string strPackName, strTexCachePackName;

	for (auto& elem : indexVec)
	{
		const std::string& c_rstFolder = elem[0];
		const std::string& c_rstName = elem[1];

		strPackName = stFolder + c_rstName;
		strTexCachePackName = strPackName + "_texcache";

		CEterPackManager::Instance().RegisterPack(strPackName.c_str(), c_rstFolder.c_str());
		CEterPackManager::Instance().RegisterPack(strTexCachePackName.c_str(), c_rstFolder.c_str());
	}

	CEterPackManager::Instance().RegisterRootPack((stFolder + std::string("root")).c_str());
	NANOEND
		return true;
}
#else
bool PackInitialize(const char * c_pszFolder)
{
	NANOBEGIN
	if (_access(c_pszFolder, 0) != 0)
		return true;

	std::string stFolder(c_pszFolder);
	stFolder += "/";

	std::string stFileName(stFolder);
	stFileName += "Index";

	CMappedFile file;
	LPCVOID pvData;

	if (!file.Create(stFileName.c_str(), &pvData, 0, 0))
	{
		LogBoxf("FATAL ERROR! File not exist: %s", stFileName.c_str());
		TraceError("FATAL ERROR! File not exist: %s", stFileName.c_str());
		return true;
	}

	CMemoryTextFileLoader TextLoader;
	TextLoader.Bind(file.Size(), pvData);

	bool bPackFirst = TRUE;

	const std::string& strPackType = TextLoader.GetLineString(0);

	if (strPackType.compare("FILE") && strPackType.compare("PACK"))
	{
		TraceError("Pack/Index has invalid syntax. First line must be 'PACK' or 'FILE'");
		return false;
	}

#ifdef _DISTRIBUTE
	Tracef("¾Ë¸²: ÆÑ ¸ğµåÀÔ´Ï´Ù.\n");
	
	//if (0 == strPackType.compare("FILE"))
	//{
	//	bPackFirst = FALSE;
	//	Tracef("¾Ë¸²: ÆÄÀÏ ¸ğµåÀÔ´Ï´Ù.\n");
	//}
	//else
	//{
	//	Tracef("¾Ë¸²: ÆÑ ¸ğµåÀÔ´Ï´Ù.\n");
	//}
#else
	bPackFirst = FALSE;
	Tracef("¾Ë¸²: ÆÄÀÏ ¸ğµåÀÔ´Ï´Ù.\n");
#endif

	CTextFileLoader::SetCacheMode();
#if defined(USE_RELATIVE_PATH)
	CEterPackManager::Instance().SetRelativePathMode();
#endif
	CEterPackManager::Instance().SetCacheMode();
	CEterPackManager::Instance().SetSearchMode(bPackFirst);

	CSoundData::SetPackMode(); // Miles ÆÄÀÏ Äİ¹éÀ» ¼ÂÆÃ

	std::string strPackName, strTexCachePackName;
	for (DWORD i = 1; i < TextLoader.GetLineCount() - 1; i += 2)
	{
		const std::string & c_rstFolder = TextLoader.GetLineString(i);
		const std::string & c_rstName = TextLoader.GetLineString(i + 1);

		strPackName = stFolder + c_rstName;
		strTexCachePackName = strPackName + "_texcache";

		CEterPackManager::Instance().RegisterPack(strPackName.c_str(), c_rstFolder.c_str());
		CEterPackManager::Instance().RegisterPack(strTexCachePackName.c_str(), c_rstFolder.c_str());
	}

	CEterPackManager::Instance().RegisterRootPack((stFolder + std::string("root")).c_str());
	NANOEND
	return true;
}
#endif

bool RunMainScript(CPythonLauncher& pyLauncher, const char* lpCmdLine)
{
	initpack();
	initdbg();
	initime();
	initgrp();
	initgrpImage();
	initgrpText();
	initwndMgr();
	/////////////////////////////////////////////
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
#ifdef ENABLE_TITLE_SYSTEM
	initTitleSystem();
#endif
#ifdef ENABLE_TREASURE_EVENT
	inittreasure_event();
#endif
#ifdef ENABLE_BATTLE_ROYALE
	initbattleRoyaleMgr();
#endif
#ifdef ENABLE_BATTLEPASS
	initBattlePass();
#endif
#ifdef ENABLE_SWITCHBOT
	initSwitchbot();
#endif
#ifdef ENABLE_BATTLE_FIELD
	initCombatZoneSystem();
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
	NANOBEGIN

	// RegisterDebugFlag
	{
		std::string stRegisterDebugFlag;

#ifdef _DISTRIBUTE 
		stRegisterDebugFlag ="__DEBUG__ = 0";
#else
		stRegisterDebugFlag ="__DEBUG__ = 1"; 
#endif

		if (!pyLauncher.RunLine(stRegisterDebugFlag.c_str()))
		{
			TraceError("RegisterDebugFlag Error");
			return false;
		}
	}

	// RegisterCommandLine
	{
		std::string stRegisterCmdLine;

		const char * loginMark = "-cs";
		const char * loginMark_NonEncode = "-ncs";
		const char * seperator = " ";

		std::string stCmdLine;
		const int CmdSize = 3;
		vector<std::string> stVec;
		SplitLine(lpCmdLine,seperator,&stVec);
		if (CmdSize == stVec.size() && stVec[0]==loginMark)
		{
			char buf[MAX_PATH];	//TODO ¾Æ·¡ ÇÔ¼ö string ÇüÅÂ·Î ¼öÁ¤
			base64_decode(stVec[2].c_str(),buf);
			stVec[2] = buf;
			string_join(seperator,stVec,&stCmdLine);
		}
		else if (CmdSize <= stVec.size() && stVec[0]==loginMark_NonEncode)
		{
			stVec[0] = loginMark;
			string_join(" ",stVec,&stCmdLine);
		}
		else
			stCmdLine = lpCmdLine;

		stRegisterCmdLine ="__COMMAND_LINE__ = ";
		stRegisterCmdLine+='"';
		stRegisterCmdLine+=stCmdLine;
		stRegisterCmdLine+='"';

		const CHAR* c_szRegisterCmdLine=stRegisterCmdLine.c_str();
		if (!pyLauncher.RunLine(c_szRegisterCmdLine))
		{
			TraceError("RegisterCommandLine Error");
			return false;
		}
	}
	{
		vector<std::string> stVec;
		SplitLine(lpCmdLine," " ,&stVec);

		if (stVec.size() != 0 && "--pause-before-create-window" == stVec[0])
		{
#ifdef XTRAP_CLIENT_ENABLE
			if (!XTrap_CheckInit())
				return false;
#endif
			system("pause");
		}
		if (!pyLauncher.RunFile("system.py"))
		{
			TraceError("RunMain Error");
			return false;
		}
	}

	NANOEND
	return true;
}

bool Main(HINSTANCE hInstance, LPSTR lpCmdLine)
{
#ifdef LOCALE_SERVICE_YMIR
	extern bool g_isScreenShotKey;
	g_isScreenShotKey = true;
#endif

	DWORD dwRandSeed=time(NULL)+DWORD(GetCurrentProcess());
	srandom(dwRandSeed);
	srand(random());

#ifdef EVENT_HANDLER_MASTER
	EventHandler EventHandle;
#endif

	SetLogLevel(1);

#ifdef LOCALE_SERVICE_VIETNAM_MILD
	extern BOOL USE_VIETNAM_CONVERT_WEAPON_VNUM;
	USE_VIETNAM_CONVERT_WEAPON_VNUM = true;
#endif

	if (_access("perf_game_update.txt", 0)==0)
	{
		DeleteFile("perf_game_update.txt");
	}

	if (_access("newpatch.exe", 0)==0)
	{		
		system("patchupdater.exe");
		return false;
	}
#ifndef __VTUNE__
	//ilInit(); // replaced by stb_image
#endif
	if (!Setup(lpCmdLine))
		return false;

#ifdef _DEBUG
	OpenConsoleWindow();
	OpenLogFile(true); // true == uses syserr.txt and log.txt
#else
	OpenLogFile(false); // false == uses syserr.txt only
#endif

	static CLZO				lzo;
	static CEterPackManager	EterPackManager;

	if (!PackInitialize("pack"))
	{
		LogBox("Pack Initialization failed. Check log.txt file..");
		return false;
	}

	if(LocaleService_LoadGlobal(hInstance))
		SetDefaultCodePage(LocaleService_GetCodePage());

	SDL_SetMainReady();
	CPythonApplication * app = new CPythonApplication;

	app->Initialize(hInstance);

	bool ret=false;
	{
		CPythonLauncher pyLauncher;
		CPythonExceptionSender pyExceptionSender;
		SetExceptionSender(&pyExceptionSender);

		if (pyLauncher.Create())
		{
			ret=RunMainScript(pyLauncher, lpCmdLine);	//°ÔÀÓ ½ÇÇàÁß¿£ ÇÔ¼ö°¡ ³¡³ªÁö ¾Ê´Â´Ù.
		}

		//ProcessScanner_ReleaseQuitEvent();
		
		//°ÔÀÓ Á¾·á½Ã.
		app->Clear();

		timeEndPeriod(1);
		pyLauncher.Clear();
	}

	app->Destroy();
	delete app;
	SDL_Quit();
	
	return ret;
}

HANDLE CreateMetin2GameMutex()
{
	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&sa, sizeof(SECURITY_ATTRIBUTES));
	sa.nLength				= sizeof(sa);
	sa.lpSecurityDescriptor	= NULL;
	sa.bInheritHandle		= FALSE;

	return CreateMutex(&sa, FALSE, "Metin2GameMutex");
}

void DestroyMetin2GameMutex(HANDLE hMutex)
{
	if (hMutex)
	{
		ReleaseMutex(hMutex);
		hMutex = NULL;
	}
}

void __ErrorPythonLibraryIsNotExist()
{
	LogBoxf("FATAL ERROR!! Python Library file not exist!");
}

bool __IsTimeStampOption(LPSTR lpCmdLine)
{
	const char* TIMESTAMP = "/timestamp";
	return (strncmp(lpCmdLine, TIMESTAMP, strlen(TIMESTAMP))==0);
}

void __PrintTimeStamp()
{
#ifdef	_DEBUG
	if (__IS_TEST_SERVER_MODE__)
		LogBoxf("METIN2 BINARY TEST DEBUG VERSION %s  ( MS C++ %d Compiled )", __TIMESTAMP__, _MSC_VER);
	else
		LogBoxf("METIN2 BINARY DEBUG VERSION %s ( MS C++ %d Compiled )", __TIMESTAMP__, _MSC_VER);
	
#else
	if (__IS_TEST_SERVER_MODE__)
		LogBoxf("METIN2 BINARY TEST VERSION %s  ( MS C++ %d Compiled )", __TIMESTAMP__, _MSC_VER);
	else
		LogBoxf("METIN2 BINARY DISTRIBUTE VERSION %s ( MS C++ %d Compiled )", __TIMESTAMP__, _MSC_VER);			
#endif			
}

bool __IsLocaleOption(LPSTR lpCmdLine)
{
	return (strcmp(lpCmdLine, "--locale") == 0);
}

bool __IsLocaleVersion(LPSTR lpCmdLine)
{
	return (strcmp(lpCmdLine, "--perforce-revision") == 0);
}

#ifdef USE_OPENID
//2012.07.16 ±è¿ë¿í
//ÀÏº» OpenID Áö¿ø. ÀÎÁõÅ° ÀÎÀÚ Ãß°¡
bool __IsOpenIDAuthKeyOption(LPSTR lpCmdLine)
{
	return (strcmp(lpCmdLine, "--openid-authkey") == 0);
}

bool __IsOpenIDTestOption(LPSTR lpCmdLine) //Å¬¶óÀÌ¾ğÆ®¿¡¼­ ·Î±×ÀÎÀÌ °¡´ÉÇÏ´Ù.
{
	return (strcmp(lpCmdLine, "--openid-test") == 0);
}
#endif /* USE_OPENID */

// ==============  DLL d ==============
// typedef int(__stdcall* fnPSAuthenticate)(void);
// typedef const char* (__stdcall* fnPSGetLastError)(void);
// static HMODULE g_hPrimeModule = NULL;

// static void PrimeShield_Initialize()
// {
	// CHAR szFilename[] = { 'V', 'e', 'r', 'i', 'b', 'l', 'e', 'P', 'r', 'o', 't', 'e', 'c', 't', '.', 'd', 'l', 'l', 0x0 };
	// CHAR szFuncName[] = { 'P', 'S', 'A', 'u', 't', 'h', 'e', 'n', 't', 'i', 'c', 'a', 't', 'e', 0x0 };


	// DWORD dwAttrib = GetFileAttributesA(szFilename);
	// if (dwAttrib == INVALID_FILE_ATTRIBUTES)
	// {
		// MessageBoxA(NULL, "VeribleProtect.dll bulunamadı!", "Uyarı", MB_ICONWARNING | MB_OK);
		// return;
	// }

	// HMODULE hDll = LoadLibraryA(szFilename);
	// if (!hDll)
		// return;  

	// g_hPrimeModule = hDll;

	
	// fnPSAuthenticate pAuth = (fnPSAuthenticate)GetProcAddress(hDll, szFuncName);
	// if (!pAuth)
		// return;  
	// pAuth();
// }

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// PrimeShield_Initialize();

	if (strstr(lpCmdLine, "--hackshield") != 0)
		return 0;

#ifdef ENABLE_FIX_D_YMIR_WORK
	if (WrongPathYmir("D:\\ymir work\\"))
	{
		MessageBox(NULL,("Lütfen D:\\ymir work\\ siliniz !"),("OtoPatcher"), MB_ICONSTOP);
		return 0;
	}
#endif

#ifdef _DEBUG
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_CRT_DF | _CRTDBG_LEAK_CHECK_DF );
	//_CrtSetBreakAlloc( 110247 ); 
#endif

	ApplicationStringTable_Initialize(hInstance);

	LocaleService_LoadConfig("locale.cfg");
	SetDefaultCodePage(LocaleService_GetCodePage());	

#ifdef XTRAP_CLIENT_ENABLE
	if (!XTrap_Init())
		return 0;
#endif

#ifdef USE_AHNLAB_HACKSHIELD
	if (!HackShield_Init())
		return 0;
#endif

#ifdef USE_NPROTECT_GAMEGUARD
	if (!GameGuard_Init())
		return 0;
#endif

#if defined(CHECK_LATEST_DATA_FILES)
	if (!CheckLatestFiles())
		return 0;
#endif

	bool bQuit = false;
	bool bAuthKeyChecked = false;	//OpenID ¹öÀü¿¡¼­ ÀÎÁõÅ°°¡ µé¾î¿Ô´ÂÁö ¾Ë±â À§ÇÑ ÀÎÀÚ.
	int nArgc = 0;
	PCHAR* szArgv = CommandLineToArgv( lpCmdLine, &nArgc );

	for( int i=0; i < nArgc; i++ ) {
		if(szArgv[i] == 0)
			continue;
		if (__IsLocaleVersion(szArgv[i])) // #0000829: [M2EU] ¹öÀü ÆÄÀÏÀÌ Ç×»ó »ı±âÁö ¾Êµµ·Ï ¼öÁ¤ 
		{
			char szModuleName[MAX_PATH];
			char szVersionPath[MAX_PATH];
			GetModuleFileName(NULL, szModuleName, sizeof(szModuleName));
			sprintf(szVersionPath, "%s.version", szModuleName);
			FILE* fp = fopen(szVersionPath, "wt");
			if (fp)
			{
				extern int METIN2_GET_VERSION();
				fprintf(fp, "r%d\n", METIN2_GET_VERSION());
				fclose(fp);
			}
			bQuit = true;
		} else if (__IsLocaleOption(szArgv[i]))
		{
			FILE* fp=fopen("locale.txt", "wt");
			fprintf(fp, "service[%s] code_page[%d]", 
				LocaleService_GetName(), LocaleService_GetCodePage());
			fclose(fp);
			bQuit = true;
		} else if (__IsTimeStampOption(szArgv[i]))
		{
			__PrintTimeStamp();
			bQuit = true;
		} else if ((strcmp(szArgv[i], "--force-set-locale") == 0))
		{
			// locale ¼³Á¤¿£ ÀÎÀÚ°¡ µÎ °³ ´õ ÇÊ¿äÇÔ (·ÎÄÉÀÏ ¸íÄª, µ¥ÀÌÅÍ °æ·Î)
			if (nArgc <= i + 2)
			{
				MessageBox(NULL, "Invalid arguments", ApplicationStringTable_GetStringz(IDS_APP_NAME, "APP_NAME"), MB_ICONSTOP);
				goto Clean;
			}

			const char* localeName = szArgv[++i];
			const char* localePath = szArgv[++i];

			LocaleService_ForceSetLocale(localeName, localePath);
		}
#ifdef USE_OPENID
		else if (__IsOpenIDAuthKeyOption(szArgv[i]))	//2012.07.16 OpenID : ±è¿ë¿í
		{
			// ÀÎÁõÅ° ¼³Á¤¿£ ÀÎÀÚ°¡ ÇÑ °³ ´õ ÇÊ¿äÇÔ (ÀÎÁõÅ°)
			if (nArgc <= i + 1)
			{
				MessageBox(NULL, "Invalid arguments", ApplicationStringTable_GetStringz(IDS_APP_NAME, "APP_NAME"), MB_ICONSTOP);
				goto Clean;
			}

			const char* authKey = szArgv[++i];

			//ongoing (2012.07.16)
			//ÀÎÁõÅ° ÀúÀåÇÏ´Â ºÎºĞ
			LocaleService_SetOpenIDAuthKey(authKey);

			bAuthKeyChecked = true;
		}
		else if (__IsOpenIDTestOption(szArgv[i]))
		{
			openid_test = 1;

		}
#endif /* USE_OPENID */
	}

#ifdef USE_OPENID
	//OpenID
	//OpenID Å¬¶óÀÌ¾ğÆ®ÀÇ °æ¿ìÀÎÁõÅ°¸¦ ¹Ş¾Æ¿ÀÁö ¾ÊÀ» °æ¿ì (À¥À» Á¦¿ÜÇÏ°í ½ÇÇà ½Ã) Å¬¶óÀÌ¾ğÆ® Á¾·á.
	
	if (false == bAuthKeyChecked && !openid_test)
	{
		MessageBox(NULL, "Invalid execution", ApplicationStringTable_GetStringz(IDS_APP_NAME, "APP_NAME"), MB_ICONSTOP);
		goto Clean;
	}
#endif /* USE_OPENID */


	if(bQuit)
		goto Clean;

#if defined(NEEDED_COMMAND_ARGUMENT)
	// ¿É¼ÇÀÌ ¾øÀ¸¸é ºñÁ¤»ó ½ÇÇàÀ¸·Î °£ÁÖ, ÇÁ·Î±×·¥ Á¾·á
	if (strstr(lpCmdLine, NEEDED_COMMAND_ARGUMENT) == 0) {
		MessageBox(NULL, ApplicationStringTable_GetStringz(IDS_ERR_MUST_LAUNCH_FROM_PATCHER, "ERR_MUST_LAUNCH_FROM_PATCHER"), ApplicationStringTable_GetStringz(IDS_APP_NAME, "APP_NAME"), MB_ICONSTOP);
			goto Clean;
	}
#endif

#if defined(NEEDED_COMMAND_CLIPBOARD)
	{
		CHAR szSecKey[256];
		CPostIt cPostIt( "VOLUME1" );

		if( cPostIt.Get( "SEC_KEY", szSecKey, sizeof(szSecKey) ) == FALSE ) {
			MessageBox(NULL, ApplicationStringTable_GetStringz(IDS_ERR_MUST_LAUNCH_FROM_PATCHER, "ERR_MUST_LAUNCH_FROM_PATCHER"), ApplicationStringTable_GetStringz(IDS_APP_NAME, "APP_NAME"), MB_ICONSTOP);
			goto Clean;
		}
		if( strstr(szSecKey, NEEDED_COMMAND_CLIPBOARD) == 0 ) {
			MessageBox(NULL, ApplicationStringTable_GetStringz(IDS_ERR_MUST_LAUNCH_FROM_PATCHER, "ERR_MUST_LAUNCH_FROM_PATCHER"), ApplicationStringTable_GetStringz(IDS_APP_NAME, "APP_NAME"), MB_ICONSTOP);
			goto Clean;
		}
		cPostIt.Empty();
	}
#endif

#ifdef CEF_BROWSER
	CefWebBrowser_Startup(hInstance);
#else
	WebBrowser_Startup(hInstance);
#endif

	if (!CheckPythonLibraryFilenames())
	{
		__ErrorPythonLibraryIsNotExist();
		goto Clean;
	}

	Main(hInstance, lpCmdLine);

#ifdef USE_NPROTECT_GAMEGUARD
	GameGuard_NoticeMessage();
#endif

#ifdef CEF_BROWSER
	CefWebBrowser_Cleanup();
#else
	WebBrowser_Cleanup();
#endif

	::CoUninitialize();

	if(gs_szErrorString[0])
		MessageBox(NULL, gs_szErrorString, ApplicationStringTable_GetStringz(IDS_APP_NAME, "APP_NAME"), MB_ICONSTOP);

Clean:
#ifdef USE_AHNLAB_HACKSHIELD
	HackShield_Shutdown();
#endif
	SAFE_FREE_GLOBAL(szArgv);

	return 0;
}

static void GrannyError(granny_log_message_type Type,
						granny_log_message_origin Origin,
						char const* File,
						granny_int32x Line,
						char const *Error,
						void *UserData)
{
//TraceError("GRANNY: %s", Error);
}

int Setup(LPSTR lpCmdLine)
{
	/* 
	 *	Å¸ÀÌ¸Ó Á¤¹Ğµµ¸¦ ¿Ã¸°´Ù.
	 */
	TIMECAPS tc; 
	UINT wTimerRes; 

	if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR) 
		return 0;

	wTimerRes = MINMAX(tc.wPeriodMin, 1, tc.wPeriodMax); 
	timeBeginPeriod(wTimerRes); 

	/*
	 *	±×·¡´Ï ¿¡·¯ ÇÚµé¸µ
	 */

	granny_log_callback Callback;
	Callback.Function = GrannyError;
	Callback.UserData = 0;
	GrannySetLogCallback(&Callback);
	return 1;
}
