
#include "rk_aiq.h"
#include "rk_aiq_comm.h"
#include "common/rkisp32-config.h"
#include "uAPI2/rk_aiq_user_api2_isp32.h"
#include "rk_aiq_isp32_modules.h"

#include "range_check.h"
#include "rk_aiq_module_dm21.c"
#include "rk_aiq_module_gamma21.c"
//#include "rk_aiq_module_dehaze22.c"
#include "rk_aiq_module_btnr32.c"
#include "rk_aiq_module_sharp32.c"
#include "rk_aiq_module_ynr32.c"
#ifdef RKAIQ_HAVE_DRC_V12
#include "rk_aiq_module_drc32.c"
#endif