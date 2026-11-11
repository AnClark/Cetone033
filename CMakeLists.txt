cmake_minimum_required (VERSION 3.20)

project (cetone033
    VERSION 1.0.0
    LANGUAGES C CXX
)

# Metadata (for CPack)
set (AUTHOR "AnClark Liu")
set (INSTALL_DIRECTORY ${PROJECT_NAME})
set (ICON_PATH "${PROJECT_SOURCE_DIR}/Screenshot.png")  # TODO: Re-design icon
set (PRODUCTBUILD_IDENTIFIER "com.anclark.cetone.033")

# Explicitily specify C++ standard to prevent build error on Github CI - macOS build
set (CMAKE_CXX_STANDARD 17)

# Option to enable polyphony mode
option (CETONE_ENABLE_POLYPHONY "Enable polyphonic mode (16 voices)" ON)
option (CETONE_ALLOW_VOLUME_BOOSTING "Allow volume boosting when polyphony is enabled.\nThis enables output normalization as well, in order to prevent overflow." ON)
option (CETONE_BUILD_JACK_STANDALONE "Build JACK standalone application on supported platforms (if JACK is installed)" OFF)

if (CETONE_BUILD_JACK_STANDALONE)
    find_package (PkgConfig REQUIRED)
endif ()

# Only build standalone (JACK) application if JACK is installed on system
if (CETONE_BUILD_JACK_STANDALONE)
    set (JACK_TARGET jack)
endif ()

# GitHub CI's Ubuntu runner cannot directly run lv2-ttl-generator.exe, even though Wine and wine-binfmt is installed.
# And, LV2 is not so common on Windows (only REAPER, Ardour and Harrison Mixbus support LV2 on Windows).
# So disable LV2 on Windows by default.
option (CONFIG_BUILD_LV2_ON_WINDOWS "Build LV2 on Windows (including cross-compiling)." OFF)
if (WIN32 AND CONFIG_BUILD_LV2_ON_WINDOWS)
    message (STATUS "Win32 environment and LV2 build is enabled, will build LV2 plugin")
    set (TARGET_LV2 lv2)
elseif (WIN32)
    message (STATUS "Win32 environment and LV2 build is disabled. Bypass LV2")
else ()
    message (STATUS "Non-Win32 environment, will build LV2 plugin")
    set (TARGET_LV2 lv2)
endif ()

# Generate dynamic metadata file
configure_file (cmake/metadata.h.in ${PROJECT_BINARY_DIR}/metadata.gen.h @ONLY)

add_subdirectory (dpf)

include_directories (   
    src/
    widgets/dpf-imgui/
    ${PROJECT_BINARY_DIR}
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
    TARGETS vst2 vst3 ${TARGET_LV2} clap ${JACK_TARGET}
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
        widgets/dpf-imgui/DearImGui.cpp
        src/Widgets/ImGui_UI.cpp
        src/Widgets/ImGui_UI_MessageBox.cpp
        src/Widgets/ImGui_UI_FileBrowser.cpp
        src/Widgets/ImGui_UI_PresetManager.cpp
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

# CPack stuff
include (cmake/Packaging.cmake)
