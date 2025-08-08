
#ifndef __ALGO_COMMON_H__
#define __ALGO_COMMON_H__
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include "algo_types_priv.h"
#include "xcam_log.h"

void get_illu_estm_info(
        illu_estm_info_t *info,
        RkAiqAlgoProcResAwbShared_t *awbRes,
        RKAiqAecExpInfo_t *aeRes,
        int working_mode
);

float algo_strength_to_percent(float fStrength);

#ifdef __cplusplus
}
#endif

#endif
