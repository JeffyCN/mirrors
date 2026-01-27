/******************************************************************************
 *
 * Copyright 2019, Fuzhou Rockchip Electronics Co.Ltd . All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Fuzhou Rockchip Electronics Co.Ltd .
 *
 *
 *****************************************************************************/
#ifndef __RK_AIQ_UAPI_AFD_INT_H__
#define __RK_AIQ_UAPI_AFD_INT_H__
#include "xcore/base/xcam_common.h"
#include "algos/rk_aiq_algo_des.h"

XCAM_BEGIN_DECLARE

typedef struct Uapi_AfdAttr_s {
    bool enable;
    int  scale_ratio;
} Uapi_AfdAttr_t;

XCamReturn
rk_aiq_uapi_afd_v2_GetAttr(RkAiqAlgoContext*  ctx, Uapi_AfdAttr_t* pAfdAttr);

/*XCamReturn
rk_aiq_uapi_afd_v2_GetEnable(RkAiqAlgoContext*  ctx, bool* en);

XCamReturn
rk_aiq_uapi_afd_v2_GetScaleRatio(RkAiqAlgoContext*  ctx, int* scale_ratio);*/

XCAM_END_DECLARE

#endif /*__RK_AIQ_UAPI_AFD_INT_H__*/
