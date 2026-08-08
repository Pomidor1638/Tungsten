#pragma once

#include <cstdint>
//#include "../../sys/sys_event.h"

namespace tungsten::sys
{
    struct sys_event;
}

namespace tungsten::window
{
    
    /*
        window_*.cpp
        where '*' - is an implementation name
    */

    bool init(const char* title, int x, int y, int width, int height, uint32_t flags);
    void quit();
    

    void process_event(const sys::sys_event& event);

    void set_size(int  width, int  height);
    void get_size(int& width, int& height);

    void set_title(const char* title);

    void set_relative_mode(bool relative);
    bool get_relative_mode();

    void set_fullscreen(bool fullscreen);
    bool get_fullscreen();

    void* get_handle();
}