#include "StdAfx.h"
#ifdef ENABLE_TITLE_SYSTEM
#include "PythonTitleSystem.h"
#include "PythonTextTail.h"
#include "PythonCharacterManager.h"
#include "PythonNetworkStream.h"
#include "InstanceBase.h"
#include "PythonPlayer.h"
#include "../EterPack/EterPackManager.h"

const char* CPythonTitleSystem::TITLE_DATA_PATH = "d:/ymir work/ui/game/title/";
const char* CPythonTitleSystem::TITLE_PREVIEW_IMAGE = "d:/ymir work/ui/titlesystem_001.dds";

std::map<int, CPythonTitleSystem::TNameplatePaths> CPythonTitleSystem::ms_NameplateByColor;

CPythonTitleSystem::CPythonTitleSystem() : m_iEquippedTitleIndex(-1), m_bDataReceived(false)
{
	// Initialize static nameplate paths
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

CPythonTitleSystem::~CPythonTitleSystem()
{
}

void CPythonTitleSystem::Clear()
{
	m_PlayerTitleMap.clear();
	m_iEquippedTitleIndex = -1;
	m_bDataReceived = false;
}

void CPythonTitleSystem::Initialize()
{
	LoadTitleResourceList("title_resource_list.txt");

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

	bool bLoaded = LoadTitleData(strTitleDataFile.c_str());
	if (!bLoaded)
	{
		std::string strFallback = "locale/" + std::string(localePath) + "/title_gf.txt";
		bLoaded = LoadTitleData(strFallback.c_str());
	}
	if (!bLoaded)
	{
		LoadTitleData("title_gf.txt");
	}

	std::string strWzFile = std::string(localePath) + "/title_wz.txt";
	bool bLoadedWz = LoadTitleData(strWzFile.c_str());
	if (!bLoadedWz)
	{
		std::string strWzFallback = "locale/" + std::string(localePath) + "/title_wz.txt";
		bLoadedWz = LoadTitleData(strWzFallback.c_str());
	}
	if (!bLoadedWz)
		LoadTitleData("title_wz.txt");

	BuildColorFamilies();

	RegisterTitleEffects();

	m_bDataReceived = true;

	Tracef("CPythonTitleSystem::Initialize: Loaded %d titles, %d resources\n", m_TitleDataMap.size(), m_ResourceMap.size());
}

bool CPythonTitleSystem::LoadTitleData(const char* c_szFilename)
{
	const VOID* pvData;
	CMappedFile kFile;

	if (!CEterPackManager::Instance().Get(kFile, c_szFilename, &pvData))
	{
		TraceError("CPythonTitleSystem::LoadTitleData: Failed to open %s", c_szFilename);
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
		titleData.iTitleIndex = atoi(kTokenVector[0].c_str());
		titleData.iResourceIndex = atoi(kTokenVector[1].c_str());
		titleData.iTitleType = atoi(kTokenVector[2].c_str());
		titleData.strName = kTokenVector[3];
		titleData.strConditionTooltip = kTokenVector[4];
		titleData.bIsPermanent = true;
		titleData.strOpenTime = kTokenVector.size() > 6 ? kTokenVector[6] : "";
		titleData.dwFontColor = kTokenVector.size() > 7 ? strtoul(kTokenVector[7].c_str(), NULL, 10) : 0xFFFFFFFF;

		m_TitleDataMap[titleData.iTitleIndex] = titleData;
	}

	return true;
}

bool CPythonTitleSystem::LoadTitleResourceList(const char* c_szFilename)
{
	const VOID* pvData;
	CMappedFile kFile;

	if (!CEterPackManager::Instance().Get(kFile, c_szFilename, &pvData))
	{
		TraceError("CPythonTitleSystem::LoadTitleResourceList: Failed to open %s", c_szFilename);
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

CPythonTitleSystem::TSubFileData& CPythonTitleSystem::ParseSubFile(const std::string& strSubFilename)
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
		TraceError("CPythonTitleSystem::ParseSubFile: Failed to open %s", strFullPath.c_str());
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

int CPythonTitleSystem::DetectColorFamily(const TSubFileData& subData, int iResourceIndex) const
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

void CPythonTitleSystem::BuildColorFamilies()
{
	m_ColorFamilyMap.clear();

	for (auto it = m_ResourceMap.begin(); it != m_ResourceMap.end(); ++it)
	{
		int iResourceIndex = it->first;
		const TSubFileData& subData = ParseSubFile(it->second.strSubFilename);
		int iColorFamily = DetectColorFamily(subData, iResourceIndex);
		m_ColorFamilyMap[iResourceIndex] = iColorFamily;
	}

	Tracef("CPythonTitleSystem::BuildColorFamilies: Mapped %d resources to color families\n",
		m_ColorFamilyMap.size());
}

bool CPythonTitleSystem::GetTitleData(int iTitleIndex, TTitleData* pData) const
{
	auto it = m_TitleDataMap.find(iTitleIndex);
	if (it == m_TitleDataMap.end())
		return false;
	*pData = it->second;
	return true;
}

int CPythonTitleSystem::GetColorFamily(int iTitleIndex) const
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

bool CPythonTitleSystem::GetNameplatePaths(int iColorFamily, TNameplatePaths* pPaths) const
{
	auto it = ms_NameplateByColor.find(iColorFamily);
	if (it == ms_NameplateByColor.end())
		return false;
	*pPaths = it->second;
	return true;
}

bool CPythonTitleSystem::GetPreviewSpriteData(int iTitleIndex, TSpritePreviewData* pData)
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

bool CPythonTitleSystem::GetPreviewEffectData(int iTitleIndex, TEffectPreviewData* pData)
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

	int iEffectIndex = itRes->second.iEffectIndex;
	if (iEffectIndex > 0 && iEffectIndex <= 8)
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

		pData->strEffectPath = s_effectPaths[iEffectIndex - 1];
		pData->iColorFamily = COLOR_FAMILY_GOLD;
		return true;
	}

	return false;
}

bool CPythonTitleSystem::GetPreviewImageData(int iTitleIndex, TImagePreviewData* pData)
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

const char* CPythonTitleSystem::GetItemTooltip(int iTitleIndex) const
{
	auto it = m_TitleDataMap.find(iTitleIndex);
	if (it != m_TitleDataMap.end())
		return it->second.strConditionTooltip.c_str();
	return "";
}

void CPythonTitleSystem::SetPlayerTitleData(int iTitleIndex, int iEndTime, bool bIsEquip, bool bIsObtain)
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

void CPythonTitleSystem::RemovePlayerTitleData(int iTitleIndex)
{
	if (m_iEquippedTitleIndex == iTitleIndex)
		m_iEquippedTitleIndex = -1;

	m_PlayerTitleMap.erase(iTitleIndex);
}

void CPythonTitleSystem::RequestEquip(int iTitleIndex)
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

void CPythonTitleSystem::ShowEffect(DWORD dwVID, int iTitleIndex)
{
	HideEffect(dwVID);

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
		int iEffectIndex = itRes->second.iEffectIndex;
		if (iEffectIndex > 0)
		{
			UINT effectType = GetEffectTypeForIndex(iEffectIndex);
			if (effectType != 0)
			{
				CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwVID);
				if (pInstance)
					pInstance->AttachSpecialEffect(effectType);
			}
		}

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
		int iEffectIndex = itRes->second.iEffectIndex;
		if (iEffectIndex > 0)
		{
			UINT effectType = GetEffectTypeForIndex(iEffectIndex);
			if (effectType != 0)
			{
				CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwVID);
				if (pInstance)
					pInstance->AttachSpecialEffect(effectType);
			}
		}
	}
	else
	{
		SetTitleForCharacter(dwVID, titleData.strName.c_str(), titleData.dwFontColor, szNpLeft, szNpMiddle, szNpRight, "", 0, 0, 0, 6);
	}
}

