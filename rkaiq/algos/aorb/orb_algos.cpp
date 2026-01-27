#include "orb_algos.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "xcam_log.h"

#define EPS 1e-10

#include <time.h>
double begin_tick;
double end_tick;

unsigned long getTime()
{
    struct timespec ts;
    clock_gettime(1, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

void work_begin()
{
    begin_tick = getTime();
}

void work_end(const char* module_name)
{
    end_tick = getTime() - begin_tick;
    LOGE_AORB("[%s] TIME = %lf ms \n", module_name, end_tick);
}

ORBList* initList(U32 bytes) {
    ORBList* list = (ORBList*)malloc(sizeof(ORBList));
    list->bytes = bytes;
    list->start = NULL;
    list->end = NULL;
    list->length = 0;
    return list;
}

void freeList(ORBList* list) {
    if (list != NULL) {
        for(Node* item = list->start; item != NULL; item = item->next) {
            if (item->data != NULL) {
                free(item->data);
                item->data = NULL;
            }
        }
        free(list);
        list = NULL;
    }
}

int push(ORBList* list, void* data) {

    Node* node = (Node*)malloc(sizeof(Node));
    node->data = malloc(list->bytes);
    node->next = NULL;
    memcpy(node->data, data, list->bytes);
    if (list->start == NULL) {
        list->start = node;
        list->end = node;
    } else {
        list->end->next = node;
        list->end = node;
    }

    list->length++;
    return list->length;
}

void init_matchpoints(orb_matched_point_t* point, U32 row1, U32 col1, U32 row2, U32 col2, U32 score) {
    if (point == NULL) {
        return;
    }

    point->col1 = col1;
    point->row1 = row1;
    point->col2 = col2;
    point->row2 = row2;
    point->score = score;
}

ORBList* get_roi_points_list (rk_aiq_orb_algo_stat_t* keypoints, orb_rect_t roi)
{
    ORBList* roi_points_list = initList(sizeof(rk_aiq_orb_featue_point));

    for (unsigned int i = 0; i < keypoints->num_points; i++) {
        rk_aiq_orb_featue_point* point = keypoints->points + i;
        if (point->x < roi.left ||
                point->x > roi.right ||
                point->y < roi.top ||
                point->y > roi.bottom) {
            continue;
        }

        push(roi_points_list, point);
    }

#if 1
    FILE* fp = fopen("/tmp/points0.txt", "wb");
    for (unsigned int i = 0; i < keypoints->num_points; i++) {
        rk_aiq_orb_featue_point* point = keypoints->points + i;
        fprintf(fp, "%d, %d\n", point->x, point->y);
    }
    fflush(fp);
    fclose(fp);
#endif
    return roi_points_list;
}

ORBList* matching(ORBList* roi_points_list, rk_aiq_orb_algo_stat_t* keypoints2, orb_rect_t roi) {
    orb_matched_point_t keypoint{};
    ORBList* matched_list = initList(sizeof(orb_matched_point_t));

    for(Node* point = roi_points_list->start; point != NULL; point = point->next) {
        rk_aiq_orb_featue_point* descriptor1 = (rk_aiq_orb_featue_point*)point->data;
        keypoint.score = 0;

        for (unsigned int j = 0; j < keypoints2->num_points; j++) {
            rk_aiq_orb_featue_point* descriptor2 = keypoints2->points + j;

            unsigned int success_num = 120;
            for(int k = 0; k < DESCRIPTOR_SIZE; k++) {
                U8 xor_val = descriptor1->brief[k] ^ descriptor2->brief[k];
                while (xor_val) {
                    if (xor_val & 1) {
                        success_num--;
                    }
                    xor_val = xor_val >> 1;
                }
            }

            if(success_num > 110) { //120
                if (success_num > keypoint.score) {
                    init_matchpoints(&keypoint,
                                     descriptor1->y, descriptor1->x,
                                     descriptor2->y, descriptor2->x,
                                     success_num);
                }

                //orb_matched_point_t* f = (orb_matched_point_t*)(matched_list->start);
                //LOGE_AORB("found match: (%d-%d)-(%d-%d)",
                //    f->col1, f->row1, f->col2, f->row2);
            }
        }

        if (keypoint.score > 0) {
            push(matched_list, &keypoint);
        }

        if (matched_list->length > 30) {
            break;
        }
    }

#if 1
    FILE* fp = fopen("/tmp/points1.txt", "wb");
    for (unsigned int i = 0; i < keypoints2->num_points; i++) {
        rk_aiq_orb_featue_point* point = keypoints2->points + i;
        fprintf(fp, "%d, %d\n", point->x, point->y);
    }
    fflush(fp);
    fclose(fp);
#endif

    return matched_list;
}

void gaussian_elimination(double *input, int n)
{
    double * A = input;
    int i = 0;
    int j = 0;
    //m = 8 rows, n = 9 cols
    int m = n - 1;
    while (i < m && j < n)
    {
        // Find pivot in column j, starting in row i:
        int maxi = i;
        for(int k = i + 1; k < m; k++)
        {
            if(fabs(A[k * n + j]) > fabs(A[maxi * n + j]))
            {
                maxi = k;
            }
        }
        if (A[maxi * n + j] != 0)
        {
            //swap rows i and maxi, but do not change the value of i
            if(i != maxi)
                for(int k = 0; k < n; k++)
                {
                    float aux = A[i * n + k];
                    A[i * n + k] = A[maxi * n + k];
                    A[maxi * n + k] = aux;
                }
            //Now A[i,j] will contain the old value of A[maxi,j].
            //divide each entry in row i by A[i,j]
            float A_ij = A[i * n + j];
            for(int k = 0; k < n; k++)
            {
                A[i * n + k] /= A_ij;
            }
            //Now A[i,j] will have the value 1.
            for(int u = i + 1; u < m; u++)
            {
                //subtract A[u,j] * row i from row u
                float A_uj = A[u * n + j];
                for(int k = 0; k < n; k++)
                {
                    A[u * n + k] -= A_uj * A[i * n + k];
                }
                //Now A[u,j] will be 0, since A[u,j] - A[i,j] * A[u,j] = A[u,j] - 1 * A[u,j] = 0.
            }
            i++;
        }
        j++;
    }

    //back substitution
    for(int i = m - 2; i >= 0; i--)
    {
        for(int j = i + 1; j < n - 1; j++)
        {
            A[i * n + m] -= A[i * n + j] * A[j * n + m];
        }
    }
}

int get_trusted_four_points(ORBList* matched_keypoints, point_t src[4], point_t dst[4])
{
    point_t center{};

    if (matched_keypoints->length < 4) {
        return -1;
    }

    for(Node* point = matched_keypoints->start; point != NULL; point = point->next) {
        orb_matched_point_t* keypoints = (orb_matched_point_t*)point->data;
        center.col += keypoints->col1;
        center.row += keypoints->row1;
    }

    center.col = center.col / matched_keypoints->length;
    center.row = center.row / matched_keypoints->length;

    int subx, suby, dist;
    int max_dist_lt = 0;
    int max_dist_rt = 0;
    int max_dist_lb = 0;
    int max_dist_rb = 0;
    bool found_lt, found_rt, found_lb, found_rb;

    found_lt = found_rt = found_lb = found_rb = false;
    for(Node* point = matched_keypoints->start; point != NULL; point = point->next) {
        orb_matched_point_t* keypoints = (orb_matched_point_t*)point->data;
        if (keypoints->col1 < center.col &&
                keypoints->row1 < center.row) {
            subx = keypoints->col1 - center.col;
            suby = keypoints->row1 - center.row;
            dist = subx * subx + suby * suby;
            if (dist > max_dist_lt) {
                found_lt = true;
                max_dist_lt = dist;
                src[0].col = keypoints->col1;
                src[0].row = keypoints->row1;
                dst[0].col = keypoints->col2;
                dst[0].row = keypoints->row2;
            }
        }

        if (keypoints->col1 > center.col &&
                keypoints->row1 < center.row) {
            subx = keypoints->col1 - center.col;
            suby = keypoints->row1 - center.row;
            dist = subx * subx + suby * suby;
            if (dist > max_dist_rt) {
                found_rt = true;
                max_dist_rt = dist;
                src[1].col = keypoints->col1;
                src[1].row = keypoints->row1;
                dst[1].col = keypoints->col2;
                dst[1].row = keypoints->row2;
            }
        }

        if (keypoints->col1 < center.col &&
                keypoints->row1 > center.row) {
            subx = keypoints->col1 - center.col;
            suby = keypoints->row1 - center.row;
            dist = subx * subx + suby * suby;
            if (dist > max_dist_lb) {
                found_lb = true;
                max_dist_lb = dist;
                src[2].col = keypoints->col1;
                src[2].row = keypoints->row1;
                dst[2].col = keypoints->col2;
                dst[2].row = keypoints->row2;
            }
        }

        if (keypoints->col1 > center.col &&
                keypoints->row1 > center.row) {
            subx = keypoints->col1 - center.col;
            suby = keypoints->row1 - center.row;
            dist = subx * subx + suby * suby;
            if (dist > max_dist_rb) {
                found_rb = true;
                max_dist_rb = dist;
                src[3].col = keypoints->col1;
                src[3].row = keypoints->row1;
                dst[3].col = keypoints->col2;
                dst[3].row = keypoints->row2;
            }
        }
    }

    if (!found_lt || !found_rt || !found_lb || !found_rb) {
        return -1;
    }

    return 0;
}

int find_homography_by_four_points(ORBList* matched_keypoints, double homography[9])
{
    int ret = 0;
    point_t src[4]{};
    point_t dst[4]{};

    ret = get_trusted_four_points(matched_keypoints, src, dst);
    if (ret < 0) {
        return -1;
    }

    // create the equation system to be solved
    // src and dst must implement [] operator for point access
    //
    // from: Multiple View Geometry in Computer Vision 2ed
    //       Hartley R. and Zisserman A.
    //
    // x' = xH
    // where H is the homography: a 3 by 3 matrix
    // that transformed to inhomogeneous coordinates for each point
    // gives the following equations for each point:
    //
    // x' * (h31*x + h32*y + h33) = h11*x + h12*y + h13
    // y' * (h31*x + h32*y + h33) = h21*x + h22*y + h23
    //
    // as the homography is scale independent we can let h33 be 1 (indeed any of the terms)
    // so for 4 points we have 8 equations for 8 terms to solve: h11 - h32
    // after ordering the terms it gives the following matrix
    // that can be solved with gaussian elimination:

    double P[8][9] =
    {
        {-src[0].col, -src[0].row, -1,   0,   0,  0, src[0].col * dst[0].col, src[0].row * dst[0].col, -dst[0].col }, // h11
        {  0,   0,  0, -src[0].col, -src[0].row, -1, src[0].col * dst[0].row, src[0].row * dst[0].row, -dst[0].row }, // h12

        {-src[1].col, -src[1].row, -1,   0,   0,  0, src[1].col * dst[1].col, src[1].row * dst[1].col, -dst[1].col }, // h13
        {  0,   0,  0, -src[1].col, -src[1].row, -1, src[1].col * dst[1].row, src[1].row * dst[1].row, -dst[1].row }, // h21

        {-src[2].col, -src[2].row, -1,   0,   0,  0, src[2].col * dst[2].col, src[2].row * dst[2].col, -dst[2].col }, // h22
        {  0,   0,  0, -src[2].col, -src[2].row, -1, src[2].col * dst[2].row, src[2].row * dst[2].row, -dst[2].row }, // h23

        {-src[3].col, -src[3].row, -1,   0,   0,  0, src[3].col * dst[3].col, src[3].row * dst[3].col, -dst[3].col }, // h31
        {  0,   0,  0, -src[3].col, -src[3].row, -1, src[3].col * dst[3].row, src[3].row * dst[3].row, -dst[3].row }, // h32
    };

    gaussian_elimination((&P[0][0]), 9);

    // gaussian elimination gives the results of the equation system
    // in the last column of the original matrix.
    // opengl needs the transposed 4x4 matrix:
    double aux_H[] = { P[0][8], P[1][8], P[2][8], // h11  h21 0 h31
                       P[3][8], P[4][8], P[5][8],  // h12  h22 0 h32
                       P[6][8], P[7][8], 1
                     };       // h13  h23 0 h33

    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            homography[i * 3 + j] = aux_H[i * 3 + j];
        }
    }

    return 0;
}

