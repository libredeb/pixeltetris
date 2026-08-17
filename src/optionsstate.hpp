#ifndef OPTIONSSTATE_HPP
#define OPTIONSSTATE_HPP

#include "button.hpp"
#include "inputmanager.hpp"
#include "state.hpp"

class OptionsState: public State
{
    enum class SettingChange {left, right};
public:
    OptionsState (InputManager *inputmanager);
    ~OptionsState ();
    void initialize () override;
    void exit () override;

    void run () override;
    void update () override;
    void draw () override;

private:
    Button *OKButton;
    Texture *title_text;
    Texture *ghost_block_setting_text;
    Texture *ghost_value_text;
    Texture *hint_text;
    int index;

    void refreshGhostLabel ();
    void changeGhostBlock (SettingChange s);
};

#endif // OPTIONSSTATE_HPP
