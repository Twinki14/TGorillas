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
    std::string GetStateString(std::int32_t State = -1);
    std::int32_t GetEquippedStyle();
    std::int32_t GetEquippedWeaponStyle();
    std::int32_t GetDefenseAgainst(std::int32_t Style);
    std::int32_t GetProtectedStyle();
    std::int32_t GetProtectedStyle(const Interactable::Player& P);

    std::vector<WorldArea> GetValidMovementAreas();
    std::vector<std::pair<bool, WorldArea>> GetValidMoveTiles(const std::shared_ptr<Gorilla>& Gorilla);
    Tile GetMeleeMoveTile(const std::shared_ptr<Gorilla>& Gorilla);
    Tile GetBoulderMoveTile(const std::shared_ptr<Gorilla>& Gorilla);
    Tile GetGorillaMoveTile(const std::shared_ptr<Gorilla>& Gorilla);
    bool IsAttacking();
    bool IsAttacking(const std::shared_ptr<Gorilla>& Gorilla);
    bool ShouldLeave();
    bool ShouldInterrupt();

    bool AdjustCamera();
    bool Attack( const std::shared_ptr<Gorilla>& Gorilla, bool Force, bool Wait = false);
    bool SwitchPrayer(Prayer::PRAYERS Prayer, bool Force = false);
    bool StopCasting();

    namespace
    {
        std::shared_ptr<Tile> CurrentMeleeMoveTile;
        std::shared_ptr<Tile> CurrentBoulderMoveTile;

        std::shared_ptr<Tile> GetCurrentMeleeMoveTile() { return CurrentMeleeMoveTile; };
        std::shared_ptr<Tile> GetCurrentBoulderMoveTile() { return CurrentBoulderMoveTile; };
    }

    bool MeleeMove(std::int32_t& State, const std::shared_ptr<Gorilla>& Gorilla);
    bool BoulderMove(std::int32_t& State, const std::shared_ptr<Gorilla>& Gorilla);
    bool Prayers(std::int32_t& State, const std::shared_ptr<Gorilla>& Gorilla);
    bool Gear(std::int32_t& State, const std::shared_ptr<Gorilla>& Gorilla);
    bool Special();

    bool Food(); // used in combat - returns true if it needs to/did do something
    bool Restore();  // used in combat - returns true if it needs to/did do something
    bool Topoff(double MaxOverheal = 0.15, double MaxOverrrestore = 0.05); // Returns true if it did do something - used outside of combat

    bool WalkTo();
    bool Fight();

    void OnGameTick();

    void Draw();
}

#endif // GORILLAS_HPP_INCLUDED