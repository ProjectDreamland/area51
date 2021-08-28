//==============================================================================
//
//  x_fbbox.hpp
//
//==============================================================================

#ifndef X_FBBOX_HPP
#define X_FBBOX_HPP

#include "x_math.hpp"

//==============================================================================
//==============================================================================
// FAST BBOX
//==============================================================================
//==============================================================================

struct fbbox
{
    vector4 Min, Max;

                    fbbox           ( void );
                    fbbox           ( const fbbox&    BBox );
                    fbbox           ( const  bbox&    BBox );
                    fbbox           ( const vector4& P0 );
                    fbbox           ( const vector4& P0, const vector4& P1 );
                    fbbox           ( const vector4& P0, const vector4& P1, const vector4& P2 );
                                    
        void        Clear           ( void );
        vector3     GetSize         ( void ) const;
        vector3     GetCenter       ( void ) const;
        f32         GetRadius       ( void ) const;
        f32         GetRadiusSquared( void ) const;
        bbox        GetBBox         ( void ) const;


        void        Inflate         ( f32 x, f32 y, f32 z );
        void        Deflate         ( f32 x, f32 y, f32 z );
        void        Translate       ( f32 x, f32 y, f32 z );

        void        Transform       ( const matrix4& M );

        void        AddVerts        ( const vector4* pVerts, s32 nVerts );
        
        xbool       Intersect       ( const vector4& Point ) const;

        xbool       Intersect       ( const fbbox&    BBox ) const;

        xbool       Intersect       ( const plane&   Plane ) const;

        xbool       Intersect       (       f32&     t,
                                      const vector4& P0, 
                                      const vector4& P1 ) const;

        xbool       Intersect       ( const vector4& Center,
                                            f32      Radius ) const;

        xbool       IntersectTriBBox( const vector4& P0,
                                      const vector4& P1,
                                      const vector4& P2 )  const;

        xbool       ClipRay         ( const vector4& P0,
                                      const vector4& P1,
                                            f32&     T0,
                                            f32&     T1)  const;

        void        SetupFastTests  ( void )              const;
        xbool       FastIntersect   ( const fbbox& BBox ) const;

const   fbbox&      operator =      ( const fbbox&    BBox  );
const   fbbox&      operator =      ( const  bbox&    BBox  );
        fbbox&      operator +=     ( const fbbox&    BBox  );
        fbbox&      operator +=     ( const vector4&  Point );
        fbbox&      operator +=     ( const vector3&  Point );

friend  fbbox       operator +      ( const fbbox&    BBox1, const fbbox&    BBox2 );
friend  fbbox       operator +      ( const fbbox&    BBox,  const vector4&  Point );
friend  fbbox       operator +      ( const vector4&  Point, const fbbox&    BBox  );

}  PS2_ALIGNMENT(16);

//==============================================================================

inline 
fbbox::fbbox( void )
{
}
 
//==============================================================================

inline
void fbbox::Clear( void )
{
    FORCE_ALIGNED_16( this );
    Min(  F32_MAX,  F32_MAX,  F32_MAX, 0 );
    Max( -F32_MAX, -F32_MAX, -F32_MAX, 0 );
}

//==============================================================================

inline 
fbbox::fbbox( const fbbox& BBox )
{
    FORCE_ALIGNED_16( this );
    Min = BBox.Min;
    Max = BBox.Max;
}

//==============================================================================

inline 
fbbox::fbbox( const vector4& P0 )
{
    FORCE_ALIGNED_16( this );
    Min = P0;
    Max = P0;
}

//==============================================================================

inline 
fbbox::fbbox( const vector4& P0, const vector4& P1 )
{
    FORCE_ALIGNED_16( this );
    Min = P0;
    Max = P0;
    *this += P1;
}

//==============================================================================

inline 
fbbox::fbbox( const vector4& P0, const vector4& P1, const vector4& P2 )
{
    FORCE_ALIGNED_16( this );
    Min = P0;
    Max = P0;
    *this += P1;
    *this += P2;
}

//==============================================================================

