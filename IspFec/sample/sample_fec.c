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

/**
 * Example Command:
 *
 * sample_fec --set-fmt-video=width=3840,height=2160,pixelformat=NV12 \
 *            --set-fmt-video-out=width=3840,height=2160,pixelformat=NV12 \
 *            --stream-count 3 \
 *            --border-mode 0 \
 *            --cross-buf-mode 0 \
 *            --stream-from "/data/pic_33_0_0_3840_2160.bin" \
 *            --stream-to "/data/output_pic_33_0_0_3840_2160.bin" \
 *            --mesh-from "/data/mesh.bin"
 *
 * sample_fec --set-fmt-video=width=3840,height=2160,pixelformat=NV12 \
 *            --set-fmt-video-out=width=3840,height=2160,pixelformat=NV12 \
 *            --stream-count 3 \
 *            --border-mode 0 \
 *            --cross-buf-mode 0 \
 *            --stream-from "/data/pic_33_0_0_3840_2160.bin" \
 *            --stream-to "/data/output_pic_33_0_0_3840_2160.bin" \
 *            --calib-ini-from /data/imx415_CMK-OT1948-PV1_ldc.ini
 *
 **/

#include "sample_fec.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <termios.h>
#include <unistd.h>

#include "inih/ini.h"
#include "rk_ispfec_api.h"
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
    bool need_sync;
};

typedef struct {
    int calib_image_width;
    int calib_image_height;
    float optic_center_x;
    float optic_center_y;
    double coeff[4];
    int correctStrg_maxLimit;
    int correctStrg_val;
    int saveMaxFovX;
} FecCailbParams;

#define IN_IMAGE_FILE "/data/imx415_4k_nv12.yuv"
#define IN_MESH_FILE_1 "/data/rv1126b_fecMesh_src3840x2160_dst3840x2160_step32x16_level000.bin"
#define IN_MESH_FILE_2 "/data/rv1126b_fecMesh_src3840x2160_dst3840x2160_step32x16_level200.bin"

static const char* file_from;
static const char* file_to;
static const char* mesh_from;
static const char* calib_ini_from;
static unsigned stream_count;
static unsigned stream_skip;
static char options[OptLast];

static int width, height, out_width, out_height, pixfmt, out_pixfmt;
static int offset_x, stride, out_offset_x, out_stride;
static int mesh_density, border_mode, cross_buf_mode;
static rk_ispfec_bg_val bg_val;

static rk_ispfec_ctx_t* g_ispfec_ctx = NULL;
static rk_ispfec_cfg_t g_ispfec_cfg;

struct rkfec_buffer g_rkfec_buf_pic_in;
struct rkfec_buffer g_rkfec_buf_pic_out;

#ifdef RKFEC_HW_V20
/* add g_rkfec_buf_mesh[1] for api test */
#define MESH_BUF_NUM 2
struct rkfec_buffer g_rkfec_buf_mesh[MESH_BUF_NUM];
#else
struct rkfec_buffer g_rkfec_buf_xint;
struct rkfec_buffer g_rkfec_buf_xfra;
struct rkfec_buffer g_rkfec_buf_yint;
struct rkfec_buffer g_rkfec_buf_yfra;
#endif

struct mem_ops* cur_mem_pos;
static int dma_node_fd = -1;

static int target_fps = 30;
static long target_interval_ns = 33333333L;
static struct termios oldt;
static bool is_quit          = false;
static bool enable_uapi_test = false;
static bool auto_parse_ini   = false;

#define IS_DMA_INVALID() (dma_node_fd < 0)

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
    {"calib-ini-from", required_argument, NULL, OptCalibFrom},
    {"auto-parse-ini", required_argument, NULL, OptAutoParseIni},
    {"mesh-density", required_argument, NULL, OptMeshDensity},
    {"border-mode", required_argument, NULL, OptBorderMode},
    {"cross-buf-mode", required_argument, NULL, OptCrossBufMode},
    {"set-bg-val", required_argument, NULL, OptSetBgVal},
    {"fps", required_argument, NULL, OptSetFps},
    {"api-test",  no_argument, NULL, OptEnableApiTest},
    {"stream-skip", required_argument, NULL, OptStreamSkip},
    {"offset-x", required_argument, NULL, OptSetOffsetX},
    {"stride", required_argument, NULL, OptSetStride},
    {"offset-x-out", required_argument, NULL, OptSetOutOffsetX},
    {"stride-out", required_argument, NULL, OptSetOutStride},
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
        "  --stream-skip <count>\n"
        "                     skip the first <count> buffers. The default is 0.\n"
        "  --stream-from <file> stream from this file.\n"
        "  --stream-to <file>\n"
        "                     stream to this file. The default is to discard the data.\n"
        "  --mesh-from <file> mesh from this file.\n"
        "  --calib-ini-from <file> calib from this ini.\n"
        "  --auto-parse-ini <val> auto parse ini.\n"
        "  --mesh-density <val> mesh density.\n"
        "                       0: 32x16. 1: 16x8. 2: 4x4\n"
        "  --border-mode <val> border mode.\n"
        "                       0: fill with bg_value. 1: copy with the nearest pixel."
        "  --cross-buf-mode <val> cross buffer mode\n"
        "                       0: fill with bg_value. 1: copy with the nearest pixel."
        "  --set-bg-val <val> y=<y>,u=<u>,v=<v>\n"
        "                     fill with the bg_val."
        "  -s, --silent       subpress debug log.\n"
        "  -f, --fps <val>    set fps(1-1000).\n"
        "  -t, --api-test     enable api test.\n"
        "  --offset-x <val>      set offset_x to video.\n"
        "  --stride <val>       set stride to video.\n"
        "  --offset-x-out <val>      set out_offset_x to video out.\n"
        "  --stride-out <val>       set out_stride to video out.\n"
        "  --api-test <val> enable api test.\n"
        "  -h, --help         display this help message.\n");
}

