#include "StdAfx.h"
#include "PythonSafeBox.h"

#include "PythonNetworkStream.h"

void CPythonSafeBox::OpenSafeBox(int iSize)
{
	m_dwMoney = 0;
	m_ItemInstanceVector.clear();
	m_isOpenSafebox = true;
	m_ItemInstanceVector.resize(SAFEBOX_SLOT_X_COUNT * iSize);

	for (DWORD i = 0; i < m_ItemInstanceVector.size(); ++i)
	{
		TItemData & rInstance = m_ItemInstanceVector[i];
		ZeroMemory(&rInstance, sizeof(rInstance));
	}
}

void CPythonSafeBox::SetItemData(DWORD dwSlotIndex, const TItemData & rItemInstance)
{
	if (dwSlotIndex >= m_ItemInstanceVector.size())
	{
		TraceError("CPythonSafeBox::SetItemData(dwSlotIndex=%d) - Strange slot index", dwSlotIndex);
		return;
	}

	m_ItemInstanceVector[dwSlotIndex] = rItemInstance;
}

void CPythonSafeBox::DelItemData(DWORD dwSlotIndex)
{
	if (dwSlotIndex >= m_ItemInstanceVector.size())
	{
		TraceError("CPythonSafeBox::DelItemData(dwSlotIndex=%d) - Strange slot index", dwSlotIndex);
		return;
	}

	TItemData & rInstance = m_ItemInstanceVector[dwSlotIndex];
	ZeroMemory(&rInstance, sizeof(rInstance));
}

void CPythonSafeBox::SetMoney(DWORD dwMoney)
{
	m_dwMoney = dwMoney;
}

DWORD CPythonSafeBox::GetMoney()
{
	return m_dwMoney;
}

int CPythonSafeBox::GetCurrentSafeBoxSize()
{
	return m_ItemInstanceVector.size();
}

BOOL CPythonSafeBox::GetSlotItemID(DWORD dwSlotIndex, DWORD* pdwItemID)
{
	if (dwSlotIndex >= m_ItemInstanceVector.size())
	{
		TraceError("CPythonSafeBox::GetSlotItemID(dwSlotIndex=%d) - Strange slot index", dwSlotIndex);
		return FALSE;
	}

	*pdwItemID=m_ItemInstanceVector[dwSlotIndex].vnum;

	return TRUE;
}

BOOL CPythonSafeBox::GetItemDataPtr(DWORD dwSlotIndex, TItemData ** ppInstance)
{
	if (dwSlotIndex >= m_ItemInstanceVector.size())
	{
		TraceError("CPythonSafeBox::GetItemData(dwSlotIndex=%d) - Strange slot index", dwSlotIndex);
		return FALSE;
	}

	*ppInstance = &m_ItemInstanceVector[dwSlotIndex];

	return TRUE;
}

void CPythonSafeBox::OpenMall(int iSize)
{
	m_MallItemInstanceVector.clear();
	m_MallItemInstanceVector.resize(SAFEBOX_SLOT_X_COUNT * iSize);

	for (DWORD i = 0; i < m_MallItemInstanceVector.size(); ++i)
	{
		TItemData & rInstance = m_MallItemInstanceVector[i];
		ZeroMemory(&rInstance, sizeof(rInstance));
	}
}

void CPythonSafeBox::SetMallItemData(DWORD dwSlotIndex, const TItemData & rItemData)
{
	if (dwSlotIndex >= m_MallItemInstanceVector.size())
	{
		TraceError("CPythonSafeBox::SetMallItemData(dwSlotIndex=%d) - Strange slot index", dwSlotIndex);
		return;
	}

	m_MallItemInstanceVector[dwSlotIndex] = rItemData;
}

void CPythonSafeBox::DelMallItemData(DWORD dwSlotIndex)
{
	if (dwSlotIndex >= m_MallItemInstanceVector.size())
	{
		TraceError("CPythonSafeBox::DelMallItemData(dwSlotIndex=%d) - Strange slot index", dwSlotIndex);
		return;
	}

	TItemData & rInstance = m_MallItemInstanceVector[dwSlotIndex];
	ZeroMemory(&rInstance, sizeof(rInstance));
}

BOOL CPythonSafeBox::GetMallItemDataPtr(DWORD dwSlotIndex, TItemData ** ppInstance)
{
	if (dwSlotIndex >= m_MallItemInstanceVector.size())
	{
		TraceError("CPythonSafeBox::GetMallSlotItemID(dwSlotIndex=%d) - Strange slot index", dwSlotIndex);
		return FALSE;
	}

	*ppInstance = &m_MallItemInstanceVector[dwSlotIndex];

	return TRUE;
}

