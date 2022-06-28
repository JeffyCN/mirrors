/*
 * Copyright (c) 2019-2021 Rockchip Eletronics Co., Ltd.
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

#ifndef _RK_AIQ_ANALYZE_GROUP_MANAGER_
#define _RK_AIQ_ANALYZE_GROUP_MANAGER_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <functional>
#include <map>

#include "MessageBus.h"
#include "RkAiqCore.h"
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
typedef std::function<XCamReturn(std::vector<SmartPtr<XCamMessage>>&, uint32_t, uint64_t)>
    MessageHandleWrapper;

class RkAiqAnalyzerGroup {
 public:
    struct GroupMessage {
        std::vector<SmartPtr<XCamMessage>> msgList;
        uint64_t msg_flags;
    };

    RkAiqAnalyzerGroup(RkAiqCore* aiqCore, enum rk_aiq_core_analyze_type_e type,
                       const uint64_t flag, const RkAiqGrpConditions_t* grpConds,
                       const bool singleThrd);
    virtual ~RkAiqAnalyzerGroup() = default;

    void setConcreteHandler(const MessageHandleWrapper handler) { mHandler = handler; }
    XCamReturn start();
    bool pushMsg(const SmartPtr<XCamMessage>& msg);
    XCamReturn msgHandle(const SmartPtr<XCamMessage>& msg);
    XCamReturn stop();

    const rk_aiq_core_analyze_type_e getType() const { return mGroupType; }
    const uint64_t getDepsFlag() const { return mDepsFlag; }
    void setDepsFlag(uint64_t new_deps) { mDepsFlag = new_deps; }

 private:
    void msgReduction(std::map<uint32_t, GroupMessage>& msgMap);
    int8_t getMsgDelayCnt(XCamMessageType &msg_id);

    // TODO(Cody): use weak ptr
    RkAiqCore* mAiqCore;
    const rk_aiq_core_analyze_type_e mGroupType;
    uint64_t mDepsFlag;
    RkAiqGrpConditions_t mGrpConds;
    SmartPtr<RkAiqAnalyzeGroupMsgHdlThread> mRkAiqGroupMsgHdlTh;
    std::map<uint32_t, GroupMessage> mGroupMsgMap;
    MessageHandleWrapper mHandler;
};

class RkAiqAnalyzeGroupMsgHdlThread : public Thread {
 public:
    RkAiqAnalyzeGroupMsgHdlThread(const std::string name, RkAiqAnalyzerGroup* group)
        : Thread(name.c_str()) {
        if (group != nullptr) mHandlerGroups.push_back(group);
    };
    ~RkAiqAnalyzeGroupMsgHdlThread() { mMsgsQueue.clear(); };

    void add_group(RkAiqAnalyzerGroup* group) {
        mHandlerGroups.push_back(group);
    }

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
    std::vector<RkAiqAnalyzerGroup*> mHandlerGroups;
    SafeList<XCamMessage> mMsgsQueue;
};

class RkAiqAnalyzeGroupManager {
 public:
    RkAiqAnalyzeGroupManager(RkAiqCore* aiqCore, bool single_thread);
    virtual ~RkAiqAnalyzeGroupManager(){};

    void parseAlgoGroup(const struct RkAiqAlgoDesCommExt* algoDes);

    uint64_t getGrpDeps(rk_aiq_core_analyze_type_e group);
    XCamReturn setGrpDeps(rk_aiq_core_analyze_type_e group, uint64_t new_deps);
    XCamReturn start();
    XCamReturn stop();

    XCamReturn firstAnalyze();
    XCamReturn handleMessage(const SmartPtr<XCamMessage> &msg);
    std::vector<SmartPtr<RkAiqHandle>>& getGroupAlgoList(rk_aiq_core_analyze_type_e group) {
        return mGroupAlgoListMap[group];
    }

 protected:
    XCamReturn groupMessageHandler(std::vector<SmartPtr<XCamMessage>>& msgs, uint32_t id,
                                   uint64_t grpId);
    XCamReturn thumbnailsGroupMessageHandler(std::vector<SmartPtr<XCamMessage>>& msgs, uint32_t id,
                                             uint64_t grpId);

 private:
    RkAiqCore* mAiqCore;
    const bool mSingleThreadMode;
    std::map<uint64_t, SmartPtr<RkAiqAnalyzerGroup>> mGroupMap;
    std::map<uint64_t, std::vector<SmartPtr<RkAiqHandle>>> mGroupAlgoListMap;
    SmartPtr<RkAiqAnalyzeGroupMsgHdlThread> mMsgThrd;
};

}  // namespace RkCam

#endif  // _RK_AIQ_ANALYZE_GROUP_MANAGER_

