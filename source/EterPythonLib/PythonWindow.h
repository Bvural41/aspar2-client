#pragma once

#include "../EterBase/Utils.h"
#include <cstdint>
#include "../UserInterface/Locale_inc.h"

namespace UI
{
	class CWindow
	{
	public:
		typedef std::list<CWindow*> TWindowContainer;

		static DWORD Type();
		BOOL IsType(DWORD dwType);

		enum EHorizontalAlign
		{
			HORIZONTAL_ALIGN_LEFT = 0,
			HORIZONTAL_ALIGN_CENTER = 1,
			HORIZONTAL_ALIGN_RIGHT = 2,
		};

		enum EVerticalAlign
		{
			VERTICAL_ALIGN_TOP = 0,
			VERTICAL_ALIGN_CENTER = 1,
			VERTICAL_ALIGN_BOTTOM = 2,
		};

		enum EFlags
		{
			FLAG_MOVABLE = (1 << 0),	// 움직일 수 있는 창
			FLAG_LIMIT = (1 << 1),	// 창이 화면을 벗어나지 않음
			FLAG_SNAP = (1 << 2),	// 스냅 될 수 있는 창
			FLAG_DRAGABLE = (1 << 3),
			FLAG_ATTACH = (1 << 4),	// 완전히 부모에 붙어 있는 창 (For Drag / ex. ScriptWindow)
			FLAG_RESTRICT_X = (1 << 5),	// 좌우 이동 제한
			FLAG_RESTRICT_Y = (1 << 6),	// 상하 이동 제한
			FLAG_NOT_CAPTURE = (1 << 7),
			FLAG_FLOAT = (1 << 8),	// 공중에 떠있어서 순서 재배치가 되는 창
			FLAG_NOT_PICK = (1 << 9),	// 마우스에 의해 Pick되지 않는 창
			FLAG_IGNORE_SIZE = (1 << 10),
			FLAG_RTL = (1 << 11),	// Right-to-left
			FLAG_ALPHA_SENSITIVE	= (1 << 12),	// flag for activating alpha sensitive
			FLAG_REMOVE_LIMIT	= (1 << 13),	// flag for activating remove limit
			FLAG_IS_BOARD_WITHOUT_ALPHA	= (1 << 14),
		};

		enum WindowTypes // window type flags to recognize expanded_image class
		{
			WINDOW_TYPE_WINDOW,
			WINDOW_TYPE_EX_IMAGE,

			WINDOW_TYPE_MAX_NUM
		};

	public:
		CWindow(PyObject* ppyObject);
		virtual ~CWindow();

		void			AddChild(CWindow* pWin);

		void			Clear();
		void			DestroyHandle();
		void			Update();
		void			Render();

		void			SetName(const char* c_szName);
		const char* GetName() { return m_strName.c_str(); }
		void			SetSize(long width, long height);
		long			GetWidth() { return m_lWidth; }
		long			GetHeight() { return m_lHeight; }
		void			SetHorizontalAlign(DWORD dwAlign);
		void			SetVerticalAlign(DWORD dwAlign);
		void			SetPosition(long x, long y);
		void			GetPosition(long* plx, long* ply);
		long			GetPositionX(void) const { return m_x; }
		long			GetPositionY(void) const { return m_y; }
		RECT& GetRect() { return m_rect; }
		void			GetLocalPosition(long& rlx, long& rly);
		void			GetMouseLocalPosition(long& rlx, long& rly);
		long			UpdateRect();

		RECT& GetLimitBias() { return m_limitBiasRect; }
		void			SetLimitBias(long l, long r, long t, long b) { m_limitBiasRect.left = l, m_limitBiasRect.right = r, m_limitBiasRect.top = t, m_limitBiasRect.bottom = b; }

		void			Show();
		void			Hide();
		bool			IsShow() { return m_bShow; }
		bool			IsRendering();

		bool			HasParent() { return m_pParent ? true : false; }
		bool			HasChild() { return m_pChildList.empty() ? false : true; }
		int				GetChildCount() { return m_pChildList.size(); }

