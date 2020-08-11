#ifndef GORILLA_OLD_HPP_INCLUDED
#define GORILLA_OLD_HPP_INCLUDED

#include <thread>
#include <mutex>
#include <atomic>
#include <Game/Interactable/NPC.hpp>
#include <Game/Interactable/Player.hpp>
#include <Game/Interactable/Projectile.hpp>

/*
 * https://github.com/xKylee/plugins-source/blob/master/demonicgorilla/src/main/java/net/runelite/client/plugins/demonicgorilla/DemonicGorillaPlugin.java
 *
 *  Gorilla object - updated in a separate thread
 *      separate thread will update the current state based on other factors within the object
 *      thread-safe getters for the current state
 *      main thread will respond to whatever state is returned in the getter
 * State thread will only monitor one gorilla, set by the state thread
 *      the state thread needs some type of atomic bool that can tell main thread to stop doing whatever it needs to be doing
 *      an 'interrupt' type opcode
 * State thread will need to
 *      track projectiles from the gorilla
 *      track damage applied to the gorilla
 *      have some type of tick counter/tracker
 *      determine what gorilla we're currently attacking
 *
 *
 *  Current protection only changes after 50 damage is applied, it isn't on a rotation
 *  only the style the gorilla attacks with is dynamic and based on what's happened
*/

namespace Gorillas
{
    enum STATE { };
    enum STYLE { RANGED, MELEE, MAGIC };

    class Gorilla
    {
    public:

        static Gorilla& GetInstance();

        void SetGorilla(const Interactable::NPC& G);
        void SetGorilla(const Interactable::NPC& G, std::int32_t Index);
        void SetPlayer(const Interactable::Player& P);
        const Interactable::NPC& GetGorilla() const;
        const Interactable::Player& GetPlayer() const;

        std::int32_t GetState() const;
        std::vector<STYLE> GetPredictedStyles() const;

        std::int32_t GetProtectionStyle() const;
        Internal::Character GetInteracting() const;
        Interactable::Player GetTarget() const;
        Interactable::NPC GetPlayerTarget() const;
        Tile GetTrueLocation() const;

        bool IsDead() const;
        bool InMemory();

        void Find();
        void Listen();

        void OnAnimation(std::int32_t A);
        void OnProjectile(Interactable::Projectile& P);
        void OnHitsplat(std::int32_t Tick, std::int32_t Value);

        void Draw() const;

    private:
        Gorilla() = default;
        ~Gorilla() = default;

        std::atomic<std::int32_t> LastAnimationID = -1;
        std::atomic<std::int32_t> LastProjectileID = -1;
        std::atomic<std::int32_t> LastPlayerHitsplatTick = -1;

        std::atomic<std::int32_t> CurrentGorillaIndex = -1;
        std::atomic<std::int32_t> CurrentLocalPlayerIndex = -1;

        Interactable::NPC GorillaNPC = Interactable::NPC(nullptr);
        Interactable::Player Player = Interactable::Player(nullptr);

        std::mutex m_NPCIndicesLock;
        std::vector<std::int32_t> CachedNPCIndices;
        void CacheIndices();
        bool IndexInCache(std::int32_t Index);
        std::int32_t GetNPCIndexOf(const Interactable::NPC& N);

        static bool IndexIsGorilla(std::int32_t Index);
        static bool IsDead(const Interactable::NPC& N);

        std::vector<Interactable::NPC> GetGorillas();

        std::mutex m_ActiveProjectilesLock;
        std::vector<Interactable::Projectile> ActiveProjectiles;
        void AddActiveProjectile(Interactable::Projectile& P);
        bool IsProjectileActive(const Interactable::Projectile& P);
    };
}


#endif // GORILLA_OLD_HPP_INCLUDED