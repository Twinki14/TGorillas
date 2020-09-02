#include <Game/Core.hpp>
#include <TScript.hpp>
#include <Tools/OSRSBox/Items.hpp>
#include <Utilities/BackgroundTask/CameraTask.hpp>
#include <Utilities/BackgroundTask/AuthenticateTask.hpp>
#include <Utilities/Mainscreen.hpp>
#include <Utilities/GearSets.hpp>
#include <bitset>
#include <Tools/PaintBitmap.hpp>
#include "Config.hpp"
#include "GUI/GUI.hpp"
#include "Logic/Gorillas.hpp"
#include "Logic/Listeners/GameListener.hpp"
#include "Logic/Types/WorldArea.hpp"
#include "Logic/Travel.hpp"
#include "Logic/Supplies.hpp"

// move all loot items to coins tab

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

const auto BITMAP_COINS = PaintBitmap("HgAAAB0AAABNTU0yTU1NMk1NTTJNTU0yTU1NMk1NTTJNTU0yTU1NMk1NTTJNTU0yTU1NMgAAAP8AAAD/AAAA/wAAAP8AAAD/TU1NMk1NTTJNTU0yTU1NMk1NTTJNTU0yTU1NMk1NTTJNTU0yTU1NMk1NTTJNTU0yTU1NMk1NTTJNTU0yTU1NMk1NTTJNTU0yTU1NMk1NTTJNTU0yTU1NMk1NTTJNTU0yAAAA/7ORDv+zkQ7/s5EO/7ORDv+zkQ7/AAAA/01NTTJNTU0yTU1NMk1NTTJNTU0yAAAA/wAAAP8AAAD/AAAA/wAAAP8AAAD/TU1NMk1NTTJNTU0yTU1NMk1NTTJNTU0yTU1NMk1NTTJNTU0yTU1NMk1NTTIAAAD/s5EO/7ORDv+zkQ7/s5EO/7ORDv8ZEgD/AAAA/01NTTJNTU0yTU1NMk1NTTIAAAD/s5EO/7ORDv+zkQ7/s5EO/7ORDv+zkQ7/AAAA/01NTTJNTU0yTU1NMk1NTTJNTU0yTU1NMk1NTTJNTU0yTU1NMk1NTTIAAAD/8MUX//zRM//ovxb/zacS/7CPDv8ZEgD/AAAA/01NTTJNTU0yTU1NMk1NTTIAAAD/68EW/7ORDv+zkQ7/s5EO/7ORDv8ZEgD/AAAA/01NTTJNTU0yTU1NMk1NTTJNTU0yTU1NMk1NTTJNTU0yTU1NMk1NTTIAAAD/9MgX//zQK//mvBb/yqUS/7CPDv8ZEgD/AAAA/01NTTJNTU0yTU1NMk1NTTIAAAD/7cMW//TIF//WsBT/tpQO/491Cf8ZEgD/AAAA/01NTTJNTU0yTU1NMk1NTTJNTU0yTU1NMk1NTTJNTU0yTU1NMk1NTTIAAAD/8scX//zPI//juhX/yqUS/6SFDP8ZEgD/AAAA/01NTTJNTU0yTU1NMk1NTTIAAAD/8MUX//LHF//RrBP/sI8O/4xyCP8ZEgD/AAAA/01NTTJNTU0yTU1NMk1NTTJNTU0yTU1NMk1NTTJNTU0yTU1NMgAAAP/ovxb/8MUX//zOGf/huBX/wp4Q/52ACv8AAAD/AAAA/wAAAP9NTU0yTU1NMk1NTTIAAAD/7cMW/+3DFv/PqRP/rYwN/4huCP8ZEgD/AAAA/01NTTJNTU0yTU1NMk1NTTJNTU0yTU1NMk1NTTJNTU0yTU1NMgAAAP/rwRb/8scX//nMGf/ctBT/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");

