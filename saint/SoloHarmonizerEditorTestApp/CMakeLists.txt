add_executable(SoloHarmonizerEditorTestApp
    FilePlaybackComponent.cpp
    SoloHarmonizerEditorTestApp.cpp
    TestAppMainWindow.cpp
)

target_link_libraries(SoloHarmonizerEditorTestApp
    PRIVATE
        SoloHarmonizerVst
        TestUtils
        gmock
        ${JuceLibDeps_SoloHarmonizerVst}
        ${JuceLibDeps_TestUtils}
)