		void			IsTransparentOnPixel(long* x, long* y, bool* ret);
		void			HaveSomeChildOnOnPixel(long* x, long* y, bool* ret);

		CWindow* GetRoot();
		CWindow* GetParent() const;
		bool			IsChild(CWindow* pWin);
		void			DeleteChild(CWindow* pWin);
		void			SetTop(CWindow* pWin);

		bool			IsIn(long x, long y);
		bool			IsIn();
		CWindow* PickWindow(long x, long y);
		CWindow* PickTopWindow(long x, long y);	// NOTE : Children으로 내려가지 않고 상위에서만 
														//        체크 하는 특화된 함수

		void			__RemoveReserveChildren();

		void			AddFlag(DWORD flag) { SET_BIT(m_dwFlag, flag); }
		void			RemoveFlag(DWORD flag) { REMOVE_BIT(m_dwFlag, flag); }
		bool			IsFlag(DWORD flag) { return (m_dwFlag & flag) ? true : false; }
		/////////////////////////////////////

		virtual void	OnRender();
		virtual void	OnUpdate();
		virtual void	OnChangePosition() {}

		virtual void	OnSetFocus();
		virtual void	OnKillFocus();

		virtual void	OnMouseDrag(long lx, long ly);
		virtual void	OnMouseOverIn();
		virtual void	OnMouseOverOut();
		virtual void	OnMouseOver();
		virtual void	OnDrop();
		virtual void	OnTop();
		virtual void	OnIMEUpdate();

		virtual void	OnMoveWindow(long x, long y);

		///////////////////////////////////////

		BOOL			RunIMETabEvent();
		BOOL			RunIMEReturnEvent();
		BOOL			RunIMEKeyDownEvent(int ikey);

		CWindow* RunKeyDownEvent(int ikey);
		BOOL			RunKeyUpEvent(int ikey);
		BOOL			RunPressEscapeKeyEvent();
		BOOL			RunPressExitKeyEvent();

		virtual BOOL	OnIMETabEvent();
		virtual BOOL	OnIMEReturnEvent();
		virtual BOOL	OnIMEKeyDownEvent(int ikey);

		virtual BOOL	OnIMEChangeCodePage();
		virtual BOOL	OnIMEOpenCandidateListEvent();
		virtual BOOL	OnIMECloseCandidateListEvent();
		virtual BOOL	OnIMEOpenReadingWndEvent();
		virtual BOOL	OnIMECloseReadingWndEvent();

		virtual BOOL	OnMouseLeftButtonDown();
		virtual BOOL	OnMouseLeftButtonUp();
		virtual BOOL	OnMouseLeftButtonDoubleClick();
		virtual BOOL	OnMouseRightButtonDown();
		virtual BOOL	OnMouseRightButtonUp();
		virtual BOOL	OnMouseRightButtonDoubleClick();
		virtual BOOL	OnMouseMiddleButtonDown();
		virtual BOOL	OnMouseMiddleButtonUp();
		virtual BOOL	RunMouseWheelEvent(long nLen);
#ifdef ENABLE_MOUSEWHEEL_EVENT
		virtual BOOL	OnMouseWheelScroll(short wDelta);
		virtual void	SetScrollable();
#endif
		virtual BOOL	OnKeyDown(int ikey);
		virtual BOOL	OnKeyUp(int ikey);
		virtual BOOL	OnPressEscapeKey();
		virtual BOOL	OnPressExitKey();
		///////////////////////////////////////

		virtual void	SetColor(DWORD dwColor) {}
		virtual BOOL	OnIsType(DWORD dwType);
		/////////////////////////////////////

