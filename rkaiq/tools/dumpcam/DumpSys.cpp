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

#include <thread>
#include <iostream>
#include <sys/socket.h>
#include "stringprintf.h"
#include "DumpDefine.h"
#include "DumpSys.h"

static void usage() {
    fprintf(stderr,
            "usage: \n"
            "         dumpcam [moudle] [moudle ARGS] [--uri uriName] [uri ARGS] \n"
            "         --help: shows this help\n"
            "         --uri uriName: register service base on uriName \n"
            "         [uri ARGS]: optionally passing ARGS to service \n"
            "         moudle [moudle ARGS]: dumps only module, optionally passing ARGS to it\n");
}

Dumpsys::Dumpsys() {
    dumpData = NULL;
    dumpClient = new DumpClient();
    dumpVersion = new DumpVersion();
    dumpMemory = new DumpMemory();
    servicerUri = "dump";
    dumpService = true;
    dumpVersionList = false;
    dumpVersionService = false;
    dumpFreqService = false;
    dumpMemService = false;
}

Dumpsys::~Dumpsys() {
    if (dumpClient) {
        delete dumpClient;
        dumpClient = NULL;
    }
    if (dumpData) {
        delete[] dumpData;
        dumpData = NULL;
    }
    if (dumpVersion) {
        delete dumpVersion;
        dumpVersion = NULL;
    }
    if (dumpMemory) {
        delete dumpMemory;
        dumpMemory = NULL;
    }
}

int Dumpsys::start(int argc, char* const argv[]) {
    int result = DUMP_SUCCESS;
    std::vector<std::string > services;
    std::vector<std::string> args;
    static struct option longOptions[] = {{"uri", required_argument, 0, 0},
                                          {"version", no_argument, 0, 0},
                                          {"list", no_argument, 0, 0},
                                          {"freq", no_argument, 0, 0},
                                          {"mem", no_argument, 0, 0},
                                          {"help", no_argument, 0, 0},
                                          {0, 0, 0, 0}};
    optind = 1;
    while (1) {
        int c;
        int optionIndex = 0;
        c = getopt_long(argc, argv, "+", longOptions, &optionIndex);
        if (c == -1) {
            break;
        }
        switch (c) {
            case 0:
                if (!strcmp(longOptions[optionIndex].name, "help")) {
                    usage();
                    return 0;
                } else if (!strcmp(longOptions[optionIndex].name, "uri")) {
                    servicerUri = std::string(optarg);
                    dumpService = false;
                } else if (!strcmp(longOptions[optionIndex].name, "version")) {
                    dumpVersionService = true;
                } else if (!strcmp(longOptions[optionIndex].name, "ver-list")) {
                    dumpVersionList = true;
                } else if (!strcmp(longOptions[optionIndex].name, "freq")) {
                    dumpFreqService = true;
                } else if (!strcmp(longOptions[optionIndex].name, "mem")) {
                    dumpMemService = true;
                }
                break;
            default:
                fprintf(stderr, "\n");
                usage();
                break;
        }

        if (!dumpService) {
            break;
        }
    }

    if (dumpFreqService) {
        result = dumpMemory->dumpModuleFreq();
        return result;
    }

    if (dumpMemService) {
        result = dumpMemory->dumpMemoryInfo();
        return result;
    }

    if (dumpVersionService || (argc >= 2 && !strcmp(argv[1], "version"))) {
        dumpVersion->dumpSetDumpList(dumpVersionList);
        result = dumpVersion->dumpModuleVersion();
        return result;
    }

    // set defaule service name all
    if (argc == 1) {
        services.push_back("all");
        snprintf(moduleName, sizeof(moduleName), "%s", "all");
    }

    // dumpsys usage module name + argv
    for (int i = optind; i < argc; i++) {
        if (dumpService && i == optind) {
            services.push_back(std::string(argv[i]));
            snprintf(moduleName, sizeof(moduleName), "%s", argv[i]);
        } else {
            const std::string arg(argv[i]);
            args.push_back(arg);
        }
    }

    generateJson(services, args);

    result =  InitDumpClient();
    if (result == DUMP_SUCCESS) {
        result = DumpClientSendModule();
        if (result == DUMP_SUCCESS) {
            result = DumpClientReceiveDump();
        } else {
            DEBUG_MSG_ERROR("%s: Confirm RKAIQ has been initialized and running?\n", __FUNCTION__);
        }
    }
    result = writeDumpToStd();
    return result;
}

