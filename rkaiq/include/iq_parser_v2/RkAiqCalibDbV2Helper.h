/*
 * Copyright (c) 2021-2022 Rockchip Eletronics Co., Ltd.
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
#ifndef ___RK_AIQ_CALIB_DB_V2_HELPER_H__
#define ___RK_AIQ_CALIB_DB_V2_HELPER_H__

#include "iq_parser/RkAiqCalibDbTypes.h"
#include "iq_parser_v2/RkAiqCalibDbTypesV2.h"

//typedef void CamCalibDbV2Context_t;

#define CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(nm) \
    CALIB_MODULE_RELATIVE_OFFSET(CamCalibDbV2ContextIsp20_t, nm)

#ifdef ISP_HW_V20
static calibdb_ctx_member_offset_info_t info_CamCalibDbV2ContextIsp20_t[] = {
    {"ae_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(ae_calib)},
    {"wb_v20", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(wb_v20)},
    {"agamma_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(agamma_calib_v10)},
    {"ablc_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(ablc_calib)},
    {"adegamma_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(adegamma_calib)},
    {"agic_calib_v20", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(agic_calib_v20)},
    {"adehaze_calib_v10", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(adehaze_calib_v10)},
    {"adpcc_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(adpcc_calib)},
    {"amerge_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(amerge_calib_v10)},
    {"atmo_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(atmo_calib)},
    {"cpsl", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(cpsl)},
    {"orb", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(orb)},
    {"bayernr_v1", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(bayernr_v1)},
    {"mfnr_v1", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(mfnr_v1)},
    {"uvnr_v1", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(uvnr_v1)},
    {"ynr_v1", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(ynr_v1)},
    {"sharp_v1", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(sharp_v1)},
    {"edgefilter_v1", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(edgefilter_v1)},
    {"debayer", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(debayer)},
    {"cproc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(cproc)},
    {"ie", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(ie)},
    {"lsc_v2", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(lsc_v2)},
    {"eis_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(eis_calib)},
    {"aldch", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(aldch)},
    {"afec", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(afec)},
    {"lumaDetect", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(lumaDetect)},
    {"colorAsGrey", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(colorAsGrey)},
    {"ccm_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(ccm_calib)},
    {"lut3d_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(lut3d_calib)},
    {"af", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(af)},
    {"thumbnails", CALIBV2_MODULE_RELATIVE_OFFSET_ISP20(thumbnails)},
    {NULL, 0},
};
#endif

#define CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(nm) \
    CALIB_MODULE_RELATIVE_OFFSET(CamCalibDbV2ContextIsp21_t, nm)

#ifdef ISP_HW_V21
static calibdb_ctx_member_offset_info_t info_CamCalibDbV2ContextIsp21_t[] = {
    {"ae_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(ae_calib)},
    {"wb_v21", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(wb_v21)},
    {"agamma_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(agamma_calib_v10)},
    {"ablc_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(ablc_calib)},
    {"adegamma_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(adegamma_calib)},
    {"agic_calib_v21", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(agic_calib_v21)},
    {"adehaze_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(adehaze_calib_v11)},
    {"adpcc_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(adpcc_calib)},
    {"amerge_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(amerge_calib_v10)},
    {"cpsl", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(cpsl)},
    {"debayer", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(debayer)},
    {"cproc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(cproc)},
    {"ie", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(ie)},
    {"lsc_v2", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(lsc_v2)},
    {"aldch", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(aldch)},
    {"lumaDetect", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(lumaDetect)},
    {"colorAsGrey", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(colorAsGrey)},
    {"ccm_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(ccm_calib)},
    {"lut3d_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(lut3d_calib)},
    {"af", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(af)},
    {"adrc_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(adrc_calib_v10)},
    {"thumbnails", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(thumbnails)},
    {"bayernr_v2", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(bayernr_v2)},
    {"cnr_v1", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(cnr_v1)},
    {"ynr_v2", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(ynr_v2)},
    {"sharp_v3", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(sharp_v3)},
    {"csm", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(csm)},
    {"cgc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP21(cgc)},
    {NULL, 0},
};
#endif

#define CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(nm) \
    CALIB_MODULE_RELATIVE_OFFSET(CamCalibDbV2ContextIsp30_t, nm)

#ifdef ISP_HW_V30
static calibdb_ctx_member_offset_info_t info_CamCalibDbV2ContextIsp30_t[] = {
    {"ae_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(ae_calib)},
    {"wb_v21", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(wb_v21)},
    {"ablc_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(ablc_calib)},
    {"adegamma_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(adegamma_calib)},
    {"agic_calib_v21", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(agic_calib_v21)},
    {"debayer", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(debayer)},
    {"colorAsGrey", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(colorAsGrey)},
    {"ccm_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(ccm_calib)},
    {"lut3d_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(lut3d_calib)},
    {"aldch", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(aldch)},
    {"adpcc_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(adpcc_calib)},
    {"ie", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(ie)},
    {"cpsl", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(cpsl)},
    {"cproc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(cproc)},
    {"amerge_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(amerge_calib_v11)},
    {"adrc_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(adrc_calib_v11)},
    {"agamma_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(agamma_calib_v11)},
    {"adehaze_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(adehaze_calib_v11)},
    {"lsc_v2", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(lsc_v2)},
    {"ynr_v3", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(ynr_v3)},
    {"cnr_v2", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(cnr_v2)},
    {"sharp_v4", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(sharp_v4)},
    {"bayer2dnr_v2", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(bayer2dnr_v2)},
    {"bayertnr_v2", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(bayertnr_v2)},
    {"cac_v03", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(cac_v03)},
    {"cac_v10", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(cac_v10)},
    {"af_v30", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(af_v30)},
    {"gain_v2", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(gain_v2)},
    {"csm", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(csm)},
    {"cgc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(cgc)},
    {"afec", CALIBV2_MODULE_RELATIVE_OFFSET_ISP30(afec)},
    {NULL, 0},
};
#endif

#define CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(nm) \
    CALIB_MODULE_RELATIVE_OFFSET(CamCalibDbV2ContextIsp32_t, nm)

#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
static calibdb_ctx_member_offset_info_t info_CamCalibDbV2ContextIsp32_t[] = {
    {"ae_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(ae_calib)},
    {"wb_v32", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(wb_v32)},
    {"ablcV32_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(ablcV32_calib)},
    {"adegamma_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(adegamma_calib)},
#if RKAIQ_HAVE_DEBAYER_V2_LITE
    {"debayer_v2_lite", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(debayer_v2_lite)},
#endif
#if RKAIQ_HAVE_DEBAYER_V2
    #ifdef USE_NEWSTRUCT
    {"demosaic", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(demosaic)},
    #else
    {"debayer_v2", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(debayer_v2)},
    #endif
#endif
    {"colorAsGrey", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(colorAsGrey)},
    {"ccm_calib_v2", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(ccm_calib_v2)},
    {"lut3d_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(lut3d_calib)},
    #ifdef USE_NEWSTRUCT
    {"ldch", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(ldch)},
    {"dpcc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(dpcc)},
    #else
    {"aldch", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(aldch)},
    {"adpcc_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(adpcc_calib)},
    #endif
    {"ie", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(ie)},
#if RKAIQ_HAVE_ASD_V10
    {"cpsl", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(cpsl)},
#endif
    {"cproc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(cproc)},
    #ifdef USE_NEWSTRUCT
    {"mge", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(mge)},
    #else
    {"amerge_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(amerge_calib_v12)},
    #endif
#if RKAIQ_HAVE_DRC_V12
    #ifdef USE_NEWSTRUCT
    {"drc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(drc)},
    #else
    {"adrc_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(adrc_calib_v12)},
    #endif
#endif
#if RKAIQ_HAVE_DRC_V12_LITE
    {"adrc_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(adrc_calib_v12_lite)},
#endif
    #ifdef USE_NEWSTRUCT
    {"lsc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(lsc)},
    #else
    {"lsc_v2", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(lsc_v2)},
    #endif
    #ifdef USE_NEWSTRUCT
    {"ynr", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(ynr)},
    {"sharp", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(sharp)},
    {"cnr", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(cnr)},
    {"gamma", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(gamma)},
    {"dehaze", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(dhzEhz)},
    {"gic", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(gic)},
    #else
    {"ynr_v22", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(ynr_v22)},
    {"cnr_v30", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(cnr_v30)},
    {"sharp_v33", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(sharp_v33)},
    {"agamma_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(agamma_calib_v11)},
    {"adehaze_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(adehaze_calib_v12)},
#if defined(ISP_HW_V32)
    {"agic_calib_v21", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(agic_calib_v21)},
#endif
    #endif
    {"ainr_v1", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(ainr_v1)},
#if (RKAIQ_HAVE_BAYER2DNR_V23)
    {"bayer2dnr_v23", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(bayer2dnr_v23)},
#endif
    #ifdef USE_NEWSTRUCT
    {"bayertnr", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(bayertnr)},
    #else
    {"bayertnr_v23", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(bayertnr_v23)},
    #endif
#if defined(ISP_HW_V32)
    #ifdef USE_NEWSTRUCT
    {"cac", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(cac)},
    #else
    {"cac_v11", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(cac_v11)},
    #endif
#endif
#if defined(ISP_HW_V32)
    {"af_v31", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(af_v31)},
#elif defined(ISP_HW_V32_LITE)
    {"af_v32", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(af_v32)},
#endif
    {"gain_v2", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(gain_v2)},
    {"csm", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(csm)},
    {"cgc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP32(cgc)},
    {NULL, 0},
};
#endif

#define CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(nm) \
    CALIB_MODULE_RELATIVE_OFFSET(CamCalibDbV2ContextIsp39_t, nm)

#if defined(ISP_HW_V39)
static calibdb_ctx_member_offset_info_t info_CamCalibDbV2ContextIsp39_t[] = {
    {"ae_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(ae_calib)},
    {"wb", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(wb)},
#ifdef USE_NEWSTRUCT
    {"blc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(blc)},
#else
    {"ablcV32_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(ablcV32_calib)},
#endif
    {"adegamma_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(adegamma_calib)},
#if RKAIQ_HAVE_DEBAYER_V3
    #ifdef USE_NEWSTRUCT
    {"demosaic", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(demosaic)},
    #else
    {"debayer_v3", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(debayer_v3)},
    #endif
#endif
    {"colorAsGrey", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(colorAsGrey)},
#ifdef USE_NEWSTRUCT
    {"lut3d", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(lut3d)},
    {"ccm", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(ccm)},
#else
    {"lut3d_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(lut3d_calib)},
    {"ccm_calib_v2", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(ccm_calib_v2)},
#endif
#ifdef USE_NEWSTRUCT
    {"dpc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(dpc)},
    {"ldc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(ldc)},
#else
#if (RKAIQ_HAVE_LDCH_V10 || RKAIQ_HAVE_LDCH_V21)
    {"aldch", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(aldch)},
#endif
    {"adpcc_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(adpcc_calib)},
#endif
    {"ie", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(ie)},
#ifdef USE_NEWSTRUCT
    {"cp", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(cp)},
    {"mge", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(mge)},
#else
    {"cproc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(cproc)},
    {"amerge_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(amerge_calib_v12)},
#endif
#ifdef USE_NEWSTRUCT
    {"drc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(drc)},
    {"trans", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(trans)},
    {"rgbir", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(rgbir)},
#else
    {"adrc_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(adrc_calib_v20)},
    {"argbir_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(argbir_calib_v10)},
#endif
    #ifdef USE_NEWSTRUCT
    {"lsc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(lsc)},
    #else
    {"lsc_v2", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(lsc_v2)},
    #endif
    #ifdef USE_NEWSTRUCT
    {"bayertnr", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(bayertnr)},
    {"ynr", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(ynr)},
    {"sharp", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(sharp)},
    {"cnr", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(cnr)},
    {"gamma", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(gamma)},
    {"dhzEhz", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(dhzEhz)},
    {"gic", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(gic)},
#if RKAIQ_HAVE_YUVME
    {"yme", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(yme)},
#endif
    {"histEQ", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(histEQ)},
    #else
    {"bayertnr_v30", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(bayertnr_v30)},
    {"ynr_v24", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(ynr_v24)},
    {"sharp_v34", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(sharp_v34)},
    {"cnr_v31", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(cnr_v31)},
    {"agamma_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(agamma_calib_v11)},
    {"adehaze_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(adehaze_calib_v14)},
    {"agic_calib_v21", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(agic_calib_v21)},
    {"yuvme_v1", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(yuvme_v1)},
    #endif
#if defined(ISP_HW_V39)
    #ifdef USE_NEWSTRUCT
        {"cac", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(cac)},
    #else
        {"cac_v11", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(cac_v11)},
    #endif
#endif
    {"af_v33", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(af_v33)},
    #ifdef USE_NEWSTRUCT
    {"gain", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(gain)},
    #else
    {"gain_v2", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(gain_v2)},
    #endif
    {"csm", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(csm)},
    {"cgc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP39(cgc)},
    {NULL, 0},
};
#endif

#define CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(nm) \
    CALIB_MODULE_RELATIVE_OFFSET(CamCalibDbV2ContextIsp33_t, nm)

#if defined(ISP_HW_V33)
static calibdb_ctx_member_offset_info_t info_CamCalibDbV2ContextIsp33_t[] = {
    {"ae_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(ae_calib)},
    {"wb", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(wb)},
    {"blc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(blc)},
    {"demosaic", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(demosaic)},
    {"colorAsGrey", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(colorAsGrey)},
    {"ccm", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(ccm)},
    {"dpc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(dpc)},
    {"ie", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(ie)},
    {"cp", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(cp)},
    {"mge", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(mge)},
    {"drc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(drc)},
    {"trans", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(trans)},
    {"lsc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(lsc)},
    {"bayertnr", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(bayertnr)},
    {"texEst", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(texEst)},
    {"ynr", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(ynr)},
    {"sharp", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(sharp)},
    {"cnr", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(cnr)},
    {"gamma", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(gamma)},
    {"enh", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(enh)},
    {"hsv", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(hsv)},
    {"gic", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(gic)},
    {"histEQ", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(histEQ)},
    {"cac", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(cac)},
    {"gain", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(gain)},
    {"csm", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(csm)},
    {"cgc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(cgc)},
    {"ldc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(ldc)},
    {"postisp", CALIBV2_MODULE_RELATIVE_OFFSET_ISP33(postisp)},
    {NULL, 0},
};
#endif

#define CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(nm) \
    CALIB_MODULE_RELATIVE_OFFSET(CamCalibDbV2ContextIsp35_t, nm)

#if defined(ISP_HW_V35)
static calibdb_ctx_member_offset_info_t info_CamCalibDbV2ContextIsp35_t[] = {
    {"ae_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(ae_calib)},
    {"wb", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(wb)},
    {"blc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(blc)},
    {"demosaic", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(demosaic)},
    {"colorAsGrey", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(colorAsGrey)},
    {"ccm", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(ccm)},
    {"dpc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(dpc)},
    {"ie", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(ie)},
    {"cp", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(cp)},
    {"mge", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(mge)},
    {"drc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(drc)},
    {"trans", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(trans)},
    {"lsc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(lsc)},
    {"bayertnr", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(bayertnr)},
    {"texEst", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(texEst)},
    {"ynr", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(ynr)},
    {"sharp", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(sharp)},
    {"cnr", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(cnr)},
    {"gamma", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(gamma)},
    {"enh", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(enh)},
    {"hsv", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(hsv)},
    {"gic", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(gic)},
    {"histEQ", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(histEQ)},
    {"cac", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(cac)},
    {"gain", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(gain)},
    {"csm", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(csm)},
    {"cgc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(cgc)},
    {"ldc", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(ldc)},
    {"postisp", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(postisp)},
    {"rgbir", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(rgbir)},
    {"af_calib", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(af_calib)},
    {"aibnr", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(aibnr)},
    {"airms", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(airms)},
    {"aiynr", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(aiynr)},
    {"fpnSw", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(fpnSw)},
    {"bayertnr2", CALIBV2_MODULE_RELATIVE_OFFSET_ISP35(bayertnr2)},
    {NULL, 0},
};
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
static calibdb_ctx_infos_t info_CamCalibDbV2Context_array[] = {
#ifdef ISP_HW_V20
    {20, info_CamCalibDbV2ContextIsp20_t},
#endif
#ifdef ISP_HW_V21
    {21, info_CamCalibDbV2ContextIsp21_t},
#endif
#ifdef ISP_HW_V30
    {30, info_CamCalibDbV2ContextIsp30_t},
#endif
#if defined(ISP_HW_V32)
    {32, info_CamCalibDbV2ContextIsp32_t},
#endif
#if defined(ISP_HW_V32_LITE)
    {321, info_CamCalibDbV2ContextIsp32_t},
#endif
#if defined(ISP_HW_V39)
    {39, info_CamCalibDbV2ContextIsp39_t},
#endif
#if defined(ISP_HW_V33)
    {33, info_CamCalibDbV2ContextIsp33_t},
#endif
#if defined(ISP_HW_V35)
    {35, info_CamCalibDbV2ContextIsp35_t},
#endif
};
#pragma GCC diagnostic pop

// TODO: implement ops for different isp hw
static inline size_t calibdbV2_scene_ctx_size(CamCalibDbContext_t* ctx) {
    (void)(ctx);
#ifdef ISP_HW_V20
    return sizeof(CamCalibDbV2ContextIsp20_t);
#endif
#ifdef ISP_HW_V21
    return sizeof(CamCalibDbV2ContextIsp21_t);
#endif
#ifdef ISP_HW_V30
    return sizeof(CamCalibDbV2ContextIsp30_t);
#endif
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    return sizeof(CamCalibDbV2ContextIsp32_t);
#endif
#ifdef ISP_HW_V39
    return sizeof(CamCalibDbV2ContextIsp39_t);
#endif
#ifdef ISP_HW_V33
    return sizeof(CamCalibDbV2ContextIsp33_t);
#endif
#ifdef ISP_HW_V35
    return sizeof(CamCalibDbV2ContextIsp35_t);
#endif
    return 0;
}

CamCalibDbV2Context_t* calibdbV2_ctx_new();
void calibdbV2_ctx_delete(CamCalibDbV2Context_t* ctx);

static inline const char* calibdbv2_get_scene_ctx_struct_name(const void* scene_ctx) {
    (void)(scene_ctx);
#ifdef ISP_HW_V20
    return "CamCalibDbV2ContextIsp20_t";
#endif
#ifdef ISP_HW_V21
    return "CamCalibDbV2ContextIsp21_t";
#endif
#ifdef ISP_HW_V30
    return "CamCalibDbV2ContextIsp30_t";
#endif
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    return "CamCalibDbV2ContextIsp32_t";
#endif
#ifdef ISP_HW_V39
    return "CamCalibDbV2ContextIsp39_t";
#endif
#ifdef ISP_HW_V33
    return "CamCalibDbV2ContextIsp33_t";
#endif
#ifdef ISP_HW_V35
    return "CamCalibDbV2ContextIsp35_t";
#endif
    return NULL;
}

static inline void*
calibdbv2_get_scene_ptr(CamCalibSubSceneList_t* scene) {
    (void)(scene);
#if defined(ISP_HW_V20)
    if (CHECK_ISP_HW_V20())
        return (&scene->scene_isp20);
#elif defined(ISP_HW_V21)
    if (CHECK_ISP_HW_V21())
        return (&scene->scene_isp21);
#elif defined(ISP_HW_V30)
    if (CHECK_ISP_HW_V30())
        return (&scene->scene_isp30);
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    if (CHECK_ISP_HW_V32() || CHECK_ISP_HW_V32_LITE())
        return (&scene->scene_isp32);
#elif defined(ISP_HW_V39)
    if (CHECK_ISP_HW_V39())
        return (&scene->scene_isp39);
#elif defined(ISP_HW_V33)
    if (CHECK_ISP_HW_V33())
        return (&scene->scene_isp33);
#elif defined(ISP_HW_V35)
    if (CHECK_ISP_HW_V35())
        return (&scene->scene_isp35);
#else
    return NULL;
#endif
    return NULL;
}

static inline void* calibdbV2_get_module_ptr(void* ctx, calibdb_ctx_infos_t* info_array,
                                             const char* module_name, const char* node_name) {
    if (strcmp(module_name, "sensor_calib") == 0)
        return ((CamCalibDbV2Context_t*)ctx)->sensor_info;
    else if(strcmp(module_name, "module_calib") == 0)
        return ((CamCalibDbV2Context_t*)ctx)->module_info;
    else if(strcmp(module_name, "sys_static_cfg") == 0)
        return ((CamCalibDbV2Context_t*)ctx)->sys_cfg;
    else {
        CamCalibDbV2Context_t* calib_ctx = (CamCalibDbV2Context_t*)ctx;
        if (node_name) {
            if (strcmp(node_name, "sence_cis") == 0)
                return calibdb_get_module_ptr(calib_ctx->scene_cis, info_array, module_name);
        }
        return calibdb_get_module_ptr(calib_ctx->calib_scene, info_array, module_name);
    }
}

static inline int calibdbV2_to_tuningdb(CamCalibDbV2Tuning_t *dst,
                                        const CamCalibDbV2Context_t *src) {
    memcpy(&dst->sensor_calib, src->sensor_info, sizeof(CalibDb_Sensor_ParaV2_t));
    memcpy(&dst->module_calib, src->module_info, sizeof(CalibDb_Module_ParaV2_t));
    memcpy(&dst->sys_static_cfg, src->sys_cfg,
           sizeof(CalibDb_SysStaticCfg_ParaV2_t));
    memcpy(&dst->calib_cis, src->scene_cis, sizeof(calibdb_cis_para_t));
#if defined(ISP_HW_V20)
    memcpy(&dst->calib_scene, src->calib_scene,
           sizeof(CamCalibDbV2ContextIsp20_t));
#elif defined(ISP_HW_V21)
    memcpy(&dst->calib_scene, src->calib_scene,
           sizeof(CamCalibDbV2ContextIsp21_t));
#elif defined(ISP_HW_V30)
    memcpy(&dst->calib_scene, src->calib_scene,
           sizeof(CamCalibDbV2ContextIsp30_t));
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    memcpy(&dst->calib_scene, src->calib_scene,
           sizeof(CamCalibDbV2ContextIsp32_t));
#elif defined(ISP_HW_V39)
    memcpy(&dst->calib_scene, src->calib_scene,
           sizeof(CamCalibDbV2ContextIsp39_t));
#elif defined(ISP_HW_V33)
    memcpy(&dst->calib_scene, src->calib_scene,
           sizeof(CamCalibDbV2ContextIsp33_t));
#elif defined(ISP_HW_V35)
    memcpy(&dst->calib_scene, src->calib_scene,
           sizeof(CamCalibDbV2ContextIsp35_t));
#else
#error "WRONG ISP_HW_VERSION, ONLY SUPPORT V20 AND V21 AND V30 AND V32 AND V32_LITE V39 AND V33 V35 NOW !"
#endif
    return 0;
}

static inline int calibdbV2_from_tuningdb(CamCalibDbV2Context_t *dst,
        const CamCalibDbV2Tuning_t *src) {
    memcpy(dst->sensor_info, &src->sensor_calib, sizeof(CalibDb_Sensor_ParaV2_t));
    memcpy(dst->module_info, &src->module_calib, sizeof(CalibDb_Module_ParaV2_t));
    memcpy(dst->sys_cfg, &src->sys_static_cfg,
           sizeof(CalibDb_SysStaticCfg_ParaV2_t));
    memcpy(&dst->scene_cis, &src->calib_cis, sizeof(calibdb_cis_para_t));
#if defined(ISP_HW_V20)
    memcpy(dst->calib_scene, &src->calib_scene,
           sizeof(CamCalibDbV2ContextIsp20_t));
#elif defined(ISP_HW_V21)
    memcpy(dst->calib_scene, &src->calib_scene,
           sizeof(CamCalibDbV2ContextIsp21_t));
#elif defined(ISP_HW_V30)
    memcpy(dst->calib_scene, &src->calib_scene,
           sizeof(CamCalibDbV2ContextIsp30_t));
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    memcpy(dst->calib_scene, &src->calib_scene,
           sizeof(CamCalibDbV2ContextIsp32_t));
#elif defined(ISP_HW_V39)
    memcpy(dst->calib_scene, &src->calib_scene,
           sizeof(CamCalibDbV2ContextIsp39_t));
#elif defined(ISP_HW_V33)
    memcpy(dst->calib_scene, &src->calib_scene,
           sizeof(CamCalibDbV2ContextIsp33_t));
#elif defined(ISP_HW_V35)
    memcpy(dst->calib_scene, &src->calib_scene,
           sizeof(CamCalibDbV2ContextIsp35_t));
#else
#error "WRONG ISP_HW_VERSION, ONLY SUPPORT V20 AND V21 AND V30 AND V32 AND V32_LITE V39 AND V33 V35 NOW !"
#endif
    return 0;
}

/*
 *    S C E N E    C I S
 */

