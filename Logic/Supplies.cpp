#include <Utilities/GearSets.hpp>
#include <Game/Core.hpp>
#include <TScript.hpp>
#include <Utilities/Inventory.hpp>
#include <Tools/Prices.hpp>
#include <Utilities/Bank.hpp>
#include <Utilities/Runes.hpp>
#include <Utilities/Containers.hpp>
#include <Utilities/Antiban.hpp>
#include "Supplies.hpp"
#include "../Config.hpp"
#include "Listeners/GameListener.hpp"
#include "Banking.hpp"


namespace Supplies
{
    namespace
    {
        std::int32_t CountAlchemyCasts()
        {
            const auto FireRunes = Runes::Count(Runes::FIRE, Runes::INVENTORY | Runes::POUCH);
            const auto NatureRunes = Runes::Count(Runes::NATURE, Runes::INVENTORY | Runes::POUCH);
            std::vector<std::int32_t> Runes = { (std::int32_t) std::floor(FireRunes / 5), NatureRunes };
            auto min_ele = std::min_element(Runes.begin(), Runes.end());
            if (min_ele != Runes.end())
                return *min_ele;
            return 0;
        }

        Supplies::Potion CountPotions(const std::string& PotionName, const Containers::Container& Container)
        {
            const auto DegradableNames = Globals::GetDegradableArray(PotionName, 1, 4);
            std::vector<std::uint32_t> Count = Container.CountIndividual(DegradableNames);

            std::array<std::uint32_t, 5> Result{};
            for (int I = 1; I < Result.size(); ++I)
            {
                Result[I] = Count[I - 1];
                Result[0] += (Result[I] * I);
            }
            return Supplies::Potion { (std::int32_t) Result[0], Result[1], Result[2], Result[3], Result[4] };
        }

        void SetWhitelist(bool Force = false)
        {
            if (!Force && !Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.empty() && !Supplies::SUPPLY_ITEMS_WHITELIST_ITEM_IDS.empty())
                return;

            const auto StatRefreshMethod = Config::Get("StatRefreshMethod").as_integer<int>();
            const auto UseDivinePots = Config::Get("UseDivinePots").as_bool();
            const auto UseRunePouch = Config::Get("UseRunePouch").as_bool();
            const auto UseHighAlchemy = Config::Get("UseHighAlchemy").as_bool();
            const auto FoodCfg = (Food::FOOD) Config::Get("Food").as_integer<int>();

            Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.clear();
            Supplies::SUPPLY_ITEMS_WHITELIST_ITEM_IDS.clear();

            for (auto& D : Globals::GetDegradableArray(Globals::ITEM_PRAYER_POTION, 1, 4))
                Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace(std::move(D));

            for (auto& D : Globals::GetDegradableArray(Globals::ITEM_SUPER_RESTORE, 1, 4))
                Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace(std::move(D));

            for (auto& D : Globals::GetDegradableArray((UseDivinePots) ? Globals::ITEM_DIVINE_RANGING_POTION : Globals::ITEM_RANGING_POTION, 1, 4))
                Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace(std::move(D));

            for (auto& D : Globals::GetDegradableArray((UseDivinePots) ? Globals::ITEM_DIVINE_SUPER_COMBAT_POTION : Globals::ITEM_SUPER_COMBAT_POTION, 1, 4))
                Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace(std::move(D));

            if (GearSets::Sets.count("Melee"))
            {
                for (std::uint32_t I = Equipment::HEAD; I <= Equipment::AMMO; I++)
                {
                    if (GearSets::Sets["Melee"].Items[I])
                    {
                        Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace(GearSets::Sets["Melee"].Items[I].GetName());

                        if (GearSets::Sets["Melee"].Items[I].IsDegradable())
                            for (const auto& Degradable : GearSets::Sets["Melee"].Items[I].GetDegradedAliases())
                                Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace(Degradable.Name);

                        if (GearSets::Sets["Melee"].Items[I].IsChargeable())
                            for (const auto& UnCharged : GearSets::Sets["Melee"].Items[I].GetUnchargedAliases())
                                Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace(UnCharged.Name);
                    }

                }
            }

            if (GearSets::Sets.count("Ranged"))
            {
                for (std::uint32_t I = Equipment::HEAD; I <= Equipment::AMMO; I++)
                {
                    if (GearSets::Sets["Ranged"].Items[I])
                    {
                        Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace(GearSets::Sets["Ranged"].Items[I].GetName());

                        if (GearSets::Sets["Ranged"].Items[I].IsDegradable())
                            for (const auto& Degradable : GearSets::Sets["Ranged"].Items[I].GetDegradedAliases())
                                Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace(Degradable.Name);

                        if (GearSets::Sets["Ranged"].Items[I].IsChargeable())
                            for (const auto& UnCharged : GearSets::Sets["Ranged"].Items[I].GetUnchargedAliases())
                                Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace(UnCharged.Name);
                    }
                }
            }

            if (GearSets::Sets.count("Special") && GearSets::Sets["Special"].Items[Equipment::WEAPON].GetName() != "NULL")
                Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace(GearSets::Sets["Special"].Items[Equipment::WEAPON].GetName());

            switch (StatRefreshMethod)
            {
                case Config::CLAN_WARS:
                {
                    for (auto& D : Globals::GetDegradableArray(Globals::ITEM_RING_OF_DUELING, 1, 8))
                        Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace(std::move(D));
                } break;

                case Config::POH:
                {
                    // TODO
                } break;
                default: break;
            }

            if (UseRunePouch) Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace(Globals::ITEM_RUNE_POUCH);

            Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace(Food::GetName(FoodCfg));
            Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace(Food::GetName(Food::SHARK));
            Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace(Globals::ITEM_ROYAL_SEED_POD);
        }

