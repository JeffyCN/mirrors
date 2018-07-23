/******************************************************************************
 *
 * Copyright 2016, Fuzhou Rockchip Electronics Co.Ltd . All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Fuzhou Rockchip Electronics Co.Ltd .
 * 
 *
 *****************************************************************************/
/**
 * @file    oslayer_generic.c
 *
 * @brief   Encapsulates and abstracts services from different operating
 *          system, including user-mode as well as kernel-mode services.
 *
 *****************************************************************************/

#include "oslayer.h"

#ifdef OSLAYER_QUEUE
#include <stdlib.h>

#define OSLAYER_CROAK OSLAYER_ASSERT // use own definition because of wrong 'polarity' of OSLAYER_ASSERT

/******************************************************************************
 *  osQueueInit()
 ******************************************************************************
 *  @brief  Initialize queue object.
 *
 *  Init a queue. Item size and item number can be set.
 *
 *  @param  pQueue         Reference of the queue object.
 *
 *  @param  ItemNum        Number of items this queue can hold.
 *
 *  @param  ItemSize       Size of a single queue item.
 *
 *  @return                Status of operation.
 *  @retval OSLAYER_OK     Queue successfully created.
 *  @retval OSLAYER_ERROR  Queue is not created.
 *
 ******************************************************************************/
int32_t osQueueInit(osQueue* pQueue, int32_t ItemNum, int32_t ItemSize) {
  // check params
  OSLAYER_CROAK(pQueue == NULL);
  if ((ItemSize == 0) || (ItemNum == 0)) {
    return OSLAYER_INVALID_PARAM;
  }

  // clear queue object
  memset(pQueue, 0, sizeof(osQueue));

  // try to allocate item storage
  uint32_t ItemBufferSize = ItemSize * ItemNum;
  pQueue->pItemBuffer = malloc(ItemBufferSize);
  if (!pQueue->pItemBuffer) {
    return OSLAYER_ERROR;
  }

  // initialize queue buffer & remaining context
  memset(pQueue->pItemBuffer, 0, ItemBufferSize);

  pQueue->pItemBufferEnd   = pQueue->pItemBuffer + ItemBufferSize;
  pQueue->pItemBufferRead  = pQueue->pItemBuffer;
  pQueue->pItemBufferWrite = pQueue->pItemBuffer;

  pQueue->ItemSize  = ItemSize;
  pQueue->ItemNum   = ItemNum;
  pQueue->ItemCount = 0;

  if (OSLAYER_OK != osMutexInit(&pQueue->AccessMutex)) {
    free(pQueue->pItemBuffer);
    memset(pQueue, 0, sizeof(osQueue));
    return OSLAYER_ERROR;
  }

  if (OSLAYER_OK != osSemaphoreInit(&pQueue->ItemsFreeSema, ItemNum)) {
    osMutexDestroy(&pQueue->AccessMutex);
    free(pQueue->pItemBuffer);
    memset(pQueue, 0, sizeof(osQueue));
    return OSLAYER_ERROR;
  }

  if (OSLAYER_OK != osSemaphoreInit(&pQueue->ItemsUsedSema, 0)) {
    osMutexDestroy(&pQueue->AccessMutex);
    osSemaphoreDestroy(&pQueue->ItemsFreeSema);
    free(pQueue->pItemBuffer);
    memset(pQueue, 0, sizeof(osQueue));
    return OSLAYER_ERROR;
  }

  // queue is ready to be used
  return OSLAYER_OK;
}

/******************************************************************************
 *  osQueueReadInternal()
 ******************************************************************************
 *  @brief  Read an item from a queue.
 *
 *  Read an item from a queue. The item _must_ be available and 'reserved' by the caller.
 *
 *  @param  pQueue                    Reference of the queue object.
 *
 *  @param  pvItem                    Reference to item to read from queue.
 *
 *  @return                           Status of operation.
 *  @retval OSLAYER_OK                Reading from queue succeeded (ALWAYS).
 *
 ******************************************************************************/
