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

/**
 * SECTION:element-mppvpxalphadecodebin
 * @title: Wrapper to decode MPP VP8/VP9 alpha using mppvideodec
 *
 * Use two `mppvideodec` instance in order to decode VP8/VP9 alpha channel.
 *
 * Since: 1.20
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstmppvpxalphadecodebin.h"

static GstStaticPadTemplate sink_template = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-vp8, codec-alpha = (boolean) true; "
        "video/x-vp9, codec-alpha = (boolean) true"));

struct _GstMppVpxAlphaDecodeBin
{
  GstMppAlphaDecodeBin parent;
};

#define gst_mpp_vpx_alpha_decode_bin_parent_class parent_class
G_DEFINE_TYPE (GstMppVpxAlphaDecodeBin, gst_mpp_vpx_alpha_decode_bin,
    GST_TYPE_MPP_ALPHA_DECODE_BIN);

static void
gst_mpp_vpx_alpha_decode_bin_class_init (GstMppVpxAlphaDecodeBinClass * klass)
{
  GstMppAlphaDecodeBinClass *adbin_class = (GstMppAlphaDecodeBinClass *) klass;
  GstElementClass *element_class = (GstElementClass *) klass;

  adbin_class->decoder_name = "mppvideodec";
  gst_element_class_add_static_pad_template (element_class, &sink_template);

  gst_element_class_set_static_metadata (element_class,
      "VP8/VP9 Alpha Decoder", "Codec/Decoder/Video",
      "Wrapper bin to decode VP8/VP9 with alpha stream.",
      "Julian Bouzas <julian.bouzas@collabora.com>");
}

static void
gst_mpp_vpx_alpha_decode_bin_init (GstMppVpxAlphaDecodeBin * self)
{
  (void) self;
}
