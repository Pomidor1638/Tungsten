#pragma once

#include <cstdint>

namespace tungsten::input
{
    bool init();

    // main event process

    // void process_event(const SDL_Event& e);
    
    void begin_process_event();
    void end_process_event();
    
    // keyboard
    //bool is_key_pressed      (const SDL_Keycode& key);
    //bool is_key_just_pressed (const SDL_Keycode& key);
    //bool is_key_just_released(const SDL_Keycode& key);

    // mouse
    void set_mouse_pos(int x, int y);
    void get_mouse_pos(int& x, int& y);
    
    float get_mouse_wheel_delta();
    void get_mouse_wheel_delta(int& x, int& y);
    
    void showCursor(bool show);
    void setCursorRelativeMode(bool relative);
}
