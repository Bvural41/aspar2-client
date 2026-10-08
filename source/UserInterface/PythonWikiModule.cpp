#include "StdAfx.h"
#ifdef ENABLE_INGAME_WIKI
#include "../GameLib/ItemManager.h"
#include "../GameLib/InGameWiki.h"
#include "../EterBase/Stl.h"
#include "PythonNetworkStream.h"
#include "PythonNonPlayer.h"
#include "PythonWikiRenderTarget.h"
#include "PythonApplication.h"
#include "PythonWiki.h"

PyObject* wikiModule = NULL;

PyObject* wikiGetBaseClass(PyObject* poSelf, PyObject* poArgs)
{
	if (!wikiModule)
		return Py_BuildNone();
	
	return Py_BuildValue("O", wikiModule);
}

PyObject* wikiGetChestInfo(PyObject* poSelf, PyObject* poArgs)
{
	int itemVnum;
	if (!PyTuple_GetInteger(poArgs, 0, &itemVnum))
		return Py_BadArgument();
	
	CItemData* table;
	if (CItemManager::instance().GetItemDataPointer(itemVnum, &table) && !table->GetWikiTable()->pChestInfo.empty())
	{
		const size_t dataCount = table->GetWikiTable()->pChestInfo.size();
		PyObject* ret = PyTuple_New(dataCount);
		for (size_t i = 0; i < dataCount; ++i)
			PyTuple_SetItem(ret, i, Py_BuildValue("ii", table->GetWikiTable()->pChestInfo[i].vnum, table->GetWikiTable()->pChestInfo[i].count));
		return Py_BuildValue("iiO", table->GetWikiTable()->dwOrigin, table->GetWikiTable()->bIsCommon ? 1 : 0, ret);
	}
	
	return Py_BuildValue("ii", table ? table->GetWikiTable()->dwOrigin : 0, (table && table->GetWikiTable()->bIsCommon) ? 1 : 0);
}

PyObject* wikiGetChestReviewInfo(PyObject* poSelf, PyObject* poArgs)
{
	int itemVnum;
	if (!PyTuple_GetInteger(poArgs, 0, &itemVnum))
		return Py_BadArgument();

	CPythonWiki::instance().CreateSpecialGroupDetails(itemVnum);

	return Py_BuildNone();
}

PyObject* wikiIsChestHaveDrop(PyObject* poSelf, PyObject* poArgs)
{
	int itemVnum;
	if (!PyTuple_GetInteger(poArgs, 0, &itemVnum))
		return Py_BadArgument();

	const bool ret = CPythonWiki::instance().IsChestHaveDrop(itemVnum);

	return Py_BuildValue("b", ret);
}

PyObject* wikiGetMobInfo(PyObject* poSelf, PyObject* poArgs)
{
	int itemVnum;
	if (!PyTuple_GetInteger(poArgs, 0, &itemVnum))
		return Py_BadArgument();
	
	const CPythonNonPlayer::TWikiInfoTable* mobData = CPythonNonPlayer::instance().GetWikiTable(itemVnum);
	if (mobData && !mobData->dropList.empty())
	{
		const size_t dataCount = mobData->dropList.size();
		PyObject* ret = PyTuple_New(dataCount);

		int idx = 0;
		for (const auto& elem : mobData->dropList) {
			PyTuple_SetItem(ret, idx, Py_BuildValue("ii", elem.vnum, elem.count));
			++idx;
		}

		return Py_BuildValue("O", ret);
	}
	
	return Py_BuildNone();
}

PyObject* wikiLoadClassItems(PyObject* poSelf, PyObject* poArgs)
{
	unsigned char classType;
	int raceFilter;
	
	if (!PyTuple_GetByte(poArgs, 0, &classType))
		return Py_BadArgument();
	
	if (!PyTuple_GetInteger(poArgs, 1, &raceFilter))
		return Py_BadArgument();
	
	return Py_BuildValue("i", CItemManager::Instance().WikiLoadClassItems(classType, raceFilter));
}