        std::vector<Supplies::ItemRecord> GetNonSupplyItems(const std::vector<CONTAINER_ITEM>& InventoryItems)
        {
            SetWhitelist(false);

            // TODO Adapt this to use OSRSBox::Items::Item
            static std::map<std::int32_t, bool> StackableMap; // Workaround for Internal::ItemInfo issues

            std::vector<Supplies::ItemRecord> Result;
            for (const auto& Item : InventoryItems)
            {
                if (Item.ID < 0) continue;

                if (Supplies::InWhitelist(Item.Name) || Supplies::InWhitelist(Item.ID)) continue;

                const auto PriceInfo = Prices::Get(Item.Name, true, false);

                Supplies::ItemRecord NonSupplyItem;
                NonSupplyItem.ID = Item.ID;
                NonSupplyItem.Name = Item.Name;
                NonSupplyItem.Item = Interactable::Item(Item.Index, Item.ID, Interactable::Item::INVENTORY);
                NonSupplyItem.Stackable = StackableMap.count(Item.ID) || Internal::IsItemStackable(Item.ID) || NonSupplyItem.Item.GetInfo().GetStackable();
                NonSupplyItem.StackAmount = NonSupplyItem.Item.GetStackAmount();
                NonSupplyItem.ExchangeValue = PriceInfo.AveragePrice;
                NonSupplyItem.TotalExchangeValue = NonSupplyItem.StackAmount * NonSupplyItem.ExchangeValue;
                NonSupplyItem.HighAlchValue = PriceInfo.HighAlchemyValue;
                NonSupplyItem.HighAlchProfit = 0;
                NonSupplyItem.ShouldAlch = false;

                static const auto UseHighAlchemy = Config::Get("UseHighAlchemy").as_bool();
                static const auto MinimumHighAlchemyProfit = Config::Get("MinimumHighAlchemyProfit").as_integer<int>();
                if (NonSupplyItem.ExchangeValue > 0 && NonSupplyItem.HighAlchValue > 0 && !NonSupplyItem.Stackable)
                {
                    NonSupplyItem.HighAlchProfit = (NonSupplyItem.HighAlchValue + Prices::GetHighAlchemyCost(Globals::ITEM_LAVA_RUNE_ID)) - NonSupplyItem.ExchangeValue;
                    NonSupplyItem.ShouldAlch = UseHighAlchemy && NonSupplyItem.HighAlchProfit >= MinimumHighAlchemyProfit;
                }

                if (NonSupplyItem.Stackable && !StackableMap.count(Item.ID))
                    StackableMap[Item.ID] = true;

                Result.emplace_back(std::move(NonSupplyItem));
            }
            return Result;
        }

