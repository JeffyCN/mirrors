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

#include "rk_ispfec_api.h"
#include "RkIspFecHwMgr.h"
#include "RkIspFecGenMesh.h"
#include "RkIspFecVersion.h"
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

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
} rk_ispfec_ctx_t;

static
void _print_versions()
{
    printf("\n"
         "************************** VERSION INFOS **************************\n"
         "version release date: %s\n"
         "         IspFec:       %s\n"
         "************************ VERSION INFOS END ************************\n"
         , RK_ISP_FEC_RELEASE_DATE
         , RK_ISP_FEC_VERSION
        );
}

static int
_load_mesh(const char* file, void* vir_addr, size_t size)
{
    FILE *fp = fopen(file, "rb");
    int ret = 0;

    if (fp) {
        ret = fseek(fp, 0L, SEEK_END);
        if (ret < 0) {
            printf("E: %s fseek to end failed\n", file);
            fclose(fp);
            return ret;
        }

        size_t len = ftell(fp);
        rewind(fp);

        if (len > size)
            printf("E: mesh file len %lu > buf size %lu", len, size);
#if 0
        void* map = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, dstFd, 0);

        if (map == MAP_FAILED) {
            printf("E: failed to mmap buffer\n");
            ret = -1;
        } else {
            size_t rd_size = fread(map, 1, len, fp);
            if (rd_size < len)
                printf("file read size:%lu < %lu \n", rd_size, len);
            munmap(map, len);
        }
#else
        size_t rd_size = fread(vir_addr, 1, len, fp);
        if (rd_size < len)
            printf("file read size:%lu < %lu \n", rd_size, len);
#endif
        fclose(fp);
    } else {
        printf("E: open file %s failed: %s \n", file, strerror(errno));
    }

    return ret;
}

static int
_configure(rk_ispfec_ctx_t* ctx, rk_ispfec_cfg_t* cfg)
{
    #define MESH_FILE_MAX_LEN 256
    int ret = 0;

    pthread_mutex_lock (&ctx->_mutex);

    if (cfg->mesh_upd_mode == RK_ISPFEC_UPDATE_MESH_ONLINE) {
#ifdef GENMESH_ONLINE
        if (!ctx->_fecGenMesh) {
            ctx->_fecGenMesh = new RkIspFecGenMesh();
            if (!ctx->_fecGenMesh) {
                printf("E: Failed to new RkIspFecGenMesh\n");
                goto err;
            }
        }

        ret = ctx->_fecGenMesh->init(cfg->in_width, cfg->in_height, cfg->out_width, cfg->out_height,
                                cfg->u.mesh_online.light_center, cfg->u.mesh_online.coeff,
                                cfg->u.mesh_online.direction, cfg->u.mesh_online.style);
        if (ret < 0) {
            printf("E: Failed to init RkIspFecGenMesh\n");
            goto err;
        }

        ctx->_fecGenMesh->setMeshBuf(cfg->mesh_xint.vir_addr, cfg->mesh_yint.vir_addr,
                                     cfg->mesh_xfra.vir_addr, cfg->mesh_yfra.vir_addr);

        ret = ctx->_fecGenMesh->genMesh(cfg->u.mesh_online.correct_level);
        if (ret < 0) {
            printf("E: Failed to genMesh RkIspFecGenMesh\n");
            goto err;
        }
#endif
    } else {
        char mesh_file[MESH_FILE_MAX_LEN] = {'\0'};

        if (ctx == NULL)
            return -1;

        // load mesh_xint
        snprintf(mesh_file, MESH_FILE_MAX_LEN, "%s/%s", cfg->u.mesh_file_path, cfg->mesh_xint.mesh_file);
        ret = _load_mesh(mesh_file, cfg->mesh_xint.vir_addr, cfg->mesh_xint.size);
        if (ret) {
            printf("E: load mesh_xint error!\n");
            goto err;
        }
        // load mesh_xfra
        snprintf(mesh_file, MESH_FILE_MAX_LEN, "%s/%s", cfg->u.mesh_file_path, cfg->mesh_xfra.mesh_file);
        ret = _load_mesh(mesh_file, cfg->mesh_xfra.vir_addr, cfg->mesh_xfra.size);
        if (ret) {
            printf("E: load mesh_xfra error!\n");
            goto err;
        }

        // load mesh_yint
        snprintf(mesh_file, MESH_FILE_MAX_LEN, "%s/%s", cfg->u.mesh_file_path, cfg->mesh_yint.mesh_file);
        ret = _load_mesh(mesh_file, cfg->mesh_yint.vir_addr, cfg->mesh_yint.size);
        if (ret) {
            printf("E: load mesh_yint error!\n");
            goto err;
        }

        // load mesh_yfra
        snprintf(mesh_file, MESH_FILE_MAX_LEN, "%s/%s", cfg->u.mesh_file_path, cfg->mesh_yfra.mesh_file);
        ret = _load_mesh(mesh_file, cfg->mesh_yfra.vir_addr, cfg->mesh_yfra.size);
        if (ret) {
            printf("E: load mesh_yfra error!\n");
            goto err;
        }
//#ifdef GENMESH_ONLINE
    }
//#endif

    memcpy(&ctx->_cfg, cfg, sizeof(rk_ispfec_cfg_t));
    ctx->_isCfg = true;

    pthread_mutex_unlock (&ctx->_mutex);

    return 0;
err:
    pthread_mutex_unlock (&ctx->_mutex);
    return -1;
}

