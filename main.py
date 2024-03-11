import os
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
                            others.append(line)

    except FileNotFoundError as e:
        print(e)

def checkConfig():
    config = configparser.ConfigParser()
    config.read(os.path.join(project_path, 'kosongg.ini'))

    adds = []
    for k in config['DEFAULT'].keys():
        if config['DEFAULT'].getboolean(k):
            libmgr.enable(k)
            adds.append(k)
    for k in adds:
        libmgr.add(k)

if __name__ == '__main__':
    checkConfig()
    getExistingSources()
    createCMake()

# cd output/ext
# git clone --branch=mob --depth=1 git://repo.or.cz/tinycc.git