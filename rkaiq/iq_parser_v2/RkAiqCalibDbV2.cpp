/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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
#include "RkAiqCalibDbV2.h"

#include <fstream>
#include <sys/stat.h>

#include "RkAiqCalibDbV2Helper.h"
#include "cJSON_Utils.h"
#include "j2s.h"
#include "scene/scene_manager.h"
#include "RkAiqVersion.h"

#ifndef __ANDROID__
//#define IQ_DEBUG
#endif

#define CamCalibDbProj_JSON "CamCalibDbProj_t"
#define CamCalibDbContextV2_JSON "CamCalibDbV2Context_t"
#define JSON_PATCH_PATH "path"
#define JSON_PATCH_VALUE "value"

CamCalibDbV2Context_t* calibdbV2_ctx_new() {
    void* calib_scene = NULL;

#if defined(ISP_HW_V20)
    calib_scene = new CamCalibDbV2ContextIsp20_t();
    memset(calib_scene, 0, sizeof(CamCalibDbV2ContextIsp20_t));
#elif defined(ISP_HW_V21)
    calib_scene = new CamCalibDbV2ContextIsp21_t();
    memset(calib_scene, 0, sizeof(CamCalibDbV2ContextIsp21_t));
#elif defined(ISP_HW_V30)
    calib_scene = new CamCalibDbV2ContextIsp30_t();
    memset(calib_scene, 0, sizeof(CamCalibDbV2ContextIsp30_t));
#elif defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    calib_scene = new CamCalibDbV2ContextIsp32_t();
    memset(calib_scene, 0, sizeof(CamCalibDbV2ContextIsp32_t));
#elif defined(ISP_HW_V39)
    calib_scene = new CamCalibDbV2ContextIsp39_t();
    memset(calib_scene, 0, sizeof(CamCalibDbV2ContextIsp39_t));
#else
    XCAM_LOG_ERROR("not supported ISP plateform");
    return NULL;
#endif

    CamCalibDbV2Context_t* calib_ctx = new CamCalibDbV2Context_t();
    calib_ctx->calib_scene = (char*)calib_scene;

    calib_ctx->sensor_info = new CalibDb_Sensor_ParaV2_t();
    memset(calib_ctx->sensor_info, 0, sizeof(CalibDb_Sensor_ParaV2_t));

    calib_ctx->module_info = new CalibDb_Module_ParaV2_t();
    memset(calib_ctx->module_info, 0, sizeof(CalibDb_Module_ParaV2_t));

    calib_ctx->sys_cfg = new CalibDb_SysStaticCfg_ParaV2_t();
    memset(calib_ctx->sys_cfg, 0, sizeof(CalibDb_SysStaticCfg_ParaV2_t));

    return calib_ctx;
}

void calibdbV2_ctx_delete(CamCalibDbV2Context_t* calib_ctx) {
    if (calib_ctx->calib_scene)
        delete calib_ctx->calib_scene;
    if (calib_ctx->sensor_info)
        delete calib_ctx->sensor_info;
    if (calib_ctx->module_info)
        delete calib_ctx->module_info;
    if (calib_ctx->sys_cfg)
        delete calib_ctx->sys_cfg;
    delete calib_ctx;
}

