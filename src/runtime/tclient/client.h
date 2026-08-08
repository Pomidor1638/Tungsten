//
// Created by UBER_USER on 22.12.2025.
//

#pragma once
#include <cstdint>

namespace tungsten::client
{
    bool init ();
    bool frame(uint64_t delta_us);
    void quit ();
}
