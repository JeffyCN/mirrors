#ifndef _UAPI_RK_ISP_CONFIG_H
#define _UAPI_RK_ISP_CONFIG_H

// for new isp driver ported from chrome
// for rk3326 ISP
#if defined(RKISP_v12)
#include <rkisp1-config_v12.h>
#include <rk_vcm_head.h>
#elif defined(RKISP)
// for rk3399,rk3288 ISP
#include <rkisp1-config.h>
#include <rk_vcm_head.h>
// for old isp driver ported from sofia
#else
#ifdef RK_ISP10
#include "rk-isp10-config.h"
#endif
#ifdef RK_ISP11
#include "rk-isp11-config.h"
#endif
#endif
#endif
