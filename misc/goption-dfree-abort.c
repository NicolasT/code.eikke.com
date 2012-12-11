#include <glib.h>

int main(int argc, char *argv[]) {
        gboolean b = FALSE;
        gboolean c = FALSE;
        gchar *s = NULL;
        GOptionEntry entries[] = {
                { "booleantest", 'b', 0, G_OPTION_ARG_NONE, &b, "BooleanTest", NULL },
                { "stringtestblablabla", 's', 0, G_OPTION_ARG_STRING, &s, "StringTest", "S" },
                { NULL }
        };
        GOptionContext *goCtx;
        GOptionGroup *group;
        GOptionEntry groupentries[] = {
                { "bool", 'c', 0, G_OPTION_ARG_NONE, &c, "Bool test in group", NULL },
                { NULL }
        };

        goCtx = g_option_context_new("- The ultimate GOption sample");
        g_option_context_add_main_entries(goCtx, entries, "test");
        group = g_option_group_new("testgroup", "this is some test group", "help description", NULL, NULL);
        g_option_group_add_entries(group, groupentries);
        g_option_context_add_group(goCtx, group);
        g_option_context_parse(goCtx, &argc, &argv, NULL);
        g_option_group_free(group);
        g_option_context_free(goCtx);

        g_print("'boolean' b == '%s'\n", (b == TRUE? "TRUE" : "FALSE"));
        g_print("'boolean' b == '%s'\n", (c == TRUE? "TRUE" : "FALSE"));
        g_print("'string' s == '%s'\n", s);
        
        return 0;
}