PyObject* wikiLoadClassMobs(PyObject* poSelf, PyObject* poArgs)
{
	unsigned char classType;
	if (!PyTuple_GetByte(poArgs, 0, &classType))
		return Py_BadArgument();
	
	unsigned short fromLvl, toLvl;
	if (!PyTuple_GetInteger(poArgs, 1, &fromLvl))
		return Py_BadArgument();
	
	if (!PyTuple_GetInteger(poArgs, 2, &toLvl))
		return Py_BadArgument();
	
	return Py_BuildValue("i", CPythonNonPlayer::instance().WikiLoadClassMobs(classType, fromLvl, toLvl));
}

PyObject* GetWikiModule()
{
	return wikiModule;
}

PyObject* wikiChangePage(PyObject* poSelf, PyObject* poArgs)
{
	unsigned short from;
	unsigned short to;
	int isMob = 0;
	
	if (!PyTuple_GetInteger(poArgs, 0, &from))
		return Py_BadArgument();
	
	if (!PyTuple_GetInteger(poArgs, 1, &to))
		return Py_BadArgument();
	
	if (PyTuple_Size(poArgs) > 2 && !PyTuple_GetInteger(poArgs, 2, &isMob))
		return Py_BadArgument();
	
	if (from > to)
		return Py_BuildException("to must be bigger than from");
	
	std::vector<DWORD>* loadVec;
	
	if (!isMob)
		loadVec = CItemManager::Instance().WikiGetLastItems();
	else
		loadVec = CPythonNonPlayer::instance().WikiGetLastMobs();
	
	if (loadVec && loadVec->size() > 0)
	{
		if (loadVec->size() < to)
			to = (unsigned short)loadVec->size();
		
		if (from > to)
			return Py_BuildException("to must be bigger than from");
		
		PyObject* tuple = PyTuple_New(to - from);
		for (unsigned short i = from; i < to; ++i)
			PyTuple_SetItem(tuple, i - from, Py_BuildValue("i", (*loadVec)[i]));
		
		return tuple;
	}
	
	return Py_BuildNone();
}

PyObject* wikiLoadInfo(PyObject* poSelf, PyObject* poArgs)
{
	long long retID;
	if (!PyTuple_GetLongLong(poArgs, 0, &retID)) {
		return Py_BadArgument();
	}

	int itemVnum;
	if (!PyTuple_GetInteger(poArgs, 1, &itemVnum)) {
		return Py_BadArgument();
	}

	int isMob = 0;
	if (PyTuple_Size(poArgs) > 2 && !PyTuple_GetInteger(poArgs, 2, &isMob)) {
		return Py_BadArgument();
	}

	if (!isMob) {
		CItemData* itemData;
		if (CItemManager::Instance().GetItemDataPointer(itemVnum, &itemData))
		{
			if (!itemData->GetWikiTable()->isSet) {
				CPythonWiki::instance().LoadItemRefineInfos(itemVnum);
				itemData->GetWikiTable()->isSet = true;
			}
		}
	}
	else {
		CPythonNonPlayer::TWikiInfoTable* mobData = CPythonNonPlayer::instance().GetWikiTable(itemVnum);
		if (mobData) {
			if (!mobData->isSet) {
				mobData->isSet = true;
			}
		}
	}
	if (wikiModule) {
		PyCallClassMemberFunc(wikiModule, "BINARY_LoadInfo", Py_BuildValue("(Li)", retID, itemVnum));
	}

	return Py_BuildNone();
}

