/*
 * Browse your Evolution Addressbooks using GnomeVFS, e.g. in Nautilus
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

#include <glib.h>

#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-module.h>

#include <bonobo/bonobo-main.h>
#include <libebook/e-book.h>

#define MODULE_METHOD "contact"

typedef struct {
        GNode *gnode;
        gchar *name;
        gboolean directory;

        EContact *contact;
} ContactNode;

typedef struct {
        GnomeVFSFileInfoOptions options;

        GNode *gnode;
        GNode *current_child;
} ContactDirectory;

typedef struct {
        ContactNode *fnode;

        guint bytes_written;
        gchar *content;
        guint size;
} ContactFile;

G_LOCK_DEFINE_STATIC(tree_root);
static GNode *tree_root = NULL;

static gboolean check_string_valid(const gchar *s) {
        if(s == NULL)
                return FALSE;
        if(strlen(s) == 0)
                return FALSE;

        return TRUE;
}

static gchar * econtact_to_string(const EContact *c) {
        const gchar *uid = NULL, *name = NULL, *nick = NULL, *messenger = NULL, *blog = NULL;
        GString *ret = NULL;
        gchar *sret = NULL;
        EContact *contact;

        contact = (EContact *) c;

        ret = g_string_new("Contact: ");
        
        uid = e_contact_get_const(contact, E_CONTACT_UID);
        name = e_contact_get_const(contact, E_CONTACT_FULL_NAME);
        nick = e_contact_get_const(contact, E_CONTACT_NICKNAME);
        messenger = e_contact_get_const(contact, E_CONTACT_IM_JABBER_HOME_1);
        blog = e_contact_get_const(contact, E_CONTACT_BLOG_URL);

        if(uid == NULL || name == NULL) {
                return;
        }
        
        g_string_append_printf(ret, "%s (%s)\n",  name, uid);
        if(check_string_valid(nick) == TRUE)
                g_string_append_printf(ret, "\tNick:\t%s\n", nick);
        if(check_string_valid(messenger) == TRUE)
                g_string_append_printf(ret, "\tJabber:\t%s\n", messenger);
        if(check_string_valid(blog) == TRUE)
                g_string_append_printf(ret, "\tBlog:\t%s\n", blog);

        sret = g_strdup(ret->str);
        g_string_free(ret, TRUE);

        return sret;
}

static gchar * econtact_to_vcard_string(const EContact *c) {
        return e_vcard_to_string(E_VCARD(c), EVC_FORMAT_VCARD_30);
}

static GList * split_path(const gchar *path) {
        gchar *tmp = NULL;
        gchar **strv = NULL, **p = NULL;
        GList *list = NULL;

        if(path[0] == '/') {
                path++;
        }

        tmp = g_strdup(path);

        if(tmp[strlen(tmp) - 1] == '/') {
                tmp[strlen(tmp) - 1] = '\0';
        }

        strv = g_strsplit(tmp, G_DIR_SEPARATOR_S, 0);

        p = strv;
        while(*p != NULL) {
                list = g_list_append(list, g_strdup(*p));
                p++;
        }

        g_strfreev(strv);
        g_free(tmp);

        return list;
}
        
static ContactNode * get_contact_node_from_uri(const GnomeVFSURI *uri) {
        const gchar *tmp = NULL;
        GList *list = NULL, *l = NULL;
        gchar *part = NULL;
        GNode *gnode = NULL, *found_node = NULL;
        ContactNode *ret = NULL;
        gchar *path = NULL;

        tmp = gnome_vfs_uri_get_path(uri);
        /* g_debug("Node for uri \"%s\" requested", tmp); */
        
        if(tmp == NULL || g_ascii_strcasecmp(tmp, "/") == 0) {
                return tree_root->data;
        }

        if(tmp[0] == '/') {
                tmp++;
        }

        list = split_path(tmp);

        G_LOCK(tree_root);

        l = list;
        gnode = tree_root->children;
        while(gnode) {
                part = l->data;
                ret = gnode->data;

                path = ret->name;
                if(path[0] == '/') {
                        path++;
                }

                if(g_ascii_strcasecmp(part, path) == 0) {
                        l = l->next;
                        if(l == NULL) {
                                found_node = gnode;
                                break;
                        }
                        gnode = gnode->children;
                        continue;
                }
                gnode = gnode->next;
        }
        for(l = list; l != NULL; l = l->next) {
                g_free(l->data);
        }
        g_list_free(list);

        ret = found_node? found_node->data : NULL;

        G_UNLOCK(tree_root);
        
        return ret;
}

