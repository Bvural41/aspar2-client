#include "StdAfx.h"
#include "PythonChat.h"
#include "PythonItem.h"
#include "../gamelib/ItemManager.h"

PyObject * chatSetChatColor(PyObject* poSelf, PyObject* poArgs)
{
	int iType;
	if (!PyTuple_GetInteger(poArgs, 0, &iType))
		return Py_BuildException();

	int r;
	if (!PyTuple_GetInteger(poArgs, 1, &r))
		return Py_BuildException();

	int g;
	if (!PyTuple_GetInteger(poArgs, 2, &g))
		return Py_BuildException();

	int b;
	if (!PyTuple_GetInteger(poArgs, 3, &b))
		return Py_BuildException();

	CPythonChat::Instance().SetChatColor(iType, r, g, b);
	return Py_BuildNone();
}

PyObject * chatClear(PyObject* poSelf, PyObject* poArgs)
{
	CPythonChat::Instance().Destroy();
	return Py_BuildNone();
}

PyObject * chatClose(PyObject* poSelf, PyObject* poArgs)
{
	CPythonChat::Instance().Close();
	return Py_BuildNone();
}

PyObject * chatCreateChatSet(PyObject* poSelf, PyObject* poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonChat::Instance().CreateChatSet(iID));
}

PyObject * chatUpdate(PyObject* poSelf, PyObject* poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();

	CPythonChat::Instance().Update(iID);
	return Py_BuildNone();
}

PyObject * chatRender(PyObject* poSelf, PyObject* poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();

	CPythonChat::Instance().Render(iID);
	return Py_BuildNone();
}

PyObject * chatSetBoardState(PyObject* poSelf, PyObject* poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();
	int iState;
	if (!PyTuple_GetInteger(poArgs, 1, &iState))
		return Py_BuildException();

	CPythonChat::Instance().SetBoardState(iID, iState);

	return Py_BuildNone();
}

PyObject * chatSetPosition(PyObject* poSelf, PyObject* poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();
	int ix;
	if (!PyTuple_GetInteger(poArgs, 1, &ix))
		return Py_BuildException();
	int iy;
	if (!PyTuple_GetInteger(poArgs, 2, &iy))
		return Py_BuildException();

	CPythonChat::Instance().SetPosition(iID, ix, iy);

	return Py_BuildNone();
}

PyObject * chatSetHeight(PyObject* poSelf, PyObject* poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();
	int iHeight;
	if (!PyTuple_GetInteger(poArgs, 1, &iHeight))
		return Py_BuildException();

	CPythonChat::Instance().SetHeight(iID, iHeight);

	return Py_BuildNone();
}

PyObject * chatSetStep(PyObject* poSelf, PyObject* poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();
	int iStep;
	if (!PyTuple_GetInteger(poArgs, 1, &iStep))
		return Py_BuildException();

	CPythonChat::Instance().SetStep(iID, iStep);

	return Py_BuildNone();
}

PyObject * chatToggleChatMode(PyObject* poSelf, PyObject* poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();
	int iType;
	if (!PyTuple_GetInteger(poArgs, 1, &iType))
		return Py_BuildException();

	CPythonChat::Instance().ToggleChatMode(iID, iType);

	return Py_BuildNone();
}

PyObject * chatEnableChatMode(PyObject* poSelf, PyObject* poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();
	int iType;
	if (!PyTuple_GetInteger(poArgs, 1, &iType))
		return Py_BuildException();

	CPythonChat::Instance().EnableChatMode(iID, iType);

	return Py_BuildNone();
}

PyObject * chatDisableChatMode(PyObject* poSelf, PyObject* poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();
	int iType;
	if (!PyTuple_GetInteger(poArgs, 1, &iType))
		return Py_BuildException();

	CPythonChat::Instance().DisableChatMode(iID, iType);

	return Py_BuildNone();
}

