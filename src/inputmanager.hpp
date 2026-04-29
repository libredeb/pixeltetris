#ifndef INPUTMANAGER_HPP
#define INPUTMANAGER_HPP

#include <SDL2/SDL.h>

// A simple enumeration to hold all possible actions when playing.
enum class Action
{
    stay_idle, back, move_up, move_down, move_left, move_right, select, drop, rotate, hold, pause
};

// Manages all input polled from keyboard and gamepad
class InputManager
{
public:
    InputManager ();
    ~InputManager ();
    void clearEventQueue ();
    Action getAction ();
    bool isGameExiting ();
    bool pollAction ();
    void setExit ();
    bool isDownHeld () const;
private:
    bool quit_game;
    Action action;
    bool mDownHeld;
    SDL_GameController *mController;
    Sint16 mPrevAxisX;
    Sint16 mPrevAxisY;
};

#endif // INPUTMANAGER_HPP