static ContactNode * contact_node_new(const gchar *name, const EContact *data) {
        ContactNode *ret = NULL;

        /* g_debug("Creating contact node \"%s\"", name); */

        ret = g_new0(ContactNode, 1);
        ret->name = g_strdup(name);

        if(data != NULL) {
                ret->contact = e_contact_duplicate((EContact *)data);
        }

        ret->directory = FALSE;

        return ret;
}

static gboolean free_node_func(GNode *node, gpointer data) {
        ContactNode *contact;

        contact = node->data;

        g_free(contact->name);
        g_object_unref(contact->contact);
        g_free(contact);

        return FALSE;
}

static void process_contacts(GList *contacts, GNode *parent) {
        GList *iter = NULL;

        for(iter = contacts; iter != NULL; iter = iter->next) {
                EContact *contact = NULL;
                ContactNode *curr = NULL;
                gchar *name = NULL;
                
                contact = E_CONTACT(iter->data);
                if(contact == NULL) {
                        continue;
                }

                name = e_contact_get_const(contact, E_CONTACT_FULL_NAME);
                if(name == NULL) {
                        continue;
                }

                curr = contact_node_new(name, contact);
                curr->gnode = g_node_append_data(parent, curr);
                
                //g_free(name);
        }
        g_list_free(iter);
}                

static void process_book(EBook *book, GNode *parent) {
        GError *error = NULL;
        EBookQuery *query = NULL;
        gboolean b;
        GList *contacts = NULL;

        if(e_book_open(book, TRUE, &error) == FALSE) {
                g_warning("Unable to open book: %s", error->message);
                g_error_free(error);
                return;
        }

        query = e_book_query_any_field_contains("");

        if(query == NULL) {
                g_warning("Unable to create EBookQuery");
                return;
        }

        b = e_book_get_contacts(book, query, &contacts, &error);
        e_book_query_unref(query);
        if(b == FALSE) {
                g_warning("Unable to execute query: %s", error->message);
                g_error_free(error);
                return;
        }
        else {
                process_contacts(contacts, parent);
        }

        g_list_free(contacts);
}

static void init_contact_tree() {
        ContactNode *curr = NULL;
        GNode *node = NULL;
        ESourceList *books = NULL;
        GError *error = NULL;
        GSList *list = NULL, *l = NULL;

        g_debug("Initializing contact tree");

        G_LOCK(tree_root);
        
        curr = contact_node_new("<root>", NULL);
        tree_root = g_node_new(curr);
        curr->gnode = tree_root;
        curr->directory = TRUE;

        if(bonobo_init(0, NULL) == FALSE) {
                g_error("Error initializing Bonobo");
        }

        if(!e_book_get_addressbooks(&books, &error)) {
                g_warning("Failed to load address books: %s", error->message);
                g_error_free(error);
        }

        list = e_source_list_peek_groups(books);
        if(list == NULL) {
                g_warning("No addressbooks found");
                return;
        }

        for(l = list; l != NULL; l = l->next) {
                GSList *groups = NULL, *s = NULL;

                groups = e_source_group_peek_sources(l->data);

                for(s = groups; s != NULL; s = s->next) {
                        ESource *source = NULL;
                        EBook *book = NULL;
                        GError *serror = NULL;

                        source = E_SOURCE(s->data);

                        g_debug("Found source \"%s\"", e_source_peek_name(source));
                        curr = contact_node_new(e_source_peek_name(source), NULL);
                        node = g_node_append_data(tree_root, curr);
                        curr->gnode = node;
                        curr->directory = TRUE;

                        book = e_book_new(source, &serror);
                        if(serror == NULL) {
                               process_book(book, node);
                        }
                        else {
                                g_error_free(serror);
                        }

                        g_object_unref(book);
                        g_object_unref(source);
                }
        }

        g_object_unref(books);
        g_slist_free(list);

        G_UNLOCK(tree_root);
}

