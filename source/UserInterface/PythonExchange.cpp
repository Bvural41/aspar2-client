#include "StdAfx.h"
#include "PythonExchange.h"

void CPythonExchange::SetSelfName(const char *name)
{
	strncpy(m_self.name, name, CHARACTER_NAME_MAX_LEN);
}

void CPythonExchange::SetTargetName(const char *name)
{
	strncpy(m_victim.name, name, CHARACTER_NAME_MAX_LEN);
}

char * CPythonExchange::GetNameFromSelf()
{
	return m_self.name;
}

char * CPythonExchange::GetNameFromTarget()
{
	return m_victim.name;
}

void CPythonExchange::SetElkToTarget(long long elk)
{	
	m_victim.elk = elk;
}

void CPythonExchange::SetElkToSelf(long long elk)
{
	m_self.elk = elk;
}

long long CPythonExchange::GetElkFromTarget()
{
	return m_victim.elk;
}

long long CPythonExchange::GetElkFromSelf()
{
	return m_self.elk;
}

#ifdef ENABLE_CHEQUE_SYSTEM
void CPythonExchange::SetWonToTarget(DWORD	won)
{	
	m_victim.won = won;
}

void CPythonExchange::SetWonToSelf(DWORD won)
{
	m_self.won = won;
}

DWORD CPythonExchange::GetWonFromTarget()
{
	return m_victim.won;
}

DWORD CPythonExchange::GetWonFromSelf()
{
	return m_self.won;
}
#endif

#ifdef ENABLE_SET_ITEM
void CPythonExchange::SetItemSetValue(int iPos, uint8_t bSetItem, bool bSelf)
{
	if (iPos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	if (bSelf)
		m_self.set_value[iPos] = bSetItem;
	else
		m_victim.set_value[iPos] = bSetItem;
}

uint32_t CPythonExchange::GetItemSetValueFromTarget(uint8_t pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.set_value[pos];
}

uint32_t CPythonExchange::GetItemSetValueFromSelf(uint8_t pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.set_value[pos];
}
#endif

#ifdef ENABLE_EXTENDED_ITEM_COUNT
void CPythonExchange::SetItemToTarget(DWORD pos, DWORD vnum, WORD count)
#else
void CPythonExchange::SetItemToTarget(DWORD pos, DWORD vnum, BYTE count)
#endif
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_vnum[pos] = vnum;
	m_victim.item_count[pos] = count;
}

#ifdef ENABLE_EXTENDED_ITEM_COUNT
void CPythonExchange::SetItemToSelf(DWORD pos, DWORD vnum, WORD count)
#else
void CPythonExchange::SetItemToSelf(DWORD pos, DWORD vnum, BYTE count)
#endif
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_vnum[pos] = vnum;
	m_self.item_count[pos] = count;
}

void CPythonExchange::SetItemMetinSocketToTarget(int pos, int imetinpos, DWORD vnum)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_metin[pos][imetinpos] = vnum;
}

void CPythonExchange::SetItemMetinSocketToSelf(int pos, int imetinpos, DWORD vnum)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_metin[pos][imetinpos] = vnum;
}

void CPythonExchange::SetItemAttributeToTarget(int pos, int iattrpos, BYTE byType, short sValue)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_attr[pos][iattrpos].bType = byType;
	m_victim.item_attr[pos][iattrpos].sValue = sValue;
}

void CPythonExchange::SetItemAttributeToSelf(int pos, int iattrpos, BYTE byType, short sValue)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_attr[pos][iattrpos].bType = byType;
	m_self.item_attr[pos][iattrpos].sValue = sValue;
}

void CPythonExchange::DelItemOfTarget(BYTE pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_vnum[pos] = 0;
	m_victim.item_count[pos] = 0;
}

void CPythonExchange::DelItemOfSelf(BYTE pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_vnum[pos] = 0;
	m_self.item_count[pos] = 0;
}

DWORD CPythonExchange::GetItemVnumFromTarget(BYTE pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.item_vnum[pos];
}

DWORD CPythonExchange::GetItemVnumFromSelf(BYTE pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.item_vnum[pos];
}

#ifdef ENABLE_EXTENDED_ITEM_COUNT
WORD CPythonExchange::GetItemCountFromTarget(BYTE pos)
#else
BYTE CPythonExchange::GetItemCountFromTarget(BYTE pos)
#endif
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.item_count[pos];
}

#ifdef ENABLE_EXTENDED_ITEM_COUNT
WORD CPythonExchange::GetItemCountFromSelf(BYTE pos)
#else
BYTE CPythonExchange::GetItemCountFromSelf(BYTE pos)
#endif
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.item_count[pos];
}

DWORD CPythonExchange::GetItemMetinSocketFromTarget(BYTE pos, int iMetinSocketPos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.item_metin[pos][iMetinSocketPos];
}

DWORD CPythonExchange::GetItemMetinSocketFromSelf(BYTE pos, int iMetinSocketPos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.item_metin[pos][iMetinSocketPos];
}

