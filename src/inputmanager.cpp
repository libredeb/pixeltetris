#include "inputmanager.hpp"

#include <iostream>

#include "config.hpp"
#include "paths.hpp"

// Built-in fallbacks for devices that SDL does not ship (Gamer Card / THEGamepad / 8BitDo).
// The full Linux database is still loaded from gamecontrollerdb.txt when present.
static const char *kFallbackMappings[] = {
    "03000000412300003680000001010000,Arduino Leonardo,a:b0,b:b1,x:b3,y:b4,back:b10,start:b11,leftshoulder:b5,rightshoulder:b6,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftx:a0,lefty:a1,platform:Linux,",
    "03000000591c00002600000010010000,THEGamepad,a:b2,b:b1,back:b6,leftshoulder:b4,leftx:a0,lefty:a1,rightshoulder:b5,start:b7,x:b3,y:b0,platform:Linux,",
    "05000000a00500003232000001000000,8BitDo Zero,a:b0,b:b1,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b3,y:b4,platform:Linux,",
    "05000000a00500003232000008010000,8BitDo Zero,a:b0,b:b1,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b3,y:b4,platform:Linux,",
    "03000000c82d00001890000011010000,8BitDo Zero 2,a:b1,b:b0,back:b10,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,rightshoulder:b7,start:b11,x:b4,y:b3,platform:Linux,",
    "05000000c82d00003032000000010000,8BitDo Zero 2,a:b1,b:b0,back:b10,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b6,leftx:a0,lefty:a1,rightshoulder:b7,rightx:a2,righty:a3,start:b11,x:b4,y:b3,platform:Linux,",
    nullptr
};

InputManager::InputManager ()
{
    quit_game = false;
    action = Action::stay_idle;
    repeat_policy = RepeatPolicy::menu;
    held_up = held_down = held_left = held_right = false;
    trigger_left_down = trigger_right_down = false;
    axis_x_dir = 0;
    axis_y_dir = 0;
    direction_stamp = 0;
    first_repeat_done = false;

    for (int i = 0; i < max_controllers; i++)
    {
        controllers[i] = nullptr;
    }

    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
#if SDL_VERSION_ATLEAST(2, 0, 12)
    SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "0");
#endif
    SDL_GameControllerEventState(SDL_ENABLE);

    loadControllerMappings();
    openAllControllers();
}

InputManager::~InputManager ()
{
    closeAllControllers();
}

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

bool InputManager::pollAction ()
{
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0)
    {
        Action translated = translateEvent(event);
        if (quit_game)
        {
            action = Action::stay_idle;
            return true;
        }
        if (translated != Action::stay_idle)
        {
            action = translated;
            return true;
        }
    }

    Action repeated = repeatAction();
    if (repeated != Action::stay_idle)
    {
        action = repeated;
        return true;
    }

    action = Action::stay_idle;
    return false;
}

void InputManager::setExit ()
{
    quit_game = true;
}

void InputManager::setRepeatPolicy (RepeatPolicy policy)
{
    repeat_policy = policy;
}

void InputManager::loadControllerMappings ()
{
    const std::string db_path = dataPath("gamecontrollerdb.txt");
    int added = SDL_GameControllerAddMappingsFromFile(db_path.c_str());
    if (added >= 0)
    {
        std::cerr << "Loaded " << added << " game controller mappings from " << db_path << '\n';
    }
    else
    {
        std::cerr << "Could not load " << db_path << ": " << SDL_GetError() << '\n';
    }

    for (int i = 0; kFallbackMappings[i] != nullptr; i++)
    {
        SDL_GameControllerAddMapping(kFallbackMappings[i]);
    }
}

void InputManager::openAllControllers ()
{
    const int n = SDL_NumJoysticks();
    if (n < 1)
    {
        std::cerr << "No game controller detected\n";
        return;
    }
    for (int i = 0; i < n; i++)
    {
        openController(i);
    }
}

void InputManager::openController (int device_index)
{
    if (!SDL_IsGameController(device_index))
    {
        const char *name = SDL_JoystickNameForIndex(device_index);
        std::cerr << "Joystick " << device_index << " (" << (name ? name : "unknown")
                  << ") is not a mapped game controller\n";
        return;
    }

    for (int i = 0; i < max_controllers; i++)
    {
        if (controllers[i] != nullptr)
        {
            SDL_Joystick *js = SDL_GameControllerGetJoystick(controllers[i]);
            if (js != nullptr)
            {
#if SDL_VERSION_ATLEAST(2, 0, 6)
                if (SDL_JoystickInstanceID(js) == SDL_JoystickGetDeviceInstanceID(device_index))
                {
                    return;
                }
#endif
            }
        }
    }

    for (int i = 0; i < max_controllers; i++)
    {
        if (controllers[i] == nullptr)
        {
            controllers[i] = SDL_GameControllerOpen(device_index);
            if (controllers[i] == nullptr)
            {
                std::cerr << "Unable to open game controller: " << SDL_GetError() << '\n';
            }
            else
            {
                std::cerr << "Opened controller: " << SDL_GameControllerName(controllers[i]) << '\n';
            }
            return;
        }
    }
}