namespace RkCam {

std::map<std::string, CamCalibDbProj_t *> RkAiqCalibDbV2::mCalibDbsMap;
std::mutex RkAiqCalibDbV2::calib_mutex;

CamCalibDbV2Context_t *RkAiqCalibDbV2::CalibV2Alloc() {
    CamCalibDbV2Context_t *calibv2 = calibdbV2_ctx_new();

    if (!calibv2) {
        XCAM_LOG_ERROR("malloc for new calibv2 failed.");
        return nullptr;
    }
    //memset(calibv2, 0, calibdbV2_ctx_size(calibv2));

    return calibv2;
}

int RkAiqCalibDbV2::CalibV2Free(CamCalibDbV2Context_t *calibv2) {
    if (calibv2) {
        // TODO: free calibv2 internal space which allocated by j2s
        calibdbV2_ctx_delete(calibv2);
    } else {
        XCAM_LOG_WARNING("try to free an empty calibv2");
    }

    return 0;
}

CamCalibDbProj_t *RkAiqCalibDbV2::CamCalibDbProjAlloc() {
    CamCalibDbProj_t *calibproj =
        (CamCalibDbProj_t *)malloc(sizeof(CamCalibDbProj_t));
    if (!calibproj) {
        XCAM_LOG_ERROR("malloc for new calibv2 failed.");
        return nullptr;
    }
    memset(calibproj, 0, sizeof(CamCalibDbProj_t));

    return calibproj;
}

void calib_free(void *ptr)
{
    if (ptr)
        free(ptr);
    XCAM_LOG_DEBUG("free %p\n", ptr);
}

int RkAiqCalibDbV2::CamCalibDbProjFree(CamCalibDbProj_t *calibproj) {
    if (calibproj) {
        CamCalibDbFreeSensorCtx(&calibproj->sensor_calib);
        for (int i = 0; i < calibproj->main_scene_len; i++) {
            CamCalibMainSceneList_t* main_scene = calibproj->main_scene + i;
            if (main_scene->name)
                calib_free(main_scene->name);
            for (int j = 0; j < main_scene->sub_scene_len; j++) {
                CamCalibSubSceneList_t* sub_scene = main_scene->sub_scene + j;
                CamCalibDbFreeSceneCtx(calibdbv2_get_scene_ptr(sub_scene));
                if (sub_scene->name)
                    calib_free(sub_scene->name);
            }
            calib_free(main_scene->sub_scene);
        }
        if (calibproj->main_scene)
            calib_free(calibproj->main_scene);
        if (calibproj->uapi) {
            for (int i = 0; i < calibproj->uapi_len; i++)
                CamCalibDbFreeUapiCtx(calibproj->uapi + i);
            calib_free(calibproj->uapi);
        }
        CamCalibDbFreeSysStaticCtx(&calibproj->sys_static_cfg);
        free(calibproj);
    } else {
        XCAM_LOG_WARNING("try to free an empty CamCalibDbProj_t");
    }

    return 0;
}

CamCalibDbV2Context_t *RkAiqCalibDbV2::json2calib(const char *jsfile) {
#if 0
    j2s_ctx ctx;
    int ret = -1;

    j2s_init(&ctx);
    ctx.format_json = false;
    ctx.manage_data = false;

    CamCalibDbV2Context_t *calib = CalibV2Alloc();

    ret = j2s_json_file_to_struct(&ctx, jsfile,
                                  calibdbv2_get_ctx_struct_name(calib), calib);
    j2s_deinit(&ctx);

    if (ret) {
        CalibV2Free(calib);
        return nullptr;
    }

#ifdef IQ_DEBUG
    calib2json("/tmp/iq_dump.json", calib);
#endif

    return calib;
#else
    // TODO
    return NULL;
#endif
}

CamCalibDbProj_t *RkAiqCalibDbV2::json2calibproj(const char *jsfile) {
    j2s_ctx ctx;
    char* json_buff = NULL;
    size_t json_size = 0;
    cJSON* base_json = NULL;
    int ret = -1;

    json_buff = (char*)j2s_read_file(jsfile, &json_size);
    if (!json_buff) {
        return nullptr;
    }

    base_json = cJSON_Parse(json_buff);
    if (!base_json) {
        free(json_buff);
        return nullptr;
    }

    j2s_init(&ctx);
    ctx.format_json = false;
    ctx.manage_data = false;

    if (!RkAiqSceneManager::mergeMultiSceneIQ(base_json)) {
        cJSON_Delete(base_json);
        j2s_deinit(&ctx);
        free(json_buff);
        return nullptr;
    }

    CamCalibDbProj_t *calibproj = CamCalibDbProjAlloc();

    ret = j2s_json_to_struct(&ctx, base_json, NULL, calibproj);
    cJSON_Delete(base_json);
    j2s_deinit(&ctx);

    if (ret) {
        CamCalibDbProjFree(calibproj);
        free(json_buff);
        return nullptr;
    }

#ifdef IQ_DEBUG
    calibproj2json("/tmp/iq_dump.json", calibproj);
#endif

    free(json_buff);
    return calibproj;
}

CamCalibDbProj_t *RkAiqCalibDbV2::json2calibproj(const char *jstr, size_t len) {
    j2s_ctx ctx;
    cJSON* base_json = NULL;
    int ret = -1;

    if (!jstr || !len) {
        return nullptr;
    }

    base_json = cJSON_Parse(jstr);
    if (!base_json) {
        return nullptr;
    }

    j2s_init(&ctx);
    ctx.format_json = false;
    ctx.manage_data = false;

    if (!RkAiqSceneManager::mergeMultiSceneIQ(base_json)) {
        cJSON_Delete(base_json);
        j2s_deinit(&ctx);
        return nullptr;
    }

    CamCalibDbProj_t *calibproj = CamCalibDbProjAlloc();

    ret = j2s_json_to_struct(&ctx, base_json, NULL, calibproj);
    cJSON_Delete(base_json);
    j2s_deinit(&ctx);

    if (ret) {
        CamCalibDbProjFree(calibproj);
        return nullptr;
    }

#ifdef IQ_DEBUG
    calibproj2json("/tmp/iq_dump.json", calibproj);
#endif

    return calibproj;
}

void RkAiqCalibDbV2::bin2calib(void *bin_buff, void *struct_ptr) {
#if defined(ISP_HW_V32)
    uint8_t *buf_start = (uint8_t *)bin_buff;
    calib2bin_header_t *header;
    calib2bin_block_t *block_list;
    CalibDb_Sensor_ParaV2_t *sns_ptr = NULL;
    CalibDb_Aec_ParaV2_t* ae_ptr = NULL;
    CalibDbV2_Bayer2dnrV23_t* bayer2dnr_v23_ptr = NULL;
    CalibDbV2_BayerTnrV23_t* bayertnr_v23_ptr = NULL;
    CalibDbV2_Wb_Para_V32_t* wb_v32 = NULL;
    CalibDbV2_Blc_V32_t* ablcV32_calib = NULL;

    header = (calib2bin_header_t *)bin_buff;
    block_list = (calib2bin_block_t *)(buf_start + header->block_offset);
    buf_start += header->bin_offset;
    for (int i = 0; i < header->block_len; i++) {
        LOGD("header info: %s offset %d size %d\n", block_list[i].name, block_list[i].offset, block_list[i].size);
        if (strcmp("CalibDb_Sensor_ParaV2_t", block_list[i].name) == 0)
            sns_ptr = (CalibDb_Sensor_ParaV2_t *)bin2calibproj(buf_start + block_list[i].offset, block_list[i].size);
        else if (strcmp("CalibDb_Aec_ParaV2_t", block_list[i].name) == 0)
            ae_ptr = (CalibDb_Aec_ParaV2_t *)bin2calibproj(buf_start + block_list[i].offset, block_list[i].size);
        else if (strcmp("CalibDbV2_Bayer2dnrV23_t", block_list[i].name) == 0)
            bayer2dnr_v23_ptr = (CalibDbV2_Bayer2dnrV23_t *)bin2calibproj(buf_start + block_list[i].offset, block_list[i].size);
        else if (strcmp("CalibDbV2_BayerTnrV23_t", block_list[i].name) == 0)
            bayertnr_v23_ptr = (CalibDbV2_BayerTnrV23_t *)bin2calibproj(buf_start + block_list[i].offset, block_list[i].size);
        else if (strcmp("CalibDbV2_Wb_Para_V32_t", block_list[i].name) == 0)
            wb_v32 = (CalibDbV2_Wb_Para_V32_t *)bin2calibproj(buf_start + block_list[i].offset, block_list[i].size);
        else if (strcmp("CalibDbV2_Blc_V32_t", block_list[i].name) == 0)
            ablcV32_calib = (CalibDbV2_Blc_V32_t *)bin2calibproj(buf_start + block_list[i].offset, block_list[i].size);
    }


    // printf("sensor calib: %d %d %d\n", sns_ptr->CISDcgSet.Linear.dcg_mode.Coeff[0],
    //                                       sns_ptr->CISDcgSet.Linear.dcg_mode.Coeff[1],
    //                                       sns_ptr->CISDcgSet.Linear.dcg_mode.Coeff[2]);


    // printf("ae calib: rout: IspDGainDot: ");
    // for (int i = 0; i < ae_ptr->LinearAeCtrl.Route.IspDGainDot_len; i++) {
    //     printf("%f ", ae_ptr->LinearAeCtrl.Route.IspDGainDot[i]);
    // }
    // printf("\n");

    // printf("ae calib: rout: PIrisDot: ");
    // for (int i = 0; i < ae_ptr->LinearAeCtrl.Route.PIrisDot_len; i++) {
    //     printf("%d ", ae_ptr->LinearAeCtrl.Route.PIrisDot[i]);
    // }
    // printf("\n");

    // printf("ae calib: rout: TimeDot: ");
    // for (int i = 0; i < ae_ptr->LinearAeCtrl.Route.TimeDot_len; i++) {
    //     printf("%f ", ae_ptr->LinearAeCtrl.Route.TimeDot[i]);
    // }
    // printf("\n");

    // printf("ae calib: rout: GainDot: ");
    // for (int i = 0; i < ae_ptr->LinearAeCtrl.Route.GainDot_len; i++) {
    //     printf("%f ", ae_ptr->LinearAeCtrl.Route.GainDot[i]);
    // }
    // printf("\n");

    // printf("2dnr: %d %d %s\n", bayer2dnr_v23_ptr->TuningPara.hdrdgain_ctrl_en,
    //                               bayer2dnr_v23_ptr->TuningPara.Setting_len,
    //                               bayer2dnr_v23_ptr->TuningPara.Setting[0].SNR_Mode);

    // printf("tnr: %d %d %d %d %s\n", bayertnr_v23_ptr->TuningPara.thumbds_w,
    //                               bayertnr_v23_ptr->TuningPara.thumbds_h,
    //                               bayertnr_v23_ptr->TuningPara.trans_en,
    //                               bayertnr_v23_ptr->TuningPara.Setting_len,
    //                               bayertnr_v23_ptr->TuningPara.Setting[0].SNR_Mode);

    // printf("awb: %d %d %d %f %f %f %f\n", wb_v32->control.byPass,
    //                                       wb_v32->control.mode,
    //                                       wb_v32->manualPara.mode,
    //                                       wb_v32->manualPara.cfg.mwbGain[0],
    //                                       wb_v32->manualPara.cfg.mwbGain[1],
    //                                       wb_v32->manualPara.cfg.mwbGain[2],
    //                                       wb_v32->manualPara.cfg.mwbGain[3]);

    // printf("blc %f %f %f %f\n", ablcV32_calib->Blc0TuningPara.BLC_Data.R_Channel[0],
    //                          ablcV32_calib->Blc0TuningPara.BLC_Data.B_Channel[0],
    //                          ablcV32_calib->Blc0TuningPara.BLC_Data.Gb_Channel[0],
    //                          ablcV32_calib->Blc0TuningPara.BLC_Data.Gr_Channel[0]);
#endif
}

CamCalibDbProj_t *RkAiqCalibDbV2::bin2calibproj(const char *binfile) {
    CamCalibDbProj_t *calibproj = NULL;
    char* bin_buff = NULL;
    size_t bin_size = 0;
    int ret = -1;

    bin_buff = (char*)loadWholeFile(binfile, &bin_size);
    if (!bin_buff) {
        return NULL;
    }

    ret = checkBinVersion((uint8_t*)bin_buff, bin_size);
    if (ret) {
        LOGW("iq bin version is no current");
    }

    ret = parseBinStructMap((uint8_t*)bin_buff, bin_size);
    if (ret) {
        return NULL;
    }

    calibproj = (CamCalibDbProj_t*) bin_buff;

    return calibproj;
}

CamCalibDbProj_t *RkAiqCalibDbV2::bin2calibproj(const void *bin_buff, size_t len) {
    CamCalibDbProj_t *calibproj = NULL;
    int ret = -1;

    if (!bin_buff || !len)
        return NULL;

    ret = checkBinVersion((uint8_t*)bin_buff, len);
    if (ret) {
        LOGW("iq bin version is no current");
    }

    ret = parseBinStructMap((uint8_t*)bin_buff, len);
    if (ret) {
        return NULL;
    }

    calibproj = (CamCalibDbProj_t*) bin_buff;

    return calibproj;
}

CamCalibDbCamgroup_t* RkAiqCalibDbV2::createCalibDbCamgroup(const char *jsfile) {
    j2s_ctx ctx;
    int ret = -1;

    j2s_camgroup_init(&ctx);
    ctx.format_json = false;
    ctx.manage_data = false;

    CamCalibDbCamgroup_t* calib_camgroup =
        (CamCalibDbCamgroup_t*)calloc(1, sizeof(CamCalibDbCamgroup_t));
    ret = j2s_json_file_to_root_struct(&ctx, jsfile, calib_camgroup);
    j2s_deinit(&ctx);

    if (ret) {
        CamCalibDbCamgroupFree(calib_camgroup);
        return nullptr;
    }

    return calib_camgroup;
}

int RkAiqCalibDbV2::CamCalibDbCamgroupFree(CamCalibDbCamgroup_t* calib_camgroup)
{
    if (calib_camgroup)
        free(calib_camgroup);
    return 0;
}

int RkAiqCalibDbV2::calibproj2json(const char *jsfile,
                                   CamCalibDbProj_t *calibproj) {
    char *json_buff = NULL;
    j2s_ctx ctx;

    ctx.format_json = true;
    ctx.manage_data = false;

    if (0 != access(jsfile, F_OK)) {
        XCAM_LOG_INFO("Calibproj file already %s exist, oaverride now!", jsfile);
    }

    std::ofstream ofile(jsfile);
    if (ofile.fail()) {
        XCAM_LOG_ERROR("write %s failed!", jsfile);
        return -1;
    }

    j2s_init(&ctx);
    json_buff = j2s_dump_root_struct(&ctx, calibproj);
    j2s_deinit(&ctx);

    if (!json_buff) {
        XCAM_LOG_ERROR("create %s failed.", jsfile);
        return -1;
    }

    ofile << json_buff;

    free(json_buff);

    return 0;
}

int RkAiqCalibDbV2::calib2bin(void *bin_buf,
                              CamCalibDbV2Context_t *CalibDbV2) {
#ifdef ISP_HW_V32
    j2s_ctx ctx;
    uint8_t *bin_start;
    calib2bin_header_t *header;
    calib2bin_block_t *block_list;
    CamCalibDbV2ContextIsp32_t *sub_scene;

    header               = (calib2bin_header_t *)bin_buf;
    sub_scene            = (CamCalibDbV2ContextIsp32_t *)CalibDbV2->calib_scene;
    header->bin_size     = sizeof(calib2bin_header_t);
    header->block_offset = header->bin_size;
    header->mask         |= 1;
    ctx.format_json      = true;
    ctx.manage_data      = false;

    struct block_s {
        const char* name;
        void* offset;
    } calib_array[] = {
        { "CalibDb_Sensor_ParaV2_t",  (void *)CalibDbV2->sensor_info   },
        { "CalibDb_Aec_ParaV2_t",     (void *)&sub_scene->ae_calib     },
        { "CalibDbV2_Bayer2dnrV23_t", (void *)&sub_scene->bayer2dnr_v23},
#ifndef USE_NEWSTRUCT
        { "CalibDbV2_BayerTnrV23_t",  (void *)&sub_scene->bayertnr_v23 },
#endif
        { "CalibDbV2_Wb_Para_V32_t",  (void *)&sub_scene->wb_v32       },
        { "CalibDbV2_Blc_V32_t",      (void *)&sub_scene->ablcV32_calib},
        {NULL, NULL},
    };

    block_list         = (calib2bin_block_t *)((uint8_t *)bin_buf + sizeof(calib2bin_header_t));
    header->block_len  = sizeof(calib_array) / sizeof(calib_array[0]) - 1;
    header->bin_size   += header->block_len * sizeof(calib2bin_block_t);
    header->bin_offset = header->bin_size;
    bin_start          = (uint8_t *)bin_buf + header->bin_size;

    j2s_init(&ctx);
    for (int i = 0; calib_array[i].offset != NULL; i++) {
        block_list[i].offset = header->bin_size - header->bin_offset;
        block_list[i].size   = j2s_calib_to_bin(&ctx, calib_array[i].name, calib_array[i].offset, bin_start);
        header->bin_size     += block_list[i].size;
        bin_start            += block_list[i].size;
        snprintf(block_list[i].name, 32, "%s", calib_array[i].name);
        LOGD("header info: %s offset %d size %d\n", block_list[i].name, block_list[i].offset, block_list[i].size);
    }
    j2s_deinit(&ctx);
#endif
    return 0;
}

int RkAiqCalibDbV2::calib2json(const char *jsfile,
                               CamCalibDbV2Context_t *calib) {
    char *json_buff = NULL;
    j2s_ctx ctx;

    ctx.format_json = true;

    if (0 != access(jsfile, F_OK)) {
        XCAM_LOG_INFO("Calib file already %s exist, oaverride now!", jsfile);
    }

    std::ofstream ofile(jsfile);
    if (ofile.fail()) {
        XCAM_LOG_ERROR("write %s failed!", jsfile);
        return -1;
    }

    j2s_init(&ctx);
    json_buff = j2s_dump_struct(&ctx, calibdbv2_get_scene_ctx_struct_name(calib), calib->calib_scene);
    j2s_deinit(&ctx);

    if (!json_buff) {
        XCAM_LOG_ERROR("create %s failed.", jsfile);
        return -1;
    }

    ofile << json_buff;

    free(json_buff);

    return 0;
}

CamCalibDbV2Context_t *RkAiqCalibDbV2::cjson2calib(cJSON *json) {
    j2s_ctx ctx;
    int ret = -1;

    j2s_init(&ctx);
    ctx.format_json = false;
    ctx.manage_data = false;

    CamCalibDbV2Context_t *calib = CalibV2Alloc();

    ret = j2s_json_to_struct(&ctx, json, calibdbv2_get_scene_ctx_struct_name(calib), calib->calib_scene);
    j2s_deinit(&ctx);

    if (ret) {
        CalibV2Free(calib);
        return nullptr;
    }

    return calib;
}

cJSON *RkAiqCalibDbV2::calib2cjson(const CamCalibDbV2Context_t *calib) {
    cJSON *json = NULL;
    j2s_ctx ctx;

    j2s_init(&ctx);
    json = j2s_struct_to_json(&ctx, calibdbv2_get_scene_ctx_struct_name(calib), (void *)calib->calib_scene);
    j2s_deinit(&ctx);

    if (!json) {
        XCAM_LOG_ERROR("%s failed.", __func__);
        return nullptr;
    }

    return json;
}

CamCalibDbProj_t *RkAiqCalibDbV2::createCalibDbProj(const char *jsfile) {
    std::map<std::string, CamCalibDbProj_t *>::iterator it;
    CamCalibDbProj_t *calibproj = NULL;
    std::string str(jsfile);
    const std::lock_guard<std::mutex> lock(RkAiqCalibDbV2::calib_mutex);

    it = mCalibDbsMap.find(str);
    if (it != mCalibDbsMap.end()) {
        XCAM_LOG_INFO("use cached calibdb for %s!", jsfile);
        return it->second;
    } else {
        std::string binfile = str.substr(0, str.find_last_of(".")) + ".bin";
        if (0 == access(jsfile, F_OK)) {
            calibproj = json2calibproj(jsfile);
        } else if (0 == access(binfile.c_str(), F_OK)) {
            calibproj = bin2calibproj(binfile.c_str());
        } else {
            XCAM_LOG_ERROR("access %s && %s failed!", jsfile, binfile.c_str());
            return nullptr;
        }

        if (calibproj) {
            mCalibDbsMap[str] = calibproj;
            XCAM_LOG_INFO("create calibdb from %s success.", jsfile);
            return calibproj;
        } else {
            XCAM_LOG_ERROR("parse %s failed.", jsfile);
            return nullptr;
        }
    }
}

CamCalibDbProj_t *RkAiqCalibDbV2::createCalibDbProj(const void *bin_buff,
        size_t len) {
    CamCalibDbProj_t *calibproj = NULL;
    const std::lock_guard<std::mutex> lock(RkAiqCalibDbV2::calib_mutex);

    calibproj = bin2calibproj(bin_buff, len);

    if (calibproj) {
        XCAM_LOG_INFO("create calibdb from buffer success.");
        return calibproj;
    }

    XCAM_LOG_ERROR("parse binary iq buffer failed.");
    return nullptr;
}

void RkAiqCalibDbV2::releaseCalibDbProj() {
    std::map<std::string, CamCalibDbProj_t *>::iterator it;
    for (it = mCalibDbsMap.begin(); it != mCalibDbsMap.end(); it++) {
        CamCalibDbProj_t *calibproj = it->second;
        if (calibproj) {
            CamCalibDbProjFree(calibproj);
        }
    }
    mCalibDbsMap.clear();
}

CamCalibDbV2Context_t
RkAiqCalibDbV2::toDefaultCalibDb(CamCalibDbProj_t *calibproj) {
    CamCalibDbV2Context_t ctx;
    memset(&ctx, 0, sizeof(CamCalibDbV2Context_t));
    if (!calibproj || !calibproj->main_scene ||
            !calibproj->main_scene[0].sub_scene) {
        XCAM_LOG_ERROR("CamCalibDbProj in invalied!");
        return ctx;
    }
    ctx.calib_scene = (char*)(calibdbv2_get_scene_ptr(&calibproj->main_scene[0].sub_scene[0]));
    ctx.sensor_info = &calibproj->sensor_calib;
    ctx.module_info = &calibproj->module_calib;
    ctx.sys_cfg = &calibproj->sys_static_cfg;

    return ctx;
}

CamCalibDbV2Context_t *
RkAiqCalibDbV2::applyPatch(const CamCalibDbV2Context_t *calib, cJSON *patch) {
    cJSON *base_json = NULL;
    int ret = -1;

    if (!calib || !patch) {
        XCAM_LOG_ERROR("%s input invalied!", __func__);
        return nullptr;
    }

    base_json = calib2cjson(calib);

    if (!base_json) {
        XCAM_LOG_ERROR("%s conver failed!", __func__);
        return nullptr;
    }

    ret = cJSONUtils_ApplyPatches(base_json, patch);
    if (0 != ret) {
        XCAM_LOG_ERROR("%s apply patch failed %d!", __func__, ret);
        return nullptr;
    }

    return cjson2calib(base_json);
}

CamCalibDbV2Context_t *
RkAiqCalibDbV2::applyPatch(const CamCalibDbV2Context_t *calib,
                           const char *patch_str) {
    cJSON *patch_json = NULL;

    patch_json = cJSON_Parse(patch_str);

    return applyPatch(calib, patch_json);
}

CamCalibDbV2Context_t *
RkAiqCalibDbV2::applyPatch2(const CamCalibDbV2Context_t *calib, cJSON *patch) {
    CamCalibDbV2Context_t *new_calib = NULL;
    cJSON *base_json = NULL;
    cJSON *arr_item = NULL;
    j2s_ctx ctx;
    ctx.format_json = true;
    int change_sum = 0;
    int ret = -1;
    const std::lock_guard<std::mutex> lock(RkAiqCalibDbV2::calib_mutex);

    if (!calib || !patch) {
        XCAM_LOG_ERROR("%s input invalied!", __func__);
        return nullptr;
    }

    change_sum = cJSON_GetArraySize(patch);

    if (change_sum <= 0) {
        XCAM_LOG_INFO("[Tuning]: analyz change failed!\n");
        return nullptr;
    }

    // check if not common info add "calib_scene" prefix to path
    arr_item = patch->child;

    for (int i = 0; i <= (change_sum - 1); ++i) {
        if (arr_item) {
            auto path_str = std::string(
                                cJSON_GetObjectItem(arr_item, JSON_PATCH_PATH)->valuestring);
            auto headless = path_str.substr(path_str.find_first_not_of("/"),
                                            path_str.find_last_not_of("/"));
            auto module_name = headless.substr(0, headless.find_first_of("/"));

            if (module_name.compare(std::string("sensor_calib")) == 0) {
                continue;
            } else if (module_name.compare(std::string("module_calib")) == 0) {
                continue;
            } else if (module_name.compare(std::string("sys_static_cfg")) == 0) {
                continue;
            }
            path_str = std::string("/calib_scene") + path_str;
            cJSON_ReplaceItemInObject(arr_item, JSON_PATCH_PATH,
                                      cJSON_CreateString(path_str.c_str()));
        }
        arr_item = arr_item->next;
    }

    // convert CamCalibDbV2Context_t to CamCalibDbV2Tuning_t
    CamCalibDbV2Tuning_t tuning_base;
    memset(&tuning_base, 0, sizeof(CamCalibDbV2Tuning_t));
    calibdbV2_to_tuningdb(&tuning_base, calib);

    j2s_init(&ctx);
    base_json = j2s_struct_to_json(&ctx, "CamCalibDbV2Tuning_t", &tuning_base);

    if (!base_json) {
        XCAM_LOG_ERROR("create CamCalibDbV2Tuning_t failed.");
        goto patch_failed;
    }

    ret = cJSONUtils_ApplyPatches(base_json, patch);
    if (0 != ret) {
        XCAM_LOG_ERROR("%s apply patch failed %d!", __func__, ret);
        goto patch_failed;
    }

    CamCalibDbV2Tuning_t tuning_patched;
    memset(&tuning_patched, 0, sizeof(CamCalibDbV2Tuning_t));
    ret = j2s_json_to_struct(&ctx, base_json, "CamCalibDbV2Tuning_t",
                             &tuning_patched);
    if (0 != ret) {
        XCAM_LOG_ERROR("%s apply patch failed %d!", __func__, ret);
        goto patch_failed;
    }

    new_calib = CalibV2Alloc();
    calibdbV2_from_tuningdb(new_calib, &tuning_patched);

patch_failed:
    j2s_deinit(&ctx);

    if (base_json)
        cJSON_Delete(base_json);

    return new_calib;
}

CamCalibDbV2Context_t *
RkAiqCalibDbV2::applyPatchFile(const CamCalibDbV2Context_t *calib,
                               const char *patch_file) {
    size_t json_size = 0;
    char *json_str = NULL;

    json_str = (char *)j2s_read_file(patch_file, &json_size);

    return applyPatch(calib, json_str);
}

CamCalibDbV2Context_t *RkAiqCalibDbV2::applyPatch(const char *jsfile,
        const char *js_patch) {
    return NULL;
}

RkAiqAlgoType_t RkAiqCalibDbV2::string2algostype(const char *str) {
    static std::unordered_map<std::string, RkAiqAlgoType_t> const table = {
        {"sensor_calib", RK_AIQ_ALGO_TYPE_AE},
        {"module_calib", RK_AIQ_ALGO_TYPE_AE},
        {"ae_calib", RK_AIQ_ALGO_TYPE_AE},
        {"wb_v20", RK_AIQ_ALGO_TYPE_AWB},
        {"wb_v21", RK_AIQ_ALGO_TYPE_AWB},
        {"af_calib", RK_AIQ_ALGO_TYPE_AF},
        {"af_v30", RK_AIQ_ALGO_TYPE_AF},
        {"af_v31", RK_AIQ_ALGO_TYPE_AF},
        {"af_v32", RK_AIQ_ALGO_TYPE_AF},
        {"af_v33", RK_AIQ_ALGO_TYPE_AF},
        {"ablc_calib", RK_AIQ_ALGO_TYPE_ABLC},
        {"adpcc_calib", RK_AIQ_ALGO_TYPE_ADPCC},
        {"dpc", RK_AIQ_ALGO_TYPE_ADPCC},
        {"amerge_calib_v10", RK_AIQ_ALGO_TYPE_AMERGE},
        {"amerge_calib_v11", RK_AIQ_ALGO_TYPE_AMERGE},
        {"amerge_calib_v12", RK_AIQ_ALGO_TYPE_AMERGE},
        {"atmo_calib", RK_AIQ_ALGO_TYPE_ATMO},
        {"anr_calib", RK_AIQ_ALGO_TYPE_ANR},
        {"lsc_v2", RK_AIQ_ALGO_TYPE_ALSC},
        {"agic_calib_v20", RK_AIQ_ALGO_TYPE_AGIC},
        {"agic_calib_v21", RK_AIQ_ALGO_TYPE_AGIC},
        {"debayer_v2", RK_AIQ_ALGO_TYPE_ADEBAYER},
        {"debayer", RK_AIQ_ALGO_TYPE_ADEBAYER},
        {"ccm_calib", RK_AIQ_ALGO_TYPE_ACCM},
        {"ccm_calib_v2", RK_AIQ_ALGO_TYPE_ACCM},
        {"agamma_calib_v10", RK_AIQ_ALGO_TYPE_AGAMMA},
        {"agamma_calib_v11", RK_AIQ_ALGO_TYPE_AGAMMA},
        {"gamma", RK_AIQ_ALGO_TYPE_AGAMMA},
        {"awdr_calib", RK_AIQ_ALGO_TYPE_AWDR},
        {"adehaze_calib_v10", RK_AIQ_ALGO_TYPE_ADHAZ},
        {"adehaze_calib_v11", RK_AIQ_ALGO_TYPE_ADHAZ},
        {"adehaze_calib_v12", RK_AIQ_ALGO_TYPE_ADHAZ},
        {"dehaze", RK_AIQ_ALGO_TYPE_ADHAZ},
        {"lut3d_calib", RK_AIQ_ALGO_TYPE_A3DLUT},
        {"aldch", RK_AIQ_ALGO_TYPE_ALDCH},
        {"csm", RK_AIQ_ALGO_TYPE_ACSM},
        {"cproc", RK_AIQ_ALGO_TYPE_ACP},
        {"ie", RK_AIQ_ALGO_TYPE_AIE},
        {"sharp_v1", RK_AIQ_ALGO_TYPE_ASHARP},
        {"edgefilter_v1", RK_AIQ_ALGO_TYPE_ASHARP},
        {"aorb_calib", RK_AIQ_ALGO_TYPE_AORB},
        {"cgc", RK_AIQ_ALGO_TYPE_ACGC},
        {"asd_calib", RK_AIQ_ALGO_TYPE_ASD},
        {"adrc_calib_v10", RK_AIQ_ALGO_TYPE_ADRC},
        {"adrc_calib_v11", RK_AIQ_ALGO_TYPE_ADRC},
        {"adrc_calib_v12", RK_AIQ_ALGO_TYPE_ADRC},
        {"drc", RK_AIQ_ALGO_TYPE_ADRC},
        {"trans", RK_AIQ_ALGO_TYPE_ATRANS},
        {"adegamma_calib", RK_AIQ_ALGO_TYPE_ADEGAMMA},
        {"cac_v03", RK_AIQ_ALGO_TYPE_ACAC},
        {"cac_v10", RK_AIQ_ALGO_TYPE_ACAC},
        {"cac_v11", RK_AIQ_ALGO_TYPE_ACAC},

#if ANR_NO_SEPERATE_MARCO
        {"bayernr_v1", RK_AIQ_ALGO_TYPE_ANR},
        {"mfnr_v1", RK_AIQ_ALGO_TYPE_ANR},
        {"ynr_v1", RK_AIQ_ALGO_TYPE_ANR},
        {"uvnr_v1", RK_AIQ_ALGO_TYPE_ANR},
#else
        {"bayernr_v1", RK_AIQ_ALGO_TYPE_ARAWNR},
        {"mfnr_v1", RK_AIQ_ALGO_TYPE_AMFNR},
        {"ynr_v1", RK_AIQ_ALGO_TYPE_AYNR},
        {"uvnr_v1", RK_AIQ_ALGO_TYPE_ACNR},
#endif
        {"aeis_calib", RK_AIQ_ALGO_TYPE_AEIS},
        {"afec_calib", RK_AIQ_ALGO_TYPE_AFEC},

        // ror rk356x nr & sharp
        {"bayernr_v2", RK_AIQ_ALGO_TYPE_ARAWNR},
        {"ynr_v2", RK_AIQ_ALGO_TYPE_AYNR},
        {"cnr_v1", RK_AIQ_ALGO_TYPE_ACNR},
        {"sharp_v3", RK_AIQ_ALGO_TYPE_ASHARP},

        // rk3588 nr & sharp
        {"bayer2dnr_v2", RK_AIQ_ALGO_TYPE_ARAWNR},
        {"bayertnr_v2", RK_AIQ_ALGO_TYPE_AMFNR},
        {"ynr_v3", RK_AIQ_ALGO_TYPE_AYNR},
        {"cnr_v2", RK_AIQ_ALGO_TYPE_ACNR},
        {"sharp_v4", RK_AIQ_ALGO_TYPE_ASHARP},
        {"gain_v2", RK_AIQ_ALGO_TYPE_AGAIN},

        // rv1106 blc nr & sharp
        {"ablcV32_calib", RK_AIQ_ALGO_TYPE_ABLC},
        {"bayer2dnr_v23", RK_AIQ_ALGO_TYPE_ARAWNR},
        {"bayertnr_v23", RK_AIQ_ALGO_TYPE_AMFNR},
        {"bayertnr", RK_AIQ_ALGO_TYPE_AMFNR},
        {"ynr_v22", RK_AIQ_ALGO_TYPE_AYNR},
        {"cnr_v30", RK_AIQ_ALGO_TYPE_ACNR},
        {"sharp_v33", RK_AIQ_ALGO_TYPE_ASHARP},
        {"wb_v32", RK_AIQ_ALGO_TYPE_AWB},
        {"yuvme_v1", RK_AIQ_ALGO_TYPE_AMD},
    };

    auto it = table.find(std::string(str));
    if (it != table.end()) {
        return it->second;
    }

    return RK_AIQ_ALGO_TYPE_NONE;
}

ModuleNameList RkAiqCalibDbV2::analyzChange(cJSON *patch) {
    auto change_name_list = std::make_shared<std::list<std::string>>();
    cJSON *arr_item = NULL;
    int change_sum = 0;

    change_sum = cJSON_GetArraySize(patch);

    XCAM_LOG_INFO("[Tuning]: %d changes in this cmd!\n", change_sum);

    if (change_sum <= 0) {
        XCAM_LOG_INFO("[Tuning]: analyz change failed!\n");
        return nullptr;
    }

    arr_item = patch->child;

    for (int i = 0; i <= (change_sum - 1); ++i) {
        if (arr_item) {
            auto path_str = std::string(
                                cJSON_GetObjectItem(arr_item, JSON_PATCH_PATH)->valuestring);
            auto headless = path_str.substr(path_str.find_first_not_of("/"),
                                            path_str.find_last_not_of("/"));
            auto calib_name = headless.substr(0, headless.find_first_of("/"));
            XCAM_LOG_INFO("[Tuning]: %s changed!\n", calib_name.c_str());
            change_name_list->push_back(calib_name);
        }
        arr_item = arr_item->next;
    }

    // Remove duplicated item
    change_name_list->sort();
    change_name_list->unique();

    return change_name_list;
}

RkAiqCalibDbV2::TuningCalib
RkAiqCalibDbV2::analyzTuningCalib(const CamCalibDbV2Context_t *calib,
                                  cJSON *patch) {
    TuningCalib tuning_calib;

    tuning_calib.ModuleNames = analyzChange(patch);
    tuning_calib.calib = applyPatch2(calib, patch);

    if (patch)
        cJSON_Delete(patch);

    return tuning_calib;
}

RkAiqCalibDbV2::TuningCalib
RkAiqCalibDbV2::analyzTuningCalib(const CamCalibDbV2Context_t *calib,
                                  const char *patch_str) {
    cJSON *patch_json = NULL;

    patch_json = cJSON_Parse(patch_str);

    return analyzTuningCalib(calib, patch_json);
}

cJSON *RkAiqCalibDbV2::readIQNode(const CamCalibDbV2Context_t *calib,
                                  const char *node_path) {
    cJSON *base_json = NULL;
    cJSON *ret_json = NULL;
    auto path_str = std::string(node_path);
    auto headless = path_str.substr(path_str.find_first_not_of("/"),
                                    path_str.find_last_not_of("/"));
    auto module_name = headless.substr(0, headless.find_first_of("/"));

    if (module_name.compare(std::string("sensor_calib")) == 0 ||
            module_name.compare(std::string("module_calib")) == 0 ||
            module_name.compare(std::string("sys_static_cfg")) == 0) {
        j2s_ctx ctx;
        ctx.format_json = true;
        CamCalibDbV2Tuning_t tuning_base;

        memset(&tuning_base, 0, sizeof(CamCalibDbV2Tuning_t));
        calibdbV2_to_tuningdb(&tuning_base, calib);

        j2s_init(&ctx);
        base_json = j2s_struct_to_json(&ctx, "CamCalibDbV2Tuning_t", &tuning_base);
        j2s_deinit(&ctx);
    } else {
        base_json = calib2cjson(calib);
    }

    if (!base_json) {
        XCAM_LOG_ERROR("%s conver failed!", __func__);
        return nullptr;
    }

    cJSON *node_json = cJSONUtils_GetPointer(base_json, node_path);
    if (node_json) {
        ret_json = cJSON_Duplicate(node_json, 1);
    }

    cJSON_Delete(base_json);

    return ret_json;
}

/**
 * @brief read json string from a json iq
 *
 * @param calib
 * @param node_path
 *
 * @return string buffer or NULL, must be free outside.
 */
char* RkAiqCalibDbV2::readIQNodeStr(const CamCalibDbV2Context_t* calib,
                                    const char* node_path) {
    cJSON* node_json = readIQNode(calib, node_path);
    if (!node_json) {
        XCAM_LOG_ERROR("%s find json node failed!", __func__);
        return nullptr;
    }

    return cJSON_Print(node_json);
}

char* RkAiqCalibDbV2::readIQNodeStrFromJstr(const CamCalibDbV2Context_t* calib,
        const char* json_str) {
    cJSON *request_json = NULL;
    cJSON *arr_item = NULL;
    char* ret_str = NULL;
    int request_sum = 0;

    cJSON *ret_json;
    ret_json = cJSON_CreateArray();

    request_json = cJSON_Parse(json_str);
    request_sum = cJSON_GetArraySize(request_json);

    if (request_sum <= 0) {
        XCAM_LOG_INFO("[Tuning]: analyz request failed!\n");
        return nullptr;
    }

    arr_item = request_json->child;

    for (int i = 0; i <= (request_sum - 1); ++i) {
        if (arr_item) {
            char* path_str = cJSON_GetObjectItem(arr_item, JSON_PATCH_PATH)->valuestring;
            cJSON* ret_node = readIQNode(calib, path_str);
            if (ret_node) {
                cJSON* ret_item = cJSON_CreateObject();
                cJSON_AddStringToObject(ret_item, JSON_PATCH_PATH, path_str);
                cJSON_AddItemToObject(ret_item, JSON_PATCH_VALUE, ret_node);
                cJSON_AddItemToArray(ret_json, ret_item);
            }
        }
        arr_item = arr_item->next;
    }

    ret_str = cJSON_Print(ret_json);

    cJSON_Delete(ret_json);
    cJSON_Delete(request_json);

    return ret_str;
}

int RkAiqCalibDbV2::CamCalibDbFreeSysStaticCtx(CalibDb_SysStaticCfg_ParaV2_t* sys_static)
{
    CalibDb_AlgoSwitch_t* algoSwitch = &sys_static->algoSwitch;

    if (algoSwitch->disable_algos)
        calib_free(algoSwitch->disable_algos);

    return 0;
}

int RkAiqCalibDbV2::CamCalibDbFreeUapiCtx(RkaiqUapi_t* uapi)
{
    RkaiqSysCtl_t* system = &uapi->system;
    aiq_scene_t* scene = &system->scene;
    if (scene->main_scene)
        calib_free(scene->main_scene);
    if (scene->sub_scene)
        calib_free(scene->sub_scene);

    return 0;
}

int RkAiqCalibDbV2::CamCalibDbFreeSensorCtx(CalibDb_Sensor_ParaV2_t* sensor)
{
    CalibDb_AecGainRangeV2_t* Gain2Reg = &sensor->Gain2Reg;
    if (Gain2Reg->GainRange)
        calib_free(Gain2Reg->GainRange);
    return 0;
}

int RkAiqCalibDbV2::CamCalibDbFreeModuleCtx(CalibDb_Module_ParaV2_t* module)
{
    return 0;
}

int RkAiqCalibDbV2::CamCalibDbFreeAeCtx(CalibDb_Aec_ParaV2_t* ae)
{
    CalibDb_LinearAE_AttrV2_t *LinearAeCtrl = &ae->LinearAeCtrl;
    CalibDb_LinAeRoute_AttrV2_t *Route = &LinearAeCtrl->Route;
    if (Route->TimeDot)
        calib_free(Route->TimeDot);
    if (Route->GainDot)
        calib_free(Route->GainDot);
    if (Route->IspDGainDot)
        calib_free(Route->IspDGainDot);
    if (Route->PIrisDot)
        calib_free(Route->PIrisDot);

    CalibDb_AecDynamicSetpointV2_t* DySetpoint = &LinearAeCtrl->DySetpoint;
    if (DySetpoint->ExpLevel)
        calib_free(DySetpoint->ExpLevel);
    if (DySetpoint->DySetpoint)
        calib_free(DySetpoint->DySetpoint);

    CalibDb_AecBacklightV2_t *BackLightCtrl = &LinearAeCtrl->BackLightCtrl;
    CalibDb_BacklitSetPointV2_t* BacklitSetPoint = &BackLightCtrl->BacklitSetPoint;
    if (BacklitSetPoint->ExpLevel)
        calib_free(BacklitSetPoint->ExpLevel);
    if (BacklitSetPoint->NonOEPdfTh)
        calib_free(BacklitSetPoint->NonOEPdfTh);
    if (BacklitSetPoint->LowLightPdfTh)
        calib_free(BacklitSetPoint->LowLightPdfTh);
    if (BacklitSetPoint->TargetLLLuma)
        calib_free(BacklitSetPoint->TargetLLLuma);

    CalibDb_AecOverExpCtrlV2_t* OverExpCtrl = &LinearAeCtrl->OverExpCtrl;
    CalibDb_OverExpSetPointV2_t* OverExpSetPoint = &OverExpCtrl->OverExpSetPoint;
    if (OverExpSetPoint->OEpdf)
        calib_free(OverExpSetPoint->OEpdf);
    if (OverExpSetPoint->LowLightWeight)
        calib_free(OverExpSetPoint->LowLightWeight);
    if (OverExpSetPoint->HighLightWeight)
        calib_free(OverExpSetPoint->HighLightWeight);

    CalibDb_HdrAE_AttrV2_t* HdrAeCtrl = &ae->HdrAeCtrl;
    CalibDb_ExpRatioCtrlV2_t* ExpRatioCtrl = &HdrAeCtrl->ExpRatioCtrl;
    CalibDb_ExpRatioV2_t* ExpRatio = &ExpRatioCtrl->ExpRatio;
    if (ExpRatio->RatioExpDot)
        calib_free(ExpRatio->RatioExpDot);
    if (ExpRatio->M2SRatioFix)
        calib_free(ExpRatio->M2SRatioFix);
    if (ExpRatio->L2MRatioFix)
        calib_free(ExpRatio->L2MRatioFix);
    if (ExpRatio->M2SRatioMax)
        calib_free(ExpRatio->M2SRatioMax);
    if (ExpRatio->L2MRatioMax)
        calib_free(ExpRatio->L2MRatioMax);

    CalibDb_HdrAeRoute_AttrV2_t* hdr_Route = &HdrAeCtrl->Route;
    if (hdr_Route->Frm0TimeDot)
        calib_free(hdr_Route->Frm0TimeDot);
    if (hdr_Route->Frm0GainDot)
        calib_free(hdr_Route->Frm0GainDot);
    if (hdr_Route->Frm0IspDGainDot)
        calib_free(hdr_Route->Frm0IspDGainDot);
    if (hdr_Route->Frm1TimeDot)
        calib_free(hdr_Route->Frm1TimeDot);
    if (hdr_Route->Frm1GainDot)
        calib_free(hdr_Route->Frm1GainDot);
    if (hdr_Route->Frm1IspDGainDot)
        calib_free(hdr_Route->Frm1IspDGainDot);
    if (hdr_Route->Frm2TimeDot)
        calib_free(hdr_Route->Frm2TimeDot);
    if (hdr_Route->Frm2GainDot)
        calib_free(hdr_Route->Frm2GainDot);
    if (hdr_Route->Frm2IspDGainDot)
        calib_free(hdr_Route->Frm2IspDGainDot);
    if (hdr_Route->PIrisDot)
        calib_free(hdr_Route->PIrisDot);

    CalibDb_LfrmCtrlV2_t* LframeCtrl = &HdrAeCtrl->LframeCtrl;
    CalibDb_LfrmSetPointV2_t* LfrmSetPoint = &LframeCtrl->LfrmSetPoint;
    if (LfrmSetPoint->LExpLevel)
        calib_free(LfrmSetPoint->LExpLevel);
    if (LfrmSetPoint->NonOEPdfTh)
        calib_free(LfrmSetPoint->NonOEPdfTh);
    if (LfrmSetPoint->LowLightPdfTh)
        calib_free(LfrmSetPoint->LowLightPdfTh);
    if (LfrmSetPoint->LSetPoint)
        calib_free(LfrmSetPoint->LSetPoint);
    if (LfrmSetPoint->TargetLLLuma)
        calib_free(LfrmSetPoint->TargetLLLuma);

    CalibDb_MfrmCtrlV2_t* MframeCtrl = &HdrAeCtrl->MframeCtrl;
    if (MframeCtrl->MExpLevel)
        calib_free(MframeCtrl->MExpLevel);
    if (MframeCtrl->MSetPoint)
        calib_free(MframeCtrl->MSetPoint);

    CalibDb_SfrmCtrlV2_t* SframeCtrl = &HdrAeCtrl->SframeCtrl;
    CalibDb_SfrmSetPointV2_t* SfrmSetPoint = &SframeCtrl->SfrmSetPoint;
    if (SfrmSetPoint->SExpLevel)
        calib_free(SfrmSetPoint->SExpLevel);
    if (SfrmSetPoint->SSetPoint)
        calib_free(SfrmSetPoint->SSetPoint);
    if (SfrmSetPoint->TargetHLLuma)
        calib_free(SfrmSetPoint->TargetHLLuma);

    CalibDb_AeSyncTestV2_t* SyncTest = &ae->SyncTest;
    CalibDb_AlterExpV2_t* AlterExp = &SyncTest->AlterExp;
    if (AlterExp->LinearAE)
        calib_free(AlterExp->LinearAE);
    if (AlterExp->HdrAE)
        calib_free(AlterExp->HdrAE);

    return 0;
}
#if RKAIQ_HAVE_AWB_V21
int RkAiqCalibDbV2::CamCalibDbFreeAwbV21Ctx(CalibDbV2_Wb_Para_V21_t* awb)
{
    CalibDbV2_Wb_Awb_Para_V21_t* autoPara = &awb->autoPara;
    if (autoPara->lsUsedForYuvDet) {
        for (int i = 0; i < autoPara->lsUsedForYuvDet_len; i++)
            calib_free(autoPara->lsUsedForYuvDet[i]);
    }
    calib_free(autoPara->lsUsedForYuvDet);

    CalibDbV2_Awb_Luma_Weight_t* wpDiffLumaWeight = &autoPara->wpDiffLumaWeight;
    if (wpDiffLumaWeight->wpDiffWeightLvSet) {
        for (int i = 0; i < wpDiffLumaWeight->wpDiffWeightLvSet_len; i++) {
            CalibDbV2_Awb_Luma_Weight_Lv_t* wpDiffWeightLvSet = wpDiffLumaWeight->wpDiffWeightLvSet + i;
            if (wpDiffWeightLvSet->ratioSet)
                calib_free(wpDiffWeightLvSet->ratioSet);
        }
        calib_free(wpDiffLumaWeight->wpDiffWeightLvSet);
    }

    if (autoPara->lightSources) {
        for (int i = 0; i < autoPara->lightSources_len; i++) {
            CalibDbV2_Awb_Light_V21_t* lightSource = autoPara->lightSources + i;
            calib_free(lightSource->name);
            if(lightSource->weight.lumaValue)
                calib_free(lightSource->weight.lumaValue);
            if(lightSource->weight.weight)
                calib_free(lightSource->weight.weight);
        }
        calib_free(autoPara->lightSources);
    }
    if(autoPara->limitRange.lumaValue)
        calib_free(autoPara->limitRange.lumaValue);
    if(autoPara->limitRange.maxB)
        calib_free(autoPara->limitRange.maxB);
    if(autoPara->limitRange.maxR)
        calib_free(autoPara->limitRange.maxR);
    if(autoPara->limitRange.maxG)
        calib_free(autoPara->limitRange.maxG);
    if(autoPara->limitRange.maxY)
        calib_free(autoPara->limitRange.maxY);
    if(autoPara->limitRange.minB)
        calib_free(autoPara->limitRange.minB);
    if(autoPara->limitRange.minR)
        calib_free(autoPara->limitRange.minR);
    if(autoPara->limitRange.minG)
        calib_free(autoPara->limitRange.minG);
    if(autoPara->limitRange.minY)
        calib_free(autoPara->limitRange.minY);
    CalibDbV2_Wb_Awb_Ext_Com_Para_t* autoExtPara = &awb->autoExtPara;
    if (autoExtPara->lightSourceForFirstFrame)
        calib_free(autoExtPara->lightSourceForFirstFrame);
    CalibDbV2_Awb_Tolerance_t* tolerance = &autoExtPara->tolerance;
    if (tolerance->lumaValue)
        calib_free(tolerance->lumaValue);
    if (tolerance->toleranceValue)
        calib_free(tolerance->toleranceValue);

    CalibDbV2_Awb_runinterval_t* runInterval = &autoExtPara->runInterval;
    if (runInterval->lumaValue)
        calib_free(runInterval->lumaValue);
    if (runInterval->intervalValue)
        calib_free(runInterval->intervalValue);

    CalibDbV2_Awb_GainAdjust_t* wbGainAdjust = &autoExtPara->wbGainAdjust;
    if (wbGainAdjust->lutAll) {
        for (int i = 0; i < wbGainAdjust->lutAll_len; i++) {
            CalibDbV2_Awb_Cct_Lut_Cfg_Lv_t *lutAll = wbGainAdjust->lutAll + i;
            if (lutAll->ct_lut_out)
                calib_free(lutAll->ct_lut_out);
            if (lutAll->cri_lut_out)
                calib_free(lutAll->cri_lut_out);
        }
        calib_free(wbGainAdjust->lutAll);
    }

    CalibDbV2_Awb_Cct_Clip_Cfg_t* wbGainClip = &autoExtPara->wbGainClip;
    if (wbGainClip->cct)
        calib_free(wbGainClip->cct);
    if (wbGainClip->cri_bound_up)
        calib_free(wbGainClip->cri_bound_up);
    if (wbGainClip->cri_bound_low)
        calib_free(wbGainClip->cri_bound_low);

    if (autoExtPara->weightForNightGainCalc)
        calib_free(autoExtPara->weightForNightGainCalc);

    CalibDbV2_Awb_Sgc_t* singleColorProces = &autoExtPara->singleColorProces;
    if (singleColorProces->colorBlock)
        calib_free(singleColorProces->colorBlock);
    if (singleColorProces->lsUsedForEstimation) {
        for (int i = 0; i < singleColorProces->lsUsedForEstimation_len; i++) {
            CalibDbV2_Awb_Sgc_Ls_t *lsUsedForEstimation = singleColorProces->lsUsedForEstimation + i;
            calib_free(lsUsedForEstimation->name);
        }
        calib_free(singleColorProces->lsUsedForEstimation);
    }
    if(autoExtPara->division.wpNumTh.high)
        calib_free(autoExtPara->division.wpNumTh.high);
    if(autoExtPara->division.wpNumTh.low)
        calib_free(autoExtPara->division.wpNumTh.low);
    if(autoExtPara->division.wpNumTh.lumaValue)
        calib_free(autoExtPara->division.wpNumTh.lumaValue);
    if(autoExtPara->xyRegionStableSelection.wpNumTh.lumaValue)
        calib_free(autoExtPara->xyRegionStableSelection.wpNumTh.lumaValue);
    if(autoExtPara->xyRegionStableSelection.wpNumTh.forBigType)
        calib_free(autoExtPara->xyRegionStableSelection.wpNumTh.forBigType);
    if(autoExtPara->xyRegionStableSelection.wpNumTh.forExtraType)
        calib_free(autoExtPara->xyRegionStableSelection.wpNumTh.forExtraType);
    CalibDbV2_Awb_SmartRun_t* smartRun = &autoExtPara->smartRun;
    if (smartRun->cfg.lumaValue)
        calib_free(smartRun->cfg.lumaValue);
    if (smartRun->cfg.lvVarTh)
        calib_free(smartRun->cfg.lvVarTh);
    if (smartRun->cfg.wbgainAlgDiffTh)
        calib_free(smartRun->cfg.wbgainAlgDiffTh);
    if (smartRun->cfg.wbgainHwDiffTh)
        calib_free(smartRun->cfg.wbgainHwDiffTh);
    return 0;
}
#endif
#if RKAIQ_HAVE_AWB_V32
int RkAiqCalibDbV2::CamCalibDbFreeAwbV32Ctx(CalibDbV2_Wb_Para_V32_t* awb)
{
    CalibDbV2_Wb_Awb_Para_V32_t* autoPara = &awb->autoPara;
    CalibDbV2_Awb_offset_data_t* offset = &autoPara->blc2ForAwb.offset;
    if (offset->ISO)
        calib_free(offset->ISO);
    if (offset->R_Channel)
        calib_free(offset->R_Channel);
    if (offset->Gr_Channel)
        calib_free(offset->Gr_Channel);
    if (offset->Gb_Channel)
        calib_free(offset->Gb_Channel);
    if (offset->B_Channel)
        calib_free(offset->B_Channel);


    CalibDbV2_Awb_Luma_Weight_t* wpDiffLumaWeight = &autoPara->wpDiffLumaWeight;
    if (wpDiffLumaWeight->wpDiffWeightLvSet) {
        for (int i = 0; i < wpDiffLumaWeight->wpDiffWeightLvSet_len; i++) {
            CalibDbV2_Awb_Luma_Weight_Lv_t* wpDiffWeightLvSet = wpDiffLumaWeight->wpDiffWeightLvSet + i;
            if (wpDiffWeightLvSet->ratioSet)
                calib_free(wpDiffWeightLvSet->ratioSet);
        }
        calib_free(wpDiffLumaWeight->wpDiffWeightLvSet);
    }

    if (autoPara->lightSources) {
        for (int i = 0; i < autoPara->lightSources_len; i++) {
            CalibDbV2_Awb_Light_V32_t* lightSource = autoPara->lightSources + i;
            calib_free(lightSource->name);
            if(lightSource->weight.lumaValue)
                calib_free(lightSource->weight.lumaValue);
            if(lightSource->weight.weight)
                calib_free(lightSource->weight.weight);
        }
        calib_free(autoPara->lightSources);
    }
    if(autoPara->limitRange.lumaValue)
        calib_free(autoPara->limitRange.lumaValue);
    if(autoPara->limitRange.maxB)
        calib_free(autoPara->limitRange.maxB);
    if(autoPara->limitRange.maxR)
        calib_free(autoPara->limitRange.maxR);
    if(autoPara->limitRange.maxG)
        calib_free(autoPara->limitRange.maxG);
    if(autoPara->limitRange.maxY)
        calib_free(autoPara->limitRange.maxY);
    if(autoPara->limitRange.minB)
        calib_free(autoPara->limitRange.minB);
    if(autoPara->limitRange.minR)
        calib_free(autoPara->limitRange.minR);
    if(autoPara->limitRange.minG)
        calib_free(autoPara->limitRange.minG);
    if(autoPara->limitRange.minY)
        calib_free(autoPara->limitRange.minY);
    for(int i = 0; i < CALD_AWB_EXCRANGE_NUM_MAX; i++) {
        if(autoPara->extraWpRange[i].weightInculde.lumaValue)
            calib_free(autoPara->extraWpRange[i].weightInculde.lumaValue);
        if(autoPara->extraWpRange[i].weightInculde.weight)
            calib_free(autoPara->extraWpRange[i].weightInculde.weight);
    }
    CalibDbV2_Wb_Awb_Ext_Para_V32_t* autoExtPara = &awb->autoExtPara;
    CalibDbV2_Awb_SmartRun_t* smartRun = &autoExtPara->smartRun;
    if (smartRun->cfg.lumaValue)
        calib_free(smartRun->cfg.lumaValue);
    if (smartRun->cfg.lvVarTh)
        calib_free(smartRun->cfg.lvVarTh);
    if (smartRun->cfg.wbgainAlgDiffTh)
        calib_free(smartRun->cfg.wbgainAlgDiffTh);
    if (smartRun->cfg.wbgainHwDiffTh)
        calib_free(smartRun->cfg.wbgainHwDiffTh);


    CalibDbV2_Awb_Tolerance_t* tolerance = &autoExtPara->tolerance;
    if (tolerance->lumaValue)
        calib_free(tolerance->lumaValue);
    if (tolerance->toleranceValue)
        calib_free(tolerance->toleranceValue);

    CalibDbV2_Awb_runinterval_t* runInterval = &autoExtPara->runInterval;
    if (runInterval->lumaValue)
        calib_free(runInterval->lumaValue);
    if (runInterval->intervalValue)
        calib_free(runInterval->intervalValue);

    CalibDbV2_Awb_GainAdjust2_t* wbGainAdjust = &autoExtPara->wbGainAdjust;
    if (wbGainAdjust->lutAll) {
        calib_free(wbGainAdjust->lutAll);
    }

    CalibDbV2_Awb_Cct_Clip_Cfg_t* wbGainClip = &autoExtPara->wbGainClip;
    if (wbGainClip->cct)
        calib_free(wbGainClip->cct);
    if (wbGainClip->cri_bound_up)
        calib_free(wbGainClip->cri_bound_up);
    if (wbGainClip->cri_bound_low)
        calib_free(wbGainClip->cri_bound_low);

    if (autoExtPara->weightForNightGainCalc)
        calib_free(autoExtPara->weightForNightGainCalc);

    CalibDbV2_Awb_Sgc_t* singleColorProces = &autoExtPara->singleColorProces;
    if (singleColorProces->colorBlock)
        calib_free(singleColorProces->colorBlock);
    if (singleColorProces->lsUsedForEstimation) {
        for (int i = 0; i < singleColorProces->lsUsedForEstimation_len; i++) {
            CalibDbV2_Awb_Sgc_Ls_t *lsUsedForEstimation = singleColorProces->lsUsedForEstimation + i;
            calib_free(lsUsedForEstimation->name);
        }
        calib_free(singleColorProces->lsUsedForEstimation);
    }
    if(autoExtPara->division.wpNumTh.high)
        calib_free(autoExtPara->division.wpNumTh.high);
    if(autoExtPara->division.wpNumTh.low)
        calib_free(autoExtPara->division.wpNumTh.low);
    if(autoExtPara->division.wpNumTh.lumaValue)
        calib_free(autoExtPara->division.wpNumTh.lumaValue);
    if(autoExtPara->xyRegionStableSelection.wpNumTh.lumaValue)
        calib_free(autoExtPara->xyRegionStableSelection.wpNumTh.lumaValue);
    if(autoExtPara->xyRegionStableSelection.wpNumTh.forBigType)
        calib_free(autoExtPara->xyRegionStableSelection.wpNumTh.forBigType);
    if(autoExtPara->xyRegionStableSelection.wpNumTh.forExtraType)
        calib_free(autoExtPara->xyRegionStableSelection.wpNumTh.forExtraType);
    return 0;
}
#endif

#if RKAIQ_HAVE_GAMMA_V10
#ifndef USE_NEWSTRUCT
int RkAiqCalibDbV2::CamCalibDbFreeGammaCtx(CalibDbV2_gamma_v10_t* gamma) {
    return 0;
}
#endif
#endif

#if RKAIQ_HAVE_GAMMA_V11
#ifndef USE_NEWSTRUCT
int RkAiqCalibDbV2::CamCalibDbFreeGammaV2Ctx(CalibDbV2_gamma_v11_t* gamma) {
    return 0;
}
#endif
#endif

#if RKAIQ_HAVE_BLC_V1
int RkAiqCalibDbV2::CamCalibDbFreeBlcCtx(CalibDbV2_Ablc_t* blc)
{
    return 0;
}
#endif

#if RKAIQ_HAVE_BLC_V32
int RkAiqCalibDbV2::CamCalibDbFreeBlcV32Ctx(CalibDbV2_Blc_V32_t* blc_v32)
{
    return 0;
}
#endif
#if RKAIQ_HAVE_GIC_V21
int RkAiqCalibDbV2::CamCalibDbFreeGicV21Ctx(CalibDbV2_Gic_V21_t* gic)
{
    CalibDbGicV21_t* GicTuningPara = &gic->GicTuningPara;
    Gic_setting_v21_t* GicData = &GicTuningPara->GicData;
    if (GicData->ISO)
        calib_free(GicData->ISO);
    if (GicData->min_busy_thre)
        calib_free(GicData->min_busy_thre);
    if (GicData->min_grad_thr1)
        calib_free(GicData->min_grad_thr1);
    if (GicData->min_grad_thr2)
        calib_free(GicData->min_grad_thr2);
    if (GicData->k_grad1)
        calib_free(GicData->k_grad1);
    if (GicData->k_grad2)
        calib_free(GicData->k_grad2);
    if (GicData->gb_thre)
        calib_free(GicData->gb_thre);
    if (GicData->maxCorV)
        calib_free(GicData->maxCorV);
    if (GicData->maxCorVboth)
        calib_free(GicData->maxCorVboth);
    if (GicData->dark_thre)
        calib_free(GicData->dark_thre);
    if (GicData->dark_threHi)
        calib_free(GicData->dark_threHi);
    if (GicData->k_grad1_dark)
        calib_free(GicData->k_grad1_dark);
    if (GicData->k_grad2_dark)
        calib_free(GicData->k_grad2_dark);
    if (GicData->min_grad_thr_dark1)
        calib_free(GicData->min_grad_thr_dark1);
    if (GicData->min_grad_thr_dark2)
        calib_free(GicData->min_grad_thr_dark2);
    if (GicData->noiseCurve_0)
        calib_free(GicData->noiseCurve_0);
    if (GicData->noiseCurve_1)
        calib_free(GicData->noiseCurve_1);
    if (GicData->NoiseScale)
        calib_free(GicData->NoiseScale);
    if (GicData->NoiseBase)
        calib_free(GicData->NoiseBase);
    if (GicData->globalStrength)
        calib_free(GicData->globalStrength);
    if (GicData->diff_clip)
        calib_free(GicData->diff_clip);

    return 0;
}
#endif

#ifndef USE_NEWSTRUCT
int RkAiqCalibDbV2::CamCalibDbFreeDpccCtx(CalibDbV2_Dpcc_t* dpcc)
{
    CalibDbDpccV20_t* DpccTuningPara = &dpcc->DpccTuningPara;
    CalibDb_Dpcc_Fast_Mode_V20_t* Fast_Mode = &DpccTuningPara->Fast_Mode;
    FastData_t* Fast_Data = &Fast_Mode->Fast_Data;

    if (Fast_Data->ISO)
        calib_free(Fast_Data->ISO);
    if (Fast_Data->Single_level)
        calib_free(Fast_Data->Single_level);
    if (Fast_Data->Double_level)
        calib_free(Fast_Data->Double_level);
    if (Fast_Data->Triple_level)
        calib_free(Fast_Data->Triple_level);

    CalibDb_Dpcc_Sensor_V20_t* Sensor_dpcc = &DpccTuningPara->Sensor_dpcc;
    SensorDpccData_t* SensorDpcc_Data = &Sensor_dpcc->SensorDpcc_Data;
    if (SensorDpcc_Data->ISO)
        calib_free(SensorDpcc_Data->ISO);
    if (SensorDpcc_Data->level_single)
        calib_free(SensorDpcc_Data->level_single);
    if (SensorDpcc_Data->level_multiple)
        calib_free(SensorDpcc_Data->level_multiple);

    return 0;
}
#endif

#if RKAIQ_HAVE_ORB_V1
int RkAiqCalibDbV2::CamCalibDbFreeOrbCtx(CalibDbV2_Orb_t* orb)
{
    return 0;
}
#endif


#if RKAIQ_HAVE_DEBAYER_V1
int RkAiqCalibDbV2::CamCalibDbFreeDebayerCtx(CalibDbV2_Debayer_t* debayer_v1)
{
    return 0;
}
#endif

#if RKAIQ_HAVE_DEBAYER_V2
int RkAiqCalibDbV2::CamCalibDbFreeDebayerV2Ctx(CalibDbV2_Debayer_v2_t* debayer_v2)
{
    return 0;
}
#endif

#if RKAIQ_HAVE_DEBAYER_V2_LITE
int RkAiqCalibDbV2::CamCalibDbFreeDebayerV2Ctx(CalibDbV2_Debayer_v2_lite_t* debayer_v2)
{
    return 0;
}
#endif
#if RKAIQ_HAVE_DEBAYER_V3
int RkAiqCalibDbV2::CamCalibDbFreeDebayerV3Ctx(CalibDbV2_Debayer_v3_t* debayer_v3)
{
    return 0;
}
#endif

#if RKAIQ_HAVE_ACP_V10
int RkAiqCalibDbV2::CamCalibDbFreeCprocCtx(CalibDbV2_Cproc_t* cproc)
{
    return 0;
}
#endif
#if RKAIQ_HAVE_AIE_V10
int RkAiqCalibDbV2::CamCalibDbFreeIeCtx(CalibDbV2_IE_t* ie)
{
    return 0;
}
#endif

int RkAiqCalibDbV2::CamCalibDbFreeLscCtx(CalibDbV2_LSC_t* lsc)
{
    CalibDbV2_Lsc_Common_t* common = &lsc->common;
    if (common->resolutionAll)
        calib_free(common->resolutionAll);

    CalibDbV2_AlscCof_t* alscCoef = &lsc->alscCoef;
    if (alscCoef->illAll) {
        for (int i = 0; i < alscCoef->illAll_len; i++) {
            CalibDbV2_AlscCof_ill_t* ill = alscCoef->illAll + i;
            if (ill->tableUsed)
                calib_free(ill->tableUsed);
            if (ill->gains)
                calib_free(ill->gains);
            if (ill->vig)
                calib_free(ill->vig);
        }
        calib_free(alscCoef->illAll);
    }

    CalibDbV2_LscTable_t* tbl = &lsc->tbl;
    if (tbl->tableAll)
        calib_free(tbl->tableAll);

    return 0;
}

int RkAiqCalibDbV2::CamCalibDbFreeColorAsGreyCtx(CalibDbV2_ColorAsGrey_t* colorAsGrey)
{
    return 0;
}

int RkAiqCalibDbV2::CamCalibDbFreeLumaDetectCtx(CalibDbV2_LUMA_DETECT_t* lumaDetect)
{
    return 0;
}

#if (RKAIQ_HAVE_LDCH_V10 || RKAIQ_HAVE_LDCH_V21)
int RkAiqCalibDbV2::CamCalibDbFreeLdchCtx(CalibDbV2_LDCH_t* ldch)
{
    return 0;
}
#endif

#if RKAIQ_HAVE_CCM_V1
int RkAiqCalibDbV2::CamCalibDbFreeCcmV1Ctx(CalibDbV2_Ccm_Para_V2_t* ccm) {
    CalibDbV2_Ccm_Tuning_Para_t* TuningPara     = &ccm->TuningPara;
    CalibDbV2_Ccm_illu_est_Para_t* illu_estim = &TuningPara->illu_estim;

    if (illu_estim->default_illu) calib_free(illu_estim->default_illu);

    if (TuningPara->aCcmCof) {
        for (int i = 0; i < TuningPara->aCcmCof_len; i++) {
            CalibDbV2_Ccm_Accm_Cof_Para_t* aCcmCof = TuningPara->aCcmCof + i;
            if (aCcmCof->name) calib_free(aCcmCof->name);
            for (int j = 0; j < aCcmCof->matrixUsed_len; j++) {
                if (aCcmCof->matrixUsed[j]) calib_free(aCcmCof->matrixUsed[j]);
            }
            if (aCcmCof->matrixUsed) calib_free(aCcmCof->matrixUsed);
        }
        calib_free(TuningPara->aCcmCof);
    }

    if (TuningPara->matrixAll) {
        for (int i = 0; i < TuningPara->matrixAll_len; i++) {
            CalibDbV2_Ccm_Matrix_Para_t* matrixAll = TuningPara->matrixAll + i;
            if (matrixAll->name) calib_free(matrixAll->name);
            if (matrixAll->illumination) calib_free(matrixAll->illumination);
        }
        calib_free(TuningPara->matrixAll);
    }

    return 0;
}
#endif

#if RKAIQ_HAVE_CCM_V2 || RKAIQ_HAVE_CCM_V3
template<class T>
int RkAiqCalibDbV2::CamCalibDbFreeCcmV2Ctx(T* ccm) {
    CalibDbV2_Ccm_Tuning_Para_t* TuningPara = &ccm->TuningPara;
    CalibDbV2_Ccm_illu_est_Para_t* illu_estim = &TuningPara->illu_estim;

    if (illu_estim->default_illu)
        calib_free(illu_estim->default_illu);

    if (TuningPara->aCcmCof) {
        for (int i = 0; i < TuningPara->aCcmCof_len; i++) {
            CalibDbV2_Ccm_Accm_Cof_Para_t* aCcmCof = TuningPara->aCcmCof + i;
            if (aCcmCof->name)
                calib_free(aCcmCof->name);
            for ( int j = 0; j < aCcmCof->matrixUsed_len; j++) {
                if (aCcmCof->matrixUsed[j])
                    calib_free(aCcmCof->matrixUsed[j]);
            }
            if (aCcmCof->matrixUsed)
                calib_free(aCcmCof->matrixUsed);
        }
        calib_free(TuningPara->aCcmCof);
    }

    if (TuningPara->matrixAll) {
        for (int i = 0; i < TuningPara->matrixAll_len; i++) {
            CalibDbV2_Ccm_Matrix_Para_t* matrixAll = TuningPara->matrixAll + i;
            if (matrixAll->name)
                calib_free(matrixAll->name);
            if (matrixAll->illumination)
                calib_free(matrixAll->illumination);
        }
        calib_free(TuningPara->matrixAll);
    }

    return 0;
}
#endif

#if RKAIQ_HAVE_3DLUT_V1
int RkAiqCalibDbV2::CamCalibDbFreeLut3dCtx(CalibDbV2_Lut3D_Para_V2_t* lut3d)
{
    CalibDbV2_Lut3D_Auto_Para_t* ALut3D = &lut3d->ALut3D;
    if (ALut3D->lutAll) {
        CalibDbV2_Lut3D_LutPara_t* lutAll = ALut3D->lutAll;
        for (int i = 0; i < ALut3D->lutAll_len; i++) {
            lutAll = lutAll + i;
            if (lutAll->name)
                calib_free(lutAll->name);
        }

        calib_free(ALut3D->lutAll);
    }

    return 0;
}
#endif

#if RKAIQ_HAVE_AF_V20
int RkAiqCalibDbV2::CamCalibDbFreeAfV2xCtx(CalibDbV2_AF_t* af)
{
    return 0;
}
#endif

#if RKAIQ_HAVE_AF_V32_LITE
int RkAiqCalibDbV2::CamCalibDbFreeAfV32Ctx(CalibDbV2_AFV32_t* af)
{
    return 0;
}
#endif

#if RKAIQ_HAVE_AF_V33
int RkAiqCalibDbV2::CamCalibDbFreeAfV33Ctx(CalibDbV2_AFV33_t* af)
{
    return 0;
}
#endif

#if RKAIQ_HAVE_AF_V31
int RkAiqCalibDbV2::CamCalibDbFreeAfV31Ctx(CalibDbV2_AFV31_t* af)
{
    return 0;
}
#endif

#if RKAIQ_HAVE_AF_V30
int RkAiqCalibDbV2::CamCalibDbFreeAfV30Ctx(CalibDbV2_AFV30_t* af)
{
    return 0;
}
#endif

int RkAiqCalibDbV2::CamCalibDbFreeThumbnailsCtx(CalibDbV2_Thumbnails_t* thumbnails)
{
    CalibDbV2_Thumbnails_Param_t* param = &thumbnails->param;
    if (param->thumbnail_configs)
        calib_free(param->thumbnail_configs);

    return 0;
}
#if RKAIQ_HAVE_BAYERNR_V2
int RkAiqCalibDbV2::CamCalibDbFreeBayerNrV2Ctx(CalibDbV2_BayerNrV2_t* bayernr_v1)
{
    if (bayernr_v1->Version)
        calib_free(bayernr_v1->Version);

    CalibDbV2_BayerNrV2_Calib_t* CalibPara = &bayernr_v1->CalibPara;
    for (int i = 0; i < CalibPara->Setting_len; i++) {
        CalibDbV2_BayerNrV2_C_Set_t *Setting = CalibPara->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Calib_ISO)
            calib_free(Setting->Calib_ISO);
    }
    if (CalibPara->Setting)
        calib_free(CalibPara->Setting);