PyObject * chatSetEndPos(PyObject* poSelf, PyObject* poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();
	float fPos;
	if (!PyTuple_GetFloat(poArgs, 1, &fPos))
		return Py_BuildException();

	CPythonChat::Instance().SetEndPos(iID, fPos);

	return Py_BuildNone();
}

PyObject * chatGetLineCount(PyObject* poSelf, PyObject* poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonChat::Instance().GetLineCount(iID));
}

PyObject * chatGetVisibleLineCount(PyObject* poSelf, PyObject* poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonChat::Instance().GetVisibleLineCount(iID));
}

PyObject * chatGetLineStep(PyObject* poSelf, PyObject* poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonChat::Instance().GetLineStep(iID));
}

PyObject * chatAppendChat(PyObject* poSelf, PyObject* poArgs)
{
	int iType;
	if (!PyTuple_GetInteger(poArgs, 0, &iType))
		return Py_BuildException();

	char * szChat;
	if (!PyTuple_GetString(poArgs, 1, &szChat))
		return Py_BuildException();

	CPythonChat::Instance().AppendChat(iType, szChat);

	return Py_BuildNone();
}

PyObject * chatAppendChatWithDelay(PyObject* poSelf, PyObject* poArgs)
{
	int iType;
	if (!PyTuple_GetInteger(poArgs, 0, &iType))
		return Py_BuildException();

	char * szChat;
	if (!PyTuple_GetString(poArgs, 1, &szChat))
		return Py_BuildException();

	int iDelay;
	if (!PyTuple_GetInteger(poArgs, 2, &iDelay))
		return Py_BuildException();

	CPythonChat::Instance().AppendChatWithDelay(iType, szChat, iDelay);

	return Py_BuildNone();
}

#ifdef ENABLE_CHAT_STACK
PyObject * chatAppendChatStack(PyObject* poSelf, PyObject* poArgs)
{
	char * szChat;
	if (!PyTuple_GetString(poArgs, 0, &szChat))
		return Py_BuildException();

	CPythonChat::Instance().AddChatStack(szChat);

	return Py_BuildNone();
}

PyObject * chatGetChatStack(PyObject* poSelf, PyObject* poArgs)
{
	BYTE index;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	const char* stack = CPythonChat::Instance().GetChatStack(index);
	if (strcmp(stack, "") == 0)
		return Py_BuildNone();

	return Py_BuildValue("s", stack);
}

PyObject * chatGetChatStackSize(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonChat::Instance().GetChatStackSize());
}
#endif

PyObject * chatArrangeShowingChat(PyObject* poSelf, PyObject* poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();

	CPythonChat::Instance().ArrangeShowingChat(iID);

	return Py_BuildNone();
}

#ifdef ENABLE_CHAT_STOP_SYSTEM
PyObject * chatSetChatStop(PyObject* poSelf, PyObject* poArgs)
{
	bool bStop;
	if (!PyTuple_GetBoolean(poArgs, 0, &bStop))
		return Py_BuildException();

	CPythonChat::Instance().SetChatStop(bStop);
	return Py_BuildNone();
}
#endif

PyObject * chatIgnoreCharacter(PyObject* poSelf, PyObject* poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	CPythonChat::Instance().IgnoreCharacter(szName);

	return Py_BuildNone();
}

PyObject * chatIsIgnoreCharacter(PyObject* poSelf, PyObject* poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	CPythonChat::Instance().IsIgnoreCharacter(szName);

	return Py_BuildNone();
}

PyObject * chatCreateWhisper(PyObject* poSelf, PyObject* poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	CPythonChat::Instance().CreateWhisper(szName);

	return Py_BuildNone();
}

PyObject * chatAppendWhisper(PyObject* poSelf, PyObject* poArgs)
{
	int iType;
	if (!PyTuple_GetInteger(poArgs, 0, &iType))
		return Py_BuildException();

	char * szName;
	if (!PyTuple_GetString(poArgs, 1, &szName))
		return Py_BuildException();

	char * szChat;
	if (!PyTuple_GetString(poArgs, 2, &szChat))
		return Py_BuildException();

	CPythonChat::Instance().AppendWhisper(iType, szName, szChat);
	return Py_BuildNone();
}