PyObject* wikiGetCubeRecipes(PyObject* poSelf, PyObject* poArgs)
{
	int rewardVnum;
	if (!PyTuple_GetInteger(poArgs, 0, &rewardVnum))
		return Py_BadArgument();

	const CPythonWiki::CUBE_MAP& cubeMap = CPythonWiki::instance().GetCubeMap();
	CPythonWiki::CUBE_MAP::const_iterator it = cubeMap.find(rewardVnum);
	if (it == cubeMap.end())
		return Py_BuildValue("O", PyTuple_New(0));

	const std::vector<CommonWikiData::TWikiCubeRecipe>& recipes = it->second;
	PyObject* ret = PyTuple_New(recipes.size());
	for (size_t i = 0; i < recipes.size(); ++i)
	{
		const CommonWikiData::TWikiCubeRecipe& recipe = recipes[i];
		PyObject* ret2 = PyTuple_New(CommonWikiData::CUBE_MATERIAL_MAX_NUM);
		for (int j = 0; j < CommonWikiData::CUBE_MATERIAL_MAX_NUM; ++j)
		{
			PyTuple_SetItem(ret2, j, Py_BuildValue("ii", recipe.materials[j].vnum, recipe.materials[j].count));
		}
		PyTuple_SetItem(ret, i, Py_BuildValue("Oiiiiiiis", ret2, recipe.mat_count, recipe.reward_count, recipe.gold, recipe.gem, recipe.cheque, recipe.percent, recipe.npc, recipe.category));
	}
	return Py_BuildValue("O", ret);
}

PyObject* wikiGetCubeCategories(PyObject* poSelf, PyObject* poArgs)
{
	const CPythonWiki::CUBE_MAP& cubeMap = CPythonWiki::instance().GetCubeMap();
	std::vector<std::string> cats;
	for (CPythonWiki::CUBE_MAP::const_iterator it = cubeMap.begin(); it != cubeMap.end(); ++it)
	{
		for (size_t i = 0; i < it->second.size(); ++i)
		{
			std::string cat = it->second[i].category;
			if (!cat.empty() && std::find(cats.begin(), cats.end(), cat) == cats.end())
				cats.push_back(cat);
		}
	}
	
	PyObject* ret = PyTuple_New(cats.size());
	for (size_t i = 0; i < cats.size(); ++i)
		PyTuple_SetItem(ret, i, Py_BuildValue("s", cats[i].c_str()));
	return Py_BuildValue("O", ret);
}

PyObject* wikiGetCubeVnumsByCategory(PyObject* poSelf, PyObject* poArgs)
{
	char* category;
	if (!PyTuple_GetString(poArgs, 0, &category))
		return Py_BadArgument();

	const CPythonWiki::CUBE_MAP& cubeMap = CPythonWiki::instance().GetCubeMap();
	std::vector<DWORD> vnums;
	for (CPythonWiki::CUBE_MAP::const_iterator it = cubeMap.begin(); it != cubeMap.end(); ++it)
	{
		for (size_t i = 0; i < it->second.size(); ++i)
		{
			if (strcmp(it->second[i].category, category) == 0)
			{
				if (std::find(vnums.begin(), vnums.end(), it->first) == vnums.end())
					vnums.push_back(it->first);
				break;
			}
		}
	}

	PyObject* ret = PyTuple_New(vnums.size());
	for (size_t i = 0; i < vnums.size(); ++i)
		PyTuple_SetItem(ret, i, Py_BuildValue("i", vnums[i]));
	return Py_BuildValue("O", ret);
}

PyObject* wikiGetRefineInfo(PyObject* poSelf, PyObject* poArgs)
{
	int itemVnum;
	if (!PyTuple_GetInteger(poArgs, 0, &itemVnum))
		return Py_BadArgument();
	
	CItemData* table;
	if (CItemManager::instance().GetItemDataPointer(itemVnum, &table))
	{
		CPythonWiki::instance().LoadItemRefineInfos(itemVnum);
		const int refineMaxLevel = table->GetWikiTable()->maxRefineLevel;
		if (refineMaxLevel <= 0)
			return Py_BuildValue("iO", 0, PyTuple_New(0));

		PyObject* ret = PyTuple_New(refineMaxLevel);
		
		for (int i = 0; i < refineMaxLevel; ++i)
		{
			PyObject* ret2 = PyTuple_New(CommonWikiData::REFINE_MATERIAL_MAX_NUM);
			
			for (int j = 0; j < CommonWikiData::REFINE_MATERIAL_MAX_NUM; ++j)
			{
				if (i < static_cast<int>(table->GetWikiTable()->pRefineData.size()))
				{
					PyTuple_SetItem(ret2, j, Py_BuildValue("ii", table->GetWikiTable()->pRefineData.at(i).materials[j].vnum, table->GetWikiTable()->pRefineData.at(i).materials[j].count));
				}
				else
				{
					PyTuple_SetItem(ret2, j, Py_BuildValue("ii", 0, 0));
				}
			}
				
			if (i < static_cast<int>(table->GetWikiTable()->pRefineData.size()))
			{
				PyTuple_SetItem(ret, i, Py_BuildValue("iO", table->GetWikiTable()->pRefineData.at(i).price, ret2));
			}
			else
			{
				PyTuple_SetItem(ret, i, Py_BuildValue("iO", 0, ret2));
			}
		}
		return Py_BuildValue("iO", refineMaxLevel, ret);
	}
	
	return Py_BuildNone();
}

