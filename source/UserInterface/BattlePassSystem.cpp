#include "StdAfx.h"
#ifdef ENABLE_BATTLEPASS
#include "BattlePassSystem.hpp"
#include "PythonNetworkStream.h"
#include <fstream>
#include <picojson.h>
#include <chrono>
#include "../eterPack/EterPackManager.h"

PyObject* battlepassMainClass = NULL;

void BattlePassManager::LoadBattlePass()
{
	m_battle_pass_missions.clear();
	m_battle_pass_tier_map.clear();

	std::string file_name = "locale/common/battlepass_config.json";

	const VOID* pvData;
	CMappedFile file;
	if (!CEterPackManager::Instance().Get(file, file_name.c_str(), &pvData))
	{
		TraceError("Check your locale/common folder!, battlepass_config.json is not found!");
		return;
	}

	picojson::value jf;
	std::string err = picojson::parse(jf, (const char*)pvData, (const char*)pvData + file.Size());
	if (!err.empty() || !jf.is_object())
	{
		TraceError("BattlePass json parse error: %s", err.c_str());
		return;
	}

	if (jf.contains("general") && jf.get("general").is_object())
	{
		const picojson::value& general = jf.get("general");

		if (general.contains("id") && general.contains("start_date") && general.contains("end_date"))
		{
			m_battlepass_info.id = static_cast<uint32_t>(general.get("id").get_number());
			m_battlepass_info.start_date = static_cast<int64_t>(general.get("start_date").get_number());
			m_battlepass_info.end_date = static_cast<int64_t>(general.get("end_date").get_number());
		}
		else
		{
			TraceError("Error: Missing id or start_date or end_date in 'general' key in battlepass_config.json");
		}
	}

	if (jf.contains("missions") && jf.get("missions").is_array())
	{
		const picojson::array& missions = jf.get("missions").get_array();
		for (size_t it = 0; it < missions.size(); ++it)
		{
			const picojson::value& mission = missions[it];
			if (!mission.is_object())
				continue;

			if (mission.contains("mission_index") &&
				mission.contains("mission_type") &&
				mission.contains("target_object") &&
				mission.contains("target_count") &&
				mission.contains("mission_exp_reward"))
			{
				int32_t mission_type = GetBattlePassMissionIndex(mission.get("mission_type").get_string());

				if (mission_type == -1)
				{
					TraceError("Invalid Mission_Type %s", mission.get("mission_type").get_string().c_str());
					continue;
				}

				uint16_t min_level = 1;
				uint16_t max_level = 120;

				if (mission.contains("min_level"))
					min_level = static_cast<uint16_t>(mission.get("min_level").get_number());

				if (mission.contains("max_level"))
					max_level = static_cast<uint16_t>(mission.get("max_level").get_number());

				uint32_t mission_index = static_cast<uint32_t>(mission.get("mission_index").get_number());
				uint32_t target_object = static_cast<uint32_t>(mission.get("target_object").get_number());
				uint32_t target_count = static_cast<uint32_t>(mission.get("target_count").get_number());
				uint32_t mission_exp_reward = static_cast<uint32_t>(mission.get("mission_exp_reward").get_number());

				BattlePassMission mission_info(mission_type, target_object, target_count, mission_exp_reward,
											min_level, max_level);

				m_battle_pass_missions.insert(std::make_pair(mission_index, mission_info));
			}
			else
			{
				TraceError("Error: Missing required fields in one of the missions");
			}
		}
	}
	else
	{
		TraceError("Error: 'missions' key not found or not an array in JSON");
	}

	if (jf.contains("tiers") && jf.get("tiers").is_array())
	{
		const picojson::array& tiers = jf.get("tiers").get_array();
		for (size_t it = 0; it < tiers.size(); ++it)
		{
			const picojson::value& tier = tiers[it];
			if (!tier.is_object())
				continue;

			if (tier.contains("tier") &&
				tier.contains("needed_exp") &&
				tier.contains("reward_vnum") &&
				tier.contains("reward_count") &&
				tier.contains("reward_vnum_premium") &&
				tier.contains("reward_premium_count"))
			{
				uint8_t tier_index = static_cast<uint8_t>(tier.get("tier").get_number());
				uint32_t needed_exp = static_cast<uint32_t>(tier.get("needed_exp").get_number());
				uint32_t reward_vnum = static_cast<uint32_t>(tier.get("reward_vnum").get_number());
				uint32_t reward_count = static_cast<uint32_t>(tier.get("reward_count").get_number());
				uint32_t reward_vnum_premium = static_cast<uint32_t>(tier.get("reward_vnum_premium").get_number());
				uint32_t reward_premium_count = static_cast<uint32_t>(tier.get("reward_premium_count").get_number());

				TierInfo tier_info(needed_exp, reward_vnum, reward_count, reward_vnum_premium,
								reward_premium_count);

				m_battle_pass_tier_map.insert(std::make_pair(tier_index, tier_info));
			}
			else
			{
				TraceError("Error: Missing required fields in one of the tiers");
			}
		}
	}
	else
	{
		TraceError("Error: 'tiers' key not found or not an array in JSON");
	}
}

