
#ifndef __INTERPOLATION_H__
#define __INTERPOLATION_H__
#include <stdint.h>
#include <typeinfo>

void interpolation(const float *x, const float *y, int Num, float x0, float*y0);
void interpolation(const float *x, const unsigned short *y, int Num, float x0, unsigned short *y0);
int interpolation(unsigned char *x, bool *y, int xNum, unsigned char x0, bool *y0);
void pre_interp(int iso, int *iso_list, int num, int &lo, int &hi, float &ratio);

#define RATIO_FIXBIT 14

template <class T>
inline T interpolation_temp1(T lower, T upper, uint16_t ratio){
    int32_t tmp = (int32_t)upper - (int32_t) lower;
    tmp = (tmp * ratio) >> RATIO_FIXBIT;
    return lower + tmp;
}

template <class T>
inline T interpolation_temp2(T lower, T upper, float ratio){
    if (lower == upper) {
        return lower;
    } else {
        float tmp = (float)upper - (float) lower;
        tmp = lower + tmp * ratio;
        if (typeid(T) == typeid(float))
            return tmp;
        else
            return (T) (tmp + 0.5);
    }
}

inline bool interpolation_bool(bool lower, bool upper, uint16_t ratio) {
    bool is_upper = !! (ratio & (1 << (RATIO_FIXBIT - 1)));
    // return is_upper?: upper: lower;
    return (is_upper && upper) || (!is_upper && lower);
}

inline uint8_t interpolation_u8(uint8_t lower, uint8_t upper, uint16_t ratio) {
    return interpolation_temp1<uint8_t>(lower, upper, ratio);
}

inline int8_t interpolation_i8(int8_t lower, int8_t upper, uint16_t ratio) {
    return interpolation_temp1<int8_t>(lower, upper, ratio);
}

inline uint16_t interpolation_u16(uint16_t lower, uint16_t upper, uint16_t ratio) {
    return interpolation_temp1<uint16_t>(lower, upper, ratio);
}

inline int16_t interpolation_i16(int16_t lower, int16_t upper, uint16_t ratio) {
    return interpolation_temp1<int16_t>(lower, upper, ratio);
}

inline int32_t interpolation_i32(int32_t lower, int32_t upper, float ratio) {
    return interpolation_temp2<int32_t>(lower, upper, ratio);
}

inline uint32_t interpolation_u32(uint32_t lower, uint32_t upper, float ratio) {
    return interpolation_temp2<uint32_t>(lower, upper, ratio);
}

inline float interpolation_f32(float lower, float upper, float ratio) {
    return interpolation_temp2<float>(lower, upper, ratio);
}

#include "interpolation_ex.h"

#endif
