/*
 *  Copyright (c) 2023 Rockchip Corporation
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

#ifndef _RK_AIQ_USER_API2_ISP33_H_
#define _RK_AIQ_USER_API2_ISP33_H_

#ifndef ISP_HW_V33
#define ISP_HW_V33 1
#endif

#ifndef USE_NEWSTRUCT
#define USE_NEWSTRUCT 1
#endif

#include "uAPI2/rk_aiq_user_api2_imgproc.h"

#include "isp/rk_aiq_isp_ae25.h"
#include "uAPI2/rk_aiq_user_api2_ae.h"
#include "uAPI2/rk_aiq_user_ae_thread_v25_itf.h"
#include "isp/rk_aiq_stats_aeStats25.h"
#include "uAPI2/rk_aiq_user_api2_custom2_awb.h"
#include "isp/awb_head2.h"
#include "uAPI2/rk_aiq_user_api2_awb_v3.h"
#include "isp/rk_aiq_stats_af33.h"
#include "uAPI2/rk_aiq_user_api2_af.h"

#include "isp/rk_aiq_isp_dm24.h"
#include "uAPI2/rk_aiq_user_api2_dm.h"
#include "isp/rk_aiq_isp_btnr41.h"
#include "uAPI2/rk_aiq_user_api2_btnr.h"
#include "isp/rk_aiq_isp_gamma21.h"
#include "uAPI2/rk_aiq_user_api2_gamma.h"
#include "isp/rk_aiq_isp_ynr40.h"
#include "uAPI2/rk_aiq_user_api2_ynr.h"
#include "isp/rk_aiq_isp_drc40.h"
#include "uAPI2/rk_aiq_user_api2_drc.h"
#include "isp/rk_aiq_isp_sharp40.h"
#include "uAPI2/rk_aiq_user_api2_sharp.h"
#include "isp/rk_aiq_isp_cnr35.h"
#include "uAPI2/rk_aiq_user_api2_cnr.h"
#include "isp/rk_aiq_isp_blc30.h"
#include "uAPI2/rk_aiq_user_api2_blc.h"
#include "isp/rk_aiq_isp_trans10.h"
#include "uAPI2/rk_aiq_user_api2_trans.h"
#include "isp/rk_aiq_isp_dpc21.h"
#include "uAPI2/rk_aiq_user_api2_dpc.h"
#include "isp/rk_aiq_isp_gic30.h"
#include "uAPI2/rk_aiq_user_api2_gic.h"
#include "isp/rk_aiq_isp_histeq30.h"
#include "uAPI2/rk_aiq_user_api2_histeq.h"
#include "isp/rk_aiq_isp_enh30.h"
#include "uAPI2/rk_aiq_user_api2_enh.h"
#include "isp/rk_aiq_isp_hsv10.h"
#include "uAPI2/rk_aiq_user_api2_hsv.h"
#include "isp/rk_aiq_isp_cac30.h"
#include "uAPI2/rk_aiq_user_api2_cac.h"
#include "isp/rk_aiq_isp_csm21.h"
#include "uAPI2/rk_aiq_user_api2_csm.h"
#include "isp/rk_aiq_isp_merge22.h"
#include "uAPI2/rk_aiq_user_api2_merge.h"
#include "isp/rk_aiq_isp_lsc21.h"
#include "uAPI2/rk_aiq_user_api2_lsc.h"
#include "isp/rk_aiq_isp_cgc10.h"
#include "uAPI2/rk_aiq_user_api2_cgc.h"
#include "isp/rk_aiq_isp_cp10.h"
#include "uAPI2/rk_aiq_user_api2_cp.h"
#include "isp/rk_aiq_isp_ie10.h"
#include "uAPI2/rk_aiq_user_api2_ie.h"
#include "isp/rk_aiq_isp_gain20.h"
#include "uAPI2/rk_aiq_user_api2_gain.h"
#include "isp/rk_aiq_isp_ccm22.h"
#include "uAPI2/rk_aiq_user_api2_ccm.h"
#include "isp/rk_aiq_isp_texEst40.h"
#include "uAPI2/rk_aiq_user_api2_texEst.h"
#include "isp/rk_aiq_isp_stats_v33.h"
#include "uAPI2/rk_aiq_user_api2_stats.h"
#include "isp/rk_aiq_isp_ldc22.h"
#include "uAPI2/rk_aiq_user_api2_ldc.h"

#include "uAPI2/rk_aiq_user_api2_postisp.h"
#include "uAPI2/rk_aiq_user_api2_amtd.h"

#endif
