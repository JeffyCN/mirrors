/*****************************************************************************/
/*!
 *  @file        list.h
 *  @version     1.0
 *  @author      Ulrich Marx
 *
 *  @brief       implementation of a single linked list
 *
 *  @note        (the tag is omitted if there are no notes) \n
 */
/*  This is an unpublished work, the copyright in which vests in Silicon Image
 *  GmbH. The information contained herein is the property of Silicon Image GmbH
 *  and is supplied without liability for errors or omissions. No part may be
 *  reproduced or used expect as authorized by contract or other written
 *  permission. Copyright(c) Silicon Image GmbH, 2009, all rights reserved.
 */

/**
 * @file list.h
 *
 * <pre>
 *
 *   Principal Author: Ulrich Marx <ulrich.marx@siliconimage.com>
 *   Company: Silicon Image
 *
 *   Programming Language: C
 *   Date:    Tue 22 Apr 2008 07:58:35 PM CEST
 *   Designed for any OS (conformable to ANSI)
 *
 *   Description:
 *
 *
 * </pre>
 *
 ******************************************************************************/
#ifndef __LIST_H__
#define __LIST_H__

#include <ebase/dct_assert.h>      /* for DCT_ASSERT */

#define ASSERT  DCT_ASSERT

/******************************************************************************/
/**
 * @brief  Implementation of single linked list
 *
 ******************************************************************************/
typedef struct _List {
  struct _List* p_next;
} List;


/******************************************************************************/
/**
 * @brief   Prepare a list item.
 *
 * @note    This function prepares a list item so that it can be added to a list.
 *
 * @param   p_item  The list item to be initialized.
 *
 ******************************************************************************/
static inline void ListPrepareItem(void* p_item) {
  ASSERT(p_item != NULL);
  ((List*)p_item)->p_next = NULL;
}



/******************************************************************************/
/**
 * @brief   Initialize a list.
 *
 * @note    This function initalizes a list. The list will be empty after this
 *          function has been called.
 *
 * @param   p_list  The list to be initialized.
 *
 ******************************************************************************/
static inline void ListInit(List* p_list) {
  ASSERT(p_list != NULL);
  p_list->p_next = NULL;
}



/******************************************************************************/
/**
 * @brief   Get a pointer to the first element of a list.
 *
 * @note    This function returns a pointer to the first element of the list.
 *          The element will \b not be removed from the list.
 *
 * @param   p_list  The list.
 *
 * @return  A pointer to the first element on the list.
 *
 ******************************************************************************/
static inline List* ListHead(const List* p_list) {
  List* p_head;

  ASSERT(p_list != NULL);

  p_head = p_list->p_next;

  return (p_head);
}



/******************************************************************************/
/**
 * @brief   Get the tail of a list.
 *
 * @note    This function returns a pointer to the elements following the first
 *          element of a list. No elements are removed by this function.
 *
 * @param   p_list  The list
 *
 * @return  A pointer to the element after the first element on the list.
 *
 ******************************************************************************/
static inline List* ListTail(List* p_list) {
  List* l;

  ASSERT(p_list != NULL);

  if (p_list->p_next == NULL) {
    return (NULL);
  }

  for (l = p_list; l->p_next != NULL; l = l->p_next) {
    /* make lint happy */
  }

  return (l);
}



/******************************************************************************/
/**
 * @brief  Tests wheter a list is empty.
 *
 * @param  p_list The list
 *
 ******************************************************************************/
static inline int ListEmpty(const List* p_list) {
  ASSERT(p_list != NULL);

  return (((p_list->p_next == NULL) ? 1 : 0));
}



/******************************************************************************/
/**
 * @brief  Returns No of Elemtens
 *
 * @param  p_list The list
 *
 ******************************************************************************/
static inline int ListNoItems(const List* p_list) {
  int cnt = 0;
  List* l;

  ASSERT(p_list != NULL);

  l = ListHead(p_list);
  while (l != NULL) {
    ++cnt;
    l = l->p_next;
  }

  return (cnt);
}



/******************************************************************************/
/**
 * @brief   Add an item at the end of a list.
 *
 * @note    This function adds an item to the end of the list.
 *
 * @param   p_list  The list.
 * @param   p_item  A pointer to the item to be added.
 *
 ******************************************************************************/
static inline void ListAddTail(List* p_list, void* p_item) {
  List* l;

  ASSERT(p_list != NULL);
  ASSERT(p_item != NULL);

  ((List*)p_item)->p_next = NULL;
  l = ListTail(p_list);
  if (l == NULL) {
    p_list->p_next = (List*)p_item;
  } else {
    l->p_next = (List*)p_item;
  }
}



