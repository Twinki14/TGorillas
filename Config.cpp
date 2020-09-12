#include <Game/Core.hpp>
#include <TScript.hpp>
#include <Tools/TGotify.hpp>
#include <Utilities/TProfile.hpp>
#include <Utilities/Antiban.hpp>
#include "Config.hpp"

#define CONFIGURU_IMPLEMENTATION 1
#include <Tools/configuru.hpp>
#include <Utilities/GearSets.hpp>
#include <Tools/OSRSBox/Items.hpp>

void Config::RequestArgs()
{
    for (const auto& O : Config::Cfg.as_object())
        RequestArgument(O.key(), "");
}

void Config::LoadArgs(const std::string& FileName)
{
    if (!FileName.empty())
    {
        std::string Extension = (FileName.length() > 4) ? FileName.substr(FileName.length() - 5, 5) : "";
        if (Extension != ".json")
            LoadArguments(FileName + ".json");
        else
            LoadArguments(FileName);
    }

    for (const auto& O : Config::Cfg.as_object())
    {
        const auto& Key = O.key();
        const auto Arg = GetArgument(Key);
        if (!Arg.empty())
        {
            try
            {
                switch (Config::Cfg[Key].type())
                {
                    case configuru::Config::Bool: Config::Cfg[Key] = (bool) std::stoi(Arg); break;
                    case configuru::Config::Int: Config::Cfg[Key] = (int) std::stoi(Arg); break;
                    case configuru::Config::Float: Config::Cfg[Key] = (float) std::stof(Arg); break;
                    case configuru::Config::String: Config::Cfg[Key] = Arg; break;
                    case configuru::Config::Array:
                    {
                        configuru::FormatOptions Options = configuru::make_json_options();
                        Options.end_with_newline = false;
                        Options.indentation = "";
                        auto Conf = configuru::parse_string(Arg.c_str(), Options, Key.c_str());
                        Config::Cfg[Key] = Conf.as_array();
                    }
                    default: break;
                }
            } catch (const std::exception& E)
            {
                //DebugLog << "Caught " << E.what() << std::endl;
            }
        }
    }

    auto CheckIntConfig = [](const std::string& Key)
    {
        if (Config::Cfg.count(Key) && Config::Cfg[Key].is_int())
        {
            if (Config::Cfg[Key].as_integer<int>() < 0) Config::Cfg[Key] = 0;
            if (Config::Cfg[Key].as_integer<int>() > 4) Config::Cfg[Key] = 4;
        }
    };

    CheckIntConfig("Passivity");
    CheckIntConfig("AFK_Frequency");
    CheckIntConfig("TabOut_Tendency");
    CheckIntConfig("Camera_Tendency");
}

void Config::SaveArgs(const std::string& FileName)
{
    if (FileName.empty())
        return;

    auto CheckIntConfig = [](const std::string& Key)
    {
        if (Config::Cfg.count(Key) && Config::Cfg[Key].is_int())
        {
            if (Config::Cfg[Key].as_integer<int>() < 0) Config::Cfg[Key] = 0;
            if (Config::Cfg[Key].as_integer<int>() > 4) Config::Cfg[Key] = 4;
        }
    };

    CheckIntConfig("Passivity");
    CheckIntConfig("AFK_Frequency");
    CheckIntConfig("TabOut_Tendency");
    CheckIntConfig("Camera_Tendency");

    for (const auto& O : Config::Cfg.as_object())
    {
        std::string Value;
        switch (Config::Cfg[O.key()].type())
        {
            case configuru::Config::Bool:   Value = std::to_string(O.value().as_bool()); break;
            case configuru::Config::Int:    Value = std::to_string((int) O.value()); break;
            case configuru::Config::Float:  Value = std::to_string(O.value().as_float()); break;
            case configuru::Config::String: Value = O.value().as_string(); break;
            case configuru::Config::Array:
            {
                configuru::FormatOptions Options = configuru::make_json_options();
                Options.end_with_newline = false;
                Options.indentation = "";
                Value = configuru::dump_string(O.value().as_array(), Options);
                break;
            }
            default: break;
        }
        SetArgument(O.key(), Value);
    }

    SetArgument("Config_FromFile", "1");
    SaveArguments(FileName);
}

configuru::Config& Config::Get(const std::string& Key)
{
    const std::lock_guard<std::mutex> Lock(Config::CfgAccessLock);
    return Config::Cfg[Key];
}

void Config::Set(const std::string& Key, const configuru::Config& Conf)
{
    const std::lock_guard<std::mutex> Lock(Config::CfgAccessLock);
    Config::Cfg[Key] = Conf;
}

