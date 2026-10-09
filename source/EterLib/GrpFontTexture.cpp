#include "StdAfx.h"
#include "GrpText.h"
#include "../eterBase/Stl.h"

#include "Util.h"

#if defined(__ANDROID__) || defined(__APPLE__) || defined(__IOS__)
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#include <vector>
#include <string>
#include "../eterPack/EterPackManager.h"

#if defined(__ANDROID__)
#include <android/log.h>
#include <android/asset_manager.h>
#define FONT_LOG(...) __android_log_print(ANDROID_LOG_INFO, "Metin2Font", __VA_ARGS__)
extern AAssetManager* g_pAssetManager;
#else
#include <stdio.h>
#define FONT_LOG(...) printf("[Metin2Font] " __VA_ARGS__)
extern std::string g_strDocsPath;
extern std::string g_strBundlePath;
#endif

static std::vector<unsigned char> s_fontBuffer;
static stbtt_fontinfo s_fontInfo;
static bool s_fontLoaded = false;

static bool EnsureFontLoaded()
{
	if (s_fontLoaded)
		return true;

	// 0. Try loading Tahoma/Arial font from Metin2 pack via CEterPackManager
	CMappedFile mapFile;
	LPCVOID pData = nullptr;
	const char* packFontPaths[] = {
		"locale/common/font/tahoma.ttf",
		"locale/common/font/arial.ttf",
		"tahoma.ttf",
		"arial.ttf"
	};
	for (const char* pFont : packFontPaths)
	{
		if (CEterPackManager::Instance().Get(mapFile, pFont, &pData) && pData && mapFile.Size() > 0)
		{
			s_fontBuffer.assign((const unsigned char*)pData, (const unsigned char*)pData + mapFile.Size());
			int offset = stbtt_GetFontOffsetForIndex(s_fontBuffer.data(), 0);
			if (offset < 0) offset = 0;
			if (stbtt_InitFont(&s_fontInfo, s_fontBuffer.data(), offset))
			{
				s_fontLoaded = true;
				FONT_LOG("Successfully loaded %s from pack! (%zu bytes)\n", pFont, s_fontBuffer.size());
				return true;
			}
		}
	}

#if defined(__ANDROID__)
	// 1. Try loading Tahoma font from APK assets via AAssetManager
	if (g_pAssetManager)
	{
		const char* assetNames[] = { "tahoma.ttf", "arial.ttf" };
		for (const char* assetName : assetNames)
		{
			AAsset* asset = AAssetManager_open(g_pAssetManager, assetName, AASSET_MODE_BUFFER);
			if (asset)
			{
				off_t sz = AAsset_getLength(asset);
				if (sz > 0)
				{
					s_fontBuffer.resize(sz);
					int readBytes = AAsset_read(asset, s_fontBuffer.data(), sz);
					AAsset_close(asset);
					int offset = stbtt_GetFontOffsetForIndex(s_fontBuffer.data(), 0);
					if (offset < 0) offset = 0;
					if (readBytes == sz && stbtt_InitFont(&s_fontInfo, s_fontBuffer.data(), offset))
					{
						s_fontLoaded = true;
						FONT_LOG("Successfully loaded %s from APK assets! (%ld bytes)\n", assetName, (long)sz);
						return true;
					}
				}
				else
				{
					AAsset_close(asset);
				}
			}
		}
	}
#endif

	// 2. Try loading font from filesystem
	std::vector<std::string> filePaths = {
		"pack/tahoma.ttf",
		"tahoma.ttf",
		"pack/arial.ttf",
		"arial.ttf"
	};
#if defined(__ANDROID__)
	filePaths.push_back("/sdcard/aspar2/tahoma.ttf");
	filePaths.push_back("/storage/emulated/0/aspar2/tahoma.ttf");
	filePaths.push_back("/sdcard/metin2/tahoma.ttf");
	filePaths.push_back("/storage/emulated/0/metin2/tahoma.ttf");
#endif
#if defined(__APPLE__) || defined(__IOS__)
	if (!g_strDocsPath.empty()) {
		filePaths.push_back(g_strDocsPath + "/tahoma.ttf");
		filePaths.push_back(g_strDocsPath + "/pack/tahoma.ttf");
		filePaths.push_back(g_strDocsPath + "/arial.ttf");
		filePaths.push_back(g_strDocsPath + "/aspar2/pack/tahoma.ttf");
	}
	if (!g_strBundlePath.empty()) {
		filePaths.push_back(g_strBundlePath + "/tahoma.ttf");
		filePaths.push_back(g_strBundlePath + "/arial.ttf");
		filePaths.push_back(g_strBundlePath + "/assets/tahoma.ttf");
	}
#endif

	for (const std::string& path : filePaths)
	{
		FILE* f = fopen(path.c_str(), "rb");
		if (f)
		{
			fseek(f, 0, SEEK_END);
			long sz = ftell(f);
			fseek(f, 0, SEEK_SET);
			if (sz > 0)
			{
				s_fontBuffer.resize(sz);
				size_t readBytes = fread(s_fontBuffer.data(), 1, sz, f);
				fclose(f);
				int offset = stbtt_GetFontOffsetForIndex(s_fontBuffer.data(), 0);
				if (offset < 0) offset = 0;
				if (readBytes == (size_t)sz && stbtt_InitFont(&s_fontInfo, s_fontBuffer.data(), offset))
				{
					s_fontLoaded = true;
					FONT_LOG("Successfully loaded font from: %s (%ld bytes)\n", path.c_str(), sz);
					return true;
				}
			}
			else
			{
				fclose(f);
			}
		}
	}

	// 3. Fallback to system fonts
#if defined(__ANDROID__)
	const char* fontPaths[] = {
		"/system/fonts/Roboto-Regular.ttf",
		"/system/fonts/RobotoStatic-Regular.ttf",
		"/system/fonts/DroidSans.ttf"
	};
#elif defined(__APPLE__) || defined(__IOS__)
	const char* fontPaths[] = {
		"/System/Library/Fonts/Core/Arial.ttf",
		"/System/Library/Fonts/CoreAddition/Arial.ttf",
		"/System/Library/Fonts/Cache/Arial.ttf",
		"/System/Library/Fonts/Helvetica.ttc",
		"/System/Library/Fonts/AppleSDGothicNeo.ttc"
	};
#else
	const char* fontPaths[] = {};
#endif

	for (const char* path : fontPaths)
	{
		FILE* f = fopen(path, "rb");
		if (f)
		{
			fseek(f, 0, SEEK_END);
			long sz = ftell(f);
			fseek(f, 0, SEEK_SET);
			if (sz > 0)
			{
				s_fontBuffer.resize(sz);
				size_t readBytes = fread(s_fontBuffer.data(), 1, sz, f);
				fclose(f);
				int offset = stbtt_GetFontOffsetForIndex(s_fontBuffer.data(), 0);
				if (offset < 0) offset = 0;
				if (readBytes == (size_t)sz && stbtt_InitFont(&s_fontInfo, s_fontBuffer.data(), offset))
				{
					s_fontLoaded = true;
					FONT_LOG("Fallback: loaded system font: %s (%ld bytes)\n", path, sz);
					return true;
				}
			}
			else
			{
				fclose(f);
			}
		}
	}

	FONT_LOG("ERROR: Failed to load any font!\n");
	return false;
}
#endif

