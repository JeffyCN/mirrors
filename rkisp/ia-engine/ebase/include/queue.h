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
 * @file queue.h
 *
 * @brief
 *   Extended data types: Queue
 *
 *****************************************************************************/
/**
 * @defgroup module_ext_queue Queue
 *
 * @{
 *
 *****************************************************************************/
#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "types.h"
#include "ext_types.h"
#include "list.h"


/**
 * @brief Structure that represents an element in the queue.
 */
typedef struct {
  GList* head;        /**< Head element of queue */
  GList* tail;        /**< Tail  element of queue  */
  uint32_t length;    /**< Length  of queue  */
} GQueue;


/*****************************************************************************/
/**
 * @brief
 *
 * @param
 *
 * @return
 * @retval
 *
 *****************************************************************************/
void* queuePopHead(GQueue* queue);


/*****************************************************************************/
/**
 * @brief
 *
 * @param
 *
 * @return
 * @retval
 *
 *****************************************************************************/
void queuePushHead(GQueue* queue, void* data);

/*****************************************************************************/
/**
 * @brief
 *
 * @param
 *
 * @return
 * @retval
 *
 *****************************************************************************/
bool_t queueIsEmpty(GQueue* queue);

/*****************************************************************************/
/**
 * @brief
 *
 * @param
 *
 * @return
 * @retval
 *
 *****************************************************************************/
GQueue* queueNew(void);

/*****************************************************************************/
/**
 * @brief
 *
 * @param
 *
 * @return
 * @retval
 *
 *****************************************************************************/
void queueFree(GQueue* queue);

/* @} module_ext_queue */

#endif /* __QUEUE_H__ */
