/*
 * sensor_descriptor.h - sensor descriptor
 *
 *  Copyright (c) 2015 Intel Corporation
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
 * Author: Wind Yuan <feng.yuan@intel.com>
 */

#include "settings_processor.h"
#include "rkisp_dev_manager.h"
#include <base/log.h>

SettingsProcessor::SettingsProcessor()
{
}

SettingsProcessor::~SettingsProcessor()
{
}

void
SettingsProcessor::parseMeteringRegion(const CameraMetadata *settings,
                                   int tagId, XCam3AWindow *meteringWindow) {
    camera_metadata_ro_entry_t entry;
    int weight = 0;

    if (tagId == ANDROID_CONTROL_AE_REGIONS ||
        tagId == ANDROID_CONTROL_AWB_REGIONS ||
        tagId == ANDROID_CONTROL_AF_REGIONS) {
        entry = settings->find(tagId);
        if (entry.count >= 5) {
            meteringWindow->x_start = entry.data.i32[0];
            meteringWindow->y_start = entry.data.i32[1];
            meteringWindow->x_end = entry.data.i32[2];
            meteringWindow->y_end = entry.data.i32[3];
            meteringWindow->weight = entry.data.i32[4];
            // TODO support more than one metering region
        } else
            LOGW("invalid control entry count %d", entry.count);
    } else {
        LOGW("Unsupported tag ID (%d) is given", tagId);
    }
}

/**
 * \brief Converts AE related metadata into AeInputParams
 *
 * \param[in]  settings request settings in Google format
 * \param[out] aeInputParams all parameters for ae processing
 *
 * \return success or error.
 */
