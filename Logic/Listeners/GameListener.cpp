#include "GameListener.hpp"
#include <Core/Internal.hpp>
#include <TScript.hpp>
#include <functional>
#include <Game/Interfaces/Mainscreen.hpp>

GameListener& GameListener::Instance()
{
    static GameListener Inst;
    return Inst;
}

void GameListener::DrawGorillas()
{
    std::shared_lock Lock(GorillasLock);
    for (auto [Index, G] : GameListener::TrackedGorillas)
    {
        if (!G) continue;
        Point Next = Internal::TileToMainscreen(G->GetTile(), 0, 0, 0);

        Paint::DrawString(std::to_string(G->AttacksUntilSwitch), Next, 0, 255, 0, 255); Next.Y += 20;

        if (G->NextPossibleAttackStyles & Gorilla::MELEE_FLAG)
        {
            Paint::DrawString("MELEE", Next, 0, 255, 255, 255);
            Next.Y += 20;
        }

        if (G->NextPossibleAttackStyles & Gorilla::RANGED_FLAG)
        {
            Paint::DrawString("RANGED", Next, 0, 255, 255, 255);
            Next.Y += 20;
        }

        if (G->NextPossibleAttackStyles & Gorilla::MAGIC_FLAG)
        {
            Paint::DrawString("MAGIC", Next, 0, 255, 255, 255);
            Next.Y += 20;
        }

        if (G->NextPossibleAttackStyles & Gorilla::BOULDER_FLAG)
        {
            Paint::DrawString("BOULDER", Next, 0, 255, 255, 255);
            Next.Y += 20;
        }
    }
}

bool GameListener::IsGorilla(std::int32_t ID)
{
    return ID == Globals::Gorillas::DEMONIC_GORILLA_MELEE_7144 ||
           ID == Globals::Gorillas::DEMONIC_GORILLA_RANGE_7145 ||
           ID == Globals::Gorillas::DEMONIC_GORILLA_MAGE_7146 ||
           ID == Globals::Gorillas::DEMONIC_GORILLA_MELEE_7147 ||
           ID == Globals::Gorillas::DEMONIC_GORILLA_RANGE_7148 ||
           ID == Globals::Gorillas::DEMONIC_GORILLA_MAGE_7149;
}

void GameListener::CheckNPCs()
{
    Timer T;
    std::int32_t HighestNPCTick = 0;
    const auto NPCCount = Internal::GetNPCCount();
    auto NPCIndices = Internal::GetNPCIndices();

    std::vector<Internal::NPC> NPCs;
    if (NPCIndices.size() > NPCCount)
    {
        NPCIndices.resize(NPCCount);
        for (const auto& Index : NPCIndices)
        {
            Internal::NPC N = Internal::GetNPC(Index);
            if (!N) continue;
            auto Tick = N.GetNPCTick();
            if (Tick > HighestNPCTick) HighestNPCTick = Tick;
            NPCs.emplace_back(std::move(N));
        }
    }

    if (GameListener::LastNPCUpdateTick < 0) GameListener::LastNPCUpdateTick = Internal::GetGameTick();
    if (HighestNPCTick > 0 && HighestNPCTick > GameListener::LastNPCUpdateTick)
    {
        if (Mainscreen::GetState() == Mainscreen::PLAYING)
        {
            GameListener::OnNPCUpdate(NPCs, NPCIndices);
            GameListener::ProcessGameTick = true;
        }
        GameListener::LastNPCUpdateTick = HighestNPCTick;
    }

    if (T.GetTimeElapsed() >= 2) DebugLog("[GameListener] > Took {}ms", T.GetTimeElapsed());
}

void GameListener::CheckPlayers()
{
    Timer T;
    std::int32_t HighestPlayerTick = 0;
    const auto PlayerCount = Internal::GetPlayerCount();
    auto PlayerIndices = Internal::GetPlayerIndices();
    std::vector<Internal::Player> Players;
    if (PlayerIndices.size() > PlayerCount)
    {
        PlayerIndices.resize(PlayerCount);
        for (const auto& Index : PlayerIndices)
        {
            Internal::Player P = Internal::GetPlayer(Index);
            if (!P) continue;
            auto Tick = P.GetPlayerTick();
            if (Tick > HighestPlayerTick) HighestPlayerTick = Tick;
            Players.emplace_back(std::move(P));
        }
    }

    if (GameListener::LastPlayerUpdateTick < 0) GameListener::LastPlayerUpdateTick = Internal::GetGameTick();
    if (HighestPlayerTick > 0 && HighestPlayerTick > GameListener::LastPlayerUpdateTick)
    {
        if (Mainscreen::GetState() == Mainscreen::PLAYING)
            GameListener::OnPlayerUpdate(Players, PlayerIndices);
        GameListener::LastPlayerUpdateTick = HighestPlayerTick;
    }

    if (T.GetTimeElapsed() >= 2) DebugLog("[GameListener] > Took {}ms", T.GetTimeElapsed());
}

