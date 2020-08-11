#include "TrackedPlayer.hpp"

#include <utility>

TrackedPlayer::TrackedPlayer() : Interactable::Player(nullptr)
{
    this->LastHitsplat = std::make_shared<Hitsplat>(Hitsplat());
    this->LastWorldArea = std::make_shared<WorldArea>(WorldArea());
}

TrackedPlayer::TrackedPlayer(Interactable::Player& P, std::int32_t Index) : Interactable::Player(P), Index(Index)
{
    this->LastHitsplat = std::make_shared<Hitsplat>(Hitsplat());
    this->LastWorldArea = std::make_shared<WorldArea>(WorldArea());
}

void TrackedPlayer::AddHitsplat(const Hitsplat& H)
{
    if (H.GetEndCycle() > LastHitsplat->GetEndCycle())
    {
        this->LastHitsplat = std::make_shared<Hitsplat>(Hitsplat(H));
        std::unique_lock Lock(HitsplatsLock);
        this->RecentHitsplats.emplace_back(H);
    }
}

void TrackedPlayer::SetLastWorldArea(std::shared_ptr<WorldArea> Area)
{
    this->LastWorldArea = std::move(Area);
}

std::shared_ptr<Hitsplat> TrackedPlayer::GetLastHitsplat() const
{
    return LastHitsplat;
}

std::shared_ptr<WorldArea> TrackedPlayer::GetLastWorldArea() const
{
    return LastWorldArea;
}

WorldArea TrackedPlayer::GetWorldArea() const
{
    return WorldArea(*this);
}

bool TrackedPlayer::HasRecentHitsplats()
{
    std::shared_lock Lock(HitsplatsLock);
    return !RecentHitsplats.empty();
}

bool TrackedPlayer::BlockedRecentHitsplat()
{
    std::shared_lock Lock(HitsplatsLock);
    return std::any_of(RecentHitsplats.begin(), RecentHitsplats.end(), [](const Hitsplat& H)
    { return H.GetType() == Hitsplat::BLOCK_ME; });
}

void TrackedPlayer::ClearRecentHitsplats()
{
    std::unique_lock Lock(HitsplatsLock);
    RecentHitsplats.clear();
}

std::int32_t TrackedPlayer::GetIndex() const
{
    return Index;
}