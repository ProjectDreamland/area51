//=========================================================================
//
// ForceField.cpp
//
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================
#include "ForceField.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "x_context.hpp"
#include "NetworkMgr\NetObjMgr.hpp"
#include "Player.hpp"
#include "Render\LightMgr.hpp"
#include "GameLib\RigidGeomCollision.hpp"
#include "CollisionMgr\PolyCache.hpp"

#ifndef X_EDITOR
#include "GameLib\RenderContext.hpp"
#endif

//=========================================================================

rhandle<xbitmap>            force_field::m_ForceTexture;
rhandle<xbitmap>            force_field::m_ForceCloudTexture;

#define DEPTH 4

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

#define COLLIDABLE_FLAGS ( object::ATTR_COLLIDABLE | \
    object::ATTR_BLOCKS_ALL_PROJECTILES | \
    object::ATTR_BLOCKS_ALL_ACTORS | \
    object::ATTR_BLOCKS_RAGDOLL | \
    object::ATTR_BLOCKS_CHARACTER_LOS | \
    object::ATTR_BLOCKS_PLAYER_LOS | \
    object::ATTR_BLOCKS_PAIN_LOS | \
    object::ATTR_BLOCKS_SMALL_DEBRIS)

static struct force_field_desc : public object_desc
{
    force_field_desc( void ) : object_desc( 
        object::TYPE_FORCE_FIELD, 
        "Force Field",
        "Multiplayer",
        COLLIDABLE_FLAGS                    |
        object::ATTR_RENDERABLE             |
        object::ATTR_SPACIAL_ENTRY          |
        object::ATTR_TRANSPARENT            |
        object::ATTR_NEEDS_LOGIC_TIME,       

        FLAGS_GENERIC_EDITOR_CREATE | 
        FLAGS_IS_DYNAMIC            |
        FLAGS_NO_ICON
        ) {}

        //-------------------------------------------------------------------------

        virtual object* Create( void ) 
        {                                  
            return new force_field; 
        }

} s_force_field_Desc;

//=========================================================================

const object_desc& force_field::GetTypeDesc( void ) const
{
    return s_force_field_Desc;
}

//=========================================================================

const object_desc& force_field::GetObjectType( void )
{
    return s_force_field_Desc;
}

//=========================================================================
// FUNCTIONS
//=========================================================================

force_field::force_field( void )
{ 
    m_NewState               = FRIENDLY_ALL;
    m_OldState               = FRIENDLY_ALL;
    m_TransitionValue        = 1.0f;

    m_bInitialized           = FALSE;
    m_pVertices              = NULL;
    m_pSeekingVertices       = NULL;

    m_Width                  = 100.0f;
    m_Height                 = 100.0f;

    m_ScrollFactor           = 0.0f;

    m_bOn                    = TRUE;
    m_PercentageOn           = 0.5f;
    m_LastState              = STATE_CLOSED;

    m_Brightness             = 1.0f;
    m_Phase                  = 0.0f;
    m_Hit                    = 0.0f;

    m_bActiveWhenFriendlyAll = TRUE;

    m_ForceTexture.SetName     ( "ForceField.xbmp"      );
    m_ForceCloudTexture.SetName( "ForceFieldCloud.xbmp" );
}

//=========================================================================

force_field::~force_field( void )
{
    ASSERT( m_pVertices );
    delete m_pVertices;
    delete m_pSeekingVertices;
}

//=========================================================================

void force_field::OnInit( void )
{
}

//=========================================================================

void force_field::CreateVertices( void )
{
    m_bInitialized = TRUE;

    if( m_pVertices )
    {
        delete m_pVertices;
        delete m_pSeekingVertices;
    }

    m_NumRows = (s32)x_sqrt(m_Height / 5) + 1;
    m_NumCols = (s32)x_sqrt(m_Width  / 5) + 1;

    if( (m_NumRows % 2) == 0 ) m_NumRows++;
    if( (m_NumCols % 2) == 0 ) m_NumCols++;

    m_pVertices        = new vector2[ m_NumRows * m_NumCols ];
    m_pSeekingVertices = new vector2[ m_NumRows * m_NumCols ];

    for( s32 i = 0; i < m_NumRows; i++ )

    {
        for( s32 j = 0; j < m_NumCols; j++ )
        {
            m_pSeekingVertices[ i * m_NumCols + j ].X = (m_Width  * ((f32)j / (f32)(m_NumCols - 1))) - (m_Width  / 2);
            m_pSeekingVertices[ i * m_NumCols + j ].Y = (m_Height * ((f32)i / (f32)(m_NumRows - 1))) - (m_Height / 2);

            m_pVertices[ i * m_NumCols + j ].X = (m_Width  * ((f32)j / (f32)(m_NumCols - 1))) - (m_Width  / 2); 
            m_pVertices[ i * m_NumCols + j ].Y = (m_Height * ((f32)i / (f32)(m_NumRows - 1))) - (m_Height / 2);
        }
    }
}

