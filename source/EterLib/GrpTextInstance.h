#ifndef __INC_ETERLIB_GRPTEXTINSTANCE_H__
#define __INC_ETERLIB_GRPTEXTINSTANCE_H__
#include "../UserInterface/Locale_inc.h"
#include <memory>
#include "Pool.h"
#include "GrpText.h"

#ifdef ENABLE_TEXT_IMAGE_LINE
#include "GrpImageInstance.h"
#endif

class CGraphicTextInstance
{
	public:
		typedef CDynamicPool<CGraphicTextInstance> TPool;

	public:
		enum EHorizontalAlign
		{
			HORIZONTAL_ALIGN_LEFT		= 0x01,
			HORIZONTAL_ALIGN_CENTER		= 0x02,
			HORIZONTAL_ALIGN_RIGHT		= 0x03,
		};
		enum EVerticalAlign
		{
			VERTICAL_ALIGN_TOP		= 0x10,
			VERTICAL_ALIGN_CENTER	= 0x20,
			VERTICAL_ALIGN_BOTTOM	= 0x30
		};

	public:
		static void Hyperlink_UpdateMousePos(int x, int y);
		static int  Hyperlink_GetText(char* buf, int len);
// #ifdef ENABLE_TRANSLATOR_GOOGLE_SYSTEM
		static void UpdateMousePos(int x, int y);
		static std::string & GetTextFromMousePos();
// #endif
	public:
		CGraphicTextInstance();
		virtual ~CGraphicTextInstance();

		void Destroy();

		void Update();
		void Render(RECT * pClipRect = NULL);

		void ShowCursor();
		void HideCursor();

#ifdef ENABLE_INGAME_WIKI
		void SetFixedRenderPos(WORD startPos, WORD endPos) { m_startPos = startPos; m_endPos = endPos; m_isFixedRenderPos = true; }
		void GetRenderPositions(WORD& startPos, WORD& endPos) { startPos = m_startPos; endPos = m_endPos; }
		bool IsShowCursor() { return m_isCursor; }
		DWORD GetColor() const;
		float GetPositionX() const { return m_v3Position.x; }
		float GetPositionY() const { return m_v3Position.y; }
		void SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom);
		void iSetRenderingRect(int iLeft, int iTop, int iRight, int iBottom);
		void SetRenderBox(RECT& renderBox);
#endif

		void ShowOutLine();
		void HideOutLine();
#ifdef ENABLE_MULTI_TEXTLINE
		void CheckMultiLine();
		void SetEnterToken(bool v) { m_EnterToken = v; }
#endif
		void SetColor(DWORD color);
		void SetColor(float r, float g, float b, float a = 1.0f);

		void SetOutLineColor(DWORD color);
		void SetOutLineColor(float r, float g, float b, float a = 1.0f);
#ifdef ENABLE_SUNG_MAHI_TOWER
		void GetCharacterWidth(short* sWidth);
#endif
		void SetHorizonalAlign(int hAlign);
		void SetVerticalAlign(int vAlign);
		void SetMax(int iMax);
		void SetTextPointer(CGraphicText* pText);
		void SetValueString(const string& c_stValue);
		void SetValue(const char* c_szValue, size_t len = -1);
		void SetPosition(float fx, float fy, float fz = 0.0f);
		void SetSecret(bool Value);
		void SetOutline(bool Value);
		void SetFeather(bool Value);
		void SetMultiLine(bool Value);
		void SetLimitWidth(float fWidth);

		void GetTextSize(int* pRetWidth, int* pRetHeight);
		int GetWidth() const { return m_textWidth; }
		int GetHeight() const { return m_textHeight; }
		const std::string& GetValueStringReference();
		WORD GetTextLineCount();
		WORD GetLineHeight() { return m_textLineHeight; }
		void SetLineHeight(const int c_iLineHeight) { m_textLineHeight = c_iLineHeight; }

		int PixelPositionToCharacterPosition(int iPixelPosition);
		int GetHorizontalAlign();

	protected:
		void __Initialize();
		int  __DrawCharacter(CGraphicFontTexture * pFontTexture, WORD codePage, wchar_t text, DWORD dwColor);
		void __GetTextPos(DWORD index, float* x, float* y);
		int __GetTextTag(const wchar_t * src, int maxLen, int & tagLen, std::wstring & extraInfo);

	protected:
		struct SHyperlink
		{
			short sx;
			short ex;
			std::wstring text;

			SHyperlink() : sx(0), ex(0) { }
		};
#ifdef ENABLE_TEXT_IMAGE_LINE
		struct SEmoji
		{
			short x;
			CGraphicImageInstance* pInstance;

			SEmoji() : x(0)
			{
				pInstance = NULL;
			}
		};
#endif
	protected:
		DWORD m_dwTextColor;
		DWORD m_dwOutLineColor;

		WORD m_textWidth;
		WORD m_textHeight;
		WORD m_textLineHeight;

		BYTE m_hAlign;
		BYTE m_vAlign;

		WORD m_iMax;
		float m_fLimitWidth;

		bool m_isCursor;
		bool m_isSecret;
		bool m_isMultiLine;

		bool m_isOutline;
		float m_fFontFeather;

		/////

		std::string m_stText;
		D3DXVECTOR3 m_v3Position;

#ifdef ENABLE_INGAME_WIKI
		WORD m_startPos, m_endPos;
		bool m_isFixedRenderPos;
		RECT m_renderBox;
		bool m_bUseRenderingRect;
		RECT m_RenderingRect;
#endif

	private:
		bool m_isUpdate;
		bool m_isUpdateFontTexture;

		CGraphicText::TRef m_roText;
		CGraphicFontTexture::TPCharacterInfomationVector m_pCharInfoVector;
		std::vector<DWORD> m_dwColorInfoVector;
		std::vector<SHyperlink> m_hyperlinkVector;
#ifdef ENABLE_MULTI_TEXTLINE
		std::vector<std::shared_ptr<CGraphicTextInstance>> multi_text;
		bool m_EnterToken;
#endif
#ifdef ENABLE_TEXT_IMAGE_LINE
		std::vector<SEmoji> m_emojiVector;
#endif
	public:
		static void CreateSystem(UINT uCapacity);
		static void DestroySystem();

		static CGraphicTextInstance* New();
		static void Delete(CGraphicTextInstance* pkInst);

		static CDynamicPool<CGraphicTextInstance>		ms_kPool;
};

extern const char* FindToken(const char* begin, const char* end);
extern int ReadToken(const char* token);

#endif