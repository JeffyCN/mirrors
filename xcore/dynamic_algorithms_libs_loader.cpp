/*
 * dynamic_analyzer_loader.cpp - dynamic analyzer loader
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

#include "dynamic_algorithms_libs_loader.h"
#include "handler_interface.h"
#include <dlfcn.h>
#include <dirent.h>


namespace XCam {

#ifdef ANDROID_OS
#if defined(ANDROID_VERSION_ABOVE_8_X)
#define XCAM_DEFAULT_3A_LIB_DIR "/vendor/lib/rkisp/3a"
#define XCAM_DEFAULT_AE_LIB_DIR "/vendor/lib/rkisp/ae"
#define XCAM_DEFAULT_AWB_LIB_DIR "/vendor/lib/rkisp/awb"
#define XCAM_DEFAULT_AF_LIB_DIR "/vendor/lib/rkisp/af"
#else
#define XCAM_DEFAULT_3A_LIB_DIR "/system/lib/rkisp/3a"
#define XCAM_DEFAULT_AE_LIB_DIR "/system/lib/rkisp/ae"
#define XCAM_DEFAULT_AWB_LIB_DIR "/system/lib/rkisp/awb"
#define XCAM_DEFAULT_AF_LIB_DIR "/system/lib/rkisp/af"
#endif
#else
#define XCAM_DEFAULT_3A_LIB_DIR "/usr/lib/rkisp/3a"
#define XCAM_DEFAULT_AE_LIB_DIR "/usr/lib/rkisp/ae"
#define XCAM_DEFAULT_AWB_LIB_DIR "/usr/lib/rkisp/awb"
#define XCAM_DEFAULT_AF_LIB_DIR "/usr/lib/rkisp/af"
#endif

SmartPtr<X3aHandlerManager> X3aHandlerManager::_instance(NULL);
Mutex X3aHandlerManager::_mutex;

SmartPtr<X3aHandlerManager>
X3aHandlerManager::instance()
{
    SmartLock lock(_mutex);
    if (_instance.ptr())
        return _instance;
    _instance = new X3aHandlerManager;
    return _instance;
}

X3aHandlerManager::X3aHandlerManager ():
    _loader_ae(NULL),
    _loader_awb(NULL),
    _loader_af(NULL),
    _ae_handler(NULL),
    _awb_handler(NULL),
    _af_handler(NULL)
{
    XCAM_LOG_DEBUG ("X3aHandlerManager construction");
    find_handler();
}
X3aHandlerManager::~X3aHandlerManager ()
{
    XCAM_LOG_DEBUG ("~X3aHandlerManager destruction");
/*
    if (_ae_handler != NULL) {
        delete _ae_handler;
        _ae_handler = NULL;
    }

    if (_awb_handler != NULL) {
        delete _awb_handler;
        _awb_handler = NULL;
    }

    if (_af_handler != NULL) {
        delete _af_handler;
        _af_handler = NULL;
    }
*/
}

XCamAEDescription*
X3aHandlerManager::get_ae_handler_desc() {
    return _ae_handler;
}

XCamAWBDescription*
X3aHandlerManager::get_awb_handler_desc() {
    return _awb_handler;
}

XCamAFDescription*
X3aHandlerManager::get_af_handler_desc() {
    return _af_handler;
}

