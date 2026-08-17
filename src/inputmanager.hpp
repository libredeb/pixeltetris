#ifndef INPUTMANAGER_HPP
#define INPUTMANAGER_HPP

#include <SDL2/SDL.h>

// A simple enumeration to hold all possible actions when playing.
enum class Action
{
    stay_idle, back, move_up, move_down, move_left, move_right, select, drop, rotate, hold, pause
};

enum class RepeatPolicy
{
    menu, gameplay
};

// Manages keyboard and SDL GameController input (Vinyl gamecontrollerdb mappings).
class InputManager
{
public:
    static const int max_controllers = 4;

    InputManager ();
    ~InputManager ();
    void clearEventQueue ();
    Action getAction ();
    bool isGameExiting ();
    bool pollAction ();
    void setExit ();
    void setRepeatPolicy (RepeatPolicy policy);

private:
    bool quit_game;
    Action action;
    RepeatPolicy repeat_policy;

    enum DirSource : Uint8
    {
        DIR_SRC_KEYBOARD = 1 << 0,
        DIR_SRC_DPAD     = 1 << 1,
        DIR_SRC_STICK    = 1 << 2
    };

    SDL_GameController *controllers[max_controllers];

    bool trigger_left_down;
    bool trigger_right_down;
    int axis_x_dir;
    int axis_y_dir;
    Uint8 held_mask_up;
    Uint8 held_mask_down;
    Uint8 held_mask_left;
    Uint8 held_mask_right;
    Uint32 direction_stamp;
    bool first_repeat_done;

    void loadControllerMappings ();
    void openAllControllers ();
    void openController (int device_index);
    void closeController (SDL_JoystickID instance_id);
    void closeAllControllers ();

    Action translateEvent (const SDL_Event &event);
    Action actionFromControllerButton (SDL_GameControllerButton button) const;
    Action setDirectionSource (Action direction, Uint8 source, bool down);
    Action applyStickAxis (int *axis_dir, Action negative, Action positive, Sint16 value);
    Action repeatAction ();
    bool isRepeatable (Action direction) const;
    bool isHeld (Action direction) const;
    Uint8 *maskFor (Action direction);
};

#endif // INPUTMANAGER_HPP