void GameListener::CheckHitsplats()
{
    Timer T;

    PlayersLock.lock();
    for (auto& [Index, Player] : GameListener::TrackedPlayers)
    {
        if (!Player) continue;

        const auto HitsplatValues = Player->GetHitsplatValues();
        const auto HitsplatTypes = Player->GetHitsplatTypes();
        const auto HitsplatTicks = Player->GetHitsplatTicks();
        if (HitsplatValues.size() == HitsplatTypes.size() && HitsplatValues.size() == HitsplatTicks.size())
        {
            for (std::uint32_t I = 0; I < HitsplatTicks.size(); I++)
            {
                Hitsplat H = Hitsplat(HitsplatValues[I], HitsplatTypes[I], HitsplatTicks[I]);
                if (!Player->GetLastHitsplat() || H.GetEndCycle() > Player->GetLastHitsplat()->GetEndCycle())
                {
                    GameListener::OnHitsplat(Player, H);
                    Player->AddHitsplat(H);
                }
            }
        }
    }
    PlayersLock.unlock();

    GorillasLock.lock_shared();
    for (auto& [Index, Gorilla] : GameListener::TrackedGorillas)
    {
        if (!Gorilla) continue;

        const auto HitsplatValues = Gorilla->GetHitsplatValues();
        const auto HitsplatTypes = Gorilla->GetHitsplatTypes();
        const auto HitsplatTicks = Gorilla->GetHitsplatTicks();
        if (HitsplatValues.size() == HitsplatTypes.size() && HitsplatValues.size() == HitsplatTicks.size())
        {
            for (std::uint32_t I = 0; I < HitsplatTicks.size(); I++)
            {
                Hitsplat H = Hitsplat(HitsplatValues[I], HitsplatTypes[I], HitsplatTicks[I]);
                if (Gorilla->LastHitsplatEndTick < 0 || H.GetEndCycle() > Gorilla->LastHitsplatEndTick)
                {
                    Gorilla->LastHitsplatEndTick = H.GetEndCycle();
                    if (H.GetType() == Hitsplat::BLOCK_ME || H.GetType() == Hitsplat::DAMAGE_ME)
                        Gorilla->RecentlyTookDamage = true;
                }
            }
        }
    }
    GorillasLock.unlock_shared();

    if (T.GetTimeElapsed() >= 2) DebugLog("[GameListener] > Took {}ms", T.GetTimeElapsed());
}

void GameListener::CheckProjectiles()
{
    Timer T;
    Internal::Deque RawProjectiles = Internal::GetProjectiles();
    Internal::Node Head = RawProjectiles.GetHead();
    Internal::Node Node = Head.GetNext();
    std::vector<Interactable::Projectile> CurrentProjectiles;
    while (Node)
    {
        Internal::Projectile I(Node);
        if (I)
        {
            auto Found = std::find(Globals::Gorillas::PROJECTILES.begin(), Globals::Gorillas::PROJECTILES.end(), I.GetID());
            if (Found != Globals::Gorillas::PROJECTILES.end())
                CurrentProjectiles.emplace_back(std::move(I));
        }

        Node = Node.GetNext();
        if (Node.Equals(Head)) break;
    }

    auto CurrentGameTick = Internal::GetGameTick();
    for (auto& P : CurrentProjectiles)
    {
        if (!P) continue;
        if (!GameListener::IsProjectileTracked(P) && CurrentGameTick < P.GetEndTick())
        {
            GameListener::OnProjectile(P);
            GameListener::TrackProjectile(P);
        }
    }
    if (T.GetTimeElapsed() >= 2)  DebugLog("[GameListener] > Took {}ms", T.GetTimeElapsed());
}

void GameListener::CheckGorillaAttackStyleSwitch(std::shared_ptr<Gorilla> G, std::int32_t ProtectedAttackStyleFlags)
{
    if (!G) return;

    if (G->AttacksUntilSwitch <= 0 || G->NextPossibleAttackStyles == 0)
    {
        G->NextPossibleAttackStyles = 0;
        if ((ProtectedAttackStyleFlags & Gorilla::MELEE_FLAG) == 0)     G->NextPossibleAttackStyles |= Gorilla::MELEE_FLAG;
        if ((ProtectedAttackStyleFlags & Gorilla::RANGED_FLAG) == 0)    G->NextPossibleAttackStyles |= Gorilla::RANGED_FLAG;
        if ((ProtectedAttackStyleFlags & Gorilla::MAGIC_FLAG) == 0)     G->NextPossibleAttackStyles |= Gorilla::MAGIC_FLAG;
        G->AttacksUntilSwitch = Globals::Gorillas::ATTACKS_PER_SWITCH;
        G->ChangedAttackStyleThisTick = true;
    }
}

