cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
add_definitions(-DJUCE_MODAL_LOOPS_PERMITTED -DJUCE_ASIO -DJucePlugin_PreferredChannelConfigurations={1,2})
set(PFFFT_DISABLE_LINK_WITH_M 1)

# just if(MSVC) didn't work for me (wasn't true, even with a VS generator)
string(FIND ${CMAKE_GENERATOR} "Visual Studio" SAINT_VS_GEN_INDEX)
if(SAINT_VS_GEN_INDEX EQUAL -1)
    message("saint: Using non-VS generator")
    set(SAINT_ANNOYING_WARNINGS
        -Wno-implicit-int-float-conversion
        -Wno-sign-compare
        -Wno-sign-conversion
        -Wno-shorten-64-to-32
        -Wno-switch-enum
        -Wno-deprecated # or we get this annoying spam 'warning: out-of-line definition of constexpr static data member is redundant in C++17 and is deprecated' from juce_Variant.cpp)
    )
else()
    message("saint: Using some VS generator")
endif()

project(SoloHarmonizer VERSION 0.0.1)

set(JuceLibDeps_TestUtils
    juce::juce_audio_utils
    juce::juce_audio_formats
)
set(JuceLibDeps_DavidCNAntonia
    juce::juce_dsp
    juce::juce_audio_basics
)
set(JuceLibDeps_MidiFileOwner
    ${JuceLibDeps_IntervalGetter}
    juce::juce_audio_basics
)
set(JuceLibDeps_PitchDetector
    ${JuceLibDeps_TestUtils}
)
set(JuceLibDeps_SoloHarmonizer
    ${JuceLibDeps_DavidCNAntonia}
)
set(JuceLibDeps_SoloHarmonizerVst
    ${JuceLibDeps_SoloHarmonizer}
    juce::juce_audio_processors
)
set(JuceLibDeps_IntervalGetter
    ${JuceLibDeps_TestUtils}
)

set(PFFFT_DISABLE_LINK_WITH_M ON CACHE BOOL "" FORCE)

add_subdirectory(_thirdParty/DavidCNAntonia)
add_subdirectory(rubberband-wrapper)
add_subdirectory(googletest)
add_subdirectory(JUCE)
add_subdirectory(spdlog)
add_subdirectory(pffft)
add_subdirectory(eigen)
add_subdirectory(saint)

# Below are all PFFFT test executables we don't want to compile.
# It would be great to be able to remove these targets without modifying the fft repo, haven't found out how.
set_target_properties(
  test_fftpack_float
  test_fftpack_double
  test_pffft_float
  test_fft_factors
  test_pffft_cpp
  test_pffft_cpp_11
  test_pffastconv
  bench_pffft_float
  bench_pf_mixer_float
  bench_pf_conv_float
  example_cpp98_real_flt_fwd
  example_cpp98_cplx_flt_fwd
  example_c_real_flt_fwd
  PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1
)

target_compile_options(pf_conv_dispatcher PRIVATE -w)
target_compile_options(pf_conv_arch_none PRIVATE -w)
target_compile_options(pf_conv_arch_dflt PRIVATE -w)
target_compile_options(pf_conv_arch_avx2 PRIVATE -w)
target_compile_options(pf_conv_arch_avx PRIVATE -w)
target_compile_options(gmock_main PRIVATE -w)
target_compile_options(FFTPACK_DOUBLE PRIVATE -w)
target_compile_options(spdlog PRIVATE -w)
target_compile_options(PFFASTCONV PRIVATE -w)
target_compile_options(PFFFT PRIVATE -w)
target_compile_options(PFDSP PRIVATE -w)
target_compile_options(gtest_main PRIVATE -w)
target_compile_options(gtest PRIVATE -w)
target_compile_options(gmock PRIVATE -w)
target_compile_options(FFTPACK_FLOAT PRIVATE -w)
target_compile_options(spdlog PRIVATE -w)
if (SAINT_VS_GEN_INDEX EQUAL -1)
    target_compile_options(pf_conv_arch_sse4 PRIVATE -w)
    target_compile_options(pf_conv_arch_sse3 PRIVATE -w)
endif()
