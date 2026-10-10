#include "StdAfx.h"
#include "../EterBase/CRC32.h"
#include "PythonWindow.h"
#include "PythonSlotWindow.h"
#include "PythonWindowManager.h"
#include "../EterLib/CRenderTargetManager.h"
#ifdef ENABLE_INGAME_WIKI
#include "../EterLib/CWikiRenderTargetManager.h"
#endif
#include "../UserInterface/Locale_inc.h"

BOOL g_bOutlineBoxEnable = FALSE;

namespace UI
{

	CWindow::CWindow(PyObject* ppyObject) :
		m_x(0),
		m_y(0),
		m_lWidth(0),
		m_lHeight(0),
		m_poHandler(ppyObject),
		m_bShow(false),
		m_pParent(NULL),
		m_dwFlag(0),
		m_isUpdatingChildren(FALSE)
#ifdef ENABLE_MOUSEWHEEL_EVENT
		,m_bIsScrollable(false)
#endif
#ifdef ENABLE_INGAME_WIKI
		,m_isInsideRender(false)
#endif
	{
#ifdef _DEBUG
		static DWORD DEBUG_dwGlobalCounter = 0;
		DEBUG_dwCounter = DEBUG_dwGlobalCounter++;

		m_strName = "!!debug";
#endif
		//assert(m_poHandler != NULL);
		m_HorizontalAlign = HORIZONTAL_ALIGN_LEFT;
		m_VerticalAlign = VERTICAL_ALIGN_TOP;
		m_rect.bottom = m_rect.left = m_rect.right = m_rect.top = 0;
		m_limitBiasRect.bottom = m_limitBiasRect.left = m_limitBiasRect.right = m_limitBiasRect.top = 0;
#ifdef ENABLE_USE_CLIP_MASK
		m_bEnableMask = false;
		m_pMaskWindow = nullptr;
		memset(&m_rMaskRect, 0, sizeof(m_rMaskRect));
#endif
#ifdef ENABLE_INGAME_WIKI
		memset(&m_renderBox, 0, sizeof(m_renderBox));
		memset(&m_renderingRect, 0, sizeof(m_renderingRect));
#endif
	}

	CWindow::~CWindow()
	{
	}

	DWORD CWindow::Type()
	{
		static DWORD s_dwType = GetCRC32("CWindow", strlen("CWindow"));
		return (s_dwType);
	}

	BOOL CWindow::IsType(DWORD dwType)
	{
		return OnIsType(dwType);
	}

	BOOL CWindow::OnIsType(DWORD dwType)
	{
		if (CWindow::Type() == dwType)
			return TRUE;

		return FALSE;
	}

	struct FClear
	{
		void operator () (CWindow* pWin)
		{
			pWin->Clear();
		}
	};

	void CWindow::Clear()
	{
#ifdef ENABLE_USE_CLIP_MASK
		m_pMaskWindow = nullptr;
#endif
		// FIXME : Children을 즉시 Delete하지는 않는다.
		//         어차피 Python쪽에서 Destroy가 하나씩 다시 호출 될 것이므로..
		//         하지만 만약을 위해 링크는 끊어 놓는다.
		//         더 좋은 형태는 있는가? - [levites]
		std::for_each(m_pChildList.begin(), m_pChildList.end(), FClear());
		m_pChildList.clear();

		m_pParent = NULL;
		DestroyHandle();
		Hide();
	}

	void CWindow::DestroyHandle()
	{
		m_poHandler = NULL;
	}

	void CWindow::Show()
	{
		m_bShow = true;
	}

	void CWindow::Hide()
	{
		m_bShow = false;
	}

	// NOTE : IsShow는 "자신이 보이는가?" 이지만, __IsShowing은 "자신이 그려지고 있는가?" 를 체크한다
	//        자신은 Show 지만 Tree 위쪽의 Parent 중 하나는 Hide 일 수 있으므로.. - [levites]
	bool CWindow::IsRendering()
	{
		if (!IsShow())
			return false;

		if (!m_pParent)
			return true;

		return m_pParent->IsRendering();
	}

	void CWindow::__RemoveReserveChildren()
	{
		if (m_pReserveChildList.empty())
			return;

		TWindowContainer::iterator it;
		for (it = m_pReserveChildList.begin(); it != m_pReserveChildList.end(); ++it)
		{
			m_pChildList.remove(*it);
		}
		m_pReserveChildList.clear();
	}

	void CWindow::Update()
	{
		if (!IsShow())
			return;

		__RemoveReserveChildren();

		OnUpdate();

		m_isUpdatingChildren = TRUE;
		TWindowContainer::iterator it;
		for (it = m_pChildList.begin(); it != m_pChildList.end();)
		{
			TWindowContainer::iterator it_next = it;
			++it_next;
			(*it)->Update();
			it = it_next;
		}
		m_isUpdatingChildren = FALSE;
	}

	void CWindow::Render()
	{
		if (!IsShow())
			return;

		OnRender();

		if (g_bOutlineBoxEnable)
		{
			CPythonGraphic::Instance().SetDiffuseColor(1.0f, 1.0f, 1.0f);
			CPythonGraphic::Instance().RenderBox2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
		}

		std::for_each(m_pChildList.begin(), m_pChildList.end(), std::void_mem_fun(&CWindow::Render));

	}

	void CWindow::OnUpdate()
	{
		if (!m_poHandler)
			return;

		if (!IsShow())
			return;

		static PyObject* poFuncName_OnUpdate = PyString_InternFromString("OnUpdate");

		//PyCallClassMemberFunc(m_poHandler, "OnUpdate", BuildEmptyTuple());
		PyCallClassMemberFunc_ByPyString(m_poHandler, poFuncName_OnUpdate, BuildEmptyTuple());

	}

	void CWindow::OnRender()
	{
		if (!m_poHandler)
			return;

		if (!IsShow())
			return;

		//PyCallClassMemberFunc(m_poHandler, "OnRender", BuildEmptyTuple());
		PyCallClassMemberFunc(m_poHandler, "OnRender", BuildEmptyTuple());
	}

	void CWindow::SetName(const char* c_szName)
	{
		m_strName = c_szName;
	}

	void CWindow::SetSize(long width, long height)
	{
		m_lWidth = width;
		m_lHeight = height;

		m_rect.right = m_rect.left + m_lWidth;
		m_rect.bottom = m_rect.top + m_lHeight;
	}

	void CWindow::SetHorizontalAlign(DWORD dwAlign)
	{
		m_HorizontalAlign = (EHorizontalAlign)dwAlign;
		UpdateRect();
	}

	void CWindow::SetVerticalAlign(DWORD dwAlign)
	{
		m_VerticalAlign = (EVerticalAlign)dwAlign;
		UpdateRect();
	}

	void CWindow::SetPosition(long x, long y)
	{
		m_x = x;
		m_y = y;

		UpdateRect();
	}

	void CWindow::GetPosition(long* plx, long* ply)
	{
		*plx = m_x;
		*ply = m_y;
	}

	long CWindow::UpdateRect()
	{
		m_rect.top = m_y;
		if (m_pParent)
		{
			switch (m_VerticalAlign)
			{
			case VERTICAL_ALIGN_BOTTOM:
				m_rect.top = m_pParent->GetHeight() - m_rect.top;
				break;
			case VERTICAL_ALIGN_CENTER:
				m_rect.top = (m_pParent->GetHeight() - GetHeight()) / 2 + m_rect.top;
				break;
			}
			m_rect.top += m_pParent->m_rect.top;
		}
		m_rect.bottom = m_rect.top + m_lHeight;

#if defined( _USE_CPP_RTL_FLIP )
		if (m_pParent == NULL) {
			m_rect.left = m_x;
			m_rect.right = m_rect.left + m_lWidth;
		}
		else {
			if (m_pParent->IsFlag(UI::CWindow::FLAG_RTL) == true) {
				m_rect.left = m_pParent->GetWidth() - m_lWidth - m_x;
				switch (m_HorizontalAlign)
				{
				case HORIZONTAL_ALIGN_RIGHT:
					m_rect.left = -m_x;
					break;
				case HORIZONTAL_ALIGN_CENTER:
					m_rect.left = m_pParent->GetWidth() / 2 - GetWidth() - m_x;
					break;
				}
				m_rect.left += m_pParent->m_rect.left;
				m_rect.right = m_rect.left + m_lWidth;
			}
			else {
				m_rect.left = m_x;
				switch (m_HorizontalAlign)
				{
				case HORIZONTAL_ALIGN_RIGHT:
					m_rect.left = m_pParent->GetWidth() - m_rect.left;
					break;
				case HORIZONTAL_ALIGN_CENTER:
					m_rect.left = (m_pParent->GetWidth() - GetWidth()) / 2 + m_rect.left;
					break;
				}
				m_rect.left += m_pParent->m_rect.left;
				m_rect.right = m_rect.left + m_lWidth;
			}
		}
#else
		m_rect.left = m_x;
		if (m_pParent)
		{
			switch (m_HorizontalAlign)
			{
			case HORIZONTAL_ALIGN_RIGHT:
				m_rect.left = ::abs(m_pParent->GetWidth()) - m_rect.left;
				break;
			case HORIZONTAL_ALIGN_CENTER:
				m_rect.left = m_pParent->GetWidth() / 2 - GetWidth() / 2 + m_rect.left;
				break;
			}
			m_rect.left += 0L < m_pParent->GetWidth() ? m_pParent->m_rect.left : m_pParent->m_rect.right + ::abs(m_pParent->GetWidth());
		}
		m_rect.right = m_rect.left + m_lWidth;
#endif
#ifdef ENABLE_INGAME_WIKI
		UpdateRenderBox();
#endif

		for (auto * pChild : m_pChildList) pChild->UpdateRect();

		OnChangePosition();

		return 1;
	}

	void CWindow::GetLocalPosition(long& rlx, long& rly)
	{
		rlx = rlx - m_rect.left;
		rly = rly - m_rect.top;
	}

	void CWindow::GetMouseLocalPosition(long& rlx, long& rly)
	{
		CWindowManager::Instance().GetMousePosition(rlx, rly);
		rlx = rlx - m_rect.left;
		rly = rly - m_rect.top;
	}

	void CWindow::AddChild(CWindow* pWin)
	{
		m_pChildList.push_back(pWin);
		pWin->m_pParent = this;
#ifdef ENABLE_INGAME_WIKI
		pWin->SetInsideRender(m_isInsideRender);
#endif
#ifdef ENABLE_USE_CLIP_MASK
		if (m_bEnableMask && m_pMaskWindow)
		{
			pWin->SetClippingMaskWindow(m_pMaskWindow);
		}
#endif
	}

#ifdef ENABLE_INGAME_WIKI
	void CWindow::SetInsideRender(BOOL flag)
	{
		if (!m_pParent || (m_isInsideRender && m_pParent->m_isInsideRender))
			return;

		if (m_isInsideRender && flag)
			return;

		m_isInsideRender = flag;
		UpdateRenderBox();
		for (auto it = m_pChildList.begin(); it != m_pChildList.end(); ++it)
			(*it)->SetInsideRender(m_isInsideRender);
	}

