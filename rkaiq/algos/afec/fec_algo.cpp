#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#if OPENCV_SUPPORT
#include <Eigen/Core>
#include <opencv2/opencv.hpp>
#define MIN_VALUE 2.2204e-16                        //������Ҫ�������ֵ
#define IS_DOUBLE_ZERO(d) ((d < MIN_VALUE)&&((-d)<MIN_VALUE))
/* ���۲��� */
struct CameraCoeff
{
    double cx, cy;                  // �����ͷ�Ĺ���
    double a0, a2, a3, a4;          // ���۾�ͷ�Ļ���ϵ��
    double sf;                      // sf�����ӽǣ�sfԽ���ӽ�Խ��
    double invpol[8];
    double big_rho[2002];           // Ԥ�����ɵ�tan(theta)��rho�Ķ�Ӧ��
    double small_rho[2001];         // Ԥ�����ɵ�cot(theta)��rho�Ķ�Ӧ��
    double Z[5000];
};

void SmallMeshCorrect(int imgWidth, int imgHeight, int meshSizeW, int meshSizeH, CameraCoeff camCoeff, double *mapx, double *mapy, unsigned short *pMeshXY)
{
    double cx = camCoeff.cx;
    double cy = camCoeff.cy;
    double a0;
    a0 = camCoeff.a0;

    double *invpol = camCoeff.invpol;
    double sf = camCoeff.sf;// sf�����ӽǣ�sfԽ���ӽ�Խ��
    double Nz = a0 / sf;
    double x1, y1, rho_1;
    double theta_1, r, t_i;
    double x2, y2;
    int index = 0;
    double temp;
    int x_cnt = 0;
    int y_cnt = 2 * meshSizeH * meshSizeW;
    cv::Mat theta_rho = cv::Mat(meshSizeH, meshSizeW, CV_64FC1);
    cv::Mat mapX = cv::Mat(meshSizeH, meshSizeW, CV_64FC1);
    cv::Mat mapY = cv::Mat(meshSizeH, meshSizeW, CV_64FC1);
    for (int j = 0; j < meshSizeH; ++j)
    {
        for (int i = 0; i < meshSizeW; ++i)
        {
            /* ��У�����귴�Ƶ��������� */
            x1 = mapx[index] - cx;
            y1 = mapy[index] - cy;
            rho_1 = sqrt(x1 * x1 + y1 * y1);
            theta_1 = atan(Nz / rho_1);
            theta_rho.at<double>(j, i) = theta_1;

            if (IS_DOUBLE_ZERO(rho_1))
            {
                x2 = cx;
                y2 = cy;
            }
            else
            {
                r = invpol[0];
                t_i = 1;
                for (int k = 1; k < 8; ++k)
                {
                    t_i = t_i * theta_1;
                    r = r + invpol[k] * t_i;
                }
                x2 = (x1 / rho_1) * r + cx;
                y2 = (y1 / rho_1) * r + cy;
            }
            mapX.at<double>(j, i) = x2;
            mapY.at<double>(j, i) = y2;
            // ���Ʊ߽�
            if (x2 < 0) {
                x2 = 0;
            }
            if (x2 > (imgWidth - 1)) {
                x2 = (imgWidth - 1);    // 1919
            }
            if (y2 < 0) {
                y2 = 0;
            }
            if (y2 > (imgHeight - 1)) {
                y2 = (imgHeight - 1);    // 1079
            }

            // X���궨�㻯
            pMeshXY[x_cnt] = (unsigned short)x2;
            temp = x2 - pMeshXY[x_cnt];
            pMeshXY[x_cnt + 1] = unsigned short(temp * 128);
            x_cnt = x_cnt + 2;

            // Y���궨�㻯
            pMeshXY[y_cnt] = (unsigned short)y2;
            temp = y2 - pMeshXY[y_cnt];
            pMeshXY[y_cnt + 1] = unsigned short(temp * 128);
            y_cnt = y_cnt + 2;
            ++index;
        }
    }
    cv::Mat meshXY = cv::Mat(meshSizeH, meshSizeW, CV_8UC1, pMeshXY);
}
void GenMeshTable(int imgWidth, int imgHeight, int meshStepW, int meshStepH, int meshSizeW, int meshSizeH, CameraCoeff camCoeff,
                  unsigned short  *pMeshXY, unsigned short    *pMeshXI, unsigned short    *pMeshYI, unsigned char *pMeshXF, unsigned char *pMeshYF)
{
    double stride_w = meshStepW;
    double stride_h = meshStepH;


    /* �����mesh���� */
    double *mapx = new double[meshSizeW * meshSizeH];
    double *mapy = new double[meshSizeW * meshSizeH];
    double *mapz = new double[meshSizeW * meshSizeH];
    // ��ʼ��
    double start_w = 0;
    double start_h = 0;

    double a, b;
    int index = 0;
    b = start_h;
    for (int j = 0; j < meshSizeH; ++j, b = b + stride_h)
    {
        a = start_w;
        for (int i = 0; i < meshSizeW; ++i, a = a + stride_w)
        {
            mapx[index] = a;
            mapy[index] = b;
            mapz[index] = (double)1;
            ++index;
        }
    }

    /* ����У�����meshXY */
    SmallMeshCorrect(imgWidth, imgHeight, meshSizeW, meshSizeH, camCoeff, mapx, mapy, pMeshXY);

    /* ����pMeshXI��pMeshXF��pMeshYI��pMeshYF */
    unsigned short *pTmpXi = (unsigned short *)&pMeshXY[0];
    unsigned short *pTmpXf = (unsigned short *)&pMeshXY[1];
    unsigned short *pTmpYi = (unsigned short *)&pMeshXY[meshSizeW * meshSizeH * 2];
    unsigned short *pTmpYf = (unsigned short *)&pMeshXY[meshSizeW * meshSizeH * 2 + 1];

    for (int i = 0; i < meshSizeW * meshSizeH; i++)
    {
        pMeshXI[i] = *pTmpXi;
        pTmpXi += 2;
        pMeshXF[i] = (unsigned char) * pTmpXf;
        pTmpXf += 2;
        pMeshYI[i] = *pTmpYi;
        pTmpYi += 2;
        pMeshYF[i] = (unsigned char) * pTmpYf;
        pTmpYf += 2;
    }

    /* ����Ϊbin�ļ� */
    /* MeshXY.bin */
    FILE *fpMeshXY = fopen("MeshXY.bin", "wb");
    if (fpMeshXY == NULL) {
        printf("MeshXY.bin open error!!!");
        goto err;
    }
    fwrite(&imgWidth, sizeof(unsigned short), 1, fpMeshXY);
    fwrite(&imgHeight, sizeof(unsigned short), 1, fpMeshXY);
    fwrite(&meshSizeW, sizeof(unsigned short), 1, fpMeshXY);
    fwrite(&meshSizeH, sizeof(unsigned short), 1, fpMeshXY);
    fwrite(&meshStepW, sizeof(unsigned short), 1, fpMeshXY);
    fwrite(&meshStepH, sizeof(unsigned short), 1, fpMeshXY);
    fwrite(pMeshXY, sizeof(unsigned short), meshSizeW * meshSizeH * 2 * 2, fpMeshXY);
    if (fclose(fpMeshXY) != 0) {
        printf("MeshXY.bin close error!!!");
        goto err;
    }
    /* MeshXI.bin */
    FILE *fpMeshXI = fopen("MeshXI.bin", "wb");
    if (fpMeshXI == NULL) {
        printf("MeshXI.bin open error!!!");
        goto err;
    }
    fwrite(pMeshXI, sizeof(unsigned short), meshSizeW * meshSizeH, fpMeshXI);
    if (fclose(fpMeshXI) != 0) {
        printf("MeshXI.bin close error!!!");
        goto err;
    }
    /* MeshXF.bin */
    FILE *fpMeshXF = fopen("MeshXF.bin", "wb");
    if (fpMeshXF == NULL) {
        printf("MeshXF.bin open error!!!");
        goto err;
    }
    fwrite(pMeshXF, sizeof(unsigned char), meshSizeW * meshSizeH, fpMeshXF);
    if (fclose(fpMeshXF) != 0) {
        printf("MeshXF.bin close error!!!");
        goto err;
    }
    /* MeshYI.bin */
    FILE *fpMeshYI = fopen("MeshYI.bin", "wb");
    if (fpMeshYI == NULL) {
        printf("MeshYI.bin open error!!!");
        goto err;
    }
    fwrite(pMeshYI, sizeof(unsigned short), meshSizeW * meshSizeH, fpMeshYI);
    if (fclose(fpMeshYI) != 0) {
        printf("MeshYI.bin close error!!!");
        goto err;
    }
    /* MeshYF.bin */
    FILE *fpMeshYF = fopen("MeshYF.bin", "wb");
    if (fpMeshYF == NULL) {
        printf("MeshYF.bin open error!!!");
        goto err;
    }
    fwrite(pMeshYF, sizeof(unsigned char), meshSizeW * meshSizeH, fpMeshYF);
    if (fclose(fpMeshYF) != 0) {
        printf("MeshYF.bin close error!!!");
        goto err;
    }

err:
    delete[] mapx;
    delete[] mapy;
    delete[] mapz;
}