		virtual BOOL	IsWindow() { return TRUE; }
		/////////////////////////////////////

#ifdef ENABLE_USE_CLIP_MASK
		virtual void SetClippingMaskRect(const RECT& rMask);
		virtual void SetClippingMaskWindow(CWindow* pMaskWindow);
		bool GetClippingMaskRect(RECT* pOutRect);
#endif

#ifdef ENABLE_INGAME_WIKI
	public:
		void			SetInsideRender(BOOL flag);
		void			GetRenderBox(RECT* box);
		void			UpdateTextLineRenderBox();
		void			UpdateRenderBox();
		void			UpdateRenderBoxRecursive();
		virtual void	OnUpdateRenderBox() {}
		virtual void	OnAfterRender() {}
		virtual void	iSetRenderingRect(int iLeft, int iTop, int iRight, int iBottom);
		virtual void	SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom);
		virtual int		GetRenderingWidth();
		virtual int		GetRenderingHeight();
		void			ResetRenderingRect(bool bCallEvent = true);
#endif

	protected:
		std::string			m_strName;

		EHorizontalAlign	m_HorizontalAlign;
		EVerticalAlign		m_VerticalAlign;
		long				m_x, m_y;				// X,Y 상대좌표
		long				m_lWidth, m_lHeight;	// 크기
		RECT				m_rect;					// Global 좌표
		RECT				m_limitBiasRect;		// limit bias 값

		bool				m_bMovable;
		bool				m_bShow;

		DWORD				m_dwFlag;

		PyObject* m_poHandler;

		CWindow* m_pParent;
		TWindowContainer	m_pChildList;

		BOOL				m_isUpdatingChildren;
		TWindowContainer	m_pReserveChildList;

#ifdef ENABLE_MOUSEWHEEL_EVENT
		bool				m_bIsScrollable;
#endif

#ifdef ENABLE_INGAME_WIKI
		BOOL				m_isInsideRender;
		RECT				m_renderBox;
		RECT				m_renderingRect;
#endif

		BYTE				m_windowType; // to recognize window type

#ifdef ENABLE_USE_CLIP_MASK
		bool m_bEnableMask;
		CWindow* m_pMaskWindow;
		RECT m_rMaskRect;
#endif

#ifdef _DEBUG
	public:
		DWORD				DEBUG_dwCounter;
#endif
	};

	class CLayer : public CWindow
	{
	public:
		CLayer(PyObject* ppyObject) : CWindow(ppyObject) {}
		virtual ~CLayer() {}

		BOOL IsWindow() { return FALSE; }
	};

	class CBox : public CWindow
	{
	public:
		CBox(PyObject* ppyObject);
		virtual ~CBox();

		void SetColor(DWORD dwColor);

	protected:
		void OnRender();

	protected:
		DWORD m_dwColor;
	};

	class CUiRenderTarget : public CWindow
	{
	public:
		CUiRenderTarget(PyObject * ppyObject);
		virtual ~CUiRenderTarget();

		bool SetRenderTarget(uint8_t index);

	protected:
		DWORD m_dwIndex;
		void OnRender();
	};

	class CBar : public CWindow
	{
	public:
		CBar(PyObject* ppyObject);
		virtual ~CBar();

		void SetColor(DWORD dwColor);

	protected:
		void OnRender();

	protected:
		DWORD m_dwColor;
	};

	class CLine : public CWindow
	{
	public:
		CLine(PyObject* ppyObject);
		virtual ~CLine();

		void SetColor(DWORD dwColor);

	protected:
		void OnRender();

	protected:
		DWORD m_dwColor;
	};

	class CBar3D : public CWindow
	{
	public:
		static DWORD Type();

	public:
		CBar3D(PyObject* ppyObject);
		virtual ~CBar3D();

		void SetColor(DWORD dwLeft, DWORD dwRight, DWORD dwCenter);

	protected:
		void OnRender();

	protected:
		DWORD m_dwLeftColor;
		DWORD m_dwRightColor;
		DWORD m_dwCenterColor;
	};

#ifdef ENABLE_INGAME_WIKI
	class CUiWikiRenderTarget : public CWindow
	{
		public:
			CUiWikiRenderTarget(PyObject * ppyObject);
			virtual ~CUiWikiRenderTarget();
		
		public:
			bool	SetWikiRenderTargetModule(int iRenderTargetModule);
		
		protected:
			void	OnUpdateRenderBox();
			void	OnRender();
		
		protected:
			int		m_dwIndex;
	};
