include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.4
)
FetchContent_MakeAvailable(Catch2)

add_executable(LardossaTests
    FmVoiceTests.cpp
    PresetXmlTests.cpp
    VoicePoolTests.cpp
    ../Source/FmVoice.cpp
    ../Source/FmOperator.cpp
    ../Source/PresetManager.cpp
    ../Source/VoicePool.cpp
    ../Source/SubtractiveVoice.cpp
)

target_include_directories(LardossaTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../Source"
)

target_link_libraries(LardossaTests PRIVATE
    Catch2::Catch2WithMain
    LardossaPresets
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

target_compile_definitions(LardossaTests PRIVATE
    JUCE_MODAL_LOOPS_PERMITTED=1
    JUCE_WEB_BROWSER=0
    JUCE_USE_WIN_WEBVIEW2=0
)

add_test(NAME LardossaTests COMMAND LardossaTests)
