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

#include "rkfec_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FEC_VERSION_ENCODE(a, b, c) (((a) << 16) + ((b) << 8) + ((c) > 255 ? 255 : (c)))
#define RK_ISP_FEC_API_VERSION      FEC_VERSION_ENCODE(4, 0, 4)

#define RK_ISP_FEC_API_VERSION_MAJOR(v) (((v) >> 16) & 0xff)
#define RK_ISP_FEC_API_VERSION_MINOR(v) (((v) >> 8) & 0xff)
#define RK_ISP_FEC_API_VERSION_PATCH(v) ((v) & 0xff)

#define MAX_FILE_PATH_LEN 128

typedef struct rk_ispfec_ctx_s rk_ispfec_ctx_t;

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
 * @calib_width: the width of the calibration image.
 * @calib_height: the height of the calibration image.
 * @calib_level_maxLimit: the max distortion level of the calibration image.
 */
typedef struct gen_mesh_online_info_s {
    double light_center[2];
    double coeff[4];
    enum rk_ispfec_correct_direction direction;
    enum rk_ispfec_correct_style style;

    int correct_level;

    int calib_width;
    int calib_height;
    int calib_level_maxLimit;
} gen_mesh_online_info_t;

typedef struct mesh_info_s {
    int dmaFd;
    void* vir_addr;
    int size;
    char mesh_file[64];
} mesh_info_t;

typedef struct rk_ispfec_bg_val_s {
    int bg_y;
    int bg_u;
    int bg_v;
} rk_ispfec_bg_val;

/**
 * Buffer mode configuration descriptor (for RK_ISPFEC_UPDATE_MESH_FROM_BUFFER mode)
 *
 * @dma_fd:   DMA buffer file descriptor. Valid only when need_copy=0
 * @vir_addr: Source virtual address for memory copy. Required when need_copy=1
 * @size:     Buffer capacity, must be greater or equal to actual data size
 * @need_copy: Operation mode selector:
 *             - 0: DMA handle mode (directly use dma_fd, ignore vir_addr)
 *             - 1: Memory copy mode (copy from vir_addr to internal DMA buffer)
 */
typedef struct rk_ispfec_buffer_info {
    int dma_fd;
    void* vir_addr;
    size_t size;
    bool need_copy;
} rk_ispfec_buffer_info_t;

/**
 * FEC mesh update mode enumeration
 *
 * RK_ISPFEC_UPDATE_MESH_ONLINE             - Real-time mesh generation using lens parameters
 * RK_ISPFEC_UPDATE_MESH_ONLINE_FROM_INI    - Load lens parameters from INI file
 * RK_ISPFEC_UPDATE_MESH_FROM_FILE          - Load mesh data from external files
 * RK_ISPFEC_UPDATE_MESH_FROM_BUFFER        - Update mesh from memory buffer descriptors
 */
enum rk_ispfec_update_mesh_mode {
    RK_ISPFEC_UPDATE_MESH_ONLINE = 0,
    RK_ISPFEC_UPDATE_MESH_FROM_FILE,
    RK_ISPFEC_UPDATE_MESH_FROM_BUFFER,
    RK_ISPFEC_UPDATE_MESH_ONLINE_FROM_INI
};

/**
 * @brief FEC Mesh Configuration Structure
 *
 * @param mode    Operation mode selector
 * @param params  Mode-specific configuration parameters
 *
 * @section file_mode File Mode (RK_ISPFEC_UPDATE_MESH_FROM_FILE)
 * @note Hardware version specific configurations:
 * - V20: Single mesh file path
 * - Others: Four separate mesh files (X/Y axis integer/fractional)
 *
 * @section buffer_mode Buffer Mode (RK_ISPFEC_UPDATE_MESH_FROM_BUFFER)
 * @note Hardware version specific configurations:
 * - V20: Single buffer descriptor
 * - Others: Four buffer descriptors (X/Y axis integer/fractional)
 *
 * @section online_mode Online Mode (RK_ISPFEC_UPDATE_MESH_ONLINE)
 * Contains lens calibration parameters for real-time mesh generation
 */
