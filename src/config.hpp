#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <SDL2/SDL.h>

namespace config
{
    // Window title
    const char window_title[] = "Pixeltetris";

    // Native resolution of the Grant Sinclair handheld (4" IPS, 254ppi, 60fps).
    // All layout constants are in this logical space; SDL letterboxes on other displays.
    const int logical_window_width = 720;
    const int logical_window_height = 720;
    const int target_fps = 60;

    // Board class
    const int width_to_playfield = 220;                             // in pixels
    const int height_to_playfield = 80;                             // in pixels
    const int block_size = 28;                                      // dest size in pixels (sprites are 16px)
    const int block_src_size = 16;                                  // tetromino sprite clip size
    const int playfield_width = 10;                                 // in blocks
    const int true_playfield_height = 20;                           // in blocks
    const int playfield_height = 22;                                // The playfield+2 rows above for spawning
    const int frame_width = 6;                                      // Frame padding inside the 8px source sprite
    const int frame_sprite_size = 8;                                // Size of each sprite clip for the playfield frame
    const int board_height = 2;                                     // distance from bottom where the playfield begins
    const int matrix_blocks = 5;                                    // matrix that holds tetromino info; in blocks

    // GameState class
    const int next_box_x = 540;
    const int next_box_y = 130;
    const int hold_box_x = 40;
    const int hold_box_y = 130;

    // Menu layout — full-width rows readable on a 4" 254ppi panel
    const int menu_button_x = 36;
    const int menu_button_w = 648;
    const int menu_button_h = 112;
    const int menu_button_gap = 20;

    // Gamepad / DAS (Delayed Auto Shift)
    const int axis_deadzone = 8000;
    const int axis_press_deadzone = 16000;
    const int axis_release_deadzone = 8000;
    const int trigger_threshold = 16000;
    const int das_delay_ms = 180;
    const int das_repeat_ms = 50;
    const int menu_repeat_delay_ms = 220;
    const int menu_repeat_ms = 140;

    // Constants relating to Tetris
    const int wait_time = 1000;                                     // Time in milliseconds
    extern bool ghost_piece_enabled;

    // Visuals
    const SDL_Color default_text_color = {0x00, 0x00, 0x00, 0xFF};
    const Uint8 background_r_light = 0xF9;
    const Uint8 background_g_light = 0xE6;
    const Uint8 background_b_light = 0xCF;
    const Uint8 transparency_alpha = 100;
    const Uint8 button_fill_r = 0xE4;
    const Uint8 button_fill_g = 0xC8;
    const Uint8 button_fill_b = 0xA8;
    const Uint8 button_fill_selected_r = 0xFF;
    const Uint8 button_fill_selected_g = 0xFF;
    const Uint8 button_fill_selected_b = 0xFF;
}

#endif // CONFIG_HPP
