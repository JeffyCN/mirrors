/*
 * rk_aiq_param_asd.h
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
#ifndef _RK_AIQ_PARAM_ASD_H_
#define _RK_AIQ_PARAM_ASD_H_

typedef struct asd_faceControl_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_asdT_faceInfo_en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Enable face info function.\n
        Freq of use: high))  */
    bool sw_asdT_faceInfo_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_asdT_faceInfoDetect_num),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(3),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(Face detect number.\n
        Freq of use: high))  */
    unsigned char sw_asdT_faceInfoDetect_num;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_asdT_faceInfoNoDetect_num),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(3),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(Face no detect number.\n
        Freq of use: high))  */
    unsigned char sw_asdT_faceInfoNoDetect_num;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_asdT_faceInfoNoInput_num),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(30),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(3),
        M4_NOTES(Face no input number.\n
        Freq of use: high))  */
    unsigned char sw_asdT_faceInfoNoInput_num;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_asdT_faceInfoChange_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(4),
        M4_NOTES(Change ratio of face roi.\n
        Freq of use: high))  */
    float sw_asdT_faceInfoChange_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_asdT_faceInfoChange_num),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,30),
        M4_DEFAULT(3),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(5),
        M4_NOTES(Change frame number of face roi.\n
        Freq of use: high))  */
    unsigned char sw_asdT_faceInfoChange_num;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_asdT_faceInfoStable_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.7),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(6),
        M4_NOTES(Stable ratio of face roi.\n
        Freq of use: high))  */
    float sw_asdT_faceInfoStable_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_asdT_faceInfoStable_num),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,30),
        M4_DEFAULT(3),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(7),
        M4_NOTES(Stable frame number of face roi.\n
        Freq of use: high))  */
    unsigned char sw_asdT_faceInfoStable_num;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_asdT_faceInfoTimeOut_num),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(10),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(8),
        M4_NOTES(Timeout frame number of face roi unstable.\n
        Freq of use: high))  */
    unsigned char sw_asdT_faceInfoTimeOut_num;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_asdT_faceInfoArea_weight),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.4),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(9),
        M4_NOTES(Area weight.\n
        Freq of use: high))  */
    float sw_asdT_faceInfoArea_weight;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_asdT_faceInfoDist_weight),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.6),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(10),
        M4_NOTES(Distance weight .\n
        Freq of use: high))  */
    float sw_asdT_faceInfoDist_weight;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_asdT_preSelFaceEnc_weight),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.1),
        M4_DIGIT_EX(3),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(11),
        M4_NOTES(Enhance weight for previous selected face.\n
        Freq of use: high))  */
    float sw_asdT_preSelFaceEnc_weight;
} asd_faceControl_t;

typedef struct asd_params_static_t {
	/* M4_GENERIC_DESC(
        M4_ALIAS(faceCtl),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(face control info.))  */
    asd_faceControl_t faceCtl;
} asd_params_static_t;

typedef struct asd_params_dyn_s {
    char reserved;
} asd_params_dyn_t;

typedef struct asd_param_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(static_param),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The static params of demosaic module))  */
    asd_params_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    asd_params_dyn_t dyn;
} asd_param_t;

#endif
