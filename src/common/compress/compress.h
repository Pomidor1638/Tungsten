#pragma once

namespace tungsten::compress 
{
    int   compress(size_t size, const void* src, void* dst);
    int decompress(size_t size, const void* src, void* dst);
}