static void free_contact_tree() {
        g_debug("Freeing contact tree");

        G_LOCK(tree_root);

        g_node_traverse(tree_root, G_PRE_ORDER, G_TRAVERSE_ALL, -1, free_node_func, NULL);
        g_node_destroy(tree_root);

        G_UNLOCK(tree_root);
}

/******************************************************************************/
static GnomeVFSResult do_open_directory(GnomeVFSMethod *method, GnomeVFSMethodHandle **method_handle, GnomeVFSURI *uri, GnomeVFSFileInfoOptions options, GnomeVFSContext *context) {
        ContactDirectory *dir = NULL;
        ContactNode *node = NULL;

        dir = g_new0(ContactDirectory, 1);

        node = get_contact_node_from_uri(uri);
        if(node != NULL) {
                dir->gnode = node->gnode;
                dir->current_child = node->gnode->children;
        }
        else {
                return GNOME_VFS_ERROR_NOT_FOUND;
        }

        *method_handle = (GnomeVFSMethodHandle *) dir;

        return GNOME_VFS_OK;
}

static GnomeVFSResult do_open(GnomeVFSMethod *method, GnomeVFSMethodHandle **method_handle, GnomeVFSURI *uri, GnomeVFSOpenMode mode, GnomeVFSContext *context) {
        ContactNode *node = NULL;
        ContactFile *handle = NULL;

        node = get_contact_node_from_uri(uri);
        if(node != NULL && node->directory == TRUE) {
                return GNOME_VFS_ERROR_IS_DIRECTORY;
        }

        if(mode & GNOME_VFS_OPEN_RANDOM) {
		return GNOME_VFS_ERROR_INVALID_OPEN_MODE;
	}

	if(mode & GNOME_VFS_OPEN_READ) {
		node = get_contact_node_from_uri(uri);
		if(node == NULL) {
			return GNOME_VFS_ERROR_NOT_FOUND;
		}
	}
        else {
		return GNOME_VFS_ERROR_INVALID_OPEN_MODE;
	}

	handle = g_new0(ContactFile, 1);
	handle->fnode = node;
			
	*method_handle = (GnomeVFSMethodHandle *) handle;
		
	return GNOME_VFS_OK;
}

static gboolean do_is_local(GnomeVFSMethod *method, const GnomeVFSURI *uri) {
        return TRUE;
}

static GnomeVFSResult do_read_directory (GnomeVFSMethod *method, GnomeVFSMethodHandle *method_handle, GnomeVFSFileInfo *file_info, GnomeVFSContext *context)
{
	ContactDirectory *handle = (ContactDirectory *) method_handle;
	ContactNode  *contact;

	if (handle->current_child == NULL) {
		return GNOME_VFS_ERROR_EOF;
	}

	contact = handle->current_child->data;

	if (contact->directory == TRUE) {
		file_info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
		file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_TYPE;
		file_info->mime_type = g_strdup ("x-directory/normal");
		file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
	} else {
		file_info->type = GNOME_VFS_FILE_TYPE_REGULAR;
		file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_TYPE;
		file_info->mime_type = g_strdup ("text/plain");
		file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
                file_info->size = strlen(econtact_to_string(contact->contact));
		file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_SIZE;
	}		

	file_info->name = g_strdup(contact->name);

	handle->current_child = handle->current_child->next;

	return GNOME_VFS_OK;
}

