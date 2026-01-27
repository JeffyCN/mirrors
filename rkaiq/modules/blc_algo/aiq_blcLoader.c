/*
 *  Copyright (c) 2024 Rockchip Corporation
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

#include "aiq_blcLoader.h"

#include <dlfcn.h>
#include "xcore/base/xcam_log.h"

static const char* blcLibraryName = "libautoblc.so";

static bool Init(struct blcLibrary* obj) {
#if 0
    obj->handle_ = dlopen(blcLibraryName, RTLD_LAZY);
    char* error  = dlerror();
    if (obj->handle_ == NULL) {
        LOGE_ABLC("Failed to dlopen library : %s, error: %s", blcLibraryName,
                        error);
        return false;
    }
#endif
    return true;
}

static void Deinit(struct blcLibrary* obj) {
#if 0
    if (obj->handle_ != NULL) {
        dlclose(obj->handle_);
    }
#endif
}

static bool LoadSymbols(struct blcLibrary* obj) {
#if 0
    char* error;
    obj->ops_.blc_init = (rkaiq_autoblc_init)dlsym(obj->handle_, "rk_autoblc_init");
    error = dlerror();
    if (error != NULL) {
        LOGE_ABLC("Failed to resolve symbol rk_autoblc_init error: %s", error);
        goto error_out;
    }
    obj->ops_.blc_proc = (rkaiq_autoblc_proc)dlsym(obj->handle_, "rk_autoblc_proc");
    error                = dlerror();
    if (error != NULL) {
        LOGE_ABLC("Failed to resolve symbol rk_autoblc_proc error: %s", error);
        goto error_out;
    }
    obj->ops_.blc_deinit = (rkaiq_autoblc_deinit)dlsym(obj->handle_, "rk_autoblc_deinit");
    error                  = dlerror();
    if (error != NULL) {
        LOGE_ABLC("Failed to resolve symbol rk_autoblc_deinit error: %s", error);
        goto error_out;
    }

    return true;
error_out:
    dlclose(obj->handle_);
    obj->handle_ = NULL;
    return false;
#endif
    obj->ops_.blc_init = rk_autoblc_init;
    obj->ops_.blc_proc = rk_autoblc_proc;
    obj->ops_.blc_deinit = rk_autoblc_deinit;
    return true;
}

bool AiqBlc_Init(struct blcLibrary* obj) {
    obj->handle_ = NULL;

    obj->Init        = Init;
    obj->Deinit      = Deinit;
    obj->LoadSymbols = LoadSymbols;

    return true;
}
