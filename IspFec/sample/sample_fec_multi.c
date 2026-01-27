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
 * sample_fec_multi --set-fmt-video=width=3840,height=2160,pixelformat=NV12 \
 *            --set-fmt-video-out=width=3840,height=2160,pixelformat=NV12 \
 *            --stream-count 3 \
 *            --border-mode 0 \
 *            --cross-buf-mode 0 \
 *            --stream-from "/data/pic_33_0_0_3840_2160.bin" \
 *            --stream-to "/data/output_pic_33_0_0_3840_2160.bin" \
 *            --mesh-from "/data/mesh.bin"
 *
 */

#include "sample_fec.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <inttypes.h>
#include <termios.h>
#include <pthread.h>

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
};

struct thread_args {
    int dev_id;
    rk_ispfec_ctx_t* ctx;
    FILE* fout;
};

#define IN_IMAGE_FILE "/data/imx415_4k_nv12.yuv"
#define MAX_DEV_NUM 2

static const char* file_from;
static const char* file_to[MAX_DEV_NUM];
static const char* mesh_from;
static unsigned stream_count[MAX_DEV_NUM];
static char options[OptLast];

static int width, height, out_width, out_height, pixfmt, out_pixfmt;
static int mesh_density, border_mode, cross_buf_mode;
static rk_ispfec_bg_val bg_val;

static rk_ispfec_ctx_t* g_ispfec_ctx[MAX_DEV_NUM];
static rk_ispfec_cfg_t g_ispfec_cfg[MAX_DEV_NUM];

struct rkfec_buffer g_rkfec_buf_pic_in[MAX_DEV_NUM];
struct rkfec_buffer g_rkfec_buf_pic_out[MAX_DEV_NUM];

#ifdef RKFEC_HW_V20
struct rkfec_buffer g_rkfec_buf_mesh[MAX_DEV_NUM];
#else
struct rkfec_buffer g_rkfec_buf_xint[MAX_DEV_NUM];
struct rkfec_buffer g_rkfec_buf_xfra[MAX_DEV_NUM];
struct rkfec_buffer g_rkfec_buf_yint[MAX_DEV_NUM];
struct rkfec_buffer g_rkfec_buf_yfra[MAX_DEV_NUM];
#endif

struct mem_ops* cur_mem_pos;
static int dma_node_fd = -1;

static pthread_t g_thread[MAX_DEV_NUM];
static struct thread_args g_thread_args[MAX_DEV_NUM];
static struct termios oldt;
static bool is_quit = false;

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