        bool SetWithdrawQuantity(bool All = false)
        {
            if (Bank::GetDefaultWithdrawQuantity() != 1)
            {
                if (!Bank::SetDefaultWithdrawQuantity(1))
                    return false;

                auto DefaultQuantityChanged = [&]() -> bool
                {
                    if (Bank::GetDefaultWithdrawQuantity() == 1)
                    {
                        const auto W = Bank::GetFirstItemWidget();
                        const auto Actions = W.GetActions();
                        return !Actions.empty() && Actions.front().find((All ? "Withdraw-All" : "Withdraw-1")) != std::string::npos;
                    }
                    return false;
                };

                if (!WaitFunc(1250, 250, DefaultQuantityChanged, true))
                    return false;
            }
            return Bank::GetDefaultWithdrawQuantity() == (All ? Bank::ALL : 1);
        }

        class SlotSet : public Containers::Container
        {
        public:
            SlotSet() = default;

            explicit SlotSet(std::set<uint32_t> Slots) : Slots(std::move(Slots)), Container([&]() -> Containers::Container
                                               {
                                                   auto IDs = Inventory::GetItemIDs();
                                                   auto Names = Inventory::GetItemNames();
                                                   auto Amounts = Inventory::GetItemAmounts();
                                                   if (IDs.size() != Names.size() || IDs.size() != Amounts.size())
                                                       return Container();

                                                   std::vector<Containers::Item> SlotItems;
                                                   for (const auto& Slot : Slots)
                                                       SlotItems.emplace_back(Containers::Item(IDs[Slot], std::move(Names[Slot]), Amounts[Slot], Slot, Containers::INVENTORY));
                                                   return Container(SlotItems, Containers::INVENTORY);
                                               }())
            { }

            const std::set<uint32_t>& GetSlots() const { return Slots; }
            ~SlotSet() = default;

        private:
            std::set<uint32_t> Slots;
        };

        SlotSet& GetSwapSlots()
        {
            static SlotSet Swaps;
            if (Swaps.GetSlots().empty())
            {
                switch (Supplies::GetInventoryLayout())
                {
                    case LAYOUT_1: // https://i.imgur.com/mYCVPH7.png
                        Swaps = SlotSet(std::set<uint32_t> { 0, 4, 8, 12 }); break;
                    case LAYOUT_2: // https://i.imgur.com/AnXrwyn.png
                        break; // TODO
                    case LAYOUT_3: // https://i.imgur.com/cWfbpJL.png
                        break; // TODO
                    default: break;
                }
            }
            return Swaps;
        }
    }

