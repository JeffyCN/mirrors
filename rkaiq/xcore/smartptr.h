/*
 * xcam_SmartPtr.h - start pointer
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
#ifndef XCAM_SMARTPTR_H
#define XCAM_SMARTPTR_H

#include <base/xcam_defs.h>
#include <stdint.h>

#include <atomic>
#include <cassert>
#include <iostream>
#include <type_traits>

namespace XCam {

#ifndef M_Assert
#ifndef NDEBUG
#   define M_Assert(Expr, Msg) \
    __M_Assert(#Expr, Expr, __FILE__, __LINE__, Msg)
#else
#   define M_Assert(Expr, Msg) ;
#endif
#endif

static void __M_Assert(const char* expr_str, bool expr, const char* file, int line, const char* msg)
{
    if (!expr)
    {
        std::cerr << "Assert failed:\t" << msg << "\n"
            << "Expected:\t" << expr_str << "\n"
            << "Source:\t\t" << file << ", line " << line << "\n";
        abort();
    }
}

class RefCount;

class RefObj {
    friend class RefCount;
public:
    RefObj (): _ref_count(0) {} // derived class must set to SmartPtr at birth
    virtual ~RefObj () {}

    void ref() const {
        ++_ref_count;
    }
    uint32_t unref() const {
        return --_ref_count;
    }
    virtual bool is_a_object () const {
        return true;
    }

private:
    explicit RefObj (uint32_t i) : _ref_count (i) {}
    XCAM_DEAD_COPY (RefObj);

private:
    mutable std::atomic<uint32_t>  _ref_count;
};

class RefCount
    : public RefObj
{
public:
    RefCount () : RefObj (1) {}
    virtual bool is_a_object () const {
        return false;
    }
};

template<typename Obj>
RefObj* generate_ref_count (Obj *obj, std::true_type)
{
    M_Assert(obj != nullptr, "smartptr generate_ref_count failed\n");
    obj->ref ();
    return obj;
}

template<typename Obj>
RefCount* generate_ref_count (Obj *, std::false_type)
{
    return new RefCount;
}

template <typename Obj>
class SmartPtr {
private:
    template<typename ObjDerive> friend class SmartPtr;
public:
    SmartPtr (Obj *obj = NULL)
        : _ptr (obj), _ref(NULL)
    {
        if (obj)
            init_ref (obj);
    }

    template <typename ObjDerive>
    SmartPtr (ObjDerive *obj)
        : _ptr (obj), _ref(NULL)
    {
        if (obj)
            init_ref (obj);
    }

    // copy from pointer
    SmartPtr (const SmartPtr<Obj> &obj)
        : _ptr(obj._ptr), _ref(obj._ref)
    {
        if (_ref) {
            _ref->ref();
            M_Assert(_ptr != nullptr, "smartptr copy from pointer failed\n");
        }
    }

    template <typename ObjDerive>
    SmartPtr (const SmartPtr<ObjDerive> &obj)
        : _ptr(obj._ptr), _ref(obj._ref)
    {
        if (_ref) {
            _ref->ref();
            M_Assert(_ptr != nullptr, "smartptr copy from derived pointer failed\n");
        }
    }

    ~SmartPtr () {
        release();
    }

    /* operator = */
    SmartPtr<Obj> & operator = (Obj *obj) {
        release ();
        set_pointer (obj, NULL);
        return *this;
    }

    template <typename ObjDerive>
    SmartPtr<Obj> & operator = (ObjDerive *obj) {
        release ();
        set_pointer (obj, NULL);
        return *this;
    }

    SmartPtr<Obj> & operator = (const SmartPtr<Obj> &obj) {
        release ();
        set_pointer (obj._ptr, obj._ref);
        return *this;
    }

    template <typename ObjDerive>
    SmartPtr<Obj> & operator = (const SmartPtr<ObjDerive> &obj) {
        release ();
        set_pointer (obj._ptr, obj._ref);
        return *this;
    }

    Obj *operator -> () const {
        return _ptr;
    }

    Obj *ptr() const {
        return _ptr;
    }

    void release() {
        if (!_ptr)
            return;

        M_Assert(_ref != nullptr, "smartptr release ref failed\n");
        if (!_ref->unref()) {
            if (!_ref->is_a_object ()) {
                M_Assert(dynamic_cast<RefCount*>(_ref), "smartptr release dcast obj failed\n");
                delete _ref;
            } else {
                M_Assert(dynamic_cast<Obj*>(_ref) == _ptr, "smartptr release dcast obj failed\n");
            }
            delete _ptr;
        }
        _ptr = NULL;
        _ref = NULL;
    }

    template <typename ObjDerive>
    SmartPtr<ObjDerive> dynamic_cast_ptr () const {
        SmartPtr<ObjDerive> ret(NULL);
        ObjDerive *obj_derive(NULL);
        if (!_ref)
            return ret;
        obj_derive = dynamic_cast<ObjDerive*>(_ptr);
        if (!obj_derive)
            return ret;
        ret.set_pointer (obj_derive, _ref);
        return ret;
    }

    template <typename ObjDerive>
    ObjDerive* get_cast_ptr () const {
        ObjDerive *obj_derive(NULL);
        if (!_ref)
            return NULL;
        obj_derive = dynamic_cast<ObjDerive*>(_ptr);
        if (!obj_derive)
            return NULL;
        return obj_derive;
    }

private:
    template <typename ObjD>
    void set_pointer (ObjD *obj, RefObj *ref) {
        if (!obj)
            return;

        _ptr = obj;
        if (ref) {
            _ref = ref;
            _ref->ref();
        } else {
            init_ref (obj);
        }
    }

    template <typename ObjD>
    void init_ref (ObjD *obj)
    {
        // consider is_base_of or dynamic_cast ?
        typedef std::is_base_of<RefObj, ObjD> BaseCheck;
        _ref = generate_ref_count (obj, BaseCheck());
        M_Assert(_ref != nullptr, "smartptr init_ref failed\n");
    }

private:
    Obj              *_ptr;
    mutable RefObj   *_ref;
};

} // end namespace
#endif //XCAM_SMARTPTR_H
