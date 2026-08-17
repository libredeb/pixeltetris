#include "pausedstate.hpp"

#include "audio.hpp"
#include "config.hpp"
#include "game.hpp"

PausedState::PausedState (InputManager *manager) : State (manager)
{
    title_text = nullptr;
    index = 0;
}

PausedState::~PausedState ()
{
    exit();
}

void PausedState::initialize ()
{
    index = 0;
    title_text = new Texture();
    title_text->loadFromText("Paused", Game::getInstance()->mRenderer->bigFont, config::default_text_color);

    const int x = config::menu_button_x;
    const int w = config::menu_button_w;
    const int h = config::menu_button_h;
    const int start_y = 260;
    mButtons.push_back(new Button("RESUME", &Game::goBack, x, start_y, w, h));
    mButtons.push_back(new Button("QUIT", &Game::goDoubleBack, x, start_y + h + config::menu_button_gap, w, h));
}

void PausedState::exit ()
{
    for (auto i : mButtons)
    {
        delete i;
    }
    mButtons.clear();
    delete title_text;
    title_text = nullptr;
}

void PausedState::run ()
{
    mInputManager->setRepeatPolicy(RepeatPolicy::menu);
    update();
    draw();
}

void PausedState::update ()
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
                Game::getInstance()->goBack();
                break;
            }

            case Action::move_up:
            case Action::move_left:
            {
                if (index > 0)
                {
                    --index;
                    audio::playMenuMove();
                }
                break;
            }

            case Action::move_down:
            case Action::move_right:
            {
                if (index < static_cast<int>(mButtons.size()) - 1)
                {
                    ++index;
                    audio::playMenuMove();
                }
                break;
            }

            default:
                break;
        }
    }
}

void PausedState::draw ()
{
    Game::getInstance()->mRenderer->clearScreen();
    if (title_text != nullptr)
    {
        title_text->renderCentered(config::logical_window_width / 2, 140);
    }
    for (size_t i = 0; i < mButtons.size(); i++)
    {
        mButtons[i]->draw(static_cast<int>(i) == index);
    }
    Game::getInstance()->mRenderer->updateScreen();
}
