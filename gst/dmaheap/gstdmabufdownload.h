/*
 * GStreamer dmabufdownload element
 * Copyright (C) 2026 Rockchip Electronics Co., Ltd.
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
 * Free Software Foundation, Inc., 51 Franklin St, Suite 500,
 * Boston, MA 02110-1335, USA.
 */

#ifndef __GST_DMABUF_DOWNLOAD_H__
#define __GST_DMABUF_DOWNLOAD_H__

#include <gst/base/gstbasetransform.h>
#include <gst/video/video.h>

G_BEGIN_DECLS;

#define GST_TYPE_DMABUF_DOWNLOAD              (gst_dmabuf_download_get_type ())
#define GST_IS_DMABUF_DOWNLOAD(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_DMABUF_DOWNLOAD))
#define GST_DMABUF_DOWNLOAD(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_DMABUF_DOWNLOAD, GstDmaBufDownload))
#define GST_IS_DMABUF_DOWNLOAD_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GST_TYPE_DMABUF_DOWNLOAD))
#define GST_DMABUF_DOWNLOAD_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GST_TYPE_DMABUF_DOWNLOAD, GstDmaBufDownloadClass))
#define GST_DMABUF_DOWNLOAD_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_DMABUF_DOWNLOAD, GstDmaBufDownloadClass))

typedef struct _GstDmaBufDownload GstDmaBufDownload;
typedef struct _GstDmaBufDownloadClass GstDmaBufDownloadClass;

struct _GstDmaBufDownload
{
  GstBaseTransform parent;

  GstVideoInfo in_info;
  GstVideoInfo out_info;
  GstBufferPool *pool;
  GstCaps *pool_caps;

  gboolean use_rga;

#ifdef HAVE_RGA_UTIL
  gint rga_fmt_index;           /* rga format table index, -1 = no rga */
  gboolean rga_active;          /* this negotiation can use the rga path */
  gboolean rga_failed;          /* an rga job failed: stick to the CPU */
#endif
};

struct _GstDmaBufDownloadClass
{
  GstBaseTransformClass parent_class;
};

GType gst_dmabuf_download_get_type (void);

gboolean gst_dmabuf_download_plugin_init (GstPlugin * plugin);

G_END_DECLS;

#endif /* __GST_DMABUF_DOWNLOAD_H__ */
