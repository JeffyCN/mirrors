#include "cam_types.h"
#include <linux/videodev2.h>
using namespace std;

const char RK_HAL_FMT_STRING::HAL_FMT_STRING_NV12[] = "nv12";
const char RK_HAL_FMT_STRING::HAL_FMT_STRING_NV21[] = "nv21";
const char RK_HAL_FMT_STRING::HAL_FMT_STRING_YVU420[] = "yvu420";
const char RK_HAL_FMT_STRING::HAL_FMT_STRING_RGB565[] = "rgb565";
const char RK_HAL_FMT_STRING::HAL_FMT_STRING_RGB32[] = "rgb32";
const char RK_HAL_FMT_STRING::HAL_FMT_STRING_YUV422P[] = "yuv422p";
const char RK_HAL_FMT_STRING::HAL_FMT_STRING_NV16[] = "nv16";
const char RK_HAL_FMT_STRING::HAL_FMT_STRING_YUYV[] = "yuyv";
const char RK_HAL_FMT_STRING::HAL_FMT_STRING_JPEG[] = "jpeg";
const char RK_HAL_FMT_STRING::HAL_FMT_STRING_MJPEG[] = "mjpeg";
const char RK_HAL_FMT_STRING::HAL_FMT_STRING_H264[] = "h264";
const char RK_HAL_FMT_STRING::HAL_FMT_STRING_Y8[] = "y8";
const char RK_HAL_FMT_STRING::HAL_FMT_STRING_Y10[] = "y10";
const char RK_HAL_FMT_STRING::HAL_FMT_STRING_Y12[] = "y12";
const char RK_HAL_FMT_STRING::HAL_FMT_STRING_SBGGR10[] = "SBGGR10";
const char RK_HAL_FMT_STRING::HAL_FMT_STRING_SGBRG10[] = "SGBRG10";
const char RK_HAL_FMT_STRING::HAL_FMT_STRING_SGRBG10[] = "SGRBG10";
const char RK_HAL_FMT_STRING::HAL_FMT_STRING_SRGGB10[] = "SRGGB10";
const char RK_HAL_FMT_STRING::HAL_FMT_STRING_SBGGR8[] = "SBGGR8";
const char RK_HAL_FMT_STRING::HAL_FMT_STRING_SGBRG8[] = "SGBRG8";
const char RK_HAL_FMT_STRING::HAL_FMT_STRING_SGRBG8[] = "SGRBG8";
const char RK_HAL_FMT_STRING::HAL_FMT_STRING_SRGGB8[] = "SRGGB8";

RK_FRMAE_FORMAT V4l2FmtToHalFmt(unsigned int v4l2fmt) {
  switch (v4l2fmt) {
    case V4L2_PIX_FMT_NV12 :
      return HAL_FRMAE_FMT_NV12;
    case V4L2_PIX_FMT_NV21 :
      return HAL_FRMAE_FMT_NV21;
    case V4L2_PIX_FMT_YVU420 :
      return HAL_FRMAE_FMT_YVU420;
    case V4L2_PIX_FMT_RGB565 :
      return HAL_FRMAE_FMT_RGB565;
    case V4L2_PIX_FMT_RGB32  :
      return HAL_FRMAE_FMT_RGB32;
    case V4L2_PIX_FMT_YUV422P :
      return HAL_FRMAE_FMT_YUV422P;
    case V4L2_PIX_FMT_NV16 :
      return HAL_FRMAE_FMT_NV16;
    case V4L2_PIX_FMT_YUYV :
      return HAL_FRMAE_FMT_YUYV;
    case V4L2_PIX_FMT_JPEG :
      return HAL_FRMAE_FMT_JPEG;
    case V4L2_PIX_FMT_MJPEG:
      return HAL_FRMAE_FMT_MJPEG;
    case V4L2_PIX_FMT_H264:
      return HAL_FRMAE_FMT_H264;
    case V4L2_PIX_FMT_GREY:
      return HAL_FRMAE_FMT_Y8;
    case V4L2_PIX_FMT_Y10:
      return HAL_FRMAE_FMT_Y10;
    case V4L2_PIX_FMT_Y12:
      return HAL_FRMAE_FMT_Y12;
    case V4L2_PIX_FMT_SBGGR10:
      return HAL_FRMAE_FMT_SBGGR10;
    case V4L2_PIX_FMT_SGBRG10:
      return HAL_FRMAE_FMT_SGBRG10;
    case V4L2_PIX_FMT_SGRBG10:
      return HAL_FRMAE_FMT_SGRBG10;
    case V4L2_PIX_FMT_SRGGB10:
      return HAL_FRMAE_FMT_SRGGB10;
    case V4L2_PIX_FMT_SBGGR8:
      return HAL_FRMAE_FMT_SBGGR8;
    case V4L2_PIX_FMT_SGBRG8:
      return HAL_FRMAE_FMT_SGBRG8;
    case V4L2_PIX_FMT_SGRBG8:
      return HAL_FRMAE_FMT_SGRBG8;
    case V4L2_PIX_FMT_SRGGB8:
      return HAL_FRMAE_FMT_SRGGB8;
    case HAL_FRMAE_FMT_MAX :
    default :
      return HAL_FRMAE_FMT_MAX;
  }
}

