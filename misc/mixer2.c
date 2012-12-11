#include <gst/gst.h>

#define FREQ1 440.00

#define VOL1 1.00
#define VOL2 1.00

#define MP3 ("test.mp3")

int main(int argc, char *argv[]) {
        GstElement *pipeline, *src1, *filesrc, *src2, *volume1, *volume2, *adder, *out;
        GstPad *pad;

        gst_init (&argc, &argv);

        g_print("Creating elements... ");
        {
                pipeline = gst_pipeline_new("mixer");
                g_assert(pipeline != NULL);
        
                src1 = gst_element_factory_make("sinesrc", "sine1");
                g_assert(src1 != NULL);

                /* src2 should result in a decoded stream... */
                filesrc = gst_element_factory_make("filesrc", "file1");
                g_assert(filesrc != NULL);
                src2 = gst_element_factory_make("decodebin", "decoder1");
                g_assert(src2 != NULL);

                volume1 = gst_element_factory_make("volume", "volume1");
                g_assert(volume1 != NULL);
                volume2 = gst_element_factory_make("volume", "volume2");
                g_assert(volume2 != NULL);

                adder = gst_element_factory_make("adder", "adder1");
                g_assert(adder != NULL);

                out = gst_element_factory_make("alsasink", "out");
                g_assert(out != NULL);
        }
        g_print("done\n");

        g_print("Setting sine frequencies... ");
        {
                g_object_set(G_OBJECT(src1), "freq", (gdouble) FREQ1, NULL);
                g_object_set(G_OBJECT(filesrc), "location", (const gchar *) MP3, NULL);
        }
        g_print("done\n");

        g_print("Setting mixer volumes... ");
        {
                g_object_set(G_OBJECT(volume1), "volume", (gdouble) VOL1, NULL);
                g_object_set(G_OBJECT(volume2), "volume", (gdouble) VOL2, NULL);
        }
        g_print("done\n");

        g_print("Linking elements... ");
        {
                /* filesrc to decoderbin */
                gst_element_link(filesrc, src2);
                
                /* Sine generators go to volume control */
                gst_element_link(src1, volume1);

                /* decoder src2 to volume2 */
                g_print("\n\tRequesting src path of decodebin src2\n");
                pad = gst_element_get_request_pad(src2, "src%d");
                g_assert(pad != NULL);
                g_print("\tGot new decoder src pad '%s'\n", gst_pad_get_name(pad));
                gst_pad_link(pad, gst_element_get_pad(volume2, "sink"));
                
                /* Adder output goes to soundcard */
                gst_element_link(adder, out);

                /* Now link volume outs to adder ins, on request */
                /* src #1 */
                pad = gst_element_get_request_pad(adder, "sink%d");
                g_assert(pad != NULL);
                g_print("\tGot new adder sink pad '%s'\n", gst_pad_get_name (pad));
                gst_pad_link(gst_element_get_pad(volume1, "src"), pad);
                /* Same thing for #2 */
                pad = gst_element_get_request_pad (adder, "sink%d");
                g_assert(pad != NULL);
                g_print("\tGot new adder sink pad '%s'\n", gst_pad_get_name (pad));
                gst_pad_link(gst_element_get_pad(volume2, "src"), pad);
        }
        g_print("done\n");

        g_print("Adding elements to pipeline... ");
        {
                gst_bin_add_many(GST_BIN(pipeline), src1, filesrc, src2, volume1, volume2, adder, out, NULL);
        }
        g_print("done\n");

        g_print("Starting playback... ");
        {
                gst_element_set_state(pipeline, GST_STATE_PLAYING);
        }
        g_print("done\n");

        g_print("Playing... ");
        {
                while(gst_bin_iterate(GST_BIN (pipeline)));
        }
        g_print("stop\n");

        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(GST_OBJECT (pipeline));

        return 0;
}
