//==============================================================================
//
//  x_math_basic.cpp
//
//==============================================================================

//==============================================================================
//  IMPLEMENTATION NOTES
//==============================================================================
//  
//  IEEE 754 32-bit floating point representation:
//  
//      S.EEEEEEEE.MMMMMMMMMMMMMMMMMMMMMMM
//      0.1      8.9                    31
//  
//      1.11111111.00000000000000000000000  =  +INF  =  0xFF800000
//      0.11111111.00000000000000000000000  =  -INF  =  0x7F800000
//      ?.11111111.<any non-zero mantissa>  =   NAN  =  0x7FC00000, for example
//      ?.00000000.???????????????????????  =  denormalized value (very small)
//  
//  For normalized values:
//      Value =  (-1)^S  *  2^(E-127)  *  1.MMMMMMMMMMMMMMMMMMMMMMM
//
//  For denormalized values:
//      Value =  (-1)^S  *  2^(-126)   *  0.MMMMMMMMMMMMMMMMMMMMMMM
//
//  Note the implicit "1." and "0." prefixed on the mantissa for normalized and 
//  denormalized values respectively.
//
//  Examples:
//
//      0.00000000.00000000000000000000000 =  0 (positive zero)
//      1.00000000.00000000000000000000000 = -0 (negative zero)
//
//      0.11111111.00000000000000000000000 = +INF
//      1.11111111.00000000000000000000000 = -INF
//
//      0.11111111.10000000000000000000000 = NaN
//      0.11111111.00000100000000000000000 = NaN
//      1.11111111.00100010001001010101010 = NaN
//      1.11111111.11111111111111111111111 = NaN
//
//      0.10000000.00000000000000000000000 = +1 * 2^(128-127) * 1.0   =  2.0
//      0.10000001.10100000000000000000000 = +1 * 2^(129-127) * 1.101 =  6.5
//      1.10000001.10100000000000000000000 = -1 * 2^(129-127) * 1.101 = -6.5
//    
//      0.00000001.00000000000000000000000 = +1 * 2^(  1-127) * 1.0 = 2^(-126)
//      0.00000000.10000000000000000000000 = +1 * 2^( -126  ) * 0.1 = 2^(-127) 
//      0.00000000.00000000000000000000001 = +1 * 2^( -126  ) * 0.0000...00001
//                                                                  = 2^(-149)
//  
//==============================================================================
//
//  Various float operations are simplified when the value in question is in
//  the range [0.5, 1.0), or [ (2^-1), (2^0) ).  Unfortunately, the incoming 
//  value will rarely fall conveniently into that range.  But, we can FORCE any
//  original float value into the desired range by "shifting" it up or down.
//  Obviously, this modifies the value.  To compensate, we will use an "external
//  exponent" (ExtExp) to keep track of what we "took out" of the f32's own 
//  orignal exponent value.
//
//  For a normalized 32-bit float value, the "actual exponent" is equal to the
//  stored exponent MINUS 127.  To get the range we want, we need an actual 
//  exponent of -1, and thus, a stored exponent of 126.
//  
//  Let's consider an example.  How about 2.5:
//
//      0.10000000.01000000000000000000000 = +1 * 2^(128-127) * 1.01 = 2.5
//
//  The actual exponent is 1.  However, we want it to be -1.  So, we just go 
//  ahead force a -1 (which is 126 stored) into the exponent field.  Since we 
//  changed the exponent by 2 (from the orginally stored 128 to 126), we save 
//  that for later.  Now we have:
//
//      0.01111110.01000000000000000000000 = +1 * 2^(126-127) * 1.01 = 0.625
//      [ExtExp=2]
//      
//  Now our value falls in [0.5, 1.0) like we wanted.  We just have to remember
//  to compensate with the ExtExp before we are through.
//
//  Here is a formula that puts everything in place:
//
//      OriginalValue = ModifiedValue * 2^ExtExp
//
//  And, checking our example...
//
//      OriginalValue = ModifiedValue * 2^ExtExp
//                2.5 =         0.625 * 2^2
//                2.5 =         0.625 * 4
//
//  Note that normalized cases are pretty easy.  We just pound 126 into the 
//  exponent, while remembering how much we changed from the original exponent.
//  Denormalized values are a little tougher.  Consider another example:
//  
//      0.00000000.01100000000000000000000 = +1 * 2^(-126) * 0.011 
//                                         = 2^(-128) + 2^(-129)
//  
//  Since this value is denormalized, there is no implicit "1." in front of the
//  mantissa.  But, to get the desired range, we need to be normalized.  So, now
//  we will have to tinker with the mantissa.
//  
//  If we ignore the sign bit (that is, if we save it off and then clear it), 
//  then we know that there are only 0's to the left of the mantissa.  So, let's
//  just left shift until we get a bit to qualify for our implied "1.".  And, of
//  course, we must record compensation for these shifts, too!
//  
//      0.00000000.01100000000000000000000  [ExtExp= 0] (E=0)
//      0.00000000.11000000000000000000000  [ExtExp=-1] (E=0)
//      0.00000001.10000000000000000000000  [ExtExp=-2] (E=1)
//  
//  Once a bit reaches the implied "1." position, it is no longer needed in the 
//  mantissa.  BUT!  We now have a normalized value.  And normalized values
//  can't have a stored exponent of 0.  So, the exponent must become 1.  (Rather
//  convenient, actually, since we just shifted a bit in there anyway!)
//  
//  Checking our formula on each of the iterations shows that they are all 
//  equal.  (To get everything to fit, I've dropped off 8 bits.)
//
//      0.00000000.011000000000000  [ExtExp= 0] = 2^( -126) * 0.011  *  2^( 0)
//      0.00000000.110000000000000  [ExtExp=-1] = 2^( -126) * 0.110  *  2^(-1)
//      0.00000001.100000000000000  [ExtExp=-2] = 2^(1-127) * 1.100  *  2^(-2)
//  
//  Now we can just proceed as if we have a simple normalized value.  Force 126
//  into the exponent, but record the change in exponent value.  Note that we
//  started with a ExtExp value of -2.  So when the exponent changes from 1 to
//  126, we have to add -125 into the ExtExp for a final value of -127.  So:
//  
//      0.01111110.10000000000000000000000 = +1 * 2^(126-127) * 1.1 
//                                         = 2^(-1) * 1.1
//                                         = 2^(-1) + 2^(-2)
//  
//  And then if we factor in ExtExp, we see that our formula still holds:
//  
//      OriginalValue        =  ModifiedValue               * 2^ExtExp
//      2^(-128) + 2^(-129)  =  ( 2^(-1)     + 2^(-2)     ) * 2^(-127) 
//      2^(-128) + 2^(-129)  =    2^(-1-127) + 2^(-2-127)
//      2^(-128) + 2^(-129)  =    2^(-128)   + 2^(-129)
//      
//  Outrageous!
//  
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_MATH_HPP
#include "..\x_math.hpp"
#endif

