#include "StdAfx.h"
#ifdef ENABLE_TITLE_SYSTEM
#include "PythonTitleManager.h"
#include "PythonApplication.h"
#include "PythonSystem.h"
#include "../EterLib/Camera.h"
#include "../EterLib/CRenderTargetManager.h"
#include "PythonTextTail.h"
#include "PythonCharacterManager.h"
#include "PythonNetworkStream.h"
#include "InstanceBase.h"
#include "PythonPlayer.h"
#include "../EterPack/EterPackManager.h"

const char* CPythonTitleManager::TITLE_DATA_PATH = "d:/ymir work/ui/game/title/";
const char* CPythonTitleManager::TITLE_PREVIEW_IMAGE = "d:/ymir work/ui/titlesystem_001.dds";

namespace
{
	const char* GetFallbackEffectPath(int iEffectIndex)
	{
		static const char* s_effectPaths[] = {
			"d:/ymir work/effect/etc/title/title_01_shield.mse",
			"d:/ymir work/effect/etc/title/title_02_dragon.mse",
			"d:/ymir work/effect/etc/title/title_03_fist.mse",
			"d:/ymir work/effect/etc/title/title_04_trophy.mse",
			"d:/ymir work/effect/etc/title/title_05_medal.mse",
			"d:/ymir work/effect/etc/title/title_06_banner_gold.mse",
			"d:/ymir work/effect/etc/title/title_07_banner_red.mse",
			"d:/ymir work/effect/etc/title/title_08_banner_blue.mse",
		};

		if (iEffectIndex > 0 && iEffectIndex <= _countof(s_effectPaths))
			return s_effectPaths[iEffectIndex - 1];

		return "";
	}
}

std::map<int, CPythonTitleManager::TNameplatePaths> CPythonTitleManager::ms_NameplateByColor;

CPythonTitleManager::CPythonTitleManager() : m_iEquippedTitleIndex(-1), m_bDataReceived(false), m_pEffectInstance(nullptr), m_pBackgroundImage(nullptr), m_bShowPreview(false), m_iCurrentPreviewTitleIndex(-1)
{
	if (ms_NameplateByColor.empty())
	{
		TNameplatePaths goldPaths;
		goldPaths.strLeft = std::string(TITLE_DATA_PATH) + "title_06_gold_left.tga";
		goldPaths.strMiddle = std::string(TITLE_DATA_PATH) + "title_06_gold_middle.tga";
		goldPaths.strRight = std::string(TITLE_DATA_PATH) + "title_06_gold_right.tga";
		ms_NameplateByColor[COLOR_FAMILY_GOLD] = goldPaths;

		TNameplatePaths redPaths;
		redPaths.strLeft = std::string(TITLE_DATA_PATH) + "title_07_red_left.tga";
		redPaths.strMiddle = std::string(TITLE_DATA_PATH) + "title_07_red_middle.tga";
		redPaths.strRight = std::string(TITLE_DATA_PATH) + "title_07_red_right.tga";
		ms_NameplateByColor[COLOR_FAMILY_RED] = redPaths;

		TNameplatePaths bluePaths;
		bluePaths.strLeft = std::string(TITLE_DATA_PATH) + "title_08_blue_left.tga";
		bluePaths.strMiddle = std::string(TITLE_DATA_PATH) + "title_08_blue_middle.tga";
		bluePaths.strRight = std::string(TITLE_DATA_PATH) + "title_08_blue_right.tga";
		ms_NameplateByColor[COLOR_FAMILY_BLUE] = bluePaths;
	}

	static const struct { int resourceIndex; int colorFamily; } s_effectColors[] = {
		{ 3000, COLOR_FAMILY_GOLD }, { 3001, COLOR_FAMILY_GOLD },
		{ 3002, COLOR_FAMILY_RED },  { 3003, COLOR_FAMILY_GOLD },
		{ 3004, COLOR_FAMILY_GOLD },
	};
	for (int i = 0; i < _countof(s_effectColors); ++i)
		m_EffectResourceColorMap[s_effectColors[i].resourceIndex] = s_effectColors[i].colorFamily;
}

CPythonTitleManager::~CPythonTitleManager()
{
}

