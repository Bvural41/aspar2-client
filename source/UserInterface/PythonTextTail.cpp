#include "StdAfx.h"
#include "InstanceBase.h"
#include "resource.h"
#include "PythonTextTail.h"
#include "PythonCharacterManager.h"
#include "PythonGuild.h"
#include "Locale.h"
#include "PythonPlayer.h"
#include "MarkManager.h"
#if defined(ENABLE_SHOW_MOB_INFO)
#include "PythonSystem.h"
#endif
#ifdef ENABLE_BATTLE_ROYALE
#include "PythonBattleRoyaleManager.h"
#endif
#ifdef ENABLE_TITLE_SYSTEM
#include "../EterLib/StateManager.h"
#include "PythonTitleManager.h"
#endif

const D3DXCOLOR c_TextTail_Player_Color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
const D3DXCOLOR c_TextTail_Monster_Color = D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f);
const D3DXCOLOR c_TextTail_Item_Color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
#ifdef ENABLE_ITEM_DROP_RENEWAL
const D3DXCOLOR c_TextTail_SpecialItem_Color = D3DXCOLOR(1.0f, 0.67f, 0.0f, 1.0f); // Golden
#endif
const D3DXCOLOR c_TextTail_Chat_Color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
const D3DXCOLOR c_TextTail_Info_Color = D3DXCOLOR(1.0f, 0.785f, 0.785f, 1.0f);
const D3DXCOLOR c_TextTail_Guild_Name_Color = 0xFFEFD3FF;
const float c_TextTail_Name_Position = -10.0f;
const float c_fxMarkPosition = 1.5f;
const float c_fyGuildNamePosition = 15.0f;
const float c_fyMarkPosition = 15.0f + 11.0f;
BOOL bPKTitleEnable = TRUE;

#ifdef ENABLE_LEFT_SEAT
const float c_fyLeftSeatTextPosition = -15.0f;
const D3DXCOLOR c_TextTail_LeftSeat_Text_Color = 0xFFFFFF00;
#endif

// TEXTTAIL_LIVINGTIME_CONTROL
long gs_TextTail_LivingTime = 5000;

long TextTail_GetLivingTime()
{
	assert(gs_TextTail_LivingTime>1000);
	return gs_TextTail_LivingTime;
}

void TextTail_SetLivingTime(long livingTime)
{
	gs_TextTail_LivingTime = livingTime;
}
// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

CGraphicText * ms_pFont = NULL;

void CPythonTextTail::GetInfo(std::string* pstInfo)
{
	char szInfo[256];
	sprintf(szInfo, "TextTail: ChatTail %d, ChrTail (Map %d, List %d), ItemTail (Map %d, List %d), Pool %d", 
		m_ChatTailMap.size(), 
		m_CharacterTextTailMap.size(), m_CharacterTextTailList.size(), 
		m_ItemTextTailMap.size(), m_ItemTextTailList.size(), 
		m_TextTailPool.GetCapacity());

	pstInfo->append(szInfo);
}

void CPythonTextTail::UpdateAllTextTail()
{
	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
	if (pInstance)
	{
		TPixelPosition pixelPos;
		pInstance->NEW_GetPixelPosition(&pixelPos);

		TTextTailMap::iterator itorMap;

		for (itorMap = m_CharacterTextTailMap.begin(); itorMap != m_CharacterTextTailMap.end(); ++itorMap)
		{
			UpdateDistance(pixelPos, itorMap->second);
		}

		for (itorMap = m_ItemTextTailMap.begin(); itorMap != m_ItemTextTailMap.end(); ++itorMap)
		{
			UpdateDistance(pixelPos, itorMap->second);
		}

		for (TChatTailMap::iterator itorChat=m_ChatTailMap.begin(); itorChat!=m_ChatTailMap.end(); ++itorChat)
		{
			UpdateDistance(pixelPos, itorChat->second);

			// NOTE : Chat TextTail? ??? ??? ??? ????.
			if (itorChat->second->bNameFlag)
			{
				DWORD dwVID = itorChat->first;
				ShowCharacterTextTail(dwVID);
			}
		}
	}
}

void CPythonTextTail::UpdateShowingTextTail()
{
	TTextTailList::iterator itor;

	for (itor = m_ItemTextTailList.begin(); itor != m_ItemTextTailList.end(); ++itor)
	{
		UpdateTextTail(*itor);
	}

	for (TChatTailMap::iterator itorChat=m_ChatTailMap.begin(); itorChat!=m_ChatTailMap.end(); ++itorChat)
	{
		UpdateTextTail(itorChat->second);
	}

	for (itor = m_CharacterTextTailList.begin(); itor != m_CharacterTextTailList.end(); ++itor)
	{
		TTextTail * pTextTail = *itor;
		UpdateTextTail(pTextTail);

		// NOTE : Chat TextTail? ?? ?? ??? ???.
		TChatTailMap::iterator itor = m_ChatTailMap.find(pTextTail->dwVirtualID);
		if (m_ChatTailMap.end() != itor)
		{
			TTextTail * pChatTail = itor->second;
			if (pChatTail->bNameFlag)
			{
				pTextTail->y = pChatTail->y - 17.0f;
			}
		}
	}
}

void CPythonTextTail::UpdateTextTail(TTextTail * pTextTail)
{
	if (!pTextTail->pOwner)
		return;

	CPythonGraphic & rpyGraphic = CPythonGraphic::Instance();
	rpyGraphic.Identity();

	const D3DXVECTOR3 & c_rv3Position = pTextTail->pOwner->GetPosition();

	float fAdjustedHeight = pTextTail->fHeight;
#ifdef ENABLE_GROWTH_PET_SYSTEM
	if (pTextTail->bIsGrowthPet)
	{
		const D3DXVECTOR3& scale = pTextTail->pOwner->GetScale();
		if (scale.z > 0.0f)
		{
			const float fPetBaseHeight = 100.0f;
			fAdjustedHeight = (fPetBaseHeight * scale.z);
		}
	}
#endif
	
	rpyGraphic.ProjectPosition(c_rv3Position.x,
							   c_rv3Position.y,
							   c_rv3Position.z + fAdjustedHeight,
							   &pTextTail->x,
							   &pTextTail->y,
							   &pTextTail->z);

	pTextTail->x = floorf(pTextTail->x);
	pTextTail->y = floorf(pTextTail->y);

	// NOTE : 13m ?? ???? ??? ???? - [levites]
	if (pTextTail->fDistanceFromPlayer < 1300.0f)
	{
		pTextTail->z = 0.0f;
	}
	else
	{
		pTextTail->z = pTextTail->z * CPythonGraphic::Instance().GetOrthoDepth() * -1.0f;
		pTextTail->z += 10.0f;
	}
}

