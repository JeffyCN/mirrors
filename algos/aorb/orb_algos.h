#ifndef _ORB_ALGOS_H_
#define _ORB_ALGOS_H_
#include "rk_aiq_types_orb_algo.h"

typedef  unsigned char  U8 ;
typedef  unsigned short U16 ;
typedef  unsigned int   U32 ;
typedef enum Byte { Uchar = 1, Float = 4, Rgb = 3} Byte;

typedef struct point_s {
    float row;
    float col;
} point_t;

typedef struct Mat {
    void* buffer; //buffer to save the image
    U16 height;
    U16 width;
    U8 channels;
    Byte bytes;
} Mat;

typedef struct Node {
    struct Node* next;
    void* data;
} Node;

typedef struct ORBList {
    struct Node* start;
    struct Node* end;
    U16 length;
    U32 bytes;
} ORBList;

ORBList* initList(U32 bytes);
void resetList(ORBList* list);
void freeList(ORBList* list);
#if 0
int push(ORBList* list, void* data);
orb_point_t* init_orbpoint(U16 row, U16 col, bool descriptor[ORB_FEATURE_DESCRIPTOR_BITS]);
orb_matched_point_t* init_matchpoints(U16 row1, U16 col1, U16 row2, U16 col2);
#endif
void freeList(ORBList* list);

ORBList* get_roi_points_list (rk_aiq_orb_algo_stat_t* keypoints, orb_rect_t roi);

ORBList* matching(ORBList* roi_points_list, rk_aiq_orb_algo_stat_t* keypoints2, orb_rect_t roi);

int find_homography_by_four_points(ORBList* matched_keypoints, double homography[9]);
int elimate_affine_transform(ORBList* matched_keypoints, double homography[9]);
orb_rect_t map_rect(double homography[9], orb_rect_t* rect_src);

void work_begin();
void work_end(const char* module_name);

#endif
