
#ifndef __INTERPOLATION_H__
#define __INTERPOLATION_H__

void interpolation(const float *x, const float *y, int Num, float x0, float*y0);
void interpolation(const float *x, const unsigned short *y, int Num, float x0, unsigned short *y0);
int interpolation(unsigned char *x, bool *y, int xNum, unsigned char x0, bool *y0);

#endif
