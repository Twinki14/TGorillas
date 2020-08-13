#include <Game/Core.hpp>
#include <TScript.hpp>
#include <Tools/OSRSBox/Items.hpp>
#include <Utilities/BackgroundTask/CameraTask.hpp>
#include <Utilities/BackgroundTask/AuthenticateTask.hpp>
#include <Utilities/Mainscreen.hpp>
#include "Config.hpp"
#include "GUI/GUI.hpp"
#include "Logic/Gorillas.hpp"
#include "Logic/Listeners/GameListener.hpp"
#include "Logic/Types/WorldArea.hpp"

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
    GameListener::DrawGorillas();
    GameListener::DrawPlayers();
    GameListener::DrawProjectiles();
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
    }

    SetLoopDelay(0);

    //if (GUI::Init())
    {
        Config::SaveArgs("default.json");
        Config::SetAntiban();
        Script::Start(PaintMethod, true);
        return true;
    }
    Script::Shutdown();
    return false;
}

bool Loop()
{
    GameListener::Instance().Start();
    Wait(1000);
    return true;

    if (Mainscreen::IsLoggedIn())
    {
        if (BreakHandler::Break())
        {
            Script::ResumeTimer();
            return true;
        }

        return true;
    } else
        return Login::LoginPlayer();
}

bool OnBreak()
{
    Script::PauseTimer();
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
