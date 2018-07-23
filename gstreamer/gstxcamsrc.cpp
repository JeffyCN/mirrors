/*
 * gstrkisp.cpp - gst rkisp plugin
 *
 *  Copyright (c) 2015 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * SECTION:element-rkisp
 *
 * FIXME:Describe rkisp here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 rkisp io-mode=4 sensor-id=0 imageprocessor=0 analyzer=1 \
 *  ! video/x-raw, format=NV12, width=1920, height=1080, framerate=25/1     \
 *  ! vaapiencode_h264 ! fakesink
 * ]|
 * </refsect2>
 */

#include "gstrkisp.h"
#include "gstxcambufferpool.h"
#if HAVE_IA_AIQ
#include "gstxcaminterface.h"
#include "dynamic_analyzer_loader.h"
#include "isp/hybrid_analyzer_loader.h"
#include "x3a_analyze_tuner.h"
#include "isp/isp_poll_thread.h"
#endif

#include "fake_poll_thread.h"
#include "fake_v4l2_device.h"

#include <signal.h>
#include <uvc_device.h>

using namespace XCam;
using namespace GstXCam;

#define CAPTURE_DEVICE_STILL    "/dev/video0"
#define CAPTURE_DEVICE_VIDEO    "/dev/video0"
#define ISP_DEVICE_VIDEO    "/dev/video1"

#if HAVE_IA_AIQ
#define DEFAULT_CPF_FILE_NAME   "/etc/atomisp/imx185.cpf"
#define DEFAULT_DYNAMIC_3A_LIB  "/usr/lib/xcam/plugins/3a/libxcam_3a_aiq.so"
#endif

#define V4L2_CAPTURE_MODE_STILL 0x2000
#define V4L2_CAPTURE_MODE_VIDEO 0x4000
#define V4L2_CAPTURE_MODE_PREVIEW 0x8000

#define DEFAULT_PROP_SENSOR             0
#define DEFAULT_PROP_MEM_MODE           V4L2_MEMORY_DMABUF
#if HAVE_IA_AIQ
#define DEFAULT_PROP_ENABLE_3A          TRUE
#endif
#define DEFAULT_PROP_ENABLE_USB         FALSE
#define DEFAULT_PROP_BUFFERCOUNT        8
#define DEFAULT_PROP_PIXELFORMAT        V4L2_PIX_FMT_NV12 //420 instead of 0
#define DEFAULT_PROP_FIELD              V4L2_FIELD_NONE // 0
#define DEFAULT_PROP_ANALYZER           SIMPLE_ANALYZER
//#if HAVE_IA_AIQ
#define DEFAULT_PROP_IMAGE_PROCESSOR    ISP_IMAGE_PROCESSOR
//#endif

#define DEFAULT_VIDEO_WIDTH             1920
#define DEFAULT_VIDEO_HEIGHT            1080

#define GST_XCAM_INTERFACE_HEADER(from, src, device_manager, analyzer)     \
    GstXCamSrc  *src = GST_XCAM_SRC (from);                              \
    XCAM_ASSERT (src);                                                     \
    SmartPtr<MainDeviceManager> device_manager = src->device_manager;      \
    XCAM_ASSERT (src->device_manager.ptr ());                              \
    SmartPtr<X3aAnalyzer> analyzer = device_manager->get_analyzer ();      \
    XCAM_ASSERT (analyzer.ptr ())


XCAM_BEGIN_DECLARE

static GstStaticPadTemplate gst_xcam_src_factory =
    GST_STATIC_PAD_TEMPLATE ("src",
                             GST_PAD_SRC,
                             GST_PAD_ALWAYS,
                             GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE (GST_VIDEO_FORMATS_ALL)));


GST_DEBUG_CATEGORY (gst_xcam_src_debug);
#define GST_CAT_DEFAULT gst_xcam_src_debug

#define GST_TYPE_XCAM_SRC_MEM_MODE (gst_xcam_src_mem_mode_get_type ())
static GType
gst_xcam_src_mem_mode_get_type (void)
{
    static GType g_type = 0;

    if (!g_type) {
        static const GEnumValue mem_types [] = {
            {V4L2_MEMORY_MMAP, "memory map mode", "mmap"},
            {V4L2_MEMORY_USERPTR, "user pointer mode", "userptr"},
            {V4L2_MEMORY_OVERLAY, "overlay mode", "overlay"},
            {V4L2_MEMORY_DMABUF, "dmabuf mode", "dmabuf"},
            {0, NULL, NULL}
        };
        g_type = g_enum_register_static ("GstXCamMemoryModeType", mem_types);
    }
    return g_type;
}

#define GST_TYPE_XCAM_SRC_FIELD (gst_xcam_src_field_get_type ())
static GType
gst_xcam_src_field_get_type (void)
{
    static GType g_type = 0;

    if (!g_type) {
        static const GEnumValue field_types [] = {
            {V4L2_FIELD_NONE, "no field", "none"},
            {V4L2_FIELD_TOP, "top field", "top"},
            {V4L2_FIELD_BOTTOM, "bottom field", "bottom"},
            {V4L2_FIELD_INTERLACED, "interlaced fields", "interlaced"},
            {V4L2_FIELD_SEQ_TB, "both fields sequential, top first", "seq-tb"},
            {V4L2_FIELD_SEQ_BT, "both fields sequential, bottom first", "seq-bt"},
            {V4L2_FIELD_ALTERNATE, "both fields alternating", "alternate"},
            {V4L2_FIELD_INTERLACED_TB, "interlaced fields, top first", "interlaced-tb"},
            {V4L2_FIELD_INTERLACED_BT, "interlaced fields, bottom first", "interlaced-bt"},
            {0, NULL, NULL}
        };
        g_type = g_enum_register_static ("GstXCamSrcFieldType", field_types);
    }
    return g_type;
}


#define GST_TYPE_XCAM_SRC_IMAGE_PROCESSOR (gst_xcam_src_image_processor_get_type ())
static GType
gst_xcam_src_image_processor_get_type (void)
{
    static GType g_type = 0;
    static const GEnumValue image_processor_types[] = {
#if HAVE_IA_AIQ
        {ISP_IMAGE_PROCESSOR, "ISP image processor", "isp"},
#endif
        {0, NULL, NULL},
    };

    if (g_once_init_enter (&g_type)) {
        const GType type =
            g_enum_register_static ("GstXCamSrcImageProcessorType", image_processor_types);
        g_once_init_leave (&g_type, type);
    }

    return g_type;
}

