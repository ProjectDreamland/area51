//==============================================================================
//
//  x_math_bb_inline.hpp
//
//==============================================================================

#ifndef X_MATH_BB_INLINE_HPP
#define X_MATH_BB_INLINE_HPP
#else
#error "File " __FILE__ " has been included twice!"
#endif

#ifndef X_DEBUG_HPP
#include "../x_debug.hpp"
#endif

//==============================================================================
//  FUNCTIONS
//==============================================================================

inline 
bbox::bbox( void )
{
    FORCE_ALIGNED_16( this );
}

//==============================================================================

inline
void bbox::Clear( void )
{
    FORCE_ALIGNED_16( this );
    Min(  F32_MAX,  F32_MAX,  F32_MAX );
    Max( -F32_MAX, -F32_MAX, -F32_MAX );
}

//==============================================================================

inline 
bbox::bbox( const vector3& P1 )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &P1 );
    Min = Max = P1;    
}

//==============================================================================

inline 
bbox::bbox( const vector3& P1, const vector3& P2 )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &P1 );
    FORCE_ALIGNED_16( &P2 );

    Min = Max = P1;
    Min.Min( P2 );
    Max.Max( P2 );
}

//==============================================================================

inline 
bbox::bbox( const vector3& P1, const vector3& P2, const vector3& P3 )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &P1 );
    FORCE_ALIGNED_16( &P2 );
    FORCE_ALIGNED_16( &P3 );

    Min = Max = P1;
    Min.Min( P2 );
    Max.Max( P2 );
    Min.Min( P3 );
    Max.Max( P3 );
}

//==============================================================================

inline 
bbox::bbox( const vector3& Center, f32 Radius )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &Center );
    Min = Center - vector3(Radius,Radius,Radius);
    Max = Center + vector3(Radius,Radius,Radius);
}

//==============================================================================

inline 
bbox::bbox( const vector3* pVerts, s32 NVerts )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( pVerts );
    ASSERT( pVerts );
    ASSERT( NVerts > 0 );

    Clear();

    // Start with first vertex as the bbox.
    Min = Max = *pVerts;
    pVerts++;
    NVerts--;

    // Add in all the others.
    while( NVerts > 0 )
    {
        Min.Min( *pVerts );
        Max.Max( *pVerts );

        pVerts++;
        NVerts--;
    }
}

//==============================================================================

inline 
void bbox::Set( const vector3& Center, f32 Radius )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &Center );

    Min = Center - vector3(Radius,Radius,Radius);
    Max = Center + vector3(Radius,Radius,Radius);
}

//==============================================================================

inline
void bbox::Set( const vector3& P1, const vector3& P2 )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &P1 );
    FORCE_ALIGNED_16( &P2 );
    
    Min = Max = P1;
    Min.Min( P2 );
    Max.Max( P2 );
}

//==============================================================================

inline
void bbox::operator () ( const vector3& P1, const vector3& P2 )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &P1 );
    FORCE_ALIGNED_16( &P2 );

    Min = Max = P1;
    Min.Min( P2 );
    Max.Max( P2 );
}

//==============================================================================

inline 
vector3 bbox::GetSize( void ) const
{
    FORCE_ALIGNED_16( this );

    return( Max - Min );
}

//==============================================================================

inline
f32 bbox::GetSurfaceArea( void ) const
{
    FORCE_ALIGNED_16( this );

    vector3 SZ = GetSize();
    return( 2.0f*((SZ.GetX()*SZ.GetY())+(SZ.GetY()*SZ.GetZ())+(SZ.GetZ()*SZ.GetX())) );
}

//==============================================================================

inline 
vector3 bbox::GetCenter( void ) const
{
    FORCE_ALIGNED_16( this );

    return( (Min + Max) * 0.5f );
}

//==============================================================================

inline 
f32 bbox::GetRadius( void ) const
{
    FORCE_ALIGNED_16( this );

    return( (Max-Min).Length() * 0.5f );
}

//==============================================================================

inline 
f32 bbox::GetRadiusSquared( void ) const
{
    FORCE_ALIGNED_16( this );

    vector3 R = (Max-Min) * 0.5f;
    return( R.Dot(R) );
}

//==============================================================================

inline 
void bbox::Transform( const matrix4& M )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &M );

