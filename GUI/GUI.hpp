#ifndef GUI_HPP_INCLUDED
#define GUI_HPP_INCLUDED

#include <mutex>
#include <ScriptGUI.hpp>
#include <Core/Paint.hpp>

class GUI
{
    public:
        static bool Init();
        inline static Paint::Pixel GetTextColor()
        {
            std::lock_guard<std::mutex> Lock(MiscLock);
            return TextColor;
        }

    private:
        static ScriptGUI::GUIData GUIConfig;
        static HWND RShWnd;
        static bool Authenticated;

        inline static std::mutex MiscLock;
        inline static Paint::Pixel TextColor = { 64, 241, 255, 255 };

        static void OnStart();
        static void LoadStyle();
        static void DrawFrame();
        static void DrawPane_Authenticate();
        static void DrawPane_Script();
        static void OnEnd();

        static bool CanStart();
};

#endif // GUI_HPP_INCLUDED