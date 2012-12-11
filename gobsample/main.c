#include <glib.h>
#include "sample-object.h"

gint main(gint argc, const gchar *argv[]) {
        SampleObject *obj = NULL;

        g_type_init();

        obj = SAMPLE_OBJECT(sample_object_new(5));
        sample_object_do_something(obj);

        g_object_unref(obj);

        return 0;
}
