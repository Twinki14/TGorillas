#ifndef TRACKEDPLAYER_HPP_INCLUDED
#define TRACKEDPLAYER_HPP_INCLUDED

#include "Hitsplat.hpp"
#include "WorldArea.hpp"
#include <Game/Interactable/Player.hpp>
#include <shared_mutex>

class TrackedPlayer : public Interactable::Player
{
public:
    TrackedPlayer();
    TrackedPlayer(Interactable::Player& P, std::int32_t Index);

    void AddHitsplat(const Hitsplat& H);
    void SetLastWorldArea(std::shared_ptr<WorldArea> Area);

    std::shared_ptr<Hitsplat> GetLastHitsplat() const;
    std::shared_ptr<WorldArea> GetLastWorldArea() const;
    WorldArea GetWorldArea() const;

    bool HasRecentHitsplats();
    bool BlockedRecentHitsplat();
    void ClearRecentHitsplats();



    std::int32_t GetIndex() const;

private:

    std::shared_ptr<Hitsplat> LastHitsplat;
    std::shared_ptr<WorldArea> LastWorldArea;

    std::shared_mutex HitsplatsLock;
    std::vector<Hitsplat> RecentHitsplats;

    std::int32_t Index = -1;
};

#endif // TRACKEDPLAYER_HPP_INCLUDED