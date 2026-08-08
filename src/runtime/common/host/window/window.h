#pragma once

#include <cstdint>

namespace tungsten::window
{
    
    bool init(const char* title, int x, int y, int width, int height, uint32_t flags);
    void quit();
    
    extern union SDL_Event;
    void process_event(const SDL_Event& event);

    void set_size(int  width, int  height);
    void get_size(int& width, int& height);

    void set_title(const char* title);

    void set_relative_mode(bool relative);
    bool get_relative_mode();

    void set_fullscreen(bool fullscreen);
    bool get_fullscreen();

    void* get_handle();
}