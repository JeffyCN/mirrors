/*
 * ghashtable.c: Hashtable implementation
 *
 * Author:
 *   Miguel de Icaza (miguel@novell.com)
 *
 * (C) 2006 Novell, Inc.
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
 */
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "dct_assert.h"
#include "hashtable.h"

typedef struct _Slot Slot;

struct _Slot {
  void* key;
  void* value;
  Slot*    next;
};

static void* KEYMARKER_REMOVED = &KEYMARKER_REMOVED;

struct _GHashTable {
  GHashFunc      hash_func;
  GEqualFunc     key_equal_func;

  Slot** table;
  int32_t   table_size;
  int32_t   in_use;
  int32_t   threshold;
  int32_t   last_rehash;
  GDestroyNotify value_destroy_func, key_destroy_func;
};

static const uint32_t prime_tbl[] = {
  11, 19, 37, 73, 109, 163, 251, 367, 557, 823, 1237,
  1861, 2777, 4177, 6247, 9371, 14057, 21089, 31627,
  47431, 71143, 106721, 160073, 240101, 360163,
  540217, 810343, 1215497, 1823231, 2734867, 4102283,
  6153409, 9230113, 13845163
};

static bool_t
test_prime(int32_t x) {
  if ((x & 1) != 0) {
    int32_t n;
    for (n = 3; n < (int32_t)sqrt(x); n += 2) {
      if ((x % n) == 0)
        return BOOL_FALSE;
    }
    return BOOL_TRUE;
  }
  // There is only one even prime - 2.
  return (x == 2);
}

static int32_t
calc_prime(int32_t x) {
  int32_t i;

  for (i = (x & (~1)) - 1; i < INT32_MAX; i += 2) {
    if (test_prime(i))
      return i;
  }
  return x;
}

uint32_t
spacedPrimesClosest(uint32_t x) {
  int32_t i;

  for (i = 0; i < (int32_t)N_ELEMENTS(prime_tbl); i++) {
    if (x <= prime_tbl [i])
      return prime_tbl [i];
  }
  return calc_prime(x);
}

GHashTable*
hashTableNew(GHashFunc hash_func, GEqualFunc key_equal_func) {
  GHashTable* hash;

  if (hash_func == NULL)
    hash_func = directHash;
  if (key_equal_func == NULL)
    key_equal_func = directEqual;

  hash = (GHashTable*)calloc(1, sizeof(GHashTable));

  hash->hash_func = hash_func;
  hash->key_equal_func = key_equal_func;

  hash->table_size = spacedPrimesClosest(1);

  hash->table = (Slot**)calloc(hash->table_size, sizeof(Slot*));
  hash->last_rehash = hash->table_size;

  return hash;
}

GHashTable*
hashTableNewFull(GHashFunc hash_func, GEqualFunc key_equal_func,
                 GDestroyNotify key_destroy_func, GDestroyNotify value_destroy_func) {
  GHashTable* hash = hashTableNew(hash_func, key_equal_func);
  if (hash == NULL)
    return NULL;

  hash->key_destroy_func = key_destroy_func;
  hash->value_destroy_func = value_destroy_func;

  return hash;
}

static void
do_rehash(GHashTable* hash) {
  int32_t current_size, i;
  Slot** table;

  /* printf ("Resizing diff=%d slots=%d\n", hash->in_use - hash->last_rehash, hash->table_size); */
  hash->last_rehash = hash->table_size;
  current_size = hash->table_size;
  hash->table_size = spacedPrimesClosest(hash->in_use);
  /* printf ("New size: %d\n", hash->table_size); */
  table = hash->table;
  hash->table = calloc(hash->table_size, sizeof(Slot*));

  for (i = 0; i < current_size; i++) {
    Slot* s, *next;

    for (s = table [i]; s != NULL; s = next) {
      uint32_t hashcode = ((*hash->hash_func)(s->key)) % hash->table_size;
      next = s->next;

      s->next = hash->table [hashcode];
      hash->table [hashcode] = s;
    }
  }
  free(table);
}

static void
rehash(GHashTable* hash) {
  int32_t diff = ABS(hash->last_rehash - hash->in_use);

  /* These are the factors to play with to change the rehashing strategy */
  /* I played with them with a large range, and could not really get */
  /* something that was too good, maybe the tests are not that great */
  if (!(diff * 0.75 > hash->table_size * 2))
    return;
  do_rehash(hash);
}

void
hashTableInsertReplace(GHashTable* hash, void* key, void* value, bool_t replace) {
  uint32_t hashcode;
  Slot* s;
  GEqualFunc equal;

  DCT_ASSERT(hash != NULL);

  equal = hash->key_equal_func;
  if (hash->in_use >= hash->threshold)
    rehash(hash);

  hashcode = ((*hash->hash_func)(key)) % hash->table_size;
  for (s = hash->table [hashcode]; s != NULL; s = s->next) {
    if ((*equal)(s->key, key)) {
      if (replace) {
        if (hash->key_destroy_func != NULL)
          (*hash->key_destroy_func)(s->key);
        s->key = key;
      }
      if (hash->value_destroy_func != NULL)
        (*hash->value_destroy_func)(s->value);
      s->value = value;
      return;
    }
  }

  s = (Slot*)malloc(sizeof(Slot));
  s->key = key;
  s->value = value;
  s->next = hash->table [hashcode];
  hash->table [hashcode] = s;
  hash->in_use++;
}

uint32_t
hashTableSize(GHashTable* hash) {
  DCT_ASSERT(hash != NULL);

  return hash->in_use;
}

void*
hashTableLookup(GHashTable* hash, const void* key) {
  void* orig_key;
  void* value;

  if (hashTableLookupExtended(hash, key, &orig_key, &value))
    return value;
  else
    return NULL;
}

