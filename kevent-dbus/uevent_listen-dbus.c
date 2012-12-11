/*
 * uevent_listen-dbus.c - dump netlink uevents from the kernel to stdout/dbus
 *
 * Sample Makefile:
 *      uevent_listen: uevent_listen.c
 *              gcc -DDEBUG `pkg-config --cflags --libs dbus-1` -g -o uevent_listen uevent_listen.c
 *
 *
 * Copyright (C) 2005 Nicolas Trangez <eikke@eikke.com>
 * 
 *      Based on uevents_listen.c by Kay Sievers (http://www.us.kernel.org/pub/linux/utils/kernel/hotplug/uevent_listen.c)
 *      
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License as published by the
 *	Free Software Foundation version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful, but
 *	WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *	General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License along
 *	with this program; if not, write to the Free Software Foundation, Inc.,
 *	675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/user.h>
#include <asm/types.h>
#include <linux/netlink.h>

#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>

/* environment buffer, the kernel's size in lib/kobject_uevent.c should fit in */
#define HOTPLUG_BUFFER_SIZE		1024
#define HOTPLUG_NUM_ENVP		32
#define OBJECT_SIZE			512

/* debug stuff */
#ifndef __GNUC__
        #define __FUNCTION__    ""
        #define __LINE__        ""
#endif
#ifdef DEBUG
        #undef DEBUG
        #define DEBUG(fmt, arg...)                                      \
                do {                                                    \
                        printf("%s (%i): " fmt "\n", __FUNCTION__, __LINE__, ## arg); \
                } while (0)
#else
        #undef DEBUG
        #define DEBUG(fmt, arg...)                                      \
                do {} while(0)
#endif

/* error handling */
#define ERROR(fmt, arg...)                                              \
        do {                                                            \
                printf("%s: " fmt "\n", __FUNCTION__, ## arg);      \
        } while (0)

/* This is an ugly hack. It hard-defines the netlink socket number (as in Linux kernel 2.6.10) if it isn't defined */
/* This could be so when older headers are uses in /usr/include/linux or something alike */
#ifndef NETLINK_KOBJECT_UEVENT
        #define NETLINK_KOBJECT_UEVENT  15      /* Kernel messages to userspace */
#endif

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_nl snl;
	int retval;

        DBusConnection *dbusConnection;
        DBusError dbusError;

	if (getuid() != 0) {
		ERROR("Need to be root to run this tool, exit.");
		exit(1);
	}

	memset(&snl, 0x00, sizeof(struct sockaddr_nl));
	snl.nl_family = AF_NETLINK;
	snl.nl_pid = getpid();
        /* this is "all groups", intended like this? */
	snl.nl_groups = 0xffffffff;

	sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (sock == -1) {
		ERROR("Error getting socket, exit.");
		exit(1);
	}

	retval = bind(sock, (struct sockaddr *) &snl,
		      sizeof(struct sockaddr_nl));
	if (retval < 0) {
		ERROR("Bind failed, exit.");
		goto exit;
	}

        /* Initialize our DBus connection */
        dbus_error_init(&dbusError);
        dbusConnection = dbus_bus_get(DBUS_BUS_SYSTEM, &dbusError);
        if (dbusConnection == NULL) {
                ERROR("Cannot connect to system message bus, error (%s): %s", dbusError.name, dbusError.message);
                dbus_error_free(&dbusError);
                goto exit;
        }

        dbus_bus_acquire_service(dbusConnection, "org.kernel", 0, &dbusError);
        if (dbus_error_is_set(&dbusError)) {
                ERROR("Cannot acquire org.kernel service, error (%s): %s", dbusError.name, dbusError.message);
                goto dbusDisconnect;
        }

	while (1) {
		static char buffer[HOTPLUG_BUFFER_SIZE + OBJECT_SIZE];
		static char object[OBJECT_SIZE];
		const char *ueDevpath;
		const char *ueAction;
		const char *envp[HOTPLUG_NUM_ENVP];
		int i;
		char *pos;
		size_t bufpos;
		ssize_t buflen;
                DBusMessage *dbusMessage;
                DBusMessageIter dbusIter;

		buflen = recv(sock, &buffer, sizeof(buffer), 0);
		if (buflen <  0) {
			DEBUG("Error receiving message\n");
			continue;
		}

		if ((size_t)buflen > sizeof(buffer)-1)
			buflen = sizeof(buffer)-1;

		buffer[buflen] = '\0';

		/* save start of payload */
		bufpos = strlen(buffer) + 1;

		/* action string */
		ueAction = buffer;
		pos = strchr(buffer, '@');
		if (!pos)
			continue;
		pos[0] = '\0';

		/* sysfs path */
		ueDevpath = &pos[1];

		/* hotplug events have the environment attached - reconstruct envp[] */
		for (i = 0; (bufpos < (size_t)buflen) && (i < HOTPLUG_NUM_ENVP-1); i++) {
			int keylen;
			char *key;

			key = &buffer[bufpos];
			keylen = strlen(key);
			envp[i] = key;
			bufpos += keylen + 1;
		}
		envp[i] = NULL;

                if (!dbus_connection_get_is_connected(dbusConnection)) {
                        DEBUG("Lost DBus connection... Exiting.");
                        goto exit;
                }
                
		DEBUG("[%i] received '%s' from '%s'", time(NULL), ueAction, ueDevpath);

                /* generate and send DBUS message */
                dbusMessage = dbus_message_new_signal("/org/kernel/KobjectUevent", "org.kernel.KobjectUevent", "KobjectUevent");
                dbus_message_append_iter_init(dbusMessage, &dbusIter);

                /* add payload */
                for(i = 0; envp[i] != NULL; i++) {
                        DEBUG("%s", envp[i]);
                        dbus_message_iter_append_string(&dbusIter, envp[i]);
                }
                DEBUG("");
                
                if (dbusMessage == NULL) {
                        DEBUG("Error creating DBUS message.");
                        goto exit;
                }
                if (!dbus_connection_send(dbusConnection, dbusMessage, NULL))
                        DEBUG("Error sending DBUS message.");
                dbus_message_unref(dbusMessage);
                dbus_connection_flush(dbusConnection);
        }

dbusDisconnect:
        dbus_connection_disconnect(dbusConnection);
        
exit:
	close(sock);
	exit(1);
}
