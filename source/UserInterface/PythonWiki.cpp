#include "StdAfx.h"
#ifdef ENABLE_INGAME_WIKI
#include "PythonWiki.h"
#include <picojson.h>
#include "../eterPack/EterPackManager.h"
#include "../GameLib/ItemManager.h"
#include "PythonNetworkStream.h"
#include "pythonnonplayer.h"

CPythonWiki::CPythonWiki()
{
}

CPythonWiki::~CPythonWiki()
{
}

bool CPythonWiki::LoadJsonFiles()
{
	if (!ReadRefineProto("locale/common/wiki_json/refine_proto.json"))
		return false;

	if (!ReadMobDrop("locale/common/wiki_json/mob_drop_item.json"))
		return false;

	if (!ReadSpecialItemGroup("locale/common/wiki_json/special_item_group.json"))
		return false;

	if (!ReadCubeProto("locale/common/wiki_json/cube_proto.json"))
		return false;

	LoadWikiDatas(false);
	LoadWikiDatas(true);

	return true;
}

bool CPythonWiki::ReadRefineProto(const char* fileName)
{
	CMappedFile file;
	LPCVOID pvData = NULL;
	if (CEterPackManager::Instance().Get(file, fileName, &pvData))
	{
		const char* pszJson = (const char*)pvData;
		size_t size = file.Size();
		picojson::value v;
		std::string err = picojson::parse(v, pszJson, pszJson + size);
		if (!err.empty() || !v.is_object())
		{
			TraceError("ReadRefineProto parse error: %s in file: %s", err.c_str(), fileName);
			return false;
		}

		const picojson::object& obj = v.get_object();
		for (picojson::object::const_iterator it = obj.begin(); it != obj.end(); ++it)
		{
			UINT refine_id = static_cast<UINT>(atoi(it->first.c_str()));
			if (!it->second.is_object())
				continue;

			const picojson::object& subObj = it->second.get_object();
			CommonWikiData::TWikiRefineProtoInfo info;
			info.dwCost = subObj.count("refine_cost") ? static_cast<DWORD>(subObj.at("refine_cost").get_number()) : 0;
			info.bProb = subObj.count("prob") ? static_cast<BYTE>(subObj.at("prob").get_number()) : 0;

			for (int k = 0; k < CommonWikiData::REFINE_MATERIAL_MAX_NUM; ++k)
			{
				char keyVnum[32];
				char keyCount[32];
				sprintf(keyVnum, "refine_vnum%d", k);
				sprintf(keyCount, "refine_count%d", k);
				DWORD vnum = subObj.count(keyVnum) ? static_cast<DWORD>(subObj.at(keyVnum).get_number()) : 0;
				DWORD count = subObj.count(keyCount) ? static_cast<DWORD>(subObj.at(keyCount).get_number()) : 0;
				info.item_vnums[k] = vnum;
				info.item_counts[k] = count;
			}

			m_RefineProtoMap[refine_id] = info;
		}
		return true;
	}
	return false;
}

bool CPythonWiki::ReadMobDrop(const char* fileName)
{
	CMappedFile file;
	LPCVOID pvData = NULL;
	if (CEterPackManager::Instance().Get(file, fileName, &pvData))
	{
		const char* pszJson = (const char*)pvData;
		size_t size = file.Size();
		picojson::value v;
		std::string err = picojson::parse(v, pszJson, pszJson + size);
		if (!err.empty() || !v.is_object())
		{
			TraceError("ReadMobDrop parse error: %s in file: %s", err.c_str(), fileName);
			return false;
		}

		const picojson::object& obj = v.get_object();
		for (picojson::object::const_iterator it = obj.begin(); it != obj.end(); ++it)
		{
			DWORD mobVnum = static_cast<DWORD>(atoi(it->first.c_str()));
			if (!it->second.is_object())
				continue;

			const picojson::object& subObj = it->second.get_object();
			if (!subObj.count("vnums") || !subObj.at("vnums").is_array() ||
				!subObj.count("counts") || !subObj.at("counts").is_array())
				continue;

			const picojson::array& vnums = subObj.at("vnums").get_array();
			const picojson::array& counts = subObj.at("counts").get_array();
			const picojson::array* probs = NULL;
			if (subObj.count("probs") && subObj.at("probs").is_array())
				probs = &subObj.at("probs").get_array();

			std::vector<CommonWikiData::TWikiMobDropInfo>& mob_vec = m_MobProtoMap[mobVnum];
			size_t total = min(vnums.size(), counts.size());
			for (size_t i = 0; i < total; ++i)
			{
				DWORD itemVnum = static_cast<DWORD>(vnums[i].get_number());
				int count = static_cast<int>(counts[i].get_number());
				float prob = (probs && i < probs->size()) ? static_cast<float>((*probs)[i].get_number()) : 0.0f;
				if (itemVnum == 0 || count == 0)
					continue;

				CommonWikiData::TWikiMobDropInfo tmpStruct;
				tmpStruct.vnum = itemVnum;
				tmpStruct.count = count;
				tmpStruct.prob = prob;
				mob_vec.push_back(tmpStruct);
			}
		}
		return true;
	}
	return false;
}

