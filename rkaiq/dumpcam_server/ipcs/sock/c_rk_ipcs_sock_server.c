/* Copyright 2020 Rockchip Electronics Co. LTD
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

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <strings.h>

#include "c_rk_ipcs_sock_server.h"
#include "c_rk_info_service_control.h"
#ifdef HAVE_RK_OSAL
#include "rk_osal.h"
#endif
#include "xcam_log.h"
#include "xcam_defs.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "RKSockServer"

#define MESSAGE_BUFF_SIZE 1024

void AIQIPCSockServer_Constructor(AIQIPCSockServer *const me,\
                                 const char *uri,\
                                 fptr_AIQIPCSockServer_start            start_function,\
                                 fptr_AIQIPCSockServer_stop             stop_function,\
                                 fptr_AIQIPCSockServer_post             post_function,\
                                 fptr_AIQIPCSockServer_initServer       initServer_function,\
                                 fptr_AIQIPCSockServer_initDomainServer initDomainServer_function,\
                                 fptr_AIQIPCSockServer_initNetServer    initNetServer_function,\
                                 fptr_AIQIPCSockServer_deinit           deinit_function,\
                                 fptr_AIQIPCSockServer_receiveMessage   receiveMessage_function,\
                                 fptr_AIQIPCSockServer_receiveBuffer    receiveBuffer_function,\
                                 fptr_AIQIPCSockServer_handleNextClient handleNextClient_function,\
                                 fptr_AIQIPCSockServer_sendMessage      sendMessage_function,\
                                 fptr_AIQIPCSockServer_sendBuffer       sendBuffer_function,\
                                 fptr_AIQIPCSockServer_flush            flush_function,\
                                 fptr_AIQIPCSockServer_urlSplit         urlSplit_function) {
    /* initialize attributes */
    me->mServerSocketFd  = -1;
    me->mClientSocketFd  = -1;
    me->mKeepWorking     = RK_TRUE;
    me->mMsgBuf          = RK_NULL;
    me->mMsgBuf          = (char *)malloc(MESSAGE_BUFF_SIZE * sizeof(char));

    /* initialize member function pointers */
    me->start            = start_function;
    me->stop             = stop_function;
    me->post             = post_function;
    me->initServer       = initServer_function;
    me->initDomainServer = initDomainServer_function;
    me->initNetServer    = initNetServer_function;
    me->deinit           = deinit_function;
    me->receiveMessage   = receiveMessage_function;
    me->receiveBuffer    = receiveBuffer_function;
    me->handleNextClient = handleNextClient_function;
    me->sendMessage      = sendMessage_function;
    me->sendBuffer       = sendBuffer_function;
    me->flush            = flush_function;
    me->urlSplit         = urlSplit_function;

    me->initServer(me, uri);
    return;
}

void AIQIPCSockServer_Destructor(AIQIPCSockServer *const me) {
    if (!me)
        return;

    me->deinit(me);

    if (me->mMsgBuf) {
        free(me->mMsgBuf);
        me->mMsgBuf = RK_NULL;
    }

    return;
}

RK_BOOL AIQIPCSockServer_initServer(AIQIPCSockServer *const me, const char *uri) {
    RK_S32 port;
    char proto[MESSAGE_BUFF_SIZE], hostname[MESSAGE_BUFF_SIZE], path[MESSAGE_BUFF_SIZE];
    if (!me || !uri)
        return RK_FALSE;

    me->urlSplit(proto, sizeof(proto),
                 RK_NULL, 0,
                 hostname, sizeof(hostname),
                 &port, path, sizeof(path), uri);
    LOGD("proto: %s, hostname: %s, path: %s, port: %d", proto, hostname, path, port);

    if (!strcmp(proto, "tcp")) {
        me->initNetServer(me, hostname, port);
    } else if (!strcmp(proto, "domain")) {
        me->initDomainServer(me, path);
    }

    return RK_TRUE;
}