inline 
fbbox::fbbox( const bbox& BBox )
{
    FORCE_ALIGNED_16( this );
    Min.GetX() = BBox.Min.GetX();
    Min.GetY() = BBox.Min.GetY();
    Min.GetZ() = BBox.Min.GetZ();
    Max.GetX() = BBox.Max.GetX();
    Max.GetY() = BBox.Max.GetY();
    Max.GetZ() = BBox.Max.GetZ();
    Max.GetW() = 0;
    Max.GetW() = 0;
}

//==============================================================================

inline 
bbox fbbox::GetBBox( void ) const
{
    FORCE_ALIGNED_16( this );
    bbox BBox;
    BBox.Min.GetX() = Min.GetX();
    BBox.Min.GetY() = Min.GetY();
    BBox.Min.GetZ() = Min.GetZ();
    BBox.Max.GetX() = Max.GetX();
    BBox.Max.GetY() = Max.GetY();
    BBox.Max.GetZ() = Max.GetZ();
    return BBox;
}

//==============================================================================

inline 
void fbbox::AddVerts( const vector4* pVerts, s32 nVerts )
{
    ASSERT( pVerts );
    ASSERT( nVerts > 0 );
    FORCE_ALIGNED_16( pVerts );
    FORCE_ALIGNED_16( this );

    const vector4* pVertEnd = pVerts + nVerts;

    // Add in all the others.
    while( pVerts != pVertEnd )
    {
        if( pVerts->GetX() < Min.GetX() )     Min.GetX() = pVerts->GetX();
        if( pVerts->GetY() < Min.GetY() )     Min.GetY() = pVerts->GetY();
        if( pVerts->GetZ() < Min.GetZ() )     Min.GetZ() = pVerts->GetZ();
        if( pVerts->GetX() > Max.GetX() )     Max.GetX() = pVerts->GetX();
        if( pVerts->GetY() > Max.GetY() )     Max.GetY() = pVerts->GetY();
        if( pVerts->GetZ() > Max.GetZ() )     Max.GetZ() = pVerts->GetZ();
        pVerts++;
    }
}

//==============================================================================

inline 
vector3 fbbox::GetSize( void ) const
{
    return vector3( Max.GetX()-Min.GetX(), Max.GetY()-Min.GetY(), Max.GetZ()-Min.GetZ() );
}

//==============================================================================

inline 
vector3 fbbox::GetCenter( void ) const
{
    return vector3( (Max.GetX()+Min.GetX())*0.5f, (Max.GetY()+Min.GetY())*0.5f, (Max.GetZ()+Min.GetZ())*0.5f );
}

//==============================================================================

inline 
f32 fbbox::GetRadius( void ) const
{
    return( (Max-Min).Length() * 0.5f );
}

//==============================================================================

inline 
f32 fbbox::GetRadiusSquared( void ) const
{
    vector4 R = (Max-Min) * 0.5f;
    return( (R.GetX()*R.GetX()) + (R.GetY()*R.GetY()) + (R.GetZ()*R.GetZ()) );
}

//==============================================================================

inline
void fbbox::Inflate( f32 x, f32 y, f32 z )
{
    Min.GetX() -= x;
    Min.GetY() -= y;
    Min.GetZ() -= z;
    Max.GetX() += x;
    Max.GetY() += y;
    Max.GetZ() += z;
}

//==============================================================================

inline
void fbbox::Deflate( f32 x, f32 y, f32 z )
{
    Min.GetX() += x;
    Min.GetY() += y;
    Min.GetZ() += z;
    Max.GetX() -= x;
    Max.GetY() -= y;
    Max.GetZ() -= z;
}

//==============================================================================

inline
void fbbox::Translate( f32 x, f32 y, f32 z )
{
    Min.GetX() += x;
    Min.GetY() += y;
    Min.GetZ() += z;
    Max.GetX() += x;
    Max.GetY() += y;
    Max.GetZ() += z;
}

//==============================================================================

