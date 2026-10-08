#include "Stdafx.h"
#include "SoundManager3D.h"

CSoundManager3D::CSoundManager3D()
{
	m_bInit = false;
}

CSoundManager3D::~CSoundManager3D()
{
}

bool CSoundManager3D::Initialize()
{
	CSoundBase::Initialize();

	for (int i = 0; i < INSTANCE_MAX_COUNT; ++i)
	{
		m_Instances[i].Initialize();
		m_bLockingFlag[i] = false;
	}

	SetListenerPosition(0.0f, 0.0f, 0.0f);
	SetListenerDirection(0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	m_bInit = true;
	return true;
}

void CSoundManager3D::Destroy()
{		
	if (!m_bInit)
		return;
	
	for (int i = 0; i < INSTANCE_MAX_COUNT; ++i)
		m_Instances[i].Destroy();

	CSoundBase::Destroy();
	m_bInit = false;
}

void CSoundManager3D::SetListenerDirection(float fxDir, float fyDir, float fzDir, float fxUp, float fyUp, float fzUp)
{
	ma_engine* pEngine = CSoundBase::GetEngine();
	if (pEngine)
	{
		ma_engine_listener_set_direction(pEngine, 0, fxDir, fyDir, -fzDir);
		ma_engine_listener_set_world_up(pEngine, 0, fxUp, fyUp, -fzUp);
	}
}

void CSoundManager3D::SetListenerPosition(float fX, float fY, float fZ)
{
	ma_engine* pEngine = CSoundBase::GetEngine();
	if (pEngine)
	{
		ma_engine_listener_set_position(pEngine, 0, fX, fY, -fZ);
	}
}

void CSoundManager3D::SetListenerVelocity(float fDistanceX, float fDistanceY, float fDistanceZ, float fNagnitude)
{
	ma_engine* pEngine = CSoundBase::GetEngine();
	if (pEngine)
	{
		ma_engine_listener_set_velocity(pEngine, 0, fDistanceX, fDistanceY, -fDistanceZ);
	}
}

int CSoundManager3D::SetInstance(const char * c_pszFileName)
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
		int idx = start % INSTANCE_MAX_COUNT;
		if (!m_bLockingFlag[idx])
		{
			CSoundInstance3D * pkInst = &m_Instances[idx];

			if (pkInst->IsDone())
			{
				if (!pkInst->SetSound(pkSoundData))
				{
					TraceError("CSoundManager3D::GetInstance (filename: %s)", c_pszFileName);
					return -1;
				}

				return idx;
			}
		}

		++start;

		if (start > 50000)
		{
			start = 0;
			return -1;
		}
	}

	return -1;
}

ISoundInstance * CSoundManager3D::GetInstance(DWORD dwIndex)
{
	if (dwIndex >= INSTANCE_MAX_COUNT)
	{
		assert(dwIndex < INSTANCE_MAX_COUNT);
		return NULL;
	}
	return &m_Instances[dwIndex];
}

__forceinline bool CSoundManager3D::IsValidInstanceIndex(int iIndex)
{
	if (iIndex >= 0 && iIndex < INSTANCE_MAX_COUNT)
		return true;

	return false;
}

void CSoundManager3D::Lock(int iIndex)
{
	if (!IsValidInstanceIndex(iIndex))
		return;

	m_bLockingFlag[iIndex] = true;
}

void CSoundManager3D::Unlock(int iIndex)
{
	if (!IsValidInstanceIndex(iIndex))
		return;

	m_bLockingFlag[iIndex] = false;
}
