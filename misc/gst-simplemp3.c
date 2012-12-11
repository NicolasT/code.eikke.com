/* Compile using "gcc `pkg-config --libs --cflags gstreamer-0.8` -o example1 example1.c" */
#include <gst/gst.h>
#include <glib.h>
int main(int argc, char *argv[]) {
        GstElement *pipeline, *filesrc, *decoder, *audiosink;

        gst_init(&argc, &argv);
        pipeline = gst_pipeline_new("test");
        filesrc = gst_element_factory_make("filesrc", "mysrc");
        g_object_set(G_OBJECT(filesrc), "location", argv[1], NULL);
        g_assert(filesrc != NULL);
        decoder = gst_element_factory_make("mad", "decoder");
        g_assert(decoder != NULL);
        audiosink = gst_element_factory_make("alsasink", "out");
        g_assert(audiosink != NULL);
        gst_bin_add_many(GST_BIN(pipeline), filesrc, decoder, audiosink, NULL);
        gst_element_link_many(filesrc, decoder, audiosink, NULL);
        gst_element_set_state(pipeline, GST_STATE_PLAYING);
        while(gst_bin_iterate(GST_BIN(pipeline)));
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(GST_OBJECT(pipeline));
        return 0;
}