inline 
void fbbox::Transform( const matrix4& M )
{
    FORCE_ALIGNED_16( this );
    vector4 AMin, AMax;
    f32     a, b;
    s32     i, j;

    // Copy box A into min and max array.
    AMin = Min;
    AMax = Max;

    // Begin at T.
    Min.GetX() = Max.GetX() = M(3,0);
    Min.GetY() = Max.GetY() = M(3,1);
    Min.GetZ() = Max.GetZ() = M(3,2);

    // Find extreme points by considering product of 
    // min and max with each component of M.
    for( j=0; j<3; j++ )
    {
        for( i=0; i<3; i++ )
        {
            a = M(i,j) * AMin[i];
            b = M(i,j) * AMax[i];

            if( a < b )
            {
                Min[j] += a;
                Max[j] += b;
            }
            else
            {
                Min[j] += b;
                Max[j] += a;
            }
        }
    }
}

//==============================================================================

inline 
xbool fbbox::Intersect( const vector4& Point ) const
{
    FORCE_ALIGNED_16( this );

    return( (Point.GetX() <= Max.GetX()) && 
            (Point.GetY() <= Max.GetY()) && 
            (Point.GetZ() <= Max.GetZ()) && 
            (Point.GetX() >= Min.GetX()) && 
            (Point.GetY() >= Min.GetY()) && 
            (Point.GetZ() >= Min.GetZ()) );
}

//==============================================================================

inline 
xbool fbbox::Intersect( const fbbox& BBox ) const
{
    FORCE_ALIGNED_16( this );

    if( BBox.Min.GetX() > Max.GetX() ) return FALSE;
    if( BBox.Max.GetX() < Min.GetX() ) return FALSE;
    if( BBox.Min.GetZ() > Max.GetZ() ) return FALSE;
    if( BBox.Max.GetZ() < Min.GetZ() ) return FALSE;
    if( BBox.Min.GetY() > Max.GetY() ) return FALSE;
    if( BBox.Max.GetY() < Min.GetY() ) return FALSE;
    return TRUE;
}

//==============================================================================

inline 
xbool fbbox::Intersect( const plane& Plane ) const
{
    vector3 PMin, PMax;
    f32     DMax, DMin;

    if( Plane.Normal.GetX() > 0 )   { PMax.GetX() = Max.GetX();  PMin.GetX() = Min.GetX(); }
    else                            { PMax.GetX() = Min.GetX();  PMin.GetX() = Max.GetX(); }

    if( Plane.Normal.GetY() > 0 )   { PMax.GetY() = Max.GetY();  PMin.GetY() = Min.GetY(); }
    else                            { PMax.GetY() = Min.GetY();  PMin.GetY() = Max.GetY(); }

    if( Plane.Normal.GetZ() > 0 )   { PMax.GetZ() = Max.GetZ();  PMin.GetZ() = Min.GetZ(); }
    else                            { PMax.GetZ() = Min.GetZ();  PMin.GetZ() = Max.GetZ(); }

    DMax = Plane.Distance( PMax );
    DMin = Plane.Distance( PMin );

    return( (DMax > 0.0f) && (DMin < 0.0f ) );
}

//==============================================================================

inline 
xbool fbbox::Intersect( f32& t, const vector4& P0, const vector4& P1 ) const
{
    f32     PlaneD   [3];
    xbool   PlaneUsed[3] = { TRUE, TRUE, TRUE };
    f32     T        [3] = { -1, -1, -1 };
    vector4 Direction    = P1 - P0;
    s32     MaxPlane;
    s32     i;
    f32     Component;

    Direction.GetW() = 0;

    // Set a value until we have something better.
    t = 0.0f;

    // Consider relationship of each component of P0 to the box.
    for( i = 0; i < 3; i++ )
    {
        if     ( P0[i] > Max[i] )   { PlaneD[i]    = Max[i]; }
        else if( P0[i] < Min[i] )   { PlaneD[i]    = Min[i]; }
        else                        { PlaneUsed[i] = FALSE;  }
    }
    
    // Is the starting point in the box?
    if( !PlaneUsed[0] && !PlaneUsed[1] && !PlaneUsed[2] )
        return( TRUE );

    // For each plane to be used, compute the distance to the plane.
    for( i = 0; i < 3; i++ )
    {
        if( PlaneUsed[i] && (Direction[i] != 0.0f) )
            T[i] = (PlaneD[i] - P0[i]) / Direction[i];
    }

    // We need to know which plane had the largest distance.
    if( T[0] > T[1] )
    {
        MaxPlane = ((T[0] > T[2]) ? 0 : 2);
    }
    else
    {
        MaxPlane = ((T[1] > T[2]) ? 1 : 2);
    }

    // If the largest plane distance is less than zero, then there is no hit.
    if( T[MaxPlane] < 0.0f )
        return( FALSE );

    // See if the point we think is the hit point is a real hit.
    for( i = 0; i < 3; i++ )
    {
        // See if component 'i' of the hit point is on the box.
        if( i != MaxPlane )
        {
            Component = P0[i] + T[MaxPlane] * Direction[i];
            if( (Component < Min[i]) || (Component > Max[i]) )
            {
                // We missed!  Hit point was not on the box.
                return( FALSE );
            }
        }
    }

    // We have a verified hit.  Set t and we're done.
    t = T[MaxPlane];
    return( TRUE );
}

