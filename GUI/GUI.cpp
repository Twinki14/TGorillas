#include <iostream>
#include <iomanip>
#include <Game/Interfaces/GameTabs/Inventory.hpp>
#include "GUI.hpp"
#include "../Config.hpp"
#include <TScript.hpp>
#include <BackgroundTask/AuthenticateTask.hpp>

ScriptGUI::GUIData GUI::GUIConfig;
HWND GUI::RShWnd;
bool GUI::Authenticated = false;

bool GUI::Init()
{
    GUI::GUIConfig.X                        = 0;            // X position of window
    GUI::GUIConfig.Y                        = 0;            // Y position of window
    GUI::GUIConfig.Width                    = 385;            // Window Width for creation
    GUI::GUIConfig.Height                   = 385;            // Window Height for creation - 370
    GUI::GUIConfig.FPSLimit                 = 30;
    GUI::GUIConfig.DrawGUIFrameMethod       = &GUI::DrawFrame;      // (Required) The method to run that contains your ImGui logic/code
    GUI::GUIConfig.Title                    = "TGorillas";           // (Optional) Window Title - Defaults to Script Name
    GUI::GUIConfig.LoadAlpacaClientStyle    = true;         // (Optional) Loads the same style used in the AlpacaClient GUI, if false the default imgui style is used
    GUI::GUIConfig.LoadFonts                = false;         // (Optional) Loads the Robot Regular text font and Material Design Icon font and sets it as the default for ImGui to use
    GUI::GUIConfig.OnGUIStartMethod         = &GUI::OnStart;      // (Optional) Runs this method after ImGui creates the context and loads fonts (if passed), directly before running the frame loop
    GUI::GUIConfig.OnGUIEndMethod           = &GUI::OnEnd;      // (Optional) Runs this method after the GUI shutdowns and cleans up
    GUI::Authenticated = false;
    ScriptGUI::Run(&GUI::GUIConfig);
    return ScriptGUI::Result;
}

void GUI::OnStart()
{
    GUI::RShWnd = ScriptGUI::GetRSWindowHandle();
    //GUI::LoadStyle();
}

void GUI::LoadStyle()
{
    ImGuiStyle& Style = ImGui::GetStyle();
    Style.ScrollbarSize = 12;
    Style.ScrollbarRounding = 0.0f;
    Style.WindowRounding = 0.0f;
    Style.FrameBorderSize = 0.0f;
    Style.PopupBorderSize = 0.5f;
    Style.PopupRounding = Style.WindowRounding;
    Style.TabRounding = Style.WindowRounding;

    ImVec4* Colors = Style.Colors;
    Colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    Colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    Colors[ImGuiCol_WindowBg]               = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    Colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    Colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    Colors[ImGuiCol_Border]                 = ImVec4(0.43f, 0.43f, 0.50f, 0.41f);
    Colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    Colors[ImGuiCol_FrameBg]                = ImVec4(0.11f, 0.15f, 0.15f, 0.99f);
    Colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.09f, 0.41f, 0.41f, 0.53f);
    Colors[ImGuiCol_FrameBgActive]          = ImVec4(0.18f, 0.59f, 0.58f, 0.28f);
    Colors[ImGuiCol_TitleBg]                = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    Colors[ImGuiCol_TitleBgActive]          = ImVec4(0.10f, 0.39f, 0.39f, 0.46f);
    Colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    Colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    Colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    Colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    Colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    Colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    Colors[ImGuiCol_CheckMark]              = ImVec4(0.07f, 0.57f, 0.56f, 1.00f);
    Colors[ImGuiCol_SliderGrab]             = ImVec4(0.05f, 0.48f, 0.47f, 0.64f);
    Colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.06f, 0.57f, 0.56f, 0.78f);
    Colors[ImGuiCol_Button]                 = ImVec4(0.05f, 0.48f, 0.47f, 0.37f);
    Colors[ImGuiCol_ButtonHovered]          = ImVec4(0.05f, 0.48f, 0.47f, 0.42f);
    Colors[ImGuiCol_ButtonActive]           = ImVec4(0.05f, 0.48f, 0.47f, 0.54f);
    Colors[ImGuiCol_Header]                 = ImVec4(0.05f, 0.48f, 0.47f, 0.64f);
    Colors[ImGuiCol_HeaderHovered]          = ImVec4(0.05f, 0.48f, 0.47f, 0.76f);
    Colors[ImGuiCol_HeaderActive]           = ImVec4(0.05f, 0.57f, 0.56f, 0.87f);
    Colors[ImGuiCol_Separator]              = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    Colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.05f, 0.48f, 0.47f, 0.76f);
    Colors[ImGuiCol_SeparatorActive]        = ImVec4(0.05f, 0.57f, 0.56f, 0.87f);
    Colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    Colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    Colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    Colors[ImGuiCol_Tab]                    = ImVec4(0.10f, 0.39f, 0.39f, 0.46f);
    Colors[ImGuiCol_TabHovered]             = ImVec4(0.10f, 0.39f, 0.39f, 0.61f);
    Colors[ImGuiCol_TabActive]              = ImVec4(0.10f, 0.39f, 0.39f, 0.83f);
    Colors[ImGuiCol_TabUnfocused]           = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
    Colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
    Colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    Colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    Colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    Colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    Colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.11f, 0.51f, 0.50f, 0.83f);
    Colors[ImGuiCol_DragDropTarget]         = ImVec4(0.110f, 0.510f, 0.500f, 0.830f);
    Colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    Colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    Colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    Colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