	void CWindow::GetRenderBox(RECT* box)
	{
		memcpy(box, &m_renderBox, sizeof(RECT));
	}
	
	void CWindow::UpdateTextLineRenderBox()
	{
		if (!m_isInsideRender || !m_pParent)
		{
			memset(&m_renderBox, 0, sizeof(m_renderBox));
			return;
		}

		int width, height;
		((CTextLine*)this)->GetTextSize(&width, &height);
		
		int pWidth = m_pParent->GetWidth();
		int pHeight = m_pParent->GetHeight();

		if (m_x - m_pParent->m_renderBox.left < 0)
			m_renderBox.left = -m_x + m_pParent->m_renderBox.left;
		else
			m_renderBox.left = 0;

		if (m_y - m_pParent->m_renderBox.top < 0)
			m_renderBox.top = -m_y + m_pParent->m_renderBox.top;
		else
			m_renderBox.top = 0;

		if (m_x + width > pWidth - m_pParent->m_renderBox.right)
			m_renderBox.right = m_x + width - pWidth + m_pParent->m_renderBox.right;
		else
			m_renderBox.right = 0;

		if (m_y + height > pHeight - m_pParent->m_renderBox.bottom)
			m_renderBox.bottom = m_y + height - pHeight + m_pParent->m_renderBox.bottom;
		else
			m_renderBox.bottom = 0;
	}

	void CWindow::UpdateRenderBox()
	{
		if (!m_isInsideRender || !m_pParent)
			memset(&m_renderBox, 0, sizeof(m_renderBox));
		else
		{
			int width = m_lWidth;
			int height = m_lHeight;
			int pWidth = m_pParent->GetWidth();
			int pHeight = m_pParent->GetHeight();

			if (m_x - m_pParent->m_renderBox.left < 0)
				m_renderBox.left = -m_x + m_pParent->m_renderBox.left;
			else
				m_renderBox.left = 0;

			if (m_y - m_pParent->m_renderBox.top < 0)
				m_renderBox.top = -m_y + m_pParent->m_renderBox.top;
			else
				m_renderBox.top = 0;

			if (m_x + width > pWidth - m_pParent->m_renderBox.right)
				m_renderBox.right = m_x + width - pWidth + m_pParent->m_renderBox.right;
			else
				m_renderBox.right = 0;

			if (m_y + height > pHeight - m_pParent->m_renderBox.bottom)
				m_renderBox.bottom = m_y + height - pHeight + m_pParent->m_renderBox.bottom;
			else
				m_renderBox.bottom = 0;
		}

		OnUpdateRenderBox();
	}

	void CWindow::UpdateRenderBoxRecursive()
	{
		UpdateRenderBox();
		for (auto it = m_pChildList.begin(); it != m_pChildList.end(); ++it)
			(*it)->UpdateRenderBoxRecursive();
	}

	void CWindow::SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom)
	{
		m_renderingRect.left = static_cast<LONG>(m_lWidth * fLeft);
		m_renderingRect.top = static_cast<LONG>(m_lHeight * fTop);
		m_renderingRect.right = static_cast<LONG>(m_lWidth * fRight);
		m_renderingRect.bottom = static_cast<LONG>(m_lHeight * fBottom);
	}

	void CWindow::iSetRenderingRect(int iLeft, int iTop, int iRight, int iBottom)
	{
		m_renderingRect.left = iLeft;
		m_renderingRect.top = iTop;
		m_renderingRect.right = iRight;
		m_renderingRect.bottom = iBottom;
	}

	int CWindow::GetRenderingWidth()
	{
		return m_lWidth - m_renderingRect.left - m_renderingRect.right;
	}

	int CWindow::GetRenderingHeight()
	{
		return m_lHeight - m_renderingRect.top - m_renderingRect.bottom;
	}

	void CWindow::ResetRenderingRect(bool bCallEvent)
	{
		memset(&m_renderingRect, 0, sizeof(m_renderingRect));
	}
#endif

	CWindow* CWindow::GetRoot()
	{
		if (m_pParent)
			if (m_pParent->IsWindow())
				return m_pParent->GetRoot();

		return this;
	}

	CWindow* CWindow::GetParent() const
	{
		return m_pParent;
	}

	bool CWindow::IsChild(CWindow* pWin)
	{
		std::list<CWindow*>::iterator itor = m_pChildList.begin();

		while (itor != m_pChildList.end())
		{
			if (*itor == pWin)
				return true;

			++itor;
		}

		return false;
	}

	void CWindow::DeleteChild(CWindow* pWin)
	{
		if (m_isUpdatingChildren)
		{
			m_pReserveChildList.push_back(pWin);
		}
		else
		{
			m_pChildList.remove(pWin);
		}
	}

	void CWindow::SetTop(CWindow* pWin)
	{
		if (!pWin->IsFlag(CWindow::FLAG_FLOAT))
			return;

		TWindowContainer::iterator itor = std::find(m_pChildList.begin(), m_pChildList.end(), pWin);
		if (m_pChildList.end() != itor)
		{
			m_pChildList.push_back(*itor);
			m_pChildList.erase(itor);

			pWin->OnTop();
		}
		else
		{
			TraceError(" CWindow::SetTop - Failed to find child window\n");
		}
	}

	void CWindow::OnMouseDrag(long lx, long ly)
	{
		PyCallClassMemberFunc(m_poHandler, "OnMouseDrag", Py_BuildValue("(ii)", lx, ly));
	}

	void CWindow::OnMoveWindow(long lx, long ly)
	{
		PyCallClassMemberFunc(m_poHandler, "OnMoveWindow", Py_BuildValue("(ii)", lx, ly));
	}

	void CWindow::OnSetFocus()
	{
		//PyCallClassMemberFunc(m_poHandler, "OnSetFocus", BuildEmptyTuple());
		PyCallClassMemberFunc(m_poHandler, "OnSetFocus", BuildEmptyTuple());
	}

	void CWindow::OnKillFocus()
	{
		PyCallClassMemberFunc(m_poHandler, "OnKillFocus", BuildEmptyTuple());
	}

	void CWindow::OnMouseOverIn()
	{
		PyCallClassMemberFunc(m_poHandler, "OnMouseOverIn", BuildEmptyTuple());
	}

	void CWindow::OnMouseOverOut()
	{
		PyCallClassMemberFunc(m_poHandler, "OnMouseOverOut", BuildEmptyTuple());
	}

	void CWindow::OnMouseOver()
	{
	}

	void CWindow::OnDrop()
	{
		PyCallClassMemberFunc(m_poHandler, "OnDrop", BuildEmptyTuple());
	}

	void CWindow::OnTop()
	{
		PyCallClassMemberFunc(m_poHandler, "OnTop", BuildEmptyTuple());
	}

	void CWindow::OnIMEUpdate()
	{
		PyCallClassMemberFunc(m_poHandler, "OnIMEUpdate", BuildEmptyTuple());
	}

#ifdef ENABLE_USE_CLIP_MASK
    void CWindow::SetClippingMaskRect(const RECT& rMask)
    {
        m_bEnableMask = true;
        m_rMaskRect = rMask;

        TWindowContainer::const_reverse_iterator itor;
        for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
        {
            CWindow* pWindow = *itor;
            if (pWindow)
            {
                pWindow->SetClippingMaskRect(rMask);
            }
        }
    }

    void CWindow::SetClippingMaskWindow(CWindow* pMaskWindow)
    {
        if (!pMaskWindow)
        {
            return;
        }

        m_bEnableMask = true;
        m_pMaskWindow = pMaskWindow;

        TWindowContainer::const_reverse_iterator itor;
        for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
        {
            CWindow* pWindow = *itor;
            if (pWindow)
            {
                pWindow->SetClippingMaskWindow(pMaskWindow);
            }
        }
    }

	bool CWindow::GetClippingMaskRect(RECT* pOutRect)
	{
		if (!pOutRect)
			return false;

		bool bHasMask = false;
		RECT resultRect;
		resultRect.left = -999999;
		resultRect.top = -999999;
		resultRect.right = 999999;
		resultRect.bottom = 999999;

		for (const CWindow* pCur = this; pCur != nullptr; pCur = pCur->m_pParent)
		{
			if (pCur->m_bEnableMask && pCur->m_pMaskWindow)
			{
				const RECT& r = pCur->m_pMaskWindow->GetRect();
				resultRect.left = max(resultRect.left, r.left);
				resultRect.top = max(resultRect.top, r.top);
				resultRect.right = min(resultRect.right, r.right);
				resultRect.bottom = min(resultRect.bottom, r.bottom);
				bHasMask = true;
			}
		}

		if (!bHasMask)
			return false;

		if (resultRect.left >= resultRect.right || resultRect.top >= resultRect.bottom)
		{
			resultRect.left = 0;
			resultRect.top = 0;
			resultRect.right = 0;
			resultRect.bottom = 0;
		}

		*pOutRect = resultRect;
		return true;
	}
