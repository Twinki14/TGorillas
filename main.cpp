#include <Game/Core.hpp>
#include <TScript.hpp>
#include <Tools/OSRSBox/OSRSBox.hpp>
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

/*
 * Abyssal whip - Prefers arclight if there's charges available, will switch to whip as backup
 * Blowpipe - With plenty of scales/darts, the script should recharge
 * Prayer/Super restores - The script should tell you what the "player" will prefer, the setting is unique to the player name, but can be overriden
 * Manta rays - Primary food
 * Anglerfish - Should only use one or two per inventory/trip, it's only used for 'filler' slots for equipment swaps
 * Divine ranging/super combat potions - one of each per trip
 * Ring of dueling / POH tabs - These are used to restore stats at the end of a trip, if the player has a good POH, it'll use teleport tabs, otherwise RoDs for the pvp portal
 * Royal seed pods - Used for deathtrips, have like 5-10 in the bank
 * Rune pouch / High alchemy runes - Lava runes and nature runes for high alchemy, will only high alch if the account has a rune pouch available, can no longer just buy rune pouches
 */

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

    bool LoggedIn = Mainscreen::IsLoggedIn();
    std:int32_t LeftStartX = LoggedIn ? 5 : Internal::GetLoginScreenX() + 5;

    Paint::Pixel BaseTextColor = { 0, 214, 0, 255};  //GUI::GetTextColor();

    auto StatusBox = GameListener::GetPaintStatusBox();
    if (StatusBox.X < 0 || StatusBox.Y < 0 || !LoggedIn)
    {
        if (!LoggedIn)
        {
            StatusBox.X = LeftStartX;
            StatusBox.Y = 435;
        } else
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
    }

    Paint::DrawSquare(StatusBox, 77, 77, 77, 50);
    Paint::DrawBox(StatusBox, 77, 77, 77, 255);
    Paint::DrawString("Status - " + Script::GetStatus(), Point(StatusBox.X + 5, StatusBox.Y + 5), BaseTextColor.Red, BaseTextColor.Green, BaseTextColor.Blue, BaseTextColor.Alpha);

    const auto TitleBox = Box(LeftStartX, 20, 185, 25);
    const auto CoinsBox = Box(LeftStartX, TitleBox.GetY2() + 5, TitleBox.Width, 35);
    const auto StatsBox = Box(LeftStartX, CoinsBox.GetY2() + 5, TitleBox.Width, 69);

    const std::string TitleStr = "TGorillas | " + MillisToHumanShort(Script::GetTimeElapsed());

    const auto TotalLootProfit = 0; //Tracker::GetTotalProfit();
    const std::string CoinsStr = Script::Tools::FormatRunescapeGold(TotalLootProfit) + " [" + Script::Tools::FormatRunescapeGold(Script::Tools::ToHour(0, TotalLootProfit)) + " p/h]";
    const auto CoinsStart = Point((CoinsBox.GetMiddle().X) - ((BITMAP_COINS.GetWidth() + (CoinsStr.size() * 7) + 6) / 2), CoinsBox.Y + 5);

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
    Paint::DrawString(TitleStr, Point( ((TitleBox.GetMiddle().X) - (TitleStr.size() * 3)), TitleBox.Y + 6), BaseTextColor.Red, BaseTextColor.Green, BaseTextColor.Blue, BaseTextColor.Alpha);

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

    auto AccountBox = Box(StatusBox.X, StatusBox.Y - 195, 225, 184);