PyObject * chatRenderWhisper(PyObject* poSelf, PyObject* poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	float fx;
	if (!PyTuple_GetFloat(poArgs, 1, &fx))
		return Py_BuildException();

	float fy;
	if (!PyTuple_GetFloat(poArgs, 2, &fy))
		return Py_BuildException();

	CWhisper * pWhisper;
	if (CPythonChat::Instance().GetWhisper(szName, &pWhisper))
	{
		pWhisper->Render(fx, fy);
	}

	return Py_BuildNone();
}

PyObject * chatSetWhisperBoxSize(PyObject* poSelf, PyObject* poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	float fWidth;
	if (!PyTuple_GetFloat(poArgs, 1, &fWidth))
		return Py_BuildException();

	float fHeight;
	if (!PyTuple_GetFloat(poArgs, 2, &fHeight))
		return Py_BuildException();

	CWhisper * pWhisper;
	if (CPythonChat::Instance().GetWhisper(szName, &pWhisper))
	{
		pWhisper->SetBoxSize(fWidth, fHeight);
	}

	return Py_BuildNone();
}

PyObject * chatSetWhisperPosition(PyObject* poSelf, PyObject* poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	float fPosition;
	if (!PyTuple_GetFloat(poArgs, 1, &fPosition))
		return Py_BuildException();

	CWhisper * pWhisper;
	if (CPythonChat::Instance().GetWhisper(szName, &pWhisper))
	{
		pWhisper->SetPosition(fPosition);
	}

	return Py_BuildNone();
}

PyObject * chatClearWhisper(PyObject* poSelf, PyObject* poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	CPythonChat::Instance().ClearWhisper(szName);

	return Py_BuildNone();
}

PyObject * chatInitWhisper(PyObject* poSelf, PyObject* poArgs)
{
	PyObject * poInterface;
	if (!PyTuple_GetObject(poArgs, 0, &poInterface))
		return Py_BuildException();

	CPythonChat::Instance().InitWhisper(poInterface);
	return Py_BuildNone();
}

