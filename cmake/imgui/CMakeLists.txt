cmake_minimum_required(VERSION 3.20)
project(imgui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)

set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/ext/imgui-docking")
message("IMGUI_DIR = ${IMGUI_DIR}")
set(IMGUI_DIR "${IMGUI_DIR}" PARENT_SCOPE)

set(imgui_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
)

add_library(imgui
    ${imgui_SOURCES}
)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(imgui INTERFACE imgui_IMPORTS)
endif()

target_include_directories(imgui INTERFACE
    ${IMGUI_DIR}
)