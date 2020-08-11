#ifndef WORLDAREA_HPP_INCLUDED
#define WORLDAREA_HPP_INCLUDED

/*
 * Based off RuneLite original code, ported for use with AlpacaBot
 * https://github.com/runelite/runelite/blob/master/runelite-api/src/main/java/net/runelite/api/coords/WorldArea.java
 */

/*
 * Copyright (c) 2018, Woox <https://github.com/wooxsolo>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <cstdint>
#include <Core/Classes/Player.hpp>
#include <Core/Classes/NPC.hpp>
#include <functional>

class WorldArea
{
public:
    WorldArea() = default;
    WorldArea(const Tile& WorldTile);
    WorldArea(const Tile& WorldTile, std::int32_t Width, std::int32_t Height);
    WorldArea(const Internal::Player& P);
    WorldArea(const Internal::NPC& N);

    std::int32_t GetX() const;
    std::int32_t GetY() const;
    std::uint32_t GetWidth() const;
    std::uint32_t GetHeight() const;
    std::int32_t GetPlane() const;

    std::int32_t GetSceneX() const;
    std::int32_t GetSceneY() const;

    std::int32_t DistanceTo(const WorldArea& Other) const;
    std::int32_t DistanceTo2D(const WorldArea& Other) const;

    bool HasLineOfSightTo(const WorldArea& Other) const;
    Point GetComparisonPoint(const WorldArea& Other) const;
    Point GetAxisDistances(const WorldArea& Other) const;
    bool IsInMeleeDistance(const WorldArea& Other) const;
    bool IntersectsWith(const WorldArea& Other) const;
    bool CanTravelInDirection(std::int32_t DirectionX, std::int32_t DirectionY) const;
    bool CanTravelInDirection(std::int32_t DirectionX, std::int32_t DirectionY, const std::function<bool(const Tile&)>& Pred) const;
    WorldArea CalculateNextTravellingPoint(const WorldArea& Target, bool StopAtMeleeDistance);
    WorldArea CalculateNextTravellingPoint(const WorldArea& Target, bool StopAtMeleeDistance, const std::function<bool(const Tile&)>& Pred);

    Tile AsTile() const;
    operator bool() const;

    ~WorldArea();
private:
    std::int32_t SceneX = -1;
    std::int32_t SceneY = -1;
    std::int32_t X = -1;
    std::int32_t Y = -1;
    std::int32_t Width = 1;
    std::int32_t Height = 1;
    std::int32_t Plane = 0;

    enum COLLISION_FLAG
    {
        /**
         * Directional movement blocking flags.
         */
        BLOCK_MOVEMENT_NORTH_WEST = 0x1,
        BLOCK_MOVEMENT_NORTH = 0x2,
        BLOCK_MOVEMENT_NORTH_EAST = 0x4,
        BLOCK_MOVEMENT_EAST = 0x8,
        BLOCK_MOVEMENT_SOUTH_EAST = 0x10,
        BLOCK_MOVEMENT_SOUTH = 0x20,
        BLOCK_MOVEMENT_SOUTH_WEST = 0x40,
        BLOCK_MOVEMENT_WEST = 0x80,

        /**
         * Movement blocking type flags.
         */
        BLOCK_MOVEMENT_OBJECT = 0x100,
        BLOCK_MOVEMENT_FLOOR_DECORATION = 0x40000,
        BLOCK_MOVEMENT_FLOOR = 0x200000, // Eg. water
        BLOCK_MOVEMENT_FULL = BLOCK_MOVEMENT_OBJECT | BLOCK_MOVEMENT_FLOOR_DECORATION | BLOCK_MOVEMENT_FLOOR,

        /**
         * Directional line of sight blocking flags.
         */
        BLOCK_LINE_OF_SIGHT_NORTH = BLOCK_MOVEMENT_NORTH << 9, // 0x400
        BLOCK_LINE_OF_SIGHT_EAST = BLOCK_MOVEMENT_EAST << 9, // 0x1000
        BLOCK_LINE_OF_SIGHT_SOUTH = BLOCK_MOVEMENT_SOUTH << 9, // 0x4000
        BLOCK_LINE_OF_SIGHT_WEST = BLOCK_MOVEMENT_WEST << 9, // 0x10000
        BLOCK_LINE_OF_SIGHT_FULL = 0x20000
    };
};

#endif // WORLDAREA_HPP_INCLUDED