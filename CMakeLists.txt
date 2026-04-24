cmake_minimum_required(VERSION 3.22)

project(LARDOSSA VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC AND MSVC_VERSION LESS 1934)
    message(FATAL_ERROR "LARDOSSA requires Visual Studio 2022 17.4+ (MSVC 19.34+).")
endif()

option(LARDOSSA_FETCH_JUCE "Fetch JUCE via CMake if unavailable" ON)

if(LARDOSSA_FETCH_JUCE)
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
    )
    FetchContent_MakeAvailable(JUCE)
elseif(DEFINED ENV{JUCE_DIR})
    add_subdirectory($ENV{JUCE_DIR} JUCE)
else()
    message(FATAL_ERROR "Set JUCE_DIR or enable LARDOSSA_FETCH_JUCE")
endif()

if(APPLE)
    set(LARDOSSA_FORMATS VST3 AU Standalone)
else()
    set(LARDOSSA_FORMATS VST3 Standalone)
endif()

juce_add_plugin(LARDOSSA
    COMPANY_NAME "Supine Music"
    BUNDLE_ID com.supinemusic.lardossa
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE SpMu
    PLUGIN_CODE LrDS
    FORMATS ${LARDOSSA_FORMATS}
    PRODUCT_NAME "LARDOSSA"
)

juce_generate_juce_header(LARDOSSA)

juce_add_binary_data(LardossaUI
    HEADER_NAME LardossaUiData.h
    NAMESPACE lardossa_ui
    SOURCES
        ui_kits/lardossa-vst/dist/index.html
        ui_kits/lardossa-vst/dist/assets/index.js
        ui_kits/lardossa-vst/dist/assets/index.css
)

juce_add_binary_data(LardossaPresets
    HEADER_NAME LardossaPresets.h
    NAMESPACE lardossa_presets
    SOURCES
        Presets/Factory/LARDOSSEN_BASS.xml
        Presets/Factory/POSITRON_STAB.xml
        Presets/Factory/AQUABAHN_LEAD.xml
        Presets/Factory/GRAVA_PAD.xml
        Presets/Factory/LARDOSSA_FLOOD.xml
)

target_include_directories(LARDOSSA PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Source")

if(WIN32)
    set(_LARDOSSA_WV2 "${CMAKE_CURRENT_BINARY_DIR}/webview2_pkg")
    file(GLOB_RECURSE _LARDOSSA_WV2_HDR0 "${_LARDOSSA_WV2}/WebView2.h")
    if(NOT _LARDOSSA_WV2_HDR0)
        set(_LARDOSSA_NUPKG "${CMAKE_CURRENT_BINARY_DIR}/Microsoft.Web.WebView2.nupkg")
        if(NOT EXISTS "${_LARDOSSA_NUPKG}")
            message(STATUS "Downloading WebView2 SDK headers (NuGet)...")
            file(DOWNLOAD
                "https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2/1.0.2849.39"
                "${_LARDOSSA_NUPKG}"
                TLS_VERIFY ON)
        endif()
        file(MAKE_DIRECTORY "${_LARDOSSA_WV2}")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xf "${_LARDOSSA_NUPKG}"
            WORKING_DIRECTORY "${_LARDOSSA_WV2}"
            RESULT_VARIABLE _LARDOSSA_WV2_EXTRACT)
        if(NOT _LARDOSSA_WV2_EXTRACT EQUAL 0)
            message(WARNING "WebView2 NuGet extract failed (code ${_LARDOSSA_WV2_EXTRACT}).")
        endif()
    endif()
    file(GLOB_RECURSE _LARDOSSA_WV2_HDR1 "${_LARDOSSA_WV2}/WebView2.h")
    if(_LARDOSSA_WV2_HDR1)
        get_filename_component(LARDOSSA_WEBVIEW2_INCLUDE "${_LARDOSSA_WV2_HDR1}" DIRECTORY)
        message(STATUS "WebView2 headers: ${LARDOSSA_WEBVIEW2_INCLUDE}")
    endif()

    set(LARDOSSA_WEBVIEW2_INCLUDE_LEGACY "${CMAKE_CURRENT_BINARY_DIR}/_webview2_sdk/build/native/include")
    if(DEFINED LARDOSSA_WEBVIEW2_INCLUDE AND EXISTS "${LARDOSSA_WEBVIEW2_INCLUDE}/WebView2.h")
        target_include_directories(LARDOSSA PRIVATE "${LARDOSSA_WEBVIEW2_INCLUDE}")
    elseif(EXISTS "${LARDOSSA_WEBVIEW2_INCLUDE_LEGACY}/WebView2.h")
        target_include_directories(LARDOSSA PRIVATE "${LARDOSSA_WEBVIEW2_INCLUDE_LEGACY}")
    elseif(DEFINED ENV{WEBVIEW2_SDK_DIR} AND EXISTS "$ENV{WEBVIEW2_SDK_DIR}/build/native/include/WebView2.h")
        target_include_directories(LARDOSSA PRIVATE "$ENV{WEBVIEW2_SDK_DIR}/build/native/include")
        message(STATUS "Using WEBVIEW2_SDK_DIR include path")
    else()
        message(STATUS "WebView2 SDK include path not found; relying on JUCE configuration")
    endif()
endif()

target_sources(LARDOSSA
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/Parameters.h
        Source/BridgeTypes.h
        Source/FmOperator.h
        Source/FmOperator.cpp
        Source/FmVoice.cpp
        Source/FmVoice.h
        Source/VoicePool.h
        Source/VoicePool.cpp
        Source/SubtractiveVoice.cpp
        Source/SubtractiveVoice.h
        Source/SequencerTypes.h
        Source/SequencerTiming.cpp
        Source/SequencerTiming.h
        Source/PatternGenerators.cpp
        Source/PatternGenerators.h
        Source/WebStepPatch.cpp
        Source/WebStepPatch.h
        Source/TransportResolver.cpp
        Source/TransportResolver.h
        Source/TransportTypes.h
        Source/PresetManager.cpp
        Source/PresetManager.h
)

target_compile_definitions(LARDOSSA
    PUBLIC
        JUCE_WEB_BROWSER=1
        JUCE_USE_WIN_WEBVIEW2=1
        JUCE_USE_CURL=0
)

target_link_libraries(LARDOSSA
    PRIVATE
        LardossaUI
        LardossaPresets
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_compile_options(LARDOSSA PRIVATE
    $<$<CXX_COMPILER_ID:Clang,AppleClang>:-Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    target_compile_options(LARDOSSA PRIVATE -fsanitize=address,undefined)
    target_link_options(LARDOSSA PRIVATE -fsanitize=address,undefined)
endif()

enable_testing()
add_subdirectory(Tests)
