#include "StdAfx.h"
#include "SoundInstance.h"

CSoundInstance2D::CSoundInstance2D() : m_pSoundData(NULL), m_bSoundInitialized(false)
{
}

CSoundInstance2D::~CSoundInstance2D()
{
	Destroy();
}

void CSoundInstance2D::Destroy()
{
	if (m_bSoundInitialized)
	{
		ma_sound_uninit(&m_sound);
		ma_audio_buffer_uninit(&m_audioBuffer);
		m_bSoundInitialized = false;
	}

	if (m_pSoundData)
	{
		m_pSoundData->Release();
		m_pSoundData = NULL;
	}
}

bool CSoundInstance2D::Initialize()
{
	return true;
}

bool CSoundInstance2D::SetSound(CSoundData * pSoundData)
{
	assert(pSoundData != NULL);
	if (!pSoundData)
		return false;

	if (m_bSoundInitialized)
	{
		ma_sound_uninit(&m_sound);
		ma_audio_buffer_uninit(&m_audioBuffer);
		m_bSoundInitialized = false;
	}

	if (m_pSoundData)
	{
		m_pSoundData->Release();
		m_pSoundData = NULL;
	}

	pSoundData->Get();
	if (!pSoundData->IsLoaded() || !pSoundData->GetPCMData())
	{
		pSoundData->Release();
		return false;
	}

	ma_audio_buffer_config config = ma_audio_buffer_config_init(
		pSoundData->GetFormat(),
		pSoundData->GetChannels(),
		pSoundData->GetPCMFrameCount(),
		pSoundData->GetPCMData(),
		NULL
	);
	config.sampleRate = pSoundData->GetSampleRate();

	ma_result result = ma_audio_buffer_init(&config, &m_audioBuffer);
	if (result != MA_SUCCESS)
	{
		pSoundData->Release();
		return false;
	}

	result = ma_sound_init_from_data_source(CSoundBase::GetEngine(), &m_audioBuffer, 0, NULL, &m_sound);
	if (result != MA_SUCCESS)
	{
		ma_audio_buffer_uninit(&m_audioBuffer);
		pSoundData->Release();
		return false;
	}

	ma_sound_set_spatialization_enabled(&m_sound, MA_FALSE);
	m_bSoundInitialized = true;
	m_pSoundData = pSoundData;
	return true;
}

bool CSoundInstance2D::IsDone() const
{
	if (!m_bSoundInitialized)
		return true;

	return ma_sound_at_end(&m_sound) == MA_TRUE;
}

void CSoundInstance2D::Play(int iLoopCount, DWORD dwPlayCycleTimeLimit) const
{
	if (!m_bSoundInitialized)
		return;

	ma_sound_set_looping(&m_sound, (iLoopCount == 0 || iLoopCount == -1) ? MA_TRUE : MA_FALSE);
	ma_audio_buffer_seek_to_pcm_frame(&m_audioBuffer, 0);
	ma_sound_start(&m_sound);
}

void CSoundInstance2D::Pause() const
{
	if (m_bSoundInitialized)
		ma_sound_stop(&m_sound);
}

void CSoundInstance2D::Resume() const
{
	if (m_bSoundInitialized)
		ma_sound_start(&m_sound);
}

void CSoundInstance2D::Stop()
{
	if (m_bSoundInitialized)
	{
		ma_sound_stop(&m_sound);
		ma_audio_buffer_seek_to_pcm_frame(&m_audioBuffer, 0);
	}
}

void CSoundInstance2D::GetVolume(float& rfVolume) const
{
	if (m_bSoundInitialized)
		rfVolume = ma_sound_get_volume(&m_sound);
	else
		rfVolume = 0.0f;
}

void CSoundInstance2D::SetVolume(float volume) const
{
	if (m_bSoundInitialized)
	{
		if (volume < 0.0f) volume = 0.0f;
		else if (volume > 1.0f) volume = 1.0f;
		ma_sound_set_volume(&m_sound, volume);
	}
}

void CSoundInstance2D::SetPosition(float x, float y, float z) const
{
}

void CSoundInstance2D::SetOrientation(float x_face, float y_face, float z_face, 
									  float x_normal, float y_normal, float z_normal) const
{
}

void CSoundInstance2D::SetVelocity(float fx, float fy, float fz, float fMagnitude) const
{
}
