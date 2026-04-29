#include "inputmanager.hpp"

#include <iostream>

/*
 * ====================================
 * Public methods start here
 * ====================================
 */

InputManager::InputManager ()
{
    quit_game = false;
    action = Action::stay_idle;
    mController = nullptr;
    for (int i = 0; i < SDL_NumJoysticks(); i++)
    {
        if (SDL_IsGameController(i))
        {
            mController = SDL_GameControllerOpen(i);
            if (mController != nullptr)
            {
                std::cout << "Gamepad connected: " << SDL_GameControllerName(mController) << '\n';
                break;
            }
        }
    }
}

InputManager::~InputManager ()
{
    if (mController != nullptr)
    {
        SDL_GameControllerClose(mController);
        mController = nullptr;
    }
}

// Clears all events in the event queue
void InputManager::clearEventQueue ()
{
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0)
    {
        
    }
}

Action InputManager::getAction ()
{
    return action;
}

bool InputManager::isGameExiting ()
{
    return quit_game;
}

// Polls from keyboard/gamepad and returns the corresponding action; Returns false if all events have been polled
bool InputManager::pollAction ()
{
    SDL_Event event;
    if (SDL_PollEvent(&event) != 0)
    {
        if (event.type == SDL_QUIT)
        {
            quit_game = true;
        }
        else if (event.type == SDL_KEYDOWN)
        {
            switch (event.key.keysym.sym)
            {
                case SDLK_UP:
                {
                    action = Action::move_up;
                    break;
                }

                case SDLK_DOWN:
                {
                    action = Action::move_down;
                    break;
                }

                case SDLK_LEFT:
                {
                    action = Action::move_left;
                    break;
                }

                case SDLK_RIGHT:
                {
                    action = Action::move_right;
                    break;
                }

                case SDLK_RETURN:
                {
                    action = Action::select;
                    break;
                }

                case SDLK_SPACE:
                {
                    action = Action::drop;
                    break;
                }

                case SDLK_q: case SDLK_ESCAPE:
                {
                    action = Action::back;
                    break;
                }

                case SDLK_c: case SDLK_LSHIFT:
                {
                    action = Action::hold;
                    break;
                }

                case SDLK_x:
                {
                    action = Action::rotate;
                    break;
                }

                case SDLK_p:
                {
                    action = Action::pause;
                    break;
                }

                default:
                {
                    action = Action::stay_idle;
                    break;
                }
            }
        }
        else if (event.type == SDL_CONTROLLERBUTTONDOWN)
        {
            switch (event.cbutton.button)
            {
                case SDL_CONTROLLER_BUTTON_DPAD_UP:
                {
                    action = Action::move_up;
                    break;
                }

                case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                {
                    action = Action::move_down;
                    break;
                }

                case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                {
                    action = Action::move_left;
                    break;
                }

                case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                {
                    action = Action::move_right;
                    break;
                }

                // A = rotate in game / confirm in menus (handled via Action::rotate fallthrough)
                case SDL_CONTROLLER_BUTTON_A:
                {
                    action = Action::rotate;
                    break;
                }

                // B = soft drop (same as d-pad down)
                case SDL_CONTROLLER_BUTTON_B:
                {
                    action = Action::move_down;
                    break;
                }

                // START = pause during gameplay
                case SDL_CONTROLLER_BUTTON_START:
                {
                    action = Action::pause;
                    break;
                }

                // BACK/SELECT = go back / exit
                case SDL_CONTROLLER_BUTTON_BACK:
                {
                    action = Action::back;
                    break;
                }

                default:
                {
                    action = Action::stay_idle;
                    break;
                }
            }
        }
        else if (event.type == SDL_CONTROLLERDEVICEADDED)
        {
            if (mController == nullptr && SDL_IsGameController(event.cdevice.which))
            {
                mController = SDL_GameControllerOpen(event.cdevice.which);
                if (mController != nullptr)
                {
                    std::cout << "Gamepad connected: " << SDL_GameControllerName(mController) << '\n';
                }
            }
            action = Action::stay_idle;
        }
        else if (event.type == SDL_CONTROLLERDEVICEREMOVED)
        {
            if (mController != nullptr)
            {
                SDL_JoystickID openID = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(mController));
                if (openID == event.cdevice.which)
                {
                    std::cout << "Gamepad disconnected.\n";
                    SDL_GameControllerClose(mController);
                    mController = nullptr;
                    // Try to reopen another available controller
                    for (int i = 0; i < SDL_NumJoysticks(); i++)
                    {
                        if (SDL_IsGameController(i))
                        {
                            mController = SDL_GameControllerOpen(i);
                            if (mController != nullptr)
                            {
                                std::cout << "Gamepad reconnected: " << SDL_GameControllerName(mController) << '\n';
                                break;
                            }
                        }
                    }
                }
            }
            action = Action::stay_idle;
        }
        else
        {
            action = Action::stay_idle;
        }
    }
    else
    {
        action = Action::stay_idle;
        return false;
    }
    return true;
}

void InputManager::setExit ()
{
    quit_game = true;
}