#if USE_VU0
    vector3 Rows[3];
    M.GetRows( Rows[0], Rows[1], Rows[2] );
    u128 TRow0Min, TRow0Max, TRow1Min, TRow1Max, TRow2Min, TRow2Max;
    u128 Row0Min,  Row0Max,  Row1Min,  Row1Max,  Row2Min,  Row2Max;
    u128 NewMin, NewMax;
    u128 Trans = M.GetTranslation().GetU128();
    vector3 One( 1.0f, 1.0f, 1.0f );
    asm( "vmul  RMN0, ROW0, BMIN" : "=j RMN0" (TRow0Min) : "j ROW0" (Rows[0].GetU128()), "j BMIN" (Min.GetU128()) );
    asm( "vmul  RMN1, ROW1, BMIN" : "=j RMN1" (TRow1Min) : "j ROW1" (Rows[1].GetU128()), "j BMIN" (Min.GetU128()) );
    asm( "vmul  RMN2, ROW2, BMIN" : "=j RMN2" (TRow2Min) : "j ROW2" (Rows[2].GetU128()), "j BMIN" (Min.GetU128()) );
    asm( "vmul  RMX0, ROW0, BMAX" : "=j RMX0" (TRow0Max) : "j ROW0" (Rows[0].GetU128()), "j BMAX" (Max.GetU128()) );
    asm( "vmul  RMX1, ROW1, BMAX" : "=j RMX1" (TRow1Max) : "j ROW1" (Rows[1].GetU128()), "j BMAX" (Max.GetU128()) );
    asm( "vmul  RMX2, ROW2, BMAX" : "=j RMX2" (TRow2Max) : "j ROW2" (Rows[2].GetU128()), "j BMAX" (Max.GetU128()) );
    asm( "vmini RMN0, RMIN, RMAX" : "=j RMN0" (Row0Min)  : "j RMIN" (TRow0Min), "j RMAX" (TRow0Max) );
    asm( "vmini RMN1, RMIN, RMAX" : "=j RMN1" (Row1Min)  : "j RMIN" (TRow1Min), "j RMAX" (TRow1Max) );
    asm( "vmini RMN2, RMIN, RMAX" : "=j RMN2" (Row2Min)  : "j RMIN" (TRow2Min), "j RMAX" (TRow2Max) );
    asm( "vmax  RMX0, RMIN, RMAX" : "=j RMX0" (Row0Max)  : "j RMIN" (TRow0Min), "j RMAX" (TRow0Max) );
    asm( "vmax  RMX1, RMIN, RMAX" : "=j RMX1" (Row1Max)  : "j RMIN" (TRow1Min), "j RMAX" (TRow1Max) );
    asm( "vmax  RMX2, RMIN, RMAX" : "=j RMX2" (Row2Max)  : "j RMIN" (TRow2Min), "j RMAX" (TRow2Max) );
    asm( "vmulax.x  acc,  SONE, RMN0x
          vmadday.x acc,  SONE, RMN0y
          vmaddz.x  BMIN, SONE, RMN0z
          vmulax.y  acc,  SONE, RMN1x
          vmadday.y acc,  SONE, RMN1y
          vmaddz.y  BMIN, SONE, RMN1z
          vmulax.z  acc,  SONE, RMN2x
          vmadday.z acc,  SONE, RMN2y
          vmaddz.z  BMIN, SONE, RMN2z" : "=j BMIN" (NewMin) : "j SONE" (One.GetU128()), "j RMN0" (Row0Min), "j RMN1" (Row1Min), "j RMN2" (Row2Min) );
    asm( "vmulax.x  acc,  SONE, RMX0x
          vmadday.x acc,  SONE, RMX0y
          vmaddz.x  BMAX, SONE, RMX0z
          vmulax.y  acc,  SONE, RMX1x
          vmadday.y acc,  SONE, RMX1y
          vmaddz.y  BMAX, SONE, RMX1z
          vmulax.z  acc,  SONE, RMX2x
          vmadday.z acc,  SONE, RMX2y
          vmaddz.z  BMAX, SONE, RMX2z" : "=j BMAX" (NewMax) : "j SONE" (One.GetU128()), "j RMX0" (Row0Max), "j RMX1" (Row1Max), "j RMX2" (Row2Max) );
    asm( "vadd.xyz  BMNO, BMNI, TRNS" : "=j BMNO" (NewMin) : "j BMNI" (NewMin), "j TRNS" (Trans) );
    asm( "vadd.xyz  BMXO, BMXI, TRNS" : "=j BMXO" (NewMax) : "j BMXI" (NewMax), "j TRNS" (Trans) );

    Min.Set( NewMin );
    Max.Set( NewMax );
#else
    vector3 AMin, AMax;
    f32     a, b;
    s32     i, j;

    // Copy box A into min and max array.
    AMin = Min;
    AMax = Max;

    // Begin at T.
    Min = Max = M.GetTranslation();

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
#endif
}

//==============================================================================

inline 
void bbox::Translate( const vector3& T )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &T );

    Min += T;
    Max += T;
}

//==============================================================================

inline 
xbool bbox::Intersect( const vector3& Point ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &Point );

    return( (Point.GetX() <= Max.GetX()) && 
            (Point.GetY() <= Max.GetY()) && 
            (Point.GetZ() <= Max.GetZ()) && 
            (Point.GetX() >= Min.GetX()) && 
            (Point.GetY() >= Min.GetY()) && 
            (Point.GetZ() >= Min.GetZ()) );
}