//=========================================================================

void force_field::OnColCheck( void )
{
    CONTEXT("force_field::OnColCheck");

    //
    // Compute corners
    //
    vector3 Corner[8];
    {
        vector3 Pos[8];
        //vector3 C[4];
        f32 W  = m_Width  * 0.5f;
        f32 H  = m_Height * 0.5f;
        f32 D  = DEPTH    * 0.5f;
        Pos[0] = vector3( -W, +H, D );
        Pos[1] = vector3( -W, -H, D );
        Pos[2] = vector3( +W, -H, D );
        Pos[3] = vector3( +W, +H, D );

        Pos[4] = vector3( -W, +H, -D ); 
        Pos[5] = vector3( -W, -H, -D ); 
        Pos[6] = vector3( +W, -H, -D ); 
        Pos[7] = vector3( +W, +H, -D ); 

        matrix4 L2W = GetL2W();
        L2W.Transform( Corner, Pos, 8 );
    }

    g_CollisionMgr.StartApply( GetGuid() );

    // Side 1
    g_CollisionMgr.ApplyTriangle( Corner[0],Corner[1],Corner[2], object::MAT_TYPE_ENERGY_FIELD ); 
    g_CollisionMgr.ApplyTriangle( Corner[0],Corner[2],Corner[3], object::MAT_TYPE_ENERGY_FIELD ); 

    // Side 6
    g_CollisionMgr.ApplyTriangle( Corner[5],Corner[4],Corner[7], object::MAT_TYPE_ENERGY_FIELD ); 
    g_CollisionMgr.ApplyTriangle( Corner[6],Corner[5],Corner[7], object::MAT_TYPE_ENERGY_FIELD ); 
    //
    g_CollisionMgr.EndApply();

    //m_Hit = 1.0f;
}

//=========================================================================

