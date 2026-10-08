#include "Stdafx.h"
#include "SoundManager2D.h"

CSoundInstance2D CSoundManager2D::ms_Instances[INSTANCE_MAX_COUNT];

CSoundManager2D::CSoundManager2D()
{
}

CSoundManager2D::~CSoundManager2D()
{	
}

bool CSoundManager2D::Initialize()
{
	CSoundBase::Initialize();

	for (int i = 0; i < INSTANCE_MAX_COUNT; ++i)
		ms_Instances[i].Initialize();

	return true;
}

void CSoundManager2D::Destroy()
{	
	for (int i = 0; i < INSTANCE_MAX_COUNT; ++i)
		ms_Instances[i].Destroy();

	CSoundBase::Destroy();
}

ISoundInstance * CSoundManager2D::GetInstance(const char * c_pszFileName)
{
	DWORD dwFileCRC = GetFileCRC(c_pszFileName);
	TSoundDataMap::iterator itor = ms_dataMap.find(dwFileCRC);

	CSoundData * pkSoundData;

	if (itor == ms_dataMap.end())
		pkSoundData = AddFile(dwFileCRC, c_pszFileName);
	else
		pkSoundData = itor->second;

	assert(pkSoundData != NULL);

	static DWORD k = 0;

	DWORD start = k++;
	DWORD end = start + INSTANCE_MAX_COUNT;

	while (start < end)
	{
		CSoundInstance2D * pkInst = &ms_Instances[start % INSTANCE_MAX_COUNT];

		if (pkInst->IsDone())
		{
			if (!pkInst->SetSound(pkSoundData))
				TraceError("CSoundManager2D::GetInstance (filename: %s)", c_pszFileName);
			return (pkInst);
		}

		++start;
	}

	return NULL;
}
