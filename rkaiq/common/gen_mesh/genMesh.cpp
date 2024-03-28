#include "genMesh.h"
#include <iostream>
#include <math.h>
//#include <cmath>

/* FEC: 初始化, 根据图像输出分辨率, 计算FEC映射表的相关参数, 申请需要的buffer */
void genFecMeshInit(int srcW, int srcH, int dstW, int dstH, FecParams &fecParams, CameraCoeff &camCoeff)
{
	fecParams.srcW = srcW;
	fecParams.srcH = srcH;
	fecParams.dstW = dstW;
	fecParams.dstH = dstH;
	/* 扩展对齐 */
	fecParams.srcW_ex = 32 * ((srcW + 31) / 32);
	fecParams.srcH_ex = 32 * ((srcH + 31) / 32);
	fecParams.dstW_ex = 32 * ((dstW + 31) / 32);
	fecParams.dstH_ex = 32 * ((dstH + 31) / 32);
	/* 映射表的步长 */
	int meshStepW, meshStepH;
	if (dstW > 1920) { //32x16
		meshStepW = 32;
		meshStepH = 16;
	}
	else { //16x8
		meshStepW = 16;
		meshStepH = 8;
	}
	/* 映射表的宽高 */
	fecParams.meshSizeW = (fecParams.dstW_ex + meshStepW - 1) / meshStepW + 1;//modify to mesh alligned to 32x32
	fecParams.meshSizeH = (fecParams.dstH_ex + meshStepH - 1) / meshStepH + 1;//modify to mesh alligned to 32x32

	/* mesh表降采样的步长 */
	fecParams.meshStepW = meshStepW;
	fecParams.meshStepH = meshStepH;
	//fecParams.meshStepW = double(dstW) / double(fecParams.meshSizeW);
	//fecParams.meshStepH = double(dstH) / double(fecParams.meshSizeH);

	/* MeshXY的大小 */
	fecParams.meshSize1bin = fecParams.meshSizeW * fecParams.meshSizeH;

	/* 定点的小表MeshXY */
	fecParams.pMeshXY = new unsigned short[fecParams.meshSize1bin * 2 * 2];

	/* 计算4个mesh的相关参数 */
	unsigned short SpbMeshPNum = 128 / meshStepH * fecParams.meshSizeW;
	unsigned long MeshNumW;
	int LastSpbH;
	fecParams.SpbNum = (dstH + 128 - 1) / 128;
	MeshNumW = fecParams.dstW_ex / meshStepW;
	fecParams.MeshPointNumW = MeshNumW + 1;
	fecParams.SpbMeshPNumH = 128 / meshStepH + 1;//16x8 -> 17, 32x16 -> 9
	LastSpbH = (fecParams.dstH_ex % 128 == 0) ? 128 : (fecParams.dstH_ex % 128);//modify to mesh alligned to 32x32
	fecParams.LastSpbMeshPNumH = LastSpbH / meshStepH + 1;
	/* 4个mesh的大小 */
	fecParams.meshSize4bin = (fecParams.SpbNum - 1) * fecParams.MeshPointNumW * fecParams.SpbMeshPNumH + fecParams.MeshPointNumW * fecParams.LastSpbMeshPNumH;

	/* 预先计算的部分: 浮点未校正的小表和level=0,level=255的多项式参数 */
	genFecPreCalcPart(fecParams, camCoeff);
}

/* FEC: 反初始化 */
void genFecMeshDeInit(FecParams &fecParams)
{
	delete[] fecParams.pMeshXY;
}

/* FEC: 4个mesh 内存申请 */
void mallocFecMesh(int meshSize, unsigned short **pMeshXI, unsigned char **pMeshXF, unsigned short **pMeshYI, unsigned char **pMeshYF)
{
	*pMeshXI = new unsigned short[meshSize];
	*pMeshXF = new unsigned char[meshSize];
	*pMeshYI = new unsigned short[meshSize];
	*pMeshYF = new unsigned char[meshSize];
}

/* FEC: 4个mesh 内存释放 */
void freeFecMesh(unsigned short *pMeshXI, unsigned char *pMeshXF, unsigned short *pMeshYI, unsigned char *pMeshYF)
{
	delete[] pMeshXI;
	delete[] pMeshXF;
	delete[] pMeshYI;
	delete[] pMeshYF;
}


/* =============================================================================================================================================================================== */


/* LDCH: 初始化, 根据图像输出分辨率, 计算LDCH映射表的相关参数, 申请需要的buffer */
void genLdchMeshInit(int srcW, int srcH, int dstW, int dstH, LdchParams &ldchParams, CameraCoeff &camCoeff)
{
	ldchParams.srcW = srcW;
	ldchParams.srcH = srcH;
	ldchParams.dstW = dstW;
	ldchParams.dstH = dstH;

	int map_scale_bit_X = 4;
	int map_scale_bit_Y = 3;
	/* 定点化左移位数 */
    if (RKALGO_LDCH_VERSION_0 == ldchParams.enLdchVersion) {
        ldchParams.mapxFixBit = (dstW > 4096) ? 3 : 4;
    }
    if (RKALGO_LDCH_VERSION_1 == ldchParams.enLdchVersion) {
        ldchParams.mapxFixBit = (dstW > 4080) ? 3 : 4;
    }

	// mesh表的宽, 如2688->169
	ldchParams.meshSizeW = ((dstW + (1 << map_scale_bit_X) - 1) >> map_scale_bit_X) + 1;
	// mesh表的高, 如1520->191
	ldchParams.meshSizeH = ((dstH + (1 << map_scale_bit_Y) - 1) >> map_scale_bit_Y) + 1;

	/* mesh表降采样的步长 */
	//ldchParams.meshStepW = double(dstW) / double(ldchParams.meshSizeW - 1);/* 这里减1需要硬件上验证一下 */
	//ldchParams.meshStepH = double(dstH) / double(ldchParams.meshSizeH - 1);
	ldchParams.meshStepW = 16;
	ldchParams.meshStepH = 8;

	/* 对齐后的宽 */
	int mapWidAlign = ((ldchParams.meshSizeW + 1) >> 1) << 1;//例如, 分辨率2688*1520, 169->170
	ldchParams.meshSize = mapWidAlign * ldchParams.meshSizeH;

	//ldchParams.meshSizeW = mapWidAlign;

	/* 浮点的mesh网格 */
	ldchParams.mapx = new double[ldchParams.meshSize];
	ldchParams.mapy = new double[ldchParams.meshSize];

	/* LDCH: 预先计算的部分: 浮点未校正的小表和level=0,level=255的多项式参数 */
	genLdchPreCalcPart(ldchParams, camCoeff);

	/* LDCH: 计算LDCH能够校正的最大程度 */
	if (RKALGO_LDCH_VERSION_0 == ldchParams.enLdchVersion || RKALGO_LDCH_VERSION_1 == ldchParams.enLdchVersion)
	{
		calcLdchMaxLevel(ldchParams, camCoeff);
	}
	else
	{
		ldchParams.maxLevel = 255;
	}
}

/* LDCH: 反初始化 */
void genLdchMeshDeInit(LdchParams &ldchParams)
{
	delete[] ldchParams.mapx;
	delete[] ldchParams.mapy;
}

