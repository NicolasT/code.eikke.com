#include <gtk/gtk.h>
#include "gtk-oxo-widget.h"

static void oxo_winner(GtkOxoWidget *oxo, gpointer data) {
        g_print("Winner move!\n");
}

static void oxo_invalid_move(GtkOxoWidget *oxo, guint row, guint col, gpointer data) {
        g_print("Invalid move on %d:%d\n", row, col);
}

static void oxo_move(GtkOxoWidget *oxo, guint row, guint col, gpointer data) {
        g_print("Move on %d:%d\n", row, col);
}

gint main(gint argc, gchar *argv[]) {
        GtkWidget *window = NULL, *oxo = NULL;

        gtk_init(&argc, &argv);

        window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        oxo = gtk_oxo_widget_new(5, 5);
        g_assert(window != NULL);
        g_assert(oxo != NULL);
        gtk_container_add(GTK_CONTAINER(window), oxo);

        g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
        gtk_oxo_widget_connect__winner(GTK_OXO_WIDGET(oxo), oxo_winner, NULL);
        gtk_oxo_widget_connect__invalid_move(GTK_OXO_WIDGET(oxo), oxo_invalid_move, NULL);
        gtk_oxo_widget_connect__move(GTK_OXO_WIDGET(oxo), oxo_move, NULL);

        gtk_widget_show_all(window);

        gtk_main();

        return 0;
}