#define CALIB_MODULE_RELATIVE_OFFSET_CIS(nm) CALIB_MODULE_RELATIVE_OFFSET(calibdb_cis_para_t, nm)

static calibdb_ctx_member_offset_info_t info_CamCalibDbSenceCis_t[] = {
    {"CisRegSetting", CALIB_MODULE_RELATIVE_OFFSET_CIS(cis_regSetting)},
    {"CisBlc", CALIB_MODULE_RELATIVE_OFFSET_CIS(cis_blc)},
    {"CisHdr", CALIB_MODULE_RELATIVE_OFFSET_CIS(cis_hdr)},
    {"CisQbcRmsc", CALIB_MODULE_RELATIVE_OFFSET_CIS(cis_qbcRmsc)},
    {"CisCmpsOut", CALIB_MODULE_RELATIVE_OFFSET_CIS(cis_cmpsOut)},
    {NULL, 0},
};

static inline void* calibdbv2_get_scene_cis_ptr(CamCalibSubSceneList_t* scene) {
    return (&scene->scene_cis);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
static calibdb_ctx_infos_t info_CamCalibDbV2SenceCis_array[] = {
    {VERSION_SKIP_CHECK, info_CamCalibDbSenceCis_t},
};
#pragma GCC diagnostic pop

static inline bool calibdbv2_is_ptr_valid(const void* ptr);

/**
 * @brief Get a module pointer from the CalibDB V2 context
 * @param ctx Calibration database context pointer
 * @param module_name Module name as string
 * @return Pointer to the requested module, or NULL if not found
 */
static inline void* calibdbv2_get_module_ptr_wrapper(void* ctx, const char* module_name) {
    void* ptr = calibdbV2_get_module_ptr(ctx, info_CamCalibDbV2Context_array, module_name, NULL);
    if (ptr) {
        return ptr;
    } else {
        // If module is "sence_cis", try to get it from the scene CIS array
        ptr = calibdbV2_get_module_ptr(ctx, info_CamCalibDbV2SenceCis_array, module_name,
                                       "sence_cis");
        return calibdbv2_is_ptr_valid(ptr) ? ptr : NULL;
    }
}

#define CALIBDBV2_GET_MODULE_PTR(ctx, module) calibdbv2_get_module_ptr_wrapper(ctx, #module)

/* Magic value for checking invalid struct pointer */
#define J2S_INVALID_STRUCT_MAGIC 0xDEADBEEF

/**
 * @brief Check if a pointer is valid (does not contain the invalid magic value)
 * @param ptr Pointer to check
 * @return true if pointer is valid, false if contains magic value (invalid) or NULL
 */
static inline bool calibdbv2_is_ptr_valid(const void* ptr) {
    if (!ptr) return false;

    /* Try to check magic value at different sizes to handle small structs */
    const uint8_t* byte_ptr = (const uint8_t*)ptr;

    /* First try 4-byte check for most common case */
    const uint32_t* magic_ptr32 = (const uint32_t*)ptr;
    if (*magic_ptr32 == J2S_INVALID_STRUCT_MAGIC) {
        return false;
    }

    /* Then try 2-byte check for smaller structs */
    const uint16_t* magic_ptr16 = (const uint16_t*)ptr;
    uint16_t expected16         = (uint16_t)(J2S_INVALID_STRUCT_MAGIC & 0xFFFF);
    if (*magic_ptr16 == expected16) {
        return false;
    }

    /* Finally try 1-byte check for smallest structs */
    uint8_t expected8 = (uint8_t)(J2S_INVALID_STRUCT_MAGIC & 0xFF);
    if (*byte_ptr == expected8) {
        return false;
    }

    return true;
}

/* Macro for checking if a module pointer is valid */
#define CALIBDBV2_IS_MODULE_VALID(ptr) calibdbv2_is_ptr_valid(ptr)

#endif
