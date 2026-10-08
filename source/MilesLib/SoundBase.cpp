#include "Stdafx.h"

#ifdef interface
#undef interface
#endif

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include "SoundBase.h"

ma_engine				CSoundBase::ms_engine;
TSoundDataMap			CSoundBase::ms_dataMap;
bool					CSoundBase::ms_bInitialized = false;
int						CSoundBase::ms_iRefCount = 0;

CSoundBase::CSoundBase()
{
}

CSoundBase::~CSoundBase()
{
}

void CSoundBase::Destroy()
{
	if (ms_iRefCount > 1)
	{
		--ms_iRefCount;
		return;
	}

	ms_iRefCount = 0;

	if (!ms_dataMap.empty())
	{
		TSoundDataMap::iterator i;
		for (i = ms_dataMap.begin(); i != ms_dataMap.end(); ++i)
		{
			CSoundData * pSoundData = i->second;
			delete pSoundData;
		}

		ms_dataMap.clear();
	}

	if (ms_bInitialized)
	{
		ma_engine_uninit(&ms_engine);
		ms_bInitialized = false;
	}
}

void CSoundBase::Initialize()
{
	++ms_iRefCount;

	if (ms_iRefCount > 1)
		return;

	ms_dataMap.clear();

	ma_engine_config engineConfig = ma_engine_config_init();
	engineConfig.listenerCount = 1;

	ma_result result = ma_engine_init(&engineConfig, &ms_engine);
	if (result != MA_SUCCESS)
	{
		TraceError("CSoundBase::Initialize - ma_engine_init failed (result: %d)", result);
		ms_bInitialized = false;
		return;
	}

	ms_bInitialized = true;
}

DWORD CSoundBase::GetFileCRC(const char * filename)
{
	return GetCRC32(filename, strlen(filename));
}

CSoundData * CSoundBase::AddFile(DWORD dwFileCRC, const char* filename)
{
	CSoundData * pSoundData = new CSoundData;
	pSoundData->Assign(filename);
	ms_dataMap.insert(TSoundDataMap::value_type(dwFileCRC, pSoundData));
	return pSoundData;
}

void CSoundBase::PauseAudio()
{
	if (ms_bInitialized)
	{
		ma_engine_stop(&ms_engine);
	}
}

void CSoundBase::ResumeAudio()
{
	if (ms_bInitialized)
	{
		ma_engine_start(&ms_engine);
	}
}
