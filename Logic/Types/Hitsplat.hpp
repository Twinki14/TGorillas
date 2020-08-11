#ifndef HITSPLAT_HPP_INCLUDED
#define HITSPLAT_HPP_INCLUDED

#include <cstdint>
#include <Core/Internal.hpp>

class Hitsplat
{
public:
    Hitsplat() = default;

    Hitsplat(std::uint32_t Damage, std::int32_t Type, std::int32_t EndCycle) : Damage(Damage), Type(Type),
                                                                               EndCycle(EndCycle)
    { }

    enum TYPE
    {
        BLOCK_ME = 12,
        BLOCK_OTHER = 13,
        DAMAGE_ME = 16,
        DAMAGE_OTHER = 17,
        POISON = 2,
        DISEASE = 4,
        VENOM = 5,
        HEAL = 6,
        DAMAGE_ME_CYAN = 18,
        DAMAGE_OTHER_CYAN = 19,
        DAMAGE_ME_ORANGE = 20,
        DAMAGE_OTHER_ORANGE = 21,
        DAMAGE_ME_YELLOW = 22,
        DAMAGE_OTHER_YELLOW = 23,
        DAMAGE_ME_WHITE = 24,
        DAMAGE_OTHER_WHITE = 25,
    };

    std::uint32_t GetDamage() const { return this->Damage; };
    std::int32_t GetType() const { return this->Type; };
    std::int32_t GetEndCycle() const { return this->EndCycle; };
    void SetEnded(bool R) { this->ForceEnded = R; };

    bool IsLocals() const
    {
        switch (this->GetType())
        {
            case BLOCK_ME:
            case DAMAGE_ME:
            case DAMAGE_ME_CYAN:
            case DAMAGE_ME_YELLOW:
            case DAMAGE_ME_ORANGE:
            case DAMAGE_ME_WHITE: return true;
            default: return false;
        }
    }

    bool IsOthers() const
    {
        switch (this->GetType())
        {
            case BLOCK_OTHER:
            case DAMAGE_OTHER:
            case DAMAGE_OTHER_CYAN:
            case DAMAGE_OTHER_YELLOW:
            case DAMAGE_OTHER_ORANGE:
            case DAMAGE_OTHER_WHITE: return true;
            default: return false;
        }
    }

    bool IsRecent() const
    {
        if (this->ForceEnded) return false;
        return this->IsRecent(Internal::GetGameTick());
    }

    bool IsRecent(std::int32_t GameTick) const
    {
        if (this->ForceEnded) return false;
        if (GameTick >= this->EndCycle) return true;
        return false;
    }

    ~Hitsplat() = default;
private:
    std::uint32_t Damage = 0;
    std::int32_t Type = -1;
    std::int32_t EndCycle = 0;
    bool ForceEnded = false;
};

#endif // HITSPLAT_HPP_INCLUDED