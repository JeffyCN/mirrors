
#ifndef __AIQ_BTNR_REPROC_H__
#define __AIQ_BTNR_REPROC_H__

#include "hwi_c/aiq_CamHwBase.h"

typedef struct aiq_btnrReprocMsg_s
{
    uint32_t frame_id;
    void *buf_addr;
    int buf_fd;
} aiq_btnrReprocMsg_t;

typedef struct AiqBtnrDsReproc_s AiqBtnrDsReproc_t;

typedef struct AiqBtnrDsReprocThread_s
{
    AiqThread_t* _base;
    /* data */
    AiqList_t* mMsgsQueue;
    AiqMutex_t _mutex;
    AiqCond_t _cond;
    bool bQuit;
    AiqBtnrDsReproc_t *mBtnrDsProc;
} AiqBtnrDsReprocThread_t;

struct AiqBtnrDsReproc_s {
    AiqBtnrDsReprocThread_t* _procTh;
    void* _isp_subdev;
    uint32_t _width;
    uint32_t _height;
    uint32_t _ds_width;
    uint32_t _ds_width_vir;
    uint32_t _ds_height;
    void *_btnr_attr;
    void *_btnr_wgt_addr_array[RKISP_BUFFER_MAX];
    int _btnr_wgt_fd_array[RKISP_BUFFER_MAX];
    uint32_t _btnr_wgt_size;
    void* _filter_buffer;
    int filter_mode;
    bool _doBtnrWgtReproc_en;
    uint32_t _last_proc_frame_id;
    bool _btnr2_en;
};

XCamReturn AiqBtnrDsReproc_init(AiqBtnrDsReproc_t* pBtnrDsProc, void *isp_subdev);
XCamReturn AiqBtnrDsReproc_deinit(AiqBtnrDsReproc_t* pBtnrDsProc);
XCamReturn AiqBtnrDsReproc_prepare(AiqBtnrDsReproc_t* pBtnrDsProc, uint32_t width, uint32_t height);
XCamReturn AiqBtnrDsReproc_start(AiqBtnrDsReproc_t* pBtnrDsProc);
XCamReturn AiqBtnrDsReproc_stop(AiqBtnrDsReproc_t* pBtnrDsProc);
XCamReturn AiqBtnrDsReproc_pushMsg(AiqBtnrDsReproc_t* pBtnrDsProc, void *stats);
XCamReturn AiqBtnrDsReproc_setCalib(AiqBtnrDsReproc_t* pBtnrDsProc, const CamCalibDbV2Context_t *calib);
XCamReturn AiqBtnrDsReproc_setBtnrWgtBuf(AiqBtnrDsReproc_t* pBtnrDsProc, void *buf_addr, uint32_t buf_size, int *buf_fd);
XCamReturn AiqBtnrDsReproc_updateEn(AiqBtnrDsReproc_t* pBtnrDsProc, bool en);

#endif