#ifndef __INTERPOLATION_EX_H__
#define __INTERPOLATION_EX_H__

#include <stdlib.h>
#include <typeinfo>

template<typename S, typename D>
void interpolation_ex(const S *x, int Num, S x0, D* y, D* y0, size_t y_stride = sizeof(D));

// support struct interpolation, data type could be bool, uint16_t, uint32_t, float
template<typename S, typename D>
void interpolation_ex(const S *x, int Num, S x0, D* y, D* y0, size_t y_stride)
{
    int i, index;

    if (Num <= 0)
        return;

    if (!x)
        return;

    if (x0 <= x[0])
    {
        *y0 = *y;
    }
    else if (x0 >= x[Num - 1])
    {
        *y0 = *(D*)((char*)y+(Num - 1) * y_stride);
    }
    else
    {
        for (i = 0; i < Num; i++)
        {
            if (x0 < x[i])
                break;
        }

        index = i - 1;
        D y_ind = *(D*)((char*)y + index * y_stride);

        if ((float)x[index + 1] - (float)x[index] < 0.001) {
            *y0 = y_ind;
        } else {
            float ratio = ((float)x0 - (float)x[index]) / ((float)x[index + 1] - (float)x[index]);
            D y_ind1 = *(D*)((char*)y + (index+1) * y_stride);

            if (typeid(D) == typeid(bool)) {
                *y0 = ratio >= 0.5f ? y_ind1 : y_ind;
            } else {
                float k = ratio * (y_ind1 - y_ind) + y_ind;

                if ((typeid(D) == typeid(float)) || typeid(D) == typeid(double))
                    *y0 = k;
                else
                    *y0 = (D)(k+0.5);
            }
        }
    }
}

#endif
