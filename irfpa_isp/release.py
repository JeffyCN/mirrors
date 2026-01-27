#!/usr/bin/python3
# -*- coding: UTF-8 -*-

import sys
import os

ext_dir = "/home/ydb/projects/rv1126bipc/media/isp/camera_engine_rkaiq/irfpa_isp"

def remove_source(root, algo):
    algo_dir = os.path.join(root, "rk_irfpa/algos/%s" % algo)

    items = os.listdir(algo_dir)
    for item in items:
        if item.endswith(".cpp"):
            os.remove(os.path.join(algo_dir, item))
        if item.endswith(".h") and not item.startswith("rk_irfpa_"):
            os.remove(os.path.join(algo_dir, item))

def copy_build_lib(root, algo):
    algo_dir = os.path.join(root, "rk_irfpa/algos/%s" % algo)
    libfile = os.path.join(root, "build_arm/rk_irfpa/algos/%s/lib_rk_irfpa_%s_arm.a" % (algo, algo))
    libfile64 = os.path.join(root, "build_arm64/rk_irfpa/algos/%s/lib_rk_irfpa_%s_aarch64.a" % (algo, algo))
    assert os.path.isfile(libfile), "%s: static lib 32 not found!" % algo
    assert os.path.isfile(libfile64), "%s: static lib 64 not found!" % algo
    os.system("cp %s %s/" %(libfile, algo_dir))
    os.system("cp %s %s/" %(libfile64, algo_dir))

def remove_lib_file(root, algo):
    libfile = os.path.join(root, "rk_irfpa/algos/%s/lib_rk_irfpa_%s.a" % (algo, algo))
    os.system("rm %s" %(libfile))

def generate_version(root, rev):
    rev_file = os.path.join(root, "rk_irfpa/common/rk_irfpa_version.h")
    f = open(rev_file, "w")
    f.write('#pragma once\n')
    f.write('#define RK_IRFPA_GIT_REV "%s"' % rev)
    f.close()

script_path = os.path.realpath(__file__)
sdk_dir = os.path.dirname(script_path)
algos_dir = os.path.join(sdk_dir, "rk_irfpa/algos")

# scan for all algos
algos = []
items = os.listdir(algos_dir)
for item in items:
    if os.path.isdir(os.path.join(algos_dir, item)) and (item.startswith("hw_") or item.startswith("sw_")):
        algos.append(item)

os.chdir(sdk_dir)

git_status = os.popen("git status").read()
#if "working tree clean" not in git_status:
#    print("git not clean, please commit your changes first")
    #sys.exit()

git_head = os.popen("git rev-parse --short HEAD").read().strip()
print("git head ", git_head)

generate_version(sdk_dir, git_head)

if len(sys.argv) == 1:
    sys.exit()

if sys.argv[1] == "build":
    print("running cmake ..")
    os.system("cmake -S . -B build_arm -DOPTION_BUILD_STATIC=ON -DOPTION_BUILD_DEMO=OFF")

    print("build static libs 32 ..")
    os.system("cmake --build build_arm")

    print("running cmake ..")
    os.system("cmake -S . -B build_arm64 -DOPTION_BUILD_STATIC=ON -DOPTION_BUILD_DEMO=OFF -DARCH=aarch64")

    print("build static libs 64 ..")
    os.system("cmake --build build_arm64")
    #for algo in algos:
    #    print("copy lib --> ", algo)
    #    copy_build_lib(sdk_dir, algo)

if sys.argv[1] == "copy":
    os.system("rm -rf %s" %(ext_dir))
    os.system("cp -r %s %s" %(sdk_dir, ext_dir))

    for algo in algos:
        print("copy lib -->", algo)
        remove_source(ext_dir, algo)
        copy_build_lib(ext_dir, algo)

    os.system("rm -rf %s" %(os.path.join(ext_dir, "build_arm")))
    os.system("rm -rf %s" %(os.path.join(ext_dir, "build_arm64")))
