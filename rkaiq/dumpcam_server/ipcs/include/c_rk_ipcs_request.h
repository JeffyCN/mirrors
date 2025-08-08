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

#ifndef INCLUDE_RT_MPI_C_RK_IPCS_REQUEST_H__
#define INCLUDE_RT_MPI_C_RK_IPCS_REQUEST_H__

#include <string.h>
#include "rkaiq_type.h"
#include "c_rk_ipcs_response.h"

typedef struct _AIQIPCRequest AIQIPCRequest;

typedef void (*fptr_AIQIPCRequest_setId)(AIQIPCRequest *me, RK_U32 id);
typedef RK_U32 (*fptr_AIQIPCRequest_getId)(AIQIPCRequest *me);
typedef void (*fptr_AIQIPCRequest_setUri)(AIQIPCRequest *me, const char *uri, RK_U32 length);
typedef const char* (*fptr_AIQIPCRequest_getUri)(AIQIPCRequest *me);
typedef RK_U32 (*fptr_AIQIPCRequest_getUriLength)(AIQIPCRequest *me);

typedef void (*fptr_AIQIPCRequest_setContent)(AIQIPCRequest *me, const char *content, RK_U32 length);
typedef const char* (*fptr_AIQIPCRequest_getContent)(AIQIPCRequest *me);
typedef RK_U32 (*fptr_AIQIPCRequest_getContentLength)(AIQIPCRequest *me);

typedef void (*fptr_AIQIPCRequest_setIPCServer)(void *server);
typedef RK_BOOL (*fptr_AIQIPCRequest_post)(AIQIPCResponse *response);

struct _AIQIPCRequest {
    RK_U32 mId;
    RK_U32 mUriLength;
    RK_U32 mContentLength;
    char *mUri;
    char *mContent;

    fptr_AIQIPCRequest_setId setId;
    fptr_AIQIPCRequest_getId getId;

    fptr_AIQIPCRequest_setUri setUri;
    fptr_AIQIPCRequest_getUri getUri;
    fptr_AIQIPCRequest_getUriLength getUriLength;

    fptr_AIQIPCRequest_setContent setContent;
    fptr_AIQIPCRequest_getContent getContent;
    fptr_AIQIPCRequest_getContentLength getContentLength;

    fptr_AIQIPCRequest_setIPCServer setIPCServer;
    fptr_AIQIPCRequest_post post;
};


#ifdef __cplusplus
extern "C" {
#endif

/* Constructor and Destructor */
void AIQIPCRequest_Constructor(AIQIPCRequest *me,\
                              fptr_AIQIPCRequest_setId setId_function,\
                              fptr_AIQIPCRequest_getId getId_function,\
                              fptr_AIQIPCRequest_setUri setUri_function,\
                              fptr_AIQIPCRequest_getUri getUri_function,\
                              fptr_AIQIPCRequest_getUriLength getUriLength_function,\
                              fptr_AIQIPCRequest_setContent setContent_function,\
                              fptr_AIQIPCRequest_getContent getContent_function,\
                              fptr_AIQIPCRequest_getContentLength getContentLength_function);
void AIQIPCRequest_Destructor(AIQIPCRequest *me);

/* Operation */
void AIQIPCRequest_setId(AIQIPCRequest *me, RK_U32 id);
RK_U32 AIQIPCRequest_getId(AIQIPCRequest *me);

void AIQIPCRequest_setUri(AIQIPCRequest *me, const char *uri, RK_U32 length);
const char* AIQIPCRequest_getUri(AIQIPCRequest *const me);
RK_U32 AIQIPCRequest_getUriLength(AIQIPCRequest *const me);

void AIQIPCRequest_setContent(AIQIPCRequest *me, const char *content, RK_U32 length);
const char* AIQIPCRequest_getContent(AIQIPCRequest *const me);
RK_U32 AIQIPCRequest_getContentLength(AIQIPCRequest *const me);

AIQIPCRequest *AIQIPCRequestCreate(void);
void AIQIPCRequestDestroy(AIQIPCRequest *const me);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_RT_MPI_C_RK_IPCS_REQUEST_H__ */