/******************************************************************************/
/**
 * @brief   Get the first object on a list.
 *
 * @note    This function removes the first object on the list and returns a
 *          pointer to the list.
 *
 * @param   p_list  The list.
 *
 * @return  The removed head item of the list.
 *
 ******************************************************************************/
static inline List* ListGetHead(List* p_list) {
  List* l;

  ASSERT(p_list != NULL);

  if (p_list->p_next != NULL) {
    l = p_list->p_next;
    return (l);
  }

  return (NULL);
}



/******************************************************************************/
/**
 * @brief   Get the first object on a list.
 *
 * @note    This function removes the first object on the list and returns a
 *          pointer to the list.
 *
 * @param   p_list  The list.
 *
 * @return  The removed head item of the list.
 *
 ******************************************************************************/
static inline List* ListGetItemByIdx(List* p_list, const int idx) {
  List* l;
  int cnt = 0;

  ASSERT(p_list != NULL);

  l = ListHead(p_list);
  while ((l != NULL) && (cnt < idx)) {
    ++cnt;
    l = l->p_next;
  }

  return (l);
}



/******************************************************************************/
/**
 * @brief   Remove the first object on a list.
 *
 * @note    This function removes the first object on the list and returns a
 *          pointer to the list.
 *
 * @param   p_list  The list.
 *
 * @return  The removed head item of the list.
 *
 ******************************************************************************/
static inline List* ListRemoveHead(List* p_list) {
  List* l;

  ASSERT(p_list != NULL);

  if (p_list->p_next != NULL) {
    l = p_list->p_next;
    p_list->p_next = l->p_next;
    return (l);
  }

  return (NULL);
}



/******************************************************************************/
/**
 * @brief   Search an element in list
 *
 * @note    This function searches the first element which matches to the
 *          given search-function
 *
 * @param   p_list  The list.
 *          func    The search function out side this file
 *          key     The key that search function matches
 *
 * @return  The first element
 * @retval  NULL    no element matches
 *
 ******************************************************************************/
typedef int (*pSearchFunc)(List*, void* key);

static inline List* ListSearch(List* p_list, pSearchFunc func, void* key) {
  List* l;

  ASSERT(p_list != NULL);
  ASSERT(func != NULL);

  l = ListHead(p_list);
  while (l) {
    if (func(l, key)) {
      return (l);
    }
    l = l->p_next;
  }

  return (NULL);
}



/******************************************************************************/
/**
 * @brief   Search an element in list
 *
 * @note    This function searches the first element which matches to the
 *          given search-function
 *
 * @param   p_list  The list.
 *          func    The search function out side this file
 *          key     The key that search function matches
 *
 * @return  The first element
 * @retval  NULL    no element matches
 *
 ******************************************************************************/
typedef void (*pForEachFunc)(List*, void*);

static inline List* ListForEach(List* p_list, pForEachFunc func, void* param) {
  List* l;

  ASSERT(p_list != NULL);
  ASSERT(func != NULL);

  l = ListHead(p_list);
  while (l) {
    func(l, param);
    l = l->p_next;
  }

  return (NULL);
}



/******************************************************************************/
/**
 * @brief   Get the Index of an Item
 *
 * @note    This function removes the first object on the list and returns a
 *          pointer to the list.
 *
 * @param   p_list  The list.
 *
 * @return  The removed head item of the list.
 *
 ******************************************************************************/
static inline int ListGetIdxByItem(List* p_list, pSearchFunc func, void* key) {
  List* l;
  int cnt = 0;

  ASSERT(p_list != NULL);

  l = ListHead(p_list);
  while (l) {
    if (func(l, key)) {
      return (cnt);
    }

    ++cnt;
    l = l->p_next;
  }

  return (-1);
}




/******************************************************************************/
/**
 * @brief   Search an element in list
 *
 * @note    This function searches the first element which matches to the
 *          given search-function
 *
 * @param   p_list  The list.
 *          func    The search function out side this file
 *          key     The key that search function matches
 *
 * @return  The first element
 * @retval  NULL    no element matches
 *
 ******************************************************************************/
static inline List* ListRemoveItem(List* p_list, pSearchFunc func, void* key) {
  List* l, *pre_l;

  ASSERT(p_list != NULL);

  pre_l = p_list;
  l = ListHead(p_list);
  while (l) {
    if (func(l, key)) {
      /* isolate l from list */
      pre_l->p_next = l->p_next;
      l->p_next = NULL;
      return (l);
    }
    pre_l = l;
    l = l->p_next;
  }

  return (NULL);
}


#endif /* __LIST_H__ */