void CPythonTitleSystem::HideEffect(DWORD dwVID)
{
	CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwVID);
	if (pInstance)
		pInstance->DetachTitleEffect();

	ClearTitleForCharacter(dwVID);
}

void CPythonTitleSystem::SetTitleForCharacter(DWORD dwVID, const char* c_szName, DWORD dwFontColor,
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

void CPythonTitleSystem::ClearTitleForCharacter(DWORD dwVID)
{
	CPythonTextTail::Instance().ClearTitleSystemData(dwVID);
}

void CPythonTitleSystem::UpdateMyCharacterTitle()
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

void CPythonTitleSystem::RefreshMyTitle()
{
	UpdateMyCharacterTitle();
}

void CPythonTitleSystem::RegisterTitleEffects()
{
	static const struct { UINT effectType; const char* msePath; } s_titleEffects[] =
	{
		{ CInstanceBase::EFFECT_TITLE_01, "d:/ymir work/effect/etc/title/title_01_shield.mse" },
		{ CInstanceBase::EFFECT_TITLE_02, "d:/ymir work/effect/etc/title/title_02_dragon.mse" },
		{ CInstanceBase::EFFECT_TITLE_03, "d:/ymir work/effect/etc/title/title_03_fist.mse" },
		{ CInstanceBase::EFFECT_TITLE_04, "d:/ymir work/effect/etc/title/title_04_trophy.mse" },
		{ CInstanceBase::EFFECT_TITLE_05, "d:/ymir work/effect/etc/title/title_05_medal.mse" },
		{ CInstanceBase::EFFECT_TITLE_06, "d:/ymir work/effect/etc/title/title_06_banner_gold.mse" },
		{ CInstanceBase::EFFECT_TITLE_07, "d:/ymir work/effect/etc/title/title_07_banner_red.mse" },
		{ CInstanceBase::EFFECT_TITLE_08, "d:/ymir work/effect/etc/title/title_08_banner_blue.mse" },
	};

	for (int i = 0; i < _countof(s_titleEffects); ++i)
	{
		if (!CInstanceBase::RegisterEffect(s_titleEffects[i].effectType, "", s_titleEffects[i].msePath, false))
		{
			TraceError("CPythonTitleSystem::RegisterTitleEffects: Failed to register effect %d (%s)",
				s_titleEffects[i].effectType, s_titleEffects[i].msePath);
		}
	}

	Tracef("CPythonTitleSystem::RegisterTitleEffects: Registered %d title effects\n", _countof(s_titleEffects));
}

UINT CPythonTitleSystem::GetEffectTypeForIndex(int iEffectIndex) const
{
	if (iEffectIndex >= 1 && iEffectIndex <= 8)
		return CInstanceBase::EFFECT_TITLE_01 + (iEffectIndex - 1);

	return 0;
}
#endif // ENABLE_TITLE_SYSTEM
