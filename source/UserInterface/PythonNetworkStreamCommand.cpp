#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "PythonNonPlayer.h"
#include "AbstractApplication.h"
#include "AbstractPlayer.h"
#include "AbstractCharacterManager.h"
#include "AbstractChat.h"
#include "InstanceBase.h"
#ifdef ENABLE_BALATHOR_DUNGEON
#include "PythonPlayer.h"
#endif
#ifdef ENABLE_WHITE_DRAGON
#include "PythonBackground.h"
#endif

#define ishan(ch)		(((ch) & 0xE0) > 0x90)
#define ishanasc(ch)	(isascii(ch) || ishan(ch))
#define ishanalp(ch)	(isalpha(ch) || ishan(ch))
#define isnhdigit(ch)	(!ishan(ch) && isdigit(ch))
#define isnhspace(ch)	(!ishan(ch) && isspace(ch))

#define LOWER(c)		(((c) >= 'A' && (c) <= 'Z') ? ((c) + ('a' - 'A')) : (c))
#define UPPER(c)		(((c) >= 'a' && (c) <= 'z') ? ((c) + ('A' - 'a')) : (c))

#ifdef ENABLE_OFFLINESHOP_SYSTEM
#include "PythonShop.h"
#endif
#ifdef ENABLE_TITLE_SYSTEM
#include "PythonTitleManager.h"
#endif

void SkipSpaces(char **string)
{
    for (; **string != '\0' && isnhspace((unsigned char) **string); ++(*string));
}

char *OneArgument(char *argument, char *first_arg)
{
    char mark = FALSE;

    if (!argument)
    {
        *first_arg = '\0';
        return NULL;
    }

    SkipSpaces(&argument);

    while (*argument)
    {
        if (*argument == '\"')
        {
            mark = !mark;
            ++argument;
            continue;
        }

        if (!mark && isnhspace((unsigned char) *argument))
            break;

		*(first_arg++) = LOWER(*argument);
		++argument;
    }

    *first_arg = '\0';

    SkipSpaces(&argument);
    return (argument);
}

void AppendMonsterList(const CPythonNonPlayer::TMobTableList & c_rMobTableList, const char * c_szHeader, int iType)
{
	DWORD dwMonsterCount = 0;
	std::string strMonsterList = c_szHeader;

	CPythonNonPlayer::TMobTableList::const_iterator itor = c_rMobTableList.begin();
	for (; itor!=c_rMobTableList.end(); ++itor)
	{
		const CPythonNonPlayer::TMobTable * c_pMobTable = *itor;
		if (iType == c_pMobTable->bRank)
		{
			if (dwMonsterCount != 0)
				strMonsterList += ", ";
			strMonsterList += c_pMobTable->szLocaleName;
			if (++dwMonsterCount > 5)
				break;
		}
	}
	if (dwMonsterCount > 0)
	{
		IAbstractChat& rkChat=IAbstractChat::GetSingleton();
		rkChat.AppendChat(CHAT_TYPE_INFO, strMonsterList.c_str());
	}
}

bool CPythonNetworkStream::ClientCommand(const char * c_szCommand)
{
	return false;
}

bool SplitToken(const char * c_szLine, CTokenVector * pstTokenVector, const char * c_szDelimeter = " ")
{
	pstTokenVector->reserve(10);
	pstTokenVector->clear();

	std::string stToken;
	std::string strLine = c_szLine;

	DWORD basePos = 0;

	do
	{
		int beginPos = strLine.find_first_not_of(c_szDelimeter, basePos);
		if (beginPos < 0)
			return false;

		int endPos;

		if (strLine[beginPos] == '"')
		{
			++beginPos;
			endPos = strLine.find_first_of("\"", beginPos);

			if (endPos < 0)
				return false;
			
			basePos = endPos + 1;
		}
		else
		{
			endPos = strLine.find_first_of(c_szDelimeter, beginPos);
			basePos = endPos;
		}

		pstTokenVector->push_back(strLine.substr(beginPos, endPos - beginPos));

		// 추가 코드. 맨뒤에 탭이 있는 경우를 체크한다. - [levites]
		if (int(strLine.find_first_not_of(c_szDelimeter, basePos)) < 0)
			break;
	} while (basePos < strLine.length());

	return true;
}

