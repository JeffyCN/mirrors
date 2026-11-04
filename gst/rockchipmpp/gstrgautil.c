/* Shared RGA conversion core.
 *
 * Extracted from the rgavideoconvert element so the rockchipmpp
 * decoder/encoder conversion tiers and the element describe frames
 * to librga through exactly the same mapping, stride and alignment
 * rules.  Compiled into each plugin that needs it (no cross-plugin
 * symbols). */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstrgautil.h"

#include <string.h>

/* The formats both the driver and this mapping can describe; mirrors
 * the gst <-> rga format table of the legacy mpp conversion tier.
 *
 * The planar YUV formats stay out on purpose: this kernel's RGA
 * driver fails every 3-plane request with "failed to map job info"
 * (rk3588, legacy rga driver), so advertising them would turn a
 * link-time promise into a runtime error instead of a clean
 * not-negotiated and a videoconvert fallback.
 *
 * The packed 10-bit YUV formats (NV12_10LE40, NV16_10LE40) pass the
 * plane-0 byte stride directly as the RGA wstride (see the pixel
 * stride column: 0 = byte-unit wstride) and take 64-byte rows. */
const struct gst_rga_format_entry gst_rga_formats_[] = {
  {GST_VIDEO_FORMAT_NV12, RK_FORMAT_YCbCr_420_SP, 1, TRUE, 16, 1},
  {GST_VIDEO_FORMAT_NV21, RK_FORMAT_YCrCb_420_SP, 1, TRUE, 16, 1},
  {GST_VIDEO_FORMAT_NV16, RK_FORMAT_YCbCr_422_SP, 1, TRUE, 16, 0},
  {GST_VIDEO_FORMAT_NV61, RK_FORMAT_YCrCb_422_SP, 1, TRUE, 16, 0},
  {GST_VIDEO_FORMAT_BGR16, RK_FORMAT_RGB_565, 2, FALSE, 16, 0},
  {GST_VIDEO_FORMAT_RGB16, RK_FORMAT_BGR_565, 2, FALSE, 16, 0},
  /* The 5-5-5 formats stay out: this driver advertises ARGB_5551 in
   * its capability list but fails every such job at commit time. */
  {GST_VIDEO_FORMAT_RGB, RK_FORMAT_RGB_888, 3, FALSE, 48, 0},
  {GST_VIDEO_FORMAT_BGR, RK_FORMAT_BGR_888, 3, FALSE, 48, 0},
  {GST_VIDEO_FORMAT_RGBA, RK_FORMAT_RGBA_8888, 4, FALSE, 16, 0},
  {GST_VIDEO_FORMAT_BGRA, RK_FORMAT_BGRA_8888, 4, FALSE, 16, 0},
  {GST_VIDEO_FORMAT_RGBx, RK_FORMAT_RGBX_8888, 4, FALSE, 16, 0},
  {GST_VIDEO_FORMAT_BGRx, RK_FORMAT_BGRX_8888, 4, FALSE, 16, 0},
#ifdef HAVE_NV12_10LE40
  /* Same 4-pixels-per-5-bytes packing as the DRM NV15 fourcc the
   * librga drm_utils table maps to this RGA format. */
  {GST_VIDEO_FORMAT_NV12_10LE40, RK_FORMAT_YCbCr_420_SP_10B, 0, TRUE, 64, 1},
#endif
#ifdef HAVE_NV16_10LE40
  {GST_VIDEO_FORMAT_NV16_10LE40, RK_FORMAT_YCbCr_422_SP_10B, 0, TRUE, 64, 0},
#endif
};

guint
gst_rga_formats_count (void)
{
  return G_N_ELEMENTS (gst_rga_formats_);
}

const struct gst_rga_format_entry *
gst_rga_format_at (guint index)
{
  if (index >= G_N_ELEMENTS (gst_rga_formats_))
    return NULL;
  return &gst_rga_formats_[index];
}

guint
gst_rga_format_index (GstVideoFormat format)
{
  guint i;

  for (i = 0; i < G_N_ELEMENTS (gst_rga_formats_); i++) {
    if (gst_rga_formats_[i].gst_format == format)
      return i;
  }
  return (guint) - 1;
}

gboolean
gst_rga_pitch_unsupported_at (guint index, gint width)
{
  GstVideoInfo info;

  if (index >= G_N_ELEMENTS (gst_rga_formats_) || width < 1)
    return FALSE;

  gst_video_info_set_format (&info, gst_rga_formats_[index].gst_format, width,
      1);

  return GST_VIDEO_INFO_PLANE_STRIDE (&info, 0) %
      gst_rga_formats_[index].walign != 0;
}

