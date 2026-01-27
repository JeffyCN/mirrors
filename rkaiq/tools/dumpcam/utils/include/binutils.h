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
#ifndef BIN_UTILS_H
#define BIN_UTILS_H

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <cstring>

bool strings(const char *fileName, const char *searchString, std::string &versionStr, bool showlist  = true);
int executeCMD(const char *cmd, std::string &versionStr, bool showlist = true);

#endif
