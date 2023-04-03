/*
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

#ifndef _FLASH_LIGHT_H_
#define _FLASH_LIGHT_H_

#include "v4l2_device.h"
#include "rk_aiq_types.h"
#include <string>

using namespace XCam;

namespace RkCam {

#define FLASH_MAX_NUM 2
#define FL_SUBM (0x8)

class FlashLightHw {
public:
    explicit FlashLightHw(std::string name[], int num);
    virtual ~FlashLightHw();

    XCamReturn init(int active_num);
    XCamReturn deinit();
    XCamReturn start();
    void keep_status(bool ks) {
        _keep_status = ks;
    };
    XCamReturn stop();
    bool isStrengthAdj() {
        return  _v4l_flash_info[0].fl_strth_adj_enable || _v4l_flash_info[0].tc_strth_adj_enable;}
    XCamReturn set_params(rk_aiq_flash_setting_t& flash_settings);
    XCamReturn get_status (rk_aiq_flash_setting_t& flash_settings, uint32_t frame_id = -1);
private:
    int get_flash_info ();
    XCamReturn v4l_set_params(int fl_mode, float fl_intensity[], int fl_timeout, int fl_on);
private:
    XCAM_DEAD_COPY (FlashLightHw);
    int _dev_num;
    int _active_fl_num;
    SmartPtr<V4l2SubDevice> _fl_device[FLASH_MAX_NUM];
    rk_aiq_flash_setting_t _flash_settings;

    enum RK_AIQ_V4L_FLASH_QUERY_TYPE_E {
       RK_AIQ_V4L_FLASH_QUERY_TYPE_E_MIN,
       RK_AIQ_V4L_FLASH_QUERY_TYPE_E_MAX,
       RK_AIQ_V4L_FLASH_QUERY_TYPE_E_DEFAULT,
       RK_AIQ_V4L_FLASH_QUERY_TYPE_E_STEP,
       RK_AIQ_V4L_FLASH_QUERY_TYPE_E_LAST,
    };

    struct rk_aiq_v4l_flash_info_s {
      // [min, max, default, step]
      int torch_power_info[RK_AIQ_V4L_FLASH_QUERY_TYPE_E_LAST];
      int flash_power_info[RK_AIQ_V4L_FLASH_QUERY_TYPE_E_LAST];
      bool fl_strth_adj_enable;
      bool tc_strth_adj_enable;
    };

    struct rk_aiq_v4l_flash_info_s _v4l_flash_info[FLASH_MAX_NUM];
    bool _keep_status;
};

} //namespace RkCam
#endif // _FLASH_LIGHT_H_
