#ifndef _CATIN_LIBC_H
#define _CATIN_LIBC_H
#include "catin_types.h"

usize catin_strlen(const char *s);
char *catin_strcpy(char *d, const char *s);
char *catin_strncpy(char *d, const char *s, usize n);
int   catin_strcmp(const char *a, const char *b);
int   catin_strncmp(const char *a, const char *b, usize n);
char *catin_strchr(const char *s, int c);
char *catin_strrchr(const char *s, int c);
char *catin_strstr(const char *h, const char *n);
void *catin_memset(void *d, int c, usize n);
void *catin_memcpy(void *d, const void *s, usize n);
int   catin_memcmp(const void *a, const void *b, usize n);
char *catin_strcat(char *d, const char *s);
int   catin_atoi(const char *s);
long  catin_atol(const char *s);
void  catin_itoa(int v, char *out);
int   catin_printf(const char *fmt, ...);
int   catin_snprintf(char *buf, usize cap, const char *fmt, ...);
void *catin_malloc(usize n);
void *catin_calloc(usize n, usize sz);
void *catin_realloc(void *p, usize n);
void  catin_free(void *p);

#define strlen  catin_strlen
#define strcpy  catin_strcpy
#define strncpy catin_strncpy
#define strcmp  catin_strcmp
#define strncmp catin_strncmp
#define strchr  catin_strchr
#define strrchr catin_strrchr
#define strstr  catin_strstr
#define memset  catin_memset
#define memcpy  catin_memcpy
#define memcmp  catin_memcmp
#define strcat  catin_strcat
#define atoi    catin_atoi
#define atol    catin_atol
#define printf  catin_printf
#define snprintf catin_snprintf
#define malloc  catin_malloc
#define calloc  catin_calloc
#define realloc catin_realloc
#define free    catin_free
#endif
