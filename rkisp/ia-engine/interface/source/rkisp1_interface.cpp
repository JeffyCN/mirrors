#include <ebase/types.h>
//#include <ebase/trace.h>
#include <ebase/builtins.h>
#include <sys/ioctl.h>
#include <calib_xml/calibdb.h>
#include <base/log.h>

#include <HAL/CamIsp101CtrItf.h>
#include <af_ctrl.h>
//#include <isp_ctrl.h>
#include <rkisp1_interface.h>
#include <rkisp1_media.h>

int
rkisp1_start(void* &engine, int vidFd, const char* tuningFile) {
	int ret;
	char vdevStats[32];
	char vdevParams[32];
	CamIsp101CtrItf* ispDev = new CamIsp101CtrItf();

	//init
	vidFd;
	rk_common_v4l2device_find_by_name ("rkisp1-statistics", vdevStats);
	rk_common_v4l2device_find_by_name ("rkisp1-input-params", vdevParams);

	//config
	//memset(&mIspCfg, 0, sizeof(mIspCfg));
	//configIsp(ispDev, &sensor_mode_data);

	//init ispDev
	ispDev->initIsp1(tuningFile, vidFd, vdevStats, vdevParams);
	ispDev->start();
	engine = ispDev;
	LOGD("%s: interface isp dev started", __func__);
}

int
rkisp1_stop(void* &engine) {
	if (engine != NULL) {
		LOGD("%s: rkisp interface ready to deinit", __func__);
		CamIsp101CtrItf *ispDev = (CamIsp101CtrItf*)engine;
		ispDev->stop();
		ispDev->deInit();
		delete ispDev;
		ispDev = NULL;
	}
}

