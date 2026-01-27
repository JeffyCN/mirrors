/*
	1) 大表cv::Mat格式转FEC的4个小表xi,xf,yi,yf
*/
void mapBig2FecMesh(cv::Mat &mapxBig, cv::Mat &mapyBig, int srcW, int srcH, const char *fecMeshPath)
{
	/* 希望得到的校正后的目标图像dst的宽高 */
	int dstW = srcW;
	int dstH = srcH;
	/* dst图像宽高扩展对齐 */
	int dstW_ex = 32 * ((dstW + 31) / 32);
	int dstH_ex = 32 * ((dstH + 31) / 32);
	/* dst图像对应的映射表的步长 */
	int meshStepW, meshStepH;
	if (dstW > 1920) { //32x16
		meshStepW = 32;
		meshStepH = 16;
	}
	else { //16x8
		meshStepW = 16;
		meshStepH = 8;
	}
	/* dst图像对应的映射表的宽高 */
	int meshSizeW = (dstW_ex + meshStepW - 1) / meshStepW + 1;
	int meshSizeH = (dstH_ex + meshStepH - 1) / meshStepH + 1;
	/* dst图像对应的FEC mesh的数据大小 */
	int dstFecMeshSize = meshSizeW * meshSizeH * 2 * 2;
	/* 申请FEC mesh的内存 */
	unsigned short *pMeshXY = new unsigned short[dstFecMeshSize];

	double xVal = 0;
	double yVal = 0;
	/* FEC mesh的index */
	int xIdxMesh = 0;
	int yIdxMesh = meshSizeW * meshSizeH * 2;

	/* 大表降采样为dst图像对应的FEC mesh */
	for (int j = 0; j <= dstH_ex; j = j + meshStepH)
	{
		for (int i = 0; i <= dstW_ex; i = i + meshStepW)
		{
			if (j == dstH_ex)
			{
				j = dstH_ex - 1;
			}
			if (i == dstW_ex)
			{
				i = dstW_ex - 1;
			}
			xVal = mapxBig.at<double>(j, i);
			/* 定点化 */
			pMeshXY[xIdxMesh] = (unsigned short)xVal;
			pMeshXY[xIdxMesh + 1] = (unsigned short)((xVal - (unsigned short)xVal) * 256);

			yVal = mapyBig.at<double>(j, i);
			/* 定点化 */
			pMeshXY[yIdxMesh] = (unsigned short)yVal;
			pMeshXY[yIdxMesh + 1] = (unsigned short)((yVal - (unsigned short)yVal) * 256);

			xIdxMesh += 2;
			yIdxMesh += 2;
		}
	}

	/* 计算4个mesh的相关参数 */
	unsigned short SpbMeshPNum = 128 / meshStepH * meshSizeW;
	unsigned long MeshNumW;
	int LastSpbH;
	unsigned short SpbNum = (dstH + 128 - 1) / 128;
	MeshNumW = dstW_ex / meshStepW;
	unsigned long MeshPointNumW = MeshNumW + 1;
	unsigned short SpbMeshPNumH = 128 / meshStepH + 1;
	LastSpbH = (dstH_ex % 128 == 0) ? 128 : (dstH_ex % 128);
	unsigned short LastSpbMeshPNumH = LastSpbH / meshStepH + 1;
	/* 4个mesh的大小 */
	int meshSize4bin = (SpbNum - 1) * MeshPointNumW * SpbMeshPNumH + MeshPointNumW * LastSpbMeshPNumH;

	unsigned short	*pMeshXI = new unsigned short[meshSize4bin];		/* X整数部分 */
	unsigned char	*pMeshXF = new unsigned char[meshSize4bin];			/* X小数部分 */
	unsigned short	*pMeshYI = new unsigned short[meshSize4bin];		/* Y整数部分 */
	unsigned char	*pMeshYF = new unsigned char[meshSize4bin];			/* Y小数部分 */

	unsigned short *pTmpXi = (unsigned short *)&pMeshXY[0];
	unsigned short *pTmpXf = (unsigned short *)&pMeshXY[1];
	unsigned short *pTmpYi = (unsigned short *)&pMeshXY[meshSizeW * meshSizeH * 2];
	unsigned short *pTmpYf = (unsigned short *)&pMeshXY[meshSizeW * meshSizeH * 2 + 1];

	unsigned short *pTmpXI = new unsigned short[meshSizeW * meshSizeH];
	unsigned char  *pTmpXF = new unsigned char[meshSizeW * meshSizeH];
	unsigned short *pTmpYI = new unsigned short[meshSizeW * meshSizeH];
	unsigned char  *pTmpYF = new unsigned char[meshSizeW * meshSizeH];


	for (int i = 0; i < meshSizeW * meshSizeH; ++i) {
		pTmpXI[i] = *pTmpXi;					pTmpXi += 2;
		pTmpXF[i] = (unsigned char)*pTmpXf;		pTmpXf += 2;
		pTmpYI[i] = *pTmpYi;					pTmpYi += 2;
		pTmpYF[i] = (unsigned char)*pTmpYf;		pTmpYf += 2;
	}

	unsigned long OutIdx = 0;
	unsigned long InIdx = 0;

	unsigned short mesh_blkh;

	unsigned short ColMeshPNum = meshSizeW;
	unsigned short RowMeshPNum = meshSizeH;

	for (int spb_cnt = 0; spb_cnt < SpbNum; ++spb_cnt) {
		mesh_blkh = (spb_cnt == SpbNum - 1) ? LastSpbMeshPNumH : SpbMeshPNumH;
		for (int j = 0; j < MeshPointNumW; ++j) {
			for (int i = 0; i < mesh_blkh; ++i) {
				unsigned long IdxC = MIN(j, ColMeshPNum - 1);
				unsigned long IdxR = MIN((spb_cnt*(128 / meshStepH) + i), RowMeshPNum - 1);
				InIdx = IdxR * ColMeshPNum + IdxC;

				pMeshXI[OutIdx] = pTmpXI[InIdx];
				pMeshXF[OutIdx] = pTmpXF[InIdx];
				pMeshYI[OutIdx] = pTmpYI[InIdx];
				pMeshYF[OutIdx] = pTmpYF[InIdx];
				++OutIdx;
			}
		}
	}

	/* 内存释放 */
	delete[] pTmpXI;
	delete[] pTmpXF;
	delete[] pTmpYI;
	delete[] pTmpYF;

	char meshxiName[256];	strcpy(meshxiName, fecMeshPath);	strcat(meshxiName, "meshxi.bin");
	FILE* fp_meshxi = fopen(meshxiName, "wb");
	char meshxfName[256];	strcpy(meshxfName, fecMeshPath);	strcat(meshxfName, "meshxf.bin");
	FILE* fp_meshxf = fopen(meshxfName, "wb");
	char meshyiName[256];	strcpy(meshyiName, fecMeshPath);	strcat(meshyiName, "meshyi.bin");
	FILE* fp_meshyi = fopen(meshyiName, "wb");
	char meshyfName[256];	strcpy(meshyfName, fecMeshPath);	strcat(meshyfName, "meshyf.bin");
	FILE* fp_meshyf = fopen(meshyfName, "wb");

	if (fp_meshxi == NULL || fp_meshxf == NULL || fp_meshyi == NULL || fp_meshyf == NULL)
	{
		printf("save FEC 4 Mesh.bin error!!!\n");
		return;
	}

	fwrite(pMeshXI, sizeof(unsigned short), meshSize4bin, fp_meshxi);
	fwrite(pMeshXF, sizeof(unsigned char), meshSize4bin, fp_meshxf);
	fwrite(pMeshYI, sizeof(unsigned short), meshSize4bin, fp_meshyi);
	fwrite(pMeshYF, sizeof(unsigned char), meshSize4bin, fp_meshyf);

	fclose(fp_meshxi);
	fclose(fp_meshxf);
	fclose(fp_meshyi);
	fclose(fp_meshyf);

	/* 内存释放 */
	delete[] pMeshXY;
	delete[] pMeshXI;
	delete[] pMeshXF;
	delete[] pMeshYI;
	delete[] pMeshYF;
}

