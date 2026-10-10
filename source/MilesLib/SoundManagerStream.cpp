#include "StdAfx.h"
#include "SoundManagerStream.h"

CSoundManagerStream::CSoundManagerStream()
{
}

CSoundManagerStream::~CSoundManagerStream()
{	
}

bool CSoundManagerStream::Initialize()
{
	CSoundBase::Initialize();

	for (int i = 0; i < MUSIC_INSTANCE_MAX_NUM; ++i)
		m_Instances[i].Initialize();

	return true;
}

void CSoundManagerStream::Destroy()
{	
	for (int i = 0; i < MUSIC_INSTANCE_MAX_NUM; ++i)
		m_Instances[i].Destroy();

	CSoundBase::Destroy();
}

bool CSoundManagerStream::SetInstance(DWORD dwIndex, const char* filename)
{
	if (!CheckInstanceIndex(dwIndex))
		return false;

	return m_Instances[dwIndex].SetStream(filename);
}

CSoundInstanceStream * CSoundManagerStream::GetInstance(DWORD dwIndex)
{
	if (!CheckInstanceIndex(dwIndex))
		return NULL;

	return &m_Instances[dwIndex];
}

bool CSoundManagerStream::CheckInstanceIndex(DWORD dwIndex)
{
	if (dwIndex >= DWORD(MUSIC_INSTANCE_MAX_NUM))
		return false;

	return true;
}
