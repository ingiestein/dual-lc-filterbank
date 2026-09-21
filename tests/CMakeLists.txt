include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.7.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

add_executable(dual_lc_tests test_dual_lc.cpp)
target_link_libraries(dual_lc_tests PRIVATE dual_lc_dsp Catch2::Catch2WithMain)
target_compile_features(dual_lc_tests PRIVATE cxx_std_17)

include(CTest)
include(Catch)
catch_discover_tests(dual_lc_tests)
