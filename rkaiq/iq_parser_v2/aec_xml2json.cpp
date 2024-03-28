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
#include "RkAiqCalibApi.h"
#include "aec_xml2json.h"
#include "RkAiqCalibDbV2.h"
#include <string>

/******************************************************************************
* AecGridWeight5x5to15x15()
*****************************************************************************/
void AecGridWeight5x5to15x15
(
    const unsigned char* inWeights,
    unsigned char*  outWeights
)
{

    //initial check
    if (NULL == inWeights) {
        printf("Error: inWeight == NULL!");
    }

    uint8_t line_5x5, col_5x5;
    uint8_t line_15x15, col_15x15;
    int i, j, k;

    for (i = 0; i < 25; i++) {
        line_5x5 = i / 5;
        col_5x5 = i % 5;

        line_15x15 = line_5x5 * 3;
        col_15x15 = col_5x5 * 3;

        //line_5x5 operation
        switch (line_5x5) {
        case 0:
        case 1:
            switch (col_5x5) {
            case 0:
            case 1:
                for (j = 0; j < 3 + line_5x5; j++)
                    outWeights[(line_15x15 + j) * 15 + col_15x15] = inWeights[i] + (float)(inWeights[i + 5] - inWeights[i]) / (3.0f + line_5x5) * j;
                for (j = 0; j < 3 + col_5x5; j++)
                    outWeights[line_15x15 * 15 + col_15x15 + j] = inWeights[i] + (float)(inWeights[i + 1] - inWeights[i]) / (3.0f + col_5x5) * j;
                break;

            case 2:
                for (j = 0; j < 3 + line_5x5; j++)
                    outWeights[(line_15x15 + j) * 15 + col_15x15 + 1] = inWeights[i] + (float)(inWeights[i + 5] - inWeights[i]) / (3.0f + line_5x5) * j;
                break;

            case 3:
            case 4:
                for (j = 0; j < 3 + line_5x5; j++)
                    outWeights[(line_15x15 + j) * 15 + col_15x15 + 2] = inWeights[line_5x5 * 5 + col_5x5] + (float)(inWeights[(line_5x5 + 1) * 5 + col_5x5] - inWeights[line_5x5 * 5 + col_5x5]) / (3.0f + line_5x5) * j;
                for (j = 0; j < 7 - col_5x5; j++)
                    outWeights[line_15x15 * 15 + col_15x15 + 2 - j] = inWeights[i] + (float)(inWeights[i - 1] - inWeights[i]) / (7 - col_5x5) * j;
                break;

            default:
                break;
            }
            break;

        case 2:
            switch (col_5x5) {
            case 0:
            case 1:
                for (j = 0; j < 3 + col_5x5; j++)
                    outWeights[(line_15x15 + 1) * 15 + col_15x15 + j] = inWeights[i] + (float)(inWeights[i + 1] - inWeights[i]) / (3.0f + col_5x5) * j;
                break;

            case 2:
                outWeights[(line_15x15 + 1) * 15 + col_15x15 + 1] = inWeights[i];
                break;

            case 3:
            case 4:
                for (j = 0; j < 7 - col_5x5; j++) {
                    outWeights[(line_15x15 + 1) * 15 + col_15x15 + 2 - j] = inWeights[i] + (float)(inWeights[i - 1] - inWeights[i]) / (7.0f - col_5x5) * j;
                }
                break;

            default:
                break;
            }
            break;

        case 3:
        case 4:
            switch (col_5x5) {
            case 0:
            case 1:
                for (j = 0; j < 7 - line_5x5; j++)
                    outWeights[(line_15x15 + 2 - j) * 15 + col_15x15] = inWeights[i] + (float)(inWeights[i - 5] - inWeights[i]) / (7.0f - line_5x5) * j;
                for (j = 0; j < 3 + col_5x5; j++)
                    outWeights[(line_15x15 + 2) * 15 + col_15x15 + j] = inWeights[i] + (float)(inWeights[i + 1] - inWeights[i]) / (3.0f + col_5x5) * j;
                break;

            case 2:
                for (j = 0; j < 7 - line_5x5; j++)
                    outWeights[(line_15x15 + 2 - j) * 15 + col_15x15 + 1] = inWeights[i] + (float)(inWeights[i - 5] - inWeights[i]) / (7.0f - line_5x5) * j;
                break;

            case 3:
            case 4:
                for (j = 0; j < 7 - line_5x5; j++)
                    outWeights[(line_15x15 + 2 - j) * 15 + col_15x15 + 2] = inWeights[i] + (float)(inWeights[i - 5] - inWeights[i]) / (7.0f - line_5x5) * j;
                for (j = 0; j < 7 - col_5x5; j++)
                    outWeights[(line_15x15 + 2) * 15 + col_15x15 + 2 - j] = inWeights[i] + (float)(inWeights[i - 1] - inWeights[i]) / (7.0f - col_5x5) * j;
                break;

            default:
                break;
            }
            break;
        default:
            break;
        }

    }

    //diagonal operation
    for (k = 0; k < 225; k++) {
        line_15x15 = k / 15;
        col_15x15 = k % 15;
        if ((0 < line_15x15 && line_15x15 < 3) || (3 < line_15x15 && line_15x15 < 7) ||
                (7 < line_15x15 && line_15x15 < 11) || (11 < line_15x15 && line_15x15 < 14)) {
            if (0 < col_15x15 &&  col_15x15 < 3)
                outWeights[k] = outWeights[line_15x15 * 15 + 0] + (float)(outWeights[line_15x15 * 15 + 3] - outWeights[line_15x15 * 15 + 0]) / 3.0f * (float)(col_15x15 - 0);
            else if (3 < col_15x15 &&  col_15x15 < 7)
                outWeights[k] = outWeights[line_15x15 * 15 + 3] + (float)(outWeights[line_15x15 * 15 + 7] - outWeights[line_15x15 * 15 + 3]) / 4.0f * (float)(col_15x15 - 3);
            else if (7 < col_15x15 &&  col_15x15 < 11)
                outWeights[k] = outWeights[line_15x15 * 15 + 7] + (float)(outWeights[line_15x15 * 15 + 11] - outWeights[line_15x15 * 15 + 7]) / 4.0f * (float)(col_15x15 - 7);
            else if (11 < col_15x15 &&  col_15x15 < 14)
                outWeights[k] = outWeights[line_15x15 * 15 + 11] + (float)(outWeights[line_15x15 * 15 + 14] - outWeights[line_15x15 * 15 + 11]) / 3.0f * (float)(col_15x15 - 11);
        }
    }
    /*for (int i = 0; i<15; i++){

            printf("%2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d %2d \n",
              outWeights[i * 15 + 0], outWeights[i * 15 + 1], outWeights[i * 15 + 2], outWeights[i * 15 + 3], outWeights[i * 15 + 4],
              outWeights[i * 15 + 5], outWeights[i * 15 + 6], outWeights[i * 15 + 7], outWeights[i * 15 + 8], outWeights[i * 15 + 9],
              outWeights[i * 15 + 10], outWeights[i * 15 + 11], outWeights[i * 15 + 12], outWeights[i * 15 + 13], outWeights[i * 15 + 14]);
    }*/


}

void CalibV2AecFree(CamCalibDbV2Context_t *calibV2) {
    CalibDb_Aec_ParaV2_t* aec_json =
        (CalibDb_Aec_ParaV2_t*)(CALIBDBV2_GET_MODULE_PTR(calibV2, ae_calib));
    if(aec_json == NULL)
        return;

    //LinCtrl
    free(aec_json->LinearAeCtrl.Route.TimeDot);
    free(aec_json->LinearAeCtrl.Route.GainDot);
    free(aec_json->LinearAeCtrl.Route.IspDGainDot);
    free(aec_json->LinearAeCtrl.Route.PIrisDot);
    free(aec_json->LinearAeCtrl.DySetpoint.ExpLevel);
    free(aec_json->LinearAeCtrl.DySetpoint.DySetpoint);
    free(aec_json->LinearAeCtrl.BackLightCtrl.BacklitSetPoint.ExpLevel);
    free(aec_json->LinearAeCtrl.BackLightCtrl.BacklitSetPoint.TargetLLLuma);
    free(aec_json->LinearAeCtrl.BackLightCtrl.BacklitSetPoint.NonOEPdfTh);
    free(aec_json->LinearAeCtrl.BackLightCtrl.BacklitSetPoint.LowLightPdfTh);
    free(aec_json->LinearAeCtrl.OverExpCtrl.OverExpSetPoint.OEpdf);
    free(aec_json->LinearAeCtrl.OverExpCtrl.OverExpSetPoint.HighLightWeight);
    free(aec_json->LinearAeCtrl.OverExpCtrl.OverExpSetPoint.LowLightWeight);

    //HdrCtrl
    free(aec_json->HdrAeCtrl.Route.Frm0TimeDot);
    free(aec_json->HdrAeCtrl.Route.Frm0GainDot);
    free(aec_json->HdrAeCtrl.Route.Frm0IspDGainDot);
    free(aec_json->HdrAeCtrl.Route.Frm1TimeDot);
    free(aec_json->HdrAeCtrl.Route.Frm1GainDot);
    free(aec_json->HdrAeCtrl.Route.Frm1IspDGainDot);
    free(aec_json->HdrAeCtrl.Route.Frm2TimeDot);
    free(aec_json->HdrAeCtrl.Route.Frm2GainDot);
    free(aec_json->HdrAeCtrl.Route.Frm2IspDGainDot);
    free(aec_json->HdrAeCtrl.Route.PIrisDot);
    free(aec_json->HdrAeCtrl.ExpRatioCtrl.ExpRatio.RatioExpDot);
    free(aec_json->HdrAeCtrl.ExpRatioCtrl.ExpRatio.M2SRatioFix);
    free(aec_json->HdrAeCtrl.ExpRatioCtrl.ExpRatio.L2MRatioFix);
    free(aec_json->HdrAeCtrl.ExpRatioCtrl.ExpRatio.M2SRatioMax);
    free(aec_json->HdrAeCtrl.ExpRatioCtrl.ExpRatio.L2MRatioMax);

    free(aec_json->HdrAeCtrl.LframeCtrl.LfrmSetPoint.LExpLevel);
    free(aec_json->HdrAeCtrl.LframeCtrl.LfrmSetPoint.LSetPoint);
    free(aec_json->HdrAeCtrl.LframeCtrl.LfrmSetPoint.LowLightPdfTh);
    free(aec_json->HdrAeCtrl.LframeCtrl.LfrmSetPoint.NonOEPdfTh);
    free(aec_json->HdrAeCtrl.LframeCtrl.LfrmSetPoint.TargetLLLuma);

    free(aec_json->HdrAeCtrl.MframeCtrl.MExpLevel);
    free(aec_json->HdrAeCtrl.MframeCtrl.MSetPoint);

    free(aec_json->HdrAeCtrl.SframeCtrl.SfrmSetPoint.SExpLevel);
    free(aec_json->HdrAeCtrl.SframeCtrl.SfrmSetPoint.SSetPoint);
    free(aec_json->HdrAeCtrl.SframeCtrl.SfrmSetPoint.TargetHLLuma);

    //SyncTest
    free(aec_json->SyncTest.AlterExp.LinearAE);
    free(aec_json->SyncTest.AlterExp.HdrAE);

    //free(aec_json);

}

void convertModuleinfoCalibV1ToCalibV2(const CamCalibDbContext_t *calib, CalibDb_Module_ParaV2_t* module_info) {

    //1.1) module
    if(CHECK_ISP_HW_V20()) {

        CalibDb_Module_Info_t module_xml =
            *(CalibDb_Module_Info_t*)CALIBDB_GET_MODULE_PTR((void*)calib, module);

        module_info->sensor_module.EFL = module_xml.EFL;
        module_info->sensor_module.LensT = module_xml.LensT;
        module_info->sensor_module.FNumber = module_xml.FNumber;
        module_info->sensor_module.IRCutT = module_xml.IRCutT;
    }
    if(CHECK_ISP_HW_V21()) {

        CalibDb_Module_Info_t moduleinfo_xml =
            *(CalibDb_Module_Info_t*)CALIBDB_GET_MODULE_PTR((void*)calib, module);

        module_info->sensor_module.EFL = moduleinfo_xml.EFL;
        module_info->sensor_module.LensT = moduleinfo_xml.LensT;
        module_info->sensor_module.FNumber = moduleinfo_xml.FNumber;
        module_info->sensor_module.IRCutT = moduleinfo_xml.IRCutT;
    }
}


