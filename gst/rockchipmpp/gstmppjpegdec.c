/*
 * Copyright 2018 Rockchip Electronics Co., Ltd
 *     Author: Randy Li <randy.li@rock-chips.com>
 *
 * Copyright 2021 Rockchip Electronics Co., Ltd
 *     Author: Jeffy Chen <jeffy.chen@rock-chips.com>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/base/gstadapter.h>

#include "gstmppallocator.h"
#include "gstmppjpegdec.h"

#define GST_MPP_JPEG_DEC(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), \
    GST_TYPE_MPP_JPEG_DEC, GstMppJpegDec))

#define GST_CAT_DEFAULT mpp_jpeg_dec_debug
GST_DEBUG_CATEGORY (GST_CAT_DEFAULT);

struct _GstMppJpegDec
{
  GstMppDec parent;

  /* size of output buffer */
  guint buf_size;

  /* group for input packet buffer allocations */
  MppBufferGroup input_group;

  /* framing state for the parse() vfunc (unparsed input) */
  gboolean saw_header;
  gboolean parse_resync;
  guint parse_entropy_len;
};

#define parent_class gst_mpp_jpeg_dec_parent_class
G_DEFINE_TYPE (GstMppJpegDec, gst_mpp_jpeg_dec, GST_TYPE_MPP_DEC);

/* Default output format is auto */
static GstVideoFormat DEFAULT_PROP_FORMAT = GST_VIDEO_FORMAT_UNKNOWN;

enum
{
  PROP_0,
  PROP_FORMAT,
  PROP_LAST,
};

/* Accept both parsed and unparsed JPEG input: parsed streams (jpegparse,
 * the mpp jpeg encoder) arrive one image per buffer, while raw streams
 * are framed by the parse() vfunc below, mirroring the software jpegdec.
 * No "parsed" field here: a caps subset check would otherwise reject
 * every producer that does not carry the field. */
static GstStaticPadTemplate gst_mpp_jpeg_dec_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("image/jpeg")
    );

#define MPP_JPEGDEC_FORMATS \
    MPP_DEC_FORMATS ", BGR16, RGB16, " \
    "ABGR, ARGB, BGRA, RGBA, xBGR, xRGB, BGRx, RGBx"

static GstStaticPadTemplate gst_mpp_jpeg_dec_src_template =
    GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (MPP_DEC_CAPS_MAKE ("{" MPP_JPEGDEC_FORMATS "}") ";")
    );

static const GstVideoFormat gst_mpp_jpeg_dec_pp_formats[] = {
  GST_VIDEO_FORMAT_UNKNOWN,
  GST_VIDEO_FORMAT_NV12,
  GST_VIDEO_FORMAT_RGB16,
  GST_VIDEO_FORMAT_BGR16,
  GST_VIDEO_FORMAT_ARGB,
  GST_VIDEO_FORMAT_ABGR,
  GST_VIDEO_FORMAT_RGBA,
  GST_VIDEO_FORMAT_BGRA,
  GST_VIDEO_FORMAT_xRGB,
  GST_VIDEO_FORMAT_xBGR,
  GST_VIDEO_FORMAT_RGBx,
  GST_VIDEO_FORMAT_BGRx,
};

static GstVideoFormat
gst_mpp_jpeg_dec_try_pp_convert (GstVideoDecoder * decoder,
    GstVideoFormat format, gboolean force)
{
  GstMppDec *mppdec = GST_MPP_DEC (decoder);
  MppFrameFormat mpp_format = force ? MPP_FMT_YUV420SP : MPP_FMT_BUTT;
  guint i;

  for (i = 0; i < ARRAY_SIZE (gst_mpp_jpeg_dec_pp_formats); i++) {
    if (format == gst_mpp_jpeg_dec_pp_formats[i]) {
      mpp_format = gst_mpp_gst_format_to_mpp_format (format);
      break;
    }
  }

  /* Using MPP internal format conversion (PP) */
  if (mpp_format != MPP_FMT_BUTT) {
    if (mppdec->mpi->control (mppdec->mpp_ctx, MPP_DEC_SET_OUTPUT_FORMAT,
            &mpp_format) >= 0)
      return gst_mpp_mpp_format_to_gst_format (mpp_format);
  }

  return GST_VIDEO_FORMAT_UNKNOWN;
}