void CPythonTitleManager::Clear()
{
	for (auto it = m_ActiveTitleByVID.begin(); it != m_ActiveTitleByVID.end(); ++it)
	{
		CPythonTextTail::Instance().ClearTitleSystemData(it->first);
	}

	m_PlayerTitleMap.clear();
	m_iEquippedTitleIndex = -1;
	m_bDataReceived = false;
}

void CPythonTitleManager::Destroy()
{
	DestroyEffectPreview();
	Clear();

	if (m_pBackgroundImage)
	{
		delete m_pBackgroundImage;
		m_pBackgroundImage = nullptr;
	}
}

void CPythonTitleManager::Initialize()
{
	static const struct { int resourceIndex; int effectIndex; } s_effectOverrides[] = {
		{ 3000, 1 }, { 3001, 2 }, { 3002, 3 }, { 3003, 4 }, { 3004, 5 },
	};
	for (int i = 0; i < _countof(s_effectOverrides); ++i)
	{
		auto it = m_ResourceMap.find(s_effectOverrides[i].resourceIndex);
		if (it != m_ResourceMap.end())
			it->second.iEffectIndex = s_effectOverrides[i].effectIndex;
	}

	const char* localePath = LocaleService_GetLocalePath();
	std::string strTitleDataFile = std::string(localePath) + "/title_gf.txt";

	bool bLoaded = LoadTitleData(strTitleDataFile.c_str(), false);
	if (!bLoaded)
	{
		std::string strFallback = "locale/" + std::string(localePath) + "/title_gf.txt";
		bLoaded = LoadTitleData(strFallback.c_str(), false);
	}
	if (!bLoaded)
	{
		LoadTitleData("title_gf.txt", false);
	}

	std::string strWzFile = std::string(localePath) + "/title_wz.txt";
	bool bLoadedWz = LoadTitleData(strWzFile.c_str(), true);
	if (!bLoadedWz)
	{
		std::string strWzFallback = "locale/" + std::string(localePath) + "/title_wz.txt";
		bLoadedWz = LoadTitleData(strWzFallback.c_str(), true);
	}
	if (!bLoadedWz)
		LoadTitleData("title_wz.txt", true);

	BuildColorFamilies();

	m_bDataReceived = true;

	Tracef("CPythonTitleManager::Initialize: Loaded %d titles, %d resources\n", m_TitleDataMap.size(), m_ResourceMap.size());
}

bool CPythonTitleManager::LoadTitleData(const char* c_szFilename, bool bIsWZ)
{
	const VOID* pvData;
	CMappedFile kFile;

	if (!CEterPackManager::Instance().Get(kFile, c_szFilename, &pvData))
	{
		TraceError("CPythonTitleManager::LoadTitleData: Failed to open %s", c_szFilename);
		return false;
	}

	CMemoryTextFileLoader kTextFileLoader;
	kTextFileLoader.Bind(kFile.Size(), pvData);

	CTokenVector kTokenVector;
	for (DWORD i = 0; i < kTextFileLoader.GetLineCount(); ++i)
	{
		const std::string& c_rstLine = kTextFileLoader.GetLineString(i);
		if (c_rstLine.empty() || c_rstLine[0] == '#')
			continue;

		if (!kTextFileLoader.SplitLineByTab(i, &kTokenVector))
			continue;

		if (kTokenVector.size() < 8)
			continue;

		TTitleData titleData;
		if (bIsWZ)
		{
			titleData.iTitleIndex = atoi(kTokenVector[1].c_str());
			titleData.iResourceIndex = atoi(kTokenVector[1].c_str());
			titleData.iTitleType = atoi(kTokenVector[2].c_str());
		}
		else
		{
			titleData.iTitleIndex = atoi(kTokenVector[0].c_str());
			titleData.iResourceIndex = atoi(kTokenVector[1].c_str());
			titleData.iTitleType = atoi(kTokenVector[2].c_str());
		}
		titleData.strName = kTokenVector[3];
		titleData.strConditionTooltip = kTokenVector[4];
		titleData.bIsPermanent = true;
		titleData.strOpenTime = kTokenVector.size() > 6 ? kTokenVector[6] : "";
		titleData.dwFontColor = kTokenVector.size() > 7 ? strtoul(kTokenVector[7].c_str(), NULL, 10) : 0xFFFFFFFF;
		titleData.bIsWZ = bIsWZ;

		m_TitleDataMap[titleData.iTitleIndex] = titleData;
	}

	return true;
}

