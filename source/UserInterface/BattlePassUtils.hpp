#pragma once
#ifdef ENABLE_BATTLEPASS
#include <map>
#include <string>
#include <stdint.h>
#include <ctime>

enum EBattlePassMissions
{
	MISSION_NONE,
	MISSION_REACH_LEVEL,
	MISSION_COLLECT_GOLD,
	MISSION_COLLECT_EXP,
	MISSION_KILL_PLAYER,
	MISSION_KILL_MOB,
	MISSION_CATCH_FISH,
	MISSION_MINE,
	MISSION_SEND_SHOUT_MESSAGE,
	MISSION_UPGRADE_OBJECT,
	MISSION_USE_ITEM,
	MISSION_REACH_CONQUEROR_LEVEL,
	MISSION_COLLECT_STONE_POINT,
	MISSION_BATTLEPASS_MAX,
};

struct BattlePassInfo
{
	uint8_t id;
	long long start_date;
	long long end_date;

	long long GetRemainingTime() const
	{
		return (long long)std::difftime(end_date, start_date);
	}
};

#define ENUM_ENTRY(enum_val) { #enum_val, enum_val }

inline int32_t GetBattlePassMissionIndex(const std::string& mission_text)
{
	static std::map<std::string, uint8_t> mission_index_map;
	if (mission_index_map.empty())
	{
		mission_index_map.insert(ENUM_ENTRY(MISSION_NONE));
		mission_index_map.insert(ENUM_ENTRY(MISSION_REACH_LEVEL));
		mission_index_map.insert(ENUM_ENTRY(MISSION_COLLECT_GOLD));
		mission_index_map.insert(ENUM_ENTRY(MISSION_COLLECT_EXP));
		mission_index_map.insert(ENUM_ENTRY(MISSION_KILL_PLAYER));
		mission_index_map.insert(ENUM_ENTRY(MISSION_KILL_MOB));
		mission_index_map.insert(ENUM_ENTRY(MISSION_CATCH_FISH));
		mission_index_map.insert(ENUM_ENTRY(MISSION_MINE));
		mission_index_map.insert(ENUM_ENTRY(MISSION_SEND_SHOUT_MESSAGE));
		mission_index_map.insert(ENUM_ENTRY(MISSION_UPGRADE_OBJECT));
		mission_index_map.insert(ENUM_ENTRY(MISSION_USE_ITEM));
		mission_index_map.insert(ENUM_ENTRY(MISSION_REACH_CONQUEROR_LEVEL));
		mission_index_map.insert(ENUM_ENTRY(MISSION_COLLECT_STONE_POINT));
		mission_index_map.insert(ENUM_ENTRY(MISSION_BATTLEPASS_MAX));
	}

	std::map<std::string, uint8_t>::const_iterator it = mission_index_map.find(mission_text);
	if (it != mission_index_map.end())
		return it->second;

	return -1;
}

struct BattlePassMission
{
	int32_t mission_type;
	uint32_t target_object;
	uint64_t target_count;
	uint32_t exp_reward;
	uint16_t min_level;
	uint16_t max_level;

	BattlePassMission(int32_t type, uint32_t object, uint64_t count, uint32_t reward,
					uint16_t level_min, uint16_t level_max)
		: mission_type(type),
		target_object(object),
		target_count(count),
		exp_reward(reward),
		min_level(level_min),
		max_level(level_max)
	{
	}

	BattlePassMission() :
		mission_type(0),
		target_object(0),
		target_count(0),
		exp_reward(0),
		min_level(0),
		max_level(0)
	{
	}
};

struct TierInfo
{
	uint32_t needed_exp;
	uint32_t reward_vnum;
	uint32_t reward_count;
	uint32_t reward_vnum_premium;
	uint32_t reward_premium_count;

	TierInfo(uint32_t exp, uint32_t vnum, uint32_t count,
			uint32_t vnum_premium, uint32_t premium_count)
		: needed_exp(exp),
		reward_vnum(vnum),
		reward_count(count),
		reward_vnum_premium(vnum_premium),
		reward_premium_count(premium_count)
	{
	}

	TierInfo() :
		needed_exp(0),
		reward_vnum(0),
		reward_count(0),
		reward_vnum_premium(0),
		reward_premium_count(0)
	{
	}
};

namespace net_battlepass
{
#pragma pack(1)
struct packet_battlepass
{
	uint8_t header;
	uint16_t size;
	uint8_t subheader;
};

struct packet_battlepass_info
{
	uint32_t tier_count;
	uint32_t mission_count;
};

struct BattlePassCharacterInfo
{
	uint32_t current_level;
	uint32_t current_exp;
	bool premium_status;

	BattlePassCharacterInfo() :
		current_level(0),
		current_exp(0),
		premium_status(false)
	{
	}
};

struct BattlePassCharacterMissionInfo
{
	uint32_t mission_index;
	uint32_t progress;
	bool completed;
	bool collected;

	BattlePassCharacterMissionInfo() :
		mission_index(0),
		progress(0),
		completed(false),
		collected(false)
	{
	}
};

struct BattlePassCharacterTierInfo
{
	uint16_t mission_index;
	bool reward_collected;
	bool reward_premium_collected;

	BattlePassCharacterTierInfo() :
		mission_index(0),
		reward_collected(false),
		reward_premium_collected(false)
	{
	}
};
#pragma pack()

enum BattlePassPacket
{
	HEADER_GC_BATTLEPASS = 173
};

enum BattlePassSubPackets
{
	SUBHEADER_SEND_CHARACTER_DATA,
	SUBHEADER_SEND_CHARACTER_PROGRESS_UPDATE,
	SUBHEADER_SEND_MISSION_PROGRESS_UPDATE,
	SUBHEADER_SEND_REWARD_UPDATE,
};

enum BattlePassCommand
{
	REQUEST_CURRENT_PROGRESS_DATA = 1,
	REDEEM_MISSION_REWARD,
	REDEEM_MISSION_EXP,
	COMPLETE_MISSION_WITH_ITEM,
};

} // namespace net_battlepass
#endif // ENABLE_BATTLEPASS
