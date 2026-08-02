cmake_minimum_required(VERSION 3.20)
project(krita_quickshape VERSION 0.0.1 LANGUAGES CXX)

include(CTest)

add_library(quickshape_core
    core/src/stroke.cpp
)
target_include_directories(quickshape_core PUBLIC core/include)
target_compile_features(quickshape_core PUBLIC cxx_std_20)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(quickshape_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
elseif(MSVC)
    target_compile_options(quickshape_core PRIVATE /W4 /permissive-)
endif()

if(BUILD_TESTING)
    add_executable(stroke_test tests/stroke_test.cpp)
    target_link_libraries(stroke_test PRIVATE quickshape_core)
    add_test(NAME stroke_test COMMAND stroke_test)
endif()
