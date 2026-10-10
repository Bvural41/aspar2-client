#pragma once

#include "../EterBase/Singleton.h"

class CPythonTextTail : public CSingleton<CPythonTextTail>
{
	public:
		typedef struct STextTail
		{
			CGraphicTextInstance*			pTextInstance;
			CGraphicTextInstance*			pOwnerTextInstance;

			CGraphicMarkInstance*			pMarkInstance;
			CGraphicTextInstance*			pGuildNameTextInstance;

			CGraphicTextInstance*			pTitleTextInstance;
#ifdef ENABLE_TITLE_SYSTEM
			CGraphicTextInstance* 			pTitleSystemTextInstance;
			CGraphicImageInstance*			pTitleNameplateLeft;
			CGraphicImageInstance*			pTitleNameplateMiddle;
			CGraphicImageInstance*			pTitleNameplateRight;
	
			CGraphicImageInstance* pSpriteInstance;
			std::vector<CGraphicImage*> vSpriteImages;
			int iSpriteCurrentFrame;
			float fSpriteTimer;
			float fTitleSystemY;
#endif
			CGraphicTextInstance*			pLevelTextInstance;
#ifdef ENABLE_BATTLE_ROYALE
			CGraphicImageInstance*			pBattleRoyaleCrownImageInstance;
#endif
#ifdef ENABLE_BATTLE_FIELD
			CGraphicImageInstance*			pCombatZoneRankInstance;
#endif
#if defined(ENABLE_SHOW_MOB_INFO)
			CGraphicTextInstance*			pAIFlagTextInstance;
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
			CGraphicImageInstance*			pLanguageInstance;
#endif
#ifdef ENABLE_LEFT_SEAT
			CGraphicTextInstance*			pLeftSeatTextInstance;
#endif
			CGraphicObjectInstance *		pOwner;

			DWORD							dwVirtualID;

			float							x, y, z;
			float							fDistanceFromPlayer;
			D3DXCOLOR						Color;
			BOOL							bNameFlag;

			float							xStart, yStart;
			float							xEnd, yEnd;

			DWORD							LivingTime;

			float							fHeight;
#if defined(ENABLE_SHOW_MOB_INFO)
			BOOL							bIsPC;
#ifdef ENABLE_GROWTH_PET_SYSTEM
			BOOL							bIsGrowthPet;
#endif
#endif
			STextTail() {}
			virtual ~STextTail() {}
		} TTextTail;

		typedef std::map<DWORD, TTextTail*>		TTextTailMap;
		typedef std::list<TTextTail*>			TTextTailList;
		typedef TTextTailMap					TChatTailMap;

#ifdef ENABLE_TITLE_SYSTEM
	private:
		void AttachTitleSystem(DWORD dwVID, const char* c_szName, const D3DXCOLOR& c_rColor);
		void DetachTitleSystem(DWORD dwVID);
		void SetTitleNameplate(DWORD dwVID, const char* c_szLeft, const char* c_szMiddle, const char* c_szRight);
		void ClearTitleNameplate(DWORD dwVID);
		void SetTitleSpriteAnimation(DWORD dwVID, const char* c_szImagePath, int iFrameCount, int iSizeX, int iSizeY, int iColumns);
		void ClearTitleSpriteAnimation(DWORD dwVID);
		void ClearTitleVisuals(TTextTail* pTextTail);
#endif

	public:
		CPythonTextTail(void);
		virtual ~CPythonTextTail(void);

		void GetInfo(std::string* pstInfo);

		void Initialize();
		void Destroy();

		void Clear();

		void UpdateAllTextTail();
		void UpdateShowingTextTail();
		void Render();

		void ArrangeTextTail();
		void HideAllTextTail();
		void ShowAllTextTail();

		void ShowCharacterTextTail(DWORD VirtualID);
		void ShowItemTextTail(DWORD VirtualID);

