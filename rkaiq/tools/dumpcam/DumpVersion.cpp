/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cassert>
#include <unistd.h>
#include <stdio.h>
#include "stringprintf.h"
#include "binutils.h"
#include "DumpDefine.h"
#include "DumpLog.h"
#include "DumpVersion.h"

const unsigned DEF_LIB_PATH = 8;
const unsigned MAX_LIB_PATH = 30;
const unsigned MAX_LIB_PATH_NAME = 1024;
char sLibCommPath[MAX_LIB_PATH][MAX_LIB_PATH_NAME];
char sDefaultLibPath[DEF_LIB_PATH][MAX_LIB_PATH_NAME] = {
    "/lib/",          // linux
    "/lib64/",        // linux
    "/oem/usr/lib",   // linux
    "/usr/lib",       // linux
    "/vendor/lib",    // android
    "/system/lib",    // android
};

static const char* dumpVersionStr = "v2.2";
static const char* rkaiqName = "librkaiq";
static const char* appendSo = ".so";
static const char* appendSo1 = ".so.1";
static const char* appendSo2 = ".so.2";

DumpVersion::DumpVersion() {
    showlist = false;
}

DumpVersion::~DumpVersion() {
}

int DumpVersion::dumpModuleVersion() {
    char libPath[1024] = {0};
    char exeCMD[1024] = {0};
    int result = DUMP_SUCCESS;
    std::string versionStr;
    getLibPath();
    dumpDumpVersion(versionStr);
    dumpRkAiqVersion(versionStr);
    if (!WriteFully(STDOUT_FILENO, versionStr.c_str(), versionStr.size())) {
        result = DUMP_FAIL;
        DEBUG_MSG("Failed to write while dumping version \n");
    }
    return result;
}

int DumpVersion::dumpRkAiqVersion(std::string &versionStr) {
    int result = DUMP_SUCCESS;
    bool isPrinted = false;
    char libPath[1024] = {0};
    char exeCMD1[1024] = {0};
    char exeCMD2[1024] = {0};
    const char *searchString1 = "AIQ v";
    writeVersionHead(rkaiqName, versionStr);
    for (int j = 0; j < (int)ARRAY_ELEMS(sLibCommPath); j++) {
        Snprintf(libPath, sizeof(libPath), "%s/%s%s", sLibCommPath[j], rkaiqName, appendSo);
        if (!access(libPath, R_OK)) {
            if ((getVersionStrings(libPath, searchString1, versionStr) > 0)) {
                isPrinted  = true;
            } else {
                continue;
            }
        } else {
            continue;
        }

        if (isPrinted) {
            break;
        }
    }
    if (!isPrinted) {
        result = DUMP_FAIL;
        DEBUG_MSG("dump:%s version failed \n", rkaiqName);
    }
    versionStr.append("\n");
    return result;
}

int DumpVersion::dumpDumpVersion(std::string &versionStr) {
    char exeCMD[1024] = {0};
    int result = DUMP_SUCCESS;
    versionStr.append("dumpsys version:");
    versionStr.append(dumpVersionStr);
    versionStr.append("\n");
    versionStr.append("\n");
    return result;
}

int DumpVersion::getLibPath() {
    int result = DUMP_SUCCESS;
    int pathIndex = 0;
    int pathStart = 0;
    char exeCMD[1024] = {0};
    char *libPath;
    char exportPath[4096];
    char exportPathStr[4096];
    std::string libPathStr;
    Snprintf(exeCMD, sizeof(exeCMD), "export  | grep LD_LIBRARY_PATH");
    if (executeCMD(exeCMD, libPathStr) > 0) {
        strncpy(exportPath, libPathStr.c_str(), libPathStr.size());
        const char *deliStr = "'";
        const char *deliPathStr = ":";
        char *leftStr = NULL;
        char *pathStr;
        pathStr = strtok_r(exportPath, deliStr, &leftStr);
        if (pathStr) {
            pathStr = strtok_r(NULL, deliStr, &leftStr);
            Snprintf(exportPathStr, sizeof(exportPathStr), "%s", pathStr);
        }
        leftStr = NULL;
        pathStr = strtok_r(exportPathStr, deliPathStr, &leftStr);
        while (pathStr) {
            Snprintf(sLibCommPath[pathIndex++], sizeof(sLibCommPath[pathIndex]), "%s", pathStr);
            pathStr = strtok_r(NULL, deliPathStr, &leftStr);
        }
    }

    for (int i = 0; i < (int)ARRAY_ELEMS(sDefaultLibPath); i++) {
        Snprintf(sLibCommPath[pathIndex++], sizeof(sLibCommPath[pathIndex]), "%s", sDefaultLibPath[i]);
    }
    return result;
}

int DumpVersion::getVersionStrings(const char *libPath, const char *searchString, std::string &versionStr) {
    int result = DUMP_SUCCESS;
    char exeCMD[1024] = {0};
    result = executeStrings(libPath, searchString, versionStr);
    if (result == DUMP_FAIL) {
        Snprintf(exeCMD, sizeof(exeCMD), "strings %s | grep %s ", libPath, searchString);
        result = executeCMD(exeCMD, versionStr, showlist);
    }
    return result;
}

int DumpVersion::executeStrings(const char *libPath, const char *searchString, std::string &versionStr) {
    int ret = DUMP_SUCCESS;
    ret = strings(libPath, searchString, versionStr, showlist);
    return ret;
}

int DumpVersion::writeVersionHead(const char *module, std::string &versionStr) {
    char moduleHead[1024] = {0};
    Snprintf(moduleHead, sizeof(moduleHead), "----------------- %s version -----------------\n", module);
    versionStr.append(moduleHead);
    return DUMP_SUCCESS;
}

int DumpVersion::dumpSetDumpList(bool listEnable) {
    showlist = listEnable;
    return DUMP_SUCCESS;
}
