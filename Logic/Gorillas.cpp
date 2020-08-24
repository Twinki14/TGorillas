#include <TScript.hpp>
#include <Utilities/GearSets.hpp>
#include <Utilities/Mainscreen.hpp>
#include <Utilities/TProfile.hpp>
#include <Utilities/Antiban.hpp>
#include <Game/Models/Players.hpp>
#include <Game/Interfaces/GameTabs/Prayer.hpp>
#include <Game/Interfaces/Minimap.hpp>
#include <Game/Tools/Pathfinding.hpp>
#include <Tools/OSRSBox/Items.hpp>
#include <Game/Interfaces/GameTabs/Equipment.hpp>
#include "Gorillas.hpp"
#include "Travel.hpp"
#include "Listeners/GameListener.hpp"
#include "../Config.hpp"

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
            if (Gorilla->NextPossibleAttackStyles & MELEE_FLAG)     State |= SWITCH_PRAYER_MELEE;
            if (Gorilla->NextPossibleAttackStyles & RANGED_FLAG)    State |= SWITCH_PRAYER_RANGED;
            if (Gorilla->NextPossibleAttackStyles & MAGIC_FLAG)     State |= SWITCH_PRAYER_MAGIC;
        }
    } else
    {
        if (Gorilla->NextPossibleAttackStyles & RANGED_FLAG && Gorilla->NextPossibleAttackStyles & MAGIC_FLAG)
        {
            if ((ProtectedStyle & RANGED_FLAG) == 0 && (ProtectedStyle & MAGIC_FLAG) == 0)
            {
                State |= SWITCH_PRAYER_RANGED;
                State |= SWITCH_PRAYER_MAGIC;
            }
        }

        if (NumberOfPossibleAttacks == 2 && Gorilla->NextPossibleAttackStyles & MELEE_FLAG)
            State |= MELEE_MOVE; // TODO check if we're already far enough away?
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
    if (GearSets::Sets.count("Melee") && GearSets::Sets.count("Ranged"))
    {
        if (GearSets::Sets["Melee"].Equipped()) return MELEE_FLAG;
        if (GearSets::Sets["Ranged"].Equipped()) return RANGED_FLAG;

        if (GearSets::Sets.count("Special") && GearSets::Sets["Special"].Equipped())
        {
            switch (Config::Get("SpecialWeapon").as_integer<int>())
            {
                case Config::TOXIC_BLOWPIPE: return RANGED_FLAG; break;
                case Config::SARADOMIN_GODSWORD: return MELEE_FLAG; break;
                default: break;
            }
        }
    }
    return 0;
}

std::int32_t Gorillas::GetEquippedWeaponStyle()
{
    if (GearSets::Sets.count("Melee") && GearSets::Sets.count("Ranged"))
    {
        auto WeaponItem = Equipment::GetItem(Equipment::WEAPON);
        if (WeaponItem.GetName() == GearSets::Sets["Melee"].Items[Equipment::WEAPON].Name) return MELEE_FLAG;
        if (WeaponItem.GetName() == GearSets::Sets["Ranged"].Items[Equipment::WEAPON].Name) return RANGED_FLAG;

        if (GearSets::Sets.count("Special") && WeaponItem)
        {
            if (WeaponItem.GetName() == GearSets::Sets["Special"].Items[Equipment::WEAPON].Name)
            {
                switch (Config::Get("SpecialWeapon").as_integer<int>())
                {
                    case Config::TOXIC_BLOWPIPE: return RANGED_FLAG; break;
                    case Config::SARADOMIN_GODSWORD: return MELEE_FLAG; break;
                    default: break;
                }
            }
        }
    }
    return 0;
}

