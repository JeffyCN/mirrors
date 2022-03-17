
/*
 * RkAiqSharedDataWrapper.h
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

#ifndef _RK_AIQ_SHARED_DATA_MANAGER_H_
#define _RK_AIQ_SHARED_DATA_MANAGER_H_

#include <pthread.h>

#include <list>
#include <map>

#include "rk_aiq_algo_types_int.h"
#include "rk_aiq_pool.h"
#include "thumbnails.h"

namespace RkCam {

typedef SharedItemBase sharedData;
typedef std::list<SmartPtr<sharedData>> sharedDataList;
typedef std::map<int32_t, sharedDataList> sharedDataMap;

typedef struct RkAiqSofInfoWrapper_s : public SharedItemData {
    uint32_t sequence;
    SmartPtr<RkAiqExpParamsProxy> preExp;
    SmartPtr<RkAiqExpParamsProxy> curExp;
    int64_t sof;
} RkAiqSofInfoWrapper_t;

typedef SharedItemPool<class RkAiqIspStats>                 RkAiqIspStatsIntPool;
typedef SharedItemProxy<class RkAiqIspStats>                RkAiqIspStatsIntProxy;

typedef SharedItemPool<class RkAiqAecStats>                 RkAiqAecStatsPool;
typedef SharedItemProxy<class RkAiqAecStats>                RkAiqAecStatsProxy;

typedef SharedItemPool<class RkAiqAwbStats>                 RkAiqAwbStatsPool;
typedef SharedItemProxy<class RkAiqAwbStats>                RkAiqAwbStatsProxy;

typedef SharedItemPool<class RkAiqAtmoStats>                RkAiqAtmoStatsPool;
typedef SharedItemProxy<class RkAiqAtmoStats>               RkAiqAtmoStatsProxy;

typedef SharedItemPool<class RkAiqAdehazeStats>             RkAiqAdehazeStatsPool;
typedef SharedItemProxy<class RkAiqAdehazeStats>            RkAiqAdehazeStatsProxy;

typedef SharedItemPool<class RkAiqAfStats>                  RkAiqAfStatsPool;
typedef SharedItemProxy<class RkAiqAfStats>                 RkAiqAfStatsProxy;

typedef SharedItemPool<RkAiqSofInfoWrapper_t>               RkAiqSofInfoWrapperPool;
typedef SharedItemProxy<RkAiqSofInfoWrapper_t>              RkAiqSofInfoWrapperProxy;

typedef SharedItemPool<RkAiqOrbStats>                       RkAiqOrbStatsPool;
typedef SharedItemProxy<RkAiqOrbStats>                      RkAiqOrbStatsProxy;

typedef struct RkAiqThumbnailSrcWrapper_s : public SharedItemData {
    SmartPtr<rkaiq_image_source_t>     thumbnail;
} RkAiqThumbnailSrcWrapper_t;
typedef SharedItemPool<RkAiqThumbnailSrcWrapper_t>           RkAiqThumbnailSrcPool;
typedef SharedItemProxy<RkAiqThumbnailSrcWrapper_t>          RkAiqThumbnailSrcProxy;

typedef SharedItemPool<AecProcResult_t>                     RkAiqAeProcResultPool;
typedef SharedItemProxy<AecProcResult_t>                    RkAiqAeProcResultProxy;

class RkAiqSharedDataManager
{
public:
     RkAiqSharedDataManager() {
         rwlock = PTHREAD_RWLOCK_INITIALIZER;
         if (pthread_rwlock_init(&rwlock, nullptr))
             LOGE_ANALYZER("rwlock init failed!");
     };
     ~RkAiqSharedDataManager() {
         if (pthread_rwlock_destroy(&rwlock))
             LOGE_ANALYZER("rwlock destory failed!");
    };

    bool push (const SmartPtr<sharedData> &data) {
        pthread_rwlock_wrlock(&rwlock);
        sharedDataList &list = mSharedDataMap[data->getId()];
        list.push_back(data);
        pthread_rwlock_unlock(&rwlock);

        LOGD_ANALYZER("%s, sharedDatamap size %d, id(%d) push type: %d, list size: %d",
                      __FUNCTION__, mSharedDataMap.size(),
                      data->getId(), data->getType(), list.size());
        return true;
    };

    bool pop() {
        pthread_rwlock_wrlock(&rwlock);
        if (!mSharedDataMap.empty()) {
            auto list = mSharedDataMap.begin()->second;

            LOGD_ANALYZER("%s, sharedDatamap size %d, pop id(%d), list size: %d",
                    __FUNCTION__, mSharedDataMap.size(),
                    mSharedDataMap.begin()->first,
                    list.size());
            list.clear();
            mSharedDataMap.erase(mSharedDataMap.begin());
        }
        pthread_rwlock_unlock(&rwlock);

        return true;
    };

    bool find (int32_t id, RkAiqSharedDataType type, SmartPtr<sharedData> &data) {
        pthread_rwlock_rdlock(&rwlock);
        auto mapIt = mSharedDataMap.find(id);
        if (mapIt == mSharedDataMap.end()) {
            LOGE_ANALYZER("can't find id(%d) in mSharedDataMap");
            pthread_rwlock_unlock(&rwlock);
            return false;
        }

        auto list = mapIt->second;
        auto listIt = list.begin();
        for (; listIt != list.end(); ++listIt) {
            if (type == listIt->ptr()->getType()) {
                data = *listIt;
                break;
            }
        }

        if (listIt == list.end()) {
            LOGE_ANALYZER("can't find type(%d) data in list", type);
            pthread_rwlock_unlock(&rwlock);
            return false;
        }

        pthread_rwlock_unlock(&rwlock);

        return true;
    };

    uint32_t clear () {
        pthread_rwlock_wrlock(&rwlock);
        for (auto mapIt = mSharedDataMap.begin(); mapIt != mSharedDataMap.end();) {
            auto list = mapIt->second;
            list.clear();
            mSharedDataMap.erase(mapIt++);
        }
        pthread_rwlock_unlock(&rwlock);

        return true;
    };

    uint32_t get_size () {
        return mSharedDataMap.size();
    };

    bool get_exposure_params(int32_t id, RKAiqAecExpInfo_t **exp_param);
    bool get_ae_stats(int32_t id, rk_aiq_isp_aec_stats_t *aec_stats);
    bool get_awb_stats(int32_t id, rk_aiq_awb_stat_res_v200_t *awb_stats);
    bool get_af_stats(int32_t id, rk_aiq_isp_af_stats_t *af_stats);

protected:

private:
    sharedDataMap mSharedDataMap;
    pthread_rwlock_t rwlock;
};

template<class T>
struct AlgoRstShared: VideoBuffer {
    T   result;
    virtual uint8_t *map ();
    virtual bool unmap ();
    virtual int get_fd ();
};

template<class T>
uint8_t *AlgoRstShared<T>::map() {
   return (uint8_t *)(&result);
}

template<class T>
bool AlgoRstShared<T>::unmap() {
   return true;
}

template<class T>
int AlgoRstShared<T>::get_fd ()
{
    return -1;
}

using RkAiqAlgoPreResAeIntShared = AlgoRstShared<RkAiqAlgoPreResAeInt>;
using RkAiqAlgoProcResAeIntShared = AlgoRstShared<RkAiqAlgoProcResAeInt>;
using RkAiqAlgoProcResAwbIntShared = AlgoRstShared<RkAiqAlgoProcResAwbInt>;
using RkAiqAlgoProcResAfIntShared = AlgoRstShared<RkAiqAlgoProcResAfInt>;
using RkAiqAlgoProcResAmdIntShared = AlgoRstShared<RkAiqAlgoProcResAmdInt>;

};

#endif // _RK_AIQ_SHARED_DATA_MANAGER_H_