//==============================================================================

inline
xbool fbbox::Intersect( const vector4& Center, f32 Radius ) const
{
    f32     d,dmin = 0;

    // X Axis
    if( Center.GetX() < Min.GetX() )      
    {
        d = Center.GetX() - Min.GetX();    
        dmin += d*d; 
    }
    else if (Center.GetX() > Max.GetX() )
    {
        d = Center.GetX() - Max.GetX();
        dmin+= d*d;
    }

    // Y Axis
    if( Center.GetY() < Min.GetY() )      
    {
        d = Center.GetY() - Min.GetY();    
        dmin += d*d; 
    }
    else if (Center.GetY() > Max.GetY() )
    {
        d = Center.GetY() - Max.GetY();
        dmin+= d*d;
    }

    // Z Axis
    if( Center.GetZ() < Min.GetZ() )      
    {
        d = Center.GetZ() - Min.GetZ();    
        dmin += d*d; 
    }
    else if (Center.GetZ() > Max.GetZ() )
    {
        d = Center.GetZ() - Max.GetZ();
        dmin+= d*d;
    }
            
    if( dmin <= Radius*Radius ) 
        return TRUE;

    return FALSE;
}

//==============================================================================

inline 
const fbbox& fbbox::operator = ( const fbbox& BBox )
{
    Min = BBox.Min;
    Max = BBox.Max;

    return( *this );
}

//==============================================================================

inline 
const fbbox& fbbox::operator = ( const bbox& BBox )
{
    Min.GetX() = BBox.Min.GetX();
    Min.GetY() = BBox.Min.GetY();
    Min.GetZ() = BBox.Min.GetZ();
    Max.GetX() = BBox.Max.GetX();
    Max.GetY() = BBox.Max.GetY();
    Max.GetZ() = BBox.Max.GetZ();
    Min.GetW() = 0;
    Max.GetW() = 0;
    return( *this );
}

//==============================================================================

inline 
fbbox& fbbox::operator += ( const fbbox& BBox )
{
    if( BBox.Min.GetX() < Min.GetX() )     Min.GetX() = BBox.Min.GetX();
    if( BBox.Min.GetY() < Min.GetY() )     Min.GetY() = BBox.Min.GetY();
    if( BBox.Min.GetZ() < Min.GetZ() )     Min.GetZ() = BBox.Min.GetZ();
    if( BBox.Max.GetX() > Max.GetX() )     Max.GetX() = BBox.Max.GetX();
    if( BBox.Max.GetY() > Max.GetY() )     Max.GetY() = BBox.Max.GetY();
    if( BBox.Max.GetZ() > Max.GetZ() )     Max.GetZ() = BBox.Max.GetZ();

    return( *this );
}

//==============================================================================

inline 
fbbox& fbbox::operator += ( const vector4& Point )
{
    FORCE_ALIGNED_16( this );

    if( Point.GetX() < Min.GetX() )     Min.GetX() = Point.GetX();
    if( Point.GetY() < Min.GetY() )     Min.GetY() = Point.GetY();
    if( Point.GetZ() < Min.GetZ() )     Min.GetZ() = Point.GetZ();
    if( Point.GetX() > Max.GetX() )     Max.GetX() = Point.GetX();
    if( Point.GetY() > Max.GetY() )     Max.GetY() = Point.GetY();
    if( Point.GetZ() > Max.GetZ() )     Max.GetZ() = Point.GetZ();

    return( *this );
}

