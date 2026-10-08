#pragma once

#include "Packet.h"

/*
 *	교환 창 관련
 */
class CPythonExchange : public CSingleton<CPythonExchange>
{
	public:
		enum
		{
#ifdef ENABLE_NEW_EXCHANGE_WINDOW
			EXCHANGE_ITEM_MAX_NUM = 24,
#else
			EXCHANGE_ITEM_MAX_NUM = 12,
#endif
		};

		typedef struct trade
		{
			char					name[CHARACTER_NAME_MAX_LEN + 1];

			DWORD					item_vnum[EXCHANGE_ITEM_MAX_NUM];
#ifdef ENABLE_EXTENDED_ITEM_COUNT
			WORD					item_count[EXCHANGE_ITEM_MAX_NUM];
#else
			BYTE					item_count[EXCHANGE_ITEM_MAX_NUM];
#endif
			DWORD					item_metin[EXCHANGE_ITEM_MAX_NUM][ITEM_SOCKET_SLOT_MAX_NUM];
#ifdef ENABLE_REFINE_ELEMENT
			DWORD					dwRefineElement[EXCHANGE_ITEM_MAX_NUM];
#endif
			TPlayerItemAttribute	item_attr[EXCHANGE_ITEM_MAX_NUM][ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#ifdef ENABLE_GLOVE_SYSTEM
			TPlayerItemAttribute	item_apply_random[EXCHANGE_ITEM_MAX_NUM][ITEM_APPLY_RANDOM_SLOT_MAX_NUM];
#endif
			BYTE					accept;
			long long				elk;
#ifdef ENABLE_CHEQUE_SYSTEM
			DWORD					won;
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
			DWORD					transmutation[EXCHANGE_ITEM_MAX_NUM];
#endif
#ifdef ENABLE_SET_ITEM
			uint8_t					set_value[EXCHANGE_ITEM_MAX_NUM];
#endif
		} TExchangeData;

	public:
		CPythonExchange();
		virtual ~CPythonExchange();

		void			Clear();

		void			Start();
		void			End();
		bool			isTrading();

		// Interface

		void			SetSelfName(const char *name);
		void			SetTargetName(const char *name);

		char			*GetNameFromSelf();
		char			*GetNameFromTarget();

		void			SetElkToTarget(long long elk);
		void			SetElkToSelf(long long elk);

		long long		GetElkFromTarget();
		long long		GetElkFromSelf();
#ifdef ENABLE_CHEQUE_SYSTEM
		void			SetWonToTarget(DWORD won);
		void			SetWonToSelf(DWORD won);

		DWORD			GetWonFromTarget();
		DWORD			GetWonFromSelf();
#endif

#ifdef ENABLE_SET_ITEM
		void			SetItemSetValue(int iPos, uint8_t bSetItem, bool bSelf);
		uint32_t		GetItemSetValueFromTarget(uint8_t pos);
		uint32_t		GetItemSetValueFromSelf(uint8_t pos);
#endif

#ifdef ENABLE_EXTENDED_ITEM_COUNT
		void			SetItemToTarget(DWORD pos, DWORD vnum, WORD count);
		void			SetItemToSelf(DWORD pos, DWORD vnum, WORD count);
#else
		void			SetItemToTarget(DWORD pos, DWORD vnum, BYTE count);
		void			SetItemToSelf(DWORD pos, DWORD vnum, BYTE count);
#endif

		void			SetItemMetinSocketToTarget(int pos, int imetinpos, DWORD vnum);
		void			SetItemMetinSocketToSelf(int pos, int imetinpos, DWORD vnum);

		void			SetItemAttributeToTarget(int pos, int iattrpos, BYTE byType, short sValue);
		void			SetItemAttributeToSelf(int pos, int iattrpos, BYTE byType, short sValue);

#ifdef ENABLE_GLOVE_SYSTEM
		void			SetItemApplyRandomToTarget(int pos, int iApplyPos, BYTE byType, short sValue);
		void			SetItemApplyRandomToSelf(int pos, int iApplyPos, BYTE byType, short sValue);

		void			GetItemApplyRandomFromTarget(BYTE pos, int iApplyPos, BYTE* pbyType, short* psValue);
		void			GetItemApplyRandomFromSelf(BYTE pos, int iApplyPos, BYTE* pbyType, short* psValue);
#endif

#ifdef ENABLE_REFINE_ELEMENT
		void 			SetItemRefineElementToSelf(BYTE bPos, DWORD dwRefineElement);
		void 			SetItemRefineElementToTarget(BYTE bPos, DWORD dwRefineElement);
		DWORD 			GetItemRefineElementFromSelf(BYTE bPos);
		DWORD 			GetItemRefineElementFromTarget(BYTE bPos);
#endif

		void			DelItemOfTarget(BYTE pos);
		void			DelItemOfSelf(BYTE pos);

		DWORD			GetItemVnumFromTarget(BYTE pos);
		DWORD			GetItemVnumFromSelf(BYTE pos);

#ifdef ENABLE_EXTENDED_ITEM_COUNT
		WORD			GetItemCountFromTarget(BYTE pos);
		WORD			GetItemCountFromSelf(BYTE pos);
#else
		BYTE			GetItemCountFromTarget(BYTE pos);
		BYTE			GetItemCountFromSelf(BYTE pos);
#endif

		DWORD			GetItemMetinSocketFromTarget(BYTE pos, int iMetinSocketPos);
		DWORD			GetItemMetinSocketFromSelf(BYTE pos, int iMetinSocketPos);

		void			GetItemAttributeFromTarget(BYTE pos, int iAttrPos, BYTE * pbyType, short * psValue);
		void			GetItemAttributeFromSelf(BYTE pos, int iAttrPos, BYTE * pbyType, short * psValue);
#ifdef ENABLE_CHANGELOOK_SYSTEM
		void			SetItemTransmutation(int iPos, DWORD dwTransmutation, bool bSelf);
		DWORD			GetItemTransmutation(int iPos, bool bSelf);
#endif
		void			SetAcceptToTarget(BYTE Accept);
		void			SetAcceptToSelf(BYTE Accept);

		bool			GetAcceptFromTarget();
		bool			GetAcceptFromSelf();

		bool			GetElkMode();
		void			SetElkMode(bool value);

	protected:
		bool				m_isTrading;

		bool				m_elk_mode;   // 엘크를 클릭해서 교환했을때를 위한 변종임.
		TExchangeData		m_self;
		TExchangeData		m_victim;
};
