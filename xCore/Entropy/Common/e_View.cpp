//==============================================================================
//  
//  e_View.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "..\e_View.hpp"

//==============================================================================
//  DEFINES   
//==============================================================================

#define DIRTY_W2V               ( 1 <<  0 )
#define DIRTY_V2W               ( 1 <<  1 )
                                
#define DIRTY_V2C               ( 1 <<  2 )
#define DIRTY_C2S               ( 1 <<  3 )
#define DIRTY_V2S               ( 1 <<  4 )
#define DIRTY_W2C               ( 1 <<  5 )
#define DIRTY_W2S               ( 1 <<  6 )
#define DIRTY_YFOV              ( 1 <<  7 )

#define DIRTY_PLANES            ( 1 <<  8 )
#define DIRTY_EDGES             ( 1 <<  9 )
#define DIRTY_PROJECTION        ( 1 << 10 )
#define DIRTY_SCREENDIST        ( 1 << 11 )

#define DIRTY_ALL       0xFFFFFFFF
#define DIRTY_MATRICES  ( DIRTY_W2V |   \
                          DIRTY_V2W |   \
                          DIRTY_V2C |   \
                          DIRTY_C2S |   \
                          DIRTY_V2S |   \
                          DIRTY_W2C |   \
                          DIRTY_W2S )

//==============================================================================
//  FUNCTIONS
//==============================================================================

view::view( void )
{
    m_WorldPos( 0.0f, 0.0f, 0.0f );
    m_WorldOrient.Identity();
    m_ViewportX0 =  50;             // Default viewport is 640x480, but
    m_ViewportY0 =  50;             // pulled in 50 pixels on all sides.
    m_ViewportX1 = 589;             //   639 - 50 = 589
    m_ViewportY1 = 429;             //   479 - 50 = 249
    m_XFOV       = R_60;   
    m_PixelScale = DEFAULT_PIXEL_SCALE;         
    m_ZNear      =    0.1f;
    m_ZFar       = 1000.0f;

#if !defined( CONFIG_RETAIL )
    m_SubShotX   = 0 ;                // Current X shot
    m_SubShotY   = 0 ;                // Current Y shot
    m_ShotSize   = 1 ;                // Total shots is m_ShotSize*m_ShotSize
#endif // !defined( CONFIG_RETAIL )

    m_Dirty      = DIRTY_ALL;

    m_nScissors  = 0;
    m_iScissor   = 0;
}

//==============================================================================

view::view( const view& View )
{
    m_WorldPos    = View.m_WorldPos;
    m_WorldOrient = View.m_WorldOrient;
    m_ViewportX0  = View.m_ViewportX0;
    m_ViewportY0  = View.m_ViewportY0;
    m_ViewportX1  = View.m_ViewportX1;
    m_ViewportY1  = View.m_ViewportY1;
    m_XFOV        = View.m_XFOV;
    m_PixelScale  = View.m_PixelScale;         
    m_ZNear       = View.m_ZNear;
    m_ZFar        = View.m_ZFar;

#if !defined( CONFIG_RETAIL )
    m_SubShotX    = View.m_SubShotX ;
    m_SubShotY    = View.m_SubShotY ;
    m_ShotSize    = View.m_ShotSize ;
#endif // !defined( CONFIG_RETAIL )

    m_Dirty       = DIRTY_ALL;
}

//==============================================================================

view::~view( void )
{
}

//==============================================================================

void view::SetViewport( s32 X0, s32 Y0, s32 X1, s32 Y1 )
{
    m_ViewportX0 = X0;
    m_ViewportY0 = Y0;
    m_ViewportX1 = X1;
    m_ViewportY1 = Y1;

    m_Dirty |= DIRTY_YFOV;
    m_Dirty |= DIRTY_PLANES;
    m_Dirty |= DIRTY_EDGES;
    m_Dirty |= DIRTY_PROJECTION;
    m_Dirty |= DIRTY_SCREENDIST;
}

//==============================================================================

void view::SetXFOV( radian XFOV )
{
    m_XFOV = XFOV;

    m_Dirty |= DIRTY_MATRICES;
    m_Dirty |= DIRTY_YFOV;
    m_Dirty |= DIRTY_PLANES;
    m_Dirty |= DIRTY_EDGES;
    m_Dirty |= DIRTY_PROJECTION;
    m_Dirty |= DIRTY_SCREENDIST;
}

//==============================================================================

void view::SetYFOV( radian YFOV )
{
    // We need a distance where the viewport pixel units are the same as our
    // world units.  Since we know the size of the viewport vertically (in 
    // pixels) and we were given the vertical angle, we can find this distance.  
    // Note that we want only half of the viewport height and half of the angle 
    // to give us a right triangle for the trig.

    f32 HalfVPHeight = (m_ViewportY1 - m_ViewportY0 + 1) * 0.5f;
    f32 HalfVPWidth  = (m_ViewportX1 - m_ViewportX0 + 1) * 0.5f / m_PixelScale;

    m_ScaledScreenDistY = HalfVPHeight / x_tan( YFOV * 0.5f );

    // Given this distance, and knowing the size of the viewport laterally 
    // (again, in pixels), we can find the laterial angle.  Note, the trig uses
    // with half the viewport size to find half the anlge.

    m_XFOV = x_atan( HalfVPWidth / m_ScaledScreenDistY ) * 2.0f;
    m_YFOV = YFOV;

    m_Dirty &= ~DIRTY_YFOV;
    m_Dirty |=  DIRTY_SCREENDIST;
    m_Dirty |=  DIRTY_PLANES;
    m_Dirty |=  DIRTY_EDGES;
    m_Dirty |=  DIRTY_MATRICES;
    m_Dirty |=  DIRTY_PROJECTION;
}

//==============================================================================

void view::SetPixelScale( f32 PixelScale )
{
    m_PixelScale = PixelScale;
    
    m_Dirty |= DIRTY_MATRICES;
    m_Dirty |= DIRTY_YFOV;
    m_Dirty |= DIRTY_PLANES;
    m_Dirty |= DIRTY_EDGES;
    m_Dirty |= DIRTY_PROJECTION;
    m_Dirty |= DIRTY_SCREENDIST;
}

//==============================================================================