bool CPythonTitleManager::LoadTitleResourceList(const char* c_szFilename)
{
	const VOID* pvData;
	CMappedFile kFile;

	if (!CEterPackManager::Instance().Get(kFile, c_szFilename, &pvData))
	{
		TraceError("CPythonTitleManager::LoadTitleResourceList: Failed to open %s", c_szFilename);
		return false;
	}

	CMemoryTextFileLoader kTextFileLoader;
	kTextFileLoader.Bind(kFile.Size(), pvData);

	CTokenVector kTokenVector;
	for (DWORD i = 0; i < kTextFileLoader.GetLineCount(); ++i)
	{
		const std::string& c_rstLine = kTextFileLoader.GetLineString(i);
		if (c_rstLine.empty() || c_rstLine[0] == '#')
			continue;

		if (!kTextFileLoader.SplitLineByTab(i, &kTokenVector))
			continue;

		if (kTokenVector.size() < 3)
			continue;

		TResourceData resData;
		int iResourceIndex = atoi(kTokenVector[0].c_str());
		resData.strSubFilename = kTokenVector[1];
		resData.iEffectIndex = atoi(kTokenVector[2].c_str());

		m_ResourceMap[iResourceIndex] = resData;
	}

	return true;
}

CPythonTitleManager::TSubFileData& CPythonTitleManager::ParseSubFile(const std::string& strSubFilename)
{
	auto it = m_SubDataCache.find(strSubFilename);
	if (it != m_SubDataCache.end())
		return it->second;

	TSubFileData& subData = m_SubDataCache[strSubFilename];

	std::string strFullPath = "d:/ymir work/ui/game/title/titles/" + strSubFilename;

	const VOID* pvData;
	CMappedFile kFile;

	if (!CEterPackManager::Instance().Get(kFile, strFullPath.c_str(), &pvData))
	{
		TraceError("CPythonTitleManager::ParseSubFile: Failed to open %s", strFullPath.c_str());
		return subData;
	}

	CMemoryTextFileLoader kTextFileLoader;
	kTextFileLoader.Bind(kFile.Size(), pvData);

	CTokenVector kTokenVector;
	for (DWORD i = 0; i < kTextFileLoader.GetLineCount(); ++i)
	{
		const std::string& c_rstLine = kTextFileLoader.GetLineString(i);
		if (c_rstLine.empty() || c_rstLine[0] == '#')
			continue;

		if (!kTextFileLoader.SplitLineByTab(i, &kTokenVector))
			continue;

		if (kTokenVector.size() >= 2)
			subData[kTokenVector[0]] = kTokenVector[1];
		else if (kTokenVector.size() == 1)
			subData[kTokenVector[0]] = "";
	}

	return subData;
}

int CPythonTitleManager::DetectColorFamily(const TSubFileData& subData, int iResourceIndex) const
{
	auto itType = subData.find("type");
	std::string strType = (itType != subData.end()) ? itType->second : "";

	if (strType == "NAMEPLATE")
	{
		auto itLeft = subData.find("left_image");
		if (itLeft != subData.end())
		{
			const std::string& strLeftImg = itLeft->second;
			if (strLeftImg.find("gold") != std::string::npos)
				return COLOR_FAMILY_GOLD;
			else if (strLeftImg.find("red") != std::string::npos)
				return COLOR_FAMILY_RED;
			else if (strLeftImg.find("blue") != std::string::npos)
				return COLOR_FAMILY_BLUE;
		}
		return COLOR_FAMILY_GOLD; // fallback
	}
	else if (strType == "EFFECT")
	{
		auto itColor = m_EffectResourceColorMap.find(iResourceIndex);
		if (itColor != m_EffectResourceColorMap.end())
			return itColor->second;
		return COLOR_FAMILY_GOLD; // default
	}

	return COLOR_FAMILY_NONE;
}

