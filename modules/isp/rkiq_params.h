/*
 * Copyright 2017 Rockchip Electronics Co., Ltd
 *     Author: Jacob Chen <jacob2.chen@rock-chips.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#ifndef __RKIQ_PARAMS_H__
#define __RKIQ_PARAMS_H__

#include "regs.h"
#include "rk_aiq.h"
#include <xcam_defs.h>
#include "linux/v4l2-controls.h"
#include <linux/rkisp.h>
#include <rk-isp-config.h>

struct AiqResults {
    rk_aiq_ae_results aeResults;
    rk_aiq_awb_results awbResults;
    rk_aiq_af_results afResults;
    rk_aiq_misc_isp_results miscIspResults;
};

XCamReturn rkisp1_check_params(struct rkisp1_isp_params_cfg *configs,
                               int isp_acq_out_width, int isp_acq_out_height,
                               int isp_ver);
XCamReturn rkisp1_convert_params(struct rkisp1_isp_params_cfg* isp_cfg,
                                            struct AiqResults* aiqResults);
XCamReturn rkisp1_convert_results(struct rkisp1_isp_params_cfg* isp_cfg,
                                            struct rkisp_parameters *aiq_results,
                                            struct rkisp_parameters &last_aiq_results);

#endif