#ifndef X_DEBUG_HPP
#include "..\x_debug.hpp"
#endif

//==============================================================================
//  DEFINITIONS
//==============================================================================

//==============================================================================
//  TYPES
//==============================================================================

union f32_forge
{
    u32     Bits;
    f32     F32;
    f32_forge( void )            { }
    f32_forge( u32 b ) : Bits(b) { }
    f32_forge( f32 f ) : F32 (f) { }
};

//==============================================================================
//  LOCAL VARIABLES
//==============================================================================

//==============================================================================
//  LOCAL FUNCTIONS
//==============================================================================

static
void f32_UnloadExponent( f32& f, s32& ExtExp )
{   
    // We've done nothing sneaky yet, so there is no external exponent.
    ExtExp = 0;

    // Watch out for 0!
    if( f == 0.0f )
    {
        f = 0.0f;   // Why?  Just in case we actually had -0.
        return;
    }

    //
    // We want to work with a f32 value in [0.5, 1.0).  So we will "shift"
    // our f32 around until it is so.  This may take "points" out of the f32's
    // exponent.  We'll store them in the external exponent, ExtExp, for safe
    // keeping.
    //
    // We have two courses of action depending on the state of the f32.  The 
    // tough case is a "denormalized" value.
    //

    f32_forge Forge( f );

    // If the exponent has some bits set...
    if( (Forge.Bits & 0x7F800000) != 0x00000000 )
    {
        // The exponent has some bits...
        //
        // Normalized value.  This is easy.  To get the desired value range, 
        // the actual exponent must be -1.  Note that:
        //
        //      2^(-1) = 0.5
        // 
        // To get an actual exponent of -1, we need to stick 126 into the stored
        // exponent field.  Remember, for normalized values, the actual exponent 
        // equals the stored exponent minus 127.  Or:
        //
        //      Actual_Exponent = (Stored_Exponent - 127)
        //
        // So, we save the difference between the stored exponent and 126 in our
        // ExtExp field for future reference.  And we pound 126 into the stored
        // exponent.

        // Store change in exponent in ExtExp.
        ExtExp = (s32)((Forge.Bits & 0x7F800000) >> 23) - 126;

        // Pound 126 into the stored exponent.
        Forge.Bits &= ~0x7F800000;      // Clear exponent.
        Forge.Bits |= (126 << 23);      // Set in 126.
    }
    else
    {
        u32 SignBit;

        // The exponent has no bits, and the mantissa has some bits...
        //
        // Denormalized value.  Ugh!  The stored exponent is 0, and the actual
        // exponent is -126.  But more importantly, there is no implied "1." in
        // front of the mantissa.  So, we need to shift the mantissa to the left
        // until we shift out one bit.
        //
        // After we are done shifting, we will have un-denormalized our value.
        // At that point, the stored exponent can no longer remain 0.  It 
        // actually becomes 1.  We would normally start our ExtExp at -126 and
        // go from there.  But since we are going the end up with an extra point
        // later, we will just add it in now and start with -125.
        //
        // Before we get going, though, save off the sign bit.  No need to clear
        // it, though, since it gets shifted out anyway.

        SignBit = Forge.Bits & 0x80000000;      // Save sign bit.
        ExtExp  = -125;

        // Shift 8 at a time.
        while( !(Forge.Bits & 0x00FF8000) )
        {
            Forge.Bits <<= 8;
            ExtExp      -= 8;
        }

        // Shift 1 at a time.
        while( !(Forge.Bits & 0x00800000) )
        {
            Forge.Bits <<= 1;
            ExtExp      -= 1;
        }

        // We can now build our new "normalized" f32.  Nuke out that bit that
        // shifted from the mantissa to the exponent.  Add in the saved sign 
        // bit.  And pound in an exponent of 126.

        Forge.Bits &= 0x007FFFFF;
        Forge.Bits |= SignBit;
        Forge.Bits |= (126 << 23);
    }

    //
    // Whew!  That wasn't too bad, was it?
    //

    f = Forge.F32;
}

