import os
import stat
import jinja2
import recipes
import configparser
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
others  = []

libmgr = recipes.LibraryManager(project_path)
jenv = jinja2.Environment(
    loader=jinja2.FileSystemLoader(current_path + "/templates")
)

def value_format(value):
    if type(value) is bool:
        return "ON" if value else "OFF"
    return '"' + value + '"'

jenv.filters["value_format"] = value_format

def createCMake():
    print(project_path, project_name)

    template = jenv.get_template("CMakeLists.txt.jinja")
    libs = libmgr.enable_libraries()

    msvc = dict(
        defs = [],
        link_libs = [],
    )
    for lib in libs:
        lib_msvc = lib.get('msvc')
        if lib_msvc is not None:
            msvc_defs = lib_msvc.get('defs')
            msvc_link_libs = lib_msvc.get('link_libs')
            if msvc_link_libs:
                msvc['link_libs'] += msvc_link_libs
            if msvc_defs:
                msvc['defs'] += msvc_defs

    contents = template.render(
        cmake_min_version=cmake_min_version,
        project_name=project_name,
        cpp_std=cpp_std,
        c_std=c_std,
        libraries=libs,
        sources=sources,
        headers=headers,
        others=others,
        msvc=msvc,
    )

    with open(os.path.join(project_path, 'CMakeLists.txt'), 'w') as f:
        f.write(contents)

def createStarter():
    cmakelists_path = os.path.join(project_path, 'CMakeLists.txt')
    if os.path.exists(cmakelists_path): return

    try:
        os.makedirs(os.path.join(project_path, 'src'))
    except FileExistsError as e:
        print("DEBUG:", e)

    sources.append("src/MainApp.cpp")

    mainapp_cpp_path = os.path.join(project_path, 'src/MainApp.cpp')
    if not os.path.exists(mainapp_cpp_path):
        template = jenv.get_template("MainApp.cpp.jinja")
        contents = template.render(project_name=project_name,)
        with open(mainapp_cpp_path, 'w') as f:
            f.write(contents)


    mainapp_hpp_path = os.path.join(project_path, 'src/MainApp.hpp')
    if not os.path.exists(mainapp_hpp_path):
        template = jenv.get_template("MainApp.hpp.jinja")
        contents = template.render()
        with open(mainapp_hpp_path, 'w') as f:
            f.write(contents)

    headers.append("src/MainApp.hpp")

def checkRunPy():
    run_py_path = os.path.join(project_path, 'run.py')
    if not os.path.exists(run_py_path):
        template = jenv.get_template("run.py.jinja")
        contents = template.render(project_name=project_name,)
        with open(run_py_path, 'w') as f:
            f.write(contents)

        st = os.stat(run_py_path)
        os.chmod(run_py_path, st.st_mode | stat.S_IEXEC)

def checkExt():
    ext_path = project_path + '/ext'

    if not os.path.exists(ext_path):
        os.mkdir(ext_path)

    # for extpath in os.listdir(ext_path):
    #     try:
    #         r = Repo(ext_path + '/' + extpath)
    #         cfg = r.get_config()
    #         secs = list(cfg.sections())
    #         for s in secs:
    #             print(s[0])
    #             print(s, list(cfg.items(secs[1])))
    #         print(cfg.get(("remote", "origin"), "url"))
    #     except NotGitRepository as e:
    #         print(extpath, e)

def checkGitIgnore():
    try:
        contents = ["build-*/", "*.dSYM/", "ext/",
            project_name, project_name + ".exe",
            project_name + "d", project_name + "d.exe"
        ]

        with open(os.path.join(project_path, '.gitignore'), 'r') as f:
            for line in f.readlines():
                line = line.strip()
                if line.startswith("#"): continue

                if line in contents:
                    contents.remove(line)
                    if len(contents) == 0: break

        if len(contents) > 0:
            with open(os.path.join(project_path, '.gitignore'), 'a') as f:
                f.write("\n".join(contents))

    except FileNotFoundError as e:
        print("DEBUG:", e)

def getExistingSources():
    try:
        with open(os.path.join(project_path, 'CMakeLists.txt'), 'r') as f:
            capture = None
            for line in f.readlines():
                lineb4 = line.rstrip()
                line = line.strip()
                if line.startswith("## -- sources"):
                    capture = "sources"
                elif line.startswith("## -- headers"):
                    capture = "headers"
                elif line.startswith("## -- others"):
                    capture = "others"
                elif line.startswith("## -- end"):
                    capture = None
                else:
                    match capture:
                        case "sources":
                            sources.append(line)
                        case "headers":
                            headers.append(line)
                        case "others":
                            others.append(lineb4)

    except FileNotFoundError as e:
        print("DEBUG:", e)

def checkConfig():
    config = configparser.ConfigParser()
    cfgpath = os.path.join(project_path, 'kosongg.ini')
    if not os.path.exists(cfgpath):
        template = jenv.get_template("kosongg.ini.jinja")
        contents = template.render()
        with open(cfgpath, 'w') as f:
            f.write(contents)
    config.read(cfgpath)

    adds = []
    for k in config['DEFAULT'].keys():
        if config['DEFAULT'].getboolean(k):
            libmgr.enable(k)
            adds.append(k)
    for k in adds:
        libmgr.add(k)
    libmgr.apply_hooks()

if __name__ == '__main__':
    checkExt()
    checkConfig()
    getExistingSources()
    createStarter()
    createCMake()
    checkRunPy()
    checkGitIgnore()

# cd output/ext
# git clone --branch=mob --depth=1 git://repo.or.cz/tinycc.git