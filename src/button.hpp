#ifndef BUTTON_HPP
#define BUTTON_HPP

#include <string>

#include "texture.hpp"

// Full-width (or sized) text row, sized for a 4" 720x720 handheld panel.
class Button
{
public:
    Button (const std::string &label, void (*callback) (), int posX, int posY, int w, int h);
    ~Button ();
    void draw (bool highlighted = false);
    void (*callbackFunction) ();

    int getX ();
    int getY ();
    int getWidth ();
    int getHeight ();

private:
    int position_x;
    int position_y;
    int width;
    int height;
    Texture *label_texture;
};

#endif // BUTTON_HPP
