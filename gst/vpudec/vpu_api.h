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

#ifndef _VPU_API_H_
#define _VPU_API_H_

#include "vpu_global.h"

#define VPU_API_NOPTS_VALUE          (0x8000000000000000LL)

typedef struct VideoPacket {
    RK_S64 pts;                /* with unit of us*/
    RK_S64 dts;                /* with unit of us*/
    RK_U8 *data;
    RK_S32 size;
    RK_U32 capability;
    RK_U32 nFlags;
}VideoPacket_t;

typedef struct DecoderOut {
    RK_U8 *data;
    RK_U32 size;
    RK_S64 timeUs;
    RK_U32 nFlags;
}DecoderOut_t;

typedef struct EncInputStream {
    RK_U8 *buf;
    RK_S32 size;
    RK_U32 bufPhyAddr;
    RK_S64 timeUs;
    RK_U32 nFlags;
}EncInputStream_t;

typedef struct EncoderOut {
    RK_U8 *data;
    RK_S32 size;
    RK_S64 timeUs;
    RK_S32 keyFrame;

}EncoderOut_t;

typedef enum
{
    VPU_H264ENC_YUV420_PLANAR = 0,  /* YYYY... UUUU... VVVV */
    VPU_H264ENC_YUV420_SEMIPLANAR = 1,  /* YYYY... UVUVUV...    */
    VPU_H264ENC_YUV422_INTERLEAVED_YUYV = 2,    /* YUYVYUYV...          */
    VPU_H264ENC_YUV422_INTERLEAVED_UYVY = 3,    /* UYVYUYVY...          */
    VPU_H264ENC_RGB565 = 4, /* 16-bit RGB           */
    VPU_H264ENC_BGR565 = 5, /* 16-bit RGB           */
    VPU_H264ENC_RGB555 = 6, /* 15-bit RGB           */
    VPU_H264ENC_BGR555 = 7, /* 15-bit RGB           */
    VPU_H264ENC_RGB444 = 8, /* 12-bit RGB           */
    VPU_H264ENC_BGR444 = 9, /* 12-bit RGB           */
    VPU_H264ENC_RGB888 = 10,    /* 24-bit RGB           */
    VPU_H264ENC_BGR888 = 11,    /* 24-bit RGB           */
    VPU_H264ENC_RGB101010 = 12, /* 30-bit RGB           */
    VPU_H264ENC_BGR101010 = 13  /* 30-bit RGB           */
} H264EncPictureType;

/**
 * Enumeration used to define the possible video compression codings.
 * NOTE:  This essentially refers to file extensions. If the coding is
 *        being used to specify the ENCODE type, then additional work
 *        must be done to configure the exact flavor of the compression
 *        to be used.  For decode cases where the user application can
 *        not differentiate between MPEG-4 and H.264 bit streams, it is
 *        up to the codec to handle this.
 */

typedef enum VPU_VIDEO_CODINGTYPE {
    VPU_VIDEO_CodingUnused,     /**< Value when coding is N/A */
    VPU_VIDEO_CodingAutoDetect, /**< Autodetection of coding type */
    VPU_VIDEO_CodingMPEG2,      /**< AKA: H.262 */
    VPU_VIDEO_CodingH263,       /**< H.263 */
    VPU_VIDEO_CodingMPEG4,      /**< MPEG-4 */
    VPU_VIDEO_CodingWMV,        /**< Windows Media Video (WMV1,WMV2,WMV3)*/
    VPU_VIDEO_CodingRV,         /**< all versions of Real Video */
    VPU_VIDEO_CodingAVC,        /**< H.264/AVC */
    VPU_VIDEO_CodingMJPEG,      /**< Motion JPEG */
    VPU_VIDEO_CodingVP8,                     /**< VP8 */
    VPU_VIDEO_CodingVP9,                     /**< VP9 */
    VPU_VIDEO_CodingVC1 = 0x01000000, /**< Windows Media Video (WMV1,WMV2,WMV3)*/
    VPU_VIDEO_CodingFLV1,       /**< Sorenson H.263 */
    VPU_VIDEO_CodingDIVX3,                   /**< DIVX3 */
    VPU_VIDEO_CodingVP6,
    VPU_VIDEO_CodingHEVC,        /**< H.265/HEVC */
    VPU_VIDEO_CodingKhronosExtensions = 0x6F000000, /**< Reserved region for introducing Khronos Standard Extensions */
    VPU_VIDEO_CodingVendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
    VPU_VIDEO_CodingMax = 0x7FFFFFFF
} VPU_VIDEO_CODINGTYPE;

typedef enum VPU_VIDEO_PIXEL_FMT {
    VPU_VIDEO_PIXEL_FMT_NV12
} VPU_VIDEO_PIXEL_FMT;

typedef struct DecoderFormat {
    VPU_VIDEO_PIXEL_FMT format;
    RK_U32 width;
    RK_U32 height;
    RK_U32 stride;
    RK_U32 framesize;
    RK_U32 aligned_width;
    RK_U32 aligned_height;
    RK_U32 aligned_stride;
    RK_U32 aligned_framesize;
} DecoderFormat_t;

