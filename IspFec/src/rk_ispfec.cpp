/*
 *  Copyright (c) 2022 Rockchip Corporation
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

#include <errno.h>
#include <libgen.h>
#include <linux/videodev2.h>
#include <rk-video-format.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <list>

#include "RkIspFecComm.h"
#include "RkIspFecGenMesh.h"
#include "RkIspFecHwMgr.h"
#include "RkIspFecVersion.h"
#include "param.h"
#include "rk_ispfec_api.h"

#ifdef __cplusplus
extern "C" {
#endif

using namespace RKISPFEC;

typedef struct rk_ispfec_ctx_s {
    rk_ispfec_cfg_t _cfg;
    bool _isCfg;
    RkIspFecHwMgr* _fecHwMgr;
    enum {
        STATE_IDLE,
        STATE_RUNNING,
        STATE_QUIT,
    };
    pthread_mutex_t _mutex;
    int _state;

    RkIspFecGenMesh* _fecGenMesh;

    std::list<int> buf_list;
} rk_ispfec_ctx_t;

static void _print_versions() {
    printf(
        ""
        "\n************************** VERSION INFOS **************************\n"
        "version release date: %s\n"
        "         IspFec:      v%02x.%02x.%02x\n"
        "************************ VERSION INFOS END ************************\n\n",
        RK_ISP_FEC_RELEASE_DATE, RK_ISP_FEC_API_VERSION_MAJOR(RK_ISP_FEC_API_VERSION),
        RK_ISP_FEC_API_VERSION_MINOR(RK_ISP_FEC_API_VERSION),
        RK_ISP_FEC_API_VERSION_PATCH(RK_ISP_FEC_API_VERSION));
}

static int
_load_mesh(const char* file, void* vir_addr, size_t size)
{
    FILE *fp = fopen(file, "rb");
    int ret = 0;

    if (fp) {
        ret = fseek(fp, 0L, SEEK_END);
        if (ret < 0) {
            rkfec_err("%s fseek to end failed", file);
            fclose(fp);
            return ret;
        }

        size_t len = ftell(fp);
        rewind(fp);

        if (len > size) {
            rkfec_err("mesh file len %zu > buf size %zu, limit read to %zu bytes only", len, size,
                      size);
            len = size;
        }
#if 0
        void* map = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, dstFd, 0);

        if (map == MAP_FAILED) {
            rkfec_err("E: failed to mmap buffer");
            ret = -1;
        } else {
            size_t rd_size = fread(map, 1, len, fp);
            if (rd_size < len)
                rkfec_err("file read size:%zu < %zu ", rd_size, len);
            munmap(map, len);
        }
#else
        size_t rd_size = fread(vir_addr, 1, len, fp);
        if (rd_size < len) rkfec_err("file read size:%zu < %zu ", rd_size, len);

        rkfec_info("mesh file len %zu, read size %zu", len, rd_size);
#endif
        fclose(fp);
    } else {
        rkfec_err("E: open file %s failed: %s ", file, strerror(errno));
    }

    return ret;
}

static int _validate_file_path(const char* path) {
    if (!path || path[0] == '\0') {
        rkfec_dbg(4, rkfec_debug, "Empty path\n");
        return -1;
    }

    if (access(path, F_OK) != 0) {
        rkfec_dbg(4, rkfec_debug, "File not exist: %s\n", path);
        return -1;
    }

    if (access(path, R_OK) != 0) {
        rkfec_dbg(4, rkfec_debug, "No read permission: %s\n", path);
        return -1;
    }

    struct stat st;
    if (stat(path, &st) == 0 && !S_ISREG(st.st_mode)) {
        rkfec_dbg(4, rkfec_debug, "Not a regular file: %s\n", path);
        return -1;
    }

    return 0;
}

static int _load_calib_ini(rk_ispfec_cfg_t* cfg) {
    if (_validate_file_path(cfg->calib_ini_from)) {
        rkfec_err("calib ini path is Invalid: %s", cfg->calib_ini_from);
        return -1;
    }

    rk_ispfec_param_init(cfg->calib_ini_from);
    rkfec_dbg(3, rkfec_debug, "calib ini path: %s", cfg->calib_ini_from);

    cfg->u.mesh_online.calib_width  = rk_ispfec_param_get_int("Calibration:sw_ldcC_image_width", 0);
    cfg->u.mesh_online.calib_height = rk_ispfec_param_get_int("Calibration:sw_ldcC_image_height", 0);

    cfg->u.mesh_online.light_center[0] =
        rk_ispfec_param_get_double("Calibration:sw_ldcC_opticCenter_x", 0);
    cfg->u.mesh_online.light_center[1] =
        rk_ispfec_param_get_double("Calibration:sw_ldcC_opticCenter_y", 0);
    cfg->u.mesh_online.coeff[0] =
        rk_ispfec_param_get_double("Calibration:sw_ldcC_lensDistor_coeff0", 0);
    cfg->u.mesh_online.coeff[1] =
        rk_ispfec_param_get_double("Calibration:sw_ldcC_lensDistor_coeff1", 0);
    cfg->u.mesh_online.coeff[2] =
        rk_ispfec_param_get_double("Calibration:sw_ldcC_lensDistor_coeff2", 0);
    cfg->u.mesh_online.coeff[3] =
        rk_ispfec_param_get_double("Calibration:sw_ldcC_lensDistor_coeff3", 0);
    cfg->u.mesh_online.calib_level_maxLimit =
        rk_ispfec_param_get_int("Calibration:sw_ldcC_correctStrg_maxLimit", 0);

    cfg->u.mesh_online.correct_level = rk_ispfec_param_get_int("Config:sw_ldcT_correctstrg_val", 0);
    cfg->u.mesh_online.style = (enum rk_ispfec_correct_style)rk_ispfec_param_get_int(
        "config:sw_ldcT_saveMaxFovX_bit", 0);

    rkfec_dbg(3, rkfec_debug, "calib_width: %d, calib_height: %d", cfg->u.mesh_online.calib_width,
              cfg->u.mesh_online.calib_height);
    rkfec_dbg(3, rkfec_debug, "light_center: %.16f, %.16f", cfg->u.mesh_online.light_center[0],
              cfg->u.mesh_online.light_center[1]);
    rkfec_dbg(3, rkfec_debug, "coeff: %.16f, %.16f, %.16f, %.16f", cfg->u.mesh_online.coeff[0],
              cfg->u.mesh_online.coeff[1], cfg->u.mesh_online.coeff[2],
              cfg->u.mesh_online.coeff[3]);
    rkfec_dbg(3, rkfec_debug, "calib_level_maxLimit: %d", cfg->u.mesh_online.calib_level_maxLimit);

    rkfec_dbg(3, rkfec_debug, "style: %d", cfg->u.mesh_online.style);
    rkfec_dbg(3, rkfec_debug, "correct_level: %d", cfg->u.mesh_online.correct_level);

    return 0;
}

static int
_configure(rk_ispfec_ctx_t* ctx, rk_ispfec_cfg_t* cfg)
{
    #define MESH_FILE_MAX_LEN 256
    int ret = 0;

    pthread_mutex_lock (&ctx->_mutex);

    static const char* const upd_mesh_mode[] = {"mesh_online", "mesh_from_file", "mesh_from_buffer",
                                                "mesh_online_from_ini"};

    rkfec_info("rkfec mesh update mode: %s", upd_mesh_mode[cfg->mesh_upd_mode]);

    if (cfg->mesh_upd_mode == RK_ISPFEC_UPDATE_MESH_ONLINE_FROM_INI) _load_calib_ini(cfg);

    if (cfg->mesh_upd_mode == RK_ISPFEC_UPDATE_MESH_ONLINE ||
        cfg->mesh_upd_mode == RK_ISPFEC_UPDATE_MESH_ONLINE_FROM_INI) {
#ifdef GENMESH_ONLINE
        if (!ctx->_fecGenMesh) {
            ctx->_fecGenMesh = new RkIspFecGenMesh();
            if (!ctx->_fecGenMesh) {
                rkfec_err("Failed to new RkIspFecGenMesh");
                goto err;
            }
        }

        if (cfg->u.mesh_online.direction == 0)
            cfg->u.mesh_online.direction = RK_ISPFEC_CORRECT_DIRECTION_XY;

        ret = ctx->_fecGenMesh->init(cfg->in_width, cfg->in_height, cfg->out_width, cfg->out_height,
                                cfg->u.mesh_online.light_center, cfg->u.mesh_online.coeff,
                                cfg->u.mesh_online.direction, cfg->u.mesh_online.style);
        if (ret < 0) {
            rkfec_err("Failed to init RkIspFecGenMesh");
            goto err;
        }

#ifdef RKFEC_HW_V20
        ctx->_fecGenMesh->setMeshBuf(cfg->mesh_info.vir_addr);
#else
        ctx->_fecGenMesh->setMeshBuf(cfg->mesh_xint.vir_addr, cfg->mesh_yint.vir_addr,
                                     cfg->mesh_xfra.vir_addr, cfg->mesh_yfra.vir_addr);
#endif

        if (ctx->_state == rk_ispfec_ctx_t::STATE_IDLE ||
            cfg->u.mesh_online.correct_level != ctx->_cfg.u.mesh_online.correct_level) {
            rkfec_dbg(3, rkfec_debug, "correct level changed from %d to %d",
                      ctx->_cfg.u.mesh_online.correct_level, cfg->u.mesh_online.correct_level);

            ret = ctx->_fecGenMesh->genMesh(cfg->u.mesh_online.correct_level);
            if (ret < 0) {
                rkfec_err("Failed to genMesh RkIspFecGenMesh");
                goto err;
            }
        }
#endif
    } else if (cfg->mesh_upd_mode == RK_ISPFEC_UPDATE_MESH_FROM_FILE) {
        char mesh_file[MESH_FILE_MAX_LEN] = {'\0'};

        if (ctx == NULL)
            return -1;

#ifdef RKFEC_HW_V20
        snprintf(mesh_file, MESH_FILE_MAX_LEN, "%s/%s", cfg->u.mesh_file_path,
                 cfg->mesh_info.mesh_file);
        ret = _load_mesh(mesh_file, cfg->mesh_info.vir_addr, cfg->mesh_info.size);
        if (ret) {
            rkfec_err("load mesh error!");
            goto err;
        }
#else
        // load mesh_xint
        snprintf(mesh_file, MESH_FILE_MAX_LEN, "%s/%s", cfg->u.mesh_file_path,
                 cfg->mesh_xint.mesh_file);
        ret = _load_mesh(mesh_file, cfg->mesh_xint.vir_addr, cfg->mesh_xint.size);
        if (ret) {
            rkfec_err("load mesh_xint error!");
            goto err;
        }
        // load mesh_xfra
        snprintf(mesh_file, MESH_FILE_MAX_LEN, "%s/%s", cfg->u.mesh_file_path,
                 cfg->mesh_xfra.mesh_file);
        ret = _load_mesh(mesh_file, cfg->mesh_xfra.vir_addr, cfg->mesh_xfra.size);
        if (ret) {
            rkfec_err("load mesh_xfra error!");
            goto err;
        }

        // load mesh_yint
        snprintf(mesh_file, MESH_FILE_MAX_LEN, "%s/%s", cfg->u.mesh_file_path,
                 cfg->mesh_yint.mesh_file);
        ret = _load_mesh(mesh_file, cfg->mesh_yint.vir_addr, cfg->mesh_yint.size);
        if (ret) {
            rkfec_err("load mesh_yint error!");
            goto err;
        }

        // load mesh_yfra
        snprintf(mesh_file, MESH_FILE_MAX_LEN, "%s/%s", cfg->u.mesh_file_path,
                 cfg->mesh_yfra.mesh_file);
        ret = _load_mesh(mesh_file, cfg->mesh_yfra.vir_addr, cfg->mesh_yfra.size);
        if (ret) {
            rkfec_err("load mesh_yfra error!");
            goto err;
        }
#endif
    }

    memcpy(&ctx->_cfg, cfg, sizeof(rk_ispfec_cfg_t));
    ctx->_isCfg = true;

    pthread_mutex_unlock (&ctx->_mutex);

    return 0;
err:
    pthread_mutex_unlock (&ctx->_mutex);
    return -1;
}

static void _add_buffer(rk_ispfec_ctx_t* ctx, int fd) {
    if (!ctx || fd < 0) return;

    auto it = std::find(ctx->buf_list.begin(), ctx->buf_list.end(), fd);
    if (it != ctx->buf_list.end()) {
        rkfec_dbg(5, rkfec_debug, "FD:%d already in list", fd);
        return;
    }

    ctx->buf_list.push_back(fd);
    rkfec_dbg(1, rkfec_debug, "Added FD:%d to list", fd);
}

static void _free_buffer(rk_ispfec_ctx_t* ctx, int fd, bool free_all) {
    if (!ctx) return;

    auto it = ctx->buf_list.begin();
    while (it != ctx->buf_list.end()) {
        if (free_all || *it == fd) {
            rkfec_dbg(1, rkfec_debug, "Freed FD:%d from list", *it);
            ctx->_fecHwMgr->detach_dma_buffer(*it);
            it = ctx->buf_list.erase(it);
            if (!free_all) return;
        } else {
            ++it;
        }
    }
}

void
rk_ispfec_api_deinit(rk_ispfec_ctx_t* ctx)
{
    if (ctx == NULL)
        return ;

    pthread_mutex_lock (&ctx->_mutex);
    while (ctx->_state == rk_ispfec_ctx_t::STATE_RUNNING) {
        usleep(1 * 1000);
        rkfec_info("in Running state, wait to IDLE !");
    }

    ctx->_state = rk_ispfec_ctx_t::STATE_QUIT;
    pthread_mutex_unlock (&ctx->_mutex);

#ifdef GENMESH_ONLINE
    if (ctx->_fecGenMesh) {
        ctx->_fecGenMesh->deinit();
        delete ctx->_fecGenMesh;
    }
#endif

    rk_ispfec_param_deinit();
    _free_buffer(ctx, 0, true);
    ctx->_fecHwMgr->deinit();
    pthread_mutex_destroy (&ctx->_mutex);

    delete ctx;

    return;
}

rk_ispfec_ctx_t*
rk_ispfec_api_init(rk_ispfec_cfg_t* cfg)
{
    _print_versions();

    if (RK_ISP_FEC_API_VERSION_MAJOR(cfg->version) !=
            RK_ISP_FEC_API_VERSION_MAJOR(RK_ISP_FEC_API_VERSION) ||
        cfg->size != sizeof(rk_ispfec_cfg_s)) {
        rkfec_err("Invalid config (version:v%02x.%02x.%02x/size:%d), required v%02x.%02x.%02x/%zu",
                  RK_ISP_FEC_API_VERSION_MAJOR(cfg->version),
                  RK_ISP_FEC_API_VERSION_MINOR(cfg->version),
                  RK_ISP_FEC_API_VERSION_PATCH(cfg->version), cfg->size,
                  RK_ISP_FEC_API_VERSION_MAJOR(RK_ISP_FEC_API_VERSION),
                  RK_ISP_FEC_API_VERSION_MINOR(RK_ISP_FEC_API_VERSION),
                  RK_ISP_FEC_API_VERSION_PATCH(RK_ISP_FEC_API_VERSION), sizeof(rk_ispfec_cfg_s));
        return NULL;
    }

    rkfec_debug = rkfec_read_sysfs_int(RKFEC_DEBUG_LEVEL_PATH);
    rkfec_info("RkIspFec debug level: %d", rkfec_debug);

    rk_ispfec_ctx_t* ctx = new rk_ispfec_ctx_t();

    ctx->_state = rk_ispfec_ctx_t::STATE_IDLE;

    int ret = pthread_mutex_init (&ctx->_mutex, NULL);
    if (ret) {
        rkfec_err("Mutex init failed: %d", ret);
        goto err;
    }
    ctx->_isCfg = false;
    ctx->_fecGenMesh = NULL;
    ctx->_fecHwMgr = RkIspFecHwMgr::getInstance();
    if (!ctx->_fecHwMgr) {
        rkfec_err("get RkIspFecHwMgr failed: %d", ret);
        goto err;
    }

    if (cfg) {
        ret = _configure(ctx, cfg);
        if (ret) {
            rkfec_err("configure failed: %d", ret);
            goto err;
        }
    }

    return ctx;
err:
    if (ctx) {
        rk_ispfec_api_deinit(ctx);
    }

    return NULL;
}

int
rk_ispfec_api_prepare(rk_ispfec_ctx_t* ctx, rk_ispfec_cfg_t* cfg)
{
    int ret = 0;

    if (ctx == NULL)
        return -1;

#if 0
    while (ctx->_state == rk_ispfec_ctx_t::STATE_RUNNING) {
        usleep(1 * 1000);
        rkfec_info("in Running state, wait to IDLE !");
    }
#endif


    if (cfg) {
        ret = _configure(ctx, cfg);
        if (ret)
            ctx->_isCfg = false;
    }


    return ret;
}

int
rk_ispfec_api_process(rk_ispfec_ctx_t* ctx, int src_fd, int dst_fd)
{
    int ret = 0;
#ifdef RKFEC_HW_V20
    struct rkfec_in_out param = {0};
#else
    struct rkispp_fec_in_out param;
#endif

    if (ctx == NULL)
        return -1;

    Profiler prof = {0};
    if (rkfec_debug > 4) rkfec_profiling_start(&prof);

    pthread_mutex_lock (&ctx->_mutex);
    if (ctx->_state == rk_ispfec_ctx_t::STATE_QUIT) {
        pthread_mutex_unlock (&ctx->_mutex);
        return 0;
    }
    ctx->_state = rk_ispfec_ctx_t::STATE_RUNNING;

    if (!ctx->_isCfg) {
        rkfec_err("not configured !");
        goto out;
    }

#ifdef RKFEC_HW_V20
    param.in_width   = ctx->_cfg.in_width;
    param.in_height  = ctx->_cfg.in_height;
    param.out_width  = ctx->_cfg.out_width;
    param.out_height = ctx->_cfg.out_height;
    param.in_fourcc  = ctx->_cfg.in_fourcc;
    param.out_fourcc = ctx->_cfg.out_fourcc;

    param.buf_cfg.in_pic_fd  = src_fd;
    param.buf_cfg.out_pic_fd = dst_fd;
    param.buf_cfg.in_size    = param.in_width * param.in_height * 3 / 2;
    if (param.out_fourcc == V4L2_PIX_FMT_FBC0) {
        int stride_y = (param.out_width + 63) / 64 * 384;
        int stride_c = (param.out_width + 63) / 64 * 16;

        param.buf_cfg.out_size = (stride_y * param.out_height + stride_c * param.out_height) / 4;
    } else {
        param.buf_cfg.out_size = param.out_width * param.out_height * 3 / 2;
    }

    param.buf_cfg.in_offs    = RKFEC_CLAMP(ctx->_cfg.in_offset_x, 0, param.in_width);
    param.buf_cfg.in_stride  = ctx->_cfg.in_stride ? ctx->_cfg.in_stride : param.in_width;
    param.buf_cfg.out_offs   = RKFEC_CLAMP(ctx->_cfg.out_offset_x, 0, param.out_width);
    param.buf_cfg.out_stride = ctx->_cfg.out_stride ? ctx->_cfg.out_stride : param.out_width;
    param.buf_cfg.lut_fd     = ctx->_cfg.mesh_info.dmaFd;
    param.buf_cfg.lut_size   = rk_ispfec_api_calFecMeshsize(param.out_width, param.out_height);

    param.core_ctrl.density  = ctx->_cfg.out_width >= 1920 ? 0 : ctx->_cfg.out_width >= 512 ? 1 : 2;
    param.core_ctrl.bic_mode = 0;
    param.core_ctrl.border_mode  = ctx->_cfg.border_mode;
    param.core_ctrl.pbuf_crs_dis = 0;
    param.core_ctrl.buf_mode     = ctx->_cfg.cross_buf_mode;

    memcpy(&param.bg_val, &ctx->_cfg.bg_val, sizeof(rk_ispfec_bg_val));

    _add_buffer(ctx, param.buf_cfg.lut_fd);
    _add_buffer(ctx, param.buf_cfg.in_pic_fd);
    _add_buffer(ctx, param.buf_cfg.out_pic_fd);

    rkfec_dbg(3, rkfec_debug,
              "mode: border %d, cross_buf: %d, bg_val: %d, %d, %d, offs(in: %d, out: %d), "
              "stride(in: %d, out: %d)",
              param.core_ctrl.border_mode, param.core_ctrl.buf_mode, param.bg_val.bg_y,
              param.bg_val.bg_u, param.bg_val.bg_v, param.buf_cfg.in_offs, param.buf_cfg.out_offs,
              param.buf_cfg.in_stride, param.buf_cfg.out_stride);
    rkfec_dbg(3, rkfec_debug, "lut_fd: %d, lut_size: %d", param.buf_cfg.lut_fd,
              param.buf_cfg.lut_size);
#else
    param.in_width      = ctx->_cfg.in_width;
    param.in_height     = ctx->_cfg.in_height;
    param.out_width     = ctx->_cfg.out_width;
    param.out_height    = ctx->_cfg.out_height;
    param.in_fourcc     = V4L2_PIX_FMT_NV12;
    param.out_fourcc    = V4L2_PIX_FMT_NV12;
    param.in_pic_fd     = src_fd;
    param.out_pic_fd    = dst_fd;
    param.mesh_xint_fd  = ctx->_cfg.mesh_xint.dmaFd;
    param.mesh_xfra_fd  = ctx->_cfg.mesh_xfra.dmaFd;
    param.mesh_yint_fd  = ctx->_cfg.mesh_yint.dmaFd;
    param.mesh_yfra_fd  = ctx->_cfg.mesh_yfra.dmaFd;

    _add_buffer(ctx, param.mesh_xint_fd);
    _add_buffer(ctx, param.mesh_xfra_fd);
    _add_buffer(ctx, param.mesh_yint_fd);
    _add_buffer(ctx, param.mesh_yfra_fd);
    _add_buffer(ctx, param.in_pic_fd);
    _add_buffer(ctx, param.out_pic_fd);
#endif

    rkfec_dbg(3, rkfec_debug, "%dx%d->%dx%d format(in:%c%c%c%c out:%c%c%c%c)", param.in_width,
              param.in_height, param.out_width, param.out_height, param.in_fourcc,
              param.in_fourcc >> 8, param.in_fourcc >> 16, param.in_fourcc >> 24, param.out_fourcc,
              param.out_fourcc >> 8, param.out_fourcc >> 16, param.out_fourcc >> 24);

    ret = ctx->_fecHwMgr->process(param);
    if (ret) {
        rkfec_err("process error:%d", ret);
    }

out:
    ctx->_state = rk_ispfec_ctx_t::STATE_IDLE;
    pthread_mutex_unlock (&ctx->_mutex);

    if (rkfec_debug > 4) rkfec_profiling_end(&prof, "rk_ispfec_api_proces", 30);

    return ret;
}

#ifdef RKFEC_HW_V20
int rk_ispfec_api_calFecMeshsize(int width, int height) {
    int block_w = 16, block_h = 8;
    int density = width >= 1920 ? 0 : width >= 512 ? 1 : 2;

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

    rkfec_dbg(3, rkfec_debug, "lut info: %dx%d, desity %d, size %d", lut_w, lut_h, density,
              lut_size);

    return lut_size;
}
#else
int
rk_ispfec_api_calFecMeshsize(int width, int height)
{
    int mesh_size, mesh_left_height;
    int w = 32 * ((width + 31) / 32);
    int h = 32 * ((height + 31) / 32);
    int spb_num = (h + 127) >> 7;
    int left_height = h & 127;
    bool density = (width > 1920) ? true : false;
    int mesh_width = density ? (w / 32 + 1) : (w / 16 + 1);
    int mesh_height = density ? 9 : 17;

    if (!left_height)
        left_height = 128;

    mesh_left_height = density ? (left_height / 16 + 1) : (left_height / 8 + 1);
    mesh_size = (spb_num - 1) * mesh_width * mesh_height + mesh_width * mesh_left_height;

    return mesh_size;
}
#endif

int rk_ispfec_api_get_cfg(rk_ispfec_ctx_t* ctx, rk_ispfec_cfg_t* cfg) {
    if (!ctx || !cfg) return -1;

    pthread_mutex_lock(&ctx->_mutex);
    memcpy(cfg, &ctx->_cfg, sizeof(rk_ispfec_cfg_t));
    pthread_mutex_unlock(&ctx->_mutex);

    return 0;
}

int rk_ispfec_api_set_cfg(rk_ispfec_ctx_t* ctx, rk_ispfec_cfg_t* cfg) {
    int ret = 0;

    if (!ctx || !cfg) return -1;

    if (cfg) {
        ret = _configure(ctx, cfg);
        if (ret) ctx->_isCfg = false;
    }

    return ret;
}

/**
 * @brief Updates FEC mesh configuration based on input parameters
 * @param ctx FEC context pointer
 * @param config Mesh configuration parameters
 * @return 0 on success, negative error code on failure
 *
 * Handles three operation modes:
 * 1. Online mesh generation using calibration data
 * 2. Mesh loading from file system
 * 3. DMA buffer configuration with copy control
 */
