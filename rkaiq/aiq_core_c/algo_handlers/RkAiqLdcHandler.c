/*
 * Copyright (c) 2024 Rockchip Eletronics Co., Ltd.
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

#include "RkAiqLdcHandler.h"

#include <inttypes.h>
#include <sys/stat.h>

#include "RkAiqGlobalParamsManager_c.h"
#include "aiq_base.h"
#include "aiq_core_c/aiq_core.h"
#include "algos/newStruct/ldc/include/ldc_algo_api.h"
#include "newStruct/algo_common.h"

static void _handlerLdc_init(AiqAlgoHandler_t* pHdl) {
    ENTER_ANALYZER_FUNCTION();

    AiqAlgoHandler_deinit(pHdl);
    pHdl->mConfig       = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoConfigLdc)));
    pHdl->mProcInParam  = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcLdc)));
    pHdl->mProcOutParam = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoResCom)));

    pHdl->mResultType = RESULT_TYPE_LDC_PARAM;
    pHdl->mResultSize = sizeof(ldc_param_t);

    EXIT_ANALYZER_FUNCTION();
}

static XCamReturn _handlerLdc_prepare(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_prepare(pAlgoHandler);
    RKAIQCORE_CHECK_RET(ret, "cdc handle prepare failed");

    AiqAlgoHandlerLdc_t* pLdcHdl = (AiqAlgoHandlerLdc_t*)pAlgoHandler;
    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom   = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoConfigLdc* ldc_config_int = (RkAiqAlgoConfigLdc*)pAlgoHandler->mConfig;

    if (sharedCom->resourcePath) {
        strcpy(ldc_config_int->iqpath, sharedCom->resourcePath);
    } else {
        strcpy(ldc_config_int->iqpath, "/etc/iqfiles");
    }

    ldc_config_int->mem_ops      = pAlgoHandler->mAiqCore->mShareMemOps;
    ldc_config_int->is_multi_isp = sharedCom->is_multi_isp_mode;

    aiqMutex_lock(&pAlgoHandler->mCfgMutex);
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret                       = des->prepare(pAlgoHandler->mConfig);
    aiqMutex_unlock(&pAlgoHandler->mCfgMutex);
    RKAIQCORE_CHECK_RET(ret, "cdc algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

#define HAVE_HEADINFO
static XCamReturn _handlerLdc_readMeshFile(const char* file_name, const void* addr) {
    FILE* ofp;
    ofp = fopen(file_name, "rb");
    if (ofp != NULL) {
        struct stat file_stat;
        if (fstat(fileno(ofp), &file_stat) == -1) {
            LOGE_ALDC("Failed to get size from file %s", file_name);
            fclose(ofp);
            return XCAM_RETURN_ERROR_FAILED;
        }

        LOGI_ALDC("%s size %" PRId64 "", file_name, file_stat.st_size);

        uint32_t lut_size = 0;
#ifdef HAVE_HEADINFO
        uint16_t hpic = 0, vpic = 0, hsize = 0, vsize = 0, hstep = 0, vstep = 0;

        fread(&hpic, sizeof(uint16_t), 1, ofp);
        fread(&vpic, sizeof(uint16_t), 1, ofp);
        fread(&hsize, sizeof(uint16_t), 1, ofp);
        fread(&vsize, sizeof(uint16_t), 1, ofp);
        fread(&hstep, sizeof(uint16_t), 1, ofp);
        fread(&vstep, sizeof(uint16_t), 1, ofp);

        LOGI_ALDC("head info: hpic %d, vpic %d, hsize %d, vsize %d, hstep %d, vstep %d", hpic, vpic,
                  hsize, vsize, hstep, vstep);

        lut_size = hsize * vsize * sizeof(uint16_t);
#else
        lut_size = file_stat.st_size;
#endif
        uint16_t* buf = (uint16_t*)addr;
        size_t num    = fread((void*)addr, 1, lut_size, ofp);

        fclose(ofp);

        LOGD_ALDC("[0:15]: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", buf[0], buf[1], buf[2],
                  buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10], buf[11], buf[12],
                  buf[13], buf[14], buf[15]);

#ifdef HAVE_HEADINFO
        if (num != lut_size) {
            LOGE_ALDCH("mismatched the size of mesh file %s, expected %u, read %u", file_name,
                       lut_size, num);
            return XCAM_RETURN_ERROR_FAILED;
        }
#endif

        LOGI_ALDC("the mesh size is %d in %s", lut_size, file_name);
    } else {
        LOGW_ALDC("lut file %s not exist", file_name);
        return XCAM_RETURN_ERROR_FAILED;
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handlerLdc_updMeshFromFile(AiqAlgoHandler_t* pAlgoHandler,
                                              ldc_param_auto_t* aut_param, ldc_param_t* man_param) {
    char file_name[800] = "\0";

    RkAiqAlgoConfigLdc* config = (RkAiqAlgoConfigLdc*)pAlgoHandler->mConfig;
    char* path                 = aut_param->extMeshFile.sw_ldcT_extMeshFile_path;
    char* name                 = aut_param->extMeshFile.sw_ldcT_ldchExtMeshFile_name;
    if (!strlen(path)) path = config->iqpath;
    if (!strlen(name)) name = DEFAULT_LDCH_MESH;

    sprintf(file_name, "%s/%s", path, name);
    LOGK_ALDC("try reading mesh from %s", file_name);
    if (access(file_name, F_OK) == 0) {
        LdcLutBuffer* lut_buf = algo_ldc_getLdchFreeLutBuf(pAlgoHandler->mAlgoCtx);
        if (lut_buf && lut_buf->Addr && lut_buf->Fd > 0) {
            if (_handlerLdc_readMeshFile(file_name, lut_buf->Addr) < 0) {
                LOGE_ALDC("Failed to read mesh from %s for LDCH", file_name);
                return XCAM_RETURN_ERROR_FAILED;
            } else {
                man_param->sta.ldchCfg.en                                = true;
                man_param->sta.ldchCfg.lutMapCfg.sw_ldcT_lutMapBuf_fd[0] = lut_buf->Fd;

                lut_buf->Fd = LDC_BUF_FD_DEFAULT;
            }
        }
    } else {
        LOGE_ALDC("%s does not exist", file_name);
        return XCAM_RETURN_ERROR_FAILED;
    }

#if RKAIQ_HAVE_LDCV
    if (aut_param->enMode == LDC_LDCH_LDCV_EN) {
        name = aut_param->extMeshFile.sw_ldcT_ldcvExtMeshFile_name;
        if (!strlen(name)) name = DEFAULT_LDCV_MESH;

        sprintf(file_name, "%s/%s", path, name);
        LOGK_ALDC("try reading mesh from %s", file_name);
        if (access(file_name, F_OK) == 0) {
            LdcLutBuffer* lut_buf = algo_ldc_getLdcvFreeLutBuf(pAlgoHandler->mAlgoCtx);
            if (lut_buf->Addr && lut_buf->Fd > 0) {
                if (_handlerLdc_readMeshFile(file_name, lut_buf->Addr) < 0) {
                    LOGE_ALDC("Failed to read mesh from %s for LDCV", file_name);
                    return XCAM_RETURN_ERROR_FAILED;
                } else {
                    man_param->sta.ldcvCfg.en                                = true;
                    man_param->sta.ldcvCfg.lutMapCfg.sw_ldcT_lutMapBuf_fd[0] = lut_buf->Fd;

                    lut_buf->Fd = LDC_BUF_FD_DEFAULT;
                }
            }
        } else {
            LOGE_ALDC("%s does not exist", file_name);
            return XCAM_RETURN_ERROR_FAILED;
        }
    }
#endif

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handlerLdc_updMeshFromUapiBuf(AiqAlgoHandler_t* pAlgoHandler,
                                                 ldc_param_t* ldc_param) {
    if (ldc_param->sta.ldchCfg.en) {
        uint32_t size = ldc_param->sta.ldchCfg.lutMapCfg.sw_ldcT_lutMap_size;
        uint16_t* vaddr = (uint16_t*)ldc_param->sta.ldchCfg.lutMapCfg.sw_ldcT_lutMapBuf_vaddr[0];
        if (size > 0 && vaddr) {
            LdcLutBuffer* lut_buf = algo_ldc_getLdchFreeLutBuf(pAlgoHandler->mAlgoCtx);
            if (lut_buf && lut_buf->Addr && lut_buf->Fd > 0) {
                uint16_t hpic, vpic, hsize, vsize, hstep, vstep;
                uint32_t lut_size = 0;

                hpic  = *vaddr++;
                vpic  = *vaddr++;
                hsize = *vaddr++;
                vsize = *vaddr++;
                hstep = *vaddr++;
                vstep = *vaddr++;

                lut_size = hsize * vsize * sizeof(uint16_t);
                LOGD_ALDC("lut info: [%d-%d-%d-%d-%d-%d]", hpic, vpic, hsize, vsize, hstep, vstep);
                LOGD_ALDC("calculate lut size: %u, sw_ldcT_lutMap_size %u", lut_size, size);

                memcpy(lut_buf->Addr, vaddr, size);

                ldc_param->sta.ldchCfg.lutMapCfg.sw_ldcT_lutMapBuf_fd[0] = lut_buf->Fd;
                LOGD_ALDC("copy lut buf(%p, size %d) to ldch mesh buf(%p, fd %d)", vaddr, size,
                          lut_buf->Addr, lut_buf->Fd);

                lut_buf->Fd = LDC_BUF_FD_DEFAULT;

                LOGD_ALDC("LDCH lut[0:15]: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                          vaddr[0], vaddr[1], vaddr[2], vaddr[3], vaddr[4], vaddr[5], vaddr[6],
                          vaddr[7], vaddr[8], vaddr[9], vaddr[10], vaddr[11], vaddr[12], vaddr[13],
                          vaddr[14], vaddr[15]);

                return XCAM_RETURN_NO_ERROR;
            } else {
                LOGE_ALDC("Failed to get free lut buf for LDCH, don't update cfg!");
                return XCAM_RETURN_ERROR_FAILED;
            }
        } else {
            LOGE_ALDC("LDCH map addr %p, size %d is error, don't update cfg!", vaddr, size);
            return XCAM_RETURN_ERROR_FAILED;
        }
    }

#if RKAIQ_HAVE_LDCV
    if (ldc_param->sta.ldcvCfg.en) {
        uint32_t size = ldc_param->sta.ldcvCfg.lutMapCfg.sw_ldcT_lutMap_size;
        uint16_t* vaddr = ldc_param->sta.ldcvCfg.lutMapCfg.sw_ldcT_lutMapBuf_vaddr[0];
        if (size > 0 && vaddr) {
            const LdcLutBuffer* lut_buf = algo_ldc_getLdcvFreeLutBuf(pAlgoHandler->mAlgoCtx);
            if (lut_buf && lut_buf->Addr && lut_buf->Fd > 0) {
                uint16_t hpic, vpic, hsize, vsize, hstep, vstep;
                uint32_t lut_size = 0;

                hpic  = *vaddr++;
                vpic  = *vaddr++;
                hsize = *vaddr++;
                vsize = *vaddr++;
                hstep = *vaddr++;
                vstep = *vaddr++;

                lut_size = hsize * vsize * sizeof(uint16_t);
                LOGD_ALDC("lut info: [%d-%d-%d-%d-%d-%d]", hpic, vpic, hsize, vsize, hstep, vstep);
                LOGD_ALDC("calculate lut size: %u, sw_ldcT_lutMap_size %u", lut_size, size);

                memcpy(lut_buf->Addr, vaddr, size);

                ldc_param->sta.ldcvCfg.lutMapCfg.sw_ldcT_lutMapBuf_fd[0] = lut_buf->Fd;
                LOGD_ALDC("copy lut buf(%p, size %d) to ldcv mesh buf(%p, fd %d)", vaddr, size,
                          lut_buf->Addr, lut_buf->Fd);

                LOGD_ALDC("LDCV lut[0:15]: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                          vaddr[0], vaddr[1], vaddr[2], vaddr[3], vaddr[4], vaddr[5], vaddr[6],
                          vaddr[7], vaddr[8], vaddr[9], vaddr[10], vaddr[11], vaddr[12], vaddr[13],
                          vaddr[14], vaddr[15]);

                return XCAM_RETURN_NO_ERROR;
            } else {
                LOGE_ALDC("Failed to get free lut buf for LDCV, don't update cfg!");
                return XCAM_RETURN_ERROR_FAILED;
            }
        } else {
            LOGE_ALDC("LDCV map addr %p, size %d is error, don't update cfg!", vaddr, size);
            return XCAM_RETURN_ERROR_FAILED;
        }
    }
#endif

    return XCAM_RETURN_ERROR_FAILED;
}

XCamReturn _handlerLdc_do_processing_common(AiqAlgoHandler_t* pAlgoHandler) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcLdc* ldc_proc_int   = (RkAiqAlgoProcLdc*)pAlgoHandler->mProcInParam;

    int restype            = pAlgoHandler->mResultType;
    const char* ResTypeStr = Cam3aResultType2Str[restype];

    aiq_params_base_t* pBase = shared->fullParams->pParamsArray[restype];
    if (!pBase) {
        LOGW("%s: no params buf !", ResTypeStr);
        return XCAM_RETURN_BYPASS;
    }

    RkAiqAlgoResCom* proc_res = pAlgoHandler->mProcOutParam;
    proc_res->algoRes         = (void*)pBase->_data;

    GlobalParamsManager_t* globalParamsManager = pAlgoHandler->mAiqCore->mGlobalParamsManger;
    GlobalParamsManager_lockAlgoParam(globalParamsManager, pAlgoHandler->mResultType);
    if (globalParamsManager && !GlobalParamsManager_isFullManualMode(globalParamsManager) &&
        GlobalParamsManager_isManualLocked(globalParamsManager, pAlgoHandler->mResultType)) {
        rk_aiq_global_params_wrap_t wrap_param;

        ldc_param_auto_t aut_param;
        wrap_param.type           = pAlgoHandler->mResultType;
        wrap_param.man_param_size = pAlgoHandler->mResultSize;
        wrap_param.man_param_ptr  = proc_res->algoRes;
        wrap_param.aut_param_ptr  = &aut_param;
        wrap_param.aut_param_size = sizeof(aut_param);
        XCamReturn ret1 =
            GlobalParamsManager_getAndClearPendingLocked(globalParamsManager, &wrap_param);
        if (ret1 == XCAM_RETURN_NO_ERROR) {
            LOGD_ALDC("%s: %p, man en:%d, bypass:%d", ResTypeStr, &proc_res->en, wrap_param.en,
                      wrap_param.bypass);
            proc_res->en          = wrap_param.en;
            proc_res->bypass      = wrap_param.bypass;
            proc_res->cfg_update  = true;
            pAlgoHandler->mOpMode = RK_AIQ_OP_MODE_MANUAL;

            ldc_param_t* man_param = (ldc_param_t*)(wrap_param.man_param_ptr);
            LOGD_ALDC("%s, man sta ldch en %d, upd_mesh_mode %d", ResTypeStr,
                      man_param->sta.ldchCfg.en, ldc_proc_int->upd_mesh_mode);

#if RKAIQ_HAVE_LDCV
            LOGD_ALDC("ldcv en %d", man_param->sta.ldchCfg.en);
#endif

            // 1. Check whether to enable the LDC in manual mode.
            if (man_param->sta.ldchCfg.en &&
                ldc_proc_int->upd_mesh_mode != kAiqLdcUpdMeshFromFile) {
                // 1.1 Check whether the uapi updates mesh with vaddr.
                XCamReturn ret_b = _handlerLdc_updMeshFromUapiBuf(pAlgoHandler, man_param);
                if (ret_b < 0) {
                    if (sharedCom->init) {
                        man_param->sta.ldchCfg.en = false;
#if RKAIQ_HAVE_LDCV
                        man_param->sta.ldcvCfg.en = false;
#endif
                    } else {
                        proc_res->cfg_update = false;
                    }
                } else {
                    ldc_proc_int->upd_mesh_mode = kAiqLdcUpdMeshFromExtBuf;
                    LOGK_ALDC("upd mesh from extern buffer in manual mode");
                }
            } else if (wrap_param.en && ldc_proc_int->upd_mesh_mode != kAiqLdcUpdMeshFromExtBuf) {
                // 1.2 Check whether the mesh file exist in 'calidb -> sw_ldcT_meshfile_pathfile'.
                XCamReturn ret_b = _handlerLdc_updMeshFromFile(pAlgoHandler, &aut_param, man_param);
                if (ret_b < 0) {
                    if (sharedCom->init) {
                        man_param->sta.ldchCfg.en = false;
#if RKAIQ_HAVE_LDCV
                        man_param->sta.ldcvCfg.en = false;
#endif
                    } else {
                        proc_res->cfg_update = false;
                    }
                } else {
                    ldc_proc_int->upd_mesh_mode = kAiqLdcUpdMeshFromFile;
                    LOGK_ALDC("upd mesh from file in manual mode");
                }
            }

            // 2. Check whether to disable LDC in manual mode.
            if (ldc_proc_int->upd_mesh_mode == kAiqLdcUpdMeshFromFile) {
                if (!wrap_param.en) {
                    man_param->sta.ldchCfg.en = false;
#if RKAIQ_HAVE_LDCV
                    man_param->sta.ldcvCfg.en = false;
#endif
                }
            } else if (ldc_proc_int->upd_mesh_mode == kAiqLdcUpdMeshFromExtBuf) {
                if (!man_param->sta.ldchCfg.en) man_param->sta.ldchCfg.en = false;
#if RKAIQ_HAVE_LDCV
                if (!man_param->sta.ldcvCfg.en) man_param->sta.ldcvCfg.en = false;
#endif
            }
        } else {
            proc_res->cfg_update = false;
        }
    } else {
        pAlgoHandler->mProcInParam->u.proc.is_attrib_update =
            GlobalParamsManager_getAndClearAlgoParamUpdateFlagLocked(globalParamsManager,
                                                                     pAlgoHandler->mResultType);

        pAlgoHandler->mIsUpdateGrpAttr = pAlgoHandler->mProcInParam->u.proc.is_attrib_update;

        if (!AiqCore_isGroupAlgo(pAlgoHandler->mAiqCore, pAlgoHandler->mDes->type)) {
            RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
            ret = des->processing(pAlgoHandler->mProcInParam, pAlgoHandler->mProcOutParam);
        } else {
            proc_res->cfg_update = false;
        }
        if (pAlgoHandler->mProcInParam->u.proc.is_attrib_update)
            pAlgoHandler->mOpMode = RK_AIQ_OP_MODE_AUTO;

        ldc_proc_int->upd_mesh_mode = kAiqLdcUpdMeshAuto;
    }
    GlobalParamsManager_unlockAlgoParam(globalParamsManager, pAlgoHandler->mResultType);

    return ret;
}

static XCamReturn _handlerLdc_processing(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_processing(pAlgoHandler);
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "ccm handle processing failed");
    }

    ret = _handlerLdc_do_processing_common(pAlgoHandler);

    RKAIQCORE_CHECK_RET(ret, "adebayer algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

AiqAlgoHandler_t* AiqAlgoHandlerLdc_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore) {
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)aiq_mallocz(sizeof(AiqAlgoHandlerLdc_t));
    if (!pHdl) return NULL;
    AiqAlgoHandler_constructor(pHdl, des, aiqCore);
    pHdl->processing   = _handlerLdc_processing;
    pHdl->genIspResult = AiqAlgoHandler_genIspResult_common;
    pHdl->prepare      = _handlerLdc_prepare;
    pHdl->init         = _handlerLdc_init;
    return pHdl;
}
