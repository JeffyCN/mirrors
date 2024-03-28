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


#include "lut3d_xml2json.h"

void CalibV2Lut3DFree(CamCalibDbV2Context_t *calibV2)
{
    CalibDbV2_Lut3D_Para_V2_t* lut3d_v2 =  (CalibDbV2_Lut3D_Para_V2_t*)(CALIBDBV2_GET_MODULE_PTR(calibV2, lut3d_calib));
    if(lut3d_v2 ==NULL){
          return;
    }
    for (int i = 0; i < lut3d_v2->ALut3D.lutAll_len; i++)
        free(lut3d_v2->ALut3D.lutAll[i].name);
    free(lut3d_v2->ALut3D.lutAll);
}

void convertLut3DCalib2CalibV2(const CamCalibDbContext_t *calib,CamCalibDbV2Context_t *calibV2)
{
    const CalibDb_Lut3d_t *lut3d =
        (CalibDb_Lut3d_t*)(CALIBDB_GET_MODULE_PTR((void*)calib, lut3d));
    if (lut3d == NULL)
        return;

    CalibDbV2_Lut3D_Para_V2_t *lut3d_v2 =
        (CalibDbV2_Lut3D_Para_V2_t*)(CALIBDBV2_GET_MODULE_PTR(calibV2, lut3d_calib));
    if (lut3d_v2 == NULL)
        return;
    // malloc
    memset(lut3d_v2, 0, sizeof(CalibDbV2_Lut3D_Para_V2_t));

    lut3d_v2->ALut3D.lutAll_len = 1;
    lut3d_v2->ALut3D.lutAll = (CalibDbV2_Lut3D_LutPara_t*)malloc(sizeof(CalibDbV2_Lut3D_LutPara_t));
    lut3d_v2->ALut3D.lutAll[0].name = (char*)malloc(sizeof(char)*20);

    //coppy value
    lut3d_v2->common.enable = lut3d->enable;
    lut3d_v2->common.gain_tolerance = 0.1;

    lut3d_v2->ALut3D.damp_en = 1;
    sprintf(lut3d_v2->ALut3D.lutAll[0].name, "%s", "Normal");
    lut3d_v2->ALut3D.lutAll[0].awbGain[0] = 1.0;
    lut3d_v2->ALut3D.lutAll[0].awbGain[1] = 1.0;

    for (int i = 0; i < 9; i++) {
        lut3d_v2->ALut3D.lutAll[0].gain_alpha.gain[i] = 1<<i;
        lut3d_v2->ALut3D.lutAll[0].gain_alpha.alpha[i] =1;
    }

    CalibDbV2_Lut3D_Table_Para_t *Table = &lut3d_v2->ALut3D.lutAll[0].Table;
    memcpy(Table->look_up_table_b, lut3d->look_up_table_b, sizeof(Table->look_up_table_b));
    memcpy(Table->look_up_table_r, lut3d->look_up_table_r, sizeof(Table->look_up_table_r));
    memcpy(Table->look_up_table_g, lut3d->look_up_table_g, sizeof(Table->look_up_table_g));
}


