#ifndef __RK_AIQ_TYPES_ALDC_ALGO_INT_H__
#define __RK_AIQ_TYPES_ALDC_ALGO_INT_H__

#include <stddef.h>
#include <stdint.h>

#include "algos/aldc/rk_aiq_types_aldc_algo.h"
#include "common/rk_aiq_comm.h"

typedef struct rk_aiq_uapi_sync_s rk_aiq_uapi_sync_t;

typedef enum {
    kRKAIQLdcUpdateLutOnLine = 0,  // generate lut inside rkaiq
    kRKAIQLdcUpdateLutFromFile,    // external file import lut
    kRKAIQLdcUpdateLutFromBuffer,  // external buffer import lut
} RKAIQLdcUpdateLutMode;

typedef struct rk_aiq_ldc_lut_external_file_s {
    char config_file_dir[64];
    char mesh_file_name[32];
} rk_aiq_ldc_lut_external_file_t;

typedef struct rk_aiq_ldc_lut_external_buffer_s {
    void* addr;
    size_t size;
} rk_aiq_ldc_lut_external_buffer_t;

typedef struct rk_aiq_ldc_custom_lut_s {
    bool update_flag;
    union {
        rk_aiq_ldc_lut_external_file_t file;
        rk_aiq_ldc_lut_external_buffer_t buffer;
    } u;
} rk_aiq_ldc_custom_lut_t;

typedef struct rk_aiq_ldch_v22_attrib_s {
    RKAIQLdcUpdateLutMode lut_upd_mode;
    uint8_t bic_weight[ISP39_LDC_BIC_NUM];

    uint8_t correct_level;
    rk_aiq_ldc_custom_lut_t lut;
} rk_aiq_ldc_ldch_attrib_t;

typedef struct rk_aiq_ldcv_attrib_s {
    RKAIQLdcUpdateLutMode lut_upd_mode;
    uint8_t bic_weight[ISP39_LDC_BIC_NUM];

    uint8_t correct_level;
    rk_aiq_ldc_custom_lut_t lut;
} rk_aiq_ldc_ldcv_attrib_t;

typedef struct rk_aiq_ldc_attrib_s {
    rk_aiq_uapi_sync_t sync;

    rk_aiq_ldc_ldch_attrib_t ldch;
    rk_aiq_ldc_ldcv_attrib_t ldcv;
} rk_aiq_ldc_attrib_t;

#endif
