#pragma once

#include "SoundInstance.h"

class CSoundManager2D : public CSoundBase
{
	public:
		enum
		{
			INSTANCE_MAX_COUNT = 16,
		};

	public:
		CSoundManager2D();
		virtual ~CSoundManager2D();

		bool					Initialize();
		void					Destroy();

		ISoundInstance *		GetInstance(const char * c_pszFileName);

	protected:
		static CSoundInstance2D	ms_Instances[INSTANCE_MAX_COUNT];
};
