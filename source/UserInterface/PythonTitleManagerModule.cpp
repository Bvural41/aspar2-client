#include "StdAfx.h"
#ifdef ENABLE_TITLE_SYSTEM
#include "PythonTitleManager.h"
#include "PythonApplication.h"

PyObject* titleSystemInitialize(PyObject* poSelf, PyObject* poArgs)
{
	CPythonTitleManager::Instance().Initialize();
	return Py_BuildNone();
}

PyObject* titleSystemClear(PyObject* poSelf, PyObject* poArgs)
{
	CPythonTitleManager::Instance().Clear();
	return Py_BuildNone();
}

PyObject* titleSystemIsDataReceived(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonTitleManager::Instance().IsDataReceived() ? 1 : 0);
}

PyObject* titleSystemGetAllTitleData(PyObject* poSelf, PyObject* poArgs)
{
	const std::map<int, CPythonTitleManager::TTitleData>& dataMap =
		CPythonTitleManager::Instance().GetAllTitleData();

	PyObject* pyDict = PyDict_New();

	for (auto it = dataMap.begin(); it != dataMap.end(); ++it)
	{
		const CPythonTitleManager::TTitleData& td = it->second;

		PyObject* pyTuple = Py_BuildValue("(iissiisii)",
			td.iTitleIndex,
			td.iTitleType,
			td.strName.c_str(),
			td.strConditionTooltip.c_str(),
			td.bIsPermanent ? 1 : 0,
			td.strOpenTime.c_str(),
			(int)td.dwFontColor,
			td.iResourceIndex,
			td.bIsWZ ? 1 : 0
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
	CPythonTitleManager& rkTitleMgr = CPythonTitleManager::Instance();
	const std::map<int, CPythonTitleManager::TPlayerTitleData>& playerMap = rkTitleMgr.GetPlayerTitleMap();

	int iNow = (int)CPythonApplication::Instance().GetServerTimeStamp();

	std::vector<int> expiredTitles;

	PyObject* pyDict = PyDict_New();
	for (auto it = playerMap.begin(); it != playerMap.end(); ++it)
	{
		const CPythonTitleManager::TPlayerTitleData& pd = it->second;

		if (pd.iEndTime > 0 && pd.iEndTime <= iNow)
		{
			expiredTitles.push_back(it->first);
			continue;
		}

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

	for (size_t i = 0; i < expiredTitles.size(); ++i)
		rkTitleMgr.RemovePlayerTitleData(expiredTitles[i]);

	return pyDict;
}

PyObject* titleSystemGetEquippedTitle(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonTitleManager::Instance().GetEquippedTitle());
}

PyObject* titleSystemIsTitleEquipped(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonTitleManager::Instance().IsTitleEquipped() ? 1 : 0);
}

PyObject* titleSystemGetColorFamily(PyObject* poSelf, PyObject* poArgs)
{
	int iTitleIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iTitleIndex))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonTitleManager::Instance().GetColorFamily(iTitleIndex));
}

PyObject* titleSystemGetNameplatePaths(PyObject* poSelf, PyObject* poArgs)
{
	int iColorFamily;
	if (!PyTuple_GetInteger(poArgs, 0, &iColorFamily))
		return Py_BuildException();

	CPythonTitleManager::TNameplatePaths paths;
	if (!CPythonTitleManager::Instance().GetNameplatePaths(iColorFamily, &paths))
		return Py_BuildNone();

	return Py_BuildValue("(sss)", paths.strLeft.c_str(), paths.strMiddle.c_str(), paths.strRight.c_str());
}

PyObject* titleSystemGetPreviewSpriteData(PyObject* poSelf, PyObject* poArgs)
{
	int iTitleIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iTitleIndex))
		return Py_BuildException();

	CPythonTitleManager::TSpritePreviewData spriteData;
	if (!CPythonTitleManager::Instance().GetPreviewSpriteData(iTitleIndex, &spriteData))
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

	CPythonTitleManager::TEffectPreviewData effectData;
	if (!CPythonTitleManager::Instance().GetPreviewEffectData(iTitleIndex, &effectData))
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

	const char* pszTooltip = CPythonTitleManager::Instance().GetItemTooltip(iTitleIndex);
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

	CPythonTitleManager::Instance().SetPlayerTitleData(iTitleIndex, iEndTime, iIsEquip != 0, iIsObtain != 0);
	return Py_BuildNone();
}

