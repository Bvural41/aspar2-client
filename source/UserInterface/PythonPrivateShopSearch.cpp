#include "StdAfx.h"
#ifdef ENABLE_SHOP_SEARCH_SYSTEM
#include "PythonPrivateShopSearch.h"
#include "Packet.h"
#ifdef PYTHON_DYNAMIC_MODULE_NAME
#include "PythonDynamicModuleNames.h"
#endif

CPythonPrivateShopSearch::CPythonPrivateShopSearch()
    : m_sortType(0)
{
}

CPythonPrivateShopSearch::~CPythonPrivateShopSearch() {}

void CPythonPrivateShopSearch::AddItemData(TOfflineShopItemData& rItemData)
{
	m_ItemInstanceVector.push_back(rItemData);
}

void CPythonPrivateShopSearch::ClearItemData()
{
	m_ItemInstanceVector.clear();
}

void CPythonPrivateShopSearch::SetSortType(int sortType)
{
	m_sortType = sortType;
}

DWORD CPythonPrivateShopSearch::GetItemDataPtr(DWORD index, TOfflineShopItemData** ppInstance)
{
	*ppInstance = &m_ItemInstanceVector.at(index);
	return 1;
}

PyObject* privateShopSearchGetSearchItemMetinSocket(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BadArgument();
	int iSocketIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSocketIndex))
		return Py_BadArgument();

	if (iSocketIndex >= ITEM_SOCKET_SLOT_MAX_NUM)
		return Py_BuildException();
	TOfflineShopItemData* pItemData;
	if (!CPythonPrivateShopSearch::Instance().GetItemDataPtr(iSlotIndex, &pItemData))
	{
		return Py_BuildException();
	}

	return Py_BuildValue("i", pItemData->alSockets[iSocketIndex]);
}

PyObject* privateShopSearchGetSearchItemAttribute(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
	{
		return Py_BuildException();
	}
	int iAttrSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iAttrSlotIndex))
	{
		return Py_BuildException();
	}

	if (iAttrSlotIndex >= 0 && iAttrSlotIndex < ITEM_ATTRIBUTE_SLOT_MAX_NUM)
	{
		TOfflineShopItemData* pItemData;
		if (CPythonPrivateShopSearch::Instance().GetItemDataPtr(iSlotIndex, &pItemData))
		{
			return Py_BuildValue("ii", pItemData->aAttr[iAttrSlotIndex].bType, pItemData->aAttr[iAttrSlotIndex].sValue);
		}
	}

	return Py_BuildValue("ii", 0, 0);
}

PyObject* privateShopSearchGetItemCount(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPrivateShopSearch::Instance().GetItemDataCount());
}

