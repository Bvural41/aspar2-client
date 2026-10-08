#if defined(USE_OPENGL_ES)
#include <SDL2/SDL.h>
#endif
#include "StdAfx.h"
#include "PythonSystem.h"
#if defined(ENABLE_TITLE_SYSTEM)
#include "PythonTitleManager.h"
#endif
#include "PythonApplication.h"

#ifdef __ANDROID__
#include <android/log.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "Metin2System", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "Metin2System", __VA_ARGS__)

extern "C" int g_nAndroidScreenWidth;
extern "C" int g_nAndroidScreenHeight;
extern std::string g_strAndroidExternalPath;

static std::string GetConfigFilePath(const char* szFileName, bool bForWrite)
{
	std::vector<std::string> candidateDirs;
	candidateDirs.push_back("/sdcard/aspar2/");
	candidateDirs.push_back("/storage/emulated/0/aspar2/");
	candidateDirs.push_back("/sdcard/metin2/");
	candidateDirs.push_back("/storage/emulated/0/metin2/");
	if (!g_strAndroidExternalPath.empty())
	{
		std::string ext = g_strAndroidExternalPath;
		if (ext.back() != '/') ext += "/";
		candidateDirs.push_back(ext);
	}
	candidateDirs.push_back(""); // current working directory (internal files)

	if (!bForWrite)
	{
		for (size_t i = 0; i < candidateDirs.size(); ++i)
		{
			std::string fullPath = candidateDirs[i] + szFileName;
			FILE* fp = fopen(fullPath.c_str(), "rb");
			if (fp)
			{
				fclose(fp);
				LOGI("GetConfigFilePath(read): Found %s", fullPath.c_str());
				return fullPath;
			}
		}
		LOGI("GetConfigFilePath(read): %s not found in candidates, defaulting to %s", szFileName, szFileName);
		return std::string(szFileName);
	}
	else
	{
		// 1. If the file already exists in one of the candidate locations, overwrite that one
		for (size_t i = 0; i < candidateDirs.size(); ++i)
		{
			std::string fullPath = candidateDirs[i] + szFileName;
			FILE* fp = fopen(fullPath.c_str(), "r+");
			if (fp)
			{
				fclose(fp);
				LOGI("GetConfigFilePath(write): Existing file found at %s", fullPath.c_str());
				return fullPath;
			}
		}

		// 2. Otherwise test which candidate directory allows creating/appending to the file
		for (size_t i = 0; i < candidateDirs.size(); ++i)
		{
			std::string fullPath = candidateDirs[i] + szFileName;
			FILE* fp = fopen(fullPath.c_str(), "a");
			if (fp)
			{
				fclose(fp);
				LOGI("GetConfigFilePath(write): Selected writable location %s", fullPath.c_str());
				return fullPath;
			}
		}

		LOGI("GetConfigFilePath(write): Fallback to %s", szFileName);
		return std::string(szFileName);
	}
}
#else
static std::string GetConfigFilePath(const char* szFileName, bool bForWrite)
{
	return std::string(szFileName);
}
#endif

#define DEFAULT_VALUE_ALWAYS_SHOW_NAME		true

void CPythonSystem::SetInterfaceHandler(PyObject * poHandler)
{
// NOTE : ???????? ?????? ??? ??????. ????????? ???? ???? Python???? ?????? ???? ??? ???.
//        ???? __del__?? Destroy? ???? Handler? NULL? ????????. - [levites]
//	if (m_poInterfaceHandler)
//		Py_DECREF(m_poInterfaceHandler);

	m_poInterfaceHandler = poHandler;

//	if (m_poInterfaceHandler)
//		Py_INCREF(m_poInterfaceHandler);
}

void CPythonSystem::DestroyInterfaceHandler()
{
	m_poInterfaceHandler = NULL;
}

void CPythonSystem::SaveWindowStatus(int iIndex, int iVisible, int iMinimized, int ix, int iy, int iHeight)
{
	m_WindowStatus[iIndex].isVisible	= iVisible;
	m_WindowStatus[iIndex].isMinimized	= iMinimized;
	m_WindowStatus[iIndex].ixPosition	= ix;
	m_WindowStatus[iIndex].iyPosition	= iy;
	m_WindowStatus[iIndex].iHeight		= iHeight;
}

