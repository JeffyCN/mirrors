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
#include "c_rk_ipcs_request.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "c_AIQIPCRequest"

void AIQIPCRequest_Constructor(AIQIPCRequest *me,\
                              fptr_AIQIPCRequest_setId setId_function,\
                              fptr_AIQIPCRequest_getId getId_function,\
                              fptr_AIQIPCRequest_setUri setUri_function,\
                              fptr_AIQIPCRequest_getUri getUri_function,\
                              fptr_AIQIPCRequest_getUriLength getUriLength_function,\
                              fptr_AIQIPCRequest_setContent setContent_function,\
                              fptr_AIQIPCRequest_getContent getContent_function,\
                              fptr_AIQIPCRequest_getContentLength getContentLength_function) {
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

void AIQIPCRequest_Destructor(AIQIPCRequest *me) {
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

void AIQIPCRequest_setId(AIQIPCRequest *me, RK_U32 id) {
    if (!me)
        return;

    me->mId = id;

    return;
}

RK_U32 AIQIPCRequest_getId(AIQIPCRequest *me) {
    if (!me)
        return 0;

    return me->mId;
}

RK_U32 AIQIPCRequest_getUriLength(AIQIPCRequest *const me) {
    if (!me)
        return 0;

    return me->mUriLength;
}

void AIQIPCRequest_setUri(AIQIPCRequest *me, const char *uri, RK_U32 length) {
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

const char *AIQIPCRequest_getUri(AIQIPCRequest *me) {
    if (!me)
        return NULL;

    return me->mUri;
}

void AIQIPCRequest_setContent(AIQIPCRequest *me, const char *content, RK_U32 length) {
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

const char* AIQIPCRequest_getContent(AIQIPCRequest *const me) {
    if (!me)
        return NULL;

    return me->mContent;
}

RK_U32 AIQIPCRequest_getContentLength(AIQIPCRequest *const me) {
    if (!me)
        return 0;

    return me->mContentLength;
}

AIQIPCRequest *AIQIPCRequestCreate(void) {
    AIQIPCRequest *me = (AIQIPCRequest *)malloc(sizeof(AIQIPCRequest));
    if (me != NULL) {
        AIQIPCRequest_Constructor(me,\
                                AIQIPCRequest_setId,\
                                AIQIPCRequest_getId,\
                                AIQIPCRequest_setUri,\
                                AIQIPCRequest_getUri,\
                                AIQIPCRequest_getUriLength,\
                                AIQIPCRequest_setContent,\
                                AIQIPCRequest_getContent,\
                                AIQIPCRequest_getContentLength);
    }

    return me;
}

void AIQIPCRequestDestroy(AIQIPCRequest *const me) {
    if (!me)
        return;
    AIQIPCRequest_Destructor(me);

    free(me);
    return;
}