int gen_default_mesh_table(int imgWidth, int imgHeight, int mesh_density,
                           unsigned short* pMeshXI, unsigned short* pMeshYI, unsigned char* pMeshXF, unsigned char* pMeshYF)
{
    /* ���ݻ����������mesh�� */
    CameraCoeff camCoeff;
    camCoeff.cx = 951.813257;
    camCoeff.cy = 700.761832;
    camCoeff.a0 = -1.547500405530367e+03;
    camCoeff.a2 = 3.399953844687639e-04;
    camCoeff.a3 = -9.201736312511578e-08;
    camCoeff.a4 = 6.793998839476364e-11;
    camCoeff.sf = 1;

    camCoeff.invpol[0] = 1350.42993320407;
    camCoeff.invpol[1] = 414.478976267835;
    camCoeff.invpol[2] = -871.475096726774;
    camCoeff.invpol[3] = -1775.87581281456;
    camCoeff.invpol[4] = -2556.70096229938;
    camCoeff.invpol[5] = -1941.21355417283;
    camCoeff.invpol[6] = -735.900048505591;
    camCoeff.invpol[7] = -111.196712124121;
#if 0
    int meshStepW = 16;
    int meshStepH = 8;
    if (imgWidth > 1920)
    {
        int mapStepW = 16;
        int mapStepH = 8;
    }

    int imgWidth = 1920;
    int imgHeight = 1080;
    int meshSizeW = imgWidth / meshStepW + 1;       // ��1920->121
    int meshSizeH = imgHeight / meshStepH + 1;      // ��1080->136
    unsigned short  *pMeshXY;               // remap����
    unsigned short  *pMeshXI;
    unsigned short  *pMeshYI;
    unsigned char   *pMeshXF;
    unsigned char   *pMeshYF;
    pMeshXY = new unsigned short[meshSizeW * meshSizeH * 2 * 2];
    pMeshXI = new unsigned short[meshSizeW * meshSizeH];
    pMeshXF = new unsigned char[meshSizeW * meshSizeH];
    pMeshYI = new unsigned short[meshSizeW * meshSizeH];
    pMeshYF = new unsigned char[meshSizeW * meshSizeH];
#else
    int meshStepW = 32;
    int meshStepH = 16;
    if (mesh_density == 0)
    {
        int mapStepW = 16;
        int mapStepH = 8;
    }
    int meshSizeW = imgWidth / meshStepW + 1;       // ��1920->121
    int meshSizeH = imgHeight / meshStepH + 1;      // ��1080->136
#endif

    GenMeshTable(imgWidth, imgHeight, meshStepW, meshStepH, meshSizeW, meshSizeH, camCoeff, pMeshXY, pMeshXI, pMeshYI, pMeshXF, pMeshYF);
    return 0;
}
#endif
