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

#include "shared_item_pool.h"

namespace RkCam {

template<typename T>
SharedItemPool<T>::SharedItemPool(const char* name, uint32_t max_count)
    : BufferPool()
    ,_name(name ? name : "default")
    , _max_count(max_count)
{
    LOG1("ENTER SharedItemPool<%s>:%s", _name, __FUNCTION__);
    if (_max_count > 0)
        reserve (_max_count);
    LOG1("EXIT SharedItemPool<%s>:%s", _name, __FUNCTION__);
}

template<typename T>
SharedItemPool<T>::~SharedItemPool()
{
    LOG1("ENTER SharedItemPool<%s>:%s", _name, __FUNCTION__);
    LOG1("EXIT SharedItemPool<%s>:%s", _name, __FUNCTION__);
}

template<typename T>
int8_t SharedItemPool<T>::init(uint32_t max_count)
{
    if (_max_count > 0)
        return -1;
    if (!reserve (max_count))
        return -1;

    _max_count = get_free_buffer_size ();
    
    return 0;
}

template<typename T>
SmartPtr<SharedItemProxy<T>>
SharedItemPool<T>::get_item()
{
#if 0 // dynamic_cast_ptr has performance issue
    SmartPtr<VideoBuffer> buf = get_buffer();
    SmartPtr<SharedItemProxy<T>> proxy =
        buf.dynamic_cast_ptr<SharedItemProxy<T>>() ;
    return proxy;
#else
    // get buf directly, save one call of dynamic_cast_ptr
    SmartPtr<SharedItemProxy<T>> ret_buf;
    SmartPtr<BufferData> data;

    {
        SmartLock lock (_mutex);
        if (!_started)
            return NULL;
    }

    data = _buf_list.pop ();
    if (!data.ptr ()) {
        XCAM_LOG_DEBUG ("BufferPool failed to get buffer");
        return NULL;
    }
    SmartPtr<T> data_t = data.dynamic_cast_ptr<T>();
    ret_buf = new SharedItemProxy<T> (data_t);;
    ret_buf->set_buf_pool (SmartPtr<BufferPool>(this));

    return ret_buf;
#endif
}

template<typename T>
SmartPtr<BufferData> SharedItemPool<T>::allocate_data (const VideoBufferInfo &buffer_info)
{
    return new T();
}

template<typename T>
SmartPtr<BufferProxy> SharedItemPool<T>::create_buffer_from_data (SmartPtr<BufferData> &data)
{
    XCAM_ASSERT (data.ptr ());
    SmartPtr<T> data_t = data.dynamic_cast_ptr<T>();
    return new SharedItemProxy<T> (data_t);
}

};