static int _update_mesh_config(rk_ispfec_ctx_t* ctx, rk_ispfec_mesh_config_t* config) {
    int ret             = 0;

    if (!ctx || !config) return -EINVAL;

    // Update base configuration mode
    ctx->_cfg.mesh_upd_mode = config->mode;

    switch (config->mode) {
        case RK_ISPFEC_UPDATE_MESH_ONLINE:
        case RK_ISPFEC_UPDATE_MESH_ONLINE_FROM_INI:
#ifdef GENMESH_ONLINE
            // Copy online generation parameters
            if (ctx->_fecGenMesh) {
                // Generate new mesh data
                ret = ctx->_fecGenMesh->genMesh(config->params.online_cfg.correct_level);
                if (ret < 0) {
                    rkfec_err("Mesh generation failed: %d", ret);
                    return ret;
                }

                memcpy(&ctx->_cfg.u.mesh_online, &config->params.online_cfg,
                       sizeof(gen_mesh_online_info_t));
            } else {
                rkfec_err("Online mesh generation not initialized");
                return -EINVAL;
            }
#else
            rkfec_err("Online mesh generation not supported");
            return -ENOTSUP;
#endif
            break;

        case RK_ISPFEC_UPDATE_MESH_FROM_FILE: {
#ifdef RKFEC_HW_V20
            // Load V20 single mesh file
            char tmp_path[256] = {0};
            snprintf(tmp_path, sizeof(tmp_path), "%s", config->params.file_cfg.mesh_path);
            char* base_name = basename(tmp_path);
            char* dir_name  = dirname(tmp_path);

            strncpy(ctx->_cfg.u.mesh_file_path, dir_name, sizeof(ctx->_cfg.u.mesh_file_path));
            strncpy(ctx->_cfg.mesh_info.mesh_file, base_name,
                    sizeof(ctx->_cfg.mesh_info.mesh_file));

            if ((ret = _load_mesh(config->params.file_cfg.mesh_path, ctx->_cfg.mesh_info.vir_addr,
                                  ctx->_cfg.mesh_info.size))) {
                rkfec_err("V20 mesh load failed: %s", config->params.file_cfg.mesh_path);
                return ret;
            }

#else
            // Load four component files for non-V20
            auto process_component = [&](const char* full_path, mesh_info_t& mesh_info) {
                char tmp_path[256] = {0};
                snprintf(tmp_path, sizeof(tmp_path), "%s", full_path);

                char* base_name = basename(tmp_path);
                char* dir_name  = dirname(tmp_path);

                strncpy(ctx->_cfg.u.mesh_file_path, dir_name, sizeof(ctx->_cfg.u.mesh_file_path));
                strncpy(mesh_info.mesh_file, base_name, sizeof(mesh_info.mesh_file));

                return _load_mesh(full_path, mesh_info.vir_addr, mesh_info.size);
            };

            // Process X integer mesh
            if ((ret = process_component(config->params.file_cfg.mesh_xint, ctx->_cfg.mesh_xint))) {
                rkfec_err("X integer mesh load failed");
                return ret;
            }

            // Process X fractional mesh
            if ((ret = process_component(config->params.file_cfg.mesh_xfra, ctx->_cfg.mesh_xfra))) {
                rkfec_err("X fractional mesh load failed");
                return ret;
            }

            // Process Y integer mesh
            if ((ret = process_component(config->params.file_cfg.mesh_yint, ctx->_cfg.mesh_yint))) {
                rkfec_err("Y integer mesh load failed");
                return ret;
            }

            // Process Y fractional mesh
            if ((ret = process_component(config->params.file_cfg.mesh_yfra, ctx->_cfg.mesh_yfra))) {
                rkfec_err("Y fractional mesh load failed");
                return ret;
            }
#endif
        } break;
        case RK_ISPFEC_UPDATE_MESH_FROM_BUFFER: {
#ifdef RKFEC_HW_V20
            // Handle V20 single buffer configuration
            ctx->_cfg.mesh_info.size = config->params.buffer_cfg.mesh.size;

            // Memory copy decision logic
            if (config->params.buffer_cfg.mesh.need_copy) {
                if (ctx->_cfg.mesh_info.vir_addr && config->params.buffer_cfg.mesh.vir_addr) {
                    size_t copy_size =
                        std::min(ctx->_cfg.mesh_info.size,
                                 static_cast<int>(config->params.buffer_cfg.mesh.size));
                    memcpy(ctx->_cfg.mesh_info.vir_addr, config->params.buffer_cfg.mesh.vir_addr,
                           copy_size);
                    rkfec_dbg(3, rkfec_debug, "Copied %zu bytes to V20 buffer", copy_size);
                } else {
                    rkfec_err("Invalid buffer addresses for copy");
                    return -EINVAL;
                }
            } else {
                ctx->_cfg.mesh_info.dmaFd    = config->params.buffer_cfg.mesh.dma_fd;
                ctx->_cfg.mesh_info.vir_addr = config->params.buffer_cfg.mesh.vir_addr;
            }
#else
            // Handle four component buffers with copy logic
            auto process_component = [&](mesh_info_t& dst, const rk_ispfec_buffer_info_t& src) {
                dst.size = src.size;
                memset(dst.mesh_file, 0, sizeof(dst.mesh_file));

                if (src.need_copy) {
                    if (dst.vir_addr && src.vir_addr) {
                        size_t copy_size = std::min(dst.size, static_cast<int>(src.size));
                        memcpy(dst.vir_addr, src.vir_addr, copy_size);
                        rkfec_dbg(3, rkfec_debug, "Copied %zu bytes to component buffer",
                                  copy_size);
                    } else {
                        rkfec_err("Invalid component addresses: src=%p dst=%p", src.vir_addr,
                                  dst.vir_addr);
                        return -EINVAL;
                    }
                } else {
                    dst.dmaFd    = src.dma_fd;
                    dst.vir_addr = src.vir_addr;
                }
                return 0;
            };

            // Process all four components with error propagation
            if ((ret = process_component(ctx->_cfg.mesh_xint,
                                         config->params.buffer_cfg.mesh.xint)) ||
                (ret = process_component(ctx->_cfg.mesh_xfra,
                                         config->params.buffer_cfg.mesh.xfra)) ||
                (ret = process_component(ctx->_cfg.mesh_yint,
                                         config->params.buffer_cfg.mesh.yint)) ||
                (ret = process_component(ctx->_cfg.mesh_yfra,
                                         config->params.buffer_cfg.mesh.yfra))) {
                return ret;
            }
#endif
        } break;
        default:
            rkfec_err("Invalid mesh update mode: %d", config->mode);
            return -EINVAL;
    }

    return ret;
}