PyObject * chatGetLinkFromHyperlink(PyObject * poSelf, PyObject * poArgs)
{
	char * szHyperlink;

	if (!PyTuple_GetString(poArgs, 0, &szHyperlink))
		return Py_BuildException();

	std::string stHyperlink(szHyperlink);
	std::vector<std::string> results;

	split_string(stHyperlink, ":", results, false);

	// item:vnum:flag:socket0:socket1:socket2
	if ("item" == results[0])
	{
		if (results.size() < CPythonChat::HYPER_LINK_ITEM_SOCKET3 + 1)
		{
			return Py_BuildValue("s", "");
		}

		CItemData * pItemData = nullptr;

		if (CItemManager::Instance().GetItemDataPointer(htoi(results[1].c_str()), &pItemData))
		{
			char buf[1024] = {0};
			char itemlink[512];
			bool isAttr = false;

			int len = snprintf(itemlink, sizeof(itemlink),
				"item"
				":%x"    // HYPER_LINK_ITEM_VNUM
				":%x"    // HYPER_LINK_ITEM_FLAGS
				":%x"    // HYPER_LINK_ITEM_SOCKET0
				":%x"    // HYPER_LINK_ITEM_SOCKET1
				":%x"    // HYPER_LINK_ITEM_SOCKET2
				":%x"    // HYPER_LINK_ITEM_SOCKET3
				":%x"    // HYPER_LINK_ITEM_SOCKET4
				":%x",    // HYPER_LINK_ITEM_SOCKET5
				htoi(results[CPythonChat::HYPER_LINK_ITEM_VNUM].c_str()),
				htoi(results[CPythonChat::HYPER_LINK_ITEM_FLAGS].c_str()),
				htoi(results[CPythonChat::HYPER_LINK_ITEM_SOCKET0].c_str()),
				htoi(results[CPythonChat::HYPER_LINK_ITEM_SOCKET1].c_str()),
				htoi(results[CPythonChat::HYPER_LINK_ITEM_SOCKET2].c_str()),
				htoi(results[CPythonChat::HYPER_LINK_ITEM_SOCKET3].c_str()),
				htoi(results[CPythonChat::HYPER_LINK_ITEM_SOCKET4].c_str()),
				htoi(results[CPythonChat::HYPER_LINK_ITEM_SOCKET5].c_str()));

#ifdef ENABLE_CHANGELOOK_SYSTEM
			len += snprintf(itemlink + len, sizeof(itemlink) - len, ":%x", htoi(results[CPythonChat::HYPER_LINK_ITEM_CHANGE_ITEM_VNUM].c_str())); // HYPER_LINK_ITEM_CHANGE_ITEM_VNUM
#endif

#ifdef ENABLE_REFINE_ELEMENT
			len += snprintf(itemlink + len, sizeof(itemlink) - len, ":%x", htoi(results[CPythonChat::HYPER_LINK_ITEM_ELEMENT_ITEM_VNUM].c_str())); // HYPER_LINK_ITEM_ELEMENT_ITEM_VNUM
#endif

#ifdef ENABLE_SET_ITEM
			len += snprintf(itemlink + len, sizeof(itemlink) - len, ":%d", htoi(results[CPythonChat::HYPER_LINK_ITEM_PRE_SET_VALUE].c_str())); // HYPER_LINK_ITEM_PRE_SET_VALUE
#endif

#ifdef ENABLE_GLOVE_SYSTEM
			for (int s = CPythonChat::HYPER_LINK_ITEM_APPLY_RANDOM_TYPE0; s <= CPythonChat::HYPER_LINK_ITEM_APPLY_RANDOM_VALUE3; s += 2)
			{
				len += snprintf(itemlink + len, sizeof(itemlink) - len,
					":%x" // HYPER_LINK_ITEM_APPLY_RANDOM_TYPE0~3
					":%lld", // HYPER_LINK_ITEM_APPLY_RANDOM_VALUE0~3
					htoi(results[s].c_str()), atoi(results[s + 1].c_str()));
			}
#endif

			if (results.size() >= CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_TYPE1)
			{
				for (int i = CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_TYPE0; i < results.size(); i += 2)
				{
					len += snprintf(itemlink + len, sizeof(itemlink) - len,
						":%x" // HYPER_LINK_ITEM_ATTRIBUTE_TYPE0~6
						":%lld", // HYPER_LINK_ITEM_ATTRIBUTE_VALUE0~6
						htoi(results[i].c_str()), atoi(results[i + 1].c_str()));

					isAttr = true;
				}
			}

#ifdef ENABLE_SET_ITEM
			int set_value = 0;
			if (results.size() > 8)
				set_value = htoi(results[8].c_str());

			static const char* setPrefixes[] = { "", "Kuts. ", "Görk. ", "Kahr. ", "Soylu ", "Heyb. " };
			const char* setPrefix = "";

			if (set_value >= 1 && set_value <= 5)
				setPrefix = setPrefixes[set_value];
#endif

			if (isAttr)
#ifdef ENABLE_SET_ITEM
				snprintf(buf, sizeof(buf), "|cffffc700|H%s|h[%s%s]|h|r", itemlink, setPrefix, pItemData->GetName());
#else
				snprintf(buf, sizeof(buf), "|cffffc700|H%s|h[%s]|h|r", itemlink, pItemData->GetName());
#endif
			else
#ifdef ENABLE_SET_ITEM
				snprintf(buf, sizeof(buf), "|cfff1e6c0|H%s|h[%s%s]|h|r", itemlink, setPrefix, pItemData->GetName());
#else
				snprintf(buf, sizeof(buf), "|cfff1e6c0|H%s|h[%s]|h|r", itemlink, pItemData->GetName());
#endif

			return Py_BuildValue("s", buf);
		}
	}

#ifdef ENABLE_LINK_IN_CHAT
	else if (0 == results[0].compare("link"))
	{
		char buf[1024] = { 0 };
		snprintf(buf, sizeof(buf), "|cffc9c0f1|H|h%s|h|r", results[1].c_str());
		return Py_BuildValue("s", buf);
	}
#endif

	return Py_BuildValue("s", "");
}

