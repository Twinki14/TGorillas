#ifndef CONFIG_HPP_INCLUDED
#define CONFIG_HPP_INCLUDED

#include <string>
#include <Tools/configuru.hpp>
#include <Utilities/TProfile.hpp>
#include <mutex>
#include <Utilities/Food.hpp>

class Config
{
public:

    enum SPECIAL_WEAPON
    {
        INVALID = -1,
        MAGIC_SHORTBOW,
        TOXIC_BLOWPIPE,
        SARADOMIN_GODSWORD
    };

    enum STAT_REFRESH_METHOD { CLAN_WARS, POH };

    static void RequestArgs(); // NOT THREAD SAFE
    static void LoadArgs(const std::string& FileName = ""); // NOT THREAD SAFE
    static void SaveArgs(const std::string& FileName); // NOT THREAD SAFE

    inline static configuru::Config Cfg = configuru::Config
    {
            { "ScriptAuthToken","" },
            { "ScriptNotificationToken","" },
            { "ScriptNotificationCountdown", 60 },  // In Minutes (ScriptNotificationCountdown * 60000)

            { "Passivity", Profile::PASSIVITY_MILD }, // Affects how quickly the 'player' will click on the next obstacle, and how long AFKs last
            { "AFK_Frequency", Profile::FREQUENCY_REGULARLY }, // Affects how often the 'player' will go AFK
            { "TabOut_Tendency", Profile::TENDENCY_NORMAL }, // Affects how often 'AFKs' include tabbing out
            { "Camera_Tendency", Profile::TENDENCY_NORMAL }, // Affects how often the 'player' will move the camera around

            { "UseHotkeys_Esc", true },
            { "UseHotkeys_Gametabs", true },

            { "GearSet_Melee_Names", configuru::Config::array(std::vector<std::string>(11, "")) },
            { "GearSet_Melee_IDs", configuru::Config::array(std::vector<std::int32_t>(11, -1)) },

            { "GearSet_Ranged_Names", configuru::Config::array(std::vector<std::string>(11, "")) },
            { "GearSet_Ranged_IDs", configuru::Config::array(std::vector<std::int32_t>(11, -1)) },

            { "UseRunePouch", false },
            { "SpecialWeapon", INVALID },
            { "StatRefreshMethod", CLAN_WARS },
            { "Food", Food::SHARK },
            { "RestorePotionAmount", 8 } ,
            { "UsePrayerPots", false },
            { "UseDivinePots", false },

            { "Recharge_Blowpipe", false },

            { "UseHighAlchemy", false },
            { "Loot_MinimumHighAlchemyProfit", 0 },
            { "Loot_Blacklist", configuru::Config::array(std::vector<std::int32_t>()) },

            { "Debug_Paint", false },
            { "Debug_Logging", true },

            { "Config_FromFile", false }, // For determining if config has been loaded from a file
    };

    static configuru::Config& Get(const std::string& Key);
    static void Set(const std::string& Key, const configuru::Config& Conf);

    static void SetGearsets();
    static void SetAntiban();
    inline static std::vector<std::string> AntibanTasks;
    inline static std::vector<std::string> AFKTasks;

    static void CacheOSRSBoxItems();

    inline static const std::string GotifyHost = "twinki.ddns.net";
    inline static const std::string GotifyAuthPluginToken = "PzPu8plcKnRd37S";
    inline static const std::string GotifyAuthHeaderToken = "PzPu8plcKnRd37S";
    inline static const std::uint32_t GotifyAuthPluginID = 1;

    static bool AuthenticateScript();

private:
    inline static std::mutex CfgAccessLock;
};

#endif // CONFIG_HPP_INCLUDED