void CPythonTitleManager::BuildColorFamilies()
{
	m_ColorFamilyMap.clear();

	for (auto it = m_ResourceMap.begin(); it != m_ResourceMap.end(); ++it)
	{
		int iResourceIndex = it->first;
		const TSubFileData& subData = ParseSubFile(it->second.strSubFilename);
		int iColorFamily = DetectColorFamily(subData, iResourceIndex);
		m_ColorFamilyMap[iResourceIndex] = iColorFamily;
	}

	Tracef("CPythonTitleManager::BuildColorFamilies: Mapped %d resources to color families\n",
		m_ColorFamilyMap.size());
}

bool CPythonTitleManager::GetTitleData(int iTitleIndex, TTitleData* pData) const
{
	auto it = m_TitleDataMap.find(iTitleIndex);
	if (it == m_TitleDataMap.end())
		return false;
	*pData = it->second;
	return true;
}

int CPythonTitleManager::GetColorFamily(int iTitleIndex) const
{
	auto itTitle = m_TitleDataMap.find(iTitleIndex);
	if (itTitle == m_TitleDataMap.end())
		return COLOR_FAMILY_NONE;

	int iResourceIndex = itTitle->second.iResourceIndex;
	auto itColor = m_ColorFamilyMap.find(iResourceIndex);
	if (itColor != m_ColorFamilyMap.end())
		return itColor->second;

	return COLOR_FAMILY_NONE;
}

bool CPythonTitleManager::GetNameplatePaths(int iColorFamily, TNameplatePaths* pPaths) const
{
	auto it = ms_NameplateByColor.find(iColorFamily);
	if (it == ms_NameplateByColor.end())
		return false;
	*pPaths = it->second;
	return true;
}

bool CPythonTitleManager::GetPreviewSpriteData(int iTitleIndex, TSpritePreviewData* pData)
{
	auto itTitle = m_TitleDataMap.find(iTitleIndex);
	if (itTitle == m_TitleDataMap.end())
		return false;

	int iResourceIndex = itTitle->second.iResourceIndex;

	auto itRes = m_ResourceMap.find(iResourceIndex);
	if (itRes == m_ResourceMap.end())
		return false;

	const TSubFileData& subData = ParseSubFile(itRes->second.strSubFilename);

	auto itType = subData.find("type");
	if (itType == subData.end() || itType->second != "NAMEPLATE")
		return false;

	auto itSprites = subData.find("sprites");
	auto itCount = subData.find("sprites_count");

	if (itSprites == subData.end() || itCount == subData.end())
		return false;

	int iCount = atoi(itCount->second.c_str());
	if (iCount <= 0 || itSprites->second.empty())
		return false;

	auto itSizeX = subData.find("sprite_size_x");
	auto itSizeY = subData.find("sprite_size_y");
	auto itCols = subData.find("sprite_columns");

	pData->strDirPath = "d:/ymir work/" + itSprites->second;
	pData->iFrameCount = iCount;
	pData->iSizeX = (itSizeX != subData.end()) ? atoi(itSizeX->second.c_str()) : 0;
	pData->iSizeY = (itSizeY != subData.end()) ? atoi(itSizeY->second.c_str()) : 0;
	pData->iColumns = (itCols != subData.end()) ? atoi(itCols->second.c_str()) : 6;

	return true;
}

bool CPythonTitleManager::GetPreviewEffectData(int iTitleIndex, TEffectPreviewData* pData)
{
	auto itTitle = m_TitleDataMap.find(iTitleIndex);
	if (itTitle == m_TitleDataMap.end())
		return false;

	int iResourceIndex = itTitle->second.iResourceIndex;

	auto itRes = m_ResourceMap.find(iResourceIndex);
	if (itRes == m_ResourceMap.end())
		return false;

	const TSubFileData& subData = ParseSubFile(itRes->second.strSubFilename);
	auto itType = subData.find("type");

	if (itType != subData.end() && itType->second == "EFFECT")
	{
		auto itEffect = subData.find("effect");
		if (itEffect != subData.end() && !itEffect->second.empty())
		{
			pData->strEffectPath = "d:/ymir work/" + itEffect->second;

			auto itColor = m_EffectResourceColorMap.find(iResourceIndex);
			if (itColor != m_EffectResourceColorMap.end())
				pData->iColorFamily = itColor->second;
			else
				pData->iColorFamily = COLOR_FAMILY_GOLD;

			return true;
		}
	}

	if (itType != subData.end() && itType->second == "IMAGE")
		return false;

	int iEffectIndex = itRes->second.iEffectIndex;
	const char* szFallbackPath = GetFallbackEffectPath(iEffectIndex);
	if (szFallbackPath && szFallbackPath[0] != '\0')
	{
		pData->strEffectPath = szFallbackPath;
		pData->iColorFamily = COLOR_FAMILY_GOLD;
		return true;
	}

	return false;
}