#ifdef ENABLE_CHAT_SETTINGS
PyObject* chatDeleteChatSet(PyObject* poSelf, PyObject* poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();

	CPythonChat::Instance().DeleteChatSet(iID);
	return Py_BuildNone();
}
#endif
#if defined (ENABLE_CHAT_SETTINGS) && defined(ENABLE_MULTI_LANGUAGE_SYSTEM)
PyObject* chatEnableCountryMode(PyObject* poSelf, PyObject* poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();
	int iType;
	if (!PyTuple_GetInteger(poArgs, 1, &iType))
		return Py_BuildException();

	CPythonChat::Instance().EnableCountryMode(iID, iType);

	return Py_BuildNone();
}
PyObject* chatDisableCountryMode(PyObject* poSelf, PyObject* poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();
	int iType;
	if (!PyTuple_GetInteger(poArgs, 1, &iType))
		return Py_BuildException();

	CPythonChat::Instance().DisableCountryMode(iID, iType);

	return Py_BuildNone();
}
#endif

void initChat()
{
	static PyMethodDef s_methods[] = 
	{
		{ "SetChatColor",			chatSetChatColor,			METH_VARARGS },
		{ "Clear",					chatClear,					METH_VARARGS },
		{ "Close",					chatClose,					METH_VARARGS },

		{ "CreateChatSet",			chatCreateChatSet,			METH_VARARGS },
		{ "Update",					chatUpdate,					METH_VARARGS },
		{ "Render",					chatRender,					METH_VARARGS },

		{ "SetBoardState",			chatSetBoardState,			METH_VARARGS },
		{ "SetPosition",			chatSetPosition,			METH_VARARGS },
		{ "SetHeight",				chatSetHeight,				METH_VARARGS },
		{ "SetStep",				chatSetStep,				METH_VARARGS },
		{ "ToggleChatMode",			chatToggleChatMode,			METH_VARARGS },
		{ "EnableChatMode",			chatEnableChatMode,			METH_VARARGS },
		{ "DisableChatMode",		chatDisableChatMode,		METH_VARARGS },
		{ "SetEndPos",				chatSetEndPos,				METH_VARARGS },

		{ "GetLineCount",			chatGetLineCount,			METH_VARARGS },
		{ "GetVisibleLineCount",	chatGetVisibleLineCount,	METH_VARARGS },
		{ "GetLineStep",			chatGetLineStep,			METH_VARARGS },

		// Chat
		{ "AppendChat",				chatAppendChat,				METH_VARARGS },
		{ "AppendChatWithDelay",	chatAppendChatWithDelay,	METH_VARARGS },
		{ "ArrangeShowingChat",		chatArrangeShowingChat,		METH_VARARGS },
#ifdef ENABLE_CHAT_STOP_SYSTEM
		{ "SetChatStop",			chatSetChatStop,			METH_VARARGS },
#endif

		#ifdef ENABLE_CHAT_STACK
		{ "AppendChatStack",		chatAppendChatStack,		METH_VARARGS },
		{ "GetChatStack",		chatGetChatStack,		METH_VARARGS },
		{ "GetChatStackSize",		chatGetChatStackSize,		METH_VARARGS },
		#endif

		// Ignore
		{ "IgnoreCharacter",		chatIgnoreCharacter,		METH_VARARGS },
		{ "IsIgnoreCharacter",		chatIsIgnoreCharacter,		METH_VARARGS },

		// Whisper
		{ "CreateWhisper",			chatCreateWhisper,			METH_VARARGS },
		{ "AppendWhisper",			chatAppendWhisper,			METH_VARARGS },
		{ "RenderWhisper",			chatRenderWhisper,			METH_VARARGS },
		{ "SetWhisperBoxSize",		chatSetWhisperBoxSize,		METH_VARARGS },
		{ "SetWhisperPosition",		chatSetWhisperPosition,		METH_VARARGS },
		{ "ClearWhisper",			chatClearWhisper,			METH_VARARGS },
		{ "InitWhisper",			chatInitWhisper,			METH_VARARGS },

		// Link
		{ "GetLinkFromHyperlink",	chatGetLinkFromHyperlink,	METH_VARARGS },
#ifdef ENABLE_CHAT_SETTINGS
		{ "DeleteChatSet",			chatDeleteChatSet,			METH_VARARGS },
#endif
#if defined (ENABLE_CHAT_SETTINGS) && defined(ENABLE_MULTI_LANGUAGE_SYSTEM)
		{ "EnableCountryMode",	chatEnableCountryMode,	METH_VARARGS },
		{ "DisableCountryMode",	chatDisableCountryMode,	METH_VARARGS },
#endif
		{ NULL,						NULL,						NULL },
	};

	PyObject * poModule = Py_InitModule("chat", s_methods);

	PyModule_AddIntConstant(poModule, "CHAT_TYPE_TALKING",		CHAT_TYPE_TALKING);
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_INFO",			CHAT_TYPE_INFO);
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_NOTICE",		CHAT_TYPE_NOTICE);

#ifdef ENABLE_OX_RENEWAL
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_CONTROL_NOTICE", CHAT_TYPE_CONTROL_NOTICE);
#endif

#ifdef ENABLE_DICE_SYSTEM
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_DICE_INFO",	CHAT_TYPE_DICE_INFO);
#endif

#ifdef ENABLE_12ZI
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_ZODIAC_NOTICE", CHAT_TYPE_ZODIAC_NOTICE);
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_ZODIAC_SUB_NOTICE", CHAT_TYPE_ZODIAC_SUB_NOTICE);
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_ZODIAC_NOTICE_CLEAR", CHAT_TYPE_ZODIAC_NOTICE_CLEAR);
#endif

#ifdef ENABLE_CHAT_SETTINGS_EXTEND
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_EXP_INFO",			CHAT_TYPE_EXP_INFO);
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_ITEM_INFO",		CHAT_TYPE_ITEM_INFO);
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_MONEY_INFO",		CHAT_TYPE_MONEY_INFO);
#endif

	PyModule_AddIntConstant(poModule, "CHAT_TYPE_PARTY",		CHAT_TYPE_PARTY);
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_GUILD",		CHAT_TYPE_GUILD);
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_COMMAND",		CHAT_TYPE_COMMAND);
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_SHOUT",		CHAT_TYPE_SHOUT);
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_WHISPER",		CHAT_TYPE_WHISPER);
	PyModule_AddIntConstant(poModule, "CHAT_TYPE_BIG_NOTICE",		CHAT_TYPE_BIG_NOTICE);
	PyModule_AddIntConstant(poModule, "WHISPER_TYPE_CHAT",		CPythonChat::WHISPER_TYPE_CHAT);
	PyModule_AddIntConstant(poModule, "WHISPER_TYPE_SYSTEM",	CPythonChat::WHISPER_TYPE_SYSTEM);
	PyModule_AddIntConstant(poModule, "WHISPER_TYPE_GM",		CPythonChat::WHISPER_TYPE_GM);

	PyModule_AddIntConstant(poModule, "BOARD_STATE_VIEW",		CPythonChat::BOARD_STATE_VIEW);
	PyModule_AddIntConstant(poModule, "BOARD_STATE_EDIT",		CPythonChat::BOARD_STATE_EDIT);
	PyModule_AddIntConstant(poModule, "BOARD_STATE_LOG",		CPythonChat::BOARD_STATE_LOG);

	PyModule_AddIntConstant(poModule, "CHAT_SET_CHAT_WINDOW",	0);
	PyModule_AddIntConstant(poModule, "CHAT_SET_LOG_WINDOW",	1);

	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_KEYWORD", CPythonChat::HYPER_LINK_ITEM_KEYWORD);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_VNUM", CPythonChat::HYPER_LINK_ITEM_VNUM);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_FLAGS", CPythonChat::HYPER_LINK_ITEM_FLAGS);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_SOCKET0", CPythonChat::HYPER_LINK_ITEM_SOCKET0);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_SOCKET1", CPythonChat::HYPER_LINK_ITEM_SOCKET1);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_SOCKET2", CPythonChat::HYPER_LINK_ITEM_SOCKET2);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_SOCKET3", CPythonChat::HYPER_LINK_ITEM_SOCKET3);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_SOCKET4", CPythonChat::HYPER_LINK_ITEM_SOCKET4);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_SOCKET5", CPythonChat::HYPER_LINK_ITEM_SOCKET5);

	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_CHANGE_ITEM_VNUM", CPythonChat::HYPER_LINK_ITEM_CHANGE_ITEM_VNUM);

	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ELEMENT_ITEM_VNUM", CPythonChat::HYPER_LINK_ITEM_ELEMENT_ITEM_VNUM);

	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_PRE_SET_VALUE", CPythonChat::HYPER_LINK_ITEM_PRE_SET_VALUE);

	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_APPLY_RANDOM_TYPE0", CPythonChat::HYPER_LINK_ITEM_APPLY_RANDOM_TYPE0);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_APPLY_RANDOM_VALUE0", CPythonChat::HYPER_LINK_ITEM_APPLY_RANDOM_VALUE0);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_APPLY_RANDOM_TYPE1", CPythonChat::HYPER_LINK_ITEM_APPLY_RANDOM_TYPE1);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_APPLY_RANDOM_VALUE1", CPythonChat::HYPER_LINK_ITEM_APPLY_RANDOM_VALUE1);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_APPLY_RANDOM_TYPE2", CPythonChat::HYPER_LINK_ITEM_APPLY_RANDOM_TYPE2);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_APPLY_RANDOM_VALUE2", CPythonChat::HYPER_LINK_ITEM_APPLY_RANDOM_VALUE2);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_APPLY_RANDOM_TYPE3", CPythonChat::HYPER_LINK_ITEM_APPLY_RANDOM_TYPE3);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_APPLY_RANDOM_VALUE3", CPythonChat::HYPER_LINK_ITEM_APPLY_RANDOM_VALUE3);

	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_TYPE0", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_TYPE0);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_VALUE0", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_VALUE0);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_TYPE1", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_TYPE1);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_VALUE1", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_VALUE1);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_TYPE2", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_TYPE2);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_VALUE2", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_VALUE2);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_TYPE3", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_TYPE3);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_VALUE3", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_VALUE3);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_TYPE4", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_TYPE4);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_VALUE4", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_VALUE4);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_TYPE5", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_TYPE5);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_VALUE5", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_VALUE5);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_TYPE6", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_TYPE6);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_VALUE6", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_VALUE6);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_TYPE7", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_TYPE7);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_VALUE7", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_VALUE7);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_TYPE8", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_TYPE8);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_VALUE8", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_VALUE8);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_TYPE9", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_TYPE9);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_VALUE9", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_VALUE9);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_TYPE10", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_TYPE10);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_VALUE10", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_VALUE10);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_TYPE11", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_TYPE11);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_VALUE11", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_VALUE11);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_TYPE12", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_TYPE12);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_VALUE12", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_VALUE12);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_TYPE13", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_TYPE13);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_VALUE13", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_VALUE13);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_TYPE14", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_TYPE14);
	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_ATTRIBUTE_VALUE14", CPythonChat::HYPER_LINK_ITEM_ATTRIBUTE_VALUE14);

	PyModule_AddIntConstant(poModule, "HYPER_LINK_ITEM_MAX", CPythonChat::HYPER_LINK_ITEM_MAX);

}
