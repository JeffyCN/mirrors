#ifndef _ORB_ALGOS_OPENCV_H_
#define _ORB_ALGOS_OPENCV_H_

#if OPENCV_SUPPORT
#include <vector>
#include "opencv2/opencv.hpp"

#define HOMOGRAPHY_MATRIX_SIZE 9

int push_orbpoint_cv(U32 num_points, U16* points, vector<Point2f> m_Points);
int get_homography_matrix(
    vector<Point2f> m_queryPoints,
    vector<Point2f> m_trainPoints,
    double* homographyMat,
    U32 size);
#endif
#endif
