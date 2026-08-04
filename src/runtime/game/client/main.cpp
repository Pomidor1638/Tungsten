
#include <cstdio>
#include <cstdlib>


#include "../common/sys/sys.h"
#include "../common/host/host.h"

using namespace tungsten;

int main(int argc, char** argv)
{
    host::params params{};
    params.argc = argc;
    params.argv = argv;
    params.permanent_size = 32 * 1024 * 1024ull;
    params.    level_size = 64 * 1024 * 1024ull;
    params.    frame_size = 16 * 1024 * 1024ull;
    params.  scratch_size = 16 * 1024 * 1024ull;

    if (!sys::init(argc, argv))
        return EXIT_FAILURE;

    if (!host::init(params))
    {
        sys::quit();
        return EXIT_FAILURE;
    }

    uint64_t prev = sys::time_us();

    while (host::is_running())
    {
        uint64_t now = sys::time_us();
        uint64_t delta_us = now - prev;
        prev = now;

        if (!host::frame(delta_us))
            break;
    }

    host::quit();
    sys::quit();

    return EXIT_SUCCESS;
}