bool CPythonWiki::ReadSpecialItemGroup(const char* fileName)
{
	CMappedFile file;
	LPCVOID pvData = NULL;
	if (CEterPackManager::Instance().Get(file, fileName, &pvData))
	{
		const char* pszJson = (const char*)pvData;
		size_t size = file.Size();
		picojson::value v;
		std::string err = picojson::parse(v, pszJson, pszJson + size);
		if (!err.empty() || !v.is_object())
		{
			TraceError("ReadSpecialItemGroup parse error: %s in file: %s", err.c_str(), fileName);
			return false;
		}

		const picojson::object& obj = v.get_object();
		for (picojson::object::const_iterator it = obj.begin(); it != obj.end(); ++it)
		{
			DWORD chestVnum = static_cast<DWORD>(atoi(it->first.c_str()));
			if (!it->second.is_object())
				continue;

			const picojson::object& subObj = it->second.get_object();
			if (!subObj.count("vnums") || !subObj.at("vnums").is_array() ||
				!subObj.count("counts") || !subObj.at("counts").is_array())
				continue;

			const picojson::array& vnums = subObj.at("vnums").get_array();
			const picojson::array& counts = subObj.at("counts").get_array();

			std::vector<CommonWikiData::TWikiChestInfo>& item_vec = m_SpecialGroupMap[chestVnum];
			size_t total = min(vnums.size(), counts.size());
			for (size_t i = 0; i < total; ++i)
			{
				DWORD itemVnum = static_cast<DWORD>(vnums[i].get_number());
				int count = static_cast<int>(counts[i].get_number());
				if (itemVnum == 0 || count == 0)
					continue;

				item_vec.push_back(CommonWikiData::TWikiChestInfo(itemVnum, count));
			}
		}
		return true;
	}
	return false;
}

bool CPythonWiki::ReadCubeProto(const char* fileName)
{
	CMappedFile file;
	LPCVOID pvData = NULL;
	if (CEterPackManager::Instance().Get(file, fileName, &pvData))
	{
		const char* pszJson = (const char*)pvData;
		size_t size = file.Size();
		picojson::value v;
		std::string err = picojson::parse(v, pszJson, pszJson + size);
		if (!err.empty() || !v.is_object())
		{
			TraceError("ReadCubeProto parse error: %s in file: %s", err.c_str(), fileName);
			return false;
		}

		const picojson::object& obj = v.get_object();
		for (picojson::object::const_iterator it = obj.begin(); it != obj.end(); ++it)
		{
			DWORD rewardVnum = static_cast<DWORD>(atoi(it->first.c_str()));
			if (!it->second.is_array())
				continue;

			const picojson::array& arr = it->second.get_array();
			std::vector<CommonWikiData::TWikiCubeRecipe>& recipe_vec = m_CubeMap[rewardVnum];

			for (size_t i = 0; i < arr.size(); ++i)
			{
				if (!arr[i].is_object()) continue;
				const picojson::object& subObj = arr[i].get_object();

				CommonWikiData::TWikiCubeRecipe recipe;
				recipe.gold = subObj.count("gold") ? static_cast<int>(subObj.at("gold").get_number()) : 0;
				recipe.gem = subObj.count("gem") ? static_cast<int>(subObj.at("gem").get_number()) : 0;
				recipe.cheque = subObj.count("cheque") ? static_cast<int>(subObj.at("cheque").get_number()) : 0;
				recipe.percent = subObj.count("percent") ? static_cast<int>(subObj.at("percent").get_number()) : 0;
				recipe.npc = subObj.count("npc") ? static_cast<int>(subObj.at("npc").get_number()) : 0;
				recipe.reward_count = subObj.count("reward_count") ? static_cast<int>(subObj.at("reward_count").get_number()) : 1;
				
				if (subObj.count("category") && subObj.at("category").is_string()) {
					strncpy(recipe.category, subObj.at("category").get_string().c_str(), sizeof(recipe.category) - 1);
				}

				if (subObj.count("items") && subObj.at("items").is_array()) {
					const picojson::array& items_arr = subObj.at("items").get_array();
					int mat_idx = 0;
					for (size_t k = 0; k < items_arr.size() && mat_idx < CommonWikiData::CUBE_MATERIAL_MAX_NUM; ++k) {
						if (!items_arr[k].is_array()) continue;
						const picojson::array& item_pair = items_arr[k].get_array();
						if (item_pair.size() >= 2) {
							recipe.materials[mat_idx].vnum = static_cast<DWORD>(item_pair[0].get_number());
							recipe.materials[mat_idx].count = static_cast<int>(item_pair[1].get_number());
							mat_idx++;
						}
					}
					recipe.mat_count = mat_idx;
				}
				recipe_vec.push_back(recipe);
			}
		}
		return true;
	}
	return false;
}

