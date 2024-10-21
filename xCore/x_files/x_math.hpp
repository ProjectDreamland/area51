//==============================================================================
//
//  x_math.hpp
//
//==============================================================================

#ifndef X_MATH_HPP
#define X_MATH_HPP

//==============================================================================
//
//  This file provides cross platform math support.  Lots of math support.  The
//  following areas are present:
//
//    - Basic math functions.  Similar to standard C math library.
//
//    - Definition of PI.
//    - Various radian support macros and definitions.
//
//    - radian ....................... Standard angular representation.                      
//    - radian3 ...................... Orientation via roll, pitch, yaw.
//
//    - vector2, vector3, vector4
//    - matrix4
//  
//    - plane
//    - bbox ......................... Axis aligned bounding box
//
//  TO DO: m4 transposition, matrix3, triangle?
//
//==============================================================================

//==============================================================================
//  DISCUSSION OF MATRIX4
//==============================================================================
//  
//  In the x_files, matrices are LOGICALLY represented as follows:
//  
//      +----+----+----+----+
//      | Rx | Ry | Rz | Tx |
//      |----+----+----+----|
//      | Rx | Ry | Rz | Ty |
//      |----+----+----+----|
//      | Rx | Ry | Rz | Tz |
//      |----+----+----+----|
//      |  0 |  0 |  0 |  1 |
//      +----+----+----+----+
//  
//  A "point" is a "column vector".  To transform a point using a matrix, the 
//  point is "post-multiplied" with the matrix.  Operations performed by a 
//  sequence of matrix multiplications occur right to left.  Thus:
//  
//      |X|     | M |     | M |     | M |     |x|
//      |Y|  =  |op3|  *  |op2|  *  |op1|  *  |y|
//      |Z|     |   |     |   |     |   |     |z|
//  
//  Looking at it mathematically:
//  
//      |X|   |5 2 3|   |x|                       X = 5x + 2y + 3z   
//      |Y| = |7 6 1| * |y|          and          Y = 7x + 6y + 1z   
//      |Z|   |8 4 9|   |z|                       Z = 8x + 4y + 9z   
//                         
//  Note that this logical representation of matrices is the more common form
//  used in recent books and literature regarding 3D graphics.
//  
//==============================================================================
//  
//------------------------------------------------------------------------------
//  WARNING -- THE PHYSICAL LAYOUT OF MATRIX4 IS CHANGING!  
//             DO NOT RELY UPON THIS COMMENT SECTION!
//------------------------------------------------------------------------------
//X 
//X 
//X Matrices are PHYSICALLY stored with the following linear memory layout:
//X 
//X          0    1    2    3   
//X       +----+----+----+----+      +----+----+----+----+
//X     0 | 00 | 04 | 08 | 12 |      | Rx | Ry | Rz | Tx |      Tx = M[12]
//X       |----+----+----+----|      |----+----+----+----|      Tx = M[3][0]
//X     1 | 01 | 05 | 09 | 13 |      | Rx | Ry | Rz | Ty |
//X       |----+----+----+----|      |----+----+----+----|
//X     2 | 02 | 06 | 10 | 14 |      | Rx | Ry | Rz | Tz |
//X       |----+----+----+----|      |----+----+----+----|
//X     3 | 03 | 07 | 11 | 15 |      |  0 |  0 |  0 |  1 |
//X       +----+----+----+----+      +----+----+----+----+
//X 
//X The data within a matrix4 is stored as a two dimensional array.
//X 
//X     f32 M[4][4];
//X 
//X Matrix elements are accessed via M[col][row].  Note that this differs from 
//X the way elements are normally accessed in standard 2D arrays in C/C++.  So,
//X the "Tx" element is M[3][0].
//X 
//X This representation is "assignment compatible" with 4x4 matrices in both
//X OpenGL and DirectX.  For example:
//X 
//X     matrix4 M;
//X 
//X     glMatrixMode ( GL_MODELVIEW );
//X     glLoadMatrixf( &M );
//X 
//X     SetTransform( D3DTRANSFORMSTATE_WORLD, &(D3DMATRIX(M)) );
//X 
//==============================================================================
//  
//  Given a matrix which transforms Source (SRC) space to a Destination (DST) 
//  space, the following holds:
//  
//      +----+----+----+----+       R012 is SRC space X-axis in DST space
//      | R0 | R3 | R6 | TX |       R345 is SRC space Y-axis in DST space
//      |----+----+----+----|       R678 is SRC space Z-axis in DST space
//      | R1 | R4 | R7 | TY |       
//      |----+----+----+----|       TXYZ is SRC space origin in DST space
//      | R2 | R5 | R8 | TZ |       
//      |----+----+----+----|       R036 is DST space X-axis in SRC space
//      |  0 |  0 |  0 |  1 |       R147 is DST space Y-axis in SRC space
//      +----+----+----+----+       R258 is DST space Z-axis in SRC space
//                                  
//  Example:                        
//                                  
//      Given a matrix W2V which transforms from World space into View (or 
//      camera) space.  What is a "line of sight" vector in World space?
//  
//      A "line of sight" unit vector is just a view space (0,0,1) expressed in
//      world space.  From above, we know that "R258" is the destination (view) 
//      space's z-axis in the source (world) space.  This is exactly what we 
//      need.  So:
//  
//          vector3  LOS;           // Line Of Sight
//          LOS.X = W2V(0,2);       // R2
//          LOS.Y = W2V(1,2);       // R5
//          LOS.Z = W2V(2,2);       // R8
//  
//      Similarly, the "view up" and "view left" can easily be extracted.  And
//      these vectors can be useful for things like 3D sprites for particle 
//      systems.
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#endif
                 
// This is included so inline routines can use asserts       
#ifndef X_DEBUG_HPP
#include "x_debug.hpp"
#endif

