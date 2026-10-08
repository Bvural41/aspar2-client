#include "StdAfx.h"

#if defined(USE_OPENGL_ES)
static const float c_fHalfPixel = 0.0f;
#else
static const float c_fHalfPixel = 0.5f;
#endif
#include "GrpTextInstance.h"
#include "StateManager.h"
#include "IME.h"
#include "TextTag.h"
#include "../EterLocale/StringCodec.h"
#include "../EterBase/Utils.h"
#include "../EterLocale/Arabic.h"
#include "../UserInterface/Locale_inc.h"

#ifdef ENABLE_TEXT_IMAGE_LINE
#include "ResourceManager.h"
#endif

extern DWORD GetDefaultCodePage();

const float c_fFontFeather = 0.5f;

CDynamicPool<CGraphicTextInstance>		CGraphicTextInstance::ms_kPool;

static int gs_mx = 0;
static int gs_my = 0;

static std::wstring gs_hyperlinkText;
// #ifdef ENABLE_TRANSLATOR_GOOGLE_SYSTEM
static std::string gs_text;
// #endif

#ifdef ENABLE_MULTI_TEXTLINE
#include <regex>
static const TTokenVector SplitMultiLine(const TTokenVector::value_type& m_stText)
{
	static const std::regex re(R"(\\n|\n)");
	const std::sregex_token_iterator first{ m_stText.begin(), m_stText.end(), re, -1 }, last;
	return { first, last };
};
void CGraphicTextInstance::CheckMultiLine()
{
	if (!multi_text.empty())
		multi_text.clear();

	if (!m_EnterToken)
		return;

	auto MultiVec = SplitMultiLine(m_stText);
	if (MultiVec.size() < 2)
		return;

	m_stText.assign(MultiVec.at(0));
	MultiVec.erase(MultiVec.begin());
	multi_text.reserve(MultiVec.size());

	for (const auto& v : MultiVec) {
		auto pTextInstance = std::make_shared<CGraphicTextInstance>(*this);
		pTextInstance->SetEnterToken(false);
		pTextInstance->SetValueString(v);
		multi_text.emplace_back(std::move(pTextInstance));
	}
	SetPosition(m_v3Position.x, m_v3Position.y, m_v3Position.z);
}
#endif

void CGraphicTextInstance::Hyperlink_UpdateMousePos(int x, int y)
{
	gs_mx = x;
	gs_my = y;
	gs_hyperlinkText = L"";
}

int CGraphicTextInstance::Hyperlink_GetText(char* buf, int len)
{
	if (gs_hyperlinkText.empty())
		return 0;

	int codePage = GetDefaultCodePage();

	return Ymir_WideCharToMultiByte(codePage, 0, gs_hyperlinkText.c_str(), gs_hyperlinkText.length(), buf, len, NULL, NULL);
}

// #ifdef ENABLE_TRANSLATOR_GOOGLE_SYSTEM
void CGraphicTextInstance::UpdateMousePos(int x, int y)
{
	gs_mx = x;
	gs_my = y;
	gs_text = "";
}

std::string & CGraphicTextInstance::GetTextFromMousePos()
{
	return gs_text;
}
// #endif

int CGraphicTextInstance::__DrawCharacter(CGraphicFontTexture * pFontTexture, WORD codePage, wchar_t text, DWORD dwColor)
{
	CGraphicFontTexture::TCharacterInfomation* pInsCharInfo = pFontTexture->GetCharacterInfomation(codePage, text);

	if (pInsCharInfo)
	{
		m_dwColorInfoVector.push_back(dwColor);
		m_pCharInfoVector.push_back(pInsCharInfo);

		m_textWidth += pInsCharInfo->advance;
		m_textHeight = max(pInsCharInfo->height, m_textHeight);
		return pInsCharInfo->advance;
	}

	return 0;
}

void CGraphicTextInstance::__GetTextPos(DWORD index, float* x, float* y)
{
	index = min(index, m_pCharInfoVector.size());

	float sx = 0;
	float sy = 0;
	float fFontMaxHeight = 0;

#ifdef ENABLE_INGAME_WIKI
	for (DWORD i = m_startPos; i < index; ++i)
#else
	for(DWORD i=0; i<index; ++i)
#endif
	{
		if (sx+float(m_pCharInfoVector[i]->width) > m_fLimitWidth)
		{
			sx = 0;
			sy += fFontMaxHeight;
		}

		sx += float(m_pCharInfoVector[i]->advance);
		fFontMaxHeight = max(float(m_pCharInfoVector[i]->height), fFontMaxHeight);
	}

	*x = sx;
	*y = sy;
}

bool isNumberic(const char chr)
{
	if (chr >= '0' && chr <= '9')
		return true;
	return false;
}

bool IsValidToken(const char* iter)
{
	return	iter[0]=='@' &&
		isNumberic(iter[1]) &&
		isNumberic(iter[2]) &&
		isNumberic(iter[3]) &&
		isNumberic(iter[4]);
}

const char* FindToken(const char* begin, const char* end)
{
	while(begin < end)
	{
		begin = std::find(begin, end, '@');

		if(end-begin>5 && IsValidToken(begin))
		{
			return begin;
		}
		else
		{
			++begin;
		}
	}

	return end;
}

int ReadToken(const char* token)
{
	int nRet = (token[1]-'0')*1000 + (token[2]-'0')*100 + (token[3]-'0')*10 + (token[4]-'0');
	if (nRet == 9999)
		return CP_UTF8;
	return nRet;
}