void view::SetZLimits( f32 ZNear, f32 ZFar )
{
    m_ZNear = ZNear;
    m_ZFar  = ZFar;

    m_Dirty = DIRTY_ALL;
}

//==============================================================================

void view::SetPosition( const vector3& Position )
{
    m_WorldPos = Position;
    m_Dirty = DIRTY_ALL;
}

//==============================================================================

void view::SetRotation( const radian3& Rotation )
{
    m_WorldOrient.Setup( Rotation );
    m_Dirty = DIRTY_ALL;
}

//==============================================================================

void view::SetRotation( const quaternion& Quat )
{
    m_WorldOrient.Setup( Quat );
    m_Dirty = DIRTY_ALL;
}

//==============================================================================

void view::SetV2W( const matrix4& V2W )
{
    m_WorldOrient = V2W;
    m_WorldOrient.ClearTranslation();

    m_WorldPos = V2W.GetTranslation();

    m_Dirty = DIRTY_ALL;
}


//==============================================================================

void view::Translate( const vector3& Translation, system System )
{   
    switch( System )
    {
        case WORLD: m_WorldPos += Translation;                      break;
        case VIEW:  m_WorldPos += (m_WorldOrient * Translation);    break;
        default:    ASSERT( FALSE );
    }

    m_Dirty = DIRTY_ALL;
}

//==============================================================================

void view::RotateX( radian Angle, system System )
{
    switch( System )
    {
        case WORLD: m_WorldOrient.RotateX( Angle );         break;                        
        case VIEW:  m_WorldOrient.PreRotateX( Angle );      break;
        default:    ASSERT( FALSE );
    }

    m_Dirty = DIRTY_ALL;
}

//==============================================================================

void view::RotateY( radian Angle, system System )
{
    switch( System )
    {
        case WORLD: m_WorldOrient.RotateY( Angle );         break;                        
        case VIEW:  m_WorldOrient.PreRotateY( Angle );      break;
        default:    ASSERT( FALSE );
    }

    m_Dirty = DIRTY_ALL;
}

//==============================================================================

void view::RotateZ( radian Angle, system System )
{
    switch( System )
    {
        case WORLD: m_WorldOrient.RotateZ( Angle );         break;                        
        case VIEW:  m_WorldOrient.PreRotateZ( Angle );      break;                        
        default:    ASSERT( FALSE );
    }

    m_Dirty = DIRTY_ALL;
}

//==============================================================================

void view::GetViewport( s32& X0, s32& Y0, s32& X1, s32& Y1 ) const
{
    X0 = m_ViewportX0;
    Y0 = m_ViewportY0;
    X1 = m_ViewportX1;
    Y1 = m_ViewportY1;
}

//==============================================================================

void view::GetViewport( rect& Rect ) const
{
    Rect.Min.X = (f32)m_ViewportX0;
    Rect.Min.Y = (f32)m_ViewportY0;
    Rect.Max.X = (f32)m_ViewportX1;
    Rect.Max.Y = (f32)m_ViewportY1;
}


//==============================================================================

void view::GetPixel( f32 ParamX, f32 ParamY, s32& X, s32& Y ) const
{
    X = (s32)(m_ViewportX0 + ParamX*(m_ViewportX1-m_ViewportX0));
    Y = (s32)(m_ViewportY0 + ParamY*(m_ViewportY1-m_ViewportY0));

#if !defined( CONFIG_RETAIL )
    // Must be at least one screen!
    ASSERT(m_ShotSize >= 1) ;

    // Take big screen shot into account?
    if (m_ShotSize > 1)
    {
        // Scale onto big virtual screen
        X *= m_ShotSize ;
        Y *= m_ShotSize ;

        // Make relative to top corner of regular screen
        X -= m_ViewportX0 ;
        Y -= m_ViewportY1 ;
    }
#endif // !defined( CONFIG_RETAIL )
}

//==============================================================================

void view::GetZLimits( f32& ZNear, f32& ZFar ) const
{
    ZNear = m_ZNear;
    ZFar  = m_ZFar;
}

//==============================================================================

void view::GetPitchYaw( radian& Pitch, radian& Yaw ) const
{
    // We want the view's Z axis (line of sight) in World space.  We can easily
    // pull that unit vector from the internal orientation matrix which converts
    // View oriented space to World oriented space.

    vector3 LOS( m_WorldOrient(2,0), 
                 m_WorldOrient(2,1), 
                 m_WorldOrient(2,2) );

    LOS.GetPitchYaw( Pitch, Yaw );
}

//==============================================================================

radian view::GetXFOV( void ) const
{
    return( m_XFOV );
}

//==============================================================================

radian view::GetYFOV( void ) const
{
    UpdateYFOV();
    return( m_YFOV );
}

//==============================================================================

f32 view::GetPixelScale( void ) const
{
    return m_PixelScale;
}

//==============================================================================

vector3 view::GetPosition( void ) const
{
    return( m_WorldPos );
}

//==============================================================================

const matrix4& view::GetW2V( void ) const
{
    UpdateW2V();
    return( m_W2V );
}

//==============================================================================

const matrix4& view::GetV2W( void ) const
{
    UpdateV2W();
    return( m_V2W );
}

//==============================================================================

const matrix4& view::GetV2C( void ) const
{
    UpdateV2C();
    return( m_V2C );
}

//==============================================================================

const matrix4& view::GetC2S( void ) const
{
    UpdateC2S();
    return( m_C2S );
}

//==============================================================================

const matrix4& view::GetV2S( void ) const
{
    UpdateV2S();
    return( m_V2S );
}

//==============================================================================

const matrix4& view::GetW2C( void ) const
{
    UpdateW2C();
    return( m_W2C );
}

//==============================================================================

