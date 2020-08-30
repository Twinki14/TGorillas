#include <TScript.hpp>
#include <Utilities/GearSets.hpp>
#include <Utilities/Mainscreen.hpp>
#include <Utilities/TProfile.hpp>
#include <Utilities/Antiban.hpp>
#include <Utilities/Prayer.hpp>
#include <Game/Core.hpp>
#include <Tools/OSRSBox/Items.hpp>
#include "Gorillas.hpp"
#include "Travel.hpp"
#include "Supplies.hpp"
#include "Listeners/GameListener.hpp"
#include "../Config.hpp"

std::int32_t Gorillas::GetState()
{
    auto Gorilla = GameListener::GetCurrentGorilla();

    if (!Gorilla || !*Gorilla || Gorilla->IsDead())
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

std::string Gorillas::GetStateString(std::int32_t State)
{
    if (State < 0) State = Gorillas::GetState();

    std::string Str;
    if (State & Gorillas::MELEE_MOVE) Str += "MELEE_MOVE\n";
    if (State & Gorillas::BOULDER) Str += "BOULDER\n";
    if (State & Gorillas::SWITCH_PRAYER_MELEE) Str += "SWITCH_PRAYER_MELEE\n";
    if (State & Gorillas::SWITCH_PRAYER_RANGED) Str += "SWITCH_PRAYER_RANGED\n";
    if (State & Gorillas::SWITCH_PRAYER_MAGIC) Str += "SWITCH_PRAYER_MAGIC\n";
    if (State & Gorillas::EQUIP_MELEE) Str += "EQUIP_MELEE\n";
    if (State & Gorillas::EQUIP_RANGED) Str += "EQUIP_RANGED\n";
    if (State & Gorillas::EQUIP_SPECIAL) Str += "EQUIP_SPECIAL\n";
    return Str;
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
                case Config::MAGIC_SHORTBOW: return RANGED_FLAG; break;
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
                    case Config::MAGIC_SHORTBOW: return RANGED_FLAG; break;
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

std::vector<std::pair<bool, WorldArea>> Gorillas::GetValidMoveTiles(const std::shared_ptr<Gorilla>& Gorilla)
{
    std::vector<std::pair<bool, WorldArea>> Tiles;
    if (!Gorilla || !*Gorilla) return Tiles;

    const auto PlayerPos = Mainscreen::GetTrueLocation();

    auto ClientX = Internal::GetClientX();
    auto ClientY = Internal::GetClientY();
    auto CollisionFlags = Internal::GetCollisionMap(PlayerPos.Plane).GetFlags();

    std::vector<WorldArea> PlayerAreas = GameListener::GetPlayerAreas(false);
    std::vector<WorldArea> GorillaAreas = GameListener::GetGorillaAreas(Gorilla->GetIndex(), true);
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
    WorldArea Area = Gorilla->GetWorldArea();
    auto FacingDirection = Gorilla->GetFacingDirection();

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
                auto PredictedNewArea = Gorilla->GetNextTravelingPoint(Next, GorillaAreas, PlayerAreas);
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
                                InFront = TileDirection != Gorilla::WEST
                                          && TileDirection != Gorilla::SOUTH_WEST
                                          && TileDirection != Gorilla::SOUTH;
                                break;

                            case Gorilla::EAST:
                                InFront = TileDirection != Gorilla::NORTH_WEST
                                          && TileDirection != Gorilla::WEST
                                          && TileDirection != Gorilla::SOUTH_WEST;
                                break;

                            case Gorilla::SOUTH_EAST:
                                InFront = TileDirection != Gorilla::NORTH
                                          && TileDirection != Gorilla::NORTH_WEST
                                          && TileDirection != Gorilla::WEST;
                                break;

                            case Gorilla::SOUTH:
                                InFront = TileDirection != Gorilla::NORTH
                                          && TileDirection != Gorilla::NORTH_WEST
                                          && TileDirection != Gorilla::NORTH_EAST;
                                break;

                            case Gorilla::SOUTH_WEST:
                                InFront = TileDirection != Gorilla::EAST
                                          && TileDirection != Gorilla::NORTH_EAST
                                          && TileDirection != Gorilla::NORTH;
                                break;

                            case Gorilla::WEST:
                                InFront = TileDirection != Gorilla::NORTH_EAST
                                          && TileDirection != Gorilla::EAST
                                          && TileDirection != Gorilla::SOUTH_EAST;
                                break;

                            case Gorilla::NORTH_WEST:
                                InFront = TileDirection != Gorilla::SOUTH
                                          && TileDirection != Gorilla::SOUTH_EAST
                                          && TileDirection != Gorilla::EAST;
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

Tile Gorillas::GetMeleeMoveTile(const std::shared_ptr<Gorilla>& Gorilla)
{
    if (!Gorilla || !*Gorilla) return Tile();
    auto ViableTiles = Gorillas::GetValidMoveTiles(Gorilla);
    if (ViableTiles.empty()) return Tile();

    auto PlayerArea = WorldArea(Internal::GetLocalPlayer());
    auto GorillaArea = Gorilla->GetWorldArea();

    std::int32_t MaxPlayerDistance = 4;
    std::int32_t MaxGorillaDistance = 4;
    std::int32_t MinPlayerDistance = 1;
    std::int32_t MinGorillaDistance = 2;

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

Tile Gorillas::GetBoulderMoveTile(const std::shared_ptr<Gorilla>& Gorilla)
{
    if (!Gorilla || !*Gorilla) return Tile();

    auto GorillaArea = Gorilla->GetWorldArea();
    auto ValidMovements = Gorillas::GetValidMovementAreas();

    std::vector<WorldArea> BoulderAreas = GameListener::GetBoulderAreas();
    std::vector<WorldArea> GorillaAreas = GameListener::GetGorillaAreas(Gorilla->GetIndex(), true);

    ValidMovements.erase(
            std::remove_if(ValidMovements.begin(), ValidMovements.end(), [&](const WorldArea& A) -> bool
            {
                return std::any_of(GorillaAreas.begin(), GorillaAreas.end(), [&](const WorldArea& GA) -> bool { return GA.IntersectsWith(A); })
                       || std::any_of(BoulderAreas.begin(), BoulderAreas.end(), [&](const WorldArea& BA) -> bool { return BA.IntersectsWith(A); });
            }), ValidMovements.end());

    if (Gorillas::GetEquippedWeaponStyle() == MELEE_FLAG)
    {
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
    } else // RANGED
    {
        bool SomeInMelee = std::any_of(ValidMovements.begin(), ValidMovements.end(), [&](const WorldArea& W) -> bool
        {
            return W.IntersectsWith(GorillaArea) || W.IsInMeleeDistance(GorillaArea);
        });

        if (SomeInMelee) // Generate tile away from gorilla
        {
            auto Sort = [&](const WorldArea& WA, const WorldArea& WB) -> bool
            {
                return WA.DistanceTo(GorillaArea) > WB.DistanceTo(GorillaArea);
            };
            std::sort(ValidMovements.begin(), ValidMovements.end(), Sort);

            for (const auto& ValidMovement : ValidMovements)
            {
                if (!ValidMovement.IntersectsWith(GorillaArea) && ValidMovement.HasLineOfSightTo(GorillaArea))
                    return ValidMovement.AsTile();
            }

            for (const auto& ValidMovement : ValidMovements)
            {
                if (!ValidMovement.IntersectsWith(GorillaArea))
                    return ValidMovement.AsTile();
            }
        } else // Generate tile towards gorilla
        {
            auto Sort = [&](const WorldArea& WA, const WorldArea& WB) -> bool
            {
                return WA.DistanceTo(GorillaArea) < WB.DistanceTo(GorillaArea);
            };
            std::sort(ValidMovements.begin(), ValidMovements.end(), Sort);

            for (const auto& ValidMovement : ValidMovements)
            {
                if (!ValidMovement.IntersectsWith(GorillaArea) && ValidMovement.HasLineOfSightTo(GorillaArea))
                    return ValidMovement.AsTile();
            }

            for (const auto& ValidMovement : ValidMovements)
            {
                if (!ValidMovement.IntersectsWith(GorillaArea))
                    return ValidMovement.AsTile();
            }
        }
    }
    return Tile();
}

Tile Gorillas::GetGorillaMoveTile(const std::shared_ptr<Gorilla>& Gorilla)
{
    if (!Gorilla || !*Gorilla) return Tile();

    auto ValidMovements = Gorilla->GetSurroundingMovementAreas();

    WorldArea PlayerArea(Internal::GetLocalPlayer());
    std::vector<WorldArea> BoulderAreas = GameListener::GetBoulderAreas();
    std::vector<WorldArea> GorillaAreas = GameListener::GetGorillaAreas(Gorilla->GetIndex(), true);

    ValidMovements.erase(
            std::remove_if(ValidMovements.begin(), ValidMovements.end(), [&](const WorldArea& A) -> bool
            {
                return std::any_of(GorillaAreas.begin(), GorillaAreas.end(), [&](const WorldArea& GA) -> bool { return GA.IntersectsWith(A); })
                       || std::any_of(BoulderAreas.begin(), BoulderAreas.end(), [&](const WorldArea& BA) -> bool { return BA.IntersectsWith(A); });
            }), ValidMovements.end());

    if (!ValidMovements.empty())
    {
        auto Sort = [&](const WorldArea& AA, const WorldArea& AB) -> bool { return AA.AsTile().DistanceFrom(PlayerArea.AsTile()) < AB.AsTile().DistanceFrom(PlayerArea.AsTile()); };
        std::sort(ValidMovements.begin(), ValidMovements.end(), Sort);
        return ValidMovements.front().AsTile();
    }
    return Tile();
}

bool Gorillas::IsAttacking()
{
    auto Current = GameListener::GetCurrentGorilla();
    return Gorillas::IsAttacking(Current);
}

bool Gorillas::IsAttacking(const std::shared_ptr<Gorilla>& Gorilla)
{
    auto Player = Players::GetLocal();
    if (!Gorilla || !*Gorilla || !Player) return false;

    auto Interacting = Player.GetInteracting();
    if (!Interacting) return false;

    auto Interacting_NPC = ((Interacting && Interacting.InstanceOf(Internal::NPC::GetClass())) ? (Internal::NPC((Internal::Object) Interacting)) : (Internal::NPC(nullptr)));
    if (!Interacting_NPC) return false;

    auto Interacting_Interactable = Interactable::NPC(Interacting_NPC);
    auto InteractableGorilla = Interactable::NPC(*Gorilla);
    return Interacting_Interactable && Interacting_Interactable == InteractableGorilla;
}

bool Gorillas::ShouldLeave()
{
    return false;
}

bool Gorillas::ShouldInterrupt()
{
    return Gorillas::GetState() & BOULDER;
}

bool Gorillas::AdjustCamera()
{
    // TODO
    return false;
}

bool Gorillas::Attack( const std::shared_ptr<Gorilla>& Gorilla, bool Force, bool Wait)
{
    if (!Gorilla || !*Gorilla || Gorilla->IsDead()) return false;
    if (Force || !Gorillas::IsAttacking(Gorilla))
    {
        //if (!Force && Vorkath::GetTimeSinceLastAttack() < 1600) return false;

        Script::SetStatus("Attacking");
        Profile::Push(Profile::Var_RawMoveMean, 40);
        Profile::Push(Profile::Var_RawMoveDeviation, 0.01);
        Profile::Push(Profile::Var_RawMouseDownMean, 35);
        Profile::Push(Profile::Var_RawMouseDownDeviation, 0.01);
        Profile::Push(Profile::Var_RawMouseUpMean, 0);
        const auto GorillaPoint = Gorilla->GetConvex().GetMiddle();
        bool Result = Interact::Click(GorillaPoint, "Attack");
        Profile::Pop(5);

        if (!Result) Gorillas::AdjustCamera();
        if (Wait) return Result && WaitFunc(450, 25, [&]() -> bool { return Gorillas::IsAttacking(Gorilla); } , true);
        return Result;
    }
    return false;
}

bool Gorillas::SwitchPrayer(Prayer::PRAYERS Prayer, bool Force)
{
    if (Prayer::IsActive(Prayer)) return true;

    Profile::Push(Profile::Var_RawInteractableMean, 125);
    Profile::Push(Profile::Var_RawMoveMean, 75);
    Profile::Push(Profile::Var_RawMouseUpMean, 0);

    if (Prayer::Open(Profile::RollUseGametabHotKey()))
    {
        static auto LastPassivity = Profile::GetInt(Profile::Var_Passivity);
        auto Passivity = Profile::GetInt(Profile::Var_Passivity);
        if (!Antiban::Tasks.count("GORILLAS_SWITCH_PRAYER") || LastPassivity != Passivity)
        {
            Antiban::Task T;
            LastPassivity = Passivity;
            switch (Passivity)
            {
                // TODO Consider a new profile var - Focus maybe?
                case Profile::PASSIVITY_EXHILARATED: T = Antiban::Task(8000, 0.00, 0.08); break; // 30-45 times an hour
                case Profile::PASSIVITY_HYPER: T = Antiban::Task(8000, 0.00, 0.05); break; // 20-30 times an hour
                case Profile::PASSIVITY_MILD: T = Antiban::Task(8000, 0.00, 0.3); break; // 10-20 times an hour

                default:
                case Profile::PASSIVITY_MELLOW:
                case Profile::PASSIVITY_DISINTERESTED:  T = Antiban::Task(8000, 0.00, 0.01); break; // 0-10 times an hour
            }
            Antiban::Tasks.insert_or_assign("GORILLAS_SWITCH_PRAYER", std::move(T));
        }

        bool SwitchToWrongPrayer = false; //Antiban::RunTask("GORILLAS_SWITCH_PRAYER");

        bool Missclicked = false;
        if (!Force && SwitchToWrongPrayer)
        {
            std::vector<Prayer::PRAYERS> MissclickPrayers;
            switch (Prayer)
            {
                case Prayer::PROTECT_FROM_MELEE: MissclickPrayers = { Prayer::PROTECT_FROM_MISSILES, Prayer::PROTECT_FROM_MAGIC, Prayer::SMITE }; break;
                case Prayer::PROTECT_FROM_MISSILES: MissclickPrayers = { Prayer::PROTECT_FROM_MELEE, Prayer::PROTECT_FROM_MAGIC, Prayer::REDEMPTION, Prayer::RETRIBUTION, Prayer::SMITE }; break;
                case Prayer::PROTECT_FROM_MAGIC: MissclickPrayers = { Prayer::PROTECT_FROM_MISSILES, Prayer::PROTECT_FROM_MELEE, Prayer::RETRIBUTION }; break;
                default: break;
            }

            if (!MissclickPrayers.empty())
            {
                Prayer::PRAYERS P = MissclickPrayers[UniformRandom(0, MissclickPrayers.size() - 1)];
                DebugLog("Miss-clicking prayer > {}", P);
                Script::SetStatus("Miss-clicking prayer");
                Missclicked = Prayer::Activate(P);
                if (Missclicked)
                    Antiban::DelayFromPassivity(650, 2250, 1.8, 0.10);
            }
        }

        if (Missclicked)
            Script::SetStatus("Correcting miss-clicked prayer");
        else
            Script::SetStatus("Switching prayer");

        bool Result = Prayer::Activate(Prayer);
        Profile::Pop(3);
        return Result;
    }
    Profile::Pop(3);
    return false;
}

bool Gorillas::StopCasting()
{
    return false;
}

bool Gorillas::MeleeMove(std::int32_t& State, const std::shared_ptr<Gorilla>& Gorilla)
{
    CurrentMeleeMoveTile = std::make_shared<Tile>(Tile());

    static auto LastPassivity = Profile::GetInt(Profile::Var_Passivity);
    auto Passivity = Profile::GetInt(Profile::Var_Passivity);
    if (!Antiban::Tasks.count("GORILLAS_MELEE_MOVE_OPEN_PRAYER") || LastPassivity != Passivity)
    {
        Antiban::Task T;
        LastPassivity = Passivity;
        switch (Passivity)
        {
            // TODO Consider a new profile var - Focus maybe?
            // TODO This needs to be changed for this specific purpose, this is using values from switch prayer
            case Profile::PASSIVITY_EXHILARATED: T = Antiban::Task(8000, 0.00, 0.08); break; // 30-45 times an hour
            case Profile::PASSIVITY_HYPER: T = Antiban::Task(8000, 0.00, 0.05); break; // 20-30 times an hour
            case Profile::PASSIVITY_MILD: T = Antiban::Task(8000, 0.00, 0.3); break; // 10-20 times an hour

            default:
            case Profile::PASSIVITY_MELLOW:
            case Profile::PASSIVITY_DISINTERESTED:  T = Antiban::Task(8000, 0.00, 0.01); break; // 0-10 times an hour
        }
        Antiban::Tasks.insert_or_assign("GORILLAS_MELEE_MOVE_OPEN_PRAYER", std::move(T));
    }

    bool OpenPrayerBefore = Antiban::RunTask("GORILLAS_MELEE_MOVE_OPEN_PRAYER");
    bool OpenedPrayerBefore = false;

    Tile LocalMoveTile;
    bool Moved = false;
    while (State & MELEE_MOVE)
    {
        /*
         * After clicking the move tile, sometimes open the prayer menu as if to be ready
         */

        if (Terminate) return false;
        if (Combat::GetHealth() <= 0) break;
        if (!Gorilla || !*Gorilla) break;
        if (Gorilla->IsDead()) break;

/*        auto GetNextGorillaTravelPoint = [&]() -> Tile
        {
            std::vector<WorldArea> PlayerAreas = GameListener::GetPlayerAreas();
            std::vector<WorldArea> GorillaAreas = GameListener::GetGorillaAreas(Gorilla->GetIndex(), true);

            auto NextTravelingPoint = Gorilla->GetNextTravelingPoint(Mainscreen::GetTrueLocation(), GorillaAreas, PlayerAreas);
            return NextTravelingPoint.AsTile();
        };

        auto NextTravelingTile = GetNextGorillaTravelPoint();*/
        bool FarEnoughAway = Gorilla->GetWorldArea().DistanceTo(Mainscreen::GetTrueLocation()) >= 2; //NextTravelingTile && NextTravelingTile.DistanceFrom(Gorilla->GetTrueLocation()) >= 1.00;

        if (!FarEnoughAway)
        {
            if (!CurrentMeleeMoveTile || !*CurrentMeleeMoveTile || !LocalMoveTile)
            {
                LocalMoveTile = Gorillas::GetMeleeMoveTile(Gorilla);
                CurrentMeleeMoveTile = std::make_shared<Tile>(Tile(LocalMoveTile));
            }

            if (!LocalMoveTile)
                DebugLog("Failed to generate a tile!");

            if (LocalMoveTile && !Moved)
            {
                if (OpenPrayerBefore && !OpenedPrayerBefore)
                {
                    Prayer::Open(Profile::RollUseGametabHotKey());
                    OpenedPrayerBefore = true;
                }

                Moved = Mainscreen::ClickTileEx(LocalMoveTile);
            }

            if (Moved)
            {
                /*
                 * TODO Antipattern
                 * Have this use the FOCUS var or whatever instead
                 * - "Do we pray the 'at-ranged' prayer here? or wait until numstyles becomes 1"
                 */

                if (Gorilla->CountNextPossibleAttackStyles() > 1 && Prayer::Open(Profile::RollUseGametabHotKey()))
                {
                    if (Gorilla->NextPossibleAttackStyles & Gorilla::MAGIC_FLAG)
                        Gorillas::SwitchPrayer(Prayer::PROTECT_FROM_MAGIC);
                    else if (Gorilla->NextPossibleAttackStyles & Gorilla::RANGED_FLAG)
                        Gorillas::SwitchPrayer(Prayer::PROTECT_FROM_MISSILES);
                }
            }

        }
        State = Gorillas::GetState();
    }
    CurrentMeleeMoveTile = std::make_shared<Tile>(Tile());
    return true;
}

bool Gorillas::BoulderMove(int32_t& State, const std::shared_ptr<Gorilla>& Gorilla)
{
    /*
     * if we are equipped for melee, and the next traveling point will not be the boulder tile, then just click attack - DONE
     *
     * if we are equipped ranged, but we need to switch to melee, switch to melee after clicking to move, but we need to generate a tile that can path safely to melee range if possible
     *      we could switch to melee before moving, and we should sometimes for sake of anti-bot
     *
     * if we are equipped ranged, and we don't need to switch to melee, randomly decide to generate a tile > 1 distance away from the gorilla, efficiently be ready for MELEE_MOVE
     *      if we are already > 2 away, pick a tile within attack range, preferably in front of the gorilla (the ones with the lowest distance)  - DONE
     */

    CurrentBoulderMoveTile = std::make_shared<Tile>(Tile());
    Tile LocalMoveTile;
    bool Moved = false;
    while (State & BOULDER)
    {
        if (Terminate) return false;
        if (Combat::GetHealth() <= 0) break;
        if (!Gorilla || !*Gorilla) break;
        if (Gorilla->IsDead()) break;

        if (!CurrentBoulderMoveTile || !*CurrentBoulderMoveTile || !LocalMoveTile)
        {
            LocalMoveTile = Gorillas::GetBoulderMoveTile(Gorilla);
            CurrentBoulderMoveTile = std::make_shared<Tile>(Tile(LocalMoveTile));
        }

        if (!Moved)
        {
            // TODO Anti-pattern/Anti-ban, maybe have some of these more specific things happen randomly

            if (State & EQUIP_MELEE) // generate tile near gorilla, "walk to melee", equip melee while walking
            {
                
            }

            if (Gorillas::GetEquippedWeaponStyle() == MELEE_FLAG)
            {
                auto PlayerArea = WorldArea(Internal::GetLocalPlayer());
                auto NextPoint = PlayerArea.CalculateNextTravellingPoint(Gorilla->GetWorldArea(), true);
                if (NextPoint.DistanceTo(PlayerArea) > 0)
                {
                    if (Gorillas::Attack(Gorilla, true, true))
                    {
                        Moved = true;
                        continue;
                    }
                }
            }

            if (Mainscreen::ClickTileEx(LocalMoveTile, true))
            {
                Moved = true;
                continue;
            }

        } else
        {
            if (Gorillas::GetEquippedWeaponStyle() == MELEE_FLAG)
            {
                auto PlayerArea = WorldArea(Internal::GetLocalPlayer());
                auto NextPoint = PlayerArea.CalculateNextTravellingPoint(Gorilla->GetWorldArea(), true);

                if (!GameListener::AnyActiveBoulderOn(PlayerArea.AsTile()) && !GameListener::AnyActiveBoulderOn(NextPoint.AsTile()))
                    Gorillas::Attack(Gorilla, false, true);
            } else
            {
                if (!GameListener::AnyActiveBoulderOn(Mainscreen::GetTrueLocation()))
                    Gorillas::Attack(Gorilla, false, true);
            }
        }
        State = Gorillas::GetState();
    }
    CurrentBoulderMoveTile = std::make_shared<Tile>(Tile());
    return false;
}

bool Gorillas::Prayers(int32_t& State, const std::shared_ptr<Gorilla>& Gorilla)
{
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

bool Gorillas::Gear(int32_t& State, const std::shared_ptr<Gorilla>& Gorilla)
{
    if (State & EQUIP_MELEE)
    {
        if (GearSets::Sets.count("Melee"))
        {
            if (Gorilla && *Gorilla && Gorilla->InCombat())
            {
                static auto LastPassivity = Profile::GetInt(Profile::Var_Passivity);
                auto Passivity = Profile::GetInt(Profile::Var_Passivity);
                if (!Antiban::Tasks.count("GORILLAS_GEAR_MOVE_MELEE") || LastPassivity != Passivity)
                {
                    Antiban::Task T;
                    LastPassivity = Passivity;
                    switch (Passivity)
                    {
                        // TODO Consider a new profile var - Focus maybe?
                        case Profile::PASSIVITY_EXHILARATED: T = Antiban::Task(5000, 0.00, 0.75); break;
                        case Profile::PASSIVITY_HYPER: T = Antiban::Task(5000, 0.00, 0.65); break;
                        case Profile::PASSIVITY_MILD: T = Antiban::Task(5000, 0.00, 0.50); break;

                        default:
                        case Profile::PASSIVITY_MELLOW:
                        case Profile::PASSIVITY_DISINTERESTED:  T = Antiban::Task(5000, 0.00, 0.35); break;
                    }
                    Antiban::Tasks.insert_or_assign("GORILLAS_GEAR_MOVE_MELEE", std::move(T));
                }

                if (Gorilla->GetWorldArea().DistanceTo(Mainscreen::GetTrueLocation()) >= 2 && Antiban::RunTask("GORILLAS_GEAR_MOVE_MELEE"))
                {
                    auto GorillaTile = Gorillas::GetGorillaMoveTile(Gorilla);
                    if (GorillaTile && Mainscreen::ClickTileEx(GorillaTile, true))
                        return GearSets::Sets["Melee"].Equip(Gorillas::ShouldInterrupt);
                }
            }
            return GearSets::Sets["Melee"].Equip(Gorillas::ShouldInterrupt);
        }
        return false;
    }
    if (State & EQUIP_RANGED) return GearSets::Sets.count("Ranged") && GearSets::Sets["Ranged"].Equip(Gorillas::ShouldInterrupt);
    if (State & EQUIP_SPECIAL) return GearSets::Sets.count("Special") && GearSets::Sets["Special"].Equip(Gorillas::ShouldInterrupt);
    return false;
}

bool Gorillas::Special()
{
    // don't instantly activate when ready
    // use a antiban task, run every 10-30ss if CanSpecial
    switch (Config::Get("SpecialWeapon").as_integer<int>())
    {
        case Config::MAGIC_SHORTBOW: break;
        case Config::TOXIC_BLOWPIPE: break;
        case Config::SARADOMIN_GODSWORD: break;
        default: break;
    }
    return false;
}

bool Gorillas::Food()
{
    static const auto FoodCfg = (Food::FOOD) Config::Get("Food").as_integer<int>();
    static const auto FoodID = Food::GetItemID(FoodCfg);
    static const auto SharkID = Food::GetItemID(Food::SHARK);

    std::int32_t CurrentHealth = Stats::GetCurrentLevel(Stats::HITPOINTS);
    if (CurrentHealth <= 0) return false;

    static std::int32_t NextEatTick = -1;
    static std::int32_t NextCheck = NormalRandom(30, 34, 32, 32 * 0.06);

    if (CurrentHealth <= NextCheck)
    {
        auto Snapshot = Supplies::GetSnapshot(true);
        if (Snapshot.Food_Inv <= 0 && Snapshot.Sharks_Inv <= 0)
            return false;

        if (GameListener::GetTickCount() >= NextEatTick)
        {
            Gorillas::StopCasting();
            Script::SetStatus("Eating food");

            bool Clicked = false;
            if (Snapshot.Sharks_Inv > 0)
                Clicked = Food::QuickEat(Food::SHARK);
            else
                Clicked = Food::QuickEat(FoodCfg);

            if (Clicked)
            {
                // TODO Antipattern - Change NextEatTick to something lower for low-passivity/high focus
                static auto LastPassivity = Profile::GetInt(Profile::Var_Passivity);
                auto Passivity = Profile::GetInt(Profile::Var_Passivity);
                if (!Antiban::Tasks.count("GORILLAS_FOOD_SPAM") || !!Antiban::Tasks.count("GORILLAS_FOOD_EAT_TICK") || LastPassivity != Passivity)
                {
                    Antiban::Task Spam;
                    Antiban::Task LowNextEatTick;
                    LastPassivity = Passivity;
                    switch (Passivity)
                    {
                        // TODO Consider a new profile var - Focus maybe?
                        case Profile::PASSIVITY_EXHILARATED:
                        {
                            Spam = Antiban::Task(6000, 0.00, 0.65);
                            LowNextEatTick = Antiban::Task(8000, 0.00, 0.45);
                        } break;

                        case Profile::PASSIVITY_HYPER:
                        {
                            Spam = Antiban::Task(6000, 0.00, 0.55);
                            LowNextEatTick = Antiban::Task(8000, 0.00, 0.35);
                        } break;

                        case Profile::PASSIVITY_MILD:
                        {
                            Spam = Antiban::Task(6000, 0.00, 0.40);
                            LowNextEatTick = Antiban::Task(8000, 0.00, 0.25);
                        } break;

                        default:
                        case Profile::PASSIVITY_MELLOW:
                        case Profile::PASSIVITY_DISINTERESTED:
                        {
                            Spam = Antiban::Task(6000, 0.00, 0.25);
                            LowNextEatTick = Antiban::Task(8000, 0.00, 0.15);
                        } break;
                    }
                    Antiban::Tasks.insert_or_assign("GORILLAS_FOOD_SPAM", std::move(Spam));
                    Antiban::Tasks.insert_or_assign("GORILLAS_FOOD_EAT_TICK", std::move(LowNextEatTick));
                }

                bool SpamFood = Antiban::RunTask("GORILLAS_FOOD_SPAM");
                bool SetLowNextEatTick = Antiban::RunTask("GORILLAS_FOOD_EAT_TICK");

                if (SpamFood)
                {
                    Counter C(UniformRandom(3, 6));
                    while (C.Increment())
                    {
                        bool Ate = false;
                        if (Snapshot.Sharks_Inv > 0)
                            Ate = Food::QuickEat(Food::SHARK);
                        else
                            Ate = Food::QuickEat(FoodCfg);

                        if (Ate) Antiban::DelayFromPassivity(175, 450, 1.8, 0.10);
                        if (CurrentHealth > NextCheck) break;
                    }
                }

                NextEatTick = GameListener::GetTickCount() + 6;
                if (SetLowNextEatTick) NextEatTick = GameListener::GetTickCount() + UniformRandom(1, 4);
                return true;
            }
        }
        return true;
    } else
    {
        if (NextEatTick != -1)
        {
            NextEatTick = -1;
            // TODO Antiban - Rarely change this to something higher
            NextCheck = NormalRandom(30, 34, 32, 32 * 0.06);
        }
    }
    return false;
}

bool Gorillas::Restore()
{
    std::int32_t CurrentPrayer = Prayer::GetPoints();
    //static bool Trigger = false;
    static bool Trigger = false;
    static std::int32_t NextCheck = NormalRandom(8, 18, 14, 14 * 0.06);

    if (CurrentPrayer <= NextCheck)
    {
        if (Supplies::GetSnapshot(true).Potions_Inv_PrayerRestore.Total <= 0)
        {
            Trigger = false;
            return false;
        }

        Trigger = true;
        Script::SetStatus("Restoring prayer");
        return Prayer::QuickDrinkRestore();
    } else if (Trigger)
    {
        Trigger = false;
        NextCheck = NormalRandom(8, 18, 14, 14 * 0.06);
    }
    return false;
}

bool Gorillas::TopOff(double MaxOverheal, double MaxOverrestore)
{
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
    GameListener::Instance().Start();
    while (Travel::GetLocation() == Travel::CRASH_SITE_CAVERN_INNER)
    {
        if (Terminate) break;

        std::int32_t State = Gorillas::GetState();
        std::shared_ptr<Gorilla> Gorilla = GameListener::GetCurrentGorilla();
        if (!Gorilla || !*Gorilla) continue;
        if (Gorilla->IsDead()) continue;

        if (!Gorilla->InCombat())
        {
            if (IsKeyDown(KEY_F1))
                Gorillas::Attack(Gorilla, true, true);
            else
                Wait(250);
            continue;
        }

        if (State & MELEE_MOVE)
        {
            Gorillas::MeleeMove(State, Gorilla);
            State = Gorillas::GetState();
        }

        if (State & BOULDER)
        {
            Gorillas::BoulderMove(State, Gorilla);
            State = Gorillas::GetState();
        }

        if (State & SWITCH_PRAYER_MELEE || State & SWITCH_PRAYER_RANGED || State & SWITCH_PRAYER_MAGIC)
        {
            Gorillas::Prayers(State, Gorilla);
            State = Gorillas::GetState();
        }

        if (State & EQUIP_MELEE || State & EQUIP_RANGED || State & EQUIP_SPECIAL)
        {
            Gorillas::Gear(State, Gorilla);
            State = Gorillas::GetState();
        }

        static auto LastIndex = Gorilla->GetIndex();
        static auto LastProtectedStyle = Gorilla->GetProtectionStyle();
        if (LastIndex != Gorilla->GetIndex())
        {
            LastIndex = Gorilla->GetIndex();
            LastProtectedStyle = Gorilla->GetProtectionStyle();
        } else
        {
            if (LastProtectedStyle != Gorilla->GetProtectionStyle())
            {
                Gorillas::Special();
                LastProtectedStyle = Gorilla->GetProtectionStyle();
            }
        }

        if (Gorillas::Food()) continue;
        if (Gorillas::Restore()) continue;

        if (State == 0) Gorillas::Attack(Gorilla, false, true);

        bool CanRelax = Gorillas::IsAttacking() && Gorilla->AttacksUntilSwitch >= 2 && Gorilla->CountNextPossibleAttackStyles() == 1;
        if (CanRelax)
        {
            //DebugLog("Relaxing...");
        }
    }
    GameListener::Instance().Stop(true);
    return true;
}

void Gorillas::Draw()
{
    auto Current = GameListener::GetCurrentGorilla();
    if (Current && *Current)
    {
        auto State = Gorillas::GetState();

        if (State & MELEE_MOVE)
        {
            auto MeleeMoveTile = Gorillas::GetCurrentMeleeMoveTile();
            if (MeleeMoveTile) Paint::DrawTile(*MeleeMoveTile, 0, 255, 0, 255);
        }

        if (State & BOULDER)
        {
            auto BoulderMoveTile = Gorillas::GetCurrentBoulderMoveTile();
            if (BoulderMoveTile) Paint::DrawTile(*BoulderMoveTile, 200, 0, 255, 255);
        }

        Paint::DrawTile(Gorillas::GetGorillaMoveTile(Current), 0, 255, 0, 255);

        Paint::DrawString(Gorillas::GetStateString(State), Internal::TileToMainscreen(Minimap::GetPosition(), 0, 0, 0) + Point(20, 0), 0, 255, 255, 255);
        Current->Draw(true);
    }
}
