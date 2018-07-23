#include "ia_types.h"
#include "ia_cmc_types.h"
#include "ia_mkn_types.h"
#include "ia_abstraction.h"
#include "ia_aiq_types.h"
#include "ia_isp_types.h"
#include "ia_coordinate.h"
#include "ia_isp_2_2.h"
#include "ia_aiq.h"
#include "ia_cmc_parser.h"
#include "ia_mkn_encoder.h"
#include <rk_intel_bufmgr.h>

/* ------------- USELESS::intel ia interface ---------- */
void
ia_cmc_parser_deinit(ia_cmc_t *ia_cmc) {

}

ia_cmc_t*
ia_cmc_parser_init(const ia_binary_data *aiqb_binary) {
	return NULL;
}

void
ia_coordinate_convert_rect(const ia_coordinate_system* a_src_system_ptr,
                                const ia_rectangle* a_src_rect_ptr,
                                const ia_coordinate_system* a_tgt_system_ptr,
                                ia_rectangle* a_tgt_rect_ptr) {

}

ia_isp*
ia_isp_2_2_init(const ia_binary_data *aiqb_data,
        unsigned int stats_max_width,
        unsigned int stats_max_height,
        ia_cmc_t *ia_cmc,
        ia_mkn *ia_mkn) {
	return NULL;
}

void
ia_isp_2_2_deinit(ia_isp *ia_isp) {
}

ia_err
ia_isp_2_2_run(ia_isp *ia_isp,
               const ia_isp_2_2_input_params *isp_input_params,
               ia_binary_data *output_data) {
	return ia_err_none;
}

void
ia_aiq_deinit(ia_aiq *ia_aiq) {
}

ia_err
ia_isp_2_2_statistics_convert(ia_isp *ia_isp,
    void *statistics,
    ia_aiq_rgbs_grid **out_rgbs_grid,
    ia_aiq_af_grid **out_af_grid) {

	return ia_err_none;
}

ia_mkn*
ia_mkn_init(ia_mkn_config_bits mkn_config_bits,
            size_t mkn_section_1_size,
            size_t mkn_section_2_size) {
	return NULL;
}

void
ia_mkn_uninit(ia_mkn *mkn) {
}

ia_err
ia_aiq_awb_run(ia_aiq *ia_aiq,
               const ia_aiq_awb_input_params *awb_input_params,
               ia_aiq_awb_results **awb_results) {
	return ia_err_none;
}

ia_aiq*
ia_aiq_init(const ia_binary_data *aiqb_data,
            const ia_binary_data *nvm_data,
            const ia_binary_data *aiqd_data,
            unsigned int stats_max_width,
            unsigned int stats_max_height,
            unsigned int max_num_stats_in,
            ia_cmc_t *ia_cmc,
            ia_mkn *ia_mkn) {
	return NULL;
}

ia_err
ia_aiq_pa_run(ia_aiq *ia_aiq,
               const ia_aiq_pa_input_params *pa_input_params,
               ia_aiq_pa_results **pa_results) {
	return ia_err_none;
}

ia_err
ia_aiq_gbce_run(ia_aiq *ia_aiq,
                const ia_aiq_gbce_input_params *gbce_input_params,
                ia_aiq_gbce_results **gbce_results) {
	return ia_err_none;
}

ia_err
ia_aiq_ae_run(ia_aiq *ia_aiq,
              const ia_aiq_ae_input_params *ae_input_params,
              ia_aiq_ae_results **ae_results) {
	return ia_err_none;
}

ia_err
ia_aiq_sa_run(ia_aiq *ia_aiq,
               const ia_aiq_sa_input_params *sa_input_params,
               ia_aiq_sa_results **sa_results) {
	return ia_err_none;
}

ia_err
ia_aiq_statistics_set(ia_aiq *ia_aiq,
                      const ia_aiq_statistics_input_params *statistics_input_params) {
	return ia_err_none;
}

drm_intel_bo *
drm_intel_bo_alloc(drm_intel_bufmgr *bufmgr, const char *name,
                 unsigned long size, unsigned int alignment) {
	return NULL;
}

int
drm_intel_gem_bo_unmap_gtt(drm_intel_bo *bo) {
	return 0;
}


void
drm_intel_bufmgr_destroy(drm_intel_bufmgr *bufmgr) {
}

void
drm_intel_bo_unreference(drm_intel_bo *bo) {
}

int
drm_intel_bo_map(drm_intel_bo *bo, int write_enable) {
	return 0;
}

int
drm_intel_bo_unmap(drm_intel_bo *bo) {
	return 0;
}

int
drm_intel_bo_gem_export_to_prime(drm_intel_bo *bo, int *prime_fd) {
	return 0;
}

int
drm_intel_bo_get_tiling(drm_intel_bo *bo, uint32_t * tiling_mode,
                uint32_t * swizzle_mode) {
	return 0;
}

int
drm_intel_gem_bo_map_gtt(drm_intel_bo *bo) {
	return 0;
}

drm_intel_bufmgr*
drm_intel_bufmgr_gem_init(int fd, int batch_size) {
	return NULL;
}

void
drm_intel_bufmgr_gem_enable_reuse(drm_intel_bufmgr *bufmgr) {
}

drm_intel_bo*
drm_intel_bo_gem_create_from_prime(drm_intel_bufmgr *bufmgr,
                        int prime_fd, int size) {
	return NULL;
}

