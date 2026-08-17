#include "audio.hpp"

#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "paths.hpp"

namespace
{
    Mix_Music *theme = nullptr;
    Mix_Chunk *menu_move = nullptr;
    Mix_Chunk *rotate = nullptr;
    bool mixer_open = false;

    const int kMenuMoveChannel = 0;
    const int kRotateChannel = 1;
}

bool audio::initialize ()
{
    if (Mix_Init(MIX_INIT_MP3) == 0)
    {
        std::cerr << "SDL_mixer MP3 support unavailable: " << Mix_GetError() << '\n';
    }

    // 22.05 kHz / 2048-sample buffer: light enough for a Pi Zero 2 W, still fine on desktop.
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        std::cerr << "Could not open audio device: " << Mix_GetError() << '\n';
        return false;
    }
    mixer_open = true;
    Mix_AllocateChannels(8);
    Mix_VolumeMusic(96);
    Mix_Volume(-1, 128);

    theme = Mix_LoadMUS(assetPath("main_theme.mp3").c_str());
    if (theme == nullptr)
    {
        std::cerr << "Could not load main_theme.mp3: " << Mix_GetError() << '\n';
    }

    menu_move = Mix_LoadWAV(assetPath("menu_move.wav").c_str());
    if (menu_move == nullptr)
    {
        std::cerr << "Could not load menu_move.wav: " << Mix_GetError() << '\n';
    }

    rotate = Mix_LoadWAV(assetPath("rotate.mp3").c_str());
    if (rotate == nullptr)
    {
        std::cerr << "Could not load rotate.mp3: " << Mix_GetError() << '\n';
    }

    return true;
}

void audio::shutdown ()
{
    Mix_HaltMusic();
    Mix_HaltChannel(-1);

    if (theme != nullptr)
    {
        Mix_FreeMusic(theme);
        theme = nullptr;
    }
    if (menu_move != nullptr)
    {
        Mix_FreeChunk(menu_move);
        menu_move = nullptr;
    }
    if (rotate != nullptr)
    {
        Mix_FreeChunk(rotate);
        rotate = nullptr;
    }

    if (mixer_open)
    {
        Mix_CloseAudio();
        mixer_open = false;
    }
    Mix_Quit();
}

void audio::playTheme ()
{
    if (theme == nullptr || Mix_PlayingMusic())
    {
        return;
    }
    if (Mix_PlayMusic(theme, -1) < 0)
    {
        std::cerr << "Could not play main theme: " << Mix_GetError() << '\n';
    }
}

void audio::playMenuMove ()
{
    if (menu_move != nullptr)
    {
        Mix_PlayChannel(kMenuMoveChannel, menu_move, 0);
    }
}

void audio::playRotate ()
{
    if (rotate != nullptr)
    {
        Mix_PlayChannel(kRotateChannel, rotate, 0);
    }
}
