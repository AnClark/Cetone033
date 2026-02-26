cmake_minimum_required (VERSION 3.20)

project (cetone033
    VERSION 1.0.0
    LANGUAGES C CXX
)

# Explicitily specify C++ standard to prevent build error on Github CI - macOS build
set (CMAKE_CXX_STANDARD 17)

# Option to enable polyphony mode
option (CETONE_ENABLE_POLYPHONY "Enable polyphonic mode (16 voices)" ON)
option (CETONE_ALLOW_VOLUME_BOOSTING "Allow volume boosting when polyphony is enabled.\nThis enables output normalization as well, in order to prevent overflow." ON)

add_subdirectory (dpf)

include_directories (   
    src/
    dpf-widgets/opengl/
)

set (SRC_BACKEND
    src/CetoneLpFilter.cpp
    src/FilterBiquad.cpp
    src/FilterMoog.cpp
    src/globalfunctions.cpp
    src/MidiStack.cpp
    src/SynthOscillator.cpp
)

if (CETONE_ENABLE_POLYPHONY)
    list (APPEND SRC_BACKEND src/Voice.cpp)
endif()

dpf_add_plugin (${PROJECT_NAME}
    TARGETS vst2 vst3 lv2 clap jack
    FILES_DSP
        src/Cetone033.cpp
        src/Cetone033DPF.cpp
        src/Cetone033Main.cpp
        src/Cetone033Plugin.cpp
        ${SRC_BACKEND}
    FILES_UI
        src/CetoneUI.cpp
        src/CetoneUIHelper.cpp
        src/images/CetoneArtwork.cpp
        src/Fonts/CetoneFonts.cpp 
        dpf-widgets/opengl/DearImGui.cpp
        src/Widgets/ImGui_UI.cpp
        src/Widgets/ImGui_UI_MessageBox.cpp
        src/Widgets/ImGui_UI_FileBrowser.cpp
        src/PresetManager.cpp
        src/PresetManagerHelper.cpp
    FILES_COMMON
        src/PresetData.cpp
)

if (CETONE_ENABLE_POLYPHONY)
    target_compile_definitions (${PROJECT_NAME} PUBLIC ENABLE_POLYPHONY)
    if (CETONE_ALLOW_VOLUME_BOOSTING)
        target_compile_definitions (${PROJECT_NAME} PUBLIC ENABLE_VOLUME_BOOSTING)
        message (STATUS ">>> Polyphony enabled with volume boosting.")
        message (STATUS ">>>>>> NOTE: Output normalization is active to prevent overflow. <<<<<<")
    else()
        message (STATUS ">>> Polyphony enabled with normal volume ranges.")
    endif()
endif()
