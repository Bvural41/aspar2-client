#include "StdAfx.h"
#ifdef ENABLE_TITLE_SYSTEM
#include "PythonTitleSystem.h"

PyObject* titleSystemInitialize(PyObject* poSelf, PyObject* poArgs)
{
	CPythonTitleSystem::Instance().Initialize();
	return Py_BuildNone();
}

PyObject* titleSystemClear(PyObject* poSelf, PyObject* poArgs)
{
	CPythonTitleSystem::Instance().Clear();
	return Py_BuildNone();
}

PyObject* titleSystemIsDataReceived(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonTitleSystem::Instance().IsDataReceived() ? 1 : 0);
}

PyObject* titleSystemGetAllTitleData(PyObject* poSelf, PyObject* poArgs)
{
	const std::map<int, CPythonTitleSystem::TTitleData>& dataMap =
		CPythonTitleSystem::Instance().GetAllTitleData();

	PyObject* pyDict = PyDict_New();

	for (auto it = dataMap.begin(); it != dataMap.end(); ++it)
	{
		const CPythonTitleSystem::TTitleData& td = it->second;

		PyObject* pyTuple = Py_BuildValue("(iissiisi)",
			td.iTitleIndex,
			td.iTitleType,
			td.strName.c_str(),
			td.strConditionTooltip.c_str(),
			td.bIsPermanent ? 1 : 0,
			td.strOpenTime.c_str(),
			(int)td.dwFontColor,
			td.iResourceIndex
		);

		PyObject* pyKey = PyInt_FromLong(it->first);
		PyDict_SetItem(pyDict, pyKey, pyTuple);
		Py_DECREF(pyKey);
		Py_DECREF(pyTuple);
	}

	return pyDict;
}

PyObject* titleSystemGetShowList(PyObject* poSelf, PyObject* poArgs)
{
	const std::map<int, CPythonTitleSystem::TPlayerTitleData>& playerMap = CPythonTitleSystem::Instance().GetPlayerTitleMap();

	PyObject* pyDict = PyDict_New();
	for (auto it = playerMap.begin(); it != playerMap.end(); ++it)
	{
		const CPythonTitleSystem::TPlayerTitleData& pd = it->second;

		PyObject* pyList = PyList_New(4);
		PyList_SetItem(pyList, 0, PyInt_FromLong(pd.iTitleIndex));
		PyList_SetItem(pyList, 1, PyInt_FromLong(pd.iEndTime));
		PyList_SetItem(pyList, 2, PyBool_FromLong(pd.bIsEquip ? 1 : 0));
		PyList_SetItem(pyList, 3, PyBool_FromLong(pd.bIsObtain ? 1 : 0));

		PyObject* pyKey = PyInt_FromLong(it->first);
		PyDict_SetItem(pyDict, pyKey, pyList);
		Py_DECREF(pyKey);
		Py_DECREF(pyList);
	}

	return pyDict;
}

PyObject* titleSystemGetEquippedTitle(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonTitleSystem::Instance().GetEquippedTitle());
}

PyObject* titleSystemIsTitleEquipped(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonTitleSystem::Instance().IsTitleEquipped() ? 1 : 0);
}

PyObject* titleSystemGetColorFamily(PyObject* poSelf, PyObject* poArgs)
{
	int iTitleIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iTitleIndex))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonTitleSystem::Instance().GetColorFamily(iTitleIndex));
}

PyObject* titleSystemGetNameplatePaths(PyObject* poSelf, PyObject* poArgs)
{
	int iColorFamily;
	if (!PyTuple_GetInteger(poArgs, 0, &iColorFamily))
		return Py_BuildException();

	CPythonTitleSystem::TNameplatePaths paths;
	if (!CPythonTitleSystem::Instance().GetNameplatePaths(iColorFamily, &paths))
		return Py_BuildNone();

	return Py_BuildValue("(sss)", paths.strLeft.c_str(), paths.strMiddle.c_str(), paths.strRight.c_str());
}

PyObject* titleSystemGetPreviewSpriteData(PyObject* poSelf, PyObject* poArgs)
{
	int iTitleIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iTitleIndex))
		return Py_BuildException();

	CPythonTitleSystem::TSpritePreviewData spriteData;
	if (!CPythonTitleSystem::Instance().GetPreviewSpriteData(iTitleIndex, &spriteData))
		return Py_BuildNone();

	return Py_BuildValue("(siiii)",
		spriteData.strDirPath.c_str(),
		spriteData.iFrameCount,
		spriteData.iSizeX,
		spriteData.iSizeY,
		spriteData.iColumns
	);
}

