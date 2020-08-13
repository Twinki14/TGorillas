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

Tile TrackedPlayer::GetTrueLocation() const
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

bool TrackedPlayer::HealthBarShowing() const
{
    return Internal::GetHealthPercentage(*this) != -1.00;
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