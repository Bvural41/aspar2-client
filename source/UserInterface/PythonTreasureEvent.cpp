#include "StdAfx.h"

#ifdef ENABLE_TREASURE_EVENT
#include "PythonTreasureEvent.h"

CPythonTreasureEvent::CPythonTreasureEvent()
{
	Clear();
}

void CPythonTreasureEvent::Clear()
{
	m_vecRank.clear();
	m_vecEventRewards.clear();
	m_mapAccumulatedRewards.clear();
	m_iAccumulatedIndex = 0;
	m_iRewardFlag = 0;
	m_iDoubloon = 0;
}


const TTreasureRank* CPythonTreasureEvent::GetRankInfo(BYTE bIndex)
{
	for (const auto& rank : m_vecRank)
	{
		if (rank.bIndex == bIndex)
			return &rank;
	}
	return NULL;
}

bool CPythonTreasureEvent::IsOpened()
{
	return IS_SET(m_iRewardFlag, 1 << 30) ? true : false;
}

bool CPythonTreasureEvent::HasReward()
{
	return IS_SET(m_iRewardFlag, 1 << 31) ? true : false;
}

BYTE CPythonTreasureEvent::GetTotalRecvItemCount()
{
	BYTE bReceiveCount = 0;
	for (BYTE j = 0; j < TREASURE_EVENT_EVENT_REWARDS_MAX_COUNT; ++j)
	{
		if (IsSlotReceived(j))
			bReceiveCount += 1;
	}
	return bReceiveCount;
}

bool CPythonTreasureEvent::IsSlotReceived(BYTE bIndex)
{
	if (bIndex >= TREASURE_EVENT_EVENT_REWARDS_MAX_COUNT)
		return false;
	return IS_SET(m_iRewardFlag, 1 << bIndex) ? true : false;
}

BYTE CPythonTreasureEvent::GetRequiredKeyCount(int iAccumulateIndex, BYTE bIndex)
{
	if (bIndex == 0)
		return 0;
	return 1;
}

bool CPythonTreasureEvent::GetSlotAccumulatedIndex(BYTE bIndex, BYTE& bMinAccumulatedIndex, BYTE& bMaxAccumulatedIndex)
{
	static const BYTE bSlotsData[TREASURE_EVENT_EVENT_REWARDS_MAX_COUNT][2] = {
		{0, 2}, {0, 2}, {0, 2}, {3, 4}, {10, 10},
		{0, 2}, {0, 2}, {0, 2}, {3, 4}, {10, 10},
		{0, 2}, {0, 2}, {0, 2}, {3, 4}, {10, 10},
		{3, 4}, {3, 4}, {3, 4}, {3, 4}, {10, 10},
		{5, 9}, {5, 9}, {5, 9}, {5, 9}, {5, 9}
	};
	if (bIndex >= TREASURE_EVENT_EVENT_REWARDS_MAX_COUNT)
		return false;
	bMinAccumulatedIndex = bSlotsData[bIndex][0];
	bMaxAccumulatedIndex = bSlotsData[bIndex][1];
	return true;
}

const TTreasureSimpleReward* CPythonTreasureEvent::GetEventReward(BYTE bVectorIndex)
{
	if (m_vecEventRewards.empty() || bVectorIndex >= m_vecEventRewards.size())
		return NULL;
	return &m_vecEventRewards[bVectorIndex];
}

const TTreasureRandomReward* CPythonTreasureEvent::GetRandomReward(BYTE bIndex, BYTE bVectorIndex)
{
	const auto it = m_mapAccumulatedRewards.find(bIndex);
	if (it == m_mapAccumulatedRewards.end())
		return NULL;
	else if (it->second.random_rewards.empty() || bVectorIndex >= it->second.random_rewards.size())
		return NULL;
	return &it->second.random_rewards[bVectorIndex];
}

