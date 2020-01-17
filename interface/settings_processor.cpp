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
#include "rkcamera_vendor_tags.h"
#include <base/xcam_log.h>

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

    uint8_t flash_mode = ANDROID_FLASH_MODE_OFF;
    entry = settings->find(ANDROID_FLASH_MODE);
    if (entry.count == 1) {
        flash_mode = entry.data.u8[0];
    }

    // if aemode is *_flash, overide the flash mode of ANDROID_FLASH_MODE
    if (aeMode == ANDROID_CONTROL_AE_MODE_ON_AUTO_FLASH)
        aeParams->flash_mode = AE_FLASH_MODE_AUTO;
    else if (aeMode == ANDROID_CONTROL_AE_MODE_ON_ALWAYS_FLASH)
        aeParams->flash_mode = AE_FLASH_MODE_ON;
    else if (flash_mode  == ANDROID_FLASH_MODE_TORCH)
        aeParams->flash_mode = AE_FLASH_MODE_TORCH;
    else if (flash_mode  == ANDROID_FLASH_MODE_SINGLE)
        aeParams->flash_mode = AE_FLASH_MODE_ON;
    else
        aeParams->flash_mode = AE_FLASH_MODE_OFF;

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

    rw_entry = staticMeta.find(ANDROID_CONTROL_MAX_REGIONS);
    if(rw_entry.count == 3) {
        if(rw_entry.data.i32[0] == 1) {
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
            }
        }
    }

    // ******** exposure_coordinate
    rw_entry = staticMeta.find(ANDROID_SENSOR_INFO_EXPOSURE_TIME_RANGE);
    if (rw_entry.count == 2) {
        aeParams->exposure_time_min = rw_entry.data.i64[0];
        aeParams->exposure_time_max = rw_entry.data.i64[1];
    }

    int32_t iso_min, iso_max;
    rw_entry = staticMeta.find(ANDROID_SENSOR_INFO_SENSITIVITY_RANGE);
    if (rw_entry.count == 2) {
        iso_min = rw_entry.data.i32[0];
        iso_max = rw_entry.data.i32[1];
        aeParams->max_analog_gain = (double)iso_max / 100;
        LOGD("iso_max %f",aeParams->max_analog_gain);
    }
    /*
     * MANUAL AE CONTROL
     */
    if (aeParams->mode == XCAM_AE_MODE_MANUAL) {
        // ******** manual_exposure_time_us
        //# METADATA_Control sensor.exposureTime done
        entry = settings->find(ANDROID_SENSOR_EXPOSURE_TIME);
        if (entry.count == 1) {
            int64_t timens = entry.data.i64[0];
            if (timens > 0) {
                /* TODO  need add exposure time limited mechanism*/
                if (timens > aeParams->exposure_time_max) {
                    LOGE("exposure time %" PRId64 " ms is bigger than the max exposure time %" PRId64 " ms",
                        timens, aeParams->exposure_time_max);
                    //return XCAM_RETURN_ERROR_UNKNOWN;
                } else if (timens < aeParams->exposure_time_min) {
                    LOGE("exposure time %" PRId64 " ms is smaller than the min exposure time %" PRId64 " ms",
                        timens, aeParams->exposure_time_min);
                    //return XCAM_RETURN_ERROR_UNKNOWN;
                } else
                    aeParams->manual_exposure_time = timens;
            } else {
                // Don't constrain AIQ.
                aeParams->manual_exposure_time = 0;
            }
        }

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
            frameDurationMax = 1e9 / aeCtrl->aeTargetFpsRange[0];
            frameDurationMin = 1e9 / aeCtrl->aeTargetFpsRange[1];
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
    CameraMetadata& staticMeta = RkispDeviceManager::get_static_metadata();

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
    camera_metadata_entry_t rw_entry;

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

    rw_entry = staticMeta.find(ANDROID_CONTROL_MAX_REGIONS);
    if(rw_entry.count == 3) {
        if(rw_entry.data.i32[2] == 1) {
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
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
SettingsProcessor::fillBlsInputParams(const CameraMetadata *settings,
                                    AiqInputParams *aiqInputParams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    camera_metadata_ro_entry entry;

    entry = settings->find(RKCAMERA3_PRIVATEDATA_ISP_BLS_SET);
    if(!entry.count){
        aiqInputParams->blsInputParams.updateFlag = 0;
    }else{
        aiqInputParams->blsInputParams.updateFlag = 1;
        aiqInputParams->blsInputParams.enable = entry.data.u8[0];
        aiqInputParams->blsInputParams.mode = entry.data.u8[1];
        aiqInputParams->blsInputParams.winEnable = entry.data.u8[2];
        memcpy(aiqInputParams->blsInputParams.win, &entry.data.u8[3], 16);
        aiqInputParams->blsInputParams.samples = entry.data.u8[19];
        memcpy(&aiqInputParams->blsInputParams.fixedVal, &entry.data.u8[20], 8);
    }
    return ret;
}


XCamReturn
SettingsProcessor::fillLscInputParams(const CameraMetadata *settings,
                                    AiqInputParams *aiqInputParams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    camera_metadata_ro_entry entry;

    entry = settings->find(RKCAMERA3_PRIVATEDATA_ISP_LSC_SET);
    if(!entry.count){
        aiqInputParams->lscInputParams.updateFlag = 0;
    }else{
        aiqInputParams->lscInputParams.updateFlag = 1;
        aiqInputParams->lscInputParams.on = entry.data.u8[0];
        memcpy(aiqInputParams->lscInputParams.LscName, &entry.data.u8[1], HAL_ISP_LSC_NAME_LEN);
        memcpy(&aiqInputParams->lscInputParams.LscSectors, &entry.data.u8[26], 2);
        memcpy(&aiqInputParams->lscInputParams.LscNo, &entry.data.u8[28], 2);
        memcpy(&aiqInputParams->lscInputParams.LscXo, &entry.data.u8[30], 2);
        memcpy(&aiqInputParams->lscInputParams.LscYo, &entry.data.u8[32], 2);
        memcpy(aiqInputParams->lscInputParams.LscXSizeTbl, &entry.data.u8[34], 16);
        memcpy(aiqInputParams->lscInputParams.LscYSizeTbl, &entry.data.u8[50], 16);
        memcpy(aiqInputParams->lscInputParams.LscMatrix, &entry.data.u8[66], 2312);
    }
    return ret;
}

XCamReturn
SettingsProcessor::fillCcmInputParams(const CameraMetadata *settings,
                                    AiqInputParams *aiqInputParams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    camera_metadata_ro_entry entry;

    entry = settings->find(RKCAMERA3_PRIVATEDATA_ISP_CCM_SET);
    if(!entry.count){
        aiqInputParams->ccmInputParams.updateFlag = 0;
    }else{
        aiqInputParams->ccmInputParams.updateFlag = 1;
        aiqInputParams->ccmInputParams.on = entry.data.u8[0];
        memcpy(aiqInputParams->ccmInputParams.name, &entry.data.u8[1], 20);
        memcpy(&aiqInputParams->ccmInputParams.matrix, &entry.data.u8[21], 36);
        memcpy(&aiqInputParams->ccmInputParams.offsets, &entry.data.u8[57], 12);
    }
    return ret;
}

XCamReturn
SettingsProcessor::fillAwbToolInputParams(const CameraMetadata *settings,
                                    AiqInputParams *aiqInputParams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    camera_metadata_ro_entry entry;

    entry = settings->find(RKCAMERA3_PRIVATEDATA_ISP_AWB_SET);
    if(!entry.count){
        aiqInputParams->awbToolInputParams.updateFlag = 0;
    }else{
        aiqInputParams->awbToolInputParams.updateFlag = 1;
        aiqInputParams->awbToolInputParams.on = entry.data.u8[0];
        memcpy(&aiqInputParams->awbToolInputParams.r_gain, &entry.data.u8[1], 4);
        memcpy(&aiqInputParams->awbToolInputParams.gr_gain, &entry.data.u8[5], 4);
        memcpy(&aiqInputParams->awbToolInputParams.gb_gain, &entry.data.u8[9], 4);
        memcpy(&aiqInputParams->awbToolInputParams.b_gain, &entry.data.u8[13], 4);
        aiqInputParams->awbToolInputParams.lock_ill = entry.data.u8[17];
        memcpy(aiqInputParams->awbToolInputParams.ill_name, &entry.data.u8[18], HAL_ISP_ILL_NAME_LEN);
    }
    return ret;
}

XCamReturn
SettingsProcessor::fillAwbWhitePointInputParams(const CameraMetadata *settings,
                                    AiqInputParams *aiqInputParams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    camera_metadata_ro_entry entry;

    entry = settings->find(RKCAMERA3_PRIVATEDATA_ISP_AWB_WP_SET);
    if(!entry.count){
        aiqInputParams->awbWpInputParams.updateFlag = 0;
    }else{
        aiqInputParams->awbWpInputParams.updateFlag = 1;
        const uint8_t *pchr = NULL;
        pchr = &entry.data.u8[0];
        memcpy(&aiqInputParams->awbWpInputParams.win_h_offs, pchr, 2);
        pchr += 2;
        memcpy(&aiqInputParams->awbWpInputParams.win_v_offs, pchr, 2);
        pchr += 2;
        memcpy(&aiqInputParams->awbWpInputParams.win_width, pchr, 2);
        pchr += 2;
        memcpy(&aiqInputParams->awbWpInputParams.win_height, pchr, 2);
        pchr += 2;
        aiqInputParams->awbWpInputParams.awb_mode = *pchr++;
        memcpy(&aiqInputParams->awbWpInputParams.afFade, pchr, HAL_ISP_AWBFADE2PARM_LEN*4);
        pchr += HAL_ISP_AWBFADE2PARM_LEN*4;
        #if 1//awb_v11
        memcpy(&aiqInputParams->awbWpInputParams.afmaxCSum_br, pchr, HAL_ISP_AWBFADE2PARM_LEN*4);
        pchr += HAL_ISP_AWBFADE2PARM_LEN*4;
        memcpy(&aiqInputParams->awbWpInputParams.afmaxCSum_sr, pchr, HAL_ISP_AWBFADE2PARM_LEN*4);
        pchr += HAL_ISP_AWBFADE2PARM_LEN*4;
        memcpy(&aiqInputParams->awbWpInputParams.afminC_br, pchr, HAL_ISP_AWBFADE2PARM_LEN*4);
        pchr += HAL_ISP_AWBFADE2PARM_LEN*4;
        memcpy(&aiqInputParams->awbWpInputParams.afminC_sr, pchr, HAL_ISP_AWBFADE2PARM_LEN*4);
        pchr += HAL_ISP_AWBFADE2PARM_LEN*4;
        memcpy(&aiqInputParams->awbWpInputParams.afMaxY_br, pchr, HAL_ISP_AWBFADE2PARM_LEN*4);
        pchr += HAL_ISP_AWBFADE2PARM_LEN*4;
        memcpy(&aiqInputParams->awbWpInputParams.afMaxY_sr, pchr, HAL_ISP_AWBFADE2PARM_LEN*4);
        pchr += HAL_ISP_AWBFADE2PARM_LEN*4;
        memcpy(&aiqInputParams->awbWpInputParams.afMinY_br, pchr, HAL_ISP_AWBFADE2PARM_LEN*4);
        pchr += HAL_ISP_AWBFADE2PARM_LEN*4;
        memcpy(&aiqInputParams->awbWpInputParams.afMinY_sr, pchr, HAL_ISP_AWBFADE2PARM_LEN*4);
        pchr += HAL_ISP_AWBFADE2PARM_LEN*4;
        #else//awb_v10
        memcpy(&aiqInputParams->awbWpInputParams.afCbMinRegionMax, pchr, HAL_ISP_AWBFADE2PARM_LEN*4);
        pchr += HAL_ISP_AWBFADE2PARM_LEN*4;
        memcpy(&aiqInputParams->awbWpInputParams.afCrMinRegionMax, pchr, HAL_ISP_AWBFADE2PARM_LEN*4);
        pchr += HAL_ISP_AWBFADE2PARM_LEN*4;
        memcpy(&aiqInputParams->awbWpInputParams.afMaxCSumRegionMax, pchr, HAL_ISP_AWBFADE2PARM_LEN*4);
        pchr += HAL_ISP_AWBFADE2PARM_LEN*4;
        memcpy(&aiqInputParams->awbWpInputParams.afCbMinRegionMin, pchr, HAL_ISP_AWBFADE2PARM_LEN*4);
        pchr += HAL_ISP_AWBFADE2PARM_LEN*4;
        memcpy(&aiqInputParams->awbWpInputParams.afCrMinRegionMin, pchr, HAL_ISP_AWBFADE2PARM_LEN*4);
        pchr += HAL_ISP_AWBFADE2PARM_LEN*4;
        memcpy(&aiqInputParams->awbWpInputParams.afMaxCSumRegionMin, pchr, HAL_ISP_AWBFADE2PARM_LEN*4);
        pchr += HAL_ISP_AWBFADE2PARM_LEN*4;
        memcpy(&aiqInputParams->awbWpInputParams.afMinCRegionMax, pchr, HAL_ISP_AWBFADE2PARM_LEN*4);
        pchr += HAL_ISP_AWBFADE2PARM_LEN*4;
        memcpy(&aiqInputParams->awbWpInputParams.afMinCRegionMin, pchr, HAL_ISP_AWBFADE2PARM_LEN*4);
        pchr += HAL_ISP_AWBFADE2PARM_LEN*4;
        memcpy(&aiqInputParams->awbWpInputParams.afMaxYRegionMax, pchr, HAL_ISP_AWBFADE2PARM_LEN*4);
        pchr += HAL_ISP_AWBFADE2PARM_LEN*4;
        memcpy(&aiqInputParams->awbWpInputParams.afMaxYRegionMin, pchr, HAL_ISP_AWBFADE2PARM_LEN*4);
        pchr += HAL_ISP_AWBFADE2PARM_LEN*4;
        memcpy(&aiqInputParams->awbWpInputParams.afMinYMaxGRegionMax, pchr, HAL_ISP_AWBFADE2PARM_LEN*4);
        pchr += HAL_ISP_AWBFADE2PARM_LEN*4;
        memcpy(&aiqInputParams->awbWpInputParams.afMinYMaxGRegionMin, pchr, HAL_ISP_AWBFADE2PARM_LEN*4);
        pchr += HAL_ISP_AWBFADE2PARM_LEN*4;
        #endif
        memcpy(&aiqInputParams->awbWpInputParams.afRefCb, pchr, HAL_ISP_AWBFADE2PARM_LEN*4);
        pchr += HAL_ISP_AWBFADE2PARM_LEN*4;
        memcpy(&aiqInputParams->awbWpInputParams.afRefCr, pchr, HAL_ISP_AWBFADE2PARM_LEN*4);
        pchr += HAL_ISP_AWBFADE2PARM_LEN*4;
        memcpy(&aiqInputParams->awbWpInputParams.fRgProjIndoorMin, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->awbWpInputParams.fRgProjOutdoorMin, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->awbWpInputParams.fRgProjMax, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->awbWpInputParams.fRgProjMaxSky, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->awbWpInputParams.fRgProjALimit, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->awbWpInputParams.fRgProjAWeight, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->awbWpInputParams.fRgProjYellowLimitEnable, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->awbWpInputParams.fRgProjYellowLimit, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->awbWpInputParams.fRgProjIllToCwfEnable, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->awbWpInputParams.fRgProjIllToCwf, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->awbWpInputParams.fRgProjIllToCwfWeight, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->awbWpInputParams.fRegionSize, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->awbWpInputParams.fRegionSizeInc, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->awbWpInputParams.fRegionSizeDec, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->awbWpInputParams.cnt, pchr, 4);
        pchr += 4;
        aiqInputParams->awbWpInputParams.mean_y = *pchr++;
        aiqInputParams->awbWpInputParams.mean_cb = *pchr++;
        aiqInputParams->awbWpInputParams.mean_cr = *pchr++;
        memcpy(&aiqInputParams->awbWpInputParams.mean_r, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->awbWpInputParams.mean_b, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->awbWpInputParams.mean_g, pchr, 4);
    }
    return ret;
}

XCamReturn
SettingsProcessor::fillAwbCurvInputParams(const CameraMetadata *settings,
                                    AiqInputParams *aiqInputParams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    camera_metadata_ro_entry entry;

    entry = settings->find(RKCAMERA3_PRIVATEDATA_ISP_AWB_CURV_SET);
    if(!entry.count){
        aiqInputParams->awbCurveInputParams.updateFlag = 0;
    }else{
        const uint8_t *pchr = NULL;
        pchr = &entry.data.u8[0];
        aiqInputParams->awbCurveInputParams.updateFlag = 1;
        memcpy(&aiqInputParams->awbCurveInputParams.f_N0_Rg, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->awbCurveInputParams.f_N0_Bg, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->awbCurveInputParams.f_d, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->awbCurveInputParams.Kfactor, pchr, 4);
        pchr += 4;
        memcpy(aiqInputParams->awbCurveInputParams.afRg1, pchr, HAL_ISP_AWBCLIPPARM_LEN*4);
        pchr += HAL_ISP_AWBCLIPPARM_LEN*4;
        memcpy(aiqInputParams->awbCurveInputParams.afMaxDist1, pchr, HAL_ISP_AWBCLIPPARM_LEN*4);
        pchr += HAL_ISP_AWBCLIPPARM_LEN*4;
        memcpy(aiqInputParams->awbCurveInputParams.afRg2, pchr, HAL_ISP_AWBCLIPPARM_LEN*4);
        pchr += HAL_ISP_AWBCLIPPARM_LEN*4;
        memcpy(aiqInputParams->awbCurveInputParams.afMaxDist2, pchr, HAL_ISP_AWBCLIPPARM_LEN*4);
        pchr += HAL_ISP_AWBCLIPPARM_LEN*4;
        memcpy(aiqInputParams->awbCurveInputParams.afGlobalFade1, pchr, HAL_ISP_AWBCLIPPARM_LEN*4);
        pchr += HAL_ISP_AWBCLIPPARM_LEN*4;
        memcpy(aiqInputParams->awbCurveInputParams.afGlobalGainDistance1, pchr, HAL_ISP_AWBCLIPPARM_LEN*4);
        pchr += HAL_ISP_AWBCLIPPARM_LEN*4;
        memcpy(aiqInputParams->awbCurveInputParams.afGlobalFade2, pchr, HAL_ISP_AWBCLIPPARM_LEN*4);
        pchr += HAL_ISP_AWBCLIPPARM_LEN*4;
        memcpy(aiqInputParams->awbCurveInputParams.afGlobalGainDistance2, pchr, HAL_ISP_AWBCLIPPARM_LEN*4);
    }
    return ret;
}

XCamReturn
SettingsProcessor::fillAwbRefGainInputParams(const CameraMetadata *settings,
                                    AiqInputParams *aiqInputParams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    camera_metadata_ro_entry entry;

    entry = settings->find(RKCAMERA3_PRIVATEDATA_ISP_AWB_REFGAIN_SET);
    if(!entry.count){
        aiqInputParams->awbRefGainInputParams.updateFlag = 0;
    }else{
        const uint8_t *pchr = NULL;
        pchr = &entry.data.u8[0];
        aiqInputParams->awbRefGainInputParams.updateFlag = 1;
        memcpy(&aiqInputParams->awbRefGainInputParams.ill_name, pchr, HAL_ISP_ILL_NAME_LEN);
        pchr += 4;
        memcpy(&aiqInputParams->awbRefGainInputParams.refRGain, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->awbRefGainInputParams.refGrGain, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->awbRefGainInputParams.refGbGain, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->awbRefGainInputParams.refBGain, pchr, 4);
    }
    return ret;
}

XCamReturn
SettingsProcessor::fillGocInputParams(const CameraMetadata *settings,
                                    AiqInputParams *aiqInputParams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    camera_metadata_ro_entry entry;

    entry = settings->find(RKCAMERA3_PRIVATEDATA_ISP_GOC_SET);
    if(!entry.count){
        aiqInputParams->gocInputParams.updateFlag = 0;
    }else{
        const uint8_t *pchr = NULL;
        pchr = &entry.data.u8[0];
        aiqInputParams->gocInputParams.updateFlag = 1;
        aiqInputParams->gocInputParams.on = *pchr;
        pchr++;
        memcpy(&aiqInputParams->gocInputParams.scene_name, pchr, sizeof(aiqInputParams->gocInputParams.scene_name));
        pchr += sizeof(aiqInputParams->gocInputParams.scene_name);
        aiqInputParams->gocInputParams.wdr_status = *pchr;
        pchr++;
        aiqInputParams->gocInputParams.cfg_mode = *pchr;
        pchr++;
        memcpy(&aiqInputParams->gocInputParams.gamma_y, pchr, sizeof(aiqInputParams->gocInputParams.gamma_y));
    }
    return ret;
}

XCamReturn
SettingsProcessor::fillCprocInputParams(const CameraMetadata *settings,
                                    AiqInputParams *aiqInputParams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    camera_metadata_ro_entry entry;

    entry = settings->find(RKCAMERA3_PRIVATEDATA_ISP_CPROC_SET);
    if(!entry.count){
        aiqInputParams->cprocInputParams.updateFlag = 0;
    }else{
        const uint8_t *pchr = NULL;
        pchr = &entry.data.u8[0];
        aiqInputParams->cprocInputParams.updateFlag = 1;
        aiqInputParams->cprocInputParams.on = *pchr;
        pchr++;
        aiqInputParams->cprocInputParams.mode = *pchr;
        pchr++;
        memcpy(&aiqInputParams->cprocInputParams.cproc_contrast, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->cprocInputParams.cproc_hue, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->cprocInputParams.cproc_saturation, pchr,4);
        pchr += 4;
        aiqInputParams->cprocInputParams.cproc_brightness = *pchr;
        LOGV("%f,%f,%f,%d",aiqInputParams->cprocInputParams.cproc_contrast,aiqInputParams->cprocInputParams.cproc_hue,aiqInputParams->cprocInputParams.cproc_saturation,
        aiqInputParams->cprocInputParams.cproc_brightness);
    }
    return ret;
}

XCamReturn
SettingsProcessor::fillAdpfInputParams(const CameraMetadata *settings,
                                    AiqInputParams *aiqInputParams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    camera_metadata_ro_entry entry;

    entry = settings->find(RKCAMERA3_PRIVATEDATA_ISP_DPF_SET);
    if(!entry.count){
        aiqInputParams->adpfInputParams.updateFlag = 0;
    }else{
        const uint8_t *pchr = NULL;
        pchr = &entry.data.u8[0];
        aiqInputParams->adpfInputParams.updateFlag = 1;
        memcpy(aiqInputParams->adpfInputParams.dpf_name, pchr, 20);
        pchr += 20;
        aiqInputParams->adpfInputParams.dpf_enable = *pchr;
        pchr++;
        aiqInputParams->adpfInputParams.nll_segment = *pchr;
        pchr++;
        memcpy(aiqInputParams->adpfInputParams.nll_coeff, pchr, 17*2);
        pchr += 34;
        memcpy(&aiqInputParams->adpfInputParams.sigma_green, pchr, 2);
        pchr += 2;
        memcpy(&aiqInputParams->adpfInputParams.sigma_redblue, pchr, 2);
        pchr += 2;
        memcpy(&aiqInputParams->adpfInputParams.gradient, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->adpfInputParams.offset, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->adpfInputParams.fRed, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->adpfInputParams.fGreenR, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->adpfInputParams.fGreenB, pchr, 4);
        pchr += 4;
        memcpy(&aiqInputParams->adpfInputParams.fBlue, pchr, 4);
    }
    return ret;
}

XCamReturn
SettingsProcessor::fillFltInputParams(const CameraMetadata *settings,
                                    AiqInputParams *aiqInputParams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    camera_metadata_ro_entry entry;

    entry = settings->find(RKCAMERA3_PRIVATEDATA_ISP_FLT_SET);
    if(!entry.count){
        aiqInputParams->fltInputParams.updateFlag = 0;
    }else{
        const uint8_t *pchr = NULL;
        pchr = &entry.data.u8[0];
        aiqInputParams->fltInputParams.updateFlag = 1;
        memcpy(aiqInputParams->fltInputParams.filter_name, pchr, 20);
        pchr += 20;
        aiqInputParams->fltInputParams.scene_mode = *pchr;
        pchr++;
        aiqInputParams->fltInputParams.filter_enable = *pchr;
        LOGV("%d",aiqInputParams->fltInputParams.filter_enable);
        pchr++;
        memcpy(aiqInputParams->fltInputParams.denoise_gain, pchr, 5);
        LOGV("%d,%d,%d,%d,%d",aiqInputParams->fltInputParams.denoise_gain[0],aiqInputParams->fltInputParams.denoise_gain[1],aiqInputParams->fltInputParams.denoise_gain[2],
                              aiqInputParams->fltInputParams.denoise_gain[3],aiqInputParams->fltInputParams.denoise_gain[4]);
        pchr += 5;
        memcpy(aiqInputParams->fltInputParams.denoise_level, pchr, 5);
        pchr += 5;
        memcpy(aiqInputParams->fltInputParams.sharp_gain, pchr, 5);
        pchr += 5;
        memcpy(aiqInputParams->fltInputParams.sharp_level, pchr, 5);
        pchr += 5;
        aiqInputParams->fltInputParams.level_conf_enable = *pchr;
        pchr++;
        aiqInputParams->fltInputParams.level = *pchr;
        LOGV("en:%d,level:%d",aiqInputParams->fltInputParams.level_conf_enable,aiqInputParams->fltInputParams.level);
        pchr++;
        memcpy(&aiqInputParams->fltInputParams.level_conf, pchr, 39);
        LOGV("%d %d %d",aiqInputParams->fltInputParams.level_conf.grn_stage1,aiqInputParams->fltInputParams.level_conf.fac_sh1,aiqInputParams->fltInputParams.level_conf.fac_bl1);
    }
    return ret;
}

XCamReturn
SettingsProcessor::restartInputParams(const CameraMetadata *settings, AiqInputParams *aiqInputParams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    camera_metadata_ro_entry entry;

    entry = settings->find(RKCAMERA3_PRIVATEDATA_ISP_RESTART);
    if(!entry.count){
        aiqInputParams->restartInputParams.updateFlag = 0;
    }else{
        aiqInputParams->restartInputParams.updateFlag = 1;
        aiqInputParams->restartInputParams.on = entry.data.u8[0];
    }
    return ret;
}

XCamReturn
SettingsProcessor::tuningFlagInputParams(const CameraMetadata *settings, AiqInputParams *aiqInputParams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    camera_metadata_ro_entry entry;

    entry = settings->find(RKCAMERA3_PRIVATEDATA_TUNING_FLAG);
    if(!entry.count){
        aiqInputParams->tuningFlag = 0;
    }else{
         aiqInputParams->tuningFlag = entry.data.u8[0];
    }
    return ret;
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
SettingsProcessor::processTuningParamsSettings(const CameraMetadata &settings,
                            AiqInputParams &aiqparams) {
  XCamReturn ret = XCAM_RETURN_NO_ERROR;

  if((ret = fillBlsInputParams(&settings, &aiqparams)) != XCAM_RETURN_NO_ERROR)
    return ret;
  if((ret = fillLscInputParams(&settings, &aiqparams)) != XCAM_RETURN_NO_ERROR)
    return ret;
  if((ret = fillCcmInputParams(&settings, &aiqparams)) != XCAM_RETURN_NO_ERROR)
    return ret;
  if((ret = fillAwbToolInputParams(&settings, &aiqparams)) != XCAM_RETURN_NO_ERROR)
    return ret;
  if((ret = fillAwbWhitePointInputParams(&settings, &aiqparams)) != XCAM_RETURN_NO_ERROR)
    return ret;
  if((ret = fillAwbCurvInputParams(&settings, &aiqparams)) != XCAM_RETURN_NO_ERROR)
    return ret;
  if((ret = fillAwbRefGainInputParams(&settings, &aiqparams)) != XCAM_RETURN_NO_ERROR)
    return ret;
  if((ret = fillGocInputParams(&settings, &aiqparams)) != XCAM_RETURN_NO_ERROR)
    return ret;
  if((ret = fillCprocInputParams(&settings, &aiqparams)) != XCAM_RETURN_NO_ERROR)
    return ret;
  if((ret = fillAdpfInputParams(&settings, &aiqparams)) != XCAM_RETURN_NO_ERROR)
    return ret;
  if((ret = fillFltInputParams(&settings, &aiqparams)) != XCAM_RETURN_NO_ERROR)
    return ret;
  if((ret = restartInputParams(&settings, &aiqparams)) != XCAM_RETURN_NO_ERROR)
    return ret;
  if((ret = tuningFlagInputParams(&settings, &aiqparams)) != XCAM_RETURN_NO_ERROR)
    return ret;

  return ret;
}

XCamReturn
SettingsProcessor::processRequestSettings(const CameraMetadata &settings,
                             AiqInputParams &aiqparams) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // get use case
    aiqparams.frameUseCase = AIQ_FRAME_USECASE_PREVIEW;
    camera_metadata_ro_entry entry = settings.find(ANDROID_CONTROL_CAPTURE_INTENT);
    camera_metadata_entry_t rw_entry;
    if (entry.count == 1) {
        switch (entry.data.u8[0]) {
            case ANDROID_CONTROL_CAPTURE_INTENT_PREVIEW:
                aiqparams.frameUseCase = AIQ_FRAME_USECASE_PREVIEW;
                break;
            case ANDROID_CONTROL_CAPTURE_INTENT_STILL_CAPTURE:
                aiqparams.frameUseCase = AIQ_FRAME_USECASE_STILL_CAPTURE;
                break;
            case ANDROID_CONTROL_CAPTURE_INTENT_VIDEO_RECORD:
                aiqparams.frameUseCase = AIQ_FRAME_USECASE_VIDEO_RECORDING;
                break;
            default :
                break;
        }
    }

    entry = settings.find(RKCAMERA3_PRIVATEDATA_STILLCAP_SYNC_CMD);
    if (entry.count == 1) {
        switch (entry.data.u8[0]) {
            case RKCAMERA3_PRIVATEDATA_STILLCAP_SYNC_CMD_SYNCSTART:
                aiqparams.stillCapSyncCmd = RKCAMERA3_PRIVATEDATA_STILLCAP_SYNC_CMD_SYNCSTART;
                break;
            case RKCAMERA3_PRIVATEDATA_STILLCAP_SYNC_CMD_SYNCEND:
                aiqparams.stillCapSyncCmd = RKCAMERA3_PRIVATEDATA_STILLCAP_SYNC_CMD_SYNCEND;
                break;
            default :
                aiqparams.stillCapSyncCmd = 0;
                break;
        }
    } else
        aiqparams.stillCapSyncCmd = 0;

    if ((ret = processAeSettings(settings, aiqparams)) != XCAM_RETURN_NO_ERROR)
        return ret;

    if ((ret = processAwbSettings(settings, aiqparams)) != XCAM_RETURN_NO_ERROR)
        return ret;

    if ((ret = processAfSettings(settings, aiqparams)) != XCAM_RETURN_NO_ERROR)
        return ret;
    if ((ret = processTuningParamsSettings(settings, aiqparams)) != XCAM_RETURN_NO_ERROR)
        return ret;
    return ret;
}