    Supplies::Snapshot GetSnapshot(bool InventoryOnly)
    {
        static const auto UseRunePouch = Config::Get("UseRunePouch").as_bool();
        static const auto UsePrayerPots = Config::Get("UsePrayerPots").as_bool();
        static const auto UseDivinePots = Config::Get("UseDivinePots").as_bool();
        static const auto UseHighAlchemy = Config::Get("UseHighAlchemy").as_bool();
        static const auto Food = (Food::FOOD) Config::Get("Food").as_integer<int>();
        static const auto FoodID = Food::GetItemID(Food);
        static const auto SharkID = Food::GetItemID(Food::SHARK);

        Supplies::Snapshot Result = { };

        if (!Mainscreen::IsLoggedIn()) return Result;

        bool BankOpen = !InventoryOnly && Bank::IsOpen();
        Result.BankContainer = BankOpen ? Containers::Container(Containers::BANK) : Containers::Container();
        Result.InventoryContainer = Containers::Container(Containers::INVENTORY);

        if (GearSets::Sets.count("Melee"))
        {
            Result.MeleeSet_Holding = GearSets::Sets["Melee"].Holding();
            Result.MeleeSet_Equipped = GearSets::Sets["Melee"].Equipped();
        }

        if (GearSets::Sets.count("Ranged"))
        {
            Result.RangedSet_Holding = GearSets::Sets["Ranged"].Holding();
            Result.RangedSet_Equipped = GearSets::Sets["Ranged"].Equipped();
        }

        if (GearSets::Sets.count("Special"))
        {
            Result.SpecialSet_Holding = GearSets::Sets["Special"].Holding();
            Result.SpecialSet_Equipped = GearSets::Sets["Special"].Equipped();
        }

        Result.HasRunePouch_Inv     = Result.InventoryContainer.Contains(Globals::ITEM_RUNE_POUCH);
        Result.HasRoyalSeedPod_Inv  = Result.InventoryContainer.Contains(Globals::ITEM_ROYAL_SEED_POD);

        Result.Potions_Inv_Restore  = CountPotions(Globals::ITEM_SUPER_RESTORE, Result.InventoryContainer);
        Result.Potions_Inv_Prayer   = CountPotions(Globals::ITEM_PRAYER_POTION, Result.InventoryContainer);
        Result.Potions_Inv_PrayerRestore =
                { Result.Potions_Inv_Restore.Total + Result.Potions_Inv_Prayer.Total,
                  Result.Potions_Inv_Restore.Dose_1 + Result.Potions_Inv_Prayer.Dose_1,
                  Result.Potions_Inv_Restore.Dose_2 + Result.Potions_Inv_Prayer.Dose_2,
                  Result.Potions_Inv_Restore.Dose_3 + Result.Potions_Inv_Prayer.Dose_3,
                  Result.Potions_Inv_Restore.Dose_4 + Result.Potions_Inv_Prayer.Dose_4, };

        Result.Potions_Inv_Ranging     = CountPotions((UseDivinePots) ? Globals::ITEM_DIVINE_RANGING_POTION : Globals::ITEM_RANGING_POTION, Result.InventoryContainer);
        Result.Potions_Inv_SuperCombat = CountPotions((UseDivinePots) ? Globals::ITEM_DIVINE_SUPER_COMBAT_POTION : Globals::ITEM_SUPER_COMBAT_POTION, Result.InventoryContainer);
        Result.Food_Inv                = Result.InventoryContainer.Count(FoodID);
        Result.Sharks_Inv              = Result.InventoryContainer.Count(SharkID);
        Result.HighAlchemyCasts_Inv    = CountAlchemyCasts();
        Result.EmptySlots_Inv          = Result.InventoryContainer.CountEmpty();

        //Result.NonSupplyItems_Inv = GetNonSupplyItems(InventoryContainerItems); TODO

        if (BankOpen)
        {
            Result.HasRunePouch_Bank    = Result.BankContainer.Contains(Globals::ITEM_RUNE_POUCH);
            Result.HasRoyalSeedPod_Bank = Result.BankContainer.Contains(Globals::ITEM_ROYAL_SEED_POD);

            Result.Potions_Bank_PrayerRestore   = CountPotions((UsePrayerPots) ? Globals::ITEM_PRAYER_POTION : Globals::ITEM_SUPER_RESTORE, Result.BankContainer);
            Result.Potions_Bank_Ranging         = CountPotions((UseDivinePots) ? Globals::ITEM_DIVINE_RANGING_POTION : Globals::ITEM_RANGING_POTION, Result.BankContainer);
            Result.Potions_Bank_SuperCombat     = CountPotions((UseDivinePots) ? Globals::ITEM_DIVINE_SUPER_COMBAT_POTION : Globals::ITEM_SUPER_COMBAT_POTION, Result.BankContainer);
            Result.Food_Inv = Result.BankContainer.Count(FoodID);
        }

        if (UseRunePouch)
        {
            Result.HasCorrectRunePouchRunes = Runes::Contains(Runes::LAW, Runes::POUCH);
            if (UseHighAlchemy) Result.HasCorrectRunePouchRunes = Runes::Contains({ Runes::LAVA, Runes::NATURE, Runes::LAW }, Runes::POUCH);
        } else
            Result.HasCorrectRunePouchRunes = true;

        return Result;
    }

    std::uint32_t GetInventoryLayout()
    {
        static auto PlayerPreference = Profile::GeneratePlayerUnique(GameListener::GetLocalPlayerName(), "InventorySetup", LAYOUT_1, LAYOUT_3);
        return PlayerPreference;
    }

    Supplies::GroupState GetState(const Supplies::Group& G)
    {
        return Supplies::GetState(G, Supplies::GetSnapshot(true));
    }

