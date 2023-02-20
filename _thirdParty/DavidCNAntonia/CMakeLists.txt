add_library(DavidCNAntonia)

target_sources(DavidCNAntonia
    PRIVATE
        PitchShifter.cpp
        RingBuffer.cpp)

target_link_libraries(DavidCNAntonia
    PUBLIC
        juce::juce_audio_basics
        juce::juce_dsp
        rubberband-wrapper)
