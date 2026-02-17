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
    // Toolbar area - resides at the same line as the title bar, on the right hand side of the logo
    //
    if (ImGui::Begin("Main Toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground))
    {
        ImGui::SetWindowPos(ImVec2(120, -2));
        ImGui::SetWindowSize(ImVec2(100, 50));

        constexpr ImVec4 _labelColor(25.0f / 255.0f, 132.0f / 255.0f, 25.0f / 255.0f, 1.0f);


        // Lambda to apply button style and execute a function with RAII (Resource Acquisition Is Initialization)
        auto withButtonStyle_Toolbar = [&](auto&& func) {
            constexpr ImVec4 _buttonColor(50.0f / 255.0f, 158.0f / 255.0f, 56.0f / 255.0f, 1.0f);
            constexpr ImVec4 _buttonHoverColor(60.0f / 255.0f, 168.0f / 255.0f, 66.0f / 255.0f, 1.0f);
            constexpr ImVec4 _buttonActiveColor(30.0f / 255.0f, 138.0f / 255.0f, 36.0f / 255.0f, 1.0f);

            ImGui::PushStyleColor(ImGuiCol_Button, _buttonColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, _buttonHoverColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, _buttonActiveColor);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
            func();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);
        };

#ifdef ENABLE_POLYPHONY
        // Polyphony switch
        {
            ImGui::AlignTextToFramePadding();   // Make text vertically centered with the button
            ImGui::TextColored(_labelColor, "Poly.");

            ImGui::SameLine(0, 5);

            String _buttonLabel = String(ui->fMaxPolyphony) + "##PolyphonyButton";
            withButtonStyle_Toolbar([&]() {
                if (ImGui::Button(_buttonLabel.buffer(), ImVec2(60 - 20, 0)))
                {
                    ImGui::OpenPopup("Polyphony Config");
                }
            });
        }

        // Polyphony configuration popup
        if (ImGui::BeginPopup("Polyphony Config"))
        {
            ImGui::SeparatorText("Polyphony Configuration");
            {
                ImGui::Text("Max polyphony:");
                ImGui::Dummy(ImVec2(0, 2));

                if (ImGui::SliderInt("##PolyphonySlider", reinterpret_cast<int*>(&ui->fMaxPolyphony), 1, MAX_POLYPHONY, ui->fMaxPolyphony <= 1 ? "Monopoly" : "%d"))
                {
                    _triggerParamUpdate(pMaxPolyphony, static_cast<float>(ui->fMaxPolyphony));
                }
            }
            ImGui::Dummy(ImVec2(0, 2));
            {
                if (ImGui::Button("OK", ImVec2(70, 0)))
                {
                    ImGui::CloseCurrentPopup();
                }

                ImGui::SameLine(0, 18);

                if (ImGui::Button("Set to Monopoly", ImVec2(120, 0)))
                {
                    _triggerParamUpdate(pMaxPolyphony, 1);
                }
            }

            ImGui::EndPopup();
        }
#endif

        ImGui::End();
    }
}

void ImGuiUI::_triggerParamUpdate(uint32_t paramId, float newValue)
{
    ui->setParameterValue(paramId, newValue);   // Tell the DSP to update parameter value
    ui->parameterChanged(paramId, newValue);    // Request UI refresh
}