void CPythonSystem::GetDisplaySettings()
{
	memset(m_ResolutionList, 0, sizeof(TResolution) * RESOLUTION_MAX_NUM);
	m_ResolutionCount = 0;

#if defined(USE_OPENGL_ES)
	int numModes = SDL_GetNumDisplayModes(0);
	for (int i = 0; i < numModes && m_ResolutionCount < RESOLUTION_MAX_NUM; ++i)
	{
		SDL_DisplayMode mode;
		if (SDL_GetDisplayMode(0, i, &mode) == 0)
		{
			if (mode.w < 800 || mode.h < 600)
				continue;

			DWORD bpp = SDL_BITSPERPIXEL(mode.format);
			if (bpp == 0) bpp = 32;

			bool found = false;
			for (int r = 0; r < m_ResolutionCount; ++r)
			{
				if (m_ResolutionList[r].width == (DWORD)mode.w &&
					m_ResolutionList[r].height == (DWORD)mode.h &&
					m_ResolutionList[r].bpp == bpp)
				{
					bool freFound = false;
					for (int f = 0; f < m_ResolutionList[r].frequency_count; ++f)
					{
						if (m_ResolutionList[r].frequency[f] == (DWORD)mode.refresh_rate)
						{
							freFound = true;
							break;
						}
					}
					if (!freFound && m_ResolutionList[r].frequency_count < FREQUENCY_MAX_NUM)
					{
						m_ResolutionList[r].frequency[m_ResolutionList[r].frequency_count++] = mode.refresh_rate;
					}
					found = true;
					break;
				}
			}

			if (!found)
			{
				m_ResolutionList[m_ResolutionCount].width = mode.w;
				m_ResolutionList[m_ResolutionCount].height = mode.h;
				m_ResolutionList[m_ResolutionCount].bpp = bpp;
				m_ResolutionList[m_ResolutionCount].frequency[0] = mode.refresh_rate ? mode.refresh_rate : 60;
				m_ResolutionList[m_ResolutionCount].frequency_count = 1;
				m_ResolutionCount++;
			}
		}
	}

	if (m_ResolutionCount == 0)
	{
		m_ResolutionList[0].width = 1024;
		m_ResolutionList[0].height = 768;
		m_ResolutionList[0].bpp = 32;
		m_ResolutionList[0].frequency[0] = 60;
		m_ResolutionList[0].frequency_count = 1;
		m_ResolutionCount = 1;
	}
#else
	const LPDIRECT3D9EX lpD3D = CPythonGraphic::Instance().GetD3D();
	if (!lpD3D)
		return;

	D3DADAPTER_IDENTIFIER9 d3dAdapterIdentifier;
	D3DDISPLAYMODEEX d3ddmDesktop = {};
	d3ddmDesktop.Size = sizeof(D3DDISPLAYMODEEX);

	lpD3D->GetAdapterIdentifier(D3DADAPTER_DEFAULT, D3DENUM_WHQL_LEVEL, &d3dAdapterIdentifier);
	lpD3D->GetAdapterDisplayModeEx(D3DADAPTER_DEFAULT, &d3ddmDesktop, nullptr);

	D3DDISPLAYMODEFILTER filter = {};
	filter.Size = sizeof(D3DDISPLAYMODEFILTER);
	filter.Format = d3ddmDesktop.Format;

	const DWORD dwNumAdapterModes = lpD3D->GetAdapterModeCountEx(D3DADAPTER_DEFAULT, &filter);

	for (UINT iMode = 0; iMode < dwNumAdapterModes; iMode++)
	{
		D3DDISPLAYMODEEX DisplayMode = {};
		DisplayMode.Size = sizeof(D3DDISPLAYMODEEX);

		lpD3D->EnumAdapterModesEx(D3DADAPTER_DEFAULT, &filter, iMode, &DisplayMode);
		DWORD bpp = 0;

		if (DisplayMode.Width < 800 || DisplayMode.Height < 600)
			continue;

		if (DisplayMode.Format == D3DFMT_R5G6B5)
			bpp = 16;
		else if (DisplayMode.Format == D3DFMT_X8R8G8B8)
			bpp = 32;
		else
			continue;

	int check_res = false;

		for (int i = 0; !check_res && i < m_ResolutionCount; ++i)
		{
			if (m_ResolutionList[i].bpp != bpp ||
				m_ResolutionList[i].width != DisplayMode.Width ||
				m_ResolutionList[i].height != DisplayMode.Height)
				continue;

			int check_fre = false;

			for (int j = 0; j < m_ResolutionList[i].frequency_count; ++j)
			{
				if (m_ResolutionList[i].frequency[j] == DisplayMode.RefreshRate)
				{
					check_fre = true;
					break;
				}
			}

			if (!check_fre)
				if (m_ResolutionList[i].frequency_count < FREQUENCY_MAX_NUM)
					m_ResolutionList[i].frequency[m_ResolutionList[i].frequency_count++] = DisplayMode.RefreshRate;

			check_res = true;
		}

		if (!check_res)
		{
			if (m_ResolutionCount < RESOLUTION_MAX_NUM)
			{
				m_ResolutionList[m_ResolutionCount].width			= DisplayMode.Width;
				m_ResolutionList[m_ResolutionCount].height			= DisplayMode.Height;
				m_ResolutionList[m_ResolutionCount].bpp				= bpp;
				m_ResolutionList[m_ResolutionCount].frequency[0]	= DisplayMode.RefreshRate;
				m_ResolutionList[m_ResolutionCount].frequency_count	= 1;

				++m_ResolutionCount;
			}
		}
	}
#endif
}

int	CPythonSystem::GetResolutionCount()
{
	return m_ResolutionCount;
}

int CPythonSystem::GetFrequencyCount(int index)
{
	if (index >= m_ResolutionCount)
		return 0;

    return m_ResolutionList[index].frequency_count;
}

bool CPythonSystem::GetResolution(int index, OUT DWORD *width, OUT DWORD *height, OUT DWORD *bpp)
{
	if (index >= m_ResolutionCount)
		return false;

	*width = m_ResolutionList[index].width;
	*height = m_ResolutionList[index].height;
	*bpp = m_ResolutionList[index].bpp;
	return true;
}

bool CPythonSystem::GetFrequency(int index, int freq_index, OUT DWORD *frequncy)
{
	if (index >= m_ResolutionCount)
		return false;

	if (freq_index >= m_ResolutionList[index].frequency_count)
		return false;

	*frequncy = m_ResolutionList[index].frequency[freq_index];
	return true;
}

int	CPythonSystem::GetResolutionIndex(DWORD width, DWORD height, DWORD bit)
{
	DWORD re_width, re_height, re_bit;
	int i = 0;

	while (GetResolution(i, &re_width, &re_height, &re_bit))
	{
		if (re_width == width)
			if (re_height == height)
				if (re_bit == bit)
					return i;
		i++;
	}

	return 0;
}

int	CPythonSystem::GetFrequencyIndex(int res_index, DWORD frequency)
{
	DWORD re_frequency;
	int i = 0;

	while (GetFrequency(res_index, i, &re_frequency))
	{
		if (re_frequency == frequency)
			return i;

		i++;
	}

	return 0;
}

DWORD CPythonSystem::GetWidth()
{
	return m_Config.width;
}

DWORD CPythonSystem::GetHeight()
{
	return m_Config.height;
}
DWORD CPythonSystem::GetBPP()
{
	return m_Config.bpp;
}
DWORD CPythonSystem::GetFrequency()
{
	return m_Config.frequency;
}

bool CPythonSystem::IsNoSoundCard()
{
	return m_Config.bNoSoundCard;
}

bool CPythonSystem::IsSoftwareCursor()
{
	return m_Config.is_software_cursor;
}

float CPythonSystem::GetMusicVolume()
{
	return m_Config.music_volume;
}

int CPythonSystem::GetSoundVolume()
{
	return m_Config.voice_volume;
}

void CPythonSystem::SetMusicVolume(float fVolume)
{
	m_Config.music_volume = fVolume;
}

void CPythonSystem::SetSoundVolumef(float fVolume)
{
	m_Config.voice_volume = int(5 * fVolume);
}

int CPythonSystem::GetDistance()
{
	return m_Config.iDistance;
}

#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
int CPythonSystem::GetShadowTargetLevel()
{
	return m_Config.iShadowTargetLevel;
}

void CPythonSystem::SetShadowTargetLevel(unsigned int level)
{
	m_Config.iShadowTargetLevel = MINMAX(CPythonBackground::SHADOW_NONE, level, CPythonBackground::SHADOW_ALL);
	CPythonBackground::Instance().RefreshShadowTargetLevel();
}

int CPythonSystem::GetShadowQualityLevel()
{
	return m_Config.iShadowQualityLevel;
}

void CPythonSystem::SetShadowQualityLevel(unsigned int level)
{
	m_Config.iShadowQualityLevel = MINMAX(CPythonBackground::SHADOW_BAD, level, CPythonBackground::SHADOW_GOOD);
	CPythonBackground::Instance().RefreshShadowQualityLevel();
}
#else
int CPythonSystem::GetShadowLevel() const
{
	return m_Config.iShadowLevel;
}

void CPythonSystem::SetShadowLevel(unsigned int level)
{
	m_Config.iShadowLevel = MIN(level, 5);
	CPythonBackground::Instance().RefreshShadowLevel();
}
#endif

