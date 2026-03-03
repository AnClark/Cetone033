#include "PresetManager.h"

#include "3rdparty/json.hpp"
#include "CetoneUI.hpp"  // For class CCetoneUI
#include "defines.h"     // For constants like WAVE_MAX, FILTER_TYPE_BIQUAD


#ifdef DISTRHO_OS_WINDOWS
#include <shlobj.h>
#include <windows.h>

#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#endif

using json = nlohmann::json;

extern unsigned char PresetData[];

// Internal conversion functions (UI-side, no dependency on CCetone033)
namespace {

// Convert bool to parameter value [0.0, 1.0]
inline float bool2val(bool value) { return value ? 1.0f : 0.0f; }

// Convert coarse pitch [-24, 24] to parameter value [0.0, 1.0]
inline float coarse2val(int value) {
    return static_cast<float>(value + 50) / 100.0f;
}

// Convert fine pitch [-50, 50] to parameter value [0.0, 1.0]
inline float fine2val(int value) {
    return static_cast<float>(value + 100) / 200.0f;
}

// Convert integer enum [0, max] to parameter value [0.0, 1.0]
inline float int2val(int value, int max) {
    return static_cast<float>(value) / static_cast<float>(max + 1);
}

// Clamp float to range [min, max]
inline float clampf(float value, float min, float max) {
    return (value < min) ? min : ((value > max) ? max : value);
}

// Clamp int to range [min, max]
inline int clampi(int value, int min, int max) {
    return (value < min) ? min : ((value > max) ? max : value);
}

}  // anonymous namespace

// Default program data (refered to CCetone033::InitParameters())
static const UserProgram DefaultProgram = {
    DEFAULT_PRESET_NAME,  // Name

    {0, -12},  // Coarse (Osc1: 0 semitones, Osc2: -12 semitones / -1 octave)
    {0, 0},    // Fine
    {WAVE_SAW, WAVE_PULSE},  // Wave (Osc1: Saw, Osc2: Pulse)
    {0.5f, 0.5f},            // Morph
    {1.0f, 1.0f},            // Volume (internal value, 0-2 range)

    {0.02f, 0.02f},  // Attack
    {0.40f, 0.15f},  // Decay

    0.2f,  // ModEnv (internal value, with -0.5 offset applied)
    0.0f,  // ModVel
    0.0f,  // ModRes

    1.0f,  // Cutoff
    0.0f,  // Resonance
    FILTER_TYPE_BIQUAD,

    false,  // GlideState
    0.01f,  // GlideSpeed

    false,  // ClipState
    1.0f,   // MainVolume (internal value, 0-2 range)

#ifdef ENABLE_POLYPHONY
	16, // MaxPolyphony
#endif
};

void CPresetManager::initFactoryPrograms() {
    // Load factory preset data from binary array `PresetData` into
    // `FactoryPrograms`.
    memcpy(this->FactoryPrograms, PresetData, sizeof(FactoryProgram) * 128);
}

String CPresetManager::getFactoryProgramName(uint32_t index) const {
    DISTRHO_SAFE_ASSERT_RETURN(index < 128, String())

    // Return the name of the factory program at the specified index.
    return String(this->FactoryPrograms[index].Name);
}

void CPresetManager::loadFactoryProgram(uint32_t index) {
    DISTRHO_SAFE_ASSERT_RETURN(index < 128, )

    // Load the selected factory program into the UI.
    const auto& selectedProgram = this->FactoryPrograms[index];
    this->loadProgram(selectedProgram);
}

void CPresetManager::loadProgram(const FactoryProgram& program) {
    // Oscillator 1 parameters
    _triggerParamUpdate(pOsc1Coarse,
                        coarse2val(clampi(program.Coarse[0], -24, 24)));
    _triggerParamUpdate(pOsc1Fine, fine2val(clampi(program.Fine[0], -50, 50)));
    _triggerParamUpdate(
        pOsc1Wave, int2val(clampi(program.Wave[0], 0, WAVE_MAX), WAVE_MAX));
    _triggerParamUpdate(pOsc1Morph, clampf(program.Morph[0], 0.0f, 1.0f));
#if defined(ENABLE_POLYPHONY) && defined(ENABLE_VOLUME_BOOSTING)
    _triggerParamUpdate(pOsc1Volume,
                        clampf(program.Volume[0], 0.0f, 10.0f) * 0.5f);
#else
    _triggerParamUpdate(pOsc1Volume,
                        clampf(program.Volume[0], 0.0f, 2.0f) * 0.5f);
#endif

    // Oscillator 2 parameters
    _triggerParamUpdate(pOsc2Coarse,
                        coarse2val(clampi(program.Coarse[1], -24, 24)));
    _triggerParamUpdate(pOsc2Fine, fine2val(clampi(program.Fine[1], -50, 50)));
    _triggerParamUpdate(
        pOsc2Wave, int2val(clampi(program.Wave[1], 0, WAVE_MAX), WAVE_MAX));
    _triggerParamUpdate(pOsc2Morph, clampf(program.Morph[1], 0.0f, 1.0f));
#if defined(ENABLE_POLYPHONY) && defined(ENABLE_VOLUME_BOOSTING)
    _triggerParamUpdate(pOsc2Volume,
                        clampf(program.Volume[1], 0.0f, 10.0f) * 0.5f);
#else
    _triggerParamUpdate(pOsc2Volume,
                        clampf(program.Volume[1], 0.0f, 2.0f) * 0.5f);
#endif

    // Envelope 1 parameters
    _triggerParamUpdate(pEnv1Attack, clampf(program.Attack[0], 0.0f, 1.0f));
    _triggerParamUpdate(pEnv1Decay, clampf(program.Decay[0], 0.0f, 1.0f));

    // Envelope 2 parameters
    _triggerParamUpdate(pEnv2Attack, clampf(program.Attack[1], 0.0f, 1.0f));
    _triggerParamUpdate(pEnv2Decay, clampf(program.Decay[1], 0.0f, 1.0f));

    // Modulation parameters
    _triggerParamUpdate(pModEnv, clampf(program.ModEnv + 0.5f, 0.0f, 1.0f));
    _triggerParamUpdate(pModVel, clampf(program.ModVel, 0.0f, 1.0f));
    _triggerParamUpdate(pModRes, clampf(program.ModRes, 0.0f, 1.0f));

    // Filter parameters
    _triggerParamUpdate(pCutoff, clampf(program.Cutoff, 0.0f, 1.0f));
    _triggerParamUpdate(pResonance, clampf(program.Resonance, 0.0f, 1.0f));

    // Note: SynthProgramOld doesn't have FilterType, use default Biquad (0)
    _triggerParamUpdate(pFilterType,
                        int2val(FILTER_TYPE_BIQUAD, FILTER_TYPE_MAX));

    // Glide parameters
    _triggerParamUpdate(pGlideState, bool2val(program.GlideState));
    _triggerParamUpdate(pGlideSpeed, clampf(program.GlideSpeed, 0.0f, 1.0f));

    // Global parameters
    _triggerParamUpdate(pClipState, bool2val(program.ClipState));
#if defined(ENABLE_POLYPHONY) && defined(ENABLE_VOLUME_BOOSTING)
    _triggerParamUpdate(pVolume,
                        clampf(program.MainVolume, 0.0f, 10.0f) * 0.5f);
#else
    _triggerParamUpdate(pVolume, clampf(program.MainVolume, 0.0f, 2.0f) * 0.5f);
#endif
}