void get_affine_matrix(double m[3][5], int dim, double affineM[9]) {
    int index = 0;
    char res[1024] = {0};

    strcat(res, "\n");
    for(int j = 0; j < dim; j++) {
        char str1[128];
        sprintf(str1, "x%d' = ", j);
        strcat(res, str1);
        for(int i = 0; i < dim; i++) {
            char str2[128];
            sprintf(str2, "x%d * %f + ", i, m[i][j + dim + 1]);
            strcat(res, str2);

            affineM[index] = m[i][j + dim + 1];
            index++;
        }
        char str3[128];
        sprintf(str3, "%f\n", m[dim][j + dim + 1]);
        strcat(res, str3);

        affineM[index] = m[dim][j + dim + 1];
        index++;
    }

    affineM[8] = 1;

    LOGE_ORB("\n-------------------------------------\n");
    LOGE_ORB("%s", res);
}

bool gauss_jordan(double m[3][5], int length, int dim) {
    int h = length;
    int w = dim;
    /*
        LOGV_ORB("gauss1: \n[[%f, %f, %f, %f, %f]\n[%f, %f, %f, %f, %f]\n[%f, %f, %f, %f, %f]]\n",
            m[0][0], m[0][1], m[0][2], m[0][3], m[0][4],
            m[1][0], m[1][1], m[1][2], m[1][3], m[1][4],
            m[2][0], m[2][1], m[2][2], m[2][3], m[2][4]);
    */
    for(int y = 0; y < h; y++) {
        int maxrow = y;
        for(int y2 = y + 1; y2 < h; y2++) {
            if(fabs(m[y2][y]) > fabs(m[maxrow][y])) {
                maxrow = y2;
            }
        }

        //(m[y], m[maxrow]) = (m[maxrow], m[y])
        if (y != maxrow) {
            for(int i = 0; i < w; i++) {
                double temp = m[y][i];
                m[y][i] = m[maxrow][i];
                m[maxrow][i] = temp;
            }
        }

        if(fabs(m[y][y]) <= EPS) {
            return false;
        }

        for(int y2 = y + 1; y2 < h; y2++) {
            double c = m[y2][y] / m[y][y];
            for(int x = y; x < w; x++) {
                m[y2][x] -= m[y][x] * c;
            }
        }
    }

    for(int y = h - 1; y > -1; y--) {
        double c = m[y][y];
        for(int y2 = 0; y2 < y; y2++) {
            for(int x = w - 1; x > y - 1; x--) {
                m[y2][x] -= m[y][x] * m[y2][y] / c;
            }
        }
        m[y][y] /= c;
        for(int x = h; x < w; x++) {
            m[y][x] /= c;
        }
    }

    return true;
}