XCamReturn
SettingsProcessor::fillAeInputParams(const CameraMetadata *settings,
                                     AiqInputParams *aiqInputParams)
{
    LOGI("@%s %d: enter", __FUNCTION__, __LINE__);
    if (settings == nullptr || aiqInputParams == nullptr) {
        LOGE("@%s %d: invalid settings(%p) or aiqInputParams(%p)",
             __FUNCTION__, __LINE__, settings, aiqInputParams);
        return XCAM_RETURN_ERROR_UNKNOWN;
    }
    AeInputParams* aeInputParams = &aiqInputParams->aeInputParams;
    AeControls *aeCtrl = &aiqInputParams->aaaControls.ae;
    XCamAeParam *aeParams = &aeInputParams->aeParams;

    //# METADATA_Control control.aeLock done
    camera_metadata_ro_entry entry = settings->find(ANDROID_CONTROL_AE_LOCK);
    if (entry.count == 1) {
        aeCtrl->aeLock = entry.data.u8[0];
    }

    uint8_t controlMode = ANDROID_CONTROL_MODE_AUTO;
    uint8_t aeMode = ANDROID_CONTROL_AE_MODE_ON;
    entry = settings->find(ANDROID_CONTROL_AE_MODE);
    if (entry.count == 1)
        aeMode = entry.data.u8[0];
    aeCtrl->aeMode = aeMode;

    entry = settings->find(ANDROID_CONTROL_MODE);
    if (entry.count == 1)
        controlMode = entry.data.u8[0];
    aiqInputParams->aaaControls.controlMode = controlMode;

    if (controlMode == ANDROID_CONTROL_MODE_AUTO ||
        controlMode == ANDROID_CONTROL_MODE_USE_SCENE_MODE)
        aeParams->mode = XCAM_AE_MODE_AUTO;
    else if (controlMode == ANDROID_CONTROL_MODE_OFF || aeMode == ANDROID_CONTROL_AE_MODE_OFF)
        aeParams->mode = XCAM_AE_MODE_MANUAL;

    // ******** metering_mode
    // TODO: implement the metering mode. For now the metering mode is fixed
    // to whole frame
    aeParams->metering_mode = XCAM_AE_METERING_MODE_AUTO;

    // ******** flicker_reduction_mode
    //# METADATA_Control control.aeAntibandingMode done
    entry = settings->find(ANDROID_CONTROL_AE_ANTIBANDING_MODE);
    if (entry.count == 1) {
        uint8_t flickerMode = entry.data.u8[0];
        aeCtrl->aeAntibanding = flickerMode;

        switch (flickerMode) {
            case ANDROID_CONTROL_AE_ANTIBANDING_MODE_OFF:
                aeParams->flicker_mode = XCAM_AE_FLICKER_MODE_OFF;
                break;
            case ANDROID_CONTROL_AE_ANTIBANDING_MODE_50HZ:
                aeParams->flicker_mode = XCAM_AE_FLICKER_MODE_50HZ;
                break;
            case ANDROID_CONTROL_AE_ANTIBANDING_MODE_60HZ:
                aeParams->flicker_mode = XCAM_AE_FLICKER_MODE_60HZ;
                break;
            case ANDROID_CONTROL_AE_ANTIBANDING_MODE_AUTO:
                aeParams->flicker_mode = XCAM_AE_FLICKER_MODE_AUTO;
                break;
            default:
                LOGE("ERROR @%s: Unknow flicker mode %d", __FUNCTION__, flickerMode);
                return XCAM_RETURN_ERROR_UNKNOWN;
        }
    }

    parseMeteringRegion(settings, ANDROID_CONTROL_AE_REGIONS, &aeParams->window);

    /* TODO  crop region*/
    /* CameraWindow *aeRegion = aeInputParams->aeRegion; */
    /* CameraWindow *croppingRegion = aeInputParams->croppingRegion; */
    /* if (aeRegion && croppingRegion) { */
        // ******** exposure_window
        //# METADATA_Control control.aeRegions done
        /* parseMeteringRegion(settings, ANDROID_CONTROL_AE_REGIONS, aeRegion); */
        /* if (aeRegion->isValid()) { */
        /*     // Clip the region to the crop rectangle */
        /*     if (croppingRegion->isValid()) */
        /*         aeRegion->clip(*croppingRegion); */

        /*     aiqInputParams->aeParams.window->h_offset = aeRegion->left(); */
        /*     aiqInputParams->aeParams.window->v_offset = aeRegion->top(); */
        /*     aiqInputParams->aeParams.window->width = aeRegion->width(); */
        /*     aiqInputParams->aeParams.window->height = aeRegion->height(); */
        /* } */
    /* } */
    // ******** exposure_coordinate
    /*
     * MANUAL AE CONTROL
     */
    if (aeParams->mode == XCAM_AE_MODE_MANUAL) {
        // ******** manual_exposure_time_us
        //# METADATA_Control sensor.exposureTime done
        entry = settings->find(ANDROID_SENSOR_EXPOSURE_TIME);
        if (entry.count == 1) {
            int64_t timeMicros = entry.data.i64[0] / 1000;
            if (timeMicros > 0) {
                /* TODO  need add exposure time limited mechanism*/
                /* if (timeMicros > mMaxExposureTime / 1000) { */
                /*     LOGE("exposure time %" PRId64 " ms is bigger than the max exposure time %" PRId64 " ms", */
                /*         timeMicros, mMaxExposureTime / 1000); */
                /*     return XCAM_RETURN_ERROR_UNKNOWN; */
                /* } else if (timeMicros < mMinExposureTime / 1000) { */
                /*     LOGE("exposure time %" PRId64 " ms is smaller than the min exposure time %" PRId64 " ms", */
                /*         timeMicros, mMinExposureTime / 1000); */
                /*     return XCAM_RETURN_ERROR_UNKNOWN; */
                /* } */
                aeParams->manual_exposure_time = timeMicros;
            } else {
                // Don't constrain AIQ.
                aeParams->manual_exposure_time = 0;
            }
        }

        // ******** manual frame time --> frame rate
        //# METADATA_Control sensor.frameDuration done
        /* TODO  need add frameduration*/
        /* entry = settings->find(ANDROID_SENSOR_FRAME_DURATION); */
        /* if (entry.count == 1) { */
        /*     int64_t timeMicros = entry.data.i64[0] / 1000; */
        /*     if (timeMicros > 0) { */
        /*         if (timeMicros > mMaxFrameDuration / 1000) { */
        /*             LOGE("frame duration %" PRId64 " ms is bigger than the max frame duration %" PRId64 " ms", */
        /*                 timeMicros, mMaxFrameDuration / 1000); */
        /*             return XCAM_RETURN_ERROR_UNKNOWN; */
        /*         } */
        /*         aiqInputParams->aeParams.manual_limits-> */
        /*           manual_frame_time_us_min = (int)timeMicros; */
        /*         aiqInputParams->aeParams.manual_limits-> */
        /*           manual_frame_time_us_max = (int)timeMicros; */
        /*     } else { */
        /*         // Don't constrain AIQ. */
        /*         aiqInputParams->aeParams.manual_limits-> */
        /*             manual_frame_time_us_min = -1; */
        /*         aiqInputParams->aeParams.manual_limits-> */
        /*             manual_frame_time_us_max = -1; */
        /*     } */
        /* } */

        // ******** manual_iso
        //# METADATA_Control sensor.sensitivity done
        entry = settings->find(ANDROID_SENSOR_SENSITIVITY);
        if (entry.count == 1) {
            int32_t iso = entry.data.i32[0];
            aeParams->manual_analog_gain = iso;
            /* TODO  need add iso limited mechanism*/
            /* if (iso >= mMinSensitivity && iso <= mMaxSensitivity) { */
            /*     aiqInputParams->aeParams.manual_iso[0] = iso; */
            /*     aiqInputParams->aeParams.manual_limits-> */
            /*         manual_iso_min = aiqInputParams->aeParams.manual_iso[0]; */
            /*     aiqInputParams->aeParams.manual_limits-> */
            /*         manual_iso_max = aiqInputParams->aeParams.manual_iso[0]; */
            /* } else */
            /*     aiqInputParams->aeParams.manual_iso = nullptr; */
        }
        // fill target fps range, it needs to be proper in results anyway
        entry = settings->find(ANDROID_CONTROL_AE_TARGET_FPS_RANGE);
        if (entry.count == 2) {
            aeCtrl->aeTargetFpsRange[0] = entry.data.i32[0];
            aeCtrl->aeTargetFpsRange[1] = entry.data.i32[1];
        }

    } else {
        /*
         *  AUTO AE CONTROL
         */
        // ******** ev_shift
        //# METADATA_Control control.aeExposureCompensation done
        entry = settings->find(ANDROID_CONTROL_AE_EXPOSURE_COMPENSATION);
        if (entry.count == 1) {

            CameraMetadata& staticMeta = RkispDeviceManager::get_static_metadata();
            float stepEV = 1 / 3.0f;

            camera_metadata_entry_t _entry;
            _entry = staticMeta.find(ANDROID_CONTROL_AE_COMPENSATION_STEP);
            if (_entry.type == TYPE_RATIONAL || _entry.count == 1) {
                const camera_metadata_rational_t* aeCompStep = _entry.data.r;
                stepEV = (float)aeCompStep->numerator / aeCompStep->denominator;
            } else {
                LOGE("@%s %d: can't get EV step, type:%d, count:%d", __FUNCTION__, __LINE__, _entry.type, _entry.count);
            }

            /* TODO  need add compensation limitation */
            /* int32_t evCompensation = CLIP(entry.data.i32[0] + aeInputParams->extraEvShift, */
            /*                               mMaxAeCompensation, mMinAeCompensation); */

            aeCtrl->evCompensation = entry.data.i32[0];

            aeParams->ev_shift = aeCtrl->evCompensation * stepEV;
        } else {
            aeParams->ev_shift = 0.0f;
        }
        aeParams->manual_exposure_time = 0;
        aeParams->manual_analog_gain = 0;

        // ******** target fps
        /* int32_t maxSupportedFps = INT_MAX; */
        /* if (aeInputParams->maxSupportedFps != 0) */
        /*     maxSupportedFps = aeInputParams->maxSupportedFps; */
        //# METADATA_Control control.aeTargetFpsRange done
        entry = settings->find(ANDROID_CONTROL_AE_TARGET_FPS_RANGE);
        if (entry.count == 2) {
            /* int32_t minFps = MIN(entry.data.i32[0], maxSupportedFps); */
            /* int32_t maxFps = MIN(entry.data.i32[1], maxSupportedFps); */
            /* aeCtrl->aeTargetFpsRange[0] = minFps; */
            /* aeCtrl->aeTargetFpsRange[1] = maxFps; */

            aeCtrl->aeTargetFpsRange[0] = entry.data.i32[0];
            aeCtrl->aeTargetFpsRange[1] = entry.data.i32[1];
        }

        //# METADATA_Control control.aePrecaptureTrigger done
        entry = settings->find(ANDROID_CONTROL_AE_PRECAPTURE_TRIGGER);
        if (entry.count == 1) {
            aeCtrl->aePreCaptureTrigger = entry.data.u8[0];
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

/**
 * fillAwbInputParams
 *
 * Converts the capture request settings in format into input parameters
 * for the AWB algorithm and Parameter Adaptor that is in charge of color
 * correction.
 *
 * It also provides the AWB mode that is used in PSL code.
 * we do the parsing here so that it is done only once.
 *
 * \param[in] settings: Camera metadata with the capture request settings.
 * \param[out] awbInputParams: parameters for aiq and other awb processing.
 *
 * \return BAD_VALUE if settings was nullptr.
 * \return NO_ERROR in normal situation.
 */
XCamReturn
SettingsProcessor::fillAwbInputParams(const CameraMetadata *settings,
                                      AiqInputParams *aiqInputParams)
{
    LOGI("@%s %d: enter", __FUNCTION__, __LINE__);
    _XCamAwbParam *awbCfg;
    AwbControls *awbCtrl;

    if (settings == nullptr
        || aiqInputParams == nullptr) {
        LOGE("settings = %p, aiqInput = %p", settings, aiqInputParams);
        return XCAM_RETURN_ERROR_UNKNOWN;
    }

    awbCfg = &aiqInputParams->awbInputParams.awbParams;
    awbCtrl = &aiqInputParams->aaaControls.awb;

    // AWB lock
    camera_metadata_ro_entry entry;
    //# METADATA_Control control.awbLock done
    entry = settings->find(ANDROID_CONTROL_AWB_LOCK);
    if (entry.count == 1) {
        awbCtrl->awbLock = entry.data.u8[0];
    }

    //# METADATA_Control control.awbMode done
    awbCtrl->awbMode = ANDROID_CONTROL_AWB_MODE_AUTO;
    entry = settings->find(ANDROID_CONTROL_AWB_MODE);
    if (entry.count == 1) {
        awbCtrl->awbMode = entry.data.u8[0];
    }
    LOGI("@%s %d:metadata awbMode:%d, awbLock:%d", __FUNCTION__, __LINE__, awbCtrl->awbMode, awbCtrl->awbLock);

    switch (awbCtrl->awbMode) {
    case ANDROID_CONTROL_AWB_MODE_OFF:
        awbCfg->mode = XCAM_AWB_MODE_MANUAL;
        break;
    case ANDROID_CONTROL_AWB_MODE_AUTO:
        awbCfg->mode = XCAM_AWB_MODE_AUTO;
        break;
    case ANDROID_CONTROL_AWB_MODE_INCANDESCENT:
        awbCfg->mode = XCAM_AWB_MODE_WARM_INCANDESCENT;
        break;
    case ANDROID_CONTROL_AWB_MODE_FLUORESCENT:
        awbCfg->mode = XCAM_AWB_MODE_FLUORESCENT;
        break;
    case ANDROID_CONTROL_AWB_MODE_WARM_FLUORESCENT:
        awbCfg->mode = XCAM_AWB_MODE_WARM_FLUORESCENT;
        break;
    case ANDROID_CONTROL_AWB_MODE_DAYLIGHT:
        awbCfg->mode = XCAM_AWB_MODE_DAYLIGHT;
        break;
    case ANDROID_CONTROL_AWB_MODE_CLOUDY_DAYLIGHT:
        awbCfg->mode = XCAM_AWB_MODE_CLOUDY;
        break;
    case ANDROID_CONTROL_AWB_MODE_SHADE:
        awbCfg->mode = XCAM_AWB_MODE_SHADOW;
        break;
    default :
        awbCfg->mode = XCAM_AWB_MODE_AUTO;
        break;
    }

    /* frame_use
     *  BEWARE - THIS VALUE MAY NOT WORK WITH AIQ WHICH RUNS PRE-CAPTURE
     *  WITH STILL FRAME_USE, WHILE THE HAL GETS PREVIEW INTENTS DURING PRE-
     *  CAPTURE!!!
     */

    //# METADATA_Control control.awbRegion done
    parseMeteringRegion(settings, ANDROID_CONTROL_AWB_REGIONS, &awbCfg->window);

    /*
     * MANUAL COLOR CORRECTION
     */
    awbCtrl->colorCorrectionMode = ANDROID_COLOR_CORRECTION_MODE_FAST;
    //# METADATA_Control colorCorrection.mode done
    entry = settings->find(ANDROID_COLOR_CORRECTION_MODE);
    if (entry.count == 1) {
        awbCtrl->colorCorrectionMode = entry.data.u8[0];
    }

    awbCtrl->colorCorrectionAberrationMode = ANDROID_COLOR_CORRECTION_ABERRATION_MODE_FAST;
    //# METADATA_Control colorCorrection.aberrationMode done
    entry = settings->find(ANDROID_COLOR_CORRECTION_ABERRATION_MODE);
    if (entry.count == 1) {
        awbCtrl->colorCorrectionAberrationMode = entry.data.u8[0];
    }

    // if awbMode is not OFF, then colorCorrection mode TRANSFORM_MATRIX should
    // be ignored and overwrittern to FAST.
    if (awbCtrl->awbMode != ANDROID_CONTROL_AWB_MODE_OFF &&
        awbCtrl->colorCorrectionMode == ANDROID_COLOR_CORRECTION_MODE_TRANSFORM_MATRIX) {
        awbCtrl->colorCorrectionMode = ANDROID_COLOR_CORRECTION_MODE_FAST;
    }

    if (awbCtrl->awbMode == ANDROID_CONTROL_AWB_MODE_OFF) {
        //# METADATA_Control colorCorrection.transform done
        entry = settings->find(ANDROID_COLOR_CORRECTION_TRANSFORM);
        if (entry.count == 9) {
            for (size_t i = 0; i < entry.count; i++) {
                int32_t numerator = entry.data.r[i].numerator;
                int32_t denominator = entry.data.r[i].denominator;
                /* TODO no struct to contain the colortransform,  need add */
                /* awbInputParams->aiqInputParams->manualColorTransform[i] = (float) numerator / denominator; */
            }
        }

        //# METADATA_Control colorCorrection.gains done
        entry = settings->find(ANDROID_COLOR_CORRECTION_GAINS);
        if (entry.count == 4) {
            // The color gains from application is in order of RGGB
            awbCfg->r_gain = entry.data.f[0];
            awbCfg->gr_gain = entry.data.f[1];
            awbCfg->gb_gain = entry.data.f[2];
            awbCfg->b_gain = entry.data.f[3];
        }
    }

    //# METADATA_Control control.awbRegions done
    //# METADATA_Dynamic control.awbRegions done
    //# AM Not Supported by 3a
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
SettingsProcessor::processAeSettings(const CameraMetadata &settings,
                             AiqInputParams &aiqparams) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = fillAeInputParams(&settings, &aiqparams);

    return ret;
}

XCamReturn
SettingsProcessor::processAwbSettings(const CameraMetadata &settings,
                              AiqInputParams &aiqparams) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = fillAwbInputParams(&settings, &aiqparams);

    return ret;
}

XCamReturn
SettingsProcessor::processRequestSettings(const CameraMetadata &settings,
                             AiqInputParams &aiqparams) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if ((ret = processAeSettings(settings, aiqparams)) != XCAM_RETURN_NO_ERROR)
        return ret;

    if ((ret = processAwbSettings(settings, aiqparams)) != XCAM_RETURN_NO_ERROR)
        return ret;
    return ret;
}
