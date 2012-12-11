/*
 * Simple usage of evolution-data-server's libEbook
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
 *      gcc -o evo-addressbooks-test `pkg-config --cflags --libs libbonobo-2.0 libebook-1.2` evo-addressbooks-test.c
 * Then run
 *      ./evo-addressbook-test
 */

#include <glib.h>
#include <bonobo/bonobo-main.h>
#include <libebook/e-book.h>

/*
 * Check whether a string is not empty
 */
static gboolean checkStringValid(const gchar *s) {
        if(s == NULL)
                return FALSE;
        if(strlen(s) == 0)
                return FALSE;

        return TRUE;
}

/*
 * Dump one EContact
 */
static void dumpContact(EContact *contact, int cnt) {
        const gchar *uid = NULL, *name = NULL, *nick = NULL, *messenger = NULL, *blog = NULL;
        
        /* Get all data
         * You can get a list of all possible constants here
         *      http://www.gnome.org/projects/evolution/developer-doc/libebook/EContact.html#EContactField
         */
        uid = e_contact_get_const(contact, E_CONTACT_UID);
        name = e_contact_get_const(contact, E_CONTACT_FULL_NAME);
        nick = e_contact_get_const(contact, E_CONTACT_NICKNAME);
        messenger = e_contact_get_const(contact, E_CONTACT_IM_MSN_HOME_1);
        blog = e_contact_get_const(contact, E_CONTACT_BLOG_URL);

        /* Check whether this is a real record
         * Should always be OK, but you never know ;-) */
        if(uid == NULL || name == NULL) {
                return;
        }
        
        /* Dump the contact data */
        g_print("%d. %s\n", cnt, name);
        if(checkStringValid(nick) == TRUE)
                g_print("\tNick: %s\n", nick);
        if(checkStringValid(messenger) == TRUE)
                g_print("\tMessenger: %s\n", messenger);
        if(checkStringValid(blog) == TRUE)
                g_print("\tBlog: %s\n", blog);
}

/*
 * Loop though a list of contacts and dump them
 */
static void processContacts(GList *contacts) {
        GList *c;
        int cnt = 1;

        /* Loop through our GList and dump every item in it */
        for(c = contacts; c != NULL; c = c->next) {
                EContact *contact = E_CONTACT(c->data);
                dumpContact(contact, cnt);
                cnt++;
                g_object_unref(contact);
        }

        g_list_free(c);
}

/*
 * Fetch all contacts from an EBook and process them
 */
static void processBook(EBook *book) {
        gboolean b;
        GError *err = NULL;
        EBookQuery *query;
        GList *contacts;

        /* Open the book */
        if(!e_book_open(book, TRUE, &err)) {
                g_warning("Failed to open book: %s\n", err->message);
                g_error_free(err);
                return;
        }

        /* Create a query
         * Here we use something like "ls *" ;-) */
        query = e_book_query_any_field_contains("");

        /* Perform the query, filling "contacts"
         * Returns a gboolean which tells us whether the query 
         * succeeded or not */
        b = e_book_get_contacts(book, query, &contacts, &err);
        /* We don't need the query object anymore */
        e_book_query_unref(query);
        if(b == TRUE) {
                processContacts(contacts);
        }
        else {
                g_warning("Error processing book: %s\n", err->message);
                g_error_free(err);
        }
        g_list_free(contacts);

        g_print("---------------------------------------------------\n\n\n");
        
        return;
}

/*
 * Our main code
 */
int main(int argc, char *argv[]) {
        ESourceList *books = NULL;
        GError *err = NULL;
        GSList *list = NULL, *l = NULL;
        
        /* We need to initialize the Bonobo framework before doing anything
         * Like this Bonobo can activate ("start") EDS if it isn't running yet
         * so we can query it */
        if(bonobo_init(&argc, argv) == FALSE) {
                g_error("Error initializing bonobo");
        }

        /* Query all addressbooks available on the system */
        if(!e_book_get_addressbooks(&books, &err)) {
                g_error("Error fetching addressbooks: %s", err->message);
        }

        /* Get a real list of the books */
        list = e_source_list_peek_groups(books);
        if(list == NULL) {
                g_warning("No addressbooks found :-(");
                return 1;
        }

        /* Loop through the list of books */
        for(l = list; l!= NULL; l = l->next) {
                GSList *groups, *s;

                groups = e_source_group_peek_sources(l->data);

                for(s = groups; s != NULL; s = s->next) {
                        ESource *source = E_SOURCE(s->data);
                        EBook *book;
                        GError *serr;

                        g_print("Source name: %s, URI: %s\n\n", e_source_peek_name(source), e_source_get_uri(source));
                        /* Get an EBook from the source identifier */
                        book = e_book_new(source, &serr);
                        /* And process the book */
                        processBook(book);
                        /* Dispose of the book */
                        g_object_unref(book);
                        g_object_unref(source);
                }
        }
        
        /* Clean up */
        g_object_unref(books);
        g_slist_free(list);

        return 0;
}
