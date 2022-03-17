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
 * @file media_buffer_pool.c
 *
 * @brief
 *          Buffer Pool implementation
 *
 * <pre>
 *
 *   Principal Author: Joerg Detert
 *   Creation date:    Feb 28, 2008
 *
 * </pre>
 *
 *****************************************************************************/

#include <string.h>

#include "media_buffer_pool.h"



#define MEDIA_BUF_ALIGN(buf, align) (((buf)+((align)-1U)) & ~((align)-1U))


/******************************************************************************
 * MediaBufPoolGetSize
 *****************************************************************************/
RESULT MediaBufPoolGetSize(MediaBufPoolConfig_t* pPoolConfig)
{
    RESULT  ret = RET_SUCCESS;

    if(pPoolConfig == NULL)
    {
        return RET_WRONG_HANDLE;
    }

    if(pPoolConfig->bufNum > pPoolConfig->maxBufNum)
    {
        return RET_WRONG_CONFIG;
    }

    /* size of extra metadata */
    pPoolConfig->metaDataMemSize = pPoolConfig->maxBufNum * pPoolConfig->metaDataSizeMediaBuf;

    /* size of media buffer array */
    pPoolConfig->metaDataMemSize += (uint32_t)(pPoolConfig->maxBufNum * sizeof(MediaBuffer_t));

    /* calculate size of buffer array*/
    if(pPoolConfig->flags & (uint32_t)BUFPOOL_RINGBUFFER)
    {
        /* results in no gaps between buffers but an offset depending on the alignment */
        pPoolConfig->bufMemSize = (pPoolConfig->maxBufNum * pPoolConfig->bufSize) +
                                   pPoolConfig->bufAlign;

        /* the buffer size must be correct to the alignment otherwise we would have
         * unaligned buffers after the first which is always aligned */
        if((pPoolConfig->bufSize & ((uint32_t)pPoolConfig->bufAlign - 1U)) != 0U)
        {
            return RET_WRONG_CONFIG;
        }
    }
    else
    {
        /* this leads to gaps between the buffers dependend on the alignment */
        pPoolConfig->bufMemSize = pPoolConfig->maxBufNum * (pPoolConfig->bufSize +
                                  pPoolConfig->bufAlign);
    }

    return ret;
}


/******************************************************************************
 * MediaBufPoolCreate
 *****************************************************************************/
RESULT MediaBufPoolCreate(MediaBufPool_t*       pBufPool,
                          MediaBufPoolConfig_t* pPoolConfig,
                          MediaBufPoolMemory_t  poolMemory)
{
    uint32_t        i;
    uint32_t        offset;
    void*           pBuf;
    RESULT  ret = RET_SUCCESS;
    if(pBufPool == NULL)
    {
        return RET_WRONG_HANDLE;
    }

    if((poolMemory.pMetaDataMemory  == NULL) ||
       (poolMemory.pBufferMemory    == NULL))
    {
        return RET_INVALID_PARM;
    }

    if((pPoolConfig->bufNum == 0U) || (pPoolConfig->bufSize == 0U) ||
       (pPoolConfig->maxBufNum < pPoolConfig->bufNum))
    {
        return RET_WRONG_CONFIG;
    }

    /* If buffer pool is switched to ringbuffer mode, there must not be any gap*/
    /* between individual buffers inside static buffer pool, introduced by data*/
    /* alignement issues (e.g. rouding up of internal buffer sizes).*/
    /* That is, the buffer size must be a multiple of the data alignement.*/
    if((pPoolConfig->flags & BUFPOOL_RINGBUFFER) &&
       (pPoolConfig->bufSize % pPoolConfig->bufAlign))
    {
        return RET_WRONG_CONFIG;
    }

    /* initialize buffer pool object*/
    (void) memset(pBufPool, 0, sizeof(MediaBufPool_t));

    pBufPool->bufSize              = pPoolConfig->bufSize;
    pBufPool->metaDataSizeMediaBuf = pPoolConfig->metaDataSizeMediaBuf;
    pBufPool->bufNum               = pPoolConfig->bufNum;
    pBufPool->freeBufNum           = pPoolConfig->bufNum;
    pBufPool->maxBufNum            = pPoolConfig->maxBufNum;
    pBufPool->poolSize             = pPoolConfig->bufNum * pPoolConfig->bufSize;
    pBufPool->flags                = pPoolConfig->flags;

    /* The memory given by the caller is assigned to the buffer pool */
    pBufPool->pBaseAddress = poolMemory.pMetaDataMemory;

    /* Make sure that sizes in pPoolConfig are written */
    (void) MediaBufPoolGetSize(pPoolConfig);
    /* initialize buffer memory to zero */
    (void) memset(poolMemory.pMetaDataMemory, 0, (size_t) pPoolConfig->metaDataMemSize);

    /* We use the first partition of memory for the media buffer array */
    pBufPool->pBufArray = (MediaBuffer_t*) pBufPool->pBaseAddress;

    /* The second partition of memory is used for the buffer meta data array for Media Buffers */
    offset = (pPoolConfig->maxBufNum * sizeof(MediaBuffer_t));
    pBufPool->pMetaDataMediaBufBase = (void*) (((ulong_t) pBufPool->pBaseAddress) + offset);

    /* The memory pointed to by pBufferMemory is used for
     * the real buffer memory. This memory is assigned to the pointers stored in the corresponding
     * MediaBuffer_t[]. Due to the alignment restrictions given by the user it must be kept in mind
     * that the alignment is added. */
    pBuf = poolMemory.pBufferMemory;
    for(i = 0U; i < pBufPool->maxBufNum; i++)
    {
        /* set base address of buffer
           (will be changed in case that buffer pool is resized) */
        pBufPool->pBufArray[i].pBaseAddress = (uint8_t*) MEDIA_BUF_ALIGN((ulong_t)pBuf +
                        (i * pPoolConfig->bufSize), (uint32_t)pPoolConfig->bufAlign);
        /* set base size of buffer
           (will be changed in case that buffer pool is resized) */
        pBufPool->pBufArray[i].baseSize = pBufPool->bufSize;

        if ( pPoolConfig->metaDataSizeMediaBuf != 0 )
        {
            /* set Media Buffer meta data structure address for
               (stays the same during lifetime of buffer pool) */
            pBufPool->pBufArray[i].pMetaData = (void *)((ulong_t) pBufPool->pMetaDataMediaBufBase +
                                               (i * pPoolConfig->metaDataSizeMediaBuf));
        }
        else
        {
            pBufPool->pBufArray[i].pMetaData = NULL;
        }

        MediaBufInit(&pBufPool->pBufArray[i]);
    }

    AtomicMutexInit();

    return ret;
}


