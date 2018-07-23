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
 * @file slist.h
 *
 * @brief
 *   Extended data types: Linked list
 *
 *****************************************************************************/
/**
 * @defgroup module_ext_slist Linked List
 *
 * @brief This module implements a "Single Linked List".
 *
 * @{
 *
 *****************************************************************************/
#ifndef __SLIST_H__
#define __SLIST_H__

#include "types.h"
#include "ext_types.h"


typedef struct _GSList GSList;

/**
 * @brief Structure that represents an element in the list.
 */
struct _GSList {
  void* data;     /**< Pointer to object using the list for */
  GSList* next;   /**< Pointer to next element in list */
};


#define slistNext(slist) ((slist) ? (((GSList *) (slist))->next) : NULL)


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
GSList* slistAlloc(void);


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
GSList* slistAppend(GSList* list, void* data);


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
GSList* slistPrepend(GSList* list, void* data);


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
void slistFree(GSList* list);


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
void slistFree1(GSList* list);


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
GSList* slistCopy(GSList* list);


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
GSList* slistConcat(GSList* list1, GSList* list2);


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
void slistForeach(GSList* list, GFunc func, void* user_data);


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
GSList* slistLast(GSList* list);


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
GSList* slistFind(GSList* list, const void*  data);


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
GSList* slistFindCustom(GSList* list, const void*  data, GCompareFunc func);


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
GSList* slistRemove(GSList* list, const void*  data);


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
GSList* slistRemoveAll(GSList* list, const void*  data);


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
GSList* slistReverse(GSList* list);


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
uint32_t slistLength(GSList* list);


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
GSList* slistRemoveLink(GSList* list, GSList* link);


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
GSList* slistDeleteLink(GSList* list, GSList* link);


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
GSList* slistInsertSorted(GSList* list, void* data, GCompareFunc func);


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
GSList* slistInsertBefore(GSList* list, GSList* sibling, void* data);


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
GSList* slistSort(GSList* list, GCompareFunc func);


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
int32_t slistIndex(GSList* list, const void* data);


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
GSList* slistNth(GSList* list, uint32_t n);


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
void* slistNthData(GSList* list, uint32_t n);

/* @} module_ext_slist */

#endif /* __SLIST_H__ */
