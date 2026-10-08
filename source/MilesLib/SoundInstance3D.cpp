#include "stdafx.h"
#include "SoundInstance.h"
#include "../eterBase/Timer.h"

CSoundInstance3D::CSoundInstance3D() : m_pSoundData(NULL), m_bSoundInitialized(false)
{
}

CSoundInstance3D::~CSoundInstance3D()
{
	Destroy();
}

void CSoundInstance3D::Destroy()
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

bool CSoundInstance3D::Initialize()
{
	return true;
}

bool CSoundInstance3D::SetSound(CSoundData* pSoundData)
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

	ma_sound_set_spatialization_enabled(&m_sound, MA_TRUE);
	ma_sound_set_attenuation_model(&m_sound, ma_attenuation_model_inverse);
	ma_sound_set_min_distance(&m_sound, 1.0f);
	ma_sound_set_max_distance(&m_sound, 50.0f);
	ma_sound_set_rolloff(&m_sound, 1.0f);
	ma_sound_set_position(&m_sound, 0.0f, 0.0f, 0.0f);

	m_bSoundInitialized = true;
	m_pSoundData = pSoundData;
	return true;
}

bool CSoundInstance3D::IsDone() const
{
	if (!m_bSoundInitialized)
		return true;

	return ma_sound_at_end(&m_sound) == MA_TRUE;
}

void CSoundInstance3D::Play(int iLoopCount, DWORD dwPlayCycleTimeLimit) const
{
	if (!m_bSoundInitialized || !m_pSoundData)
		return;

	DWORD dwCurTime = ELTimer_GetMSec();

	if (dwCurTime - m_pSoundData->GetPlayTime() < dwPlayCycleTimeLimit)
		return;

	m_pSoundData->SetPlayTime(dwCurTime);

	ma_sound_set_looping(&m_sound, (iLoopCount == 0 || iLoopCount == -1) ? MA_TRUE : MA_FALSE);
	ma_audio_buffer_seek_to_pcm_frame(&m_audioBuffer, 0);
	ma_sound_start(&m_sound);
}

void CSoundInstance3D::Pause() const
{
	if (m_bSoundInitialized)
		ma_sound_stop(&m_sound);
}

void CSoundInstance3D::Resume() const
{
	if (m_bSoundInitialized)
		ma_sound_start(&m_sound);
}

void CSoundInstance3D::Stop()
{
	if (m_bSoundInitialized)
	{
		ma_sound_stop(&m_sound);
		ma_audio_buffer_seek_to_pcm_frame(&m_audioBuffer, 0);
	}
}

void CSoundInstance3D::GetVolume(float& rfVolume) const
{
	if (m_bSoundInitialized)
		rfVolume = ma_sound_get_volume(&m_sound);
	else
		rfVolume = 0.0f;
}

void CSoundInstance3D::SetVolume(float volume) const
{
	if (m_bSoundInitialized)
	{
		volume = max(0.0f, min(1.0f, volume));
		ma_sound_set_volume(&m_sound, volume);
	}
}

void CSoundInstance3D::SetPosition(float x, float y, float z) const
{
	if (m_bSoundInitialized)
	{
		ma_sound_set_position(&m_sound, x, y, -z);
	}
}

void CSoundInstance3D::SetOrientation(float x_face, float y_face, float z_face, 
									  float x_normal, float y_normal, float z_normal) const
{
	if (m_bSoundInitialized)
	{
		ma_sound_set_direction(&m_sound, x_face, y_face, -z_face);
	}
}

void CSoundInstance3D::SetVelocity(float fDistanceX, float fDistanceY, float fDistanceZ, float fNagnitude) const
{
	if (m_bSoundInitialized)
	{
		ma_sound_set_velocity(&m_sound, fDistanceX, fDistanceY, -fDistanceZ);
	}
}

void CSoundInstance3D::UpdatePosition(float fElapsedTime)
{
}
