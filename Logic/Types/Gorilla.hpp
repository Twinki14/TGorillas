#ifndef GORILLA_HPP_INCLUDED
#define GORILLA_HPP_INCLUDED

#include <Game/Interactable/NPC.hpp>
#include <Game/Interactable/Player.hpp>
#include <atomic>
#include <shared_mutex>
#include "Hitsplat.hpp"
#include "WorldArea.hpp"

namespace Globals
{
    enum OVERHEAD_ICON
    {
        ICON_MELEE,
        ICON_RANGED,
        ICON_MAGIC
    };
}

namespace Globals::Gorillas
{
    constexpr int MAX_ATTACK_RANGE = 10; // Needs <= 10 tiles to reach target
    constexpr int ATTACK_RATE = 5; // 5 ticks between each attack
    constexpr int ATTACKS_PER_SWITCH = 3; // 3 unsuccessful attacks per style switch

    constexpr int PROJECTILE_MAGIC_SPEED = 8; // Travels 8 tiles per tick
    constexpr int PROJECTILE_RANGED_SPEED = 6; // Travels 6 tiles per tick
    constexpr int PROJECTILE_MAGIC_DELAY = 12; // Requires an extra 12 tiles
    constexpr int PROJECTILE_RANGED_DELAY = 9; // Requires an extra 9 tiles

    constexpr int DEMONIC_GORILLA_MELEE_7144    = 7144; // MELEE
    constexpr int DEMONIC_GORILLA_RANGE_7145    = 7145; // RANGE
    constexpr int DEMONIC_GORILLA_MAGE_7146     = 7146; // MAGE
    constexpr int DEMONIC_GORILLA_MELEE_7147    = 7147; // MELEE
    constexpr int DEMONIC_GORILLA_RANGE_7148    = 7148; // RANGE
    constexpr int DEMONIC_GORILLA_MAGE_7149     = 7149; // MAGE

    constexpr int ANIMATION_DYING = 7229;
    constexpr int ANIMATION_MAGIC_ATTACK = 7225;
    constexpr int ANIMATION_MELEE_ATTACK = 7226;
    constexpr int ANIMATION_RANGED_ATTACK = 7227;
    constexpr int ANIMATION_AOE_ATTACK = 7228;
    constexpr int ANIMATION_PRAYER_SWITCH = 7228;
    constexpr int ANIMATION_DEFEND = 7224;

    constexpr int PROJECTILE_RANGED = 1302;
    constexpr int PROJECTILE_MAGIC = 1304;
    constexpr int PROJECTILE_BOULDER = 856;

    inline const std::vector<std::int32_t> PROJECTILES { PROJECTILE_RANGED, PROJECTILE_MAGIC, PROJECTILE_BOULDER };
}

class Gorilla : public Interactable::NPC
{
public:
    Gorilla();
    explicit Gorilla(Interactable::NPC& N, std::int32_t Index);

    enum STYLE_FLAGS
    {
        MELEE_FLAG      = 1 << 0,
        RANGED_FLAG     = 1 << 1,
        MAGIC_FLAG      = 1 << 2,
        BOULDER_FLAG    = 1 << 3
    };

    std::int32_t GetIndex() const;
    std::int32_t GetOverheadIcon() const;
    std::int32_t GetProtectionStyle() const;
    Internal::Player GetInteractingPlayer() const;
    Tile GetTrueLocation() const;
    WorldArea GetWorldArea() const;
    WorldArea GetNextTravelingPoint(const WorldArea& TargetArea, const std::vector<WorldArea>& Gorillas, const std::vector<WorldArea>& Players) const;
    bool IsDead() const;
    bool HealthBarShowing() const;

    void SetLastWorldArea(std::shared_ptr<WorldArea> Area);
    std::shared_ptr<WorldArea> GetLastWorldArea() const;

    std::uint32_t CountNextPossibleAttackStyles() const;

    std::atomic<std::int32_t> NextAttackTick = -100;
    std::atomic<std::int32_t> AttacksUntilSwitch = Globals::Gorillas::ATTACKS_PER_SWITCH;
    std::atomic<std::int32_t> DisabledMeleeMovementTicks = 0;
    std::atomic<std::int32_t> NextPossibleAttackStyles = MELEE_FLAG | RANGED_FLAG | MAGIC_FLAG;

    std::atomic<bool> InitiatedCombat = false;
    std::atomic<bool> RecentlyTookDamage = false;
    std::atomic<bool> ChangedAttackStyleThisTick = false;
    std::atomic<bool> ChangedAttackStyleLastTick = false;
    std::atomic<bool> ChangedPrayerThisTick = false;

    std::atomic<std::int32_t> LastTickAnimation = -1;
    std::atomic<std::int32_t> LastTickOverheadIcon = -1;
    std::atomic<std::int32_t> LastTickInteractingIndex = -1;
    std::atomic<std::int32_t> LastHitsplatEndTick = -1;
    std::atomic<std::int32_t> LastProjectileID = -1;
    std::atomic<double> LastHealthPercentage = -1.00;

    void Draw(bool Emphasize = false, const WorldArea& NextTravelingPoint = WorldArea()) const;

private:
    std::int32_t Index = -1;
    std::shared_ptr<WorldArea> LastWorldArea;
};

#endif // GORILLA_HPP_INCLUDED