#ifdef ENABLE_DICE_SYSTEM
void CPythonSystem::SetDiceChatShow(int iFlag)
{
	m_Config.bDiceFlag = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsDiceChatShow()
{
	return m_Config.bDiceFlag;
}
#endif

int CPythonSystem::IsSaveID()
{
	return m_Config.isSaveID;
}

const char * CPythonSystem::GetSaveID()
{
	return m_Config.SaveID;
}

bool CPythonSystem::isViewCulling()
{
	return m_Config.is_object_culling;
}

void CPythonSystem::SetSaveID(int iValue, const char * c_szSaveID)
{
	if (iValue != 1)
		return;
	
	m_Config.isSaveID = iValue;
	strncpy(m_Config.SaveID, c_szSaveID, sizeof(m_Config.SaveID) - 1);
}

#ifdef ENABLE_TRANSLATOR_GOOGLE_SYSTEM
const char * CPythonSystem::GetTransLangKey()
{
	return m_Config.TransLangKey;
}

void CPythonSystem::SetTransLangKey(const char * c_szTransLangKey)
{
	strncpy(m_Config.TransLangKey, c_szTransLangKey, sizeof(m_Config.TransLangKey) - 1);
}
#endif

CPythonSystem::TConfig * CPythonSystem::GetConfig()
{
	return &m_Config;
}

void CPythonSystem::SetConfig(TConfig * pNewConfig)
{
	m_Config = *pNewConfig;
}

void CPythonSystem::SetDefaultConfig()
{
	memset(&m_Config, 0, sizeof(m_Config));

#ifdef __ANDROID__
	m_Config.width				= (g_nAndroidScreenWidth > 0) ? g_nAndroidScreenWidth : 1067;
	m_Config.height				= (g_nAndroidScreenHeight > 0) ? g_nAndroidScreenHeight : 600;
#else
	m_Config.width				= 1024;
	m_Config.height				= 768;
#endif
	m_Config.bpp				= 32;

#if defined( LOCALE_SERVICE_WE_JAPAN )
	m_Config.bWindowed			= true;
#else
	m_Config.bWindowed			= false;
#endif

	m_Config.is_software_cursor	= false;
	m_Config.is_object_culling	= true;
	m_Config.iDistance			= 3;

	m_Config.gamma				= 3;
	m_Config.music_volume		= (float)pow(10.0f, (-1.0f + (1.0f / 5.0f))); // MUSIC_VOLUME 1
	m_Config.voice_volume		= 2;                          // VOICE_VOLUME 2

	m_Config.bDecompressDDS		= 0;
	m_Config.bSoftwareTiling	= 0;
#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
	m_Config.iShadowTargetLevel = 3;
	m_Config.iShadowQualityLevel = 2;
#else
	m_Config.iShadowLevel = 3;
#endif
	m_Config.bViewChat			= true;
#ifdef ENABLE_TITLE_SYSTEM
	m_Config.bShowTitle			= true;
#endif
	m_Config.bAlwaysShowName	= DEFAULT_VALUE_ALWAYS_SHOW_NAME;
	m_Config.bShowDamage		= true;
	m_Config.bShowSalesText		= true;
#ifdef ENABLE_REFINE_RENEWAL	
	m_Config.bRefineStatus		= true;
#endif
#ifdef ENABLE_AUTO_SYSTEM
	m_Config.bAutoHuntStone = false;
	m_Config.bAutoHuntPawn = true;
	m_Config.bAutoHuntSPawn = true;
	m_Config.bAutoHuntKnight = true;
	m_Config.bAutoHuntSKnight = true;
	m_Config.bAutoHuntBoss = true;
	m_Config.bAutoHuntKing = true;
	m_Config.iAutoHuntLevelMin = 0;
	m_Config.iAutoHuntLevelMax = 150;
#endif
#ifdef ENABLE_FOG_FIX
	m_Config.bShowFogMode		= true;
#endif
#if defined(ENABLE_ENVIRONMENT_EFFECT_OPTION)
	m_Config.bShowNightMode			= false;
	m_Config.bShowSnowMode			= false;
	m_Config.bShowSnowTextureMode	= false;
#endif
#if defined(ENABLE_SHOW_MOB_INFO)
	m_Config.bShowMobLevel		= false;
	m_Config.bShowMobAIFlag		= false;
#endif
#ifdef ENABLE_AUTO_PICKUP_SYSTEM
	m_Config.bAutoPickup		= true;
#endif
#ifdef ENABLE_FOV_OPTION
	m_Config.bExtendedFOV = false;
#endif
#ifdef ENABLE_GRAPHIC_ON_OFF
	m_Config.iEffectLevel		= 0;
	// m_Config.iPrivateShopLevel	= 0;
	m_Config.iDropItemLevel		= 0;

	m_Config.bPetStatus			= false;
	m_Config.bNpcNameStatus		= false;
#endif
#ifdef ENABLE_PREMIUM_AFFECT_SYSTEM
	m_Config.bShowPremiumAffect = true;
#endif
#ifdef ENABLE_DAMAGE_BAR
	m_Config.bShowDamageBar = false;
#endif
#ifdef ENABLE_TIME_SYSTEM
	m_Config.bShowTimeSystem = true;
#endif
#ifdef ENABLE_INVENTORY_ADDITION
#ifdef __ANDROID__
	m_Config.bShowInventoryAddition = false;
#else
	m_Config.bShowInventoryAddition = true;
#endif
#endif
#ifdef ENABLE_PACKET_INFO_SYSTEM
	m_Config.bShowInfoWindow = true;
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	m_Config.bShowFlag			= false;
#endif
#ifdef ENABLE_DICE_SYSTEM
	m_Config.bDiceFlag = true;
#endif
#ifdef ENABLE_SAVE_CAMERA_MODE
	m_Config.bCameraMode = 1;
#endif
#ifdef ENABLE_ANIMATION_OPTIMIZATION
	m_Config.bShowOtherCharAttacked = true;
#endif
#ifdef ENABLE_FAST_ITEM_DELETE_SYSTEM
	m_Config.bAutoSellStatus = false;
#endif
}

bool CPythonSystem::IsWindowed()
{
	return m_Config.bWindowed;
}

bool CPythonSystem::IsViewChat()
{
	return m_Config.bViewChat;
}

void CPythonSystem::SetViewChatFlag(int iFlag)
{
	m_Config.bViewChat = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsAlwaysShowName()
{
	return m_Config.bAlwaysShowName;
}

void CPythonSystem::SetAlwaysShowNameFlag(int iFlag)
{
	m_Config.bAlwaysShowName = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsShowDamage()
{
	return m_Config.bShowDamage;
}

void CPythonSystem::SetShowDamageFlag(int iFlag)
{
	m_Config.bShowDamage = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsShowSalesText()
{
	return m_Config.bShowSalesText;
}

void CPythonSystem::SetShowSalesTextFlag(int iFlag)
{
	m_Config.bShowSalesText = iFlag == 1 ? true : false;
}

#if defined(ENABLE_ENVIRONMENT_EFFECT_OPTION)
void CPythonSystem::SetFogModeOption(int iOpt)
{
	m_Config.bShowFogMode = iOpt == 1 ? true : false;
}

bool CPythonSystem::GetFogModeOption()
{
	return m_Config.bShowFogMode;
}
#endif

#if defined(ENABLE_ENVIRONMENT_EFFECT_OPTION)
void CPythonSystem::SetNightModeOption(int iOpt)
{
	m_Config.bShowNightMode = iOpt == 1 ? true : false;
}

bool CPythonSystem::GetNightModeOption()
{
	return m_Config.bShowNightMode;
}

void CPythonSystem::SetSnowModeOption(int iOpt)
{
	m_Config.bShowSnowMode = iOpt == 1 ? true : false;
}

bool CPythonSystem::GetSnowModeOption()
{
	return m_Config.bShowSnowMode;
}

void CPythonSystem::SetSnowTextureModeOption(int iOpt)
{
	m_Config.bShowSnowTextureMode = iOpt == 1 ? true : false;
}

bool CPythonSystem::GetSnowTextureModeOption()
{
	return m_Config.bShowSnowTextureMode;
}
#endif

#if defined(ENABLE_SHOW_MOB_INFO)
void CPythonSystem::SetShowMobLevel(int iOpt)
{
	m_Config.bShowMobLevel = iOpt == 1 ? true : false;
}

bool CPythonSystem::IsShowMobLevel()
{
	return m_Config.bShowMobLevel;
}

void CPythonSystem::SetShowMobAIFlag(int iOpt)
{
	m_Config.bShowMobAIFlag = iOpt == 1 ? true : false;
}

bool CPythonSystem::IsShowMobAIFlag()
{
	return m_Config.bShowMobAIFlag;
}
#endif

#ifdef ENABLE_AUTO_PICKUP_SYSTEM
void CPythonSystem::SetAutoPickup(int iFlag)
{
	m_Config.bAutoPickup = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsAutoPickup()
{
	return m_Config.bAutoPickup;
}
#endif

#ifdef ENABLE_GRAPHIC_ON_OFF
int CPythonSystem::GetEffectLevel()
{
	return m_Config.iEffectLevel;
}
void CPythonSystem::SetEffectLevel(unsigned int level)
{
	m_Config.iEffectLevel = MIN(level, 5);
}

// int CPythonSystem::GetPrivateShopLevel()
// {
	// return m_Config.iPrivateShopLevel;
// }
// void CPythonSystem::SetPrivateShopLevel(unsigned int level)
// {
	// m_Config.iPrivateShopLevel = MIN(level, 5);
// }

int CPythonSystem::GetDropItemLevel()
{
	return m_Config.iDropItemLevel;
}
void CPythonSystem::SetDropItemLevel(unsigned int level)
{
	m_Config.iDropItemLevel = MIN(level, 5);
}

bool CPythonSystem::IsPetStatus()
{
	return m_Config.bPetStatus;
}
void CPythonSystem::SetPetStatusFlag(int iFlag)
{
	m_Config.bPetStatus = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsNpcNameStatus()
{
	return m_Config.bNpcNameStatus;
}
void CPythonSystem::SetNpcNameStatusFlag(int iFlag)
{
	m_Config.bNpcNameStatus = iFlag == 1 ? true : false;
}
#endif

#ifdef ENABLE_FOV_OPTION
bool CPythonSystem::IsExtendedFOV()
{
	return m_Config.bExtendedFOV;
}
void CPythonSystem::SetExtendedFOV(int iFlag)
{
	m_Config.bExtendedFOV = iFlag == 1 ? true : false;
}
#endif

#ifdef ENABLE_PREMIUM_AFFECT_SYSTEM
bool CPythonSystem::IsEnablePremiumAffect()
{
	return m_Config.bShowPremiumAffect;
}

void CPythonSystem::SetPremiumAffect(int iFlag)
{
	m_Config.bShowPremiumAffect = iFlag == 1 ? true : false;
}
#endif

#ifdef ENABLE_DAMAGE_BAR
bool CPythonSystem::IsEnableDamageBar()
{
	return m_Config.bShowDamageBar;
}

void CPythonSystem::SetDamageBar(int iFlag)
{
	m_Config.bShowDamageBar = iFlag == 1 ? true : false;
}
#endif

#ifdef ENABLE_TIME_SYSTEM
bool CPythonSystem::IsEnableTimeSystem()
{
	return m_Config.bShowTimeSystem;
}

void CPythonSystem::SetTimeSystem(int iFlag)
{
	m_Config.bShowTimeSystem = iFlag == 1 ? true : false;
}
#endif

#ifdef ENABLE_INVENTORY_ADDITION
bool CPythonSystem::IsEnableInventoryAddition()
{
	return m_Config.bShowInventoryAddition;
}

void CPythonSystem::SetInventoryAddition(int iFlag)
{
	m_Config.bShowInventoryAddition = iFlag == 1 ? true : false;
}
#endif

#ifdef ENABLE_PACKET_INFO_SYSTEM
bool CPythonSystem::IsEnableInfoWindow()
{
	return m_Config.bShowInfoWindow;
}

void CPythonSystem::SetInfoWindow(int iFlag)
{
	m_Config.bShowInfoWindow = iFlag == 1 ? true : false;
}
#endif

bool CPythonSystem::IsAutoTiling()
{
	if (m_Config.bSoftwareTiling == 0)
		return true;

	return false;
}

#ifdef ENABLE_AUTO_SYSTEM
bool CPythonSystem::IsAutoHuntStone()
{
	return m_Config.bAutoHuntStone;
}

bool CPythonSystem::SetAutoHuntStoneFlag(int iFlag)
{
#ifdef ENABLE_MULTI_FARM_BLOCK
	if (iFlag == 1)
	{
		if (m_hAutoStoneMutex == NULL)
		{
			m_hAutoStoneMutex = CreateMutex(NULL, TRUE, "Global\\MT2_AutoStone_SingleClient");
			if (m_hAutoStoneMutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS)
			{
				if (m_hAutoStoneMutex)
				{
					CloseHandle(m_hAutoStoneMutex);
					m_hAutoStoneMutex = NULL;
				}
				return false;
			}
		}
	}
	else
	{
		if (m_hAutoStoneMutex)
		{
			ReleaseMutex(m_hAutoStoneMutex);
			CloseHandle(m_hAutoStoneMutex);
			m_hAutoStoneMutex = NULL;
		}
	}
#endif
	m_Config.bAutoHuntStone = iFlag == 1 ? true : false;
	return true;
}

bool CPythonSystem::IsAutoHuntPawn()
{
	return m_Config.bAutoHuntPawn;
}

void CPythonSystem::SetAutoHuntPawnFlag(int iFlag)
{
	m_Config.bAutoHuntPawn = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsAutoHuntSPawn()
{
	return m_Config.bAutoHuntSPawn;
}

void CPythonSystem::SetAutoHuntSPawnFlag(int iFlag)
{
	m_Config.bAutoHuntSPawn = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsAutoHuntKnight()
{
	return m_Config.bAutoHuntKnight;
}

void CPythonSystem::SetAutoHuntKnightFlag(int iFlag)
{
	m_Config.bAutoHuntKnight = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsAutoHuntSKnight()
{
	return m_Config.bAutoHuntSKnight;
}

void CPythonSystem::SetAutoHuntSKnightFlag(int iFlag)
{
	m_Config.bAutoHuntSKnight = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsAutoHuntBoss()
{
	return m_Config.bAutoHuntBoss;
}

void CPythonSystem::SetAutoHuntBossFlag(int iFlag)
{
	m_Config.bAutoHuntBoss = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsAutoHuntKing()
{
	return m_Config.bAutoHuntKing;
}

void CPythonSystem::SetAutoHuntKingFlag(int iFlag)
{
	m_Config.bAutoHuntKing = iFlag == 1 ? true : false;
}

int CPythonSystem::GetAutoHuntLevelMin()
{
	return m_Config.iAutoHuntLevelMin;
}

void CPythonSystem::SetAutoHuntLevelMin(int iLevel)
{
	m_Config.iAutoHuntLevelMin = iLevel;
}

int CPythonSystem::GetAutoHuntLevelMax()
{
	return m_Config.iAutoHuntLevelMax;
}

void CPythonSystem::SetAutoHuntLevelMax(int iLevel)
{
	m_Config.iAutoHuntLevelMax = iLevel;
}
#endif

#ifdef ENABLE_REFINE_RENEWAL
bool CPythonSystem::IsRefineStatusShow()
{
	return m_Config.bRefineStatus;
}

void CPythonSystem::SetRefineStatus(int iFlag)
{
	m_Config.bRefineStatus = iFlag == 1 ? true : false;
}
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
bool CPythonSystem::IsShowFlag()
{
	return m_Config.bShowFlag;
}

void CPythonSystem::SetShowFlag(int iFlag)
{
	m_Config.bShowFlag = iFlag == 1 ? true : false;
}
#endif

#ifdef ENABLE_SAVE_CAMERA_MODE
void CPythonSystem::SetCameraMode(BYTE bMode)
{
	m_Config.bCameraMode = bMode;
}

BYTE CPythonSystem::GetCameraMode() const
{
	return m_Config.bCameraMode;
}
#endif

#ifdef ENABLE_ANIMATION_OPTIMIZATION
void CPythonSystem::SetShowOtherCharAttacked(bool bEnable)
{
	m_Config.bShowOtherCharAttacked = bEnable;
}

bool CPythonSystem::IsShowOtherCharAttacked() const
{
	return m_Config.bShowOtherCharAttacked;
}
#endif

#ifdef ENABLE_FAST_ITEM_DELETE_SYSTEM
bool CPythonSystem::IsAutoSellStatus()
{
	return m_Config.bAutoSellStatus;
}

void CPythonSystem::SetAutoSellStatus(int iFlag)
{
	m_Config.bAutoSellStatus = iFlag == 1 ? true : false;
}
#endif

void CPythonSystem::SetSoftwareTiling(bool isEnable)
{
	if (isEnable)
		m_Config.bSoftwareTiling=1;
	else
		m_Config.bSoftwareTiling=2;
}

bool CPythonSystem::IsSoftwareTiling()
{
	if (m_Config.bSoftwareTiling==1)
		return true;

	return false;
}

bool CPythonSystem::IsUseDefaultIME()
{
	return m_Config.bUseDefaultIME;
}

bool CPythonSystem::LoadConfig()
{
	FILE * fp = NULL;

	std::string filePath = GetConfigFilePath("metin2.cfg", false);
	if (NULL == (fp = fopen(filePath.c_str(), "rt")))
		return false;

	char buf[256];
	char command[256];
	char value[256];

	while (fgets(buf, 256, fp))
	{
		if (sscanf(buf, " %s %s\n", command, value) == EOF)
			break;

		if (!stricmp(command, "WIDTH"))
#if !defined(__ANDROID__) && !defined(__APPLE__)
			m_Config.width		= atoi(value);
#else
			m_Config.width		= (g_nAndroidScreenWidth > 0) ? g_nAndroidScreenWidth : 1067;
#endif
		else if (!stricmp(command, "HEIGHT"))
#if !defined(__ANDROID__) && !defined(__APPLE__)
			m_Config.height	= atoi(value);
#else
			m_Config.height	= (g_nAndroidScreenHeight > 0) ? g_nAndroidScreenHeight : 600;
#endif

		else if (!stricmp(command, "BPP"))
			m_Config.bpp		= atoi(value);
		else if (!stricmp(command, "FREQUENCY"))
			m_Config.frequency = atoi(value);
		else if (!stricmp(command, "SOFTWARE_CURSOR"))
			m_Config.is_software_cursor = atoi(value) ? true : false;
		else if (!stricmp(command, "OBJECT_CULLING"))
			m_Config.is_object_culling = atoi(value) ? true : false;
		else if (!stricmp(command, "VISIBILITY"))
			m_Config.iDistance = atoi(value);
		else if (!stricmp(command, "MUSIC_VOLUME")) {
			if(strchr(value, '.') == 0) { // Old compatiability
				m_Config.music_volume = pow(10.0f, (-1.0f + (((float) atoi(value)) / 5.0f)));
				if(atoi(value) == 0)
					m_Config.music_volume = 0.0f;
			} else
				m_Config.music_volume = atof(value);
		} else if (!stricmp(command, "VOICE_VOLUME"))
			m_Config.voice_volume = (char) atoi(value);
		else if (!stricmp(command, "GAMMA"))
			m_Config.gamma = atoi(value);
		else if (!stricmp(command, "IS_SAVE_ID"))
			m_Config.isSaveID = atoi(value);
		else if (!stricmp(command, "SAVE_ID"))
			strncpy(m_Config.SaveID, value, 20);
		else if (!stricmp(command, "PRE_LOADING_DELAY_TIME"))
			g_iLoadingDelayTime = atoi(value);
		else if (!stricmp(command, "WINDOWED"))
		{
			m_Config.bWindowed = atoi(value) == 1 ? true : false;
		}
		else if (!stricmp(command, "USE_DEFAULT_IME"))
			m_Config.bUseDefaultIME = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SOFTWARE_TILING"))
			m_Config.bSoftwareTiling = atoi(value);
#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
		else if (!stricmp(command, "SHADOW_TARGET_LEVEL"))
			m_Config.iShadowTargetLevel = atoi(value);
		else if (!stricmp(command, "SHADOW_QUALITY_LEVEL"))
			m_Config.iShadowQualityLevel = atoi(value);
#else
		else if (!stricmp(command, "SHADOW_LEVEL"))
			m_Config.iShadowLevel = atoi(value);
#endif
		else if (!stricmp(command, "DECOMPRESSED_TEXTURE"))
			m_Config.bDecompressDDS = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "NO_SOUND_CARD"))
			m_Config.bNoSoundCard = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "VIEW_CHAT"))
			m_Config.bViewChat = atoi(value) == 1 ? true : false;
#ifdef ENABLE_TITLE_SYSTEM
		else if (!stricmp(command, "SHOW_TITLE"))
			m_Config.bShowTitle = atoi(value) == 1 ? true : false;
#endif
		else if (!stricmp(command, "ALWAYS_VIEW_NAME"))
			m_Config.bAlwaysShowName = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SHOW_DAMAGE"))
			m_Config.bShowDamage = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SHOW_SALESTEXT"))
			m_Config.bShowSalesText = atoi(value) == 1 ? true : false;
#ifdef ENABLE_TRANSLATOR_GOOGLE_SYSTEM
		else if (!stricmp(command, "TRANS_LANG_KEY"))
			strncpy(m_Config.TransLangKey, value, 5);
#endif
#ifdef ENABLE_AUTO_SYSTEM
				else if (!stricmp(command, "AUTO_HUNT_STONE"))
		{
			m_Config.bAutoHuntStone = atoi(value) == 1 ? true : false;
#ifdef ENABLE_MULTI_FARM_BLOCK
			if (m_Config.bAutoHuntStone)
			{
				if (!SetAutoHuntStoneFlag(1))
					m_Config.bAutoHuntStone = false;
			}
#endif
		}
		else if (!stricmp(command, "AUTO_HUNT_PAWN"))
			m_Config.bAutoHuntPawn = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "AUTO_HUNT_SPAWN"))
			m_Config.bAutoHuntSPawn = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "AUTO_HUNT_KNIGHT"))
			m_Config.bAutoHuntKnight = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "AUTO_HUNT_SKNIGHT"))
			m_Config.bAutoHuntSKnight = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "AUTO_HUNT_BOSS"))
			m_Config.bAutoHuntBoss = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "AUTO_HUNT_KING"))
			m_Config.bAutoHuntKing = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "AUTO_HUNT_LEVEL_MIN"))
			m_Config.iAutoHuntLevelMin = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "AUTO_HUNT_LEVEL_MAX"))
			m_Config.iAutoHuntLevelMax = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_REFINE_RENEWAL
		else if (!stricmp(command, "SHOW_REFINE_DIALOG"))
			m_Config.bRefineStatus = atoi(value) == 1 ? true : false;
