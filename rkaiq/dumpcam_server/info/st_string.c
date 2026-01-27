/*
    a dynamic string implementation using macros
 */

#include "st_string.h"

ST_STRING_UNUSED void aiq_string_printf_va(st_string *s, const char *fmt, va_list ap) {
    int n;
    va_list cp;
    for (;;) {
#ifdef _WIN32
        cp = ap;
#else
        va_copy(cp, ap);
#endif
        n = vsnprintf (&s->d[s->i], s->n-s->i, fmt, cp);
        va_end(cp);

        if ((n > -1) && ((size_t) n < (s->n-s->i))) {
            s->i += n;
            return;
        }

        /* Else try again with more space. */
        if (n > -1) string_reserver(s,n+1); /* exact */
        else string_reserver(s,(s->n)*2);   /* 2x */
    }
}

#ifdef __GNUC__
/* support printf format checking (2=the format string, 3=start of varargs) */
void aiq_string_printf(st_string *s, const char *fmt, ...)
    __attribute__ (( format( printf, 2, 3) ));
#endif

ST_STRING_UNUSED void aiq_string_printf(st_string *s, const char *fmt, ...) {
    va_list ap;
    va_start(ap,fmt);
    aiq_string_printf_va(s,fmt,ap);
    va_end(ap);
}