void view::GetV2C( s32 ClipSize, matrix4& V2C ) const
{
#if !defined( CONFIG_RETAIL )
    if ( m_ShotSize == 1 )
#endif // !defined( CONFIG_RETAIL )
    {
        f32 Ratio = (f32)(m_ViewportY1 - m_ViewportY0) / (f32)(m_ViewportX1 - m_ViewportX0);
        f32 ClipX = (f32)ClipSize;
        f32 ClipY = (f32)ClipSize * Ratio;

        UpdateScreenDist();
        f32 DX = m_ScaledScreenDistX;
        f32 DY = m_ScaledScreenDistY;
        f32 X = x_atan( ClipX / DX );
        f32 Y = x_atan( ClipY / DY );
        f32 W = (f32)(1.0f / x_tan( X ));
        f32 H = (f32)(1.0f / x_tan( Y ));

        V2C.Zero();
    
        V2C(0,0) = -W;
        V2C(1,1) =  H;
        V2C(2,2) =         (m_ZNear + m_ZFar) / (m_ZFar - m_ZNear);
        V2C(3,2) = (-2.0f * m_ZNear * m_ZFar) / (m_ZFar - m_ZNear);
        V2C(2,3) = 1.0f;
    }
#if !defined( CONFIG_RETAIL )
    else
    {
        // this will cause a lot more polys to get clipped, but only happens
        // during a screenshot, so oh well...
        
        // Get full furstrum
        f32 W = m_ZNear * (f32)x_tan(m_XFOV*0.5f) ;
        f32 H = m_ZNear * (f32)x_tan(m_YFOV*0.5f) ;

        // Setup sub frustrum L + R
        f32 R = (-W) + (((f32)m_SubShotX     / (f32)m_ShotSize) * (W - (-W))) ;
        f32 L = (-W) + (((f32)(m_SubShotX+1) / (f32)m_ShotSize) * (W - (-W))) ;

        // Setup sub frustrum T + B
        f32 T = (-H) + (((f32)m_SubShotY     / (f32)m_ShotSize) * (H - (-H))) ;
        f32 B = (-H) + (((f32)(m_SubShotY+1) / (f32)m_ShotSize) * (H - (-H))) ;

        // Finally, setup the view to clip matrix
        m_V2C(0,0) = (2 * m_ZNear) / (R - L) ;
        m_V2C(1,1) = (2 * m_ZNear) / (B - T) ;
        m_V2C(2,0) = (R + L) / (R - L) ;
        m_V2C(2,1) = (B + T) / (B - T) ;
        m_V2C(2,2) = (m_ZFar+m_ZNear)/(m_ZFar-m_ZNear);
        m_V2C(2,3) = 1.0f;
        m_V2C(3,2) = (-2*m_ZNear*m_ZFar)/(m_ZFar-m_ZNear);
    }
#endif // !defined( CONFIG_RETAIL )
}

//==============================================================================

void view::GetW2C( s32 ClipSize, matrix4& W2C ) const
{
    UpdateW2C();

    matrix4 V2C;
    GetV2C( ClipSize, V2C );        
    
    W2C = V2C * m_W2V;
}

//==============================================================================

void view::GetC2W( s32 ClipSize, matrix4& C2W ) const
{
    matrix4 V2C;
    
    GetV2C( ClipSize, V2C );
    V2C.Invert();
        
    C2W = GetV2W() * V2C;
}

//==============================================================================

void view::GetC2W( matrix4& C2W ) const
{
    matrix4 C2V( GetV2C() );
    C2V.Invert();
        
    C2W = GetV2W() * C2V;
}

//==============================================================================

void view::GetC2S( s32 ClipSize, matrix4& C2S ) const
{
    f32 Ratio = (f32)(m_ViewportY1 - m_ViewportY0) / (f32)(m_ViewportX1 - m_ViewportX0);
    f32 ClipX = (f32)ClipSize;
    f32 ClipY = (f32)ClipSize * Ratio;
    
    f32 W = ClipX;
    f32 H = ClipY;

    f32 SW     = (m_ViewportX1 - m_ViewportX0) * 0.5f;
    f32 ZScale = (f32)((s32)1<<19);
    
    C2S.Zero();
    C2S(0,0) =  W;
    C2S(1,1) = -H;
    C2S(2,2) = -ZScale / 2.0f;
    C2S(3,3) = 1.0f;
    C2S(3,0) = W + m_ViewportX0 + (2048.0f - ClipX);
    C2S(3,1) = H + m_ViewportY0 + (2048.0f - ClipY) - ((1.0f - Ratio) * SW);
    C2S(3,2) = ZScale / 2.0f;
}

//==============================================================================

const matrix4& view::GetW2S( void ) const
{
    UpdateW2S();
    return( m_W2S );
}     

//==============================================================================

vector3 view::GetViewX( void ) const
{
    return( vector3( m_WorldOrient(0,0),  
                     m_WorldOrient(0,1),  
                     m_WorldOrient(0,2) ) );
}

//==============================================================================

vector3 view::GetViewY( void ) const
{
    return( vector3( m_WorldOrient(1,0),  
                     m_WorldOrient(1,1),  
                     m_WorldOrient(1,2) ) );
}

//==============================================================================

vector3 view::GetViewZ( void ) const
{
    return( vector3( m_WorldOrient(2,0),  
                     m_WorldOrient(2,1),  
                     m_WorldOrient(2,2) ) );
}

//==============================================================================

void view::LookAtPoint( const vector3& FromPoint, 
                        const vector3& ToPoint,
                        system System )
{
    vector3 WorldFrom;

    // We need the "Target" point to be in World space.

    switch( System )
    {
        case WORLD: WorldFrom = FromPoint; break;
        case VIEW:  WorldFrom = ConvertV2W( FromPoint ); break;
        default:    ASSERT( FALSE );
    }

    // Now, look at the Target.

    SetPosition( WorldFrom );
    LookAtPoint( ToPoint, System );

    m_Dirty = DIRTY_ALL;
}

//==============================================================================

void view::LookAtPoint( const vector3& Point, system System )
{
    vector3 Target;

    // We need the "Target" point to be in World space.

    switch( System )
    {
        case WORLD: Target = Point; break;
        case VIEW:  Target = ConvertV2W( Point ); break;
        default:    ASSERT( FALSE );
    }

    // Now, look at the Target.

    Target -= m_WorldPos;

    m_WorldOrient.Identity();
    m_WorldOrient.RotateX( Target.GetPitch() );
    m_WorldOrient.RotateY( Target.GetYaw()   );

    m_Dirty = DIRTY_ALL;
}

//==============================================================================

