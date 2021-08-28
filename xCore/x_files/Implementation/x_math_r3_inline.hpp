//==============================================================================
//
//  x_math_r3_inline.hpp
//
//==============================================================================

#ifndef X_MATH_R3_INLINE_HPP
#define X_MATH_R3_INLINE_HPP
#else
#error "File " __FILE__ " has been included twice!"
#endif

#ifndef X_DEBUG_HPP
#include "../x_debug.hpp"
#endif

//==============================================================================

inline radian3:: radian3( void )        
{   
}

//==============================================================================

inline radian3::radian3( const radian3& R )
{
    Pitch = R.Pitch;    
    Yaw   = R.Yaw;    
    Roll  = R.Roll;
}

//==============================================================================

inline radian3::radian3( radian aPitch, radian aYaw, radian aRoll )
{
    Pitch = aPitch;     
    Yaw   = aYaw;     
    Roll  = aRoll;
}

//==============================================================================

inline radian3::radian3( const quaternion& Q )
{
   FORCE_ALIGNED_16( &Q );
   *this = Q.GetRotation();
}

//==============================================================================

inline radian3::radian3( const matrix4& M )
{
    FORCE_ALIGNED_16( &M );
    *this = M.GetRotation();
}

//==============================================================================
inline void radian3::Zero( void )
{
    Pitch = 0.0f;
    Yaw   = 0.0f;
    Roll  = 0.0f;
}

//==============================================================================

inline void radian3::Set( radian aPitch, radian aYaw, radian aRoll )
{
    Pitch = aPitch;
    Yaw   = aYaw;  
    Roll  = aRoll;
}

//==============================================================================

inline void radian3::ModAngle( void )
{
    Pitch = x_ModAngle( Pitch ) ;
    Yaw   = x_ModAngle( Yaw ) ;
    Roll  = x_ModAngle( Roll ) ;
}

//==============================================================================

inline void radian3::ModAngle2( void )
{
    Pitch = x_ModAngle2( Pitch ) ;
    Yaw   = x_ModAngle2( Yaw ) ;
    Roll  = x_ModAngle2( Roll ) ;
}

//==============================================================================

inline radian3 radian3::MinAngleDiff( const radian3& R ) const
{
    return radian3( x_MinAngleDiff( Pitch, R.Pitch ),
                    x_MinAngleDiff( Yaw,   R.Yaw ),
                    x_MinAngleDiff( Roll,  R.Roll ) ) ;
}

//==============================================================================

inline radian3 r3_MinAngleDiff( const radian3& R1, const radian3& R2 )
{
    return radian3( x_MinAngleDiff( R1.Pitch, R2.Pitch ),
                    x_MinAngleDiff( R1.Yaw,   R2.Yaw ),
                    x_MinAngleDiff( R1.Roll,  R2.Roll ) ) ;
}

//==============================================================================

inline void radian3::operator () ( radian aPitch, radian aYaw, radian aRoll )
{
    Pitch = aPitch;
    Yaw   = aYaw;  
    Roll  = aRoll;
}

//==============================================================================

inline 
radian3 radian3::operator - ( void ) const
{
    return( radian3( -Pitch, -Yaw, -Roll ) );
}

//==============================================================================

inline 
const radian3& radian3::operator = ( const radian3& R )
{
    Pitch = R.Pitch; 
    Yaw = R.Yaw; 
    Roll = R.Roll;
    return( *this );
}

//==============================================================================

inline 
radian3& radian3::operator += ( const radian3& R )
{
    Pitch += R.Pitch; 
    Yaw += R.Yaw; 
    Roll += R.Roll;
    return( *this );
}

//==============================================================================

inline 
radian3& radian3::operator -= ( const radian3& R )
{
    Pitch -= R.Pitch; 
    Yaw -= R.Yaw; 
    Roll -= R.Roll;
    return( *this );
}

//==============================================================================

inline 
radian3& radian3::operator *= ( const f32 Scalar )
{
    Pitch *= Scalar; 
    Yaw *= Scalar; 
    Roll *= Scalar;
    return( *this );
}

//==============================================================================

inline 
radian3& radian3::operator /= ( const f32 Scalar )
{
    ASSERT( Scalar != 0.0f );
    f32 d = 1.0f / Scalar;
    Pitch *= d; 
    Yaw *= d; 
    Roll *= d;
    return( *this );
}

//==============================================================================

inline 
xbool radian3::operator == ( const radian3& R ) const
{
    return( (Pitch == R.Pitch) && (Yaw == R.Yaw) && (Roll == R.Roll) );
}

//==============================================================================

inline 
xbool radian3::operator != ( const radian3& R ) const
{
    return( (Pitch != R.Pitch) || (Yaw != R.Yaw) || (Roll != R.Roll) );
}

//==============================================================================

inline 
radian3 operator + ( const radian3& R1, const radian3& R2 )
{
    return( radian3( R1.Pitch + R2.Pitch, 
                     R1.Yaw + R2.Yaw, 
                     R1.Roll + R2.Roll ) );
}

//==============================================================================

inline 
radian3 operator - ( const radian3& R1, const radian3& R2 )
{
    return( radian3( R1.Pitch - R2.Pitch, 
                     R1.Yaw - R2.Yaw, 
                     R1.Roll - R2.Roll ) );
}

//==============================================================================

inline 
radian3 operator / ( const radian3& R, f32 S )
{
    ASSERT( (S > 0.00001f) || (S < -0.00001f) );
    S = 1.0f / S;
    return( radian3( R.Pitch * S, 
                     R.Yaw * S, 
                     R.Roll * S ) );
}

//==============================================================================

inline 
radian3 operator * ( const radian3& R, f32 S )
{
    return( radian3( R.Pitch * S, 
                     R.Yaw * S, 
                     R.Roll * S ) );
}

//==============================================================================

inline 
radian3 operator * ( f32 S, const radian3& R )
{
    return( radian3( R.Pitch * S, 
                     R.Yaw * S, 
                     R.Roll * S ) );
}

//==============================================================================

inline
f32 radian3::Difference( const radian3& R ) const
{
    return  (Pitch-R.Pitch)*(Pitch-R.Pitch) +
            (Yaw-R.Yaw)*(Yaw-R.Yaw) +
            (Roll-R.Roll)*(Roll-R.Roll);
}

//==============================================================================

inline
xbool radian3::InRange( radian Min, radian Max ) const
{
    return( (Pitch>=Min) && (Pitch<=Max) &&
            (Yaw  >=Min) && (Yaw  <=Max) &&
            (Roll >=Min) && (Roll <=Max) );
}

//==============================================================================

inline
xbool radian3::IsValid( void ) const
{
    return( x_isvalid(Pitch) && x_isvalid(Yaw) && x_isvalid(Roll) );
}

//==============================================================================


