/*
 *  Copyright (c) 2024 Rockchip Corporation
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

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
// #include <linux/videodev2.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "rk-fec-config.h"
#include "sample_fec.h"
#ifdef RKFEC_HW_V20
#include "rk_ispfec_videobuf.h"
#endif
#include <rk-video-format.h>
#include "sample_common.h"
#include "sample_drm_buffer.h"

struct mem_ops {
    int (*init)(void);
    void (*deinit)(int fd);
    int (*alloc)(int fd, uint32_t size, struct rkfec_buffer* buf);
    int (*free)(int fd, struct rkfec_buffer* buf);
};

typedef struct rk_ispfec_bg_val_s {
    int bg_y;
    int bg_u;
    int bg_v;
} rkfec_bg_val;

#define SEARCH_MAX_VIDEO_NODES 128
#define IS_DMA_INVALID()       (dma_node_fd < 0)

#define DIR_FROM "/data/xuhf/fec/testData7_camera2/"
#define DIR_TO   "/data/xuhf/fec/sample_ioctl_output/"

#define IN_CAM0_FILE "camera_2560x1520_0.nv12"
#define IN_CAM1_FILE "camera_2560x1520_1.nv12"

#define LUT_CAM0_NONOVERLAPBLOCK0 "camera0_fecNonoverlapBlock0_lut.bin"
#define LUT_CAM0_OVERLAPBLOCK1    "camera0_fecOverlapBlock1_lut.bin"
#define LUT_CAM1_OVERLAPBLOCK0    "camera1_fecOverlapBlock0_lut.bin"
#define LUT_CAM1_NONOVERLAPBLOCK0 "camera1_fecNonoverlapBlock0_lut.bin"

#define OUT_CAM0_OVERLAP_BLOCK1_FILE    "cam0_overlap_block1_out.quad"
#define OUT_CAM1_OVERLAP_BLOCK0_FILE    "cam1_overlap_block0_out.quad"
#define OUT_CAM1_NONOVERLAP_BLOCK0_FILE "cam1_nonoverlap_block0_out.yuv"

static unsigned stream_count;
static char options[OptLast];

static int g_rkfec_fd = -1;

static int width, height, pixfmt, out_pixfmt;
static int out_width = 1728, out_height = 1024;
static int out1_width = 512, out1_height = 1024;
static int out2_width = 512, out2_height = 1024;
static int out3_width = 576, out3_height = 1024;
static int out_width_total = 2816, out_height_total = 1024;
static int mesh_density, border_mode, cross_buf_mode;
static rkfec_bg_val bg_val;
static int cam0_nonoverlap_offs = 1088;

struct rkfec_buffer g_buf_cam0_pic_in;
struct rkfec_buffer g_buf_cam1_pic_in;
struct rkfec_buffer g_buf_pic_out;
struct rkfec_buffer g_buf_pic_out1;
struct rkfec_buffer g_buf_pic_out2;

struct rkfec_buffer g_buf_cam0_nonoverlap_block0_mesh;
struct rkfec_buffer g_buf_cam0_overlap_block1_mesh;
struct rkfec_buffer g_buf_cam1_nonoverlap_block0_mesh;
struct rkfec_buffer g_buf_cam1_overlap_block0_mesh;

struct rkfec_in_out g_rkfec_param = {0};

static char* file_from;
static char* mesh_from;
static char* file_to;

struct mem_ops* cur_mem_pos;
static int dma_node_fd = -1;

static int cal_fec_mesh_size(int width, int height);

static struct option long_options[] = {
    // clang-format off
    {"help", no_argument, 0, OptHelp},
    {"silent", no_argument, 0, OptSilent},
    {"set-fmt-video", required_argument, NULL, OptSetVideoFormat},
    {"set-fmt-video-out", required_argument, NULL, OptSetVideoOutFormat},
    {"stream-count", required_argument, NULL, OptStreamCount},
    {"stream-to", required_argument, NULL, OptStreamTo},
    {"stream-from", required_argument, NULL, OptStreamFrom},
    {"mesh-from", required_argument, NULL, OptMeshFrom},
    {"mesh-density", required_argument, NULL, OptMeshDensity},
    {"border-mode", required_argument, NULL, OptBorderMode},
    {"cross-buf-mode", required_argument, NULL, OptCrossBufMode},
    {"set-bg-val", required_argument, NULL, OptSetBgVal},
    {0, 0, 0, 0}
    // clang-format on
};

void common_usage(void) {
    printf(
        "\nGeneral/Common options:\n"
        "  -v, --set-fmt-video\n"
        "  -x, --set-fmt-video-out "
        "width=<w>,height=<h>,pixelformat=<pf>,field=<f>,colorspace=<c>,\n"
        "  --stream-count <count>\n"
        "                     stream <count> buffers. The default is to keep streaming\n"
        "                     forever. This count does not include the number of initial\n"
        "                     skipped buffers as is passed by --stream-skip.\n"
        "  --stream-from <file> stream from this file.\n"
        "  --stream-to <file>\n"
        "                     stream to this file. The default is to discard the data.\n"
        "  --mesh-from <file> mesh from this file.\n"
        "  --mesh-density <val> mesh density.\n"
        "                       0: 32x16. 1: 16x8. 2: 4x4\n"
        "  --border-mode <val> border mode.\n"
        "                       0: fill with bg_value. 1: copy with the nearest pixel."
        "  --cross-buf-mode <val> cross buffer mode\n"
        "                       0: fill with bg_value. 1: copy with the nearest pixel."
        "  --set-bg-val <val> y=<y>,u=<u>,v=<v>\n"
        "                     fill with the bg_val."
        "  -s, --silent       subpress debug log.\n"
        "  -h, --help         display this help message.\n");
}

/* ------------------------------------------------------------------------- */