void CPythonTextTail::ArrangeTextTail()
{
	TTextTailList::iterator itor;
	TTextTailList::iterator itorCompare;

	DWORD dwTime = CTimer::Instance().GetCurrentMillisecond();

	for (itor = m_ItemTextTailList.begin(); itor != m_ItemTextTailList.end(); ++itor)
	{
		TTextTail * pInsertTextTail = *itor;

		int yTemp = 5;
		int LimitCount = 0;

		for (itorCompare = m_ItemTextTailList.begin(); itorCompare != m_ItemTextTailList.end();)
		{
			TTextTail * pCompareTextTail = *itorCompare;

			if (*itorCompare == *itor)
			{
				++itorCompare;
				continue;
			}

			if (LimitCount >= 20)
				break;

			if (isIn(pInsertTextTail, pCompareTextTail))
			{
				pInsertTextTail->y = (pCompareTextTail->y + pCompareTextTail->yEnd + yTemp);

				itorCompare = m_ItemTextTailList.begin();
				++LimitCount;
				continue;
			}

			++itorCompare;
		}


		if (pInsertTextTail->pOwnerTextInstance)
		{
			pInsertTextTail->pOwnerTextInstance->SetPosition(pInsertTextTail->x, pInsertTextTail->y, pInsertTextTail->z);
			pInsertTextTail->pOwnerTextInstance->Update();

			pInsertTextTail->pTextInstance->SetColor(pInsertTextTail->Color.r, pInsertTextTail->Color.g, pInsertTextTail->Color.b);
			pInsertTextTail->pTextInstance->SetPosition(pInsertTextTail->x, pInsertTextTail->y + 15.0f, pInsertTextTail->z);
			pInsertTextTail->pTextInstance->Update();

		}
		else
		{
			pInsertTextTail->pTextInstance->SetColor(pInsertTextTail->Color.r, pInsertTextTail->Color.g, pInsertTextTail->Color.b);
			pInsertTextTail->pTextInstance->SetPosition(pInsertTextTail->x, pInsertTextTail->y, pInsertTextTail->z);
			pInsertTextTail->pTextInstance->Update();

		}
	}

	for (itor = m_CharacterTextTailList.begin(); itor != m_CharacterTextTailList.end(); ++itor)
	{
		TTextTail * pTextTail = *itor;

		float fxAdd = 0.0f;
#ifdef ENABLE_BATTLE_ROYALE
		float fyAdd = 5.0f;
#endif

		// Mark ?? ????
		CGraphicMarkInstance * pMarkInstance = pTextTail->pMarkInstance;
		CGraphicTextInstance * pGuildNameInstance = pTextTail->pGuildNameTextInstance;
#ifdef ENABLE_BATTLE_FIELD
		CGraphicImageInstance * pCombatZoneRank = pTextTail->pCombatZoneRankInstance;
#endif
		if (pMarkInstance && pGuildNameInstance)
		{
			int iWidth, iHeight;
			int iImageHalfSize = pMarkInstance->GetWidth()/2 + c_fxMarkPosition;
			pGuildNameInstance->GetTextSize(&iWidth, &iHeight);

			pMarkInstance->SetPosition(pTextTail->x - iWidth/2 - iImageHalfSize, pTextTail->y - c_fyMarkPosition);
			pGuildNameInstance->SetPosition(pTextTail->x + iImageHalfSize, pTextTail->y - c_fyGuildNamePosition, pTextTail->z);
			pGuildNameInstance->Update();
		}

#ifdef ENABLE_TITLE_SYSTEM
		CGraphicTextInstance *pTitleSystem = pTextTail->pTitleSystemTextInstance;
		if (pTitleSystem)
		{
			float fyTitleSystemPos = c_fyGuildNamePosition + 20.0f;
			if (!pGuildNameInstance)
				fyTitleSystemPos -= 10.0f;

			float fTitleY = pTextTail->y - fyTitleSystemPos;

				pTextTail->fTitleSystemY = fTitleY;
				pTitleSystem->SetPosition(pTextTail->x, fTitleY, pTextTail->z);
				pTitleSystem->Update();

				if (pTextTail->pTitleNameplateLeft && pTextTail->pTitleNameplateMiddle && pTextTail->pTitleNameplateRight)
				{
					int iNameWidth, iNameHeight;
					pTitleSystem->GetTextSize(&iNameWidth, &iNameHeight);

					float fMiddleScaleX = (float)iNameWidth / (float)pTextTail->pTitleNameplateMiddle->GetWidth();
					pTextTail->pTitleNameplateMiddle->SetScale(fMiddleScaleX, 1.0f);

					float fNameplateY = fTitleY - ((float)iNameHeight / 2.0f) - ((float)pTextTail->pTitleNameplateMiddle->GetHeight() / 2.0f);
					int iHalfWidth = iNameWidth / 2;
					int iLeftWidth = pTextTail->pTitleNameplateLeft->GetWidth();

					pTextTail->pTitleNameplateMiddle->SetPosition(pTextTail->x - iHalfWidth, fNameplateY);
					pTextTail->pTitleNameplateLeft->SetPosition(pTextTail->x - iHalfWidth - iLeftWidth, fNameplateY);
					pTextTail->pTitleNameplateRight->SetPosition(pTextTail->x + iHalfWidth, fNameplateY);
				}

				if (pTextTail->pSpriteInstance && !pTextTail->vSpriteImages.empty())
			{
				pTextTail->pSpriteInstance->SetPosition(pTextTail->x - (pTextTail->pSpriteInstance->GetWidth() / 2), fTitleY - pTextTail->pSpriteInstance->GetHeight() + 15.0f);
			}
		}
#endif

#ifdef ENABLE_LEFT_SEAT
		CGraphicTextInstance* pLeftSeatTextInstance = pTextTail->pLeftSeatTextInstance;
		if (pLeftSeatTextInstance)
		{
			pLeftSeatTextInstance->SetPosition(pTextTail->x, pTextTail->y - c_fyLeftSeatTextPosition, pTextTail->z);
			pLeftSeatTextInstance->Update();
#ifdef ENABLE_BATTLE_ROYALE
			fyAdd += 15.0f;
#endif
		}
#endif

		int iNameWidth, iNameHeight;
		pTextTail->pTextInstance->GetTextSize(&iNameWidth, &iNameHeight);
#ifdef ENABLE_BATTLE_ROYALE
		if (pTextTail->pBattleRoyaleCrownImageInstance)
		{
			float fX = pTextTail->x - pTextTail->pBattleRoyaleCrownImageInstance->GetWidth() / 2;
			float Fy = pTextTail->y - pTextTail->pBattleRoyaleCrownImageInstance->GetHeight() - fyAdd;
			pTextTail->pBattleRoyaleCrownImageInstance->SetPosition(fX, Fy);
		}
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		CGraphicImageInstance* pLanguageInstance = pTextTail->pLanguageInstance;
#endif

#ifdef ENABLE_BATTLE_FIELD
		if (pCombatZoneRank)
		{
			if (pGuildNameInstance)
			{
				pCombatZoneRank->SetPosition (pTextTail->x - pCombatZoneRank->GetWidth()/2, pTextTail->y - (c_fyMarkPosition * 4) + 15.0f);
			}
			else
			{
				pCombatZoneRank->SetPosition (pTextTail->x - pCombatZoneRank->GetWidth()/2, pTextTail->y - (c_fyMarkPosition * 4) + 30.0f);
			}
		}
#endif

		// Title ?? ????
		CGraphicTextInstance * pTitle = pTextTail->pTitleTextInstance;
		if (pTitle)
		{			
			int iTitleWidth, iTitleHeight;
			pTitle->GetTextSize(&iTitleWidth, &iTitleHeight);

			// fxAdd = 8.0f; Bvural41 Fix
			fxAdd = 3.0f;

			if (LocaleService_IsEUROPE()) // ???? ??? ?? ????
			{
				if( GetDefaultCodePage() == CP_ARABIC )
				{
					pTitle->SetPosition(pTextTail->x - (iNameWidth / 2) - iTitleWidth - 4.0f, pTextTail->y, pTextTail->z);
				}
				else
				{
					pTitle->SetPosition(pTextTail->x - (iNameWidth / 2), pTextTail->y, pTextTail->z);
				}
			}
			else
			{
				pTitle->SetPosition(pTextTail->x - (iNameWidth / 2) - fxAdd, pTextTail->y, pTextTail->z);
			}			
			pTitle->Update();

			// Level ?? ????
			CGraphicTextInstance * pLevel = pTextTail->pLevelTextInstance;
			if (pLevel)
			{
				int iLevelWidth, iLevelHeight;
				pLevel->GetTextSize(&iLevelWidth, &iLevelHeight);
				
				if (LocaleService_IsEUROPE()) // ???? ??? ?? ????
				{
					if( GetDefaultCodePage() == CP_ARABIC )
					{
						pLevel->SetPosition(pTextTail->x - (iNameWidth / 2) - iLevelWidth - iTitleWidth - 8.0f, pTextTail->y, pTextTail->z);
					}
					else
					{
						pLevel->SetPosition(pTextTail->x - (iNameWidth / 2) - iTitleWidth, pTextTail->y, pTextTail->z);
					}
				}
				else
				{
					pLevel->SetPosition(pTextTail->x - (iNameWidth / 2) - fxAdd - iTitleWidth, pTextTail->y, pTextTail->z);
				}

				pLevel->Update();

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
				if (pLanguageInstance)
				{
					int iLevelWidth, iLevelHeight;
					pLevel->GetTextSize(&iLevelWidth, &iLevelHeight);
					pLanguageInstance->SetPosition(pTextTail->x - (iNameWidth / 2) - iTitleWidth - iLevelWidth - pLanguageInstance->GetWidth() - 12.0f, pTextTail->y - 10.0f);
				}
#endif

			}
		}
		else
		{
			// fxAdd = 4.0f; Bvural41 Fix
			fxAdd = 0.0f;

			// Level ?? ????
			CGraphicTextInstance * pLevel = pTextTail->pLevelTextInstance;
			if (pLevel)
			{
				int iLevelWidth, iLevelHeight;
				pLevel->GetTextSize(&iLevelWidth, &iLevelHeight);
				
				if (LocaleService_IsEUROPE()) // ???? ??? ?? ????
				{
					if( GetDefaultCodePage() == CP_ARABIC )
					{
						pLevel->SetPosition(pTextTail->x - (iNameWidth / 2) - iLevelWidth - 4.0f, pTextTail->y, pTextTail->z);
					}
					else
					{
						pLevel->SetPosition(pTextTail->x - (iNameWidth / 2), pTextTail->y, pTextTail->z);
					}
				}
				else
				{
					pLevel->SetPosition(pTextTail->x - (iNameWidth / 2) - fxAdd, pTextTail->y, pTextTail->z);
				}

				pLevel->Update();

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
				if (pLanguageInstance)
				{
					pLanguageInstance->SetPosition(pTextTail->x - (iNameWidth / 2) - iLevelWidth - pLanguageInstance->GetWidth() - 8.0f, pTextTail->y - 10.0f);
				}
#endif

			}
		}

		pTextTail->pTextInstance->SetColor(pTextTail->Color.r, pTextTail->Color.g, pTextTail->Color.b);
		pTextTail->pTextInstance->SetPosition(pTextTail->x + fxAdd, pTextTail->y, pTextTail->z);
		pTextTail->pTextInstance->Update();

#if defined(ENABLE_SHOW_MOB_INFO)
		CGraphicTextInstance * pAIFlag = pTextTail->pAIFlagTextInstance;
		if (pAIFlag)
		{
			pAIFlag->SetColor(pTextTail->Color.r, pTextTail->Color.g, pTextTail->Color.b);
			pAIFlag->SetPosition(pTextTail->x + fxAdd + (iNameWidth / 2) + 1.0f, pTextTail->y, pTextTail->z);//+1.0f is not neccesarry
			pAIFlag->Update();
		}
#endif
	}

	for (TChatTailMap::iterator itorChat=m_ChatTailMap.begin(); itorChat!=m_ChatTailMap.end();)
	{
		TTextTail * pTextTail = itorChat->second;

		if (pTextTail->LivingTime < dwTime)
		{
			DeleteTextTail(pTextTail);
			itorChat = m_ChatTailMap.erase(itorChat);
			continue;
		}
		else
			++itorChat;

		pTextTail->pTextInstance->SetColor(pTextTail->Color);
		pTextTail->pTextInstance->SetPosition(pTextTail->x, pTextTail->y, pTextTail->z);
		pTextTail->pTextInstance->Update();
	}
}

