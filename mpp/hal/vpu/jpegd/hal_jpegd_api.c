/*
 * Copyright 2017 Rockchip Electronics Co. LTD
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

#define MODULE_TAG "hal_jpegd_api"

#include <string.h>

#include "mpp_env.h"
#include "mpp_debug.h"
#include "mpp_common.h"
#include "osal_2str.h"

#include "mpp_hal.h"
#include "mpp_platform.h"
#include "hal_jpegd_base.h"
#include "hal_jpegd_vdpu2.h"
#include "hal_jpegd_vdpu1.h"
#include "hal_jpegd_rkv.h"

static MPP_RET hal_jpegd_reg_gen(void *hal, HalTaskInfo *task)
{
    JpegdHalCtx *self = (JpegdHalCtx *)hal;
    return self->hal_api.reg_gen (hal, task);
}

static MPP_RET hal_jpegd_start(void *hal, HalTaskInfo *task)
{
    JpegdHalCtx *self = (JpegdHalCtx *)hal;
    return self->hal_api.start (hal, task);
}

static MPP_RET hal_jpegd_wait(void *hal, HalTaskInfo *task)
{
    JpegdHalCtx *self = (JpegdHalCtx *)hal;
    return self->hal_api.wait (hal, task);
}

static MPP_RET hal_jpegd_reset(void *hal)
{
    JpegdHalCtx *self = (JpegdHalCtx *)hal;
    return self->hal_api.reset (hal);
}

static MPP_RET hal_jpegd_flush(void *hal)
{
    JpegdHalCtx *self = (JpegdHalCtx *)hal;
    return self->hal_api.flush (hal);
}

static MPP_RET hal_jpegd_control(void *hal, MpiCmd cmd_type, void *param)
{
    JpegdHalCtx *self = (JpegdHalCtx *)hal;
    return self->hal_api.control (hal, cmd_type, param);
}

static MPP_RET hal_jpegd_deinit(void *hal)
{
    JpegdHalCtx *self = (JpegdHalCtx *)hal;
    return self->hal_api.deinit (hal);
}

static MPP_RET hal_jpegd_init(void *hal, MppHalCfg *cfg)
{
    MPP_RET ret = MPP_ERR_UNKNOW;
    JpegdHalCtx *self = (JpegdHalCtx *)hal;
    MppHalApi *p_api = NULL;
    MppClientType client_type = VPU_CLIENT_BUTT;
    MppDecBaseCfg *base = &cfg->cfg->base;
    RK_S32 hw_type = -1;
    RK_U32 hw_flag = 0;

    if (NULL == self)
        return MPP_ERR_VALUE;

    memset(self, 0, sizeof(JpegdHalCtx));

    p_api = &self->hal_api;

    hw_flag = mpp_get_vcodec_type();

    if (mpp_check_soc_cap(base->type, base->coding))
        hw_type = base->hw_type;

    if (hw_type > 0) {
        if (hw_flag & (1 << hw_type)) {
            mpp_log("init with %s hw\n", strof_client_type(hw_type));
            client_type = hw_type;
        } else
            mpp_err_f("invalid hw_type %d with vcodec_type %08x\n",
                      hw_type, hw_flag);
    }

    if (client_type == VPU_CLIENT_BUTT) {
        if (hw_flag & HAVE_VDPU2)
            client_type = VPU_CLIENT_VDPU2;
        if (hw_flag & HAVE_VDPU1)
            client_type = VPU_CLIENT_VDPU1;
        if (hw_flag & HAVE_VDPU2_PP)
            client_type = VPU_CLIENT_VDPU2_PP;
        if (hw_flag & HAVE_VDPU1_PP)
            client_type = VPU_CLIENT_VDPU1_PP;
        if (hw_flag & HAVE_JPEG_DEC)
            client_type = VPU_CLIENT_JPEG_DEC;
    }

    mpp_env_get_u32("jpegd_mode", &client_type, client_type);

    switch (client_type) {
    case VPU_CLIENT_VDPU2 :
    case VPU_CLIENT_VDPU2_PP : {
        p_api->init = hal_jpegd_vdpu2_init;
        p_api->deinit = hal_jpegd_vdpu2_deinit;
        p_api->reg_gen = hal_jpegd_vdpu2_gen_regs;
        p_api->start = hal_jpegd_vdpu2_start;
        p_api->wait = hal_jpegd_vdpu2_wait;
        p_api->reset = hal_jpegd_vdpu2_reset;
        p_api->flush = hal_jpegd_vdpu2_flush;
        p_api->control = hal_jpegd_vdpu2_control;
    } break;
    case VPU_CLIENT_VDPU1 :
    case VPU_CLIENT_VDPU1_PP : {
        p_api->init = hal_jpegd_vdpu1_init;
        p_api->deinit = hal_jpegd_vdpu1_deinit;
        p_api->reg_gen = hal_jpegd_vdpu1_gen_regs;
        p_api->start = hal_jpegd_vdpu1_start;
        p_api->wait = hal_jpegd_vdpu1_wait;
        p_api->reset = hal_jpegd_vdpu1_reset;
        p_api->flush = hal_jpegd_vdpu1_flush;
        p_api->control = hal_jpegd_vdpu1_control;
    } break;
    case VPU_CLIENT_JPEG_DEC : {
        p_api->init = hal_jpegd_rkv_init;
        p_api->deinit = hal_jpegd_rkv_deinit;
        p_api->reg_gen = hal_jpegd_rkv_gen_regs;
        p_api->start = hal_jpegd_rkv_start;
        p_api->wait = hal_jpegd_rkv_wait;
        p_api->reset = NULL;
        p_api->flush = NULL;
        p_api->control = hal_jpegd_rkv_control;
    } break;
    default : {
        return MPP_ERR_INIT;
    } break;
    }

    ret = mpp_dev_init(&cfg->dev, client_type);
    if (ret) {
        mpp_err("mpp_dev_init failed ret: %d\n", ret);
        goto __RETURN;
    }

    cfg->hw_info = mpp_get_dec_hw_info_by_client_type(client_type);
    self->hw_info = cfg->hw_info;
    self->dev = cfg->dev;

    ret = p_api->init(hal, cfg);
    if (ret) {
        mpp_err("init device with client_type %d failed!\n", client_type);
        mpp_dev_deinit(cfg->dev);
    }

__RETURN:
    return ret;
}

const MppHalApi hal_api_jpegd = {
    .name = "jpegd",
    .type = MPP_CTX_DEC,
    .coding = MPP_VIDEO_CodingMJPEG,
    .ctx_size = sizeof(JpegdHalCtx),
    .flag = 0,
    .init = hal_jpegd_init,
    .deinit = hal_jpegd_deinit,
    .reg_gen = hal_jpegd_reg_gen,
    .start = hal_jpegd_start,
    .wait = hal_jpegd_wait,
    .reset = hal_jpegd_reset,
    .flush = hal_jpegd_flush,
    .control = hal_jpegd_control,
};
