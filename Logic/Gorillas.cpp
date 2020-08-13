#include <TScript.hpp>
#include <Utilities/GearSets.hpp>
#include "Gorillas.hpp"

std::int32_t Gorillas::GetCurrentlyEquippedStyle()
{
    if (GearSets::Sets["Ranged"].Equipped()) return RANGED;
    if (GearSets::Sets["Melee"].Equipped()) return MELEE;
    return -1;
}