void CPythonNetworkStream::ServerCommand(char * c_szCommand)
{
	// #0000811: [M2EU] 콘솔창 기능 차단 
	if (strcmpi(c_szCommand, "ConsoleEnable") == 0)
		return;

	if (m_apoPhaseWnd[PHASE_WINDOW_GAME])
	{
		bool isTrue;
		if (PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], 
			"BINARY_ServerCommand_Run", 
			Py_BuildValue("(s)", c_szCommand),
			&isTrue
		))
		{
			if (isTrue)
				return;
		}
	}
	else if (m_poSerCommandParserWnd)
	{
		bool isTrue;
		if (PyCallClassMemberFunc(m_poSerCommandParserWnd, 
			"BINARY_ServerCommand_Run", 
			Py_BuildValue("(s)", c_szCommand),
			&isTrue
		))
		{
			if (isTrue)
				return;
		}
	}

	CTokenVector TokenVector;
	if (!SplitToken(c_szCommand, &TokenVector))
		return;
	if (TokenVector.empty())
		return;

	const char * szCmd = TokenVector[0].c_str();
	
	if (!strcmpi(szCmd, "quit"))
	{
		IAbstractApplication::GetSingleton().Exit();
	}
	else if (!strcmpi(szCmd, "BettingMoney"))
	{
		if (2 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		//UINT uMoney= atoi(TokenVector[1].c_str());		
		
	}
	// GIFT NOTIFY
	else if (!strcmpi(szCmd, "gift"))
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "Gift_Show", Py_BuildValue("()")); 
	}

#ifdef ENABLE_WHITE_DRAGON
	else if (!strcmpi(szCmd, "ClearWhiteDragonEffect"))
	{
		CPythonBackground::Instance().DeleteWhiteDragonEffect();
	}
	else if (!strcmpi(szCmd, "AddWhiteDragonEffect"))
	{
		if (4 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %d", c_szCommand, TokenVector.size());
			return;
		}
		CPythonBackground::Instance().CreateWhiteDragonEffect((BYTE)atoi(TokenVector[1].c_str()), atol(TokenVector[2].c_str()), atol(TokenVector[3].c_str()));
	}
#endif

#ifdef ENABLE_CONQUEROR_LEVEL
	else if (!strcmpi(szCmd, "SungMaAttr"))
	{
		if (TokenVector.size() < 4)
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		BYTE yStr = atoi(TokenVector[1].c_str());
		BYTE yHp = atoi(TokenVector[2].c_str());
		BYTE yMove = atoi(TokenVector[3].c_str());
		BYTE yImmune = atoi(TokenVector[4].c_str());

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SungMaAttr", Py_BuildValue("(iiii)", yStr, yHp, yMove, yImmune));
	}
