
#pragma once
#include "../shared.h"

namespace tungsten::twad
{
    constexpr char      TPROG_MAGIC[] = "TUNGSTEN TWAD PROG";
    constexpr size_t    TPROG_MAGIC_SIZE = sizeof(TPROG_MAGIC) - 1;
    constexpr uint64_t  TPROG_VERSION = 1;
}