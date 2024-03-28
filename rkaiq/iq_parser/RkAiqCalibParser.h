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

#ifndef _RK_AIQ_CALIB_PARSER_H_
#define _RK_AIQ_CALIB_PARSER_H_
#include <string.h>
#include "tinyxml2.h"

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibTag.h"
#include "xmltags.h"
#include "RkAiqCalibApi.h"

using namespace tinyxml2;

#if defined(_MSC_VER)
#define strcasecmp _stricmp
#define snprintf(buf,len, format,...) _snprintf_s(buf, len, len-1, format, __VA_ARGS__)
#endif



#if defined(__linux__)
#include "smartptr.h"
#include <xcam_common.h>
#include "xcam_log.h"

#ifdef DCT_ASSERT
#undef DCT_ASSERT
#endif
#define DCT_ASSERT  assert

#elif defined(_WIN32)

#ifdef DCT_ASSERT
#undef DCT_ASSERT
#endif
#define DCT_ASSERT(x) if(!(x))return false

#define LOGI printf
#define LOGD printf
#define LOGW printf
#define LOGE printf
#define LOG1 printf

#endif

#define XML_PARSER_READ    0
#define XML_PARSER_WRITE   1


namespace RkCam {

class RkAiqCalibParser
{
private:
    typedef bool (RkAiqCalibParser::*parseCellContent)(const XMLElement*, void* param);
    typedef bool (RkAiqCalibParser::*parseCellContent2)(const XMLElement*, void* param, int index);

    // parse helper
    bool parseCellNoElement(const XMLElement* pelement, int noElements, int &RealNo);
    bool parseEntryCell(const XMLElement*, int, parseCellContent, void* param = NULL,
                        uint32_t cur_id = 0, uint32_t parent_id = 0);
    bool parseEntryCell2(const XMLElement*, int, parseCellContent2, void* param = NULL,
                         uint32_t cur_id = 0, uint32_t parent_id = 0);
    bool parseEntryCell3(XMLElement*, int, int, parseCellContent, void* param = NULL,
                         uint32_t cur_id = 0, uint32_t parent_id = 0);
    bool parseEntryCell4(XMLElement*, int, int, parseCellContent2, void* param = NULL,
                         uint32_t cur_id = 0, uint32_t parent_id = 0);
    // parse read/write control
    bool xmlParseReadWrite;
    char autoTabStr[128];
    int  autoTabIdx;
    void parseReadWriteCtrl(bool ctrl);
    void autoTabForward();
    void autoTabBackward();

    // parse Kit
    int ParseCharToHex(XmlTag* tag, uint32_t* reg_value);
    int ParseByteArray(const XMLNode *pNode, uint8_t* values, const int num);
    int ParseFloatArray(const XMLNode *pNode, float* values, const int num, int printAccuracy = 4);
    int ParseDoubleArray(const XMLNode *pNode, double* values, const int num);
    int ParseUintArray(const XMLNode *pNode, uint32_t* values, const int num);
    int ParseIntArray(const XMLNode *pNode, int32_t* values, const int num);
    int ParseUcharArray(const XMLNode *pNode, uint8_t* values, const int num);
    int ParseCharArray(const XMLNode *pNode, int8_t* values, const int num);
    int ParseUshortArray(const XMLNode *pNode, uint16_t* values, const int num);
    int ParseShortArray(const XMLNode *pNode, int16_t* values, const int num);
    int ParseString(const XMLNode *pNode, char* values, const int size);
    int ParseLscProfileArray(const XMLNode *pNode, CalibDb_Lsc_ProfileName_t values[], const int num);


