#ifndef __ISP_CTRL__
#define __ISP_CTRL__

#include <HAL/CamIsp10CtrItf.h>
int getSensorModeData(int devFd,
    struct isp_supplemental_sensor_mode_data* data);
int configIsp(CamIsp10CtrItf *ispDev,
        struct isp_supplemental_sensor_mode_data* sensor);
#endif
