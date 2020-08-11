#include <TScript.hpp>
#include <Utilities/GearSets.hpp>
#include "Gorillas.hpp"

bool Gorillas::IsGorilla(std::int32_t ID)
{
    return false;
/*    return ID == Globals::NPCs::DEMONIC_GORILLA_MELEE_7144 ||
           ID == Globals::NPCs::DEMONIC_GORILLA_RANGE_7145 ||
           ID == Globals::NPCs::DEMONIC_GORILLA_MAGE_7146 ||
           ID == Globals::NPCs::DEMONIC_GORILLA_MELEE_7147 ||
           ID == Globals::NPCs::DEMONIC_GORILLA_RANGE_7148 ||
           ID == Globals::NPCs::DEMONIC_GORILLA_MAGE_7149;*/
}

std::int32_t Gorillas::GetCurrentlyEquippedStyle()
{
    if (GearSets::Sets["Ranged"].Equipped()) return RANGED;
    if (GearSets::Sets["Melee"].Equipped()) return MELEE;
    return -1;
}
