#ifndef ARGS_H
#define ARGS_H

typedef enum _mode_code
{
    MODE_QUERYSTRING = 0,
    MODE_COPY,
    MODE_RESIZE,
    MODE_CROP,
    MODE_ROTATE,
    MODE_FLIP,
    MODE_TRANSLATE,
    MODE_BLEND,
    MODE_CVTCOLOR,
    MODE_FILL,
    MODE_NONE
}mode_code;

typedef enum {
    INFO_VENDOR = 0,
    INFO_VERSION,
    INFO_MAX_INPUT,
    INFO_MAX_OUTPUT,
    INFO_SCALE_LIMIT,
    INFO_INPUT_FORMAT,
    INFO_OUTPUT_FORMAT,
    INFO_ALL,
    INFO_ERR
} querystring_info;

typedef enum {
    RESIZE_MODE = 0,
    ROTATION_MODE,
    FLIP_MODE,
    BLEND_MODE,
    COLOR,
    NONE,
} parm_name;

#define readSrcWidth(parm)        parm[SRC_WIDTH]
#define readSrcHeight(parm)       parm[SRC_HEIGHT]
#define readSrcStrideWidth(parm)  parm[SRC_STRIDE_WIDTH]
#define readSrcStrideHeight(parm) parm[SRC_STRIDE_HEIGHT]
#define readSrcFormat(parm)       parm[SRC_FORMAT]
#define readSrcXOffset(parm)      parm[SRC_X_OFFSET]
#define readSrcYOffset(parm)      parm[SRC_Y_OFFSET]
#define readDstWidth(parm)        parm[DST_WIDTH]
#define readDstHeight(parm)       parm[DST_HEIGHT]
#define readDstStrideWidth(parm)  parm[DST_STRIDE_WIDTH]
#define readDstStrideHeight(parm) parm[DST_STRIDE_HEIGHT]
#define readDstFormat(parm)       parm[DST_FORMAT]
#define readDstXOffset(parm)      parm[DST_X_OFFSET]
#define readDstYOffset(parm)      parm[DST_Y_OFFSET]

#define UP_RESIZE 0
#define DOWN_RESIZE 1



mode_code readArguments(int argc, char *argv[]);
querystring_info readInfo(int argc, char *argv[]);
const int * readParm(int argc, char *argv[]);

int readResizeMode(const int * parm);
int readRotationMode(const int * parm);
int readFlipMode(const int * parm);
int readColor(const int * parm);

#endif