#endif

	BOOL CWindow::RunIMETabEvent()
	{
		if (!IsRendering())
			return FALSE;

		if (OnIMETabEvent())
			return TRUE;

		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow* pWindow = *itor;

			if (pWindow->RunIMETabEvent())
				return TRUE;
		}

		return FALSE;
	}

	BOOL CWindow::RunIMEReturnEvent()
	{
		if (!IsRendering())
			return FALSE;

		if (OnIMEReturnEvent())
			return TRUE;

		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow* pWindow = *itor;

			if (pWindow->RunIMEReturnEvent())
				return TRUE;
		}

		return FALSE;
	}

	BOOL CWindow::RunIMEKeyDownEvent(int ikey)
	{
		if (!IsRendering())
			return FALSE;

		if (OnIMEKeyDownEvent(ikey))
			return TRUE;

		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow* pWindow = *itor;

			if (pWindow->RunIMEKeyDownEvent(ikey))
				return TRUE;
		}

		return FALSE;
	}

	CWindow* CWindow::RunKeyDownEvent(int ikey)
	{
		if (OnKeyDown(ikey))
			return this;

		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow* pWindow = *itor;

			if (pWindow->IsShow())
			{
				CWindow* pProcessedWindow = pWindow->RunKeyDownEvent(ikey);
				if (NULL != pProcessedWindow)
				{
					return pProcessedWindow;
				}
			}
		}

		return NULL;
	}

	BOOL CWindow::RunKeyUpEvent(int ikey)
	{
		if (OnKeyUp(ikey))
			return TRUE;

		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow* pWindow = *itor;

			if (pWindow->IsShow())
				if (pWindow->RunKeyUpEvent(ikey))
					return TRUE;
		}

		return FALSE;
	}

	BOOL CWindow::RunPressEscapeKeyEvent()
	{
		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow* pWindow = *itor;

			if (pWindow->IsShow())
				if (pWindow->RunPressEscapeKeyEvent())
					return TRUE;
		}

		if (OnPressEscapeKey())
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::RunPressExitKeyEvent()
	{
		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow* pWindow = *itor;

			if (pWindow->RunPressExitKeyEvent())
				return TRUE;

			if (pWindow->IsShow())
				if (pWindow->OnPressExitKey())
					return TRUE;
		}

		return FALSE;
	}

	BOOL CWindow::OnMouseLeftButtonDown()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseLeftButtonDown", BuildEmptyTuple(), &lValue))
			if (0 != lValue)
				return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnMouseLeftButtonUp()
	{
		PyCallClassMemberFunc(m_poHandler, "OnMouseLeftButtonUp", BuildEmptyTuple());
		return TRUE; // NOTE : ButtonUp은 예외로 무조건 TRUE
	}

	BOOL CWindow::OnMouseLeftButtonDoubleClick()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseLeftButtonDoubleClick", BuildEmptyTuple(), &lValue))
			if (0 != lValue)
				return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnMouseRightButtonDown()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseRightButtonDown", BuildEmptyTuple(), &lValue))
			if (0 != lValue)
				return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnMouseRightButtonUp()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseRightButtonUp", BuildEmptyTuple(), &lValue))
			if (0 != lValue)
				return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnMouseRightButtonDoubleClick()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseRightButtonDoubleClick", BuildEmptyTuple(), &lValue))
			if (0 != lValue)
				return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnMouseMiddleButtonDown()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseMiddleButtonDown", BuildEmptyTuple(), &lValue))
			if (0 != lValue)
				return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnMouseMiddleButtonUp()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseMiddleButtonUp", BuildEmptyTuple(), &lValue))
			if (0 != lValue)
				return TRUE;

		return FALSE;
	}

	BOOL CWindow::RunMouseWheelEvent(long nLen)
	{
		bool bValue = false;
		if (PyCallClassMemberFunc(m_poHandler, "OnRunMouseWheel", Py_BuildValue("(l)", nLen), &bValue))
			return bValue;
		return bValue;
	}

#ifdef ENABLE_MOUSEWHEEL_EVENT
	BOOL CWindow::OnMouseWheelScroll(short wDelta)
	{
#ifdef _DEBUG
		Tracenf("Mouse Wheel Scroll : wDelta %d ",wDelta);
#endif
		
		PyCallClassMemberFunc(m_poHandler , "OnMouseWheelScroll" , Py_BuildValue("(s)" , wDelta > 0? "UP":"DOWN") );
		return m_bIsScrollable;
	}


	void CWindow::SetScrollable()
	{
		m_bIsScrollable = true;
	}
