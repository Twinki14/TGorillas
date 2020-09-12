#include "Banking.hpp"
#include "Listeners/GameListener.hpp"
#include "../Config.hpp"
#include <TProfile.hpp>
#include <GearSets.hpp>
#include <Food.hpp>
#include <Game/Interfaces/Mainscreen.hpp>
#include <Game/Interfaces/Bank.hpp>
#include <Game/Interfaces/GameTabs/Inventory.hpp>
#include <Utilities/Bank.hpp>
#include <Utilities/Inventory.hpp>
#include <Game/Interfaces/GameTabs/Equipment.hpp>
#include <TScript.hpp>

namespace Banking
{
    namespace
    {
        bool ClearInventory(std::int32_t Amount)
        {
            if (!Bank::IsOpen()) return false;
            if (Amount <= 0) return true;
            if (Inventory::CountEmpty() >= Amount) return true;
            const auto FoodItems = Inventory::GetItems(Food::GetItemID((Food::FOOD) Config::Get("Food").as_integer<int>()));
            if (FoodItems.size() >= Amount)
                return Bank::SmartDeposit(FoodItems.back(), Amount); // TODO
            return Bank::DepositAll();
        }
    }
}


bool Banking::Withdraw(const Supplies::SUPPLY_ITEMS_SNAPSHOT& Snapshot)
{
    static const auto AnglerfishID = Food::GetItemID(Food::ANGLERFISH);

    enum INVENTORY_SETUP
    {
        SETUP_1, // https://i.imgur.com/mYCVPH7.png
        SETUP_2, // https://i.imgur.com/AnXrwyn.png
        SETUP_3 // https://i.imgur.com/cWfbpJL.png
    };

    static std::vector<int32_t> RangedGearIDs;
    static std::vector<int32_t> MeleeGearIDs;

/*    if (RangedGearIDs.empty())
    {
        for (std::uint32_t I = Equipment::HEAD; I <= Equipment::AMMO; I++)
        {
            if (GearSets::Sets["Melee"].Items[I].Name != "NULL" && GearSets::Sets["Ranged"].Items[I].Name != "NULL")
            {
                if (GearSets::Sets["Melee"].Items[I].Name != GearSets::Sets["Ranged"].Items[I].Name)
                {
                    MeleeGearIDs.emplace_back(GearSets::Sets["Melee"].Items[I].ID);
                    RangedGearIDs.emplace_back(GearSets::Sets["Ranged"].Items[I].ID);

                    DebugLog("Melee > {}, {}", GearSets::Sets["Melee"].Items[I].Name, GearSets::Sets["Melee"].Items[I].ID);
                    DebugLog("Ranged > {}, {}", GearSets::Sets["Ranged"].Items[I].Name, GearSets::Sets["Ranged"].Items[I].ID);
                }
            }
        }
    }*/

    static auto PlayerPreference = Profile::GeneratePlayerUnique(GameListener::GetLocalPlayerName(), "InventorySetup", SETUP_1, SETUP_3);

    while (true)
    {
        if (!Snapshot.NonSupplyItems_Inv.empty()) return false;
        const auto InventoryContainer = Inventory::GetContainerItems();




        switch (PlayerPreference)
        {
            case SETUP_1:
            {
                static const auto SwapSlots = SlotSet(std::set<uint32_t> { 0, 5, 9, 13 });
                static const auto DivineSlots = SlotSet(std::set<uint32_t> { 1, 2 });
                static const auto RestoreSlots = SlotSet(std::set<uint32_t> { 4, 6, 7, 8, 10, 11, 12 });

                if (Snapshot.MeleeSet_Equipped)
                {
                    static std::vector<int32_t> GearIDs;
/*                    if (GearIDs.empty())
                    {
                        for (std::uint32_t I = Equipment::HEAD; I <= Equipment::AMMO; I++)
                        {
                            if (GearSets::Sets["Melee"].Items[I].Name != "NULL")
                            {
                                if (GearSets::Sets["Melee"].Items[I].Name != GearSets::Sets["Ranged"].Items[I].Name)
                                    GearIDs.emplace_back(GearSets::Sets["Ranged"].Items[I].ID);
                            }
                        }
                    }*/


                } else if (Snapshot.RangedSet_Equipped)
                {
                    static std::vector<int32_t> GearIDs;
/*                    if (GearIDs.empty())
                    {
                        for (std::uint32_t I = Equipment::HEAD; I <= Equipment::AMMO; I++)
                        {
                            if (GearSets::Sets["Ranged"].Items[I].Name != "NULL")
                            {
                                if (GearSets::Sets["Ranged"].Items[I].Name != GearSets::Sets["Melee"].Items[I].Name)
                                    GearIDs.emplace_back(GearSets::Sets["Melee"].Items[I].ID);
                            }
                        }
                    }*/
                } else
                {

                }


            } break;

            case SETUP_2:
            {

            } break;

            case SETUP_3:
            {

            } break;
            default: break;
        }
    }
    return false;
}