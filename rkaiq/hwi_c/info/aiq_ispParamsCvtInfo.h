/*
 *   Copyright (c) 2024 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *
 */

#ifndef _AIQ_ISP_PARAMS_CVT_INFO_H_
#define _AIQ_ISP_PARAMS_CVT_INFO_H_

#include "hwi_c/aiq_ispParamsCvt.h"

void cvt_isp_params_dump_by_type(void* self, int type, st_string* result);
void cvt_isp_params_dump_mod_param(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_rgbir_attr(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_bls_attr(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_wbgain_attr(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_dpc_attr(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_lsc_attr(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_gic_attr(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_debayer_attr(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_ccm_attr(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_gammaout_attr(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_cp_attr(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_sdg_attr(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_drc_attr(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_mge_attr(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_dhaz_attr(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_3dlut_attr(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_ldch_attr(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_ldcv_attr(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_bay3d_attr(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_ynr_attr(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_cnr_attr(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_sharp_attr(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_cac_attr(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_gain_attr(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_csm_attr(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_cgc_attr(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_yme_attr(AiqIspParamsCvt_t* self, st_string* result);
void cvt_isp_params_dump_aec_attr(AiqIspParamsCvt_t* self, st_string* result);


#endif  // _AIQ_ISP_PARAMS_CVT_INFO_H_