PyObject* moduleBattlePassGetRemainingTime(PyObject* poSelf, PyObject* poArgs)
{
	time_t now_time_t = time(NULL);

	const BattlePassInfo& battlePassInfo = BattlePassManager::Instance().GetBattlePassInfo();

	return Py_BuildValue("LLL", battlePassInfo.start_date, (long long)now_time_t, battlePassInfo.end_date);
}

PyObject* moduleBattlePasCompleteMissionWithItem(PyObject* poSelf, PyObject* poArgs)
{
	uint32_t mission_index;
	if (!PyTuple_GetInteger(poArgs, 0, (int*)&mission_index))
		return Py_BuildException();

	char command[256];
	snprintf(command, sizeof command, "/battlepass_command %u %u", net_battlepass::COMPLETE_MISSION_WITH_ITEM,
			mission_index);

	CPythonNetworkStream::Instance().SendChatPacket(command, CHAT_TYPE_COMMAND);

	return Py_BuildNone();
}

PyObject* moduleBattlePassGetTierSize(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", BattlePassManager::Instance().GetBattlePassTierMap().size());
}

PyObject* moduleBattlePassGetTierObject(PyObject* poSelf, PyObject* poArgs)
{
	uint32_t tier_index{};

	if (!PyTuple_GetUnsignedInteger(poArgs, 0, &tier_index))
		return Py_BadArgument();

	const auto& tier_map = BattlePassManager::Instance().GetBattlePassTierMap();
	const auto& tier_object = tier_map.find(tier_index);
	if (tier_object == tier_map.end())
		return nullptr;

	return Py_BuildValue("iiii", tier_object->second.reward_vnum, tier_object->second.reward_count,
						tier_object->second.reward_vnum_premium, tier_object->second.reward_premium_count);
}

PyObject* moduleBattlePassGetMissionObject(PyObject* poSelf, PyObject* poArgs)
{
	uint32_t mission_index{};

	if (!PyTuple_GetUnsignedInteger(poArgs, 0, &mission_index))
		return Py_BadArgument();

	const auto& mission_map = BattlePassManager::Instance().GetBattlePassMissionMap();
	const auto& mission_object = mission_map.find(mission_index);

	if (mission_object == mission_map.end())
		return nullptr;

	return Py_BuildValue("ikLkii", mission_object->second.mission_type, mission_object->second.target_object,
						mission_object->second.target_count, mission_object->second.exp_reward,
						mission_object->second.min_level, mission_object->second.max_level);
}

PyObject* moduleBattlePassGetMissionSize(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", BattlePassManager::Instance().GetBattlePassMissionMap().size());
}

PyObject* moduleBattlePassGetNextLevelExp(PyObject* poSelf, PyObject* poArgs)
{
	uint32_t tier_index{};

	if (!PyTuple_GetUnsignedInteger(poArgs, 0, &tier_index))
		return Py_BadArgument();

	const auto& tier_map = BattlePassManager::Instance().GetBattlePassTierMap();
	const auto& tier_object = tier_map.find(tier_index);
	if (tier_object == tier_map.end())
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", tier_object->second.needed_exp);
}