/*    static auto RawUsername = LoggedIn ? Internal::GetUsername() : "-----";
    static auto RunescapeUsername = LoggedIn ? Internal::GetLocalPlayer().GetNamePair().GetCleanName() : "-----";
    if (LoggedIn && (RawUsername == "-----" || RunescapeUsername == "-----" || RawUsername.empty() || RunescapeUsername.empty()))
    {
        if (RawUsername == "-----" || RawUsername.empty()) RawUsername = Internal::GetUsername();
        if (RunescapeUsername == "-----" || RunescapeUsername.empty()) RunescapeUsername = Internal::GetLocalPlayer().GetNamePair().GetCleanName();
    }*/

    const std::string AccountLabel = GameListener::GetLocalPlayerUsername() + " [" + GameListener::GetLocalPlayerName() + "]";
    const std::string BankValueLabel = "Bank value - 20M";
    const std::string FocusLabel = "Focus - Hyper";
    const std::string PassivityLabel = "Passivity - Mild";
    const std::string AFKFrequencyLabel = "AFK Frequency - Very frequently";
    const std::string WorldLabel = "World - " + std::to_string(Internal::GetCurrentWorld());
    const std::string TimeSpentAFKLabel = "Spent " + MillisToHumanShort(100) + " AFK";
    const std::string TimeSpentABreakingLabel = "Spent " +  MillisToHumanShort(BreakHandler::GetBreakTimer().GetTimeElapsed()) + " breaking";

    AccountBox.Width = AccountLabel.size() * 8 + 6;
    if (AccountBox.Width < 225) AccountBox.Width = 225;

    Paint::DrawSquare(AccountBox, 77, 77, 77, 50);
    Paint::DrawBox(AccountBox, 77, 77, 77, 255);

    Paint::DrawString(AccountLabel, Point(AccountBox.X + 5, AccountBox.Y + 6), 0, 255, 255, 255);
    Paint::DrawLine(Point(AccountBox.X, AccountBox.Y + 26), Point(AccountBox.GetX2(), AccountBox.Y + 26), 77, 77, 77, 125);
    Paint::DrawString(BankValueLabel, Point(AccountBox.X + 5, AccountBox.Y + 30), 0, 255, 0, 255);
    Paint::DrawLine(Point(AccountBox.X, AccountBox.Y + 50), Point(AccountBox.GetX2(), AccountBox.Y + 50), 77, 77, 77, 125);
    Paint::DrawString(FocusLabel, Point(AccountBox.X +  5, AccountBox.Y + 54), 255, 255, 255, 255);
    Paint::DrawString(PassivityLabel, Point(AccountBox.X +  5, AccountBox.Y + 74), 255, 255, 255, 255);
    Paint::DrawString(AFKFrequencyLabel, Point(AccountBox.X +  5, AccountBox.Y + 94), 255, 255, 255, 255);
    Paint::DrawLine(Point(AccountBox.X, AccountBox.Y + 114), Point(AccountBox.GetX2(), AccountBox.Y + 114), 77, 77, 77, 125);
    Paint::DrawString(WorldLabel, Point(AccountBox.X +  5, AccountBox.Y + 118), 0, 255, 255, 255);
    Paint::DrawLine(Point(AccountBox.X, AccountBox.Y + 138), Point(AccountBox.GetX2(), AccountBox.Y + 138), 77, 77, 77, 125);
    Paint::DrawString(TimeSpentAFKLabel, Point(AccountBox.X +  5, AccountBox.Y + 142), 255, 255, 255, 255);
    Paint::DrawString(TimeSpentABreakingLabel, Point(AccountBox.X +  5, AccountBox.Y + 162), 255, 255, 255, 255);

    auto SuppliesBox = GameListener::GetPaintSuppliesBox();
    if (SuppliesBox.X < 0 || SuppliesBox.Y < 0 || !LoggedIn)
    {
        if (!LoggedIn)
        {
            SuppliesBox.X = LeftStartX + 544;
            SuppliesBox.Y = 200;
        } else
        {
            const auto MinimapMiddle = Minimap::GetMiddle();
            const auto Canvas = Internal::Client.GetCanvas();
            const auto CanvasBounds = Box(0, 0, Canvas.GetWidth(), Canvas.GetHeight());
            if (CanvasBounds.Height >= 748)
            {
                SuppliesBox.X = MinimapMiddle.X - (SuppliesBox.Width / 2 + 32);
                SuppliesBox.Y = MinimapMiddle.Y + MinimapMiddle.Y + 6;
            } else
            {
                SuppliesBox.X = MinimapMiddle.X - (SuppliesBox.Width + 145);
                SuppliesBox.Y = MinimapMiddle.Y - 35;
            }
        }
    }

    std::int32_t SupplyTripsLeft = 0;
    std::int32_t RestoresLeft = 0;
    std::int32_t FoodLeft = 0;
    std::int32_t RangingLeft = 0;
    std::int32_t CombatLeft = 0;
    std::int32_t HighAlchemyCastsLeft = 0;
    std::int32_t RoDTeleportsLeft = 0;
    std::int32_t RoyalSeedPodsLeft = 0;
    std::int32_t ScalesLeft = 0;
    std::int32_t DartsLeft = 0;
    std::int32_t ArclightChargesLeft = 0;
    const std::string SuppliesLabel = "Supplies - " + std::to_string(SupplyTripsLeft) + " est. trips left";
    const std::string RestoresLabel = "Restores - " + std::to_string(RestoresLeft);
    const std::string FoodsLabel = "Manta rays - " + std::to_string(FoodLeft);
    const std::string RangingLabel = "Ranging [4] - " + std::to_string(RangingLeft);
    const std::string CombatsLabel = "Sup. Combat [4] - " + std::to_string(CombatLeft);
    const std::string HighAlchsLabel = "High Alchemy casts - " + std::to_string(HighAlchemyCastsLeft);
    const std::string RoDLabel = "RoD [Total teleports] - " + std::to_string(RoDTeleportsLeft);
    const std::string RoyalSeedPodsLabel = "Royal seed pods - " + std::to_string(RoyalSeedPodsLeft);
    const std::string BlowPipeScalesLabel = "Scales - " + std::to_string(ScalesLeft) + " Darts - " + std::to_string(DartsLeft);
    const std::string ArclightCharges = "Arclight charges - " + std::to_string(ArclightChargesLeft);
    const std::string BarrowsLabel = "Barrows - 4/4";

    Paint::DrawSquare(SuppliesBox, 77, 77, 77, 50);
    Paint::DrawBox(SuppliesBox, 77, 77, 77, 255);

    Point Next = Point( ((SuppliesBox.GetMiddle().X) - (SuppliesLabel.size() * 3)), SuppliesBox.Y + 6);
    Paint::DrawString(SuppliesLabel, Next, BaseTextColor.Red, BaseTextColor.Green, BaseTextColor.Blue, BaseTextColor.Alpha); Next = Point(SuppliesBox.X, SuppliesBox.Y + 26);
    Paint::DrawLine(Next, Point(SuppliesBox.GetX2(), SuppliesBox.Y + 26), 77, 77, 77, 125); Next = Point(SuppliesBox.X + 5, SuppliesBox.Y + 30);
    Paint::DrawString(RestoresLabel, Next, 255, 255, 255, 255); Next.Y += 20;
    Paint::DrawString(FoodsLabel, Next, 255, 255, 255, 255); Next.Y += 20;
    Paint::DrawString(RangingLabel, Next, 255, 255, 255, 255); Next.Y += 20;
    Paint::DrawString(CombatsLabel, Next, 255, 255, 255, 255); Next.Y += 20;
    Paint::DrawString(HighAlchsLabel, Next, 255, 255, 255, 255); Next = Point(SuppliesBox.X, Next.Y + 20);
    Paint::DrawLine(Next, Point(SuppliesBox.GetX2(), Next.Y), 77, 77, 77, 125); Next = Point(SuppliesBox.X + 5, Next.Y + 4);
    Paint::DrawString(RoDLabel, Next, 255, 255, 255, 255); Next.Y += 20;
    Paint::DrawString(RoyalSeedPodsLabel, Next, 255, 255, 255, 255); Next = Point(SuppliesBox.X, Next.Y + 20);
    Paint::DrawLine(Next, Point(SuppliesBox.GetX2(), Next.Y), 77, 77, 77, 125); Next = Point(SuppliesBox.X + 5, Next.Y + 4);
    Paint::DrawString(BlowPipeScalesLabel, Next, 255, 255, 255, 255); Next.Y += 20;
    Paint::DrawString(ArclightCharges, Next, 255, 255, 255, 255); Next.Y += 20;
    Paint::DrawString(BarrowsLabel, Next, 255, 255, 255, 255); Next.Y += 20;

    Gorillas::Draw();
    Paint::DrawDot(GetMousePos(), 2.5f, 0, 255, 255, 255);

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
        GameListener::Instance().Start();
        return true;
    }
    Script::Shutdown();
    return false;
}

