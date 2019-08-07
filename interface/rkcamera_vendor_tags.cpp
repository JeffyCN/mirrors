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

#define LOG_TAG "rkcamera3VendorTags"

// Camera dependencies
#include "rkcamera_vendor_tags.h"
#include <base/xcam_log.h>

enum rkcamera3_ext_tags rkcamera3_ext3_section_bounds[RKCAMERA3_EXT_SECTION_END -
    VENDOR_SECTION] = {
        RKCAMERA3_PRIVATEDATA_END,
};

typedef struct vendor_tag_info {
    const char *tag_name;
    uint8_t     tag_type;
} vendor_tag_info_t;

const char *rkcamera3_ext_section_names[RKCAMERA3_EXT_SECTION_END -
        VENDOR_SECTION] = {
    "org.codeaurora.rkcamera3.privatedata",
};

vendor_tag_info_t rkcamera3_privatedata[RKCAMERA3_PRIVATEDATA_END - RKCAMERA3_PRIVATEDATA_START] = {
    { "privatedata_effective_driver_frame_id", TYPE_INT64 },
    { "privatedata_frame_sof_timestamp", TYPE_INT64 },
    { "privatedata_stillcap_sync_needed", TYPE_BYTE },
    { "privatedata_stillcap_sync_cmd", TYPE_BYTE },
    { "privatedata_stillcap_isp_param", TYPE_BYTE },
    { "privatedata_isp_bls", TYPE_BYTE },
    { "privatedata_isp_lsc_get", TYPE_BYTE},
    { "privatedata_isp_ccm_get", TYPE_BYTE},
    { "privatedata_isp_awb_get", TYPE_BYTE},
    { "privatedata_isp_awb_wp", TYPE_BYTE},
    { "privatedata_isp_awb_curv", TYPE_BYTE},
    { "privatedata_isp_awb_refgain", TYPE_BYTE},
    { "privatedata_isp_goc_normal", TYPE_BYTE },
    { "privatedata_isp_goc_night", TYPE_BYTE },
    { "privatedata_isp_cproc_preview", TYPE_BYTE },
    { "privatedata_isp_cproc_capture", TYPE_BYTE },
    { "privatedata_isp_cproc_video", TYPE_BYTE },
    { "privatedata_isp_dpf_get", TYPE_BYTE },
    { "privatedata_isp_flt_normal", TYPE_BYTE },
    { "privatedata_isp_flt_night", TYPE_BYTE },
    { "privatedata_isp_sensor_info", TYPE_BYTE },
    { "privatedata_isp_module_info", TYPE_BYTE },
    { "privatedata_isp_sensor_reg", TYPE_BYTE },
    { "privatedata_isp_sensor_flip", TYPE_BYTE },
    { "privatedata_isp_system_info", TYPE_BYTE },
    { "privatedata_isp_bls_set", TYPE_BYTE },
    { "privatedata_isp_lsc_set", TYPE_BYTE },
    { "privatedata_isp_ccm_set", TYPE_BYTE },
    { "privatedata_isp_awb_set", TYPE_BYTE },
    { "privatedata_isp_awb_wp_set", TYPE_BYTE },
    { "privatedata_isp_awb_curv_set", TYPE_BYTE },
    { "privatedata_isp_awb_refgain_set", TYPE_BYTE },
    { "privatedata_isp_goc_set", TYPE_BYTE },
    { "privatedata_isp_cproc_set", TYPE_BYTE },
    { "privatedata_isp_dpf_set", TYPE_BYTE },
    { "privatedata_isp_flt_set", TYPE_BYTE },
    { "privatedata_isp_restart", TYPE_BYTE },
    { "privatedata_isp_protocol_info", TYPE_BYTE },
    { "privatedata_tuning_flag", TYPE_BYTE },
};

vendor_tag_info_t *rkcamera3_tag_info[RKCAMERA3_EXT_SECTION_END -
        VENDOR_SECTION] = {
    rkcamera3_privatedata,
};

uint32_t rkcamera3_all_tags[] = {
    // rkcamera3_PRIVATEDATA
    (uint32_t)RKCAMERA3_PRIVATEDATA_EFFECTIVE_DRIVER_FRAME_ID,
    (uint32_t)RKCAMERA3_PRIVATEDATA_FRAME_SOF_TIMESTAMP,
    (uint32_t)RKCAMERA3_PRIVATEDATA_STILLCAP_SYNC_NEEDED,
    (uint32_t)RKCAMERA3_PRIVATEDATA_STILLCAP_SYNC_CMD,
    (uint32_t)RKCAMERA3_PRIVATEDATA_STILLCAP_ISP_PARAM,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_BLS,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_LSC_GET,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_CCM_GET,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_AWB_GET,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_AWB_WP,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_AWB_CURV,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_AWB_REFGAIN,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_GOC_NORMAL,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_GOC_NIGHT,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_CPROC_PREVIEW,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_CPROC_CAPTURE,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_CPROC_VIDEO,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_DPF_GET,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_FLT_NORMAL,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_FLT_NIGHT,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_SENSOR_INFO,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_MODULE_INFO,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_SENSOR_REG,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_SENSOR_FLIP,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_SYSTEM_INFO,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_BLS_SET,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_LSC_SET,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_CCM_SET,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_AWB_SET,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_AWB_WP_SET,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_AWB_CURV_SET,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_AWB_REFGAIN_SET,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_GOC_SET,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_CPROC_SET,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_DPF_SET,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_FLT_SET,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_RESTART,
    (uint32_t)RKCAMERA3_PRIVATEDATA_ISP_PROTOCOL_INFO,
    (uint32_t)RKCAMERA3_PRIVATEDATA_TUNING_FLAG,
};

