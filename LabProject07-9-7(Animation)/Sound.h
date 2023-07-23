#pragma once



enum class Sounds : char
{
    GUN, EXP, WALK, BAD, CLEAR
    ,CLICK, DARK, ROAR, FIRE, GROWL
    ,COUNT
};

enum class DragonSounds : char
{
    ROAR, FIRE, GROWL
    , COUNT
};

enum class PlayerSounds : char
{
    WALK
    , COUNT
};

class CSound {
private:
    static FMOD_SYSTEM* SoundSystem;

    FMOD_RESULT       result;

    FMOD_SOUND* m_sound;
    FMOD_CHANNEL* m_channel;

    float m_volume;
    FMOD_BOOL m_bool;
public:
    CSound(const char* path, bool loop, float volume = 1.0f);
    ~CSound();

    static int Init();
    static int Release();
    int play();
    int pause();
    int resume();
    int stop();
    int Update();
};