add_executable(SoloHarmonizerEditorTestApp
    SoloHarmonizerEditorTestApp.cpp
    TestAppMainWindow.cpp
)

target_link_libraries(SoloHarmonizerEditorTestApp
    PRIVATE
        SoloHarmonizerVst
        gmock
        ${JuceLibDeps_SoloHarmonizerVst}
)
