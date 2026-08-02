cmake_minimum_required(VERSION 3.20)
project(quickshape_krita_5_3_3 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 5.15 REQUIRED COMPONENTS
    Core Gui Widgets Xml Network PrintSupport Svg Concurrent Sql)
find_package(ECM 5.91 REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH "${ECM_MODULE_PATH}")
find_package(KF5 5.44 REQUIRED COMPONENTS
    Config WidgetsAddons Completion CoreAddons GuiAddons I18n ItemViews)

set(QUICKSHAPE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../..")
set(KRITA_SOURCE "${QUICKSHAPE_ROOT}/build/krita-6.0.3-source")
set(KRITA_BUILD "${QUICKSHAPE_ROOT}/build/krita-qt5-configure")
set(KRITA_APPDIR "${QUICKSHAPE_ROOT}/build/krita-5.3.3-appimage/squashfs-root/usr")
set(LOCAL_DEPS "${QUICKSHAPE_ROOT}/build/deps/root/usr")

if(NOT EXISTS "${KRITA_BUILD}/libs/ui/kritaui_export.h")
    message(FATAL_ERROR "Run scripts/configure-krita-qt5.sh first")
endif()

file(GLOB_RECURSE KRITA_HEADERS CONFIGURE_DEPENDS
    "${KRITA_SOURCE}/libs/*.h"
    "${KRITA_BUILD}/libs/*.h")
set(KRITA_INCLUDE_DIRS "${KRITA_SOURCE}" "${KRITA_BUILD}")
foreach(header IN LISTS KRITA_HEADERS)
    get_filename_component(header_dir "${header}" DIRECTORY)
    list(APPEND KRITA_INCLUDE_DIRS "${header_dir}")
endforeach()
list(REMOVE_DUPLICATES KRITA_INCLUDE_DIRS)

add_library(quickshape_krita_5_3_3 MODULE
    "${QUICKSHAPE_ROOT}/core/src/host_contract.cpp"
    "${QUICKSHAPE_ROOT}/core/src/stroke.cpp"
    quickshape_plugin.cpp
    quickshape_tool.cpp
    quickshape_tool.hpp
)
target_include_directories(quickshape_krita_5_3_3 PRIVATE
    "${QUICKSHAPE_ROOT}/core/include"
    ${KRITA_INCLUDE_DIRS}
    "${LOCAL_DEPS}/include"
    "${LOCAL_DEPS}/include/eigen3"
    "${LOCAL_DEPS}/include/KF5"
)
target_compile_definitions(quickshape_krita_5_3_3 PRIVATE
    HAVE_X11
    QT_NO_SIGNALS_SLOTS_KEYWORDS
    TRANSLATION_DOMAIN="quickshape"
)
target_compile_options(quickshape_krita_5_3_3 PRIVATE
    -fno-operator-names
    -Wall -Wextra -Wpedantic -Wconversion
)
target_link_libraries(quickshape_krita_5_3_3 PRIVATE
    "${KRITA_APPDIR}/lib/libkritaui.so.20.0.0"
    "${KRITA_APPDIR}/lib/libkritaflake.so.20.0.0"
    Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Xml Qt5::Network
    Qt5::PrintSupport Qt5::Svg Qt5::Concurrent Qt5::Sql
    KF5::ConfigCore KF5::WidgetsAddons KF5::Completion KF5::CoreAddons
    KF5::GuiAddons KF5::I18n KF5::ItemViews
)

set_target_properties(quickshape_krita_5_3_3 PROPERTIES
    PREFIX ""
    OUTPUT_NAME "kritaquickshape_krita_5_3_3"
    BUILD_RPATH "${KRITA_APPDIR}/lib"
)