void GameListener::CheckGorillaAttacks()
{
    GorillasLock.lock();
    std::vector<WorldArea> PlayerAreas;
    for (auto [Index, Gorilla] : GameListener::TrackedGorillas)
    {
        Internal::Player Interacting = (Internal::Player) Gorilla->GetInteracting();
        auto Target = GameListener::GetPlayer(Interacting);

        if (Gorilla->LastTickInteractingIndex != -1 && !Interacting) // No longer in combat
        {
            Gorilla->InitiatedCombat = false;
        } else if (Target && *Target && *Target->GetLastWorldArea() && !Gorilla->InitiatedCombat
                   && TickCount < Gorilla->NextAttackTick
                   && Gorilla->GetWorldArea().IsInMeleeDistance(*Target->GetLastWorldArea()))
        {
            Gorilla->InitiatedCombat = true;
            Gorilla->NextAttackTick = TickCount + 1;
        }

        const auto AnimationID = Gorilla->GetAnimationID();
        if (Gorilla->RecentlyTookDamage && TickCount >= Gorilla->NextAttackTick + 4)
        {
            // The gorilla was flinched, so its next attack gets delayed
            Gorilla->NextAttackTick = TickCount + Globals::Gorillas::ATTACK_RATE / 2; //TODO - Test rounding
            Gorilla->InitiatedCombat = true;

            if (Target && *Target && *Target->GetLastWorldArea()
                && !Gorilla->GetWorldArea().IsInMeleeDistance(*Target->GetLastWorldArea())
                && !Gorilla->GetWorldArea().IntersectsWith(*Target->GetLastWorldArea()))
            {
                // Gorillas stop meleeing when they get flinched
                // and the target isn't in melee distance

                Gorilla->NextPossibleAttackStyles &= ~Gorilla::MELEE_FLAG;

                if (Interacting)
                {
                    std::int32_t ProtectedStyles = Gorilla::MELEE_FLAG;
                    const auto ProtectionIcon = Interacting.GetOverheadIcon();
                    switch (ProtectionIcon)
                    {
                        case Globals::ICON_MELEE: ProtectedStyles |= Gorilla::MELEE_FLAG; break;
                        case Globals::ICON_RANGED: ProtectedStyles |= Gorilla::RANGED_FLAG; break;
                        case Globals::ICON_MAGIC: ProtectedStyles |= Gorilla::MAGIC_FLAG; break;
                        default: break;
                    }
                    GameListener::CheckGorillaAttackStyleSwitch(Gorilla, ProtectedStyles);
                }
            }
        } else if (AnimationID != Gorilla->LastTickAnimation)
        {
            switch (AnimationID)
            {
                case Globals::Gorillas::ANIMATION_MELEE_ATTACK: OnGorillaAttack(Gorilla, Gorilla::MELEE_FLAG); break;
                case Globals::Gorillas::ANIMATION_MAGIC_ATTACK: OnGorillaAttack(Gorilla, Gorilla::MAGIC_FLAG); break;
                case Globals::Gorillas::ANIMATION_RANGED_ATTACK: OnGorillaAttack(Gorilla, Gorilla::RANGED_FLAG); break;
                case Globals::Gorillas::ANIMATION_AOE_ATTACK:
                {
                    // Note that AoE animation is the same as prayer switch animation
                    // so we need to check if the prayer was switched or not.
                    // It also does this animation when it spawns, so
                    // we need the interacting != null check.

                    if (Gorilla->GetOverheadIcon() == Gorilla->LastTickOverheadIcon)
                    {
                        // Confirmed, the gorilla used the AoE attack
                        GameListener::OnGorillaAttack(Gorilla, Gorilla::BOULDER_FLAG);
                    } else
                    {
                        if (TickCount >= Gorilla->NextAttackTick)
                        {
                            Gorilla->ChangedPrayerThisTick = true;
                            switch (Gorilla->LastProjectileID)
                            {
                                case Globals::Gorillas::PROJECTILE_MAGIC: GameListener::OnGorillaAttack(Gorilla, Gorilla::MAGIC_FLAG); break;
                                case Globals::Gorillas::PROJECTILE_RANGED: GameListener::OnGorillaAttack(Gorilla, Gorilla::RANGED_FLAG); break;
                                default:
                                {
                                    if (Target && *Target)
                                    {
                                        if (*Target->GetLastWorldArea() && GameListener::AnyBoulderOn(Target->GetLastWorldArea()->AsTile()))
                                        {
                                            // A boulder started falling on the gorillas target,
                                            // so we assume it was the gorilla who shot it
                                            GameListener::OnGorillaAttack(Gorilla, Gorilla::BOULDER_FLAG);
                                        } else if (Target->HasRecentHitsplats())
                                        {
                                            // It wasn't any of the three other attacks,
                                            // but the player took damage, so we assume
                                            // it's a melee attack
                                            GameListener::OnGorillaAttack(Gorilla, Gorilla::MELEE_FLAG);
                                        }
                                    }
                                }
                            }

                            // The next attack tick is always delayed if the
                            // gorilla switched prayer
                            Gorilla->NextAttackTick = TickCount + Globals::Gorillas::ATTACK_RATE;
                            Gorilla->ChangedPrayerThisTick = true;
                        }
                    }
                }
                default: break;
            }
        }

        if (Gorilla->DisabledMeleeMovementTicks > 0)
            Gorilla->DisabledMeleeMovementTicks -= 1;
        else if (Gorilla->InitiatedCombat && Gorilla->GetInteracting()
                 && !Gorilla->ChangedAttackStyleThisTick
                 && Gorilla->CountNextPossibleAttackStyles() >= 2
                 && (Gorilla->NextPossibleAttackStyles & Gorilla::MELEE_FLAG))
        {
            // If melee is a possibility, we can check if the gorilla
            // is or isn't moving toward the player to determine if
            // it is actually attempting to melee or not.
            // We only run this check if the gorilla is in combat
            // because otherwise it attempts to travel to melee
            // distance before attacking its target.

            if (Target && *Target && *Target->GetLastWorldArea() && *Gorilla->GetLastWorldArea())
            {
                std::vector<WorldArea> GorillaAreas;
                for (const auto&[I, G] : GameListener::TrackedGorillas)
                    if (G && I != Gorilla->GetIndex())
                        GorillaAreas.emplace_back(G->GetIndex() < Gorilla->GetIndex() ? G->GetWorldArea() : *G->GetLastWorldArea());

                if (PlayerAreas.empty())
                {
                    PlayersLock.lock();
                    for (const auto&[I, P] : GameListener::TrackedPlayers)
                        if (P) PlayerAreas.emplace_back(*P->GetLastWorldArea());
                    PlayersLock.unlock();
                }

                WorldArea PredictedNewArea = Gorilla->GetLastWorldArea()->CalculateNextTravellingPoint(
                        Target->GetWorldArea(), true,
                        [&](const Tile& T) -> bool
                        {
                            // Gorillas can't normally walk through other gorillas
                            // or other players

                            const auto Area1 = WorldArea(T, 1, 1);
                            if (!Area1) return true;

                            auto MatchingG = std::find_if(GorillaAreas.begin(), GorillaAreas.end(),
                                                          [&Area1](const WorldArea& A) -> bool
                                                          { return A && Area1.IntersectsWith(A); });

                            if (MatchingG == GorillaAreas.end())
                            {
                                auto MatchingP = std::find_if(PlayerAreas.begin(), PlayerAreas.end(),
                                                              [&Area1](const WorldArea& A) -> bool
                                                              { return A && Area1.IntersectsWith(A); });
                                return MatchingP == PlayerAreas.end();
                            }

                            return false;

                            // There is a special case where if a player walked through
                            // a gorilla, or a player walked through another player,
                            // the tiles that were walked through becomes
                            // walkable, but I didn't feel like it's necessary to handle
                            // that special case as it should rarely happen.
                        });

                if (PredictedNewArea)
                {
                    std::int32_t Distance = Gorilla->GetLastWorldArea()->DistanceTo(*Target->GetLastWorldArea());
                    if (Distance <= Globals::Gorillas::MAX_ATTACK_RANGE && Target->GetLastWorldArea()->HasLineOfSightTo(*Gorilla->GetLastWorldArea()))
                    {
                        const auto PredictedTile = PredictedNewArea.AsTile();
                        if (std::floor(PredictedTile.DistanceFrom(Gorilla->GetLastWorldArea()->AsTile())) != 0.00)
                        {
                            if (std::floor(PredictedTile.DistanceFrom(Gorilla->GetTrueLocation())) == 0.00)
                            {
                                // Turn off all but MELEE
                                Gorilla->NextPossibleAttackStyles &= ~Gorilla::RANGED_FLAG;
                                Gorilla->NextPossibleAttackStyles &= ~Gorilla::MAGIC_FLAG;
                                Gorilla->NextPossibleAttackStyles &= ~Gorilla::BOULDER_FLAG;
                            } else
                                Gorilla->NextPossibleAttackStyles &= ~Gorilla::MELEE_FLAG;
                        } else if (TickCount >= Gorilla->NextAttackTick && Gorilla->LastProjectileID == -1
                                   && !GameListener::AnyBoulderOn(Target->GetLastWorldArea()->AsTile()))
                        {
                            // Turn off all but MELEE
                            Gorilla->NextPossibleAttackStyles &= ~Gorilla::RANGED_FLAG;
                            Gorilla->NextPossibleAttackStyles &= ~Gorilla::MAGIC_FLAG;
                            Gorilla->NextPossibleAttackStyles &= ~Gorilla::BOULDER_FLAG;
                        }
                    }

                }
            }
        }

        if (Gorilla->RecentlyTookDamage)
            Gorilla->InitiatedCombat = true;

        if (Gorilla->GetOverheadIcon() != Gorilla->LastTickOverheadIcon)
        {
            if (Gorilla->ChangedAttackStyleLastTick || Gorilla->ChangedAttackStyleThisTick)
            {
                // Apparently if it changes attack style and changes
                // prayer on the same tick or 1 tick apart, it won't
                // be able to move for the next 2 ticks if it attempts
                // to melee
                Gorilla->DisabledMeleeMovementTicks = 2;
            } else
            {
                // If it didn't change attack style lately,
                // it's only for the next 1 tick
                Gorilla->DisabledMeleeMovementTicks = 1;
            }
        }

        auto Area = std::make_shared<WorldArea>(WorldArea(Gorilla->GetWorldArea()));

        Gorilla->LastTickAnimation = Gorilla->GetAnimationID();
        Gorilla->SetLastWorldArea(std::move(Area));
        Gorilla->LastTickInteractingIndex = Gorilla->GetInteractIndex();
        Gorilla->RecentlyTookDamage = false;
        Gorilla->ChangedPrayerThisTick = false;
        Gorilla->ChangedAttackStyleLastTick = Gorilla->ChangedAttackStyleThisTick.load();
        Gorilla->ChangedAttackStyleThisTick = false;
        Gorilla->LastTickOverheadIcon = Gorilla->GetOverheadIcon();
        Gorilla->LastProjectileID = -1;
    }
    GorillasLock.unlock();
}