typedef struct rk_ispfec_mesh_config {
    enum rk_ispfec_update_mesh_mode mode;

    union {
        struct {
#ifdef RKFEC_HW_V20
            char mesh_path[256];
#else
            char mesh_xint[256];
            char mesh_xfra[256];
            char mesh_yint[256];
            char mesh_yfra[256];
#endif
        } file_cfg;

        struct {
#ifdef RKFEC_HW_V20
            rk_ispfec_buffer_info_t mesh;
#else
            struct {
                rk_ispfec_buffer_info_t xint;
                rk_ispfec_buffer_info_t xfra;
                rk_ispfec_buffer_info_t yint;
                rk_ispfec_buffer_info_t yfra;
            } mesh;
#endif
        } buffer_cfg;

        gen_mesh_online_info_t online_cfg;
    } params;
} rk_ispfec_mesh_config_t;

/**
 * @brief FEC Configuration Structure
 *
 * @version      Must be set to RK_ISP_FEC_API_VERSION for version compatibility
 * @size         Structure size for binary compatibility checks (auto-set via ISP_FEC_CONFIG_INIT)
 * @in_width     Input image width in pixels
 * @in_height    Input image height in pixels
 * @out_width    Output image width in pixels
 * @out_height   Output image height in pixels
 * @in_fourcc    Input pixel format (V4L2 fourcc):
 *               - V4L2_PIX_FMT_NV12: YUV420 semi-planar
 *               - V4L2_PIX_FMT_TILE420: Tiled 4x4 format
 * @out_fourcc   Output pixel format (V4L2 fourcc):
 *               - V4L2_PIX_FMT_NV12: YUV420 semi-planar
 *               - V4L2_PIX_FMT_TILE420: Tiled 4x4 format
 *               - V4L2_PIX_FMT_FBC0: Frame Buffer Compression
 *               - V4L2_PIX_FMT_QUAD: AVSP compatible format
 *
 * @section hw_v20 V20 Hardware Specific
 * @border_mode    Edge handling (0: fill background color, 1: extend border pixels)
 * @cross_buf_mode Overshrink handling (0: fill background color, 1: replicate edges)
 * @bg_val         Background YUV color components (0-255)
 * @mesh_info      Pre-allocated mesh buffer (use rk_ispfec_api_calFecMeshsize for size)
 *
 * @section mesh_params Mesh Configuration
 * @mesh_upd_mode  Mesh update operation mode
 * @u              Union configuration based on mesh update mode:
 *                 - mesh_file_path: External mesh file path (file mode)
 *                 - mesh_online:    Online generation parameters (online mode)
 *
 * @section offsets Offsets and Strides
 * @in_offset_x    Input horizontal crop offset (API 3.0.0+)
 * @out_offset_x   Output horizontal placement offset (API 3.0.0+)
 * @in_stride      Input buffer stride (0=auto-calculate)
 * @out_stride     Output buffer stride (0=auto-calculate)
 * @calib_ini_from Lens calibration file path (API 3.1.0+)
 */
typedef struct rk_ispfec_cfg_s {
    int version;
    int size;
    int in_width;
    int in_height;
    int out_width;
    int out_height;
    int in_fourcc;
    int out_fourcc;

#ifdef RKFEC_HW_V20
    int border_mode;
    int cross_buf_mode;
    rk_ispfec_bg_val bg_val;
    mesh_info_t mesh_info;
#else
    mesh_info_t mesh_xint;
    mesh_info_t mesh_xfra;
    mesh_info_t mesh_yint;
    mesh_info_t mesh_yfra;
#endif

    enum rk_ispfec_update_mesh_mode mesh_upd_mode;
    union {
        char mesh_file_path[128];
        gen_mesh_online_info_t mesh_online;
    } u;
    int in_offset_x;
    int out_offset_x;
    int in_stride;
    int out_stride;
    char calib_ini_from[MAX_FILE_PATH_LEN];
} rk_ispfec_cfg_t;

/**
 * @brief Initialize FEC configuration structure
 * @param cfg Configuration structure to initialize
 *
 * @note This macro will:
 * 1. Set version field to RK_ISP_FEC_API_VERSION
 * 2. Set size field to actual structure size
 *
 * Usage example:
 * @code
 * rk_ispfec_cfg_t cfg;
 * ISP_FEC_CONFIG_INIT(cfg);
 * @endcode
 */
#define ISP_FEC_CONFIG_INIT(cfg)                    \
    do {                                            \
        memset(&(cfg), 0, sizeof(rk_ispfec_cfg_t)); \
        (cfg).version = RK_ISP_FEC_API_VERSION;     \
        (cfg).size    = sizeof(rk_ispfec_cfg_t);    \
    } while (0)

rk_ispfec_ctx_t* rk_ispfec_api_init(rk_ispfec_cfg_t* cfg);

