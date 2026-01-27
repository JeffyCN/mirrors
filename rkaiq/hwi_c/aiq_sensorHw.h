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

#ifndef _AIQ_SENSOR_HW_BASE_C_H_
#define _AIQ_SENSOR_HW_BASE_C_H_

#include "aiq_list.h"
#include "aiq_map.h"
#include "aiq_pool.h"
#include "aiq_v4l2_device.h"
#include "common/rk-camera-module.h"
#include "hwi_c/aiq_camHw.h"
#include "iq_parser_v2/cis_head.h"
#include "rk_aiq.h"
#include "rk_aiq_module_common.h"
#include "rk_aiq_offline_raw.h"
#include "rk_aiq_types_priv_c.h"

#if RKAIQ_HAVE_DUMPSYS
#include "dumpcam_server/info/include/st_string.h"
#endif

/************ BELOW FROM kernel/include/uapi/linux/rk-preisp.h ************/

/* test hdr function */
/*
 * struct hdrae_exp_s - hdrae exposure
 *
 */

enum cg_mode_e {
    GAIN_MODE_LCG,
    GAIN_MODE_HCG,
};

struct hdrae_exp_s {
    unsigned int long_exp_reg;
    unsigned int long_gain_reg;
    unsigned int middle_exp_reg;
    unsigned int middle_gain_reg;
    unsigned int short_exp_reg;
    unsigned int short_gain_reg;
    unsigned int long_exp_val;
    unsigned int long_gain_val;
    unsigned int middle_exp_val;
    unsigned int middle_gain_val;
    unsigned int short_exp_val;
    unsigned int short_gain_val;
    unsigned char long_cg_mode;
    unsigned char middle_cg_mode;
    unsigned char short_cg_mode;
};

#define SENSOR_CMD_SET_HDRAE_EXP _IOW('V', BASE_VIDIOC_PRIVATE + 0, struct hdrae_exp_s)

/************ UPPER FROM kernel/include/uapi/linux/rk-preisp.h ************/

RKAIQ_BEGIN_DECLARE

#define SENSOR_SUBM (0x4)

enum Aiq_EXP_UPDATE_TYPE_e {
    RK_EXP_UPDATE_TIME,
    RK_EXP_UPDATE_GAIN,
    RK_EXP_UPDATE_DCG,
};

typedef struct aiq_pending_split_exps_s {
    bool is_rk_exp_res;
    union {
        struct {
            uint16_t line_length_pixels;
            uint32_t frame_length_lines;
            float pixel_clock_freq_mhz;
            int dcg_mode[3];
            RkAiqExpSensorParam_t sensor_params[3];
            uint32_t update_bits;
        } rk_exp_res;

        struct {
            unsigned int nNumRegs;
            unsigned int RegAddr[MAX_I2CDATA_LEN];
            unsigned int RegValue[MAX_I2CDATA_LEN];
            unsigned int AddrByteNum[MAX_I2CDATA_LEN];
            unsigned int ValueByteNum[MAX_I2CDATA_LEN];
        } i2c_exp_res;
    };
} aiq_pending_split_exps_t;

typedef struct SensorHwExpItem_s {
    AiqSensorExpInfo_t* _pSnsExp;
    bool _isFirst;
} SensorHwExpItem_t;

typedef struct AiqSensorHw_s AiqSensorHw_t;
struct AiqSensorHw_s {
    AiqV4l2SubDevice_t* mSd;
    rk_aiq_exposure_sensor_descriptor _sensor_desc;
    AiqMutex_t _mutex;
    /* AiqSensorExpInfo_t* */
    AiqList_t* _exp_list;
    /* AiqSensorExpInfo_t* */
    AiqMap_t* _effecting_exp_map;
    /* AiqSensorExpInfo_t* */
    AiqList_t* _delayed_gain_list;
    /* AiqSensorExpInfo_t* */
    AiqList_t* _delayed_dcg_gain_mode_list;
    /* AiqSensorExpInfo_t* */
    AiqPool_t* _expParamsPool;
    /* AiqSensorExpInfo_t* */
    AiqMap_t* _pending_spilt_map;
    AiqSensorExpInfo_t* _last_exp_time;
    AiqSensorExpInfo_t* _last_exp_gain;
    AiqSensorExpInfo_t* _last_dcg_gain_mode;
    int _working_mode;
    bool _first;
    uint32_t _frame_sequence;
    int _gain_delay;
    int _time_delay;
    bool _gain_delayed;
    int _dcg_gain_mode_delay;
    bool _dcg_gain_mode_delayed;
    bool _dcg_gain_mode_with_time;
    bool _is_i2c_exp;
    const char* _sns_entity_name;
    bool _mirror;
    bool _flip;
    bool _update_mirror_flip;
    bool mPauseFlag;
    uint32_t mPauseId;
    bool mIsSingleMode;
    int mCamPhyId;
    int32_t dcg_mode;
    AiqCamHwBase_t* _mCamHw;
#if RKAIQ_HAVE_DUMPSYS
    // dump info
    rk_aiq_exposure_sensor_descriptor desc;
#endif

