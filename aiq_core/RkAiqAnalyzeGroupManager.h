

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

#ifndef _RK_AIQ_ANALYZE_GROUP_MANAGER_
#define _RK_AIQ_ANALYZE_GROUP_MANAGER_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <functional>
#include <map>

#include "MessageBus.h"
#include "RkAiqCore.h"
#include "RkAiqSharedDataManager.h"
#include "rk_aiq_algo_types_int.h"
#include "rk_aiq_comm.h"
#include "rk_aiq_pool.h"
#include "video_buffer.h"
#include "xcam_std.h"
#include "xcam_thread.h"

// using namespace XCam;
namespace RkCam {

class RkAiqAnalyzeGroupManager;
class RkAiqCore;
class RkAiqAnalyzeGroupMsgHdlThread;

// TODO(Cody): This is just workaround for current implementation
//using MessageHandleWrapper = std::function<XCamReturn(const std::list<SmartPtr<XCamMessage>>&)>;
typedef std::function<XCamReturn(std::list<SmartPtr<XCamMessage>>&, uint32_t&)> MessageHandleWrapper;

class RkAiqAnalyzerGroup {
 public:
    struct GroupMessage {
        std::list<SmartPtr<XCamMessage>> msgList;
        uint64_t msg_flags;
    };

    RkAiqAnalyzerGroup(RkAiqCore* aiqCore, enum rk_aiq_core_analyze_type_e type,
                       const uint64_t flag, const RkAiqGrpConditions_t* grpConds);
    virtual ~RkAiqAnalyzerGroup() = default;

    void setConcreteHandler(const MessageHandleWrapper handler) { mHandler = handler; }
    XCamReturn start();
    bool pushMsg(const SmartPtr<XCamMessage>& msg);
    XCamReturn msgHandle(const SmartPtr<XCamMessage>& msg);
    XCamReturn stop();

    const rk_aiq_core_analyze_type_e getType() const { return mGroupType; }
    const uint64_t getDepsFlag() const { return mDepsFlag; }

 private:
    void msgReduction(std::map<uint32_t, GroupMessage>& msgMap);
    int8_t getMsgDelayCnt(XCamMessageType &msg_id);

    // TODO(Cody): use weak ptr
    RkAiqCore* mAiqCore;
    const rk_aiq_core_analyze_type_e mGroupType;
    const uint64_t mDepsFlag;
    RkAiqGrpConditions_t mGrpConds;
    SmartPtr<RkAiqAnalyzeGroupMsgHdlThread> mRkAiqGroupMsgHdlTh;
    std::map<uint32_t, GroupMessage> mGroupMsgMap;
    MessageHandleWrapper mHandler;
};

class RkAiqAnalyzeGroupMsgHdlThread : public Thread {
 public:
    RkAiqAnalyzeGroupMsgHdlThread(const std::string name, RkAiqAnalyzerGroup* group)
        : Thread(name.c_str()), mRkAiqAnalyzerGroup(group){};
    ~RkAiqAnalyzeGroupMsgHdlThread() { mMsgsQueue.clear(); };

    void triger_stop() { mMsgsQueue.pause_pop(); };

    void triger_start() {
        mMsgsQueue.clear();
        mMsgsQueue.resume_pop();
    };

    bool push_msg(const SmartPtr<XCamMessage>& buffer) {
        mMsgsQueue.push(buffer);
        return true;
    };

 protected:
    // virtual bool started ();
    virtual void stopped() { mMsgsQueue.clear(); };

    virtual bool loop();

 private:
    RkAiqAnalyzerGroup* mRkAiqAnalyzerGroup;
    SafeList<XCamMessage> mMsgsQueue;
};

class RkAiqAnalyzeGroupManager {
 public:
    RkAiqAnalyzeGroupManager(RkAiqCore* aiqCore);
    virtual ~RkAiqAnalyzeGroupManager(){};

    void parseAlgoGroup(const struct RkAiqAlgoDesCommExt* algoDes);

    XCamReturn start();
    XCamReturn stop();

    XCamReturn handleMessage(const SmartPtr<XCamMessage> &msg);
    std::list<int>& getGroupAlgoList(rk_aiq_core_analyze_type_e group) {
        return mGroupAlgoListMap[group];
    }
 protected:
    XCamReturn measGroupMessageHandler(std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id);
    XCamReturn otherGroupMessageHandler(std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id);
    XCamReturn amdGroupMessageHandler(std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id);
    XCamReturn thumbnailsGroupMessageHandler(std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id);
    XCamReturn lscGroupMessageHandler(std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id);
    XCamReturn aeGroupMessageHandler(std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id);
    XCamReturn awbGroupMessageHandler(std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id);
    XCamReturn amfnrGroupMessageHandler(std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id);
    XCamReturn aynrGroupMessageHandler(std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id);
    XCamReturn grp0MessageHandler(std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id);
    XCamReturn grp1MessageHandler(std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id);
    XCamReturn afMessageHandler(std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id);
    XCamReturn eisGroupMessageHandler(std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id);
    XCamReturn orbGroupMessageHandler(std::list<SmartPtr<XCamMessage>>& msgs, uint32_t& id);

 private:
    RkAiqCore* mAiqCore;
    std::map<uint64_t, SmartPtr<RkAiqAnalyzerGroup>> mGroupMap;
    std::map<uint64_t, std::list<int>> mGroupAlgoListMap;
};

}  // namespace RkCam

#endif  // _RK_AIQ_ANALYZE_GROUP_MANAGER_

