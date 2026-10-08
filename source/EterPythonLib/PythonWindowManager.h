#pragma once
#include "../UserInterface/Locale_inc.h"

namespace UI 
{
	class CWindow;

	class CWindowManager : public CSingleton<CWindowManager>
	{
		public:
			typedef std::map<std::string, CWindow *> TLayerContainer;
			typedef std::list<CWindow *> TWindowContainer;
			typedef std::map<int, CWindow *> TKeyCaptureWindowMap;

		public:
			CWindowManager();
			virtual ~CWindowManager();

			void		Destroy();

			float		GetAspect();
			void		SetScreenSize(long lWidth, long lHeight);
			void		SetResolution(int hres, int vres);

			void		GetResolution(long & rx, long & ry)
			{
				rx=m_iHres;
				ry=m_iVres;
			}

			void		SetMouseHandler(PyObject * poMouseHandler);
			long		GetScreenWidth()		{ return m_lWidth; }
			long		GetScreenHeight()		{ return m_lHeight; }
			void		GetMousePosition(long & rx, long & ry);
			BOOL		IsDragging();

			CWindow *	GetLockWindow()		{ return m_pLockWindow; }
			CWindow *	GetPointWindow()	{ return m_pPointWindow; }
			bool		IsFocus()			{ return (m_pActiveWindow || m_pLockWindow); }
			bool		IsFocusWindow(CWindow * pWindow)	{ return pWindow == m_pActiveWindow; }

			void		SetParent(CWindow * pWindow, CWindow * pParentWindow);
			void		SetPickAlways(CWindow * pWindow);

			void		DelPickAlways(CWindow * pWindow);

			enum
			{		
				WT_NORMAL,
				WT_SLOT,
				WT_GRIDSLOT,
				WT_TEXTLINE,
				WT_MARKBOX,
				WT_IMAGEBOX,
				WT_EXP_IMAGEBOX,
				WT_ANI_IMAGEBOX,
				WT_BUTTON,
				WT_RATIOBUTTON,
				WT_TOGGLEBUTTON,
				WT_DRAGBUTTON,
				WT_BOX,
				WT_BAR,
				WT_LINE,
				WT_BAR3D,
				WT_NUMLINE,				
			};