#if defined(TARGET_PS2)
    #define USE_VU0                1
    #define FORCE_ALIGNED_16( x )  
    #define FORCE_ALIGNED_64( x )
    //extern s32 x_GetThreadID        (void);
    //extern s32 x_GetMainThreadID    (void);
    //#define FORCE_ALIGNED_16( x )  { ASSERT( (((u32)(x))&0x0f)==0 ); ASSERT( x_GetThreadID() == x_GetMainThreadID() ); }
    //#define FORCE_ALIGNED_64( x )  { ASSERT( (((u32)(x))&0x3f)==0 ); ASSERT( x_GetThreadID() == x_GetMainThreadID() ); }
#else
    #define USE_VU0                0
    #define FORCE_ALIGNED_16( x )  
    #define FORCE_ALIGNED_64( x )
#endif

//==============================================================================
//  TYPE DECLARATIONS
//==============================================================================

typedef f32 radian;

struct  vector2;
#ifdef TARGET_PS2
struct  vector3;
struct  vector4;
#else
union   vector3;
union   vector4;
#endif
struct  radian3;
struct  quaternion;
class   matrix3;
union   matrix4;
struct  plane;
struct  bbox;

//==============================================================================
//  DEFINES
//==============================================================================

#ifdef PI
#undef PI
#endif
#define PI  (3.14159265358979323846f)

//------------------------------------------------------------------------------
//
//  Macros for radian angles:
//
//      RADIAN( degrees )       - Angle in radians, given angle in degrees.
//      DEG_TO_RAD( degrees )   - Angle in radians, given angle in degrees.
//      RAD_TO_DEG( radians )   - Angle in degrees, given angle in radians.
//
//      R_0, R_1, ... R_360     - Angles in radians of all integral degree 
//                                angles from 0 to 360.
//
//  As an aside:  Note that "R_0" means "no rotation" whereas "0.0f" just means
//  "zero".  What's the difference?  None to the compiler.  But "R_0" gives more
//  information to anybody reading the code.  Think of it as "strong typing".
//
//------------------------------------------------------------------------------

#define RADIAN(A)     ((f32)((A) * 0.0174532925199432957692369055556f))
#define DEG_TO_RAD(A) ((f32)((A) * 0.0174532925199432957692369055556f))
#define RAD_TO_DEG(A) ((f32)((A) * 57.295779513082320876798161804285f))

#define R_0     RADIAN(   0 )
#define R_1     RADIAN(   1 )
#define R_2     RADIAN(   2 )
#define R_3     RADIAN(   3 )
#define R_4     RADIAN(   4 )
#define R_5     RADIAN(   5 )
//
// Definitions for R_6 through R_354 are in x_math_inline.hpp.
//
#define R_355   RADIAN( 355 )
#define R_356   RADIAN( 356 )
#define R_357   RADIAN( 357 )
#define R_358   RADIAN( 358 )
#define R_359   RADIAN( 359 )
#define R_360   RADIAN( 360 )

//==============================================================================
//  BASIC MATH FUNCTIONS
//==============================================================================
//
//  These functions are patterned after the comperable ANSI standard C library 
//  functions.  However...
//
//  *** ATTENTION ***  These functions all operate on floats (f32)!  
//                     Not doubles (f64)!
//
//  x_sqrt      - Square root.
//  x_floor     - Floor.
//  x_ceil      - Ceiling.
//  x_log       - Log base e.  Natural log.
//  x_log10     - Log base 10.
//  x_exp       - Raise e to a power.
//  x_pow       - Raise a number to a power.
//  x_fmod      - True modulus between two numbers.
//  x_modf      - Break number into whole (integer) and fractional parts.
//  x_frexp     - Break number into exponent and mantissa.
//  x_ldexp     - Assemble number from exponent and mantissa.
//  x_parametric- Return parametric value between the two ranges
//  x_clamp     - clamp input within the specified range
//                       
//  Additional functions:
//                
//  x_sqr       - Square.
//  x_1sqrt     - One over square root.  Optimized per platform.
//  x_lpr       - Least Positive Residue.  Non-negative modulus value.
//  x_abs       - Absolute value of any signed numeric type.
//  x_log2      - Log base 2.
//  x_round     - Round a number to nearest multiple of another number.
//  x_isvalid   - Returns TRUE if value is valid (not INF or NAN).
//
//  SQR         - Simple macro to square a value.
//
//  The ANSI functions abs() and fabs() are both satisfied by x_abs() which 
//  is overloaded for each signed numeric type.
//
//==============================================================================

f32     x_sqr       ( f32 a );
f32     x_sqrt      ( f32 a );
f32     x_1sqrt     ( f32 a );
f32     x_floor     ( f32 a );
f32     x_ceil      ( f32 a );
f32     x_log       ( f32 a );
f32     x_log2      ( f32 a );
f32     x_log10     ( f32 a );
f32     x_exp       ( f32 a );
f32     x_pow       ( f32 a, f32 b );
f32     x_fmod      ( f32 a, f32 b );
f32     x_lpr       ( f32 a, f32 b );
f32     x_round     ( f32 a, f32 b );
f32     x_modf      ( f32 a, f32* pWhole );
f32     x_frexp     ( f32 a, s32* pExp   );
f32     x_ldexp     ( f32 a, s32   Exp   );
f32     x_parametric( f32 V, f32 ValueAtT0, f32 ValueAtT1, xbool bClamp=FALSE );
f32     x_clamp     ( f32 V, f32 VMin, f32 VMax );

xbool   x_isvalid   ( f32 a );

#define SQR(a)  ((a) * (a))
   