#endif

	// Text
	class CTextLine : public CWindow
	{
	public:
		CTextLine(PyObject* ppyObject);
		virtual ~CTextLine();

		void SetMax(int iMax);
#ifdef ENABLE_MULTI_TEXTLINE
		void SetEnterToken(bool v) { m_TextInstance.SetEnterToken(v); }
#endif
		void SetHorizontalAlign(int iType);
		void SetVerticalAlign(int iType);
		void SetSecret(BOOL bFlag);
		void SetOutline(BOOL bFlag);
		void SetFeather(BOOL bFlag);
		void SetMultiLine(BOOL bFlag);
		void SetFontName(const char* c_szFontName);
		void SetFontColor(DWORD dwColor);
		void SetLimitWidth(float fWidth);

		void ShowCursor();
		void HideCursor();
		int GetCursorPosition();

#ifdef ENABLE_INGAME_WIKI
		void SetFixedRenderPos(WORD startPos, WORD endPos) { m_TextInstance.SetFixedRenderPos(startPos, endPos); }
		void GetRenderPositions(WORD& startPos, WORD& endPos) { m_TextInstance.GetRenderPositions(startPos, endPos); }
		bool IsShowCursor() { return m_TextInstance.IsShowCursor(); }
		int GetRenderingWidth();
		int GetRenderingHeight();
		void OnSetRenderingRect();
		void OnUpdateRenderBox() {
			if (m_isInsideRender && m_pParent) {
				UpdateTextLineRenderBox();
			} else {
				memset(&m_renderBox, 0, sizeof(m_renderBox));
			}
			m_TextInstance.SetRenderBox(m_renderBox);
		}
#endif

		void SetText(const char* c_szText);
		const char* GetText();

		void GetTextSize(int* pnWidth, int* pnHeight);
		WORD GetTextLineCount();
		WORD GetLineHeight();
#ifdef ENABLE_SUNG_MAHI_TOWER
		void GetCharSize(short* sWidth);
#endif
		void SetLineHeight(int iHeight);

	protected:
		void OnUpdate();
		void OnRender();
		void OnChangePosition();

		virtual void OnSetText(const char* c_szText);

	protected:
		CGraphicTextInstance m_TextInstance;
	};

	class CNumberLine : public CWindow
	{
	public:
		CNumberLine(PyObject* ppyObject);
		CNumberLine(CWindow* pParent);
		virtual ~CNumberLine();

		void SetPath(const char* c_szPath);
		void SetHorizontalAlign(int iType);
		void SetNumber(const char* c_szNumber);

	protected:
		void ClearNumber();
		void OnRender();
		void OnChangePosition();

	protected:
		std::string m_strPath;
		std::string m_strNumber;
		std::vector<CGraphicImageInstance*> m_ImageInstanceVector;

		int m_iHorizontalAlign;
		DWORD m_dwWidthSummary;
#ifdef ENABLE_SLOT_TEXT_SIZE
		bool m_bIsZoomed;
#endif
	};

	// Image
	class CImageBox : public CWindow
	{
	public:
		CImageBox(PyObject* ppyObject);
		virtual ~CImageBox();

		BOOL LoadImage(const char* c_szFileName);
		void SetDiffuseColor(float fr, float fg, float fb, float fa);
		void SetScale(float sx, float sy);

		int GetWidth();
		int GetHeight();

		void SetCoolTime(float fCoolTime, float fElapsedTime);
		void SetCoolTimeStart(float fCoolTimeStart);
		BOOL IsInCoolTime();

		// void SetCoolTime(float fCoolTime);
		// void SetCoolTimeStart(float fCoolTimeStart);

#ifdef ENABLE_12ZI
		void SetCoolTimeImageBox(float CT);
		void SetStartCoolTimeImageBox(float SCT);
#endif

#ifdef ENABLE_INGAME_WIKI
		void UnloadImage()
		{
			OnDestroyInstance();
			SetSize(GetWidth(), GetHeight());
			UpdateRect();
		}
#endif

	protected:
		virtual void OnCreateInstance();
		virtual void OnDestroyInstance();

		virtual void OnUpdate();
		virtual void OnRender();
		void OnChangePosition();

	protected:
		CGraphicImageInstance* m_pImageInstance;
		// float m_fCoolTime;
		// float m_fCoolTimeStart;
#ifdef ENABLE_12ZI
		float fCoolTime;
		float fStartCoolTime;
#endif
		float m_fCoolTime;
		float m_fCoolElapsedTime;
		float m_fCoolTimeStart;
	};
	class CMarkBox : public CWindow
	{
	public:
		CMarkBox(PyObject* ppyObject);
		virtual ~CMarkBox();

		void LoadImage(const char* c_szFilename);
		void SetDiffuseColor(float fr, float fg, float fb, float fa);
		void SetIndex(UINT uIndex);
		void SetScale(FLOAT fScale);

	protected:
		virtual void OnCreateInstance();
		virtual void OnDestroyInstance();

		virtual void OnUpdate();
		virtual void OnRender();
		void OnChangePosition();
	protected:
		CGraphicMarkInstance* m_pMarkInstance;
	};
	class CExpandedImageBox : public CImageBox
	{
	public:
		static DWORD Type();

	public:
		CExpandedImageBox(PyObject* ppyObject);
		virtual ~CExpandedImageBox();

		void SetScale(float fx, float fy);
		void SetOrigin(float fx, float fy);
		void SetRotation(float fRotation);
		void SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom);
		void SetRenderingMode(int iMode);
		void SetImageClipRect(float fLeft, float fTop, float fRight, float fBottom, bool bIsVertical = false);

		D3DXCOLOR GetPixelColor(int x, int y) { if (m_pImageInstance) return m_pImageInstance->GetPixelColor(x, y); else return D3DXCOLOR(0, 0, 0, 0); }