#endif
#if defined(ENABLE_SHOW_MOB_INFO)
		else if (!stricmp(command, "SHOW_MOBLEVEL"))
			m_Config.bShowMobLevel = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SHOW_MOBAIFLAG"))
			m_Config.bShowMobAIFlag = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_AUTO_PICKUP_SYSTEM
		else if (!stricmp(command, "AUTO_PICKUP"))
			m_Config.bAutoPickup = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_FOV_OPTION
		else if (!stricmp(command, "EXTENDED_FOV"))
			m_Config.bExtendedFOV = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_FOG_FIX
		else if (!stricmp(command, "FOG_MODE_ON"))
			m_Config.bShowFogMode = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_PREMIUM_AFFECT_SYSTEM
		else if (!stricmp(command, "PREMIUM_AFFECT"))
			m_Config.bShowPremiumAffect = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_DAMAGE_BAR
		else if (!stricmp(command, "DAMAGE_BAR"))
			m_Config.bShowDamageBar = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_TIME_SYSTEM
		else if (!stricmp(command, "TIME_SYSTEM"))
			m_Config.bShowTimeSystem = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_INVENTORY_ADDITION
		else if (!stricmp(command, "INVENTORY_ADDITION"))
			m_Config.bShowInventoryAddition = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_PACKET_INFO_SYSTEM
		else if (!stricmp(command, "INFO_WINDOW"))
			m_Config.bShowInfoWindow = atoi(value) == 1 ? true : false;