void CPythonExchange::GetItemAttributeFromTarget(BYTE pos, int iAttrPos, BYTE * pbyType, short * psValue)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	*pbyType = m_victim.item_attr[pos][iAttrPos].bType;
	*psValue = m_victim.item_attr[pos][iAttrPos].sValue;
}

void CPythonExchange::GetItemAttributeFromSelf(BYTE pos, int iAttrPos, BYTE * pbyType, short * psValue)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	*pbyType = m_self.item_attr[pos][iAttrPos].bType;
	*psValue = m_self.item_attr[pos][iAttrPos].sValue;
}

#ifdef ENABLE_REFINE_ELEMENT
void CPythonExchange::SetItemRefineElementToSelf(BYTE bPos, DWORD dwRefineElement)
{
	if (bPos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.dwRefineElement[bPos] = dwRefineElement;
}

void CPythonExchange::SetItemRefineElementToTarget(BYTE bPos, DWORD dwRefineElement)
{
	if (bPos >= EXCHANGE_ITEM_MAX_NUM)
		return;
	
	m_victim.dwRefineElement[bPos] = dwRefineElement;
}

DWORD CPythonExchange::GetItemRefineElementFromSelf(BYTE bPos)
{
	if (bPos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.dwRefineElement[bPos];
}

DWORD CPythonExchange::GetItemRefineElementFromTarget(BYTE bPos)
{
	if (bPos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.dwRefineElement[bPos];
}
#endif

#ifdef ENABLE_CHANGELOOK_SYSTEM
void CPythonExchange::SetItemTransmutation(int iPos, DWORD dwTransmutation, bool bSelf)
{
	if (iPos >= EXCHANGE_ITEM_MAX_NUM)
		return;
	
	if (bSelf)
		m_self.transmutation[iPos] = dwTransmutation;
	else
		m_victim.transmutation[iPos] = dwTransmutation;
}

DWORD CPythonExchange::GetItemTransmutation(int iPos, bool bSelf)
{
	if (iPos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;
	
	DWORD dwTransmutation = bSelf == true ? m_self.transmutation[iPos] : m_victim.transmutation[iPos];
	return dwTransmutation;
}
#endif

#ifdef ENABLE_GLOVE_SYSTEM
void CPythonExchange::SetItemApplyRandomToTarget(int pos, int iApplyPos, BYTE byType, short sValue)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_apply_random[pos][iApplyPos].bType = byType;
	m_victim.item_apply_random[pos][iApplyPos].sValue = sValue;
}

void CPythonExchange::SetItemApplyRandomToSelf(int pos, int iApplyPos, BYTE byType, short sValue)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_apply_random[pos][iApplyPos].bType = byType;
	m_self.item_apply_random[pos][iApplyPos].sValue = sValue;
}

void CPythonExchange::GetItemApplyRandomFromTarget(BYTE pos, int iApplyPos, BYTE* pbyType, short* psValue)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	*pbyType = m_victim.item_apply_random[pos][iApplyPos].bType;
	*psValue = m_victim.item_apply_random[pos][iApplyPos].sValue;
}

void CPythonExchange::GetItemApplyRandomFromSelf(BYTE pos, int iApplyPos, BYTE* pbyType, short* psValue)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	*pbyType = m_self.item_apply_random[pos][iApplyPos].bType;
	*psValue = m_self.item_apply_random[pos][iApplyPos].sValue;
}
#endif

void CPythonExchange::SetAcceptToTarget(BYTE Accept)
{
	m_victim.accept = Accept ? true : false;
}

void CPythonExchange::SetAcceptToSelf(BYTE Accept)
{
	m_self.accept = Accept ? true : false;
}

bool CPythonExchange::GetAcceptFromTarget()
{
	return m_victim.accept ? true : false;
}

bool CPythonExchange::GetAcceptFromSelf()
{
	return m_self.accept ? true : false;
}

bool CPythonExchange::GetElkMode()
{
	return m_elk_mode;
}

void CPythonExchange::SetElkMode(bool value)
{
	m_elk_mode = value;
}

void CPythonExchange::Start()
{
	m_isTrading = true;
}

void CPythonExchange::End()
{
	m_isTrading = false;
}

bool CPythonExchange::isTrading()
{
	return m_isTrading;
}

void CPythonExchange::Clear()
{
	memset(&m_self, 0, sizeof(m_self));
	memset(&m_victim, 0, sizeof(m_victim));
/*
	m_self.item_vnum[0] = 30;
	m_victim.item_vnum[0] = 30;
	m_victim.item_vnum[1] = 40;
	m_victim.item_vnum[2] = 50;
*/
}

CPythonExchange::CPythonExchange()
{
	Clear();
	m_isTrading = false;
	m_elk_mode = false;
		// Clear로 옴겨놓으면 안됨. 
		// trade_start 페킷이 오면 Clear를 실행하는데
		// m_elk_mode는 클리어 되선 안됨.;  
}
CPythonExchange::~CPythonExchange()
{
}