static int32_t osQueueReadInternal(osQueue* pQueue, void* pvItem) {
  OSLAYER_STATUS osStatus;
  UNUSED_PARAM(osStatus);
  // check params
  OSLAYER_CROAK(pQueue == NULL);
  OSLAYER_CROAK(pvItem == NULL);

  // lock the queue
  osStatus = osMutexLock(&pQueue->AccessMutex);
  OSLAYER_CROAK(OSLAYER_OK != osStatus);

  // while holding the lock, we only do the math involved with reading an item to speed up things:
  // - just get a pointer to the item
  char* pItemRead = pQueue->pItemBufferRead;
  // - advance the read pointer to the next item
  pQueue->pItemBufferRead += pQueue->ItemSize;
  // -  handle wrap-around at end of buffer
  OSLAYER_CROAK(pQueue->pItemBufferRead > pQueue->pItemBufferEnd);
  if (pQueue->pItemBufferRead >= pQueue->pItemBufferEnd) {
    pQueue->pItemBufferRead = pQueue->pItemBuffer;
  }

  // now we can release the lock already
  osStatus = osMutexUnlock(&pQueue->AccessMutex);
  OSLAYER_CROAK(OSLAYER_OK != osStatus);

  // do the real work of copying the item from the queue
  memcpy(pvItem, pItemRead, pQueue->ItemSize);

  // finally free the item in the queue
  osStatus = osSemaphorePost(&pQueue->ItemsFreeSema);
  OSLAYER_CROAK(OSLAYER_OK != osStatus);

  return OSLAYER_OK;
}

/******************************************************************************
 *  osQueueRead()
 ******************************************************************************
 *  @brief  Blocking read from the queue.
 *
 *  Wait for item being available in the queue and block calling thread, then copy item
 *  from queue. The function call returns immediatly, if an item is already available.
 *
 *  @param  pQueue                    Reference of the queue object.
 *
 *  @param  pvItem                    Reference to item to read from queue.
 *
 *  @return                           Status of operation.
 *  @retval OSLAYER_OK                Reading from queue succeeded.
 *  @retval OSLAYER_ERROR             Tried to read from a not initialized queue.
 *  @retval OSLAYER_OPERATION_FAILED  Reading from queue failed.
 *
 ******************************************************************************/
int32_t osQueueRead(osQueue* pQueue, void* pvItem) {
  OSLAYER_STATUS osStatus;

  // check params
  OSLAYER_CROAK(pQueue == NULL);
  OSLAYER_CROAK(pvItem == NULL);

  // wait for used item available in queue
  osStatus = osSemaphoreWait(&pQueue->ItemsUsedSema);
  if (OSLAYER_OK != osStatus) {
    return osStatus;
  }

  // we're allowed to read from the queue now, one of the used items is 'reserved' for us
  osStatus = osQueueReadInternal(pQueue, pvItem);

  return osStatus;
}

/******************************************************************************
 *  osQueueTimedRead()
 ******************************************************************************
 *  @brief  Blocking read from the queue.
 *
 *  Wait for item being available in the queue and block calling thread, then copy item
 *  from queue. The function call returns immediatly, if an item is already available.
 *
 *  @param  pQueue                    Reference of the queue object.
 *
 *  @param  pvItem                    Reference to item to read from queue.
 *
 *  @return                           Status of operation.
 *  @retval OSLAYER_OK                Reading from queue succeeded.
 *  @retval OSLAYER_ERROR             Tried to read from a not initialized queue.
 *  @retval OSLAYER_TIMEOUT           Timeout occurred while waiting for item being
 *                                    available in the queue.
 *  @retval OSLAYER_OPERATION_FAILED  Reading from queue failed.
 *
 ******************************************************************************/
int32_t osQueueTimedRead(osQueue* pQueue, void* pvItem, uint32_t msec) {
  OSLAYER_STATUS osStatus;

  // check params
  OSLAYER_CROAK(pQueue == NULL);
  OSLAYER_CROAK(pvItem == NULL);

  // wait with timeout for used item available in queue
  osStatus = osSemaphoreTimedWait(&pQueue->ItemsUsedSema, msec);
  if (OSLAYER_OK != osStatus) {
    return osStatus;
  }

  // we're allowed to read from the queue now, one of the used items is 'reserved' for us
  osStatus = osQueueReadInternal(pQueue, pvItem);

  return osStatus;
}

