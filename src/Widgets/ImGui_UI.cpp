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
    // NOTE: Putting ImGui::BeginPopupModal() within a ImGui window is OK in pure Dear ImGui application.
    //       But this project is a DPF plugin, with a hybrid UI architecture (DGL Widgets + Dear ImGui).
    //       Putting ImGui::BeginPopupModal() within a ImGui Window (e.g. "Main Toolbar" window) will
    //       cause the modal popup to fail to show.
    //
    if (requestRenamePresetPopup)
    {
        ImGui::OpenPopup("Rename Preset");
        requestRenamePresetPopup = false;
    }
    
    if (requestSavePresetPopup)
    {
        ImGui::OpenPopup("Save Preset");
        requestSavePresetPopup = false;
    }
    
    if (requestDeletePresetPopup)
    {
        ImGui::OpenPopup("Delete Preset");
        requestDeletePresetPopup = false;
    }

    if (requestNewBankPopup)
    {
        ImGui::OpenPopup("New Bank");
        requestNewBankPopup = false;
    }

    if (requestRenameBankPopup)
    {
        ImGui::OpenPopup("Rename Bank");
        requestRenameBankPopup = false;
    }

    if (requestDeleteBankPopup)
    {
        ImGui::OpenPopup("Delete Bank");
        requestDeleteBankPopup = false;
    }

    if (requestConfirmOverwritePresetPopup)
    {
        ImGui::OpenPopup("Overwrite Preset?");
        requestConfirmOverwritePresetPopup = false;
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

        // Preset menu popup
        if (ImGui::BeginPopup("Preset Menu"))
        {
            ImGui::SeparatorText("Preset Bank");
            {
                if (ImGui::BeginMenu("Factory Presets"))
                {
                    for (uint32_t i = 0; i < 128; i++)
                    {
                        const String presetNameLabel = ui->fPresetManager->getFactoryProgramName(i) + String("##FactoryPreset") + String(i);
                        if (ImGui::MenuItem(presetNameLabel.buffer()))
                        {
                            ui->fPresetManager->loadFactoryProgram(i);
                            ui->_updateState(ui->fPresetManager->getFactoryProgramName(i).buffer(), FACTORY_BANK_NAME, false);
                        }
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::MenuItem("Default Preset"))
                {
                    ui->fPresetManager->loadDefaultProgram();
                    ui->_updateState(DEFAULT_PRESET_NAME, FACTORY_BANK_NAME, false);
                }

                // Default bank submenu
                if (ImGui::BeginMenu(DEFAULT_USER_BANK_NAME))
                {
                    size_t userPresetCount = ui->fPresetManager->getDefaultBankPresetCount();
                    
                    if (userPresetCount == 0)
                    {
                        ImGui::MenuItem("(No presets)", nullptr, false, false);
                    }
                    else
                    {
                        for (size_t i = 0; i < userPresetCount; i++)
                        {
                            String presetName = ui->fPresetManager->getDefaultBankPresetName(i);
                            const String presetNameLabel = presetName + String("##DefaultBank") + String((int)i);
                            
                            if (ImGui::MenuItem(presetNameLabel.buffer()))
                            {
                                if (ui->fPresetManager->loadPresetFromDefaultBank(presetName.buffer()))
                                {
                                    ui->_updateState(presetName.buffer(), DEFAULT_USER_BANK_NAME, false);
                                }
                            }
                        }
                    }
                    
                    ImGui::Separator();
                    
                    if (ImGui::MenuItem("Save Current as..."))
                    {
                        strncpy(_presetNameEditorBuffer, ui->fCurrentPresetName.buffer(), MAX_PRESET_NAME_LENGTH);
                        _presetNameEditorBuffer[MAX_PRESET_NAME_LENGTH - 1] = '\0';
                        _pendingSaveBankName = DEFAULT_USER_BANK_NAME;
                        requestSavePresetPopup = true;
                    }
                    
                    // Only enable "Overwrite Current" when the currently loaded preset actually belongs to Default Bank
                    const bool isCurrentInDefaultBank = (ui->fCurrentPresetBank == DEFAULT_USER_BANK_NAME);
                    if (ImGui::MenuItem("Overwrite Current", nullptr, false, isCurrentInDefaultBank))
                    {
                        // Save current preset with same name (overwrite) to Default Bank
                        SynthProgram snapshot = ui->fPresetManager->captureCurrentParameters();
                        ui->fPresetManager->savePresetToDefaultBank(ui->fCurrentPresetName.buffer(), snapshot);
                        ui->_updateState(ui->fCurrentPresetName.buffer(), DEFAULT_USER_BANK_NAME, false);
                        ui->logAndShowMessage("Preset '%s' overwritten in Default Bank.", ui->fCurrentPresetName.buffer());
                    }
                    
                    ImGui::Separator();

                    if (ImGui::MenuItem("Export Bank...")) {
                        _fileBrowserBankName = DEFAULT_USER_BANK_NAME;
                        DGL_NAMESPACE::FileBrowserOptions opts;
                        opts.saving = true;
                        opts.title = "Export Default Bank";
                        opts.defaultName = DEFAULT_USER_BANK_FILENAME;
                        _fileBrowserHandle = DGL_NAMESPACE::fileBrowserCreate(false, getWindow().getNativeWindowHandle(),
                                                               getScaleFactor(), opts);
                        _fileBrowserAction = kFileBrowserExportBank;
                    }

                    ImGui::Separator();
                    
                    String bankInfo = String("Bank: ") + ui->fPresetManager->getDefaultBankName();
                    ImGui::MenuItem(bankInfo.buffer(), nullptr, false, false);
                    
                    // Note: User Bank cannot be renamed to avoid naming conflicts
                    // and maintain its special status as the default bank
                    
                    ImGui::EndMenu();
                }


                // Dynamically loaded Banks (imported by user)
                if (_shouldRefreshBankList) {
                    importedBanks = ui->fPresetManager->getImportedBankNames();
                    importedBankPresets.clear();
                    for (const auto& bank : importedBanks)
                        importedBankPresets[bank.buffer()] = ui->fPresetManager->getPresetsInBank(bank.buffer());
                    _shouldRefreshBankList = false;
                }
                for (const auto& bankName : importedBanks) {
                    // Get presets from cache
                    const std::vector<String>& presets = importedBankPresets[bankName.buffer()];
                    
                    // Build menu label with preset count
                    char labelBuf[256];
                    std::snprintf(labelBuf, sizeof(labelBuf), "%s (%d)", 
                                 bankName.buffer(), (int)presets.size());
                    
                    if (ImGui::BeginMenu(labelBuf)) {
                        if (presets.empty()) {
                            ImGui::MenuItem("(No presets)", nullptr, false, false);
                        } else {
                            for (size_t i = 0; i < presets.size(); i++) {
                                // Build unique label for each preset
                                char presetLabel[256];
                                std::snprintf(presetLabel, sizeof(presetLabel), "%s##%s%d",
                                            presets[i].buffer(), bankName.buffer(), (int)i);
                                
                                if (ImGui::MenuItem(presetLabel)) {
                                    if (ui->fPresetManager->loadPresetFromBank(bankName.buffer(), presets[i].buffer()))
                                    {
                                        ui->_updateState(presets[i].buffer(), bankName.buffer(), false);
                                    }
                                }
                            }
                        }

                        ImGui::Separator();

                        // Save current preset to this bank
                        if (ImGui::MenuItem("Save Current Preset Here...")) {
                            strncpy(_presetNameEditorBuffer, ui->fCurrentPresetName.buffer(), MAX_PRESET_NAME_LENGTH);
                            _presetNameEditorBuffer[MAX_PRESET_NAME_LENGTH - 1] = '\0';
                            _pendingSaveBankName = bankName.buffer();
                            requestSavePresetPopup = true;
                        }

                        // Overwrite current preset in this bank (only enabled when the current preset belongs here)
                        const bool isCurrentInThisBank = (ui->fCurrentPresetBank == bankName);
                        if (ImGui::MenuItem("Overwrite Current", nullptr, false, isCurrentInThisBank))
                        {
                            SynthProgram snap = ui->fPresetManager->captureCurrentParameters();
                            ui->fPresetManager->savePresetToBank(bankName.buffer(), ui->fCurrentPresetName.buffer(), snap);
                            ui->_updateState(ui->fCurrentPresetName.buffer(), bankName.buffer(), false);
                            ui->logAndShowMessage("Preset '%s' overwritten in bank '%s'.",
                                                  ui->fCurrentPresetName.buffer(), bankName.buffer());
                        }

                        ImGui::Separator();

                        // Export this bank to file
                        if (ImGui::MenuItem("Export Bank...")) {
                            _fileBrowserBankName = bankName.buffer();
                            char defaultFilename[256];
                            std::snprintf(defaultFilename, sizeof(defaultFilename), "%s%s",
                                         bankName.buffer(), USER_PRESET_BANK_EXTENSION);
                            DGL_NAMESPACE::FileBrowserOptions opts;
                            opts.saving = true;
                            opts.title = "Export Bank";
                            opts.defaultName = defaultFilename;
                            _fileBrowserHandle = DGL_NAMESPACE::fileBrowserCreate(false, getWindow().getNativeWindowHandle(),
                                                                   getScaleFactor(), opts);
                            _fileBrowserAction = kFileBrowserExportBank;
                        }

                        // Rename this bank
                        if (ImGui::MenuItem("Rename Bank...")) {
                            strncpy(_bankNameEditorBuffer, bankName.buffer(), MAX_PRESET_NAME_LENGTH);
                            _bankNameEditorBuffer[MAX_PRESET_NAME_LENGTH - 1] = '\0';
                            _fileBrowserBankName = bankName.buffer();
                            requestRenameBankPopup = true;
                        }

                        // Delete this bank
                        if (ImGui::MenuItem("Delete Bank...")) {
                            _fileBrowserBankName = bankName.buffer();
                            requestDeleteBankPopup = true;
                        }
                        
                        ImGui::EndMenu();
                    }
                }


                // Bank Management Section
                ImGui::Separator();
                
                if (ImGui::MenuItem("New Bank...")) {
                    memset(_bankNameEditorBuffer, '\0', MAX_PRESET_NAME_LENGTH);
                    requestNewBankPopup = true;
                }

                if (ImGui::MenuItem("Import Bank...")) {
                    DGL_NAMESPACE::FileBrowserOptions opts;
                    opts.saving = false;
                    opts.title = "Import Preset Bank";
                    
                    _fileBrowserHandle = DGL_NAMESPACE::fileBrowserCreate(false, getWindow().getNativeWindowHandle(), 
                                                           getScaleFactor(), opts);
                    _fileBrowserAction = kFileBrowserImportBank;
                }
            }

                // TODO: Add the following entries:
                // - Load user preset bank from file
                // - Save user preset bank to file
                // - Manage presets in user bank (e.g. delete, rename, reorder, etc.)
                // Preset bank uses JSON format.

            const String sectionLabel = String("Preset: ") + ui->fCurrentPresetName + String(ui->fPresetIsModified ? "*" : "");
            ImGui::SeparatorText(sectionLabel.buffer());
            {
                // Save to Default Bank (quick save)
                if (ImGui::MenuItem("Save to Default Bank"))
                {
                    // Check if preset with same name already exists
                    bool exists = false;
                    for (size_t i = 0; i < ui->fPresetManager->getDefaultBankPresetCount(); i++) {
                        if (ui->fPresetManager->getDefaultBankPresetName(i) == ui->fCurrentPresetName) {
                            exists = true;
                            break;
                        }
                    }

                    if (exists) {
                        // Ask user for confirmation before overwriting
                        strncpy(_pendingSavePresetName, ui->fCurrentPresetName.buffer(), MAX_PRESET_NAME_LENGTH);
                        _pendingSavePresetName[MAX_PRESET_NAME_LENGTH - 1] = '\0';
                        _pendingSaveBankName = DEFAULT_USER_BANK_NAME;
                        requestConfirmOverwritePresetPopup = true;
                    } else {
                        SynthProgram snapshot = ui->fPresetManager->captureCurrentParameters();
                        const char* presetName = ui->fCurrentPresetName.buffer();
                        ui->fPresetManager->savePresetToDefaultBank(presetName, snapshot);
                        ui->_updateState(presetName, DEFAULT_USER_BANK_NAME, false);
                    }
                }
                
                if (ImGui::MenuItem("Save As..."))
                {
                    strncpy(_presetNameEditorBuffer, ui->fCurrentPresetName.buffer(), MAX_PRESET_NAME_LENGTH);
                    _presetNameEditorBuffer[MAX_PRESET_NAME_LENGTH - 1] = '\0';
                    _pendingSaveBankName = DEFAULT_USER_BANK_NAME;
                    requestSavePresetPopup = true;
                }

                ImGui::Separator();
                
                // Check if current preset is from User Banks (Non-factory presets, and not the single imported preset)
                const bool isUserPreset = (ui->fCurrentPresetBank != FACTORY_BANK_NAME) && (ui->fCurrentPresetBank != BANK_NAME_FOR_SINGLE_IMPORTED_PRESET);
                
                if (ImGui::MenuItem("Rename Preset...", nullptr, false, isUserPreset))
                {
                    strncpy(_presetNameEditorBuffer, ui->fCurrentPresetName.buffer(), MAX_PRESET_NAME_LENGTH);
                    _presetNameEditorBuffer[MAX_PRESET_NAME_LENGTH - 1] = '\0';

                    requestRenamePresetPopup = true;
                }
                
                if (ImGui::MenuItem("Delete Preset...", nullptr, false, isUserPreset))
                {
                    requestDeletePresetPopup = true;
                }
                
                ImGui::Separator();
                
                // Import/Export
                if (ImGui::MenuItem("Export Preset to File..."))
                {
                    DGL_NAMESPACE::FileBrowserOptions opts;
                    opts.saving = true;
                    opts.title = "Export Preset";
                    opts.defaultName = (ui->fCurrentPresetName + String(USER_PRESET_FILE_EXTENSION)).buffer();
                    
                    _fileBrowserHandle = DGL_NAMESPACE::fileBrowserCreate(false, getWindow().getNativeWindowHandle(), 
                                                           getScaleFactor(), opts);
                    _fileBrowserAction = kFileBrowserExportPreset;
                }
                
                if (ImGui::MenuItem("Import Preset from File..."))
                {
                    DGL_NAMESPACE::FileBrowserOptions opts;
                    opts.saving = false;
                    opts.title = "Import Preset";
                    
                    _fileBrowserHandle = DGL_NAMESPACE::fileBrowserCreate(false, getWindow().getNativeWindowHandle(), 
                                                           getScaleFactor(), opts);
                    _fileBrowserAction = kFileBrowserImportPreset;
                }
            }

            ImGui::EndPopup();
        }

        ImGui::End();
    }

    //
    // Create modal popups
    // [NOTICE] Similar to menu popups above, ImGui::BeginModal() MUST be put after ImGui::OpenPopup(), otherwise popup won't show!
    //

    // Always center this window when appearing
    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Save Preset", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        // Bank selector combo
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Target bank:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(160.0f);
        if (ImGui::BeginCombo("##SaveBankSelect", _pendingSaveBankName.c_str()))
        {
            // Default user bank
            const bool isSelected = (_pendingSaveBankName == DEFAULT_USER_BANK_NAME);
            if (ImGui::Selectable(DEFAULT_USER_BANK_NAME, isSelected))
                _pendingSaveBankName = DEFAULT_USER_BANK_NAME;
            if (isSelected) ImGui::SetItemDefaultFocus();
            // Imported banks
            for (const auto& bk : importedBanks)
            {
                const bool isBkSelected = (_pendingSaveBankName == bk.buffer());
                if (ImGui::Selectable(bk.buffer(), isBkSelected))
                    _pendingSaveBankName = bk.buffer();
                if (isBkSelected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::Dummy(ImVec2(0, 2));

        ImGui::SetNextItemWidth(250.0f);
        ImGui::InputText("##PresetNameSaveInput", _presetNameEditorBuffer, MAX_PRESET_NAME_LENGTH);

        // Check if preset already exists in the target bank
        bool presetAlreadyExists = false;
        if (_pendingSaveBankName == DEFAULT_USER_BANK_NAME) {
            for (size_t i = 0; i < ui->fPresetManager->getDefaultBankPresetCount(); i++) {
                if (ui->fPresetManager->getDefaultBankPresetName(i) == String(_presetNameEditorBuffer)) {
                    presetAlreadyExists = true;
                    break;
                }
            }
        } else {
            // Use cached preset list to avoid disk reads every frame
            auto it = importedBankPresets.find(_pendingSaveBankName);
            if (it != importedBankPresets.end()) {
                for (const auto& p : it->second) {
                    if (p == String(_presetNameEditorBuffer)) {
                        presetAlreadyExists = true;
                        break;
                    }
                }
            }
        }

        if (presetAlreadyExists) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.75f, 0.0f, 1.0f));
            ImGui::TextWrapped("A preset with this name already exists. Saving will overwrite it.");
            ImGui::PopStyleColor();
        }

        ImGui::Dummy(ImVec2(0, 4));
        ImGui::Separator();

        const bool presetNameIsEmpty = (_presetNameEditorBuffer[0] == '\0');
        if (presetNameIsEmpty)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
            ImGui::TextWrapped("Preset name cannot be empty.");
            ImGui::PopStyleColor();
        }

        const char* saveButtonLabel = presetAlreadyExists ? "Overwrite" : "Save";
        ImGui::BeginDisabled(presetNameIsEmpty);
        if (ImGui::Button(saveButtonLabel, ImVec2(120, 0)))
        {
            // Capture current parameters
            SynthProgram snapshot = ui->fPresetManager->captureCurrentParameters();
            
            // Save to the target bank
            if (_pendingSaveBankName == DEFAULT_USER_BANK_NAME) {
                ui->fPresetManager->savePresetToDefaultBank(_presetNameEditorBuffer, snapshot);
                ui->_updateState(_presetNameEditorBuffer, DEFAULT_USER_BANK_NAME, false);
            } else {
                ui->fPresetManager->savePresetToBank(_pendingSaveBankName.c_str(), _presetNameEditorBuffer, snapshot);
                ui->_updateState(_presetNameEditorBuffer, _pendingSaveBankName.c_str(), false);
                _shouldRefreshBankList = true; // Preset list of this bank changed
            }

            ImGui::CloseCurrentPopup();
            requestSavePresetPopup = false;
        }
        ImGui::EndDisabled();
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
            requestSavePresetPopup = false;
        }

        ImGui::EndPopup();
    }

    // Always center this window when appearing
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Rename Preset", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        const bool isNonRenameablePreset = (ui->fCurrentPresetBank == FACTORY_BANK_NAME) || (ui->fCurrentPresetBank == BANK_NAME_FOR_SINGLE_IMPORTED_PRESET);

        if (isNonRenameablePreset)    // In case the menu entry is enabled by mistake, show a warning message and do not allow renaming
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.0f, 1.0f));
            ImGui::TextWrapped("This is a Factory preset or a single imported preset, which cannot be renamed.");
            ImGui::TextWrapped("Use 'Save As...' to save it to User Bank with a new name.");
            ImGui::PopStyleColor();

            ImGui::Dummy(ImVec2(0, 8));

            if (ImGui::Button("OK", ImVec2(240, 0)))
            {
                ImGui::CloseCurrentPopup();
                requestRenamePresetPopup = false;
            }
        }
        else
        {
            // User preset (Default Bank or imported bank) - allow renaming
            ImGui::Text("Rename preset in '%s':", ui->fCurrentPresetBank.buffer());
            ImGui::Dummy(ImVec2(0, 4));

            ImGui::SetNextItemWidth(250.0f);
            ImGui::InputText("##PresetNameInput", _presetNameEditorBuffer, MAX_PRESET_NAME_LENGTH);

            ImGui::Dummy(ImVec2(0, 4));
            ImGui::Separator();

            const bool renamePresetDisabled = (_presetNameEditorBuffer[0] == '\0') ||
                                              (ui->fCurrentPresetName == String(_presetNameEditorBuffer));
            if (renamePresetDisabled)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
                ImGui::TextWrapped(_presetNameEditorBuffer[0] == '\0' ? "Preset name cannot be empty." : "New name is the same as the current name.");
                ImGui::PopStyleColor();
            }

            ImGui::BeginDisabled(renamePresetDisabled);
            if (ImGui::Button("Rename", ImVec2(120, 0)))
            {
                String oldName = ui->fCurrentPresetName;
                String newName = String(_presetNameEditorBuffer);
                String bankName = ui->fCurrentPresetBank;

                {
                    if (ui->fPresetManager->renamePresetInBank(bankName.buffer(), oldName.buffer(), newName.buffer()))
                    {
                        ui->_updateState(newName.buffer(), bankName.buffer(), false);
                        _shouldRefreshBankList = true;

                        ui->logAndShowMessage("Successfully renamed preset from '%s' to '%s'", oldName.buffer(), newName.buffer());
                    }
                    else
                    {
                        ui->logAndShowMessage("Failed to rename preset from '%s' to '%s'", oldName.buffer(), newName.buffer());
                    }
                }

                ImGui::CloseCurrentPopup();
                requestRenamePresetPopup = false;
            }
            ImGui::EndDisabled();
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
                requestRenamePresetPopup = false;
            }
        }

        ImGui::EndPopup();
    }
    
    // Always center this window when appearing
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    
    if (ImGui::BeginPopupModal("Delete Preset", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Delete preset from '%s'", ui->fCurrentPresetBank.buffer());
        ImGui::Dummy(ImVec2(0, 8));

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
        ImGui::TextWrapped("Are you sure you want to delete preset '%s'?", ui->fCurrentPresetName.buffer());
        ImGui::PopStyleColor();

        ImGui::Dummy(ImVec2(0, 4));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        ImGui::TextWrapped("This action cannot be undone.");
        ImGui::PopStyleColor();

        ImGui::Separator();

        if (ImGui::Button("Delete", ImVec2(120, 0)))
        {
            String presetName = ui->fCurrentPresetName;
            String bankName = ui->fCurrentPresetBank;

            if (ui->fPresetManager->deletePresetFromBank(bankName.buffer(), presetName.buffer()))
            {
                ui->logAndShowMessage("Successfully deleted preset '%s' from bank '%s'", presetName.buffer(), bankName.buffer());
                _shouldRefreshBankList = true;

                // Reset to default state
                ui->fPresetManager->loadDefaultProgram();
                ui->_updateState(DEFAULT_PRESET_NAME, FACTORY_BANK_NAME, false);
            }
            else
            {
                ui->logAndShowMessage("Failed to delete preset '%s' from bank '%s'", presetName.buffer(), bankName.buffer());
            }

            ImGui::CloseCurrentPopup();
            requestDeletePresetPopup = false;
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
            requestDeletePresetPopup = false;
        }

        ImGui::EndPopup();
    }

    // Always center this window when appearing
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Overwrite Preset?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Dummy(ImVec2(0, 4));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.75f, 0.0f, 1.0f));
        ImGui::TextWrapped("Preset '%s' already exists in bank '%s'.", _pendingSavePresetName, _pendingSaveBankName.c_str());
        ImGui::TextWrapped("Do you want to overwrite it?");
        ImGui::PopStyleColor();
        ImGui::Dummy(ImVec2(0, 4));

        ImGui::Separator();

        if (ImGui::Button("Overwrite", ImVec2(120, 0)))
        {
            SynthProgram snapshot = ui->fPresetManager->captureCurrentParameters();

            if (_pendingSaveBankName == DEFAULT_USER_BANK_NAME) {
                ui->fPresetManager->savePresetToDefaultBank(_pendingSavePresetName, snapshot);
                ui->_updateState(_pendingSavePresetName, DEFAULT_USER_BANK_NAME, false);
            } else {
                ui->fPresetManager->savePresetToBank(_pendingSaveBankName.c_str(), _pendingSavePresetName, snapshot);
                ui->_updateState(_pendingSavePresetName, _pendingSaveBankName.c_str(), false);
                _shouldRefreshBankList = true; // Preset list of this bank changed
            }

            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    // Always center this window when appearing
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("New Bank", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Create a new preset bank:");
        ImGui::Dummy(ImVec2(0, 4));

        ImGui::SetNextItemWidth(250.0f);
        ImGui::InputText("##NewBankNameInput", _bankNameEditorBuffer, MAX_PRESET_NAME_LENGTH);

        ImGui::Dummy(ImVec2(0, 4));
        ImGui::Separator();

        const bool bankNameIsEmpty = (_bankNameEditorBuffer[0] == '\0');
        ImGui::BeginDisabled(bankNameIsEmpty);
        if (ImGui::Button("Create", ImVec2(120, 0)))
        {
            String newBankName(_bankNameEditorBuffer);
            if (ui->fPresetManager->createNewBank(newBankName.buffer()))
            {
                ui->logAndShowMessage("Bank '%s' created successfully.", newBankName.buffer());
                _shouldRefreshBankList = true;
            }
            else
            {
                ui->logAndShowMessage("Failed to create bank '%s'. It may already exist.", newBankName.buffer());
            }
            ImGui::CloseCurrentPopup();
            requestNewBankPopup = false;
        }
        ImGui::EndDisabled();
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
            requestNewBankPopup = false;
        }

        ImGui::EndPopup();
    }

    // Always center this window when appearing
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Rename Bank", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        char renameBankInfoBuf[256];
        std::snprintf(renameBankInfoBuf, sizeof(renameBankInfoBuf), "Rename bank '%s':", _fileBrowserBankName.c_str());
        ImGui::Text("%s", renameBankInfoBuf);
        ImGui::Dummy(ImVec2(0, 4));

        ImGui::SetNextItemWidth(250.0f);
        ImGui::InputText("##RenameBankInput", _bankNameEditorBuffer, MAX_PRESET_NAME_LENGTH);

        ImGui::Dummy(ImVec2(0, 4));
        ImGui::Separator();

        const bool renameBankDisabled = (_bankNameEditorBuffer[0] == '\0') ||
                                        (std::strcmp(_bankNameEditorBuffer, _fileBrowserBankName.c_str()) == 0);
        if (renameBankDisabled)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
            ImGui::TextWrapped(_bankNameEditorBuffer[0] == '\0' ? "Bank name cannot be empty." : "New name is the same as the current name.");
            ImGui::PopStyleColor();
        }

        ImGui::BeginDisabled(renameBankDisabled);
        if (ImGui::Button("Rename", ImVec2(120, 0)))
        {
            String oldName(_fileBrowserBankName.c_str());
            String newName(_bankNameEditorBuffer);

            {
                if (ui->fPresetManager->renameBankByName(oldName.buffer(), newName.buffer()))
                {
                    // If current preset was from the renamed bank, update the bank reference
                    if (ui->fCurrentPresetBank == oldName)
                        ui->_updateState(ui->fCurrentPresetName.buffer(), newName.buffer(), ui->fPresetIsModified);

                    ui->logAndShowMessage("Bank renamed from '%s' to '%s'.", oldName.buffer(), newName.buffer());
                    _shouldRefreshBankList = true;
                }
                else
                {
                    ui->logAndShowMessage("Failed to rename bank '%s'. Target name may already exist.", oldName.buffer());
                }
            }
            ImGui::CloseCurrentPopup();
            requestRenameBankPopup = false;
        }
        ImGui::EndDisabled();
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
            requestRenameBankPopup = false;
        }

        ImGui::EndPopup();
    }

    // Always center this window when appearing
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Delete Bank", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Dummy(ImVec2(0, 4));

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
        char deleteBankInfoBuf[256];
        std::snprintf(deleteBankInfoBuf, sizeof(deleteBankInfoBuf), "Are you sure you want to delete bank '%s'?", _fileBrowserBankName.c_str());
        ImGui::TextWrapped("%s", deleteBankInfoBuf);
        ImGui::PopStyleColor();

        ImGui::Dummy(ImVec2(0, 4));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        ImGui::TextWrapped("All presets in this bank will be deleted. This action cannot be undone.");
        ImGui::PopStyleColor();

        ImGui::Dummy(ImVec2(0, 4));
        ImGui::Separator();

        if (ImGui::Button("Delete", ImVec2(120, 0)))
        {
            String bankToDelete(_fileBrowserBankName.c_str());

            if (ui->fPresetManager->deleteBankByName(bankToDelete.buffer()))
            {
                // If current preset was from the deleted bank, reset to default
                if (ui->fCurrentPresetBank == bankToDelete)
                {
                    ui->fPresetManager->loadDefaultProgram();
                    ui->_updateState(DEFAULT_PRESET_NAME, FACTORY_BANK_NAME, false);
                }
                ui->logAndShowMessage("Bank '%s' deleted successfully.", bankToDelete.buffer());
                _shouldRefreshBankList = true;
            }
            else
            {
                ui->logAndShowMessage("Failed to delete bank '%s'.", bankToDelete.buffer());
            }

            ImGui::CloseCurrentPopup();
            requestDeleteBankPopup = false;
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
            requestDeleteBankPopup = false;
        }

        ImGui::EndPopup();
    }

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
