#include <Game/Core.hpp>
#include <TScript.hpp>
#include "Gorilla.hpp"
#include "../Listeners/GameListener.hpp"

Gorilla::Gorilla() : Interactable::NPC(nullptr)
{
    this->LastWorldArea = std::make_shared<WorldArea>(WorldArea());
}

Gorilla::Gorilla(Interactable::NPC& N, std::int32_t Index) : Interactable::NPC(N), Index(Index)
{
    this->LastWorldArea = std::make_shared<WorldArea>(WorldArea());
}

std::int32_t Gorilla::GetIndex() const
{
    return this->Index;
}

std::int32_t Gorilla::GetOverheadIcon() const
{
    return this->GetInfo().GetOverheadPrayer();
}

std::int32_t Gorilla::GetProtectionStyle() const
{
    enum STYLE { MELEE_PRAYER, RANGED_PRAYER, MAGIC_PRAYER };
    switch (this->GetInfo().GetOverheadPrayer())
    {
        case MELEE_PRAYER:     return Gorilla::MELEE_FLAG;
        case RANGED_PRAYER:    return Gorilla::RANGED_FLAG;
        case MAGIC_PRAYER:     return Gorilla::MAGIC_FLAG;
        default: break;
    }
    return -1;
}

std::int32_t Gorilla::GetFacingDirection() const
{
    switch (this->GetAngle())
    {
        case NORTH_WEST + 1 ... NORTH: return NORTH;
        case NORTH + 1 ... NORTH_EAST: return NORTH_EAST;
        case NORTH_EAST + 1 ... EAST: return EAST;
        case EAST + 1 ... SOUTH_EAST: return SOUTH_EAST;

        case SOUTH:
        case SOUTH_EAST + 1 ... 2016: return SOUTH;
        case SOUTH + 1 ... SOUTH_WEST: return SOUTH_WEST;
        case SOUTH_WEST + 1 ... WEST: return WEST;
        case WEST + 1 ... NORTH_WEST: return NORTH_WEST;
        default: break;
    }
    return -1;
}

Internal::Player Gorilla::GetInteractingPlayer() const
{
    return (Internal::Player) this->GetInteracting();
}

Tile Gorilla::GetTrueLocation() const
{
    const auto PathX = this->GetPathX();
    const auto PathY = this->GetPathY();
    if (PathX.empty() || PathY.empty()) return Tile();

    const auto ClientX = Internal::GetClientX();
    const auto ClientY = Internal::GetClientY();
    const auto ClientPlane = Internal::GetClientPlane();
    if (ClientX < 0 || (ClientX >= 65536) || ClientY < 0 || (ClientY >= 65536))
        return Tile(-1, -1, -1);

    return Tile(ClientX + PathX[0], ClientY + PathY[0], ClientPlane);
}

WorldArea Gorilla::GetWorldArea() const
{
    return WorldArea(*this);
}

std::vector<WorldArea> Gorilla::GetSurroundingMovementAreas() const
{
    auto W = this->GetWorldArea();
    if (!W) return std::vector<WorldArea>();

    const auto CollisionFlags = Internal::GetCollisionMap(W.GetPlane()).GetFlags();
    std::vector<WorldArea> Result = W.GetSurroundingAreas();
    Result.erase(std::remove_if(Result.begin(), Result.end(), [&](const WorldArea& A) -> bool
    {
        if (A.GetSceneX() >= 0 && A.GetSceneX() < CollisionFlags.size()
            && A.GetSceneY() >= 0 && A.GetSceneY() < CollisionFlags.size())
        {
            const auto Flag = CollisionFlags[A.GetSceneX()][A.GetSceneY()];
            return (Flag == Pathfinding::CLOSED) || (Flag & Pathfinding::BLOCKED) || (Flag & Pathfinding::OCCUPIED) || (Flag & Pathfinding::SOLID);
        }
        return false;
    }), Result.end());

    return Result;
}

WorldArea Gorilla::GetNextTravelingPoint(const WorldArea& TargetArea, const std::vector<WorldArea>& Gorillas, const std::vector<WorldArea>& Players) const
{
    if (!this->LastWorldArea) return WorldArea();

    std::function<bool(const Tile&)> MovementBlocked = [&](const Tile& T) -> bool
    {
        // Gorillas can't normally walk through other gorillas
        // or other players

        const auto Area1 = WorldArea(T, 1, 1);
        if (!Area1)
            return true;

        bool IntersectsWithGorilla = std::any_of(Gorillas.begin(), Gorillas.end(), [&Area1](const WorldArea& A) -> bool { return A && Area1.IntersectsWith(A); });
        bool IntersectsWithPlayer = std::any_of(Players.begin(), Players.end(), [&Area1](const WorldArea& A) -> bool { return A && Area1.IntersectsWith(A); });

        return !IntersectsWithGorilla && !IntersectsWithPlayer;

        // There is a special case where if a player walked through
        // a gorilla, or a player walked through another player,
        // the tiles that were walked through becomes
        // walkable, but I didn't feel like it's necessary to handle
        // that special case as it should rarely happen.
    };

    return this->GetLastWorldArea()->CalculateNextTravellingPoint(TargetArea, true, MovementBlocked);
}

bool Gorilla::IsDead() const
{
    return this->LastHealthPercentage == 0.00 || Internal::GetHealthPercentage(*this) == 0.00 || this->GetAnimationID() == Globals::Gorillas::ANIMATION_DYING;
}