PyObject* wikiGetOriginInfo(PyObject* poSelf, PyObject* poArgs)
{
	int itemVnum;
	if (!PyTuple_GetInteger(poArgs, 0, &itemVnum))
		return Py_BadArgument();
	
	CItemData* table;
	if (CItemManager::instance().GetItemDataPointer(itemVnum, &table) && !table->GetWikiTable()->pOriginInfo.empty())
	{
		const CItemData::TWikiItemInfo* wikiTbl = table->GetWikiTable();
		PyObject* ret = PyTuple_New(wikiTbl->pOriginInfo.size());
		for (size_t i = 0; i < wikiTbl->pOriginInfo.size(); ++i)
			PyTuple_SetItem(ret, i, Py_BuildValue("ii", wikiTbl->pOriginInfo[i].vnum, wikiTbl->pOriginInfo[i].is_mob ? 1 : 0));

		return Py_BuildValue("O", ret);
	}
	
	return Py_BuildNone();
}

PyObject* wikiGetWikiItemStartRefineVnum(PyObject* poSelf, PyObject* poArgs)
{
	int itemVnum;
	if (!PyTuple_GetInteger(poArgs, 0, &itemVnum))
		return Py_BadArgument();
	
	return Py_BuildValue("i", CItemManager::instance().GetWikiItemStartRefineVnum(itemVnum));
}

PyObject* wikiIsSet(PyObject* poSelf, PyObject* poArgs)
{
	int itemVnum;
	if (!PyTuple_GetInteger(poArgs, 0, &itemVnum))
		return Py_BadArgument();
	
	bool isMob = false;
	if (PyTuple_Size(poArgs) > 1)
		if (!PyTuple_GetBoolean(poArgs, 1, &isMob))
			return Py_BadArgument();
	
	if (!isMob)
	{
		CItemData* table;
		if (CItemManager::instance().GetItemDataPointer(itemVnum, &table))
			return Py_BuildValue("i", table->GetWikiTable()->isSet ? 1 : 0);
	}
	else
	{
		const CPythonNonPlayer::TWikiInfoTable* ptr = CPythonNonPlayer::instance().GetWikiTable(itemVnum);
		if (ptr)
			return Py_BuildValue("i", ptr->isSet ? 1 : 0);
	}
	
	return Py_BuildValue("i", 0);
}

PyObject* wikiHasData(PyObject* poSelf, PyObject* poArgs)
{
	int itemVnum;
	if (!PyTuple_GetInteger(poArgs, 0, &itemVnum))
		return Py_BadArgument();
	
	CItemData* table;
	if (CItemManager::instance().GetItemDataPointer(itemVnum, &table))
	{
		return Py_BuildValue("i", table->GetWikiTable()->hasData ? 1 : 0);
	}
	
	return Py_BuildValue("i", 0);
}

PyObject* wikiRegisterClass(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* mClass;
	if (!PyTuple_GetObject(poArgs, 0, &mClass))
		return Py_BadArgument();

	wikiModule = mClass;
	return Py_BuildNone();
}

PyObject* wikiUnregisterClass(PyObject* poSelf, PyObject* poArgs)
{
	wikiModule = nullptr;
	return Py_BuildNone();
}