#endif
#if defined(ENABLE_ENVIRONMENT_EFFECT_OPTION)
		else if (!stricmp(command, "NIGHT_MODE_ON"))
			m_Config.bShowNightMode = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SNOW_MODE_ON"))
			m_Config.bShowSnowMode = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SNOW_TEXTURE_MODE"))
			m_Config.bShowSnowTextureMode = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_DICE_SYSTEM
		else if (!stricmp(command, "DICE"))
			m_Config.bDiceFlag = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_GRAPHIC_ON_OFF
		else if (!stricmp(command, "EFFECT_LEVEL"))
			m_Config.iEffectLevel = atoi(value);
		// else if (!stricmp(command, "PRIVATE_SHOP_LEVEL"))
			// m_Config.iPrivateShopLevel = atoi(value);
		else if (!stricmp(command, "DROP_ITEM_LEVEL"))
			m_Config.iDropItemLevel = atoi(value);

		else if (!stricmp(command, "PET_STATUS"))
			m_Config.bPetStatus = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "NPC_NAME_STATUS"))
			m_Config.bNpcNameStatus = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		else if (!stricmp(command, "SHOW_FLAG"))
			m_Config.bShowFlag = atoi(value) == 1 ? true : false;
#endif
#ifdef ENABLE_SAVE_CAMERA_MODE
		else if (!stricmp(command, "CAMERA_MODE"))
			m_Config.bCameraMode = atoi(value);
