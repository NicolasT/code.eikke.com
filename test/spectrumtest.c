/*
 * Spectrum analyser using GStreamer
 * Source is a simple sinesrc
 * Code based on the spectrum analyser sample in CVS, using OSS input
 * See http://cvs.freedesktop.org/gstreamer/gst-plugins/gst/spectrum/demo-osssrc.c?rev=1.9&content-type=text%2Fplain
 *
 * You need gstreamer-0.8 and gtk+-2 to compile and run this
 */

/*
 * Expected result: one stable peak around 440Hz
 * Result: something very fuzzy
 */

#include <gst/gst.h>
#include <gtk/gtk.h>

/* For testing purposes, set the desired frequency of the sine wave */
/* Results do vary, but are not what I expect them to be */
/* Maybe it's just me */
#define SINEFREQ 440

extern gboolean _gst_plugin_spew;

gboolean idle_func (gpointer data);

GtkWidget *drawingarea;

/* Draw the spectrum in some simple GTK window */
void
spectrum_chain (GstElement * sink, GstBuffer * buf, GstPad * pad,
    gpointer unused)
{
  gint i;
  guchar *data = buf->data;
  GdkRectangle rect = { 0, 0, GST_BUFFER_SIZE (buf), 25 };

  gdk_window_begin_paint_rect (drawingarea->window, &rect);
  gdk_draw_rectangle (drawingarea->window, drawingarea->style->black_gc,
      TRUE, 0, 0, GST_BUFFER_SIZE (buf), 25);
  for (i = 0; i < GST_BUFFER_SIZE (buf); i++) {
    gdk_draw_rectangle (drawingarea->window, drawingarea->style->white_gc,
        TRUE, i, 32 - data[i], 1, data[i]);
  }
  gdk_window_end_paint (drawingarea->window);
}

int
main (int argc, char *argv[])
{
  /* Create all our elements */
  GstElement *bin;
  GstElement *src, *spectrum, *sink;

  GtkWidget *appwindow;

  gst_init (&argc, &argv);
  gtk_init (&argc, &argv);

  bin = gst_pipeline_new ("bin");

  src = gst_element_factory_make ("sinesrc", "src");
  /* Use same buffer size as in the osssrc sample */
  g_object_set(G_OBJECT(src), "samplesperbuffer", (gulong) 1024, NULL);
  /* We can change this, for testing purpose */
  g_object_set(G_OBJECT(src), "freq", (double) SINEFREQ, NULL);
  spectrum = gst_element_factory_make ("spectrum", "spectrum");
  g_object_set (G_OBJECT (spectrum), "width", 256, NULL);
  sink = gst_element_factory_make ("fakesink", "sink");
  g_object_set (G_OBJECT (sink), "signal-handoffs", TRUE, NULL);
  g_signal_connect (sink, "handoff", G_CALLBACK (spectrum_chain), NULL);

  gst_bin_add_many (GST_BIN (bin), src, spectrum, sink, NULL);
  gst_element_link_many (src, spectrum, sink, NULL);

  appwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  drawingarea = gtk_drawing_area_new ();
  gtk_drawing_area_size (GTK_DRAWING_AREA (drawingarea), 256, 32);
  gtk_container_add (GTK_CONTAINER (appwindow), drawingarea);
  gtk_widget_show_all (appwindow);

  gst_element_set_state (GST_ELEMENT (bin), GST_STATE_PLAYING);

  g_idle_add (idle_func, bin);

  gtk_main ();

  return 0;
}


gboolean
idle_func (gpointer data)
{
  return gst_bin_iterate (data);
}