//==============================================================================
//  TRIGONOMETRIC MATH FUNCTIONS
//==============================================================================
//
//  These functions are patterned after the comperable ANSI standard C library 
//  functions.  However...
//
//  *** ATTENTION ***  These functions all operate on floats (f32)!  
//                     Not doubles (f64)!
//
//  Pay careful attention to the use of f32 versus radian.
//  
//  x_sin          - Sine.
//  x_cos          - Cosine.
//  x_tan          - Tangent.
//  x_asin         - Arc sine.
//  x_acos         - Arc cosine.
//  x_atan         - Arc tangent.
//  x_atan2        - Standard "atan2" arc tangent where y can equal 0.
//                
//  Additional functions:
//
//  x_sincos       - Sine and cosine in one function call.
//  x_ModAngle     - Provide equivalent angle in [    0, 360 ) degrees.
//  x_ModAngle2    - Provide equivalent angle in [ -180, 180 ) degrees.
//  x_MinAngleDiff - Provide smallest angle between two given angles.
//
//==============================================================================

f32     x_sin           ( radian Angle   );
f32     x_cos           ( radian Angle   );
f32     x_tan           ( radian Angle   );
radian  x_asin          ( f32    Sine    );
radian  x_acos          ( f32    Cosine  );
radian  x_atan          ( f32    Tangent );
radian  x_atan2         ( f32 y, f32 x   );

radian  x_ModAngle      ( radian Angle );
radian  x_ModAngle2     ( radian Angle );
radian  x_MinAngleDiff  ( radian Angle1, radian Angle2 );

//==============================================================================

#if ( defined TARGET_XBOX || defined TARGET_PC )

/* fast sine-cosine */

__forceinline __declspec(naked) void __cdecl x_sincos( radian Angle,f32& Sin,f32& Cos )
{
    __asm
    {
        fld     dword ptr [esp+4]
        fsincos
        mov     eax,[esp+ 8]    // sin
        mov     edx,[esp+12]    // cos
        fstp    dword ptr [edx] // st0=sin
        fstp    dword ptr [eax] // st1=cos
        ret
    }
}

#else

void    x_sincos        ( radian Angle, f32& Sine, f32& Cosine );

#endif

//==============================================================================

#include "Implementation/x_math_inline.hpp"

//==============================================================================
//  RADIAN3
//==============================================================================

struct radian3
{

//------------------------------------------------------------------------------
//  Fields
//------------------------------------------------------------------------------

    radian      Pitch,  Yaw,  Roll;

//------------------------------------------------------------------------------
//  Functions
//------------------------------------------------------------------------------
                                
                        radian3             ( void ); 
                        radian3             ( const radian3& R ); 
                        radian3             ( radian Pitch, radian Yaw, radian Roll );
                        radian3             ( const quaternion& Q );
                        radian3             ( const matrix4& M );
                                            
        void            Zero                ( void );

        void            Set                 ( radian Pitch, radian Yaw, radian Roll );
        void            operator ()         ( radian Pitch, radian Yaw, radian Roll );

        void            ModAngle            ( void ) ;
        void            ModAngle2           ( void ) ;
        radian3         MinAngleDiff        ( const radian3& R ) const;

        f32             Difference          ( const radian3& R ) const;
        xbool           InRange             ( radian Min, radian Max ) const;
        xbool           IsValid             ( void ) const;

friend  radian3         r3_MinAngleDiff     ( const radian3& R1, const radian3& R2 );


        radian3         operator -          ( void ) const;
const   radian3&        operator =          ( const radian3& R );
        radian3&        operator +=         ( const radian3& R );
        radian3&        operator -=         ( const radian3& R );
        radian3&        operator *=         ( f32 Scalar );
        radian3&        operator /=         ( f32 Scalar );
                                 
        xbool           operator ==         ( const radian3& R ) const;
        xbool           operator !=         ( const radian3& R ) const;
                                 
friend  radian3         operator +          ( const radian3& R1, const radian3& R2 );
friend  radian3         operator -          ( const radian3& R1, const radian3& R2 );
friend  radian3         operator /          ( const radian3& R,        f32      S  );
friend  radian3         operator *          ( const radian3& R,        f32      S  );
friend  radian3         operator *          (       f32      S,  const radian3& R  );


};


//==============================================================================
//  VECTOR2
//==============================================================================

struct vector2
{

//------------------------------------------------------------------------------
//  Fields
//------------------------------------------------------------------------------

    f32     X, Y;                                                      

//------------------------------------------------------------------------------
//  Functions
//------------------------------------------------------------------------------

                        vector2             ( void );
                        vector2             ( const vector2& V );
                        vector2             ( f32 X, f32 Y );
                        vector2             ( radian Angle );
        
        void            Set                 ( f32 X, f32 Y );
        void            operator ()         ( f32 X, f32 Y );

        f32             operator []         ( s32 Index ) const;
        f32&            operator []         ( s32 Index );

        void            Zero                ( void );
        void            Negate              ( void );
        xbool           Normalize           ( void );
        xbool           NormalizeAndScale   ( f32 Scalar );
        void            Scale               ( f32 Scalar );
        f32             Length              ( void ) const;
        f32             LengthSquared       ( void ) const;
        radian          Angle               ( void ) const;
        f32             Difference          ( const vector2& V ) const;
        xbool           InRange             ( f32 Min, f32 Max ) const;
        xbool           IsValid             ( void ) const;
                                            
        void            Rotate              ( radian Angle );
        
        f32             Dot                 ( const vector2& V );
friend  f32             v2_Dot              ( const vector2& V1, const vector2& V2 );

                        operator const f32* ( void ) const;

        vector2         operator -          ( void ) const;
const   vector2&        operator =          ( const vector2& V );
        vector2&        operator +=         ( const vector2& V );
        vector2&        operator -=         ( const vector2& V );
        vector2&        operator *=         ( f32 Scalar );
        vector2&        operator /=         ( f32 Scalar );
                                        
