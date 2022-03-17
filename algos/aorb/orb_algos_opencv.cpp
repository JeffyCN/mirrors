#include "orb_algos_opencv.h"

#if OPENCV_SUPPORT
int
push_orbpoint_cv(U32 num_points, U16* points, vector<Point2f> m_Points) {
    Point2f point;
    m_Points.clear();
    vector<Point2f>(m_Points).swap(m_Points);

    for (int i = 0; i < num_points; i++) {
        point.x = points[i].x;
        point.y = points[i].y;

        m_Points.push_back(point);
    }
    return 0;
}

int
get_homography_matrix(
    vector<Point2f> m_queryPoints,
    vector<Point2f> m_trainPoints,
    double* homographyMat,
    U32 size)
{
    double reprojectionThreshold = 10;

    Mat homography = findHomography(m_queryPoints, m_trainPoints, RANSAC,
                                    reprojectionThreshold, noArray(), 2000, 0.995);
    double* data = (double*)homography.data;

    for (int i = 0; i < size; i++) {
        homographyMat[i] = data[i];
        //LOGI_ORB("%s: feature point match ret: %f\n", __FUNCTION__, data[i]);
    }
    return 0;
}
#endif
