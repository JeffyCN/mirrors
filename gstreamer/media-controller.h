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
#ifndef __MEDIA_CONTROLLER_H__
#define __MEDIA_CONTROLLER_H__

#include <gst/gst.h>

#include "mediactl-priv.h"
#include "mediactl.h"
#include "v4l2subdev.h"

/**
 * SECTION:mediacontroller
 * @short_description: Media Controller framework handler
 *
 * #GstMediaController uses media controller framework to probe available
 * media devices and their topology.
 */

G_BEGIN_DECLS typedef struct _GstMediaController GstMediaController;
typedef struct media_entity GstMediaEntity;

struct _GstMediaController
{
  struct media_device *device;
};

GstMediaController *gst_media_controller_new_by_vnode (const gchar * vnode);
void gst_media_controller_delete (GstMediaController * controller);

GstMediaEntity *gst_media_find_entity_by_name (GstMediaController * controller,
    const gchar * dev_name);
GstMediaEntity *gst_media_get_last_entity (GstMediaController * controller);
G_END_DECLS
#endif