void CPythonTextTail::Render()
{

#ifdef ENABLE_WHITE_DRAGON
	if (UI::CWindowManager::Instance().IsMouseAndKeyboardLocked())
		return;
#endif

	TTextTailList::iterator itor;

#ifdef ENABLE_TITLE_SYSTEM
	STATEMANAGER.SaveRenderState(D3DRS_ZWRITEENABLE, FALSE);
#endif
	for (itor = m_CharacterTextTailList.begin(); itor != m_CharacterTextTailList.end(); ++itor)
	{
		TTextTail * pTextTail = *itor;

#ifdef ENABLE_TITLE_SYSTEM
		if (CPythonSystem::Instance().IsShowTitle() && pTextTail->pSpriteInstance)
		{
			pTextTail->pSpriteInstance->Render();

			pTextTail->fSpriteTimer += CTimer::Instance().GetElapsedSecond();
			if (pTextTail->fSpriteTimer >= 0.05f)
			{
				pTextTail->fSpriteTimer = 0.0f;
				pTextTail->iSpriteCurrentFrame++;
				if (pTextTail->iSpriteCurrentFrame >= pTextTail->vSpriteImages.size())
					pTextTail->iSpriteCurrentFrame = 0;

				pTextTail->pSpriteInstance->SetImagePointer(pTextTail->vSpriteImages[pTextTail->iSpriteCurrentFrame]);
			}
		}
#endif
		pTextTail->pTextInstance->Render();
		if (pTextTail->pMarkInstance && pTextTail->pGuildNameTextInstance)
		{
			pTextTail->pMarkInstance->Render();
			pTextTail->pGuildNameTextInstance->Render();
		}
		if (pTextTail->pTitleTextInstance)
		{
			pTextTail->pTitleTextInstance->Render();
		}

#ifdef ENABLE_TITLE_SYSTEM
		if (CPythonSystem::Instance().IsShowTitle())
		{
			if (pTextTail->pSpriteInstance)
				pTextTail->pSpriteInstance->Render();
			if (pTextTail->pTitleNameplateLeft)
				pTextTail->pTitleNameplateLeft->Render();
			if (pTextTail->pTitleNameplateMiddle)
				pTextTail->pTitleNameplateMiddle->Render();
			if (pTextTail->pTitleNameplateRight)
				pTextTail->pTitleNameplateRight->Render();
			if (pTextTail->pTitleSystemTextInstance)
				pTextTail->pTitleSystemTextInstance->Render();
		}
#endif
#if defined(ENABLE_SHOW_MOB_INFO)
		if (pTextTail->pLevelTextInstance && 
		(pTextTail->bIsPC == TRUE ||
#ifdef ENABLE_GROWTH_PET_SYSTEM
			pTextTail->bIsGrowthPet == TRUE ||
#endif
			CPythonSystem::Instance().IsShowMobLevel()))
#else
		if (pTextTail->pLevelTextInstance)
#endif
		{
			pTextTail->pLevelTextInstance->Render();
		}
#ifdef ENABLE_BATTLE_ROYALE
		if (pTextTail->pBattleRoyaleCrownImageInstance)
		{
			pTextTail->pBattleRoyaleCrownImageInstance->Render();
		}
#endif
#ifdef ENABLE_BATTLE_FIELD
		if (pTextTail->pCombatZoneRankInstance)
		{
			pTextTail->pCombatZoneRankInstance->Render();
		}
#endif
#if defined(ENABLE_SHOW_MOB_INFO)
		if (pTextTail->pAIFlagTextInstance && CPythonSystem::Instance().IsShowMobAIFlag())
		{
			pTextTail->pAIFlagTextInstance->Render();
		}
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		if (pTextTail->pLanguageInstance && (CPythonSystem::Instance().IsShowFlag()))
		{
			pTextTail->pLanguageInstance->Render();
		}
#endif
#ifdef ENABLE_LEFT_SEAT
		if (pTextTail->pLeftSeatTextInstance)
			pTextTail->pLeftSeatTextInstance->Render();
#endif
	}
#ifdef ENABLE_TITLE_SYSTEM
	STATEMANAGER.RestoreRenderState(D3DRS_ZWRITEENABLE);
#endif

	for (itor = m_ItemTextTailList.begin(); itor != m_ItemTextTailList.end(); ++itor)
	{
		TTextTail * pTextTail = *itor;

		RenderTextTailBox(pTextTail);
		pTextTail->pTextInstance->Render();
		if (pTextTail->pOwnerTextInstance)
			pTextTail->pOwnerTextInstance->Render();
	}

	for (TChatTailMap::iterator itorChat = m_ChatTailMap.begin(); itorChat!=m_ChatTailMap.end(); ++itorChat)
	{
		TTextTail * pTextTail = itorChat->second;
		if (pTextTail->pOwner->isShow())
			RenderTextTailName(pTextTail);
	}
}

void CPythonTextTail::RenderTextTailBox(TTextTail * pTextTail)
{
	// ??? ???
	CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 1.0f);
	CPythonGraphic::Instance().RenderBox2d(pTextTail->x + pTextTail->xStart,
										   pTextTail->y + pTextTail->yStart,
										   pTextTail->x + pTextTail->xEnd,
										   pTextTail->y + pTextTail->yEnd,
										   pTextTail->z);

	// ??? ????
	CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 0.3f);
	CPythonGraphic::Instance().RenderBar2d(pTextTail->x + pTextTail->xStart,
										   pTextTail->y + pTextTail->yStart,
										   pTextTail->x + pTextTail->xEnd,
										   pTextTail->y + pTextTail->yEnd,
										   pTextTail->z);
}

void CPythonTextTail::RenderTextTailName(TTextTail * pTextTail)
{
	pTextTail->pTextInstance->Render();
}

void CPythonTextTail::HideAllTextTail()
{
	m_CharacterTextTailList.clear();
	m_ItemTextTailList.clear();
}

void CPythonTextTail::UpdateDistance(const TPixelPosition & c_rCenterPosition, TTextTail * pTextTail)
{
	const D3DXVECTOR3 & c_rv3Position = pTextTail->pOwner->GetPosition();
	D3DXVECTOR2 v2Distance(c_rv3Position.x - c_rCenterPosition.x, -c_rv3Position.y - c_rCenterPosition.y);
	pTextTail->fDistanceFromPlayer = D3DXVec2Length(&v2Distance);
}