bool CPythonTitleManager::GetPreviewImageData(int iTitleIndex, TImagePreviewData* pData)
{
	auto itTitle = m_TitleDataMap.find(iTitleIndex);
	if (itTitle == m_TitleDataMap.end())
		return false;

	int iResourceIndex = itTitle->second.iResourceIndex;

	auto itRes = m_ResourceMap.find(iResourceIndex);
	if (itRes == m_ResourceMap.end())
		return false;

	const TSubFileData& subData = ParseSubFile(itRes->second.strSubFilename);

	auto itImage = subData.find("image");
	if (itImage == subData.end() || itImage->second.empty())
		return false;

	pData->strImagePath = "d:/ymir work/" + itImage->second;
	return true;
}

const char* CPythonTitleManager::GetItemTooltip(int iTitleIndex) const
{
	auto it = m_TitleDataMap.find(iTitleIndex);
	if (it != m_TitleDataMap.end())
		return it->second.strConditionTooltip.c_str();
	return "";
}

void CPythonTitleManager::SetPlayerTitleData(int iTitleIndex, int iEndTime, bool bIsEquip, bool bIsObtain)
{
	if (bIsEquip && m_iEquippedTitleIndex >= 0 && m_iEquippedTitleIndex != iTitleIndex)
	{
		auto itOld = m_PlayerTitleMap.find(m_iEquippedTitleIndex);
		if (itOld != m_PlayerTitleMap.end())
			itOld->second.bIsEquip = false;
	}

	TPlayerTitleData data;
	data.iTitleIndex = iTitleIndex;
	data.iEndTime = iEndTime;
	data.bIsEquip = bIsEquip;
	data.bIsObtain = bIsObtain;

	m_PlayerTitleMap[iTitleIndex] = data;

	if (bIsEquip)
		m_iEquippedTitleIndex = iTitleIndex;
	else if (m_iEquippedTitleIndex == iTitleIndex)
		m_iEquippedTitleIndex = -1;
}

void CPythonTitleManager::RemovePlayerTitleData(int iTitleIndex)
{
	if (m_iEquippedTitleIndex == iTitleIndex)
		m_iEquippedTitleIndex = -1;

	m_PlayerTitleMap.erase(iTitleIndex);
}

void CPythonTitleManager::RequestEquip(int iTitleIndex)
{
	DWORD dwTitleID = (iTitleIndex < 0) ? 0 : (DWORD)iTitleIndex;
	CPythonNetworkStream::Instance().SendTitleEquipPacket(dwTitleID);

	if (m_iEquippedTitleIndex >= 0)
	{
		auto it = m_PlayerTitleMap.find(m_iEquippedTitleIndex);
		if (it != m_PlayerTitleMap.end())
			it->second.bIsEquip = false;
	}

	if (iTitleIndex < 0)
	{
		m_iEquippedTitleIndex = -1;
		UpdateMyCharacterTitle();
		return;
	}

	auto it = m_PlayerTitleMap.find(iTitleIndex);
	if (it != m_PlayerTitleMap.end())
	{
		it->second.bIsEquip = true;
		m_iEquippedTitleIndex = iTitleIndex;
		UpdateMyCharacterTitle();
	}
}

