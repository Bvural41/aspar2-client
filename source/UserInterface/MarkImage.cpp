#include "StdAfx.h"
#include "MarkImage.h"
#include <map>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "../EterBase/CRC32.h"
#include "../EterBase/lzo.h"
#include <lzo-2.03/lzo1x.h>

#define sys_err TraceError
#define sys_log(...)
#define thecore_memcpy memcpy

CGuildMarkImage * NewMarkImage()
{
	return new CGuildMarkImage;
}

void DeleteMarkImage(CGuildMarkImage * pkImage)
{
	delete pkImage;
}

CGuildMarkImage::CGuildMarkImage()
{
	memset(m_apxImage, 0, sizeof(m_apxImage));
}

CGuildMarkImage::~CGuildMarkImage()
{
	Destroy();
}

void CGuildMarkImage::Destroy()
{
	memset(m_apxImage, 0, sizeof(m_apxImage));
}

void CGuildMarkImage::Create()
{
	memset(m_apxImage, 0, sizeof(m_apxImage));
}

bool CGuildMarkImage::Save(const char* c_szFileName) 
{
	return stbi_write_tga(c_szFileName, WIDTH, HEIGHT, 4, m_apxImage) != 0;
}

bool CGuildMarkImage::Build(const char * c_szFileName)
{
	Destroy();
	Create();

	if (!Save(c_szFileName))
	{
		sys_err("CGuildMarkImage: cannot initialize image %s", c_szFileName);
		return false;
	}

	return true;
}

bool CGuildMarkImage::Load(const char * c_szFileName) 
{
	Destroy();
	Create();	

	int width = 0;
	int height = 0;
	int channels = 0;
	unsigned char* pData = stbi_load(c_szFileName, &width, &height, &channels, 4);

	if (!pData)
	{
		Build(c_szFileName);
		pData = stbi_load(c_szFileName, &width, &height, &channels, 4);

		if (!pData)
		{
			sys_err("CGuildMarkImage: cannot open file %s", c_szFileName);
			return false;
		}
	}

	if (width != WIDTH)	
	{
		sys_err("CGuildMarkImage: %s width must be %u", c_szFileName, WIDTH);
		stbi_image_free(pData);
		return false;
	}

	if (height != HEIGHT)
	{
		sys_err("CGuildMarkImage: %s height must be %u", c_szFileName, HEIGHT);
		stbi_image_free(pData);
		return false;
	}

	memcpy(m_apxImage, pData, sizeof(m_apxImage));
	stbi_image_free(pData);

	BuildAllBlocks();
	return true;
}

void CGuildMarkImage::PutData(UINT x, UINT y, UINT width, UINT height, void * data)
{
	Pixel* pSrc = (Pixel*)data;
	for (UINT row = 0; row < height; ++row)
	{
		if (y + row < HEIGHT)
		{
			UINT copyWidth = (x + width <= WIDTH) ? width : (WIDTH - x);
			memcpy(&m_apxImage[(y + row) * WIDTH + x], &pSrc[row * width], copyWidth * sizeof(Pixel));
		}
	}
}

void CGuildMarkImage::GetData(UINT x, UINT y, UINT width, UINT height, void * data)
{
	Pixel* pDst = (Pixel*)data;
	for (UINT row = 0; row < height; ++row)
	{
		if (y + row < HEIGHT)
		{
			UINT copyWidth = (x + width <= WIDTH) ? width : (WIDTH - x);
			memcpy(&pDst[row * width], &m_apxImage[(y + row) * WIDTH + x], copyWidth * sizeof(Pixel));
		}
	}
}

// SERVER
bool CGuildMarkImage::SaveMark(DWORD posMark, BYTE * pbImage)
{
	if (posMark >= MARK_TOTAL_COUNT)
	{
		sys_err("CGuildMarkImage::CopyMarkFromData: Invalid mark position %u", posMark);
		return false;
	}

	DWORD colMark = posMark % MARK_COL_COUNT;
	DWORD rowMark = posMark / MARK_COL_COUNT;

	PutData(colMark * SGuildMark::WIDTH, rowMark * SGuildMark::HEIGHT, SGuildMark::WIDTH, SGuildMark::HEIGHT, pbImage);

	DWORD rowBlock = rowMark / SGuildMarkBlock::MARK_PER_BLOCK_HEIGHT;
	DWORD colBlock = colMark / SGuildMarkBlock::MARK_PER_BLOCK_WIDTH;

	Pixel apxBuf[SGuildMarkBlock::SIZE];
	GetData(colBlock * SGuildMarkBlock::WIDTH, rowBlock * SGuildMarkBlock::HEIGHT, SGuildMarkBlock::WIDTH, SGuildMarkBlock::HEIGHT, apxBuf);
	m_aakBlock[rowBlock][colBlock].Compress(apxBuf);
	return true;
}

bool CGuildMarkImage::DeleteMark(DWORD posMark)
{
	Pixel image[SGuildMark::SIZE];
	memset(&image, 0, sizeof(image));
	return SaveMark(posMark, (BYTE *) &image);
}

