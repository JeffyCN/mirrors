#include <linux/v4l2-subdev.h>
#include <linux/videodev2.h>
#include "linux/rk-video-format.h"
#include "code_to_pixel_format.h"
#include "xcam_log.h"


RKAIQ_BEGIN_DECLARE
uint32_t get_v4l2_pixelformat(uint32_t pixelcode)
{
    uint32_t pixelformat = -1;

    switch (pixelcode) {
    case MEDIA_BUS_FMT_SRGGB8_1X8:
        pixelformat = V4L2_PIX_FMT_SRGGB8;
        break;
    case MEDIA_BUS_FMT_SBGGR8_1X8:
        pixelformat = V4L2_PIX_FMT_SBGGR8;
        break;
    case MEDIA_BUS_FMT_SGBRG8_1X8:
        pixelformat = V4L2_PIX_FMT_SGBRG8;
        break;
    case MEDIA_BUS_FMT_SGRBG8_1X8:
        pixelformat = V4L2_PIX_FMT_SGRBG8;
        break;
    case MEDIA_BUS_FMT_SBGGR10_1X10:
        pixelformat = V4L2_PIX_FMT_SBGGR10;
        break;
    case MEDIA_BUS_FMT_SRGGB10_1X10:
        pixelformat = V4L2_PIX_FMT_SRGGB10;
        break;
    case MEDIA_BUS_FMT_SGBRG10_1X10:
        pixelformat = V4L2_PIX_FMT_SGBRG10;
        break;
    case MEDIA_BUS_FMT_SGRBG10_1X10:
        pixelformat = V4L2_PIX_FMT_SGRBG10;
        break;
    case MEDIA_BUS_FMT_SRGGB12_1X12:
        pixelformat = V4L2_PIX_FMT_SRGGB12;
        break;
    case MEDIA_BUS_FMT_SBGGR12_1X12:
        pixelformat = V4L2_PIX_FMT_SBGGR12;
        break;
    case MEDIA_BUS_FMT_SGBRG12_1X12:
        pixelformat = V4L2_PIX_FMT_SGBRG12;
        break;
    case MEDIA_BUS_FMT_SGRBG12_1X12:
        pixelformat = V4L2_PIX_FMT_SGRBG12;
        break;
    case MEDIA_BUS_FMT_Y8_1X8:
        pixelformat = V4L2_PIX_FMT_GREY;
        break;
    case MEDIA_BUS_FMT_Y10_1X10:
        pixelformat = V4L2_PIX_FMT_Y10;
        break;
    case MEDIA_BUS_FMT_Y12_1X12:
        pixelformat = V4L2_PIX_FMT_Y12;
        break;
    case MEDIA_BUS_FMT_SPD_2X8:
        pixelformat = V4l2_PIX_FMT_SPD16;
        break;
    case MEDIA_BUS_FMT_SBGGR16_1X16:
        pixelformat = V4L2_PIX_FMT_SBGGR16;
        break;
    case MEDIA_BUS_FMT_SGBRG16_1X16:
        pixelformat = V4L2_PIX_FMT_SGBRG16;
        break;
    case MEDIA_BUS_FMT_SGRBG16_1X16:
        pixelformat = V4L2_PIX_FMT_SGRBG16;
        break;
    case MEDIA_BUS_FMT_SRGGB16_1X16:
        pixelformat = V4L2_PIX_FMT_SRGGB16;
        break;
    default:
        //TODO add other
        LOGE("%s no support pixelcode:0x%x\n",
                        __func__, pixelcode);
    }
    return pixelformat;
}
RKAIQ_END_DECLARE

