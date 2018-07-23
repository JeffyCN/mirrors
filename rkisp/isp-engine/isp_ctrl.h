#ifndef __ISP_CTRL__
#define __ISP_CTRL__

#include <isp10_engine.h>
int getSensorModeData(int devFd,
    struct isp_supplemental_sensor_mode_data* data);
int configIsp(Isp10Engine *ispDev,
        struct isp_supplemental_sensor_mode_data* sensor);
#endif