static GnomeVFSResult do_close_directory (GnomeVFSMethod *method, GnomeVFSMethodHandle *method_handle, GnomeVFSContext *context)
{
	ContactDirectory *handle = NULL;
        
        handle = (ContactDirectory *) method_handle;

	g_free(handle);

	return GNOME_VFS_OK;
}

static GnomeVFSResult do_close(GnomeVFSMethod *method, GnomeVFSMethodHandle *method_handle, GnomeVFSContext *context) {
	ContactFile *handle = NULL;
        
        handle = (ContactFile *) method_handle;

        g_free(handle->content);
	g_free (handle);

	return GNOME_VFS_OK;
}

static GnomeVFSResult do_read(GnomeVFSMethod *method, GnomeVFSMethodHandle *method_handle, gpointer buffer, GnomeVFSFileSize bytes, GnomeVFSFileSize *bytes_read, GnomeVFSContext *context) {
	ContactFile *handle = NULL;
        
        handle = (ContactFile *) method_handle;

	if(handle->content == NULL) {
		/* This is the first pass, get the content string. */
		handle->content = econtact_to_string(handle->fnode->contact);
		handle->size = strlen(handle->content);
		handle->bytes_written = 0;
	}

	if (handle->bytes_written >= handle->size) {
		/* The whole file is read, return EOF. */
		*bytes_read = 0;
		return GNOME_VFS_ERROR_EOF;
	}

	*bytes_read = MIN(bytes, handle->size - handle->bytes_written);

	memcpy(buffer, handle->content + handle->bytes_written, *bytes_read);
	
	handle->bytes_written += *bytes_read;

	return GNOME_VFS_OK;
}

static GnomeVFSResult do_get_file_info (GnomeVFSMethod *method, GnomeVFSURI *uri, GnomeVFSFileInfo *file_info, GnomeVFSFileInfoOptions options, GnomeVFSContext *context) {
	ContactNode *contact = NULL;

	contact = get_contact_node_from_uri(uri);
	if(contact == NULL) {
		return GNOME_VFS_ERROR_NOT_FOUND;
	}

	file_info->name = g_strdup(contact->name);
	
	if (contact->directory == TRUE) {
		file_info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
		file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_TYPE;
		file_info->mime_type = g_strdup ("x-directory/normal");
		file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
	} else {
		file_info->type = GNOME_VFS_FILE_TYPE_REGULAR;
		file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_TYPE;
		file_info->mime_type = g_strdup ("text/plain");
		file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		file_info->size = strlen(econtact_to_string(contact->contact));
		file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_SIZE;
	}
	
	return GNOME_VFS_OK;
}

/******************************************************************************/

/* Our module vtable */
static GnomeVFSMethod vtable = {
        sizeof(GnomeVFSMethod),

        do_open,           /* open */
        NULL,           /* create */
        do_close,           /* close */
        do_read,           /* read */
        NULL,           /* write */
        NULL,           /* seek */
        NULL,           /* tell */
        NULL,           /* truncate handle */
        do_open_directory,           /* open directory */
        do_close_directory,           /* close directory */
        do_read_directory,           /* read directory */
        do_get_file_info,           /* get file info */
        NULL,           /* get file info from handle */
        do_is_local,           /* is local */
        NULL,           /* create directory */
        NULL,           /* remove directory */
        NULL,           /* move */
        NULL,           /* unlink */
        NULL,           /* check same fs */
        NULL,           /* set file info */
        NULL,           /* truncate */
        NULL,           /* find directory */
        NULL,           /* create symbolic link */
        NULL,           /* monitor add */
        NULL,           /* monitor cancel */
        NULL,           /* file control */
};

/* Initialize the module */
GnomeVFSMethod * vfs_module_init(const gchar *methodname, const gchar *args) {
        g_debug("Module \"%s\" loaded, args is \"%s\"", methodname, args);

        if(g_ascii_strcasecmp(methodname, "" MODULE_METHOD) == 0) {
                init_contact_tree();
                return &vtable;
        }

        return NULL;
}

/* Shutdown the module */
void vfs_module_shutdown(GnomeVFSMethod *method) {
        free_contact_tree();
}
