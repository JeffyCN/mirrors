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

/**
 * Parses the request setting to find one of the 3 metering regions
 *
 * CONTROL_AE_REGIONS
 * CONTROL_AWB_REGIONS
 * CONTROL_AF_REGIONS
 *
 * It then initializes a CameraWindow structure. If no metering region is found
 * the CameraWindow is initialized empty. Users of this method can check this
 * by calling CameraWindow::isValid().
 *
 * \param[in] settings request settings to parse
 * \param[in] tagID one of the 3 metadata tags for the metering regions
 *                   (AE,AWB or AF)
 * \param[out] meteringWindow initialized region.
 *
 */
void SettingsProcessor::parseMeteringRegion(const CameraMetadata *settings,
                                   int tagId, CameraWindow *meteringWindow)
{
    camera_metadata_ro_entry_t entry;
    ia_coordinate topLeft, bottomRight;
    CLEAR(topLeft);
    CLEAR(bottomRight);
    int weight = 0;

    CameraWindow croppingRegion;
    int width, height;

    entry = settings->find(ANDROID_SCALER_CROP_REGION);
    if (entry.count == 4) {
        topLeft.x = entry.data.i32[0];
        topLeft.y = entry.data.i32[1];
        width = entry.data.i32[2];
        height = entry.data.i32[3];
        // TODO support more than one metering region
    } else {
        LOGW("invalid control entry count for crop region: %d", entry.count);
    }
    croppingRegion.init(topLeft, width, height, 0);

    if (tagId == ANDROID_CONTROL_AE_REGIONS ||
        tagId == ANDROID_CONTROL_AWB_REGIONS ||
        tagId == ANDROID_CONTROL_AF_REGIONS) {
        entry = settings->find(tagId);
        if (entry.count >= 5) {
            topLeft.x = entry.data.i32[0];
            topLeft.y = entry.data.i32[1];
            bottomRight.x = entry.data.i32[2];
            bottomRight.y = entry.data.i32[3];
            weight = entry.data.i32[4];
            // TODO support more than one metering region
        } else
            LOGW("invalid control entry count %d", entry.count);
    } else {
        LOGW("Unsupported tag ID (%d) is given", tagId);
    }

    meteringWindow->init(topLeft, bottomRight, weight);
    if (meteringWindow->isValid() && croppingRegion.isValid()) {
        // Clip the region to the crop rectangle
        meteringWindow->clip(croppingRegion);
        if (meteringWindow->isValid()){
            LOGI("%s(%d,%d,%d,%d) + cropRegion(%d,%d,%d,%d) --> window:(%d,%d,%d,%d)",
                 tagId == ANDROID_CONTROL_AE_REGIONS ? "AeRegion" : "AfRegion",
                 topLeft.x, topLeft.y, bottomRight.x, bottomRight.y,
                 croppingRegion.left(), croppingRegion.top(), croppingRegion.width(), croppingRegion.height(),
                 meteringWindow->left(), meteringWindow->top(), meteringWindow->width(), meteringWindow->height());
        }
    }
}

