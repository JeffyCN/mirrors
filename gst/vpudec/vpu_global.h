/*
 * Copyright 2010 Rockchip Electronics S.LSI Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _VPU_GLOBAL_
#define _VPU_GLOBAL_

#include "vpu_macro.h"
#include "vpu_mem.h"

typedef struct
{
    RK_U32   TimeLow;
    RK_U32   TimeHigh;
}TIME_STAMP;

typedef struct
{
    RK_U32   CodecType;
    RK_U32   ImgWidth;
    RK_U32   ImgHeight;
}VPU_GENERIC;

typedef enum DEC_TYPE_T {
   HW,
   SW,
}DEC_TYPE;

typedef struct
{
    RK_U32      StartCode;
    RK_U32      SliceLength;
    TIME_STAMP  SliceTime;
    RK_U32      SliceType;
    RK_U32      SliceNum;
    RK_U32      Res[2];
}VPU_BITSTREAM;  /*completely same as RK28*/

typedef struct
{
    RK_U32      InputAddr[2];
    RK_U32      OutputAddr[2];
    RK_U32      InputWidth;
    RK_U32      InputHeight;
    RK_U32      OutputWidth;
    RK_U32      OutputHeight;
    RK_U32      ColorType;
    RK_U32      ScaleEn;
    RK_U32      RotateEn;
    RK_U32      DitherEn;
    RK_U32      DeblkEn;
    RK_U32      DeinterlaceEn;
    RK_U32      Res[5];
}VPU_POSTPROCESSING;

typedef struct tVPU_FRAME
{
    RK_U32          FrameBusAddr[2];       //0: Y address; 1: UV address;
    RK_U32          FrameWidth;         //16X对齐宽度
    RK_U32          FrameHeight;        //16X对齐高度
    RK_U32          OutputWidth;        //非16X必须
    RK_U32          OutputHeight;       //非16X必须
    RK_U32          DisplayWidth;       //显示宽度
    RK_U32          DisplayHeight;      //显示高度
    RK_U32          CodingType;
    RK_U32          FrameType;          //frame;top_field_first;bot_field_first
    RK_U32          ColorType;
    RK_U32          DecodeFrmNum;
    TIME_STAMP      ShowTime;
    RK_U32          ErrorInfo;          //该帧的错误信息，返回给系统方便调试
    RK_U32	    employ_cnt;
    VPUMemLinear_t  vpumem;
    struct tVPU_FRAME *next_frame;
    RK_U32          Res[4];
}VPU_FRAME;

typedef enum VPU_API_CMD {
    VPU_API_ENC_SETCFG,
    VPU_API_ENC_GETCFG,
    VPU_API_ENC_SETFORMAT,
    VPU_API_ENC_SETIDRFRAME,
    VPU_API_ENABLE_DEINTERLACE,
    VPU_API_SET_VPUMEM_CONTEXT,
    VPU_API_USE_PRESENT_TIME_ORDER,
    VPU_API_SET_IMMEDIATE_DECODE,
    VPU_API_MALLOC_THREAD,
    VPU_API_DEC_GETFORMAT,
} VPU_API_CMD;

#endif /*_VPU_GLOBAL_*/