#define GST_TYPE_XCAM_SRC_ANALYZER (gst_xcam_src_analyzer_get_type ())
static GType
gst_xcam_src_analyzer_get_type (void)
{
    static GType g_type = 0;
    static const GEnumValue analyzer_types[] = {
        {SIMPLE_ANALYZER, "simple 3A analyzer", "simple"},
#if HAVE_IA_AIQ
        {AIQ_TUNER_ANALYZER, "aiq 3A analyzer", "aiq"},
#endif
        {0, NULL, NULL},
    };

    if (g_once_init_enter (&g_type)) {
        const GType type =
            g_enum_register_static ("GstXCamSrcAnalyzerType", analyzer_types);
        g_once_init_leave (&g_type, type);
    }

    return g_type;
}

enum {
    PROP_0,
    PROP_DEVICE,
    PROP_SENSOR,
    PROP_MEM_MODE,
    PROP_BUFFERCOUNT,
    PROP_FIELD,
    PROP_IMAGE_PROCESSOR,
    PROP_WDR_MODE,
    PROP_3A_ANALYZER,
    PROP_PIPE_PROFLE,
    PROP_CPF,
#if HAVE_IA_AIQ
    PROP_ENABLE_3A,
    PROP_3A_LIB,
#endif
    PROP_INPUT_FMT,
    PROP_ENABLE_USB,
    PROP_WAVELET_MODE,
    PROP_DEFOG_MODE,
    PROP_DENOISE_3D_MODE,
    PROP_ENABLE_WIREFRAME,
    PROP_ENABLE_IMAGE_WARP,
    PROP_FAKE_INPUT
};

#if HAVE_IA_AIQ
static void gst_xcam_src_xcam_3a_interface_init (GstXCam3AInterface *iface);

G_DEFINE_TYPE_WITH_CODE  (GstXCamSrc, gst_xcam_src, GST_TYPE_PUSH_SRC,
                          G_IMPLEMENT_INTERFACE (GST_TYPE_XCAM_3A_IF,
                                  gst_xcam_src_xcam_3a_interface_init));
#else
G_DEFINE_TYPE (GstXCamSrc, gst_xcam_src, GST_TYPE_PUSH_SRC);
#endif

#define parent_class gst_xcam_src_parent_class

static void gst_xcam_src_finalize (GObject * object);
static void gst_xcam_src_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void gst_xcam_src_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static GstCaps* gst_xcam_src_get_caps (GstBaseSrc *src, GstCaps *filter);
static gboolean gst_xcam_src_set_caps (GstBaseSrc *src, GstCaps *caps);
static gboolean gst_xcam_src_decide_allocation (GstBaseSrc *src, GstQuery *query);
static gboolean gst_xcam_src_start (GstBaseSrc *src);
static gboolean gst_xcam_src_stop (GstBaseSrc *src);
static gboolean gst_xcam_src_unlock (GstBaseSrc *src);
static gboolean gst_xcam_src_unlock_stop (GstBaseSrc *src);
static GstFlowReturn gst_xcam_src_alloc (GstBaseSrc *src, guint64 offset, guint size, GstBuffer **buffer);
static GstFlowReturn gst_xcam_src_fill (GstPushSrc *src, GstBuffer *out);

#if HAVE_IA_AIQ
/* GstXCamInterface implementation */
static gboolean gst_xcam_src_set_white_balance_mode (GstXCam3A *xcam3a, XCamAwbMode mode);
static gboolean gst_xcam_src_set_awb_speed (GstXCam3A *xcam3a, double speed);
static gboolean gst_xcam_src_set_wb_color_temperature_range (GstXCam3A *xcam3a, guint cct_min, guint cct_max);
static gboolean gst_xcam_src_set_manual_wb_gain (GstXCam3A *xcam3a, double gr, double r, double b, double gb);
static gboolean gst_xcam_src_set_exposure_mode (GstXCam3A *xcam3a, XCamAeMode mode);
static gboolean gst_xcam_src_set_ae_metering_mode (GstXCam3A *xcam3a, XCamAeMeteringMode mode);
static gboolean gst_xcam_src_set_exposure_window (GstXCam3A *xcam3a, XCam3AWindow *window, guint8 count = 1);
static gboolean gst_xcam_src_set_exposure_value_offset (GstXCam3A *xcam3a, double ev_offset);
static gboolean gst_xcam_src_set_ae_speed (GstXCam3A *xcam3a, double speed);
static gboolean gst_xcam_src_set_exposure_flicker_mode (GstXCam3A *xcam3a, XCamFlickerMode flicker);
static XCamFlickerMode gst_xcam_src_get_exposure_flicker_mode (GstXCam3A *xcam3a);
static gint64 gst_xcam_src_get_current_exposure_time (GstXCam3A *xcam3a);
static double gst_xcam_src_get_current_analog_gain (GstXCam3A *xcam3a);
static gboolean gst_xcam_src_set_manual_exposure_time (GstXCam3A *xcam3a, gint64 time_in_us);
static gboolean gst_xcam_src_set_manual_analog_gain (GstXCam3A *xcam3a, double gain);
static gboolean gst_xcam_src_set_aperture (GstXCam3A *xcam3a, double fn);
static gboolean gst_xcam_src_set_max_analog_gain (GstXCam3A *xcam3a, double max_gain);
static double gst_xcam_src_get_max_analog_gain (GstXCam3A *xcam3a);
static gboolean gst_xcam_src_set_exposure_time_range (GstXCam3A *xcam3a, gint64 min_time_in_us, gint64 max_time_in_us);
static gboolean gst_xcam_src_get_exposure_time_range (GstXCam3A *xcam3a, gint64 *min_time_in_us, gint64 *max_time_in_us);
static gboolean gst_xcam_src_set_noise_reduction_level (GstXCam3A *xcam3a, guint8 level);
static gboolean gst_xcam_src_set_temporal_noise_reduction_level (GstXCam3A *xcam3a, guint8 level, gint8 mode);
static gboolean gst_xcam_src_set_gamma_table (GstXCam3A *xcam3a, double *r_table, double *g_table, double *b_table);
static gboolean gst_xcam_src_set_gbce (GstXCam3A *xcam3a, gboolean enable);
static gboolean gst_xcam_src_set_manual_brightness (GstXCam3A *xcam3a, guint8 value);
static gboolean gst_xcam_src_set_manual_contrast (GstXCam3A *xcam3a, guint8 value);
static gboolean gst_xcam_src_set_manual_hue (GstXCam3A *xcam3a, guint8 value);
static gboolean gst_xcam_src_set_manual_saturation (GstXCam3A *xcam3a, guint8 value);
static gboolean gst_xcam_src_set_manual_sharpness (GstXCam3A *xcam3a, guint8 value);
static gboolean gst_xcam_src_set_dvs (GstXCam3A *xcam3a, gboolean enable);
static gboolean gst_xcam_src_set_night_mode (GstXCam3A *xcam3a, gboolean enable);
static gboolean gst_xcam_src_set_hdr_mode (GstXCam3A *xcam3a, guint8 mode);
static gboolean gst_xcam_src_set_denoise_mode (GstXCam3A *xcam3a, guint32 mode);
static gboolean gst_xcam_src_set_gamma_mode (GstXCam3A *xcam3a, gboolean enable);
static gboolean gst_xcam_src_set_dpc_mode(GstXCam3A * xcam3a, gboolean enable);
#endif