void CGraphicTextInstance::Update()
{
	if (m_isUpdate) // 臾몄옄뿴씠 諛붾뚯뿀쓣 븣留 뾽뜲씠듃 븳떎.
		return;

#ifdef ENABLE_MULTI_TEXTLINE
	for (const auto& v : multi_text)
		v->Update();
#endif

	if (m_roText.IsNull())
	{
		Tracef("CGraphicTextInstance::Update - Font has not been set\n");
		return;
	}

	if (m_roText->IsEmpty())
		return;

	CGraphicFontTexture* pFontTexture = m_roText->GetFontTexturePointer();
	if (!pFontTexture)
		return;

	UINT defCodePage = GetDefaultCodePage();

	UINT dataCodePage = defCodePage; // 븘엻 諛 踰좏듃궓 궡遺 뜲씠꽣瑜 UTF8 쓣 궗슜븯젮 뻽쑝굹 떎뙣

	CGraphicFontTexture::TCharacterInfomation* pSpaceInfo = pFontTexture->GetCharacterInfomation(dataCodePage, ' ');

	int spaceHeight = pSpaceInfo ? pSpaceInfo->height : 12;

	m_pCharInfoVector.clear();
	m_dwColorInfoVector.clear();
	m_hyperlinkVector.clear();
#ifdef ENABLE_TEXT_IMAGE_LINE
	if (m_emojiVector.size() != 0)
	{
		for (std::vector<SEmoji>::iterator itor = m_emojiVector.begin(); itor != m_emojiVector.end(); ++itor)
		{
			SEmoji& rEmo = *itor;
			if (rEmo.pInstance)
			{
				CGraphicImageInstance::Delete(rEmo.pInstance);
				rEmo.pInstance = NULL;
			}
		}
	}
	m_emojiVector.clear();
#endif
	m_textWidth = 0;
	m_textHeight = spaceHeight;

	/* wstring begin */

	const char* begin = m_stText.c_str();
	const char* end = begin + m_stText.length();

	int wTextMax = (end - begin) * 2;
	wchar_t* wText = (wchar_t*)_alloca(sizeof(wchar_t)*wTextMax);

	DWORD dwColor = m_dwTextColor;

	/* wstring end */
	while (begin < end)
	{
		const char * token = FindToken(begin, end);

		int wTextLen = Ymir_MultiByteToWideChar(dataCodePage, 0, begin, token - begin, wText, wTextMax);

		if (m_isSecret)
		{
			for(int i=0; i<wTextLen; ++i)
				__DrawCharacter(pFontTexture, dataCodePage, '*', dwColor);
		}
		else
		{
			if (defCodePage == CP_ARABIC) // ARABIC
			{

				wchar_t* wArabicText = (wchar_t*)_alloca(sizeof(wchar_t) * wTextLen);
				int wArabicTextLen = Arabic_MakeShape(wText, wTextLen, wArabicText, wTextLen);

				bool isEnglish = true;
				int nEnglishBase = wArabicTextLen - 1;

				//<<븯씠띁 留곹겕>>
				int x = 0;

				int len;
				int hyperlinkStep = 0;
				SHyperlink kHyperlink;
				std::wstring hyperlinkBuffer;
				int no_hyperlink = 0;

				// 떖蹂쇰줈 걹굹硫 븘엻뼱 紐⑤뱶濡 떆옉빐빞븳떎
				if (Arabic_IsInSymbol(wArabicText[wArabicTextLen - 1]))
				{
					isEnglish = false;
				}

				int i = 0;
				for (i = wArabicTextLen - 1 ; i >= 0; --i)
				{
					wchar_t wArabicChar = wArabicText[i];

					if (isEnglish)
					{

						// <<떖蹂쇱쓽 寃쎌슦 (ex. 湲고샇, 怨듬갚)>> -> 쁺뼱 紐⑤뱶 쑀吏.
						//		<<(떖蹂쇱씠 븘땶 寃껊뱾 : 닽옄, 쁺뼱, 븘엻뼱)>>
						//  (1) 留 븵쓽 떖蹂 or
						//	(2)
						//		1) 븵 湲옄媛 븘엻뼱 븘떂 &&
						//		2) 뮮 湲옄媛 븘엻뼱 븘떂 &&
						//		3) 뮮 湲옄媛 떖蹂'|'씠 븘떂 &&
						//		or
						//	(3) 쁽옱 떖蹂쇱씠 '|'
						// <<븘엻뼱 紐⑤뱶濡 꽆뼱媛뒗 寃쎌슦 : 떖蹂쇱뿉꽌.>>
						//	1) 븵湲옄 븘엻뼱
						//	2) 뮮湲옄 븘엻뼱
						//
						//
						if (Arabic_IsInSymbol(wArabicChar) && (
								(i == 0) ||
								(i > 0 &&
									!(Arabic_HasPresentation(wArabicText, i - 1) || Arabic_IsInPresentation(wArabicText[i + 1]))  && //븵湲옄, 뮮湲옄媛 븘엻뼱 븘떂.
									wArabicText[i+1] != '|'
								) ||
								wArabicText[i] == '|'
							))//if end.
						{
							// pass
							int temptest = 1;
						}
						// (1)븘엻뼱씠嫄곕굹 (2)븘엻뼱 떎쓬쓽 떖蹂쇱씠씪硫 븘엻뼱 紐⑤뱶 쟾솚
						else if (Arabic_IsInPresentation(wArabicChar) || Arabic_IsInSymbol(wArabicChar))
						{
							//洹 쟾源뚯쓽 쁺뼱瑜 洹몃┛떎.
							for (int e = i + 1; e <= nEnglishBase;) {
								int ret = GetTextTag(&wArabicText[e], wArabicTextLen - e, len, hyperlinkBuffer);

								if (ret == TEXT_TAG_PLAIN || ret == TEXT_TAG_TAG)
								{
									if (hyperlinkStep == 1)
										hyperlinkBuffer.append(1, wArabicText[e]);
									else
									{
										int charWidth = __DrawCharacter(pFontTexture, dataCodePage, wArabicText[e], dwColor);
										kHyperlink.ex += charWidth;
										//x += charWidth;

										//湲곗〈 異붽븳 븯씠띁留곹겕쓽 醫뚰몴 닔젙.
										for (int j = 1; j <= no_hyperlink; j++)
										{
											if(m_hyperlinkVector.size() < j)
												break;

											SHyperlink & tempLink = m_hyperlinkVector[m_hyperlinkVector.size() - j];
											tempLink.ex += charWidth;
											tempLink.sx += charWidth;
										}
									}
								}
								else
								{
									if (ret == TEXT_TAG_COLOR)
										dwColor = htoi(hyperlinkBuffer.c_str(), 8);
									else if (ret == TEXT_TAG_RESTORE_COLOR)
										dwColor = m_dwTextColor;
									else if (ret == TEXT_TAG_HYPERLINK_START)
									{
										hyperlinkStep = 1;
										hyperlinkBuffer = L"";
									}
									else if (ret == TEXT_TAG_HYPERLINK_END)
									{
										if (hyperlinkStep == 1)
										{
											++hyperlinkStep;
											kHyperlink.ex = kHyperlink.sx = 0; // 떎젣 뀓뒪듃媛 떆옉릺뒗 쐞移
										}
										else
										{
											kHyperlink.text = hyperlinkBuffer;
											m_hyperlinkVector.push_back(kHyperlink);
											no_hyperlink++;


											hyperlinkStep = 0;
											hyperlinkBuffer = L"";
										}
									}
								}
								e += len;
							}

							int charWidth = __DrawCharacter(pFontTexture, dataCodePage, Arabic_ConvSymbol(wArabicText[i]), dwColor);
							kHyperlink.ex += charWidth;

							//湲곗〈 異붽븳 븯씠띁留곹겕쓽 醫뚰몴 닔젙.
							for (int j = 1; j <= no_hyperlink; j++)
							{
								if(m_hyperlinkVector.size() < j)
									break;

								SHyperlink & tempLink = m_hyperlinkVector[m_hyperlinkVector.size() - j];
								tempLink.ex += charWidth;
								tempLink.sx += charWidth;
							}

							isEnglish = false;
						}
					}
					else //[[[븘엻뼱 紐⑤뱶]]]
					{
						// 븘엻뼱씠嫄곕굹 븘엻뼱 異쒕젰以 굹삤뒗 떖蹂쇱씠씪硫
						if (Arabic_IsInPresentation(wArabicChar) || Arabic_IsInSymbol(wArabicChar))
						{
							int charWidth = __DrawCharacter(pFontTexture, dataCodePage, Arabic_ConvSymbol(wArabicText[i]), dwColor);
							kHyperlink.ex += charWidth;
							x += charWidth;

							//湲곗〈 異붽븳 븯씠띁留곹겕쓽 醫뚰몴 닔젙.
							for (int j = 1; j <= no_hyperlink; j++)
							{
								if(m_hyperlinkVector.size() < j)
									break;

								SHyperlink & tempLink = m_hyperlinkVector[m_hyperlinkVector.size() - j];
								tempLink.ex += charWidth;
								tempLink.sx += charWidth;
							}
						}
						else //쁺뼱씠嫄곕굹, 쁺뼱 떎쓬뿉 굹삤뒗 떖蹂쇱씠씪硫,
						{
							nEnglishBase = i;
							isEnglish = true;
						}
					}
				}

				if (isEnglish)
				{
					for (int e = i + 1; e <= nEnglishBase;) {
						int ret = GetTextTag(&wArabicText[e], wArabicTextLen - e, len, hyperlinkBuffer);

						if (ret == TEXT_TAG_PLAIN || ret == TEXT_TAG_TAG)
						{
							if (hyperlinkStep == 1)
								hyperlinkBuffer.append(1, wArabicText[e]);
							else
							{
								int charWidth = __DrawCharacter(pFontTexture, dataCodePage, wArabicText[e], dwColor);
								kHyperlink.ex += charWidth;

								//湲곗〈 異붽븳 븯씠띁留곹겕쓽 醫뚰몴 닔젙.
								for (int j = 1; j <= no_hyperlink; j++)
								{
									if(m_hyperlinkVector.size() < j)
										break;

									SHyperlink & tempLink = m_hyperlinkVector[m_hyperlinkVector.size() - j];
									tempLink.ex += charWidth;
									tempLink.sx += charWidth;
								}
							}
						}
						else
						{
							if (ret == TEXT_TAG_COLOR)
								dwColor = htoi(hyperlinkBuffer.c_str(), 8);
							else if (ret == TEXT_TAG_RESTORE_COLOR)
								dwColor = m_dwTextColor;
							else if (ret == TEXT_TAG_HYPERLINK_START)
							{
								hyperlinkStep = 1;
								hyperlinkBuffer = L"";
							}
							else if (ret == TEXT_TAG_HYPERLINK_END)
							{
								if (hyperlinkStep == 1)
								{
									++hyperlinkStep;
									kHyperlink.ex = kHyperlink.sx = 0; // 떎젣 뀓뒪듃媛 떆옉릺뒗 쐞移
								}
								else
								{
									kHyperlink.text = hyperlinkBuffer;
									m_hyperlinkVector.push_back(kHyperlink);
									no_hyperlink++;

									hyperlinkStep = 0;
									hyperlinkBuffer = L"";
								}
							}
						}
						e += len;
					}

				}
			}
			else	// 븘엻쇅 떎瑜 吏뿭.
			{
				int x = 0;
				int len;
				int hyperlinkStep = 0;
				SHyperlink kHyperlink;
				std::wstring hyperlinkBuffer;

#ifdef ENABLE_TEXT_IMAGE_LINE
				SEmoji kEmoji;
				int emojiStep = 0;
				std::wstring emojiBuffer;
#endif

				for (int i = 0; i < wTextLen; )
				{
					int ret = GetTextTag(&wText[i], wTextLen - i, len, hyperlinkBuffer);

					if (ret == TEXT_TAG_PLAIN || ret == TEXT_TAG_TAG)
					{
						if (hyperlinkStep == 1)
							hyperlinkBuffer.append(1, wText[i]);
#ifdef ENABLE_TEXT_IMAGE_LINE
						else if (emojiStep == 1)
							emojiBuffer.append(1, wText[i]);
#endif
						else
						{
							int charWidth = __DrawCharacter(pFontTexture, dataCodePage, wText[i], dwColor);
							kHyperlink.ex += charWidth;
							x += charWidth;
						}
					}
					else
					{
						if (ret == TEXT_TAG_COLOR)
							dwColor = htoi(hyperlinkBuffer.c_str(), 8);
						else if (ret == TEXT_TAG_RESTORE_COLOR)
							dwColor = m_dwTextColor;
						else if (ret == TEXT_TAG_HYPERLINK_START)
						{
							hyperlinkStep = 1;
							hyperlinkBuffer = L"";
						}
						else if (ret == TEXT_TAG_HYPERLINK_END)
						{
							if (hyperlinkStep == 1)
							{
								++hyperlinkStep;
								kHyperlink.ex = kHyperlink.sx = x; // 떎젣 뀓뒪듃媛 떆옉릺뒗 쐞移
							}
							else
							{
								kHyperlink.text = hyperlinkBuffer;
								m_hyperlinkVector.push_back(kHyperlink);

								hyperlinkStep = 0;
								hyperlinkBuffer = L"";
							}
						}
#ifdef ENABLE_TEXT_IMAGE_LINE
						else if (ret == TEXT_TAG_EMOJI_START)
						{
							emojiStep = 1;
							emojiBuffer = L"";
						}
						else if (ret == TEXT_TAG_EMOJI_END)
						{
							kEmoji.x = x;

							char retBuf[1024];
							int retLen = Ymir_WideCharToMultiByte(GetDefaultCodePage(), 0, emojiBuffer.c_str(), emojiBuffer.length(), retBuf, sizeof(retBuf) - 1, NULL, NULL);
							retBuf[retLen] = '\0';

							char szPath[255];
							char szPath2[255];
							_snprintf(szPath, sizeof(szPath), "icon/%s.tga", retBuf);
							_snprintf(szPath2, sizeof(szPath2), "%s", retBuf);
							if (CResourceManager::Instance().IsFileExist(szPath))
							{
								CGraphicImage* pImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(szPath);
								kEmoji.pInstance = CGraphicImageInstance::New();
								kEmoji.pInstance->SetImagePointer(pImage);
								m_emojiVector.push_back(kEmoji);
								memset(&kEmoji, 0, sizeof(SEmoji));
								for (int i = 0; i < pImage->GetWidth() / (pSpaceInfo->width - 1); ++i)
									x += __DrawCharacter(pFontTexture, dataCodePage, ' ', dwColor);
								if (pImage->GetWidth() % (pSpaceInfo->width - 1) > 1)
									x += __DrawCharacter(pFontTexture, dataCodePage, ' ', dwColor);
							}
							else if (CResourceManager::Instance().IsFileExist(szPath2))
							{
								CGraphicImage* pImage = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(szPath2);
								kEmoji.pInstance = CGraphicImageInstance::New();
								kEmoji.pInstance->SetImagePointer(pImage);
								m_emojiVector.push_back(kEmoji);
								memset(&kEmoji, 0, sizeof(SEmoji));
								for (int i = 0; i < pImage->GetWidth() / (pSpaceInfo->width - 1); ++i)
									x += __DrawCharacter(pFontTexture, dataCodePage, ' ', dwColor);
								if (pImage->GetWidth() % (pSpaceInfo->width - 1) > 1)
									x += __DrawCharacter(pFontTexture, dataCodePage, ' ', dwColor);
							}
							emojiStep = 0;
							emojiBuffer = L"";
						}
#endif
					}
					i += len;
				}
			}
		}

		if (token < end)
		{
			int newCodePage = ReadToken(token);
			dataCodePage = newCodePage;	// 븘엻 諛 踰좏듃궓 궡遺 뜲씠꽣瑜 UTF8 쓣 궗슜븯젮 뻽쑝굹 떎뙣
			begin = token + 5;
		}
		else
		{
			begin = token;
		}
	}

	pFontTexture->UpdateTexture();

	m_isUpdate = true;
}