void CPresetManager::loadProgram(const UserProgram& program) {
    // Oscillator 1 parameters
    _triggerParamUpdate(pOsc1Coarse,
                        coarse2val(clampi(program.Coarse[0], -24, 24)));
    _triggerParamUpdate(pOsc1Fine, fine2val(clampi(program.Fine[0], -50, 50)));
    _triggerParamUpdate(
        pOsc1Wave, int2val(clampi(program.Wave[0], 0, WAVE_MAX), WAVE_MAX));
    _triggerParamUpdate(pOsc1Morph, clampf(program.Morph[0], 0.0f, 1.0f));
#if defined(ENABLE_POLYPHONY) && defined(ENABLE_VOLUME_BOOSTING)
    _triggerParamUpdate(pOsc1Volume,
                        clampf(program.Volume[0], 0.0f, 10.0f) * 0.5f);
#else
    _triggerParamUpdate(pOsc1Volume,
                        clampf(program.Volume[0], 0.0f, 2.0f) * 0.5f);
#endif

    // Oscillator 2 parameters
    _triggerParamUpdate(pOsc2Coarse,
                        coarse2val(clampi(program.Coarse[1], -24, 24)));
    _triggerParamUpdate(pOsc2Fine, fine2val(clampi(program.Fine[1], -50, 50)));
    _triggerParamUpdate(
        pOsc2Wave, int2val(clampi(program.Wave[1], 0, WAVE_MAX), WAVE_MAX));
    _triggerParamUpdate(pOsc2Morph, clampf(program.Morph[1], 0.0f, 1.0f));
#if defined(ENABLE_POLYPHONY) && defined(ENABLE_VOLUME_BOOSTING)
    _triggerParamUpdate(pOsc2Volume,
                        clampf(program.Volume[1], 0.0f, 10.0f) * 0.5f);
#else
    _triggerParamUpdate(pOsc2Volume,
                        clampf(program.Volume[1], 0.0f, 2.0f) * 0.5f);
#endif

    // Envelope 1 parameters
    _triggerParamUpdate(pEnv1Attack, clampf(program.Attack[0], 0.0f, 1.0f));
    _triggerParamUpdate(pEnv1Decay, clampf(program.Decay[0], 0.0f, 1.0f));

    // Envelope 2 parameters
    _triggerParamUpdate(pEnv2Attack, clampf(program.Attack[1], 0.0f, 1.0f));
    _triggerParamUpdate(pEnv2Decay, clampf(program.Decay[1], 0.0f, 1.0f));

    // Modulation parameters
    _triggerParamUpdate(pModEnv, clampf(program.ModEnv + 0.5f, 0.0f, 1.0f));
    _triggerParamUpdate(pModVel, clampf(program.ModVel, 0.0f, 1.0f));
    _triggerParamUpdate(pModRes, clampf(program.ModRes, 0.0f, 1.0f));

    // Filter parameters
    _triggerParamUpdate(pCutoff, clampf(program.Cutoff, 0.0f, 1.0f));
    _triggerParamUpdate(pResonance, clampf(program.Resonance, 0.0f, 1.0f));

    // UserProgram (SynthProgram) has FilterType field - this is the key
    // difference!
    _triggerParamUpdate(pFilterType,
                        int2val(clampi(program.FilterType, 0, FILTER_TYPE_MAX),
                                FILTER_TYPE_MAX));

    // Glide parameters
    _triggerParamUpdate(pGlideState, bool2val(program.GlideState));
    _triggerParamUpdate(pGlideSpeed, clampf(program.GlideSpeed, 0.0f, 1.0f));

    // Global parameters
    _triggerParamUpdate(pClipState, bool2val(program.ClipState));
#if defined(ENABLE_POLYPHONY) && defined(ENABLE_VOLUME_BOOSTING)
    _triggerParamUpdate(pVolume,
                        clampf(program.MainVolume, 0.0f, 10.0f) * 0.5f);
#else
    _triggerParamUpdate(pVolume, clampf(program.MainVolume, 0.0f, 2.0f) * 0.5f);
#endif

#ifdef ENABLE_POLYPHONY
    // MaxPolyphony is also unique to UserProgram
    if (program.MaxPolyphony > 0) {
        _triggerParamUpdate(
            pMaxPolyphony,
            static_cast<float>(clampi(program.MaxPolyphony, 1, MAX_POLYPHONY)));
    }
#endif
}

void CPresetManager::loadDefaultProgram() { this->loadProgram(DefaultProgram); }

void CPresetManager::_triggerParamUpdate(uint32_t paramId, float newValue) {
    ui->setParameterValue(paramId,
                          newValue);  // Tell the DSP to update parameter value
    ui->parameterChanged(paramId, newValue);  // Request UI refresh
}

// ============================================================================
// JSON Serialization Implementation
// ============================================================================

String CPresetManager::serializeBankToJSON(const PresetBank& bank) const {
    try {
        json j;
        j["formatVersion"] = "1.0.0";
        j["bankName"] = bank.Name.buffer();
        j["presetCount"] = bank.Presets.size();
        j["presets"] = json::array();

        for (const auto& preset : bank.Presets) {
            json p;
            p["name"] = preset.Name;

            // Oscillator 1
            p["osc1Coarse"] = preset.Coarse[0];
            p["osc1Fine"] = preset.Fine[0];
            p["osc1Wave"] = preset.Wave[0];
            p["osc1Morph"] = preset.Morph[0];
            p["osc1Volume"] = preset.Volume[0];

            // Oscillator 2
            p["osc2Coarse"] = preset.Coarse[1];
            p["osc2Fine"] = preset.Fine[1];
            p["osc2Wave"] = preset.Wave[1];
            p["osc2Morph"] = preset.Morph[1];
            p["osc2Volume"] = preset.Volume[1];

            // Envelopes
            p["env1Attack"] = preset.Attack[0];
            p["env1Decay"] = preset.Decay[0];
            p["env2Attack"] = preset.Attack[1];
            p["env2Decay"] = preset.Decay[1];

            // Modulation
            p["modEnv"] = preset.ModEnv;
            p["modVel"] = preset.ModVel;
            p["modRes"] = preset.ModRes;

            // Filter
            p["cutoff"] = preset.Cutoff;
            p["resonance"] = preset.Resonance;
            p["filterType"] = preset.FilterType;

            // Glide
            p["glideState"] = preset.GlideState;
            p["glideSpeed"] = preset.GlideSpeed;

            // Global
            p["clipState"] = preset.ClipState;
            p["mainVolume"] = preset.MainVolume;

#ifdef ENABLE_POLYPHONY
            p["maxPolyphony"] = preset.MaxPolyphony;
#endif

            j["presets"].push_back(p);
        }

        // Convert to string with indentation for readability
        return String(j.dump(2).c_str());
    } catch (const std::exception& e) {
        // TODO: Show error message on a modal window (dialog)
        d_stderr("serializeBankToJSON: Exception - %s", e.what());
        return String();
    }
}

