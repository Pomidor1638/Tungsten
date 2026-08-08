
#pragma once

#include "host_local.h"

#include "../sys/sys.h"

#include "window/window.h"


namespace tungsten::host
{

    namespace
    {
        bool initialized = false;

        struct window_params
        {
            char title[32];

            int width;
            int height;

            int x_pos = 0;
            int y_pos = 0;

            bool fullscreen = false;
        };

        window_params win_params{};

        bool load_window_params()
        {

            sys::printf("\t\tload_window_params()");

            win_params = window_params
            {
                .title  = "Tungsten",
                .width  = 800,
                .height = 600,
                .x_pos  = -1, // means default pos
                .y_pos  = -1, // same
                .fullscreen = false
            };

            //sys::printf("%s can't load params", title_failure);

            sys::printf(title_ok);

            return true;
        }
    }



    bool window_init()
    {
        sys::printf("\twindow_init()\n");
        
        if (!load_window_params()) return false;
        //if (!window::init(win_params.title, win_params.x_pos, win_params.y_pos, win_params.width, win_params.height, 0)) return false;

        return initialized = true;
    }

    void window_quit()
    {
        sys::printf("\twindow_quit()");
        if (initialized)
        {
            sys::printf(title_ok);
        }
        else
        {
            sys::printf(title_skip);
        }
    }

}