void view::OrbitPoint( const vector3& Point, 
                             f32      Distance,
                             radian   Pitch,
                             radian   Yaw )
{
    m_WorldPos( 0, 0, Distance );
    m_WorldPos.RotateX( Pitch );
    m_WorldPos.RotateY( Yaw   );
    m_WorldPos += Point;
    LookAtPoint( Point );
}

//==============================================================================

vector3 view::ConvertW2V( const vector3& Point ) const
{
    UpdateW2V();
    return( m_W2V * Point );
}

//==============================================================================

vector3 view::ConvertV2W( const vector3& Point ) const
{
    UpdateV2W();
    return( m_V2W * Point );
}

//==============================================================================

const plane* view::GetViewPlanes( system System ) const
{
    if( m_Dirty & DIRTY_PLANES )
        UpdatePlanes();
    return( (System==WORLD) ? (m_WorldSpacePlane) : (m_ViewSpacePlane) );
}

//==============================================================================

const s32* view::GetViewPlaneMinBBoxIndices( system System ) const
{
    if( m_Dirty & DIRTY_PLANES )
        UpdatePlanes();
    return( (System==WORLD) ? (m_WorldPlaneMinIndex) : (m_ViewPlaneMinIndex) );
}

//==============================================================================

const s32* view::GetViewPlaneMaxBBoxIndices( system System ) const
{
    if( m_Dirty & DIRTY_PLANES )
        UpdatePlanes();
    return( (System==WORLD) ? (m_WorldPlaneMaxIndex) : (m_ViewPlaneMaxIndex) );
}

//==============================================================================

void view::GetMinMaxZ( const bbox& BBox, f32& MinZ, f32& MaxZ ) const
{
    f32* pF = ((f32*)&BBox);

    // Be sure planes have been constructed.
    if( m_Dirty & DIRTY_PLANES )
        UpdatePlanes();

    // Compute min and max dist along normal
    MinZ = m_ZPlane.Normal.GetX() * pF[m_ZPlaneMinI[0]] +
           m_ZPlane.Normal.GetY() * pF[m_ZPlaneMinI[1]] +
           m_ZPlane.Normal.GetZ() * pF[m_ZPlaneMinI[2]] +
           m_ZPlane.D;

    MaxZ = m_ZPlane.Normal.GetX() * pF[m_ZPlaneMaxI[0]] +
           m_ZPlane.Normal.GetY() * pF[m_ZPlaneMaxI[1]] +
           m_ZPlane.Normal.GetZ() * pF[m_ZPlaneMaxI[2]] +
           m_ZPlane.D;
}

//==============================================================================

void view::GetViewPlanes( plane& Top,
                          plane& Bottom,
                          plane& Left,
                          plane& Right,
                          system System) const
{
    plane* pPlane;

    UpdatePlanes();

    pPlane = (System==WORLD) ? (m_WorldSpacePlane) : (m_ViewSpacePlane);

    Left   = pPlane[0];
    Right  = pPlane[1];
    Bottom = pPlane[2];
    Top    = pPlane[3];
}

//==============================================================================

void view::GetViewPlanes( plane& Top,
                          plane& Bottom,
                          plane& Left,
                          plane& Right,
                          plane& Near,
                          plane& Far,
                          system System ) const
{
    plane* pPlane;

    UpdatePlanes();

    pPlane = (System==WORLD) ? (m_WorldSpacePlane) : (m_ViewSpacePlane);

    Left   = pPlane[0];
    Right  = pPlane[1];
    Bottom = pPlane[2];
    Top    = pPlane[3];
    Near   = pPlane[4];
    Far    = pPlane[5];
}

//==============================================================================

void view::GetViewPlanes( f32 X0, f32 Y0, f32 X1, f32 Y1,
                          plane& Top,
                          plane& Bottom,
                          plane& Left,
                          plane& Right,
                          plane& Near,
                          plane& Far,
                          system System ) const
{
    // Imagine sitting at the origin and looking down Z+.  Build the points on 
    // the frustum and transform into correct system.  Then build the planes 
    // the points.

    // Compute distance to screen in camera space.
    UpdateScreenDist();

    // Get sub shot frustrum
    f32 L,R,T,B ;
    
#if !defined( CONFIG_RETAIL )
    // Must be at least one screen!
    ASSERT(m_ShotSize >= 1) ;

    // Simple case
    if (m_ShotSize == 1)
#endif // !defined( CONFIG_RETAIL )
    {
        L  = X0 ;
        T  = Y0 ;
        R  = X1 ;
        B  = Y1 ;
    }
#if !defined( CONFIG_RETAIL )
    else
    {
        // SB.
        // If during a multi-part screen shot we must the normal viewport as inputs
        // for correct clipping since the view volume is not symetrical anymore!
        // I'm putting this here and it should be okay since this function is only ever
        // getting called to return the PS2 clipping planes (which are bigger than the
        // regular view due to the scissoring)

        // Get full frustrum
        f32 W = (m_ViewportX1 - m_ViewportX0 + 1)*0.5f;
        f32 H = (m_ViewportY1 - m_ViewportY0 + 1)*0.5f;
        X0 = -W ;
        X1 = W ;
        Y0 = -H ;
        Y1 = H ;

        // Setup sub frustrum L + R
        L = X0 + (((f32)m_SubShotX     / (f32)m_ShotSize) * (X1 - X0)) ;
        R = X0 + (((f32)(m_SubShotX+1) / (f32)m_ShotSize) * (X1 - X0)) ;

        // Setup sub frustrum T + B
        T = Y0 + (((f32)m_SubShotY     / (f32)m_ShotSize) * (Y1 - Y0)) ;
        B = Y0 + (((f32)(m_SubShotY+1) / (f32)m_ShotSize) * (Y1 - Y0)) ;
    }
#endif // !defined( CONFIG_RETAIL )

    // Flip signs to go from screen coordinates to view space.
    L = -L;
    R = -R;
    T = -T;
    B = -B;

    // Build camera space coordinates.
    f32 YScale   = m_ScaledScreenDistX / m_ScaledScreenDistY;
    vector3 PTL  = vector3(  L,  T*YScale,  m_ScaledScreenDistX );
    vector3 PTR  = vector3(  R,  T*YScale,  m_ScaledScreenDistX );
    vector3 PBL  = vector3(  L,  B*YScale,  m_ScaledScreenDistX );
    vector3 PBR  = vector3(  R,  B*YScale,  m_ScaledScreenDistX );
    vector3 PEYE = vector3(  0,  0, 0 );
    vector3 PNR  = vector3(  0,  0, m_ZNear );
    vector3 PFR  = vector3(  0,  0, m_ZFar );
    vector3 PUP  = vector3(  0,  1,  0 );
    vector3 PLFT = vector3(  1,  0,  0 );

    // Transform into correct system.
    if( System == WORLD )
    {
        PTL  = ConvertV2W(PTL );
        PTR  = ConvertV2W(PTR );
        PBL  = ConvertV2W(PBL );
        PBR  = ConvertV2W(PBR );
        PEYE = ConvertV2W(PEYE);
        PNR  = ConvertV2W(PNR );
        PFR  = ConvertV2W(PFR );
        PUP  = ConvertV2W(PUP );
        PLFT = ConvertV2W(PLFT);
    }

    // Construct side planes.
    Top.Setup   ( PEYE, PTL, PTR );
    Bottom.Setup( PEYE, PBR, PBL );
    Left.Setup  ( PEYE, PBL, PTL );
    Right.Setup ( PEYE, PTR, PBR );

    // Construct LOS normal for near and far.
    Near.Setup( PEYE, PLFT, PUP );
    Near.D = -Near.Dot(PNR);
    Far = Near;
    Far.Negate();
    Far.D  = -Far.Dot(PFR);
}

