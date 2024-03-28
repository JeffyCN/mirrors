/*
 * rkisp_aiq_core.h
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

#ifndef _RK_AIQ_HANDLE_H_
#define _RK_AIQ_HANDLE_H_

#include <map>

#include "rk_aiq_algo_types.h"
#include "rk_aiq_types.h"
#include "xcam_mutex.h"
#include "rk_aiq_pool.h"
#include "RkAiqGlobalParamsManager.h"

namespace RkCam {

/*
 --------------------------------
|         :RkAiqHandle           |
 --------------------------------
|  rk_aiq_xxx_attrib_t mCurAtt;  |
|  rk_aiq_xxx_attrib_t mNewAtt;  |
|        Mutex mCfgMutex;        |
|        bool updateAtt;         |
 --------------------------------
|         updateConfig()         |
 --------------------------------
*/

#define DISABLE_HANDLE_ATTRIB

class RkAiqCore;
struct RkAiqAlgosGroupShared_s;

class RkAiqHandle {
 public:
    explicit RkAiqHandle(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore);
    virtual ~RkAiqHandle();
    void setEnable(bool enable) { mEnable = enable; };
    void setReConfig(bool reconfig) { mReConfig = reconfig; };
    bool getEnable() { return mEnable; };
    virtual XCamReturn prepare();
    virtual XCamReturn preProcess();
    virtual XCamReturn processing();
    virtual XCamReturn postProcess();
    virtual XCamReturn genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params) { return XCAM_RETURN_NO_ERROR; };
    RkAiqAlgoContext* getAlgoCtx() { return mAlgoCtx; }
    int getAlgoId() const { return mDes->id; }
    int getAlgoType() const { return mDes->type; }
    void setGroupId(int32_t gId) {
        mGroupId = gId;
    }
    int32_t getGroupId() {
       return mGroupId;
    }

    void setNextHdl(RkAiqHandle* next) {
        mNextHdl = next;
    }

    void setParentHdl(RkAiqHandle* parent) {
        mParentHdl = parent;
    }

    RkAiqHandle* getNextHdl() {
       return mNextHdl;
    }

    RkAiqHandle* getParent() {
       return mParentHdl;
    }

    // rk algo running with custom algo concunrrently
    void setMulRun(bool isMulRun) {
        mIsMulRun = isMulRun;
        if (isMulRun && mDes->id == 0)
            mPostShared = false;
        else
            mPostShared = true;
    }

    void setGroupShared(void* grp_shared) {
        mAlogsGroupSharedParams = grp_shared;
    }
    void* getGroupShared() {
       return mAlogsGroupSharedParams;
    }
    virtual XCamReturn updateConfig(bool needSync) { return XCAM_RETURN_NO_ERROR; };
    virtual RkAiqAlgoResCom* getPreProcRes() {
        return mPreOutParam;
    }
    virtual RkAiqAlgoResCom* getProcProcRes() {
        return mProcOutParam;
    }
    bool isUpdateGrpAttr(void) {
       return mIsUpdateGrpAttr;
    }
    void clearUpdateGrpAttr(void) {
       mIsUpdateGrpAttr = false;
    }
 protected:
    virtual void init() = 0;
    virtual void deInit();
    void waitSignal(rk_aiq_uapi_mode_sync_e sync = RK_AIQ_UAPI_MODE_DEFAULT);
    void sendSignal(rk_aiq_uapi_mode_sync_e sync = RK_AIQ_UAPI_MODE_DEFAULT);
    enum {
        RKAIQ_CONFIG_COM_PREPARE,
        RKAIQ_CONFIG_COM_PRE,
        RKAIQ_CONFIG_COM_PROC,
        RKAIQ_CONFIG_COM_POST,
    };
    virtual XCamReturn configInparamsCom(RkAiqAlgoCom* com, int type);
    inline uint64_t grpId2GrpMask(uint32_t grpId) {
        return grpId == RK_AIQ_CORE_ANALYZE_ALL ? (uint64_t)grpId : (1ULL << grpId);
    }
    RkAiqAlgoCom* mConfig;
    RkAiqAlgoCom* mPreInParam;
    RkAiqAlgoResCom* mPreOutParam;
    RkAiqAlgoCom* mProcInParam;
    RkAiqAlgoResCom* mProcOutParam;
    RkAiqAlgoCom* mPostInParam;
    RkAiqAlgoResCom* mPostOutParam;
    const RkAiqAlgoDesComm* mDes;
    RkAiqAlgoContext* mAlgoCtx;
    RkAiqCore* mAiqCore;
    bool mEnable;
    bool mReConfig;
    uint32_t mGroupId;
    void* mAlogsGroupSharedParams;
    XCam::Mutex mCfgMutex;
    mutable std::atomic<bool> updateAtt;
    XCam::Cond mUpdateCond;
    RkAiqHandle* mNextHdl;
    RkAiqHandle* mParentHdl;
    bool mIsMulRun;
    bool mPostShared;
    uint32_t mSyncFlag{(uint32_t)(-1)};
    bool mIsUpdateGrpAttr;
};

template <typename T>
RkAiqHandle* createT(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore) {
    return new T(des, aiqCore);
}

struct RkAiqHandleFactory {
    typedef std::map<std::string, RkAiqHandle* (*)(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore)>
        map_type;

    ~RkAiqHandleFactory() {
        if (map != nullptr) {
            if (map->empty()) {
                delete map;
            }
        }
    }

    static RkAiqHandle* createInstance(std::string const& s, RkAiqAlgoDesComm* des,
                                       RkAiqCore* aiqCore) {
        map_type::iterator it = getMap()->find(s);
        if (it == getMap()->end()) return 0;
        return it->second(des, aiqCore);
    }

 protected:
    static map_type* getMap() {
        // never delete'ed. (exist until program termination)
        // because we can't guarantee correct destruction order
        if (!map) {
            map = new map_type;
        }
        return map;
    }

 private:
    static map_type* map;
};

template <typename T>
struct RkAiqHandleRegister : RkAiqHandleFactory {
    RkAiqHandleRegister(std::string const& s) : s_(s) { getMap()->insert(std::make_pair(s, &createT<T>)); }
    ~RkAiqHandleRegister() { getMap()->erase(s_); }
private:
    const std::string s_;
};

#define DECLARE_HANDLE_REGISTER_TYPE(NAME) static RkAiqHandleRegister<NAME> reg

#define DEFINE_HANDLE_REGISTER_TYPE(NAME) RkAiqHandleRegister<NAME> NAME::reg(#NAME)

}  // namespace RkCam

#endif
