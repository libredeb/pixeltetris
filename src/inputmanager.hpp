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

    SDL_GameController *controllers[max_controllers];

    bool held_up;
    bool held_down;
    bool held_left;
    bool held_right;
    bool trigger_left_down;
    bool trigger_right_down;
    int axis_x_dir;
    int axis_y_dir;
    Uint32 direction_stamp;
    bool first_repeat_done;

    void loadControllerMappings ();
    void openAllControllers ();
    void openController (int device_index);
    void closeController (SDL_JoystickID instance_id);
    void closeAllControllers ();

    Action translateEvent (const SDL_Event &event);
    Action actionFromControllerButton (SDL_GameControllerButton button) const;
    void setHeldDirection (Action direction, bool down);
    Action edgeDirection (Action direction, bool down);
    Action repeatAction ();
    bool isRepeatable (Action direction) const;
};

#endif // INPUTMANAGER_HPP
