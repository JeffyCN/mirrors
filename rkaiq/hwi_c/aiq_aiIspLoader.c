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

#include "hwi_c/aiq_aiIspLoader.h"

#include <dlfcn.h>

#include "hwi_c/aiq_camHw.h"
#include "xcore/base/xcam_log.h"

static const char* AiispLibraryName = "libRkAIISP.so";

static bool Init(struct AiispLibrary* obj) {
    obj->handle_ = dlopen(AiispLibraryName, RTLD_LAZY);
    char* error  = dlerror();
    if (obj->handle_ == NULL) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "Failed to dlopen library : %s, error: %s", AiispLibraryName,
                        error);
        return false;
    }

    return true;
}

static void Deinit(struct AiispLibrary* obj) {
    if (obj->handle_ != NULL) {
        dlclose(obj->handle_);
    }
}

static bool LoadSymbols(struct AiispLibrary* obj) {
    char* error;

    obj->ops_.aiisp_init = (rkaiq_aiisp_init)dlsym(obj->handle_, "rk_aiisp_init");
    error                = dlerror();
    if (error != NULL) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "Failed to resolve symbol rk_aiisp_init error: %s", error);
        goto error_out;
    }
    obj->ops_.aiisp_proc = (rkaiq_aiisp_proc)dlsym(obj->handle_, "rk_aiisp_proc");
    error                = dlerror();
    if (error != NULL) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "Failed to resolve symbol rk_aiisp_proc error: %s", error);
        goto error_out;
    }
    obj->ops_.aiisp_deinit = (rkaiq_aiisp_deinit)dlsym(obj->handle_, "rk_aiisp_deinit");
    error                  = dlerror();
    if (error != NULL) {
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "Failed to resolve symbol rk_aiisp_deinit error: %s", error);
        goto error_out;
    }

    return true;
error_out:
    dlclose(obj->handle_);
    obj->handle_ = NULL;
    return false;
}

bool AiqAiIsp_Init(struct AiispLibrary* obj) {
    obj->handle_ = NULL;

    obj->Init        = Init;
    obj->Deinit      = Deinit;
    obj->LoadSymbols = LoadSymbols;

    return true;
}