//==============================================================================

xbool view::PointInView( const vector3& Point, 
                               system   System ) const
{
    // Be sure planes have been constructed.
    if( m_Dirty & DIRTY_PLANES )
        UpdatePlanes();

    // Decide which planes to use.
    plane* pPlane = (System==WORLD) ? (m_WorldSpacePlane) : (m_ViewSpacePlane);

    // Loop through planes looking for a trivial reject.
    for( s32 i=0; i<6; i++ )
    {
        f32 Dist = pPlane[i].Distance( Point );

        // If completely plane, we are culled.
        if( Dist < 0 )
            return( FALSE );
    }

    return( TRUE );
}

//==============================================================================

s32 view::SphereInView( const vector3& Center,
                              f32      Radius,
                              system   System ) const
{
    s32 ReturnValue = 1;

    // Be sure planes have been constructed.
    if( m_Dirty & DIRTY_PLANES )
        UpdatePlanes();

    // Decide which planes to use.
    plane* pPlane = (System==WORLD) ? (m_WorldSpacePlane) : (m_ViewSpacePlane);

    // Loop through planes looking for a trivial reject.
    for(s32 i=0; i<6; i++)
    {
        f32 Dist = pPlane[i].Distance( Center );

        // If completely plane, we are culled.
        if( Dist < -Radius )
            return 0;

        // If partially out, remember.
        if( Dist < Radius )
            ReturnValue = 2;
    }

    return( ReturnValue );
}

//==============================================================================

s32 view::BBoxInView( const bbox&  BBox,
                            u32&   CheckPlaneMask,
                            system System ) const
{
    s32 ReturnValue = VISIBLE_FULL ;    // Means "Fully In View".
    CheckPlaneMask  = 0;

    // Be sure planes have been constructed.
    if( m_Dirty & DIRTY_PLANES )
        UpdatePlanes();

    // Decide which planes to use.
    plane* pPlane = (System==WORLD) ? (m_WorldSpacePlane)    : (m_ViewSpacePlane);
    s32*   pMinI  = (System==WORLD) ? (m_WorldPlaneMinIndex) : (m_ViewPlaneMinIndex);
    s32*   pMaxI  = (System==WORLD) ? (m_WorldPlaneMaxIndex) : (m_ViewPlaneMaxIndex);
    f32*   pF     = (f32*)&BBox;

    // Loop through planes looking for a trivial reject.
    for( s32 i=0; i<6; i++ )
    {
        // Compute max dist along normal
        f32 MaxDist = pPlane->Normal.GetX() * pF[pMaxI[0]] +
                      pPlane->Normal.GetY() * pF[pMaxI[1]] +
                      pPlane->Normal.GetZ() * pF[pMaxI[2]] +
                      pPlane->D;

        // If outside plane, we are culled.
        if( MaxDist < 0 )
        {
            CheckPlaneMask = 0;
            return( VISIBLE_NONE );
        }

        // Compute min dist along normal
        f32 MinDist = pPlane->Normal.GetX() * pF[pMinI[0]] +
                      pPlane->Normal.GetY() * pF[pMinI[1]] +
                      pPlane->Normal.GetZ() * pF[pMinI[2]] +
                      pPlane->D;

        // If partially out, remember.
        if( MinDist < 0 )
        {
            ReturnValue     = VISIBLE_PARTIAL ;
            CheckPlaneMask |= (1<<i);
        }

        // Move to next plane
        pMinI += 3;
        pMaxI += 3;
        pPlane++;
    }

    return( ReturnValue );
} 

//==============================================================================

s32 view::BBoxInView( const bbox& BBox, system System ) const
{
    u32 CheckPlaneMask;
    return BBoxInView( BBox, CheckPlaneMask, System );
}

//==============================================================================

xbool view::SphereInCone    ( const vector3& Center,
                                    f32      Radius ) const
{
    f32 Radius2 = Radius*Radius;

    // Be sure planes have been constructed.
    if( m_Dirty & DIRTY_PLANES )
        UpdatePlanes();

    // Get delta from eye to center of sphere
    vector3 Delta = Center - m_WorldPos;
    f32     DeltaLen2 = Delta.Dot(Delta);
    f32     ZDist = m_ConeAxis.Dot(Delta);

    // Check if eye is contained in sphere
    if( DeltaLen2 < Radius2 )
        return TRUE;

    // Check if sphere is behind camera
    if( (ZDist<m_ZNear) || (ZDist>m_ZFar) )
        return FALSE;

    // Get dist from axis to center and radius of cone
    f32 ConeRadius = m_ConeSlope * ZDist;
    f32 PerpDist2 = DeltaLen2 - (ZDist*ZDist);

    // Check if sphere is trivially rejected
    if( PerpDist2 > (ConeRadius+Radius)*(ConeRadius+Radius) )
        return FALSE;

    return TRUE;
}

//==============================================================================