        xbool           operator ==         ( const vector2& V ) const;
        xbool           operator !=         ( const vector2& V ) const;
                                
friend  vector2         operator +          ( const vector2& V1, const vector2& V2 );
friend  vector2         operator -          ( const vector2& V1, const vector2& V2 );
friend  vector2         operator /          ( const vector2& V,        f32      S  );
friend  vector2         operator *          ( const vector2& V,        f32      S  );
friend  vector2         operator *          (       f32      S,  const vector2& V  );

};                                          


//==============================================================================
//  VECTOR3
//==============================================================================

PC_ALIGNMENT(16)
#ifdef TARGET_PS2
struct vector3
#else
union vector3
#endif
{


//------------------------------------------------------------------------------
//  Functions
//------------------------------------------------------------------------------

                        vector3                 ( void );                     
                        vector3                 ( f32 X, f32 Y, f32 Z ); 
                        vector3                 ( radian Pitch, radian Yaw );
                        vector3                 ( const radian3& R );

                        #ifdef TARGET_PS2
                        vector3                 ( u128 V );
                        #endif  

        f32             GetX                    ( void ) const;
        f32&            GetX                    ( void );
        f32             GetY                    ( void ) const;
        f32&            GetY                    ( void );
        f32             GetZ                    ( void ) const;
        f32&            GetZ                    ( void );
        s32             GetIW                   ( void ) const;
        s32&            GetIW                   ( void );
 
        #ifdef TARGET_PS2
        u128            GetU128                 ( void ) const;
        #endif

        void            Set                     ( radian Pitch, radian Yaw );
        void            Set                     ( const radian3& R );

        #ifdef TARGET_PS2
        void            Set                     ( u128 V );
        #endif

        void            Set                     ( f32 X, f32 Y, f32 Z );
        void            operator ()             ( f32 X, f32 Y, f32 Z );

        f32             operator []             ( s32 Index ) const;
        f32&            operator []             ( s32 Index );

        void            Zero                    ( void );                    
        void            Negate                  ( void );                    
        void            Min                     ( f32 Min );
        void            Max                     ( f32 Max );
        void            Min                     ( const vector3& V );
        void            Max                     ( const vector3& V );
        void            Normalize               ( void );                    
        void            NormalizeAndScale       ( f32 Scalar );
        xbool           SafeNormalize           ( void );
        xbool           SafeNormalizeAndScale   ( f32 Scalar );
        void            Scale                   ( f32 Scalar );          
        f32             Length                  ( void ) const; 
        f32             LengthSquared           ( void ) const; 
        f32             Difference              ( const vector3& V ) const;
        xbool           InRange                 ( f32 Min, f32 Max ) const;
        xbool           IsValid                 ( void ) const;
        
        void            Rotate                  ( const radian3& R );           
        void            RotateX                 ( radian Rx );           
        void            RotateY                 ( radian Ry );           
        void            RotateZ                 ( radian Rz );           
        
        f32             Dot                     ( const vector3& V ) const;
        vector3         Cross                   ( const vector3& V ) const;
        vector3         Reflect                 ( const vector3& Normal ) const;
        vector3&        Snap                    ( f32 GridX, f32 GridY, f32 GridZ );

        f32             GetSqrtDistToLineSeg    ( const vector3& Start, const vector3& End ) const;
        vector3         GetClosestPToLSeg       ( const vector3& Start, const vector3& End ) const;
        vector3         GetClosestVToLSeg       ( const vector3& Start, const vector3& End ) const;
        f32             GetClosestPToLSegRatio  ( const vector3& Start, const vector3& End ) const;
		f32				ClosestPointToRectangle ( const vector3& P0,   // Origin from the edges. 
												  const vector3& E0, 
												  const vector3& E1,	
												  vector3&       OutClosestPoint ) const;


friend  f32             v3_Dot                  ( const vector3& V1, const vector3& V2 );
friend  vector3         v3_Cross                ( const vector3& V1, const vector3& V2 );
friend  vector3         v3_Reflect              ( const vector3& V,  const vector3& Normal );
friend  radian          v3_AngleBetween         ( const vector3& V1, const vector3& V2 );

        radian          GetPitch                ( void ) const;
        radian          GetYaw                  ( void ) const;
        void            GetPitchYaw             ( radian& Pitch, radian& Yaw ) const;

        // Returns axis and angle to rotate vector onto DestV
        void            GetRotationTowards      ( const vector3& DestV,
                                                        vector3& RotAxis, 
                                                        radian&  RotAngle ) const;

                        operator const f32*     ( void ) const;
        
        vector3         operator -              ( void ) const;
const   vector3&        operator =              ( const vector4& V );
        vector3&        operator +=             ( const vector3& V );
        vector3&        operator -=             ( const vector3& V );
        vector3&        operator *=             ( f32 Scalar );
        vector3&        operator /=             ( f32 Scalar );
                                 
        xbool           operator ==             ( const vector3& V ) const;
        xbool           operator !=             ( const vector3& V ) const;

friend  vector3         operator +              ( const vector3& V1, const vector3& V2 );
friend  vector3         operator -              ( const vector3& V1, const vector3& V2 );
friend  vector3         operator /              ( const vector3& V,        f32      S  );
friend  vector3         operator *              ( const vector3& V,        f32      S  );
friend  vector3         operator *              (       f32      S,  const vector3& V  );
friend  vector3         operator *              ( const vector3& V1, const vector3& V2 );

//------------------------------------------------------------------------------
//  Fields
//------------------------------------------------------------------------------

protected:

    #ifdef TARGET_XBOX
    f128    XYZW;
    #endif

    #ifdef TARGET_PS2
    u128    XYZW;
    #endif

    #ifndef TARGET_PS2
    struct
    {
        f32 X;
        f32 Y;
        f32 Z;
        f32 W;
    };
    #endif

} PS2_ALIGNMENT(16);

