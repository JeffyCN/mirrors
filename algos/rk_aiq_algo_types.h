/*
 * rk_aiq_algo_types.h
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

#ifndef _RK_AIQ_ALGO_TYPES_H_
#define _RK_AIQ_ALGO_TYPES_H_


#include "rk_aiq_algo_des.h"
#include "rk_aiq_types.h"
#include <video_buffer.h>

// Ae
typedef struct _RkAiqAlgoConfigAe {
    RkAiqAlgoCom com;
    /*params related to Hardware setting*/
    RkAiqAecHwConfig_t HwCtrl;
    /*params related to driver setting*/
    float        LinePeriodsPerField;
    float        PixelClockFreqMHZ;
    float        PixelPeriodsPerLine;
} RkAiqAlgoConfigAe;

typedef struct _RkAiqAlgoPreAe {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAe;

typedef struct _RkAiqAlgoPreResAe {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAe;

typedef struct _RkAiqAlgoProcAe {
    RkAiqAlgoCom com;
    XCamVideoBuffer* aecStatsBuf;
} RkAiqAlgoProcAe;

typedef struct _RkAiqAlgoProcResAe {
    RkAiqAlgoResCom proc_res_com;
    rk_aiq_exposure_params_t cur_ae_exp;
    rk_aiq_exposure_params_t new_ae_exp;
    rk_aiq_isp_aec_meas_t ae_meas;
    rk_aiq_isp_hist_meas_t hist_meas;
} RkAiqAlgoProcResAe;

typedef struct _RkAiqAlgoPostAe {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAe;

typedef struct _RkAiqAlgoPostResAe {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAe;

//Awb
typedef struct _RkAiqAlgoConfigAwb {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAwb;

typedef struct _RkAiqAlgoPreAwb {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAwb;

typedef struct _RkAiqAlgoPreResAwb {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAwb;

typedef struct _RkAiqAlgoProcAwb {
    RkAiqAlgoCom com;
    union {
        rk_aiq_awb_stat_res_v200_t awb_hw0_statis;
        rk_aiq_awb_stat_res_v201_t awb_hw1_statis;
    };
} RkAiqAlgoProcAwb;

typedef struct _RkAiqAlgoProcResAwb {
    RkAiqAlgoResCom proc_res_com;
    bool awb_cfg_update;
    union {
        rk_aiq_awb_stat_cfg_v200_t awb_hw0_para;
        rk_aiq_awb_stat_cfg_v201_t awb_hw1_para;
    };
    bool awb_gain_update;
    rk_aiq_wb_gain_t awb_gain_algo;
} RkAiqAlgoProcResAwb;

typedef struct _RkAiqAlgoPostAwb {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAwb;

typedef struct _RkAiqAlgoPostResAwb {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAwb;

// af
typedef struct _RkAiqAlgoConfigAf {
    RkAiqAlgoCom af_com;
    unsigned char af_mode;
    unsigned short win_h_offs;
    unsigned short win_v_offs;
    unsigned short win_h_size;
    unsigned short win_v_size;
    rk_aiq_lens_descriptor lens_des;
} RkAiqAlgoConfigAf;

typedef struct _RkAiqAlgoPreAf {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAf;

typedef struct _RkAiqAlgoPreResAf {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAf;

typedef struct _RkAiqAlgoProcAf {
    RkAiqAlgoCom com;
    XCamVideoBuffer *xcam_af_stats;
    XCamVideoBuffer *xcam_aec_stats;
} RkAiqAlgoProcAf;

typedef struct _RkAiqAlgoProcResAf {
    bool af_cfg_update;
    bool af_focus_update;
    RkAiqAlgoResCom proc_res_com;
    rk_aiq_isp_af_meas_t af_isp_param;
    rk_aiq_af_focus_pos_meas_t af_focus_param;
} RkAiqAlgoProcResAf;

typedef struct _RkAiqAlgoPostAf {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAf;

typedef struct _RkAiqAlgoPostResAf {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAf;

// anr
typedef struct _RkAiqAlgoConfigAnr {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAnr ;

typedef struct _RkAiqAlgoPreAnr {
    RkAiqAlgoCom anr_com;
} RkAiqAlgoPreAnr;

typedef struct _RkAiqAlgoPreResAnr {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAnr;

typedef struct _RkAiqAlgoProcAnr {
    RkAiqAlgoCom anr_com;
} RkAiqAlgoProcAnr;

typedef struct _RkAiqAlgoProcResAnr {
    RkAiqAlgoResCom proc_res_com;
} RkAiqAlgoProcResAnr;

typedef struct _RkAiqAlgoPostAnr {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAnr;

typedef struct _RkAiqAlgoPostResAnr {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAnr;

// asharp
typedef struct _RkAiqAlgoConfigAsharp {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAsharp ;

typedef struct _RkAiqAlgoPreAsharp {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAsharp;

typedef struct _RkAiqAlgoPreResAsharp {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAsharp;

typedef struct _RkAiqAlgoProcAsharp {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAsharp;

typedef struct _RkAiqAlgoProcResAsharp {
    RkAiqAlgoResCom proc_res_com;
} RkAiqAlgoProcResAsharp;

typedef struct _RkAiqAlgoPostAsharp {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAsharp;

typedef struct _RkAiqAlgoPostResAsharp {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAsharp;

// asd
typedef struct _RkAiqAlgoConfigAsd {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAsd ;

typedef struct _RkAiqAlgoPreAsd {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAsd;

typedef struct _RkAiqAlgoPreResAsd {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAsd;

typedef struct _RkAiqAlgoProcAsd {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAsd;

typedef struct _RkAiqAlgoProcResAsd {
    RkAiqAlgoResCom proc_res_com;
} RkAiqAlgoProcResAsd;

typedef struct _RkAiqAlgoPostAsd {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAsd;

typedef struct _RkAiqAlgoPostResAsd {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAsd;

// amerge
typedef struct _RkAiqAlgoConfigAmerge {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAmerge;

typedef struct _RkAiqAlgoPreAmerge {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAmerge;

typedef struct _RkAiqAlgoPreResAmerge {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAmerge;

typedef struct _RkAiqAlgoProcAmerge {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAmerge;

typedef struct _RkAiqAlgoProcResAmerge {
    RkAiqAlgoResCom proc_res_com;
} RkAiqAlgoProcResAmerge;

typedef struct _RkAiqAlgoPostAmerge {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAmerge;

typedef struct _RkAiqAlgoPostResAmerge {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAmerge;

// atmo
typedef struct _RkAiqAlgoConfigAtmo {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAtmo;

typedef struct _RkAiqAlgoPreAtmo {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAtmo;

typedef struct _RkAiqAlgoPreResAtmo {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAtmo;

typedef struct _RkAiqAlgoProcAtmo {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAtmo;

typedef struct _RkAiqAlgoProcResAtmo {
    RkAiqAlgoResCom proc_res_com;
} RkAiqAlgoProcResAtmo;

typedef struct _RkAiqAlgoPostAtmo {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAtmo;

typedef struct _RkAiqAlgoPostResAtmo {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAtmo;


// acp
typedef struct _RkAiqAlgoConfigAcp {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAcp;

typedef struct _RkAiqAlgoPreAcp {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAcp;

typedef struct _RkAiqAlgoPreResAcp {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAcp;

typedef struct _RkAiqAlgoProcAcp {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAcp;

typedef struct _RkAiqAlgoProcResAcp {
    RkAiqAlgoResCom proc_res_com;
    rk_aiq_acp_params_t acp_res;
} RkAiqAlgoProcResAcp;

typedef struct _RkAiqAlgoPostAcp {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAcp;

typedef struct _RkAiqAlgoPostResAcp {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAcp;

//adehaze
typedef struct _RkAiqAlgoConfigAdhaz {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAdhaz;

typedef struct _RkAiqAlgoPreAdhaz {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAdhaz;

typedef struct _RkAiqAlgoPreResAdhaz {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAdhaz;

typedef struct _RkAiqAlgoProcAdhaz {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAdhaz;

typedef struct _RkAiqAlgoProcResAdhaz {
    RkAiqAlgoResCom proc_res_com;
    RkAiqAdehazeProcResult_t AdehzeProcRes;
} RkAiqAlgoProcResAdhaz;

typedef struct _RkAiqAlgoPostAdhaz {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAdhaz;

typedef struct _RkAiqAlgoPostResAdhaz {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAdhaz;

// a3dlut
typedef struct _RkAiqAlgoConfigA3dlut {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigA3dlut;

typedef struct _RkAiqAlgoPreA3dlut {
    RkAiqAlgoCom com;
} RkAiqAlgoPreA3dlut;

typedef struct _RkAiqAlgoPreResA3dlut {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResA3dlut;

typedef struct _RkAiqAlgoProcA3dlut {
    RkAiqAlgoCom com;
} RkAiqAlgoProcA3dlut;

typedef struct _RkAiqAlgoProcResA3dlut {
    RkAiqAlgoResCom proc_res_com;
    rk_aiq_lut3d_cfg_t lut3d_hw_conf;
    bool lut3d_update;
} RkAiqAlgoProcResA3dlut;

typedef struct _RkAiqAlgoPostA3dlut {
    RkAiqAlgoCom com;
} RkAiqAlgoPostA3dlut;

typedef struct _RkAiqAlgoPostResA3dlut {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResA3dlut;

// ablc
typedef struct _RkAiqAlgoConfigAblc {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAblc;

typedef struct _RkAiqAlgoPreAblc {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAblc;

typedef struct _RkAiqAlgoPreResAblc {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAblc;

typedef struct _RkAiqAlgoProcAblc {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAblc;

typedef struct _RkAiqAlgoProcResAblc {
    RkAiqAlgoResCom proc_res_com;
} RkAiqAlgoProcResAblc;

typedef struct _RkAiqAlgoPostAblc {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAblc;

typedef struct _RkAiqAlgoPostResAblc {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAblc;

// accm
typedef struct _RkAiqAlgoConfigAccm {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAccm;

typedef struct _RkAiqAlgoPreAccm {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAccm;

typedef struct _RkAiqAlgoPreResAccm {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAccm;

typedef struct _RkAiqAlgoProcAccm {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAccm;

typedef struct _RkAiqAlgoProcResAccm {
    RkAiqAlgoResCom proc_res_com;
    rk_aiq_ccm_cfg_t accm_hw_conf;
    bool ccm_update;
} RkAiqAlgoProcResAccm;

typedef struct _RkAiqAlgoPostAccm {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAccm;

typedef struct _RkAiqAlgoPostResAccm {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAccm;

// cgc
typedef struct _RkAiqAlgoConfigAcgc {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAcgc;

typedef struct _RkAiqAlgoPreAcgc {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAcgc;


typedef struct _RkAiqAlgoPreResAcgc {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAcgc;

typedef struct _RkAiqAlgoProcAcgc {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAcgc;

typedef struct _RkAiqAlgoProcResAcgc {
    RkAiqAlgoResCom proc_res_com;
} RkAiqAlgoProcResAcgc;

typedef struct _RkAiqAlgoPostAcgc {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAcgc;

typedef struct _RkAiqAlgoPostResAcgc {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAcgc;

// adebayer
typedef struct _RkAiqAlgoConfigAdebayer {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAdebayer;

typedef struct _RkAiqAlgoPreAdebayer {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAdebayer;

typedef struct _RkAiqAlgoPreResAdebayer {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAdebayer;

typedef struct _RkAiqAlgoProcAdebayer {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAdebayer;

typedef struct _RkAiqAlgoProcResAdebayer {
    RkAiqAlgoResCom proc_res_com;
} RkAiqAlgoProcResAdebayer;

typedef struct _RkAiqAlgoPostAdebayer {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAdebayer;

typedef struct _RkAiqAlgoPostResAdebayer {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAdebayer;

// adpcc
typedef struct _RkAiqAlgoConfigAdpcc {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAdpcc;

typedef struct _RkAiqAlgoPreAdpcc {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAdpcc;

typedef struct _RkAiqAlgoPreResAdpcc {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAdpcc;

typedef struct _RkAiqAlgoProcAdpcc {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAdpcc;

typedef struct _RkAiqAlgoProcResAdpcc {
    RkAiqAlgoResCom proc_res_com;
    Sensor_dpcc_res_t SenDpccRes;
} RkAiqAlgoProcResAdpcc;

typedef struct _RkAiqAlgoPostAdpcc {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAdpcc;

typedef struct _RkAiqAlgoPostResAdpcc {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAdpcc;

// afec
typedef struct _RkAiqAlgoConfigAfec {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAfec;

typedef struct _RkAiqAlgoPreAfec {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAfec;

typedef struct _RkAiqAlgoPreResAfec {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAfec;

typedef struct _RkAiqAlgoProcAfec {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAfec;

typedef struct _RkAiqAlgoProcResAfec {
    RkAiqAlgoResCom proc_res_com;
} RkAiqAlgoProcResAfec;

typedef struct _RkAiqAlgoPostAfec {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAfec;

typedef struct _RkAiqAlgoPostResAfec {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAfec;

// agamma
typedef struct _RkAiqAlgoConfigAgamma {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAgamma;

typedef struct _RkAiqAlgoPreAgamma {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAgamma;

typedef struct _RkAiqAlgoPreResAgamma {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAgamma;

typedef struct _RkAiqAlgoProcAgamma {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAgamma;

typedef struct _RkAiqAlgoProcResAgamma {
    RkAiqAlgoResCom proc_res_com;
    AgammaProcRes_t agamma_proc_res;
} RkAiqAlgoProcResAgamma;

typedef struct _RkAiqAlgoPostAgamma {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAgamma;

typedef struct _RkAiqAlgoPostResAgamma {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAgamma;

// adegamma
typedef struct _RkAiqAlgoConfigAdegamma {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAdegamma;

typedef struct _RkAiqAlgoPreAdegamma {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAdegamma;

typedef struct _RkAiqAlgoPreResAdegamma {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAdegamma;

typedef struct _RkAiqAlgoProcAdegamma {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAdegamma;

typedef struct _RkAiqAlgoProcResAdegamma {
    RkAiqAlgoResCom proc_res_com;
    AdegammaProcRes_t adegamma_proc_res;
} RkAiqAlgoProcResAdegamma;

typedef struct _RkAiqAlgoPostAdegamma {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAdegamma;

typedef struct _RkAiqAlgoPostResAdegamma {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAdegamma;

// agic
typedef struct _RkAiqAlgoConfigAgic {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAgic;

typedef struct _RkAiqAlgoPreAgic {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAgic;

typedef struct _RkAiqAlgoPreResAgic {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAgic;

typedef struct _RkAiqAlgoProcAgic {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAgic;

typedef struct _RkAiqAlgoProcResAgic {
    RkAiqAlgoResCom proc_res_com;
} RkAiqAlgoProcResAgic;

typedef struct _RkAiqAlgoPostAgic {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAgic;

typedef struct _RkAiqAlgoPostResAgic {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAgic;

// aie
typedef struct _RkAiqAlgoConfigAie {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAie;

typedef struct _RkAiqAlgoPreAie {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAie;

typedef struct _RkAiqAlgoPreResAie {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAie;

typedef struct _RkAiqAlgoProcAie {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAie;

typedef struct _RkAiqAlgoProcResAie {
    RkAiqAlgoResCom proc_res_com;
    rk_aiq_aie_params_t params;
} RkAiqAlgoProcResAie;

typedef struct _RkAiqAlgoPostAie {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAie;

typedef struct _RkAiqAlgoPostResAie {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAie;

// aldch
typedef struct _RkAiqAlgoConfigAldch {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAldch;

typedef struct _RkAiqAlgoPreAldch {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAldch;

typedef struct _RkAiqAlgoPreResAldch {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAldch;

typedef struct _RkAiqAlgoProcAldch {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAldch;

typedef struct _RkAiqAlgoProcResAldch {
    RkAiqAlgoResCom proc_res_com;
} RkAiqAlgoProcResAldch;

typedef struct _RkAiqAlgoPostAldch {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAldch;

typedef struct _RkAiqAlgoPostResAldch {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAldch;

// alsc
typedef struct _RkAiqAlgoConfigAlsc {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAlsc;

typedef struct _RkAiqAlgoPreAlsc {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAlsc;

typedef struct _RkAiqAlgoPreResAlsc {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAlsc;

typedef struct _RkAiqAlgoProcAlsc {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAlsc;

typedef struct _RkAiqAlgoProcResAlsc {
    RkAiqAlgoResCom proc_res_com;
    rk_aiq_lsc_cfg_t alsc_hw_conf;
} RkAiqAlgoProcResAlsc;

typedef struct _RkAiqAlgoPostAlsc {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAlsc;

typedef struct _RkAiqAlgoPostResAlsc {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAlsc;

// aorb
typedef struct _RkAiqAlgoConfigAorb {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAorb;

typedef struct _RkAiqAlgoPreAorb {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAorb;

typedef struct _RkAiqAlgoPreResAorb {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAorb;

typedef struct _RkAiqAlgoProcAorb {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAorb;

typedef struct _RkAiqAlgoProcResAorb {
    RkAiqAlgoResCom proc_res_com;
    rk_aiq_orb_algo_meas_t aorb_meas;
} RkAiqAlgoProcResAorb;

typedef struct _RkAiqAlgoPostAorb {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAorb;

typedef struct _RkAiqAlgoPostResAorb {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAorb;

// ar2y
typedef struct _RkAiqAlgoConfigAr2y {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAr2y;

typedef struct _RkAiqAlgoPreAr2y {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAr2y;

typedef struct _RkAiqAlgoPreResAr2y {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAr2y;

typedef struct _RkAiqAlgoProcAr2y {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAr2y;

typedef struct _RkAiqAlgoProcResAr2y {
    RkAiqAlgoResCom proc_res_com;
} RkAiqAlgoProcResAr2y;

typedef struct _RkAiqAlgoPostAr2y {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAr2y;

typedef struct _RkAiqAlgoPostResAr2y {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAr2y;

// awdr
typedef struct _RkAiqAlgoConfigAwdr {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAwdr;

typedef struct _RkAiqAlgoPreAwdr {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAwdr;

typedef struct _RkAiqAlgoPreResAwdr {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAwdr;

typedef struct _RkAiqAlgoProcAwdr {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAwdr;

typedef struct _RkAiqAlgoProcResAwdr {
    RkAiqAlgoResCom proc_res_com;
} RkAiqAlgoProcResAwdr;

typedef struct _RkAiqAlgoPostAwdr {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAwdr;

typedef struct _RkAiqAlgoPostResAwdr {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAwdr;

// arawnr
typedef struct _RkAiqAlgoConfigArawnr {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigArawnr;

typedef struct _RkAiqAlgoPreArawnr {
    RkAiqAlgoCom com;
} RkAiqAlgoPreArawnr;

typedef struct _RkAiqAlgoPreResArawnr {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResArawnr;

typedef struct _RkAiqAlgoProcArawnr {
    RkAiqAlgoCom com;
} RkAiqAlgoProcArawnr;

typedef struct _RkAiqAlgoProcResArawnr {
    RkAiqAlgoResCom proc_res_com;
} RkAiqAlgoProcResArawnr;

typedef struct _RkAiqAlgoPostArawnr {
    RkAiqAlgoCom com;
} RkAiqAlgoPostArawnr;

typedef struct _RkAiqAlgoPostResArawnr {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResArawnr;

// amfnr
typedef struct _RkAiqAlgoConfigAmfnr {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAmfnr;

typedef struct _RkAiqAlgoPreAmfnr {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAmfnr;

typedef struct _RkAiqAlgoPreResAmfnr {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAmfnr;

typedef struct _RkAiqAlgoProcAmfnr {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAmfnr;

typedef struct _RkAiqAlgoProcResAmfnr {
    RkAiqAlgoResCom proc_res_com;
} RkAiqAlgoProcResAmfnr;

typedef struct _RkAiqAlgoPostAmfnr {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAmfnr;

typedef struct _RkAiqAlgoPostResAmfnr {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAmfnr;

// aynr
typedef struct _RkAiqAlgoConfigAynr {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAynr;

typedef struct _RkAiqAlgoPreAynr {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAynr;

typedef struct _RkAiqAlgoPreResAynr {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAynr;

typedef struct _RkAiqAlgoProcAynr {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAynr;

typedef struct _RkAiqAlgoProcResAynr {
    RkAiqAlgoResCom proc_res_com;
} RkAiqAlgoProcResAynr;

typedef struct _RkAiqAlgoPostAynr {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAynr;

typedef struct _RkAiqAlgoPostResAynr {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAynr;

// acnr
typedef struct _RkAiqAlgoConfigAcnr {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAcnr;

typedef struct _RkAiqAlgoPreAcnr {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAcnr;

typedef struct _RkAiqAlgoPreResAcnr {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAcnr;

typedef struct _RkAiqAlgoProcAcnr {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAcnr;

typedef struct _RkAiqAlgoProcResAcnr {
    RkAiqAlgoResCom proc_res_com;
} RkAiqAlgoProcResAcnr;

typedef struct _RkAiqAlgoPostAcnr {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAcnr;

typedef struct _RkAiqAlgoPostResAcnr {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAcnr;

// adrc
typedef struct _RkAiqAlgoConfigAdrc {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAdrc;

typedef struct _RkAiqAlgoPreAdrc {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAdrc;

typedef struct _RkAiqAlgoPreResAdrc {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAdrc;

typedef struct _RkAiqAlgoProcAdrc {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAdrc;

typedef struct _RkAiqAlgoProcResAdrc {
    RkAiqAlgoResCom proc_res_com;
} RkAiqAlgoProcResAdrc;

typedef struct _RkAiqAlgoPostAdrc {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAdrc;

typedef struct _RkAiqAlgoPostResAdrc {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAdrc;

// aeis
typedef struct _RkAiqAlgoConfigAeis {
    RkAiqAlgoCom com;
    const rk_aiq_mems_sensor_intf_t *mems_sensor_intf;
} RkAiqAlgoConfigAeis;

typedef struct _RkAiqAlgoPreAeis {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAeis;

typedef struct _RkAiqAlgoPreResAeis {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAeis;

typedef struct _RkAiqAlgoProcAeis {
    RkAiqAlgoCom com;
    int64_t sof;
    double rolling_shutter_skew;
    float integration_time;
    int analog_gain;
    uint32_t frame_width;
    uint32_t frame_height;
} RkAiqAlgoProcAeis;

typedef struct _RkAiqAlgoProcResAeis {
    RkAiqAlgoResCom proc_res_com;
} RkAiqAlgoProcResAeis;

typedef struct _RkAiqAlgoPostAeis {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAeis;

typedef struct _RkAiqAlgoPostResAeis {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAeis;

// amd
typedef struct _RkAiqAlgoConfigAmd {
    RkAiqAlgoCom com;
    int spWidth;
    int spHeight;
    int spAlignedW;
    int spAlignedH;
} RkAiqAlgoConfigAmd;

typedef struct _RkAiqAlgoPreAmd {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAmd;

typedef struct _RkAiqAlgoPreResAmd {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAmd;

typedef struct _RkAiqAlgoProcAmd {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAmd;

typedef struct _RkAiqAlgoProcResAmd {
    RkAiqAlgoResCom proc_res_com;
    rk_aiq_amd_params_t amd_proc_res;
} RkAiqAlgoProcResAmd;

typedef struct _RkAiqAlgoPostAmd {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAmd;

typedef struct _RkAiqAlgoPostResAmd {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAmd;


// again
typedef struct _RkAiqAlgoConfigAgain {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAgain;

typedef struct _RkAiqAlgoPreAgain {
    RkAiqAlgoCom com;
} RkAiqAlgoPreAgain;

typedef struct _RkAiqAlgoPreResAgain {
    RkAiqAlgoResCom pre_res_com;
} RkAiqAlgoPreResAgain;

typedef struct _RkAiqAlgoProcAgain {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAgain;

typedef struct _RkAiqAlgoProcResAgain {
    RkAiqAlgoResCom proc_res_com;
} RkAiqAlgoProcResAgain;

typedef struct _RkAiqAlgoPostAgain {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAgain;

typedef struct _RkAiqAlgoPostResAgain {
    RkAiqAlgoResCom post_res_com;
} RkAiqAlgoPostResAgain;


#endif