bool Gorilla::InCombat() const
{
    auto AnimationID = this->GetAnimationID();
    return this->InitiatedCombat && !this->IsDead() &&
           (
                   (this->NextAttackTick + 3 >= GameListener::GetTickCount())
                   || (AnimationID >= Globals::Gorillas::ANIMATION_DEFEND && AnimationID <= Globals::Gorillas::ANIMATION_AOE_ATTACK)
           );
}

bool Gorilla::HealthBarShowing() const
{
    return Internal::GetHealthPercentage(*this) != -1.00;
}

double Gorilla::GetHealthPercentage() const
{
    return Internal::GetHealthPercentage(*this);
}

void Gorilla::SetLastWorldArea(std::shared_ptr<WorldArea> Area)
{
    this->LastWorldArea = std::move(Area);
}

std::shared_ptr<WorldArea> Gorilla::GetLastWorldArea() const
{
    return LastWorldArea;
}

std::uint32_t Gorilla::CountNextPossibleAttackStyles() const
{
    std::uint32_t Styles = 0;
    if (NextPossibleAttackStyles & MELEE_FLAG) Styles++;
    if (NextPossibleAttackStyles & RANGED_FLAG) Styles++;
    if (NextPossibleAttackStyles & MAGIC_FLAG) Styles++;
    return Styles;
}

void Gorilla::Draw(bool Emphasize, const WorldArea& NextTravelingPoint) const
{
    auto Interacting = this->GetInteractingPlayer();
    auto Target = GameListener::GetPlayer(Interacting);
    auto TrueLoc = this->GetTrueLocation();
    auto TrueLocPoint = Internal::TileToMainscreen(TrueLoc, 0, 0, 0);

    Paint::Pixel IndexColor = { 0, 255, 0, static_cast<uint8_t>(Emphasize ? 255 : 100) };
    Paint::Pixel StyleColor = { 0, 255, 255, static_cast<uint8_t>(Emphasize ? 255 : 100) };
    Paint::Pixel LineColor = { 255, 0, 0, static_cast<uint8_t>(Emphasize ? 255 : 100) };
    Paint::Pixel TileColor = { 0, 255, 255, static_cast<uint8_t>(Emphasize ? 255 : 100) };
    Paint::Pixel TravelPointColor = { 0, 255, 0, static_cast<uint8_t>(Emphasize ? 255 : 100) };

    if (Emphasize)
    {
        Paint::DrawConvex(this->GetConvex(), IndexColor.Red, IndexColor.Green, IndexColor.Blue, IndexColor.Alpha);
        Paint::DrawTile(TrueLoc, TileColor.Red, TileColor.Green, TileColor.Blue, TileColor.Alpha);
        if (NextTravelingPoint && LastWorldArea)
        {
            const auto PredictedTile = NextTravelingPoint.AsTile();
            const auto LastWorldAreaTile = GetLastWorldArea()->AsTile();

            auto DistAB = PredictedTile.DistanceFrom(LastWorldAreaTile);
            auto DistRL = std::max(std::abs(PredictedTile.X - LastWorldAreaTile.X), std::abs(PredictedTile.Y - LastWorldAreaTile.Y));

            auto DistAB_TrueLoc = PredictedTile.DistanceFrom(TrueLoc);
            auto DistRL_TrueLoc = std::max(std::abs(PredictedTile.X - TrueLoc.X), std::abs(PredictedTile.Y - TrueLoc.Y));

            std::string Text = std::to_string(DistAB) + " | " + std::to_string(DistRL)
                               + "\n" + std::to_string(DistAB_TrueLoc) + " | " + std::to_string(DistRL_TrueLoc);
            Paint::DrawTile(PredictedTile, TravelPointColor.Red, TravelPointColor.Green, TravelPointColor.Blue, TravelPointColor.Alpha);
            Paint::DrawString(Text, Internal::TileToMainscreen(PredictedTile, 0, 0, 0), TravelPointColor.Red, TravelPointColor.Green, TravelPointColor.Blue, TravelPointColor.Alpha);
        }
    }

    if (Target && *Target)
    {
        auto TargetLoc = Target->GetTile();
        auto PointB = Internal::TileToMainscreen(TargetLoc, 0, 0, 0);
        Paint::DrawLine(TrueLocPoint, PointB, LineColor.Red, LineColor.Green, LineColor.Blue, LineColor.Alpha);
        Paint::DrawString(std::to_string(Target->GetIndex()), PointB - Point(0, 15), LineColor.Red, LineColor.Green, LineColor.Blue, LineColor.Alpha);
    }

    std::string StyleStr;
    if (this->NextPossibleAttackStyles & MELEE_FLAG) StyleStr += "MELEE ";
    if (this->NextPossibleAttackStyles & RANGED_FLAG) StyleStr += "RANGED ";
    if (this->NextPossibleAttackStyles & MAGIC_FLAG) StyleStr += "MAGIC ";
    if (this->NextPossibleAttackStyles & BOULDER_FLAG) StyleStr += "BOULDER ";
    StyleStr += "(" + std::to_string(this->CountNextPossibleAttackStyles()) + ")";
    Point Text = Internal::TileToMainscreen(this->GetTile(), 0, 0, 0) - Point(0, 25);
    Paint::DrawString(std::to_string(this->GetIndex()) + " | " + std::to_string(this->InCombat()) + " | " + StyleStr, Text, IndexColor.Red, IndexColor.Green, IndexColor.Blue, IndexColor.Alpha); Text.Y += 20;
    //Paint::DrawString(StyleStr, Text, StyleColor.Red, StyleColor.Green, StyleColor.Blue, StyleColor.Alpha); Text.Y += 20;
}