PyObject* wikiRegisterItemBlacklist(PyObject* poSelf, PyObject* poArgs)
{
	int vnum;
	if (!PyTuple_GetInteger(poArgs, 0, &vnum))
		return Py_BadArgument();
	
	CItemManager::instance().WikiAddVnumToBlacklist(vnum);
	return Py_BuildNone();
}

PyObject* wikiRegisterMonsterBlacklist(PyObject* poSelf, PyObject* poArgs)
{
	int vnum;
	if (!PyTuple_GetInteger(poArgs, 0, &vnum))
		return Py_BadArgument();
	
	CPythonNonPlayer::instance().WikiSetBlacklisted(vnum);
	return Py_BuildNone();
}

PyObject* wikiCanIncrRefineLevel(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CItemManager::instance().CanIncrRefineLevel());
}

PyObject* wikiRegisterModelViewWindow(PyObject* poSelf, PyObject* poArgs)
{
	int moduleID;
	if (!PyTuple_GetInteger(poArgs, 0, &moduleID))
		return Py_BadArgument();
	
	UI::CUiWikiRenderTarget* hwnd = nullptr;
	if (!PyTuple_GetPointer(poArgs, 1, &hwnd))
		return Py_BadArgument();
	
	CPythonWikiRenderTarget::instance().RegisterRenderModule(moduleID, hwnd);
	return Py_BuildNone();
}

PyObject* wikiGetFreeModelViewID(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonWikiRenderTarget::instance().GetFreeID());
}

PyObject* wikiManageModelViewVisibility(PyObject* poSelf, PyObject* poArgs)
{
	int moduleID;
	if (!PyTuple_GetInteger(poArgs, 0, &moduleID))
		return Py_BadArgument();
	
	bool flag = false;
	if (!PyTuple_GetBoolean(poArgs, 1, &flag))
		return Py_BadArgument();
	
	CPythonWikiRenderTarget::instance().ManageModelViewVisibility(moduleID, flag);
	return Py_BuildNone();
}

PyObject* wikiShowModelViewManager(PyObject* poSelf, PyObject* poArgs)
{
	bool flag = false;
	if (!PyTuple_GetBoolean(poArgs, 0, &flag))
		return Py_BadArgument();
	
	CPythonWikiRenderTarget::instance().ShowModelViewManager(flag);
	return Py_BuildNone();
}

PyObject* wikiSetModelViewModel(PyObject* poSelf, PyObject* poArgs)
{
	int moduleID;
	if (!PyTuple_GetInteger(poArgs, 0, &moduleID))
		return Py_BadArgument();
	
	int moduleVnum;
	if (!PyTuple_GetInteger(poArgs, 1, &moduleVnum))
		return Py_BadArgument();
	
	CPythonWikiRenderTarget::instance().SetModelViewModel(moduleID, moduleVnum);
	return Py_BuildNone();
}

PyObject* wikiSetWeaponModel(PyObject* poSelf, PyObject* poArgs)
{
	int moduleID;
	if (!PyTuple_GetInteger(poArgs, 0, &moduleID))
		return Py_BadArgument();
	
	int weaponVnum;
	if (!PyTuple_GetInteger(poArgs, 1, &weaponVnum))
		return Py_BadArgument();
	
	CPythonWikiRenderTarget::instance().SetWeaponModel(moduleID, weaponVnum);
	return Py_BuildNone();
}

PyObject* wikiSetModelForm(PyObject* poSelf, PyObject* poArgs)
{
	int moduleID;
	if (!PyTuple_GetInteger(poArgs, 0, &moduleID))
		return Py_BadArgument();
	
	int mainVnum;
	if (!PyTuple_GetInteger(poArgs, 1, &mainVnum))
		return Py_BadArgument();
	
	CPythonWikiRenderTarget::instance().SetModelForm(moduleID, mainVnum);
	return Py_BuildNone();
}

PyObject* wikiSetModelHair(PyObject* poSelf, PyObject* poArgs)
{
	int moduleID;
	if (!PyTuple_GetInteger(poArgs, 0, &moduleID))
		return Py_BadArgument();
	
	int hairVnum;
	if (!PyTuple_GetInteger(poArgs, 1, &hairVnum))
		return Py_BadArgument();
	
	CPythonWikiRenderTarget::instance().SetModelHair(moduleID, hairVnum);
	return Py_BuildNone();
}