			CWindow *	RegisterWindow(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterTypeWindow(PyObject * po, DWORD dwWndType, const char * c_szLayer);

			CWindow *	RegisterSlotWindow(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterGridSlotWindow(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterTextLine(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterMarkBox(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterImageBox(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterExpandedImageBox(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterAniImageBox(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterButton(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterRadioButton(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterToggleButton(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterDragButton(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterBox(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterRenderTarget(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterBar(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterLine(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterBar3D(PyObject * po, const char * c_szLayer);
			CWindow *	RegisterNumberLine(PyObject * po, const char * c_szLayer);
#ifdef ENABLE_INGAME_WIKI
			CWindow *	RegisterWikiRenderTarget(PyObject * po, const char * c_szLayer);
#endif

			void		DestroyWindow(CWindow * pWin);
			void		NotifyDestroyWindow(CWindow * pWindow);

			// Attaching Icon
			BOOL		IsAttaching();
			DWORD		GetAttachingType();
			DWORD		GetAttachingIndex();
			DWORD		GetAttachingSlotNumber();
			DWORD		GetAttachingRealSlotNumber();
			void		GetAttachingIconSize(BYTE * pbyWidth, BYTE * pbyHeight);
			void		AttachIcon(DWORD dwType, DWORD dwIndex, DWORD dwSlotNumber, BYTE byWidth, BYTE byHeight);
			void		DeattachIcon();
			void		SetAttachingFlag(BOOL bFlag);
			void		SetAttachingRealSlotNumber(DWORD dwRealslotNumber);
			// Attaching Icon

#ifdef ENABLE_FISH_EVENT_SYSTEM
			void		SetDisableDeattach(bool bFlag) { m_bIsDisableDettach = bFlag; }
			bool		IsDisableDeattach() { return m_bIsDisableDettach; }
#endif

			void		OnceIgnoreMouseLeftButtonUpEvent();
			void		LockWindow(CWindow * pWin);
			void		UnlockWindow();

			void		ActivateWindow(CWindow * pWin);
			void		DeactivateWindow();
			CWindow *	GetActivateWindow();
			void		SetTop(CWindow * pWin);
			void		SetTopUIWindow();
			void		ResetCapture();

			void		Update();
			void		Render();

			void		RunMouseMove(long x, long y);
			void		RunMouseLeftButtonDown(long x, long y);
			void		RunMouseLeftButtonUp(long x, long y);
			void		RunMouseLeftButtonDoubleClick(long x, long y);
			void		RunMouseRightButtonDown(long x, long y);
			void		RunMouseRightButtonUp(long x, long y);
			void		RunMouseRightButtonDoubleClick(long x, long y);
			void		RunMouseMiddleButtonDown(long x, long y);
			void		RunMouseMiddleButtonUp(long x, long y);

#ifdef ENABLE_MOUSEWHEEL_EVENT
			bool		RunMouseWheelEvent(long nLen);
			bool		RunMouseWheelScroll(long x, long y , short wDelta);
#endif

			void		RunIMEUpdate();
			void		RunIMETabEvent();
			void		RunIMEReturnEvent();
			void		RunIMEKeyDown(int vkey);
			void		RunChangeCodePage();
			void		RunOpenCandidate();
			void		RunCloseCandidate();
			void		RunOpenReading();
			void		RunCloseReading();

			void		RunKeyDown(int vkey);
			void		RunKeyUp(int vkey);
			void		RunPressEscapeKey();
			void		RunPressExitKey();

			bool		IsUIWindowAt(long x, long y);
			bool		IsGameWorldAt(long x, long y);

			// Multi-touch (mobile): a 2nd finger pressing/dragging a UI window while the 1st finger
			// still holds another one (e.g. attack button held + skill slot / slider touched).
			void		RunSecondaryTouchDown(long x, long y);
			void		RunSecondaryTouchMove(long x, long y);
			void		RunSecondaryTouchUp(long x, long y);

		private:
			void		SetMousePosition(long x, long y);
			CWindow *	__PickWindow(long x, long y);
			void		__RunCaptureDrag(CWindow * pWin, long lDragX, long lDragY);
			
			CWindow *	__NewWindow(PyObject * po, DWORD dwWndType);
			void		__ClearReserveDeleteWindowList();

		private:
			long					m_lWidth;
			long					m_lHeight;

			int						m_iVres;
			int						m_iHres;

			long					m_lMouseX, m_lMouseY;
			long					m_lDragX, m_lDragY;
			long					m_lPickedX, m_lPickedY;

			BOOL					m_bOnceIgnoreMouseLeftButtonUpEventFlag;
			int						m_iIgnoreEndTime;

			// Attaching Icon
			PyObject *				m_poMouseHandler;
			BOOL					m_bAttachingFlag;
			DWORD					m_dwAttachingType;
			DWORD					m_dwAttachingIndex;
			DWORD					m_dwAttachingSlotNumber;
			DWORD					m_dwAttachingRealSlotNumber;
			BYTE					m_byAttachingIconWidth;
			BYTE					m_byAttachingIconHeight;
			// Attaching Icon

#ifdef ENABLE_FISH_EVENT_SYSTEM
			bool					m_bIsDisableDettach;
#endif

			CWindow	*				m_pActiveWindow;
			TWindowContainer		m_ActiveWindowList;
			CWindow *				m_pLockWindow;
			TWindowContainer		m_LockWindowList;
			CWindow	*				m_pPointWindow;
			CWindow	*				m_pLeftCaptureWindow;
			CWindow	*				m_pSecondCaptureWindow;
			long					m_lSecondDragX, m_lSecondDragY;
			CWindow	*				m_pRightCaptureWindow;
			CWindow *				m_pMiddleCaptureWindow;
			TKeyCaptureWindowMap	m_KeyCaptureWindowMap;
			TWindowContainer		m_ReserveDeleteWindowList;
			TWindowContainer		m_PickAlwaysWindowList;

			CWindow *				m_pRootWindow;
			TWindowContainer		m_LayerWindowList;
			TLayerContainer			m_LayerWindowMap;

#ifdef ENABLE_WHITE_DRAGON
		public:
			void SetLockMouseAndKeyboard(bool bLock) { m_bLockMouseAndKeyboard = bLock; }
			bool IsMouseAndKeyboardLocked() const { return m_bLockMouseAndKeyboard; }
		private:
			bool m_bLockMouseAndKeyboard;
#endif
	};

	PyObject * BuildEmptyTuple();
};
