#include "button.hpp"

#include "config.hpp"
#include "game.hpp"
#include "renderer.hpp"

Button::Button (const std::string &label, void (*callback) (), int posX, int posY, int w, int h)
{
    callbackFunction = callback;
    position_x = posX;
    position_y = posY;
    width = w;
    height = h;
    label_texture = new Texture;
    TTF_Font *font = Game::getInstance()->mRenderer->mediumFont;
    label_texture->loadFromText(label, font, config::default_text_color);
}

Button::~Button ()
{
    delete label_texture;
    label_texture = nullptr;
}

void Button::draw (bool highlighted)
{
    SDL_Renderer *renderer = Game::getInstance()->mRenderer->mSDLRenderer;
    SDL_Rect rect = {position_x, position_y, width, height};

    if (highlighted)
    {
        SDL_SetRenderDrawColor(renderer, config::button_fill_selected_r,
            config::button_fill_selected_g, config::button_fill_selected_b, 0xFF);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, config::button_fill_r,
            config::button_fill_g, config::button_fill_b, 0xFF);
    }
    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    const int border = highlighted ? 4 : 2;
    for (int i = 0; i < border; i++)
    {
        SDL_Rect outline = {rect.x + i, rect.y + i, rect.w - 2 * i, rect.h - 2 * i};
        SDL_RenderDrawRect(renderer, &outline);
    }

    if (label_texture != nullptr)
    {
        label_texture->renderCentered(position_x + width / 2, position_y + height / 2);
    }
}

int Button::getX ()
{
    return position_x;
}

int Button::getY ()
{
    return position_y;
}

int Button::getWidth ()
{
    return width;
}

int Button::getHeight ()
{
    return height;
}
