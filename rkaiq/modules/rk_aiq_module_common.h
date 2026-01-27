#ifndef _RK_AIQ_MODULE_COMMON_H_
#define _RK_AIQ_MODULE_COMMON_H_

#define MGE_WGT_USE_OLD_STRUCT (0)

#if MGE_WGT_USE_OLD_STRUCT
typedef struct mergeLuma2Wgt_s {
    int luma_idx[17];
    float luma_wgt[17];
} mergeLuma2Wgt_t;

#else
#define MERGE_MAX_WGT_CURVE_NUM (2)

typedef struct rmge_wgtCurve_s {
    int idx[17];
    float val[17];
} mge_wgtCurve_t;

typedef struct mergeLuma2Wgt_s {
    int wgtCurve_num;
    mge_wgtCurve_t wgtCurve[MERGE_MAX_WGT_CURVE_NUM];
} mergeLuma2Wgt_t;
#endif

uint16_t convert_coordinate(int x, uint16_t real);
uint16_t isp39_logtransf(uint32_t tmpfix, uint8_t is15bit_mode, uint8_t offsetbit);

#endif