PyObject* wikiSetModelSash(PyObject* poSelf, PyObject* poArgs)
{
	int moduleID;
	if (!PyTuple_GetInteger(poArgs, 0, &moduleID))
		return Py_BadArgument();
	
	int sashVnum;
	if (!PyTuple_GetInteger(poArgs, 1, &sashVnum))
		return Py_BadArgument();
	
	CPythonWikiRenderTarget::instance().SetModelSash(moduleID, sashVnum);
	return Py_BuildNone();
}

PyObject* wikiSetModelWeaponEffect(PyObject* poSelf, PyObject* poArgs)
{
	int moduleID;
	if (!PyTuple_GetInteger(poArgs, 0, &moduleID))
		return Py_BadArgument();
	
	int weaponeffectVnum;
	if (!PyTuple_GetInteger(poArgs, 1, &weaponeffectVnum))
		return Py_BadArgument();
	
	return Py_BuildNone();
}

PyObject* wikiSetModelArmorEffect(PyObject* poSelf, PyObject* poArgs)
{
	int moduleID;
	if (!PyTuple_GetInteger(poArgs, 0, &moduleID))
		return Py_BadArgument();
	
	int armoreffectVnum;
	if (!PyTuple_GetInteger(poArgs, 1, &armoreffectVnum))
		return Py_BadArgument();
	
	return Py_BuildNone();
}

PyObject* wikiSetModelV3Eye(PyObject* poSelf, PyObject* poArgs)
{
	int moduleID;
	if (!PyTuple_GetInteger(poArgs, 0, &moduleID))
		return Py_BadArgument();
	
	float x, y, z;
	if (!PyTuple_GetFloat(poArgs, 1, &x))
		return Py_BadArgument();

	if (!PyTuple_GetFloat(poArgs, 2, &y))
		return Py_BadArgument();

	if (!PyTuple_GetFloat(poArgs, 3, &z))
		return Py_BadArgument();
	
	CPythonWikiRenderTarget::instance().SetModelV3Eye(moduleID, x, y, z);
	return Py_BuildNone();
}

PyObject* wikiSetModelV3Target(PyObject* poSelf, PyObject* poArgs)
{
	int moduleID;
	if (!PyTuple_GetInteger(poArgs, 0, &moduleID))
		return Py_BadArgument();
	
	float x, y, z;
	if (!PyTuple_GetFloat(poArgs, 1, &x))
		return Py_BadArgument();

	if (!PyTuple_GetFloat(poArgs, 2, &y))
		return Py_BadArgument();

	if (!PyTuple_GetFloat(poArgs, 3, &z))
		return Py_BadArgument();
	
	CPythonWikiRenderTarget::instance().SetModelV3Target(moduleID, x, y, z);
	return Py_BuildNone();
}

PyObject* wikiRenderTargetSetBackground(PyObject* poSelf, PyObject* poArgs)
{
	int moduleID;
	if (!PyTuple_GetInteger(poArgs, 0, &moduleID))
		return Py_BadArgument();

	char * szPathName;
	if (!PyTuple_GetString(poArgs, 1, &szPathName))
		return Py_BadArgument();

	int iWidth;
	if (!PyTuple_GetInteger(poArgs, 2, &iWidth))
		iWidth = CPythonApplication::Instance().GetWidth();

	int iHeight;
	if (!PyTuple_GetInteger(poArgs, 3, &iHeight))
		iHeight = CPythonApplication::Instance().GetHeight();

	CPythonWikiRenderTarget::instance().CreateBackground(moduleID, szPathName, iWidth, iHeight);
	return Py_BuildNone();
}

