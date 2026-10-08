#pragma once

enum
{
	EVENT_FOOTBALL				= 0,
	EVENT_AYISIGI				= 1,
	EVENT_OKEYCARD				= 2,
	EVENT_FISH					= 3,
	EVENT_CATCHKING				= 4,
	EVENT_WORD					= 5,
	EVENT_RITUELSOUL			= 6,
	EVENT_CHEQUEDESK			= 7,
	EVENT_2X_STONE_POINT		= 8,
	EVENT_FATE_ROULETTE			= 9,
	EVENT_BATTLE_ROYALE			= 10,
	EVENT_FLOWER				= 11,
	EVENT_TREASURE				= 12,
	EVENT_MAX_NUM				= 13,
};

class CPythonGameEvents : public CSingleton<CPythonGameEvents>
{
public:
	CPythonGameEvents();
	virtual ~CPythonGameEvents();

	void	SetActivateEvent(bool isActivate, BYTE bEventID);
	void	SetEventTime(BYTE bEventID, DWORD event_time);
	bool	IsActivateEvent(BYTE bEventID);
	DWORD	GetEventTime(BYTE bEventID);
protected:
	bool	m_pkActivateEvents[EVENT_MAX_NUM];
	DWORD	m_dwEventEndTime[EVENT_MAX_NUM];
private:
};