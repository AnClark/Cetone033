#include "ImGui_UI.hpp"
#include "DistrhoPluginInfo.h"

ImGuiAboutWindow::ImGuiAboutWindow(TopLevelWidget* tlw)
    : ImGuiStandaloneWindow(tlw->getApp(), tlw->getWindow())
{
    setResizable(false);
    setTitle("About " DISTRHO_PLUGIN_NAME);
    setSize(Size<uint>(500, 220 - 2));

    // Must call this at the end of constructor, otherwise the main plugin window will be messed up (black background etc.) when opening this window.
    done(); 
}

void ImGuiAboutWindow::idleCallback()
{
    if (_pendingHide)
    {
        _pendingHide = false;
        // idleCallback() is timer-driven and runs BETWEEN frames, so it is never
        // on the render call stack. This makes it safe to call close() here.
        //
        // Why close() and not hide():
        //   hide() only makes the native window invisible but keeps the window object
        //   "alive" in the Application. When the main window is closed afterwards,
        //   the hidden-but-not-closed window prevents the Application from quitting,
        //   leaving the process lingering in the background.
        //   close() properly unregisters the window from the Application, so the
        //   process exits cleanly once all windows are gone.
        this->close();
        return; // Window is closing – skip the repaint.
    }

    // Normal path: trigger the next frame repaint.
    ImGuiStandaloneWindow::idleCallback();
}

void ImGuiAboutWindow::onImGuiDisplay()
{
    double scaleFactor = getScaleFactor() * userScaling;
    const double initialSize = 500 * scaleFactor;

    //
    // "About" Window
    //
    {
        // Make window fullscreen
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);

        ImGui::Begin("About " DISTRHO_PLUGIN_NAME, NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
        {
            ImGui::SeparatorText(DISTRHO_PLUGIN_NAME);

            ImGui::Text("Monophonic Chiptune synthesizer, by Neotec Software.");
            ImGui::SameLine(0, 80 - 8);
            if (ImGui::Button("OK", ImVec2(80, 0)))
            {
                // Do NOT call hide() / close() directly here!
                // onImGuiDisplay() is called from within the Pugl paint event (onPuglExpose
                // → onDisplay → onImGuiDisplay). Any window-lifecycle call made mid-frame
                // (hide, close, stopModal…) will corrupt the ImGui context or terminate the
                // modal event loop while the render call stack is still alive, causing
                // crashes on subsequent ImGui calls in the same frame.
                //
                // Solution: set a deferred flag here and let idleCallback() – which is
                // timer-driven and executes between frames, outside the render call stack –
                // safely perform the actual close().
                _pendingHide = true;
            }

            ImGui::Text("Copyright © 2007, Neotec Software.");
            ImGui::Text("Copyright © 2024-2025, AnClark Liu <clarklaw4701@qq.com>.");

            ImGui::SeparatorText("Authors");
            ImGui::BulletText("René 'Neotec' Jeschke - Original developer");
            ImGui::BulletText("AnClark Liu - Ported to DPF, Further developments");

            ImGui::SeparatorText("License");
            ImGui::BulletText("This project is licensed under GNU General Public License, version 3.");
        }
        ImGui::End(); 

    }

}
