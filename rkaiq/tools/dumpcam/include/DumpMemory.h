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

#ifndef DUMP_MEMORY_H_
#define DUMP_MEMORY_H_

#include <stdlib.h>
#include <cstdio>
#include <string>
#include <stdarg.h>
#include <mutex>

#define ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))

class DumpMemory {
 public:
    DumpMemory();
    virtual ~DumpMemory();

 public:
    int dumpModuleFreq();
    int dumpCpuFreq(std::string &freqStr);
    int dumpVpuFreq(std::string &freqStr);
    int dumpGpuFreq(std::string &freqStr);
    int dumpRgaFreq(std::string &freqStr);
    int dumpMemoryInfo();
    int dumpMemoryMMdump(std::string &memoryStr);
    int dumpMemoryFree(std::string &memoryStr);
    int writeStrHead(const char *module, std::string &writeStr);

 private:
    bool showlist;
};


#endif /* DUMP_MEMORY_H_ */
