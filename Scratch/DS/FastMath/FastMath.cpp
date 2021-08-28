
#include <stdio.h>
#include <conio.h>
#include <math.h>

float FastInvSqrt(float x)
{
    int i = reinterpret_cast<int &>(x);
    float f = float(i);
    f = float(0x5f400000) - f * 0.5f;
    //f = float(0x3f800000) - f * 0.5f;
    i = int(f);
    x = reinterpret_cast<float &>(i);
    return x;
}

float FastInv(float x)
{
    float DistMax = 1.0f;
    float magicValue = float(reinterpret_cast<int &>(DistMax)) + float(0x3f800000);

    union floatint
    {
        float   f;
        int     i;
    };
    floatint magic;
    magic.f = magicValue;

    int i = reinterpret_cast<int &>(x);
    float f = float(i);
    f = magicValue - f;
    i = int(f);
    x = reinterpret_cast<float &>(i);
    return x;
}

float FastLog2 (float x)
{
    // trick to get a really fast log2 approximation for floating-point numbers.

    // float = sign.exp.mantissa (1.8.23 bits)
    //       = sign * 2^(exp+127) * 1.mantissa
    // method 1 using integer instructions:
    //        a) mask out and shift the exponent
    //        b) subtract 127
    // method 2 using floating-point instructions:
    //        a) treat the float as an integer, and convert it to float again
    //           when the float is looked at as an integer the upper bits have the exponent,
    //           which are the important ones. Convert that integer to float, and we have one
    //           big-ass floating-point number
    //        b) divide by 2^23 to rescale the decimal point to its proper range
    //        c) subtract 127.0f
    // With both methods, the lower bits are essentially ignored, so this is only
    // a rough approximation, and you should use some other algorithm to get accurate results.
    //
    // As a side note...converting integer to float is approximately in the form y=A*log2(x)+B,
    // and converting float to integer is the inverse x=pow(2.0, (y-B)/A). With this knowledge,
    // you can convert a "float" to a logarithmic space by doing a float->float conversion,
    // multiply by any power, add an approriate offset, and convert it back to linear space by
    // doing a float->int conversion. With this idea you can approximate any power. Useful for
    // inverse sqrts, sqrts, specular powers, etc...

    int i = reinterpret_cast<int &>(x);
    float f = float(i);
    f *= 0.00000011920928955078125f;
    f -= 127.0f;
    return f;
}

void main( void )
{
    int i;

    union floatint
    {
        float f;
        int   i;
    };

    floatint blarg;
    blarg.i = 0x5c100000;

    printf( "%3.3f", FastLog2( blarg.f ) );

    for ( i = 10; i <= 10000; i+=10 )
    {
        if ( (i%200) == 0 )
        {
            while ( !getch() )
                ;
        }

        float approx, real;

        /*
        approx = FastInvSqrt( (float)i );
        real   = 1.0f / sqrtf((float)i);
        printf( "InvSqrt(%3.3f) == %3.3f, Actual == %3.3f\n", (float)i, approx, real );

        approx = FastInv( (float)i );
        real   = 1.0f / (float)i;
        printf( "Inv(%3.3f)     == %3.3f, Actual == %3.3f\n", (float)i, approx, real );
        */

        approx = FastLog2( (float)i );
        real   = (float)(log( (double)i ) / log( 2.0 ));
        printf( "Log2(%3.3f)    == %3.3f, Actual == %3.3f\n", (float)i, approx, real );
    }

    while ( !getch() )
    {
    }
}