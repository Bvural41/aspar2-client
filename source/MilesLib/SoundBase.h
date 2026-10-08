#ifndef __MILESLIB_CSOUNDBASE_H__
#define __MILESLIB_CSOUNDBASE_H__

#include <map>
#include <vector>
#include <miniaudio.h>
#include "SoundData.h"

typedef std::map<DWORD, CSoundData*> TSoundDataMap;

class CSoundBase
{
	public:
		CSoundBase();
		virtual ~CSoundBase();

		void					Initialize();
		void					Destroy();

		CSoundData *			AddFile(DWORD dwFileCRC, const char* filename);
		DWORD					GetFileCRC(const char* filename);

		static ma_engine*		GetEngine() { return &ms_engine; }
		static void				PauseAudio();
		static void				ResumeAudio();
		static bool				IsInitialized() { return ms_bInitialized; }

	protected:
		static int								ms_iRefCount;
		static ma_engine						ms_engine;
		static TSoundDataMap					ms_dataMap;
		static bool								ms_bInitialized;
};

#endif