    // parse Header
    bool parseEntryHeader(const XMLElement*, void* param = NULL);
    bool parseEntrySensor(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwb(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbCalibParaV200(const XMLElement*, void* param, int index);
    bool parseEntrySensorAwbCalibParaV201(const XMLElement*, void* param, int index);
    bool parseEntrySensorAwbAdjustPara(const XMLElement*, void* param, int index);
    bool parseEntrySensorAwbMeasureGlobalsV200(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbMeasureLightSourcesV200(const XMLElement*, void* param, int index);
    bool parseEntrySensorAwbMeasureGlobalsV201(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbLightXYRegionV201(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbLightRTYUVRegionV201(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbMeasureLightSourcesV201(const XMLElement*, void* param, int index);
    bool parseEntrySensorAwbStrategyLightSources(const XMLElement*, void* param, int index);
    bool parseEntrySensorAwbStrategyGlobals(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbLsForYuvDet(const XMLElement*, void* param, int index);
    bool parseEntrySensorAwbLsForYuvDetV201(const XMLElement*, void* param, int index);
    bool parseEntrySensorAwbWindowV201(const XMLElement*, void* param, int index);
    bool parseEntrySensorAwbMeasureWindowV201(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbLimitRangeV201(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbWpDiffWeiEnableTh(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbWpDiffwei_w_HighLV(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbWpDiffwei_w_LowLV(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbWpDiffLumaWeight(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbFrameChoose(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbFrameChooseV201(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbMeasureWindow(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbLimitRange(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbWindow(const XMLElement*, void* param, int index);
    bool parseEntrySensorAwbSingleColorV200(const XMLElement*, void* param);
    bool parseEntrySensorAwbColBlkV200(const XMLElement*, void* param, int index);
    bool parseEntrySensorAwbLsForEstimationV200(const XMLElement*, void* param, int index);
    bool parseEntrySensorAwbSingleColorV201(const XMLElement*, void* param);
    bool parseEntrySensorAwbColBlkV201(const XMLElement*, void* param, int index);
    bool parseEntrySensorAwbLsForEstimationV201(const XMLElement*, void* param, int index);



    bool parseEntrySensorAwbwbGainAdjust(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbwbGainOffset(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbDampFactor(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbXyRegionStableSelection(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbwbGainDaylightClipV200(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbwbGainClipV200(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbwbGainDaylightClipV201(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbwbGainClipV201(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbCctLutAll(const XMLElement*, void* param, int index);
    bool parseEntrySensorAwbGlobalsExclude(const XMLElement*, void* param, int index);
    bool parseEntrySensorAwbLightXYRegion(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbLightYUVRegion(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbGlobalsExcludeV201(const XMLElement*, void* param, int index);
    bool parseEntrySensorAwbLightSources(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbRemosaicPara(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAwbXyRegionWpnumthCell(const XMLElement*, void* param, int index);
    bool parseEntrySensorAwbwbWpTh(const XMLElement*, void* param, int index);
    bool parseEntrySensorAwbLimitRangeCell(const XMLElement*, void* param, int index);
    bool parseEntrySensorAwbLimitRangeV201Cell(const XMLElement*, void* param, int index);
    bool parseEntrySensorAecLinAlterExp(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecHdrAlterExp(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecAlterExp(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecSyncTest(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecSpeed(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecDelayFrmNum(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecVBNightMode(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecIRNightMode(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecDNSwitch(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecAntiFlicker(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecFrameRateMode(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecRangeLinearAE(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecRangeHdrAE(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecRange(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecInitValueLinearAE(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecInitValueHdrAE(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecInitValue(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecGridWeight(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecIrisCtrlPAttr(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecIrisCtrlDCAttr(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecIrisCtrlHDCAttr(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecIrisCtrl(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecManualCtrlLinearAE(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecManualCtrlHdrAE(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecManualCtrl(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecEnvLvCalib(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecRoute(const XMLElement*, void* param = NULL);
    bool parseEntrySensorLinearAECtrlAoe(const XMLElement*, void* param = NULL);
    bool parseEntrySensorLinearAECtrl(const XMLElement*, void* param = NULL);
    bool parseEntrySensorLinearAECtrlWeightMethod(const XMLElement*, void* param = NULL);
    bool parseEntrySensorLinearAECtrlDarkROIMethod(const XMLElement*, void* param = NULL);
    bool parseEntrySensorLinearAECtrlBackLight(const XMLElement*, void* param = NULL);
    bool parseEntrySensorLinearAECtrlOverExp(const XMLElement*, void* param = NULL);
    bool parseEntrySensorIntervalAdjustStrategy(const XMLElement*, void* param = NULL);
    bool parseEntrySensorLinearAECtrlHist2hal(const XMLElement*, void* param = NULL);
    bool parseEntrySensorHdrAECtrlExpRatioCtrl(const XMLElement*, void* param = NULL);
    bool parseEntrySensorHdrAECtrlLframeCtrl(const XMLElement*, void* param = NULL);
    bool parseEntrySensorHdrAECtrlLframeMode(const XMLElement*, void* param = NULL);
    bool parseEntrySensorHdrAECtrlMframeCtrl(const XMLElement*, void* param = NULL);
    bool parseEntrySensorHdrAECtrlSframeCtrl(const XMLElement*, void* param = NULL);
    bool parseEntrySensorHdrAECtrl(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAec(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecLinearAeRoute(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecHdrAeRoute(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecLinearAeDynamicPoint(const XMLElement*, void* param = NULL);

    bool parseEntrySensorAecLinAlterExpV21(const XMLElement*, void* param, int index);
    bool parseEntrySensorAecHdrAlterExpV21(const XMLElement*, void* param, int index);
    bool parseEntrySensorAecAlterExpV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecSyncTestV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecSpeedV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecDelayFrmNumV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecVBNightModeV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecIRNightModeV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecDNSwitchV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecAntiFlickerV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecFrameRateModeV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecRangeLinearAEV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecRangeHdrAEV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecRangeV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecInitValueLinearAEV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecInitValueHdrAEV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecInitValueV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecIrisCtrlPAttrV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecIrisCtrlDCAttrV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecIrisCtrlHDCAttrV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecIrisCtrlV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecManualCtrlLinearAEV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecManualCtrlHdrAEV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecManualCtrlV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecEnvLvCalibV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecLinearRouteV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecHdrRouteV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecRouteV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecLinearAeDynamicPointV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorLinearAECtrlBackLightV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorLinearAECtrlOverExpV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorLinearAECtrlV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorIntervalAdjustStrategyV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorLinearAECtrlHist2halV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorHdrAECtrlExpRatioCtrlV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorHdrAECtrlLframeCtrlV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorHdrAECtrlLframeModeV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorHdrAECtrlMframeCtrlV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorHdrAECtrlSframeCtrlV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorHdrAECtrlV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecWinScaleV21(const XMLElement*, void* param = NULL);
    bool parseEntrySensorAecCalibPara(const XMLElement*, void* param, int index);
    bool parseEntrySensorAecTunePara(const XMLElement*, void* param, int index);
    bool parseEntrySensorAecV21(const XMLElement*, void* param = NULL);

    bool parseEntrySensorAhdrMerge(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorAhdrTmo(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorAhdrTmoEn(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorAhdrTmoGlobalLuma(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorAhdrTmoDetailsHighLight(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorAhdrTmoDetailsLowLight(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorAhdrLocalTMO(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorAhdrGlobalTMO(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorDrc(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorDrcCalibPara(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorDrcTuningPara(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorBlcModeCell(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorBlc(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorLut3d(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorDpcc(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorDpccFastMode(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorDpccExpertMode(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorDpccSetCell(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorDpccSetCellRK(const XMLElement* pelement, int index );
    bool parseEntrySensorDpccSetCellLC(const XMLElement* pelement, int index );
    bool parseEntrySensorDpccSetCellPG(const XMLElement* pelement, int index );
    bool parseEntrySensorDpccSetCellRND(const XMLElement* pelement, int index );
    bool parseEntrySensorDpccSetCellRG(const XMLElement* pelement, int index);
    bool parseEntrySensorDpccSetCellRO(const XMLElement* pelement, int index );
    bool parseEntrySensorDpccPdaf(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorDpccSensor(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorBayerNr(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorBayerNrModeCell(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorBayerNrSetting(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorLsc(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorLscAlscCof(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorLscAlscCofResAll(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorLscAlscCofIllAll(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorLscTableAll(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorRKDM(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorlumaCCMGAC(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorlumaCCM(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorCCM(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorCCMModeCell(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorCcmAccmCof(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorCcmAccmCofIllAll(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorCcmMatrixAll(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorUVNR(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorUVNRModeCell(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorUVNRSetting(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorGamma(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorDegamma(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorDegammaModeCell(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorYnr(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorYnrModeCell(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorYnrSetting(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorYnrISO(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorGic(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorGicCalibParaV20(const XMLElement* pelement, void* param);
    bool parseEntrySensorGicTuningParaV20(const XMLElement* pelement, void* param);
    bool parseEntrySensorGicCalibSettingV20( const XMLElement* pelement, void* param, int index) ;
    bool parseEntrySensorGicTuningSettingV20( const XMLElement* pelement, void* param, int index) ;
    bool parseEntrySensorGicCalibParaV21(const XMLElement* pelement, void* param);
    bool parseEntrySensorGicTuningParaV21(const XMLElement* pelement, void* param);
    bool parseEntrySensorGicCalibSettingV21( const XMLElement* pelement, void* param, int index) ;
    bool parseEntrySensorGicTuningSettingV21( const XMLElement* pelement, void* param, int index) ;
    bool parseEntrySensorMFNR(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorMFNRDynamic(const XMLElement*   pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorMFNRMotionDetection(const XMLElement*   pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorMFNRModeCell(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorMFNRAwbUvRatio(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorMFNRISO(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorMFNRSetting(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorSharp(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorSharpModeCell(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorSharpSetting(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorSharpISO(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorEdgeFilter(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorEdgeFilterModeCell(const XMLElement* pelement, void* param = NULL );
    bool parseEntrySensorEdgeFilterSetting(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorEdgeFilterISO(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorDehaze(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorDehazeCalibParaV20(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorDehazeCalibParaV21(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorDehazeTuningParaV20(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorDehazeTuningParaV21(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorDehazeSettingV20(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorEnhanceSettingV20(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorHistSettingV20(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorDehazeTuningSettingV20(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorEnhanceTuningSettingV20(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorHistTuningSettingV20(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorDehazeSettingV21(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorEnhanceSettingV21(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorHistSettingV21(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorDehazeTuningSettingV21(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorEnhanceTuningSettingV21(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorHistTuningSettingV21(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorIIRSetting(const XMLElement* pelement, void* param = NULL, int index = 0);
    bool parseEntrySensorAfWindow(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorAfFixedMode(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorAfMacroMode(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorAfInfinityMode(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorAfContrastAf(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorAfLaserAf(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorAfPdaf(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorAfVcmCfg(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorAfMeasISO(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorAfZoomFocusTbl(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorAf(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorLdch(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorFec(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorEis(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorLumaDetect(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorOrb(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorInfoGainRange(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorInfo(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorModuleInfo(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorCpsl(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorColorAsGrey(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorCproc(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySensorIE(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySystemHDR(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySystemDcgNormalGainCtrl(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySystemDcgHdrGainCtrl(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySystemDcgNormalEnvCtrl(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySystemDcgHdrEnvCtrl(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySystemDcgNormalSetting(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySystemDcgHdrSetting(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySystemDcgSetting(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySystemExpDelayHdr(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySystemExpDelayNormal(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySystemExpDelay(const XMLElement* pelement, void* param = NULL);
    bool parseEntrySystem(const XMLElement*, void* param = NULL);
    bool parseEntryExpSet(const XMLElement* pelement, void* param = NULL);
    bool parseEntryExpSetGain2Reg(const XMLElement* pelement, void* param = NULL);
    bool parseEntryExpSetGainSet(const XMLElement* pelement, void* param = NULL);
    bool parseEntryExpSetTimeSet(const XMLElement* pelement, void* param = NULL);
    bool parseEntryExpSetHdrTimeSet(const XMLElement* pelement, void* param = NULL);
    bool parseEntryExpSetNormalTimeSet(const XMLElement* pelement, void* param = NULL);
    bool parseEntryExpSetHdrSet(const XMLElement* pelement, void* param = NULL);
    bool parseEntryExpSetDcgSet(const XMLElement* pelement, void* param = NULL);
    bool parseEntryExpSetDcgNormalSet(const XMLElement* pelement, void* param = NULL);
    bool parseEntryExpSetDcgHdrSet(const XMLElement* pelement, void* param = NULL);
    bool parseEntryExpSetDcgNormalGainCtrl(const XMLElement* pelement, void* param = NULL);
    bool parseEntryExpSetDcgHdrGainCtrl(const XMLElement* pelement, void* param = NULL);
    bool parseEntryExpSetDcgNormalEnvCtrl(const XMLElement* pelement, void* param = NULL);
    bool parseEntryExpSetDcgHdrEnvCtrl(const XMLElement* pelement, void* param = NULL);
    bool parseEntryExpSetExpUpdateHdr(const XMLElement* pelement, void* param = NULL);
    bool parseEntryExpSetExpUpdateNormal(const XMLElement* pelement, void* param = NULL);
    bool parseEntryExpSetExpUpdate(const XMLElement* pelement, void* param = NULL);
    bool parseEntryModuleInfo(const XMLElement* pelement, void* param = NULL);


    bool parseEntrySensorBayernrV2
    (
        const XMLElement* pelement,
        void* param = NULL
    ) ;

    bool parseEntryBayernrV2Setting_2D
    (
        const XMLElement*   pelement,
        void*                param,
        int                  index
    );

    bool parseEntryBayernrV2Setting_3D
    (
        const XMLElement*   pelement,
        void*                param,
        int                  index
    );

    bool parseEntrySensorBayernrV2Setting2D
    (
        const XMLElement*   pelement,
        void*                param
    );

    bool parseEntrySensorBayernrV2Setting3D
    (
        const XMLElement*   pelement,
        void*                param
    );
    bool parseEntrySensorYnrV2
    (
        const XMLElement* pelement,
        void* param
    );
    bool parseEntryYnrV2Setting
    (
        const XMLElement*   pelement,
        void*                param,
        int                  index
    );
    bool parseEntrySensorCnrV1
    (
        const XMLElement* pelement,
        void* param
    );
    bool parseEntryCnrV1Setting
    (
        const XMLElement*   pelement,
        void*                param,
        int                  index
    );
    bool parseEntrySharpV3Setting
    (
        const XMLElement*   pelement,
        void*                param,
        int                  index
    );
    bool parseEntrySensorSharpV3
    (
        const XMLElement* pelement,
        void* param
    );
public:
    explicit RkAiqCalibParser(CamCalibDbContext_t *pCalibDb);
    virtual ~RkAiqCalibParser();
    bool doParse(const char* device);
    bool doGenerate(const char* deviceRef, const char* deviceOutput);
    void  updateXmlParseReadWriteFlag(int flag);


private:
    CamCalibDbContext_t *mCalibDb;

#if defined(__linux__)
    XCAM_DEAD_COPY (RkAiqCalibParser);
#endif
};

} //namespace RkCam

#endif
