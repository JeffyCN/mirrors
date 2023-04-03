/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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
 */

#ifndef RKCAM_SHARED_ITEM_POOL_H
#define RKCAM_SHARED_ITEM_POOL_H

#include <iostream>
#include <typeinfo>

#include "safe_list.h"
#include "buffer_pool.h"

using namespace XCam;

namespace RkCam {

class SharedItemBase : public BufferProxy {
public:
    explicit SharedItemBase (const SmartPtr<BufferData> &data):BufferProxy(data) {}
    virtual ~SharedItemBase () = default;

    void setType(uint32_t type) { _type = type; }
    void setId(uint32_t id) { _id = id; }

    int getType() { return _type; }
    uint32_t getId() { return _id; }

protected:
    XCAM_DEAD_COPY (SharedItemBase);

    uint32_t _type = -1;
    uint32_t _id = -1;
};

class RkAiqFullParams;
class RkAiqIspStats;
typedef struct RkAiqSofInfoWrapper_s RkAiqSofInfoWrapper_t;

/**
 * Two SFINAE helpers to check if class has typedef or has member function
 * Reference: https://tinyurl.com/v4f2f5m
 */
/**
 * @class      : HAS_TYPEDEF
 * @brief      : This macro will be used to check if a class has a particular
 * typedef or not.
 * @param typedef_name : Name of Typedef
 * @param name  : Name of struct which is going to be run the test for
 * the given particular typedef specified in typedef_name
 */
#define HAS_TYPEDEF(typedef_name, name)                             \
    template <typename T>                                           \
    struct name {                                                   \
        typedef char yes[1];                                        \
        typedef char no[2];                                         \
        template <typename U>                                       \
        struct type_check;                                          \
        template <typename _1>                                      \
        static yes& chk(type_check<typename _1::typedef_name>*);    \
        template <typename>                                         \
        static no& chk(...);                                        \
        static bool const value = sizeof(chk<T>(0)) == sizeof(yes); \
    }

/**
 * @class      : HAS_MEM_FUNC
 * @brief      : This macro will be used to check if a class has a particular
 * member function implemented in the public section or not.
 * @param func : Name of Member Function
 * @param name : Name of struct which is going to be run the test for
 * the given particular member function name specified in func
 * @param return_type: Return type of the member function
 * @param ellipsis(...) : Since this is macro should provide test case for every
 * possible member function we use variadic macros to cover all possibilities
 */
#define HAS_MEM_FUNC(func, name, return_type, ...)                  \
    template <typename T>                                           \
    struct name {                                                   \
        typedef return_type (T::*Sign)(__VA_ARGS__);                \
        typedef char yes[1];                                        \
        typedef char no[2];                                         \
        template <typename U, U>                                    \
        struct type_check;                                          \
        template <typename _1>                                      \
        static yes& chk(type_check<Sign, &_1::func>*);              \
        template <typename>                                         \
        static no& chk(...);                                        \
        static bool const value = sizeof(chk<T>(0)) == sizeof(yes); \
    }

HAS_MEM_FUNC(reset, has_reset, void, void);

template<typename T>
class SharedItemProxy : public SharedItemBase
{
public:
    explicit SharedItemProxy(const SmartPtr<T> &data) : SharedItemBase(data), _data(data) {};
    virtual ~SharedItemProxy() {
        reset();
        _data.release();
        LOG1_ANALYZER("Release item : %s", typeid(T).name());
    };

    template <typename U = T>
    typename std::enable_if<has_reset<U>::value, bool>::type reset() {
        _data->reset();
		return true;
    }

    template <typename U = T>
    typename std::enable_if<!has_reset<U>::value, bool>::type reset() {
		return false;
    }


    SmartPtr<T> &data() {
        return _data;
    }

    uint8_t *map () {
        return (uint8_t *)_data.ptr();
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

}

#include "shared_item_pool.cpp"

#endif //RKCAM_SHARED_ITEM_POOL_H


