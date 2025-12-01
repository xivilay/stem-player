cmake_minimum_required(VERSION 3.22)

project(StemPlayer VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE
include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
)

FetchContent_MakeAvailable(JUCE)

# Plugin/Standalone target
juce_add_plugin(StemPlayer
    COMPANY_NAME "XivilaY"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE XivY
    PLUGIN_CODE Stpl
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Stem Player"
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon_512.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon_128.png"
)

target_sources(StemPlayer
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/Core/StemEngine.cpp
        Source/Core/StemEngine.h
        Source/Core/StemTrack.cpp
        Source/Core/StemTrack.h
        Source/Core/StemDetector.cpp
        Source/Core/StemDetector.h
        Source/Core/MidiLearnManager.cpp
        Source/Core/MidiLearnManager.h
        Source/Core/AppSettings.cpp
        Source/Core/AppSettings.h
        Source/UI/SelectionScreen.cpp
        Source/UI/SelectionScreen.h
        Source/UI/MainScreen.cpp
        Source/UI/MainScreen.h
        Source/UI/SettingsScreen.cpp
        Source/UI/SettingsScreen.h
        Source/UI/WaveformDisplay.cpp
        Source/UI/WaveformDisplay.h
        Source/UI/StemTrackComponent.cpp
        Source/UI/StemTrackComponent.h
        Source/UI/LookAndFeel.cpp
        Source/UI/LookAndFeel.h
        Source/UI/IconButton.cpp
        Source/UI/IconButton.h
)

# Add icon resources as binary data
juce_add_binary_data(IconResources
    SOURCES
        Resources/icons/back.svg
        Resources/icons/play.svg
        Resources/icons/pause.svg
        Resources/icons/stop.svg
        Resources/icons/settings.svg
        Resources/icons/load.svg
        Resources/icons/browse.svg
)

target_link_libraries(StemPlayer PRIVATE IconResources)

juce_generate_juce_header(StemPlayer)

target_compile_definitions(StemPlayer
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(StemPlayer
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

