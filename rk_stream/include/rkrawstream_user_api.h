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

#ifndef RK_AIQ_USER_API2_STREM_CFG_H
#define RK_AIQ_USER_API2_STREM_CFG_H

#include <stdint.h>
#include "rk_aiq_offline_raw.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RKRAWSTREAM_DEF_BUFCNT 5

/* same with defines in rkcif-config.h */
typedef enum {
	RKRAWSTREAM_MEM_COMPACT = 0,
	RKRAWSTREAM_MEM_WORD_LOW_ALIGN = 1,
	RKRAWSTREAM_MEM_WORD_HIGH_ALIGN = 2,
} rkrawstream_memory_type_t;

/* same with defines in rkaiq_common.h */
typedef enum {
    RKRAWSTREAM_HDR_MODE_NORMAL      = 0,
    RKRAWSTREAM_HDR_MODE_ISP_HDR2    = 0x10,
    RKRAWSTREAM_HDR_MODE_ISP_HDR3    = 0x20,
} rkrawstream_hdr_mode_t;

/* same with defines in linux/videodev2.h */
typedef enum {
    RKRAWSTREAM_BUFTYPE_MMAP             = 1,
    RKRAWSTREAM_BUFTYPE_USERPTR          = 2,
    RKRAWSTREAM_BUFTYPE_OVERLAY          = 3,
    RKRAWSTREAM_BUFTYPE_DMABUF           = 4,
} rkrawstream_buf_type_t;

typedef struct {
    char* sns_ent_name;
    char* dev0_name;
    char* dev1_name;
    char* dev2_name;
    void* user_data;
    uint8_t use_offline;
} rkraw_vi_init_params_t;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t pix_fmt;
    int hdr_mode;
    int mem_mode;
    uint8_t buf_memory_type;
    uint8_t buf_cnt;
} rkraw_vi_prepare_params_t;

struct rkraw2_plane
{
    uint32_t mode;      // 0: only addr 1:contain real image
    uint32_t idx;
    uint32_t fd;
    uint32_t size;
    uint64_t addr;
    uint64_t timestamp;
};

typedef struct {
    struct _raw_format _rawfmt;
    struct rkraw2_plane plane[3];
    rk_aiq_frame_info_t _finfo;
} rkrawstream_rkraw2_t;

typedef struct {
    void *user_data;
    rkrawstream_rkraw2_t rkraw2;
} rkrawstream_vicap_cb_param_t;

typedef struct {
    uint32_t seq;
    uint32_t fd;
    uint32_t size;
    void* addr;
    uint64_t timestamp;
    void *user_data;
} rkrawstream_isp_cb_param_t;

typedef struct {
    void *user_data;
    int dev_index;
} rkrawstream_readback_cb_param_t;

typedef struct rkraw_vi_ctx_s rkraw_vi_ctx_t;

typedef void (*rkrawstream_readback_cb_t)(rkrawstream_readback_cb_param_t *param);
typedef int (*rkrawstream_vicap_cb_t)(rkrawstream_vicap_cb_param_t *param);
typedef int (*rkrawstream_isp_cb_t)(rkrawstream_isp_cb_param_t *param);

/*!
 * \brief initialze rkrawstream user api
 * Should call before any other APIs
 *
 * \param[in] sns_ent_name    active sensor media entity name. This represents the unique camera module\n
 *                            in system. And the whole active pipeline could be retrieved by this.
 * \param[in] iq_file_dir     define the search directory of the iq files.
 * \param[in] err_cb          not mandatory. it's used to return system errors to user.
 * \param[in] metas_cb        not mandatory. it's used to return the metas(sensor exposure settings,\n
 *                            isp settings, etc.) for each frame
 * \return return rkrawstream context if success, or NULL if failure.
 */
rkraw_vi_ctx_t *rkrawstream_uapi_init();

/*!
 * \brief deinitialze rkrawstream context
 * Should not be called in started state
 *
 * \param[in] ctx             the context returned by rkrawstream_uapi_init
 */
void rkrawstream_uapi_deinit(rkraw_vi_ctx_t *ctx);

/*!
 * \brief convert rkraw2 buffer to rkrawstream_rkraw2_t
 * \param[in] rkraw2          a buffer contain rkraw2 data
 * \param[out] rawdata        pointer to rkrawstream_rkraw2_t
 *
 * \return return 0 on success, -1 on failed
 */
int rkrawstream_uapi_parase_rkraw2(uint8_t *rawdata, rkrawstream_rkraw2_t *rkraw2);

/*!
 * \brief initialze vicap
 * set up media pipeline, open vicap device, select memory mode, etc.
 *
 * \param[in] ctx            the context returned by rkrawstream_uapi_init
 * \param[in] p              see rkraw_vi_init_params_t for details.
 */
int rkrawstream_vicap_init(rkraw_vi_ctx_t* ctx, rkraw_vi_init_params_t *p);

/*!
 * \brief setup vicap capture format
 * Set up resolution, pixel format hdr mode, etc.
 *
 * \param[in] ctx            the context returned by rkrawstream_uapi_init
 * \param[in] p              see rkraw_vi_prepare_params_t for details.
 */