std::int32_t Gorillas::GetDefenseAgainst(std::int32_t Style)
{
    if (Style & MELEE_FLAG)
    {
        std::int32_t TotalCrushDefense = 0;
        for (const auto& ID : Equipment::GetItemIDs())
        {
            auto OSRSBoxItem = OSRSBox::Items::Get(ID);
            if (OSRSBoxItem.equipment)
                TotalCrushDefense += OSRSBoxItem.equipment->defenceCrush;
        }
        return TotalCrushDefense;
    }

    if (Style & RANGED_FLAG)
    {
        std::int32_t TotalRangedDefense = 0;
        for (const auto& ID : Equipment::GetItemIDs())
        {
            auto OSRSBoxItem = OSRSBox::Items::Get(ID);
            if (OSRSBoxItem.equipment)
                TotalRangedDefense += OSRSBoxItem.equipment->defenceRanged;
        }
        return TotalRangedDefense;
    }

    if (Style & MAGIC_FLAG)
    {
        std::int32_t TotalMagicDefense = 0;
        for (const auto& ID : Equipment::GetItemIDs())
        {
            auto OSRSBoxItem = OSRSBox::Items::Get(ID);
            if (OSRSBoxItem.equipment)
                TotalMagicDefense += OSRSBoxItem.equipment->defenceMagic;
        }
        return TotalMagicDefense;
    }

    return 0;
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

std::vector<WorldArea> Gorillas::GetValidMovementAreas()
{
    std::vector<WorldArea> Result;
    auto Area = WorldArea(Internal::GetLocalPlayer());
    if (Area)
    {
        for (int X = -1; X <= 1; X++)
        {
            for (int Y = -1; Y <= 1; Y++)
            {
                if (X == 0 && Y == 0) continue;

                if (Area.CanTravelInDirection(X, Y))
                    Result.emplace_back(Area.AsTile() + Tile(X, Y));
            }
        }
    }
    return Result;
}

std::vector<std::pair<bool, WorldArea>> Gorillas::GetValidMoveTiles()
{
    std::vector<std::pair<bool, WorldArea>> Tiles;
    std::shared_ptr<Gorilla> CurrentGorilla = GameListener::GetCurrentGorilla();
    if (!CurrentGorilla || !*CurrentGorilla) return Tiles;

    const auto PlayerPos = Mainscreen::GetTrueLocation();

    auto ClientX = Internal::GetClientX();
    auto ClientY = Internal::GetClientY();
    auto CollisionFlags = Internal::GetCollisionMap(PlayerPos.Plane).GetFlags();

    std::vector<WorldArea> PlayerAreas = GameListener::GetPlayerAreas(false);
    std::vector<WorldArea> GorillaAreas = GameListener::GetGorillaAreas(CurrentGorilla->GetIndex(), true);
    std::vector<WorldArea> BoulderAreas = GameListener::GetBoulderAreas();

    auto ModifiedCollisionFlags = CollisionFlags;
    for (const auto& W : GorillaAreas)
    {
        for (int X = 0; X < W.GetWidth(); X++)
        {
            for (int Y = 0; Y < W.GetHeight(); Y++)
            {
                Tile SceneT(W.GetSceneX() + X, W.GetSceneY() + Y);
                if (SceneT.IsPositive()
                    && SceneT.X < ModifiedCollisionFlags.size()
                    && SceneT.Y < ModifiedCollisionFlags[SceneT.X].size())
                    ModifiedCollisionFlags[SceneT.X][SceneT.Y] |= 0x20000; // BLOCK_LINE_OF_SIGHT_FULL
            }
        }
    }

    constexpr int CULL_LINE_OF_SIGHT_RANGE = 5;
    WorldArea Area = CurrentGorilla->GetWorldArea();
    auto FacingDirection = CurrentGorilla->GetFacingDirection();

    for (int X = Area.GetX() - CULL_LINE_OF_SIGHT_RANGE; X <= Area.GetX() + CULL_LINE_OF_SIGHT_RANGE; X++)
    {
        for (int Y = Area.GetY() - CULL_LINE_OF_SIGHT_RANGE; Y <= Area.GetY() + CULL_LINE_OF_SIGHT_RANGE; Y++)
        {
            if (X == Area.GetX() && Y == Area.GetY())
                continue;

            // check against local player
            if (X == PlayerPos.X && Y == PlayerPos.Y)
                continue;

            if (X - ClientX < 0 || X - ClientX > 103) continue;
            if (Y - ClientY < 0 || Y - ClientY > 103) continue;
            auto Flag = CollisionFlags[X - ClientX][Y - ClientY];
            if ((Flag == Pathfinding::CLOSED) || (Flag & Pathfinding::BLOCKED) || (Flag & Pathfinding::OCCUPIED) || (Flag & Pathfinding::SOLID))
                continue; // remove tiles that are blocked

            WorldArea Next(Tile(X, Y, Area.GetPlane()));

            // check against current gorilla
            if (Area.IntersectsWith(Next))
                continue;

            // check against boulders
            if (std::any_of(BoulderAreas.begin(), BoulderAreas.end(), [&](const WorldArea& B) -> bool
            { return B.IntersectsWith(Next); }))
                continue;
            // check against gorillas
            if (std::any_of(GorillaAreas.begin(), GorillaAreas.end(), [&](const WorldArea& G) -> bool
            { return G.IntersectsWith(Next); }))
                continue;
            // check against players
            if (std::any_of(PlayerAreas.begin(), PlayerAreas.end(), [&](const WorldArea& P) -> bool
            { return P.IntersectsWith(Next); }))
                continue;

            if (Area.DistanceTo(Next) < Globals::Gorillas::MAX_ATTACK_RANGE && Area.HasLineOfSightTo(Next, ModifiedCollisionFlags))
            {
                auto PredictedNewArea = CurrentGorilla->GetNextTravelingPoint(Next, GorillaAreas, PlayerAreas);
                if (PredictedNewArea)
                {
                    auto PredictedTile = PredictedNewArea.AsTile();
                    if (PredictedTile.DistanceFrom(Area.AsTile()) >= 1.00) // Can path to 'next'
                    {
                        std::int32_t TileDirection = -1;
                        const auto Diff = Next.AsTile() - Area.AsTile();
                        if (Diff.X == 0 && Diff.Y > 0) TileDirection = Gorilla::NORTH;
                        if (Diff.X > 0 && Diff.Y > 0) TileDirection = Gorilla::NORTH_EAST;
                        if (Diff.X > 0 && Diff.Y == 0) TileDirection = Gorilla::EAST;
                        if (Diff.X > 0 && Diff.Y < 0) TileDirection = Gorilla::SOUTH_EAST;
                        if (Diff.X == 0 && Diff.Y < 0) TileDirection = Gorilla::SOUTH;
                        if (Diff.X < 0 && Diff.Y < 0) TileDirection = Gorilla::SOUTH_WEST;
                        if (Diff.X < 0 && Diff.Y == 0) TileDirection = Gorilla::WEST;
                        if (Diff.X < 0 && Diff.Y > 0) TileDirection = Gorilla::NORTH_WEST;

                        bool InFront = false;
                        switch (FacingDirection)
                        {
                            case Gorilla::NORTH:
                                InFront = TileDirection != Gorilla::SOUTH_EAST
                                          && TileDirection != Gorilla::SOUTH
                                          && TileDirection != Gorilla::SOUTH_WEST;
                                break;

                            case Gorilla::NORTH_EAST:
                                InFront = TileDirection != Gorilla::NORTH_WEST
                                          && TileDirection != Gorilla::WEST
                                          && TileDirection != Gorilla::SOUTH_WEST
                                          && TileDirection != Gorilla::SOUTH
                                          && TileDirection != Gorilla::SOUTH_EAST;
                                break;

                            case Gorilla::EAST:
                                InFront = TileDirection != Gorilla::NORTH_WEST
                                          && TileDirection != Gorilla::WEST
                                          && TileDirection != Gorilla::SOUTH_WEST;
                                break;

                            case Gorilla::SOUTH_EAST:
                                InFront = TileDirection != Gorilla::NORTH_EAST
                                          && TileDirection != Gorilla::NORTH
                                          && TileDirection != Gorilla::NORTH_WEST
                                          && TileDirection != Gorilla::WEST
                                          && TileDirection != Gorilla::SOUTH_WEST;
                                break;

                            case Gorilla::SOUTH:
                                InFront = TileDirection != Gorilla::NORTH
                                          && TileDirection != Gorilla::NORTH_WEST
                                          && TileDirection != Gorilla::NORTH_EAST;
                                break;

                            case Gorilla::SOUTH_WEST:
                                InFront = TileDirection != Gorilla::SOUTH_EAST
                                          && TileDirection != Gorilla::EAST
                                          && TileDirection != Gorilla::NORTH_EAST
                                          && TileDirection != Gorilla::NORTH
                                          && TileDirection != Gorilla::NORTH_WEST;
                                break;

                            case Gorilla::WEST:
                                InFront = TileDirection != Gorilla::NORTH_EAST
                                          && TileDirection != Gorilla::EAST
                                          && TileDirection != Gorilla::SOUTH_EAST;
                                break;

                            case Gorilla::NORTH_WEST:
                                InFront = TileDirection != Gorilla::SOUTH_WEST
                                          && TileDirection != Gorilla::SOUTH
                                          && TileDirection != Gorilla::SOUTH_EAST
                                          && TileDirection != Gorilla::EAST
                                          && TileDirection != Gorilla::NORTH_EAST;
                                break;

                            default: break;
                        }
                        Tiles.emplace_back(std::make_pair(InFront, std::move(Next)));
                    }
                }
            }
        }
    }
    return Tiles;
}

Tile Gorillas::GetMeleeMoveTile(double Distance)
{
    auto ViableTiles = Gorillas::GetValidMoveTiles();
    if (ViableTiles.empty()) return Tile();

    auto Gorilla = GameListener::GetCurrentGorilla();
    if (!Gorilla || !*Gorilla) return Tile();

    auto PlayerArea = WorldArea(Internal::GetLocalPlayer());
    auto GorillaArea = Gorilla->GetWorldArea();

    std::int32_t MaxPlayerDistance = 4;
    std::int32_t MaxGorillaDistance = 4;
    std::int32_t MinPlayerDistance = 2;
    std::int32_t MinGorillaDistance = 3;

    auto Sort = [&](const std::pair<bool, WorldArea>& A, const std::pair<bool, WorldArea>& B) -> bool
    {
        if (A.first == B.first)
            return Mainscreen::GetProjectedDistance(A.second.AsTile(), PlayerArea.AsTile()) < Mainscreen::GetProjectedDistance(B.second.AsTile(), PlayerArea.AsTile());
        return A.first && !B.first;
    };

    auto Find_Distance = [&](const std::pair<bool, WorldArea>& P) -> bool
    {
        auto PlayerDist = P.second.DistanceTo(PlayerArea);
        auto GorillaDist = P.second.DistanceTo(GorillaArea);

        return PlayerDist >= MinPlayerDistance && PlayerDist <= MaxPlayerDistance
           && GorillaDist >= MinGorillaDistance && GorillaDist <= MaxGorillaDistance;
    };

    auto Find_InFront = [&](const std::pair<bool, WorldArea>& P) -> bool
    {
        return P.first && Find_Distance(P);
    };

    std::sort(ViableTiles.begin(), ViableTiles.end(), Sort);

    bool TryFurther = false;
    if (TryFurther)
    {
        auto Found_InFront = std::find_if(ViableTiles.rbegin(), ViableTiles.rend(), Find_InFront);
        if (Found_InFront != ViableTiles.rend())
            return Found_InFront->second.AsTile();

        auto Found_Distance = std::find_if(ViableTiles.rbegin(), ViableTiles.rend(), Find_Distance);
        if (Found_Distance != ViableTiles.rend())
            return Found_Distance->second.AsTile();
    } else
    {
        auto Found_InFront = std::find_if(ViableTiles.begin(), ViableTiles.end(), Find_InFront);
        if (Found_InFront != ViableTiles.end())
            return Found_InFront->second.AsTile();

        auto Found_Distance = std::find_if(ViableTiles.begin(), ViableTiles.end(), Find_Distance);
        if (Found_Distance != ViableTiles.end())
            return Found_Distance->second.AsTile();
    }

    MinPlayerDistance = 1;
    MinGorillaDistance = 2;

    auto Found_InFront = std::find_if(ViableTiles.begin(), ViableTiles.end(), Find_InFront);
    if (Found_InFront != ViableTiles.end())
        return Found_InFront->second.AsTile();

    auto Found_Distance = std::find_if(ViableTiles.begin(), ViableTiles.end(), Find_Distance);
    if (Found_Distance != ViableTiles.end())
        return Found_Distance->second.AsTile();

    return Tile();
}

Tile Gorillas::GetMeleeBoulderMoveTile(double Distance)
{
    std::shared_ptr<Gorilla> CurrentGorilla = GameListener::GetCurrentGorilla();
    if (!CurrentGorilla || !*CurrentGorilla) return Tile();

    auto GorillaArea = CurrentGorilla->GetWorldArea();
    auto ValidMovements = Gorillas::GetValidMovementAreas();

    std::vector<WorldArea> BoulderAreas = GameListener::GetBoulderAreas();
    std::vector<WorldArea> GorillaAreas = GameListener::GetGorillaAreas(CurrentGorilla->GetIndex(), true);

    ValidMovements.erase(
            std::remove_if(ValidMovements.begin(), ValidMovements.end(), [&](const WorldArea& A) -> bool
            {
                return std::any_of(GorillaAreas.begin(), GorillaAreas.end(), [&](const WorldArea& GA) -> bool { return GA.IntersectsWith(A); })
                       || std::any_of(BoulderAreas.begin(), BoulderAreas.end(), [&](const WorldArea& BA) -> bool { return BA.IntersectsWith(A); });;
            }), ValidMovements.end());

    for (const auto& ValidMovement : ValidMovements)
    {
        if (!ValidMovement.IntersectsWith(GorillaArea) && ValidMovement.IsInMeleeDistance(GorillaArea))
            return ValidMovement.AsTile();
    }

    for (const auto& ValidMovement : ValidMovements)
    {
        if (!ValidMovement.IntersectsWith(GorillaArea))
            return ValidMovement.AsTile();
    }

    return Tile();
}

Tile Gorillas::GetRangedBoulderMoveTile(double Distance)
{
    auto ValidMovements = Gorillas::GetValidMovementAreas();

    return Tile();
}

bool Gorillas::SwitchPrayer(Prayer::PRAYERS Prayer)
{
    if (Prayer::IsActive(Prayer)) return true;

    Profile::Push(Profile::Var_RawInteractableMean, 125);
    Profile::Push(Profile::Var_RawMoveMean, 75);
    Profile::Push(Profile::Var_RawMouseUpMean, 0);

    if (Prayer::Open(Profile::RollUseGametabHotKey()))
    {
        if (!Antiban::Tasks.count("GORILLAS_SWITCH_PRAYER"))
        {
            Antiban::Task T;
            switch (Profile::GetInt(Profile::Var_Passivity))
            {
                case Profile::PASSIVITY_EXHILARATED: // 40-60 times an hour
                {

                } break;

                case Profile::PASSIVITY_HYPER: // 30-45 times an hour
                {

                } break;

                case Profile::PASSIVITY_MILD: // switches to wrong prayer 15-25 times an hour
                {

                } break;

                case Profile::PASSIVITY_MELLOW:
                case Profile::PASSIVITY_DISINTERESTED: // switches to wrong prayer 0-10 times an hour
                {

                } break;
            }
            Antiban::AddTask("GORILLAS_SWITCH_PRAYER", std::move(T));
        }

        bool SwitchToWrongPrayer = Antiban::RunTask("GORILLAS_SWITCH_PRAYER");
    }

    return false;
}

bool Gorillas::WalkTo()
{
    if (!Travel::InCavern()) return false;
    if (Travel::GetLocation() == Travel::CRASH_SITE_CAVERN_INNER) return true;


    return true;
}

bool Gorillas::Fight()
{
    //if (Travel::GetLocation() != Travel::CRASH_SITE_CAVERN_INNER) return false;

    std::shared_ptr<Gorilla> Gorilla = GameListener::GetCurrentGorilla();
    if (!Gorilla || !*Gorilla) return false;

    std::string Text;
    std::int32_t State = Gorillas::GetState();

    if (State & Gorillas::SWITCH_PRAYER_MELEE) Text += "SWITCH_PRAYER_MELEE\n";
    if (State & Gorillas::SWITCH_PRAYER_RANGED) Text += "SWITCH_PRAYER_RANGED\n";
    if (State & Gorillas::SWITCH_PRAYER_MAGIC) Text += "SWITCH_PRAYER_MAGIC\n";
    //if (State & Gorillas::SINGLE_SWITCH_PRAYER) Text += "SINGLE_SWITCH_PRAYER\n";
    if (State & Gorillas::EQUIP_MELEE) Text += "EQUIP_MELEE\n";
    if (State & Gorillas::MELEE_MOVE) Text += "MELEE_MOVE\n";
    if (State & Gorillas::EQUIP_RANGED) Text += "EQUIP_RANGED\n";
    if (State & Gorillas::EQUIP_SPECIAL) Text += "EQUIP_SPECIAL\n";
    if (State & Gorillas::BOULDER) Text += "BOULDER\n";
    Paint::DrawString(Text, Internal::TileToMainscreen(Minimap::GetPosition(), 0, 0, 0) + Point(20, 0), 0, 255, 255, 255);

    std::vector<WorldArea> BoulderAreas = GameListener::GetBoulderAreas();
    std::vector<WorldArea> GorillaAreas = GameListener::GetGorillaAreas(Gorilla->GetIndex(), true);
    std::vector<WorldArea> PlayerAreas = GameListener::GetPlayerAreas(true);

    if (State & MELEE_MOVE)
    {
        /*auto ViableTiles = Gorillas::GetValidMoveTiles();
        if (!ViableTiles.empty())
        {
            auto PlayerArea = WorldArea(Internal::GetLocalPlayer());
            auto GorillaArea = Gorilla->GetWorldArea();

            std::int32_t MinPlayerDistance = 2;
            std::int32_t MinGorillaDistance = 3;

            auto Sort = [&](const std::pair<bool, WorldArea>& A, const std::pair<bool, WorldArea>& B) -> bool
            {
                if (A.first == B.first)
                    return Mainscreen::GetProjectedDistance(A.second.AsTile(), PlayerArea.AsTile()) < Mainscreen::GetProjectedDistance(B.second.AsTile(), PlayerArea.AsTile());
                return A.first && !B.first;
            };

            std::sort(ViableTiles.begin(), ViableTiles.end(), Sort);
            for (std::uint32_t I = 0; I < ViableTiles.size(); I++)
            {
                std::string TextStr = std::to_string(ViableTiles[I].second.DistanceTo(PlayerArea)) + " | " + std::to_string(ViableTiles[I].second.DistanceTo(GorillaArea))
                                                                                                                            + "\n[" + std::to_string(I) + "]";
                Paint::DrawTile(ViableTiles[I].second.AsTile(), ViableTiles[I].first ? 0 : 255, ViableTiles[I].first ? 255 : 0, 0, 255);
                Paint::DrawString(TextStr, Internal::TileToMainscreen(ViableTiles[I].second.AsTile(), 0, 0, 0), 0, 255, 255, 255);
            }
        }*/


        static Tile MeleeMoveTile;

        auto NextTravelingPoint = Gorilla->GetNextTravelingPoint(Mainscreen::GetTrueLocation(), GorillaAreas, PlayerAreas);
        auto NextTravelingTile = NextTravelingPoint.AsTile();

        if (!NextTravelingTile || NextTravelingTile.DistanceFrom(Gorilla->GetTrueLocation()) == 0.00)
        {
            if (!MeleeMoveTile)
                MeleeMoveTile = Gorillas::GetMeleeMoveTile(1.00);
            Paint::DrawTile(MeleeMoveTile, 0, 255, 0, 255);
        } else
            MeleeMoveTile = Tile();
    }

    static Tile BoulderMeleeMoveTile;
    if (State & BOULDER)
    {
        if (!BoulderMeleeMoveTile)
            BoulderMeleeMoveTile = Gorillas::GetMeleeBoulderMoveTile(1.00);
        Paint::DrawTile(BoulderMeleeMoveTile, 255, 255, 0, 255);
    } else
        BoulderMeleeMoveTile = Tile();

    Gorilla->Draw(true);

/*    auto MoveTiles = Gorillas::GetValidMoveTiles();
    for (const auto& T : MoveTiles)
    {
        Paint::DrawTile(T.second.AsTile(), T.first ? 0 : 255, T.first ? 255 : 0, 0, 255);
        Paint::DrawString(std::to_string(T.second.DistanceTo(WorldArea(Internal::GetLocalPlayer()))) + " | " +
                            std::to_string(T.second.DistanceTo(Gorilla->GetWorldArea())),
                          Internal::TileToMainscreen(T.second.AsTile(), 0, 0, 0), 0, 255, 0, 255);
    }*/

    auto NextPoint = WorldArea(Internal::GetLocalPlayer()).CalculateNextTravellingPoint(Gorilla->GetWorldArea(), true);
    if (NextPoint) Paint::DrawTile(NextPoint.AsTile(), 255, 0, 255, 255);
    //Paint::DrawTile(GetMeleeBoulderMoveTile(1.00), 255, 0, 255, 255);

    auto ViableTiles = Gorillas::GetValidMoveTiles();
    for (const auto& [Front, T] : ViableTiles )
        Paint::DrawTile(T.AsTile(), Front ? 0 : 255, Front ? 255 : 0, 0, 255);

    return true;
}

bool Gorillas::MeleeMove(std::int32_t& State, const std::shared_ptr<Gorilla>& Gorilla)
{
    Tile MeleeMoveTile;
    while (State & MELEE_MOVE)
    {
        /*
         * After clicking the move tile, sometimes open the prayer menu as if to be ready
         */
        std::vector<WorldArea> PlayerAreas = GameListener::GetPlayerAreas();
        std::vector<WorldArea> GorillaAreas = GameListener::GetGorillaAreas(Gorilla->GetIndex(), true);

        auto NextTravelingPoint = Gorilla->GetNextTravelingPoint(Mainscreen::GetTrueLocation(), GorillaAreas, PlayerAreas);
        auto NextTravelingTile = NextTravelingPoint.AsTile();

        if (!NextTravelingTile || NextTravelingTile.DistanceFrom(Gorilla->GetTrueLocation()) == 0.00)
        {
            if (!MeleeMoveTile)
                MeleeMoveTile = Gorillas::GetMeleeMoveTile(2.00);
        }

        State = Gorillas::GetState();
    }
    return true;
}

bool Gorillas::BoulderMove(int32_t& State, const std::shared_ptr<Gorilla>& Gorilla)
{
    /*
     * if we are equipped for melee, and the next traveling point will not be the boulder tile, then just click attack
     * if we are equipped ranged, but we need to switch to melee, switch to melee after clicking to move, but we need to generate a tile that can path safely to melee range if possible
     *      we could switch to melee before moving, and we should sometimes for sake of anti-bot
     * if we are equipped ranged, and we don't need to switch to melee, randomly decide to generate a tile > 1 distance away from the gorilla, efficiently be ready for MELEE_MOVE
     *      if we are already > 2 away, pick a tile within attack range, preferably in front of the gorilla (the ones with the lowest distance)
     *
     */


    while (State & MELEE_MOVE)
    {

    }
    //State = Gorillas::GetState();
    return false;
}

bool Gorillas::Prayers(int32_t& State, const std::shared_ptr<Gorilla>& Gorilla)
{
    Profile::Push(Profile::Var_RawInteractableMean, 125);
    Profile::Push(Profile::Var_RawMoveMean, 75);
    Profile::Push(Profile::Var_RawMouseUpMean, 0);

    if (Gorilla->CountNextPossibleAttackStyles() == 1)
    {
        if (State & SWITCH_PRAYER_MELEE) return Gorillas::SwitchPrayer(Prayer::PROTECT_FROM_MELEE);
        if (State & SWITCH_PRAYER_RANGED) return Gorillas::SwitchPrayer(Prayer::PROTECT_FROM_MISSILES);
        if (State & SWITCH_PRAYER_MAGIC) return Gorillas::SwitchPrayer(Prayer::PROTECT_FROM_MAGIC);
    } else
    {
        if (State & SWITCH_PRAYER_MELEE) return false; // Can't happen
        if (State & SWITCH_PRAYER_RANGED && State & SWITCH_PRAYER_MAGIC)
        {
            // Already praying one of the two
            if (Prayer::IsActive(Prayer::PROTECT_FROM_MISSILES)) return true;
            if (Prayer::IsActive(Prayer::PROTECT_FROM_MAGIC)) return true;

            std::int32_t MagicDefense = Gorillas::GetDefenseAgainst(MAGIC_FLAG);
            std::int32_t RangeDefense = Gorillas::GetDefenseAgainst(RANGED_FLAG);

            // Set to weaker defense prayer
            if (MagicDefense < RangeDefense)
                return Gorillas::SwitchPrayer(Prayer::PROTECT_FROM_MAGIC);
            return Gorillas::SwitchPrayer(Prayer::PROTECT_FROM_MISSILES);
        }
    }
    return false;
}