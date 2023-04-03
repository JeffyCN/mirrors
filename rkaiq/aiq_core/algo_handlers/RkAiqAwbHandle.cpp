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
    // mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAwb());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAwbHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
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

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::setAttrib(rk_aiq_wb_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
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
    mCfgMutex.lock();
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

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getWbV20Attrib(rk_aiq_uapiV2_wbV20_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    SmartLock lock (mCfgMutex);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awbV20_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::setWbOpModeAttrib(rk_aiq_uapiV2_wb_opMode_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

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

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getWbOpModeAttrib(rk_aiq_uapiV2_wb_opMode_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

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

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::setMwbAttrib(rk_aiq_wb_mwb_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

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

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getMwbAttrib(rk_aiq_wb_mwb_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

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

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::setAwbV20Attrib(rk_aiq_uapiV2_wbV20_awb_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
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

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getAwbV20Attrib(rk_aiq_uapiV2_wbV20_awb_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    SmartLock lock (mCfgMutex);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapiV2_awbV20_GetAwbAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

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


XCamReturn RkAiqAwbHandleInt::setWbAwbWbGainAdjustAttrib(rk_aiq_uapiV2_wb_awb_wbGainAdjust_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

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

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getWbAwbWbGainAdjustAttrib(rk_aiq_uapiV2_wb_awb_wbGainAdjust_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

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


    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::setWbAwbWbGainOffsetAttrib(rk_aiq_uapiV2_wb_awb_wbGainOffset_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

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

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getWbAwbWbGainOffsetAttrib(rk_aiq_uapiV2_wb_awb_wbGainOffset_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

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

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::setWbAwbMultiWindowAttrib(rk_aiq_uapiV2_wb_awb_mulWindow_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

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

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::getWbAwbMultiWindowAttrib(rk_aiq_uapiV2_wb_awb_mulWindow_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

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

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandleInt::setFFWbgainAttrib(rk_aiq_uapiV2_awb_ffwbgain_attr_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

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
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
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
#if 0
    RkAiqAlgoProcResAwb* awb_proc_res_int = (RkAiqAlgoProcResAwb*)mProcOutParam;
#else
    mProcResShared           = new RkAiqAlgoProcResAwbIntShared();
    if (!mProcResShared.ptr()) {
        LOGE("new awb mProcOutParam failed, bypass!");
        return XCAM_RETURN_BYPASS;
    }
    RkAiqAlgoProcResAwb* awb_proc_res_int = &mProcResShared->result;
    // mProcOutParam = (RkAiqAlgoResCom*)awb_proc_res_int;
#endif
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    awb_proc_int->awbStatsBuf = shared->awbStatsBuf;
    awb_proc_int->ablcProcResVaid = false;
#if defined(ISP_HW_V30)
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
#endif
#if RKAIQ_HAVE_BLC_V32
        awb_proc_int->ablcProcResV32= shared->res_comb.ablcV32_proc_res;
        awb_proc_int->ablcProcResVaid = true;
#endif
    // for otp awb
    awb_proc_int->awb_otp = &sharedCom->snsDes.otp_awb;

    ret = RkAiqHandle::processing();
    if (ret < 0) {
        LOGE_ANALYZER("awb handle processing failed ret %d", ret);
        mProcResShared = NULL;
        return ret;
    } else if (ret == XCAM_RETURN_BYPASS) {
        LOGW_ANALYZER("%s:%d bypass !", __func__, __LINE__);
        mProcResShared = NULL;
        return ret;
    }

    if (!sharedCom->init) {
        if (shared->awbStatsBuf == nullptr) {
            LOGE("no awb stats, ignore!");
            mProcResShared.release();
            return XCAM_RETURN_BYPASS;
        }
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
#if 0
    ret = des->processing(mProcInParam, mProcOutParam);
#else
    ret = des->processing(mProcInParam, (RkAiqAlgoResCom*)awb_proc_res_int);
#endif
    if (ret < 0) {
        LOGE_ANALYZER("awb algo processing failed ret %d", ret);
        mProcResShared = NULL;
        return ret;
    } else if (ret == XCAM_RETURN_BYPASS) {
        LOGW_ANALYZER("%s:%d bypass !", __func__, __LINE__);
        ret = XCAM_RETURN_NO_ERROR;
        mProcResShared = NULL;
    }

    if (mAiqCore->mAlogsComSharedParams.init) {
        RkAiqCore::RkAiqAlgosGroupShared_t* grpShared = nullptr;
        uint64_t grpMask = grpId2GrpMask(RK_AIQ_CORE_ANALYZE_GRP1);
        if (!mAiqCore->getGroupSharedParams(grpMask, grpShared)) {
            if (grpShared) {
                SmartPtr<BufferProxy> procResProxy = new BufferProxy(mProcResShared);
                procResProxy->set_sequence(0);

                if (procResProxy.ptr()) {
                    XCamVideoBuffer* xCamAwbProcRes = convert_to_XCamVideoBuffer(procResProxy);
                    grpShared->res_comb.awb_proc_res = xCamAwbProcRes;
                }
            }
        }
    } else if (mPostShared) {
        SmartPtr<BufferProxy> msg_data = new BufferProxy(mProcResShared);
        msg_data->set_sequence(shared->frameId);
        SmartPtr<XCamMessage> msg =
            new RkAiqCoreVdBufMsg(XCAM_MESSAGE_AWB_PROC_RES_OK, shared->frameId, msg_data);
        mAiqCore->post_message(msg);
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
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (!mProcResShared.ptr()) {
#if defined(ISP_HW_V30)
        params->mAwbV3xParams = cur_params->mAwbV3xParams;
        params->mAwbV3xParams->data()->frame_id = shared->frameId;
#elif defined(ISP_HW_V21)
        params->mAwbV21Params = cur_params->mAwbV21Params;
        params->mAwbV21Params->data()->frame_id = shared->frameId;
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
        params->mAwbV32Params = cur_params->mAwbV32Params;
        params->mAwbV32Params->data()->frame_id = shared->frameId;
#else
        params->mAwbParams = cur_params->mAwbParams;
        params->mAwbParams->data()->frame_id = shared->frameId;
#endif
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
        params->mAwbGainV32Params = cur_params->mAwbGainV32Params;
        params->mAwbGainV32Params->data()->frame_id = shared->frameId;
#else
        params->mAwbGainParams = cur_params->mAwbGainParams;
        params->mAwbGainParams->data()->frame_id = shared->frameId;
#endif
        return XCAM_RETURN_BYPASS;
    }

    RkAiqAlgoProcResAwb* awb_com                = &mProcResShared->result;

    if (!awb_com) {
        LOGD_ANALYZER("no awb result");
#if defined(ISP_HW_V30)
        params->mAwbV3xParams = cur_params->mAwbV3xParams;
        params->mAwbV3xParams->data()->frame_id = shared->frameId;
#elif defined(ISP_HW_V21)
        params->mAwbV21Params = cur_params->mAwbV21Params;
        params->mAwbV21Params->data()->frame_id = shared->frameId;
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
        params->mAwbV32Params = cur_params->mAwbV32Params;
        params->mAwbV32Params->data()->frame_id = shared->frameId;
#else
        params->mAwbParams = cur_params->mAwbParams;
        params->mAwbParams->data()->frame_id = shared->frameId;
#endif
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
        params->mAwbGainV32Params = cur_params->mAwbGainV32Params;
        params->mAwbGainV32Params->data()->frame_id = shared->frameId;
#else
        params->mAwbGainParams = cur_params->mAwbGainParams;
        params->mAwbGainParams->data()->frame_id = shared->frameId;
#endif
        mProcResShared = NULL;
        return XCAM_RETURN_BYPASS;
    }

#if defined(ISP_HW_V30)
    rk_aiq_isp_awb_params_v3x_t* awb_param = params->mAwbV3xParams->data().ptr();
#elif defined(ISP_HW_V21)
    rk_aiq_isp_awb_params_v21_t* awb_param = params->mAwbV21Params->data().ptr();
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    rk_aiq_isp_awb_params_v32_t* awb_param = params->mAwbV32Params->data().ptr();
#else
    rk_aiq_isp_awb_params_v20_t* awb_param = params->mAwbParams->data().ptr();
#endif
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    rk_aiq_isp_awb_gain_params_v32_t* awb_gain_param = params->mAwbGainV32Params->data().ptr();
#else
    rk_aiq_isp_awb_gain_params_v20_t* awb_gain_param = params->mAwbGainParams->data().ptr();
#endif
    RkAiqAlgoProcResAwb* awb_rk                      = (RkAiqAlgoProcResAwb*)awb_com;

#if 0
    isp_param->awb_gain_update = awb_rk->awb_gain_update;
    isp_param->awb_cfg_update = awb_rk->awb_cfg_update;
    isp_param->awb_gain = awb_rk->awb_gain_algo;
    isp_param->awb_cfg = awb_rk->awb_hw0_para;
    //isp_param->awb_cfg_v201 = awb_rk->awb_hw1_para;
#else
    // TODO: update states
    // awb_gain_param->result.awb_gain_update = awb_rk->awb_gain_update;
    // isp_param->awb_cfg_update = awb_rk->awb_cfg_update;
    if (sharedCom->init) {
        awb_gain_param->frame_id = 0;
        awb_param->frame_id      = 0;
    } else {
        awb_gain_param->frame_id = shared->frameId;
        awb_param->frame_id      = shared->frameId;
    }
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    rk_aiq_wb_gain_v32_t *awb_gain_v32 = &awb_gain_param->result;
    awb_gain_v32->rgain = awb_rk->awb_gain_algo.rgain;
    awb_gain_v32->grgain = awb_rk->awb_gain_algo.grgain;
    awb_gain_v32->gbgain = awb_rk->awb_gain_algo.gbgain;
    awb_gain_v32->bgain = awb_rk->awb_gain_algo.bgain;
    awb_gain_v32->applyPosition = awb_rk->wbgainApplyPosition;
#else
    awb_gain_param->result     = awb_rk->awb_gain_algo;
#endif
#if defined(ISP_HW_V30) || defined(ISP_HW_V21)
    awb_param->result = awb_rk->awb_hw1_para;
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    awb_param->result = awb_rk->awb_hw32_para;
#else
    awb_param->result = awb_rk->awb_hw0_para;
#endif

#endif

    if (!this->getAlgoId()) {
        RkAiqAlgoProcResAwb* awb_rk_int = (RkAiqAlgoProcResAwb*)awb_com;
    }

#if defined(ISP_HW_V30)
    cur_params->mAwbV3xParams  = params->mAwbV3xParams;
#elif defined(ISP_HW_V21)
    cur_params->mAwbV21Params  = params->mAwbV21Params;
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    cur_params->mAwbV32Params  = params->mAwbV32Params;
#else
    cur_params->mAwbParams     = params->mAwbParams;
#endif
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    cur_params->mAwbGainV32Params = params->mAwbGainV32Params;
#else
    cur_params->mAwbGainParams = params->mAwbGainParams;
#endif

    mProcResShared = NULL;

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