void convertSensorinfoCalibV1ToCalibV2(const CamCalibDbContext_t *calib, CalibDb_Sensor_ParaV2_t* sensor_info) {
    //1. convert xml params 2 json params

    //1.0) sensorinfo

    if(CHECK_ISP_HW_V20()) {

        CalibDb_Sensor_Para_t sensor_xml =
            *(CalibDb_Sensor_Para_t*)CALIBDB_GET_MODULE_PTR((void*)calib, sensor);

        CalibDb_System_t system_xml =
            *(CalibDb_System_t*)CALIBDB_GET_MODULE_PTR((void*)calib, sysContrl);

        CalibDb_Lsc_t* lsc_xml =
            (CalibDb_Lsc_t*)CALIBDB_GET_MODULE_PTR((void*)calib, lsc);

        std::string resname(lsc_xml->aLscCof.lscResName[0]);
        std::string::size_type pos;
        std::string w, h;

        pos = resname.find("x");
        w = resname.substr(0, pos);
        h = resname.substr(pos + 1, resname.length() - pos);

        sensor_info->resolution.width = atoi(w.c_str());
        sensor_info->resolution.height = atoi(h.c_str());

        if(sensor_xml.GainRange.IsLinear)
            sensor_info->Gain2Reg.GainMode = EXPGAIN_MODE_LINEAR;
        else
            sensor_info->Gain2Reg.GainMode = EXPGAIN_MODE_NONLINEAR_DB;

        sensor_info->Gain2Reg.GainRange_len = sensor_xml.GainRange.array_size;
        sensor_info->Gain2Reg.GainRange = (float*)malloc(sensor_info->Gain2Reg.GainRange_len * sizeof(float));
        memcpy(sensor_info->Gain2Reg.GainRange, sensor_xml.GainRange.pGainRange, sensor_info->Gain2Reg.GainRange_len * sizeof(float));

        sensor_info->Time2Reg.fCoeff[0] = sensor_xml.TimeFactor[0];
        sensor_info->Time2Reg.fCoeff[1] = sensor_xml.TimeFactor[1];
        sensor_info->Time2Reg.fCoeff[2] = sensor_xml.TimeFactor[2];
        sensor_info->Time2Reg.fCoeff[3] = sensor_xml.TimeFactor[3];

        sensor_info->CISGainSet.CISAgainRange.Max = sensor_xml.CISAgainRange.Max;
        sensor_info->CISGainSet.CISAgainRange.Min = sensor_xml.CISAgainRange.Min;
        sensor_info->CISGainSet.CISExtraAgainRange.Max = sensor_xml.CISExtraAgainRange.Max;
        sensor_info->CISGainSet.CISExtraAgainRange.Min = sensor_xml.CISExtraAgainRange.Min;
        sensor_info->CISGainSet.CISDgainRange.Max = sensor_xml.CISDgainRange.Max;
        sensor_info->CISGainSet.CISDgainRange.Min = sensor_xml.CISDgainRange.Min;
        sensor_info->CISGainSet.CISIspDgainRange.Max = sensor_xml.CISIspDgainRange.Max;
        sensor_info->CISGainSet.CISIspDgainRange.Min = sensor_xml.CISIspDgainRange.Min;
        sensor_info->CISGainSet.CISHdrGainIndSetEn = (sensor_xml.CISHdrGainIndSetEn == 0) ? false : true;

        sensor_info->CISTimeSet.Linear.CISLinTimeRegMaxFac = sensor_xml.CISLinTimeRegMaxFac;
        sensor_info->CISTimeSet.Linear.CISTimeRegMin = sensor_xml.CISTimeRegMin;
        sensor_info->CISTimeSet.Linear.CISTimeRegOdevity = sensor_xml.CISTimeRegOdevity;
        sensor_info->CISTimeSet.Hdr[0].name = HDR_TWO_FRAME;
        sensor_info->CISTimeSet.Hdr[0].CISHdrTimeRegSumFac = sensor_xml.CISHdrTimeRegSumFac;
        sensor_info->CISTimeSet.Hdr[0].CISTimeRegMin = sensor_xml.CISHdrTimeRegMin;
        sensor_info->CISTimeSet.Hdr[0].CISTimeRegMax = sensor_xml.CISHdrTimeRegMax;
        sensor_info->CISTimeSet.Hdr[0].CISTimeRegOdevity = sensor_xml.CISHdrTimeRegOdevity;
        sensor_info->CISTimeSet.Hdr[0].CISTimeRegUnEqualEn = (sensor_xml.CISTimeRegUnEqualEn == 0) ? false : true;
        sensor_info->CISTimeSet.Hdr[1].name = HDR_THREE_FRAME;
        sensor_info->CISTimeSet.Hdr[1].CISHdrTimeRegSumFac = sensor_xml.CISHdrTimeRegSumFac;
        sensor_info->CISTimeSet.Hdr[1].CISTimeRegMin = sensor_xml.CISHdrTimeRegMin;
        sensor_info->CISTimeSet.Hdr[1].CISTimeRegMax = sensor_xml.CISHdrTimeRegMax;
        sensor_info->CISTimeSet.Hdr[1].CISTimeRegOdevity = sensor_xml.CISHdrTimeRegOdevity;
        sensor_info->CISTimeSet.Hdr[1].CISTimeRegUnEqualEn = (sensor_xml.CISTimeRegUnEqualEn == 0) ? false : true;

        sensor_info->CISExpUpdate.Linear.time_update = system_xml.exp_delay.Normal.time_delay;
        sensor_info->CISExpUpdate.Linear.gain_update = system_xml.exp_delay.Normal.gain_delay;
        sensor_info->CISExpUpdate.Linear.dcg_update = system_xml.exp_delay.Normal.dcg_delay;
        sensor_info->CISExpUpdate.Hdr.time_update = system_xml.exp_delay.Hdr.time_delay;
        sensor_info->CISExpUpdate.Hdr.gain_update = system_xml.exp_delay.Hdr.gain_delay;
        sensor_info->CISExpUpdate.Hdr.dcg_update = system_xml.exp_delay.Hdr.dcg_delay;

        sensor_info->CISDcgSet.Linear.support_en = system_xml.dcg.Normal.support_en;
        sensor_info->CISDcgSet.Linear.dcg_mode = system_xml.dcg.Normal.dcg_mode;
        sensor_info->CISDcgSet.Linear.dcg_optype = system_xml.dcg.Normal.dcg_optype;
        sensor_info->CISDcgSet.Linear.dcg_ratio = system_xml.dcg.Normal.dcg_ratio;
        sensor_info->CISDcgSet.Linear.sync_switch = system_xml.dcg.Normal.sync_switch;
        sensor_info->CISDcgSet.Linear.lcg2hcg_gain_th = system_xml.dcg.Normal.lcg2hcg_gain_th;
        sensor_info->CISDcgSet.Linear.hcg2lcg_gain_th = system_xml.dcg.Normal.hcg2lcg_gain_th;

        sensor_info->CISDcgSet.Hdr.support_en = system_xml.dcg.Hdr.support_en;
        sensor_info->CISDcgSet.Hdr.dcg_mode = system_xml.dcg.Hdr.dcg_mode;
        sensor_info->CISDcgSet.Hdr.dcg_optype = system_xml.dcg.Hdr.dcg_optype;
        sensor_info->CISDcgSet.Hdr.dcg_ratio = system_xml.dcg.Hdr.dcg_ratio;
        sensor_info->CISDcgSet.Hdr.sync_switch = system_xml.dcg.Hdr.sync_switch;
        sensor_info->CISDcgSet.Hdr.lcg2hcg_gain_th = system_xml.dcg.Hdr.lcg2hcg_gain_th;
        sensor_info->CISDcgSet.Hdr.hcg2lcg_gain_th = system_xml.dcg.Hdr.hcg2lcg_gain_th;

        sensor_info->CISHdrSet.hdr_en = (system_xml.hdr_en == 0) ? false : true;
        sensor_info->CISHdrSet.hdr_mode = system_xml.hdr_mode;
        sensor_info->CISHdrSet.line_mode = system_xml.line_mode;

        sensor_info->CISFlip = sensor_xml.flip;
        sensor_info->CISMinFps = sensor_xml.CISMinFps;
    }

    if(CHECK_ISP_HW_V21()) {

        CalibDb_ExpSet_para_t sensorinfo_xml =
            *(CalibDb_ExpSet_para_t*)CALIBDB_GET_MODULE_PTR((void*)calib, expset);

        CalibDb_Lsc_t* lsc_xml =
            (CalibDb_Lsc_t*)CALIBDB_GET_MODULE_PTR((void*)calib, lsc);

        std::string resname(lsc_xml->aLscCof.lscResName[0]);
        std::string::size_type pos;
        std::string w, h;

        pos = resname.find("x");
        w = resname.substr(0, pos);
        h = resname.substr(pos + 1, resname.length() - pos);

        sensor_info->resolution.width = atoi(w.c_str());
        sensor_info->resolution.height = atoi(h.c_str());

        if(sensorinfo_xml.Gain2Reg.IsLinear)
            sensor_info->Gain2Reg.GainMode = EXPGAIN_MODE_LINEAR;
        else
            sensor_info->Gain2Reg.GainMode = EXPGAIN_MODE_NONLINEAR_DB;

        sensor_info->Gain2Reg.GainRange_len = sensorinfo_xml.Gain2Reg.array_size;
        sensor_info->Gain2Reg.GainRange = (float*)malloc(sensor_info->Gain2Reg.GainRange_len * sizeof(float));
        memcpy(sensor_info->Gain2Reg.GainRange, sensorinfo_xml.Gain2Reg.pGainRange, sensor_info->Gain2Reg.GainRange_len * sizeof(float));

        sensor_info->Time2Reg.fCoeff[0] = sensorinfo_xml.Time2Reg[0];
        sensor_info->Time2Reg.fCoeff[1] = sensorinfo_xml.Time2Reg[1];
        sensor_info->Time2Reg.fCoeff[2] = sensorinfo_xml.Time2Reg[2];
        sensor_info->Time2Reg.fCoeff[3] = sensorinfo_xml.Time2Reg[3];

        sensor_info->CISGainSet.CISAgainRange.Max = sensorinfo_xml.CISGainSet.CISAgainRange.Max;
        sensor_info->CISGainSet.CISAgainRange.Min = sensorinfo_xml.CISGainSet.CISAgainRange.Min;
        sensor_info->CISGainSet.CISExtraAgainRange.Max = sensorinfo_xml.CISGainSet.CISExtraAgainRange.Max;
        sensor_info->CISGainSet.CISExtraAgainRange.Min = sensorinfo_xml.CISGainSet.CISExtraAgainRange.Min;
        sensor_info->CISGainSet.CISDgainRange.Max = sensorinfo_xml.CISGainSet.CISDgainRange.Max;
        sensor_info->CISGainSet.CISDgainRange.Min = sensorinfo_xml.CISGainSet.CISDgainRange.Min;
        sensor_info->CISGainSet.CISIspDgainRange.Max = sensorinfo_xml.CISGainSet.CISIspDgainRange.Max;
        sensor_info->CISGainSet.CISIspDgainRange.Min = sensorinfo_xml.CISGainSet.CISIspDgainRange.Min;
        sensor_info->CISGainSet.CISHdrGainIndSetEn = (sensorinfo_xml.CISGainSet.CISHdrGainIndSetEn == 0) ? false : true;

        sensor_info->CISTimeSet.Linear.CISLinTimeRegMaxFac = sensorinfo_xml.CISTimeSet.Normal.CISLinTimeRegMaxFac;
        sensor_info->CISTimeSet.Linear.CISTimeRegMin = sensorinfo_xml.CISTimeSet.Normal.CISTimeRegMin;
        sensor_info->CISTimeSet.Linear.CISTimeRegOdevity = sensorinfo_xml.CISTimeSet.Normal.CISTimeRegOdevity;
        sensor_info->CISTimeSet.Hdr[0].name = HDR_TWO_FRAME;
        sensor_info->CISTimeSet.Hdr[0].CISHdrTimeRegSumFac = sensorinfo_xml.CISTimeSet.Hdr[0].CISHdrTimeRegSumFac;
        sensor_info->CISTimeSet.Hdr[0].CISTimeRegMin = sensorinfo_xml.CISTimeSet.Hdr[0].CISTimeRegMin;
        sensor_info->CISTimeSet.Hdr[0].CISTimeRegMax = sensorinfo_xml.CISTimeSet.Hdr[0].CISTimeRegMax;
        sensor_info->CISTimeSet.Hdr[0].CISTimeRegOdevity = sensorinfo_xml.CISTimeSet.Hdr[0].CISTimeRegOdevity;
        sensor_info->CISTimeSet.Hdr[0].CISTimeRegUnEqualEn = (sensorinfo_xml.CISTimeSet.Hdr[0].CISTimeRegUnEqualEn == 0) ? false : true;
        sensor_info->CISTimeSet.Hdr[1].name = HDR_THREE_FRAME;
        sensor_info->CISTimeSet.Hdr[1].CISHdrTimeRegSumFac = sensorinfo_xml.CISTimeSet.Hdr[1].CISHdrTimeRegSumFac;
        sensor_info->CISTimeSet.Hdr[1].CISTimeRegMin = sensorinfo_xml.CISTimeSet.Hdr[1].CISTimeRegMin;
        sensor_info->CISTimeSet.Hdr[1].CISTimeRegMax = sensorinfo_xml.CISTimeSet.Hdr[1].CISTimeRegMax;
        sensor_info->CISTimeSet.Hdr[1].CISTimeRegOdevity = sensorinfo_xml.CISTimeSet.Hdr[1].CISTimeRegOdevity;
        sensor_info->CISTimeSet.Hdr[1].CISTimeRegUnEqualEn = (sensorinfo_xml.CISTimeSet.Hdr[1].CISTimeRegUnEqualEn == 0) ? false : true;

        sensor_info->CISExpUpdate.Linear.time_update = sensorinfo_xml.CISExpUpdate.Normal.time_update;
        sensor_info->CISExpUpdate.Linear.gain_update = sensorinfo_xml.CISExpUpdate.Normal.gain_update;
        sensor_info->CISExpUpdate.Linear.dcg_update = sensorinfo_xml.CISExpUpdate.Normal.dcg_update;
        sensor_info->CISExpUpdate.Hdr.time_update = sensorinfo_xml.CISExpUpdate.Hdr.time_update;
        sensor_info->CISExpUpdate.Hdr.gain_update = sensorinfo_xml.CISExpUpdate.Hdr.gain_update;
        sensor_info->CISExpUpdate.Hdr.dcg_update = sensorinfo_xml.CISExpUpdate.Hdr.dcg_update;

        sensor_info->CISDcgSet.Linear.support_en = sensorinfo_xml.CISDcgSet.Normal.support_en;
        sensor_info->CISDcgSet.Linear.dcg_mode = sensorinfo_xml.CISDcgSet.Normal.dcg_mode;
        sensor_info->CISDcgSet.Linear.dcg_optype = sensorinfo_xml.CISDcgSet.Normal.dcg_optype;
        sensor_info->CISDcgSet.Linear.dcg_ratio = sensorinfo_xml.CISDcgSet.Normal.dcg_ratio;
        sensor_info->CISDcgSet.Linear.sync_switch = sensorinfo_xml.CISDcgSet.Normal.sync_switch;
        sensor_info->CISDcgSet.Linear.lcg2hcg_gain_th = sensorinfo_xml.CISDcgSet.Normal.lcg2hcg_gain_th;
        sensor_info->CISDcgSet.Linear.hcg2lcg_gain_th = sensorinfo_xml.CISDcgSet.Normal.hcg2lcg_gain_th;

        sensor_info->CISDcgSet.Hdr.support_en = sensorinfo_xml.CISDcgSet.Hdr.support_en;
        sensor_info->CISDcgSet.Hdr.dcg_mode = sensorinfo_xml.CISDcgSet.Hdr.dcg_mode;
        sensor_info->CISDcgSet.Hdr.dcg_optype = sensorinfo_xml.CISDcgSet.Hdr.dcg_optype;
        sensor_info->CISDcgSet.Hdr.dcg_ratio = sensorinfo_xml.CISDcgSet.Hdr.dcg_ratio;
        sensor_info->CISDcgSet.Hdr.sync_switch = sensorinfo_xml.CISDcgSet.Hdr.sync_switch;
        sensor_info->CISDcgSet.Hdr.lcg2hcg_gain_th = sensorinfo_xml.CISDcgSet.Hdr.lcg2hcg_gain_th;
        sensor_info->CISDcgSet.Hdr.hcg2lcg_gain_th = sensorinfo_xml.CISDcgSet.Hdr.hcg2lcg_gain_th;

        sensor_info->CISHdrSet.hdr_en = (sensorinfo_xml.CISHdrSet.hdr_en == 0) ? false : true;
        sensor_info->CISHdrSet.hdr_mode = sensorinfo_xml.CISHdrSet.hdr_mode;
        sensor_info->CISHdrSet.line_mode = sensorinfo_xml.CISHdrSet.line_mode;

        sensor_info->CISFlip = sensorinfo_xml.CISFlip;
        sensor_info->CISMinFps = sensorinfo_xml.CISMinFps;
    }
}