void CGraphicTextInstance::Render(RECT * pClipRect)
{
	if (!m_isUpdate)
		return;

#ifdef ENABLE_MULTI_TEXTLINE
	for (const auto& v : multi_text)
		v->Render(pClipRect);
#endif

	CGraphicText* pkText=m_roText.GetPointer();
	if (!pkText)
		return;

	CGraphicFontTexture* pFontTexture = pkText->GetFontTexturePointer();
	if (!pFontTexture)
		return;

	float fStanX = m_v3Position.x;
	float fStanY = m_v3Position.y + 1.0f;

	UINT defCodePage = GetDefaultCodePage();

	if (defCodePage == CP_ARABIC)
	{
		switch (m_hAlign)
		{
			case HORIZONTAL_ALIGN_LEFT:
				fStanX -= m_textWidth;
				break;

			case HORIZONTAL_ALIGN_CENTER:
				fStanX -= float(m_textWidth / 2);
				break;
		}
	}
	else
	{
		switch (m_hAlign)
		{
			case HORIZONTAL_ALIGN_RIGHT:
				fStanX -= m_textWidth;
				break;

			case HORIZONTAL_ALIGN_CENTER:
				fStanX -= float(m_textWidth / 2);
				break;
		}
	}

	switch (m_vAlign)
	{
		case VERTICAL_ALIGN_BOTTOM:
			fStanY -= m_textHeight;
			break;

		case VERTICAL_ALIGN_CENTER:
			fStanY -= float(m_textHeight) / 2.0f;
			break;
	}

	//WORD FillRectIndices[6] = { 0, 2, 1, 2, 3, 1 };

	STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	DWORD dwFogEnable = STATEMANAGER.GetRenderState(D3DRS_FOGENABLE);
	DWORD dwLighting = STATEMANAGER.GetRenderState(D3DRS_LIGHTING);
	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

	STATEMANAGER.SetFVF(D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2,	D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2,	D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_MODULATE);
#if defined(USE_OPENGL_ES)
	STATEMANAGER.SaveSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	STATEMANAGER.SaveSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
#endif
#ifdef ENABLE_FIX_MOBS_LAG
	std::map<CGraphicImageTexture*, std::vector<SPDTVertexRaw>> verticesMap;
#endif
	{
		const float fFontHalfWeight=1.0f;

		float fCurX;
		float fCurY;

		float fFontSx;
		float fFontSy;
		float fFontEx;
		float fFontEy;
		float fFontWidth;
		float fFontHeight;
		float fFontMaxHeight;
		float fFontAdvance;

#ifdef ENABLE_FIX_MOBS_LAG
		SPDTVertexRaw akVertex[4];
		akVertex[0].pz = m_v3Position.z;
		akVertex[1].pz = m_v3Position.z;
		akVertex[2].pz = m_v3Position.z;
		akVertex[3].pz = m_v3Position.z;
#else
		SVertex akVertex[4];
		akVertex[0].z = m_v3Position.z;
		akVertex[1].z = m_v3Position.z;
		akVertex[2].z = m_v3Position.z;
		akVertex[3].z = m_v3Position.z;
#endif

		CGraphicFontTexture::TCharacterInfomation* pCurCharInfo;

		// 뀒몢由
		if (m_isOutline)
		{
			fCurX=fStanX;
			fCurY=fStanY;
			fFontMaxHeight=0.0f;

			CGraphicFontTexture::TPCharacterInfomationVector::iterator i;
			for (i=m_pCharInfoVector.begin(); i!=m_pCharInfoVector.end(); ++i)
			{
				pCurCharInfo = *i;

				fFontWidth=float(pCurCharInfo->width);
				fFontHeight=float(pCurCharInfo->height);
				fFontAdvance=float(pCurCharInfo->advance);

				// NOTE : 룿듃 異쒕젰뿉 Width 젣븳쓣 몼땲떎. - [levites]
				if ((fCurX+fFontWidth)-m_v3Position.x > m_fLimitWidth)
				{
					if (m_isMultiLine)
					{
						fCurX=fStanX;
						fCurY+=fFontMaxHeight;
					}
					else
					{
						break;
					}
				}

#if !defined(ENABLE_USE_CLIP_MASK)
				if (pClipRect)
				{
					if (fCurY <= pClipRect->top)
					{
						fCurX += fFontAdvance;
						continue;
					}
				}
#endif

				fFontSx = fCurX - c_fHalfPixel;
				fFontSy = fCurY - c_fHalfPixel;
				fFontEx = fFontSx + fFontWidth;
				fFontEy = fFontSy + fFontHeight;

#if defined(ENABLE_USE_CLIP_MASK)
				float pleft = pCurCharInfo->left;
				float ptop = pCurCharInfo->top;
				float pright = pCurCharInfo->right;
				float pbottom = pCurCharInfo->bottom;
				
				if (pClipRect)
				{
					const float v1 = pCurCharInfo->right - pCurCharInfo->left;
					const float v2 = pCurCharInfo->bottom - pCurCharInfo->top;

					if (fFontEx <= pClipRect->left)
					{
						fCurX += fFontAdvance;
						continue;
					}

					if (fFontSx < pClipRect->left)
					{
						const float fCal = pClipRect->left - fFontSx;
						fFontSx += fCal;
						pleft += fCal / fFontWidth * v1;
					}

					if (fFontEy <= pClipRect->top)
					{
						fCurX += fFontAdvance;
						continue;
					}

					if (fFontSy < pClipRect->top)
					{
						const float fCal = pClipRect->top - fFontSy;
						fFontSy += fCal;
						ptop += fCal / fFontHeight * v2;
					}

					if (fFontSx >= pClipRect->right)
					{
						fCurX += fFontAdvance;
						continue;
					}

					if (fFontEx > pClipRect->right)
					{
						const float fCal = fFontEx - pClipRect->right;
						fFontEx -= fCal;
						pright -= fCal / fFontWidth * v1;
					}

					if (fFontSy >= pClipRect->bottom)
					{
						fCurX += fFontAdvance;
						continue;
					}

					if (fFontEy > pClipRect->bottom)
					{
						const float fCal = fFontEy - pClipRect->bottom;
						fFontEy -= fCal;
						pbottom -= fCal / fFontHeight * v2;
					}
				}

#ifdef ENABLE_FIX_MOBS_LAG
				const auto tex = pFontTexture->GetTexture(pCurCharInfo->index);
				auto& batchVertices = verticesMap[tex];
#else
				pFontTexture->SelectTexture(pCurCharInfo->index);
				STATEMANAGER.SetTexture(0, pFontTexture->GetD3DTexture());
#endif

				akVertex[0].u = pleft;
				akVertex[0].v = ptop;
				akVertex[1].u = pleft;
				akVertex[1].v = pbottom;
				akVertex[2].u = pright;
				akVertex[2].v = ptop;
				akVertex[3].u = pright;
				akVertex[3].v = pbottom;
#else
#ifdef ENABLE_FIX_MOBS_LAG
				const auto tex = pFontTexture->GetTexture(pCurCharInfo->index);
				auto& batchVertices = verticesMap[tex];
#else
				pFontTexture->SelectTexture(pCurCharInfo->index);
				STATEMANAGER.SetTexture(0, pFontTexture->GetD3DTexture());
#endif

				akVertex[0].u = pCurCharInfo->left;
				akVertex[0].v = pCurCharInfo->top;
				akVertex[1].u = pCurCharInfo->left;
				akVertex[1].v = pCurCharInfo->bottom;
				akVertex[2].u = pCurCharInfo->right;
				akVertex[2].v = pCurCharInfo->top;
				akVertex[3].u = pCurCharInfo->right;
				akVertex[3].v = pCurCharInfo->bottom;
#endif

#ifdef ENABLE_FIX_MOBS_LAG
				akVertex[3].diffuse = akVertex[2].diffuse = akVertex[1].diffuse = akVertex[0].diffuse = m_dwOutLineColor;
#else
				akVertex[3].color = akVertex[2].color = akVertex[1].color = akVertex[0].color = m_dwOutLineColor;
#endif


				float feather = 0.0f; // m_fFontFeather

#ifdef ENABLE_FIX_MOBS_LAG
				akVertex[0].py = fFontSy - feather;
				akVertex[1].py = fFontEy + feather;
				akVertex[2].py = fFontSy - feather;
				akVertex[3].py = fFontEy + feather;
				akVertex[0].px = fFontSx - fFontHalfWeight - feather;
				akVertex[1].px = fFontSx - fFontHalfWeight - feather;
				akVertex[2].px = fFontEx - fFontHalfWeight + feather;
				akVertex[3].px = fFontEx - fFontHalfWeight + feather;
#else
				akVertex[0].y = fFontSy - feather;
				akVertex[1].y = fFontEy + feather;
				akVertex[2].y = fFontSy - feather;
				akVertex[3].y = fFontEy + feather;
				akVertex[0].x = fFontSx - fFontHalfWeight - feather;
				akVertex[1].x = fFontSx - fFontHalfWeight - feather;
				akVertex[2].x = fFontEx - fFontHalfWeight + feather;
				akVertex[3].x = fFontEx - fFontHalfWeight + feather;
#endif

#ifdef ENABLE_FIX_MOBS_LAG
				batchVertices.insert(batchVertices.end(),
					std::begin(akVertex), std::end(akVertex));

				akVertex[0].px = fFontSx + fFontHalfWeight - feather;
				akVertex[1].px = fFontSx + fFontHalfWeight - feather;
				akVertex[2].px = fFontEx + fFontHalfWeight + feather;
				akVertex[3].px = fFontEx + fFontHalfWeight + feather;
#else
				if (CGraphicBase::SetPDTStream((SPDTVertex*)akVertex, 4))
				{
					STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
				}

				akVertex[0].x = fFontSx + fFontHalfWeight - feather;
				akVertex[1].x = fFontSx + fFontHalfWeight - feather;
				akVertex[2].x = fFontEx + fFontHalfWeight + feather;
				akVertex[3].x = fFontEx + fFontHalfWeight + feather;
#endif


#ifdef ENABLE_FIX_MOBS_LAG
				batchVertices.insert(batchVertices.end(),
					std::begin(akVertex), std::end(akVertex));

				akVertex[0].px = fFontSx - feather;
				akVertex[1].px = fFontSx - feather;
				akVertex[2].px = fFontEx + feather;
				akVertex[3].px = fFontEx + feather;

				akVertex[0].py = fFontSy - fFontHalfWeight - feather;
				akVertex[1].py = fFontEy - fFontHalfWeight + feather;
				akVertex[2].py = fFontSy - fFontHalfWeight - feather;
				akVertex[3].py = fFontEy - fFontHalfWeight + feather;
#else
				if (CGraphicBase::SetPDTStream((SPDTVertex*)akVertex, 4))
				{
					STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
				}

				akVertex[0].x = fFontSx - feather;
				akVertex[1].x = fFontSx - feather;
				akVertex[2].x = fFontEx + feather;
				akVertex[3].x = fFontEx + feather;
				akVertex[0].y = fFontSy - fFontHalfWeight - feather;
				akVertex[1].y = fFontEy - fFontHalfWeight + feather;
				akVertex[2].y = fFontSy - fFontHalfWeight - feather;
				akVertex[3].y = fFontEy - fFontHalfWeight + feather;
#endif

#ifdef ENABLE_FIX_MOBS_LAG
				batchVertices.insert(batchVertices.end(),
					std::begin(akVertex), std::end(akVertex));

				akVertex[0].py = fFontSy + fFontHalfWeight - feather;
				akVertex[1].py = fFontEy + fFontHalfWeight + feather;
				akVertex[2].py = fFontSy + fFontHalfWeight - feather;
				akVertex[3].py = fFontEy + fFontHalfWeight + feather;
#else
				if (CGraphicBase::SetPDTStream((SPDTVertex*)akVertex, 4))
				{
					STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
				}

				akVertex[0].y = fFontSy + fFontHalfWeight - feather;
				akVertex[1].y = fFontEy + fFontHalfWeight + feather;
				akVertex[2].y = fFontSy + fFontHalfWeight - feather;
				akVertex[3].y = fFontEy + fFontHalfWeight + feather;
#endif

#ifdef ENABLE_FIX_MOBS_LAG
				batchVertices.insert(batchVertices.end(),
					std::begin(akVertex), std::end(akVertex));
#else
				if (CGraphicBase::SetPDTStream((SPDTVertex*)akVertex, 4))
				{
					STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
				}
#endif

				fCurX += fFontAdvance;
			}
		}

		// 硫붿씤 룿듃
		fCurX=fStanX;
		fCurY=fStanY;
		fFontMaxHeight=0.0f;

		for (int i = 0; i < m_pCharInfoVector.size(); ++i)
		{
			pCurCharInfo = m_pCharInfoVector[i];

			fFontWidth=float(pCurCharInfo->width);
			fFontHeight=float(pCurCharInfo->height);
			fFontMaxHeight=max(fFontHeight, pCurCharInfo->height);
			fFontAdvance=float(pCurCharInfo->advance);

			// NOTE : 룿듃 異쒕젰뿉 Width 젣븳쓣 몼땲떎. - [levites]
			if ((fCurX+fFontWidth)-m_v3Position.x > m_fLimitWidth)
			{
				if (m_isMultiLine)
				{
					fCurX=fStanX;
					fCurY+=fFontMaxHeight;
				}
				else
				{
					break;
				}
			}

#if !defined(ENABLE_USE_CLIP_MASK)
			if (pClipRect)
			{
				if (fCurY <= pClipRect->top)
				{
					fCurX += fFontAdvance;
					continue;
				}
			}
#endif

			fFontSx = fCurX - c_fHalfPixel;
			fFontSy = fCurY - c_fHalfPixel;
			fFontEx = fFontSx + fFontWidth;
			fFontEy = fFontSy + fFontHeight;

#if defined(ENABLE_USE_CLIP_MASK)
			float pleft = pCurCharInfo->left;
			float ptop = pCurCharInfo->top;
			float pright = pCurCharInfo->right;
			float pbottom = pCurCharInfo->bottom;

			if (pClipRect)
			{
				const float v1 = pCurCharInfo->right - pCurCharInfo->left;
				const float v2 = pCurCharInfo->bottom - pCurCharInfo->top;

				if (fFontEx <= pClipRect->left)
				{
					fCurX += fFontAdvance;
					continue;
				}

				if (fFontSx < pClipRect->left)
				{
					const float fCal = pClipRect->left - fFontSx;
					fFontSx += fCal;
					pleft += fCal / fFontWidth * v1;
				}

				if (fFontEy <= pClipRect->top)
				{
					fCurX += fFontAdvance;
					continue;
				}

				if (fFontSy < pClipRect->top)
				{
					const float fCal = pClipRect->top - fFontSy;
					fFontSy += fCal;
					ptop += fCal / fFontHeight * v2;
				}

				if (fFontSx >= pClipRect->right)
				{
					fCurX += fFontAdvance;
					continue;
				}

				if (fFontEx > pClipRect->right)
				{
					const float fCal = fFontEx - pClipRect->right;
					fFontEx -= fCal;
					pright -= fCal / fFontWidth * v1;
				}

				if (fFontSy >= pClipRect->bottom)
				{
					fCurX += fFontAdvance;
					continue;
				}

				if (fFontEy > pClipRect->bottom)
				{
					const float fCal = fFontEy - pClipRect->bottom;
					fFontEy -= fCal;
					pbottom -= fCal / fFontHeight * v2;
				}
			}

#ifdef ENABLE_FIX_MOBS_LAG
			const auto tex = pFontTexture->GetTexture(pCurCharInfo->index);
			auto& batchVertices = verticesMap[tex];
#else
			pFontTexture->SelectTexture(pCurCharInfo->index);
			STATEMANAGER.SetTexture(0, pFontTexture->GetD3DTexture());
#endif

#ifdef ENABLE_FIX_MOBS_LAG
			akVertex[0].px = fFontSx;
			akVertex[0].py = fFontSy;
			akVertex[0].u = pleft;
			akVertex[0].v = ptop;

			akVertex[1].px = fFontSx;
			akVertex[1].py = fFontEy;
			akVertex[1].u = pleft;
			akVertex[1].v = pbottom;

			akVertex[2].px = fFontEx;
			akVertex[2].py = fFontSy;
			akVertex[2].u = pright;
			akVertex[2].v = ptop;

			akVertex[3].px = fFontEx;
			akVertex[3].py = fFontEy;
			akVertex[3].u = pright;
			akVertex[3].v = pbottom;
#else
			akVertex[0].x = fFontSx;
			akVertex[0].y = fFontSy;
			akVertex[0].u = pleft;
			akVertex[0].v = ptop;

			akVertex[1].x = fFontSx;
			akVertex[1].y = fFontEy;
			akVertex[1].u = pleft;
			akVertex[1].v = pbottom;

			akVertex[2].x = fFontEx;
			akVertex[2].y = fFontSy;
			akVertex[2].u = pright;
			akVertex[2].v = ptop;

			akVertex[3].x = fFontEx;
			akVertex[3].y = fFontEy;
			akVertex[3].u = pright;
			akVertex[3].v = pbottom;
#endif
#else
#ifdef ENABLE_FIX_MOBS_LAG
			const auto tex = pFontTexture->GetTexture(pCurCharInfo->index);
			auto& batchVertices = verticesMap[tex];
#else
			pFontTexture->SelectTexture(pCurCharInfo->index);
			STATEMANAGER.SetTexture(0, pFontTexture->GetD3DTexture());
#endif

#ifdef ENABLE_FIX_MOBS_LAG
			akVertex[0].px=fFontSx;
			akVertex[0].py=fFontSy;
			akVertex[0].u=pCurCharInfo->left;
			akVertex[0].v=pCurCharInfo->top;

			akVertex[1].px=fFontSx;
			akVertex[1].py=fFontEy;
			akVertex[1].u=pCurCharInfo->left;
			akVertex[1].v=pCurCharInfo->bottom;

			akVertex[2].px=fFontEx;
			akVertex[2].py=fFontSy;
			akVertex[2].u=pCurCharInfo->right;
			akVertex[2].v=pCurCharInfo->top;

			akVertex[3].px=fFontEx;
			akVertex[3].py=fFontEy;
			akVertex[3].u=pCurCharInfo->right;
			akVertex[3].v=pCurCharInfo->bottom;
#else
			akVertex[0].x=fFontSx;
			akVertex[0].y=fFontSy;
			akVertex[0].u=pCurCharInfo->left;
			akVertex[0].v=pCurCharInfo->top;

			akVertex[1].x=fFontSx;
			akVertex[1].y=fFontEy;
			akVertex[1].u=pCurCharInfo->left;
			akVertex[1].v=pCurCharInfo->bottom;

			akVertex[2].x=fFontEx;
			akVertex[2].y=fFontSy;
			akVertex[2].u=pCurCharInfo->right;
			akVertex[2].v=pCurCharInfo->top;

			akVertex[3].x=fFontEx;
			akVertex[3].y=fFontEy;
			akVertex[3].u=pCurCharInfo->right;
			akVertex[3].v=pCurCharInfo->bottom;
#endif
#endif

#ifdef ENABLE_FIX_MOBS_LAG
			akVertex[0].diffuse = akVertex[1].diffuse = akVertex[2].diffuse = akVertex[3].diffuse = m_dwColorInfoVector[i];
			batchVertices.insert(batchVertices.end(), std::begin(akVertex), std::end(akVertex));
#else
			akVertex[0].color = akVertex[1].color = akVertex[2].color = akVertex[3].color = m_dwColorInfoVector[i];

			if (CGraphicBase::SetPDTStream((SPDTVertex*)akVertex, 4))
			{
				STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
			}
#endif

			fCurX += fFontAdvance;
		}
	}

#ifdef ENABLE_FIX_MOBS_LAG
	if (!verticesMap.empty())
	{
		for (auto& pair : verticesMap)
		{
			const auto tex = pair.first;
			auto& vertices = pair.second;
			
			if (!tex || vertices.empty())
				continue;

			STATEMANAGER.SetTexture(0, tex->GetD3DTexture());
			
			size_t totalVertices = vertices.size();
			size_t offset = 0;
			const size_t maxBatchSize = 200; // LARGE_PDT_VERTEX_BUFFER_SIZE
			
			while (offset < totalVertices)
			{
				size_t batchSize = totalVertices - offset;
				if (batchSize > maxBatchSize)
					batchSize = maxBatchSize;
				
				if (CGraphicBase::SetPDTStream(vertices.data() + offset, batchSize))
				{
					for (size_t i = 0; i < (batchSize / 4); ++i)
					{
						STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLESTRIP, i * 4, 2);
					}
				}
				offset += batchSize;
			}
		}
	}
