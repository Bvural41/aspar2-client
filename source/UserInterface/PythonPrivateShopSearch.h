#pragma once
#ifdef ENABLE_SHOP_SEARCH_SYSTEM
#include "Packet.h"

class CPythonPrivateShopSearch : public CSingleton<CPythonPrivateShopSearch>
{
public:
	CPythonPrivateShopSearch();
	virtual ~CPythonPrivateShopSearch();

	void AddItemData(TOfflineShopItemData& rItemData);
	void ClearItemData();
	DWORD GetItemDataCount() { return m_ItemInstanceVector.size(); }
	DWORD GetItemDataPtr(DWORD index, TOfflineShopItemData** ppInstance);

	void SetSortType(int sortType);
	int GetSortType() const { return m_sortType; }

protected:
	std::vector<TOfflineShopItemData> m_ItemInstanceVector;

	int m_sortType;
};
#endif