PyObject* titleSystemRemovePlayerTitleData(PyObject* poSelf, PyObject* poArgs)
{
	int iTitleIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iTitleIndex))
		return Py_BuildException();

	CPythonTitleManager::Instance().RemovePlayerTitleData(iTitleIndex);
	return Py_BuildNone();
}

PyObject* titleSystemRequestEquip(PyObject* poSelf, PyObject* poArgs)
{
	int iTitleIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iTitleIndex))
		return Py_BuildException();

	CPythonTitleManager::Instance().RequestEquip(iTitleIndex);
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

	CPythonTitleManager::Instance().ShowEffect((DWORD)iVID, iTitleIndex);
	return Py_BuildNone();
}

PyObject* titleSystemHideEffect(PyObject* poSelf, PyObject* poArgs)
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();

	CPythonTitleManager::Instance().HideEffect((DWORD)iVID);
	return Py_BuildNone();
}

PyObject* titleSystemRefreshMyTitle(PyObject* poSelf, PyObject* poArgs)
{
	CPythonTitleManager::Instance().RefreshMyTitle();
	return Py_BuildNone();
}

PyObject* titleSystemSetShow(PyObject* poSelf, PyObject* poArgs)
{
	int bShow;
	if (!PyTuple_GetInteger(poArgs, 0, &bShow))
		return Py_BuildException();

	CPythonTitleManager::Instance().SetShowPreview(bShow != 0);
	return Py_BuildNone();
}

PyObject* titleSystemCreateEffectPreview(PyObject* poSelf, PyObject* poArgs)
{
	int iTitleIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iTitleIndex))
		return Py_BuildException();

	CPythonTitleManager::Instance().CreateEffectPreview(iTitleIndex);
	return Py_BuildNone();
}

PyObject* titleSystemDestroyEffectPreview(PyObject* poSelf, PyObject* poArgs)
{
	CPythonTitleManager::Instance().DestroyEffectPreview();
	return Py_BuildNone();
}

PyObject* titleSystemGetImagePath(PyObject* poSelf, PyObject* poArgs)
{
	int iTitleIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iTitleIndex))
		return Py_BuildException();

	CPythonTitleManager::TImagePreviewData imageData;
	if (!CPythonTitleManager::Instance().GetPreviewImageData(iTitleIndex, &imageData))
		return Py_BuildNone();

	return Py_BuildValue("s", imageData.strImagePath.c_str());
}

PyObject* titleSystemGetBannerImage(PyObject* poSelf, PyObject* poArgs)
{
	int iTitleIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iTitleIndex))
		return Py_BuildException();

	int iColorFamily = CPythonTitleManager::Instance().GetColorFamily(iTitleIndex);
	if (iColorFamily != CPythonTitleManager::COLOR_FAMILY_NONE)
		return Py_BuildValue("s", CPythonTitleManager::TITLE_PREVIEW_IMAGE);

	return Py_BuildValue("s", "");
}