// CLIENT
bool CGuildMarkImage::SaveBlockFromCompressedData(DWORD posBlock, const BYTE * pbComp, DWORD dwCompSize)
{
	if (posBlock >= BLOCK_TOTAL_COUNT)
		return false;

	Pixel apxBuf[SGuildMarkBlock::SIZE];
	size_t sizeBuf = sizeof(apxBuf);

	if (LZO_E_OK != lzo1x_decompress_safe(pbComp, dwCompSize, (BYTE *) apxBuf, (lzo_uint*) &sizeBuf, CLZO::Instance().GetWorkMemory()))
	{
		sys_err("CGuildMarkImage::CopyBlockFromCompressedData: cannot decompress, compressed size = %u", dwCompSize);
		return false;
	}

	if (sizeBuf != sizeof(apxBuf))
	{
		sys_err("CGuildMarkImage::CopyBlockFromCompressedData: image corrupted, decompressed size = %zu", sizeBuf);
		return false;
	}

	DWORD rowBlock = posBlock / BLOCK_COL_COUNT;
	DWORD colBlock = posBlock % BLOCK_COL_COUNT;

	PutData(colBlock * SGuildMarkBlock::WIDTH, rowBlock * SGuildMarkBlock::HEIGHT, SGuildMarkBlock::WIDTH, SGuildMarkBlock::HEIGHT, apxBuf);

	m_aakBlock[rowBlock][colBlock].CopyFrom(pbComp, dwCompSize, GetCRC32((const char *) apxBuf, sizeof(Pixel) * SGuildMarkBlock::SIZE));
	return true;
}

void CGuildMarkImage::BuildAllBlocks()
{
	Pixel apxBuf[SGuildMarkBlock::SIZE];
	sys_log(0, "CGuildMarkImage::BuildAllBlocks");

	for (UINT row = 0; row < BLOCK_ROW_COUNT; ++row)
		for (UINT col = 0; col < BLOCK_COL_COUNT; ++col)
		{
			GetData(col * SGuildMarkBlock::WIDTH, row * SGuildMarkBlock::HEIGHT, SGuildMarkBlock::WIDTH, SGuildMarkBlock::HEIGHT, apxBuf);
			m_aakBlock[row][col].Compress(apxBuf);
		}
}

DWORD CGuildMarkImage::GetEmptyPosition()
{
	SGuildMark kMark;

	for (DWORD row = 0; row < MARK_ROW_COUNT; ++row)
	{
		for (DWORD col = 0; col < MARK_COL_COUNT; ++col)
		{
			GetData(col * SGuildMark::WIDTH, row * SGuildMark::HEIGHT, SGuildMark::WIDTH, SGuildMark::HEIGHT, kMark.m_apxBuf);

			if (kMark.IsEmpty())
				return (row * MARK_COL_COUNT + col);
		}
	}

	return INVALID_MARK_POSITION;
}

void CGuildMarkImage::GetDiffBlocks(const DWORD * crcList, std::map<BYTE, const SGuildMarkBlock *> & mapDiffBlocks)
{
	BYTE posBlock = 0;

	for (DWORD row = 0; row < BLOCK_ROW_COUNT; ++row)
		for (DWORD col = 0; col < BLOCK_COL_COUNT; ++col)
		{
			if (m_aakBlock[row][col].m_crc != *crcList)
			{
				mapDiffBlocks.insert(std::map<BYTE, const SGuildMarkBlock *>::value_type(posBlock, &m_aakBlock[row][col]));
			}
			++crcList;
			++posBlock;
		}
}

void CGuildMarkImage::GetBlockCRCList(DWORD * crcList)
{
	for (DWORD row = 0; row < BLOCK_ROW_COUNT; ++row)
		for (DWORD col = 0; col < BLOCK_COL_COUNT; ++col)
			*(crcList++) = m_aakBlock[row][col].GetCRC();
}

////////////////////////////////////////////////////////////////////////////////
void SGuildMark::Clear()
{
	for (DWORD iPixel = 0; iPixel < SIZE; ++iPixel)
		m_apxBuf[iPixel] = 0xff000000;	
}

bool SGuildMark::IsEmpty()
{
	for (DWORD iPixel = 0; iPixel < SIZE; ++iPixel)
		if (m_apxBuf[iPixel] != 0x00000000)
			return false;

	return true;
}

////////////////////////////////////////////////////////////////////////////////
DWORD SGuildMarkBlock::GetCRC() const
{
	return m_crc;
}

void SGuildMarkBlock::CopyFrom(const BYTE * pbCompBuf, DWORD dwCompSize, DWORD crc)
{
	if (dwCompSize > MAX_COMP_SIZE)
		return;

	m_sizeCompBuf = dwCompSize;
	thecore_memcpy(m_abCompBuf, pbCompBuf, dwCompSize);
	m_crc = crc;
}

void SGuildMarkBlock::Compress(const Pixel * pxBuf)
{
	m_sizeCompBuf = MAX_COMP_SIZE;

	if (LZO_E_OK != lzo1x_999_compress((const BYTE *) pxBuf,
		sizeof(Pixel) * SGuildMarkBlock::SIZE, m_abCompBuf,
		(lzo_uint*) &m_sizeCompBuf,
		CLZO::Instance().GetWorkMemory()))
	{
		sys_err("SGuildMarkBlock::Compress: Error! %zu > %zu", sizeof(Pixel) * SGuildMarkBlock::SIZE, m_sizeCompBuf);
		return;
	}

	m_crc = GetCRC32((const char *) pxBuf, sizeof(Pixel) * SGuildMarkBlock::SIZE);
}