void CPythonTitleManager::ShowEffect(DWORD dwVID, int iTitleIndex)
{
	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwVID);
	if (pInstance && !pInstance->IsPC())
		return;

	HideEffect(dwVID);

	m_ActiveTitleByVID[dwVID] = iTitleIndex;

	auto itTitle = m_TitleDataMap.find(iTitleIndex);
	if (itTitle == m_TitleDataMap.end())
		return;

	const TTitleData& titleData = itTitle->second;

	auto itRes = m_ResourceMap.find(titleData.iResourceIndex);
	if (itRes == m_ResourceMap.end())
		return;

	const TSubFileData& subData = ParseSubFile(itRes->second.strSubFilename);

	auto itType = subData.find("type");
	std::string strSubType = (itType != subData.end()) ? itType->second : "";

	if (CPythonSystem::Instance().IsShowTitle())
	{
		auto itEffect = subData.find("effect");
		if (itEffect != subData.end() && !itEffect->second.empty())
		{
			std::string strEffectPath = "d:/ymir work/" + itEffect->second;
			CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwVID);
			if (pInstance)
				pInstance->AttachTitleEffectByPath(strEffectPath.c_str());
		}
		else if (strSubType != "IMAGE")
		{
			int iEffectIndex = itRes->second.iEffectIndex;
			const char* szFallbackPath = GetFallbackEffectPath(iEffectIndex);
			if (szFallbackPath && szFallbackPath[0] != '\0')
			{
				CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwVID);
				if (pInstance)
					pInstance->AttachTitleEffectByPath(szFallbackPath);
			}
		}
	}

	int iColorFamily = COLOR_FAMILY_NONE;
	auto itColor = m_ColorFamilyMap.find(titleData.iResourceIndex);
	if (itColor != m_ColorFamilyMap.end())
		iColorFamily = itColor->second;

	const char* szNpLeft = "";
	const char* szNpMiddle = "";
	const char* szNpRight = "";

	auto itNp = ms_NameplateByColor.find(iColorFamily);
	if (itNp != ms_NameplateByColor.end())
	{
		szNpLeft = itNp->second.strLeft.c_str();
		szNpMiddle = itNp->second.strMiddle.c_str();
		szNpRight = itNp->second.strRight.c_str();
	}

	if (strSubType == "NAMEPLATE")
	{
		std::string strSpriteImage;
		int iSpriteFrameCount = 0;
		int iSpriteSizeX = 0, iSpriteSizeY = 0, iSpriteColumns = 6;

		auto itSprites = subData.find("sprites");
		auto itCount = subData.find("sprites_count");

		if (itSprites != subData.end() && itCount != subData.end())
		{
			int iCount = atoi(itCount->second.c_str());
			if (!itSprites->second.empty() && iCount > 0)
			{
				strSpriteImage = "d:/ymir work/" + itSprites->second;
				iSpriteFrameCount = iCount;

				auto itSX = subData.find("sprite_size_x");
				auto itSY = subData.find("sprite_size_y");
				auto itSC = subData.find("sprite_columns");

				if (itSX != subData.end()) iSpriteSizeX = atoi(itSX->second.c_str());
				if (itSY != subData.end()) iSpriteSizeY = atoi(itSY->second.c_str());
				if (itSC != subData.end()) iSpriteColumns = atoi(itSC->second.c_str());
			}
		}

		SetTitleForCharacter(dwVID, titleData.strName.c_str(), titleData.dwFontColor,
			szNpLeft, szNpMiddle, szNpRight,
			strSpriteImage.c_str(), iSpriteFrameCount, iSpriteSizeX, iSpriteSizeY, iSpriteColumns);
	}
	else if (strSubType == "IMAGE")
	{
		auto itImage = subData.find("image");
		if (itImage != subData.end() && !itImage->second.empty())
		{
			std::string strImagePath = "d:/ymir work/" + itImage->second;
			SetTitleForCharacter(dwVID, "", 0, "", "", "", strImagePath.c_str(), 1, 0, 0, 1);
		}
	}
	else if (strSubType == "EFFECT")
	{
		SetTitleForCharacter(dwVID, "", 0, "", "", "", "", 0, 0, 0, 1);
	}
	else
	{
		SetTitleForCharacter(dwVID, titleData.strName.c_str(), titleData.dwFontColor, szNpLeft, szNpMiddle, szNpRight, "", 0, 0, 0, 6);
	}
}

void CPythonTitleManager::HideEffect(DWORD dwVID)
{
	m_ActiveTitleByVID.erase(dwVID);

	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwVID);
	if (pInstance)
		pInstance->DetachTitleEffect();

	ClearTitleForCharacter(dwVID);
}