bool Loop()
{
    auto I = GearSets::Item("Verac's plateskirt 0");
    return false;

   /* for (std::uint32_t I = Equipment::HEAD; I <= Equipment::AMMO; I++)
    {
        if (GearSets::Sets["Melee"].Items[I].Name != "NULL" && GearSets::Sets["Ranged"].Items[I].Name != "NULL")
        {
            if (GearSets::Sets["Melee"].Items[I].Name != GearSets::Sets["Ranged"].Items[I].Name)
            {
                //MeleeGearIDs.emplace_back(GearSets::Sets["Melee"].Items[I].ID);
                //RangedGearIDs.emplace_back(GearSets::Sets["Ranged"].Items[I].ID);

                if (GearSets::Sets["Ranged"].Items[I].Name.find("Toxic blowpipe") != std::string::npos) // 2h
                {
                    DebugLog("Melee > {}, {}", GearSets::Sets["Melee"].Items[Equipment::SHIELD].Name, GearSets::Sets["Melee"].Items[Equipment::SHIELD].ID);
                }

                DebugLog("Melee > {}, {}", GearSets::Sets["Melee"].Items[I].Name, GearSets::Sets["Melee"].Items[I].ID);
                DebugLog("Ranged > {}, {}", GearSets::Sets["Ranged"].Items[I].Name, GearSets::Sets["Ranged"].Items[I].ID);
            }
        }
    }*/
    return false;
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
