add_library(SoloHarmonizer)

target_sources(SoloHarmonizer
  PUBLIC
    Playheads/HostDrivenPlayhead.cpp
    Playheads/ProcessCallbackDrivenPlayhead.cpp
    SoloHarmonizer.cpp
    SoloHarmonizerHelper.cpp
)

target_include_directories(SoloHarmonizer
  PUBLIC
    ${CMAKE_SOURCE_DIR}/spdlog/include
    ${CMAKE_SOURCE_DIR}/_thirdParty
    ${CMAKE_SOURCE_DIR}/JUCE/modules
)

target_link_libraries(SoloHarmonizer
  PUBLIC
    IntervalGetter
    MidiFileOwner
    PitchDetector
    spdlog
    DavidCNAntonia
    Utils
)

juce_add_plugin(SoloHarmonizerVst
    # VERSION ...                               # Set this if the plugin version is different to the project version
    # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
    # ICON_SMALL ...
    # COMPANY_NAME ...                          # Specify the name of the plugin's author
    # IS_SYNTH TRUE/FALSE                       # Is this a synth or an effect?
    # NEEDS_MIDI_INPUT TRUE/FALSE               # Does the plugin need midi input?
    # NEEDS_MIDI_OUTPUT TRUE/FALSE              # Does the plugin need midi output?
    # IS_MIDI_EFFECT TRUE/FALSE                 # Is this plugin a MIDI effect?
    # EDITOR_WANTS_KEYBOARD_FOCUS TRUE/FALSE    # Does the editor need keyboard focus?
    # COPY_PLUGIN_AFTER_BUILD TRUE/FALSE        # Should the plugin be installed to a default location after building?
    PLUGIN_MANUFACTURER_CODE DGK_               # A four-character manufacturer id with at least one upper-case character
    PLUGIN_CODE SHID                            # A unique four-character plugin id with exactly one upper-case character
                                                # GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case
    FORMATS VST3 Standalone                     # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
    PRODUCT_NAME "Solo Harmonizer")        # The name of the final executable, which can differ from the target name

target_sources(SoloHarmonizerVst
    PRIVATE
        SoloHarmonizerEditor.cpp
        SoloHarmonizerVst.cpp
)

target_compile_definitions(SoloHarmonizerVst
    PUBLIC
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_include_directories(SoloHarmonizerVst
    PUBLIC
        ${CMAKE_SOURCE_DIR}/JUCE/modules
        ${CMAKE_SOURCE_DIR}/_thirdParty/asiosdk/common # Needed by JUCE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(SoloHarmonizerVst
    PUBLIC
        SoloHarmonizer
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

add_executable(SoloHarmonizerTests
    SoloHarmonizerTests.cpp
    Playheads/ProcessCallbackDrivenPlayheadTests.cpp
)

target_include_directories(SoloHarmonizerTests
  PRIVATE
    ${CMAKE_SOURCE_DIR}/_thirdParty/asiosdk/common # Needed by JUCE
)

target_link_libraries(SoloHarmonizerTests
    PRIVATE
        SoloHarmonizer
        TestUtils
        gtest_main
        gmock
        ${JuceLibDeps_SoloHarmonizer}
        ${JuceLibDeps_TestUtils}
        ${JuceLibDeps_MidiFileOwner}
)