void test1()
{
	/* 输入图像真实宽高 */
	int srcW = 1280;
	int srcH = 1072;
	/* 输入图像宽高32对齐 */
	int srcW_ex = 32 * ((srcW + 31) / 32);
	int srcH_ex = 32 * ((srcH + 31) / 32);

	// 使用opencv生成全分辨率的表，表的宽高设置需要对齐
	cv::Size imageSize = cv::Size(srcW_ex, srcH_ex);
	cv::Mat mapxBig, mapyBig;
	cv::initUndistortRectifyMap(camMatrix, distCoeffs, R, newCamMatrix, imageSize, CV_32FC1, mapxBig, mapyBig);	// 这个函数只能生成float32类型的数据

	mapxBig.convertTo(mapxBig, CV_64FC1);// 所以这里需要转为float64类型
	mapyBig.convertTo(mapyBig, CV_64FC1);

	for (int i = 0; i < srcH_ex; i++)
	{
		for (int j = 0; j < srcW_ex; j++)
		{
			if (mapxBig.at<double>(i, j) < 0) { mapxBig.at<double>(i, j) = 0; }
			if (mapyBig.at<double>(i, j) < 0) { mapyBig.at<double>(i, j) = 0; }
			if (mapxBig.at<double>(i, j) > (srcW - 3)) { mapxBig.at<double>(i, j) = (srcW - 3); }/* 这里注意需要在右侧和下侧边界做限制 */
			if (mapyBig.at<double>(i, j) > (srcH - 3)) { mapyBig.at<double>(i, j) = (srcH - 3); }
		}
	}

	/* 生成FEC的4个mesh小表 */
	char *fecMeshPath = "../data_out/";
	mapBig2FecMesh(mapxBig, mapyBig, srcW, srcH, fecMeshPath);
}

