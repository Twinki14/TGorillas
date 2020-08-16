#ifndef TRAVEL_HPP_INCLUDED
#define TRAVEL_HPP_INCLUDED

#include <cstdint>
#include <Core/Types/Area.hpp>

namespace Globals
{
    constexpr std::int32_t REGION_CRASH_SITE = 8023;
    constexpr std::int32_t REGION_CRASH_SITE_CAVERN_LEFT = 8280;
    constexpr std::int32_t REGION_CRASH_SITE_CAVERN_RIGHT = 8536;
    constexpr std::int32_t REGION_GRAND_TREE = 9782;
    constexpr std::int32_t PLANE_GRAND_TREE_LOWER = 0;
    constexpr std::int32_t PLANE_GRAND_TREE_MIDDLE = 1;

    const Area AREA_CRASH_SITE_CAVERN_INNER = Area(Tile(2091, 5666), Tile(2119, 5638));
}

namespace Travel
{
    enum AREA
    {
        UNKNOWN = -1,
        CRASH_SITE,
        CRASH_SITE_CAVERN,
        CRASH_SITE_CAVERN_INNER,
        GRAND_TREE_LOWER,
        GRAND_TREE_MIDDLE,
    };

    bool InCavern();
    std::int32_t GetLocation();
}

#endif // TRAVEL_HPP_INCLUDED