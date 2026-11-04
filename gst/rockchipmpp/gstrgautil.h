#ifndef __GST_RGA_UTIL_H__
#define __GST_RGA_UTIL_H__

#include <gst/gst.h>
#include <gst/video/video.h>
#include <rga/rga.h>
#include <rga/im2d.h>

G_BEGIN_DECLS;

/* Format table entry: the gst <-> rga mapping plus the stride rules
 * the im2d check layer enforces: every plane-0 row must be 16-byte
 * aligned (48 for 24-bit RGB, where gst pads the stride to 4 bytes),
 * 64 bytes for the packed 10-bit formats, whose wstride is in bytes
 * in the first place. */
struct gst_rga_format_entry
{
  GstVideoFormat gst_format;
  RgaSURF_FORMAT rga_format;
  guint pixel_stride;           /* bytes per pixel of plane 0, 0 = byte-unit wstride */
  gboolean is_yuv;
  guint walign;                 /* plane-0 row alignment in bytes */
  guint uv_vshift;              /* last-plane row shift: 1 for 4:2:0, else 0 */
};

extern const struct gst_rga_format_entry gst_rga_formats_[];

guint gst_rga_formats_count (void);
guint gst_rga_format_index (GstVideoFormat format);
const struct gst_rga_format_entry *gst_rga_format_at (guint index);

/* True when the tight plane-0 byte pitch table entry @index has at
 * @width fails the wstride alignment the im2d check layer demands.
 * The tight pitch follows from the caps width alone, so the geometry
 * gate is decided at caps time already. */
gboolean gst_rga_pitch_unsupported_at (guint index, gint width);

/* Overlay a buffer's GstVideoMeta strides and offsets onto @info so
 * the wrap below describes the memory the producer actually wrote. */
void gst_rga_info_apply_video_meta (GstVideoInfo * info, GstBuffer * buffer);

gsize gst_rga_frame_size (guint index, GstVideoInfo * info);

/* Build the rga_buffer_t over a dma fd or a mapped virtual address.
 * Fails (all-zero return) when the geometry misses the stride or
 * rect alignment rules; callers treat !wstride as unsupported. */
rga_buffer_t gst_rga_wrap_info (guint index, gint fd, void *vir_addr,
    GstVideoInfo * info);

/* Same with an explicit RGA vstride: > 0 pins it for single-plane
 * packed formats (NV12_10LE40 et al), whose gst representation has
 * no uv offset to derive the producer's ver_stride from; 0 derives
 * it from the info. */
rga_buffer_t gst_rga_wrap_info_vs (guint index, gint fd, void *vir_addr,
    GstVideoInfo * info, gint vstride_override);

/* Submit one RGA job.  @srect selects the source region (all-zero
 * means the whole frame, cropping); @drect the destination region
 * (all-zero means the whole frame, scaling follows from differing
 * src/dst geometry); @usage carries IM_HAL_TRANSFORM_ROT_* /
 * IM_HAL_TRANSFORM_FLIP_* bits (rotation and mirroring), IM_SYNC is
 * always added.  Returns the im2d status, <= 0 on failure. */
IM_STATUS gst_rga_process (rga_buffer_t * src, rga_buffer_t * dst,
    im_rect * srect, im_rect * drect, gint usage);

G_END_DECLS;

#endif /* __GST_RGA_UTIL_H__ */