void force_field::OnPolyCacheGather( void )
{
    if( GetState() != STATE_CLOSED )
    {
        return;
    }

    //
    // Compute corners
    //
    vector3 Corner[8];
    vector3 Normal[6];
    {
        vector3 Pos[8];
        f32 W  = m_Width  * 0.5f;
        f32 H  = m_Height * 0.5f;
        f32 D  = DEPTH  * 0.5f;

        Pos[0] = vector3( -W, +H, D );
        Pos[1] = vector3( -W, -H, D );
        Pos[2] = vector3( +W, -H, D );
        Pos[3] = vector3( +W, +H, D );

        Pos[4] = vector3( -W, +H, -D ); 
        Pos[5] = vector3( -W, -H, -D ); 
        Pos[6] = vector3( +W, -H, -D ); 
        Pos[7] = vector3( +W, +H, -D ); 

        matrix4 L2W = GetL2W();
        L2W.Transform( Corner, Pos, 8 );

        Normal[0] = v3_Cross( Corner[1]-Corner[0], Corner[2]-Corner[0] );
        Normal[0].Normalize();
        Normal[1] = v3_Cross( Corner[5]-Corner[4], Corner[1]-Corner[4] );
        Normal[1].Normalize();
        Normal[2] = v3_Cross( Corner[0]-Corner[4], Corner[3]-Corner[4] );
        Normal[2].Normalize();
        Normal[3] = v3_Cross( Corner[2]-Corner[3], Corner[6]-Corner[3] );
        Normal[3].Normalize();
        Normal[4] = v3_Cross( Corner[5]-Corner[1], Corner[6]-Corner[1] );
        Normal[4].Normalize();
        Normal[5] = v3_Cross( Corner[6]-Corner[7], Corner[5]-Corner[7] );
        Normal[5].Normalize();
    }

    //
    // Setup basic collision info arrays
    //
    collision_data                  CD;
    collision_data::low_cluster     LowCluster;
    collision_data::low_quad        LowQuad[6];
    vector3                         LowVector[24+6];

    CD.nHighClusters     = 0;
    CD.nHighIndices      = 0;
    CD.pHighCluster      = NULL;
    CD.pHighIndexToVert0 = 0;
    CD.nLowClusters      = 1;
    CD.nLowQuads         = 6;
    CD.nLowVectors       = 24+6;
    CD.pLowCluster       = &LowCluster;
    CD.pLowQuad          = LowQuad;
    CD.pLowVector        = LowVector;

    // Set up LowCluster
    LowCluster.iVectorOffset    =  0;
    LowCluster.iBone            =  0;
    LowCluster.iMesh            =  0;
    LowCluster.iQuadOffset      =  0;
    LowCluster.nPoints          = 24;
    LowCluster.nNormals         =  6;
    LowCluster.nQuads           =  6;
    LowCluster.BBox.Clear();
    LowCluster.BBox.AddVerts( Corner, 8 );

    // Setup vertex positions
    CD.pLowVector[0]  =  Corner[0];
    CD.pLowVector[1]  =  Corner[1];
    CD.pLowVector[2]  =  Corner[2];
    CD.pLowVector[3]  =  Corner[3];

    CD.pLowVector[4]  =  Corner[5]; 
    CD.pLowVector[5]  =  Corner[1]; 
    CD.pLowVector[6]  =  Corner[0]; 
    CD.pLowVector[7]  =  Corner[4]; 

    CD.pLowVector[8]  =  Corner[0]; 
    CD.pLowVector[9]  =  Corner[3]; 
    CD.pLowVector[10] =  Corner[7]; 
    CD.pLowVector[11] =  Corner[4]; 

    CD.pLowVector[12] =  Corner[2]; 
    CD.pLowVector[13] =  Corner[6]; 
    CD.pLowVector[14] =  Corner[7]; 
    CD.pLowVector[15] =  Corner[3]; 

    CD.pLowVector[16] =  Corner[5]; 
    CD.pLowVector[17] =  Corner[6]; 
    CD.pLowVector[18] =  Corner[2]; 
    CD.pLowVector[19] =  Corner[1]; 

    CD.pLowVector[20] =  Corner[6]; 
    CD.pLowVector[21] =  Corner[5]; 
    CD.pLowVector[22] =  Corner[4]; 
    CD.pLowVector[23] =  Corner[7]; 

    CD.pLowVector[24] =  Normal[0];
    CD.pLowVector[25] =  Normal[1];
    CD.pLowVector[26] =  Normal[2];
    CD.pLowVector[27] =  Normal[3];
    CD.pLowVector[28] =  Normal[4];
    CD.pLowVector[29] =  Normal[5];

    // Setup quad 1
    LowQuad[0].iP[0] =  0;
    LowQuad[0].iP[1] =  1;
    LowQuad[0].iP[2] =  2;
    LowQuad[0].iP[3] =  3;
    LowQuad[0].iN    =  0;
    LowQuad[0].Flags =  0;

    // Setup quad 2
    LowQuad[1].iP[0] =  4;
    LowQuad[1].iP[1] =  5;
    LowQuad[1].iP[2] =  6;
    LowQuad[1].iP[3] =  7;
    LowQuad[1].iN    =  1;
    LowQuad[1].Flags =  0;

    // Setup quad 3
    LowQuad[2].iP[0] =  8;
    LowQuad[2].iP[1] =  9;
    LowQuad[2].iP[2] = 10;
    LowQuad[2].iP[3] = 11;
    LowQuad[2].iN    =  2;
    LowQuad[2].Flags =  0;

    // Setup quad 4
    LowQuad[3].iP[0] = 12;
    LowQuad[3].iP[1] = 13;
    LowQuad[3].iP[2] = 14;
    LowQuad[3].iP[3] = 15;
    LowQuad[3].iN    =  3;
    LowQuad[3].Flags =  0;

    // Setup quad 5
    LowQuad[4].iP[0] = 16;
    LowQuad[4].iP[1] = 17;
    LowQuad[4].iP[2] = 18;
    LowQuad[4].iP[3] = 19;
    LowQuad[4].iN    =  4;
    LowQuad[4].Flags =  0;

    // Setup quad 6
    LowQuad[5].iP[0] = 20;
    LowQuad[5].iP[1] = 21;
    LowQuad[5].iP[2] = 22;
    LowQuad[5].iP[3] = 23;
    LowQuad[5].iN    =  5;
    LowQuad[5].Flags =  0;

    // Pass it to polycache.
    matrix4 L2W;
    L2W.Identity();
    g_PolyCache.GatherCluster( CD, &L2W, ~((u64)0), GetGuid() );
}

