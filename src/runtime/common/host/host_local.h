
#pragma once

#include <cstdint>

namespace tungsten::host
{
    // host.cpp

    extern const char title_failure[];
    extern const char title_ok[];
    extern const char title_skip[];

    void error(const char* fmt, ...);

    // host_args.cpp
    struct params
    {
        int      argc           = 0;
        char**   argv           = nullptr;
                 
        size_t   permanent_size = 0;
        size_t       level_size = 0;
        size_t       frame_size = 0;
        size_t     scratch_size = 0;

        bool     dedicated      = false;
        uint32_t thread_count   = 1;
    };

    extern params init_params;


    bool parse_args();

    // host_log.cpp
    bool log_init();
    void log_quit();

    // host_console.cpp
    bool console_init();
    void console_quit();

    // host_zone.cpp
    bool zone_init();
    void zone_quit();

    // host_window.cpp
    bool window_init();
    void window_quit();

    // host_renderer.cpp
    bool renderer_init();
    void renderer_quit();

    // host_input.cpp
    bool input_init();
    void input_quit();

    // host_client.cpp
    bool client_init();
    void client_quit();


    // host_events.cpp
    bool process_events();

}