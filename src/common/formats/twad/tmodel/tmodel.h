

#pragma once
#include "../shared.h"

namespace tungsten::twad
{
    constexpr char      TMODEL_MAGIC[] = "TUNGSTEN TWAD MODEL";
    constexpr size_t    TMODEL_MAGIC_SIZE = sizeof(TMODEL_MAGIC) - 1;
    constexpr uint64_t  TMODEL_VERSION = 1;
}