#endif

	// CUBE
	else if (!strcmpi(szCmd, "cube"))
	{
		if (TokenVector.size() < 2)
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		if ("open" == TokenVector[1])
		{
			if (3 > TokenVector.size())
			{
				TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
				return;
			}

			DWORD npcVNUM = (DWORD)atoi(TokenVector[2].c_str());
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cube_Open", Py_BuildValue("(i)", npcVNUM));
		}
		else if ("close" == TokenVector[1])
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cube_Close", Py_BuildValue("()"));
		}
		else if ("info" == TokenVector[1])
		{
			if (5 != TokenVector.size())
			{
				TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
				return;
			}

			UINT gold = atoi(TokenVector[2].c_str());
			UINT itemVnum = atoi(TokenVector[3].c_str());
			UINT count = atoi(TokenVector[4].c_str());
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cube_UpdateInfo", Py_BuildValue("(iii)", gold, itemVnum, count));
		}
		else if ("success" == TokenVector[1])
		{
			if (4 != TokenVector.size())
			{
				TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
				return;
			}

			UINT itemVnum = atoi(TokenVector[2].c_str());
			UINT count = atoi(TokenVector[3].c_str());
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cube_Succeed", Py_BuildValue("(ii)", itemVnum, count));
		}
		else if ("fail" == TokenVector[1])
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cube_Failed", Py_BuildValue("()"));
		}
		else if ("r_list" == TokenVector[1])
		{
			// result list (/cube r_list npcVNUM resultCount resultText)
			// 20383 4 72723,1/72725,1/72730.1/50001,5 <- 이런식으로 "/" 문자로 구분된 리스트를 줌
			if (5 != TokenVector.size())
			{
				TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %d", c_szCommand, 5);
				return;
			}

			DWORD npcVNUM = (DWORD)atoi(TokenVector[2].c_str());

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cube_ResultList", Py_BuildValue("(is)", npcVNUM, TokenVector[4].c_str()));
		}
		else if ("m_info" == TokenVector[1])
		{
			// material list (/cube m_info requestStartIndex resultCount MaterialText)
			// ex) requestStartIndex: 0, resultCount : 5 - 해당 NPC가 만들수 있는 아이템 중 0~4번째에 해당하는 아이템을 만드는 데 필요한 모든 재료들이 MaterialText에 들어있음
			// 위 예시처럼 아이템이 다수인 경우 구분자 "@" 문자를 사용
			// 0 5 125,1|126,2|127,2|123,5&555,5&555,4/120000 <- 이런식으로 서버에서 클라로 리스트를 줌

			if (5 != TokenVector.size())
			{
				TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %d", c_szCommand, 5);
				return;
			}

			UINT requestStartIndex = (UINT)atoi(TokenVector[2].c_str());
			UINT resultCount = (UINT)atoi(TokenVector[3].c_str());

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cube_MaterialInfo", Py_BuildValue("(iis)", requestStartIndex, resultCount, TokenVector[4].c_str()));
		}
	}
	else if (!strcmpi(szCmd, "AttrTransfer"))
	{
		if (TokenVector.size() < 2)
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}
		
		if ("open" == TokenVector[1])
		{
			if (TokenVector.size() < 2)
			{
				TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
				return;
			}
			
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AttrTransfer_Open", Py_BuildValue("()"));
		}
		else if ("close" == TokenVector[1])
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AttrTransfer_Close", Py_BuildValue("()"));
		}
		else if ("success" == TokenVector[1])
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AttrTransfer_Success", Py_BuildValue("()"));
		}
	}
	// CUEBE_END
	else if (!strcmpi(szCmd, "ObserverCount"))
	{
		if (2 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		UINT uObserverCount= atoi(TokenVector[1].c_str());				

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], 
			"BINARY_BettingGuildWar_UpdateObserverCount", 
			Py_BuildValue("(i)", uObserverCount)
		);
	}
	else if (!strcmpi(szCmd, "UserCount"))
	{
		if (2 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		UINT uUserCount = atoi(TokenVector[1].c_str());
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_UpdateUserCount", Py_BuildValue("(i)", uUserCount));
	}
	else if (!strcmpi(szCmd, "ObserverMode"))
	{		
		if (2 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		UINT uMode= atoi(TokenVector[1].c_str());
		
		IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
		rkPlayer.SetObserverMode(uMode ? true : false);

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], 
			"BINARY_BettingGuildWar_SetObserverMode", 
			Py_BuildValue("(i)", uMode)
		);
	}
#ifdef ENABLE_OFFLINESHOP_SYSTEM
	else if (!strcmpi(szCmd, "OfflineShopSetFlag"))
	{
		unsigned long long flag = atoi(TokenVector[1].c_str());
		CPythonShop::Instance().SetShopFlag(flag);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshOfflineShop", Py_BuildValue("()"));
	}
#endif
	else if (!strcmpi(szCmd, "ObserverTeamInfo"))
	{
	}
