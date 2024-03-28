/*
 *  Copyright (c) 2022 Rockchip Corporation
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
 *
 */

#ifndef _RK_ISPFEC_API_H_
#define _RK_ISPFEC_API_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rk_ispfec_ctx_s rk_ispfec_ctx_t;

enum rk_ispfec_update_mesh_mode {
    RK_ISPFEC_UPDATE_MESH_ONLINE              = 0,    	// generate lut online
    RK_ISPFEC_UPDATE_MESH_FROM_FILE,          			// external file import mesh
};

enum rk_ispfec_correct_direction {
    RK_ISPFEC_CORRECT_DIRECTION_X = 0x1,
    RK_ISPFEC_CORRECT_DIRECTION_Y,
    RK_ISPFEC_CORRECT_DIRECTION_XY
};

enum rk_ispfec_correct_style {
    RK_ISPFEC_KEEP_ASPECT_RATIO_REDUCE_FOV = 0x1,
    RK_ISPFEC_COMPRES_IMAGE_KEEP_FOV,
};

/**
 * the following prarams are only effective in 'RK_ISPFEC_UPDATE_MESH_ON_LINE'.
 *
 * @light_center: the optical center of the lens.
 * @coefficient: the distortion coefficient of the lens.
 * @direction: configure the correction direction of the generated mesh.
 * @style: configure the correction style of the generated mesh.
 * @correct_level: the distortion type of FEC.
 */
typedef struct gen_mesh_online_info_s {
    double  light_center[2];
    double  coeff[4];
    enum rk_ispfec_correct_direction 	direction;
    enum rk_ispfec_correct_style 		style;

    int     correct_level;
} gen_mesh_online_info_t;

typedef struct mesh_info_s {
    int dmaFd;
    void* vir_addr;
    int size;
    char mesh_file[64];
} mesh_info_t;

typedef struct rk_ispfec_cfg_s {
    // 输入输出的分辨率
    int in_width;
    int in_height;
    int out_width;
    int out_height;
    // 输入输出的 v4l2 fmt
    int in_fourcc;
    int out_fourcc;
    // mesh 表 buf fd，外部分配，可用 rk_ispfec_api_calFecMeshsize 计算size
    mesh_info_t mesh_xint;
    mesh_info_t mesh_xfra;
    mesh_info_t mesh_yint;
    mesh_info_t mesh_yfra;

    enum rk_ispfec_update_mesh_mode mesh_upd_mode;

    /**
     * @UPDATE_MESH_ONLINE :   configure the online info from fec calibration for 'mesh_file_path'
     * @UPDATE_MESH_FROM_FILE: configure the path of mesh file for 'mesh_online'
     */
    union {
        char mesh_file_path[128];
        gen_mesh_online_info_t mesh_online;
    } u;
} rk_ispfec_cfg_t;

rk_ispfec_ctx_t* rk_ispfec_api_init(rk_ispfec_cfg_t* cfg);

// prepare 时cfg可为 NULL，为NULL时使用 init时的。
// prepare 可用于 reconfig 参数
int rk_ispfec_api_prepare(rk_ispfec_ctx_t* ctx, rk_ispfec_cfg_t* cfg);

int rk_ispfec_api_process(rk_ispfec_ctx_t* ctx, int src_fd, int dst_fd);

void rk_ispfec_api_deinit(rk_ispfec_ctx_t* ctx);

// helper functions
// 用于计算 mesh buf 所需 size
int rk_ispfec_api_calFecMeshsize(int width, int height);

#ifdef __cplusplus
}
#endif

#endif