#ifdef ENABLE_INGAME_WIKI
		int GetRenderingWidth();
		int GetRenderingHeight();
		void OnSetRenderingRect();
		void SetExpandedRenderingRect(float fLeftTop, float fLeftBottom, float fTopLeft, float fTopRight, float fRightTop, float fRightBottom, float fBottomLeft, float fBottomRight);
		void SetTextureRenderingRect(float fLeft, float fTop, float fRight, float fBottom);
		void OnUpdateRenderBox();
#endif


	protected:
		void OnCreateInstance();
		void OnDestroyInstance();

		virtual void OnUpdate();
		virtual void OnRender();

		BOOL OnIsType(DWORD dwType);
	};
	class CAniImageBox : public CWindow
	{
	public:
		static DWORD Type();

	public:
		CAniImageBox(PyObject* ppyObject);
		virtual ~CAniImageBox();

		void SetDelay(int iDelay);
#if defined(ENABLE_SASH_SYSTEM) || defined(ENABLE_CHANGELOOK_SYSTEM)
		void AppendImage(const char* c_szFileName, float r = 1.0, float g = 1.0, float b = 1.0, float a = 1.0);
#else
		void AppendImage(const char* c_szFileName);
#endif
		void SetScale(float fx, float fy);
		void SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom);
		void SetRenderingMode(int iMode);

		void ResetFrame();

#ifdef ENABLE_FISHING_RENEWAL
		void SetRotation(float fRotation) const;
#endif

	protected:
		void OnUpdate();
		void OnRender();
		void OnChangePosition();
		virtual void OnEndFrame();

		BOOL OnIsType(DWORD dwType);

	protected:
		BYTE m_bycurDelay;
		BYTE m_byDelay;
		BYTE m_bycurIndex;
		std::vector<CGraphicExpandedImageInstance*> m_ImageVector;
	};

	// Button
	class CButton : public CWindow
	{
	public:
		CButton(PyObject* ppyObject);
		virtual ~CButton();

		BOOL SetUpVisual(const char* c_szFileName);
		BOOL SetOverVisual(const char* c_szFileName);
		BOOL SetDownVisual(const char* c_szFileName);
		BOOL SetDisableVisual(const char* c_szFileName);

#ifdef ENABLE_NEW_GAMEOPTION
		static DWORD Type();
		void SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom);
