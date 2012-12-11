#include <glib.h>
#include <dbus/dbus-glib.h>

#include "dbus-test.h"

static void
test_hint_signal_handler (DBusGProxy * proxy, const gchar * hint,
			 gpointer user_data)
{
  g_assert (hint != NULL);

  g_debug ("Hint: %s", hint);
}

gint
main (gint argc, gchar *argv[])
{
  DBusGConnection *dbus_glib_connection = NULL;
  GMainLoop *loop = NULL;
  GError *error = NULL;
  DBusGProxy *control_proxy = NULL;

  g_type_init ();

  loop = g_main_loop_new (NULL, FALSE);

  dbus_glib_connection = dbus_g_bus_get (DBUS_BUS_SYSTEM, &error);
  if (error != NULL)
    {
      g_warning ("Failed connecting to system bus: %s", error->message);
      dbus_glib_connection = NULL;
      g_error_free (error);
    }
  if (dbus_glib_connection != NULL)
    {
      control_proxy =
	dbus_g_proxy_new_for_name (dbus_glib_connection, "" TEST_SERVICE_NAME,
				   "" TEST_SERVICE_PATH_NAME,
				   "" TEST_SERVICE_INTERFACE_NAME);
      if (control_proxy == NULL)
	{
	  g_warning ("Failed to get proxy");
	}
    }
  if (control_proxy != NULL)
    {
      dbus_g_proxy_add_signal (control_proxy, "" TEST_SIGNAL_NAME, G_TYPE_STRING,
			       G_TYPE_INVALID);
      dbus_g_proxy_connect_signal (control_proxy, "" TEST_SIGNAL_NAME,
				   G_CALLBACK (test_hint_signal_handler), NULL,
				   NULL);
    }

  g_debug("Starting mainloop");
  g_main_loop_run (loop);

  return 0;
}