void
rk_ispfec_api_deinit(rk_ispfec_ctx_t* ctx)
{
    if (ctx == NULL)
        return ;

    pthread_mutex_lock (&ctx->_mutex);
    while (ctx->_state == rk_ispfec_ctx_t::STATE_RUNNING) {
        usleep(1 * 1000);
        printf("I: in Running state, wait to IDLE ! \n");
    }

    ctx->_state = rk_ispfec_ctx_t::STATE_QUIT;
    pthread_mutex_unlock (&ctx->_mutex);

#ifdef GENMESH_ONLINE
    if (ctx->_fecGenMesh) {
        ctx->_fecGenMesh->deinit();
        delete ctx->_fecGenMesh;
    }
#endif

    ctx->_fecHwMgr->deinit();
    pthread_mutex_destroy (&ctx->_mutex);

    delete ctx;

    return;
}

rk_ispfec_ctx_t*
rk_ispfec_api_init(rk_ispfec_cfg_t* cfg)
{
    _print_versions();

    rk_ispfec_ctx_t* ctx = new rk_ispfec_ctx_t();

    ctx->_state = rk_ispfec_ctx_t::STATE_IDLE;

    int ret = pthread_mutex_init (&ctx->_mutex, NULL);
    if (ret) {
        printf("Mutex init failed: %d \n", ret);
        goto err;
    }
    ctx->_isCfg = false;
    ctx->_fecGenMesh = NULL;
    ctx->_fecHwMgr = RkIspFecHwMgr::getInstance();
    if (!ctx->_fecHwMgr) {
        printf("get RkIspFecHwMgr failed: %d \n", ret);
        goto err;
    }

    if (cfg) {
        ret = _configure(ctx, cfg);
        if (ret) {
            printf("E: configure failed: %d \n", ret);
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
        printf("I: in Running state, wait to IDLE ! \n");
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
    struct rkispp_fec_in_out param;

    if (ctx == NULL)
        return -1;

    pthread_mutex_lock (&ctx->_mutex);
    if (ctx->_state == rk_ispfec_ctx_t::STATE_QUIT) {
        pthread_mutex_unlock (&ctx->_mutex);
        return 0;
    }
    ctx->_state = rk_ispfec_ctx_t::STATE_RUNNING;

    if (!ctx->_isCfg) {
        printf("E: not configured !\n");
        goto out;
    }

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

    ret = ctx->_fecHwMgr->process(param);
    if (ret) {
        printf("E: process error:%d\n", ret);
    }

out:
    ctx->_state = rk_ispfec_ctx_t::STATE_IDLE;
    pthread_mutex_unlock (&ctx->_mutex);

    return ret;
}

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

#ifdef __cplusplus
}
#endif