typedef enum CODEC_TYPE {
    CODEC_NONE,
    CODEC_DECODER,
    CODEC_ENCODER,
    CODEC_BUTT,
}CODEC_TYPE;

typedef enum VPU_API_ERR {
   VPU_API_OK = 0,
   VPU_API_ERR_UNKNOWN          =   -1,
   VPU_API_ERR_BASE             =   -1000,
   VPU_API_ERR_LIST_STREAM      =   VPU_API_ERR_BASE - 1,
   VPU_API_ERR_INIT             =   VPU_API_ERR_BASE - 2,
   VPU_API_ERR_VPU_CODEC_INIT   =   VPU_API_ERR_BASE - 3,
   VPU_API_ERR_STREAM           =   VPU_API_ERR_BASE - 4,
   VPU_API_ERR_FATAL_THREAD     =   VPU_API_ERR_BASE - 5,
   VPU_API_EOS_STREAM_REACHED   =   VPU_API_ERR_BASE - 11,
   VPU_API_ERR_BUTT,
}VPU_API_ERR;

typedef struct EncParameter
{
   int width;
   int height;
   int rc_mode;
   int bitRate;
   int framerate;
   int  qp;
   int  enableCabac;
   int  cabacInitIdc;
   int  format;
   int	intraPicRate;
   int  framerateout;
   int  profileIdc;
   int  levelIdc;
   int	reserved[3];
}EncParameter_t;

typedef struct EXtraCfg {
    RK_S32 vc1extra_size;
    RK_S32 vp6codeid;
    RK_S32 tsformat;
    RK_U32 reserved[20];
}EXtraCfg_t;
/**
 * @addtogroup rk_vpu_codec
 * @{
 */
typedef struct VpuCodecContext {
    void* vpuApiObj;

    CODEC_TYPE codecType;
    VPU_VIDEO_CODINGTYPE videoCoding;

    RK_U32 width;
    RK_U32 height;
    RK_U8 *extradata;
    RK_S32 extradata_size;
    RK_U8  enableparsing;
    RK_S32 no_thread;
    EXtraCfg_t extra_cfg;

    void* private_data;

    /*
     ** 1: error state(not working)  0: working
    */
    RK_S32 decoder_err;

    /**
     * Allocate and initialize an VpuCodecContext.
     *
     * @param ctx The context of vpu api, allocated in this function.
     * @param extraData The extra data of codec, some codecs need / can
     *        use extradata like Huffman tables, also live VC1 codec can
     *        use extradata to initialize itself.
     * @param extra_size The size of extra data.
     *
     * @return 0 for init success, others for failure.
     * note: check whether ctx has been allocated success after you do init.
     */
    RK_S32 (*init)(struct VpuCodecContext *ctx, RK_U8 *extraData, RK_U32 extra_size);

    /**
     * @return 0 for decode success, others for failure.
     */
    RK_S32 (*decode)(struct VpuCodecContext *ctx, VideoPacket_t *pkt, DecoderOut_t *aDecOut);

    /**
     * @return 0 for encode success, others for failure.
     */
    RK_S32 (*encode)(struct VpuCodecContext *ctx, EncInputStream_t *aEncInStrm, EncoderOut_t *aEncOut);

    /**
     * flush codec while do fast forward playing.
     *
     * @return 0 for flush success, others for failure.
     */
    RK_S32 (*flush)(struct VpuCodecContext *ctx);
    RK_S32 (*control)(struct VpuCodecContext *ctx, VPU_API_CMD cmdType, void* param);
    /**
     *seperate the decode function to two function
     *
    */
    RK_S32 (*decode_sendEventEoS)(struct VpuCodecContext *ctx);
    RK_S32 (*decode_sendstream)(struct VpuCodecContext *ctx, VideoPacket_t *pkt);
    RK_S32 (*decode_getframe_async)(struct VpuCodecContext *ctx, DecoderOut_t *aDecOut);
    RK_S32 (*decode_getframe_sync)(struct VpuCodecContext *ctx, DecoderOut_t *aDecOut);

    RK_S32 (*encoder_sendframe)(struct VpuCodecContext *ctx,  EncInputStream_t *aEncInStrm);
    RK_S32 (*encoder_getstream)(struct VpuCodecContext *ctx, EncoderOut_t *aEncOut);

}VpuCodecContext_t;

typedef RK_S32 (*VpuOpenContextFactoryFun)(struct VpuCodecContext **ctx);
typedef RK_S32 (*VpuCloseContextFactoryFun)(struct VpuCodecContext **ctx);

/* allocated vpu codec context */
#ifdef __cplusplus
extern "C"
{
#endif
RK_S32 vpu_open_context(struct VpuCodecContext **ctx);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif
RK_S32 vpu_close_context(struct VpuCodecContext **ctx);
#ifdef __cplusplus
}
#endif

#endif
