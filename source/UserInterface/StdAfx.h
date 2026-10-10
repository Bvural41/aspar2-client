#pragma once

#pragma warning(disable:4702)
#pragma warning(disable:4100)
#pragma warning(disable:4201)
#pragma warning(disable:4511)
#pragma warning(disable:4663)
#pragma warning(disable:4018)
#pragma warning(disable:4245)

#if _MSC_VER >= 1400
//if don't use below, time_t is 64bit
#define _USE_32BIT_TIME_T
#endif

#include "../EterLib/StdAfx.h"
#include "../EterPythonLib/StdAfx.h"
#include "../GameLib/StdAfx.h"
#include "../ScriptLib/StdAfx.h"
#include "../MilesLib/StdAfx.h"
#include "../EffectLib/StdAfx.h"
#include "../PRTerrainLib/StdAfx.h"
#include "../SpeedTreeLib/StdAfx.h"

#ifndef __D3DRM_H__
#define __D3DRM_H__
#endif

#include <dshow.h>
#if !defined(__ANDROID__) && !defined(__APPLE__)
#include <qedit.h>
#endif


#include "Locale.h"

#include "GameType.h"
extern DWORD __DEFAULT_CODE_PAGE__;

#define APP_NAME	"Metin 2"

enum
{
	POINT_MAX_NUM = 256,
	CHARACTER_NAME_MAX_LEN = 32,
#if defined(LOCALE_SERVICE_JAPAN)
	PLAYER_NAME_MAX_LEN = 16,
#else
	PLAYER_NAME_MAX_LEN = 12,
#endif
};

void initudp();
void initapp();
void initime();
void initsystem();
void initchr();
void initchrmgr();
void initChat();
void initTextTail();
void initime();
void initItem();
void initNonPlayer();
void initnet();
void initPlayer();
void initSectionDisplayer();
void initServerStateChecker();
void initTrade();
void initMiniMap();
void initProfiler();
void initEvent();
void initeffect();
void initsnd();
void initeventmgr();
void initBackground();
void initwndMgr();
void initshop();
void initpack();
void initskill();
void initfly();
void initquest();
void initsafebox();
void initguild();
void initMessenger();
void initRenderTarget();
#ifdef ENABLE_MESSENGER_RENEWAL
void initCommunity();
#endif
#ifdef ENABLE_INGAME_WIKI
void initWiki();
#endif
#ifdef ENABLE_TITLE_SYSTEM
void initTitleSystem();
#endif
#ifdef ENABLE_TREASURE_EVENT
void inittreasure_event();
#endif
#ifdef ENABLE_BATTLE_ROYALE
void initbattleRoyaleMgr();
#endif
#ifdef ENABLE_BATTLEPASS
void initBattlePass();
#endif
#ifdef ENABLE_SHOP_SEARCH_SYSTEM
void initprivateShopSearch();
#endif
#ifdef ENABLE_SWITCHBOT
void initSwitchbot();
#endif
#ifdef ENABLE_EVENT_SYSTEM
void initGameEvents();
#endif
#ifdef ENABLE_CUBE_RENEWAL
void intcuberenewal();
#endif
#ifdef ENABLE_BATTLE_FIELD
void initCombatZoneSystem();
#endif
#ifdef ENABLE_SASH_SYSTEM
void initSash();
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
void	initChangeLook();
#endif
#ifdef ENABLE_AURA_SYSTEM
void initAura();
#endif
#ifdef ENABLE_RIDING_EXTENDED
void initmountupgrade();
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
void initskillpet();
#endif

extern const std::string& ApplicationStringTable_GetString(DWORD dwID);
extern const std::string& ApplicationStringTable_GetString(DWORD dwID, LPCSTR szKey);

extern const char* ApplicationStringTable_GetStringz(DWORD dwID);
extern const char* ApplicationStringTable_GetStringz(DWORD dwID, LPCSTR szKey);

extern void ApplicationSetErrorString(const char* szErrorString);
