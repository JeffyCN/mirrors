#include "args.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "im2d_api/im2d.h"

mode_code readArguments(int argc, char *argv[])
{
    int i = 1;
    //read the arguments of the program
    if(argc <= 1)
    {
        goto out;
    }
    if (strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"-help") == 0 )
    {
        return MODE_NONE;
    }
    else if (strcmp(argv[i],"-vendor") == 0 )
    {
        printf("\nim2d querystring ..\n");
        return MODE_QUERYSTRING;
    }
    else if (strcmp(argv[i],"-v") == 0 || strcmp(argv[i],"-version") == 0 )
    {
        printf("\nim2d querystring ..\n");
        return MODE_QUERYSTRING;
    }
    else if (strcmp(argv[i],"-mi") == 0 || strcmp(argv[i],"-maxinput") == 0 )
    {
        printf("\nim2d querystring ..\n");
        return MODE_QUERYSTRING;
    }
    else if (strcmp(argv[i],"-mo") == 0 || strcmp(argv[i],"-maxoutput") == 0 )
    {
        printf("\nim2d querystring ..\n");
        return MODE_QUERYSTRING;
    }
    else if (strcmp(argv[i],"-s") == 0 || strcmp(argv[i],"-scale") == 0 )
    {
        printf("\nim2d querystring ..\n");
        return MODE_QUERYSTRING;
    }
    else if (strcmp(argv[i],"-if") == 0 || strcmp(argv[i],"-inputformat") == 0 )
    {
        printf("\nim2d querystring ..\n");
        return MODE_QUERYSTRING;
    }
    else if (strcmp(argv[i],"-of") == 0 || strcmp(argv[i],"-outputformat") == 0 )
    {
        printf("\nim2d querystring ..\n");
        return MODE_QUERYSTRING;
    }
    else if (strcmp(argv[i],"-a") == 0 || strcmp(argv[i],"-all") == 0 )
    {
        printf("\nim2d querystring ..\n");
        return MODE_QUERYSTRING;
    }
    //read the image name
    else if(strcmp(argv[i],"-copy")==0)
    {
        printf("im2d copy ..\n");
        return MODE_COPY;
    }
    else if(strcmp(argv[i],"-resize")==0)
    {
        printf("im2d resize ..\n");
        return MODE_RESIZE;
    }
    else if(strcmp(argv[i],"-crop")==0)
    {
        printf("im2d crop ..\n");
        return MODE_CROP;
    }
    else if(strcmp(argv[i],"-rotate")==0)
    {
        printf("im2d rotate ..\n");
        return MODE_ROTATE;
    }
    else if(strcmp(argv[i],"-flip")==0)
    {
        printf("im2d flip ..\n");
        return MODE_FLIP;
    }
    else if(strcmp(argv[i],"-translate")==0)
    {
        printf("im2d translate ..\n");
        return MODE_TRANSLATE;
    }
    else if(strcmp(argv[i],"-blend")==0)
    {
        printf("im2d blend ..\n");
        return MODE_BLEND;
    }
    else if(strcmp(argv[i],"-cvtcolor")==0)
    {
        printf("im2d cvtcolor ..\n");
        return MODE_CVTCOLOR;
    }
    else if(strcmp(argv[i],"-fill")==0)
    {
        printf("im2d fill ..\n");
        return MODE_FILL;
    }
    else
    {
        goto out;
    }
out :
    printf("Invalid parameter\n");
    return MODE_NONE;
}

