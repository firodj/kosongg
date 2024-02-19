import os

cmake_min_version = '3.20'
project_name = 'cobacoba'
cpp_std='17'
c_std='17'
output_path = '..'

options = [
    {
        "variable": "USE_VM",
        "help": "Use hardware virtualization backend for Unicorn-Engine",
        "value": False,
    }
]

if __name__ == '__main__':
    current_path = os.path.dirname(__file__)
    project_path = os.path.normpath(os.path.join(current_path, '../'))
    project_name = os.path.basename(project_path)
    print(project_path, project_name)

    with open(os.path.join(project_path, 'CMakeLists.txt'), 'w') as f:
        f.write("cmake_minimum_required(VERSION %s)\n" % (cmake_min_version))
        f.write("project(%s)\n" % (project_name))
        f.write("\n")
        f.write('set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")\n')
        f.write("set(CMAKE_CXX_STANDARD %s)\n" % (cpp_std))
        f.write("set(CMAKE_C_STANDARD %s)\n" % (c_std))
        for opt in options:
            value = str(opt["value"])
            match opt["value"]:
                case True:
                    value = "ON"
                case False:
                    value = "OFF"
            f.write('option("%s" "%s" %s)\n'  % (opt["variable"], opt["help"], value))

        f.write("find_package(OpenGL REQUIRED)\n")
        f.write("\n")

        f.write("set(SDL2_DIR ext/sdl2)\n")
        f.write("set(IMGUI_DIR ext/imgui-docking)\n")
        f.write("set(VENDOR_DIR kosongg/vendor)\n")
        f.write("\n")

        f.write("set(SDL_TEST OFF)\n")
        f.write("set(SDL2_DISABLE_SDL2MAIN ON)\n")
        f.write("add_subdirectory(${SDL2_DIR})\n")
        f.write("\n")


        f.write("include_directories(SYSTEM\n")
        f.write("  ${OPENGL_INCLUDE_DIR}\n")
        f.write("  ${SDL2_DIR}/include\n")
        f.write("  glad/include\n")
        f.write("  ${IMGUI_DIR}\n")
        f.write("  ${IMGUI_DIR}/backends\n")
        f.write(")\n")
        f.write("\n")

        f.write("add_executable(%s\n"% (project_name))
        f.write("  ${VENDOR_DIR}/main.cpp\n")
        f.write("  ${VENDOR_DIR}/Engine.cpp\n")
        f.write("  ${VENDOR_DIR}/Component.cpp\n")

        f.write("  ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp\n")
        f.write("  ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp\n")
        f.write("  ${IMGUI_DIR}/imgui.cpp\n")
        f.write("  ${IMGUI_DIR}/imgui_demo.cpp\n")
        f.write("  ${IMGUI_DIR}/imgui_draw.cpp\n")
        f.write("  ${IMGUI_DIR}/imgui_tables.cpp\n")
        f.write("  ${IMGUI_DIR}/imgui_widgets.cpp\n")
        f.write(")\n")
        f.write("\n")

        f.write("target_compile_definitions(%s PUBLIC\n" % (project_name))
        f.write("  -D_CRT_SECURE_NO_WARNINGS\n")
        f.write(")\n")
        f.write("\n")

        f.write("target_link_libraries(%s\n" % (project_name))
        f.write("  ${OPENGL_LIBRARIES}\n")
        f.write("  SDL2-static\n")
        f.write(")\n")
        f.write("\n")

        f.write("set_target_properties(%s PROPERTIES\n" % (project_name))
        f.write('  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}"\n')
        f.write('  VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"\n')
        f.write('  DEBUG_POSTFIX "d"\n')
        f.write(')\n')

        f.write('set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT emulasi)\n')


# cd output/ext
# git clone --branch release-2.28.5 --depth=1 https://github.com/libsdl-org/SDL.git sdl2
# git clone --branch docking --depth=1 https://github.com/ocornut/imgui.git imgui-docking
# git clone --branch 0.8.0 --depth=1 https://github.com/jbeder/yaml-cpp.git yaml-cpp
# git clone --branch 2.0.1.post1 --depth=1 https://github.com/unicorn-engine/unicorn.git
# git clone --branch glad2 --depth=1 https://github.com/Dav1dde/glad.git glad2