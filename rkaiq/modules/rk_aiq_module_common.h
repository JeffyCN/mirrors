#ifndef _RK_AIQ_MODULE_COMMON_H_
#define _RK_AIQ_MODULE_COMMON_H_

typedef struct mergeLuma2Wgt_s {
    int luma_idx[17];
    float luma_wgt[17];
} mergeLuma2Wgt_t;

uint16_t convert_coordinate(int x, uint16_t real);
uint16_t isp39_logtransf(uint32_t tmpfix, uint8_t is15bit_mode, uint8_t offsetbit);

#endif
