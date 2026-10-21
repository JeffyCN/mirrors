/*
 * Copyright 2026 Rockchip Electronics Co., Ltd
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

/*
 * MPP frame format conversion via the Mali GPU (GLES2 + EGL dma-buf).
 *
 * Second conversion tier, used when RGA is missing or cannot handle the
 * format (e.g. 10-bit HEVC -> NV12, H.264 4:2:2 -> NV12). The source
 * dmabuf is imported as two single-component EGLImages (luma and the
 * interleaved chroma row), rendered through a BT.601 limited-range
 * fragment shader into an intermediate RGBA framebuffer, read back and
 * packed into the destination YUV dmabuf by the CPU. The final pack is
 * cheap (4K@30 takes a few hundred us on a single A76 core), and this
 * tier is only meant for content which RGA cannot process.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include "gstmpp.h"

#ifndef HAVE_GPU

/* GPU tier not built: keep the entry points so the decoder and the
 * output-format property wiring compile unchanged, and report the tier
 * as unavailable. */
gboolean
gst_mpp_use_gpu (void)
{
  return FALSE;
}

gboolean
gst_mpp_gpu_convert_from_mpp_frame (MppFrame mframe, GstMemory * out_mem,
    GstVideoInfo * dst_vinfo, gint rotation)
{
  (void) mframe;
  (void) out_mem;
  (void) dst_vinfo;
  (void) rotation;

  return FALSE;
}

#else

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include <gbm.h>
#include <xf86drm.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <drm_fourcc.h>

GST_DEBUG_CATEGORY_STATIC (mpp_gpu_debug);
#define GST_CAT_DEFAULT mpp_gpu_debug

/* eglGetProcAddress is not free and these never change: resolve once */
static PFNEGLCREATEIMAGEKHRPROC p_create_image;
static PFNEGLDESTROYIMAGEKHRPROC p_destroy_image;
static PFNGLEGLIMAGETARGETTEXTURE2DOESPROC p_image_target;

static void
gst_mpp_gpu_load_procs (void)
{
  if (p_create_image)
    return;

  p_create_image =
      (PFNEGLCREATEIMAGEKHRPROC) eglGetProcAddress ("eglCreateImageKHR");
  p_destroy_image =
      (PFNEGLDESTROYIMAGEKHRPROC) eglGetProcAddress ("eglDestroyImageKHR");
  p_image_target = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)
      eglGetProcAddress ("glEGLImageTargetTexture2DOES");
}

#define EGL_LOAD_PROC(val, type, func) \
    do { val = (type) eglGetProcAddress (func); } while (0)

/*
 * Plane-based YUV import. Mali's samplerExternalOES only converts the
 * 4:2:0 family, an NV16 image imports fine but samples black, and
 * two-component GR88 imports are rejected outright -- single-component
 * R8 imports do work. So bind luma and the interleaved chroma row as
 * two single-plane R8 images (the chroma texel grid spans the whole
 * CbCr row: one texel == one byte) and reconstruct Cb/Cr in the shader
 * by sampling one texel apart. BT.601 limited-range YUV -> RGB.
 *
 * NOTE on rkvdec H.264 4:2:2 (MPP_FMT_YUV422SP): the buffer is standard
 * NV16, the chroma plane hor_stride bytes wide and ver_stride rows tall
 * (one chroma row per luma row), as libv4l-rkmpp 14b5d17 verified
 * against a software decode on RK3588 and RK3576.
 */
static const char *vs_src =
    "attribute vec4 position;\n"
    "uniform int rotation;\n"
    "varying vec2 texcoord;\n"
    "void main() {\n"
    "   vec2 pos = position.xy;\n"
    "   if (rotation == 1) {\n"
    "       pos = vec2(pos.y, -pos.x);\n"
    "   } else if (rotation == 2) {\n"
    "       pos = vec2(-pos.x, -pos.y);\n"
    "   } else if (rotation == 3) {\n"
    "       pos = vec2(-pos.y, pos.x);\n"
    "   }\n" "   gl_Position = vec4(pos, 0.0, 1.0);\n"
    /* No y-flip here: the framebuffer origin is the first dmabuf row,
     * which already mirrors the image into the top-down YUV layout the
     * CPU pack below expects. Flipping the texture coordinate as well
     * would output the frame upside down. */
    "   vec2 orig_tex = pos.xy * 0.5 + 0.5;\n"
    "   if (rotation == 1) {\n"
    "       texcoord = vec2(orig_tex.y, 1.0 - orig_tex.x);\n"
    "   } else if (rotation == 2) {\n"
    "       texcoord = vec2(1.0 - orig_tex.x, 1.0 - orig_tex.y);\n"
    "   } else if (rotation == 3) {\n"
    "       texcoord = vec2(1.0 - orig_tex.y, orig_tex.x);\n"
    "   } else {\n" "       texcoord = orig_tex;\n" "   }\n" "}\n";

static const char *fs_src =
    "precision highp float;\n"
    "varying vec2 texcoord;\n"
    "uniform sampler2D tex_y;\n"
    "uniform sampler2D tex_c;\n"
    "uniform vec2 src_scale;\n" "uniform float chr_dx;\n" "uniform float vu;\n"
    "void main() {\n"
    /* texcoord spans the visible source area; chroma texels track luma
     * columns and the chroma rows cover the same physical height at
     * half density, so both planes share the same sampling position */
    "   vec2 t = texcoord * src_scale;\n"
    "   float y = (texture2D(tex_y, t).r - 0.0625) * 1.164;\n"
    "   float c1 = texture2D(tex_c, t).r - 0.5;\n"
    "   float c2 = texture2D(tex_c, t + vec2(chr_dx, 0.0)).r - 0.5;\n"
    "   float cb = mix(c1, c2, vu);\n"
    "   float cr = mix(c2, c1, vu);\n"
    "   float r = clamp(y + 1.596 * cr, 0.0, 1.0);\n"
    "   float g = clamp(y - 0.391 * cb - 0.813 * cr, 0.0, 1.0);\n"
    "   float b = clamp(y + 2.018 * cb, 0.0, 1.0);\n"
    "   gl_FragColor = vec4(r, g, b, 1.0);\n" "}\n";

/*
 * Compact 10-bit variant (MPP_FMT_YUV420SP_10BIT, the Rockchip packing
 * with 4 samples in 5 bytes). Mali advertises this layout as
 * DRM_FORMAT_NV15, so import the whole frame as one NV15 image and let
 * the hardware sampler do the unpack plus the BT.601 limited-range
 * YUV -> RGB conversion. The image width must be derived from the byte
 * stride (hor_stride / 10 * 8) for the sampler addressing to line up
 * with the packed rows -- same trick as the glamor XV dmabuf path.
 * The import carries REC601 / narrow-range hints so the sampler
 * expands limited-range Y like the shader above does for 8-bit.
 */
static const char *fs_src_c10 =
    "#extension GL_OES_EGL_image_external : require\n"
    "precision highp float;\n"
    "varying vec2 texcoord;\n"
    "uniform samplerExternalOES tex_y;\n"
    "uniform vec2 src_scale;\n"
    "void main() {\n"
    "   gl_FragColor = vec4(texture2D(tex_y, texcoord * src_scale).rgb, 1.0);\n"
    "}\n";

/*
 * Plane-path shaders: render straight into the dst Y (R8) and interleaved
 * UV (ABGR8888, one texel = two UV pairs) planes.  Both targets are really
 * color-renderable on the Mali DDK, unlike a whole-frame NV12 attachment
 * which reports a false FBO-complete and corrupts the output.
 */

/*
 * The plane and pass-B shaders below address dst pixels by index. GLSL
 * ES 1.00 declares gl_FragCoord mediump, and Mesa on Mali evaluates it in
 * fp16, which cannot hold pixel centres from 1024.5 on: those samples
 * land on texel edges and blend two texels.  Derive the index from the
 * highp texcoord varying instead; u_vp is the viewport size in pixels.
 */

/* 8-bit src: Y is a plain single-component copy */
static const char *fs_plane8_y =
    "precision highp float;\n"
    "varying vec2 texcoord;\n"
    "uniform sampler2D tex_y;\n"
    "uniform vec2 t_step;\n"
    "uniform vec2 u_vp;\n"
    "void main() {\n"
    "   vec2 t = (floor(texcoord * u_vp) + 0.5) * t_step;\n"
    "   float v = texture2D(tex_y, t).r;\n"
    "   gl_FragColor = vec4(v, v, v, 1.0);\n" "}\n";

/* 8-bit src: one dst texel = two interleaved UV pairs = four chroma
 * bytes, reassembled from the byte-granular R8 chroma view.  vu swaps
 * the Cb/Cr read order for VU-ordered sources, dvu swaps the write
 * order for NV21/NV61 destinations.
 *
 * Chroma row mapping, valid at any dst/src size ratio: dst chroma row
 * r sits at dst luma line Ld = u_rowmul * r + 0.5 (u_rowmul is 2 for
 * 4:2:0 dst, 1 for 4:2:2 dst); that line maps to src luma line
 * L = Ld * u_ratio_y and src chroma row floor(L * u_vhalf) (u_vhalf is
 * 0.5 for 4:2:0 src, 1 for 4:2:2 src whose chroma rows match the luma
 * rows); u_vs2 is one over the src chroma plane height in rows.
 *
 * Horizontal pair pick: both src families carry one chroma pair per two
 * luma columns, so the dst texel for luma columns 4tx..4tx+3 takes the
 * consecutive bytes 4tx..4tx+3.  At 1:1 the row mapping collapses onto
 * texel centres and the copy is bit exact; scaled output is
 * horizontally bilinear (texture filter) and vertically nearest-row,
 * and a 4:2:2 src into a 4:2:0 dst keeps the even chroma rows. */
static const char *fs_plane8_uv =
    "precision highp float;\n"
    "varying vec2 texcoord;\n"
    "uniform sampler2D tex_c;\n"
    "uniform vec2 u_vp;\n"
    "uniform float u_tx;\n"
    "uniform float u_ratio_x;\n"
    "uniform float u_ratio_y;\n"
    "uniform float u_rowmul;\n"
    "uniform float u_vhalf;\n"
    "uniform float u_vs2;\n"
    "uniform float vu;\n"
    "uniform float dvu;\n"
    "void main() {\n"
    "   vec2 px = floor(texcoord * u_vp);\n"
    "   float srow = floor((u_rowmul * px.y + 0.5) * u_ratio_y * u_vhalf);\n"
    "   float ty = (srow + 0.5) * u_vs2;\n"
    "   float bx = px.x * 4.0;\n"
    "   float b0 = texture2D(tex_c, vec2((bx + 0.5) * u_ratio_x * u_tx, ty)).r;\n"
    "   float b1 = texture2D(tex_c, vec2((bx + 1.5) * u_ratio_x * u_tx, ty)).r;\n"
    "   float b2 = texture2D(tex_c, vec2((bx + 2.5) * u_ratio_x * u_tx, ty)).r;\n"
    "   float b3 = texture2D(tex_c, vec2((bx + 3.5) * u_ratio_x * u_tx, ty)).r;\n"
    "   float cb0 = mix(b0, b1, vu);\n"
    "   float cr0 = mix(b1, b0, vu);\n"
    "   float cb1 = mix(b2, b3, vu);\n"
    "   float cr1 = mix(b3, b2, vu);\n"
    "   gl_FragColor = mix(vec4(cb0, cr0, cb1, cr1),\n"
    "                      vec4(cr0, cb0, cr1, cb1), dvu);\n" "}\n";

/*
 * RGB-intermediate pass-B shaders: read the intermediate RGBA texture
 * (tight dst-sized), convert RGB -> YUV BT.601 limited-range and render
 * into the dst planes (R8 Y, R8 single chroma, or ABGR8888 interleaved
 * pairs).  With these, every YUV dst format is reachable as
 * src -> RGB -> dst, all GPU side.
 */