void CPythonTextTail::ShowAllTextTail()
{
	TTextTailMap::iterator itor;
	for (itor = m_CharacterTextTailMap.begin(); itor != m_CharacterTextTailMap.end(); ++itor)
	{
		TTextTail * pTextTail = itor->second;
		if (pTextTail->fDistanceFromPlayer < 3500.0f)
			ShowCharacterTextTail(itor->first);
	}
	for (itor = m_ItemTextTailMap.begin(); itor != m_ItemTextTailMap.end(); ++itor)
	{
		TTextTail * pTextTail = itor->second;
		if (pTextTail->fDistanceFromPlayer < 3500.0f)
			ShowItemTextTail(itor->first);
	}
}

void CPythonTextTail::ShowCharacterTextTail(DWORD VirtualID)
{
	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(VirtualID);

	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail * pTextTail = itor->second;

	if (m_CharacterTextTailList.end() != std::find(m_CharacterTextTailList.begin(), m_CharacterTextTailList.end(), pTextTail))
	{
		//Tracef("?? ???? ?? : %d\n", VirtualID);
		return;
	}

	// NOTE : ShowAll ??? ?? Instance ? Pointer ? ??? ????? ??? ?? ???? ??.
	//        CInstanceBase ? TextTail ? ?? ??? ?? ?? ?? ?? ??? ?..
	if (!pTextTail->pOwner->isShow())
		return;
	
	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(pTextTail->dwVirtualID);

#ifdef ENABLE_GRAPHIC_ON_OFF
	if (CPythonSystem::instance().IsNpcNameStatus() == 1)
		if (pInstance->IsNPC())
			return;
#endif

#ifdef ENABLE_EVENT_BANNER_FLAG
	if (pInstance->IsBannerFlag())
		return;
#endif

	if (!pInstance)
		return;

	if (pInstance->IsGuildWall())
		return;

	if (pInstance->CanPickInstance())
		m_CharacterTextTailList.push_back(pTextTail);
}

void CPythonTextTail::ShowItemTextTail(DWORD VirtualID)
{
	TTextTailMap::iterator itor = m_ItemTextTailMap.find(VirtualID);

	if (m_ItemTextTailMap.end() == itor)
		return;

#ifdef ENABLE_GRAPHIC_ON_OFF
	if (CPythonSystem::instance().GetDropItemLevel() >= 2)
		return;
#endif

	TTextTail * pTextTail = itor->second;

	if (m_ItemTextTailList.end() != std::find(m_ItemTextTailList.begin(), m_ItemTextTailList.end(), pTextTail))
	{
		//Tracef("?? ???? ?? : %d\n", VirtualID);
		return;
	}

	m_ItemTextTailList.push_back(pTextTail);
}

bool CPythonTextTail::isIn(CPythonTextTail::TTextTail * pSource, CPythonTextTail::TTextTail * pTarget)
{
	float x1Source = pSource->x + pSource->xStart;
	float y1Source = pSource->y + pSource->yStart;
	float x2Source = pSource->x + pSource->xEnd;
	float y2Source = pSource->y + pSource->yEnd;
	float x1Target = pTarget->x + pTarget->xStart;
	float y1Target = pTarget->y + pTarget->yStart;
	float x2Target = pTarget->x + pTarget->xEnd;
	float y2Target = pTarget->y + pTarget->yEnd;

	if (x1Source <= x2Target && x2Source >= x1Target &&
	    y1Source <= y2Target && y2Source >= y1Target)
	{
		return true;
	}

	return false;
}

void CPythonTextTail::RegisterCharacterTextTail(DWORD dwGuildID, DWORD dwVirtualID, const D3DXCOLOR & c_rColor, float fAddHeight)
{
	CInstanceBase * pCharacterInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwVirtualID);

	if (!pCharacterInstance)
		return;

#ifdef ENABLE_BATTLE_FIELD
	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();
	CInstanceBase* pkInstMain = rkChrMgr.GetMainInstancePtr();

	if (pCharacterInstance->IsCombatZoneMap() && pCharacterInstance != pkInstMain)
		return;
#endif

	float fTotalHeight = fAddHeight;
#ifdef ENABLE_GROWTH_PET_SYSTEM
	if (!pCharacterInstance->IsGrowthPet())
#endif
	{
		fTotalHeight = pCharacterInstance->GetGraphicThingInstanceRef().GetHeight() + fAddHeight;
	}
	
	TTextTail * pTextTail = RegisterTextTail(dwVirtualID,
											 pCharacterInstance->GetNameString(),
											 pCharacterInstance->GetGraphicThingInstancePtr(),
											 fTotalHeight,
											 c_rColor);

	CGraphicTextInstance * pTextInstance = pTextTail->pTextInstance;
	pTextInstance->SetOutline(true); //Bvural41 010921
	pTextInstance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);

	pTextTail->pMarkInstance=NULL;	
	pTextTail->pGuildNameTextInstance=NULL;
	pTextTail->pTitleTextInstance=NULL;
	pTextTail->pLevelTextInstance=NULL;
#ifdef ENABLE_BATTLE_ROYALE
	pTextTail->pBattleRoyaleCrownImageInstance = NULL;

	if (pCharacterInstance->IsAffect(CInstanceBase::AFFECT_BATTLE_ROYALE_CROWN))
	{
		CGraphicImage* pImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(PythonBattleRoyaleManager::Instance().GetCrownImagePath().c_str());
		pTextTail->pBattleRoyaleCrownImageInstance = CGraphicImageInstance::New();
		pTextTail->pBattleRoyaleCrownImageInstance->SetImagePointer(pImage);
	}
#endif
#ifdef ENABLE_BATTLE_FIELD
	pTextTail->pCombatZoneRankInstance=NULL;
#endif
#if defined(ENABLE_SHOW_MOB_INFO)
	pTextTail->pAIFlagTextInstance = NULL;
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	pTextTail->pLanguageInstance = NULL;
#endif
#ifdef ENABLE_LEFT_SEAT
	pTextTail->pLeftSeatTextInstance = NULL;
#endif

	if (0 != dwGuildID)
	{
		pTextTail->pMarkInstance = CGraphicMarkInstance::New();

		DWORD dwMarkID = CGuildMarkManager::Instance().GetMarkID(dwGuildID);

		if (dwMarkID != CGuildMarkManager::INVALID_MARK_ID)
		{
			std::string markImagePath;

			if (CGuildMarkManager::Instance().GetMarkImageFilename(dwMarkID / CGuildMarkImage::MARK_TOTAL_COUNT, markImagePath))
			{
				pTextTail->pMarkInstance->SetImageFileName(markImagePath.c_str());
				pTextTail->pMarkInstance->Load();
				pTextTail->pMarkInstance->SetIndex(dwMarkID % CGuildMarkImage::MARK_TOTAL_COUNT);
			}
		}

		std::string strGuildName;
		if (!CPythonGuild::Instance().GetGuildName(dwGuildID, &strGuildName))
			strGuildName = "Noname";

		CGraphicTextInstance *& prGuildNameInstance = pTextTail->pGuildNameTextInstance;
		prGuildNameInstance = CGraphicTextInstance::New();
		prGuildNameInstance->SetTextPointer(ms_pFont);
		prGuildNameInstance->SetOutline(true);
		prGuildNameInstance->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
		prGuildNameInstance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
		prGuildNameInstance->SetValue(strGuildName.c_str());
		prGuildNameInstance->SetColor(c_TextTail_Guild_Name_Color.r, c_TextTail_Guild_Name_Color.g, c_TextTail_Guild_Name_Color.b);
		prGuildNameInstance->Update();
	}

#if defined(ENABLE_SHOW_MOB_INFO)
	if (IS_SET(pCharacterInstance->GetAIFlag(), CInstanceBase::AIFLAG_AGGRESSIVE))
	{
		CGraphicTextInstance *& prAIFlagInstance = pTextTail->pAIFlagTextInstance;
		prAIFlagInstance = CGraphicTextInstance::New();
		prAIFlagInstance->SetTextPointer(ms_pFont);
		prAIFlagInstance->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_LEFT);
		prAIFlagInstance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
		prAIFlagInstance->SetValue("*");
		prAIFlagInstance->SetOutline(true);
		prAIFlagInstance->SetColor(c_rColor.r, c_rColor.g, c_rColor.b);
		prAIFlagInstance->Update();

	}
	pTextTail->bIsPC = pCharacterInstance->IsPC() != FALSE;
#ifdef ENABLE_GROWTH_PET_SYSTEM
	pTextTail->bIsGrowthPet = pCharacterInstance->IsGrowthPet() != FALSE;