CGraphicFontTexture::CGraphicFontTexture()
{
	Initialize();
}

CGraphicFontTexture::~CGraphicFontTexture()
{
	Destroy();
}

void CGraphicFontTexture::Initialize()
{
	CGraphicTexture::Initialize();
	m_hFontOld = NULL;
	m_hFont = NULL;
	m_isDirty = false;
	m_bItalic = false;
}

bool CGraphicFontTexture::IsEmpty() const
{
#if defined(__ANDROID__) || defined(__APPLE__) || defined(__IOS__)
	return m_pFontTextureVector.empty();
#else
	return m_fontMap.size() == 0;
#endif
}

void CGraphicFontTexture::Destroy()
{
#if !defined(__ANDROID__) && !defined(__APPLE__) && !defined(__IOS__)
	HDC hDC = m_dib.GetDCHandle();
	if (hDC)
		SelectObject(hDC, m_hFontOld);
#endif

	m_dib.Destroy();

	m_lpd3dTexture = NULL;
	CGraphicTexture::Destroy();
	stl_wipe(m_pFontTextureVector);
	m_charInfoMap.clear();

	if (m_fontMap.size())
	{
		TFontMap::iterator i = m_fontMap.begin();

		while(i != m_fontMap.end())
		{
			DeleteObject((HGDIOBJ)i->second);
			++i;
		}

		m_fontMap.clear();
	}
	
	Initialize();
}

bool CGraphicFontTexture::CreateDeviceObjects()
{
	return true;
}

void CGraphicFontTexture::DestroyDeviceObjects()
{
}