#ifdef ENABLE_BATTLE_ROYALE
	else if (!strcmpi(szCmd, "SetMinimapTimer"))
	{
		if (2 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetMinimapTimer", Py_BuildValue("(i)", atoi(TokenVector[1].c_str())));
	}
#endif
#ifdef ENABLE_REFINE_FAIL_TYPE
	else if (!strcmpi(szCmd, "RefineFailedType"))
	{
		if (2 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		const BYTE bType = atoi(TokenVector[1].c_str());
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_RefineFailedTypeMessage", Py_BuildValue("(i)", bType));	
	}
#endif
	else if (!strcmpi(szCmd, "cards"))
	{
		if (TokenVector.size() < 2)
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}
		if ("open" == TokenVector[1])
		{
			DWORD safemode = atoi(TokenVector[2].c_str());
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cards_Open", Py_BuildValue("(i)", safemode));
		}
		else if ("info" == TokenVector[1])
		{
			if (14 != TokenVector.size())
			{
				TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
				return;
			}
			DWORD card_in_hand_1 = atoi(TokenVector[2].c_str());
			DWORD card_in_hand_1_v = atoi(TokenVector[3].c_str());
			DWORD card_in_hand_2 = atoi(TokenVector[4].c_str());
			DWORD card_in_hand_2_v = atoi(TokenVector[5].c_str());
			DWORD card_in_hand_3 = atoi(TokenVector[6].c_str());
			DWORD card_in_hand_3_v = atoi(TokenVector[7].c_str());
			DWORD card_in_hand_4 = atoi(TokenVector[8].c_str());
			DWORD card_in_hand_4_v = atoi(TokenVector[9].c_str());
			DWORD card_in_hand_5 = atoi(TokenVector[10].c_str());
			DWORD card_in_hand_5_v = atoi(TokenVector[11].c_str());
			DWORD cards_left = atoi(TokenVector[12].c_str());
			DWORD points = atoi(TokenVector[13].c_str());
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cards_UpdateInfo", Py_BuildValue("(iiiiiiiiiiii)", card_in_hand_1, card_in_hand_1_v, card_in_hand_2, card_in_hand_2_v,
																								card_in_hand_3, card_in_hand_3_v, card_in_hand_4, card_in_hand_4_v, card_in_hand_5, card_in_hand_5_v,
																								cards_left, points));
		}
		else if ("finfo" == TokenVector[1])
		{
			if (9 != TokenVector.size())
			{
				TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
				return;
			}
			DWORD hand_1 = atoi(TokenVector[2].c_str());
			DWORD hand_1_v = atoi(TokenVector[3].c_str());
			DWORD hand_2 = atoi(TokenVector[4].c_str());
			DWORD hand_2_v = atoi(TokenVector[5].c_str());
			DWORD hand_3 = atoi(TokenVector[6].c_str());
			DWORD hand_3_v = atoi(TokenVector[7].c_str());
			DWORD points = atoi(TokenVector[8].c_str());
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cards_FieldUpdateInfo", Py_BuildValue("(iiiiiii)", hand_1, hand_1_v, hand_2, hand_2_v, hand_3, hand_3_v, points));
		}
		else if ("reward" == TokenVector[1])
		{
			if (9 != TokenVector.size())
			{
				TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
				return;
			}
			DWORD hand_1 = atoi(TokenVector[2].c_str());
			DWORD hand_1_v = atoi(TokenVector[3].c_str());
			DWORD hand_2 = atoi(TokenVector[4].c_str());
			DWORD hand_2_v = atoi(TokenVector[5].c_str());
			DWORD hand_3 = atoi(TokenVector[6].c_str());
			DWORD hand_3_v = atoi(TokenVector[7].c_str());
			DWORD points = atoi(TokenVector[8].c_str());
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cards_PutReward", Py_BuildValue("(iiiiiii)", hand_1, hand_1_v, hand_2, hand_2_v, hand_3, hand_3_v, points));
		}
		else if ("icon" == TokenVector[1])
		{
			if (2 != TokenVector.size())
			{
				TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
				return;
			}
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Cards_ShowIcon", Py_BuildValue("()"));
		}
	}