#endif
#ifdef ENABLE_ANIMATION_OPTIMIZATION
		else if (!stricmp(command, "SHOW_OTHER_CHAR_ATTACKED"))
			m_Config.bShowOtherCharAttacked = atoi(value) == 1;
#endif
#ifdef ENABLE_FAST_ITEM_DELETE_SYSTEM
		else if (!stricmp(command, "FAST_ITEM_SELL"))
			m_Config.bAutoSellStatus = atoi(value) == 1;
#endif
#ifdef ENABLE_GPU_CONFIG
		else if (!stricmp(command, "GPU_DESCRIPTION"))
		{
			const std::string sBuf = buf;
			m_Config.sGPU = sBuf.substr(strlen(command) + 1);
			if (!m_Config.sGPU.empty() && m_Config.sGPU.back() == '\n')
				m_Config.sGPU.pop_back();
		}
#endif
	}

#if !defined(__ANDROID__) && !defined(__APPLE__)
	if (m_Config.bWindowed)

	{
		unsigned screen_width = GetSystemMetrics(SM_CXFULLSCREEN);
		unsigned screen_height = GetSystemMetrics(SM_CYFULLSCREEN);

		if (m_Config.width >= screen_width)
		{
			m_Config.width = screen_width;
		}
		if (m_Config.height >= screen_height)
		{
			int config_height = m_Config.height;
			int difference = (config_height-screen_height)+7;
			m_Config.height = config_height - difference;
		}
	}
#endif
	

	m_OldConfig = m_Config;

	fclose(fp);

//	Tracef("LoadConfig: Resolution: %dx%d %dBPP %dHZ Software Cursor: %d, Music/Voice Volume: %d/%d Gamma: %d\n",
//		m_Config.width,
//		m_Config.height,
//		m_Config.bpp,
//		m_Config.frequency,
//		m_Config.is_software_cursor,
//		m_Config.music_volume,
//		m_Config.voice_volume,
//		m_Config.gamma);

	return true;
}