//==============================================================================

inline 
fbbox& fbbox::operator += ( const vector3& Point )
{ 
    FORCE_ALIGNED_16( this );

    if( Point.GetX() < Min.GetX() )     Min.GetX() = Point.GetX();
    if( Point.GetY() < Min.GetY() )     Min.GetY() = Point.GetY();
    if( Point.GetZ() < Min.GetZ() )     Min.GetZ() = Point.GetZ();
    if( Point.GetX() > Max.GetX() )     Max.GetX() = Point.GetX();
    if( Point.GetY() > Max.GetY() )     Max.GetY() = Point.GetY();
    if( Point.GetZ() > Max.GetZ() )     Max.GetZ() = Point.GetZ();

    return( *this );
}

//==============================================================================

inline 
fbbox operator + ( const fbbox& BBox1, const fbbox& BBox2 )
{
    fbbox Result( BBox1 );
    return( Result += BBox2 );
}

//==============================================================================

inline 
fbbox operator + ( const fbbox& BBox, const vector4& Point )
{
    fbbox Result( BBox );
    return( Result += Point );
}

//==============================================================================

inline 
fbbox operator + ( const vector4& Point, const fbbox& BBox )
{
    fbbox Result( BBox );
    return( Result += Point );
}

//==============================================================================

inline
xbool fbbox::IntersectTriBBox( const vector4& P0,
                               const vector4& P1,
                               const vector4& P2 )  const
{
    fbbox TriBBox;

    //
    // HANDLE X
    //
    {
        TriBBox.Min.GetX() = P0.GetX();
        TriBBox.Max.GetX() = P0.GetX();

        if( P2.GetX() > P1.GetX() )
        {
            if( P2.GetX() > TriBBox.Max.GetX() ) TriBBox.Max.GetX() = P2.GetX();
            if( P1.GetX() < TriBBox.Min.GetX() ) TriBBox.Min.GetX() = P1.GetX();
        }
        else
        {
            if( P1.GetX() > TriBBox.Max.GetX() ) TriBBox.Max.GetX() = P1.GetX();
            if( P2.GetX() < TriBBox.Min.GetX() ) TriBBox.Min.GetX() = P2.GetX();
        }

        // X's are solved so compare to BBox.
        if( Min.GetX() > TriBBox.Max.GetX() ) return FALSE;
        if( Max.GetX() < TriBBox.Min.GetX() ) return FALSE;
    }

    //
    // HANDLE Z
    //
    {
        TriBBox.Min.GetZ() = P0.GetZ();
        TriBBox.Max.GetZ() = P0.GetZ();

        if( P2.GetZ() > P1.GetZ() )
        {
            if( P2.GetZ() > TriBBox.Max.GetZ() ) TriBBox.Max.GetZ() = P2.GetZ();
            if( P1.GetZ() < TriBBox.Min.GetZ() ) TriBBox.Min.GetZ() = P1.GetZ();
        }
        else
        {
            if( P1.GetZ() > TriBBox.Max.GetZ() ) TriBBox.Max.GetZ() = P1.GetZ();
            if( P2.GetZ() < TriBBox.Min.GetZ() ) TriBBox.Min.GetZ() = P2.GetZ();
        }

        // Y's are solved so compare to BBox.
        if( Min.GetZ() > TriBBox.Max.GetZ() ) return FALSE;
        if( Max.GetZ() < TriBBox.Min.GetZ() ) return FALSE;
    }

    //
    // HANDLE Y
    //
    {
        TriBBox.Min.GetY() = P0.GetY();
        TriBBox.Max.GetY() = P0.GetY();

        if( P2.GetY() > P1.GetY() )
        {
            if( P2.GetY() > TriBBox.Max.GetY() ) TriBBox.Max.GetY() = P2.GetY();
            if( P1.GetY() < TriBBox.Min.GetY() ) TriBBox.Min.GetY() = P1.GetY();
        }
        else
        {
            if( P1.GetY() > TriBBox.Max.GetY() ) TriBBox.Max.GetY() = P1.GetY();
            if( P2.GetY() < TriBBox.Min.GetY() ) TriBBox.Min.GetY() = P2.GetY();
        }

        // Z's are solved so compare to BBox.
        if( Min.GetY() > TriBBox.Max.GetY() ) return FALSE;
        if( Max.GetY() < TriBBox.Min.GetY() ) return FALSE;
    }

    return TRUE;
}