struct mem_ops rk_mem_ops_tbl[] = {
    {
        .init      = init_drm,
        .deinit    = deinit_drm,
        .alloc     = alloc_drm_buffer,
        .free      = free_drm_buffer,
        .need_sync = false,
    },
#ifdef RKFEC_HW_V20
    {
        .init      = rk_ispfec_api_vb_init,
        .deinit    = rk_ispfec_api_vb_deinit,
        .alloc     = rk_ispfec_api_vb_alloc,
        .free      = rk_ispfec_api_vb_free,
        .need_sync = true,
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

int init_ispfec_bufs(rk_ispfec_cfg_t* cfg) {
    int ret    = 0;

    ret = ispfec_mem_init();
    if (ret < 0) {
        stderr_info("failed to init mem\n");
        return ret;
    }

    int mesh_size = rk_ispfec_api_calFecMeshsize(cfg->out_width, cfg->out_height);
    // 1. alloc mesh buffer
#ifdef RKFEC_HW_V20
    for (int i = 0; i < MESH_BUF_NUM; i++) {
        ret = cur_mem_pos->alloc(dma_node_fd, mesh_size * 6, &g_rkfec_buf_mesh[i]);
        if (ret) goto deinit_mem;
        info("mesh fd:%d size:%" PRId64 "\n", g_rkfec_buf_mesh[i].dmabuf_fd,
             g_rkfec_buf_mesh[i].size);
    }
#else
    ret = cur_mem_pos->alloc(dma_node_fd, mesh_size * 2, &g_rkfec_buf_xint);
    if (ret) goto deinit_mem;
    info("xint fd:%d size:%" PRId64 "\n", g_rkfec_buf_xint.dmabuf_fd, g_rkfec_buf_xint.size);

    ret = cur_mem_pos->alloc(dma_node_fd, mesh_size, &g_rkfec_buf_xfra);
    if (ret) goto free_drm_buf_xint;
    info("xfra fd:%d size:%" PRId64 "\n", g_rkfec_buf_xfra.dmabuf_fd, g_rkfec_buf_xfra.size);

    ret = cur_mem_pos->alloc(dma_node_fd, mesh_size * 2, &g_rkfec_buf_yint);
    if (ret) goto free_drm_buf_xfra;
    info("yint fd:%d size:%" PRId64 "\n", g_rkfec_buf_yint.dmabuf_fd, g_rkfec_buf_yint.size);

    ret = cur_mem_pos->alloc(dma_node_fd, mesh_size, &g_rkfec_buf_yfra);
    if (ret) goto free_drm_buf_yint;
    info("yfra fd:%d size:%" PRId64 "\n", g_rkfec_buf_yfra.dmabuf_fd, g_rkfec_buf_yfra.size);
#endif

    // 2. alloc input picture buffer
    int size = MAX(cfg->in_width, cfg->in_stride) * cfg->in_height * 3 / 2;
    ret = cur_mem_pos->alloc(dma_node_fd, size,
                             &g_rkfec_buf_pic_in);
    if (ret) goto free_drm_buf_mesh;
    info("in pic fd:%d size: %" PRId64 "\n", g_rkfec_buf_pic_in.dmabuf_fd, g_rkfec_buf_pic_in.size);

    // 3. alloc output picture buffer
    int out_width_max = MAX(cfg->out_width, cfg->out_stride);
    if (out_pixfmt == V4L2_PIX_FMT_FBC0) {
        int stride_y = (out_width_max + 63) / 64 * 384;
        int stride_c = (out_width_max + 63) / 64 * 16;
        int size     = (stride_y * cfg->out_height + stride_c * cfg->out_height) / 4;

        ret = cur_mem_pos->alloc(dma_node_fd, size, &g_rkfec_buf_pic_out);
	g_rkfec_buf_pic_out.size = size;
    } else {
        ret = cur_mem_pos->alloc(dma_node_fd, out_width_max * (cfg->in_height + 1) * 3 / 2,
                                 &g_rkfec_buf_pic_out);
	g_rkfec_buf_pic_out.size = out_width_max * cfg->out_height * 3 / 2;
    }
    if (ret) goto free_drm_buf_pic_in;
    info("out pic fd:%d size: %" PRId64 "\n", g_rkfec_buf_pic_out.dmabuf_fd, g_rkfec_buf_pic_out.size);

#ifdef RKFEC_HW_V20
    cfg->mesh_info.dmaFd    = g_rkfec_buf_mesh[0].dmabuf_fd;
    cfg->mesh_info.size     = g_rkfec_buf_mesh[0].size;
    cfg->mesh_info.vir_addr = g_rkfec_buf_mesh[0].map;
#else
    cfg->mesh_xint.dmaFd    = g_rkfec_buf_xint.dmabuf_fd;
    cfg->mesh_xint.size     = g_rkfec_buf_xint.size;
    cfg->mesh_xint.vir_addr = g_rkfec_buf_xint.map;

    cfg->mesh_xfra.dmaFd    = g_rkfec_buf_xfra.dmabuf_fd;
    cfg->mesh_xfra.size     = g_rkfec_buf_xfra.size;
    cfg->mesh_xfra.vir_addr = g_rkfec_buf_xfra.map;

    cfg->mesh_yint.dmaFd    = g_rkfec_buf_yint.dmabuf_fd;
    cfg->mesh_yint.size     = g_rkfec_buf_yint.size;
    cfg->mesh_yint.vir_addr = g_rkfec_buf_yint.map;

    cfg->mesh_yfra.dmaFd    = g_rkfec_buf_yfra.dmabuf_fd;
    cfg->mesh_yfra.size     = g_rkfec_buf_yfra.size;
    cfg->mesh_yfra.vir_addr = g_rkfec_buf_yfra.map;
#endif

    return ret;

free_drm_buf_pic_in:
    cur_mem_pos->free(dma_node_fd, &g_rkfec_buf_pic_in);
free_drm_buf_mesh:
#ifdef RKFEC_HW_V20
    for (int i = 0; i < MESH_BUF_NUM; i++) cur_mem_pos->free(dma_node_fd, &g_rkfec_buf_mesh[i]);
#else
    cur_mem_pos->free(dma_node_fd, &g_rkfec_buf_yfra);
free_drm_buf_yint:
    cur_mem_pos->free(dma_node_fd, &g_rkfec_buf_yint);
free_drm_buf_xfra:
    cur_mem_pos->free(dma_node_fd, &g_rkfec_buf_xfra);
free_drm_buf_xint:
    cur_mem_pos->free(dma_node_fd, &g_rkfec_buf_xint);
#endif
deinit_mem:
    rkfec_mem_deinit();

    return ret;
}

void deinit_ispfec_bufs() {
#ifdef RKFEC_HW_V20
    for (int i = 0; i < MESH_BUF_NUM; i++) cur_mem_pos->free(dma_node_fd, &g_rkfec_buf_mesh[i]);
#else
    cur_mem_pos->free(dma_node_fd, &g_rkfec_buf_yfra);
    cur_mem_pos->free(dma_node_fd, &g_rkfec_buf_yint);
    cur_mem_pos->free(dma_node_fd, &g_rkfec_buf_xfra);
    cur_mem_pos->free(dma_node_fd, &g_rkfec_buf_xint);
#endif
    cur_mem_pos->free(dma_node_fd, &g_rkfec_buf_pic_in);
    cur_mem_pos->free(dma_node_fd, &g_rkfec_buf_pic_out);
    rkfec_mem_deinit();
}

size_t read_image_from_file(const char* file_path, char* buffer, size_t buffer_size) {
    FILE* file = fopen(file_path, "r");
    int ret    = 0;

    if (file == NULL) {
        stderr_info("Could not open %s for reading\n", file_path);
        return 0;
    }

#ifdef RKFEC_HW_V20
    if (cur_mem_pos->need_sync) {
        ret = rk_ispfec_api_vb_sync_start(g_rkfec_buf_pic_in.dmabuf_fd, 1);
        if (ret) stderr_info("DMA input sync(pre-write) failed: %s\n", strerror(errno));
    }
#endif

    size_t bytes_read = fread(buffer, 1, buffer_size - 1, file);

#ifdef RKFEC_HW_V20
    if (cur_mem_pos->need_sync) {
        ret = rk_ispfec_api_vb_sync_end(g_rkfec_buf_pic_in.dmabuf_fd, 1);
        if (ret) stderr_info("DMA input sync(post-write) failed: %s\n", strerror(errno));
    }
#endif

    fclose(file);
    return bytes_read;
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

int do_handle_fec(FILE* fout) {
    char ch = '<';
    int ret;

    ret = rk_ispfec_api_process(g_ispfec_ctx, g_rkfec_buf_pic_in.dmabuf_fd,
                                g_rkfec_buf_pic_out.dmabuf_fd);
    if (ret) {
        stderr_info("rk_ispfec_api_process failed: %s\n", strerror(errno));
        return -1;
    }

    if (fout && !stream_skip) {
#ifdef RKFEC_HW_V20
        if (cur_mem_pos->need_sync) {
            ret = rk_ispfec_api_vb_sync_start(g_rkfec_buf_pic_out.dmabuf_fd, 0);
            if (ret) stderr_info("DMA output sync(pre-read) failed: %s\n", strerror(errno));
        }
#endif

        ret = write_buffer_to_file(fout, g_rkfec_buf_pic_out.map, g_rkfec_buf_pic_out.size);
        if (ret <= 0) {
            stderr_info("write_buffer_to_file failed: %s\n", strerror(errno));
            return -1;
        }

#ifdef RKFEC_HW_V20
        if (cur_mem_pos->need_sync) {
            ret = rk_ispfec_api_vb_sync_end(g_rkfec_buf_pic_out.dmabuf_fd, 0);
            if (ret) stderr_info("DMA output sync(post-read) failed: %s\n", strerror(errno));
        }
#endif

        stderr_info("%c\n", ch);
        fflush(stderr);
    }

    if (stream_skip) {
        stream_skip--;
        return 0;
    }

    if (stream_count == 0) return 0;

    if (--stream_count == 0) return -1;

    return 0;
}

void parse_path(const char* path, char* directory, char* filename) {
    // 找到最后一个 '/' 的位置
    const char* last_slash = strrchr(path, '/');
    if (last_slash) {
        // 提取目录部分
        size_t dir_len = last_slash - path + 1;  // 包括最后的 '/'
        strncpy(directory, path, dir_len);
        directory[dir_len] = '\0';  // 确保字符串以 '\0' 结尾

        // 提取文件名部分
        strcpy(filename, last_slash + 1);
    } else {
        // 如果没有 '/'，认为路径中只有文件名
        directory[0] = '\0';  // 目录为空
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

static int ini_file_handler(void* user, const char* section, const char* name, const char* value) {
    FecCailbParams* p = (FecCailbParams*)user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

    if (MATCH("Calibration", "sw_ldcC_image_width")) {
        p->calib_image_width = atoi(value);
    } else if (MATCH("Calibration", "sw_ldcC_image_height")) {
        p->calib_image_height = atoi(value);
    } else if (MATCH("Calibration", "sw_ldcC_opticCenter_x")) {
        p->optic_center_x = atof(value);
    } else if (MATCH("Calibration", "sw_ldcC_opticCenter_y")) {
        p->optic_center_y = atof(value);
    } else if (MATCH("Calibration", "sw_ldcC_lensDistor_coeff0")) {
        p->coeff[0] = strtod(value, NULL);
    } else if (MATCH("Calibration", "sw_ldcC_lensDistor_coeff1")) {
        p->coeff[1] = strtod(value, NULL);
    } else if (MATCH("Calibration", "sw_ldcC_lensDistor_coeff2")) {
        p->coeff[2] = strtod(value, NULL);
    } else if (MATCH("Calibration", "sw_ldcC_lensDistor_coeff3")) {
        p->coeff[3] = strtod(value, NULL);
    } else if (MATCH("Calibration", "sw_ldcC_correctStrg_maxLimit")) {
        p->correctStrg_maxLimit = atoi(value);
    } else if (MATCH("Config", "sw_ldcT_correctStrg_val")) {
        p->correctStrg_val = atoi(value);
    } else if (MATCH("Config", "sw_ldcT_saveMaxFovX_bit")) {
        p->saveMaxFovX = atoi(value);
    }

    return 1;
}

static int parse_fmt(char* optarg, int* width, int* height, int* pixelformat) {
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
// restore terminal settings
void restore_terminal_settings(void) {
    // Apply saved settings
    tcsetattr(0, TCSANOW, &oldt);
}

// make terminal read 1 char at a time
void disable_terminal_return(void) {
    struct termios newt;

    // save terminal settings
    tcgetattr(0, &oldt);
    // init new settings
    newt = oldt;
    // change settings
    newt.c_lflag &= ~(ICANON | ECHO);
    // apply settings
    tcsetattr(0, TCSANOW, &newt);

    // make sure settings will be restored when program ends
    atexit(restore_terminal_settings);
}

static int load_mesh_file(const char* file, void* vir_addr, size_t size) {
    FILE* fp = fopen(file, "rb");
    int ret  = 0;

    if (fp) {
        ret = fseek(fp, 0L, SEEK_END);
        if (ret < 0) {
            stderr_info("%s fseek to end failed\n", file);
            fclose(fp);
            return ret;
        }

        size_t len = ftell(fp);
        rewind(fp);

        if (len > size) {
            stderr_info("mesh file len %zu > buf size %zu, limit read to %zu bytes only\n", len,
                        size, size);
            len = size;
        }

        size_t rd_size = fread(vir_addr, 1, len, fp);
        if (rd_size < len) stderr_info("file read size:%zu < %zu\n", rd_size, len);

        info("mesh file len %zu, read size %zu\n", len, rd_size);
        fclose(fp);
    } else {
        stderr_info("open file %s failed: %s\n", file, strerror(errno));
    }

    return ret;
}

static int test_api_set_cfg(rk_ispfec_ctx_t* ctx) {
    int ret             = 0;
    rk_ispfec_cfg_t cfg = {0};

    for (int i = 0; i < 10; i++) {
        ret = rk_ispfec_api_get_cfg(ctx, &cfg);
        if (ret) {
            stderr_info("rk_ispfec_api_get_cfg failed: %s\n", strerror(errno));
            return ret;
        }

        cfg.u.mesh_online.correct_level = (cfg.u.mesh_online.correct_level == 0) ? 200 : 0;

        ret = rk_ispfec_api_set_cfg(ctx, &cfg);
        if (ret) {
            stderr_info("rk_ispfec_api_set_cfg failed: %s\n", strerror(errno));
            return ret;
        }
        info("set cfg: correct_level = %d\n", cfg.u.mesh_online.correct_level);

        // This is a simple way to ensure that the parameter update is complete.
        usleep(100000);  // 100ms delay
        info("\n");
    }
    return 0;
}

static int test_api_set_mesh_gen_online(rk_ispfec_ctx_t* ctx) {
    int ret                          = 0;
    rk_ispfec_mesh_config_t mesh_cfg = {};

    for (int i = 0; i < 10; i++) {
        if ((ret = rk_ispfec_api_get_mesh_config(ctx, &mesh_cfg))) {
            stderr_info("Get mesh config failed: %s\n", strerror(errno));
            return ret;
        }

        mesh_cfg.mode = RK_ISPFEC_UPDATE_MESH_ONLINE;
        mesh_cfg.params.online_cfg.correct_level =
            (mesh_cfg.params.online_cfg.correct_level == 0) ? 200 : 0;

        if ((ret = rk_ispfec_api_set_mesh_config(ctx, &mesh_cfg))) {
            stderr_info("Set mesh config failed: %s\n", strerror(errno));
            return ret;
        }

        // This is a simple way to ensure that the parameter update is complete.
        usleep(100000);
        info("\n");
    }
    return 0;
}

static int test_api_set_mesh_from_file(rk_ispfec_ctx_t* ctx) {
    int ret                          = 0;
    rk_ispfec_mesh_config_t mesh_cfg = {};

    for (int i = 0; i < 10; i++) {
        if ((ret = rk_ispfec_api_get_mesh_config(ctx, &mesh_cfg))) {
            stderr_info("Get mesh config failed: %s\n", strerror(errno));
            return ret;
        }


#ifdef RKFEC_HW_V20
        info("Get mesh path: %s\n", mesh_cfg.params.file_cfg.mesh_path);
        // Toggle between two mesh file paths
        const char* new_path = (strstr(mesh_cfg.params.file_cfg.mesh_path, IN_MESH_FILE_1))
                                   ? IN_MESH_FILE_2
                                   : IN_MESH_FILE_1;
        strncpy(mesh_cfg.params.file_cfg.mesh_path, new_path,
                sizeof(mesh_cfg.params.file_cfg.mesh_path));

        info("Set mesh config: %s\n", mesh_cfg.params.file_cfg.mesh_path);
#else
        ret = -ENOTSUP;  // Feature not supported
        break;
#endif

        mesh_cfg.mode = RK_ISPFEC_UPDATE_MESH_FROM_FILE;
        if ((ret = rk_ispfec_api_set_mesh_config(ctx, &mesh_cfg))) {
            stderr_info("Set mesh config failed: %s\n", strerror(errno));
            return ret;
        }


        // This is a simple way to ensure that the parameter update is complete.
        usleep(300000);
        info("\n");
    }
    return ret;
}

static int test_api_set_mesh_from_vaddr(rk_ispfec_ctx_t* ctx) {
    int ret                          = 0;
    rk_ispfec_mesh_config_t mesh_cfg = {};
    static char cur_file[256];
    const int mesh_size = rk_ispfec_api_calFecMeshsize(out_width, out_height) * 6;
    void* mesh_buf      = malloc(mesh_size);

    if (!mesh_buf) {
        stderr_info("Memory allocation failed for %d-byte mesh buffer\n", mesh_size);
        return -ENOMEM;
    }

    for (int i = 0; i < 10; i++) {
        if ((ret = rk_ispfec_api_get_mesh_config(ctx, &mesh_cfg))) {
            stderr_info("Get config failed: %s\n", strerror(errno));
            break;
        }

#ifdef RKFEC_HW_V20
        // Configure virtual address parameters
        mesh_cfg.mode                             = RK_ISPFEC_UPDATE_MESH_FROM_BUFFER;
        mesh_cfg.params.buffer_cfg.mesh.vir_addr  = mesh_buf;
        mesh_cfg.params.buffer_cfg.mesh.size      = mesh_size;
        mesh_cfg.params.buffer_cfg.mesh.need_copy = true;

        // Toggle between two mesh files
        const char* target_file =
            strstr(cur_file, IN_MESH_FILE_1) ? IN_MESH_FILE_2 : IN_MESH_FILE_1;
        if ((ret = load_mesh_file(target_file, mesh_buf, mesh_size))) {
            stderr_info("Load %s failed: %s\n", target_file, strerror(errno));
            break;
        }
        strncpy(cur_file, target_file, sizeof(cur_file));

        info("Set mesh config: %s, via vaddr\n", mesh_cfg.params.file_cfg.mesh_path);
#else
        ret = -ENOTSUP;  // Feature not supported
        break;
#endif

        if ((ret = rk_ispfec_api_set_mesh_config(ctx, &mesh_cfg))) {
            stderr_info("Set config failed: %s\n", strerror(errno));
            break;
        }

        // This is a simple way to ensure that the parameter update is complete.
        usleep(100000);  // 100ms cooldown
        info("\n");
    }

    free(mesh_buf);
    return ret;
}

static int test_api_set_mesh_from_dmafd(rk_ispfec_ctx_t* ctx) {
    int ret                          = 0;
    rk_ispfec_mesh_config_t mesh_cfg = {};
    static char cur_file[256];

    for (int i = 0; i < 10; i++) {
        if ((ret = rk_ispfec_api_get_mesh_config(ctx, &mesh_cfg))) {
            stderr_info("Get config failed: %s\n", strerror(errno));
            break;
        }

#ifdef RKFEC_HW_V20
        const int buf_idx       = (strstr(cur_file, IN_MESH_FILE_1)) ? 1 : 0;
        const char* target_file = (buf_idx) ? IN_MESH_FILE_2 : IN_MESH_FILE_1;

        mesh_cfg.params.buffer_cfg.mesh.dma_fd    = g_rkfec_buf_mesh[buf_idx].dmabuf_fd;
        mesh_cfg.params.buffer_cfg.mesh.vir_addr  = g_rkfec_buf_mesh[buf_idx].map;
        mesh_cfg.params.buffer_cfg.mesh.size      = g_rkfec_buf_mesh[buf_idx].size;
        mesh_cfg.params.buffer_cfg.mesh.need_copy = false;

        if ((ret = load_mesh_file(target_file, mesh_cfg.params.buffer_cfg.mesh.vir_addr,
                                  mesh_cfg.params.buffer_cfg.mesh.size))) {
            stderr_info("Load %s failed: %s\n", target_file, strerror(errno));
            break;
        }

        strncpy(cur_file, target_file, sizeof(cur_file));
        info("Loaded %s with DMA FD %d\n", target_file, mesh_cfg.params.buffer_cfg.mesh.dma_fd);
#else
        ret = -ENOTSUP;  // Feature not supported
        break;
#endif

        mesh_cfg.mode = RK_ISPFEC_UPDATE_MESH_FROM_BUFFER;
        if ((ret = rk_ispfec_api_set_mesh_config(ctx, &mesh_cfg))) {
            stderr_info("Set config failed: %s\n", strerror(errno));
            break;
        }

        // This is a simple way to ensure that the parameter update is complete.
        usleep(100000);
        info("\n");
    }
    return ret;
}

static int test_api_get_mesh_config(rk_ispfec_ctx_t* ctx) {
    int ret                          = 0;
    rk_ispfec_mesh_config_t mesh_cfg = {};

    if ((ret = rk_ispfec_api_get_mesh_config(ctx, &mesh_cfg))) {
        stderr_info("Get mesh config failed: %s\n", strerror(errno));
        return ret;
    }

    info("Get mesh config: mode: %d\n", mesh_cfg.mode);

    info("Get mesh online_config: calib_width: %d, calib_height: %d\n",
         mesh_cfg.params.online_cfg.calib_width, mesh_cfg.params.online_cfg.calib_height);
    info("Get mesh online_config: optic_center: %.4f %.4f\n",
         mesh_cfg.params.online_cfg.light_center[0], mesh_cfg.params.online_cfg.light_center[1]);
    info("Get mesh online_config: coeff: %.16f %.16f %.16f %.16f\n",
         mesh_cfg.params.online_cfg.coeff[0], mesh_cfg.params.online_cfg.coeff[1],
         mesh_cfg.params.online_cfg.coeff[2], mesh_cfg.params.online_cfg.coeff[3]);

    info("Get mesh online_config: level: %d\n", mesh_cfg.params.online_cfg.correct_level);

    return 0;
}

static void test_rkfec_params(rk_ispfec_ctx_t* ctx) {
    if (ctx == NULL) {
        return;
    }

    int ret = 0;
    int key = getchar();
    info("press key=[%c]\n", key);

    switch (key) {
        case '1':
            info("==== set_cfg API Test ====\n");
            if ((ret = test_api_set_cfg(ctx)) != 0) {
                stderr_info("set_cfg API test failed\n");
            }
            break;
        case '2':
            info("==== set_mesh API Test: Online Mesh Generation ====\n");
            if ((ret = test_api_set_mesh_gen_online(ctx)) != 0) {
                stderr_info("Online generation test failed\n");
            }
            break;
        case '3':
            info("==== set_mesh API Test: Mesh from file ====\n");
            if ((ret = test_api_set_mesh_from_file(ctx)) != 0) {
                stderr_info("File-based config test failed\n");
            }
            break;
        case '4':
            info("==== set_mesh API Test: Mesh from vaddr ====\n");
            if ((ret = test_api_set_mesh_from_vaddr(ctx)) != 0) {
                stderr_info("Virtual address config test failed\n");
            }
            break;
        case '5':
            info("==== set_mesh API Test: Mesh from DMA FD ====\n");
            if ((ret = test_api_set_mesh_from_dmafd(ctx)) != 0) {
                stderr_info("DMA FD config test failed\n");
            }
            break;
        case '6':
            info("==== get_mesh API Test: Get mesh config ====\n");
            if ((ret = test_api_get_mesh_config(ctx)) != 0) {
                stderr_info("Get mesh config test failed\n");
            }
            break;
        default:
            stderr_info("Not support key %d\n", key);
            break;
    }
}

static void* test_uapi_thread(void* args) {
    disable_terminal_return();
    while (!is_quit) {
        test_rkfec_params((rk_ispfec_ctx_t*)args);
    }
    restore_terminal_settings();
    return 0;
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
            case OptStreamSkip:
                stream_skip = strtoul(optarg, NULL, 0);
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
            case OptCalibFrom:
                calib_ini_from = optarg;
                break;
            case OptAutoParseIni:
                auto_parse_ini = strtoul(optarg, NULL, 0);
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
                if (ret < 0) stderr_info("Failed to parse bg_val\n");
                break;
            case OptSetFps:
                target_fps = atoi(optarg);
                if (target_fps < 1 || target_fps > 1000) {
                     stderr_info("Invalid fps %d\n", target_fps);
                }
                target_interval_ns = 1000000000L / target_fps;
                break;
            case OptEnableApiTest:
                enable_uapi_test = true;
                break;
            case OptSetOffsetX:
                offset_x = atoi(optarg);
                break;
            case OptSetStride:
                stride = atoi(optarg);
                break;
            case OptSetOutOffsetX:
                out_offset_x = atoi(optarg);
                break;
            case OptSetOutStride:
                out_stride = atoi(optarg);
                break;
            default:
                break;
        }
    }
    if (optind < argc) {
        stderr_info("unknown arguments: ");
        while (optind < argc) stderr_info("%s ", argv[optind++]);
        stderr_info("\n");
        common_usage();
        exit(EXIT_FAILURE);
    }

    ISP_FEC_CONFIG_INIT(g_ispfec_cfg);

    g_ispfec_cfg.in_width     = width;
    g_ispfec_cfg.in_height    = height;
    g_ispfec_cfg.out_width    = out_width;
    g_ispfec_cfg.out_height   = out_height;
    g_ispfec_cfg.in_fourcc    = pixfmt;
    g_ispfec_cfg.out_fourcc   = out_pixfmt;
    g_ispfec_cfg.in_offset_x  = offset_x;
    g_ispfec_cfg.in_stride    = stride;
    g_ispfec_cfg.out_offset_x = out_offset_x;
    g_ispfec_cfg.out_stride   = out_stride;
#ifdef RKFEC_HW_V20
    g_ispfec_cfg.border_mode    = border_mode;
    g_ispfec_cfg.cross_buf_mode = cross_buf_mode;
    g_ispfec_cfg.bg_val         = bg_val;
#endif

    if (mesh_from) {
        g_ispfec_cfg.mesh_upd_mode = RK_ISPFEC_UPDATE_MESH_FROM_FILE;
#ifdef RKFEC_HW_V20
        parse_path(mesh_from, g_ispfec_cfg.u.mesh_file_path, g_ispfec_cfg.mesh_info.mesh_file);
#else
        strcpy(g_ispfec_cfg.u.mesh_file_path, mesh_from);
        strcpy(g_ispfec_cfg.mesh_xint.mesh_file, "meshxi_level0.bin");
        strcpy(g_ispfec_cfg.mesh_xfra.mesh_file, "meshxf_level0.bin");
        strcpy(g_ispfec_cfg.mesh_yint.mesh_file, "meshyi_level0.bin");
        strcpy(g_ispfec_cfg.mesh_yfra.mesh_file, "meshyf_level0.bin");
#endif
    } else if (calib_ini_from) {
        if (auto_parse_ini) {
            g_ispfec_cfg.mesh_upd_mode = RK_ISPFEC_UPDATE_MESH_ONLINE_FROM_INI;
            strncpy(g_ispfec_cfg.calib_ini_from, calib_ini_from,
                    sizeof(g_ispfec_cfg.calib_ini_from));
        } else {
            gen_mesh_online_info_t info_out = {0};
            if (rk_ispfec_api_load_online_config_from_ini(calib_ini_from, &info_out)) {
                stderr_info("Failed to load online config from ini file\n");
                return -1;
            }

            g_ispfec_cfg.mesh_upd_mode = RK_ISPFEC_UPDATE_MESH_ONLINE;

            g_ispfec_cfg.u.mesh_online.calib_width  = info_out.calib_width;
            g_ispfec_cfg.u.mesh_online.calib_height = info_out.calib_height;

            g_ispfec_cfg.u.mesh_online.light_center[0]      = info_out.light_center[0];
            g_ispfec_cfg.u.mesh_online.light_center[1]      = info_out.light_center[1];
            g_ispfec_cfg.u.mesh_online.coeff[0]             = info_out.coeff[0];
            g_ispfec_cfg.u.mesh_online.coeff[1]             = info_out.coeff[1];
            g_ispfec_cfg.u.mesh_online.coeff[2]             = info_out.coeff[2];
            g_ispfec_cfg.u.mesh_online.coeff[3]             = info_out.coeff[3];
            g_ispfec_cfg.u.mesh_online.calib_level_maxLimit = info_out.calib_level_maxLimit;

            g_ispfec_cfg.u.mesh_online.correct_level = info_out.correct_level;
            g_ispfec_cfg.u.mesh_online.direction     = info_out.direction;
            g_ispfec_cfg.u.mesh_online.style         = info_out.style;

            info("calib_ini_from: %s\n", calib_ini_from);

            info("calib_width: %d\n", g_ispfec_cfg.u.mesh_online.calib_width);
            info("light_center: %.4f %.4f\n", g_ispfec_cfg.u.mesh_online.light_center[0],
                 g_ispfec_cfg.u.mesh_online.light_center[1]);
            info("coeff: %.16f %.16f %.16f %.16f\n", g_ispfec_cfg.u.mesh_online.coeff[0],
                 g_ispfec_cfg.u.mesh_online.coeff[1], g_ispfec_cfg.u.mesh_online.coeff[2],
                 g_ispfec_cfg.u.mesh_online.coeff[3]);
            info("calib_level_maxLimit: %d\n", g_ispfec_cfg.u.mesh_online.calib_level_maxLimit);

            info("correct_level: %d\n", g_ispfec_cfg.u.mesh_online.correct_level);
            info("direction: %d\n", g_ispfec_cfg.u.mesh_online.direction);
            info("style: %d\n", g_ispfec_cfg.u.mesh_online.style);
        }
    } else {
        stderr_info("Please specify the calibration file.\n");
        return -1;
    }

    int ret = init_ispfec_bufs(&g_ispfec_cfg);
    if (ret < 0) {
        stderr_info("init_ispfec_bufs failed: %s\n", strerror(errno));
        return -1;
    }

    g_ispfec_ctx = rk_ispfec_api_init(&g_ispfec_cfg);
    if (!g_ispfec_ctx) {
        stderr_info("rk_ispfec_api_init failed: %s\n", strerror(errno));
        goto error_init;
    }

    pthread_t api_thread;
    if (enable_uapi_test) {
        info("create thread for uapi test...\n");
        ret = pthread_create(&api_thread, NULL, test_uapi_thread, g_ispfec_ctx);
        if (ret) stderr_info("pthread_create failed: %s\n", strerror(errno));
    }

    if (!file_from) file_from = IN_IMAGE_FILE;

    ret = read_image_from_file(file_from, g_rkfec_buf_pic_in.map, g_rkfec_buf_pic_in.size);
    if (ret <= 0) {
        stderr_info("Failed to read image from %s, ret: %s\n", file_from, strerror(errno));
        goto clean_up;
    }

    fout = open_output_file();

    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);

    while (1) {
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);

        ret = do_handle_fec(fout);
        if (ret < 0) break;

        next.tv_nsec += target_interval_ns;
        const long carry = next.tv_nsec / 1000000000L;
        next.tv_sec += carry;
        next.tv_nsec = next.tv_nsec % 1000000000L;

        FEC_STATIC_FPS_CALCULATION(sample_fec, target_fps);
    }

    is_quit = true;

    if (enable_uapi_test) pthread_join(api_thread, NULL);
clean_up:
    if (fout) fclose(fout);
    rk_ispfec_api_deinit(g_ispfec_ctx);
error_init:
    deinit_ispfec_bufs();
    g_ispfec_ctx = NULL;

    return 0;
}