void GUI::DrawPane_Authenticate()
{
    static bool Attempted = false;
    static bool TryAuth = !Config::Get("ScriptAuthToken").as_string().empty();
    if (!Attempted) Attempted = AuthenticateTask::Task && AuthenticateTask::Task->Started();
    if (TryAuth)
    {
        AuthenticateTask::Start();
        TryAuth = false;
    }

    const auto RegionMax = ImGui::GetWindowContentRegionMax();

    if (AuthenticateTask::Task && AuthenticateTask::Task->Running())
    {
        ImGui::SetCursorPosX((RegionMax.x / 2) - 45.0f);
        ImGui::SetCursorPosY((RegionMax.y / 2) - (75.0f));
        ImGui::Spinner("###_Spinner", 45.0f, 5, ImGui::GetColorU32(ImGuiCol_ButtonHovered));
    } else
    {
        if (AuthenticateTask::Task && AuthenticateTask::Task->Finished())
        {
            GUI::Authenticated = AuthenticateTask::GetSucceeded();
            if (GUI::Authenticated)
            {
                GUI::DrawPane_Script();
                return;
            }
        }

        const float ItemWidth = RegionMax.x / 2.0f;
        if (Attempted)
            ImGui::SetCursorPosY((RegionMax.y / 2) - 43.0f);
        else
            ImGui::SetCursorPosY((RegionMax.y / 2) - 25.0f);

        if (Attempted)
        {
            std::string FailedReason;
            const auto Response = AuthenticateTask::GetResponse();

            switch (Response.Status)
            {
                case 0: FailedReason = "- Connection failed "; break;
                case 400: FailedReason = "- Incorrect token "; break;
                case 403: FailedReason = "- Forbidden - Header token invalid "; break;
                case 404: FailedReason = "- Page not found "; break;
                default: break;
            }

            char Buffer [256];
            sprintf(Buffer, "Failed %s(%i)", FailedReason.c_str(), Response.Status);
            const auto TextSize = ImGui::CalcTextSize(Buffer);
            ImGui::SetCursorPosX((RegionMax.x / 2) - (TextSize.x / 2));
            ImGui::TextColored(ImVec4(0.845f, 0.747f, 0.523f, 1.000f), "%s", Buffer);
        }

        static bool Copied = false;
        static char EditorScriptAuthToken[32];
        if (!Copied)
        {
            strcpy(EditorScriptAuthToken, Config::Get("ScriptAuthToken").as_string().c_str());
            Copied = true;
        }

        ImGui::PushItemWidth(ItemWidth);
        ImGui::SetCursorPosX((RegionMax.x / 2) - (ItemWidth / 2));
        bool HitEnter = ImGui::InputTextWithHint("###_Token", "Auth Token", EditorScriptAuthToken, sizeof(EditorScriptAuthToken), ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::PopItemWidth();

        static const auto GuiColHeader = ImGui::GetStyleColorVec4(ImGuiCol_Header);
        ImGui::PushStyleColor(ImGuiCol_Button, GuiColHeader);
        ImGui::SetCursorPosX((RegionMax.x / 2) - (ItemWidth / 2));
        bool CanAuth = strlen(EditorScriptAuthToken) > 0;
        ImGui::PushDisabled(!CanAuth);
        if (ImGui::Button("Authenticate", ImVec2(ItemWidth, 0)) || (CanAuth && HitEnter))
        {
            Config::Set("ScriptAuthToken", std::string(EditorScriptAuthToken));
            AuthenticateTask::Start();
        }
        ImGui::PopDisabled(!CanAuth);
        ImGui::PopStyleColor();
    }
}

void DrawTab_GUI()
{
    if (ImGui::BeginTabItem("GUI Debug"))
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("Dear ImGui %s", ImGui::GetVersion());
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::Separator();
        ImGui::ShowStyleEditor();
        ImGui::EndTabItem();
    }
}