// prepare 时cfg可为 NULL，为NULL时使用 init时的。
// prepare 可用于 reconfig 参数
int rk_ispfec_api_prepare(rk_ispfec_ctx_t* ctx, rk_ispfec_cfg_t* cfg);

int rk_ispfec_api_process(rk_ispfec_ctx_t* ctx, int src_fd, int dst_fd);

void rk_ispfec_api_deinit(rk_ispfec_ctx_t* ctx);

// helper functions
// 用于计算 mesh buf 所需 size
int rk_ispfec_api_calFecMeshsize(int width, int height);

/**
 * @brief Get current FEC processing configuration
 * @param[in] ctx Context handle created by rk_ispfec_api_init
 * @param[out] cfg Pointer to store configuration parameters
 * @return 0 for success, non-zero for error code
 */
int rk_ispfec_api_get_cfg(rk_ispfec_ctx_t* ctx, rk_ispfec_cfg_t* cfg);

/**
 * @brief Update FEC processing configuration
 * @param[in] ctx Context handle created by rk_ispfec_api_init
 * @param[in] cfg Pointer to new configuration parameters
 * @return 0 for success, non-zero for error code
 */
int rk_ispfec_api_set_cfg(rk_ispfec_ctx_t* ctx, rk_ispfec_cfg_t* cfg);

/**
 * @brief Lock FEC processing pipeline for thread-safe parameter updates
 * @param[in] ctx Context handle created by rk_ispfec_api_init
 * @return 0 on success, non-zero error code on failure
 *
 * @details Typical usage scenario:
 * 1. Application calls this function to acquire lock
 * 2. Update ctx->_cfg.mesh_info.vir_addr contents
 * 3. Call rk_ispfec_api_unlock_process() when done
 *
 * @note This will block until any in-progress processing completes.
 * Must be paired with unlock call.
 */
int rk_ispfec_api_lock_process(rk_ispfec_ctx_t* ctx);

/**
 * @brief Unlock FEC processing pipeline
 * @param[in] ctx Context handle created by rk_ispfec_api_init
 * @return 0 on success, non-zero error code on failure
 *
 * @details Releases the lock acquired by rk_ispfec_api_lock_process().
 *
 * @warning Failing to call unlock may lead to deadlocks. Use RAII pattern
 * in C++ or try-finally blocks in other languages for safety.
 */
int rk_ispfec_api_unlock_process(rk_ispfec_ctx_t* ctx);

/**
 * @brief Update FEC mesh correction configuration
 * @param[in] ctx Initialized FEC context handle
 * @param[in] config Pointer to mesh configuration parameters
 * @return 0 on success, -1 for invalid parameters
 *
 * @details Handles three operation modes:
 * - RK_ISPFEC_UPDATE_MESH_ONLINE: Generate mesh using lens coefficients
 * - RK_ISPFEC_UPDATE_MESH_FROM_FILE: Load mesh from filesystem
 * - RK_ISPFEC_UPDATE_MESH_FROM_BUFFER: Load mesh from pre-allocated buffers
 *
 * @note For file mode:
 * - V20 expects single mesh file path
 * - Legacy hardware requires four component files
 * Thread-safe through internal mutex locking
 */
int rk_ispfec_api_set_mesh_config(rk_ispfec_ctx_t* ctx, rk_ispfec_mesh_config_t* config);

/**
 * @brief Retrieve current mesh correction configuration
 * @param[in] ctx Initialized FEC context handle
 * @param[out] config Pointer to store current configuration
 * @return 0 on success, -1 for invalid parameters
 *
 * @details Provides actual configuration used by hardware:
 * - Online mode returns applied correction coefficients
 * - File mode returns full mesh file paths
 * - DMA mode returns active buffer descriptors
 *
 * @warning Returned DMA file descriptors remain valid until next configuration
 *          change or context destruction
 */
int rk_ispfec_api_get_mesh_config(rk_ispfec_ctx_t* ctx, rk_ispfec_mesh_config_t* config);

/**
 * @brief Load lens calibration parameters from INI file
 * @param[in] ini_path Path to INI file containing lens parameters
 * @param[out] info_out Pointer to structure to store loaded parameters
 * @return 0 on success, -1 for file read error or invalid format
 *
 */
int rk_ispfec_api_load_online_config_from_ini(const char* ini_path,
                                              gen_mesh_online_info_t* info_out);

#ifdef __cplusplus
}
#endif

#endif