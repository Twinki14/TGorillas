#include "GorillaOld.hpp"
#include "Gorillas.hpp"
#include <Game/Core.hpp>
#include <TScript.hpp>
#include <Utilities/Mainscreen.hpp>

Gorillas::Gorilla& Gorillas::Gorilla::GetInstance()
{
    static Gorillas::Gorilla Instance;
    return Instance;
}

std::int32_t Gorillas::Gorilla::GetProtectionStyle() const
{
/*    switch (this->GetGorilla().GetID())
    {
        case Globals::NPCs::DEMONIC_GORILLA_MELEE_7144:
        case Globals::NPCs::DEMONIC_GORILLA_MELEE_7147: return Gorillas::MELEE;

        case Globals::NPCs::DEMONIC_GORILLA_RANGE_7145:
        case Globals::NPCs::DEMONIC_GORILLA_RANGE_7148: return Gorillas::RANGED;

        case Globals::NPCs::DEMONIC_GORILLA_MAGE_7146:
        case Globals::NPCs::DEMONIC_GORILLA_MAGE_7149: return Gorillas::MAGIC;
    }*/
    return -1;
}

void Gorillas::Gorilla::SetGorilla(const Interactable::NPC& G)
{
    this->CacheIndices();
    this->SetGorilla(G, this->GetNPCIndexOf(G));
}

void Gorillas::Gorilla::SetGorilla(const Interactable::NPC& G, std::int32_t Index)
{
    this->GorillaNPC = G;
    this->CurrentGorillaIndex = Index;
}

void Gorillas::Gorilla::SetPlayer(const Interactable::Player& P)
{
    this->Player = P;
}

const Interactable::NPC& Gorillas::Gorilla::GetGorilla() const
{
    return this->GorillaNPC;
}

const Interactable::Player& Gorillas::Gorilla::GetPlayer() const
{
    return this->Player;
}

std::int32_t Gorillas::Gorilla::GetState() const
{
    return 0;
}

std::vector<Gorillas::STYLE> Gorillas::Gorilla::GetPredictedStyles() const
{
    return std::vector<STYLE>();
}

Internal::Character Gorillas::Gorilla::GetInteracting() const
{
    return this->GetGorilla().GetInteracting();
}

Interactable::Player Gorillas::Gorilla::GetTarget() const
{
    return (Internal::Player) this->GetGorilla().GetInteracting();
}

Interactable::NPC Gorillas::Gorilla::GetPlayerTarget() const
{
    return (Internal::NPC) this->GetPlayer().GetInteracting();
}

Tile Gorillas::Gorilla::GetTrueLocation() const
{
    const auto PathX = this->GetGorilla().GetPathX();
    const auto PathY = this->GetGorilla().GetPathY();
    if (PathX.empty() || PathY.empty()) return Tile();

    const auto ClientX = Internal::GetClientX();
    const auto ClientY = Internal::GetClientY();
    const auto ClientPlane = Internal::GetClientPlane();
    if (ClientX < 0 || (ClientX >= 65536) || ClientY < 0 || (ClientY >= 65536))
        return Tile(-1, -1, -1);

    auto LocalTrueLoc = Tile(PathX[0] * 128 + 128 / 2,  PathY[0] * 128 + 128 / 2, ClientPlane);
    return Tile(ClientX + (LocalTrueLoc.X >> 7), ClientY + (LocalTrueLoc.Y >> 7), ClientPlane);
}

bool Gorillas::Gorilla::IsDead() const
{
    return Gorillas::Gorilla::IsDead(this->GetGorilla());
}

bool Gorillas::Gorilla::InMemory()
{
    if (!this->GetGorilla()) return false;
    if (this->CurrentGorillaIndex < 0) return false;
    return this->IndexInCache(this->CurrentGorillaIndex) && Gorillas::Gorilla::IndexIsGorilla(this->CurrentGorillaIndex);
}

