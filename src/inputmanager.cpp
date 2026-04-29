#include "inputmanager.hpp"

#include <iostream>

/*
 * ====================================
 * Public methods start here
 * ====================================
 */

InputManager::InputManager ()
{
    quit_game  = false;
    action     = Action::stay_idle;
    mDownHeld  = false;
    mPrevAxisX = 0;
    mPrevAxisY = 0;
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

bool InputManager::isDownHeld () const
{
    return mDownHeld;
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
                    mDownHeld = true;
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
        else if (event.type == SDL_KEYUP)
        {
            if (event.key.keysym.sym == SDLK_DOWN)
            {
                mDownHeld = false;
            }
            action = Action::stay_idle;
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
                    mDownHeld = true;
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

                // SDL_A = botón físico B (abajo) en layout Nintendo/Switch
                case SDL_CONTROLLER_BUTTON_A:
                {
                    action = Action::move_down;
                    mDownHeld = true;
                    break;
                }

                // SDL_B = botón físico A (derecha) en layout Nintendo/Switch
                // → rota en juego, confirma en menús (via fallthrough en menu states)
                case SDL_CONTROLLER_BUTTON_B:
                {
                    action = Action::rotate;
                    break;
                }

                // START = pausa durante el juego
                case SDL_CONTROLLER_BUTTON_START:
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
        else if (event.type == SDL_CONTROLLERBUTTONUP)
        {
            if (event.cbutton.button == SDL_CONTROLLER_BUTTON_A ||
                event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN)
            {
                mDownHeld = false;
            }
            action = Action::stay_idle;
        }
        // D-pad enviado como hat (8BitDo en modo DirectInput u otros)
        else if (event.type == SDL_JOYHATMOTION)
        {
            if (mController != nullptr && event.jhat.hat == 0 &&
                event.jhat.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(mController)))
            {
                Uint8 hat = event.jhat.value;
                if (hat == SDL_HAT_CENTERED)
                {
                    mDownHeld = false;
                    action = Action::stay_idle;
                }
                else if (hat & SDL_HAT_UP)
                {
                    mDownHeld = false;
                    action = Action::move_up;
                }
                else if (hat & SDL_HAT_DOWN)
                {
                    mDownHeld = true;
                    action = Action::move_down;
                }
                else if (hat & SDL_HAT_LEFT)
                {
                    action = Action::move_left;
                }
                else if (hat & SDL_HAT_RIGHT)
                {
                    action = Action::move_right;
                }
                else
                {
                    action = Action::stay_idle;
                }
            }
            else
            {
                action = Action::stay_idle;
            }
        }
        // D-pad o stick analógico enviado como eje (fallback)
        // Solo dispara al cruzar el umbral (una acción por movimiento)
        else if (event.type == SDL_CONTROLLERAXISMOTION)
        {
            const Sint16 DEAD_ZONE = 10000;
            Sint16 val = event.caxis.value;
            if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX)
            {
                if      (mPrevAxisX > -DEAD_ZONE && val <= -DEAD_ZONE) action = Action::move_left;
                else if (mPrevAxisX < DEAD_ZONE  && val >= DEAD_ZONE)  action = Action::move_right;
                else                                                    action = Action::stay_idle;
                mPrevAxisX = val;
            }
            else if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY)
            {
                if (mPrevAxisY > -DEAD_ZONE && val <= -DEAD_ZONE)
                {
                    mDownHeld = false;
                    action = Action::move_up;
                }
                else if (mPrevAxisY < DEAD_ZONE && val >= DEAD_ZONE)
                {
                    mDownHeld = true;
                    action = Action::move_down;
                }
                else
                {
                    if (val > -DEAD_ZONE && val < DEAD_ZONE) mDownHeld = false;
                    action = Action::stay_idle;
                }
                mPrevAxisY = val;
            }
            else
            {
                action = Action::stay_idle;
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
                    mDownHeld = false;
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
