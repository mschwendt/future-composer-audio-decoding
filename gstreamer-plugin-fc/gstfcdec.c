/*
 * GStreamer plugin - AMIGA Future Composer audio file decoder
 * Copyright (C) 2008 Michael Schwendt <mschwendt@users.sf.net>
 *
 * Based on GStreamer Plugin Writer's Guide (0.10.20.1) templates:
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-fcdec
 *
 * AMIGA Future Composer module decoder/player.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v filesrc location=Astaroth.FC13 ! fcdec ! audioconvert ! alsasink
 * ]|
 * </refsect2>
 */

#include <string.h>
#include <fc14audiodecoder.h>
#include <gst/gst.h>

#include "config.h"
#include "gstfcdec.h"

#define OUR_MIME_TYPE   "audio/x-futcomp"

#define DEFAULT_BLOCKSIZE    4096
#define DEFAULT_MAXSIZE    128*1024

#define GST_CAT_DEFAULT gst_fcdec_debug
GST_DEBUG_CATEGORY_STATIC (gst_fcdec_debug);

enum
{
    PROP_0,
    PROP_BLOCKSIZE,
    PROP_METADATA
};

static GstStaticPadTemplate sink_templ = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (OUR_MIME_TYPE)
    );

static GstStaticPadTemplate src_templ = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/x-raw-int, "
        "endianness = (int) BYTE_ORDER, "
        "signed = (boolean) { true, false }, "
        "width = (int) { 8, 16 }, "
        "depth = (int) { 8, 16 }, "
        "rate = (int) [ 11025, 48000 ], " "channels = (int) [ 1, 2 ]")
    );

GST_BOILERPLATE (GstFCDec, gst_fcdec, GstElement, GST_TYPE_ELEMENT);

static void gst_fcdec_base_init (gpointer g_class);
static void gst_fcdec_class_init (GstFCDecClass *gclass);
static void gst_fcdec_init (GstFCDec *fcdec, GstFCDecClass *gclass);
static void gst_fcdec_finalize (GObject *object);

static GstFlowReturn gst_fcdec_chain (GstPad *pad, GstBuffer *buf);
static gboolean gst_fcdec_sink_event (GstPad *pad, GstEvent *event);

static gboolean gst_fcdec_src_convert (GstPad *pad, GstFormat src_format,
    gint64 src_value, GstFormat *dest_format, gint64 *dest_value);

static gboolean gst_fcdec_src_event (GstPad *pad, GstEvent *event);
static gboolean gst_fcdec_src_query (GstPad *pad, GstQuery *query);

static gboolean gst_fcdec_set_caps (GstPad *pad, GstCaps *caps);

static void gst_fcdec_get_property (GObject *object, guint prop_id,
    GValue *value, GParamSpec *pspec);
static void gst_fcdec_set_property (GObject *object, guint prop_id,
    const GValue *value, GParamSpec *pspec);

/* GObject vmethod implementations */

static void
gst_fcdec_base_init (gpointer gclass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (gclass);

  gst_element_class_set_details_simple(element_class,
    "Future Composer decoder",
    "Codec/Decoder/Audio",
    "decodes AMIGA Future Composer modules",
    "Michael Schwendt <mschwendt@users.sf.net>");

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_templ));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&sink_templ));
}