void InputManager::closeController (SDL_JoystickID instance_id)
{
    for (int i = 0; i < max_controllers; i++)
    {
        if (controllers[i] == nullptr)
        {
            continue;
        }
        SDL_Joystick *js = SDL_GameControllerGetJoystick(controllers[i]);
        if (js != nullptr && SDL_JoystickInstanceID(js) == instance_id)
        {
            std::cerr << "Closed controller: " << SDL_GameControllerName(controllers[i]) << '\n';
            SDL_GameControllerClose(controllers[i]);
            controllers[i] = nullptr;
        }
    }
}

void InputManager::closeAllControllers ()
{
    for (int i = 0; i < max_controllers; i++)
    {
        if (controllers[i] != nullptr)
        {
            SDL_GameControllerClose(controllers[i]);
            controllers[i] = nullptr;
        }
    }
}

Action InputManager::actionFromControllerButton (SDL_GameControllerButton button) const
{
    switch (button)
    {
        case SDL_CONTROLLER_BUTTON_DPAD_UP: return Action::move_up;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return Action::move_down;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return Action::move_left;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return Action::move_right;
        case SDL_CONTROLLER_BUTTON_A: return Action::select;
        case SDL_CONTROLLER_BUTTON_B: return Action::back;
        case SDL_CONTROLLER_BUTTON_X: return Action::hold;
        case SDL_CONTROLLER_BUTTON_Y: return Action::drop;
        case SDL_CONTROLLER_BUTTON_START: return Action::pause;
        case SDL_CONTROLLER_BUTTON_BACK: return Action::back;
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return Action::hold;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return Action::rotate;
        case SDL_CONTROLLER_BUTTON_GUIDE: return Action::pause;
        default: return Action::stay_idle;
    }
}

void InputManager::setHeldDirection (Action direction, bool down)
{
    bool *flag = nullptr;
    switch (direction)
    {
        case Action::move_up: flag = &held_up; break;
        case Action::move_down: flag = &held_down; break;
        case Action::move_left: flag = &held_left; break;
        case Action::move_right: flag = &held_right; break;
        default: return;
    }
    if (*flag == down)
    {
        return;
    }
    *flag = down;
    if (down)
    {
        direction_stamp = SDL_GetTicks();
        first_repeat_done = false;
    }
}

Action InputManager::edgeDirection (Action direction, bool down)
{
    bool already_held = false;
    switch (direction)
    {
        case Action::move_up: already_held = held_up; break;
        case Action::move_down: already_held = held_down; break;
        case Action::move_left: already_held = held_left; break;
        case Action::move_right: already_held = held_right; break;
        default: return down ? direction : Action::stay_idle;
    }
    setHeldDirection(direction, down);
    if (down && !already_held)
    {
        return direction;
    }
    return Action::stay_idle;
}

bool InputManager::isRepeatable (Action direction) const
{
    if (repeat_policy == RepeatPolicy::menu)
    {
        return direction == Action::move_up || direction == Action::move_down
            || direction == Action::move_left || direction == Action::move_right;
    }
    // In gameplay, DAS applies to horizontal movement and soft drop. Up is rotate.
    return direction == Action::move_down || direction == Action::move_left
        || direction == Action::move_right;
}

Action InputManager::repeatAction ()
{
    Action direction = Action::stay_idle;
    if (held_down) { direction = Action::move_down; }
    else if (held_left) { direction = Action::move_left; }
    else if (held_right) { direction = Action::move_right; }
    else if (held_up) { direction = Action::move_up; }

    if (direction == Action::stay_idle || !isRepeatable(direction))
    {
        return Action::stay_idle;
    }

    const Uint32 now = SDL_GetTicks();
    const Uint32 delay = (repeat_policy == RepeatPolicy::menu)
        ? (first_repeat_done ? config::menu_repeat_ms : config::menu_repeat_delay_ms)
        : (first_repeat_done ? config::das_repeat_ms : config::das_delay_ms);

    if (now - direction_stamp >= delay)
    {
        direction_stamp = now;
        first_repeat_done = true;
        return direction;
    }
    return Action::stay_idle;
}

