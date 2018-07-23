/*
 * dynamic_analyzer_loader.h - dynamic analyzer loader
 *
 *  Copyright (c) 2015 Intel Corporation
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
 *         Zong Wei  <wei.zong@intel.com>
 */

#ifndef XCAM_DYNAMIC_3ALIBS_LOADER_H
#define XCAM_DYNAMIC_3ALIBS_LOADER_H

#include <xcam_std.h>
#include <x3a_analyzer.h>
#include <base/xcam_3a_description.h>
#include <analyzer_loader.h>
#include <sys/types.h>

namespace XCam {
class Dynamic3aLibsLoader
    : public AnalyzerLoader
{
public:
    Dynamic3aLibsLoader (const char *lib_path, const char *symbol = XCAM_3A_LIB_DESCRIPTION);
    virtual ~Dynamic3aLibsLoader ();

    virtual void* load_handler (SmartPtr<AnalyzerLoader> &self, const int type);

protected:
    virtual void* load_symbol (void* handle, const int type = 0);
    void* load_ae_handler_symbol (void* handle);
    void* load_awb_handler_symbol (void* handle);
    void* load_af_handler_symbol (void* handle);

private:
    XCAM_DEAD_COPY(Dynamic3aLibsLoader);
};

class X3aHandlerManager
{
/*
protected:
    explicit X3aHandlerManager ();
*/
public:
    explicit X3aHandlerManager ();
    virtual ~X3aHandlerManager ();

    static SmartPtr<X3aHandlerManager> instance();

    XCamAEDescription* get_ae_handler_desc();

    XCamAWBDescription* get_awb_handler_desc();

    XCamAFDescription* get_af_handler_desc();

private:
    void find_handler ();
    void* load_handler_from_binary (const char *path, const int type);

private:
    XCAM_DEAD_COPY (X3aHandlerManager);

private:
    static SmartPtr<X3aHandlerManager> _instance;
    static Mutex                        _mutex;

    SmartPtr<Dynamic3aLibsLoader>     _loader_ae;
    SmartPtr<Dynamic3aLibsLoader>     _loader_awb;
    SmartPtr<Dynamic3aLibsLoader>     _loader_af;

    XCamAEDescription* _ae_handler;
    XCamAWBDescription* _awb_handler;
    XCamAFDescription* _af_handler;
};
};

#endif // XCAM_DYNAMIC_ANALYZER_LOADER_H