/******************************************************************************
 *  osQueueTryRead()
 ******************************************************************************
 *  @brief  Non-blocking read from the queue.
 *
 *  If an item is available in the queue, then copy item from queue. If no
 *  item is available, then return with error.
 *
 *  @param  pQueue                    Reference of the queue object.
 *
 *  @param  pvItem                    Reference to item to read from queue.
 *
 *  @return                           Status of operation.
 *  @retval OSLAYER_OK                Reading from queue succeeded.
 *  @retval OSLAYER_ERROR             Tried to read from a not initialized queue.
 *  @retval OSLAYER_TIMEOUT           No item was available in the queue.
 *  @retval OSLAYER_OPERATION_FAILED  Reading from queue failed.
 *
 ******************************************************************************/
int32_t osQueueTryRead(osQueue* pQueue, void* pvItem) {
  OSLAYER_STATUS osStatus;

  // check params
  OSLAYER_CROAK(pQueue == NULL);
  OSLAYER_CROAK(pvItem == NULL);

  // wait with timeout for used item available in queue
  osStatus = osSemaphoreTryWait(&pQueue->ItemsUsedSema);
  if (OSLAYER_OK != osStatus) {
    return osStatus;
  }

  // we're allowed to read from the queue now, one of the used items is 'reserved' for us
  osStatus = osQueueReadInternal(pQueue, pvItem);

  return osStatus;
}


/******************************************************************************
 *  osQueueWriteInternal()
 ******************************************************************************
 *  @brief  Write an item into a queue.
 *
 *  Write an item into a queue. A free item _must_ be available and 'reserved' by the caller.
 *
 *  @param  pQueue                    Reference of the queue object.
 *
 *  @param  pvItem                    Reference to item to write into queue.
 *
 *  @return                           Status of operation.
 *  @retval OSLAYER_OK                Writing into queue succeeded (ALWAYS).
 *
 ******************************************************************************/
static int32_t osQueueWriteInternal(osQueue* pQueue, void* pvItem) {
  OSLAYER_STATUS osStatus;
  UNUSED_PARAM(osStatus);
  // check params
  OSLAYER_CROAK(pQueue == NULL);
  OSLAYER_CROAK(pvItem == NULL);

  // lock the queue
  osStatus = osMutexLock(&pQueue->AccessMutex);
  OSLAYER_CROAK(OSLAYER_OK != osStatus);

  // while holding the lock, we only do the math involved with writing an item to speed up things:
  // - just get a pointer to the item
  char* pItemWrite = pQueue->pItemBufferWrite;
  // - advance the write pointer to the next item
  pQueue->pItemBufferWrite += pQueue->ItemSize;
  // -  handle wrap-around at end of buffer
  OSLAYER_CROAK(pQueue->pItemBufferWrite > pQueue->pItemBufferEnd);
  if (pQueue->pItemBufferWrite >= pQueue->pItemBufferEnd) {
    pQueue->pItemBufferWrite = pQueue->pItemBuffer;
  }

  // now we can release the lock already
  osStatus = osMutexUnlock(&pQueue->AccessMutex);
  OSLAYER_CROAK(OSLAYER_OK != osStatus);

  // do the real work of copying the item into the queue
  memcpy(pItemWrite, pvItem, pQueue->ItemSize);

  // finally 'use' the item in the queue
  osStatus = osSemaphorePost(&pQueue->ItemsUsedSema);
  OSLAYER_CROAK(OSLAYER_OK != osStatus);

  return OSLAYER_OK;
}

/******************************************************************************
 *  osQueueWrite()
 ******************************************************************************
 *  @brief  Blocking write into the queue.
 *
 *  Wait for space in queue being available and block calling thread, then copy item
 *  into queue. The function call returns immediatly, if space is already available.
 *
 *  @param  pQueue                    Reference of the queue object.
 *
 *  @param  pvItem                    Reference to item to write into queue.
 *
 *  @return                           Status of operation
 *  @retval OSLAYER_OK                Writing into queue succeeded.
 *  @retval OSLAYER_ERROR             Tried to write into a not initialized queue.
 *  @retval OSLAYER_OPERATION_FAILED  Writing into queue failed.
 *
 ******************************************************************************/
