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

#include "RknnManager.h"

XCamReturn RknnManager_prepare(RknnManager_t* pRknnManager, AiqCamHwBase_t* pCamHw, CamCalibDbV2Context_t* pCalibDbV2)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    pRknnManager->pCamHw = pCamHw;
    pRknnManager->pCalibDbV2 = pCalibDbV2;

    return ret;
}

XCamReturn RknnManager_start(RknnManager_t* pRknnManager)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    return ret;
}

XCamReturn RknnManager_stop(RknnManager_t* pRknnManager)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    return ret;
}

XCamReturn RknnManager_deinit(RknnManager_t* pRknnManager)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    return ret;
}

XCamReturn RknnManager_sendEvent(RknnManager_t* pRknnManager)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pRknnManager->pCamHw->_hwResListener.hwResCb) {
        AiqHwAinnEvt_t event;
        event._base.frame_id = 0;
        event._base.type     = ISP_POLL_RKNN_DONE;
        event._base.vb       = NULL;
        pRknnManager->pCamHw->_hwResListener.hwResCb(pRknnManager->pCamHw->_hwResListener._pCtx, (AiqHwEvt_t*)&event);
    }

    return ret;
}

XCamReturn RknnManager_hdlEvent(RknnManager_t* pRknnManager, AiqHwAinnEvt_t *event)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // aiisp event
    if (event->_base.type == ISP_POLL_AIBNR_DONE || event->_base.type == ISP_POLL_AIYNR_DONE) {
        // call rknn run
        
    }
    // rknn event
    else if (event->_base.type == ISP_POLL_RKNN_DONE) {
        // if NRNN is done, call rknn or inform isp to do BE
        // make event to inform Rknn
        RknnManager_sendEvent(pRknnManager);
    }

    return ret;
}

XCamReturn RknnManager_init(RknnManager_t* pRknnManager)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    memset(pRknnManager, 0, sizeof(RknnManager_t));

    return ret;
}

