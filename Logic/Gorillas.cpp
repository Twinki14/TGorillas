#include <TScript.hpp>
#include <Utilities/GearSets.hpp>
#include "Gorillas.hpp"

std::int32_t Gorillas::GetEquippedStyle()
{
    if (GearSets::Sets["Melee"].Equipped()) return MELEE_FLAG;
    if (GearSets::Sets["Ranged"].Equipped()) return RANGED_FLAG;
    return -1;
}