#endif
#endif
#ifdef ENABLE_BATTLE_FIELD
	if (pCharacterInstance->IsPC())
	{
		BYTE iCombatZoneRankID = pCharacterInstance->GetCombatZoneRank();
		switch (iCombatZoneRankID)
		{
			case 1:
			case 2:
			case 3:
			{
				pTextTail->pCombatZoneRankInstance = CGraphicImageInstance::New();
				char c_pszRank[256];
				sprintf(c_pszRank, "d:/ymir work/effect/etc/ranking_battle/ranker_%d.tga", iCombatZoneRankID);
				pTextTail->pCombatZoneRankInstance->SetImagePointer((CGraphicImage*)CResourceManager::Instance().GetResourcePointer(c_pszRank));
			}
			break;
		}
	}
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	CGraphicImageInstance *& prLanguage = pTextTail->pLanguageInstance;

	if (!prLanguage)
	{
		BYTE bLanguage = pCharacterInstance->GetLanguage();
		if(pCharacterInstance->IsPC() && bLanguage)
		{
			std::string langName = "en";
			if (bLanguage == 1)
				langName = "en";
			else if (bLanguage == 2)
				langName = "pt";
			else if (bLanguage == 3)
				langName = "es";
			else if (bLanguage == 4)
				langName = "fr";
			else if (bLanguage == 5)
				langName = "de";
			else if (bLanguage == 6)
				langName = "ro";
			else if (bLanguage == 7)
				langName = "pl";
			else if (bLanguage == 8)
				langName = "it";
			else if (bLanguage == 9)
				langName = "cz";
			else if (bLanguage == 10)
				langName = "hu";
			else if (bLanguage == 11)
				langName = "tr";
			else
				langName = "eu"; // en

			char szFileName[256];
			sprintf(szFileName, "d:/ymir work/ui/game/flag/%s.tga", langName.c_str());

			if (CResourceManager::Instance().IsFileExist(szFileName))
			{
				CGraphicImage * pLanguageImage = (CGraphicImage *)CResourceManager::Instance().GetResourcePointer(szFileName);
				if (pLanguageImage)
				{
					prLanguage = CGraphicImageInstance::New();
					prLanguage->SetImagePointer(pLanguageImage);
				}
			}
		}
	}
#endif

	m_CharacterTextTailMap.insert(TTextTailMap::value_type(dwVirtualID, pTextTail));
#ifdef ENABLE_TITLE_SYSTEM
	if (pCharacterInstance->IsPC())
	{
		auto itPending = m_PendingTitleSystemMap.find(dwVirtualID);
		if (itPending != m_PendingTitleSystemMap.end())
		{
			TTitleSystemData& d = itPending->second;
			AttachTitleSystem(dwVirtualID, d.strName.c_str(), d.color);
			if (!d.strNameplateLeft.empty())
				SetTitleNameplate(dwVirtualID, d.strNameplateLeft.c_str(), d.strNameplateMiddle.c_str(), d.strNameplateRight.c_str());
			if (!d.strSpriteImage.empty() && d.iSpriteFrameCount > 0)
				SetTitleSpriteAnimation(dwVirtualID, d.strSpriteImage.c_str(), d.iSpriteFrameCount, d.iSpriteSizeX, d.iSpriteSizeY, d.iSpriteColumns);
		}
	}
#endif
}

#ifdef ENABLE_ITEM_DROP_RENEWAL
void CPythonTextTail::RegisterItemTextTail(DWORD VirtualID, const char* c_szText, CGraphicObjectInstance* pOwner, bool bHasAttr)
#else
void CPythonTextTail::RegisterItemTextTail(DWORD VirtualID, const char* c_szText, CGraphicObjectInstance* pOwner)
#endif
{
#ifdef __DEBUG
	char szName[256];
	spritnf(szName, "%s[%d]", c_szText, VirtualID);
#endif

	D3DXCOLOR c_d3dColor = c_TextTail_Item_Color;

	TTextTail* pTextTail = RegisterTextTail(VirtualID, c_szText, pOwner, c_TextTail_Name_Position, c_d3dColor);
	m_ItemTextTailMap.insert(TTextTailMap::value_type(VirtualID, pTextTail));
}

void CPythonTextTail::RegisterChatTail(DWORD VirtualID, const char * c_szChat)
{
	CInstanceBase * pCharacterInstance = CPythonCharacterManager::Instance().GetInstancePtr(VirtualID);

	if (!pCharacterInstance)
		return;

	TChatTailMap::iterator itor = m_ChatTailMap.find(VirtualID);

	if (m_ChatTailMap.end() != itor)
	{
		TTextTail * pTextTail = itor->second;

		pTextTail->pTextInstance->SetValue(c_szChat);
		pTextTail->pTextInstance->Update();
		pTextTail->Color = c_TextTail_Chat_Color;
		pTextTail->pTextInstance->SetColor(c_TextTail_Chat_Color);

		// TEXTTAIL_LIVINGTIME_CONTROL
		pTextTail->LivingTime = CTimer::Instance().GetCurrentMillisecond() + TextTail_GetLivingTime();
		// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

		pTextTail->bNameFlag = TRUE;

		return;
	}

	TTextTail* pTextTail = RegisterTextTail(VirtualID, c_szChat, pCharacterInstance->GetGraphicThingInstancePtr(), pCharacterInstance->GetGraphicThingInstanceRef().GetHeight() + 10.0f, c_TextTail_Chat_Color);

	// TEXTTAIL_LIVINGTIME_CONTROL
	pTextTail->LivingTime = CTimer::Instance().GetCurrentMillisecond() + TextTail_GetLivingTime();
	// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

	pTextTail->bNameFlag = TRUE;
	pTextTail->pTextInstance->SetOutline(true); //Bvural41 010921
	pTextTail->pTextInstance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	m_ChatTailMap.insert(TTextTailMap::value_type(VirtualID, pTextTail));
}

void CPythonTextTail::RegisterInfoTail(DWORD VirtualID, const char * c_szChat)
{
	CInstanceBase * pCharacterInstance = CPythonCharacterManager::Instance().GetInstancePtr(VirtualID);

	if (!pCharacterInstance)
		return;

	TChatTailMap::iterator itor = m_ChatTailMap.find(VirtualID);

	if (m_ChatTailMap.end() != itor)
	{
		TTextTail * pTextTail = itor->second;

		pTextTail->pTextInstance->SetValue(c_szChat);
		pTextTail->pTextInstance->Update();
		pTextTail->Color = c_TextTail_Info_Color;
		pTextTail->pTextInstance->SetColor(c_TextTail_Info_Color);

		// TEXTTAIL_LIVINGTIME_CONTROL
		pTextTail->LivingTime = CTimer::Instance().GetCurrentMillisecond() + TextTail_GetLivingTime();
		// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

		pTextTail->bNameFlag = FALSE;

		return;
	}

	TTextTail * pTextTail = RegisterTextTail(VirtualID,
											 c_szChat,
											 pCharacterInstance->GetGraphicThingInstancePtr(),
											 pCharacterInstance->GetGraphicThingInstanceRef().GetHeight() + 10.0f,
											 c_TextTail_Info_Color);

	// TEXTTAIL_LIVINGTIME_CONTROL
	pTextTail->LivingTime = CTimer::Instance().GetCurrentMillisecond() + TextTail_GetLivingTime();
	// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

	pTextTail->bNameFlag = FALSE;
	pTextTail->pTextInstance->SetOutline(false);
	pTextTail->pTextInstance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	m_ChatTailMap.insert(TTextTailMap::value_type(VirtualID, pTextTail));
}

bool CPythonTextTail::GetTextTailPosition(DWORD dwVID, float* px, float* py, float* pz)
{
	TTextTailMap::iterator itorCharacter = m_CharacterTextTailMap.find(dwVID);

	if (m_CharacterTextTailMap.end() == itorCharacter)
	{
		return false;
	}

	TTextTail * pTextTail = itorCharacter->second;
	*px=pTextTail->x;
	*py=pTextTail->y;
	*pz=pTextTail->z;

	return true;
}

bool CPythonTextTail::IsChatTextTail(DWORD dwVID)
{
	TChatTailMap::iterator itorChat = m_ChatTailMap.find(dwVID);

	if (m_ChatTailMap.end() == itorChat)
		return false;

	return true;
}

