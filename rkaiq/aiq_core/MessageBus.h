
/*
 * MessageBus.h
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

#ifndef _RK_MESSAGEBUS_H_
#define _RK_MESSAGEBUS_H_

#include "xcam_thread.h"
#include "safe_list.h"

using namespace XCam;
namespace RkCam {

enum XCamMessageType {
    XCAM_MESSAGE_SOF_INFO_OK,
    XCAM_MESSAGE_ISP_STATS_OK,
    XCAM_MESSAGE_ISPP_STATS_OK,
    XCAM_MESSAGE_ISP_POLL_SP_OK,
    XCAM_MESSAGE_ISP_POLL_TX_OK,
    XCAM_MESSAGE_ISP_GAIN_OK,
    XCAM_MESSAGE_ISPP_GAIN_KG_OK,
    XCAM_MESSAGE_NR_IMG_OK,
    XCAM_MESSAGE_AWB_PROC_RES_OK,
    XCAM_MESSAGE_AE_PROC_RES_OK,
    XCAM_MESSAGE_AF_PROC_RES_OK,
    XCAM_MESSAGE_AEC_STATS_OK,
    XCAM_MESSAGE_AWB_STATS_OK,
    XCAM_MESSAGE_AF_STATS_OK,
    XCAM_MESSAGE_ATMO_STATS_OK,
    XCAM_MESSAGE_ADEHAZE_STATS_OK,
    XCAM_MESSAGE_AGAIN_STATS_OK,
    XCAM_MESSAGE_ISPP_GAIN_WR_OK,
    XCAM_MESSAGE_AMD_PROC_RES_OK,
    XCAM_MESSAGE_AE_PRE_RES_OK,
    XCAM_MESSAGE_PDAF_STATS_OK,
    XCAM_MESSAGE_ORB_STATS_OK,
    XCAM_MESSAGE_BLC_PROC_RES_OK,
    XCAM_MESSAGE_BLC_V32_PROC_RES_OK,
    XCAM_MESSAGE_YNR_V3_PROC_RES_OK,
    XCAM_MESSAGE_YNR_V22_PROC_RES_OK,
    XCAM_MESSAGE_YNR_V24_PROC_RES_OK,
    XCAM_MESSAGE_YNR_PROC_RES_OK,
    XCAM_MESSAGE_VICAP_POLL_SCL_OK,
    XCAM_MESSAGE_TNR_STATS_OK,
};

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
static const char* MessageType2Str[64] = {
    [XCAM_MESSAGE_SOF_INFO_OK]         = "SOF_INFO",
    [XCAM_MESSAGE_ISP_STATS_OK]        = "ISP_STATS",
    [XCAM_MESSAGE_ISPP_STATS_OK]       = "ISPP_STATS",
    [XCAM_MESSAGE_ISP_POLL_SP_OK]      = "ISP_SP",
    [XCAM_MESSAGE_ISP_POLL_TX_OK]      = "ISP_TX",
    [XCAM_MESSAGE_ISP_GAIN_OK]         = "ISP_GAIN",
    [XCAM_MESSAGE_ISPP_GAIN_KG_OK]     = "ISPP_KG_GAIN",
    [XCAM_MESSAGE_NR_IMG_OK]           = "NR_IMG",
    [XCAM_MESSAGE_AWB_PROC_RES_OK]     = "AWB_PROC_RES",
    [XCAM_MESSAGE_AE_PROC_RES_OK]      = "AE_PROC_RES",
    [XCAM_MESSAGE_AF_PROC_RES_OK]      = "AF_PROC_RES",
    [XCAM_MESSAGE_AEC_STATS_OK]        = "AEC_STATS",
    [XCAM_MESSAGE_AWB_STATS_OK]        = "AWB_STATS",
    [XCAM_MESSAGE_AF_STATS_OK]         = "AF_STATS",
    [XCAM_MESSAGE_ATMO_STATS_OK]       = "ATMO_STATS",
    [XCAM_MESSAGE_ADEHAZE_STATS_OK]    = "ADHAZ_STATS",
    [XCAM_MESSAGE_AGAIN_STATS_OK]      = "AGAIN_STATS",
    [XCAM_MESSAGE_ISPP_GAIN_WR_OK]     = "ISPP_WR_GAIN",
    [XCAM_MESSAGE_AMD_PROC_RES_OK]     = "AMD_PROC_RES",
    [XCAM_MESSAGE_AE_PRE_RES_OK]       = "AE_PRE_RES",
    [XCAM_MESSAGE_PDAF_STATS_OK]       = "PDAF_STATS",
    [XCAM_MESSAGE_ORB_STATS_OK]        = "ORB_STATS",
    [XCAM_MESSAGE_BLC_PROC_RES_OK]     = "BLC_PROC_RES",
    [XCAM_MESSAGE_BLC_V32_PROC_RES_OK] = "BLC_V32_PROC_RES",
    [XCAM_MESSAGE_YNR_V3_PROC_RES_OK]  = "YNR_V3_PROC_RES",
    [XCAM_MESSAGE_YNR_V22_PROC_RES_OK] = "YNR_V22_PROC_RES",
    [XCAM_MESSAGE_YNR_V24_PROC_RES_OK] = "YNR_V24_PROC_RES",
    [XCAM_MESSAGE_YNR_PROC_RES_OK]     = "YNR_PROC_RES",
    [XCAM_MESSAGE_VICAP_POLL_SCL_OK]   = "VICAP_POLL_SCL",
    [XCAM_MESSAGE_TNR_STATS_OK]        = "TNR_STATS",
};
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

struct XCamMessage {
    XCamMessageType  msg_id;
    unsigned frame_id;

    XCamMessage () {};
    XCamMessage (XCamMessageType type, unsigned id)
        : msg_id (type)
        , frame_id (id) {};
    virtual ~XCamMessage () {};

};

class RkAiqCore;
class MessageThread
    : public Thread
{
public:
    explicit MessageThread (RkAiqCore *rkAiqCore)
        : Thread ("MessageThread")
        , mRkAiqCore(rkAiqCore)
    {}

    void triger_stop() {
        mMsgQueue.pause_pop ();
    };

    void triger_start() {
        mMsgQueue.clear ();
        mMsgQueue.resume_pop ();
    };

    bool push_msg (const SmartPtr<XCamMessage> &msg) {
        mMsgQueue.push (msg);
        return true;
    };

protected:
    //virtual bool started ();
    virtual void stopped () {
        mMsgQueue.clear ();
    };
    virtual bool loop ();

private:
    RkAiqCore* mRkAiqCore;
    SafeList<XCamMessage> mMsgQueue;
};

}
#endif //_RK_MESSAGEBUS_H_
