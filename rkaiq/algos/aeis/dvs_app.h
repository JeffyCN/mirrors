#ifndef __DVS_APP_H__
#define __DVS_APP_H__

#include <stdint.h>

#include "rk_aiq_mems_sensor.h"

/*
 * @enum    rk_dvs_format_type_t
 * @brief   Specify the format of input image
 */
typedef enum {
    RK_DVS_PIXEL_FMT_GRAY, /**< Gray */
    RK_DVS_PIXEL_FMT_NV21, /**< NV21 */
    RK_DVS_PIXEL_FMT_NV12, /**< NV12 */
    RK_DVS_PIXEL_FMT_NUM,
} rk_dvs_format_type_t;

#if 0
typedef struct sensor_vec_s {
    union {
        double v[3];
        struct {
            double x;
            double y;
            double z;
        };
    };
} sensor_vec_t;

typedef struct uncalib_event_s {
    union {
        double uncalib[3];
        struct {
            double x_uncalib;
            double y_uncalib;
            double z_uncalib;
        };
    };
    union {
        double bias[3];
        struct {
            double x_bias;
            double y_bias;
            double z_bias;
        };
    };
} uncalib_event_t;

typedef sensor_vec_t gyro_data_t;
typedef sensor_vec_t accel_data_t;
typedef int temperature_data_t;

typedef struct sensor_vec_all_s {
    gyro_data_t gyro;
    accel_data_t accel;
    temperature_data_t temperature;
} sensor_vec_all_t;

typedef struct mems_sensor_event_s {
    uint64_t id;
    uint64_t timestamp_us;
    union {
        double data[16];
        accel_data_t accel;
        gyro_data_t gyro;
        temperature_data_t temperature;
        sensor_vec_all_t all;

        uncalib_event_t uncalib_gyro;
        uncalib_event_t uncalib_accel;
    };
} mems_sensor_event_t;
#endif

struct dvsImageSize {
    int width;
    int height;
};

// initial params
struct initialParams {
    rk_dvs_format_type_t image_format;  // input image format
    dvsImageSize input_image_size;      // input image size
    dvsImageSize output_image_size;     // output image size
    int image_buffer_number;            // number of image buffer
    int image_stride;                   // input image width width extra invalid pixels
    int auto_scaling;                   // output image will be upscale
    double clip_ratio_x;                // clip ratio in x direction
    double clip_ratio_y;                // clip ratio in y direction
};

struct dvsMetaData {
    int iso_speed;    // iso speed
    double exp_time;  // exposure time. unit: second.
    double
        rolling_shutter_skew;  // unit: second. Duration between the start of exposure for the first
                               // row of the image sensor,and the start of exposure for one past the
    double zoom_ratio;         // zoom ratio
    uint64_t timestamp_sof_us;  // sof time
};

// input image params
struct imageData {
    int buffer_index;         // image buffer index
    int frame_index;          // image index
    int pyramid_number;       // pyramid number,Range[0-4](1/2,1/4,1/8,1/16)
    int* pyramid_stride;      // pyramid image stride
    uint8_t** pdata_pyramid;  // pyramid image
    uint8_t* pdata_original;  // original image
    dvsMetaData meta_data;    // image meta data
};

//输出结构体
struct meshxyFEC {
    int mesh_buffer_index;   // mesh index
    int image_buffer_index;  // mesh表对应图像buffer的index
    int image_index;         // mesh表对应图像的index
    int is_skip;  //是否跳帧，0:不跳帧;1:跳帧，跳帧时，输出的mesh表为上一帧的mesh表
    unsigned long mesh_size;  // mesh表的长度
    // FEC所需的四个小表
    unsigned short* pMeshXI;
    unsigned char* pMeshXF;
    unsigned short* pMeshYI;
    unsigned char* pMeshYF;
};

//接口结构体
struct dvsEngine {
    void* private_data;  // dvs库所需参数
};

