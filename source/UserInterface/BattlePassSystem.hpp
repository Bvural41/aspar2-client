#pragma once
#include "StdAfx.h"
#include "BattlePassUtils.hpp"
#ifdef ENABLE_BATTLEPASS
#include <ctime>

extern PyObject* battlepassMainClass;

#define CallPyMethodBattlePass(str, ...)                                                \
	if (battlepassMainClass)                                                            \
		PyCallClassMemberFunc(battlepassMainClass, str, __VA_ARGS__);                   \
	else                                                                                 \
		TraceError("BattlePassManager:: battlepassMainClass is not valid %s", str);

class BattlePassManager : public CSingleton<BattlePassManager>
{
public:
	void LoadBattlePass();

	BattlePassInfo& GetBattlePassInfo() { return m_battlepass_info; }
	std::map<uint32_t /*tier id*/, TierInfo>& GetBattlePassTierMap() { return m_battle_pass_tier_map; }
	std::map<uint32_t /*mission id*/, BattlePassMission>& GetBattlePassMissionMap() { return m_battle_pass_missions; }

private:
	BattlePassInfo m_battlepass_info;
	std::map<uint32_t /*mission id*/, BattlePassMission> m_battle_pass_missions;
	std::map<uint32_t /*tier id*/, TierInfo> m_battle_pass_tier_map;
	bool m_premium_status = false;

public:
	bool GetPremiumStatus() const { return m_premium_status; }

public:
	/*
	*      *** Networking ***
	* Do not touch unless you know what you are doing from now on.
	*/
	static bool RecvBattlePassPacket();
	static void UpdateCharacterData(net_battlepass::BattlePassCharacterInfo data);
	static void UpdateMissionState(net_battlepass::BattlePassCharacterMissionInfo data);
	static void UpdateTierState(net_battlepass::BattlePassCharacterTierInfo data);

};
#endif
