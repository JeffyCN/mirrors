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
#ifndef INCLUDE_RT_MPI_C_RK_IPCS_SOCK_SERVER_H_
#define INCLUDE_RT_MPI_C_RK_IPCS_SOCK_SERVER_H_

#include "rkaiq_type.h"
#include "c_rk_ipcs_response.h"
#include "c_rk_ipcs_request.h"

typedef struct _AIQIPCSockServer AIQIPCSockServer;

typedef RK_BOOL (*fptr_AIQIPCSockServer_start)(AIQIPCSockServer *const me);
typedef RK_BOOL (*fptr_AIQIPCSockServer_stop)(AIQIPCSockServer *const me);
typedef RK_BOOL (*fptr_AIQIPCSockServer_post)(AIQIPCSockServer *const me, AIQIPCResponse *response);

typedef RK_BOOL (*fptr_AIQIPCSockServer_initServer)(AIQIPCSockServer *const me, const char *uri);
typedef RK_BOOL (*fptr_AIQIPCSockServer_initDomainServer)(AIQIPCSockServer *const me, const char *path);
typedef RK_BOOL (*fptr_AIQIPCSockServer_initNetServer)(AIQIPCSockServer *const me, const char *hostname, RK_U32 port);
typedef RK_BOOL (*fptr_AIQIPCSockServer_deinit)(AIQIPCSockServer *const me);

typedef RK_BOOL (*fptr_AIQIPCSockServer_receiveMessage)(RK_S32 client_socket_fd,
                            RK_U32 *id, char *buf, RK_U32 *len);
typedef RK_BOOL (*fptr_AIQIPCSockServer_receiveBuffer)(RK_S32 client_socket_fd, char *buf, RK_U32 len);
typedef RK_BOOL (*fptr_AIQIPCSockServer_handleNextClient)(AIQIPCSockServer *const me, RK_S32 client_socket_fd);
typedef RK_BOOL (*fptr_AIQIPCSockServer_sendMessage)(RK_S32 server_socket_fd, RK_S32 client_socket_fd,\
                                                    RK_U32 id, const char *buf, RK_U32 len);
typedef RK_BOOL (*fptr_AIQIPCSockServer_sendBuffer)(RK_S32 client_socket_fd, const char *buf, RK_U32 len);
typedef RK_BOOL (*fptr_AIQIPCSockServer_flush)(RK_S32 client_socket_fd, char *buf, RK_U32 len);
typedef RK_VOID (*fptr_AIQIPCSockServer_urlSplit)(char *proto, RK_S32 protoSize,
                    char *authorization, RK_S32 authorizationSize,
                    char *hostname, RK_S32 hostnameSize,
                    RK_S32 *portPtr, char *path, RK_S32 pathSize, const char *url);

struct _AIQIPCSockServer {
    RK_S32 mServerSocketFd;
    RK_S32 mClientSocketFd;
    RK_BOOL mKeepWorking;
    char *mMsgBuf;

    fptr_AIQIPCSockServer_start start;
    fptr_AIQIPCSockServer_stop stop;
    fptr_AIQIPCSockServer_post post;
    fptr_AIQIPCSockServer_initServer initServer;
    fptr_AIQIPCSockServer_initDomainServer initDomainServer;
    fptr_AIQIPCSockServer_initNetServer initNetServer;
    fptr_AIQIPCSockServer_deinit deinit;
    fptr_AIQIPCSockServer_receiveMessage receiveMessage;
    fptr_AIQIPCSockServer_receiveBuffer receiveBuffer;
    fptr_AIQIPCSockServer_handleNextClient handleNextClient;
    fptr_AIQIPCSockServer_sendMessage sendMessage;
    fptr_AIQIPCSockServer_sendBuffer sendBuffer;
    fptr_AIQIPCSockServer_flush flush;
    fptr_AIQIPCSockServer_urlSplit urlSplit;
};

#ifdef __cplusplus
extern "C" {
#endif

/* Constructor and Destructor */
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
                                 fptr_AIQIPCSockServer_urlSplit         urlSplit_function);
void AIQIPCSockServer_Destructor(AIQIPCSockServer *const me);

/* Operation */
RK_BOOL AIQIPCSockServer_start(AIQIPCSockServer *const me);
RK_BOOL AIQIPCSockServer_stop(AIQIPCSockServer *const me);
RK_BOOL AIQIPCSockServer_post(AIQIPCSockServer *const me, AIQIPCResponse *response);
RK_BOOL AIQIPCSockServer_initServer(AIQIPCSockServer *const me, const char *uri);
RK_BOOL AIQIPCSockServer_initDomainServer(AIQIPCSockServer *const me, const char *path);
RK_BOOL AIQIPCSockServer_initNetServer(AIQIPCSockServer *const me, const char *hostname, RK_U32 port);
RK_BOOL AIQIPCSockServer_deinit(AIQIPCSockServer *const me);

RK_BOOL AIQIPCSockServer_receiveMessage(RK_S32 client_socket_fd,
                                        RK_U32 *id, char *buf, RK_U32 *len);
RK_BOOL AIQIPCSockServer_receiveBuffer(RK_S32 client_socket_fd, char *buf, RK_U32 len);

RK_BOOL AIQIPCSockServer_handleNextClient(AIQIPCSockServer *const me, RK_S32 client_socket_fd);

RK_BOOL AIQIPCSockServer_sendMessage(RK_S32 server_socket_fd, RK_S32 client_socket_fd,\
                                    RK_U32 id, const char *buf, RK_U32 len);
RK_BOOL AIQIPCSockServer_sendBuffer(RK_S32 client_socket_fd, const char *buf, RK_U32 len);

RK_BOOL AIQIPCSockServer_flush(RK_S32 client_socket_fd, char *buf, RK_U32 len);
RK_VOID AIQIPCSockServer_urlSplit(char *proto, RK_S32 protoSize,
                                    char *authorization, RK_S32 authorizationSize,
                                    char *hostname, RK_S32 hostnameSize,
                                    RK_S32 *portPtr, char *path, RK_S32 pathSize, const char *url);

AIQIPCSockServer *AIQIPCSockServerCreate(const char *uri);
void AIQIPCSockServerDestroy(AIQIPCSockServer *const me);
#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_RT_MPI_C_RK_IPCS_SOCK_SERVER_H_ */