int init_ispfec_bufs(int dev_id, rk_ispfec_cfg_t* cfg) {
    int ret    = 0;

    int mesh_size = rk_ispfec_api_calFecMeshsize(cfg->out_width, cfg->out_height);
    // 1. alloc mesh buffer
#ifdef RKFEC_HW_V20
    ret = cur_mem_pos->alloc(dma_node_fd, mesh_size * 6, &g_rkfec_buf_mesh[dev_id]);
    if (ret) return ret;
    info("mesh fd:%d size:%" PRId64 "\n", g_rkfec_buf_mesh[dev_id].dmabuf_fd, g_rkfec_buf_mesh[dev_id].size);
#else
    ret = cur_mem_pos->alloc(dma_node_fd, mesh_size * 2, &g_rkfec_buf_xint[dev_id]);
    if (ret) return ret;
    info("xint fd:%d size:%" PRId64 "\n", g_rkfec_buf_xint[dev_id].dmabuf_fd, g_rkfec_buf_xint[dev_id].size);

    ret = cur_mem_pos->alloc(dma_node_fd, mesh_size, &g_rkfec_buf_xfra[dev_id]);
    if (ret) goto free_drm_buf_xint;
    info("xfra fd:%d size:%" PRId64 "\n", g_rkfec_buf_xfra[dev_id].dmabuf_fd, g_rkfec_buf_xfra[dev_id].size);

    ret = cur_mem_pos->alloc(dma_node_fd, mesh_size * 2, &g_rkfec_buf_yint[dev_id]);
    if (ret) goto free_drm_buf_xfra;
    info("yint fd:%d size:%" PRId64 "\n", g_rkfec_buf_yint[dev_id].dmabuf_fd, g_rkfec_buf_yint[dev_id].size);

    ret = cur_mem_pos->alloc(dma_node_fd, mesh_size, &g_rkfec_buf_yfra[dev_id]);
    if (ret) goto free_drm_buf_yint;
    info("yfra fd:%d size:%" PRId64 "\n", g_rkfec_buf_yfra[dev_id].dmabuf_fd, g_rkfec_buf_yfra[dev_id].size);
#endif

    // 2. alloc input picture buffer
    ret = cur_mem_pos->alloc(dma_node_fd, cfg->in_width * (cfg->in_height + 1) * 3 / 2,
                             &g_rkfec_buf_pic_in[dev_id]);
    if (ret) goto free_drm_buf_mesh;
    info("in pic fd:%d size: %" PRId64 "\n", g_rkfec_buf_pic_in[dev_id].dmabuf_fd, g_rkfec_buf_pic_in[dev_id].size);

    // 3. alloc output picture buffer
    if (out_pixfmt == V4L2_PIX_FMT_FBC0) {
        int stride_y = (cfg->out_width + 63) / 64 * 384;
        int stride_c = (cfg->out_width + 63) / 64 * 16;
        int size     = (stride_y * cfg->out_height + stride_c * cfg->out_height) / 4;

        ret = cur_mem_pos->alloc(dma_node_fd, size, &g_rkfec_buf_pic_out[dev_id]);
	g_rkfec_buf_pic_out[dev_id].size = size;
    } else {
        ret = cur_mem_pos->alloc(dma_node_fd, cfg->in_width * (cfg->in_height + 1) * 3 / 2,
                                 &g_rkfec_buf_pic_out[dev_id]);
	g_rkfec_buf_pic_out[dev_id].size = cfg->out_width * cfg->out_height * 3 / 2;
    }
    if (ret) goto free_drm_buf_pic_in;
    info("out pic fd:%d size: %" PRId64 "\n", g_rkfec_buf_pic_out[dev_id].dmabuf_fd, g_rkfec_buf_pic_out[dev_id].size);

#ifdef RKFEC_HW_V20
    cfg->mesh_info.dmaFd        = g_rkfec_buf_mesh[dev_id].dmabuf_fd;
    cfg->mesh_info.size         = g_rkfec_buf_mesh[dev_id].size;
    cfg->mesh_info.vir_addr     = g_rkfec_buf_mesh[dev_id].map;
#else
    cfg->mesh_xint.dmaFd    = g_rkfec_buf_xint[dev_id].dmabuf_fd;
    cfg->mesh_xint.size     = g_rkfec_buf_xint[dev_id].size;
    cfg->mesh_xint.vir_addr = g_rkfec_buf_xint[dev_id].map;

    cfg->mesh_xfra.dmaFd    = g_rkfec_buf_xfra[dev_id].dmabuf_fd;
    cfg->mesh_xfra.size     = g_rkfec_buf_xfra[dev_id].size;
    cfg->mesh_xfra.vir_addr = g_rkfec_buf_xfra[dev_id].map;

    cfg->mesh_yint.dmaFd    = g_rkfec_buf_yint[dev_id].dmabuf_fd;
    cfg->mesh_yint.size     = g_rkfec_buf_yint[dev_id].size;
    cfg->mesh_yint.vir_addr = g_rkfec_buf_yint[dev_id].map;

    cfg->mesh_yfra.dmaFd    = g_rkfec_buf_yfra[dev_id].dmabuf_fd;
    cfg->mesh_yfra.size     = g_rkfec_buf_yfra[dev_id].size;
    cfg->mesh_yfra.vir_addr = g_rkfec_buf_yfra[dev_id].map;
#endif

    return ret;

free_drm_buf_pic_in:
    cur_mem_pos->free(dma_node_fd, &g_rkfec_buf_pic_in[dev_id]);
free_drm_buf_mesh:
#ifdef RKFEC_HW_V20
    cur_mem_pos->free(dma_node_fd, &g_rkfec_buf_mesh[dev_id]);
#else
    cur_mem_pos->free(dma_node_fd, &g_rkfec_buf_yfra[dev_id]);
free_drm_buf_yint:
    cur_mem_pos->free(dma_node_fd, &g_rkfec_buf_yint[dev_id]);
free_drm_buf_xfra:
    cur_mem_pos->free(dma_node_fd, &g_rkfec_buf_xfra[dev_id]);
free_drm_buf_xint:
    cur_mem_pos->free(dma_node_fd, &g_rkfec_buf_xint[dev_id]);
#endif

    return ret;
}

void deinit_ispfec_bufs(int dev_id) {
#ifdef RKFEC_HW_V20
    cur_mem_pos->free(dma_node_fd, &g_rkfec_buf_mesh[dev_id]);
#else
    cur_mem_pos->free(dma_node_fd, &g_rkfec_buf_yfra[dev_id]);
    cur_mem_pos->free(dma_node_fd, &g_rkfec_buf_yint[dev_id]);
    cur_mem_pos->free(dma_node_fd, &g_rkfec_buf_xfra[dev_id]);
    cur_mem_pos->free(dma_node_fd, &g_rkfec_buf_xint[dev_id]);
#endif
    cur_mem_pos->free(dma_node_fd, &g_rkfec_buf_pic_in[dev_id]);
    cur_mem_pos->free(dma_node_fd, &g_rkfec_buf_pic_out[dev_id]);
}

