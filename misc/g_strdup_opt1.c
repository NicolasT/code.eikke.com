#include <glib.h>
#include <string.h>

#ifdef USE_G_STRDUP 
        #warning "Using g_strdup"
        /* Just a dumb g_strdup alias */
        #define my_strdup(str) g_strdup(str)
#else
        #warning "Using own implementation"
        #include <stdlib.h>
        #include <string.h>
        /* This is more advanced. Comments inline */
        #define my_strdup(str) \
                /* Using a GCC extension */                  \
                (G_GNUC_EXTENSION                            \
                  /* If "str" is a constant string at compile time, then */ \
                  (__builtin_constant_p(str)                      \
                   /* If the string is empty */                 \
                   ? (G_UNLIKELY(((const gchar *)(str))[0] == '\0')          \
                      /* Return a pointer to one NULL byte. Using calloc \
                       * will auto-nullify the result, but (maybe) in a  \
                       * more efficient way than using malloc/memset     \
                       * (mmap stuff)                                    \
                       */                                                \
                      ? (gchar *)calloc(1, 1)               \
                      /* else (str[0] != '\0') */           \
                      : ({                                  \
                            /* Calculate the memory we need */ \
                            gsize _mlen = sizeof(str);  \
                            /* Copy the string content verbatim (much more   \
                             * efficient than using str(n)cpy) and return it \
                             * Using g_malloc is fine as it'll abort() when  \
                             * allocation fails, so we can use it inside the \
                             * memcpy call                                   \
                             */                                              \
                            (gchar *)memcpy((gchar *)g_malloc(_mlen), str, _mlen); \
                         }))  \
                   /* else (string not constant at compile time) \
                    * use the normal g_strdup                     \
                    */                                           \
                   : g_strdup(str)))
#endif

int main(int argc, char *argv[]) {
        guint32 i;

        /* We try to be as correct as possible:
         *     take relocation time into account
         */
        gchar *foo = g_strdup("test");
        g_free(foo);

        /* Run some tests */
        foo = my_strdup("foo");
        g_print("foo = %s\n", foo);
        g_free(foo);
        foo = my_strdup(argv[0]);
        g_print("%s = %s\n", argv[0], foo);
        g_free(foo);
        foo = my_strdup("");
        g_print("0 = %d\n", strlen(foo));
        g_free(foo);

        for(i = 0; i < 10000000; ++i) {
                /* Duplicate a string, constant at compile time
                 * As you could see, this should give a performance "boost"
                 * when using our own little dup function
                 */
                gchar *test1 = my_strdup("foo");
                /* Duplicate a string that's not constant at compile time.
                 * Will always fallback to a g_strdup call
                 */
                gchar *test2 = my_strdup(argv[0]);
                /* One more test handled by "our" special version:
                 * duplicate an empty string (str[0] == '\0')
                 */
                gchar *test3 = my_strdup("");
                g_free(test1);
                g_free(test2);
                g_free(test3);
        }

        return 0;
}
