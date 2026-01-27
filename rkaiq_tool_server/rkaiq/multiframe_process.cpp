#include "multiframe_process.h"

#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>
#include <thread>
#include <vector>
#include <cassert>

void DumpRawData(uint16_t* buf, uint32_t len, uint32_t plen)
{
    uint16_t a;
    for (uint32_t n = 0; n < len; n++)
    {
        if (n < plen)
        {
            a = buf[n];
            fprintf(stderr, "buf16[%d]      0x%04x\n", n, a);
        }
        else
        {
            break;
        }
    }
}

void DumpRawData32(uint32_t* buf, uint32_t len, uint32_t plen)
{
    uint32_t a;
    for (uint32_t n = 0; n < len; n++)
    {
        if (n < plen)
        {
            a = buf[n];
            fprintf(stderr, "buf32[%d]      0x%08x\n", n, a);
        }
        else
        {
            break;
        }
    }
}

void ConverToLE(uint16_t* buf, uint32_t len)
{
    uint32_t a;
    for (uint32_t n = 0; n < len; n++)
    {
        a = buf[n];
        buf[n] = (a << 8) | (a >> 8);
    }
}

// only for even number frame
void MultiFrameAverage(uint32_t* pIn1_pOut, uint16_t* POut, uint16_t width, uint16_t height, uint16_t frameNumber)
{
    uint16_t n;
    uint16_t roundOffset = 0;
    switch (frameNumber)
    {
        case 128:
            n = 7;
            break;
        case 64:
            n = 6;
            break;
        case 32:
            n = 5;
            break;
        case 16:
            n = 4;
            break;
        case 8:
            n = 3;
            break;
        case 4:
            n = 2;
            break;
        case 2:
            n = 1;
            break;
        default:
            printf("frame number error! %d", frameNumber);
            return;
            break;
    }
    roundOffset = pow(2, n - 1);

    int len = height * width;
    for (int i = 0; i < len; i++)
    {
        pIn1_pOut[i] += roundOffset;
        pIn1_pOut[i] = pIn1_pOut[i] >> n;
        POut[i] = (uint16_t)(pIn1_pOut[i]);
    }
}

void MultiFrameAddition(uint32_t* pIn1_pOut, uint16_t* pIn2, uint16_t width, uint16_t height, bool biToLi)
{
    int n;
    int len = height * width;
    uint16_t temp = 0;
    for (n = 0; n < len; n++)
    {
        temp = 0;
        if (biToLi)
        {
            temp |= ((pIn2[n] & 0xff) << 8) & 0xff00;
            temp |= ((pIn2[n] & 0xff00) >> 8) & 0xff;
            pIn1_pOut[n] += temp;
        }
        else
        {
            temp = pIn2[n] & 0xffff;
            pIn1_pOut[n] += temp;
        }
    }
}

void FrameU32ToU16(uint32_t* pIn, uint16_t* pOut, uint16_t width, uint16_t height)
{
    int n;
    int len = height * width;
    for (n = 0; n < len; n++)
    {
        pOut[n] += pIn[n];
    }
}

void FrameU16ToU32(uint16_t* pIn, uint32_t* pOut, uint16_t width, uint16_t height)
{
    int n;
    int len = height * width;
    for (n = 0; n < len; n++)
    {
        pOut[n] += pIn[n];
    }
}
