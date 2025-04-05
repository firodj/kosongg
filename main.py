import os
import stat
import jinja2
import recipes
import configparser
from dulwich.repo import Repo, NotGitRepository
from dotenv import load_dotenv
load_dotenv()
EXT_PATH = os.environ.get('EXT_PATH')

cmake_min_version = '3.20'
project_name = 'cobacoba'
cpp_std='17'
c_std='17'
output_path = '..'
bundle_reverse_domain = 'org.kosongg'

current_path = os.path.dirname(__file__)
project_path = os.path.normpath(os.path.join(current_path, '../'))
project_name = os.path.basename(project_path)

def find_ext_path(path):
    orig_path = path
    while True:
        path = os.path.dirname(path)
        if len(path) > 0:
            p = os.path.basename(path)
            if (not p.startswith("@")):
                break
        else:
            path = orig_path
    return path.replace('\\', '/') + '/ext'

if EXT_PATH:
    ext_path = EXT_PATH
else:
    ext_path = find_ext_path(project_path)

ext_path_rel = os.path.relpath(ext_path, project_path)
print("DEBUG: ext_path = ", ext_path, ext_path_rel)

sources = []
headers = []
others  = []
commands = []

libmgr = recipes.LibraryManager(project_path, ext_path)
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
    libs = libmgr.get_enabled_libraries()

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
        bundle_reverse_domain=bundle_reverse_domain,
    )

    with open(os.path.join(project_path, 'CMakeLists.txt'), 'w') as f:
        f.write(contents)

def copyTemplate(src, dst, *args, **kwargs):
    force = kwargs.pop('force', False)
    dst_path = os.path.join(project_path, dst)
    exists = os.path.exists(dst_path)

    if not exists or force:
        template = jenv.get_template(src)
        contents = template.render(*args, **kwargs)
        with open(dst_path, 'w') as f:
            f.write(contents)

    if exists:
        return dst_path

def createStarter():
    cmakelists_path = os.path.join(project_path, 'CMakeLists.txt')
    if os.path.exists(cmakelists_path): return

    try:
        os.makedirs(os.path.join(project_path, 'src'))
    except FileExistsError as e:
        print("DEBUG:", e)

    try:
        os.makedirs(os.path.join(project_path, 'src', 'watched'))
    except FileExistsError as e:
        print("DEBUG:", e)

    copyTemplate("Globals.hpp.jinja", "src/Globals.hpp")

    sources.append("src/MainApp.cpp")
    headers.append("src/MainApp.hpp")

    copyTemplate("MainApp.cpp.jinja", "src/MainApp.cpp", project_name=project_name)
    copyTemplate("MainApp.hpp.jinja", "src/MainApp.hpp")

    sources.append("src/watched/MainWidget.cpp")
    headers.append("src/watched/MainWidget.hpp")

    copyTemplate("MainWidget.cpp.jinja", "src/watched/MainWidget.cpp")
    copyTemplate("MainWidget.hpp.jinja", "src/watched/MainWidget.hpp")


def checkRunPy():
    dst = copyTemplate("run.py.jinja", "run.py",
            project_name=project_name,
            commands=commands,
            force=True,
            )
    if dst:
        st = os.stat(dst)
        os.chmod(dst, st.st_mode | stat.S_IEXEC)

def checkInfoPlist():
    copyTemplate("Info.plist.in.jinja", "Info.plist.in",
            project_name=project_name,
            bundle_reverse_domain=bundle_reverse_domain,
        )

def checkExt():
    copyTemplate(".env.jinja", '.env', ext_path=ext_path)
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
        contents = ["build-*/", "*.dSYM/", "ext/", ".venv", ".DS_Store"]

        with open(os.path.join(project_path, '.gitignore'), 'r') as f:
            for line in f.readlines():
                line = line.strip()
                if line.startswith("#"): continue

                if line in contents:
                    contents.remove(line)
                    if len(contents) == 0: break
    except FileNotFoundError as e:
        print("DEBUG:", e)

    if len(contents) > 0:
        with open(os.path.join(project_path, '.gitignore'), 'a') as f:
            f.write("\n".join(contents))

def getExistingSources(filename):
    try:
        with open(os.path.join(project_path, filename), 'r') as f:
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
                elif line.startswith("## -- commands"):
                    capture = "commands"
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
                        case "commands":
                            commands.append(lineb4)

    except FileNotFoundError as e:
        print("DEBUG:", e)

def checkConfig():
    global bundle_reverse_domain
    config = configparser.ConfigParser()
    cfgpath = copyTemplate("kosongg.ini.jinja", 'kosongg.ini')
    config.read(cfgpath)

    adds = []
    for k in config['DEFAULT'].keys():
        if config['DEFAULT'].getboolean(k):
            libmgr.enable(k)
            adds.append(k)
    for k in adds:
        print("DEBUG:", k)
        libmgr.add(k)
    libmgr.apply_hooks()

    bundle_reverse_domain = config['BUNDLE'].get('reverse_domain')

def checkVscode():
    vscode_path = os.path.join(project_path, '.vscode')
    if not os.path.exists(vscode_path):
        os.mkdir(vscode_path)

    copyTemplate("c_cpp_properties.json.jinja", '.vscode/c_cpp_properties.json', project_name=project_name)
    copyTemplate("launch.json.jinja", '.vscode/launch.json', project_name=project_name)


if __name__ == '__main__':
    checkExt()
    checkConfig()
    getExistingSources('CMakeLists.txt')
    getExistingSources('run.py')
    createStarter()
    createCMake()
    checkRunPy()
    checkInfoPlist()
    checkGitIgnore()
    checkVscode()

# cd output/ext
# git clone --branch=mob --depth=1 git://repo.or.cz/tinycc.git