void parse_path(const char* path, char* directory, char* filename) {
    const char* last_slash = strrchr(path, '/');
    if (last_slash) {
        size_t dir_len = last_slash - path + 1;
        strncpy(directory, path, dir_len);
        directory[dir_len] = '\0';

        strcpy(filename, last_slash + 1);
    } else {
        directory[0] = '\0';
        strcpy(filename, path);
    }
}

int getsubopt(char** opt, char* const* keys, char** val) {
    char* s = *opt;
    int i;

    *val = NULL;
    *opt = strchr(s, ',');
    if (*opt)
        *(*opt)++ = 0;
    else
        *opt = s + strlen(s);

    for (i = 0; keys[i]; i++) {
        size_t l = strlen(keys[i]);
        if (strncmp(keys[i], s, l)) continue;
        if (s[l] == '=')
            *val = s + l + 1;
        else if (s[l])
            continue;
        return i;
    }
    return -1;
}

int parse_subopt(char** subs, const char* const* subopts, char** value) {
    int opt = getsubopt(subs, (char* const*)(subopts), value);

    if (opt == -1) {
        fprintf(stderr, "Invalid suboptions specified\n");
        return -1;
    }
    if (*value == NULL) {
        fprintf(stderr, "No value given to suboption <%s>\n", subopts[opt]);
        return -1;
    }
    return opt;
}

int parse_bg_val(char* optarg, int* bg_y, int* bg_u, int* bg_v) {
    char *value, *subs;

    subs = optarg;
    while (*subs != '\0') {
        static const char* subopts[] = {"y", "u", "v", NULL};

        switch (parse_subopt(&subs, subopts, &value)) {
            case 0:
                *bg_y = strtoul(value, NULL, 0);
                break;
            case 1:
                *bg_u = strtoul(value, NULL, 0);
                break;
            case 2:
                *bg_v = strtoul(value, NULL, 0);
                break;
            default:
                return -1;
        }
    }
    return 0;
}

