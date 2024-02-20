import os
import jinja2

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

    env = jinja2.Environment(
        loader=jinja2.FileSystemLoader(current_path + "/templates")
    )
    template = env.get_template("CMakeLists.txt.jinja")
    with open(os.path.join(project_path, 'CMakeLists.txt'), 'w') as f:
        f.write(template.render(
            cmake_min_version=cmake_min_version,
            project_name=project_name,
            cpp_std=cpp_std,
            c_std=c_std,
            options=options,
        ))


# cd output/ext
# git clone --branch release-2.28.5 --depth=1 https://github.com/libsdl-org/SDL.git sdl2
# git clone --branch docking --depth=1 https://github.com/ocornut/imgui.git imgui-docking
# git clone --branch 0.8.0 --depth=1 https://github.com/jbeder/yaml-cpp.git yaml-cpp
# git clone --branch 2.0.1.post1 --depth=1 https://github.com/unicorn-engine/unicorn.git
# git clone --branch glad2 --depth=1 https://github.com/Dav1dde/glad.git glad2