TTreasureAffectReward* CPythonTreasureEvent::GetAffectReward(BYTE bIndex, BYTE bVectorIndex)
{
	const auto it = m_mapAccumulatedRewards.find(bIndex);
	if (it == m_mapAccumulatedRewards.end())
		return NULL;
	else if (it->second.affect_rewards.empty() || bVectorIndex >= it->second.affect_rewards.size())
		return NULL;
	return &it->second.affect_rewards[bVectorIndex];
}

PyObject* treasure_eventClear(PyObject* poSelf, PyObject* poArgs)
{
	CPythonTreasureEvent::Instance().Clear();
	return Py_BuildNone();
}

PyObject* treasure_eventGetEventReward(PyObject* poSelf, PyObject* poArgs)
{
	int	iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	const TTreasureSimpleReward* pReward = CPythonTreasureEvent::Instance().GetEventReward(static_cast<BYTE>(iIndex));
	if (pReward)
		return Py_BuildValue("ii", pReward->vnum, pReward->count);
	return Py_BuildValue("ii", 0, 0);
}

PyObject* treasure_eventGetRandomReward(PyObject* poSelf, PyObject* poArgs)
{
	int	iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();
	int	iVectorIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iVectorIndex))
		return Py_BuildException();

	const TTreasureRandomReward* pRandomReward = CPythonTreasureEvent::Instance().GetRandomReward(static_cast<BYTE>(iIndex), static_cast<BYTE>(iVectorIndex));
	if (pRandomReward)
		return Py_BuildValue("iii", pRandomReward->vnum, pRandomReward->count, pRandomReward->prob);
	return Py_BuildValue("iii", 0, 0, 0);
}

PyObject* treasure_eventGetAffectReward(PyObject* poSelf, PyObject* poArgs)
{
	int	iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();
	int	iVectorIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iVectorIndex))
		return Py_BuildException();

	const TTreasureAffectReward* pAffectReward = CPythonTreasureEvent::Instance().GetAffectReward(static_cast<BYTE>(iIndex), static_cast<BYTE>(iVectorIndex));
	if (pAffectReward)
		return Py_BuildValue("iiiiiii", pAffectReward->first_come, pAffectReward->global_used, pAffectReward->affect_type, pAffectReward->point_type, pAffectReward->apply_point, pAffectReward->flag, pAffectReward->duration);
	return Py_BuildValue("iiiiiii", 0, 0, 0, 0, 0, 0, 0);
}

PyObject* treasure_eventGetSlotAccumulatedInfo(PyObject* poSelf, PyObject* poArgs)
{
	int	iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();
	BYTE bMinAccumulatedIndex = 0, bMaxAccumulatedIndex = 0;
	CPythonTreasureEvent::Instance().GetSlotAccumulatedIndex(static_cast<BYTE>(iIndex), bMinAccumulatedIndex, bMaxAccumulatedIndex);
	return Py_BuildValue("ii", bMinAccumulatedIndex, bMaxAccumulatedIndex);
}

PyObject* treasure_eventGetDoubloon(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonTreasureEvent::Instance().GetDoubloon());
}

PyObject* treasure_eventGetAccumulatedIndex(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonTreasureEvent::Instance().GetAccumulatedIndex());
}

PyObject* treasure_eventGetRequiredKeyCount(PyObject* poSelf, PyObject* poArgs)
{
	int	iAccumulateIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iAccumulateIndex))
		return Py_BuildException();

	int	iIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iIndex))
		return Py_BuildException();
	return Py_BuildValue("i", CPythonTreasureEvent::Instance().GetRequiredKeyCount(iAccumulateIndex, static_cast<BYTE>(iIndex)));
}

PyObject* treasure_eventIsSlotReceived(PyObject* poSelf, PyObject* poArgs)
{
	int	iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();
	return Py_BuildValue("i", CPythonTreasureEvent::Instance().IsSlotReceived(static_cast<BYTE>(iIndex)));
}

PyObject* treasure_eventHasReward(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonTreasureEvent::Instance().HasReward());
}

PyObject* treasure_eventIsOpened(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonTreasureEvent::Instance().IsOpened());
}

