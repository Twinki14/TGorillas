#ifndef GORILLAS_HPP_INCLUDED
#define GORILLAS_HPP_INCLUDED

#include <cstdint>
#include <Core/Paint.hpp>
#include <Game/Core.hpp>
#include "GorillaOld.hpp"

namespace Gorillas
{
    bool IsGorilla(std::int32_t ID);








    std::int32_t GetCurrentlyEquippedStyle();

    void DrawGorilla();
    void AccessGorilla();
}

#endif // GORILLAS_HPP_INCLUDED