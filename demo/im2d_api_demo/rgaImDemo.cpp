#include "im2d_api/im2d.hpp"
#include "args.h"
#include <stdint.h>
#include <sys/types.h>
#include <math.h>
#include <fcntl.h>

#include "RockchipFileOps.h"

sp<GraphicBuffer> GraphicBuffer_Init(int width, int height,int format)
{
#ifdef ANDROID_7_DRM
    sp<GraphicBuffer> gb(new GraphicBuffer(width,height,format,
        GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_HW_FB));
#else
    sp<GraphicBuffer> gb(new GraphicBuffer(width,height,format,
        GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN));
#endif

    if (gb->initCheck())
    {
        printf("GraphicBuffer check error : %s\n",strerror(errno));
        return NULL;
    }
    else
        printf("GraphicBuffer check %s \n","ok");

    return gb;
}

#define write_buff 0
#define Empty_buff 1

/********** write data to buffer or init buffer**********/
int Write_GraphicBuffer(sp<GraphicBuffer> gb, bool flag, int index)
{
    int ret;
	  char* buf = NULL;
    ret = gb->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&buf);

    if (ret) {
        printf("lock buffer error : %s\n",strerror(errno));
        return -1;
    } else
        printf("lock buffer %s \n","ok");

    if(flag)
        memset(buf,0x00,gb->getPixelFormat()*gb->getWidth()*gb->getHeight());
    else
    {
        ret = get_buf_from_file(buf, gb->getPixelFormat(), gb->getWidth(), gb->getHeight(), index);
        if (!ret)
            printf("open file\n");
        else
            printf ("can not open file\n");
    }
    printf("flag = %d", flag);

    ret = gb->unlock();
	  if (ret) {
        printf("unlock buffer error : %s\n",strerror(errno));
        return -1;
    } else
        printf("unlock buffer %s \n","ok");

    return 0;
}

const char * GetError(IM_STATUS status)
{
    switch(status)
    {
        case IM_STATUS_SUCCESS :
            return "Succed!";

        case IM_STATUS_NOT_SUPPORTED :
            return "Not supported!";

        case IM_STATUS_OUT_OF_MEMORY :
            return "Out of memory!";

        case IM_STATUS_INVALID_PARAM :
            return "Invalid parameter!";

        case IM_STATUS_FAILED :
            return "Anther failed!";
    }
}