bool CPresetManager::deserializeBankFromJSON(const String& jsonString,
                                             PresetBank& outBank) const {
    if (jsonString.isEmpty()) {
        d_stderr("deserializeBankFromJSON: Empty JSON string");
        return false;
    }

    try {
        json j = json::parse(jsonString.buffer());

        // Validate format version
        if (!j.contains("formatVersion")) {
            d_stderr("deserializeBankFromJSON: Missing formatVersion");
            return false;
        }

        std::string version = j["formatVersion"];
        if (version != "1.0.0") {
            d_stderr("deserializeBankFromJSON: Unsupported version %s",
                     version.c_str());
            // Could implement version migration here in the future
        }

        // Parse bank metadata
        if (!j.contains("bankName")) {
            d_stderr("deserializeBankFromJSON: Missing bankName");
            return false;
        }

        outBank.Name = String(j["bankName"].get<std::string>().c_str());
        outBank.Presets.clear();

        // Parse presets array
        if (!j.contains("presets") || !j["presets"].is_array()) {
            d_stderr(
                "deserializeBankFromJSON: Missing or invalid presets array");
            return false;
        }

        for (const auto& presetJson : j["presets"]) {
            SynthProgram preset(DefaultProgram); // Start with defaults in case some fields are missing

            // Name
            if (presetJson.contains("name")) {
                std::string name = presetJson["name"];
                strncpy(preset.Name, name.c_str(), 127);
                preset.Name[127] = '\0';
            } else {
                d_stderr("deserializeBankFromJSON: Preset missing name. Will specify a fallback name");
                strncpy(preset.Name, PRESET_NO_NAME_FALLBACK, 63);
                preset.Name[63] = '\0';
            }

            // Oscillator 1
            if (presetJson.contains("osc1Coarse"))
                preset.Coarse[0] = presetJson["osc1Coarse"];
            if (presetJson.contains("osc1Fine"))
                preset.Fine[0] = presetJson["osc1Fine"];
            if (presetJson.contains("osc1Wave"))
                preset.Wave[0] = presetJson["osc1Wave"];
            if (presetJson.contains("osc1Morph"))
                preset.Morph[0] = presetJson["osc1Morph"];
            if (presetJson.contains("osc1Volume"))
                preset.Volume[0] = presetJson["osc1Volume"];

            // Oscillator 2
            if (presetJson.contains("osc2Coarse"))
                preset.Coarse[1] = presetJson["osc2Coarse"];
            if (presetJson.contains("osc2Fine"))
                preset.Fine[1] = presetJson["osc2Fine"];
            if (presetJson.contains("osc2Wave"))
                preset.Wave[1] = presetJson["osc2Wave"];
            if (presetJson.contains("osc2Morph"))
                preset.Morph[1] = presetJson["osc2Morph"];
            if (presetJson.contains("osc2Volume"))
                preset.Volume[1] = presetJson["osc2Volume"];

            // Envelopes
            if (presetJson.contains("env1Attack"))
                preset.Attack[0] = presetJson["env1Attack"];
            if (presetJson.contains("env1Decay"))
                preset.Decay[0] = presetJson["env1Decay"];
            if (presetJson.contains("env2Attack"))
                preset.Attack[1] = presetJson["env2Attack"];
            if (presetJson.contains("env2Decay"))
                preset.Decay[1] = presetJson["env2Decay"];

            // Modulation
            if (presetJson.contains("modEnv"))
                preset.ModEnv = presetJson["modEnv"];
            if (presetJson.contains("modVel"))
                preset.ModVel = presetJson["modVel"];
            if (presetJson.contains("modRes"))
                preset.ModRes = presetJson["modRes"];

            // Filter
            if (presetJson.contains("cutoff"))
                preset.Cutoff = presetJson["cutoff"];
            if (presetJson.contains("resonance"))
                preset.Resonance = presetJson["resonance"];
            if (presetJson.contains("filterType"))
                preset.FilterType = presetJson["filterType"];

            // Glide
            if (presetJson.contains("glideState"))
                preset.GlideState = presetJson["glideState"];
            if (presetJson.contains("glideSpeed"))
                preset.GlideSpeed = presetJson["glideSpeed"];

            // Global
            if (presetJson.contains("clipState"))
                preset.ClipState = presetJson["clipState"];
            if (presetJson.contains("mainVolume"))
                preset.MainVolume = presetJson["mainVolume"];

#ifdef ENABLE_POLYPHONY
            if (presetJson.contains("maxPolyphony"))
                preset.MaxPolyphony = presetJson["maxPolyphony"];
#endif

            outBank.Presets.push_back(preset);
        }

        d_stderr("Successfully loaded bank '%s' with %zu presets",
                 outBank.Name.buffer(), outBank.Presets.size());

        return true;
    } catch (const json::parse_error& e) {
        d_stderr("deserializeBankFromJSON: JSON parse error - %s", e.what());
        return false;
    } catch (const std::exception& e) {
        d_stderr("deserializeBankFromJSON: Exception - %s", e.what());
        return false;
    }
}

// ============================================================================
// Single Preset Import/Export
// ============================================================================

String CPresetManager::serializePresetToJSON(const SynthProgram& preset) const {
    try {
        json j;
        j["formatVersion"] = "1.0.0";
        j["presetType"] = "singlePreset";

        // Preset data
        j["name"] = preset.Name;

        // Oscillator 1
        j["osc1Coarse"] = preset.Coarse[0];
        j["osc1Fine"] = preset.Fine[0];
        j["osc1Wave"] = preset.Wave[0];
        j["osc1Morph"] = preset.Morph[0];
        j["osc1Volume"] = preset.Volume[0];

        // Oscillator 2
        j["osc2Coarse"] = preset.Coarse[1];
        j["osc2Fine"] = preset.Fine[1];
        j["osc2Wave"] = preset.Wave[1];
        j["osc2Morph"] = preset.Morph[1];
        j["osc2Volume"] = preset.Volume[1];

        // Envelopes
        j["env1Attack"] = preset.Attack[0];
        j["env1Decay"] = preset.Decay[0];
        j["env2Attack"] = preset.Attack[1];
        j["env2Decay"] = preset.Decay[1];

        // Modulation
        j["modEnv"] = preset.ModEnv;
        j["modVel"] = preset.ModVel;
        j["modRes"] = preset.ModRes;

        // Filter
        j["cutoff"] = preset.Cutoff;
        j["resonance"] = preset.Resonance;
        j["filterType"] = preset.FilterType;

        // Glide
        j["glideState"] = preset.GlideState;
        j["glideSpeed"] = preset.GlideSpeed;

        // Global
        j["clipState"] = preset.ClipState;
        j["mainVolume"] = preset.MainVolume;

#ifdef ENABLE_POLYPHONY
        j["maxPolyphony"] = preset.MaxPolyphony;
#endif

        // Convert to string with indentation for readability
        return String(j.dump(2).c_str());
    } catch (const std::exception& e) {
        d_stderr("serializePresetToJSON: Exception - %s", e.what());
        return String();
    }
}

