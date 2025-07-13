#include "ImGui_UI.hpp"
#include "DistrhoPluginInfo.h"

#include "structures.h"
#include "defines.h"

#include "CetoneUI.hpp" // For class CCetoneUI

void ImGuiUI::onImGuiDisplay()
{
    double scaleFactor = getScaleFactor() * userScaling;
    const double initialSize = 500 * scaleFactor;

    //
    // "About" Window
    //
    {
        ImGui::SetNextWindowPos(ImVec2(initialSize / 6, initialSize / 16), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(500, 220 - 2), ImGuiCond_Once);

        if (isAboutWindowOpen)
        {
            ImGui::Begin("About " DISTRHO_PLUGIN_NAME, &isAboutWindowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
            {
                ImGui::SeparatorText(DISTRHO_PLUGIN_NAME);

                ImGui::Text("Monophonic Chiptune synthesizer, by Neotec Software.");
                ImGui::SameLine(0, 80 - 8);
                if (ImGui::Button("OK", ImVec2(80, 0)))
                    isAboutWindowOpen = false;    

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

    //
    // Handle menu opening requests
    //
    // Here, variable `requestTestMenuOpen` acts as an "event flag" to request ImGui to show the menu.
    //
    // Dear ImGui has its own mechanism to show popup menus, which does not require a flag to control its exisitance.
    // This is quite different from window (ImGui::Begin()).
    // So just call this function once, your popup will stick on the screen unless you do some operations.
    //
    switch (requestMenuId)
    {
        case pOsc1Wave:
            ImGui::OpenPopup("menu_osc1_wave");
            requestMenuId = 0;
            break;
        case pOsc2Wave:
            ImGui::OpenPopup("menu_osc2_wave");
            requestMenuId = 0;
            break;
        case pFilterType:
            ImGui::OpenPopup("menu_filter_type");
            requestMenuId = 0;
            break;

        default:
            requestMenuId = 0;
    }

    //
    // Create popup menus
    // [NOTICE] ImGui::BeginPopup() MUST be put after ImGui::OpenPopup(), otherwise popup won't show!
    //

    ImGui::SetNextWindowPos(menuPos);       // Specify menu position

    if (ImGui::BeginPopup("menu_osc1_wave"))
    {
        ImGui::SeparatorText("Waveform");
        if (ImGui::MenuItem("Saw")) { _triggerParamUpdate(pOsc1Wave, ui->_pi2f(WAVE_SAW, WAVE_MAX)); }
        if (ImGui::MenuItem("Pulse")) { _triggerParamUpdate(pOsc1Wave, ui->_pi2f(WAVE_PULSE, WAVE_MAX)); }
        if (ImGui::MenuItem("Triangle")) { _triggerParamUpdate(pOsc1Wave, ui->_pi2f(WAVE_TRI, WAVE_MAX)); }
        ImGui::EndPopup();
    }

    ImGui::SetNextWindowPos(menuPos);       // Specify menu position

    if (ImGui::BeginPopup("menu_osc2_wave"))
    {
        ImGui::SeparatorText("Waveform");
        if (ImGui::MenuItem("Saw")) { _triggerParamUpdate(pOsc2Wave, ui->_pi2f(WAVE_SAW, WAVE_MAX)); }
        if (ImGui::MenuItem("Pulse")) { _triggerParamUpdate(pOsc2Wave, ui->_pi2f(WAVE_PULSE, WAVE_MAX)); }
        if (ImGui::MenuItem("Triangle")) { _triggerParamUpdate(pOsc2Wave, ui->_pi2f(WAVE_TRI, WAVE_MAX)); }
        ImGui::EndPopup();
    }

    // NOTICE:
    // For menus below, no need to specify menu position. Let Dear ImGui decide menu's position.
    // Otherwise, menu will partially show on the screen due to insufficient space.

    if (ImGui::BeginPopup("menu_filter_type"))
    {
        ImGui::SeparatorText("Filter Type");
        if (ImGui::MenuItem("Biquad")) { _triggerParamUpdate(pFilterType, ui->_pi2f(FILTER_TYPE_BIQUAD, FILTER_TYPE_MAX)); }
        if (ImGui::MenuItem("Moogle (Moog)")) { _triggerParamUpdate(pFilterType, ui->_pi2f(FILTER_TYPE_MOOG, FILTER_TYPE_MAX)); }
        ImGui::EndPopup();
    }
    
    //
    // Toolbar
    //
    // TIPS:
    // By adding a Dear ImGui window without border and decorations, we can add any Dear ImGui widgets to our plugin UI,
    // without relying on a standard Dear ImGui window.
    //

    // Set the size and location of toolbar area
    ImGui::SetNextWindowPos(ImVec2(100, 5));
    ImGui::SetNextWindowSize(ImVec2(100, 20));

    // Set window padding to zero
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    // Create a window as the "container" of ImGui widgets. This window has no backgrounds and decorations, and not moveable.
    ImGui::Begin("Toolbar", NULL, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar);
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0x3c / 255.0f, 0x9e / 255.0f, 0x3c / 255.0f, 1.0f));

        ImGui::Button("Test Button", ImVec2(100, 20));

        ImGui::PopStyleColor();
    }
    ImGui::End();

    ImGui::PopStyleVar();
}

void ImGuiUI::_triggerParamUpdate(uint32_t paramId, float newValue)
{
    ui->setParameterValue(paramId, newValue);   // Tell the DSP to update parameter value
    ui->parameterChanged(paramId, newValue);    // Request UI refresh
}