//==============================================================================

inline
xbool fbbox::ClipRay( const vector4& P0,
                      const vector4& P1,
                            f32&     T0,
                            f32&     T1)  const
{
    vector4 Dir = P1 - P0;
    f32  tx_min, tx_max;
    f32  ty_min, ty_max;
    f32  tz_min, tz_max;
    f32  t_min,  t_max;

    if( Dir.GetX() >= 0 )
    {  
        t_min = tx_min = (Min.GetX() - P0.GetX()) / Dir.GetX();
        t_max = tx_max = (Max.GetX() - P0.GetX()) / Dir.GetX();
    }
    else
    {  
        t_min = tx_min = (Max.GetX() - P0.GetX()) / Dir.GetX();
        t_max = tx_max = (Min.GetX() - P0.GetX()) / Dir.GetX();
    }

    if( Dir.GetY() >= 0)
    {  
        ty_min = (Min.GetY() - P0.GetY()) / Dir.GetY();
        ty_max = (Max.GetY() - P0.GetY()) / Dir.GetY();
    }
    else
    {  
        ty_min = (Max.GetY() - P0.GetY()) / Dir.GetY();
        ty_max = (Min.GetY() - P0.GetY()) / Dir.GetY();
    }

    if( t_min > ty_max || ty_min > t_max )
        return FALSE;

    if( t_min < ty_min )
        t_min = ty_min;

    if( t_max > ty_max )
        t_max = ty_max;

    if( Dir.GetZ() >= 0 )
    {  
        tz_min = (Min.GetZ() - P0.GetZ()) / Dir.GetZ();
        tz_max = (Max.GetZ() - P0.GetZ()) / Dir.GetZ();
    }
    else
    {  
        tz_min = (Max.GetZ() - P0.GetZ()) / Dir.GetZ();
        tz_max = (Min.GetZ() - P0.GetZ()) / Dir.GetZ();
    }

    if( t_min > tz_max || tz_min > t_max )
        return FALSE;

    if( t_min < tz_min )
        t_min = tz_min;

    if( t_max > tz_max )
        t_max = tz_max;

   if( !((t_min <= 1.0f) && (t_max >= 0.0f)) ) 
       return FALSE;

   // Set the initial entry and exit points of the ray
    T0 = t_min;
    T1 = t_max;

    return TRUE;
}

//==============================================================================

inline
void fbbox::SetupFastTests( void ) const
{
    FORCE_ALIGNED_16( this );
    
#ifdef TARGET_PS2
    asm( "
         lqc2   vf01, 0x00(%0)
         lqc2   vf02, 0x10(%0)
         " : : "r" (this) );
#endif
}

//==============================================================================

inline
xbool fbbox::FastIntersect( const fbbox& BBox ) const
{
/*
    //#### Disabled until vector3 optimizations are finished
#ifdef TARGET_PS2
    register s32 MinBMinusMaxAFlags;
    register s32 MinAMinusMaxBFlags;

    asm( "
         lqc2  vf03, 0x00(%2)
         lqc2  vf04, 0x10(%2)
         vsub  vf00, vf03, vf02
         vnop
         vnop
         vnop
         vnop
         cfc2  %0, vi17
         vsub  vf00, vf01, vf04
         vnop
         vnop
         vnop
         vnop
         cfc2  %1, vi17
         " : "=r" (MinBMinusMaxAFlags), "=r" (MinAMinusMaxBFlags) : "r" (&BBox) );

    if ( ((MinBMinusMaxAFlags&0xe0)!=0xe0) ||
         ((MinAMinusMaxBFlags&0xe0)!=0xe0) )
    {
        return FALSE;
    }

    return TRUE;
#else
*/
    return Intersect( BBox );
//#endif
}

//==============================================================================
#endif  // X_FBBOX_HPP
//==============================================================================

