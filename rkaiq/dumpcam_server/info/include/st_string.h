/*
    a dynamic string implementation using macros
 */

#ifndef ST_STRING_H
#define ST_STRING_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef __GNUC__
#define ST_STRING_UNUSED __attribute__((__unused__))
#else
#define ST_STRING_UNUSED
#endif

#ifdef oom
#error "The name of macro 'oom' has been changed to 'st_string_oom'. Please update your code."
#define st_string_oom() oom()
#endif

#ifndef st_string_oom
#define st_string_oom() exit(-1)
#endif

typedef struct st_string_s {
    char *d;  /* pointer to allocated buffer */
    size_t n; /* allocated capacity */
    size_t i; /* index of first unused byte */
} st_string;

#define string_reserver(s,amt)                            \
do {                                                       \
    if (((s)->n - (s)->i) < (size_t)(amt)) {                 \
        char *string_tmp = (char*)realloc(                   \
        (s)->d, (s)->n + (amt));                             \
        if (!string_tmp) {                                   \
            st_string_oom();                                      \
        }                                                      \
        (s)->d = string_tmp;                                 \
        (s)->n += (amt);                                       \
    }                                                        \
} while(0)

#define string_init(s)                                   \
do {                                                       \
    (s)->n = 0; (s)->i = 0; (s)->d = NULL;                   \
    string_reserver(s,100);                                 \
    (s)->d[0] = '\0';                                        \
} while(0)

#define string_done(s)                                   \
do {                                                       \
    if ((s)->d != NULL) free((s)->d);                        \
    (s)->n = 0;                                              \
} while(0)

#define string_free(s)                                   \
do {                                                       \
    string_done(s);                                        \
    free(s);                                                 \
} while(0)

#define string_new(s)                                    \
do {                                                       \
    (s) = (st_string*)malloc(sizeof(st_string));             \
    if (!(s)) {                                              \
        st_string_oom();                                        \
    }                                                        \
    string_init(s);                                        \
} while(0)

#define string_renew(s)                                  \
do {                                                       \
    if (s) {                                                \
        string_clear(s);                                    \
    } else {                                                \
        string_new(s);                                      \
    }                                                       \
} while(0)

#define string_clear(s)                                  \
do {                                                       \
    (s)->i = 0;                                              \
    (s)->d[0] = '\0';                                        \
} while(0)

#define string_bincpy(s,b,l)                             \
do {                                                       \
    string_reserver((s),(l)+1);                             \
    if (l) memcpy(&(s)->d[(s)->i], b, l);                    \
    (s)->i += (l);                                           \
    (s)->d[(s)->i]='\0';                                     \
} while(0)

#define string_concat(dst,src)                                 \
do {                                                             \
    string_reserver((dst),((src)->i)+1);                          \
    if ((src)->i) memcpy(&(dst)->d[(dst)->i], (src)->d, (src)->i); \
    (dst)->i += (src)->i;                                          \
    (dst)->d[(dst)->i]='\0';                                       \
} while(0)

#define string_len(s) ((s)->i)

#define string_body(s) ((s)->d)

ST_STRING_UNUSED void aiq_string_printf_va(st_string *s, const char *fmt, va_list ap);

ST_STRING_UNUSED void aiq_string_printf(st_string *s, const char *fmt, ...);

#endif /* ST_STRING_H */