bool CGraphicFontTexture::Create(const char* c_szFontName, int fontSize, bool bItalic)
{
	Destroy();
	
	strncpy(m_fontName, c_szFontName, sizeof(m_fontName)-1);
	m_fontSize	= fontSize;
	m_bItalic	= bItalic;

	m_x = 0;
	m_y = 0;
	m_step = 0;

	DWORD width = 256,height = 256;
	if (GetMaxTextureWidth() > 512)
		width = 512;
	if (GetMaxTextureHeight() > 512)
		height = 512;
	
	if (!m_dib.Create(ms_hDC, width, height))
		return false;

#if defined(__ANDROID__) || defined(__APPLE__) || defined(__IOS__)
	EnsureFontLoaded();
	m_fontMap[GetDefaultCodePage()] = (HFONT)1;
#else
	HDC hDC = m_dib.GetDCHandle();

	m_hFont = GetFont(GetDefaultCodePage());

	m_hFontOld=(HFONT)SelectObject(hDC, m_hFont);
	SetTextColor(hDC, RGB(255, 255, 255));
	SetBkColor(hDC,	0);
#endif

	if (!AppendTexture())
		return false;
	
	return true;
}



HFONT CGraphicFontTexture::GetFont(WORD codePage)
{
	HFONT hFont = NULL;
	TFontMap::iterator i = m_fontMap.find(codePage);

	if(i != m_fontMap.end())
	{
		hFont = i->second;
	}
	else
	{
		LOGFONT logFont;

		memset(&logFont, 0, sizeof(LOGFONT));

		logFont.lfHeight			= m_fontSize;
		logFont.lfEscapement		= 0;
		logFont.lfOrientation		= 0;
		logFont.lfWeight			= FW_NORMAL;
		logFont.lfItalic			= (BYTE) m_bItalic;
		logFont.lfUnderline			= FALSE;
		logFont.lfStrikeOut			= FALSE;
		logFont.lfCharSet			= GetCharsetFromCodePage(codePage);
		logFont.lfOutPrecision		= OUT_DEFAULT_PRECIS;
		logFont.lfClipPrecision		= CLIP_DEFAULT_PRECIS;
		logFont.lfQuality			= ANTIALIASED_QUALITY;
		logFont.lfPitchAndFamily	= DEFAULT_PITCH;
		//Tracenf("font: %s", GetFontFaceFromCodePage(codePage));
		strcpy(logFont.lfFaceName, m_fontName); //GetFontFaceFromCodePage(codePage));
		//strcpy(logFont.lfFaceName, GetFontFaceFromCodePage(codePage));

		hFont = CreateFontIndirect(&logFont);

		m_fontMap.insert(TFontMap::value_type(codePage, hFont));
	}

	return hFont;
}

bool CGraphicFontTexture::AppendTexture()
{
	CGraphicImageTexture * pNewTexture = new CGraphicImageTexture;

	if (!pNewTexture->Create(m_dib.GetWidth(), m_dib.GetHeight(), D3DFMT_A4R4G4B4))
	{
		delete pNewTexture;
		return false;
	}

	m_pFontTextureVector.push_back(pNewTexture);
	return true;
}

bool CGraphicFontTexture::UpdateTexture()
{
	if(!m_isDirty)
		return true;

	m_isDirty = false;

	CGraphicImageTexture * pFontTexture = m_pFontTextureVector.back();

	if (!pFontTexture)
		return false;

	WORD* pwDst;
	int pitch;

	if (!pFontTexture->Lock(&pitch, (void**)&pwDst))
		return false;

	pitch /= 2;

	int width = m_dib.GetWidth();
	int height = m_dib.GetHeight();

	DWORD * pdwSrc = (DWORD*)m_dib.GetPointer();

	for (int y = 0; y < height; ++y, pwDst += pitch, pdwSrc += width)
		for (int x = 0; x < width; ++x)
			pwDst[x]=pdwSrc[x];
	
	pFontTexture->Unlock();
	return true;
}

CGraphicFontTexture::TCharacterInfomation* CGraphicFontTexture::GetCharacterInfomation(WORD codePage, wchar_t keyValue)
{
	TCharacterKey code(codePage, keyValue);

	TCharacterInfomationMap::iterator f = m_charInfoMap.find(code);

	if (m_charInfoMap.end() == f)
	{
		return UpdateCharacterInfomation(code);
	}
	else
	{
		return &f->second;	
	}
}

