/******************************************************************************
 *
 * Copyright 2007, Silicon Image, Inc.  All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of: Silicon Image, Inc., 1060
 * East Arques Avenue, Sunnyvale, California 94085
 *
 *****************************************************************************/
/**
 * @file media_buffer.c
 *
 * @brief
 *          Media Buffer implementation
 *
 * <pre>
 *
 *   Principal Author: Joerg Detert
 *   Creation date:    Feb 28, 2008
 *
 * </pre>
 *
 *****************************************************************************/
#include <pthread.h>

#include "media_buffer.h"
#include "media_buffer_pool.h"

static pthread_mutex_t gAtomicMutex;

void AtomicMutexInit()
{
    pthread_mutex_init (&gAtomicMutex, NULL);
}

void AtomicMutexDestory()
{
    pthread_mutex_destroy (&gAtomicMutex);
}

static uint32_t osAtomicIncrement(uint32_t* pVar)
{
    uint32_t ret;

    /* Lock the atomic mutex */
    pthread_mutex_lock(&gAtomicMutex);

    /* increment */
    ret = ++(*pVar);

    /* Unlock the atomic mutex */
    pthread_mutex_unlock(&gAtomicMutex);

    return ret;
}


static uint32_t osAtomicDecrement(uint32_t* pVar)
{
    uint32_t ret;

    /* Lock the atomic mutex */
    pthread_mutex_lock(&gAtomicMutex);

    /* increment */
    ret = --(*pVar);

    /* Unlock the atomic mutex */
    pthread_mutex_unlock(&gAtomicMutex);

    return ret;
}

/******************************************************************************
 * MediaBufInit
 *****************************************************************************/
void MediaBufInit(MediaBuffer_t *pBuf)
{
    DCT_ASSERT(pBuf != NULL);

    pBuf->lockCount        = 0U;
    pBuf->pOwner           = NULL;
}


/******************************************************************************
 * MediaBufLockBuffer
 *****************************************************************************/
RESULT MediaBufLockBuffer(MediaBuffer_t* pBuf)
{
    DCT_ASSERT(pBuf != NULL);
    DCT_ASSERT(pBuf->pOwner != NULL);

    osAtomicIncrement( &pBuf->lockCount );

    return RET_SUCCESS;
}


/******************************************************************************
 * MediaBufUnlockBuffer
 *****************************************************************************/
RESULT MediaBufUnlockBuffer(MediaBuffer_t* pBuf)
{
    DCT_ASSERT(pBuf != NULL);
	if (pBuf->pOwner == NULL)
	{
		return RET_FAILURE;
	}

    uint32_t val = osAtomicDecrement( &pBuf->lockCount );

    if(val == 0U)
    {
        MediaBufPoolFreeBuffer(pBuf->pOwner, pBuf);
    }

    return RET_SUCCESS;
}
