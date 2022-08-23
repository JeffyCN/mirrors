/*
 * Copyright (C) 2022  Rockchip Electronics Co., Ltd.
 * Authors:
 *     YuQiaowei <cerf.yu@rock-chips.com>
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

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstddef>
#include <cmath>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

int64_t get_cur_us() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

int64_t get_cur_ms() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void draw_rgba(char *buffer, int width, int height) {
    for (int i = 0; i < height; i++) {
       for (int j = 0; j < width/4; j++) {
           buffer[(i*width*4) + j*4 + 0] = 0xff;   //R
           buffer[(i*width*4) + j*4 + 1] = 0x00;   //G
           buffer[(i*width*4) + j*4 + 2] = 0x00;   //B
           buffer[(i*width*4) + j*4 + 3] = 0xff;   //A
       }
       for (int j = width/4; j < width/4*2; j++) {
           buffer[(i*width*4) + j*4 + 0] = 0x00;
           buffer[(i*width*4) + j*4 + 1] = 0xff;
           buffer[(i*width*4) + j*4 + 2] = 0x00;
           buffer[(i*width*4) + j*4 + 3] = 0xff;
       }
       for (int j = width/4*2; j < width/4*3; j++) {
           buffer[(i*width*4) + j*4 + 0] = 0x00;
           buffer[(i*width*4) + j*4 + 1] = 0x00;
           buffer[(i*width*4) + j*4 + 2] = 0xff;
           buffer[(i*width*4) + j*4 + 3] = 0xff;
       }
       for (int j = width/4*3; j < width; j++) {
           buffer[(i*width*4) + j*4 + 0] = 0xff;
           buffer[(i*width*4) + j*4 + 1] = 0xff;
           buffer[(i*width*4) + j*4 + 2] = 0xff;
           buffer[(i*width*4) + j*4 + 3] = 0xff;
       }
    }
}