bool CPresetManager::deserializePresetFromJSON(const String& jsonString,
                                               SynthProgram& outPreset) const {
    if (jsonString.isEmpty()) {
        d_stderr("deserializePresetFromJSON: Empty JSON string");
        return false;
    }

    try {
        json j = json::parse(jsonString.buffer());

        // Validate format version
        if (!j.contains("formatVersion")) {
            d_stderr("deserializePresetFromJSON: Missing formatVersion");
            return false;
        }

        std::string version = j["formatVersion"];
        if (version != "1.0.0") {
            d_stderr("deserializePresetFromJSON: Unsupported version %s",
                     version.c_str());
        }

        // Initialize preset
        memset(&outPreset, 0, sizeof(SynthProgram));

        // Name
        if (j.contains("name")) {
            std::string name = j["name"];
            strncpy(outPreset.Name, name.c_str(), 127);
            outPreset.Name[127] = '\0';
        }

        // Oscillator 1
        if (j.contains("osc1Coarse")) outPreset.Coarse[0] = j["osc1Coarse"];
        if (j.contains("osc1Fine")) outPreset.Fine[0] = j["osc1Fine"];
        if (j.contains("osc1Wave")) outPreset.Wave[0] = j["osc1Wave"];
        if (j.contains("osc1Morph")) outPreset.Morph[0] = j["osc1Morph"];
        if (j.contains("osc1Volume")) outPreset.Volume[0] = j["osc1Volume"];

        // Oscillator 2
        if (j.contains("osc2Coarse")) outPreset.Coarse[1] = j["osc2Coarse"];
        if (j.contains("osc2Fine")) outPreset.Fine[1] = j["osc2Fine"];
        if (j.contains("osc2Wave")) outPreset.Wave[1] = j["osc2Wave"];
        if (j.contains("osc2Morph")) outPreset.Morph[1] = j["osc2Morph"];
        if (j.contains("osc2Volume")) outPreset.Volume[1] = j["osc2Volume"];

        // Envelopes
        if (j.contains("env1Attack")) outPreset.Attack[0] = j["env1Attack"];
        if (j.contains("env1Decay")) outPreset.Decay[0] = j["env1Decay"];
        if (j.contains("env2Attack")) outPreset.Attack[1] = j["env2Attack"];
        if (j.contains("env2Decay")) outPreset.Decay[1] = j["env2Decay"];

        // Modulation
        if (j.contains("modEnv")) outPreset.ModEnv = j["modEnv"];
        if (j.contains("modVel")) outPreset.ModVel = j["modVel"];
        if (j.contains("modRes")) outPreset.ModRes = j["modRes"];

        // Filter
        if (j.contains("cutoff")) outPreset.Cutoff = j["cutoff"];
        if (j.contains("resonance")) outPreset.Resonance = j["resonance"];
        if (j.contains("filterType")) outPreset.FilterType = j["filterType"];

        // Glide
        if (j.contains("glideState")) outPreset.GlideState = j["glideState"];
        if (j.contains("glideSpeed")) outPreset.GlideSpeed = j["glideSpeed"];

        // Global
        if (j.contains("clipState")) outPreset.ClipState = j["clipState"];
        if (j.contains("mainVolume")) outPreset.MainVolume = j["mainVolume"];

#ifdef ENABLE_POLYPHONY
        if (j.contains("maxPolyphony"))
            outPreset.MaxPolyphony = j["maxPolyphony"];
#endif

        d_stderr("Successfully loaded preset '%s'", outPreset.Name);
        return true;
    } catch (const json::parse_error& e) {
        d_stderr("deserializePresetFromJSON: JSON parse error - %s", e.what());
        return false;
    } catch (const std::exception& e) {
        d_stderr("deserializePresetFromJSON: Exception - %s", e.what());
        return false;
    }
}

bool CPresetManager::exportCurrentPresetToFile(const char* filePath) {
    if (!filePath || filePath[0] == '\0') {
        d_stderr("exportCurrentPresetToFile: Invalid file path");
        return false;
    }

    // Capture current parameters
    SynthProgram preset = captureCurrentParameters();

    // Set preset name from current preset name
    strncpy(preset.Name, ui->fCurrentPresetName.buffer(), 127);
    preset.Name[127] = '\0';

    // Serialize to JSON
    String jsonContent = serializePresetToJSON(preset);
    if (jsonContent.isEmpty()) {
        d_stderr("exportCurrentPresetToFile: Failed to serialize preset");
        return false;
    }

    // Write to file
    if (!_writeFileContent(String(filePath), jsonContent)) {
        d_stderr("exportCurrentPresetToFile: Failed to write file '%s'",
                 filePath);
        return false;
    }

    d_stderr("Successfully exported preset to '%s'", filePath);
    return true;
}

bool CPresetManager::importPresetFromFile(const char* filePath, String* outPresetName) {
    if (!filePath || filePath[0] == '\0') {
        d_stderr("importPresetFromFile: Invalid file path");
        return false;
    }

    // Read file content
    String jsonContent = _readFileContent(String(filePath));
    if (jsonContent.isEmpty()) {
        d_stderr("importPresetFromFile: Failed to read file '%s'", filePath);
        return false;
    }

    // Deserialize from JSON
    UserProgram preset;
    if (!deserializePresetFromJSON(jsonContent, preset)) {
        d_stderr("importPresetFromFile: Failed to deserialize preset");
        return false;
    }

    // Load the preset using new format loader (supports FilterType and MaxPolyphony)
    loadProgram(preset);

    if (outPresetName)
        *outPresetName = String(preset.Name);

    d_stderr("Successfully imported preset '%s' from file", preset.Name);
    return true;
}

// ============================================================================
// File I/O Helper Functions
// ============================================================================

String CPresetManager::_getUserPresetsDirectory() const {
#ifdef DISTRHO_OS_WINDOWS
    // Windows: %APPDATA%\Cetone033
    const char* appData = std::getenv("APPDATA");
    if (appData) {
        String path(appData);
        path += "\\" + String(DISTRHO_PLUGIN_NAME);
        return path;
    }
#elif defined(DISTRHO_OS_MAC)
    // macOS: ~/Library/Application Support/Cetone033
    const char* home = std::getenv("HOME");
    if (home) {
        String path(home);
        path += "/Library/Application Support/" + String(DISTRHO_PLUGIN_NAME);
        return path;
    }
#else
    // Linux: ~/.config/Cetone033
    const char* home = std::getenv("HOME");
    if (home) {
        String path(home);
        path += "/.config/" + String(DISTRHO_PLUGIN_NAME);
        return path;
    }
#endif
    return String();
}

// ============================================================================
// User Preset Bank Management
// ============================================================================

bool CPresetManager::loadDefaultBank() {
    String dirPath = _getUserPresetsDirectory();
    if (!_createDirectoryIfNeeded(dirPath)) {
        d_stderr("Failed to create user presets directory");
        // Continue anyway, initialize with empty bank
    }

    String path = _getDefaultBankPath();

    if (_fileExists(path)) {
        String jsonContent = _readFileContent(path);
        if (jsonContent.isNotEmpty()) {
            if (deserializeBankFromJSON(jsonContent, fDefaultUserBank)) {
                d_stderr("Loaded default user bank from: %s", path.buffer());
                return true;
            }
        }
    }

    // File doesn't exist or failed to load, initialize with empty bank
    d_stderr("Initializing empty default user bank");
    fDefaultUserBank.Name = String(DEFAULT_USER_BANK_NAME);
    fDefaultUserBank.Presets.clear();

    // Save the empty bank to create the file
    return saveDefaultBank();
}

bool CPresetManager::saveDefaultBank() {
    String path = _getDefaultBankPath();
    if (path.isEmpty()) {
        d_stderr("Failed to get user presets path");
        return false;
    }

    // Ensure directory exists
    String dirPath = _getUserPresetsDirectory();
    if (!_createDirectoryIfNeeded(dirPath)) {
        d_stderr("Failed to create user presets directory");
        return false;
    }

    String jsonContent = serializeBankToJSON(fDefaultUserBank);
    if (jsonContent.isEmpty()) {
        d_stderr("Failed to serialize user bank to JSON");
        return false;
    }

    if (_writeFileContent(path, jsonContent)) {
        d_stderr("Saved default user bank to: %s", path.buffer());
        return true;
    }

    return false;
}

void CPresetManager::savePresetToDefaultBank(const char* presetName,
                                             const SynthProgram& preset) {
    // Check if preset with same name already exists
    bool found = false;
    for (auto& p : fDefaultUserBank.Presets) {
        if (std::strcmp(p.Name, presetName) == 0) {
            p = preset;  // Overwrite existing preset
            std::strncpy(p.Name, presetName, 127);
            p.Name[127] = '\0';
            found = true;
            d_stderr("Updated preset '%s' in default user bank", presetName);
            break;
        }
    }

    if (!found) {
        // Add new preset
        SynthProgram newPreset = preset;
        std::strncpy(newPreset.Name, presetName, 127);
        newPreset.Name[127] = '\0';
        fDefaultUserBank.Presets.push_back(newPreset);
        d_stderr("Added preset '%s' to default user bank", presetName);
    }

    // Save to disk immediately
    saveDefaultBank();
}

