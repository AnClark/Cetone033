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

class ImGuiUI : public ImGuiTopLevelWidget
{

public:
    CCetoneUI *ui;

    bool isAboutWindowOpen = false;
    uint16_t requestMenuId = 0;

    std::queue<std::string> messageBoxQueue;

    bool requestRenamePresetPopup = false;
    bool requestSavePresetPopup = false;
    bool requestDeletePresetPopup = false;

    // Bank management popup flags
    bool requestNewBankPopup = false;
    bool requestRenameBankPopup = false;
    bool requestDeleteBankPopup = false;

    // Overwrite confirmation popup flag
    bool requestConfirmOverwritePresetPopup = false;

    enum FileBrowserAction {
        kFileBrowserNone,
        kFileBrowserExportPreset,
        kFileBrowserImportPreset,
        kFileBrowserExportBank,
        kFileBrowserImportBank,
    };

    // Which bank to operate on in file browser / bank management popups
    std::string _fileBrowserBankName;

    ImVec2 menuPos{0, 0};

    double userScaling = 1.0f;

    ImGuiUI(TopLevelWidget* const tlw, CCetoneUI* const ui) : 
        ImGuiTopLevelWidget(tlw->getWindow()),
        ui(ui)
        {
            memset(_presetNameEditorBuffer, '\0', sizeof(char) * (MAX_PRESET_NAME_LENGTH));
            memset(_bankNameEditorBuffer, '\0', sizeof(char) * (MAX_PRESET_NAME_LENGTH));
            memset(_pendingSavePresetName, '\0', sizeof(char) * (MAX_PRESET_NAME_LENGTH));
        }

protected:
    void onImGuiDisplay() override;

private:
    void _triggerParamUpdate(uint32_t paramId, float newValue);

    char _presetNameEditorBuffer[MAX_PRESET_NAME_LENGTH];
    char _bankNameEditorBuffer[MAX_PRESET_NAME_LENGTH];

    // Pending save data for overwrite confirmation
    char _pendingSavePresetName[MAX_PRESET_NAME_LENGTH];
    std::string _pendingSaveBankName;

    // File browser stuff
    DGL_NAMESPACE::FileBrowserHandle _fileBrowserHandle = nullptr; // Handle for the active file browser dialog (nullptr if no dialog is open)
    FileBrowserAction                _fileBrowserAction = kFileBrowserNone; // Action flag to determine what to do
    void                             _handleFileBrowserIdle(); // Handles the idle state of the file browser

    uint16_t _requestedModParam = 0;

    // Message box stuff
    bool _requestMessagePopup = false;
    void _handleMessageBoxIdle();   // Handle the idle state of the message box mechanism
    std::mutex _messageQueueMutex;

    // Local cache of bank list for menu display (to avoid hitting filesystem every frame)
    std::vector<String>                              importedBanks;
    std::map<std::string, std::vector<String>>       importedBankPresets; // Preset list per bank, populated together with importedBanks
    bool _shouldRefreshBankList = true; // Flag to indicate when bank list cache should be refreshed
};