//==============================================================================

struct vector3p
{
                    vector3p                ( void );
                    vector3p                ( f32 aX, f32 aY, f32 aZ );
                    vector3p                ( const vector3& V );
    void            Set                     ( f32 aX, f32 aY, f32 aZ );
    const vector3p& operator =              ( const vector3& V );
                    operator const vector3  ( void )    const;
    
    f32 X, Y, Z;
};

//==============================================================================
//  VECTOR4
//==============================================================================

PC_ALIGNMENT(16)
#ifdef TARGET_PS2
struct vector4
#else
union vector4
#endif
{

//------------------------------------------------------------------------------
//  Functions
//------------------------------------------------------------------------------

                        vector4                 ( void );                     
                        vector4                 ( const vector3& V );    
                        vector4                 ( f32 X, f32 Y, f32 Z, f32 W ); 
                        #ifdef TARGET_PS2
                        vector4                 ( u128 V );
                        #endif  

        f32             GetX                    ( void ) const;
        f32&            GetX                    ( void );
        f32             GetY                    ( void ) const;
        f32&            GetY                    ( void );
        f32             GetZ                    ( void ) const;
        f32&            GetZ                    ( void );
        f32             GetW                    ( void ) const;
        f32&            GetW                    ( void );
        s32             GetIW                   ( void ) const;
        s32&            GetIW                   ( void );

        #ifdef TARGET_PS2
        u128            GetU128                 ( void ) const;
        #endif
                                            
        void            Set                     ( f32 X, f32 Y, f32 Z, f32 W );
        void            operator ()             ( f32 X, f32 Y, f32 Z, f32 W );

        f32             operator []             ( s32 Index ) const;
        f32&            operator []             ( s32 Index );

        void            Zero                    ( void );                    
        void            Negate                  ( void );                    
        void            Normalize               ( void );                    
        void            NormalizeAndScale       ( f32 Scalar );
        xbool           SafeNormalize           ( void );
        xbool           SafeNormalizeAndScale   ( f32 Scalar );
        void            Scale                   ( f32 Scalar );          
        f32             Length                  ( void ) const; 
        f32             LengthSquared           ( void ) const; 
        f32             Dot                     ( const vector4& V ) const;
        f32             Difference              ( const vector4& V ) const;
        xbool           InRange                 ( f32 Min, f32 Max ) const;
        xbool           IsValid                 ( void ) const;
        
                        operator const f32*     ( void ) const;
        
        vector4         operator -              ( void ) const;
const   vector4&        operator =              ( const vector3& V );
        vector4&        operator +=             ( const vector4& V );
        vector4&        operator -=             ( const vector4& V );
        vector4&        operator *=             ( f32 Scalar );
        vector4&        operator /=             ( f32 Scalar );
                                 
        xbool           operator ==             ( const vector4& V ) const;
        xbool           operator !=             ( const vector4& V ) const;

friend  const vector3&  vector3::operator =     ( const vector4& V );
friend  vector4         operator +              ( const vector4& V1, const vector4& V2 );
friend  vector4         operator -              ( const vector4& V1, const vector4& V2 );
friend  vector4         operator /              ( const vector4& V,        f32      S  );
friend  vector4         operator *              ( const vector4& V,        f32      S  );
friend  vector4         operator *              (       f32      S,  const vector4& V  );
friend  vector4         operator *              ( const vector4& V1, const vector4& V2 );

//------------------------------------------------------------------------------
//  Fields
//------------------------------------------------------------------------------

protected:

    #ifdef TARGET_XBOX
    f128    XYWZ;
    #endif

    #ifdef TARGET_PS2
    u128    XYZW;
    #endif

    #ifndef TARGET_PS2
    struct
    {
        f32 X;
        f32 Y;
        f32 Z;
        f32 W;
    };

    struct
    {
        s32 iX;
        s32 iY;
        s32 iZ;
        s32 iW;
    };
    #endif

} PS2_ALIGNMENT(16);


//==============================================================================
//  QUATERNION
//==============================================================================

struct quaternion
{

//------------------------------------------------------------------------------
//  Fields
//------------------------------------------------------------------------------

    f32     X, Y, Z, W;

//------------------------------------------------------------------------------
//  Functions
//------------------------------------------------------------------------------

                    quaternion      ( void );    
                    quaternion      ( const quaternion& Q );
                    quaternion      ( const radian3&    R );
                    quaternion      ( const vector3& Axis, radian Angle );
                    quaternion      ( const matrix4&    M );
                    quaternion      ( f32 X, f32 Y, f32 Z, f32 W );
                                    
        void        Identity        ( void );
        void        Zero            ( void );
        
        void        Invert          ( void );
        void        Normalize       ( void );

        void        Setup           ( const radian3& R );
        void        Setup           ( const vector3& Axis, radian Angle );
        void        Setup           ( const matrix4& M );
        void        Setup           ( const vector3& StartDir, const vector3& EndDir );
        f32         Difference      ( const quaternion& Q ) const;
        xbool       InRange         ( void ) const;
        xbool       IsValid         ( void ) const;

        vector3     GetAxis         ( void ) const;
        radian      GetAngle        ( void ) const;

        radian3     GetRotation     ( void ) const;

        void        RotateX         ( radian Rx );
        void        RotateY         ( radian Ry );
        void        RotateZ         ( radian Rz );

        void        PreRotateX      ( radian Rx );
        void        PreRotateY      ( radian Ry );
        void        PreRotateZ      ( radian Rz );