xbool view::SphereInConeAngle   ( const vector3& Center,
                                        f32      Radius,
                                        f32      tanAngle ) const
{
    f32 Radius2 = Radius*Radius;

    // Be sure planes have been constructed.
    if( m_Dirty & DIRTY_PLANES )
        UpdatePlanes();

    // Get delta from eye to center of sphere
    vector3 Delta = Center - m_WorldPos;
    f32     DeltaLen2 = Delta.Dot(Delta);
    f32     ZDist = m_ConeAxis.Dot(Delta);

    // Check if eye is contained in sphere
    if( DeltaLen2 < Radius2 )
        return TRUE;

    // Check if sphere is behind camera
    if( (ZDist<m_ZNear) || (ZDist>m_ZFar) )
        return FALSE;

    // Get dist from axis to center and radius of cone
    f32 ConeRadius = tanAngle * ZDist;
    f32 PerpDist2 = DeltaLen2 - (ZDist*ZDist);

    // Check if sphere is trivially rejected
    if( PerpDist2 > (ConeRadius+Radius)*(ConeRadius+Radius) )
        return FALSE;

    return TRUE;
}

//==============================================================================

void view::GetProjection( f32& XP0, f32& XP1, f32& YP0, f32& YP1 ) const
{
    UpdateProjection();
    XP0 = m_ProjectX[0];
    XP1 = m_ProjectX[1];
    YP0 = m_ProjectY[0];
    YP1 = m_ProjectY[1];
}

//==============================================================================

vector3 view::PointToScreen( const vector3& Point,
                                   system   System ) const
{
    // Move point to view space.
    vector3 P = Point;
    if( System==WORLD )
        P = ConvertW2V(P);

    // Be sure projection is updated.
    UpdateProjection();

    // Handle strange/dangerous Z's.
    f32 ProjZ = P.GetZ();
    if( ProjZ < 0.001f )
    {
        if( ProjZ > -0.001f ) ProjZ = 0.001f;
        if( ProjZ <  0.000f ) ProjZ = -ProjZ;
    }

    // Project to screen.
    return( vector3(m_ProjectX[0] + m_ProjectX[1] * (P.GetX()/ProjZ),
                    m_ProjectY[0] + m_ProjectY[1] * (P.GetY()/ProjZ),
                    P.GetZ()) );
}

//==============================================================================

vector3 view::RayFromScreen( f32    ScreenX,
                             f32    ScreenY,
                             system System ) const
{
    if( m_Dirty & DIRTY_SCREENDIST )
        UpdateScreenDist();

    // Build ray in viewspace.
    vector3 Ray( -(ScreenX - (m_ViewportX0 + m_ViewportX1) * 0.5f),
                 -(ScreenY - (m_ViewportY0 + m_ViewportY1) * 0.5f) * m_PixelScale,
                 m_ScaledScreenDistX );

    // Transform into correct space.
    if( System == WORLD )
    {
        Ray  = ConvertV2W( Ray );
        Ray -= m_WorldPos;
    }

    Ray.Normalize();
    return( Ray );
}

//==============================================================================

f32 view::CalcScreenSize  ( const vector3& Position,
                                  f32      WorldRadius,
                                  system   System /* = WORLD */ ) const
{
    f32 ZDist;

    if( m_Dirty & DIRTY_SCREENDIST )
        UpdateScreenDist();

    // Get view distance
    if( System == WORLD )
    {
        vector3 Diff = Position - m_WorldPos;
        ZDist = m_WorldOrient(2,0) * Diff.GetX() + 
                m_WorldOrient(2,1) * Diff.GetY() + 
                m_WorldOrient(2,2) * Diff.GetZ();
    }
    else
    {
        ZDist = Position.GetZ();
    }

    // Cull everything behind camera
    if( ZDist < -WorldRadius )
        return 0;

    if( ZDist < WorldRadius )
        ZDist = WorldRadius;

#if !defined( CONFIG_RETAIL )
    // Must be at least one screen!
    ASSERT(m_ShotSize >= 1) ;
#endif // !defined( CONFIG_RETAIL )

    // Return projected size
    f32 ScreenDist = MIN(m_ScaledScreenDistX, m_ScaledScreenDistY);
#if !defined( CONFIG_RETAIL )
    return ((f32)m_ShotSize * WorldRadius * 2 * ScreenDist)/ZDist;
#else
    return ( WorldRadius * 2.0f * ScreenDist ) / ZDist;
#endif // !defined( CONFIG_RETAIL )
}

//==============================================================================

#if !defined( CONFIG_RETAIL )

// Use to take a sub shot of the screen! (useful for multi-part screen shots)
void view::SetSubShot( s32 SubShotX, s32 SubShotY, s32 ShotSize )
{
    // Must be at least one screen!
    ASSERT(ShotSize >= 1) ;
    ASSERT(m_ShotSize >= 1) ;

    // Need to update?
    if ((m_SubShotX != SubShotX) || (m_SubShotY != SubShotY) || (m_ShotSize != ShotSize))
    {
        // Keep new sub shot settings
        m_SubShotX = SubShotX ;
        m_SubShotY = SubShotY ;
        m_ShotSize = ShotSize ;

        // Need to rebuild everything!
        m_Dirty    = DIRTY_ALL;
    }
}

#endif // !defined( CONFIG_RETAIL )

//==============================================================================

void view::UpdateW2V( void ) const
{
    if( m_Dirty & DIRTY_W2V )
    {
        m_W2V = m_WorldOrient;
        m_W2V.Transpose();
        m_W2V.PreTranslate( -m_WorldPos );
        m_Dirty &= ~DIRTY_W2V;
    }
}

//==============================================================================

void view::UpdateV2W( void ) const
{
    if( m_Dirty & DIRTY_V2W )
    {
        m_V2W = m_WorldOrient;
        m_V2W.SetTranslation( m_WorldPos );
        m_Dirty &= ~DIRTY_V2W;
    }
}

//==============================================================================