BOOL CPythonSafeBox::GetSlotMallItemID(DWORD dwSlotIndex, DWORD * pdwItemID)
{
	if (dwSlotIndex >= m_MallItemInstanceVector.size())
	{
		TraceError("CPythonSafeBox::GetMallSlotItemID(dwSlotIndex=%d) - Strange slot index", dwSlotIndex);
		return FALSE;
	}

	*pdwItemID = m_MallItemInstanceVector[dwSlotIndex].vnum;

	return TRUE;
}

DWORD CPythonSafeBox::GetMallSize()
{
	return m_MallItemInstanceVector.size();
}

bool CPythonSafeBox::isOpen()
{
	return m_isOpenSafebox;
}

void CPythonSafeBox::Close()
{
	m_isOpenSafebox = false;
}

CPythonSafeBox::CPythonSafeBox()
{
	m_dwMoney = 0;
	m_isOpenSafebox = false;
}

CPythonSafeBox::~CPythonSafeBox()
{
}

PyObject * safeboxGetCurrentSafeboxSize(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSafeBox::Instance().GetCurrentSafeBoxSize());
}

PyObject * safeboxGetItemID(PyObject * poSelf, PyObject * poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	TItemData * pInstance;
	if (!CPythonSafeBox::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pInstance->vnum);
}

PyObject * safeboxGetItemCount(PyObject * poSelf, PyObject * poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	TItemData * pInstance;
	if (!CPythonSafeBox::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pInstance->count);
}

#ifdef ENABLE_SOULBIND_SYSTEM
PyObject * safeboxGetItemBind(PyObject * poSelf, PyObject * poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();
	
	TItemData * pInstance;
	if (!CPythonSafeBox::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildException();
	
	return Py_BuildValue("i", pInstance->bind);
}

PyObject * safeboxGetMallItemBind(PyObject * poSelf, PyObject * poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();
	
	TItemData * pInstance;
	if (!CPythonSafeBox::Instance().GetMallItemDataPtr(ipos, &pInstance))
		return Py_BuildException();
	
	return Py_BuildValue("i", pInstance->bind);
}
#endif

PyObject * safeboxGetItemFlags(PyObject * poSelf, PyObject * poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	TItemData * pInstance;
	if (!CPythonSafeBox::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pInstance->flags);
}

PyObject * safeboxGetItemMetinSocket(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BadArgument();
	int iSocketIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSocketIndex))
		return Py_BadArgument();

	if (iSocketIndex >= ITEM_SOCKET_SLOT_MAX_NUM)
		return Py_BuildException();

	TItemData * pItemData;
	if (!CPythonSafeBox::Instance().GetItemDataPtr(iSlotIndex, &pItemData))
		return Py_BuildException();

	return Py_BuildValue("i", pItemData->alSockets[iSocketIndex]);
}

PyObject * safeboxGetItemAttribute(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();
	int iAttrSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iAttrSlotIndex))
		return Py_BuildException();

	if (iAttrSlotIndex >= 0 && iAttrSlotIndex < ITEM_ATTRIBUTE_SLOT_MAX_NUM)
	{
		TItemData * pItemData;
		if (CPythonSafeBox::Instance().GetItemDataPtr(iSlotIndex, &pItemData))
			return Py_BuildValue("ii", pItemData->aAttr[iAttrSlotIndex].bType, pItemData->aAttr[iAttrSlotIndex].sValue);
	}

	return Py_BuildValue("ii", 0, 0);
}

#ifdef ENABLE_GLOVE_SYSTEM
PyObject* safeboxGetItemApplyRandom(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	int iApplySlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iApplySlotIndex))
		return Py_BuildException();

	if (iApplySlotIndex >= 0 && iApplySlotIndex < ITEM_APPLY_RANDOM_SLOT_MAX_NUM)
	{
		TItemData* pItemData;
		if (CPythonSafeBox::Instance().GetItemDataPtr(iSlotIndex, &pItemData))
			return Py_BuildValue("ii", pItemData->aApplyRandom[iApplySlotIndex].bType, pItemData->aApplyRandom[iApplySlotIndex].sValue);
	}

	return Py_BuildValue("ii", 0, 0);
}
#endif

PyObject * safeboxGetMoney(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSafeBox::Instance().GetMoney());
}