int main(int argc, char*  argv[])
{
    int ret;
    int rotation = 0, flip = 0, color = 0;
    const int *parm_data = NULL;
    int srcWidth,srcHeight,srcFormat;
    int dstWidth,dstHeight,dstFormat;

    mode_code MODE;
    querystring_info IM_INFO;
    IM_STATUS STATUS;

    im_rect src_rect;
    im_rect dst_rect;
    buffer_t src;
    buffer_t dst;

    sp<GraphicBuffer> src_buf;
    sp<GraphicBuffer> dst_buf;

    /********** SrcInfo set **********/
    srcWidth  = 1280;
    srcHeight = 720;
    srcFormat = HAL_PIXEL_FORMAT_RGBA_8888;

    /********** DstInfo set **********/
    dstWidth  = 1280;
    dstHeight = 720;
    dstFormat = HAL_PIXEL_FORMAT_RGBA_8888;

    MODE = readArguments(argc, argv);
    /********** Get parameters **********/
    if(MODE != MODE_QUERYSTRING)
    {
        src_buf = GraphicBuffer_Init(srcWidth, srcHeight, srcFormat);
        dst_buf = GraphicBuffer_Init(dstWidth, dstHeight, dstFormat);
        if (src_buf == NULL || dst_buf == NULL)
        {
            printf("GraphicBuff init error!\n");
            return -1;
        }

        Write_GraphicBuffer(src_buf, write_buff, 0);
        if(MODE == MODE_BLEND)
            Write_GraphicBuffer(dst_buf, write_buff, 1);
        else
            Write_GraphicBuffer(dst_buf, Empty_buff, 1);

        src = warpbuffer_GraphicBuffer(src_buf);
        dst = warpbuffer_GraphicBuffer(dst_buf);
    }

    /********** Execution function according to mode **********/
    switch(MODE)
    {
        case MODE_QUERYSTRING :

            IM_INFO = readInfo(argc, argv);
            printf("\n%s\n", querystring(IM_INFO));

            break;

        case MODE_COPY :      //rgaImDemo -copy

            STATUS = imcopy(src, dst);
            printf("copying .... %s\n", GetError(STATUS));

            break;

        case MODE_RESIZE :    //rgaImDemo -resize -up/down

            parm_data = readParm(argc, argv);

            switch(readResizeMode(parm_data))
            {
                case UP_RESIZE :

                    dst_buf = GraphicBuffer_Init(1920, 1080, dstFormat);
                    if (dst_buf == NULL)
                    {
                        printf("dst GraphicBuff init error!\n");
                        return -1;
                    }
                    Write_GraphicBuffer(dst_buf, Empty_buff, 1);
                    dst = warpbuffer_GraphicBuffer(dst_buf);

                    break;
                case DOWN_RESIZE :

                    dst_buf = GraphicBuffer_Init(720, 480, dstFormat);
                    if (dst_buf == NULL)
                    {
                        printf("dst GraphicBuff init error!\n");
                        return -1;
                    }
                    Write_GraphicBuffer(dst_buf, Empty_buff, 1);
                    dst = warpbuffer_GraphicBuffer(dst_buf);

                    break;
            }

            STATUS = imresize(src, dst);
            printf("resizing .... %s\n", GetError(STATUS));

            break;

        case MODE_CROP :      //rgaImDemo -crop

            src_rect.x      = 100;
            src_rect.y      = 100;
            src_rect.width  = 300;
            src_rect.height = 300;

            STATUS = imcrop(src, dst, src_rect);
            printf("cropping .... %s\n", GetError(STATUS));

            break;

        case MODE_ROTATE :    //rgaImDemo -rotate -90/180/270

                parm_data = readParm(argc, argv);
            rotation = readRotationMode(parm_data);

            STATUS = imrotate(src, dst, rotation);
            printf("rotating .... %s\n", GetError(STATUS));

            break;

        case MODE_FLIP :      //rgaImDemo -flip -H/V

                parm_data = readParm(argc, argv);

            flip = readFlipMode(parm_data);

            STATUS = imflip(src, dst, flip);
            printf("flipping .... %s\n", GetError(STATUS));

            break;

        case MODE_TRANSLATE : //rgaImDemo -translate

            src_rect.x = 300;
            src_rect.y = 300;

            STATUS = imtranslate(src, dst, src_rect.x, src_rect.y);
            printf("translating .... %s\n", GetError(STATUS));

            break;

        case MODE_BLEND :     //rgaImDemo -blend

            STATUS = imblend(src, src, dst);
            printf("blending .... %s\n", GetError(STATUS));

            break;

        case MODE_CVTCOLOR :  //rgaImDemo -cvtcolor

            src.format = HAL_PIXEL_FORMAT_RGBA_8888;
            dst.format = HAL_PIXEL_FORMAT_YCrCb_NV12;

            STATUS = imcvtcolor(src, dst, src.format, dst.format);
            printf("cvtcolor .... %s\n", GetError(STATUS));

            break;

        case MODE_FILL :      //rgaImDemo -fill -blue/green/red

            parm_data = readParm(argc, argv);

            color = readColor(parm_data);

            dst_rect.x      = 100;
            dst_rect.y      = 100;
            dst_rect.width  = 300;
            dst_rect.height = 300;

            STATUS = imfill(dst, dst_rect, color);
            printf("filling .... %s\n", GetError(STATUS));

            break;

        case MODE_NONE :

            printf("Unknown mode\n");

            break;

        default :

            printf("Invalid mode\n");

            break;
    }

    /********** output buf data to file **********/
    char* dstbuf = NULL;
    if (dst_buf != NULL)
    {
        ret = dst_buf->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&dstbuf);
        output_buf_data_to_file(dstbuf, dst.format, dst.width, dst.height, 0);
        ret = dst_buf->unlock();
    }

    return 0;
}

