#include <gst/gst.h>
#include <glib.h>
int main(int argc, char *argv[]) {
        GstElement *pipeline, *filesrc, *demuxer, *decoder, *converter, *audiosink;

        gst_init(&argc, &argv);
        pipeline = gst_pipeline_new("test");
        filesrc = gst_element_factory_make("filesrc", "mysrc");
        g_object_set(G_OBJECT(filesrc), "location", argv[1], NULL);
        g_assert(filesrc != NULL);
        demuxer = gst_element_factory_make("oggdemux", "demux");
        g_assert(demuxer != NULL);
        decoder = gst_element_factory_make("vorbisdec", "decoder");
        g_assert(decoder != NULL);
        converter = gst_element_factory_make("audioconvert", "convert");
        g_assert(converter != NULL);
        audiosink = gst_element_factory_make("alsasink", "out");
        g_assert(audiosink != NULL);
        gst_bin_add_many(GST_BIN(pipeline), filesrc, demuxer, decoder, converter, audiosink, NULL);
        gst_element_link_many(filesrc, demuxer, decoder, converter, audiosink, NULL);
        gst_element_set_state(pipeline, GST_STATE_PLAYING);
        while(gst_bin_iterate(GST_BIN(pipeline)));
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(GST_OBJECT(pipeline));
        return 0;
}
