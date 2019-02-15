#ifndef _UAPI_RK_ISP_CONFIG_H
#define _UAPI_RK_ISP_CONFIG_H

// for new isp driver ported from chrome
// for rk3326(v12), rk3288(v10), rk3399(v10) ISP
#if defined(RKISP)
#include <rkisp1-config.h>
#include <rk_vcm_head.h>
#include <rk-preisp.h>
#include <rk-camera-module.h>
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