PyObject* treasure_eventGetTotalRecvItemCount(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonTreasureEvent::Instance().GetTotalRecvItemCount());
}

PyObject* treasure_eventGetImage(PyObject* poSelf, PyObject* poArgs)
{
	char* iconFile;
	if (!PyTuple_GetString(poArgs, 0, &iconFile))
		return Py_BadArgument();
	CGraphicImage* pImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(iconFile);
	if (!pImage)
		return Py_BuildValue("i", 0);
	return PyLong_FromVoidPtr(pImage);
}

PyObject* treasure_eventGetRankInfo(PyObject* poSelf, PyObject* poArgs)
{
	int	iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	const TTreasureRank* pRankInfo = CPythonTreasureEvent::Instance().GetRankInfo(static_cast<BYTE>(iIndex));
	if (pRankInfo)
		return Py_BuildValue("isii", pRankInfo->dwPID, pRankInfo->szName, pRankInfo->iAccumulateIndex, pRankInfo->bReward);
	return Py_BuildValue("isii", 0, "", 0, 0);
}

void inittreasure_event()
{
	static PyMethodDef s_methods[] =
	{
		{ "Clear", treasure_eventClear, METH_VARARGS },
		{ "GetEventReward", treasure_eventGetEventReward, METH_VARARGS },
		{ "GetRandomReward", treasure_eventGetRandomReward, METH_VARARGS },
		{ "GetAffectReward", treasure_eventGetAffectReward, METH_VARARGS },
		{ "GetSlotAccumulatedInfo", treasure_eventGetSlotAccumulatedInfo, METH_VARARGS },
		{ "GetDoubloon", treasure_eventGetDoubloon, METH_VARARGS },
		{ "GetAccumulatedIndex", treasure_eventGetAccumulatedIndex, METH_VARARGS },
		{ "GetRequiredKeyCount", treasure_eventGetRequiredKeyCount, METH_VARARGS },
		{ "IsSlotReceived", treasure_eventIsSlotReceived, METH_VARARGS },
		{ "HasReward", treasure_eventHasReward, METH_VARARGS },
		{ "IsOpened", treasure_eventIsOpened, METH_VARARGS },
		{ "GetTotalRecvItemCount", treasure_eventGetTotalRecvItemCount, METH_VARARGS },
		{ "GetImage", treasure_eventGetImage, METH_VARARGS },
		{ "GetRankInfo", treasure_eventGetRankInfo, METH_VARARGS },
		{ NULL, NULL },
	};
	PyObject* poModule = Py_InitModule("treasure_event", s_methods);

	PyModule_AddIntConstant(poModule, "EVENT_REWARDS_MAX_COUNT", TREASURE_EVENT_EVENT_REWARDS_MAX_COUNT);
	PyModule_AddIntConstant(poModule, "ACCUMULATED_REWARDS_MAX_COUNT", TREASURE_EVENT_ACCUMULATED_REWARDS_MAX_COUNT);
	PyModule_AddIntConstant(poModule, "ACCUMULATED_MIN_INDEX", TREASURE_EVENT_ACCUMULATED_MIN_INDEX);
	PyModule_AddIntConstant(poModule, "ACCUMULATED_MAX_INDEX", TREASURE_EVENT_ACCUMULATED_MAX_INDEX);
	PyModule_AddIntConstant(poModule, "REQUIRED_DOUBLOON_COUNT", TREASURE_EVENT_REQUIRED_DOUBLOON_COUNT);
	PyModule_AddIntConstant(poModule, "MAX_DOUBLOON_COUNT", TREASURE_EVENT_MAX_DOUBLOON_COUNT);
	PyModule_AddIntConstant(poModule, "REQUIRED_MIN_LEVEL", TREASURE_EVENT_REQUIRED_MIN_LEVEL);
	PyModule_AddIntConstant(poModule, "RECV_REWARD_MAX", TREASURE_EVENT_RECV_REWARD_MAX);
}
#endif