static void
gst_fcdec_class_init (GstFCDecClass *gclass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) gclass;
  gstelement_class = (GstElementClass *) gclass;

  parent_class = GST_ELEMENT_CLASS (g_type_class_peek_parent (gclass));

  //gstelement_class->change_state = GST_DEBUG_FUNCPTR (gst_fcdec_parse_change_state);

  //gobject_class->dispose = gst_fcdec_dispose;
  gobject_class->finalize = gst_fcdec_finalize;
  gobject_class->set_property = gst_fcdec_set_property;
  gobject_class->get_property = gst_fcdec_get_property;

  g_object_class_install_property (G_OBJECT_CLASS (gclass), PROP_BLOCKSIZE,
      g_param_spec_ulong ("blocksize", "Block size",
          "Size in bytes to output per buffer", 1, G_MAXULONG,
          DEFAULT_BLOCKSIZE, (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property (gobject_class, PROP_METADATA,
      g_param_spec_boxed ("metadata", "Metadata", "Metadata", GST_TYPE_CAPS,
          (GParamFlags) G_PARAM_READABLE));
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad callback functions
 * initialize instance structure
 */
static void
gst_fcdec_init (GstFCDec *fcdec, GstFCDecClass *gclass)
{
  fcdec->sinkpad = gst_pad_new_from_static_template (&sink_templ, "sink");
  gst_pad_set_chain_function (fcdec->sinkpad, gst_fcdec_chain);
  gst_pad_set_event_function (fcdec->sinkpad, gst_fcdec_sink_event);
  //gst_pad_set_query_function (fcdec->sinkpad, NULL);
  gst_element_add_pad (GST_ELEMENT (fcdec), fcdec->sinkpad);
  //gst_pad_set_setcaps_function (fcdec->sinkpad, gst_fcdec_set_caps);
  //gst_pad_set_getcaps_function (fcdec->sinkpad, gst_pad_proxy_getcaps);

  fcdec->srcpad = gst_pad_new_from_static_template (&src_templ, "src");
  gst_pad_set_event_function (fcdec->srcpad, gst_fcdec_src_event);
  gst_pad_set_query_function (fcdec->srcpad, gst_fcdec_src_query);
  gst_pad_use_fixed_caps (fcdec->srcpad);
  //gst_pad_set_active (fcdec->srcpad, TRUE);
  gst_element_add_pad (GST_ELEMENT (fcdec), fcdec->srcpad);

  fcdec->decoder = fc14dec_new();
  
  fcdec->filebuf = (guchar *) g_malloc (DEFAULT_MAXSIZE);
  if (fcdec->filebuf) {
      fcdec->filebufsize = DEFAULT_MAXSIZE;
  }
  else {
      fcdec->filebufsize = 0;
  }
  fcdec->filelen = 0;
  fcdec->blocksize = DEFAULT_BLOCKSIZE;
  fcdec->totalbytes = 0;
  fcdec->nsecs = 0;
}

static void
gst_fcdec_finalize (GObject *object)
{
  GstFCDec *fcdec = GST_FCDEC (object);

  g_free (fcdec->filebuf);
  fc14dec_delete(fcdec->decoder);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
fcdec_negotiate (GstFCDec *fcdec)
{
  GstCaps *allowed;
  gboolean sign = TRUE;
  gint width = 16, depth = 16;
  
  GstStructure *structure;
  int rate = 44100;
  int channels = 1;
  GstCaps *caps;

  allowed = gst_pad_get_allowed_caps (fcdec->srcpad);
  if (!allowed) {
      goto nothing_allowed;
  }
  GST_DEBUG_OBJECT (fcdec, "allowed caps: %" GST_PTR_FORMAT, allowed);

  structure = gst_caps_get_structure (allowed, 0);

  gst_structure_get_int (structure, "width", &width);
  gst_structure_get_int (structure, "depth", &depth);
  if (width && depth && width != depth)
    goto wrong_width;
  width = width | depth;
  if (width) {
      fcdec->bits = width;
  }
  gst_structure_get_boolean (structure, "signed", &sign);
  gst_structure_get_int (structure, "rate", &rate);
  fcdec->frequency = rate;
  gst_structure_get_int (structure, "channels", &channels);
  fcdec->channels = channels;
  if (fcdec->bits == 8) {
      fcdec->zerosample = 0x80;
      sign = FALSE;
  }
  else {
      fcdec->zerosample = 0x0000;
      sign = TRUE;
  }
  caps = gst_caps_new_simple ("audio/x-raw-int",
      "endianness", G_TYPE_INT, G_BYTE_ORDER,
      "signed", G_TYPE_BOOLEAN, sign,
      "width", G_TYPE_INT, fcdec->bits,
      "depth", G_TYPE_INT, fcdec->bits,
      "rate", G_TYPE_INT, fcdec->frequency,
      "channels", G_TYPE_INT, fcdec->channels, NULL);
  gst_pad_set_caps (fcdec->srcpad, caps);
  gst_caps_unref (caps);

  return TRUE;

  /* ERRORS */
nothing_allowed:
  {
    GST_DEBUG_OBJECT (fcdec, "could not get allowed caps");
    return FALSE;
  }
wrong_width:
  {
    GST_DEBUG_OBJECT (fcdec, "width %d and depth %d are different",
        width, depth);
    return FALSE;
  }
}

static void
play_loop (GstPad *pad)
{
  GstFlowReturn ret;
  GstFCDec *fcdec;
  GstBuffer *out;
  gint64 value, offset, time;
  GstFormat format;

  fcdec = GST_FCDEC (gst_pad_get_parent (pad));

  out = gst_buffer_new_and_alloc (fcdec->blocksize);
  gst_buffer_set_caps (out, GST_PAD_CAPS (pad));

  fc14dec_buffer_fill(fcdec->decoder,GST_BUFFER_DATA(out),GST_BUFFER_SIZE(out));

  if (fc14dec_song_end(fcdec->decoder)) {
      gst_pad_pause_task (pad);
      gst_pad_push_event (pad, gst_event_new_eos ());
      goto done;
  }

  format = GST_FORMAT_DEFAULT;
  gst_fcdec_src_convert (fcdec->srcpad,
      GST_FORMAT_BYTES, fcdec->totalbytes, &format, &offset);
  GST_BUFFER_OFFSET (out) = offset;

  format = GST_FORMAT_TIME;
  gst_fcdec_src_convert (fcdec->srcpad,
      GST_FORMAT_BYTES, fcdec->totalbytes, &format, &time);
  GST_BUFFER_TIMESTAMP (out) = time;

  fcdec->totalbytes += fcdec->blocksize;

  format = GST_FORMAT_DEFAULT;
  gst_fcdec_src_convert (fcdec->srcpad,
      GST_FORMAT_BYTES, fcdec->totalbytes, &format, &value);
  GST_BUFFER_OFFSET_END (out) = value;

  format = GST_FORMAT_TIME;
  gst_fcdec_src_convert (fcdec->srcpad,
      GST_FORMAT_BYTES, fcdec->totalbytes, &format, &value);
  GST_BUFFER_DURATION (out) = value - time;

  if ((ret = gst_pad_push (fcdec->srcpad, out)) != GST_FLOW_OK)
    goto pause;

done:
  gst_object_unref (fcdec);
  return;

  /* ERRORS */
pause:
  {
    const gchar *reason = gst_flow_get_name (ret);

    GST_DEBUG_OBJECT (fcdec, "pausing task, reason %s", reason);
    gst_pad_pause_task (pad);

    if (GST_FLOW_IS_FATAL (ret) || ret == GST_FLOW_NOT_LINKED) {
      if (ret == GST_FLOW_UNEXPECTED) {
        /* perform EOS logic, FIXME, segment seek? */
        gst_pad_push_event (pad, gst_event_new_eos ());
      } else {
        /* for fatal errors we post an error message */
        GST_ELEMENT_ERROR (fcdec, STREAM, FAILED,
            (NULL), ("streaming task paused, reason %s", reason));
        gst_pad_push_event (pad, gst_event_new_eos ());
      }
    }
    goto done;
  }
}

static gboolean
start_play_file(GstFCDec *fcdec)
{
    if (!fcdec->filebuf || !fcdec->filelen ||
        !fc14dec_init(fcdec->decoder,fcdec->filebuf,fcdec->filelen) ) {
        GST_ELEMENT_ERROR (fcdec, LIBRARY, INIT,
                           ("Could not load FC module"), ("Could not load FC module"));
        return FALSE;
    }

    GstTagList *list = gst_tag_list_new();
    gst_element_found_tags_for_pad( GST_ELEMENT_CAST(fcdec), fcdec->srcpad, list);

    if (!fcdec_negotiate (fcdec)) {
        GST_ELEMENT_ERROR (fcdec, CORE, NEGOTIATION,
                           ("Could not negotiate format"), ("Could not negotiate format"));
        return FALSE;
    }

    /* printf("format = %s\n",fc14dec_format_name(fcdec->decoder)); */
    fcdec->nsecs = 1000*1000*fc14dec_duration(fcdec->decoder);
    fc14dec_mixer_init(fcdec->decoder,fcdec->frequency,fcdec->bits,fcdec->channels,fcdec->zerosample);
    
    gst_pad_push_event (fcdec->srcpad,
                        gst_event_new_new_segment (FALSE, 1.0, GST_FORMAT_TIME, 0, -1, 0));

    return gst_pad_start_task (fcdec->srcpad,
                               (GstTaskFunction) play_loop, fcdec->srcpad);
}

static gboolean
gst_fcdec_handle_seek (GstFCDec *fcdec, GstEvent *event)
{
  GstSeekType starttype, stoptype;
  GstSeekFlags flags;
  GstFormat format;
  gdouble rate;
  gint64 start, stop, jumppos;

  gst_event_parse_seek (event, &rate, &format, &flags, &starttype, &start,
      &stoptype, &stop);

  // TODO: convert this if necessary
  if (format != GST_FORMAT_TIME) {
    GST_DEBUG_OBJECT (fcdec, "only support seeks in TIME format");
    return FALSE;
  }

  gst_pad_push_event (fcdec->srcpad, gst_event_new_flush_start ());
    
  format = GST_FORMAT_BYTES;
  gst_fcdec_src_convert (fcdec->srcpad,
                         GST_FORMAT_TIME, start, &format, &fcdec->totalbytes);

  fc14dec_seek(fcdec->decoder, start/(1000*1000));

  gst_pad_push_event (fcdec->srcpad, gst_event_new_flush_stop ());
  gst_pad_push_event (fcdec->srcpad,
                      gst_event_new_new_segment (FALSE, rate, GST_FORMAT_TIME, start, -1, start));

  gst_pad_start_task (fcdec->srcpad,
                      (GstTaskFunction) play_loop, fcdec->srcpad);
  return TRUE;
}

static gboolean
gst_fcdec_sink_event (GstPad *pad, GstEvent *event)
{
  GstFCDec *fcdec;
  gboolean res;

  fcdec = GST_FCDEC (gst_pad_get_parent (pad));

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_EOS:
      res = start_play_file (fcdec);
      break;
    case GST_EVENT_NEWSEGMENT:
      res = FALSE;
      break;
    default:
      res = FALSE;
      break;
  }
  gst_event_unref (event);
  gst_object_unref (fcdec);

  return res;
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_fcdec_chain (GstPad *pad, GstBuffer *buf)
{
  GstFCDec *fcdec;
  guint64 size;

  fcdec = GST_FCDEC (gst_pad_get_parent (pad));

  size = GST_BUFFER_SIZE (buf);
  if (fcdec->filelen + size > fcdec->filebufsize) {
      fcdec->filebufsize += DEFAULT_MAXSIZE;
      fcdec->filebuf = (guchar *) g_realloc(fcdec->filebuf,fcdec->filebufsize);
      if ( !fcdec->filebuf ) {
          GST_ELEMENT_ERROR (fcdec, STREAM, DECODE,
                             (NULL), ("Input data buffer reallocation failed"));
          fcdec->filebufsize = fcdec->filelen = 0;
          gst_object_unref (fcdec);
          return GST_FLOW_ERROR;
      }
  }

  memcpy (fcdec->filebuf + fcdec->filelen, GST_BUFFER_DATA (buf), size);
  fcdec->filelen += size;

  gst_buffer_unref (buf);
  gst_object_unref (fcdec);

  return GST_FLOW_OK;
}

static gboolean
gst_fcdec_src_convert (GstPad *pad, GstFormat src_format, gint64 src_value,
    GstFormat *dest_format, gint64 *dest_value)
{
  gboolean res = TRUE;
  guint scale = 1;
  GstFCDec *fcdec;
  gint bytes_per_sample;

  fcdec = GST_FCDEC (gst_pad_get_parent (pad));

  if (src_format == *dest_format) {
    *dest_value = src_value;
    return TRUE;
  }

  bytes_per_sample = (fcdec->bits >> 3) * fcdec->channels;

  switch (src_format) {
    case GST_FORMAT_BYTES:
      switch (*dest_format) {
        case GST_FORMAT_DEFAULT:
          if (bytes_per_sample == 0)
            return FALSE;
          *dest_value = src_value / bytes_per_sample;
          break;
        case GST_FORMAT_TIME:
        {
          gint byterate = bytes_per_sample * fcdec->frequency;

          if (byterate == 0)
            return FALSE;
          *dest_value =
              gst_util_uint64_scale_int (src_value, GST_SECOND, byterate);
          break;
        }
        default:
          res = FALSE;
      }
      break;
    case GST_FORMAT_DEFAULT:
      switch (*dest_format) {
        case GST_FORMAT_BYTES:
          *dest_value = src_value * bytes_per_sample;
          break;
        case GST_FORMAT_TIME:
          if (fcdec->frequency == 0)
            return FALSE;
          *dest_value =
              gst_util_uint64_scale_int (src_value, GST_SECOND,
              fcdec->frequency);
          break;
        default:
          res = FALSE;
      }
      break;
    case GST_FORMAT_TIME:
      switch (*dest_format) {
        case GST_FORMAT_BYTES:
          scale = bytes_per_sample;
          // fallthrough
        case GST_FORMAT_DEFAULT:
          *dest_value =
              gst_util_uint64_scale_int (src_value,
              scale * fcdec->frequency, GST_SECOND);
          break;
        default:
          res = FALSE;
      }
      break;
    default:
      res = FALSE;
  }
  return res;
}

static gboolean
gst_fcdec_src_event (GstPad *pad, GstEvent *event)
{
  gboolean res = FALSE;
  GstFCDec *fcdec;

  fcdec = GST_FCDEC (gst_pad_get_parent (pad));

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_SEEK:
        res = gst_fcdec_handle_seek (fcdec, event);
        gst_event_unref( event);
        break;
    default:
        res = gst_pad_event_default (pad, event);
        break;
  }
  gst_object_unref (fcdec);

  return res;
}

static gboolean
gst_fcdec_src_query (GstPad *pad, GstQuery *query)
{
  gboolean res = TRUE;
  GstFCDec *fcdec;
  GstFormat format;

  fcdec = GST_FCDEC (gst_pad_get_parent (pad));

  switch (GST_QUERY_TYPE (query)) {
    case GST_QUERY_POSITION:
    {
      gint64 current;

      gst_query_parse_position (query, &format, NULL);
      /* we only know about our bytes, convert to requested format */
      res &= gst_fcdec_src_convert (pad,
           GST_FORMAT_BYTES, fcdec->totalbytes, &format, &current);
      if (res) {
        gst_query_set_position (query, format, current);
      }
      break;
    }
  case GST_QUERY_DURATION:
      gst_query_parse_duration (query, &format, NULL);
      GST_DEBUG_OBJECT(fcdec, "nsec song length: %" G_GUINT64_FORMAT, fcdec->nsecs);
      gint64 val;
      res = gst_fcdec_src_convert (pad, GST_FORMAT_TIME,
                                   fcdec->nsecs,
                                   &format, &val);
      if (res) {
          gst_query_set_duration (query, format, val);
      }
      break;
  case GST_QUERY_SEEKING:
      //break;
  default:
      res = gst_pad_query_default (pad, query);
      break;
  }
  gst_object_unref (fcdec);

  return res;
}

static void
gst_fcdec_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
    GstFCDec *fcdec = GST_FCDEC (object);
    
    switch (prop_id) {
    case PROP_BLOCKSIZE:
        fcdec->blocksize = g_value_get_ulong (value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
gst_fcdec_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstFCDec *fcdec = GST_FCDEC (object);

  switch (prop_id) {
    case PROP_BLOCKSIZE:
      g_value_set_ulong (value, fcdec->blocksize);
      break;
    case PROP_METADATA:
      g_value_set_boxed (value, NULL);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstElement vmethod implementations */

/* this function handles the link with other elements */
static gboolean
gst_fcdec_set_caps (GstPad * pad, GstCaps * caps)
{
  GstFCDec *fcdec;
  GstPad *otherpad;

  fcdec = GST_FCDEC (gst_pad_get_parent (pad));
  otherpad = (pad == fcdec->srcpad) ? fcdec->sinkpad : fcdec->srcpad;
  gst_object_unref (fcdec);

  return gst_pad_set_caps (otherpad, caps);
}

static void
gst_fcdec_type_find (GstTypeFind * tf, gpointer ignore)
{
    guint8 *data = gst_type_find_peek (tf, 0, 5);
    if (data == NULL)
        return;

    void *decoder = fc14dec_new();
    if (fc14dec_detect(decoder,data,5)) {
        gchar ourtype[] = OUR_MIME_TYPE;
        GST_DEBUG ("suggesting mime type %s", ourtype);
        GstCaps *caps = gst_caps_new_simple (ourtype, NULL);
        gst_type_find_suggest (tf, GST_TYPE_FIND_MAXIMUM, caps);
        gst_caps_unref (caps);
    }
    fc14dec_delete(decoder);
}

/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
fcdec_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (gst_fcdec_debug, "fcdec",
      0, "Future Composer decoder");

  GstCaps *caps = gst_caps_new_simple(OUR_MIME_TYPE,NULL);
  gst_type_find_register (plugin, OUR_MIME_TYPE, GST_RANK_PRIMARY,
                          gst_fcdec_type_find, NULL, caps, NULL, NULL);
  gst_caps_unref(caps);

  gboolean reg = gst_element_register (plugin, "fcdec", GST_RANK_PRIMARY,
                                       GST_TYPE_FCDEC);
  return reg;
}

/* gstreamer looks for this structure to register fcdecs */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "fcdec",
    "AMIGA Future Composer audio file decoder",
    fcdec_init,
    VERSION,
    "GPL",
    "GStreamer FC",
    "http://xmms-fc.sf.net/"
)
