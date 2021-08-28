#ifndef MD5_H
#define MD5_H

#include "x_files.hpp"

struct MD5Context {
    u32 buf[4];
    u32 bits[2];
    unsigned char in[64];
};

void MD5Init        ( struct MD5Context* context );
void MD5Update      ( struct MD5Context* context, u8 const* buf, u32 len );
void MD5Final       ( struct MD5Context* context, u8* digest );
void MD5Transform   ( u32 buf[4], u32 const in[16] );

xstring MD5ToString( u8* pMD5 );

/*
* This is needed to make RSAREF happy on some MS-DOS compilers.
*/
typedef struct MD5Context MD5_CTX;

#endif /* !MD5_H */