    CalibDbV2_BayerNrV2_2d_t* Bayernr2D = &bayernr_v1->Bayernr2D;
    for (int i = 0; i < CalibPara->Setting_len; i++) {
        CalibDbV2_BayerNrV2_2d_Set_t *Setting = Bayernr2D->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Tuning_ISO)
            calib_free(Setting->Tuning_ISO);
    }
    if (Bayernr2D->Setting)
        calib_free(Bayernr2D->Setting);

    CalibDbV2_BayerNrV2_3d_t * Bayernr3D = &bayernr_v1->Bayernr3D;
    for (int i = 0; i < CalibPara->Setting_len; i++) {
        CalibDbV2_BayerNrV2_3d_Set_t *Setting = Bayernr3D->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Tuning_ISO)
            calib_free(Setting->Tuning_ISO);
    }
    if (Bayernr3D->Setting)
        calib_free(Bayernr3D->Setting);

    return 0;
}
#endif

#if RKAIQ_HAVE_BAYER2DNR_V23
int RkAiqCalibDbV2::CamCalibDbFreeBayer2dnrV23Ctx(CalibDbV2_Bayer2dnrV23_t* bayer2dnr_v23)
{
    if (bayer2dnr_v23->Version)
        calib_free(bayer2dnr_v23->Version);

    CalibDbV2_Bayer2dnrV23_Calib_t* CalibPara = &bayer2dnr_v23->CalibPara;
    for (int i = 0; i < CalibPara->Setting_len; i++) {
        CalibDbV2_Bayer2dnrV23_C_Set_t *Setting = CalibPara->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Calib_ISO)
            calib_free(Setting->Calib_ISO);
    }
    if (CalibPara->Setting)
        calib_free(CalibPara->Setting);

    CalibDbV2_Bayer2dnrV23_Tuning_t* TuningPara = &bayer2dnr_v23->TuningPara;
    for (int i = 0; i < TuningPara->Setting_len; i++) {
        CalibDbV2_Bayer2dnrV23_T_Set_t *Setting = TuningPara->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Tuning_ISO)
            calib_free(Setting->Tuning_ISO);
    }
    if (CalibPara->Setting)
        calib_free(CalibPara->Setting);

    return 0;
}
#endif

