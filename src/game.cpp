#include "game.hpp"

#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "button.hpp"
#include "config.hpp"
#include "gamestate.hpp"
#include "menustate.hpp"
#include "optionsstate.hpp"
#include "paths.hpp"
#include "pausedstate.hpp"
#include "state.hpp"

Game *Game::getInstance()
{
    if (mInstance == nullptr)
    {
        mInstance = new Game;
    }
    return mInstance;
}

bool Game::detectHandheldDisplay ()
{
    SDL_DisplayMode mode;
    if (SDL_GetDesktopDisplayMode(0, &mode) != 0)
    {
        return false;
    }
    // Grant Sinclair Gamer Card: 720x720. Also treat other small square panels as handheld.
    const bool square = (mode.w == mode.h);
    const bool small = (mode.w <= 800 && mode.h <= 800);
    return square && small;
}

bool Game::initialize()
{
    bool success = true;

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
#if SDL_VERSION_ATLEAST(2, 0, 12)
    SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "0");
#endif

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) < 0)
    {
        std::cerr << "Could not initialize SDL! SDL_Error: " << SDL_GetError() << '\n';
        success = false;
    }
    else
    {
        if (!mWindowedForced)
        {
            mWindowed = !detectHandheldDisplay();
        }

        Uint32 window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
        if (!mWindowed)
        {
            window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }

        mWindow = SDL_CreateWindow(config::window_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            config::logical_window_width, config::logical_window_height, window_flags);

        if (mWindow == nullptr)
        {
            std::cerr << "Could not create window! SDL_Error: " << SDL_GetError() << '\n';
            success = false;
        }
        else
        {
            if (IMG_Init(IMG_INIT_PNG) == 0)
            {
                std::cerr << "Could not initialize SDL_image! SDL_image error: " << IMG_GetError() << '\n';
                success = false;
            }
            if (TTF_Init() == -1)
            {
                std::cerr << "Could not initialize SDL_ttf! SDL_ttf error: " << TTF_GetError() << '\n';
                success = false;
            }
            SDL_Surface *icon = IMG_Load(dataPath("icon.png").c_str());
            if (icon != nullptr)
            {
                SDL_SetWindowIcon(mWindow, icon);
                SDL_FreeSurface(icon);
            }
        }
    }
    if (!success)
    {
        return false;
    }

    mRenderer = new Renderer;
    mRenderer->initialize(mWindow);
    if (mRenderer->mSDLRenderer == nullptr)
    {
        return false;
    }

    mManager = new InputManager;

    mMainMenuState = new MenuState(mManager);
    mMainMenuState->initialize();
    pushState(mMainMenuState);
    mFrameStart = SDL_GetTicks();
    return success;
}

void Game::exit ()
{
    mStates.clear();
    delete mPlayState;
    delete mOptionsState;
    delete mPausedState;
    delete mMainMenuState;
    mMainMenuState = nullptr;
    mPlayState = nullptr;
    mOptionsState = nullptr;
    mPausedState = nullptr;

    delete mManager;
    mManager = nullptr;

    delete mRenderer;
    mRenderer = nullptr;

    SDL_DestroyWindow(mWindow);
    mWindow = nullptr;

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void Game::run ()
{
    mFrameStart = SDL_GetTicks();
    if (!mStates.empty())
    {
        mStates.back()->run();
    }

    const Uint32 frame_budget = 1000 / config::target_fps;
    const Uint32 elapsed = SDL_GetTicks() - mFrameStart;
    if (elapsed < frame_budget)
    {
        SDL_Delay(frame_budget - elapsed);
    }
}

void Game::popState ()
{
    mStates.pop_back();
}

void Game::pushState (State *state)
{
    mStates.push_back(state);
}

void Game::changeState (State *state)
{
    popState();
    pushState(state);
}

void Game::pushNewGame ()
{
    delete Game::getInstance()->mPlayState;
    Game::getInstance()->mPlayState = new GameState(Game::getInstance()->mManager);
    Game::getInstance()->mPlayState->initialize();
    Game::getInstance()->pushState(Game::getInstance()->mPlayState);
}

void Game::pushOptions ()
{
    delete Game::getInstance()->mOptionsState;
    Game::getInstance()->mOptionsState = new OptionsState(Game::getInstance()->mManager);
    Game::getInstance()->mOptionsState->initialize();
    Game::getInstance()->pushState(Game::getInstance()->mOptionsState);
}

void Game::pushPaused ()
{
    delete Game::getInstance()->mPausedState;
    Game::getInstance()->mPausedState = new PausedState (Game::getInstance()->mManager);
    Game::getInstance()->mPausedState->initialize();
    Game::getInstance()->pushState(Game::getInstance()->mPausedState);
}

void Game::goBack ()
{
    Game::getInstance()->popState();
}

void Game::goDoubleBack ()
{
    Game::getInstance()->popState();
    Game::getInstance()->popState();
}

bool Game::isGameExiting ()
{
    if (mStates.empty())
    {
        return true;
    }
    else
    {
        return mStates.back()->nextStateID == STATE_EXIT;
    }
}

void Game::setWindowed (bool windowed, bool forced)
{
    mWindowed = windowed;
    mWindowedForced = forced;
}

Game *Game::mInstance = 0;

Game::Game ()
{
    mWindow = nullptr;
    mRenderer = nullptr;
    mManager = nullptr;
    mPlayState = nullptr;
    mMainMenuState = nullptr;
    mOptionsState = nullptr;
    mPausedState = nullptr;
    mWindowed = true;
    mWindowedForced = false;
    mFrameStart = 0;
}