//==============================================================================

inline 
xbool bbox::Intersect( const bbox& BBox ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &BBox );

#if USE_VU0
    register s32 BMinMinusAMaxFlags;
    register s32 AMinMinusBMaxFlags;
    asm __volatile__ ( "vsub.xyz vf00, BMINxyz, AMAXxyz" : : "j BMIN" (BBox.Min.GetU128()), "j AMAX" (Max.GetU128()) );
    asm __volatile__ ( "vnop" );
    asm __volatile__ ( "vnop" );
    asm __volatile__ ( "vnop" );
    asm __volatile__ ( "vnop" );
    asm __volatile__ ( "cfc2 FLAG, vi17" : "=r FLAG" (BMinMinusAMaxFlags) );
    asm __volatile__ ( "vsub.xyz vf00, AMINxyz, BMAXxyz" : : "j AMIN" (Min.GetU128()), "j BMAX" (BBox.Max.GetU128()) );
    asm __volatile__ ( "vnop" );
    asm __volatile__ ( "vnop" );
    asm __volatile__ ( "vnop" );
    asm __volatile__ ( "vnop" );
    asm __volatile__ ( "cfc2 FLAG, vi17" : "=r FLAG" (AMinMinusBMaxFlags) );

    if ( ((BMinMinusAMaxFlags&0xe0)!=0xe0) ||
         ((AMinMinusBMaxFlags&0xe0)!=0xe0) )
    {
        return FALSE;
    }

    return TRUE;
#else
    if( BBox.Min.GetX() > Max.GetX() ) return FALSE;
    if( BBox.Max.GetX() < Min.GetX() ) return FALSE;
    if( BBox.Min.GetZ() > Max.GetZ() ) return FALSE;
    if( BBox.Max.GetZ() < Min.GetZ() ) return FALSE;
    if( BBox.Min.GetY() > Max.GetY() ) return FALSE;
    if( BBox.Max.GetY() < Min.GetY() ) return FALSE;
    return TRUE;
#endif
}

//==============================================================================

inline 
xbool bbox::Intersect( const plane& Plane ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &Plane );

    vector3 PMin, PMax;
    f32     DMax, DMin;

    if( Plane.Normal.GetX() > 0 )   { PMax.GetX() = Max.GetX();   PMin.GetX() = Min.GetX(); }
    else                            { PMax.GetX() = Min.GetX();   PMin.GetX() = Max.GetX(); }

    if( Plane.Normal.GetY() > 0 )   { PMax.GetY() = Max.GetY();   PMin.GetY() = Min.GetY(); }
    else                            { PMax.GetY() = Min.GetY();   PMin.GetY() = Max.GetY(); }

    if( Plane.Normal.GetZ() > 0 )   { PMax.GetZ() = Max.GetZ();   PMin.GetZ() = Min.GetZ(); }
    else                            { PMax.GetZ() = Min.GetZ();   PMin.GetZ() = Max.GetZ(); }

    DMax = Plane.Distance( PMax );
    DMin = Plane.Distance( PMin );

    return( (DMax > 0.0f) && (DMin < 0.0f ) );
}

//==============================================================================

inline 
xbool bbox::Intersect( f32& t, const vector3& P0, const vector3& P1 ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &P0 );
    FORCE_ALIGNED_16( &P1 );

    f32     PlaneD   [3];
    xbool   PlaneUsed[3] = { TRUE, TRUE, TRUE };
    f32     T        [3] = { -1, -1, -1 };
    vector3 Direction    = P1 - P0;
    s32     MaxPlane;
    s32     i;
    f32     Component;

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
xbool bbox::Intersect( const vector3& Center, f32 Radius ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &Center );

#if USE_VU0
    // TMP0 = MAX( Zero, Min - Center );
    // TMP1 = MAX( Zero, Center - Max );
    // TMP2 = TMP0 + TMP1
    // dmin = TMP2.Dot(TMP2)

    f32  dmin;
    u128 TMP0, TMP1, TMP2;
    asm( "vsub TMP0, BMIN, CNTR" : "=j TMP0" (TMP0) : "j BMIN" (Min.GetU128()),    "j CNTR" (Center.GetU128()) );
    asm( "vsub TMP1, CNTR, BMAX" : "=j TMP1" (TMP1) : "j CNTR" (Center.GetU128()), "j BMAX" (Max.GetU128()) );
    asm( "vmax TMP0, TMP0, vf00" : "+j TMP0" (TMP0) );
    asm( "vmax TMP1, TMP1, vf00" : "+j TMP1" (TMP1) );
    asm( "vadd TMP2, TMP0, TMP1" : "=j TMP2" (TMP2) : "j TMP0" (TMP0), "j TMP1" (TMP1) );
    asm( "vmul TMP2, TMP2, TMP2" : "+j TMP2" (TMP2) );
    asm( "vaddy.x TMP2, TMP2y
          vaddz.x TMP2, TMP2z"   : "+j TMP2" (TMP2) );
    asm( "qmfc2 DMIN, TMP2" : "=r DMIN" (dmin) : "j TMP2" (TMP2) );

    if( dmin <= Radius*Radius ) 
        return TRUE;

    return FALSE;
#else
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
#endif
}