void convertAecCalibV1ToCalibV2(const CamCalibDbContext_t *calib, CamCalibDbV2Context_t *calibV2) {

    //1. convert xml params 2 json params

    CalibDb_Aec_ParaV2_t aec_json;
    memset(&aec_json, 0x00, sizeof(CalibDb_Aec_ParaV2_t));

    if(CHECK_ISP_HW_V20()) {

        CalibDb_Aec_Para_t aec_xml =
            *(CalibDb_Aec_Para_t*)CALIBDB_GET_MODULE_PTR((void*)calib, aec);

        //1.2.0 CommCtrl
        aec_json.CommCtrl.Enable = (aec_xml.CommCtrl.enable == 0) ? false : true;
        aec_json.CommCtrl.AecRunInterval = aec_xml.CommCtrl.AecRunInterval;
        aec_json.CommCtrl.AecOpType = aec_xml.CommCtrl.AecOpType;
        aec_json.CommCtrl.HistStatsMode = (CalibDb_CamHistStatsModeV2_t)aec_xml.CommCtrl.HistStatsMode;
        switch(aec_xml.CommCtrl.RawStatsMode) {
        case CAM_RAWSTATS_MODE_R:
            aec_json.CommCtrl.RawStatsMode = CAM_RAWSTATSV2_MODE_R;
            break;
        case CAM_RAWSTATS_MODE_G:
            aec_json.CommCtrl.RawStatsMode = CAM_RAWSTATSV2_MODE_G;
            break;
        case CAM_RAWSTATS_MODE_B:
            aec_json.CommCtrl.RawStatsMode = CAM_RAWSTATSV2_MODE_B;
            break;
        case CAM_RAWSTATS_MODE_Y:
        default:
            aec_json.CommCtrl.RawStatsMode = CAM_RAWSTATSV2_MODE_Y;
            break;
        }

        if(aec_xml.CommCtrl.YRangeMode <= CAM_YRANGE_MODE_FULL)
            aec_json.CommCtrl.YRangeMode = CAM_YRANGEV2_MODE_FULL;
        else
            aec_json.CommCtrl.YRangeMode = CAM_YRANGEV2_MODE_LIMITED;

        if(aec_xml.CommCtrl.DayWeightNum == AECV2_MAX_GRIDWEIGHT_NUM) {
            for(int i = 0; i < AECV2_MAX_GRIDWEIGHT_NUM; i++)
                aec_json.CommCtrl.AecGridWeight[i] = aec_xml.CommCtrl.DayGridWeights.uCoeff[i];
        } else {
            AecGridWeight5x5to15x15(aec_xml.CommCtrl.DayGridWeights.uCoeff, aec_json.CommCtrl.AecGridWeight);
        }

        aec_json.CommCtrl.AecSpeed.SmoothEn = aec_xml.CommCtrl.stAuto.stAeSpeed.SmoothEn;
        aec_json.CommCtrl.AecSpeed.DyDampEn = aec_xml.CommCtrl.stAuto.stAeSpeed.DyDampEn;
        aec_json.CommCtrl.AecSpeed.DampOver = aec_xml.CommCtrl.stAuto.stAeSpeed.DampOver;
        aec_json.CommCtrl.AecSpeed.DampUnder = aec_xml.CommCtrl.stAuto.stAeSpeed.DampUnder;
        aec_json.CommCtrl.AecSpeed.DampBright2Dark = aec_xml.CommCtrl.stAuto.stAeSpeed.DampBright2Dark;
        aec_json.CommCtrl.AecSpeed.DampDark2Bright = aec_xml.CommCtrl.stAuto.stAeSpeed.DampDark2Bright;

        aec_json.CommCtrl.AecDelayFrmNum.BlackDelay = aec_xml.CommCtrl.stAuto.BlackDelayFrame;
        aec_json.CommCtrl.AecDelayFrmNum.WhiteDelay = aec_xml.CommCtrl.stAuto.WhiteDelayFrame;

        aec_json.CommCtrl.AecAntiFlicker.enable = aec_xml.CommCtrl.stAntiFlicker.enable;
        aec_json.CommCtrl.AecAntiFlicker.Frequency = (CalibDb_FlickerFreqV2_t)aec_xml.CommCtrl.stAntiFlicker.Frequency;
        if(aec_xml.CommCtrl.stAntiFlicker.Mode == AEC_ANTIFLICKER_AUTO_MODE)
            aec_json.CommCtrl.AecAntiFlicker.Mode = AECV2_ANTIFLICKER_AUTO_MODE;
        else
            aec_json.CommCtrl.AecAntiFlicker.Mode = AECV2_ANTIFLICKER_NORMAL_MODE;

        aec_json.CommCtrl.AecFrameRateMode.isFpsFix = aec_xml.CommCtrl.stAuto.stFrmRate.isFpsFix;
        aec_json.CommCtrl.AecFrameRateMode.FpsValue = aec_xml.CommCtrl.stAuto.stFrmRate.FpsValue;

        aec_json.CommCtrl.AecEnvLvCalib.CalibFNumber = aec_xml.CommCtrl.stEnvLvCalib.CalibFN;
        aec_json.CommCtrl.AecEnvLvCalib.CurveCoeff[0] = aec_xml.CommCtrl.stEnvLvCalib.Curve.fCoeff[0];
        aec_json.CommCtrl.AecEnvLvCalib.CurveCoeff[1] = aec_xml.CommCtrl.stEnvLvCalib.Curve.fCoeff[1];

        aec_json.CommCtrl.AecWinScale.InRawWinScale.h_offs = 0;
        aec_json.CommCtrl.AecWinScale.InRawWinScale.v_offs = 0;
        aec_json.CommCtrl.AecWinScale.InRawWinScale.h_size = 1;
        aec_json.CommCtrl.AecWinScale.InRawWinScale.v_size = 1;
        aec_json.CommCtrl.AecWinScale.YuvWinScale.h_offs = 0;
        aec_json.CommCtrl.AecWinScale.YuvWinScale.v_offs = 0;
        aec_json.CommCtrl.AecWinScale.YuvWinScale.h_size = 1;
        aec_json.CommCtrl.AecWinScale.YuvWinScale.v_size = 1;
        aec_json.CommCtrl.AecWinScale.TmoRawWinScale.h_offs = 0.1;
        aec_json.CommCtrl.AecWinScale.TmoRawWinScale.v_offs = 0.1;
        aec_json.CommCtrl.AecWinScale.TmoRawWinScale.h_size = 0.8;
        aec_json.CommCtrl.AecWinScale.TmoRawWinScale.v_size = 0.8;

        aec_json.CommCtrl.AecManualCtrl.LinearAE.ManualTimeEn = aec_xml.CommCtrl.stManual.stLinMe.ManualTimeEn;
        aec_json.CommCtrl.AecManualCtrl.LinearAE.ManualGainEn = aec_xml.CommCtrl.stManual.stLinMe.ManualGainEn;
        aec_json.CommCtrl.AecManualCtrl.LinearAE.ManualIspDgainEn = aec_xml.CommCtrl.stManual.stLinMe.ManualIspDgainEn;
        aec_json.CommCtrl.AecManualCtrl.LinearAE.TimeValue = aec_xml.CommCtrl.stManual.stLinMe.TimeValue;
        aec_json.CommCtrl.AecManualCtrl.LinearAE.GainValue = aec_xml.CommCtrl.stManual.stLinMe.GainValue;
        aec_json.CommCtrl.AecManualCtrl.LinearAE.IspDGainValue = aec_xml.CommCtrl.stManual.stLinMe.IspDGainValue;

        aec_json.CommCtrl.AecManualCtrl.HdrAE.ManualTimeEn = aec_xml.CommCtrl.stManual.stHdrMe.ManualTimeEn;
        aec_json.CommCtrl.AecManualCtrl.HdrAE.ManualGainEn = aec_xml.CommCtrl.stManual.stHdrMe.ManualGainEn;
        aec_json.CommCtrl.AecManualCtrl.HdrAE.ManualIspDgainEn = aec_xml.CommCtrl.stManual.stHdrMe.ManualIspDgainEn;
        memcpy(aec_json.CommCtrl.AecManualCtrl.HdrAE.TimeValue, aec_xml.CommCtrl.stManual.stHdrMe.TimeValue.fCoeff, 3 * sizeof(float));
        memcpy(aec_json.CommCtrl.AecManualCtrl.HdrAE.GainValue, aec_xml.CommCtrl.stManual.stHdrMe.GainValue.fCoeff, 3 * sizeof(float));
        memcpy(aec_json.CommCtrl.AecManualCtrl.HdrAE.IspDGainValue, aec_xml.CommCtrl.stManual.stHdrMe.IspDGainValue.fCoeff, 3 * sizeof(float));

        //1.2.1 LinearAeCtrl
        aec_json.LinearAeCtrl.RawStatsEn = (aec_xml.LinearAeCtrl.RawStatsEn == 0) ? false : true;
        aec_json.LinearAeCtrl.Evbias = aec_xml.LinearAeCtrl.Evbias;
        aec_json.LinearAeCtrl.ToleranceIn = aec_xml.LinearAeCtrl.ToleranceIn;
        aec_json.LinearAeCtrl.ToleranceOut = aec_xml.LinearAeCtrl.ToleranceOut;
        if(aec_xml.LinearAeCtrl.StrategyMode <= RKAIQ_AEC_STRATEGY_MODE_LOWLIGHT_PRIOR)
            aec_json.LinearAeCtrl.StrategyMode = AECV2_STRATEGY_MODE_LOWLIGHT;
        else
            aec_json.LinearAeCtrl.StrategyMode = AECV2_STRATEGY_MODE_HIGHLIGHT;

        aec_json.LinearAeCtrl.InitExp.InitTimeValue = aec_xml.CommCtrl.stInitExp.stLinExpInitExp.InitTimeValue;
        aec_json.LinearAeCtrl.InitExp.InitGainValue = aec_xml.CommCtrl.stInitExp.stLinExpInitExp.InitGainValue;
        aec_json.LinearAeCtrl.InitExp.InitIspDGainValue = aec_xml.CommCtrl.stInitExp.stLinExpInitExp.InitIspDGainValue;
        aec_json.LinearAeCtrl.InitExp.InitPIrisGainValue = aec_xml.CommCtrl.stInitExp.stLinExpInitExp.InitPIrisGainValue;
        aec_json.LinearAeCtrl.InitExp.InitDCIrisDutyValue = aec_xml.CommCtrl.stInitExp.stLinExpInitExp.InitDCIrisDutyValue;
        aec_json.LinearAeCtrl.InitExp.InitHDCIrisTargetValue = aec_xml.CommCtrl.stInitExp.stLinExpInitExp.InitHDCIrisTargetValue;

        aec_json.LinearAeCtrl.Route.TimeDot_len = aec_xml.CommCtrl.stAeRoute.LinAeSeperate[AEC_DNMODE_DAY].array_size;
        aec_json.LinearAeCtrl.Route.GainDot_len = aec_xml.CommCtrl.stAeRoute.LinAeSeperate[AEC_DNMODE_DAY].array_size;
        aec_json.LinearAeCtrl.Route.IspDGainDot_len = aec_xml.CommCtrl.stAeRoute.LinAeSeperate[AEC_DNMODE_DAY].array_size;
        aec_json.LinearAeCtrl.Route.PIrisDot_len = aec_xml.CommCtrl.stAeRoute.LinAeSeperate[AEC_DNMODE_DAY].array_size;

        aec_json.LinearAeCtrl.Route.TimeDot = (float*)malloc(aec_json.LinearAeCtrl.Route.TimeDot_len * sizeof(float));
        memcpy(aec_json.LinearAeCtrl.Route.TimeDot, aec_xml.CommCtrl.stAeRoute.LinAeSeperate[AEC_DNMODE_DAY].TimeDot, aec_json.LinearAeCtrl.Route.TimeDot_len * sizeof(float));
        aec_json.LinearAeCtrl.Route.GainDot = (float*)malloc(aec_json.LinearAeCtrl.Route.GainDot_len * sizeof(float));
        memcpy(aec_json.LinearAeCtrl.Route.GainDot, aec_xml.CommCtrl.stAeRoute.LinAeSeperate[AEC_DNMODE_DAY].GainDot, aec_json.LinearAeCtrl.Route.GainDot_len * sizeof(float));
        aec_json.LinearAeCtrl.Route.IspDGainDot = (float*)malloc(aec_json.LinearAeCtrl.Route.IspDGainDot_len * sizeof(float));
        memcpy(aec_json.LinearAeCtrl.Route.IspDGainDot, aec_xml.CommCtrl.stAeRoute.LinAeSeperate[AEC_DNMODE_DAY].IspgainDot, aec_json.LinearAeCtrl.Route.IspDGainDot_len * sizeof(float));
        aec_json.LinearAeCtrl.Route.PIrisDot = (int*)malloc(aec_json.LinearAeCtrl.Route.PIrisDot_len * sizeof(int));
        memcpy(aec_json.LinearAeCtrl.Route.PIrisDot, aec_xml.CommCtrl.stAeRoute.LinAeSeperate[AEC_DNMODE_DAY].PIrisGainDot, aec_json.LinearAeCtrl.Route.PIrisDot_len * sizeof(float));


        aec_json.LinearAeCtrl.DySetpoint.ExpLevel_len = aec_xml.LinearAeCtrl.DySetpoint[AEC_DNMODE_DAY].array_size;
        aec_json.LinearAeCtrl.DySetpoint.DySetpoint_len = aec_xml.LinearAeCtrl.DySetpoint[AEC_DNMODE_DAY].array_size;

        aec_json.LinearAeCtrl.DySetpoint.ExpLevel = (float*)malloc(aec_json.LinearAeCtrl.DySetpoint.ExpLevel_len * sizeof(float));
        aec_json.LinearAeCtrl.DySetpoint.DySetpoint = (float*)malloc(aec_json.LinearAeCtrl.DySetpoint.DySetpoint_len * sizeof(float));
        /*NOTE: use exp value here, not curexp/maxexp*/
        float maxexp = aec_json.LinearAeCtrl.Route.TimeDot[aec_json.LinearAeCtrl.Route.TimeDot_len - 1] *
                       aec_json.LinearAeCtrl.Route.GainDot[aec_json.LinearAeCtrl.Route.GainDot_len - 1];
        for(int i = 0; i < aec_json.LinearAeCtrl.DySetpoint.ExpLevel_len; i++)
            aec_json.LinearAeCtrl.DySetpoint.ExpLevel[i] = maxexp * aec_xml.LinearAeCtrl.DySetpoint[AEC_DNMODE_DAY].ExpValue[i];

        if(aec_xml.LinearAeCtrl.DySetPointEn == false) {
            for(int i = 0; i < aec_json.LinearAeCtrl.DySetpoint.DySetpoint_len; i++)
                aec_json.LinearAeCtrl.DySetpoint.DySetpoint[i] = aec_xml.LinearAeCtrl.SetPoint;
        } else {
            memcpy(aec_json.LinearAeCtrl.DySetpoint.DySetpoint, aec_xml.LinearAeCtrl.DySetpoint[AEC_DNMODE_DAY].DySetpoint, aec_json.LinearAeCtrl.DySetpoint.DySetpoint_len * sizeof(float));
        }

        aec_json.LinearAeCtrl.BackLightCtrl.Enable = (aec_xml.LinearAeCtrl.BackLightConf.enable == 0) ? false : true;
        aec_json.LinearAeCtrl.BackLightCtrl.StrBias = aec_xml.LinearAeCtrl.BackLightConf.StrBias;
        aec_json.LinearAeCtrl.BackLightCtrl.MeasArea = (CalibDb_AecMeasAreaModeV2_t)aec_xml.LinearAeCtrl.BackLightConf.MeasArea;
        aec_json.LinearAeCtrl.BackLightCtrl.OEROILowTh = aec_xml.LinearAeCtrl.BackLightConf.OEROILowTh;
        aec_json.LinearAeCtrl.BackLightCtrl.LumaDistTh = aec_xml.LinearAeCtrl.BackLightConf.LumaDistTh;
        aec_json.LinearAeCtrl.BackLightCtrl.LvHighTh = aec_xml.LinearAeCtrl.BackLightConf.LvHightTh;
        aec_json.LinearAeCtrl.BackLightCtrl.LvLowTh = aec_xml.LinearAeCtrl.BackLightConf.LvLowTh;
        aec_json.LinearAeCtrl.BackLightCtrl.BacklitSetPoint.ExpLevel_len = 6;
        aec_json.LinearAeCtrl.BackLightCtrl.BacklitSetPoint.TargetLLLuma_len = 6;
        aec_json.LinearAeCtrl.BackLightCtrl.BacklitSetPoint.NonOEPdfTh_len = 6;
        aec_json.LinearAeCtrl.BackLightCtrl.BacklitSetPoint.LowLightPdfTh_len = 6;
        aec_json.LinearAeCtrl.BackLightCtrl.BacklitSetPoint.ExpLevel = (float*)malloc(6 * sizeof(float));
        /*NOTE: use exp value here, not curexp/maxexp*/
        for(int i = 0; i < 6; i++)
            aec_json.LinearAeCtrl.BackLightCtrl.BacklitSetPoint.ExpLevel[i] = maxexp * aec_xml.LinearAeCtrl.BackLightConf.ExpLevel.fCoeff[i];
        aec_json.LinearAeCtrl.BackLightCtrl.BacklitSetPoint.TargetLLLuma = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.LinearAeCtrl.BackLightCtrl.BacklitSetPoint.TargetLLLuma, aec_xml.LinearAeCtrl.BackLightConf.TargetLLLuma.fCoeff, 6 * sizeof(float));
        aec_json.LinearAeCtrl.BackLightCtrl.BacklitSetPoint.NonOEPdfTh = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.LinearAeCtrl.BackLightCtrl.BacklitSetPoint.NonOEPdfTh, aec_xml.LinearAeCtrl.BackLightConf.NonOEPdfTh.fCoeff, 6 * sizeof(float));
        aec_json.LinearAeCtrl.BackLightCtrl.BacklitSetPoint.LowLightPdfTh = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.LinearAeCtrl.BackLightCtrl.BacklitSetPoint.LowLightPdfTh, aec_xml.LinearAeCtrl.BackLightConf.LowLightPdfTh.fCoeff, 6 * sizeof(float));

        aec_json.LinearAeCtrl.OverExpCtrl.Enable = (aec_xml.LinearAeCtrl.OverExpCtrl.enable == 0) ? false : true;
        aec_json.LinearAeCtrl.OverExpCtrl.StrBias = aec_xml.LinearAeCtrl.OverExpCtrl.StrBias;
        aec_json.LinearAeCtrl.OverExpCtrl.MaxWeight = aec_xml.LinearAeCtrl.OverExpCtrl.MaxWeight;
        aec_json.LinearAeCtrl.OverExpCtrl.HighLightTh = aec_xml.LinearAeCtrl.OverExpCtrl.HighLightTh;
        aec_json.LinearAeCtrl.OverExpCtrl.LowLightTh = aec_xml.LinearAeCtrl.OverExpCtrl.LowLightTh;
        aec_json.LinearAeCtrl.OverExpCtrl.OverExpSetPoint.HighLightWeight_len = 6;
        aec_json.LinearAeCtrl.OverExpCtrl.OverExpSetPoint.LowLightWeight_len = 6;
        aec_json.LinearAeCtrl.OverExpCtrl.OverExpSetPoint.OEpdf_len = 6;
        aec_json.LinearAeCtrl.OverExpCtrl.OverExpSetPoint.HighLightWeight = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.LinearAeCtrl.OverExpCtrl.OverExpSetPoint.HighLightWeight, aec_xml.LinearAeCtrl.OverExpCtrl.HighLightWeight.fCoeff, 6 * sizeof(float));
        aec_json.LinearAeCtrl.OverExpCtrl.OverExpSetPoint.LowLightWeight = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.LinearAeCtrl.OverExpCtrl.OverExpSetPoint.LowLightWeight, aec_xml.LinearAeCtrl.OverExpCtrl.LowLightWeight.fCoeff, 6 * sizeof(float));
        aec_json.LinearAeCtrl.OverExpCtrl.OverExpSetPoint.OEpdf = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.LinearAeCtrl.OverExpCtrl.OverExpSetPoint.OEpdf, aec_xml.LinearAeCtrl.OverExpCtrl.OEpdf.fCoeff, 6 * sizeof(float));

        //1.2.2 HdrAeCtrl
        aec_json.HdrAeCtrl.ToleranceIn = aec_xml.HdrAeCtrl.ToleranceIn;
        aec_json.HdrAeCtrl.ToleranceOut = aec_xml.HdrAeCtrl.ToleranceOut;
        aec_json.HdrAeCtrl.Evbias = aec_xml.HdrAeCtrl.Evbias;
        aec_json.HdrAeCtrl.LumaDistTh = aec_xml.HdrAeCtrl.LumaDistTh;
        if(aec_xml.HdrAeCtrl.StrategyMode <= RKAIQ_AEC_STRATEGY_MODE_LOWLIGHT_PRIOR)
            aec_json.HdrAeCtrl.StrategyMode = AECV2_STRATEGY_MODE_LOWLIGHT;
        else
            aec_json.HdrAeCtrl.StrategyMode = AECV2_STRATEGY_MODE_HIGHLIGHT;


        memcpy(aec_json.HdrAeCtrl.InitExp.InitTimeValue, aec_xml.CommCtrl.stInitExp.stHdrExpInitExp.InitTimeValue.fCoeff, 3 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.InitExp.InitGainValue, aec_xml.CommCtrl.stInitExp.stHdrExpInitExp.InitGainValue.fCoeff, 3 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.InitExp.InitIspDGainValue, aec_xml.CommCtrl.stInitExp.stHdrExpInitExp.InitIspDGainValue.fCoeff, 3 * sizeof(float));

        aec_json.HdrAeCtrl.InitExp.InitPIrisGainValue = aec_xml.CommCtrl.stInitExp.stHdrExpInitExp.InitPIrisGainValue;
        aec_json.HdrAeCtrl.InitExp.InitDCIrisDutyValue = aec_xml.CommCtrl.stInitExp.stHdrExpInitExp.InitDCIrisDutyValue;
        aec_json.HdrAeCtrl.InitExp.InitHDCIrisTargetValue = aec_xml.CommCtrl.stInitExp.stHdrExpInitExp.InitHDCIrisTargetValue;

        aec_json.HdrAeCtrl.Route.Frm0TimeDot_len = aec_xml.CommCtrl.stAeRoute.HdrAeSeperate[AEC_DNMODE_DAY].array_size;
        aec_json.HdrAeCtrl.Route.Frm0GainDot_len = aec_xml.CommCtrl.stAeRoute.HdrAeSeperate[AEC_DNMODE_DAY].array_size;
        aec_json.HdrAeCtrl.Route.Frm0IspDGainDot_len = aec_xml.CommCtrl.stAeRoute.HdrAeSeperate[AEC_DNMODE_DAY].array_size;
        aec_json.HdrAeCtrl.Route.Frm1TimeDot_len = aec_xml.CommCtrl.stAeRoute.HdrAeSeperate[AEC_DNMODE_DAY].array_size;
        aec_json.HdrAeCtrl.Route.Frm1GainDot_len = aec_xml.CommCtrl.stAeRoute.HdrAeSeperate[AEC_DNMODE_DAY].array_size;
        aec_json.HdrAeCtrl.Route.Frm1IspDGainDot_len = aec_xml.CommCtrl.stAeRoute.HdrAeSeperate[AEC_DNMODE_DAY].array_size;
        aec_json.HdrAeCtrl.Route.Frm2TimeDot_len = aec_xml.CommCtrl.stAeRoute.HdrAeSeperate[AEC_DNMODE_DAY].array_size;
        aec_json.HdrAeCtrl.Route.Frm2GainDot_len = aec_xml.CommCtrl.stAeRoute.HdrAeSeperate[AEC_DNMODE_DAY].array_size;
        aec_json.HdrAeCtrl.Route.Frm2IspDGainDot_len = aec_xml.CommCtrl.stAeRoute.HdrAeSeperate[AEC_DNMODE_DAY].array_size;
        aec_json.HdrAeCtrl.Route.PIrisDot_len = aec_xml.CommCtrl.stAeRoute.HdrAeSeperate[AEC_DNMODE_DAY].array_size;

        aec_json.HdrAeCtrl.Route.Frm0TimeDot = (float*)malloc(aec_json.HdrAeCtrl.Route.Frm0TimeDot_len * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.Route.Frm0TimeDot, aec_xml.CommCtrl.stAeRoute.HdrAeSeperate[AEC_DNMODE_DAY].HdrTimeDot[0], aec_json.HdrAeCtrl.Route.Frm0TimeDot_len * sizeof(float));
        aec_json.HdrAeCtrl.Route.Frm0GainDot = (float*)malloc(aec_json.HdrAeCtrl.Route.Frm0GainDot_len * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.Route.Frm0GainDot, aec_xml.CommCtrl.stAeRoute.HdrAeSeperate[AEC_DNMODE_DAY].HdrGainDot[0], aec_json.HdrAeCtrl.Route.Frm0GainDot_len * sizeof(float));
        aec_json.HdrAeCtrl.Route.Frm0IspDGainDot = (float*)malloc(aec_json.HdrAeCtrl.Route.Frm0IspDGainDot_len * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.Route.Frm0IspDGainDot, aec_xml.CommCtrl.stAeRoute.HdrAeSeperate[AEC_DNMODE_DAY].HdrIspDGainDot[0], aec_json.HdrAeCtrl.Route.Frm0IspDGainDot_len * sizeof(float));
        aec_json.HdrAeCtrl.Route.Frm1TimeDot = (float*)malloc(aec_json.HdrAeCtrl.Route.Frm1TimeDot_len * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.Route.Frm1TimeDot, aec_xml.CommCtrl.stAeRoute.HdrAeSeperate[AEC_DNMODE_DAY].HdrTimeDot[1], aec_json.HdrAeCtrl.Route.Frm1TimeDot_len * sizeof(float));
        aec_json.HdrAeCtrl.Route.Frm1GainDot = (float*)malloc(aec_json.HdrAeCtrl.Route.Frm1GainDot_len * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.Route.Frm1GainDot, aec_xml.CommCtrl.stAeRoute.HdrAeSeperate[AEC_DNMODE_DAY].HdrGainDot[1], aec_json.HdrAeCtrl.Route.Frm1GainDot_len * sizeof(float));
        aec_json.HdrAeCtrl.Route.Frm1IspDGainDot = (float*)malloc(aec_json.HdrAeCtrl.Route.Frm1IspDGainDot_len * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.Route.Frm1IspDGainDot, aec_xml.CommCtrl.stAeRoute.HdrAeSeperate[AEC_DNMODE_DAY].HdrIspDGainDot[1], aec_json.HdrAeCtrl.Route.Frm1IspDGainDot_len * sizeof(float));
        aec_json.HdrAeCtrl.Route.Frm2TimeDot = (float*)malloc(aec_json.HdrAeCtrl.Route.Frm2TimeDot_len * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.Route.Frm2TimeDot, aec_xml.CommCtrl.stAeRoute.HdrAeSeperate[AEC_DNMODE_DAY].HdrTimeDot[2], aec_json.HdrAeCtrl.Route.Frm2TimeDot_len * sizeof(float));
        aec_json.HdrAeCtrl.Route.Frm2GainDot = (float*)malloc(aec_json.HdrAeCtrl.Route.Frm2GainDot_len * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.Route.Frm2GainDot, aec_xml.CommCtrl.stAeRoute.HdrAeSeperate[AEC_DNMODE_DAY].HdrGainDot[2], aec_json.HdrAeCtrl.Route.Frm2GainDot_len * sizeof(float));
        aec_json.HdrAeCtrl.Route.Frm2IspDGainDot = (float*)malloc(aec_json.HdrAeCtrl.Route.Frm2IspDGainDot_len * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.Route.Frm2IspDGainDot, aec_xml.CommCtrl.stAeRoute.HdrAeSeperate[AEC_DNMODE_DAY].HdrIspDGainDot[2], aec_json.HdrAeCtrl.Route.Frm2IspDGainDot_len * sizeof(float));
        aec_json.HdrAeCtrl.Route.PIrisDot = (int*)malloc(aec_json.HdrAeCtrl.Route.PIrisDot_len * sizeof(int));
        memcpy(aec_json.HdrAeCtrl.Route.PIrisDot, aec_xml.CommCtrl.stAeRoute.HdrAeSeperate[AEC_DNMODE_DAY].PIrisGainDot, aec_json.HdrAeCtrl.Route.PIrisDot_len * sizeof(float));

        aec_json.HdrAeCtrl.LongFrmMode.mode = (CalibDb_AeHdrLongFrmModeV2_t)aec_xml.HdrAeCtrl.LongfrmMode;
        aec_json.HdrAeCtrl.LongFrmMode.SfrmMinLine = aec_xml.HdrAeCtrl.SfrmMinLine;
        aec_json.HdrAeCtrl.LongFrmMode.LfrmModeExpTh = aec_xml.HdrAeCtrl.LfrmModeExpTh;

        if(aec_xml.HdrAeCtrl.ExpRatioType <= RKAIQ_HDRAE_RATIOTYPE_MODE_AUTO)
            aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatioType = AECV2_HDR_RATIOTYPE_MODE_AUTO;
        else
            aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatioType = AECV2_HDR_RATIOTYPE_MODE_FIX;
        aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.RatioExpDot_len = 6;
        aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.M2SRatioFix_len = 6;
        aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.L2MRatioFix_len = 6;
        aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.M2SRatioMax_len = 6;
        aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.L2MRatioMax_len = 6;
        aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.RatioExpDot = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.RatioExpDot, aec_xml.HdrAeCtrl.RatioExpDot.fCoeff, 6 * sizeof(float));
        aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.M2SRatioFix = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.M2SRatioFix, aec_xml.HdrAeCtrl.M2SRatioFix.fCoeff, 6 * sizeof(float));
        aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.L2MRatioFix = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.L2MRatioFix, aec_xml.HdrAeCtrl.L2MRatioFix.fCoeff, 6 * sizeof(float));
        aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.M2SRatioMax = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.M2SRatioMax, aec_xml.HdrAeCtrl.M2SRatioMax.fCoeff, 6 * sizeof(float));
        aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.L2MRatioMax = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.L2MRatioMax, aec_xml.HdrAeCtrl.L2MRatioMax.fCoeff, 6 * sizeof(float));

        aec_json.HdrAeCtrl.LframeCtrl.LvHighTh = aec_xml.HdrAeCtrl.LframeCtrl.LvHighTh;
        aec_json.HdrAeCtrl.LframeCtrl.LvLowTh = aec_xml.HdrAeCtrl.LframeCtrl.LvLowTh;
        aec_json.HdrAeCtrl.LframeCtrl.OEROILowTh = aec_xml.HdrAeCtrl.LframeCtrl.OEROILowTh;
        aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.LExpLevel_len = 6;
        aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.NonOEPdfTh_len = 6;
        aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.LowLightPdfTh_len = 6;
        aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.LSetPoint_len = 6;
        aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.TargetLLLuma_len = 6;
        float lmaxexp = aec_json.HdrAeCtrl.Route.Frm1TimeDot[aec_json.HdrAeCtrl.Route.Frm1TimeDot_len - 1] *
                        aec_json.HdrAeCtrl.Route.Frm1GainDot[aec_json.HdrAeCtrl.Route.Frm1GainDot_len - 1];
        float mmaxexp = aec_json.HdrAeCtrl.Route.Frm1TimeDot[aec_json.HdrAeCtrl.Route.Frm1TimeDot_len - 1] *
                        aec_json.HdrAeCtrl.Route.Frm1GainDot[aec_json.HdrAeCtrl.Route.Frm1GainDot_len - 1];
        float smaxexp = aec_json.HdrAeCtrl.Route.Frm0TimeDot[aec_json.HdrAeCtrl.Route.Frm0TimeDot_len - 1] *
                        aec_json.HdrAeCtrl.Route.Frm0GainDot[aec_json.HdrAeCtrl.Route.Frm0GainDot_len - 1];
        aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.LExpLevel = (float*)malloc(6 * sizeof(float));
        /*NOTE: use exp value here, not curexp/maxexp*/
        for(int i = 0; i < 6; i++) {
            aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.LExpLevel[i] = lmaxexp * aec_xml.HdrAeCtrl.LframeCtrl.LExpLevel.fCoeff[i];
        }
        aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.NonOEPdfTh = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.NonOEPdfTh, aec_xml.HdrAeCtrl.LframeCtrl.NonOEPdfTh.fCoeff, 6 * sizeof(float));
        aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.LowLightPdfTh = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.LowLightPdfTh, aec_xml.HdrAeCtrl.LframeCtrl.LowLightPdfTh.fCoeff, 6 * sizeof(float));
        aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.LSetPoint = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.LSetPoint, aec_xml.HdrAeCtrl.LframeCtrl.LSetPoint.fCoeff, 6 * sizeof(float));
        aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.TargetLLLuma = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.TargetLLLuma, aec_xml.HdrAeCtrl.LframeCtrl.TargetLLLuma.fCoeff, 6 * sizeof(float));

        aec_json.HdrAeCtrl.MframeCtrl.MExpLevel_len = 6;
        aec_json.HdrAeCtrl.MframeCtrl.MSetPoint_len = 6;
        aec_json.HdrAeCtrl.MframeCtrl.MExpLevel = (float*)malloc(6 * sizeof(float));
        /*NOTE: use exp value here, not curexp/maxexp*/
        for(int i = 0; i < 6; i++) {
            aec_json.HdrAeCtrl.MframeCtrl.MExpLevel[i] = mmaxexp * aec_xml.HdrAeCtrl.MframeCtrl.MExpLevel.fCoeff[i];
        }
        aec_json.HdrAeCtrl.MframeCtrl.MSetPoint = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.MframeCtrl.MSetPoint, aec_xml.HdrAeCtrl.MframeCtrl.MSetPoint.fCoeff, 6 * sizeof(float));


        aec_json.HdrAeCtrl.SframeCtrl.HLROIExpandEn = aec_xml.HdrAeCtrl.SframeCtrl.HLROIExpandEn;
        aec_json.HdrAeCtrl.SframeCtrl.HLLumaTolerance = aec_xml.HdrAeCtrl.SframeCtrl.HLLumaTolerance;
        aec_json.HdrAeCtrl.SframeCtrl.SfrmSetPoint.SExpLevel_len = 6;
        aec_json.HdrAeCtrl.SframeCtrl.SfrmSetPoint.SSetPoint_len = 6;
        aec_json.HdrAeCtrl.SframeCtrl.SfrmSetPoint.TargetHLLuma_len = 6;
        aec_json.HdrAeCtrl.SframeCtrl.SfrmSetPoint.SExpLevel = (float*)malloc(6 * sizeof(float));
        /*NOTE: use exp value here, not curexp/maxexp*/
        for(int i = 0; i < 6; i++) {
            aec_json.HdrAeCtrl.SframeCtrl.SfrmSetPoint.SExpLevel[i] = smaxexp * aec_xml.HdrAeCtrl.SframeCtrl.SExpLevel.fCoeff[i];
        }
        aec_json.HdrAeCtrl.SframeCtrl.SfrmSetPoint.SSetPoint = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.SframeCtrl.SfrmSetPoint.SSetPoint, aec_xml.HdrAeCtrl.SframeCtrl.SSetPoint.fCoeff, 6 * sizeof(float));
        aec_json.HdrAeCtrl.SframeCtrl.SfrmSetPoint.TargetHLLuma = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.SframeCtrl.SfrmSetPoint.TargetHLLuma, aec_xml.HdrAeCtrl.SframeCtrl.TargetHLLuma.fCoeff, 6 * sizeof(float));

        //1.2.3 IrisCtrl
        aec_json.IrisCtrl.Enable = (aec_xml.CommCtrl.stIris.enable == 0) ? false : true;
        aec_json.IrisCtrl.IrisType = (CalibDb_IrisTypeV2_t)aec_xml.CommCtrl.stIris.IrisType;
        if(aec_xml.CommCtrl.AecOpType == RK_AIQ_OP_MODE_AUTO)
            aec_json.IrisCtrl.ManualEn = false;
        else
            aec_json.IrisCtrl.ManualEn = aec_xml.CommCtrl.stManual.stLinMe.ManualIrisEn;
        aec_json.IrisCtrl.ManualAttr.DCIrisHoldValue = aec_xml.CommCtrl.stManual.stLinMe.DCIrisValue;
        aec_json.IrisCtrl.ManualAttr.PIrisGainValue = aec_xml.CommCtrl.stManual.stLinMe.PIrisGainValue;
        aec_json.IrisCtrl.InitAttr.PIrisGainValue = aec_xml.CommCtrl.stInitExp.stLinExpInitExp.InitPIrisGainValue;
        aec_json.IrisCtrl.InitAttr.DCIrisHoldValue = aec_xml.CommCtrl.stInitExp.stLinExpInitExp.InitDCIrisDutyValue;
        aec_json.IrisCtrl.PIrisAttr.EffcStep = aec_xml.CommCtrl.stIris.PIrisAttr.EffcStep;
        aec_json.IrisCtrl.PIrisAttr.TotalStep = aec_xml.CommCtrl.stIris.PIrisAttr.TotalStep;
        aec_json.IrisCtrl.PIrisAttr.ZeroIsMax = aec_xml.CommCtrl.stIris.PIrisAttr.ZeroIsMax;
        memcpy(aec_json.IrisCtrl.PIrisAttr.StepTable, aec_xml.CommCtrl.stIris.PIrisAttr.StepTable, AECV2_PIRIS_STEP_TABLE_MAX * sizeof(uint16_t));
        aec_json.IrisCtrl.DCIrisAttr.Kp = aec_xml.CommCtrl.stIris.DCIrisAttr.Kp;
        aec_json.IrisCtrl.DCIrisAttr.Ki = aec_xml.CommCtrl.stIris.DCIrisAttr.Ki;
        aec_json.IrisCtrl.DCIrisAttr.Kd = aec_xml.CommCtrl.stIris.DCIrisAttr.Kd;
        aec_json.IrisCtrl.DCIrisAttr.MinPwmDuty = aec_xml.CommCtrl.stIris.DCIrisAttr.MinPwmDuty;
        aec_json.IrisCtrl.DCIrisAttr.MaxPwmDuty = aec_xml.CommCtrl.stIris.DCIrisAttr.MaxPwmDuty;
        aec_json.IrisCtrl.DCIrisAttr.OpenPwmDuty = aec_xml.CommCtrl.stIris.DCIrisAttr.OpenPwmDuty;
        aec_json.IrisCtrl.DCIrisAttr.ClosePwmDuty = aec_xml.CommCtrl.stIris.DCIrisAttr.ClosePwmDuty;

        aec_json.IrisCtrl.ManualAttr.HDCIrisTargetValue = aec_xml.CommCtrl.stManual.stLinMe.HDCIrisValue;
        aec_json.IrisCtrl.InitAttr.HDCIrisTargetValue = aec_xml.CommCtrl.stInitExp.stLinExpInitExp.InitHDCIrisTargetValue;
        aec_json.IrisCtrl.HDCIrisAttr.DampOver = aec_xml.CommCtrl.stIris.HDCIrisAttr.DampOver;
        aec_json.IrisCtrl.HDCIrisAttr.DampUnder = aec_xml.CommCtrl.stIris.HDCIrisAttr.DampUnder;
        aec_json.IrisCtrl.HDCIrisAttr.ZeroIsMax = aec_xml.CommCtrl.stIris.HDCIrisAttr.ZeroIsMax;
        aec_json.IrisCtrl.HDCIrisAttr.MinTarget = aec_xml.CommCtrl.stIris.HDCIrisAttr.MinTarget;
        aec_json.IrisCtrl.HDCIrisAttr.MaxTarget = aec_xml.CommCtrl.stIris.HDCIrisAttr.MaxTarget;
        memcpy(aec_json.IrisCtrl.HDCIrisAttr.ZoomTargetDot, aec_xml.CommCtrl.stIris.HDCIrisAttr.ZoomTargetDot, aec_xml.CommCtrl.stIris.HDCIrisAttr.zoom_array_size * sizeof(int));
        memcpy(aec_json.IrisCtrl.HDCIrisAttr.ZoomDot, aec_xml.CommCtrl.stIris.HDCIrisAttr.ZoomDot, aec_xml.CommCtrl.stIris.HDCIrisAttr.zoom_array_size * sizeof(int));
        memcpy(aec_json.IrisCtrl.HDCIrisAttr.IrisTargetDot, aec_xml.CommCtrl.stIris.HDCIrisAttr.IrisTargetDot, aec_xml.CommCtrl.stIris.HDCIrisAttr.iris_array_size * sizeof(int));
        memcpy(aec_json.IrisCtrl.HDCIrisAttr.GainDot, aec_xml.CommCtrl.stIris.HDCIrisAttr.GainDot, aec_xml.CommCtrl.stIris.HDCIrisAttr.iris_array_size * sizeof(int));
        aec_json.IrisCtrl.HDCIrisAttr.ZoomTargetDot_len = aec_xml.CommCtrl.stIris.HDCIrisAttr.zoom_array_size;
        aec_json.IrisCtrl.HDCIrisAttr.IrisTargetDot_len = aec_xml.CommCtrl.stIris.HDCIrisAttr.iris_array_size;

        //1.2.4 SyncTest
        aec_json.SyncTest.Enable = (aec_xml.CommCtrl.stSyncTest.enable == 0) ? false : true;
        aec_json.SyncTest.IntervalFrm = aec_xml.CommCtrl.stSyncTest.IntervalFrm;
        aec_json.SyncTest.AlterExp.HdrAE_len = aec_xml.CommCtrl.stSyncTest.HdrAlterExp.array_size;
        aec_json.SyncTest.AlterExp.HdrAE = (CalibDb_HdrAlterExpV2_t*)malloc(aec_xml.CommCtrl.stSyncTest.HdrAlterExp.array_size * sizeof(CalibDb_HdrAlterExpV2_t));
        for(int i = 0; i < aec_json.SyncTest.AlterExp.HdrAE_len; i++) {
            memcpy(aec_json.SyncTest.AlterExp.HdrAE[i].TimeValue, aec_xml.CommCtrl.stSyncTest.HdrAlterExp.TimeValue[i], 3 * sizeof(float));
            memcpy(aec_json.SyncTest.AlterExp.HdrAE[i].GainValue, aec_xml.CommCtrl.stSyncTest.HdrAlterExp.GainValue[i], 3 * sizeof(float));
            memcpy(aec_json.SyncTest.AlterExp.HdrAE[i].IspDGainValue, aec_xml.CommCtrl.stSyncTest.HdrAlterExp.IspDGainValue[i], 3 * sizeof(float));
            memcpy(aec_json.SyncTest.AlterExp.HdrAE[i].DcgMode, aec_xml.CommCtrl.stSyncTest.HdrAlterExp.DcgMode[i], 3 * sizeof(float));
            aec_json.SyncTest.AlterExp.HdrAE[i].PIrisGainValue = aec_xml.CommCtrl.stSyncTest.HdrAlterExp.PIrisGainValue[i];
        }
        aec_json.SyncTest.AlterExp.LinearAE_len = aec_xml.CommCtrl.stSyncTest.LinAlterExp.array_size;
        aec_json.SyncTest.AlterExp.LinearAE = (CalibDb_LinAlterExpV2_t*)malloc(aec_xml.CommCtrl.stSyncTest.LinAlterExp.array_size * sizeof(CalibDb_LinAlterExpV2_t));
        for(int i = 0; i < aec_json.SyncTest.AlterExp.LinearAE_len; i++) {
            aec_json.SyncTest.AlterExp.LinearAE[i].TimeValue = aec_xml.CommCtrl.stSyncTest.LinAlterExp.TimeValue[i];
            aec_json.SyncTest.AlterExp.LinearAE[i].GainValue = aec_xml.CommCtrl.stSyncTest.LinAlterExp.GainValue[i];
            aec_json.SyncTest.AlterExp.LinearAE[i].IspDGainValue = aec_xml.CommCtrl.stSyncTest.LinAlterExp.IspgainValue[i];
            aec_json.SyncTest.AlterExp.LinearAE[i].DcgMode = aec_xml.CommCtrl.stSyncTest.LinAlterExp.DcgMode[i];
            aec_json.SyncTest.AlterExp.LinearAE[i].PIrisGainValue = aec_xml.CommCtrl.stSyncTest.LinAlterExp.PIrisGainValue[i];
        }

    }


    if(CHECK_ISP_HW_V21()) {

        const CalibDb_Aec_CalibPara_t *aec_calib_xml = NULL;

        list_head *aec_calib_para_list =
            (list_head*)CALIBDB_GET_MODULE_PTR((void*)calib, ae_calib_para);

        if (aec_calib_para_list)
            GetAecProfileFromAecCalibListByIdx(aec_calib_para_list, 0, &aec_calib_xml);
        else
            return;

        const CalibDb_Aec_TunePara_t*aec_tune_xml = NULL;

        list_head *aec_tune_para_list =
            (list_head*)CALIBDB_GET_MODULE_PTR((void*)calib, ae_tuning_para);

        if (aec_tune_para_list)
            GetAecProfileFromAecTuneListByIdx(aec_tune_para_list, 0, &aec_tune_xml);
        else
            return;


        //1.2.0 CommCtrl
        aec_json.CommCtrl.Enable = (aec_tune_xml->CommCtrl.enable == 0) ? false : true;
        aec_json.CommCtrl.AecRunInterval = aec_tune_xml->CommCtrl.AecRunInterval;
        aec_json.CommCtrl.AecOpType = aec_tune_xml->CommCtrl.AecOpType;
        aec_json.CommCtrl.HistStatsMode = (CalibDb_CamHistStatsModeV2_t)aec_tune_xml->CommCtrl.HistStatsMode;
        switch(aec_tune_xml->CommCtrl.RawStatsMode) {
        case CAM_RAWSTATS_MODE_R:
            aec_json.CommCtrl.RawStatsMode = CAM_RAWSTATSV2_MODE_R;
            break;
        case CAM_RAWSTATS_MODE_G:
            aec_json.CommCtrl.RawStatsMode = CAM_RAWSTATSV2_MODE_G;
            break;
        case CAM_RAWSTATS_MODE_B:
            aec_json.CommCtrl.RawStatsMode = CAM_RAWSTATSV2_MODE_B;
            break;
        case CAM_RAWSTATS_MODE_Y:
        default:
            aec_json.CommCtrl.RawStatsMode = CAM_RAWSTATSV2_MODE_Y;
            break;
        }

        if(aec_tune_xml->CommCtrl.YRangeMode <= CAM_YRANGE_MODE_FULL)
            aec_json.CommCtrl.YRangeMode = CAM_YRANGEV2_MODE_FULL;
        else
            aec_json.CommCtrl.YRangeMode = CAM_YRANGEV2_MODE_LIMITED;

        for(int i = 0; i < AECV2_MAX_GRIDWEIGHT_NUM; i++)
            aec_json.CommCtrl.AecGridWeight[i] = aec_tune_xml->CommCtrl.GridWeights[i];

        aec_json.CommCtrl.AecSpeed.SmoothEn = aec_tune_xml->CommCtrl.stAuto.stAeSpeed.SmoothEn;
        aec_json.CommCtrl.AecSpeed.DyDampEn = aec_tune_xml->CommCtrl.stAuto.stAeSpeed.DyDampEn;
        aec_json.CommCtrl.AecSpeed.DampOver = aec_tune_xml->CommCtrl.stAuto.stAeSpeed.DampOver;
        aec_json.CommCtrl.AecSpeed.DampUnder = aec_tune_xml->CommCtrl.stAuto.stAeSpeed.DampUnder;
        aec_json.CommCtrl.AecSpeed.DampBright2Dark = aec_tune_xml->CommCtrl.stAuto.stAeSpeed.DampBright2Dark;
        aec_json.CommCtrl.AecSpeed.DampDark2Bright = aec_tune_xml->CommCtrl.stAuto.stAeSpeed.DampDark2Bright;

        aec_json.CommCtrl.AecDelayFrmNum.BlackDelay = aec_tune_xml->CommCtrl.stAuto.BlackDelayFrame;
        aec_json.CommCtrl.AecDelayFrmNum.WhiteDelay = aec_tune_xml->CommCtrl.stAuto.WhiteDelayFrame;

        aec_json.CommCtrl.AecAntiFlicker.enable = aec_tune_xml->CommCtrl.stAntiFlicker.enable;
        aec_json.CommCtrl.AecAntiFlicker.Frequency = (CalibDb_FlickerFreqV2_t)aec_tune_xml->CommCtrl.stAntiFlicker.Frequency;
        if(aec_tune_xml->CommCtrl.stAntiFlicker.Mode == AEC_ANTIFLICKER_AUTO_MODE)
            aec_json.CommCtrl.AecAntiFlicker.Mode = AECV2_ANTIFLICKER_AUTO_MODE;
        else
            aec_json.CommCtrl.AecAntiFlicker.Mode = AECV2_ANTIFLICKER_NORMAL_MODE;

        aec_json.CommCtrl.AecFrameRateMode.isFpsFix = aec_tune_xml->CommCtrl.stAuto.stFrmRate.isFpsFix;
        aec_json.CommCtrl.AecFrameRateMode.FpsValue = aec_tune_xml->CommCtrl.stAuto.stFrmRate.FpsValue;

        aec_json.CommCtrl.AecEnvLvCalib.CalibFNumber = aec_calib_xml->stEnvLvCalib.CalibFN;
        aec_json.CommCtrl.AecEnvLvCalib.CurveCoeff[0] = aec_calib_xml->stEnvLvCalib.Curve.fCoeff[0];
        aec_json.CommCtrl.AecEnvLvCalib.CurveCoeff[1] = aec_calib_xml->stEnvLvCalib.Curve.fCoeff[1];

        aec_json.CommCtrl.AecWinScale.InRawWinScale.h_offs = aec_calib_xml->InRawWinScale.h_offs;
        aec_json.CommCtrl.AecWinScale.InRawWinScale.v_offs = aec_calib_xml->InRawWinScale.v_offs;
        aec_json.CommCtrl.AecWinScale.InRawWinScale.h_size = aec_calib_xml->InRawWinScale.h_size;
        aec_json.CommCtrl.AecWinScale.InRawWinScale.v_size = aec_calib_xml->InRawWinScale.v_size;
        aec_json.CommCtrl.AecWinScale.YuvWinScale.h_offs = aec_calib_xml->YuvWinScale.h_offs;
        aec_json.CommCtrl.AecWinScale.YuvWinScale.v_offs = aec_calib_xml->YuvWinScale.v_offs;
        aec_json.CommCtrl.AecWinScale.YuvWinScale.h_size = aec_calib_xml->YuvWinScale.h_size;
        aec_json.CommCtrl.AecWinScale.YuvWinScale.v_size = aec_calib_xml->YuvWinScale.v_size;
        aec_json.CommCtrl.AecWinScale.TmoRawWinScale.h_offs = aec_calib_xml->TmoRawWinScale.h_offs;
        aec_json.CommCtrl.AecWinScale.TmoRawWinScale.v_offs = aec_calib_xml->TmoRawWinScale.v_offs;
        aec_json.CommCtrl.AecWinScale.TmoRawWinScale.h_size = aec_calib_xml->TmoRawWinScale.h_size;
        aec_json.CommCtrl.AecWinScale.TmoRawWinScale.v_size = aec_calib_xml->TmoRawWinScale.v_size;

        aec_json.CommCtrl.AecManualCtrl.LinearAE.ManualTimeEn = aec_tune_xml->CommCtrl.stManual.stLinMe.ManualTimeEn;
        aec_json.CommCtrl.AecManualCtrl.LinearAE.ManualGainEn = aec_tune_xml->CommCtrl.stManual.stLinMe.ManualGainEn;
        aec_json.CommCtrl.AecManualCtrl.LinearAE.ManualIspDgainEn = aec_tune_xml->CommCtrl.stManual.stLinMe.ManualIspDgainEn;
        aec_json.CommCtrl.AecManualCtrl.LinearAE.TimeValue = aec_tune_xml->CommCtrl.stManual.stLinMe.TimeValue;
        aec_json.CommCtrl.AecManualCtrl.LinearAE.GainValue = aec_tune_xml->CommCtrl.stManual.stLinMe.GainValue;
        aec_json.CommCtrl.AecManualCtrl.LinearAE.IspDGainValue = aec_tune_xml->CommCtrl.stManual.stLinMe.IspDGainValue;

        aec_json.CommCtrl.AecManualCtrl.HdrAE.ManualTimeEn = aec_tune_xml->CommCtrl.stManual.stHdrMe.ManualTimeEn;
        aec_json.CommCtrl.AecManualCtrl.HdrAE.ManualGainEn = aec_tune_xml->CommCtrl.stManual.stHdrMe.ManualGainEn;
        aec_json.CommCtrl.AecManualCtrl.HdrAE.ManualIspDgainEn = aec_tune_xml->CommCtrl.stManual.stHdrMe.ManualIspDgainEn;
        memcpy(aec_json.CommCtrl.AecManualCtrl.HdrAE.TimeValue, aec_tune_xml->CommCtrl.stManual.stHdrMe.TimeValue.fCoeff, 3 * sizeof(float));
        memcpy(aec_json.CommCtrl.AecManualCtrl.HdrAE.GainValue, aec_tune_xml->CommCtrl.stManual.stHdrMe.GainValue.fCoeff, 3 * sizeof(float));
        memcpy(aec_json.CommCtrl.AecManualCtrl.HdrAE.IspDGainValue, aec_tune_xml->CommCtrl.stManual.stHdrMe.IspDGainValue.fCoeff, 3 * sizeof(float));

        //1.2.1 LinearAeCtrl
        aec_json.LinearAeCtrl.RawStatsEn = (aec_tune_xml->LinearAeCtrl.RawStatsEn == 0) ? false : true;
        aec_json.LinearAeCtrl.Evbias = aec_tune_xml->LinearAeCtrl.Evbias;
        aec_json.LinearAeCtrl.ToleranceIn = aec_tune_xml->LinearAeCtrl.ToleranceIn;
        aec_json.LinearAeCtrl.ToleranceOut = aec_tune_xml->LinearAeCtrl.ToleranceOut;
        if(aec_tune_xml->LinearAeCtrl.StrategyMode <= RKAIQ_AEC_STRATEGY_MODE_LOWLIGHT_PRIOR)
            aec_json.LinearAeCtrl.StrategyMode = AECV2_STRATEGY_MODE_LOWLIGHT;
        else
            aec_json.LinearAeCtrl.StrategyMode = AECV2_STRATEGY_MODE_HIGHLIGHT;

        aec_json.LinearAeCtrl.InitExp.InitTimeValue = aec_tune_xml->CommCtrl.stInitExp.stLinExpInitExp.InitTimeValue;
        aec_json.LinearAeCtrl.InitExp.InitGainValue = aec_tune_xml->CommCtrl.stInitExp.stLinExpInitExp.InitGainValue;
        aec_json.LinearAeCtrl.InitExp.InitIspDGainValue = aec_tune_xml->CommCtrl.stInitExp.stLinExpInitExp.InitIspDGainValue;
        aec_json.LinearAeCtrl.InitExp.InitPIrisGainValue = aec_tune_xml->CommCtrl.stInitExp.stLinExpInitExp.InitPIrisGainValue;
        aec_json.LinearAeCtrl.InitExp.InitDCIrisDutyValue = aec_tune_xml->CommCtrl.stInitExp.stLinExpInitExp.InitDCIrisDutyValue;
        aec_json.LinearAeCtrl.InitExp.InitHDCIrisTargetValue = aec_tune_xml->CommCtrl.stInitExp.stLinExpInitExp.InitHDCIrisTargetValue;

        aec_json.LinearAeCtrl.Route.TimeDot_len = aec_tune_xml->CommCtrl.stAeRoute.LinAeSeperate.array_size;
        aec_json.LinearAeCtrl.Route.GainDot_len = aec_tune_xml->CommCtrl.stAeRoute.LinAeSeperate.array_size;
        aec_json.LinearAeCtrl.Route.IspDGainDot_len = aec_tune_xml->CommCtrl.stAeRoute.LinAeSeperate.array_size;
        aec_json.LinearAeCtrl.Route.PIrisDot_len = aec_tune_xml->CommCtrl.stAeRoute.LinAeSeperate.array_size;

        aec_json.LinearAeCtrl.Route.TimeDot = (float*)malloc(aec_json.LinearAeCtrl.Route.TimeDot_len * sizeof(float));
        memcpy(aec_json.LinearAeCtrl.Route.TimeDot, aec_tune_xml->CommCtrl.stAeRoute.LinAeSeperate.TimeDot, aec_json.LinearAeCtrl.Route.TimeDot_len * sizeof(float));
        aec_json.LinearAeCtrl.Route.GainDot = (float*)malloc(aec_json.LinearAeCtrl.Route.GainDot_len * sizeof(float));
        memcpy(aec_json.LinearAeCtrl.Route.GainDot, aec_tune_xml->CommCtrl.stAeRoute.LinAeSeperate.GainDot, aec_json.LinearAeCtrl.Route.GainDot_len * sizeof(float));
        aec_json.LinearAeCtrl.Route.IspDGainDot = (float*)malloc(aec_json.LinearAeCtrl.Route.IspDGainDot_len * sizeof(float));
        memcpy(aec_json.LinearAeCtrl.Route.IspDGainDot, aec_tune_xml->CommCtrl.stAeRoute.LinAeSeperate.IspgainDot, aec_json.LinearAeCtrl.Route.IspDGainDot_len * sizeof(float));
        aec_json.LinearAeCtrl.Route.PIrisDot = (int*)malloc(aec_json.LinearAeCtrl.Route.PIrisDot_len * sizeof(int));
        memcpy(aec_json.LinearAeCtrl.Route.PIrisDot, aec_tune_xml->CommCtrl.stAeRoute.LinAeSeperate.PIrisGainDot, aec_json.LinearAeCtrl.Route.PIrisDot_len * sizeof(float));


        aec_json.LinearAeCtrl.DySetpoint.ExpLevel_len = aec_tune_xml->LinearAeCtrl.DySetpoint.array_size;
        aec_json.LinearAeCtrl.DySetpoint.DySetpoint_len = aec_tune_xml->LinearAeCtrl.DySetpoint.array_size;

        aec_json.LinearAeCtrl.DySetpoint.ExpLevel = (float*)malloc(aec_json.LinearAeCtrl.DySetpoint.ExpLevel_len * sizeof(float));
        aec_json.LinearAeCtrl.DySetpoint.DySetpoint = (float*)malloc(aec_json.LinearAeCtrl.DySetpoint.DySetpoint_len * sizeof(float));
        /*NOTE: use exp value here, not curexp/maxexp*/
        float maxexp = aec_json.LinearAeCtrl.Route.TimeDot[aec_json.LinearAeCtrl.Route.TimeDot_len - 1] *
                       aec_json.LinearAeCtrl.Route.GainDot[aec_json.LinearAeCtrl.Route.GainDot_len - 1];
        for(int i = 0; i < aec_json.LinearAeCtrl.DySetpoint.ExpLevel_len; i++)
            aec_json.LinearAeCtrl.DySetpoint.ExpLevel[i] = maxexp * aec_tune_xml->LinearAeCtrl.DySetpoint.ExpValue[i];

        if(aec_tune_xml->LinearAeCtrl.DySetPointEn == false) {
            for(int i = 0; i < aec_json.LinearAeCtrl.DySetpoint.DySetpoint_len; i++)
                aec_json.LinearAeCtrl.DySetpoint.DySetpoint[i] = aec_tune_xml->LinearAeCtrl.SetPoint;
        } else {
            memcpy(aec_json.LinearAeCtrl.DySetpoint.DySetpoint, aec_tune_xml->LinearAeCtrl.DySetpoint.DySetpoint, aec_json.LinearAeCtrl.DySetpoint.DySetpoint_len * sizeof(float));
        }

        aec_json.LinearAeCtrl.BackLightCtrl.Enable = (aec_tune_xml->LinearAeCtrl.BackLightConf.enable == 0) ? false : true;
        aec_json.LinearAeCtrl.BackLightCtrl.StrBias = aec_tune_xml->LinearAeCtrl.BackLightConf.StrBias;
        aec_json.LinearAeCtrl.BackLightCtrl.MeasArea = (CalibDb_AecMeasAreaModeV2_t)aec_tune_xml->LinearAeCtrl.BackLightConf.MeasArea;
        aec_json.LinearAeCtrl.BackLightCtrl.OEROILowTh = aec_tune_xml->LinearAeCtrl.BackLightConf.OEROILowTh;
        aec_json.LinearAeCtrl.BackLightCtrl.LumaDistTh = aec_tune_xml->LinearAeCtrl.BackLightConf.LumaDistTh;
        aec_json.LinearAeCtrl.BackLightCtrl.LvHighTh = aec_tune_xml->LinearAeCtrl.BackLightConf.LvHightTh;
        aec_json.LinearAeCtrl.BackLightCtrl.LvLowTh = aec_tune_xml->LinearAeCtrl.BackLightConf.LvLowTh;
        aec_json.LinearAeCtrl.BackLightCtrl.BacklitSetPoint.ExpLevel_len = 6;
        aec_json.LinearAeCtrl.BackLightCtrl.BacklitSetPoint.TargetLLLuma_len = 6;
        aec_json.LinearAeCtrl.BackLightCtrl.BacklitSetPoint.NonOEPdfTh_len = 6;
        aec_json.LinearAeCtrl.BackLightCtrl.BacklitSetPoint.LowLightPdfTh_len = 6;
        aec_json.LinearAeCtrl.BackLightCtrl.BacklitSetPoint.ExpLevel = (float*)malloc(6 * sizeof(float));
        /*NOTE: use exp value here, not curexp/maxexp*/
        for(int i = 0; i < 6; i++)
            aec_json.LinearAeCtrl.BackLightCtrl.BacklitSetPoint.ExpLevel[i] = maxexp * aec_tune_xml->LinearAeCtrl.BackLightConf.ExpLevel.fCoeff[i];
        aec_json.LinearAeCtrl.BackLightCtrl.BacklitSetPoint.TargetLLLuma = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.LinearAeCtrl.BackLightCtrl.BacklitSetPoint.TargetLLLuma, aec_tune_xml->LinearAeCtrl.BackLightConf.TargetLLLuma.fCoeff, 6 * sizeof(float));
        aec_json.LinearAeCtrl.BackLightCtrl.BacklitSetPoint.NonOEPdfTh = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.LinearAeCtrl.BackLightCtrl.BacklitSetPoint.NonOEPdfTh, aec_tune_xml->LinearAeCtrl.BackLightConf.NonOEPdfTh.fCoeff, 6 * sizeof(float));
        aec_json.LinearAeCtrl.BackLightCtrl.BacklitSetPoint.LowLightPdfTh = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.LinearAeCtrl.BackLightCtrl.BacklitSetPoint.LowLightPdfTh, aec_tune_xml->LinearAeCtrl.BackLightConf.LowLightPdfTh.fCoeff, 6 * sizeof(float));

        aec_json.LinearAeCtrl.OverExpCtrl.Enable = (aec_tune_xml->LinearAeCtrl.OverExpCtrl.enable == 0) ? false : true;
        aec_json.LinearAeCtrl.OverExpCtrl.StrBias = aec_tune_xml->LinearAeCtrl.OverExpCtrl.StrBias;
        aec_json.LinearAeCtrl.OverExpCtrl.MaxWeight = aec_tune_xml->LinearAeCtrl.OverExpCtrl.MaxWeight;
        aec_json.LinearAeCtrl.OverExpCtrl.HighLightTh = aec_tune_xml->LinearAeCtrl.OverExpCtrl.HighLightTh;
        aec_json.LinearAeCtrl.OverExpCtrl.LowLightTh = aec_tune_xml->LinearAeCtrl.OverExpCtrl.LowLightTh;
        aec_json.LinearAeCtrl.OverExpCtrl.OverExpSetPoint.HighLightWeight_len = 6;
        aec_json.LinearAeCtrl.OverExpCtrl.OverExpSetPoint.LowLightWeight_len = 6;
        aec_json.LinearAeCtrl.OverExpCtrl.OverExpSetPoint.OEpdf_len = 6;
        aec_json.LinearAeCtrl.OverExpCtrl.OverExpSetPoint.HighLightWeight = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.LinearAeCtrl.OverExpCtrl.OverExpSetPoint.HighLightWeight, aec_tune_xml->LinearAeCtrl.OverExpCtrl.HighLightWeight.fCoeff, 6 * sizeof(float));
        aec_json.LinearAeCtrl.OverExpCtrl.OverExpSetPoint.LowLightWeight = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.LinearAeCtrl.OverExpCtrl.OverExpSetPoint.LowLightWeight, aec_tune_xml->LinearAeCtrl.OverExpCtrl.LowLightWeight.fCoeff, 6 * sizeof(float));
        aec_json.LinearAeCtrl.OverExpCtrl.OverExpSetPoint.OEpdf = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.LinearAeCtrl.OverExpCtrl.OverExpSetPoint.OEpdf, aec_tune_xml->LinearAeCtrl.OverExpCtrl.OEpdf.fCoeff, 6 * sizeof(float));

        //1.2.2 HdrAeCtrl
        aec_json.HdrAeCtrl.ToleranceIn = aec_tune_xml->HdrAeCtrl.ToleranceIn;
        aec_json.HdrAeCtrl.ToleranceOut = aec_tune_xml->HdrAeCtrl.ToleranceOut;
        aec_json.HdrAeCtrl.Evbias = aec_tune_xml->HdrAeCtrl.Evbias;
        aec_json.HdrAeCtrl.LumaDistTh = aec_tune_xml->HdrAeCtrl.LumaDistTh;
        if(aec_tune_xml->HdrAeCtrl.StrategyMode <= RKAIQ_AEC_STRATEGY_MODE_LOWLIGHT_PRIOR)
            aec_json.HdrAeCtrl.StrategyMode = AECV2_STRATEGY_MODE_LOWLIGHT;
        else
            aec_json.HdrAeCtrl.StrategyMode = AECV2_STRATEGY_MODE_HIGHLIGHT;


        memcpy(aec_json.HdrAeCtrl.InitExp.InitTimeValue, aec_tune_xml->CommCtrl.stInitExp.stHdrExpInitExp.InitTimeValue.fCoeff, 3 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.InitExp.InitGainValue, aec_tune_xml->CommCtrl.stInitExp.stHdrExpInitExp.InitGainValue.fCoeff, 3 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.InitExp.InitIspDGainValue, aec_tune_xml->CommCtrl.stInitExp.stHdrExpInitExp.InitIspDGainValue.fCoeff, 3 * sizeof(float));

        aec_json.HdrAeCtrl.InitExp.InitPIrisGainValue = aec_tune_xml->CommCtrl.stInitExp.stHdrExpInitExp.InitPIrisGainValue;
        aec_json.HdrAeCtrl.InitExp.InitDCIrisDutyValue = aec_tune_xml->CommCtrl.stInitExp.stHdrExpInitExp.InitDCIrisDutyValue;
        aec_json.HdrAeCtrl.InitExp.InitHDCIrisTargetValue = aec_tune_xml->CommCtrl.stInitExp.stHdrExpInitExp.InitHDCIrisTargetValue;

        aec_json.HdrAeCtrl.Route.Frm0TimeDot_len = aec_tune_xml->CommCtrl.stAeRoute.HdrAeSeperate.array_size;
        aec_json.HdrAeCtrl.Route.Frm0GainDot_len = aec_tune_xml->CommCtrl.stAeRoute.HdrAeSeperate.array_size;
        aec_json.HdrAeCtrl.Route.Frm0IspDGainDot_len = aec_tune_xml->CommCtrl.stAeRoute.HdrAeSeperate.array_size;
        aec_json.HdrAeCtrl.Route.Frm1TimeDot_len = aec_tune_xml->CommCtrl.stAeRoute.HdrAeSeperate.array_size;
        aec_json.HdrAeCtrl.Route.Frm1GainDot_len = aec_tune_xml->CommCtrl.stAeRoute.HdrAeSeperate.array_size;
        aec_json.HdrAeCtrl.Route.Frm1IspDGainDot_len = aec_tune_xml->CommCtrl.stAeRoute.HdrAeSeperate.array_size;
        aec_json.HdrAeCtrl.Route.Frm2TimeDot_len = aec_tune_xml->CommCtrl.stAeRoute.HdrAeSeperate.array_size;
        aec_json.HdrAeCtrl.Route.Frm2GainDot_len = aec_tune_xml->CommCtrl.stAeRoute.HdrAeSeperate.array_size;
        aec_json.HdrAeCtrl.Route.Frm2IspDGainDot_len = aec_tune_xml->CommCtrl.stAeRoute.HdrAeSeperate.array_size;
        aec_json.HdrAeCtrl.Route.PIrisDot_len = aec_tune_xml->CommCtrl.stAeRoute.HdrAeSeperate.array_size;

        aec_json.HdrAeCtrl.Route.Frm0TimeDot = (float*)malloc(aec_json.HdrAeCtrl.Route.Frm0TimeDot_len * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.Route.Frm0TimeDot, aec_tune_xml->CommCtrl.stAeRoute.HdrAeSeperate.HdrTimeDot[0], aec_json.HdrAeCtrl.Route.Frm0TimeDot_len * sizeof(float));
        aec_json.HdrAeCtrl.Route.Frm0GainDot = (float*)malloc(aec_json.HdrAeCtrl.Route.Frm0GainDot_len * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.Route.Frm0GainDot, aec_tune_xml->CommCtrl.stAeRoute.HdrAeSeperate.HdrGainDot[0], aec_json.HdrAeCtrl.Route.Frm0GainDot_len * sizeof(float));
        aec_json.HdrAeCtrl.Route.Frm0IspDGainDot = (float*)malloc(aec_json.HdrAeCtrl.Route.Frm0IspDGainDot_len * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.Route.Frm0IspDGainDot, aec_tune_xml->CommCtrl.stAeRoute.HdrAeSeperate.HdrIspDGainDot[0], aec_json.HdrAeCtrl.Route.Frm0IspDGainDot_len * sizeof(float));
        aec_json.HdrAeCtrl.Route.Frm1TimeDot = (float*)malloc(aec_json.HdrAeCtrl.Route.Frm1TimeDot_len * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.Route.Frm1TimeDot, aec_tune_xml->CommCtrl.stAeRoute.HdrAeSeperate.HdrTimeDot[1], aec_json.HdrAeCtrl.Route.Frm1TimeDot_len * sizeof(float));
        aec_json.HdrAeCtrl.Route.Frm1GainDot = (float*)malloc(aec_json.HdrAeCtrl.Route.Frm1GainDot_len * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.Route.Frm1GainDot, aec_tune_xml->CommCtrl.stAeRoute.HdrAeSeperate.HdrGainDot[1], aec_json.HdrAeCtrl.Route.Frm1GainDot_len * sizeof(float));
        aec_json.HdrAeCtrl.Route.Frm1IspDGainDot = (float*)malloc(aec_json.HdrAeCtrl.Route.Frm1IspDGainDot_len * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.Route.Frm1IspDGainDot, aec_tune_xml->CommCtrl.stAeRoute.HdrAeSeperate.HdrIspDGainDot[1], aec_json.HdrAeCtrl.Route.Frm1IspDGainDot_len * sizeof(float));
        aec_json.HdrAeCtrl.Route.Frm2TimeDot = (float*)malloc(aec_json.HdrAeCtrl.Route.Frm2TimeDot_len * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.Route.Frm2TimeDot, aec_tune_xml->CommCtrl.stAeRoute.HdrAeSeperate.HdrTimeDot[2], aec_json.HdrAeCtrl.Route.Frm2TimeDot_len * sizeof(float));
        aec_json.HdrAeCtrl.Route.Frm2GainDot = (float*)malloc(aec_json.HdrAeCtrl.Route.Frm2GainDot_len * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.Route.Frm2GainDot, aec_tune_xml->CommCtrl.stAeRoute.HdrAeSeperate.HdrGainDot[2], aec_json.HdrAeCtrl.Route.Frm2GainDot_len * sizeof(float));
        aec_json.HdrAeCtrl.Route.Frm2IspDGainDot = (float*)malloc(aec_json.HdrAeCtrl.Route.Frm2IspDGainDot_len * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.Route.Frm2IspDGainDot, aec_tune_xml->CommCtrl.stAeRoute.HdrAeSeperate.HdrIspDGainDot[2], aec_json.HdrAeCtrl.Route.Frm2IspDGainDot_len * sizeof(float));
        aec_json.HdrAeCtrl.Route.PIrisDot = (int*)malloc(aec_json.HdrAeCtrl.Route.PIrisDot_len * sizeof(int));
        memcpy(aec_json.HdrAeCtrl.Route.PIrisDot, aec_tune_xml->CommCtrl.stAeRoute.HdrAeSeperate.PIrisGainDot, aec_json.HdrAeCtrl.Route.PIrisDot_len * sizeof(float));

        aec_json.HdrAeCtrl.LongFrmMode.mode = (CalibDb_AeHdrLongFrmModeV2_t)aec_tune_xml->HdrAeCtrl.LongfrmMode;
        aec_json.HdrAeCtrl.LongFrmMode.SfrmMinLine = aec_tune_xml->HdrAeCtrl.SfrmMinLine;
        aec_json.HdrAeCtrl.LongFrmMode.LfrmModeExpTh = aec_tune_xml->HdrAeCtrl.LfrmModeExpTh;

        if(aec_tune_xml->HdrAeCtrl.ExpRatioType <= RKAIQ_HDRAE_RATIOTYPE_MODE_AUTO)
            aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatioType = AECV2_HDR_RATIOTYPE_MODE_AUTO;
        else
            aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatioType = AECV2_HDR_RATIOTYPE_MODE_FIX;
        aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.RatioExpDot_len = 6;
        aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.M2SRatioFix_len = 6;
        aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.L2MRatioFix_len = 6;
        aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.M2SRatioMax_len = 6;
        aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.L2MRatioMax_len = 6;
        aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.RatioExpDot = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.RatioExpDot, aec_tune_xml->HdrAeCtrl.RatioExpDot.fCoeff, 6 * sizeof(float));
        aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.M2SRatioFix = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.M2SRatioFix, aec_tune_xml->HdrAeCtrl.M2SRatioFix.fCoeff, 6 * sizeof(float));
        aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.L2MRatioFix = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.L2MRatioFix, aec_tune_xml->HdrAeCtrl.L2MRatioFix.fCoeff, 6 * sizeof(float));
        aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.M2SRatioMax = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.M2SRatioMax, aec_tune_xml->HdrAeCtrl.M2SRatioMax.fCoeff, 6 * sizeof(float));
        aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.L2MRatioMax = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.ExpRatioCtrl.ExpRatio.L2MRatioMax, aec_tune_xml->HdrAeCtrl.L2MRatioMax.fCoeff, 6 * sizeof(float));

        aec_json.HdrAeCtrl.LframeCtrl.LvHighTh = aec_tune_xml->HdrAeCtrl.LframeCtrl.LvHighTh;
        aec_json.HdrAeCtrl.LframeCtrl.LvLowTh = aec_tune_xml->HdrAeCtrl.LframeCtrl.LvLowTh;
        aec_json.HdrAeCtrl.LframeCtrl.OEROILowTh = aec_tune_xml->HdrAeCtrl.LframeCtrl.OEROILowTh;
        aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.LExpLevel_len = 6;
        aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.NonOEPdfTh_len = 6;
        aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.LowLightPdfTh_len = 6;
        aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.LSetPoint_len = 6;
        aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.TargetLLLuma_len = 6;
        float lmaxexp = aec_json.HdrAeCtrl.Route.Frm1TimeDot[aec_json.HdrAeCtrl.Route.Frm1TimeDot_len - 1] *
                        aec_json.HdrAeCtrl.Route.Frm1GainDot[aec_json.HdrAeCtrl.Route.Frm1GainDot_len - 1];
        float mmaxexp = aec_json.HdrAeCtrl.Route.Frm1TimeDot[aec_json.HdrAeCtrl.Route.Frm1TimeDot_len - 1] *
                        aec_json.HdrAeCtrl.Route.Frm1GainDot[aec_json.HdrAeCtrl.Route.Frm1GainDot_len - 1];
        float smaxexp = aec_json.HdrAeCtrl.Route.Frm0TimeDot[aec_json.HdrAeCtrl.Route.Frm0TimeDot_len - 1] *
                        aec_json.HdrAeCtrl.Route.Frm0GainDot[aec_json.HdrAeCtrl.Route.Frm0GainDot_len - 1];
        aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.LExpLevel = (float*)malloc(6 * sizeof(float));
        /*NOTE: use exp value here, not curexp/maxexp*/
        for(int i = 0; i < 6; i++) {
            aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.LExpLevel[i] = lmaxexp * aec_tune_xml->HdrAeCtrl.LframeCtrl.LExpLevel.fCoeff[i];
        }
        aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.NonOEPdfTh = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.NonOEPdfTh, aec_tune_xml->HdrAeCtrl.LframeCtrl.NonOEPdfTh.fCoeff, 6 * sizeof(float));
        aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.LowLightPdfTh = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.LowLightPdfTh, aec_tune_xml->HdrAeCtrl.LframeCtrl.LowLightPdfTh.fCoeff, 6 * sizeof(float));
        aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.LSetPoint = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.LSetPoint, aec_tune_xml->HdrAeCtrl.LframeCtrl.LSetPoint.fCoeff, 6 * sizeof(float));
        aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.TargetLLLuma = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.LframeCtrl.LfrmSetPoint.TargetLLLuma, aec_tune_xml->HdrAeCtrl.LframeCtrl.TargetLLLuma.fCoeff, 6 * sizeof(float));

        aec_json.HdrAeCtrl.MframeCtrl.MExpLevel_len = 6;
        aec_json.HdrAeCtrl.MframeCtrl.MSetPoint_len = 6;
        aec_json.HdrAeCtrl.MframeCtrl.MExpLevel = (float*)malloc(6 * sizeof(float));
        /*NOTE: use exp value here, not curexp/maxexp*/
        for(int i = 0; i < 6; i++) {
            aec_json.HdrAeCtrl.MframeCtrl.MExpLevel[i] = mmaxexp * aec_tune_xml->HdrAeCtrl.MframeCtrl.MExpLevel.fCoeff[i];
        }
        aec_json.HdrAeCtrl.MframeCtrl.MSetPoint = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.MframeCtrl.MSetPoint, aec_tune_xml->HdrAeCtrl.MframeCtrl.MSetPoint.fCoeff, 6 * sizeof(float));


        aec_json.HdrAeCtrl.SframeCtrl.HLROIExpandEn = aec_tune_xml->HdrAeCtrl.SframeCtrl.HLROIExpandEn;
        aec_json.HdrAeCtrl.SframeCtrl.HLLumaTolerance = aec_tune_xml->HdrAeCtrl.SframeCtrl.HLLumaTolerance;
        aec_json.HdrAeCtrl.SframeCtrl.SfrmSetPoint.SExpLevel_len = 6;
        aec_json.HdrAeCtrl.SframeCtrl.SfrmSetPoint.SSetPoint_len = 6;
        aec_json.HdrAeCtrl.SframeCtrl.SfrmSetPoint.TargetHLLuma_len = 6;
        aec_json.HdrAeCtrl.SframeCtrl.SfrmSetPoint.SExpLevel = (float*)malloc(6 * sizeof(float));
        /*NOTE: use exp value here, not curexp/maxexp*/
        for(int i = 0; i < 6; i++) {
            aec_json.HdrAeCtrl.SframeCtrl.SfrmSetPoint.SExpLevel[i] = smaxexp * aec_tune_xml->HdrAeCtrl.SframeCtrl.SExpLevel.fCoeff[i];
        }
        aec_json.HdrAeCtrl.SframeCtrl.SfrmSetPoint.SSetPoint = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.SframeCtrl.SfrmSetPoint.SSetPoint, aec_tune_xml->HdrAeCtrl.SframeCtrl.SSetPoint.fCoeff, 6 * sizeof(float));
        aec_json.HdrAeCtrl.SframeCtrl.SfrmSetPoint.TargetHLLuma = (float*)malloc(6 * sizeof(float));
        memcpy(aec_json.HdrAeCtrl.SframeCtrl.SfrmSetPoint.TargetHLLuma, aec_tune_xml->HdrAeCtrl.SframeCtrl.TargetHLLuma.fCoeff, 6 * sizeof(float));

        //1.2.3 IrisCtrl
        aec_json.IrisCtrl.Enable = (aec_tune_xml->CommCtrl.stIris.enable == 0) ? false : true;
        aec_json.IrisCtrl.IrisType = (CalibDb_IrisTypeV2_t)aec_tune_xml->CommCtrl.stIris.IrisType;
        if(aec_tune_xml->CommCtrl.AecOpType == RK_AIQ_OP_MODE_AUTO)
            aec_json.IrisCtrl.ManualEn = false;
        else
            aec_json.IrisCtrl.ManualEn = aec_tune_xml->CommCtrl.stManual.stLinMe.ManualIrisEn;
        aec_json.IrisCtrl.ManualAttr.DCIrisHoldValue = aec_tune_xml->CommCtrl.stManual.stLinMe.DCIrisValue;
        aec_json.IrisCtrl.ManualAttr.PIrisGainValue = aec_tune_xml->CommCtrl.stManual.stLinMe.PIrisGainValue;
        aec_json.IrisCtrl.InitAttr.PIrisGainValue = aec_tune_xml->CommCtrl.stInitExp.stLinExpInitExp.InitPIrisGainValue;
        aec_json.IrisCtrl.InitAttr.DCIrisHoldValue = aec_tune_xml->CommCtrl.stInitExp.stLinExpInitExp.InitDCIrisDutyValue;
        aec_json.IrisCtrl.PIrisAttr.EffcStep = aec_tune_xml->CommCtrl.stIris.PIrisAttr.EffcStep;
        aec_json.IrisCtrl.PIrisAttr.TotalStep = aec_tune_xml->CommCtrl.stIris.PIrisAttr.TotalStep;
        aec_json.IrisCtrl.PIrisAttr.ZeroIsMax = aec_tune_xml->CommCtrl.stIris.PIrisAttr.ZeroIsMax;
        memcpy(aec_json.IrisCtrl.PIrisAttr.StepTable, aec_tune_xml->CommCtrl.stIris.PIrisAttr.StepTable, AECV2_PIRIS_STEP_TABLE_MAX * sizeof(uint16_t));
        aec_json.IrisCtrl.DCIrisAttr.Kp = aec_tune_xml->CommCtrl.stIris.DCIrisAttr.Kp;
        aec_json.IrisCtrl.DCIrisAttr.Ki = aec_tune_xml->CommCtrl.stIris.DCIrisAttr.Ki;
        aec_json.IrisCtrl.DCIrisAttr.Kd = aec_tune_xml->CommCtrl.stIris.DCIrisAttr.Kd;
        aec_json.IrisCtrl.DCIrisAttr.MinPwmDuty = aec_tune_xml->CommCtrl.stIris.DCIrisAttr.MinPwmDuty;
        aec_json.IrisCtrl.DCIrisAttr.MaxPwmDuty = aec_tune_xml->CommCtrl.stIris.DCIrisAttr.MaxPwmDuty;
        aec_json.IrisCtrl.DCIrisAttr.OpenPwmDuty = aec_tune_xml->CommCtrl.stIris.DCIrisAttr.OpenPwmDuty;
        aec_json.IrisCtrl.DCIrisAttr.ClosePwmDuty = aec_tune_xml->CommCtrl.stIris.DCIrisAttr.ClosePwmDuty;

        aec_json.IrisCtrl.ManualAttr.HDCIrisTargetValue = aec_tune_xml->CommCtrl.stManual.stLinMe.HDCIrisValue;
        aec_json.IrisCtrl.InitAttr.HDCIrisTargetValue = aec_tune_xml->CommCtrl.stInitExp.stLinExpInitExp.InitHDCIrisTargetValue;
        aec_json.IrisCtrl.HDCIrisAttr.DampOver = aec_tune_xml->CommCtrl.stIris.HDCIrisAttr.DampOver;
        aec_json.IrisCtrl.HDCIrisAttr.DampUnder = aec_tune_xml->CommCtrl.stIris.HDCIrisAttr.DampUnder;
        aec_json.IrisCtrl.HDCIrisAttr.ZeroIsMax = aec_tune_xml->CommCtrl.stIris.HDCIrisAttr.ZeroIsMax;
        aec_json.IrisCtrl.HDCIrisAttr.MinTarget = aec_tune_xml->CommCtrl.stIris.HDCIrisAttr.MinTarget;
        aec_json.IrisCtrl.HDCIrisAttr.MaxTarget = aec_tune_xml->CommCtrl.stIris.HDCIrisAttr.MaxTarget;
        memcpy(aec_json.IrisCtrl.HDCIrisAttr.ZoomTargetDot, aec_tune_xml->CommCtrl.stIris.HDCIrisAttr.ZoomTargetDot, aec_tune_xml->CommCtrl.stIris.HDCIrisAttr.zoom_array_size * sizeof(int));
        memcpy(aec_json.IrisCtrl.HDCIrisAttr.ZoomDot, aec_tune_xml->CommCtrl.stIris.HDCIrisAttr.ZoomDot, aec_tune_xml->CommCtrl.stIris.HDCIrisAttr.zoom_array_size * sizeof(int));
        memcpy(aec_json.IrisCtrl.HDCIrisAttr.IrisTargetDot, aec_tune_xml->CommCtrl.stIris.HDCIrisAttr.IrisTargetDot, aec_tune_xml->CommCtrl.stIris.HDCIrisAttr.iris_array_size * sizeof(int));
        memcpy(aec_json.IrisCtrl.HDCIrisAttr.GainDot, aec_tune_xml->CommCtrl.stIris.HDCIrisAttr.GainDot, aec_tune_xml->CommCtrl.stIris.HDCIrisAttr.iris_array_size * sizeof(int));
        aec_json.IrisCtrl.HDCIrisAttr.ZoomTargetDot_len = aec_tune_xml->CommCtrl.stIris.HDCIrisAttr.zoom_array_size;
        aec_json.IrisCtrl.HDCIrisAttr.IrisTargetDot_len = aec_tune_xml->CommCtrl.stIris.HDCIrisAttr.iris_array_size;

        //1.2.4 SyncTest
        aec_json.SyncTest.Enable = (aec_tune_xml->CommCtrl.stSyncTest.enable == 0) ? false : true;
        aec_json.SyncTest.IntervalFrm = aec_tune_xml->CommCtrl.stSyncTest.IntervalFrm;
        aec_json.SyncTest.AlterExp.HdrAE_len = aec_tune_xml->CommCtrl.stSyncTest.HdrAlterExp.array_size;
        aec_json.SyncTest.AlterExp.HdrAE = (CalibDb_HdrAlterExpV2_t*)malloc(aec_tune_xml->CommCtrl.stSyncTest.HdrAlterExp.array_size * sizeof(CalibDb_HdrAlterExpV2_t));
        for(int i = 0; i < aec_json.SyncTest.AlterExp.HdrAE_len; i++) {
            memcpy(aec_json.SyncTest.AlterExp.HdrAE[i].TimeValue, aec_tune_xml->CommCtrl.stSyncTest.HdrAlterExp.TimeValue[i], 3 * sizeof(float));
            memcpy(aec_json.SyncTest.AlterExp.HdrAE[i].GainValue, aec_tune_xml->CommCtrl.stSyncTest.HdrAlterExp.GainValue[i], 3 * sizeof(float));
            memcpy(aec_json.SyncTest.AlterExp.HdrAE[i].IspDGainValue, aec_tune_xml->CommCtrl.stSyncTest.HdrAlterExp.IspDGainValue[i], 3 * sizeof(float));
            memcpy(aec_json.SyncTest.AlterExp.HdrAE[i].DcgMode, aec_tune_xml->CommCtrl.stSyncTest.HdrAlterExp.DcgMode[i], 3 * sizeof(float));
            aec_json.SyncTest.AlterExp.HdrAE[i].PIrisGainValue = aec_tune_xml->CommCtrl.stSyncTest.HdrAlterExp.PIrisGainValue[i];
        }
        aec_json.SyncTest.AlterExp.LinearAE_len = aec_tune_xml->CommCtrl.stSyncTest.LinAlterExp.array_size;
        aec_json.SyncTest.AlterExp.LinearAE = (CalibDb_LinAlterExpV2_t*)malloc(aec_tune_xml->CommCtrl.stSyncTest.LinAlterExp.array_size * sizeof(CalibDb_LinAlterExpV2_t));
        for(int i = 0; i < aec_json.SyncTest.AlterExp.LinearAE_len; i++) {
            aec_json.SyncTest.AlterExp.LinearAE[i].TimeValue = aec_tune_xml->CommCtrl.stSyncTest.LinAlterExp.TimeValue[i];
            aec_json.SyncTest.AlterExp.LinearAE[i].GainValue = aec_tune_xml->CommCtrl.stSyncTest.LinAlterExp.GainValue[i];
            aec_json.SyncTest.AlterExp.LinearAE[i].IspDGainValue = aec_tune_xml->CommCtrl.stSyncTest.LinAlterExp.IspgainValue[i];
            aec_json.SyncTest.AlterExp.LinearAE[i].DcgMode = aec_tune_xml->CommCtrl.stSyncTest.LinAlterExp.DcgMode[i];
            aec_json.SyncTest.AlterExp.LinearAE[i].PIrisGainValue = aec_tune_xml->CommCtrl.stSyncTest.LinAlterExp.PIrisGainValue[i];
        }

    }

    CalibDb_Aec_ParaV2_t* calibv2_ae_calib =
        (CalibDb_Aec_ParaV2_t*)(CALIBDBV2_GET_MODULE_PTR(calibV2, ae_calib));
    *calibv2_ae_calib = aec_json;

}