Action InputManager::translateEvent (const SDL_Event &event)
{
    switch (event.type)
    {
        case SDL_QUIT:
        {
            quit_game = true;
            return Action::stay_idle;
        }

        case SDL_CONTROLLERDEVICEADDED:
        {
            openController(event.cdevice.which);
            return Action::stay_idle;
        }

        case SDL_CONTROLLERDEVICEREMOVED:
        {
            closeController(event.cdevice.which);
            return Action::stay_idle;
        }

        case SDL_KEYDOWN:
        {
            if (event.key.repeat)
            {
                return Action::stay_idle;
            }
            switch (event.key.keysym.sym)
            {
                case SDLK_UP: return edgeDirection(Action::move_up, true);
                case SDLK_DOWN: return edgeDirection(Action::move_down, true);
                case SDLK_LEFT: return edgeDirection(Action::move_left, true);
                case SDLK_RIGHT: return edgeDirection(Action::move_right, true);
                case SDLK_RETURN: return Action::select;
                case SDLK_SPACE: return Action::drop;
                case SDLK_q:
                case SDLK_ESCAPE: return Action::back;
                case SDLK_c:
                case SDLK_LSHIFT:
                case SDLK_RSHIFT: return Action::hold;
                case SDLK_x: return Action::rotate;
                case SDLK_p: return Action::pause;
                default: return Action::stay_idle;
            }
        }

        case SDL_KEYUP:
        {
            switch (event.key.keysym.sym)
            {
                case SDLK_UP: return edgeDirection(Action::move_up, false);
                case SDLK_DOWN: return edgeDirection(Action::move_down, false);
                case SDLK_LEFT: return edgeDirection(Action::move_left, false);
                case SDLK_RIGHT: return edgeDirection(Action::move_right, false);
                default: return Action::stay_idle;
            }
        }

        case SDL_CONTROLLERBUTTONDOWN:
        {
            Action mapped = actionFromControllerButton(
                static_cast<SDL_GameControllerButton>(event.cbutton.button));
            if (mapped == Action::move_up || mapped == Action::move_down
                || mapped == Action::move_left || mapped == Action::move_right)
            {
                return edgeDirection(mapped, true);
            }
            return mapped;
        }

        case SDL_CONTROLLERBUTTONUP:
        {
            Action mapped = actionFromControllerButton(
                static_cast<SDL_GameControllerButton>(event.cbutton.button));
            if (mapped == Action::move_up || mapped == Action::move_down
                || mapped == Action::move_left || mapped == Action::move_right)
            {
                return edgeDirection(mapped, false);
            }
            return Action::stay_idle;
        }

        case SDL_CONTROLLERAXISMOTION:
        {
            const Sint16 value = event.caxis.value;
            if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX
                || event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX)
            {
                int dir = 0;
                if (value < -config::axis_deadzone) { dir = -1; }
                else if (value > config::axis_deadzone) { dir = 1; }
                if (dir == axis_x_dir)
                {
                    return Action::stay_idle;
                }
                Action previous = (axis_x_dir < 0) ? Action::move_left
                    : (axis_x_dir > 0) ? Action::move_right : Action::stay_idle;
                if (previous != Action::stay_idle)
                {
                    edgeDirection(previous, false);
                }
                axis_x_dir = dir;
                if (dir < 0) { return edgeDirection(Action::move_left, true); }
                if (dir > 0) { return edgeDirection(Action::move_right, true); }
                return Action::stay_idle;
            }
            if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY
                || event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY)
            {
                int dir = 0;
                if (value < -config::axis_deadzone) { dir = -1; }
                else if (value > config::axis_deadzone) { dir = 1; }
                if (dir == axis_y_dir)
                {
                    return Action::stay_idle;
                }
                Action previous = (axis_y_dir < 0) ? Action::move_up
                    : (axis_y_dir > 0) ? Action::move_down : Action::stay_idle;
                if (previous != Action::stay_idle)
                {
                    edgeDirection(previous, false);
                }
                axis_y_dir = dir;
                if (dir < 0) { return edgeDirection(Action::move_up, true); }
                if (dir > 0) { return edgeDirection(Action::move_down, true); }
                return Action::stay_idle;
            }
            if (event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT)
            {
                const bool down = value > config::trigger_threshold;
                if (down == trigger_left_down)
                {
                    return Action::stay_idle;
                }
                trigger_left_down = down;
                return down ? Action::hold : Action::stay_idle;
            }
            if (event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
            {
                const bool down = value > config::trigger_threshold;
                if (down == trigger_right_down)
                {
                    return Action::stay_idle;
                }
                trigger_right_down = down;
                return down ? Action::rotate : Action::stay_idle;
            }
            return Action::stay_idle;
        }

        default:
            return Action::stay_idle;
    }
}
