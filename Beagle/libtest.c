/* (c) Blahblahblah Ikke - http://www.eikke.com
 *
 * Compile using
 *      gcc -g -shared -lmagic -o libtest.so libtest.c
 */

#include <stdlib.h>
#include <magic.h>

char *
gettype(const char *file)
{
    struct magic_set *ms;
    const char *m;
    char *ret;

    ms = magic_open(MAGIC_NONE);
    if (ms == NULL) {
	ret = NULL;
    }
    if (magic_load(ms, NULL) == -1) {
	ret = NULL;
    }

    if ((m = magic_file(ms, file)) == NULL)
            ret = NULL;
    else
    {
            ret = (char *)m;
                printf("%s\n", ret);
    }

    magic_close(ms);

    return ret;
}
