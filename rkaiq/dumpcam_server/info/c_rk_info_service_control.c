/*
 * Copyright 2020 Rockchip Electronics Co. LTD
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

#include "c_rk_info_service_control.h"

#include "RkAiqVersion.h"
#include "dumpcam_server/include/rk_aiq_registry.h"
#include "rk_cmd_service_control.h"
#include "uAPI2_c/rk_aiq_api_private_c.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "c_AIQInfoServiceControl"

static void AIQInfo_dumpLogHelp(int type, st_string* dump_info);
static void AIQInfo_dumpAlgoHelp(int type, st_string* dump_info);
static void AIQInfo_dumpCoreHelp(int type, st_string* dump_info);
static void AIQInfo_dumpHwiHelp(int type, st_string* dump_info);
static void AIQInfo_dumpCamgroupHelp(int type, st_string* dump_info);
static void AIQInfo_dumpIspModHelp(int type, st_string* dump_info);

void AIQInfoServiceControl_Constructor(AIQInfoServiceControl* const me,
                                       fptr_AIQInfoServiceControl_onReceived onReceived_function) {
    me->parser = aiq_create_json_parser_ctx();
    me->onReceived = onReceived_function;
}

void AIQInfoServiceControl_Destructor(AIQInfoServiceControl *const me) {
    aiq_release_json_parser_ctx(me->parser);
}

AIQInfoServiceControl* AIQInfoServiceControlCreate() {
    AIQInfoServiceControl *me = (AIQInfoServiceControl*)malloc(sizeof(AIQInfoServiceControl));

    if (me) {
        AIQInfoServiceControl_Constructor(me, AIQInfoServiceControl_onReceived);
    }

    return me;
}

void AIQInfoServiceControlDestroy(AIQInfoServiceControl *const me) {
    if (!me)
        return;
    AIQInfoServiceControl_Destructor(me);
    free(me);
    return;
}

enum dump_mod_type {
    DUMP_AIQ_ALGO,
    DUMP_AIQ_CAMGROUP,
    DUMP_AIQ_CORE,
    DUMP_AIQ_HWI,

    DUMP_AIQ_MOD_AE,
    DUMP_AIQ_MOD_AWB,
    DUMP_AIQ_MOD_AF,
    DUMP_AIQ_MOD_DPC,
    DUMP_AIQ_MOD_MRG,
    DUMP_AIQ_MOD_CCM,
    DUMP_AIQ_MOD_LSC,
    DUMP_AIQ_MOD_BLC,
    DUMP_AIQ_MOD_RAWNR,
    DUMP_AIQ_MOD_GIC,
    DUMP_AIQ_MOD_DEBAYER,
    DUMP_AIQ_MOD_LUT3D,
    DUMP_AIQ_MOD_DEHZ,
    DUMP_AIQ_MOD_GAMMA,
    DUMP_AIQ_MOD_DEGAMMA,
    DUMP_AIQ_MOD_CSM,
    DUMP_AIQ_MOD_CGC,
    DUMP_AIQ_MOD_GAIN,
    DUMP_AIQ_MOD_CP,
    DUMP_AIQ_MOD_IE,
    DUMP_AIQ_MOD_TNR,
    DUMP_AIQ_MOD_YNR,
    DUMP_AIQ_MOD_CNR,
    DUMP_AIQ_MOD_SHARP,
    DUMP_AIQ_MOD_DRC,
    DUMP_AIQ_MOD_CAC,
    DUMP_AIQ_MOD_AFD,
    DUMP_AIQ_MOD_RGBIR,
    DUMP_AIQ_MOD_LDC,
    DUMP_AIQ_MOD_HISTEQ,
    DUMP_AIQ_MOD_ENH,
    DUMP_AIQ_MOD_TEXEST,
    DUMP_AIQ_MOD_HSV,
    DUMP_AIQ_MOD_AIBNR,
    DUMP_AIQ_MOD_AIYNR,
    DUMP_AIQ_MOD_MAX,

    DUMP_AIQ_MAX,
};

struct dump_mod_descr {
    enum dump_mod_type type;
    const char *descr;
    struct {
        int (*dump_fn_t)(void *dumper, st_string *dump_info, int argc, void *argv[]);
        void *dumper;
    } dump;
    void (*help_info)(int type, st_string* dump_info);
};

static struct dump_mod_descr aiq_modules[] = {
    // clang-format off
    { DUMP_AIQ_ALGO,            "algo",         { NULL, NULL }, AIQInfo_dumpAlgoHelp     },
    { DUMP_AIQ_CORE,            "core",         { NULL, NULL }, AIQInfo_dumpCoreHelp     },
    { DUMP_AIQ_HWI,             "hwi",          { NULL, NULL }, AIQInfo_dumpHwiHelp      },
    { DUMP_AIQ_CAMGROUP,        "camgroup",     { NULL, NULL }, AIQInfo_dumpCamgroupHelp },

    { DUMP_AIQ_MOD_AE,          "ae",           { NULL, NULL }, AIQInfo_dumpIspModHelp },
    { DUMP_AIQ_MOD_AWB,         "awb",          { NULL, NULL }, AIQInfo_dumpIspModHelp },
    { DUMP_AIQ_MOD_DPC,         "dpc",          { NULL, NULL }, AIQInfo_dumpIspModHelp },
    { DUMP_AIQ_MOD_MRG,         "merge",        { NULL, NULL }, AIQInfo_dumpIspModHelp },
    { DUMP_AIQ_MOD_CCM,         "ccm",          { NULL, NULL }, AIQInfo_dumpIspModHelp },
    { DUMP_AIQ_MOD_LSC,         "lsc",          { NULL, NULL }, AIQInfo_dumpIspModHelp },
    { DUMP_AIQ_MOD_BLC,         "blc",          { NULL, NULL }, AIQInfo_dumpIspModHelp },
    { DUMP_AIQ_MOD_DEBAYER,     "debayer",      { NULL, NULL }, AIQInfo_dumpIspModHelp },
    { DUMP_AIQ_MOD_GAMMA,       "gamma",        { NULL, NULL }, AIQInfo_dumpIspModHelp },
    { DUMP_AIQ_MOD_CSM,         "csm",          { NULL, NULL }, AIQInfo_dumpIspModHelp },
    { DUMP_AIQ_MOD_CGC,         "cgc",          { NULL, NULL }, AIQInfo_dumpIspModHelp },
    { DUMP_AIQ_MOD_GAIN,        "gain",         { NULL, NULL }, AIQInfo_dumpIspModHelp },
    { DUMP_AIQ_MOD_CP,          "cp",           { NULL, NULL }, AIQInfo_dumpIspModHelp },
    { DUMP_AIQ_MOD_TNR,         "tnr",          { NULL, NULL }, AIQInfo_dumpIspModHelp },
    { DUMP_AIQ_MOD_CNR,         "cnr",          { NULL, NULL }, AIQInfo_dumpIspModHelp },
    { DUMP_AIQ_MOD_SHARP,       "sharp",        { NULL, NULL }, AIQInfo_dumpIspModHelp },
    { DUMP_AIQ_MOD_DRC,         "drc",          { NULL, NULL }, AIQInfo_dumpIspModHelp },
    { DUMP_AIQ_MOD_CAC,         "cac",          { NULL, NULL }, AIQInfo_dumpIspModHelp },
    { DUMP_AIQ_MOD_AFD,         "afd",          { NULL, NULL }, AIQInfo_dumpIspModHelp },
    { DUMP_AIQ_MOD_LDC,         "ldc",          { NULL, NULL }, AIQInfo_dumpIspModHelp },
#if RKAIQ_HAVE_AF
    { DUMP_AIQ_MOD_AF,          "af",           { NULL, NULL }, AIQInfo_dumpIspModHelp },
#endif
#if RKAIQ_HAVE_DEHAZE
    { DUMP_AIQ_MOD_DEHZ,        "dehaz",        { NULL, NULL }, AIQInfo_dumpIspModHelp },
#endif
#if RKAIQ_HAVE_ENHANCE
    { DUMP_AIQ_MOD_ENH,         "enhaz",        { NULL, NULL }, AIQInfo_dumpIspModHelp },
#endif
#if RKAIQ_HAVE_DEHAZE
    { RKAIQ_HAVE_3DLUT,       "lut3d",        { NULL, NULL }, AIQInfo_dumpIspModHelp },
#endif
#if RKAIQ_HAVE_GIC
    { DUMP_AIQ_MOD_GIC,         "gic",          { NULL, NULL }, AIQInfo_dumpIspModHelp },
#endif
#if RKAIQ_HAVE_RGBIR_REMOSAIC
    { DUMP_AIQ_MOD_RGBIR,       "rgbir",        { NULL, NULL }, AIQInfo_dumpIspModHelp },
#endif
#if RKAIQ_HAVE_HISTEQ
    { DUMP_AIQ_MOD_HISTEQ,      "histeq",       { NULL, NULL }, AIQInfo_dumpIspModHelp },
#endif
#if RKAIQ_HAVE_HSV
    { DUMP_AIQ_MOD_HSV,         "hsv",          { NULL, NULL }, AIQInfo_dumpIspModHelp },
#endif
#if RKAIQ_HAVE_AIBNR
    { DUMP_AIQ_MOD_AIBNR,        "aibnr",         { NULL, NULL }, AIQInfo_dumpIspModHelp },
#endif
#if RKAIQ_HAVE_AIYNR
    { DUMP_AIQ_MOD_AIYNR,        "aiynr",         { NULL, NULL }, AIQInfo_dumpIspModHelp },
#endif
    { DUMP_AIQ_MOD_MAX,         "max",          { NULL, NULL }, AIQInfo_dumpIspModHelp },

    { DUMP_AIQ_MAX,             "unknown",      { NULL, NULL }, NULL }
    // clang-format on
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif
#define AIQ_MODS ARRAY_SIZE(aiq_modules)

static void AIQInfo_dumpVerInfoHelp(int type, st_string* dump_info) {
    aiq_string_printf(dump_info, "Dump detailed AIQ version info.");
}

static void AIQInfo_dumpLogHelp(int type, st_string* dump_info) {
    aiq_string_printf(dump_info, "Query or configure the debug log level of AIQ.");
}

static void AIQInfo_dumpAlgoHelp(int type, st_string* dump_info) {
    aiq_string_printf(dump_info, "Dump the state information of the ISP software algorithm.");
}

static void AIQInfo_dumpCoreHelp(int type, st_string* dump_info) {
    aiq_string_printf(dump_info, "Dump the state information of core management layer.");
}

static void AIQInfo_dumpHwiHelp(int type, st_string* dump_info) {
    aiq_string_printf(dump_info, "Dump the state information of hardware abstraction layer.");
}

static void AIQInfo_dumpCamgroupHelp(int type, st_string* dump_info) {
    aiq_string_printf(dump_info, "Dump the state information of camgroup management layer.");
}

static void AIQInfo_dumpIspModHelp(int type, st_string* dump_info) {
    char buffer[MAX_LINE_LENGTH] = {0};

    snprintf(buffer, MAX_LINE_LENGTH, "Dump the complete state information of %s module.",
             aiq_modules[type].descr);
    aiq_string_printf(dump_info, buffer);
}

static bool AIQInfo_validateArgs(int argc, char* argv[]) {
    for (int i = 0; i < argc; i++) {
        if (strcasecmp(argv[i], "-h") == 0 || strcasecmp(argv[i], "--help") == 0 ||
            strcasecmp(argv[i], "--log") == 0) {
            return true;
        }
    }
    return false;
}

static void __dumpListInfo(st_string* dumpInfo) {
    unsigned int i, longest = 0;
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_string_printf(dumpInfo, "\n Usage: ./dumpcam [moudle] [moudle ARGS]\n\n");
    aiq_string_printf(dumpInfo, " The modules supported by dumpcam include:\n");

    for (i = 0; i < AIQ_MODS - 2; i++) {
        if (longest < strlen(aiq_modules[i].descr)) longest = strlen(aiq_modules[i].descr);
    }

    snprintf(buffer, MAX_LINE_LENGTH, "   %-*s   ", longest, "ver-info");
    aiq_string_printf(dumpInfo, buffer);
    AIQInfo_dumpVerInfoHelp(-1, dumpInfo);
    aiq_string_printf(dumpInfo, "\n");

    aiq_memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "   %-*s   ", longest, "log");
    aiq_string_printf(dumpInfo, buffer);
    AIQInfo_dumpLogHelp(-1, dumpInfo);
    aiq_string_printf(dumpInfo, "\n");

    for (i = 0; i < AIQ_MODS - 2; i++) {
        aiq_memset(buffer, 0, MAX_LINE_LENGTH);

        snprintf(buffer, MAX_LINE_LENGTH, "   %-*s   ", longest, aiq_modules[i].descr);
        aiq_string_printf(dumpInfo, buffer);
        aiq_modules[i].help_info(i, dumpInfo);
        aiq_string_printf(dumpInfo, "\n");
    }

    aiq_string_printf(dumpInfo, "\n");
}

static void __dumpAiqVersionInfo(st_string* dumpInfo) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_string_printf(dumpInfo, "\n");

    snprintf(buffer, MAX_LINE_LENGTH,
             "************************** VERSION INFOS **************************\n\n"
             "Version Release Date:     %s\n"
             "AIQ Version:              %s\n"
             "\nGit Logs:\n%s\n"
             "************************ VERSION INFOS END ************************\n",
             RK_AIQ_RELEASE_DATE, RK_AIQ_VERSION
#ifdef GITLOGS
             ,
             GITLOGS
#else
             ,
             "null"
#endif
    );

    aiq_string_printf(dumpInfo, buffer);
    aiq_string_printf(dumpInfo, "\n");
}

static void __dumpAllModule(rk_aiq_sys_ctx_t* ctx, bool dumpPartialMods, st_string* dumpInfo,
                            int argc, void* argv[]) {
    if (!ctx) return;

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_string_printf(dumpInfo, "\n");

    for (int i = 0; i < (int)AIQ_MODS; i ++) {
        aiq_modules[i].dump.dumper    = NULL;
        aiq_modules[i].dump.dump_fn_t = NULL;

        if (aiq_modules[i].type == DUMP_AIQ_HWI) {
            AiqCamHwBase_t* camHw           = ctx->_camHw;
            aiq_modules[i].dump.dumper      = camHw;
            aiq_modules[i].dump.dump_fn_t   = camHw->dump;
        }

        if (aiq_modules[i].type == DUMP_AIQ_CORE) {
            AiqCore_t* analyzer           = ctx->_analyzer;
            aiq_modules[i].dump.dumper    = analyzer;
            aiq_modules[i].dump.dump_fn_t = analyzer->dump_core;
        }

        if (aiq_modules[i].type >= DUMP_AIQ_MOD_AE && aiq_modules[i].type < DUMP_AIQ_MOD_MAX) {
            AiqManager_t* mgr             = ctx->_rkAiqManager;
            aiq_modules[i].dump.dumper    = mgr;
            aiq_modules[i].dump.dump_fn_t = mgr->dump_mods;
        }

        if (!aiq_modules[i].dump.dumper || !aiq_modules[i].dump.dump_fn_t)
            continue;

        if (aiq_modules[i].type >= DUMP_AIQ_MOD_AE && aiq_modules[i].type < DUMP_AIQ_MOD_MAX) {
            // Only need to dump once in camgroup
            if (!dumpPartialMods) continue;

            char argvArray[1][256];
            char* extended_argv[256];
            int extended_argc = 0;

            snprintf(argvArray[0], sizeof(argvArray[extended_argc]), "%s", aiq_modules[i].descr);
            extended_argv[0] = argvArray[0];
            extended_argc++;

            aiq_modules[i].dump.dump_fn_t(aiq_modules[i].dump.dumper, dumpInfo, extended_argc,
                                          (void**)extended_argv);
            continue;
        }

        aiq_modules[i].dump.dump_fn_t(aiq_modules[i].dump.dumper, dumpInfo, argc, argv);
    }
}

void AIQInfoServiceControl_dumpAllModule(AIQInfoServiceControl* const me, st_string* dumpInfo,
                                         int argc, void* argv[]) {
    if (!me) return;

    RKAIQRegistry* registery = RKAIQRegistry_get();
    if (!registery) return;

    rk_aiq_sys_ctx_t* ctx = registery->mArray[0];
    if (!ctx) return;

    bool executeOnce = AIQInfo_validateArgs(argc, (char**)argv);

    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        const rk_aiq_camgroup_ctx_t* grpCtx = (rk_aiq_camgroup_ctx_t*)ctx;
        bool first_cam                      = true;
        for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
            __dumpAllModule(grpCtx->cam_ctxs_array[i], first_cam, dumpInfo, argc, argv);

            if (executeOnce) break;

            // Only need to dump once in camgroup
            if (grpCtx->cam_ctxs_array[i]) first_cam = false;
        }
    } else if (ctx->cam_type == RK_AIQ_CAM_TYPE_SINGLE) {
        for (int i = 0; i < registery->mCnt; i++) {
            __dumpAllModule(registery->mArray[i], true, dumpInfo, argc, argv);

            if (executeOnce) break;
        }
    }

    return;
}

static void __dumpMultiModule(rk_aiq_sys_ctx_t* ctx, bool dumpPartialMods, st_string* dumpInfo,
                              int argc, void* argv[], char* str) {
    if (!ctx) return;

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_string_printf(dumpInfo, "\n");

    for (int i = 0; i < (int)AIQ_MODS; i++) {
        aiq_modules[i].dump.dumper    = NULL;
        aiq_modules[i].dump.dump_fn_t = NULL;

        if (!strcasecmp(aiq_modules[i].descr, str)) {
            if (aiq_modules[i].type == DUMP_AIQ_HWI) {
                AiqCamHwBase_t* camHw         = ctx->_camHw;
                aiq_modules[i].dump.dumper    = camHw;
                aiq_modules[i].dump.dump_fn_t = camHw->dump;
            }

            if (aiq_modules[i].type == DUMP_AIQ_CORE) {
                AiqCore_t* analyzer           = ctx->_analyzer;
                aiq_modules[i].dump.dumper    = analyzer;
                aiq_modules[i].dump.dump_fn_t = analyzer->dump_core;
            }

            if (aiq_modules[i].type == DUMP_AIQ_ALGO) {
                // Only need to dump once in camgroup
                if (!dumpPartialMods) continue;

                AiqCore_t* analyzer           = ctx->_analyzer;
                aiq_modules[i].dump.dumper    = analyzer;
                aiq_modules[i].dump.dump_fn_t = analyzer->dump_algos;
            }

            if (aiq_modules[i].type >= DUMP_AIQ_MOD_AE && aiq_modules[i].type < DUMP_AIQ_MOD_MAX) {
                // Only need to dump once in camgroup
                if (!dumpPartialMods) continue;

                AiqManager_t* mgr             = ctx->_rkAiqManager;
                aiq_modules[i].dump.dumper    = mgr;
                aiq_modules[i].dump.dump_fn_t = mgr->dump_mods;
            }

            if (!aiq_modules[i].dump.dumper || !aiq_modules[i].dump.dump_fn_t) continue;

            aiq_modules[i].dump.dump_fn_t(aiq_modules[i].dump.dumper, dumpInfo, argc, argv);
        }
    }
}

void AIQInfo_prepareExtendedArgs(char* pos_str, int argc, void* argv[], char argvArray[][256],
                                 char* extended_argv[], int* extended_argc) {
    *extended_argc = 0;

    snprintf(argvArray[0], sizeof(argvArray[*extended_argc]), "%s", pos_str);
    extended_argv[0] = argvArray[0];
    (*extended_argc)++;
    for (int i = 0; i < argc; i++) {
        LOG1("argv[%d]: %s", i, *((const char**)argv + i));
        snprintf(argvArray[*extended_argc], sizeof(argvArray[*extended_argc]), "%s",
                 *((const char**)argv + i));
        extended_argv[*extended_argc] = argvArray[*extended_argc];
        (*extended_argc)++;
    }
}

void AIQInfoServiceControl_dumpMultiModule(AIQInfoServiceControl* const me, st_string* dumpInfo,
                                           char* moduleName, int argc, void* argv[]) {
    if (!me) return;

    int modulesCount = 0;
    string_list    *n = NULL;
    string_list    *pos = NULL;
    string_list    *modules = AIQInfoServiceControl_getModules(me, moduleName, '-');
    char argvArray[256][256];
    char* extended_argv[256];
    int extended_argc = 0;

    list_for_each_entry(pos, &modules->list, list) {
        modulesCount++;
    }
    pos = NULL;

    list_for_each_entry(pos, &modules->list, list) {
        RKAIQRegistry* registery = RKAIQRegistry_get();
        if (!registery) return;

        rk_aiq_sys_ctx_t* ctx = registery->mArray[0];
        if (!ctx) return;

        bool executeOnce = AIQInfo_validateArgs(argc, (char**)argv);

        if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
            const rk_aiq_camgroup_ctx_t* grpCtx = (rk_aiq_camgroup_ctx_t*)ctx;
            bool first_cam                      = true;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                AIQInfo_prepareExtendedArgs(pos->str, argc, argv, argvArray, extended_argv,
                                            &extended_argc);
                __dumpMultiModule(grpCtx->cam_ctxs_array[i], first_cam, dumpInfo, extended_argc,
                                  (void**)extended_argv, pos->str);

                if (executeOnce) break;

                // Only need to dump once in camgroup
                if (grpCtx->cam_ctxs_array[i]) first_cam = false;
            }
        } else if (ctx->cam_type == RK_AIQ_CAM_TYPE_SINGLE) {
            for (int i = 0; i < registery->mCnt; i++) {
                AIQInfo_prepareExtendedArgs(pos->str, argc, argv, argvArray, extended_argv,
                                            &extended_argc);
                __dumpMultiModule(registery->mArray[i], true, dumpInfo, extended_argc,
                                  (void**)extended_argv, pos->str);

                if (executeOnce) break;
            }
        }
    }
    pos = NULL;

    list_for_each_entry_safe(pos, n, &modules->list, list) {
        list_del(&pos->list);
        if (pos) {
            if (pos->str)
                free(pos->str);
            free(pos);
        }
    }
    if (modules) {
        free(modules);
        modules = NULL;
    }

    return;
}

static void AIQRawDump(st_string* result, int argc, void* argv[]) {
    RKAIQRegistry* registery = RKAIQRegistry_get();
    if (!registery) return;

    rk_aiq_sys_ctx_t* ctx = registery->mArray[0];
    if (!ctx) return;

    if (argc < 3) {
        aiq_string_printf(result, "\ninput param error\n");
        aiq_string_printf(result, "\n");
        return;
    }

    int dump_camid = atoi(argv[0]);

    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        const rk_aiq_camgroup_ctx_t* grpCtx = (rk_aiq_camgroup_ctx_t*)ctx;
        for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
            if (dump_camid == i || dump_camid == -1)
                grpCtx->cam_ctxs_array[i]->_rkAiqManager->dump_raw(grpCtx->cam_ctxs_array[i]->_rkAiqManager, result, argc, argv);
        }
    } else if (ctx->cam_type == RK_AIQ_CAM_TYPE_SINGLE) {
        for (int i = 0; i < registery->mCnt; i++) {
            if (dump_camid == i || dump_camid == -1)
                registery->mArray[i]->_rkAiqManager->dump_raw(registery->mArray[i]->_rkAiqManager, result, argc, argv);
        }
    }

    return;
}

void AIQInfoServiceControl_onReceived(AIQInfoServiceControl *const me,\
                                     AIQIPCRequest *request, AIQIPCResponse *response) {
    c_AIQJsonObject json = RK_NULL;
    char  *moudleName = RK_NULL;
    char **moduleArgsValue = RK_NULL;
    int    moduleArgsNum = 0;
    st_string *stResult = NULL;
    string_new(stResult);

    aiq_parseText(me->parser, request->getContent(request));
    aiq_json_get_string(me->parser, "module", &moudleName);
    aiq_json_get_string_array(me->parser, "moduleArgs", &moduleArgsValue, &moduleArgsNum);

    if (!moudleName)
        goto out;

    if (!strcmp(moudleName, "all")) {
        AIQInfoServiceControl_dumpAllModule(me, stResult, moduleArgsNum, (void **)moduleArgsValue);
        response->setContent(response, string_body(stResult), string_len(stResult));
    } else if (!strcmp(moudleName, "record")) {
        AIQRawDump(stResult, moduleArgsNum, (void**)moduleArgsValue);
        response->setContent(response, string_body(stResult), string_len(stResult));
    } else if (!strcmp(moudleName, "log")) {
        xcam_dump_log(stResult, moduleArgsNum, (void**)moduleArgsValue);
        response->setContent(response, string_body(stResult), string_len(stResult));
    } else if (!strcmp(moudleName, "ver-info")) {
        __dumpAiqVersionInfo(stResult);
        response->setContent(response, string_body(stResult), string_len(stResult));
    } else if (!strcmp(moudleName, "cat")) {
        char argvArray[1024];
        memcpy(argvArray, "cat ", 5);
        for (int i = 0; i < moduleArgsNum; i++) {
            snprintf(argvArray + strlen(argvArray), 1024 - strlen(argvArray), "%s ", moduleArgsValue[i]);
        }
        aiq_cmd_service_process(argvArray, stResult);
        response->setContent(response, string_body(stResult), string_len(stResult));
    } else if (!strcmp(moudleName, "list")) {
        __dumpListInfo(stResult);
        response->setContent(response, string_body(stResult), string_len(stResult));
    } else {
        AIQInfoServiceControl_dumpMultiModule(me, stResult, moudleName,  moduleArgsNum, (void **)moduleArgsValue);
        response->setContent(response, string_body(stResult), string_len(stResult));
    }

out:
    if (moduleArgsValue) {
        for (int i = 0; i < moduleArgsNum; i++) {
            if (moduleArgsValue[i])
                free(moduleArgsValue[i]);
        }
        free(moduleArgsValue);
    }

    if (moudleName)
        free(moudleName);

    string_free(stResult);  // important! ut string destroy at here.
}

string_list *AIQInfoServiceControl_getModules(AIQInfoServiceControl *const me,\
                                            const char *moduleName, const char delim) {
    if (!me)
        return NULL;

    string_list *head = NULL;
    char *token;
    char *tmpModuleName = (char *)malloc(strlen(moduleName) + 1);
    if (!tmpModuleName) {
        return head;
    }

    // Initialize the head of the list
    head = (string_list *)malloc(sizeof(string_list));
    if (!head) {
        free(tmpModuleName);
        goto __FREE_TMP_MOD_NAME;
    }
    INIT_LIST_HEAD(&head->list);

    strcpy(tmpModuleName, moduleName);

    token = strtok(tmpModuleName, &delim);

    while (NULL != token) {
        string_list *add = (string_list *)malloc(sizeof(string_list));
        if (add) {
            add->str = (char *)malloc(strlen(token) + 1);
            if (add->str) {
                strcpy(add->str, token);
                list_add_tail(&add->list, &head->list);
            } else {
                free(add);
            }
        }
        token = strtok(NULL, &delim);
    }

__FREE_TMP_MOD_NAME:
    free(tmpModuleName);

    return head;
}
