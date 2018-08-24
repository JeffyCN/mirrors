/*
 * Copyright 2017 Rockchip Electronics Co., Ltd
 *     Author: Jacob Chen <jacob2.chen@rock-chips.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#include "media-controller.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_MEDIA_INDEX 16

GstMediaController *
gst_media_controller_new_by_vnode (const gchar * vnode)
{
  GstMediaController *it;
  gchar sys_path[64];
  struct media_device *device = NULL;
  guint nents, j, i = 0;
  FILE *fp;

  while (i < MAX_MEDIA_INDEX) {
    snprintf (sys_path, 64, "/dev/media%d", i++);
    fp = fopen (sys_path, "r");
    if (!fp)
      continue;
    fclose (fp);

    device = media_device_new (sys_path);

    /* Enumerate entities, pads and links. */
    media_device_enumerate (device);

    nents = media_get_entities_count (device);
    for (j = 0; j < nents; ++j) {
      struct media_entity *entity = media_get_entity (device, j);
      const char *devname = media_entity_get_devname (entity);
      if (NULL != devname) {
        if (!strcmp (devname, vnode)) {
          goto out;
        }
      }
    }

    media_device_unref (device);
  }

out:
  if (!device)
    return NULL;

  it = g_slice_new0 (GstMediaController);
  it->device = device;

  return it;
}

void
gst_media_controller_delete (GstMediaController * controller)
{
  media_device_unref (controller->device);

  g_slice_free (GstMediaController, controller);
}

GstMediaEntity *
gst_media_find_entity_by_name (GstMediaController * controller,
    const gchar * dev_name)
{
  return media_get_entity_by_name (controller->device, dev_name,
      strlen (dev_name));
}

GstMediaEntity *
gst_media_get_last_entity (GstMediaController * controller)
{
  GstMediaEntity *entity;
  int nents;

  nents = media_get_entities_count (controller->device);
  entity = media_get_entity (controller->device, nents - 1);

  return entity;
}
