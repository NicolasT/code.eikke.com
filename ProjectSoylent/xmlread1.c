/*
 * Fetch a file using GnomeVFS and create an XmlDocument out of it
 *
 * Copyright (C) 2005 Nicolas "Ikke" Trangez (eikke eikke commercial)
 * Code based on code by Chrsitian Kellner <gicmo@gnome.org>. Thanks a lot!
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

/* Compile using
 *      gcc -o xmlread1 `pkg-config --cflags --libs gnome-vfs-2.0 libxml-2.0` xmlread1.c
 * Then run
 *      ./xmlread1 http://blog.eikke.com/xmlsrv/rss2.php?blog=5
 */

#include <glib.h>
#include <libgnomevfs/gnome-vfs.h>

#include <libxml/tree.h>

#define BUFSIZE 4096

gint main (gint argc, gchar *argv[])
{
        /* GnomeVFS does not use GError's for error handling, but return values */
	GnomeVFSResult result;
        /* A "File Descriptor" is called a "Handle" in GnomeVFS */
	GnomeVFSHandle *handle;
        /* These are actually just ints */
	GnomeVFSFileSize bytes_read;
	GnomeVFSFileSize space = 0, total = 0;
        /* A pointer to an XmlDocument */
	xmlDocPtr doc;
        /* This string will contain the raw data we get (e.g. from HTTP) */
	gchar *document = NULL;

	if (argc < 2) {
		g_print("Usage: %s <location>\n", argv[0]);
		return 1;
	}

        /* Init the GnomeVFS framework */
	if (!gnome_vfs_init()) {
		g_error("Cannot initialize gnome-vfs.\n");
		return 1;
	}
	
        /* Open the URI */
	result = gnome_vfs_open (&handle, argv[1], GNOME_VFS_OPEN_READ);
	
        /* An error occured... */
	if(result != GNOME_VFS_OK) {
		g_error("%s\n", gnome_vfs_result_to_string(result));
		return 1;
	}
	
	do {
		if(space == 0) {
                        /* We did not get any data yet */
			space = BUFSIZE;
                        /* Enlarge our buffer */
			document = g_renew (gchar, document, total + space);
		}

                /* Read max. "space" bytes, and append them to our buffer */
		result = gnome_vfs_read (handle, document + total, space, &bytes_read);
		
		total += bytes_read;
		space -= bytes_read;

	} while(result == GNOME_VFS_OK);

	if(result != GNOME_VFS_ERROR_EOF) {
                /* Something went wrong before we got to the end of the stream... */
		g_error("%s\n", gnome_vfs_result_to_string (result));
		return 1;
	}

        /* Close the "FD" */
	gnome_vfs_close(handle);

        /* Create a new XmlDocument, using memory (our string) as input */
	doc = xmlReadMemory(document, total, NULL, NULL, 0);
	
        /* Free our string, we dont need twice the same information
         * Notice "document" can be very big, so we have to free it as soon as
         * possible
         */
	g_free(document);

	if(doc == NULL) {
                /* Not good... Could e.g. be a parser error */
		g_error("Could not parse XML");
		return 1;
	}

        /* Dump our document to stdout */
	xmlDocDump (stdout, doc);
        /* And free it. This is very important, because XML DOM trees
         * tend to be big
         */
	xmlFreeDoc (doc);	

        /* Shutdown the GnomeVFS subsystem */
	gnome_vfs_shutdown ();
        
	return 0;
}
