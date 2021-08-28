/* CYGNUS LOCAL whole file */

#ifndef _MMUTILS_H_INCLUDED
#define _MMUTILS_H_INCLUDED

#include <stdio.h>
#include "mmintrin.h"

/* Extract the 8 bit integer at the given index from a 128 bit value.  */
static __inline signed char _MM_CHAR (__m128 v, int t)
{
  union {
    __m128 v;
    signed char i[16];
  } u;
  u.v = v;
  /* Bytes are laid out in little endian order.  */
  return u.i[15 - t];
}

#define _MM_CHAR0(x) _MM_CHAR (x, 0)
#define _MM_CHAR1(x) _MM_CHAR (x, 1)
#define _MM_CHAR2(x) _MM_CHAR (x, 2)
#define _MM_CHAR3(x) _MM_CHAR (x, 3)
#define _MM_CHAR4(x) _MM_CHAR (x, 4)
#define _MM_CHAR5(x) _MM_CHAR (x, 5)
#define _MM_CHAR6(x) _MM_CHAR (x, 6)
#define _MM_CHAR7(x) _MM_CHAR (x, 7)
#define _MM_CHAR8(x) _MM_CHAR (x, 8)
#define _MM_CHAR9(x) _MM_CHAR (x, 9)
#define _MM_CHAR10(x) _MM_CHAR (x, 10)
#define _MM_CHAR11(x) _MM_CHAR (x, 11)
#define _MM_CHAR12(x) _MM_CHAR (x, 12)
#define _MM_CHAR13(x) _MM_CHAR (x, 13)
#define _MM_CHAR14(x) _MM_CHAR (x, 14)
#define _MM_CHAR15(x) _MM_CHAR (x, 15)

/* Extract the 16 bit integer at the given index from a 128 bit value.  */
static __inline short _MM_SHORT (__m128 v, int t)
{
  union {
    __m128 v;
    short i[8];
  } u;
  u.v = v;
  /* Bytes are laid out in little endian order.  */
  return u.i[7 - t];
}

#define _MM_SHORT0(x) _MM_SHORT (x, 0)
#define _MM_SHORT1(x) _MM_SHORT (x, 1)
#define _MM_SHORT2(x) _MM_SHORT (x, 2)
#define _MM_SHORT3(x) _MM_SHORT (x, 3)
#define _MM_SHORT4(x) _MM_SHORT (x, 4)
#define _MM_SHORT5(x) _MM_SHORT (x, 5)
#define _MM_SHORT6(x) _MM_SHORT (x, 6)
#define _MM_SHORT7(x) _MM_SHORT (x, 7)

/* Extract the 32 bit integer at the given index from a 128 bit value.  */
static __inline int _MM_INT (__m128 v, int t)
{
  union {
    __m128 v;
    int i[4];
  } u;
  u.v = v;
  /* Bytes are laid out in little endian order.  */
  return u.i[3 - t];
}

#define _MM_INT0(x) _MM_INT (x, 0)
#define _MM_INT1(x) _MM_INT (x, 1)
#define _MM_INT2(x) _MM_INT (x, 2)
#define _MM_INT3(x) _MM_INT (x, 3)

/* Extract the 64 bit integer at the given index from a 128 bit value.  */
static __inline int _MM_LONG (__m128 v, int t)
{
  union {
    __m128 v;
    long i[2];
  } u;
  u.v = v;
  /* Bytes are laid out in little endian order.  */
  return u.i[1 - t];
}

#define _MM_LONG0(x) _MM_LONG (x, 0)
#define _MM_LONG1(x) _MM_LONG (x, 1)

/* Define a function to print out a vector value.
   NAME is the name fo the function to define.
   VECTYPE is the vector type to be printed.
   TYPE is the type of the vector elements
   COUNT is the number of elements in the vector
   PRINTF is the printf format specifier used to print each element.
*/
#define __MM_PRINT_FN(NAME, VECTYPE, TYPE, COUNT, PRINTF)     \
static void NAME (FILE *f, VECTYPE v)			      \
{							      \
  int i;						      \
  union {						      \
    VECTYPE vec;					      \
    TYPE i[COUNT];					      \
  } u;							      \
  u.vec = v;						      \
  fprintf (f, "[");					      \
  /* Bytes are laid out in little endian order.  */           \
  for (i = COUNT - 1; i >= 0; i--)			      \
    fprintf (f, " " PRINTF "%c", u.i[i], i == 0 ? ' ' : ','); \
  fprintf (f, "]\n");					      \
}

