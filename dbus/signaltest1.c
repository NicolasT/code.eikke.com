#include <glib.h>
#include <dbus/dbus-glib.h>

#include "dbus-test.h"

#define TEST_SERVICE_NAME "com.eikke.test"
#define TEST_SERVICE_PATH_NAME "/com/eikke/test/Hint"
#define TEST_SERVICE_INTERFACE_NAME "com.eikke.test"
#define TEST_SIGNAL_NAME "TestHint"

static DBusGConnection *dbus_glib_connection = NULL;

static void
test_hint_signal_handler (DBusGProxy * proxy, const gchar * hint,
			  gpointer user_data)
{
  g_debug ("In signal handler");
  g_assert (hint != NULL);

  g_debug ("Hint: %s", hint);
}

static gboolean
send_signal (gpointer d)
{
  DBusGProxy *p = (DBusGProxy *) d;
  g_assert(p != NULL);

  g_debug ("Sending signal");
  dbus_g_proxy_call_no_reply (p, "" TEST_SIGNAL_NAME, G_TYPE_STRING, "testje",
			      G_TYPE_INVALID);
  dbus_g_connection_flush (dbus_glib_connection);

  return TRUE;
}

gint
main (gint argc, gchar * argv[])
{
  GMainLoop *loop = NULL;
  GError *error = NULL;
  DBusGProxy *control_proxy = NULL;

  g_type_init ();

  loop = g_main_loop_new (NULL, FALSE);

  dbus_glib_connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (error != NULL)
    {
      g_warning ("Failed connecting to system bus: %s", error->message);
      dbus_glib_connection = NULL;
      g_error_free (error);
      return 1;
    }
  g_assert (dbus_glib_connection != NULL);

  control_proxy =
    dbus_g_proxy_new_for_name (dbus_glib_connection, "" TEST_SERVICE_NAME,
			       "" TEST_SERVICE_PATH_NAME,
			       "" TEST_SERVICE_INTERFACE_NAME);
  if (control_proxy == NULL)
    {
      g_error ("Failed to get proxy");
    }

  dbus_g_proxy_add_signal (control_proxy, "" TEST_SIGNAL_NAME, G_TYPE_STRING,
			   G_TYPE_INVALID);
  dbus_g_proxy_connect_signal (control_proxy, "" TEST_SIGNAL_NAME,
			       G_CALLBACK (test_hint_signal_handler), NULL,
			       NULL);

  g_debug ("Starting mainloop");

  g_timeout_add (2 * 1000, send_signal, control_proxy);
  g_main_loop_run (loop);

  return 0;
}
