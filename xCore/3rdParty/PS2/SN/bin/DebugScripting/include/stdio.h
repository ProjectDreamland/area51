
#ifndef STDIOH_
#define STDIOH_

#pragma  push_safeptr

#include <stdarg.h>   /* this must get fixed */

#define _need_NULL
#define _need_size_t
 
#include "sys/stdtypes.h"

#undef _need_size_t
#undef _need_NULL

#include <sys/stdio.h>

int fsetpos (FILE *stream, const fpos_t *pos);
int fgetpos (FILE *stream, fpos_t *pos);
int feof (FILE *stream);
int ferror (FILE *stream);
int puts (const char *s);
void rewind (FILE *stream);

#define vprintf(x,y)  vfprintf(stdout,x,y)

#define setbuf(x,y) do { char *b = (y); \
    setvbuf(x, b, b ? _IOFBF : _IONBF, BUFSIZ); } while (0)

#define getchar()     fgetc(stdin)
#define getc(x)       fgetc(x)
#define putchar(x)    fputc(x,stdout)
#define putc(x,y)     fputc(x,y)

/** Prototypes */
int sprintf(char *buf, const char * fmt, ...);
int printf(const char *fmt, ...);
int fprintf(FILE * fp,const char *fmt, ...);

int vfprintf(FILE * fp,const char *fmt, va_list args);
int vsprintf(char * str,const char *fmt, va_list args);

/** PROTOTYPES from fopen.c **/
long ftell(FILE *fp);
int ungetc(int c, FILE *fp);
char * gets(char *s);
int fgetc(FILE * fp);
int fputc(int c, FILE * fp);
int fputs(const char *s, FILE *fp);
char * fgets(char *s, int n, FILE *fp);
size_t fread(void *buf, size_t elsize, size_t nelems, FILE * fp);
size_t fwrite(const void *buf, size_t elsize, size_t nelems, FILE * fp);
FILE *freopen(const char *name, const char *mode, FILE *fp);
FILE *fopen(const char *name, const char *mode);
int fflush(FILE * fp);
int fclose(FILE * fp);
int fscanf(FILE *fp, const char *fmt, ...);
int scanf(const char *fmt, ...);
int sscanf(const char *str,const char *fmt, ...);
int setvbuf(FILE *fp, char *buf, int mode, size_t size);
char * tmpnam(char*s);
FILE * tmpfile(void);
int remove(const char *fname);
int rename(const char *oldname, const char *newname);
int fseek(FILE *fp, long off, int origin);

void clearerr(FILE *fp);
void perror(const char *msg);

/* UNIX system stuff */
FILE *popen( const char *cmd, const char *mode);
int pclose( FILE *fp);

#pragma pop_ptr

#endif