/******************************************************************************
 * MediaBufPoolDestroy
 *****************************************************************************/
RESULT MediaBufPoolDestroy(MediaBufPool_t *pBufPool)
{
    DCT_ASSERT(pBufPool != NULL);

    (void) memset(pBufPool, 0, sizeof(MediaBufPool_t));
    AtomicMutexDestory();
    return RET_SUCCESS;
}


/******************************************************************************
 * MediaBufPoolReset
 *****************************************************************************/
RESULT MediaBufPoolReset(MediaBufPool_t* pBufPool)
{
    uint32_t i;

    DCT_ASSERT(pBufPool != NULL);

    /* reset state variables */
    pBufPool->freeBufNum = pBufPool->bufNum;
    pBufPool->fillLevel  = 0;
    pBufPool->index      = 0;

    /* re-init media buffers */
    for(i = 0U; i < pBufPool->maxBufNum; i++)
    {
        MediaBufInit(&pBufPool->pBufArray[i]);
    }

    return RET_SUCCESS;
}


/******************************************************************************
 * MediaBufPoolRegisterCb
 *****************************************************************************/
RESULT MediaBufPoolRegisterCb(MediaBufPool_t*         pBufPool,
                              MediaBufPoolCbNotify_t  fpCallback,
                              void*                   pUserContext)
{
    RESULT ret = RET_NOTAVAILABLE;
    int32_t i;

    if(pBufPool == NULL)
    {
        return RET_WRONG_HANDLE;
    }

    if(fpCallback == NULL)
    {
        return RET_INVALID_PARM;
    }

    if(pBufPool->notify.fpCallback == NULL)
    {
        pBufPool->notify.fpCallback    = fpCallback;
        pBufPool->notify.pUserContext  = pUserContext;
        ret = RET_SUCCESS;
    }

    return ret;
}


/******************************************************************************
 * MediaBufPoolDeregisterCb
 *****************************************************************************/
RESULT MediaBufPoolDeregisterCb(MediaBufPool_t*         pBufPool,
                                MediaBufPoolCbNotify_t  fpCallback)
{
    RESULT ret = RET_NOTAVAILABLE;
    int32_t i;

    if(pBufPool == NULL)
    {
        return RET_WRONG_HANDLE;
    }

    if(fpCallback == NULL)
    {
        return RET_INVALID_PARM;
    }

    if(pBufPool->notify.fpCallback == fpCallback)
    {
        pBufPool->notify.fpCallback   = NULL;
        pBufPool->notify.pUserContext = NULL;
        ret = RET_SUCCESS;
    }

    return ret;
}



/******************************************************************************
 * MediaBufPoolGetBuffer
 *****************************************************************************/
MediaBuffer_t* MediaBufPoolGetBuffer(MediaBufPool_t* pBufPool)
{
    MediaBuffer_t *pMediaBuffer;

    DCT_ASSERT(pBufPool != NULL);

    for(;;)
    {
        /* if the requested number of buffers is greater than the maximum then wait*/
        while( pBufPool->freeBufNum == 0U )
        {
            return  NULL;
        }

        /* at least one buffer is free*/
        for(;;)
        {
            if(!pBufPool->pBufArray[pBufPool->index].lockCount)
            {
                /* adjust the resources count*/
                pBufPool->freeBufNum--;
                pBufPool->pBufArray[pBufPool->index].lockCount = 1;
                pBufPool->pBufArray[pBufPool->index].pOwner = pBufPool;
                pMediaBuffer = &pBufPool->pBufArray[pBufPool->index];

                if(++(pBufPool->index) >= pBufPool->bufNum)
                {
                    pBufPool->index = 0U;
                }

                return pMediaBuffer;
            }
            else
            {
                /* pBufPool->uiIndex points to the next buffer to be checked for a get empty request.*/
                /* If that buffer is locked, no media buffer is returned, because random access is*/
                /* not allowed in ringbuffer mode.*/
                if(pBufPool->flags & BUFPOOL_RINGBUFFER)
                {
                    return  NULL;
                }
            }
            if(++(pBufPool->index) >= pBufPool->bufNum)
            {
                pBufPool->index = 0U;
            }
        }
    } /* for( ;;)*/
}


/******************************************************************************
 * MediaBufPoolFreeBuffer
 *****************************************************************************/
void MediaBufPoolFreeBuffer(MediaBufPool_t* pBufPool, MediaBuffer_t *pBuf)
{
    int32_t i;

    DCT_ASSERT(pBufPool != NULL);
    DCT_ASSERT(pBuf != NULL);

    pBuf->lockCount = 0;
    pBufPool->freeBufNum++;

    if(pBufPool->notify.fpCallback != NULL)
    {
        (pBufPool->notify.fpCallback)(pBufPool->notify.pUserContext, pBuf);
    }
}