int affine_fit(double fpt[][2], double tpt[][2], int length, int dim, double affineM[9]) {
    if (length <= dim) {
        LOGE_ORB("number of points must bigger than dimension\n");
        return -1;
    }

    // dim+1 x dim
    double c[3][2]{};
    for(int j = 0; j < dim; j++) {
        for(int k = 0; k < (dim + 1); k++) {
            for(int i = 0; i < length; i++) {
                double qt[3] = {fpt[i][0], fpt[i][1], 1};
                c[k][j] += qt[k] * tpt[i][j];
            }
        }
    }

    // dim+1 x dim+1
    double Q[3][3]{};
    for(int k = 0; k < length; k++) {
        double qt[3] = {fpt[k][0], fpt[k][1], 1};
        for(int i = 0; i < (dim + 1); i++) {
            for(int j = 0; j < (dim + 1); j++) {
                Q[i][j] += qt[i] * qt[j];
            }
        }
    }

    double M[3][5];
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            M[i][j] = Q[i][j];
        }
    }

    for(int i = 0; i < 3; i++) {
        for(int j = 3; j < 5; j++) {
            M[i][j] = c[i][j - 3];
        }
    }

    gauss_jordan(M, 3, 5);
    get_affine_matrix(M, 2, affineM);

    return 0;
}

