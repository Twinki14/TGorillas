#include <Game/Core.hpp>
#include <TScript.hpp>
#include <Tools/OSRSBox/Items.hpp>
#include <Utilities/BackgroundTask/CameraTask.hpp>
#include <Utilities/BackgroundTask/AuthenticateTask.hpp>
#include <Utilities/Mainscreen.hpp>
#include <Utilities/GearSets.hpp>
#include <bitset>
#include "Config.hpp"
#include "GUI/GUI.hpp"
#include "Logic/Gorillas.hpp"
#include "Logic/Listeners/GameListener.hpp"
#include "Logic/Types/WorldArea.hpp"
#include "Logic/Travel.hpp"
#include "Logic/Supplies.hpp"

void Setup()
{
    ScriptInfo Info;
    Info.Name = "TGorillas";
    Info.Description = "";
    Info.Version = "1.00";
    Info.Category = "Other";
    Info.Author = "Twinki";
    Info.UID = "UID";
    Info.ForumPage = "forum.alpacabot.org";
    SetScriptInfo(Info);
    Config::RequestArgs();
}

void PaintMethod()
{
    Paint::Clear();
    Gorillas::Draw();
    Paint::DrawDot(GetMousePos(), 1.5f, 153, 51, 255, 255);
    Paint::SwapBuffer();
}

bool OnStart()
{
    Debug::Clear();
    Script::Setup();
    Config::LoadArgs();

    if (!Config::Get("Config_FromFile").as_bool())
    {
        DebugLog("Trying to load args from default.json");
        Config::LoadArgs("default.json");
        Config::SetGearsets();
    }

    SetLoopDelay(0);
    //return true;

    //if (GUI::Init())
    {

/*        GearSets::SetFromEquipped("Melee");
        //GearSets::SetFromEquipped("Ranged");
        for (std::uint32_t I = Equipment::HEAD; I <= Equipment::AMMO; I++)
        {
            Config::Cfg["GearSet_Melee_Names"][I] = GearSets::Sets["Melee"].Items[I].Name;
            Config::Cfg["GearSet_Melee_IDs"][I] = GearSets::Sets["Melee"].Items[I].ID;

            Config::Cfg["GearSet_Ranged_Names"][I] = GearSets::Sets["Ranged"].Items[I].Name;
            Config::Cfg["GearSet_Ranged_IDs"][I] = GearSets::Sets["Ranged"].Items[I].ID;
        }*/

        Config::SaveArgs("default.json");
        Config::SetGearsets();
        Config::SetAntiban();
        Config::CacheOSRSBoxItems();
        Supplies::SetWhitelist();
        Script::Start(PaintMethod, true);
        return true;
    }
    Script::Shutdown();
    return false;
}

bool Loop()
{
    if (Mainscreen::IsLoggedIn())
    {
        if (BreakHandler::Break())
        {
            Script::ResumeTimer();
            return true;
        }

        if (Travel::GetLocation() == Travel::CRASH_SITE_CAVERN_INNER)
            Gorillas::Fight();
        else
        {
            GameListener::Instance().Stop(true);
        }

        return true;
    } else
        return Login::LoginPlayer();
}

bool OnBreak()
{
    Script::PauseTimer();
    GameListener::Instance().Stop(true);
    Script::SetStatus("Breaking");
    return false;
}

void OnEnd()
{
    CameraTask::TerminateTask();
    AuthenticateTask::TerminateTask();
    GameListener::Instance().Stop(true);
    Script::Shutdown();
}