#endif

		const char* GetUpVisualFileName();
		const char* GetOverVisualFileName();
		const char* GetDownVisualFileName();
		// void SetScale(float fx, float fy);

		void Flash();
#ifdef ENABLE_BUTTON_FLASH
		void EnableFlash();
		void DisableFlash();
#endif
		void FlashEx();
		void Enable();
		void Disable();

		void SetUp();
		void Up();
		void Over();
		void Down();

		BOOL IsDisable();
		BOOL IsPressed();

	protected:
		void OnUpdate();
		void OnRender();
		void OnChangePosition();

		BOOL OnMouseLeftButtonDown();
		BOOL OnMouseLeftButtonDoubleClick();
		BOOL OnMouseLeftButtonUp();
		void OnMouseOverIn();
		void OnMouseOverOut();

		BOOL IsEnable();

#ifdef ENABLE_NEW_GAMEOPTION
		void SetCurrentVisual(CGraphicExpandedImageInstance* pVisual);
		BOOL OnIsType(DWORD dwType);
#else
		void SetCurrentVisual(CGraphicImageInstance* pVisual);
#endif

	protected:
		BOOL m_bEnable;
		BOOL m_isPressed;
		BOOL m_isFlash;
#ifdef ENABLE_TREASURE_EVENT
		DWORD m_dwLastFlashTime;
#endif
#ifdef ENABLE_BUTTON_FLASH
		BOOL m_isButtonFlash;
#endif
		BOOL m_isFlashEx;
#ifdef ENABLE_NEW_GAMEOPTION
		CGraphicExpandedImageInstance* m_pcurVisual;
		CGraphicExpandedImageInstance m_upVisual;
		CGraphicExpandedImageInstance m_overVisual;
		CGraphicExpandedImageInstance m_downVisual;
		CGraphicExpandedImageInstance m_disableVisual;
#else
		CGraphicImageInstance* m_pcurVisual;
		CGraphicImageInstance m_upVisual;
		CGraphicImageInstance m_overVisual;
		CGraphicImageInstance m_downVisual;
		CGraphicImageInstance m_disableVisual;
#endif
	};
	class CRadioButton : public CButton
	{
	public:
#ifdef ENABLE_NEW_GAMEOPTION
		static DWORD Type();
#endif
		CRadioButton(PyObject* ppyObject);
		virtual ~CRadioButton();

	protected:
		BOOL OnMouseLeftButtonDown();
		BOOL OnMouseLeftButtonUp();
		void OnMouseOverIn();
		void OnMouseOverOut();
#ifdef ENABLE_NEW_GAMEOPTION
		BOOL OnIsType(DWORD dwType);
#endif
	};
	class CToggleButton : public CButton
	{
	public:
#ifdef ENABLE_NEW_GAMEOPTION
		static DWORD Type();
#endif
		CToggleButton(PyObject* ppyObject);
		virtual ~CToggleButton();

	protected:
		BOOL OnMouseLeftButtonDown();
		BOOL OnMouseLeftButtonUp();
		void OnMouseOverIn();
		void OnMouseOverOut();
#ifdef ENABLE_NEW_GAMEOPTION
		BOOL OnIsType(DWORD dwType);
#endif
	};
	class CDragButton : public CButton
	{
	public:
		CDragButton(PyObject* ppyObject);
		virtual ~CDragButton();

		void SetRestrictMovementArea(int ix, int iy, int iwidth, int iheight);

	protected:
		void OnChangePosition();
		void OnMouseOverIn();
		void OnMouseOverOut();

	protected:
		RECT m_restrictArea;
	};
};

extern BOOL g_bOutlineBoxEnable;