void GameListener::ProcessPendingAttacks()
{
    std::lock_guard<std::mutex> Lock(PendingAttacksLock);
    auto Iterator = GameListener::PendingAttacks.begin();
    while (Iterator != GameListener::PendingAttacks.end())
    {
        if (TickCount >= Iterator->FinishTick)
        {
            bool DecreaseCounter = false;
            auto Gorilla = GameListener::GetGorilla(Iterator->GorillaIndex);
            auto Target = GameListener::GetPlayer(Iterator->Target);

            if (!Target || !*Target) // Player went out of memory, so assume the hit was a 0
                DecreaseCounter = true;
            else if (Target->HasRecentHitsplats())
            {
                // No hitsplats was applied. This may happen in some cases
                // where the player was out of memory while the
                // projectile was travelling. So we assume the hit was a 0.
                DecreaseCounter = true;
            } else if (Target->BlockedRecentHitsplat())
            {
                // A blue hitsplat appeared, so we assume the gorilla hit a 0
                DecreaseCounter = true;
            }

            if (Gorilla && DecreaseCounter)
            {
                Gorilla->AttacksUntilSwitch -= 1;
                GameListener::CheckGorillaAttackStyleSwitch(Gorilla, 0);
            }

            Iterator = GameListener::PendingAttacks.erase(Iterator);
        } else
            ++Iterator;
    }
}