bool CPresetManager::loadPresetFromDefaultBank(const char* presetName,
                                               SynthProgram& outPreset) {
    for (const auto& p : fDefaultUserBank.Presets) {
        if (std::strcmp(p.Name, presetName) == 0) {
            outPreset = p;
            return true;
        }
    }

    d_stderr("Preset '%s' not found in default user bank", presetName);
    return false;
}

bool CPresetManager::deletePresetFromDefaultBank(const char* presetName) {
    for (auto it = fDefaultUserBank.Presets.begin();
         it != fDefaultUserBank.Presets.end(); ++it) {
        if (std::strcmp(it->Name, presetName) == 0) {
            fDefaultUserBank.Presets.erase(it);
            d_stderr("Deleted preset '%s' from default user bank", presetName);
            saveDefaultBank();  // Save changes immediately
            return true;
        }
    }

    d_stderr("Preset '%s' not found in default user bank", presetName);
    return false;
}

bool CPresetManager::renamePresetInDefaultBank(const char* oldName,
                                               const char* newName) {
    // Check if new name already exists
    for (const auto& p : fDefaultUserBank.Presets) {
        if (std::strcmp(p.Name, newName) == 0) {
            d_stderr("Preset with name '%s' already exists", newName);
            return false;
        }
    }

    // Find and rename the preset
    for (auto& p : fDefaultUserBank.Presets) {
        if (std::strcmp(p.Name, oldName) == 0) {
            std::strncpy(p.Name, newName, 127);
            p.Name[127] = '\0';
            d_stderr("Renamed preset '%s' to '%s'", oldName, newName);
            saveDefaultBank();  // Save changes immediately
            return true;
        }
    }

    d_stderr("Preset '%s' not found in default user bank", oldName);
    return false;
}

[[deprecated(
    "renameDefaultBank() is deprecated. Default User Bank cannot be renamed.")]]
void CPresetManager::renameDefaultBank(const char* newName) {
    // DEPRECATED: Default User Bank should not be renamed
    // This method is kept for backward compatibility but logs a warning
    d_stderr(
        "WARNING: renameDefaultBank() is deprecated. Default User Bank "
        "cannot be renamed to avoid naming conflicts.");

    // No-op: do not actually rename the bank
    (void)newName;  // Suppress unused parameter warning
}

String CPresetManager::getDefaultBankName() const {
    return fDefaultUserBank.Name;
}

size_t CPresetManager::getDefaultBankPresetCount() const {
    return fDefaultUserBank.Presets.size();
}

String CPresetManager::getDefaultBankPresetName(size_t index) const {
    if (index < fDefaultUserBank.Presets.size()) {
        return String(fDefaultUserBank.Presets[index].Name);
    }
    return String();
}

// ============================================================================
// Parameter Snapshot
// ============================================================================

SynthProgram CPresetManager::captureCurrentParameters() const {
    SynthProgram snapshot;
    std::memset(&snapshot, 0, sizeof(SynthProgram));

    // Capture all current parameter values from UI knobs and switches
    // Note: We access the knob values directly since they reflect current DSP
    // state

    snapshot.Coarse[0] = ui->_c_val2coarse(ui->fKnobOsc1Coarse->getValue());
    snapshot.Fine[0] = ui->_c_val2fine(ui->fKnobOsc1Fine->getValue());
    snapshot.Wave[0] = ui->_pf2i(ui->fKnobOsc1Waveform->getValue(), WAVE_MAX);
    snapshot.Morph[0] = ui->fKnobOsc1Morph->getValue();
    snapshot.Volume[0] = ui->fKnobOsc1Volume->getValue() *
                         2.0f;  // Convert back to internal value

    snapshot.Coarse[1] = ui->_c_val2coarse(ui->fKnobOsc2Coarse->getValue());
    snapshot.Fine[1] = ui->_c_val2fine(ui->fKnobOsc2Fine->getValue());
    snapshot.Wave[1] = ui->_pf2i(ui->fKnobOsc2Waveform->getValue(), WAVE_MAX);
    snapshot.Morph[1] = ui->fKnobOsc2Morph->getValue();
    snapshot.Volume[1] = ui->fKnobOsc2Volume->getValue() * 2.0f;

    snapshot.Attack[0] = ui->fAmpAttack->getValue();
    snapshot.Decay[0] = ui->fAmpDecay->getValue();
    snapshot.Attack[1] = ui->fModAttack->getValue();
    snapshot.Decay[1] = ui->fModDecay->getValue();

    snapshot.ModEnv = ui->fModEnvelope->getValue() - 0.5f;  // Remove offset
    snapshot.ModVel = ui->fModVelocity->getValue();
    snapshot.ModRes = ui->fModResDecay->getValue();

    snapshot.Cutoff = ui->fFilterCutoff->getValue();
    snapshot.Resonance = ui->fFilterResonance->getValue();
    snapshot.FilterType =
        ui->_pf2i(ui->fFilterType->getValue(), FILTER_TYPE_MAX);

    snapshot.GlideState = ui->fBtnGlideState->isDown();
    snapshot.GlideSpeed = ui->fKnobGlideSpeed->getValue();

    snapshot.ClipState = ui->fBtnClipState->isDown();
    snapshot.MainVolume = ui->fKnobVolume->getValue() * 2.0f;

#ifdef ENABLE_POLYPHONY
    snapshot.MaxPolyphony = static_cast<int>(ui->fMaxPolyphony);
#endif

    return snapshot;
}

bool CPresetManager::loadPresetFromDefaultBank(const char* presetName) {
    UserProgram preset;

    if (!loadPresetFromDefaultBank(presetName, preset)) return false;

    // Load the preset using new format loader (supports FilterType and
    // MaxPolyphony)
    loadProgram(preset);

    return true;
}

// ============================================================================
// Bank Management Implementation
// ============================================================================

std::vector<String> CPresetManager::getImportedBankNames() {
    std::vector<String> bankNames;
    
    String banksDir = _getBanksDirectory();
    if (banksDir.isEmpty()) {
        return bankNames;
    }
    
#ifdef DISTRHO_OS_WINDOWS
    // Windows: use FindFirstFile/FindNextFile
    WIN32_FIND_DATAA findData;
    String searchPattern = banksDir + "\\*" USER_PRESET_BANK_EXTENSION;
    HANDLE hFind = FindFirstFileA(searchPattern.buffer(), &findData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                String fileName(findData.cFileName);
                
                if (fileName.endsWith(USER_PRESET_BANK_EXTENSION)) {
                    // Extract bank name (remove extension)
                    size_t nameLen = fileName.length() - strlen(USER_PRESET_BANK_EXTENSION);
                    char* bankNameBuf = (char*)std::malloc(nameLen + 1);
                    std::strncpy(bankNameBuf, fileName.buffer(), nameLen);
                    bankNameBuf[nameLen] = '\0';
                    String bankName(bankNameBuf);
                    std::free(bankNameBuf);
                    
                    // Skip reserved names
                    if (!_isDefaultUserBank(bankName.buffer()) && !_isFactoryBank(bankName.buffer())) {
                        bankNames.push_back(bankName);
                    }
                }
            }
        } while (FindNextFileA(hFind, &findData));
        
        FindClose(hFind);
    }
