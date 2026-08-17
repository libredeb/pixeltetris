#include "renderer.hpp"

#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "config.hpp"
#include "paths.hpp"

void Renderer::initialize (SDL_Window *window)
{
    smallFont = nullptr;
    mediumFont = nullptr;
    bigFont = nullptr;

    const std::string munro = assetPath("munro.ttf");
    const std::string munro_small = assetPath("munro-small.ttf");

    // Sizes chosen for a 4" 720x720 / 254ppi panel (Vinyl uses ~38pt on the same screen).
    bigFont = TTF_OpenFont(munro.c_str(), 56);
    mediumFont = TTF_OpenFont(munro_small.c_str(), 42);
    smallFont = TTF_OpenFont(munro_small.c_str(), 28);
    if (mediumFont == nullptr)
    {
        mediumFont = TTF_OpenFont(munro.c_str(), 42);
    }
    if (smallFont == nullptr)
    {
        smallFont = TTF_OpenFont(munro.c_str(), 28);
    }
    if (mediumFont == nullptr || bigFont == nullptr)
    {
        std::cerr << "Could not load font from " << munro << " / " << munro_small
                  << "! SDL_ttf error: " << TTF_GetError() << '\n';
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
#if SDL_VERSION_ATLEAST(2, 0, 10)
    SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");
#endif

    mSDLRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (mSDLRenderer == nullptr)
    {
        std::cerr << "Accelerated VSync renderer not available: " << SDL_GetError() << '\n';
        mSDLRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    }
    if (mSDLRenderer == nullptr)
    {
        std::cerr << "Accelerated renderer not available, falling back to software: " << SDL_GetError() << '\n';
        mSDLRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (mSDLRenderer == nullptr)
    {
        std::cerr << "Could not create renderer! SDL_Error: " << SDL_GetError() << '\n';
        return;
    }

    SDL_RendererInfo info;
    if (SDL_GetRendererInfo(mSDLRenderer, &info) == 0)
    {
        std::cerr << "Using renderer: " << info.name << '\n';
    }

    SDL_RenderSetLogicalSize(mSDLRenderer, config::logical_window_width, config::logical_window_height);
    SDL_SetRenderDrawBlendMode(mSDLRenderer, SDL_BLENDMODE_BLEND);
    clearScreen();
}

Renderer::~Renderer ()
{
    if (smallFont != nullptr) { TTF_CloseFont(smallFont); smallFont = nullptr; }
    if (mediumFont != nullptr) { TTF_CloseFont(mediumFont); mediumFont = nullptr; }
    if (bigFont != nullptr) { TTF_CloseFont(bigFont); bigFont = nullptr; }
    SDL_DestroyRenderer(mSDLRenderer);
    mSDLRenderer = nullptr;
}

void Renderer::clearScreen ()
{
    SDL_SetRenderDrawColor(mSDLRenderer, config::background_r_light, config::background_g_light, config::background_b_light, 0xFF);
    SDL_RenderClear(mSDLRenderer);
}

void Renderer::renderTexture (Texture *texture, int x, int y)
{
    texture->renderCentered(x, y);
}

void Renderer::updateScreen ()
{
    SDL_RenderPresent(mSDLRenderer);
}
