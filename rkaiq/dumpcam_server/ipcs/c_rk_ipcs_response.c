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
#include "c_rk_ipcs_response.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "c_AIQIPCResponse"

void AIQIPCResponse_Constructor(AIQIPCResponse *me,\
                              void (*setId_function)(AIQIPCResponse *me, RK_U32 id),\
                              RK_U32 (*getId_function)(AIQIPCResponse *me),\
                              void (*setUri_function)(AIQIPCResponse *me, const char *uri, RK_U32 length),\
                              const char* (*getUri_function)(AIQIPCResponse *me),\
                              RK_U32 (*getUriLength_function)(AIQIPCResponse *me),\
                              void (*setContent_function)(AIQIPCResponse *me, const char *content, RK_U32 length),\
                              const char* (*getContent_function)(AIQIPCResponse *me),\
                              RK_U32 (*getContentLength_function)(AIQIPCResponse *me)) {
    me->mId              = 0;
    me->mUriLength       = 0;
    me->mContentLength   = 0;
    me->mUri             = RK_NULL;
    me->mContent         = RK_NULL;

    me->setId            = setId_function;
    me->getId            = getId_function;
    me->setUri           = setUri_function;
    me->getUri           = getUri_function;
    me->getUriLength     = getUriLength_function;
    me->setContent       = setContent_function;
    me->getContent       = getContent_function;
    me->getContentLength = getContentLength_function;

    return;
}

void AIQIPCResponse_Destructor(AIQIPCResponse *me) {
    if (!me)
        return;
    if (me->mUri) {
        free(me->mUri);
    }

    if (me->mContent) {
        free(me->mContent);
    }
    return;
}

void AIQIPCResponse_setId(AIQIPCResponse *me, RK_U32 id) {
    if (!me)
        return;

    me->mId = id;

    return;
}

RK_U32 AIQIPCResponse_getId(AIQIPCResponse *me) {
    if (!me)
        return 0;

    return me->mId;
}

RK_U32 AIQIPCResponse_getUriLength(AIQIPCResponse *const me) {
    if (!me)
        return 0;

    return me->mUriLength;
}

void AIQIPCResponse_setUri(AIQIPCResponse *me, const char *uri, RK_U32 length) {
    if (!me)
        return;

    if (me->mUri) {
        free(me->mUri);
    }

    if (0 != length) {
        me->mUri = (char*)malloc(length * sizeof(char));
        strncpy(me->mUri, uri, length);
    }
    me->mUriLength = length;

    return;
}

const char *AIQIPCResponse_getUri(AIQIPCResponse *me) {
    if (!me)
        return NULL;

    return me->mUri;
}

void AIQIPCResponse_setContent(AIQIPCResponse *me, const char *content, RK_U32 length) {
    if (!me)
        return;

    if (me->mContent) {
        free(me->mContent);
    }

    if (0 != length) {
        me->mContent = (char*)malloc(length * sizeof(char));
        strncpy(me->mContent, content, length);
    }
    me->mContentLength = length;

    return;
}

const char* AIQIPCResponse_getContent(AIQIPCResponse *const me) {
    if (!me)
        return NULL;

    return me->mContent;
}

RK_U32 AIQIPCResponse_getContentLength(AIQIPCResponse *const me) {
    if (!me)
        return 0;

    return me->mContentLength;
}

AIQIPCResponse *AIQIPCResponseCreate(void) {
    AIQIPCResponse *me = (AIQIPCResponse *)malloc(sizeof(AIQIPCResponse));
    if (me != NULL) {
        AIQIPCResponse_Constructor(me,\
                                AIQIPCResponse_setId,\
                                AIQIPCResponse_getId,\
                                AIQIPCResponse_setUri,\
                                AIQIPCResponse_getUri,\
                                AIQIPCResponse_getUriLength,\
                                AIQIPCResponse_setContent,\
                                AIQIPCResponse_getContent,\
                                AIQIPCResponse_getContentLength);
    }

    return me;
}

void AIQIPCResponseDestroy(AIQIPCResponse *const me) {
    if (!me)
        return;
    AIQIPCResponse_Destructor(me);

    free(me);
    return;
}