size_t read_image_from_file(const char* file_path, char* buffer, size_t buffer_size) {
    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        stderr_info("Could not open %s for reading\n", file_path);
        return 0;
    }

    size_t bytes_read = fread(buffer, 1, buffer_size - 1, file);

    fclose(file);
    return bytes_read;
}

FILE* open_output_file(int dev_id) {
    FILE* fout = NULL;

    if (file_to[dev_id]) {
        fout = fopen(file_to[dev_id], "w+");
        if (!fout) stderr_info("Could not open %s for writing\n", file_to[dev_id]);
        return fout;
    }

    return fout;
}

size_t write_buffer_to_file(FILE* fout, const char* buffer, size_t buffer_size) {
    size_t bytes_written = fwrite(buffer, 1, buffer_size, fout);
    if (bytes_written != buffer_size) {
        stderr_info("Error writing to file\n");
    }
    return bytes_written;
}

int do_handle_fec(int dev_id, FILE* fout) {
    char ch = '<';
    int ret;

    ret = rk_ispfec_api_process(g_ispfec_ctx[dev_id], g_rkfec_buf_pic_in[dev_id].dmabuf_fd,
                                g_rkfec_buf_pic_out[dev_id].dmabuf_fd);
    if (ret) {
        stderr_info("rk_ispfec_api_process failed: %s\n", strerror(errno));
        return -1;
    }

    if (fout && stream_count[dev_id] > 0) {
        ret = write_buffer_to_file(fout, g_rkfec_buf_pic_out[dev_id].map, g_rkfec_buf_pic_out[dev_id].size);

        if (ret <= 0) {
            stderr_info("write_buffer_to_file failed: %s\n", strerror(errno));
            return -1;
        }

        stderr_info("%c\n", ch);
        fflush(stderr);
    }

    if (stream_count[dev_id] == 0) return 0;

    if (--stream_count[dev_id] == 0) return -1;

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

//restore terminal settings
void restore_terminal_settings(void)
{
    // Apply saved settings
    tcsetattr(0, TCSANOW, &oldt);
}

//make terminal read 1 char at a time
void disable_terminal_return(void)
{
    struct termios newt;

    //save terminal settings
    tcgetattr(0, &oldt);
    //init new settings
    newt = oldt;
    //change settings
    newt.c_lflag &= ~(ICANON | ECHO);
    //apply settings
    tcsetattr(0, TCSANOW, &newt);

    //make sure settings will be restored when program ends
    atexit(restore_terminal_settings);
}

static void insert_number_before_extension(const char *filename, int num, char *output) {
    const char *ext = strrchr(filename, '.'); // 找到最后一个 '.' 作为扩展名
    if (!ext) {
        // 如果没有找到 `.bin`，直接拼接数字
        sprintf(output, "%s_%d", filename, num);
        return;
    }

    // 计算主文件名长度
    size_t base_len = ext - filename;
    
    // 组合新的文件名
    snprintf(output, 256, "%.*s_%d%s", (int)base_len, filename, num, ext);
}

static void process_fec_loop(int dev_id, FILE* fout) {
    while (!is_quit) {
        int ret = do_handle_fec(dev_id, fout);
        if (ret < 0) break;
        FEC_STATIC_FPS_CALCULATION(sample_fec_multi, 30);
    }
}

static void* thread_fec_process(void* args) {
    disable_terminal_return();

    struct thread_args* thread_args = (struct thread_args*)args;
    process_fec_loop(thread_args->dev_id, thread_args->fout);

    restore_terminal_settings();
    return 0;
}

int32_t main(int argc, char** argv) {
    int32_t i;

    /* command args */
    int32_t ch;
    char short_options[6 * 2 * 3 + 1];
    int32_t idx = 0;
    char out_file[256] = {0};
    FILE* fout[MAX_DEV_NUM] = {NULL, NULL};

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
                for (int i = 0; i < MAX_DEV_NUM; i++)
                    stream_count[i] = strtoul(optarg, NULL, 0);
                break;
            case OptStreamTo:
                file_to[0] = optarg;
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

    int ret = ispfec_mem_init();
    if (ret < 0) {
        stderr_info("failed to init mem\n");
        return ret;
    }

    for (int i = 0; i < MAX_DEV_NUM; i++) {
        g_ispfec_ctx[i]         = NULL;
        g_ispfec_cfg[i].in_width   = width;
        g_ispfec_cfg[i].in_height  = height;
        g_ispfec_cfg[i].out_width  = out_width;
        g_ispfec_cfg[i].out_height = out_height;
        g_ispfec_cfg[i].in_fourcc  = pixfmt;
        g_ispfec_cfg[i].out_fourcc = out_pixfmt;
#ifdef RKFEC_HW_V20
        g_ispfec_cfg[i].border_mode    = border_mode;
        g_ispfec_cfg[i].cross_buf_mode = cross_buf_mode;
        g_ispfec_cfg[i].bg_val         = bg_val;
#endif

#if 1
        g_ispfec_cfg[i].mesh_upd_mode = RK_ISPFEC_UPDATE_MESH_FROM_FILE;
#ifdef RKFEC_HW_V20
        parse_path(mesh_from, g_ispfec_cfg[i].u.mesh_file_path, g_ispfec_cfg[i].mesh_info.mesh_file);
#else
        strcpy(g_ispfec_cfg[i].u.mesh_file_path, "/etc/iqfiles/FEC_mesh_3840_2160_imx415_3.6mm/");
        strcpy(g_ispfec_cfg[i].mesh_xint.mesh_file, "meshxi_level0.bin");
        strcpy(g_ispfec_cfg[i].mesh_xfra.mesh_file, "meshxf_level0.bin");
        strcpy(g_ispfec_cfg[i].mesh_yint.mesh_file, "meshyi_level0.bin");
        strcpy(g_ispfec_cfg[i].mesh_yfra.mesh_file, "meshyf_level0.bin");
#endif
#else
        g_ispfec_cfg[i].mesh_upd_mode                 = RK_ISPFEC_UPDATE_MESH_ONLINE;
        g_ispfec_cfg[i].u.mesh_online.light_center[0] = 1956.3909119999998438;
        g_ispfec_cfg[i].u.mesh_online.light_center[1] = 1140.6355200000000422;
        g_ispfec_cfg[i].u.mesh_online.coeff[0]        = -2819.4072493821618081;
        g_ispfec_cfg[i].u.mesh_online.coeff[1]        = 0.0000316126581792;
        g_ispfec_cfg[i].u.mesh_online.coeff[2]        = 0.0000000688410142;
        g_ispfec_cfg[i].u.mesh_online.coeff[3]        = -0.0000000000130686;
        g_ispfec_cfg[i].u.mesh_online.correct_level   = 250;
        g_ispfec_cfg[i].u.mesh_online.direction       = RK_ISPFEC_CORRECT_DIRECTION_XY;
        g_ispfec_cfg[i].u.mesh_online.style           = RK_ISPFEC_KEEP_ASPECT_RATIO_REDUCE_FOV;
#endif

        ret = init_ispfec_bufs(i, &g_ispfec_cfg[i]);
        if (ret < 0) {
            stderr_info("init_ispfec_bufs failed: %s\n", strerror(errno));
            return -1;
        }

        g_ispfec_ctx[i] = rk_ispfec_api_init(&g_ispfec_cfg[i]);
        if (!g_ispfec_ctx[i]) {
            stderr_info("rk_ispfec_api_init failed: %s\n", strerror(errno));
            goto error_init;
        }

        if (!file_from) file_from = IN_IMAGE_FILE;

        ret = read_image_from_file(file_from, g_rkfec_buf_pic_in[i].map, g_rkfec_buf_pic_in[i].size);
        if (ret <= 0) {
            stderr_info("read_image_from_file failed: %s\n", strerror(errno));
            goto clean_up;
        }

        insert_number_before_extension(file_to[0], i, out_file);
        if (file_to[i] == NULL) file_to[i] = out_file;

        fout[i] = open_output_file(i);

        g_thread_args[i].dev_id = i;
        g_thread_args[i].ctx    = g_ispfec_ctx[i];
        g_thread_args[i].fout   = fout[i];
        ret = pthread_create(&g_thread[i], NULL, thread_fec_process, &g_thread_args[i]);
        if (ret) stderr_info("pthread_create failed: %s\n", strerror(errno));
    }

    // Wait for threads to finish
    for (int i = 0; i < MAX_DEV_NUM; i++) {
        if (g_thread[i]) {
            pthread_join(g_thread[i], NULL);
            g_thread[i] = 0;
        }
    }

    is_quit = true;
clean_up:
    for (int i = 0; i < MAX_DEV_NUM; i++) {
        if (fout[i]) fclose(fout[i]);
        rk_ispfec_api_deinit(g_ispfec_ctx[i]);
    }
error_init:
    for (int i = 0; i < MAX_DEV_NUM; i++) {
        deinit_ispfec_bufs(i);
        g_ispfec_ctx[i] = NULL;
    }

    rkfec_mem_deinit();

    return 0;
}
