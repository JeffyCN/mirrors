/*
 * cis_head.h
 *
 *  Copyright (c) 2025 Rockchip Corporation
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

#ifndef __CALIBDBV2_CIS_HEADER_H__
#define __CALIBDBV2_CIS_HEADER_H__

#include <stdbool.h>
#include <stdint.h>

#include "iq_parser_v2/sensorinfo_head.h"

/**
 * Register control structure
 *
 * @regAddr: Register address
 * @regVal: Register value
 */
typedef struct {
    uint32_t regAddr;
    uint32_t regVal;
} calibdb_cis_reg_ctrl_t;

/**
 * DS mode structure
 *
 * @enum calibdb_cis_ds_mode_t: DS mode enumeration
 * Naming rule: CIS_<mode>_MODE
 * - mode: The specific mode of the CIS, e.g., bayerBinning2X2, bayerSkipN, qbcBinninG2X2
 * - cis_bayerBinning2X2_mode: Binning 2x2 mode for Bayer sensors
 * - cis_bayerSkipN_mode: Skip mode for Bayer sensors, where n indicates the number of skipped lines
 * - cis_qbcBinning2X2_mode: Quad Bayer Color Binning 2x2 mode
 */
typedef enum {
    cis_bayerBinning2X2_mode,
    cis_bayerSkipN_mode,
    cis_qbcBinning2X2_mode,
} calibdb_cis_ds_mode_t;

/**
 * Register setting structure
 *
 * @chksum: Checksum for driver to select setting, 0 means driver control
 * @ds_mode: Notify driver to adapt corresponding setting based on checksum
 * @regCtrl_num: Number of valid register configurations in regCtrl array (0-16)
 * @regCtrl: Support 16 register configurations for CIS specific mode extension control
 */
typedef struct {
    uint32_t hw_cisCfg_setting_id;
    calibdb_cis_ds_mode_t hw_cisCfg_ds_mode;
    uint32_t regCtrl_num;
    calibdb_cis_reg_ctrl_t regCtrl[16];
} calibdb_cis_reg_setting_t;

/**
 * Black level digital gain structure
 *
 * @ hw_cisCfg_bkdgSw_en: Enable switch
 * @hw_cisCfg_dgBk2BkDg_thred: dgbk -> bkdg gain threshold
 * @hw_cisCfg_bkDg2DgBk_thred: bkdg -> dgbk gain threshold
 */
typedef struct {
    bool hw_cisCfg_bkdgSw_en;
    uint32_t hw_cisCfg_dgBk2BkDg_thred;
    uint32_t hw_cisCfg_bkDg2DgBk_thred;
} calibdb_cis_bkDg_t;

/**
 * BLC static structure
 *
 * @blklevel: CIS 12bit black level clamp offset value
 * @bkdg: CIS internal BLC first, then dgain, avoid black level offset caused by excessive digital gain
 * @regCtrl_num: Number of valid register configurations in regCtrl array (0-8)
 * @regCtrl: Support 16 register configurations for CIS specific mode extension control
 */
typedef struct {
    uint32_t hw_cisCfg_blkLevel_val;
    calibdb_cis_bkDg_t bkDg;
    uint32_t regCtrl_num;
    calibdb_cis_reg_ctrl_t regCtrl[8];
} calibdb_cis_blc_sta_t;

/**
 * BLC structure
 *
 * @en: Enable
 * @sta: Static parameters
 *
 * Note: dyn is empty, not defined for now
 */
typedef struct {
    bool en;
    calibdb_cis_blc_sta_t sta;
} calibdb_cis_blc_t;

/*
 * HDR static structure
 *
 * @hw_cisCfg_hdrMge_mode: HDR merge mode
 * @hw_cisCfg_cisHdr_mode: HDR mode type
 * @regCtrl_num: Number of valid register configurations in regCtrl array (0-8)
 * @regCtrl: Support 8 register configurations for QBC remosaic specific mode extension control
 */
typedef struct {
    rk_aiq_isp_hdr_mode_t hw_cisCfg_hdrMge_mode;
    rk_aiq_sensor_hdr_line_mode_t hw_cisCfg_cisHdr_mode;
    uint32_t regCtrl_num;
    calibdb_cis_reg_ctrl_t regCtrl[8];
} calibdb_cis_hdr_sta_t;

/**
 * HDR structure
 *
 * @en: Off indicates linear mode
 * @sta: Static parameters
 *
 * Note: dyn is empty, not defined for now
 */
typedef struct {
    calibdb_cis_hdr_sta_t sta;
} calibdb_cis_hdr_t;

/**
 * QBC remosaic static structure
 *
 * @regCtrl_num: Number of valid register configurations in regCtrl array (0-8)
 * @regCtrl: Support 8 register configurations for QBC remosaic specific mode extension control
 */
typedef struct {
    uint32_t regCtrl_num;
    calibdb_cis_reg_ctrl_t regCtrl[8];
} calibdb_cis_qbc_rmsc_sta_t;

/**
 * QBC remosaic structure
 *
 * @en: QBC CIS remosaic enable, internally completes quad bayer code to bayer raw conversion
 * @sta: Static parameters
 */
typedef struct {
    bool en;
    calibdb_cis_qbc_rmsc_sta_t sta;
} calibdb_cis_qbc_rmsc_t;

/**
 * Compression mode enumeration
 *
 * @cis_cmpsLowBW_mode: Select low bit width compression curve in driver
 * @cis_cmpsHighBW_mode: Select high bit width compression curve in driver
 */
typedef enum {
    cis_cmpsLowBW_mode,
    cis_cmpsHighBW_mode
} calibdb_cis_cmps_mode_e;

/**
 * Compression output status structure
 *
 * @cmps_mode: Compression mode
 * @cmpsCurve: CIS compression curve
 */
typedef struct {
    calibdb_cis_cmps_mode_e hw_cisCfgCmps_mode;
} calibdb_cis_cmps_out_sta_t;

/**
 * Compression output structure
 *
 * @en: Enable
 * @sta: Static parameters
 */
typedef struct {
    bool en;
    calibdb_cis_cmps_out_sta_t sta;
} calibdb_cis_cmps_out_t;

/**
 * CIS configuration main structure
 *
 * The following parameters control different CIS modules. When a corresponding node
 * is missing in the configuration, AIQ does not control that module, which is
 * equivalent to full driver control.
 *
 * @regSetting: Register settings, no regSetting node means AIQ does not control
 * @blc: Black level correction, no blc node means AIQ does not control
 * @hdr: HDR configuration, no hdr node means AIQ does not control
 * @qbcRmsc: QBC remosaic, no qbcRmc node means AIQ does not control
 * @cmpsOut: Compression output, no outputCmps node means AIQ does not control
 */
typedef struct {
    calibdb_cis_reg_setting_t cis_regSetting;
    calibdb_cis_blc_t cis_blc;
    calibdb_cis_hdr_t cis_hdr;
    calibdb_cis_qbc_rmsc_t cis_qbcRmsc;
    calibdb_cis_cmps_out_t cis_cmpsOut;
} calibdb_cis_para_t;

#endif  // __CALIBDBV2_CIS_HEADER_H__