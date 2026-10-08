#ifndef __MILESLIB_CSOUNDDATA_H__
#define __MILESLIB_CSOUNDDATA_H__

#include <miniaudio.h>
#include "../eterBase/MappedFile.h"

class CSoundData
{
	public:
		enum
		{
			SOUND_FILE_MAX_NUM = 5,
		};

	public:
		static void		SetPackMode();

		CSoundData();
		virtual ~CSoundData();

		void			Assign(const char* filename);
		LPVOID			Get();
		ULONG			GetSize();
		void			Release();
		DWORD			GetAccessTime();
		const char *	GetFileName();

		void			SetPlayTime(DWORD dwPlayTime);
		DWORD			GetPlayTime();

		void*			GetPCMData() const { return m_pPCMData; }
		ma_uint64		GetPCMFrameCount() const { return m_pcmFrameCount; }
		ma_format		GetFormat() const { return m_format; }
		ma_uint32		GetChannels() const { return m_channels; }
		ma_uint32		GetSampleRate() const { return m_sampleRate; }
		bool			IsLoaded() const { return m_bLoaded; }

	protected:
		bool			ReadFromDisk();
		void			Destroy();

	protected:
		char			m_filename[128];
		int				m_iRefCount;
		DWORD			m_dwAccessTime;
		DWORD			m_dwPlayTime;

		void*			m_pPCMData;
		ma_uint64		m_pcmFrameCount;
		ma_format		m_format;
		ma_uint32		m_channels;
		ma_uint32		m_sampleRate;

		bool			m_assigned;
		bool			m_bLoaded;
};

#endif