#endif

	BOOL CWindow::OnIMETabEvent()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMETab", BuildEmptyTuple(), &lValue))
			if (0 != lValue)
				return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMEReturnEvent()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMEReturn", BuildEmptyTuple(), &lValue))
			if (0 != lValue)
				return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMEKeyDownEvent(int ikey)
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMEKeyDown", Py_BuildValue("(i)", ikey), &lValue))
			if (0 != lValue)
				return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMEChangeCodePage()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMEChangeCodePage", BuildEmptyTuple(), &lValue))
			if (0 != lValue)
				return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMEOpenCandidateListEvent()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMEOpenCandidateList", BuildEmptyTuple(), &lValue))
			if (0 != lValue)
				return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMECloseCandidateListEvent()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMECloseCandidateList", BuildEmptyTuple(), &lValue))
			if (0 != lValue)
				return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMEOpenReadingWndEvent()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMEOpenReadingWnd", BuildEmptyTuple(), &lValue))
			if (0 != lValue)
				return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMECloseReadingWndEvent()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMECloseReadingWnd", BuildEmptyTuple(), &lValue))
			if (0 != lValue)
				return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnKeyDown(int ikey)
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnKeyDown", Py_BuildValue("(i)", ikey), &lValue))
			if (0 != lValue)
				return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnKeyUp(int ikey)
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnKeyUp", Py_BuildValue("(i)", ikey), &lValue))
			if (0 != lValue)
				return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnPressEscapeKey()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnPressEscapeKey", BuildEmptyTuple(), &lValue))
			if (0 != lValue)
				return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnPressExitKey()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnPressExitKey", BuildEmptyTuple(), &lValue))
			if (0 != lValue)
				return TRUE;

		return FALSE;
	}

	/////

	bool CWindow::IsIn(long x, long y)
	{
		if (x >= m_rect.left && x <= m_rect.right)
			if (y >= m_rect.top && y <= m_rect.bottom)
				return true;

		return false;
	}

	bool CWindow::IsIn()
	{
		long lx, ly;
		UI::CWindowManager::Instance().GetMousePosition(lx, ly);

		return IsIn(lx, ly);
	}

	CWindow* CWindow::PickWindow(long x, long y)
	{
		std::list<CWindow*>::reverse_iterator ritor = m_pChildList.rbegin();
		for (; ritor != m_pChildList.rend(); ++ritor)
		{
			CWindow* pWin = *ritor;
			if (pWin->IsShow())
			{
				if (!pWin->IsFlag(CWindow::FLAG_IGNORE_SIZE))
				{
					if (!pWin->IsIn(x, y)) {
						if (0L <= pWin->GetWidth()) {
							continue;
						}
					}
				}

				CWindow* pResult = pWin->PickWindow(x, y);
				if (pResult)
					return pResult;
			}
		}

		if (IsFlag(CWindow::FLAG_NOT_PICK))
			return NULL;

		if (IsFlag(CWindow::FLAG_IS_BOARD_WITHOUT_ALPHA)) // check flag
		{
			return NULL;
		}

		if (IsFlag(CWindow::FLAG_ALPHA_SENSITIVE)) // check flag
		{
			bool isFullTransparent = true;
			IsTransparentOnPixel(&x, &y, &isFullTransparent); // check transparency of the clicking position
			if (isFullTransparent)
				return NULL; // if transparent then return nothing, else give current window
		}

		return (this);
	}

	void CWindow::IsTransparentOnPixel(long* x, long* y, bool* ret)
	{
		if (IsShow() && IsIn(*x, *y)) // check if the window is active and the cursor is in the window
		{
			if (m_windowType == WINDOW_TYPE_EX_IMAGE) // check if its an expanded_image
			{
				D3DXCOLOR pixel = ((CExpandedImageBox*)this)->GetPixelColor(*x - m_rect.left, *y - m_rect.top); // get current pixel color

				if ((BYTE)pixel.a != 0) // if the pixel is not trasparent then the whole window is not trasparent
				{
					*ret = false;
					return;
				}
			}
			else if (m_pChildList.empty()) // if its not ex_image and has no child then its NOT transparent [default for other components]
			{
				*ret = false;
				return;
			}
		}
		if (!m_pChildList.empty()) // check if all the childs are trasparent on the current position
		{
			std::list<CWindow *>::reverse_iterator ritor = m_pChildList.rbegin();
			for (; ritor != m_pChildList.rend(); ritor++)
			{
				(*ritor)->IsTransparentOnPixel(x, y, ret);
				if (!*ret)
					return;
			}
		}
	}

	void CWindow::HaveSomeChildOnOnPixel(long* x, long* y, bool* ret)
	{
		if (IsShow() && IsIn(*x, *y)) // check if the window is active and the cursor is in the window
		{
			if (m_windowType == WINDOW_TYPE_EX_IMAGE) // check if its an expanded_image
			{
				D3DXCOLOR pixel = ((CExpandedImageBox*)this)->GetPixelColor(*x - m_rect.left, *y - m_rect.top); // get current pixel color

				if ((BYTE)pixel.a != 0) // if the pixel is not trasparent then the whole window is not trasparent
				{
					*ret = false;
					return;
				}
			}
			else if (m_pChildList.empty()) // if its not ex_image and has no child then its NOT transparent [default for other components]
			{
				*ret = false;
				return;
			}
		}
		if (!m_pChildList.empty()) // check if all the childs are trasparent on the current position
		{
			std::list<CWindow *>::reverse_iterator ritor = m_pChildList.rbegin();
			for (; ritor != m_pChildList.rend(); ritor++)
			{
				(*ritor)->IsTransparentOnPixel(x, y, ret);
				if (!*ret)
					return;
			}
		}
	}

	CWindow* CWindow::PickTopWindow(long x, long y)
	{
		std::list<CWindow*>::reverse_iterator ritor = m_pChildList.rbegin();
		for (; ritor != m_pChildList.rend(); ++ritor)
		{
			CWindow* pWin = *ritor;
			if (pWin->IsShow())
				if (pWin->IsIn(x, y))
					if (!pWin->IsFlag(CWindow::FLAG_NOT_PICK))
						return pWin;
		}

		return NULL;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CUiRenderTarget::CUiRenderTarget(PyObject * ppyObject) : CWindow(ppyObject)
	{
		m_dwIndex = -1;
	}
	CUiRenderTarget::~CUiRenderTarget() = default;

	bool CUiRenderTarget::SetRenderTarget(uint8_t index)
	{
		if (!CRenderTargetManager::Instance().GetRenderTarget(index))
		{
			if (!CRenderTargetManager::Instance().CreateRenderTarget(index, GetWidth(), GetHeight()))
			{
				TraceError("CRenderTargetManager could not create the texture. w %d h %d", GetWidth(), GetHeight());
				return false;
			}
		}
		m_dwIndex = index;

		UpdateRect();
		return true;
	}

	void CUiRenderTarget::OnRender()
	{
		auto target = CRenderTargetManager::Instance().GetRenderTarget(m_dwIndex);
		if (!target)
		{
			TraceError("SetRenderingRect -> target empty!");
			return;
		}

		target->SetRenderingRect(&m_rect);
		target->RenderTexture();
	}

#ifdef ENABLE_INGAME_WIKI
	CUiWikiRenderTarget::~CUiWikiRenderTarget() = default;
	CUiWikiRenderTarget::CUiWikiRenderTarget(PyObject * ppyObject) :
															CWindow(ppyObject),
															m_dwIndex(-1) {}
	
	bool CUiWikiRenderTarget::SetWikiRenderTargetModule(int iRenderTargetModule)
	{
		if (!CWikiRenderTargetManager::Instance().GetRenderTarget(iRenderTargetModule))
		{
			if (!CWikiRenderTargetManager::Instance().CreateRenderTarget(iRenderTargetModule, GetWidth(), GetHeight()))
			{
				TraceError("CWikiRenderTargetManager could not create the texture. w %d h %d", GetWidth(), GetHeight());
				return false;
			}
		}
		
		m_dwIndex = iRenderTargetModule;
		
		UpdateRect();
		return true;
	}
	
	void CUiWikiRenderTarget::OnUpdateRenderBox()
	{
		if (m_dwIndex == -1)
			return;
		
		auto target = CWikiRenderTargetManager::Instance().GetRenderTarget(m_dwIndex);
		if (!target)
			return;
		
		target->SetRenderingBox(&m_renderBox);
	}
	
	/*----------------------------
	-----PROTECTED CLASS FUNCTIONS
	-----------------------------*/
	
	void CUiWikiRenderTarget::OnRender()
	{
		if (m_dwIndex == -1)
			return;
		
		auto target = CWikiRenderTargetManager::Instance().GetRenderTarget(m_dwIndex);
		if (!target)
			return;
		
#ifdef ENABLE_USE_CLIP_MASK
		RECT mask;
		if (GetClippingMaskRect(&mask))
		{
			RECT clipped = m_rect;
			clipped.left   = max(clipped.left,   mask.left);
			clipped.top    = max(clipped.top,    mask.top);
			clipped.right  = min(clipped.right,  mask.right);
			clipped.bottom = min(clipped.bottom, mask.bottom);
			
			if (clipped.left >= clipped.right || clipped.top >= clipped.bottom)
				return;
			
			target->SetRenderingRect(&clipped);
		}
		else
#endif
		{
			target->SetRenderingRect(&m_rect);
		}
		target->SetRenderedThisFrame();
		target->RenderTexture();
	}
#endif

	CBox::CBox(PyObject * ppyObject) : CWindow(ppyObject), m_dwColor(0xff000000)
	{
	}
	CBox::~CBox()
	{
	}

	void CBox::SetColor(DWORD dwColor)
	{
		m_dwColor = dwColor;
	}

	void CBox::OnRender()
	{
		RECT rect = m_rect;

#ifdef ENABLE_USE_CLIP_MASK
		RECT mask;
		if (GetClippingMaskRect(&mask))
		{
			rect.left = max(rect.left, mask.left);
			rect.top = max(rect.top, mask.top);
			rect.right = min(rect.right, mask.right);
			rect.bottom = min(rect.bottom, mask.bottom);

			if (rect.left >= rect.right || rect.top >= rect.bottom)
				return;
		}
#endif

		CPythonGraphic::Instance().SetDiffuseColor(m_dwColor);
		CPythonGraphic::Instance().RenderBox2d(rect.left, rect.top, rect.right, rect.bottom);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CBar::CBar(PyObject* ppyObject) : CWindow(ppyObject), m_dwColor(0xff000000)
	{
	}
	CBar::~CBar()
	{
	}

	void CBar::SetColor(DWORD dwColor)
	{
		m_dwColor = dwColor;
	}

	void CBar::OnRender()
	{
		RECT rect = m_rect;

#ifdef ENABLE_USE_CLIP_MASK
		RECT mask;
		if (GetClippingMaskRect(&mask))
		{
			rect.left = max(rect.left, mask.left);
			rect.top = max(rect.top, mask.top);
			rect.right = min(rect.right, mask.right);
			rect.bottom = min(rect.bottom, mask.bottom);

			if (rect.left >= rect.right || rect.top >= rect.bottom)
				return;
		}
#endif

		CPythonGraphic::Instance().SetDiffuseColor(m_dwColor);
		CPythonGraphic::Instance().RenderBar2d(rect.left, rect.top, rect.right, rect.bottom);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CLine::CLine(PyObject* ppyObject) : CWindow(ppyObject), m_dwColor(0xff000000)
	{
	}
	CLine::~CLine()
	{
	}

	void CLine::SetColor(DWORD dwColor)
	{
		m_dwColor = dwColor;
	}

	void CLine::OnRender()
	{
		RECT rect = m_rect;

#ifdef ENABLE_USE_CLIP_MASK
		RECT mask;
		if (GetClippingMaskRect(&mask))
		{
			if (rect.top == rect.bottom) // Horizontal
			{
				if (rect.top < mask.top || rect.top > mask.bottom) return;
				rect.left = max(rect.left, mask.left);
				rect.right = min(rect.right, mask.right);
				if (rect.left >= rect.right) return;
			}
			else if (rect.left == rect.right) // Vertical
			{
				if (rect.left < mask.left || rect.left > mask.right) return;
				rect.top = max(rect.top, mask.top);
				rect.bottom = min(rect.bottom, mask.bottom);
				if (rect.top >= rect.bottom) return;
			}
			else
			{
				if (rect.left > mask.right || rect.right < mask.left || rect.top > mask.bottom || rect.bottom < mask.top)
					return;
				rect.left = max(rect.left, mask.left);
				rect.top = max(rect.top, mask.top);
				rect.right = min(rect.right, mask.right);
				rect.bottom = min(rect.bottom, mask.bottom);
			}
		}
#endif

		CPythonGraphic& rkpyGraphic = CPythonGraphic::Instance();
		rkpyGraphic.SetDiffuseColor(m_dwColor);
		rkpyGraphic.RenderLine2d(rect.left, rect.top, rect.right, rect.bottom);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	DWORD CBar3D::Type()
	{
		static DWORD s_dwType = GetCRC32("CBar3D", strlen("CBar3D"));
		return (s_dwType);
	}

	CBar3D::CBar3D(PyObject* ppyObject) : CWindow(ppyObject)
	{
		m_dwLeftColor = D3DXCOLOR(0.2f, 0.2f, 0.2f, 1.0f);
		m_dwRightColor = D3DXCOLOR(0.7f, 0.7f, 0.7f, 1.0f);
		m_dwCenterColor = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	}
	CBar3D::~CBar3D()
	{
	}

	void CBar3D::SetColor(DWORD dwLeft, DWORD dwRight, DWORD dwCenter)
	{
		m_dwLeftColor = dwLeft;
		m_dwRightColor = dwRight;
		m_dwCenterColor = dwCenter;
	}

	void CBar3D::OnRender()
	{
		CPythonGraphic& rkpyGraphic = CPythonGraphic::Instance();

		rkpyGraphic.SetDiffuseColor(m_dwCenterColor);
		rkpyGraphic.RenderBar2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);

		rkpyGraphic.SetDiffuseColor(m_dwLeftColor);
		rkpyGraphic.RenderLine2d(m_rect.left, m_rect.top, m_rect.right, m_rect.top);
		rkpyGraphic.RenderLine2d(m_rect.left, m_rect.top, m_rect.left, m_rect.bottom);

		rkpyGraphic.SetDiffuseColor(m_dwRightColor);
		rkpyGraphic.RenderLine2d(m_rect.left, m_rect.bottom, m_rect.right, m_rect.bottom);
		rkpyGraphic.RenderLine2d(m_rect.right, m_rect.top, m_rect.right, m_rect.bottom);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CTextLine::CTextLine(PyObject* ppyObject) : CWindow(ppyObject)
	{
		m_TextInstance.SetColor(0.78f, 0.78f, 0.78f);
		m_TextInstance.SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_LEFT);
		m_TextInstance.SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_TOP);
	}
	CTextLine::~CTextLine()
	{
		m_TextInstance.Destroy();
	}

	void CTextLine::SetMax(int iMax)
	{
		m_TextInstance.SetMax(iMax);
	}
	void CTextLine::SetHorizontalAlign(int iType)
	{
		m_TextInstance.SetHorizonalAlign(iType);
	}
	void CTextLine::SetVerticalAlign(int iType)
	{
		m_TextInstance.SetVerticalAlign(iType);
	}
	void CTextLine::SetSecret(BOOL bFlag)
	{
		m_TextInstance.SetSecret(bFlag ? true : false);
	}
	void CTextLine::SetOutline(BOOL bFlag)
	{
		m_TextInstance.SetOutline(bFlag ? true : false);
	}
	void CTextLine::SetFeather(BOOL bFlag)
	{
		m_TextInstance.SetFeather(bFlag ? true : false);
	}
	void CTextLine::SetMultiLine(BOOL bFlag)
	{
		m_TextInstance.SetMultiLine(bFlag ? true : false);
	}
	WORD CTextLine::GetTextLineCount()
	{
		return m_TextInstance.GetTextLineCount();
	}
	WORD CTextLine::GetLineHeight()
	{
		return m_TextInstance.GetLineHeight();
	}
#ifdef ENABLE_SUNG_MAHI_TOWER
	void CTextLine::GetCharSize(short* sWidth)
	{
		m_TextInstance.GetCharacterWidth(sWidth);
	}
#endif
	void CTextLine::SetLineHeight(int iHeight)
	{
		m_TextInstance.SetLineHeight(iHeight);
		m_TextInstance.Update();
	}
	void CTextLine::SetFontName(const char* c_szFontName)
	{
		std::string stFontName = c_szFontName;
		stFontName += ".fnt";

		CResourceManager& rkResMgr = CResourceManager::Instance();
		CResource* pkRes = rkResMgr.GetTypeResourcePointer(stFontName.c_str());
		CGraphicText* pkResFont = static_cast<CGraphicText*>(pkRes);
		m_TextInstance.SetTextPointer(pkResFont);
	}
	void CTextLine::SetFontColor(DWORD dwColor)
	{
		m_TextInstance.SetColor(dwColor);
	}
	void CTextLine::SetLimitWidth(float fWidth)
	{
		m_TextInstance.SetLimitWidth(fWidth);
	}
	void CTextLine::SetText(const char* c_szText)
	{
		OnSetText(c_szText);
	}
	void CTextLine::GetTextSize(int* pnWidth, int* pnHeight)
	{
		m_TextInstance.GetTextSize(pnWidth, pnHeight);
	}
	const char* CTextLine::GetText()
	{
		return m_TextInstance.GetValueStringReference().c_str();
	}
	void CTextLine::ShowCursor()
	{
		m_TextInstance.ShowCursor();
	}
	void CTextLine::HideCursor()
	{
		m_TextInstance.HideCursor();
	}
	int CTextLine::GetCursorPosition()
	{
		long lx, ly;
		CWindow::GetMouseLocalPosition(lx, ly);
		return m_TextInstance.PixelPositionToCharacterPosition(lx);
	}

	void CTextLine::OnSetText(const char* c_szText)
	{
		m_TextInstance.SetValue(c_szText);
		m_TextInstance.Update();
	}

#ifdef ENABLE_INGAME_WIKI
	int CTextLine::GetRenderingWidth()
	{
		return m_TextInstance.GetWidth() - m_renderingRect.left - m_renderingRect.right;
	}

	int CTextLine::GetRenderingHeight()
	{
		return m_TextInstance.GetHeight() - m_renderingRect.top - m_renderingRect.bottom;
	}

	void CTextLine::OnSetRenderingRect()
	{
		m_TextInstance.iSetRenderingRect(m_renderingRect.left, m_renderingRect.top, m_renderingRect.right, m_renderingRect.bottom);
	}
#endif

    void CTextLine::OnUpdate()
    {
        if (IsShow())
        {
#ifdef ENABLE_USE_CLIP_MASK
            if (m_bEnableMask && m_pMaskWindow)
            {
                m_rMaskRect = m_pMaskWindow->GetRect();
            }
#endif

            m_TextInstance.Update();
        }
    }

    void CTextLine::OnRender()
    {
        if (IsShow())
        {
#ifdef ENABLE_USE_CLIP_MASK
            RECT mask;
            if (GetClippingMaskRect(&mask))
            {
                m_TextInstance.Render(&mask);
            }
            else
            {
                m_TextInstance.Render(nullptr);
            }
#else
            m_TextInstance.Render();
#endif
        }
    }

	void CTextLine::OnChangePosition()
	{
		// FOR_ARABIC_ALIGN
		//if (m_TextInstance.GetHorizontalAlign() == CGraphicTextInstance::HORIZONTAL_ALIGN_ARABIC)
		if (GetDefaultCodePage() == CP_ARABIC)
		{
			m_TextInstance.SetPosition(m_rect.right, m_rect.top);
		}
		else
		{
			m_TextInstance.SetPosition(m_rect.left, m_rect.top);
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CNumberLine::CNumberLine(PyObject* ppyObject) : CWindow(ppyObject)
	{
		m_strPath = "d:/ymir work/ui/game/taskbar/";
		m_iHorizontalAlign = HORIZONTAL_ALIGN_LEFT;
		m_dwWidthSummary = 0;
#ifdef ENABLE_SLOT_TEXT_SIZE
		m_bIsZoomed = false;
#endif
	}
	CNumberLine::CNumberLine(CWindow* pParent) : CWindow(NULL)
	{
		m_strPath = "d:/ymir work/ui/game/taskbar/";
		m_iHorizontalAlign = HORIZONTAL_ALIGN_LEFT;
		m_dwWidthSummary = 0;

		m_pParent = pParent;
#ifdef ENABLE_SLOT_TEXT_SIZE
		m_bIsZoomed = false;
#endif
	}
	CNumberLine::~CNumberLine()
	{
		ClearNumber();
	}

	void CNumberLine::SetPath(const char* c_szPath)
	{
		m_strPath = c_szPath;
	}
	void CNumberLine::SetHorizontalAlign(int iType)
	{
		m_iHorizontalAlign = iType;
	}
	void CNumberLine::SetNumber(const char* c_szNumber)
	{
		if (0 == m_strNumber.compare(c_szNumber))
			return;

		ClearNumber();

		m_strNumber = c_szNumber;

		for (DWORD i = 0; i < m_strNumber.size(); ++i)
		{
			char cChar = m_strNumber[i];
			std::string strImageFileName;

			if (':' == cChar)
			{
				strImageFileName = m_strPath + "colon.sub";
			}
			else if ('?' == cChar)
			{
				strImageFileName = m_strPath + "questionmark.sub";
			}
			else if ('/' == cChar)
			{
				strImageFileName = m_strPath + "slash.sub";
			}
			else if ('%' == cChar)
			{
				strImageFileName = m_strPath + "percent.sub";
			}
			else if ('+' == cChar)
			{
				strImageFileName = m_strPath + "plus.sub";
			}
			else if ('m' == cChar)
			{
				strImageFileName = m_strPath + "m.sub";
			}
			else if ('g' == cChar)
			{
				strImageFileName = m_strPath + "g.sub";
			}
			else if ('p' == cChar)
			{
				strImageFileName = m_strPath + "p.sub";
			}
			else if (cChar >= '0' && cChar <= '9')
			{
				strImageFileName = m_strPath;
				strImageFileName += cChar;
				strImageFileName += ".sub";
			}
			else
				continue;

			if (!CResourceManager::Instance().IsFileExist(strImageFileName.c_str()))
				continue;

#ifdef ENABLE_SLOT_TEXT_SIZE
			CGraphicImage * pImage = (CGraphicImage *)CResourceManager::Instance().GetResourcePointer(strImageFileName.c_str());

			CGraphicExpandedImageInstance * pInstance = CGraphicExpandedImageInstance::New();
			pInstance->SetImagePointer(pImage);

			pInstance->SetScale(m_bIsZoomed ? 2.0f : 1.0f, m_bIsZoomed ? 2.0f : 1.0f); 

			m_ImageInstanceVector.push_back(pInstance);
			m_dwWidthSummary += pImage->GetWidth(); 
#else
			CGraphicImage * pImage = (CGraphicImage *)CResourceManager::Instance().GetResourcePointer(strImageFileName.c_str());

			CGraphicImageInstance * pInstance = CGraphicImageInstance::New();
			pInstance->SetImagePointer(pImage);
			m_ImageInstanceVector.push_back(pInstance);

			m_dwWidthSummary += pInstance->GetWidth();
#endif
		}
	}

	void CNumberLine::ClearNumber()
	{
		m_ImageInstanceVector.clear();
		m_dwWidthSummary = 0;
		m_strNumber = "";
	}

	void CNumberLine::OnRender()
	{
#ifdef ENABLE_SLOT_TEXT_SIZE
		bool bIsCtrlPressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;

		if (m_bIsZoomed != bIsCtrlPressed)
		{
			m_bIsZoomed = bIsCtrlPressed;

			for (DWORD i = 0; i < m_ImageInstanceVector.size(); ++i)
			{
				CGraphicExpandedImageInstance* pInstance = (CGraphicExpandedImageInstance*)m_ImageInstanceVector[i];
				pInstance->SetScale(m_bIsZoomed ? 2.0f : 1.0f, m_bIsZoomed ? 2.0f : 1.0f);
			}
			
			OnChangePosition(); 
		}
#endif
		for (DWORD i = 0; i < m_ImageInstanceVector.size(); ++i)
		{
			m_ImageInstanceVector[i]->Render();
		}
	}

	void CNumberLine::OnChangePosition()
	{
		int ix = m_x;
		int iy = m_y;

		if (m_pParent)
		{
			ix = m_rect.left;
			iy = m_rect.top;
		}

#ifdef ENABLE_SLOT_TEXT_SIZE
		int iCurrentWidthSummary = m_dwWidthSummary * (m_bIsZoomed ? 2 : 1);
#endif

		switch (m_iHorizontalAlign)
		{
			case HORIZONTAL_ALIGN_LEFT:
				break;
			case HORIZONTAL_ALIGN_CENTER:
#ifdef ENABLE_SLOT_TEXT_SIZE
				ix -= int(iCurrentWidthSummary) / 2;
#else
				ix -= int(m_dwWidthSummary) / 2;
#endif
				break;
			case HORIZONTAL_ALIGN_RIGHT:
#ifdef ENABLE_SLOT_TEXT_SIZE
				ix -= int(iCurrentWidthSummary);
#else
				ix -= int(m_dwWidthSummary);
#endif
				break;
		}

#ifdef ENABLE_SLOT_TEXT_SIZE
		if (m_bIsZoomed)
		{
			iy -= 4; 
			ix += 4; 
		}

		for (DWORD i = 0; i < m_ImageInstanceVector.size(); ++i)
		{
			m_ImageInstanceVector[i]->SetPosition(ix, iy);
			ix += (m_ImageInstanceVector[i]->GetWidth() * (m_bIsZoomed ? 2 : 1));
		}
#else
		for (DWORD i = 0; i < m_ImageInstanceVector.size(); ++i)
		{
			m_ImageInstanceVector[i]->SetPosition(ix, iy);
			ix += m_ImageInstanceVector[i]->GetWidth();
		}
#endif

	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CImageBox::CImageBox(PyObject* ppyObject) :
		CWindow(ppyObject),
		m_fCoolTime(0.0f),
		m_fCoolTimeStart(0.0f)
	{
		m_pImageInstance = NULL;
#ifdef ENABLE_12ZI
		fCoolTime = 0.0f;
		fStartCoolTime = 0.0f;
#endif
	}
	CImageBox::~CImageBox()
	{
		OnDestroyInstance();
	}

	void CImageBox::OnCreateInstance()
	{
		OnDestroyInstance();

		m_pImageInstance = CGraphicImageInstance::New();
	}
	void CImageBox::OnDestroyInstance()
	{
		if (m_pImageInstance)
		{
			CGraphicImageInstance::Delete(m_pImageInstance);
			m_pImageInstance = NULL;
		}
	}

	BOOL CImageBox::LoadImage(const char* c_szFileName)
	{
		if (!c_szFileName[0])
			return FALSE;

		OnCreateInstance();

		CResource* pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
		if (!pResource)
			return FALSE;
		if (!pResource->IsType(CGraphicImage::Type()))
			return FALSE;

		m_pImageInstance->SetImagePointer(static_cast<CGraphicImage*>(pResource));
		if (m_pImageInstance->IsEmpty())
			return FALSE;

		SetSize(m_pImageInstance->GetWidth(), m_pImageInstance->GetHeight());
		UpdateRect();

		return TRUE;
	}

	void CImageBox::SetScale(float sx, float sy)
	{
		if (!m_pImageInstance)
			return;

		((CGraphicImageInstance*)m_pImageInstance)->SetScale(sx, sy);
		CWindow::SetSize(long(float(GetWidth()) * sx), long(float(GetHeight()) * sy));
	}

	void CImageBox::SetDiffuseColor(float fr, float fg, float fb, float fa)
	{
		if (!m_pImageInstance)
			return;

		m_pImageInstance->SetDiffuseColor(fr, fg, fb, fa);
	}

	int CImageBox::GetWidth()
	{
		if (!m_pImageInstance)
			return 0;

		return m_pImageInstance->GetWidth();
	}

	int CImageBox::GetHeight()
	{
		if (!m_pImageInstance)
			return 0;

		return m_pImageInstance->GetHeight();
	}

    void CImageBox::OnUpdate()
    {
#ifdef ENABLE_USE_CLIP_MASK
        if (!m_pImageInstance)
        {
            return;
        }

        if (!IsShow())
        {
            return;
        }

        if (!m_bEnableMask)
        {
            return;
        }

        if (!m_pMaskWindow)
        {
            return;
        }

        m_rMaskRect = m_pMaskWindow->GetRect();
#endif
    }
	void CImageBox::OnRender()
	{
		if (!m_pImageInstance)
			return;

		if (IsShow())
		{
#ifdef ENABLE_USE_CLIP_MASK
			RECT mask;
			RECT* pMaskRect = GetClippingMaskRect(&mask) ? &mask : nullptr;
			if (m_fCoolTime == 0.0f)
				m_pImageInstance->Render(pMaskRect);
			else
				m_pImageInstance->RenderCoolTime((CTimer::Instance().GetCurrentSecond() - m_fCoolTimeStart) / m_fCoolTime, pMaskRect);
#else
			if (m_fCoolTime == 0.0f)
				m_pImageInstance->Render();
			else
				m_pImageInstance->RenderCoolTime((CTimer::Instance().GetCurrentSecond() - m_fCoolTimeStart) / m_fCoolTime);
#endif
		}

#ifdef ENABLE_12ZI
		if (fCoolTime != 0.0f)
		{
			float fcurTime = CTimer::Instance().GetCurrentSecond();
			float fPercentage = (fcurTime - fStartCoolTime) / fCoolTime;
			// CPythonGraphic::Instance().RenderZodiacCoolTimeBox(m_rect.left + GetWidth() + 16.0f, m_rect.top + GetHeight() + 16.0f, 16.0f, fPercentage);
			CPythonGraphic::Instance().RenderCoolTimeBox(m_rect.left + GetWidth() + 16.0f, m_rect.top + GetHeight() + 16.0f, 16.0f, fPercentage);

			if (fcurTime - fStartCoolTime >= fCoolTime)
			{
				fCoolTime = 0.0f;
				fStartCoolTime = 0.0f;
			}
		}
#endif
	}
	void CImageBox::OnChangePosition()
	{
		if (!m_pImageInstance)
			return;

		m_pImageInstance->SetPosition(m_rect.left, m_rect.top);
	}

	void CImageBox::SetCoolTime(float fCoolTime, float fElapsedTime)
	{
		m_fCoolTime = fCoolTime;
		m_fCoolElapsedTime = fElapsedTime;
	}

	void CImageBox::SetCoolTimeStart(float fCoolTimeStart)
	{
		m_fCoolTimeStart = fCoolTimeStart;
	}
	
	BOOL CImageBox::IsInCoolTime()
	{
		BOOL inInCoolTime = (m_fCoolTime > 0.0f);
		return inInCoolTime;
	}

#ifdef ENABLE_12ZI
	void CImageBox::SetCoolTimeImageBox(float CT)
	{
		if (!m_pImageInstance)
			return;

		fCoolTime = CT;
	}

	void CImageBox::SetStartCoolTimeImageBox(float SCT)
	{
		if (!m_pImageInstance)
			return;

		fStartCoolTime = SCT;
	}
#endif

	///////////////////////////////////////////////////////////////////////////////////////////////
	// MarkBox - 마크 출력용 UI 윈도우
	///////////////////////////////////////////////////////////////////////////////////////////////
	CMarkBox::CMarkBox(PyObject* ppyObject) : CWindow(ppyObject)
	{
		m_pMarkInstance = NULL;
	}

	CMarkBox::~CMarkBox()
	{
		OnDestroyInstance();
	}

	void CMarkBox::OnCreateInstance()
	{
		OnDestroyInstance();
		m_pMarkInstance = CGraphicMarkInstance::New();
	}

	void CMarkBox::OnDestroyInstance()
	{
		if (m_pMarkInstance)
		{
			CGraphicMarkInstance::Delete(m_pMarkInstance);
			m_pMarkInstance = NULL;
		}
	}

	void CMarkBox::LoadImage(const char* c_szFilename)
	{
		OnCreateInstance();

		m_pMarkInstance->SetImageFileName(c_szFilename);
		m_pMarkInstance->Load();
		SetSize(m_pMarkInstance->GetWidth(), m_pMarkInstance->GetHeight());

		UpdateRect();
	}

	void CMarkBox::SetScale(FLOAT fScale)
	{
		if (!m_pMarkInstance)
			return;

		m_pMarkInstance->SetScale(fScale);
	}

	void CMarkBox::SetIndex(UINT uIndex)
	{
		if (!m_pMarkInstance)
			return;

		m_pMarkInstance->SetIndex(uIndex);
	}

	void CMarkBox::SetDiffuseColor(float fr, float fg, float fb, float fa)
	{
		if (!m_pMarkInstance)
			return;

		m_pMarkInstance->SetDiffuseColor(fr, fg, fb, fa);
	}

	void CMarkBox::OnUpdate()
	{
	}

	void CMarkBox::OnRender()
	{
		if (!m_pMarkInstance)
			return;

		if (IsShow())
			m_pMarkInstance->Render();
	}

	void CMarkBox::OnChangePosition()
	{
		if (!m_pMarkInstance)
			return;

		m_pMarkInstance->SetPosition(m_rect.left, m_rect.top);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	DWORD CExpandedImageBox::Type()
	{
		static DWORD s_dwType = GetCRC32("CExpandedImageBox", strlen("CExpandedImageBox"));
		return (s_dwType);
	}

	BOOL CExpandedImageBox::OnIsType(DWORD dwType)
	{
		if (CExpandedImageBox::Type() == dwType)
			return TRUE;

		return FALSE;
	}

	CExpandedImageBox::CExpandedImageBox(PyObject* ppyObject) : CImageBox(ppyObject)
	{
	}
	CExpandedImageBox::~CExpandedImageBox()
	{
		OnDestroyInstance();
	}

	void CExpandedImageBox::OnCreateInstance()
	{
		OnDestroyInstance();

		m_pImageInstance = CGraphicExpandedImageInstance::New();
	}
	void CExpandedImageBox::OnDestroyInstance()
	{
		if (m_pImageInstance)
		{
			CGraphicExpandedImageInstance::Delete((CGraphicExpandedImageInstance*)m_pImageInstance);
			m_pImageInstance = NULL;
		}
	}

	void CExpandedImageBox::SetScale(float fx, float fy)
	{
		if (!m_pImageInstance)
			return;

		((CGraphicExpandedImageInstance*)m_pImageInstance)->SetScale(fx, fy);
		CWindow::SetSize(long(float(GetWidth()) * fx), long(float(GetHeight()) * fy));
	}
	void CExpandedImageBox::SetOrigin(float fx, float fy)
	{
		if (!m_pImageInstance)
			return;

		((CGraphicExpandedImageInstance*)m_pImageInstance)->SetOrigin(fx, fy);
	}
	void CExpandedImageBox::SetRotation(float fRotation)
	{
		if (!m_pImageInstance)
			return;

		((CGraphicExpandedImageInstance*)m_pImageInstance)->SetRotation(fRotation);
	}
	void CExpandedImageBox::SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom)
	{
		if (!m_pImageInstance)
			return;

		((CGraphicExpandedImageInstance*)m_pImageInstance)->SetRenderingRect(fLeft, fTop, fRight, fBottom);
	}

	void CExpandedImageBox::SetImageClipRect(float fLeft, float fTop, float fRight, float fBottom, bool bIsVertical)
	{
		if (!m_pImageInstance)
			return;
		
		const RECT & c_rRect = GetRect();
		
		float fDifLeft = (c_rRect.left < fLeft) ? -(float(fLeft - c_rRect.left) / float(GetWidth())) : 0.0f;
		float fDifTop = (c_rRect.top < fTop) ? -(float(fTop - c_rRect.top) / float(GetHeight())) : 0.0f;
		float fDifRight = (c_rRect.right > fRight) ? -(float(c_rRect.right - fRight) / float(GetWidth())) : 0.0f;
		float fDifBottom = (c_rRect.bottom > fBottom) ? -(float(c_rRect.bottom - fBottom) / float(GetHeight())) : 0.0f;

		if(bIsVertical)
			((CGraphicExpandedImageInstance*)m_pImageInstance)->SetRenderingRect(fLeft, fDifTop, fRight, fDifBottom);
		else
			((CGraphicExpandedImageInstance*)m_pImageInstance)->SetRenderingRect(fDifLeft, fDifTop, fDifRight, fDifBottom);
	}

	void CExpandedImageBox::SetRenderingMode(int iMode)
	{
		((CGraphicExpandedImageInstance*)m_pImageInstance)->SetRenderingMode(iMode);
	}

#ifdef ENABLE_INGAME_WIKI
	int CExpandedImageBox::GetRenderingWidth()
	{
		return GetWidth() - m_renderingRect.left - m_renderingRect.right;
	}

	int CExpandedImageBox::GetRenderingHeight()
	{
		return GetHeight() - m_renderingRect.top - m_renderingRect.bottom;
	}

	void CExpandedImageBox::OnSetRenderingRect()
	{
		if (!m_pImageInstance)
			return;

		((CGraphicExpandedImageInstance*)m_pImageInstance)->iSetRenderingRect(m_renderingRect.left, m_renderingRect.top, m_renderingRect.right, m_renderingRect.bottom);
	}

	void CExpandedImageBox::SetExpandedRenderingRect(float fLeftTop, float fLeftBottom, float fTopLeft, float fTopRight, float fRightTop, float fRightBottom, float fBottomLeft, float fBottomRight)
	{
		if (!m_pImageInstance)
			return;

		((CGraphicExpandedImageInstance*)m_pImageInstance)->SetExpandedRenderingRect(fLeftTop, fLeftBottom, fTopLeft, fTopRight, fRightTop, fRightBottom, fBottomLeft, fBottomRight);
	}

	void CExpandedImageBox::SetTextureRenderingRect(float fLeft, float fTop, float fRight, float fBottom)
	{
		if (!m_pImageInstance)
			return;

		((CGraphicExpandedImageInstance*)m_pImageInstance)->SetTextureRenderingRect(fLeft, fTop, fRight, fBottom);
	}

	void CExpandedImageBox::OnUpdateRenderBox()
	{
		if (!m_pImageInstance)
			return;

		((CGraphicExpandedImageInstance*)m_pImageInstance)->SetRenderBox(m_renderBox);
	}
#endif

	void CExpandedImageBox::OnUpdate()
	{
#ifdef ENABLE_USE_CLIP_MASK
		if (!m_pImageInstance)
			return;

		if (!m_pMaskWindow)
			return;

		m_rMaskRect = m_pMaskWindow->GetRect();
#endif
	}
	void CExpandedImageBox::OnRender()
	{
		if (!m_pImageInstance)
			return;

		if (IsShow())
		{
			if (m_fCoolTime == 0.0f)
#ifdef ENABLE_USE_CLIP_MASK
			{
				RECT mask;
				RECT* pMaskRect = GetClippingMaskRect(&mask) ? &mask : nullptr;
				m_pImageInstance->Render(pMaskRect);
			}
#else
				m_pImageInstance->Render();
#endif
			else
				((CGraphicExpandedImageInstance*)m_pImageInstance)->RenderCoolTime((CTimer::Instance().GetCurrentSecond() - m_fCoolTimeStart) / m_fCoolTime);
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	DWORD CAniImageBox::Type()
	{
		static DWORD s_dwType = GetCRC32("CAniImageBox", strlen("CAniImageBox"));
		return (s_dwType);
	}

	BOOL CAniImageBox::OnIsType(DWORD dwType)
	{
		if (CAniImageBox::Type() == dwType)
			return TRUE;

		return FALSE;
	}

	CAniImageBox::CAniImageBox(PyObject* ppyObject)
		: CWindow(ppyObject),
		m_bycurDelay(0),
		m_byDelay(4),
		m_bycurIndex(0)
	{
		m_ImageVector.clear();
	}
	CAniImageBox::~CAniImageBox()
	{
		for_each(m_ImageVector.begin(), m_ImageVector.end(), CGraphicExpandedImageInstance::DeleteExpandedImageInstance);
	}

	void CAniImageBox::SetDelay(int iDelay)
	{
		m_byDelay = iDelay;
	}
#if defined(ENABLE_SASH_SYSTEM) || defined(ENABLE_CHANGELOOK_SYSTEM)
	void CAniImageBox::AppendImage(const char* c_szFileName, float r, float g, float b, float a)
#else
	void CAniImageBox::AppendImage(const char* c_szFileName)
#endif
	{
		CResource* pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
		if (!pResource->IsType(CGraphicImage::Type()))
			return;

		CGraphicExpandedImageInstance* pImageInstance = CGraphicExpandedImageInstance::New();

		pImageInstance->SetImagePointer(static_cast<CGraphicImage*>(pResource));
#if defined(ENABLE_SASH_SYSTEM) || defined(ENABLE_CHANGELOOK_SYSTEM)
		pImageInstance->SetDiffuseColor(r, g, b, a);
#endif

		if (pImageInstance->IsEmpty())
		{
			CGraphicExpandedImageInstance::Delete(pImageInstance);
			return;
		}

		m_ImageVector.push_back(pImageInstance);

		m_bycurIndex = rand() % m_ImageVector.size();
		// SetSize(pImageInstance->GetWidth(), pImageInstance->GetHeight());
		// UpdateRect();
	}

	struct FSetRenderingRect
	{
		float fLeft, fTop, fRight, fBottom;
		void operator () (CGraphicExpandedImageInstance* pInstance)
		{
			pInstance->SetRenderingRect(fLeft, fTop, fRight, fBottom);
		}
	};
	void CAniImageBox::SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom)
	{
		FSetRenderingRect setRenderingRect;
		setRenderingRect.fLeft = fLeft;
		setRenderingRect.fTop = fTop;
		setRenderingRect.fRight = fRight;
		setRenderingRect.fBottom = fBottom;
		for_each(m_ImageVector.begin(), m_ImageVector.end(), setRenderingRect);
	}

	struct FSetRenderingMode
	{
		int iMode;
		void operator () (CGraphicExpandedImageInstance* pInstance)
		{
			pInstance->SetRenderingMode(iMode);
		}
	};
	void CAniImageBox::SetRenderingMode(int iMode)
	{
		FSetRenderingMode setRenderingMode;
		setRenderingMode.iMode = iMode;
		for_each(m_ImageVector.begin(), m_ImageVector.end(), setRenderingMode);
	}

	void CAniImageBox::ResetFrame()
	{
		m_bycurIndex = 0;
	}

#ifdef ENABLE_FISHING_RENEWAL
	void CAniImageBox::SetRotation(float fRotation) const
	{
		for (uint8_t a = 0; a < m_ImageVector.size(); a++)
		{
			CGraphicExpandedImageInstance* pImage = m_ImageVector[a];
			if (pImage)
				pImage->SetRotation(fRotation);
		}
	}
#endif

	void CAniImageBox::OnUpdate()
	{
		++m_bycurDelay;
		if (m_bycurDelay < m_byDelay)
			return;

		m_bycurDelay = 0;

		++m_bycurIndex;
		if (m_bycurIndex >= m_ImageVector.size())
		{
			m_bycurIndex = 0;

			OnEndFrame();
		}
	}
	void CAniImageBox::OnRender()
	{
		if (m_bycurIndex < m_ImageVector.size())
		{
			CGraphicExpandedImageInstance* pImage = m_ImageVector[m_bycurIndex];
#ifdef ENABLE_USE_CLIP_MASK
			RECT mask;
			RECT* pMaskRect = GetClippingMaskRect(&mask) ? &mask : nullptr;
			pImage->Render(pMaskRect);
#else
			pImage->Render();
#endif
		}
	}

	struct FChangePosition
	{
		float fx, fy;
		void operator () (CGraphicExpandedImageInstance* pInstance)
		{
			pInstance->SetPosition(fx, fy);
		}
	};

	void CAniImageBox::OnChangePosition()
	{
		FChangePosition changePosition;
		changePosition.fx = m_rect.left;
		changePosition.fy = m_rect.top;
		for_each(m_ImageVector.begin(), m_ImageVector.end(), changePosition);
	}

	void CAniImageBox::OnEndFrame()
	{
		PyCallClassMemberFunc(m_poHandler, "OnEndFrame", BuildEmptyTuple());
	}

	struct FSetScale
	{
		float fx, fy;
		void operator () (CGraphicExpandedImageInstance* pInstance)
		{
			pInstance->SetScale(fx, fy);
		}
	};
	
	void CAniImageBox::SetScale(float fx, float fy)
	{
		FSetScale setScale;
		setScale.fx = fx;
		setScale.fy = fy;
		for_each(m_ImageVector.begin(), m_ImageVector.end(), setScale);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CButton::CButton(PyObject* ppyObject)
		: CWindow(ppyObject),
		m_pcurVisual(NULL),
		m_bEnable(TRUE),
		m_isPressed(FALSE),
		m_isFlash(FALSE),
#ifdef ENABLE_BUTTON_FLASH
		m_isButtonFlash(FALSE),
#endif
		m_isFlashEx(FALSE)
	{
		CWindow::AddFlag(CWindow::FLAG_NOT_CAPTURE);
#ifdef ENABLE_TREASURE_EVENT
		m_dwLastFlashTime = 0;
#endif
	}
	CButton::~CButton()
	{
	}

	BOOL CButton::SetUpVisual(const char * c_szFileName)
	{
		CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
		if (!pResource->IsType(CGraphicImage::Type()))
			return FALSE;

		m_upVisual.SetImagePointer(static_cast<CGraphicImage*>(pResource));
		if (m_upVisual.IsEmpty())
			return FALSE;

		SetSize(m_upVisual.GetWidth(), m_upVisual.GetHeight());
		//
		SetCurrentVisual(&m_upVisual);
		//

		return TRUE;
	}
	BOOL CButton::SetOverVisual(const char * c_szFileName)
	{
		CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
		if (!pResource->IsType(CGraphicImage::Type()))
			return FALSE;

		m_overVisual.SetImagePointer(static_cast<CGraphicImage*>(pResource));
		if (m_overVisual.IsEmpty())
			return FALSE;

		SetSize(m_overVisual.GetWidth(), m_overVisual.GetHeight());

		return TRUE;
	}
	BOOL CButton::SetDownVisual(const char * c_szFileName)
	{
		CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
		if (!pResource->IsType(CGraphicImage::Type()))
			return FALSE;

		m_downVisual.SetImagePointer(static_cast<CGraphicImage*>(pResource));
		if (m_downVisual.IsEmpty())
			return FALSE;

		SetSize(m_downVisual.GetWidth(), m_downVisual.GetHeight());

		return TRUE;
	}
	BOOL CButton::SetDisableVisual(const char * c_szFileName)
	{
		CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
		if (!pResource->IsType(CGraphicImage::Type()))
			return FALSE;

		m_disableVisual.SetImagePointer(static_cast<CGraphicImage*>(pResource));
		if (m_downVisual.IsEmpty())
			return FALSE;

		SetSize(m_disableVisual.GetWidth(), m_disableVisual.GetHeight());

		return TRUE;
	}

	const char* CButton::GetUpVisualFileName()
	{
		return m_upVisual.GetGraphicImagePointer()->GetFileName();
	}
	const char* CButton::GetOverVisualFileName()
	{
		return m_overVisual.GetGraphicImagePointer()->GetFileName();
	}
	const char* CButton::GetDownVisualFileName()
	{
		return m_downVisual.GetGraphicImagePointer()->GetFileName();
	}

	void CButton::Flash()
	{
		m_isFlash = true;
	}

#ifdef ENABLE_BUTTON_FLASH
	void CButton::EnableFlash()
	{
		m_isButtonFlash = true;
		if (!m_overVisual.IsEmpty())
			SetCurrentVisual(&m_overVisual);
	}

	void CButton::DisableFlash()
	{
		m_isButtonFlash = false;
		if (!m_upVisual.IsEmpty())
			SetCurrentVisual(&m_upVisual);
	}
#endif

	void CButton::FlashEx()
	{
		m_isFlashEx = TRUE;
	}

	void CButton::Enable()
	{
		SetUp();
		m_bEnable = TRUE;
	}

	void CButton::Disable()
	{
		m_bEnable = FALSE;
#ifdef ENABLE_BUTTON_FLASH
		m_isButtonFlash = false;
#endif
		if (!m_disableVisual.IsEmpty())
			SetCurrentVisual(&m_disableVisual);
	}

	BOOL CButton::IsDisable()
	{
		return m_bEnable;
	}



	void CButton::SetUp()
	{
		SetCurrentVisual(&m_upVisual);
		m_isPressed = FALSE;
	}
	void CButton::Up()
	{
		if (IsIn())
			SetCurrentVisual(&m_overVisual);
		else
			SetCurrentVisual(&m_upVisual);

		PyCallClassMemberFunc(m_poHandler, "CallEvent", BuildEmptyTuple());
	}
	void CButton::Over()
	{
		SetCurrentVisual(&m_overVisual);
	}
	void CButton::Down()
	{
		m_isPressed = TRUE;
		SetCurrentVisual(&m_downVisual);
		PyCallClassMemberFunc(m_poHandler, "DownEvent", BuildEmptyTuple());
	}

    void CButton::OnUpdate()
    {
#ifdef ENABLE_USE_CLIP_MASK
        if (!m_bEnableMask)
        {
            return;
        }

        if (!m_pMaskWindow)
        {
            return;
        }

        m_rMaskRect = m_pMaskWindow->GetRect();
#endif
    }

	void CButton::OnRender()
	{
		if (!IsShow())
			return;

		if (m_pcurVisual)
		{
#ifdef ENABLE_BUTTON_FLASH
			if (m_isButtonFlash) {
				if ((int(timeGetTime() / 500) % 2) && !IsIn() && !IsPressed()) {
					if (!m_overVisual.IsEmpty())
						SetCurrentVisual(&m_overVisual);
				} else {
					if (IsIn() && !IsPressed()) {
						if (!m_overVisual.IsEmpty())
							SetCurrentVisual(&m_overVisual);
					} else if (IsPressed()) {
						if (!m_downVisual.IsEmpty())
							SetCurrentVisual(&m_downVisual);
					} else {
						if (!m_upVisual.IsEmpty())
							SetCurrentVisual(&m_upVisual);
					}
				}
			}
#endif
#ifdef ENABLE_TREASURE_EVENT
			if (m_isFlash)
			{
				if (!IsIn())
				{
					if (!m_upVisual.IsEmpty() && !m_overVisual.IsEmpty() && m_upVisual.GetGraphicImagePointer() != m_overVisual.GetGraphicImagePointer())
					{
						DWORD now = timeGetTime();

						if (now - m_dwLastFlashTime >= 500) 
						{
							m_dwLastFlashTime = now;

							if(&m_upVisual == m_pcurVisual)
								SetCurrentVisual(&m_overVisual);
							else
								SetCurrentVisual(&m_upVisual);
						}
					}
					else
					{
						if (int(timeGetTime() / 500) % 2)
						{
							return;
						}
					}
				}
			}
#else
			if (m_isFlash)
				if (!IsIn())
					if (int(timeGetTime() / 500) % 2)
					{
						return;
					}
#endif

#ifdef ENABLE_USE_CLIP_MASK
			{
				RECT mask;
				RECT* pMaskRect = GetClippingMaskRect(&mask) ? &mask : nullptr;
				m_pcurVisual->Render(pMaskRect);
			}
#else
			m_pcurVisual->Render();
#endif
		}

		PyCallClassMemberFunc(m_poHandler, "OnRender", BuildEmptyTuple());
	}

	void CButton::OnChangePosition()
	{
		if (m_pcurVisual)
			m_pcurVisual->SetPosition(m_rect.left, m_rect.top);
	}

	BOOL CButton::OnMouseLeftButtonDown()
	{
		if (!IsEnable())
			return TRUE;

		m_isPressed = TRUE;
		Down();

		return TRUE;
	}
	BOOL CButton::OnMouseLeftButtonDoubleClick()
	{
		if (!IsEnable())
			return TRUE;

		OnMouseLeftButtonDown();

		return TRUE;
	}
	BOOL CButton::OnMouseLeftButtonUp()
	{
		if (!IsEnable())
			return TRUE;
		if (!IsPressed())
			return TRUE;

		m_isPressed = FALSE;
		Up();

		return TRUE;
	}

#ifdef ENABLE_BUTTON_TOOLTIP_RENEWAL
	void CButton::OnMouseOverIn()
	{
		if (IsEnable())
		{
			Over();
		}

		PyCallClassMemberFunc(m_poHandler, "ShowToolTip", BuildEmptyTuple());
#ifdef ENABLE_DETAILS_UI
		PyCallClassMemberFunc(m_poHandler, "OnMouseOverIn", BuildEmptyTuple());
#endif
	}
	void CButton::OnMouseOverOut()
	{
		if (IsEnable())
		{
			SetUp();
		}

		PyCallClassMemberFunc(m_poHandler, "HideToolTip", BuildEmptyTuple());
	}
#else
	void CButton::OnMouseOverIn()
	{
		if (!IsEnable())
			return;

		Over();
		PyCallClassMemberFunc(m_poHandler, "ShowToolTip", BuildEmptyTuple());
	}
	void CButton::OnMouseOverOut()
	{
		if (!IsEnable())
			return;

		SetUp();
		PyCallClassMemberFunc(m_poHandler, "HideToolTip", BuildEmptyTuple());
#ifdef ENABLE_DETAILS_UI
		PyCallClassMemberFunc(m_poHandler, "OnMouseOverOut", BuildEmptyTuple());
#endif
	}
#endif

#ifdef ENABLE_NEW_GAMEOPTION
	DWORD CButton::Type()
	{
		static DWORD s_dwType = GetCRC32("CButton", strlen("CButton"));
		return (s_dwType);
	}
	BOOL CButton::OnIsType(DWORD dwType)
	{
		if (CButton::Type() == dwType)
			return TRUE;
		return FALSE;
	}
	void CButton::SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom)
	{
		m_upVisual.SetRenderingRect(fLeft, fTop, fRight, fBottom);
		m_overVisual.SetRenderingRect(fLeft, fTop, fRight, fBottom);
		m_downVisual.SetRenderingRect(fLeft, fTop, fRight, fBottom);
	}
	void CButton::SetCurrentVisual(CGraphicExpandedImageInstance* pVisual)
#else
	void CButton::SetCurrentVisual(CGraphicImageInstance* pVisual)
#endif
	{
		m_pcurVisual = pVisual;
		m_pcurVisual->SetPosition(m_rect.left, m_rect.top);
	}

	BOOL CButton::IsEnable()
	{
		return m_bEnable;
	}

	BOOL CButton::IsPressed()
	{
		return m_isPressed;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CRadioButton::CRadioButton(PyObject* ppyObject) : CButton(ppyObject)
	{
	}
	CRadioButton::~CRadioButton()
	{
	}

#ifdef ENABLE_NEW_GAMEOPTION
	DWORD CRadioButton::Type()
	{
		static DWORD s_dwType = GetCRC32("CRadioButton", strlen("CRadioButton"));
		return (s_dwType);
	}
	BOOL CRadioButton::OnIsType(DWORD dwType)
	{
		if (CRadioButton::Type() == dwType)
			return TRUE;
		return FALSE;
	}
#endif

	BOOL CRadioButton::OnMouseLeftButtonDown()
	{
		if (!IsEnable())
			return TRUE;

		if (!m_isPressed)
		{
			Down();
			PyCallClassMemberFunc(m_poHandler, "CallEvent", BuildEmptyTuple());
		}

		return TRUE;
	}
	BOOL CRadioButton::OnMouseLeftButtonUp()
	{
		return TRUE;
	}
	void CRadioButton::OnMouseOverIn()
	{
		if (!IsEnable())
			return;

		if (!m_isPressed)
		{
			SetCurrentVisual(&m_overVisual);
		}

		PyCallClassMemberFunc(m_poHandler, "ShowToolTip", BuildEmptyTuple());
	}
	void CRadioButton::OnMouseOverOut()
	{
		if (!IsEnable())
			return;

		if (!m_isPressed)
		{
			SetCurrentVisual(&m_upVisual);
		}

		PyCallClassMemberFunc(m_poHandler, "HideToolTip", BuildEmptyTuple());
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CToggleButton::CToggleButton(PyObject* ppyObject) : CButton(ppyObject)
	{
	}
	CToggleButton::~CToggleButton()
	{
	}

#ifdef ENABLE_NEW_GAMEOPTION
	DWORD CToggleButton::Type()
	{
		static DWORD s_dwType = GetCRC32("CToggleButton", strlen("CToggleButton"));
		return (s_dwType);
	}
	BOOL CToggleButton::OnIsType(DWORD dwType)
	{
		if (CToggleButton::Type() == dwType)
			return TRUE;
		return FALSE;
	}
#endif

	BOOL CToggleButton::OnMouseLeftButtonDown()
	{
		if (!IsEnable())
			return TRUE;

		if (m_isPressed)
		{
			SetUp();
			if (IsIn())
				SetCurrentVisual(&m_overVisual);
			else
				SetCurrentVisual(&m_upVisual);
			PyCallClassMemberFunc(m_poHandler, "OnToggleUp", BuildEmptyTuple());
		}
		else
		{
			Down();
			PyCallClassMemberFunc(m_poHandler, "OnToggleDown", BuildEmptyTuple());
		}

		return TRUE;
	}
	BOOL CToggleButton::OnMouseLeftButtonUp()
	{
		return TRUE;
	}

	void CToggleButton::OnMouseOverIn()
	{
		if (!IsEnable())
			return;

		if (!m_isPressed)
		{
			SetCurrentVisual(&m_overVisual);
		}

		PyCallClassMemberFunc(m_poHandler, "ShowToolTip", BuildEmptyTuple());
#ifdef ENABLE_DETAILS_UI
		PyCallClassMemberFunc(m_poHandler, "OnMouseOverIn", BuildEmptyTuple());
#endif
	}
	void CToggleButton::OnMouseOverOut()
	{
		if (!IsEnable())
			return;

		if (!m_isPressed)
		{
			SetCurrentVisual(&m_upVisual);
		}

		PyCallClassMemberFunc(m_poHandler, "HideToolTip", BuildEmptyTuple());
#ifdef ENABLE_DETAILS_UI
		PyCallClassMemberFunc(m_poHandler, "OnMouseOverOut", BuildEmptyTuple());
#endif
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CDragButton::CDragButton(PyObject* ppyObject) : CButton(ppyObject)
	{
		CWindow::RemoveFlag(CWindow::FLAG_NOT_CAPTURE);
		m_restrictArea.left = 0;
		m_restrictArea.top = 0;
		m_restrictArea.right = CWindowManager::Instance().GetScreenWidth();
		m_restrictArea.bottom = CWindowManager::Instance().GetScreenHeight();
	}
	CDragButton::~CDragButton()
	{
	}

	void CDragButton::SetRestrictMovementArea(int ix, int iy, int iwidth, int iheight)
	{
		m_restrictArea.left = ix;
		m_restrictArea.top = iy;
		m_restrictArea.right = ix + iwidth;
		m_restrictArea.bottom = iy + iheight;
	}

	void CDragButton::OnChangePosition()
	{
		m_x = max(m_x, m_restrictArea.left);
		m_y = max(m_y, m_restrictArea.top);
		m_x = min(m_x, max(0, m_restrictArea.right - m_lWidth));
		m_y = min(m_y, max(0, m_restrictArea.bottom - m_lHeight));

		m_rect.left = m_x;
		m_rect.top = m_y;

		if (m_pParent)
		{
			const RECT& c_rRect = m_pParent->GetRect();
			m_rect.left += c_rRect.left;
			m_rect.top += c_rRect.top;
		}

		m_rect.right = m_rect.left + m_lWidth;
		m_rect.bottom = m_rect.top + m_lHeight;

		for (auto * pChild : m_pChildList) pChild->UpdateRect();

		if (m_pcurVisual)
			m_pcurVisual->SetPosition(m_rect.left, m_rect.top);

		if (IsPressed())
			PyCallClassMemberFunc(m_poHandler, "OnMove", BuildEmptyTuple());
	}

	void CDragButton::OnMouseOverIn()
	{
		if (!IsEnable())

			return;

		CButton::OnMouseOverIn();
		PyCallClassMemberFunc(m_poHandler, "OnMouseOverIn", BuildEmptyTuple());
	}

	void CDragButton::OnMouseOverOut()
	{
		if (!IsEnable())
			return;

		CButton::OnMouseOverIn();
		PyCallClassMemberFunc(m_poHandler, "OnMouseOverOut", BuildEmptyTuple());
	}
};