void GameListener::UpdateTrackedPlayers()
{
    PlayersLock.lock();
    for (auto [Index, Player] : GameListener::TrackedPlayers)
    {
        if (!Player) continue;
        auto Area = std::make_shared<WorldArea>(WorldArea(*Player));
        assert(Area);
        Player->SetLastWorldArea(std::move(Area));
        assert(!Area);
        Player->ClearRecentHitsplats();
    }
    PlayersLock.unlock();
}

void GameListener::ClearRecentBoulders()
{
    BouldersLock.lock();
    GameListener::RecentBoulderLocations.clear();
    BouldersLock.unlock();
}

void GameListener::OnStart()
{
    GameListener::LastTickTime = CurrentTimeMillis();
}

void GameListener::Loop()
{
    Timer T;

    // gorillas -> players -> hitsplats -> projectiles
    GameListener::CheckNPCs();
    GameListener::CheckPlayers();
    GameListener::CheckHitsplats();
    GameListener::CheckProjectiles();

    if (GameListener::ProcessGameTick)
    {
        GameListener::OnGameTick();
        GameListener::TickCount++;
        GameListener::LastTickTime = CurrentTimeMillis();
        GameListener::ProcessGameTick = false;
    }

    if (T.GetTimeElapsed() >= 2) DebugLog("[GameListener] > Took {}ms", T.GetTimeElapsed());
}

