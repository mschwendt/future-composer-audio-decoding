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

#ifndef __GST_FCDEC_H__
#define __GST_FCDEC_H__

#include <gst/gst.h>

G_BEGIN_DECLS

/* #defines don't like whitespacey bits */
#define GST_TYPE_FCDEC \
  (gst_fcdec_get_type())
#define GST_FCDEC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_FCDEC,GstFCDec))
#define GST_FCDEC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_FCDEC,GstFCDecClass))
#define GST_IS_FCDEC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_FCDEC))
#define GST_IS_FCDEC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_FCDEC))

typedef struct _GstFCDec      GstFCDec;
typedef struct _GstFCDecClass GstFCDecClass;

struct _GstFCDec
{
  GstElement element;

  GstPad *sinkpad, *srcpad;

  guchar *filebuf;
  gint filebufsize, filelen;
  gint64 totalbytes;

  gulong blocksize;

  gint frequency, bits, channels, zerosample;
  gint64 nsecs;
};

struct _GstFCDecClass 
{
  GstElementClass parent_class;
};

GType gst_fcdec_get_type (void);

G_END_DECLS

#endif /* __GST_FCDEC_H__ */
