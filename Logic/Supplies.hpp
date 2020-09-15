#ifndef SUPPLIES_HPP_INCLUDED
#define SUPPLIES_HPP_INCLUDED

#include <cstdint>
#include <Utilities/Inventory.hpp>
#include <Utilities/Containers.hpp>
#include <Tools/OSRSBox/Items.hpp>
#include <Game/Interactable/Item.hpp>

namespace Globals
{
    inline std::vector<std::string> GetDegradableArray(const std::string& Name, std::uint32_t From, std::uint32_t To)
    {
        std::vector<std::string> Result;
        for (std::uint32_t I = From; I <= To; I++)
            Result.emplace_back(Name + std::to_string(I) + ")");
        return Result;
    }

    // Items
    const std::string ITEM_RING_OF_DUELING = "Ring of dueling(";
    const std::string ITEM_RUNE_POUCH = "Rune pouch";
    const std::string ITEM_ROYAL_SEED_POD = "Royal seed pod";
    const std::string ITEM_SUPER_RESTORE = "Super restore(";
    const std::string ITEM_PRAYER_POTION = "Prayer potion(";
    const std::string ITEM_RANGING_POTION = "Ranging potion(";
    const std::string ITEM_SUPER_COMBAT_POTION = "Super combat potion(";
    const std::string ITEM_DIVINE_RANGING_POTION = "Divine ranging potion(";
    const std::string ITEM_DIVINE_SUPER_COMBAT_POTION = "Divine super combat potion(";
    const std::string ITEM_LAW_RUNE = "Law rune";
    const std::string ITEM_LAVA_RUNE = "Lava rune";
    const std::string ITEM_NATURE_RUNE = "Nature rune";
    const std::int32_t ITEM_LAVA_RUNE_ID = 4699;
}

namespace Supplies
{
    enum InventoryLayout
    {
        LAYOUT_1, // https://i.imgur.com/mYCVPH7.png
        LAYOUT_2, // https://i.imgur.com/AnXrwyn.png
        LAYOUT_3 // https://i.imgur.com/cWfbpJL.png
    };

    typedef struct Potion
    {
        std::int32_t Total; // -1 if unlimited
        std::uint32_t Dose_1;
        std::uint32_t Dose_2;
        std::uint32_t Dose_3;
        std::uint32_t Dose_4;
    } Potion;

    typedef struct ItemRecord
    {
        std::int32_t ID;
        std::string Name;
        Interactable::Item Item;
        std::int32_t StackAmount;
        std::uint32_t ExchangeValue;
        std::uint32_t TotalExchangeValue;
        bool Stackable;
        std::uint32_t HighAlchValue;
        std::int32_t HighAlchProfit;
        bool ShouldAlch;

        OSRSBox::Items::Item Info;
    } ItemRecord;

    typedef struct Snapshot
    {
        Containers::Container BankContainer;
        Containers::Container InventoryContainer;

        bool MeleeSet_Holding = false;
        bool MeleeSet_Equipped = false;
        bool RangedSet_Holding = false;
        bool RangedSet_Equipped = false;
        bool SpecialSet_Holding = false;
        bool SpecialSet_Equipped = false;

        bool HasRunePouch_Inv = false;
        bool HasRoyalSeedPod_Inv = false;

        Potion Potions_Inv_Restore;
        Potion Potions_Inv_Prayer;
        Potion Potions_Inv_PrayerRestore;
        Potion Potions_Inv_Ranging;
        Potion Potions_Inv_SuperCombat;
        std::int32_t Food_Inv;
        std::int32_t Sharks_Inv;
        std::int32_t HighAlchemyCasts_Inv;
        std::int32_t EmptySlots_Inv;
        std::vector<ItemRecord> NonWhitelistedItems_Inv;

        bool HasRunePouch_Bank = false;
        bool HasRoyalSeedPod_Bank = false;

        Potion Potions_Bank_PrayerRestore;
        Potion Potions_Bank_Ranging;
        Potion Potions_Bank_SuperCombat;
        std::int32_t Food_Bank;

        bool HasCorrectRunePouchRunes = false;
    } Snapshot;

    // TELEPORTS for missing teleport items, like frem boots, max/construction capes, law runes
    // RUNE_POUCH will set the pouch up to what's needed, and withdraw any needed runes like nature runes
    typedef enum Group
    {
        GEAR,
        PRAYER_RESTORE,
        RANGING_POTION,
        SUPER_COMBAT,
        TELEPORTS, // takes care of any teleport items, and determines if we have enough teleports
        RUNES, // Takes care of rune setup in the pouch, and any runes needed for high alch
        FOOD
    } Group;

    typedef enum GroupState
    {
        UNUSED = -1, // the supply_item isn't used, ignored in supply handles
        NOT_READY, // not enough of the item to finish a full trip
        READY_OR_ADEQUATE, // enough for a full trip, but it's not fully ready
        READY_OR_FULL, // ready/full on supplies
    } GroupState;

    namespace
    {
        inline std::set<std::string> SUPPLY_ITEMS_WHITELIST_NAMES;
        inline std::set<std::int32_t> SUPPLY_ITEMS_WHITELIST_ITEM_IDS;

        bool InWhitelist(const std::string_view& Name)
        {
            return std::find(SUPPLY_ITEMS_WHITELIST_NAMES.begin(), SUPPLY_ITEMS_WHITELIST_NAMES.end(), Name)
                   != SUPPLY_ITEMS_WHITELIST_NAMES.end();
        }

        bool InWhitelist(std::int32_t ItemID)
        {
            return std::find(SUPPLY_ITEMS_WHITELIST_ITEM_IDS.begin(), SUPPLY_ITEMS_WHITELIST_ITEM_IDS.end(), ItemID)
                   != SUPPLY_ITEMS_WHITELIST_ITEM_IDS.end();
        }
    }

    std::uint32_t GetInventoryLayout();
    Snapshot GetSnapshot(bool InventoryOnly = false);

    GroupState GetState(const Group& G);
    GroupState GetState(const Group& G, const Supplies::Snapshot& Snapshot);

    // TODO - Maybe remove changed and handle it automatically - with containers I can just wait until it changes, checking every tick via GameListener
    bool Withdraw(const Group& G, bool& Changed, Supplies::Snapshot& Snapshot); // Take out only whats needed
    bool Deposit(const Group& G, bool& Changed, Supplies::Snapshot& Snapshot); // Deposit excess + used
    bool Supply(const Group& G, bool& Changed, Supplies::Snapshot& Snapshot);

    bool SetupRunePouch(bool& Changed);
    bool Recharge(Supplies::Snapshot& Snapshot);
    bool OrganizeInventory(const Group& G, Supplies::Snapshot& Snapshot);

    std::uint32_t GetRemainingHP(bool CheckGround = false);
    std::uint32_t GetRemainingHP(bool CheckGround, const Supplies::Snapshot& Snapshot);
    std::uint64_t GetRemainingPotionTime(const Group& G, bool PotionOnly = false); // Only in inventory
    std::uint64_t GetRemainingPotionTime(const Group& G, const Supplies::Snapshot& Snapshot, bool PotionOnly = false);
}
#endif // SUPPLIES_HPP_INCLUDED