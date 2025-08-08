#include "rk_aiq_isp39_modules.h"
#include <stdint.h>

uint16_t convert_coordinate(int x, uint16_t real)
{
    int center = real / 2;
    float percent = (float) x / 1000.0;
    uint16_t res = center + center * percent;
    return res;
}


/* 
 * zero dependency isp39 logtransf implementation
 */

/* Theory:
 * 对于任意大于1的正整数 I ,都可以表示为 I = M * 2^n
 * 其中 n 是非负整数, M 是 [1,2) 之间的小数,故 log2(I) = log2(M) + n
 * TODO: explain offsetbit
 */

/* 
 * logtablef[i] = log2(i/64 + 1) * 4096
 */
static const uint16_t isp39_logtablef[65] = {
   0,
  91, 181, 270, 358, 444, 529, 613, 696,
 777, 857, 937,1015,1092,1169,1244,1318,
1392,1464,1536,1606,1676,1745,1814,1881,
1948,2014,2079,2144,2208,2271,2334,2396,
2457,2517,2577,2637,2696,2754,2811,2869,
2925,2981,3037,3092,3146,3200,3253,3306,
3359,3411,3463,3514,3565,3615,3665,3714,
3763,3812,3860,3908,3955,4002,4049,4096,
};

/*
 * tmpfix: 输入像素值
 * is15bit_mode: 是否使用 15bit mode, 目前支持两种转换模式
 *     15bit mode: log域 3位整数, 9位小数, 输入范围为 15bit (0 - 32768)
 *     20bit mode: log域 4位整数, 8位小数, 输入范围为 20bit (0 - 1048576)
 * offsetbit: 通常设置为8
 * 
 * return: log 域值 (0 - 4096)
 */
uint16_t isp39_logtransf(uint32_t tmpfix, uint8_t is15bit_mode, uint8_t offsetbit)
{
//#define MIN(a,b)  ((a) <= (b) ? (a):(b))
#define isp39_LOGPRECISION 6
    uint32_t x8, one = 1;
    uint16_t lt1, lt2;

    uint16_t isp39_logprecesion = 1 << isp39_LOGPRECISION;
    uint16_t transf_mode_offset = 1 << offsetbit;

    uint8_t isp39_logscalebit = is15bit_mode? 9: 8;
    uint16_t itransf_mode_offset = offsetbit << isp39_logscalebit;
    uint32_t transf_data_max_limit = is15bit_mode? 32768: 1048576;

    // add offset, limit to data_max_limit
    x8 = MIN((tmpfix + transf_mode_offset), transf_data_max_limit);

    // find the highest bit, which is 'n'
    uint8_t n = 0;
    uint32_t tmp_x8 = x8;
    while (tmp_x8 >>= 1) {
        n ++;
    }

    // gxf = M - 1.0
    uint32_t gx = x8 - (one << n);
    float gxf = 1.0 * gx / (one << n);

    // lookup gxf in logtablef
    uint8_t ix1 = gxf * isp39_logprecesion;
    uint8_t ix2 = ix1 + 1;
    float dpf = (float)((gxf * isp39_logprecesion) - ix1);

    lt1 = isp39_logtablef[ix1];
    lt2 = isp39_logtablef[ix2];

    // dxf almost equal to log2(M) * 4096
    float dxf = (float)lt1 * (1 - dpf) + (float)lt2 *dpf;

    // fxf = (log2(M) + n) * 4096
    float fxf = n * 4096.0 + dxf;

    // fxf = (log2(M) + n) * (1 << isp39_logscalebit)
    fxf = fxf * (1 << isp39_logscalebit) / 4096.0;

    uint16_t fx = (uint16_t) (fxf + 0.5);
    fx = fx - itransf_mode_offset;

    return (int)fx;
}


