#ifndef BANKING_HPP_INCLUDED
#define BANKING_HPP_INCLUDED

#include <utility>
#include <set>


#include "Supplies.hpp"

namespace Banking
{
    namespace
    {
        class SlotSet
        {
        public:
            SlotSet() = default;
            explicit SlotSet(std::set<uint32_t> Slots) : Slots(std::move(Slots)) { }

            [[nodiscard]] std::vector<int32_t> GetSlotIDs(const std::vector<int32_t>& InventoryIDs) const
            {
                std::vector<int32_t> Result;
                if (InventoryIDs.empty()) return Result;

                for (const auto& Index : Slots)
                    Result.emplace_back(InventoryIDs[Index]);

                return Result;
            }

            [[nodiscard]] const std::set<uint32_t>& GetSlots() const { return Slots; }
            ~SlotSet() = default;
        private:
            std::set<uint32_t> Slots;
        };

        bool ClearInventory(std::int32_t Amount);
    }
    bool Withdraw(const Supplies::SUPPLY_ITEMS_SNAPSHOT& Snapshot);
};

#endif // BANKING_HPP_INCLUDED