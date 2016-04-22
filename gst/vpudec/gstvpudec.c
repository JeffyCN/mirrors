
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "gstvpudec.h"
#include <gst/gst.h>


GST_DEBUG_CATEGORY (gst_vpudec_debug);
#define GST_CAT_DEFAULT gst_vpudec_debug

#define parent_class gst_vpudec_parent_class
G_DEFINE_TYPE (GstVPUDec, gst_vpudec, GST_TYPE_VIDEO_DECODER);


enum
{
  PROP_0,
  PROP_DEVICE,
};

/* GstVideoDecoder base class method */
static gboolean gst_vpudec_start (GstVideoDecoder * decoder);
static gboolean gst_vpudec_stop (GstVideoDecoder * video_decoder);
static gboolean gst_vpudec_set_format (GstVideoDecoder * decoder,
    GstVideoCodecState * state);
static GstFlowReturn gst_vpudec_handle_frame (GstVideoDecoder * decoder,
    GstVideoCodecFrame * frame);
static void gst_vpudec_finalize (GObject * object);
static GstFlowReturn
gst_vpudec_decode (GstVideoDecoder * decoder, GstVideoCodecFrame * frame);
gboolean plugin_init (GstPlugin * plugin);

static GstStaticPadTemplate gst_vpudec_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-h264,"
        "stream-format = (string) { byte-stream },"
        "alignment = (string) { au }"
        ";" )
    );

    /* Add sink caps here based on input codecs supported by the decoder */

static GstStaticPadTemplate gst_vpudec_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw, "
        "format = (string) NV12, "
        "width  = (int) [ 32, 4096 ], " "height =  (int) [ 32, 4096 ]"));

    /* Add src caps here based on decoder output format(s) */

gboolean
plugin_init (GstPlugin * plugin)
{
  if (!gst_element_register (plugin, "vpudec", GST_RANK_PRIMARY + 1,
          GST_TYPE_VPUDEC))
    return FALSE;
  return TRUE;
}

static void
gst_vpudec_class_init (GstVPUDecClass * klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
  GstVideoDecoderClass *video_decoder_class = GST_VIDEO_DECODER_CLASS (klass);
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = gst_vpudec_finalize;

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_vpudec_src_template));

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_vpudec_sink_template));

  video_decoder_class->start = GST_DEBUG_FUNCPTR (gst_vpudec_start);
  video_decoder_class->stop = GST_DEBUG_FUNCPTR (gst_vpudec_stop);
  video_decoder_class->set_format = GST_DEBUG_FUNCPTR (gst_vpudec_set_format);
  video_decoder_class->handle_frame =
      GST_DEBUG_FUNCPTR (gst_vpudec_handle_frame);

  GST_DEBUG_CATEGORY_INIT (gst_vpudec_debug, "vpudec", 0,
      "vpu video decoder");

  gst_element_class_set_static_metadata (element_class,
      "VPU decoder", "Decoder/Video", "A vpu decoder", "None");
}

/* Init the vpudec structure */
static void
gst_vpudec_init (GstVPUDec * decoder)
{
  GstVideoDecoder *dec = (GstVideoDecoder *) decoder;

  GST_DEBUG_OBJECT (dec, "init");
    /* Open the decode
     - Set the VPU context 
     - Do all the initial setting for plugin
     */
}

static void
gst_vpudec_finalize (GObject * object)
{
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

/* Open the device */
static gboolean
gst_vpudec_start (GstVideoDecoder * decoder)
{
  GstVPUDec *dec = GST_VPUDEC (decoder);

  GST_DEBUG_OBJECT (dec, "Starting");

  return TRUE;
}

static gboolean
gst_vpudec_stop (GstVideoDecoder * decoder)
{
  GstVPUDec *dec = GST_VPUDEC (decoder);

  GST_DEBUG_OBJECT (dec, "Stopping");

  /* Free all the buffers
     Close the decoder handle */

  return TRUE;
}

/* Setup input (Output for VPU) */
static gboolean
gst_vpudec_set_format (GstVideoDecoder * decoder, GstVideoCodecState * state)
{
  GstVPUDec *dec = GST_VPUDEC (decoder);
  GstStructure *structure;

  GST_DEBUG_OBJECT (dec, "Setting format: %" GST_PTR_FORMAT, state->caps);

  structure = gst_caps_get_structure (state->caps, 0);

  GST_DEBUG_OBJECT (dec, "received structure %s", gst_structure_to_string (structure));

  /* Strcuture contains codec information, width and height,
     - Set codec information to the decoder 
     - Set width and height info to the decoder 
     - Set dec->input_setup to TRUE 
  */

  return GST_FLOW_OK;
}

static GstFlowReturn
gst_vpudec_handle_frame (GstVideoDecoder * decoder, GstVideoCodecFrame * frame)
{
  GstVPUDec *dec = GST_VPUDEC (decoder);

  GST_DEBUG_OBJECT (dec, "handle frame");
  /* Setup output if not yet done */
    
  /* For every frame, decode */
  gst_vpudec_decode (decoder, frame);

  return GST_FLOW_OK;
}

/* Here we push AU to VPU, header found so */
static GstFlowReturn
gst_vpudec_decode (GstVideoDecoder * decoder, GstVideoCodecFrame * frame)
{
  GstVPUDec *dec = GST_VPUDEC (decoder);

  GST_DEBUG_OBJECT (dec, "decode");
  /* DO decoding
     - Lock
     - Push encoded data
     - Get output raw data 
     - Apply PTS 
     - Finish frame 
     - Free previous buffer 
     - Unlock 
   */ 

  return GST_FLOW_OK;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    vpudec,
    "VPU decoder",
    plugin_init, VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN);