RK_BOOL AIQIPCSockServer_initNetServer(AIQIPCSockServer *const me, const char *hostname, RK_U32 port) {
    RK_S32 optval = 1;
    /* ipv4 address */
    struct sockaddr_in local;
    if (!me || !hostname || port <= 0 || port >= 65536) {
        LOGE("Port missing in uri");
        return RK_FALSE;
    }
    LOGD("binding to host:%s, port:%d", hostname, port);

    me->mServerSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (me->mServerSocketFd < 0) {
        LOGE("failed to create socket(AF_INET, SOCK_STREAM, 0)");
        return RK_FALSE;
    }

    if (setsockopt(me->mServerSocketFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        LOGE("Cannot set socket to reuse addr");
    }

    bzero(&local, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_port = htons(port);
    local.sin_addr.s_addr = inet_addr(hostname);
    if (bind(me->mServerSocketFd, (struct sockaddr* )(&local), sizeof(local)) == -1) {
        LOGE("bind failed, reason: %s", strerror(errno));
        close(me->mServerSocketFd);
        me->mServerSocketFd = -1;
        return RK_FALSE;
    }
    LOGD("sock server initialize succeed");

    return RK_TRUE;
}

RK_BOOL AIQIPCSockServer_initDomainServer(AIQIPCSockServer *const me, const char *path) {
    struct sockaddr_un local;
    unsigned local_length = 0;
    if (!me || !path)
        return RK_FALSE;
    LOGD("binding to %s", path);

    me->mServerSocketFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (me->mServerSocketFd < 0) {
        LOGE("failed to create socket(AF_UNIX, SOCK_STREAM, 0)");
        return RK_FALSE;
    }

    memset(&local, 0, sizeof(struct sockaddr_un));
    local.sun_family = AF_UNIX;
    snprintf(local.sun_path, sizeof(local.sun_path), "%s", path);
    unlink(local.sun_path);

    local_length = strlen(local.sun_path) + sizeof(local.sun_family);
    if (bind(me->mServerSocketFd, (struct sockaddr* )(&local), local_length) == -1) {
        LOGE("bind failed, reason: %s", strerror(errno));
        close(me->mServerSocketFd);
        me->mServerSocketFd = -1;
        return RK_FALSE;
    }
    LOGD("sock server initialize succeed");

    return RK_TRUE;
}

RK_BOOL AIQIPCSockServer_deinit(AIQIPCSockServer *const me) {
    if (!me)
        return RK_FALSE;

    if (me->mServerSocketFd >= 0) {
        close(me->mServerSocketFd);
        me->mServerSocketFd = -1;
    }

    if (me->mClientSocketFd >= 0) {
        close(me->mClientSocketFd);
        me->mClientSocketFd = -1;
    }

    return RK_TRUE;
}

RK_BOOL AIQIPCSockServer_start(AIQIPCSockServer *const me) {
    fd_set fds;
    struct timeval timeout;

    if (!me)
        return RK_FALSE;

    if (me->mServerSocketFd < 0) {
        LOGE("sock server not initialized");
        return RK_FALSE;
    }

    listen(me->mServerSocketFd, 1);


    while (me->mKeepWorking) {
        FD_ZERO(&fds);
        FD_SET(me->mServerSocketFd, &fds);
        timeout.tv_sec  = 1;
        timeout.tv_usec = 0;
        if (select(me->mServerSocketFd + 1, &fds, RK_NULL, RK_NULL, &timeout) <= 0) {

            continue;
        }

        me->mClientSocketFd = accept(me->mServerSocketFd, RK_NULL, RK_NULL);
        if (me->mClientSocketFd < 0) {
            LOGE("accept failed, reason: %s", strerror(errno));
            return RK_FALSE;
        }

        AIQIPCSockServer_handleNextClient(me, me->mClientSocketFd);

        close(me->mClientSocketFd);
        me->mClientSocketFd = -1;
    }
    LOGD("listening done");

    return RK_TRUE;
}

RK_BOOL AIQIPCSockServer_stop(AIQIPCSockServer *const me) {
    if (!me)
        return RK_FALSE;

    if (me->mServerSocketFd >= 0) {
        close(me->mServerSocketFd);
        me->mServerSocketFd = -1;
    }

    if (me->mClientSocketFd >= 0) {
        close(me->mClientSocketFd);
        me->mClientSocketFd = -1;
    }

    return RK_TRUE;
}

RK_BOOL AIQIPCSockServer_post(AIQIPCSockServer *const me, AIQIPCResponse *response) {
    return AIQIPCSockServer_sendMessage(me->mServerSocketFd, me->mClientSocketFd,
                                        response->getId(response),
                                        response->getContent(response),
                                        response->getContentLength(response));
}

RK_BOOL AIQIPCSockServer_handleNextClient(AIQIPCSockServer *const me, RK_S32 clientSocketFd) {
    if (!me)
        return RK_FALSE;
    LOGD("client connected. waiting for msg...");
    RK_U32 id  = 0;
    RK_U32 len = MESSAGE_BUFF_SIZE;

    while (RK_TRUE) {
        id = 0;
        len = MESSAGE_BUFF_SIZE;

        if (!me->receiveMessage(clientSocketFd, &id, me->mMsgBuf, &len)) {
            LOGE("receive message failed");
            return RK_TRUE;
        }
        // Message: [ID 4bytes][Length 4bytes][Data(uri#json_str#)]([CRC32 4bytes])

        if (len > 0) {
            char *uri        = RK_NULL;
            char *content    = RK_NULL;
            char *p          = RK_NULL;
            char *msgContent = me->mMsgBuf;
            char  delims[]   = "#";

            // find message uri
            p = strsep(&msgContent, delims);
            if (p == RK_NULL) {
                LOGE("Error to parse uri: %s", msgContent);
                return RK_FALSE;
            }
            uri = p;

            // find message content
            p = strsep(&msgContent, delims);
            if (p == RK_NULL) {
                LOGE("Error to parse message content: %s", msgContent);
                return RK_FALSE;
            }
            content = p;

            if (!strcmp(uri, "/cam/rkaiq/dump")) {
                AIQIPCRequest *request = AIQIPCRequestCreate();
                AIQIPCResponse *response = AIQIPCResponseCreate();
                AIQInfoServiceControl* infoService = AIQInfoServiceControlCreate();

                request->setId(request, id);
                request->setUri(request, uri, strlen(uri) +1);
                request->setContent(request, content, strlen(content) + 1);
                // request.setIPCServer(this);
                AIQInfoServiceControl_onReceived(infoService, request, response);
                if (response->getContentLength(response) > 0) {
                    if (!AIQIPCSockServer_post(me, response)) {
                        LOGE("Error send reponse: %s", strerror(errno));
                        return RK_FALSE;
                    }
                }

                AIQInfoServiceControlDestroy(infoService);
                AIQIPCResponseDestroy(response);
                AIQIPCRequestDestroy(request);
            }
        }

        return RK_TRUE;
    }
}

RK_BOOL AIQIPCSockServer_receiveMessage(RK_S32 clientSocketFd, RK_U32 *id, char *buf, RK_U32 *len) {
    if (!AIQIPCSockServer_receiveBuffer(clientSocketFd, (char *)id, sizeof(*id)))
        return RK_FALSE;

    if (!AIQIPCSockServer_receiveBuffer(clientSocketFd, (char *)len, sizeof(*len)))
        return RK_FALSE;

    if (*len > MESSAGE_BUFF_SIZE) {
        LOGE("received message len exceeds reception buffer size!");
        len = 0;
        AIQIPCSockServer_flush(clientSocketFd, buf, MESSAGE_BUFF_SIZE);
        return RK_FALSE;
    }

    if (!AIQIPCSockServer_receiveBuffer(clientSocketFd, buf, *len))
        return RK_FALSE;

    return RK_TRUE;
}

RK_BOOL AIQIPCSockServer_receiveBuffer(RK_S32 clientSocketFd, char *buf, RK_U32 len) {
    RK_S32 numBytesLeft = len;
    RK_S32 numBytesReceived;
    while ((numBytesLeft > 0) && ((numBytesReceived = recv(clientSocketFd, buf, numBytesLeft, 0)) > 0)) {
        numBytesLeft -= numBytesReceived;
        buf += numBytesReceived;
    }

    return (numBytesLeft == 0) ? RK_TRUE : RK_FALSE;
}

RK_BOOL AIQIPCSockServer_flush(RK_S32 clientSocketFd, char *buf, RK_U32 len) {
    RK_S32 numBytesReceived;
    while (((numBytesReceived = recv(clientSocketFd, buf, len, 0)) > 0)) {
        LOGD("flush: received %d bytes", numBytesReceived);
    }

    return RK_TRUE;
}

RK_BOOL AIQIPCSockServer_sendMessage(RK_S32 serverSocketFd, RK_S32 clientSocketFd,
                                    RK_U32 id, const char *buf, RK_U32 len) {
    if (!buf)
        return RK_FALSE;

    if (serverSocketFd < 0) {
        LOGE("sock server not initialized");
        return RK_FALSE;
    }

    if (!AIQIPCSockServer_sendBuffer(clientSocketFd, (char *)(&id), sizeof(id)))
        return RK_FALSE;

    if (!AIQIPCSockServer_sendBuffer(clientSocketFd, (char *)(&len), sizeof(len)))
        return RK_FALSE;

    if (!AIQIPCSockServer_sendBuffer(clientSocketFd, buf, len))
        return RK_FALSE;

    return RK_TRUE;
}

RK_BOOL AIQIPCSockServer_sendBuffer(RK_S32 clientSocketFd, const char *buf, RK_U32 len) {
    if (clientSocketFd < 0) {
        LOGE("%s: not initialized", __FUNCTION__);
        return RK_FALSE;
    }

    RK_S32 bytesLeft = len;
    RK_S32 numBytesSent;
    while ((bytesLeft > 0) && ((numBytesSent = send(clientSocketFd, buf, bytesLeft, MSG_NOSIGNAL)) > 0)) {
        bytesLeft -= numBytesSent;
        buf += numBytesSent;
    }

    return (bytesLeft == 0) ? RK_TRUE : RK_FALSE;
}

static RK_S32 util_strlcpy(char *dst, const char *src, RK_S32 size) {
    RK_S32 len = 0;
    while (++len < size && *src)
        *dst++ = *src++;
    if (len <= size)
        *dst = 0;
    return len + strlen(src) - 1;
}

RK_VOID AIQIPCSockServer_urlSplit(char *proto, RK_S32 protoSize,
                  char *authorization, RK_S32 authorizationSize,
                  char *hostname, RK_S32 hostnameSize,
                  RK_S32 *portPtr, char *path, RK_S32 pathSize, const char *url) {
    const char *p, *ls, *ls2, *at, *at2, *col, *brk;

    if (portPtr)
        *portPtr = -1;
    if (protoSize > 0)
        proto[0] = 0;
    if (authorizationSize > 0)
        authorization[0] = 0;
    if (hostnameSize > 0)
        hostname[0] = 0;
    if (pathSize > 0)
        path[0] = 0;

    /* parse protocol */
    if ((p = strchr(url, ':'))) {
        util_strlcpy(proto, url, XCAM_MIN(protoSize, p + 1 - url));
        p++; /* skip ':' */
        if (*p == '/')
            p++;
        if (*p == '/')
            p++;
    } else {
        /* no protocol means plain filename */
        util_strlcpy(path, url, pathSize);
        return;
    }

    /* separate path from hostname */
    ls = strchr(p, '/');
    ls2 = strchr(p, '?');
    if (!ls)
        ls = ls2;
    else if (ls && ls2)
        ls = XCAM_MIN(ls, ls2);
    if (ls)
        util_strlcpy(path, ls, pathSize);
    else
        ls = &p[strlen(p)];  // XXX

    /* the rest is hostname, use that to parse auth/port */
    if (ls != p) {
        /* authorization (user[:pass]@hostname) */
        at2 = p;
        while ((at = strchr(p, '@')) && at < ls) {
            util_strlcpy(authorization, at2,
                       XCAM_MIN(authorizationSize, at + 1 - at2));
            p = at + 1; /* skip '@' */
        }

        if (*p == '[' && (brk = strchr(p, ']')) && brk < ls) {
            /* [host]:port */
            util_strlcpy(hostname, p + 1,
                       XCAM_MIN(hostnameSize, brk - p));
            if (brk[1] == ':' && portPtr)
                *portPtr = atoi(brk + 2);
        } else if ((col = strchr(p, ':')) && col < ls) {
            util_strlcpy(hostname, p,
                       XCAM_MIN(col + 1 - p, hostnameSize));
            if (portPtr)
                *portPtr = atoi(col + 1);
        } else {
            util_strlcpy(hostname, p, XCAM_MIN(ls + 1 - p, hostnameSize));
        }
    }
}

AIQIPCSockServer *AIQIPCSockServerCreate(const char *uri) {
    AIQIPCSockServer *me = (AIQIPCSockServer *)malloc(sizeof(AIQIPCSockServer));
    if (NULL != me) {
        AIQIPCSockServer_Constructor(me,\
            uri,\
            AIQIPCSockServer_start,\
            AIQIPCSockServer_stop,\
            AIQIPCSockServer_post,\
            AIQIPCSockServer_initServer,\
            AIQIPCSockServer_initDomainServer,\
            AIQIPCSockServer_initNetServer,\
            AIQIPCSockServer_deinit,\
            AIQIPCSockServer_receiveMessage,\
            AIQIPCSockServer_receiveBuffer,\
            AIQIPCSockServer_handleNextClient,\
            AIQIPCSockServer_sendMessage,\
            AIQIPCSockServer_sendBuffer,\
            AIQIPCSockServer_flush,\
            AIQIPCSockServer_urlSplit);
    }

    return me;
}

void AIQIPCSockServerDestroy(AIQIPCSockServer *const me) {
    if (!me)
        return;

    AIQIPCSockServer_Destructor(me);
    free(me);
    return;
}