void GUI::DrawPane_Script()
{
    static float TextV4[4] = { (float) TextColor.Red / 255.0f, (float) TextColor.Green / 255.0f, (float) TextColor.Blue / 255.0f, (float) TextColor.Alpha / 255.0f };

    if (ImGui::ColorEdit4("TextColor", (float*)&TextV4))
    {
        MiscLock.lock();
        TextColor = { static_cast<uint8_t>(TextV4[0] * 255), static_cast<uint8_t>(TextV4[1] * 255), static_cast<uint8_t>(TextV4[2] * 255), static_cast<uint8_t>(TextV4[3] * 255) };
        MiscLock.unlock();
    }

    constexpr const char* PassivityStrings[] = { "Exhilarated", "Hyper", "Mild", "Mellow", "Disinterested" };
    constexpr const char* FrequencyStrings[] = { "Never", "Rarely", "Regularly", "Frequently", "Very frequently" };
    constexpr const char* TendencyStrings[] = { "Very low", "Low", "Normal", "High", "Very high" };

    ImGui::Separator();
    {

    }

    ImGui::Separator();
    {
        static int Var_Passivity = Config::Get("Passivity").as_integer<int>();
        ImGui::Text("Passivity");
        ImGui::SameLine(125);
        ImGui::SetNextItemWidth(145);
        if (ImGui::Combo("###_Passivity", &Var_Passivity, PassivityStrings, IM_ARRAYSIZE(PassivityStrings)))
        {
            Config::Set("Passivity", Var_Passivity);
            Profile::Set(Profile::Var_Passivity, Var_Passivity);
        }
        ImGui::SameLine(0, 5);
        ImGui::HelpIcon("Affects how quickly the 'player' will click on the\nnext obstacle, and how long AFKs last", "[?]"); // Affects how quickly the 'player' will click on the next obstacle, and how long AFKs last
    }

    ImGui::Separator();
    {
        ImGui::Checkbox("Use hotkeys - Escape", (bool*) &Config::Cfg["UseHotkeys_Esc"]);
        ImGui::SameLine(0, 5.0f);
        ImGui::Checkbox("Use hotkeys - Gametabs", (bool*) &Config::Cfg["UseHotkeys_Gametabs"]);
    }

    ImGui::Separator();
    {
        static int Var_AFK_Frequency = Config::Get("AFK_Frequency").as_integer<int>();
        ImGui::Text("AFK Frequency");
        ImGui::SameLine(125);
        ImGui::SetNextItemWidth(145);
        if (ImGui::Combo("###_AFKFrequency", &Var_AFK_Frequency, FrequencyStrings, IM_ARRAYSIZE(FrequencyStrings)))
        {
            Config::Set("AFK_Frequency", Var_AFK_Frequency);
            Profile::Set(Profile::Var_AFK_Frequency, Var_AFK_Frequency);
        }
        ImGui::SameLine(0, 5);
        ImGui::HelpIcon("Affects how often the 'player' will go AFK", "[?]"); // Affects how often the 'player' will go AFK

        static int Var_TabOut_Tendency = Config::Get("TabOut_Tendency").as_integer<int>();
        ImGui::Text("Tabout tendency");
        ImGui::SameLine(125);
        ImGui::SetNextItemWidth(145);
        if (ImGui::Combo("###_TabOutTendency", &Var_TabOut_Tendency, TendencyStrings, IM_ARRAYSIZE(TendencyStrings)))
        {
            Config::Set("TabOut_Tendency", Var_TabOut_Tendency);
            Profile::Set(Profile::Var_TabOut_Tendency, Var_TabOut_Tendency);
        }
        ImGui::SameLine(0, 5);
        ImGui::HelpIcon("Affects how often 'AFKs' include tabbing out", "[?]"); // Affects how often 'AFKs' include tabbing out

        static int Var_Camera_Tendency = Config::Get("Camera_Tendency").as_integer<int>();
        ImGui::Text("Camera tendency");
        ImGui::SameLine(125);
        ImGui::SetNextItemWidth(145);
        if (ImGui::Combo("###Camera_Tendency", &Var_Camera_Tendency, FrequencyStrings, IM_ARRAYSIZE(FrequencyStrings)))
        {
            Config::Set("Camera_Tendency", Var_Camera_Tendency);
            Profile::Set(Profile::Var_Camera_Tendency, Var_Camera_Tendency);
        }
        ImGui::SameLine(0, 5);
        ImGui::HelpIcon("Affects how often the 'player' will move the camera around", "[?]"); // Affects how often the 'player' will move the camera around

    }

    ImGui::Separator();
    {
        ImGui::Checkbox("Debug logging", (bool*) &Config::Cfg["Debug_Logging"]);
        ImGui::SameLine(0, 5.0f);
        ImGui::Checkbox("Debug paint", (bool*) &Config::Cfg["Debug_Paint"]);
    }


/*    if (ImGui::BeginTabBar("###_Script", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton))
    {
        DrawTab_GUI();
        ImGui::EndTabBar();
    }*/
}