    Supplies::GroupState GetState(const Supplies::Group& G, const Supplies::Snapshot& Snapshot)
    {
        switch (G)
        {
            case GEAR:
            {
                if (!Snapshot.MeleeSet_Holding) return NOT_READY;
                if (!Snapshot.RangedSet_Holding) return NOT_READY;
                if (GearSets::Sets.count("Special") && !Snapshot.SpecialSet_Holding) return NOT_READY;
                return READY_OR_FULL;
            }

            case PRAYER_RESTORE:
            {
                static const auto SuperRestoreAmt = Config::Get("RestorePotionAmount").as_integer<int>() * 4;
                const double Dosages = ((Snapshot.Potions_Inv_PrayerRestore.Total + 0.00) / SuperRestoreAmt);
                return (Dosages == 1.00) ? READY_OR_FULL : NOT_READY;
            }

            case RANGING_POTION: return Snapshot.Potions_Inv_Ranging.Total == 4 ? READY_OR_FULL : NOT_READY;
            case SUPER_COMBAT: return Snapshot.Potions_Inv_SuperCombat.Total == 4 ? READY_OR_FULL : NOT_READY;

            case TELEPORTS: return Snapshot.HasRoyalSeedPod_Inv ? READY_OR_FULL : NOT_READY;

            case RUNES:
            {
                static const auto UseRunePouch = Config::Get("UseRunePouch").as_bool();
                static const auto UseHighAlchemy = Config::Get("UseHighAlchemy").as_bool();

                if (UseRunePouch) return Snapshot.HasRunePouch_Inv && Snapshot.HasCorrectRunePouchRunes ? READY_OR_FULL : NOT_READY;
                if (UseHighAlchemy) return Snapshot.HighAlchemyCasts_Inv >= 50 ? READY_OR_FULL : NOT_READY;
                return READY_OR_ADEQUATE;
            }

            case FOOD:
            {
                // inventory is full with manta ray, and empty swap slots are filled with anglerfish
            }

            default: break;
        }
        return Supplies::UNUSED;
    }

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

        std::vector<OSRSBox::Items::Item>& GetSwapRangedItems()
        {
            static std::vector<OSRSBox::Items::Item> SwapRangedItems;
            if (SwapRangedItems.empty())
            {
                for (std::uint32_t S = Equipment::HEAD; S <= Equipment::AMMO; S++)
                {
                    auto RangedItem = GearSets::Sets["Ranged"].Items[S];
                    auto MeleeItem = GearSets::Sets["Melee"].Items[S];

                    if (!RangedItem && !MeleeItem) continue;

                    if ((bool) RangedItem != (bool) MeleeItem)
                    {
                        if (RangedItem) SwapRangedItems.emplace_back(RangedItem.GetInfo());
                        continue;
                    }

                    if (RangedItem.GetID() != MeleeItem.GetID())
                    {
                        if (RangedItem) SwapRangedItems.emplace_back(RangedItem.GetInfo());
                        continue;
                    }
                }

                for (const auto& Ranged : SwapRangedItems)
                    DebugLog("Range swap > {} - {}", Ranged.id, Ranged.name);
            }
            return SwapRangedItems;
        }

        std::vector<OSRSBox::Items::Item>& GetSwapMeleeItems()
        {
            static std::vector<OSRSBox::Items::Item> SwapMeleeItems;
            if (SwapMeleeItems.empty())
            {
                for (std::uint32_t S = Equipment::HEAD; S <= Equipment::AMMO; S++)
                {
                    auto RangedItem = GearSets::Sets["Ranged"].Items[S];
                    auto MeleeItem = GearSets::Sets["Melee"].Items[S];

                    if (!RangedItem && !MeleeItem) continue;

                    if ((bool) RangedItem != (bool) MeleeItem)
                    {
                        if (MeleeItem) SwapMeleeItems.emplace_back(MeleeItem.GetInfo());
                        continue;
                    }

                    if (RangedItem.GetID() != MeleeItem.GetID())
                    {
                        if (MeleeItem) SwapMeleeItems.emplace_back(MeleeItem.GetInfo());
                        continue;
                    }
                }

                for (const auto& Melee : SwapMeleeItems)
                    DebugLog("Melee swap > {} - {}", Melee.id, Melee.name);
            }
            return SwapMeleeItems;
        }

        bool RechargeGear(Supplies::Snapshot& Snapshot)
        {
            return false;
        }

        bool OrganizeGear(Supplies::Snapshot& Snapshot)
        {
            return false;
        }

