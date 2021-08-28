#ifndef VU0MATHTEST_INCLUDED
#define VU0MATHTEST_INCLUDED

#include "x_types.hpp"

PC_ALIGNMENT(16)
union vector3t
{
//------------------------------------------------------------------------------
//  Functions
//------------------------------------------------------------------------------

                        vector3t             ( void );                     
//                        vector3t             ( const vector3t& V );    
                        vector3t             ( f32 X, f32 Y, f32 Z ); 
                        vector3t             ( radian Pitch, radian Yaw );
                        vector3t             ( const radian3& R );

        f32             GetX                ( void ) const;
        f32&            GetX                ( void );
        f32             GetY                ( void ) const;
        f32&            GetY                ( void );
        f32             GetZ                ( void ) const;
        f32&            GetZ                ( void );
                                            
        void            Set                 ( radian Pitch, radian Yaw );
        void            Set                 ( const radian3& R );

        void            Set                 ( f32 X, f32 Y, f32 Z );
        void            operator ()         ( f32 X, f32 Y, f32 Z );

        f32             operator []         ( s32 Index ) const;
        f32&            operator []         ( s32 Index );

        void            Zero                ( void );                    
        void            Negate              ( void );                    
        void            Normalize           ( void );                    
        void            NormalizeAndScale   ( f32 Scalar );
        xbool           SafeNormalize           ( void );                    
        xbool           SafeNormalizeAndScale   ( f32 Scalar );
        void            Scale               ( f32 Scalar );          
        f32             Length              ( void ) const; 
        f32             LengthSquared       ( void ) const; 
        f32             Difference          ( const vector3t& V ) const;
        xbool           InRange             ( f32 Min, f32 Max ) const;
        xbool           IsValid             ( void ) const;
        
        void            Rotate              ( const radian3& R );           
        void            RotateX             ( radian Rx );           
        void            RotateY             ( radian Ry );           
        void            RotateZ             ( radian Rz );           
        
        f32             Dot                 ( const vector3t& V ) const;
        vector3t        Cross               ( const vector3t& V ) const;
        vector3t        Reflect             ( const vector3t& Normal ) const;
        vector3t&       Snap                ( f32 GridX, f32 GridY, f32 GridZ );

        f32             GetSqrtDistToLineSeg    ( const vector3t& Start, const vector3t& End ) const;
        vector3t        GetClosestPToLSeg       ( const vector3t& Start, const vector3t& End ) const;
        vector3t        GetClosestVToLSeg       ( const vector3t& Start, const vector3t& End ) const;
        f32             GetClosestPToLSegRatio  ( const vector3t& Start, const vector3t& End ) const;

friend  f32             v3_Dot              ( const vector3t& V1, const vector3t& V2 );
friend  vector3t        v3_Cross            ( const vector3t& V1, const vector3t& V2 );
friend  vector3t        v3_Reflect          ( const vector3t& V,  const vector3t& Normal );
friend  radian          v3_AngleBetween     ( const vector3t& V1, const vector3t& V2 );

        radian          GetPitch            ( void ) const;
        radian          GetYaw              ( void ) const;
        void            GetPitchYaw         ( radian& Pitch, radian& Yaw ) const;

        // Returns axis and angle to rotate vector onto DestV
        void            GetRotationTowards  ( const vector3t& DestV,
                                                    vector3t& RotAxis, 
                                                    radian&  RotAngle ) const;

                        operator const f32* ( void ) const;
        
        vector3t         operator -          ( void ) const;
//const   vector3t&        operator =          ( const vector3t& V );
const   vector3t&        operator =          ( const vector4& V );
        vector3t&        operator +=         ( const vector3t& V );
        vector3t&        operator -=         ( const vector3t& V );
        vector3t&        operator *=         ( f32 Scalar );
        vector3t&        operator /=         ( f32 Scalar );
                                 
        xbool           operator ==         ( const vector3t& V ) const;
        xbool           operator !=         ( const vector3t& V ) const;
                                 
friend  vector3t         operator +          ( const vector3t& V1, const vector3t& V2 );
friend  vector3t         operator -          ( const vector3t& V1, const vector3t& V2 );
friend  vector3t         operator /          ( const vector3t& V,        f32      S  );
friend  vector3t         operator *          ( const vector3t& V,        f32      S  );
friend  vector3t         operator *          (       f32      S,  const vector3t& V  );
friend  vector3t         operator *          ( const vector3t& V1, const vector3t& V2 );

//------------------------------------------------------------------------------
//  Fields
//------------------------------------------------------------------------------

protected:
#ifdef TARGET_PS2
    u128    XYZW;
#endif

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
} PS2_ALIGNMENT(16);

//==============================================================================

PC_ALIGNMENT(16)
union vector4t
{

//------------------------------------------------------------------------------
//  Functions
//------------------------------------------------------------------------------

                        vector4t                 ( void );                     
                        vector4t                 ( const vector3& V );    
                        vector4t                 ( f32 X, f32 Y, f32 Z, f32 W ); 

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
        f32             Dot                     ( const vector4t& V ) const;
        f32             Difference              ( const vector4t& V ) const;
        xbool           InRange                 ( f32 Min, f32 Max ) const;
        xbool           IsValid                 ( void ) const;
        
                        operator const f32*     ( void ) const;
        
        vector4t         operator -              ( void ) const;
const   vector4t&        operator =              ( const vector3& V );
        vector4t&        operator +=             ( const vector4t& V );
        vector4t&        operator -=             ( const vector4t& V );
        vector4t&        operator *=             ( f32 Scalar );
        vector4t&        operator /=             ( f32 Scalar );
                                 
        xbool           operator ==             ( const vector4t& V ) const;
        xbool           operator !=             ( const vector4t& V ) const;
                                 
friend  vector4t         operator +              ( const vector4t& V1, const vector4t& V2 );
friend  vector4t         operator -              ( const vector4t& V1, const vector4t& V2 );
friend  vector4t         operator /              ( const vector4t& V,        f32      S  );
friend  vector4t         operator *              ( const vector4t& V,        f32      S  );
friend  vector4t         operator *              (       f32      S,  const vector4t& V  );

//------------------------------------------------------------------------------
//  Fields
//------------------------------------------------------------------------------

protected:
#ifdef TARGET_PS2
    u128    XYZW;
#endif

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

} PS2_ALIGNMENT(16);


//==============================================================================

#include "VU0MathTest_v3_inline.hpp"
#include "VU0MathTest_v4_inline.hpp"
//#include "VU0MathTest_m4_inline.hpp"

//==============================================================================

#endif // VU0MATHTEST_INCLUDED