#if RKAIQ_HAVE_UVNR_V1
int RkAiqCalibDbV2::CamCalibDbFreeCnrCtx(CalibDbV2_CNR_t* cnr)
{
    if (cnr->Version)
        calib_free(cnr->Version);

    CalibDbV2_CNR_Tuning_t* TuningPara = &cnr->TuningPara;
    for (int i = 0; i < TuningPara->Setting_len; i++) {
        CalibDbV2_CNR_T_Set_t *Setting = TuningPara->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Tuning_ISO)
            calib_free(Setting->Tuning_ISO);
    }
    if (TuningPara->Setting)
        calib_free(TuningPara->Setting);

    return 0;
}
#endif

#if RKAIQ_HAVE_YNR_V2
int RkAiqCalibDbV2::CamCalibDbFreeYnrV2Ctx(CalibDbV2_YnrV2_t* ynr_v2)
{
    if (ynr_v2->Version)
        calib_free(ynr_v2->Version);

    CalibDbV2_YnrV2_Calib_t* CalibPara = &ynr_v2->CalibPara;
    for (int i = 0; i < CalibPara->Setting_len; i++) {
        CalibDbV2_YnrV2_C_Set_t *Setting = CalibPara->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Calib_ISO)
            calib_free(Setting->Calib_ISO);
    }
    if (CalibPara->Setting)
        calib_free(CalibPara->Setting);

    CalibDbV2_YnrV2_Tuning_t* TuningPara = &ynr_v2->TuningPara;
    for (int i = 0; i < TuningPara->Setting_len; i++) {
        CalibDbV2_YnrV2_T_Set_t *Setting = TuningPara->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Tuning_ISO)
            calib_free(Setting->Tuning_ISO);
    }
    if (TuningPara->Setting)
        calib_free(TuningPara->Setting);

    return 0;
}
#endif

