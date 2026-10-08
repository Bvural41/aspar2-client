#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "PythonItem.h"
#include "PythonShop.h"
#include "PythonExchange.h"
#include "PythonSafeBox.h"
#include "PythonCharacterManager.h"
#include "AbstractPlayer.h"
#ifdef ENABLE_GRAPHIC_ON_OFF
#include "PythonSystem.h"
#endif

//////////////////////////////////////////////////////////////////////////
// SafeBox

bool CPythonNetworkStream::SendSafeBoxMoneyPacket(BYTE byState, DWORD dwMoney)
{
	assert(!"CPythonNetworkStream::SendSafeBoxMoneyPacket - 사용하지 않는 함수");
	return false;

//	TPacketCGSafeboxMoney kSafeboxMoney;
//	kSafeboxMoney.bHeader = HEADER_CG_SAFEBOX_MONEY;
//	kSafeboxMoney.bState = byState;
//	kSafeboxMoney.dwMoney = dwMoney;
//	if (!Send(sizeof(kSafeboxMoney), &kSafeboxMoney))
//		return false;
//
//	return SendSequence();
}

bool CPythonNetworkStream::SendSafeBoxCheckinPacket(TItemPos InventoryPos, BYTE bySafeBoxPos)
{
	__PlayInventoryItemDropSound(InventoryPos);

	TPacketCGSafeboxCheckin kSafeboxCheckin;
	kSafeboxCheckin.bHeader = HEADER_CG_SAFEBOX_CHECKIN;
	kSafeboxCheckin.ItemPos = InventoryPos;
	kSafeboxCheckin.bSafePos = bySafeBoxPos;
	if (!Send(sizeof(kSafeboxCheckin), &kSafeboxCheckin))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendSafeBoxCheckoutPacket(BYTE bySafeBoxPos, TItemPos InventoryPos)
{
	__PlaySafeBoxItemDropSound(bySafeBoxPos);

	TPacketCGSafeboxCheckout kSafeboxCheckout;
	kSafeboxCheckout.bHeader = HEADER_CG_SAFEBOX_CHECKOUT;
	kSafeboxCheckout.bSafePos = bySafeBoxPos;
	kSafeboxCheckout.ItemPos = InventoryPos;
	if (!Send(sizeof(kSafeboxCheckout), &kSafeboxCheckout))
		return false;

	return SendSequence();
}

#ifdef ENABLE_EXTENDED_ITEM_COUNT
bool CPythonNetworkStream::SendSafeBoxItemMovePacket(BYTE bySourcePos, BYTE byTargetPos, WORD byCount)
#else
bool CPythonNetworkStream::SendSafeBoxItemMovePacket(BYTE bySourcePos, BYTE byTargetPos, BYTE byCount)
#endif
{
	__PlaySafeBoxItemDropSound(bySourcePos);

	TPacketCGItemMove kItemMove;
	kItemMove.header = HEADER_CG_SAFEBOX_ITEM_MOVE;
	kItemMove.pos = TItemPos(INVENTORY, bySourcePos);
	kItemMove.num = byCount;
	kItemMove.change_pos = TItemPos(INVENTORY, byTargetPos);
	if (!Send(sizeof(kItemMove), &kItemMove))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvSafeBoxSetPacket()
{
	TPacketGCItemSet2 kItemSet;
	if (!Recv(sizeof(kItemSet), &kItemSet))
		return false;

	TItemData kItemData;
	kItemData.vnum	= kItemSet.vnum;
	kItemData.count = kItemSet.count;
#ifdef ENABLE_SOULBIND_SYSTEM
	kItemData.bind = kItemSet.bind;
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
	kItemData.transmutation = kItemSet.transmutation;
#endif
#ifdef ENABLE_REFINE_ELEMENT
	kItemData.dwRefineElement = kItemSet.dwRefineElement;
#endif
#ifdef ENABLE_SET_ITEM
	kItemData.set_value = kItemSet.set_value;
#endif
	kItemData.flags = kItemSet.flags;
	kItemData.anti_flags = kItemSet.anti_flags;
	for (int isocket=0; isocket<ITEM_SOCKET_SLOT_MAX_NUM; ++isocket)
		kItemData.alSockets[isocket] = kItemSet.alSockets[isocket];
	for (int iattr=0; iattr<ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++iattr)
		kItemData.aAttr[iattr] = kItemSet.aAttr[iattr];
#ifdef ENABLE_GLOVE_SYSTEM
	for (int iapply = 0; iapply < ITEM_APPLY_RANDOM_SLOT_MAX_NUM; ++iapply)
		kItemData.aApplyRandom[iapply] = kItemSet.aApplyRandom[iapply];
#endif
	CPythonSafeBox::Instance().SetItemData(kItemSet.Cell.cell, kItemData);

	__RefreshSafeboxWindow();

	return true;
}

bool CPythonNetworkStream::RecvSafeBoxDelPacket()
{
	TPacketGCItemDel kItemDel;
	if (!Recv(sizeof(kItemDel), &kItemDel))
		return false;

	CPythonSafeBox::Instance().DelItemData(kItemDel.pos);

	__RefreshSafeboxWindow();

	return true;
}

bool CPythonNetworkStream::RecvSafeBoxWrongPasswordPacket()
{
	TPacketGCSafeboxWrongPassword kSafeboxWrongPassword;

	if (!Recv(sizeof(kSafeboxWrongPassword), &kSafeboxWrongPassword))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnSafeBoxError", Py_BuildValue("()"));

	return true;
}

bool CPythonNetworkStream::RecvSafeBoxSizePacket()
{
	TPacketGCSafeboxSize kSafeBoxSize;
	if (!Recv(sizeof(kSafeBoxSize), &kSafeBoxSize))
		return false;

	CPythonSafeBox::Instance().OpenSafeBox(kSafeBoxSize.bSize);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenSafeboxWindow", Py_BuildValue("(i)", kSafeBoxSize.bSize));

	return true;
}

bool CPythonNetworkStream::RecvSafeBoxMoneyChangePacket()
{
	TPacketGCSafeboxMoneyChange kMoneyChange;
	if (!Recv(sizeof(kMoneyChange), &kMoneyChange))
		return false;

	CPythonSafeBox::Instance().SetMoney(kMoneyChange.dwMoney);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSafeboxMoney", Py_BuildValue("()"));

	return true;
}

// SafeBox
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Mall
bool CPythonNetworkStream::SendMallCheckoutPacket(BYTE byMallPos, TItemPos InventoryPos)
{
	__PlayMallItemDropSound(byMallPos);

	TPacketCGMallCheckout kMallCheckoutPacket;
	kMallCheckoutPacket.bHeader = HEADER_CG_MALL_CHECKOUT;
	kMallCheckoutPacket.bMallPos = byMallPos;
	kMallCheckoutPacket.ItemPos = InventoryPos;
	if (!Send(sizeof(kMallCheckoutPacket), &kMallCheckoutPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvMallOpenPacket()
{
	TPacketGCMallOpen kMallOpen;
	if (!Recv(sizeof(kMallOpen), &kMallOpen))
		return false;

	CPythonSafeBox::Instance().OpenMall(kMallOpen.bSize);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenMallWindow", Py_BuildValue("(i)", kMallOpen.bSize));

	return true;
}
bool CPythonNetworkStream::RecvMallItemSetPacket()
{
	TPacketGCItemSet2 kItemSet;
	if (!Recv(sizeof(kItemSet), &kItemSet))
		return false;

	TItemData kItemData;
	kItemData.vnum = kItemSet.vnum;
	kItemData.count = kItemSet.count;
#ifdef ENABLE_SOULBIND_SYSTEM
	kItemData.bind = kItemSet.bind;
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
	kItemData.transmutation = kItemSet.transmutation;
#endif
#ifdef ENABLE_REFINE_ELEMENT
	kItemData.dwRefineElement = kItemSet.dwRefineElement;
#endif
#ifdef ENABLE_SET_ITEM
	kItemData.set_value = kItemSet.set_value;
#endif
	kItemData.flags = kItemSet.flags;
	kItemData.anti_flags = kItemSet.anti_flags;
	for (int isocket=0; isocket<ITEM_SOCKET_SLOT_MAX_NUM; ++isocket)
		kItemData.alSockets[isocket] = kItemSet.alSockets[isocket];
	for (int iattr=0; iattr<ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++iattr)
		kItemData.aAttr[iattr] = kItemSet.aAttr[iattr];
#ifdef ENABLE_GLOVE_SYSTEM
	for (int iapply = 0; iapply < ITEM_APPLY_RANDOM_SLOT_MAX_NUM; ++iapply)
		kItemData.aApplyRandom[iapply] = kItemSet.aApplyRandom[iapply];
#endif
	CPythonSafeBox::Instance().SetMallItemData(kItemSet.Cell.cell, kItemData);

	__RefreshMallWindow();

	return true;
}
bool CPythonNetworkStream::RecvMallItemDelPacket()
{
	TPacketGCItemDel kItemDel;
	if (!Recv(sizeof(kItemDel), &kItemDel))
		return false;

	CPythonSafeBox::Instance().DelMallItemData(kItemDel.pos);

	__RefreshMallWindow();
	Tracef(" >> CPythonNetworkStream::RecvMallItemDelPacket\n");

	return true;
}
// Mall
//////////////////////////////////////////////////////////////////////////

// Item
// Recieve
bool CPythonNetworkStream::RecvItemSetPacket()
{
	TPacketGCItemSet packet_item_set;

	if (!Recv(sizeof(TPacketGCItemSet), &packet_item_set))
		return false;

	TItemData kItemData;
	kItemData.vnum	= packet_item_set.vnum;
	kItemData.count	= packet_item_set.count;
#ifdef ENABLE_SOULBIND_SYSTEM
	kItemData.bind = packet_item_set.bind;
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
	kItemData.transmutation = packet_item_set.transmutation;
#endif
#ifdef ENABLE_REFINE_ELEMENT
	kItemData.dwRefineElement = packet_item_set.dwRefineElement;
#endif
#ifdef ENABLE_SET_ITEM
	kItemData.set_value = packet_item_set.set_value;
#endif
	kItemData.flags = packet_item_set.flags;
	kItemData.anti_flags = packet_item_set.anti_flags;

	for (int i=0; i<ITEM_SOCKET_SLOT_MAX_NUM; ++i)
		kItemData.alSockets[i]=packet_item_set.alSockets[i];
	for (int j=0; j<ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
		kItemData.aAttr[j]=packet_item_set.aAttr[j];
#ifdef ENABLE_GLOVE_SYSTEM
	for (int k = 0; k < ITEM_APPLY_RANDOM_SLOT_MAX_NUM; ++k)
		kItemData.aApplyRandom[k] = packet_item_set.aApplyRandom[k];
#endif
	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	rkPlayer.SetItemData(packet_item_set.Cell, kItemData);
#ifdef ENABLE_SWITCHBOT
	if (packet_item_set.Cell.window_type == SWITCHBOT)
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSwitchbotWindow", Py_BuildValue("()"));
		return true;
	}
#endif
	if (packet_item_set.highlight)
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Highlight_Item", Py_BuildValue("(ii)", packet_item_set.Cell.window_type, packet_item_set.Cell.cell));
	
#ifdef ENABLE_GIFTBOX_MULTI_OPEN
	if (packet_item_set.update == 1)
		__RefreshInventoryWindow();
#else
	__RefreshInventoryWindow();
#endif
	return true;
}

bool CPythonNetworkStream::RecvItemSetPacket2()
{
	TPacketGCItemSet2 packet_item_set;

	if (!Recv(sizeof(TPacketGCItemSet2), &packet_item_set))
		return false;

	TItemData kItemData;
	kItemData.vnum	= packet_item_set.vnum;
	kItemData.count	= packet_item_set.count;
#ifdef ENABLE_SOULBIND_SYSTEM
	kItemData.bind = packet_item_set.bind;
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
	kItemData.transmutation = packet_item_set.transmutation;
#endif
#ifdef ENABLE_REFINE_ELEMENT
	kItemData.dwRefineElement = packet_item_set.dwRefineElement;
#endif
#ifdef ENABLE_SET_ITEM
	kItemData.set_value = packet_item_set.set_value;
#endif
	kItemData.flags = packet_item_set.flags;
	kItemData.anti_flags = packet_item_set.anti_flags;

	for (int i=0; i<ITEM_SOCKET_SLOT_MAX_NUM; ++i)
		kItemData.alSockets[i]=packet_item_set.alSockets[i];
	for (int j=0; j<ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
		kItemData.aAttr[j]=packet_item_set.aAttr[j];
#ifdef ENABLE_GLOVE_SYSTEM
	for (int k = 0; k < ITEM_APPLY_RANDOM_SLOT_MAX_NUM; ++k)
		kItemData.aApplyRandom[k] = packet_item_set.aApplyRandom[k];
#endif
	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	rkPlayer.SetItemData(packet_item_set.Cell, kItemData);
#ifdef ENABLE_SWITCHBOT
	if (packet_item_set.Cell.window_type == SWITCHBOT)
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSwitchbotWindow", Py_BuildValue("()"));
		return true;
	}
#endif
	if (packet_item_set.highlight)
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Highlight_Item", Py_BuildValue("(ii)", packet_item_set.Cell.window_type, packet_item_set.Cell.cell));
	
#ifdef ENABLE_GIFTBOX_MULTI_OPEN
	if (packet_item_set.update == 1)
		__RefreshInventoryWindow();
#else
	__RefreshInventoryWindow();
#endif
	return true;
}

bool CPythonNetworkStream::RecvItemUsePacket()
{
	TPacketGCItemUse packet_item_use;

	if (!Recv(sizeof(TPacketGCItemUse), &packet_item_use))
		return false;

	__RefreshInventoryWindow();
	return true;
}

bool CPythonNetworkStream::RecvItemUpdatePacket()
{
	TPacketGCItemUpdate packet_item_update;

	if (!Recv(sizeof(TPacketGCItemUpdate), &packet_item_update))
		return false;

	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	rkPlayer.SetItemCount(packet_item_update.Cell, packet_item_update.count, true);
	// rkPlayer.SetItemCount(packet_item_update.Cell, packet_item_update.count); //Bvural41 06-03-26 orjinal kod 500ms deaktif
#ifdef ENABLE_SOULBIND_SYSTEM
	rkPlayer.SetItemBind(packet_item_update.Cell, packet_item_update.bind);
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
	rkPlayer.SetItemTransmutation(packet_item_update.Cell, packet_item_update.transmutation);
#endif
#ifdef ENABLE_REFINE_ELEMENT
	rkPlayer.SetItemRefineElement(packet_item_update.Cell, packet_item_update.dwRefineElement);
#endif
#ifdef ENABLE_SET_ITEM
	rkPlayer.SetItemSetValue(packet_item_update.Cell, packet_item_update.set_value);
#endif
	for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
		rkPlayer.SetItemMetinSocket(packet_item_update.Cell, i, packet_item_update.alSockets[i]);
	for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
		rkPlayer.SetItemAttribute(packet_item_update.Cell, j, packet_item_update.aAttr[j].bType, packet_item_update.aAttr[j].sValue);
#ifdef ENABLE_GLOVE_SYSTEM
	for (int j = 0; j < ITEM_APPLY_RANDOM_SLOT_MAX_NUM; ++j)
		rkPlayer.SetItemApplyRandom(packet_item_update.Cell, j, packet_item_update.aApplyRandom[j].bType, packet_item_update.aApplyRandom[j].sValue);
#endif
#ifdef ENABLE_GIFTBOX_MULTI_OPEN
	if (packet_item_update.update == 1)
		__RefreshInventoryWindow();
#else
	__RefreshInventoryWindow();
#endif
	return true;
}

bool CPythonNetworkStream::RecvItemGroundAddPacket()
{
	TPacketGCItemGroundAdd recv;
	if (!Recv(sizeof(TPacketGCItemGroundAdd), &recv))
		return false;

	__GlobalPositionToLocalPosition(recv.lX, recv.lY);

#ifdef ENABLE_ITEM_DROP_RENEWAL
	CPythonItem::Instance().CreateItem(recv.dwVID, recv.dwVnum, recv.lX, recv.lY, recv.lZ, true, recv.alSockets, recv.aAttrs
#ifdef ENABLE_SET_ITEM
		, recv.set_value
#endif
	);
#else
	CPythonItem::Instance().CreateItem(recv.dwVID, recv.dwVnum, recv.lX, recv.lY, recv.lZ, true
#ifdef ENABLE_SET_ITEM
		, recv.set_value
#endif
	);
#endif
	return true;
}

bool CPythonNetworkStream::RecvItemOwnership()
{
	TPacketGCItemOwnership p;

	if (!Recv(sizeof(TPacketGCItemOwnership), &p))
		return false;

	CPythonItem::Instance().SetOwnership(p.dwVID, p.szName);
	return true;
}

#ifdef ENABLE_FISH_EVENT_SYSTEM
bool CPythonNetworkStream::SendFishBoxUse(BYTE bWindow, WORD wCell)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGFishEvent packetFishEvent;
	packetFishEvent.bHeader = HEADER_CG_FISH_EVENT_SEND;
	packetFishEvent.bSubheader = FISH_EVENT_SUBHEADER_BOX_USE;

	if (!Send(sizeof(TPacketCGFishEvent), &packetFishEvent))
	{
		Tracef("SendFishEventPacket Error\n");
		return false;
	}
	
	if (!Send(sizeof(bWindow), &bWindow))
	{
		Tracef("SendFishBoxUse Error\n");
		return false;
	}
	
	if (!Send(sizeof(wCell), &wCell))
	{
		Tracef("SendFishBoxUse Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendFishShapeAdd(BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGFishEvent packetFishEvent;
	packetFishEvent.bHeader = HEADER_CG_FISH_EVENT_SEND;
	packetFishEvent.bSubheader = FISH_EVENT_SUBHEADER_SHAPE_ADD;

	if (!Send(sizeof(TPacketCGFishEvent), &packetFishEvent))
	{
		Tracef("SendFishEventPacket Error\n");
		return false;
	}
	
	if (!Send(sizeof(BYTE), &bPos))
	{
		Tracef("SendFishShapeAdd Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvFishEventInfo()
{
	TPacketGCFishEventInfo fishEventPacket;

	if (!Recv(sizeof(fishEventPacket), &fishEventPacket))
		return false;

	switch (fishEventPacket.bSubheader)
	{
		case FISH_EVENT_SUBHEADER_BOX_USE:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameFishUse", Py_BuildValue("(ii)", fishEventPacket.dwFirstArg, fishEventPacket.dwSecondArg));
			break;
			
		case FISH_EVENT_SUBHEADER_SHAPE_ADD:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameFishAdd", Py_BuildValue("(ii)", fishEventPacket.dwFirstArg, fishEventPacket.dwSecondArg));
			break;
			
		case FISH_EVENT_SUBHEADER_GC_REWARD:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameFishReward", Py_BuildValue("(i)", fishEventPacket.dwFirstArg));
			break;
			
		case FISH_EVENT_SUBHEADER_GC_ENABLE:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameFishEvent", Py_BuildValue("(ii)", fishEventPacket.dwFirstArg, fishEventPacket.dwSecondArg));
			break;
	};

	return true;
}
#endif

bool CPythonNetworkStream::RecvItemGroundDelPacket()
{
	TPacketGCItemGroundDel	packet_item_ground_del;

	if (!Recv(sizeof(TPacketGCItemGroundDel), &packet_item_ground_del))
		return false;

	CPythonItem::Instance().DeleteItem(packet_item_ground_del.vid);
	return true;
}

bool CPythonNetworkStream::RecvQuickSlotAddPacket()
{
	TPacketGCQuickSlotAdd packet_quick_slot_add;

	if (!Recv(sizeof(TPacketGCQuickSlotAdd), &packet_quick_slot_add))
		return false;

	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	rkPlayer.AddQuickSlot(packet_quick_slot_add.pos, packet_quick_slot_add.slot.Type, packet_quick_slot_add.slot.Position);

	__RefreshInventoryWindow();

	return true;
}

bool CPythonNetworkStream::RecvQuickSlotDelPacket()
{
	TPacketGCQuickSlotDel packet_quick_slot_del;

	if (!Recv(sizeof(TPacketGCQuickSlotDel), &packet_quick_slot_del))
		return false;

	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	rkPlayer.DeleteQuickSlot(packet_quick_slot_del.pos);

	__RefreshInventoryWindow();

	return true;
}

bool CPythonNetworkStream::RecvQuickSlotMovePacket()
{
	TPacketGCQuickSlotSwap packet_quick_slot_swap;

	if (!Recv(sizeof(TPacketGCQuickSlotSwap), &packet_quick_slot_swap))
		return false;

	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	rkPlayer.MoveQuickSlot(packet_quick_slot_swap.pos, packet_quick_slot_swap.change_pos);

	__RefreshInventoryWindow();

	return true;
}



bool CPythonNetworkStream::SendShopEndPacket()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGShop packet_shop;
	packet_shop.header = HEADER_CG_SHOP;
	packet_shop.subheader = SHOP_SUBHEADER_CG_END;

	if (!Send(sizeof(packet_shop), &packet_shop))
	{
		Tracef("SendShopEndPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendShopBuyPacket(BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;
	
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_BUY;

	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendShopBuyPacket Error\n");
		return false;
	}

	BYTE bCount=1;
	if (!Send(sizeof(BYTE), &bCount))
	{
		Tracef("SendShopBuyPacket Error\n");
		return false;
	}

	if (!Send(sizeof(BYTE), &bPos))
	{
		Tracef("SendShopBuyPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendShopSellPacket(BYTE bySlot)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_SELL;

	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendShopSellPacket Error\n");
		return false;
	}
	if (!Send(sizeof(BYTE), &bySlot))
	{
		Tracef("SendShopAddSellPacket Error\n");
		return false;
	}

	return SendSequence();
}

#ifdef ENABLE_EXTENDED_ITEM_COUNT
bool CPythonNetworkStream::SendShopSellPacketNew(WORD wSlot, WORD byCount, BYTE byType)
#else
bool CPythonNetworkStream::SendShopSellPacketNew(WORD wSlot, BYTE byCount, BYTE byType)
#endif
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_SELL2;

	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendShopSellPacket Error\n");
		return false;
	}
	if (!Send(sizeof(WORD), &wSlot))
	{
		Tracef("SendShopAddSellPacket Error\n");
		return false;
	}
#ifdef ENABLE_EXTENDED_ITEM_COUNT
	if (!Send(sizeof(WORD), &byCount))
#else
	if (!Send(sizeof(BYTE), &byCount))
#endif
	{
		Tracef("SendShopAddSellPacket Error\n");
		return false;
	}
	if (!Send(sizeof(BYTE), &byType))
	{
		Tracef("SendShopAddSellPacket Error\n");
		return false;
	}

	Tracef(" SendShopSellPacketNew(wSlot=%d, byCount=%d, byType=%d)\n", wSlot, byCount, byType);

	return SendSequence();
}

// Send
bool CPythonNetworkStream::SendItemUsePacket(TItemPos pos
#ifdef ENABLE_GIFTBOX_MULTI_OPEN
	, ICOUNT open_count
#endif
)
{
	if (!__CanActMainInstance())
		return true;

	if (__IsEquipItemInSlot(pos))
	{
		if (CPythonExchange::Instance().isTrading())
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AppendNotifyMessage", Py_BuildValue("(s)", "CANNOT_EQUIP_EXCHANGE"));
			return true;
		}

		if (CPythonShop::Instance().IsOpen())
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AppendNotifyMessage", Py_BuildValue("(s)", "CANNOT_EQUIP_SHOP"));
			return true;
		}

		if (__IsPlayerAttacking())
			return true;
	}

	__PlayInventoryItemUseSound(pos);

	TPacketCGItemUse itemUsePacket;
	itemUsePacket.header = HEADER_CG_ITEM_USE;
	itemUsePacket.pos = pos;

#ifdef ENABLE_GIFTBOX_MULTI_OPEN
	itemUsePacket.open_count = open_count;
#endif

	if (!Send(sizeof(TPacketCGItemUse), &itemUsePacket))
	{
		Tracen("SendItemUsePacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendItemUseToItemPacket(TItemPos source_pos, TItemPos target_pos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGItemUseToItem itemUseToItemPacket;
	itemUseToItemPacket.header = HEADER_CG_ITEM_USE_TO_ITEM;
	itemUseToItemPacket.source_pos = source_pos;
	itemUseToItemPacket.target_pos = target_pos;

	if (!Send(sizeof(TPacketCGItemUseToItem), &itemUseToItemPacket))
	{
		Tracen("SendItemUseToItemPacket Error");
		return false;
	}

#ifdef _DEBUG
	Tracef(" << SendItemUseToItemPacket(src=%d, dst=%d)\n", source_pos, target_pos);
#endif

	return SendSequence();
}

#ifdef ENABLE_CHEQUE_SYSTEM
bool CPythonNetworkStream::SendItemDropPacket(TItemPos pos, DWORD elk, DWORD won)
#else
bool CPythonNetworkStream::SendItemDropPacket(TItemPos pos, DWORD elk)
#endif
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGItemDrop itemDropPacket;
	itemDropPacket.header = HEADER_CG_ITEM_DROP;
	itemDropPacket.pos = pos;
	itemDropPacket.elk = elk;
#ifdef ENABLE_CHEQUE_SYSTEM
	itemDropPacket.won = won;
#endif

	if (!Send(sizeof(TPacketCGItemDrop), &itemDropPacket))
	{
		Tracen("SendItemDropPacket Error");
		return false;
	}

	return SendSequence();
}

#ifdef ENABLE_CHEQUE_SYSTEM
bool CPythonNetworkStream::SendItemDropPacketNew(TItemPos pos, DWORD elk, DWORD won, DWORD count)
#else
bool CPythonNetworkStream::SendItemDropPacketNew(TItemPos pos, DWORD elk, DWORD count)
#endif
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGItemDrop2 itemDropPacket;
	itemDropPacket.header = HEADER_CG_ITEM_DROP2;
	itemDropPacket.pos = pos;
	itemDropPacket.gold = elk;
#ifdef ENABLE_CHEQUE_SYSTEM
	itemDropPacket.cheque = won;
#endif
	itemDropPacket.count = count;

	if (!Send(sizeof(itemDropPacket), &itemDropPacket))
	{
		Tracen("SendItemDropPacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendItemDestroyPacket(TItemPos pos)
{
    if (!__CanActMainInstance())
        return true;
    TPacketCGItemDestroy itemDestroyPacket;
    itemDestroyPacket.header = HEADER_CG_ITEM_DESTROY;
    itemDestroyPacket.pos = pos;
    if (!Send(sizeof(itemDestroyPacket), &itemDestroyPacket))
    {
        Tracen("SendItemDestroyPacket Error");
        return false;
    }
    return SendSequence();
}

bool CPythonNetworkStream::SendItemSellPacket(TItemPos pos)
{
    if (!__CanActMainInstance())
        return true;
    TPacketCGItemSell itemSellPacket;
    itemSellPacket.header = HEADER_CG_ITEM_SELL;
    itemSellPacket.pos = pos;
    if (!Send(sizeof(itemSellPacket), &itemSellPacket))
    {
        Tracen("SendItemDestroyPacket Error");
        return false;
    }
    return SendSequence();	
}

bool CPythonNetworkStream::__IsEquipItemInSlot(TItemPos uSlotPos)
{
	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	return rkPlayer.IsEquipItemInSlot(uSlotPos);
}

void CPythonNetworkStream::__PlayInventoryItemUseSound(TItemPos uSlotPos)
{
	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	DWORD dwItemID=rkPlayer.GetItemIndex(uSlotPos);

	CPythonItem& rkItem=CPythonItem::Instance();
	rkItem.PlayUseSound(dwItemID);
}

void CPythonNetworkStream::__PlayInventoryItemDropSound(TItemPos uSlotPos)
{
	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	DWORD dwItemID=rkPlayer.GetItemIndex(uSlotPos);

	CPythonItem& rkItem=CPythonItem::Instance();
	rkItem.PlayDropSound(dwItemID);
}

void CPythonNetworkStream::__PlaySafeBoxItemDropSound(UINT uSlotPos)
{
	DWORD dwItemID;
	CPythonSafeBox& rkSafeBox=CPythonSafeBox::Instance();
	if (!rkSafeBox.GetSlotItemID(uSlotPos, &dwItemID))
		return;

	CPythonItem& rkItem=CPythonItem::Instance();
	rkItem.PlayDropSound(dwItemID);
}

void CPythonNetworkStream::__PlayMallItemDropSound(UINT uSlotPos)
{
	DWORD dwItemID;
	CPythonSafeBox& rkSafeBox=CPythonSafeBox::Instance();
	if (!rkSafeBox.GetSlotMallItemID(uSlotPos, &dwItemID))
		return;

	CPythonItem& rkItem=CPythonItem::Instance();
	rkItem.PlayDropSound(dwItemID);
}

#ifdef ENABLE_EXTENDED_ITEM_COUNT
bool CPythonNetworkStream::SendItemMovePacket(TItemPos pos, TItemPos change_pos, WORD num)
#else
bool CPythonNetworkStream::SendItemMovePacket(TItemPos pos, TItemPos change_pos, BYTE num)
#endif
{
	if (!__CanActMainInstance())
		return true;
	
	if (__IsEquipItemInSlot(pos))
	{
		if (CPythonExchange::Instance().isTrading())
		{
			if (pos.IsEquipCell() || change_pos.IsEquipCell())
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AppendNotifyMessage", Py_BuildValue("(s)", "CANNOT_EQUIP_EXCHANGE"));
				return true;
			}
		}

		if (CPythonShop::Instance().IsOpen())
		{
			if (pos.IsEquipCell() || change_pos.IsEquipCell())
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AppendNotifyMessage", Py_BuildValue("(s)", "CANNOT_EQUIP_SHOP"));
				return true;
			}
		}

		if (__IsPlayerAttacking())
			return true;
	}

	__PlayInventoryItemDropSound(pos);

	TPacketCGItemMove	itemMovePacket;
	itemMovePacket.header = HEADER_CG_ITEM_MOVE;
	itemMovePacket.pos = pos;
	itemMovePacket.change_pos = change_pos;
	itemMovePacket.num = num;

	if (!Send(sizeof(TPacketCGItemMove), &itemMovePacket))
	{
		Tracen("SendItemMovePacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendItemPickUpPacket(DWORD vid)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGItemPickUp	itemPickUpPacket;
	itemPickUpPacket.header = HEADER_CG_ITEM_PICKUP;
	itemPickUpPacket.vid = vid;

	if (!Send(sizeof(TPacketCGItemPickUp), &itemPickUpPacket))
	{
		Tracen("SendItemPickUpPacket Error");
		return false;
	}

	return SendSequence();
}


bool CPythonNetworkStream::SendQuickSlotAddPacket(UINT wpos, BYTE type, UINT pos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGQuickSlotAdd quickSlotAddPacket;

	quickSlotAddPacket.header		= HEADER_CG_QUICKSLOT_ADD;
	quickSlotAddPacket.pos			= wpos;
	quickSlotAddPacket.slot.Type	= type;
	quickSlotAddPacket.slot.Position = pos;

	if (!Send(sizeof(TPacketCGQuickSlotAdd), &quickSlotAddPacket))
	{
		Tracen("SendQuickSlotAddPacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendQuickSlotDelPacket(UINT pos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGQuickSlotDel quickSlotDelPacket;

	quickSlotDelPacket.header = HEADER_CG_QUICKSLOT_DEL;
	quickSlotDelPacket.pos = pos;

	if (!Send(sizeof(TPacketCGQuickSlotDel), &quickSlotDelPacket))
	{
		Tracen("SendQuickSlotDelPacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendQuickSlotMovePacket(UINT pos, UINT change_pos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGQuickSlotSwap quickSlotSwapPacket;

	quickSlotSwapPacket.header = HEADER_CG_QUICKSLOT_SWAP;
	quickSlotSwapPacket.pos = pos;
	quickSlotSwapPacket.change_pos = change_pos;

	if (!Send(sizeof(TPacketCGQuickSlotSwap), &quickSlotSwapPacket))
	{
		Tracen("SendQuickSlotSwapPacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvSpecialEffect()
{
	TPacketGCSpecialEffect kSpecialEffect;
	if (!Recv(sizeof(kSpecialEffect), &kSpecialEffect))
		return false;

	DWORD effect = -1;
	bool bPlayPotionSound = false;	//포션을 먹을 경우는 포션 사운드를 출력하자.!!
	bool bAttachEffect = true;		//캐리터에 붙는 어태치 이펙트와 일반 이펙트 구분.!!
	switch (kSpecialEffect.type)
	{
		case SE_HPUP_RED:
			effect = CInstanceBase::EFFECT_HPUP_RED;
			bPlayPotionSound = true;
			break;
		case SE_SPUP_BLUE:
			effect = CInstanceBase::EFFECT_SPUP_BLUE;
			bPlayPotionSound = true;
			break;
		case SE_SPEEDUP_GREEN:
			effect = CInstanceBase::EFFECT_SPEEDUP_GREEN;
			bPlayPotionSound = true;
			break;
		case SE_DXUP_PURPLE:
			effect = CInstanceBase::EFFECT_DXUP_PURPLE;
			bPlayPotionSound = true;
			break;
		case SE_CRITICAL:
			effect = CInstanceBase::EFFECT_CRITICAL;
			break;
		case SE_PENETRATE:
			effect = CInstanceBase::EFFECT_PENETRATE;
			break;
		case SE_BLOCK:
			effect = CInstanceBase::EFFECT_BLOCK;
			break;
		case SE_DODGE:
			effect = CInstanceBase::EFFECT_DODGE;
			break;
		case SE_CHINA_FIREWORK:
			effect = CInstanceBase::EFFECT_FIRECRACKER;
			bAttachEffect = false;
			break;
		case SE_SPIN_TOP:
			effect = CInstanceBase::EFFECT_SPIN_TOP;
			bAttachEffect = false;
			break;
		case SE_SUCCESS :
			effect = CInstanceBase::EFFECT_SUCCESS ;
			bAttachEffect = false ;
			break ;
		case SE_FAIL :
			effect = CInstanceBase::EFFECT_FAIL ;
			break ;
		case SE_FR_SUCCESS:
			effect = CInstanceBase::EFFECT_FR_SUCCESS;
			bAttachEffect = false ;
			break;
		case SE_LEVELUP_ON_14_FOR_GERMANY:	//레벨업 14일때 ( 독일전용 )
			effect = CInstanceBase::EFFECT_LEVELUP_ON_14_FOR_GERMANY;
			bAttachEffect = false ;
			break;
		case SE_LEVELUP_UNDER_15_FOR_GERMANY: //레벨업 15일때 ( 독일전용 )
			effect = CInstanceBase::EFFECT_LEVELUP_UNDER_15_FOR_GERMANY;
			bAttachEffect = false ;
			break;
		case SE_PERCENT_DAMAGE1:
			effect = CInstanceBase::EFFECT_PERCENT_DAMAGE1;
			break;
		case SE_PERCENT_DAMAGE2:
			effect = CInstanceBase::EFFECT_PERCENT_DAMAGE2;
			break;
		case SE_PERCENT_DAMAGE3:
			effect = CInstanceBase::EFFECT_PERCENT_DAMAGE3;
			break;
		case SE_AUTO_HPUP:
			effect = CInstanceBase::EFFECT_AUTO_HPUP;
			break;
		case SE_AUTO_SPUP:
			effect = CInstanceBase::EFFECT_AUTO_SPUP;
			break;
		case SE_EQUIP_RAMADAN_RING:
			effect = CInstanceBase::EFFECT_RAMADAN_RING_EQUIP;
			break;
		case SE_EQUIP_HALLOWEEN_CANDY:
			effect = CInstanceBase::EFFECT_HALLOWEEN_CANDY_EQUIP;
			break;
		case SE_EQUIP_HAPPINESS_RING:
 			effect = CInstanceBase::EFFECT_HAPPINESS_RING_EQUIP;
			break;
		case SE_EQUIP_LOVE_PENDANT:
			effect = CInstanceBase::EFFECT_LOVE_PENDANT_EQUIP;
			break;
#ifdef ENABLE_PASSIVE_SYSTEM
		case SE_PASSIVE_EFFECT:
			effect = CInstanceBase::EFFECT_PASSIVE;
			break;
#endif
#ifdef ENABLE_EQUIPMENT_AFFECT
		case SE_EQUIPMENT_1:
			effect = CInstanceBase::EFFECT_EQUIPMENT_1;
			break;
		case SE_EQUIPMENT_2:
			effect = CInstanceBase::EFFECT_EQUIPMENT_2;
			break;
		case SE_EQUIPMENT_3:
			effect = CInstanceBase::EFFECT_EQUIPMENT_3;
			break;
		case SE_EQUIPMENT_4:
			effect = CInstanceBase::EFFECT_EQUIPMENT_4;
			break;
		case SE_EQUIPMENT_5:
			effect = CInstanceBase::EFFECT_EQUIPMENT_5;
			break;
		case SE_EQUIPMENT_6:
			effect = CInstanceBase::EFFECT_EQUIPMENT_6;
			break;
#endif
#ifdef ENABLE_BATTLE_FIELD
		case SE_COMBAT_ZONE_POTION:
			effect = CInstanceBase::EFFECT_COMBAT_ZONE_POTION;
			break;
#endif
#ifdef ENABLE_SASH_SYSTEM
		case SE_EFFECT_SASH_SUCCEDED:
			effect = CInstanceBase::EFFECT_SASH_SUCCEDED;
			break;
		case SE_EFFECT_SASH_EQUIP:
			effect = CInstanceBase::EFFECT_SASH_EQUIP;
			break;
#endif
#ifdef VERSION_162_ENABLED
		case SE_EFFECT_HEALER:
			effect = CInstanceBase::EFFECT_HEALER;
			break;
#endif

#ifdef ENABLE_12ZI
		case SE_SKILL_DAMAGE_ZONE:
			effect = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE;
			break;

		case SE_SKILL_DAMAGE_ZONE_BUYUK:
			effect = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_BUYUK;
			break;

		case SE_SKILL_DAMAGE_ZONE_ORTA:
			effect = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_ORTA;
			break;

		case SE_SKILL_DAMAGE_ZONE_KUCUK:
			effect = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_KUCUK;
			break;

		case SE_SKILL_SAFE_ZONE:
			effect = CInstanceBase::EFFECT_SKILL_SAFE_ZONE;
			break;

		case SE_SKILL_SAFE_ZONE_BUYUK:
			effect = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_BUYUK;
			break;

		case SE_SKILL_SAFE_ZONE_ORTA:
			effect = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_ORTA;
			break;

		case SE_SKILL_SAFE_ZONE_KUCUK:
			effect = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_KUCUK;
			break;

		case SE_METEOR:
			effect = CInstanceBase::EFFECT_METEOR;
			break;

		case SE_BEAD_RAIN:
			effect = CInstanceBase::EFFECT_BEAD_RAIN;
			break;

		case SE_FALL_ROCK:
			effect = CInstanceBase::EFFECT_FALL_ROCK;
			break;

		case SE_ARROW_RAIN:
			effect = CInstanceBase::EFFECT_ARROW_RAIN;
			break;

		case SE_HORSE_DROP:
			effect = CInstanceBase::EFFECT_HORSE_DROP;
			break;

		case SE_EGG_DROP:
			effect = CInstanceBase::EFFECT_EGG_DROP;
			break;

		case SE_DEAPO_BOOM:
			effect = CInstanceBase::EFFECT_DEAPO_BOOM;
			break;
#endif

#ifdef ENABLE_AGGREGATE_MONSTER_EFFECT
		case SE_AGGREGATE_MONSTER_EFFECT:
			effect = CInstanceBase::EFFECT_AGGREGATE_MONSTER;
			break;
#endif

#ifdef ENABLE_QUEEN_NETHIS
		case SE_EFFECT_SNAKE_REGEN:
			effect = CInstanceBase::EFFECT_SNAKE_REGEN;
			bAttachEffect = false;
			break;
#endif

#ifdef ENABLE_GROWTH_PET_SYSTEM
		case SE_EFFECT_PET_SKILL_RESTAURATION:
			effect = CInstanceBase::EFFECT_PET_SKILL_RESTAURATION;
			break;
		case SE_EFFECT_PET_SKILL_IMMORTAL:
			effect = CInstanceBase::EFFECT_PET_SKILL_IMMORTAL;
			break;
		case SE_EFFECT_PET_SKILL_PANACEA:
			effect = CInstanceBase::EFFECT_PET_SKILL_PANACEA;
			break;
		case SE_EFFECT_PET_SKILL_FEATHERLIGHT:
			effect = CInstanceBase::EFFECT_PET_SKILL_FEATHERLIGHT;
			break;
		case SE_EFFECT_PET_CHANGE_ATTR:
			effect = CInstanceBase::EFFECT_PET_CHANGE_ATTR;
			break;
#endif

#if defined(ENABLE_DEFENSE_WAVE)
		case SE_DEFENSE_WAVE_LASER:
			effect = CInstanceBase::EFFECT_DEFENSE_WAVE_LASER;
			break;
#endif

#ifdef ENABLE_FLOWER_EVENT
		case SE_FLOWER_EVENT:
			effect = CInstanceBase::EFFECT_FLOWER_EVENT;
			break;
#endif

		default:
			TraceError("%d 는 없는 스페셜 이펙트 번호입니다.TPacketGCSpecialEffect",kSpecialEffect.type);
			break;
	}

#ifdef ENABLE_GRAPHIC_ON_OFF
	if (-1 != effect && CPythonSystem::Instance().GetEffectLevel() != 4)
	{
		if (CPythonSystem::Instance().GetEffectLevel() == 0 || CPythonSystem::Instance().GetEffectLevel() == 2)
		{
			CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(kSpecialEffect.vid);
			if (pInstance)
			{
				if (bAttachEffect)
					pInstance->AttachSpecialEffect(effect);
				else
					pInstance->CreateSpecialEffect(effect);
			}
		}
		else if (CPythonSystem::Instance().GetEffectLevel() == 1 || CPythonSystem::Instance().GetEffectLevel() == 3)
		{
			IAbstractPlayer & rkPlayer = IAbstractPlayer::GetSingleton();
			if (rkPlayer.IsMainCharacterIndex(kSpecialEffect.vid))
			{
				CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(kSpecialEffect.vid);
				if (pInstance)
				{
					if (bAttachEffect)
						pInstance->AttachSpecialEffect(effect);
					else
						pInstance->CreateSpecialEffect(effect);
				}
			}
		}
	}
#else
	if (-1 != effect)
	{
		CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(kSpecialEffect.vid);
		if (pInstance)
		{
			if (bAttachEffect)
				pInstance->AttachSpecialEffect(effect);
			else
				pInstance->CreateSpecialEffect(effect);
		}
	}
#endif

	return true;
}

#ifdef ENABLE_12ZI
bool CPythonNetworkStream::RecvSpecialZodiacEffect()
{
	TPacketGCSpecialZodiacEffect kSpecialZodiacEffect;
	if (!Recv(sizeof(kSpecialZodiacEffect), &kSpecialZodiacEffect))
		return false;

	DWORD effect = -1;
	switch (kSpecialZodiacEffect.type)
	{
		case SE_SKILL_DAMAGE_ZONE:
			effect = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE;
			break;

		case SE_SKILL_DAMAGE_ZONE_BUYUK:
			effect = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_BUYUK;
			break;

		case SE_SKILL_DAMAGE_ZONE_ORTA:
			effect = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_ORTA;
			break;

		case SE_SKILL_DAMAGE_ZONE_KUCUK:
			effect = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_KUCUK;
			break;

		case SE_SKILL_SAFE_ZONE:
			effect = CInstanceBase::EFFECT_SKILL_SAFE_ZONE;
			break;

		case SE_SKILL_SAFE_ZONE_BUYUK:
			effect = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_BUYUK;
			break;

		case SE_SKILL_SAFE_ZONE_ORTA:
			effect = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_ORTA;
			break;

		case SE_SKILL_SAFE_ZONE_KUCUK:
			effect = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_KUCUK;
			break;

		case SE_METEOR:
			effect = CInstanceBase::EFFECT_METEOR;
			break;

		case SE_BEAD_RAIN:
			effect = CInstanceBase::EFFECT_BEAD_RAIN;
			break;

		case SE_FALL_ROCK:
			effect = CInstanceBase::EFFECT_FALL_ROCK;
			break;

		case SE_ARROW_RAIN:
			effect = CInstanceBase::EFFECT_ARROW_RAIN;
			break;

		case SE_HORSE_DROP:
			effect = CInstanceBase::EFFECT_HORSE_DROP;
			break;

		case SE_EGG_DROP:
			effect = CInstanceBase::EFFECT_EGG_DROP;
			break;

		default:
			TraceError("TPacketGCSpecialZodiacEffect.type::Cannot packet:: %d", kSpecialZodiacEffect.type);
			break;
	}

	if (-1 != effect)
	{
		CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(kSpecialZodiacEffect.vid);
		if (pInstance)
		{
			pInstance->AttachSpecialZodiacEffect(effect, kSpecialZodiacEffect.x, kSpecialZodiacEffect.y);
		}
	}

	DWORD effect2 = -1;
	switch (kSpecialZodiacEffect.type2)
	{
		case SE_SKILL_DAMAGE_ZONE:
			effect2 = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE;
			break;

		case SE_SKILL_DAMAGE_ZONE_BUYUK:
			effect2 = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_BUYUK;
			break;

		case SE_SKILL_DAMAGE_ZONE_ORTA:
			effect2 = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_ORTA;
			break;

		case SE_SKILL_DAMAGE_ZONE_KUCUK:
			effect2 = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_KUCUK;
			break;

		case SE_SKILL_SAFE_ZONE:
			effect2 = CInstanceBase::EFFECT_SKILL_SAFE_ZONE;
			break;

		case SE_SKILL_SAFE_ZONE_BUYUK:
			effect2 = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_BUYUK;
			break;

		case SE_SKILL_SAFE_ZONE_ORTA:
			effect2 = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_ORTA;
			break;

		case SE_SKILL_SAFE_ZONE_KUCUK:
			effect2 = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_KUCUK;
			break;

		case SE_METEOR:
			effect2 = CInstanceBase::EFFECT_METEOR;
			break;

		case SE_BEAD_RAIN:
			effect2 = CInstanceBase::EFFECT_BEAD_RAIN;
			break;

		case SE_FALL_ROCK:
			effect2 = CInstanceBase::EFFECT_FALL_ROCK;
			break;

		case SE_ARROW_RAIN:
			effect2 = CInstanceBase::EFFECT_ARROW_RAIN;
			break;

		case SE_HORSE_DROP:
			effect2 = CInstanceBase::EFFECT_HORSE_DROP;
			break;

		case SE_EGG_DROP:
			effect2 = CInstanceBase::EFFECT_EGG_DROP;
			break;
	}

	if (-1 != effect2)
	{
		CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(kSpecialZodiacEffect.vid);
		if (pInstance)
		{
			pInstance->AttachSpecialZodiacEffect(effect2, kSpecialZodiacEffect.x, kSpecialZodiacEffect.y);
		}
	}

	return true;
}
#endif

bool CPythonNetworkStream::RecvSpecificEffect()
{
	TPacketGCSpecificEffect kSpecificEffect;
	if (!Recv(sizeof(kSpecificEffect), &kSpecificEffect))
		return false;

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(kSpecificEffect.vid);
	//EFFECT_TEMP
	if (pInstance)
	{
		CInstanceBase::RegisterEffect(CInstanceBase::EFFECT_TEMP, "", kSpecificEffect.effect_file, false);
		pInstance->AttachSpecialEffect(CInstanceBase::EFFECT_TEMP);
	}

	if (strstr(kSpecificEffect.effect_file, "yellow_tigerman_24_1"))
	{
		CSoundManager::Instance().PlaySound2D("sound/common/spell/spell_archer2r_explode.wav");
	}

	return true;
}

bool CPythonNetworkStream::RecvDragonSoulRefine()
{
	TPacketGCDragonSoulRefine kDragonSoul;

	if (!Recv(sizeof(kDragonSoul), &kDragonSoul))
		return false;
	
	
	switch (kDragonSoul.bSubType)
	{
	case DS_SUB_HEADER_OPEN:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DragonSoulRefineWindow_Open", Py_BuildValue("()"));
		break;
	case DS_SUB_HEADER_REFINE_FAIL:
	case DS_SUB_HEADER_REFINE_FAIL_MAX_REFINE:
	case DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL:
	case DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MONEY:
	case DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL:
	case DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DragonSoulRefineWindow_RefineFail", Py_BuildValue("(iii)", 
			kDragonSoul.bSubType, kDragonSoul.Pos.window_type, kDragonSoul.Pos.cell));
		break;
	case DS_SUB_HEADER_REFINE_SUCCEED:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DragonSoulRefineWindow_RefineSucceed", 
				Py_BuildValue("(ii)", kDragonSoul.Pos.window_type, kDragonSoul.Pos.cell));
		break;
	}

	return true;
}

#ifdef ENABLE_OFFLINESHOP_SYSTEM
bool CPythonNetworkStream::SendOfflineShopEndPacket()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGShop packet_shop;
	packet_shop.header = HEADER_CG_OFFLINE_SHOP;
	packet_shop.subheader = SHOP_SUBHEADER_CG_END;

	if (!Send(sizeof(packet_shop), &packet_shop))
	{
		Tracef("SendOfflineShopEndPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendOfflineShopBuyPacket(DWORD vid, BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_BUY;
	PacketShop.vid = vid;
	PacketShop.pos = bPos;
	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendOfflineShopBuyPacket Error\n");
		return false;
	}
	//TraceError("SendOfflineShopBuyPacket vid=%d sub=%d",vid,bPos);
	return SendSequence();
}

bool CPythonNetworkStream::SendAddOfflineShopItem(TItemPos bDisplayPos, BYTE bPos, long long lPrice, int lPrice_Cheque)
{
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_ADD_ITEM;

	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendAddOfflineShopItem Error\n");
		return false;
	}

	TOfflineShopAddItem pTable;

	pTable.bDisplayPos = bDisplayPos;
	pTable.bPos = bPos;
	pTable.lPrice = lPrice;
	pTable.dwPrice_Cheque = lPrice_Cheque;
	if (!Send(sizeof(TOfflineShopAddItem), &pTable))
	{
		Tracef("SendAddOfflineShopItem Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendRemoveOfflineShopItem(BYTE bPos)
{
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_REMOVE_ITEM;

	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendRemoveOfflineShopItem Error\n");
		return false;
	}

	if (!Send(sizeof(BYTE), &bPos))
	{
		Tracef("SendRemoveOfflineShopItem Packet Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendOpenShopSlot(BYTE bPos)
{
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_OPEN_SLOT;

	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendOpenShopSlot Error\n");
		return false;
	}

	if (!Send(sizeof(BYTE), &bPos))
	{
		Tracef("SendOpenShopSlot Packet Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendGetBackItems()
{
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_GET_ITEM;
	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendOpenShopSlot Error\n");
		return false;
	}
	return SendSequence();
}

bool CPythonNetworkStream::SendAddTime()
{
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_ADD_TIME;
	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendOpenShopSlot Error\n");
		return false;
	}
	return SendSequence();
}

bool CPythonNetworkStream::SendDestroyOfflineShop()
{
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_DESTROY_OFFLINE_SHOP;
	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendDestroyOfflineShop Packet Error\n");
		return false;
	}
	return SendSequence();
}

bool CPythonNetworkStream::SendTakeOfflineMoney()
{
	static DWORD s_LastTime = timeGetTime() - 5001;
	if (timeGetTime() - s_LastTime < 5000)
		return true;
	s_LastTime = timeGetTime();
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_TAKE_MONEY;
	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendRefreshOfflineShopMoney Packet Error\n");
		return false;
	}
	return SendSequence();
}

bool CPythonNetworkStream::SendTakeOfflineMoneyCheque()
{
	static DWORD s_LastTime = timeGetTime() - 5001;
	if (timeGetTime() - s_LastTime < 5000)
		return true;
	s_LastTime = timeGetTime();
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_TAKE_MONEY_CHEQUE;
	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendRefreshOfflineShopMoney Packet Error\n");
		return false;
	}
	return SendSequence();
}

bool CPythonNetworkStream::SendOfflineShopChangeTitle(const char* title)
{
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_CHANGE_TITLE;
	strcpy(PacketShop.title, title);
	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendOfflineShopChangeTitle Packet Error\n");
		return false;
	}
	return SendSequence();
}

bool CPythonNetworkStream::SendOfflineShopCheck()
{
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_CHECK;
	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendOfflineShopCheck Packet error\n");
		return false;
	}
	return SendSequence();
}

bool CPythonNetworkStream::SendOfflineShopButton()
{
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_BUTTON;
	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendOfflineShopCheck Packet error\n");
		return false;
	}
	return SendSequence();
}

bool CPythonNetworkStream::SendOfflineOpenWithVID(DWORD vid)
{
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_OPEN_WITH_VID;
	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendOfflineOpenWithVID Packet error\n");
		return false;
	}
	if (!Send(sizeof(DWORD), &vid))
	{
		Tracef("SendOfflineOpenWithVID Packet Error\n");
		return false;
	}
	return SendSequence();
}

bool CPythonNetworkStream::SendOfflineShopChangeDecoration(const char* sign, DWORD vnum, DWORD type)
{
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_CHANGE_DECORATION;
	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendOfflineShopChangeDecoration Packet error\n");
		return false;
	}

	TShopDecoration p;
	snprintf(p.sign, sizeof(p.sign), sign);
	p.vnum = vnum;
	p.type = type;
	if (!Send(sizeof(TShopDecoration), &p))
	{
		Tracef("SendOfflineShopChangeDecoration Packet Error\n");
		return false;
	}

	return SendSequence();
}
#endif

#ifdef ENABLE_MAILBOX_SYSTEM
bool CPythonNetworkStream::RecvMailBox()
{
	TPacketGCMailBoxReceive KMailBox;

	if (!Recv(sizeof(KMailBox), &KMailBox))
		return false;

	switch (KMailBox.bSubType)
	{
		case MAILBOX_SUB_HEADER_OPEN:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_MAILBOX_OPEN", Py_BuildValue("()"));
			break;

		case MAILBOX_SUB_HEADER_RECEIVE_INFO:
#ifdef ENABLE_CHANGELOOK_SYSTEM
#ifdef ENABLE_CHEQUE_SYSTEM
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_MAILBOX_RECEIVE_INFO", Py_BuildValue("(isssiiiiiiiiii)",
						KMailBox.id_mail , KMailBox.nombre_pj,  KMailBox.asunto,  KMailBox.descrip,  KMailBox.item_vnum,  KMailBox.item_count, KMailBox.transmutation, KMailBox.dwRefineElement, KMailBox.set_value,  KMailBox.gold, KMailBox.won, KMailBox.time, KMailBox.check_acept, KMailBox.check_view
					));
#else
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_MAILBOX_RECEIVE_INFO", Py_BuildValue("(isssiiiiiii)",
						KMailBox.id_mail , KMailBox.nombre_pj,  KMailBox.asunto,  KMailBox.descrip,  KMailBox.item_vnum,  KMailBox.item_count, KMailBox.transmutation,  KMailBox.gold, KMailBox.time, KMailBox.check_acept, KMailBox.check_view
					));
#endif
#else
#ifdef ENABLE_CHEQUE_SYSTEM
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_MAILBOX_RECEIVE_INFO", Py_BuildValue("(isssiiiiiii)",
						KMailBox.id_mail , KMailBox.nombre_pj,  KMailBox.asunto,  KMailBox.descrip,  KMailBox.item_vnum,  KMailBox.item_count, KMailBox.gold, KMailBox.won, KMailBox.time, KMailBox.check_acept, KMailBox.check_view
					));
#else
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_MAILBOX_RECEIVE_INFO", Py_BuildValue("(isssiiiiii)",
						KMailBox.id_mail , KMailBox.nombre_pj,  KMailBox.asunto,  KMailBox.descrip,  KMailBox.item_vnum,  KMailBox.item_count, KMailBox.gold, KMailBox.time, KMailBox.check_acept, KMailBox.check_view
					));

#endif
#endif

			if (KMailBox.item_vnum != 0){
				for (int j = 0; j < ITEM_SOCKET_SLOT_MAX_NUM; ++j)
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_MAILBOX_RECEIVE_ITEM_SOCKET", Py_BuildValue("(iii)", KMailBox.id_mail, j, KMailBox.alSockets[j]));

				for (int k = 0; k < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++k)
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_MAILBOX_RECEIVE_ITEM_ATTR", Py_BuildValue("(iiii)",KMailBox.id_mail,k,KMailBox.aAttr[k].bType,KMailBox.aAttr[k].sValue));

				for (int k = 0; k < ITEM_APPLY_RANDOM_SLOT_MAX_NUM; ++k)
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_MAILBOX_RECEIVE_ITEM_RANDOM_BONUS", Py_BuildValue("(iiii)",KMailBox.id_mail,k,KMailBox.aApplyRandom[k].bType,KMailBox.aApplyRandom[k].sValue));

			}
			break;

		case MAILBOX_SUB_HEADER_CHECK_NAME_SUCCESSFUL:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_MAILBOX_CHECK_NAME_SUCCESSFUL", Py_BuildValue("()"));
			break;

		case MAILBOX_SUB_HEADER_SEND_MAIL_SUCCESSFUL:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_MAILBOX_SEND_MAIL_SUCCESSFUL", Py_BuildValue("()"));
			break;

		case MAILBOX_SUB_HEADER_CHECK_ACCEPT_SUCCESSFUL:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_MAILBOX_ACCEPT_SUCCESSFUL", Py_BuildValue("(i)",KMailBox.id_mail));
			break;

		case MAILBOX_SUB_HEADER_CHECK_DELETE_SUCCESSFUL:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_MAILBOX_DELETE_SUCCESSFUL", Py_BuildValue("(i)",KMailBox.id_mail));
			break;

		case MAILBOX_SUB_HEADER_LOADING_ACCEPT:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_MAILBOX_SEND_LOADING_ACCEPT", Py_BuildValue("()"));
			break;

		case MAILBOX_SUB_HEADER_LOADING_DELETE:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_MAILBOX_SEND_LOADING_DELETE", Py_BuildValue("()"));
			break;

		case MAILBOX_SUB_HEADER_CHECK_VIEW_SUCCESSFUL:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_MAILBOX_CHECK_VIEW_SUCCESSFULL", Py_BuildValue("(i)",KMailBox.id_mail));
			break;
	}

	return true;
}
#endif

#ifdef ENABLE_DS_SET
bool CPythonNetworkStream::RecvDSTablePacket() {
	TPacketDSTable p;
	if (!Recv(sizeof(TPacketDSTable), &p))
		return false;
	
	return CPythonItem::Instance().SetDSTable(p);
}
#endif