void initWiki()
{
	static PyMethodDef s_methods[] =
	{
		{ "GetBaseClass",						wikiGetBaseClass,					METH_VARARGS },
		{ "GetChestInfo",						wikiGetChestInfo,					METH_VARARGS },
		{ "CreateDropInfo",						wikiGetChestReviewInfo,				METH_VARARGS },
		{ "IsChestHaveDrop",					wikiIsChestHaveDrop,				METH_VARARGS },
		{ "LoadClassItems",						wikiLoadClassItems,					METH_VARARGS },
		{ "ChangePage",							wikiChangePage,						METH_VARARGS },
		{ "LoadInfo",							wikiLoadInfo,						METH_VARARGS },
		{ "RegisterClass",						wikiRegisterClass,					METH_VARARGS },
		{ "UnregisterClass",					wikiUnregisterClass,				METH_VARARGS },
		{ "HasData",							wikiHasData,						METH_VARARGS },
		{ "IsSet",								wikiIsSet,							METH_VARARGS },
		{ "GetRefineInfo",						wikiGetRefineInfo,					METH_VARARGS },
		{ "GetCubeRecipes",						wikiGetCubeRecipes,					METH_VARARGS },
		{ "GetCubeCategories",					wikiGetCubeCategories,				METH_VARARGS },
		{ "GetCubeVnumsByCategory",				wikiGetCubeVnumsByCategory,			METH_VARARGS },
		{ "GetOriginInfo",						wikiGetOriginInfo,					METH_VARARGS },
		{ "GetWikiItemStartRefineVnum",			wikiGetWikiItemStartRefineVnum,		METH_VARARGS },
		
		{ "RegisterItemBlacklist",				wikiRegisterItemBlacklist,			METH_VARARGS },
		{ "RegisterMonsterBlacklist",			wikiRegisterMonsterBlacklist,		METH_VARARGS },
		
		{ "LoadClassMobs",						wikiLoadClassMobs,					METH_VARARGS },
		{ "GetMobInfo",							wikiGetMobInfo,						METH_VARARGS },
		
		{ "CanIncrRefineLevel",					wikiCanIncrRefineLevel,				METH_VARARGS },
		
		{ "GetFreeModelViewID",					wikiGetFreeModelViewID,				METH_VARARGS },
		{ "RegisterModelViewWindow",			wikiRegisterModelViewWindow,		METH_VARARGS },
		{ "ManageModelViewVisibility",			wikiManageModelViewVisibility,		METH_VARARGS },
		{ "ShowModelViewManager",				wikiShowModelViewManager,			METH_VARARGS },
		
		{ "SetModelViewModel",					wikiSetModelViewModel,				METH_VARARGS },
		{ "SetWeaponModel",						wikiSetWeaponModel,					METH_VARARGS },
		{ "SetModelForm",						wikiSetModelForm,					METH_VARARGS },
		{ "SetModelHair",						wikiSetModelHair,					METH_VARARGS },
		{ "SetModelSash",						wikiSetModelSash,					METH_VARARGS },
		{ "SetModelWeaponEffect",				wikiSetModelWeaponEffect,			METH_VARARGS },
		{ "SetModelArmorEffect",				wikiSetModelArmorEffect,			METH_VARARGS },
		{ "SetModelV3Eye",						wikiSetModelV3Eye,					METH_VARARGS },
		{ "SetModelV3Target",					wikiSetModelV3Target,				METH_VARARGS },
		{ "RenderTargetSetBackground",			wikiRenderTargetSetBackground,		METH_VARARGS },
		{ NULL,									NULL,								NULL		 }
	};
	
	PyObject* module = Py_InitModule("wiki", s_methods);
	
	PyModule_AddIntConstant(module, "MAX_REFINE_COUNT", CommonWikiData::MAX_REFINE_COUNT);
	PyModule_AddIntConstant(module, "REFINE_MATERIAL_MAX_NUM", CommonWikiData::REFINE_MATERIAL_MAX_NUM);
	PyModule_AddIntConstant(module, "CONTROL_ITEM_VNUM", 10);
	
	PyModule_AddIntConstant(module, "WIKI_RENDER_MODULE_DELETE", CPythonWikiRenderTarget::DELETE_PARM);
	PyModule_AddIntConstant(module, "WIKI_RENDER_MODULE_START", CPythonWikiRenderTarget::START_MODULE);
}
#endif