int rk_ispfec_api_set_mesh_config(rk_ispfec_ctx_t* ctx, rk_ispfec_mesh_config_t* config) {
    if (!ctx || !config) return -1;

    pthread_mutex_lock(&ctx->_mutex);
    int ret = _update_mesh_config(ctx, config);
    pthread_mutex_unlock(&ctx->_mutex);

    return ret;
}

int rk_ispfec_api_get_mesh_config(rk_ispfec_ctx_t* ctx, rk_ispfec_mesh_config_t* config) {
    // Validate input parameters
    if (!ctx || !config) {
        rkfec_err("Invalid parameters: ctx=%p config=%p", ctx, config);
        return -EINVAL;
    }

    pthread_mutex_lock(&ctx->_mutex);

    config->mode = ctx->_cfg.mesh_upd_mode;

    switch (config->mode) {
        case RK_ISPFEC_UPDATE_MESH_ONLINE:
        case RK_ISPFEC_UPDATE_MESH_ONLINE_FROM_INI:
            memcpy(&config->params.online_cfg, &ctx->_cfg.u.mesh_online,
                   sizeof(gen_mesh_online_info_t));
            break;

        case RK_ISPFEC_UPDATE_MESH_FROM_FILE:
#ifdef RKFEC_HW_V20
            // For V20 hardware: combine directory path and mesh filename
            snprintf(config->params.file_cfg.mesh_path, sizeof(config->params.file_cfg.mesh_path),
                     "%s/%s", ctx->_cfg.u.mesh_file_path, ctx->_cfg.mesh_info.mesh_file);
#else
            // For legacy hardware: process four component paths
            snprintf(config->params.file_cfg.mesh_xint, sizeof(config->params.file_cfg.mesh_xint),
                     "%s/%s", ctx->_cfg.u.mesh_file_path, ctx->_cfg.mesh_xint.mesh_file);
            snprintf(config->params.file_cfg.mesh_xfra, sizeof(config->params.file_cfg.mesh_xfra),
                     "%s/%s", ctx->_cfg.u.mesh_file_path, ctx->_cfg.mesh_xfra.mesh_file);
            snprintf(config->params.file_cfg.mesh_yint, sizeof(config->params.file_cfg.mesh_yint),
                     "%s/%s", ctx->_cfg.u.mesh_file_path, ctx->_cfg.mesh_yint.mesh_file);
            snprintf(config->params.file_cfg.mesh_yfra, sizeof(config->params.file_cfg.mesh_yfra),
                     "%s/%s", ctx->_cfg.u.mesh_file_path, ctx->_cfg.mesh_yfra.mesh_file);
#endif
            break;

        case RK_ISPFEC_UPDATE_MESH_FROM_BUFFER:
#ifdef RKFEC_HW_V20
            // Single buffer configuration for V20
            config->params.buffer_cfg.mesh.dma_fd    = ctx->_cfg.mesh_info.dmaFd;
            config->params.buffer_cfg.mesh.size      = ctx->_cfg.mesh_info.size;
            config->params.buffer_cfg.mesh.need_copy = false;
#else
            // Four component configuration for legacy hardware
            config->params.buffer_cfg.mesh.xint.dma_fd = ctx->_cfg.mesh_xint.dmaFd;
            config->params.buffer_cfg.mesh.xfra.dma_fd = ctx->_cfg.mesh_xfra.dmaFd;
            config->params.buffer_cfg.mesh.yint.dma_fd = ctx->_cfg.mesh_yint.dmaFd;
            config->params.buffer_cfg.mesh.yfra.dma_fd = ctx->_cfg.mesh_yfra.dmaFd;
#endif
            break;

        default:
            pthread_mutex_unlock(&ctx->_mutex);
            rkfec_err("Invalid mesh update mode: %d", config->mode);
            return -EINVAL;
    }

    pthread_mutex_unlock(&ctx->_mutex);
    return 0;
}

