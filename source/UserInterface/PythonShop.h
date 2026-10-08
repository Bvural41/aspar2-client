#pragma once

#include "Packet.h"

/*
 *	상점 처리
 *
 *	2003-01-16 anoa	일차 완료
 *	2003-12-26 levites 수정
 *
 *	2012-10-29 rtsummit 새로운 화폐 출현 및 tab 기능 추가로 인한 shop 확장.
 *
 */
typedef enum
{
	SHOP_COIN_TYPE_GOLD, // DEFAULT VALUE
	SHOP_COIN_TYPE_SECONDARY_COIN,
#ifdef ENABLE_BATTLE_FIELD
	SHOP_COIN_TYPE_COMBAT_ZONE,
#endif
#ifdef ENABLE_CHEQUE_DESK_SYSTEM
	SHOP_COIN_TYPE_CHEQUE_DESK,
#endif
} EShopCoinType;

#ifdef ENABLE_TYPE_SHOPEX_SYSTEM
typedef enum
{
	SHOP_TYPE_GOLD,	// DEFULT VALUE
} EShopType;
#endif

class CPythonShop : public CSingleton<CPythonShop>
{
	public:
		CPythonShop(void);
		virtual ~CPythonShop(void);

		void Clear();
		
		void SetItemData(DWORD dwIndex, const TShopItemData & c_rShopItemData);
		BOOL GetItemData(DWORD dwIndex, const TShopItemData ** c_ppItemData);

		void SetItemData(BYTE tabIdx, DWORD dwSlotPos, const TShopItemData & c_rShopItemData);
		BOOL GetItemData(BYTE tabIdx, DWORD dwSlotPos, const TShopItemData ** c_ppItemData);
		void SetTabCount(BYTE bTabCount) { m_bTabCount = bTabCount; }
		BYTE GetTabCount() { return m_bTabCount; }

		void SetTabCoinType(BYTE tabIdx, BYTE coinType);
		BYTE GetTabCoinType(BYTE tabIdx);

		void SetTabName(BYTE tabIdx, const char* name);
		const char* GetTabName(BYTE tabIdx);

#ifdef ENABLE_TYPE_SHOPEX_SYSTEM
		void SetPriceType(short price_type);
		short GetPriceType();
#endif

		void Open(BOOL isPrivateShop, BOOL isMainPrivateShop);
#ifdef ENABLE_12ZI
		void SetLimitedItemShop(bool isLimitedItemShop);
		BOOL IsLimitedItemShop();
#endif
#ifdef ENABLE_STONE_POINT_SYSTEM
		void SetStonePointShop(bool isStonePointShop);
		BOOL IsStonePointShop();
#endif
#ifdef ENABLE_GUILD_DONATE_SYSTEM
		void SetMedalHonorShop(bool isMedalHonorShop);
		BOOL IsMedalHonorShop();
#endif
#ifdef ENABLE_CHEQUE_DESK_SYSTEM
		void SetChequeDeskShop(bool isChequeDeskNpc);
		BOOL IsChequeDeskNpc();
#endif
		void Close();
		BOOL IsOpen();
		BOOL IsPrivateShop();
		BOOL IsMainPlayerPrivateShop();

#ifdef ENABLE_OFFLINESHOP_SYSTEM
		void SetOfflineShopItemData(DWORD dwIndex, const TOfflineShopItemData& c_rShopItemData);
		BOOL GetOfflineShopItemData(DWORD dwIndex, const TOfflineShopItemData** c_ppItemData);

		void SetOfflineShopItemData(BYTE tabIdx, DWORD dwSlotPos, const TOfflineShopItemData& c_rShopItemData);
		BOOL GetOfflineShopItemData(BYTE tabIdx, DWORD dwSlotPos, const TOfflineShopItemData** c_ppItemData);

		void SetShopDisplayedCount(DWORD dwDisplayedCount) { m_dwDisplayedCount = dwDisplayedCount; }
		void SetShopSign(const char* g_sign) { strcpy(sign, g_sign); }
		const char* GetShopSign() { return sign; }
		DWORD	GetShopDisplayedCount() { return m_dwDisplayedCount; }
		BOOL IsOfflineShop() { return m_isOfflineShop; }
		void ClearOfflineShopStock() { m_OfflineShopItemStock.clear(); }
		void AddOfflineShopItemStock(TItemPos ItemPos, BYTE byDisplayPos, long long dwPrice, int dwPrice_Cheque);
		void DelOfflineShopItemStock(TItemPos ItemPos);
		long long	 GetOfflineShopItemPrice(TItemPos ItemPos);
		DWORD GetOfflineShopItemPriceCheque(TItemPos ItemPos);
		BYTE	GetOfflineShopItemStatus(TItemPos ItemPos);
		void BuildOfflineShop(const char* c_szName, DWORD shopVnum, BYTE shopTitle);