#endif

	if (m_isCursor)
	{
		// Draw Cursor
		float sx, sy, ex, ey;
		TDiffuse diffuse;

		int curpos = CIME::GetCurPos();
		int compend = curpos + CIME::GetCompLen();

		__GetTextPos(curpos, &sx, &sy);

		// If Composition
		if(curpos<compend)
		{
			diffuse = 0x7fffffff;
			__GetTextPos(compend, &ex, &sy);
		}
		else
		{
			diffuse = 0xffffffff;
			ex = sx + 2;
		}

		// FOR_ARABIC_ALIGN
		if (defCodePage == CP_ARABIC)
		{
			sx += m_v3Position.x - m_textWidth;
			ex += m_v3Position.x - m_textWidth;
			sy += m_v3Position.y;
			ey = sy + m_textHeight;
		}
		else
		{
			sx += m_v3Position.x;
			sy += m_v3Position.y;
			ex += m_v3Position.x;
			ey = sy + m_textHeight;
		}

		switch (m_vAlign)
		{
			case VERTICAL_ALIGN_BOTTOM:
				sy -= m_textHeight;
				break;

			case VERTICAL_ALIGN_CENTER:
				sy -= float(m_textHeight) / 2.0f;
				break;
		}
		// 理쒖쟻솕 궗빆
		// 媛숈뀓뒪爾먮 궗슜븳떎硫... STRIP쓣 援ъ꽦븯怨, 뀓뒪爾먭 蹂寃쎈릺嫄곕굹 걹굹硫 DrawPrimitive瑜 샇異쒗빐
		// 理쒕븳 닽옄瑜 以꾩씠룄濡앺븯옄!

		TPDTVertex vertices[4];
		vertices[0].diffuse = diffuse;
		vertices[1].diffuse = diffuse;
		vertices[2].diffuse = diffuse;
		vertices[3].diffuse = diffuse;
		vertices[0].position = TPosition(sx, sy, 0.0f);
		vertices[1].position = TPosition(ex, sy, 0.0f);
		vertices[2].position = TPosition(sx, ey, 0.0f);
		vertices[3].position = TPosition(ex, ey, 0.0f);

		STATEMANAGER.SetTexture(0, NULL);


		// 2004.11.18.myevan.DrawIndexPrimitiveUP -> DynamicVertexBuffer
		CGraphicBase::SetDefaultIndexBuffer(CGraphicBase::DEFAULT_IB_FILL_RECT);
		if (CGraphicBase::SetPDTStream(vertices, 4))
			STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2);

		int ulbegin = CIME::GetULBegin();
		int ulend = CIME::GetULEnd();

		if(ulbegin < ulend)
		{
			__GetTextPos(curpos+ulbegin, &sx, &sy);
			__GetTextPos(curpos+ulend, &ex, &sy);

			sx += m_v3Position.x;
			sy += m_v3Position.y + m_textHeight;
			ex += m_v3Position.x;
			ey = sy + 2;

			vertices[0].diffuse = 0xFFFF0000;
			vertices[1].diffuse = 0xFFFF0000;
			vertices[2].diffuse = 0xFFFF0000;
			vertices[3].diffuse = 0xFFFF0000;
			vertices[0].position = TPosition(sx, sy, 0.0f);
			vertices[1].position = TPosition(ex, sy, 0.0f);
			vertices[2].position = TPosition(sx, ey, 0.0f);
			vertices[3].position = TPosition(ex, ey, 0.0f);

			STATEMANAGER.DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 4, 2, c_FillRectIndices, D3DFMT_INDEX16, vertices, sizeof(TPDTVertex));
		}
	}

