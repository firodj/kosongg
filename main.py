import os
import jinja2
from dulwich import porcelain
from dulwich.repo import Repo, NotGitRepository

cmake_min_version = '3.20'
project_name = 'cobacoba'
cpp_std='17'
c_std='17'
output_path = '..'

current_path = os.path.dirname(__file__)
project_path = os.path.normpath(os.path.join(current_path, '../'))
project_name = os.path.basename(project_path)

sources = []
headers = []

class LibraryManager:
    def __init__(self):
        self.libraries = dict()
    def append(self, libinfo):
        self.libraries[libinfo['name']] = libinfo
    def enable(self, name):
        if name in self.libraries:
            self.libraries[name]['enable'] = True
        else:
            raise Exception("unknown lib name: " + name)
    def enable_libraries(self):
        return list(filter(lambda libinfo: libinfo.get('enable') == True, self.libraries.values()))
    def add(self, name):
        repo = self.libraries[name]['repo']
        repo_path = project_path + '/' + repo['path']

        try:
            print(repo)
            porcelain.clone(repo['url'], repo_path, depth=1, branch=repo['branch'])
        except FileExistsError as e:
            print(e)

        r = Repo(repo_path)
        print(r)
        refs = [k for k, v in r.get_refs().items() if v == r.head()]
        refs.append(r.head())
        print(refs)

        cfg = r.get_config()
        secs = list(cfg.sections())
        print(secs)

libmgr = LibraryManager()

def useUnicornEngine():
    libmgr.append(dict(
        name = "unicorn",
        include_dirs = ['${unicorn_SOURCE_DIR}/include'],
        sets = [
            dict(name="UNICORN_BUILD_SAMPLES", value=False),
            dict(name="UNICORN_ARCH", value="x86")
        ],
        link_libs = ["unicorn"],
        options = [
            dict(name="USE_VM",
                 help="Use hardware virtualization backend for Unicorn-Engine",
                 value=False)
        ],
        path = "ext/unicorn2",
        repo = dict(
            url='https://github.com/unicorn-engine/unicorn.git',
            path='ext/unicorn2',
            branch='2.0.1.post1',
        )
    ))

def useSDL2():
    libmgr.append(dict(
        name = "sdl2",
        include_dirs = ['${SDL2_SOURCE_DIR}/include'],
        sets = [
            dict(name="SDL_TEST", value=False),
            dict(name="SDL2_DISABLE_SDL2MAIN", value=True)
        ],
        link_libs = ["SDL2-static"],
        options = [],
        path = "ext/sdl2",
        sources = [
            '${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp'
        ],
        repo = dict(
            url='https://github.com/libsdl-org/SDL.git',
            path='ext/sdl2',
            branch='release-2.28.5',
        )
    ))

def useOpenGL():
    libmgr.append(dict(
        name = "opengl",
        requires = ['OpenGL'],
        sources = [
            '${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp'
        ],
        include_dirs = [
            '${OPENGL_INCLUDE_DIR}',
        ],
        link_libs = [
            '${OPENGL_LIBRARIES}',
        ],
    ))

def useGlad():
    libmgr.append(dict(
        name = "glad",
        link_libs = [
            'glad_gl_core_33',
        ],
        sets = [
            dict(name="GLAD_SOURCES_DIR", value='${PROJECT_SOURCE_DIR}/ext/glad2')
        ],
        cmds = [
            'add_subdirectory("${GLAD_SOURCES_DIR}/cmake" glad_cmake)',
            'glad_add_library(glad_gl_core_33 REPRODUCIBLE API gl:core=3.3)',
        ],
        repo = dict(
            url='https://github.com/Dav1dde/glad.git',
            path='ext/glad2',
            branch='glad2',
        )
    ))

def useGlm():
    libmgr.append(dict(
        name = "glm",
        sets = [dict(name='GLM_DIR', value='ext/glm')],
        include_dirs = [
            '${GLM_DIR}',
        ],
        repo = dict(
            url='https://github.com/g-truc/glm.git',
            path='ext/glm',
            branch='1.0.0',
        )
    ))

def useImgui():
    libmgr.append(dict(
        name = "imgui",
        sets = [
            dict(name='IMGUI_DIR', value='ext/imgui-docking'),
        ],
        include_dirs = [
            '${IMGUI_DIR}',
            '${IMGUI_DIR}/backends'
        ],
        sources = [
            '${IMGUI_DIR}/imgui.cpp',
            '${IMGUI_DIR}/imgui_demo.cpp',
            '${IMGUI_DIR}/imgui_draw.cpp',
            '${IMGUI_DIR}/imgui_tables.cpp',
            '${IMGUI_DIR}/imgui_widgets.cpp',
        ],
        repo = dict(
            url='https://github.com/ocornut/imgui.git',
            path='ext/imgui-docking',
            branch='docking',
        )
    ))

def useYaml():
    libmgr.append(dict(
        name = "yaml-cpp",
        path = "ext/yaml-cpp",
        iinclude_dirs = [
            '${YAML_CPP_SOURCE_DIR}/include',
        ],
        link_libs = [
            'yaml-cpp'
        ],
        repo = dict(
            url='https://github.com/jbeder/yaml-cpp.git',
            path='ext/yaml-cpp',
            branch='0.8.0',
        )
    ))

def value_format(value):
    if type(value) is bool:
        return "ON" if value else "OFF"
    return '"' + value + '"'

def createCMake():
    print(project_path, project_name)

    env = jinja2.Environment(
        loader=jinja2.FileSystemLoader(current_path + "/templates")
    )
    env.filters["value_format"] = value_format
    template = env.get_template("CMakeLists.txt.jinja")

    contents = template.render(
        cmake_min_version=cmake_min_version,
        project_name=project_name,
        cpp_std=cpp_std,
        c_std=c_std,
        libraries=libmgr.enable_libraries(),
        sources=sources,
        headers=headers,
    )

    with open(os.path.join(project_path, 'CMakeLists.txt'), 'w') as f:
        f.write(contents)

def checkExt():
    for extpath in os.listdir(project_path + '/ext'):
        try:
            r = Repo(project_path + '/ext/' + extpath)

            cfg = r.get_config()
            secs = list(cfg.sections())
            for s in secs:
                print(s[0])
                print(s, list(cfg.items(secs[1])))
            print(cfg.get(("remote", "origin"), "url"))
        except NotGitRepository as e:
            print(extpath, e)

def getExistingSources():
    try:
        with open(os.path.join(project_path, 'CMakeLists.txt'), 'r') as f:
            capture = None
            for line in f.readlines():
                line = line.strip()
                if line.startswith("## -- sources"):
                    capture = "sources"
                elif line.startswith("## -- headers"):
                    capture = "headers"
                elif line.startswith("## -- end"):
                    capture = None
                else:
                    match capture:
                        case "sources":
                            sources.append(line)
                        case "headers":
                            headers.append(line)
    except FileNotFoundError as e:
        print(e)

if __name__ == '__main__':
    useOpenGL()
    useUnicornEngine()
    useSDL2()
    useImgui()
    useGlad()
    useGlm()
    useYaml()

    libmgr.enable('opengl')
    libmgr.enable('unicorn')
    libmgr.enable('sdl2')
    libmgr.enable('glad')
    libmgr.enable('glm')
    libmgr.enable('imgui')
    #libmgr.enable('yaml-cpp')

    libmgr.add('sdl2')
    libmgr.add('imgui')
    libmgr.add('glad')
    libmgr.add('glm')
    libmgr.add('unicorn')

    getExistingSources()
    createCMake()

# cd output/ext
# git clone --branch=mob --depth=1 git://repo.or.cz/tinycc.git