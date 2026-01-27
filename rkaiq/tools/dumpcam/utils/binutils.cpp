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

#include <unistd.h>
#include "binutils.h"
const unsigned CMD_RESULT_LINE_SIZE = 4096;

bool strings(const char *fileName, const char *searchString,
        std::string &versionStr, bool showlist) {
    int c;
    unsigned int last = 0;
    char buf[CMD_RESULT_LINE_SIZE];
    FILE *fp = NULL;
    bool ret = false;
    if ((fp = fopen(fileName, "r")) == NULL) {
        fprintf(stderr, "open %s error.\n", fileName);
        return false;
    }
    while (1) {
        c = getc(fp);
        if ((isprint(c) || c =='\t') && (last < (CMD_RESULT_LINE_SIZE-1))) {
            buf[last++] = c;
        } else {
            if (last >= 4) {
                buf[last] = '\0';
                if (strstr(buf, searchString) != NULL) {
                    versionStr.append(buf);
                    versionStr.append("\n");
                    ret = true;
                    if (!showlist) {
                        break;
                    }
                }
            }
            last = 0;
        }
        if (c == EOF) {
            break;
        }
    }
    return ret;
}

int executeCMD(const char *cmd, std::string &versionStr, bool showlist) {
    int ret = 0;
    char lineBuf[CMD_RESULT_LINE_SIZE] = {0};
    FILE *cmdPtr = NULL;
    if ((cmdPtr = popen(cmd, "r")) != NULL) {
        if (showlist) {
            while (fgets(lineBuf, sizeof(lineBuf), cmdPtr) != NULL) {
                versionStr.append(lineBuf);
                ret++;
            }
        } else {
            if (fgets(lineBuf, sizeof(lineBuf), cmdPtr) != NULL) {
                versionStr.append(lineBuf);
                ret++;
            }
        }
        pclose(cmdPtr);
        cmdPtr = NULL;
    }
    return ret;
}