//==============================================================================

static
void f32_LoadExponent( f32& f, s32 ExtExp )
{
    s32 StoredExp;

    // Watch out for 0!
    if( f == 0.0f )
    {
        f = 0.0f;   // Why?  Just in case we actually had -0.
        return;
    }

    f32_forge Forge( f );

    // We need the STORED exponent that is in f.
    StoredExp = (s32)((Forge.Bits & 0x7F800000) >> 23);

    // Add the external exponent into the stored exponent.
    StoredExp += ExtExp;

    // Watch out for trouble!
    ASSERT( StoredExp < 256 );     // Overflow!
    ASSERT( StoredExp > -23 );     // Underflow!

    // If the stored exponent is positive, then we are in good shape.
    if( StoredExp > 0 )
    {
        // This is easy.  Pound the updated stored exponent into the f32.
        Forge.Bits &= ~0x7F800000;          // Clear old exponent.
        Forge.Bits |= (StoredExp << 23);    // Install new exponent.
    }
    else
    {
        u32 SignBit;

        // If we are here, then the stored exponent is in [0,-23].  This is going to
        // result in a denormalized value.  Oh well.  Such is life.

        SignBit = Forge.Bits & 0x80000000;      // Save sign bit.

        Forge.Bits &= 0x007FFFFF;       // Isolate the mantissa bits.
        Forge.Bits |= 0x00800000;       // Stick in the implied "1." bit.

        // We need to get the stored exponent up to +1.

        // Shift 8 at a time.
        while( StoredExp < -6 )
        {
            Forge.Bits >>= 8;
            StoredExp   += 8;
        }

        // Shift 1 at a time.
        while( StoredExp < 1 )
        {
            Forge.Bits >>= 1;
            StoredExp   += 1;
        }

        // We can now build our new denormalized f32.  Put the sign bit back in.
        // Since the stored exponent is zero for denormalized values, we don't 
        // have to do anything for the exponent.

        Forge.Bits |= SignBit;
    }

    // Make the resulting value available to the caller.
    f = Forge.F32;
}