void view::UpdateV2C( void ) const
{
    if( m_Dirty & DIRTY_V2C )
    {
        m_Dirty &= ~DIRTY_V2C;
        x_memset( &m_V2C, 0, sizeof(matrix4) );

        UpdateYFOV();

#if !defined( CONFIG_RETAIL )
        // Must be at least 1 screen size!
        ASSERT(m_ShotSize >= 1) ;
#endif // !defined( CONFIG_RETAIL )

    #ifdef TARGET_PC

        f32 W = (f32)(1.0f / x_tan( m_XFOV*0.5f ));
        f32 H = (f32)(1.0f / x_tan( m_YFOV*0.5f ));
        f32 Q = m_ZFar/( m_ZFar - m_ZNear);
        m_V2C(0,0) = -W;
        m_V2C(1,1) =  H;
        m_V2C(2,2) =  Q;
        m_V2C(3,2) = -Q*m_ZNear;
        m_V2C(2,3) =  1;

    #elif defined TARGET_XBOX

        f32 W = (f32)(1.0f / x_tan( m_XFOV*0.5f ));
        f32 H = (f32)(1.0f / x_tan( m_YFOV*0.5f ));
        f32 Q = m_ZFar/( m_ZFar - m_ZNear);
        m_V2C(0,0) = -W;
        m_V2C(1,1) =  H;
        m_V2C(2,2) =  Q;
        m_V2C(3,2) = -Q*m_ZNear;
        m_V2C(2,3) =  1;

    #elif defined(TARGET_PS2)
    
        // Simple case
#if !defined( CONFIG_RETAIL )
        if (m_ShotSize == 1)
#endif // !defined( CONFIG_RETAIL )
        {
            f32 W = (f32)(1.0f / x_tan( m_XFOV*0.5f ));
            f32 H = (f32)(1.0f / x_tan( m_YFOV*0.5f ));
            m_V2C(0,0) = -W;
            m_V2C(1,1) =  H;
            m_V2C(2,2) = (m_ZFar+m_ZNear)/(m_ZFar-m_ZNear);
            m_V2C(3,2) = (-2*m_ZNear*m_ZFar)/(m_ZFar-m_ZNear);
            m_V2C(2,3) = 1.0f;
        }
#if !defined( CONFIG_RETAIL )
        else
        {
            // Get full furstrum
            f32 W = m_ZNear * (f32)x_tan(m_XFOV*0.5f) ;
            f32 H = m_ZNear * (f32)x_tan(m_YFOV*0.5f) ;

            // Setup sub frustrum L + R
            f32 R = (-W) + (((f32)m_SubShotX     / (f32)m_ShotSize) * (W - (-W))) ;
            f32 L = (-W) + (((f32)(m_SubShotX+1) / (f32)m_ShotSize) * (W - (-W))) ;

            // Setup sub frustrum T + B
            f32 T = (-H) + (((f32)m_SubShotY     / (f32)m_ShotSize) * (H - (-H))) ;
            f32 B = (-H) + (((f32)(m_SubShotY+1) / (f32)m_ShotSize) * (H - (-H))) ;

            // Finally, setup the view to clip matrix
            m_V2C(0,0) = (2 * m_ZNear) / (R - L) ;
            m_V2C(1,1) = (2 * m_ZNear) / (B - T) ;
            m_V2C(2,0) = (R + L) / (R - L) ;
            m_V2C(2,1) = (B + T) / (B - T) ;
            m_V2C(2,2) = (m_ZFar+m_ZNear)/(m_ZFar-m_ZNear);
            m_V2C(2,3) = 1.0f;
            m_V2C(3,2) = (-2*m_ZNear*m_ZFar)/(m_ZFar-m_ZNear);
        }
#endif // !defined( CONFIG_RETAIL )
    #else

        ASSERT( FALSE );
    
    #endif
    }
}

//==============================================================================

void view::UpdateC2S( void ) const
{
    if( m_Dirty & DIRTY_C2S )
    {
        m_Dirty &= ~DIRTY_C2S;

        x_memset( &m_C2S, 0, sizeof(matrix4) );

    #ifdef TARGET_PC

        f32 W = (m_ViewportX1 - m_ViewportX0+1)*0.5f;
        f32 H = (m_ViewportY1 - m_ViewportY0+1)*0.5f;

        m_C2S(0,0) =  W;
        m_C2S(1,1) = -H;
        m_C2S(2,2) =  1;
        m_C2S(3,3) =  1;
        m_C2S(3,0) =  W + m_ViewportX0;
        m_C2S(3,1) =  H + m_ViewportY0;

    #elif defined(TARGET_PS2)

        f32 W = (m_ViewportX1 - m_ViewportX0+1)*0.5f;
        f32 H = (m_ViewportY1 - m_ViewportY0+1)*0.5f;
        f32 ZScale = (f32)((s32)1<<19);
        
        m_C2S(0,0) =  W;
        m_C2S(1,1) = -H;
        m_C2S(2,2) = -ZScale/2;
        m_C2S(3,3) = 1.0f;
        m_C2S(3,0) = W + m_ViewportX0 + (f32)(2048-(512/2));
        m_C2S(3,1) = H + m_ViewportY0 + (f32)(2048-(512/2));
        m_C2S(3,2) =  ZScale/2;
    
    #else

        ASSERT( FALSE );

    #endif
    }
}

//==============================================================================

void view::UpdateV2S( void ) const
{
    if( m_Dirty & DIRTY_V2S )
    {
        m_Dirty &= ~DIRTY_V2S;

        UpdateV2C();
        UpdateC2S();

        m_V2S = m_C2S * m_V2C;
    }
}

//==============================================================================

void view::UpdateW2C( void ) const
{
    if( m_Dirty & DIRTY_W2C )
    {
        m_Dirty &= ~DIRTY_W2C;

        UpdateW2V();
        UpdateV2C();

        m_W2C = m_V2C * m_W2V;
    }
}

//==============================================================================

void view::UpdateW2S( void ) const
{
    if( m_Dirty & DIRTY_W2S )
    {
        m_Dirty &= ~DIRTY_W2S;

        UpdateW2C();
        UpdateC2S();

        m_W2S = m_C2S * m_W2C;
    }
}

//==============================================================================

