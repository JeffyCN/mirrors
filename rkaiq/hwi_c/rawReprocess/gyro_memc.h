#ifndef GYRO_MEMC_H
#define GYRO_MEMC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef _WIN32
#include "NEON_2_SSE.h"
#else
#include "arm_neon.h"
#endif

typedef double timeStampDataType;

#define MAX_GYRO_LIST_SAMPLES 500

#define use_hw_interpolation	1
#define ColBLKsize				32

#define RKLDC_IDXTAB_DOWNSCALE_COL_BIT           4
#define RKLDC_IDXTAB_DOWNSCALE_COL               (1 << RKLDC_IDXTAB_DOWNSCALE_COL_BIT)
#define RKLDC_IDXTAB_X_DOWNSCALE_ROW_BIT         3
#define RKLDC_IDXTAB_X_DOWNSCALE_ROW             (1 << RKLDC_IDXTAB_X_DOWNSCALE_ROW_BIT)
#define RKLDC_IDXTAB_Y_DOWNSCALE_ROW_BIT         4
#define RKLDC_IDXTAB_Y_DOWNSCALE_ROW             (1 << RKLDC_IDXTAB_Y_DOWNSCALE_ROW_BIT)

#ifdef __cplusplus
extern "C" {
#endif
typedef struct
{
    timeStampDataType timeStamp;
    double dTemp;
    double gyroData[3];
    double accData[3];
} rk_imu_data;

typedef struct
{
    double R_T[3][3];
    double H[3][3];
    int large_distort;
    int frame_matrix_id;
} rk_matrix_data;

typedef struct
{
    timeStampDataType timestamp_offset;
    int earlier_imu_idx;

    // internal param
    timeStampDataType timestamp[2]; // 0:iir, 1:cur
    rk_imu_data imu_data_internal_list[MAX_GYRO_LIST_SAMPLES];
    int list_valid_num;
    int list_start_idx;

    int large_distort;

    // camera_intrinsics
    int sensor_axes_type;
    double focal_length_x;
    double focal_length_y;
    double center_x;
    double center_y;

    double k1;
    double k2;
    double p1;
    double p2;
    double k3;
    double xi;

    int vertical_start_idx;
    int horizontal_start_idx;

    bool bias_initialized;
    int bias_sample_count;
    double bias_sum[3];
    double gyro_bias[3];

    rk_matrix_data m_data[2];

    int* mv_i;
    int* mv_j;
    uint16_t* unpack_pk_ptr;
    uint16_t* unpack_pk_warp_ptr;

    int align_128pix;

    int unpack_start_idx;
    int warp_start_idx;
    int warp_end_idx;

    int log_en;
} rk_gyro_memc_internal_param_t;

typedef struct
{
    int frame_idx;
    int enable_default_map;

    int	raw_wid;
    int	raw_hgt;
    int rk_b3dldc_itp_mode;
    int warp_extBound_set;
    int	map_fix_bit_y;
    int	map_fix_bit_x;
    uint8_t* cur_frame_ptr;
    uint8_t* ref_frame_ptr;

    double iso_speed;
    double exp_time;
    double rs_skew;
    double again;
    double dgain;
    double ispgain;
    double iso;

    // camera_intrinsics
    int sensor_axes_type;
    double focal_length_x;
    double focal_length_y;
    double center_x;
    double center_y;

    double k1;
    double k2;
    double p1;
    double p2;
    double k3;
    double xi;

    int vertical_start_idx;
    int horizontal_start_idx;

    int online_prediction_mode_en;

    // external param
    timeStampDataType camera_timestamp;
    rk_imu_data imu_data_external_list[MAX_GYRO_LIST_SAMPLES];
    int imu_data_num;

    uint16_t *map_mesh_fix_ptr;
    uint16_t* mapX_start_fix_ptr;

    int b3dldc_offset;

    int effect_line_cnt;
    int process_idx;
    int event_idx;

    uint16_t* pk_ptr;

    int use_external_h;
    double external_h[3][3];
} rk_gyro_memc_param_t;

extern rk_gyro_memc_internal_param_t* rk_gyro_memc_init(rk_gyro_memc_param_t* param);
extern int rk_gyro_memc_proc(rk_gyro_memc_internal_param_t* ctx, rk_gyro_memc_param_t* param);
extern int rk_gyro_memc_pk_warp(rk_gyro_memc_internal_param_t* ctx, rk_gyro_memc_param_t* param);
extern int rk_gyro_memc_default_map(rk_gyro_memc_internal_param_t* ctx, rk_gyro_memc_param_t* param);
extern int rk_gyro_memc_wrong_map(rk_gyro_memc_internal_param_t* ctx, rk_gyro_memc_param_t* param);
extern int rk_gyro_memc_deinit(rk_gyro_memc_internal_param_t* ctx);

#ifdef __cplusplus
}
#endif

#endif // GYRO_MEMC_H
