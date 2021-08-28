/*  This header file is machine generated. 
Modify EiC/config/genstdio.c, or the target independent source 
files it reads, in order to modify this file.  Any 
direct modifications to this file will be lost. 
*/

#define BUFSIZ	512
#define EOF	-1
#define FILENAME_MAX	260
#define FOPEN_MAX	20
#define L_tmpnam	14
#define TMP_MAX	32767
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2
#define _IOFBF	0
#define _IOLBF	64
#define _IONBF	4
typedef double  fpos_t;

typedef struct { char dummy[32]; } FILE;

FILE * _get_stdin();
FILE * stdin = _get_stdin();
FILE * _get_stdout();
FILE * stdout = _get_stdout();
FILE * _get_stderr();
FILE * stderr = _get_stderr();