#ifdef ENABLE_12ZI
	else if (!strcmpi(szCmd, "ZodiacTime"))
	{
		if (5 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		BYTE current = atoi(TokenVector[1].c_str());
		BYTE next = atoi(TokenVector[2].c_str());
		int time = atoi(TokenVector[3].c_str());
		int elapseTime = atoi(TokenVector[4].c_str());

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "Refresh12ziTimer", Py_BuildValue("(bbii)", current, next, time, elapseTime));
	}
	else if (!strcmpi(szCmd, "ZodiacTimeClear"))
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "Hide12ziTimer", Py_BuildValue("()"));
	}
	else if (!strcmpi(szCmd, "Bead_count"))
	{
		if (2 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		int count = atoi(TokenVector[1].c_str());
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetBeadCount", Py_BuildValue("(i)", count));
	}
	else if (!strcmpi(szCmd, "OpenReviveDialog"))
	{
		if (3 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		DWORD vid = atoi(TokenVector[1].c_str());
		BYTE count = atoi(TokenVector[2].c_str());
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenReviveDialog", Py_BuildValue("(ib)", vid, count));
	}
	else if (!strcmpi(szCmd, "NotEnoughPrism"))
	{
		if (2 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		BYTE count = atoi(TokenVector[1].c_str());
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "NotEnoughPrism", Py_BuildValue("(b)", count));
	}
#endif
	else if (!strcmpi(szCmd, "StoneDetect"))
	{
		if (4 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		// vid distance(1-3) angle(0-360)
		DWORD dwVID = atoi(TokenVector[1].c_str());
		BYTE byDistance = atoi(TokenVector[2].c_str());
		float fAngle = atof(TokenVector[3].c_str());
		fAngle = fmod(540.0f - fAngle, 360.0f);
		Tracef("StoneDetect [VID:%d] [Distance:%d] [Angle:%d->%f]\n", dwVID, byDistance, atoi(TokenVector[3].c_str()), fAngle);

		IAbstractCharacterManager& rkChrMgr=IAbstractCharacterManager::GetSingleton();

		CInstanceBase * pInstance = rkChrMgr.GetInstancePtr(dwVID);
		if (!pInstance)
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Not Exist Instance", c_szCommand);
			return;
		}

		TPixelPosition PixelPosition;
		D3DXVECTOR3 v3Rotation(0.0f, 0.0f, fAngle);
		pInstance->NEW_GetPixelPosition(&PixelPosition);

		PixelPosition.y *= -1.0f;

		switch (byDistance)
		{
			case 0:
				CEffectManager::Instance().RegisterEffect("d:/ymir work/effect/etc/firecracker/find_out.mse");
				CEffectManager::Instance().CreateEffect("d:/ymir work/effect/etc/firecracker/find_out.mse", PixelPosition, v3Rotation);
				break;
			case 1:
				CEffectManager::Instance().RegisterEffect("d:/ymir work/effect/etc/compass/appear_small.mse");
				CEffectManager::Instance().CreateEffect("d:/ymir work/effect/etc/compass/appear_small.mse", PixelPosition, v3Rotation);
				break;
			case 2:
				CEffectManager::Instance().RegisterEffect("d:/ymir work/effect/etc/compass/appear_middle.mse");
				CEffectManager::Instance().CreateEffect("d:/ymir work/effect/etc/compass/appear_middle.mse", PixelPosition, v3Rotation);
				break;
			case 3:
				CEffectManager::Instance().RegisterEffect("d:/ymir work/effect/etc/compass/appear_large.mse");
				CEffectManager::Instance().CreateEffect("d:/ymir work/effect/etc/compass/appear_large.mse", PixelPosition, v3Rotation);
				break;
			default:
				TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Distance", c_szCommand);
				break;
		}

#ifdef _DEBUG
		IAbstractChat& rkChat=IAbstractChat::GetSingleton();
		rkChat.AppendChat(CHAT_TYPE_INFO, c_szCommand);
#endif
	}
	else if (!strcmpi(szCmd, "StartStaminaConsume"))
	{
		if (3 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}

		DWORD dwConsumePerSec = atoi(TokenVector[1].c_str());
		DWORD dwCurrentStamina = atoi(TokenVector[2].c_str());

		IAbstractPlayer& rPlayer=IAbstractPlayer::GetSingleton();
		rPlayer.StartStaminaConsume(dwConsumePerSec, dwCurrentStamina);
	}
	
	else if (!strcmpi(szCmd, "StopStaminaConsume"))
	{
		if (2 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %d", c_szCommand, TokenVector.size());
			return;
		}

		DWORD dwCurrentStamina = atoi(TokenVector[1].c_str());

		IAbstractPlayer& rPlayer=IAbstractPlayer::GetSingleton();
		rPlayer.StopStaminaConsume(dwCurrentStamina);
	}
	else if (!strcmpi(szCmd, "sms"))
	{
		IAbstractPlayer& rPlayer=IAbstractPlayer::GetSingleton();
		rPlayer.SetMobileFlag(TRUE);
	}
	else if (!strcmpi(szCmd, "nosms"))
	{
		IAbstractPlayer& rPlayer=IAbstractPlayer::GetSingleton();
		rPlayer.SetMobileFlag(FALSE);
	}
	else if (!strcmpi(szCmd, "mobile_auth"))
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnMobileAuthority", Py_BuildValue("()"));
	}
	else if (!strcmpi(szCmd, "messenger_auth"))
	{
		const std::string & c_rstrName = TokenVector[1].c_str();
#ifdef ENABLE_MESSENGER_RENEWAL
		if (TokenVector.size() >= 5)
		{
			BYTE bLevel = (BYTE)atoi(TokenVector[2].c_str());
			BYTE bChannel = (BYTE)atoi(TokenVector[3].c_str());
			long lMapIndex = (long)atoi(TokenVector[4].c_str());
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnMessengerAddFriendQuestion", Py_BuildValue("(siii)", c_rstrName.c_str(), bLevel, bChannel, lMapIndex));
		}
		else
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnMessengerAddFriendQuestion", Py_BuildValue("(s)", c_rstrName.c_str()));
		}
#else
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnMessengerAddFriendQuestion", Py_BuildValue("(s)", c_rstrName.c_str()));
#endif
	}
