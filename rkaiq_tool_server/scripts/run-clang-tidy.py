#!/usr/bin/env python

# Run clang-tidy recursively and parallel on directory
# Usage: run-clang-tidy sourcedir builddir excludedirs extensions
# extensions and excludedirs are specified as comma-separated
# string without dot, e.g. 'c,cpp'
# e.g. run-clang-tidy . build test,other c,cpp file

import os, sys, subprocess, multiprocessing

manager = multiprocessing.Manager()
failedfiles = manager.list()

# Get absolute current path and remove trailing seperators
currentdir = os.path.realpath(os.getcwd()).rstrip(os.sep)
print("Arguments: " + str(sys.argv))
# Get absolute source dir after removing leading and trailing seperators from input.
sourcedir = currentdir + sys.argv[1].lstrip(os.sep).rstrip(os.sep)
print("Source directory: " + sourcedir)
builddir = sourcedir + os.sep + sys.argv[2].rstrip(os.sep)
print("Build directory: " + builddir)
# If exclude dirs is not empty, split it into a tuple
excludedirs = ()
if sys.argv[3]:
    excludedirs = tuple([(sourcedir + os.sep + s).rstrip(os.sep)
                         for s in sys.argv[3].split(',')])
# If the build directory is not the same as the source directory, exclude it
if not sourcedir == builddir:
    excludedirs = excludedirs + (builddir, )
print("Exclude directories: " + str(excludedirs))
# Split extensions into a tuple
extensions = tuple([("." + s) for s in sys.argv[4].split(',')])
print("Extensions: " + str(extensions))


def runclangtidy(filepath):
    print("Checking: " + filepath)
    proc = subprocess.Popen("clang-tidy --quiet -p=" + builddir + " " +
                            filepath,
                            shell=True)
    if proc.wait() != 0:
        failedfiles.append(filepath)


def collectfiles(dir, exclude, exts):
    collectedfiles = []
    for root, dirs, files in os.walk(dir):
        for file in files:
            filepath = root + os.sep + file
            if (len(exclude) == 0 or not filepath.startswith(exclude)
                ) and filepath.endswith(exts):
                collectedfiles.append(filepath)
    return collectedfiles


# Define the pool AFTER the global variables and subprocess function because WTF python
# See: https://stackoverflow.com/questions/41385708/multiprocessing-example-giving-attributeerror
pool = multiprocessing.Pool()
pool.map(runclangtidy, collectfiles(sourcedir, excludedirs, extensions))
pool.close()
pool.join()
if len(failedfiles) > 0:
    print("Errors in %d files"  % len(failedfiles))
    sys.exit(1)
print("No errors found")
sys.exit(0)