void PaintMethod()
{
    Paint::Clear();

    Paint::Pixel BaseTextColor = { 0, 214, 0, 255};  //GUI::GetTextColor();

    auto StatusBox = GameListener::GetPaintStatusBox();
    if (StatusBox.X < 0 || StatusBox.Y < 0)
    {
        const auto ChatWidget = Widgets::Get(162, 38); // 162, 5
        if (ChatWidget.IsVisible())
        {
            const auto B = ChatWidget.GetBox();
            StatusBox.X = B.X + 5;
            StatusBox.Y = B.Y - 25;
        } else
        {
            const auto AllButtonWidget = Widgets::Get(162, 5);
            const auto B = AllButtonWidget.GetBox();
            StatusBox.X = B.X + 5;
            StatusBox.Y = B.Y - 35;
        }
    }

    Paint::DrawSquare(StatusBox, 77, 77, 77, 50);
    Paint::DrawBox(StatusBox, 77, 77, 77, 255);
    Paint::DrawString("Status - " + Script::GetStatus(), Point(StatusBox.X + 5, StatusBox.Y + 5), BaseTextColor.Red, BaseTextColor.Green, BaseTextColor.Blue, BaseTextColor.Alpha);

    const auto TitleBox = Box(5, 20, 185, 25);
    const auto CoinsBox = Box(5, TitleBox.GetY2() + 5, TitleBox.Width, 35);
    const auto StatsBox = Box(5, CoinsBox.GetY2() + 5, TitleBox.Width, 69);

    const std::string TitleStr = "TGorillas | " + MillisToHumanShort(Script::GetTimeElapsed());

    const auto TotalLootProfit = 0; //Tracker::GetTotalProfit();
    const std::string CoinsStr = Script::Tools::FormatRunescapeGold(TotalLootProfit) + " [" + Script::Tools::FormatRunescapeGold(Script::Tools::ToHour(0, TotalLootProfit)) + " p/h]";
    const auto CoinsStart = Point((CoinsBox.GetX2() / 2) - ((BITMAP_COINS.GetWidth() + (CoinsStr.size() * 7) + 6) / 2), CoinsBox.Y + 5);

    Paint::Pixel CoinsColor = { 255, 255, 255, 255 };
    if (((double) TotalLootProfit / 1000000.0) >= 1.00)
        CoinsColor = { 0, 255, 0, 255 };

    const auto ShardCount = 0; // Tracker::GetGainedZenytes();
    const auto SPH = Script::Tools::ToHour(0, ShardCount);
    const auto KillCount = 0; //Vorkath::GetKillCount();
    const auto KPH = Script::Tools::ToHour(0, KillCount);

    const auto DeathCount = 0; //Death::GetDeathCount();
    Paint::Pixel ShardColor = { 255, 255, 255, 255 };
    Paint::Pixel KillColor = { 0, 255, 0, 255 };
    Paint::Pixel DeathColor = { 0, 255, 0, 255 };
    if (ShardCount > 0) ShardColor = { 0, 255, 0, 255 };
    if (DeathCount > 0) DeathColor = { 255, 191, 0, 255 };

    const std::string ZenyteStr = "Shards " + std::to_string(ShardCount) + " (" + std::to_string(SPH) + " p/h)";
    const std::string KilledStr = "Killed " + std::to_string(KillCount) + " (" + std::to_string(KPH) + " p/h)";
    const std::string DiedStr = "Died " + std::to_string(DeathCount) + " times";

    Paint::DrawSquare(TitleBox, 77, 77, 77, 50);
    Paint::DrawBox(TitleBox, 77, 77, 77, 255);
    Paint::DrawString(TitleStr, Point( ((TitleBox.GetX2() / 2) - (TitleStr.size() * 3)), TitleBox.Y + 6), BaseTextColor.Red, BaseTextColor.Green, BaseTextColor.Blue, BaseTextColor.Alpha);

    Paint::DrawSquare(CoinsBox, 77, 77, 77, 50);
    Paint::DrawBox(CoinsBox, 77, 77, 77, 255);

    BITMAP_COINS.Paint(CoinsStart);
    Paint::DrawString(CoinsStr, Point(CoinsStart.X + BITMAP_COINS.GetWidth() + 6, CoinsStart.Y + 5), CoinsColor.Red, CoinsColor.Green, CoinsColor.Blue, CoinsColor.Alpha);

    Paint::DrawSquare(StatsBox, 77, 77, 77, 50);
    Paint::DrawBox(StatsBox, 77, 77, 77, 255);

    Paint::DrawString(ZenyteStr, Point(StatsBox.X + 5, StatsBox.Y + 6), ShardColor.Red, ShardColor.Green, ShardColor.Blue, ShardColor.Alpha);
    Paint::DrawString(KilledStr, Point(StatsBox.X + 5, StatsBox.Y + 26), KillColor.Red, KillColor.Green, KillColor.Blue, KillColor.Alpha);
    Paint::DrawLine(Point(StatsBox.X, StatsBox.Y + 46), Point(StatsBox.GetX2(), StatsBox.Y + 46), 77, 77, 77, 125);
    Paint::DrawString(DiedStr, Point(StatsBox.X + 5, StatsBox.Y + 50), DeathColor.Red, DeathColor.Green, DeathColor.Blue, DeathColor.Alpha);

    //Paint::DrawLine(Point(InfoBox.X, InfoBox.Y + 35), Point(InfoBox.GetX2(), InfoBox.Y + 35), 77, 77, 77, 125);

    // zenytes, kills, deaths

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
    Script::Start(PaintMethod, true);
    return true;

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
    //GameListener::Instance().Start();
    return true;
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
