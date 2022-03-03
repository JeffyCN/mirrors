#include"interpolation.h"
#include "math.h"
#include <stdlib.h>

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
