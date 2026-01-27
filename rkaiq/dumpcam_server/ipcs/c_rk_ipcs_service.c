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
#include <stdlib.h>
#ifdef USE_IPCS_SOCK
#include "c_rk_ipcs_sock_server.h"
#endif
#include "c_rk_info_service_control.h"
#ifdef HAVE_RK_OSAL
#include "aiq_osal.h"
#else
#include "c_base/aiq_base.h"
#include "c_base/aiq_thread.h"
#endif

#ifndef __cplusplus

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "C_AIQIPCService"

#define SOCK_DOMAIN_PATH     "domain:///tmp/rkaiq_sock"
#define SOCK_TCP_PATH        "tcp://127.0.0.1:4894"

#define MESSAGE_BUFF_SIZE 1024

#ifdef HAVE_RK_OSAL
static rkos_task_t *gIPCThread = RK_NULL;
#else
static AiqThread_t* gIPCThread = RK_NULL;
#endif

#ifdef USE_IPCS_SOCK
static AIQIPCSockServer *gIPCServer = RK_NULL;
#endif

static RK_U32 ShellItemExtract1(RK_CHAR *pstr, RK_CHAR **pItem, RK_CHAR *Space)
{
    RK_U32 i = 0;
    RK_U32 ItemSize = 0;

    while (*pstr == *Space)
    {
        pstr++;
    }

    (*pItem) = (RK_CHAR *)pstr;

    while ((*(pstr + i) != *Space) && (*(pstr + i)))
    {
        ItemSize++;
        i++;
    }
    return ItemSize;
}

#ifdef USE_IPCS_SOCK
bool aiq_ipcs_server_proc(void *pArgs) {
    AIQIPCSockServer *server = (AIQIPCSockServer *)(pArgs);
#ifdef HAVE_RK_OSAL
    rkos_kthread_set_name("ipcs_server");
#endif
    AIQIPCSockServer_start(server);
    return 0;
}
#endif

RK_S32 aiq_ipcs_server_start() {
    RK_S32 result = RK_SUCCESS;
#ifdef USE_IPCS_SOCK
    if (gIPCServer == RK_NULL) {
        gIPCServer = AIQIPCSockServerCreate(SOCK_TCP_PATH);
        gIPCServer->mKeepWorking = RK_TRUE;
#ifdef HAVE_RK_OSAL
        gIPCThread = rkos_kthread_create(aiq_ipcs_server_proc, (void*)(gIPCServer), NULL, 2048, 0);
#else
        gIPCThread = aiqThread_init("aiq_ipcs_server", aiq_ipcs_server_proc, (void*)(gIPCServer));

        bool ret = aiqThread_start(gIPCThread);
        if (!ret) printf("Failed to start the thread of `ipcs_server`\n");
#endif
#endif
    }
    return result;
}

RK_S32 aiq_ipcs_server_stop() {
    RK_S32 result = RK_SUCCESS;
#ifdef USE_IPCS_SOCK
    if (gIPCServer != RK_NULL && gIPCServer->mKeepWorking) {
        gIPCServer->mKeepWorking = RK_FALSE;
        if (gIPCThread != RK_NULL) {
#ifdef HAVE_RK_OSAL
            rkos_kthread_destory(gIPCThread, 1);
#else
            bool ret = aiqThread_stop(gIPCThread);
            if (!ret)
                printf("Failed to start the thread of `ipcs_server`\n");
            aiqThread_deinit(gIPCThread);
#endif
            gIPCThread = RK_NULL;
        }
        AIQIPCSockServer_stop(gIPCServer);
        AIQIPCSockServerDestroy(gIPCServer);
        gIPCServer = RK_NULL;
    }
#endif
    return result;
}

RK_S32 aiq_ipcs_init() { return aiq_ipcs_server_start(); }

RK_S32 aiq_ipcs_exit(RK_VOID) {
    return aiq_ipcs_server_stop();
}

RK_S32 aiq_ipcs_execute_cmd(RK_CHAR *cmd, RK_CHAR *result, RK_U32 result_max_len) {
#ifdef HAVE_RK_OSAL
    char *jsonBuffer = rkos_vmalloc(512);
    char *command = rkos_vmalloc(512);
#else
    char *jsonBuffer = aiq_mallocz(512);
    char *command = aiq_mallocz(512);
#endif
    char *pcmd;
    char *argv[10];
    int cnt[10];
    int argc = 0;
    RK_U32 str_cnt;
    RK_CHAR *pitem;
    RK_CHAR space = ' ';
    memset(&argv[0], 0, 40);
    memset(command, 0, 512);
    memset(jsonBuffer, 0, 512);
    memcpy(command, cmd, strlen(cmd));
    pcmd = command;

    str_cnt = ShellItemExtract1(pcmd, &pitem, &space);
    while (str_cnt)
    {
        argv[argc] = pitem;
        cnt[argc] = str_cnt;
        pitem[str_cnt] = 0;
        argc++;
        pcmd = pitem + str_cnt + 1;
    }

    // set defaule service name all
    if (argc == 1) {
        strcat(jsonBuffer, "{\"");
        strcat(jsonBuffer, "module");
        strcat(jsonBuffer, "\":\"");
        strcat(jsonBuffer, "all");
        strcat(jsonBuffer, "\"");
        strcat(jsonBuffer, "}");
    } else {
        strcat(jsonBuffer, "{\"");
        strcat(jsonBuffer, "module");
        strcat(jsonBuffer, "\":\"");
        // dumpsys usage module name + argv
        for (int i = 1; i < argc; i++) {
            if (i == 1) {
                strcat(jsonBuffer, argv[i]);
                strcat(jsonBuffer, "\"");
                strcat(jsonBuffer, ", \"moduleArgs\":[");
            } else {
                strcat(jsonBuffer, "\"");
                strcat(jsonBuffer, argv[i]);
                strcat(jsonBuffer, "\"");
                if (i != (argc -1)) {
                    strcat(jsonBuffer, ",");
                } else {
                    strcat(jsonBuffer, "]}");
                }
            }
        }
    }
    for (int i = 1; i < argc - 1; i++)
        argv[i][cnt[i]] = ' ';

    RK_CHAR *content = RK_NULL;
    RK_CHAR *msgContent = jsonBuffer;
    content = msgContent;
    AIQIPCRequest *request = AIQIPCRequestCreate();
    AIQIPCResponse *response = AIQIPCResponseCreate();
    AIQInfoServiceControl* infoService = AIQInfoServiceControlCreate();
    request->setId(request, 0); //app call
    request->setUri(request, "/cam/rkaiq/dump", strlen("/cam/rkaiq/dump") + 1);
    request->setContent(request, content, strlen(content) + 1);
    AIQInfoServiceControl_onReceived(infoService, request, response);
    if (response->getContentLength(response) > 0) {
        if ((result != NULL) && result_max_len) {
            memcpy(result, cmd, strlen(cmd));
            result = result + strlen(cmd);
            result[0] = ':';
            result[1] = '\n';
            result = result + 2;
            memcpy(result, response->getContent(response), response->getContentLength(response));
            result[response->getContentLength(response)] = 0;
        }
    } else
        result[0] = 0;
#ifdef HAVE_RK_OSAL
    rkos_vfree(jsonBuffer);
    rkos_vfree(command);
#else
    aiq_free(jsonBuffer);
    aiq_free(command);
#endif

    return RK_SUCCESS;
}
#endif
