#include "soundmanager.hpp"

#include <iostream>

SoundManager* SoundManager::mInstance = nullptr;

SoundManager::SoundManager()
    : mMusic(nullptr), mButtonPress(nullptr), mMove(nullptr), mRotate(nullptr),
      mLineComplete(nullptr)
{
}

SoundManager* SoundManager::getInstance()
{
    if (mInstance == nullptr)
    {
        mInstance = new SoundManager;
    }
    return mInstance;
}

bool SoundManager::initialize()
{
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        std::cerr << "SDL_mixer could not initialize! Mix_Error: " << Mix_GetError() << '\n';
        return false;
    }

    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    mMusic        = Mix_LoadMUS("../../assets/theme-song.ogg");
    mButtonPress  = Mix_LoadWAV("../../assets/button-press.ogg");
    mMove         = Mix_LoadWAV("../../assets/move.ogg");
    mRotate       = Mix_LoadWAV("../../assets/rotate.ogg");
    mLineComplete = Mix_LoadWAV("../../assets/line-complete.ogg");
    #else
    mMusic        = Mix_LoadMUS("../assets/theme-song.ogg");
    mButtonPress  = Mix_LoadWAV("../assets/button-press.ogg");
    mMove         = Mix_LoadWAV("../assets/move.ogg");
    mRotate       = Mix_LoadWAV("../assets/rotate.ogg");
    mLineComplete = Mix_LoadWAV("../assets/line-complete.ogg");
    #endif

    if (!mMusic)         std::cerr << "Failed to load theme-song.ogg: "    << Mix_GetError() << '\n';
    if (!mButtonPress)   std::cerr << "Failed to load button-press.ogg: "  << Mix_GetError() << '\n';
    if (!mMove)          std::cerr << "Failed to load move.ogg: "          << Mix_GetError() << '\n';
    if (!mRotate)        std::cerr << "Failed to load rotate.ogg: "        << Mix_GetError() << '\n';
    if (!mLineComplete)  std::cerr << "Failed to load line-complete.ogg: " << Mix_GetError() << '\n';

    return true;
}

void SoundManager::exit()
{
    Mix_HaltMusic();
    Mix_HaltChannel(-1);

    if (mMusic)         { Mix_FreeMusic(mMusic);        mMusic = nullptr; }
    if (mButtonPress)   { Mix_FreeChunk(mButtonPress);  mButtonPress = nullptr; }
    if (mMove)          { Mix_FreeChunk(mMove);          mMove = nullptr; }
    if (mRotate)        { Mix_FreeChunk(mRotate);        mRotate = nullptr; }
    if (mLineComplete)  { Mix_FreeChunk(mLineComplete);  mLineComplete = nullptr; }

    Mix_CloseAudio();
}

void SoundManager::playMusic()
{
    if (mMusic && !Mix_PlayingMusic())
    {
        Mix_PlayMusic(mMusic, -1);
    }
}

void SoundManager::playButtonPress()
{
    if (mButtonPress)
    {
        Mix_PlayChannel(-1, mButtonPress, 0);
    }
}

void SoundManager::playMove()
{
    if (mMove)
    {
        Mix_PlayChannel(-1, mMove, 0);
    }
}

void SoundManager::playRotate()
{
    if (mRotate)
    {
        Mix_PlayChannel(-1, mRotate, 0);
    }
}

void SoundManager::playLineComplete()
{
    if (mLineComplete)
    {
        Mix_PlayChannel(-1, mLineComplete, 0);
    }
}
