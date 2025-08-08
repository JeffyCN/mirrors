/*
 * Copyright 2021 Rockchip Electronics Co. LTD
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

#ifndef DUMP_VERSION_H_
#define DUMP_VERSION_H_

#include <stdlib.h>
#include <cstdio>
#include <string>
#include <stdarg.h>
#include <mutex>

#define ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))

class DumpVersion {
 public:
    DumpVersion();
    virtual ~DumpVersion();

 public:
    int dumpModuleVersion();
    int dumpRkAiqVersion(std::string &versionStr);
    int dumpDumpVersion(std::string &versionStr);
    int getLibPath();
    int getVersionStrings(const char *libPath, const char *searchString, std::string &versionStr);
    int executeStrings(const char *libPath, const char *searchString, std::string &versionStr);
    int writeVersionHead(const char *module, std::string &versionStr);
    int dumpSetDumpList(bool listEnable);

 private:
    bool showlist;
};

#endif /* DUMP_CLIENT_H_ */
