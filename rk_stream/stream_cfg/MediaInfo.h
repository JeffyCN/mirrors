/*
 *  Copyright (c) 2021 Rockchip Corporation
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
#ifndef _MEDIA_INFO_H_
#define _MEDIA_INFO_H_

#include <v4l2_device.h>
#include <map>
#include "mediactl/mediactl-priv.h"
#include <linux/v4l2-subdev.h>
#include "xcam_thread.h"
#include "xcam_log.h"
#include "Stream.h"
#include <time.h>
#include <unistd.h>

using namespace XCam;

namespace RkRawStream {

int8_t pixFmt2Bpp(uint32_t pixFmt);


class MediaInfo
{
public:
    MediaInfo();
    virtual ~MediaInfo();
    rk_aiq_isp_hw_info_t mIspHwInfos;
    rk_aiq_cif_hw_info_t mCifHwInfos;
    std::map<std::string, SmartPtr<rk_aiq_static_info_t>> mCamHwInfos;
    std::map<std::string, SmartPtr<rk_sensor_full_info_t>> mSensorHwInfos;
    bool mIsMultiIspMode;
    uint16_t mMultiIspExtendedPixel;
    void findAttachedSubdevs(struct media_device *device, uint32_t count, rk_sensor_full_info_t *s_info);
    XCamReturn initCamHwInfos();
    rk_aiq_static_info_t* getStaticCamHwInfo(const char* sns_ent_name, uint16_t index = 0);
    XCamReturn clearStaticCamHwInfo();
    void getCamHwEntNames(char buf[12][32]);
    rk_sensor_full_info_t* getSensorFullInfo(char* sns_ent_name, uint16_t index);
    rk_aiq_isp_t* getIspInfo(char* isp_driver_name);
    const char* offline(int isp_index, const char* offline_sns_ent_name);
    XCamReturn setupOffLineLink(const char *media_path, bool enable, int hdr_mode);
    XCamReturn ispDevReUpdateHwStatus(const char *dev_path);
};

}
#endif