#else
    // Unix/Linux/macOS: use opendir/readdir
    DIR* dir = opendir(banksDir.buffer());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            String fileName(entry->d_name);
            
            if (fileName == "." || fileName == "..")
                continue;
            
            if (fileName.endsWith(USER_PRESET_BANK_EXTENSION)) {
                // Extract bank name (remove extension)
                size_t nameLen = fileName.length() - strlen(USER_PRESET_BANK_EXTENSION);
                char* bankNameBuf = (char*)std::malloc(nameLen + 1);
                std::strncpy(bankNameBuf, fileName.buffer(), nameLen);
                bankNameBuf[nameLen] = '\0';
                String bankName(bankNameBuf);
                std::free(bankNameBuf);
                
                // Skip reserved names
                if (!_isDefaultUserBank(bankName.buffer()) && !_isFactoryBank(bankName.buffer())) {
                    bankNames.push_back(bankName);
                }
            }
        }
        closedir(dir);
    }
#endif
    
    return bankNames;
}

bool CPresetManager::importBankFromFile(const char* filePath) {
    if (!filePath || filePath[0] == '\0') {
        d_stderr("importBankFromFile: Invalid file path");
        return false;
    }

    // Read and validate bank file
    PresetBank bank;
    if (!_loadBankFromFile(String(filePath), bank)) {
        d_stderr("importBankFromFile: Failed to load bank from '%s'", filePath);
        return false;
    }

    // Extract filename from path
    String fileName;
    const char* lastSlash = std::strrchr(filePath, '/');
    const char* lastBackslash = std::strrchr(filePath, '\\');
    const char* separator =
        (lastBackslash > lastSlash) ? lastBackslash : lastSlash;

    if (separator) {
        fileName = String(separator + 1);
    } else {
        fileName = String(filePath);
    }

    // Extract bank name from filename (remove extension)
    String bankName;
    if (fileName.endsWith(USER_PRESET_BANK_EXTENSION)) {
        size_t nameLen = fileName.length() - strlen(USER_PRESET_BANK_EXTENSION);
        char* nameBuf = (char*)std::malloc(nameLen + 1);
        std::strncpy(nameBuf, fileName.buffer(), nameLen);
        nameBuf[nameLen] = '\0';
        bankName = String(nameBuf);
        std::free(nameBuf);
    } else {
        bankName = fileName;
    }
    
    // Sanitize bank name for use as filename (remove illegal characters)
    _sanitizeBankName(bankName);
    
    // Rebuild filename with sanitized bank name
    fileName = bankName + USER_PRESET_BANK_EXTENSION;

    // Check if trying to import with reserved name
    if (_isDefaultUserBank(bankName.buffer()) || _isFactoryBank(bankName.buffer())) {
        d_stderr("importBankFromFile: Cannot import bank with reserved name '%s'", bankName.buffer());
        return false;
    }

    // Ensure Banks directory exists
    String banksDir = _getBanksDirectory();
    if (banksDir.isEmpty()) {
        d_stderr("importBankFromFile: Failed to get banks directory");
        return false;
    }

    if (!_createDirectoryIfNeeded(banksDir)) {
        d_stderr("importBankFromFile: Failed to create banks directory");
        return false;
    }

    // Check for name conflicts and generate unique name if needed
    String targetPath;
#ifdef DISTRHO_OS_WINDOWS
    targetPath = banksDir + "\\" + fileName;
#else
    targetPath = banksDir + "/" + fileName;
#endif

    if (_fileExists(targetPath)) {
        // Generate unique name: MyBank_2.c033bank, MyBank_3.c033bank, etc.
        // Remove extension
        size_t baseNameLen =
            fileName.length() - strlen(USER_PRESET_BANK_EXTENSION);
        char* baseNameBuf = (char*)std::malloc(baseNameLen + 1);
        std::strncpy(baseNameBuf, fileName.buffer(), baseNameLen);
        baseNameBuf[baseNameLen] = '\0';
        String baseName(baseNameBuf);
        std::free(baseNameBuf);

        int counter = 2;

        do {
            String newFileName = baseName + String("_") + String(counter) +
                                 USER_PRESET_BANK_EXTENSION;
#ifdef DISTRHO_OS_WINDOWS
            targetPath = banksDir + "\\" + newFileName;
#else
            targetPath = banksDir + "/" + newFileName;
#endif
            counter++;
        } while (_fileExists(targetPath) && counter < 100);

        if (counter >= 100) {
            d_stderr("importBankFromFile: Too many files with similar names");
            return false;
        }

        d_stderr("importBankFromFile: Name conflict resolved, using '%s'",
                 targetPath.buffer());
    }

    // Enforce consistency: Sync JSON bankName to match filename
    // Extract final bank name from targetPath (may have been renamed for uniqueness)
    String finalFileName;
    const char* lastSlashFinal = std::strrchr(targetPath.buffer(), '/');
    const char* lastBackslashFinal = std::strrchr(targetPath.buffer(), '\\');
    const char* separatorFinal = (lastBackslashFinal > lastSlashFinal) ? lastBackslashFinal : lastSlashFinal;
    
    if (separatorFinal) {
        finalFileName = String(separatorFinal + 1);
    } else {
        finalFileName = String(targetPath.buffer());
    }
    
    // Extract bank name from final filename
    String finalBankName;
    if (finalFileName.endsWith(USER_PRESET_BANK_EXTENSION)) {
        size_t nameLen = finalFileName.length() - strlen(USER_PRESET_BANK_EXTENSION);
        char* nameBuf = (char*)std::malloc(nameLen + 1);
        std::strncpy(nameBuf, finalFileName.buffer(), nameLen);
        nameBuf[nameLen] = '\0';
        finalBankName = String(nameBuf);
        std::free(nameBuf);
    } else {
        finalBankName = finalFileName;
    }
    
    // Note: finalBankName should already be sanitized since targetPath was built from sanitized fileName
    // But we verify it doesn't contain illegal characters (defensive programming)
    // In case the conflict resolution added unsanitized suffix (though current code only adds "_N")
    String verifiedBankName = finalBankName;
    _sanitizeBankName(verifiedBankName);
    if (verifiedBankName != finalBankName) {
        d_stderr("importBankFromFile: Unexpected: final filename contained illegal chars, fixed: '%s' -> '%s'",
                 finalBankName.buffer(), verifiedBankName.buffer());
        finalBankName = verifiedBankName;
    }
    
    // Force sync: JSON bankName must match filename
    if (bank.Name != finalBankName) {
        d_stderr("importBankFromFile: Fixing bank name mismatch: file='%s', JSON='%s'",
                 finalBankName.buffer(), bank.Name.buffer());
        bank.Name = finalBankName;
    }
    
    // Save bank with corrected name to Banks directory
    if (!_saveBankToFile(targetPath, bank)) {
        d_stderr("importBankFromFile: Failed to write to target path");
        return false;
    }

    d_stderr("Successfully imported bank to '%s'", targetPath.buffer());
    return true;
}

