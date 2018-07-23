/*
 * glist.c: Doubly-linked list implementation
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

#include "list.h"

GList*
listAlloc() {
  return (GList*)calloc(1, sizeof(GList));
}

static inline GList*
new_node(GList* prev, void* data, GList* next) {
  GList* node = listAlloc();
  node->data = data;
  node->prev = prev;
  node->next = next;
  if (prev)
    prev->next = node;
  if (next)
    next->prev = node;
  return node;
}

static inline GList*
disconnect_node(GList* node) {
  if (node->next)
    node->next->prev = node->prev;
  if (node->prev)
    node->prev->next = node->next;
  return node;
}

GList*
listPrepend(GList* list, void* data) {
  return new_node(list ? list->prev : NULL, data, list);
}

void
listFree1(GList* list) {
  free(list);
}

void
listFree(GList* list) {
  while (list) {
    GList* next = list->next;
    listFree1(list);
    list = next;
  }
}

GList*
listAppend(GList* list, void* data) {
  GList* node = new_node(listLast(list), data, NULL);
  return list ? list : node;
}

GList*
listConcat(GList* list1, GList* list2) {
  if (list1 && list2) {
    list2->prev = listLast(list1);
    list2->prev->next = list2;
  }
  return list1 ? list1 : list2;
}

uint32_t
listLength(GList* list) {
  uint32_t length = 0;

  while (list) {
    length ++;
    list = list->next;
  }

  return length;
}

GList*
listRemove(GList* list, const void* data) {
  GList* current = listFind(list, data);
  if (!current)
    return list;

  if (current == list)
    list = list->next;
  listFree1(disconnect_node(current));

  return list;
}

GList*
listRemoveLink(GList* list, GList* link) {
  if (list == link)
    list = list->next;

  disconnect_node(link);
  link->next = NULL;
  link->prev = NULL;

  return list;
}

GList*
listDeleteLink(GList* list, GList* link) {
  list = listRemoveLink(list, link);
  listFree1(link);

  return list;
}

GList*
listFind(GList* list, const void* data) {
  while (list) {
    if (list->data == data)
      return list;

    list = list->next;
  }

  return NULL;
}

GList*
listFindCustom(GList* list, const void* data, GCompareFunc func) {
  if (!func)
    return NULL;

  while (list) {
    if (func(list->data, data) == 0)
      return list;

    list = list->next;
  }

  return NULL;
}

GList*
listReverse(GList* list) {
  GList* reverse = NULL;

  while (list) {
    reverse = list;
    list = reverse->next;

    reverse->next = reverse->prev;
    reverse->prev = list;
  }

  return reverse;
}

GList*
listFirst(GList* list) {
  if (!list)
    return NULL;

  while (list->prev)
    list = list->prev;

  return list;
}

GList*
listLast(GList* list) {
  if (!list)
    return NULL;

  while (list->next)
    list = list->next;

  return list;
}

GList*
listInsertSorted(GList* list, void* data, GCompareFunc func) {
  GList* prev = NULL;
  GList* current;
  GList* node;

  if (!func)
    return list;

  /* Invariant: !prev || func (prev->data, data) <= 0) */
  for (current = list; current; current = current->next) {
    if (func(current->data, data) > 0)
      break;
    prev = current;
  }

  node = new_node(prev, data, current);
  return list == current ? node : list;
}

GList*
listInsertBefore(GList* list, GList* sibling, void* data) {
  if (sibling) {
    GList* node = new_node(sibling->prev, data, sibling);
    return list == sibling ? node : list;
  }
  return listAppend(list, data);
}

void
listForeach(GList* list, GFunc func, void* user_data) {
  while (list) {
    (*func)(list->data, user_data);
    list = list->next;
  }
}

int32_t
listIndex(GList* list, const void* data) {
  int32_t index = 0;

  while (list) {
    if (list->data == data)
      return index;

    index ++;
    list = list->next;
  }

  return -1;
}

GList*
listNth(GList* list, uint32_t n) {
  for (; list; list = list->next) {
    if (n == 0)
      break;
    n--;
  }
  return list;
}

void*
listNthData(GList* list, uint32_t n) {
  GList* node = listNth(list, n);
  return node ? node->data : NULL;
}

GList*
listCopy(GList* list) {
  GList* copy = NULL;

  if (list) {
    GList* tmp = new_node(NULL, list->data, NULL);
    copy = tmp;

    for (list = list->next; list; list = list->next)
      tmp = new_node(tmp, list->data, NULL);
  }

  return copy;
}

typedef GList list_node;
#include "sort.frag.h"

GList*
listSort(GList* list, GCompareFunc func) {
  GList* current;
  if (!list || !list->next)
    return list;
  list = do_sort(list, func);

  /* Fixup: do_sort doesn't update 'prev' pointers */
  list->prev = NULL;
  for (current = list; current->next; current = current->next)
    current->next->prev = current;

  return list;
}