void CPythonWiki::LoadWikiDatas(const bool isMob)
{
	if (!isMob)
	{
		for (SPECIAL_GROUP::iterator it = m_SpecialGroupMap.begin(); it != m_SpecialGroupMap.end(); ++it)
		{
			DWORD chest_vnum = it->first;
			std::vector<CommonWikiData::TWikiChestInfo>& chest_vec = it->second;

			CItemData* pData = NULL;
			if (!CItemManager::Instance().GetItemDataPointer(chest_vnum, &pData) || !pData)
				continue;

			CItemData::TWikiItemInfo* wikiInfo = pData->GetWikiTable();
			if (!wikiInfo)
				continue;

			if (!wikiInfo->isSet)
			{
				wikiInfo->isSet = true;
				wikiInfo->hasData = true;
				wikiInfo->pChestInfo = chest_vec;
			}

			for (size_t i = 0; i < chest_vec.size(); ++i)
			{
				CItemData* pItemData = NULL;
				if (CItemManager::Instance().GetItemDataPointer(chest_vec[i].vnum, &pItemData) && pItemData)
				{
					CItemData::TWikiItemInfo* pItemWikiInfo = pItemData->GetWikiTable();
					if (pItemWikiInfo)
					{
						bool bAlready = false;
						for (size_t j = 0; j < pItemWikiInfo->pOriginInfo.size(); ++j)
						{
							if (pItemWikiInfo->pOriginInfo[j].vnum == chest_vnum && !pItemWikiInfo->pOriginInfo[j].is_mob)
							{
								bAlready = true;
								break;
							}
						}
						if (!bAlready)
							pItemWikiInfo->pOriginInfo.push_back(CommonWikiData::TWikiItemOriginInfo(chest_vnum, false));
					}
				}
			}
		}
	}
	else
	{
		for (MOB_PROTO_MAP::iterator it = m_MobProtoMap.begin(); it != m_MobProtoMap.end(); ++it)
		{
			DWORD mob_vnum = it->first;
			std::vector<CommonWikiData::TWikiMobDropInfo>& mob_vec = it->second;

			CPythonNonPlayer::TWikiInfoTable* mobData = CPythonNonPlayer::Instance().GetWikiTable(mob_vnum);
			if (!mobData)
				continue;

			if (!mobData->isSet)
			{
				mobData->isSet = (m_MobProtoMap.size() > 0);
				mobData->dropList = mob_vec;
			}

			for (size_t i = 0; i < mob_vec.size(); ++i)
			{
				CItemData* pItemData = NULL;
				if (CItemManager::Instance().GetItemDataPointer(mob_vec[i].vnum, &pItemData) && pItemData)
				{
					CItemData::TWikiItemInfo* pItemWikiInfo = pItemData->GetWikiTable();
					if (pItemWikiInfo)
					{
						bool bAlready = false;
						for (size_t j = 0; j < pItemWikiInfo->pOriginInfo.size(); ++j)
						{
							if (pItemWikiInfo->pOriginInfo[j].vnum == mob_vnum && pItemWikiInfo->pOriginInfo[j].is_mob)
							{
								bAlready = true;
								break;
							}
						}
						if (!bAlready)
							pItemWikiInfo->pOriginInfo.push_back(CommonWikiData::TWikiItemOriginInfo(mob_vnum, true));
					}
				}
			}
		}
	}
}

