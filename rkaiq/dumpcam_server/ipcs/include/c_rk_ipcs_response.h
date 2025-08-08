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
#ifndef INCLUDE_RT_MPI_C_RK_IPCS_RESPONSE_H__
#define INCLUDE_RT_MPI_C_RK_IPCS_RESPONSE_H__

#include <string.h>
#include "rkaiq_type.h"

typedef struct _AIQIPCResponse AIQIPCResponse;

struct _AIQIPCResponse {
    RK_U32 mId;
    RK_U32 mUriLength;
    RK_U32 mContentLength;
    char *mUri;
    char *mContent;

    void (*setId)(AIQIPCResponse *me, RK_U32 id);
    RK_U32 (*getId)(AIQIPCResponse *me);

    void (*setUri)(AIQIPCResponse *me, const char *uri, RK_U32 length);
    const char* (*getUri)(AIQIPCResponse *me);
    RK_U32 (*getUriLength)(AIQIPCResponse *me);

    void (*setContent)(AIQIPCResponse *me, const char *content, RK_U32 length);
    const char* (*getContent)(AIQIPCResponse *me);
    RK_U32 (*getContentLength)(AIQIPCResponse *me);
};


#ifdef __cplusplus
extern "C" {
#endif

/* Constructor and Destructor */
void AIQIPCResponse_Constructor(AIQIPCResponse *me,\
                              void (*setId_function)(AIQIPCResponse *me, RK_U32 id),\
                              RK_U32 (*getId_function)(AIQIPCResponse *me),\
                              void (*setUri_function)(AIQIPCResponse *me, const char *uri, RK_U32 length),\
                              const char* (*getUri_function)(AIQIPCResponse *me),\
                              RK_U32 (*getUriLength_function)(AIQIPCResponse *me),\
                              void (*setContent_function)(AIQIPCResponse *me, const char *content, RK_U32 length),\
                              const char* (*getContent_function)(AIQIPCResponse *me),\
                              RK_U32 (*getContentLength_function)(AIQIPCResponse *me));
void AIQIPCResponse_Destructor(AIQIPCResponse *me);

/* Operation */
void AIQIPCResponse_setId(AIQIPCResponse *me, RK_U32 id);
RK_U32 AIQIPCResponse_getId(AIQIPCResponse *me);

void AIQIPCResponse_setUri(AIQIPCResponse *me, const char *uri, RK_U32 length);
const char* AIQIPCResponse_getUri(AIQIPCResponse *const me);
RK_U32 AIQIPCResponse_getUriLength(AIQIPCResponse *const me);

void AIQIPCResponse_setContent(AIQIPCResponse *me, const char *content, RK_U32 length);
const char* AIQIPCResponse_getContent(AIQIPCResponse *const me);
RK_U32 AIQIPCResponse_getContentLength(AIQIPCResponse *const me);

AIQIPCResponse *AIQIPCResponseCreate(void);
void AIQIPCResponseDestroy(AIQIPCResponse *const me);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_RT_MPI_C_RK_IPCS_RESPONSE_H__ */
