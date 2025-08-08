/*
 *  Copyright (c) 2025 Rockchip Corporation
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

#include "rk_ispfec_videobuf.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "rkfec_config.h"
#ifdef RKFEC_HW_V20
#include "rk-fec-config.h"
#else
#include "rkispp-config.h"
#endif

#include "RkIspFecComm.h"

#define SEARCH_MAX_VIDEO_NODES 128

static int rkfec_fd = -1;

static int rk_ispfec_readFileList(const char* basePath) {
    DIR* dir;
    int found = -1;
    struct dirent* ptr;
    char filename[1000];

    if ((dir = opendir(basePath)) == NULL) {
        rkfec_err("Open dir error...");
        return found;
    }

    while ((ptr = readdir(dir)) != NULL) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            /// current dir OR parrent dir
            continue;
        } else if (ptr->d_type == 8) {
            /// file, check video*/name whether has rkispp_fec info
            if (strcmp(ptr->d_name, "name") == 0) {
                memset(filename, '\0', sizeof(filename));
                strcpy(filename, basePath);
                strcat(filename, "/");
                strcat(filename, ptr->d_name);
                FILE* fp = fopen(filename, "rb");
                if (fp) {
                    char buf[128];
                    const char* ret = fgets(buf, 128, fp);
                    // printf("buf=%s", buf);
                    if (strstr(buf, "rkfec_offline") != NULL) {
                        rkfec_info("found rk fec node");
                        found = 1;
                        fclose(fp);
                        break;
                    }
                    fclose(fp);
                }
            }
        } else if (ptr->d_type == 10) {
            // TODO: nothing
        } else if (ptr->d_type == 4) {
            // TODO: nothing
        }
    }
    closedir(dir);
    return found;
}

static int rk_ispfec_findFecEntry(char* videoName) {
    int found      = -1;
    char path[128] = {0};

    for (int i = 0; i < SEARCH_MAX_VIDEO_NODES; i++) {
        memset(path, 0, sizeof(path));
        snprintf(path, sizeof(path), "/sys/class/video4linux/video%d", i);
        if (0 == access(path, F_OK)) {
            found = rk_ispfec_readFileList(path);
            if (found > 0) {
                sprintf(videoName, "/dev/video%d", i);
                break;
            }
        }
    }

    return found;
}

int rk_ispfec_api_vb_init(int fd) {
    int ret      = -1;
    char dev[32] = {0};
    ret          = rk_ispfec_findFecEntry(dev);
    if (ret < 0) {
        rkfec_err("not found fec hw !");
        return -1;
    }

    rkfec_fd = open(dev, O_RDWR | O_CLOEXEC);
    if (rkfec_fd == -1) rkfec_err("open %s failed %s !", dev, strerror(errno));
    rkfec_info("open fec dev %s done !", dev);
    return 0;
}

void rk_ispfec_api_vb_deinit(int fd) {
    if (rkfec_fd >= 0) close(rkfec_fd);
}

int rk_ispfec_api_vb_alloc(int fd, uint32_t size, struct rkfec_buffer* buf) {
    if (buf == NULL) {
        return -1;
    }

    int ret                 = 0;
    struct rkfec_buf buffer = {0};

    buffer.size = size;
    ret         = ioctl(rkfec_fd, RKFEC_CMD_BUF_ALLOC, &buffer);
    if (ret < 0) {
        rkfec_err("ioctl return failed on fd(%d), cmd:0x%.8x", rkfec_fd, RKFEC_CMD_BUF_ALLOC);
        return ret;
    }

    buf->dmabuf_fd = buffer.buf_fd;
    buf->size      = buffer.size;
    buf->map       = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, buffer.buf_fd, 0);
    if (buf->map == MAP_FAILED) {
        rkfec_err("Mmap failed");
        return -1;
    }

    return 0;
}

int rk_ispfec_api_vb_free(int fd, struct rkfec_buffer* buf) {
    if (buf == NULL || buf->map == NULL) {
        return -1;
    }

    int ret = 0;

    ret = ioctl(rkfec_fd, RKFEC_CMD_BUF_DEL, buf);
    if (ret < 0) {
        rkfec_err("ioctl return failed on fd(%d), cmd:0x%.8x", rkfec_fd, RKFEC_CMD_BUF_DEL);
        return ret;
    }

    if (munmap(buf->map, buf->size) == -1) {
        rkfec_err("Munmap failed");
        return -1;
    }

    buf->map       = NULL;
    buf->size      = 0;
    buf->dmabuf_fd = -1;

    return 0;
}