/*
 *   Copyright (c) 2024 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef _AIQ_NOTIFIER_H_
#define _AIQ_NOTIFIER_H_

#include "common/list.h"
#include "dumpcam_server/info/include/st_string.h"

#define _MODS_NUM_OFFSET 0x20
#define _MODS_COUNT      (AIQ_NOTIFIER_MATCH_ALL - _MODS_NUM_OFFSET)

/**
 * enum v4l2_async_match_type - type of asynchronous subdevice logic to be used
 *  in order to identify a match
 *
 * @V4L2_ASYNC_MATCH_CUSTOM: Match will use the logic provided by &struct
 *  v4l2_async_subdev.match ops
 * @V4L2_ASYNC_MATCH_DEVNAME: Match will use the device name
 * @V4L2_ASYNC_MATCH_I2C: Match will check for I2C adapter ID and address
 * @V4L2_ASYNC_MATCH_FWNODE: Match will use firmware node
 *
 * This enum is used by the asyncrhronous sub-device logic to define the
 * algorithm that will be used to match an asynchronous device.
 */
enum aiq_notifier_match_type {
    // HWI
    AIQ_NOTIFIER_MATCH_HWI_BASE = 0,
    AIQ_NOTIFIER_MATCH_HWI_STREAM_CAP,
    AIQ_NOTIFIER_MATCH_HWI_STREAM_PROC,
    AIQ_NOTIFIER_MATCH_HWI_SENSOR,
    AIQ_NOTIFIER_MATCH_HWI_ISP_PARAMS,
    AIQ_NOTIFIER_MATCH_HWI_ISP_ACTIVE_PARAMS,
    AIQ_NOTIFIER_MATCH_HWI_ALL,

    // CORE
    AIQ_NOTIFIER_MATCH_CORE,
    AIQ_NOTIFIER_MATCH_CORE_BUF_MGR,
    AIQ_NOTIFIER_MATCH_CORE_GRP_ANALYZER,
    AIQ_NOTIFIER_MATCH_CORE_ISP_PARAMS,
    AIQ_NOTIFIER_MATCH_CORE_ALL,

    // mods
    AIQ_NOTIFIER_MATCH_AEC = _MODS_NUM_OFFSET,
    AIQ_NOTIFIER_MATCH_HIST,
    AIQ_NOTIFIER_MATCH_AWB,
    AIQ_NOTIFIER_MATCH_AWBGAIN,
    AIQ_NOTIFIER_MATCH_AF,
    AIQ_NOTIFIER_MATCH_DPCC,
    AIQ_NOTIFIER_MATCH_MERGE,
    AIQ_NOTIFIER_MATCH_CCM,
    AIQ_NOTIFIER_MATCH_LSC,
    AIQ_NOTIFIER_MATCH_BLC,
    AIQ_NOTIFIER_MATCH_RAWNR,
    AIQ_NOTIFIER_MATCH_GIC,
    AIQ_NOTIFIER_MATCH_DEBAYER,
    AIQ_NOTIFIER_MATCH_LUT3D,
    AIQ_NOTIFIER_MATCH_DEHAZE,
    AIQ_NOTIFIER_MATCH_AGAMMA,
    AIQ_NOTIFIER_MATCH_ADEGAMMA,
    AIQ_NOTIFIER_MATCH_CSM,
    AIQ_NOTIFIER_MATCH_CGC,
    AIQ_NOTIFIER_MATCH_GAIN,
    AIQ_NOTIFIER_MATCH_CP,
    AIQ_NOTIFIER_MATCH_IE,
    AIQ_NOTIFIER_MATCH_TNR,
    AIQ_NOTIFIER_MATCH_YNR,
    AIQ_NOTIFIER_MATCH_CNR,
    AIQ_NOTIFIER_MATCH_SHARPEN,
    AIQ_NOTIFIER_MATCH_DRC,
    AIQ_NOTIFIER_MATCH_CAC,
    AIQ_NOTIFIER_MATCH_AFD,
    AIQ_NOTIFIER_MATCH_RGBIR,
    AIQ_NOTIFIER_MATCH_LDC,
    AIQ_NOTIFIER_MATCH_AESTATS,
    AIQ_NOTIFIER_MATCH_HISTEQ,
    AIQ_NOTIFIER_MATCH_ENH,
    AIQ_NOTIFIER_MATCH_TEXEST,
    AIQ_NOTIFIER_MATCH_HSV,
    AIQ_NOTIFIER_MATCH_AIBNR,
    AIQ_NOTIFIER_MATCH_AIYNR,
    AIQ_NOTIFIER_MATCH_MODS_ALL,

