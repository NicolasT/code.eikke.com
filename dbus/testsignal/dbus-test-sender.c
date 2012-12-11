#define DBUS_API_SUBJECT_TO_CHANGE 1

#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "dbus-test.h"

int
main (int argc, char *argv[])
{
  dbus_uint32_t serial = 0;	// unique number to associate replies with requests
  DBusMessage *msg;
  DBusMessageIter args;
  DBusConnection *conn;
  char *t = "test";

  conn = dbus_bus_get (DBUS_BUS_SYSTEM, NULL);
  assert (conn != NULL);

  // create a signal and check for errors 
  msg = dbus_message_new_signal ("" TEST_SERVICE_NAME,	// object name of the signal
                                 "" TEST_SERVICE_INTERFACE_NAME,	// interface name of the signal
				 "" TEST_SIGNAL_NAME);	// name of the signal
  if (NULL == msg)
    {
      printf ("Message Null\n");
      exit (1);
    }

  // append arguments onto signal
  dbus_message_iter_init_append (msg, &args);
  if (!dbus_message_iter_append_basic (&args, DBUS_TYPE_STRING, &t))
    {
      printf ("Out Of Memory!\n");
      exit (1);
    }

  // send the message and flush the connection
  if (!dbus_connection_send (conn, msg, &serial))
    {
      printf ("Out Of Memory!\n");
      exit (1);
    }
  dbus_connection_flush (conn);

  // free the message 
  dbus_message_unref (msg);

  printf("Sent!\n");

  return 0;
}
