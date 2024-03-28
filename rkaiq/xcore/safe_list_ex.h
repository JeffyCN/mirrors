/*
 * safe_list.h - safe list template
 *
 *  Copyright (c) 2014 Intel Corporation
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

#ifndef XCAM_SAFE_LIST_EX_H
#define XCAM_SAFE_LIST_EX_H

#include <base/xcam_defs.h>
#include <base/xcam_common.h>
#include <errno.h>
#include <list>
#include <xcam_mutex.h>

namespace XCam {

template<class OBj>
class SafeListEx {
public:
    typedef OBj ObjPtr;
    typedef std::list<ObjPtr> ObjList;
    typedef typename std::list<typename SafeListEx<OBj>::ObjPtr>::iterator ObjIter;

    SafeListEx ()
        : _pop_paused (false)
    {}
    ~SafeListEx () {
    }

    /*
     * timeout, -1,  wait until wakeup
     *         >=0,  wait for @timeout microsseconds
    */
    //inline ObjPtr pop (int32_t timeout = -1);
    inline XCamReturn pop (ObjPtr& obj_out, int32_t timeout = -1);
    inline bool push (const ObjPtr &obj);
    inline bool erase (const ObjPtr &obj);
    inline ObjPtr front ();
    uint32_t size () {
        SmartLock lock(_mutex);
        return _obj_list.size();
    }
    bool is_empty () {
        SmartLock lock(_mutex);
        return _obj_list.empty();
    }
    void wakeup () {
        _new_obj_cond.broadcast ();
    }
    void pause_pop () {
        SmartLock lock(_mutex);
        _pop_paused = true;
        wakeup ();
    }
    void resume_pop () {
        SmartLock lock(_mutex);
        _pop_paused = false;
    }
    inline void clear ();

protected:
    ObjList           _obj_list;
    Mutex             _mutex;
    XCam::Cond        _new_obj_cond;
    volatile bool              _pop_paused;
};


template<class OBj>
XCamReturn
SafeListEx<OBj>::pop (ObjPtr& obj_out, int32_t timeout)
{
    SmartLock lock (_mutex);
    int code = 0;

    while (!_pop_paused && _obj_list.empty() && code == 0) {
        if (timeout < 0)
            code = _new_obj_cond.wait(_mutex);
        else
            code = _new_obj_cond.timedwait(_mutex, timeout);
    }

    if (_pop_paused)
        return XCAM_RETURN_ERROR_FAILED;

    if (_obj_list.empty()) {
        if (code == ETIMEDOUT) {
            XCAM_LOG_DEBUG ("safe list pop timeout");
        } else {
            XCAM_LOG_ERROR ("safe list pop failed, code:%d", code);
        }
        return XCAM_RETURN_ERROR_FAILED;
    }

    obj_out = *_obj_list.begin ();
    _obj_list.erase (_obj_list.begin ());
    return XCAM_RETURN_NO_ERROR;
}

template<class OBj>
bool
SafeListEx<OBj>::push (const SafeListEx<OBj>::ObjPtr &obj)
{
    SmartLock lock (_mutex);
    _obj_list.push_back (obj);
    _new_obj_cond.signal ();
    return true;
}

template<class OBj>
bool
SafeListEx<OBj>::erase (const SafeListEx<OBj>::ObjPtr &obj)
{
    LOGE("not suport erase now");
    return false;
}

template<class OBj>
typename SafeListEx<OBj>::ObjPtr
SafeListEx<OBj>::front ()
{
    SmartLock lock (_mutex);
    SafeListEx<OBj>::ObjIter i = _obj_list.begin ();
    if (i == _obj_list.end ())
        return NULL;
    return *i;
}

template<class OBj>
void SafeListEx<OBj>::clear ()
{
    SmartLock lock (_mutex);
    _obj_list.clear();
}

}
#endif //XCAM_SAFE_LIST_EX_H
