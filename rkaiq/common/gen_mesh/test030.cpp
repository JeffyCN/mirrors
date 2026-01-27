
/* 8k图--->2个4k图，进行LDCH分级校正 */
void test030()
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
	LdchParams ldchParams;
	ldchParams.saveMaxFovX = 0;											/* 保留水平x方向最大FOV: 1代表保留, 0代表不保留 */
	ldchParams.isLdchOld = 1;											/* 是否旧版LDCH: 1代表是，0代表不是 */
	ldchParams.saveMeshX = 1;											/* 是否保存MeshX.bin文件: 1代表保存, 0代表不保存 */
	sprintf(ldchParams.meshPath, "../data_out/");						/* 保存MeshX.bin文件的根目录 */
	/* 左图4k相关参数 */
	CameraCoeff camCoeff_left;
	LdchParams ldchParams_left;
	/* 右图4k相关参数 */
	CameraCoeff camCoeff_right;
	LdchParams ldchParams_right;
	/* LDCH相关参数初始化 */
	genLdchMeshInit8kTo4k(srcW, srcH, dstW, dstH, margin, camCoeff, camCoeff_left, camCoeff_right, ldchParams, ldchParams_left, ldchParams_right);
	/* 映射表buffer申请 */
	unsigned short *pMeshX = new unsigned short[ldchParams.meshSize];							/* 全图 */
	unsigned short *pMeshX_left = new unsigned short[ldchParams_left.meshSize];					/* 左图 */
	unsigned short *pMeshX_right = new unsigned short[ldchParams_right.meshSize];				/* 右图 */

	/* 输入输出图像buffer申请 */
	/* 全图 */
	unsigned long srcSize = (srcW * srcH) > (dstW * dstH) ? (srcW * srcH) : (dstW * dstH);
	unsigned short *pImgIn = new unsigned short[srcSize * 3];
	unsigned short *pImgOut = new unsigned short[srcSize * 3];
	/* 左图 */
	int srcW_left = srcW * 0.5 + margin;
	int srcH_left = srcH;
	int dstW_left = srcW * 0.5 + margin;
	int dstH_left = srcH;
	unsigned long srcSize_left = (srcW_left * srcH_left) > (dstW_left * dstH_left) ? (srcW_left * srcH_left) : (dstW_left * dstH_left);
	unsigned short *pImgIn_left = new unsigned short[srcSize_left * 3];
	unsigned short *pImgOut_left = new unsigned short[srcSize_left * 3];
	/* 右图 */
	int srcW_right = srcW * 0.5 + margin;
	int srcH_right = srcH;
	int dstW_right = srcW * 0.5 + margin;
	int dstH_right = srcH;
	unsigned long srcSize_right = (srcW_right * srcH_right) > (dstW_right * dstH_right) ? (srcW_right * srcH_right) : (dstW_right * dstH_right);
	unsigned short *pImgIn_right = new unsigned short[srcSize_right * 3];
	unsigned short *pImgOut_right = new unsigned short[srcSize_right * 3];

	/* 读取输入图像 */
	char srcBGRPath[256] = "../data_in/image/group_023_imx415_2.8mm_7680x4320_half/imx415_2.8mm_full_7680x4320_08.bmp";
	readRGBforLDCH(srcBGRPath, srcW, srcH, pImgIn);
	char srcBGRPath_left[256] = "../data_in/image/group_023_imx415_2.8mm_7680x4320_half/imx415_2.8mm_left_4096x4320_08.bmp";
	readRGBforLDCH(srcBGRPath_left, srcW_left, srcH_left, pImgIn_left);
	char srcBGRPath_right[256] = "../data_in/image/group_023_imx415_2.8mm_7680x4320_half/imx415_2.8mm_right_4096x4320_08.bmp";
	readRGBforLDCH(srcBGRPath_right, srcW_right, srcH_right, pImgIn_right);

	/* 生成LDCH映射表并进行校正 */
	int level = 0;				/* level范围: 0-255 */
	int levelValue[] = { 0,64,128,192,255 };
	//for (level = 0; level <= 255; level = level + 1)
	for (int levelIdx = 0; levelIdx < 5; ++levelIdx)
	{
		level = levelValue[levelIdx];
		printf("level = %d\n", level);

		/* 左图进行LDCH校正 */
		bool success_left = genLDCMeshNLevel(ldchParams_left, camCoeff_left, level, pMeshX_left);					/* 生成对应校正level的LDCH映射表 */
		LDCH_Cmodel(dstW_left, dstH_left, pImgIn_left, pImgOut_left, pMeshX_left);									/* 调用LDCH_Cmodel */
		cv::Mat dstImgBGR_left;
		ldchOut2Mat(dstW_left, dstH_left, 8, pImgOut_left, dstImgBGR_left);											/* LDCH输出结果转为cv::Mat格式 */
		char dstBmpPath_left[256];																					/* 保存 */
		sprintf(dstBmpPath_left, "../data_out/ldch_left_%dx%d_level%03d.bmp", dstW_left, dstH_left, level);
		cv::imwrite(dstBmpPath_left, dstImgBGR_left);

		/* 右图进行LDCH校正 */
		bool success_right = genLDCMeshNLevel(ldchParams_right, camCoeff_right, level, pMeshX_right);				/* 生成对应校正level的LDCH映射表 */
		LDCH_Cmodel(dstW_right, dstH_right, pImgIn_right, pImgOut_right, pMeshX_right);								/* 调用LDCH_Cmodel */
		cv::Mat dstImgBGR_right;
		ldchOut2Mat(dstW_right, dstH_right, 8, pImgOut_right, dstImgBGR_right);										/* LDCH输出结果转为cv::Mat格式 */
		char dstBmpPath_right[256];																					/* 保存 */
		sprintf(dstBmpPath_right, "../data_out/ldch_right_%dx%d_level%03d.bmp", dstW_right, dstH_right, level);
		cv::imwrite(dstBmpPath_right, dstImgBGR_right);

		/* 验证: 全图进行LDCH校正 */
		bool success_full = genLDCMeshNLevel(ldchParams, camCoeff, level, pMeshX);									/* 生成对应校正level的LDCH映射表 */
		LDCH_Cmodel(dstW, dstH, pImgIn, pImgOut, pMeshX);															/* 调用LDCH_Cmodel */
		cv::Mat dstImgBGR_full;
		ldchOut2Mat(dstW, dstH, 8, pImgOut, dstImgBGR_full);														/* LDCH输出结果转为cv::Mat格式 */
		char dstBmpPath_full[256];																					/* 保存 */
		sprintf(dstBmpPath_full, "../data_out/ldch_full_%dx%d_level%03d.bmp", dstW, dstH, level);
		cv::imwrite(dstBmpPath_full, dstImgBGR_full);

		/* 验证: 左图FEC结果 + 右图FEC结果 ---> 拼接的全图结果 */
		cv::Mat dstImgBGR_stitch = cv::Mat(dstH, dstW, CV_8UC3);
		dstImgBGR_left(cv::Range(0, dstH_left), cv::Range(0, dstW_left - margin)).copyTo(dstImgBGR_stitch(cv::Range(0, dstH), cv::Range(0, dstW * 0.5)));
		dstImgBGR_right(cv::Range(0, dstH_left), cv::Range(margin, dstW_right)).copyTo(dstImgBGR_stitch(cv::Range(0, dstH), cv::Range(dstW * 0.5, dstW)));
		char dstBmpPath_stitch[256];
		sprintf(dstBmpPath_stitch, "../data_out/ldch_stitch_%dx%d_level%03d.bmp", dstW, dstH, level);
		cv::imwrite(dstBmpPath_stitch, dstImgBGR_stitch);
	}
	/* 全图相关内存释放和反初始化 */
	delete[] pImgIn;						/* 内存释放 */
	delete[] pImgOut;
	delete[] pMeshX;
	genLdchMeshDeInit(ldchParams);			/* 反初始化 */

	/* 左图相关内存释放和反初始化 */
	delete[] pImgIn_left;					/* 内存释放 */
	delete[] pImgOut_left;
	delete[] pMeshX_left;
	genLdchMeshDeInit(ldchParams_left);		/* 反初始化 */

	/* 右图相关内存释放和反初始化 */
	delete[] pImgIn_right;					/* 内存释放 */
	delete[] pImgOut_right;
	delete[] pMeshX_right;
	genLdchMeshDeInit(ldchParams_right);	/* 反初始化 */
}