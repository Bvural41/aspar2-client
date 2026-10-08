#ifndef __IS_SERVER_WIKI_INCLUDE__
#pragma once
#endif
#include <array>

#ifndef __INC_IN_GAME_WIKI_H__
#define __INC_IN_GAME_WIKI_H__

#pragma pack(1)

namespace CommonWikiData
{
	static const int MAX_REFINE_COUNT = 200;
	static const int REFINE_MATERIAL_MAX_NUM = 5;

	using TWikiRefineProtoInfo = struct SWikiRefineProtoInfo
	{
		SWikiRefineProtoInfo()
		{
			std::fill(std::begin(item_vnums), std::end(item_vnums), 0);
			std::fill(std::begin(item_counts), std::end(item_counts), 0);
			dwCost = 0;
			bProb = 0;
		}

		std::array<DWORD, REFINE_MATERIAL_MAX_NUM> item_vnums;
		std::array<DWORD, REFINE_MATERIAL_MAX_NUM> item_counts;
		DWORD dwCost;
		BYTE bProb;
	};
	
	typedef struct SRefineMaterialNew
	{
		~SRefineMaterialNew() = default;
		SRefineMaterialNew() {
			vnum = 0;
			count = 0;
		}
		
		DWORD vnum;
		int count;
	} TRefineMaterialNew;
	
	typedef struct SWikiChestInfo
	{
		~SWikiChestInfo() = default;
		SWikiChestInfo(DWORD r_vnum = 0, DWORD r_count = 0) {
			vnum = r_vnum;
			count = r_count;
		}
		
		DWORD vnum;
		int count;
	} TWikiChestInfo;
	
	typedef struct SWikiMobDropInfo
	{
		~SWikiMobDropInfo() = default;
		SWikiMobDropInfo(DWORD r_vnum = 0, DWORD r_count = 0, float r_prob = 0.0f) {
			vnum = r_vnum;
			count = r_count;
			prob = r_prob;
		}
		
		DWORD vnum;
		int count;
		float prob;
	} TWikiMobDropInfo;
	
	typedef struct SWikiRefineInfo
	{
		~SWikiRefineInfo() = default;
		SWikiRefineInfo() {
			index = 0;
			memset(&materials, 0, sizeof(materials));
			mat_count = 0;
			price = 0;
		}
		
		int index;
		TRefineMaterialNew materials[REFINE_MATERIAL_MAX_NUM];
		DWORD mat_count;
		int price;
	} TWikiRefineInfo;
	
	typedef struct SWikiItemOriginInfo
	{
		~SWikiItemOriginInfo() = default;
		SWikiItemOriginInfo(DWORD r_vnum = 0, bool r_is_mob = false) {
			vnum = r_vnum;
			is_mob = r_is_mob;
		}
		
		void set_vnum(DWORD r_vnum) { vnum = r_vnum; }
		void set_is_mob(bool flag) { is_mob = flag; }
		
		DWORD vnum;
		bool is_mob;
	} TWikiItemOriginInfo;
	
	static const int CUBE_MATERIAL_MAX_NUM = 5;

	typedef struct SWikiCubeMaterial {
		DWORD vnum;
		int count;
	} TWikiCubeMaterial;

	typedef struct SWikiCubeRecipe {
		SWikiCubeRecipe() {
			memset(materials, 0, sizeof(materials));
			mat_count = 0;
			reward_count = 0;
			gold = 0;
			gem = 0;
			cheque = 0;
			percent = 0;
			npc = 0;
			memset(category, 0, sizeof(category));
		}
		TWikiCubeMaterial materials[CUBE_MATERIAL_MAX_NUM];
		int mat_count;
		int reward_count;
		int gold;
		int gem;
		int cheque;
		int percent;
		int npc;
		char category[32];
	} TWikiCubeRecipe;
	
	typedef struct SWikiInfoTable
	{
		~SWikiInfoTable() = default;
		SWikiInfoTable() {
			is_common = false;
			refine_infos_count = 0;
			chest_info_count = 0;
			origin_vnum = 0;
		}
		
		bool is_common;
		int refine_infos_count;
		int chest_info_count;
		DWORD origin_vnum;
	} TWikiInfoTable;
}

#pragma pack()
#endif // __INC_IN_GAME_WIKI_H__
