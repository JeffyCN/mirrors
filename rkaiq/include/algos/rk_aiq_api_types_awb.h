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

#ifndef _RK_AIQ_API_TYPES_AWB_H_
#define _RK_AIQ_API_TYPES_AWB_H_


RKAIQ_BEGIN_DECLARE

typedef struct{
    /* M4_GENERIC_DESC(
        M4_ALIAS(wbGainCtrl),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(\n
        Freq of use: high))  */
    awb_gainCtrl_t wbGainCtrl;
    /* M4_GENERIC_DESC(
        M4_ALIAS(awbStats),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(\n
        Freq of use: high))  */
    awb_Stats_t awbStats;
    /* M4_GENERIC_DESC(
        M4_ALIAS(awbGnCalcStep),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(\n
        Freq of use: high))  */
    awb_gainCalcStep_t awbGnCalcStep;
    /* M4_GENERIC_DESC(
        M4_ALIAS(awbGnCalcOth),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(\n
        Freq of use: high))  */
    awb_gainCalcOth_t awbGnCalcOth;
} awb_api_attrib_t;//awb_api_attrib_t

typedef struct{
    // TODO
} awb_status_t;



RKAIQ_END_DECLARE

#endif
