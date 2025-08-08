#ifndef __AIQ_FPN_SW_STREAM_pFpnProc_H__
#define __AIQ_FPN_SW_STREAM_pFpnProc_H__

#include "modules/btnr_algo/FPNCorrection.h"

#include "hwi_c/aiq_rawStreamProcUnit.h"

#include "isp/rk_aiq_isp_fpnSw.h"
#include "algos/rk_aiq_api_types_fpnSw.h"

typedef struct AiqFpnSwStreamProcUnit_s AiqFpnSwStreamProcUnit_t;

typedef struct fpn_proc_msg_s
{
    AiqV4l2Buffer_t* buf_s;
    AiqV4l2Buffer_t* buf_m;
    AiqV4l2Buffer_t* buf_l;
    AiqSensorExpInfo_t* expParam;
} fpn_proc_msg_t;

typedef struct AiqFpnSwStreamProcThread_s
{
    AiqThread_t* _base;
    /* data */
    AiqList_t* mMsgsQueue;
    AiqMutex_t _mutex;
    AiqCond_t _cond;
    bool bQuit;
    AiqFpnSwStreamProcUnit_t *mFpnProc;
} AiqFpnSwStreamProcThread_t;

typedef struct AiqFpnSwStreamProcUnit_s {
    AiqFpnSwStreamProcThread_t* _procTh;
    AiqRawStreamProcUnit_t* _proc_stream;
    void* _memc_manager;
    rk_fpn_correction_param_t _fpn_param;
    int mCamPhyId;
    uint32_t _width;
    uint32_t _height;
    float _csi_again_max;
    float _csi_dgain_max;
    bool _fpn_en;
    int _fpn_offset;
    bool _fpn_init;
    fpnSw_api_attrib_t *_fpn_attr;

    rawStream_send_sync_buf_func _send_sync_buf_func;

    void* _sensor_dev;
} AiqFpnSwStreamProcUnit_t;

XCamReturn AiqFpnSwStreamProcUnit_init(AiqFpnSwStreamProcUnit_t* pFpnProc, void *sns_dev);
XCamReturn AiqFpnSwStreamProcUnit_deinit(AiqFpnSwStreamProcUnit_t* pFpnProc);
XCamReturn AiqFpnSwStreamProcUnit_prepare(AiqFpnSwStreamProcUnit_t* pFpnProc, uint32_t width, uint32_t height);
XCamReturn AiqFpnSwStreamProcUnit_start(AiqFpnSwStreamProcUnit_t* pFpnProc);
XCamReturn AiqFpnSwStreamProcUnit_stop(AiqFpnSwStreamProcUnit_t* pFpnProc);
XCamReturn AiqFpnSwStreamProcUnit_send_sync_buf(void* fpn_ctx, AiqV4l2Buffer_t* buf_s, AiqV4l2Buffer_t* buf_m, AiqV4l2Buffer_t* buf_l);

XCamReturn AiqFpnSwStreamProcUnit_set_fpnSw_attrib(AiqFpnSwStreamProcUnit_t* pFpnProc, fpnSw_api_attrib_t* attr);
XCamReturn AiqFpnSwStreamProcUnit_set_csi_gain_max(AiqFpnSwStreamProcUnit_t* pFpnProc, float csi_again_max, float csi_dgain_max);
XCamReturn AiqFpnSwStreamProcUnit_setCamPhyId(AiqFpnSwStreamProcUnit_t* pFpnProc, int cam_phy_id);
XCamReturn AiqFpnSwStreamProcUnit_setSwStreamInfo(AiqFpnSwStreamProcUnit_t* pFpnProc, void* stream_ctx, rawStream_send_sync_buf_func func);
XCamReturn AiqFpnSwStreamProcUnit_set_proc_stream(AiqFpnSwStreamProcUnit_t* pFpnProc, AiqRawStreamProcUnit_t* proc_stream);
XCamReturn AiqFpnSwStreamProcUnit_set_fpn_offset(AiqFpnSwStreamProcUnit_t* pFpnProc, int fpn_offset);


#endif