void view::UpdateYFOV( void ) const
{
    if( m_Dirty & DIRTY_YFOV )
    {
        // See the comments in the function SetYFOV().

        f32 Distance;
        f32 HalfVPHeight = m_PixelScale * (m_ViewportY1 - m_ViewportY0 + 1) * 0.5f;
        f32 HalfVPWidth  = (m_ViewportX1 - m_ViewportX0 + 1) * 0.5f;

        Distance = HalfVPWidth / x_tan( m_XFOV * 0.5f );
        m_YFOV   = x_atan( HalfVPHeight / Distance ) * 2.0f;
        m_Dirty &= ~DIRTY_YFOV;
        m_Dirty |= DIRTY_V2C;
    }
}

//==============================================================================

void view::UpdatePlanes( void ) const
{
    if( m_Dirty & DIRTY_PLANES )
    {
        m_Dirty &= ~DIRTY_PLANES;

        // Get full frustrum
        f32 W = (m_ViewportX1 - m_ViewportX0 + 1)*0.5f;
        f32 H = (m_ViewportY1 - m_ViewportY0 + 1)*0.5f;

        GetViewPlanes( -W, -H, W, H,
                       m_WorldSpacePlane[3], 
                       m_WorldSpacePlane[2], 
                       m_WorldSpacePlane[0], 
                       m_WorldSpacePlane[1], 
                       m_WorldSpacePlane[4], 
                       m_WorldSpacePlane[5], 
                       WORLD );

        GetViewPlanes( -W, -H, W, H,
                       m_ViewSpacePlane[3], 
                       m_ViewSpacePlane[2], 
                       m_ViewSpacePlane[0], 
                       m_ViewSpacePlane[1], 
                       m_ViewSpacePlane[4], 
                       m_ViewSpacePlane[5], 
                       VIEW );

        for( s32 i=0; i<6; i++ )
        {
            m_WorldSpacePlane[i].GetBBoxIndices( &m_WorldPlaneMinIndex[i*3], &m_WorldPlaneMaxIndex[i*3] );
            m_ViewSpacePlane[i].GetBBoxIndices ( &m_ViewPlaneMinIndex[i*3],  &m_ViewPlaneMaxIndex[i*3] );
        }

        // Build ZPlane and bbox indices
        m_ZPlane = m_WorldSpacePlane[4];
        m_ZPlane.ComputeD( m_WorldPos );
        m_ZPlane.GetBBoxIndices( m_ZPlaneMinI, m_ZPlaneMaxI );

        // Build cone values
        m_ConeAxis = GetViewZ();

        f32     ScreenDist = MIN(m_ScaledScreenDistX, m_ScaledScreenDistY);
        vector3 Ray( -(m_ViewportX0 - ((m_ViewportX0 + m_ViewportX1) * 0.5f)),
                     -(m_ViewportY0 - ((m_ViewportY0 + m_ViewportY1) * 0.5f)),
                     0.0f );
        Ray = Ray * vector3( m_ZFar / ScreenDist, m_ZFar / ScreenDist, 1.0f );
        m_ConeSlope = Ray.Length() / m_ZFar;
    }
}

//==============================================================================

void view::UpdateEdges( void ) const
{
}

//==============================================================================

f32 view::GetScreenDist( void ) const
{
    if( m_Dirty & DIRTY_SCREENDIST )
        UpdateScreenDist();

#if !defined( CONFIG_RETAIL )
    // Must be at least 1 screen size!
    ASSERT(m_ShotSize >= 1) ;
#endif // !defined( CONFIG_RETAIL )

#if !defined( CONFIG_RETAIL )
    return m_ScaledScreenDistX * m_ShotSize ;  // Take shot into account for correct PS2 MipK
#else
    return m_ScaledScreenDistX ;  // Take shot into account for correct PS2 MipK
#endif // !defined( CONFIG_RETAIL )
}

//==============================================================================

void view::UpdateScreenDist( void ) const
{
    if( m_Dirty & DIRTY_SCREENDIST )
    {
        m_Dirty &= ~DIRTY_SCREENDIST;

        // Compute dist using the FOV's. Note that the YFOV has been adjusted to
        // take into account the pixel scale, so the scaled screen distances will
        // not be the same.
        UpdateYFOV();
        f32 HalfVPWidth     = (m_ViewportX1 - m_ViewportX0 + 1) * 0.5f;
        m_ScaledScreenDistX = HalfVPWidth / x_tan( m_XFOV * 0.5f );
        f32 HalfVPHeight    = (m_ViewportY1 - m_ViewportY0 + 1) * 0.5f;
        m_ScaledScreenDistY = HalfVPHeight / x_tan( m_YFOV * 0.5f );
    }
}

//==============================================================================

void view::UpdateProjection( void ) const
{
    if( m_Dirty & DIRTY_PROJECTION )
    {
        m_Dirty &= ~DIRTY_PROJECTION;

        // Compute centers of viewport.
        m_ProjectX[0] = (m_ViewportX0 + m_ViewportX1) * 0.5f;
        m_ProjectY[0] = (m_ViewportY0 + m_ViewportY1) * 0.5f;

        // Setup 'focal length'.
        UpdateScreenDist();
        UpdateYFOV();

        m_ProjectX[1] = -m_ScaledScreenDistX;
        m_ProjectY[1] = -m_ScaledScreenDistY;

#if !defined( CONFIG_RETAIL )
        // Must be at least 1 screen size!
        ASSERT(m_ShotSize >= 1) ;

        // Take big screen shot into account?
        if (m_ShotSize > 1)
        {
            // Scale onto big virtual screen
            m_ProjectX[1] *= m_ShotSize ;
            m_ProjectY[1] *= m_ShotSize ;

            // Make relative to top corner of regular screen
//            m_ProjectX[0] = m_ProjectX[0] - 0.5f*(f32)m_ShotSize * (m_ViewportX1 - m_ViewportX0) +
//                            ((f32)m_SubShotX / (f32)m_ShotSize) * (m_ViewportX1 - m_ViewportX0);
//            m_ProjectY[0] = m_ProjectY[0] - 0.5f*(f32)m_ShotSize * (m_ViewportY1 - m_ViewportY0) +
//                            ((f32)m_SubShotY / (f32)m_ShotSize) * (m_ViewportY1 - m_ViewportY0);

            m_ProjectX[0] -= m_ViewportX0 ;
            m_ProjectY[0] -= m_ViewportY0 ;
        }
#endif // !defined( CONFIG_RETAIL )
    }
}

//==============================================================================
