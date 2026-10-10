#include "StdAfx.h"
#include "../EterBase/MappedFile.h"
#include "../EterPack/EterPackManager.h"
#include "GrpImageTexture.h"

#if defined(USE_OPENGL_ES)
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <vector>
#endif

bool CGraphicImageTexture::Lock(int* pRetPitch, void** ppRetPixels, int level) const
{
#if defined(USE_OPENGL_ES)
	if (!m_uGLTexture)
		return false;

	int bytesPerPixel = (m_d3dFmt == D3DFMT_A4R4G4B4) ? 2 : 4;
	int pitch = m_width * bytesPerPixel;
	int totalBytes = pitch * m_height;

	if (!m_pPixelData && totalBytes > 0)
	{
		m_pPixelData = malloc(totalBytes);
		if (m_pPixelData)
			memset(m_pPixelData, 0, totalBytes);
	}

	if (!m_pPixelData)
		return false;

	*pRetPitch = pitch;
	*ppRetPixels = m_pPixelData;
	return true;
#else
	D3DLOCKED_RECT lockedRect;
	if (FAILED(m_lpd3dTexture->LockRect(level, &lockedRect, nullptr, 0)))
		return false;

	*pRetPitch = lockedRect.Pitch;
	*ppRetPixels = (void*)lockedRect.pBits;
	return true;
#endif
}

void CGraphicImageTexture::Unlock(int level) const
{
#if defined(USE_OPENGL_ES)
	if (!m_uGLTexture || !m_pPixelData)
		return;

	glBindTexture(GL_TEXTURE_2D, m_uGLTexture);
	if (m_d3dFmt == D3DFMT_A4R4G4B4)
	{
		glTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, m_pPixelData);
	}
	else
	{
		glTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, m_pPixelData);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
#else
	assert(m_lpd3dTexture != nullptr);
	m_lpd3dTexture->UnlockRect(level);
#endif
}

void CGraphicImageTexture::Initialize()
{
	CGraphicTexture::Initialize();

	m_stFileName = "";

	m_d3dFmt=D3DFMT_UNKNOWN;
	m_dwFilter=0;

#if defined(USE_OPENGL_ES)
	m_pPixelData = nullptr;
	m_nPixelPitch = 0;
#endif
}

void CGraphicImageTexture::Destroy()
{
	CGraphicTexture::Destroy();

#if defined(USE_OPENGL_ES)
	if (m_pPixelData)
	{
		free(m_pPixelData);
		m_pPixelData = nullptr;
	}
#endif

	Initialize();
}

bool CGraphicImageTexture::CreateDeviceObjects()
{
#if defined(USE_OPENGL_ES)
	if (m_uGLTexture != 0)
		return true;

	if (m_stFileName.empty())
	{
		glGenTextures(1, &m_uGLTexture);
		if (!m_uGLTexture)
			return false;

		glBindTexture(GL_TEXTURE_2D, m_uGLTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		if (m_d3dFmt == D3DFMT_A4R4G4B4)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, nullptr);
			m_nPixelPitch = m_width * 2;
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			m_nPixelPitch = m_width * 4;
		}
		glBindTexture(GL_TEXTURE_2D, 0);

		if (!m_pPixelData && m_width > 0 && m_height > 0)
		{
			int totalBytes = m_nPixelPitch * m_height;
			m_pPixelData = malloc(totalBytes);
			if (m_pPixelData)
				memset(m_pPixelData, 0, totalBytes);
		}

		m_bEmpty = false;
		return true;
	}
	else
	{
		CMappedFile	mappedFile;
		LPCVOID		c_pvMap;

		if (!CEterPackManager::Instance().Get(mappedFile, m_stFileName.c_str(), &c_pvMap))
			return false;

		if (!CreateFromMemoryFile(mappedFile.Size(), c_pvMap, m_d3dFmt, m_dwFilter))
		{
			TraceError("CGraphicImageTexture::CreateDeviceObjects: CreateFromMemoryFile: texture not found(%s)", m_stFileName.c_str());
			return false;
		}
		return true;
	}