void GameListener::OnGameTick()
{
    DebugLog("[GameListener] > OnGameTick > x{} [{}ms]", GameListener::TickCount, CurrentTimeMillis() - GameListener::LastTickTime);

    GameListener::CheckGorillaAttacks();
    GameListener::ProcessPendingAttacks();
    GameListener::UpdateTrackedPlayers();
    GameListener::ClearRecentBoulders();
}

void GameListener::OnNPCUpdate(std::vector<Internal::NPC>& NPCs, std::vector<std::int32_t>& NPCIndices)
{
    //DebugLog("[GameListener] > NPC update");

    if (NPCs.size() == NPCIndices.size())
    {
        for (std::uint32_t I = 0; I < NPCs.size(); I++)
        {
            Interactable::NPC N = NPCs[I];
            if (!N || !IsGorilla(N.GetID())) continue;

            std::shared_ptr<Gorilla> Tracked = GameListener::GetGorilla(NPCIndices[I]);
            if (!Tracked || N != *Tracked)
            {
                auto G = std::make_shared<Gorilla>(N, NPCIndices[I]);
                GameListener::OnGorillaLoaded(G, G->GetIndex());
                GameListener::TrackGorilla(G, G->GetIndex());
            }
        }
    }

    GorillasLock.lock();
    auto Iterator = GameListener::TrackedGorillas.begin();
    while (Iterator != GameListener::TrackedGorillas.end())
    {
        auto Match = std::find(NPCIndices.begin(), NPCIndices.end(), Iterator->first);
        if (Match == NPCIndices.end())
        {
            GameListener::OnGorillaUnloaded(Iterator->second, Iterator->first);
            Iterator = GameListener::TrackedGorillas.erase(Iterator);
        } else
            ++Iterator;
    }
    GorillasLock.unlock();
}

void GameListener::OnGorillaLoaded(const std::shared_ptr<Gorilla>& G, std::int32_t Index)
{
    DebugLog("[GameListener] > Gorilla loaded > {} | {} | {}", G->GetName(), G->GetID(), Index);
}

void GameListener::OnGorillaUnloaded(const std::shared_ptr<Gorilla>& G, std::int32_t Index)
{
    DebugLog("[GameListener] > Gorilla unloaded > {} | {}", G->GetID(), Index);
}