#if RKAIQ_HAVE_SHARP_V3
int RkAiqCalibDbV2::CamCalibDbFreeSharpV3Ctx(CalibDbV2_SharpV3_t* sharp_v3)
{
    if (sharp_v3->Version)
        calib_free(sharp_v3->Version);

    CalibDbV2_SharpV3_Tuning_t* TuningPara = &sharp_v3->TuningPara;
    for (int i = 0; i < TuningPara->Setting_len; i++) {
        CalibDbV2_SharpV3_T_Set_t *Setting = TuningPara->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Tuning_ISO)
            calib_free(Setting->Tuning_ISO);
    }
    if (TuningPara->Setting)
        calib_free(TuningPara->Setting);

    return 0;
}
#endif

#if RKAIQ_HAVE_BAYER2DNR_V2
int RkAiqCalibDbV2::CamCalibDbFreeBayer2dnrV2Ctx(CalibDbV2_Bayer2dnrV2_t* bayer2dnr_v2)
{
    if (bayer2dnr_v2->Version)
        calib_free(bayer2dnr_v2->Version);

    CalibDbV2_Bayer2dnrV2_Calib_t* CalibPara = &bayer2dnr_v2->CalibPara;
    for (int i = 0; i < CalibPara->Setting_len; i++) {
        CalibDbV2_Bayer2dnrV2_C_Set_t *Setting = CalibPara->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Calib_ISO)
            calib_free(Setting->Calib_ISO);
    }
    if (CalibPara->Setting)
        calib_free(CalibPara->Setting);

    CalibDbV2_Bayer2dnrV2_Tuning_t* TuningPara = &bayer2dnr_v2->TuningPara;
    for (int i = 0; i < TuningPara->Setting_len; i++) {
        CalibDbV2_Bayer2dnrV2_T_Set_t *Setting = TuningPara->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Tuning_ISO)
            calib_free(Setting->Tuning_ISO);
    }
    if (TuningPara->Setting)
        calib_free(TuningPara->Setting);


    return 0;
}
#endif