__MM_PRINT_FN (_m_fprint_mask,   __m128, unsigned int,    4, "%8x");
__MM_PRINT_FN (_m_fprint_v16qi,  __m128, signed char,    16, "%d");
__MM_PRINT_FN (_m_fprint_v8hi,   __m128, short,           8, "%d");
__MM_PRINT_FN (_m_fprint_v4si,   __m128, int,             4, "%d");
__MM_PRINT_FN (_m_fprint_v2si,     long, int,             2, "%d");
__MM_PRINT_FN (_m_fprint_v2di,   __m128, long,            2, "%ld");
__MM_PRINT_FN (_m_fprint_v16uqi, __m128, unsigned char,  16, "%u");
__MM_PRINT_FN (_m_fprint_v8uhi,  __m128, unsigned short,  8, "%u");
__MM_PRINT_FN (_m_fprint_v4usi,  __m128, unsigned int,    4, "%u");

#define _m_print_mask(V)   _m_fprint_mask   (stdout, (__m128)(V))
#define _m_print_v16qi(V)  _m_fprint_v16qi  (stdout, (__m128)(V))
#define _m_print_v8hi(V)   _m_fprint_v8hi   (stdout, (__m128)(V))
#define _m_print_v4si(V)   _m_fprint_v4si   (stdout, (__m128)(V))
#define _m_print_v2si(V)   _m_fprint_v2si   (stdout,   (long)(V))
#define _m_print_v2di(V)   _m_fprint_v2di   (stdout, (__m128)(V))
#define _m_print_v16uqi(V) _m_fprint_v16uqi (stdout, (__m128)(V))
#define _m_print_v8uhi(V)  _m_fprint_v8uhi  (stdout, (__m128)(V))
#define _m_print_v4usi(V)  _m_fprint_v4usi  (stdout, (__m128)(V))

/* Functions to set 128 bit regs from integer values.  */

static __m128 _m_set_v16qi(signed char a, signed char b, signed char c,
			  signed char d, signed char e, signed char f,
			  signed char g, signed char h, signed char i,
			  signed char j, signed char k, signed char l,
			  signed char m, signed char n, signed char o,
			  signed char p)
{
  union {
    __m128 vec;
    signed char i[16];
  } u;
  /* Bytes are laid out in little endian order.  */
  u.i[0] = p;
  u.i[1] = o;
  u.i[2] = n;
  u.i[3] = m;
  u.i[4] = l;
  u.i[5] = k;
  u.i[6] = j;
  u.i[7] = i;
  u.i[8] = h;
  u.i[9] = g;
  u.i[10] = f;
  u.i[11] = e;
  u.i[12] = d;
  u.i[13] = c;
  u.i[14] = b;
  u.i[15] = a;
  return u.vec;
}

static __inline __m128 _m_set_v8hi(short a, short b, short c, short d,
				  short e, short f, short g, short h)
{
  union {
    __m128 vec;
    short i[8];
  } u;
  /* Bytes are laid out in little endian order.  */
  u.i[0] = h;
  u.i[1] = g;
  u.i[2] = f;
  u.i[3] = e;
  u.i[4] = d;
  u.i[5] = c;
  u.i[6] = b;
  u.i[7] = a;
  return u.vec;
}

static __inline __m128 _m_set_v4si(int a, int b, int c, int d)
{
  union {
    __m128 vec;
    int i[4];
  } u;
  /* Bytes are laid out in little endian order.  */
  u.i[0] = d;
  u.i[1] = c;
  u.i[2] = b;
  u.i[3] = a;
  return u.vec;
}

static __inline long _m_set_v2si(int a, int b)
{
  union {
    long vec;
    int i[2];
  } u;
  /* Bytes are laid out in little endian order.  */
  u.i[0] = b;
  u.i[1] = a;
  return u.vec;
}

static __inline __m128 _m_set_v2di(long a, long b)
{
  union {
    __m128 vec;
    long i[2];
  } u;
  /* Bytes are laid out in little endian order.  */
  u.i[0] = b;
  u.i[1] = a;
  return u.vec;
}

static __inline __m128 _m_set_v16qi1(signed char a)
{
  return _m_set_v16qi (a, a, a, a, a, a, a, a, a, a, a, a, a, a, a, a);
}

static __inline __m128 _m_set_v8hi1(short a)
{
  return _m_set_v8hi (a, a, a, a, a, a, a, a);
}

static __inline __m128 _m_set_v4si1(int a)
{
  return _m_set_v4si (a, a, a, a);
}

static __inline long _m_set_v2si1(int a)
{
  return _m_set_v2si (a, a);
}

static __inline __m128 _m_set_v2di1(long a)
{
  return _m_set_v2di (a, a);
}

#endif