#if defined(USE_OPENGL_ES)
	STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MINFILTER);
	STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MAGFILTER);
#endif
	STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
	STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);

	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, dwFogEnable);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, dwLighting);

// #ifdef ENABLE_TRANSLATOR_GOOGLE_SYSTEM
	int lx = gs_mx - m_v3Position.x;
	int ly = gs_my - m_v3Position.y;

	//븘엻 醫뚰몴 遺샇瑜 諛붽퓭以떎.
	if (GetDefaultCodePage() == CP_ARABIC) {
		lx = -lx;
		ly = -ly + m_textHeight;
	}

	if (lx >= 0 && ly >= 0 && lx < m_textWidth && ly < m_textHeight)
	{
		gs_text = m_stText;
	}
// #endif

	//湲덇컯寃 留곹겕 쓣썙二쇰뒗 遺遺.
	if (m_hyperlinkVector.size() != 0)
	{
		int lx = gs_mx - m_v3Position.x;
		int ly = gs_my - m_v3Position.y;

		//븘엻 醫뚰몴 遺샇瑜 諛붽퓭以떎.
		if (GetDefaultCodePage() == CP_ARABIC) {
			lx = -lx;
			ly = -ly + m_textHeight;
		}

		if (lx >= 0 && ly >= 0 && lx < m_textWidth && ly < m_textHeight)
		{
			std::vector<SHyperlink>::iterator it = m_hyperlinkVector.begin();

			while (it != m_hyperlinkVector.end())
			{
				SHyperlink & link = *it++;
				if (lx >= link.sx && lx < link.ex)
				{
					gs_hyperlinkText = link.text;
					/*
					OutputDebugStringW(link.text.c_str());
					OutputDebugStringW(L"\n");
					*/
					break;
				}
			}
		}
	}