PyObject* titleSystemGetPreviewEffectData(PyObject* poSelf, PyObject* poArgs)
{
	int iTitleIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iTitleIndex))
		return Py_BuildException();

	CPythonTitleSystem::TEffectPreviewData effectData;
	if (!CPythonTitleSystem::Instance().GetPreviewEffectData(iTitleIndex, &effectData))
		return Py_BuildNone();

	return Py_BuildValue("(si)",
		effectData.strEffectPath.c_str(),
		effectData.iColorFamily
	);
}

PyObject* titleSystemGetItemTooltip(PyObject* poSelf, PyObject* poArgs)
{
	int iTitleIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iTitleIndex))
		return Py_BuildException();

	const char* pszTooltip = CPythonTitleSystem::Instance().GetItemTooltip(iTitleIndex);
	return Py_BuildValue("s", pszTooltip ? pszTooltip : "");
}

PyObject* titleSystemSetPlayerTitleData(PyObject* poSelf, PyObject* poArgs)
{
	int iTitleIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iTitleIndex))
		return Py_BuildException();

	int iEndTime;
	if (!PyTuple_GetInteger(poArgs, 1, &iEndTime))
		return Py_BuildException();

	int iIsEquip;
	if (!PyTuple_GetInteger(poArgs, 2, &iIsEquip))
		return Py_BuildException();

	int iIsObtain;
	if (!PyTuple_GetInteger(poArgs, 3, &iIsObtain))
		return Py_BuildException();

	CPythonTitleSystem::Instance().SetPlayerTitleData(iTitleIndex, iEndTime, iIsEquip != 0, iIsObtain != 0);
	return Py_BuildNone();
}

PyObject* titleSystemRemovePlayerTitleData(PyObject* poSelf, PyObject* poArgs)
{
	int iTitleIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iTitleIndex))
		return Py_BuildException();

	CPythonTitleSystem::Instance().RemovePlayerTitleData(iTitleIndex);
	return Py_BuildNone();
}

PyObject* titleSystemRequestEquip(PyObject* poSelf, PyObject* poArgs)
{
	int iTitleIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iTitleIndex))
		return Py_BuildException();

	CPythonTitleSystem::Instance().RequestEquip(iTitleIndex);
	return Py_BuildNone();
}

PyObject* titleSystemShowEffect(PyObject* poSelf, PyObject* poArgs)
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();

	int iTitleIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iTitleIndex))
		return Py_BuildException();

	CPythonTitleSystem::Instance().ShowEffect((DWORD)iVID, iTitleIndex);
	return Py_BuildNone();
}

PyObject* titleSystemHideEffect(PyObject* poSelf, PyObject* poArgs)
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();

	CPythonTitleSystem::Instance().HideEffect((DWORD)iVID);
	return Py_BuildNone();
}

PyObject* titleSystemRefreshMyTitle(PyObject* poSelf, PyObject* poArgs)
{
	CPythonTitleSystem::Instance().RefreshMyTitle();
	return Py_BuildNone();
}

PyObject* titleSystemSetHandler(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildNone();
}

PyObject* titleSystemSetShow(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildNone();
}

PyObject* titleSystemGetImagePath(PyObject* poSelf, PyObject* poArgs)
{
	int iTitleIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iTitleIndex))
		return Py_BuildException();

	CPythonTitleSystem::TImagePreviewData imageData;
	if (!CPythonTitleSystem::Instance().GetPreviewImageData(iTitleIndex, &imageData))
		return Py_BuildNone();

	return Py_BuildValue("s", imageData.strImagePath.c_str());
}

PyObject* titleSystemGetBannerImage(PyObject* poSelf, PyObject* poArgs)
{
	int iTitleIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iTitleIndex))
		return Py_BuildException();

	int iColorFamily = CPythonTitleSystem::Instance().GetColorFamily(iTitleIndex);
	if (iColorFamily != CPythonTitleSystem::COLOR_FAMILY_NONE)
		return Py_BuildValue("s", CPythonTitleSystem::TITLE_PREVIEW_IMAGE);

	return Py_BuildValue("s", "");
}

