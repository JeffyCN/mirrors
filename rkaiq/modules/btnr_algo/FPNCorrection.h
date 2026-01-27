#ifndef FPN_CORRECTION_H
#define FPN_CORRECTION_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef _WIN32
#include "NEON_2_SSE.h"
#else
#include "arm_neon.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct
    {
        int raw_wid;
        int raw_hgt;
        int fpn_offset;

        float sensor_gain;
        float max_sensor_gain;
        uint8_t* current_raw_ptr;
        char fpn_path[256];
        uint8_t* fpn_raw_ptr;

        int raw_format;     // 0 = packed10, 1 = unpacked16
        int unpacked_shift;

        int log_en;
    } rk_fpn_correction_param_t;

    extern int rk_fpn_correction_init(rk_fpn_correction_param_t* param);
    extern int rk_fpn_correction_proc(rk_fpn_correction_param_t* param);
    extern int rk_fpn_correction_deinit(rk_fpn_correction_param_t* param);

#ifdef __cplusplus
}
#endif

#endif // FPN_CORRECTION_H