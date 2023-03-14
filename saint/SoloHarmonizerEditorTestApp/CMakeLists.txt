add_executable(SoloHarmonizerEditorTestApp
    FilePlaybackComponent.cpp
    SoloHarmonizerEditorTestApp.cpp
    TestAppMainWindow.cpp
)

target_compile_options(SoloHarmonizerEditorTestApp PRIVATE ${SAINT_ANNOYING_WARNINGS})

target_link_libraries(SoloHarmonizerEditorTestApp
    PRIVATE
        SoloHarmonizerVst
        TestUtils
        gmock
        ${JuceLibDeps_SoloHarmonizerVst}
        ${JuceLibDeps_TestUtils}
)