void
X3aHandlerManager::find_handler ()
{
    char lib_path[512];
    const char *dir_path = NULL;
    DIR  *dir_3a = NULL;
    struct dirent *dirent_3a = NULL;

    dir_path = getenv ("XCAM_AE_LIB");
    if (!dir_path) {
        dir_path = XCAM_DEFAULT_AE_LIB_DIR;
        XCAM_LOG_INFO ("doesn't find environment=>XCAM_AE_LIB, change to default dir:%s", dir_path);
    }
    dir_3a = opendir (dir_path);
    if (dir_3a) {
        while ((dirent_3a = readdir (dir_3a)) != NULL) {
            if (dirent_3a->d_type != DT_LNK &&
                    dirent_3a->d_type != DT_REG)
                continue;
            snprintf (lib_path, sizeof(lib_path), "%s/%s", dir_path, dirent_3a->d_name);
            _ae_handler = (XCamAEDescription*)load_handler_from_binary (lib_path, XCAM_HANDLER_AE);
        }
    }
    if (dir_3a)
        closedir (dir_3a);

    
    dir_path = getenv ("XCAM_AWB_LIB");
    if (!dir_path) {
        dir_path = XCAM_DEFAULT_AWB_LIB_DIR;
        XCAM_LOG_INFO ("doesn't find environment=>XCAM_AWB_LIB, change to default dir:%s", dir_path);
    }
    dir_3a = opendir (dir_path);
    if (dir_3a) {
        while ((dirent_3a = readdir (dir_3a)) != NULL) {
            if (dirent_3a->d_type != DT_LNK &&
                    dirent_3a->d_type != DT_REG)
                continue;
            snprintf (lib_path, sizeof(lib_path), "%s/%s", dir_path, dirent_3a->d_name);
            _awb_handler = (XCamAWBDescription*)load_handler_from_binary (lib_path, XCAM_HANDLER_AWB);
        }
    }
    if (dir_3a)
        closedir (dir_3a);
    
    dir_path = getenv ("XCAM_AF_LIB");
    if (!dir_path) {
        dir_path = XCAM_DEFAULT_AF_LIB_DIR;
        XCAM_LOG_INFO ("doesn't find environment=>XCAM_AF_LIB, change to default dir:%s", dir_path);
    }
    dir_3a = opendir (dir_path);
    if (dir_3a) {
        while ((dirent_3a = readdir (dir_3a)) != NULL) {
            if (dirent_3a->d_type != DT_LNK &&
                    dirent_3a->d_type != DT_REG)
                continue;
            snprintf (lib_path, sizeof(lib_path), "%s/%s", dir_path, dirent_3a->d_name);
            _af_handler = (XCamAFDescription*)load_handler_from_binary (lib_path, XCAM_HANDLER_AF);
        }
    }
    if (dir_3a)
        closedir (dir_3a);
}

void*
X3aHandlerManager::load_handler_from_binary (const char *path, const int type)
{
    void* desc;
    SmartPtr<Dynamic3aLibsLoader> _loader;
    XCAM_ASSERT (path);

    XCAM_LOG_INFO("load handler(%d) from path(%s)",type, path);

    if (type == XCAM_HANDLER_AE) {
        _loader_ae = new Dynamic3aLibsLoader (path);
        _loader = _loader_ae;
    } else if (type == XCAM_HANDLER_AWB) {
        _loader_awb = new Dynamic3aLibsLoader (path);
        _loader = _loader_awb;
    } else if (type == XCAM_HANDLER_AF) {
        _loader_af= new Dynamic3aLibsLoader (path);
        _loader = _loader_af;
    }

    SmartPtr<AnalyzerLoader> loader = _loader.dynamic_cast_ptr<AnalyzerLoader> ();
    desc = _loader->load_handler (loader, type);

    return desc;
}


// Dynamic3aLibsLoader
Dynamic3aLibsLoader::Dynamic3aLibsLoader (const char *lib_path, const char *symbol)
    : AnalyzerLoader (lib_path, symbol)
{
}

Dynamic3aLibsLoader::~Dynamic3aLibsLoader ()
{
}

void*
Dynamic3aLibsLoader::load_handler (SmartPtr<AnalyzerLoader> &self, const int type)
{
    XCAM_ASSERT (self.ptr () == this);

    void* desc = load_library (get_lib_path (), type);
    XCAM_LOG_INFO ("handler created from 3a lib", type);

    return desc;
}

void *
Dynamic3aLibsLoader::load_symbol (void* handle, const int type)
{
    if (type == XCAM_HANDLER_AE) {
        return (void*)load_ae_handler_symbol(handle);
    } else if (type == XCAM_HANDLER_AWB) {
        return (void*)load_awb_handler_symbol(handle);
    } else if (type == XCAM_HANDLER_AF) {
        return (void*)load_af_handler_symbol(handle);
    }

    return NULL;
}

