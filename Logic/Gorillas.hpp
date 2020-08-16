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

    enum STATE_FLAGS
    {
        NONE = 0,
        SWITCH_PRAYER_MELEE         = 1 << 0,
        SWITCH_PRAYER_RANGED        = 1 << 1,
        SWITCH_PRAYER_MAGIC         = 1 << 2,
        SINGLE_SWITCH_PRAYER        = 1 << 3,
        MELEE_MOVE                  = 1 << 4,
        BOULDER                     = 1 << 5,
        EQUIP_MELEE                 = 1 << 6,
        EQUIP_RANGED                = 1 << 7,
        EQUIP_SPECIAL               = 1 << 8,
    };

    std::int32_t GetState();
    std::int32_t GetEquippedStyle();
    std::int32_t GetDefenseAgainst(std::int32_t Style);
    std::int32_t GetProtectedStyle();
    std::int32_t GetProtectedStyle(const Interactable::Player& P);

    bool WalkTo();
    bool Fight();

    void OnGameTick();
}

#endif // GORILLAS_HPP_INCLUDED