PyObject* privateShopSearchGetSearchItemCount(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
	{
		return Py_BadArgument();
	}

	TOfflineShopItemData* pInstance;
	if (!CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
	{
		return Py_BuildException();
	}

	return Py_BuildValue("i", pInstance->count);
}

PyObject* privateShopSearchGetSearchItemPrice(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
	{
		return Py_BadArgument();
	}

	TOfflineShopItemData* pInstance;
	if (!CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
	{
		return Py_BuildException();
	}

	return PyLong_FromLongLong(pInstance->price);
}

PyObject* privateShopSearchGetSearchItemPriceCheque(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
	{
		return Py_BadArgument();
	}

	TOfflineShopItemData* pInstance;
	if (!CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
	{
		return Py_BuildException();
	}

	return Py_BuildValue("i", pInstance->price_cheque);
}

PyObject* privateShopSearchGetSearchItemShopVID(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
	{
		return Py_BadArgument();
	}

	TOfflineShopItemData* pInstance;
	if (!CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
	{
		return Py_BuildException();
	}

	return Py_BuildValue("i", pInstance->owner_id);
}

PyObject* privateShopSearchGetSearchItemPos(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
	{
		return Py_BadArgument();
	}

	TOfflineShopItemData* pInstance;
	if (!CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
	{
		return Py_BuildException();
	}

	return Py_BuildValue("i", pInstance->display_pos);
}

PyObject* privateShopSearchGetSearchItemVnum(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
	{
		return Py_BadArgument();
	}

	TOfflineShopItemData* pInstance;
	if (!CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
	{
		return Py_BuildException();
	}

	return Py_BuildValue("i", pInstance->vnum);
}

PyObject* privateShopSearchGetSearchItemID(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();
	TOfflineShopItemData* pInstance;
	if (!CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildException();
	return Py_BuildValue("i", pInstance->id);
}

PyObject* privateShopSearchGetSearchItemBuyerName(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
	{
		return Py_BadArgument();
	}

	TOfflineShopItemData* pInstance;
	if (!CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
	{
		return Py_BuildException();
	}

	return Py_BuildValue("s", pInstance->szBuyerName);
}

PyObject* privateShopSearchClearSearchItems(PyObject* poSelf, PyObject* poArgs)
{
	CPythonPrivateShopSearch::Instance().ClearItemData();
	return Py_BuildNone();
}

#ifdef ENABLE_CHANGELOOK_SYSTEM
PyObject* privateShopSearchGetSearchItemTransmutation(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
	{
		return Py_BadArgument();
	}

	TOfflineShopItemData* pInstance;
	if (CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildValue("i", pInstance->transmutation);

	return Py_BuildValue("i",0);
}
#endif

#ifdef ENABLE_REFINE_ELEMENT
PyObject* privateShopSearchGetSearchItemRefineElement(PyObject* poSelf, PyObject* poArgs)
{

	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
	{
		return Py_BadArgument();
	}

	TOfflineShopItemData* pInstance;
	if (CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildValue("i", pInstance->dwRefineElement);

	return Py_BuildValue("i",0);
}
#endif

#ifdef ENABLE_SET_ITEM
PyObject* privateShopSearchGetSearchItemSetValue(PyObject* poSelf, PyObject* poArgs)
{

	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
	{
		return Py_BadArgument();
	}

	TOfflineShopItemData* pInstance;
	if (CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildValue("i", pInstance->set_value);

	return Py_BuildValue("i",0);
}
#endif

#ifdef ENABLE_GLOVE_SYSTEM
PyObject* privateShopSearchGetSearchItemRandomAttribute(PyObject* poSelf, PyObject* poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	int iApplySlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iApplySlotIndex))
		return Py_BuildException();

	if (iApplySlotIndex >= 0 && iApplySlotIndex < ITEM_APPLY_RANDOM_SLOT_MAX_NUM)
	{
		TOfflineShopItemData* c_pItemData;
		if (CPythonPrivateShopSearch::Instance().GetItemDataPtr(iIndex, &c_pItemData))
		{
			return Py_BuildValue("ii", c_pItemData->aApplyRandom[iApplySlotIndex].bType, c_pItemData->aApplyRandom[iApplySlotIndex].sValue);
		}
	}

	return Py_BuildValue("ii", 0, 0);
}
#endif

PyObject* privateShopSearchSetSortType(PyObject* poSelf, PyObject* poArgs)
{
	int sortType;
	if (!PyTuple_GetInteger(poArgs, 0, &sortType))
		return Py_BadArgument();

	CPythonPrivateShopSearch::Instance().SetSortType(sortType);

	Py_RETURN_NONE;
}

void initprivateShopSearch()
{
	static PyMethodDef s_methods[] =
	{
		{ "GetItemCount",					privateShopSearchGetItemCount,						METH_VARARGS },
		{ "GetSearchItemCount",				privateShopSearchGetSearchItemCount,				METH_VARARGS },
		{ "GetSearchItemPrice",				privateShopSearchGetSearchItemPrice,				METH_VARARGS },
		{ "GetSearchItemPriceCheque",		privateShopSearchGetSearchItemPriceCheque,			METH_VARARGS },
		{ "GetSearchItemVnum",				privateShopSearchGetSearchItemVnum,					METH_VARARGS },
		{ "GetSearchItemShopVID",			privateShopSearchGetSearchItemShopVID,				METH_VARARGS },
		{ "ClearSearchItems",				privateShopSearchClearSearchItems,					METH_VARARGS },
		{ "GetSearchItemMetinSocket",		privateShopSearchGetSearchItemMetinSocket,			METH_VARARGS },
		{ "GetSearchItemAttribute",			privateShopSearchGetSearchItemAttribute,			METH_VARARGS },
		{ "GetSearchItemPos",				privateShopSearchGetSearchItemPos,					METH_VARARGS },
		{ "GetSearchItemSeller",			privateShopSearchGetSearchItemBuyerName,			METH_VARARGS },
		{ "GetSearchItemID",				privateShopSearchGetSearchItemID,					METH_VARARGS },
#ifdef ENABLE_CHANGELOOK_SYSTEM
		{ "GetSearchItemTransmutation",		privateShopSearchGetSearchItemTransmutation,		METH_VARARGS },
#endif
#ifdef ENABLE_REFINE_ELEMENT
		{ "GetSearchItemRefineElement",		privateShopSearchGetSearchItemRefineElement,		METH_VARARGS },
#endif
#ifdef ENABLE_SET_ITEM
		{ "GetSearchItemItemSetValue",		privateShopSearchGetSearchItemSetValue,				METH_VARARGS },
#endif
#ifdef ENABLE_GLOVE_SYSTEM
		{ "GetSearchItemRandomAttribute",	privateShopSearchGetSearchItemRandomAttribute,		METH_VARARGS },
#endif
		{ "SetSortType",					privateShopSearchSetSortType,						METH_VARARGS },

		{ NULL,								NULL,												NULL },
	};

#ifdef PYTHON_DYNAMIC_MODULE_NAME
	PyObject* poModule = Py_InitModule(GetModuleName(SHOP_SEARCH_MODULE).c_str(), s_methods);
#else
	PyObject* poModule = Py_InitModule("privateShopSearch", s_methods);
#endif
}
#endif

