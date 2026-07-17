#pragma once

#include <cstdint>
#include <concepts>

namespace tungsten::platform::endian 
{
    // --- Target Architecture Configuration ---
    // Toggle these macros depending on your target machine architecture
    #define TUNGSTEN_LITTLE_ENDIAN
    //#define TUNGSTEN_BIG_ENDIAN

    #if defined(TUNGSTEN_LITTLE_ENDIAN) && defined(TUNGSTEN_BIG_ENDIAN)
        static_assert(false, "Conflicting target endianness definitions!");
    #endif

    // --- Core Bit-Shifting Swap Engine ---
    
    // Pure platform-independent byte inversion loop using bitwise masks.
    // Must remain header-only to guarantee maximum inline expansion by Clang/GCC.
    template <std::integral T>
    [[nodiscard]] constexpr T swap(T value) noexcept 
    {
        if constexpr (sizeof(T) == 1) 
        {
            return value;
        } 
        else if constexpr (sizeof(T) == 2) 
        {
            auto val = static_cast<uint16_t>(value);
            return static_cast<T>(((val & 0x00ffu) << 8) | ((val & 0xff00u) >> 8));
        } 
        else if constexpr (sizeof(T) == 4) 
        {
            auto val = static_cast<uint32_t>(value);
            return static_cast<T>(
                ((val & 0x000000ffu) << 24) | 
                ((val & 0x0000ff00u) <<  8) |
                ((val & 0x00ff0000u) >>  8) |
                ((val & 0xff000000u) >> 24)
            );
        } 
        else if constexpr (sizeof(T) == 8) 
        {
            auto val = static_cast<uint64_t>(value);
            return static_cast<T>(
                ((val & 0x00000000000000ffull) << 56) |
                ((val & 0x000000000000ff00ull) << 40) |
                ((val & 0x0000000000ff0000ull) << 24) |
                ((val & 0x00000000ff000000ull) <<  8) |
                ((val & 0x000000ff00000000ull) >>  8) |
                ((val & 0x0000ff0000000000ull) >> 24) |
                ((val & 0x00ff000000000000ull) >> 40) |
                ((val & 0xff00000000000000ull) >> 56)
            );
        }
    }

    // --- Basic Endian Host Mappings ---

    template <std::integral T>
    [[nodiscard]] constexpr T to_little(T value) noexcept 
    {
    #if defined(TUNGSTEN_LITTLE_ENDIAN)
        return value; 
    #else
        return swap(value);
    #endif
    }

    template <std::integral T>
    [[nodiscard]] constexpr T to_big(T value) noexcept 
    {
    #if defined(TUNGSTEN_LITTLE_ENDIAN)
        return swap(value);
    #else
        return value;
    #endif
    }
}
