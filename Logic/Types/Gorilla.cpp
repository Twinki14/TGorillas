#include <Game/Core.hpp>
#include "Gorilla.hpp"

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

bool Gorilla::IsDead() const
{
    return this->LastHealthPercentage == 0.00 || Internal::GetHealthPercentage(*this) == 0.00 || this->GetAnimationID() == Globals::Gorillas::ANIMATION_DYING;
}

bool Gorilla::HealthBarShowing() const
{
    return Internal::GetHealthPercentage(*this) != -1.00;
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
    if (NextPossibleAttackStyles & BOULDER_FLAG) Styles++;
    return Styles;
}

void Gorilla::Draw() const
{
    Paint::DrawConvex(this->GetConvex(), 0, 255, 255, 255);
    Paint::DrawTile(this->GetTrueLocation(), 0, 255, 255, 255);
}