//==============================================================================

inline xcolor Interpolate( xcolor Color1, xcolor Color2, f32 Percentage )
{
    xcolor Color( 
        (u8)(Color2.R * Percentage + Color1.R * (1.0f - Percentage)),
        (u8)(Color2.G * Percentage + Color1.G * (1.0f - Percentage)),
        (u8)(Color2.B * Percentage + Color1.B * (1.0f - Percentage)),
        (u8)(Color2.A * Percentage + Color1.A * (1.0f - Percentage))
        );
    return Color;
}

//=========================================================================

void force_field::OnRenderTransparent( void )
{
    CONTEXT( "force_field::OnRenderTransparent" );

    if( !m_bInitialized )
    {
        CreateVertices();
    }

    ASSERT( m_bInitialized );
    ASSERT( m_pVertices );

    //draw_BBox( GetBBox() );

    // Vertices.
    if( FALSE )
    {
        draw_Begin( DRAW_LINES, 0 );
        xcolor Color = XCOLOR_YELLOW;

        draw_Color( Color );

        for( s32 i = 0; i < (m_NumCols * m_NumRows); i++ )
        {
            {
                vector3 V1( m_pVertices[ i ].X, m_pVertices[ i ].Y, +2);
                vector3 V2( m_pVertices[ i ].X, m_pVertices[ i ].Y, -2);

                matrix4 L2W = GetL2W();

                V1 = L2W.Transform( V1 );
                V2 = L2W.Transform( V2 );

                draw_Vertex( V1 );
                draw_Vertex( V2 );
            }
        }

        draw_End();
    }

#ifndef X_EDITOR
    u32 States[ 2 ]     = { m_OldState, m_NewState };
    xcolor Colors[ 2 ]  = { FRIEND_TO_ALL, FRIEND_TO_ALL };

    player* pPlayer = (player*)NetObjMgr.GetObjFromSlot( g_RenderContext.NetPlayerSlot );

    // We know what the alignment is, just check how that relates to the player viewing it.
    for( s32 i = 0; i < 2; i++ )
    {
        switch( States[ i ] )
        {
        case FRIENDLY_NONE:
            Colors[ i ] = XCOLOR_YELLOW;
            break;

        case FRIENDLY_ALPHA: // Fall through.
        case FRIENDLY_OMEGA:
            Colors[ i ] = (pPlayer->net_GetTeamBits() & States[ i ]) ? XCOLOR_GREEN : XCOLOR_RED;
            break;

        case FRIENDLY_ALL:
            Colors[ i ] = XCOLOR_BLUE;
            break;

        default:
            break;
        }
    }

    xcolor BaseColor = Interpolate( Colors[ 0 ], Colors[ 1 ], m_TransitionValue );
#else
    xcolor BaseColor = m_Circuit.GetColor();
#endif

    static f32 HexAlpha    = 0.5f;
    static f32 CloudAlpha  = 0.2f;

    s32 EvenSquare[ 4 ] = { 0, 1, 2, 3 };
    s32 OddSquare [ 4 ] = { 1, 3, 0, 2 };

    static f32 DynamicAlpha = 0.01f;

    xcolor FringeColor = XCOLOR_WHITE;
    FringeColor.A = (u8)(255 * m_PercentageOn);

    // Hex Pattern.
    {
        draw_ClearL2W();

        vector3 Points[ 4 ];
        for( s32 i = 0; i < m_NumRows - 1; i++ )
        {
            for( s32 j = 0; j < m_NumCols - 1; j++) 
            {
                s32 Indices[ 4 ] =
                {
                    (i * m_NumCols) + j,
                    (i * m_NumCols) + j + 1,
                    ((i + 1) * m_NumCols) + j,
                    ((i + 1) * m_NumCols) + j + 1
                };

#ifdef X_DEBUG
                for( s32 l = 0; l < 4; l++ )
                {
                    ASSERT( IN_RANGE( 0, Indices[ l ], (m_NumCols * m_NumRows) - 1 ) );
                }
#endif

                Points[0]( m_pVertices[ Indices[ 0 ] ].X, m_pVertices[ Indices[ 0 ] ].Y,  0.0f );
                Points[1]( m_pVertices[ Indices[ 1 ] ].X, m_pVertices[ Indices[ 1 ] ].Y,  0.0f );
                Points[2]( m_pVertices[ Indices[ 2 ] ].X, m_pVertices[ Indices[ 2 ] ].Y,  0.0f );
                Points[3]( m_pVertices[ Indices[ 3 ] ].X, m_pVertices[ Indices[ 3 ] ].Y,  0.0f );

                xcolor Color = Interpolate( BaseColor, FringeColor, x_min( m_PercentageOn, MINMAX( 0.4f, m_Hit, 1.0f ) ) );
                Color.A = x_max( (u8)(255 * ((HexAlpha * m_PercentageOn))), (u8)(255 * x_min( 1.0f, m_Hit ) * m_PercentageOn ));

                matrix4 L2W = GetL2W();
                L2W.Transform( Points, Points, 4 );

                draw_Begin( DRAW_TRIANGLE_STRIPS, DRAW_USE_ALPHA|DRAW_CULL_NONE|DRAW_TEXTURED|DRAW_NO_ZWRITE );

                const xbitmap* pBitmap = m_ForceTexture.GetPointer();

                draw_SetTexture( *pBitmap );

                draw_Color( Color );

                static f32 TexDim = 400.0f;

                vector2 PointUV[ 4 ] = 
                {
                    vector2( (((f32)(j + 0) / (f32)m_NumCols) * m_Width ) / TexDim,
                             (((f32)(i + 0) / (f32)m_NumRows) * m_Height) / TexDim ),
                    vector2( (((f32)(j + 1) / (f32)m_NumCols) * m_Width ) / TexDim,
                             (((f32)(i + 0) / (f32)m_NumRows) * m_Height) / TexDim ),
                    vector2( (((f32)(j + 0) / (f32)m_NumCols) * m_Width ) / TexDim,
                             (((f32)(i + 1) / (f32)m_NumRows) * m_Height) / TexDim ),
                    vector2( (((f32)(j + 1) / (f32)m_NumCols) * m_Width ) / TexDim,
                             (((f32)(i + 1) / (f32)m_NumRows) * m_Height) / TexDim )
                };

                for( s32 k = 0; k < 4; k++ )
                {
                    s32 PointNum = (((i + j) % 2) == 0) ? EvenSquare[ k ] : OddSquare[ k ];

                    draw_UV    (    PointUV[ PointNum ] );
                    draw_Vertex(     Points[ PointNum ] );
                }

                draw_End();
            }
        }
    }

    // Clouds.
    const xbitmap* pBitmap = m_ForceCloudTexture.GetPointer();
    if( pBitmap )
    {
        // Set up the "random" colors.
        f32 ColorVal1 = ((x_sin( m_Phase * 2.0f ) + 1.0f) / 2.0f);
        f32 ColorVal2 = 1.0f - ColorVal1;

        xcolor PossibleColors[ 4 ] =
        {
                xcolor( (u8)(127 * ColorVal1), (u8)(255 * ColorVal2),       (u8)(255 * ColorVal1), (u8)(255 * m_PercentageOn) ),
                xcolor( (u8)(255 * ColorVal2), (u8)(127 * ColorVal1 + 127), (u8)(127 * ColorVal1), (u8)(255 * m_PercentageOn) ),
                xcolor( (u8)(255 * ColorVal2), (u8)(255 * ColorVal1),       (u8)(255 * ColorVal2), (u8)(255 * m_PercentageOn) ),
                xcolor( (u8)(127 * ColorVal1), (u8)(255 * ColorVal2),       (u8)(255 * ColorVal1), (u8)(255 * m_PercentageOn) ),
        };

        draw_ClearL2W();

        vector3 Points[ 4 ];
        for( s32 i = 0; i < m_NumRows - 1; i++ )
        {
            for( s32 j = 0; j < m_NumCols - 1; j++) 
            {
                s32 Indices[ 4 ] =
                {
                    (i * m_NumCols) + j,
                        (i * m_NumCols) + j + 1,
                        ((i + 1) * m_NumCols) + j,
                        ((i + 1) * m_NumCols) + j + 1
                };

#ifdef X_DEBUG
                for( s32 l = 0; l < 4; l++ )
                {
                    ASSERT( IN_RANGE( 0, Indices[ l ], (m_NumCols * m_NumRows) - 1 ) );
                }
#endif

                Points[0]( m_pVertices[ Indices[ 0 ] ].X, m_pVertices[ Indices[ 0 ] ].Y,  0.0f );
                Points[1]( m_pVertices[ Indices[ 1 ] ].X, m_pVertices[ Indices[ 1 ] ].Y,  0.0f );
                Points[2]( m_pVertices[ Indices[ 2 ] ].X, m_pVertices[ Indices[ 2 ] ].Y,  0.0f );
                Points[3]( m_pVertices[ Indices[ 3 ] ].X, m_pVertices[ Indices[ 3 ] ].Y,  0.0f );

                static f32 InterpAmount = 0.10f;

                xcolor Color = BaseColor;
                Color.A = (u8)(255 * ((CloudAlpha + DynamicAlpha * x_sin( m_Phase )) * m_PercentageOn));

                xcolor HitColors[ 4 ] =
                {
                    (IN_RANGE( m_NumCols, Indices[ 0 ], m_NumCols * (m_NumRows - 1) - 1 ) && (((Indices[ 0 ] % m_NumCols) != 0) && ((Indices[ 0 ] % m_NumCols) != (m_NumCols - 1)))) ? Interpolate( Color, PossibleColors[ Indices[ 0 ] % 4 ], InterpAmount ) : Interpolate( Color, FringeColor, 0.5f ),
                    (IN_RANGE( m_NumCols, Indices[ 1 ], m_NumCols * (m_NumRows - 1) - 1 ) && (((Indices[ 1 ] % m_NumCols) != 0) && ((Indices[ 1 ] % m_NumCols) != (m_NumCols - 1)))) ? Interpolate( Color, PossibleColors[ Indices[ 1 ] % 4 ], InterpAmount ) : Interpolate( Color, FringeColor, 0.5f ),
                    (IN_RANGE( m_NumCols, Indices[ 2 ], m_NumCols * (m_NumRows - 1) - 1 ) && (((Indices[ 2 ] % m_NumCols) != 0) && ((Indices[ 2 ] % m_NumCols) != (m_NumCols - 1)))) ? Interpolate( Color, PossibleColors[ Indices[ 2 ] % 4 ], InterpAmount ) : Interpolate( Color, FringeColor, 0.5f ),
                    (IN_RANGE( m_NumCols, Indices[ 3 ], m_NumCols * (m_NumRows - 1) - 1 ) && (((Indices[ 3 ] % m_NumCols) != 0) && ((Indices[ 3 ] % m_NumCols) != (m_NumCols - 1)))) ? Interpolate( Color, PossibleColors[ Indices[ 3 ] % 4 ], InterpAmount ) : Interpolate( Color, FringeColor, 0.5f )
                };

                matrix4 L2W = GetL2W();
                L2W.Transform( Points, Points, 4 );

                draw_Begin( DRAW_TRIANGLE_STRIPS, DRAW_USE_ALPHA|DRAW_CULL_NONE|DRAW_TEXTURED|DRAW_NO_ZWRITE );

                draw_SetTexture( *pBitmap );

                draw_Color( Color );

                static f32 TexDim = 256.0f;

                vector2 PointUV[ 4 ] = 
                {
                    vector2( (((f32)(j + 0) / (f32)m_NumCols) * m_Width ) / TexDim + m_ScrollFactor,
                             (((f32)(i + 0) / (f32)m_NumRows) * m_Height) / TexDim + m_ScrollFactor ),
                    vector2( (((f32)(j + 1) / (f32)m_NumCols) * m_Width ) / TexDim + m_ScrollFactor,
                             (((f32)(i + 0) / (f32)m_NumRows) * m_Height) / TexDim + m_ScrollFactor ),
                    vector2( (((f32)(j + 0) / (f32)m_NumCols) * m_Width ) / TexDim + m_ScrollFactor,
                             (((f32)(i + 1) / (f32)m_NumRows) * m_Height) / TexDim + m_ScrollFactor ),
                    vector2( (((f32)(j + 1) / (f32)m_NumCols) * m_Width ) / TexDim + m_ScrollFactor,
                             (((f32)(i + 1) / (f32)m_NumRows) * m_Height) / TexDim + m_ScrollFactor )
                };

                for( s32 k = 0; k < 4; k++ )
                {
                    s32 PointNum = (((i + j) % 2) == 0) ? EvenSquare[ k ] : OddSquare[ k ];

                    draw_UV    (    PointUV[ PointNum ] );
                    draw_Color (  HitColors[ PointNum ] );
                    draw_Vertex(     Points[ PointNum ] );
                }

                draw_End();
            }
        }
    }

#ifdef X_EDITOR
    m_Circuit.SpecialRender( GetPosition() );
#endif
}

