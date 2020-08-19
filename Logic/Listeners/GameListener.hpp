#ifndef GAMELISTENER_HPP_INCLUDED
#define GAMELISTENER_HPP_INCLUDED

#include <Core/Classes/NPC.hpp>
#include <shared_mutex>
#include <map>
#include <Game/Interactable/Projectile.hpp>
#include "LoopTask.hpp"
#include "../Types/Gorilla.hpp"
#include "../Types/TrackedPlayer.hpp"

class GameListener : public LoopTask
{
public:
    static GameListener& Instance();

    /**
     * Time markers marking the lifespan of players/gorillas
     * Gorillas -
     *      current attacks / max attacks (attack progress)
     * Boulders tied to relevant player (DrawLine), with time left
     * Recent hitsplats (not sure)
     * GameTicks - maybe like a blinking dot, idk
     */

    static void DrawGorillas();
    static void DrawPlayers();
    static void DrawProjectiles();

    static std::shared_ptr<Gorilla> GetCurrentGorilla();
    static std::shared_ptr<TrackedPlayer> GetPlayer(const Internal::Player& P);
    static bool AnyActiveBoulderOn(const Tile& T);

    static std::vector<WorldArea> GetPlayerAreas(bool IncludeLocal = false);
    static std::vector<WorldArea> GetGorillaAreas(std::int32_t Index, bool Lock = true);
    static std::vector<WorldArea> GetBoulderAreas();

private:

    static bool IsGorilla(std::int32_t ID);

    static void CheckNPCs();
    static void CheckPlayers();
    static void CheckHitsplats();
    static void CheckProjectiles();
    static void CheckCurrentGorilla();

    static void CheckGorillaAttackStyleSwitch(std::shared_ptr<Gorilla> G, std::int32_t ProtectedAttackStyleFlags);

    static void CheckGorillaAttacks();
    static void ProcessPendingAttacks();
    static void UpdateTrackedPlayers();

    static void ClearPlayers();
    static void ClearProjectiles();
    static void ClearPendingAttacks();
    static void ClearRecentBoulders();
    static void ClearCurrentGorilla();

    void OnStart() override;
    static void Loop();

    static void OnGameTick();
    static void OnNPCUpdate(std::vector<Internal::NPC>& NPCs, std::vector<std::int32_t>& NPCIndices);
    static void OnGorillaLoaded(const std::shared_ptr<Gorilla>& G, std::int32_t Index);
    static void OnGorillaUnloaded(const std::shared_ptr<Gorilla>& G, std::int32_t Index);
    static void OnGorillaAttack(std::shared_ptr<Gorilla> G, std::int32_t AttackStyle);
    static void OnPlayerUpdate(std::vector<Internal::Player>& Players, std::vector<std::int32_t>& PlayerIndices);
    static void OnPlayerLoaded(const std::shared_ptr<TrackedPlayer>& P, std::int32_t Index);
    static void OnPlayerUnloaded(const std::shared_ptr<TrackedPlayer>& P, std::int32_t Index);
    static void OnHitsplat(const std::shared_ptr<TrackedPlayer>& P, Hitsplat& H);
    static void OnProjectile(Interactable::Projectile& P);

    inline static std::atomic<bool> ProcessGameTick = false;
    inline static std::atomic<std::uint32_t> TickCount = 0;
    inline static std::atomic<std::int64_t> LastTickTime = 0;
    inline static std::atomic<std::int32_t> LastNPCUpdateTick = -1;
    inline static std::atomic<std::int32_t> LastPlayerUpdateTick = -1;
    inline static std::atomic<std::int32_t> LocalPlayerIndex = -1;
    inline static std::shared_ptr<Gorilla> CurrentGorilla;

    inline static std::shared_mutex GorillasLock;
    inline static std::map<std::int32_t, std::shared_ptr<Gorilla>> TrackedGorillas;
    static void TrackGorilla(std::shared_ptr<Gorilla> G, std::int32_t Index);
    static std::shared_ptr<Gorilla> GetGorilla(std::int32_t Index);
    static std::shared_ptr<Gorilla> GetGorilla(Gorilla& G);
    static std::shared_ptr<Gorilla> GetGorilla(Internal::NPC& N);
    static bool GorillaTracked(std::int32_t Index);

    inline static std::shared_mutex PlayersLock;
    inline static std::map<std::int32_t, std::shared_ptr<TrackedPlayer>> TrackedPlayers;
    static void TrackPlayer(std::shared_ptr<TrackedPlayer> P, std::int32_t Index);
    static std::shared_ptr<TrackedPlayer> GetPlayer(std::int32_t Index);
    //static std::shared_ptr<TrackedPlayer> GetPlayer(const Internal::Player& P);

    inline static std::shared_mutex ProjectilesLock;
    inline static std::mutex BouldersLock;
    inline static std::vector<Interactable::Projectile> TrackedProjectiles;
    inline static std::vector<Tile> RecentBoulderLocations;
    static void TrackProjectile(Interactable::Projectile& P);
    static bool IsProjectileTracked(const Interactable::Projectile& P);
    static bool AnyBoulderOn(const Tile& T);

    struct PendingAttack
    {
        Internal::NPC Gorilla = Internal::NPC(nullptr);
        Internal::Player Target = Internal::Player(nullptr);
        std::int32_t AttackStyle = 0;
        std::int32_t FinishTick = 0;
    };

    inline static std::vector<PendingAttack> PendingAttacks;
    static void AddPendingAttack(const PendingAttack& P);

    template<typename Function>
    inline static void ForEachGorillas(Function _f)
    {
        std::shared_lock Lock(GorillasLock);
        std::for_each(GameListener::TrackedGorillas.begin(), GameListener::TrackedGorillas.end(), _f);
    }

    GameListener();
};

#endif // GAMELISTENER_HPP_INCLUDED