#if RKAIQ_HAVE_BAYERTNR_V2
int RkAiqCalibDbV2::CamCalibDbFreeBayertnrV2Ctx(CalibDbV2_BayerTnrV2_t* bayertnr_v2)
{
    if (bayertnr_v2->Version)
        calib_free(bayertnr_v2->Version);

    CalibDbV2_BayerTnrV2_Calib_t* CalibPara = &bayertnr_v2->CalibPara;
    for (int i = 0; i < CalibPara->Setting_len; i++) {
        CalibDbV2_BayerTnrV2_C_Set_t *Setting = CalibPara->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Calib_ISO)
            calib_free(Setting->Calib_ISO);
    }
    if (CalibPara->Setting)
        calib_free(CalibPara->Setting);

    CalibDbV2_BayerTnrV2_Tuning_t* TuningPara = &bayertnr_v2->TuningPara;
    for (int i = 0; i < TuningPara->Setting_len; i++) {
        CalibDbV2_BayerTnrV2_T_Set_t *Setting = TuningPara->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Tuning_ISO)
            calib_free(Setting->Tuning_ISO);
    }
    if (TuningPara->Setting)
        calib_free(TuningPara->Setting);


    return 0;
}
#endif

#if RKAIQ_HAVE_BAYERTNR_V23
#ifndef USE_NEWSTRUCT
int RkAiqCalibDbV2::CamCalibDbFreeBayertnrV23Ctx(CalibDbV2_BayerTnrV23_t* bayertnr_v23)
{
    if (bayertnr_v23->Version)
        calib_free(bayertnr_v23->Version);

    CalibDbV2_BayerTnrV23_Calib_t* CalibPara = &bayertnr_v23->CalibPara;
    for (int i = 0; i < CalibPara->Setting_len; i++) {
        CalibDbV2_BayerTnrV23_C_Set_t *Setting = CalibPara->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Calib_ISO)
            calib_free(Setting->Calib_ISO);
    }
    if (CalibPara->Setting)
        calib_free(CalibPara->Setting);

    CalibDbV2_BayerTnrV23_Tuning_t* TuningPara = &bayertnr_v23->TuningPara;

    for (int i = 0; i < TuningPara->Setting_len; i++) {
        CalibDbV2_BayerTnrV23_T_Set_t *Setting = TuningPara->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Tuning_ISO)
            calib_free(Setting->Tuning_ISO);
    }
    if (TuningPara->Setting)
        calib_free(TuningPara->Setting);

    return 0;
}
#endif
#endif

#if RKAIQ_HAVE_BAYERTNR_V23_LITE
int RkAiqCalibDbV2::CamCalibDbFreeBayertnrV23LiteCtx(
    CalibDbV2_BayerTnrV23Lite_t* bayertnr_v23_lite) {
    if (bayertnr_v23_lite->Version) calib_free(bayertnr_v23_lite->Version);

    CalibDbV2_BayerTnrV23_Calib_t* CalibPara = &bayertnr_v23_lite->CalibPara;
    for (int i = 0; i < CalibPara->Setting_len; i++) {
        CalibDbV2_BayerTnrV23_C_Set_t* Setting = CalibPara->Setting + i;

        if (Setting->SNR_Mode) calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode) calib_free(Setting->Sensor_Mode);
        if (Setting->Calib_ISO) calib_free(Setting->Calib_ISO);
    }
    if (CalibPara->Setting) calib_free(CalibPara->Setting);

    CalibDbV2_BayerTnrV23L_Tuning_t* TuningPara = &bayertnr_v23_lite->TuningPara;

    for (int i = 0; i < TuningPara->Setting_len; i++) {
        CalibDbV2_BayerTnrV23L_T_Set_t* Setting = TuningPara->Setting + i;

        if (Setting->SNR_Mode) calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode) calib_free(Setting->Sensor_Mode);
        if (Setting->Tuning_ISO) calib_free(Setting->Tuning_ISO);
    }
    if (TuningPara->Setting) calib_free(TuningPara->Setting);

    return 0;
}
#endif

#if RKAIQ_HAVE_CNR_V2
#if (USE_NEWSTRUCT == 0)
int RkAiqCalibDbV2::CamCalibDbFreeCnrV2Ctx(CalibDbV2_CNRV2_t* cnr_v2)
{
    if (cnr_v2->Version)
        calib_free(cnr_v2->Version);

    CalibDbV2_CNRV2_Tuning_t* TuningPara = &cnr_v2->TuningPara;
    for (int i = 0; i < TuningPara->Setting_len; i++) {
        CalibDbV2_CNRV2_T_Set_t *Setting = TuningPara->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Tuning_ISO)
            calib_free(Setting->Tuning_ISO);
    }
    if (TuningPara->Setting)
        calib_free(TuningPara->Setting);

    return 0;
}
#endif
#endif

#if (RKAIQ_HAVE_CNR_V30 || RKAIQ_HAVE_CNR_V30_LITE)
int RkAiqCalibDbV2::CamCalibDbFreeCnrV30Ctx(CalibDbV2_CNRV30_t* cnr_v30)
{
    if (cnr_v30->Version)
        calib_free(cnr_v30->Version);

    CalibDbV2_CNRV30_Tuning_t* TuningPara = &cnr_v30->TuningPara;
    for (int i = 0; i < TuningPara->Setting_len; i++) {
        CalibDbV2_CNRV30_T_Set_t *Setting = TuningPara->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Tuning_ISO)
            calib_free(Setting->Tuning_ISO);
    }
    if (TuningPara->Setting)
        calib_free(TuningPara->Setting);

    return 0;
}
#endif

#if RKAIQ_HAVE_GAIN_V2
int RkAiqCalibDbV2::CamCalibDbFreeGainV2Ctx(CalibDbV2_GainV2_t* gain_v2)
{
    if (gain_v2->Version)
        calib_free(gain_v2->Version);

    CalibDbV2_GainV2_Tuning_t* TuningPara = &gain_v2->TuningPara;
    for (int i = 0; i < TuningPara->Setting_len; i++) {
        CalibDbV2_GainV2_T_Set_t *Setting = TuningPara->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Tuning_ISO)
            calib_free(Setting->Tuning_ISO);
    }
    if (TuningPara->Setting)
        calib_free(TuningPara->Setting);

    return 0;
}
#endif


#if RKAIQ_HAVE_YNR_V3
int RkAiqCalibDbV2::CamCalibDbFreeYnrV3Ctx(CalibDbV2_YnrV3_t* ynr_v3)
{
    if (ynr_v3->Version)
        calib_free(ynr_v3->Version);

    CalibDbV2_YnrV3_Calib_t* CalibPara = &ynr_v3->CalibPara;
    for (int i = 0; i < CalibPara->Setting_len; i++) {
        CalibDbV2_YnrV3_C_Set_t *Setting = CalibPara->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Calib_ISO)
            calib_free(Setting->Calib_ISO);
    }
    if (CalibPara->Setting)
        calib_free(CalibPara->Setting);

    CalibDbV2_YnrV3_Tuning_t* TuningPara = &ynr_v3->TuningPara;
    for (int i = 0; i < TuningPara->Setting_len; i++) {
        CalibDbV2_YnrV3_T_Set_t *Setting = TuningPara->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Tuning_ISO)
            calib_free(Setting->Tuning_ISO);
    }
    if (TuningPara->Setting)
        calib_free(TuningPara->Setting);

    return 0;
}
#endif

