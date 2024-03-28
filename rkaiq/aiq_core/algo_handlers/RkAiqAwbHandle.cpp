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
#include "RkAiqAwbHandle.h"

#include "RkAiqAblcHandle.h"
#include "RkAiqCore.h"
#include "awb/rk_aiq_uapiv2_awb_int.h"
#include "smart_buffer_priv.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAwbHandleInt);

void RkAiqAwbHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig      = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAwb());
    mProcInParam = (RkAiqAlgoCom*)(new RkAiqAlgoProcAwb());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAwb());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAwbHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
#ifdef RKAIQ_HAVE_AWB_V20
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt   = mNewAtt;
        rk_aiq_uapi_awb_SetAttrib(mAlgoCtx, mCurAtt, false);
        updateAtt = false;
        sendSignal();
    }
    if (updateWbV20Attr) {
        rk_aiq_uapiV2_wb_awb_wbGainAdjust_t wbGainAdjustBK = mCurWbV20Attr.stAuto.wbGainAdjust;
        mCurWbV20Attr   = mNewWbV20Attr;
        mCurWbV20Attr.stAuto.wbGainAdjust = wbGainAdjustBK;
        mallocAndCopyWbGainAdjustAttrib(&mCurWbV20Attr.stAuto.wbGainAdjust,&mNewWbV20Attr.stAuto.wbGainAdjust);
        rk_aiq_uapiV2_awbV20_SetAttrib(mAlgoCtx, mCurWbV20Attr, false);
        updateWbV20Attr = false;
        sendSignal();
    }
    if (updateWbOpModeAttr) {
        mCurWbOpModeAttr   = mNewWbOpModeAttr;
        rk_aiq_uapiV2_awb_SetMwbMode(mAlgoCtx, mCurWbOpModeAttr.mode, false);
        updateWbOpModeAttr = false;
        sendSignal(mCurWbOpModeAttr.sync.sync_mode);
    }
    if (updateWbMwbAttr) {
        mCurWbMwbAttr   = mNewWbMwbAttr;
        rk_aiq_uapiV2_awb_SetMwbAttrib(mAlgoCtx, mCurWbMwbAttr, false);
        updateWbMwbAttr = false;
        sendSignal(mCurWbMwbAttr.sync.sync_mode);
    }
    if (updateWbAwbAttr) {
        rk_aiq_uapiV2_wb_awb_wbGainAdjust_t wbGainAdjustBK = mCurWbAwbAttr.wbGainAdjust;
        mCurWbAwbAttr   = mNewWbAwbAttr;
        mCurWbAwbAttr.wbGainAdjust = wbGainAdjustBK;
        mallocAndCopyWbGainAdjustAttrib(&mCurWbAwbAttr.wbGainAdjust,&mNewWbAwbAttr.wbGainAdjust);
        rk_aiq_uapiV2_awbV20_SetAwbAttrib(mAlgoCtx, mCurWbAwbAttr, false);
        updateWbAwbAttr = false;
        sendSignal();
    }
    if (updateWbAwbWbGainAdjustAttr) {
        mallocAndCopyWbGainAdjustAttrib(&mCurWbAwbWbGainAdjustAttr,&mNewWbAwbWbGainAdjustAttr);
        rk_aiq_uapiV2_awb_SetAwbGainAdjust(mAlgoCtx, mCurWbAwbWbGainAdjustAttr, false);
        updateWbAwbWbGainAdjustAttr = false;
        sendSignal(mCurWbAwbWbGainAdjustAttr.sync.sync_mode);
    }
    if (updateWbAwbWbGainOffsetAttr) {
        mCurWbAwbWbGainOffsetAttr   = mNewWbAwbWbGainOffsetAttr;
        rk_aiq_uapiV2_awb_SetAwbGainOffset(mAlgoCtx, mCurWbAwbWbGainOffsetAttr.gainOffset, false);
        updateWbAwbWbGainOffsetAttr = false;
        sendSignal(mCurWbAwbWbGainOffsetAttr.sync.sync_mode);
    }
    if (updateWbAwbMultiWindowAttr) {
        mCurWbAwbMultiWindowAttr   = mNewWbAwbMultiWindowAttr;
        rk_aiq_uapiV2_awb_SetAwbMultiwindow(mAlgoCtx, mCurWbAwbMultiWindowAttr.multiWindw, false);
        updateWbAwbMultiWindowAttr = false;
        sendSignal(mCurWbAwbMultiWindowAttr.sync.sync_mode);
    }
    if (updateFFWbgainAttr) {
        mCurFFWbgainAttr   = mNewFFWbgainAttr;
        rk_aiq_uapiV2_awb_SetFstFrWbgain(mAlgoCtx, mCurFFWbgainAttr.wggain, false);
        updateFFWbgainAttr = false;
        sendSignal(mCurFFWbgainAttr.sync.sync_mode);
    }
    if (needSync) mCfgMutex.unlock();
