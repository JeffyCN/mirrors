/*
 * aiq3a_util.cpp - aiq 3a utility:
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
 * Author: Shincy Tu <shincy.tu@intel.com>
 */

#include "aiq3a_utils.h"
#include "x3a_isp_config.h"
#include "x3a_statistics_queue.h"
#include "cam_types.h"

namespace XCam {

bool
translate_3a_stats (XCam3AStats *from, struct cifisp_stat_buffer *to)
{
    XCAM_ASSERT (from);
    XCAM_ASSERT (to);

    XCamGridStat *standard_data = from->stats;

    for (uint32_t i = 0; i < 5; ++i) {
        for (uint32_t j = 0; j < 5; ++j) {
            to->params.ae.exp_mean[i * 5 + j] = standard_data[i * 5 + j].avg_y;
        }
    }
#if RKISP
    to->params.awb.awb_mean[0].mean_y_or_g = standard_data[0].mean_y_or_g;
    to->params.awb.awb_mean[0].mean_cr_or_r = standard_data[0].mean_cr_or_r;
    to->params.awb.awb_mean[0].mean_cb_or_b = standard_data[0].mean_cb_or_b;
#else
    to->params.awb.awb_mean[0].mean_r = standard_data[0].mean_cr_or_r;
    to->params.awb.awb_mean[0].mean_g = standard_data[0].mean_y_or_g;
    to->params.awb.awb_mean[0].mean_b = standard_data[0].mean_cb_or_b;
    to->params.awb.awb_mean[0].mean_y = standard_data[0].mean_y_or_g;
    to->params.awb.awb_mean[0].mean_cb = standard_data[0].mean_cb_or_b;
    to->params.awb.awb_mean[0].mean_cr = standard_data[0].mean_cr_or_r;
    to->params.awb.awb_mean[0].cnt = standard_data[0].valid_wb_count;
#endif

    uint32_t hist_bins = 16;
    uint32_t *hist_y = from->hist_y;

    for (uint32_t i = 0; i < hist_bins; i++) {
        to->params.hist.hist_bins[i] = hist_y[i];
    }

    return true;
}

static void
matrix_3x3_mutiply (double *dest, const double *src1, const double *src2)
{
    dest[0] = src1[0] * src2[0] + src1[1] * src2[3] + src1[2] * src2[6];
    dest[1] = src1[0] * src2[1] + src1[1] * src2[4] + src1[2] * src2[7];
    dest[2] = src1[0] * src2[2] + src1[1] * src2[5] + src1[2] * src2[8];

    dest[3] = src1[3] * src2[0] + src1[4] * src2[3] + src1[5] * src2[6];
    dest[4] = src1[3] * src2[1] + src1[4] * src2[4] + src1[5] * src2[7];
    dest[5] = src1[3] * src2[2] + src1[4] * src2[5] + src1[5] * src2[8];

    dest[6] = src1[6] * src2[0] + src1[7] * src2[3] + src1[8] * src2[6];
    dest[7] = src1[6] * src2[1] + src1[7] * src2[4] + src1[8] * src2[7];
    dest[8] = src1[6] * src2[2] + src1[7] * src2[5] + src1[8] * src2[8];
}

static uint32_t
translate_rkisp_parameters (
    const struct rkisp_parameters &rkisp_params,
    XCam3aResultHead *results[], uint32_t max_count)
{
    uint32_t result_count = 0;
    double coefficient = 0.0;

    XCAM_ASSERT (result_count < max_count);
    XCam3aResultAll *all = xcam_malloc0_type (XCam3aResultAll);
    all->head.type = XCAM_3A_RESULT_ALL;
    all->head.process_type = XCAM_IMAGE_PROCESS_ALWAYS;
    all->head.version = xcam_version ();

    all->active_configs = rkisp_params.active_configs;
    all->dpcc_config = rkisp_params.dpcc_config;
    all->bls_config = rkisp_params.bls_config;
    all->sdg_config = rkisp_params.sdg_config;
    all->hst_config = rkisp_params.hst_config;
    all->lsc_config = rkisp_params.lsc_config;
    all->awb_gain_config = rkisp_params.awb_gain_config;
    all->awb_meas_config = rkisp_params.awb_meas_config;
    all->flt_config = rkisp_params.flt_config;
    all->bdm_config = rkisp_params.bdm_config;
    all->ctk_config = rkisp_params.ctk_config;
    all->goc_config = rkisp_params.goc_config;
    all->cproc_config = rkisp_params.cproc_config;
    all->aec_config = rkisp_params.aec_config;
    all->afc_config = rkisp_params.afc_config;
    all->ie_config = rkisp_params.ie_config;
    all->dpf_config = rkisp_params.dpf_config;
    all->dpf_strength_config = rkisp_params.dpf_strength_config;
    all->aec_config = rkisp_params.aec_config;
    all->flt_denoise_level= rkisp_params.flt_denoise_level;
    all->flt_sharp_level= rkisp_params.flt_sharp_level;

    for (int i=0; i < HAL_ISP_MODULE_MAX_ID_ID + 1; i++) {
        all->enabled[i] = rkisp_params.enabled[i];
    }
    results[result_count++] = (XCam3aResultHead*)all;


#if 0
    /* Translation for white balance */
    XCAM_ASSERT (result_count < max_count);
    if (rkisp_params.wb_config) {
        XCam3aResultWhiteBalance *wb = xcam_malloc0_type (XCam3aResultWhiteBalance);
        XCAM_ASSERT (wb);
        wb->head.type = XCAM_3A_RESULT_WHITE_BALANCE;
        wb->head.process_type = XCAM_IMAGE_PROCESS_ALWAYS;
        wb->head.version = xcam_version ();
        coefficient = pow (2, (16 - rkisp_params.wb_config->integer_bits));
        wb->r_gain = rkisp_params.wb_config->r / coefficient;
        wb->gr_gain = rkisp_params.wb_config->gr / coefficient;
        wb->gb_gain = rkisp_params.wb_config->gb / coefficient;
        wb->b_gain = rkisp_params.wb_config->b / coefficient;
        results[result_count++] = (XCam3aResultHead*)wb;
    }

    /* Translation for black level correction */
    XCAM_ASSERT (result_count < max_count);
    if (rkisp_params.ob_config) {
        XCam3aResultBlackLevel *blc = xcam_malloc0_type (XCam3aResultBlackLevel);
        XCAM_ASSERT (blc);
        blc->head.type =    XCAM_3A_RESULT_BLACK_LEVEL;
        blc->head.process_type = XCAM_IMAGE_PROCESS_ALWAYS;
        blc->head.version = xcam_version ();
        if (rkisp_params.ob_config->mode == rkisp_ob_mode_fixed) {
            blc->r_level = rkisp_params.ob_config->level_r / (double)65536;
            blc->gr_level = rkisp_params.ob_config->level_gr / (double)65536;
            blc->gb_level = rkisp_params.ob_config->level_gb / (double)65536;
            blc->b_level = rkisp_params.ob_config->level_b / (double)65536;
        }
        results[result_count++] = (XCam3aResultHead*)blc;
    }

    /* Translation for color correction */
    XCAM_ASSERT (result_count < max_count);
    if (rkisp_params.yuv2rgb_cc_config) {
        static const double rgb2yuv_matrix [XCAM_COLOR_MATRIX_SIZE] = {
            0.299, 0.587, 0.114,
            -0.14713, -0.28886, 0.436,
            0.615, -0.51499, -0.10001
        };
        static const double r_ycgco_matrix [XCAM_COLOR_MATRIX_SIZE] = {
            0.25, 0.5, 0.25,
            -0.25, 0.5, -0.25,
            0.5, 0, -0.5
        };

        double tmp_matrix [XCAM_COLOR_MATRIX_SIZE] = {0.0};
        double cc_matrix [XCAM_COLOR_MATRIX_SIZE] = {0.0};
        XCam3aResultColorMatrix *cm = xcam_malloc0_type (XCam3aResultColorMatrix);
        XCAM_ASSERT (cm);
        cm->head.type = XCAM_3A_RESULT_RGB2YUV_MATRIX;
        cm->head.process_type = XCAM_IMAGE_PROCESS_ALWAYS;
        cm->head.version = xcam_version ();
        coefficient = pow (2, rkisp_params.yuv2rgb_cc_config->fraction_bits);
        for (int i = 0; i < XCAM_COLOR_MATRIX_SIZE; i++) {
            tmp_matrix [i] = rkisp_params.yuv2rgb_cc_config->matrix [i] / coefficient;
        }
        matrix_3x3_mutiply (cc_matrix, tmp_matrix, r_ycgco_matrix);
        matrix_3x3_mutiply (cm->matrix, rgb2yuv_matrix, cc_matrix);
        //results = yuv2rgb_matrix * tmp_matrix * r_ycgco_matrix
        results[result_count++] = (XCam3aResultHead*)cm;
    }

    /* Translation for gamma table */
    XCAM_ASSERT (result_count < max_count);
    if (rkisp_params.g_gamma_table) {
        XCam3aResultGammaTable *gt = xcam_malloc0_type (XCam3aResultGammaTable);
        XCAM_ASSERT (gt);
        gt->head.type = XCAM_3A_RESULT_G_GAMMA;
        gt->head.process_type = XCAM_IMAGE_PROCESS_ALWAYS;
        gt->head.version = xcam_version ();
        for (int i = 0; i < XCAM_GAMMA_TABLE_SIZE; i++) {
            gt->table[i] = (double)rkisp_params.g_gamma_table->data.vamem_2[i] / 16;
        }
        results[result_count++] = (XCam3aResultHead*)gt;
    }

    /* Translation for macc matrix table */
    XCAM_ASSERT (result_count < max_count);
    if (rkisp_params.macc_config) {
        XCam3aResultMaccMatrix *macc = xcam_malloc0_type (XCam3aResultMaccMatrix);
        XCAM_ASSERT (macc);
        macc->head.type = XCAM_3A_RESULT_MACC;
        macc->head.process_type = XCAM_IMAGE_PROCESS_ALWAYS;
        macc->head.version = xcam_version ();
        coefficient = pow (2, (13 - rkisp_params.macc_config->color_effect));
        for (int i = 0; i < XCAM_CHROMA_AXIS_SIZE * XCAM_CHROMA_MATRIX_SIZE; i++) {
            macc->table[i] = (double)rkisp_params.macc_table->data[i] / coefficient;
        }
        results[result_count++] = (XCam3aResultHead*)macc;
    }

    /* Translation for defect pixel correction */
    XCAM_ASSERT (result_count < max_count);
    if (rkisp_params.dp_config) {
        XCam3aResultDefectPixel *dpc = xcam_malloc0_type (XCam3aResultDefectPixel);
        XCAM_ASSERT (dpc);
        dpc->head.type = XCAM_3A_RESULT_DEFECT_PIXEL_CORRECTION;
        dpc->head.process_type = XCAM_IMAGE_PROCESS_ALWAYS;
        dpc->head.version = xcam_version ();
        coefficient = pow (2, 16);
        dpc->gr_threshold = rkisp_params.dp_config->threshold / coefficient;
        dpc->r_threshold = rkisp_params.dp_config->threshold / coefficient;
        dpc->b_threshold = rkisp_params.dp_config->threshold / coefficient;
        dpc->gb_threshold = rkisp_params.dp_config->threshold / coefficient;
        results[result_count++] = (XCam3aResultHead*)dpc;
    }

    /* OCL has defined BNR config, no need to translate ISP BNR config */
#if 0
    /* Translation for bnr config */
    XCAM_ASSERT (result_count < max_count);
    if (rkisp_params.nr_config) {
        XCam3aResultBayerNoiseReduction *bnr = xcam_malloc0_type (XCam3aResultBayerNoiseReduction);
        XCAM_ASSERT (bnr);
        bnr->head.type = XCAM_3A_RESULT_BAYER_NOISE_REDUCTION;
        bnr->head.process_type = XCAM_IMAGE_PROCESS_ALWAYS;
        bnr->head.version = xcam_version ();
        bnr->bnr_gain = (double)rkisp_params.nr_config->bnr_gain / pow(2, 16);
        bnr->direction = (double)rkisp_params.nr_config->direction / pow(2, 16);
        results[result_count++] = (XCam3aResultHead*)bnr;
    }
#endif
#endif
    return result_count;
}

uint32_t
translate_3a_results_to_xcam (X3aResultList &list,
                              XCam3aResultHead *results[], uint32_t max_count)
{
    uint32_t result_count = 0;
    for (X3aResultList::iterator iter = list.begin (); iter != list.end (); ++iter) {
        SmartPtr<X3aResult> &isp_result = *iter;

        switch (isp_result->get_type()) {
        case X3aIspConfig::IspExposureParameters: {
            SmartPtr<X3aIspExposureResult> isp_exposure =
                isp_result.dynamic_cast_ptr<X3aIspExposureResult> ();
            XCAM_ASSERT (isp_exposure.ptr ());
            const XCam3aResultExposure &exposure = isp_exposure->get_standard_result ();
            XCam3aResultExposure *new_exposure = xcam_malloc0_type (XCam3aResultExposure);
            XCAM_ASSERT (new_exposure);
            *new_exposure = exposure;
            new_exposure->head.type = XCAM_3A_RESULT_EXPOSURE;
            new_exposure->head.process_type = XCAM_IMAGE_PROCESS_ALWAYS;
            new_exposure->head.version = xcam_version ();
            results[result_count++] = (XCam3aResultHead*)new_exposure;
            break;
        }
        case X3aIspConfig::IspAllParameters: {
            SmartPtr<X3aAtomIspParametersResult> isp_3a_all =
                isp_result.dynamic_cast_ptr<X3aAtomIspParametersResult> ();
            XCAM_ASSERT (isp_3a_all.ptr ());
            const struct rkisp_parameters &rkisp_params = isp_3a_all->get_isp_config ();
            XCAM_LOG_ERROR ("translate active_configs: %x", rkisp_params.active_configs);
            result_count += translate_rkisp_parameters (rkisp_params, &results[result_count], max_count - result_count);
            break;
        }
        case XCAM_3A_RESULT_BRIGHTNESS: {
            SmartPtr<X3aBrightnessResult> xcam_brightness =
                isp_result.dynamic_cast_ptr<X3aBrightnessResult>();
            const XCam3aResultBrightness &brightness = xcam_brightness->get_standard_result();
            XCam3aResultBrightness *new_brightness = xcam_malloc0_type(XCam3aResultBrightness);
            XCAM_ASSERT (new_brightness);
            *new_brightness = brightness;
            results[result_count++] = (XCam3aResultHead*)new_brightness;
            break;
        }
        case XCAM_3A_RESULT_3D_NOISE_REDUCTION:
        case XCAM_3A_RESULT_TEMPORAL_NOISE_REDUCTION_YUV:
        {
            SmartPtr<X3aTemporalNoiseReduction> xcam_tnr =
                isp_result.dynamic_cast_ptr<X3aTemporalNoiseReduction> ();
            const XCam3aResultTemporalNoiseReduction &tnr = xcam_tnr->get_standard_result();
            XCam3aResultTemporalNoiseReduction *new_tnr = xcam_malloc0_type(XCam3aResultTemporalNoiseReduction);
            XCAM_ASSERT (new_tnr);
            *new_tnr = tnr;
            results[result_count++] = (XCam3aResultHead*)new_tnr;
            break;
        }
        case XCAM_3A_RESULT_EDGE_ENHANCEMENT:
        {
            SmartPtr<X3aEdgeEnhancementResult> xcam_ee =
                isp_result.dynamic_cast_ptr<X3aEdgeEnhancementResult> ();
            const XCam3aResultEdgeEnhancement &ee = xcam_ee->get_standard_result();
            XCam3aResultEdgeEnhancement *new_ee = xcam_malloc0_type(XCam3aResultEdgeEnhancement);
            XCAM_ASSERT (new_ee);
            *new_ee = ee;
            results[result_count++] = (XCam3aResultHead*)new_ee;
            break;
        }
        case XCAM_3A_RESULT_BAYER_NOISE_REDUCTION:
        {
            SmartPtr<X3aBayerNoiseReduction> xcam_bnr =
                isp_result.dynamic_cast_ptr<X3aBayerNoiseReduction> ();
            const XCam3aResultBayerNoiseReduction &bnr = xcam_bnr->get_standard_result();
            XCam3aResultBayerNoiseReduction *new_bnr = xcam_malloc0_type(XCam3aResultBayerNoiseReduction);
            XCAM_ASSERT (new_bnr);
            *new_bnr = bnr;
            results[result_count++] = (XCam3aResultHead*)new_bnr;
            break;
        }
        case XCAM_3A_RESULT_WAVELET_NOISE_REDUCTION:
        {
            SmartPtr<X3aWaveletNoiseReduction> xcam_wavelet =
                isp_result.dynamic_cast_ptr<X3aWaveletNoiseReduction> ();
            const XCam3aResultWaveletNoiseReduction &wavelet = xcam_wavelet->get_standard_result();
            XCam3aResultWaveletNoiseReduction *new_wavelet = xcam_malloc0_type(XCam3aResultWaveletNoiseReduction);
            XCAM_ASSERT (new_wavelet);
            *new_wavelet = wavelet;
            results[result_count++] = (XCam3aResultHead*)new_wavelet;
            break;
        }
        default: {
            XCAM_LOG_WARNING ("unknown type(%d) in translation", isp_result->get_type());
            break;
        }
        }
    }
    return result_count;
}

void
free_3a_result (XCam3aResultHead *result)
{
    xcam_free (result);
}

}
