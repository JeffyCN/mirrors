/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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
#ifndef _RK_AIQ_HANDLE_INT_H_
#define _RK_AIQ_HANDLE_INT_H_

#include "RkAiqHandle.h"
#if RKAIQ_HAVE_3DLUT_V1
#include "algo_handlers/RkAiqA3dlutHandle.h"
#endif
#include "algo_handlers/RkAiqAbayer2dnrV2Handle.h"
#include "algo_handlers/RkAiqAbayertnrV2Handle.h"
#include "algo_handlers/RkAiqAblcHandle.h"
#if RKAIQ_HAVE_CAC_V10
#include "algo_handlers/RkAiqAcacHandle.h"
#endif
#if RKAIQ_HAVE_CAC_V11
#include "algo_handlers/RkAiqAcacV11Handle.h"
#endif
#if RKAIQ_HAVE_CAC_V03
#include "algo_handlers/RkAiqAcacV3Handle.h"
#endif
#if RKAIQ_HAVE_CCM
#include "algo_handlers/RkAiqAccmHandle.h"
#endif
#if RKAIQ_HAVE_CGC_V1
#include "algo_handlers/RkAiqAcgcHandle.h"
#endif
#include "algo_handlers/RkAiqAcnrHandle.h"
#include "algo_handlers/RkAiqAcnrV1Handle.h"
#include "algo_handlers/RkAiqAcnrV2Handle.h"
#if RKAIQ_HAVE_ACP_V10
#include "algo_handlers/RkAiqAcpHandle.h"
#endif
#if RKAIQ_HAVE_CSM_V1
#include "algo_handlers/RkAiqAcsmHandle.h"
#endif
#if RKAIQ_HAVE_DEBAYER
#include "algo_handlers/RkAiqAdebayerHandle.h"
#endif
#if RKAIQ_HAVE_DEGAMMA_V1
#include "algo_handlers/RkAiqAdegammaHandle.h"
#endif
#if RKAIQ_HAVE_DEHAZE
#include "algo_handlers/RkAiqAdehazeHandle.h"
#endif
#include "algo_handlers/RkAiqAdpccHandle.h"
#if RKAIQ_HAVE_DRC
#include "algo_handlers/RkAiqAdrcHandle.h"
#endif
#if RKAIQ_HAVE_AE_V1
#include "algo_handlers/RkAiqAeHandle.h"
#endif
#include "algo_handlers/RkAiqAeisHandle.h"
#include "algo_handlers/RkAiqAfecHandle.h"
#if RKAIQ_HAVE_AF
#include "algo_handlers/RkAiqAfHandle.h"
#endif
#include "algo_handlers/RkAiqAgainHandle.h"
#include "algo_handlers/RkAiqAgainV2Handle.h"
#if RKAIQ_HAVE_GAMMA
#include "algo_handlers/RkAiqAgammaHandle.h"
#endif
#if RKAIQ_HAVE_GIC
#include "algo_handlers/RkAiqAgicHandle.h"
#endif
#if RKAIQ_HAVE_AIE_V10
#include "algo_handlers/RkAiqAieHandle.h"
#endif
#include "algo_handlers/RkAiqAldchHandle.h"
#if RKAIQ_HAVE_LSC
#include "algo_handlers/RkAiqAlscHandle.h"
#endif
#include "algo_handlers/RkAiqAmdHandle.h"
#if RKAIQ_HAVE_MERGE
#include "algo_handlers/RkAiqAmergeHandle.h"
#endif
#include "algo_handlers/RkAiqAmfnrHandle.h"
#include "algo_handlers/RkAiqAnrHandle.h"
#include "algo_handlers/RkAiqAorbHandle.h"
#include "algo_handlers/RkAiqArawnrHandle.h"
#include "algo_handlers/RkAiqArawnrV2Handle.h"
#if RKAIQ_HAVE_ASD_V10
#include "algo_handlers/RkAiqAsdHandle.h"
#endif
#include "algo_handlers/RkAiqAsharpHandle.h"
#include "algo_handlers/RkAiqAsharpV3Handle.h"
#include "algo_handlers/RkAiqAsharpV4Handle.h"
#if RKAIQ_HAVE_TMO_V1
#include "algo_handlers/RkAiqAtmoHandle.h"
#endif
#include "algo_handlers/RkAiqAwbHandle.h"
#if RKAIQ_HAVE_AWB_V32
#include "algo_handlers/RkAiqAwbV32Handle.h"
#endif
#if RKAIQ_HAVE_AWB_V21
#include "algo_handlers/RkAiqAwbV21Handle.h"
#endif
#if RKAIQ_HAVE_WDR_V1
#include "algo_handlers/RkAiqAwdrHandle.h"
#endif
#include "algo_handlers/RkAiqAynrHandle.h"
#if RKAIQ_HAVE_YNR_V2
#include "algo_handlers/RkAiqAynrV2Handle.h"
#endif
#include "algo_handlers/RkAiqAynrV3Handle.h"


#if RKAIQ_HAVE_YNR_V22
#include "algo_handlers/RkAiqAynrV22Handle.h"
#endif
#if RKAIQ_HAVE_YNR_V24
#include "algo_handlers/RkAiqAynrV24Handle.h"
#endif
#if (RKAIQ_HAVE_CNR_V30 || RKAIQ_HAVE_CNR_V30_LITE)
#include "algo_handlers/RkAiqAcnrV30Handle.h"
#endif
#if (RKAIQ_HAVE_SHARP_V33 || RKAIQ_HAVE_SHARP_V33_LITE)
#include "algo_handlers/RkAiqAsharpV33Handle.h"
#endif
#if (RKAIQ_HAVE_SHARP_V34)
#include "algo_handlers/RkAiqAsharpV34Handle.h"
#endif
#if RKAIQ_HAVE_BAYER2DNR_V23
#include "algo_handlers/RkAiqAbayer2dnrV23Handle.h"
#endif
#if (RKAIQ_HAVE_BAYERTNR_V23 || RKAIQ_HAVE_BAYERTNR_V23_LITE)
#include "algo_handlers/RkAiqAbayertnrV23Handle.h"
#endif
#if RKAIQ_HAVE_BLC_V32
#include "algo_handlers/RkAiqAblcV32Handle.h"
#endif

#if (RKAIQ_HAVE_BAYERTNR_V30)
#include "algo_handlers/RkAiqAbayertnrV30Handle.h"
#endif
#if (RKAIQ_HAVE_YUVME_V1)
#include "algo_handlers/RkAiqAyuvmeV1Handle.h"
#endif
#if (RKAIQ_HAVE_CNR_V31)
#include "algo_handlers/RkAiqAcnrV31Handle.h"
#endif

#endif
