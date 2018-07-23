/*
 * Copyright (C) 2014-2017 Intel Corporation
 * Copyright (c) 2017, Fuzhou Rockchip Electronics Co., Ltd
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
#define LOG_TAG "Metadata2Str"

#include <string>
#include "Metadata2Str.h"

namespace XCam {
const char *metaId2String(const metadata_value_t array[], int size, int value)
{
    for (int i = 0; i < size; i++) {
        if (array[i].value == value)
            return array[i].name;
    }

    std::string out("id not found ");
    out = out + std::to_string(value);
    return out.c_str();
}
}

