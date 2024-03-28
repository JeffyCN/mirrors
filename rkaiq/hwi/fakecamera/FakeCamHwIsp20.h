
/*
 *  Copyright (c) 2019 Rockchip Corporation
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

#ifndef _FAKE_CAM_HW_ISP20_H_
#define _FAKE_CAM_HW_ISP20_H_
#include "CamHwIsp20.h"
#include "isp21/CamHwIsp21.h"
#include "isp3x/CamHwIsp3x.h"
#include "isp32/CamHwIsp32.h"
#include "isp39/CamHwIsp39.h"

namespace RkCam {

class FakeCamHwIsp20 : virtual public CamHwIsp20 {
public:
    explicit FakeCamHwIsp20();
    virtual ~FakeCamHwIsp20();
    virtual XCamReturn init(const char* sns_ent_name);
    virtual XCamReturn prepare(uint32_t width, uint32_t height, int mode, int t_delay, int g_delay);
    virtual XCamReturn enqueueRawBuffer(void *rawdata, bool sync);
    virtual XCamReturn enqueueRawFile(const char *path);
    virtual XCamReturn registRawdataCb(void (*callback)(void *));
    virtual XCamReturn rawdataPrepare(rk_aiq_raw_prop_t prop);
    virtual XCamReturn poll_event_ready (uint32_t sequence, int type);
    virtual XCamReturn poll_buffer_ready (SmartPtr<V4l2BufferProxy> &buf, int dev_index);
protected:
    using CamHwIsp20::poll_buffer_ready;
    virtual XCamReturn init_mipi_devices(rk_sensor_full_info_t *s_info);
    void parse_rk_rawdata(void *rawdata, struct rk_aiq_vbuf *vbuf);
    XCamReturn parse_rk_rawfile(FILE *fp, struct rk_aiq_vbuf *vbuf);
    XCamReturn setupOffLineLink(int isp_index, bool enable);
    struct _st_addrinfo _st_addr[3];
    struct _raw_format _rawfmt;
    rk_aiq_frame_info_t _finfo;
    enum v4l2_memory    _rx_memory_type;
    enum v4l2_memory    _tx_memory_type;
    rk_aiq_rawbuf_type_t _rawbuf_type;
    SmartPtr<V4l2Device> _mipi_tx_devs[3];
    SmartPtr<V4l2Device> _mipi_rx_devs[3];
    int isp_index;
};

class FakeCamHwIsp21 : virtual public CamHwIsp21, virtual public FakeCamHwIsp20 {
public:
    explicit FakeCamHwIsp21();
    virtual ~FakeCamHwIsp21();
    virtual XCamReturn init(const char* sns_ent_name) override;
    virtual XCamReturn prepare(uint32_t width, uint32_t height, int mode, int t_delay, int g_delay) override;
    virtual XCamReturn enqueueRawBuffer(void *rawdata, bool sync) override;
    virtual XCamReturn enqueueRawFile(const char *path) override;
    virtual XCamReturn registRawdataCb(void (*callback)(void *)) override;
    virtual XCamReturn rawdataPrepare(rk_aiq_raw_prop_t prop) override;
    virtual XCamReturn poll_event_ready (uint32_t sequence, int type) override;
    virtual XCamReturn poll_buffer_ready (SmartPtr<V4l2BufferProxy> &buf, int dev_index) override {
            return FakeCamHwIsp20::poll_buffer_ready(buf, dev_index);
    }
private:
    using CamHwIsp20::poll_buffer_ready;
};

class FakeCamHwIsp3x : virtual public CamHwIsp3x, virtual public FakeCamHwIsp20 {
public:
    explicit FakeCamHwIsp3x();
    virtual ~FakeCamHwIsp3x();
    virtual XCamReturn init(const char* sns_ent_name) override;
    virtual XCamReturn prepare(uint32_t width, uint32_t height, int mode, int t_delay, int g_delay) override;
    virtual XCamReturn enqueueRawBuffer(void *rawdata, bool sync) override;
    virtual XCamReturn enqueueRawFile(const char *path) override;
    virtual XCamReturn registRawdataCb(void (*callback)(void *)) override;
    virtual XCamReturn rawdataPrepare(rk_aiq_raw_prop_t prop) override;
    virtual XCamReturn poll_event_ready (uint32_t sequence, int type) override;
    virtual XCamReturn poll_buffer_ready (SmartPtr<V4l2BufferProxy> &buf, int dev_index) override {
            return FakeCamHwIsp20::poll_buffer_ready(buf, dev_index);
    }
private:
    using CamHwIsp20::poll_buffer_ready;
};

class FakeCamHwIsp32 : virtual public CamHwIsp32, virtual public FakeCamHwIsp20 {
public:
    explicit FakeCamHwIsp32();
    virtual ~FakeCamHwIsp32();
    virtual XCamReturn init(const char* sns_ent_name) override;
    virtual XCamReturn prepare(uint32_t width, uint32_t height, int mode, int t_delay, int g_delay) override;
    virtual XCamReturn enqueueRawBuffer(void *rawdata, bool sync) override;
    virtual XCamReturn enqueueRawFile(const char *path) override;
    virtual XCamReturn registRawdataCb(void (*callback)(void *)) override;
    virtual XCamReturn rawdataPrepare(rk_aiq_raw_prop_t prop) override;
    virtual XCamReturn poll_event_ready (uint32_t sequence, int type) override;
    virtual XCamReturn poll_buffer_ready (SmartPtr<V4l2BufferProxy> &buf, int dev_index) override {
            return FakeCamHwIsp20::poll_buffer_ready(buf, dev_index);
    }
private:
    using CamHwIsp20::poll_buffer_ready;
};

class FakeCamHwIsp39 : virtual public CamHwIsp39, virtual public FakeCamHwIsp20 {
public:
    explicit FakeCamHwIsp39();
    virtual ~FakeCamHwIsp39();
    virtual XCamReturn init(const char* sns_ent_name) override;
    virtual XCamReturn prepare(uint32_t width, uint32_t height, int mode, int t_delay, int g_delay) override;
    virtual XCamReturn enqueueRawBuffer(void *rawdata, bool sync) override;
    virtual XCamReturn enqueueRawFile(const char *path) override;
    virtual XCamReturn registRawdataCb(void (*callback)(void *)) override;
    virtual XCamReturn rawdataPrepare(rk_aiq_raw_prop_t prop) override;
    virtual XCamReturn poll_event_ready (uint32_t sequence, int type) override;
    virtual XCamReturn poll_buffer_ready (SmartPtr<V4l2BufferProxy> &buf, int dev_index) override {
            return FakeCamHwIsp20::poll_buffer_ready(buf, dev_index);
    }
private:
    using CamHwIsp20::poll_buffer_ready;
};

}
#endif
