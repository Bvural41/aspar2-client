#pragma once
#ifdef ENABLE_TITLE_SYSTEM
#include "../EterBase/Singleton.h"
#include <map>
#include <string>
#include <vector>

#include "../EffectLib/EffectInstance.h"

class CGraphicImageInstance;

class CPythonTitleManager : public CSingleton<CPythonTitleManager>
{
public:
	enum ETitleDataColumn
	{
		DATA_COLUMN_TITLE_INDEX = 0,
		DATA_COLUMN_TITLE_TYPE,
		DATA_COLUMN_NAME,
		DATA_COLUMN_CONDITION_TOOLTIP,
		DATA_COLUMN_IS_PERMANENT,
		DATA_COLUMN_OPEN_TIME,
		DATA_COLUMN_FONT_COLOR,
		DATA_COLUMN_RESOURCE_INDEX,
		DATA_COLUMN_IS_WZ,
		DATA_COLUMN_MAX,
	};

	enum EPlayerColumn
	{
		PLAYER_COLUMN_TITLE_INDEX = 0,
		PLAYER_COLUMN_END_TIME,
		PLAYER_COLUMN_IS_EQUIP,
		PLAYER_COLUMN_IS_OBTAIN,
		PLAYER_COLUMN_MAX,
	};

	enum ETitleType
	{
		TYPE_NONE = 0,
		TYPE_TEXT,
		TYPE_IMAGE,
		TYPE_EFFECT,
		TYPE_NAMEPLATE,
	};

	enum EColorFamily
	{
		COLOR_FAMILY_NONE = 0,
		COLOR_FAMILY_GOLD,
		COLOR_FAMILY_RED,
		COLOR_FAMILY_BLUE,
	};

	struct TTitleData
	{
		int iTitleIndex;
		int iTitleType;
		std::string strName;
		std::string strConditionTooltip;
		bool bIsPermanent;
		std::string strOpenTime;
		DWORD dwFontColor;
		int iResourceIndex;
		bool bIsWZ;
	};

	struct TResourceData
	{
		std::string strSubFilename;
		int iEffectIndex;
	};

	struct TPlayerTitleData
	{
		int iTitleIndex;
		int iEndTime;
		bool bIsEquip;
		bool bIsObtain;
	};

	typedef std::map<std::string, std::string> TSubFileData;

	struct TNameplatePaths
	{
		std::string strLeft;
		std::string strMiddle;
		std::string strRight;
	};

	struct TSpritePreviewData
	{
		std::string strDirPath;
		int iFrameCount;
		int iSizeX;
		int iSizeY;
		int iColumns;
	};

	struct TEffectPreviewData
	{
		std::string strEffectPath;
		int iColorFamily;
	};

	struct TImagePreviewData
	{
		std::string strImagePath;
	};

public:
	CPythonTitleManager();
	virtual ~CPythonTitleManager();

	void Destroy();

	void Initialize();
	void Clear();
	bool IsDataReceived() const { return m_bDataReceived; }

	bool LoadTitleData(const char* c_szFilename, bool bIsWZ = false);
	bool LoadTitleResourceList(const char* c_szFilename);
	void BuildColorFamilies();

	const std::map<int, TTitleData>& GetAllTitleData() const { return m_TitleDataMap; }
	const std::map<int, TPlayerTitleData>& GetPlayerTitleMap() const { return m_PlayerTitleMap; }
	int GetEquippedTitle() const { return m_iEquippedTitleIndex; }
	bool IsTitleEquipped() const { return m_iEquippedTitleIndex >= 0; }

	bool GetTitleData(int iTitleIndex, TTitleData* pData) const;
	int GetColorFamily(int iTitleIndex) const;
	bool GetNameplatePaths(int iColorFamily, TNameplatePaths* pPaths) const;
	bool GetPreviewSpriteData(int iTitleIndex, TSpritePreviewData* pData);
	bool GetPreviewEffectData(int iTitleIndex, TEffectPreviewData* pData);
	bool GetPreviewImageData(int iTitleIndex, TImagePreviewData* pData);
	const char* GetItemTooltip(int iTitleIndex) const;

	void SetPlayerTitleData(int iTitleIndex, int iEndTime, bool bIsEquip, bool bIsObtain);
	void RemovePlayerTitleData(int iTitleIndex);
	void RequestEquip(int iTitleIndex);

	void ShowEffect(DWORD dwVID, int iTitleIndex);
	void HideEffect(DWORD dwVID);
	void RefreshMyTitle();
	void RefreshAllVisibleTitles();
	void RefreshTitleByVID(DWORD dwVID);

	bool Create(DWORD dwWidth, DWORD dwHeight);
	void CreateEffectPreview(int iTitleIndex);
	void DestroyEffectPreview();
	void SetShowPreview(bool bShow);
	void UpdateModel();

	void RenderBackground() const;
	void RenderModel() const;

	static const char* TITLE_DATA_PATH;
	static const char* TITLE_PREVIEW_IMAGE;
	static std::map<int, TNameplatePaths> ms_NameplateByColor;

private:
	TSubFileData& ParseSubFile(const std::string& strSubFilename);
	int DetectColorFamily(const TSubFileData& subData, int iResourceIndex) const;
	void SetTitleForCharacter(DWORD dwVID, const char* c_szName, DWORD dwFontColor, const char* c_szNpLeft, const char* c_szNpMiddle, const char* c_szNpRight, const char* c_szSpriteImage, int iSpriteFrameCount, int iSpriteSizeX, int iSpriteSizeY, int iSpriteColumns);
	void ClearTitleForCharacter(DWORD dwVID);
	void UpdateMyCharacterTitle();
	
private:
	std::map<int, TTitleData> m_TitleDataMap;
	std::map<int, TResourceData> m_ResourceMap;
	std::map<int, TPlayerTitleData> m_PlayerTitleMap;
	std::map<int, int> m_ColorFamilyMap;
	std::map<std::string, TSubFileData> m_SubDataCache;
	std::map<int, int> m_EffectResourceColorMap;
	std::map<DWORD, int> m_ActiveTitleByVID;

	int m_iEquippedTitleIndex;
	bool m_bDataReceived;

	CEffectInstance* m_pEffectInstance;
	CGraphicImageInstance* m_pBackgroundImage;
	bool m_bShowPreview;
	int m_iCurrentPreviewTitleIndex;
};
#endif // ENABLE_TITLE_SYSTEM