		void RegisterCharacterTextTail(DWORD dwGuildID, DWORD dwVirtualID, const D3DXCOLOR & c_rColor, float fAddHeight=10.0f);
#ifdef ENABLE_ITEM_DROP_RENEWAL
		void RegisterItemTextTail(DWORD VirtualID, const char* c_szText, CGraphicObjectInstance* pOwner, bool bHasAttr = false);
#else
		void RegisterItemTextTail(DWORD VirtualID, const char* c_szText, CGraphicObjectInstance* pOwner);
#endif
		void RegisterChatTail(DWORD VirtualID, const char * c_szChat);
		void RegisterInfoTail(DWORD VirtualID, const char * c_szChat);
		void SetCharacterTextTailColor(DWORD VirtualID, const D3DXCOLOR & c_rColor);
		void SetItemTextTailOwner(DWORD dwVID, const char * c_szName);
		void DeleteCharacterTextTail(DWORD virtualID);
#ifdef ENABLE_TITLE_SYSTEM
		void SetTitleSystemData(DWORD dwVID, const char* c_szName, const D3DXCOLOR& c_rColor, const char* c_szNameplateLeft = "", const char* c_szNameplateMiddle = "", const char* c_szNameplateRight = "",
		const char* c_szSpriteImage = "", int iSpriteFrameCount = 0, int iSpriteSizeX = 0, int iSpriteSizeY = 0, int iSpriteColumns = 6);
		void ClearTitleSystemData(DWORD dwVID);
#endif

		void DeleteItemTextTail(DWORD VirtualID);

		int Pick(int ixMouse, int iyMouse);
		void SelectItemName(DWORD dwVirtualID);

		bool GetTextTailPosition(DWORD dwVID, float* px, float* py, float* pz);
		bool IsChatTextTail(DWORD dwVID);

		void EnablePKTitle(BOOL bFlag);
		void AttachTitle(DWORD dwVID, const char * c_szName, const D3DXCOLOR& c_rColor);
		void DetachTitle(DWORD dwVID);

		void AttachLevel(DWORD dwVID, const char* c_szText, const D3DXCOLOR& c_rColor);
		void DetachLevel(DWORD dwVID);

#ifdef ENABLE_CONQUEROR_LEVEL
		void AttachConquerorLevel(DWORD dwVID, const char* c_szText);
#endif

#ifdef ENABLE_LEFT_SEAT
		void AttachLeftSeatText(DWORD dwVID, const std::string& c_rstrText);
		void DetachLeftSeatText(DWORD dwVID);
#endif

	protected:
		TTextTail * RegisterTextTail(DWORD dwVirtualID, const char * c_szText, CGraphicObjectInstance * pOwner, float fHeight, const D3DXCOLOR & c_rColor);
		void DeleteTextTail(TTextTail * pTextTail);

		void UpdateTextTail(TTextTail * pTextTail);
		void RenderTextTailBox(TTextTail * pTextTail);
		void RenderTextTailName(TTextTail * pTextTail);
		void UpdateDistance(const TPixelPosition & c_rCenterPosition, TTextTail * pTextTail);

		bool isIn(TTextTail * pSource, TTextTail * pTarget);

	protected:
		TTextTailMap				m_CharacterTextTailMap;
		TTextTailMap				m_ItemTextTailMap;
		TChatTailMap				m_ChatTailMap;

		TTextTailList				m_CharacterTextTailList;
		TTextTailList				m_ItemTextTailList;

	private:
		CDynamicPool<STextTail>		m_TextTailPool;

#ifdef ENABLE_TITLE_SYSTEM
	public:
		struct TTitleSystemData
		{
			std::string strName;
			D3DXCOLOR color;
			std::string strNameplateLeft;
			std::string strNameplateMiddle;
			std::string strNameplateRight;
			std::string strSpriteImage;
			int iSpriteFrameCount;
			int iSpriteSizeX;
			int iSpriteSizeY;
			int iSpriteColumns;
		};
		std::map<DWORD, TTitleSystemData> m_PendingTitleSystemMap;
#endif
};