/*
	1) 大表cv::Mat格式转LDCH的小表
*/
void mapBig2LdchMesh(cv::Mat &mapxBig, int srcW, int srcH, const char *ldchMeshPath)
{
	int map_scale_bit_X = 4;	/* Map表宽度下采样bit数 */
	int map_scale_bit_Y = 3;	/* Map表高度下采样bit数 */
	/* 定点化左移位数 */
	int mapxFixBit = 4;
	if (srcW > 4096)
	{
		mapxFixBit = 3;
	}
	/* mesh表的宽 */
	int meshSizeW = ((srcW + (1 << map_scale_bit_X) - 1) >> map_scale_bit_X) + 1;
	/* mesh表的高 */
	int meshSizeH = ((srcH + (1 << map_scale_bit_Y) - 1) >> map_scale_bit_Y) + 1;

	/* mesh表降采样的步长 */
	double meshStepW = double(srcW) / double(meshSizeW - 1);
	double meshStepH = double(srcH) / double(meshSizeH - 1);

	/* 对齐后的宽 */
	int mapWidAlign = ((meshSizeW + 1) >> 1) << 1;
	int meshSize = mapWidAlign * meshSizeH;
	unsigned short *pMeshX = new unsigned short[meshSize];

	double xVal = 0;
	int a = 0, b = 0;
	double mapxtmp;
	unsigned short tmpi;
	int indexX = 0;
	bool doAlign = (meshSizeW % 2) != 0;		/* 每行做2对齐*/
	//cv::Mat mapX = cv::Mat_<double>(meshSizeH, meshSizeW);/* 查看网格用的 */
	for (int j = 0; j < meshSizeH; ++j, b = b + meshStepH)
	{
		a = 0;
		for (int i = 0; i < meshSizeW; ++i, a = a + meshStepW)
		{
			if (a > srcW - 1) { a = srcW - 1; }
			if (b > srcH - 1) { b = srcH - 1; }
			xVal = mapxBig.at<double>(b, a);

			// 限制边界
			xVal = xVal > 0 ? xVal : 0;
			xVal = xVal < (srcW - 1) ? xVal : (srcW - 1);
			//mapX.at<double>(j, i) = xVal;
			mapxtmp = xVal;
			tmpi = mapxtmp * (1 << mapxFixBit);
			pMeshX[indexX] = tmpi;
			++indexX;
		}
		/* 每行做2对齐*/
		if (doAlign)
		{
			pMeshX[indexX] = 0;
			++indexX;
		}
	}
	/* 将MeshX保存为bin文件 */
	FILE *fpMeshX = fopen(ldchMeshPath, "wb");
	if (fpMeshX == NULL)
	{
		printf("save LDCH MeshX.bin open error!!!");
		return;
	}
	fwrite(&srcW, sizeof(unsigned short), 1, fpMeshX);
	fwrite(&srcH, sizeof(unsigned short), 1, fpMeshX);
	fwrite(&mapWidAlign, sizeof(unsigned short), 1, fpMeshX);
	fwrite(&meshSizeH, sizeof(unsigned short), 1, fpMeshX);
	fwrite(&meshStepW, sizeof(unsigned short), 1, fpMeshX);
	fwrite(&meshStepH, sizeof(unsigned short), 1, fpMeshX);
	fwrite(pMeshX, sizeof(unsigned short), meshSize, fpMeshX);
	fclose(fpMeshX);


	delete[] pMeshX;
}

void test2()
{
	/* 输入图像真实宽高 */
	int srcW = 2688;
	int srcH = 1520;
	/* 需要客户自己生成原分辨率的校正表mapxBig, 为了方便说明,以下用opencv的数据cv::Mat来表示全表,客户也可以用其他形式,如二维数组等 */
	cv::Mat mapxBig = cv::Mat_<double>(srcH, srcW);
	for (int i = 0; i < srcH; i++)
	{
		for (int j = 0; j < srcW; j++)
		{
			mapxBig.at<double>(i, j) = j;/* 这里以生成无校正效果的全表为例 */
			if (mapxBig.at<double>(i, j) < 0) { mapxBig.at<double>(i, j) = 0; }						/* 注意不要超出图像坐标范围 */
			if (mapxBig.at<double>(i, j) > (srcW - 1)) { mapxBig.at<double>(i, j) = (srcW - 1); }
		}
	}

	/* 生成LDCH的mesh表 */
	char *ldchMeshPath = "../data_out/ldch_meshx.bin";
	mapBig2LdchMesh(mapxBig, srcW, srcH, ldchMeshPath);
}
