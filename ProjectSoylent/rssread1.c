/*
 * Fetch a file using GnomeVFS, figure out whether it's an RSS file and
 * retrieve the RSS version, and parse it.
 * Currently only RSS2.0 parsing implemented
 *
 * Copyright (C) 2005 Nicolas "Ikke" Trangez (eikke eikke commercial)
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
 *      gcc -o rssread1 `pkg-config --cflags --libs gnome-vfs-2.0 libxml-2.0` rssread1.c
 * Then run
 *      ./rssread1 http://blog.eikke.com/xmlsrv/rss2.php?blog=5
 */

#include <glib.h>
#include <libgnomevfs/gnome-vfs.h>

#include <libxml/tree.h>

#define BUFSIZE 4096

enum RssVersion{
        RSS_091,
        RSS_200,
        RSS_UNKNOWN
};

/* Try to figure out the RSS version of the feed we're dealing with */
static enum RssVersion getRssVersion(xmlDoc *doc) {
        xmlNode *node;
        xmlChar *version = NULL;
        enum RssVersion ret = RSS_UNKNOWN;

        /* Get the root node of the document */
        node = xmlDocGetRootElement(doc);
        /* Check whether it's an RSS feed, with "rss" as first node name */
        if(!xmlStrcmp(node->name, (const xmlChar *) "rss")) {
                g_debug("Valid RSS");
                g_debug("Trying to find version...");
                version = xmlGetProp(node, (const xmlChar *)"version");
                if(version == NULL) {
                        g_warning("No version attribute in rss tag");
                        /* RSS_UNKNOWN */
                        return ret;
                }
                /* Now we got a string representation */
                g_debug("RSS Version is '%s', parsing it", version);

                if(!xmlStrcmp(version, (const xmlChar *) "0.91"))
                        ret = RSS_091;
                if(!xmlStrcmp(version, (const xmlChar *) "2.0"))
                        ret = RSS_200;                                
        }
        else {
                g_warning("First element is not \"rss\", aborting");
                /* RSS_UNKNOWN */
                return ret;
        }

        /* The version, or RSS_UNKNOWN if we couldn't find the version */
        return ret;
}

/* Parse one RSS2.0 "item" at "node" */
static void parseRss200Item(xmlDoc *doc, xmlNode *node) {
        xmlNode *cur;
        xmlChar *title = NULL, *content = NULL, *pubDate = NULL;
        
        /* Loop through all subnodes */
        for(cur = node->xmlChildrenNode; cur; cur = cur->next) {
                if(!xmlStrcmp(cur->name, (const xmlChar *) "title")) {
                        title = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
                }
                if(!xmlStrcmp(cur->name, (const xmlChar *) "pubDate")) {
                        pubDate = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
                }
                if(!xmlStrcmp(cur->name, (const xmlChar *) "description")) {
                        content = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
                }
        }

        /* Display the information */
        if(pubDate != NULL) {
                g_print("%s: ", pubDate);
                xmlFree(pubDate);
        }
        if(title != NULL) {
                g_print("%s\n", title);
                xmlFree(title);
        }
        if(content != NULL) {
                g_print("\t%s\n", content);
                xmlFree(content);
        }
        title = pubDate = content = NULL;
        g_print("-------------------------------------------------\n\n");
}

/* Parse an RSS2.0 "items" block, in turn parsing all "item" nodes in it */
static void parseRss200Items(xmlDoc *doc, xmlNode *node) {
        xmlNode *cur;
        
        g_debug("Rss \"items\" node found, parsing it");

        for(cur = node->xmlChildrenNode; cur; cur = cur->next) {
                if(!xmlStrcmp(cur->name, (const xmlChar *) "item")) {
                        parseRss200Item(doc, cur);
                }
        }
}

/* Parse an RSS2.0 document */
static void parseRss200Document(xmlDoc *doc) {
        xmlNode *node;

        node = xmlDocGetRootElement(doc);
        if(node == NULL) {
                g_warning("Cannot get RSS root element");
                return;
        }

        if(xmlStrcmp(node->name, (const xmlChar *) "rss")) {
                g_warning("This is not an RSS document, returning");
                return;
        }

        for(node = node->xmlChildrenNode; node; node = node->next) {
                if(!xmlStrcmp(node->name, (const xmlChar *) "channel")) {
                        parseRss200Items(doc, node);
                }
        }

        return;
}

/* Parse a feed document "doc" with retrieved version "version" */
static void parseRssDocument(xmlDoc *doc, enum RssVersion version) {
        switch(version) {
                case RSS_200:
                        parseRss200Document(doc);
                        break;
                default:
                        /* Others aren't implemented yet */
                        break;
        }

        return;
}

gint main (gint argc, gchar *argv[])
{
        /* GnomeVFS does not use GError's for error handling, but return values */
	GnomeVFSResult result;
        /* A "File Descriptor" is called a "Handle" in GnomeVFS */
	GnomeVFSHandle *handle = NULL;
        /* These are actually just ints */
	GnomeVFSFileSize bytes_read = 0;
	GnomeVFSFileSize space = 0, total = 0;
        /* A pointer to an XmlDocument */
	xmlDocPtr doc = NULL;
        /* This string will contain the raw data we get (e.g. from HTTP) */
	gchar *document = NULL;
        /* We'll use this to check whether we're dealing with XML input */
        gboolean firstblock = TRUE;
        gboolean validxml = FALSE;
        gint cnt = 0;
        const gchar *xmlstart = "<?xml";
        enum RssVersion version = RSS_UNKNOWN;

        LIBXML_TEST_VERSION

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
                if(firstblock == TRUE) {
                        /* If this is the first block we read, wheck whether
                         * we got an XML document. If not, it'd be pointless
                         * to fetch the complete document
                         */
                        for(cnt = 0; cnt < strlen(xmlstart); cnt++) {
                                if(document[cnt] != xmlstart[cnt]) {
                                        validxml = FALSE;
                                        g_warning("Invalid XML");
                                        break;
                                }
                                else {
                                        validxml = TRUE;
                                }
                        }
                        firstblock = FALSE;
                }
		
		total += bytes_read;
		space -= bytes_read;

	} while(result == GNOME_VFS_OK && validxml == TRUE);

	if(result != GNOME_VFS_ERROR_EOF && validxml == TRUE) {
                /* Something went wrong before we got to the end of the stream */
		g_error("%s\n", gnome_vfs_result_to_string (result));
		return 1;
	}

        /* Close the "FD" */
        /* Help, this crashes sometimes*/
        if(handle != NULL) {
        	gnome_vfs_close(handle);
        }

        /* This is no valid XML document. Actually, we don't need this,
         * because xmlReadMemory will return NULL if "document" is invalid
         */
        if(validxml == FALSE) {
                g_free(document);
                gnome_vfs_shutdown();
                g_error("Invalid XML source");
                return 1;
        }

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

        /* Get RSS version */
        version = getRssVersion(doc);

        /* and parse the document */
        parseRssDocument(doc, version);

        /* And free it. This is very important, because XML DOM trees
         * tend to be big
         */
	xmlFreeDoc (doc);
        xmlCleanupParser();

        /* Shutdown the GnomeVFS subsystem */
	gnome_vfs_shutdown ();
        
	return 0;
}