    AIQ_NOTIFIER_MATCH_ALL,
    AIQ_NOTIFIER_MATCH_MAX,
};

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
static const char* NotifierMatchType2Str[AIQ_NOTIFIER_MATCH_ALL] = {
    [AIQ_NOTIFIER_MATCH_AEC]      = "ae",
    [AIQ_NOTIFIER_MATCH_HIST]     = "hist",
    [AIQ_NOTIFIER_MATCH_AWB]      = "awb",
    [AIQ_NOTIFIER_MATCH_AWBGAIN]  = "wbgain",
    [AIQ_NOTIFIER_MATCH_AF]       = "af",
    [AIQ_NOTIFIER_MATCH_DPCC]     = "dpc",
    [AIQ_NOTIFIER_MATCH_MERGE]    = "merge",
    [AIQ_NOTIFIER_MATCH_CCM]      = "ccm",
    [AIQ_NOTIFIER_MATCH_LSC]      = "lsc",
    [AIQ_NOTIFIER_MATCH_BLC]      = "blc",
    [AIQ_NOTIFIER_MATCH_RAWNR]    = "rawnr",
    [AIQ_NOTIFIER_MATCH_GIC]      = "gic",
    [AIQ_NOTIFIER_MATCH_DEBAYER]  = "debayer",
    [AIQ_NOTIFIER_MATCH_LUT3D]    = "lut3d",
    [AIQ_NOTIFIER_MATCH_DEHAZE]   = "dehz",
    [AIQ_NOTIFIER_MATCH_AGAMMA]   = "gamma",
    [AIQ_NOTIFIER_MATCH_ADEGAMMA] = "degamma",
    [AIQ_NOTIFIER_MATCH_CSM]      = "csm",
    [AIQ_NOTIFIER_MATCH_CGC]      = "cgc",
    [AIQ_NOTIFIER_MATCH_GAIN]     = "gain",
    [AIQ_NOTIFIER_MATCH_CP]       = "cp",
    [AIQ_NOTIFIER_MATCH_IE]       = "ie",
    [AIQ_NOTIFIER_MATCH_TNR]      = "tnr",
    [AIQ_NOTIFIER_MATCH_YNR]      = "ynr",
    [AIQ_NOTIFIER_MATCH_CNR]      = "cnr",
    [AIQ_NOTIFIER_MATCH_SHARPEN]  = "sharp",
    [AIQ_NOTIFIER_MATCH_DRC]      = "drc",
    [AIQ_NOTIFIER_MATCH_CAC]      = "cac",
    [AIQ_NOTIFIER_MATCH_AFD]      = "afd",
    [AIQ_NOTIFIER_MATCH_RGBIR]    = "rgbir",
    [AIQ_NOTIFIER_MATCH_LDC]      = "ldc",
    [AIQ_NOTIFIER_MATCH_AESTATS]  = "aestats",
    [AIQ_NOTIFIER_MATCH_HISTEQ]   = "histeq",
    [AIQ_NOTIFIER_MATCH_ENH]      = "enh",
    [AIQ_NOTIFIER_MATCH_TEXEST]   = "texest",
    [AIQ_NOTIFIER_MATCH_HSV]      = "hsv",
    [AIQ_NOTIFIER_MATCH_AIBNR]     = "aibnr",
    [AIQ_NOTIFIER_MATCH_AIYNR]     = "aiynr",
};

