#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>
#include <sys/ioctl.h>
#include <calib_xml/calibdb.h>
#include <utils/Log.h>

#include <HAL/CamIsp10CtrItf.h>
#include <af_ctrl.h>
#include <isp_ctrl.h>
#include <rkisp_interface.h>

//#include "oslayer/oslayer_linux.h"
//#include "shared_ptr.h"
//#include <cam_ia_api/cam_ia10_engine_api.h>
//#include <cam_ia_api/cam_ia10_engine.h>

#ifndef __cplusplus
#define __cplusplus
#endif

using namespace std;

int mVideoFd = -1;
CamIspCtrItf::Configuration mIspCfg;

struct IsiSensor_s gCamHwLtfConfig = {
  .pIsiMdiInitMotoDriveMds = rkisp_init_moto_drive,
  .pIsiMdiSetupMotoDrive = rkisp_setup_moto_drive,
  .pIsiMdiFocusSet = rkisp_af_focus_set,
  .pIsiMdiFocusGet = rkisp_af_focus_get,
};

/* -------- CamIsp10CtrItf interface -----------*/
int
rkisp_start(void* &engine, int vidFd, const char* ispNode, const char* tuningFile) {
	int ret;
	struct isp_supplemental_sensor_mode_data sensor_mode_data;
	CamIsp10CtrItf* ispDev = new CamIsp10CtrItf();

	//init
	mVideoFd = vidFd;

	//config
	memset(&mIspCfg, 0, sizeof(mIspCfg));
	ret = getSensorModeData(mVideoFd, &sensor_mode_data);
	configIsp(ispDev, &sensor_mode_data);

	//init ispDev
	ispDev->init(tuningFile, ispNode/*"/dev/video1"*/, mVideoFd);
	ispDev->start();
	engine = ispDev;
	LOGD("%s: interface isp dev started", __func__);
}

int
rkisp_stop(void* &engine) {
	if (engine != NULL) {
		LOGD("%s: rkisp interface ready to deinit", __func__);
		CamIsp10CtrItf *ispDev = (CamIsp10CtrItf*)engine;
		ispDev->stop();
		ispDev->deInit();
		delete ispDev;
		ispDev = NULL;
	}
}

/* ------------- CamIA10EngineItf interface ---------------------*/
int
rkisp_iq_init(void* engine, const char* tuningFile/*, struct CamIA10_DyCfg* ia_dcfg*/) {
	shared_ptr<CamIA10EngineItf> iqEngine = getCamIA10EngineItf();
	if (iqEngine->initStatic((char*)tuningFile) != RET_SUCCESS) {
		ALOGE("%s: initstatic failed", __func__);
		rkisp_iq_deinit(engine);
		return -1;
	}
	/*
	if (ia_dcfg)
		iqEngine->initDynamic(ia_dcfg);
	*/
	engine = iqEngine.get();
	return 0;
}

void
rkisp_iq_deinit(void* engine) {
	if (engine != NULL) {
		shared_ptr<CamIA10EngineItf> iqEngine =
			shared_ptr<CamIA10EngineItf>((CamIA10EngineItf*)engine);
		iqEngine.reset();
		iqEngine = NULL;
	}
}

int
rkisp_iq_statistics_set(void* engine, struct CamIA10_Stats* ia_stats) {
	if (engine != NULL) {
		shared_ptr<CamIA10EngineItf> iqEngine =
			shared_ptr<CamIA10EngineItf>((CamIA10EngineItf*)engine);
		iqEngine->setStatistics(ia_stats);
	}
	return 0;
}

int
rkisp_iq_ae_run(void* engine) {
	if (engine != NULL) {
		shared_ptr<CamIA10EngineItf> iqEngine =
			shared_ptr<CamIA10EngineItf>((CamIA10EngineItf*)engine);
		iqEngine->runAEC();
	}
	return 0;
}

int
rkisp_iq_get_aec_result(void* engine, AecResult_t* result) {
	if (engine != NULL) {
		shared_ptr<CamIA10EngineItf> iqEngine =
			shared_ptr<CamIA10EngineItf>((CamIA10EngineItf*)engine);
		iqEngine->getAECResults(result);
	}
	return 0;
}

int
rkisp_iq_af_run(void* engine) {
	if (engine != NULL) {
		shared_ptr<CamIA10EngineItf> iqEngine =
			shared_ptr<CamIA10EngineItf>((CamIA10EngineItf*)engine);
		iqEngine->runAF();
	}
	return 0;
}

int
rkisp_iq_get_af_result(void* engine, CamIA10_AFC_Result_t* result) {
	if (engine != NULL) {
		shared_ptr<CamIA10EngineItf> iqEngine =
			shared_ptr<CamIA10EngineItf>((CamIA10EngineItf*)engine);
		iqEngine->getAFResults(result);
	}
	return 0;
}

int
rkisp_iq_awb_run(void* engine) {
	if (engine != NULL) {
		shared_ptr<CamIA10EngineItf> iqEngine =
			shared_ptr<CamIA10EngineItf>((CamIA10EngineItf*)engine);
		iqEngine->runAWB();		
	}
	return 0;
}

int
rkisp_iq_get_awb_result(void* engine, CamIA10_AWB_Result_t* result) {
	if (engine != NULL) {
		shared_ptr<CamIA10EngineItf> iqEngine =
			shared_ptr<CamIA10EngineItf>((CamIA10EngineItf*)engine);
		iqEngine->getAWBResults(result);
	}
	return 0;
}

