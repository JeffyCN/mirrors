/*
 * gqueue.c: Queue
 *
 * Author:
 *   Duncan Mak (duncan@novell.com)
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
 *
 */
#include <stdio.h>

#include "queue.h"

void*
queuePopHead(GQueue* queue) {
  void* head = NULL;

  if (queue->length != 0) {
    GList* old_head;

    old_head = queue->head;
    head = old_head->data;
    queue->head = old_head->next;
    queue->length--;
    listFree1(old_head);
  }

  return head;
}

bool_t
queueIsEmpty(GQueue* queue) {
  if (!queue)
    return BOOL_TRUE;

  return queue->length == 0;
}

void
queuePushHead(GQueue* queue, void* head) {
  if (!queue)
    return;

  queue->head = listPrepend(queue->head, head);

  if (!queue->tail)
    queue->tail = queue->head;

  queue->length ++;
}

GQueue*
queueNew(void) {
  GQueue* queue = (GQueue*)calloc(1, sizeof(GQueue));
  queue->length = 0;

  return queue;
}

void
queueFree(GQueue* queue) {
  if (!queue)
    return;

  listFree(queue->head);
  free(queue);
}
