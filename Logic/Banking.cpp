#include "Banking.hpp"
#include "Listeners/GameListener.hpp"
#include "../Config.hpp"
#include <TScript.hpp>
#include <TProfile.hpp>
#include <GearSets.hpp>
#include <Food.hpp>
#include <Game/Core.hpp>
#include <Utilities/Bank.hpp>
#include <Utilities/Inventory.hpp>
#include <Utilities/Containers.hpp>
#include <Utilities/Antiban.hpp>

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


bool Banking::Withdraw(Supplies::Snapshot& Snapshot)
{
    static const auto AnglerfishID = Food::GetItemID(Food::ANGLERFISH);

    // - move to Supplies
    // - Withdraw if it isn't in inventory
    // - recharge blowpipe
    // - Organize items - should also move anglerfish - have this be an optional skip - so withdrawing won't require gear items to be in the right spot
    // - Withdraw Anglerfish recharge blowpipe
    // - Organize items - force, and double check as it's required to withdraw the rest
    // - recharge blowpipe

/*        std::vector<std::int32_t> SwapItemIDs;
        std::vector<OSRSBox::Items::Item> SwapItems = Snapshot.MeleeSet_Equipped ? SwapMeleeItems : SwapRangedItems;
        std::for_each(SwapItems.begin(), SwapItems.end(), [&](const OSRSBox::Items::Item& I) { return SwapItemIDs.emplace_back(I.id); });
        std::vector<std::int32_t> SwapItemIDsAndAnglerfish = SwapItemIDs;
        SwapItemIDsAndAnglerfish.emplace_back(AnglerfishID);*/

/*    static const auto SwapSlots = SlotSet(std::set<uint32_t> { 0, 4, 8, 12 });
    static const auto DivineSlots = SlotSet(std::set<uint32_t> { 1, 2 });
    static const auto RestoreSlots = SlotSet(std::set<uint32_t> { 3, 5, 6, 7, 8, 9, 10 });*/

    return false;
}

bool Banking::Open(const Tile& Override)
{
    static auto LastPassivity = Profile::GetInt(Profile::Var_Passivity);
    auto Passivity = Profile::GetInt(Profile::Var_Passivity);
    if (!Antiban::Tasks.count("BANK_TABOUT") || LastPassivity != Passivity)
    {
/*        Antiban::Task T;
        LastPassivity = Passivity;
        switch (Passivity)
        {
            case Profile::PASSIVITY_EXHILARATED: T = Antiban::Task(8000, 0.00, 0.08); break; // 30-45 times an hour
            case Profile::PASSIVITY_HYPER: T = Antiban::Task(8000, 0.00, 0.05); break; // 20-30 times an hour
            case Profile::PASSIVITY_MILD: T = Antiban::Task(8000, 0.00, 0.3); break; // 10-20 times an hour

            default:
            case Profile::PASSIVITY_MELLOW:
            case Profile::PASSIVITY_DISINTERESTED:  T = Antiban::Task(8000, 0.00, 0.01); break; // 0-10 times an hour
        }*/
        Antiban::Task T = Antiban::Task(60000, 0.00, 0.35);;
        Antiban::Tasks.insert_or_assign("BANK_TABOUT", std::move(T));
    }

    if (Bank::IsOpen()) return true;

    Interactable::NPC NPC;
    Interactable::GameObject Object;

    if (!Override)
    {
        NPC = NPCs::Get(Globals::NPCS_BANKER, 20);
        Object = GameObjects::Get(Globals::GAMEOBJECTS_BANKS, 20);
    } else
    {
        NPC = NPCs::Get(Override);
        Object = GameObjects::Get(Override);
    }

    if (Object && Object.GetVisibility() <= 0.25) Object = Interactable::GameObject(nullptr);
    if (NPC && Object && Object.GetVisibility() <= 0.65) Object = Interactable::GameObject(nullptr);
    if (NPC && Object && UniformRandom() <= 0.15 && NPC.GetVisibility() <= 0.50) Object = Interactable::GameObject(nullptr);
    if (NPC && NPC.GetVisibility() <= 0.10) NPC = Interactable::NPC(nullptr);
    if (!NPC && !Object)
    {
        DebugLog("Failed > No known bank nearby");
        return false;
    }

    if (!Object) DebugLog("Interacting with NPC");
    if (Object) DebugLog("Interacting with Object");

    const auto DistFrom = [&Object, &NPC]() -> double
    {
        return Minimap::GetPosition().DistanceFrom(((bool) Object) ? Object.GetTile() : NPC.GetTile());
    };

    const auto ClickBank = [&Object, &NPC]() -> bool
    {
        Script::SetStatus("Opening bank > Clicking");
        return ((bool) Object) ? Object.Interact(std::vector<std::string> { "Bank " + Object.GetName(), "Use " + Object.GetName() }) :
               NPC.Interact("Bank " + NPC.GetName());
    };

    Script::SetStatus("Opening bank");
    bool Clicked = ClickBank();

    if (Clicked)
    {
        Countdown Failsafe = Countdown(16000);
        while (Failsafe)
        {
            if (Terminate) return false;

            if (Bank::IsOpen())
                return true;

            if (Mainscreen::IsMoving())
            {
                if (Antiban::RunTask("BANK_TABOUT"))
                {
                    Script::SetStatus("Opening bank > Tabbing out");
                    if (UniformRandom() <= 0.75)
                        Antiban::MouseOffClient();
                    else
                        Antiban::LoseClientFocus();
                    Script::SetStatus("Opening bank > Tabbed out");
                }

                if (DistFrom() >= 4)
                {
                    if (UniformRandom() <= 0.15) ClickBank();
                }
            } else
            {
                Countdown C = Countdown(Antiban::GenerateDelayFromPassivity(1750, 3250, 2.2, 0.10));
                while (C)
                {
                    if (Terminate) return false;
                    if (Bank::IsOpen()) return true;
                    if (Mainscreen::IsMoving()) break;
                    Wait(100);
                }

                if (C.IsFinished())
                    return false;
            }
            Antiban::DelayFromPassivity(175, 325, 3.0, 0.10);
        }
    }
    return false;
}
