#include "RgaApi.h"

#include "RockchipRga.h"

RockchipRga& rkRga(RockchipRga::get());

int c_RkRgaInit()
{
    int ret;
    ret = rkRga.RkRgaInit();
    return ret;
}

void c_RkRgaDeInit()
{
    rkRga.RkRgaDeInit();
}

int c_RkRgaGetAllocBuffer(bo_t *bo_info, int width, int height, int bpp)
{
    rkRga.RkRgaGetAllocBuffer(bo_info, width, height, bpp);
    return 0;
}

int c_RkRgaGetMmap(bo_t *bo_info)
{
    rkRga.RkRgaGetMmap(bo_info);
    return 0;
}

int c_RkRgaUnmap(bo_t *bo_info)
{
    rkRga.RkRgaUnmap(bo_info);
    return 0;
}

int c_RkRgaFree(bo_t *bo_info)
{
    rkRga.RkRgaFree(bo_info);
    return 0;
}

int c_RkRgaGetBufferFd(bo_t *bo_info, int *fd)
{
    rkRga.RkRgaGetBufferFd(bo_info, fd);
    return 0;
}

int c_RkRgaBlit(rga_info_t *src, rga_info_t *dst, rga_info_t *src1)
{
    int ret;
    ret = rkRga.RkRgaBlit(src, dst, NULL);
    return ret ;
}

int c_RkRgaColorFill(rga_info_t *dst)
{
    return rkRga.RkRgaCollorFill(dst);
}

