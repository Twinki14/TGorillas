#ifndef GORILLAS_HPP_INCLUDED
#define GORILLAS_HPP_INCLUDED

#include <cstdint>

namespace Gorillas
{
    enum STYLE_FLAGS
    {
        MELEE_FLAG      = 1 << 0,
        RANGED_FLAG     = 1 << 1,
        MAGIC_FLAG      = 1 << 2,
        BOULDER_FLAG    = 1 << 3
    };

    enum STATE { };

    std::int32_t GetEquippedStyle();

    void OnGameTick();
}

#endif // GORILLAS_HPP_INCLUDED