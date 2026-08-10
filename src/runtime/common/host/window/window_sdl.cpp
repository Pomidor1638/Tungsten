

#include <SDL2/SDL.h>

#include "window.h"

#include "../../host/host.h"
#include "../log/log.h"


namespace tungsten::window
{
    namespace
    {
        bool window_initialized = false;
        SDL_Window* window_handle = nullptr;
    }

    bool init(const char* title, int x, int y, int width, int height, uint32_t flags)
    {
        log::printf("\twindow::init()");

        if (window_initialized)
        {
            return true;
        }




    }

    void quit();


    void process_event(const sys::sys_event& event);

    void set_size(int  width, int  height);
    void get_size(int& width, int& height);

    void set_title(const char* title);

    void set_relative_mode(bool relative);
    bool get_relative_mode();

    void set_fullscreen(bool fullscreen);
    bool get_fullscreen();

    void* get_handle()
    {
        return window_handle;
    }
}