bool CPresetManager::exportBankToFile(const char* bankName,
                                      const char* filePath) {
    if (!bankName || bankName[0] == '\0' || !filePath || filePath[0] == '\0') {
        d_stderr("exportBankToFile: Invalid parameters");
        return false;
    }

    PresetBank bank;

    if (_isDefaultUserBank(bankName)) {
        // Export default User Bank
        bank = fDefaultUserBank;
    } else if (_isFactoryBank(bankName)) {
        // Cannot export Factory Bank (it's virtual)
        d_stderr("exportBankToFile: Cannot export Factory bank");
        return false;
    } else {
        // Load from file
        String bankFilePath = _getBankFilePath(bankName);
        if (bankFilePath.isEmpty() || !_loadBankFromFile(bankFilePath, bank)) {
            d_stderr("exportBankToFile: Failed to load bank '%s'", bankName);
            return false;
        }
    }
    
    // Ensure exported bank name matches the target filename (optional but recommended)
    // Extract filename from export path
    String exportFileName;
    const char* lastSlash = std::strrchr(filePath, '/');
    const char* lastBackslash = std::strrchr(filePath, '\\');
    const char* separator = (lastBackslash > lastSlash) ? lastBackslash : lastSlash;
    
    if (separator) {
        exportFileName = String(separator + 1);
    } else {
        exportFileName = String(filePath);
    }
    
    // Extract bank name from export filename
    if (exportFileName.endsWith(USER_PRESET_BANK_EXTENSION)) {
        size_t nameLen = exportFileName.length() - strlen(USER_PRESET_BANK_EXTENSION);
        char* nameBuf = (char*)std::malloc(nameLen + 1);
        std::strncpy(nameBuf, exportFileName.buffer(), nameLen);
        nameBuf[nameLen] = '\0';
        String exportBankName(nameBuf);
        std::free(nameBuf);
        
        // Update bank name to match export filename
        if (bank.Name != exportBankName) {
            d_stderr("exportBankToFile: Updating bank name to match filename: '%s'", exportBankName.buffer());
            bank.Name = exportBankName;
        }
    }

    // Serialize and write
    if (!_saveBankToFile(String(filePath), bank)) {
        d_stderr("exportBankToFile: Failed to write bank to '%s'", filePath);
        return false;
    }

    d_stderr("Successfully exported bank '%s' to '%s'", bankName, filePath);
    return true;
}

bool CPresetManager::deleteBankByName(const char* bankName) {
    if (!bankName || bankName[0] == '\0') {
        d_stderr("deleteBankByName: Invalid bank name");
        return false;
    }

    // Cannot delete special banks
    if (_isDefaultUserBank(bankName)) {
        d_stderr("deleteBankByName: Cannot delete default User bank");
        return false;
    }

    if (_isFactoryBank(bankName)) {
        d_stderr("deleteBankByName: Cannot delete Factory bank");
        return false;
    }

    // Get file path
    String filePath = _getBankFilePath(bankName);
    if (filePath.isEmpty() || !_fileExists(filePath)) {
        d_stderr("deleteBankByName: Bank file not found");
        return false;
    }

    // Delete file
#ifdef DISTRHO_OS_WINDOWS
    if (DeleteFileA(filePath.buffer()) == 0) {
        d_stderr("deleteBankByName: Failed to delete file");
        return false;
    }
#else
    if (std::remove(filePath.buffer()) != 0) {
        d_stderr("deleteBankByName: Failed to delete file");
        return false;
    }
#endif

    d_stderr("Successfully deleted bank '%s'", bankName);
    return true;
}

bool CPresetManager::renameBankByName(const char* oldName,
                                      const char* newName) {
    if (!oldName || oldName[0] == '\0' || !newName || newName[0] == '\0') {
        d_stderr("renameBankByName: Invalid parameters");
        return false;
    }

    // Sanitize new bank name for use as filename (remove illegal characters)
    String sanitizedNewName = String(newName);
    _sanitizeBankName(sanitizedNewName);

    // Cannot rename special banks
    if (_isDefaultUserBank(oldName) || _isFactoryBank(oldName)) {
        d_stderr("renameBankByName: Cannot rename Factory or User bank");
        return false;
    }

    // Cannot rename to reserved names
    if (_isDefaultUserBank(sanitizedNewName.buffer()) || _isFactoryBank(sanitizedNewName.buffer())) {
        d_stderr("renameBankByName: Cannot rename to reserved name '%s'", sanitizedNewName.buffer());
        return false;
    }

    // Check if new name already exists
    String newFilePath = _getBankFilePath(sanitizedNewName.buffer());
    if (_fileExists(newFilePath)) {
        d_stderr("renameBankByName: A bank with name '%s' already exists",
                 sanitizedNewName.buffer());
        return false;
    }

    // Get old file path
    String oldFilePath = _getBankFilePath(oldName);
    if (oldFilePath.isEmpty() || !_fileExists(oldFilePath)) {
        d_stderr("renameBankByName: Source bank not found");
        return false;
    }

    // Rename file
#ifdef DISTRHO_OS_WINDOWS
    if (MoveFileA(oldFilePath.buffer(), newFilePath.buffer()) == 0) {
        d_stderr("renameBankByName: Failed to rename file");
        return false;
    }
#else
    if (std::rename(oldFilePath.buffer(), newFilePath.buffer()) != 0) {
        d_stderr("renameBankByName: Failed to rename file");
        return false;
    }
#endif

    // Update bank internal metadata
    PresetBank bank;
    if (_loadBankFromFile(newFilePath, bank)) {
        bank.Name = sanitizedNewName;
        _saveBankToFile(newFilePath, bank);
    }

    d_stderr("Successfully renamed bank from '%s' to '%s'", oldName, sanitizedNewName.buffer());
    return true;
}

bool CPresetManager::loadPresetFromBank(const char* bankName,
                                        const char* presetName) {
    if (!bankName || bankName[0] == '\0' || !presetName ||
        presetName[0] == '\0') {
        d_stderr("loadPresetFromBank: Invalid parameters");
        return false;
    }

    // Handle Factory Bank
    if (_isFactoryBank(bankName)) {
        // Find factory preset by name
        for (uint32_t i = 0; i < 128; i++) {
            if (std::strcmp(FactoryPrograms[i].Name, presetName) == 0) {
                loadFactoryProgram(i);
                return true;
            }
        }
        d_stderr("loadPresetFromBank: Factory preset '%s' not found",
                 presetName);
        return false;
    }

    // Handle default User Bank
    if (_isDefaultUserBank(bankName)) {
        return loadPresetFromDefaultBank(presetName);
    }

    // Load from external bank file
    String bankFilePath = _getBankFilePath(bankName);
    PresetBank bank;

    if (!_loadBankFromFile(bankFilePath, bank)) {
        d_stderr("loadPresetFromBank: Failed to load bank '%s'", bankName);
        return false;
    }

    // Find preset in bank
    for (const auto& preset : bank.Presets) {
        if (std::strcmp(preset.Name, presetName) == 0) {
            loadProgram(preset);
            return true;
        }
    }

    d_stderr("loadPresetFromBank: Preset '%s' not found in bank '%s'",
             presetName, bankName);
    return false;
}

std::vector<String> CPresetManager::getPresetsInBank(const char* bankName) {
    std::vector<String> presetNames;

    if (!bankName || bankName[0] == '\0') return presetNames;

    // Handle Factory Bank
    if (_isFactoryBank(bankName)) {
        for (uint32_t i = 0; i < 128; i++) {
            presetNames.push_back(String(FactoryPrograms[i].Name));
        }
        return presetNames;
    }

    // Handle default User Bank
    if (_isDefaultUserBank(bankName)) {
        for (const auto& preset : fDefaultUserBank.Presets) {
            presetNames.push_back(String(preset.Name));
        }
        return presetNames;
    }

    // Load from external bank file
    String bankFilePath = _getBankFilePath(bankName);
    PresetBank bank;

    if (_loadBankFromFile(bankFilePath, bank)) {
        // Warn if JSON bankName doesn't match filename (optional validation)
        if (bank.Name != bankName) {
            d_stderr("Warning: Bank file '%s' has internal name '%s' (mismatch)",
                     bankName, bank.Name.buffer());
        }
        
        for (const auto& preset : bank.Presets) {
            presetNames.push_back(String(preset.Name));
        }
    }

    return presetNames;
}