    uint32_t userVts;

    calibdb_cis_blc_t cis_blc;
    int32_t mCisHdrMode;

    int mVicapExpFd;
    // export api
    XCamReturn (*setExposureParams)(AiqSensorHw_t* pBaseSns, AiqAecExpInfoWrapper_t* expPar);
    XCamReturn (*getSensorModeData)(AiqSensorHw_t* pBaseSns, const char* sns_ent_name,
                                    rk_aiq_exposure_sensor_descriptor* sns_des);
    XCamReturn (*handle_sof)(AiqSensorHw_t* pBaseSns, int64_t time, uint32_t frameid);
    XCamReturn (*get_sensor_descriptor)(AiqSensorHw_t* pBaseSns,
                                        rk_aiq_exposure_sensor_descriptor* sns_des);
    AiqSensorExpInfo_t* (*getEffectiveExpParams)(AiqSensorHw_t* pBaseSns, uint32_t frame_id);
#if RKAIQ_HAVE_DUMPSYS
    int (*dump)(void* dumper, st_string* result, int argc, void* argv[]);
#endif
    // override v4l2
    XCamReturn (*open)(AiqSensorHw_t* pBaseSns);
    XCamReturn (*start)(AiqSensorHw_t* pBaseSns, bool prepared);
    XCamReturn (*stop)(AiqSensorHw_t* pBaseSns);
    XCamReturn (*close)(AiqSensorHw_t* pBaseSns);
    // internal api for sub class
    XCamReturn (*set_working_mode)(AiqSensorHw_t* pBaseSns, int mode);
    XCamReturn (*set_exp_delay_info)(AiqSensorHw_t* pBaseSns, int time_delay, int gain_delay,
                                     int hcg_lcg_mode_delay);
    XCamReturn (*set_mirror_flip)(AiqSensorHw_t* pBaseSns, bool mirror, bool flip,
                                  int32_t* skip_frame_sequence);
    XCamReturn (*get_mirror_flip)(AiqSensorHw_t* pBaseSns, bool* mirror, bool* flip);
    XCamReturn (*set_sync_mode)(AiqSensorHw_t* pBaseSns, uint32_t mode);
    bool (*get_is_single_mode)(AiqSensorHw_t* pSnsHw);
    XCamReturn (*set_effecting_exp_map)(AiqSensorHw_t* pSnsHw, uint32_t sequence, void* exp_ptr,
                                        int mode);
    XCamReturn (*set_pause_flag)(AiqSensorHw_t* pSnsHw, bool mode, uint32_t frameId,
                                 bool isSingleMode);
    XCamReturn (*set_exposure_mode)(AiqSensorHw_t* pBaseSns, uint32_t mode);

    // set scene_cis parameters
    XCamReturn (*set_regSetting)(AiqSensorHw_t* pBaseSns, calibdb_cis_reg_setting_t* regSetting);
    XCamReturn (*set_blc)(AiqSensorHw_t* pBaseSns, calibdb_cis_blc_t* blc);
    XCamReturn (*set_hdr)(AiqSensorHw_t* pBaseSns, calibdb_cis_hdr_t* hdr);
    XCamReturn (*set_qbcRmsc)(AiqSensorHw_t* pBaseSns, calibdb_cis_qbc_rmsc_t* qbcRmsc);
    XCamReturn (*set_cmpsOut)(AiqSensorHw_t* pBaseSns, calibdb_cis_cmps_out_t* cmpsOut);

    void (*setVicapExpFd)(AiqSensorHw_t* pSnsHw, int fd);
};

#define AiqSensorHw_setCamHw(pSnsHw, pCamHw) \
    pSnsHw->_mCamHw = pCamHw

void AiqSensorHw_init(AiqSensorHw_t* pSnsHw, const char* name, int cid);
void AiqSensorHw_deinit(AiqSensorHw_t* pSnsHw);

XCamReturn _SensorHw_stop(AiqSensorHw_t* pSnsHw);
void AiqSensorHw_clean(AiqSensorHw_t* pSnsHw);
XCamReturn AiqSensorHw_getHdrComprCurve(AiqSensorHw_t* pSnsHw, RkAiqHdrCompr_t* compr);
int AiqSensorHw_getMergeWgtCurve(AiqSensorHw_t* pSnsHw, struct rkmodule_mge_oeWgt* mge_oe_wgt);
int AiqSensorHwsendMergeWgtCurve2NR(struct rkmodule_mge_oeWgt* mge_oe_wgt,
                                    mergeLuma2Wgt_t* pMergeLuma2Wgt);
int AiqSensorHw_getBuiltInHdrSingleBit(
    AiqSensorHw_t* pSnsHw, struct rkmodule_hdr_compr_single_frame_info* compr_single_info);

#define AiqSensorHw_setUserVts(pSnsHw, vts) \
    (pSnsHw)->userVts = vts

#define AiqSensorHw_getUserVts(pSnsHw) \
    pSnsHw->userVts

RKAIQ_END_DECLARE

#endif
