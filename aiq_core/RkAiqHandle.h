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

#include "rk_aiq_algo_types.h"
#include "rk_aiq_types.h"

namespace RkCam {

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
    RkAiqAlgoContext* getAlgoCtx() { return mAlgoCtx; }
    int getAlgoId() { return mDes->id; }
    int getAlgoType() { return mDes->type; }
    void setGroupId(int32_t gId) {
        mGroupId = gId;
    }
    int32_t getGroupId() {
       return mGroupId;
    }
    void setGroupShared(void* grp_shared) {
        mAlogsGroupSharedParams = grp_shared;
    }
    void* getGroupShared() {
       return mAlogsGroupSharedParams;
    }
    virtual XCamReturn updateConfig(bool needSync) { return XCAM_RETURN_NO_ERROR; };

 protected:
    virtual void init() = 0;
    virtual void deInit();
    enum {
        RKAIQ_CONFIG_COM_PREPARE,
        RKAIQ_CONFIG_COM_PRE,
        RKAIQ_CONFIG_COM_PROC,
        RKAIQ_CONFIG_COM_POST,
    };
    virtual XCamReturn configInparamsCom(RkAiqAlgoCom* com, int type);
    RkAiqAlgoCom* mConfig;
    RkAiqAlgoCom* mPreInParam;
    RkAiqAlgoResCom* mPreOutParam;
    RkAiqAlgoCom* mProcInParam;
    RkAiqAlgoResCom* mProcOutParam;
    RkAiqAlgoCom* mPostInParam;
    RkAiqAlgoResCom* mPostOutParam;
    RkAiqAlgoDesComm* mDes;
    RkAiqAlgoContext* mAlgoCtx;
    RkAiqCore* mAiqCore;
    bool mEnable;
    bool mReConfig;
    uint32_t mGroupId;
    void* mAlogsGroupSharedParams;
};

#define RKAIQHANDLE(algo)                                                       \
    class RkAiq##algo##Handle : virtual public RkAiqHandle {                    \
     public:                                                                    \
        explicit RkAiq##algo##Handle(RkAiqAlgoDesComm* des, RkAiqCore* aiqCore) \
            : RkAiqHandle(des, aiqCore){};                                      \
        virtual ~RkAiq##algo##Handle() { deInit(); };                           \
        virtual XCamReturn prepare();                                           \
        virtual XCamReturn preProcess();                                        \
        virtual XCamReturn processing();                                        \
        virtual XCamReturn postProcess();                                       \
                                                                                \
     protected:                                                                 \
        virtual void init();                                                    \
        virtual void deInit() { RkAiqHandle::deInit(); };                       \
    }

// define
RKAIQHANDLE(Ae);
RKAIQHANDLE(Awb);
RKAIQHANDLE(Af);
RKAIQHANDLE(Amerge);
RKAIQHANDLE(Atmo);
RKAIQHANDLE(Anr);
RKAIQHANDLE(Alsc);
RKAIQHANDLE(Asharp);
RKAIQHANDLE(Adhaz);
RKAIQHANDLE(Asd);
RKAIQHANDLE(Acp);
RKAIQHANDLE(A3dlut);
RKAIQHANDLE(Ablc);
RKAIQHANDLE(Accm);
RKAIQHANDLE(Acgc);
RKAIQHANDLE(Adebayer);
RKAIQHANDLE(Adpcc);
RKAIQHANDLE(Afec);
RKAIQHANDLE(Agamma);
RKAIQHANDLE(Adegamma);
RKAIQHANDLE(Agic);
RKAIQHANDLE(Aie);
RKAIQHANDLE(Aldch);
RKAIQHANDLE(Ar2y);
RKAIQHANDLE(Awdr);
RKAIQHANDLE(Aorb);
RKAIQHANDLE(Amfnr);
RKAIQHANDLE(Aynr);
RKAIQHANDLE(Acnr);
RKAIQHANDLE(Arawnr);
RKAIQHANDLE(Adrc);
RKAIQHANDLE(AdrcV21);
RKAIQHANDLE(Aeis);
RKAIQHANDLE(Amd);
RKAIQHANDLE(Again);

};  // namespace RkCam

#endif
