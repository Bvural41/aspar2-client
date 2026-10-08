#pragma once

#ifdef ENABLE_TREASURE_EVENT
enum ETreasureEvent
{
	TREASURE_EVENT_EVENT_REWARDS_MAX_COUNT = 25,
	TREASURE_EVENT_ACCUMULATED_REWARDS_MAX_COUNT = 15,

	TREASURE_EVENT_ACCUMULATED_MIN_INDEX = 0,
	TREASURE_EVENT_ACCUMULATED_MAX_INDEX = 10,

	TREASURE_EVENT_REQUIRED_DOUBLOON_COUNT = 90,
	TREASURE_EVENT_MAX_DOUBLOON_COUNT = 1000,
	TREASURE_EVENT_REQUIRED_MIN_LEVEL = 70,
	TREASURE_EVENT_RECV_REWARD_MAX = 9,
};

#pragma pack(1)
typedef struct STreasureSimpleReward
{
	DWORD vnum;
	DWORD count;
}TTreasureSimpleReward;
typedef struct STreasureRandomReward
{
	DWORD vnum;
	DWORD count;
	BYTE prob;
}TTreasureRandomReward;
typedef struct STreasureAffectReward
{
	BYTE first_come;
	int global_used;
	int affect_type;
	BYTE point_type;
	long apply_point;
	int flag;
	int duration;
}TTreasureAffectReward;
typedef struct STreasureRank
{
	BYTE	bIndex;
	DWORD	dwPID;
	char	szName[CHARACTER_NAME_MAX_LEN + 1];
	int		iAccumulateIndex;
	bool	bReward;
}TTreasureRank;
#pragma pack()
struct STreasureAccumulatedReward
{
	std::vector<TTreasureRandomReward> random_rewards;
	std::vector<TTreasureAffectReward> affect_rewards;
};

class CPythonTreasureEvent : public CSingleton<CPythonTreasureEvent>
{
public:
	CPythonTreasureEvent();
	void Clear();

	bool GetSlotAccumulatedIndex(BYTE bIndex, BYTE& bMinAccumulatedIndex, BYTE& bMaxAccumulatedIndex);
	
	BYTE GetRequiredKeyCount(int iAccumulateIndex, BYTE bIndex);

	int	GetDoubloon() { return m_iDoubloon; }
	void SetDoubloon(int iDoubloon) { m_iDoubloon = iDoubloon; }

	int GetAccumulatedIndex() { return m_iAccumulatedIndex; }
	void SetAccumulatedIndex(int iIndex) { m_iAccumulatedIndex = iIndex; }

	BYTE GetTotalRecvItemCount();
	bool IsOpened();
	bool HasReward();
	bool IsSlotReceived(BYTE bIndex);
	int GetRewardFlag() { return m_iRewardFlag; }
	void SetRewardFlag(int iFlag) { m_iRewardFlag = iFlag; }

	const TTreasureRank* GetRankInfo(BYTE bIndex);

	const TTreasureSimpleReward* GetEventReward(BYTE bVectorIndex);
	const TTreasureRandomReward* GetRandomReward(BYTE bIndex, BYTE bVectorIndex);
	TTreasureAffectReward* GetAffectReward(BYTE bIndex, BYTE bVectorIndex);

	std::vector<TTreasureSimpleReward>& GetEventRewardsVector() { return m_vecEventRewards; }
	std::map<BYTE, STreasureAccumulatedReward>& GetAccumulatedRewardsMap() { return m_mapAccumulatedRewards; }

	std::vector<TTreasureRank>& GetRankVector() { return m_vecRank; }
protected:
	int m_iAccumulatedIndex;
	int m_iRewardFlag;
	int	m_iDoubloon;
	std::vector<TTreasureRank>	m_vecRank;
	std::vector<TTreasureSimpleReward> m_vecEventRewards;
	std::map<BYTE, STreasureAccumulatedReward> m_mapAccumulatedRewards;
};
#endif
