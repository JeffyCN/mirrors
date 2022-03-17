/*
 * rk_aiq_algo_types_int.h
 *
 *  Copyright (c) 2019 Rockchip Corporation
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

#ifndef _RK_AIQ_ALGO_TYPES_INT_H_
#define _RK_AIQ_ALGO_TYPES_INT_H_


#include "rk_aiq_algo_des.h"
#include "rk_aiq_types_priv.h"
#include "rk_aiq_algo_types.h"
#include "adehaze/rk_aiq_types_adehaze_algo_int.h"
#include "anr/rk_aiq_types_anr_algo_int.h"
#include "asharp/rk_aiq_types_asharp_algo_int.h"
#include "amerge/rk_aiq_types_amerge_algo_int.h"
#include "atmo/rk_aiq_types_atmo_algo_int.h"
#include "adrc/rk_aiq_types_adrc_algo_int.h"
#include "ae/rk_aiq_types_ae_algo_int.h"
#include "af/rk_aiq_types_af_algo_int.h"
#include "aorb/rk_aiq_types_orb_algo.h"
#include "awb/rk_aiq_types_awb_algo_int.h"
#include "agamma/rk_aiq_types_agamma_algo_int.h"
#include "adegamma/rk_aiq_types_adegamma_algo_int.h"
#include "adpcc/rk_aiq_types_adpcc_algo_int.h"
#include "adebayer/rk_aiq_types_algo_adebayer_int.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbV2.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "ablc/rk_aiq_types_ablc_algo_int.h"
#include "alsc/rk_aiq_types_alsc_algo_int.h"
#include "accm/rk_aiq_types_accm_algo_int.h"
#include "a3dlut/rk_aiq_types_a3dlut_algo_int.h"
#include "acp/rk_aiq_types_acp_algo_int.h"
#include "aie/rk_aiq_types_aie_algo_int.h"
#include "aldch/rk_aiq_types_aldch_algo_int.h"
#include "afec/rk_aiq_types_afec_algo_int.h"
#include "asharp3/rk_aiq_types_asharp_algo_int_v3.h"
#include "aynr2/rk_aiq_types_aynr_algo_int_v2.h"
#include "acnr/rk_aiq_types_acnr_algo_int_v1.h"
#include "arawnr2/rk_aiq_types_abayernr_algo_int_v2.h"
#include "amd/rk_aiq_types_algo_amd_int.h"
#include "arawnr/rk_aiq_types_abayernr_algo_int_v1.h"
#include "aynr/rk_aiq_types_aynr_algo_int_v1.h"
#include "auvnr/rk_aiq_types_auvnr_algo_int_v1.h"
#include "amfnr/rk_aiq_types_amfnr_algo_int_v1.h"
#include "again/rk_aiq_types_again_algo_int.h"


// forward declare
typedef struct _RkAiqAlgoPreResAe     RkAiqAlgoPreResAe;
typedef struct _RkAiqAlgoPreResAwb    RkAiqAlgoPreResAwb;
typedef struct _RkAiqAlgoPreResAf     RkAiqAlgoPreResAf;
typedef struct _RkAiqAlgoPreResAsd    RkAiqAlgoPreResAsd;
typedef struct _RkAiqAlgoPreResAnr    RkAiqAlgoPreResAnr;
typedef struct _RkAiqAlgoPreResAmerge RkAiqAlgoPreResAmerge;
typedef struct _RkAiqAlgoPreResAtmo   RkAiqAlgoPreResAtmo;
typedef struct _RkAiqAlgoPreResAcp    RkAiqAlgoPreResAcp;
typedef struct _RkAiqAlgoPreResAsharp RkAiqAlgoPreResAsharp;
typedef struct _RkAiqAlgoPreResAdhaz  RkAiqAlgoPreResAdhaz;
typedef struct _RkAiqAlgoPreResA3dlut RkAiqAlgoPreResA3dlut;
typedef struct _RkAiqAlgoPreResAblc   RkAiqAlgoPreResAblc;
typedef struct _RkAiqAlgoPreResAccm   RkAiqAlgoPreResAccm;
typedef struct _RkAiqAlgoPreResAcgc   RkAiqAlgoPreResAcgc;
typedef struct _RkAiqAlgoPreResAdebayer RkAiqAlgoPreResAdebayer;
typedef struct _RkAiqAlgoPreResAdpcc  RkAiqAlgoPreResAdpcc;
typedef struct _RkAiqAlgoPreResAfec   RkAiqAlgoPreResAfec;
typedef struct _RkAiqAlgoPreResAgamma RkAiqAlgoPreResAgamma;
typedef struct _RkAiqAlgoPreResAdegamma RkAiqAlgoPreResAdegamma;
typedef struct _RkAiqAlgoPreResAgic   RkAiqAlgoPreResAgic;
typedef struct _RkAiqAlgoPreResAie    RkAiqAlgoPreResAie;
typedef struct _RkAiqAlgoPreResAldch  RkAiqAlgoPreResAldch;
typedef struct _RkAiqAlgoPreResAlsc   RkAiqAlgoPreResAlsc;
typedef struct _RkAiqAlgoPreResAorb   RkAiqAlgoPreResAorb;
typedef struct _RkAiqAlgoPreResAr2y   RkAiqAlgoPreResAr2y;
typedef struct _RkAiqAlgoPreResAwdr   RkAiqAlgoPreResAwdr;
typedef struct _RkAiqAlgoPreResArawnr   RkAiqAlgoPreResArawnr;
typedef struct _RkAiqAlgoPreResAmfnr   RkAiqAlgoPreResAmfnr;
typedef struct _RkAiqAlgoPreResAynr   RkAiqAlgoPreResAynr;
typedef struct _RkAiqAlgoPreResAcnr   RkAiqAlgoPreResAcnr;
typedef struct _RkAiqAlgoPreResAdrc   RkAiqAlgoPreResAdrc;
typedef struct _RkAiqAlgoPreResAeis   RkAiqAlgoPreResAeis;
typedef struct _RkAiqAlgoPreResAmd   RkAiqAlgoPreResAmd;
typedef struct _RkAiqAlgoPreResAgain   RkAiqAlgoPreResAgain;


typedef struct _RkAiqAlgoProcResAe    RkAiqAlgoProcResAe;
typedef struct _RkAiqAlgoProcResAwb   RkAiqAlgoProcResAwb;
typedef struct _RkAiqAlgoProcResAf    RkAiqAlgoProcResAf;
typedef struct _RkAiqAlgoProcResAsd   RkAiqAlgoProcResAsd;
typedef struct _RkAiqAlgoProcResAnr   RkAiqAlgoProcResAnr;
typedef struct _RkAiqAlgoProcResAmerge RkAiqAlgoProcResAmerge;
typedef struct _RkAiqAlgoProcResAtmo  RkAiqAlgoProcResAtmo;
typedef struct _RkAiqAlgoProcResAcp   RkAiqAlgoProcResAcp;
typedef struct _RkAiqAlgoProcResAsharp RkAiqAlgoProcResAsharp;
typedef struct _RkAiqAlgoProcResAdhaz RkAiqAlgoProcResAdhaz;
typedef struct _RkAiqAlgoProcResA3dlut RkAiqAlgoProcResA3dlut;
typedef struct _RkAiqAlgoProcResAblc  RkAiqAlgoProcResAblc;
typedef struct _RkAiqAlgoProcResAccm  RkAiqAlgoProcResAccm;
typedef struct _RkAiqAlgoProcResAcgc  RkAiqAlgoProcResAcgc;
typedef struct _RkAiqAlgoProcResAdebayer RkAiqAlgoProcResAdebayer;
typedef struct _RkAiqAlgoProcResAdpcc RkAiqAlgoProcResAdpcc;
typedef struct _RkAiqAlgoProcResAfec  RkAiqAlgoProcResAfec;
typedef struct _RkAiqAlgoProcResAgamma RkAiqAlgoProcResAgamma;
typedef struct _RkAiqAlgoProcResAdegamma RkAiqAlgoProcResAdegamma;
typedef struct _RkAiqAlgoProcResAgic  RkAiqAlgoProcResAgic;
typedef struct _RkAiqAlgoProcResAie   RkAiqAlgoProcResAie;
typedef struct _RkAiqAlgoProcResAldch RkAiqAlgoProcResAldch;
typedef struct _RkAiqAlgoProcResAlsc  RkAiqAlgoProcResAlsc;
typedef struct _RkAiqAlgoProcResAorb  RkAiqAlgoProcResAorb;
typedef struct _RkAiqAlgoProcResAr2y  RkAiqAlgoProcResAr2y;
typedef struct _RkAiqAlgoProcResAwdr  RkAiqAlgoProcResAwdr;
typedef struct _RkAiqAlgoProcResArawnr  RkAiqAlgoProcResArawnr;
typedef struct _RkAiqAlgoProcResAmfnr  RkAiqAlgoProcResAmfnr;
typedef struct _RkAiqAlgoProcResAynr  RkAiqAlgoProcResAynr;
typedef struct _RkAiqAlgoProcResAcnr  RkAiqAlgoProcResAcnr;
typedef struct _RkAiqAlgoProcResAdrc  RkAiqAlgoProcResAdrc;
typedef struct _RkAiqAlgoProcResAeis  RkAiqAlgoProcResAeis;
typedef struct _RkAiqAlgoProcResAmd  RkAiqAlgoProcResAmd;
typedef struct _RkAiqAlgoProcResAgain  RkAiqAlgoProcResAgain;


typedef struct _RkAiqAlgoPostResAe    RkAiqAlgoPostResAe;
typedef struct _RkAiqAlgoPostResAwb   RkAiqAlgoPostResAwb;
typedef struct _RkAiqAlgoPostResAf    RkAiqAlgoPostResAf;
typedef struct _RkAiqAlgoPostResAsd   RkAiqAlgoPostResAsd;
typedef struct _RkAiqAlgoPostResAnr   RkAiqAlgoPostResAnr;
typedef struct _RkAiqAlgoPostResAmerge  RkAiqAlgoPostResAmerge;
typedef struct _RkAiqAlgoPostResAtmo  RkAiqAlgoPostResAtmo;
typedef struct _RkAiqAlgoPostResAcp   RkAiqAlgoPostResAcp;
typedef struct _RkAiqAlgoPostResAsharp RkAiqAlgoPostResAsharp;
typedef struct _RkAiqAlgoPostResAdhaz RkAiqAlgoPostResAdhaz;
typedef struct _RkAiqAlgoPostResA3dlut RkAiqAlgoPostResA3dlut;
typedef struct _RkAiqAlgoPostResAblc  RkAiqAlgoPostResAblc;
typedef struct _RkAiqAlgoPostResAccm  RkAiqAlgoPostResAccm;
typedef struct _RkAiqAlgoPostResAcgc  RkAiqAlgoPostResAcgc;
typedef struct _RkAiqAlgoPostResAdebayer RkAiqAlgoPostResAdebayer;
typedef struct _RkAiqAlgoPostResAdpcc RkAiqAlgoPostResAdpcc;
typedef struct _RkAiqAlgoPostResAfec  RkAiqAlgoPostResAfec;
typedef struct _RkAiqAlgoPostResAgamma RkAiqAlgoPostResAgamma;
typedef struct _RkAiqAlgoPostResAdegamma RkAiqAlgoPostResAdegamma;
typedef struct _RkAiqAlgoPostResAgic  RkAiqAlgoPostResAgic;
typedef struct _RkAiqAlgoPostResAie   RkAiqAlgoPostResAie;
typedef struct _RkAiqAlgoPostResAldch RkAiqAlgoPostResAldch;
typedef struct _RkAiqAlgoPostResAlsc  RkAiqAlgoPostResAlsc;
typedef struct _RkAiqAlgoPostResAorb  RkAiqAlgoPostResAorb;
typedef struct _RkAiqAlgoPostResAr2y  RkAiqAlgoPostResAr2y;
typedef struct _RkAiqAlgoPostResAwdr  RkAiqAlgoPostResAwdr;
typedef struct _RkAiqAlgoPostResArawnr  RkAiqAlgoPostResArawnr;
typedef struct _RkAiqAlgoPostResAmfnr  RkAiqAlgoPostResAmfnr;
typedef struct _RkAiqAlgoPostResAynr  RkAiqAlgoPostResAynr;
typedef struct _RkAiqAlgoPostResAcnr  RkAiqAlgoPostResAcnr;
typedef struct _RkAiqAlgoPostResAdrc  RkAiqAlgoPostResAdrc;
typedef struct _RkAiqAlgoPostResAeis  RkAiqAlgoPostResAeis;
typedef struct _RkAiqAlgoPostResAmd  RkAiqAlgoPostResAmd;
typedef struct _RkAiqAlgoPostResAgain  RkAiqAlgoPostResAgain;


typedef struct _RkAiqPreResComb {
    RkAiqAlgoPreResAe*    ae_pre_res;
    RkAiqAlgoPreResAwb*   awb_pre_res;
    RkAiqAlgoPreResAf*    af_pre_res;
    RkAiqAlgoPreResAsd*   asd_pre_res;
    RkAiqAlgoPreResAnr*   anr_pre_res;
    RkAiqAlgoPreResAmerge*  amerge_pre_res;
    RkAiqAlgoPreResAtmo*  atmo_pre_res;
    RkAiqAlgoPreResAcp* acp_pre_res;
    RkAiqAlgoPreResAsharp* asharp_pre_res;
    RkAiqAlgoPreResAdhaz* adhaz_pre_res;
    RkAiqAlgoPreResA3dlut* a3dlut_pre_res;
    RkAiqAlgoPreResAblc* ablc_pre_res;
    RkAiqAlgoPreResAccm* accm_pre_res;
    RkAiqAlgoPreResAcgc* acgc_pre_res;
    RkAiqAlgoPreResAdebayer* adebayer_pre_res;
    RkAiqAlgoPreResAdpcc* adpcc_pre_res;
    RkAiqAlgoPreResAfec* afec_pre_res;
    RkAiqAlgoPreResAgamma* agamma_pre_res;
    RkAiqAlgoPreResAdegamma* adegamma_pre_res;
    RkAiqAlgoPreResAgic* agic_pre_res;
    RkAiqAlgoPreResAie* aie_pre_res;
    RkAiqAlgoPreResAldch* aldch_pre_res;
    RkAiqAlgoPreResAlsc* alsc_pre_res;
    RkAiqAlgoPreResAorb* aorb_pre_res;
    RkAiqAlgoPreResAr2y* ar2y_pre_res;
    RkAiqAlgoPreResAwdr* awdr_pre_res;
    RkAiqAlgoPreResArawnr* arawnr_pre_res;
    RkAiqAlgoPreResAmfnr* amfnr_pre_res;
    RkAiqAlgoPreResAynr* aynr_pre_res;
    RkAiqAlgoPreResAcnr* acnr_pre_res;
    RkAiqAlgoPreResAdrc* adrc_pre_res;
    RkAiqAlgoPreResAeis* aeis_pre_res;
    RkAiqAlgoPreResAmd* amd_pre_res;
    RkAiqAlgoPreResAgain* again_pre_res;
} RkAiqPreResComb;

typedef struct _RkAiqProcResComb {
    RkAiqAlgoProcResAe*    ae_proc_res;
    RkAiqAlgoProcResAwb*   awb_proc_res;
    RkAiqAlgoProcResAf*    af_proc_res;
    RkAiqAlgoProcResAsd*   asd_proc_res;
    RkAiqAlgoProcResAnr*   anr_proc_res;
    RkAiqAlgoProcResAmerge*  amerge_proc_res;
    RkAiqAlgoProcResAtmo*  atmo_proc_res;
    RkAiqAlgoProcResAcp* acp_proc_res;
    RkAiqAlgoProcResAsharp* asharp_proc_res;
    RkAiqAlgoProcResAdhaz* adhaz_proc_res;
    RkAiqAlgoProcResA3dlut* a3dlut_proc_res;
    RkAiqAlgoProcResAblc* ablc_proc_res;
    RkAiqAlgoProcResAccm* accm_proc_res;
    RkAiqAlgoProcResAcgc* acgc_proc_res;
    RkAiqAlgoProcResAdebayer* adebayer_proc_res;
    RkAiqAlgoProcResAdpcc* adpcc_proc_res;
    RkAiqAlgoProcResAfec* afec_proc_res;
    RkAiqAlgoProcResAgamma* agamma_proc_res;
    RkAiqAlgoProcResAdegamma* adegamma_proc_res;
    RkAiqAlgoProcResAgic* agic_proc_res;
    RkAiqAlgoProcResAie* aie_proc_res;
    RkAiqAlgoProcResAldch* aldch_proc_res;
    RkAiqAlgoProcResAlsc* alsc_proc_res;
    RkAiqAlgoProcResAorb* aorb_proc_res;
    RkAiqAlgoProcResAr2y* ar2y_proc_res;
    RkAiqAlgoProcResAwdr* awdr_proc_res;
    RkAiqAlgoProcResArawnr* arawnr_proc_res;
    RkAiqAlgoProcResAmfnr* amfnr_proc_res;
    RkAiqAlgoProcResAynr* aynr_proc_res;
    RkAiqAlgoProcResAcnr* acnr_proc_res;
    RkAiqAlgoProcResAdrc* adrc_proc_res;
    RkAiqAlgoProcResAeis* aeis_proc_res;
    RkAiqAlgoProcResAmd* amd_proc_res;
    RkAiqAlgoProcResAgain* again_proc_res;
} RkAiqProcResComb;

typedef struct _RkAiqPostResComb {
    RkAiqAlgoPostResAe*    ae_post_res;
    RkAiqAlgoPostResAwb*   awb_post_res;
    RkAiqAlgoPostResAf*    af_post_res;
    RkAiqAlgoPostResAsd*   asd_post_res;
    RkAiqAlgoPostResAnr*   anr_post_res;
    RkAiqAlgoPostResAmerge*  amerge_post_res;
    RkAiqAlgoPostResAtmo*  atmo_post_res;
    RkAiqAlgoPostResAcp* acp_post_res;
    RkAiqAlgoPostResAsharp* asharp_post_res;
    RkAiqAlgoPostResAdhaz* adhaz_post_res;
    RkAiqAlgoPostResA3dlut* a3dlut_post_res;
    RkAiqAlgoPostResAblc* ablc_post_res;
    RkAiqAlgoPostResAccm* accm_post_res;
    RkAiqAlgoPostResAcgc* acgc_post_res;
    RkAiqAlgoPostResAdebayer* adebayer_post_res;
    RkAiqAlgoPostResAdpcc* adpcc_post_res;
    RkAiqAlgoPostResAfec* afec_post_res;
    RkAiqAlgoPostResAgamma* agamma_post_res;
    RkAiqAlgoPostResAdegamma* adegamma_post_res;
    RkAiqAlgoPostResAgic* agic_post_res;
    RkAiqAlgoPostResAie* aie_post_res;
    RkAiqAlgoPostResAldch* aldch_post_res;
    RkAiqAlgoPostResAlsc* alsc_post_res;
    RkAiqAlgoPostResAorb* aorb_post_res;
    RkAiqAlgoPostResAr2y* ar2y_post_res;
    RkAiqAlgoPostResAwdr* awdr_post_res;
    RkAiqAlgoPostResArawnr* arawnr_post_res;
    RkAiqAlgoPostResAmfnr* amfnr_post_res;
    RkAiqAlgoPostResAynr* aynr_post_res;
    RkAiqAlgoPostResAcnr* acnr_post_res;
    RkAiqAlgoPostResAdrc* adrc_post_res;
    RkAiqAlgoPostResAeis* aeis_post_res;
    RkAiqAlgoPostResAmd* amd_post_res;
    RkAiqAlgoPostResAgain* again_post_res;
} RkAiqPostResComb;

typedef struct _RkAiqResComb {
    XCamVideoBuffer* ae_pre_res;
    XCamVideoBuffer* ae_proc_res;
    XCamVideoBuffer* awb_proc_res;
} RkAiqResComb;

typedef struct _RkAiqAlgoComInt {
    union {
        struct {
            CamCalibDbContext_t* calib;
            CamCalibDbV2Context_t* calibv2;
        } prepare;

        struct {
            RkAiqPreResComb*  pre_res_comb;
            RkAiqProcResComb* proc_res_comb;
            RkAiqPostResComb* post_res_comb;
            int iso;
            bool fill_light_on;
            bool gray_mode;
            bool is_bw_sensor;
            RKAiqAecExpInfo_t *preExp;
            RKAiqAecExpInfo_t *curExp;
            RkAiqResComb* res_comb;
        } proc;
    } u;
} RkAiqAlgoComInt;

typedef struct _AlgoCtxInstanceCfgInt {
    AlgoCtxInstanceCfg cfg_com;
    CamCalibDbContext_t* calib;
    CamCalibDbV2Context_t* calibv2;
} AlgoCtxInstanceCfgInt;

// Ae
typedef struct _RkAiqAlgoConfigAeInt {
    RkAiqAlgoConfigAe        ae_cfg_com;
    RkAiqAlgoComInt          rk_com;
    int                      RawWidth;
    int                      RawHeight;
    rk_aiq_sensor_nr_switch_t nr_switch;
} RkAiqAlgoConfigAeInt;

typedef struct _RkAiqAlgoPreAeInt {
    RkAiqAlgoPreAe ae_pre_com;
    RkAiqAlgoComInt rk_com;
    XCamVideoBuffer* aecStatsBuf;
} RkAiqAlgoPreAeInt;

typedef struct _RkAiqAlgoPreResAeInt {
    RkAiqAlgoPreResAe ae_pre_res_com;
    AecPreResult_t    ae_pre_res_rk;
} RkAiqAlgoPreResAeInt;

typedef struct _RkAiqAlgoProcAeInt {
    RkAiqAlgoProcAe ae_proc_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoProcAeInt;

typedef struct _RkAiqAlgoProcResAeInt {
    RkAiqAlgoProcResAe   ae_proc_res_com;
    AecProcResult_t      ae_proc_res_rk;
} RkAiqAlgoProcResAeInt;

typedef struct _RkAiqAlgoPostAeInt {
    RkAiqAlgoPostAe ae_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAeInt;

typedef struct _RkAiqAlgoPostResAeInt {
    RkAiqAlgoPostResAe ae_post_res_com;
    AecPostResult_t    ae_post_res_rk;
} RkAiqAlgoPostResAeInt;

//Awb
typedef struct _RkAiqAlgoConfigAwbInt {
    RkAiqAlgoConfigAwb awb_config_com;
    RkAiqAlgoComInt rk_com;
    int rawBit;
} RkAiqAlgoConfigAwbInt;

typedef struct _RkAiqAlgoPreAwbInt {
    RkAiqAlgoPreAwb awb_pre_com;
    RkAiqAlgoComInt rk_com;
    XCamVideoBuffer* awbStatsBuf;
} RkAiqAlgoPreAwbInt;

typedef struct _RkAiqAlgoPreResAwbInt {
    RkAiqAlgoPreResAwb awb_pre_res_com;
    color_tempture_info_t cctGloabl;
    color_tempture_info_t cctFirst[4];
    float awb_smooth_factor;
    float varianceLuma;
    rk_aiq_wb_gain_t awb_gain_algo;
    bool awbConverged;
    //blk
    bool blkWpFlagVaLid[RK_AIQ_AWB_GRID_NUM_TOTAL][3];
    int  blkWpFlag[RK_AIQ_AWB_GRID_NUM_TOTAL][3];
    bool blkSgcResVaLid;
    awb_measure_blk_res_fl_t blkSgcResult[RK_AIQ_AWB_GRID_NUM_TOTAL];
} RkAiqAlgoPreResAwbInt;

typedef struct _RkAiqAlgoProcAwbInt {
    RkAiqAlgoProcAwb awb_proc_com;
    RkAiqAlgoComInt rk_com;

} RkAiqAlgoProcAwbInt;

typedef struct _RkAiqAlgoProcResAwbInt {

    RkAiqAlgoProcResAwb awb_proc_res_com;
    color_tempture_info_t cctGloabl;
    color_tempture_info_t cctFirst[4];
    color_tempture_info_t cctBlk[RK_AIQ_AWB_GRID_NUM_TOTAL];
    float awb_smooth_factor;
    float varianceLuma;
    bool awbConverged;
    //blk
    bool blkWpFlagVaLid[RK_AIQ_AWB_GRID_NUM_TOTAL];
    int  blkWpFlag[RK_AIQ_AWB_GRID_NUM_TOTAL][3];
    bool blkSgcResVaLid;
    awb_measure_blk_res_fl_t blkSgcResult[RK_AIQ_AWB_GRID_NUM_TOTAL];

    uint32_t id;
} RkAiqAlgoProcResAwbInt;

typedef struct _RkAiqAlgoPostAwbInt {
    RkAiqAlgoPostAwb awb_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAwbInt;

typedef struct _RkAiqAlgoPostResAwbInt {
    RkAiqAlgoPostResAwb awb_post_res_com;
} RkAiqAlgoPostResAwbInt;

// af
typedef struct _RkAiqAlgoConfigAfInt {
    RkAiqAlgoConfigAf af_config_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoConfigAfInt;

typedef struct _RkAiqAlgoPreAfInt {
    RkAiqAlgoPreAf af_pre_com;
    RkAiqAlgoComInt rk_com;
    rk_aiq_isp_af_stats_t *af_stats;
    rk_aiq_isp_aec_stats_t *aec_stats;
    XCamVideoBuffer* xcam_af_stats;
    XCamVideoBuffer* xcam_aec_stats;
} RkAiqAlgoPreAfInt;

typedef struct _RkAiqAlgoPreResAfInt {
    RkAiqAlgoPreResAf af_pre_res_com;
    af_preprocess_result_t af_pre_result;
} RkAiqAlgoPreResAfInt;

typedef struct _RkAiqAlgoProcAfInt {
    RkAiqAlgoProcAf af_proc_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoProcAfInt;

typedef struct _RkAiqAlgoProcResAfInt {
    RkAiqAlgoProcResAf af_proc_res_com;

    uint32_t id;
} RkAiqAlgoProcResAfInt;

typedef struct _RkAiqAlgoPostAfInt {
    RkAiqAlgoPostAf af_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAfInt;

typedef struct _RkAiqAlgoPostResAfInt {
    RkAiqAlgoPostResAf af_post_res_com;
} RkAiqAlgoPostResAfInt;

// anr
typedef struct _RkAiqAlgoConfigAnrInt {
    RkAiqAlgoConfigAnr anr_config_com;
    RkAiqAlgoComInt rk_com;
    ANRConfig_t stANRConfig;
} RkAiqAlgoConfigAnrInt;

typedef struct _RkAiqAlgoPreAnrInt {
    RkAiqAlgoPreAnr anr_pre_com;
    RkAiqAlgoComInt rk_com;
    ANRConfig_t stANRConfig;
} RkAiqAlgoPreAnrInt;

typedef struct _RkAiqAlgoPreResAnrInt {
    RkAiqAlgoPreResAnr anr_pre_res_com;
} RkAiqAlgoPreResAnrInt;

typedef struct _RkAiqAlgoProcAnrInt {
    RkAiqAlgoProcAnr anr_proc_com;
    RkAiqAlgoComInt rk_com;
    int iso;
    int hdr_mode;
} RkAiqAlgoProcAnrInt;

typedef struct _RkAiqAlgoProcResAnrInt {
    RkAiqAlgoProcResAnr anr_proc_res_com;
    ANRProcResult_t stAnrProcResult;
} RkAiqAlgoProcResAnrInt;

typedef struct _RkAiqAlgoPostAnrInt {
    RkAiqAlgoPostAnr anr_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAnrInt;

typedef struct _RkAiqAlgoPostResAnrInt {
    RkAiqAlgoPostResAnr anr_post_res_com;
} RkAiqAlgoPostResAnrInt;

// asharp
typedef struct _RkAiqAlgoConfigAsharpInt {
    RkAiqAlgoConfigAsharp asharp_config_com;
    RkAiqAlgoComInt rk_com;
    AsharpConfig_t stAsharpConfig;
} RkAiqAlgoConfigAsharpInt;

typedef struct _RkAiqAlgoPreAsharpInt {
    RkAiqAlgoPreAsharp asharp_pre_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPreAsharpInt;

typedef struct _RkAiqAlgoPreResAsharpInt {
    RkAiqAlgoPreResAsharp asharp_pre_res_com;
} RkAiqAlgoPreResAsharpInt;

typedef struct _RkAiqAlgoProcAsharpInt {
    RkAiqAlgoProcAsharp asharp_proc_com;
    RkAiqAlgoComInt rk_com;
    int iso;
    int hdr_mode;
} RkAiqAlgoProcAsharpInt;

typedef struct _RkAiqAlgoProcResAsharpInt {
    RkAiqAlgoProcResAsharp asharp_proc_res_com;
    AsharpProcResult_t stAsharpProcResult;
} RkAiqAlgoProcResAsharpInt;

typedef struct _RkAiqAlgoPostAsharpInt {
    RkAiqAlgoPostAsharp asharp_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAsharpInt;

typedef struct _RkAiqAlgoPostResAsharpInt {
    RkAiqAlgoPostResAsharp asharp_post_res_com;
} RkAiqAlgoPostResAsharpInt;


// asd
typedef struct _RkAiqAlgoConfigAsdInt {
    RkAiqAlgoConfigAsd asd_config_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoConfigAsdInt;

typedef struct _RkAiqAlgoPreAsdInt {
    RkAiqAlgoPreAsd asd_pre_com;
    RkAiqAlgoComInt rk_com;
    asd_preprocess_in_t pre_params;
} RkAiqAlgoPreAsdInt;

typedef struct _RkAiqAlgoPreResAsdInt {
    RkAiqAlgoPreResAsd asd_pre_res_com;
    asd_preprocess_result_t asd_result;
} RkAiqAlgoPreResAsdInt;

typedef struct _RkAiqAlgoProcAsdInt {
    RkAiqAlgoProcAsd asd_proc_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoProcAsdInt;

typedef struct _RkAiqAlgoProcResAsdInt {
    RkAiqAlgoProcResAsd asd_proc_res_com;
} RkAiqAlgoProcResAsdInt;

typedef struct _RkAiqAlgoPostAsdInt {
    RkAiqAlgoPostAsd asd_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAsdInt;

typedef struct _RkAiqAlgoPostResAsdInt {
    RkAiqAlgoPostResAsd asd_post_res_com;
} RkAiqAlgoPostResAsdInt;

// amerge
typedef struct _RkAiqAlgoConfigAmergeInt {
    RkAiqAlgoConfigAmerge amerge_config_com;
    RkAiqAlgoComInt rk_com;
    //todo
    AmergeConfig_t* amergeCfg;
    int working_mode;
    unsigned short  rawWidth;               // raw图宽
    unsigned short  rawHeight;
} RkAiqAlgoConfigAmergeInt;

typedef struct _RkAiqAlgoPreAmergeInt {
    RkAiqAlgoPreAmerge amerge_pre_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPreAmergeInt;

typedef struct _RkAiqAlgoPreResAmergeInt {
    RkAiqAlgoPreResAmerge amerge_pre_res_com;
} RkAiqAlgoPreResAmergeInt;

typedef struct _RkAiqAlgoProcAmergeInt {
    RkAiqAlgoProcAmerge amerge_proc_com;
    RkAiqAlgoComInt rk_com;
    uint32_t width;
    uint32_t height;
} RkAiqAlgoProcAmergeInt;

typedef struct _RkAiqAlgoProcResAmergeInt {
    RkAiqAlgoProcResAmerge amerge_proc_res_com;
    //todo lrk
    RkAiqAmergeProcResult_t AmergeProcRes;
} RkAiqAlgoProcResAmergeInt;

typedef struct _RkAiqAlgoPostAmergeInt {
    RkAiqAlgoPostAmerge amerge_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAmergeInt;

typedef struct _RkAiqAlgoPostResAmergeInt {
    RkAiqAlgoPostResAmerge amerge_post_res_com;
} RkAiqAlgoPostResAmergeInt;

// atmo
typedef struct _RkAiqAlgoConfigAtmoInt {
    RkAiqAlgoConfigAtmo atmo_config_com;
    RkAiqAlgoComInt rk_com;
    //todo
    AtmoConfig_t* atmoCfg;
    int working_mode;
    unsigned short  rawWidth;               // raw图宽
    unsigned short  rawHeight;
} RkAiqAlgoConfigAtmoInt;

typedef struct _RkAiqAlgoPreAtmoInt {
    RkAiqAlgoPreAtmo atmo_pre_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPreAtmoInt;

typedef struct _RkAiqAlgoPreResAtmoInt {
    RkAiqAlgoPreResAtmo atmo_pre_res_com;
} RkAiqAlgoPreResAtmoInt;

typedef struct _RkAiqAlgoProcAtmoInt {
    RkAiqAlgoProcAtmo atmo_proc_com;
    RkAiqAlgoComInt rk_com;
    rkisp_atmo_stats_t ispAtmoStats;
    uint32_t width;
    uint32_t height;
} RkAiqAlgoProcAtmoInt;

typedef struct _RkAiqAlgoProcResAtmoInt {
    RkAiqAlgoProcResAtmo atmo_proc_res_com;
    //todo lrk
    RkAiqAtmoProcResult_t AtmoProcRes;
} RkAiqAlgoProcResAtmoInt;

typedef struct _RkAiqAlgoPostAtmoInt {
    RkAiqAlgoPostAtmo atmo_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAtmoInt;

typedef struct _RkAiqAlgoPostResAtmoInt {
    RkAiqAlgoPostResAtmo atmo_post_res_com;
} RkAiqAlgoPostResAtmoInt;


// acp
typedef struct _RkAiqAlgoConfigAcpInt {
    RkAiqAlgoConfigAcp acp_config_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoConfigAcpInt;

typedef struct _RkAiqAlgoPreAcpInt {
    RkAiqAlgoPreAcp acp_pre_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPreAcpInt;

typedef struct _RkAiqAlgoPreResAcpInt {
    RkAiqAlgoPreResAcp acp_pre_res_com;
} RkAiqAlgoPreResAcpInt;

typedef struct _RkAiqAlgoProcAcpInt {
    RkAiqAlgoProcAcp acp_proc_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoProcAcpInt;

typedef struct _RkAiqAlgoProcResAcpInt {
    RkAiqAlgoProcResAcp acp_proc_res_com;
} RkAiqAlgoProcResAcpInt;

typedef struct _RkAiqAlgoPostAcpInt {
    RkAiqAlgoPostAcp acp_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAcpInt;

typedef struct _RkAiqAlgoPostResAcpInt {
    RkAiqAlgoPostResAcp acp_post_res_com;
} RkAiqAlgoPostResAcpInt;

//adehaze
typedef struct _RkAiqAlgoConfigAdhazInt {
    RkAiqAlgoConfigAdhaz adhaz_config_com;
    RkAiqAlgoComInt rk_com;
    const CamCalibDbContext_t *calib;
    int working_mode;
} RkAiqAlgoConfigAdhazInt;

typedef struct _RkAiqAlgoPreAdhazInt {
    RkAiqAlgoPreAdhaz adhaz_pre_com;
    RkAiqAlgoComInt rk_com;
    int rawHeight;
    int rawWidth;
} RkAiqAlgoPreAdhazInt;

typedef struct _RkAiqAlgoPreResAdhazInt {
    RkAiqAlgoPreResAdhaz adhaz_pre_res_com;
} RkAiqAlgoPreResAdhazInt;

typedef struct _RkAiqAlgoProcAdhazInt {
    RkAiqAlgoProcAdhaz adhaz_proc_com;
    RkAiqAlgoComInt rk_com;
    const CamCalibDbContext_t *pCalibDehaze;
    int iso;
    int hdr_mode;
    rkisp_adehaze_stats_t stats;
} RkAiqAlgoProcAdhazInt;

typedef struct _RkAiqAlgoProcResAdhazInt {
    RkAiqAlgoProcResAdhaz adhaz_proc_res_com;

} RkAiqAlgoProcResAdhazInt;


typedef struct _RkAiqAlgoPostAdhazInt {
    RkAiqAlgoPostAdhaz adhaz_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAdhazInt;

typedef struct _RkAiqAlgoPostResAdhazInt {
    RkAiqAlgoPostResAdhaz adhaz_post_res_com;
} RkAiqAlgoPostResAdhazInt;

// a3dlut
typedef struct _RkAiqAlgoConfigA3dlutInt {
    RkAiqAlgoConfigA3dlut a3dlut_config_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoConfigA3dlutInt;

typedef struct _RkAiqAlgoPreA3dlutInt {
    RkAiqAlgoPreA3dlut a3dlut_pre_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPreA3dlutInt;

typedef struct _RkAiqAlgoPreResA3dlutInt {
    RkAiqAlgoPreResA3dlut a3dlut_pre_res_com;
} RkAiqAlgoPreResA3dlutInt;

typedef struct _RkAiqAlgoProcA3dlutInt {
    RkAiqAlgoProcA3dlut a3dlut_proc_com;
    RkAiqAlgoComInt rk_com;
    float sensorGain;
    float awbGain[2];
    float awbIIRDampCoef;
    bool awbConverged;
} RkAiqAlgoProcA3dlutInt;

typedef struct _RkAiqAlgoProcResA3dlutInt {
    RkAiqAlgoProcResA3dlut a3dlut_proc_res_com;
} RkAiqAlgoProcResA3dlutInt;

typedef struct _RkAiqAlgoPostA3dlutInt {
    RkAiqAlgoPostA3dlut a3dlut_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostA3dlutInt;

typedef struct _RkAiqAlgoPostResA3dlutInt {
    RkAiqAlgoPostResA3dlut a3dlut_post_res_com;
} RkAiqAlgoPostResA3dlutInt;

// ablc
typedef struct _RkAiqAlgoConfigAblcInt {
    RkAiqAlgoConfigAblc ablc_config_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoConfigAblcInt;

typedef struct _RkAiqAlgoPreAblcInt {
    RkAiqAlgoPreAblc ablc_pre_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPreAblcInt;

typedef struct _RkAiqAlgoPreResAblcInt {
    RkAiqAlgoPreResAblc ablc_pre_res_com;
} RkAiqAlgoPreResAblcInt;

typedef struct _RkAiqAlgoProcAblcInt {
    RkAiqAlgoProcAblc ablc_proc_com;
    RkAiqAlgoComInt rk_com;
    int iso;
    int hdr_mode;
} RkAiqAlgoProcAblcInt;

typedef struct _RkAiqAlgoProcResAblcInt {
    RkAiqAlgoProcResAblc ablc_proc_res_com;
    AblcProc_t ablc_proc_res;
} RkAiqAlgoProcResAblcInt;

typedef struct _RkAiqAlgoPostAblcInt {
    RkAiqAlgoPostAblc ablc_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAblcInt;

typedef struct _RkAiqAlgoPostResAblcInt {
    RkAiqAlgoPostResAblc ablc_post_res_com;
} RkAiqAlgoPostResAblcInt;

// accm
typedef struct _RkAiqAlgoConfigAccmInt {
    RkAiqAlgoConfigAccm accm_config_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoConfigAccmInt;

typedef struct _RkAiqAlgoPreAccmInt {
    RkAiqAlgoPreAccm accm_pre_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPreAccmInt;

typedef struct _RkAiqAlgoPreResAccmInt {
    RkAiqAlgoPreResAccm accm_pre_res_com;
} RkAiqAlgoPreResAccmInt;

typedef struct _RkAiqAlgoProcAccmInt {
    RkAiqAlgoProcAccm accm_proc_com;
    RkAiqAlgoComInt rk_com;
    accm_sw_info_t   accm_sw_info;
} RkAiqAlgoProcAccmInt;

typedef struct _RkAiqAlgoProcResAccmInt {
    RkAiqAlgoProcResAccm accm_proc_res_com;
} RkAiqAlgoProcResAccmInt;

typedef struct _RkAiqAlgoPostAccmInt {
    RkAiqAlgoPostAccm accm_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAccmInt;

typedef struct _RkAiqAlgoPostResAccmInt {
    RkAiqAlgoPostResAccm accm_post_res_com;
} RkAiqAlgoPostResAccmInt;

// acgc
typedef struct _RkAiqAlgoConfigAcgcInt {
    RkAiqAlgoConfigAcgc acgc_config_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoConfigAcgcInt;

typedef struct _RkAiqAlgoPreAcgcInt {
    RkAiqAlgoPreAcgc acgc_pre_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPreAcgcInt;

typedef struct _RkAiqAlgoPreResAcgcInt {
    RkAiqAlgoPreResAcgc acgc_pre_res_com;
} RkAiqAlgoPreResAcgcInt;

typedef struct _RkAiqAlgoProcAcgcInt {
    RkAiqAlgoProcAcgc acgc_proc_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoProcAcgcInt;

typedef struct _RkAiqAlgoProcResAcgcInt {
    RkAiqAlgoProcResAcgc acgc_proc_res_com;
} RkAiqAlgoProcResAcgcInt;

typedef struct _RkAiqAlgoPostAcgcInt {
    RkAiqAlgoPostAcgc acgc_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAcgcInt;

typedef struct _RkAiqAlgoPostResAcgcInt {
    RkAiqAlgoPostResAcgc acgc_post_res_com;
} RkAiqAlgoPostResAcgcInt;

// adebayer
typedef struct _RkAiqAlgoConfigAdebayerInt {
    RkAiqAlgoConfigAdebayer adebayer_config_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoConfigAdebayerInt;

typedef struct _RkAiqAlgoPreAdebayerInt {
    RkAiqAlgoPreAdebayer adebayer_pre_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPreAdebayerInt;

typedef struct _RkAiqAlgoPreResAdebayerInt {
    RkAiqAlgoPreResAdebayer adebayer_pre_res_com;
} RkAiqAlgoPreResAdebayerInt;

typedef struct _RkAiqAlgoProcAdebayerInt {
    RkAiqAlgoProcAdebayer adebayer_proc_com;
    RkAiqAlgoComInt rk_com;
    int hdr_mode;
} RkAiqAlgoProcAdebayerInt;

typedef struct _RkAiqAlgoProcResAdebayerInt {
    RkAiqAlgoProcResAdebayer adebayer_proc_res_com;
    AdebayerProcResult_t debayerRes;
} RkAiqAlgoProcResAdebayerInt;

typedef struct _RkAiqAlgoPostAdebayerInt {
    RkAiqAlgoPostAdebayer adebayer_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAdebayerInt;

typedef struct _RkAiqAlgoPostResAdebayerInt {
    RkAiqAlgoPostResAdebayer adebayer_post_res_com;
} RkAiqAlgoPostResAdebayerInt;

// adpcc
typedef struct _RkAiqAlgoConfigAdpccInt {
    RkAiqAlgoConfigAdpcc adpcc_config_com;
    RkAiqAlgoComInt rk_com;
    AdpccConfig_t stAdpccConfig;
} RkAiqAlgoConfigAdpccInt;

typedef struct _RkAiqAlgoPreAdpccInt {
    RkAiqAlgoPreAdpcc adpcc_pre_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPreAdpccInt;

typedef struct _RkAiqAlgoPreResAdpccInt {
    RkAiqAlgoPreResAdpcc adpcc_pre_res_com;
} RkAiqAlgoPreResAdpccInt;

typedef struct _RkAiqAlgoProcAdpccInt {
    RkAiqAlgoProcAdpcc adpcc_proc_com;
    RkAiqAlgoComInt rk_com;
    int iso;
    int hdr_mode;
} RkAiqAlgoProcAdpccInt;

typedef struct _RkAiqAlgoProcResAdpccInt {
    RkAiqAlgoProcResAdpcc adpcc_proc_res_com;
    AdpccProcResult_t stAdpccProcResult;
} RkAiqAlgoProcResAdpccInt;

typedef struct _RkAiqAlgoPostAdpccInt {
    RkAiqAlgoPostAdpcc adpcc_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAdpccInt;

typedef struct _RkAiqAlgoPostResAdpccInt {
    RkAiqAlgoPostResAdpcc adpcc_post_res_com;
} RkAiqAlgoPostResAdpccInt;

// afec
typedef struct _RkAiqAlgoConfigAfecInt {
    RkAiqAlgoConfigAfec afec_config_com;
    RkAiqAlgoComInt rk_com;
    CalibDb_FEC_t afec_calib_cfg;
    const char* resource_path;
    isp_drv_share_mem_ops_t *mem_ops_ptr;
} RkAiqAlgoConfigAfecInt;

typedef struct _RkAiqAlgoPreAfecInt {
    RkAiqAlgoPreAfec afec_pre_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPreAfecInt;

typedef struct _RkAiqAlgoPreResAfecInt {
    RkAiqAlgoPreResAfec afec_pre_res_com;
} RkAiqAlgoPreResAfecInt;

typedef struct _RkAiqAlgoProcAfecInt {
    RkAiqAlgoProcAfec afec_proc_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoProcAfecInt;

typedef struct _RkAiqAlgoProcResAfecInt {
    RkAiqAlgoProcResAfec afec_proc_res_com;
    fec_preprocess_result_t afec_result;
} RkAiqAlgoProcResAfecInt;

typedef struct _RkAiqAlgoPostAfecInt {
    RkAiqAlgoPostAfec afec_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAfecInt;

typedef struct _RkAiqAlgoPostResAfecInt {
    RkAiqAlgoPostResAfec afec_post_res_com;
} RkAiqAlgoPostResAfecInt;

// agamma
typedef struct _RkAiqAlgoConfigAgammaInt {
    RkAiqAlgoConfigAgamma agamma_config_com;
    RkAiqAlgoComInt rk_com;
    const CamCalibDbContext_t *calib;
} RkAiqAlgoConfigAgammaInt;

typedef struct _RkAiqAlgoPreAgammaInt {
    RkAiqAlgoPreAgamma agamma_pre_com;
    RkAiqAlgoComInt rk_com;
    int work_mode;
} RkAiqAlgoPreAgammaInt;

typedef struct _RkAiqAlgoPreResAgammaInt {
    RkAiqAlgoPreResAgamma agamma_pre_res_com;
    rk_aiq_gamma_cfg_t agamma_config;
} RkAiqAlgoPreResAgammaInt;

typedef struct _RkAiqAlgoProcAgammaInt {
    RkAiqAlgoProcAgamma agamma_proc_com;
    RkAiqAlgoComInt rk_com;
    const CamCalibDbContext_t *calib;
} RkAiqAlgoProcAgammaInt;

typedef struct _RkAiqAlgoProcResAgammaInt {
    RkAiqAlgoProcResAgamma agamma_proc_res_com;

} RkAiqAlgoProcResAgammaInt;

typedef struct _RkAiqAlgoPostAgammaInt {
    RkAiqAlgoPostAgamma agamma_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAgammaInt;

typedef struct _RkAiqAlgoPostResAgammaInt {
    RkAiqAlgoPostResAgamma agamma_post_res_com;
} RkAiqAlgoPostResAgammaInt;

// adegamma
typedef struct _RkAiqAlgoConfigAdegammaInt {
    RkAiqAlgoConfigAdegamma adegamma_config_com;
    RkAiqAlgoComInt rk_com;
    const CamCalibDbContext_t *calib;
} RkAiqAlgoConfigAdegammaInt;

typedef struct _RkAiqAlgoPreAdegammaInt {
    RkAiqAlgoPreAdegamma adegamma_pre_com;
    RkAiqAlgoComInt rk_com;
    int work_mode;
} RkAiqAlgoPreAdegammaInt;

typedef struct _RkAiqAlgoPreResAdegammaInt {
    RkAiqAlgoPreResAdegamma adegamma_pre_res_com;
    rk_aiq_degamma_cfg_t adegamma_config;
} RkAiqAlgoPreResAdegammaInt;

typedef struct _RkAiqAlgoProcAdegammaInt {
    RkAiqAlgoProcAdegamma adegamma_proc_com;
    RkAiqAlgoComInt rk_com;
    const CamCalibDbContext_t *calib;
} RkAiqAlgoProcAdegammaInt;

typedef struct _RkAiqAlgoProcResAdegammaInt {
    RkAiqAlgoProcResAdegamma adegamma_proc_res_com;

} RkAiqAlgoProcResAdegammaInt;

typedef struct _RkAiqAlgoPostAdegammaInt {
    RkAiqAlgoPostAdegamma adegamma_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAdegammaInt;

typedef struct _RkAiqAlgoPostResAdegammaInt {
    RkAiqAlgoPostResAdegamma adegamma_post_res_com;
} RkAiqAlgoPostResAdegammaInt;


// agic
typedef struct _RkAiqAlgoConfigAgicInt {
    RkAiqAlgoConfigAgic agic_config_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoConfigAgicInt;

typedef struct _RkAiqAlgoPreAgicInt {
    RkAiqAlgoPreAgic agic_pre_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPreAgicInt;

typedef struct _RkAiqAlgoPreResAgicInt {
    RkAiqAlgoPreResAgic agic_pre_res_com;
} RkAiqAlgoPreResAgicInt;

typedef struct _RkAiqAlgoProcAgicInt {
    RkAiqAlgoProcAgic agic_proc_com;
    RkAiqAlgoComInt rk_com;
    int hdr_mode;
} RkAiqAlgoProcAgicInt;

typedef struct _RkAiqAlgoProcResAgicInt {
    RkAiqAlgoProcResAgic agic_proc_res_com;
    AgicProcResult_t gicRes;
} RkAiqAlgoProcResAgicInt;

typedef struct _RkAiqAlgoPostAgicInt {
    RkAiqAlgoPostAgic agic_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAgicInt;

typedef struct _RkAiqAlgoPostResAgicInt {
    RkAiqAlgoPostResAgic agic_post_res_com;
} RkAiqAlgoPostResAgicInt;

// aie
typedef struct _RkAiqAlgoConfigAieInt {
    RkAiqAlgoConfigAie aie_config_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoConfigAieInt;

typedef struct _RkAiqAlgoPreAieInt {
    RkAiqAlgoPreAie aie_pre_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPreAieInt;

typedef struct _RkAiqAlgoPreResAieInt {
    RkAiqAlgoPreResAie aie_pre_res_com;
} RkAiqAlgoPreResAieInt;

typedef struct _RkAiqAlgoProcAieInt {
    RkAiqAlgoProcAie aie_proc_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoProcAieInt;

typedef struct _RkAiqAlgoProcResAieInt {
    RkAiqAlgoProcResAie aie_proc_res_com;
    rk_aiq_aie_params_int_t params;
} RkAiqAlgoProcResAieInt;

typedef struct _RkAiqAlgoPostAieInt {
    RkAiqAlgoPostAie aie_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAieInt;

typedef struct _RkAiqAlgoPostResAieInt {
    RkAiqAlgoPostResAie aie_post_res_com;
} RkAiqAlgoPostResAieInt;

// aldch
typedef struct _RkAiqAlgoConfigAldchInt {
    RkAiqAlgoConfigAldch aldch_config_com;
    RkAiqAlgoComInt rk_com;
    CalibDb_LDCH_t aldch_calib_cfg;
    const char* resource_path;
    isp_drv_share_mem_ops_t *mem_ops_ptr;
} RkAiqAlgoConfigAldchInt;

typedef struct _RkAiqAlgoPreAldchInt {
    RkAiqAlgoPreAldch aldch_pre_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPreAldchInt;

typedef struct _RkAiqAlgoPreResAldchInt {
    RkAiqAlgoPreResAldch aldch_pre_res_com;
} RkAiqAlgoPreResAldchInt;

typedef struct _RkAiqAlgoProcAldchInt {
    RkAiqAlgoProcAldch aldch_proc_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoProcAldchInt;

typedef struct _RkAiqAlgoProcResAldchInt {
    RkAiqAlgoProcResAldch aldch_proc_res_com;
    ldch_process_result_t ldch_result;
} RkAiqAlgoProcResAldchInt;

typedef struct _RkAiqAlgoPostAldchInt {
    RkAiqAlgoPostAldch aldch_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAldchInt;

typedef struct _RkAiqAlgoPostResAldchInt {
    RkAiqAlgoPostResAldch aldch_post_res_com;
} RkAiqAlgoPostResAldchInt;

// alsc
typedef struct _RkAiqAlgoConfigAlscInt {
    RkAiqAlgoConfigAlsc alsc_config_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoConfigAlscInt;

typedef struct _RkAiqAlgoPreAlscInt {
    RkAiqAlgoPreAlsc alsc_pre_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPreAlscInt;

typedef struct _RkAiqAlgoPreResAlscInt {
    RkAiqAlgoPreResAlsc alsc_pre_res_com;
} RkAiqAlgoPreResAlscInt;

typedef struct _RkAiqAlgoProcAlscInt {
    RkAiqAlgoProcAlsc alsc_proc_com;
    RkAiqAlgoComInt rk_com;
    alsc_sw_info_t   alsc_sw_info;
    XCamVideoBuffer* tx;
} RkAiqAlgoProcAlscInt;

typedef struct _RkAiqAlgoProcResAlscInt {
    RkAiqAlgoProcResAlsc alsc_proc_res_com;
} RkAiqAlgoProcResAlscInt;

typedef struct _RkAiqAlgoPostAlscInt {
    RkAiqAlgoPostAlsc alsc_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAlscInt;

typedef struct _RkAiqAlgoPostResAlscInt {
    RkAiqAlgoPostResAlsc alsc_post_res_com;
} RkAiqAlgoPostResAlscInt;

// aorb
typedef struct _RkAiqAlgoConfigAorbInt {
    RkAiqAlgoConfigAorb aorb_config_com;
    RkAiqAlgoComInt rk_com;
    CalibDbV2_Orb_t orb_calib_cfg;
} RkAiqAlgoConfigAorbInt;

typedef struct _RkAiqAlgoPreAorbInt {
    RkAiqAlgoPreAorb aorb_pre_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPreAorbInt;

typedef struct _RkAiqAlgoPreResAorbInt {
    RkAiqAlgoPreResAorb aorb_pre_res_com;
} RkAiqAlgoPreResAorbInt;

typedef struct _RkAiqAlgoProcAorbInt {
    RkAiqAlgoProcAorb aorb_proc_com;
    RkAiqAlgoComInt rk_com;
    XCamVideoBuffer* orb_stats_buf;
} RkAiqAlgoProcAorbInt;

typedef struct _RkAiqAlgoProcResAorbInt {
    RkAiqAlgoProcResAorb aorb_proc_res_com;
    rk_aiq_isp_orb_meas_t aorb_meas;
    orb_preprocess_result_t orb_pre_result;
} RkAiqAlgoProcResAorbInt;

typedef struct _RkAiqAlgoPostAorbInt {
    RkAiqAlgoPostAorb aorb_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAorbInt;

typedef struct _RkAiqAlgoPostResAorbInt {
    RkAiqAlgoPostResAorb aorb_post_res_com;
} RkAiqAlgoPostResAorbInt;

// ar2y
typedef struct _RkAiqAlgoConfigAr2yInt {
    RkAiqAlgoConfigAr2y ar2y_config_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoConfigAr2yInt;

typedef struct _RkAiqAlgoPreAr2yInt {
    RkAiqAlgoPreAr2y ar2y_pre_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPreAr2yInt;

typedef struct _RkAiqAlgoPreResAr2yInt {
    RkAiqAlgoPreResAr2y ar2y_pre_res_com;
} RkAiqAlgoPreResAr2yInt;

typedef struct _RkAiqAlgoProcAr2yInt {
    RkAiqAlgoProcAr2y ar2y_proc_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoProcAr2yInt;

typedef struct _RkAiqAlgoProcResAr2yInt {
    RkAiqAlgoProcResAr2y ar2y_proc_res_com;
} RkAiqAlgoProcResAr2yInt;

typedef struct _RkAiqAlgoPostAr2yInt {
    RkAiqAlgoPostAr2y ar2y_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAr2yInt;

typedef struct _RkAiqAlgoPostResAr2yInt {
    RkAiqAlgoPostResAr2y ar2y_post_res_com;
} RkAiqAlgoPostResAr2yInt;

// awdr
typedef struct _RkAiqAlgoConfigAwdrInt {
    RkAiqAlgoConfigAwdr awdr_config_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoConfigAwdrInt;

typedef struct _RkAiqAlgoPreAwdrInt {
    RkAiqAlgoPreAwdr awdr_pre_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPreAwdrInt;

typedef struct _RkAiqAlgoPreResAwdrInt {
    RkAiqAlgoPreResAwdr awdr_pre_res_com;
} RkAiqAlgoPreResAwdrInt;

typedef struct _RkAiqAlgoProcAwdrInt {
    RkAiqAlgoProcAwdr awdr_proc_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoProcAwdrInt;

typedef struct _RkAiqAlgoProcResAwdrInt {
    RkAiqAlgoProcResAwdr awdr_proc_res_com;
} RkAiqAlgoProcResAwdrInt;

typedef struct _RkAiqAlgoPostAwdrInt {
    RkAiqAlgoPostAwdr awdr_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAwdrInt;

typedef struct _RkAiqAlgoPostResAwdrInt {
    RkAiqAlgoPostResAwdr awdr_post_res_com;
} RkAiqAlgoPostResAwdrInt;


// adrc
typedef struct _RkAiqAlgoConfigAdrcInt {
    RkAiqAlgoConfigAdrc adrc_config_com;
    RkAiqAlgoComInt rk_com;
    //AdrcConfig_t* adrcCfg;
    int working_mode;
    unsigned short  rawWidth;               // raw图宽
    unsigned short  rawHeight;
} RkAiqAlgoConfigAdrcInt;

typedef struct _RkAiqAlgoPreAdrcInt {
    RkAiqAlgoPreAdrc adrc_pre_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPreAdrcInt;

typedef struct _RkAiqAlgoPreResAdrcInt {
    RkAiqAlgoPreResAdrc adrc_pre_res_com;
} RkAiqAlgoPreResAdrcInt;

typedef struct _RkAiqAlgoProcAdrcInt {
    RkAiqAlgoProcAdrc adrc_proc_com;
    RkAiqAlgoComInt rk_com;
    rkisp_adrc_stats_t ispAdrcStats;
    uint32_t width;
    uint32_t height;
} RkAiqAlgoProcAdrcInt;

typedef struct _RkAiqAlgoProcResAdrcInt {
    RkAiqAlgoProcResAdrc adrc_proc_res_com;
    RkAiqAdrcProcResult_t AdrcProcRes;
} RkAiqAlgoProcResAdrcInt;

typedef struct _RkAiqAlgoPostAdrcInt {
    RkAiqAlgoPostAdrc adrc_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAdrcInt;

typedef struct _RkAiqAlgoPostResAdrcInt {
    RkAiqAlgoPostResAdrc adrc_post_res_com;
} RkAiqAlgoPostResAdrcInt;

// adrcV1
typedef struct _RkAiqAlgoConfigAdrcV1Int {
    RkAiqAlgoConfigAdrc adrc_config_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoConfigAdrcV1Int;

typedef struct _RkAiqAlgoPreAdrcV1Int {
    RkAiqAlgoPreAdrc adrc_pre_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPreAdrcV1Int;

typedef struct _RkAiqAlgoPreResAdrcV1Int {
    RkAiqAlgoPreResAdrc adrc_pre_res_com;
} RkAiqAlgoPreResAdrcV1Int;

typedef struct _RkAiqAlgoProcAdrcV1Int {
    RkAiqAlgoProcAdrc adrc_proc_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoProcAdrcV1Int;

typedef struct _RkAiqAlgoProcResAdrcV1Int {
    RkAiqAlgoProcResAdrc adrc_proc_res_com;
} RkAiqAlgoProcResAdrcV1Int;

typedef struct _RkAiqAlgoPostAdrcV1Int {
    RkAiqAlgoPostAdrc adrc_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAdrcV1Int;

typedef struct _RkAiqAlgoPostResAdrcV1Int {
    RkAiqAlgoPostResAdrc adrc_post_res_com;
} RkAiqAlgoPostResAdrcV1Int;


//sharp v3
typedef struct _RkAiqAlgoConfigAsharpV3Int {
    RkAiqAlgoConfigAsharp asharp_config_com;
    RkAiqAlgoComInt rk_com;
    Asharp_Config_V3_t stAsharpConfig;
} RkAiqAlgoConfigAsharpV3Int;

typedef struct _RkAiqAlgoPreAsharpV3Int {
    RkAiqAlgoPreAsharp asharp_pre_com;
    RkAiqAlgoComInt rk_com;
    Asharp_Config_V3_t stAsharpConfig;
} RkAiqAlgoPreAsharpV3Int;

typedef struct _RkAiqAlgoPreResAsharpV3Int {
    RkAiqAlgoPreResAsharp asharp_pre_res_com;
} RkAiqAlgoPreResAsharpV3Int;

typedef struct _RkAiqAlgoProcAsharpV3Int {
    RkAiqAlgoProcAsharp asharp_proc_com;
    RkAiqAlgoComInt rk_com;
    int iso;
    int hdr_mode;
} RkAiqAlgoProcAsharpV3Int;

typedef struct _RkAiqAlgoProcResAsharpV3Int {
    RkAiqAlgoProcResAsharp asharp_proc_res_com;
    Asharp_ProcResult_V3_t stAsharpProcResult;
} RkAiqAlgoProcResAsharpV3Int;

typedef struct _RkAiqAlgoPostAsharpV3Int {
    RkAiqAlgoPostAsharp asharp_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAsharpV3Int;

typedef struct _RkAiqAlgoPostResAsharpV3Int {
    RkAiqAlgoPostResAsharp asharp_post_res_com;
} RkAiqAlgoPostResAsharpV3Int;


//ynr v2
typedef struct _RkAiqAlgoConfigAynrV2Int {
    RkAiqAlgoConfigAynr aynr_config_com;
    RkAiqAlgoComInt rk_com;
    Aynr_Config_V2_t stAynrConfig;
} RkAiqAlgoConfigAynrV2Int;

typedef struct _RkAiqAlgoPreAynrV2Int {
    RkAiqAlgoPreAynr aynr_pre_com;
    RkAiqAlgoComInt rk_com;
    Aynr_Config_V2_t stAynrConfig;
} RkAiqAlgoPreAynrV2Int;

typedef struct _RkAiqAlgoPreResAynrV2Int {
    RkAiqAlgoPreResAynr aynr_pre_res_com;
} RkAiqAlgoPreResAynrV2Int;

typedef struct _RkAiqAlgoProcAynrV2Int {
    RkAiqAlgoProcAynr aynr_proc_com;
    RkAiqAlgoComInt rk_com;
    int iso;
    int hdr_mode;
} RkAiqAlgoProcAynrV2Int;

typedef struct _RkAiqAlgoProcResAynrV2Int {
    RkAiqAlgoProcResAynr aynr_proc_res_com;
    Aynr_ProcResult_V2_t stAynrProcResult;
} RkAiqAlgoProcResAynrV2Int;

typedef struct _RkAiqAlgoPostAynrV2Int {
    RkAiqAlgoPostAynr aynr_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAynrV2Int;

typedef struct _RkAiqAlgoPostResAynrV2Int {
    RkAiqAlgoPostResAynr aynr_post_res_com;
} RkAiqAlgoPostResAynrV2Int;

//cnr v1
typedef struct _RkAiqAlgoConfigAcnrV1Int {
    RkAiqAlgoConfigAcnr acnr_config_com;
    RkAiqAlgoComInt rk_com;
    Acnr_Config_V1_t stAcnrConfig;
} RkAiqAlgoConfigAcnrV1Int;

typedef struct _RkAiqAlgoPreAcnrV1Int {
    RkAiqAlgoPreAynr acnr_pre_com;
    RkAiqAlgoComInt rk_com;
    Acnr_Config_V1_t stAcnrConfig;
} RkAiqAlgoPreAcnrV1Int;

typedef struct _RkAiqAlgoPreResAcnrV1Int {
    RkAiqAlgoPreResAcnr acnr_pre_res_com;
} RkAiqAlgoPreResAcnrV1Int;

typedef struct _RkAiqAlgoProcAcnrV1Int {
    RkAiqAlgoProcAcnr acnr_proc_com;
    RkAiqAlgoComInt rk_com;
    int iso;
    int hdr_mode;
} RkAiqAlgoProcAcnrV1Int;

typedef struct _RkAiqAlgoProcResAcnrV1Int {
    RkAiqAlgoProcResAcnr acnr_proc_res_com;
    Acnr_ProcResult_V1_t stAcnrProcResult;
} RkAiqAlgoProcResAcnrV1Int;

typedef struct _RkAiqAlgoPostAcnrV1Int {
    RkAiqAlgoPostAcnr acnr_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAcnrV1Int;

typedef struct _RkAiqAlgoPostResAcnrV1Int {
    RkAiqAlgoPostResAcnr acnr_post_res_com;
} RkAiqAlgoPostResAcnrV1Int;

//abayernr v2
typedef struct _RkAiqAlgoConfigArawnrV2Int {
    RkAiqAlgoConfigArawnr arawnr_config_com;
    RkAiqAlgoComInt rk_com;
    Abayernr_Config_V2_t stArawnrConfig;
} RkAiqAlgoConfigArawnrV2Int;

typedef struct _RkAiqAlgoPreArawnrV2Int {
    RkAiqAlgoPreArawnr arawnr_pre_com;
    RkAiqAlgoComInt rk_com;
    Abayernr_Config_V2_t stArawnrConfig;
} RkAiqAlgoPreArawnrV2Int;

typedef struct _RkAiqAlgoPreResArawnrV2Int {
    RkAiqAlgoPreResArawnr arawnr_pre_res_com;
} RkAiqAlgoPreResArawnrV2Int;

typedef struct _RkAiqAlgoProcArawnrV2Int {
    RkAiqAlgoProcArawnr arawnr_proc_com;
    RkAiqAlgoComInt rk_com;
    int iso;
    int hdr_mode;
} RkAiqAlgoProcArawnrV2Int;

typedef struct _RkAiqAlgoProcResArawnrV2Int {
    RkAiqAlgoProcResArawnr arawnr_proc_res_com;
    Abayernr_ProcResult_V2_t stArawnrProcResult;
} RkAiqAlgoProcResArawnrV2Int;

typedef struct _RkAiqAlgoPostArawnrV2Int {
    RkAiqAlgoPostArawnr arawnr_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostArawnrV2Int;

typedef struct _RkAiqAlgoPostResArawnrV2Int {
    RkAiqAlgoPostResArawnr arawnr_post_res_com;
} RkAiqAlgoPostResArawnrV2Int;

// aeis
typedef struct _RkAiqAlgoConfigAeisInt {
    RkAiqAlgoConfigAeis common;
    RkAiqAlgoComInt rk_com;
    CalibDbV2_Eis_t calib;
    isp_drv_share_mem_ops_t *mem_ops;
} RkAiqAlgoConfigAeisInt;

typedef struct _RkAiqAlgoPreAeisInt {
    RkAiqAlgoPreAeis common;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPreAeisInt;

typedef struct _RkAiqAlgoPreResAeisInt {
    RkAiqAlgoPreResAeis common;
} RkAiqAlgoPreResAeisInt;

typedef struct _RkAiqAlgoProcAeisInt {
    RkAiqAlgoProcAeis common;
    RkAiqAlgoComInt rk_com;
    XCamVideoBuffer* orb_stats_buf;
    XCamVideoBuffer* nr_img_buf;
} RkAiqAlgoProcAeisInt;

typedef struct _RkAiqAlgoProcResAeisInt {
    RkAiqAlgoProcResAeis common;
    bool update;
    bool fec_en;
    int frame_id;
    int fd;
    int mesh_size;
    int mesh_density;
    int img_buf_index;
    unsigned int img_size;
} RkAiqAlgoProcResAeisInt;

typedef struct _RkAiqAlgoPostAeisInt {
    RkAiqAlgoPostAeis aeis_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAeisInt;

typedef struct _RkAiqAlgoPostResAeisInt {
    RkAiqAlgoPostResAeis aeis_post_res_com;
} RkAiqAlgoPostResAeisInt;

//amd
typedef struct _RkAiqAlgoConfigAmdInt {
    RkAiqAlgoConfigAmd amd_config_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoConfigAmdInt;

typedef struct _RkAiqAlgoPreAmdInt {
    RkAiqAlgoPreAmd amd_pre_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPreAmdInt;

typedef struct _RkAiqAlgoPreResAmdInt {
    RkAiqAlgoPreResAmd amd_pre_res_com;
} RkAiqAlgoPreResAmdInt;

typedef struct _RkAiqAlgoProcAmdInt {
    RkAiqAlgoProcAmd amd_proc_com;
    RkAiqAlgoComInt rk_com;
    int iso;
    int hdr_mode;
    rk_aiq_amd_algo_stat_t stats;
} RkAiqAlgoProcAmdInt;

typedef struct _RkAiqAlgoProcResAmdInt {
    RkAiqAlgoProcResAmd amd_proc_res_com;
} RkAiqAlgoProcResAmdInt;

typedef struct _RkAiqAlgoPostAmdInt {
    RkAiqAlgoPostAmd amd_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAmdInt;

typedef struct _RkAiqAlgoPostResAmdInt {
    RkAiqAlgoPostResAmd amd_post_res_com;
} RkAiqAlgoPostResAmdInt;

// rawnr v1
typedef struct _RkAiqAlgoConfigArawnrInt {
    RkAiqAlgoConfigArawnr arawnr_config_com;
    RkAiqAlgoComInt rk_com;
    Abayernr_Config_V1_t stArawnrConfig;
} RkAiqAlgoConfigArawnrInt;

typedef struct _RkAiqAlgoPreArawnrInt {
    RkAiqAlgoPreArawnr arawnr_pre_com;
    RkAiqAlgoComInt rk_com;
    Abayernr_Config_V1_t stArawnrConfig;
} RkAiqAlgoPreArawnrInt;

typedef struct _RkAiqAlgoPreResArawnrInt {
    RkAiqAlgoPreResArawnr arawnr_pre_res_com;
} RkAiqAlgoPreResArawnrInt;

typedef struct _RkAiqAlgoProcArawnrInt {
    RkAiqAlgoProcArawnr arawnr_proc_com;
    RkAiqAlgoComInt rk_com;
    int iso;
    int hdr_mode;
} RkAiqAlgoProcArawnrInt;

typedef struct _RkAiqAlgoProcResArawnrInt {
    RkAiqAlgoProcResArawnr arawnr_proc_res_com;
    Abayernr_ProcResult_V1_t stArawnrProcResult;
} RkAiqAlgoProcResArawnrInt;

typedef struct _RkAiqAlgoPostArawnrInt {
    RkAiqAlgoPostArawnr arawnr_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostArawnrInt;

typedef struct _RkAiqAlgoPostResArawnrInt {
    RkAiqAlgoPostResArawnr arawnr_post_res_com;
} RkAiqAlgoPostResArawnrInt;

//ynr v1
typedef struct _RkAiqAlgoConfigAynrInt {
    RkAiqAlgoConfigAynr aynr_config_com;
    RkAiqAlgoComInt rk_com;
    Aynr_Config_V1_t stAynrConfig;
} RkAiqAlgoConfigAynrInt;

typedef struct _RkAiqAlgoPreAynrInt {
    RkAiqAlgoPreAynr aynr_pre_com;
    RkAiqAlgoComInt rk_com;
    Aynr_Config_V1_t stAynrConfig;
} RkAiqAlgoPreAynrInt;

typedef struct _RkAiqAlgoPreResAynrInt {
    RkAiqAlgoPreResAynr aynr_pre_res_com;
} RkAiqAlgoPreResAynrInt;

typedef struct _RkAiqAlgoProcAynrInt {
    RkAiqAlgoProcAynr aynr_proc_com;
    RkAiqAlgoComInt rk_com;
    int iso;
    int hdr_mode;
} RkAiqAlgoProcAynrInt;

typedef struct _RkAiqAlgoProcResAynrInt {
    RkAiqAlgoProcResAynr aynr_proc_res_com;
    Aynr_ProcResult_V1_t stAynrProcResult;
} RkAiqAlgoProcResAynrInt;

typedef struct _RkAiqAlgoPostAynrInt {
    RkAiqAlgoPostAynr aynr_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAynrInt;

typedef struct _RkAiqAlgoPostResAynrInt {
    RkAiqAlgoPostResAynr aynr_post_res_com;
} RkAiqAlgoPostResAynrInt;

// auvnr
typedef struct _RkAiqAlgoConfigAcnrInt {
    RkAiqAlgoConfigAcnr auvnr_config_com;
    RkAiqAlgoComInt rk_com;
    Auvnr_Config_V1_t stAuvnrConfig;
} RkAiqAlgoConfigAcnrInt;

typedef struct _RkAiqAlgoPreAcnrInt {
    RkAiqAlgoPreAcnr auvnr_pre_com;
    RkAiqAlgoComInt rk_com;
    Auvnr_Config_V1_t stAuvnrConfig;
} RkAiqAlgoPreAcnrInt;

typedef struct _RkAiqAlgoPreResAcnrInt {
    RkAiqAlgoPreResAcnr auvnr_pre_res_com;
} RkAiqAlgoPreResAcnrInt;

typedef struct _RkAiqAlgoProcAcnrInt {
    RkAiqAlgoProcAcnr auvnr_proc_com;
    RkAiqAlgoComInt rk_com;
    int iso;
    int hdr_mode;
} RkAiqAlgoProcAcnrInt;

typedef struct _RkAiqAlgoProcResAcnrInt {
    RkAiqAlgoProcResAcnr auvnr_proc_res_com;
    Auvnr_ProcResult_V1_t stAuvnrProcResult;
} RkAiqAlgoProcResAcnrInt;

typedef struct _RkAiqAlgoPostAcnrInt {
    RkAiqAlgoPostAcnr auvnr_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAcnrInt;

typedef struct _RkAiqAlgoPostResAcnrInt {
    RkAiqAlgoPostResAcnr auvnr_post_res_com;
} RkAiqAlgoPostResAcnrInt;


//mfnr
typedef struct _RkAiqAlgoConfigAmfnrInt {
    RkAiqAlgoConfigAmfnr amfnr_config_com;
    RkAiqAlgoComInt rk_com;
    Amfnr_Config_V1_t stAmfnrConfig;
} RkAiqAlgoConfigAmfnrInt;

typedef struct _RkAiqAlgoPreAmfnrInt {
    RkAiqAlgoPreAmfnr amfnr_pre_com;
    RkAiqAlgoComInt rk_com;
    Amfnr_Config_V1_t stAmfnrConfig;
} RkAiqAlgoPreAmfnrInt;

typedef struct _RkAiqAlgoPreResAmfnrInt {
    RkAiqAlgoPreResAmfnr amfnr_pre_res_com;
} RkAiqAlgoPreResAmfnrInt;

typedef struct _RkAiqAlgoProcAmfnrInt {
    RkAiqAlgoProcAmfnr amfnr_proc_com;
    RkAiqAlgoComInt rk_com;
    int iso;
    int hdr_mode;
} RkAiqAlgoProcAmfnrInt;

typedef struct _RkAiqAlgoProcResAmfnrInt {
    RkAiqAlgoProcResAmfnr amfnr_proc_res_com;
    Amfnr_ProcResult_V1_t stAmfnrProcResult;
} RkAiqAlgoProcResAmfnrInt;

typedef struct _RkAiqAlgoPostAmfnrInt {
    RkAiqAlgoPostAmfnr amfnr_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAmfnrInt;

typedef struct _RkAiqAlgoPostResAmfnrInt {
    RkAiqAlgoPostResAmfnr amfnr_post_res_com;
} RkAiqAlgoPostResAmfnrInt;


// again
typedef struct _RkAiqAlgoConfigAgainInt {
    RkAiqAlgoConfigAgain again_config_com;
    RkAiqAlgoComInt rk_com;
    Again_Config_V1_t stAgainConfig;
} RkAiqAlgoConfigAgainInt;

typedef struct _RkAiqAlgoPreAgainInt {
    RkAiqAlgoPreAgain again_pre_com;
    RkAiqAlgoComInt rk_com;
    Again_Config_V1_t stAgainConfig;
} RkAiqAlgoPreAgainInt;

typedef struct _RkAiqAlgoPreResAgainInt {
    RkAiqAlgoPreResAgain again_pre_res_com;
} RkAiqAlgoPreResAgainInt;

typedef struct _RkAiqAlgoProcAgainInt {
    RkAiqAlgoProcAcnr again_proc_com;
    RkAiqAlgoComInt rk_com;
    int iso;
    int hdr_mode;
} RkAiqAlgoProcAgainInt;

typedef struct _RkAiqAlgoProcResAgainInt {
    RkAiqAlgoProcResAgain again_proc_res_com;
    Again_ProcResult_V1_t stAgainProcResult;
} RkAiqAlgoProcResAgainInt;

typedef struct _RkAiqAlgoPostAgainInt {
    RkAiqAlgoPostAgain auvnr_post_com;
    RkAiqAlgoComInt rk_com;
} RkAiqAlgoPostAgainInt;

typedef struct _RkAiqAlgoPostResAgainInt {
    RkAiqAlgoPostResAgain auvnr_post_res_com;
} RkAiqAlgoPostResAgainInt;


#endif
