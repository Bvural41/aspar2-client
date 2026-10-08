#pragma once

#include "SoundInstance.h"

class CSoundManager3D : public CSoundBase
{
	public:
		enum
		{
			INSTANCE_MAX_COUNT = 32,
		};

	public:
		CSoundManager3D();
		virtual ~CSoundManager3D();

		bool					Initialize();
		void					Destroy();

		void					SetListenerPosition(float fX, float fY, float fZ);
		void					SetListenerDirection(float fxDir, float fyDir, float fzDir, float fxUp, float fyUp, float fzUp);
		void					SetListenerVelocity(float fDistanceX, float fDistanceY, float fDistanceZ, float fNagnitude);

		int						SetInstance(const char * c_pszFileName);
		ISoundInstance *		GetInstance(DWORD dwIndex);

		void					Lock(int iIndex);
		void					Unlock(int iIndex);

	protected:
		bool					IsValidInstanceIndex(int iIndex);

	protected:
		bool					m_bInit;
		bool					m_bLockingFlag[INSTANCE_MAX_COUNT];
		CSoundInstance3D		m_Instances[INSTANCE_MAX_COUNT];
};