bool CPythonSystem::SaveConfig()
{
	FILE *fp;

	std::string filePath = GetConfigFilePath("metin2.cfg", true);
	if (NULL == (fp = fopen(filePath.c_str(), "wt")))
	{
#ifdef __ANDROID__
		LOGE("SaveConfig: Failed to open %s for write!", filePath.c_str());
#endif
		return false;
	}

#ifdef __ANDROID__
	int saveWidth = (g_nAndroidScreenWidth > 0) ? g_nAndroidScreenWidth : m_Config.width;
	int saveHeight = (g_nAndroidScreenHeight > 0) ? g_nAndroidScreenHeight : m_Config.height;
#else
	int saveWidth = m_Config.width;
	int saveHeight = m_Config.height;
#endif

	fprintf(fp, "WIDTH						%d\n"
				"HEIGHT						%d\n"
				"BPP						%d\n"
				"FREQUENCY					%d\n"
				"SOFTWARE_CURSOR			%d\n"
				"OBJECT_CULLING				%d\n"
				"VISIBILITY					%d\n"
				"MUSIC_VOLUME				%.3f\n"
				"VOICE_VOLUME				%d\n"
				"GAMMA						%d\n"
				"IS_SAVE_ID					%d\n"
				"SAVE_ID					%s\n"
#ifdef ENABLE_TRANSLATOR_GOOGLE_SYSTEM
				"TRANS_LANG_KEY				%s\n"
#endif
				"PRE_LOADING_DELAY_TIME		%d\n"
				"DECOMPRESSED_TEXTURE		%d\n",
				saveWidth,
				saveHeight,
				m_Config.bpp,
				m_Config.frequency,
				m_Config.is_software_cursor,
				m_Config.is_object_culling,
				m_Config.iDistance,
				m_Config.music_volume,
				m_Config.voice_volume,
				m_Config.gamma,
				m_Config.isSaveID,
				m_Config.SaveID,
#ifdef ENABLE_TRANSLATOR_GOOGLE_SYSTEM
				m_Config.TransLangKey,
#endif
				g_iLoadingDelayTime,
				m_Config.bDecompressDDS);

	if (m_Config.bWindowed == 1)
		fprintf(fp, "WINDOWED				%d\n", m_Config.bWindowed);
	if (m_Config.bViewChat == 0)
		fprintf(fp, "VIEW_CHAT				%d\n", m_Config.bViewChat);
#ifdef ENABLE_TITLE_SYSTEM
	if (m_Config.bShowTitle == 0)
		fprintf(fp, "SHOW_TITLE				%d\n", m_Config.bShowTitle);
#endif
	if (m_Config.bAlwaysShowName != DEFAULT_VALUE_ALWAYS_SHOW_NAME)
		fprintf(fp, "ALWAYS_VIEW_NAME		%d\n", m_Config.bAlwaysShowName);
	if (m_Config.bShowDamage == 0)
		fprintf(fp, "SHOW_DAMAGE			%d\n", m_Config.bShowDamage);
	if (m_Config.bShowSalesText == 0)
		fprintf(fp, "SHOW_SALESTEXT			%d\n", m_Config.bShowSalesText);
#ifdef ENABLE_REFINE_RENEWAL
	fprintf(fp, "SHOW_REFINE_DIALOG			%d\n", m_Config.bRefineStatus);
#endif
#ifdef ENABLE_FOG_FIX
	fprintf(fp, "FOG_MODE_ON				%d\n", m_Config.bShowFogMode);
#endif
#if defined(ENABLE_ENVIRONMENT_EFFECT_OPTION)
	fprintf(fp, "NIGHT_MODE_ON				%d\n", m_Config.bShowNightMode);
	fprintf(fp, "SNOW_MODE_ON				%d\n", m_Config.bShowSnowMode);
	fprintf(fp, "SNOW_TEXTURE_MODE			%d\n", m_Config.bShowSnowTextureMode);
#endif
#if defined(ENABLE_SHOW_MOB_INFO)
	fprintf(fp, "SHOW_MOBLEVEL				%d\n", m_Config.bShowMobLevel);
	fprintf(fp, "SHOW_MOBAIFLAG				%d\n", m_Config.bShowMobAIFlag);
#endif
#ifdef ENABLE_AUTO_PICKUP_SYSTEM
	fprintf(fp, "AUTO_PICKUP				%d\n", m_Config.bAutoPickup);
#endif
#ifdef ENABLE_FOV_OPTION
	fprintf(fp, "EXTENDED_FOV				%d\n", m_Config.bExtendedFOV);
#endif
#ifdef ENABLE_GRAPHIC_ON_OFF
	fprintf(fp, "EFFECT_LEVEL				%d\n", m_Config.iEffectLevel);
	// fprintf(fp, "PRIVATE_SHOP_LEVEL			%d\n", m_Config.iPrivateShopLevel);
	fprintf(fp, "DROP_ITEM_LEVEL			%d\n", m_Config.iDropItemLevel);
	fprintf(fp, "PET_STATUS					%d\n", m_Config.bPetStatus);
	fprintf(fp, "NPC_NAME_STATUS			%d\n", m_Config.bNpcNameStatus);
#endif
#ifdef ENABLE_PREMIUM_AFFECT_SYSTEM
	fprintf(fp, "PREMIUM_AFFECT				%d\n", m_Config.bShowPremiumAffect);
#endif
#ifdef ENABLE_DAMAGE_BAR
	fprintf(fp, "DAMAGE_BAR					%d\n", m_Config.bShowDamageBar);
#endif
#ifdef ENABLE_TIME_SYSTEM
	fprintf(fp, "TIME_SYSTEM				%d\n", m_Config.bShowTimeSystem);
#endif
#ifdef ENABLE_INVENTORY_ADDITION
	fprintf(fp, "INVENTORY_ADDITION			%d\n", m_Config.bShowInventoryAddition);
#endif
#ifdef ENABLE_PACKET_INFO_SYSTEM
	fprintf(fp, "INFO_WINDOW			%d\n", m_Config.bShowInfoWindow);
#endif
#ifdef ENABLE_AUTO_SYSTEM
	fprintf(fp, "AUTO_HUNT_STONE			%d\n", m_Config.bAutoHuntStone);
	fprintf(fp, "AUTO_HUNT_PAWN			%d\n", m_Config.bAutoHuntPawn);
	fprintf(fp, "AUTO_HUNT_SPAWN			%d\n", m_Config.bAutoHuntSPawn);
	fprintf(fp, "AUTO_HUNT_KNIGHT			%d\n", m_Config.bAutoHuntKnight);
	fprintf(fp, "AUTO_HUNT_SKNIGHT			%d\n", m_Config.bAutoHuntSKnight);
	fprintf(fp, "AUTO_HUNT_BOSS			%d\n", m_Config.bAutoHuntBoss);
	fprintf(fp, "AUTO_HUNT_KING			%d\n", m_Config.bAutoHuntKing);
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	fprintf(fp, "SHOW_FLAG					%d\n", m_Config.bShowFlag);
#endif
	fprintf(fp, "USE_DEFAULT_IME			%d\n", m_Config.bUseDefaultIME);
	fprintf(fp, "SOFTWARE_TILING			%d\n", m_Config.bSoftwareTiling);
#ifdef ENABLE_SHADOW_RENDER_QUALITY_OPTION
	fprintf(fp, "SHADOW_TARGET_LEVEL		%d\n", m_Config.iShadowTargetLevel);
	fprintf(fp, "SHADOW_QUALITY_LEVEL		%d\n", m_Config.iShadowQualityLevel);
#else
	fprintf(fp, "SHADOW_LEVEL				%d\n", m_Config.iShadowLevel);
#endif
#ifdef ENABLE_DICE_SYSTEM
	fprintf(fp, "DICE						%d\n", m_Config.bDiceFlag);
#endif
#ifdef ENABLE_SAVE_CAMERA_MODE
	fprintf(fp, "CAMERA_MODE\t\t\t%d\n",m_Config.bCameraMode);
#endif
#ifdef ENABLE_ANIMATION_OPTIMIZATION
	fprintf(fp, "SHOW_OTHER_CHAR_ATTACKED\t%d\n", m_Config.bShowOtherCharAttacked);
#endif
#ifdef ENABLE_FAST_ITEM_DELETE_SYSTEM
	fprintf(fp, "FAST_ITEM_SELL\t%d\n", m_Config.bAutoSellStatus);
#endif
#ifdef ENABLE_GPU_CONFIG
	fprintf(fp, "GPU_DESCRIPTION %s\n", m_Config.sGPU.c_str());
#endif
	fprintf(fp, "\n");

	fclose(fp);
	return true;
}