        vector3     operator *      ( const vector3& V ) const;
        vector3     Rotate          ( const vector3& V ) const;
        void        Rotate          (       vector3* pDest, 
                                      const vector3* pSource, 
                                            s32      NVerts ) const;

const   quaternion& operator =      ( const quaternion& Q );
        quaternion& operator *=     ( const quaternion& Q );
friend  quaternion  operator *      ( const quaternion& Qa, 
                                      const quaternion& Qb );

// These are the 'correct' traditional spherical blends

friend  quaternion  BlendSlow       ( const quaternion& Q0, 
                                      const quaternion& Q1, f32 T );

friend  quaternion  BlendToIdentitySlow    ( const quaternion& Q1, f32 T );


// These are fast 'cheap' blends 

friend  quaternion  Blend           ( const quaternion& Q0, 
                                      const quaternion& Q1, f32 T );

friend  quaternion  BlendToIdentity ( const quaternion& Q1, f32 T );
};


//==============================================================================
//  MATRIX4
//==============================================================================

PC_ALIGNMENT(16)
union matrix4
{

//------------------------------------------------------------------------------
//  Functions
//------------------------------------------------------------------------------

public:

                        matrix4             ( void );
                        matrix4             ( const radian3&    R );
                        matrix4             ( const quaternion& Q );
                        
                        matrix4             ( const vector3&    Scale,
                                              const radian3&    Rotation,
                                              const vector3&    Translation );

                        matrix4             ( const vector3&    Scale,
                                              const quaternion& Rotation,
                                              const vector3&    Translation );

        
        f32             operator ()         ( s32 Column, s32 Row ) const;
        f32&            operator ()         ( s32 Column, s32 Row );

        void            Zero                ( void );
        void            Identity            ( void );
        xbool           IsIdentity          ( void ) const;
        void            Transpose           ( void );
        void            Orthogonalize       ( void );
        xbool           Invert              ( void );
        xbool           InvertSRT           ( void );
        xbool           InvertRT            ( void );
        f32             Difference          ( const matrix4& M ) const;
        xbool           InRange             ( f32 Min, f32 Max ) const;
        xbool           IsValid             ( void ) const;

        vector3         GetScale            ( void ) const;
        radian3         GetRotation         ( void ) const;
        quaternion      GetQuaternion       ( void ) const;
        vector3         GetTranslation      ( void ) const;
        void            DecomposeSRT        ( vector3& Scale, quaternion& Rot, vector3& Trans );
        void            DecomposeSRT        ( vector3& Scale, radian3&    Rot, vector3& Trans );
        
        void            ClearScale          ( void );
        void            ClearRotation       ( void );
        void            ClearTranslation    ( void );

        void            SetScale            (       f32         Scale );
        void            SetScale            ( const vector3&    Scale );
        void            SetRotation         ( const radian3&    Rotation );
        void            SetRotation         ( const quaternion& Rotation );
        void            SetTranslation      ( const vector3&    Translation );

        void            Scale               (       f32         Scale );
        void            Scale               ( const vector3&    Scale );
        void            Rotate              ( const radian3&    Rotation );
        void            Rotate              ( const quaternion& Rotation );
        void            RotateX             (       radian      Rx );
        void            RotateY             (       radian      Ry );
        void            RotateZ             (       radian      Rz );
        void            Translate           ( const vector3&    Translation );

        void            PreScale            (       f32         Scale );
        void            PreScale            ( const vector3&    Scale );
        void            PreRotate           ( const radian3&    Rotation );
        void            PreRotate           ( const quaternion& Rotation );
        void            PreRotateX          (       radian      Rx );
        void            PreRotateY          (       radian      Ry );
        void            PreRotateZ          (       radian      Rz );
        void            PreTranslate        ( const vector3&    Translation );
        
        void            GetRows             (       vector3& V1,       vector3& V2,       vector3& V3 ) const;
        void            GetColumns          (       vector3& V1,       vector3& V2,       vector3& V3 ) const;
        
#ifdef TARGET_PS2
        u128            GetCol0_U128        ( void ) const;
        u128            GetCol1_U128        ( void ) const;
        u128            GetCol2_U128        ( void ) const;
        u128            GetCol3_U128        ( void ) const;
#endif
        
        void            SetRows             ( const vector3& V1, const vector3& V2, const vector3& V3 );
        void            SetColumns          ( const vector3& V1, const vector3& V2, const vector3& V3 );
        
        void            Setup               ( const vector3& V1, const vector3& V2, radian Angle );
        void            Setup               ( const vector3& Axis,                  radian Angle );
        void            Setup               ( const radian3& Rotation );
        void            Setup               ( const quaternion& Q );
        void            Setup               ( const vector3& Scale,
                                              const radian3& Rotation,
                                              const vector3& Translation );
        void            Setup               ( const vector3&    Scale,
                                              const quaternion& Rotation,
                                              const vector3&    Translation );
        
        vector3         operator *          ( const vector3& V ) const;
        vector4         operator *          ( const vector4& V ) const;

        vector3         Transform           ( const vector3& V ) const;
        void            Transform           (       vector3* pDest, 
                                              const vector3* pSource, 
                                                    s32      NVerts ) const;
        vector4         Transform           ( const vector4& V ) const;
        void            Transform           (       vector4* pDest, 
                                              const vector4* pSource, 
                                                    s32      NVerts ) const;

        // Forces rotation to be unit length and snapped to increments
        void            CleanRotation       ( void );

        // Rotates vector by matrix rotation
        // (ignores matrix translation, but it will scale the vector if the matrix contains a scale)
        vector3         RotateVector        ( const vector3& V ) const;

        // Inverse rotates vector by matrix rotation
        // (ignores matrix translation, but it will scale the vector if the matrix contains a scale)
        vector3         InvRotateVector     ( const vector3& V ) const;
                                                
