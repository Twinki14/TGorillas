#ifndef SUPPLIES_HPP_INCLUDED
#define SUPPLIES_HPP_INCLUDED

#include <cstdint>
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
    typedef struct SUPPLY_POTIONS_INFO
    {
        std::int32_t Total; // -1 if unlimited
        std::uint32_t Dose_1;
        std::uint32_t Dose_2;
        std::uint32_t Dose_3;
        std::uint32_t Dose_4;
    } SUPPLY_POTIONS_INFO;

    typedef struct NON_SUPPLY_ITEM
    {
        Interactable::Item Item;
        std::int32_t ID;
        std::string Name;
        std::int32_t StackAmount;
        std::uint32_t ExchangeValue;
        std::uint32_t TotalExchangeValue;
        bool Stackable;
        std::uint32_t HighAlchValue;
        std::int32_t HighAlchProfit;
        bool ShouldAlch;
    } NON_SUPPLY_ITEM;

    typedef struct SUPPLY_ITEMS_SNAPSHOT
    {
        bool MeleeSet_Holding = false;
        bool MeleeSet_Equipped = false;
        bool RangedSet_Holding = false;
        bool RangedSet_Equipped = false;
        bool SpecialSet_Holding = false;
        bool SpecialSet_Equipped = false;

        bool HasRunePouch_Inv = false;
        bool HasRoyalSeedPod_Inv = false;

        SUPPLY_POTIONS_INFO Potions_Inv_Restore;
        SUPPLY_POTIONS_INFO Potions_Inv_Prayer;
        SUPPLY_POTIONS_INFO Potions_Inv_PrayerRestore;
        SUPPLY_POTIONS_INFO Potions_Inv_Ranging;
        SUPPLY_POTIONS_INFO Potions_Inv_SuperCombat;
        std::int32_t Food_Inv;
        std::int32_t Sharks_Inv;
        std::int32_t HighAlchemyCasts_Inv;
        std::int32_t EmptySlots_Inv;
        std::vector<NON_SUPPLY_ITEM> NonSupplyItems_Inv;

        bool HasRunePouch_Bank = false;
        bool HasRoyalSeedPod_Bank = false;

        SUPPLY_POTIONS_INFO Potions_Bank_PrayerRestore;
        SUPPLY_POTIONS_INFO Potions_Bank_Ranging;
        SUPPLY_POTIONS_INFO Potions_Bank_SuperCombat;
        std::int32_t Food_Bank;

        bool HasCorrectRunePouchRunes = false;
    } SUPPLY_ITEMS_SNAPSHOT;
    SUPPLY_ITEMS_SNAPSHOT GetSnapshot(bool InventoryOnly = false);

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
    void SetWhitelist();

    // TELEPORTS for missing teleport items, like frem boots, max/construction capes, law runes
    // RUNE_POUCH will set the pouch up to what's needed, and withdraw any needed runes like nature runes
    typedef enum SUPPLY_ITEM
    {
        GEAR,
        PRAYER_RESTORE,
        RANGING_POTION,
        SUPER_COMBAT,
        TELEPORTS, // takes care of any teleport items, and determines if we have enough teleports
        RUNES, // Takes care of rune setup in the pouch, and any runes needed for high alch
        FOOD
    } SUPPLY_ITEM;

    typedef enum SUPPLY_STATE
    {
        UNUSED = -1, // the supply_item isn't used, ignored in supply handles
        NOT_READY, // not enough of the item to finish a full trip
        READY_OR_ADEQUATE, // enough for a full trip, but it's not fully ready
        READY_OR_FULL, // ready/full on supplies
        //EXCESSIVE // too much of the item - moved to NOT_READY
    } SUPPLY_STATE;

    static SUPPLY_STATE GetInventoryState(const SUPPLY_ITEM& Item);
    static SUPPLY_STATE GetInventoryState(const SUPPLY_ITEM& Item, const Supplies::SUPPLY_ITEMS_SNAPSHOT& Snapshot);

    static bool Withdraw(const SUPPLY_ITEM& Item, bool& Changed, Supplies::SUPPLY_ITEMS_SNAPSHOT& Snapshot); // Take out only whats needed
    static bool Deposit(const SUPPLY_ITEM& Item, bool& Changed, Supplies::SUPPLY_ITEMS_SNAPSHOT& Snapshot); // Deposit excess + used
    static bool Supply(const SUPPLY_ITEM& Item, bool& Changed, Supplies::SUPPLY_ITEMS_SNAPSHOT& Snapshot);

    static bool SetupRunePouch(bool& Changed);

    static std::uint32_t GetRemainingHP(bool CheckGround = false);
    static std::uint32_t GetRemainingHP(bool CheckGround, const Supplies::SUPPLY_ITEMS_SNAPSHOT& Snapshot);
    static std::uint64_t GetRemainingPotionTime(const SUPPLY_ITEM& Item, bool PotionOnly = false); // Only in inventory
    static std::uint64_t GetRemainingPotionTime(const SUPPLY_ITEM& Item, const Supplies::SUPPLY_ITEMS_SNAPSHOT& Snapshot, bool PotionOnly = false);
}
#endif // SUPPLIES_HPP_INCLUDED