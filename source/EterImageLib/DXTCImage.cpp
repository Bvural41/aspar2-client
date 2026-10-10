// need the d3d.h for things in format of .dds file
#include "StdAfx.h"

#include <d3d.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../EterBase/MappedFile.h"
#include "../EterBase/Debug.h"

#include "DXTCImage.h"

struct DXTColBlock
{
	WORD col0;
	WORD col1;
	
	// no bit fields - use bytes
	BYTE row[4];
};

struct DXTAlphaBlockExplicit
{
	WORD row[4];
};

struct DXTAlphaBlock3BitLinear
{
	BYTE alpha0;
	BYTE alpha1;
	
	BYTE stuff[6];
};

// use cast to struct instead of RGBA_MAKE as struct is much
struct Color8888
{
	BYTE b;		// Last one is MSB, 1st is LSB.
	BYTE g;		// order of the output ARGB or BGRA, etc...
	BYTE r;		// change the order of names to change the 
	BYTE a;
};

struct Color565
{
	unsigned nBlue  : 5;		// order of names changes
	unsigned nGreen : 6;		//  byte order of output to 32 bit
	unsigned nRed	: 5;
};

/////////////////////////////////////
// should be in ddraw.h
#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                                      \
				((DWORD)(BYTE) (ch0       ) | ((DWORD)(BYTE) (ch1) <<  8) | \
				((DWORD)(BYTE) (ch2) << 16) | ((DWORD)(BYTE) (ch3) << 24))
#endif // defined(MAKEFOURCC)

CDXTCImage::CDXTCImage()
{
	Initialize();
}

CDXTCImage::~CDXTCImage()
{
}

void CDXTCImage::Initialize()
{
	m_nWidth = 0;
	m_nHeight = 0;

	for (int i = 0; i < MAX_MIPLEVELS; ++i)
		m_pbCompBufferByLevels[i] = NULL;
}

void CDXTCImage::Clear()
{
	for (int i = 0; i < MAX_MIPLEVELS; ++i)
		m_bCompVector[i].clear();

	Initialize();
}

bool CDXTCImage::LoadFromFile(const char * filename)
{
	// only understands .dds files for now
	// return true if success
	char * exts[] = { ".DDS" };
	int next = 1;
	
	static char fileupper[MAX_PATH+1];
	
	strncpy(fileupper, filename, MAX_PATH); 
	_strupr(fileupper);
	
	int i;
	bool knownformat = false;
	
	for (i = 0; i < next; ++i)
	{
		char * found = strstr(fileupper, exts[0]);
		
		if (found != NULL)
		{
			knownformat = true;
			break;
		}
	}
	
	if (knownformat == false)
	{
		Tracef("Unknown file format encountered! [%s]\n", filename);
		return(false);
	}

	CMappedFile mappedFile;
	LPCVOID pvMap;

	if (!mappedFile.Create(filename, &pvMap, 0, 0))
	{
		Tracef("Can't open file for reading! [%s]\n", filename);
		return false;
	}

	return LoadFromMemory((const BYTE*) pvMap);
}