        matrix4&        operator +=         ( const matrix4& M );
        matrix4&        operator -=         ( const matrix4& M );
        matrix4&        operator *=         ( const matrix4& M );
                                        
friend  matrix4         operator +          ( const matrix4& M1, const matrix4& M2 );
friend  matrix4         operator -          ( const matrix4& M1, const matrix4& M2 );
friend  matrix4         operator *          ( const matrix4& M1, const matrix4& M2 );
friend  matrix4         m4_Transpose        ( const matrix4& M );
friend  matrix4         m4_InvertSRT        ( const matrix4& M );
friend  matrix4         m4_InvertRT         ( const matrix4& M );

//------------------------------------------------------------------------------
//  Fields
//------------------------------------------------------------------------------

protected:

    f32     m_Cell[4][4];

    #ifdef TARGET_PS2
    struct
    {
        u128    m_Col0;
        u128    m_Col1;
        u128    m_Col2;
        u128    m_Col3;
    };
    #endif

    #ifdef TARGET_XBOX
    struct
    {
        f128    m_Col0;
        f128    m_Col1;
        f128    m_Col2;
        f128    m_Col3;
    };
    #endif

    struct
    {
        vector4 m_vCol0;
        vector4 m_vCol1;
        vector4 m_vCol2;
        vector4 m_vCol3;
    };

} PS2_ALIGNMENT(16);

//==============================================================================
//  PLANE
//==============================================================================

PC_ALIGNMENT(16)
struct plane
{

//------------------------------------------------------------------------------
//  Fields
//------------------------------------------------------------------------------

    vector3 Normal;
    f32     D;

//------------------------------------------------------------------------------
//  Functions
//------------------------------------------------------------------------------

                    plane           ( void );
                    plane           ( const vector3& P1, 
                                      const vector3& P2,
                                      const vector3& P3 );
                    plane           ( const vector3& Normal, f32 Distance );
                    plane           ( f32 A, f32 B, f32 C, f32 D );
                                    
        void        Setup           ( const vector3& P1, 
                                      const vector3& P2,
                                      const vector3& P3 );
        void        Setup           ( const vector3& Normal, f32 Distance );
        void        Setup           ( const vector3& P, const vector3& Normal );
        void        Setup           ( f32 A, f32 B, f32 C, f32 D );
                                    
        void        Negate          ( void );
        void        Transform       ( const matrix4& M );
                                    
        f32         Dot             ( const vector3& P ) const;
        f32         Distance        ( const vector3& P ) const;
        xbool       InFront         ( const vector3& P ) const;
        xbool       InBack          ( const vector3& P ) const;
        void        ComputeD        ( const vector3& P );

        f32         Difference      ( const plane& P ) const;
        xbool       InRange         ( f32 Min, f32 Max ) const;
        xbool       IsValid         ( void ) const;
                                    
        xbool       Intersect       (       f32&     t,
                                      const vector3& P0, 
                                      const vector3& P1 ) const;

        void        GetComponents   ( const vector3& V,
                                            vector3& Parallel,
                                            vector3& Perpendicular ) const;

        void        GetOrthoVectors ( vector3& AxisA,
                                      vector3& AxisB ) const;
                                    
        vector3     ReflectVector   ( const vector3& V ) const;

        xbool       ClipNGon        (       vector3* pDst, s32& NDstVerts, 
                                      const vector3* pSrc, s32  NSrcVerts ) const;

        void        GetBBoxIndices  ( s32* pMinIndices, s32* pMaxIndices ) const;

friend  plane       operator *      ( const matrix4& M, const plane& Plane );

} PS2_ALIGNMENT(16);


//==============================================================================
//  BBOX
//==============================================================================

struct bbox
{
//------------------------------------------------------------------------------
//  Fields
//  The order of these fields should not change because they are relied on
//  in other code.    
//------------------------------------------------------------------------------

    vector3 Min, Max;

//------------------------------------------------------------------------------
//  Functions
//------------------------------------------------------------------------------

                    bbox            ( void );
                    bbox            ( const vector3& P1 );
                    bbox            ( const vector3& P1, const vector3& P2 );
                    bbox            ( const vector3& P1, const vector3& P2, const vector3& P3 );
                    bbox            ( const vector3* pVerts, s32 NVerts );
                    bbox            ( const vector3& Center, f32 Radius );
                                    
        void        Clear           ( void );
        void        Set             ( const vector3& Center, f32 Radius );
        void        Set             ( const vector3& P1, const vector3& P2 ); 
        void        operator ()     ( const vector3& P1, const vector3& P2 ); 
                                    
        vector3     GetSize         ( void ) const;
        vector3     GetCenter       ( void ) const;
        f32         GetRadius       ( void ) const;
        f32         GetRadiusSquared( void ) const;
        f32         GetSurfaceArea  ( void ) const;

        f32         Difference      ( const bbox& B ) const;
        xbool       InRange         ( f32 Min, f32 Max ) const;
        xbool       IsValid         ( void ) const;

        void        Inflate         ( f32 x, f32 y, f32 z );
        void        Deflate         ( f32 x, f32 y, f32 z );
        void        Translate       ( f32 x, f32 y, f32 z );
        void        Transform       ( const matrix4& M );
        void        Translate       ( const vector3& T );
                                    
        xbool       Intersect       ( const vector3& Point ) const;
        xbool       Intersect       ( const bbox&    BBox  ) const;
        xbool       Intersect       ( const plane&   Plane ) const;
        xbool       Intersect       (       f32&     t,
                                      const vector3& P0, 
                                      const vector3& P1 ) const;
        xbool       Intersect       ( const vector3& Center,
                                            f32      Radius ) const;
        xbool       Intersect       ( const vector3* pVert,
                                            s32      nVerts ) const;

