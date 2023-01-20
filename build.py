import os, os.path, glob, fnmatch, subprocess, sys
import argparse, filecmp, shutil
from datetime import datetime
from datetime import date
import logging

logging.basicConfig(format='%(asctime)s %(pathname)s[line:%(lineno)d] %(levelname)s: %(message)s',
                    datefmt='%Y-%m-%d %H:%M:%S', level=logging.DEBUG)

# 检查文件的函数
def check_file(cf):
    if not os.path.isfile(cf):
        logging.error("FAIL: cannot find file {}".format(cf))
        sys.exit("Aborted!")

# 判断是否是一个路径
def check_dir(cdir):
    if not os.path.exists(cdir):
        logging.error("FAIL: cannot find folder {}".format(cdir))
        sys.exit("Aborted!")

#returns true if myname is contained in components to build
def build_me(components, myname):
    if "all" in components or myname in components:
        return True
    return False

def test_it(projRoot):
    logging.info("test integration start")
    subprocess.call("{}/output/skynet {}/test/integration/test_app.conf".format(projRoot, projRoot), shell=True)

#main entry
if __name__ == '__main__': # 因此通过判断__name__的值，就可以区分py文件是直接被运行，还是被引入其他程序中 https://blog.csdn.net/wosind/article/details/90728198
    scriptDir = os.path.dirname(os.path.realpath(__file__)) # 给出脚本的路径
    # 可以通过根据脚本的路径相对于项目的路径设置项目路径，如下面，这里我们设置项目根目录为脚本所在的文件夹
    # projRoot = os.path.abspath(os.path.join(scriptDir, "..", "..", "..")) # 给出项目的路径，所以，脚本和项目的相对位置要设置好
    projRoot = os.path.abspath(os.path.join(scriptDir)) # 给出项目的路径，所以，脚本和项目的相对位置要设置好

    # logging.info("scriptDir={}, projRoot={}".format(scriptDir, projRoot))

    parser = argparse.ArgumentParser() # 解析命令
    parser.add_argument('--type', help='specify build type as Release or Debug', default='debug')
    parser.add_argument('--bt', help='specify the test type')
    args = parser.parse_args()


    # 确定build的类型
    buildType="Release"
    if (args.type == "Release") or (args.type == "release") or (args.type == "RELEASE"):
        buildType = "Release"
    elif (args.type == "Debug") or (args.type == "debug")or (args.type == "DEBUG"):
        buildType = "Debug"
    else:
        logging.error("Unrecognized build type {}".format(args.type))
        sys.exit("Aborted!")

    if (args.bt == "dt"):
        test_it(projRoot)
        logging.info("test integration end")
        exit(0)

    logging.info("Start to build project...")

    # 给出一些变量定义
    archSLTN = ""
    if os.name == "posix": # posix指的是linux/MAC
        if buildType == "Release":
            suffix = "_opt" # 后缀
        elif buildType == "Debug":
            suffix = "_dbg"
        elif buildType == "Coverage":
            suffix = "_dbg_cov"
        else:
            suffix = "_opt_cov"

        installSubDir = "linux64" + suffix # 给出install目录
        buildInstallCMD = "make -j4 && make install" # 给出build的脚本命令
        # buildTestCMD = "make tests" # 给出test的命令,如有必要的话
        cmakeSLTN = "Unix Makefiles" # cmake使用的generator类型
        #compilers
        #compilerPath = "/usr/local"      #gcc 6.1
        compilerPath = "/usr"   #gcc 9.3 #给出gcc路径
        cCompiler = compilerPath + "/bin/gcc" # gcc位置
        cxxCompiler = compilerPath + "/bin/g++"# g++位置
        makeCmd = "/usr/local/bin/make" # make的位置
        if not os.path.isfile(makeCmd):# 如果make不再‘makeCmd’中的话
            makeCmd = "/usr/bin/make" # 认为其在此处
        check_file(makeCmd) # 判断其是不是file
    else:
        temp="other paltforms"
        # 这里可以设置其他平台如windows平台的生成等等

    cmakeSLTN = "\"" + cmakeSLTN + "\"" + archSLTN # 添加双引号
    buildDir = os.path.join(projRoot, "build") # 添加builds目录
    sourceDir = os.path.join(projRoot) # 确定CMakeLists.txt文件目录
    ProjectSourceDir = os.path.join(sourceDir)
    if os.path.exists(buildDir):
            shutil.rmtree(buildDir)
    os.makedirs(buildDir) # 创建builds的子目录

    # 检查目录，这些目录需要提前创建好
    check_dir(buildDir)
    check_dir(ProjectSourceDir)

    logging.info("project dir:{}".format(projRoot))
    logging.info("build type:{}".format(buildType))
    logging.info("----------------------------------------------------------")
    try:

        # 这个方法的作用是执行一些命令行的命令，例如 sh xxx.sh , java -jar xxx.jar 等，(相当于执行shell命令？)
        # 会开启一个子进程去执行，并且等待子进程结束才继续执行其他的，使用起来非常方便，就是需要注意一些小细节
        subprocess.call(['cmake', '--version']) # subprocess 模块允许我们启动一个新进程，并连接到它们的输入/输出/错误管道，从而获取返回值。
        # subprocess.call(['pwd'])
    except OSError:
        logging.error("cannot find cmake tool.")
        sys.exit("Aborted!")

    configCMD = "cmake -D CMAKE_INSTALL_PREFIX:PATH=" + projRoot
    if os.name == "posix":
        configCMD = configCMD + " -D CMAKE_BUILD_TYPE:STRING=" + buildType # build类型
        configCMD = configCMD + " -D CMAKE_C_COMPILER:PATH=" + cCompiler # c编译器位置
        configCMD = configCMD + " -D CMAKE_CXX_COMPILER:PATH=" + cxxCompiler # c++编译器位置

    #-------------------------------------------------------------
    os.chdir(buildDir)
    if 1: # 可以作为判断生成哪个项目的入口
        # 判断是否存在此目录
        if not (os.path.exists(ProjectSourceDir)):
            sys.exit("multife source folder does not exist!")

        # 结合cmake的-D指令给出build的额外指令
        proCMD = configCMD # 给出build的命令

        proCMD = proCMD + " -G " + cmakeSLTN
        proCMD = proCMD + " " + ProjectSourceDir # 给出source的目录
        pro1CMD="cmake ../ "

        subprocess.call(proCMD, shell=True) #启动shell脚本，执行proCMD指令，即‘cmake xxx‘指令
        subprocess.call(buildInstallCMD, shell=True) # 即'make xxx'指令

    logging.info("----------------------------------------------------------")
    logging.info("----------build successfully------------")
