#include <Utilities/GearSets.hpp>
#include <Game/Core.hpp>
#include <Utilities/Inventory.hpp>
#include <Tools/Prices.hpp>
#include <Utilities/Bank.hpp>
#include <Utilities/Runes.hpp>
#include "Supplies.hpp"
#include "../Config.hpp"

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

Supplies::SUPPLY_POTIONS_INFO CountPotions(const std::string& PotionName, const std::vector<CONTAINER_ITEM>& ContainerItems, bool Inventory)
{
    const auto DegradableNames = Globals::GetDegradableArray(PotionName, 1, 4);
    std::vector<std::int32_t> Count = (Inventory) ? (Inventory::CountIndividual(DegradableNames, ContainerItems)) : (Bank::CountIndividual(DegradableNames, ContainerItems));

    std::array<std::uint32_t, 5> Result{};
    for (int I = 1; I < Result.size(); ++I)
    {
        Result[I] = Count[I - 1];
        Result[0] += (Result[I] * I);
    }
    return Supplies::SUPPLY_POTIONS_INFO { (std::int32_t) Result[0], Result[1], Result[2], Result[3], Result[4] };
}

std::vector<Supplies::NON_SUPPLY_ITEM> GetNonSupplyItems(const std::vector<CONTAINER_ITEM>& InventoryItems)
{
    static std::map<std::int32_t, bool> StackableMap; // Workaround for Internal::ItemInfo issues

    std::vector<Supplies::NON_SUPPLY_ITEM> Result;
    for (const auto& Item : InventoryItems)
    {
        if (Item.ID < 0) continue;

        if (Supplies::InWhitelist(Item.Name) || Supplies::InWhitelist(Item.ID)) continue;

        const auto PriceInfo = Prices::Get(Item.Name, true, false);

        Supplies::NON_SUPPLY_ITEM NonSupplyItem;
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

Supplies::SUPPLY_ITEMS_SNAPSHOT Supplies::GetSnapshot(bool InventoryOnly)
{
    static const auto UseRunePouch = Config::Get("UseRunePouch").as_bool();
    static const auto UsePrayerPots = Config::Get("UsePrayerPots").as_bool();
    static const auto UseDivinePots = Config::Get("UseDivinePots").as_bool();
    static const auto UseHighAlchemy = Config::Get("UseHighAlchemy").as_bool();
    static const auto Food = (Food::FOOD) Config::Get("Food").as_integer<int>();
    static const auto FoodID = Food::GetItemID(Food);
    static const auto SharkID = Food::GetItemID(Food::SHARK);

    Supplies::SUPPLY_ITEMS_SNAPSHOT Result = { };

    if (!Mainscreen::IsLoggedIn()) return Result;

    bool BankOpen = !InventoryOnly && Bank::IsOpen();
    const auto BankContainerItems = InventoryOnly ? std::vector<CONTAINER_ITEM>() : Bank::GetContainerItems(false);
    const auto InventoryContainerItems = Inventory::GetContainerItems(true); // Included empty for GetNonWhitelistedItems

    if (GearSets::Sets.count("Melee"))
    {
        Result.MeleeSet_Holding = GearSets::Sets["Melee"].Holding();
        Result.MeleeSet_Holding = GearSets::Sets["Melee"].Equipped();
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

    Result.HasRunePouch_Inv     = Inventory::Count(Globals::ITEM_RUNE_POUCH, InventoryContainerItems) > 0;
    Result.HasRoyalSeedPod_Inv  = Inventory::Count(Globals::ITEM_ROYAL_SEED_POD, InventoryContainerItems) > 0;

    Result.Potions_Inv_Restore  = CountPotions(Globals::ITEM_SUPER_RESTORE, InventoryContainerItems, true);
    Result.Potions_Inv_Prayer   = CountPotions(Globals::ITEM_PRAYER_POTION, InventoryContainerItems, true);
    Result.Potions_Inv_PrayerRestore =
            { Result.Potions_Inv_Restore.Total + Result.Potions_Inv_Prayer.Total,
              Result.Potions_Inv_Restore.Dose_1 + Result.Potions_Inv_Prayer.Dose_1,
              Result.Potions_Inv_Restore.Dose_2 + Result.Potions_Inv_Prayer.Dose_2,
              Result.Potions_Inv_Restore.Dose_3 + Result.Potions_Inv_Prayer.Dose_3,
              Result.Potions_Inv_Restore.Dose_4 + Result.Potions_Inv_Prayer.Dose_4, };

    Result.Potions_Inv_Ranging      = CountPotions((UseDivinePots) ? Globals::ITEM_DIVINE_RANGING_POTION : Globals::ITEM_RANGING_POTION, InventoryContainerItems, true);
    Result.Potions_Inv_SuperCombat  = CountPotions((UseDivinePots) ? Globals::ITEM_DIVINE_SUPER_COMBAT_POTION : Globals::ITEM_SUPER_COMBAT_POTION, InventoryContainerItems, true);
    Result.Food_Inv     = Inventory::Count(FoodID, InventoryContainerItems);
    Result.Sharks_Inv   = Inventory::Count(SharkID, InventoryContainerItems);
    Result.HighAlchemyCasts_Inv = CountAlchemyCasts();
    Result.EmptySlots_Inv = Inventory::CountEmpty();
    //Result.NonSupplyItems_Inv = GetNonSupplyItems(InventoryContainerItems); TODO

    if (BankOpen)
    {
        Result.HasRunePouch_Bank = Bank::Count(Globals::ITEM_RUNE_POUCH, BankContainerItems) > 0;
        Result.HasRoyalSeedPod_Bank = Bank::Count(Globals::ITEM_ROYAL_SEED_POD, InventoryContainerItems) > 0;

        Result.Potions_Bank_PrayerRestore   = CountPotions((UsePrayerPots) ? Globals::ITEM_PRAYER_POTION : Globals::ITEM_SUPER_RESTORE, BankContainerItems, false);
        Result.Potions_Bank_Ranging         = CountPotions((UseDivinePots) ? Globals::ITEM_DIVINE_RANGING_POTION : Globals::ITEM_RANGING_POTION, BankContainerItems, false);
        Result.Potions_Bank_SuperCombat     = CountPotions((UseDivinePots) ? Globals::ITEM_DIVINE_SUPER_COMBAT_POTION : Globals::ITEM_SUPER_COMBAT_POTION, BankContainerItems, false);
        Result.Food_Inv = Bank::Count(FoodID, BankContainerItems);
    }

    if (UseRunePouch)
    {
        Result.HasCorrectRunePouchRunes = Runes::Contains(Runes::LAW, Runes::POUCH);
        if (UseHighAlchemy) Result.HasCorrectRunePouchRunes = Runes::Contains({ Runes::LAVA, Runes::NATURE, Runes::LAW }, Runes::POUCH);
    } else
        Result.HasCorrectRunePouchRunes = true;

    return Result;
}

void Supplies::SetWhitelist()
{
    const auto StatRefreshMethod = Config::Get("StatRefreshMethod").as_integer<int>();
    const auto UseDivinePots = Config::Get("UseDivinePots").as_bool();
    const auto UseRunePouch = Config::Get("UseRunePouch").as_bool();
    const auto UseHighAlchemy = Config::Get("UseHighAlchemy").as_bool();
    const auto FoodCfg = (Food::FOOD) Config::Get("Food").as_integer<int>();

    Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.clear();
    Supplies::SUPPLY_ITEMS_WHITELIST_ITEM_IDS.clear();

    for (auto& D : Globals::GetDegradableArray(Globals::ITEM_PRAYER_POTION, 1, 4))
        Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace_back(std::move(D));

    for (auto& D : Globals::GetDegradableArray(Globals::ITEM_SUPER_RESTORE, 1, 4))
        Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace_back(std::move(D));

    for (auto& D : Globals::GetDegradableArray((UseDivinePots) ? Globals::ITEM_DIVINE_RANGING_POTION : Globals::ITEM_RANGING_POTION, 1, 4))
        Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace_back(std::move(D));

    for (auto& D : Globals::GetDegradableArray((UseDivinePots) ? Globals::ITEM_DIVINE_SUPER_COMBAT_POTION : Globals::ITEM_SUPER_COMBAT_POTION, 1, 4))
        Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace_back(std::move(D));

    if (GearSets::Sets.count("Melee"))
    {
        for (std::uint32_t I = Equipment::HEAD; I <= Equipment::AMMO; I++)
        {
            if (GearSets::Sets["Melee"].Items[I].Name != "NULL")
            {
                if (GearSets::Sets["Melee"].Items[I].Degradable && !GearSets::Sets["Melee"].Items[I].DegradableNames.empty())
                    for (const auto& D : GearSets::Sets["Melee"].Items[I].DegradableNames)
                        Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace_back(D);
                else
                    Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace_back(GearSets::Sets["Melee"].Items[I].Name);
            }

        }
    }

    if (GearSets::Sets.count("Ranged"))
    {
        for (std::uint32_t I = Equipment::HEAD; I <= Equipment::AMMO; I++)
        {
            if (GearSets::Sets["Ranged"].Items[I].Name != "NULL")
            {
                if (GearSets::Sets["Ranged"].Items[I].Degradable && !GearSets::Sets["Ranged"].Items[I].DegradableNames.empty())
                    for (const auto& D : GearSets::Sets["Ranged"].Items[I].DegradableNames)
                        Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace_back(D);
                else
                    Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace_back(GearSets::Sets["Ranged"].Items[I].Name);
            }
        }
    }

    if (GearSets::Sets.count("Special") && GearSets::Sets["Special"].Items[Equipment::WEAPON].Name != "NULL")
        Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace_back(GearSets::Sets["Special"].Items[Equipment::WEAPON].Name);

    switch (StatRefreshMethod)
    {
        case Config::CLAN_WARS:
        {
            for (auto& D : Globals::GetDegradableArray(Globals::ITEM_RING_OF_DUELING, 1, 8))
                Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace_back(std::move(D));
        } break;

        case Config::POH:
        {
            // TODO
        } break;
        default: break;
    }

    if (UseRunePouch) Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace_back(Globals::ITEM_RUNE_POUCH);

    Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace_back(Food::GetName(FoodCfg));
    Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace_back(Food::GetName(Food::SHARK));
    Supplies::SUPPLY_ITEMS_WHITELIST_NAMES.emplace_back(Globals::ITEM_ROYAL_SEED_POD);
}

Supplies::SUPPLY_STATE Supplies::GetInventoryState(const Supplies::SUPPLY_ITEM& Item)
{
    return Supplies::GetInventoryState(Item, Supplies::GetSnapshot(true));
}

Supplies::SUPPLY_STATE Supplies::GetInventoryState(const Supplies::SUPPLY_ITEM& Item, const Supplies::SUPPLY_ITEMS_SNAPSHOT& Snapshot)
{
    switch (Item)
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
            if (UseHighAlchemy) return Snapshot.HighAlchemyCasts_Inv > 100 ? READY_OR_FULL : NOT_READY;
            return READY_OR_ADEQUATE;
        }

        case FOOD:
        {

        }

        default: break;
    }
    return Supplies::UNUSED;
}

bool Supplies::Withdraw(const Supplies::SUPPLY_ITEM& Item, bool& Changed, Supplies::SUPPLY_ITEMS_SNAPSHOT& Snapshot)
{
    return false;
}

bool Supplies::Deposit(const Supplies::SUPPLY_ITEM& Item, bool& Changed, Supplies::SUPPLY_ITEMS_SNAPSHOT& Snapshot)
{
    return false;
}

bool Supplies::Supply(const Supplies::SUPPLY_ITEM& Item, bool& Changed, Supplies::SUPPLY_ITEMS_SNAPSHOT& Snapshot)
{
    return false;
}

bool Supplies::SetupRunePouch(bool& Changed)
{
    return false;
}

std::uint32_t Supplies::GetRemainingHP(bool CheckGround)
{
    return 0;
}

std::uint32_t Supplies::GetRemainingHP(bool CheckGround, const Supplies::SUPPLY_ITEMS_SNAPSHOT& Snapshot)
{
    return 0;
}

std::uint64_t Supplies::GetRemainingPotionTime(const Supplies::SUPPLY_ITEM& Item, bool PotionOnly)
{
    return 0;
}

std::uint64_t Supplies::GetRemainingPotionTime(const Supplies::SUPPLY_ITEM& Item, const Supplies::SUPPLY_ITEMS_SNAPSHOT& Snapshot, bool PotionOnly)
{
    return 0;
}