void Gorillas::Gorilla::Find()
{
    const auto LocalPlayerIndex = Internal::GetLocalPlayerIndex();
    if (!this->Player || LocalPlayerIndex != CurrentLocalPlayerIndex)
    {
        this->SetPlayer(Internal::GetLocalPlayer());
        this->CurrentLocalPlayerIndex = LocalPlayerIndex;
        DebugLog("Set local player");
    }

    this->CacheIndices();
    bool InMemory = this->InMemory();
    std::vector<Interactable::NPC> Gorillas;
    if (InMemory) // if the current gorilla is in memory (loaded)
    {
        const auto Target = this->GetTarget();
        if (Target == this->Player) // current gorilla is targeting the player
        {
            if (!this->IsDead()) return; // not dead, this is our current gorilla
        }

        const auto PlayerTarget = this->GetPlayerTarget();
        if (PlayerTarget && Gorillas::IsGorilla(PlayerTarget.GetID())) // the npc the player is currently targeting
        {
            if (this->GetGorilla() == PlayerTarget) // the current gorilla is the same as the current player target
            {
                if (!this->IsDead()) return; // not dead, this is our current gorilla
            } else
            {
                if (!Gorillas::Gorilla::IsDead(PlayerTarget)) // not dead, and it's a gorilla
                {
                    DebugLog("Set new gorilla > PlayerTarget");
                    this->SetGorilla(PlayerTarget, this->GetNPCIndexOf(PlayerTarget)); // but it's not our current gorilla
                    return;
                }
            }
        }

        Gorillas = this->GetGorillas();
        for (const auto& G : Gorillas) // check all gorillas to see if any are targeting the player
        {
            if (G.Interacting() && G.Interacting(Player))
            {
                if (Gorillas::Gorilla::IsDead(G)) continue;
                if (this->GetGorilla() != G)
                {
                    DebugLog("Set new gorilla > G Targeting player");
                    this->SetGorilla(G, this->GetNPCIndexOf(G));
                    return;
                } else
                    return;
            }
        }
    }

    if (Gorillas.empty()) Gorillas = this->GetGorillas();
    for (const auto& G : Gorillas) // find the closest viable
    {
        if (G.Interacting() && !G.Interacting(Player)) continue; // gorilla is targeting something that isn't our player
        if (Gorillas::Gorilla::IsDead(G)) continue; // it's dead

        if (InMemory) // We already have a gorilla, look for a more viable one
        {
            // TODO More viable could also take into consideration our currently equipped style
            // So we set to a gorilla that isn't already praying against our currently equipped style
            if (this->GetGorilla() != G)
            {
                this->SetGorilla(G, this->GetNPCIndexOf(G));
                DebugLog("Set new gorilla > G more viable");
            }
            return;
        } else
        {
            this->SetGorilla(G, this->GetNPCIndexOf(G));
            DebugLog("Set new gorilla");
            break;
        }
    }
}

void Gorillas::Gorilla::Listen()
{
    Projectiles::GetAll();
    return;
    this->Find();

/*    auto Projectiles = Projectiles::GetAll(Globals::Gorillas::PROJECTILES);
    for (auto& P : Projectiles)
    {
        if (!this->IsProjectileActive(P))
        {
            this->OnProjectile(P);
            this->AddActiveProjectile(P);
        }
    }*/

    /*const auto PlayerHitsplatTicks = Player.GetHitsplatTicks();
    const auto PlayerHitsplatValues = Player.GetHitsplatValues();
    if (PlayerHitsplatTicks.size() == PlayerHitsplatValues.size())
    {
        std::int32_t MostRecent_Index = -1;
        for (std::uint32_t I = 0; I < PlayerHitsplatTicks.size(); I++)
            if (MostRecent_Index < 0 || PlayerHitsplatTicks[I] > PlayerHitsplatTicks[MostRecent_Index])
                MostRecent_Index = I;

        if (MostRecent_Index >= 0 && PlayerHitsplatTicks[MostRecent_Index] > this->LastPlayerHitsplatTick)
            this->OnHitsplat(PlayerHitsplatTicks[MostRecent_Index], PlayerHitsplatValues[MostRecent_Index]);
    }*/

    if (this->GetGorilla())
    {
        auto AnimationID = this->GetGorilla().GetAnimationID();
        if (AnimationID != this->LastAnimationID) this->OnAnimation(AnimationID);
    }
}

void Gorillas::Gorilla::OnAnimation(std::int32_t A)
{
    DebugLog("OnAnimation");

    this->LastAnimationID = A;
}

void Gorillas::Gorilla::OnProjectile(Interactable::Projectile& P)
{
    DebugLog("OnProjectile");

    //if (P.GetID() != Globals::Gorillas::PROJECTILE_BOULDER && this->GetGorilla().GetTile() == P.GetTile())
        //this->LastProjectileID = P.GetID();
}

void Gorillas::Gorilla::Draw() const
{
    Paint::DrawConvex(this->GetGorilla().GetConvex(), 0, 255, 255, 255);
}

void Gorillas::Gorilla::CacheIndices()
{
    std::lock_guard<std::mutex> Lock(m_NPCIndicesLock);
    this->CachedNPCIndices = Internal::GetNPCIndices();
}

