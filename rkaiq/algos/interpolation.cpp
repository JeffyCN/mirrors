#include"interpolation.h"
#include "math.h"
#include <stdlib.h>

#define DEFAULT_ISO_STEP_MAX 13
const int default_iso_list[] = {50, 100, 200, 400, 800, 1600, 3200, 6400, 12800, 16300, 51200, 102400, 204800};

void pre_interp(int iso, int *iso_list, int num, int &lo, int &hi, float &ratio)
{
    int i,iso_lo, iso_hi;
    if (iso_list == NULL) {
        iso_list = (int *)default_iso_list;
        num = DEFAULT_ISO_STEP_MAX;
    }
    if (iso <= iso_list[0]) {
        lo = 0;
        hi = 0;
        ratio = 0.0f;
        return;
    }
    if (iso >= iso_list[num - 1]) {
        lo = num - 1;
        hi = num - 1;
        ratio = 0.0f;
        return;
    }

    for (i = 1; i < num; i++) {
        if (iso < iso_list[i])
        {
            lo = i - 1;
            hi = i;
            iso_lo = iso_list[lo];
            iso_hi = iso_list[hi];
            ratio = (float)(iso - iso_lo) / (iso_hi - iso_lo);

            return;
        }
    }
}

void interpolation(const float *x, const float *y, int Num, float x0, float*y0)
{
    int i, index;
    float k;
    if (x0 <= x[0])
    {
        k = y[0];
    }
    else if (x0 >= x[Num - 1])
    {
        k = y[Num - 1];
    }
    else
    {
        for (i = 0; i < Num; i++)
        {
            if (x0 < x[i])
                break;
        }

        index = i - 1;
        if ((float)x[index + 1] - (float)x[index] < 0.001)
            k = (float)y[index];
        else
            k = ((float)x0 - (float)x[index]) / ((float)x[index + 1] - (float)x[index])
                * ((float)y[index + 1] - (float)y[index])
                + (float)y[index];
    }

    *y0 = k;
}

void interpolation(const float *x, const unsigned short *y, int Num, float x0, unsigned short *y0)
{
    int i, index;
    float k;
    if (x0 <= x[0])
    {
        k = y[0];
    }
    else if (x0 >= x[Num - 1])
    {
        k = y[Num - 1];
    }
    else
    {
        for (i = 0; i < Num; i++)
        {
            if (x0 < x[i])
                break;
        }

        index = i - 1;
        if ((float)x[index + 1] - (float)x[index] < 0.001)
            k = (float)y[index];
        else
            k = ((float)x0 - (float)x[index]) / ((float)x[index + 1] - (float)x[index])
                * ((float)y[index + 1] - (float)y[index])
                + (float)y[index];
    }

    *y0 = (unsigned short)(k+0.5);
}

int interpolation(unsigned char *x, bool *y, int xNum, unsigned char x0, bool *y0)
{
    int i, index;
    bool k;
    if (x0 <= x[0] || x0 <= x[1])
    {
        k = y[0];
        index = 0;
    }
    else if (x0 >= x[xNum - 1])
    {
        k = y[xNum - 2];
        index = xNum - 2;
    }
    else {
        for (i = 0; i < xNum; i++)
        {
            if (x0 < x[i])
                break;
        }

        index = i - 1;
        if (abs(x[index + 1] - x0) > abs(x0 - x[index])) {
            k = y[index - 1];
        }
        else {
            k = y[index];
        }
    }
    *y0 = k;
    return(index);
}
