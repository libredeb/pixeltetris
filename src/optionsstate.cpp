#include "optionsstate.hpp"

#include "audio.hpp"
#include "config.hpp"
#include "game.hpp"

OptionsState::OptionsState (InputManager *manager) : State (manager)
{
    OKButton = nullptr;
    title_text = nullptr;
    ghost_block_setting_text = nullptr;
    ghost_value_text = nullptr;
    hint_text = nullptr;
    index = 0;
}

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

    ghost_value_text = new Texture();
    refreshGhostLabel();

    hint_text = new Texture();
    TTF_Font *hint_font = Game::getInstance()->mRenderer->smallFont;
    if (hint_font == nullptr)
    {
        hint_font = Game::getInstance()->mRenderer->mediumFont;
    }
    hint_text->loadFromText("Left/Right to toggle    A OK", hint_font, config::default_text_color);

    OKButton = new Button("OK", &Game::goBack, config::menu_button_x,
        config::logical_window_height - config::menu_button_h - 80,
        config::menu_button_w, config::menu_button_h);
}

void OptionsState::exit ()
{
    delete title_text;
    title_text = nullptr;
    delete ghost_block_setting_text;
    ghost_block_setting_text = nullptr;
    delete ghost_value_text;
    ghost_value_text = nullptr;
    delete hint_text;
    hint_text = nullptr;
    delete OKButton;
    OKButton = nullptr;
}

void OptionsState::run ()
{
    mInputManager->setRepeatPolicy(RepeatPolicy::menu);
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
            case Action::select:
            case Action::pause:
            {
                if (index == 1)
                {
                    Game::getInstance()->popState();
                }
                else
                {
                    changeGhostBlock(SettingChange::right);
                }
                break;
            }
            case Action::move_up:
            {
                if (index > 0)
                {
                    --index;
                    audio::playMenuMove();
                }
                break;
            }
            case Action::move_down:
            {
                if (index < 1)
                {
                    ++index;
                    audio::playMenuMove();
                }
                break;
            }
            case Action::move_left:
            {
                if (index == 0)
                {
                    changeGhostBlock(SettingChange::left);
                }
                break;
            }
            case Action::move_right:
            {
                if (index == 0)
                {
                    changeGhostBlock(SettingChange::right);
                }
                break;
            }
            default:
                break;
        }
    }
}

void OptionsState::draw ()
{
    Game::getInstance()->mRenderer->clearScreen();

    title_text->renderCentered(config::logical_window_width / 2, 80);

    const int row_y = 220;
    const int row_h = 140;
    SDL_Rect row = {config::menu_button_x, row_y, config::menu_button_w, row_h};
    if (index == 0)
    {
        SDL_SetRenderDrawColor(Game::getInstance()->mRenderer->mSDLRenderer,
            config::button_fill_selected_r, config::button_fill_selected_g,
            config::button_fill_selected_b, 0xFF);
    }
    else
    {
        SDL_SetRenderDrawColor(Game::getInstance()->mRenderer->mSDLRenderer,
            config::button_fill_r, config::button_fill_g, config::button_fill_b, 0xFF);
    }
    SDL_RenderFillRect(Game::getInstance()->mRenderer->mSDLRenderer, &row);
    SDL_SetRenderDrawColor(Game::getInstance()->mRenderer->mSDLRenderer, 0, 0, 0, 0xFF);
    const int border = (index == 0) ? 4 : 2;
    for (int i = 0; i < border; i++)
    {
        SDL_Rect outline = {row.x + i, row.y + i, row.w - 2 * i, row.h - 2 * i};
        SDL_RenderDrawRect(Game::getInstance()->mRenderer->mSDLRenderer, &outline);
    }

    ghost_block_setting_text->renderCentered(config::logical_window_width / 2, row_y + 40);
    ghost_value_text->renderCentered(config::logical_window_width / 2, row_y + 100);

    OKButton->draw(index == 1);
    if (hint_text != nullptr)
    {
        hint_text->renderCentered(config::logical_window_width / 2, config::logical_window_height - 36);
    }

    Game::getInstance()->mRenderer->updateScreen();
}

void OptionsState::refreshGhostLabel ()
{
    const char *label = config::ghost_piece_enabled ? "<  ON  >" : "<  OFF  >";
    ghost_value_text->loadFromText(label, Game::getInstance()->mRenderer->mediumFont, config::default_text_color);
}

void OptionsState::changeGhostBlock (SettingChange s)
{
    if ((s == SettingChange::left && config::ghost_piece_enabled)
        || (s == SettingChange::right && !config::ghost_piece_enabled))
    {
        config::ghost_piece_enabled = !config::ghost_piece_enabled;
        refreshGhostLabel();
    }
}