//==============================================================================
//  PUBLIC FUNCTIONS
//==============================================================================

//==============================================================================
//  BASIC MATH
//==============================================================================

//------------------------------------------------------------------------------
//  There are a number of functions implemented in x_math_inline.hpp
//------------------------------------------------------------------------------

//==============================================================================
//  Implementation notes for x_floor and x_ceil:
//==============================================================================
//  
//  Consider the number 5.625:
//  
//      0.10000001.01101000000000000000000
//  
//  The stored exponent is 129, so the actual exponent is 2.  Given this, we can
//  note the contributive value of each bit in the mantissa.
//  
//      0.10000001.01101000000000000000000
//                 |||||
//                 ||||'----------------------- 2^(-3) = 0.125
//                 |||'------------------------ 2^(-2) = 0.250
//                 ||'------------------------- 2^(-1) = 0.500
//                 ||
//                 |'-------------------------- 2^( 0) = 1.000
//                 '--------------------------- 2^(+1) = 2.000
//      The implied "1." in mantissa <--------- 2^(+2) = 4.000
//  
//  Given our intimate knowledge of the bits in the 32-bit float, it is easy to
//  see that we can "floor" THIS number simply by clearing all of the bits that
//  represent fractional contributions.  Thus:
//  
//      0.10000001.01101000000000000000000 = 5.625
//      0.10000001.01*********************
//      0.10000001.01000000000000000000000 = 5.000
//  
//  This trick doesn't really do a floor operation.  A proper floor "truncates
//  DOWN to the next integer".  Clearing the fractional bits has the effect of
//  "truncating TOWARDS ZERO to the next integer".  So, we have to treat 
//  negative numbers a little special.
//
//  We also have to watch out for 0, particularly -0.
//
//  And so on.  To see why the rest of the code is there, just walk a few 
//  examples through.
//
//==============================================================================

f32 x_floor( f32 a )
{
    f32_forge f;
    s32       e;
    u32       s;

    // Watch out for trouble!
    ASSERT( x_isvalid(a) );

    // Watch out for 0!
    if( a == 0.0f )  return( 0.0f );

    // We need the exponent...
    f.F32 = a;
    e     = (s32)((f.Bits & 0x7F800000) >> 23) - 127;

    // Extremely large numbers have no precision bits available for fractional
    // data, so there is no work to be done.
    if( e > 23 )  return( a );

    // We need the sign bit.
    s = f.Bits & 0x80000000;

    // Numbers between 1 and -1 must be handled special.
    if( e < 0 )  return( s ? -1.0f : 0.0f );

    // Mask out all mantissa bits used in storing fractional data.
    f.Bits &= (-1 << (23-e));

    // If value is negative, we have to be careful.
    // And otherwise, we're done!
    if( s && (a < f.F32) )    return( f.F32 - 1.0f );
    else                      return( f.F32 );
}

//==============================================================================

f32 x_ceil( f32 a )
{
    f32_forge f;
    s32       e;
    u32       s;

    // Watch out for trouble!
    ASSERT( x_isvalid(a) );

    // Watch out for 0!
    if( a == 0.0f )  return( 0.0f );

    // We need the exponent...
    f.F32 = a;
    e     = (s32)((f.Bits & 0x7F800000) >> 23) - 127;

    // Extremely large numbers have no precision bits available for fractional
    // data, so there is no work to be done.
    if( e > 23 )  return( a );

    // We need the sign bit.
    s = f.Bits & 0x80000000;

    // Numbers between 1 and -1 must be handled special.
    if( e < 0 )  return( s ? 0.0f : 1.0f );

    // Mask out all mantissa bits used in storing fractional data.
    f.Bits &= (-1 << (23-e));

    // If value is positive, we have to be careful.
    // And otherwise, we're done!
    if( !s && (a > f.F32) )    return( f.F32 + 1.0f );
    else                       return( f.F32 );
}

//==============================================================================

