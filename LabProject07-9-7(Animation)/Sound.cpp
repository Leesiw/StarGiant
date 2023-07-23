#include "stdafx.h"
#include "Sound.h"


FMOD_SYSTEM* CSound::SoundSystem;

CSound::CSound(const char* path, bool loop, float volume)
{
    if (loop) {
        FMOD_System_CreateSound(SoundSystem, path, FMOD_LOOP_NORMAL, 0, &m_sound);
    }
    else {
        FMOD_System_CreateSound(SoundSystem, path, FMOD_DEFAULT, 0, &m_sound);
    }

    m_channel = nullptr;
    m_volume = volume;
}

CSound::~CSound()
{
    FMOD_Sound_Release(m_sound);
}

int CSound::Init()
{
    FMOD_System_Create(&SoundSystem, FMOD_VERSION);
    FMOD_System_Init(SoundSystem, 32, FMOD_INIT_NORMAL, NULL);
    return 0;
}

int CSound::Release()
{
    FMOD_System_Close(SoundSystem);
    FMOD_System_Release(SoundSystem);
    return 0;
}

int CSound::play()
{
    FMOD_System_PlaySound(SoundSystem, m_sound, NULL, false, &m_channel);
    return 0;
}

int CSound::pause() {
    FMOD_Channel_SetPaused(m_channel, true);

    return 0;
}

int CSound::stop() {
    FMOD_Channel_Stop(m_channel);

    return 0;
}

int CSound::Update()
{
    FMOD_Channel_IsPlaying(m_channel, &m_bool);

    if (m_bool) {
        FMOD_System_Update(SoundSystem);
    }

    return 0;
}