static gboolean gst_xcam_src_plugin_init (GstPlugin * rkisp);

XCAM_END_DECLARE

static void
gst_xcam_src_class_init (GstXCamSrcClass * class_self)
{
    GObjectClass *gobject_class;
    GstElementClass *element_class;
    GstBaseSrcClass *basesrc_class;
    GstPushSrcClass *pushsrc_class;

    gobject_class = (GObjectClass *) class_self;
    element_class = (GstElementClass *) class_self;
    basesrc_class = GST_BASE_SRC_CLASS (class_self);
    pushsrc_class = GST_PUSH_SRC_CLASS (class_self);

    GST_DEBUG_CATEGORY_INIT (gst_xcam_src_debug, "rkisp", 0, "libXCam source plugin");

    gobject_class->finalize = gst_xcam_src_finalize;
    gobject_class->set_property = gst_xcam_src_set_property;
    gobject_class->get_property = gst_xcam_src_get_property;

    g_object_class_install_property (
        gobject_class, PROP_DEVICE,
        g_param_spec_string ("device", "device", "Device location",
                             NULL, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property (
        gobject_class, PROP_SENSOR,
        g_param_spec_int ("sensor-id", "sensor id", "Sensor ID to select",
                          0, G_MAXINT, DEFAULT_PROP_SENSOR,
                          (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS) ));

    g_object_class_install_property (
        gobject_class, PROP_MEM_MODE,
        g_param_spec_enum ("io-mode", "memory mode", "Memory mode",
                           GST_TYPE_XCAM_SRC_MEM_MODE, DEFAULT_PROP_MEM_MODE,
                           (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property (
        gobject_class, PROP_FIELD,
        g_param_spec_enum ("field", "field", "field",
                           GST_TYPE_XCAM_SRC_FIELD, DEFAULT_PROP_FIELD,
                           (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property (
        gobject_class, PROP_ENABLE_USB,
        g_param_spec_boolean ("enable-usb", "enable usbcam", "Enable USB camera",
                              DEFAULT_PROP_ENABLE_USB, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property (
        gobject_class, PROP_BUFFERCOUNT,
        g_param_spec_int ("buffercount", "buffer count", "buffer count",
                          0, G_MAXINT, DEFAULT_PROP_BUFFERCOUNT,
                          (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS) ));

    g_object_class_install_property (
        gobject_class, PROP_INPUT_FMT,
        g_param_spec_string ("input-format", "input format", "Input pixel format",
                             NULL, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property (
        gobject_class, PROP_FAKE_INPUT,
        g_param_spec_string ("fake-input", "fake input", "Use the specified raw file as fake input instead of live camera",
                             NULL, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property (
        gobject_class, PROP_IMAGE_PROCESSOR,
        g_param_spec_enum ("imageprocessor", "image processor", "Image Processor",
                           GST_TYPE_XCAM_SRC_IMAGE_PROCESSOR, DEFAULT_PROP_IMAGE_PROCESSOR,
                           (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property (
        gobject_class, PROP_3A_ANALYZER,
        g_param_spec_enum ("analyzer", "3a analyzer", "3A Analyzer",
                           GST_TYPE_XCAM_SRC_ANALYZER, DEFAULT_PROP_ANALYZER,
                           (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

#if HAVE_IA_AIQ
    g_object_class_install_property (
        gobject_class, PROP_ENABLE_3A,
        g_param_spec_boolean ("enable-3a", "enable 3a", "Enable 3A",
                              DEFAULT_PROP_ENABLE_3A, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property (
        gobject_class, PROP_CPF,
        g_param_spec_string ("path-cpf", "cpf", "Path to cpf",
                             NULL, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property (
        gobject_class, PROP_3A_LIB,
        g_param_spec_string ("path-3alib", "3a lib", "Path to dynamic 3A library",
                             NULL, (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));
#endif

    gst_element_class_set_details_simple (element_class,
                                          "Libxcam Source",
                                          "Source/Base",
                                          "Capture camera video using xcam library",
                                          "Jacob Chen <cc@rock-chips.com>");

    gst_element_class_add_pad_template (
        element_class,
        gst_static_pad_template_get (&gst_xcam_src_factory));

    basesrc_class->get_caps = GST_DEBUG_FUNCPTR (gst_xcam_src_get_caps);
    basesrc_class->set_caps = GST_DEBUG_FUNCPTR (gst_xcam_src_set_caps);
    basesrc_class->decide_allocation = GST_DEBUG_FUNCPTR (gst_xcam_src_decide_allocation);

    basesrc_class->start = GST_DEBUG_FUNCPTR (gst_xcam_src_start);
    basesrc_class->stop = GST_DEBUG_FUNCPTR (gst_xcam_src_stop);
    basesrc_class->unlock = GST_DEBUG_FUNCPTR (gst_xcam_src_unlock);
    basesrc_class->unlock_stop = GST_DEBUG_FUNCPTR (gst_xcam_src_unlock_stop);
    basesrc_class->alloc = GST_DEBUG_FUNCPTR (gst_xcam_src_alloc);
    pushsrc_class->fill = GST_DEBUG_FUNCPTR (gst_xcam_src_fill);
}

// FIXME remove this function?
static void
gst_xcam_src_init (GstXCamSrc *rkisp)
{
    gst_base_src_set_format (GST_BASE_SRC (rkisp), GST_FORMAT_TIME);
    gst_base_src_set_live (GST_BASE_SRC (rkisp), TRUE);
    gst_base_src_set_do_timestamp (GST_BASE_SRC (rkisp), TRUE);

    rkisp->buf_count = DEFAULT_PROP_BUFFERCOUNT;
    rkisp->sensor_id = 0;
    rkisp->capture_mode = V4L2_CAPTURE_MODE_VIDEO;
    rkisp->device = NULL;
    rkisp->enable_usb = DEFAULT_PROP_ENABLE_USB;

#if HAVE_IA_AIQ
    rkisp->enable_3a = DEFAULT_PROP_ENABLE_3A;
    rkisp->path_to_cpf = strndup(DEFAULT_CPF_FILE_NAME, XCAM_MAX_STR_SIZE);
    rkisp->path_to_3alib = strndup(DEFAULT_DYNAMIC_3A_LIB, XCAM_MAX_STR_SIZE);
#endif

    rkisp->path_to_fake = NULL;
    rkisp->time_offset_ready = FALSE;
    rkisp->time_offset = -1;
    rkisp->buf_mark = 0;
    rkisp->duration = 0;
    rkisp->mem_type = DEFAULT_PROP_MEM_MODE;
    rkisp->field = DEFAULT_PROP_FIELD;

    rkisp->in_format = 0;
    if (rkisp->enable_usb) {
        rkisp->out_format = GST_VIDEO_FORMAT_YUY2;
    }
    else {
        rkisp->out_format = DEFAULT_PROP_PIXELFORMAT;
    }

    gst_video_info_init (&rkisp->gst_video_info);
    if (rkisp->enable_usb) {
        gst_video_info_set_format (&rkisp->gst_video_info, GST_VIDEO_FORMAT_YUY2, DEFAULT_VIDEO_WIDTH, DEFAULT_VIDEO_HEIGHT);
    }
    else {
        gst_video_info_set_format (&rkisp->gst_video_info, GST_VIDEO_FORMAT_NV12, DEFAULT_VIDEO_WIDTH, DEFAULT_VIDEO_HEIGHT);
    }

    XCAM_CONSTRUCTOR (rkisp->xcam_video_info, VideoBufferInfo);
    rkisp->xcam_video_info.init (DEFAULT_PROP_PIXELFORMAT, DEFAULT_VIDEO_WIDTH, DEFAULT_VIDEO_HEIGHT);
    rkisp->image_processor_type = DEFAULT_PROP_IMAGE_PROCESSOR;
    rkisp->analyzer_type = DEFAULT_PROP_ANALYZER;
    XCAM_CONSTRUCTOR (rkisp->device_manager, SmartPtr<MainDeviceManager>);
    rkisp->device_manager = new MainDeviceManager;
}

static void
gst_xcam_src_finalize (GObject * object)
{
    GstXCamSrc *rkisp = GST_XCAM_SRC (object);

    rkisp->device_manager.release ();
    XCAM_DESTRUCTOR (rkisp->device_manager, SmartPtr<MainDeviceManager>);

    G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_xcam_src_get_property (
    GObject *object,
    guint prop_id,
    GValue *value,
    GParamSpec *pspec)
{
    GstXCamSrc *src = GST_XCAM_SRC (object);

    switch (prop_id) {
    case PROP_DEVICE:
        g_value_set_string (value, src->device);
        break;
    case PROP_SENSOR:
        g_value_set_int (value, src->sensor_id);
        break;
    case PROP_MEM_MODE:
        g_value_set_enum (value, src->mem_type);
        break;
    case PROP_FIELD:
        g_value_set_enum (value, src->field);
        break;
    case PROP_BUFFERCOUNT:
        g_value_set_int (value, src->buf_count);
        break;
    case PROP_INPUT_FMT:
        g_value_set_string (value, xcam_fourcc_to_string (src->in_format));
        break;
    case PROP_ENABLE_USB:
        g_value_set_boolean (value, src->enable_usb);
        break;
    case PROP_FAKE_INPUT:
        g_value_set_string (value, src->path_to_fake);
        break;
    case PROP_IMAGE_PROCESSOR:
        g_value_set_enum (value, src->image_processor_type);
        break;
    case PROP_3A_ANALYZER:
        g_value_set_enum (value, src->analyzer_type);
        break;

#if HAVE_IA_AIQ
    case PROP_ENABLE_3A:
        g_value_set_boolean (value, src->enable_3a);
        break;
    case PROP_CPF:
        g_value_set_string (value, src->path_to_cpf);
        break;
    case PROP_3A_LIB:
        g_value_set_string (value, src->path_to_3alib);
        break;
#endif

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
gst_xcam_src_set_property (
    GObject *object,
    guint prop_id,
    const GValue *value,
    GParamSpec *pspec)
{
    GstXCamSrc *src = GST_XCAM_SRC (object);

    switch (prop_id) {
    case PROP_DEVICE: {
        const char * device = g_value_get_string (value);
        if (src->device)
            xcam_free (src->device);
        src->device = NULL;
        if (device)
            src->device = strndup (device, XCAM_MAX_STR_SIZE);
        break;
    }
    case PROP_SENSOR:
        src->sensor_id = g_value_get_int (value);
        break;
    case PROP_MEM_MODE:
        src->mem_type = (enum v4l2_memory)g_value_get_enum (value);
        break;
    case PROP_BUFFERCOUNT:
        src->buf_count = g_value_get_int (value);
        break;
    case PROP_FIELD:
        src->field = (enum v4l2_field) g_value_get_enum (value);
        break;
    case PROP_INPUT_FMT: {
        const char * fmt = g_value_get_string (value);
        if (strlen (fmt) == 4)
            src->in_format = v4l2_fourcc ((unsigned)fmt[0],
                                          (unsigned)fmt[1],
                                          (unsigned)fmt[2],
                                          (unsigned)fmt[3]);
        else
            GST_ERROR_OBJECT (src, "Invalid input format: not fourcc");
        break;
    }
    case PROP_ENABLE_USB:
        src->enable_usb = g_value_get_boolean (value);
        break;
    case PROP_FAKE_INPUT: {
        const char * raw_path = g_value_get_string (value);
        if (src->path_to_fake)
            xcam_free (src->path_to_fake);
        src->path_to_fake = NULL;
        if (raw_path)
            src->path_to_fake = strndup (raw_path, XCAM_MAX_STR_SIZE);
        break;
    }
    case PROP_IMAGE_PROCESSOR:
        src->image_processor_type = (ImageProcessorType)g_value_get_enum (value);
        if (src->image_processor_type == ISP_IMAGE_PROCESSOR) {
            src->capture_mode = V4L2_CAPTURE_MODE_VIDEO;
        }
        else {
            XCAM_LOG_WARNING ("this release only supports ISP image processor");
            src->image_processor_type = ISP_IMAGE_PROCESSOR;
            src->capture_mode = V4L2_CAPTURE_MODE_VIDEO;
        }
        break;
    case PROP_3A_ANALYZER:
        src->analyzer_type = (AnalyzerType)g_value_get_enum (value);
        break;

#if HAVE_IA_AIQ
    case PROP_ENABLE_3A:
        src->enable_3a = g_value_get_boolean (value);
        break;
    case PROP_CPF: {
        const char * cpf = g_value_get_string (value);
        if (src->path_to_cpf)
            xcam_free (src->path_to_cpf);
        src->path_to_cpf = NULL;
        if (cpf)
            src->path_to_cpf = strndup (cpf, XCAM_MAX_STR_SIZE);
        break;
    }
    case PROP_3A_LIB: {
        const char * path = g_value_get_string (value);
        if (src->path_to_3alib)
            xcam_free (src->path_to_3alib);
        src->path_to_3alib = NULL;
        if (path)
            src->path_to_3alib = strndup (path, XCAM_MAX_STR_SIZE);
        break;
    }
#endif
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

#if HAVE_IA_AIQ
static void
gst_xcam_src_xcam_3a_interface_init (GstXCam3AInterface *iface)
{
    iface->set_white_balance_mode = gst_xcam_src_set_white_balance_mode;
    iface->set_awb_speed = gst_xcam_src_set_awb_speed;

    iface->set_wb_color_temperature_range = gst_xcam_src_set_wb_color_temperature_range;
    iface->set_manual_wb_gain = gst_xcam_src_set_manual_wb_gain;

    iface->set_exposure_mode = gst_xcam_src_set_exposure_mode;
    iface->set_ae_metering_mode = gst_xcam_src_set_ae_metering_mode;
    iface->set_exposure_window = gst_xcam_src_set_exposure_window;
    iface->set_exposure_value_offset = gst_xcam_src_set_exposure_value_offset;
    iface->set_ae_speed = gst_xcam_src_set_ae_speed;

    iface->set_exposure_flicker_mode = gst_xcam_src_set_exposure_flicker_mode;
    iface->get_exposure_flicker_mode = gst_xcam_src_get_exposure_flicker_mode;
    iface->get_current_exposure_time = gst_xcam_src_get_current_exposure_time;
    iface->get_current_analog_gain = gst_xcam_src_get_current_analog_gain;
    iface->set_manual_exposure_time = gst_xcam_src_set_manual_exposure_time;
    iface->set_manual_analog_gain = gst_xcam_src_set_manual_analog_gain;
    iface->set_aperture = gst_xcam_src_set_aperture;
    iface->set_max_analog_gain = gst_xcam_src_set_max_analog_gain;
    iface->get_max_analog_gain = gst_xcam_src_get_max_analog_gain;
    iface->set_exposure_time_range = gst_xcam_src_set_exposure_time_range;
    iface->get_exposure_time_range = gst_xcam_src_get_exposure_time_range;
    iface->set_dvs = gst_xcam_src_set_dvs;
    iface->set_noise_reduction_level = gst_xcam_src_set_noise_reduction_level;
    iface->set_temporal_noise_reduction_level = gst_xcam_src_set_temporal_noise_reduction_level;
    iface->set_gamma_table = gst_xcam_src_set_gamma_table;
    iface->set_gbce = gst_xcam_src_set_gbce;
    iface->set_manual_brightness = gst_xcam_src_set_manual_brightness;
    iface->set_manual_contrast = gst_xcam_src_set_manual_contrast;
    iface->set_manual_hue = gst_xcam_src_set_manual_hue;
    iface->set_manual_saturation = gst_xcam_src_set_manual_saturation;
    iface->set_manual_sharpness = gst_xcam_src_set_manual_sharpness;
    iface->set_night_mode = gst_xcam_src_set_night_mode;
    iface->set_hdr_mode = gst_xcam_src_set_hdr_mode;
    iface->set_denoise_mode = gst_xcam_src_set_denoise_mode;
    iface->set_gamma_mode = gst_xcam_src_set_gamma_mode;
    iface->set_dpc_mode = gst_xcam_src_set_dpc_mode;
}
#endif

static gboolean
gst_xcam_src_start (GstBaseSrc *src)
{
    GstXCamSrc *rkisp = GST_XCAM_SRC (src);
    SmartPtr<MainDeviceManager> device_manager = rkisp->device_manager;
    SmartPtr<X3aAnalyzer> analyzer;
#if HAVE_IA_AIQ
    SmartPtr<ImageProcessor> isp_processor;
    SmartPtr<IspController> isp_controller;
#endif
    SmartPtr<V4l2Device> capture_device;
    SmartPtr<V4l2Device> isp_device;

    SmartPtr<PollThread> poll_thread;

    // Check device
    if (rkisp->device == NULL) {
        if (rkisp->capture_mode == V4L2_CAPTURE_MODE_STILL)
            rkisp->device = strndup (CAPTURE_DEVICE_STILL, XCAM_MAX_STR_SIZE);
        else
            rkisp->device = strndup (CAPTURE_DEVICE_VIDEO, XCAM_MAX_STR_SIZE);
    }
    if (rkisp->isp_device == NULL) {
         rkisp->isp_device = strndup (ISP_DEVICE_VIDEO, XCAM_MAX_STR_SIZE);
    }
    XCAM_ASSERT (rkisp->device);

    // set default input format if set prop wasn't called
    if (rkisp->in_format == 0) {
        if (rkisp->image_processor_type == CL_IMAGE_PROCESSOR)
            rkisp->in_format = V4L2_PIX_FMT_SGRBG10;
        else if (rkisp->enable_usb)
            rkisp->in_format = V4L2_PIX_FMT_YUYV;
        else
            rkisp->in_format = V4L2_PIX_FMT_NV12;
    }

    if (rkisp->path_to_fake) {
        capture_device = new FakeV4l2Device ();
    } else if (rkisp->enable_usb) {
        capture_device = new UVCDevice (rkisp->device);
    }
#if HAVE_IA_AIQ
    else {
        capture_device = new AtomispDevice (rkisp->device);
        isp_device = new AtomispDevice (rkisp->isp_device);
    }
#endif

    capture_device->set_sensor_id (rkisp->sensor_id);
    capture_device->set_capture_mode (rkisp->capture_mode);
    capture_device->set_mem_type (rkisp->mem_type);
    capture_device->set_buffer_count (rkisp->buf_count);
    capture_device->open ();
    device_manager->set_capture_device (capture_device);

    isp_device->set_sensor_id (rkisp->sensor_id);
    isp_device->set_capture_mode (rkisp->capture_mode);
    isp_device->set_mem_type (rkisp->mem_type);
    isp_device->set_buffer_count (rkisp->buf_count);
    isp_device->open ();
    //device_manager->set_isp_device (isp_device);

#if HAVE_IA_AIQ
    isp_controller = new IspController (isp_device);
#endif

    switch (rkisp->image_processor_type) {
#if HAVE_IA_AIQ
    case ISP_IMAGE_PROCESSOR: {
        isp_processor = new IspImageProcessor (isp_controller);
        device_manager->add_image_processor (isp_processor);
        break;
    }
#endif
    default:
        XCAM_LOG_ERROR ("unknown image processor type");
        return false;
    }

    switch (rkisp->analyzer_type) {
    case SIMPLE_ANALYZER: {
        analyzer = new X3aAnalyzerSimple ();
        break;
    }
#if HAVE_IA_AIQ
    case AIQ_TUNER_ANALYZER: {
        XCAM_LOG_INFO ("cpf: %s", rkisp->path_to_cpf);
        SmartPtr<X3aAnalyzer> aiq_analyzer = new X3aAnalyzerAiq (isp_controller, rkisp->path_to_cpf);
        SmartPtr<X3aAnalyzeTuner> tuner_analyzer = new X3aAnalyzeTuner ();
        XCAM_ASSERT (aiq_analyzer.ptr () && tuner_analyzer.ptr ());
        tuner_analyzer->set_analyzer (aiq_analyzer);
        analyzer = tuner_analyzer;
        break;
    }
#endif
    default:
        XCAM_LOG_ERROR ("unknown analyzer type");
        return false;
    }

    XCAM_ASSERT (analyzer.ptr ());
    if (analyzer->prepare_handlers () != XCAM_RETURN_NO_ERROR) {
        XCAM_LOG_ERROR ("analyzer(%s) prepare handlers failed", analyzer->get_name ());
        return FALSE;
    }

    if(rkisp->wdr_mode_type != NONE_WDR)
    {
        analyzer->set_ae_exposure_time_range (80 * 1110 * 1000 / 37125, 1120 * 1110 * 1000 / 37125);
        analyzer->set_ae_max_analog_gain (3.98);
    }
    device_manager->set_3a_analyzer (analyzer);

    if (rkisp->enable_usb) {
        poll_thread = new PollThread ();
    } else if (rkisp->path_to_fake) {
        poll_thread = new FakePollThread (rkisp->path_to_fake);
    }
#if HAVE_IA_AIQ
    else {
        SmartPtr<IspPollThread> isp_poll_thread = new IspPollThread ();
        isp_poll_thread->set_isp_controller (isp_controller);
        poll_thread = isp_poll_thread;
    }
#endif
    device_manager->set_poll_thread (poll_thread);

    return TRUE;
}

static gboolean
gst_xcam_src_stop (GstBaseSrc *src)
{
    GstXCamSrc *rkisp = GST_XCAM_SRC_CAST (src);
    SmartPtr<MainDeviceManager> device_manager = rkisp->device_manager;
    XCAM_ASSERT (device_manager.ptr ());

    device_manager->stop();
    device_manager->get_capture_device()->close ();
    device_manager->pause_dequeue ();
    return TRUE;
}

static gboolean
gst_xcam_src_unlock (GstBaseSrc *src)
{
    GstXCamSrc *rkisp = GST_XCAM_SRC_CAST (src);
    SmartPtr<MainDeviceManager> device_manager = rkisp->device_manager;
    XCAM_ASSERT (device_manager.ptr ());

    device_manager->pause_dequeue ();
    return TRUE;
}

static gboolean
gst_xcam_src_unlock_stop (GstBaseSrc *src)
{
    GstXCamSrc *rkisp = GST_XCAM_SRC_CAST (src);
    SmartPtr<MainDeviceManager> device_manager = rkisp->device_manager;
    XCAM_ASSERT (device_manager.ptr ());

    device_manager->resume_dequeue ();
    return TRUE;
}

static GstCaps*
gst_xcam_src_get_caps (GstBaseSrc *src, GstCaps *filter)
{
    GstXCamSrc *rkisp = GST_XCAM_SRC (src);
    XCAM_UNUSED (filter);

    return gst_pad_get_pad_template_caps (GST_BASE_SRC_PAD (rkisp));
}

static uint32_t
translate_format_to_xcam (GstVideoFormat format)
{
    switch (format) {
    case GST_VIDEO_FORMAT_NV12:
        return V4L2_PIX_FMT_NV12;
    case GST_VIDEO_FORMAT_I420:
        return V4L2_PIX_FMT_YUV420;
    case GST_VIDEO_FORMAT_YUY2:
        return V4L2_PIX_FMT_YUYV;
    case GST_VIDEO_FORMAT_Y42B:
        return V4L2_PIX_FMT_YUV422P;

        //RGB
    case GST_VIDEO_FORMAT_RGBx:
        return V4L2_PIX_FMT_RGB32;
    case GST_VIDEO_FORMAT_BGRx:
        return V4L2_PIX_FMT_BGR32;
    default:
        break;
    }
    return 0;
}

static gboolean
gst_xcam_src_set_caps (GstBaseSrc *src, GstCaps *caps)
{
    GstXCamSrc *rkisp = GST_XCAM_SRC (src);
    struct v4l2_format format;
    uint32_t out_format = 0;
    GstVideoInfo info;

    gst_video_info_from_caps (&info, caps);
    XCAM_ASSERT ((GST_VIDEO_INFO_FORMAT (&info) == GST_VIDEO_FORMAT_NV12) ||
                 (GST_VIDEO_INFO_FORMAT (&info) == GST_VIDEO_FORMAT_YUY2));

    out_format = translate_format_to_xcam (GST_VIDEO_INFO_FORMAT (&info));
    if (!out_format) {
        GST_WARNING ("format doesn't support:%s", GST_VIDEO_INFO_NAME (&info));
        return FALSE;
    }

    rkisp->out_format = out_format;

    SmartPtr<MainDeviceManager> device_manager = rkisp->device_manager;
    SmartPtr<V4l2Device> capture_device = device_manager->get_capture_device ();
    capture_device->set_framerate (GST_VIDEO_INFO_FPS_N (&info), GST_VIDEO_INFO_FPS_D (&info));
    capture_device->set_format (
        GST_VIDEO_INFO_WIDTH (&info),
        GST_VIDEO_INFO_HEIGHT(&info),
        rkisp->in_format,
        rkisp->field,
        info.stride [0]);

    if (device_manager->start () != XCAM_RETURN_NO_ERROR)
        return FALSE;

    capture_device->get_format (format);
    rkisp->gst_video_info = info;
    size_t offset = 0;
    for (uint32_t n = 0; n < GST_VIDEO_INFO_N_PLANES (&rkisp->gst_video_info); n++) {
        GST_VIDEO_INFO_PLANE_OFFSET (&rkisp->gst_video_info, n) = offset;
        if (out_format == V4L2_PIX_FMT_NV12) {
            GST_VIDEO_INFO_PLANE_STRIDE (&rkisp->gst_video_info, n) = format.fmt.pix.bytesperline * 2 / 3;
        }
        else if (format.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV) {
            // for 4:2:2 format, stride is widthx2
            GST_VIDEO_INFO_PLANE_STRIDE (&rkisp->gst_video_info, n) = format.fmt.pix.bytesperline;
        }
        else {
            GST_VIDEO_INFO_PLANE_STRIDE (&rkisp->gst_video_info, n) = format.fmt.pix.bytesperline / 2;
        }
        offset += GST_VIDEO_INFO_PLANE_STRIDE (&rkisp->gst_video_info, n) * format.fmt.pix.height;
        //TODO, need set offsets
    }

    // TODO, need calculate aligned width/height
    rkisp->xcam_video_info.init (out_format, GST_VIDEO_INFO_WIDTH (&info),  GST_VIDEO_INFO_HEIGHT (&info));

    rkisp->duration = gst_util_uint64_scale_int (
                            GST_SECOND,
                            GST_VIDEO_INFO_FPS_D(&rkisp->gst_video_info),
                            GST_VIDEO_INFO_FPS_N(&rkisp->gst_video_info));
    rkisp->pool = gst_xcam_buffer_pool_new (rkisp, caps, rkisp->device_manager);

    return TRUE;
}

static gboolean
gst_xcam_src_decide_allocation (GstBaseSrc *src, GstQuery *query)
{
    GstXCamSrc *rkisp = GST_XCAM_SRC (src);
    GstBufferPool *pool = NULL;
    uint32_t pool_num = 0;

    XCAM_ASSERT (rkisp);
    XCAM_ASSERT (rkisp->pool);

    pool_num = gst_query_get_n_allocation_pools (query);
    if (pool_num > 0) {
        for (uint32_t i = pool_num - 1; i > 0; --i) {
            gst_query_remove_nth_allocation_pool (query, i);
        }
        gst_query_parse_nth_allocation_pool (query, 0, &pool, NULL, NULL, NULL);
        if (pool == rkisp->pool)
            return TRUE;
        gst_object_unref (pool);
        gst_query_remove_nth_allocation_pool (query, 0);
    }

    gst_query_add_allocation_pool (
        query, rkisp->pool,
        GST_VIDEO_INFO_WIDTH (&rkisp->gst_video_info),
        GST_XCAM_SRC_BUF_COUNT (rkisp),
        GST_XCAM_SRC_BUF_COUNT (rkisp));

    return GST_BASE_SRC_CLASS (parent_class)->decide_allocation (src, query);
}

static GstFlowReturn
gst_xcam_src_alloc (GstBaseSrc *src, guint64 offset, guint size, GstBuffer **buffer)
{
    GstFlowReturn ret;
    GstXCamSrc *rkisp = GST_XCAM_SRC (src);

    XCAM_UNUSED (offset);
    XCAM_UNUSED (size);

    ret = gst_buffer_pool_acquire_buffer (rkisp->pool, buffer, NULL);
    XCAM_ASSERT (*buffer);
    return ret;
}

static GstFlowReturn
gst_xcam_src_fill (GstPushSrc *basesrc, GstBuffer *buf)
{
    GstXCamSrc *src = GST_XCAM_SRC_CAST (basesrc);

    GST_BUFFER_OFFSET (buf) = src->buf_mark;
    GST_BUFFER_OFFSET_END (buf) = GST_BUFFER_OFFSET (buf) + 1;
    ++src->buf_mark;

    if (!GST_CLOCK_TIME_IS_VALID (GST_BUFFER_TIMESTAMP (buf)))
        return GST_FLOW_OK;

    if (!src->time_offset_ready) {
        GstClock *clock = GST_ELEMENT_CLOCK (src);
        GstClockTime actual_time = 0;

        if (!clock)
            return GST_FLOW_OK;

        actual_time = gst_clock_get_time (clock) - GST_ELEMENT_CAST (src)->base_time;
        src->time_offset = actual_time - GST_BUFFER_TIMESTAMP (buf);
        src->time_offset_ready = TRUE;
        gst_object_ref (clock);
    }

    GST_BUFFER_TIMESTAMP (buf) += src->time_offset;
    //GST_BUFFER_DURATION (buf) = src->duration;

    XCAM_STATIC_FPS_CALCULATION (gstrkisp, XCAM_OBJ_DUR_FRAME_NUM);
    return GST_FLOW_OK;
}

#if HAVE_IA_AIQ
static gboolean
gst_xcam_src_set_white_balance_mode (GstXCam3A *xcam3a, XCamAwbMode mode)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->set_awb_mode (mode);
}

static gboolean
gst_xcam_src_set_awb_speed (GstXCam3A *xcam3a, double speed)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->set_awb_speed (speed);
}

static gboolean
gst_xcam_src_set_wb_color_temperature_range (GstXCam3A *xcam3a, guint cct_min, guint cct_max)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->set_awb_color_temperature_range (cct_min, cct_max);
}

static gboolean
gst_xcam_src_set_manual_wb_gain (GstXCam3A *xcam3a, double gr, double r, double b, double gb)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->set_awb_manual_gain (gr, r, b, gb);
}


static gboolean
gst_xcam_src_set_exposure_mode (GstXCam3A *xcam3a, XCamAeMode mode)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->set_ae_mode (mode);
}

static gboolean
gst_xcam_src_set_ae_metering_mode (GstXCam3A *xcam3a, XCamAeMeteringMode mode)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->set_ae_metering_mode (mode);
}

static gboolean
gst_xcam_src_set_exposure_window (GstXCam3A *xcam3a, XCam3AWindow *window, guint8 count)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->set_ae_window (window, count);
}

static gboolean
gst_xcam_src_set_exposure_value_offset (GstXCam3A *xcam3a, double ev_offset)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->set_ae_ev_shift (ev_offset);
}

static gboolean
gst_xcam_src_set_ae_speed (GstXCam3A *xcam3a, double speed)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->set_ae_speed (speed);
}

static gboolean
gst_xcam_src_set_exposure_flicker_mode (GstXCam3A *xcam3a, XCamFlickerMode flicker)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->set_ae_flicker_mode (flicker);
}

static XCamFlickerMode
gst_xcam_src_get_exposure_flicker_mode (GstXCam3A *xcam3a)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->get_ae_flicker_mode ();
}

static gint64
gst_xcam_src_get_current_exposure_time (GstXCam3A *xcam3a)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->get_ae_current_exposure_time ();
}

static double
gst_xcam_src_get_current_analog_gain (GstXCam3A *xcam3a)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->get_ae_current_analog_gain ();
}

static gboolean
gst_xcam_src_set_manual_exposure_time (GstXCam3A *xcam3a, gint64 time_in_us)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->set_ae_manual_exposure_time (time_in_us);
}

static gboolean
gst_xcam_src_set_manual_analog_gain (GstXCam3A *xcam3a, double gain)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->set_ae_manual_analog_gain (gain);
}

static gboolean
gst_xcam_src_set_aperture (GstXCam3A *xcam3a, double fn)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->set_ae_aperture (fn);
}

static gboolean
gst_xcam_src_set_max_analog_gain (GstXCam3A *xcam3a, double max_gain)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->set_ae_max_analog_gain (max_gain);
}

static double
gst_xcam_src_get_max_analog_gain (GstXCam3A *xcam3a)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->get_ae_max_analog_gain ();
}

static gboolean
gst_xcam_src_set_exposure_time_range (GstXCam3A *xcam3a, gint64 min_time_in_us, gint64 max_time_in_us)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->set_ae_exposure_time_range (min_time_in_us, max_time_in_us);
}

static gboolean
gst_xcam_src_get_exposure_time_range (GstXCam3A *xcam3a, gint64 *min_time_in_us, gint64 *max_time_in_us)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->get_ae_exposure_time_range (min_time_in_us, max_time_in_us);
}

static gboolean
gst_xcam_src_set_noise_reduction_level (GstXCam3A *xcam3a, guint8 level)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->set_noise_reduction_level ((level - 128) / 128.0);
}

static gboolean
gst_xcam_src_set_temporal_noise_reduction_level (GstXCam3A *xcam3a, guint8 level, gint8 mode)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    bool ret = analyzer->set_temporal_noise_reduction_level ((level - 128) / 128.0);
    XCAM_UNUSED (mode);

    return (gboolean)ret;
}

static gboolean
gst_xcam_src_set_gamma_table (GstXCam3A *xcam3a, double *r_table, double *g_table, double *b_table)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->set_gamma_table (r_table, g_table, b_table);
}

static gboolean
gst_xcam_src_set_gbce (GstXCam3A *xcam3a, gboolean enable)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->set_gbce (enable);
}

static gboolean
gst_xcam_src_set_manual_brightness (GstXCam3A *xcam3a, guint8 value)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->set_manual_brightness ((value - 128) / 128.0);
}

static gboolean
gst_xcam_src_set_manual_contrast (GstXCam3A *xcam3a, guint8 value)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->set_manual_contrast ((value - 128) / 128.0);
}

static gboolean
gst_xcam_src_set_manual_hue (GstXCam3A *xcam3a, guint8 value)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->set_manual_hue ((value - 128) / 128.0);
}

static gboolean
gst_xcam_src_set_manual_saturation (GstXCam3A *xcam3a, guint8 value)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->set_manual_saturation ((value - 128) / 128.0);
}

static gboolean
gst_xcam_src_set_manual_sharpness (GstXCam3A *xcam3a, guint8 value)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->set_manual_sharpness ((value - 128) / 128.0);
}

static gboolean
gst_xcam_src_set_dvs (GstXCam3A *xcam3a, gboolean enable)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->set_dvs (enable);
}