//=========================================================================

force_field::field_states force_field::GetState( void )
{
    if( m_PercentageOn < 0.5f )
    {
        return STATE_OPEN;
    }
    else
    {
        return STATE_CLOSED;
    }
}

//=========================================================================

bbox force_field::GetLocalBBox( void ) const
{
    return bbox( vector3( m_Width/2, m_Height/2, DEPTH * 0.5f), 
        vector3(-m_Width/2,-m_Height/2,-DEPTH * 0.5f) );
}

//=========================================================================

void force_field::OnProjectileImpact  ( vector3& Point )
{
    (void)Point;
    //    matrix4 L2W = GetL2W();
    //    L2W.InvertRT();

    //    vector3 LocalPoint = L2W.Transform( Point );
    //    f32 PosX = LocalPoint.GetX();
    //    f32 PosY = LocalPoint.GetY();

    m_Hit = 1.5f;

    //s32 HitVertexX = MINMAX( 0, (s32)(((PosX / m_Width)  + 0.5f) * m_NumCols), m_NumCols );
    //s32 HitVertexY = MINMAX( 0, (s32)(((PosY / m_Height) + 0.5f) * m_NumRows), m_NumRows );
}

//=========================================================================

void force_field::OnAdvanceLogic ( f32 DeltaTime )
{
    CONTEXT( "force_field::OnAdvanceLogic" );

    m_Phase += 6.0f * DeltaTime;
    if( m_Phase > R_360 )
    {
        m_Phase -= R_360;
    }

    m_Hit = x_max( 0.0f, m_Hit - DeltaTime );

    if( !m_bInitialized )
    {
        CreateVertices();
    }

    u32 TeamBits = m_Circuit.GetTeamBits();

#ifndef X_EDITOR
    // Zone based check.  Locked zones trump teambits.
    if( GameMgr.IsZoneLocked( GetZone1() ) || 
        GameMgr.IsZoneLocked( GetZone2() ) )
    {
        // Set teambits friendly to none.
        TeamBits = 0;
    }
#endif

    // If the state has changed, swap the state values and reset the 
    // transition time to start the fading process.
    if( m_NewState != TeamBits )
    {
        m_OldState = m_NewState;
        m_NewState = TeamBits;
        m_TransitionValue = 0.0f;
    }

    f32 TransitionTime = 2.0f;

    m_ScrollFactor += DeltaTime / 10.0f;
    if( m_ScrollFactor > 1.0f )
    {
        m_ScrollFactor -= 1.0f;
    }

    m_TransitionValue = MINMAX( 0.0f, 
        m_TransitionValue + (DeltaTime / TransitionTime), 
        1.0f );

    // If nobody is present around it, AND it is not friendly to all 
    // with the deactive when friendly all flag, increment its power value.
    if( m_bOn && 
        !((m_NewState == FRIENDLY_ALL) && !m_bActiveWhenFriendlyAll) )
    {
        m_PercentageOn += 2.0f * DeltaTime;
    }
    else
    {
        m_PercentageOn -= 3.0f * DeltaTime;
    }
    m_PercentageOn = MINMAX( 0.0f, m_PercentageOn, 1.0f );
    m_bOn = TRUE;

    // This looks arbitrary because it is, but it seems to work.
    f32 Sins[ 5 ] =
    {
        x_sin( m_Phase - ((0.35f) * PI * 0.0f )),
        x_sin( m_Phase - ((0.35f) * PI * 1.0f )),
        x_sin( m_Phase - ((0.35f) * PI * 2.0f )),
        x_sin( m_Phase - ((0.35f) * PI * 3.0f )),
        x_sin( m_Phase - ((0.35f) * PI * 4.0f ))
    };

    // Set up the vertices.
    for( s32 i = 0; i < m_NumRows; i++ )
    {
        for( s32 j = 0; j < m_NumCols; j++ )
        {
            if( IN_RANGE( 1, i, m_NumRows - 2 ) && IN_RANGE( 1, j, m_NumCols - 2 ) )
            {
                // Get the current actual position of this point.
                vector2 CurrPos     = m_pVertices[ i * m_NumCols + j ];

                // Get the base position.
                vector2 ExpectedPos( (m_Width  * ((f32)j / (f32)(m_NumCols - 1))) - (m_Width  / 2), (m_Height * ((f32)i / (f32)(m_NumRows - 1))) - (m_Height / 2) );

                // Get the current seeking position.
                vector2 SeekingPos;
                SeekingPos.X  = (m_pSeekingVertices[ i * m_NumCols + j ].X * 0.98f) + (ExpectedPos.X * 0.02f) + (2.0f * (m_Width  / (m_NumCols * 40)) * Sins[ 0 + ((i + j) % 5) ]);
                SeekingPos.Y  = (m_pSeekingVertices[ i * m_NumCols + j ].Y * 0.98f) + (ExpectedPos.Y * 0.02f) - (2.0f * (m_Height / (m_NumRows * 40)) * Sins[ 4 - ((i + j) % 5) ]);

                // Expected position.

                m_pVertices[ i * m_NumCols + j ] = (CurrPos * 0.8f) + (SeekingPos * 0.2f);
            }
        }
    }

    // Collision.
    if( GetState() != m_LastState )
    {
        if( GetState() == STATE_CLOSED )
        {
            SetAttrBits( GetAttrBits() | COLLIDABLE_FLAGS );
            g_PolyCache.InvalidateCells( GetBBox(), GetGuid() );
        }
        else
        {
            SetAttrBits( (GetAttrBits() & ~((u32)0 | COLLIDABLE_FLAGS)) );
            g_PolyCache.InvalidateCells( GetBBox(), GetGuid() );
        }
    }

    static f32 LastFactor  = 0.96f;
    static f32 SinFactor   = 0.01f;
    static f32 ConstFactor = 0.02f;
    static f32 ConstVal    = 0.40f;

    m_Brightness = (LastFactor  * m_Brightness) + 
        (SinFactor   * (0.5f * (x_sin( m_Phase ) + 1.0f))) + 
        (ConstFactor * ConstVal);  

    m_LastState = GetState();
}