int elimate_affine_transform(ORBList* matched_keypoints, double homography[9])
{
    int ret = 0;
    int i = 0, j = 0, k = 0;

    if (matched_keypoints->length < 3) {
        LOGE_ORB("matched keypoints num(%d) not enough", matched_keypoints->length);
        k = 0;
        for(Node* point = matched_keypoints->start; point != NULL; point = point->next) {
            orb_matched_point_t* keypoint = (orb_matched_point_t*)point->data;
            LOGE_AORB("<%d>-[%d - %d]-[%d - %d]",
                      keypoint->score,
                      keypoint->col1, keypoint->row1,
                      keypoint->col2, keypoint->row2);
            k++;
        }
        return -1;
    }

    int max, count = 0;
    int maxLength = 10;
    int subX, subY;
    int length = matched_keypoints->length > maxLength ? maxLength : matched_keypoints->length;
    int distance = -1, distanceAvg = 0;
    int* distanceArray = (int*)malloc(sizeof(int) * matched_keypoints->length);
    int* distanceSortedArray = (int*)malloc(sizeof(int) * matched_keypoints->length);
#if 0 // fix compile error for Android
    int (*M0_stats)[matched_keypoints->length] =
        (int (*)[matched_keypoints->length])malloc(sizeof(int) * matched_keypoints->length * 2);
    for (k = 0; k < matched_keypoints->length; k++) {
        M0_stats[0][k] = M0_stats[1][k] = -1;
    }
#else
    int* M0_stats[2];
    for (int i = 0; i < 2 ;i++)
        M0_stats[i] = (int*)malloc(sizeof(int) * matched_keypoints->length);
     for (k = 0; k < matched_keypoints->length; k++) {
         M0_stats[0][k] = M0_stats[1][k] = -1;
     }
#endif
    double (*from_pts)[2];
    from_pts = (double (*)[2])malloc(sizeof(double) * length * 2);

    double (*to_pts)[2];
    to_pts = (double (*)[2])malloc(sizeof(double) * length * 2);


    LOGD_ORB("matched points:");
    //work_begin();
    for(Node* point = matched_keypoints->start; point != NULL; point = point->next) {
        orb_matched_point_t* keypoint = (orb_matched_point_t*)point->data;

        subX = (keypoint->col1 - keypoint->col2);
        subY = (keypoint->row1 - keypoint->row2);
        distanceArray[i] = subX * subX + subY * subY;
        distanceSortedArray[i] = distanceArray[i];
        distanceAvg += distanceArray[i];
        i++;
    }

    distanceAvg /= matched_keypoints->length;

    //sort
    for (k = 0; k < matched_keypoints->length; k++) {
        for (j = k; j < matched_keypoints->length; j++) {
            if (distanceSortedArray[k] < distanceSortedArray[j]) {
                max = distanceSortedArray[j];
                distanceSortedArray[j] = distanceSortedArray[k];
                distanceSortedArray[k] = max;
            }
        }
    }

    //find mode
    for (k = 0, j = 0; k < matched_keypoints->length - 1; k++) {
        if (distanceSortedArray[k] == distanceSortedArray[k + 1]) {
            M0_stats[0][j] = distanceSortedArray[k];
            if (M0_stats[1][j] == -1) {
                M0_stats[1][j] = 2;
            } else {
                M0_stats[1][j] += 1;
            }
        } else {
            if(M0_stats[0][j] == -1) {
                M0_stats[0][j] = distanceSortedArray[k];
                M0_stats[1][j] = 1;
            }
            j++;

            if (k == matched_keypoints->length - 2) {
                M0_stats[0][j] = distanceSortedArray[k + 1];
                M0_stats[1][j] = 1;
            }
        }
    }

    for (k = 1; k < matched_keypoints->length; k++) {
        LOGE_AORB("distanceSortedArray[%d]: %d", k, distanceSortedArray[k]);

    }

    max = M0_stats[1][0];
    distance = M0_stats[0][0];
    for (k = 1; k < matched_keypoints->length; k++) {
        LOGE_AORB("M0_stats[%d]: %d, count: %d", k, M0_stats[0][k], M0_stats[1][k]);
        if (M0_stats[0][k] == -1) {
            break;
        }

        if (M0_stats[1][k] >= max) {
            max = M0_stats[1][k];
            distance = M0_stats[0][k];
        }
    }

    LOGE_AORB("mesured distance: %d, count: %d", distance, max);

    for (k = 0; k < matched_keypoints->length; k++) {
        if (abs(distanceArray[k] - distance) > distance * 10) {
            distanceArray[k] = -1;
        }
    }

    //work_end("calc-dist");
    //work_begin();

    i = 0;
    k = 0;
    for(Node* point = matched_keypoints->start; point != NULL && i < maxLength; point = point->next) {
        orb_matched_point_t* keypoint = (orb_matched_point_t*)point->data;

        if (distanceArray[k] == -1) {
            k++;
            continue;
        }

        from_pts[i][0] = keypoint->col1;
        from_pts[i][1] = keypoint->row1;
        to_pts[i][0] = keypoint->col2;
        to_pts[i][1] = keypoint->row2;
        i++;
        k++;
    }

    if (i < 3) {
        k = 0;
        for(Node* point = matched_keypoints->start; point != NULL; point = point->next) {
            orb_matched_point_t* keypoint = (orb_matched_point_t*)point->data;
            LOGE_AORB("<%d>-[%d - %d]-[%d - %d]-<%d>",
                      keypoint->score,
                      keypoint->col1, keypoint->row1,
                      keypoint->col2, keypoint->row2,
                      distanceArray[k]);
            k++;
        }
        LOGE_ORB("valid matched keypoints %d less than 6", i);
        ret = -2;
        goto out;
        //return -2;
    }

    ret = affine_fit(from_pts, to_pts, i, 2, homography);
    //work_end("calc-affine");

out:
    free(from_pts);
    free(to_pts);
    free(distanceArray);
    free(distanceSortedArray);
#if 0 // fix compile error for Android
    free(M0_stats);
#else
    free(M0_stats[0]);
    free(M0_stats[1]);
#endif
    return ret;
}


void map_point(double mat[9], point_t* point_src, point_t* point_dst)
{
    // P[0], P[1], P[2],
    // P[3], P[4], P[5],
    // P[6], P[7], P[8]
    double D = point_src->col * mat[6] + point_src->row * mat[7] + mat[8];
    point_dst->col = (point_src->col * mat[0] + point_src->row * mat[1] + mat[2]) / D;
    point_dst->row = (point_src->col * mat[3] + point_src->row * mat[4] + mat[5]) / D;

}

orb_rect_t map_rect(double homography[9], orb_rect_t* rect_src)
{
    orb_rect_t rect_dst;
    point_t src, dst;
    src.col = rect_src->left;
    src.row = rect_src->top;
    map_point(homography, &src, &dst);
    rect_dst.left = dst.col;
    rect_dst.top = dst.row;

    src.col = rect_src->right;
    src.row = rect_src->bottom;
    map_point(homography, &src, &dst);
    rect_dst.right = dst.col;
    rect_dst.bottom = dst.row;

    rect_dst.width = rect_dst.right - rect_dst.left;
    rect_dst.height = rect_dst.bottom - rect_dst.top;

    return rect_dst;
}

