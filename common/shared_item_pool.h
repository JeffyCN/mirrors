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

#ifndef RKCAM_SHARED_ITEM_POOL_H
#define RKCAM_SHARED_ITEM_POOL_H

#include "safe_list.h"
#include "buffer_pool.h"

using namespace XCam;

namespace RkCam {

class SharedItemData : public BufferData {
public:
    explicit SharedItemData ():BufferData() {}
    uint8_t *map () { return NULL; }
    bool unmap () { return true; }
    virtual void reset() {
    };

public:
    virtual ~SharedItemData () {}

private:
    XCAM_DEAD_COPY (SharedItemData);
};

class SharedItemBase : public BufferProxy {
public:
    explicit SharedItemBase (const SmartPtr<SharedItemData> &data):BufferProxy(data) {}
    virtual ~SharedItemBase () {
        get_buffer_data ().dynamic_cast_ptr<SharedItemData>()->reset();
    }

    void setType(uint32_t type) { _type = type; }
    void setId(uint32_t id) { _id = id; }

    int getType() { return _type; }
    uint32_t getId() { return _id; }

protected:
    XCAM_DEAD_COPY (SharedItemBase);
    SmartPtr<SharedItemData> get_buffer_data () {
        return BufferProxy::get_buffer_data().dynamic_cast_ptr<SharedItemData>();
    }
    uint32_t _type;
    uint32_t _id;
};

//template<typename T> class SharedItemPool;

template<typename T>
class SharedItemProxy : public SharedItemBase
{
public:
    explicit SharedItemProxy(const SmartPtr<T> &data) : SharedItemBase(data), _data(data) {};
    ~SharedItemProxy() {/* _data->reset();*/ };
    SmartPtr<T> &data() {
#if 0 // dynamic_cast_ptr has performance issue
        return BufferProxy::get_buffer_data ().template dynamic_cast_ptr<T>();
#else
        return _data;
#endif
    }
    uint8_t *map () {
#if 0 // dynamic_cast_ptr has performance issue
        return (uint8_t *)BufferProxy::get_buffer_data ().template dynamic_cast_ptr<T>().ptr();
#else
        return (uint8_t *)_data.ptr();
#endif
    }
private:
    SmartPtr<T>       _data;
    XCAM_DEAD_COPY (SharedItemProxy);
};

template<typename T>
class SharedItemPool
    : public BufferPool
{
    friend class SharedItemProxy<T>;

public:
    explicit SharedItemPool(const char* name, uint32_t max_count = 8);
    virtual ~SharedItemPool();

    SmartPtr<SharedItemProxy<T>> get_item();
    int8_t init(uint32_t max_count = 8);
    bool has_free_items () {
        return has_free_buffers();
    }
private:
    XCAM_DEAD_COPY (SharedItemPool);

protected:
    const char*              _name;
    uint32_t                 _max_count;
    virtual SmartPtr<BufferData> allocate_data (const VideoBufferInfo &buffer_info);
    virtual SmartPtr<BufferProxy> create_buffer_from_data (SmartPtr<BufferData> &data);
};

};

#include "shared_item_pool.cpp"

#endif //RKCAM_SHARED_ITEM_POOL_H