void GameListener::OnGorillaAttack(std::shared_ptr<Gorilla> G, std::int32_t AttackStyle)
{
    if (!G) return;

    G->InitiatedCombat = true;
    Interactable::Player Target = (Internal::Player) G->GetInteracting();

    std::int32_t ProtectedStyle = 0;
    if (Target)
    {
        const auto ProtectionIcon = Target.GetOverheadIcon();
        switch (ProtectionIcon)
        {
            case Globals::ICON_MELEE:   ProtectedStyle = Gorilla::MELEE_FLAG; break;
            case Globals::ICON_RANGED:  ProtectedStyle = Gorilla::RANGED_FLAG; break;
            case Globals::ICON_MAGIC:   ProtectedStyle = Gorilla::MAGIC_FLAG; break;
            default: break;
        }
    }

    bool CorrectPrayer = !Target || (AttackStyle & ProtectedStyle);
    if (AttackStyle == Gorilla::BOULDER_FLAG)
    {
        // The gorilla can't throw boulders when it's meleeing
        G->NextPossibleAttackStyles &= ~Gorilla::MELEE_FLAG;
    } else
    {
        if (CorrectPrayer)
            G->AttacksUntilSwitch -= 1;
        else
        {
            // We're not sure if the attack will hit a 0 or not,
            // so we don't know if we should decrease the counter or not,
            // so we keep track of the attack here until the damage splat
            // has appeared on the player.

            std::int32_t DamageTick = TickCount;
            if (AttackStyle == Gorilla::MAGIC_FLAG)
            {
                auto TrackedTarget = GameListener::GetPlayer(Target);
                if (TrackedTarget && *TrackedTarget->GetLastWorldArea())
                {
                    std::int32_t Distance = G->GetWorldArea().DistanceTo(*TrackedTarget->GetLastWorldArea());
                    DamageTick += (Distance + Globals::Gorillas::PROJECTILE_MAGIC_DELAY) / Globals::Gorillas::PROJECTILE_MAGIC_SPEED;
                }
            } else if (AttackStyle == Gorilla::RANGED_FLAG)
            {
                auto TrackedTarget = GameListener::GetPlayer(Target);
                if (TrackedTarget && *TrackedTarget->GetLastWorldArea())
                {
                    std::int32_t Distance = G->GetWorldArea().DistanceTo(*TrackedTarget->GetLastWorldArea());
                    DamageTick += (Distance + Globals::Gorillas::PROJECTILE_RANGED_DELAY) / Globals::Gorillas::PROJECTILE_RANGED_SPEED;
                }
            }
            GameListener::AddPendingAttack(PendingAttack { G->GetIndex(), Target, AttackStyle, DamageTick });
        }

        if (AttackStyle != Gorilla::MELEE_FLAG)     G->NextPossibleAttackStyles &= ~Gorilla::MELEE_FLAG;
        if (AttackStyle != Gorilla::RANGED_FLAG)    G->NextPossibleAttackStyles &= ~Gorilla::RANGED_FLAG;
        if (AttackStyle != Gorilla::MAGIC_FLAG)     G->NextPossibleAttackStyles &= ~Gorilla::MAGIC_FLAG;
        if (AttackStyle != Gorilla::BOULDER_FLAG)   G->NextPossibleAttackStyles &= ~Gorilla::BOULDER_FLAG;

        if (G->NextPossibleAttackStyles == 0)
        {
            // Sometimes the gorilla can switch attack style before it's supposed to
            // if someone was fighting it earlier and then left, so we just
            // reset the counter in that case.

            if (AttackStyle == Gorilla::MELEE_FLAG)     G->NextPossibleAttackStyles = AttackStyle;
            if (AttackStyle == Gorilla::RANGED_FLAG)    G->NextPossibleAttackStyles = AttackStyle;
            if (AttackStyle == Gorilla::MAGIC_FLAG)     G->NextPossibleAttackStyles = AttackStyle;

            G->AttacksUntilSwitch = Globals::Gorillas::ATTACKS_PER_SWITCH - (CorrectPrayer ? 1 : 0);
        }
    }
    GameListener::CheckGorillaAttackStyleSwitch(G, ProtectedStyle);
    G->NextAttackTick = TickCount + Globals::Gorillas::ATTACK_RATE;
}

void GameListener::OnPlayerUpdate(std::vector<Internal::Player>& Players, std::vector<std::int32_t>& PlayerIndices)
{
    //DebugLog("[GameListener] > Player update");

    if (Players.size() == PlayerIndices.size())
    {
        for (std::uint32_t I = 0; I < Players.size(); I++)
        {
            Interactable::Player Player = Players[I];
            if (!Player) continue;

            std::shared_ptr<TrackedPlayer> Tracked = GameListener::GetPlayer(PlayerIndices[I]);
            if (!Tracked || Player != *Tracked)
            {
                auto P = std::make_shared<TrackedPlayer>(Player, PlayerIndices[I]);
                GameListener::OnPlayerLoaded(P, P->GetIndex());
                GameListener::TrackPlayer(P, P->GetIndex());
            }
        }
    }

    PlayersLock.lock();
    auto Iterator = GameListener::TrackedPlayers.begin();
    while (Iterator != GameListener::TrackedPlayers.end())
    {
        auto Match = std::find(PlayerIndices.begin(), PlayerIndices.end(), Iterator->first);
        if (Match == PlayerIndices.end())
        {
            GameListener::OnPlayerUnloaded(Iterator->second, Iterator->first);
            Iterator = GameListener::TrackedPlayers.erase(Iterator);
        } else
            ++Iterator;
    }
    PlayersLock.unlock();
}

void GameListener::OnPlayerLoaded(const std::shared_ptr<TrackedPlayer>& P, std::int32_t Index)
{
    DebugLog("[GameListener] > Player loaded > {} | {}", P->GetNamePair().GetCleanName(), Index);
}

void GameListener::OnPlayerUnloaded(const std::shared_ptr<TrackedPlayer>& P, std::int32_t Index)
{
    DebugLog("[GameListener] > Player unloaded > {}", Index);
}

void GameListener::OnHitsplat(const std::shared_ptr<TrackedPlayer>& P, Hitsplat& H)
{
    DebugLog("[GameListener] > Hitsplat spawned > {} > {} | {} | {}", P->GetIndex(), H.GetDamage(), H.GetType(), H.GetEndCycle());
}

void GameListener::OnProjectile(Interactable::Projectile& P)
{
    DebugLog("[GameListener] > Spawned > {} | {} ", P.GetID(), P.GetEndTick());

    if (P.GetID() == Globals::Gorillas::PROJECTILE_BOULDER)
    {
        BouldersLock.lock();
        GameListener::RecentBoulderLocations.emplace_back(P.GetTile());
        BouldersLock.unlock();
    } else
    {
        GorillasLock.lock();
        for (auto [Index, G] : GameListener::TrackedGorillas)
        {
            if (!G) continue;

            if (P.GetTile() == G->GetTile())
                G->LastProjectileID = P.GetID();
            // TODO add a warning here
        }
        GorillasLock.unlock();
    }
}

