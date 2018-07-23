/*
 * gslist.c: Singly-linked list implementation
 *
 * Authors:
 *   Duncan Mak (duncan@novell.com)
 *   Raja R Harinath (rharinath@novell.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * (C) 2006 Novell, Inc.
 */
#include <stdio.h>

#include "slist.h"

GSList*
slistAlloc(void) {
  return (GSList*)calloc(1, sizeof(GSList));
}

void
slistFree1(GSList* list) {
  free(list);
}

GSList*
slistAppend(GSList* list, void* data) {
  return slistConcat(list, slistPrepend(NULL, data));
}

/* This is also a list node constructor. */
GSList*
slistPrepend(GSList* list, void* data) {
  GSList* head = slistAlloc();
  head->data = data;
  head->next = list;

  return head;
}

/*
 * Insert the given data in a new node after the current node.
 * Return new node.
 */
static inline GSList*
insert_after(GSList* list, void* data) {
  list->next = slistPrepend(list->next, data);
  return list->next;
}

/*
 * Return the node prior to the node containing 'data'.
 * If the list is empty, or the first node contains 'data', return NULL.
 * If no node contains 'data', return the last node.
 */
static inline GSList*
find_prev(GSList* list, const void* data) {
  GSList* prev = NULL;
  while (list) {
    if (list->data == data)
      break;
    prev = list;
    list = list->next;
  }
  return prev;
}

/* like 'find_prev', but searches for node 'link' */
static inline GSList*
find_prev_link(GSList* list, GSList* link) {
  GSList* prev = NULL;
  while (list) {
    if (list == link)
      break;
    prev = list;
    list = list->next;
  }
  return prev;
}

GSList*
slistInsertBefore(GSList* list, GSList* sibling, void* data) {
  GSList* prev = find_prev_link(list, sibling);

  if (!prev)
    return slistPrepend(list, data);

  insert_after(prev, data);
  return list;
}

void
slistFree(GSList* list) {
  while (list) {
    GSList* next = list->next;
    slistFree1(list);
    list = next;
  }
}

GSList*
slistCopy(GSList* list) {
  GSList* copy, *tmp;

  if (!list)
    return NULL;

  copy = slistPrepend(NULL, list->data);
  tmp = copy;

  for (list = list->next; list; list = list->next)
    tmp = insert_after(tmp, list->data);

  return copy;
}

GSList*
slistConcat(GSList* list1, GSList* list2) {
  if (!list1)
    return list2;

  slistLast(list1)->next = list2;
  return list1;
}

void
slistForeach(GSList* list, GFunc func, void* user_data) {
  while (list) {
    (*func)(list->data, user_data);
    list = list->next;
  }
}

GSList*
slistLast(GSList* list) {
  if (!list)
    return NULL;

  while (list->next)
    list = list->next;

  return list;
}

GSList*
slistFind(GSList* list, const void* data) {
  for (; list; list = list->next)
    if (list->data == data)
      break;
  return list;
}

GSList*
slistFindCustom(GSList* list, const void* data, GCompareFunc func) {
  if (!func)
    return NULL;

  while (list) {
    if (func(list->data, data) == 0)
      return list;

    list = list->next;
  }

  return NULL;
}

uint32_t
slistLength(GSList* list) {
  uint32_t length = 0;

  while (list) {
    length ++;
    list = list->next;
  }

  return length;
}

GSList*
slistRemove(GSList* list, const void* data) {
  GSList* prev = find_prev(list, data);
  GSList* current = prev ? prev->next : list;

  if (current) {
    if (prev)
      prev->next = current->next;
    else
      list = current->next;
    slistFree1(current);
  }

  return list;
}

GSList*
slistRemoveAll(GSList* list, const void* data) {
  GSList* next = list;
  GSList* prev = NULL;
  GSList* current;

  while (next) {
    GSList* tmp_prev = find_prev(next, data);
    if (tmp_prev)
      prev = tmp_prev;
    current = prev ? prev->next : list;

    if (!current)
      break;

    next = current->next;

    if (prev)
      prev->next = next;
    else
      list = next;
    slistFree1(current);
  }

  return list;
}

GSList*
slistRemoveLink(GSList* list, GSList* link) {
  GSList* prev = find_prev_link(list, link);
  GSList* current = prev ? prev->next : list;

  if (current) {
    if (prev)
      prev->next = current->next;
    else
      list = current->next;
    current->next = NULL;
  }

  return list;
}

GSList*
slistDeleteLink(GSList* list, GSList* link) {
  list = slistRemoveLink(list, link);
  slistFree1(link);

  return list;
}

GSList*
slistReverse(GSList* list) {
  GSList* prev = NULL;
  while (list) {
    GSList* next = list->next;
    list->next = prev;
    prev = list;
    list = next;
  }

  return prev;
}

GSList*
slistInsertSorted(GSList* list, void* data, GCompareFunc func) {
  GSList* prev = NULL;

  if (!func)
    return list;

  if (!list || func(list->data, data) > 0)
    return slistPrepend(list, data);

  /* Invariant: func (prev->data, data) <= 0) */
  for (prev = list; prev->next; prev = prev->next)
    if (func(prev->next->data, data) > 0)
      break;

  /* ... && (prev->next == 0 || func (prev->next->data, data) > 0)) */
  insert_after(prev, data);
  return list;
}

int32_t
slistIndex(GSList* list, const void* data) {
  int32_t index = 0;

  while (list) {
    if (list->data == data)
      return index;

    index++;
    list = list->next;
  }

  return -1;
}

GSList*
slistNth(GSList* list, uint32_t n) {
  for (; list; list = list->next) {
    if (n == 0)
      break;
    n--;
  }
  return list;
}

void*
slistNthData(GSList* list, uint32_t n) {
  GSList* node = slistNth(list, n);
  return node ? node->data : NULL;
}

typedef GSList list_node;
#include "sort.frag.h"

GSList*
slistSort(GSList* list, GCompareFunc func) {
  if (!list || !list->next)
    return list;
  return do_sort(list, func);
}