PyObject * safeboxGetMallItemID(PyObject * poSelf, PyObject * poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	TItemData * pInstance;
	if (!CPythonSafeBox::Instance().GetMallItemDataPtr(ipos, &pInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pInstance->vnum);
}

PyObject * safeboxGetMallItemCount(PyObject * poSelf, PyObject * poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
		return Py_BadArgument();

	TItemData * pInstance;
	if (!CPythonSafeBox::Instance().GetMallItemDataPtr(ipos, &pInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pInstance->count);
}

PyObject * safeboxGetMallItemMetinSocket(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BadArgument();
	int iSocketIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSocketIndex))
		return Py_BadArgument();

	if (iSocketIndex >= ITEM_SOCKET_SLOT_MAX_NUM)
		return Py_BuildException();

	TItemData * pItemData;
	if (!CPythonSafeBox::Instance().GetMallItemDataPtr(iSlotIndex, &pItemData))
		return Py_BuildException();

	return Py_BuildValue("i", pItemData->alSockets[iSocketIndex]);
}

PyObject * safeboxGetMallItemAttribute(PyObject * poSelf, PyObject * poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();
	int iAttrSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iAttrSlotIndex))
		return Py_BuildException();

	if (iAttrSlotIndex >= 0 && iAttrSlotIndex < ITEM_ATTRIBUTE_SLOT_MAX_NUM)
	{
		TItemData * pItemData;
		if (CPythonSafeBox::Instance().GetMallItemDataPtr(iSlotIndex, &pItemData))
			return Py_BuildValue("ii", pItemData->aAttr[iAttrSlotIndex].bType, pItemData->aAttr[iAttrSlotIndex].sValue);
	}

	return Py_BuildValue("ii", 0, 0);
}

#ifdef ENABLE_GLOVE_SYSTEM
PyObject* safeboxGetMallItemApplyRandom(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

	int iApplySlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iApplySlotIndex))
		return Py_BuildException();

	if (iApplySlotIndex >= 0 && iApplySlotIndex < ITEM_APPLY_RANDOM_SLOT_MAX_NUM)
	{
		TItemData* pItemData;
		if (CPythonSafeBox::Instance().GetMallItemDataPtr(iSlotIndex, &pItemData))
			return Py_BuildValue("ii", pItemData->aApplyRandom[iApplySlotIndex].bType, pItemData->aApplyRandom[iApplySlotIndex].sValue);
	}

	return Py_BuildValue("ii", 0, 0);
}
#endif

PyObject * safeboxGetMallSize(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSafeBox::Instance().GetMallSize());
}

PyObject * safeboxIsOpen(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSafeBox::Instance().isOpen());
}

PyObject * safeboxClose(PyObject * poSelf, PyObject * poArgs)
{
	CPythonSafeBox::Instance().Close();
	return Py_BuildNone();
}

PyObject * safeboxRefresh(PyObject * poSelf, PyObject * poArgs)
{
	CPythonNetworkStream::Instance().__RefreshSafeboxWindow();
	return Py_BuildNone();
}

#ifdef ENABLE_CHANGELOOK_SYSTEM
PyObject * safeboxGetItemTransmutation(PyObject * poSelf, PyObject * poArgs)
{
	int iPos;
	if (!PyTuple_GetInteger(poArgs, 0, &iPos))
		return Py_BadArgument();
	
	TItemData * pInstance;
	if (!CPythonSafeBox::Instance().GetItemDataPtr(iPos, &pInstance))
		return Py_BuildException();
	
	return Py_BuildValue("i", pInstance->transmutation);
}

PyObject * safeboxGetMallItemTransmutation(PyObject * poSelf, PyObject * poArgs)
{
	int iPos;
	if (!PyTuple_GetInteger(poArgs, 0, &iPos))
		return Py_BadArgument();
	
	TItemData * pInstance;
	if (!CPythonSafeBox::Instance().GetMallItemDataPtr(iPos, &pInstance))
		return Py_BuildException();
	
	return Py_BuildValue("i", pInstance->transmutation);
}
#endif

#ifdef ENABLE_REFINE_ELEMENT
PyObject * safeboxGetItemRefineElement(PyObject * poSelf, PyObject * poArgs)
{
	int iPos;
	if (!PyTuple_GetInteger(poArgs, 0, &iPos))
		return Py_BadArgument();
	
	TItemData * pInstance;
	if (!CPythonSafeBox::Instance().GetItemDataPtr(iPos, &pInstance))
		return Py_BuildException();
	
	return Py_BuildValue("i", pInstance->dwRefineElement);
}

PyObject * safeboxGetMallItemRefineElement(PyObject * poSelf, PyObject * poArgs)
{
	int iPos;
	if (!PyTuple_GetInteger(poArgs, 0, &iPos))
		return Py_BadArgument();
	
	TItemData * pInstance;
	if (!CPythonSafeBox::Instance().GetMallItemDataPtr(iPos, &pInstance))
		return Py_BuildException();
	
	return Py_BuildValue("i", pInstance->dwRefineElement);
}
#endif