int32_t osQueueWrite(osQueue* pQueue, void* pvItem) {
  OSLAYER_STATUS osStatus;

  // check params
  OSLAYER_CROAK(pQueue == NULL);
  OSLAYER_CROAK(pvItem == NULL);

  // wait for free item available in queue
  osStatus = osSemaphoreWait(&pQueue->ItemsFreeSema);
  if (OSLAYER_OK != osStatus) {
    return osStatus;
  }

  // we're allowed to write into the queue now, one of the free items is 'reserved' for us
  osStatus = osQueueWriteInternal(pQueue, pvItem);

  return osStatus;
}

/******************************************************************************
 *  osQueueTimedWrite()
 ******************************************************************************
 *  @brief  Blocking write into the queue with timeout.
 *
 *  Wait for space in queue being available and block calling thread, then copy item
 *  into queue. The function call returns immediatly, if space is already available.
 *
 *  @param  pQueue                    Reference of the queue object.
 *
 *  @param  pvItem                    Reference to item to write into queue.
 *
 *  @param  msec                      Timeout value in milliseconds.
 *
 *  @return                           Status of operation
 *  @retval OSLAYER_OK                Writing into queue succeeded.
 *  @retval OSLAYER_ERROR             Tried to write into a not initialized queue.
 *  @retval OSLAYER_TIMEOUT           Timeout occurred while waiting for space being
 *                                    available in the queue.
 *  @retval OSLAYER_OPERATION_FAILED  Writing into queue failed.
 *
 ******************************************************************************/
int32_t osQueueTimedWrite(osQueue* pQueue, void* pvItem, uint32_t msec) {
  OSLAYER_STATUS osStatus;

  // check params
  OSLAYER_CROAK(pQueue == NULL);
  OSLAYER_CROAK(pvItem == NULL);

  // wait with timeout for free item available in queue
  osStatus = osSemaphoreTimedWait(&pQueue->ItemsFreeSema, msec);
  if (OSLAYER_OK != osStatus) {
    return osStatus;
  }

  // we're allowed to write into the queue now, one of the free items is 'reserved' for us
  osStatus = osQueueWriteInternal(pQueue, pvItem);

  return osStatus;
}

/******************************************************************************
 *  osQueueTryWrite()
 ******************************************************************************
 *  @brief  Non-blocking write into the queue.
 *
 *  If a free item is available in the queue, then copy item into queue. If no
 *  free item is available, then return with error.
 *
 *  @param  pQueue                    Reference of the queue object.
 *
 *  @param  pvItem                    Reference to item to read from queue.
 *
 *  @return                           Status of operation.
 *  @retval OSLAYER_OK                Writing into queue succeeded.
 *  @retval OSLAYER_ERROR             Tried to write into a not initialized queue.
 *  @retval OSLAYER_TIMEOUT           No space was available in the queue.
 *  @retval OSLAYER_OPERATION_FAILED  Writing into queue failed.
 *
 ******************************************************************************/
int32_t osQueueTryWrite(osQueue* pQueue, void* pvItem) {
  OSLAYER_STATUS osStatus;

  // check params
  OSLAYER_CROAK(pQueue == NULL);
  OSLAYER_CROAK(pvItem == NULL);

  // wait with timeout for free item available in queue
  osStatus = osSemaphoreTryWait(&pQueue->ItemsFreeSema);
  if (OSLAYER_OK != osStatus) {
    return osStatus;
  }

  // we're allowed to write into the queue now, one of the free items is 'reserved' for us
  osStatus = osQueueWriteInternal(pQueue, pvItem);

  return osStatus;
}

/******************************************************************************
 *  osQueueDestroy()
 ******************************************************************************
 *  @brief  Destroy the queue.
 *
 *  Destroy the queue and free resources associated with queue object.
 *
 *  @param  pQueue      Reference of the queue object
 *
 *  @return             always OSLAYER_OK
 ******************************************************************************/
int32_t osQueueDestroy(osQueue* pQueue) {
  // check pointer
  OSLAYER_CROAK(pQueue == NULL);

  osMutexDestroy(&pQueue->AccessMutex);
  osSemaphoreDestroy(&pQueue->ItemsFreeSema);
  osSemaphoreDestroy(&pQueue->ItemsUsedSema);
  free(pQueue->pItemBuffer);
  memset(pQueue, 0, sizeof(osQueue));

  return OSLAYER_OK;
}

#endif /* OSLAYER_QUEUE */
