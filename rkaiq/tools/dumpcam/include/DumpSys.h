/*
 * Copyright 2020 Rockchip Electronics Co. LTD
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
 *
 */

#ifndef DUMP_SYS_H_
#define DUMP_SYS_H_

#include <thread>
#include <vector>
#include <string>
#include <getopt.h>
#include "DumpClient.h"
#include "DumpVersion.h"
#include "DumpMemory.h"

#define MAX_MODULE_NAME            256
#define MODULE_NAME_MESSAGE_ID     5
#define MAX_LINE_LENGTH            1024
#define DUMP_VERSION               "2021.3.10"
#define SOCKET_DOMAIN_NAME         "domain:///tmp/rkaiq_sock"
#define SOCKET_TCP_NAME            "tcp://127.0.0.1:4894"

typedef struct KEY_VALUE_PAIR_S {
    std::string key;
    std::string value;
} KEY_VALUE_PARIR;

class Dumpsys {
 public:
    Dumpsys();
    ~Dumpsys();
    int start(int argc, char* const argv[]);
    bool InitDumpClient();
    bool DumpClientSendModule();
    bool DumpClientReceiveDump();
    bool generateJson(std::vector<std::string > &service, std::vector<std::string > &args);
    bool writeDump(int fd, const char * moduleName);
    bool writeDumpHeader(int fd, const char * moduleName);
    bool writeDumpFooter(int fd, const char * moduleName);
    bool writeDumpToStd();

 public:
    DumpClient      *dumpClient;
    DumpVersion     *dumpVersion;
    DumpMemory      *dumpMemory;
    char            *dumpData;
    char            moduleName[MAX_MODULE_NAME];
    uint32_t        dumpSize;
    uint32_t        dumpId;
    std::string     jsonBuffer;
    std::string     servicerUri;
    bool            dumpService;
    bool            dumpVersionService;
    bool            dumpVersionList;
    bool            dumpFreqService;
    bool            dumpMemService;
};

#endif  // FRAMEWORK_NATIVE_CMD_DUMPSYS_H_
