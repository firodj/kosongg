import os

cmake_min_version = '3.20'
project_name = 'cobacoba'
cpp_std='17'
c_std='17'

options = [
    {
        "variable": "USE_VM",
        "help": "Use hardware virtualization backend for Unicorn-Engine",
        "value": False,
    }
]

if __name__ == '__main__':
    with open('output/CMakeLists.txt', 'w') as f:
        f.write("cmake_minimum_required(VERSION %s)\n" % (cmake_min_version))
        f.write("project(%s)\n" % (project_name))
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

        f.write("set(SDL_TEST OFF)\n");
        f.write("set(SDL2_DISABLE_SDL2MAIN ON)\n");
        f.write("add_subdirectory(ext/sdl2)\n")

        f.write("include_directories(SYSTEM\n")
        f.write("  ${OPENGL_INCLUDE_DIR}\n")
        f.write("   ext/sdl2/include\n")
        f.write("  glad/include\n")
        f.write(")\n")

        f.write("add_executable(%s\n"% (project_name))
        f.write("  vendor/main.cpp\n")
        f.write("  vendor/Engine.cpp\n")
        f.write(")\n")

        f.write("target_compile_definitions(%s PUBLIC\n" % (project_name))
        f.write("  -D_CRT_SECURE_NO_WARNINGS\n")
        f.write(")\n")

        f.write("target_link_libraries(%s\n" % (project_name))
        f.write("  ${OPENGL_LIBRARIES}\n")
        f.write("  SDL2-static\n")
        f.write(")\n")

        f.write("set_target_properties(%s PROPERTIES\n" % (project_name))
        f.write('  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"\n')
        f.write('  DEBUG_POSTFIX "d"\n')
        f.write(')\n')


# git clone --branch release-2.28.5 --depth=1 https://github.com/libsdl-org/SDL.git sdl2