void
gst_rga_info_apply_video_meta (GstVideoInfo * info, GstBuffer * buffer)
{
  GstVideoMeta *meta = gst_buffer_get_video_meta (buffer);
  guint i, n = GST_VIDEO_INFO_N_PLANES (info);

  if (!meta || meta->n_planes != n)
    return;

  for (i = 0; i < n; i++) {
    GST_VIDEO_INFO_PLANE_STRIDE (info, i) = meta->stride[i];
    GST_VIDEO_INFO_PLANE_OFFSET (info, i) = meta->offset[i];
  }
}

/* Size of the frame region the buffer must hold for the layout @info
 * describes: from the plane-0 offset to the end of the last plane.
 * Unlike GST_VIDEO_INFO_SIZE this follows the strides and offsets of
 * the applied video meta, so a padded layout copies its real extent
 * rather than the tight caps size. */
gsize
gst_rga_frame_size (guint index, GstVideoInfo * info)
{
  const struct gst_rga_format_entry *e = gst_rga_format_at (index);
  gint height = GST_VIDEO_INFO_HEIGHT (info);
  guint n, vshift, last;
  gint rows;

  if (!e || height < 1)
    return 0;

  n = GST_VIDEO_INFO_N_PLANES (info);
  vshift = e->uv_vshift;
  last = n - 1;
  rows = (height + (1 << vshift) - 1) >> vshift;

  return GST_VIDEO_INFO_PLANE_OFFSET (info, last) +
      (gsize) GST_VIDEO_INFO_PLANE_STRIDE (info, last) * rows;
}

rga_buffer_t
gst_rga_wrap_info_vs (guint index, gint fd, void *vir_addr,
    GstVideoInfo * info, gint vstride_override)
{
  const struct gst_rga_format_entry *e = gst_rga_format_at (index);
  guint width = GST_VIDEO_INFO_WIDTH (info);
  guint height = GST_VIDEO_INFO_HEIGHT (info);
  gint stride = GST_VIDEO_INFO_PLANE_STRIDE (info, 0);
  gsize offset = GST_VIDEO_INFO_PLANE_OFFSET (info, 0);
  guint hstride, vstride;

  if (!e || width < 1 || height < 1)
    return (rga_buffer_t) {
    0};

  /* hstride is in pixels; the packed 10-bit formats pass the plane-0
   * byte stride directly as the RGA wstride (byte-unit, see the
   * format table). */
  if (e->pixel_stride > 0)
    hstride = stride / e->pixel_stride;
  else
    hstride = stride;

  if (stride <= 0)
    return (rga_buffer_t) {
    0};

  if (e->walign > 1 && (e->pixel_stride > 0 ? stride : hstride) % e->walign)
    return (rga_buffer_t) {
    0};

  if (vstride_override > 0)
    vstride = vstride_override;
  else if (GST_VIDEO_INFO_N_PLANES (info) > 1 &&
      GST_VIDEO_INFO_PLANE_OFFSET (info, 1) > offset)
    vstride = (GST_VIDEO_INFO_PLANE_OFFSET (info, 1) - offset) / stride;
  else
    vstride = height;

  /* RGA requires yuv image rects to be 2-aligned. */
  if (e->is_yuv && (width % 2 || height % 2 || hstride % 2 || vstride % 2))
    return (rga_buffer_t) {
    0};

  if (fd >= 0)
    return wrapbuffer_fd_t (fd, width, height, hstride, vstride, e->rga_format);

  if (vir_addr)
    return wrapbuffer_virtualaddr_t ((guint8 *) vir_addr + offset, width,
        height, hstride, vstride, e->rga_format);

  return (rga_buffer_t) {
  0};
}

rga_buffer_t
gst_rga_wrap_info (guint index, gint fd, void *vir_addr, GstVideoInfo * info)
{
  return gst_rga_wrap_info_vs (index, fd, vir_addr, info, 0);
}

IM_STATUS
gst_rga_process (rga_buffer_t * src, rga_buffer_t * dst,
    im_rect * srect, im_rect * drect, gint usage)
{
  im_rect zero = { 0, };
  im_rect prect = { 0, };

  if (!srect)
    srect = &zero;
  if (!drect)
    drect = &zero;

  rga_buffer_t pat = { 0 };

  return improcess (*src, *dst, pat, *srect, *drect, prect, usage | IM_SYNC);
}