extern "C" {

typedef int (*dvsFrameCallBackFEC)(struct dvsEngine* engine, meshxyFEC* mesh_fec);

/*************************************************************************************************
 *  Copyright (C),2021, Fuzhou Rockchip Co.,Ltd.
 *  Function name : dvsRegisterRemap()
 *  Author:         lmp
 *  Description:    callback function
 *  Input:          engine: Engine instance pointer
 *                  callback: callback function
 *  Return:         ??
 *  History:
 *           <author>      <time>     <version>       <desc>
 *            lmp		 15/03/21      1.0            org
 *
 *************************************************************************************************/
int dvsRegisterRemap(struct dvsEngine* engine, dvsFrameCallBackFEC callback);

/*************************************************************************************************
 *  Copyright (C),2021, Fuzhou Rockchip Co.,Ltd.
 *  Function name : dvsPrepare()
 *  Author:         lmp
 *  Description:    Prepare dvs engine environment
 *  Input:          engine: Engine instance pointer
 *  Return:         ??
 *  History:
 *           <author>      <time>     <version>       <desc>
 *            lmp		 15/03/21      1.0            org
 *
 *************************************************************************************************/
int dvsPrepare(struct dvsEngine* engine);

/*************************************************************************************************
 *  Copyright (C),2021, Fuzhou Rockchip Co.,Ltd.
 *  Function name : getMeshSize()
 *  Author:         lmp
 *  Description:    get mesh size
 *  Input:          image_height: image height
 *                  image_width:  image width
 *                  mesh_size: mesh size
 *  Return:         ??
 *  History:
 *           <author>      <time>     <version>       <desc>
 *            lmp		 15/03/21      1.0            org
 *
 *************************************************************************************************/
void getMeshSize(int image_height, int image_width, int* mesh_size);

/*************************************************************************************************
 *  Copyright (C),2021, Fuzhou Rockchip Co.,Ltd.
 *  Function name : getOriginalMeshXY()
 *  Author:         lmp
 *  Description:    get original mesh
 *  Input:          image_height: image height
 *                  image_width:  image width
 *                  clip_ratio_x:
 *                  clip_ratio_y:
 *  Output:         meshxyFEC: mesh
 *  Return:         ??
 *  History:
 *           <author>      <time>     <version>       <desc>
 *            lmp		 15/04/08      1.0            org
 *
 *************************************************************************************************/
void getOriginalMeshXY(int image_width, int image_height, double clip_ratio_x, double clip_ratio_y,
                       meshxyFEC* pmesh_fec);

/*************************************************************************************************
 *  Copyright (C),2021, Fuzhou Rockchip Co.,Ltd.
 *  Function name : dvsPutImageFrame()
 *  Author:         lmp
 *  Description:    put image in dvs
 *  Input:          engine:  Engine instance pointer
 *                  pimage_data: image_data
 *  Return:         ??
 *  History:
 *           <author>      <time>     <version>       <desc>
 *            lmp		 15/03/21      1.0            org
 *
 *************************************************************************************************/
int dvsPutImageFrame(struct dvsEngine* engine, struct imageData* pimage_data);

/*************************************************************************************************
 *  Copyright (C),2021, Fuzhou Rockchip Co.,Ltd.
 *  Function name : dvsPutImageFrame()
 *  Author:         lmp
 *  Description:    put image in dvs
 *  Input:          engine: Engine instance pointer
 *                  pmesh_fec: fec mesh
 *  Return:         ??
 *  History:
 *           <author>      <time>     <version>       <desc>
 *            lmp		 15/03/21      1.0            org
 *
 *************************************************************************************************/
int dvsPutMesh(struct dvsEngine* engine, struct meshxyFEC* pmesh_fec);

/*************************************************************************************************
 *  Copyright (C),2021, Fuzhou Rockchip Co.,Ltd.
 *  Function name : dvsPutImuFrame()
 *  Author:         lmp
 *  Description:    put imu data in dvs
 *  Input:          engine: Engine instance pointer
 *                  pimu_data: imu data
 *                  buff_number:  imu buffer number
 *  Return:         ??
 *  History:
 *           <author>      <time>     <version>       <desc>
 *            lmp		 15/03/21      1.0            org
 *
 *************************************************************************************************/
int dvsPutImuFrame(struct dvsEngine* engine, mems_sensor_event_s* pimu_data, int buff_number);

/*************************************************************************************************
 *  Copyright (C),2021, Fuzhou Rockchip Co.,Ltd.
 *  Function name : dvsInitFromXmlFile()
 *  Author:         lmp
 *  Description:    initialize dvs from xml
 *  Input:          engine: Engine instance pointer
 *                  path:  xml path
 *  Return:         ??
 *  History:
 *           <author>      <time>     <version>       <desc>
 *            lmp		 15/03/21      1.0            org
 *
 *************************************************************************************************/
int dvsInitFromXmlFile(struct dvsEngine* engine, const char* path);

/*************************************************************************************************
 *  Copyright (C),2021, Fuzhou Rockchip Co.,Ltd.
 *  Function name : dvsInitFromFile()
 *  Author:         lmp
 *  Description:    initialize dvs from struct
 *  Input:          engine: Engine instance pointer
 *                  init_params:  initial params struct
 *  Return:         ??
 *  History:
 *           <author>      <time>     <version>       <desc>
 *            lmp		 17/03/31      2.0            org
 *
 *************************************************************************************************/
int dvsInitParams(struct dvsEngine* engine, struct initialParams* init_params);

/*************************************************************************************************
 *  Copyright (C),2021, Fuzhou Rockchip Co.,Ltd.
 *  Function name : dvsStart()
 *  Author:         lmp
 *  Description:    start dvs
 *  Input:          engine: Engine instance pointer
 *  Return:         ??
 *  History:
 *           <author>      <time>     <version>       <desc>
 *            lmp		 15/03/21      1.0            org
 *
 *************************************************************************************************/
int dvsStart(struct dvsEngine* engine);

/*************************************************************************************************
 *  Copyright (C),2021, Fuzhou Rockchip Co.,Ltd.
 *  Function name : dvsRequestStop()
 *  Author:         lmp
 *  Description:    request dvs stop
 *  Input:          engine: Engine instance pointer
 *  Return:         ??
 *  History:
 *           <author>      <time>     <version>       <desc>
 *            lmp		 15/03/21      1.0            org
 *
 *************************************************************************************************/
int dvsRequestStop(struct dvsEngine* engine);

/*************************************************************************************************
 *  Copyright (C),2021, Fuzhou Rockchip Co.,Ltd.
 *  Function name : dvsDeinit()
 *  Author:         lmp
 *  Description:    finish dvs
 *  Input:          engine: Engine instance pointer
 *  Return:         ??
 *  History:
 *           <author>      <time>     <version>       <desc>
 *            lmp		 15/03/21      1.0            org
 *
 *************************************************************************************************/
int dvsDeinit(struct dvsEngine* engine);
}

#endif  // __DVS_APP_H__
