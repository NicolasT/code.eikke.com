#include <stdio.h>
#include <dbus/dbus.h>

int main()
{
        DBusConnection *conn;
        DBusMessage *message;
        DBusError error;

        dbus_error_init(&error);

        conn = dbus_bus_get(DBUS_BUS_SESSION, &error);
        if(!conn)
        {
                fprintf(stderr, "Cant get connection\n");
        }

        message = dbus_message_new_signal("/com/eikke/test/ping", "com.eikke.test.ping", "Ping");
        dbus_message_append_args(message, DBUS_TYPE_STRING, "test", DBUS_TYPE_INVALID);

        dbus_connection_send(conn, message, NULL);
        dbus_message_unref(message);

        dbus_error_free(&error);

        return 0;
}