PyObject* moduleBattlePasRedeemMissionReward(PyObject* poSelf, PyObject* poArgs)
{
	uint32_t mission_index{};
	if (!PyTuple_GetUnsignedInteger(poArgs, 0, &mission_index))
		return Py_BadArgument();
	
	uint32_t is_premium_item{};
	if (!PyTuple_GetUnsignedInteger(poArgs, 1, &is_premium_item))
		return Py_BadArgument();

	char command[256 + 1];
	snprintf(command, sizeof command, "/battlepass_command %u %u %u", net_battlepass::REDEEM_MISSION_REWARD,
			mission_index, is_premium_item);

	CPythonNetworkStream::Instance().SendChatPacket(command, CHAT_TYPE_COMMAND);
	return Py_BuildNone();
}

PyObject* moduleBattlePasRedeemMissionExp(PyObject* poSelf, PyObject* poArgs)
{
	uint32_t mission_index{};
	if (!PyTuple_GetUnsignedInteger(poArgs, 0, &mission_index))
		return Py_BadArgument();
	
	char command[256 + 1];
	snprintf(command, sizeof command, "/battlepass_command %u %u", net_battlepass::REDEEM_MISSION_EXP,
			mission_index);

	CPythonNetworkStream::Instance().SendChatPacket(command, CHAT_TYPE_COMMAND);
	return Py_BuildNone();
}

PyObject* moduleBattlePasRequestCurrentProgress(PyObject* poSelf, PyObject* poArgs)
{
	char command[256 + 1];
	snprintf(command, sizeof command, "/battlepass_command %u", net_battlepass::REQUEST_CURRENT_PROGRESS_DATA);
	CPythonNetworkStream::Instance().SendChatPacket(command, CHAT_TYPE_COMMAND);
	// TraceError(command);
	return Py_BuildNone();
}

PyObject* moduleBattlePassRegisterClass(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* addr = NULL;

	if (!PyTuple_GetObject(poArgs, 0, &addr))
		return Py_BadArgument();

	battlepassMainClass = addr;
	return Py_BuildNone();
}

PyObject* moduleBattlePassUnregister(PyObject* poSelf, PyObject* poArgs)
{
	battlepassMainClass = NULL;
	return Py_BuildNone();
}

PyObject* moduleBattlePassGetPremiumStatus(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", BattlePassManager::Instance().GetPremiumStatus() ? 1 : 0);
}

void initBattlePass()
{
	static PyMethodDef s_methods[] = {
		{"GetRemainingTime", moduleBattlePassGetRemainingTime, METH_VARARGS},
		{"GetTierSize", moduleBattlePassGetTierSize, METH_VARARGS},
		{"GetTierObject", moduleBattlePassGetTierObject, METH_VARARGS},

		{"GetMissionSize", moduleBattlePassGetMissionSize, METH_VARARGS},
		{"GetMissionObject", moduleBattlePassGetMissionObject, METH_VARARGS},
		{"GetNextLevelEXP", moduleBattlePassGetNextLevelExp, METH_VARARGS},

		{"RedeemMissionReward", moduleBattlePasRedeemMissionReward, METH_VARARGS},
		{"RedeemMissionExp", moduleBattlePasRedeemMissionExp, METH_VARARGS},
		{"CompleteMissionWithItem", moduleBattlePasCompleteMissionWithItem, METH_VARARGS},
		{"RequestCurrentProgress", moduleBattlePasRequestCurrentProgress, METH_VARARGS},

		{"RegisterClass", moduleBattlePassRegisterClass, METH_VARARGS},
		{"UnregisterClass", moduleBattlePassUnregister, METH_VARARGS},
		{"GetPremiumStatus", moduleBattlePassGetPremiumStatus, METH_VARARGS},

		{nullptr, nullptr, NULL},
	};

	PyObject* module = Py_InitModule("battlepass", s_methods);

	PyModule_AddIntConstant(module, "MISSION_NONE", MISSION_NONE);
	PyModule_AddIntConstant(module, "MISSION_REACH_LEVEL", MISSION_REACH_LEVEL);
	PyModule_AddIntConstant(module, "MISSION_COLLECT_GOLD", MISSION_COLLECT_GOLD);
	PyModule_AddIntConstant(module, "MISSION_COLLECT_EXP", MISSION_COLLECT_EXP);
	PyModule_AddIntConstant(module, "MISSION_KILL_PLAYER", MISSION_KILL_PLAYER);
	PyModule_AddIntConstant(module, "MISSION_KILL_MOB", MISSION_KILL_MOB);
	PyModule_AddIntConstant(module, "MISSION_CATCH_FISH", MISSION_CATCH_FISH);
	PyModule_AddIntConstant(module, "MISSION_MINE", MISSION_MINE);
	PyModule_AddIntConstant(module, "MISSION_SEND_SHOUT_MESSAGE", MISSION_SEND_SHOUT_MESSAGE);
	PyModule_AddIntConstant(module, "MISSION_UPGRADE_OBJECT", MISSION_UPGRADE_OBJECT);
	PyModule_AddIntConstant(module, "MISSION_USE_ITEM", MISSION_USE_ITEM);
	PyModule_AddIntConstant(module, "MISSION_REACH_CONQUEROR_LEVEL", MISSION_REACH_CONQUEROR_LEVEL);
	PyModule_AddIntConstant(module, "MISSION_COLLECT_STONE_POINT", MISSION_COLLECT_STONE_POINT);
	PyModule_AddIntConstant(module, "MISSION_BATTLEPASS_MAX", MISSION_BATTLEPASS_MAX);
}

