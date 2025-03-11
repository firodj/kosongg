cmake_minimum_required(VERSION 3.20)
project(imgui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)

set(THIS_VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/ext/imgui-docking")
message(STATUS "IMGUI_DIR = ${IMGUI_DIR}")
set(IMGUI_DIR "${IMGUI_DIR}" PARENT_SCOPE)

set(imgui_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imconfig.h
)

add_library(imgui
    ${imgui_SOURCES}
)
add_compile_definitions(IMGUI_USER_CONFIG="${CMAKE_CURRENT_SOURCE_DIR}/imconfig.h")
target_compile_definitions(imgui INTERFACE imgui_IMPORTS
    IMGUI_USER_CONFIG="${CMAKE_CURRENT_SOURCE_DIR}/imconfig.h")

if (BUILD_SHARED_LIBS)
    target_compile_definitions(imgui INTERFACE imgui_IMPORTS)
endif()

target_include_directories(imgui INTERFACE
    ${IMGUI_DIR}
)