#if RKAIQ_HAVE_YNR_V22
#if (USE_NEWSTRUCT == 0)
int RkAiqCalibDbV2::CamCalibDbFreeYnrV22Ctx(CalibDbV2_YnrV22_t* ynr_v22)
{
    if (ynr_v22->Version)
        calib_free(ynr_v22->Version);

    CalibDbV2_YnrV22_Calib_t* CalibPara = &ynr_v22->CalibPara;
    for (int i = 0; i < CalibPara->Setting_len; i++) {
        CalibDbV2_YnrV22_C_Set_t *Setting = CalibPara->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Calib_ISO)
            calib_free(Setting->Calib_ISO);
    }
    if (CalibPara->Setting)
        calib_free(CalibPara->Setting);

    CalibDbV2_YnrV22_Tuning_t* TuningPara = &ynr_v22->TuningPara;
    for (int i = 0; i < TuningPara->Setting_len; i++) {
        CalibDbV2_YnrV22_T_Set_t *Setting = TuningPara->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Tuning_ISO)
            calib_free(Setting->Tuning_ISO);
    }
    if (TuningPara->Setting)
        calib_free(TuningPara->Setting);

    return 0;
}
#endif
#endif

#if RKAIQ_HAVE_YNR_V24
#if (USE_NEWSTRUCT == 0)
int RkAiqCalibDbV2::CamCalibDbFreeYnrV24Ctx(CalibDbV2_YnrV24_t* ynr_v24) {
    if (ynr_v24->Version) calib_free(ynr_v24->Version);

    CalibDbV2_YnrV24_Calib_t* CalibPara = &ynr_v24->CalibPara;
    for (int i = 0; i < CalibPara->Setting_len; i++) {
        CalibDbV2_YnrV24_C_Set_t* Setting = CalibPara->Setting + i;

        if (Setting->SNR_Mode) calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode) calib_free(Setting->Sensor_Mode);
        if (Setting->Calib_ISO) calib_free(Setting->Calib_ISO);
    }
    if (CalibPara->Setting) calib_free(CalibPara->Setting);

    CalibDbV2_YnrV24_Tuning_t* TuningPara = &ynr_v24->TuningPara;
    for (int i = 0; i < TuningPara->Setting_len; i++) {
        CalibDbV2_YnrV24_T_Set_t* Setting = TuningPara->Setting + i;

        if (Setting->SNR_Mode) calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode) calib_free(Setting->Sensor_Mode);
        if (Setting->Tuning_ISO) calib_free(Setting->Tuning_ISO);
    }
    if (TuningPara->Setting) calib_free(TuningPara->Setting);

    return 0;
}
#endif
#endif

#if RKAIQ_HAVE_SHARP_V4
int RkAiqCalibDbV2::CamCalibDbFreeSharpV4Ctx(CalibDbV2_SharpV4_t* sharp_v4)
{
    if (sharp_v4->Version)
        calib_free(sharp_v4->Version);

    CalibDbV2_SharpV4_Tuning_t* TuningPara = &sharp_v4->TuningPara;
    for (int i = 0; i < TuningPara->Setting_len; i++) {
        CalibDbV2_SharpV4_Set_t *Setting = TuningPara->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Tuning_ISO)
            calib_free(Setting->Tuning_ISO);
    }
    if (TuningPara->Setting)
        calib_free(TuningPara->Setting);

    return 0;
}
#endif

#if RKAIQ_HAVE_SHARP_V33
int RkAiqCalibDbV2::CamCalibDbFreeSharpV33Ctx(CalibDbV2_SharpV33_t* sharp_v33)
{
    if (sharp_v33->Version)
        calib_free(sharp_v33->Version);

    CalibDbV2_SharpV33_Tuning_t* TuningPara = &sharp_v33->TuningPara;

    for (int i = 0; i < TuningPara->Setting_len; i++) {
        CalibDbV2_SharpV33_T_Set_t *Setting = TuningPara->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Tuning_ISO)
            calib_free(Setting->Tuning_ISO);
    }
    if (TuningPara->Setting)
        calib_free(TuningPara->Setting);

    return 0;
}
#endif

#if RKAIQ_HAVE_SHARP_V33_LITE
int RkAiqCalibDbV2::CamCalibDbFreeSharpV33LiteCtx(CalibDbV2_SharpV33Lite_t* sharp_v33) {
    if (sharp_v33->Version) calib_free(sharp_v33->Version);

    CalibDbV2_SharpV33LT_Tuning_t* TuningPara = &sharp_v33->TuningPara;

    for (int i = 0; i < TuningPara->Setting_len; i++) {
        CalibDbV2_SharpV33LT_T_Set_t* Setting = TuningPara->Setting + i;

        if (Setting->SNR_Mode) calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode) calib_free(Setting->Sensor_Mode);
        if (Setting->Tuning_ISO) calib_free(Setting->Tuning_ISO);
    }
    if (TuningPara->Setting) calib_free(TuningPara->Setting);

    return 0;
}
#endif

#if RKAIQ_HAVE_SHARP_V34
int RkAiqCalibDbV2::CamCalibDbFreeSharpV34Ctx(CalibDbV2_SharpV34_t* sharp_v34) {
    if (sharp_v34->Version) calib_free(sharp_v34->Version);

    CalibDbV2_SharpV34_Tuning_t* TuningPara = &sharp_v34->TuningPara;

    for (int i = 0; i < TuningPara->Setting_len; i++) {
        CalibDbV2_SharpV34_T_Set_t* Setting = TuningPara->Setting + i;

        if (Setting->SNR_Mode) calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode) calib_free(Setting->Sensor_Mode);
        if (Setting->Tuning_ISO) calib_free(Setting->Tuning_ISO);
    }
    if (TuningPara->Setting) calib_free(TuningPara->Setting);

    return 0;
}
#endif

#if RKAIQ_HAVE_CAC_V03
int RkAiqCalibDbV2::CamCalibDbFreeCacV03Ctx(CalibDbV2_Cac_V03_t* cac_calib) {
    if (cac_calib->TuningPara.SettingByIso) calib_free(cac_calib->TuningPara.SettingByIso);

    return 0;
}
#endif

#if RKAIQ_HAVE_CAC_V10
int RkAiqCalibDbV2::CamCalibDbFreeCacV10Ctx(CalibDbV2_Cac_V10_t* cac_calib) {
    if (cac_calib->TuningPara.SettingByIso) calib_free(cac_calib->TuningPara.SettingByIso);

    return 0;
}
#endif

#if RKAIQ_HAVE_CAC_V11
int RkAiqCalibDbV2::CamCalibDbFreeCacV11Ctx(CalibDbV2_Cac_V11_t* cac_calib) {
    if (cac_calib->TuningPara.SettingByIso) calib_free(cac_calib->TuningPara.SettingByIso);

    return 0;
}
#endif

#if RKAIQ_HAVE_BAYERTNR_V30
int RkAiqCalibDbV2::CamCalibDbFreeBayertnrV30Ctx(CalibDbV2_BayerTnrV30_t* bayertnr_v30)
{
    if (bayertnr_v30->Version)
        calib_free(bayertnr_v30->Version);

    CalibDbV2_BayerTnrV30_Calib_t* CalibPara = &bayertnr_v30->CalibPara;
    for (int i = 0; i < CalibPara->Setting_len; i++) {
        CalibDbV2_BayerTnrV30_C_Set_t *Setting = CalibPara->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Calib_ISO)
            calib_free(Setting->Calib_ISO);
    }
    if (CalibPara->Setting)
        calib_free(CalibPara->Setting);

    CalibDbV2_BayerTnrV30_Tuning_t* TuningPara = &bayertnr_v30->TuningPara;

    for (int i = 0; i < TuningPara->Setting_len; i++) {
        CalibDbV2_BayerTnrV30_T_Set_t *Setting = TuningPara->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Tuning_ISO)
            calib_free(Setting->Tuning_ISO);
    }
    if (TuningPara->Setting)
        calib_free(TuningPara->Setting);

    return 0;
}
#endif


#if RKAIQ_HAVE_YUVME_V1
int RkAiqCalibDbV2::CamCalibDbFreeYuvmeV1Ctx(CalibDbV2_YuvmeV1_t* yuvme_v1)
{
    if (yuvme_v1->Version)
        calib_free(yuvme_v1->Version);


    CalibDbV2_YuvmeV1_Tuning_t* TuningPara = &yuvme_v1->TuningPara;
    for (int i = 0; i < TuningPara->Setting_len; i++) {
        CalibDbV2_YuvmeV1_T_Set_t *Setting = TuningPara->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Tuning_ISO)
            calib_free(Setting->Tuning_ISO);
    }
    if (TuningPara->Setting)
        calib_free(TuningPara->Setting);

    return 0;
}
#endif

#if (RKAIQ_HAVE_CNR_V31)
#if (USE_NEWSTRUCT == 0)
int RkAiqCalibDbV2::CamCalibDbFreeCnrV31Ctx(CalibDbV2_CNRV31_t* cnr_v31)
{
    if (cnr_v31->Version)
        calib_free(cnr_v31->Version);

    CalibDbV2_CNRV31_Tuning_t* TuningPara = &cnr_v31->TuningPara;
    for (int i = 0; i < TuningPara->Setting_len; i++) {
        CalibDbV2_CNRV31_T_Set_t *Setting = TuningPara->Setting + i;

        if (Setting->SNR_Mode)
            calib_free(Setting->SNR_Mode);
        if (Setting->Sensor_Mode)
            calib_free(Setting->Sensor_Mode);
        if (Setting->Tuning_ISO)
            calib_free(Setting->Tuning_ISO);
    }
    if (TuningPara->Setting)
        calib_free(TuningPara->Setting);

    return 0;
}
#endif
#endif


int RkAiqCalibDbV2::FreeCalibByJ2S(void* ctx) {
    const std::lock_guard<std::mutex> lock(RkAiqCalibDbV2::calib_mutex);
    if (!ctx) {
        return -1;
    }
    j2s_ctx temp_ctx;
    CamCalibDbV2Tuning_t tuning_base;

    j2s_init(&temp_ctx);
    memset(&tuning_base, 0, sizeof(CamCalibDbV2Tuning_t));
    calibdbV2_to_tuningdb(&tuning_base, (CamCalibDbV2Context_t*)ctx);
    j2s_struct_free(&temp_ctx, "CamCalibDbV2Tuning_t", &tuning_base);
    j2s_deinit(&temp_ctx);

    calibdbV2_ctx_delete((CamCalibDbV2Context_t*)ctx);

    return 0;
}

int RkAiqCalibDbV2::CamCalibDbFreeSceneCtx(void* scene_ctx) {
    CamCalibDbV2Context_t ctx_temp;
    ctx_temp.calib_scene = (char*)scene_ctx;

    CamCalibDbV2Context_t* ctx = &ctx_temp;
#if RKAIQ_HAVE_CCM_V1
    CalibDbV2_Ccm_Para_V2_t* ccm_calib =
        (CalibDbV2_Ccm_Para_V2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, ccm_calib));
    if (ccm_calib) CamCalibDbFreeCcmV1Ctx(ccm_calib);
#endif

#if RKAIQ_HAVE_CCM_V2
    CalibDbV2_Ccm_Para_V32_t* ccm_calib_v2 =
        (CalibDbV2_Ccm_Para_V32_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, ccm_calib_v2));
#elif RKAIQ_HAVE_CCM_V3
    CalibDbV2_Ccm_Para_V39_t* ccm_calib_v2 =
        (CalibDbV2_Ccm_Para_V39_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, ccm_calib_v2));
#endif
#if RKAIQ_HAVE_CCM_V2 || RKAIQ_HAVE_CCM_V3
    if (ccm_calib_v2) CamCalibDbFreeCcmV2Ctx(ccm_calib_v2);
#endif



#if 0  // TODO: move out
    CalibDb_Module_ParaV2_t *module_calib =
        (CalibDb_Module_ParaV2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, module_calib));
    CamCalibDbFreeModuleCtx(module_calib);
#endif
    CalibDb_Aec_ParaV2_t* ae_calib =
        (CalibDb_Aec_ParaV2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, ae_calib));
    if (ae_calib) CamCalibDbFreeAeCtx(ae_calib);

#if RKAIQ_HAVE_AWB_V21
    CalibDbV2_Wb_Para_V21_t* wb_v21 =
        (CalibDbV2_Wb_Para_V21_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, wb_v21));
    if (wb_v21) CamCalibDbFreeAwbV21Ctx(wb_v21);
#endif
#if RKAIQ_HAVE_AWB_V32
    CalibDbV2_Wb_Para_V32_t* wb_v32 =
        (CalibDbV2_Wb_Para_V32_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, wb_v32));
    if (wb_v32) CamCalibDbFreeAwbV32Ctx(wb_v32);
#endif

#if RKAIQ_HAVE_GAMMA_V10 || RKAIQ_HAVE_GAMMA_V11
#ifndef USE_NEWSTRUCT
    CalibDbV2_gamma_v10_t* agamma_calib =
        (CalibDbV2_gamma_v10_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, agamma_calib));
    if (agamma_calib) {
#if RKAIQ_HAVE_GAMMA_V11
        CamCalibDbFreeGammaV2Ctx((CalibDbV2_gamma_v11_t*)agamma_calib);
#endif
#if RKAIQ_HAVE_GAMMA_V10
        CamCalibDbFreeGammaCtx(agamma_calib);
#endif
    }
#endif
#endif

#if RKAIQ_HAVE_BLC_V1
    CalibDbV2_Ablc_t* ablc_calib =
        (CalibDbV2_Ablc_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, ablc_calib));
    if (ablc_calib) CamCalibDbFreeBlcCtx(ablc_calib);
#endif

#if RKAIQ_HAVE_BLC_V32
    CalibDbV2_Blc_V32_t* ablcV32_calib =
        (CalibDbV2_Blc_V32_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, ablcV32_calib));
    if (ablcV32_calib) CamCalibDbFreeBlcV32Ctx(ablcV32_calib);
#endif

#if RKAIQ_HAVE_GIC_V21
    CalibDbV2_Gic_V21_t *agic_calib_v21 =
        (CalibDbV2_Gic_V21_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, agic_calib_v21));
    if (agic_calib_v21)
        CamCalibDbFreeGicV21Ctx(agic_calib_v21);
#endif

#if RKAIQ_HAVE_DPCC_V1 || RKAIQ_HAVE_DPCC_V2
#ifndef USE_NEWSTRUCT
    CalibDbV2_Dpcc_t *adpcc_calib =
        (CalibDbV2_Dpcc_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, adpcc_calib));
    if (adpcc_calib)
        CamCalibDbFreeDpccCtx(adpcc_calib);
#endif
#endif

#if RKAIQ_HAVE_ORB_V1
    CalibDbV2_Orb_t* orb = (CalibDbV2_Orb_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, orb));
    if (orb) CamCalibDbFreeOrbCtx(orb);
#endif


#if RKAIQ_HAVE_DEBAYER_V1
    CalibDbV2_Debayer_t* debayer =
        (CalibDbV2_Debayer_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, debayer));
    if (debayer) CamCalibDbFreeDebayerCtx(debayer);
