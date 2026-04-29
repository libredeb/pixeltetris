#include "optionsstate.hpp"

#include "config.hpp"
#include "game.hpp"
#include "soundmanager.hpp"

/*
 * ====================================
 * Public methods start here
 * ====================================
 */

OptionsState::OptionsState (InputManager *manager) : State (manager) {}

OptionsState::~OptionsState ()
{
    exit();
}

void OptionsState::initialize ()
{
    index = 0;
    title_text = new Texture();
    title_text->loadFromText("Options", Game::getInstance()->mRenderer->bigFont, config::default_text_color);
    ghost_block_setting_text = new Texture();
    ghost_block_setting_text->loadFromText("Ghost Block", Game::getInstance()->mRenderer->mediumFont, config::default_text_color);

    texture_on_on = new Texture();
    texture_on_off = new Texture();
    texture_off_on = new Texture();
    texture_off_off = new Texture();

    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    texture_on_on->loadFromImage("../../assets/button-on-on.png");
    texture_on_off->loadFromImage("../../assets/button-on-off.png");
    texture_off_on->loadFromImage("../../assets/button-off-on.png");
    texture_off_off->loadFromImage("../../assets/button-off-off.png");
    OKButton = new Button("../../assets/button-ok.png", &Game::goBack, (config::logical_window_width - 80) / 2, 200);
    #else
    texture_on_on->loadFromImage("../assets/button-on-on.png");
    texture_on_off->loadFromImage("../assets/button-on-off.png");
    texture_off_on->loadFromImage("../assets/button-off-on.png");
    texture_off_off->loadFromImage("../assets/button-off-off.png");
    OKButton = new Button("../assets/button-ok.png", &Game::goBack, (config::logical_window_width - 80) / 2, 200);
    #endif 
}

void OptionsState::exit ()
{
    delete title_text;
    delete ghost_block_setting_text;
    delete texture_on_on;
    delete texture_on_off;
    delete texture_off_on;
    delete texture_off_off;
    delete OKButton;
}

void OptionsState::run ()
{
    update();
    draw();
}

void OptionsState::update ()
{
    while (mInputManager->pollAction() != 0)
    {
        if (mInputManager->isGameExiting())
        {
            nextStateID = STATE_EXIT;
            break;
        }
        switch (mInputManager->getAction())
        {
            case Action::back:
            {
                Game::getInstance()->popState();
                break;
            }
            case Action::rotate:
            case Action::select:
            {
                if (index == 1)
                {
                    SoundManager::getInstance()->playButtonPress();
                    Game::getInstance()->popState();
                }
                break;
            }
            case Action::move_up:
            {
                if (index > 0)
                {
                    --index;
                }
                break;
            }
            case Action::move_down:
            {
                if (index < 1)
                {
                    ++index;
                }
                break;
            }
            case Action::move_left:
            {
                if (index == 0)
                {
                    changeGhostBlock (SettingChange::left);
                }
                break;
            }
            case Action::move_right:
            {
                if (index == 0)
                {
                    changeGhostBlock (SettingChange::right);
                }
                break;
            }
        }
    }
}

void OptionsState::draw ()
{
    Game::getInstance()->mRenderer->clearScreen();

    title_text->renderCentered(config::logical_window_width/2, 50);
    ghost_block_setting_text->render(50, 120);

    if (config::ghost_piece_enabled)
    {
        texture_off_off->render(200, 128);
        texture_on_on->render(280, 128);
    }
    else
    {
        texture_off_on->render(200, 128);
        texture_on_off->render(280, 128);
    }
    OKButton->draw();

    if (index == 0)
    {
        int height = ghost_block_setting_text->getHeight();
        SDL_Rect highlight_box = {0, 120, config::logical_window_width, height+5};
        SDL_SetRenderDrawBlendMode (Game::getInstance()->mRenderer->mSDLRenderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor (Game::getInstance()->mRenderer->mSDLRenderer, 255, 255, 255, config::transparency_alpha-20);
        SDL_RenderFillRect(Game::getInstance()->mRenderer->mSDLRenderer, &highlight_box);
        SDL_SetRenderDrawBlendMode (Game::getInstance()->mRenderer->mSDLRenderer, SDL_BLENDMODE_NONE);
    }
    else
    {
        SDL_Rect highlight_box = {OKButton->getX(), OKButton->getY(), OKButton->getWidth(), OKButton->getHeight()};
        SDL_SetRenderDrawBlendMode (Game::getInstance()->mRenderer->mSDLRenderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor (Game::getInstance()->mRenderer->mSDLRenderer, 255, 255, 255, config::transparency_alpha-20);
        SDL_RenderFillRect(Game::getInstance()->mRenderer->mSDLRenderer, &highlight_box);
        SDL_SetRenderDrawBlendMode (Game::getInstance()->mRenderer->mSDLRenderer, SDL_BLENDMODE_NONE);
    }

    Game::getInstance()->mRenderer->updateScreen();
}

/*
 * ====================================
 * Private methods start here
 * ====================================
 */

// Turn the ghost block on or off
void OptionsState::changeGhostBlock (SettingChange s)
{
    if ( s == SettingChange::left && config::ghost_piece_enabled || s == SettingChange::right && !config::ghost_piece_enabled)
    {
        config::ghost_piece_enabled = !config::ghost_piece_enabled;
    }
}
