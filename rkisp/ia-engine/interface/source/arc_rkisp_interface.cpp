#include <HAL/arc_rkisp_adapter.h>
#include <cam_ia_api/cam_ia10_engine_api.h>
#include <cam_ia_api/cam_ia10_engine.h>
#include <utils/Log.h>

rk_aiq* rk_aiq_init(const char* iq_xml_file) {
    CamIA10Engine* iqEngine = new CamIA10Engine();
    if (iqEngine->initStatic((char*)iq_xml_file) != RET_SUCCESS) {
        ALOGE("%s: initstatic failed", __func__);
        rk_aiq_deinit((rk_aiq*)iqEngine);
        return NULL;
    } else {
        LOGD("%s: initstatic success - rkisp ver 1.03", __func__);
    }

    return (rk_aiq*)iqEngine;
}

void rk_aiq_deinit(rk_aiq* ctx) {
    if (ctx != NULL) {
        CamIA10Engine* iqEngine =
            (CamIA10Engine*)ctx;
        delete iqEngine;
        iqEngine = NULL;
    }
}

int rk_aiq_stats_set(rk_aiq* ctx,
                     const rk_aiq_statistics_input_params* stats,
                     const rk_aiq_exposure_sensor_descriptor *sensor_desc) {
    if (ctx != NULL) {
        CamIA10Engine* iqEngine =
            (CamIA10Engine*)ctx;
        struct CamIA10_Stats sensor_stats;

	if (sensor_desc != NULL &&
		(iqEngine->mStats.sensor_mode.isp_input_width != 0 &&
			iqEngine->mStats.sensor_mode.isp_input_height!= 0) &&
		(sensor_desc->sensor_output_width != iqEngine->mStats.sensor_mode.isp_input_width ||
			sensor_desc->sensor_output_height != iqEngine->mStats.sensor_mode.isp_input_height)) {
		if (iqEngine->restart()!= RET_SUCCESS) {
			ALOGE("%s: restart isp engine failed", __func__);
			rk_aiq_deinit((rk_aiq*)iqEngine);
			return NULL;
		}

	}
        convert_to_rkisp_stats(stats, sensor_desc, &sensor_stats);
        iqEngine->setStatistics(&sensor_stats);
    }
    return 0;
}

int rk_aiq_ae_run(rk_aiq* ctx,
                   const rk_aiq_ae_input_params* aec_input_params,
                   rk_aiq_ae_results* aec_result) {
    if (ctx != NULL) {
        CamIA10Engine* iqEngine =
            (CamIA10Engine*)ctx;
	HAL_AecCfg config;
        convert_to_rkisp_aec_params(aec_input_params, &config);
        iqEngine->runAEC(&config);
	AecResult_t result;
	memset(&result, 0, sizeof(AecResult_t));
        iqEngine->getAECResults(&result);
	convert_from_rkisp_aec_result(aec_result, &result);
    }
    return 0;
}

int rk_aiq_awb_run(rk_aiq* ctx,
                   const rk_aiq_awb_input_params* awb_input_params,
                   rk_aiq_awb_results* awb_result) {
    if (ctx != NULL) {
        CamIA10Engine* iqEngine =
            (CamIA10Engine*)ctx;
	HAL_AwbCfg config;
	CamIA10_AWB_Result_t result;

        memset(&result, 0, sizeof(CamIA10_AWB_Result_t));
	convert_to_rkisp_awb_params(awb_input_params, &config);
        iqEngine->runAWB(&config);
        iqEngine->getAWBResults(&result);
	convert_from_rkisp_awb_result(awb_result, &result);
    }
    return 0;
}

int rk_aiq_af_run(rk_aiq_ctx_s* ctx,
                  const rk_aiq_af_input_params* af_input_params,
                  rk_aiq_af_results* af_result) {
    if (ctx != NULL) {
        CamIA10Engine* iqEngine =
            (CamIA10Engine*)ctx;
	HAL_AfcCfg config;
	CamIA10_AFC_Result_t result;
    	memset(&result, 0, sizeof(CamIA10_AFC_Result_t));
	convert_to_rkisp_af_params(af_input_params, &config);
        iqEngine->runAF(&config);
	iqEngine->getAFResults(&result);
	convert_from_rkisp_af_result(af_result, &result);
    }
    return 0;
}

int rk_aiq_misc_run(rk_aiq* ctx,
                    const rk_aiq_misc_isp_input_params* misc_input_params,
                    rk_aiq_misc_isp_results* misc_results) {
	if (ctx != NULL) {
		CamIA10Engine* iqEngine =
				(CamIA10Engine*)ctx;
		
		struct CamIA10_Results ia_results;
		memset(&ia_results, 0, sizeof(struct CamIA10_Results));
		iqEngine->runADPF();
		iqEngine->getADPFResults(&ia_results.adpf);
		

		convert_from_rkisp_misc_result(iqEngine, misc_results, &ia_results);
		return 0;
	}

}
