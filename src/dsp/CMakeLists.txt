add_library(dual_lc_dsp STATIC
    dual_lc_cell.cpp
    filter_bank.cpp
    amplifiers.cpp
)

target_include_directories(dual_lc_dsp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(dual_lc_dsp PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(dual_lc_dsp PRIVATE /W4 /permissive-)
else()
    target_compile_options(dual_lc_dsp PRIVATE -Wall -Wextra -Wpedantic)
endif()