#ifdef ENABLE_BALATHOR_DUNGEON
	else if (!strcmpi(szCmd, "BalathorEffectRemove"))
	{
		if (2 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}
		CPythonPlayer::Instance().RemoveBalathorEffect(static_cast<DWORD>(atoi(TokenVector[1].c_str())));
	}
	else if (!strcmpi(szCmd, "BalathorEffectAdd"))
	{
		if (9 != TokenVector.size())
		{
			TraceError("CPythonNetworkStream::ServerCommand(c_szCommand=%s) - Strange Parameter Count : %s", c_szCommand);
			return;
		}
		const DWORD dwID = static_cast<DWORD>(atoi(TokenVector[1].c_str()));
		const DWORD dwVID = static_cast<DWORD>(atoi(TokenVector[2].c_str()));
		const BYTE bEffIndex = static_cast<BYTE>(atoi(TokenVector[3].c_str()));
		const float fX = static_cast<float>(atof(TokenVector[4].c_str()));
		const float fY = static_cast<float>(atof(TokenVector[5].c_str()));
		const float fZ = static_cast<float>(atof(TokenVector[6].c_str()));
		const float fRotation = static_cast<float>(atof(TokenVector[7].c_str()));
		const bool bIsMain = static_cast<bool>(atoi(TokenVector[8].c_str()));
		CPythonPlayer::Instance().AddBalathorEffect(dwID, dwVID, bEffIndex, fX, fY, fZ, fRotation, bIsMain);
	}