        xbool       IntersectTriBBox( const vector3& P0,
                                      const vector3& P1,
                                      const vector3& P2 )  const;

                                    
        xbool       Contains        ( const bbox&    BBox  ) const;

        bbox&       AddVerts        ( const vector3* pVerts, s32 NVerts );
                                    
        bbox&       operator +=     ( const bbox&    BBox  );
        bbox&       operator +=     ( const vector3& Point );
                                    
friend  bbox        operator +      ( const bbox&    BBox1, const bbox&    BBox2 );
friend  bbox        operator +      ( const bbox&    BBox,  const vector3& Point );
friend  bbox        operator +      ( const vector3& Point, const bbox&    BBox  );

} PS2_ALIGNMENT(16);

//==============================================================================
//  SPHERE
//==============================================================================

PC_ALIGNMENT(16)
struct sphere
{
    vector3 Pos;
    f32     R;

                    sphere          ( void );
                    sphere          ( const vector3& Pos, f32 R );
                    sphere          ( const bbox& BBox );

        void        Clear           ( void );

        void        Set             ( const vector3& Pos, f32 R );
        bbox        GetBBox         ( void ) const;

        xbool       TestIntersect   ( const vector3& P0, const vector3& P1 ) const;
        s32         TestIntersection( const plane& Plane ) const;

        s32         Intersect       ( f32& t0, f32& t1, const vector3& P0, const vector3& P1 ) const;
        xbool       Intersect       ( f32& t0, const vector3& P0, const vector3& P1 ) const;
//        xbool       Intersert       ( f32& t, vector3& S0, vector3& E0, sphere& Sphere1, vector3& S1, vector3& E1 );
        //xbool       Intersect       ( sphere& Sphere ) const;
        //xbool       Intersect       ( vector3& P0, vector3& P1, vector3& P3 ) const;
} PS2_ALIGNMENT(16);


//==============================================================================
//  RECT
//==============================================================================

struct rect
{
//------------------------------------------------------------------------------
//  Fields
//------------------------------------------------------------------------------

    vector2 Min;
    vector2 Max;

//------------------------------------------------------------------------------
//  Functions
//------------------------------------------------------------------------------

                    rect            ( void );
                    rect            ( const rect& Rect );
                    rect            ( const vector2& Min, const vector2& Max );
                    rect            ( f32 l, f32 t, f32 r, f32 b );

        void        Clear           ( void );
        void        Set             ( const vector2& Min, const vector2& Max );
        void        Set             ( f32 l, f32 t, f32 r, f32 b );

        xbool       Intersect       ( const rect& Rect );
        xbool       Intersect       ( rect& R, const rect& Rect );
    
        void        AddPoint        ( const vector2& Point );
        void        AddRect         ( const rect&    Rect  );

        f32         GetWidth        ( void ) const;
        f32         GetHeight       ( void ) const;
        vector2     GetSize         ( void ) const;
        vector2     GetCenter       ( void ) const;

        void        SetWidth        ( f32 W );
        void        SetHeight       ( f32 H );
        void        SetSize         ( const vector2& S );

        void        Translate       ( const vector2& T );
        void        Inflate         ( const vector2& T );
        void        Deflate         ( const vector2& T );

        f32         Difference      ( const rect& R ) const;
        xbool       InRange         ( f32 Min, f32 Max ) const;
        xbool       IsValid         ( void ) const;
};

//==============================================================================
//  IRECT
//==============================================================================

struct irect
{
//------------------------------------------------------------------------------
//  Fields
//------------------------------------------------------------------------------

    s32 l;
    s32 t;
    s32 r;
    s32 b;

//------------------------------------------------------------------------------
//  Functions
//------------------------------------------------------------------------------

                    irect           ( void );
                    irect           ( const irect& Rect );
                    irect           ( s32 l, s32 t, s32 r, s32 b );

        void        Clear           ( void );
        void        Set             ( s32 l, s32 t, s32 r, s32 b );

        xbool       Intersect       ( const irect& Rect );
        xbool       Intersect       ( irect& R, const irect& Rect );
    
        xbool       PointInRect     ( s32 X, s32 Y );
        void        AddPoint        ( s32 X, s32 Y );
        void        AddRect         ( const irect& Rect  );

        s32         GetWidth        ( void ) const;
        s32         GetHeight       ( void ) const;
        vector2     GetSize         ( void ) const;
        vector2     GetCenter       ( void ) const;

        void        SetWidth        ( s32 W );
        void        SetHeight       ( s32 H );
        void        SetSize         ( s32 W, s32 H );

        void        Translate       ( s32 X, s32 Y );
        void        Inflate         ( s32 X, s32 Y );
        void        Deflate         ( s32 X, s32 Y );

        f32         Difference      ( const irect& R ) const;
        xbool       InRange         ( s32 Min, s32 Max ) const;
        xbool       IsEmpty         ( void ) const;
        xbool       PointInRect     ( s32 X, s32 Y ) const;

        bool        operator ==     ( const irect& R ) const;
        bool        operator !=     ( const irect& R ) const;
};

//==============================================================================
// INLINE FILES
//==============================================================================

#include "Implementation/x_math_r3_inline.hpp"
#include "Implementation/x_math_v2_inline.hpp"
#include "Implementation/x_math_v3_inline.hpp"
#include "Implementation/x_math_v4_inline.hpp"
#include "Implementation/x_math_q_inline.hpp"
#include "Implementation/x_math_m4_inline.hpp"
#include "Implementation/x_math_p_inline.hpp"
#include "Implementation/x_math_bb_inline.hpp"
#include "Implementation/x_math_rect_inline.hpp"
#include "Implementation/x_math_sph_inline.hpp"

//==============================================================================
#endif // X_MATH_HPP
//==============================================================================
