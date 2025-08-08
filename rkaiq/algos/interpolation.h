
#ifndef __INTERPOLATION_H__
#define __INTERPOLATION_H__
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

void interpolation_f(const float *x, const float *y, int Num, float x0, float*y0);
void interpolation_s(const float *x, const unsigned short *y, int Num, float x0, unsigned short *y0);
int interpolation_b(unsigned char *x, bool *y, int xNum, unsigned char x0, bool *y0);
void pre_interp(int iso, uint32_t *iso_list, int num, int *lo, int *hi, float *ratio);
void interpolation_set_iso_list(const uint32_t *iso_list);
void interpolation_get_iso_list(uint32_t *iso_list);
void get_interpolationf_wgt(const float *x, const float *y, int Num, float x0, int *idxS, int *idxE, float *wgtS, float *wgtE);
void interpolation_with_wgt(float valueS,float valueE,float wgtS,float wgtE,float *value);
#define RATIO_FIXBIT 14

static inline bool interpolation_bool(bool lower, bool upper, uint16_t ratio) {
    bool is_upper = !! (ratio & (1 << (RATIO_FIXBIT - 1)));
    // return is_upper?: upper: lower;
    return (is_upper && upper) || (!is_upper && lower);
}

static inline uint8_t interpolation_u8(uint8_t lower, uint8_t upper, uint16_t ratio) {
    int32_t tmp = (int32_t)upper - (int32_t) lower;
    tmp = (tmp * ratio) >> RATIO_FIXBIT;
    return lower + tmp;
}

static inline int8_t interpolation_i8(int8_t lower, int8_t upper, uint16_t ratio) {
    int32_t tmp = (int32_t)upper - (int32_t) lower;
    tmp = (tmp * ratio) >> RATIO_FIXBIT;
    return lower + tmp;
}

static inline uint16_t interpolation_u16(uint16_t lower, uint16_t upper, uint16_t ratio) {
    int32_t tmp = (int32_t)upper - (int32_t) lower;
    tmp = (tmp * ratio) >> RATIO_FIXBIT;
    return lower + tmp;
}

static inline int16_t interpolation_i16(int16_t lower, int16_t upper, uint16_t ratio) {
    int32_t tmp = (int32_t)upper - (int32_t) lower;
    tmp = (tmp * ratio) >> RATIO_FIXBIT;
    return lower + tmp;
}

static inline int32_t interpolation_i32(int32_t lower, int32_t upper, float ratio) {
    if (lower == upper) {
        return lower;
    } else {
        float tmp = (float)upper - (float) lower;
        tmp = lower + tmp * ratio;
        return ((int32_t)(tmp + 0.5));
    }
}

static inline uint32_t interpolation_u32(uint32_t lower, uint32_t upper, float ratio) {
    if (lower == upper) {
        return lower;
    } else {
        float tmp = (float)upper - (float) lower;
        tmp = lower + tmp * ratio;
        return ((uint32_t)(tmp + 0.5));
    }
}

static inline float interpolation_f32(float lower, float upper, float ratio) {
    if (lower == upper) {
        return lower;
    } else {
        float tmp = (float)upper - (float) lower;
        tmp = lower + tmp * ratio;
        return tmp;
    }
}

#ifdef __cplusplus
}
#endif

#endif