static const char *fs_rgb2y =
    "#extension GL_OES_EGL_image_external : require\n"
    "precision highp float;\n"
    "varying vec2 texcoord;\n"
    "uniform samplerExternalOES tex;\n"
    "uniform vec2 t_step;\n"
    "uniform vec2 u_vp;\n"
    "void main() {\n"
    "   vec3 c = texture2D(tex, (floor(texcoord * u_vp) + 0.5) * t_step).rgb;\n"
    "   float y = 0.2568 * c.r + 0.5041 * c.g + 0.0979 * c.b + 0.0625;\n"
    "   gl_FragColor = vec4(y, y, y, 1.0);\n" "}\n";

static const char *fs_rgb2uvc =
    "#extension GL_OES_EGL_image_external : require\n"
    "precision highp float;\n"
    "varying vec2 texcoord;\n"
    "uniform samplerExternalOES tex;\n"
    "uniform vec2 px_step;\n"
    "uniform vec2 u_vp;\n"
    "uniform float row_step;\n"
    "uniform float swap;\n"
    "void main() {\n"
    "   vec2 px = floor(texcoord * u_vp);\n"
    "   float base = px.x * 4.0;\n"
    "   float py = (px.y * row_step + 0.5) * px_step.y;\n"
    "   vec3 c0 = texture2D(tex, vec2((base + 0.5) * px_step.x, py)).rgb;\n"
    "   vec3 c1 = texture2D(tex, vec2((base + 2.5) * px_step.x, py)).rgb;\n"
    "   float u0 = -0.1482 * c0.r - 0.2910 * c0.g + 0.4392 * c0.b + 0.5020;\n"
    "   float v0 = 0.4392 * c0.r - 0.3678 * c0.g - 0.0714 * c0.b + 0.5020;\n"
    "   float u1 = -0.1482 * c1.r - 0.2910 * c1.g + 0.4392 * c1.b + 0.5020;\n"
    "   float v1 = 0.4392 * c1.r - 0.3678 * c1.g - 0.0714 * c1.b + 0.5020;\n"
    "   gl_FragColor = mix(vec4(u0, v0, u1, v1), vec4(v0, u0, v1, u1), swap);\n"
    "}\n";

static const char *fs_rgb2c =
    "#extension GL_OES_EGL_image_external : require\n"
    "precision highp float;\n"
    "varying vec2 texcoord;\n"
    "uniform samplerExternalOES tex;\n"
    "uniform vec2 px_step;\n"
    "uniform vec2 u_vp;\n"
    "uniform float row_step;\n"
    "uniform float sel;\n"
    "void main() {\n"
    "   vec2 px = floor(texcoord * u_vp);\n"
    "   vec3 c = texture2D(tex, vec2((px.x * 2.0 + 0.5) * px_step.x,\n"
    "                                (px.y * row_step + 0.5) * px_step.y)).rgb;\n"
    "   float u = -0.1482 * c.r - 0.2910 * c.g + 0.4392 * c.b + 0.5020;\n"
    "   float v = 0.4392 * c.r - 0.3678 * c.g - 0.0714 * c.b + 0.5020;\n"
    "   gl_FragColor = vec4(mix(u, v, sel), 0.0, 0.0, 1.0);\n" "}\n";

/* 10-bit compact src: the OES sampler already converts to RGB; go back
 * through the BT.601 limited-range inverse for the Y plane */
static const char *fs_plane10_y =
    "#extension GL_OES_EGL_image_external : require\n"
    "precision highp float;\n"
    "varying vec2 texcoord;\n"
    "uniform samplerExternalOES tex_y;\n"
    "uniform vec2 src_scale;\n"
    "void main() {\n"
    "   vec3 c = texture2D(tex_y, texcoord * src_scale).rgb;\n"
    "   float yv = 0.2568 * c.r + 0.5041 * c.g + 0.0979 * c.b + 0.0625;\n"
    "   gl_FragColor = vec4(yv, yv, yv, 1.0);\n" "}\n";

/* 10-bit compact src: one dst texel = two UV pairs = a 4-pixel span;
 * each pair samples the OES at its first pixel centre (the chroma
 * sample shared by the pair) and converts to Cb/Cr.  px_step is in
 * dst-normalized units so scaling comes free; u_rowmul is 2 for 4:2:0
 * dst, 1 for 4:2:2 dst; dvu swaps Cb/Cr for NV21/NV61. */
static const char *fs_plane10_uv =
    "#extension GL_OES_EGL_image_external : require\n"
    "precision highp float;\n"
    "varying vec2 texcoord;\n"
    "uniform samplerExternalOES tex_y;\n"
    "uniform vec2 src_scale;\n"
    "uniform vec2 px_step;\n"
    "uniform vec2 u_vp;\n"
    "uniform float u_rowmul;\n"
    "uniform float dvu;\n"
    "void main() {\n"
    "   vec2 px = floor(texcoord * u_vp);\n"
    "   float base = px.x * 4.0;\n"
    "   float py = (u_rowmul * px.y + 0.5) * px_step.y;\n"
    "   vec3 c0 = texture2D(tex_y, vec2((base + 0.5) * px_step.x, py) * src_scale).rgb;\n"
    "   vec3 c1 = texture2D(tex_y, vec2((base + 2.5) * px_step.x, py) * src_scale).rgb;\n"
    "   float u0 = -0.1482 * c0.r - 0.2910 * c0.g + 0.4392 * c0.b + 0.5020;\n"
    "   float v0 = 0.4392 * c0.r - 0.3678 * c0.g - 0.0714 * c0.b + 0.5020;\n"
    "   float u1 = -0.1482 * c1.r - 0.2910 * c1.g + 0.4392 * c1.b + 0.5020;\n"
    "   float v1 = 0.4392 * c1.r - 0.3678 * c1.g - 0.0714 * c1.b + 0.5020;\n"
    "   gl_FragColor = mix(vec4(u0, v0, u1, v1), vec4(v0, u0, v1, u1), dvu);\n"
    "}\n";

struct _GstMppGpuCtx
{
  GMutex lock;

  gboolean inited;
  /* Init-time hard failure (eglInitialize, context, shader) */
  gboolean unavailable;
  /* Per-format latch: an import failure must not disable the GPU tier
   * for other format pairs in the same process */
  MppFrameFormat unavailable_src_fmt;
  GstVideoFormat unavailable_dst_fmt;
  /* Same latch, for the direct (render into dst dmabuf) path */
  MppFrameFormat unavailable_direct_src_fmt;
  GstVideoFormat unavailable_direct_dst_fmt;
  /* Same latch, for the plane path (render into dst Y/UV planes) */
  MppFrameFormat unavailable_planes_src_fmt;
  GstVideoFormat unavailable_planes_dst_fmt;
  /* Same latch, for the via-RGB two-pass path */
  MppFrameFormat unavailable_via_src_fmt;
  GstVideoFormat unavailable_via_dst_fmt;

  gint drm_fd;
  struct gbm_device *gbm;
  EGLDisplay display;
  EGLContext context;

  GLuint program;
  GLuint vs, fs;
  GLuint program_c10;
  GLuint program_plane8_y, program_plane8_uv;
  GLuint program_plane10_y, program_plane10_uv;
  GLint plane8_uv_vu_loc;
  GLint plane8_uv_dvu_loc;
  GLint plane8_uv_tx_loc;
  GLint plane8_uv_ratio_x_loc;
  GLint plane8_uv_ratio_y_loc;
  GLint plane8_uv_rowmul_loc;
  GLint plane8_uv_vhalf_loc;
  GLint plane8_uv_vs2_loc;
  GLint plane8_y_t_step_loc;
  GLint plane10_y_src_scale_loc;
  GLint plane10_uv_src_scale_loc;
  GLint plane10_uv_px_step_loc;
  GLint plane10_uv_rowmul_loc;
  GLint plane10_uv_dvu_loc;
  GLint rgb2y_t_step_loc;
  GLint rgb2uvc_px_step_loc;
  GLint rgb2uvc_row_step_loc;
  GLint rgb2uvc_swap_loc;
  GLint rgb2c_px_step_loc;
  GLint rgb2c_row_step_loc;
  GLint rgb2c_sel_loc;
  GLint pos_plane8_y_loc;
  GLint pos_plane8_uv_loc;
  GLint pos_plane10_y_loc;
  GLint pos_plane10_uv_loc;
  GLint pos_rgb2y_loc;
  GLint pos_rgb2uvc_loc;
  GLint pos_rgb2c_loc;
  GLuint program_rgb2y, program_rgb2uvc, program_rgb2c;
  GLuint vbo;
  GLint pos_loc;
  GLint pos_c10_loc;
  GLint src_scale_loc;
  GLint chr_dx_loc;
  GLint vu_loc;
  GLint src_scale_c10_loc;
  GLint rot_loc;
  GLint rot_c10_loc;

  GLuint src_tex[2];
  GLuint src_tex_oes;
  GLuint dst_tex;
  GLuint dst_fbo;
  GLuint dst_w, dst_h;
  guint8 *rgba;

  /* RGB intermediate (GBM dmabuf) for the YUV-dst two-pass path */
  struct gbm_bo *rgb_bo;
  gint rgb_fd;
  GLuint rgb_itex;              /* imported once, cached with the bo */
  gint rgb_w, rgb_h;
};

static struct _GstMppGpuCtx gctx = {
  .unavailable_src_fmt = MPP_FMT_BUTT,
  .unavailable_dst_fmt = GST_VIDEO_FORMAT_UNKNOWN,
  .unavailable_direct_src_fmt = MPP_FMT_BUTT,
  .unavailable_direct_dst_fmt = GST_VIDEO_FORMAT_UNKNOWN,
  .unavailable_planes_src_fmt = MPP_FMT_BUTT,
  .unavailable_planes_dst_fmt = GST_VIDEO_FORMAT_UNKNOWN,
  .unavailable_via_src_fmt = MPP_FMT_BUTT,
  .unavailable_via_dst_fmt = GST_VIDEO_FORMAT_UNKNOWN,
};

static GLuint
gst_mpp_gpu_compile_shader (GLenum type, const char *src)
{
  GLuint s = glCreateShader (type);
  GLint ok;
  char msg[512];

  glShaderSource (s, 1, &src, NULL);
  glCompileShader (s);
  glGetShaderiv (s, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    glGetShaderInfoLog (s, sizeof (msg), NULL, msg);
    GST_WARNING ("shader compile failed: %s", msg);
    glDeleteShader (s);
    return 0;
  }

  return s;
}

/* Build a program sharing the common vertex shader */
static GLuint
gst_mpp_gpu_build_program (const char *fs_src)
{
  GLuint fs, program;
  GLint v;
  char msg[512];

  fs = gst_mpp_gpu_compile_shader (GL_FRAGMENT_SHADER, fs_src);
  if (!fs)
    return 0;

  program = glCreateProgram ();
  glAttachShader (program, gctx.vs);
  glAttachShader (program, fs);
  glLinkProgram (program);
  glDeleteShader (fs);
  glGetProgramiv (program, GL_LINK_STATUS, &v);
  if (!v) {
    glGetProgramInfoLog (program, sizeof (msg), NULL, msg);
    GST_WARNING ("program link failed: %s", msg);
    glDeleteProgram (program);
    return 0;
  }
  return program;
}

/* Open the render node of a Mali DRM driver. On RK3576 renderD128 is
 * the display controller; its primary node is denied to sandboxed
 * services and sends Mesa down the kmsro path. Fall back to the
 * Rockchip DDK setup, where the GPU has no DRM driver. */
static gint
gst_mpp_gpu_open_drm (void)
{
  drmDevicePtr devices[16];
  gint i, n, fd = -1;

  n = drmGetDevices2 (0, devices, G_N_ELEMENTS (devices));
  for (i = 0; i < n && fd < 0; i++) {
    drmVersionPtr ver;

    if (!(devices[i]->available_nodes & (1 << DRM_NODE_RENDER)))
      continue;

    fd = open (devices[i]->nodes[DRM_NODE_RENDER], O_RDWR | O_CLOEXEC);
    if (fd < 0)
      continue;

    ver = drmGetVersion (fd);
    if (!ver || (strcmp (ver->name, "panfrost") &&
            strcmp (ver->name, "panthor"))) {
      close (fd);
      fd = -1;
    }
    drmFreeVersion (ver);
  }
  if (n > 0)
    drmFreeDevices (devices, n);

  if (fd < 0)
    fd = drmOpen ("rockchip", NULL);
  if (fd < 0)
    fd = open ("/dev/dri/renderD128", O_RDWR | O_CLOEXEC);

  return fd;
}