//==============================================================================

void force_field::OnEnumProp( prop_enum& List )
{      
    m_Circuit.OnEnumProp ( List );
    object::OnEnumProp   ( List );

    List.PropEnumHeader  ( "ForceField",         "The forcefield object.", 0 );

    List.PropEnumFloat   ( "ForceField\\Width",           "ForceField Width.", 0              );
    List.PropEnumFloat   ( "ForceField\\Height",          "ForceField Height.", 0             );
    List.PropEnumBool    ( "ForceField\\ActiveWhenFriendlyAll", "Whether this forcefield will exist when it's friendly all and its zones are unlocked.", 0 );

    u32 Flags = PROP_TYPE_DONT_SAVE | 
        PROP_TYPE_DONT_SHOW | 
        PROP_TYPE_DONT_EXPORT | 
        PROP_TYPE_DONT_SAVE_MEMCARD;

    List.PropEnumExternal( "ForceField\\MainGraphic", "Resource\0xbmp",     "Resource Animation File", Flags );
    List.PropEnumExternal( "ForceField\\ScanGraphic", "Resource\0xbmp",     "Resource Animation File", Flags );

}

//==============================================================================

xbool force_field::OnProperty( prop_query&  Query )                
{
    if( object::OnProperty( Query ) )
    {
        return( TRUE );
    }
    if( m_Circuit.OnProperty( Query ) )
    {
        return( TRUE );
    }


    s32 iPath = Query.PushPath( "ForceField\\" );
    if( Query.VarFloat("Width", m_Width ) )
    {
        if( !Query.IsRead() )
        {
            OnMove( GetPosition() );
        }

        CreateVertices();
        return TRUE;
    }
    else if( Query.VarFloat("Height", m_Height ) )
    {
        if( !Query.IsRead() )
        {
            OnMove( GetPosition() );
        }
        CreateVertices();
        return TRUE;
    }

    if( Query.VarBool( "ActiveWhenFriendlyAll", m_bActiveWhenFriendlyAll ) )
    {
        return TRUE;
    }

    if( Query.IsVar( "MainGraphic" ) && Query.IsRead() )
    {
        Query.SetVarExternal( PRELOAD_MP_FILE( "ForceField.xbmp" ), RESOURCE_NAME_SIZE );
        return( TRUE );
    }
    if( Query.IsVar( "ScanGraphic" ) && Query.IsRead() )
    {
        Query.SetVarExternal( PRELOAD_MP_FILE( "ForceFieldCloud.xbmp" ), RESOURCE_NAME_SIZE );
        return( TRUE );
    }

    Query.PopPath( iPath );

    return( FALSE );
}

//==============================================================================

void force_field::OnPain( const pain& Pain )
{
    vector3 Point( Pain.GetPosition() );
    OnProjectileImpact( Point );
}

//==============================================================================