void SettingsProcessor::convertCoordinates(CameraWindow *region,
                                   int sensorOutputWidth, int sensorOutputHeight)
{
    int pixel_width = sensorOutputWidth;
    int pixel_height = sensorOutputHeight;
    if(!region)
        return;

    CameraMetadata& staticMeta = RkispDeviceManager::get_static_metadata();
    camera_metadata_entry_t rw_entry;
    rw_entry = staticMeta.find(ANDROID_SENSOR_INFO_PIXEL_ARRAY_SIZE);
    if(rw_entry.count == 2) {
        pixel_width = rw_entry.data.i32[0];
        pixel_height = rw_entry.data.i32[1];
    }

    if (region->isValid()) {
        // map to sensor output coordinate
        if(pixel_height != 0 && pixel_width != 0) {
            *region = region->scale((float)sensorOutputWidth / pixel_width,
                                   (float)sensorOutputHeight / pixel_height);
            LOGI("%s: map to sensor output window:(%d,%d,%d,%d)", __FUNCTION__,
                 region->left(), region->top(), region->width(), region->height());
        }
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
    CameraMetadata& staticMeta = RkispDeviceManager::get_static_metadata();
    AeInputParams* aeInputParams = &aiqInputParams->aeInputParams;
    AeControls *aeCtrl = &aiqInputParams->aaaControls.ae;
    XCamAeParam *aeParams = &aeInputParams->aeParams;

    //# METADATA_Control control.aeLock done
    camera_metadata_ro_entry entry = settings->find(ANDROID_CONTROL_AE_LOCK);
    camera_metadata_entry_t rw_entry;
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

    if (controlMode == ANDROID_CONTROL_MODE_OFF || aeMode == ANDROID_CONTROL_AE_MODE_OFF)
        aeParams->mode = XCAM_AE_MODE_MANUAL;
    else if (controlMode == ANDROID_CONTROL_MODE_AUTO ||
        controlMode == ANDROID_CONTROL_MODE_USE_SCENE_MODE)
        aeParams->mode = XCAM_AE_MODE_AUTO;

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

    CameraWindow aeRegion;
    parseMeteringRegion(settings, ANDROID_CONTROL_AE_REGIONS, &aeRegion);
    memcpy(aiqInputParams->aeInputParams.aeRegion, aeRegion.meteringRectangle(),
           sizeof(aiqInputParams->aeInputParams.aeRegion));
    convertCoordinates(&aeRegion, aiqInputParams->sensorOutputWidth, aiqInputParams->sensorOutputHeight);

    if (aeRegion.isValid()) {
        aeParams->window.x_start = aeRegion.left();
        aeParams->window.y_start = aeRegion.top();
        aeParams->window.x_end = aeRegion.right();
        aeParams->window.y_end = aeRegion.bottom();
        LOGI("@%s %d: window:(%d,%d,%d,%d)", __FUNCTION__, __LINE__,
             aeParams->window.x_start, aeParams->window.y_start, aeParams->window.x_end, aeParams->window.y_end);
    }

    // ******** exposure_coordinate
    rw_entry = staticMeta.find(ANDROID_SENSOR_INFO_EXPOSURE_TIME_RANGE);
    if (rw_entry.count == 2) {
        aeParams->exposure_time_min = rw_entry.data.i64[0] / (1000 * 1000);
        aeParams->exposure_time_max = rw_entry.data.i64[1] / (1000 * 1000);
    }
    /*
     * MANUAL AE CONTROL
     */
    if (aeParams->mode == XCAM_AE_MODE_MANUAL) {
        // ******** manual_exposure_time_us
        //# METADATA_Control sensor.exposureTime done
        entry = settings->find(ANDROID_SENSOR_EXPOSURE_TIME);
        if (entry.count == 1) {
            int64_t timeMicros = entry.data.i64[0] / (1000 * 1000);
            if (timeMicros > 0) {
                /* TODO  need add exposure time limited mechanism*/
                if (timeMicros > aeParams->exposure_time_max) {
                    LOGE("exposure time %" PRId64 " ms is bigger than the max exposure time %" PRId64 " ms",
                        timeMicros, aeParams->exposure_time_max);
                    //return XCAM_RETURN_ERROR_UNKNOWN;
                } else if (timeMicros < aeParams->exposure_time_min) {
                    LOGE("exposure time %" PRId64 " ms is smaller than the min exposure time %" PRId64 " ms",
                        timeMicros, aeParams->exposure_time_min);
                    //return XCAM_RETURN_ERROR_UNKNOWN;
                } else
                    aeParams->manual_exposure_time = timeMicros;
            } else {
                // Don't constrain AIQ.
                aeParams->manual_exposure_time = 0;
            }
        }

        int32_t iso_min, iso_max;
        rw_entry = staticMeta.find(ANDROID_SENSOR_INFO_SENSITIVITY_RANGE);
        if (rw_entry.count == 2) {
            iso_min = rw_entry.data.i32[0];
            iso_max = rw_entry.data.i32[1];
        }
        aeParams->max_analog_gain = (double)iso_max / 100;
        // ******** manual_iso
        //# METADATA_Control sensor.sensitivity done
        entry = settings->find(ANDROID_SENSOR_SENSITIVITY);
        if (entry.count == 1) {
            int32_t iso = entry.data.i32[0];
            aeParams->manual_analog_gain = iso;
            /* TODO  need add iso limited mechanism*/
            if (iso >= iso_min && iso <= iso_max) {
                aeParams->manual_analog_gain = (double)iso;
            } else {
                LOGE("@%s %d: manual iso(%d) is out of range[%d,%d]", __FUNCTION__, __LINE__, iso, iso_min, iso_max);
                aeParams->manual_analog_gain = (double)(iso_min+iso_max) / 2;
            }
            aeParams->manual_analog_gain /= 100;
        }
        // fill target fps range, it needs to be proper in results anyway
        entry = settings->find(ANDROID_CONTROL_AE_TARGET_FPS_RANGE);
        if (entry.count == 2) {
            aeCtrl->aeTargetFpsRange[0] = entry.data.i32[0];
            aeCtrl->aeTargetFpsRange[1] = entry.data.i32[1];
        }
        LOGI("@%s %d: manual iso :%f, exp time:%d", __FUNCTION__, __LINE__, aeParams->manual_analog_gain, aeParams->manual_exposure_time);

    } else {
        /*
         *  AUTO AE CONTROL
         */
        // ******** ev_shift
        //# METADATA_Control control.aeExposureCompensation done
        float stepEV = 1 / 3.0f; //if can't get stepEv, use 1/3 as default
        rw_entry = staticMeta.find(ANDROID_CONTROL_AE_COMPENSATION_STEP);
        if (rw_entry.type == TYPE_RATIONAL || rw_entry.count == 1) {
            const camera_metadata_rational_t* aeCompStep = rw_entry.data.r;
            stepEV = (float)aeCompStep->numerator / aeCompStep->denominator;
        }

        int32_t compensation_min, compensation_max;
        rw_entry = staticMeta.find(ANDROID_CONTROL_AE_COMPENSATION_RANGE);
        if (rw_entry.count == 2) {
            compensation_min = rw_entry.data.i32[0];
            compensation_max = rw_entry.data.i32[1];
        }
        entry = settings->find(ANDROID_CONTROL_AE_EXPOSURE_COMPENSATION);
        if (entry.count == 1) {
            int32_t compensation = entry.data.i32[0];

            if (compensation >= compensation_min && compensation <= compensation_max) {
                aeCtrl->evCompensation = compensation;
            } else {
                LOGE("@%s %d: evCompensation(%d) is out of range[%d,%d]", __FUNCTION__, __LINE__, compensation, compensation_min, compensation_max);
                aeCtrl->evCompensation = 0;
            }

            aeParams->ev_shift = aeCtrl->evCompensation * stepEV;
        } else {
            aeParams->ev_shift = 0.0f;
        }
        aeParams->manual_exposure_time = 0;
        aeParams->manual_analog_gain = 0;

        // ******** target fps
        //# METADATA_Control control.aeTargetFpsRange done
        entry = settings->find(ANDROID_CONTROL_AE_TARGET_FPS_RANGE);
        if (entry.count == 2) {
            aeCtrl->aeTargetFpsRange[0] = entry.data.i32[0];
            aeCtrl->aeTargetFpsRange[1] = entry.data.i32[1];

            int64_t frameDurationMax, frameDurationMin;
            frameDurationMax = 1e9 / aeCtrl->aeTargetFpsRange[0] / (1000 * 1000);
            frameDurationMin = 1e9 / aeCtrl->aeTargetFpsRange[1] / (1000 * 1000);
            if (aeParams->exposure_time_max >  frameDurationMax)
                aeParams->exposure_time_max = frameDurationMax;
            if (aeParams->exposure_time_min <  frameDurationMin)
                aeParams->exposure_time_min = frameDurationMin;
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
    CameraWindow awbRegion;
    parseMeteringRegion(settings, ANDROID_CONTROL_AWB_REGIONS, &awbRegion);
    memcpy(aiqInputParams->awbInputParams.awbRegion, awbRegion.meteringRectangle(),
           sizeof(aiqInputParams->awbInputParams.awbRegion));
    convertCoordinates(&awbRegion, aiqInputParams->sensorOutputWidth, aiqInputParams->sensorOutputHeight);
    if (awbRegion.isValid()) {
        awbCfg->window.x_start = awbRegion.left();
        awbCfg->window.y_start = awbRegion.top();
        awbCfg->window.x_end = awbRegion.right();
        awbCfg->window.y_end = awbRegion.bottom();
    }

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

/**
 * Fills the input parameters for the AF algorithm from the capture request
 * settings.
 * Not all the input parameters will be filled. This class is supposed to
 * be common for all PSL's that use Intel AIQ.
 * There are some input parameters that will be filled by the PSL specific
 * code.
 * The field initialize here are the mandatory ones:
 * frame_use: derived from the control.captureIntent
 * focus_mode: derived from control.afMode
 * focus_range: Focusing range. Only valid when focus_mode is
 *              ia_aiq_af_operation_mode_auto.
 * focus_metering_mode:  Metering mode (multispot, touch).
 * flash_mode:  User setting for flash.
 * trigger_new_search: if new AF search is needed, FALSE otherwise.
 *                     Host is responsible for flag cleaning.
 *
 * There are two mandatory fields that will be filled by the PSL code:
 * lens_position:  Current lens position.
 * lens_movement_start_timestamp: Lens movement start timestamp in us.
 *                          Timestamp is compared against statistics timestamp
 *                          to determine if lens was moving during statistics
 *                          collection.
 *
 * the OPTIONAL fields:
 * manual_focus_parameters: Manual focus parameters (manual lens position,
 *                          manual focusing distance). Used only if focus mode
 *                          'ia_aiq_af_operation_mode_manual' is used. Implies
 *                          that CONTROL_AF_MODE_OFF is used.
 *
 * focus_rect: Not filled here. The reason is that not all platforms implement
 *             touch focus using this rectangle. PSL is responsible for filling
 *             this rectangle or setting it to nullptr.
 *
 * \param[in]  settings capture request metadata settings
 * \param[out] afInputParams struct with the input parameters for the
 *              3A algorithms and also other specific settings parsed
 *              in this method
 *
 * \return OK
 */
XCamReturn
SettingsProcessor::fillAfInputParams(const CameraMetadata *settings,
                                      AiqInputParams *aiqInputParams)
{
    XCamReturn status = XCAM_RETURN_ERROR_UNKNOWN;

    if (settings == nullptr
        || aiqInputParams == nullptr) {
        LOGE("settings = %p, aiqInput = %p", settings, aiqInputParams);
        return XCAM_RETURN_ERROR_UNKNOWN;
    }

    XCamAfParam &afCfg = aiqInputParams->afInputParams.afParams;
    AfControls &afCtrl = aiqInputParams->aaaControls.af;

    uint8_t &afMode = afCtrl.afMode;
    uint8_t &trigger = afCtrl.afTrigger;

    /* frame_use
     *  BEWARE - THIS VALUE WILL NOT WORK WITH AIQ WHICH RUNS PRE-CAPTURE
     *  WITH ia_aiq_frame_use_still, WHEN HAL GETS PREVIEW INTENTS
     *  DURING PRE-CAPTURE!!!
     */
    /* afCfg.frame_use = getFrameUseFromIntent(settings); */

    camera_metadata_ro_entry entry;
    //# METADATA_Control control.afTrigger done
    entry = settings->find(ANDROID_CONTROL_AF_TRIGGER);
    if (entry.count == 1) {
        trigger = entry.data.u8[0];
        if (trigger == ANDROID_CONTROL_AF_TRIGGER_START) {
            afCfg.trigger_new_search = true;
        } else if (trigger == ANDROID_CONTROL_AF_TRIGGER_CANCEL) {
            afCfg.trigger_new_search = false;
        }
        // Otherwise should be IDLE; no effect.
    } else {
        // trigger not present in settigns, default to IDLE
        trigger = ANDROID_CONTROL_AF_TRIGGER_IDLE;
    }

    afMode = ANDROID_CONTROL_AF_MODE_OFF;

    camera_metadata_entry entry_static =
        aiqInputParams->staticMeta->find(ANDROID_CONTROL_AF_AVAILABLE_MODES);

    if (!(entry_static.count == 1 && entry_static.data.u8[0] == ANDROID_CONTROL_AF_MODE_OFF)) {
        entry = settings->find(ANDROID_CONTROL_AF_MODE);
        if (entry.count == 1) {
            afMode = entry.data.u8[0];
        }
    }

    if (aiqInputParams->aaaControls.controlMode == ANDROID_CONTROL_MODE_OFF)
        afMode = ANDROID_CONTROL_AF_MODE_OFF;

    switch (afMode) {
        case ANDROID_CONTROL_AF_MODE_CONTINUOUS_VIDEO:
            afCfg.focus_mode = AF_MODE_CONTINUOUS_VIDEO;
            afCfg.focus_range = AF_RANGE_NORMAL;
            afCfg.focus_metering_mode = AF_METERING_AUTO;
            break;
        case ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE:
            afCfg.focus_mode = AF_MODE_CONTINUOUS_PICTURE;
            afCfg.focus_range = AF_RANGE_NORMAL;
            afCfg.focus_metering_mode = AF_METERING_AUTO;
            break;
        case ANDROID_CONTROL_AF_MODE_MACRO:
            // TODO: can switch to operation_mode_auto,
            // when frame_use is not reset by value from getFrameUseFromIntent();
            afCfg.focus_mode = AF_MODE_MACRO;
            afCfg.focus_range = AF_RANGE_MACRO;
            afCfg.focus_metering_mode = AF_METERING_AUTO;
            break;
        case ANDROID_CONTROL_AF_MODE_EDOF:
            afCfg.focus_mode = AF_MODE_EDOF;
            afCfg.focus_range = AF_RANGE_EXTENDED;
            afCfg.focus_metering_mode = AF_METERING_AUTO;
            break;
        case ANDROID_CONTROL_AF_MODE_OFF:
            // Generally the infinity focus is obtained as 0.0f manual
            afCfg.focus_mode = AF_MODE_EDOF;
            afCfg.focus_range = AF_RANGE_EXTENDED;
            afCfg.focus_metering_mode = AF_METERING_AUTO;
            break;
        case ANDROID_CONTROL_AF_MODE_AUTO:
            // TODO: switch to operation_mode_auto, similar to MACRO AF
            afCfg.focus_mode = AF_MODE_AUTO;
            afCfg.focus_range = AF_RANGE_EXTENDED;
            afCfg.focus_metering_mode = AF_METERING_AUTO;
            break;
        default:
            LOGE("ERROR @%s: Unknown focus mode %d- using auto",
                    __FUNCTION__, afMode);
            afCfg.focus_mode = AF_MODE_AUTO;
            afCfg.focus_range = AF_RANGE_EXTENDED;
            afCfg.focus_metering_mode = AF_METERING_AUTO;
            break;
    }

    /* TODO manual af*/
    /* if (afMode == ANDROID_CONTROL_AF_MODE_OFF) { */
    /*     status = parseFocusDistance(*settings, afCfg); */
    /*     if (status != NO_ERROR) { */
    /*         afCfg.manual_focus_parameters = nullptr; */
    /*         LOGE("Focus distance parsing failed"); */
    /*     } */
    /* } else { */
    /*     afCfg.manual_focus_parameters = nullptr; */
    /* } */

    /* flash mode not support, set default value for aiq af*/
    afCfg.flash_mode = AF_FLASH_MODE_OFF;

    /**
     * AF region parsing
     * we only support one for the time being
     */
    //# METADATA_Control control.afRegions done
    CameraWindow afRegion;
    parseMeteringRegion(settings, ANDROID_CONTROL_AF_REGIONS, &afRegion);
    memcpy(aiqInputParams->afInputParams.afRegion, afRegion.meteringRectangle(),
           sizeof(aiqInputParams->afInputParams.afRegion));
    convertCoordinates(&afRegion, aiqInputParams->sensorOutputWidth, aiqInputParams->sensorOutputHeight);
    if (afRegion.isValid()) {
        afCfg.focus_rect[0].left_hoff = afRegion.left();
        afCfg.focus_rect[0].top_voff = afRegion.top();
        afCfg.focus_rect[0].right_width = afRegion.width();
        afCfg.focus_rect[0].bottom_height = afRegion.height();
    }

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
SettingsProcessor::processAfSettings(const CameraMetadata &settings,
                              AiqInputParams &aiqparams) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = fillAfInputParams(&settings, &aiqparams);

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

    if ((ret = processAfSettings(settings, aiqparams)) != XCAM_RETURN_NO_ERROR)
        return ret;
    return ret;
}
