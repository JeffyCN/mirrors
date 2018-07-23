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

#ifndef XCAM_3A_ISP_CONFIG_H
#define XCAM_3A_ISP_CONFIG_H

#include <xcam_std.h>
#include <x3a_result.h>
#include <linux/rkisp.h>
#include <base/xcam_3a_result.h>

namespace XCam {

#define XCAM_3A_ISP_RESULT_TYPE_START (XCAM_3A_RESULT_USER_DEFINED_TYPE + 0x1000)

struct AtomIspConfigContent {
    bool enabled[HAL_ISP_MODULE_MAX_ID_ID + 1];
    int flt_denoise_level;
    int flt_sharp_level;

    struct rkisp_parameters   isp_config;
    //content
    struct cifisp_dpcc_config dpcc_config;
    struct cifisp_bls_config bls_config;
    struct cifisp_sdg_config sdg_config;
    struct cifisp_hst_config hst_config;
    struct cifisp_lsc_config lsc_config;
    struct cifisp_awb_gain_config awb_gain_config;
    struct cifisp_awb_meas_config awb_meas_config;
    struct cifisp_flt_config flt_config;
    struct cifisp_bdm_config bdm_config;
    struct cifisp_ctk_config ctk_config;
    struct cifisp_goc_config goc_config;
    struct cifisp_cproc_config cproc_config;
    struct cifisp_aec_config aec_config;
    struct cifisp_afc_config afc_config;
    struct cifisp_ie_config ie_config;
    struct cifisp_dpf_config dpf_config;
    struct cifisp_dpf_strength_config dpf_strength_config;

    void clear ();
    void copy (const struct rkisp_parameters &config);

    AtomIspConfigContent () {
        clear ();
    }
};

class IspConfigTranslator;

class X3aIspConfig
{
public:
    enum X3aIspConfigType {
        IspAllParameters = XCAM_3A_ISP_RESULT_TYPE_START,
        IspExposureParameters,
        IspFocusParameters,
    };

    struct X3aIspResultDummy {
        XCam3aResultHead head;
    };
public:
    explicit X3aIspConfig ();
    virtual ~X3aIspConfig();

public:
    const struct rkisp_parameters &get_isp_configs () const {
        return _isp_content.isp_config;
    }
    struct rkisp_parameters &get_isp_configs () {
        return _isp_content.isp_config;
    }
    bool clear ();
    bool attach (SmartPtr<X3aResult> &result, IspConfigTranslator *translator);

private:
    XCAM_DEAD_COPY (X3aIspConfig);

protected:
    AtomIspConfigContent             _isp_content;
    std::list< SmartPtr<X3aResult> > _3a_results;
};

template <typename IspConfig, typename StandardResult, uint32_t type>
class X3aIspResultT
    : public X3aStandardResultT<StandardResult>
{
public:
    X3aIspResultT (
        XCamImageProcessType process_type = XCAM_IMAGE_PROCESS_ALWAYS
    )
        : X3aStandardResultT<StandardResult> (type, process_type)
    {
        X3aResult::set_ptr((void*)&_isp_config);
    }

    ~X3aIspResultT () {}

    // set config
    void set_isp_config (IspConfig &config) {
        _isp_config = config;
    }
    const IspConfig &get_isp_config () const {
        return _isp_config;
    }

private:
    IspConfig _isp_config;
};


/* special X3aAtomIspParametersResult type */
template <>
class X3aIspResultT<struct rkisp_parameters, X3aIspConfig::X3aIspResultDummy, X3aIspConfig::IspAllParameters>
        : public X3aStandardResultT<X3aIspConfig::X3aIspResultDummy>
    {
public:
        X3aIspResultT (
            XCamImageProcessType process_type = XCAM_IMAGE_PROCESS_ALWAYS)
            : X3aStandardResultT<X3aIspConfig::X3aIspResultDummy> ((uint32_t)X3aIspConfig::IspAllParameters, process_type)
        {
            X3aResult::set_ptr((void*)&_content.isp_config);
        }

        ~X3aIspResultT () {}

        // get config
        struct rkisp_parameters &get_isp_config () {
            return _content.isp_config;
        }
        const struct rkisp_parameters &get_isp_config () const {
            return _content.isp_config;
        }

        // set config
        void set_isp_config (struct rkisp_parameters &config) {
            _content.copy (config);
        }

private:
        AtomIspConfigContent      _content;
    };

typedef
X3aIspResultT<struct rkisp_parameters, X3aIspConfig::X3aIspResultDummy, X3aIspConfig::IspAllParameters> X3aAtomIspParametersResult;
typedef
X3aIspResultT<struct rkisp_exposure, XCam3aResultExposure, X3aIspConfig::IspExposureParameters> X3aIspExposureResult;
typedef
X3aIspResultT<struct rkisp_focus, XCam3aResultFocus, X3aIspConfig::IspFocusParameters> X3aIspFocusResult;
};

#endif //XCAM_3A_ISP_CONFIG_H