unsigned int halFmtToV4l2Fmt(unsigned int halFmt) {
  switch (halFmt) {
    case HAL_FRMAE_FMT_NV12 :
      return V4L2_PIX_FMT_NV12;
    case HAL_FRMAE_FMT_NV21 :
      return V4L2_PIX_FMT_NV21;
    case HAL_FRMAE_FMT_YVU420 :
      return V4L2_PIX_FMT_YVU420;
    case HAL_FRMAE_FMT_RGB565 :
      return V4L2_PIX_FMT_RGB565;
    case HAL_FRMAE_FMT_RGB32 :
      return V4L2_PIX_FMT_RGB32;
    case HAL_FRMAE_FMT_YUV422P :
      return V4L2_PIX_FMT_YUV422P;
    case HAL_FRMAE_FMT_NV16 :
      return V4L2_PIX_FMT_NV16;
    case HAL_FRMAE_FMT_YUYV :
      return V4L2_PIX_FMT_YUYV;
    case HAL_FRMAE_FMT_JPEG :
      return V4L2_PIX_FMT_JPEG;
    case HAL_FRMAE_FMT_MJPEG :
      return V4L2_PIX_FMT_MJPEG;
    case HAL_FRMAE_FMT_H264 :
      return V4L2_PIX_FMT_H264;
    case HAL_FRMAE_FMT_Y8 :
      return V4L2_PIX_FMT_GREY;
    case HAL_FRMAE_FMT_Y10 :
      return V4L2_PIX_FMT_Y10;
    case HAL_FRMAE_FMT_Y12 :
      return V4L2_PIX_FMT_Y12;
    case HAL_FRMAE_FMT_SBGGR10 :
      return V4L2_PIX_FMT_SBGGR10;
    case HAL_FRMAE_FMT_SGBRG10 :
      return V4L2_PIX_FMT_SGBRG10;
    case HAL_FRMAE_FMT_SGRBG10 :
      return V4L2_PIX_FMT_SGRBG10;
    case HAL_FRMAE_FMT_SRGGB10 :
      return V4L2_PIX_FMT_SRGGB10;
    case HAL_FRMAE_FMT_SBGGR8 :
      return V4L2_PIX_FMT_SBGGR8;
    case HAL_FRMAE_FMT_SGBRG8 :
      return V4L2_PIX_FMT_SGBRG8;
    case HAL_FRMAE_FMT_SGRBG8 :
      return V4L2_PIX_FMT_SGRBG8;
    case HAL_FRMAE_FMT_SRGGB8 :
      return V4L2_PIX_FMT_SRGGB8;
    case HAL_FRMAE_FMT_MAX :
    default :
      return 0;
  }
}

unsigned int halColorSpaceToV4l2ColorSpace(unsigned int halColorSpace) {
  switch (halColorSpace) {
    case HAL_COLORSPACE_SMPTE170M :
      return V4L2_COLORSPACE_SMPTE170M;
    case HAL_COLORSPACE_JPEG :
      return V4L2_COLORSPACE_JPEG;
    default :
      return 0;
  }
}

