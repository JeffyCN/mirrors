/*
 * drivers/staging/android/ion/ion.h
 *
 * Copyright (C) 2011 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _LINUX_ION_H
#define _LINUX_ION_H

#include <linux/types.h>

#include "../uapi/ion.h"

struct ion_handle;
struct ion_device;
struct ion_heap;
struct ion_mapper;
struct ion_client;
struct ion_buffer;

/* This should be removed some day when phys_addr_t's are fully
   plumbed in the kernel, and all instances of ion_phys_addr_t should
   be converted to phys_addr_t.  For the time being many kernel interfaces
   do not accept phys_addr_t's that would have to */
#define ion_phys_addr_t unsigned long

/**
 * struct ion_platform_heap - defines a heap in the given platform
 * @type:   type of the heap from ion_heap_type enum
 * @id:     unique identifier for heap.  When allocating higher numbers
 *      will be allocated from first.  At allocation these are passed
 *      as a bit mask and therefore can not exceed ION_NUM_HEAP_IDS.
 * @name:   used for debug purposes
 * @base:   base address of heap in physical memory if applicable
 * @size:   size of the heap in bytes if applicable
 * @align:  required alignment in physical memory if applicable
 * @priv:   private info passed from the board file
 *
 * Provided by the board file.
 */
struct ion_platform_heap {
  enum ion_heap_type type;
  unsigned int id;
  const char* name;
  ion_phys_addr_t base;
  size_t size;
  ion_phys_addr_t align;
  void* priv;
};

/**
 * struct ion_platform_data - array of platform heaps passed from board file
 * @nr:     number of structures in the array
 * @heaps:  array of platform_heap structions
 *
 * Provided by the board file in the form of platform data to a platform device.
 */
struct ion_platform_data {
  int nr;
  struct ion_platform_heap* heaps;
};

#endif /* _LINUX_ION_H */
