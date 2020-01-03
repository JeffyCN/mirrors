/*
 * Copyright (c) 2018, Fuzhou Rockchip Electronics Co., Ltd
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

#ifndef __RKCAMERA_VENDOR_TAGS_H__
#define __RKCAMERA_VENDOR_TAGS_H__ 

// Camera dependencies
#include "system/camera_metadata.h"
#include "system/camera_vendor_tags.h"

enum rkcamera3_ext_section {
    RKCAMERA3_PRIVATEDATA = VENDOR_SECTION,
    RKCAMERA3_EXT_SECTION_END
};

enum rkcamera3_ext_section_ranges {
    RKCAMERA3_PRIVATEDATA_START = RKCAMERA3_PRIVATEDATA << 16,
};

enum rkcamera3_ext_tags {
    RKCAMERA3_PRIVATEDATA_EFFECTIVE_DRIVER_FRAME_ID = RKCAMERA3_PRIVATEDATA_START,
    RKCAMERA3_PRIVATEDATA_FRAME_SOF_TIMESTAMP,
    RKCAMERA3_PRIVATEDATA_STILLCAP_SYNC_NEEDED,
    RKCAMERA3_PRIVATEDATA_STILLCAP_SYNC_CMD,
    RKCAMERA3_PRIVATEDATA_STILLCAP_ISP_PARAM,
    RKCAMERA3_PRIVATEDATA_ISP_BLS,
    RKCAMERA3_PRIVATEDATA_ISP_LSC_GET,
    RKCAMERA3_PRIVATEDATA_ISP_CCM_GET,
    RKCAMERA3_PRIVATEDATA_ISP_AWB_GET,
    RKCAMERA3_PRIVATEDATA_ISP_AWB_WP,
    RKCAMERA3_PRIVATEDATA_ISP_AWB_CURV,
    RKCAMERA3_PRIVATEDATA_ISP_AWB_REFGAIN,
    RKCAMERA3_PRIVATEDATA_ISP_GOC_NORMAL,
    RKCAMERA3_PRIVATEDATA_ISP_GOC_NIGHT,
    RKCAMERA3_PRIVATEDATA_ISP_CPROC_PREVIEW,
    RKCAMERA3_PRIVATEDATA_ISP_CPROC_CAPTURE,
    RKCAMERA3_PRIVATEDATA_ISP_CPROC_VIDEO,
    RKCAMERA3_PRIVATEDATA_ISP_DPF_GET,
    RKCAMERA3_PRIVATEDATA_ISP_FLT_NORMAL,
    RKCAMERA3_PRIVATEDATA_ISP_FLT_NIGHT,
    RKCAMERA3_PRIVATEDATA_ISP_SENSOR_INFO,
    RKCAMERA3_PRIVATEDATA_ISP_MODULE_INFO,
    RKCAMERA3_PRIVATEDATA_ISP_SENSOR_REG,
    RKCAMERA3_PRIVATEDATA_ISP_SENSOR_FLIP,
    RKCAMERA3_PRIVATEDATA_ISP_SYSTEM_INFO,
    RKCAMERA3_PRIVATEDATA_ISP_BLS_SET,
    RKCAMERA3_PRIVATEDATA_ISP_LSC_SET,
    RKCAMERA3_PRIVATEDATA_ISP_CCM_SET,
    RKCAMERA3_PRIVATEDATA_ISP_AWB_SET,
    RKCAMERA3_PRIVATEDATA_ISP_AWB_WP_SET,
    RKCAMERA3_PRIVATEDATA_ISP_AWB_CURV_SET,
    RKCAMERA3_PRIVATEDATA_ISP_AWB_REFGAIN_SET,
    RKCAMERA3_PRIVATEDATA_ISP_GOC_SET,
    RKCAMERA3_PRIVATEDATA_ISP_CPROC_SET,
    RKCAMERA3_PRIVATEDATA_ISP_DPF_SET,
    RKCAMERA3_PRIVATEDATA_ISP_FLT_SET,
    RKCAMERA3_PRIVATEDATA_ISP_RESTART,
    RKCAMERA3_PRIVATEDATA_ISP_PROTOCOL_INFO,
    RKCAMERA3_PRIVATEDATA_TUNING_FLAG,
    //RKCAMERA3_PRIVATEDATA_HIST_BINS,
    RKCAMERA3_PRIVATEDATA_EXP_MEANS,
    RKCAMERA3_PRIVATEDATA_EXP_MEANS_COUNT,
    RKCAMERA3_PRIVATEDATA_END,
};

// RKCAMERA3_PRIVATEDATA_STILLCAP_SYNC_CMD
typedef enum rkcamera3_privatemeta_enum_stillcap_sync_cmd {
    RKCAMERA3_PRIVATEDATA_STILLCAP_SYNC_CMD_SYNCSTART = 1,
    RKCAMERA3_PRIVATEDATA_STILLCAP_SYNC_CMD_SYNCDONE,
    RKCAMERA3_PRIVATEDATA_STILLCAP_SYNC_CMD_SYNCEND,
} rkcamera3_privatemeta_enum_stillcap_sync_cmd_t;

class RkCamera3VendorTags {
    public:
        static void get_vendor_tag_ops(vendor_tag_ops_t* ops);
        static int get_tag_count(const vendor_tag_ops_t *ops);
        static void get_all_tags(const vendor_tag_ops_t *ops, uint32_t *tag_array);
        static const char* get_section_name(const vendor_tag_ops_t *ops, uint32_t tag);
        static const char* get_tag_name(const vendor_tag_ops_t *ops, uint32_t tag);
        static int get_tag_type(const vendor_tag_ops_t *ops, uint32_t tag);
        static const vendor_tag_ops_t *Ops;
};

#endif /* __RKCAMERA_VENDOR_TAGS_H__ */