bool Dumpsys::InitDumpClient() {
    bool result = DUMP_SUCCESS;
    if (dumpClient != NULL) {
        result = dumpClient->sockClientOpen(SOCKET_TCP_NAME);
    }
    return result;
}

bool Dumpsys::DumpClientSendModule() {
    bool result = DUMP_SUCCESS;
    char buffer[MAX_LINE_LENGTH] = {0};
    snprintf(buffer, MAX_LINE_LENGTH, "/cam/rkaiq/%s#%s#", servicerUri.c_str(), jsonBuffer.c_str());
    DEBUG_MSG("send buffer:%s \n", buffer);
    if (dumpClient) {
        result = dumpClient->sendDump(MODULE_NAME_MESSAGE_ID, buffer, MAX_LINE_LENGTH);
    }
    return result;
}

bool Dumpsys::DumpClientReceiveDump() {
    bool result = DUMP_SUCCESS;
    if (dumpClient) {
        result = dumpClient->receiveDumpMessage(dumpId, dumpSize);
        if (result == DUMP_SUCCESS) {
            dumpData = new char[dumpSize];
            memset(dumpData, 0, dumpSize);
            result = dumpClient->receiveDump(dumpData, dumpSize);
            if (result == DUMP_FAIL) {
                delete[] dumpData;
                dumpData = NULL;
            }
        }
    }
    return result;
}

bool Dumpsys::generateJson(std::vector<std::string > &service, std::vector<std::string > &args) {
    int result = DUMP_SUCCESS;
    if (service.size() > 0) {
        jsonBuffer.append("{\"");
        jsonBuffer.append("module");
        jsonBuffer.append("\":\"");
        jsonBuffer.append(service[0].c_str());
        jsonBuffer.append("\"");
        if (args.size() == 0) {
            jsonBuffer.append("}");
        }
    }
    if (args.size() > 0) {
        if (dumpService) {
            jsonBuffer.append(", \"moduleArgs\":[");
        } else {
            jsonBuffer.append("{\"uriArgs\":[");
        }
    }
    std::vector<std::string >::iterator it = args.begin();
    for (; it != args.end(); ++it) {
        jsonBuffer.append("\"");
        jsonBuffer.append((*it).c_str());
        jsonBuffer.append("\"");
        if (it != (args.end() -1)) {
            jsonBuffer.append(",");
        } else {
            jsonBuffer.append("]}");
        }
    }
    DEBUG_MSG("jsonBuffer:%s \n", jsonBuffer.c_str());
    return result;
}

bool Dumpsys::writeDumpToStd() {
    int result = DUMP_SUCCESS;
    writeDumpHeader(STDOUT_FILENO, moduleName);
    result = writeDump(STDOUT_FILENO, moduleName);
    writeDumpFooter(STDOUT_FILENO, moduleName);
    return result;
}

bool Dumpsys::writeDumpHeader(int fd, const char * moduleName) {
    std::string msg(
        "----------------------------------------"
        "---------------------------------------\n");
    StringAppendF(&msg, "DUMP OF SERVICE %s:\n", moduleName);
    WriteStringToFd(msg, fd);
    return DUMP_SUCCESS;
}

bool Dumpsys::writeDumpFooter(int fd, const char * moduleName) {
    std::string msg(
        "----------------------------------------"
        "---------------------------------------\n");
    StringAppendF(&msg, "END DUMP OF SERVICE %s:\n", moduleName);
    WriteStringToFd(msg, fd);
    return DUMP_SUCCESS;
}

bool Dumpsys::writeDump(int fd, const char * moduleName) {
    bool status = DUMP_SUCCESS;

    if (dumpClient && dumpData) {
        if (!WriteFully(fd, dumpData, dumpSize)) {
            std::cerr << "Failed to write while dumping service " << moduleName << ": "
                    << strerror(errno) << std::endl;
            status = DUMP_FAIL;
        }
        delete[] dumpData;
        dumpData = NULL;
    }
    return status;
}
