cmake_minimum_required (VERSION 3.20)

project (cetone033
    VERSION 1.0.0
    LANGUAGES C CXX
)

# Explicitily specify C++ standard to prevent build error on Github CI - macOS build
set (CMAKE_CXX_STANDARD 17)

add_subdirectory (dpf)

include_directories (   
    src/
)

set (SRC_BACKEND
    src/CetoneLpFilter.cpp
    src/FilterBiquad.cpp
    src/FilterMoog.cpp
    src/GlobalFunctions.cpp
    src/MidiStack.cpp
    src/SynthOscillator.cpp
)

dpf_add_plugin (${PROJECT_NAME}
    TARGETS vst2 vst3 lv2 clap jack
    FILES_DSP
        src/Cetone033.cpp
        src/Cetone033DPF.cpp
        src/Cetone033Main.cpp
        src/Cetone033Plugin.cpp
        src/PresetData.cpp
        ${SRC_BACKEND}
    #FILES_UI
    #    src/CetoneUI.cpp
    #    src/CetoneUIHelper.cpp
    #    src/Images/CetoneArtwork.cpp
    #    src/Fonts/CetoneFonts.cpp
)

