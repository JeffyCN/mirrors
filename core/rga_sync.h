/*
 *  Copyright 2017 The Android Open Source Project
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef __RGA_IM2D_SYNC_H__
#define __RGA_IM2D_SYNC_H__

#include <stdint.h>

/* timeout in msecs */
int rga_sync_wait(int fd, int timeout);

/**
 * Merge two sync files.
 *
 * This produces a new sync file with the given name which has the union of the
 * two original sync file's fences; redundant fences may be removed.
 *
 * If one of the input sync files is signaled or invalid, then this function
 * may behave like dup(): the new file descriptor refers to the valid/unsignaled
 * sync file with its original name, rather than a new sync file.
 *
 * The original fences remain valid, and the caller is responsible for closing
 * them.
 *
 * Available since API level 26.
 */
int32_t rga_sync_merge(const char* name, int32_t fd1, int32_t fd2);

#endif