// ============================================================================
// Preset-level Operations within Banks
// ============================================================================

bool CPresetManager::createNewBank(const char* bankName)
{
    if (!bankName || bankName[0] == '\0') {
        d_stderr("createNewBank: Invalid bank name");
        return false;
    }

    // Sanitize bank name for use as filename (remove illegal characters)
    String sanitizedBankName = String(bankName);
    _sanitizeBankName(sanitizedBankName);
    
    // Check for reserved names
    if (_isDefaultUserBank(sanitizedBankName.buffer()) || _isFactoryBank(sanitizedBankName.buffer())) {
        d_stderr("createNewBank: Cannot use reserved name '%s'", sanitizedBankName.buffer());
        return false;
    }

    // Check if bank already exists
    String filePath = _getBankFilePath(sanitizedBankName.buffer());
    if (_fileExists(filePath)) {
        d_stderr("createNewBank: Bank '%s' already exists", sanitizedBankName.buffer());
        return false;
    }

    // Create empty bank
    PresetBank newBank;
    newBank.Name = sanitizedBankName;
    newBank.Presets.clear();

    // Save to file
    if (!_saveBankToFile(filePath, newBank)) {
        d_stderr("createNewBank: Failed to save new bank");
        return false;
    }

    d_stderr("Successfully created new bank '%s'", sanitizedBankName.buffer());
    return true;
}

bool CPresetManager::savePresetToBank(const char* bankName, const char* presetName, const SynthProgram& preset)
{
    if (!bankName || bankName[0] == '\0' || !presetName || presetName[0] == '\0') {
        d_stderr("savePresetToBank: Invalid parameters");
        return false;
    }

    // Handle default User Bank using existing method
    if (_isDefaultUserBank(bankName)) {
        savePresetToDefaultBank(presetName, preset);
        return true;
    }

    // Cannot save to Factory Bank
    if (_isFactoryBank(bankName)) {
        d_stderr("savePresetToBank: Cannot modify Factory bank");
        return false;
    }

    // Load bank from file
    String filePath = _getBankFilePath(bankName);
    PresetBank bank;

    if (!_loadBankFromFile(filePath, bank)) {
        d_stderr("savePresetToBank: Failed to load bank '%s'", bankName);
        return false;
    }
    
    // Ensure bank.Name matches filename (enforce consistency)
    if (bank.Name != bankName) {
        d_stderr("savePresetToBank: Correcting bank name from '%s' to '%s'",
                 bank.Name.buffer(), bankName);
        bank.Name = String(bankName);
    }

    // Check if preset already exists (update) or create new
    bool found = false;
    for (auto& p : bank.Presets) {
        if (std::strcmp(p.Name, presetName) == 0) {
            // Update existing preset
            p = preset;
            std::strncpy(p.Name, presetName, 127);
            p.Name[127] = '\0';
            found = true;
            break;
        }
    }

    if (!found) {
        // Add new preset
        SynthProgram newPreset = preset;
        std::strncpy(newPreset.Name, presetName, 127);
        newPreset.Name[127] = '\0';
        bank.Presets.push_back(newPreset);
    }

    // Save bank back to file
    if (!_saveBankToFile(filePath, bank)) {
        d_stderr("savePresetToBank: Failed to save bank");
        return false;
    }

    d_stderr("Successfully %s preset '%s' in bank '%s'", found ? "updated" : "added", presetName, bankName);
    return true;
}

bool CPresetManager::deletePresetFromBank(const char* bankName, const char* presetName)
{
    if (!bankName || bankName[0] == '\0' || !presetName || presetName[0] == '\0') {
        d_stderr("deletePresetFromBank: Invalid parameters");
        return false;
    }

    // Handle default User Bank using existing method
    if (_isDefaultUserBank(bankName)) {
        return deletePresetFromDefaultBank(presetName);
    }

    // Cannot delete from Factory Bank
    if (_isFactoryBank(bankName)) {
        d_stderr("deletePresetFromBank: Cannot modify Factory bank");
        return false;
    }

    // Load bank from file
    String filePath = _getBankFilePath(bankName);
    PresetBank bank;

    if (!_loadBankFromFile(filePath, bank)) {
        d_stderr("deletePresetFromBank: Failed to load bank '%s'", bankName);
        return false;
    }
    
    // Ensure bank.Name matches filename (enforce consistency)
    if (bank.Name != bankName) {
        d_stderr("deletePresetFromBank: Correcting bank name from '%s' to '%s'",
                 bank.Name.buffer(), bankName);
        bank.Name = String(bankName);
    }

    // Find and remove preset
    bool found = false;
    for (auto it = bank.Presets.begin(); it != bank.Presets.end(); ++it) {
        if (std::strcmp(it->Name, presetName) == 0) {
            bank.Presets.erase(it);
            found = true;
            break;
        }
    }

    if (!found) {
        d_stderr("deletePresetFromBank: Preset '%s' not found in bank '%s'", presetName, bankName);
        return false;
    }

    // Save bank back to file
    if (!_saveBankToFile(filePath, bank)) {
        d_stderr("deletePresetFromBank: Failed to save bank");
        return false;
    }

    d_stderr("Successfully deleted preset '%s' from bank '%s'", presetName, bankName);
    return true;
}

bool CPresetManager::renamePresetInBank(const char* bankName, const char* oldName, const char* newName)
{
    if (!bankName || bankName[0] == '\0' || !oldName || oldName[0] == '\0' || !newName || newName[0] == '\0') {
        d_stderr("renamePresetInBank: Invalid parameters");
        return false;
    }

    // Handle default User Bank using existing method
    if (_isDefaultUserBank(bankName)) {
        return renamePresetInDefaultBank(oldName, newName);
    }

    // Cannot rename in Factory Bank
    if (_isFactoryBank(bankName)) {
        d_stderr("renamePresetInBank: Cannot modify Factory bank");
        return false;
    }

    // Load bank from file
    String filePath = _getBankFilePath(bankName);
    PresetBank bank;

    if (!_loadBankFromFile(filePath, bank)) {
        d_stderr("renamePresetInBank: Failed to load bank '%s'", bankName);
        return false;
    }
    
    // Ensure bank.Name matches filename (enforce consistency)
    if (bank.Name != bankName) {
        d_stderr("renamePresetInBank: Correcting bank name from '%s' to '%s'",
                 bank.Name.buffer(), bankName);
        bank.Name = String(bankName);
    }

    // Check if new name already exists
    for (const auto& p : bank.Presets) {
        if (std::strcmp(p.Name, newName) == 0) {
            d_stderr("renamePresetInBank: Preset '%s' already exists in bank", newName);
            return false;
        }
    }

    // Find and rename preset
    bool found = false;
    for (auto& p : bank.Presets) {
        if (std::strcmp(p.Name, oldName) == 0) {
            std::strncpy(p.Name, newName, 127);
            p.Name[127] = '\0';
            found = true;
            break;
        }
    }

    if (!found) {
        d_stderr("renamePresetInBank: Preset '%s' not found in bank '%s'", oldName, bankName);
        return false;
    }

    // Save bank back to file
    if (!_saveBankToFile(filePath, bank)) {
        d_stderr("renamePresetInBank: Failed to save bank");
        return false;
    }

    d_stderr("Successfully renamed preset '%s' to '%s' in bank '%s'", oldName, newName, bankName);
    return true;
}