bool CPythonSystem::LoadInterfaceStatus()
{
	FILE * File;
std::string filePath = GetConfigFilePath("interface.cfg", false);
	File = fopen(filePath.c_str(), "rb");

	if (!File)
		return false;

	fread(m_WindowStatus, 1, sizeof(TWindowStatus) * WINDOW_MAX_NUM, File);
	fclose(File);
	return true;
}

void CPythonSystem::SaveInterfaceStatus()
{
	if (!m_poInterfaceHandler)
		return;

	PyCallClassMemberFunc(m_poInterfaceHandler, "OnSaveInterfaceStatus", Py_BuildValue("()"));

	FILE * File;

std::string filePath = GetConfigFilePath("interface.cfg", true);
	File = fopen(filePath.c_str(), "wb");

	if (!File)
	{
		TraceError("Cannot open interface.cfg");
		return;
	}

	fwrite(m_WindowStatus, 1, sizeof(TWindowStatus) * WINDOW_MAX_NUM, File);
	fclose(File);
}

bool CPythonSystem::isInterfaceConfig()
{
	return m_isInterfaceConfig;
}

const CPythonSystem::TWindowStatus & CPythonSystem::GetWindowStatusReference(int iIndex)
{
	return m_WindowStatus[iIndex];
}

void CPythonSystem::ApplyConfig() // ???? ????? ???? ?????? ?????? ?? ?????? ???? ????.
{
	if (m_OldConfig.gamma != m_Config.gamma)
	{
		float val = 1.0f;
		
		switch (m_Config.gamma)
		{
			case 0: val = 0.4f;	break;
			case 1: val = 0.7f; break;
			case 2: val = 1.0f; break;
			case 3: val = 1.2f; break;
			case 4: val = 1.4f; break;
		}
		
		CPythonGraphic::Instance().SetGamma(val);
	}

	if (m_OldConfig.is_software_cursor != m_Config.is_software_cursor)
	{
		if (m_Config.is_software_cursor)
			CPythonApplication::Instance().SetCursorMode(CPythonApplication::CURSOR_MODE_SOFTWARE);
		else
			CPythonApplication::Instance().SetCursorMode(CPythonApplication::CURSOR_MODE_HARDWARE);
	}


	m_OldConfig = m_Config;

	ChangeSystem();
}

void CPythonSystem::ChangeSystem()
{
	// Shadow
	/*
	if (m_Config.is_shadow)
		CScreen::SetShadowFlag(true);
	else
		CScreen::SetShadowFlag(false);
	*/
	CSoundManager& rkSndMgr = CSoundManager::Instance();
	/*
	float fMusicVolume;
	if (0 == m_Config.music_volume)
		fMusicVolume = 0.0f;
	else
		fMusicVolume= (float)pow(10.0f, (-1.0f + (float)m_Config.music_volume / 5.0f));
		*/
	rkSndMgr.SetMusicVolume(m_Config.music_volume);

	/*
	float fVoiceVolume;
	if (0 == m_Config.voice_volume)
		fVoiceVolume = 0.0f;
	else
		fVoiceVolume = (float)pow(10.0f, (-1.0f + (float)m_Config.voice_volume / 5.0f));
	*/
	rkSndMgr.SetSoundVolumeGrade(m_Config.voice_volume);	
}

void CPythonSystem::Clear()
{
	SetInterfaceHandler(NULL);
}

CPythonSystem::CPythonSystem()
{
	memset(&m_Config, 0, sizeof(TConfig));

	m_poInterfaceHandler = NULL;

#ifdef ENABLE_MULTI_FARM_BLOCK
	m_hAutoStoneMutex = NULL;
#endif

	SetDefaultConfig();

	LoadConfig();

	ChangeSystem();

	if (LoadInterfaceStatus())
		m_isInterfaceConfig = true;
	else
		m_isInterfaceConfig = false;
}

CPythonSystem::~CPythonSystem()
{
	assert(m_poInterfaceHandler==NULL && "CPythonSystem MUST CLEAR!");
#ifdef ENABLE_MULTI_FARM_BLOCK
	if (m_hAutoStoneMutex)
	{
		ReleaseMutex(m_hAutoStoneMutex);
		CloseHandle(m_hAutoStoneMutex);
		m_hAutoStoneMutex = NULL;
	}
#endif
}

#ifdef ENABLE_GPU_CONFIG
const std::string& CPythonSystem::GetGPU() const
{
	return m_Config.sGPU;
}

void CPythonSystem::SetGPU(const std::string& gpu)
{
	m_Config.sGPU = gpu;
}
#endif

#ifdef ENABLE_TITLE_SYSTEM
bool CPythonSystem::IsShowTitle()
{
	return m_Config.bShowTitle;
}

void CPythonSystem::SetShowTitle(bool bFlag)
{
	m_Config.bShowTitle = bFlag;
#ifdef ENABLE_TITLE_SYSTEM
	CPythonTitleManager::Instance().RefreshAllVisibleTitles();
#endif
}
#endif
