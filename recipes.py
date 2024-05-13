from dulwich import porcelain
from dulwich.repo import Repo

class LibraryManager:
    def __init__(self, project_path):
        self.libraries = dict()
        self.project_path = project_path
        self.registers()

    def append(self, libinfo):
        self.libraries[libinfo['name']] = libinfo

    def enable(self, name):
        if name in self.libraries:
            self.libraries[name]['enable'] = True
        else:
            raise Exception("unknown lib name: " + name)

    def is_enabled(self, name):
        if name in self.libraries:
            return self.libraries[name].get('enable')
        print("WARNING: chekcing unknown lib name: " + name)
        return None

    def enable_libraries(self):
        return list(filter(lambda libinfo: libinfo.get('enable') == True, self.libraries.values()))

    def add(self, name):
        repo = self.libraries[name].get('repo')
        if not repo: return

        repo_path = self.project_path + '/' + repo['path']

        try:
            print("Recipe:", repo)
            porcelain.clone(repo['url'], repo_path, depth=1, branch=repo['branch'])
        except FileExistsError as e:
            print("DEBUG:", e)

        r = Repo(repo_path)
        print("Repo:", r)

        refs = [k for k, v in r.get_refs().items() if v == r.head()]
        refs.append(r.head())
        print("Refs:", refs)

        branch_name = repo.get('branch').encode()
        matched = None
        chk = b'refs/heads/%s' % (branch_name,)
        if chk in refs:
            matched = chk
        chk = b'refs/tags/%s' % (branch_name,)
        if chk in refs:
            matched = chk

        if not matched:
            porcelain.checkout_branch(r, branch_name)

        # cfg = r.get_config()
        # secs = list(cfg.sections())
        # print("Git Sections:", secs)

    def apply_hooks(self):
        for libinfo in self.libraries.values():
            onhook = libinfo.get("onhook")
            if onhook is not None:
                ret = onhook(self, libinfo)

    def registers(self):
        self.useOpenGL()
        self.useOpenAL()
        self.useUnicornEngine()
        self.useKeystoneEngine()
        self.useCapstoneEngine()
        self.useSDL2()
        self.useImgui()
        self.useGlad()
        self.useGlm()
        self.useYaml()
        self.useEnet()
        self.useFmt()
        self.usePeParse()
        self.useGoogleTest()
        self.useArgh()
        self.useLibAssert()
        self.useJdksMidi()

    def useUnicornEngine(self):
        self.append(dict(
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

    def useKeystoneEngine(self):
        self.append(dict(
            name="keystone",
            path = "ext/keystone",
            link_libs = ["keystone"],
            include_dirs = ['${keystone_SOURCE_DIR}/include'],
            sets = [
                dict(name="LLVM_TARGETS_TO_BUILD", value="X86"),
                dict(name="BUILD_LIBS_ONLY", value=True),
            ],
            repo=dict(
                url='git@github.com:firodj/keystone.git',
                path='ext/keystone',
                branch='firodj'
            )
        ))

    def useCapstoneEngine(self):
        self.append(dict(
            name="capstone",
            path = "ext/capstone",
            sets = [
                dict(name="CAPSTONE_BUILD_TESTS", value=False),
                dict(name="CAPSTONE_BUILD_CSTOOL", value=False),
                dict(name="CAPSTONE_ARCHITECTURE_DEFAULT", value=False),
                dict(name="CAPSTONE_X86_SUPPORT", value=True),
                dict(name="BUILD_SHARED_LIBS", value=False),
            ],
            link_libs = ["capstone"],
            include_dirs = ['${capstone_SOURCE_DIR}/include'],
            repo=dict(
                url='https://github.com/capstone-engine/capstone.git',
                path='ext/capstone',
                branch='5.0.1'
            )
        ))

    def useSDL2(self):
        def onhook(mgr, libinfo):
            print("checking if imgui enabled, for sdl2")
            if mgr.is_enabled("imgui"):
                libinfo["sources"] = ['${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp']

        self.append(dict(
            name = "sdl2",
            include_dirs = ['${SDL2_SOURCE_DIR}/include'],
            sets = [
                dict(name="SDL_TEST", value=False),
                dict(name="SDL2_DISABLE_SDL2MAIN", value=True)
            ],
            link_libs = ["SDL2-static"],
            options = [],
            path = "ext/sdl2",
            repo = dict(
                url='https://github.com/libsdl-org/SDL.git',
                path='ext/sdl2',
                branch='release-2.30.2',
            ),
            onhook = onhook,
        ))

    def useOpenGL(self):
        self.append(dict(
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

    def useOpenAL(self):
        self.append(dict(
            name = "openal",
            requires = ['OpenAL'],
            include_dirs = [
                '${OPENAL_INCLUDE_DIR}',
            ],
            link_libs = [
                '${OPENAL_LIBRARY}',
            ],
        ))

    def useGlad(self):
        self.append(dict(
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

    def useGlm(self):
        self.append(dict(
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

    def useImgui(self):
        self.append(dict(
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
                branch='v1.90.5-docking',
            )
        ))

    def useYaml(self):
        self.append(dict(
            name = "yaml-cpp",
            path = "ext/yaml-cpp",
            include_dirs = [
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

    def useEnet(self):
        self.append(dict(
            name="enet",
            path="ext/enet",
            include_dirs=['${enet_SOURCE_DIR}/include'],
            link_libs = [
                "enet"
            ],
            defs = ["-DDPLAY_ENET"],
            repo=dict(
                url="https://github.com/lsalzman/enet.git",
                path="ext/enet",
                branch="v1.3.14",
            ),
            msvc = dict(
                link_libs = ["winmm", "ws2_32"],
                defs = ["-D_WINSOCKAPI_"]
            )
        ))

    def useFmt(self):
        self.append(dict(
            name="fmt",
            path="ext/fmt",
            include_dirs=['${fmt_SOURCE_DIR}/include'],
            link_libs = [
                "fmt"
            ],
            repo=dict(
                url="https://github.com/fmtlib/fmt.git",
                path="ext/fmt",
                branch="10.2.1",
            ),
        ))

    def usePeParse(self):
        self.append(dict(
            name="pe-parse",
            path="ext/pe-parse/pe-parser-library",
            link_libs = [
                "pe-parse::pe-parse"
            ],
            repo=dict(
                url="https://github.com/trailofbits/pe-parse.git",
                path="ext/pe-parse",
                branch="v2.1.1",
            ),
        ))

    def useGoogleTest(self):
        self.append(dict(
            name="googletest",
            path="ext/googletest",
            repo=dict(
                url="https://github.com/google/googletest.git",
                path="ext/googletest",
                branch="v1.14.0",
            ),
        ))

    def useArgh(self):
        self.append(dict(
            name="argh",
            path="ext/argh",
            sets = [
                dict(name="BUILD_TESTS", value=False),
                dict(name="BUILD_EXAMPLES", value=False),
            ],
            repo=dict(
                url="https://github.com/adishavit/argh",
                path="ext/argh",
                branch="v1.3.2",
            ),
        ))

    def useLibAssert(self):
        self.append(dict(
            name="libassert",
            path="ext/libassert",
            link_libs = [
                "libassert::assert"
            ],
            repo=dict(
                url="https://github.com/jeremy-rifkin/libassert.git",
                path="ext/libassert",
                branch="v2.0.1",
            ),
        ))

    def useJdksMidi(self):
        self.append(dict(
            name="jdksmidi",
            path="ext/jdksmidi",
            link_libs = [
                "jdksmidi"
            ],
            repo=dict(
                url="https://github.com/firodj/jdksmidi.git",
                path="ext/jdksmidi",
                branch="dev",
            ),
        ))
