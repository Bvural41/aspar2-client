#include "StdAfx.h"
#include "SoundData.h"

#include "../EterPack/EterPackManager.h"
#include "../eterBase/Timer.h"

const char * CSoundData::GetFileName()
{
	return m_filename;
}

void CSoundData::Assign(const char* filename)
{
	assert(m_assigned == false);

	strncpy(m_filename, filename, sizeof(m_filename)-1);
	m_assigned = true;
}

LPVOID CSoundData::Get()
{
	++m_iRefCount;
	m_dwAccessTime = ELTimer_GetMSec(); 

	if (!m_bLoaded)
		ReadFromDisk();

	return m_pPCMData;
}
 
ULONG CSoundData::GetSize()
{
	if (!m_pPCMData || m_pcmFrameCount == 0)
		return 0;
	return (ULONG)(m_pcmFrameCount * ma_get_bytes_per_frame(m_format, m_channels));
}

void CSoundData::Release()
{
	assert(m_iRefCount != 0);
	--m_iRefCount;
	m_dwAccessTime = ELTimer_GetMSec();
}

DWORD CSoundData::GetPlayTime()
{
	return m_dwPlayTime;
}

void CSoundData::SetPlayTime(DWORD dwPlayTime)
{
	m_dwPlayTime = dwPlayTime;
}

DWORD CSoundData::GetAccessTime()
{
	return m_dwAccessTime;
}

bool CSoundData::ReadFromDisk()
{
	assert(m_assigned == true);

	if (m_bLoaded)
		return true;

	CMappedFile mappedFile;
	LPCVOID pFileData = NULL;
	const void* pRawBytes = NULL;
	size_t rawSize = 0;
	std::vector<unsigned char> diskBuffer;

	// 1. Try reading from EterPack (epk/eix)
	if (CEterPackManager::Instance().Get(mappedFile, m_filename, &pFileData))
	{
		pRawBytes = pFileData;
		rawSize = mappedFile.Size();
	}
	else
	{
		// 2. Try reading from disk file
		FILE* fp = fopen(m_filename, "rb");
		if (!fp)
		{
			return false;
		}
		fseek(fp, 0, SEEK_END);
		rawSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		if (rawSize == 0)
		{
			fclose(fp);
			return false;
		}

		diskBuffer.resize(rawSize);
		fread(&diskBuffer[0], 1, rawSize, fp);
		fclose(fp);
		pRawBytes = &diskBuffer[0];
	}

	if (!pRawBytes || rawSize == 0)
		return false;

	// 3. Decode audio using miniaudio decoder
	ma_decoder decoder;
	ma_result result = ma_decoder_init_memory(pRawBytes, rawSize, NULL, &decoder);
	if (result != MA_SUCCESS)
	{
		TraceError("CSoundData::ReadFromDisk - ma_decoder_init_memory failed for %s (error: %d)", m_filename, result);
		return false;
	}

	ma_uint64 totalFrameCount = 0;
	result = ma_decoder_get_length_in_pcm_frames(&decoder, &totalFrameCount);
	if (result != MA_SUCCESS || totalFrameCount == 0)
	{
		ma_decoder_uninit(&decoder);
		return false;
	}

	size_t bytesPerFrame = ma_get_bytes_per_frame(decoder.outputFormat, decoder.outputChannels);
	size_t bufferSize = (size_t)(totalFrameCount * bytesPerFrame);

	m_pPCMData = malloc(bufferSize);
	if (!m_pPCMData)
	{
		ma_decoder_uninit(&decoder);
		return false;
	}

	ma_uint64 framesRead = 0;
	result = ma_decoder_read_pcm_frames(&decoder, m_pPCMData, totalFrameCount, &framesRead);
	if (result != MA_SUCCESS && framesRead == 0)
	{
		free(m_pPCMData);
		m_pPCMData = NULL;
		ma_decoder_uninit(&decoder);
		return false;
	}

	m_pcmFrameCount = framesRead;
	m_format = decoder.outputFormat;
	m_channels = decoder.outputChannels;
	m_sampleRate = decoder.outputSampleRate;
	m_bLoaded = true;

	ma_decoder_uninit(&decoder);
	return true;
}

void CSoundData::SetPackMode()
{
}

void CSoundData::Destroy()
{
	if (m_pPCMData)
	{
		free(m_pPCMData);
		m_pPCMData = NULL;
	}
	m_pcmFrameCount = 0;
	m_bLoaded = false;
}

CSoundData::CSoundData() : 
m_assigned(false),
m_bLoaded(false),
m_iRefCount(0),
m_dwPlayTime(0),
m_dwAccessTime(ELTimer_GetMSec()),
m_pPCMData(NULL),
m_pcmFrameCount(0),
m_format(ma_format_unknown),
m_channels(0),
m_sampleRate(0)
{
	memset(m_filename, 0, sizeof(m_filename));
}

CSoundData::~CSoundData()
{
	Destroy();
}