f32 x_modf( f32 a, f32* pWhole )
{
    f32_forge f;
    s32       e;

    // Watch out for trouble!
    ASSERT( x_isvalid(a) );
    ASSERT( pWhole );

    // Watch out for 0!
    if( a == 0.0f )
    {
        *pWhole = 0.0f;
        return( 0.0f );
    }

    // We need the exponent...
    f.F32 = a;
    e     = (s32)((f.Bits & 0x7F800000) >> 23) - 127;

    // Extremely large numbers have no precision bits available for fractional
    // data, so there is no work to be done.
    if( e > 23 )
    {
        *pWhole = a;
        return( 0.0f );
    }

    // Numbers between 1 and -1 must be handled special.
    if( e < 0 )  
    {
        *pWhole = 0.0f;
        return( a );
    }

    // Mask out all mantissa bits used in storing fractional data.
    f.Bits &= (-1 << (23-e));
    
    // The rest is easy.
    *pWhole = f.F32;
    return( a - f.F32 );
}

//==============================================================================

f32 x_frexp( f32 a, s32* pExp )
{
    // Watch out for trouble!
    ASSERT( x_isvalid(a) );
    ASSERT( pExp );

    // All of the work is done by f32_UnloadExponent.
    f32_UnloadExponent( a, *pExp );
    return( a );
}

//==============================================================================

f32 x_ldexp( f32 a, s32 Exp )
{
    // Watch out for trouble!
    ASSERT( x_isvalid(a) );

    // All of the work is done by f32_LoadExponent.
    f32_LoadExponent( a, Exp );
    return( a );
}

//==============================================================================

f32 x_fmod( f32 a, f32 b )
{
    return fmodf( a, b );

    // as per D. Michael and Darrin this function has a bug where 
    // if b is between 0.0f and 0.5f, it will hang in an infinite loop
    /*
    xbool Negative;
    f32   MultipleOfB;
    s32   Magnitude;
    s32   TempS32;
    f32   TempF32;

    // Watch out for trouble!
    ASSERT( x_isvalid(a) );
    ASSERT( x_isvalid(b) );
    ASSERT( b != 0.0f );

    // Watch out for 0!
    if( a == 0.0f )    return( 0.0f );

    // Let's all be positive about this.
    if( b < 0 )    { b = -b;                   }
    if( a < 0 )    { a = -a; Negative = TRUE;  }    
    else           {         Negative = FALSE; }

    // As long as a is not less than b, we don't have our answer...
    while( a >= b )
    {           
        TempF32     = a;
        MultipleOfB = b;
        f32_UnloadExponent( TempF32,     Magnitude );
        f32_UnloadExponent( MultipleOfB, TempS32   );
        f32_LoadExponent  ( MultipleOfB, Magnitude );

        while( Magnitude >= 0 )
        {
            if( a >= MultipleOfB )    
            {
                a -= MultipleOfB;
                break;
            }
            else
            {
                f32_LoadExponent( MultipleOfB, -1 );
                Magnitude--;
            }
        }
    }

    return( Negative ? -a : a );
    */
}

//==============================================================================
//  TRIGONOMETRY
//==============================================================================

//==============================================================================
//  Contants used for various trigonometry functions.
//  Note that PI_OVER_TWO_HI + PI_OVER_TWO_LO = PI/2.
//  These two values together have 2 times the accuracy of a 32-bit float.

#define PI_OVER_TWO_HI   1.5707960f
#define PI_OVER_TWO_LO   3.1391647e-7f

#if !( defined TARGET_XBOX || defined TARGET_PC )

//==============================================================================