void CPythonTitleManager::SetTitleForCharacter(DWORD dwVID, const char* c_szName, DWORD dwFontColor,
	const char* c_szNpLeft, const char* c_szNpMiddle, const char* c_szNpRight,
	const char* c_szSpriteImage, int iSpriteFrameCount,
	int iSpriteSizeX, int iSpriteSizeY, int iSpriteColumns)
{
	float fr, fg, fb;
	if (dwFontColor != 0)
	{
		fr = ((dwFontColor >> 16) & 0xFF) / 255.0f;
		fg = ((dwFontColor >> 8) & 0xFF) / 255.0f;
		fb = (dwFontColor & 0xFF) / 255.0f;
	}
	else
	{
		fr = 1.0f;
		fg = 1.0f;
		fb = 1.0f;
	}

	CPythonTextTail::Instance().SetTitleSystemData(dwVID, c_szName, D3DXCOLOR(fr, fg, fb, 1.0f), c_szNpLeft, c_szNpMiddle, c_szNpRight, c_szSpriteImage, iSpriteFrameCount, iSpriteSizeX, iSpriteSizeY, iSpriteColumns);
}

void CPythonTitleManager::ClearTitleForCharacter(DWORD dwVID)
{
	CPythonTextTail::Instance().ClearTitleSystemData(dwVID);
}

void CPythonTitleManager::UpdateMyCharacterTitle()
{
	DWORD dwMainVID = CPythonPlayer::Instance().GetMainCharacterIndex();
	if (dwMainVID == 0)
		return;

	if (m_iEquippedTitleIndex >= 0 && m_TitleDataMap.find(m_iEquippedTitleIndex) != m_TitleDataMap.end())
	{
		ShowEffect(dwMainVID, m_iEquippedTitleIndex);
	}
	else
	{
		HideEffect(dwMainVID);
	}
}

void CPythonTitleManager::RefreshMyTitle()
{
	UpdateMyCharacterTitle();
}

void CPythonTitleManager::RefreshAllVisibleTitles()
{
	std::map<DWORD, int> copyMap = m_ActiveTitleByVID;
	for (auto it = copyMap.begin(); it != copyMap.end(); ++it)
	{
		ShowEffect(it->first, it->second);
	}
}

void CPythonTitleManager::RefreshTitleByVID(DWORD dwVID)
{
	auto it = m_ActiveTitleByVID.find(dwVID);
	if (it != m_ActiveTitleByVID.end())
	{
		ShowEffect(dwVID, it->second);
	}
}

bool CPythonTitleManager::Create(DWORD dwWidth, DWORD dwHeight)
{
	if (!m_pBackgroundImage)
	{
		CResource* pResource = CResourceManager::Instance().GetResourcePointer("d:/ymir work/ui/game/title/window/model_view_background.sub");
		m_pBackgroundImage = new CGraphicImageInstance;
		m_pBackgroundImage->SetImagePointer(static_cast<CGraphicImage*>(pResource));
		
		m_pBackgroundImage->SetScale(static_cast<float>(dwWidth) / 224.0f, static_cast<float>(dwHeight) / 346.0f);
	}

	return true;
}

void CPythonTitleManager::CreateEffectPreview(int iTitleIndex)
{
	if (m_iCurrentPreviewTitleIndex == iTitleIndex && m_pEffectInstance)
		return;

	DestroyEffectPreview();

	TEffectPreviewData effectData;
	if (!GetPreviewEffectData(iTitleIndex, &effectData))
		return;

	DWORD dwEffectID = 0;
	if (!CEffectManager::Instance().RegisterEffect2(effectData.strEffectPath.c_str(), &dwEffectID))
		return;

	CEffectManager::Instance().CreateUnsafeEffectInstance(dwEffectID, &m_pEffectInstance);
	if (!m_pEffectInstance)
		return;

	D3DXMATRIX matGlobal;
	D3DXMatrixTranslation(&matGlobal, 0.0f, 0.0f, 100.0f);
	m_pEffectInstance->SetGlobalMatrix(matGlobal);
	m_pEffectInstance->SetAlwaysRender(true);
	//m_pEffectInstance->SetParticleScale(2.0f); // Enlarged to 2.0x
	m_pEffectInstance->SetActive();
	m_pEffectInstance->Show();

	m_iCurrentPreviewTitleIndex = iTitleIndex;
	Tracef("CPythonTitleManager::CreateEffectPreview: Created preview for title %d (Effect: %s)\n", iTitleIndex, effectData.strEffectPath.c_str());
}