void CPythonTextTail::SetCharacterTextTailColor(DWORD VirtualID, const D3DXCOLOR & c_rColor)
{
	TTextTailMap::iterator itorCharacter = m_CharacterTextTailMap.find(VirtualID);

	if (m_CharacterTextTailMap.end() == itorCharacter)
		return;

	TTextTail * pTextTail = itorCharacter->second;
	pTextTail->pTextInstance->SetColor(c_rColor);
	pTextTail->Color = c_rColor;
}

void CPythonTextTail::SetItemTextTailOwner(DWORD dwVID, const char * c_szName)
{
	TTextTailMap::iterator itor = m_ItemTextTailMap.find(dwVID);
	if (m_ItemTextTailMap.end() == itor)
		return;

	TTextTail * pTextTail = itor->second;

	if (strlen(c_szName) > 0)
	{
		if (!pTextTail->pOwnerTextInstance)
		{
			pTextTail->pOwnerTextInstance = CGraphicTextInstance::New();
		}

		std::string strName = c_szName;
		static const string & strOwnership = ApplicationStringTable_GetString(IDS_POSSESSIVE_MORPHENE) == "" ? "'s" : ApplicationStringTable_GetString(IDS_POSSESSIVE_MORPHENE);
		strName += strOwnership;

		pTextTail->pOwnerTextInstance->SetTextPointer(ms_pFont);
		pTextTail->pOwnerTextInstance->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
		pTextTail->pOwnerTextInstance->SetValue(strName.c_str());
#ifdef ENABLE_ITEM_DROP_RENEWAL
		CInstanceBase* pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
		if (pInstance)
		{
			if (!strcmp(pInstance->GetNameString(), c_szName))
				pTextTail->pOwnerTextInstance->SetColor(1.0f, 1.0f, 0.0f);
			else
				pTextTail->pOwnerTextInstance->SetColor(1.0f, 0.0f, 0.0f);
		}
#else
		pTextTail->pOwnerTextInstance->SetColor(1.0f, 1.0f, 0.0f);
#endif
		pTextTail->pOwnerTextInstance->Update();

		int xOwnerSize, yOwnerSize;
		pTextTail->pOwnerTextInstance->GetTextSize(&xOwnerSize, &yOwnerSize);
		pTextTail->yStart = -2.0f;
		pTextTail->yEnd += float(yOwnerSize + 4);
		pTextTail->xStart = fMIN(pTextTail->xStart, float(-xOwnerSize / 2 - 1));
		pTextTail->xEnd = fMAX(pTextTail->xEnd, float(xOwnerSize / 2 + 1));
	}
	else
	{
		if (pTextTail->pOwnerTextInstance)
		{
			CGraphicTextInstance::Delete(pTextTail->pOwnerTextInstance);
			pTextTail->pOwnerTextInstance = NULL;
		}

		int xSize, ySize;
		pTextTail->pTextInstance->GetTextSize(&xSize, &ySize);
		pTextTail->xStart	= (float) (-xSize / 2 - 2);
		pTextTail->yStart	= -2.0f;
		pTextTail->xEnd		= (float) (xSize / 2 + 2);
		pTextTail->yEnd		= (float) ySize;
	}
}

void CPythonTextTail::DeleteCharacterTextTail(DWORD VirtualID)
{
	TTextTailMap::iterator itorCharacter = m_CharacterTextTailMap.find(VirtualID);
	TTextTailMap::iterator itorChat = m_ChatTailMap.find(VirtualID);

	if (m_CharacterTextTailMap.end() != itorCharacter)
	{
		DeleteTextTail(itorCharacter->second);
		m_CharacterTextTailMap.erase(itorCharacter);
	}
	else
	{
		Tracenf("CPythonTextTail::DeleteCharacterTextTail - Find VID[%d] Error", VirtualID);
	}

	if (m_ChatTailMap.end() != itorChat)
	{
		DeleteTextTail(itorChat->second);
		m_ChatTailMap.erase(itorChat);
	}

#ifdef ENABLE_TITLE_SYSTEM
	m_PendingTitleSystemMap.erase(VirtualID);
#endif
}

void CPythonTextTail::DeleteItemTextTail(DWORD VirtualID)
{
	TTextTailMap::iterator itor = m_ItemTextTailMap.find(VirtualID);

	if (m_ItemTextTailMap.end() == itor)
	{
		Tracef(" CPythonTextTail::DeleteItemTextTail - None Item Text Tail\n");
		return;
	}

	DeleteTextTail(itor->second);
	m_ItemTextTailMap.erase(itor);
}

CPythonTextTail::TTextTail * CPythonTextTail::RegisterTextTail(DWORD dwVirtualID, const char * c_szText, CGraphicObjectInstance * pOwner, float fHeight, const D3DXCOLOR & c_rColor)
{
	TTextTail * pTextTail = m_TextTailPool.Alloc();

	pTextTail->dwVirtualID = dwVirtualID;
	pTextTail->pOwner = pOwner;
	pTextTail->pTextInstance = CGraphicTextInstance::New();
	pTextTail->pOwnerTextInstance = NULL;
	pTextTail->fHeight = fHeight;

	pTextTail->pTextInstance->SetTextPointer(ms_pFont);
	pTextTail->pTextInstance->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
	pTextTail->pTextInstance->SetValue(c_szText);
	pTextTail->pTextInstance->SetColor(c_rColor.r, c_rColor.g, c_rColor.b);
	pTextTail->pTextInstance->Update();

	int xSize, ySize;
	pTextTail->pTextInstance->GetTextSize(&xSize, &ySize);
	pTextTail->xStart				= (float) (-xSize / 2 - 2);
	pTextTail->yStart				= -2.0f;
	pTextTail->xEnd					= (float) (xSize / 2 + 2);
	pTextTail->yEnd					= (float) ySize;
	pTextTail->Color				= c_rColor;
	pTextTail->fDistanceFromPlayer	= 0.0f;
	pTextTail->x = -100.0f;
	pTextTail->y = -100.0f;
	pTextTail->z = 0.0f;
	pTextTail->pMarkInstance = NULL;
	pTextTail->pGuildNameTextInstance = NULL;
	pTextTail->pTitleTextInstance = NULL;
#ifdef ENABLE_TITLE_SYSTEM
	pTextTail->pTitleSystemTextInstance = NULL;
	pTextTail->pTitleNameplateLeft = NULL;
	pTextTail->pTitleNameplateMiddle = NULL;
	pTextTail->pTitleNameplateRight = NULL;
	pTextTail->pSpriteInstance = NULL;
#endif
	pTextTail->pLevelTextInstance = NULL;
#ifdef ENABLE_BATTLE_ROYALE
	pTextTail->pBattleRoyaleCrownImageInstance = NULL;
#endif
#ifdef ENABLE_BATTLE_FIELD
	pTextTail->pCombatZoneRankInstance = NULL;
#endif
#if defined(ENABLE_SHOW_MOB_INFO)
	pTextTail->pAIFlagTextInstance = NULL;
#endif
#ifdef ENABLE_LEFT_SEAT
	pTextTail->pLeftSeatTextInstance = NULL;
#endif
	return pTextTail;
}

void CPythonTextTail::DeleteTextTail(TTextTail * pTextTail)
{
	if (pTextTail->pTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pTextInstance);
		pTextTail->pTextInstance = NULL;
	}
	if (pTextTail->pOwnerTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pOwnerTextInstance);
		pTextTail->pOwnerTextInstance = NULL;
	}
	if (pTextTail->pMarkInstance)
	{
		CGraphicMarkInstance::Delete(pTextTail->pMarkInstance);
		pTextTail->pMarkInstance = NULL;
	}
	if (pTextTail->pGuildNameTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pGuildNameTextInstance);
		pTextTail->pGuildNameTextInstance = NULL;
	}

#ifdef ENABLE_TITLE_SYSTEM
	if (pTextTail->pTitleSystemTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pTitleSystemTextInstance);
		pTextTail->pTitleSystemTextInstance = NULL;
	}
	if (pTextTail->pTitleNameplateLeft)
	{
		CGraphicImageInstance::Delete(pTextTail->pTitleNameplateLeft);
		pTextTail->pTitleNameplateLeft = NULL;
	}
	if (pTextTail->pTitleNameplateMiddle)
	{
		CGraphicImageInstance::Delete(pTextTail->pTitleNameplateMiddle);
		pTextTail->pTitleNameplateMiddle = NULL;
	}
	if (pTextTail->pTitleNameplateRight)
	{
		CGraphicImageInstance::Delete(pTextTail->pTitleNameplateRight);
		pTextTail->pTitleNameplateRight = NULL;
	}
	if (pTextTail->pSpriteInstance)
	{
		CGraphicImageInstance::Delete(pTextTail->pSpriteInstance);
		pTextTail->pSpriteInstance = NULL;
	}