static
f32 SineAndCosine( radian Angle, int QuadrantOffset )
{
    f32   Result;
    f32   Squared;
    s32   Quadrant;     // (-45..45)=0  (45..135)=1  (135..225)=2  and so on

    // Watch out for trouble!
    ASSERT( x_isvalid(Angle) );

    // Convert angle to form where 360 degrees maps to 4.0f.  With this mapping, 
    // we can easily pick out the quadrant.
    Result = Angle * (2.0f / PI);

    // Get the Quadrant.
    Quadrant = (s32)( (Result > 0) ? Result + 0.5f : Result - 0.5f );

    // Modulate into -PI/4 through PI/4.
    Angle = (Angle - (Quadrant * PI_OVER_TWO_HI)) - (Quadrant * PI_OVER_TWO_LO);

    // Account for any quadrant offset.  Function x_cos uses this.
    Quadrant += QuadrantOffset;

    // Both approximations need this value, so compute it now.
    Squared = Angle * Angle;

    // Depending on where we are in the function, we use a different 
    // approximation.
    if( Quadrant & 0x01 )
    {
        // This approximates cosine in (-45..45).  And sine in (45..135).
        //
        //  cos(x) = x^0/0! - x^2/2! + x^4/4! - x^6/6! + x^8/8!
        //
        //         = 1/0! - x^2 * (1/2! + x^2/4! - x^4/6! + x^6/8!)
        //         :
        //         :
        //         = 1/0! - x^2 * (1/2! - x^2 * (1/4! - x^2 * (1/6! - x^2 * (1/8!)))))

        Result = (1/1.0f - Squared * 
                    (1/2.0f - Squared * 
                        (1/24.0f - Squared * 
                            (1/720.0f - Squared * 
                                (1/40320.0f)))));
    }
    else
    {
        // This approximates sine in (-45..45).  And cosine in (-135..-45).
        //
        //  sin(x) = x^1/1! - x^3/3! + x^5/5! - x^7/7! + x^9/9!
        //
        //         = x * (x^0/1! - x^2/3! + x^4/5! - x^6/7! + x^8/9!)
        //         :
        //         :
        //         = x * (1/1! + x^2 * (-1/3! + x^2 * (1/5! + x^2 * (-1/7! + x^2 * (1/9!)))))

        Result = Angle *                                   
                    (1/1.0f + Squared *                     
                        (-1/6.0f + Squared * 
                            (1/120.0f + Squared * 
                                (-1/5040.0f + Squared * 
                                    (1/362880.0f)))));
    }

    return( (Quadrant & 0x02) ? -Result : Result );
}

//==============================================================================

f32 x_sin( radian Angle )
{
    return( SineAndCosine( Angle, 0 ) );
}

//==============================================================================

f32 x_cos( radian Angle )
{
    return( SineAndCosine( Angle, 1 ) );
}

//==============================================================================

void x_sincos( radian Angle, f32& Sine, f32& Cosine )
{
    f32   Result1;
    f32   Result2;
    f32   Squared;
    s32   Quadrant;

    ASSERT( x_isvalid(Angle) );

    Result1  = Angle * (2.0f / PI);
    Quadrant = (s32)( (Result1 > 0) ? Result1 + 0.5f : Result1 - 0.5f );
    Angle    = (Angle - (Quadrant * PI_OVER_TWO_HI)) - (Quadrant * PI_OVER_TWO_LO);
    Squared  = Angle * Angle;

    Result1 = (1/1.0f - Squared * 
                (1/2.0f - Squared * 
                    (1/24.0f - Squared * 
                        (1/720.0f - Squared * 
                            (1/40320.0f)))));

    Result2 = Angle *                                   
                (1/1.0f + Squared *                     
                    (-1/6.0f + Squared * 
                        (1/120.0f + Squared * 
                            (-1/5040.0f + Squared * 
                                    (1/362880.0f)))));

    if( Quadrant & 0x01 )
    {
        Sine   = ((Quadrant+0) & 0x02) ? -Result1 : Result1;
        Cosine = ((Quadrant+1) & 0x02) ? -Result2 : Result2;
    }
    else
    {
        Sine   = ((Quadrant+1) & 0x02) ? -Result2 : Result2;
        Cosine = ((Quadrant+0) & 0x02) ? -Result1 : Result1;
    }
}

//==============================================================================