static gboolean
gst_mpp_gpu_init (void)
{
  PFNEGLGETPLATFORMDISPLAYEXTPROC get_platform_display;
  EGLint context_attrs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
  static const GLfloat quad[4][4] = {
    {-1.0f, -1.0f, 0.0f, 1.0f},
    {1.0f, -1.0f, 0.0f, 1.0f},
    {-1.0f, 1.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 0.0f, 1.0f},
  };
  GLint v;
  gint i;

  if (gctx.inited)
    return TRUE;

  if (gctx.unavailable)
    return FALSE;

  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, "mppgpu", 0, "MPP GPU converter");

  get_platform_display = (PFNEGLGETPLATFORMDISPLAYEXTPROC)
      eglGetProcAddress ("eglGetPlatformDisplayEXT");
  if (!get_platform_display)
    goto err;

  gctx.drm_fd = gst_mpp_gpu_open_drm ();
  if (gctx.drm_fd < 0)
    goto err;

  gctx.gbm = gbm_create_device (gctx.drm_fd);
  if (!gctx.gbm)
    goto err;

  gctx.display =
      get_platform_display (EGL_PLATFORM_GBM_KHR, (void *) gctx.gbm, NULL);
  if (gctx.display == EGL_NO_DISPLAY)
    goto err;

  if (!eglInitialize (gctx.display, NULL, NULL))
    goto err;

  if (!eglBindAPI (EGL_OPENGL_ES_API))
    goto err;

  gctx.context =
      eglCreateContext (gctx.display, NULL, EGL_NO_CONTEXT, context_attrs);
  if (gctx.context == EGL_NO_CONTEXT)
    goto err;

  if (!eglMakeCurrent (gctx.display, EGL_NO_SURFACE, EGL_NO_SURFACE,
          gctx.context))
    goto err;

  gctx.vs = gst_mpp_gpu_compile_shader (GL_VERTEX_SHADER, vs_src);
  gctx.fs = gst_mpp_gpu_compile_shader (GL_FRAGMENT_SHADER, fs_src);
  if (!gctx.vs || !gctx.fs)
    goto err;

  gctx.program = glCreateProgram ();
  glAttachShader (gctx.program, gctx.vs);
  glAttachShader (gctx.program, gctx.fs);
  glLinkProgram (gctx.program);
  glDeleteShader (gctx.fs);
  glGetProgramiv (gctx.program, GL_LINK_STATUS, &v);
  if (!v)
    goto err;

  gctx.fs = gst_mpp_gpu_compile_shader (GL_FRAGMENT_SHADER, fs_src_c10);
  if (!gctx.fs)
    goto err;

  gctx.program_c10 = glCreateProgram ();
  glAttachShader (gctx.program_c10, gctx.vs);
  glAttachShader (gctx.program_c10, gctx.fs);
  glLinkProgram (gctx.program_c10);
  glDeleteShader (gctx.fs);
  glGetProgramiv (gctx.program_c10, GL_LINK_STATUS, &v);
  if (!v)
    goto err;

  glGenTextures (2, gctx.src_tex);
  for (i = 0; i < 2; i++) {
    glBindTexture (GL_TEXTURE_2D, gctx.src_tex[i]);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  glGenTextures (1, &gctx.src_tex_oes);
  glBindTexture (GL_TEXTURE_EXTERNAL_OES, gctx.src_tex_oes);
  glTexParameteri (GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S,
      GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T,
      GL_CLAMP_TO_EDGE);

  glGenFramebuffers (1, &gctx.dst_fbo);

  /* Fullscreen quad for glDrawArrays. Without this the position
   * attribute stays at its default constant value, every vertex
   * collapses to the same point and nothing is ever rasterized. */
  gctx.pos_loc = glGetAttribLocation (gctx.program, "position");
  gctx.pos_c10_loc = glGetAttribLocation (gctx.program_c10, "position");
  glGenBuffers (1, &gctx.vbo);
  glBindBuffer (GL_ARRAY_BUFFER, gctx.vbo);
  glBufferData (GL_ARRAY_BUFFER, sizeof (quad), quad, GL_STATIC_DRAW);
  glBindBuffer (GL_ARRAY_BUFFER, 0);

  glUseProgram (gctx.program);
  glUniform1i (glGetUniformLocation (gctx.program, "tex_y"), 0);
  glUniform1i (glGetUniformLocation (gctx.program, "tex_c"), 1);
  gctx.src_scale_loc = glGetUniformLocation (gctx.program, "src_scale");
  gctx.chr_dx_loc = glGetUniformLocation (gctx.program, "chr_dx");
  gctx.vu_loc = glGetUniformLocation (gctx.program, "vu");

  glUseProgram (gctx.program_c10);
  glUniform1i (glGetUniformLocation (gctx.program_c10, "tex_y"), 0);
  gctx.src_scale_c10_loc = glGetUniformLocation (gctx.program_c10, "src_scale");
  gctx.rot_loc = glGetUniformLocation (gctx.program, "rotation");
  gctx.rot_c10_loc = glGetUniformLocation (gctx.program_c10, "rotation");

  gctx.program_plane8_y = gst_mpp_gpu_build_program (fs_plane8_y);
  gctx.program_plane8_uv = gst_mpp_gpu_build_program (fs_plane8_uv);
  gctx.program_plane10_y = gst_mpp_gpu_build_program (fs_plane10_y);
  gctx.program_plane10_uv = gst_mpp_gpu_build_program (fs_plane10_uv);
  if (!gctx.program_plane8_y || !gctx.program_plane8_uv ||
      !gctx.program_plane10_y || !gctx.program_plane10_uv)
    goto err;

  glUseProgram (gctx.program_plane8_uv);
  glUniform1i (glGetUniformLocation (gctx.program_plane8_uv, "tex_c"), 0);
  gctx.plane8_uv_vu_loc = glGetUniformLocation (gctx.program_plane8_uv, "vu");
  gctx.plane8_uv_dvu_loc = glGetUniformLocation (gctx.program_plane8_uv, "dvu");
  gctx.plane8_uv_tx_loc = glGetUniformLocation (gctx.program_plane8_uv, "u_tx");
  gctx.plane8_uv_ratio_x_loc =
      glGetUniformLocation (gctx.program_plane8_uv, "u_ratio_x");
  gctx.plane8_uv_ratio_y_loc =
      glGetUniformLocation (gctx.program_plane8_uv, "u_ratio_y");
  gctx.plane8_uv_rowmul_loc =
      glGetUniformLocation (gctx.program_plane8_uv, "u_rowmul");
  gctx.plane8_uv_vhalf_loc =
      glGetUniformLocation (gctx.program_plane8_uv, "u_vhalf");
  gctx.plane8_uv_vs2_loc =
      glGetUniformLocation (gctx.program_plane8_uv, "u_vs2");
  glUseProgram (gctx.program_plane8_y);
  glUniform1i (glGetUniformLocation (gctx.program_plane8_y, "tex_y"), 0);
  gctx.plane8_y_t_step_loc =
      glGetUniformLocation (gctx.program_plane8_y, "t_step");
  gctx.pos_plane8_y_loc =
      glGetAttribLocation (gctx.program_plane8_y, "position");
  glUseProgram (gctx.program_plane10_y);
  glUniform1i (glGetUniformLocation (gctx.program_plane10_y, "tex_y"), 0);
  gctx.plane10_y_src_scale_loc =
      glGetUniformLocation (gctx.program_plane10_y, "src_scale");
  gctx.pos_plane10_y_loc =
      glGetAttribLocation (gctx.program_plane10_y, "position");
  glUseProgram (gctx.program_plane10_uv);
  glUniform1i (glGetUniformLocation (gctx.program_plane10_uv, "tex_y"), 0);
  gctx.plane10_uv_src_scale_loc =
      glGetUniformLocation (gctx.program_plane10_uv, "src_scale");
  gctx.plane10_uv_px_step_loc =
      glGetUniformLocation (gctx.program_plane10_uv, "px_step");
  gctx.plane10_uv_rowmul_loc =
      glGetUniformLocation (gctx.program_plane10_uv, "u_rowmul");
  gctx.plane10_uv_dvu_loc =
      glGetUniformLocation (gctx.program_plane10_uv, "dvu");
  gctx.pos_plane10_uv_loc =
      glGetAttribLocation (gctx.program_plane10_uv, "position");

  gctx.program_rgb2y = gst_mpp_gpu_build_program (fs_rgb2y);
  gctx.program_rgb2uvc = gst_mpp_gpu_build_program (fs_rgb2uvc);
  gctx.program_rgb2c = gst_mpp_gpu_build_program (fs_rgb2c);
  if (!gctx.program_rgb2y || !gctx.program_rgb2uvc || !gctx.program_rgb2c)
    goto err;

  glUseProgram (gctx.program_rgb2y);
  glUniform1i (glGetUniformLocation (gctx.program_rgb2y, "tex"), 0);
  gctx.rgb2y_t_step_loc = glGetUniformLocation (gctx.program_rgb2y, "t_step");
  gctx.pos_rgb2y_loc = glGetAttribLocation (gctx.program_rgb2y, "position");
  glUseProgram (gctx.program_rgb2uvc);
  glUniform1i (glGetUniformLocation (gctx.program_rgb2uvc, "tex"), 0);
  gctx.rgb2uvc_px_step_loc =
      glGetUniformLocation (gctx.program_rgb2uvc, "px_step");
  gctx.rgb2uvc_row_step_loc =
      glGetUniformLocation (gctx.program_rgb2uvc, "row_step");
  gctx.rgb2uvc_swap_loc = glGetUniformLocation (gctx.program_rgb2uvc, "swap");
  gctx.pos_rgb2uvc_loc = glGetAttribLocation (gctx.program_rgb2uvc, "position");
  glUseProgram (gctx.program_rgb2c);
  glUniform1i (glGetUniformLocation (gctx.program_rgb2c, "tex"), 0);
  gctx.rgb2c_px_step_loc = glGetUniformLocation (gctx.program_rgb2c, "px_step");
  gctx.rgb2c_row_step_loc =
      glGetUniformLocation (gctx.program_rgb2c, "row_step");
  gctx.rgb2c_sel_loc = glGetUniformLocation (gctx.program_rgb2c, "sel");
  gctx.pos_rgb2c_loc = glGetAttribLocation (gctx.program_rgb2c, "position");

  /* every program holds its own reference to the shared vertex shader */
  glDeleteShader (gctx.vs);
  gctx.vs = 0;

  glUseProgram (gctx.program_rgb2y);
  glUniform1i (glGetUniformLocation (gctx.program_rgb2y, "tex"), 0);
  glUseProgram (gctx.program_rgb2uvc);
  glUniform1i (glGetUniformLocation (gctx.program_rgb2uvc, "tex"), 0);
  glUseProgram (gctx.program_rgb2c);
  glUniform1i (glGetUniformLocation (gctx.program_rgb2c, "tex"), 0);

  eglMakeCurrent (gctx.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

  /* Dump the DDK's dma-buf import format list for diagnostics */
  {
    PFNEGLQUERYDMABUFFORMATSEXTPROC query_formats;

    EGL_LOAD_PROC (query_formats, PFNEGLQUERYDMABUFFORMATSEXTPROC,
        "eglQueryDmaBufFormatsEXT");
    if (query_formats) {
      EGLint n = 0;

      if (query_formats (gctx.display, 0, NULL, &n) && n > 0) {
        EGLint *fmts = g_new0 (EGLint, n);
        gint i;

        if (query_formats (gctx.display, n, fmts, &n)) {
          for (i = 0; i < n; i++) {
            EGLint f = fmts[i];

            GST_DEBUG ("dmabuf import fmt %d: %c%c%c%c", f,
                f & 0xff, (f >> 8) & 0xff, (f >> 16) & 0xff, (f >> 24) & 0xff);
          }
        }

        g_free (fmts);
      }
    }
  }

  gctx.inited = TRUE;
  GST_INFO ("GPU converter inited");
  return TRUE;

err:
  if (gctx.display != EGL_NO_DISPLAY)
    eglMakeCurrent (gctx.display, EGL_NO_SURFACE, EGL_NO_SURFACE,
        EGL_NO_CONTEXT);
  GST_WARNING ("failed to init GPU converter");
  gctx.unavailable = TRUE;
  return FALSE;
}

static gboolean
gst_mpp_gpu_bind_src (gint src_fds[2], guint hor_stride, guint ver_stride,
    MppFrameFormat fmt, gboolean nearest, GLfloat * chr_dx, GLfloat * vu)
{
  PFNGLEGLIMAGETARGETTEXTURE2DOESPROC image_target;
  PFNEGLCREATEIMAGEKHRPROC create_image;
  PFNEGLDESTROYIMAGEKHRPROC destroy_image;
  EGLDisplay dpy = gctx.display;
  EGLImageKHR imgs[2] = { EGL_NO_IMAGE_KHR, EGL_NO_IMAGE_KHR };
  GLint filter = nearest ? GL_NEAREST : GL_LINEAR;
  /*
   * Plane geometry: { fourcc, width_texels, height, pitch_bytes,
   * offset_bytes }. The chroma plane is imported as ONE single-component
   * image spanning the whole interleaved CbCr row (texel == sample for
   * R8, == 16-bit sample for R16); the shader reads Cb at texcoord.x
   * and Cr one texel to the right (chr_dx = 1 / chroma width in
   * texels).
   */
  EGLint fcc[2], pw[2], ph[2], pp[2], off[2];
  gboolean ok = FALSE;
  gint i;

  EGL_LOAD_PROC (image_target, PFNGLEGLIMAGETARGETTEXTURE2DOESPROC,
      "glEGLImageTargetTexture2DOES");
  EGL_LOAD_PROC (create_image, PFNEGLCREATEIMAGEKHRPROC, "eglCreateImageKHR");
  EGL_LOAD_PROC (destroy_image, PFNEGLDESTROYIMAGEKHRPROC,
      "eglDestroyImageKHR");
  if (!image_target || !create_image || !destroy_image)
    return FALSE;

  *vu = 0.0f;

  switch (fmt) {
    case MPP_FMT_YUV420SP:
      /* NV12: chroma row = hor_stride/2 CbCr pairs (hs bytes) */
      fcc[0] = DRM_FORMAT_R8;
      pw[0] = hor_stride;
      ph[0] = ver_stride;
      pp[0] = hor_stride;
      off[0] = 0;
      fcc[1] = DRM_FORMAT_R8;
      pw[1] = hor_stride;
      ph[1] = ver_stride / 2;
      pp[1] = hor_stride;
      off[1] = hor_stride * ver_stride;
      *chr_dx = 1.0f / hor_stride;
      break;
    case MPP_FMT_YUV420SP_VU:
      /* NV21: same layout with VU pairs */
      fcc[0] = DRM_FORMAT_R8;
      pw[0] = hor_stride;
      ph[0] = ver_stride;
      pp[0] = hor_stride;
      off[0] = 0;
      fcc[1] = DRM_FORMAT_R8;
      pw[1] = hor_stride;
      ph[1] = ver_stride / 2;
      pp[1] = hor_stride;
      off[1] = hor_stride * ver_stride;
      *chr_dx = 1.0f / hor_stride;
      *vu = 1.0f;
      break;
    case MPP_FMT_YUV422SP:
      /* NV16: chroma row = hor_stride/2 CbCr pairs (hs bytes), one
       * chroma row per luma row */
      fcc[0] = DRM_FORMAT_R8;
      pw[0] = hor_stride;
      ph[0] = ver_stride;
      pp[0] = hor_stride;
      off[0] = 0;
      fcc[1] = DRM_FORMAT_R8;
      pw[1] = hor_stride;
      ph[1] = ver_stride;
      pp[1] = hor_stride;
      off[1] = hor_stride * ver_stride;
      *chr_dx = 1.0f / hor_stride;
      break;
    case MPP_FMT_YUV422SP_VU:
      /* NV61: same layout with VU pairs */
      fcc[0] = DRM_FORMAT_R8;
      pw[0] = hor_stride;
      ph[0] = ver_stride;
      pp[0] = hor_stride;
      off[0] = 0;
      fcc[1] = DRM_FORMAT_R8;
      pw[1] = hor_stride;
      ph[1] = ver_stride;
      pp[1] = hor_stride;
      off[1] = hor_stride * ver_stride;
      *chr_dx = 1.0f / hor_stride;
      *vu = 1.0f;
      break;
    default:
      GST_WARNING ("unsupported src fmt %d", (gint) fmt);
      return FALSE;
  }

  /* Import every frame: on this DDK an EGLImage import is an
   * independent snapshot of the dma-buf (verified: rendering through one
   * import is not visible through another import of the same fd), so
   * caching images by fd would serve stale pool contents. */
  for (i = 0; i < 2; i++) {
    EGLint attrs[] = {
      EGL_WIDTH, pw[i],
      EGL_HEIGHT, ph[i],
      EGL_LINUX_DRM_FOURCC_EXT, fcc[i],
      EGL_DMA_BUF_PLANE0_FD_EXT, src_fds[i],
      EGL_DMA_BUF_PLANE0_OFFSET_EXT, off[i],
      EGL_DMA_BUF_PLANE0_PITCH_EXT, pp[i],
      EGL_NONE
    };

    imgs[i] =
        create_image (dpy, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, attrs);
    if (imgs[i] == EGL_NO_IMAGE_KHR) {
      GST_WARNING ("failed to import plane %d (fourcc 0x%x, offset %d)", i,
          fcc[i], off[i]);
      goto out;
    }
  }

  glBindTexture (GL_TEXTURE_2D, gctx.src_tex[0]);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  image_target (GL_TEXTURE_2D, imgs[0]);
  glBindTexture (GL_TEXTURE_2D, gctx.src_tex[1]);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  image_target (GL_TEXTURE_2D, imgs[1]);
  ok = TRUE;

out:
  if (imgs[0] != EGL_NO_IMAGE_KHR)
    destroy_image (dpy, imgs[0]);
  if (imgs[1] != EGL_NO_IMAGE_KHR)
    destroy_image (dpy, imgs[1]);
  return ok;
}

/* Whole-frame NV15 import for the compact 10-bit format, following the
 * glamor XV dmabuf path: width derived from the byte stride, chroma
 * plane right after the luma plane, REC601 + narrow-range hints so the
 * hardware sampler unpacks and expands the samples for us. */
static gboolean
gst_mpp_gpu_bind_src_oes (gint src_fd, guint hor_stride, guint ver_stride)
{
  PFNGLEGLIMAGETARGETTEXTURE2DOESPROC image_target;
  PFNEGLCREATEIMAGEKHRPROC create_image;
  PFNEGLDESTROYIMAGEKHRPROC destroy_image;
  EGLDisplay dpy = gctx.display;
  EGLImageKHR img;
  guint claimed_w = hor_stride / 10 * 8;
  EGLint attrs[] = {
    EGL_WIDTH, (EGLint) claimed_w,
    EGL_HEIGHT, (EGLint) ver_stride,
    EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_NV15,
    EGL_DMA_BUF_PLANE0_FD_EXT, src_fd,
    EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
    EGL_DMA_BUF_PLANE0_PITCH_EXT, (EGLint) hor_stride,
    EGL_DMA_BUF_PLANE1_FD_EXT, src_fd,
    EGL_DMA_BUF_PLANE1_OFFSET_EXT,
    (EGLint) ((glong) hor_stride * ver_stride),
    EGL_DMA_BUF_PLANE1_PITCH_EXT, (EGLint) hor_stride,
    EGL_YUV_COLOR_SPACE_HINT_EXT, EGL_ITU_REC601_EXT,
    EGL_SAMPLE_RANGE_HINT_EXT, EGL_YUV_NARROW_RANGE_EXT,
    EGL_NONE
  };

  EGL_LOAD_PROC (image_target, PFNGLEGLIMAGETARGETTEXTURE2DOESPROC,
      "glEGLImageTargetTexture2DOES");
  EGL_LOAD_PROC (create_image, PFNEGLCREATEIMAGEKHRPROC, "eglCreateImageKHR");
  EGL_LOAD_PROC (destroy_image, PFNEGLDESTROYIMAGEKHRPROC,
      "eglDestroyImageKHR");
  if (!image_target || !create_image || !destroy_image)
    return FALSE;

  img = create_image (dpy, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, attrs);
  if (img == EGL_NO_IMAGE_KHR) {
    GST_WARNING ("failed to import NV15 image (egl error 0x%x)",
        (guint) eglGetError ());
    return FALSE;
  }

  glBindTexture (GL_TEXTURE_EXTERNAL_OES, gctx.src_tex_oes);
  image_target (GL_TEXTURE_EXTERNAL_OES, img);
  (void) glGetError ();

  destroy_image (dpy, img);
  return TRUE;
}

static void
gst_mpp_gpu_pack_yuv_pixel (const guint8 * p, guint8 * yy, guint8 * uu,
    guint8 * vv)
{
  gint r = p[0], g = p[1], b = p[2];
  gint y, u, v;

  /* BT.601 limited range, same as the shader above */
  y = ((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
  u = ((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
  v = ((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;

  *yy = CLAMP (y, 0, 255);
  *uu = CLAMP (u, 0, 255);
  *vv = CLAMP (v, 0, 255);
}

/* RGBA readback -> semi-planar YUV (NV12, NV21, NV16, NV61) */
static void
gst_mpp_gpu_argb_to_semi (const guint8 * argb, guint width, guint height,
    guint src_stride, guint8 * dy, guint8 * duv, gint dst_y_stride,
    gint dst_c_stride, gboolean v_first, gint chroma_y_step)
{
  guint y, x;

  for (y = 0; y < height; y++) {
    const guint8 *row = argb + y * src_stride;
    guint8 *dyy = dy + y * dst_y_stride;
    guint8 *dc = duv + (y / chroma_y_step) * dst_c_stride;

    for (x = 0; x < width; x++) {
      guint8 yy, uu, vv;

      gst_mpp_gpu_pack_yuv_pixel (row + 4 * x, &yy, &uu, &vv);
      dyy[x] = yy;

      if (!(y % chroma_y_step) && !(x % 2)) {
        dc[x] = v_first ? vv : uu;
        dc[x + 1] = v_first ? uu : vv;
      }
    }
  }
}

/* RGBA readback -> planar YUV (I420, YV12) */
static void
gst_mpp_gpu_argb_to_planar (const guint8 * argb, guint width, guint height,
    guint src_stride, guint8 * dy, guint8 * du, guint8 * dv, gint dst_y_stride,
    gint dst_c_stride)
{
  guint y, x;

  for (y = 0; y < height; y++) {
    const guint8 *row = argb + y * src_stride;
    guint8 *dyy = dy + y * dst_y_stride;

    for (x = 0; x < width; x++) {
      guint8 yy, uu, vv;

      gst_mpp_gpu_pack_yuv_pixel (row + 4 * x, &yy, &uu, &vv);
      dyy[x] = yy;

      if (!(y % 2) && !(x % 2)) {
        du[(y / 2) * dst_c_stride + x / 2] = uu;
        dv[(y / 2) * dst_c_stride + x / 2] = vv;
      }
    }
  }
}

static gboolean
gst_mpp_gpu_write_dst (const guint8 * argb, guint width, guint height,
    guint src_stride, GstMemory * out_mem, GstVideoInfo * info)
{
  GstVideoFormat format = GST_VIDEO_INFO_FORMAT (info);
  gsize size = gst_memory_get_sizes (out_mem, NULL, NULL);
  guint8 *data, *dy, *du, *dv;
  gint y_stride, c_stride;
  gboolean ret = FALSE;

  switch (format) {
    case GST_VIDEO_FORMAT_NV12:
    case GST_VIDEO_FORMAT_NV21:
    case GST_VIDEO_FORMAT_NV16:
    case GST_VIDEO_FORMAT_NV61:
    case GST_VIDEO_FORMAT_I420:
    case GST_VIDEO_FORMAT_YV12:
      break;
    default:
      GST_WARNING ("unsupported dst fmt %s",
          gst_mpp_video_format_to_string (format));
      return FALSE;
  }

  if (size < GST_VIDEO_INFO_SIZE (info)) {
    GST_WARNING ("dst buffer too small");
    return FALSE;
  }

  data =
      mmap (NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED,
      gst_dmabuf_memory_get_fd (out_mem), 0);
  if (data == MAP_FAILED) {
    GST_WARNING ("failed to mmap dst buffer");
    return FALSE;
  }

  dy = data + GST_VIDEO_INFO_COMP_OFFSET (info, GST_VIDEO_COMP_Y);
  du = data + GST_VIDEO_INFO_COMP_OFFSET (info, GST_VIDEO_COMP_U);
  dv = data + GST_VIDEO_INFO_COMP_OFFSET (info, GST_VIDEO_COMP_V);
  y_stride = GST_VIDEO_INFO_COMP_STRIDE (info, GST_VIDEO_COMP_Y);
  c_stride = GST_VIDEO_INFO_COMP_STRIDE (info, GST_VIDEO_COMP_U);

  switch (format) {
    case GST_VIDEO_FORMAT_NV12:
      gst_mpp_gpu_argb_to_semi (argb, width, height, src_stride,
          dy, du, y_stride, c_stride, FALSE, 2);
      ret = TRUE;
      break;
    case GST_VIDEO_FORMAT_NV21:
      gst_mpp_gpu_argb_to_semi (argb, width, height, src_stride,
          dy, du, y_stride, c_stride, TRUE, 2);
      ret = TRUE;
      break;
    case GST_VIDEO_FORMAT_NV16:
      gst_mpp_gpu_argb_to_semi (argb, width, height, src_stride,
          dy, du, y_stride, c_stride, FALSE, 1);
      ret = TRUE;
      break;
    case GST_VIDEO_FORMAT_NV61:
      gst_mpp_gpu_argb_to_semi (argb, width, height, src_stride,
          dy, du, y_stride, c_stride, TRUE, 1);
      ret = TRUE;
      break;
    case GST_VIDEO_FORMAT_I420:
    case GST_VIDEO_FORMAT_YV12:
      gst_mpp_gpu_argb_to_planar (argb, width, height, src_stride, dy, du, dv,
          y_stride, c_stride);
      ret = TRUE;
      break;
    default:
      break;
  }

  munmap (data, size);
  return ret;
}

/* GST formats the direct path can render into, mapped to their DRM
 * fourcc (little-endian byte names) for the EGL dmabuf import */
#define GST_MPP_FOURCC(a, b, c, d) ((guint32) (a) | ((guint32) (b) << 8) | \
    ((guint32) (c) << 16) | ((guint32) (d) << 24))

struct gst_mpp_gpu_direct_fmt
{
  GstVideoFormat gst_format;
  guint32 drm_fourcc;
};

static struct gst_mpp_gpu_direct_fmt gst_mpp_gpu_direct_fmts[] = {
  {GST_VIDEO_FORMAT_RGBA, GST_MPP_FOURCC ('A', 'B', '2', '4')}, /* ABGR8888 */
  {GST_VIDEO_FORMAT_BGRA, GST_MPP_FOURCC ('A', 'R', '2', '4')}, /* ARGB8888 */
  {GST_VIDEO_FORMAT_RGBx, GST_MPP_FOURCC ('X', 'B', '2', '4')},
  {GST_VIDEO_FORMAT_BGRx, GST_MPP_FOURCC ('X', 'R', '2', '4')},
  {GST_VIDEO_FORMAT_xRGB, GST_MPP_FOURCC ('B', 'X', '2', '4')},
  {GST_VIDEO_FORMAT_xBGR, GST_MPP_FOURCC ('R', 'X', '2', '4')},
  {GST_VIDEO_FORMAT_ARGB, GST_MPP_FOURCC ('B', 'G', 'R', 'A')},
  {GST_VIDEO_FORMAT_ABGR, GST_MPP_FOURCC ('R', 'G', 'B', 'A')},
  {GST_VIDEO_FORMAT_RGB16, GST_MPP_FOURCC ('R', 'G', '1', '6')},
  {GST_VIDEO_FORMAT_BGR16, GST_MPP_FOURCC ('B', 'G', '1', '6')},
};

static guint32
gst_mpp_gpu_dst_fourcc (GstVideoFormat format)
{
  for (guint i = 0; i < ARRAY_SIZE (gst_mpp_gpu_direct_fmts); i++) {
    if (gst_mpp_gpu_direct_fmts[i].gst_format == format)
      return gst_mpp_gpu_direct_fmts[i].drm_fourcc;
  }

  return 0;
}

/* Import a dmabuf as an OES texture (single plane) */
static GLuint
gst_mpp_gpu_import_oes (gint fd, guint32 fourcc, guint width, guint height,
    guint stride)
{
  PFNEGLCREATEIMAGEKHRPROC create_image =
      (PFNEGLCREATEIMAGEKHRPROC) eglGetProcAddress ("eglCreateImageKHR");
  PFNEGLDESTROYIMAGEKHRPROC destroy_image =
      (PFNEGLDESTROYIMAGEKHRPROC) eglGetProcAddress ("eglDestroyImageKHR");
  PFNGLEGLIMAGETARGETTEXTURE2DOESPROC image_target_texture_2d =
      (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)
      eglGetProcAddress ("glEGLImageTargetTexture2DOES");
  EGLint attribs[] = {
    EGL_WIDTH, (EGLint) width,
    EGL_HEIGHT, (EGLint) height,
    EGL_LINUX_DRM_FOURCC_EXT, (EGLint) fourcc,
    EGL_DMA_BUF_PLANE0_FD_EXT, fd,
    EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
    EGL_DMA_BUF_PLANE0_PITCH_EXT, (EGLint) stride,
    EGL_NONE,
  };
  EGLImageKHR image;
  GLuint tex = 0;

  if (!create_image || !destroy_image || !image_target_texture_2d)
    return 0;

  image = create_image (gctx.display, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT,
      NULL, attribs);
  if (image == EGL_NO_IMAGE_KHR)
    return 0;

  glGenTextures (1, &tex);
  glBindTexture (GL_TEXTURE_EXTERNAL_OES, tex);
  image_target_texture_2d (GL_TEXTURE_EXTERNAL_OES, image);

  destroy_image (gctx.display, image);
  return tex;
}

/* Render the bound source straight into the dst dmabuf (zero copy).
 * This is the weston fb-convert trick: import the dst as an OES
 * texture and attach it as the FBO color attachment.  Only safe for
 * RGB-family dst formats; YUV attachments report a false FBO-complete
 * on the Mali DDK and corrupt the output, so they never get here. */
static gboolean
gst_mpp_gpu_direct_convert (gint dst_fd, GstVideoInfo * dst_vinfo,
    guint src_width, guint src_height, guint src_hor_stride,
    guint src_ver_stride, GLfloat chr_dx, GLfloat vu, gboolean use_c10,
    gint rotation)
{
  guint32 fourcc = gst_mpp_gpu_dst_fourcc (GST_VIDEO_INFO_FORMAT (dst_vinfo));
  guint dst_width = GST_VIDEO_INFO_WIDTH (dst_vinfo);
  guint dst_height = GST_VIDEO_INFO_HEIGHT (dst_vinfo);
  guint dst_stride = GST_VIDEO_INFO_PLANE_STRIDE (dst_vinfo, 0);
  GLuint dst_tex = 0, fbo = 0;
  GLenum status;
  gboolean ret = FALSE;

  if (!fourcc || dst_fd < 0)
    return FALSE;

  dst_tex = gst_mpp_gpu_import_oes (dst_fd, fourcc, dst_width, dst_height,
      dst_stride);
  if (!dst_tex) {
    GST_WARNING ("failed to import dst dmabuf");
    goto out;
  }

  glGenFramebuffers (1, &fbo);
  glBindFramebuffer (GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
      GL_TEXTURE_EXTERNAL_OES, dst_tex, 0);
  status = glCheckFramebufferStatus (GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    GST_WARNING ("direct fbo incomplete: 0x%x", status);
    goto out;
  }

  glViewport (0, 0, dst_width, dst_height);
  glUseProgram (use_c10 ? gctx.program_c10 : gctx.program);
  glUniform1i (use_c10 ? gctx.rot_c10_loc : gctx.rot_loc, rotation / 90);
  if (use_c10) {
    /* the OES image is claimed at hor_stride / 10 * 8 wide; sample the
     * visible width within it */
    glUniform2f (gctx.src_scale_c10_loc,
        (GLfloat) src_width / (src_hor_stride / 10 * 8),
        (GLfloat) src_height / src_ver_stride);
  } else {
    glUniform2f (gctx.src_scale_loc, (GLfloat) src_width / src_hor_stride,
        (GLfloat) src_height / src_ver_stride);
    glUniform1f (gctx.chr_dx_loc, chr_dx);
    glUniform1f (gctx.vu_loc, vu);
  }
  glActiveTexture (GL_TEXTURE0);
  /* the 8-bit program samples sampler2D (R8 imports); only the 10-bit
   * program uses samplerExternalOES */
  if (use_c10)
    glBindTexture (GL_TEXTURE_EXTERNAL_OES, gctx.src_tex_oes);
  else
    glBindTexture (GL_TEXTURE_2D, gctx.src_tex[0]);
  if (!use_c10) {
    glActiveTexture (GL_TEXTURE1);
    glBindTexture (GL_TEXTURE_2D, gctx.src_tex[1]);
  }
  glBindBuffer (GL_ARRAY_BUFFER, gctx.vbo);
  glEnableVertexAttribArray (use_c10 ? gctx.pos_c10_loc : gctx.pos_loc);
  glVertexAttribPointer (use_c10 ? gctx.pos_c10_loc : gctx.pos_loc,
      4, GL_FLOAT, GL_FALSE, 0, 0);
  glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
  glDisableVertexAttribArray (use_c10 ? gctx.pos_c10_loc : gctx.pos_loc);
  glBindBuffer (GL_ARRAY_BUFFER, 0);

  /* The GPU must be done before downstream reads the dmabuf */
  glFinish ();

  GST_DEBUG ("converted with GPU (direct)");
  ret = TRUE;

out:
  glBindFramebuffer (GL_FRAMEBUFFER, 0);
  if (fbo)
    glDeleteFramebuffers (1, &fbo);
  if (dst_tex)
    glDeleteTextures (1, &dst_tex);
  return ret;
}

/* Render straight into the dst Y (R8) and interleaved UV (ABGR8888, one
 * texel = two UV pairs) planes.  Both targets are genuinely
 * color-renderable on the Mali DDK, unlike a whole-frame NV12 attachment
 * which reports a false FBO-complete and corrupts the output.  Two
 * passes, no intermediate buffer, no readback, no CPU pack.  The 8-bit
 * sources copy texels verbatim (bit exact); the 10-bit compact source
 * goes through the OES sampler RGB and a BT.601 limited-range inverse. */
static gboolean
gst_mpp_gpu_planes_convert (gint dst_fd, GstVideoInfo * dst_vinfo,
    guint src_width, guint src_height, guint hor_stride, guint ver_stride,
    GLfloat chr_dx, GLfloat vu, gboolean use_c10, gboolean src_422)
{
  guint y_stride = GST_VIDEO_INFO_PLANE_STRIDE (dst_vinfo, 0);
  guint c_stride = GST_VIDEO_INFO_PLANE_STRIDE (dst_vinfo, 1);
  gsize c_offset = GST_VIDEO_INFO_PLANE_OFFSET (dst_vinfo, 1);
  guint uv_fourcc = GST_MPP_FOURCC ('A', 'B', '2', '4');
  GstVideoFormat dst_fmt = GST_VIDEO_INFO_FORMAT (dst_vinfo);
  /* full-height chroma planes for the 4:2:2 dst family, half for 4:2:0 */
  gboolean dst_422 = (dst_fmt == GST_VIDEO_FORMAT_NV16 ||
      dst_fmt == GST_VIDEO_FORMAT_NV61);
  gboolean dst_swap = (dst_fmt == GST_VIDEO_FORMAT_NV21 ||
      dst_fmt == GST_VIDEO_FORMAT_NV61);
  guint dst_c_rows = dst_422 ? GST_VIDEO_INFO_HEIGHT (dst_vinfo)
      : GST_VIDEO_INFO_HEIGHT (dst_vinfo) / 2;
  guint uv_w = GST_VIDEO_INFO_WIDTH (dst_vinfo) / 4;
  guint dst_w = GST_VIDEO_INFO_WIDTH (dst_vinfo);
  guint dst_h = GST_VIDEO_INFO_HEIGHT (dst_vinfo);
  /* dst/src size ratio; 1.0 collapses the shaders onto bit-exact
   * texel-centre sampling */
  GLfloat ratio_x = (GLfloat) src_width / dst_w;
  GLfloat ratio_y = (GLfloat) src_height / dst_h;
  struct
  {
    GLuint tex, fbo;
    guint32 fourcc;
    guint w, h, pitch;
    gsize offset;
    GLuint program;
    GLint viewport_w, viewport_h;
  } pass[2] = {
    {0, 0, DRM_FORMAT_R8, GST_VIDEO_INFO_WIDTH (dst_vinfo),
          GST_VIDEO_INFO_HEIGHT (dst_vinfo), y_stride, 0,
        0, GST_VIDEO_INFO_WIDTH (dst_vinfo), GST_VIDEO_INFO_HEIGHT (dst_vinfo)},
    {0, 0, 0, uv_w, dst_c_rows, c_stride, c_offset,
        0, uv_w, dst_c_rows},
  };
  GLenum status;
  int p, i;

  pass[1].fourcc = uv_fourcc;
  pass[0].program = use_c10 ? gctx.program_plane10_y : gctx.program_plane8_y;
  pass[1].program = use_c10 ? gctx.program_plane10_uv : gctx.program_plane8_uv;

  gst_mpp_gpu_load_procs ();

  for (p = 0; p < 2; p++) {
    EGLImageKHR image = EGL_NO_IMAGE_KHR;
    PFNEGLCREATEIMAGEKHRPROC create_image = p_create_image;
    PFNEGLDESTROYIMAGEKHRPROC destroy_image = p_destroy_image;
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC image_target_texture_2d =
        p_image_target;
    EGLint attribs[] = {
      EGL_WIDTH, (EGLint) pass[p].w,
      EGL_HEIGHT, (EGLint) pass[p].h,
      EGL_LINUX_DRM_FOURCC_EXT, (EGLint) pass[p].fourcc,
      EGL_DMA_BUF_PLANE0_FD_EXT, dst_fd,
      EGL_DMA_BUF_PLANE0_OFFSET_EXT, (EGLint) pass[p].offset,
      EGL_DMA_BUF_PLANE0_PITCH_EXT, (EGLint) pass[p].pitch,
      EGL_NONE,
    };
    GLint pos_loc;

    if (!create_image || !destroy_image || !image_target_texture_2d)
      goto out;

    image = create_image (gctx.display, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT,
        NULL, attribs);
    if (image == EGL_NO_IMAGE_KHR) {
      GST_WARNING ("planes: failed to import dst plane %d", p);
      goto out;
    }

    glGenTextures (1, &pass[p].tex);
    glBindTexture (GL_TEXTURE_2D, pass[p].tex);
    image_target_texture_2d (GL_TEXTURE_2D, image);
    destroy_image (gctx.display, image);

    glGenFramebuffers (1, &pass[p].fbo);
    glBindFramebuffer (GL_FRAMEBUFFER, pass[p].fbo);
    glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, pass[p].tex, 0);
    status = glCheckFramebufferStatus (GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
      GST_WARNING ("planes: fbo incomplete (plane %d): 0x%x", p, status);
      goto out;
    }

    glViewport (0, 0, pass[p].viewport_w, pass[p].viewport_h);
    glUseProgram (pass[p].program);
    glUniform2f (glGetUniformLocation (pass[p].program, "u_vp"),
        pass[p].viewport_w, pass[p].viewport_h);
    if (pass[p].program == gctx.program_plane8_y) {
      glUniform2f (gctx.plane8_y_t_step_loc,
          ratio_x / hor_stride, ratio_y / ver_stride);
      glActiveTexture (GL_TEXTURE0);
      glBindTexture (GL_TEXTURE_2D, gctx.src_tex[0]);
    } else if (pass[p].program == gctx.program_plane8_uv) {
      glUniform1f (gctx.plane8_uv_tx_loc, chr_dx);
      glUniform1f (gctx.plane8_uv_ratio_x_loc, ratio_x);
      glUniform1f (gctx.plane8_uv_ratio_y_loc, ratio_y);
      glUniform1f (gctx.plane8_uv_rowmul_loc, dst_422 ? 1.0f : 2.0f);
      glUniform1f (gctx.plane8_uv_vhalf_loc, src_422 ? 1.0f : 0.5f);
      glUniform1f (gctx.plane8_uv_vs2_loc,
          (src_422 ? 1.0f : 2.0f) / ver_stride);
      glUniform1f (gctx.plane8_uv_vu_loc, vu);
      glUniform1f (gctx.plane8_uv_dvu_loc, dst_swap ? 1.0f : 0.0f);
      glActiveTexture (GL_TEXTURE0);
      glBindTexture (GL_TEXTURE_2D, gctx.src_tex[1]);
    } else if (pass[p].program == gctx.program_plane10_y) {
      /* texcoord spans the dst viewport; src_scale alone maps 0..1
       * onto the visible source area, so scaling needs no ratio here */
      glUniform2f (gctx.plane10_y_src_scale_loc,
          (GLfloat) src_width / (hor_stride / 10 * 8),
          (GLfloat) src_height / ver_stride);
      glActiveTexture (GL_TEXTURE0);
      glBindTexture (GL_TEXTURE_EXTERNAL_OES, gctx.src_tex_oes);
    } else {
      glUniform2f (gctx.plane10_uv_src_scale_loc,
          (GLfloat) src_width / (hor_stride / 10 * 8),
          (GLfloat) src_height / ver_stride);
      glUniform2f (gctx.plane10_uv_px_step_loc, 1.0f / dst_w, 1.0f / dst_h);
      glUniform1f (gctx.plane10_uv_rowmul_loc, dst_422 ? 1.0f : 2.0f);
      glUniform1f (gctx.plane10_uv_dvu_loc, dst_swap ? 1.0f : 0.0f);
      glActiveTexture (GL_TEXTURE0);
      glBindTexture (GL_TEXTURE_EXTERNAL_OES, gctx.src_tex_oes);
    }

    pos_loc = pass[p].program == gctx.program_plane8_y ? gctx.pos_plane8_y_loc
        : pass[p].program == gctx.program_plane8_uv ? gctx.pos_plane8_uv_loc
        : pass[p].program == gctx.program_plane10_y ? gctx.pos_plane10_y_loc
        : gctx.pos_plane10_uv_loc;
    glBindBuffer (GL_ARRAY_BUFFER, gctx.vbo);
    glEnableVertexAttribArray (pos_loc);
    glVertexAttribPointer (pos_loc, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray (pos_loc);
    glBindBuffer (GL_ARRAY_BUFFER, 0);

    glBindFramebuffer (GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers (1, &pass[p].fbo);
    pass[p].fbo = 0;
    glDeleteTextures (1, &pass[p].tex);
    pass[p].tex = 0;
  }

  /* both passes must land before downstream reads the dmabuf */
  glFinish ();

  GST_DEBUG ("converted with GPU (planes)");
  return TRUE;

out:
  glBindFramebuffer (GL_FRAMEBUFFER, 0);
  for (i = 0; i < 2; i++) {
    if (pass[i].fbo)
      glDeleteFramebuffers (1, &pass[i].fbo);
    if (pass[i].tex)
      glDeleteTextures (1, &pass[i].tex);
  }
  return FALSE;
}

/* Cached RGB intermediate (GBM dmabuf, tight dst size) */
static gboolean
gst_mpp_gpu_rgb_intermediate (guint w, guint h)
{
  if (gctx.rgb_fd >= 0 && gctx.rgb_w == (gint) w && gctx.rgb_h == (gint) h)
    return TRUE;

  if (gctx.rgb_itex) {
    glDeleteTextures (1, &gctx.rgb_itex);
    gctx.rgb_itex = 0;
  }
  if (gctx.rgb_bo) {
    gbm_bo_destroy (gctx.rgb_bo);
    gctx.rgb_bo = NULL;
  }
  if (gctx.rgb_fd >= 0) {
    close (gctx.rgb_fd);
    gctx.rgb_fd = -1;
  }

  gctx.rgb_bo = gbm_bo_create (gctx.gbm, w, h, GBM_FORMAT_XRGB8888,
      GBM_BO_USE_LINEAR);
  if (!gctx.rgb_bo)
    return FALSE;

  gctx.rgb_fd = gbm_bo_get_fd (gctx.rgb_bo);
  if (gctx.rgb_fd < 0) {
    gbm_bo_destroy (gctx.rgb_bo);
    gctx.rgb_bo = NULL;
    return FALSE;
  }

  /* import once; pass A renders into it, pass B samples it */
  gst_mpp_gpu_load_procs ();
  {
    EGLint attribs[] = {
      EGL_WIDTH, (EGLint) w,
      EGL_HEIGHT, (EGLint) h,
      EGL_LINUX_DRM_FOURCC_EXT, (EGLint) GST_MPP_FOURCC ('X', 'R', '2', '4'),
      EGL_DMA_BUF_PLANE0_FD_EXT, gctx.rgb_fd,
      EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
      EGL_DMA_BUF_PLANE0_PITCH_EXT, (EGLint) (w * 4),
      EGL_NONE,
    };
    EGLImageKHR image = p_create_image (gctx.display, EGL_NO_CONTEXT,
        EGL_LINUX_DMA_BUF_EXT, NULL, attribs);

    if (image == EGL_NO_IMAGE_KHR) {
      gbm_bo_destroy (gctx.rgb_bo);
      gctx.rgb_bo = NULL;
      close (gctx.rgb_fd);
      gctx.rgb_fd = -1;
      return FALSE;
    }

    glGenTextures (1, &gctx.rgb_itex);
    glBindTexture (GL_TEXTURE_EXTERNAL_OES, gctx.rgb_itex);
    p_image_target (GL_TEXTURE_EXTERNAL_OES, image);
    glTexParameteri (GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER,
        GL_NEAREST);
    glTexParameteri (GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER,
        GL_NEAREST);
    glTexParameteri (GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S,
        GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T,
        GL_CLAMP_TO_EDGE);
    p_destroy_image (gctx.display, image);
  }

  gctx.rgb_w = w;
  gctx.rgb_h = h;
  return TRUE;
}

static GLuint
gst_mpp_gpu_import_tex (gint fd, guint32 fourcc, guint w, guint h,
    guint pitch, guint offset, GLenum target)
{
  PFNEGLCREATEIMAGEKHRPROC create_image = p_create_image;
  PFNEGLDESTROYIMAGEKHRPROC destroy_image = p_destroy_image;
  PFNGLEGLIMAGETARGETTEXTURE2DOESPROC image_target_texture_2d = p_image_target;
  EGLint attribs[] = {
    EGL_WIDTH, (EGLint) w,
    EGL_HEIGHT, (EGLint) h,
    EGL_LINUX_DRM_FOURCC_EXT, (EGLint) fourcc,
    EGL_DMA_BUF_PLANE0_FD_EXT, fd,
    EGL_DMA_BUF_PLANE0_OFFSET_EXT, (EGLint) offset,
    EGL_DMA_BUF_PLANE0_PITCH_EXT, (EGLint) pitch,
    EGL_NONE,
  };
  EGLImageKHR image;
  GLuint tex = 0;

  if (!create_image || !destroy_image || !image_target_texture_2d)
    return 0;

  image = create_image (gctx.display, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT,
      NULL, attribs);
  if (image == EGL_NO_IMAGE_KHR) {
    GST_WARNING ("import tex failed: fourcc %08x %ux%u pitch %u offset %u: "
        "egl 0x%x", fourcc, w, h, pitch, offset, eglGetError ());
    return 0;
  }

  glGenTextures (1, &tex);
  glBindTexture (target, tex);
  image_target_texture_2d (target, image);

  destroy_image (gctx.display, image);
  return tex;
}

/* Two-pass YUV conversion through an RGB intermediate:
 *   pass A: src -> GBM RGB intermediate (existing YUV->RGB program, VS
 *           rotation and viewport scaling come free here)
 *   pass B: RGB intermediate -> dst Y/UV planes (R8 / ABGR8888 targets)
 * Everything stays on the GPU; covers NV12/NV21/NV16/NV61/I420/YV12 dst,
 * scaling and rotation. */
static gboolean
gst_mpp_gpu_yuv_via_rgb (gint dst_fd, GstVideoInfo * dst_vinfo,
    guint src_width, guint src_height, guint hor_stride, guint ver_stride,
    GLfloat chr_dx, GLfloat vu, gboolean use_c10, gint rotation)
{
  guint dst_w = GST_VIDEO_INFO_WIDTH (dst_vinfo);
  guint dst_h = GST_VIDEO_INFO_HEIGHT (dst_vinfo);
  struct
  {
    GLuint tex, fbo;
    guint32 fourcc;
    guint w, h, pitch;
    gsize offset;
    GLuint program;
    GLfloat row_step, sel;
    GLint vw, vh;
  } planes[3];
  guint nplanes = 0;
  GLuint rgb_tex = 0;
  GLenum status;
  int p, i;

  /* pass 0 is always the luma plane */
  planes[0].fourcc = DRM_FORMAT_R8;
  planes[0].w = dst_w;
  planes[0].h = dst_h;
  planes[0].pitch = GST_VIDEO_INFO_PLANE_STRIDE (dst_vinfo, 0);
  planes[0].offset = 0;
  planes[0].program = gctx.program_rgb2y;
  planes[0].row_step = 1.0f;
  planes[0].sel = 0.0f;
  planes[0].vw = dst_w;
  planes[0].vh = dst_h;
  nplanes = 1;

  switch (GST_VIDEO_INFO_FORMAT (dst_vinfo)) {
    case GST_VIDEO_FORMAT_NV12:
      planes[1].row_step = 2.0f;
      planes[1].sel = 0.0f;
      goto uvc;
    case GST_VIDEO_FORMAT_NV21:
      planes[1].row_step = 2.0f;
      planes[1].sel = 1.0f;
      goto uvc;
    case GST_VIDEO_FORMAT_NV16:
      planes[1].row_step = 1.0f;
      planes[1].sel = 0.0f;
      goto uvc;
    case GST_VIDEO_FORMAT_NV61:
      planes[1].row_step = 1.0f;
      planes[1].sel = 1.0f;
      goto uvc;
    uvc:
      planes[1].fourcc = GST_MPP_FOURCC ('A', 'B', '2', '4');
      planes[1].w = dst_w / 4;
      planes[1].h = dst_h / (guint) planes[1].row_step;
      planes[1].pitch = GST_VIDEO_INFO_PLANE_STRIDE (dst_vinfo, 1);
      planes[1].offset = GST_VIDEO_INFO_PLANE_OFFSET (dst_vinfo, 1);
      planes[1].program = gctx.program_rgb2uvc;
      planes[1].vw = planes[1].w;
      planes[1].vh = planes[1].h;
      nplanes = 2;
      break;
    case GST_VIDEO_FORMAT_I420:
      planes[1].sel = 0.0f;
      planes[2].sel = 1.0f;
      goto planar;
    case GST_VIDEO_FORMAT_YV12:
      planes[1].sel = 1.0f;
      planes[2].sel = 0.0f;
    planar:
      planes[1].fourcc = planes[2].fourcc = DRM_FORMAT_R8;
      planes[1].w = planes[2].w = dst_w / 2;
      planes[1].h = planes[2].h = dst_h / 2;
      planes[1].pitch = GST_VIDEO_INFO_PLANE_STRIDE (dst_vinfo, 1);
      planes[1].offset = GST_VIDEO_INFO_PLANE_OFFSET (dst_vinfo, 1);
      planes[2].pitch = GST_VIDEO_INFO_PLANE_STRIDE (dst_vinfo, 2);
      planes[2].offset = GST_VIDEO_INFO_PLANE_OFFSET (dst_vinfo, 2);
      planes[1].program = planes[2].program = gctx.program_rgb2c;
      planes[1].row_step = planes[2].row_step = 2.0f;
      planes[1].vw = planes[2].vw = planes[1].w;
      planes[1].vh = planes[2].vh = planes[1].h;
      nplanes = 3;
      break;
    default:
      return FALSE;
  }

  if (!gst_mpp_gpu_rgb_intermediate (dst_w, dst_h))
    return FALSE;

  /* The intermediate texture is cached with the bo: pass A renders into
   * it, pass B samples it.  Importing the same dmabuf a second time
   * does NOT see the render (the DDK keeps it in an internal buffer
   * that is dropped with the image), so both passes must share it. */
  {
    GLuint fbo = 0;
    GLint pos_loc;

    rgb_tex = gctx.rgb_itex;
    if (!rgb_tex)
      return FALSE;

    glGenFramebuffers (1, &fbo);
    glBindFramebuffer (GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_EXTERNAL_OES, rgb_tex, 0);
    status = glCheckFramebufferStatus (GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
      GST_WARNING ("via-rgb: pass A fbo incomplete: 0x%x", status);
      glBindFramebuffer (GL_FRAMEBUFFER, 0);
      glDeleteFramebuffers (1, &fbo);
      rgb_tex = 0;
      return FALSE;
    }

    glViewport (0, 0, dst_w, dst_h);
    glUseProgram (use_c10 ? gctx.program_c10 : gctx.program);
    glUniform1i (use_c10 ? gctx.rot_c10_loc : gctx.rot_loc, rotation / 90);
    if (use_c10) {
      glUniform2f (gctx.src_scale_c10_loc,
          (GLfloat) src_width / (hor_stride / 10 * 8),
          (GLfloat) src_height / ver_stride);
      glActiveTexture (GL_TEXTURE0);
      glBindTexture (GL_TEXTURE_EXTERNAL_OES, gctx.src_tex_oes);
    } else {
      glUniform2f (gctx.src_scale_loc,
          (GLfloat) src_width / hor_stride, (GLfloat) src_height / ver_stride);
      glUniform1f (gctx.chr_dx_loc, chr_dx);
      glUniform1f (gctx.vu_loc, vu);
      glActiveTexture (GL_TEXTURE0);
      glBindTexture (GL_TEXTURE_2D, gctx.src_tex[0]);
      glActiveTexture (GL_TEXTURE1);
      glBindTexture (GL_TEXTURE_2D, gctx.src_tex[1]);
    }
    pos_loc = use_c10 ? gctx.pos_c10_loc : gctx.pos_loc;
    glBindBuffer (GL_ARRAY_BUFFER, gctx.vbo);
    glEnableVertexAttribArray (pos_loc);
    glVertexAttribPointer (pos_loc, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray (pos_loc);
    glBindBuffer (GL_ARRAY_BUFFER, 0);

    glBindFramebuffer (GL_FRAMEBUFFER, 0);
    /* the fbo goes away; rgb_tex stays alive for pass B */
    glDeleteFramebuffers (1, &fbo);
  }

  /* pass B: RGB intermediate -> dst planes, sampling the same texture */
  glActiveTexture (GL_TEXTURE0);
  glBindTexture (GL_TEXTURE_EXTERNAL_OES, rgb_tex);

  for (p = 0; p < (gint) nplanes; p++) {
    GLuint tex = 0, fbo = 0;
    GLint pos_loc;

    planes[p].tex = 0;
    planes[p].fbo = 0;
    planes[p].tex = gst_mpp_gpu_import_tex (dst_fd, planes[p].fourcc,
        planes[p].w, planes[p].h, planes[p].pitch,
        (guint) planes[p].offset, GL_TEXTURE_2D);
    if (!planes[p].tex) {
      GST_WARNING ("via-rgb: failed to import dst plane %d", p);
      goto out;
    }


    glGenFramebuffers (1, &fbo);
    glBindFramebuffer (GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, tex = planes[p].tex, 0);
    status = glCheckFramebufferStatus (GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
      GST_WARNING ("via-rgb: fbo incomplete (plane %d): 0x%x", p, status);
      glBindFramebuffer (GL_FRAMEBUFFER, 0);
      glDeleteFramebuffers (1, &fbo);
      glDeleteTextures (1, &tex);
      planes[p].tex = 0;
      goto out;
    }

    glViewport (0, 0, planes[p].vw, planes[p].vh);
    glUseProgram (planes[p].program);
    glUniform2f (glGetUniformLocation (planes[p].program, "u_vp"),
        planes[p].vw, planes[p].vh);
    if (planes[p].program == gctx.program_rgb2y) {
      glUniform2f (gctx.rgb2y_t_step_loc, 1.0f / dst_w, 1.0f / dst_h);
    } else {
      glUniform2f (planes[p].program == gctx.program_rgb2uvc ?
          gctx.rgb2uvc_px_step_loc : gctx.rgb2c_px_step_loc,
          1.0f / dst_w, 1.0f / dst_h);
      glUniform1f (planes[p].program == gctx.program_rgb2uvc ?
          gctx.rgb2uvc_row_step_loc : gctx.rgb2c_row_step_loc,
          planes[p].row_step);
      glUniform1f (planes[p].program == gctx.program_rgb2uvc ?
          gctx.rgb2uvc_swap_loc : gctx.rgb2c_sel_loc, planes[p].sel);
    }

    pos_loc = planes[p].program == gctx.program_rgb2y ? gctx.pos_rgb2y_loc
        : planes[p].program == gctx.program_rgb2uvc ? gctx.pos_rgb2uvc_loc
        : gctx.pos_rgb2c_loc;
    glBindBuffer (GL_ARRAY_BUFFER, gctx.vbo);
    glEnableVertexAttribArray (pos_loc);
    glVertexAttribPointer (pos_loc, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray (pos_loc);
    glBindBuffer (GL_ARRAY_BUFFER, 0);

    glBindFramebuffer (GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers (1, &fbo);
    glDeleteTextures (1, &tex);
    planes[p].tex = 0;
  }

  /* everything must land before downstream reads the dmabuf */
  glFinish ();

  GST_DEBUG ("converted with GPU (via-rgb)");
  return TRUE;

out:
  glBindFramebuffer (GL_FRAMEBUFFER, 0);
  for (i = 0; i < (gint) nplanes; i++) {
    if (planes[i].fbo)
      glDeleteFramebuffers (1, &planes[i].fbo);
    if (planes[i].tex)
      glDeleteTextures (1, &planes[i].tex);
  }
  return FALSE;
}

gboolean
gst_mpp_gpu_convert_from_mpp_frame (MppFrame mframe, GstMemory * out_mem,
    GstVideoInfo * dst_vinfo, gint rotation)
{
  MppFrameFormat src_fmt = mpp_frame_get_fmt (mframe) & MPP_FRAME_FMT_MASK;
  GstVideoFormat dst_fmt = GST_VIDEO_INFO_FORMAT (dst_vinfo);
  guint src_width = mpp_frame_get_width (mframe);
  guint src_height = mpp_frame_get_height (mframe);
  guint width = GST_VIDEO_INFO_WIDTH (dst_vinfo);
  guint height = GST_VIDEO_INFO_HEIGHT (dst_vinfo);
  guint hor_stride = mpp_frame_get_hor_stride (mframe);
  guint ver_stride = mpp_frame_get_ver_stride (mframe);
  MppBuffer mbuf;
  GLfloat chr_dx = 0.0f, vu = 0.0f;
  GLuint fbo_status;
  gboolean use_c10;
  gsize rgba_size;
  gint src_fd;
  gint src_fds[2];
  gboolean ret = FALSE;

  if (dst_fmt == GST_VIDEO_FORMAT_UNKNOWN)
    return FALSE;

  g_mutex_lock (&gctx.lock);

  if (!gst_mpp_gpu_init ())
    goto out;

  /* Import failures are format capability gaps, not transient errors.
   * Latch the failing format pair only, so a P010 import failure does
   * not disable the GPU tier for NV12 or 4:2:2 streams in the same
   * process. */
  /* Only 90-degree multiples are supported anywhere; each path checks
   * its own capability below (direct needs a renderable fourcc, the
   * plane paths handle rotation through the vertex shader / pass A) */
  if (rotation && rotation % 90)
    goto out;

  if (gctx.unavailable_src_fmt == src_fmt
      && gctx.unavailable_dst_fmt == dst_fmt) {
    GST_DEBUG ("GPU convert unavailable for fmt pair %d -> %s", (gint) src_fmt,
        gst_mpp_video_format_to_string (dst_fmt));
    goto out;
  }

  mbuf = mpp_frame_get_buffer (mframe);
  if (!mbuf)
    goto out;
  src_fd = mpp_buffer_get_fd (mbuf);
  if (src_fd < 0)
    goto out;

  src_fds[0] = src_fds[1] = src_fd;

  /* Must be current before creating/attaching the dst texture */
  if (!eglMakeCurrent (gctx.display, EGL_NO_SURFACE, EGL_NO_SURFACE,
          gctx.context))
    goto out;

  /* The compact 10-bit format goes through a whole-frame NV15 import
   * with the hardware sampler doing unpack + CSC; the 8-bit formats go
   * through the plane-based shader */
  use_c10 = (src_fmt == MPP_FMT_YUV420SP_10BIT);

  if (use_c10) {
    if (!gst_mpp_gpu_bind_src_oes (src_fd, hor_stride, ver_stride)) {
      gctx.unavailable_src_fmt = src_fmt;
      gctx.unavailable_dst_fmt = dst_fmt;
      goto out_unmake;
    }
    ret = TRUE;
  } else {
    ret =
        gst_mpp_gpu_bind_src (src_fds, hor_stride, ver_stride, src_fmt, FALSE,
        &chr_dx, &vu);
  }

  if (!ret) {
    gctx.unavailable_src_fmt = src_fmt;
    gctx.unavailable_dst_fmt = dst_fmt;
    ret = FALSE;
    goto out_unmake;
  }

  /* Zero-copy path: render straight into the dst dmabuf.  FBO
   * completeness failures are a DDK capability gap: latch the pair
   * and fall through to the readback path. */
  if (gst_mpp_gpu_dst_fourcc (dst_fmt) &&
      !(gctx.unavailable_direct_src_fmt == src_fmt &&
          gctx.unavailable_direct_dst_fmt == dst_fmt)) {
    gint dst_fd = gst_dmabuf_memory_get_fd (out_mem);

    if (dst_fd >= 0 &&
        gst_mpp_gpu_direct_convert (dst_fd, dst_vinfo, src_width, src_height,
            hor_stride, ver_stride, chr_dx, vu, use_c10, rotation))
      goto out_unmake;

    gctx.unavailable_direct_src_fmt = src_fmt;
    gctx.unavailable_direct_dst_fmt = dst_fmt;
    GST_WARNING ("direct path unavailable for %d -> %s, falling back",
        (gint) src_fmt, gst_mpp_video_format_to_string (dst_fmt));
  }

  /* Zero-copy plane path for the semi-planar dst family: render into
   * the R8 Y plane and the ABGR8888 UV plane, both genuinely
   * renderable.  Unscaled 8-bit runs are bit exact; scaled runs become
   * horizontal bilinear + vertical row selection.  The UV carrier
   * (one texel = 4 chroma bytes) imports as ABGR8888, which the DDK
   * accepts only at 16-texel-aligned widths, so gate on the dst width
   * here and let an odd size fall through instead of latching the
   * pair.  No rotation; that keeps the via-RGB two-pass below. */
  if (!rotation && (width % 64) == 0 &&
      (dst_fmt == GST_VIDEO_FORMAT_NV12 ||
          dst_fmt == GST_VIDEO_FORMAT_NV21 ||
          dst_fmt == GST_VIDEO_FORMAT_NV16 ||
          dst_fmt == GST_VIDEO_FORMAT_NV61) &&
      !(gctx.unavailable_planes_src_fmt == src_fmt &&
          gctx.unavailable_planes_dst_fmt == dst_fmt)) {
    gint dst_fd = gst_dmabuf_memory_get_fd (out_mem);

    if (dst_fd >= 0 &&
        gst_mpp_gpu_planes_convert (dst_fd, dst_vinfo, src_width, src_height,
            hor_stride, ver_stride, chr_dx, vu, use_c10,
            src_fmt == MPP_FMT_YUV422SP || src_fmt == MPP_FMT_YUV422SP_VU))
      goto out_unmake;

    gctx.unavailable_planes_src_fmt = src_fmt;
    gctx.unavailable_planes_dst_fmt = dst_fmt;
    GST_WARNING ("plane path unavailable for %d -> %s, falling back",
        (gint) src_fmt, gst_mpp_video_format_to_string (dst_fmt));
  }

  /* RGB-intermediate two-pass for every other YUV dst (NV21/I420/YV12/
   * NV16/NV61), plus scaled or rotated NV12: src -> GBM RGB -> dst
   * planes, all GPU side.  The bit-exact plane path above keeps
   * priority where it applies. */
  if ((dst_fmt == GST_VIDEO_FORMAT_NV12 ||
          dst_fmt == GST_VIDEO_FORMAT_NV21 ||
          dst_fmt == GST_VIDEO_FORMAT_NV16 ||
          dst_fmt == GST_VIDEO_FORMAT_NV61 ||
          dst_fmt == GST_VIDEO_FORMAT_I420 ||
          dst_fmt == GST_VIDEO_FORMAT_YV12) &&
      (!rotation || (width / 4) % 16 == 0) &&
      !(gctx.unavailable_via_src_fmt == src_fmt &&
          gctx.unavailable_via_dst_fmt == dst_fmt)) {
    gint dst_fd = gst_dmabuf_memory_get_fd (out_mem);
    gboolean transient;

    /* A transient GBM allocation failure retries on the next frame
     * instead of latching the pair onto the readback path */
    transient = !gst_mpp_gpu_rgb_intermediate (width, height);

    if (!transient && dst_fd >= 0 &&
        gst_mpp_gpu_yuv_via_rgb (dst_fd, dst_vinfo, src_width, src_height,
            hor_stride, ver_stride, chr_dx, vu, use_c10, rotation))
      goto out_unmake;

    if (!transient) {
      gctx.unavailable_via_src_fmt = src_fmt;
      gctx.unavailable_via_dst_fmt = dst_fmt;
      GST_WARNING ("via-rgb path unavailable for %d -> %s, falling back",
          (gint) src_fmt, gst_mpp_video_format_to_string (dst_fmt));
    }
  }

  /* The readback path cannot rotate; a rotated YUV frame that reached
   * this point is dropped rather than converted with the wrong
   * orientation (RGA handles rotation when available) */
  if (rotation)
    goto out;

  if (gctx.dst_w != width || gctx.dst_h != height) {
    glDeleteTextures (1, &gctx.dst_tex);
    glBindTexture (GL_TEXTURE_2D, 0);
    glGenTextures (1, &gctx.dst_tex);
    glBindTexture (GL_TEXTURE_2D, gctx.dst_tex);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, NULL);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    g_free (gctx.rgba);
    rgba_size = (gsize) width *height * 4;
    gctx.rgba = g_malloc (rgba_size);

    gctx.dst_w = width;
    gctx.dst_h = height;
  }

  glBindFramebuffer (GL_FRAMEBUFFER, gctx.dst_fbo);
  glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
      gctx.dst_tex, 0);
  fbo_status = glCheckFramebufferStatus (GL_FRAMEBUFFER);
  if (fbo_status != GL_FRAMEBUFFER_COMPLETE) {
    GST_WARNING ("fbo incomplete: 0x%x", fbo_status);
    glBindFramebuffer (GL_FRAMEBUFFER, 0);
    goto out_unmake;
  }

  glViewport (0, 0, width, height);
  glClearColor (0, 0, 0, 1);
  glClear (GL_COLOR_BUFFER_BIT);
  glUseProgram (use_c10 ? gctx.program_c10 : gctx.program);
  glUniform1i (use_c10 ? gctx.rot_c10_loc : gctx.rot_loc, 0);
  if (use_c10) {
    /* the OES image is claimed at hor_stride / 10 * 8 wide; sample the
     * visible width within it */
    glUniform2f (gctx.src_scale_c10_loc,
        (GLfloat) src_width / (hor_stride / 10 * 8),
        (GLfloat) src_height / ver_stride);
  } else {
    glUniform2f (gctx.src_scale_loc,
        (GLfloat) src_width / hor_stride, (GLfloat) src_height / ver_stride);
    glUniform1f (gctx.chr_dx_loc, chr_dx);
    glUniform1f (gctx.vu_loc, vu);
  }
  glActiveTexture (GL_TEXTURE0);
  if (use_c10)
    glBindTexture (GL_TEXTURE_EXTERNAL_OES, gctx.src_tex_oes);
  else
    glBindTexture (GL_TEXTURE_2D, gctx.src_tex[0]);
  glActiveTexture (GL_TEXTURE1);
  if (!use_c10)
    glBindTexture (GL_TEXTURE_2D, gctx.src_tex[1]);
  glBindBuffer (GL_ARRAY_BUFFER, gctx.vbo);
  glEnableVertexAttribArray (use_c10 ? gctx.pos_c10_loc : gctx.pos_loc);
  glVertexAttribPointer (use_c10 ? gctx.pos_c10_loc : gctx.pos_loc, 4, GL_FLOAT,
      GL_FALSE, 0, 0);
  glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
  glDisableVertexAttribArray (use_c10 ? gctx.pos_c10_loc : gctx.pos_loc);
  glBindBuffer (GL_ARRAY_BUFFER, 0);

  glReadPixels (0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, gctx.rgba);
  glBindFramebuffer (GL_FRAMEBUFFER, 0);

  ret =
      gst_mpp_gpu_write_dst (gctx.rgba, width, height, width * 4, out_mem,
      dst_vinfo);

out_unmake:
  eglMakeCurrent (gctx.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
out:
  g_mutex_unlock (&gctx.lock);

  if (ret)
    GST_DEBUG ("converted with GPU");

  return ret;
}

gboolean
gst_mpp_use_gpu (void)
{
  static gint mpp_use_gpu = -1;

  if (mpp_use_gpu < 0) {
    const gchar *buf = g_getenv ("GST_MPP_NO_GPU");

    GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, "mppgpu", 0, "MPP GPU converter");

    if (!buf || buf[0] == '0') {
      GST_INFO ("GPU convert enabled");
      mpp_use_gpu = 1;
      return 1;
    }

    GST_INFO ("GPU convert disabled by env (GST_MPP_NO_GPU)");
    mpp_use_gpu = 0;
  }

  return mpp_use_gpu > 0;
}

#endif
