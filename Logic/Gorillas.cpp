#include <TScript.hpp>
#include <Utilities/GearSets.hpp>
#include <Game/Models/Players.hpp>
#include <Utilities/Mainscreen.hpp>
#include <Game/Interfaces/GameTabs/Prayer.hpp>
#include "Gorillas.hpp"
#include "Travel.hpp"
#include "Listeners/GameListener.hpp"

std::int32_t Gorillas::GetState()
{
    auto Gorilla = GameListener::GetCurrentGorilla();
    auto Player = Players::GetLocal();

    if (!Player || !Gorilla || !*Gorilla || Gorilla->IsDead())
        return 0;

    std::int32_t State = 0;

    std::int32_t EquippedStyle = Gorillas::GetEquippedStyle();
    std::int32_t ProtectedStyle = Gorillas::GetProtectedStyle();
    std::int32_t GorillaProtectedStyle = Gorilla->GetProtectionStyle();
    std::uint32_t NumberOfPossibleAttacks = Gorilla->CountNextPossibleAttackStyles();

    if (NumberOfPossibleAttacks == 1 && Gorilla->NextPossibleAttackStyles != BOULDER_FLAG)
    {
        if ((ProtectedStyle & Gorilla->NextPossibleAttackStyles) == 0)
        {
            State |= SINGLE_SWITCH_PRAYER;
            if (Gorilla->NextPossibleAttackStyles & MELEE_FLAG)     State |= SWITCH_PRAYER_MELEE;
            if (Gorilla->NextPossibleAttackStyles & RANGED_FLAG)    State |= SWITCH_PRAYER_RANGED;
            if (Gorilla->NextPossibleAttackStyles & MAGIC_FLAG)     State |= SWITCH_PRAYER_MAGIC;
        }
    } else
    {
        if (Gorilla->NextPossibleAttackStyles & RANGED_FLAG && Gorilla->NextPossibleAttackStyles & MAGIC_FLAG)
        {
            State &= ~SINGLE_SWITCH_PRAYER;
            State |= SWITCH_PRAYER_RANGED;
            State |= SWITCH_PRAYER_MAGIC;
        }

        if (NumberOfPossibleAttacks == 2 && Gorilla->NextPossibleAttackStyles & MELEE_FLAG)
            State |= MELEE_MOVE;
    }

    if (EquippedStyle & GorillaProtectedStyle || EquippedStyle == 0)
    {
        if (GorillaProtectedStyle & MELEE_FLAG) State |= EQUIP_RANGED;
        if (GorillaProtectedStyle & RANGED_FLAG) State |= EQUIP_MELEE;
        if (GorillaProtectedStyle & MAGIC_FLAG && EquippedStyle == 0)
        {
            State |= EQUIP_MELEE; // TODO equip best
        }
    }

    if (GameListener::AnyActiveBoulderOn(Mainscreen::GetTrueLocation()))
        State |= BOULDER;
    return State;
}

std::int32_t Gorillas::GetEquippedStyle()
{
    if (GearSets::Sets["Melee"].Equipped()) return MELEE_FLAG;
    if (GearSets::Sets["Ranged"].Equipped()) return RANGED_FLAG;
    return 0;
}

std::int32_t Gorillas::GetDefenseAgainst(std::int32_t Style)
{
    std::int32_t Defense = 0;

    if (Style & MELEE_FLAG)
    {
        // Crush
    }

    if (Style & RANGED_FLAG)
    {

    }

    if (Style & MAGIC_FLAG)
    {

    }

    return Defense;
}

std::int32_t Gorillas::GetProtectedStyle()
{
    if (Prayer::IsActive(Prayer::PROTECT_FROM_MELEE)) return Gorillas::MELEE_FLAG;
    if (Prayer::IsActive(Prayer::PROTECT_FROM_MISSILES)) return Gorillas::RANGED_FLAG;
    if (Prayer::IsActive(Prayer::PROTECT_FROM_MAGIC)) return Gorillas::MAGIC_FLAG;
    return 0;
}

std::int32_t Gorillas::GetProtectedStyle(const Interactable::Player& P)
{
    if (!P) return 0;
    const auto ProtectionIcon = P.GetOverheadIcon();
    switch (ProtectionIcon)
    {
        case Globals::ICON_MELEE:   return Gorillas::MELEE_FLAG;
        case Globals::ICON_RANGED:  return Gorillas::RANGED_FLAG;
        case Globals::ICON_MAGIC:   return Gorillas::MAGIC_FLAG;
        default: break;
    }
    return 0;
}

bool Gorillas::WalkTo()
{
    if (!Travel::InCavern()) return false;
    if (Travel::GetLocation() == Travel::CRASH_SITE_CAVERN_INNER) return true;




    return true;
}

bool Gorillas::Fight()
{
    if (Travel::GetLocation() != Travel::CRASH_SITE_CAVERN_INNER) return false;




    return true;
}