#ifndef GORILLAS_HPP_INCLUDED
#define GORILLAS_HPP_INCLUDED

#include <cstdint>
#include <Game/Interfaces/GameTabs/Prayer.hpp>
#include <Game/Interactable/Player.hpp>
#include <memory>
#include "Types/Gorilla.hpp"

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
        MELEE_MOVE                  = 1 << 3,
        BOULDER                     = 1 << 4,
        EQUIP_MELEE                 = 1 << 5,
        EQUIP_RANGED                = 1 << 6,
        EQUIP_SPECIAL               = 1 << 7,
    };

    std::int32_t GetState();
    std::int32_t GetEquippedStyle();
    std::int32_t GetEquippedWeaponStyle();
    std::int32_t GetDefenseAgainst(std::int32_t Style);
    std::int32_t GetProtectedStyle();
    std::int32_t GetProtectedStyle(const Interactable::Player& P);

    std::vector<WorldArea> GetValidMovementAreas();
    std::vector<std::pair<bool, WorldArea>> GetValidMoveTiles();
    Tile GetMeleeMoveTile(double Distance);
    Tile GetMeleeBoulderMoveTile(double Distance);
    Tile GetRangedBoulderMoveTile(double Distance);
    bool IsAttacking();
    bool ShouldLeave();

    bool AdjustCamera();
    bool Attack(bool Force, bool Wait = false);
    bool SwitchPrayer(Prayer::PRAYERS Prayer);

    bool MeleeMove(std::int32_t& State, const std::shared_ptr<Gorilla>& CurrentGorilla);
    bool BoulderMove(std::int32_t& State, const std::shared_ptr<Gorilla>& CurrentGorilla);
    bool Prayers(std::int32_t& State, const std::shared_ptr<Gorilla>& CurrentGorilla);

    bool WalkTo();
    bool Fight();

    void OnGameTick();
}

#endif // GORILLAS_HPP_INCLUDED