PyObject* titleSystemIsItemUsable(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", 1);
}

PyObject* titleSystemIsTitleAvailableMap(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", 1);
}

PyObject* titleSystemShowOtherPlayerTitle(PyObject* poSelf, PyObject* poArgs)
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();

	int iTitleIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iTitleIndex))
		return Py_BuildException();

	CPythonTitleSystem::TTitleData titleData;
	if (CPythonTitleSystem::Instance().GetTitleData(iTitleIndex, &titleData))
		CPythonTitleSystem::Instance().ShowEffect((DWORD)iVID, iTitleIndex);
	else
		CPythonTitleSystem::Instance().HideEffect((DWORD)iVID);

	return Py_BuildNone();
}

void initTitleSystem()
{
	static PyMethodDef s_methods[] =
	{
		{ "Initialize", titleSystemInitialize, METH_VARARGS },
		{ "Clear", titleSystemClear, METH_VARARGS },
		{ "IsDataReceived", titleSystemIsDataReceived, METH_VARARGS },

		{ "GetAllTitleData", titleSystemGetAllTitleData, METH_VARARGS },
		{ "GetShowList", titleSystemGetShowList, METH_VARARGS },
		{ "GetEquippedTitle", titleSystemGetEquippedTitle, METH_VARARGS },
		{ "IsTitleEquipped", titleSystemIsTitleEquipped, METH_VARARGS },
		{ "GetColorFamily", titleSystemGetColorFamily, METH_VARARGS },
		{ "GetNameplatePaths", titleSystemGetNameplatePaths, METH_VARARGS },
		{ "GetPreviewSpriteData", titleSystemGetPreviewSpriteData, METH_VARARGS },
		{ "GetPreviewEffectData", titleSystemGetPreviewEffectData, METH_VARARGS },
		{ "GetItemTooltip", titleSystemGetItemTooltip, METH_VARARGS },
		{ "GetBannerImage", titleSystemGetBannerImage, METH_VARARGS },
		{ "GetImagePath", titleSystemGetImagePath, METH_VARARGS },

		{ "SetPlayerTitleData", titleSystemSetPlayerTitleData, METH_VARARGS },
		{ "RemovePlayerTitleData", titleSystemRemovePlayerTitleData, METH_VARARGS },
		{ "RequestEquip", titleSystemRequestEquip, METH_VARARGS },

		{ "ShowEffect", titleSystemShowEffect, METH_VARARGS },
		{ "HideEffect", titleSystemHideEffect, METH_VARARGS },
		{ "RefreshMyTitle", titleSystemRefreshMyTitle, METH_VARARGS },
		{ "ShowOtherPlayerTitle", titleSystemShowOtherPlayerTitle, METH_VARARGS },

		{ "SetHandler", titleSystemSetHandler, METH_VARARGS },
		{ "SetShow", titleSystemSetShow, METH_VARARGS },
		{ "IsItemUsable", titleSystemIsItemUsable, METH_VARARGS },
		{ "IsTitleAvailableMap", titleSystemIsTitleAvailableMap, METH_VARARGS },

		{ NULL, NULL, NULL },
	};

	PyObject* poModule = Py_InitModule("titleSystem", s_methods);

	PyModule_AddIntConstant(poModule, "DATA_COLUMN_TITLE_INDEX", CPythonTitleSystem::DATA_COLUMN_TITLE_INDEX);
	PyModule_AddIntConstant(poModule, "DATA_COLUMN_TITLE_TYPE", CPythonTitleSystem::DATA_COLUMN_TITLE_TYPE);
	PyModule_AddIntConstant(poModule, "DATA_COLUMN_NAME", CPythonTitleSystem::DATA_COLUMN_NAME);
	PyModule_AddIntConstant(poModule, "DATA_COLUMN_CONDITION_TOOLTIP", CPythonTitleSystem::DATA_COLUMN_CONDITION_TOOLTIP);
	PyModule_AddIntConstant(poModule, "DATA_COLUMN_IS_PERMANENT", CPythonTitleSystem::DATA_COLUMN_IS_PERMANENT);
	PyModule_AddIntConstant(poModule, "DATA_COLUMN_OPEN_TIME", CPythonTitleSystem::DATA_COLUMN_OPEN_TIME);
	PyModule_AddIntConstant(poModule, "DATA_COLUMN_FONT_COLOR", CPythonTitleSystem::DATA_COLUMN_FONT_COLOR);
	PyModule_AddIntConstant(poModule, "DATA_COLUMN_RESOURCE_INDEX", CPythonTitleSystem::DATA_COLUMN_RESOURCE_INDEX);
	PyModule_AddIntConstant(poModule, "DATA_COLUMN_MAX", CPythonTitleSystem::DATA_COLUMN_MAX);

	PyModule_AddIntConstant(poModule, "PLAYER_COLUMN_TITLE_INDEX", CPythonTitleSystem::PLAYER_COLUMN_TITLE_INDEX);
	PyModule_AddIntConstant(poModule, "PLAYER_COLUMN_END_TIME", CPythonTitleSystem::PLAYER_COLUMN_END_TIME);
	PyModule_AddIntConstant(poModule, "PLAYER_COLUMN_IS_EQUIP", CPythonTitleSystem::PLAYER_COLUMN_IS_EQUIP);
	PyModule_AddIntConstant(poModule, "PLAYER_COLUMN_IS_OBTAIN", CPythonTitleSystem::PLAYER_COLUMN_IS_OBTAIN);
	PyModule_AddIntConstant(poModule, "PLAYER_COLUMN_MAX", CPythonTitleSystem::PLAYER_COLUMN_MAX);

	PyModule_AddIntConstant(poModule, "TYPE_NONE", CPythonTitleSystem::TYPE_NONE);
	PyModule_AddIntConstant(poModule, "TYPE_TEXT", CPythonTitleSystem::TYPE_TEXT);
	PyModule_AddIntConstant(poModule, "TYPE_IMAGE", CPythonTitleSystem::TYPE_IMAGE);
	PyModule_AddIntConstant(poModule, "TYPE_EFFECT", CPythonTitleSystem::TYPE_EFFECT);
	PyModule_AddIntConstant(poModule, "TYPE_NAMEPLATE", CPythonTitleSystem::TYPE_NAMEPLATE);

	PyModule_AddIntConstant(poModule, "COLOR_FAMILY_NONE", CPythonTitleSystem::COLOR_FAMILY_NONE);
	PyModule_AddIntConstant(poModule, "COLOR_FAMILY_GOLD", CPythonTitleSystem::COLOR_FAMILY_GOLD);
	PyModule_AddIntConstant(poModule, "COLOR_FAMILY_RED", CPythonTitleSystem::COLOR_FAMILY_RED);
	PyModule_AddIntConstant(poModule, "COLOR_FAMILY_BLUE", CPythonTitleSystem::COLOR_FAMILY_BLUE);

	PyModule_AddStringConstant(poModule, "TITLE_PREVIEW_IMAGE", (char*)CPythonTitleSystem::TITLE_PREVIEW_IMAGE);

	PyObject* pyNameplateDict = PyDict_New();
	const char* npPaths[][4] = {
		{ "1", "d:/ymir work/ui/game/title/title_06_gold_left.tga", "d:/ymir work/ui/game/title/title_06_gold_middle.tga", "d:/ymir work/ui/game/title/title_06_gold_right.tga" },
		{ "2", "d:/ymir work/ui/game/title/title_07_red_left.tga", "d:/ymir work/ui/game/title/title_07_red_middle.tga", "d:/ymir work/ui/game/title/title_07_red_right.tga" },
		{ "3", "d:/ymir work/ui/game/title/title_08_blue_left.tga", "d:/ymir work/ui/game/title/title_08_blue_middle.tga", "d:/ymir work/ui/game/title/title_08_blue_right.tga" },
	};
	for (int i = 0; i < 3; ++i)
	{
		PyObject* pyKey = PyInt_FromLong(atoi(npPaths[i][0]));
		PyObject* pyVal = Py_BuildValue("(sss)", npPaths[i][1], npPaths[i][2], npPaths[i][3]);
		PyDict_SetItem(pyNameplateDict, pyKey, pyVal);
		Py_DECREF(pyKey);
		Py_DECREF(pyVal);
	}
	PyModule_AddObject(poModule, "NAMEPLATE_BY_COLOR", pyNameplateDict);
}
#endif // ENABLE_TITLE_SYSTEM
