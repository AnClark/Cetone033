#pragma once

#include "extra/String.hpp" // For DPF String class
#include "structures.h"

#include <cstdio>
#include <cstdlib>
#include <vector>

using FactoryProgram = SynthProgramOld;
using UserProgram = SynthProgram;

// Forward declaration to avoid circular dependency
class CCetoneUI;

struct PresetBank {
    String                   Name;
    std::vector<UserProgram> Presets;
};

class CPresetManager {
public:
    CPresetManager(CCetoneUI* ui)
        : ui(ui)
    {
        initFactoryPrograms(); // Remember to load factory preset data into memory first
        loadDefaultBank(); // Load user presets from disk
    }
    ~CPresetManager() { }

    // -------------------------------------------------------------------
    // Operations on programs (aka. presets)

    void loadProgram(const FactoryProgram& program);
    void loadProgram(const UserProgram& program); // Overload for new format with FilterType and MaxPolyphony support

    // -------------------------------------------------------------------
    // Operations on factory banks

protected:
    void   initFactoryPrograms();
public:
    String getFactoryProgramName(uint32_t index) const;
    void   loadFactoryProgram(uint32_t index);
    void   loadDefaultProgram();

    // -------------------------------------------------------------------
    // JSON Serialization for User Preset Banks

    String serializeBankToJSON(const PresetBank& bank) const;
    bool   deserializeBankFromJSON(const String& jsonString, PresetBank& outBank) const;

    // Single Preset Import/Export
    String serializePresetToJSON(const SynthProgram& preset) const;
    bool   deserializePresetFromJSON(const String& jsonString, SynthProgram& outPreset) const;
    bool   exportCurrentPresetToFile(const char* filePath);
    bool   importPresetFromFile(const char* filePath, String* outPresetName = nullptr);

    // -------------------------------------------------------------------
    // Default User Preset Bank Management
    // NOTE: These methods operate on individual presets within the default User Bank.
    //       For Bank-level operations (import/export/delete entire banks), see "Bank Management" section.

    bool   loadDefaultBank();
    bool   saveDefaultBank();
    void   savePresetToDefaultBank(const char* presetName, const SynthProgram& preset);
    bool   loadPresetFromDefaultBank(const char* presetName, SynthProgram& outPreset);
    bool   deletePresetFromDefaultBank(const char* presetName);
    bool   renamePresetInDefaultBank(const char* oldName, const char* newName);
    void   renameDefaultBank(const char* newName); // DEPRECATED: Cannot rename to avoid naming conflicts
    String getDefaultBankName() const;
    size_t getDefaultBankPresetCount() const;
    String getDefaultBankPresetName(size_t index) const;
    bool   loadPresetFromDefaultBank(const char* presetName); // Load from Default User Bank and update State

    // -------------------------------------------------------------------
    // Bank Management (Multi-Bank Support)
    // NOTE: These methods provide Bank-level operations (import/export/delete entire banks)
    //       and unified cross-bank preset loading. For managing individual presets in the
    //       default User Bank, see "Default User Preset Bank Management" section.

    std::vector<String>   getImportedBankNames();                             // Get list of imported bank names (excluding Factory and Default)
    bool                  importBankFromFile(const char* filePath);           // Import bank from file to global Banks directory
    bool                  exportBankToFile(const char* bankName, const char* filePath); // Export specified bank to file
    bool                  deleteBankByName(const char* bankName);             // Delete entire bank (cannot delete default User/Factory banks)
    bool                  renameBankByName(const char* oldName, const char* newName);   // Rename bank file (cannot rename default User/Factory banks)
    bool                  loadPresetFromBank(const char* bankName, const char* presetName); // Load preset from specified bank  
    std::vector<String>   getPresetsInBank(const char* bankName);             // Get preset list from specified bank
    
    // Preset-level operations within Banks
    bool                  createNewBank(const char* bankName);                // Create new empty bank in Banks directory
    bool                  savePresetToBank(const char* bankName, const char* presetName, const SynthProgram& preset); // Save/update preset in any bank
    bool                  deletePresetFromBank(const char* bankName, const char* presetName); // Delete preset from specified bank
    bool                  renamePresetInBank(const char* bankName, const char* oldName, const char* newName); // Rename preset in specified bank
    
    // -------------------------------------------------------------------
    // Parameter Capture

    SynthProgram captureCurrentParameters() const;

private:
    CCetoneUI*                ui; // UI instance
    FactoryProgram            FactoryPrograms[128]; // Factory preset data
    PresetBank                fDefaultUserBank; // Default User Preset Bank

    // -------------------------------------------------------------------
    // Inner Helpers

    void _triggerParamUpdate(uint32_t paramId, float newValue);

    // File I/O helpers
    String _getDefaultBankPath() const;
    String _getUserPresetsDirectory() const;
    bool   _fileExists(const String& path) const;
    String _readFileContent(const String& path) const;
    bool   _writeFileContent(const String& path, const String& content) const;
    bool   _createDirectoryIfNeeded(const String& dirPath) const;
    
    // Bank management helpers
    String _getBanksDirectory() const;
    String _getBankFilePath(const char* bankName);
    bool   _loadBankFromFile(const String& filePath, PresetBank& outBank);
    bool   _saveBankToFile(const String& filePath, const PresetBank& bank);
    bool   _isDefaultUserBank(const char* bankName) const;
    bool   _isFactoryBank(const char* bankName) const;
};
