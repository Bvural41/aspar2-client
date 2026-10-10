#include "StdAfx.h"
#include "SoundInstance.h"

CSoundInstanceStream::CSoundInstanceStream() : m_bSoundInitialized(false)
{
}

CSoundInstanceStream::~CSoundInstanceStream()
{
	Destroy();
}

void CSoundInstanceStream::Destroy()
{
	if (m_bSoundInitialized)
	{
		ma_sound_uninit(&m_sound);
		m_bSoundInitialized = false;
	}
}

bool CSoundInstanceStream::Initialize()
{
	return true;
}

bool CSoundInstanceStream::SetStream(const char* filename)
{
	Destroy();

	if (!filename || !filename[0])
		return false;

	ma_result result = ma_sound_init_from_file(
		CSoundBase::GetEngine(),
		filename,
		MA_SOUND_FLAG_STREAM,
		NULL,
		NULL,
		&m_sound
	);

	if (result != MA_SUCCESS)
	{
		std::string altPath = "BGM/";
		altPath += filename;
		result = ma_sound_init_from_file(
			CSoundBase::GetEngine(),
			altPath.c_str(),
			MA_SOUND_FLAG_STREAM,
			NULL,
			NULL,
			&m_sound
		);
	}

	if (result != MA_SUCCESS)
	{
		TraceError("CSoundInstanceStream::SetStream - Failed to open stream for %s (error: %d)", filename, result);
		return false;
	}

	ma_sound_set_spatialization_enabled(&m_sound, MA_FALSE);
	m_bSoundInitialized = true;
	return true;
}

bool CSoundInstanceStream::IsDone() const
{
	if (!m_bSoundInitialized)
		return true;

	return ma_sound_at_end(&m_sound) == MA_TRUE;
}

bool CSoundInstanceStream::IsData() const
{
	return m_bSoundInitialized;
}

void CSoundInstanceStream::Play(int count, DWORD dwPlayCycleTimeLimit) const
{
	if (!m_bSoundInitialized)
		return;

	ma_sound_set_looping(&m_sound, (count == 0 || count == -1) ? MA_TRUE : MA_FALSE);
	ma_sound_seek_to_pcm_frame(&m_sound, 0);
	ma_sound_start(&m_sound);
}

void CSoundInstanceStream::Pause() const
{
	if (m_bSoundInitialized)
		ma_sound_stop(&m_sound);
}

void CSoundInstanceStream::Resume() const
{
	if (m_bSoundInitialized)
		ma_sound_start(&m_sound);
}

void CSoundInstanceStream::Stop()
{
	if (m_bSoundInitialized)
	{
		ma_sound_stop(&m_sound);
		ma_sound_seek_to_pcm_frame(&m_sound, 0);
	}
}

void CSoundInstanceStream::GetVolume(float& rfVolume) const
{
	if (m_bSoundInitialized)
		rfVolume = ma_sound_get_volume(&m_sound);
	else
		rfVolume = 0.0f;
}

void CSoundInstanceStream::SetVolume(float volume) const
{
	if (m_bSoundInitialized)
	{
		if (volume < 0.0f) volume = 0.0f;
		else if (volume > 1.0f) volume = 1.0f;
		ma_sound_set_volume(&m_sound, volume);
	}


}

bool CSoundInstanceStream::SetSound(CSoundData* pSound)
{
	return true;
}

void CSoundInstanceStream::SetPosition(float x, float y, float z) const
{
}

void CSoundInstanceStream::SetOrientation(float x_face, float y_face, float z_face, 
										  float x_normal, float y_normal, float z_normal) const
{
}

void CSoundInstanceStream::SetVelocity(float fx, float fy, float fz, float fMagnitude) const
{
}