#endif
	else if (!strcmpi(szCmd, "combo"))
	{
		int iFlag = atoi(TokenVector[1].c_str());
		IAbstractPlayer& rPlayer=IAbstractPlayer::GetSingleton();
		rPlayer.SetComboSkillFlag(iFlag);
		m_bComboSkillFlag = iFlag ? true : false;
	}
	else if (!strcmpi(szCmd, "setblockmode"))
	{
		int iFlag = atoi(TokenVector[1].c_str());
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnBlockMode", Py_BuildValue("(i)", iFlag));
	}
	// Emotion Start
	else if (!strcmpi(szCmd, "french_kiss"))
	{
		int iVID1 = atoi(TokenVector[1].c_str());
		int iVID2 = atoi(TokenVector[2].c_str());

		IAbstractCharacterManager & rkChrMgr = IAbstractCharacterManager::GetSingleton();
		CInstanceBase * pInstance1 = rkChrMgr.GetInstancePtr(iVID1);
		CInstanceBase * pInstance2 = rkChrMgr.GetInstancePtr(iVID2);
		if (pInstance1 && pInstance2)
			pInstance1->ActDualEmotion(*pInstance2, CRaceMotionData::NAME_FRENCH_KISS_START, CRaceMotionData::NAME_FRENCH_KISS_START);
	}
	else if (!strcmpi(szCmd, "kiss"))
	{
		int iVID1 = atoi(TokenVector[1].c_str());
		int iVID2 = atoi(TokenVector[2].c_str());

		IAbstractCharacterManager & rkChrMgr = IAbstractCharacterManager::GetSingleton();
		CInstanceBase * pInstance1 = rkChrMgr.GetInstancePtr(iVID1);
		CInstanceBase * pInstance2 = rkChrMgr.GetInstancePtr(iVID2);
		if (pInstance1 && pInstance2)
			pInstance1->ActDualEmotion(*pInstance2, CRaceMotionData::NAME_KISS_START, CRaceMotionData::NAME_KISS_START);
	}
	else if (!strcmpi(szCmd, "slap"))
	{
		int iVID1 = atoi(TokenVector[1].c_str());
		int iVID2 = atoi(TokenVector[2].c_str());

		IAbstractCharacterManager & rkChrMgr = IAbstractCharacterManager::GetSingleton();
		CInstanceBase * pInstance1 = rkChrMgr.GetInstancePtr(iVID1);
		CInstanceBase * pInstance2 = rkChrMgr.GetInstancePtr(iVID2);
		if (pInstance1 && pInstance2)
			pInstance1->ActDualEmotion(*pInstance2, CRaceMotionData::NAME_SLAP_HURT_START, CRaceMotionData::NAME_SLAP_HIT_START);
	}
	else if (!strcmpi(szCmd, "clap"))
	{
		int iVID = atoi(TokenVector[1].c_str());
		IAbstractCharacterManager & rkChrMgr = IAbstractCharacterManager::GetSingleton();
		CInstanceBase * pInstance = rkChrMgr.GetInstancePtr(iVID);
		if (pInstance)
			pInstance->ActEmotion(CRaceMotionData::NAME_CLAP);
	}
	else if (!strcmpi(szCmd, "cheer1"))
	{
		int iVID = atoi(TokenVector[1].c_str());
		IAbstractCharacterManager & rkChrMgr = IAbstractCharacterManager::GetSingleton();
		CInstanceBase * pInstance = rkChrMgr.GetInstancePtr(iVID);
		if (pInstance)
			pInstance->ActEmotion(CRaceMotionData::NAME_CHEERS_1);
	}
	else if (!strcmpi(szCmd, "cheer2"))
	{
		int iVID = atoi(TokenVector[1].c_str());
		IAbstractCharacterManager & rkChrMgr = IAbstractCharacterManager::GetSingleton();
		CInstanceBase * pInstance = rkChrMgr.GetInstancePtr(iVID);
		if (pInstance)
			pInstance->ActEmotion(CRaceMotionData::NAME_CHEERS_2);
	}
	else if (!strcmpi(szCmd, "dance1"))
	{
		int iVID = atoi(TokenVector[1].c_str());
		IAbstractCharacterManager & rkChrMgr = IAbstractCharacterManager::GetSingleton();
		CInstanceBase * pInstance = rkChrMgr.GetInstancePtr(iVID);
		if (pInstance)
			pInstance->ActEmotion(CRaceMotionData::NAME_DANCE_1);
	}
	else if (!strcmpi(szCmd, "dance2"))
	{
		int iVID = atoi(TokenVector[1].c_str());
		IAbstractCharacterManager & rkChrMgr = IAbstractCharacterManager::GetSingleton();
		CInstanceBase * pInstance = rkChrMgr.GetInstancePtr(iVID);
		if (pInstance)
			pInstance->ActEmotion(CRaceMotionData::NAME_DANCE_2);
	}
	else if (!strcmpi(szCmd, "dig_motion"))
	{
		int iVID = atoi(TokenVector[1].c_str());
		IAbstractCharacterManager & rkChrMgr = IAbstractCharacterManager::GetSingleton();
		CInstanceBase * pInstance = rkChrMgr.GetInstancePtr(iVID);
		if (pInstance)
			pInstance->ActEmotion(CRaceMotionData::NAME_DIG);
	}
	// Emotion End
	else
	{
		static std::map<std::string, int> s_emotionDict;

		static bool s_isFirst = true;
		if (s_isFirst)
		{
			s_isFirst = false;

			s_emotionDict["dance3"] = CRaceMotionData::NAME_DANCE_3;
			s_emotionDict["dance4"] = CRaceMotionData::NAME_DANCE_4;
			s_emotionDict["dance5"] = CRaceMotionData::NAME_DANCE_5;
			s_emotionDict["dance6"] = CRaceMotionData::NAME_DANCE_6;
			s_emotionDict["congratulation"] = CRaceMotionData::NAME_CONGRATULATION;
			s_emotionDict["forgive"] = CRaceMotionData::NAME_FORGIVE;
			s_emotionDict["angry"] = CRaceMotionData::NAME_ANGRY;
			s_emotionDict["attractive"] = CRaceMotionData::NAME_ATTRACTIVE;
			s_emotionDict["sad"] = CRaceMotionData::NAME_SAD;
			s_emotionDict["shy"] = CRaceMotionData::NAME_SHY;
			s_emotionDict["cheerup"] = CRaceMotionData::NAME_CHEERUP;
			s_emotionDict["banter"] = CRaceMotionData::NAME_BANTER;
			s_emotionDict["joy"] = CRaceMotionData::NAME_JOY;
#ifdef ENABLE_EXPRESSING_EMOTION
			s_emotionDict["dance7"] = CRaceMotionData::NAME_DANCE_7;
			s_emotionDict["doze"] = CRaceMotionData::NAME_DOZE;
			s_emotionDict["exercise"] = CRaceMotionData::NAME_EXERCISE;
			s_emotionDict["pushup"] = CRaceMotionData::NAME_PUSHUP;
			s_emotionDict["selfie"] = CRaceMotionData::NAME_SELFIE;
			// s_emotionDict["charging"] = CRaceMotionData::NAME_CHARGING;
			// s_emotionDict["nosay"] = CRaceMotionData::NAME_NOSAY;
			// s_emotionDict["weather1"] = CRaceMotionData::NAME_WEATHER1;
			// s_emotionDict["weather2"] = CRaceMotionData::NAME_WEATHER2;
			// s_emotionDict["weather3"] = CRaceMotionData::NAME_WEATHER3;
			// s_emotionDict["hungry"] = CRaceMotionData::NAME_HUNGRY;
			// s_emotionDict["siren"] = CRaceMotionData::NAME_SIREN;
			// s_emotionDict["letter"] = CRaceMotionData::NAME_LETTER;
			// s_emotionDict["call"] = CRaceMotionData::NAME_CALL;
			// s_emotionDict["celebration"] = CRaceMotionData::NAME_CELEBRATION;
			// s_emotionDict["alcohol"] = CRaceMotionData::NAME_ALCOHOL;
			// s_emotionDict["busy"] = CRaceMotionData::NAME_BUSY;
			// s_emotionDict["whirl"] = CRaceMotionData::NAME_WHIRL;
#endif
		}

		std::map<std::string, int>::iterator f = s_emotionDict.find(szCmd);
		if (f == s_emotionDict.end())
		{
			TraceError("Unknown Server Command %s | %s", c_szCommand, szCmd);
		}
		else
		{
			int emotionIndex = f->second;

			int iVID = atoi(TokenVector[1].c_str());
			IAbstractCharacterManager & rkChrMgr = IAbstractCharacterManager::GetSingleton();
			CInstanceBase * pInstance = rkChrMgr.GetInstancePtr(iVID);

			if (pInstance)
				pInstance->ActEmotion(emotionIndex);
		}		
	}
}