int parse_fmt(char* optarg, int* width, int* height, int* pixelformat) {
    char *value, *subs;
    bool be_pixfmt;

    subs = optarg;
    while (*subs != '\0') {
        static const char* subopts[] = {"width", "height", "pixelformat", NULL};

        switch (parse_subopt(&subs, subopts, &value)) {
            case 0:
                *width = strtoul(value, NULL, 0);
                break;
            case 1:
                *height = strtoul(value, NULL, 0);
                break;
            case 2:
                be_pixfmt = strlen(value) == 7 && !memcmp(value + 4, "-BE", 3);
                if (be_pixfmt || strlen(value) == 4) {
                    *pixelformat = v4l2_fourcc(value[0], value[1], value[2], value[3]);
                    if (be_pixfmt) *pixelformat |= 1U << 31;
                } else if (isdigit(value[0])) {
                    *pixelformat = strtol(value, NULL, 0);
                } else {
                    fprintf(stderr, "The pixelformat '%s' is invalid\n", value);
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                return -1;
        }
    }
    return 0;
}

/* ------------------------------------------------------------------------- */

struct mem_ops rk_mem_ops_tbl[] = {
    {
        .init   = init_drm,
        .deinit = deinit_drm,
        .alloc  = alloc_drm_buffer,
        .free   = free_drm_buffer,
    },
#ifdef RKFEC_HW_V20
    {
        .init   = rk_ispfec_api_vb_init,
        .deinit = rk_ispfec_api_vb_deinit,
        .alloc  = rk_ispfec_api_vb_alloc,
        .free   = rk_ispfec_api_vb_free,
    },
#endif
};

int ispfec_mem_init(void) {
    int ret = -1;
    uint32_t i;

    for (i = 0; i < ARRAY_SIZE(rk_mem_ops_tbl); i++) {
        dma_node_fd = rk_mem_ops_tbl[i].init();
        if (dma_node_fd >= 0) break;
    }

    if (dma_node_fd < 0) {
        ret = -1;
        goto exit;
    }

    cur_mem_pos = &rk_mem_ops_tbl[i];

    ret = 0;
exit:
    return ret;
}

void rkfec_mem_deinit(void) {
    if (IS_DMA_INVALID()) return;

    if (cur_mem_pos) {
        cur_mem_pos->deinit(dma_node_fd);
        cur_mem_pos = NULL;
    }
}

int init_ispfec_bufs() {
    int ret = 0;

    ret = ispfec_mem_init();
    if (ret < 0) {
        stderr_info("failed to init mem\n");
        return ret;
    }

    int mesh_size = cal_fec_mesh_size(out_width, out_height);
    // 1. alloc mesh buffer
    ret = cur_mem_pos->alloc(dma_node_fd, mesh_size * 6, &g_buf_cam0_nonoverlap_block0_mesh);
    if (ret) goto deinit_mem;
    info("cam0_nonoverlap_block0 mesh fd:%d size:%ld\n",
         g_buf_cam0_nonoverlap_block0_mesh.dmabuf_fd, g_buf_cam0_nonoverlap_block0_mesh.size);

    mesh_size = cal_fec_mesh_size(out1_width, out1_height);
    ret       = cur_mem_pos->alloc(dma_node_fd, mesh_size * 6, &g_buf_cam0_overlap_block1_mesh);
    if (ret) goto free_buf_cam0_nonoverlap_mesh;
    info("cam0_overlap_block1 mesh fd:%d size:%ld\n", g_buf_cam0_overlap_block1_mesh.dmabuf_fd,
         g_buf_cam0_overlap_block1_mesh.size);

    mesh_size = cal_fec_mesh_size(out2_width, out2_height);
    ret       = cur_mem_pos->alloc(dma_node_fd, mesh_size * 6, &g_buf_cam1_overlap_block0_mesh);
    if (ret) goto free_buf_cam1_nonoverlap_block0_mesh;
    info("cam1_overlap_block1 mesh fd:%d size:%ld\n", g_buf_cam1_overlap_block0_mesh.dmabuf_fd,
         g_buf_cam1_overlap_block0_mesh.size);

    mesh_size = cal_fec_mesh_size(out3_width, out3_height);
    ret       = cur_mem_pos->alloc(dma_node_fd, mesh_size * 6, &g_buf_cam1_nonoverlap_block0_mesh);
    if (ret) goto free_buf_cam0_overlap_block1_mesh;
    info("cam1_nonoverlap_block0 mesh fd:%d size:%ld\n",
         g_buf_cam1_nonoverlap_block0_mesh.dmabuf_fd, g_buf_cam1_nonoverlap_block0_mesh.size);

    // 2. alloc input picture buffer
    ret = cur_mem_pos->alloc(dma_node_fd, width * (height + 1) * 3 / 2, &g_buf_cam0_pic_in);
    if (ret) goto free_buf_overlap_block1_mesh;
    info("cam0 in pic fd:%d size:%ld\n", g_buf_cam0_pic_in.dmabuf_fd, g_buf_cam0_pic_in.size);

    ret = cur_mem_pos->alloc(dma_node_fd, width * (height + 1) * 3 / 2, &g_buf_cam1_pic_in);
    if (ret) goto free_buf_cam0_pic_in;
    info("cam1 in pic fd:%d size:%ld\n", g_buf_cam1_pic_in.dmabuf_fd, g_buf_cam1_pic_in.size);

    // 3. alloc output picture buffer
    if (out_pixfmt == V4L2_PIX_FMT_FBC0) {
        int stride_y = (out_width_total + 63) / 64 * 384;
        int stride_c = (out_width_total + 63) / 64 * 16;
        int size     = (stride_y * out_height_total + stride_c * out_height_total) / 4;

        ret = cur_mem_pos->alloc(dma_node_fd, size, &g_buf_pic_out);
        if (ret) goto free_buf_cam1_pic_in;
        info("out pic fd:%d size:%ld\n", g_buf_pic_out.dmabuf_fd, g_buf_pic_out.size);

        stride_y = (out1_width + 63) / 64 * 384;
        stride_c = (out1_width + 63) / 64 * 16;
        size     = (stride_y * out1_height + stride_c * out1_height) / 4;

        ret = cur_mem_pos->alloc(dma_node_fd, size, &g_buf_pic_out1);
        if (ret) goto free_buf_pic_out;
        info("out1 pic fd:%d size:%ld\n", g_buf_pic_out1.dmabuf_fd, g_buf_pic_out1.size);

        stride_y = (out2_width + 63) / 64 * 384;
        stride_c = (out2_width + 63) / 64 * 16;
        size     = (stride_y * out2_height + stride_c * out2_height) / 4;

        ret = cur_mem_pos->alloc(dma_node_fd, size, &g_buf_pic_out2);
        if (ret) goto free_buf_pic_out1;
        info("out2 pic fd:%d size:%ld\n", g_buf_pic_out2.dmabuf_fd, g_buf_pic_out2.size);
    } else {
        ret = cur_mem_pos->alloc(dma_node_fd, out_width_total * (out_height_total + 1) * 3 / 2,
                                 &g_buf_pic_out);
        if (ret) goto free_buf_cam1_pic_in;
        info("out pic fd:%d size:%ld\n", g_buf_pic_out.dmabuf_fd, g_buf_pic_out.size);

        ret = cur_mem_pos->alloc(dma_node_fd, out1_width * (out1_height + 1) * 3 / 2,
                                 &g_buf_pic_out1);
        if (ret) goto free_buf_pic_out;
        info("out1 pic fd:%d size:%ld\n", g_buf_pic_out1.dmabuf_fd, g_buf_pic_out1.size);

        ret = cur_mem_pos->alloc(dma_node_fd, out2_width * (out2_height + 1) * 3 / 2,
                                 &g_buf_pic_out2);
        if (ret) goto free_buf_pic_out1;
        info("out2 pic fd:%d size:%ld\n", g_buf_pic_out2.dmabuf_fd, g_buf_pic_out2.size);
    }

    return ret;

free_buf_pic_out1:
    cur_mem_pos->free(dma_node_fd, &g_buf_pic_out1);
free_buf_pic_out:
    cur_mem_pos->free(dma_node_fd, &g_buf_pic_out);
free_buf_cam1_pic_in:
    cur_mem_pos->free(dma_node_fd, &g_buf_cam1_pic_in);
free_buf_cam0_pic_in:
    cur_mem_pos->free(dma_node_fd, &g_buf_cam0_pic_in);
free_buf_overlap_block1_mesh:
    cur_mem_pos->free(dma_node_fd, &g_buf_cam1_overlap_block0_mesh);
free_buf_cam1_nonoverlap_block0_mesh:
    cur_mem_pos->free(dma_node_fd, &g_buf_cam1_nonoverlap_block0_mesh);
free_buf_cam0_overlap_block1_mesh:
    cur_mem_pos->free(dma_node_fd, &g_buf_cam0_overlap_block1_mesh);
free_buf_cam0_nonoverlap_mesh:
    cur_mem_pos->free(dma_node_fd, &g_buf_cam0_nonoverlap_block0_mesh);
deinit_mem:
    rkfec_mem_deinit();

    return ret;
}

void deinit_ispfec_bufs() {
    cur_mem_pos->free(dma_node_fd, &g_buf_pic_out2);
    cur_mem_pos->free(dma_node_fd, &g_buf_pic_out1);
    cur_mem_pos->free(dma_node_fd, &g_buf_pic_out);
    cur_mem_pos->free(dma_node_fd, &g_buf_cam1_pic_in);
    cur_mem_pos->free(dma_node_fd, &g_buf_cam0_pic_in);
    cur_mem_pos->free(dma_node_fd, &g_buf_cam1_overlap_block0_mesh);
    cur_mem_pos->free(dma_node_fd, &g_buf_cam1_nonoverlap_block0_mesh);
    cur_mem_pos->free(dma_node_fd, &g_buf_cam0_overlap_block1_mesh);
    cur_mem_pos->free(dma_node_fd, &g_buf_cam0_nonoverlap_block0_mesh);
    rkfec_mem_deinit();
}

/* ------------------------------------------------------------------------- */

static int read_image_from_file(const char* file, void* vir_addr, size_t size) {
    FILE* fp = fopen(file, "rb");
    int ret  = 0;

    if (fp) {
        ret = fseek(fp, 0L, SEEK_END);
        if (ret < 0) {
            printf("E: %s fseek to end failed\n", file);
            fclose(fp);
            return ret;
        }

        size_t len = ftell(fp);
        rewind(fp);

        if (len > size) printf("E: mesh file len %lu > buf size %lu\n", len, size);
        size_t rd_size = fread(vir_addr, 1, len, fp);
        if (rd_size < len) printf("file read size:%lu < %lu \n", rd_size, len);

        printf("mesh file len %lu, read size %lu\n", len, rd_size);
        fclose(fp);
    } else {
        printf("E: open file %s failed: %s \n", file, strerror(errno));
    }

    return ret;
}

FILE* open_output_file() {
    FILE* fout = NULL;

    if (file_to) {
        fout = fopen(file_to, "w+");
        if (!fout) stderr_info("Could not open %s for writing\n", file_to);
        return fout;
    }

    return fout;
}

size_t write_buffer_to_file(FILE* fout, const char* buffer, size_t buffer_size) {
    size_t bytes_written = fwrite(buffer, 1, buffer_size, fout);
    if (bytes_written != buffer_size) {
        stderr_info("Error writing to file");
    }
    return bytes_written;
}

/* ------------------------------------------------------------------------- */

static int read_file_list(const char* basePath) {
    DIR* dir;
    int found = -1;
    struct dirent* ptr;
    char filename[1000];

    if ((dir = opendir(basePath)) == NULL) {
        printf("E: Open dir error...\n");
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
                    // printf("buf=%s\n", buf);
                    if (strstr(buf, "rkfec_offline") != NULL) {
                        printf("found rk fec node\n");
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

static int find_fec_entry(char* videoName) {
    int found      = -1;
    char path[128] = {0};

    for (int i = 0; i < SEARCH_MAX_VIDEO_NODES; i++) {
        memset(path, 0, sizeof(path));
        snprintf(path, sizeof(path), "/sys/class/video4linux/video%d", i);
        if (0 == access(path, F_OK)) {
            found = read_file_list(path);
            if (found > 0) {
                sprintf(videoName, "/dev/video%d", i);
                break;
            }
        }
    }

    return found;
}

int rkfec_hw_init() {
    int ret      = 0;
    char dev[32] = {0};

    ret = find_fec_entry(dev);
    if (ret < 0) {
        printf("E: not found fec hw !\n");
        return -1;
    }

    printf("I: open fec dev %s\n", dev);
    g_rkfec_fd = open(dev, O_RDWR | O_CLOEXEC);
    if (g_rkfec_fd == -1) printf("E: open %s failed %s !", dev, strerror(errno));
    printf("I: open fec dev %s done !\n", dev);

    return ret;
}

void rkfec_hw_deinit() {
    if (g_rkfec_fd >= 0) {
        close(g_rkfec_fd);
        g_rkfec_fd = -1;
    }
}

int rkfec_hw_process(struct rkfec_in_out* param) {
    if (g_rkfec_fd < 0) {
        printf("%s, E: wrong fec fd \n", __FUNCTION__);
        return -1;
    }

    int ret = ioctl(g_rkfec_fd, RKFEC_CMD_IN_OUT, param);

    if (ret == -EAGAIN)  // try again
        ret = ioctl(g_rkfec_fd, RKFEC_CMD_IN_OUT, param);

    return ret;
}

int do_handle_fec(void* addr, uint64_t size) {
    char ch    = '<';
    FILE* fout = NULL;
    int ret;

    ret = rkfec_hw_process(&g_rkfec_param);
    if (ret < 0) {
        printf("E: fec process failed %d\n", ret);
        return -1;
    }

    fout = open_output_file();
    if (fout) {
        ret = write_buffer_to_file(fout, addr, size);

        if (ret <= 0) {
            stderr_info("write_buffer_to_file failed: %s\n", strerror(errno));
            return -1;
        }

        stderr_info("%c\n", ch);
        fflush(stderr);
    }
    if (fout) fclose(fout);

    return 0;
}

static int cal_fec_mesh_size(int width, int height) {
    int block_w = 16, block_h = 8;
    int density = mesh_density;

    if (density == 0) {
        block_w = 32;
        block_h = 16;
    } else if (density == 1) {
        block_w = 16;
        block_h = 8;
    } else if (density == 2) {
        block_w = 4;
        block_h = 4;
    }

    width  = 32 * ((width + 31) / 32);
    height = 32 * ((height + 31) / 32);

    int lut_w = width / block_w + 1;
    int lut_h = height / block_h + 1;

    int lut_size = lut_w * lut_h;

    printf("LUT width: %d, LUT height: %d\n", lut_w, lut_h);
    printf("LUT size: %d\n", lut_size);

    return lut_size;
}

/* ------------------------------------------------------------------------- */

int load_input_files() {
    int ret             = 0;
    char file_name[256] = {0};

    sprintf(file_name, "%s/%s", DIR_FROM, IN_CAM0_FILE);
    ret = read_image_from_file(file_name, g_buf_cam0_pic_in.map, g_buf_cam0_pic_in.size);
    if (ret < 0) {
        stderr_info("read_image_from_file %s failed: %s\n", file_name, strerror(errno));
        return ret;
    }

    sprintf(file_name, "%s/%s", DIR_FROM, IN_CAM1_FILE);
    ret = read_image_from_file(file_name, g_buf_cam1_pic_in.map, g_buf_cam1_pic_in.size);
    if (ret < 0) {
        stderr_info("read_image_from_file %s failed: %s\n", file_name, strerror(errno));
        return ret;
    }

    sprintf(file_name, "%s/%s", DIR_FROM, LUT_CAM0_NONOVERLAPBLOCK0);
    ret = read_image_from_file(file_name, g_buf_cam0_nonoverlap_block0_mesh.map,
                               g_buf_cam0_nonoverlap_block0_mesh.size);
    if (ret < 0) {
        stderr_info("read_image_from_file %s failed: %s\n", file_name, strerror(errno));
        return ret;
    }

    sprintf(file_name, "%s/%s", DIR_FROM, LUT_CAM0_OVERLAPBLOCK1);
    ret = read_image_from_file(file_name, g_buf_cam0_overlap_block1_mesh.map,
                               g_buf_cam0_overlap_block1_mesh.size);
    if (ret < 0) {
        stderr_info("read_image_from_file %s failed: %s\n", file_name, strerror(errno));
        return ret;
    }

    sprintf(file_name, "%s/%s", DIR_FROM, LUT_CAM1_OVERLAPBLOCK0);
    ret = read_image_from_file(file_name, g_buf_cam1_overlap_block0_mesh.map,
                               g_buf_cam1_overlap_block0_mesh.size);
    if (ret < 0) {
        stderr_info("read_image_from_file %s failed: %s\n", file_name, strerror(errno));
        return ret;
    }

    sprintf(file_name, "%s/%s", DIR_FROM, LUT_CAM1_NONOVERLAPBLOCK0);
    ret = read_image_from_file(file_name, g_buf_cam1_nonoverlap_block0_mesh.map,
                               g_buf_cam1_nonoverlap_block0_mesh.size);
    if (ret < 0) {
        stderr_info("read_image_from_file %s failed: %s\n", file_name, strerror(errno));
        return ret;
    }

    return ret;
}

void rkfec_hw_params_init(struct rkfec_in_out* param) {
    param->in_width   = width;
    param->in_height  = height;
    param->out_width  = out_width;
    param->out_height = out_height;
    param->in_fourcc  = pixfmt;
    param->out_fourcc = out_pixfmt;

    param->buf_cfg.in_pic_fd  = g_buf_cam0_pic_in.dmabuf_fd;
    param->buf_cfg.out_pic_fd = g_buf_pic_out.dmabuf_fd;
    memset(g_buf_pic_out.map, 0, g_buf_pic_out.size);

    param->buf_cfg.in_offs = 0;
    param->buf_cfg.in_size = param->in_width * param->in_height * 3 / 2;
    if (param->out_fourcc == V4L2_PIX_FMT_FBCE) {
        int stride_y = (param->out_width + 63) / 64 * 384;
        int stride_c = (param->out_width + 63) / 64 * 16;

        param->buf_cfg.out_offs = cam0_nonoverlap_offs;
        param->buf_cfg.out_size = (stride_y * param->out_height + stride_c * param->out_height) / 4;
    } else {
        param->buf_cfg.out_offs = cam0_nonoverlap_offs;
        param->buf_cfg.out_size = param->out_width * param->out_height * 3 / 2;
    }

    param->buf_cfg.in_stride  = width;
    param->buf_cfg.out_stride = out_width_total;
    param->buf_cfg.lut_fd     = g_buf_cam0_nonoverlap_block0_mesh.dmabuf_fd;
    param->buf_cfg.lut_size   = g_buf_cam0_nonoverlap_block0_mesh.size / 6;

    param->core_ctrl.density      = 0;
    param->core_ctrl.bic_mode     = 0;
    param->core_ctrl.border_mode  = border_mode;
    param->core_ctrl.pbuf_crs_dis = 0;
    param->core_ctrl.buf_mode     = cross_buf_mode;
}

int process_cam0_nonoverlap_block0() {
    int ret = 0;

    rkfec_hw_params_init(&g_rkfec_param);

    file_to = NULL;

    printf("Process cam0_nonoverlap_block0 ...\n");
    ret = do_handle_fec(g_buf_pic_out.map, g_buf_pic_out.size);
    if (ret < 0) printf("E: do_handle_fec failed\n");

    return ret;
}

int process_cam0_overlap_block1() {
    int ret             = 0;
    char file_name[256] = {0};

    g_rkfec_param.out_width          = out1_width;
    g_rkfec_param.out_height         = out1_height;
    g_rkfec_param.buf_cfg.out_stride = out1_width;
    g_rkfec_param.buf_cfg.out_pic_fd = g_buf_pic_out1.dmabuf_fd;
    g_rkfec_param.out_fourcc         = V4L2_PIX_FMT_QUAD;
    if (g_rkfec_param.out_fourcc == V4L2_PIX_FMT_FBCE) {
        int stride_y = (out1_width + 63) / 64 * 384;
        int stride_c = (out1_width + 63) / 64 * 16;

        g_rkfec_param.buf_cfg.out_offs = 0;
        g_rkfec_param.buf_cfg.out_size = (stride_y * out1_height + stride_c * out1_height) / 4;
    } else {
        g_rkfec_param.buf_cfg.out_offs = 0;
        g_rkfec_param.buf_cfg.out_size = out1_width * out1_height * 3 / 2;
    }

    g_rkfec_param.buf_cfg.lut_fd   = g_buf_cam0_overlap_block1_mesh.dmabuf_fd;
    g_rkfec_param.buf_cfg.lut_size = g_buf_cam0_overlap_block1_mesh.size / 6;

    sprintf(file_name, "%s/%s", DIR_TO, OUT_CAM0_OVERLAP_BLOCK1_FILE);
    file_to = file_name;

    printf("Process cam0_overlap_block1...\n");
    ret = do_handle_fec(g_buf_pic_out1.map, g_buf_pic_out1.size);

    return ret;
}

int process_cam1_overlap_block0() {
    int ret             = 0;
    char file_name[256] = {0};

    g_rkfec_param.buf_cfg.in_pic_fd = g_buf_cam1_pic_in.dmabuf_fd;

    g_rkfec_param.buf_cfg.lut_fd   = g_buf_cam1_overlap_block0_mesh.dmabuf_fd;
    g_rkfec_param.buf_cfg.lut_size = g_buf_cam1_overlap_block0_mesh.size / 6;

    g_rkfec_param.out_width          = out2_width;
    g_rkfec_param.out_height         = out2_height;
    g_rkfec_param.buf_cfg.out_stride = out2_width;
    g_rkfec_param.buf_cfg.out_pic_fd = g_buf_pic_out2.dmabuf_fd;
    g_rkfec_param.out_fourcc         = V4L2_PIX_FMT_QUAD;
    if (g_rkfec_param.out_fourcc == V4L2_PIX_FMT_FBCE) {
        int stride_y = (out2_width + 63) / 64 * 384;
        int stride_c = (out2_width + 63) / 64 * 16;

        g_rkfec_param.buf_cfg.out_offs = 0;
        g_rkfec_param.buf_cfg.out_size = (stride_y * out2_height + stride_c * out2_height) / 4;
    } else {
        g_rkfec_param.buf_cfg.out_offs = 0;
        g_rkfec_param.buf_cfg.out_size = out2_width * out2_height * 3 / 2;
    }

    sprintf(file_name, "%s/%s", DIR_TO, OUT_CAM1_OVERLAP_BLOCK0_FILE);
    file_to = file_name;

    printf("Process cam1_overlap_block0 ...\n");
    ret = do_handle_fec(g_buf_pic_out2.map, g_buf_pic_out2.size);
    if (ret < 0) printf("E: do_handle_fec failed\n");

    return ret;
}

int process_cam1_nonoverlap_block0() {
    int ret             = 0;
    char file_name[256] = {0};

    g_rkfec_param.buf_cfg.lut_fd   = g_buf_cam1_nonoverlap_block0_mesh.dmabuf_fd;
    g_rkfec_param.buf_cfg.lut_size = g_buf_cam1_nonoverlap_block0_mesh.size / 6;

    g_rkfec_param.out_width          = out3_width;
    g_rkfec_param.out_height         = out3_height;
    g_rkfec_param.buf_cfg.out_stride = out_width_total;
    g_rkfec_param.buf_cfg.out_pic_fd = g_buf_pic_out.dmabuf_fd;
    g_rkfec_param.out_fourcc         = V4L2_PIX_FMT_NV12;
    if (g_rkfec_param.out_fourcc == V4L2_PIX_FMT_FBCE) {
        int stride_y = (out3_width + 63) / 64 * 384;
        int stride_c = (out3_width + 63) / 64 * 16;

        g_rkfec_param.buf_cfg.out_offs = 0;
        g_rkfec_param.buf_cfg.out_size = (stride_y * out3_height + stride_c * out3_height) / 4;
    } else {
        g_rkfec_param.buf_cfg.out_offs = 0;
        g_rkfec_param.buf_cfg.out_size = out3_width * out3_height * 3 / 2;
    }

    sprintf(file_name, "%s/%s", DIR_TO, OUT_CAM1_NONOVERLAP_BLOCK0_FILE);
    file_to = file_name;

    printf("Process cam1_nonoverlap_block0 ...\n");
    ret = do_handle_fec(g_buf_pic_out.map, g_buf_pic_out.size);
    if (ret < 0) printf("E: do_handle_fec failed\n");

    return ret;
}

/* ------------------------------------------------------------------------- */

int32_t main(int argc, char** argv) {
    int32_t i;

    /* command args */
    int32_t ch;
    char short_options[6 * 2 * 3 + 1];
    int32_t idx = 0;
    FILE* fout  = NULL;

    if (argc == 1) {
        common_usage();
        return 0;
    }
    for (i = 0; long_options[i].name; i++) {
        if (!isalpha(long_options[i].val)) continue;
        short_options[idx++] = long_options[i].val;
        if (long_options[i].has_arg == required_argument) {
            short_options[idx++] = ':';
        } else if (long_options[i].has_arg == optional_argument) {
            short_options[idx++] = ':';
            short_options[idx++] = ':';
        }
    }
    while (1) {
        int option_index = 0;
        int ret          = 0;

        short_options[idx] = 0;
        ch                 = getopt_long(argc, argv, short_options, long_options, &option_index);
        if (ch == -1) break;

        options[(int)ch] = 1;
        switch (ch) {
            case OptHelp:
                common_usage();
                return 0;
            case OptSetVideoFormat:
                ret = parse_fmt(optarg, &width, &height, &pixfmt);
                if (ret < 0) {
                    common_usage();
                    exit(EXIT_FAILURE);
                }
                break;
            case OptSetVideoOutFormat:
                ret = parse_fmt(optarg, &out_width, &out_height, &out_pixfmt);
                if (ret < 0) {
                    common_usage();
                    exit(EXIT_FAILURE);
                }
                break;
            case OptStreamCount:
                stream_count = strtoul(optarg, NULL, 0);
                break;
            case OptStreamTo:
                file_to = optarg;
                break;
            case OptStreamFrom:
                file_from = optarg;
                break;
            case OptMeshFrom:
                mesh_from = optarg;
                break;
            case OptMeshDensity:
                mesh_density = atoi(optarg);
                break;
            case OptBorderMode:
                border_mode = atoi(optarg);
                break;
            case OptCrossBufMode:
                cross_buf_mode = atoi(optarg);
                break;
            case OptSetBgVal:
                ret = parse_bg_val(optarg, &bg_val.bg_y, &bg_val.bg_u, &bg_val.bg_v);
                if (ret < 0) printf("Failed to parse bg_val\n");
                break;
            default:
                break;
        }
    }
    if (optind < argc) {
        printf("unknown arguments: ");
        while (optind < argc) printf("%s ", argv[optind++]);
        printf("\n");
        common_usage();
        exit(EXIT_FAILURE);
    }

    int ret = rkfec_hw_init();
    if (ret < 0) {
        printf("E: failed to init fec hw!\n");
        return -1;
    }

    ret = init_ispfec_bufs();
    if (ret < 0) {
        stderr_info("init_ispfec_bufs failed: %s\n", strerror(errno));
        goto error_init;
    }

    ret = load_input_files();
    if (ret < 0) goto clean_up;

    {
        ret = process_cam0_nonoverlap_block0();
        if (ret < 0) goto clean_up;

        ret = process_cam0_overlap_block1();
        if (ret < 0) goto clean_up;

        ret = process_cam1_overlap_block0();
        if (ret < 0) goto clean_up;

        ret = process_cam1_nonoverlap_block0();
        if (ret < 0) goto clean_up;
    }

clean_up:
    deinit_ispfec_bufs();
error_init:
    rkfec_hw_deinit();

    return 0;
}
