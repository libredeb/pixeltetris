#include "menustate.hpp"

#include "config.hpp"
#include "inputmanager.hpp"
#include "renderer.hpp"
#include "state.hpp"

MenuState::MenuState (InputManager *manager) : State (manager)
{
    title_text = nullptr;
    hint_text = nullptr;
    index = 0;
}

MenuState::~MenuState ()
{
    exit();
}

void MenuState::initialize ()
{
    index = 0;
    title_text = new Texture();
    title_text->loadFromText("Pixeltetris", Game::getInstance()->mRenderer->bigFont, config::default_text_color);
    hint_text = new Texture();
    TTF_Font *hint_font = Game::getInstance()->mRenderer->smallFont;
    if (hint_font == nullptr)
    {
        hint_font = Game::getInstance()->mRenderer->mediumFont;
    }
    hint_text->loadFromText("A Select   B Back   START Pause", hint_font, config::default_text_color);

    const int x = config::menu_button_x;
    const int w = config::menu_button_w;
    const int h = config::menu_button_h;
    const int start_y = 200;
    mButtons.push_back(new Button("PLAY", &Game::pushNewGame, x, start_y, w, h));
    mButtons.push_back(new Button("OPTIONS", &Game::pushOptions, x, start_y + h + config::menu_button_gap, w, h));
    mButtons.push_back(new Button("EXIT", &Game::goBack, x, start_y + 2 * (h + config::menu_button_gap), w, h));
}

void MenuState::exit ()
{
    for (auto i : mButtons)
    {
        delete i;
    }
    mButtons.clear();
    delete title_text;
    title_text = nullptr;
    delete hint_text;
    hint_text = nullptr;
}

void MenuState::run ()
{
    mInputManager->setRepeatPolicy(RepeatPolicy::menu);
    update();
    draw();
}

void MenuState::update ()
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
            case Action::select:
            case Action::pause:
            {
                mButtons[index]->callbackFunction();
                break;
            }

            case Action::back:
            {
                nextStateID = STATE_EXIT;
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
                if (index < static_cast<int>(mButtons.size()) - 1)
                {
                    ++index;
                }
                break;
            }

            default:
                break;
        }
    }
}

void MenuState::draw ()
{
    Game::getInstance()->mRenderer->clearScreen();
    if (title_text != nullptr)
    {
        title_text->renderCentered(config::logical_window_width / 2, 90);
    }
    for (size_t i = 0; i < mButtons.size(); i++)
    {
        mButtons[i]->draw(static_cast<int>(i) == index);
    }
    if (hint_text != nullptr)
    {
        hint_text->renderCentered(config::logical_window_width / 2, config::logical_window_height - 48);
    }
    Game::getInstance()->mRenderer->updateScreen();
}

void MenuState::addButton (Button *button)
{
    mButtons.push_back(button);
}
