/*
 * Copyright (c) 2019, Fuzhou Rockchip Electronics Co., Ltd
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
#ifndef _CAM_HW_RK_ISP20_HW_H
#define _CAM_HW_RK_ISP20_HW_H

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;

#define BIT_ULL(nr)            (1ULL << (nr))

#define RK_ISP2X_DPCC_ID        0
#define RK_ISP2X_BLS_ID     1
#define RK_ISP2X_SDG_ID     2
#define RK_ISP2X_SIHST_ID       3
#define RK_ISP2X_LSC_ID     4
#define RK_ISP2X_AWB_GAIN_ID    5
#define RK_ISP2X_BDM_ID     7
#define RK_ISP2X_CTK_ID     8
#define RK_ISP2X_GOC_ID     9
#define RK_ISP2X_CPROC_ID       10
#define RK_ISP2X_SIAF_ID        11
#define RK_ISP2X_SIAWB_ID       12
#define RK_ISP2X_IE_ID      13
#define RK_ISP2X_YUVAE_ID       14
#define RK_ISP2X_WDR_ID     15
#define RK_ISP2X_RK_IESHARP_ID  16
#define RK_ISP2X_RAWAF_ID       17
#define RK_ISP2X_RAWAE_LITE_ID  18
#define RK_ISP2X_RAWAE_BIG1_ID  19
#define RK_ISP2X_RAWAE_BIG2_ID  20
#define RK_ISP2X_RAWAE_BIG3_ID  21
#define RK_ISP2X_RAWAWB_ID      22
#define RK_ISP2X_RAWHIST_LITE_ID    23
#define RK_ISP2X_RAWHIST_BIG1_ID    24
#define RK_ISP2X_RAWHIST_BIG2_ID    25
#define RK_ISP2X_RAWHIST_BIG3_ID    26
#define RK_ISP2X_HDRMGE_ID      27
#define RK_ISP2X_RAWNR_ID       28
#define RK_ISP2X_HDRTMO_ID      29
#define RK_ISP2X_GIC_ID     30
#define RK_ISP2X_DHAZ_ID        31
#define RK_ISP2X_3DLUT_ID       32
#define RK_ISP2X_LDCH_ID        33
#define RK_ISP2X_GAIN_ID        34
#define RK_ISP2X_DEBAYER_ID     35
#define Rk_ISP21_BAYNR_ID   36
#define Rk_ISP21_BAY3D_ID   37
#define Rk_ISP21_YNR_ID     38
#define Rk_ISP21_CNR_ID     39
#define Rk_ISP21_SHARP_ID   40
#define Rk_ISP21_DRC_ID     41
#define RK_ISP2X_MAX_ID         42
#define RK_ISP2X_PP_TNR_ID      43
#define RK_ISP2X_PP_NR_ID       44
#define RK_ISP2X_PP_TSHP_ID     45
#define RK_ISP2X_PP_TFEC_ID     46
#define RK_ISP2X_PP_ORB_ID      47
#define RK_ISP2X_PP_MAX_ID      48

#endif