PyObject* titleSystemShowOtherPlayerTitle(PyObject* poSelf, PyObject* poArgs)
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();

	int iTitleIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iTitleIndex))
		return Py_BuildException();

	CPythonTitleManager::TTitleData titleData;
	if (CPythonTitleManager::Instance().GetTitleData(iTitleIndex, &titleData))
		CPythonTitleManager::Instance().ShowEffect((DWORD)iVID, iTitleIndex);
	else
		CPythonTitleManager::Instance().HideEffect((DWORD)iVID);

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

		{ "SetShow", titleSystemSetShow, METH_VARARGS },

		{ "CreateEffectPreview", titleSystemCreateEffectPreview, METH_VARARGS },
		{ "DestroyEffectPreview", titleSystemDestroyEffectPreview, METH_VARARGS },

		{ NULL, NULL, NULL },
	};

	PyObject* poModule = Py_InitModule("titleSystem", s_methods);

	PyModule_AddIntConstant(poModule, "DATA_COLUMN_TITLE_INDEX", CPythonTitleManager::DATA_COLUMN_TITLE_INDEX);
	PyModule_AddIntConstant(poModule, "DATA_COLUMN_TITLE_TYPE", CPythonTitleManager::DATA_COLUMN_TITLE_TYPE);
	PyModule_AddIntConstant(poModule, "DATA_COLUMN_NAME", CPythonTitleManager::DATA_COLUMN_NAME);
	PyModule_AddIntConstant(poModule, "DATA_COLUMN_CONDITION_TOOLTIP", CPythonTitleManager::DATA_COLUMN_CONDITION_TOOLTIP);
	PyModule_AddIntConstant(poModule, "DATA_COLUMN_IS_PERMANENT", CPythonTitleManager::DATA_COLUMN_IS_PERMANENT);
	PyModule_AddIntConstant(poModule, "DATA_COLUMN_OPEN_TIME", CPythonTitleManager::DATA_COLUMN_OPEN_TIME);
	PyModule_AddIntConstant(poModule, "DATA_COLUMN_FONT_COLOR", CPythonTitleManager::DATA_COLUMN_FONT_COLOR);
	PyModule_AddIntConstant(poModule, "DATA_COLUMN_RESOURCE_INDEX", CPythonTitleManager::DATA_COLUMN_RESOURCE_INDEX);
	PyModule_AddIntConstant(poModule, "DATA_COLUMN_IS_WZ", CPythonTitleManager::DATA_COLUMN_IS_WZ);
	PyModule_AddIntConstant(poModule, "DATA_COLUMN_MAX", CPythonTitleManager::DATA_COLUMN_MAX);

	PyModule_AddIntConstant(poModule, "PLAYER_COLUMN_TITLE_INDEX", CPythonTitleManager::PLAYER_COLUMN_TITLE_INDEX);
	PyModule_AddIntConstant(poModule, "PLAYER_COLUMN_END_TIME", CPythonTitleManager::PLAYER_COLUMN_END_TIME);
	PyModule_AddIntConstant(poModule, "PLAYER_COLUMN_IS_EQUIP", CPythonTitleManager::PLAYER_COLUMN_IS_EQUIP);
	PyModule_AddIntConstant(poModule, "PLAYER_COLUMN_IS_OBTAIN", CPythonTitleManager::PLAYER_COLUMN_IS_OBTAIN);
	PyModule_AddIntConstant(poModule, "PLAYER_COLUMN_MAX", CPythonTitleManager::PLAYER_COLUMN_MAX);

	PyModule_AddIntConstant(poModule, "TYPE_NONE", CPythonTitleManager::TYPE_NONE);
	PyModule_AddIntConstant(poModule, "TYPE_TEXT", CPythonTitleManager::TYPE_TEXT);
	PyModule_AddIntConstant(poModule, "TYPE_IMAGE", CPythonTitleManager::TYPE_IMAGE);
	PyModule_AddIntConstant(poModule, "TYPE_EFFECT", CPythonTitleManager::TYPE_EFFECT);
	PyModule_AddIntConstant(poModule, "TYPE_NAMEPLATE", CPythonTitleManager::TYPE_NAMEPLATE);

	PyModule_AddIntConstant(poModule, "COLOR_FAMILY_NONE", CPythonTitleManager::COLOR_FAMILY_NONE);
	PyModule_AddIntConstant(poModule, "COLOR_FAMILY_GOLD", CPythonTitleManager::COLOR_FAMILY_GOLD);
	PyModule_AddIntConstant(poModule, "COLOR_FAMILY_RED", CPythonTitleManager::COLOR_FAMILY_RED);
	PyModule_AddIntConstant(poModule, "COLOR_FAMILY_BLUE", CPythonTitleManager::COLOR_FAMILY_BLUE);

	PyModule_AddStringConstant(poModule, "TITLE_PREVIEW_IMAGE", (char*)CPythonTitleManager::TITLE_PREVIEW_IMAGE);

	PyObject* pyNameplateDict = PyDict_New();
	for (auto it = CPythonTitleManager::ms_NameplateByColor.begin(); it != CPythonTitleManager::ms_NameplateByColor.end(); ++it)
	{
		PyObject* pyVal = Py_BuildValue("(sss)", it->second.strLeft.c_str(), it->second.strMiddle.c_str(), it->second.strRight.c_str());
		PyObject* pyKey = PyInt_FromLong(it->first);
		PyDict_SetItem(pyNameplateDict, pyKey, pyVal);
		Py_DECREF(pyKey);
		Py_DECREF(pyVal);
	}
	PyModule_AddObject(poModule, "NAMEPLATE_BY_COLOR", pyNameplateDict);
}
#endif // ENABLE_TITLE_SYSTEM
