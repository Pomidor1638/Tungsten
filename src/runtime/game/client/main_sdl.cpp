
#include <SDL2/SDL_main.h>

#include "runtime/common/sys/sys.h"
#include "runtime/common/host/host.h"

using namespace tungsten;

int main(int argc, char* argv[])
{
    if (!sys::init(argc, argv))
    {
        return EXIT_FAILURE;
    }

    if (!host::init(argc, argv))
    {
        sys::quit();
        return EXIT_FAILURE;
    }

    uint64_t prev = sys::time_us();

    while (host::is_running())
    {
        uint64_t now      = sys::time_us();
        uint64_t delta_us = now - prev;
        prev   = now;

        if (!host::frame(delta_us))
            break;
    }

    host::quit();
    sys::quit();

    return EXIT_SUCCESS;
}