void Config::SetGearsets()
{
    GearSets::Set Melee;
    GearSets::Set Ranged;
    GearSets::Set Special;

    bool CfgHasMelee = Config::Cfg.count("GearSet_Melee_Names")
                       && Config::Cfg.count("GearSet_Melee_IDs")
                       && Config::Cfg["GearSet_Melee_Names"].array_size() == 11
                       && Config::Cfg["GearSet_Melee_IDs"].array_size() == 11;

    bool CfgHasRanged= Config::Cfg.count("GearSet_Ranged_Names")
                       && Config::Cfg.count("GearSet_Ranged_IDs")
                       && Config::Cfg["GearSet_Ranged_Names"].array_size() == 11
                       && Config::Cfg["GearSet_Ranged_IDs"].array_size() == 11;

    for (std::uint32_t I = Equipment::HEAD; I <= Equipment::AMMO; I++)
    {
        if (CfgHasMelee) Melee.Items[I] = GearSets::Item(Config::Cfg["GearSet_Melee_Names"][I].as_string(), Config::Cfg["GearSet_Melee_IDs"][I].as_integer<int>());
        if (CfgHasRanged) Ranged.Items[I] = GearSets::Item(Config::Cfg["GearSet_Ranged_Names"][I].as_string(), Config::Cfg["GearSet_Ranged_IDs"][I].as_integer<int>());
    }

    if (Config::Cfg.count("SpecialWeapon"))
    {
        if (Ranged.Items[Equipment::WEAPON].GetName() == "Magic shortbow (i)") Config::Cfg["SpecialWeapon"] = MAGIC_SHORTBOW;
        if (Ranged.Items[Equipment::WEAPON].GetName() == "Toxic blowpipe") Config::Cfg["SpecialWeapon"] = TOXIC_BLOWPIPE;

        switch (Config::Cfg["SpecialWeapon"].as_integer<int32_t>())
        {
            case MAGIC_SHORTBOW:
            {
                Special = Ranged;
                Special.Items[Equipment::WEAPON] = GearSets::Item("Magic shortbow (i)", 12788);
                break;
            }

            case TOXIC_BLOWPIPE:
            {
                Special = Ranged;
                Special.Items[Equipment::WEAPON] = GearSets::Item("Toxic blowpipe", 12926);
                break;
            }

            case SARADOMIN_GODSWORD:
            {
                Special = Melee;
                Special.Items[Equipment::WEAPON] = GearSets::Item("Saradomin godsword", 11806);
                break;
            }
            default: break;
        }
    }

    GearSets::Sets["Melee"] = std::move(Melee);
    GearSets::Sets["Ranged"] = std::move(Ranged);
    GearSets::Sets["Special"] = std::move(Special);
}

void Config::SetAntiban()
{
    constexpr auto ONE_MINUTE = 60000;

    auto CheckIntConfig = [](const std::string& Key)
    {
        if (Config::Cfg.count(Key) && Config::Cfg[Key].is_int())
        {
            if (Config::Cfg[Key].as_integer<int>() < 0) Config::Cfg[Key] = 0;
            if (Config::Cfg[Key].as_integer<int>() > 4) Config::Cfg[Key] = 4;
        }
    };

    CheckIntConfig("Passivity");
    CheckIntConfig("AFK_Frequency");
    CheckIntConfig("TabOut_Tendency");
    CheckIntConfig("Camera_Tendency");

    Profile::Set(Profile::Var_Passivity, Config::Get("Passivity").as_integer<int>());
    Profile::Set(Profile::Var_AFK_Frequency, Config::Get("AFK_Frequency").as_integer<int>());
    Profile::Set(Profile::Var_TabOut_Tendency, Config::Get("TabOut_Tendency").as_integer<int>());
    Profile::Set(Profile::Var_Camera_Tendency, Config::Get("Camera_Tendency").as_integer<int>());

    Profile::Set(Profile::Var_UseHotkeys_EscCloseInterface, Config::Get("UseHotkeys_Esc").as_bool());
    Profile::Set(Profile::Var_UseHotkeys_Gametabs, Config::Get("UseHotkeys_Gametabs").as_bool());
    Profile::Set(Profile::Var_UseHotkeys_EscCloseInterface_Chance, 0.95);
    Profile::Set(Profile::Var_UseHotkeys_Gametabs_Chance, 0.96);
}

void Config::CacheOSRSBoxItems()
{
/*    if (GearSets::Sets.count("Melee")) OSRSBox::Items::GetGearset(GearSets::Sets["Melee"]);
    if (GearSets::Sets.count("Ranged")) OSRSBox::Items::GetGearset(GearSets::Sets["Ranged"]);
    if (GearSets::Sets.count("Special")) OSRSBox::Items::GetGearset(GearSets::Sets["Special"]);*/
}

bool Config::AuthenticateScript()
{
    static Countdown C = Countdown(2 * (60 * 60000));
    if (C.IsFinished())
    {
        C.Reset();
        Script::SetStatus("Authenticating script");
        const auto AuthPlugin = TGotify::Plugin(Config::GotifyHost, Config::GotifyAuthPluginToken, Config::GotifyAuthPluginID);
        const auto Auth = TGotify::Authenticator(AuthPlugin, Config::GotifyAuthHeaderToken, Script::GetName(), Config::Get("ScriptAuthToken").as_string());
        WinnHttpComm::Response Resp;
        if (Auth.Authenticate(&Resp))
        {
            DebugLog("Authenticated");
            return true;
        }

        std::string FailedReason;
        switch (Resp.Status)
        {
            case 0: FailedReason = "- Connection failed "; break;
            case 400: FailedReason = "- Incorrect token "; break;
            case 403: FailedReason = "- Forbidden - Header token invalid "; break;
            case 404: FailedReason = "- Page not found "; break;
            default: break;
        }
        Script::Stop("Authentication failed " + FailedReason + "(" +std::to_string(Resp.Status) + ")");
        return false;
    }
    return true;
}
