/*
 * x3a_isp_config.h - 3A ISP config
 *
 *  Copyright (c) 2014-2015 Intel Corporation
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

#include "x3a_isp_config.h"
#include "isp_config_translator.h"

namespace XCam {

void AtomIspConfigContent::clear ()
{
    memset (this, 0, sizeof (AtomIspConfigContent));
}

void
AtomIspConfigContent::copy (const struct rkisp_parameters &isp_params)
{
    xcam_mem_clear (isp_config);
    isp_config = isp_params;
}

X3aIspConfig::X3aIspConfig ()
{
}

X3aIspConfig::~X3aIspConfig()
{
    clear ();
}


bool X3aIspConfig::clear()
{
    _isp_content.clear ();
    _3a_results.clear ();
    return true;
}

bool
X3aIspConfig::attach (SmartPtr<X3aResult> &result, IspConfigTranslator *translator)
{
    if (result.ptr() == NULL)
        return false;

    uint32_t type = result->get_type ();

    XCAM_ASSERT (translator);

    if (!result.ptr() || !result->get_ptr ()) {
        XCAM_LOG_ERROR ("3A result empty");
        return false;
    }

    //the member _3a_results of class X3aIspConfig could hold the smartPtr ref of
    //X3aAtomIspParametersResult result, thereby causing the memory leak
    clear();

    switch (type) {
    case X3aIspConfig::IspAllParameters: {
        SmartPtr<X3aAtomIspParametersResult> isp_3a =
            result.dynamic_cast_ptr<X3aAtomIspParametersResult> ();
        XCAM_ASSERT (isp_3a.ptr ());
        _isp_content.copy (isp_3a->get_isp_config());
    }
    break;

    case XCAM_3A_RESULT_ALL: {
        struct rkisp_parameters isp_params;
        SmartPtr<X3aAllResult> all_res =
            result.dynamic_cast_ptr<X3aAllResult> ();
        XCAM_ASSERT (all_res.ptr());
        if (translator->translate_all (all_res->get_standard_result(), isp_params)
                != XCAM_RETURN_NO_ERROR) {
            XCAM_LOG_WARNING ("translate white balance failed");
            return false;
        }

        _isp_content.isp_config = isp_params;
    }
    break;
    case XCAM_3A_RESULT_WHITE_BALANCE: {
#if 0
        struct rkisp_wb_config wb;
        SmartPtr<X3aWhiteBalanceResult> wb_res =
            result.dynamic_cast_ptr<X3aWhiteBalanceResult> ();
        XCAM_ASSERT (wb_res.ptr ());
        xcam_mem_clear (wb);
        if (translator->translate_white_balance (wb_res->get_standard_result(), wb)
                != XCAM_RETURN_NO_ERROR) {
            XCAM_LOG_WARNING ("translate white balance failed");
            return false;
        }
        _isp_content.wb = wb;
        _isp_content.isp_config.wb_config = &_isp_content.wb;
#endif
    }
    break;
    case XCAM_3A_RESULT_BLACK_LEVEL: {
#if 0
        struct rkisp_ob_config ob;
        SmartPtr<X3aBlackLevelResult> bl_res =
            result.dynamic_cast_ptr<X3aBlackLevelResult> ();
        XCAM_ASSERT (bl_res.ptr ());
        xcam_mem_clear (ob);
        if (translator->translate_black_level (bl_res->get_standard_result(), ob)
                != XCAM_RETURN_NO_ERROR) {
            XCAM_LOG_WARNING ("translate black level failed");
            return false;
        }
        _isp_content.ob = ob;
        _isp_content.isp_config.ob_config = &_isp_content.ob;
#endif
    }
    break;
    case XCAM_3A_RESULT_YUV2RGB_MATRIX:
    case XCAM_3A_RESULT_RGB2YUV_MATRIX:
    {
#if 0
        struct rkisp_cc_config cc;
        SmartPtr<X3aColorMatrixResult> cc_res =
            result.dynamic_cast_ptr<X3aColorMatrixResult> ();
        XCAM_ASSERT (cc_res.ptr ());
        xcam_mem_clear (cc);
        if (translator->translate_color_matrix (cc_res->get_standard_result(), cc)
                != XCAM_RETURN_NO_ERROR) {
            XCAM_LOG_WARNING ("translate color matrix failed");
            return false;
        }
        if (type == XCAM_3A_RESULT_YUV2RGB_MATRIX) {
            _isp_content.yuv2rgb_cc = cc;
            _isp_content.isp_config.yuv2rgb_cc_config = &_isp_content.yuv2rgb_cc;
        } else {
            _isp_content.rgb2yuv_cc = cc;
            _isp_content.isp_config.rgb2yuv_cc_config = &_isp_content.rgb2yuv_cc;
        }
#endif
    }
    break;
    default:
        return false;
    }

    _3a_results.push_back (result);
    return true;
}

};

