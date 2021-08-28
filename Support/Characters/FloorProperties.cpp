//==============================================================================
//  
//  FloorProperties.cpp
//
//==============================================================================

#include "FloorProperties.hpp"
#include "Obj_mgr\obj_mgr.hpp"
#include "CollisionMgr\CollisionMgr.hpp"

//==============================================================================

floor_properties::floor_properties( void )
{
    m_StartColor.Zero();
    m_EndColor.Zero();
    m_CurrentColor = xcolor(64,64,64,255);
    m_ColorFadeTime = 0;
    m_RadiusSquared = -1;
    m_LastPosition.Zero();
}

//==============================================================================

floor_properties::~floor_properties( void )
{
}

//==============================================================================

void floor_properties::Init( f32 Radius, f32 ColorFadeTime )
{
    m_RadiusSquared = Radius*Radius;
    m_ColorFadeTime = ColorFadeTime;
}

//==============================================================================

void floor_properties::Update( const vector3& NewPosition, f32 DeltaTime, xbool bIgnorePosition )
{
    // Check if we've been initialized
    ASSERT( m_RadiusSquared >= 0 );

    //
    // Check if object has moved beyond radius
    //
    vector3 HorizDelta = NewPosition - m_LastPosition;
    HorizDelta.GetY() = 0;

    if(( HorizDelta.LengthSquared() > m_RadiusSquared ) && (!bIgnorePosition))
    {
        // Backup the new position
        m_LastPosition = NewPosition;

        xcolor NewFloorColor;
        u32 NewFloorMat=0;
        if( GrabFloorProperties( NewPosition, NewFloorColor, NewFloorMat ) )
        {
            // Backup current color into old color
            m_StartColor.Set( m_CurrentColor.R, m_CurrentColor.G, m_CurrentColor.B );

            // Store new floor material
            m_FloorMat = NewFloorMat;

            // Store new floor color
            m_EndColor.Set( NewFloorColor.R, NewFloorColor.G, NewFloorColor.B );

            // Reset color fade timer
            m_ColorFadeT = 0;
        }
    }

    //
    // Has the color timer run completely out?
    //
    if( m_ColorFadeT != m_ColorFadeTime )
    {
        // Advance the color timer
        m_ColorFadeT += DeltaTime;
        if( m_ColorFadeT > m_ColorFadeTime )
            m_ColorFadeT = m_ColorFadeTime;

        // Compute the interpolated color
        f32 T = m_ColorFadeT / m_ColorFadeTime;
        vector3 C = m_StartColor + T*(m_EndColor - m_StartColor);

        // Build the new xcolor
        m_CurrentColor.R = (u8)C.GetX();
        m_CurrentColor.G = (u8)C.GetY();
        m_CurrentColor.B = (u8)C.GetZ();
        m_CurrentColor.A = (u8)255;
    }
}

//==============================================================================

void floor_properties::ForceUpdate( const vector3& NewPosition )
{
    xcolor  NewFloorColor;
    u32     NewFloorMat=0;

    // Backup the new position
    m_LastPosition = NewPosition;


    // Try to grab new floor properties. If we can't find any we'll stick with
    // what we've got.
    if( GrabFloorProperties( NewPosition, NewFloorColor, NewFloorMat ) )
    {
        // Backup current color into old color
        m_StartColor.GetX() = NewFloorColor.R;
        m_StartColor.GetY() = NewFloorColor.G;
        m_StartColor.GetZ() = NewFloorColor.B;

        // Store new floor material
        m_FloorMat = NewFloorMat;

        // Store new floor color
        m_EndColor.GetX() = NewFloorColor.R;
        m_EndColor.GetY() = NewFloorColor.G;
        m_EndColor.GetZ() = NewFloorColor.B;

        m_CurrentColor.R = NewFloorColor.R;
        m_CurrentColor.G = NewFloorColor.G;
        m_CurrentColor.B = NewFloorColor.B;
        m_CurrentColor.A = (u8)255;
    }

    m_ColorFadeT = m_ColorFadeTime;
}

//==============================================================================

xbool floor_properties::GrabFloorProperties( const vector3& ObjectPosition, xcolor& FloorColor, u32& FloorMat )
{
    // Cast a ray down and collect the collisions
    vector3 Start( ObjectPosition );
    vector3 End  ( Start );
    Start.GetY() += 10.0f;   // sometimes the root can be below the ground, so nudge it up a bit
    End.GetY()   -= 100.0f;  // give it a meters to hit the ground

    g_CollisionMgr.RaySetup( 0, Start, End );
    g_CollisionMgr.CheckCollisions(object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, object::ATTR_LIVING);

    // from the collision info, save out the poly color and material
    s32 iCol;
    for ( iCol = 0; iCol < g_CollisionMgr.m_nCollisions; iCol++ )
    {
        object::detail_tri Tri;
        
        // if we can't get a primitive key, we can't get a color
        s32 PrimitiveKey = g_CollisionMgr.m_Collisions[iCol].PrimitiveKey;
        if ( PrimitiveKey == -1 )
            continue;

        // grab the collision info
        guid HitGuid      = g_CollisionMgr.m_Collisions[iCol].ObjectHitGuid;
        object* pObj      = g_ObjMgr.GetObjectByGuid(HitGuid);
        if ( pObj == NULL )
            continue;

        if ( pObj->GetColDetails( PrimitiveKey, Tri ) == FALSE )
            continue;

        // the color is a weighted average of the triangle colors
        // use barycentric coords to achieve that
        const vector3& P0 = Tri.Vertex[0];
        const vector3& P1 = Tri.Vertex[1];
        const vector3& P2 = Tri.Vertex[2];
        const vector3& TP = g_CollisionMgr.m_Collisions[iCol].Point;

        // Compute scaled normal
        vector3 Normal = v3_Cross(P1-P0, P2-P0);
        Normal *= 1.0f / Normal.LengthSquared();

        // Compute barycentric co-ords
        vector3 Bary( v3_Cross(P2-P1, TP-P1).Dot(Normal),
                      v3_Cross(P0-P2, TP-P2).Dot(Normal),
                      v3_Cross(P1-P0, TP-P0).Dot(Normal) );

        // now we can get the color
        const xcolor& C0 = Tri.Color[0];
        const xcolor& C1 = Tri.Color[1];
        const xcolor& C2 = Tri.Color[2];
        vector3 vFloorCol( Bary.GetX()*C0.R + Bary.GetY()*C1.R + Bary.GetZ()*C2.R,
                           Bary.GetX()*C0.G + Bary.GetY()*C1.G + Bary.GetZ()*C2.G,
                           Bary.GetX()*C0.B + Bary.GetY()*C1.B + Bary.GetZ()*C2.B );
        vFloorCol.Min( 255.0f );
        vFloorCol.Max( 0.0f   );

        // woohoo...done with the hard stuff
        FloorColor.Set((u8)vFloorCol.GetX(), (u8)vFloorCol.GetY(), (u8)vFloorCol.GetZ());
        FloorMat = g_CollisionMgr.m_Collisions[iCol].Flags;
        return TRUE;
    }

    return FALSE;
}

//==============================================================================