static GstVideoFormat
gst_mpp_jpeg_dec_get_format (GstStructure * structure)
{
  const gchar *s;

  if ((s = gst_structure_get_string (structure, "format")))
    return gst_video_format_from_string (s);

  return GST_VIDEO_FORMAT_UNKNOWN;
}

static void
gst_mpp_jpeg_dec_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec)
{
  GstVideoDecoder *decoder = GST_VIDEO_DECODER (object);
  GstMppDec *mppdec = GST_MPP_DEC (decoder);

  switch (prop_id) {
    case PROP_FORMAT:{
      if (mppdec->input_state)
        GST_WARNING_OBJECT (decoder, "unable to change output format");
      else
        mppdec->format = g_value_get_enum (value);
      break;
    }

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      return;
  }
}

static void
gst_mpp_jpeg_dec_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec)
{
  GstVideoDecoder *decoder = GST_VIDEO_DECODER (object);
  GstMppDec *mppdec = GST_MPP_DEC (decoder);

  switch (prop_id) {
    case PROP_FORMAT:
      g_value_set_enum (value, mppdec->format);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* The framing state must not survive a flush (seek): the parse()
 * vfunc would misframe the first frames of the new segment. */
static gboolean
gst_mpp_jpeg_dec_flush (GstVideoDecoder * decoder)
{
  GstMppJpegDec *self = GST_MPP_JPEG_DEC (decoder);
  GstVideoDecoderClass *pclass = GST_VIDEO_DECODER_CLASS (parent_class);

  /* Chain up: the base flush resets the MPP context and the pending
   * frames; the parse state resets on top of it. */
  if (pclass->flush && !pclass->flush (decoder))
    return FALSE;

  self->saw_header = FALSE;
  self->parse_resync = FALSE;
  self->parse_entropy_len = 0;

  return TRUE;
}

static gboolean
gst_mpp_jpeg_dec_start (GstVideoDecoder * decoder)
{
  GstVideoDecoderClass *pclass = GST_VIDEO_DECODER_CLASS (parent_class);
  GstMppJpegDec *self = GST_MPP_JPEG_DEC (decoder);
  GstMppDec *mppdec = GST_MPP_DEC (decoder);

  GST_DEBUG_OBJECT (self, "starting");

  self->saw_header = FALSE;
  self->parse_resync = FALSE;
  self->parse_entropy_len = 0;

  if (mpp_buffer_group_get_internal (&self->input_group, MPP_BUFFER_TYPE_DRM))
    return FALSE;

  if (!pclass->start (decoder)) {
    mpp_buffer_group_put (self->input_group);
    return FALSE;
  }

  mppdec->mpp_type = MPP_VIDEO_CodingMJPEG;

  GST_DEBUG_OBJECT (self, "started");

  return TRUE;
}

static gboolean
gst_mpp_jpeg_dec_stop (GstVideoDecoder * decoder)
{
  GstVideoDecoderClass *pclass = GST_VIDEO_DECODER_CLASS (parent_class);
  GstMppJpegDec *self = GST_MPP_JPEG_DEC (decoder);

  GST_DEBUG_OBJECT (self, "stopping");

  pclass->stop (decoder);

  mpp_buffer_group_put (self->input_group);

  GST_DEBUG_OBJECT (self, "stopped");

  return TRUE;
}

static guint
gst_mpp_jpeg_dec_required_size (GstVideoDecoder * decoder)
{
  GstMppDec *mppdec = GST_MPP_DEC (decoder);
  GstVideoInfo *info = &mppdec->info;
  GstVideoFormat gst_format;
  MppFrameFormat mpp_format;
  MppSysCfg cfg;
  guint width, height, hstride, vstride, size;

  gst_format = GST_VIDEO_INFO_FORMAT (info);
  mpp_format = gst_mpp_gst_format_to_mpp_format (gst_format);

  width = GST_VIDEO_INFO_WIDTH (info);
  height = GST_VIDEO_INFO_HEIGHT (info);
  hstride = GST_MPP_VIDEO_INFO_HSTRIDE (info);
  vstride = GST_MPP_VIDEO_INFO_VSTRIDE (info);

  /* FIXME: Workaround MPP's JPEG parser size requirement issue (w * h * 4) */
  size = width * height * 4;

  /* Based on mpp/base/test/mpp_sys_cfg_test.c */

  if (mpp_sys_cfg_get (&cfg))
    return size;

  mpp_sys_cfg_set_u32 (cfg, "dec_buf_chk:type", 1);
  mpp_sys_cfg_set_u32 (cfg, "dec_buf_chk:enable", 1);
  mpp_sys_cfg_set_u32 (cfg, "dec_buf_chk:type", MPP_VIDEO_CodingMJPEG);
  mpp_sys_cfg_set_u32 (cfg, "dec_buf_chk:fmt_codec", mpp_format);
  mpp_sys_cfg_set_u32 (cfg, "dec_buf_chk:fmt_fbc", 0);
  mpp_sys_cfg_set_u32 (cfg, "dec_buf_chk:width", width);
  mpp_sys_cfg_set_u32 (cfg, "dec_buf_chk:height", height);
  mpp_sys_cfg_set_u32 (cfg, "dec_buf_chk:h_stride_by_byte", hstride);
  mpp_sys_cfg_set_u32 (cfg, "dec_buf_chk:v_stride", vstride);

  mpp_sys_cfg_ioctl (cfg);
  mpp_sys_cfg_get_u32 (cfg, "dec_buf_chk:size_total", &size);

  mpp_sys_cfg_put (cfg);

  return size;
}

static gboolean
gst_mpp_jpeg_dec_set_format (GstVideoDecoder * decoder,
    GstVideoCodecState * state)
{
  GstVideoDecoderClass *pclass = GST_VIDEO_DECODER_CLASS (parent_class);
  GstMppJpegDec *self = GST_MPP_JPEG_DEC (decoder);
  GstMppDec *mppdec = GST_MPP_DEC (decoder);
  GstVideoInfo *info = &mppdec->info;
  GstStructure *structure;
  GstVideoFormat src_format, dst_format;
  gint width = GST_VIDEO_INFO_WIDTH (&state->info);
  gint height = GST_VIDEO_INFO_HEIGHT (&state->info);
  gint dst_width, dst_height;
  guint align = GST_MPP_ALIGNMENT;

  if (!width || !height) {
    if (self->buf_size) {
      GST_ERROR_OBJECT (self, "ignore invalid input video info");
      return TRUE;
    } else {
      GST_ERROR_OBJECT (self, "invalid input video info");
      return FALSE;
    }
  }

  if (!pclass->set_format (decoder, state))
    return FALSE;

  /* Unparsed input is framed by the parse() vfunc below */
  structure = gst_caps_get_structure (state->caps, 0);
  {
    gboolean parsed = FALSE;

    gst_structure_get_boolean (structure, "parsed", &parsed);
    gst_video_decoder_set_packetized (decoder, parsed);
  }

  /* Figure out original output format */
  src_format = gst_mpp_jpeg_dec_get_format (structure);
  switch (src_format) {
    case GST_VIDEO_FORMAT_NV12:
    case GST_VIDEO_FORMAT_I420:
      src_format = GST_VIDEO_FORMAT_NV12;
      break;
    case GST_VIDEO_FORMAT_UYVY:
    case GST_VIDEO_FORMAT_Y42B:
    case GST_VIDEO_FORMAT_NV16:
      src_format = GST_VIDEO_FORMAT_NV16;
      break;
    default:
      /* FIXME: Gst doesn't support semi-planar version of Y444/Y41B...etc */
      src_format = GST_VIDEO_FORMAT_UNKNOWN;
      break;
  }

  /* Figure out final output info */
  gst_mpp_dec_fixup_video_info (decoder, src_format, width, height);
  dst_format = GST_VIDEO_INFO_FORMAT (info);
  dst_width = GST_VIDEO_INFO_WIDTH (info);
  dst_height = GST_VIDEO_INFO_HEIGHT (info);

  /* Prefer MPP internal format conversion (PP) */
  if (src_format != dst_format) {
    GstVideoFormat pp_format = GST_VIDEO_FORMAT_UNKNOWN;

    if (src_format == GST_VIDEO_FORMAT_UNKNOWN) {
      /* PP conversion is required for unknown formats */
      pp_format = gst_mpp_jpeg_dec_try_pp_convert (decoder, dst_format, TRUE);
      if (pp_format == GST_VIDEO_FORMAT_UNKNOWN) {
        GST_ERROR_OBJECT (self, "unsupported video format");
        return FALSE;
      }
    } else if (dst_width == width && dst_height == height) {
      /* Prefer PP conversion */
      pp_format = gst_mpp_jpeg_dec_try_pp_convert (decoder, dst_format, TRUE);
    }

    /* MPP is going to provide the converted format */
    if (pp_format != GST_VIDEO_FORMAT_UNKNOWN)
      src_format = pp_format;
  }

  if (dst_format != src_format || dst_width != width || dst_height != height) {
    if (!gst_mpp_use_rga ()) {
      GST_ERROR_OBJECT (self, "unable to convert without RGA");
      return FALSE;
    }

    /* Conversion required */
    GST_INFO_OBJECT (self, "convert from %s (%dx%d) to %s (%dx%d)",
        gst_mpp_video_format_to_string (src_format), width, height,
        gst_mpp_video_format_to_string (dst_format), dst_width, dst_height);

    mppdec->convert = TRUE;
    align = 0;
  }

  /* Original output buffer size calculation */
  gst_mpp_video_info_update_format (info, src_format, width, height);
  if (!gst_mpp_video_info_align (info, 0, 0))
    return FALSE;

  self->buf_size = gst_mpp_jpeg_dec_required_size (decoder);

  /* Update final output info */
  return gst_mpp_dec_update_simple_video_info (decoder, dst_format,
      dst_width, dst_height, align);
}

static inline gboolean
gst_mpp_jpeg_dec_parse_tag_has_entropy_segment (guint8 tag)
{
  if (tag == 0xda || (tag >= 0xd0 && tag <= 0xd7))
    return TRUE;
  return FALSE;
}

/* Frame an unparsed JPEG byte stream into one-image frames: scan for
 * SOI/EOI markers, walk the length-coded segments and the entropy-coded
 * data so payloads containing marker-looking bytes do not false-trigger.
 * Scanner logic adapted from gst_jpeg_dec_parse() in gst-plugins-good
 * 1.28.8 (gstjpegdec.c). */
static GstFlowReturn
gst_mpp_jpeg_dec_parse (GstVideoDecoder * bdec, GstVideoCodecFrame * frame,
    GstAdapter * adapter, gboolean at_eos)
{
  GstMppJpegDec *self = GST_MPP_JPEG_DEC (bdec);
  guint size;
  gint toadd = 0;
  gboolean resync;
  gint offset = 0, noffset;

  GST_VIDEO_CODEC_FRAME_SET_SYNC_POINT (frame);

  size = gst_adapter_available (adapter);
  GST_DEBUG_OBJECT (self, "parsing jpeg image data (%u bytes)", size);

  if (at_eos) {
    /* flush everything that is left; leftover data without a header is
     * dropped instead of decoded */
    toadd = size;

    if (!self->saw_header)
      goto drop_frame;
    goto have_full_frame;
  }

  if (size < 8)
    goto need_more_data;

  if (!self->saw_header) {
    /* expect the start marker first, skip anything before it */
    gint ret = gst_adapter_masked_scan_uint32 (adapter, 0xffff0000,
        0xffd80000, 0, size - 4);

    if (ret < 0)
      goto need_more_data;

    if (ret) {
      gst_adapter_flush (adapter, ret);
      size -= ret;
    }
    self->saw_header = TRUE;
  }

  while (1) {
    guint frame_len;
    guint32 value;

    noffset = gst_adapter_masked_scan_uint32_peek (adapter, 0x0000ff00,
        0x0000ff00, offset, size - offset, &value);

    /* lost sync if the 0xff marker is not where expected */
    if ((resync = (noffset != offset))) {
      GST_DEBUG_OBJECT (self, "lost sync at 0x%08x, resyncing", offset + 2);
    }
    /* may have a marker, but could have been resyncing */
    resync = resync || self->parse_resync;
    /* skip over extra 0xff */
    while ((noffset >= 0) && ((value & 0xff) == 0xff)) {
      noffset++;
      noffset = gst_adapter_masked_scan_uint32_peek (adapter, 0x0000ff00,
          0x0000ff00, noffset, size - noffset, &value);
    }
    /* enough bytes left for the marker payload? */
    if (noffset < 0)
      goto need_more_data;

    offset = noffset;
    value = value & 0xff;
    if (value == 0xd9) {
      GST_DEBUG_OBJECT (self, "0x%08x: EOI marker", offset + 2);
      self->saw_header = FALSE;
      self->parse_resync = FALSE;
      toadd = offset + 4;
      goto have_full_frame;
    }
    if (value == 0xd8) {
      GST_DEBUG_OBJECT (self, "0x%08x: SOI marker before EOI marker",
          offset + 2);
      self->saw_header = FALSE;
      self->parse_resync = FALSE;
      toadd = offset;
      goto have_full_frame;
    }

    if (value >= 0xd0 && value <= 0xd7) {
      frame_len = 0;
    } else {
      /* peek tag and subsequent length */
      if (offset + 2 + 4 > size)
        goto need_more_data;
      else
        gst_adapter_masked_scan_uint32_peek (adapter, 0x0, 0x0, offset + 2, 4,
            &frame_len);
      frame_len = frame_len & 0xffff;
    }
    /* the frame length includes the 2 length bytes themselves; here at
     * least 2 more bytes are wanted at the end for a marker */
    if (offset + 2 + 2 + frame_len + 2 > size)
      goto need_more_data;

    if (gst_mpp_jpeg_dec_parse_tag_has_entropy_segment (value)) {
      guint eseglen = self->parse_entropy_len;

      if (size < offset + 2 + frame_len + eseglen)
        goto need_more_data;
      noffset = offset + 2 + frame_len + self->parse_entropy_len;
      while (1) {
        noffset = gst_adapter_masked_scan_uint32_peek (adapter, 0x0000ff00,
            0x0000ff00, noffset, size - noffset, &value);
        if (noffset < 0) {
          /* need more data */
          self->parse_entropy_len = size - offset - 4 - frame_len - 2;
          goto need_more_data;
        }
        if ((value & 0xff) != 0x00) {
          eseglen = noffset - offset - frame_len - 2;
          break;
        }
        noffset++;
      }
      self->parse_entropy_len = 0;
      frame_len += eseglen;
      GST_DEBUG_OBJECT (self, "entropy segment length %u => frame_len %u",
          eseglen, frame_len);
    }

    if (resync) {
      /* check if sync would hold when interpreting this as a sync point
       * and skipping this frame */
      noffset = offset + frame_len + 2;
      noffset = gst_adapter_masked_scan_uint32 (adapter, 0x0000ff00,
          0x0000ff00, noffset, 4);
      if (noffset < 0) {
        /* keep resyncing until the data runs out or a plausible sync
         * point is found */
        offset++;
        continue;
      }
    }

    /* add the current segment and move on */
    toadd += frame_len + 2;
    offset += frame_len + 2;
  }

need_more_data:
  if (toadd)
    gst_video_decoder_add_to_frame (bdec, toadd);
  return GST_VIDEO_DECODER_FLOW_NEED_DATA;

have_full_frame:
  if (toadd)
    gst_video_decoder_add_to_frame (bdec, toadd);
  GST_VIDEO_CODEC_FRAME_SET_SYNC_POINT (frame);
  return gst_video_decoder_have_frame (bdec);

drop_frame:
  gst_adapter_flush (adapter, size);
  return GST_FLOW_OK;
}

static MppPacket
gst_mpp_jpeg_dec_get_mpp_packet (GstVideoDecoder * decoder,
    GstMapInfo * mapinfo)
{
  GstMppJpegDec *self = GST_MPP_JPEG_DEC (decoder);
  GstMppDec *mppdec = GST_MPP_DEC (decoder);
  MppBuffer mbuf = NULL;
  MppFrame mframe;
  MppPacket mpkt;
  MppMeta meta;

  mpp_buffer_get (self->input_group, &mbuf, mapinfo->size);
  if (G_UNLIKELY (!mbuf))
    return NULL;

  /* FIXME: performance bad */
  memcpy (mpp_buffer_get_ptr (mbuf), mapinfo->data, mapinfo->size);

  mpp_packet_init_with_buffer (&mpkt, mbuf);
  mpp_buffer_put (mbuf);
  if (G_UNLIKELY (!mpkt))
    return NULL;

  mpp_packet_set_size (mpkt, mapinfo->size);
  mpp_packet_set_length (mpkt, mapinfo->size);

  /* Create the output frame */

  mbuf = gst_mpp_allocator_alloc_mppbuf (mppdec->allocator, self->buf_size);
  if (G_UNLIKELY (!mbuf)) {
    mpp_packet_deinit (&mpkt);
    return NULL;
  }

  mpp_frame_init (&mframe);
  mpp_frame_set_buffer (mframe, mbuf);
  mpp_buffer_put (mbuf);

  meta = mpp_packet_get_meta (mpkt);
  mpp_meta_set_frame (meta, KEY_OUTPUT_FRAME, mframe);

  return mpkt;
}

static void
gst_mpp_jpeg_dec_drain_output (GstVideoDecoder * decoder)
{
  GstMppDec *mppdec = GST_MPP_DEC (decoder);
  gint timeout_ms = MPP_TIMEOUT_NON_BLOCK;

  /* Leftover output frames carry our input packet as KEY_INPUT_PACKET
   * meta.  mpi->reset() discards queued frames without releasing the
   * attached packets, so poll them out here and release both. */
  mppdec->mpi->control (mppdec->mpp_ctx, MPP_SET_OUTPUT_TIMEOUT, &timeout_ms);

  while (1) {
    MppMeta meta;
    MppPacket mpkt = NULL;
    MppFrame mframe = NULL;

    mppdec->mpi->decode_get_frame (mppdec->mpp_ctx, &mframe);
    if (!mframe)
      break;

    meta = mpp_frame_get_meta (mframe);
    if (meta)
      mpp_meta_get_packet (meta, KEY_INPUT_PACKET, &mpkt);

    if (mpkt) {
      mpp_meta_set_packet (meta, KEY_INPUT_PACKET, NULL);
      mpp_packet_deinit (&mpkt);
    }

    mpp_frame_deinit (&mframe);
  }
}

static gboolean
gst_mpp_jpeg_dec_shutdown (GstVideoDecoder * decoder, gboolean drain)
{
  GstMppJpegDec *self = GST_MPP_JPEG_DEC (decoder);
  GstMppDec *mppdec = GST_MPP_DEC (decoder);
  MppFrame mframe;
  MppPacket mpkt;
  MppBuffer mbuf;
  MppMeta meta;
  MPP_RET ret;
  gint64 deadline =
      g_get_monotonic_time () + GST_MPP_DEC_STALL_TIMEOUT_MS * 1000;

  /* It's safe to stop decoding immediately */
  if (!drain) {
    gst_mpp_jpeg_dec_drain_output (decoder);
    mppdec->mpi->reset (mppdec->mpp_ctx);
    return FALSE;
  }

  GST_DEBUG_OBJECT (self, "sending EOS");

  /* Prepare EOS packet */
  mpp_buffer_get (self->input_group, &mbuf, 1);
  mpp_packet_init_with_buffer (&mpkt, mbuf);
  mpp_buffer_put (mbuf);
  mpp_packet_set_size (mpkt, 0);
  mpp_packet_set_length (mpkt, 0);
  mpp_packet_set_eos (mpkt);

  mpp_frame_init (&mframe);
  meta = mpp_packet_get_meta (mpkt);
  mpp_meta_set_frame (meta, KEY_OUTPUT_FRAME, mframe);

  while (1) {
    ret = mppdec->mpi->decode_put_packet (mppdec->mpp_ctx, mpkt);
    if (!ret)
      break;

    /* A wedged decoder pipeline never takes the EOS packet; stop
     * waiting and reset instead of blocking the state change */
    if (g_get_monotonic_time () > deadline) {
      GST_WARNING_OBJECT (self, "decoder stalled draining; resetting");
      mppdec->mpi->reset (mppdec->mpp_ctx);
      /* The reset flushed the packet back to us: it was never consumed
       * by the MPP worker, so we still own it. */
      mpp_packet_deinit (&mpkt);
      return FALSE;
    }

    g_usleep (1000);
  }

  /* NOTE: the EOS packet is left undeinited on purpose; its buffer and
   * output frame stay owned by the MPP worker task until it finishes */
  return TRUE;
}

#define GST_TYPE_MPP_JPEG_DEC_FORMAT (gst_mpp_jpeg_dec_format_get_type ())
static GType
gst_mpp_jpeg_dec_format_get_type (void)
{
  static GType format = 0;

  if (!format) {
    static const GEnumValue formats[] = {
      {GST_VIDEO_FORMAT_UNKNOWN, "Auto", "auto"},
      {GST_VIDEO_FORMAT_NV12, "NV12", "NV12"},
#ifdef HAVE_RGA
      {GST_VIDEO_FORMAT_NV21, "NV21", "NV21"},
      {GST_VIDEO_FORMAT_I420, "I420", "I420"},
      {GST_VIDEO_FORMAT_YV12, "YV12", "YV12"},
      {GST_VIDEO_FORMAT_NV16, "NV16", "NV16"},
      {GST_VIDEO_FORMAT_NV61, "NV61", "NV61"},
      {GST_VIDEO_FORMAT_RGB, "RGB", "RGB"},
      {GST_VIDEO_FORMAT_BGR, "BGR", "BGR"},
#endif
      {GST_VIDEO_FORMAT_RGB16, "RGB565", "RGB16"},
      {GST_VIDEO_FORMAT_BGR16, "BGR565", "BGR16"},
      {GST_VIDEO_FORMAT_ARGB, "ARGB8888", "ARGB"},
      {GST_VIDEO_FORMAT_ABGR, "ABGR8888", "ABGR"},
      {GST_VIDEO_FORMAT_RGBA, "RGBA8888", "RGBA"},
      {GST_VIDEO_FORMAT_BGRA, "BGRA8888", "BGRA"},
      {GST_VIDEO_FORMAT_xRGB, "XRGB8888", "xRGB"},
      {GST_VIDEO_FORMAT_xBGR, "XBGR8888", "xBGR"},
      {GST_VIDEO_FORMAT_RGBx, "RGBX8888", "RGBx"},
      {GST_VIDEO_FORMAT_BGRx, "BGRX8888", "BGRx"},
      {0, NULL, NULL}
    };
    format = g_enum_register_static ("GstMppJpegDecFormat", formats);
  }
  return format;
}

static void
gst_mpp_jpeg_dec_init (GstMppJpegDec * self)
{
  GstMppDec *mppdec = GST_MPP_DEC (self);
  mppdec->format = DEFAULT_PROP_FORMAT;
}

static void
gst_mpp_jpeg_dec_setup_default_format (void)
{
  GEnumClass *class;
  GEnumValue *value;
  const gchar *env;

  env = g_getenv ("GST_MPP_JPEGDEC_DEFAULT_FORMAT");
  if (!env)
    return;

  class = g_type_class_ref (GST_TYPE_MPP_JPEG_DEC_FORMAT);

  value = g_enum_get_value_by_nick (class, env);
  if (value)
    DEFAULT_PROP_FORMAT = value->value;

  g_type_class_unref (class);
}

static void
gst_mpp_jpeg_dec_class_init (GstMppJpegDecClass * klass)
{
  GstVideoDecoderClass *decoder_class = GST_VIDEO_DECODER_CLASS (klass);
  GstMppDecClass *pclass = GST_MPP_DEC_CLASS (klass);
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, "mppjpegdec", 0,
      "MPP JPEG decoder");

  decoder_class->start = GST_DEBUG_FUNCPTR (gst_mpp_jpeg_dec_start);
  decoder_class->flush = GST_DEBUG_FUNCPTR (gst_mpp_jpeg_dec_flush);
  decoder_class->stop = GST_DEBUG_FUNCPTR (gst_mpp_jpeg_dec_stop);
  decoder_class->set_format = GST_DEBUG_FUNCPTR (gst_mpp_jpeg_dec_set_format);
  decoder_class->parse = GST_DEBUG_FUNCPTR (gst_mpp_jpeg_dec_parse);

  pclass->startup = NULL;
  pclass->get_mpp_packet = GST_DEBUG_FUNCPTR (gst_mpp_jpeg_dec_get_mpp_packet);
  pclass->shutdown = GST_DEBUG_FUNCPTR (gst_mpp_jpeg_dec_shutdown);

  gobject_class->set_property =
      GST_DEBUG_FUNCPTR (gst_mpp_jpeg_dec_set_property);
  gobject_class->get_property =
      GST_DEBUG_FUNCPTR (gst_mpp_jpeg_dec_get_property);

  gst_mpp_jpeg_dec_setup_default_format ();

  g_object_class_install_property (gobject_class, PROP_FORMAT,
      g_param_spec_enum ("format", "Prefered output format",
          "Prefered output format",
          GST_TYPE_MPP_JPEG_DEC_FORMAT, DEFAULT_PROP_FORMAT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_mpp_jpeg_dec_src_template));

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_mpp_jpeg_dec_sink_template));

  gst_element_class_set_static_metadata (element_class,
      "Rockchip's MPP JPEG image decoder", "Decoder/Image",
      "JPEG hardware decoder",
      "Randy Li <randy.li@rock-chips.com>, "
      "Jeffy Chen <jeffy.chen@rock-chips.com>");
}

gboolean
gst_mpp_jpeg_dec_register (GstPlugin * plugin, guint rank)
{
  return gst_element_register (plugin, "mppjpegdec", rank,
      gst_mpp_jpeg_dec_get_type ());
}
