#pragma once

class CSound {
private:
    static FMOD_SYSTEM* g_sound_system;

    FMOD_SOUND* m_sound;
    FMOD_CHANNEL* m_channel;

    float m_volume;
    FMOD_BOOL m_bool;
public:
    CSound(const char* path, bool loop);
    ~CSound();

    static int Init();
    static int Release();

    int play();
    int pause();
    int resume();
    int stop();
    int volumeUp();
    int volumeDown();

    int Update();
};