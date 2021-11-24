/* GStreamer
 * Copyright (C) <2021> Collabora Ltd.
 *   Author: Nicolas Dufresne <nicolas.dufresne@collabora.com>
 *   Author: Julian Bouzas <julian.bouzas@collabora.com>
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
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/pbutils/pbutils.h>

#include "gstmppalphadecodebin.h"

GST_DEBUG_CATEGORY_STATIC (mppalphadecodebin_debug);
#define GST_CAT_DEFAULT (mppalphadecodebin_debug)

typedef struct
{
  GstBin parent;

  gboolean constructed;
  const gchar *missing_element;
} GstMppAlphaDecodeBinPrivate;

#define gst_mpp_alpha_decode_bin_parent_class parent_class
G_DEFINE_ABSTRACT_TYPE_WITH_CODE (GstMppAlphaDecodeBin,
    gst_mpp_alpha_decode_bin, GST_TYPE_BIN,
    G_ADD_PRIVATE (GstMppAlphaDecodeBin);
    GST_DEBUG_CATEGORY_INIT (mppalphadecodebin_debug, "mppalphadecodebin", 0,
        "mppalphadecodebin"));

static GstStaticPadTemplate gst_mpp_alpha_decode_bin_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

static gboolean
gst_mpp_alpha_decode_bin_open (GstMppAlphaDecodeBin * self)
{
  GstMppAlphaDecodeBinPrivate *priv =
      gst_mpp_alpha_decode_bin_get_instance_private (self);

  if (priv->missing_element) {
    gst_element_post_message (GST_ELEMENT (self),
        gst_missing_element_message_new (GST_ELEMENT (self),
            priv->missing_element));
  } else if (!priv->constructed) {
    GST_ELEMENT_ERROR (self, CORE, FAILED,
        ("Failed to construct mpp alpha decoder pipeline."), (NULL));
  }

  return priv->constructed;
}

static GstStateChangeReturn
gst_mpp_alpha_decode_bin_change_state (GstElement * element,
    GstStateChange transition)
{
  GstMppAlphaDecodeBin *self = GST_MPP_ALPHA_DECODE_BIN (element);

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
      if (!gst_mpp_alpha_decode_bin_open (self))
        return GST_STATE_CHANGE_FAILURE;
      break;
    default:
      break;
  }

  return GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
}

static void
gst_mpp_alpha_decode_bin_constructed (GObject * obj)
{
  GstMppAlphaDecodeBin *self = GST_MPP_ALPHA_DECODE_BIN (obj);
  GstMppAlphaDecodeBinPrivate *priv =
      gst_mpp_alpha_decode_bin_get_instance_private (self);
  GstMppAlphaDecodeBinClass *klass = GST_MPP_ALPHA_DECODE_BIN_GET_CLASS (self);
  GstPad *src_gpad, *sink_gpad;
  GstPad *src_pad = NULL, *sink_pad = NULL;
  GstElement *alphademux = NULL;
  GstElement *queue = NULL;
  GstElement *alpha_queue = NULL;
  GstElement *decoder = NULL;
  GstElement *alpha_decoder = NULL;
  GstElement *alphacombine = NULL;

  /* setup ghost pads */
  sink_gpad = gst_ghost_pad_new_no_target_from_template ("sink",
      gst_element_class_get_pad_template (GST_ELEMENT_CLASS (klass), "sink"));
  gst_element_add_pad (GST_ELEMENT (self), sink_gpad);

  src_gpad = gst_ghost_pad_new_no_target_from_template ("src",
      gst_element_class_get_pad_template (GST_ELEMENT_CLASS (klass), "src"));
  gst_element_add_pad (GST_ELEMENT (self), src_gpad);

  /* create elements */
  alphademux = gst_element_factory_make ("codecalphademux", NULL);
  if (!alphademux) {
    priv->missing_element = "codecalphademux";
    goto cleanup;
  }

  queue = gst_element_factory_make ("queue", NULL);
  alpha_queue = gst_element_factory_make ("queue", NULL);
  if (!queue || !alpha_queue) {
    priv->missing_element = "queue";
    goto cleanup;
  }

  decoder = gst_element_factory_make (klass->decoder_name, "maindec");
  if (!decoder) {
    priv->missing_element = klass->decoder_name;
    goto cleanup;
  }

  alpha_decoder = gst_element_factory_make (klass->decoder_name, "alphadec");
  if (!alpha_decoder) {
    priv->missing_element = klass->decoder_name;
    goto cleanup;
  }

  /* We disable QoS on decoders because we need to maintain frame pairing in
   * order for alphacombine to work. */
  g_object_set (decoder, "qos", FALSE, NULL);
  g_object_set (alpha_decoder, "qos", FALSE, NULL);

  alphacombine = gst_element_factory_make ("alphacombine", NULL);
  if (!alphacombine) {
    priv->missing_element = "alphacombine";
    goto cleanup;
  }

  gst_bin_add_many (GST_BIN (self), alphademux, queue, alpha_queue, decoder,
      alpha_decoder, alphacombine, NULL);

  /* link elements */
  sink_pad = gst_element_get_static_pad (alphademux, "sink");
  gst_ghost_pad_set_target (GST_GHOST_PAD (sink_gpad), sink_pad);
  gst_object_unref (sink_pad);

  gst_element_link_pads (alphademux, "src", queue, "sink");
  gst_element_link_pads (queue, "src", decoder, "sink");
  gst_element_link_pads (decoder, "src", alphacombine, "sink");

  gst_element_link_pads (alphademux, "alpha", alpha_queue, "sink");
  gst_element_link_pads (alpha_queue, "src", alpha_decoder, "sink");
  gst_element_link_pads (alpha_decoder, "src", alphacombine, "alpha");

  src_pad = gst_element_get_static_pad (alphacombine, "src");
  gst_ghost_pad_set_target (GST_GHOST_PAD (src_gpad), src_pad);
  gst_object_unref (src_pad);

  g_object_set (queue, "max-size-bytes", 0, "max-size-time", 0,
      "max-size-buffers", 1, NULL);
  g_object_set (alpha_queue, "max-size-bytes", 0, "max-size-time", 0,
      "max-size-buffers", 1, NULL);

  /* signal success, we will handle this in NULL->READY transition */
  priv->constructed = TRUE;
  return;

cleanup:
  if (alphademux)
    gst_object_unref (alphademux);
  if (queue)
    gst_object_unref (queue);
  if (alpha_queue)
    gst_object_unref (alpha_queue);
  if (decoder)
    gst_object_unref (decoder);
  if (alpha_decoder)
    gst_object_unref (alpha_decoder);
  if (alphacombine)
    gst_object_unref (alphacombine);

  G_OBJECT_CLASS (parent_class)->constructed (obj);
}

static void
gst_mpp_alpha_decode_bin_class_init (GstMppAlphaDecodeBinClass * klass)
{
  GstElementClass *element_class = (GstElementClass *) klass;
  GObjectClass *obj_class = (GObjectClass *) klass;

  /* This is needed to access the subclass class instance, otherwise we cannot
   * read the class parameters */
  obj_class->constructed = gst_mpp_alpha_decode_bin_constructed;

  gst_element_class_add_static_pad_template (element_class,
      &gst_mpp_alpha_decode_bin_src_template);
  element_class->change_state =
      GST_DEBUG_FUNCPTR (gst_mpp_alpha_decode_bin_change_state);
}

static void
gst_mpp_alpha_decode_bin_init (GstMppAlphaDecodeBin * self)
{
  (void) self;
}