bool Gorillas::Gorilla::IndexInCache(std::int32_t Index)
{
    std::lock_guard<std::mutex> Lock(m_NPCIndicesLock);
    const auto NPCCount = Internal::GetNPCCount();
    if (this->CachedNPCIndices.size() < NPCCount) return false;
    for (std::uint32_t I = 0; I < NPCCount; I++)
        if (this->CachedNPCIndices[I] == Index) return true;
    return false;
}

std::int32_t Gorillas::Gorilla::GetNPCIndexOf(const Interactable::NPC& N)
{
    std::lock_guard<std::mutex> Lock(m_NPCIndicesLock);
    const auto NPCCount = Internal::GetNPCCount();
    if (this->CachedNPCIndices.size() < NPCCount) return -1;
    for (std::uint32_t I = 0; I < NPCCount; I++)
        if (Internal::GetNPC(this->CachedNPCIndices[I]) == N) return this->CachedNPCIndices[I];
    return -1;
}

bool Gorillas::Gorilla::IndexIsGorilla(std::int32_t Index)
{
    const auto N = Internal::GetNPC(Index);
    return N && Gorillas::IsGorilla(N.GetNPCInfo().GetID());
}

bool Gorillas::Gorilla::IsDead(const Interactable::NPC& N)
{
    if (N)
    {
        if (Internal::GetHealthPercentage(N) == 0.00) return true;
        //if (N.GetAnimationID() == Globals::Gorillas::ANIMATION_DYING) return true;
    }
    return false;
}

std::vector<Interactable::NPC> Gorillas::Gorilla::GetGorillas()
{
    std::vector<Interactable::NPC> Gorillas;
    const auto NPCCount = Internal::GetNPCCount();

    m_NPCIndicesLock.lock();
    if (this->CachedNPCIndices.size() < NPCCount) return Gorillas;
    for (std::uint32_t I = 0; I < NPCCount; I++)
    {
        auto N = Internal::GetNPC(this->CachedNPCIndices[I]);
        if (N && Gorillas::IsGorilla(N.GetNPCInfo().GetID()))
            Gorillas.emplace_back(std::move(N));
    }
    m_NPCIndicesLock.unlock();

    const auto PlayerTrueLocation = Mainscreen::GetTrueLocation();
    static const auto GetTrueLocation = [](const Interactable::NPC& N, std::int32_t ClientX, std::int32_t ClientY, std::int32_t ClientPlane) -> Tile
    {
        const auto PathX = N.GetPathX();
        const auto PathY = N.GetPathY();
        if (PathX.empty() || PathY.empty() || !N) return Tile();

        if (ClientX < 0 || (ClientX >= 65536) || ClientY < 0 || (ClientY >= 65536))
            return Tile(-1, -1, -1);

        auto LocalTrueLoc = Tile(PathX[0] * 128 + 128 / 2,  PathY[0] * 128 + 128 / 2, ClientPlane);
        return Tile(ClientX + (LocalTrueLoc.X >> 7), ClientY + (LocalTrueLoc.Y >> 7), ClientPlane);
    };

    const auto ClientX = Internal::GetClientX();
    const auto ClientY = Internal::GetClientY();
    const auto ClientPlane = Internal::GetClientPlane();
    auto Sort = [&](const Interactable::NPC& A, const Interactable::NPC& B) -> bool
    {
        return Mainscreen::GetProjectedDistance(PlayerTrueLocation, GetTrueLocation(A, ClientX, ClientY, ClientPlane)) <
            Mainscreen::GetProjectedDistance(PlayerTrueLocation, GetTrueLocation(B, ClientX, ClientY, ClientPlane));
    };

    std::sort(Gorillas.begin(), Gorillas.end(), Sort);
    return Gorillas;
}

void Gorillas::Gorilla::AddActiveProjectile(Interactable::Projectile& P)
{
    std::lock_guard<std::mutex> Lock(m_ActiveProjectilesLock);
    auto Equals = [&P](const Interactable::Projectile& A) -> bool { return A == P; };
    if (!std::any_of(this->ActiveProjectiles.begin(), this->ActiveProjectiles.end(), Equals))
        this->ActiveProjectiles.emplace_back(std::move(P));
}

bool Gorillas::Gorilla::IsProjectileActive(const Interactable::Projectile& P)
{
    std::lock_guard<std::mutex> Lock(m_ActiveProjectilesLock);
    auto Projectile = std::find(this->ActiveProjectiles.begin(), this->ActiveProjectiles.end(), P);
    if (Projectile != this->ActiveProjectiles.end())
    {
        if (Internal::GetGameTick() >= Projectile->GetEndTick())
        {
            this->ActiveProjectiles.erase(Projectile);
            return true;
        }
    }
    return false;
}