#endif

#if RKAIQ_HAVE_DEBAYER_V2
    CalibDbV2_Debayer_v2_t* debayer =
        (CalibDbV2_Debayer_v2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, debayer));
    if (debayer) CamCalibDbFreeDebayerV2Ctx(debayer);
#endif

#if RKAIQ_HAVE_DEBAYER_V2_LITE
    CalibDbV2_Debayer_v2_lite_t* debayer =
        (CalibDbV2_Debayer_v2_lite_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, debayer));
    if (debayer) CamCalibDbFreeDebayerV2Ctx(debayer);
#endif

#if RKAIQ_HAVE_DEBAYER_V3
    CalibDbV2_Debayer_v3_t* debayer =
        (CalibDbV2_Debayer_v3_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, debayer));
    if (debayer) CamCalibDbFreeDebayerV3Ctx(debayer);
#endif

#if RKAIQ_HAVE_ACP_V10
    CalibDbV2_Cproc_t* cproc = (CalibDbV2_Cproc_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, cproc));
    if (cproc) CamCalibDbFreeCprocCtx(cproc);
#endif
#if RKAIQ_HAVE_AIE_V10
    CalibDbV2_IE_t* ie = (CalibDbV2_IE_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, ie));
    if (ie) CamCalibDbFreeIeCtx(ie);
#endif

    CalibDbV2_LSC_t* lsc_v2 = (CalibDbV2_LSC_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, lsc_v2));
    if (lsc_v2) CamCalibDbFreeLscCtx(lsc_v2);

    CalibDbV2_ColorAsGrey_t* colorAsGrey =
        (CalibDbV2_ColorAsGrey_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, colorAsGrey));
    if (colorAsGrey) CamCalibDbFreeColorAsGreyCtx(colorAsGrey);

    CalibDbV2_LUMA_DETECT_t* lumaDetect =
        (CalibDbV2_LUMA_DETECT_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, lumaDetect));
    if (lumaDetect) CamCalibDbFreeLumaDetectCtx(lumaDetect);

#if (RKAIQ_HAVE_LDCH_V10 || RKAIQ_HAVE_LDCH_V21)
    CalibDbV2_LDCH_t* aldch = (CalibDbV2_LDCH_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, aldch));
    if (aldch) CamCalibDbFreeLdchCtx(aldch);
#endif
#if RKAIQ_HAVE_3DLUT_V1
    CalibDbV2_Lut3D_Para_V2_t* lut3d_calib =
        (CalibDbV2_Lut3D_Para_V2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, lut3d_calib));
    if (lut3d_calib) CamCalibDbFreeLut3dCtx(lut3d_calib);
#endif
#if RKAIQ_HAVE_AF_V33
    CalibDbV2_AFV33_t* af_v33 = (CalibDbV2_AFV33_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, af_v33));
    if (af_v33) CamCalibDbFreeAfV33Ctx(af_v33);
#endif
#if RKAIQ_HAVE_AF_V32_LITE
    CalibDbV2_AFV32_t* af_v32 = (CalibDbV2_AFV32_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, af_v32));
    if (af_v32) CamCalibDbFreeAfV32Ctx(af_v32);
#endif
#if RKAIQ_HAVE_AF_V31
    CalibDbV2_AFV31_t* af_v31 = (CalibDbV2_AFV31_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, af_v31));
    if (af_v31) CamCalibDbFreeAfV31Ctx(af_v31);
#endif
#if RKAIQ_HAVE_AF_V30
    CalibDbV2_AFV30_t* af_v30 = (CalibDbV2_AFV30_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, af_v30));
    if (af_v30) CamCalibDbFreeAfV30Ctx(af_v30);
#endif
#if RKAIQ_HAVE_AF_V20
    CalibDbV2_AF_t* af = (CalibDbV2_AF_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, af));
    if (af) CamCalibDbFreeAfV2xCtx(af);
#endif
    CalibDbV2_Thumbnails_t* thumbnails =
        (CalibDbV2_Thumbnails_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, thumbnails));
    if (thumbnails) CamCalibDbFreeThumbnailsCtx(thumbnails);

#if RKAIQ_HAVE_BAYER2DNR_V2
    CalibDbV2_Bayer2dnrV2_t* bayer2dnr_v2 =
        (CalibDbV2_Bayer2dnrV2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, bayer2dnr_v2));
    if (bayer2dnr_v2) CamCalibDbFreeBayer2dnrV2Ctx(bayer2dnr_v2);
#endif

#if RKAIQ_HAVE_BAYER2DNR_V23
    CalibDbV2_Bayer2dnrV23_t* bayer2dnr_v23 =
        (CalibDbV2_Bayer2dnrV23_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, bayer2dnr_v23));
    if (bayer2dnr_v23) CamCalibDbFreeBayer2dnrV23Ctx(bayer2dnr_v23);
#endif

#if RKAIQ_HAVE_BAYERTNR_V2
    CalibDbV2_BayerTnrV2_t* bayertnr_v2 =
        (CalibDbV2_BayerTnrV2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, bayertnr_v2));
    if (bayertnr_v2) CamCalibDbFreeBayertnrV2Ctx(bayertnr_v2);
#endif

#if RKAIQ_HAVE_BAYERTNR_V23
#ifndef USE_NEWSTRUCT
    CalibDbV2_BayerTnrV23_t* bayertnr_v23 =
        (CalibDbV2_BayerTnrV23_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, bayertnr_v23));
    if (bayertnr_v23) CamCalibDbFreeBayertnrV23Ctx(bayertnr_v23);
#endif
#endif

#if RKAIQ_HAVE_BAYERTNR_V23_LITE
    CalibDbV2_BayerTnrV23Lite_t* bayertnr_v23_lite =
        (CalibDbV2_BayerTnrV23Lite_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, bayertnr_v23_lite));
    if (bayertnr_v23_lite) CamCalibDbFreeBayertnrV23LiteCtx(bayertnr_v23_lite);
#endif

#if RKAIQ_HAVE_CNR_V2
#if (USE_NEWSTRUCT == 0)
    CalibDbV2_CNRV2_t* cnr_v2 = (CalibDbV2_CNRV2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, cnr_v2));
    if (cnr_v2) CamCalibDbFreeCnrV2Ctx(cnr_v2);
#endif
#endif

#if (RKAIQ_HAVE_CNR_V30 || RKAIQ_HAVE_CNR_V30_LITE)
    CalibDbV2_CNRV30_t* cnr_v30 = (CalibDbV2_CNRV30_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, cnr_v30));
    if (cnr_v30) CamCalibDbFreeCnrV30Ctx(cnr_v30);
#endif

#if RKAIQ_HAVE_YNR_V3
    CalibDbV2_YnrV3_t* ynr_v3 = (CalibDbV2_YnrV3_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, ynr_v3));
    if (ynr_v3) CamCalibDbFreeYnrV3Ctx(ynr_v3);
#endif

#if RKAIQ_HAVE_YNR_V22
#if (USE_NEWSTRUCT == 0)
    CalibDbV2_YnrV22_t* ynr_v22 = (CalibDbV2_YnrV22_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, ynr_v22));
    if (ynr_v22) CamCalibDbFreeYnrV22Ctx(ynr_v22);
#endif
#endif

#if RKAIQ_HAVE_YNR_V24
#if (USE_NEWSTRUCT == 0)
    CalibDbV2_YnrV24_t* ynr_v24 =
        (CalibDbV2_YnrV24_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, ynr_v24));
    if (ynr_v24) CamCalibDbFreeYnrV24Ctx(ynr_v24);
#endif
#endif

#if RKAIQ_HAVE_SHARP_V4
    CalibDbV2_SharpV4_t* sharp_v4 =
        (CalibDbV2_SharpV4_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, sharp_v4));
    if (sharp_v4) CamCalibDbFreeSharpV4Ctx(sharp_v4);
#endif

#if RKAIQ_HAVE_SHARP_V33
    CalibDbV2_SharpV33_t* sharp_v33 =
        (CalibDbV2_SharpV33_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, sharp_v33));
    if (sharp_v33) CamCalibDbFreeSharpV33Ctx(sharp_v33);
#endif
#if RKAIQ_HAVE_SHARP_V33_LITE
    CalibDbV2_SharpV33Lite_t* sharp_v33 =
        (CalibDbV2_SharpV33Lite_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, sharp_v33));
    if (sharp_v33) CamCalibDbFreeSharpV33LiteCtx(sharp_v33);
#endif
#if RKAIQ_HAVE_SHARP_V34
    CalibDbV2_SharpV34_t* sharp_v34 =
        (CalibDbV2_SharpV34_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, sharp_v34));
    if (sharp_v34) CamCalibDbFreeSharpV34Ctx(sharp_v34);
#endif
#if RKAIQ_HAVE_CAC_V03
    CalibDbV2_Cac_V03_t* cac_calib =
        (CalibDbV2_Cac_V03_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, cac_v03));
    if (cac_calib) CamCalibDbFreeCacV03Ctx(cac_calib);
#endif

#if RKAIQ_HAVE_CAC_V10
    CalibDbV2_Cac_V10_t* cac_calib =
        (CalibDbV2_Cac_V10_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, cac_v10));
    if (cac_calib) CamCalibDbFreeCacV10Ctx(cac_calib);
#endif

#if RKAIQ_HAVE_CAC_V11
    CalibDbV2_Cac_V11_t* cac_calib =
        (CalibDbV2_Cac_V11_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, cac_v11));
    if (cac_calib) CamCalibDbFreeCacV11Ctx(cac_calib);
#endif

#if RKAIQ_HAVE_BAYERNR_V2
    CalibDbV2_BayerNrV2_t* bayernr_v2 =
        (CalibDbV2_BayerNrV2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, bayernr_v2));
    if (bayernr_v2) CamCalibDbFreeBayerNrV2Ctx(bayernr_v2);
#endif

#if RKAIQ_HAVE_UVNR_V1
    CalibDbV2_CNR_t* cnr_v1 = (CalibDbV2_CNR_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, cnr_v1));
    if (cnr_v1) CamCalibDbFreeCnrCtx(cnr_v1);
#endif

#if RKAIQ_HAVE_YNR_V2
    CalibDbV2_YnrV2_t* ynr_v2 = (CalibDbV2_YnrV2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, ynr_v2));
    if (ynr_v2) CamCalibDbFreeYnrV2Ctx(ynr_v2);
#endif

#if RKAIQ_HAVE_SHARP_V3
    CalibDbV2_SharpV3_t* sharp_v3 =
        (CalibDbV2_SharpV3_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, sharp_v3));
    if (sharp_v3) CamCalibDbFreeSharpV3Ctx(sharp_v3);
#endif
#if RKAIQ_HAVE_GAIN_V2
    CalibDbV2_GainV2_t* gain_v2 = (CalibDbV2_GainV2_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, gain_v2));
    if (gain_v2) CamCalibDbFreeGainV2Ctx(gain_v2);
#endif

#if RKAIQ_HAVE_BAYERTNR_V30
    CalibDbV2_BayerTnrV30_t* bayertnr_v30 =
        (CalibDbV2_BayerTnrV30_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, bayertnr_v30));
    if (bayertnr_v30) CamCalibDbFreeBayertnrV30Ctx(bayertnr_v30);
#endif

#if RKAIQ_HAVE_YUVME_V1
    CalibDbV2_YuvmeV1_t* yuvme_v1 = (CalibDbV2_YuvmeV1_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, yuvme_v1));
    if (yuvme_v1) CamCalibDbFreeYuvmeV1Ctx(yuvme_v1);
#endif

#if (RKAIQ_HAVE_CNR_V31)
#if (USE_NEWSTRUCT == 0)
    CalibDbV2_CNRV31_t* cnr_v31 = (CalibDbV2_CNRV31_t*)(CALIBDBV2_GET_MODULE_PTR((void*)ctx, cnr_v31));
    if (cnr_v31) CamCalibDbFreeCnrV31Ctx(cnr_v31);
#endif
#endif

    return 0;
}

void* RkAiqCalibDbV2::loadWholeFile(const char *fpath, size_t *fsize)
{
    struct stat st;
    void* buf;
    int fd;

    if (!fpath || (0 != ::stat(fpath, &st))) {
        LOGE("load bin file error!\n");
        return NULL;
    }

    fd = open(fpath, O_RDONLY);
    if (fd < 0) {
        LOGE("failed to open: '%s'\n", fpath);
        return NULL;
    }

    buf = malloc(st.st_size);
    if (!buf) {
        LOGE("read file oom!\n");
        close(fd);
        return NULL;
    }

    if (read(fd, buf, st.st_size) != st.st_size) {
        LOGE("failed to read: '%s'\n", fpath);
        free(buf);
        close(fd);
        return NULL;
    }

    *fsize = st.st_size;

    close(fd);

    return buf;
}

int RkAiqCalibDbV2::checkBinVersion(uint8_t *data, size_t len)
{
    size_t tmp_len = len -  sizeof(char) * 64 - sizeof(size_t) * 2;
    char *iq_version = (char*)(data + tmp_len);
    if (strstr(iq_version, RK_AIQ_IQ_HEAD_VERSION_PREFIX)) {
        if (strcmp(iq_version, RK_AIQ_IQ_HEAD_VERSION) == 0) {
            LOGK("AIQ run with iq bin, iq bin version: %s", RK_AIQ_IQ_HEAD_VERSION);
        } else {
            LOGE("iq bin version no matched, iq bin version: %s current: %s", iq_version, RK_AIQ_IQ_HEAD_VERSION);
            return -1;
        }
    } else {
        LOGE("no iq bin version info, maybe use older version bin, current: %s", RK_AIQ_IQ_HEAD_VERSION);
        return -1;
    }

    return 0;
}

int RkAiqCalibDbV2::parseBinStructMap(uint8_t *data, size_t len)
{
    size_t map_len = *(size_t *)(data + (len - sizeof(size_t)));
    size_t map_offset = *(size_t *)(data + (len - sizeof(size_t) * 2));
    size_t map_index = 0;
    map_index_t *map_addr = NULL;

    map_addr = (map_index_t *)(data + map_offset);
    for (map_index = 0; map_index < map_len; map_index++) {
        map_index_t tmap = (map_addr[map_index]);
        void** dst_obj_addr = (void**)(data + (size_t)tmap.dst_offset);
        *dst_obj_addr = data + (uintptr_t)tmap.ptr_offset;
    }

    return 0;
}

int RkAiqCalibDbV2::restoreBinStructMap(uint8_t *data, size_t len, uint8_t *restore_ptr)
{
    size_t map_len = *(size_t *)(data + (len - sizeof(size_t)));
    size_t map_offset = *(size_t *)(data + (len - sizeof(size_t) * 2));
    size_t map_index = 0;
    map_index_t *map_addr = NULL;

    map_addr = (map_index_t *)(data + map_offset);
    for (map_index = 0; map_index < map_len; map_index++) {
        map_index_t tmap = (map_addr[map_index]);
        void** dst_obj_addr = (void**)(data + (size_t)tmap.dst_offset);
        *dst_obj_addr = restore_ptr + (uintptr_t)tmap.ptr_offset;
    }

    return 0;
}

} // namespace RkCam