bool CDXTCImage::LoadHeaderFromMemory(const BYTE * c_pbMap)
{
	Clear();
	if (!c_pbMap)
		return false;

	DWORD dwMagic;
	dwMagic = *(DWORD *) c_pbMap;
	c_pbMap += sizeof(DWORD);

	if (dwMagic != MAKEFOURCC('D','D','S',' '))
		return false;
	
	DDSURFACEDESC2 ddsd; // read from dds file
	
	// Read the surface description
	memcpy(&ddsd, c_pbMap, sizeof(DDSURFACEDESC2));
	c_pbMap += sizeof(DDSURFACEDESC2);
	
	// Does texture have mipmaps?
	m_bMipTexture = (ddsd.dwMipMapCount > 0) ? TRUE : FALSE;
	
	// Clear unwanted flags
	// Can't do this!!!  surface not re-created here
	//    ddsd.dwFlags &= (~DDSD_PITCH);
	//    ddsd.dwFlags &= (~DDSD_LINEARSIZE);

	// Is it DXTC ?
	// I sure hope pixelformat is valid!
	m_xddPixelFormat.dwFlags = ddsd.ddpfPixelFormat.dwFlags;
	m_xddPixelFormat.dwFourCC = ddsd.ddpfPixelFormat.dwFourCC;
	m_xddPixelFormat.dwSize = ddsd.ddpfPixelFormat.dwSize;
	m_xddPixelFormat.dwRGBBitCount = ddsd.ddpfPixelFormat.dwRGBBitCount;
	m_xddPixelFormat.dwRGBAlphaBitMask = ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;
	m_xddPixelFormat.dwRBitMask = ddsd.ddpfPixelFormat.dwRBitMask;
	m_xddPixelFormat.dwGBitMask = ddsd.ddpfPixelFormat.dwGBitMask;
	m_xddPixelFormat.dwBBitMask = ddsd.ddpfPixelFormat.dwBBitMask;

	DecodePixelFormat(m_strFormat, &m_xddPixelFormat);
	
	if (m_CompFormat != PF_DXT1 &&
		m_CompFormat != PF_DXT3 &&
		m_CompFormat != PF_DXT5)
	{
		return false;
	}

	if (ddsd.dwMipMapCount > MAX_MIPLEVELS)
		ddsd.dwMipMapCount = MAX_MIPLEVELS;

	m_nWidth		= ddsd.dwWidth;
	m_nHeight		= ddsd.dwHeight;
	//!@#
	m_dwMipMapCount = max(1, ddsd.dwMipMapCount);
	m_dwFlags		= ddsd.dwFlags;

	if (ddsd.dwFlags & DDSD_PITCH)
	{
		m_lPitch = ddsd.lPitch;
		m_pbCompBufferByLevels[0] = c_pbMap;
	}
	else
	{
		m_lPitch = ddsd.dwLinearSize;

		if (ddsd.dwFlags & DDSD_MIPMAPCOUNT)
		{
			for (DWORD dwLinearSize = ddsd.dwLinearSize, i = 0; i < m_dwMipMapCount; ++i, dwLinearSize >>= 2)
			{
				m_pbCompBufferByLevels[i] = c_pbMap;
				c_pbMap += dwLinearSize;
			}
		}
		else
		{
			m_pbCompBufferByLevels[0] = c_pbMap;
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CDXTCImage::LoadFromMemory(const BYTE * c_pbMap)
{
	if (!LoadHeaderFromMemory(c_pbMap))
		return false;

	if (m_dwFlags & DDSD_PITCH)
	{
		DWORD dwBytesPerRow = m_nWidth * m_xddPixelFormat.dwRGBBitCount / 8;

		m_nCompSize = m_lPitch * m_nHeight;
		m_nCompLineSz = dwBytesPerRow;

		m_bCompVector[0].resize(m_nCompSize);
		BYTE * pDest = &m_bCompVector[0][0];

		c_pbMap = m_pbCompBufferByLevels[0];

		for (int yp = 0; yp < m_nHeight; ++yp)
		{
			memcpy(pDest, c_pbMap, dwBytesPerRow);
			pDest += m_lPitch;
			c_pbMap += m_lPitch;
		}
	}
	else
	{
		if (m_dwFlags & DDSD_MIPMAPCOUNT)
		{
			for (DWORD dwLinearSize = m_lPitch, i = 0; i < m_dwMipMapCount; ++i, dwLinearSize >>= 2)
			{
				m_bCompVector[i].resize(dwLinearSize);
				Copy(i, &m_bCompVector[i][0], dwLinearSize);
			}
		}
		else
		{
			m_bCompVector[0].resize(m_lPitch);
			Copy(0, &m_bCompVector[0][0], m_lPitch);
		}
	}

	// done reading file
	return true;
}

bool CDXTCImage::Copy(int miplevel, BYTE * pbDest, long lDestPitch)
{
	if (!(m_dwFlags & DDSD_MIPMAPCOUNT))
		if (miplevel)
			return false;

	/*
	DXTColBlock * pBlock;
	WORD * pPos = (WORD *) &m_pbCompBufferByLevels[miplevel][0];
	int xblocks = (m_nWidth >> miplevel) / 4;
	int yblocks = (m_nHeight >> miplevel) / 4;

	for (int y = 0; y < yblocks; ++y)
	{
		// 8 bytes per block
		pBlock = (DXTColBlock*) ((DWORD) pPos + y * xblocks * 8);

		memcpy(pbDest, pBlock, xblocks * 8);
		pbDest += lDestPitch;
	}
	*/

	memcpy(pbDest, m_pbCompBufferByLevels[miplevel], m_lPitch >> (miplevel * 2));
	pbDest += lDestPitch;
	return true;
}

void CDXTCImage::Unextract(BYTE * pbDest, int /*iWidth*/, int /*iHeight*/, int iPitch)
{
	if (!m_pbCompBufferByLevels[0])
		return;

	DXTColBlock * pBlock;
	BYTE * pPos = (BYTE *) &m_pbCompBufferByLevels[0][0];
	int xblocks = m_nWidth / 4;
	int yblocks = (m_nHeight / 4) * ((iPitch / m_nWidth) / 2);

	for (int y = 0; y < yblocks; ++y)
	{
		pBlock = (DXTColBlock*) (pPos + y * xblocks * 8);

		memcpy(pbDest, pBlock, xblocks * 8);
		pbDest += xblocks * 8;
	}

	/*
	for (int y = 0; y < iHeight; ++y)
	{
		memcpy(pbDest, &m_pbCompBufferByLevels[0][0] + y*iWidth, iWidth);
		pbDest += iWidth;
	}
	*/
}

void CDXTCImage::Decompress(int miplevel, DWORD * pdwDest)
{
	switch (m_CompFormat)
	{
		case PF_DXT1:
			DecompressDXT1(miplevel, pdwDest);
			break;

		case PF_DXT3:
			DecompressDXT3(miplevel, pdwDest);
			break;
			
		case PF_DXT5:
			DecompressDXT5(miplevel, pdwDest);
			break;
			
		case PF_ARGB:
			DecompressARGB(miplevel, pdwDest);
			break;

		case PF_UNKNOWN:
			break;
	}
}

inline void GetColorBlockColors(DXTColBlock * pBlock, Color8888 * col_0, Color8888 * col_1, 
								Color8888 * col_2, Color8888 * col_3,
								WORD & wrd)
{
	WORD c0 = pBlock->col0;
	WORD c1 = pBlock->col1;

	col_0->a = 0xff;
	col_0->r = (BYTE)(((c0 >> 11) & 0x1F) << 3);
	col_0->g = (BYTE)(((c0 >> 5) & 0x3F) << 2);
	col_0->b = (BYTE)((c0 & 0x1F) << 3);

	col_1->a = 0xff;
	col_1->r = (BYTE)(((c1 >> 11) & 0x1F) << 3);
	col_1->g = (BYTE)(((c1 >> 5) & 0x3F) << 2);
	col_1->b = (BYTE)((c1 & 0x1F) << 3);

	if (c0 > c1)
	{
		col_2->r = (BYTE)(((WORD) col_0->r * 2 + (WORD) col_1->r) / 3);
		col_2->g = (BYTE)(((WORD) col_0->g * 2 + (WORD) col_1->g) / 3);
		col_2->b = (BYTE)(((WORD) col_0->b * 2 + (WORD) col_1->b) / 3);
		col_2->a = 0xff;

		col_3->r = (BYTE)(((WORD) col_0->r + (WORD) col_1->r * 2) / 3);
		col_3->g = (BYTE)(((WORD) col_0->g + (WORD) col_1->g * 2) / 3);
		col_3->b = (BYTE)(((WORD) col_0->b + (WORD) col_1->b * 2) / 3);
		col_3->a = 0xff;
	}
	else
	{
		col_2->r = (BYTE)(((WORD) col_0->r + (WORD) col_1->r) / 2);
		col_2->g = (BYTE)(((WORD) col_0->g + (WORD) col_1->g) / 2);
		col_2->b = (BYTE)(((WORD) col_0->b + (WORD) col_1->b) / 2);
		col_2->a = 0xff;

		col_3->r = 0x00;
		col_3->g = 0x00;
		col_3->b = 0x00;
		col_3->a = 0x00;
	}
} // Get color block colors (...)


inline void DecodeColorBlock(DWORD * pImPos,
							 DXTColBlock * pColorBlock,
							 int width,
							 const DWORD * col)
{
	const DWORD masks[] = { 3, 12, 3 << 4, 3 << 6 };
	const int   shift[] = { 0, 2, 4, 6 };
	
	for (int y = 0; y < 4; ++y, pImPos += width - 4)
	{
		for (int n = 0; n < 4; ++n)
		{
			DWORD bits = (pColorBlock->row[y] & masks[n]) >> shift[n];
			*pImPos++ = col[bits];
		}
	}
}

inline void DecodeAlphaExplicit(DWORD * pImPos, DXTAlphaBlockExplicit * pAlphaBlock, int width)
{
	for (int row = 0; row < 4; ++row, pImPos += width - 4)
	{
		WORD wrd = pAlphaBlock->row[row];
		for (int pix = 0; pix < 4; ++pix)
		{
			DWORD a4 = (DWORD)(wrd & 0x000F);
			DWORD a8 = (a4 | (a4 << 4));
			*pImPos = (*pImPos & 0x00FFFFFF) | (a8 << 24);
			wrd >>= 4;
			pImPos++;
		}
	}
}

inline void DecodeAlpha3BitLinear(DWORD * pImPos, DXTAlphaBlock3BitLinear * pAlphaBlock, int width)
{
	BYTE alphas[8];
	alphas[0] = pAlphaBlock->alpha0;
	alphas[1] = pAlphaBlock->alpha1;

	if (alphas[0] > alphas[1])
	{
		alphas[2] = (BYTE) ((6 * alphas[0] + 1 * alphas[1]) / 7);
		alphas[3] = (BYTE) ((5 * alphas[0] + 2 * alphas[1]) / 7);
		alphas[4] = (BYTE) ((4 * alphas[0] + 3 * alphas[1]) / 7);
		alphas[5] = (BYTE) ((3 * alphas[0] + 4 * alphas[1]) / 7);
		alphas[6] = (BYTE) ((2 * alphas[0] + 5 * alphas[1]) / 7);
		alphas[7] = (BYTE) ((1 * alphas[0] + 6 * alphas[1]) / 7);
	}    
	else
	{
		alphas[2] = (BYTE) ((4 * alphas[0] + 1 * alphas[1]) / 5);
		alphas[3] = (BYTE) ((3 * alphas[0] + 2 * alphas[1]) / 5);
		alphas[4] = (BYTE) ((2 * alphas[0] + 3 * alphas[1]) / 5);
		alphas[5] = (BYTE) ((1 * alphas[0] + 4 * alphas[1]) / 5);
		alphas[6] = 0;
		alphas[7] = 255;
	}
	
	uint64_t stuff64 = 0;
	memcpy(&stuff64, pAlphaBlock->stuff, 6);

	for (int row = 0; row < 4; ++row, pImPos += width - 4)
	{
		for (int pix = 0; pix < 4; ++pix)
		{
			uint32_t code = (uint32_t)(stuff64 & 0x07);
			stuff64 >>= 3;
			DWORD a8 = (DWORD)alphas[code];
			*pImPos = (*pImPos & 0x00FFFFFF) | (a8 << 24);
			pImPos++;
		}
	}
}

void CDXTCImage::DecompressDXT1(int miplevel, DWORD * pdwDest)
{
	UINT nWidth = m_nWidth >> miplevel;
	UINT nHeight = m_nHeight >> miplevel;
	
	int xblocks = nWidth / 4;
	int yblocks = nHeight / 4;

	DWORD * pBase = (DWORD *) pdwDest;
	const BYTE * pComp = m_pbCompBufferByLevels[miplevel] ? m_pbCompBufferByLevels[miplevel] : (m_bCompVector[miplevel].empty() ? nullptr : &m_bCompVector[miplevel][0]);
	if (!pComp || !pBase) return;

	Color8888 col_0, col_1, col_2, col_3;
	WORD wrd;

	for (int y = 0; y < yblocks; ++y)
	{
		for (int x = 0; x < xblocks; ++x)
		{
			DXTColBlock * pBlock = (DXTColBlock *) (pComp + (y * xblocks + x) * 8);
			GetColorBlockColors(pBlock, &col_0, &col_1, &col_2, &col_3, wrd);

			DWORD col[4];
			memcpy(&col[0], &col_0, 4);
			memcpy(&col[1], &col_1, 4);
			memcpy(&col[2], &col_2, 4);
			memcpy(&col[3], &col_3, 4);

			DWORD * pImPos = pBase + (y * 4) * nWidth + (x * 4);
			DecodeColorBlock(pImPos, pBlock, nWidth, col);
		}
	}
}

void CDXTCImage::DecompressDXT3(int miplevel, DWORD* pdwDest)
{
	UINT nWidth = m_nWidth >> miplevel;
	UINT nHeight = m_nHeight >> miplevel;

	int xblocks = nWidth / 4;
	int yblocks = nHeight / 4;

	DWORD * pBase = (DWORD *) pdwDest;
	const BYTE * pComp = m_pbCompBufferByLevels[miplevel] ? m_pbCompBufferByLevels[miplevel] : (m_bCompVector[miplevel].empty() ? nullptr : &m_bCompVector[miplevel][0]);
	if (!pComp || !pBase) return;

	Color8888 col_0, col_1, col_2, col_3;
	WORD wrd;

	for (int y = 0; y < yblocks; ++y)
	{
		for (int x = 0; x < xblocks; ++x)
		{
			const BYTE * pBlockBytes = pComp + (y * xblocks + x) * 16;
			DXTAlphaBlockExplicit * pAlphaBlock = (DXTAlphaBlockExplicit *) pBlockBytes;
			DXTColBlock * pBlock = (DXTColBlock *) (pBlockBytes + 8);

			GetColorBlockColors(pBlock, &col_0, &col_1, &col_2, &col_3, wrd);

			DWORD col[4];
			memcpy(&col[0], &col_0, 4);
			memcpy(&col[1], &col_1, 4);
			memcpy(&col[2], &col_2, 4);
			memcpy(&col[3], &col_3, 4);

			DWORD * pImPos = pBase + (y * 4) * nWidth + (x * 4);
			DecodeColorBlock(pImPos, pBlock, nWidth, col);
			DecodeAlphaExplicit(pImPos, pAlphaBlock, nWidth);
		}
	}
}

void CDXTCImage::DecompressDXT5(int level, DWORD * pdwDest)
{
	UINT nWidth = m_nWidth >> level;
	UINT nHeight = m_nHeight >> level;

	int xblocks = nWidth / 4;
	int yblocks = nHeight / 4;

	DWORD * pBase = (DWORD *) pdwDest;
	const BYTE * pComp = m_pbCompBufferByLevels[level] ? m_pbCompBufferByLevels[level] : (m_bCompVector[level].empty() ? nullptr : &m_bCompVector[level][0]);
	if (!pComp || !pBase) return;

	Color8888 col_0, col_1, col_2, col_3;
	WORD wrd;

	for (int y = 0; y < yblocks; ++y)
	{
		for (int x = 0; x < xblocks; ++x)
		{
			const BYTE * pBlockBytes = pComp + (y * xblocks + x) * 16;
			DXTAlphaBlock3BitLinear * pAlphaBlock = (DXTAlphaBlock3BitLinear *) pBlockBytes;
			DXTColBlock * pBlock = (DXTColBlock *) (pBlockBytes + 8);

			GetColorBlockColors(pBlock, &col_0, &col_1, &col_2, &col_3, wrd);

			DWORD col[4];
			memcpy(&col[0], &col_0, 4);
			memcpy(&col[1], &col_1, 4);
			memcpy(&col[2], &col_2, 4);
			memcpy(&col[3], &col_3, 4);

			DWORD * pImPos = pBase + (y * 4) * nWidth + (x * 4);
			DecodeColorBlock(pImPos, pBlock, nWidth, col);
			DecodeAlpha3BitLinear(pImPos, pAlphaBlock, nWidth);
		}
	}
}

void CDXTCImage::DecompressARGB(int level, DWORD * pdwDest)
{
	UINT lPitch = m_lPitch >> (level * 2);
	const BYTE * pComp = m_pbCompBufferByLevels[level] ? m_pbCompBufferByLevels[level] : (m_bCompVector[level].empty() ? nullptr : &m_bCompVector[level][0]);
	if (pComp && pdwDest)
		memcpy(pdwDest, pComp, lPitch);
}

/*
typedef struct _DDSURFACEDESC2 {
DWORD         dwSize;
DWORD         dwFlags;
DWORD         dwHeight;
DWORD         dwWidth;
union
{
LONG      lPitch;
DWORD     dwLinearSize;
} DUMMYUNIONNAMEN(1);
DWORD         dwBackBufferCount;
union
{
DWORD     dwMipMapCount;
DWORD     dwRefreshRate;
} DUMMYUNIONNAMEN(2);
DWORD         dwAlphaBitDepth;
DWORD         dwReserved;
LPVOID        lpSurface;
union
{
DDCOLORKEY    ddckCKDestOverlay;
DWORD         dwEmptyFaceColor;
} DUMMYUNIONNAMEN(3);
DDCOLORKEY    ddckCKDestBlt;
DDCOLORKEY    ddckCKSrcOverlay;
DDCOLORKEY    ddckCKSrcBlt;
DDPIXELFORMAT ddpfPixelFormat;
DDSCAPS2      ddsCaps;
DWORD         dwTextureStage;
} DDSURFACEDESC2, FAR* LPDDSURFACEDESC2; 
*/

//-----------------------------------------------------------------------------
// Name: GetNumberOfBits()
// Desc: Returns the number of bits set in a DWORD mask
//	from microsoft mssdk d3dim sample "Compress"
//-----------------------------------------------------------------------------
static WORD GetNumberOfBits(DWORD dwMask)
{
	WORD wBits;
    for (wBits = 0; dwMask; wBits++)
        dwMask = (dwMask & (dwMask - 1)); 
	
    return wBits;
}

//-----------------------------------------------------------------------------
// Name: PixelFormatToString()
// Desc: Creates a string describing a pixel format.
//	adapted from microsoft mssdk D3DIM Compress example
//  PixelFormatToString()
//-----------------------------------------------------------------------------
VOID CDXTCImage::DecodePixelFormat(CHAR* strPixelFormat, XDDPIXELFORMAT* pxddpf)
{
	switch (pxddpf->dwFourCC)
	{
		case 0:
			{
				// This dds texture isn't compressed so write out ARGB format
				WORD a = GetNumberOfBits(pxddpf->dwRGBAlphaBitMask);
				WORD r = GetNumberOfBits(pxddpf->dwRBitMask);
				WORD g = GetNumberOfBits(pxddpf->dwGBitMask);
				WORD b = GetNumberOfBits(pxddpf->dwBBitMask);

				_snprintf(strPixelFormat, 31, "ARGB-%d%d%d%d%s", a, r, g, b,
					pxddpf->dwBBitMask & DDPF_ALPHAPREMULT ? "-premul" : "");
				m_CompFormat = PF_ARGB;
			}
			break;
			
		case MAKEFOURCC('D','X','T','1'):
			strncpy(strPixelFormat, "DXT1", 31);
			m_CompFormat = PF_DXT1;
			break;
			
		case MAKEFOURCC('D','X','T','2'):
			strncpy(strPixelFormat, "DXT2", 31);
			m_CompFormat = PF_DXT2;
			break;
			
		case MAKEFOURCC('D','X','T','3'):
			strncpy(strPixelFormat, "DXT3", 31);
			m_CompFormat = PF_DXT3;
			break;
			
		case MAKEFOURCC('D','X','T','4'):
			strncpy(strPixelFormat, "DXT4", 31);
			m_CompFormat = PF_DXT4;
			break;
			
		case MAKEFOURCC('D','X','T','5'):
			strncpy(strPixelFormat, "DXT5", 31);
			m_CompFormat = PF_DXT5;
			break;

		default:
			strcpy(strPixelFormat, "Format Unknown");
			m_CompFormat = PF_UNKNOWN;
			break;
	}
}

/*					  
// Struct to hold various timing values
struct TimingInfo
{	
	LARGE_INTEGER	m_start_clk;
	LARGE_INTEGER	m_end_clk;
	
	int			m_nSamples;
	LARGE_INTEGER	m_interval_sum;		// sum of all end-start, nSamples number added in
	
	CString		m_csName;		// text desc of what timed
};
  
void CDXTCImage::RunTimingSession()
{
	// Must have a dxt5 texture loaded
	// No special reason - just lazy coding
	// Functions called to time code are separate from non-timed
	//  code.  It's alogorithm that counts.
	ASSERT(m_pCompBytes != NULL);
	ASSERT(m_pDecompBytes != NULL);		// must already have allocated memory
	
	switch (m_CompFormat)
	{
		case PF_DXT1:
		case PF_DXT2:
		case PF_DXT3:
		case PF_DXT4:
		case PF_UNKNOWN:
			Tracef("You must have a DXT5 texture loaded to RunTimingSession()!!\n");
			Tracef("Now I will be nasty and ASSERT(false)!\n");
			ASSERT(false);
			break;
			
		case PF_DXT5:
			Tracef("Running code timing session on DXT5 color decompress\n");
			break;
	}
	
	LARGE_INTEGER	start_clk, end_clk;
	QueryPerformanceCounter(&start_clk);
#define NMETHOD   4
#define NBATCHES  4
	int passes[NBATCHES];
	passes[0] = 1;
	passes[1] = 10;
	passes[2] = 30; 
	passes[3] = 50;
	
	TimingInfo method[NMETHOD][NBATCHES];
	
	int i, n;
	
	FILE * pf = fopen("timing.txt", "wt");
	
	if (pf == NULL)
	{
		return;
	}
	
	fprintf(pf, "\n\n");
	
	for (i = 0; i < NBATCHES; ++i)
	{
		Sleep(50);
		fprintf(pf,"i: %d passes[i]: %d\n", i, passes[i]);
		Time_Decomp5_01(passes[i], &(method[0][i]));
		Time_Decomp5_02(passes[i], &(method[1][i]));
		Time_Decomp5_03(passes[i], &(method[2][i]));
		Time_Decomp5_04(passes[i], &(method[3][i]));
	}
	
	QueryPerformanceCounter(&end_clk);
	
	//	unsigned long total;
	//	total = (unsigned long) (end_clk - start_clk);
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(& freq);
	
	fprintf(pf, "\nCounter freq = %u  %d \n", freq.LowPart, freq.HighPart);
	fprintf(pf, "start:  %u  %u       end:  %u  %u\n", start_clk.LowPart, start_clk.HighPart, end_clk.LowPart, end_clk.HighPart);
	
	Tracef("\nCounter freq = %u  %d \n", freq.LowPart, freq.HighPart);
	Tracef("start:  %u  %u       end:  %u  %u\n", start_clk.LowPart, start_clk.HighPart, end_clk.LowPart, end_clk.HighPart);
	
	double dur = ((double)end_clk.LowPart - (double)start_clk.LowPart) / (double)freq.LowPart;
	
	fprintf(pf, "Total timing session took:  %u cycles = %f seconds\n", (end_clk.LowPart - start_clk.LowPart), dur);
	fprintf(pf, "\n\n");
	
	Tracef("Total timing session took:  %u cycles = %f seconds\n", (end_clk.LowPart - start_clk.LowPart), dur);
	Tracef("\n\n");
	
	for (n = 0; n < NMETHOD; ++n)
	{
		for (i = 0; i < NBATCHES; ++i)
		{
			fprintf(pf, "method %d:\n", n);
			fprintf(pf, "  %s", method[n][i].m_csName);
			fprintf(pf, "  tot:   %u %u\n", method[n][i].m_interval_sum.HighPart, method[n][i].m_interval_sum.LowPart); 
			
			Tracef("method %d:\n", n);
			Tracef("  %s", method[n][i].m_csName);
			Tracef("  tot:   %u %u\n", method[n][i].m_interval_sum.HighPart, method[n][i].m_interval_sum.LowPart); 
			
			dur = ((double)method[n][i].m_interval_sum.LowPart) / ((double)method[n][i].m_nSamples * (double)freq.LowPart);
			
			fprintf(pf, "  avg:   %u\n", method[n][i].m_interval_sum.LowPart / method[n][i].m_nSamples);
			fprintf(pf, "  avg time:  %f sec\n", dur);
			
			Tracef("  avg:   %u\n", method[n][i].m_interval_sum.LowPart / method[n][i].m_nSamples);
			Tracef("  avg time:  %f sec\n", dur);
		}
		
		fprintf(pf, "\n\n");
		Tracef("\n\n");
	}
	
	fclose(pf);
	
	MessageBeep(MB_OK);
	//BOOL QueryPerformanceFrequency(
	//  LARGE_INTEGER *lpFrequency   // address of current frequency
	//);
}

inline void GetColorBlockColors_m2(DXTColBlock * pBlock, Color8888 * col_0, Color8888 * col_1, 
								   Color8888 * col_2, Color8888 * col_3,
								   WORD & wrd )
{
	// method 2
	// freak variable bit structure method
	// normal math
	Color565 * pCol;
	
	pCol = (Color565*) & (pBlock->col0);
	
	col_0->a = 0xff;
	col_0->r = pCol->nRed;
	col_0->r <<= 3;				// shift to full precision
	col_0->g = pCol->nGreen;
	col_0->g <<= 2;
	col_0->b = pCol->nBlue;
	col_0->b <<= 3;
	
	pCol = (Color565*) & (pBlock->col1);
	col_1->a = 0xff;
	col_1->r = pCol->nRed;
	col_1->r <<= 3;				// shift to full precision
	col_1->g = pCol->nGreen;
	col_1->g <<= 2;
	col_1->b = pCol->nBlue;
	col_1->b <<= 3;
	
	if (pBlock->col0 > pBlock->col1)
	{
		// Four-color block: derive the other two colors.    
		// 00 = color_0, 01 = color_1, 10 = color_2, 11 = color_3
		// These two bit codes correspond to the 2-bit fields 
		// stored in the 64-bit block.
		wrd = ((WORD) col_0->r * 2 + (WORD) col_1->r) / 3;
		// no +1 for rounding
		// as bits have been shifted to 888
		col_2->r = (BYTE)wrd;
		
		wrd = ((WORD) col_0->g * 2 + (WORD) col_1->g) / 3;
		col_2->g = (BYTE)wrd;
		
		wrd = ((WORD) col_0->b * 2 + (WORD) col_1->b) / 3;
		col_2->b = (BYTE)wrd;
		col_2->a = 0xff;
		
		wrd = ((WORD) col_0->r + (WORD) col_1->r * 2) / 3;
		col_3->r = (BYTE)wrd;
		
		wrd = ((WORD) col_0->g + (WORD) col_1->g * 2) / 3;
		col_3->g = (BYTE)wrd;
		
		wrd = ((WORD) col_0->b + (WORD) col_1->b * 2) / 3;
		col_3->b = (BYTE)wrd;
		col_3->a = 0xff;
		
	}
	else
	{
		// Three-color block: derive the other color.
		// 00 = color_0,  01 = color_1,  10 = color_2,  
		// 11 = transparent.
		// These two bit codes correspond to the 2-bit fields 
		// stored in the 64-bit block. 
		
		// explicit for each component, unlike some refrasts...
		
		// Tracef("block has alpha\n");
		wrd = ((WORD) col_0->r + (WORD) col_1->r) / 2;
		col_2->r = (BYTE)wrd;
		wrd = ((WORD) col_0->g + (WORD) col_1->g) / 2;
		col_2->g = (BYTE)wrd;
		wrd = ((WORD) col_0->b + (WORD) col_1->b) / 2;
		col_2->b = (BYTE)wrd;
		col_2->a = 0xff;
		
		col_3->r = 0x00;		// random color to indicate alpha
		col_3->g = 0xff;
		col_3->b = 0xff;
		col_3->a = 0x00;
	}
}



inline void GetColorBlockColors_m3(DXTColBlock * pBlock, Color8888 * col_0, Color8888 * col_1, 
								   Color8888 * col_2, Color8888 * col_3,
								   WORD & wrd )
{
	// method 3
	//////////////////////////////////////////////////////
	// super-freak variable bit structure with
	//  Cool Math Trick (tm)

	// Do 2/3 1/3 math BEFORE bit shift on the whole DWORD
	// as the fields will NEVER carry into the next
	//  or overflow!! =) 
	
	Color565 * pCol;
	
	pCol = (Color565*) & (pBlock->col0);
	
	col_0->a = 0x00;			// must set to 0 to avoid overflow in DWORD add
	col_0->r = pCol->nRed;
	col_0->g = pCol->nGreen;
	col_0->b = pCol->nBlue;
	
	pCol = (Color565*) & (pBlock->col1);
	col_1->a = 0x00;
	col_1->r = pCol->nRed;
	col_1->g = pCol->nGreen;
	col_1->b = pCol->nBlue;
	
	if (pBlock->col0 > pBlock->col1)
	{
		*((DWORD*)col_2) = ((*((DWORD*)col_0)) * 2 + (*((DWORD*)col_1)));
		
		*((DWORD*)col_3) = ((*((DWORD*)col_0)) + (*((DWORD*)col_1)) * 2);
		
		// now shift to appropriate precision & divide by 3.
		col_2->r = ((WORD) col_2->r << 3) / (WORD)3;
		col_2->g = ((WORD) col_2->g << 2) / (WORD)3;
		col_2->b = ((WORD) col_2->b << 3) / (WORD)3;
		
		col_3->r = ((WORD) col_3->r << 3) / (WORD)3;
		col_3->g = ((WORD) col_3->g << 2) / (WORD)3;
		col_3->b = ((WORD) col_3->b << 3) / (WORD)3;

		col_0->a = 0xff;		// now set appropriate alpha
		col_1->a = 0xff;
		col_2->a = 0xff;
		col_3->a = 0xff;
	}
	else
	{
		*((DWORD*)col_2) = ((*((DWORD*)col_0)) + (*((DWORD*)col_1)));
		
		// now shift to appropriate precision & divide by 2.
		// << 3) / 2 == << 2
		// << 2) / 2 == << 1
		col_2->r = ((WORD) col_2->r << 2);
		col_2->g = ((WORD) col_2->g << 1);
		col_2->b = ((WORD) col_2->b << 2);
		
		col_2->a = 0xff;
		
		col_3->a = 0x00;	// 
		col_3->r = 0x00;	// random color to indicate alpha
		col_3->g = 0xff;
		col_3->b = 0xff;
	}
	
	// now shift orig color components
	col_0->r <<= 3;
	col_0->g <<= 2;
	col_0->b <<= 3;
	
	col_1->r <<= 3;
	col_1->g <<= 2;
	col_1->b <<= 3;
}


inline void GetColorBlockColors_m4(DXTColBlock * pBlock, Color8888 * col_0, Color8888 * col_1, 
								   Color8888 * col_2, Color8888 * col_3,
								   WORD & wrd )
{
	
	// m1 color extraction from 5-6-5
	// m3 color math on DWORD before bit shift to full precision
	wrd = pBlock->col0;
	col_0->a = 0x00;			// must set to 0 to avoid possible overflow & carry to next field in DWORD add
	
	// extract r,g,b bits
	col_0->b = (unsigned char) wrd & 0x1f;			// 0x1f = 0001 1111  to mask out upper 3 bits
	wrd >>= 5;
	col_0->g = (unsigned char) wrd & 0x3f;			// 0x3f = 0011 1111  to mask out upper 2 bits
	wrd >>= 6;
	col_0->r = (unsigned char) wrd & 0x1f;
	
	
	// same for col # 2:
	wrd = pBlock->col1;
	col_1->a = 0x00;			// must set to 0 to avoid possible overflow in DWORD add
	
	// extract r,g,b bits
	col_1->b = (unsigned char) wrd & 0x1f;
	wrd >>= 5;
	col_1->g = (unsigned char) wrd & 0x3f;
	wrd >>= 6;
	col_1->r = (unsigned char) wrd & 0x1f;

	if (pBlock->col0 > pBlock->col1)
	{
		*((DWORD*)col_2) = ((*((DWORD*)col_0)) * 2 + (*((DWORD*)col_1)));
		*((DWORD*)col_3) = ((*((DWORD*)col_0)) + (*((DWORD*)col_1)) * 2);
		
		// shift to appropriate precision & divide by 3.
		col_2->r = ((WORD) col_2->r << 3) / (WORD)3;
		col_2->g = ((WORD) col_2->g << 2) / (WORD)3;
		col_2->b = ((WORD) col_2->b << 3) / (WORD)3;
		
		col_3->r = ((WORD) col_3->r << 3) / (WORD)3;
		col_3->g = ((WORD) col_3->g << 2) / (WORD)3;
		col_3->b = ((WORD) col_3->b << 3) / (WORD)3;
		
		col_0->a = 0xff;		// set appropriate alpha
		col_1->a = 0xff;
		col_2->a = 0xff;
		col_3->a = 0xff;
	}
	else
	{
		*((DWORD*)col_2) = ((*((DWORD*)col_0)) + (*((DWORD*)col_1)));
		
		// shift to appropriate precision & divide by 2.
		// << 3) / 2 == << 2
		// << 2) / 2 == << 1
		col_2->r = ((WORD) col_2->r << 2);
		col_2->g = ((WORD) col_2->g << 1);
		col_2->b = ((WORD) col_2->b << 2);
		
		col_2->a = 0xff;
		
		col_3->a = 0x00;	// 
		col_3->r = 0x00;	// random color to indicate alpha
		col_3->g = 0xff;
		col_3->b = 0xff;
	}
	
	// shift orig color components to full precision
	col_0->r <<= 3;
	col_0->g <<= 2;
	col_0->b <<= 3;
	
	col_1->r <<= 3;
	col_1->g <<= 2;
	col_1->b <<= 3;
}


inline void GetColorBlockColors_m1(DXTColBlock * pBlock, Color8888 * col_0, Color8888 * col_1, 
								   Color8888 * col_2, Color8888 * col_3,
								   WORD & wrd )
{
	
	// Method 1:
	// Shifty method
	wrd = pBlock->col0;
	col_0->a = 0xff;
	
	// extract r,g,b bits
	col_0->b = (unsigned char) wrd;
	col_0->b <<= 3;		// shift to full precision
	wrd >>= 5;
	col_0->g = (unsigned char) wrd;
	col_0->g <<= 2;		// shift to full precision
	wrd >>= 6;
	col_0->r = (unsigned char) wrd;
	col_0->r <<= 3;		// shift to full precision

	// same for col # 2:
	wrd = pBlock->col1;
	col_1->a = 0xff;
	
	// extract r,g,b bits
	col_1->b = (unsigned char) wrd;
	col_1->b <<= 3;		// shift to full precision
	wrd >>= 5;
	col_1->g = (unsigned char) wrd;
	col_1->g <<= 2;		// shift to full precision
	wrd >>= 6;
	col_1->r = (unsigned char) wrd;
	col_1->r <<= 3;		// shift to full precision

	// use this for all but the super-freak math method
	if (pBlock->col0 > pBlock->col1)
	{
		// Four-color block: derive the other two colors.    
		// 00 = color_0, 01 = color_1, 10 = color_2, 11 = color_3
		// These two bit codes correspond to the 2-bit fields 
		// stored in the 64-bit block.
		
		wrd = ((WORD) col_0->r * 2 + (WORD) col_1->r) / 3;
		// no +1 for rounding
		// as bits have been shifted to 888
		col_2->r = (BYTE)wrd;
		
		wrd = ((WORD) col_0->g * 2 + (WORD) col_1->g) / 3;
		col_2->g = (BYTE)wrd;
		
		wrd = ((WORD) col_0->b * 2 + (WORD) col_1->b) / 3;
		col_2->b = (BYTE)wrd;
		col_2->a = 0xff;
		
		wrd = ((WORD) col_0->r + (WORD) col_1->r * 2) / 3;
		col_3->r = (BYTE)wrd;
		
		wrd = ((WORD) col_0->g + (WORD) col_1->g * 2) / 3;
		col_3->g = (BYTE)wrd;
		
		wrd = ((WORD) col_0->b + (WORD) col_1->b * 2) / 3;
		col_3->b = (BYTE)wrd;
		col_3->a = 0xff;
	}
	else
	{
		// Three-color block: derive the other color.
		// 00 = color_0,  01 = color_1,  10 = color_2,  
		// 11 = transparent.
		// These two bit codes correspond to the 2-bit fields 
		// stored in the 64-bit block. 
		
		// explicit for each component, unlike some refrasts...
		
		// Tracef("block has alpha\n");
		
		wrd = ((WORD) col_0->r + (WORD) col_1->r) / 2;
		col_2->r = (BYTE)wrd;
		wrd = ((WORD) col_0->g + (WORD) col_1->g) / 2;
		col_2->g = (BYTE)wrd;
		wrd = ((WORD) col_0->b + (WORD) col_1->b) / 2;
		col_2->b = (BYTE)wrd;
		col_2->a = 0xff;
		
		col_3->r = 0x00;		// random color to indicate alpha
		col_3->g = 0xff;
		col_3->b = 0xff;
		col_3->a = 0x00;
	}
}	//  Get color block colors (...)

void CDXTCImage::Time_Decomp5_01(int ntimes, TimingInfo * info)
{
	int n;
	
	info->m_nSamples = 0;
	info->m_interval_sum.QuadPart = 0;
	info->m_csName.Format("Timing decomp method 1:  bit shift, for %d times\n", ntimes);

	for (n = 0; n < ntimes; n++)
	{
		QueryPerformanceCounter(& info->m_start_clk);

		int xblocks, yblocks;
		
		xblocks = m_DDSD.dwWidth / 4;
		yblocks = m_DDSD.dwHeight / 4;

		int i,j;
		
		DWORD * pBase  = (DWORD*) m_pDecompBytes;
		DWORD * pImPos = (DWORD*) pBase;			// pos in decompressed data
		WORD  * pPos   = (WORD*) m_pCompBytes;	// pos in compressed data
		
		DXTColBlock				* pBlock;
		DXTAlphaBlock3BitLinear * pAlphaBlock;
		
		Color8888 col_0, col_1, col_2, col_3;
		WORD wrd;
		
		// fill alphazero with appropriate value to zero out alpha when
		//  alphazero is ANDed with the image color 32 bit DWORD:
		col_0.a = 0;
		col_0.r = col_0.g = col_0.b = 0xff;
		DWORD alphazero = *((DWORD*) &col_0);

		// ** See DecompressDXT5 code for comments!!
		for (j = 0; j < yblocks; ++j)
		{
			pBlock = (DXTColBlock*) ((DWORD)m_pCompBytes + j * xblocks * 16);
			for (i = 0; i < xblocks; ++i, ++pBlock)
			{
				pAlphaBlock = (DXTAlphaBlock3BitLinear*) pBlock;
				pBlock++;
				
				GetColorBlockColors_m1(pBlock, &col_0, &col_1, &col_2, &col_3, wrd);
				
				pImPos = (DWORD*)((DWORD)pBase + i*16 + (j*4) * m_nWidth * 4);
				DecodeColorBlock(pImPos, pBlock, m_nWidth, (DWORD*)&col_0, (DWORD*)&col_1,
					(DWORD*)&col_2, (DWORD*)&col_3);
				DecodeAlpha3BitLinear(pImPos, pAlphaBlock, m_nWidth, alphazero);
			}
		}

		QueryPerformanceCounter(& info->m_end_clk);
		
		info->m_nSamples ++;
		info->m_interval_sum.QuadPart += info->m_end_clk.QuadPart - info->m_start_clk.QuadPart;
	}
}


void CDXTCImage::Time_Decomp5_02(int ntimes, TimingInfo * info)
{
	int n;

	info->m_nSamples = 0;
	info->m_interval_sum.QuadPart = 0;
	info->m_csName.Format("Timing decomp method 2:  bit field struct, for %d times\n", ntimes);

	for (n = 0; n < ntimes; n++)
	{
		QueryPerformanceCounter(& info->m_start_clk);
		
		int xblocks, yblocks;
		xblocks = m_DDSD.dwWidth / 4;
		yblocks = m_DDSD.dwHeight / 4;		
		int i,j;
		DWORD * pBase  = (DWORD*) m_pDecompBytes;
		DWORD * pImPos = (DWORD*) pBase;			// pos in decompressed data
		WORD  * pPos   = (WORD*)  m_pCompBytes;	// pos in compressed data
		DXTColBlock				* pBlock;
		DXTAlphaBlock3BitLinear * pAlphaBlock;
		Color8888 col_0, col_1, col_2, col_3;
		WORD wrd;
		// fill alphazero with appropriate value to zero out alpha when
		//  alphazero is ANDed with the image color 32 bit DWORD:
		col_0.a = 0;
		col_0.r = col_0.g = col_0.b = 0xff;
		DWORD alphazero = *((DWORD*) &col_0);
		
		// ** See DecompressDXT5 code for comments!!
		for (j = 0; j < yblocks; ++j)
		{
			pBlock = (DXTColBlock*) ((DWORD)m_pCompBytes + j * xblocks * 16);
			for (i = 0; i < xblocks; ++i, ++pBlock)
			{
				pAlphaBlock = (DXTAlphaBlock3BitLinear*) pBlock;
				pBlock++;
				
				GetColorBlockColors_m2(pBlock, &col_0, &col_1, &col_2, &col_3, wrd);
				
				pImPos = (DWORD*)((DWORD)pBase + i*16 + (j*4) * m_nWidth * 4);
				DecodeColorBlock(pImPos, pBlock, m_nWidth, (DWORD*)&col_0, (DWORD*)&col_1,
					(DWORD*)&col_2, (DWORD*)&col_3);
				DecodeAlpha3BitLinear(pImPos, pAlphaBlock, m_nWidth, alphazero);
			}
		}
		
		QueryPerformanceCounter(& info->m_end_clk);
		
		info->m_nSamples ++;
		info->m_interval_sum.QuadPart += info->m_end_clk.QuadPart - info->m_start_clk.QuadPart;
	}
}

void CDXTCImage::Time_Decomp5_03(int ntimes, TimingInfo * info)
{
	int n;
	
	info->m_nSamples = 0;
	info->m_interval_sum.QuadPart = 0;
	info->m_csName.Format("Timing decomp method 3:  bit field struct w/ pre-shift math, for %d times\n", ntimes);

	for (n = 0; n < ntimes; n++)
	{
		QueryPerformanceCounter(& info->m_start_clk);
		
		int xblocks, yblocks;
		xblocks = m_DDSD.dwWidth / 4;
		yblocks = m_DDSD.dwHeight / 4;		
		int i,j;
		DWORD * pBase  = (DWORD*)  m_pDecompBytes;
		DWORD * pImPos = (DWORD*)  pBase;			// pos in decompressed data
		WORD  * pPos   = (WORD*)   m_pCompBytes;	// pos in compressed data
		DXTColBlock				* pBlock;
		DXTAlphaBlock3BitLinear * pAlphaBlock;
		Color8888 col_0, col_1, col_2, col_3;
		WORD wrd;
		
		// fill alphazero with appropriate value to zero out alpha when
		//  alphazero is ANDed with the image color 32 bit DWORD:
		col_0.a = 0;
		col_0.r = col_0.g = col_0.b = 0xff;
		DWORD alphazero = *((DWORD*) &col_0);
		
		// ** See DecompressDXT5 code for comments!!
		for (j = 0; j < yblocks; ++j)
		{
			pBlock = (DXTColBlock*) ((DWORD)m_pCompBytes + j * xblocks * 16);
			for (i = 0; i < xblocks; ++i, ++pBlock)
			{
				pAlphaBlock = (DXTAlphaBlock3BitLinear*) pBlock;
				pBlock++;
				
				GetColorBlockColors_m3(pBlock, &col_0, &col_1, &col_2, &col_3, wrd);
				
				pImPos = (DWORD*)((DWORD)pBase + i*16 + (j*4) * m_nWidth * 4);
				DecodeColorBlock(pImPos, pBlock, m_nWidth, (DWORD*)&col_0, (DWORD*)&col_1,
					(DWORD*)&col_2, (DWORD*)&col_3);
				DecodeAlpha3BitLinear(pImPos, pAlphaBlock, m_nWidth, alphazero);
			}
		}
		
		QueryPerformanceCounter(& info->m_end_clk);
		
		info->m_nSamples ++;
		info->m_interval_sum.QuadPart += info->m_end_clk.QuadPart - info->m_start_clk.QuadPart;
	}
}


void CDXTCImage::Time_Decomp5_04(int ntimes, TimingInfo * info)
{
	int n;
	
	info->m_nSamples = 0;
	info->m_interval_sum.QuadPart = 0;
	info->m_csName.Format("Timing decomp method 4:  shift extract w/ pre-shift math, for %d times\n", ntimes);
	
	QueryPerformanceCounter(& info->m_start_clk);
	
	for (n = 0; n < ntimes; n++)
	{
		int xblocks, yblocks;
		xblocks = m_DDSD.dwWidth / 4;
		yblocks = m_DDSD.dwHeight / 4;		
		int i,j;
		DWORD * pBase  = (DWORD*)  m_pDecompBytes;
		DWORD * pImPos = (DWORD*)  pBase;			// pos in decompressed data
		WORD  * pPos   = (WORD*)   m_pCompBytes;	// pos in compressed data
		DXTColBlock				* pBlock;
		DXTAlphaBlock3BitLinear * pAlphaBlock;
		Color8888 col_0, col_1, col_2, col_3;
		WORD wrd;
		// fill alphazero with appropriate value to zero out alpha when
		//  alphazero is ANDed with the image color 32 bit DWORD:
		col_0.a = 0;
		col_0.r = col_0.g = col_0.b = 0xff;
		DWORD alphazero = *((DWORD*) &col_0);
		
		// ** See DecompressDXT5 code for comments!!
		for (j = 0; j < yblocks; ++j)
		{
			pBlock = (DXTColBlock*) ((DWORD)m_pCompBytes + j * xblocks * 16);
			for (i = 0; i < xblocks; ++i, ++pBlock)
			{
				pAlphaBlock = (DXTAlphaBlock3BitLinear*) pBlock;
				pBlock++;
				
				GetColorBlockColors_m4(pBlock, &col_0, &col_1, &col_2, &col_3, wrd);
				
				pImPos = (DWORD*)((DWORD)pBase + i*16 + (j*4) * m_nWidth * 4);
				DecodeColorBlock(pImPos, pBlock, m_nWidth, (DWORD*)&col_0, (DWORD*)&col_1,
					(DWORD*)&col_2, (DWORD*)&col_3);
				DecodeAlpha3BitLinear(pImPos, pAlphaBlock, m_nWidth, alphazero);
			}
		}
	}
	
	QueryPerformanceCounter(& info->m_end_clk);
	
	info->m_nSamples = ntimes;
	info->m_interval_sum.QuadPart += info->m_end_clk.QuadPart - info->m_start_clk.QuadPart;
	
}
*/