#endif
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::setAttrib(rk_aiq_wb_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_awb_SetAttrib(mAlgoCtx, att, false);
#else
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, &att, sizeof(rk_aiq_wb_attrib_t))) {
        mNewAtt   = att;
        updateAtt = true;
        waitSignal();
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getAttrib(rk_aiq_wb_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    SmartLock lock (mCfgMutex);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_awb_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getCct(rk_aiq_wb_cct_t* cct) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awb_GetCCT(mAlgoCtx, cct);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::queryWBInfo(rk_aiq_wb_querry_info_t* wb_querry_info) {
    ENTER_ANALYZER_FUNCTION();

    SmartLock lock (mCfgMutex);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awb_QueryWBInfo(mAlgoCtx, wb_querry_info);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::lock() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awb_Lock(mAlgoCtx);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::unlock() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awb_Unlock(mAlgoCtx);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getAlgoStat(rk_tool_awb_stat_res_full_t *awb_stat_algo) {
    ENTER_ANALYZER_FUNCTION();

    SmartLock lock (mCfgMutex);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awb_GetAlgoStat(mAlgoCtx,awb_stat_algo);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getStrategyResult(rk_tool_awb_strategy_result_t *awb_strategy_result) {
    ENTER_ANALYZER_FUNCTION();

    SmartLock lock (mCfgMutex);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awb_GetStrategyResult(mAlgoCtx,awb_strategy_result);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn RkAiqAwbHandleInt::setWbV20Attrib(rk_aiq_uapiV2_wbV20_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifdef RKAIQ_HAVE_AWB_V20
    mCfgMutex.lock();
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapiV2_awbV20_SetAttrib(mAlgoCtx, att, false);
#else
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurWbV20Attr, &att, sizeof(rk_aiq_uapiV2_wbV20_attrib_t))) {
        rk_aiq_uapiV2_wb_awb_wbGainAdjust_t wbGainAdjustBK = mNewWbV20Attr.stAuto.wbGainAdjust;
        mNewWbV20Attr   = att;
        mNewWbV20Attr.stAuto.wbGainAdjust = wbGainAdjustBK;
        mallocAndCopyWbGainAdjustAttrib(&mNewWbV20Attr.stAuto.wbGainAdjust,&att.stAuto.wbGainAdjust);
        updateWbV20Attr = true;
        waitSignal();
    }
#endif

    mCfgMutex.unlock();
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getWbV20Attrib(rk_aiq_uapiV2_wbV20_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    SmartLock lock (mCfgMutex);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef RKAIQ_HAVE_AWB_V20
    rk_aiq_uapiV2_awbV20_GetAttrib(mAlgoCtx, att);
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::setWbOpModeAttrib(rk_aiq_uapiV2_wb_opMode_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapiV2_awb_SetMwbMode(mAlgoCtx, att.mode, false);
#else

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewWbOpModeAttr, &att, sizeof(att)))
        isChanged = true;
    else if (att.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurWbOpModeAttr, &att, sizeof(att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewWbOpModeAttr   = att;
        updateWbOpModeAttr = true;
        waitSignal(att.sync.sync_mode);
    }
#endif
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getWbOpModeAttrib(rk_aiq_uapiV2_wb_opMode_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapiV2_awb_GetMwbMode(mAlgoCtx, &att->mode);
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapiV2_awb_GetMwbMode(mAlgoCtx, &att->mode);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateWbOpModeAttr) {
            memcpy(att, &mNewWbOpModeAttr, sizeof(mNewWbOpModeAttr));
            att->sync.done = false;
        } else {
            rk_aiq_uapiV2_awb_GetMwbMode(mAlgoCtx, &att->mode);
            att->sync.sync_mode = mNewWbOpModeAttr.sync.sync_mode;
            att->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::setMwbAttrib(rk_aiq_wb_mwb_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapiV2_awb_SetMwbAttrib(mAlgoCtx, att, false);
#else
    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewWbMwbAttr, &att, sizeof(att)))
        isChanged = true;
    else if (att.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurWbMwbAttr, &att, sizeof(att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewWbMwbAttr   = att;
        updateWbMwbAttr = true;
        waitSignal(att.sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getMwbAttrib(rk_aiq_wb_mwb_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapiV2_awb_GetMwbAttrib(mAlgoCtx, att);
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapiV2_awb_GetMwbAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateWbMwbAttr) {
            memcpy(att, &mNewWbMwbAttr, sizeof(mNewWbMwbAttr));
            att->sync.done = false;
        } else {
            rk_aiq_uapiV2_awb_GetMwbAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewWbMwbAttr.sync.sync_mode;
            att->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::setAwbV20Attrib(rk_aiq_uapiV2_wbV20_awb_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if defined(RKAIQ_HAVE_AWB_V20) || defined(RKAIQ_HAVE_AWB_V21)
    mCfgMutex.lock();
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapiV2_awbV20_SetAwbAttrib(mAlgoCtx, att, false);
#else
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurWbAwbAttr, &att, sizeof(rk_aiq_uapiV2_wbV20_awb_attrib_t))) {
        rk_aiq_uapiV2_wb_awb_wbGainAdjust_t wbGainAdjustBK = mNewWbAwbAttr.wbGainAdjust;
        mNewWbAwbAttr   = att;
        mNewWbAwbAttr.wbGainAdjust = wbGainAdjustBK;
        mallocAndCopyWbGainAdjustAttrib(&mNewWbAwbAttr.wbGainAdjust,&att.wbGainAdjust);
        updateWbAwbAttr = true;
        waitSignal();
    }
#endif

    mCfgMutex.unlock();
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getAwbV20Attrib(rk_aiq_uapiV2_wbV20_awb_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    SmartLock lock (mCfgMutex);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if defined(RKAIQ_HAVE_AWB_V20) || defined(RKAIQ_HAVE_AWB_V21)
    rk_aiq_uapiV2_awbV20_GetAwbAttrib(mAlgoCtx, att);
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

#ifndef DISABLE_HANDLE_ATTRIB
XCamReturn RkAiqAwbHandleInt::mallocAndCopyWbGainAdjustAttrib(rk_aiq_uapiV2_wb_awb_wbGainAdjust_t* dst,
    const rk_aiq_uapiV2_wb_awb_wbGainAdjust_t *src)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    //free
    for(int i = 0; i < dst->lutAll_len; i++) {
        int srcNum2 = src->lutAll[i].ct_grid_num * src->lutAll[i].cri_grid_num;
        int dstNum2 = dst->lutAll[i].ct_grid_num * dst->lutAll[i].cri_grid_num;
        if(dst->lutAll[i].cri_lut_out && dstNum2 < srcNum2){
            //LOGE_AWB("free dst->lutAll[%d].cri_lut_out= %p",i,dst->lutAll[i].cri_lut_out);
            free(dst->lutAll[i].cri_lut_out);
            dst->lutAll[i].cri_lut_out = NULL;
        }
        if(dst->lutAll[i].ct_lut_out && dstNum2 < srcNum2){
            free(dst->lutAll[i].ct_lut_out);
            dst->lutAll[i].ct_lut_out = NULL;
        }
    }
    if (dst->lutAll && dst->lutAll_len < src->lutAll_len) {
        //LOGE_AWB(" free dst->lutAll= %p",dst->lutAll);
        free(dst->lutAll);
        dst->lutAll = NULL;
    }
    //malloc
    if (!dst->lutAll){
        dst->lutAll = (rk_aiq_uapiV2_wb_awb_wbGainAdjustLut_t*)malloc(sizeof(rk_aiq_uapiV2_wb_awb_wbGainAdjustLut_t)*src->lutAll_len);
        //LOGE_AWB("malloc dst->lutAll= %p",dst->lutAll);
        memset(dst->lutAll,0,sizeof(rk_aiq_uapiV2_wb_awb_wbGainAdjustLut_t)*src->lutAll_len);
        for(int i = 0; i < src->lutAll_len; i++) {
            int num2 = src->lutAll[i].ct_grid_num * src->lutAll[i].cri_grid_num;
            if (!dst->lutAll[i].cri_lut_out){
                dst->lutAll[i].cri_lut_out = (float*)malloc(sizeof(float) * num2);
                memset(dst->lutAll[i].cri_lut_out,0,sizeof(float) * num2);

            }
            if (!dst->lutAll[i].ct_lut_out){
                dst->lutAll[i].ct_lut_out = (float*)malloc(sizeof(float) * num2);
                memset( dst->lutAll[i].ct_lut_out,0,sizeof(float) * num2);
            }
            //LOGE_AWB("malloc  dst->lutAll[%d].cri_lut_out= %p",i,dst->lutAll[i].cri_lut_out);
        }
    }
    //copy
    dst->enable = src->enable;
    dst->lutAll_len = src->lutAll_len;
    dst->sync = src->sync;
    for(int i = 0; i < src->lutAll_len; i++) {
        dst->lutAll[i].ct_grid_num = src->lutAll[i].ct_grid_num;
        dst->lutAll[i].cri_grid_num = src->lutAll[i].cri_grid_num;
        dst->lutAll[i].ct_in_range[0] = src->lutAll[i].ct_in_range[0];
        dst->lutAll[i].ct_in_range[1] = src->lutAll[i].ct_in_range[1];
        dst->lutAll[i].cri_in_range[0] = src->lutAll[i].cri_in_range[0];
        dst->lutAll[i].cri_in_range[1] = src->lutAll[i].cri_in_range[1];
        dst->lutAll[i].lumaValue = src->lutAll[i].lumaValue;
        int num2 = dst->lutAll[i].ct_grid_num * dst->lutAll[i].cri_grid_num;
        memcpy(dst->lutAll[i].cri_lut_out, src->lutAll[i].cri_lut_out,
               sizeof(dst->lutAll[i].cri_lut_out[0])*num2);
        memcpy(dst->lutAll[i].ct_lut_out, src->lutAll[i].ct_lut_out,
               sizeof(dst->lutAll[i].ct_lut_out[0])*num2);
    }
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::freeWbGainAdjustAttrib(rk_aiq_uapiV2_wb_awb_wbGainAdjust_t* dst)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    //free
    for(int i = 0; i < dst->lutAll_len; i++) {
        //LOGE_AWB("free dst->lutAll[%d].cri_lut_out= %p",i,dst->lutAll[i].cri_lut_out);
        if(dst->lutAll[i].cri_lut_out ){
            free(dst->lutAll[i].cri_lut_out);
            dst->lutAll[i].cri_lut_out = NULL;
        }
        if(dst->lutAll[i].ct_lut_out){
            free(dst->lutAll[i].ct_lut_out);
            dst->lutAll[i].ct_lut_out = NULL;
        }
    }
    if (dst->lutAll) {
        //LOGE_AWB(" free dst->lutAll= %p",dst->lutAll);
        free(dst->lutAll);
        dst->lutAll = NULL;
    }
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif

XCamReturn RkAiqAwbHandleInt::setWbAwbWbGainAdjustAttrib(rk_aiq_uapiV2_wb_awb_wbGainAdjust_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapiV2_awb_SetAwbGainAdjust(mAlgoCtx, att, false);
#else
    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewWbAwbWbGainAdjustAttr, &att, sizeof(att)))
        isChanged = true;
    else if (att.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurWbAwbWbGainAdjustAttr, &att, sizeof(att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mallocAndCopyWbGainAdjustAttrib(&mNewWbAwbWbGainAdjustAttr,&att);
        updateWbAwbWbGainAdjustAttr = true;
        waitSignal(att.sync.sync_mode);
    }
#endif
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getWbAwbWbGainAdjustAttrib(rk_aiq_uapiV2_wb_awb_wbGainAdjust_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapiV2_awb_GetAwbGainAdjust(mAlgoCtx, att);
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapiV2_awb_GetAwbGainAdjust(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateWbAwbWbGainAdjustAttr) {
            mallocAndCopyWbGainAdjustAttrib(att,&mNewWbAwbWbGainAdjustAttr);
            att->sync.done = false;
        } else {
            mCfgMutex.lock();
            rk_aiq_uapiV2_awb_GetAwbGainAdjust(mAlgoCtx, att);
            mCfgMutex.unlock();
            att->sync.sync_mode = mNewWbAwbWbGainAdjustAttr.sync.sync_mode;
            att->sync.done      = true;
        }

    }
#endif


    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::setWbAwbWbGainOffsetAttrib(rk_aiq_uapiV2_wb_awb_wbGainOffset_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapiV2_awb_SetAwbGainOffset(mAlgoCtx, att.gainOffset, false);
#else

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewWbAwbWbGainOffsetAttr, &att, sizeof(att)))
        isChanged = true;
    else if (att.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurWbAwbWbGainOffsetAttr, &att, sizeof(att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewWbAwbWbGainOffsetAttr   = att;
        updateWbAwbWbGainOffsetAttr = true;
        waitSignal(att.sync.sync_mode);
    }
#endif
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getWbAwbWbGainOffsetAttrib(rk_aiq_uapiV2_wb_awb_wbGainOffset_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapiV2_awb_GetAwbGainOffset(mAlgoCtx, &att->gainOffset);
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapiV2_awb_GetAwbGainOffset(mAlgoCtx, &att->gainOffset);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateWbAwbWbGainOffsetAttr) {
            memcpy(att, &mNewWbAwbWbGainOffsetAttr, sizeof(mNewWbAwbWbGainOffsetAttr));
            att->sync.done = false;
        } else {
            mCfgMutex.lock();
            rk_aiq_uapiV2_awb_GetAwbGainOffset(mAlgoCtx, &att->gainOffset);
            mCfgMutex.unlock();
            att->sync.sync_mode = mNewWbAwbWbGainOffsetAttr.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::setWbAwbMultiWindowAttrib(rk_aiq_uapiV2_wb_awb_mulWindow_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapiV2_awb_SetAwbMultiwindow(mAlgoCtx, att.multiWindw, false);
#else

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewWbAwbMultiWindowAttr, &att, sizeof(att)))
        isChanged = true;
    else if (att.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurWbAwbMultiWindowAttr, &att, sizeof(att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewWbAwbMultiWindowAttr   = att;
        updateWbAwbMultiWindowAttr = true;
        waitSignal(att.sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getWbAwbMultiWindowAttrib(rk_aiq_uapiV2_wb_awb_mulWindow_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapiV2_awb_GetAwbMultiwindow(mAlgoCtx, &att->multiWindw);
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapiV2_awb_GetAwbMultiwindow(mAlgoCtx, &att->multiWindw);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateWbAwbMultiWindowAttr) {
            memcpy(att, &mNewWbAwbMultiWindowAttr, sizeof(mNewWbAwbMultiWindowAttr));
            att->sync.done = false;
        } else {
            mCfgMutex.lock();
            rk_aiq_uapiV2_awb_GetAwbMultiwindow(mAlgoCtx, &att->multiWindw);
            mCfgMutex.unlock();
            att->sync.sync_mode = mNewWbAwbMultiWindowAttr.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::setFFWbgainAttrib(rk_aiq_uapiV2_awb_ffwbgain_attr_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapiV2_awb_SetFstFrWbgain(mAlgoCtx, att.wggain, false);
#else
    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewFFWbgainAttr, &att, sizeof(att)))
        isChanged = true;
    else if (att.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurFFWbgainAttr, &att, sizeof(att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewFFWbgainAttr   = att;
        updateFFWbgainAttr = true;
        waitSignal(att.sync.sync_mode);
    }
#endif
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "awb handle prepare failed");

    RkAiqAlgoConfigAwb* awb_config_int = (RkAiqAlgoConfigAwb*)mConfig;
    // TODO
    // awb_config_int->rawBit;
    awb_config_int->mem_ops_ptr   = mAiqCore->mShareMemOps;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    mCfgMutex.lock();
    ret                       = des->prepare(mConfig);
    mCfgMutex.unlock();
    RKAIQCORE_CHECK_RET(ret, "awb algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPreAwb* awb_pre_int        = (RkAiqAlgoPreAwb*)mPreInParam;
    RkAiqAlgoPreResAwb* awb_pre_res_int = (RkAiqAlgoPreResAwb*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "awb handle preProcess failed");
    }
    if (!sharedCom->init) {
        if (shared->awbStatsBuf == nullptr) {
            LOGE("no awb stats, ignore!");
            return XCAM_RETURN_BYPASS;
        }
    }

    int module_hw_version = sharedCom->ctxCfigs[RK_AIQ_ALGO_TYPE_AWB].module_hw_version;
    //awb_pre_int->awbStatsBuf = shared->awbStatsBuf;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "awb algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return ret;
}

XCamReturn RkAiqAwbHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAwb* awb_proc_int = (RkAiqAlgoProcAwb*)mProcInParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (!sharedCom->init) {
        if (shared->awbStatsBuf == nullptr) {
            LOGE("no awb stats, ignore!");
            return XCAM_RETURN_BYPASS;
        }
    }

    RkAiqAlgoProcResAwb* awb_proc_res_int = (RkAiqAlgoProcResAwb*)mProcOutParam;

#if RKAIQ_HAVE_AWB_V32
    if (shared->awbStatsBuf)
        awb_proc_int->awb_statsBuf_v32 = &shared->awbStatsBuf->awb_stats_v32;
    else
        awb_proc_int->awb_statsBuf_v32 = NULL;
#elif RKAIQ_HAVE_AWB_V21
    #if defined(ISP_HW_V30)
        if (shared->awbStatsBuf)
            awb_proc_int->awb_statsBuf_v3x = &shared->awbStatsBuf->awb_stats_v3x;
        else
            awb_proc_int->awb_statsBuf_v3x = NULL;
        awb_proc_int->blc_cfg_effect = &shared->awbStatsBuf->blc_cfg_effect;
    #else
        if (shared->awbStatsBuf)
            awb_proc_int->awb_statsBuf_v201 = &shared->awbStatsBuf->awb_stats_v201;
        else
            awb_proc_int->awb_statsBuf_v201 = NULL;
    #endif
#elif RKAIQ_HAVE_AWB_V20
    if (shared->awbStatsBuf)
        awb_proc_int->awbStatsBuf = &shared->awbStatsBuf->awb_stats;
    else
        awb_proc_int->awbStatsBuf = NULL;
#else
    LOGE_AWB("module_hw_version of awb is isvalid!!!!");
#endif

    if (shared->aecStatsBuf)
        awb_proc_int->aecStatsBuf = &shared->aecStatsBuf->aec_stats;
    else
        awb_proc_int->aecStatsBuf = NULL;

    awb_proc_int->ablcProcResVaid = false;
#if RKAIQ_HAVE_BLC_V1
    /*SmartPtr<RkAiqHandle>* ablc_handle = mAiqCore->getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_ABLC);
    int algo_id                      = (*ablc_handle)->getAlgoId();

    if (ablc_handle) {
        if (algo_id == 0) {
            RkAiqAblcHandleInt* ablc_algo = dynamic_cast<RkAiqAblcHandleInt*>(ablc_handle->ptr());
            ablc_algo->getProcRes(&awb_proc_int->ablcProcRes);
            awb_proc_int->ablcProcResVaid = true;
        }
    }*/
    awb_proc_int->ablcProcRes= shared->res_comb.ablc_proc_res;
    awb_proc_int->ablcProcResVaid = true;
#endif
#if RKAIQ_HAVE_BLC_V32
#if USE_NEWSTRUCT
        blc_param_t* blc_res;
        AblcProc_V32_t ablcProcResV32;
        int isp_ob_max;
        ablcProcResV32.enable = shared->res_comb.blc_en;
        if (shared->res_comb.blc_proc_res) {
            blc_res = shared->res_comb.blc_proc_res;
            ablcProcResV32.blc_r  = blc_res->dyn.obcPreTnr.hw_blcC_obR_val;
            ablcProcResV32.blc_gr = blc_res->dyn.obcPreTnr.hw_blcC_obGr_val;
            ablcProcResV32.blc_gb = blc_res->dyn.obcPreTnr.hw_blcC_obGb_val;
            ablcProcResV32.blc_b  = blc_res->dyn.obcPreTnr.hw_blcC_obB_val;
            if (blc_res->dyn.obcPostTnr.sw_btnrT_obcPostTnr_en) {
                if (blc_res->dyn.obcPostTnr.sw_blcT_obcPostTnr_mode == blc_autoOBCPostTnr_mode) {
                    ablcProcResV32.blc1_enable = true;
                    ablcProcResV32.blc1_r  = 0;
                    ablcProcResV32.blc1_gr = 0;
                    ablcProcResV32.blc1_gb = 0;
                    ablcProcResV32.blc1_b  = 0;

                    ablcProcResV32.blc_ob_enable  = true;
                    ablcProcResV32.isp_ob_offset  = blc_res->dyn.obcPostTnr.sw_blcT_autoOB_offset;
                    ablcProcResV32.isp_ob_predgain = 1.0;
                    isp_ob_max = (int)(4096 * ablcProcResV32.isp_ob_predgain) - ablcProcResV32.isp_ob_offset;
                    if (isp_ob_max > 0)
                    ablcProcResV32.isp_ob_max = isp_ob_max < 1048575 ? isp_ob_max : 1048575;
                } else {
                    ablcProcResV32.blc1_enable = true;
                    ablcProcResV32.blc1_r  = blc_res->dyn.obcPostTnr.hw_blcT_manualOBR_val;
                    ablcProcResV32.blc1_gr = blc_res->dyn.obcPostTnr.hw_blcT_manualOBGr_val;
                    ablcProcResV32.blc1_gb = blc_res->dyn.obcPostTnr.hw_blcT_manualOBGb_val;
                    ablcProcResV32.blc1_b  = blc_res->dyn.obcPostTnr.hw_blcT_manualOBB_val;

                    ablcProcResV32.blc_ob_enable  = true;
                    ablcProcResV32.isp_ob_offset  = 0;
                    ablcProcResV32.isp_ob_predgain = 1.0;
                    ablcProcResV32.isp_ob_max = 4096;
                }
            } else {
                ablcProcResV32.blc1_enable = false;
                ablcProcResV32.blc_ob_enable = false;
                ablcProcResV32.blc1_r  = 0;
                ablcProcResV32.blc1_gr = 0;
                ablcProcResV32.blc1_gb = 0;
                ablcProcResV32.blc1_b  = 0;

                ablcProcResV32.blc_ob_enable  = false;
                ablcProcResV32.isp_ob_offset  = 0;
                ablcProcResV32.isp_ob_predgain = 1.0;
                ablcProcResV32.isp_ob_max = 0xfff;
            }
        }
        awb_proc_int->ablcProcResV32 = &ablcProcResV32;
#else
        awb_proc_int->ablcProcResV32= shared->res_comb.ablcV32_proc_res;
        awb_proc_int->ablcProcResVaid = true;
#endif
#endif
    // for otp awb
    awb_proc_int->awb_otp = &sharedCom->snsDes.otp_awb;

#if defined(ISP_HW_V30)
    awb_proc_res_int->awb_hw1_para = &shared->fullParams->mAwbParams->data()->result;
#elif defined(ISP_HW_V21)
    awb_proc_res_int->awb_hw1_para = &shared->fullParams->mAwbParams->data()->result;
#elif defined(ISP_HW_V39) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    awb_proc_res_int->awb_hw32_para = &shared->fullParams->mAwbParams->data()->result;
#else
    awb_proc_res_int->awb_hw0_para = &shared->fullParams->mAwbParams->data()->result;
#endif

    awb_proc_res_int->awb_gain_algo = &shared->fullParams->mAwbGainParams->data()->result;

    ret = RkAiqHandle::processing();
    if (ret < 0) {
        LOGE_ANALYZER("awb handle processing failed ret %d", ret);
        return ret;
    } else if (ret == XCAM_RETURN_BYPASS) {
        LOGW_ANALYZER("%s:%d bypass !", __func__, __LINE__);
        return ret;
    }

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
#if 0
    ret = des->processing(mProcInParam, mProcOutParam);
#else
    ret = des->processing(mProcInParam, (RkAiqAlgoResCom*)awb_proc_res_int);
#endif
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();
#endif
    if (ret < 0) {
        LOGE_ANALYZER("awb algo processing failed ret %d", ret);
        return ret;
    } else if (ret == XCAM_RETURN_BYPASS) {
        LOGW_ANALYZER("%s:%d bypass !", __func__, __LINE__);
        ret = XCAM_RETURN_NO_ERROR;
    }

    if (awb_proc_res_int->awb_cfg_update || awb_proc_res_int->awb_gain_update) {
        if (getAlgoId() == 0) {
            mProcResShared = new RkAiqAlgoProcResAwbIntShared();
        }
        memcpy(&mProcResShared->result.awb_gain_algo, awb_proc_res_int->awb_gain_algo, sizeof(rk_aiq_wb_gain_t));
        mProcResShared->result.awb_smooth_factor = awb_proc_res_int->awb_smooth_factor;
        mProcResShared->result.varianceLuma = awb_proc_res_int->varianceLuma;
        mProcResShared->result.awbConverged = awb_proc_res_int->awbConverged;
    } else {
        LOGD_AWB("awb results not updated");
    }

    if (mPostShared) {
        if (mAiqCore->mAlogsComSharedParams.init) {
            RkAiqCore::RkAiqAlgosGroupShared_t* grpShared = nullptr;
            uint64_t grpMask = grpId2GrpMask(RK_AIQ_CORE_ANALYZE_GRP1);
            if (!mAiqCore->getGroupSharedParams(grpMask, grpShared)) {
                if (grpShared) {
                    mProcResShared->set_sequence(0);
                    XCamVideoBuffer* xCamAwbProcRes = convert_to_XCamVideoBuffer(mProcResShared);
                    grpShared->res_comb.awb_proc_res = xCamAwbProcRes;
                }
            }
        } else {
            mProcResShared->set_sequence(shared->frameId);
            RkAiqCoreVdBufMsg msg(XCAM_MESSAGE_AWB_PROC_RES_OK, shared->frameId, mProcResShared);
            mAiqCore->post_message(msg);
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPostAwb* awb_post_int        = (RkAiqAlgoPostAwb*)mPostInParam;
    RkAiqAlgoPostResAwb* awb_post_res_int = (RkAiqAlgoPostResAwb*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "awb handle postProcess failed");
        return ret;
    }

    if (!sharedCom->init) {
        if (shared->awbStatsBuf == nullptr) {
            LOGE("no awb stats, ignore!");
            return XCAM_RETURN_BYPASS;
        }
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "awb algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return ret;
}

XCamReturn RkAiqAwbHandleInt::genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (mIsMulRun && this->getAlgoId() == 0) {
        // do nothing for rkawb if custom algo running with rk algo
        return ret;
    }

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    RkAiqAlgoProcResAwb* awb_com                = (RkAiqAlgoProcResAwb*)mProcOutParam;


    rk_aiq_isp_awb_params_t* awb_param = params->mAwbParams->data().ptr();
    rk_aiq_isp_awb_gain_params_t* awb_gain_param = params->mAwbGainParams->data().ptr();

    if (sharedCom->init) {
        awb_gain_param->frame_id = 0;
        awb_param->frame_id      = 0;
    } else {
        awb_gain_param->frame_id = shared->frameId;
        awb_param->frame_id      = shared->frameId;
    }

    if (awb_com->awb_gain_update) {
        mWbGainSyncFlag = shared->frameId;
        awb_gain_param->sync_flag = mWbGainSyncFlag;
        cur_params->mAwbGainParams = params->mAwbGainParams ;
        awb_gain_param->is_update = true;
        awb_com->awb_gain_update = false;
        LOGD_AWB("[%d] wbgain params from algo", mWbGainSyncFlag);
    } else if (mWbGainSyncFlag != awb_param->sync_flag) {
        awb_gain_param->sync_flag = mWbGainSyncFlag;
        if (cur_params->mAwbGainParams.ptr()) {
            awb_gain_param->is_update = true;
            awb_gain_param->result = cur_params->mAwbGainParams->data()->result;
        } else {
            LOGE_AWB("no latest params !");
            awb_gain_param->is_update = false;
        }
        LOGD_AWB("[%d] wbgain from latest [%d]", shared->frameId, mWbGainSyncFlag);
    } else {
        // do nothing, result in buf needn't update
        awb_gain_param->is_update = false;
        LOGD_AWB("[%d] wbgain params needn't update", shared->frameId);
    }

    if (awb_com->awb_cfg_update) {
        mWbParamSyncFlag = shared->frameId;
        awb_param->sync_flag = mWbParamSyncFlag;
        cur_params->mAwbParams     = params->mAwbParams;
        awb_param->is_update = true;
        awb_com->awb_cfg_update = false;
        LOGD_AWB("[%d] params from algo", mWbParamSyncFlag);
    } else if (mWbParamSyncFlag != awb_param->sync_flag) {
        awb_param->sync_flag = mWbParamSyncFlag;
        // copy from latest result
        if (cur_params->mAwbParams.ptr()) {
            awb_param->is_update = true;
            awb_param->result = cur_params->mAwbParams->data()->result;
        } else {
            LOGE_AWB("no latest params !");
            awb_param->is_update = false;
        }
        LOGD_AWB("[%d] params from latest [%d]", shared->frameId, mWbParamSyncFlag);
    } else {
        awb_param->is_update = false;
        // do nothing, result in buf needn't update
        LOGD_AWB("[%d] params needn't update", shared->frameId);
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