querystring_info readInfo(int argc, char *argv[])
{
    int i = 1;
    //read the arguments of the program
    if(argc <= 1)
    {
        goto out;
    }
    if (strcmp(argv[i],"-vendor") == 0 )
    {
        printf("\nim2d querystring : vendor ..\n");
        return INFO_VENDOR;
    }
    else if (strcmp(argv[i],"-v") == 0 || strcmp(argv[i],"-version") == 0 )
    {
        printf("\nim2d querystring : vendor ..\n");
        return INFO_VERSION;
    }
    else if (strcmp(argv[i],"-mi") == 0 || strcmp(argv[i],"-maxinput") == 0 )
    {
        printf("\nim2d querystring : max input ..\n");
        return INFO_MAX_INPUT;
    }
    else if (strcmp(argv[i],"-mo") == 0 || strcmp(argv[i],"-maxoutput") == 0 )
    {
        printf("\nim2d querystring : max output ..\n");
        return INFO_MAX_OUTPUT;
    }
    else if (strcmp(argv[i],"-s") == 0 || strcmp(argv[i],"-scale") == 0 )
    {
        printf("\nim2d querystring : scale limit ..\n");
        return INFO_SCALE_LIMIT;
    }
    else if (strcmp(argv[i],"-if") == 0 || strcmp(argv[i],"-inputformat") == 0 )
    {
        printf("\nim2d querystring : input format ..\n");
        return INFO_INPUT_FORMAT;
    }
    else if (strcmp(argv[i],"-of") == 0 || strcmp(argv[i],"-outputformat") == 0 )
    {
        printf("\nim2d querystring : output format ..\n");
        return INFO_OUTPUT_FORMAT;
    }
    else if (strcmp(argv[i],"-a") == 0 || strcmp(argv[i],"-all") == 0 )
    {
        printf("\nim2d querystring : all ..\n");
        return INFO_ALL;
    }
    else
    {
        goto out;
    }
    out :
    printf("Invalid infomation, print all\n");
    return INFO_ERR;
}

const int * readParm(int argc, char *argv[])
{
  	static int parm[30];
  	int i = 2;

    if(argc <= 1)
    {
        goto out;
    }
    if (strcmp(argv[i],"-up") == 0 )
    {
        printf("\nup resize ...\n");
        parm[RESIZE_MODE] = 0;
        return (const int*)parm;
    }
    else if (strcmp(argv[i],"-down") == 0)
    {
        printf("\ndown resize ...\n");
        parm[RESIZE_MODE] = 1;
        return (const int*)parm;
    }
    else if (strcmp(argv[i],"-90") == 0)
    {
        printf("\nrotation 90 ...\n");
        parm[ROTATION_MODE] = 90;
        return (const int*)parm;
    }
    else if (strcmp(argv[i],"-180") == 0)
    {
        printf("\nnrotation 180 ...\n");
        parm[ROTATION_MODE] = 180;
        return (const int*)parm;
    }
    else if (strcmp(argv[i],"-270") == 0)
    {
        printf("\nnrotation 270 ...\n");
        parm[ROTATION_MODE] = 270;
        return (const int*)parm;
    }
    else if (strcmp(argv[i],"-H") == 0)
    {
        printf("\nflip H ...\n");
        parm[FLIP_MODE] = 0;
        return (const int*)parm;
    }
    else if (strcmp(argv[i],"-V") == 0)
    {
        printf("\nflip V ...\n");
        parm[FLIP_MODE] = 1;
        return (const int*)parm;
    }
    else if (strcmp(argv[i],"-blue") == 0)
    {
        printf("\nfill blue ...\n");
        parm[COLOR] = 0;
        return (const int*)parm;
    }
    else if (strcmp(argv[i],"-green") == 0)
    {
        printf("\nfill green ...\n");
        parm[COLOR] = 1;
        return (const int*)parm;
    }
    else if (strcmp(argv[i],"-red") == 0)
    {
        printf("\nfill red ...\n");
        parm[COLOR] = 2;
        return (const int*)parm;
    }
    else
    {
        goto out;
    }

out :
    printf("Invalid infomation, print all\n");
    return NULL;
}

int readResizeMode(const int * parm)
{
    switch(parm[RESIZE_MODE])
    {
        case 0 :
            return UP_RESIZE;
        case 1 :
            return DOWN_RESIZE;
    }

    return -1;
}


int readRotationMode(const int * parm)
{
    switch(parm[ROTATION_MODE])
    {
        case 90 :
            return IM_HAL_TRANSFORM_ROT_90;
        case 180 :
            return IM_HAL_TRANSFORM_ROT_180;
        case 270 :
            return IM_HAL_TRANSFORM_ROT_270;
    }

    return -1;
}
int readFlipMode(const int * parm)
{
    switch(parm[FLIP_MODE])
    {
        case 0 :
            return IM_HAL_TRANSFORM_FLIP_H;
        case 1 :
            return IM_HAL_TRANSFORM_FLIP_V;
    }
    return -1;
}

int readColor(const int * parm)
{
    switch(parm[COLOR])
    {
        case 0 :
            return 0xffff0000;//蓝色
        case 1 :
            return 0xff00ff00;//绿色
        case 2 :
            return 0xff0000ff;//红色
    }

    return -1;
}