#endif
	if (pTextTail->pTitleTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pTitleTextInstance);
		pTextTail->pTitleTextInstance = NULL;
	}
	if (pTextTail->pLevelTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pLevelTextInstance);
		pTextTail->pLevelTextInstance = NULL;
	}
#ifdef ENABLE_BATTLE_ROYALE
	if (pTextTail->pBattleRoyaleCrownImageInstance)
	{
		CGraphicImageInstance::Delete(pTextTail->pBattleRoyaleCrownImageInstance);
		pTextTail->pBattleRoyaleCrownImageInstance = NULL;
	}
#endif
#ifdef ENABLE_BATTLE_FIELD
	if (pTextTail->pCombatZoneRankInstance)
	{
		CGraphicImageInstance::Delete(pTextTail->pCombatZoneRankInstance);
		pTextTail->pCombatZoneRankInstance = NULL;
	}
#endif
#if defined(ENABLE_SHOW_MOB_INFO)
	if (pTextTail->pAIFlagTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pAIFlagTextInstance);
		pTextTail->pAIFlagTextInstance = NULL;
	}
#endif
#ifdef ENABLE_LEFT_SEAT
	if (pTextTail->pLeftSeatTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pLeftSeatTextInstance);
		pTextTail->pLeftSeatTextInstance = NULL;
	}
#endif
	m_TextTailPool.Free(pTextTail);
}

int CPythonTextTail::Pick(int ixMouse, int iyMouse)
{
	for (TTextTailMap::iterator itor = m_ItemTextTailMap.begin(); itor != m_ItemTextTailMap.end(); ++itor)
	{
		TTextTail * pTextTail = itor->second;

		if (ixMouse >= pTextTail->x + pTextTail->xStart && ixMouse <= pTextTail->x + pTextTail->xEnd &&
			iyMouse >= pTextTail->y + pTextTail->yStart && iyMouse <= pTextTail->y + pTextTail->yEnd)
		{
			SelectItemName(itor->first);
			return (itor->first);
		}
	}

	return -1;
}

void CPythonTextTail::SelectItemName(DWORD dwVirtualID)
{
	TTextTailMap::iterator itor = m_ItemTextTailMap.find(dwVirtualID);

	if (m_ItemTextTailMap.end() == itor)
		return;

	TTextTail * pTextTail = itor->second;
	pTextTail->pTextInstance->SetColor(0.1f, 0.9f, 0.1f);
}

void CPythonTextTail::AttachTitle(DWORD dwVID, const char * c_szName, const D3DXCOLOR & c_rColor)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail * pTextTail = itor->second;

	CGraphicTextInstance *& prTitle = pTextTail->pTitleTextInstance;
	if (!prTitle)
	{
		prTitle = CGraphicTextInstance::New();
		prTitle->SetTextPointer(ms_pFont);
		prTitle->SetOutline(true);

		if (LocaleService_IsEUROPE())
			prTitle->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_RIGHT);
		else
			prTitle->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
		prTitle->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	}

	prTitle->SetValue(c_szName);
	prTitle->SetColor(c_rColor.r, c_rColor.g, c_rColor.b);
	prTitle->Update();
}

void CPythonTextTail::DetachTitle(DWORD dwVID)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail * pTextTail = itor->second;

	if (pTextTail->pTitleTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pTitleTextInstance);
		pTextTail->pTitleTextInstance = NULL;
	}
}

void CPythonTextTail::EnablePKTitle(BOOL bFlag)
{
	bPKTitleEnable = bFlag;
}

void CPythonTextTail::AttachLevel(DWORD dwVID, const char * c_szText, const D3DXCOLOR & c_rColor)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail * pTextTail = itor->second;

	CGraphicTextInstance *& prLevel = pTextTail->pLevelTextInstance;
	if (!prLevel)
	{
		prLevel = CGraphicTextInstance::New();
		prLevel->SetTextPointer(ms_pFont);
		prLevel->SetOutline(true);

		prLevel->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_RIGHT);
		prLevel->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	}

	prLevel->SetValue(c_szText);
	prLevel->SetColor(c_rColor.r, c_rColor.g, c_rColor.b);
	prLevel->Update();
}

void CPythonTextTail::DetachLevel(DWORD dwVID)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail * pTextTail = itor->second;

	if (pTextTail->pLevelTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pLevelTextInstance);
		pTextTail->pLevelTextInstance = NULL;
	}
}

#ifdef ENABLE_CONQUEROR_LEVEL
void CPythonTextTail::AttachConquerorLevel(DWORD dwVID, const char* c_szText)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;

	CGraphicTextInstance*& prLevel = pTextTail->pLevelTextInstance;
	if (!prLevel)
	{
		prLevel = CGraphicTextInstance::New();
		prLevel->SetTextPointer(ms_pFont);
		prLevel->SetOutline(true);

		prLevel->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_RIGHT);
		prLevel->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	}

	prLevel->SetValue(c_szText);
	prLevel->SetColor(140.0f / 255.0f, 200.0f / 255.0f, 255.0f / 255.0f, 1.0f);
	prLevel->Update();
}
#endif

#ifdef ENABLE_LEFT_SEAT
void CPythonTextTail::AttachLeftSeatText(DWORD dwVID, const std::string& c_rstrText)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;

	CGraphicTextInstance*& prLeftSeatTextInstance = pTextTail->pLeftSeatTextInstance;
	if (!prLeftSeatTextInstance)
	{
		prLeftSeatTextInstance = CGraphicTextInstance::New();
		prLeftSeatTextInstance->SetTextPointer(ms_pFont);
		prLeftSeatTextInstance->SetOutline(true);
		prLeftSeatTextInstance->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
		prLeftSeatTextInstance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	}

	prLeftSeatTextInstance->SetValue("[AFK]");
	prLeftSeatTextInstance->SetColor(c_TextTail_LeftSeat_Text_Color.r, c_TextTail_LeftSeat_Text_Color.g, c_TextTail_LeftSeat_Text_Color.b);
	prLeftSeatTextInstance->Update();
}

void CPythonTextTail::DetachLeftSeatText(DWORD dwVID)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;
	if (pTextTail->pLeftSeatTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pLeftSeatTextInstance);
		pTextTail->pLeftSeatTextInstance = NULL;
	}
}
#endif

void CPythonTextTail::Initialize()
{
	// DEFAULT_FONT
	//ms_pFont = (CGraphicText *)CResourceManager::Instance().GetTypeResourcePointer(g_strDefaultFontName.c_str());

	CGraphicText* pkDefaultFont = static_cast<CGraphicText*>(DefaultFont_GetResource());
	if (!pkDefaultFont)
	{
		TraceError("CPythonTextTail::Initialize - CANNOT_FIND_DEFAULT_FONT");
		return;
	}	

	ms_pFont = pkDefaultFont;
	// END_OF_DEFAULT_FONT
}

void CPythonTextTail::Destroy()
{
	Clear();
}

void CPythonTextTail::Clear()
{
	for (TTextTailMap::iterator itor = m_CharacterTextTailMap.begin(); itor != m_CharacterTextTailMap.end(); ++itor)
		DeleteTextTail(itor->second);
	m_CharacterTextTailMap.clear();
	m_CharacterTextTailList.clear();

	for (TTextTailMap::iterator itor = m_ItemTextTailMap.begin(); itor != m_ItemTextTailMap.end(); ++itor)
		DeleteTextTail(itor->second);
	m_ItemTextTailMap.clear();
	m_ItemTextTailList.clear();

	for (TChatTailMap::iterator itor = m_ChatTailMap.begin(); itor != m_ChatTailMap.end(); ++itor)
		DeleteTextTail(itor->second);
	m_ChatTailMap.clear();

	m_TextTailPool.Clear();
#ifdef ENABLE_TITLE_SYSTEM
	m_PendingTitleSystemMap.clear();
#endif
}

CPythonTextTail::CPythonTextTail()
{
	Clear();
}

CPythonTextTail::~CPythonTextTail()
{
	Destroy();
}

