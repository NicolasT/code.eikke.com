#include <X11/Xproto.h>         /* for CARD32 */
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>          /* for XGetClassHint() */
#include <X11/Xos.h>

#include <X11/Intrinsic.h>      /* only needed to get through xscreensaver.h */

#include <stdio.h>

#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>

# define DBUSPATH       "/com/eikke/XScreensaverDbus"
# define DBUSURI        "com.eikke.XScreensaverDbus"
# define DBUSDOMAIN     "com.eikke"
# define DBUSMESSAGE_STARTED    "ScreensaverStarted"
# define DBUSMESSAGE_STOPPED    "ScreensaverStopped"

#define MAX_MSG_LENGTH 255

Atom XA_BLANK, XA_LOCK;
Atom XA_SCREENSAVER_STATUS;

void sendDbusMessage(char *message)
{
        DBusConnection *dbusConnection;
        DBusError dbusError;
        DBusMessage *dbusMessage;

        printf("Trying to send message: %s\n", message);

        dbus_error_init(&dbusError);

        dbusConnection = dbus_bus_get(DBUS_BUS_SESSION, &dbusError);
        if (dbusConnection == NULL) {
                fprintf(stderr, "[DBUS] Cannot connect to session message bus: error (%s): %s\n", dbusError.name, dbusError.message);
                dbus_error_free(&dbusError);
                return;
        }

        dbus_bus_acquire_service(dbusConnection, DBUSDOMAIN, 0, &dbusError);
        if (dbus_error_is_set(&dbusError)) {
                fprintf(stderr, "[DBUS] Cannot acquire " DBUSDOMAIN " service, error (%s): %s\n", dbusError.name, dbusError.message);
                goto end;
        }

        /* Send message */
        if (dbus_connection_get_is_connected(dbusConnection)) {
                /* Connection is ok, we can send out a message */
                dbusMessage = dbus_message_new_signal(DBUSPATH, DBUSURI, message);
                if (dbusMessage == NULL) {
                        fprintf(stderr, "[DBUS] Error creating DBUS message.\n");
                        goto end;
                }

                if (dbusMessage == NULL) {
                        fprintf(stderr, "[DBUS] Error creating DBUS message.\n");
                        goto end;
                }

                if (!dbus_connection_send(dbusConnection, dbusMessage, NULL))
                        fprintf(stderr, "[DBUS] Error sending DBUS message.\n");
                dbus_message_unref(dbusMessage);
                dbus_connection_flush(dbusConnection);
        }

end:
        dbus_connection_disconnect(dbusConnection);
        dbus_error_free(&dbusError);
}

void watch(Display *dpy)
{
  char *v = 0;
  char *message = (char *)malloc(MAX_MSG_LENGTH * sizeof(char));
  Window window = RootWindow (dpy, 0);
  XWindowAttributes xgwa;
  XEvent event;
  CARD32 *last = 0;

  if (v) free (v);
  XGetWindowAttributes (dpy, window, &xgwa);
  XSelectInput (dpy, window, xgwa.your_event_mask | PropertyChangeMask);

  while (1)
    {
      XNextEvent (dpy, &event);
      if (event.xany.type == PropertyNotify &&
          event.xproperty.state == PropertyNewValue &&
          event.xproperty.atom == XA_SCREENSAVER_STATUS)
        {
          Atom type;
          int format;
          unsigned long nitems, bytesafter;
          unsigned char *dataP = 0;

          if (XGetWindowProperty (dpy,
                                  RootWindow (dpy, 0),  /* always screen #0 */
                                  XA_SCREENSAVER_STATUS,
                                  0, 999, False, XA_INTEGER,
                                  &type, &format, &nitems, &bytesafter,
                                  &dataP)
              == Success
              && type
              && dataP)
            {
              time_t tt;
              char *s;
              Bool changed = False;
              Bool running = False;
              CARD32 *data = (CARD32 *) dataP;

              if (type != XA_INTEGER || nitems < 3)
                {
                STATUS_LOSE:
                  if (last) XFree (last);
                  if (data) XFree (data);
                  fprintf (stderr, "bad status format on root window.\n");
                  goto end;
                }

              tt = (time_t) data[1];
              if (tt <= (time_t) 666000000L) /* early 1991 */
                goto STATUS_LOSE;
              s = ctime(&tt);
              if (s[strlen(s)-1] == '\n')
                s[strlen(s)-1] = 0;

              if (!last || data[0] != last[0])
                {
                  /* State changed. */
                  if (data[0] == XA_BLANK)
                  {
                    snprintf (message, MAX_MSG_LENGTH, "BLANK %s\n", s);
                    sendDbusMessage(message);
                  }
                  else if (data[0] == XA_LOCK)
                  {
                    snprintf (message, MAX_MSG_LENGTH, "LOCK %s\n", s);
                    sendDbusMessage(message);
                  }
                  else if (data[0] == 0)
                  {
                    snprintf (message, MAX_MSG_LENGTH, "UNBLANK %s\n", s);
                    sendDbusMessage(message);
                  }
                  else
                    goto STATUS_LOSE;
                }

              if (!last)
                changed = True;
              else
                {
                  int i;
                  for (i = 2; i < nitems; i++)
                    {
                      if (data[i] != last[i])
                        changed = True;
                      if (data[i])
                        running = True;
                    }
                }

              if (running && changed)
                {
                  int i;
                  fprintf (stdout, "RUN");
                  for (i = 2; i < nitems; i++)
                    fprintf (stdout, " %d", (int) data[i]);
                  fprintf (stdout, "\n");
                }

              fflush (stdout);

              if (last) XFree (last);
              last = data;
            }
          else
            {
              if (last) XFree (last);
              if (dataP) XFree (dataP);
              fprintf (stderr, "no saver status on root window.\n");
              goto end;
}
}
}
end:
        free(message);
}

int main()
{
        Display *dpy;

        dpy = XOpenDisplay (":0.0");
        if (!dpy)
        {
                fprintf (stderr, "can't open display");
                exit (1);
        }
          
        XA_SCREENSAVER_STATUS = XInternAtom (dpy, "_SCREENSAVER_STATUS", False);
        XA_LOCK = XInternAtom (dpy, "LOCK", False);
        XA_BLANK = XInternAtom (dpy, "BLANK", False);

        XSync (dpy, 0);
        
        watch(dpy);
        return 0;
}