void CPythonTitleManager::DestroyEffectPreview()
{
	if (m_pEffectInstance)
	{
		CEffectManager::Instance().DestroyUnsafeEffectInstance(m_pEffectInstance);
		m_pEffectInstance = nullptr;
	}

	m_iCurrentPreviewTitleIndex = -1;
}

void CPythonTitleManager::SetShowPreview(bool bShow)
{
	m_bShowPreview = bShow;
}

void CPythonTitleManager::UpdateModel()
{
	if (!m_bShowPreview)
		return;

	if (m_pEffectInstance)
		m_pEffectInstance->Update();
}

void CPythonTitleManager::RenderBackground() const
{
	if (!m_bShowPreview)
		return;

	if (!m_pBackgroundImage)
		return;

	CGraphicRenderTargetTexture* pTex = CRenderTargetManager::Instance().GetRenderTargetTexture(CRenderTargetManager::RENDER_TARGET_INDEX_TITLE);
	if (!pTex) return;

	pTex->SetRenderTarget();
	pTex->Clear();

	CPythonGraphic::Instance().SetInterfaceRenderState();

	float fOrthoX = static_cast<float>(CPythonApplication::Instance().GetWidth()) / 224.0f;
	float fOrthoY = static_cast<float>(CPythonApplication::Instance().GetHeight()) / 346.0f;

	m_pBackgroundImage->SetPosition(1.0f * fOrthoX, 1.0f * fOrthoY);
	m_pBackgroundImage->Render();

	pTex->ResetRenderTarget();
}

void CPythonTitleManager::RenderModel() const
{
	if (!m_bShowPreview)
		return;

	CGraphicRenderTargetTexture* pTex = CRenderTargetManager::Instance().GetRenderTargetTexture(CRenderTargetManager::RENDER_TARGET_INDEX_TITLE);
	if (!pTex) return;

	pTex->SetRenderTarget();


	if (!m_pEffectInstance)
	{
		pTex->ResetRenderTarget();
		return;
	}

	CPythonGraphic::Instance().SetViewport(0.0f, 0.0f, 224.0f, 346.0f);
	CPythonGraphic::Instance().ClearDepthBuffer();

	const float fFov = CPythonGraphic::Instance().GetFOV();
	const float fAspect = CPythonGraphic::Instance().GetAspect();
	const float fNearY = CPythonGraphic::Instance().GetNear();
	const float fFarY = CPythonGraphic::Instance().GetFar();

	const BOOL bIsFog = STATEMANAGER.GetRenderState(D3DRS_FOGENABLE);
	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);

	CCameraManager::Instance().SetCurrentCamera(CCameraManager::DEFAULT_TITLE_CAMERA);
	CCamera* pCam = CCameraManager::Instance().GetCurrentCamera();

	CPythonGraphic::Instance().PushState();

	pCam->SetViewParams(
		D3DXVECTOR3(0.0f, -600.0f, 80.0f),
		D3DXVECTOR3(0.0f, 0.0f, 80.0f),
		D3DXVECTOR3(0.0f, 0.0f, 1.0f)
	);
	CPythonGraphic::Instance().UpdateViewMatrix();

	CPythonGraphic::Instance().SetPerspective(45.0f, 224.0f / 346.0f, 100.0f, 15000.0f);
	CPythonGraphic::Instance().BuildViewFrustum();

	if (m_pEffectInstance)
		m_pEffectInstance->OnRender();

	CCameraManager::Instance().ResetToPreviousCamera();
	CPythonGraphic::Instance().RestoreViewport();
	CPythonGraphic::Instance().PopState();
	CPythonGraphic::Instance().SetPerspective(fFov, fAspect, fNearY, fFarY);
	CPythonGraphic::Instance().UpdateViewMatrix();
	CPythonGraphic::Instance().BuildViewFrustum();
	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, bIsFog);
	pTex->ResetRenderTarget();
}
#endif // ENABLE_TITLE_SYSTEM