void GameListener::TrackGorilla(std::shared_ptr<Gorilla> G, std::int32_t Index)
{
    if (Index > -1)
    {
        std::unique_lock Lock(GorillasLock);
        TrackedGorillas.insert_or_assign(Index, std::move(G));
    }
}

std::shared_ptr<Gorilla> GameListener::GetGorilla(std::int32_t Index)
{
    std::shared_lock Lock(GorillasLock);
    auto Iterator = GameListener::TrackedGorillas.find(Index);
    if (Iterator != GameListener::TrackedGorillas.end())
        return Iterator->second;
    return std::shared_ptr<Gorilla>();
}

std::shared_ptr<Gorilla> GameListener::GetGorilla(Gorilla& G)
{
    std::shared_lock Lock(GorillasLock);
    for (auto& Gorilla : GameListener::TrackedGorillas)
        if (*Gorilla.second == G)
            return Gorilla.second;
    return std::shared_ptr<Gorilla>();
}

void GameListener::TrackPlayer(std::shared_ptr<TrackedPlayer> P, std::int32_t Index)
{
    if (Index > -1)
    {
        std::unique_lock Lock(PlayersLock);
        TrackedPlayers.insert_or_assign(Index, std::move(P));
    }
}

std::shared_ptr<TrackedPlayer> GameListener::GetPlayer(std::int32_t Index)
{
    std::shared_lock Lock(PlayersLock);
    auto Iterator = GameListener::TrackedPlayers.find(Index);
    if (Iterator != GameListener::TrackedPlayers.end())
        return Iterator->second;
    return std::shared_ptr<TrackedPlayer>();
}

std::shared_ptr<TrackedPlayer> GameListener::GetPlayer(Internal::Player& P)
{
    std::shared_lock Lock(PlayersLock);
    if (!P) return std::shared_ptr<TrackedPlayer>();
    for (auto& TrackedPlayer : GameListener::TrackedPlayers)
        if (*TrackedPlayer.second == P)
            return TrackedPlayer.second;
    return std::shared_ptr<TrackedPlayer>();
}

void GameListener::TrackProjectile(Interactable::Projectile& P)
{
    std::lock_guard<std::mutex> Lock(ProjectilesLock);
    auto Equals = [&P](const Interactable::Projectile& A) -> bool { return A == P; };
    if (!std::any_of(GameListener::TrackedProjectiles.begin(), GameListener::TrackedProjectiles.end(), Equals))
        GameListener::TrackedProjectiles.emplace_back(std::move(P));
}

bool GameListener::IsProjectileTracked(const Interactable::Projectile& P)
{
    std::lock_guard<std::mutex> Lock(ProjectilesLock);

    const auto CurrentGameTick = Internal::GetGameTick();
    auto Active = [&CurrentGameTick](const Interactable::Projectile& P) -> bool
    {
        return CurrentGameTick >= P.GetEndTick();
/*        if (CurrentGameTick >= P.GetEndTick())
        {
            DebugLog("[GameListener] > Removed > {} | {} ", P.GetID(), P.GetEndTick());
            return true;
        }
        return false;*/
    };

    GameListener::TrackedProjectiles.erase(std::remove_if(GameListener::TrackedProjectiles.begin(),
                                                          GameListener::TrackedProjectiles.end(),
                                                          Active), GameListener::TrackedProjectiles.end());

    auto Projectile = std::find(GameListener::TrackedProjectiles.begin(), GameListener::TrackedProjectiles.end(), P);
    return Projectile != GameListener::TrackedProjectiles.end();
}

bool GameListener::AnyBoulderOn(const Tile& T)
{
    if (!T) return false;
    std::lock_guard<std::mutex> Lock(BouldersLock);
    auto Find = [&T](const Tile& P) -> bool { return T == P; };
    auto P = std::find_if(GameListener::RecentBoulderLocations.begin(), GameListener::RecentBoulderLocations.end(), Find);
    if (P != GameListener::RecentBoulderLocations.end())
        return true;
    return false;
}

void GameListener::AddPendingAttack(const GameListener::PendingAttack& P)
{
    std::lock_guard<std::mutex> Lock(PendingAttacksLock);
    PendingAttacks.emplace_back(P);
}

GameListener::GameListener() : ListenerTask("GameListener", std::chrono::milliseconds(20), GameListener::Loop)
{

}