#ifdef ENABLE_TEXT_IMAGE_LINE
	if (m_emojiVector.size() != 0)
	{
		for (std::vector<SEmoji>::iterator itor = m_emojiVector.begin(); itor != m_emojiVector.end(); ++itor)
		{
			SEmoji& rEmo = *itor;
			if (rEmo.pInstance)
			{
				rEmo.pInstance->SetPosition(fStanX + rEmo.x, (fStanY + 7.0) - (rEmo.pInstance->GetHeight() / 2));
				rEmo.pInstance->Render();
			}
		}
	}
#endif
}

void CGraphicTextInstance::CreateSystem(UINT uCapacity)
{
	ms_kPool.Create(uCapacity);
}

void CGraphicTextInstance::DestroySystem()
{
	ms_kPool.Destroy();
}

CGraphicTextInstance* CGraphicTextInstance::New()
{
	return ms_kPool.Alloc();
}

void CGraphicTextInstance::Delete(CGraphicTextInstance* pkInst)
{
	pkInst->Destroy();
	ms_kPool.Free(pkInst);
}

void CGraphicTextInstance::ShowCursor()
{
	m_isCursor = true;
}

void CGraphicTextInstance::HideCursor()
{
	m_isCursor = false;
}

void CGraphicTextInstance::ShowOutLine()
{
	m_isOutline = true;
}

