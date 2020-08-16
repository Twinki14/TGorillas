#include "Travel.hpp"
#include <Core/Internal.hpp>
#include <Game/Tools/Pathfinding.hpp>
#include <Game/Interfaces/Minimap.hpp>

bool Travel::InCavern()
{
    auto RegionID = Pathfinding::GetCurrentRegion();
    switch (RegionID)
    {
        case Globals::REGION_CRASH_SITE_CAVERN_LEFT:
        case Globals::REGION_CRASH_SITE_CAVERN_RIGHT: return true;
        default: break;
    }
    return false;
}

std::int32_t Travel::GetLocation()
{
    auto RegionID = Pathfinding::GetCurrentRegion();
    switch (RegionID)
    {
        case Globals::REGION_CRASH_SITE: return CRASH_SITE;
        case Globals::REGION_CRASH_SITE_CAVERN_LEFT:
        case Globals::REGION_CRASH_SITE_CAVERN_RIGHT:
        {
            if (Globals::AREA_CRASH_SITE_CAVERN_INNER.Contains(Minimap::GetPosition()))
                return CRASH_SITE_CAVERN_INNER;
            return CRASH_SITE_CAVERN;
        }
        case Globals::REGION_GRAND_TREE:
        {
            switch(Internal::GetClientPlane())
            {
                case Globals::PLANE_GRAND_TREE_LOWER: return GRAND_TREE_LOWER;
                case Globals::PLANE_GRAND_TREE_MIDDLE: return GRAND_TREE_MIDDLE;
                default: break;
            }
            break;
        }
        default: break;
    }

    return UNKNOWN;
}