static enum aiq_notifier_match_type Cam3aResult2NotifierType[RESULT_TYPE_MAX_PARAM] = {
    [RESULT_TYPE_AEC_PARAM]      = AIQ_NOTIFIER_MATCH_AESTATS,
    [RESULT_TYPE_AESTATS_PARAM]  = AIQ_NOTIFIER_MATCH_AESTATS,
    [RESULT_TYPE_AWB_PARAM]      = AIQ_NOTIFIER_MATCH_AWB,
    [RESULT_TYPE_AF_PARAM]       = AIQ_NOTIFIER_MATCH_AF,
    [RESULT_TYPE_DPCC_PARAM]     = AIQ_NOTIFIER_MATCH_DPCC,
    [RESULT_TYPE_MERGE_PARAM]    = AIQ_NOTIFIER_MATCH_MERGE,
    [RESULT_TYPE_CCM_PARAM]      = AIQ_NOTIFIER_MATCH_CCM,
    [RESULT_TYPE_LSC_PARAM]      = AIQ_NOTIFIER_MATCH_LSC,
    [RESULT_TYPE_BLC_PARAM]      = AIQ_NOTIFIER_MATCH_BLC,
    [RESULT_TYPE_RAWNR_PARAM]    = AIQ_NOTIFIER_MATCH_RAWNR,
    [RESULT_TYPE_GIC_PARAM]      = AIQ_NOTIFIER_MATCH_GIC,
    [RESULT_TYPE_DEBAYER_PARAM]  = AIQ_NOTIFIER_MATCH_DEBAYER,
    [RESULT_TYPE_LUT3D_PARAM]    = AIQ_NOTIFIER_MATCH_LUT3D,
    [RESULT_TYPE_DEHAZE_PARAM]   = AIQ_NOTIFIER_MATCH_DEHAZE,
    [RESULT_TYPE_AGAMMA_PARAM]   = AIQ_NOTIFIER_MATCH_AGAMMA,
    [RESULT_TYPE_ADEGAMMA_PARAM] = AIQ_NOTIFIER_MATCH_ADEGAMMA,
    [RESULT_TYPE_CSM_PARAM]      = AIQ_NOTIFIER_MATCH_CSM,
    [RESULT_TYPE_CGC_PARAM]      = AIQ_NOTIFIER_MATCH_CGC,
    [RESULT_TYPE_GAIN_PARAM]     = AIQ_NOTIFIER_MATCH_GAIN,
    [RESULT_TYPE_CP_PARAM]       = AIQ_NOTIFIER_MATCH_CP,
    [RESULT_TYPE_IE_PARAM]       = AIQ_NOTIFIER_MATCH_IE,
    [RESULT_TYPE_TNR_PARAM]      = AIQ_NOTIFIER_MATCH_TNR,
    [RESULT_TYPE_YNR_PARAM]      = AIQ_NOTIFIER_MATCH_YNR,
    [RESULT_TYPE_UVNR_PARAM]     = AIQ_NOTIFIER_MATCH_CNR,
    [RESULT_TYPE_SHARPEN_PARAM]  = AIQ_NOTIFIER_MATCH_SHARPEN,
    [RESULT_TYPE_DRC_PARAM]      = AIQ_NOTIFIER_MATCH_DRC,
    [RESULT_TYPE_CAC_PARAM]      = AIQ_NOTIFIER_MATCH_CAC,
    [RESULT_TYPE_AFD_PARAM]      = AIQ_NOTIFIER_MATCH_AFD,
    [RESULT_TYPE_RGBIR_PARAM]    = AIQ_NOTIFIER_MATCH_RGBIR,
    [RESULT_TYPE_LDC_PARAM]      = AIQ_NOTIFIER_MATCH_LDC,
    [RESULT_TYPE_HISTEQ_PARAM]   = AIQ_NOTIFIER_MATCH_HISTEQ,
    [RESULT_TYPE_ENH_PARAM]      = AIQ_NOTIFIER_MATCH_ENH,
    [RESULT_TYPE_TEXEST_PARAM]   = AIQ_NOTIFIER_MATCH_TEXEST,
    [RESULT_TYPE_HSV_PARAM]      = AIQ_NOTIFIER_MATCH_HSV,
    [RESULT_TYPE_AIBNR_PARAM]      = AIQ_NOTIFIER_MATCH_AIBNR,
    [RESULT_TYPE_AIYNR_PARAM]      = AIQ_NOTIFIER_MATCH_AIYNR,
};

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
/**
 * @struct aiq_notifier_subscriber - aiq notifier subscriber
 *
 * @owner:
 * @dump:
 * @sub_list:
 */
struct aiq_notifier_subscriber {
    enum aiq_notifier_match_type match_type;
    const char* name;
    struct {
        int (*dump_fn_t)(void* dumper, st_string* dump_info, int argc, void* argv[]);
        void* dumper;
    } dump;

    struct list_head list;
};

#define AIQ_NOTIFIER_SUBSCRIBER_INIT(type, dump_fn, dumper) \
    { .match_type = type, .dump.dump_fn_t = dump_fn, .dump.dumper = dumper }

#define _MODS_NUM(num) ((num)-_MODS_NUM_OFFSET)

/**
 * @struct aiq_notifier - aiq notifier data
 *
 * @sub_list:
 */
struct aiq_notifier {
    struct list_head sub_list;
};

/**
 * aiq_notifier_init - Initialize a notifier.
 *
 * @notifier: pointer to &struct v4l2_async_notifier
 *
 * This function initializes the notifier @list. It must be called
 * before the first call to @v4l2_async_notifier_add_subdev.
 */
void aiq_notifier_init(struct aiq_notifier* notifier);

/**
 * aiq_notifier_add_subscriber - Add an subdev to the
 *              notifier's master asd list.
 *
 * @notifier: pointer to &struct v4l2_async_notifier
 * @asd: pointer to &struct v4l2_async_subdev
 *
 * Call this function before registering a notifier to link the provided @asd to
 * the notifiers master @asd_list. The @asd must be allocated with k*alloc() as
 * it will be freed by the framework when the notifier is destroyed.
 */
int aiq_notifier_add_subscriber(struct aiq_notifier* notifier, struct aiq_notifier_subscriber* sub);

/**
 * aiq_notifier_remove_subscriber - remove subdev from the
 *              notifier's master asd list.
 *
 * @notifier: pointer to &struct v4l2_async_notifier
 * @type: subscriber type.
 */
int aiq_notifier_remove_subscriber(struct aiq_notifier* notifier, int type);

/**
 *  aiq_notifier_notify_dumpinfo - Add an subdev to the
 *              notifier's master asd list.
 *
 * @notifier: pointer to &struct v4l2_async_notifier
 * @asd: pointer to &struct v4l2_async_subdev
 *
 * Call this function before registering a notifier to link the provided @asd to
 * the notifiers master @asd_list. The @asd must be allocated with k*alloc() as
 * it will be freed by the framework when the notifier is destroyed.
 */

int aiq_notifier_notify_dumpinfo(struct aiq_notifier* notifier, int type, st_string* dump_info,
                                 int argc, void* argv[]);

const char* aiq_notifier_notify_subscriber_name(struct aiq_notifier* notifier, int type);
const char* aiq_notifier_module_name(struct aiq_notifier* notifier, int type);

#endif  // _AIQ_NOTIFIER_H_