void CGraphicTextInstance::HideOutLine()
{
	m_isOutline = false;
}

void CGraphicTextInstance::SetColor(DWORD color)
{
	if (m_dwTextColor != color)
	{
		for (int i = 0; i < m_pCharInfoVector.size(); ++i)
			if (m_dwColorInfoVector[i] == m_dwTextColor)
				m_dwColorInfoVector[i] = color;

		m_dwTextColor = color;
	}
}

void CGraphicTextInstance::SetColor(float r, float g, float b, float a)
{
	SetColor(D3DXCOLOR(r, g, b, a));
}

void CGraphicTextInstance::SetOutLineColor(DWORD color)
{
	m_dwOutLineColor=color;
}

void CGraphicTextInstance::SetOutLineColor(float r, float g, float b, float a)
{
	m_dwOutLineColor=D3DXCOLOR(r, g, b, a);
}

void CGraphicTextInstance::SetSecret(bool Value)
{
	m_isSecret = Value;
}

void CGraphicTextInstance::SetOutline(bool Value)
{
	m_isOutline = Value;
}

void CGraphicTextInstance::SetFeather(bool Value)
{
	if (Value)
	{
		m_fFontFeather = c_fFontFeather;
	}
	else
	{
		m_fFontFeather = 0.0f;
	}
}

void CGraphicTextInstance::SetMultiLine(bool Value)
{
	m_isMultiLine = Value;
}

void CGraphicTextInstance::SetHorizonalAlign(int hAlign)
{
	m_hAlign = hAlign;
}

void CGraphicTextInstance::SetVerticalAlign(int vAlign)
{
	m_vAlign = vAlign;
}

