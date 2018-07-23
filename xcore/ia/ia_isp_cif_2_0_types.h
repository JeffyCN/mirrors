/*
 *
 * Copyright 2014 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef IA_ISP_CIF_2_0_TYPES_H_
#define IA_ISP_CIF_2_0_TYPES_H_

#ifdef ANDROID
// If we are building in android this header file is exported by the kernel
// so we use the one the kernel exports.
#include <linux/xgold-isp-ioctl.h>
#else
// in other environments  (linux/windows) we use the local copy that is updated
// regularly.
// TODO: refactor this kernel header is needed to move the structures AIC
// depends on to a separate file.
#include <xgold-isp-ioctl.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------- INPUTS TO ISP ----------------------*/
#define BPC_CONFIG (1)
#define BLS_CONFIG (BPC_CONFIG << 1)
#define SDG_CONFIG (BLS_CONFIG << 1)
#define HST_CONFIG (SDG_CONFIG << 1)
#define LSC_CONFIG (HST_CONFIG << 1)
#define AWB_CONFIG (LSC_CONFIG << 1)
#define FLT_CONFIG (AWB_CONFIG << 1)
#define BDM_CONFIG (FLT_CONFIG << 1)
#define CTK_CONFIG (BDM_CONFIG << 1)
#define GOC_CONFIG (CTK_CONFIG << 1)
#define CPROC_CONFIG (GOC_CONFIG << 1)
#define YCFILT_CONFIG (CPROC_CONFIG << 1)
#define AFC_CONFIG (YCFILT_CONFIG << 1)
#define TMAP_CONFIG (AFC_CONFIG << 1)
#define MACC_CONFIG (TMAP_CONFIG << 1)
#define AWB_MEAS_CONFIG (MACC_CONFIG << 1)
#define IE_CONFIG (AWB_MEAS_CONFIG << 1)

#define CIFISP_BPC_CONFIG_SIZE (IA_ALIGN(sizeof(struct cifisp_bpc_config),8))
#define CIFISP_BLS_CONFIG_SIZE (IA_ALIGN(sizeof(struct cifisp_bls_config),8))
#define CIFISP_SDG_CONFIG_SIZE (IA_ALIGN(sizeof(struct cifisp_sdg_config),8))
#define CIFISP_LSC_CONFIG_SIZE (IA_ALIGN(sizeof(struct cifisp_lsc_config),8))
#define CIFISP_AWB_GAIN_CONFIG_SIZE (IA_ALIGN(sizeof(struct cifisp_awb_gain_config),8))
#define CIFISP_AWB_MEAS_CONFIG_SIZE (IA_ALIGN(sizeof(struct cifisp_awb_meas_config),8))
#define CIFISP_AEC_MEAS_CONFIG_SIZE (IA_ALIGN(sizeof(struct cifisp_aec_config),8))
#define CIFISP_BDM_CONFIG_SIZE (IA_ALIGN(sizeof(struct cifisp_bdm_config),8))
#define CIFISP_FLT_CONFIG_SIZE (IA_ALIGN(sizeof(struct cifisp_flt_config),8))
#define CIFISP_CTK_CONFIG_SIZE (IA_ALIGN(sizeof(struct cifisp_ctk_config),8))
#define CIFISP_GOC_CONFIG_SIZE (IA_ALIGN(sizeof(struct cifisp_goc_config),8))
#define CIFISP_CPROC_CONFIG_SIZE (IA_ALIGN(sizeof(struct cifisp_cproc_config),8))
#define CIFISP_YCFILT_CONFIG_SIZE (IA_ALIGN(sizeof(struct cifisp_ycflt_config),8))
#define CIFISP_AFC_CONFIG_SIZE (IA_ALIGN(sizeof(struct cifisp_afc_config),8))
#define CIFISP_HST_CONFIG_SIZE (IA_ALIGN(sizeof(struct cifisp_hst_config),8))
#define CIFISP_TMAP_CONFIG_SIZE (IA_ALIGN(sizeof(struct cifisp_tmap_config),8))
#define CIFISP_MACC_CONFIG_SIZE (IA_ALIGN(sizeof(struct cifisp_macc_config),8))

struct ia_cif_isp_2_0_awb_stat{
    struct cifisp_awb_stat awb_stat;
    unsigned short grid_width;
    unsigned short grid_height;
    struct cifisp_window window_config;
};

struct ia_cif_isp_2_0_af_stat{
    struct cifisp_af_stat af_stat;
    struct cifisp_window window_config[CIFISP_AFM_MAX_WINDOWS];
    unsigned short grid_width;
    unsigned short grid_height;
};

struct ia_cif_isp_2_0_stats{
    struct ia_cif_isp_2_0_awb_stat awb;
    struct ia_cif_isp_2_0_af_stat af; 
};

struct ia_cif_isp_2_0_config {
    unsigned int active_configs;
    struct cifisp_bpc_config bpc_config;
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
    struct cifisp_ycflt_config ycfilt_config;
    struct cifisp_afc_config afc_config;
    struct cifisp_tmap_config tmap_config;
    struct cifisp_macc_config macc_config;
    struct cifisp_ie_config ie_config;
};

#ifdef __cplusplus
}
#endif

#endif /* IA_CIF_2_0_TYPES_H_ */