int rk_ispfec_api_load_online_config_from_ini(const char* ini_path,
                                              gen_mesh_online_info_t* info_out) {
    if (!ini_path || !info_out) {
        rkfec_err("Invalid parameters: ini_path=%p info_out=%p", ini_path, info_out);
        return -EINVAL;
    }

    if (_validate_file_path(ini_path)) {
        rkfec_err("calib ini path is Invalid: %s", ini_path);
        return -1;
    }

    rk_ispfec_param_init((char*)ini_path);
    rkfec_dbg(3, rkfec_debug, "calib ini path: %s", ini_path);
    info_out->calib_width     = rk_ispfec_param_get_int("Calibration:sw_ldcC_image_width", 0);
    info_out->calib_height    = rk_ispfec_param_get_int("Calibration:sw_ldcC_image_height", 0);
    info_out->light_center[0] = rk_ispfec_param_get_double("Calibration:sw_ldcC_opticCenter_x", 0);
    info_out->light_center[1] = rk_ispfec_param_get_double("Calibration:sw_ldcC_opticCenter_y", 0);
    info_out->coeff[0] = rk_ispfec_param_get_double("Calibration:sw_ldcC_lensDistor_coeff0", 0);
    info_out->coeff[1] = rk_ispfec_param_get_double("Calibration:sw_ldcC_lensDistor_coeff1", 0);
    info_out->coeff[2] = rk_ispfec_param_get_double("Calibration:sw_ldcC_lensDistor_coeff2", 0);
    info_out->coeff[3] = rk_ispfec_param_get_double("Calibration:sw_ldcC_lensDistor_coeff3", 0);
    info_out->calib_level_maxLimit =
        rk_ispfec_param_get_int("Calibration:sw_ldcC_correctStrg_maxLimit", 0);

    info_out->correct_level = rk_ispfec_param_get_int("Config:sw_ldcT_correctstrg_val", 0);
    info_out->style =
        (enum rk_ispfec_correct_style)rk_ispfec_param_get_int("Config:sw_ldcT_saveMaxFovX_bit", 0);

    rk_ispfec_param_deinit();

    rkfec_dbg(3, rkfec_debug, "%s, calib info: %d %d %.4f %.4f %.16f %.16f %.16f %.16f",
              __FUNCTION__, info_out->calib_width, info_out->calib_height,
              info_out->light_center[0], info_out->light_center[1], info_out->coeff[0],
              info_out->coeff[1], info_out->coeff[2], info_out->coeff[3]);

    return 0;
}

#ifdef __cplusplus
}
#endif