		void	SetCurrentOfflineShopMoney(long long llMoney) { m_llCurrentOfflineShopMoney = llMoney; }
		long long	GetCurrentOfflineShopMoney() { return m_llCurrentOfflineShopMoney; }

#ifdef ENABLE_CHEQUE_SYSTEM
		void	SetCurrentOfflineShopMoneyCheque(DWORD dwCheque) { m_llCurrentOfflineShopMoneyCheque = dwCheque; }
		DWORD	GetCurrentOfflineShopMoneyCheque() { return m_llCurrentOfflineShopMoneyCheque; }
#endif

		bool	HasOfflineShop() { return m_bHasOfflineShop; }
		void	SetHasOfflineShop(bool bStatus) { m_bHasOfflineShop = bStatus; }

		DWORD	GetRealWatcherCount() { return m_dwRealWatcherCount; }
		void	SetRealWatcherCount(DWORD p) { m_dwRealWatcherCount = p; }

		unsigned long long	GetShopFlag() { return m_shopFlag; }
		void	SetShopFlag(unsigned long long p) { m_shopFlag = p; }

		int		GetShopTime() { return time; }
		void	SetShopTime(int a) { time = a; }

		DWORD	GetShopType() { return type; }
		void	SetShopType(DWORD a) { type = a; }
#endif

		void ClearPrivateShopStock();
#ifdef ENABLE_CHEQUE_SYSTEM
		void AddPrivateShopItemStock(TItemPos ItemPos, BYTE byDisplayPos, DWORD dwPrice, DWORD dwPriceCheque);
#else
		void AddPrivateShopItemStock(TItemPos ItemPos, BYTE byDisplayPos, DWORD dwPrice);
#endif
		void DelPrivateShopItemStock(TItemPos ItemPos);
		int GetPrivateShopItemPrice(TItemPos ItemPos);
#ifdef ENABLE_CHEQUE_SYSTEM
		int GetPrivateShopItemPriceCheque(TItemPos ItemPos);
#endif
		void BuildPrivateShop(const char * c_szName);

	protected:
		BOOL	CheckSlotIndex(DWORD dwIndex);

	protected:
#ifdef ENABLE_OFFLINESHOP_SYSTEM
		struct OfflineShopTab
		{
			TOfflineShopItemData items[OFFLINE_SHOP_HOST_ITEM_MAX_NUM];
		};

		DWORD				m_dwDisplayedCount;
		DWORD				m_dwRealWatcherCount;
		OfflineShopTab		m_aOfflineShoptabs[SHOP_TAB_COUNT_MAX];
		std::map<TItemPos, TOfflineShopItemTable>	m_OfflineShopItemStock;
		BOOL				m_isOfflineShop;
		char				sign[SHOP_SIGN_MAX_LEN + 1];
		bool				m_bHasOfflineShop;
		long long			m_llCurrentOfflineShopMoney;
		DWORD				m_llCurrentOfflineShopMoneyCheque;
		unsigned long long			m_shopFlag;
		int					time;
		DWORD				type;
#endif
		BOOL				m_isShoping;
		BOOL				m_isPrivateShop;
		BOOL				m_isMainPlayerPrivateShop;
#ifdef ENABLE_12ZI
		BOOL				m_isLimitedItemShop;
#endif
#ifdef ENABLE_STONE_POINT_SYSTEM
		BOOL				m_isStonePointShop;
#endif
#ifdef ENABLE_GUILD_DONATE_SYSTEM
		BOOL				m_isMedalHonorShop;
#endif
#ifdef ENABLE_CHEQUE_DESK_SYSTEM
		BOOL				m_isChequeDeskNpc;
#endif

		struct ShopTab
		{
			ShopTab()
			{
				coinType = SHOP_COIN_TYPE_GOLD;
			}
			BYTE				coinType;
			std::string			name;
			TShopItemData		items[SHOP_HOST_ITEM_MAX_NUM];
		};

#ifdef ENABLE_TYPE_SHOPEX_SYSTEM
		short				sPrice_type;
#endif

		BYTE m_bTabCount;
		ShopTab m_aShoptabs[SHOP_TAB_COUNT_MAX];

		typedef std::map<TItemPos, TShopItemTable> TPrivateShopItemStock;
		TPrivateShopItemStock	m_PrivateShopItemStock;
};
