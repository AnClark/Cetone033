cmake_minimum_required (VERSION 3.20)

project (cetone033
    VERSION 1.0.0
    LANGUAGES C CXX
)

# Explicitily specify C++ standard to prevent build error on Github CI - macOS build
set (CMAKE_CXX_STANDARD 17)

add_subdirectory (dpf)

option (CONFIG_ENABLE_PRESET_MENU "Enable factory preset menu so that you can access official factory banks in plugin editor" ON)
message (STATUS "Cetone033 Factory Preset menu support: ${CONFIG_ENABLE_PRESET_MENU}")

include_directories (   
    src/
)

set (SRC_BACKEND
    src/CetoneLpFilter.cpp
    src/FilterBiquad.cpp
    src/FilterMoog.cpp
    src/globalfunctions.cpp
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
        ${SRC_BACKEND}
    FILES_UI
        src/CetoneUI.cpp
        src/CetoneUIHelper.cpp
        src/images/CetoneArtwork.cpp
        src/Fonts/CetoneFonts.cpp
    FILES_COMMON
        src/PresetData.cpp
)

if (CONFIG_ENABLE_PRESET_MENU)
    target_compile_definitions (${PROJECT_NAME}-ui PRIVATE ENABLE_PRESET_MENU=1)

    target_sources (${PROJECT_NAME}-ui PRIVATE
        ext/WolfWidgets/MenuWidget.cpp
        ext/WolfWidgets/WolfWidget.cpp
        ext/WolfWidgets/Margin.cpp
        src/CetoneUIMenu.cpp
    )
    target_include_directories (${PROJECT_NAME}-ui PRIVATE ext/WolfWidgets/)
endif ()