#else
	assert(ms_lpd3dDevice != nullptr);
	assert(m_lpd3dTexture == nullptr);

	if (m_stFileName.empty())
	{
		DWORD usage = 0;
		if (m_d3dFmt == D3DFMT_A8R8G8B8 || m_d3dFmt == D3DFMT_A4R4G4B4)
			usage = D3DUSAGE_DYNAMIC;

		if (FAILED(ms_lpd3dDevice->CreateTexture(m_width, m_height, 1, usage, m_d3dFmt, D3DPOOL_DEFAULT, &m_lpd3dTexture, nullptr)))
			return false;
	}
	else
	{
		CMappedFile	mappedFile;
		LPCVOID		c_pvMap;

		if (!CEterPackManager::Instance().Get(mappedFile, m_stFileName.c_str(), &c_pvMap))
			return false;

		//@fixme002
		if (!CreateFromMemoryFile(mappedFile.Size(), c_pvMap, m_d3dFmt, m_dwFilter))
		{
			TraceError("CGraphicImageTexture::CreateDeviceObjects: CreateFromMemoryFile: texture not found(%s)", m_stFileName.c_str());
			return false;
		}
		return true;
	}

	m_bEmpty = false;
	return true;
#endif
}

bool CGraphicImageTexture::Create(UINT width, UINT height, D3DFORMAT d3dFmt, DWORD dwFilter)
{
#if !defined(USE_OPENGL_ES)
	assert(ms_lpd3dDevice != nullptr);
#endif
	Destroy();

	m_width = width;
	m_height = height;
	m_d3dFmt = d3dFmt;
	m_dwFilter = dwFilter;

	return CreateDeviceObjects();
}

void CGraphicImageTexture::CreateFromTexturePointer(const CGraphicTexture * c_pSrcTexture)
{
#if defined(USE_OPENGL_ES)
	m_width = c_pSrcTexture->GetWidth();
	m_height = c_pSrcTexture->GetHeight();
	m_uGLTexture = c_pSrcTexture->GetGLTexture();
	m_bEmpty = false;
#else
	if (m_lpd3dTexture)
		m_lpd3dTexture->Release();

	m_width = c_pSrcTexture->GetWidth();
	m_height = c_pSrcTexture->GetHeight();
	m_lpd3dTexture = c_pSrcTexture->GetD3DTexture();

	if (m_lpd3dTexture)
		m_lpd3dTexture->AddRef();

	m_bEmpty = false;
#endif
}

#if defined(USE_OPENGL_ES)
static bool ShouldGenerateMipmaps(const std::string& fileName, int mipCount = 0)
{
	if (fileName.empty())
		return false;

	std::string lower = fileName;
	for (char& c : lower) c = (char)tolower((unsigned char)c);

	// Terrain tile textures are always sampled with a mipmapped min filter by the terrain renderer
	// (SetupTerrainShaderAndStates). A texture without mip levels would be "incomplete" and sample as
	// opaque black, so always build mipmaps for them regardless of the DDS mip count.
	if (lower.find("terrainmaps") != std::string::npos)
		return true;

	if (mipCount == 1)
		return false;

	if (lower.find("ui/") != std::string::npos ||
		lower.find("uiscript/") != std::string::npos ||
		lower.find("/ui/") != std::string::npos ||
		lower.find("icon/") != std::string::npos ||
		lower.find("minimap/") != std::string::npos ||
		lower.find("guild/") != std::string::npos ||
		lower.find("mark/") != std::string::npos ||
		lower.find("font") != std::string::npos ||
		lower.find("cursor") != std::string::npos)
	{
		return false;
	}

	return true;
}
#endif