f32 x_atan( f32 x )
{
    // Arctan function as described by Robin Green in his presentation
    // "Faster Math Functions" at GDC.
    f32   k, b;
    f32   Sign = 1.0f;
    xbool Compliment = FALSE;
    
    // Remove sign from the equation. This will reduce the range we have to
    // approximate so it will help us to get more accurate results. (We know
    // that atan(-x)=-atan(x), so we can add the sign back in later.)
    if( x < 0.0f )
    {
        Sign = -Sign;
        x    = -x;
    }

    // Compliment if x > 1
    // Again this will reduce the range we have to approximate so that it is
    // between 0 and 1. A HUGE range reduction that will greatly reduce error.
    // We can do this because arctan has this very interesting property:
    // atan(x) = (PI/2) - atan(1/x)
    if( x > 1.0f )
    {
        Compliment = TRUE;
        x = 1.0f / x;
    }

    // We'll first use the double angle formula to do range reduction. This
    // should help out on any error we get with the polynomial approximation.
    // Using the double angle formula we can describe x=tan(a+b),
    // so atan(x)=a+b. What are a and b?
    // x=(tan(a)+tan(b))/(1-tan(a)tan(b)) <====== double angle formula
    // if we let z=tan(a) then a=atan(z)
    // if we let k=tan(b) then b=atan(k)
    // therefore,
    // x=(z+k)/(1-zk), and so then,
    // z=(x-k)/(1+kx)
    // then,
    // atan(x) = atan(k)+atan(z)
    //         = b+atan(z)
    //         = b+atan((x-k)/(1+kx))
    // So this remaps input x into the smaller z-range.

    // Doing the x-z range reduction.
    if( x < 0.5f )
    {
        k = 0.25f;
        b = 0.24497866312686415417208248121128f;    // atan(0.25)
    }
    else
    {
        k = 0.75f;
        b = 0.64350110879328438680280922871732;     // atan(0.75)
    }
    x = (x-k) / (1.0f+k*x);

    // Now do the polynomial approximation. Since we've reduced the range so much
    // we'll cheat and not use very many terms at all.
    f32 q  = x;
    f32 q2 = q * q;
    f32 q3 = q2 * q;
    f32 q5 = q3 * q2;
    f32 approx = q                    -
                 q3 * 0.333333333333f +
                 q5 * 0.200000000000f;
    
    // now getting it back into range...
    x = b + approx;

    // finish the compliment
    if( Compliment )
    {
        x = R_90 - x;
    }

    // reinstate the sign and return
    x = x * Sign;
    return x;
}

//==============================================================================

radian x_atan2 ( f32 y, f32 x )
{
    f32 z;
    f32 zabs;
    u32 sign;
    static f32 SignArray[] = { 0, R_180, 0, -R_180 };

    // Collapse small values to 0.0 and watch for zero length
    s32 Collapse = 0;
    if( (x > -0.00001f) && (x < 0.00001f) ) {x = 0.0f; Collapse++;}
    if( (y > -0.00001f) && (y < 0.00001f) ) {y = 0.0f; Collapse++;}
    if( Collapse == 2 )
        return 0.0f;

    // Check if at 90 or -90 degrees
    if( x == 0.0f )
    {
        if( y >= 0.0f ) return  R_90;
        else            return -R_90;
    }

    // Check if at 0 or 180 degrees
    if( y == 0.0f )
    {
        if( x >= 0.0f ) return 0.0f;
        else            return R_180;
    }

    // decide on sign index
    if( x < 0 )  sign = 1;
    else         sign = 0;
    if( y < 0 )  sign |= 2;

    // find the ratio
    z = (y/x);
    if( sign && sign != 3 ) zabs = -z;
    else                    zabs =  z;

    // Handle very small numbers
    if( zabs < 0.000001f )
    {
        return SignArray[sign];
    }

    // compute regular tan
    return (x_atan(z) + SignArray[sign]);
}  

//==============================================================================

f32 x_asin( f32 a )
{
    // asin is only defined for the range -1..1, we'll try to survive the case
    // where it goes out of range, but that is considered bad input.
    ASSERT( IN_RANGE( -1.001f, a, 1.001f ) );
    if( a <= -1.0f )
        return -R_90;
    if( a >= 1.0f )
        return R_90;

    // Use the identity asin(x) = atan(x/sqrt(1-x*x)) to our advantage to avoid
    // writing custom asin code.
    f32 Denom = x_sqrt( 1.0f - a*a );
    if( Denom > 0.000001f )
    {
        return x_atan( a / Denom );
    }
    else
    {
        return R_90;
    }
}

//==============================================================================

f32 x_acos( f32 a )
{
    // use the identity acos(x) = (PI/2) - asin(x) to our advantage to avoid
    // writing custom acos code
    return R_90 - x_asin( a );
}

//==============================================================================

#endif

//==============================================================================