bool_t
hashTableLookupExtended(GHashTable* hash, const void* key, void * *orig_key, void * *value) {
  GEqualFunc equal;
  Slot* s;
  uint32_t hashcode;

  DCT_ASSERT(hash != NULL);

  equal = hash->key_equal_func;

  hashcode = ((*hash->hash_func)(key)) % hash->table_size;

  for (s = hash->table [hashcode]; s != NULL; s = s->next) {
    if ((*equal)(s->key, key)) {
      *orig_key = s->key;
      *value = s->value;
      return BOOL_TRUE;
    }
  }
  return BOOL_FALSE;
}

void
hashTableForeach(GHashTable* hash, GHFunc func, void* user_data) {
  int32_t i;

  DCT_ASSERT(hash != NULL);
  DCT_ASSERT(func != NULL);

  for (i = 0; i < hash->table_size; i++) {
    Slot* s;

    for (s = hash->table [i]; s != NULL; s = s->next)
      (*func)(s->key, s->value, user_data);
  }
}

void*
hashTableFind(GHashTable* hash, GHRFunc predicate, void* user_data) {
  int32_t i;

  DCT_ASSERT(hash != NULL);
  DCT_ASSERT(predicate != NULL);

  for (i = 0; i < hash->table_size; i++) {
    Slot* s;

    for (s = hash->table [i]; s != NULL; s = s->next)
      if ((*predicate)(s->key, s->value, user_data))
        return s->value;
  }
  return NULL;
}

bool_t
hashTableRemove(GHashTable* hash, const void* key) {
  GEqualFunc equal;
  Slot* s, *last;
  uint32_t hashcode;

  DCT_ASSERT(hash != NULL);
  equal = hash->key_equal_func;

  hashcode = ((*hash->hash_func)(key)) % hash->table_size;
  last = NULL;
  for (s = hash->table [hashcode]; s != NULL; s = s->next) {
    if ((*equal)(s->key, key)) {
      if (hash->key_destroy_func != NULL)
        (*hash->key_destroy_func)(s->key);
      if (hash->value_destroy_func != NULL)
        (*hash->value_destroy_func)(s->value);
      if (last == NULL)
        hash->table [hashcode] = s->next;
      else
        last->next = s->next;
      free(s);
      hash->in_use--;
      return BOOL_TRUE;
    }
    last = s;
  }
  return BOOL_FALSE;
}

uint32_t
hashTableForeachRemove(GHashTable* hash, GHRFunc func, void* user_data) {
  int32_t i;
  int32_t count = 0;

  DCT_ASSERT(hash != NULL);
  DCT_ASSERT(func != NULL);

  for (i = 0; i < hash->table_size; i++) {
    Slot* s, *last;

    last = NULL;
    for (s = hash->table [i]; s != NULL;) {
      if ((*func)(s->key, s->value, user_data)) {
        Slot* n;

        if (hash->key_destroy_func != NULL)
          (*hash->key_destroy_func)(s->key);
        if (hash->value_destroy_func != NULL)
          (*hash->value_destroy_func)(s->value);
        if (last == NULL) {
          hash->table [i] = s->next;
          n = s->next;
        } else  {
          last->next = s->next;
          n = last->next;
        }
        free(s);
        hash->in_use--;
        count++;
        s = n;
      } else {
        last = s;
        s = s->next;
      }
    }
  }
  if (count > 0)
    rehash(hash);
  return count;
}

uint32_t
hashTableForeachSteal(GHashTable* hash, GHRFunc func, void* user_data) {
  int32_t i;
  int32_t count = 0;

  DCT_ASSERT(hash != NULL);
  DCT_ASSERT(func != NULL);

  for (i = 0; i < hash->table_size; i++) {
    Slot* s, *last;

    last = NULL;
    for (s = hash->table [i]; s != NULL;) {
      if ((*func)(s->key, s->value, user_data)) {
        Slot* n;

        if (last == NULL) {
          hash->table [i] = s->next;
          n = s->next;
        } else  {
          last->next = s->next;
          n = last->next;
        }
        free(s);
        hash->in_use--;
        count++;
        s = n;
      } else {
        last = s;
        s = s->next;
      }
    }
  }
  if (count > 0)
    rehash(hash);
  return count;
}

void
hashTableDestroy(GHashTable* hash) {
  int32_t i;

  DCT_ASSERT(hash != NULL);

  for (i = 0; i < hash->table_size; i++) {
    Slot* s, *next;

    for (s = hash->table [i]; s != NULL; s = next) {
      next = s->next;

      if (hash->key_destroy_func != NULL)
        (*hash->key_destroy_func)(s->key);
      if (hash->value_destroy_func != NULL)
        (*hash->value_destroy_func)(s->value);
      free(s);
    }
  }
  free(hash->table);

  free(hash);
}

bool_t
directEqual(const void* v1, const void* v2) {
  return v1 == v2;
}

uint32_t
directHash(const void* v1) {
  return CAST_POINTER_TO_UINT32(v1);
}

bool_t
intEqual(const void* v1, const void* v2) {
  return CAST_POINTER_TO_UINT32(v1) == CAST_POINTER_TO_UINT32(v2);
}

uint32_t
intHash(const void* v1) {
  return CAST_POINTER_TO_UINT32(v1);
}

bool_t
strEqual(const void* v1, const void* v2) {
  return strcmp(v1, v2) == 0;
}

uint32_t
strHash(const void* v1) {
  uint32_t hash = 0;
  char* p = (char*) v1;

  while (*p++)
    hash = (hash << 5) - (hash + *p);

  return hash;
}