void CGraphicTextInstance::SetMax(int iMax)
{
	m_iMax = iMax;
}

void CGraphicTextInstance::SetLimitWidth(float fWidth)
{
	m_fLimitWidth = fWidth;
}

void CGraphicTextInstance::SetValueString(const string& c_stValue)
{
	if (0 == m_stText.compare(c_stValue))
		return;

	m_stText = c_stValue;
#ifdef ENABLE_MULTI_TEXTLINE
	CheckMultiLine();
#endif
	m_isUpdate = false;
}

void CGraphicTextInstance::SetValue(const char* c_szText, size_t len)
{
	if (0 == m_stText.compare(c_szText))
		return;

	m_stText = c_szText;
#ifdef ENABLE_MULTI_TEXTLINE
	CheckMultiLine();
#endif
	m_isUpdate = false;
}

void CGraphicTextInstance::SetPosition(float fx, float fy, float fz)
{
	m_v3Position.x = fx;
	m_v3Position.y = fy;
	m_v3Position.z = fz;
#ifdef ENABLE_MULTI_TEXTLINE
	for (size_t i = 0; i < multi_text.size(); i++)
		multi_text.at(i)->SetPosition(fx, fy + (i+1) * 15.0f, fz);
#endif
}

void CGraphicTextInstance::SetTextPointer(CGraphicText* pText)
{
	m_roText = pText;
}

const std::string & CGraphicTextInstance::GetValueStringReference()
{
	return m_stText;
}

WORD CGraphicTextInstance::GetTextLineCount()
{
	CGraphicFontTexture::TCharacterInfomation* pCurCharInfo;
	CGraphicFontTexture::TPCharacterInfomationVector::iterator itor;

	float fx = 0.0f;
	WORD wLineCount = 1;
	for (itor=m_pCharInfoVector.begin(); itor!=m_pCharInfoVector.end(); ++itor)
	{
		pCurCharInfo = *itor;

		float fFontWidth=float(pCurCharInfo->width);
		float fFontAdvance=float(pCurCharInfo->advance);
		//float fFontHeight=float(pCurCharInfo->height);

		// NOTE : 룿듃 異쒕젰뿉 Width 젣븳쓣 몼땲떎. - [levites]
		if (fx+fFontWidth > m_fLimitWidth)
		{
			fx = 0.0f;
			++wLineCount;
		}

		fx += fFontAdvance;
	}

	return wLineCount;
}

void CGraphicTextInstance::GetTextSize(int* pRetWidth, int* pRetHeight)
{
	*pRetWidth = m_textWidth;
	*pRetHeight = m_textHeight;
}

#ifdef ENABLE_SUNG_MAHI_TOWER
void CGraphicTextInstance::GetCharacterWidth(short* sWidth)
{
	CGraphicFontTexture::TCharacterInfomation* pCurCharInfo;
	CGraphicFontTexture::TPCharacterInfomationVector::iterator itor;

	*sWidth = 0;
	for (itor = m_pCharInfoVector.begin(); itor != m_pCharInfoVector.end(); ++itor)
	{
		pCurCharInfo = *itor;
		
		*sWidth = pCurCharInfo->width;
		break;
	}
}
#endif

#ifdef ENABLE_INGAME_WIKI
DWORD CGraphicTextInstance::GetColor() const
{
	return m_dwTextColor;
}

void CGraphicTextInstance::SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom)
{
	if (m_textWidth == 0 || m_textHeight == 0)
		return;

	m_bUseRenderingRect = true;

	float fWidth = float(m_textWidth);
	float fHeight = float(m_textHeight);

	m_RenderingRect.left = static_cast<LONG>(fWidth * fLeft);
	m_RenderingRect.top = static_cast<LONG>(fHeight * fTop);
	m_RenderingRect.right = static_cast<LONG>(fWidth * fRight);
	m_RenderingRect.bottom = static_cast<LONG>(fHeight * fBottom);
}

void CGraphicTextInstance::iSetRenderingRect(int iLeft, int iTop, int iRight, int iBottom)
{
	if (m_textWidth == 0 || m_textHeight == 0)
		return;

	m_bUseRenderingRect = true;

	m_RenderingRect.left = iLeft;
	m_RenderingRect.top = iTop;
	m_RenderingRect.right = iRight;
	m_RenderingRect.bottom = iBottom;
}

void CGraphicTextInstance::SetRenderBox(RECT& renderBox)
{
	memcpy(&m_renderBox, &renderBox, sizeof(m_renderBox));
}
#endif

int CGraphicTextInstance::PixelPositionToCharacterPosition(int iPixelPosition)
{
	int icurPosition = 0;
	for (int i = 0; i < (int)m_pCharInfoVector.size(); ++i)
	{
		CGraphicFontTexture::TCharacterInfomation* pCurCharInfo = m_pCharInfoVector[i];
		icurPosition += pCurCharInfo->width;

		if (iPixelPosition < icurPosition)
			return i;
	}

	return -1;
}

int CGraphicTextInstance::GetHorizontalAlign()
{
	return m_hAlign;
}

void CGraphicTextInstance::__Initialize()
{
	m_roText = NULL;

	m_hAlign = HORIZONTAL_ALIGN_LEFT;
	m_vAlign = VERTICAL_ALIGN_TOP;

	m_iMax = 0;
	m_fLimitWidth = 1600.0f; // NOTE : 빐긽룄쓽 理쒕移. 씠蹂대떎 湲멸쾶 벝 씪씠 엳쓣源? - [levites]

	m_isCursor = false;
	m_isSecret = false;
	m_isMultiLine = false;

	m_isOutline = false;
	m_fFontFeather = c_fFontFeather;

	m_isUpdate = false;
#ifdef ENABLE_MULTI_TEXTLINE
	m_EnterToken = false; // default, you can change
#endif
	m_textWidth = 0;
	m_textHeight = 0;
	m_textLineHeight = 12;
	m_v3Position.x = m_v3Position.y = m_v3Position.z = 0.0f;

	m_dwOutLineColor=0xff000000;

#ifdef ENABLE_INGAME_WIKI
	memset(&m_RenderingRect, 0, sizeof(RECT));
	m_bUseRenderingRect = false;
	
	memset(&m_renderBox, 0, sizeof(m_renderBox));
	m_startPos = m_endPos = 0;
	m_isFixedRenderPos = false;
#endif
}

void CGraphicTextInstance::Destroy()
{
	m_stText="";
	m_pCharInfoVector.clear();
	m_dwColorInfoVector.clear();
	m_hyperlinkVector.clear();
#ifdef ENABLE_MULTI_TEXTLINE
	multi_text.clear();
#endif
#ifdef ENABLE_TEXT_IMAGE_LINE
	if (m_emojiVector.size() != 0)
	{
		for (std::vector<SEmoji>::iterator itor = m_emojiVector.begin(); itor != m_emojiVector.end(); ++itor)
		{
			SEmoji& rEmo = *itor;
			if (rEmo.pInstance)
			{
				CGraphicImageInstance::Delete(rEmo.pInstance);
				rEmo.pInstance = NULL;
			}
		}
	}
	m_emojiVector.clear();
#endif
	__Initialize();
}

CGraphicTextInstance::CGraphicTextInstance()
{
	__Initialize();
}

CGraphicTextInstance::~CGraphicTextInstance()
{
	Destroy();
}
