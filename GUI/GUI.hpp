#ifndef GUI_HPP_INCLUDED
#define GUI_HPP_INCLUDED

#include <ScriptGUI.hpp>

class GUI
{
    public:
        static bool Init();

    private:
        static ScriptGUI::GUIData GUIConfig;
        static HWND RShWnd;
        static bool Authenticated;

        static void OnStart();
        static void LoadStyle();
        static void DrawFrame();
        static void DrawPane_Authenticate();
        static void DrawPane_Script();
        static void OnEnd();

        static bool CanStart();
};

#endif // GUI_HPP_INCLUDED