void CPythonWiki::LoadMobDropInfos(const DWORD vnum)
{
	if (m_MobProtoMap.empty()) {
		CPythonNetworkStream::Instance().SendPythonFuncWithoutParameter("CloseTargetBoard");
		return;
	}

	MOB_PROTO_MAP::iterator it = m_MobProtoMap.find(vnum);
	if (it == m_MobProtoMap.end()) {
		CPythonNetworkStream::Instance().SendPythonFuncWithoutParameter("CloseTargetBoard");
		return;
	}

	for (size_t i = 0; i < it->second.size(); ++i)
	{
		const CommonWikiData::TWikiMobDropInfo& drop = it->second[i];
		PyObject* dropInfo = PyList_New(0);
		PyList_Append(dropInfo, Py_BuildValue("(iiif)", vnum, drop.vnum, drop.count, drop.prob));
		CPythonNetworkStream::Instance().SendPythonFunc(dropInfo, "BINARY_AddTargetMonsterDropInfo");
	}
	CPythonNetworkStream::Instance().SendPythonFunc(Py_BuildValue("(i)", vnum), "BINARY_RefreshTargetMonsterDropInfo");
}

void CPythonWiki::LoadItemRefineInfos(const DWORD itemVnum)
{
	CItemData* pData = NULL;
	if (!CItemManager::Instance().GetItemDataPointer(itemVnum, &pData) || !pData)
		return;

	CItemData::TWikiItemInfo* wikiInfo = pData->GetWikiTable();
	if (!wikiInfo)
		return;

	if (wikiInfo->isSet)
		return;

	wikiInfo->isSet = true;
	wikiInfo->hasData = true;
	wikiInfo->pRefineData.clear();

	const CItemManager::TItemMap& mp = CItemManager::Instance().GetItemMap();
	std::string baseItemName = CItemManager::Instance().GetWikiItemBaseRefineName(itemVnum);
	DWORD currentVnum = itemVnum;

	for (BYTE i = 0; i < CommonWikiData::MAX_REFINE_COUNT; ++i)
	{
		CItemManager::TItemMap::const_iterator findItem = mp.find(currentVnum);
		if (findItem == mp.end() || !findItem->second)
			break;

		CItemData* pCurItem = findItem->second;
		WORD refineSet = pCurItem->GetRefineSet();
		DWORD refinedVnum = pCurItem->GetRefinedVnum();

		if (refineSet == 0)
			break;

		if (refinedVnum == 0)
			break;

		if (!baseItemName.empty())
		{
			std::string nextItemName = CItemManager::Instance().GetWikiItemBaseRefineName(refinedVnum);
			if (baseItemName != nextItemName)
				break;
		}

		REFINE_PROTO_MAP::iterator it = m_RefineProtoMap.find(refineSet);
		if (it == m_RefineProtoMap.end())
			break;

		const CommonWikiData::TWikiRefineProtoInfo& info = it->second;
		CommonWikiData::TWikiRefineInfo tmpStruct;
		tmpStruct.index = i;
		tmpStruct.price = static_cast<int>(info.dwCost);
		tmpStruct.mat_count = 0;

		for (size_t idx = 0; idx < CommonWikiData::REFINE_MATERIAL_MAX_NUM; ++idx)
		{
			if (info.item_vnums[idx] > 0 && info.item_counts[idx] > 0)
			{
				tmpStruct.materials[tmpStruct.mat_count].vnum = info.item_vnums[idx];
				tmpStruct.materials[tmpStruct.mat_count].count = info.item_counts[idx];
				tmpStruct.mat_count++;
			}
		}
		wikiInfo->pRefineData.push_back(tmpStruct);

		currentVnum = refinedVnum;
	}

	wikiInfo->maxRefineLevel = static_cast<int>(wikiInfo->pRefineData.size());
}

void CPythonWiki::CreateSpecialGroupDetails(const DWORD dwVnum)
{
	SPECIAL_GROUP::iterator it = m_SpecialGroupMap.find(dwVnum);
	if (it != m_SpecialGroupMap.end())
	{
		PyObject* specialInfo = PyList_New(0);
		for (size_t i = 0; i < it->second.size(); ++i)
		{
			PyList_Append(specialInfo, Py_BuildValue("(ii)", it->second[i].vnum, it->second[i].count));
		}
		CPythonNetworkStream::Instance().SendPythonFunc(specialInfo, "LoadWikiSpecialGroupInfo");
	}
}

bool CPythonWiki::IsChestHaveDrop(const DWORD dwVnum) const
{
	SPECIAL_GROUP::const_iterator it = m_SpecialGroupMap.find(dwVnum);
	return it != m_SpecialGroupMap.end();
}
#endif
