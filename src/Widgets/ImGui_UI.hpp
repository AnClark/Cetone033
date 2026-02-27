/*
 * Inspector Window for DPF
 * Copyright (C) 2022-2025 Filipe Coelho <falktx@falktx.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#pragma once

#include "DearImGui.hpp"
#include "FileBrowserDialog.hpp"

#include <map>
#include <queue>
#include <string>
#include <mutex>
#include "extra/String.hpp"

// Forward decls.
class CCetoneUI;

// Constants.
constexpr auto MAX_PRESET_NAME_LENGTH = 128;

// --------------------------------------------------------------------------------------------------------------------

class ImGuiUI : public ImGuiTopLevelWidget {
    CCetoneUI* ui; // UI instance pointer
    double     userScaling = 1.0f; // User scaling factor for UI elements

public:
    // ----------------------------------------------------------------
    // Window states

    bool isAboutWindowOpen = false; // "About" window visibility flag

    // ----------------------------------------------------------------
    // Parameter menu stuff

    // Set this variable to the parameter ID for which the parameter menu should be opened.
    // The menu will open on the next UI update and then reset this variable to 0.
    // NOTICE: Remember to reset this variable to 0 after handling the menu, otherwise the menu will remain open.
    uint16_t requestMenuId = 0;

    // Position of the parameter menu when opened. This is set together with requestMenuId to position the menu correctly.
    ImVec2 menuPos { 0, 0 };

    // ----------------------------------------------------------------
    // Message box stuff

    // Message box queue.
    // Pushing a string to this queue will trigger a message box popup with the string as its content. The queue is thread-safe.
    // Messages will be displayed in the order they were pushed, and each message will require the user to click "OK" before the next one is displayed.
    std::queue<std::string> messageBoxQueue;

protected:
    // ----------------------------------------------------------------
    // Preset Manager stuff

    // Bank management action flags. These are used to trigger the corresponding bank management actions (e.g. creating a new bank, renaming a bank, etc.)
    // when the user confirms the action in the corresponding popup.
    // Set these variables to true to trigger the corresponding preset management popup on the next UI update.
    // Remember to reset them to false after handling the popup to avoid it being triggered again.

    // Preset management popup flags
    bool requestRenamePresetPopup = false;
    bool requestSavePresetPopup = false;
    bool requestDeletePresetPopup = false;

    // Bank management popup flags
    bool requestNewBankPopup = false;
    bool requestRenameBankPopup = false;
    bool requestDeleteBankPopup = false;

    // Overwrite confirmation popup flag
    bool requestConfirmOverwritePresetPopup = false;

    // ----------------------------------------------------------------
    // File browser stuff (works together with Preset Manager)

    // Action flags to perform when file browser dialog result is received.
    // This is used to determine what to do with the selected file path (e.g. import preset, export bank, etc.)
    enum FileBrowserAction {
        kFileBrowserNone,
        kFileBrowserExportPreset,
        kFileBrowserImportPreset,
        kFileBrowserExportBank,
        kFileBrowserImportBank,
    };

public:
    ImGuiUI(TopLevelWidget* const tlw, CCetoneUI* const ui)
        : ImGuiTopLevelWidget(tlw->getWindow())
        , ui(ui)
    {
        memset(_presetNameEditorBuffer, '\0', sizeof(char) * (MAX_PRESET_NAME_LENGTH));
        memset(_bankNameEditorBuffer, '\0', sizeof(char) * (MAX_PRESET_NAME_LENGTH));
        memset(_pendingSavePresetName, '\0', sizeof(char) * (MAX_PRESET_NAME_LENGTH));
    }

protected:
    // ----------------------------------------------------------------
    // Widget callbacks

    void onImGuiDisplay() override;

private:
    // -------------------------------------------------------------------
    // Utility functions

    void _triggerParamUpdate(uint32_t paramId, float newValue); // Request a parameter update & UI sync when applying parameter changes

    // -------------------------------------------------------------------
    // Parameter menu stuff

    // Parameter ID for which modulation source was to be modified.
    // NOTE: This is used in CetoneSynth and CetoneLight, but not in Cetone033, which doesn't have modulation sources.
    uint16_t _requestedModParam = 0;

    // -------------------------------------------------------------------
    // Message box stuff

    bool       _requestMessagePopup = false; // Trigger flag to indicate that a message box popup should be displayed
    void       _handleMessageBoxIdle(); // Handle the idle state of the message box mechanism
    std::mutex _messageQueueMutex;

    // -------------------------------------------------------------------
    // Preset Manager Stuff

    // UI Elements
    void _handlePresetModalPopupRequests(); // Handle the opening of preset management modal popups based on the corresponding event flags
                                            // (e.g. requestRenamePresetPopup)
    void _buildPresetManagementMenu(); // Build the preset management menu (the one that opens when clicking the preset name in the toolbar)
    void _buildPresetManagementPopups(); // Build the preset management modal popups (e.g. rename preset popup, save preset popup, etc.)

    // Which bank to operate on in file browser / bank management popups
    std::string _fileBrowserBankName;

    // ImGui::TextInput buffers for preset name editing (renaming, saving, etc.)
    char _presetNameEditorBuffer[MAX_PRESET_NAME_LENGTH];
    char _bankNameEditorBuffer[MAX_PRESET_NAME_LENGTH];

    // Local cache of bank list for menu display (to avoid hitting filesystem every frame)
    std::vector<String>                        importedBanks; // List of imported banks, populated on demand when opening the bank menu
    std::map<std::string, std::vector<String>> importedBankPresets; // Preset list per bank, populated together with importedBanks
    bool                                       _shouldRefreshBankList = true; // Flag to indicate when bank list cache should be refreshed

    // Pending save data for overwrite confirmation
    char        _pendingSavePresetName[MAX_PRESET_NAME_LENGTH];
    std::string _pendingSaveBankName;

    // -------------------------------------------------------------------
    // File browser stuff (works together with Preset Manager)

    DGL_NAMESPACE::FileBrowserHandle _fileBrowserHandle = nullptr; // Handle for the active file browser dialog (nullptr if no dialog is open)
    FileBrowserAction                _fileBrowserAction = kFileBrowserNone; // Action flag to determine what to do
    void                             _handleFileBrowserIdle(); // Handles the idle state of the file browser
};
