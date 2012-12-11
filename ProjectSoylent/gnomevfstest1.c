#include <glib.h>
#include <libgnomevfs/gnome-vfs.h>

#define BUFSIZE 8192

int main(int argc, char *argv[]) {
        GnomeVFSHandle *handle;
        GnomeVFSResult result;
        gchar buffer[BUFSIZE];
        GnomeVFSFileSize bytes_read;

        gnome_vfs_init();

        if(argc < 2) {
                g_print("Please run %s (URI)\n", argv[0]);
                return 1;
        }

        result = gnome_vfs_open(&handle, argv[1], GNOME_VFS_OPEN_READ);

        if(result == GNOME_VFS_OK) {
                g_debug("URI opened successfully :-)\n");
                /* Read in some bytes */
                result = gnome_vfs_read(handle, buffer, BUFSIZE, &bytes_read);
                g_print("buffer=%s\n", buffer);
                
        }
        else {
                g_debug("Unable to open URI, aborting\n");
                gnome_vfs_close(handle);
                g_error("Abort\n");
        }

        result = gnome_vfs_close(handle);

        return 0;
}