//==============================================================================

inline
xbool bbox::Contains( const bbox&    BBox  ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &BBox );

    if( BBox.Min.GetX() >= Min.GetX() &&
        BBox.Max.GetX() <= Max.GetX() &&
        BBox.Min.GetZ() >= Min.GetZ() &&
        BBox.Max.GetZ() <= Max.GetZ() &&
        BBox.Min.GetY() >= Min.GetY() &&
        BBox.Max.GetY() <= Max.GetY() )
        return TRUE;

    return FALSE;
}

//==============================================================================

inline 
bbox& bbox::operator += ( const bbox& BBox )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &BBox );

    Min.Min( BBox.Min );
    Max.Max( BBox.Max );

    return( *this );
}

//==============================================================================

inline 
bbox& bbox::operator += ( const vector3& Point )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &Point );

    Min.Min( Point );
    Max.Max( Point );

    return( *this );
}

//==============================================================================

inline 
bbox& bbox::AddVerts( const vector3* pVerts, s32 NVerts )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( pVerts );

    ASSERT( pVerts );
    ASSERT( NVerts > 0 );

    while( NVerts > 0 )
    {
        Min.Min( *pVerts );
        Max.Max( *pVerts );

        pVerts++;
        NVerts--;
    }

    return( *this );
}

//==============================================================================

inline 
bbox operator + ( const bbox& BBox1, const bbox& BBox2 )
{
    FORCE_ALIGNED_16( &BBox1 );
    FORCE_ALIGNED_16( &BBox2 );

    bbox Result( BBox1 );
    return( Result += BBox2 );
}

//==============================================================================

inline 
bbox operator + ( const bbox& BBox, const vector3& Point )
{
    FORCE_ALIGNED_16( &BBox );
    FORCE_ALIGNED_16( &Point );

    bbox Result( BBox );
    return( Result += Point );
}

//==============================================================================

inline 
bbox operator + ( const vector3& Point, const bbox& BBox )
{
    FORCE_ALIGNED_16( &Point );
    FORCE_ALIGNED_16( &BBox );

    bbox Result( BBox );
    return( Result += Point );
}

//==============================================================================
 
inline
f32 bbox::Difference( const bbox& B ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &B );

    return Min.Difference(B.Min) + Max.Difference(B.Max);
}

//==============================================================================

inline
xbool bbox::InRange( f32 aMin, f32 aMax ) const
{
    FORCE_ALIGNED_16( this );

    return( (Min.GetX()>=aMin) && (Min.GetX()<=aMax) &&
            (Min.GetY()>=aMin) && (Min.GetY()<=aMax) &&
            (Min.GetZ()>=aMin) && (Min.GetZ()<=aMax) &&
            (Max.GetX()>=aMin) && (Max.GetX()<=aMax) &&
            (Max.GetY()>=aMin) && (Max.GetY()<=aMax) &&
            (Max.GetZ()>=aMin) && (Max.GetZ()<=aMax) );
}

//==============================================================================

inline
xbool bbox::IsValid( void ) const
{
    FORCE_ALIGNED_16( this );

    return( Min.IsValid() && Max.IsValid() );
}

//==============================================================================

inline
void bbox::Inflate( f32 x, f32 y, f32 z )
{
    FORCE_ALIGNED_16( this );

    Min -= vector3( x, y, z );
    Max += vector3( x, y, z );
}

//==============================================================================

inline
void bbox::Deflate( f32 x, f32 y, f32 z )
{
    FORCE_ALIGNED_16( this );

    Min += vector3( x, y, z );
    Max -= vector3( x, y, z );
}

//==============================================================================

inline
void bbox::Translate( f32 x, f32 y, f32 z )
{
    FORCE_ALIGNED_16( this );

    Min += vector3( x, y, z );
    Max += vector3( x, y, z );
}

//==============================================================================

inline
xbool bbox::IntersectTriBBox( const vector3& P0,
                              const vector3& P1,
                              const vector3& P2 )  const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &P0 );
    FORCE_ALIGNED_16( &P1 );
    FORCE_ALIGNED_16( &P2 );

    // Compute triangle bbox
    bbox TriBBox( P0, P1, P2 );
    
    // Call bbox intersect function
    return Intersect( TriBBox );
}

//==============================================================================
