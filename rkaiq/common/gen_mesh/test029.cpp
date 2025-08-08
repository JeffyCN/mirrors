
/* 8k图--->2个4k图，进行FEC分级校正 */
void test029()
{
	int srcW = 7680;
	int srcH = 4320;
	int dstW = 7680;
	int dstH = 4320;
	int margin = 256;

	/* 全图8k相关参数 */
	CameraCoeff camCoeff;
	camCoeff.a0 = -4628.92834904855135391699150204658508300781250000000000;
	camCoeff.a2 = 0.00008439805632153267055031026222522427815420087427;
	camCoeff.a3 = -0.00000000947972529654520536345924537060744774485954;
	camCoeff.a4 = 0.00000000000276046059610868196196561020719728129884;
	camCoeff.cx = (srcW - 1.0) * 0.5;
	camCoeff.cy = (srcH - 1.0) * 0.5;
	FecParams fecParams;
	fecParams.correctX = 1;								/* 水平x方向校正: 1代表校正, 0代表不校正 */
	fecParams.correctY = 1;								/* 垂直y方向校正: 1代表校正, 0代表不校正 */
	fecParams.saveMaxFovX = 1;							/* 保留水平x方向最大FOV: 1代表保留, 0代表不保留 */
	fecParams.isFecOld = 0;								/* 是否旧版FEC: 1代表是，0代表不是 */
	fecParams.saveMesh4bin = 0;							/* 是否保存meshxi,xf,yi,yf4个bin文件: 1代表保存, 0代表不保存 */
	sprintf(fecParams.mesh4binPath, "../data_out/");	/* 保存meshxi,xf,yi,yf4个bin文件的根目录 */

	/* 左图4k相关参数 */
	CameraCoeff camCoeff_left;
	FecParams fecParams_left;
	/* 右图4k相关参数 */
	CameraCoeff camCoeff_right;
	FecParams fecParams_right;
	/* LDCH相关参数初始化 */
	genFecMeshInit8kTo4k(srcW, srcH, dstW, dstH, margin, camCoeff, camCoeff_left, camCoeff_right, fecParams, fecParams_left, fecParams_right);
	/* 映射表buffer申请 */
	unsigned short	*pMeshXI, *pMeshYI;																				/* X, Y整数部分 */
	unsigned char	*pMeshXF, *pMeshYF;																				/* X, Y小数部分 */
	mallocFecMesh(fecParams.meshSize4bin, &pMeshXI, &pMeshXF, &pMeshYI, &pMeshYF);									/* 全图FEC映射表buffer申请 */
	unsigned short	*pMeshXI_left, *pMeshYI_left;																	/* X, Y整数部分 */
	unsigned char	*pMeshXF_left, *pMeshYF_left;																	/* X, Y小数部分 */
	mallocFecMesh(fecParams_left.meshSize4bin, &pMeshXI_left, &pMeshXF_left, &pMeshYI_left, &pMeshYF_left);			/* 左图FEC映射表buffer申请 */
	unsigned short	*pMeshXI_right, *pMeshYI_right;																	/* X, Y整数部分 */
	unsigned char	*pMeshXF_right, *pMeshYF_right;																	/* X, Y小数部分 */
	mallocFecMesh(fecParams_right.meshSize4bin, &pMeshXI_right, &pMeshXF_right, &pMeshYI_right, &pMeshYF_right);	/* 右图FEC映射表buffer申请 */

	/* 输入输出图像buffer申请 */
	/* 全图 */
	unsigned long srcSize = (srcW * srcH) > (dstW * dstH) ? (srcW * srcH) : (dstW * dstH);
	unsigned char *pImgY = new unsigned char[srcSize];
	unsigned char *pImgUV = new unsigned char[srcSize];
	unsigned char *pImgOut = new unsigned char[srcSize * 2];
	/* 左图 */
	int srcW_left = srcW * 0.5 + margin;
	int srcH_left = srcH;
	int dstW_left = srcW * 0.5 + margin;
	int dstH_left = srcH;
	unsigned long srcSize_left = (srcW_left * srcH_left) > (dstW_left * dstH_left) ? (srcW_left * srcH_left) : (dstW_left * dstH_left);
	unsigned char *pImgY_left = new unsigned char[srcSize_left];
	unsigned char *pImgUV_left = new unsigned char[srcSize_left];
	unsigned char *pImgOut_left = new unsigned char[srcSize_left * 2];
	/* 右图 */
	int srcW_right = srcW * 0.5 + margin;
	int srcH_right = srcH;
	int dstW_right = srcW * 0.5 + margin;
	int dstH_right = srcH;
	unsigned long srcSize_right = (srcW_right * srcH_right) > (dstW_right * dstH_right) ? (srcW_right * srcH_right) : (dstW_right * dstH_right);
	unsigned char *pImgY_right = new unsigned char[srcSize_right];
	unsigned char *pImgUV_right = new unsigned char[srcSize_right];
	unsigned char *pImgOut_right = new unsigned char[srcSize_right * 2];

	/* 读取输入图像 */
	char srcYuvPath[512] = "../data_in/image/group_023_imx415_2.8mm_7680x4320_half/imx415_2.8mm_full_7680x4320_08.nv12";
	readYUV(srcYuvPath, srcW, srcH, 0, NULL, pImgY, pImgUV);
	char srcYuvPath_left[512] = "../data_in/image/group_023_imx415_2.8mm_7680x4320_half/imx415_2.8mm_left_4096x4320_08.nv12";
	readYUV(srcYuvPath_left, srcW_left, srcH_left, 0, NULL, pImgY_left, pImgUV_left);
	char srcYuvPath_right[512] = "../data_in/image/group_023_imx415_2.8mm_7680x4320_half/imx415_2.8mm_right_4096x4320_08.nv12";
	readYUV(srcYuvPath_right, srcW_right, srcH_right, 0, NULL, pImgY_right, pImgUV_right);

	/* 生成FEC映射表并进行校正 */
	int level = 0;					/* level范围: 0-255 */
	int levelValue[] = { 0,64,128,192,255 };
	//for (level = 0; level <= 0; level = level + 1)
	for (int levelIdx = 0; levelIdx < 5; ++levelIdx)
	{
		level = levelValue[levelIdx];
		printf("level = %d\n", level);

		/* 左图进行FEC校正 */
		bool success_left = genFECMeshNLevel(fecParams_left, camCoeff_left, level, pMeshXI_left, pMeshXF_left, pMeshYI_left, pMeshYF_left);				/* 生成对应校正level的FEC映射表 */
		FEC_Cmodel_4bin(srcW_left, srcH_left, dstW_left, dstH_left, pImgY_left, pImgUV_left,
			pMeshXI_left, pMeshXF_left, pMeshYI_left, pMeshYF_left, pImgOut_left, 0, 0, 0, 0, 0, 0);													/* 调用FEC */
		cv::Mat dstImgBGR_left;																															/* 保存 */
		NV12toRGB(pImgOut_left, dstW_left, dstH_left, dstImgBGR_left);
		char dstBmpPath_left[256];
		sprintf(dstBmpPath_left, "../data_out/fec_left_%dx%d_level%03d.bmp", dstW_left, dstH_left, level);
		cv::imwrite(dstBmpPath_left, dstImgBGR_left);

		/* 右图进行FEC校正 */
		bool success_right = genFECMeshNLevel(fecParams_right, camCoeff_right, level, pMeshXI_right, pMeshXF_right, pMeshYI_right, pMeshYF_right);		/* 生成对应校正level的FEC映射表 */
		FEC_Cmodel_4bin(srcW_right, srcH_right, dstW_right, dstH_right, pImgY_right, pImgUV_right,
			pMeshXI_right, pMeshXF_right, pMeshYI_right, pMeshYF_right, pImgOut_right, 0, 0, 0, 0, 0, 0);												/* 调用FEC */
		cv::Mat dstImgBGR_right;																														/* 保存 */
		NV12toRGB(pImgOut_right, dstW_right, dstH_right, dstImgBGR_right);
		char dstBmpPath_right[256];
		sprintf(dstBmpPath_right, "../data_out/fec_right_%dx%d_level%03d.bmp", dstW_right, dstH_right, level);
		cv::imwrite(dstBmpPath_right, dstImgBGR_right);

		/* 验证: 全图进行FEC校正 */
		bool success_full = genFECMeshNLevel(fecParams, camCoeff, level, pMeshXI, pMeshXF, pMeshYI, pMeshYF);											/* 生成对应校正level的FEC映射表 */
		FEC_Cmodel_4bin(srcW, srcH, dstW, dstH, pImgY, pImgUV,
			pMeshXI, pMeshXF, pMeshYI, pMeshYF, pImgOut, 0, 0, 0, 0, 0, 0);																				/* 调用FEC */
		cv::Mat dstImgBGR_full;																															/* 保存 */
		NV12toRGB(pImgOut, dstW, dstH, dstImgBGR_full);
		char dstBmpPath_full[256];
		sprintf(dstBmpPath_full, "../data_out/fec_full_%dx%d_level%03d.bmp", dstW, dstH, level);
		cv::imwrite(dstBmpPath_full, dstImgBGR_full);

		/* 验证: 左图FEC结果 + 右图FEC结果 ---> 拼接的全图结果 */
		cv::Mat dstImgBGR_stitch = cv::Mat(dstH, dstW, CV_8UC3);
		dstImgBGR_left(cv::Range(0, dstH_left), cv::Range(0, dstW_left - margin)).copyTo(dstImgBGR_stitch(cv::Range(0, dstH), cv::Range(0, dstW * 0.5)));
		dstImgBGR_right(cv::Range(0, dstH_left), cv::Range(margin, dstW_right)).copyTo(dstImgBGR_stitch(cv::Range(0, dstH), cv::Range(dstW * 0.5, dstW)));
		char dstBmpPath_stitch[256];
		sprintf(dstBmpPath_stitch, "../data_out/fec_stitch_%dx%d_level%03d.bmp", dstW, dstH, level);
		cv::imwrite(dstBmpPath_stitch, dstImgBGR_stitch);
	}
	/* 全图相关内存释放和反初始化 */
	delete[] pImgY;
	delete[] pImgUV;
	delete[] pImgOut;
	freeFecMesh(pMeshXI, pMeshXF, pMeshYI, pMeshYF);							/* 内存释放 */
	genFecMeshDeInit(fecParams);												/* 反初始化 */

	/* 左图相关内存释放和反初始化 */
	delete[] pImgY_left;
	delete[] pImgUV_left;
	delete[] pImgOut_left;
	freeFecMesh(pMeshXI_left, pMeshXF_left, pMeshYI_left, pMeshYF_left);		/* 内存释放 */
	genFecMeshDeInit(fecParams_left);											/* 反初始化 */

	/* 右图相关内存释放和反初始化 */
	delete[] pImgY_right;
	delete[] pImgUV_right;
	delete[] pImgOut_right;
	freeFecMesh(pMeshXI_right, pMeshXF_right, pMeshYI_right, pMeshYF_right);	/* 内存释放 */
	genFecMeshDeInit(fecParams_right);											/* 反初始化 */


}