static gboolean
gst_xcam_src_set_night_mode (GstXCam3A *xcam3a, gboolean enable)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);

    return analyzer->set_night_mode (enable);
}

static gboolean
gst_xcam_src_set_hdr_mode (GstXCam3A *xcam3a, guint8 mode)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);
    XCAM_UNUSED (analyzer);
    XCAM_UNUSED (mode);
    return true;
}

static gboolean
gst_xcam_src_set_denoise_mode (GstXCam3A *xcam3a, guint32 mode)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);
    XCAM_UNUSED (analyzer);

    XCAM_UNUSED (mode);
    return true;
}

static gboolean
gst_xcam_src_set_gamma_mode (GstXCam3A *xcam3a, gboolean enable)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);
    XCAM_UNUSED (analyzer);
    XCAM_UNUSED (enable);
    return true;
}

static gboolean
gst_xcam_src_set_dpc_mode (GstXCam3A *xcam3a, gboolean enable)
{
    GST_XCAM_INTERFACE_HEADER (xcam3a, src, device_manager, analyzer);
    XCAM_UNUSED (analyzer);
    XCAM_UNUSED (enable);

    XCAM_LOG_WARNING ("rkisp: dpc is not supported");
    return true;
}
#endif

static gboolean
gst_xcam_src_plugin_init (GstPlugin * rkisp)
{
    return gst_element_register (rkisp, "rkisp", GST_RANK_NONE,
                                 GST_TYPE_XCAM_SRC);
}

#ifndef PACKAGE
#define PACKAGE "libxam"
#endif

GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    rkisp,
    "rkisp",
    gst_xcam_src_plugin_init,
    VERSION,
    GST_LICENSE_UNKNOWN,
    "librkisp",
    "https://github.com/01org/libxcam"
)
