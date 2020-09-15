#ifndef BANKING_HPP_INCLUDED
#define BANKING_HPP_INCLUDED

#include <utility>
#include <set>
#include <Core/Types/Tile.hpp>
#include <Utilities/Containers.hpp>
#include "Supplies.hpp"

namespace Globals
{
    // NPCs
    const std::vector<std::string> NPCS_BANKER = { "Banker" };

    // Gameobjects
    const std::vector<std::string> GAMEOBJECTS_BANKS = { "Bank booth", "Bank chest", "Chest" };

    // WallObjects
    const std::vector<std::string> WALLOBJECTS_BANKS = { "Grand Exchange booth" };
}


namespace Banking
{
    bool Open(const Tile& Override = Tile());
    bool Withdraw(Supplies::Snapshot& Snapshot);
};

#endif // BANKING_HPP_INCLUDED