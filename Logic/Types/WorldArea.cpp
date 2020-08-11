/*
 * Based off RuneLite original code
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

#include <Core/Internal.hpp>
#include "WorldArea.hpp"

WorldArea::WorldArea(const Tile& WorldTile)
{
    if (WorldTile)
    {
        SceneX = WorldTile.X - Internal::GetClientX();
        SceneY = WorldTile.Y - Internal::GetClientY();
        X = WorldTile.X;
        Y = WorldTile.Y;
        Width = 1;
        Height = 1;
        Plane = WorldTile.Plane;
    }
}

WorldArea::WorldArea(const Tile& WorldTile, std::int32_t Width, std::int32_t Height)
{
    if (WorldTile)
    {
        SceneX = WorldTile.X - Internal::GetClientX();
        SceneY = WorldTile.Y - Internal::GetClientY();
        X = WorldTile.X;
        Y = WorldTile.Y;
        this->Width = Width;
        this->Height = Height;
        Plane = WorldTile.Plane;
    }
}

WorldArea::WorldArea(const Internal::Player& P)
{
    if (P)
    {
        SceneX = P.GetPathX()[0];
        SceneY = P.GetPathY()[0];
        X = SceneX + Internal::GetClientX();
        Y = SceneY + Internal::GetClientY();
        Width = 1;
        Height = 1;
        Plane = Internal::GetClientPlane();
    }
}

WorldArea::WorldArea(const Internal::NPC& N)
{
    if (N)
    {
        auto Size = N.GetNPCInfo().GetSize();
        SceneX = N.GetPathX()[0];
        SceneY = N.GetPathY()[0];
        X = Internal::GetClientX() + SceneX;
        Y = Internal::GetClientY() + SceneY;
        Width = Size;
        Height = Size;
        Plane = Internal::GetClientPlane();
    }
}

int32_t WorldArea::GetX() const
{
    return X;
}

int32_t WorldArea::GetY() const
{
    return Y;
}

uint32_t WorldArea::GetWidth() const
{
    return Width;
}

uint32_t WorldArea::GetHeight() const
{
    return Height;
}

int32_t WorldArea::GetPlane() const
{
    return Plane;
}

std::int32_t WorldArea::GetSceneX() const
{
    return SceneX;
}

std::int32_t WorldArea::GetSceneY() const
{
    return SceneY;
}

std::int32_t WorldArea::DistanceTo(const WorldArea& Other) const
{
    if (Plane != Other.GetPlane())
        return -1;
    return DistanceTo2D(Other);
}

std::int32_t WorldArea::DistanceTo2D(const WorldArea& Other) const
{
    Point Distances = GetAxisDistances(Other);
    return std::max(Distances.X, Distances.Y);
}

// Ported from https://github.com/open-osrs/runelite/blob/master/runelite-mixins/src/main/java/net/runelite/mixins/RSTileMixin.java#L122
// Thank you 'Henke'!
bool WorldArea::HasLineOfSightTo(const WorldArea& Other) const
{
    if (Plane != Other.GetPlane()) return false;

    const auto CollisionData = Internal::GetCollisionMap(Plane);
    const auto CollisionFlags = CollisionData.GetFlags();
    if (!CollisionData || CollisionFlags.empty()) return false;

    const Point p1 = Point(this->GetSceneX(), this->GetSceneY());
    const Point p2 = Point(Other.GetSceneX(), Other.GetSceneY());
    if (p1 == p2) return true;

    int dx = p2.X - p1.X;
    int dy = p2.Y - p1.Y;
    int dxAbs = std::abs(dx);
    int dyAbs = std::abs(dy);

    int xFlags = BLOCK_LINE_OF_SIGHT_FULL;
    int yFlags = BLOCK_LINE_OF_SIGHT_FULL;

    if (dx < 0)
        xFlags |= BLOCK_LINE_OF_SIGHT_EAST;
    else
        xFlags |= BLOCK_LINE_OF_SIGHT_WEST;

    if (dy < 0)
        yFlags |= BLOCK_LINE_OF_SIGHT_NORTH;
    else
        yFlags |= BLOCK_LINE_OF_SIGHT_SOUTH;

    if (dxAbs > dyAbs)
    {
        int x = p1.X;
        int yBig = p1.Y << 16; // The y position is represented as a bigger number to handle rounding
        int slope = (dy << 16) / dxAbs;
        yBig += 0x8000; // Add half of a tile
        if (dy < 0)
            yBig--; // For correct rounding
        int direction = dx < 0 ? -1 : 1;

        while (x != p2.X)
        {
            x += direction;
            int y = yBig >> 16;
            if ((CollisionFlags[x][y] & xFlags) != 0)
            {
                // Collision while traveling on the x axis
                return false;
            }
            yBig += slope;
            int nextY = yBig >> 16;
            if (nextY != y && (CollisionFlags[x][nextY] & yFlags) != 0)
            {
                // Collision while traveling on the y axis
                return false;
            }
        }
    } else
    {
        int y = p1.Y;
        int xBig = p1.X << 16; // The x position is represented as a bigger number to handle rounding
        int slope = (dx << 16) / dyAbs;
        xBig += 0x8000; // Add half of a tile
        if (dx < 0)
        {
            xBig--; // For correct rounding
        }
        int direction = dy < 0 ? -1 : 1;

        while (y != p2.Y)
        {
            y += direction;
            int x = xBig >> 16;
            if ((CollisionFlags[x][y] & yFlags) != 0)
            {
                // Collision while traveling on the y axis
                return false;
            }

            xBig += slope;
            int nextX = xBig >> 16;
            if (nextX != x && (CollisionFlags[nextX][y] & xFlags) != 0)
            {
                // Collision while traveling on the x axis
                return false;
            }
        }
    }

    // No collision
    return true;
}

Point WorldArea::GetComparisonPoint(const WorldArea& Other) const
{
    int x, y;
    if (Other.GetX() <= X)
        x = X;
    else if (Other.GetX() >= X + Width - 1)
        x = X + Width - 1;
    else
        x = Other.GetX();

    if (Other.GetY() <= Y)
        y = Y;
    else if (Other.GetY() >= Y + Height - 1)
        y = Y + Height - 1;
    else
        y = Other.GetY();

    return Point(x, y);
}

Point WorldArea::GetAxisDistances(const WorldArea& Other) const
{
    Point p1 = this->GetComparisonPoint(Other);
    Point p2 = Other.GetComparisonPoint(*this);
    return Point(std::abs(p1.X - p2.X), std::abs(p1.Y - p2.Y));
}

bool WorldArea::IsInMeleeDistance(const WorldArea& Other) const
{
    if (!Other || Plane != Other.GetPlane())
        return false;

    Point distances = GetAxisDistances(Other);
    return (distances.X + distances.Y) == 1;
}

bool WorldArea::IntersectsWith(const WorldArea& Other) const
{
    if (Plane != Other.GetPlane())
        return false;

    Point distances = GetAxisDistances(Other);
    return distances.X + distances.Y == 0;
}

bool WorldArea::CanTravelInDirection(std::int32_t DirectionX, std::int32_t DirectionY) const
{
    return CanTravelInDirection(DirectionX, DirectionY, [](const Tile& T) -> bool { return true; });
}

bool WorldArea::CanTravelInDirection(std::int32_t DirectionX, std::int32_t DirectionY, const std::function<bool(const Tile&)>& Pred) const
{
    DirectionX = (DirectionX > 0) ? 1 : ((DirectionX < 0) ? -1 : 0);
    DirectionY = (DirectionY > 0) ? 1 : ((DirectionY < 0) ? -1 : 0);

    if (DirectionX == 0 && DirectionY == 0) return true;

    std::int32_t StartX = GetSceneX() + DirectionX;
    std::int32_t StartY = GetSceneY() + DirectionY;

    std::int32_t CheckX = StartX + (DirectionX > 0 ? Width - 1 : 0);
    std::int32_t CheckY = StartY + (DirectionY > 0 ? Height - 1 : 0);
    std::int32_t EndX = StartX + Width - 1;
    std::int32_t EndY = StartY + Height - 1;

    std::int32_t XFlags = BLOCK_MOVEMENT_FULL;
    std::int32_t YFlags = BLOCK_MOVEMENT_FULL;
    std::int32_t XYFlags = BLOCK_MOVEMENT_FULL;
    std::int32_t XWallFlagsSouth = BLOCK_MOVEMENT_FULL;
    std::int32_t XWallFlagsNorth = BLOCK_MOVEMENT_FULL;
    std::int32_t YWallFlagsWest = BLOCK_MOVEMENT_FULL;
    std::int32_t YWallFlagsEast = BLOCK_MOVEMENT_FULL;

    if (DirectionX < 0)
    {
        XFlags |= BLOCK_MOVEMENT_EAST;
        XWallFlagsSouth |= BLOCK_MOVEMENT_SOUTH | BLOCK_MOVEMENT_SOUTH_EAST;
        XWallFlagsNorth |= BLOCK_MOVEMENT_NORTH | BLOCK_MOVEMENT_NORTH_EAST;
    }

    if (DirectionX > 0)
    {
        XFlags |= BLOCK_MOVEMENT_WEST;
        XWallFlagsSouth |= BLOCK_MOVEMENT_SOUTH | BLOCK_MOVEMENT_SOUTH_WEST;
        XWallFlagsNorth |= BLOCK_MOVEMENT_NORTH | BLOCK_MOVEMENT_NORTH_WEST;
    }

    if (DirectionY < 0)
    {
        YFlags |= BLOCK_MOVEMENT_NORTH;
        YWallFlagsWest |= BLOCK_MOVEMENT_WEST | BLOCK_MOVEMENT_NORTH_WEST;
        YWallFlagsEast |= BLOCK_MOVEMENT_EAST | BLOCK_MOVEMENT_NORTH_EAST;
    }

    if (DirectionY > 0)
    {
        YFlags |= BLOCK_MOVEMENT_SOUTH;
        YWallFlagsWest |= BLOCK_MOVEMENT_WEST | BLOCK_MOVEMENT_SOUTH_WEST;
        YWallFlagsEast |= BLOCK_MOVEMENT_EAST | BLOCK_MOVEMENT_SOUTH_EAST;
    }

    if (DirectionX < 0 && DirectionY < 0) XYFlags |= BLOCK_MOVEMENT_NORTH_EAST;
    if (DirectionX < 0 && DirectionY > 0) XYFlags |= BLOCK_MOVEMENT_SOUTH_EAST;
    if (DirectionX > 0 && DirectionY < 0) XYFlags |= BLOCK_MOVEMENT_NORTH_WEST;
    if (DirectionX > 0 && DirectionY > 0) XYFlags |= BLOCK_MOVEMENT_SOUTH_WEST;

    const auto ClientX = Internal::GetClientX();
    const auto ClientY = Internal::GetClientY();
    const auto ClientPlane = Internal::GetClientPlane();
    const auto ClientTile = Tile(ClientX, ClientY, ClientPlane);

    const auto CollisionData = Internal::GetCollisionMap(Plane);
    const auto CollisionFlags = CollisionData.GetFlags();
    if (!CollisionData || CollisionFlags.empty()) return false;

    if (DirectionX != 0)
    {
        // Check that the area doesn't bypass a wall
        for (int NextY = StartY; NextY <= EndY; NextY++)
        {
            if ((CollisionFlags[CheckX][NextY] & XFlags) != 0 || !Pred(Tile(CheckX, NextY, ClientPlane) + ClientTile))
                return false; // Collision while attempting to travel along the x axis
        }

        // Check that the new area tiles don't contain a wall
        for (int NextY = StartY + 1; NextY <= EndY; NextY++)
        {
            if ((CollisionFlags[CheckX][NextY] & XWallFlagsSouth) != 0)
                return false; // The new area tiles contains a wall
        }

        for (int NextY = EndY - 1; NextY >= StartY; NextY--)
        {
            if ((CollisionFlags[CheckX][NextY] & XWallFlagsNorth) != 0)
                return false;// The new area tiles contains a wall
        }
    }

    if (DirectionY != 0)
    {
        // Check that the area tiles don't bypass a wall
        for (int NextX = StartX; NextX <= EndX; NextX++)
        {
            if ((CollisionFlags[NextX][CheckY] & YFlags) != 0 || !Pred(Tile(NextX, CheckY, ClientPlane) + ClientTile))
                return false; // Collision while attempting to travel along the y axis
        }

        // Check that the new area tiles don't contain a wall
        for (int NextX = StartX + 1; NextX <= EndX; NextX++)
        {
            if ((CollisionFlags[NextX][CheckY] & YWallFlagsWest) != 0)
                return false; // The new area tiles contains a wall
        }

        for (int NextX = EndX - 1; NextX >= StartX; NextX--)
        {
            if ((CollisionFlags[NextX][CheckY] & YWallFlagsEast) != 0)
                return false; // The new area tiles contains a wall
        }
    }

    if (DirectionX != 0 && DirectionY != 0)
    {
        if ((CollisionFlags[CheckX][CheckY] & XYFlags) != 0 || !Pred(Tile(CheckX, StartY, ClientPlane) + ClientTile))
            return false; // Collision while attempting to travel diagonally

        // When the areas edge size is 1 and it attempts to travel
        // diagonally, a collision check is done for respective
        // x and y axis as well.
        if (Width == 1)
        {
            if ((CollisionFlags[CheckX][CheckY - DirectionY] & XFlags) != 0 && Pred(Tile(CheckX, StartY, ClientPlane) + ClientTile))
                return false;
        }

        if (Height == 1)
            return (CollisionFlags[CheckX - DirectionX][CheckY] & YFlags) == 0 || !Pred(Tile(StartX, CheckY, ClientPlane) + ClientTile);
    }

    return true;
}

WorldArea WorldArea::CalculateNextTravellingPoint(const WorldArea& Target, bool StopAtMeleeDistance)
{
    return CalculateNextTravellingPoint(Target, StopAtMeleeDistance, [](const Tile& T) -> bool { return true; });
}

WorldArea WorldArea::CalculateNextTravellingPoint(const WorldArea& Target, bool StopAtMeleeDistance, const std::function<bool(const Tile&)>& Pred)
{
    if (Plane != Target.GetPlane()) return WorldArea();

    if (IntersectsWith(Target))
        return StopAtMeleeDistance ? WorldArea() : *this; // Movement is unpredictable when the NPC and actor stand on top of each other

    std::int32_t DirectionX = Target.GetX() - X;
    std::int32_t DirectionY = Target.GetY() - Y;
    Point AxisDistances = GetAxisDistances(Target);
    if (StopAtMeleeDistance && AxisDistances.X + AxisDistances.Y == 1)
        return *this; // NPC is in melee distance of target, so no movement is done

    if (SceneX + DirectionX < 0 || SceneX + DirectionY >= 104 ||
        SceneY + DirectionX < 0 || SceneY + DirectionY >= 104)
            return WorldArea(); // NPC is travelling out of the scene, so collision data isn't available

    std::int32_t DirectionXSig = (DirectionX > 0) ? 1 : ((DirectionX < 0) ? -1 : 0);
    std::int32_t DirectionYSig = (DirectionY > 0) ? 1 : ((DirectionY < 0) ? -1 : 0);

    if (StopAtMeleeDistance && AxisDistances.X == 1 && AxisDistances.Y == 1)
    {
        // When it needs to stop at melee distance, it will only attempt
        // to travel along the x axis when it is standing diagonally
        // from the target
        if (CanTravelInDirection(DirectionXSig, 0, Pred))
            return WorldArea(Tile(X + DirectionXSig, Y, Plane), Width, Height);
    } else
    {
        if (CanTravelInDirection(DirectionXSig, DirectionYSig, Pred))
            return WorldArea(Tile(X + DirectionXSig, Y + DirectionYSig, Plane), Width, Height);

        else if (DirectionX != 0 && CanTravelInDirection(DirectionXSig, 0, Pred))
            return WorldArea(Tile(X + DirectionXSig, Y, Plane), Width, Height);

        else if (DirectionY != 0 && std::max(std::abs(DirectionX), std::abs(DirectionY)) > 1 && CanTravelInDirection(0, DirectionY, Pred))
            return WorldArea(Tile(X, Y + DirectionYSig, Plane), Width, Height); // Note that NPCs don't attempts to travel along the y-axis if the target is <= 1 tile distance away
    }

    return *this;
}

Tile WorldArea::AsTile() const
{
    return Tile(X, Y, Plane);
}

WorldArea::operator bool() const
{
    return !(X < 0 || Y < 0 || SceneX < 0 || SceneY < 0 || Plane < 0);
}

WorldArea::~WorldArea()
{

}