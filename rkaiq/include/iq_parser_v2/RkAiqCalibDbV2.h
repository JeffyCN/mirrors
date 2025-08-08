/*
 *  Copyright (c) 2021 Rockchip Corporation
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

#ifndef ___RK_AIQ_CALIB_DB_V2_H__
#define ___RK_AIQ_CALIB_DB_V2_H__

#include "iq_parser_v2/j2s/cJSON.h"
#include "iq_parser_v2/RkAiqCalibDbTypesV2.h"
#include "iq_parser_v2/RkAiqCalibDbV2Helper.h"
#include "iq_parser_v2/ablc_head.h"
#include "iq_parser_v2/ablc_head_V32.h"
#include "algos/rk_aiq_algo_des.h"
#include "iq_parser_v2/sharp_head_v33.h"
#include "iq_parser_v2/rkpostisp_head_v1.h"
#include "xcore/base/xcam_log.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CAMCALIBDB_JSFILE_LEN            128
#define CAMCALIBDB_MAP_LEN                32
#define CAMCALIBDB_MODULE_NAME_LEN       128
#define CAMCALIBDB_MODULE_SIZE_LEN       128

struct cJSON;

typedef struct CamCalibDbMap_s {
    char jsFileName[CAMCALIBDB_JSFILE_LEN];
    CamCalibDbProj_t* calibproj;
} CamCalibDbMap;

typedef struct {
    CamCalibDbV2Context_t* calib;
    char moduleNames[CAMCALIBDB_MODULE_SIZE_LEN][CAMCALIBDB_MODULE_NAME_LEN];
    int moduleNamesSize;
} TuningCalib;

CamCalibDbProj_t *CamCalibDbCreateCalibDbProjFromFile(int sns_id, const char *jsfile);
CamCalibDbProj_t *CamCalibDbCreateCalibDbProjFromBuf(int sns_id, const void *bin_buff, size_t len);
CamCalibDbCamgroup_t* CamCalibDbCreateCalibDbCamgroup(const char *jsfile);
int CamCalibDbCamgroupFree(CamCalibDbCamgroup_t* calib_camgroup);

CamCalibDbProj_t *CamCalibDbJson2calibprojFromFile(const char *jsfile);
CamCalibDbProj_t *CamCalibDbJson2calibprojFromStr(const char *jstr, size_t len);
CamCalibDbProj_t *CamCalibDbBin2calibprojFromFile(const char *binfile);
CamCalibDbProj_t *CamCalibDbBin2calibprojFromBuf(const void *bin_buff, size_t len);
CamCalibDbV2Context_t *CamCalibDbJson2calib(const char *jsfile);
CamCalibDbV2Context_t *CamCalibDbCjson2calib(cJSON *json);
void CamCalibDbBin2calib(void *bin_buff, void *struct_ptr);

int CamCalibDbCalib2json(const char *jsfile, CamCalibDbV2Context_t *calib);
cJSON *CamCalibDbCalib2cjson(const CamCalibDbV2Context_t *calib);
int CamCalibDbCalibproj2json(const char *jsfile, CamCalibDbProj_t *calibproj);
int CamCalibDbCalib2bin(void *bin_buf, CamCalibDbV2Context_t *CalibDbV2);

void CamCalibDbReleaseCalibDbProj();
CamCalibDbV2Context_t CamCalibDbToDefaultCalibDb(CamCalibDbProj_t *calibproj);

CamCalibDbProj_t *CamCalibDbProjAlloc();
CamCalibDbV2Context_t *CamCalibDbCalibV2Alloc();
int CamCalibDbProjFree(CamCalibDbProj_t *calibproj);
int CamCalibDbCalibV2Free(CamCalibDbV2Context_t *calibv2);

RkAiqAlgoType_t CamCalibDbString2algostype(const char* str);
int CamCalibDbAnalyzChange(cJSON *patch, TuningCalib *tuning_calib);

cJSON* CamCalibDbReadIQNode(const CamCalibDbV2Context_t *calib,
                             const char* node_path);

char* CamCalibDbReadIQNodeStr(const CamCalibDbV2Context_t* calib,
                               const char* node_path);

char* CamCalibDbReadIQNodeStrFromJstr(const CamCalibDbV2Context_t* calib,
                                       const char* json_str);

CamCalibDbV2Context_t *CamCalibDbApplyPatchFromcJSON(const CamCalibDbV2Context_t *calib,
            cJSON *patch);
CamCalibDbV2Context_t *CamCalibDbApplyPatch2(const CamCalibDbV2Context_t *calib,
            cJSON *patch);

CamCalibDbV2Context_t *CamCalibDbApplyPatch(const CamCalibDbV2Context_t *calib,
            const char *patch_str);
CamCalibDbV2Context_t *
    CamCalibDbapplyPatchFile(const CamCalibDbV2Context_t *calib, const char *patch_file);
CamCalibDbV2Context_t *CamCalibDbapplyPatch(const char *jsfile,
            const char *patch_file);

int CamCalibDbAnalyzTuningCalibFromCJson(const CamCalibDbV2Context_t *calib,
                                         cJSON* patch, TuningCalib *tuning_calib);
int CamCalibDbAnalyzTuningCalibFromStr(const CamCalibDbV2Context_t *calib,
                                         const char* patch_str, TuningCalib *tuning_calib);

int CamCalibDbFreeCalibByJ2S(void* ctx);

void *CamCalibDbLoadWholeFile(const char *fpath, size_t *fsize);
int CamCalibDbParseBinStructMap(uint8_t *data, size_t len);
int CamCalibDbRestoreBinStructMap(uint8_t *data, size_t len, uint8_t *restore_ptr);
int CamCalibDbCheckBinVersion(uint8_t *data, size_t len);

int CamCalibDbFreeSceneCtx(void* ctx);
int CamCalibDbFreeSensorCtx(CalibDb_Sensor_ParaV2_t* sensor);
int CamCalibDbFreeModuleCtx(CalibDb_Module_ParaV2_t* module);
int CamCalibDbFreeUapiCtx(RkaiqUapi_t* uapi);
int CamCalibDbFreeSysStaticCtx(CalibDb_SysStaticCfg_ParaV2_t* sys_static);

/*
private:
    static std::map<std::string, CamCalibDbProj_t *> mCalibDbsMap;
    static CalibDb_Sensor_ParaV2_t mSensorInfo;
    static std::mutex calib_mutex;
    // isp20
    static int CamCalibDbFreeSceneCtx(void* ctx);
    static int CamCalibDbFreeSensorCtx(CalibDb_Sensor_ParaV2_t* sensor);
    static int CamCalibDbFreeModuleCtx(CalibDb_Module_ParaV2_t* module);
    static int CamCalibDbFreeAeCtx(CalibDb_Aec_ParaV2_t* ae);
#if RKAIQ_HAVE_AWB_V20
    static int CamCalibDbFreeAwbV20Ctx(CalibDbV2_Wb_Para_V20_t* awb);
#endif
#if RKAIQ_HAVE_GAMMA_V10
    static int CamCalibDbFreeGammaCtx(CalibDbV2_gamma_v10_t* gamma);
#endif
    static int CamCalibDbFreeBlcCtx(CalibDbV2_Ablc_t* blc);
    static int CamCalibDbFreeDegammaCtx(CalibDbV2_Adegmma_t* degamma);
#if RKAIQ_HAVE_GIC_V1
    static int CamCalibDbFreeGicV20Ctx(CalibDbV2_Gic_V20_t* gic);
#endif
    static int CamCalibDbFreeDehazeV20Ctx(CalibDbV2_dehaze_v10_t* dehaze);
    static int CamCalibDbFreeDpccCtx(CalibDbV2_Dpcc_t* dpcc);
    static int CamCalibDbFreeTmoCtx(CalibDbV2_tmo_t* tmo);
#if RKAIQ_HAVE_ORB_V1
    static int CamCalibDbFreeOrbCtx(CalibDbV2_Orb_t* orb);
#endif
#if RKAIQ_HAVE_BAYERNR_V1
    static int CamCalibDbFreeBayerNrV1Ctx(CalibDbV2_BayerNrV1_t* bayernr_v1);
#endif
#if RKAIQ_HAVE_MFNR_V1
    static int CamCalibDbFreeMfnrCtx(CalibDbV2_MFNR_t* mfnr_v1);
#endif
#if RKAIQ_HAVE_UVNR_V1
    static int CamCalibDbFreeUvnrCtx(CalibDbV2_UVNR_t* uvnr_v1);
#endif
#if RKAIQ_HAVE_YNR_V1
    static int CamCalibDbFreeYnrV1Ctx(CalibDbV2_YnrV1_t* ynr_v1);
#endif
#if RKAIQ_HAVE_SHARP_V1
    static int CamCalibDbFreeSharpV1Ctx(CalibDbV2_SharpV1_t* sharp_v1);
    static int CamCalibDbFreeEdgefilterCtx(CalibDbV2_Edgefilter_t* edgefilter_v1);
#endif

#if RKAIQ_HAVE_DEBAYER_V1
    static int CamCalibDbFreeDebayerCtx(CalibDbV2_Debayer_t* debayer_v1);
#endif
#if RKAIQ_HAVE_ACP_V10
    static int CamCalibDbFreeCprocCtx(CalibDbV2_Cproc_t* cproc);
#endif
#if RKAIQ_HAVE_AIE_V10
    static int CamCalibDbFreeIeCtx(CalibDbV2_IE_t* ie);
#endif
    static int CamCalibDbFreeLscCtx(CalibDbV2_LSC_t* lsc);
#if RKAIQ_HAVE_EIS_V1
    static int CamCalibDbFreeEisCtx(CalibDbV2_Eis_t* eis);
#endif
    static int CamCalibDbFreeColorAsGreyCtx(CalibDbV2_ColorAsGrey_t* colorAsGrey);
    static int CamCalibDbFreeLumaDetectCtx(CalibDbV2_LUMA_DETECT_t* lumaDetect);
#if RKAIQ_HAVE_FEC_V10
    static int CamCalibDbFreeFectCtx(CalibDbV2_FEC_t* fec);
#endif
#if (RKAIQ_HAVE_LDCH_V10 || RKAIQ_HAVE_LDCH_V21)
    static int CamCalibDbFreeLdchCtx(CalibDbV2_LDCH_t* ldch);
#endif
#if RKAIQ_HAVE_CCM_V1
    static int CamCalibDbFreeCcmV1Ctx(CalibDbV2_Ccm_Para_V2_t* ccm);
#endif
#if RKAIQ_HAVE_3DLUT_V1
    static int CamCalibDbFreeLut3dCtx(CalibDbV2_Lut3D_Para_V2_t* lut3d);
#endif
#if RKAIQ_HAVE_AF_V20
    static int CamCalibDbFreeAfV2xCtx(CalibDbV2_AF_t* af);
#endif
    static int CamCalibDbFreeThumbnailsCtx(CalibDbV2_Thumbnails_t* thumbnails);

    //isp21
#if RKAIQ_HAVE_AWB_V21
    static int CamCalibDbFreeAwbV21Ctx(CalibDbV2_Wb_Para_V21_t* awb);
#endif
#if RKAIQ_HAVE_GIC_V2
    static int CamCalibDbFreeGicV21Ctx(CalibDbV2_Gic_V21_t* gic);
#endif
#if RKAIQ_HAVE_BAYERNR_V2
    static int CamCalibDbFreeBayerNrV2Ctx(CalibDbV2_BayerNrV2_t* bayernr_v1);
#endif
#if RKAIQ_HAVE_UVNR_V1
    static int CamCalibDbFreeCnrCtx(CalibDbV2_CNR_t* cnr);
#endif
#if RKAIQ_HAVE_YNR_V2
    static int CamCalibDbFreeYnrV2Ctx(CalibDbV2_YnrV2_t* ynr_v2);
#endif
#if RKAIQ_HAVE_SHARP_V3
    static int CamCalibDbFreeSharpV3Ctx(CalibDbV2_SharpV3_t* sharp_v3);
#endif

    static int CamCalibDbFreeUapiCtx(RkaiqUapi_t* uapi);
    static int CamCalibDbFreeSysStaticCtx(CalibDb_SysStaticCfg_ParaV2_t* sys_static);

    //isp30
#if RKAIQ_HAVE_GAMMA_V11
    static int CamCalibDbFreeGammaV2Ctx(CalibDbV2_gamma_v11_t* gamma);
#endif
#if RKAIQ_HAVE_CAC_V03
    static int CamCalibDbFreeCacV03Ctx(CalibDbV2_Cac_V03_t* cac_calib);
#endif
#if RKAIQ_HAVE_CAC_V10
    static int CamCalibDbFreeCacV10Ctx(CalibDbV2_Cac_V10_t* cac_calib);
#endif
#if RKAIQ_HAVE_CAC_V11
    static int CamCalibDbFreeCacV11Ctx(CalibDbV2_Cac_V11_t* cac_calib);
#endif
#if RKAIQ_HAVE_BAYER2DNR_V2
    static int CamCalibDbFreeBayer2dnrV2Ctx(CalibDbV2_Bayer2dnrV2_t* bayer2dnr_v2);
#endif
#if RKAIQ_HAVE_BAYERTNR_V2
    static int CamCalibDbFreeBayertnrV2Ctx(CalibDbV2_BayerTnrV2_t* bayertnr_v2);
#endif
#if RKAIQ_HAVE_CNR_V2
    static int CamCalibDbFreeCnrV2Ctx(CalibDbV2_CNRV2_t* cnr_v2);
#endif
#if RKAIQ_HAVE_YNR_V3
    static int CamCalibDbFreeYnrV3Ctx(CalibDbV2_YnrV3_t* ynr_v3);
#endif
#if RKAIQ_HAVE_SHARP_V4
    static int CamCalibDbFreeSharpV4Ctx(CalibDbV2_SharpV4_t* sharp_v4);
#endif
#if RKAIQ_HAVE_AF_V30
    static int CamCalibDbFreeAfV30Ctx(CalibDbV2_AFV30_t* af);
#endif

// isp 32
#if RKAIQ_HAVE_AWB_V32||RKAIQ_HAVE_AWB_V39
    static int CamCalibDbFreeAwbV32Ctx(CalibDbV2_Wb_Para_V32_t* awb);
#endif
#if RKAIQ_HAVE_BLC_V32
    static int CamCalibDbFreeBlcV32Ctx(CalibDbV2_Blc_V32_t* blc_v32);
#endif
#if RKAIQ_HAVE_DEBAYER_V2
    static int CamCalibDbFreeDebayerV2Ctx(CalibDbV2_Debayer_v2_t * debayer_v2);
#endif
#if RKAIQ_HAVE_DEBAYER_V2_LITE
    static int CamCalibDbFreeDebayerV2Ctx(CalibDbV2_Debayer_v2_lite_t * debayer_v2);
#endif
#if RKAIQ_HAVE_DEBAYER_V3
    static int CamCalibDbFreeDebayerV3Ctx(CalibDbV2_Debayer_v3_t * debayer_v3);
#endif

#if RKAIQ_HAVE_CCM_V2 || RKAIQ_HAVE_CCM_V3
    template<class T>
    static int CamCalibDbFreeCcmV2Ctx(T* ccm);
#endif
#if RKAIQ_HAVE_BAYER2DNR_V23
    static int CamCalibDbFreeBayer2dnrV23Ctx(CalibDbV2_Bayer2dnrV23_t* bayer2dnr_v23);
#endif
#if RKAIQ_HAVE_BAYERTNR_V23
    static int CamCalibDbFreeBayertnrV23Ctx(CalibDbV2_BayerTnrV23_t* bayertnr_v23);
#endif
#if RKAIQ_HAVE_BAYERTNR_V23_LITE
    static int CamCalibDbFreeBayertnrV23LiteCtx(CalibDbV2_BayerTnrV23Lite_t* bayertnr_v23_lite);
#endif
#if (RKAIQ_HAVE_CNR_V30 || RKAIQ_HAVE_CNR_V30_LITE)
    static int CamCalibDbFreeCnrV30Ctx(CalibDbV2_CNRV30_t* cnr_v30);
#endif
#if RKAIQ_HAVE_YNR_V22
    static int CamCalibDbFreeYnrV22Ctx(CalibDbV2_YnrV22_t* ynr_v22);
#endif
#if RKAIQ_HAVE_YNR_V24
    static int CamCalibDbFreeYnrV24Ctx(CalibDbV2_YnrV24_t* ynr_v24);
#endif
#if RKAIQ_HAVE_SHARP_V33
    static int CamCalibDbFreeSharpV33Ctx(CalibDbV2_SharpV33_t* sharp_v33);
#endif
#if RKAIQ_HAVE_SHARP_V33_LITE
    static int CamCalibDbFreeSharpV33LiteCtx(CalibDbV2_SharpV33Lite_t* sharp_v33);
#endif
#if RKAIQ_HAVE_SHARP_V34
    static int CamCalibDbFreeSharpV34Ctx(CalibDbV2_SharpV34_t* sharp_v34);
#endif

#if RKAIQ_HAVE_AF_V31
    static int CamCalibDbFreeAfV31Ctx(CalibDbV2_AFV31_t* af);
#endif
#if RKAIQ_HAVE_GAIN_V2
    static int CamCalibDbFreeGainV2Ctx(CalibDbV2_GainV2_t* gain_v2);
#endif

#if RKAIQ_HAVE_AF_V32_LITE
    static int CamCalibDbFreeAfV32Ctx(CalibDbV2_AFV32_t* af);
#endif
#if RKAIQ_HAVE_BAYERTNR_V30
    static int CamCalibDbFreeBayertnrV30Ctx(CalibDbV2_BayerTnrV30_t* bayertnr_v30);
#endif

#if RKAIQ_HAVE_YUVME_V1
    static int CamCalibDbFreeYuvmeV1Ctx(CalibDbV2_YuvmeV1_t* yuvme_v1);
#endif
#if (RKAIQ_HAVE_CNR_V31)
    static int CamCalibDbFreeCnrV31Ctx(CalibDbV2_CNRV31_t* cnr_v31);
#endif

#if RKAIQ_HAVE_AF_V33
    static int CamCalibDbFreeAfV33Ctx(CalibDbV2_AFV33_t* af);
#endif
*/

#ifdef __cplusplus
}
#endif

#endif /*___RK_AIQ_CALIB_DB_V2_H__*/