void *
Dynamic3aLibsLoader::load_ae_handler_symbol (void* handle)
{
    XCamAEDescription *desc = NULL;

    desc = (XCamAEDescription *)AnalyzerLoader::get_symbol (handle, XCAM_AE_LIB_DESCRIPTION);
    if (!desc) {
        XCAM_LOG_ERROR ("get symbol failed from lib");
        return NULL;
    }
    if (desc->type != XCAM_HANDLER_AE) {
        XCAM_LOG_ERROR ("get symbolfailed. type is: %d",
                        desc->type);
        return NULL;
    }
    if (desc->version < xcam_version ()) {
        XCAM_LOG_ERROR ("get symbolfailed. version is:0x%04x, but expect:0x%04x",
                        desc->version, xcam_version ());
        return NULL;
    }
    if (desc->size < sizeof (XCamAEDescription)) {
        XCAM_LOG_ERROR ("get symbol failed, XCamAEDescription size is:%" PRIu32 ", but expect:%" PRIuS,
                        desc->size, sizeof (XCamAEDescription));
        return NULL;
    }

    if (!desc->create_context || !desc->destroy_context ||
            !desc->update_ae_params || !desc->analyze_ae ||
            !desc->combine_analyze_results ||
            !desc->free_results) {
        XCAM_LOG_ERROR ("some functions in symbol not set from lib");
        return NULL;
    }

    XCAM_LOG_INFO ("get symbol from exteranl AE lib");
    return (void*)desc;
}

void *
Dynamic3aLibsLoader::load_awb_handler_symbol (void* handle)
{
    XCamAWBDescription *desc = NULL;

    desc = (XCamAWBDescription *)AnalyzerLoader::get_symbol (handle, XCAM_AWB_LIB_DESCRIPTION);
    if (!desc) {
        XCAM_LOG_ERROR ("get symbol failed from lib");
        return NULL;
    }
    if (desc->type != XCAM_HANDLER_AWB) {
        XCAM_LOG_ERROR ("get symbolfailed. type is: %d",
                        desc->type);
        return NULL;
    }
    if (desc->version < xcam_version ()) {
        XCAM_LOG_ERROR ("get symbolfailed. version is:0x%04x, but expect:0x%04x",
                        desc->version, xcam_version ());
        return NULL;
    }
    if (desc->size < sizeof (XCamAWBDescription)) {
        XCAM_LOG_ERROR ("get symbol failed, XCamAawbWBDescription size is:%" PRIu32 ", but expect:%" PRIuS,
                        desc->size, sizeof (XCamAWBDescription));
        return NULL;
    }

    if (!desc->create_context || !desc->destroy_context ||
            !desc->update_awb_params || !desc->analyze_awb ||
            !desc->combine_analyze_results ||
            !desc->free_results) {
        XCAM_LOG_ERROR ("some functions in symbol not set from lib");
        return NULL;
    }

    XCAM_LOG_INFO ("get symbol from exteranl AWB lib");
    return (void*)desc;
}

void *
Dynamic3aLibsLoader::load_af_handler_symbol (void* handle)
{
    XCamAFDescription *desc = NULL;

    desc = (XCamAFDescription *)AnalyzerLoader::get_symbol (handle, XCAM_AF_LIB_DESCRIPTION);
    if (!desc) {
        XCAM_LOG_DEBUG ("get symbol failed from lib");
        return NULL;
    }
    if (desc->type != XCAM_HANDLER_AF) {
        XCAM_LOG_DEBUG ("get symbolfailed. type is: %d",
                        desc->type);
        return NULL;
    }
    if (desc->version < xcam_version ()) {
        XCAM_LOG_DEBUG ("get symbolfailed. version is:0x%04x, but expect:0x%04x",
                        desc->version, xcam_version ());
        return NULL;
    }
    if (desc->size < sizeof (XCamAFDescription)) {
        XCAM_LOG_DEBUG ("get symbol failed, XCamAAFWBDescription size is:%" PRIu32 ", but expect:%" PRIuS,
                        desc->size, sizeof (XCamAFDescription));
        return NULL;
    }

    if (!desc->create_context || !desc->destroy_context ||
            !desc->update_af_params || !desc->analyze_af ||
            !desc->combine_analyze_results ||
            !desc->free_results) {
        XCAM_LOG_DEBUG ("some functions in symbol not set from lib");
        return NULL;
    }

    XCAM_LOG_INFO ("get symbol from exteranl AF lib");
    return (void*)desc;
}

};
