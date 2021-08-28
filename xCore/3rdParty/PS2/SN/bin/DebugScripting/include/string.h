#ifndef _STRINGH
#define _STRINGH

#pragma push_safeptr

#define _need_size_t
#define _need_NULL

#include "sys/stdtypes.h"

#undef _need_NULL
#undef _need_size_t

#ifndef _MEMCPY_
#define _MEMCPY_
void *memcpy(void * dst, const void * src, size_t n);
#endif

void *memmove(void * dst, const void * src , size_t n);
char *strcpy(char * dst, const char * src);
char *strncpy(char * dst, const char * src, size_t n);
char *strcat(char * s1, const char * s2);
char *strncat(char * s1, const char *s2, size_t n);
int memcmp(const void * s1, const void * s2, size_t n);
int strcmp(const char * s1, const char * s2);
int strcoll(const char * s1, const char * s2);
int strncmp(const char * s1, const char *s2, size_t n);
size_t strxfrm(char *dst, const char * src, size_t n);
void *memchr(const void *s, int c, size_t n);
char *strchr(const char *s, int c);
size_t strcspn(const char *s, const char *reject);
char *strpbrk(const char *s, const char *accept);
char *strrchr(const char *s, int c);
size_t strspn(const char *s, const char *accept);
char *strstr(const char *haystack, const char *needle);
char *strtok(char *s, const char *delim);
void *memset(void *s, int c, size_t n);
char *strerror(int n);
size_t strlen(const char *s);


/* non standard stuff */
char * strrev(char * s);

#if defined(_SUNOS) && !defined(_EiC)
void bcopy(char *from, char *to, int length);
#define memmove(x,y,z)  bcopy((y),(x),(z))
#endif


#pragma pop_ptr

#endif /* _STRINGH */