        bool WithdrawGear(Supplies::Snapshot& Snapshot)
        {
            if (!Snapshot.NonWhitelistedItems_Inv.empty()) return false;

            // - Withdraw if it isn't in inventory
            // - recharge blowpipe
            // - Organize items - should also move anglerfish - have this be an optional skip - so withdrawing won't require gear items to be in the right spot
            // - Withdraw Anglerfish recharge blowpipe
            // - Organize items - force, and double check as it's required to withdraw the rest
            // - recharge blowpipe

            if (!Snapshot.MeleeSet_Equipped && !Snapshot.RangedSet_Equipped)
            {
                if (Snapshot.MeleeSet_Holding && Snapshot.RangedSet_Holding)
                {
                    bool Melee = GetSwapMeleeItems().size() < GetSwapRangedItems().size();
                    std::string Set = Melee ? "Melee" : "Ranged";
                    Script::SetStatus("Equipping {} set", Set);
                    if (!GearSets::Sets[Set].Equip())
                    {
                        DebugLog("Failed to equip {} set", Set);
                        return false;
                    }
                    Snapshot = Supplies::GetSnapshot(true);
                } else
                {
                    if (Snapshot.MeleeSet_Holding)
                    {
                        Script::SetStatus("Equipping melee set");
                        if (!GearSets::Sets["Melee"].Equip())
                        {
                            DebugLog("Failed to equip melee set");
                            return false;
                        }
                        Snapshot = Supplies::GetSnapshot(true);
                    }

                    if (Snapshot.RangedSet_Holding)
                    {
                        Script::SetStatus("Equipping ranged set");
                        if (!GearSets::Sets["Ranged"].Equip())
                        {
                            DebugLog("Failed to equip ranged set");
                            return false;
                        }
                        Snapshot = Supplies::GetSnapshot(true);
                    }
                }
            }

            // if not all gear in inventory
            if (Supplies::GetState(GEAR, Snapshot) == NOT_READY)
            {
                // Set initial values - Reset becomes true whenever this function succeeds, so the next time this function is needed we have fresh new values
                static bool Reset = true;
                static bool WithdrawBothSets = false;
                if (Reset)
                {
                    if (!Antiban::Tasks.count("SUPPLIES_WITHDRAW_GEAR_BOTH"))
                        Antiban::AddTask("SUPPLIES_WITHDRAW_GEAR_BOTH", Antiban::Task(60000, 0.05, 0.35));

                    Reset = false;
                    WithdrawBothSets = Antiban::RunTask("SUPPLIES_WITHDRAW_GEAR_BOTH");
                }

                if (!Snapshot.MeleeSet_Equipped && !Snapshot.RangedSet_Equipped && !Inventory::IsEmpty())
                {
                    if (!Banking::Open())
                    {
                        DebugLog("Failed to open bank");
                        return false;
                    }

                    Script::SetStatus("Depositing inventory");
                    if (!Bank::DepositAll() || !WaitFunc(1750, 250, Inventory::IsEmpty, true))
                    {
                        DebugLog("Failed to deposit all");
                        return false;
                    }
                    Snapshot = Supplies::GetSnapshot(true);
                }

                // TODO - Recharge blowpipe

                if (WithdrawBothSets) // withdraw both before equipping
                {
                    Script::SetStatus("Withdrawing melee set");
                    if (!GearSets::Sets["Melee"].Withdraw())
                    {
                        DebugLog("Failed to withdraw melee set");
                        return false;
                    }

                    Script::SetStatus("Withdrawing ranged set");
                    if (!GearSets::Sets["Ranged"].Withdraw())
                    {
                        DebugLog("Failed to withdraw ranged set");
                        return false;
                    }

                    bool Melee = GetSwapMeleeItems().size() < GetSwapRangedItems().size();
                    std::string Set = Melee ? "Melee" : "Ranged";
                    Script::SetStatus("Equipping {} set", Melee ? "melee" : "ranged");
                    if (!GearSets::Sets[Set].Equip())
                    {
                        DebugLog("Failed to equip {} set", Melee ? "melee" : "ranged");
                        return false;
                    }
                } else // withdraw one - equip one - withdraw the other
                {
                    bool Melee = GetSwapMeleeItems().size() < GetSwapRangedItems().size();
                    std::string Set = Melee ? "Melee" : "Ranged";
                    std::string SecondSet = Melee ? "Ranged" : "Melee";

                    Script::SetStatus("Withdrawing {} set", Melee ? "melee" : "ranged");
                    if (!GearSets::Sets[Set].Withdraw())
                    {
                        DebugLog("Failed to withdraw {} set", Melee ? "melee" : "ranged");
                        return false;
                    }

                    Script::SetStatus("Equipping {} set", Melee ? "melee" : "ranged");
                    if (!GearSets::Sets[Set].Equip())
                    {
                        DebugLog("Failed to equip {} set", Melee ? "melee" : "ranged");
                        return false;
                    }

                    if (!Banking::Open())
                    {
                        DebugLog("Failed to re-open bank");
                        return false;
                    }

                    Script::SetStatus("Withdrawing {} set", Melee ? "ranged" : "melee");
                    if (!GearSets::Sets[SecondSet].Withdraw())
                    {
                        DebugLog("Failed to withdraw {} set", Melee ? "ranged" : "melee");
                        return false;
                    }
                }

                // Wait GetState is READY_OR_FULL
                Snapshot = Supplies::GetSnapshot(true);
                if (!WaitFunc(1750, 250, [&]()
                    {
                        Snapshot = Supplies::GetSnapshot(true);
                        return Supplies::GetState(GEAR, Snapshot) == READY_OR_FULL;
                    }, true))
                {
                    DebugLog("Failed waiting for expected GET_STATE after withdrawing gear");
                    return false;
                }
            }

            // if we have all gear in inventory - organize them, withdraw anglerfish to fill in empty swap slots
            if (Supplies::GetState(GEAR, Snapshot) == READY_OR_FULL)
            {
                // equip a set
            }

            return false;
        }
    }

    bool Withdraw(const Supplies::Group& G, bool& Changed, Supplies::Snapshot& Snapshot)
    {
        switch (G)
        {
            case GEAR: return WithdrawGear(Snapshot);
            case PRAYER_RESTORE:break;
            case RANGING_POTION:break;
            case SUPER_COMBAT:break;
            case TELEPORTS:break;
            case RUNES:break;
            case FOOD:break;
        }

        // Bank::Open() - TODO
/*        switch (Item)
        {
            case GEAR:
            {
                if (!Snapshot.MainSet_Equipped)
                {
                    if (!Snapshot.MainSet_Holding)
                    {
                        if (!Banking::Open())
                            return false;

                        if (Inventory::CountEmpty() < Config::MainSetItems)
                        {
                            if (Inventory::Count(Food::GetItemID(Config::GetFood())) >= Config::MainSetItems)
                            {
                                if (!Bank::Deposit(Food::GetItemID(Config::GetFood()), Bank::ALL))
                                    return false;
                            } else if (!Bank::DepositAll())
                                return false;
                        }

                        Script::SetStatus("Withdrawing main set");
                        if (!GearSets::Sets["Main"].Withdraw())
                            return false;
                        else
                        {
                            Changed = true;
                            Wait(Antiban::GenerateDelayFromPassivity(850, 1500, 1.2, 0.30));
                        }
                    }

                    Snapshot = Supplies::GetSnapshot(true);

                    if (Snapshot.MainSet_Holding && !Snapshot.MainSet_Equipped)
                    {
                        Script::SetStatus("Equipping main set");
                        if (!GearSets::Sets["Main"].Equip())
                            return false;
                        else
                            Changed = true;
                    }

                    Snapshot = Supplies::GetSnapshot(true);
                }

                if (!Snapshot.SpecialSet_Holding)
                {
                    if (!Banking::Open())
                        return false;

                    if (Inventory::CountEmpty() < Config::SpecialSetItems)
                    {
                        if (Inventory::Count(Food::GetItemID(Config::GetFood())) >= Config::SpecialSetItems)
                        {
                            if (!Bank::Deposit(Food::GetItemID(Config::GetFood()), Bank::ALL))
                                return false;
                        } else if (!Bank::DepositAll())
                            return false;
                    }

                    Script::SetStatus("Withdrawing special set");
                    if (!GearSets::Sets["Special"].Withdraw())
                        return false;
                    else
                        Changed = true;
                }
                return true;
            }
            default: break;
        }*/
        return false;
    }

    bool Deposit(const Supplies::Group& Item, bool& Changed, Supplies::Snapshot& Snapshot)
    {
        return false;
    }

    bool Supply(const Supplies::Group& Item, bool& Changed, Supplies::Snapshot& Snapshot)
    {
        return false;
    }

    bool SetupRunePouch(bool& Changed)
    {
        return false;
    }

    std::uint32_t GetRemainingHP(bool CheckGround)
    {
        return 0;
    }

    std::uint32_t GetRemainingHP(bool CheckGround, const Supplies::Snapshot& Snapshot)
    {
        return 0;
    }

    std::uint64_t GetRemainingPotionTime(const Supplies::Group& Item, bool PotionOnly)
    {
        return 0;
    }

    std::uint64_t GetRemainingPotionTime(const Supplies::Group& Item, const Supplies::Snapshot& Snapshot, bool PotionOnly)
    {
        return 0;
    }
}
