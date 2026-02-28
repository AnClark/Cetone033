#include "ImGui_UI.hpp"

#include "structures.h"
#include "defines.h"

#include "CetoneUI.hpp" // For class CCetoneUI

void ImGuiUI::onImGuiDisplay()
{
    //
    // Handle message box queue.
    //
    _handleMessageBoxIdle();

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
    // Handle modal popup opening requests
    //
    // Similar to menu popups above, we should use a event flag to control the display of modal popups.
    // You cannot invoke ImGui::OpenPopup() within a popup menu, otherwise the modal popup will not show.
    // So, an event flag is necessary to request the opening of modal popups.
    //
    // See _handlePresetModalPopupRequests() for details.
    //
    // NOTE: Putting ImGui::BeginPopupModal() within a ImGui window is OK in pure Dear ImGui application.
    //       But this project is a DPF plugin, with a hybrid UI architecture (DGL Widgets + Dear ImGui).
    //       Putting ImGui::BeginPopupModal() within a ImGui Window (e.g. "Main Toolbar" window) will
    //       cause the modal popup to fail to show.
    //
    _handlePresetModalPopupRequests();

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
        ImGui::SetWindowSize(ImVec2(300, 50));

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

        ImGui::SameLine(0, 20);
#endif

        // Preset manager button
        {
            ImGui::AlignTextToFramePadding();   // Make text vertically centered with the button
            ImGui::TextColored(_labelColor, "Preset");

            ImGui::SameLine(0, 5 + 4);

            withButtonStyle_Toolbar([&]() {
                // TODO: Button name should be the current preset name.
                const String buttonLabel = ui->fCurrentPresetName + String(ui->fPresetIsModified ? "*" : "") +  String("##PresetButton");
                if (ImGui::Button(buttonLabel.buffer(), ImVec2(120, 0)))
                {
                    _shouldRefreshBankList = true; // Refresh bank list cache before showing menu
                    ImGui::OpenPopup("Preset Menu");
                }
            });
        }

        // Preset manager menu
        _buildPresetManagementMenu();

        ImGui::End();
    }

    //
    // Create modal popups
    // [NOTICE] Similar to menu popups above, ImGui::BeginModal() MUST be put after ImGui::OpenPopup(), otherwise popup won't show!
    //

    _buildPresetManagementPopups();

    //
    // Handle file browser dialog.
    //
    // Request to open file browser dialog is triggered by setting event flags (e.g. requestSavePresetPopup) when user clicks the corresponding menu item.
    // Remember to invoke this function at the end of onImGuiDisplay(), otherwise the file browser dialog will not function correctly.
    //

    _handleFileBrowserIdle();
}

void ImGuiUI::_triggerParamUpdate(uint32_t paramId, float newValue)
{
    ui->setParameterValue(paramId, newValue);   // Tell the DSP to update parameter value
    ui->parameterChanged(paramId, newValue);    // Request UI refresh
    
    // Mark preset as modified when any parameter is changed
    if (!ui->fPresetIsModified)
    {
        ui->fPresetIsModified = true;
        ui->setState(STATE_PRESET_MODIFIED, "true");
    }
}
