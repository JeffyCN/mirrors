/*
 * smart_buffer_priv.cpp - smart buffer for XCamVideoBuffer
 *
 *  Copyright (c) 2016-2017 Intel Corporation
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
 * Author: Wind Yuan <feng.yuan@intel.com>
 */

#include "smart_buffer_priv.h"
#include "xcam_log.h"
#include "rk_aiq_types_priv.h"

#if HAS_LIBDRM
#include "xcore/drm_buffer.h"
#endif

namespace XCam {

static void buf_type_to_str(int type, char *str)
{
    switch (type)
    {
        case ISP_POLL_TX:
            strncpy(str, "RAW_BUFFER", SMTBUF_NAME_STR_LEN);
            break;
        case ISP_POLL_SP:
            strncpy(str, "SPIMG_BUFFER", SMTBUF_NAME_STR_LEN);
            break;
        case ISP_GAIN:
            strncpy(str, "ISPGAIN_BUFFER", SMTBUF_NAME_STR_LEN);
            break;
        case ISP_NR_IMG:
            strncpy(str, "NRIMG_BUFFER", SMTBUF_NAME_STR_LEN);
            break;
        case ISPP_GAIN_KG:
            strncpy(str, "GAINKG_BUFFER", SMTBUF_NAME_STR_LEN);
            break;
        case ISPP_GAIN_WR:
            strncpy(str, "GAINWR_BUFFER", SMTBUF_NAME_STR_LEN);
            break;
        default:
            strncpy(str, "UNKNOWN", SMTBUF_NAME_STR_LEN);
            break;
    }
}


SmartBufferPriv::SmartBufferPriv (const SmartPtr<VideoBuffer> &buf)
    : _ref (NULL)
{
    XCAM_ASSERT (buf.ptr ());
    this->_buf_ptr = buf;

    if (!buf.ptr ()) {
        return;
    }

    _ref = new RefCount ();

    const VideoBufferInfo& video_info = buf->get_video_info ();

    this->base.info = *((const XCamVideoBufferInfo*)&video_info);
    this->base.mem_type = XCAM_MEM_TYPE_PRIVATE_BO;
    this->base.timestamp = buf->get_timestamp ();
    this->base.frame_id = buf->get_sequence();

    this->base.ref = SmartBufferPriv::buf_ref;
    this->base.unref = SmartBufferPriv::buf_unref;
    this->base.map = SmartBufferPriv::buf_map;
    this->base.unmap = SmartBufferPriv::buf_unmap;
    this->base.get_fd = SmartBufferPriv::buf_get_fd;
    this->get_bo = SmartBufferPriv::buf_get_bo;
    this->notify = NULL;
    this->pUserContext = NULL;
    memset(this->nameStr, 0, sizeof(this->nameStr));
#ifndef NDEBUG
    buf_type_to_str(buf->_buf_type, this->nameStr);
#endif

}

SmartBufferPriv::SmartBufferPriv (uint32_t frameId, const SmartPtr<VideoBuffer> &buf)
    : _ref (NULL)
{
    XCAM_ASSERT (buf.ptr ());
    this->_buf_ptr = buf;

    if (!buf.ptr ()) {
        return;
    }

    _ref = new RefCount ();

    const VideoBufferInfo& video_info = buf->get_video_info ();

    this->base.info = *((const XCamVideoBufferInfo*)&video_info);
    this->base.mem_type = XCAM_MEM_TYPE_PRIVATE_BO;
    this->base.timestamp = buf->get_timestamp ();
    this->base.frame_id = frameId;

    this->base.ref = SmartBufferPriv::buf_ref;
    this->base.unref = SmartBufferPriv::buf_unref;
    this->base.map = SmartBufferPriv::buf_map;
    this->base.unmap = SmartBufferPriv::buf_unmap;
    this->base.get_fd = SmartBufferPriv::buf_get_fd;
    this->get_bo = SmartBufferPriv::buf_get_bo;
    this->notify = NULL;
    this->pUserContext = NULL;
    memset(this->nameStr, 0, sizeof(this->nameStr));
}

SmartBufferPriv::~SmartBufferPriv ()
{
    XCAM_LOG_DEBUG ("SmartBufferPriv destracted");
    delete _ref;
}

void
SmartBufferPriv::buf_ref (XCamVideoBuffer *data)
{
    SmartBufferPriv *buf = (SmartBufferPriv*) data;
    XCAM_ASSERT (buf->_ref);
#ifndef NDEBUG
    XCAM_LOG_DEBUG ("%s: buf ref called, fd %d", buf->nameStr, buf_get_fd(data));
#endif
    if (buf->_ref)
        buf->_ref->ref ();
}

uint32_t
SmartBufferPriv::buf_unref (XCamVideoBuffer *data)
{
    SmartBufferPriv *buf = (SmartBufferPriv*) data;
    uint32_t count = 0;
    XCAM_ASSERT (buf->_ref);
    if (buf->_ref) {
        count = buf->_ref->unref();
        if (!count) {
            XCamVideoBufferRK* upper_data = (XCamVideoBufferRK*)data;
            if (upper_data->notify)
                upper_data->notify (upper_data->pUserContext, data);
#ifndef NDEBUG
            XCAM_LOG_DEBUG ("%s: buf released, fd %d", buf->nameStr, buf_get_fd(data));
#endif
            delete buf;
        }
    }
    return count;
}

uint8_t *
SmartBufferPriv::buf_map (XCamVideoBuffer *data)
{
    SmartBufferPriv *buf = (SmartBufferPriv*) data;
    XCAM_ASSERT (buf->_buf_ptr.ptr ());
    return buf->_buf_ptr->map ();
}

void
SmartBufferPriv::buf_unmap (XCamVideoBuffer *data)
{
    SmartBufferPriv *buf = (SmartBufferPriv*) data;
    XCAM_ASSERT (buf->_buf_ptr.ptr ());
    buf->_buf_ptr->unmap ();
}

int
SmartBufferPriv::buf_get_fd (XCamVideoBuffer *data)
{
    SmartBufferPriv *buf = (SmartBufferPriv*) data;
    XCAM_ASSERT (buf->_buf_ptr.ptr ());
    return buf->_buf_ptr->get_fd ();
}

void *
SmartBufferPriv::buf_get_bo (XCamVideoBufferRK *data)
{
#if HAS_LIBDRM
    SmartBufferPriv* buf = (SmartBufferPriv*)data;
    XCAM_ASSERT(buf->_buf_ptr.ptr());

    SmartPtr<DrmBufferProxy> bo_buf = buf->_buf_ptr.dynamic_cast_ptr<DrmBufferProxy>();
    XCAM_FAIL_RETURN(ERROR, bo_buf.ptr(), NULL, "get DrmBufferProxy failed");

    return bo_buf->get_bo();
#else

    XCAM_LOG_ERROR("VideoBuffer doesn't support DrmBoBuffer");

    XCAM_UNUSED(data);
#endif
    return NULL;
}

int
SmartBufferPriv::registerNotifyCb(SmartBufferCbNotify_t fpCallback, void *pUserContext)
{
    if(pUserContext == NULL)
    {
        return -1;
    }

    if(fpCallback == NULL)
    {
        return -1;
    }
    this->notify = fpCallback;
    this->pUserContext = pUserContext;
    return 0;
}

XCamVideoBuffer *
convert_to_XCamVideoBuffer (const SmartPtr<VideoBuffer> &buf)
{
    SmartBufferPriv *priv_buf = new SmartBufferPriv (buf);
    XCAM_ASSERT (priv_buf);

    if (priv_buf->is_valid ())
        return (XCamVideoBuffer *)(priv_buf);

    delete priv_buf;
    return NULL;
}

XCamVideoBuffer *
convert_to_external_buffer (const SmartPtr<VideoBuffer> &buf)
{
	SmartBufferPriv *priv_buf = new SmartBufferPriv (buf);
	XCAM_ASSERT (priv_buf);

	if (priv_buf->is_valid ())
		return (XCamVideoBuffer *)(priv_buf);

	delete priv_buf;
	return NULL;
}

}