/*
*      *** Networking ***
* Do not touch unless you know what you are doing from now on.
*/

bool BattlePassManager::RecvBattlePassPacket()
{
	net_battlepass::packet_battlepass packet;
	if (!CPythonNetworkStream::Instance().Recv(sizeof(net_battlepass::packet_battlepass), &packet))
		return false;

	switch (packet.subheader)
	{
	case net_battlepass::SUBHEADER_SEND_CHARACTER_DATA:
	{
		net_battlepass::packet_battlepass_info info;
		if (!CPythonNetworkStream::Instance().Recv(sizeof(info), &info))
			return false;

		net_battlepass::BattlePassCharacterInfo chr_info;
		if (!CPythonNetworkStream::Instance().Recv(sizeof(chr_info), &chr_info))
			return false;

		UpdateCharacterData(chr_info);

		while (info.mission_count > 0)
		{
			net_battlepass::BattlePassCharacterMissionInfo mission;
			if (!CPythonNetworkStream::Instance().Recv(sizeof(mission), &mission))
				return false;

			UpdateMissionState(mission);
			--info.mission_count;
		}

		while (info.tier_count > 0)
		{
			net_battlepass::BattlePassCharacterTierInfo tier;
			if (!CPythonNetworkStream::Instance().Recv(sizeof(tier), &tier))
				return false;

			UpdateTierState(tier);
			--info.tier_count;
		}
		break;
	}

	case net_battlepass::SUBHEADER_SEND_CHARACTER_PROGRESS_UPDATE:
	{
		net_battlepass::BattlePassCharacterInfo info;
		if (!CPythonNetworkStream::Instance().Recv(sizeof(info), &info))
			return false;

		UpdateCharacterData(info);
		break;
	}

	case net_battlepass::SUBHEADER_SEND_MISSION_PROGRESS_UPDATE:
	{
		net_battlepass::BattlePassCharacterMissionInfo info;
		if (!CPythonNetworkStream::Instance().Recv(sizeof(info), &info))
			return false;

		UpdateMissionState(info);
		break;
	}

	case net_battlepass::SUBHEADER_SEND_REWARD_UPDATE:
	{
		net_battlepass::BattlePassCharacterTierInfo info;
		if (!CPythonNetworkStream::Instance().Recv(sizeof(info), &info))
			return false;

		UpdateTierState(info);
		break;
	}

	default:
		break;
	}
	return true;
}

void BattlePassManager::UpdateCharacterData(net_battlepass::BattlePassCharacterInfo data)
{
	// Cache premium status - battlepassMainClass olmasa bile Python'dan sorgulanabilir
	BattlePassManager::Instance().m_premium_status = (data.premium_status != 0);

	CallPyMethodBattlePass("UpdateCharacterData",
		Py_BuildValue("(iii)", data.current_level, data.current_exp, data.premium_status));
}

void BattlePassManager::UpdateMissionState(net_battlepass::BattlePassCharacterMissionInfo data)
{
	CallPyMethodBattlePass("UpdateMissionState",
		Py_BuildValue("(iiii)", data.mission_index, data.progress, data.completed, data.collected));
}

void BattlePassManager::UpdateTierState(net_battlepass::BattlePassCharacterTierInfo data)
{
	CallPyMethodBattlePass("UpdateRewardState",
		Py_BuildValue("(iii)", data.mission_index, data.reward_collected, data.reward_premium_collected));
}
#endif