const vendor_tag_ops_t* RkCamera3VendorTags::Ops = NULL;

/*===========================================================================
 * FUNCTION   : get_vendor_tag_ops
 *
 * DESCRIPTION: Get the metadata vendor tag function pointers
 *
 * PARAMETERS :
 *    @ops   : function pointer table to be filled by HAL
 *
 *
 * RETURN     : NONE
 *==========================================================================*/
void RkCamera3VendorTags::get_vendor_tag_ops(
                                vendor_tag_ops_t* ops)
{
    LOGD("E");

    Ops = ops;

    ops->get_tag_count = get_tag_count;
    ops->get_all_tags = get_all_tags;
    ops->get_section_name = get_section_name;
    ops->get_tag_name = get_tag_name;
    ops->get_tag_type = get_tag_type;
    ops->reserved[0] = NULL;

    LOGD("X");
    return;
}

/*===========================================================================
 * FUNCTION   : get_tag_count
 *
 * DESCRIPTION: Get number of vendor tags supported
 *
 * PARAMETERS :
 *    @ops   :  Vendor tag ops data structure
 *
 *
 * RETURN     : Number of vendor tags supported
 *==========================================================================*/

int RkCamera3VendorTags::get_tag_count(
                const vendor_tag_ops_t * ops)
{
    size_t count = 0;
    if (ops == Ops)
        count = sizeof(rkcamera3_all_tags)/sizeof(rkcamera3_all_tags[0]);

    LOGD("count is %d", count);
    return (int)count;
}

/*===========================================================================
 * FUNCTION   : get_all_tags
 *
 * DESCRIPTION: Fill array with all supported vendor tags
 *
 * PARAMETERS :
 *    @ops      :  Vendor tag ops data structure
 *    @tag_array:  array of metadata tags
 *
 * RETURN     : Success: the section name of the specific tag
 *              Failure: NULL
 *==========================================================================*/
void RkCamera3VendorTags::get_all_tags(
                const vendor_tag_ops_t * ops,
                uint32_t *g_array)
{
    if (ops != Ops)
        return;

    for (size_t i = 0;
            i < sizeof(rkcamera3_all_tags)/sizeof(rkcamera3_all_tags[0]);
            i++) {
        g_array[i] = rkcamera3_all_tags[i];
        LOGD("g_array[%d] is %d", i, g_array[i]);
    }
}

/*===========================================================================
 * FUNCTION   : get_section_name
 *
 * DESCRIPTION: Get section name for vendor tag
 *
 * PARAMETERS :
 *    @ops   :  Vendor tag ops structure
 *    @tag   :  Vendor specific tag
 *
 *
 * RETURN     : Success: the section name of the specific tag
 *              Failure: NULL
 *==========================================================================*/

const char* RkCamera3VendorTags::get_section_name(
                const vendor_tag_ops_t * ops,
                uint32_t tag)
{
    LOGD("E");
    if (ops != Ops)
        return NULL;

    const char *ret;
    uint32_t section = tag >> 16;

    if (section >= VENDOR_SECTION && section < RKCAMERA3_EXT_SECTION_END)
        ret = rkcamera3_ext_section_names[section - VENDOR_SECTION];
    else
        ret = NULL;

    if (ret)
        LOGD("section_name[%d] is %s", tag, ret);
    LOGD("X");
    return ret;
}

/*===========================================================================
 * FUNCTION   : get_tag_name
 *
 * DESCRIPTION: Get name of a vendor specific tag
 *
 * PARAMETERS :
 *    @tag   :  Vendor specific tag
 *
 *
 * RETURN     : Success: the name of the specific tag
 *              Failure: NULL
 *==========================================================================*/
const char* RkCamera3VendorTags::get_tag_name(
                const vendor_tag_ops_t * ops,
                uint32_t tag)
{
    LOGD("E");
    const char *ret;
    uint32_t section = tag >> 16;
    uint32_t section_index = section - VENDOR_SECTION;
    uint32_t tag_index = tag & 0xFFFF;

    if (ops != Ops) {
        ret = NULL;
        goto done;
    }

    if (section >= VENDOR_SECTION && section < RKCAMERA3_EXT_SECTION_END &&
        tag < (uint32_t)rkcamera3_ext3_section_bounds[section_index])
        ret = rkcamera3_tag_info[section_index][tag_index].tag_name;
    else
        ret = NULL;

    if (ret)
        LOGD("tag name for tag %d is %s", tag, ret);
    LOGD("X");

done:
    return ret;
}

/*===========================================================================
 * FUNCTION   : get_tag_type
 *
 * DESCRIPTION: Get type of a vendor specific tag
 *
 * PARAMETERS :
 *    @tag   :  Vendor specific tag
 *
 *
 * RETURN     : Success: the type of the specific tag
 *              Failure: -1
 *==========================================================================*/
int RkCamera3VendorTags::get_tag_type(
                const vendor_tag_ops_t *ops,
                uint32_t tag)
{
    //LOGD("E");
    int ret;
    uint32_t section = tag >> 16;
    uint32_t section_index = section - VENDOR_SECTION;
    uint32_t tag_index = tag & 0xFFFF;

    if (ops != Ops) {
        ret = -1;
        goto done;
    }
    if (section >= VENDOR_SECTION && section < RKCAMERA3_EXT_SECTION_END &&
        tag < (uint32_t)rkcamera3_ext3_section_bounds[section_index])
        ret = rkcamera3_tag_info[section_index][tag_index].tag_type;
    else
        ret = -1;

    LOGD("tag type for tag %u is %d", tag, ret);
    //LOGD("X");
done:
    return ret;
}
