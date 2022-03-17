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

#include "af_xml2json.h"

void CalibV2AfFree(CamCalibDbV2Context_t *calibV2) {

    CalibDbV2_AF_t* af_v2 = (CalibDbV2_AF_t*)(CALIBDBV2_GET_MODULE_PTR(calibV2, af));
    if (af_v2 == NULL)
        return;

    CalibDbV2_AF_Tuning_Para_t* af_json = &af_v2->TuningPara;
    if (af_json->zoomfocus_tbl.focal_length)
        free(af_json->zoomfocus_tbl.focal_length);
    if (af_json->zoomfocus_tbl.zoom_pos)
        free(af_json->zoomfocus_tbl.zoom_pos);
    if (af_json->zoomfocus_tbl.focus_infpos)
        free(af_json->zoomfocus_tbl.focus_infpos);
    if (af_json->zoomfocus_tbl.focus_macropos)
        free(af_json->zoomfocus_tbl.focus_macropos);

}

void convertAfCalibV1ToCalibV2(const CamCalibDbContext_t *calib, CamCalibDbV2Context_t *calibV2) {

    //1. copy xml params from pointer
    const CalibDb_AF_t *af_xml = (CalibDb_AF_t*)CALIBDB_GET_MODULE_PTR((void*)calib, af);
    if (af_xml == NULL)
        return;

    //2. convert xml params 2 json params
    CalibDbV2_AF_t* af_v2 = (CalibDbV2_AF_t*)(CALIBDBV2_GET_MODULE_PTR(calibV2, af));
    if (af_v2 == NULL)
        return;

    CalibDbV2_AF_Tuning_Para_t* af_json = &af_v2->TuningPara;
    memset(af_json, 0x00, sizeof(CalibDbV2_AF_Tuning_Para_t));

    af_json->af_mode = (CalibDbV2_AF_MODE_t)af_xml->af_mode;
    af_json->win_h_offs = af_xml->win_h_offs;
    af_json->win_v_offs = af_xml->win_v_offs;
    af_json->win_h_size = af_xml->win_h_size;
    af_json->win_v_size = af_xml->win_v_size;
    af_json->fixed_mode.code = af_xml->fixed_mode.code;
    af_json->macro_mode.code = af_xml->macro_mode.code;
    af_json->infinity_mode.code = af_xml->infinity_mode.code;

    af_json->contrast_af.enable = af_xml->contrast_af.enable;
    af_json->contrast_af.Afss = (CalibDbV2_Af_SS_t)af_xml->contrast_af.Afss;
    af_json->contrast_af.FullDir = (CalibDbV2_Af_SearchDir_t)af_xml->contrast_af.FullDir;
    af_json->contrast_af.FullSteps = af_xml->contrast_af.FullSteps;
    memcpy(af_json->contrast_af.FullRangeTbl, af_xml->contrast_af.FullRangeTbl, sizeof(af_json->contrast_af.FullRangeTbl));
    af_json->contrast_af.AdaptiveDir = (CalibDbV2_Af_SearchDir_t)af_xml->contrast_af.AdaptiveDir;
    af_json->contrast_af.AdaptiveSteps = af_xml->contrast_af.AdaptiveSteps;
    memcpy(af_json->contrast_af.AdaptRangeTbl, af_xml->contrast_af.AdaptRangeTbl, sizeof(af_json->contrast_af.AdaptRangeTbl));
    af_json->contrast_af.TrigThers[0] = af_xml->contrast_af.TrigThers;
    af_json->contrast_af.TrigThersFv[0] = 0;
    af_json->contrast_af.TrigThersNums = 1;
    af_json->contrast_af.LumaTrigThers = af_xml->contrast_af.LumaTrigThers;
    af_json->contrast_af.ExpTrigThers = 2.0f;

    af_json->contrast_af.StableThers = af_xml->contrast_af.StableThers;
    af_json->contrast_af.StableFrames = af_xml->contrast_af.StableFrames;
    af_json->contrast_af.StableTime = af_xml->contrast_af.StableTime;

    af_json->contrast_af.SceneDiffEnable = af_xml->contrast_af.SceneDiffEnable;
    af_json->contrast_af.SceneDiffThers = af_xml->contrast_af.SceneDiffThers;
    af_json->contrast_af.SceneDiffBlkThers = af_xml->contrast_af.SceneDiffBlkThers;
    af_json->contrast_af.CenterSceneDiffThers = af_xml->contrast_af.CenterSceneDiffThers;

    af_json->contrast_af.ValidMaxMinRatio = af_xml->contrast_af.ValidMaxMinRatio;
    af_json->contrast_af.ValidValueThers = af_xml->contrast_af.ValidValueThers;

    af_json->contrast_af.OutFocusValue = af_xml->contrast_af.OutFocusValue;
    af_json->contrast_af.OutFocusPos = af_xml->contrast_af.OutFocusPos;

    af_json->contrast_af.WeightEnable = af_xml->contrast_af.WeightEnable;
    memcpy(af_json->contrast_af.Weight, af_xml->contrast_af.Weight, sizeof(af_json->contrast_af.Weight));

    af_json->contrast_af.SearchPauseLumaEnable = af_xml->contrast_af.SearchPauseLumaEnable;
    af_json->contrast_af.SearchPauseLumaThers = af_xml->contrast_af.SearchPauseLumaThers;
    af_json->contrast_af.SearchLumaStableFrames = af_xml->contrast_af.SearchLumaStableFrames;
    af_json->contrast_af.SearchLumaStableThers = af_xml->contrast_af.SearchLumaStableThers;

    af_json->contrast_af.FlatValue = af_xml->contrast_af.FlatValue;

    af_json->contrast_af.PointLightLumaTh = 3000;
    af_json->contrast_af.PointLightCntTh = 300;

    af_json->laser_af.enable = af_xml->laser_af.enable;
    memcpy(af_json->laser_af.vcmDot, af_xml->laser_af.vcmDot, sizeof(af_json->laser_af.vcmDot));
    memcpy(af_json->laser_af.distanceDot, af_xml->laser_af.distanceDot, sizeof(af_json->laser_af.distanceDot));

    af_json->pdaf.enable = af_xml->pdaf.enable;

    af_json->vcmcfg.start_current = af_xml->vcmcfg.start_current;
    af_json->vcmcfg.rated_current = af_xml->vcmcfg.rated_current;
    af_json->vcmcfg.step_mode = af_xml->vcmcfg.step_mode;
    af_json->vcmcfg.extra_delay = af_xml->vcmcfg.extra_delay;

    for (int i = 0; i < CALIBDB_MAX_ISO_LEVEL; i++) {
        af_json->measiso_cfg[i].iso = af_xml->measiso_cfg[i].iso;
        af_json->measiso_cfg[i].afmThres = af_xml->measiso_cfg[i].afmThres;

        memcpy(af_json->measiso_cfg[i].gammaY, af_xml->measiso_cfg[i].gammaY, sizeof(af_json->measiso_cfg[i].gammaY));
        memcpy(af_json->measiso_cfg[i].gaussWeight, af_xml->measiso_cfg[i].gaussWeight, sizeof(af_json->measiso_cfg[i].gaussWeight));
    }

    af_json->zoomfocus_tbl.tbl_len = af_xml->zoomfocus_tbl.tbl_len;
    af_json->zoomfocus_tbl.zoom_pos_len = af_xml->zoomfocus_tbl.tbl_len;
    af_json->zoomfocus_tbl.focal_length_len = af_xml->zoomfocus_tbl.tbl_len;
    af_json->zoomfocus_tbl.focus_infpos_len = af_xml->zoomfocus_tbl.tbl_len;
    af_json->zoomfocus_tbl.focus_macropos_len = af_xml->zoomfocus_tbl.tbl_len;
    if (af_json->zoomfocus_tbl.tbl_len > 0) {
        af_json->zoomfocus_tbl.focal_length = (float*)malloc(af_json->zoomfocus_tbl.tbl_len * sizeof(float));
        af_json->zoomfocus_tbl.zoom_pos = (int*)malloc(af_json->zoomfocus_tbl.tbl_len * sizeof(int));
        af_json->zoomfocus_tbl.focus_infpos = (int*)malloc(af_json->zoomfocus_tbl.tbl_len * sizeof(int));
        af_json->zoomfocus_tbl.focus_macropos = (int*)malloc(af_json->zoomfocus_tbl.tbl_len * sizeof(int));
    } else {
        af_json->zoomfocus_tbl.focal_length = NULL;
        af_json->zoomfocus_tbl.zoom_pos = NULL;
        af_json->zoomfocus_tbl.focus_infpos = NULL;
        af_json->zoomfocus_tbl.focus_macropos = NULL;
    }

    for (int i = 0; i < af_json->zoomfocus_tbl.tbl_len; i++) {
        af_json->zoomfocus_tbl.focal_length[i] = af_xml->zoomfocus_tbl.focal_length[i];
        af_json->zoomfocus_tbl.zoom_pos[i] = af_xml->zoomfocus_tbl.zoom_pos[i];
        af_json->zoomfocus_tbl.focus_infpos[i] = af_xml->zoomfocus_tbl.focus_infpos[i];
        af_json->zoomfocus_tbl.focus_macropos[i] = af_xml->zoomfocus_tbl.focus_macropos[i];
    }
}
