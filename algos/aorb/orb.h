#ifndef __ORB_H__
#define __ORB_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MAX_POINTS 40000

#define FAST_CANDIDATE 16 // fast9 use surrounding 16 points
#define MAX_CONSECUTIVE 9
#define PATCH_SIZE 15
#define HALF_PATCH_SIZE (PATCH_SIZE >> 1)
#define DESCRIPTOR_SIZE 15

#define DARKER    1
#define SIMILAR   2
#define BRIGHTER  3
#define ROUND(x) (int)(((x) >= 0)?((x) + 0.5) : ((x) - 0.5))

class rkisp_orb
{
public:
    unsigned char fast9_limit;
    unsigned char cur_limit;
    int feature_size;
    int feature_capacity;
    int non_max_sup_radius;

    unsigned char* imgSrc;
    int width;
    int height;
    int data_bits;
    unsigned short* pXs;
    unsigned short* pYs;
    unsigned char*  pDescriptors;
    unsigned char* pScores;
public:
    int process();
    int fast9_detect();
    int compare_pixel(unsigned char center, unsigned char* compare, unsigned char* candidate);
    int find_9consecutive_pixel(unsigned char* compare);
    int non_max_suppress();
    int orb_feature();
    int get_rotation_value(unsigned char* center_ptr, int* cur_point_pattern, int idx, int cos_value, int sin_value);
};


#endif
