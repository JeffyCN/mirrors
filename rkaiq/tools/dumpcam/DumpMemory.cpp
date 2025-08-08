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

#include "stringprintf.h"
#include "binutils.h"
#include "DumpDefine.h"
#include "DumpLog.h"
#include "DumpMemory.h"

static const char* dumpVersionStr = "v2.0";
static const char* cpuFreqName = " cpu freq ";
static const char* vpuFreqName = " vpu freq ";
static const char* gpuFreqName = " gpu freq ";
static const char* gpuUtilsName = " gpu utils ";
static const char* rgaFreqName = " rga freq ";
static const char* memoryFree = " memory info ";
static const char* memoryDump = " memory dump ";

DumpMemory::DumpMemory() {
    showlist = false;
}

DumpMemory::~DumpMemory() {
}

int DumpMemory::dumpModuleFreq() {
    int result = DUMP_SUCCESS;
    std::string freqStr;
    dumpCpuFreq(freqStr);
    dumpVpuFreq(freqStr);
    dumpGpuFreq(freqStr);
    dumpRgaFreq(freqStr);
    if (!WriteFully(STDOUT_FILENO, freqStr.c_str(), freqStr.size())) {
        result = DUMP_FAIL;
        DEBUG_MSG_ERROR("Failed to write while dumping memory \n");
    }
    return result;
}

int DumpMemory::dumpCpuFreq(std::string &freqStr) {
    char exeCMD[1024] = {0};
    int result = DUMP_SUCCESS;
    writeStrHead(cpuFreqName, freqStr);
    Snprintf(exeCMD, sizeof(exeCMD), "cat /sys/devices/system/cpu/cpu1/cpufreq/scaling_cur_freq");
    if (executeCMD(exeCMD, freqStr) > 0) {
        result  = DUMP_FAIL;
    }
    freqStr.append("\n");
    return result;
}

int DumpMemory::dumpVpuFreq(std::string &freqStr) {
    char exeCMD[1024] = {0};
    int result = DUMP_SUCCESS;
    writeStrHead(vpuFreqName, freqStr);
    Snprintf(exeCMD, sizeof(exeCMD), "cat /sys/kernel/debug/clk/clk_summary | grep vdec");
    if (executeCMD(exeCMD, freqStr) <= 0) {
        result  = DUMP_FAIL;
    }
    freqStr.append("\n");
    return result;
}

int DumpMemory::dumpGpuFreq(std::string &freqStr) {
    char exeCMD[1024] = {0};
    int result = DUMP_SUCCESS;

    writeStrHead(gpuFreqName, freqStr);
    Snprintf(exeCMD, sizeof(exeCMD), "cat /sys/kernel/debug/clk/clk_scmi_gpu/clk_rate");
    if (executeCMD(exeCMD, freqStr) <= 0) {
        result  = DUMP_FAIL;
    }
    freqStr.append("\n");

    writeStrHead(gpuUtilsName, freqStr);
    Snprintf(exeCMD, sizeof(exeCMD), "cat /sys/devices/platform/fde60000.gpu/utilisation");
    if (executeCMD(exeCMD, freqStr) <= 0) {
        result  = DUMP_FAIL;
    }
    freqStr.append("\n");
    return result;
}

int DumpMemory::dumpRgaFreq(std::string &freqStr) {
    char exeCMD[1024] = {0};
    int result = DUMP_SUCCESS;
    writeStrHead(rgaFreqName, freqStr);
    Snprintf(exeCMD, sizeof(exeCMD), "cat /sys/kernel/debug/clk/clk_summary | grep -E rga");
    if (executeCMD(exeCMD, freqStr) <= 0) {
        result  = DUMP_FAIL;
    }
    freqStr.append("\n");
    return result;
}

int DumpMemory::dumpMemoryInfo() {
    int result = DUMP_SUCCESS;
    std::string memoryStr;
    dumpMemoryFree(memoryStr);
    dumpMemoryMMdump(memoryStr);
    if (!WriteFully(STDOUT_FILENO, memoryStr.c_str(), memoryStr.size())) {
        result = DUMP_FAIL;
        DEBUG_MSG_ERROR("Failed to write while dumping memory \n");
    }
    return result;
}

int DumpMemory::dumpMemoryMMdump(std::string &memoryStr) {
    char exeCMD[1024] = {0};
    char memStr[1024] = {0};
    int result = DUMP_SUCCESS;
    int mmdumpNumber = 0;
    std::string memoryStrTemp;
    writeStrHead(memoryDump, memoryStr);

    // total
    Snprintf(exeCMD, sizeof(exeCMD), "cat sys/kernel/debug/dri/0/mm_dump | grep total");
    mmdumpNumber = executeCMD(exeCMD, memoryStr);
    if (mmdumpNumber <= 0) {
        result  = DUMP_FAIL;
        return result;
    }


    // used
    Snprintf(exeCMD, sizeof(exeCMD), "cat sys/kernel/debug/dri/0/mm_dump | grep used");
    mmdumpNumber = executeCMD(exeCMD, memoryStrTemp);
    if (mmdumpNumber <= 0) {
        result  = DUMP_FAIL;
        return result;
    }
    Snprintf(memStr, sizeof(memStr), "used number : %-20d", mmdumpNumber - 1);
    memoryStr.append(memStr);

    // free
    Snprintf(exeCMD, sizeof(exeCMD), "cat sys/kernel/debug/dri/0/mm_dump | grep free");
    mmdumpNumber = executeCMD(exeCMD, memoryStrTemp);
    if (mmdumpNumber <= 0) {
        result  = DUMP_FAIL;
        return result;
    }
    Snprintf(memStr, sizeof(memStr), " free number : %-20d", mmdumpNumber - 1);
    memoryStr.append(memStr);
    memoryStr.append("\n");
    return result;
}

int DumpMemory::dumpMemoryFree(std::string &memoryStr) {
    char exeCMD[1024] = {0};
    int result = DUMP_SUCCESS;
    writeStrHead(memoryFree, memoryStr);
    Snprintf(exeCMD, sizeof(exeCMD), "free");
    if (executeCMD(exeCMD, memoryStr) <= 0) {
        result  = DUMP_FAIL;
    }
    memoryStr.append("\n");
    return result;
}


int DumpMemory::writeStrHead(const char *module, std::string &writeStr) {
    char moduleHead[1024] = {0};
    Snprintf(moduleHead, sizeof(moduleHead), "----------------- %s -----------------\n", module);
    writeStr.append(moduleHead);
    return DUMP_SUCCESS;
}


