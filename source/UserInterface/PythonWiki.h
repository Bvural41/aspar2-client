#pragma once

#include "../GameLib/InGameWiki.h"
#include <unordered_map>
#include <vector>

class CPythonWiki : public CSingleton<CPythonWiki>
{
public:
	typedef std::unordered_map<DWORD, std::vector<CommonWikiData::TWikiMobDropInfo> > MOB_PROTO_MAP;
	typedef std::unordered_map<UINT, CommonWikiData::TWikiRefineProtoInfo> REFINE_PROTO_MAP;
	typedef std::unordered_map<DWORD, std::vector<CommonWikiData::TWikiChestInfo> > SPECIAL_GROUP;
	typedef std::unordered_map<DWORD, std::vector<CommonWikiData::TWikiCubeRecipe> > CUBE_MAP;

public:
	CPythonWiki();
	virtual ~CPythonWiki();

	bool	LoadJsonFiles();
	bool	ReadMobDrop(const char* fileName);
	bool	ReadSpecialItemGroup(const char* fileName);
	bool	ReadRefineProto(const char* fileName);
	bool	ReadCubeProto(const char* fileName);
	void	LoadWikiDatas(bool isMob);
	void	LoadItemRefineInfos(DWORD itemVnum);
	void	CreateSpecialGroupDetails(DWORD dwVnum);
	bool	IsChestHaveDrop(const DWORD dwVnum) const;
	void	LoadMobDropInfos(DWORD vnum);
	const CUBE_MAP& GetCubeMap() const { return m_CubeMap; }

private:
	MOB_PROTO_MAP		m_MobProtoMap;
	REFINE_PROTO_MAP	m_RefineProtoMap;
	SPECIAL_GROUP		m_SpecialGroupMap;
	CUBE_MAP			m_CubeMap;
};