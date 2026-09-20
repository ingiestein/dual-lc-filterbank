cmake_minimum_required(VERSION 3.25.0)

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "macOS deployment target")

# Must be set before add_subdirectory(vst3sdk).
set(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES OFF CACHE BOOL "Build Steinberg plug-in examples" FORCE)
set(SMTG_ENABLE_VST3_HOSTING_EXAMPLES OFF CACHE BOOL "Build editorhost/audiohost" FORCE)
set(SMTG_ENABLE_VSTGUI_SUPPORT ON CACHE BOOL "Build VSTGUI" FORCE)
set(SMTG_CXX_STANDARD "17" CACHE STRING "C++ standard for SMTG plug-in targets")
option(SMTG_CREATE_PLUGIN_LINK "Create a link/copy of the .vst3 bundle in the local VST3 folder" ON)

project(DualLcFilterbank
    VERSION 1.0.0.0
    DESCRIPTION "Dual-LC Filterbank VST 3 Plug-in"
    LANGUAGES C CXX
)

if(APPLE)
    enable_language(OBJCXX)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(DUALLC_BUILD_PLUGIN "Build the VST3 plug-in bundle" ON)
option(DUALLC_BUILD_TESTS "Build offline DSP unit tests" ON)

add_subdirectory(src/dsp)

if(DUALLC_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(NOT DUALLC_BUILD_PLUGIN)
    return()
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/vst3sdk.cmake")
add_subdirectory(${vst3sdk_SOURCE_DIR} ${PROJECT_BINARY_DIR}/vst3sdk)
smtg_enable_vst3_sdk()

set(duallc_plugin_sources
    src/plugin/version.h
    src/plugin/cids.h
    src/plugin/processor.h
    src/plugin/processor.cpp
    src/plugin/controller.h
    src/plugin/controller.cpp
    src/plugin/factory.cpp
    src/gui/editor.uidesc
)

smtg_add_vst3plugin(DualLcFilterbank ${duallc_plugin_sources})
smtg_target_configure_version_file(DualLcFilterbank)

target_include_directories(DualLcFilterbank PRIVATE src/plugin src/dsp)

target_link_libraries(DualLcFilterbank
    PRIVATE
        dual_lc_dsp
        sdk
)

if(SMTG_ENABLE_VSTGUI_SUPPORT)
    target_link_libraries(DualLcFilterbank PRIVATE vstgui_support)
    smtg_target_add_plugin_resources(DualLcFilterbank
        RESOURCES
            src/gui/editor.uidesc
            resources/background.png
            resources/knob.png
            resources/knob_amber.png
    )
endif()

smtg_target_add_plugin_snapshots(DualLcFilterbank
    RESOURCES
        resources/B48F788026A44C3DAC806EF280802D49_snapshot.png
        resources/B48F788026A44C3DAC806EF280802D49_snapshot_2.0x.png
)

if(SMTG_MAC)
    smtg_target_set_bundle(DualLcFilterbank
        BUNDLE_IDENTIFIER com.ingiestein.duallcfilterbank
        COMPANY_NAME "Ingiestein"
    )
elseif(SMTG_WIN)
    target_sources(DualLcFilterbank PRIVATE resources/win32resource.rc)
    if(MSVC)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT DualLcFilterbank)
        smtg_target_set_debug_executable(DualLcFilterbank
            "$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
            "--pluginfolder \"$(OutDir)/\""
        )
    endif()
endif()