bool CGraphicImageTexture::CreateDDSTexture(CDXTCImage & image, const BYTE * /*c_pbBuf*/)
{
#if defined(USE_OPENGL_ES)
	int w = image.m_nWidth;
	int h = image.m_nHeight;
#ifdef __ANDROID__
#endif
	if (w <= 0 || h <= 0)
		return false;

	std::vector<DWORD> pixels(w * h);
#ifdef __ANDROID__
#endif
	image.Decompress(0, pixels.data());
#ifdef __ANDROID__
#endif

	// Convert BGRA to RGBA
	for (int i = 0; i < w * h; ++i)
	{
		DWORD c = pixels[i];
		pixels[i] = (c & 0xFF00FF00) | ((c & 0x00FF0000) >> 16) | ((c & 0x000000FF) << 16);
	}

	glGenTextures(1, &m_uGLTexture);
	if (!m_uGLTexture)
		return false;

	glBindTexture(GL_TEXTURE_2D, m_uGLTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

	bool bGenMipmap = ShouldGenerateMipmaps(m_stFileName, image.m_dwMipMapCount);
	if (bGenMipmap)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	m_width = w;
	m_height = h;
	m_bEmpty = false;
#ifdef __ANDROID__
#endif
	return true;
#else
	int mipmapCount = image.m_dwMipMapCount == 0 ? 1 : image.m_dwMipMapCount;

	D3DFORMAT format;
	LPDIRECT3DTEXTURE9 lpd3dTexture;

	if(image.m_CompFormat == PF_DXT5)
		format = D3DFMT_DXT5;
	else if(image.m_CompFormat == PF_DXT3)
		format = D3DFMT_DXT3;
	else
		format = D3DFMT_DXT1;

	UINT uTexBias=0;
	if (IsLowTextureMemory())
		uTexBias=1;

	UINT uMinMipMapIndex=0;
	if (uTexBias>0)
	{
		if (mipmapCount>uTexBias)
		{
			uMinMipMapIndex=uTexBias;
			image.m_nWidth>>=uTexBias;
			image.m_nHeight>>=uTexBias;
			mipmapCount-=uTexBias;
		}
	}

	if (FAILED(D3DXCreateTexture(	ms_lpd3dDevice, image.m_nWidth, image.m_nHeight,
									mipmapCount, D3DUSAGE_DYNAMIC, format, D3DPOOL_DEFAULT, &lpd3dTexture)))
	{
		TraceError("CreateDDSTexture: Cannot creatre texture");
		return false;
	}

	for (DWORD i = 0; i < mipmapCount; ++i)
	{
		D3DLOCKED_RECT lockedRect;

		if (FAILED(lpd3dTexture->LockRect(i, &lockedRect, nullptr, 0)))
		{
			TraceError("CreateDDSTexture: Cannot lock texture");
		}
		else
		{
			image.Copy(i+uMinMipMapIndex, (BYTE*)lockedRect.pBits, lockedRect.Pitch);
			lpd3dTexture->UnlockRect(i);
		}
	}

	if(ms_bSupportDXT)
	{
		m_lpd3dTexture = lpd3dTexture;
	}
	else
	{
		if(image.m_CompFormat == PF_DXT3 || image.m_CompFormat == PF_DXT5)
			format = D3DFMT_A4R4G4B4;
		else
			format = D3DFMT_A1R5G5B5;

		UINT imgWidth=image.m_nWidth;
		UINT imgHeight=image.m_nHeight;

		extern bool GRAPHICS_CAPS_HALF_SIZE_IMAGE;

		if (GRAPHICS_CAPS_HALF_SIZE_IMAGE && uTexBias>0 && mipmapCount==0)
		{
			imgWidth>>=uTexBias;
			imgHeight>>=uTexBias;
		}

		if (FAILED(D3DXCreateTexture(ms_lpd3dDevice, imgWidth, imgHeight,
			mipmapCount, 0, format, D3DPOOL_DEFAULT, &m_lpd3dTexture)))
		{
				TraceError("CreateDDSTexture: Cannot creatre texture");
				return false;
		}

		IDirect3DTexture9* pkTexSrc=lpd3dTexture;
		IDirect3DTexture9* pkTexDst=m_lpd3dTexture;

		for(int i=0; i<mipmapCount; ++i) {
			IDirect3DSurface9* ppsSrc = nullptr;
			IDirect3DSurface9* ppsDst = nullptr;

			if (SUCCEEDED(pkTexSrc->GetSurfaceLevel(i, &ppsSrc)))
			{
				if (SUCCEEDED(pkTexDst->GetSurfaceLevel(i, &ppsDst)))
				{
					D3DXLoadSurfaceFromSurface(ppsDst, nullptr, nullptr, ppsSrc, nullptr, nullptr, D3DX_FILTER_NONE, 0);
					ppsDst->Release();
				}
				ppsSrc->Release();
			}
		}

		lpd3dTexture->Release();
	}

	m_width = image.m_nWidth;
	m_height = image.m_nHeight;
	m_bEmpty = false;

	return true;
#endif
}

bool CGraphicImageTexture::CreateFromMemoryFile(UINT bufSize, const void * c_pvBuf, D3DFORMAT d3dFmt, DWORD dwFilter)
{
#if defined(USE_OPENGL_ES)
#ifdef __ANDROID__
#endif
	CDXTCImage image;

	if (image.LoadHeaderFromMemory((const BYTE *) c_pvBuf))
	{
		return (CreateDDSTexture(image, (const BYTE *) c_pvBuf));
	}

	// Check for uncompressed DDS file (Magic 'DDS ')
	if (bufSize > 128 && *(const DWORD*)c_pvBuf == 0x20534444)
	{
		D3DXIMAGE_INFO d3dxInfo;
		if (SUCCEEDED(D3DXGetImageInfoFromFileInMemory(c_pvBuf, bufSize, &d3dxInfo)))
		{
			UINT w = d3dxInfo.Width;
			UINT h = d3dxInfo.Height;
			if (w > 0 && h > 0)
			{
				std::vector<DWORD> rgba(w * h);
				const BYTE* pPixels = (const BYTE*)c_pvBuf + 128;

				if (d3dxInfo.Format == D3DFMT_A1R5G5B5) // 25
				{
					const WORD* pSrc = (const WORD*)pPixels;
					for (UINT i = 0; i < w * h; ++i)
					{
						WORD px = pSrc[i];
						BYTE a = (px & 0x8000) ? 255 : 0;
						BYTE r = (BYTE)(((px >> 10) & 0x1F) * 255 / 31);
						BYTE g = (BYTE)(((px >> 5) & 0x1F) * 255 / 31);
						BYTE b = (BYTE)((px & 0x1F) * 255 / 31);
						rgba[i] = (a << 24) | (b << 16) | (g << 8) | r;
					}
				}
				else if (d3dxInfo.Format == D3DFMT_A4R4G4B4) // 26
				{
					const WORD* pSrc = (const WORD*)pPixels;
					for (UINT i = 0; i < w * h; ++i)
					{
						WORD px = pSrc[i];
						BYTE a = (BYTE)(((px >> 12) & 0x0F) * 17);
						BYTE r = (BYTE)(((px >> 8) & 0x0F) * 17);
						BYTE g = (BYTE)(((px >> 4) & 0x0F) * 17);
						BYTE b = (BYTE)((px & 0x0F) * 17);
						rgba[i] = (a << 24) | (b << 16) | (g << 8) | r;
					}
				}
				else if (d3dxInfo.Format == D3DFMT_R5G6B5) // 23
				{
					const WORD* pSrc = (const WORD*)pPixels;
					for (UINT i = 0; i < w * h; ++i)
					{
						WORD px = pSrc[i];
						BYTE a = 255;
						BYTE r = (BYTE)(((px >> 11) & 0x1F) * 255 / 31);
						BYTE g = (BYTE)(((px >> 5) & 0x3F) * 255 / 63);
						BYTE b = (BYTE)((px & 0x1F) * 255 / 31);
						rgba[i] = (a << 24) | (b << 16) | (g << 8) | r;
					}
				}
				else if (d3dxInfo.Format == D3DFMT_A8R8G8B8 || d3dxInfo.Format == D3DFMT_X8R8G8B8) // 21, 22
				{
					const DWORD* pSrc = (const DWORD*)pPixels;
					for (UINT i = 0; i < w * h; ++i)
					{
						DWORD px = pSrc[i];
						BYTE a = (d3dxInfo.Format == D3DFMT_X8R8G8B8) ? 255 : (BYTE)((px >> 24) & 0xFF);
						BYTE r = (BYTE)((px >> 16) & 0xFF);
						BYTE g = (BYTE)((px >> 8) & 0xFF);
						BYTE b = (BYTE)(px & 0xFF);
						rgba[i] = (a << 24) | (b << 16) | (g << 8) | r;
					}
				}
				else if (d3dxInfo.Format == D3DFMT_R8G8B8) // 20
				{
					const BYTE* pSrc = pPixels;
					for (UINT i = 0; i < w * h; ++i)
					{
						BYTE b = pSrc[i * 3 + 0];
						BYTE g = pSrc[i * 3 + 1];
						BYTE r = pSrc[i * 3 + 2];
						BYTE a = 255;
						rgba[i] = (a << 24) | (b << 16) | (g << 8) | r;
					}
				}

				glGenTextures(1, &m_uGLTexture);
				if (m_uGLTexture)
				{
					glBindTexture(GL_TEXTURE_2D, m_uGLTexture);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
					bool bGenMipmap = ShouldGenerateMipmaps(m_stFileName, d3dxInfo.MipLevels);
					if (bGenMipmap)
					{
						glGenerateMipmap(GL_TEXTURE_2D);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					}
					else
					{
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					}
					glBindTexture(GL_TEXTURE_2D, 0);

					m_width = w;
					m_height = h;
					m_bEmpty = false;
					return true;
				}
			}
		}
		// It is a DDS file ('DDS '), never pass to stbi_load_from_memory
		TraceError("CreateFromMemoryFile: unsupported or failed DDS texture %s (size=%u)", m_stFileName.c_str(), bufSize);
		return false;
	}

	{
		int w = 0, h = 0, channels = 0;
		stbi_uc* pImgData = stbi_load_from_memory((const stbi_uc*)c_pvBuf, (int)bufSize, &w, &h, &channels, 4);
		if (!pImgData)
		{
			TraceError("CreateFromMemoryFile: stbi_load_from_memory failed for %s (size=%u, reason=%s)",
				m_stFileName.c_str(), bufSize, stbi_failure_reason());
			D3DXIMAGE_INFO d3dxInfo;
			if (SUCCEEDED(D3DXGetImageInfoFromFileInMemory(c_pvBuf, bufSize, &d3dxInfo)))
			{
				TraceError(" -> D3DX Info: %ux%u format=%d resourceType=%d",
					d3dxInfo.Width, d3dxInfo.Height, d3dxInfo.Format, d3dxInfo.ResourceType);
			}
			return false;
		}

		// Color key: replace magenta (255, 0, 255) with transparent
		for (int i = 0; i < w * h; ++i)
		{
			if (pImgData[i * 4 + 0] == 255 && pImgData[i * 4 + 1] == 0 && pImgData[i * 4 + 2] == 255)
			{
				pImgData[i * 4 + 0] = 0;
				pImgData[i * 4 + 1] = 0;
				pImgData[i * 4 + 2] = 0;
				pImgData[i * 4 + 3] = 0;
			}
		}

		glGenTextures(1, &m_uGLTexture);
		if (!m_uGLTexture)
		{
			stbi_image_free(pImgData);
			return false;
		}

		glBindTexture(GL_TEXTURE_2D, m_uGLTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImgData);
		bool bGenMipmap = ShouldGenerateMipmaps(m_stFileName, 0);
		if (bGenMipmap)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		glBindTexture(GL_TEXTURE_2D, 0);

		stbi_image_free(pImgData);

		m_width = w;
		m_height = h;
		m_bEmpty = false;
		return true;
	}
#else
	assert(ms_lpd3dDevice != nullptr);
	assert(m_lpd3dTexture == nullptr);

	CDXTCImage image;

	if (image.LoadHeaderFromMemory((const BYTE *) c_pvBuf))
	{
		return (CreateDDSTexture(image, (const BYTE *) c_pvBuf));
	}
	else
	{
		D3DXIMAGE_INFO imageInfo;
		if (FAILED(D3DXGetImageInfoFromFileInMemory(c_pvBuf, bufSize, &imageInfo)))
		{
			TraceError("CreateFromMemoryFile: Cannot GetImageInfo from texture");
			return false;
		}
		if (FAILED(D3DXCreateTextureFromFileInMemoryEx(
					ms_lpd3dDevice,
					c_pvBuf,
					bufSize,
					imageInfo.Width,
					imageInfo.Height,
					D3DX_DEFAULT,
					0,
					d3dFmt,
					D3DPOOL_DEFAULT,
					dwFilter,
					dwFilter,
					0xffff00ff,
					nullptr,
					nullptr,
					&m_lpd3dTexture)))
		{
			TraceError("CreateFromMemoryFile: Cannot create texture");
			return false;
		}

		m_width = imageInfo.Width;
		m_height = imageInfo.Height;

		D3DFORMAT format=imageInfo.Format;
		switch(imageInfo.Format) {
			case D3DFMT_A8R8G8B8:
				format = D3DFMT_A4R4G4B4;
				break;

			case D3DFMT_X8R8G8B8:
			case D3DFMT_R8G8B8:
				format = D3DFMT_A1R5G5B5;
				break;
		}

		UINT uTexBias=0;

		extern bool GRAPHICS_CAPS_HALF_SIZE_IMAGE;
		if (GRAPHICS_CAPS_HALF_SIZE_IMAGE)
			uTexBias=1;

		if (IsLowTextureMemory())
		if (uTexBias || format!=imageInfo.Format)
		{
			IDirect3DTexture9* pkTexSrc=m_lpd3dTexture;
			IDirect3DTexture9* pkTexDst;

			if (SUCCEEDED(D3DXCreateTexture(
				ms_lpd3dDevice,
				imageInfo.Width>>uTexBias,
				imageInfo.Height>>uTexBias,
				imageInfo.MipLevels,
				0,
				format,
				D3DPOOL_DEFAULT,
				&pkTexDst)))
			{
				m_lpd3dTexture=pkTexDst;

				for(int i=0; i<imageInfo.MipLevels; ++i) {
					IDirect3DSurface9* ppsSrc = nullptr;
					IDirect3DSurface9* ppsDst = nullptr;

					if (SUCCEEDED(pkTexSrc->GetSurfaceLevel(i, &ppsSrc)))
					{
						if (SUCCEEDED(pkTexDst->GetSurfaceLevel(i, &ppsDst)))
						{
							D3DXLoadSurfaceFromSurface(ppsDst, nullptr, nullptr, ppsSrc, nullptr, nullptr, D3DX_FILTER_LINEAR, 0);
							ppsDst->Release();
						}
						ppsSrc->Release();
					}
				}

				pkTexSrc->Release();
			}
		}
	}

	m_bEmpty = false;
	return true;
#endif
}

void CGraphicImageTexture::SetFileName(const char * c_szFileName)
{
	m_stFileName=c_szFileName;
}

bool CGraphicImageTexture::CreateFromDiskFile(const char * c_szFileName, D3DFORMAT d3dFmt, DWORD dwFilter)
{
	Destroy();

	SetFileName(c_szFileName);

	m_d3dFmt = d3dFmt;
	m_dwFilter = dwFilter;
	return CreateDeviceObjects();
}

CGraphicImageTexture::CGraphicImageTexture()
{
	Initialize();
}

CGraphicImageTexture::~CGraphicImageTexture()
{
	Destroy();
}
