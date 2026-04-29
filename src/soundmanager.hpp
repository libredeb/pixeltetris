#ifndef SOUNDMANAGER_HPP
#define SOUNDMANAGER_HPP

#include <SDL2/SDL_mixer.h>

class SoundManager
{
public:
    static SoundManager* getInstance();

    bool initialize();
    void exit();

    void playMusic();
    void playButtonPress();
    void playMove();
    void playRotate();

private:
    SoundManager();
    static SoundManager* mInstance;

    Mix_Music*  mMusic;
    Mix_Chunk*  mButtonPress;
    Mix_Chunk*  mMove;
    Mix_Chunk*  mRotate;
};

#endif // SOUNDMANAGER_HPP
