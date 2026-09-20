# Locate the Steinberg VST3 SDK submodule and export vst3sdk_SOURCE_DIR.
set(DUALLC_VST3SDK_DIR "${CMAKE_SOURCE_DIR}/third_party/vst3sdk" CACHE PATH "Path to vst3sdk")

if(NOT EXISTS "${DUALLC_VST3SDK_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "VST3 SDK not found at ${DUALLC_VST3SDK_DIR}.\n"
        "Clone recursively:  git submodule update --init --recursive")
endif()

if(NOT EXISTS "${DUALLC_VST3SDK_DIR}/public.sdk/CMakeLists.txt"
   AND NOT EXISTS "${DUALLC_VST3SDK_DIR}/public.sdk/source/vst/vstaudioeffect.h")
    message(FATAL_ERROR
        "vst3sdk nested submodules are missing (public.sdk, pluginterfaces, base, cmake, vstgui4).\n"
        "Run: git submodule update --init --recursive")
endif()

set(vst3sdk_SOURCE_DIR "${DUALLC_VST3SDK_DIR}")
set(SMTG_VSTGUI_ROOT "${vst3sdk_SOURCE_DIR}")