CGraphicFontTexture::TCharacterInfomation* CGraphicFontTexture::UpdateCharacterInfomation(TCharacterKey code)
{
#if defined(__ANDROID__) || defined(__APPLE__) || defined(__IOS__)
	if (!EnsureFontLoaded())
		return NULL;

	wchar_t keyValue = code.second;
	if (keyValue == 0x08)
		keyValue = L' ';

	int effectiveFontSize = abs((int)m_fontSize);
	if (effectiveFontSize <= 0)
		effectiveFontSize = 12;

	float scale = stbtt_ScaleForPixelHeight(&s_fontInfo, (float)effectiveFontSize);
	int ascent = 0, descent = 0, lineGap = 0;
	stbtt_GetFontVMetrics(&s_fontInfo, &ascent, &descent, &lineGap);
	int baseline = (int)(ascent * scale + 0.5f);
	int fontLineHeight = (int)((ascent - descent) * scale + 0.5f);
	if (fontLineHeight < effectiveFontSize)
		fontLineHeight = effectiveFontSize;

	int advanceWidth = 0, leftSideBearing = 0;
	stbtt_GetCodepointHMetrics(&s_fontInfo, (int)keyValue, &advanceWidth, &leftSideBearing);
	int advance = (int)(advanceWidth * scale + 0.5f);

	int ix0 = 0, iy0 = 0, ix1 = 0, iy1 = 0;
	stbtt_GetCodepointBitmapBox(&s_fontInfo, (int)keyValue, scale, scale, &ix0, &iy0, &ix1, &iy1);
	int glyphW = ix1 - ix0;
	int glyphH = iy1 - iy0;

	if (keyValue == L' ')
	{
		glyphW = 0;
		glyphH = 0;
		if (advance <= 0)
			advance = effectiveFontSize / 3;
	}

	if (advance <= 0 && glyphW == 0)
	{
		advance = effectiveFontSize / 2;
	}

	int glyphTop = baseline + iy0;
	if (glyphTop < 0) glyphTop = 0;
	int glyphLeft = (ix0 > 0) ? ix0 : 0;

	int cellW = max(advance, glyphLeft + glyphW) + 1;
	if (cellW < 2) cellW = 2;
	int cellH = max(fontLineHeight, glyphTop + glyphH) + 1;
	if (cellH < effectiveFontSize) cellH = effectiveFontSize;

	int width = m_dib.GetWidth();
	int height = m_dib.GetHeight();

	if (m_x + cellW >= (width - 1))
	{
		m_y += (m_step + 1);
		m_step = 0;
		m_x = 0;

		if (m_y + cellH >= (height - 1))
		{
			if (!UpdateTexture())
			{
				return NULL;
			}

			if (!AppendTexture())
				return NULL;

			m_y = 0;
		}
	}

	int nDIBWidth = m_dib.GetWidth();
	DWORD* pdwDIBData = (DWORD*)m_dib.GetPointer();

	// Clear slot in DIB
	for (int dy = 0; dy < cellH; ++dy)
	{
		int cy = m_y + dy;
		if (cy >= height) break;
		DWORD* row = pdwDIBData + cy * nDIBWidth;
		for (int dx = 0; dx < cellW; ++dx)
		{
			int cx = m_x + dx;
			if (cx >= width) break;
			row[cx] = 0;
		}
	}

	// Rasterize glyph bitmap
	if (glyphW > 0 && glyphH > 0)
	{
		std::vector<unsigned char> monoBitmap(glyphW * glyphH, 0);
		stbtt_MakeCodepointBitmap(&s_fontInfo, monoBitmap.data(), glyphW, glyphH, glyphW, scale, scale, (int)keyValue);

		for (int gy = 0; gy < glyphH; ++gy)
		{
			int dstY = m_y + glyphTop + gy;
			if (dstY >= height) break;
			DWORD* row = pdwDIBData + dstY * nDIBWidth;

			for (int gx = 0; gx < glyphW; ++gx)
			{
				int dstX = m_x + glyphLeft + gx;
				if (dstX >= width) break;

				unsigned char alpha = monoBitmap[gy * glyphW + gx];
				if (alpha <= 6)
				{
					row[dstX] = 0;
				}
				else
				{
					// Clean, smooth true 4-bit alpha (1..15) matching original Metin2 font rendering
					int a4 = (int)alpha * 15 / 255;
					if (a4 < 1) a4 = 1;
					row[dstX] = (DWORD)(0xfff0 | (a4 & 0x0f));
				}
			}
		}
	}

	float rhwidth = 1.0f / float(width);
	float rhheight = 1.0f / float(height);

	TCharacterInfomation& rNewCharInfo = m_charInfoMap[code];

	rNewCharInfo.index = m_pFontTextureVector.size() - 1;
	rNewCharInfo.width = cellW;
	rNewCharInfo.height = cellH;
	rNewCharInfo.left = float(m_x) * rhwidth;
	rNewCharInfo.top = float(m_y) * rhheight;
	rNewCharInfo.right = float(m_x + cellW) * rhwidth;
	rNewCharInfo.bottom = float(m_y + cellH) * rhheight;
	rNewCharInfo.advance = (float)advance;

	m_x += cellW;

	if (m_step < cellH)
		m_step = cellH;

	m_isDirty = true;

	return &rNewCharInfo;
#else
	HDC hDC = m_dib.GetDCHandle();
	SelectObject(hDC, GetFont(code.first));

	wchar_t keyValue = code.second;

	if (keyValue == 0x08)
		keyValue = L' '; // 탭은 공백으로 바꾼다 (아랍 출력시 탭 사용: NAME:\tTEXT -> TEXT\t:NAME 로 전환됨 )

	ABCFLOAT	stABC;
	SIZE		size;

	if (!GetTextExtentPoint32W(hDC, &keyValue, 1, &size) || !GetCharABCWidthsFloatW(hDC, keyValue, keyValue, &stABC))
		return NULL;

	size.cx = stABC.abcfB;
	if( stABC.abcfA > 0.0f )
		size.cx += ceilf(stABC.abcfA);
	if( stABC.abcfC > 0.0f )
		size.cx += ceilf(stABC.abcfC);
	size.cx++;

	LONG lAdvance = ceilf( stABC.abcfA + stABC.abcfB + stABC.abcfC );

	int width = m_dib.GetWidth();
	int height = m_dib.GetHeight();

	if (m_x + size.cx >= (width - 1))
	{
		m_y += (m_step + 1);
		m_step = 0;
		m_x = 0;

		if (m_y + size.cy >= (height - 1))
		{
			if (!UpdateTexture())
			{
				return NULL;
			}

			if (!AppendTexture())
				return NULL;

			m_y = 0;
		}
	}

	TextOutW(hDC, m_x, m_y, &keyValue, 1);
		
	int nChrX;
	int nChrY;
	int nChrWidth = size.cx;
	int nChrHeight = size.cy;
	int nDIBWidth = m_dib.GetWidth();

	
	DWORD*pdwDIBData=(DWORD*)m_dib.GetPointer();		
	DWORD*pdwDIBBase=pdwDIBData+nDIBWidth*m_y+m_x;
	DWORD*pdwDIBRow;
	
	pdwDIBRow=pdwDIBBase;
	for (nChrY=0; nChrY<nChrHeight; ++nChrY, pdwDIBRow+=nDIBWidth)
	{			
		for (nChrX=0; nChrX<nChrWidth; ++nChrX)
		{
			pdwDIBRow[nChrX]=(pdwDIBRow[nChrX]&0xff) ? 0xffff : 0;
		}
	}

	float rhwidth = 1.0f / float(width);
	float rhheight = 1.0f / float(height);

	TCharacterInfomation& rNewCharInfo = m_charInfoMap[code];

	rNewCharInfo.index = m_pFontTextureVector.size() - 1;
	rNewCharInfo.width = size.cx;
	rNewCharInfo.height = size.cy;
	rNewCharInfo.left = float(m_x) * rhwidth;
	rNewCharInfo.top = float(m_y) * rhheight;
	rNewCharInfo.right = float(m_x+size.cx) * rhwidth;
	rNewCharInfo.bottom = float(m_y+size.cy) * rhheight;
	rNewCharInfo.advance = (float) lAdvance;

	m_x += size.cx;

	if (m_step < size.cy)
		m_step = size.cy;	

	m_isDirty = true;

	return &rNewCharInfo;
#endif
}

bool CGraphicFontTexture::CheckTextureIndex(DWORD dwTexture)
{
	if (dwTexture >= m_pFontTextureVector.size())
		return false;

	return true;
}

void CGraphicFontTexture::SelectTexture(DWORD dwTexture)
{
	assert(CheckTextureIndex(dwTexture));
	m_lpd3dTexture = m_pFontTextureVector[dwTexture]->GetD3DTexture();
}

#ifdef ENABLE_FIX_MOBS_LAG
CGraphicImageTexture* CGraphicFontTexture::GetTexture(DWORD dwTexture)
{
	assert(CheckTextureIndex(dwTexture));
	return m_pFontTextureVector[dwTexture];
}
#endif