#ifdef ENABLE_SET_ITEM
PyObject* safeboxGetItemSetValue(PyObject* poSelf, PyObject* poArgs)
{
	int iPos;
	if (!PyTuple_GetInteger(poArgs, 0, &iPos))
		return Py_BadArgument();

	TItemData* pInstance;
	if (!CPythonSafeBox::Instance().GetItemDataPtr(iPos, &pInstance))
		return Py_BuildException();

	return Py_BuildValue("i", pInstance->set_value);
}

PyObject* safeboxGetMallItemSetValue(PyObject* poSelf, PyObject* poArgs)
{
	int iPos;
	if (!PyTuple_GetInteger(poArgs, 0, &iPos))
		return Py_BadArgument();
	TItemData* pInstance;
	if (!CPythonSafeBox::Instance().GetMallItemDataPtr(iPos, &pInstance))
		return Py_BuildException();
	return Py_BuildValue("i", pInstance->set_value);
}
#endif

void initsafebox()
{
	static PyMethodDef s_methods[] =
	{
		// SafeBox
		{ "GetCurrentSafeboxSize",		safeboxGetCurrentSafeboxSize,			METH_VARARGS },
		{ "GetItemID",					safeboxGetItemID,						METH_VARARGS },
		{ "GetItemCount",				safeboxGetItemCount,					METH_VARARGS },
		{ "GetItemFlags",				safeboxGetItemFlags,					METH_VARARGS },
		{ "GetItemMetinSocket",			safeboxGetItemMetinSocket,				METH_VARARGS },
		{ "GetItemAttribute",			safeboxGetItemAttribute,				METH_VARARGS },
		{ "GetMoney",					safeboxGetMoney,						METH_VARARGS },
		{ "isOpen",						safeboxIsOpen,							METH_VARARGS },
		{ "Close",						safeboxClose,							METH_VARARGS },

		// Mall
		{ "GetMallItemID",				safeboxGetMallItemID,					METH_VARARGS },
		{ "GetMallItemCount",			safeboxGetMallItemCount,				METH_VARARGS },
		{ "GetMallItemMetinSocket",		safeboxGetMallItemMetinSocket,			METH_VARARGS },
		{ "GetMallItemAttribute",		safeboxGetMallItemAttribute,			METH_VARARGS },
#ifdef ENABLE_GLOVE_SYSTEM
		{ "GetItemApplyRandom",			safeboxGetItemApplyRandom,				METH_VARARGS },
		{ "GetMallItemApplyRandom",		safeboxGetMallItemApplyRandom,			METH_VARARGS },
#endif
		{ "GetMallSize",				safeboxGetMallSize,						METH_VARARGS },
#ifdef ENABLE_SOULBIND_SYSTEM
		{ "GetItemBind",				safeboxGetItemBind,						METH_VARARGS },
		{ "GetMallItemBind",			safeboxGetMallItemBind,					METH_VARARGS },
#endif
		{ "safeboxRefreshPy",			safeboxRefresh,							METH_VARARGS },
#ifdef ENABLE_REFINE_ELEMENT
		{ "GetItemRefineElement", 		safeboxGetItemRefineElement, 			METH_VARARGS},
		{ "GetMallItemRefineElement", 	safeboxGetMallItemRefineElement, 		METH_VARARGS},
#endif
#ifdef ENABLE_CHANGELOOK_SYSTEM
		{"GetItemTransmutation",		safeboxGetItemTransmutation,			METH_VARARGS},
		{"GetMallItemTransmutation",	safeboxGetMallItemTransmutation,		METH_VARARGS},
#endif
#ifdef ENABLE_SET_ITEM
		{"GetItemSetValue",				safeboxGetItemSetValue,					METH_VARARGS},
		{"GetMallItemSetValue",			safeboxGetMallItemSetValue,				METH_VARARGS},
#endif
		{ NULL,							NULL,									NULL },
	};

	PyObject * poModule = Py_InitModule("safebox", s_methods);
	PyModule_AddIntConstant(poModule, "SAFEBOX_SLOT_X_COUNT", CPythonSafeBox::SAFEBOX_SLOT_X_COUNT);
	PyModule_AddIntConstant(poModule, "SAFEBOX_SLOT_Y_COUNT", CPythonSafeBox::SAFEBOX_SLOT_Y_COUNT);
	PyModule_AddIntConstant(poModule, "SAFEBOX_PAGE_SIZE", CPythonSafeBox::SAFEBOX_PAGE_SIZE);
}