int rkrawstream_vicap_prepare(rkraw_vi_ctx_t* ctx, rkraw_vi_prepare_params_t *p);

/*!
 * \brief start vicap stream
 *
 * \param[in] ctx            the context returned by rkrawstream_uapi_init
 * \param[in] cb             a user defined callback function, called when each frame comes\n
 *                           user can get, modify raw data frome sensor here\n
 *                           call rkrawstream_readback_set_buffer to send the frame to isp\n
 *                           by default, frame return to driver after this function returned,\n
 *                           see rkrawstream_vicap_buf_take for details.\n
 */
int rkrawstream_vicap_start(rkraw_vi_ctx_t* ctx, rkrawstream_vicap_cb_t cb);

/*!
 * \brief stop vicap stream
 *
 * \param[in] ctx            the context returned by rkrawstream_uapi_init
 */
int rkrawstream_vicap_stop(rkraw_vi_ctx_t* ctx);

/*!
 * \brief shutdown vicap stream, but keep buffers
 *
 * \param[in] ctx            the context returned by rkrawstream_uapi_init
 */
int rkrawstream_vicap_streamoff(rkraw_vi_ctx_t* ctx);

/*!
 * \brief release vicap buffers
 * should be called when use rkrawstream_vicap_streamoff
 *
 * \param[in] ctx            the context returned by rkrawstream_uapi_init
 */
int rkrawstream_vicap_release_buffer(rkraw_vi_ctx_t* ctx);

/*!
 * \brief keep a frame in user callback
 * note this function can only be called during on_frame_capture_callback.
 * if called, must call rkrawstream_vicap_buf_return after the buffer used.
 *
 * \param[in] ctx            the context returned by rkrawstream_uapi_init
 */
void rkrawstream_vicap_buf_take(rkraw_vi_ctx_t* ctx);

/*!
 * \brief return a frame to vicap driver
 *
 * \param[in] ctx            the context returned by rkrawstream_uapi_init
 */
void rkrawstream_vicap_buf_return(rkraw_vi_ctx_t* ctx, int dev_index);

/*!
 * \brief set dmabuf to vicap
 *
 * \param[in] ctx            the context returned by rkrawstream_uapi_init
 * \param[in] dev_index      device index
 * \param[in] buf_index      buffer index
 * \param[in] fd             dma buffer fd
 */
void rkrawstream_vicap_setdmabuf(rkraw_vi_ctx_t* ctx, int dev_index, int buf_index, int fd);

/*!
 * \brief initialze readback
 * set up media pipeline, open readback device, select memory mode, etc.
 *
 * \param[in] ctx            the context returned by rkrawstream_uapi_init
 * \param[in] p              see rkraw_vi_init_params_t for details.
 */
int rkrawstream_readback_init(rkraw_vi_ctx_t* ctx, rkraw_vi_init_params_t *p);

/*!
 * \brief setup readback frame format
 * Set up resolution, pixel format hdr mode, etc.
 *
 * \param[in] ctx            the context returned by rkrawstream_uapi_init
 * \param[in] p              see rkraw_vi_prepare_params_t for details.
 */
int rkrawstream_readback_prepare(rkraw_vi_ctx_t* ctx, rkraw_vi_prepare_params_t *p);

/*!
 * \brief start readback stream
 *
 * \param[in] ctx            the context returned by rkrawstream_uapi_init
 * \param[in] cb             a user defined callback function, called when a frame processed\n
 *                           done by isp.
 */
int rkrawstream_readback_start(rkraw_vi_ctx_t* ctx, rkrawstream_readback_cb_t cb);

/*!
 * \brief stop readback stream
 *
 * \param[in] ctx            the context returned by rkrawstream_uapi_init
 */
int rkrawstream_readback_stop(rkraw_vi_ctx_t* ctx);

/*!
 * \brief send a raw frame to isp.
 *
 * \param[in] ctx            the context returned by rkrawstream_uapi_init.
 * \param[in] rkraw_data     a buffer contain rkraw2 data.
 */
int rkrawstream_readback_set_buffer(rkraw_vi_ctx_t* ctx, uint8_t *rkraw_data);

/*!
 * \brief send a raw frame to isp.
 *
 * \param[in] ctx            the context returned by rkrawstream_uapi_init.
 * \param[in] rkraw2         a pointer to rkraw_vi_init_params_t.
 */
int rkrawstream_readback_set_rkraw2(rkraw_vi_ctx_t* ctx, rkrawstream_rkraw2_t *rkraw2);

void rkrawstream_uapi_fakesns_mode(rkraw_vi_ctx_t *ctx, int isp_index, const char *real_lens);

int rkrawstream_setup_pipline_fmt(rkraw_vi_ctx_t* ctx, int width, int height, uint32_t pixfmt);

int rkrawstream_isp_init(rkraw_vi_ctx_t* ctx, rkraw_vi_init_params_t *p);
int rkrawstream_isp_prepare(rkraw_vi_ctx_t* ctx, rkraw_vi_prepare_params_t *p);
int rkrawstream_isp_start(rkraw_vi_ctx_t* ctx, rkrawstream_isp_cb_t cb);
int rkrawstream_isp_stop(rkraw_vi_ctx_t* ctx);

#ifdef __cplusplus
}
#endif

#endif