void GUI::DrawFrame()
{
    /**
    if (GUI::RShWnd)
    {
        std::int32_t X, Y;
        RECT WinRect;
        GetWindowRect(GUI::RShWnd, &WinRect);
        X = (WinRect.left + WinRect.right) / 2 - (ScriptGUI::GetWidth() / 2);
        Y = (WinRect.top + WinRect.bottom) / 2 - (ScriptGUI::GetHeight() / 2);
        if (X < 0) X = 0;
        if (Y < 0) Y = 0;
        SDL_SetWindowPosition(ScriptGUI::GetSDLWindow(), X, Y);
    }
    **/

    if (GUI::RShWnd)
    {
        std::int32_t X, Y;
        RECT WinRect;
        GetWindowRect(GUI::RShWnd, &WinRect);
        X = (WinRect.left - ScriptGUI::GetWidth()) - 17;
        Y = WinRect.top - 19;
        if (X < 0) X = 0;
        if (Y < 0) Y = 0;
        SDL_SetWindowPosition(ScriptGUI::GetSDLWindow(), X, Y);
    }

    static const auto GuiColHeader = ImGui::GetStyleColorVec4(ImGuiCol_Header);

    ImGui::SetNextWindowSize(ImVec2(ScriptGUI::GetWidth(), ScriptGUI::GetHeight()), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);

    if (ImGui::Begin(Script::GetName().c_str(), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar))
    {
        static const auto RegionWidth = ImGui::GetWindowContentRegionMax().x;
        static const auto RegionHeight = ImGui::GetWindowContentRegionMax().y;

        if (ImGui::BeginChild("###_Body", ImVec2(0, -40), true))
        {
            //if (!GUI::Authenticated)
                //GUI::DrawPane_Authenticate();
            //else
                GUI::DrawPane_Script();
        }

        ImGui::EndChild();

        ImGui::SetCursorPosY(RegionHeight - 36);
        ImGui::SetCursorPosX((RegionWidth / 2) - 175);
        if (ImGui::BeginChild("###_Footer", ImVec2(350, 35), true))
        {
            static const auto RegionWidthFooter = ImGui::GetWindowContentRegionMax().x;
            ImGui::PushStyleColor(ImGuiCol_Button, GuiColHeader);
            bool CanStart = GUI::CanStart();
            ImGui::PushDisabled(!CanStart);
            if (ImGui::Button("Start", ImVec2((RegionWidthFooter / 2.0f) - 6, 0)))
            {
                ScriptGUI::Result = true;
                ScriptGUI::Running = false;
            }
            ImGui::PopDisabled(!CanStart);
            ImGui::PopStyleColor();

            ImGui::SameLine(0, 4.0f);
            if (ImGui::Button("Quit", ImVec2((RegionWidthFooter / 2.0f) - 6, 0)))
            {
                ScriptGUI::Result = false;
                ScriptGUI::Running = false;
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void GUI::OnEnd()
{
    CloseHandle(GUI::RShWnd);
    AuthenticateTask::TerminateTask();
}

bool GUI::CanStart()
{
    //if (!GUI::Authenticated)
        //return false;

    return true;
}