#ifdef ENABLE_TITLE_SYSTEM
void CPythonTextTail::AttachTitleSystem(DWORD dwVID, const char *c_szName, const D3DXCOLOR &c_rColor)
{
	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail *pTextTail = itor->second;

	CGraphicTextInstance *&prTitleSys = pTextTail->pTitleSystemTextInstance;
	if (!prTitleSys)
	{
		prTitleSys = CGraphicTextInstance::New();
		prTitleSys->SetTextPointer(ms_pFont);
		prTitleSys->SetOutline(true);
		prTitleSys->SetHorizonalAlign(
		CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
		prTitleSys->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	}

	prTitleSys->SetValue(c_szName);
	prTitleSys->SetColor(c_rColor.r, c_rColor.g, c_rColor.b);
	prTitleSys->Update();
}

void CPythonTextTail::DetachTitleSystem(DWORD dwVID)
{
	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail *pTextTail = itor->second;

	if (pTextTail->pTitleSystemTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pTitleSystemTextInstance);
		pTextTail->pTitleSystemTextInstance = NULL;
	}
}

void CPythonTextTail::SetTitleNameplate(DWORD dwVID, const char *c_szLeft, const char *c_szMiddle, const char *c_szRight)
{
	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail *pTextTail = itor->second;

	ClearTitleNameplate(dwVID);

	try
	{
		CGraphicImage *pLeftImg = (CGraphicImage *)CResourceManager::Instance().GetResourcePointer(c_szLeft);
		if (pLeftImg)
		{
			pTextTail->pTitleNameplateLeft = CGraphicImageInstance::New();
			pTextTail->pTitleNameplateLeft->SetImagePointer(pLeftImg);
		}

		CGraphicImage *pMidImg = (CGraphicImage *)CResourceManager::Instance().GetResourcePointer(c_szMiddle);
		if (pMidImg)
		{
			pTextTail->pTitleNameplateMiddle = CGraphicImageInstance::New();
			pTextTail->pTitleNameplateMiddle->SetImagePointer(pMidImg);
		}

		CGraphicImage *pRightImg = (CGraphicImage *)CResourceManager::Instance().GetResourcePointer(c_szRight);
		if (pRightImg)
		{
			pTextTail->pTitleNameplateRight = CGraphicImageInstance::New();
			pTextTail->pTitleNameplateRight->SetImagePointer(pRightImg);
		}
	}
	catch (...)
	{
		TraceError("CPythonTextTail::SetTitleNameplate - Failed to load nameplate images");
	}
}

void CPythonTextTail::ClearTitleNameplate(DWORD dwVID)
{
	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail *pTextTail = itor->second;

	if (pTextTail->pTitleNameplateLeft) {
		CGraphicImageInstance::Delete(pTextTail->pTitleNameplateLeft);
		pTextTail->pTitleNameplateLeft = NULL;
	}
	if (pTextTail->pTitleNameplateMiddle) {
		CGraphicImageInstance::Delete(pTextTail->pTitleNameplateMiddle);
		pTextTail->pTitleNameplateMiddle = NULL;
	}
	if (pTextTail->pTitleNameplateRight) {
		CGraphicImageInstance::Delete(pTextTail->pTitleNameplateRight);
		pTextTail->pTitleNameplateRight = NULL;
	}
}

void CPythonTextTail::SetTitleSpriteAnimation(DWORD dwVID, const char* c_szBasePath, int iFrameCount, int iSizeX, int iSizeY, int iColumns)
{
	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;

	if (pTextTail->pSpriteInstance)
	{
		CGraphicImageInstance::Delete(pTextTail->pSpriteInstance);
		pTextTail->pSpriteInstance = NULL;
	}
	pTextTail->vSpriteImages.clear();
	pTextTail->iSpriteCurrentFrame = 0;
	pTextTail->fSpriteTimer = 0.0f;

	if (!c_szBasePath || strlen(c_szBasePath) == 0 || iFrameCount <= 0)
		return;

	if (iFrameCount == 1 && strstr(c_szBasePath, "."))
	{
		try
		{
			CGraphicImage* pImg = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(c_szBasePath);
			if (pImg)
			{
				pTextTail->vSpriteImages.push_back(pImg);
				pTextTail->pSpriteInstance = CGraphicImageInstance::New();
				pTextTail->pSpriteInstance->SetImagePointer(pImg);
			}
		}
		catch (...)
		{
			TraceError("SetTitleSpriteAnimation: Failed to load direct image %s", c_szBasePath);
		}
		return;
	}

	for (int i = 0; i < iFrameCount; ++i)
	{
		char szFramePath[256];
		snprintf(szFramePath, sizeof(szFramePath), "%s%02d.sub", c_szBasePath, i);

		try
		{
			CGraphicImage* pImg = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(szFramePath);
			if (pImg)
				pTextTail->vSpriteImages.push_back(pImg);
		}
		catch (...)
		{
		}
	}

	if (!pTextTail->vSpriteImages.empty())
	{
		pTextTail->pSpriteInstance = CGraphicImageInstance::New();
		pTextTail->pSpriteInstance->SetImagePointer(pTextTail->vSpriteImages[0]);
	}
}

void CPythonTextTail::ClearTitleSpriteAnimation(DWORD dwVID)
{
	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;
	if (pTextTail->pSpriteInstance)
	{
		CGraphicImageInstance::Delete(pTextTail->pSpriteInstance);
		pTextTail->pSpriteInstance = NULL;
	}
	pTextTail->vSpriteImages.clear();
	pTextTail->iSpriteCurrentFrame = 0;
	pTextTail->fSpriteTimer = 0.0f;
}

void CPythonTextTail::ClearTitleVisuals(TTextTail* pTextTail)
{
	if (!pTextTail)
		return;

	if (pTextTail->pTitleSystemTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pTitleSystemTextInstance);
		pTextTail->pTitleSystemTextInstance = NULL;
	}
	if (pTextTail->pTitleNameplateLeft)
	{
		CGraphicImageInstance::Delete(pTextTail->pTitleNameplateLeft);
		pTextTail->pTitleNameplateLeft = NULL;
	}
	if (pTextTail->pTitleNameplateMiddle)
	{
		CGraphicImageInstance::Delete(pTextTail->pTitleNameplateMiddle);
		pTextTail->pTitleNameplateMiddle = NULL;
	}
	if (pTextTail->pTitleNameplateRight)
	{
		CGraphicImageInstance::Delete(pTextTail->pTitleNameplateRight);
		pTextTail->pTitleNameplateRight = NULL;
	}
	if (pTextTail->pSpriteInstance)
	{
		CGraphicImageInstance::Delete(pTextTail->pSpriteInstance);
		pTextTail->pSpriteInstance = NULL;
	}
	pTextTail->vSpriteImages.clear();
	pTextTail->iSpriteCurrentFrame = 0;
	pTextTail->fSpriteTimer = 0.0f;
}

void CPythonTextTail::SetTitleSystemData(DWORD dwVID, const char* c_szName, const D3DXCOLOR& c_rColor, const char* c_szNameplateLeft, const char* c_szNameplateMiddle, const char* c_szNameplateRight,
	const char* c_szSpriteImage, int iSpriteFrameCount, int iSpriteSizeX, int iSpriteSizeY, int iSpriteColumns)
{
	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() != itor)
		ClearTitleVisuals(itor->second);

	m_PendingTitleSystemMap.erase(dwVID);

	TTitleSystemData& titleData = m_PendingTitleSystemMap[dwVID];
	titleData.strName = c_szName ? c_szName : "";
	titleData.color = c_rColor;
	titleData.strNameplateLeft = c_szNameplateLeft ? c_szNameplateLeft : "";
	titleData.strNameplateMiddle = c_szNameplateMiddle ? c_szNameplateMiddle : "";
	titleData.strNameplateRight = c_szNameplateRight ? c_szNameplateRight : "";
	titleData.strSpriteImage = c_szSpriteImage ? c_szSpriteImage : "";
	titleData.iSpriteFrameCount = iSpriteFrameCount;
	titleData.iSpriteSizeX = iSpriteSizeX;
	titleData.iSpriteSizeY = iSpriteSizeY;
	titleData.iSpriteColumns = iSpriteColumns;

	if (m_CharacterTextTailMap.end() != itor)
	{
		AttachTitleSystem(dwVID, titleData.strName.c_str(), c_rColor);

		if (!titleData.strNameplateLeft.empty())
			SetTitleNameplate(dwVID, titleData.strNameplateLeft.c_str(), titleData.strNameplateMiddle.c_str(), titleData.strNameplateRight.c_str());

		if (!titleData.strSpriteImage.empty() && iSpriteFrameCount > 0)
			SetTitleSpriteAnimation(dwVID, titleData.strSpriteImage.c_str(), iSpriteFrameCount, iSpriteSizeX, iSpriteSizeY, iSpriteColumns);
	}
}

void CPythonTextTail::ClearTitleSystemData(DWORD dwVID)
{
	m_PendingTitleSystemMap.erase(dwVID);

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() != itor)
		ClearTitleVisuals(itor->second);
}
#endif
