//==============================================================================
//
//  Cloth.cpp
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================
#include "Cloth.hpp"
#include "Entropy.hpp"
#include "Gamelib\StatsMgr.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "Render\LightMgr.hpp"
#include "PainMgr\Pain.hpp"
#include "Objects\Actor\Actor.hpp"
#include "GameLib\RenderContext.hpp"

#if defined(TARGET_PS2)
#include "Entropy\PS2\ps2_misc.hpp"
#endif

#ifdef TARGET_XBOX
extern view draw_GetViewFromRenderTarget( const view& );
extern void draw_SetMatrices( const view* );
#endif

//==============================================================================
// DEFINES
//==============================================================================

#define CLOTH_PRIM_KEY_TRI_FLAG     0x80000000      // Indicates flag tri was hit



//==============================================================================
// STATIC DATA
//==============================================================================
byte cloth::m_DamageClut[4*16] PS2_ALIGNMENT(16) ;    // Clut for damage texture


//==============================================================================
// DATA
//==============================================================================
static f32      CLOTH_TIME_STEP                     = 1.0f / 30.0f;
static xbool    CLOTH_DAMAGE                        = TRUE;
static f32      CLOTH_PAIN_EXPLOSION_FORCE_SCALE    = 200.0f;
static f32      CLOTH_PAIN_MELEE_FORCE_SCALE        = 500.0f;


#ifdef X_EDITOR

extern xbool g_game_running;

#endif

//==============================================================================
// CLASSES
//==============================================================================

cloth::cloth()
{
    // Physics components (xarrays only grow on the PC!)
    m_Particles.SetGrowAmount( 128 );
    m_Connections.SetGrowAmount( 256 );
    m_Triangles.SetGrowAmount( 256 );
    m_MinDist = F32_MAX;
    m_LocalCollBBox.Set(vector3(0,0,0), 100*4);
    m_WorldCollBBox = m_LocalCollBBox;

    // Physics properties
    m_Gravity.Set( 0.0f, -9.8f * 100.0f * 2.0f, 0.0f ); // Gravity to add to cloth
    m_Dampen = 0.05f;                                   // Dampen amount
    m_Stretch = 1.0f;                                   // Stretch amount
    m_nIterations = 1;                                  // # of constraint iterations
    m_ImpactScale = 0.001f;                             // Impact scale of bullets
    m_SimulationTime = 5.0f;                            // Time to simulate before level starts

    // Rendering components
    m_RenderMask = (u64)-1;
    m_MaterialIndex = -1;
    m_DeltaTime  = 0;
    m_ObjectGuid = 0;
    m_L2W.Identity();
    m_W2L.Identity();
    m_LocalBBox.Set(vector3(0,0,0), 0);
    m_WorldBBox.Set(vector3(0,0,0), 0);
    m_LightAmbColor.Set( (u8)(255.0f * 0.2f), (u8)(255.0f * 0.2f), (u8)(255.0f * 0.2f) ); 
    m_LightDirAmount = 1.0f;

    // Wind vars
    m_bWind = FALSE;                // Wind on or off?
    m_WindTimer = 0.0f;             // Time of on/off
    m_WindDir.Set( 0, 0, 1 );       // Direction of wind
    m_WindRot.Set( 0, R_20, 0 );    // Rotation of wind
    m_WindMinOnTime = 3.0f;         // Min amount of time wind is on
    m_WindMaxOnTime = 5.0f;         // Max amount of time wind is on
    m_WindMinOffTime = 3.0f;        // Min amount of time wind is off
    m_WindMaxOffTime = 5.0f;        // Max amount of time wind is off
    m_WindMinStrength = -0.8f;      // Min strength of wind
    m_WindMaxStrength = 0.8f;       // Max strength of wind
}

//==============================================================================

cloth::~cloth()
{
}

//===========================================================================

void cloth::OnEnumProp( prop_enum& List )
{
    // Cloth
    List.PropEnumHeader ( "Cloth", "Properties of cloth", 0 );

    // Geometry
    List.PropEnumHeader ( "RenderInst", "Render Instance", 0 );
    List.PropEnumHeader ( "Cloth\\Geometry", "Render inst properties of cloth", PROP_TYPE_DONT_SHOW );
    List.PropEnumInt    ( "Cloth\\Geometry\\ParticleCount",   "# of particles in cloth",   PROP_TYPE_DONT_SHOW );
    List.PropEnumInt    ( "Cloth\\Geometry\\ConnectionCount", "# of connections in cloth", PROP_TYPE_DONT_SHOW );
    List.PropEnumInt    ( "Cloth\\Geometry\\TriangleCount",   "# of triangles in cloth",   PROP_TYPE_DONT_SHOW );

    // Rigid inst
    // NOTE: This MUST come after geometry so xarray capacities are setup,
    //       but before all other properties so particle, connections, and triangles are initialized
    m_RigidInst.OnEnumProp ( List );

    // Particles
    List.PropEnumHeader( "Cloth\\Particles", "Particle properties of cloth", 0 );
#ifdef X_EDITOR    

    // Compute bbox of particles
    bbox BBox;
    BBox.Clear();
    for ( s32 i = 0; i < m_Particles.GetCount(); i++ )
        BBox += m_Particles[i].m_BindPos;
    vector3 Size = BBox.GetSize();
    
    // Add buttons
    List.PropEnumButton( "Cloth\\Particles\\Reset",        "Sets all masses to 1", PROP_TYPE_MUST_ENUM );
    if( Size.GetX() > 10.0f )
    {
        List.PropEnumButton( "Cloth\\Particles\\PegLeftSide",  "Sets masses of left (-ve X) side to 0", PROP_TYPE_MUST_ENUM );
        List.PropEnumButton( "Cloth\\Particles\\PegRightSide", "Sets masses of right (+ve X) side to 0", PROP_TYPE_MUST_ENUM );
    }
    if( Size.GetY() > 10.0f )
    {
        List.PropEnumButton( "Cloth\\Particles\\PegTopSide",   "Sets masses of top (+ve Y) side to 0", PROP_TYPE_MUST_ENUM );
        List.PropEnumButton( "Cloth\\Particles\\PegBotSide",   "Sets masses of bottom (-ve Y) side to 0", PROP_TYPE_MUST_ENUM );
    }
    if( Size.GetZ() > 10.0f )
    {
        List.PropEnumButton( "Cloth\\Particles\\PegFrontSide", "Sets masses of front (-ve Z) side to 0", PROP_TYPE_MUST_ENUM );
        List.PropEnumButton( "Cloth\\Particles\\PegBackSide",  "Sets masses of back (+ve Z) side to 0", PROP_TYPE_MUST_ENUM );
    }        
#endif
    for ( s32 i = 0; i < m_Particles.GetCount(); i++ )
    {
        List.PropEnumVector3( xfs( "Cloth\\Particles\\Position[%d]", i ), "Position",                  PROP_TYPE_DONT_SHOW );
        List.PropEnumFloat  ( xfs( "Cloth\\Particles\\Mass[%d]",     i ), "Mass (set to zero to pin)", PROP_TYPE_MUST_ENUM );
    }
    
    // Lighting
    List.PropEnumHeader( "Cloth\\Lighting", "Lighting properties of cloth", 0 );
    List.PropEnumColor ( "Cloth\\Lighting\\AmbColor",  "Ambient lighting color of cloth", 0 );
    List.PropEnumFloat ( "Cloth\\Lighting\\DirAmount", "Amount of directional lighting to recieve", 0 );
    
    // Wind vars
    List.PropEnumHeader  ( "Cloth\\Wind", "Wind properties of cloth", 0 );
    List.PropEnumVector3 ( "Cloth\\Wind\\Dir",         "Direction in local space (relative to object)", 0 );
    List.PropEnumRotation( "Cloth\\Wind\\Rot",         "Local space rotation (per second) of wind direction", 0 );
    List.PropEnumFloat   ( "Cloth\\Wind\\MinOnTime",   "Minimum amount of time wind is active", 0 );
    List.PropEnumFloat   ( "Cloth\\Wind\\MaxOnTime",   "Maximum amount of time wind is active", 0 );
    List.PropEnumFloat   ( "Cloth\\Wind\\MinOffTime",  "Minimum amount of time wind is inactive", 0 );
    List.PropEnumFloat   ( "Cloth\\Wind\\MaxOffTime",  "Maximum amount of time wind is inactive", 0 );
    List.PropEnumFloat   ( "Cloth\\Wind\\MinStrength", "Maximum strength of wind", 0 );
    List.PropEnumFloat   ( "Cloth\\Wind\\MaxStrength", "Minimum strength of wind", 0 );
    
    // Physics properties
    List.PropEnumHeader ( "Cloth\\Physics", "Physical properties of cloth", 0 );
    List.PropEnumVector3( "Cloth\\Physics\\Gravity",       "Gravity to apply to cloth", 0 );
    List.PropEnumFloat  ( "Cloth\\Physics\\Dampen",        "Dampen factor of constraints", 0 );
    List.PropEnumFloat  ( "Cloth\\Physics\\Stretch",       "Stretch factor of cloth", 0 );
    //SB - I don't feel comfortable exposing these to artists!
    //List.PropEnumInt    ( "Cloth\\Physics\\Iterations",    "# of constraint iterations to perform", 0 );
    //List.PropEnumFloat  ( "Cloth\\Physics\\ImpactScale",   "Impact scale of bullet force", 0 );
    List.PropEnumBBox   ( "Cloth\\Physics\\CollisionBBox",  "Local collision bounding box to keep cloth inside of", 0 ) ;
    List.PropEnumFloat  ( "Cloth\\Physics\\SimulationTime", "Physics time to simulate before level starts.", PROP_TYPE_MUST_ENUM);
}

//=============================================================================

xbool cloth::OnProperty( prop_query& I )
{
    // Rigid inst?
    if( I.IsSimilarPath( "RenderInst" ) )
    {
        if( m_RigidInst.OnProperty( I ) )
        {
            if( I.IsVar( "RenderInst\\File" ) )
            {
                // Setup cloth?
                if( I.IsRead() == FALSE )
                    Init();
            }

            return TRUE ;
        }
    }
    
    // Quick exit?
    if( I.IsSimilarPath( "Cloth" ) == FALSE )
        return FALSE;
    
    // Geometry? (used to pre-allocate xarrays)
    if( I.IsSimilarPath( "Cloth\\Geometry" ) )
    {    
        if( I.IsVar( "Cloth\\Geometry\\ParticleCount" ) )
        {
            if( I.IsRead() )
                I.SetVarInt( m_Particles.GetCount() );
            else            
                m_Particles.SetCapacity( I.GetVarInt() );
            return TRUE;            
        }
        if( I.IsVar( "Cloth\\Geometry\\ConnectionCount" ) )
        {
            if( I.IsRead() )
                I.SetVarInt( m_Connections.GetCount() );
            else            
                m_Connections.SetCapacity( I.GetVarInt() );
            return TRUE;            
        }
        if( I.IsVar( "Cloth\\Geometry\\TriangleCount" ) )
        {
            if( I.IsRead() )
                I.SetVarInt( m_Triangles.GetCount() );
            else            
                m_Triangles.SetCapacity( I.GetVarInt() );
            return TRUE;            
        }
    }
        
    // Lighting properties?
    if( I.IsSimilarPath( "Cloth\\Lighting" ) )
    {    
        if( I.VarColor( "Cloth\\Lighting\\AmbColor", m_LightAmbColor ) )
            return TRUE;
        if( I.VarFloat( "Cloth\\Lighting\\DirAmount", m_LightDirAmount, 0.0f, 1.0f ) )
            return TRUE;
    }
    
    // Wind properties?
    if( I.IsSimilarPath( "Cloth\\Wind" ) )
    {
        if( I.VarVector3( "Cloth\\Wind\\Dir", m_WindDir ) )
            return TRUE;
        if( I.VarRotation( "Cloth\\Wind\\Rot", m_WindRot ) )
            return TRUE;
        if( I.VarFloat( "Cloth\\Wind\\MinOnTime", m_WindMinOnTime ) )
            return TRUE;
        if( I.VarFloat( "Cloth\\Wind\\MaxOnTime", m_WindMaxOnTime ) )
            return TRUE;
        if( I.VarFloat( "Cloth\\Wind\\MinOffTime", m_WindMinOffTime ) )
            return TRUE;
        if( I.VarFloat( "Cloth\\Wind\\MaxOffTime", m_WindMaxOffTime ) )
            return TRUE;
        if( I.VarFloat( "Cloth\\Wind\\MinStrength", m_WindMinStrength ) )
            return TRUE;
        if( I.VarFloat( "Cloth\\Wind\\MaxStrength", m_WindMaxStrength ) )
            return TRUE;
    }
    
    // Particle properties?
    if( I.IsSimilarPath( "Cloth\\Particles" ) )
    {
    #ifdef X_EDITOR    
        
        // Reset mass?
        if( I.IsVar( "Cloth\\Particles\\Reset" ) )
        {
            // Setup UI?
            if( I.IsRead() )
            {
                I.SetVarButton( "Reset" );
            }
            else
            {
                // Clear all masses
                ClearAllInvMasses();
            }
            
            return TRUE;
        }
        
        // Peg left side?
        if( I.IsVar( "Cloth\\Particles\\PegLeftSide" ) )
        {
            if( I.IsRead() )
                I.SetVarButton( "Peg Left Side" );
            else
                PegParticles( 0, -1 );   // X, -ve dir
            return TRUE;
        }

        // Peg right side?
        if( I.IsVar( "Cloth\\Particles\\PegRightSide" ) )
        {
            if( I.IsRead() )
                I.SetVarButton( "Peg Right Side" );
            else
                PegParticles( 0, +1 );   // X, +ve dir
            return TRUE;
        }

        // Peg top side?
        if( I.IsVar( "Cloth\\Particles\\PegTopSide" ) )
        {
            if( I.IsRead() )
                I.SetVarButton( "Peg Top Side" );
            else
                PegParticles( 1, 1 );   // Y, +ve dir
            return TRUE;
        }
        
        // Peg bottom side?
        if( I.IsVar( "Cloth\\Particles\\PegBotSide" ) )
        {
            if( I.IsRead() )
                I.SetVarButton( "Peg Bottom Side" );
            else
                PegParticles( 1, -1 );   // Y, -ve dir
            return TRUE;
        }

        // Peg front side?
        if( I.IsVar( "Cloth\\Particles\\PegFrontSide" ) )
        {
            if( I.IsRead() )
                I.SetVarButton( "Peg Front Side" );
            else
                PegParticles( 2, -1 );   // Z, -ve dir
            return TRUE;
        }

        // Peg right side?
        if( I.IsVar( "Cloth\\Particles\\PegBackSide" ) )
        {
            if( I.IsRead() )
                I.SetVarButton( "Peg Back Side" );
            else
                PegParticles( 2, +1 );   // Z, +ve dir
            return TRUE;
        }
    #endif  //#ifdef X_EDITOR    

        // NOTE: This particle index is static so that when loading the mass, 
        // the PS2/XBOX can use this re-mapped index (since they are different than the PC)
        static s32 iParticle = -1;

        // Particle position?
        if( I.IsVar( "Cloth\\Particles\\Position[]" ) )
        {
            // Setup UI?
            if( I.IsRead() )
            {
                // Read position from particle
                iParticle = I.GetIndex(0);
                ASSERT( iParticle >= 0 );
                ASSERT( iParticle < m_Particles.GetCount() );
                I.SetVarVector3( m_Particles[iParticle].m_BindPos );        
            }
            else
            {
                // Search for particle index ready for setting mass property
                // (search is based on position since vertex indices are different on PC/PS2/XBOX)
                iParticle = FindParticle( I.GetVarVector3() );
            }
                
            return TRUE;
        }

        // Particle mass?    
        if( I.IsVar( "Cloth\\Particles\\Mass[]" ) )
        {
            // Setup UI?
            if( I.IsRead() )
            {
                // Read mass from PC particle
                iParticle = I.GetIndex(0);
                ASSERT( iParticle >= 0 );
                ASSERT( iParticle < m_Particles.GetCount() );
                I.SetVarFloat( m_Particles[iParticle].GetMass() );
            }
            else
            {
    #ifdef X_EDITOR
                // Use UI index when editing in the editor
                iParticle = I.GetIndex(0);
    #endif        
                // Set mass particle
                if( ( iParticle >= 0 ) && ( iParticle < m_Particles.GetCount() ) )
                    m_Particles[ iParticle ].SetMass( I.GetVarFloat() );
            }
            return TRUE;
        }
    }
     
     
    // Physics properties?
    if( I.IsSimilarPath( "Cloth\\Physics" ) )
    {    
        if( I.VarVector3( "Cloth\\Physics\\Gravity", m_Gravity ) )
            return TRUE;
        if( I.VarFloat( "Cloth\\Physics\\Dampen", m_Dampen, 0.0f, 2.0f ) )
            return TRUE;
        if( I.VarFloat( "Cloth\\Physics\\Stretch", m_Stretch, 0.0f, 1.0f ) )
            return TRUE;
        //if( I.VarInt( "Cloth\\Physics\\Iterations", m_nIterations, 1, 10 ) )
        //return TRUE;
        //if( I.VarFloat( "Cloth\\Physics\\ImpactScale", m_ImpactScale, 0.0f, 100.0f ) )
        //return TRUE;
        if( I.IsVar( "Cloth\\Physics\\CollisionBBox" ) )
        {
            if( I.IsRead() )
            {
                I.SetVarBBox( m_LocalCollBBox ) ;
            }
            else
            {
                SetLocalCollBBox( I.GetVarBBox() ) ;
            }

            return TRUE ;
        }

        if( I.VarFloat( "Cloth\\Physics\\SimulationTime", m_SimulationTime, 0.0f, 10.0f ) )
        {
#ifdef X_EDITOR
            // If in the editor and not running the game, leave in bind pose
            if( !g_game_running )
                return TRUE;
#endif            
            // Simulate physics for set time so flags don't visibly fall down when level starts
            f32 Step = 1.0f / 30.0f;
            for( f32 Time = 0; Time < m_SimulationTime; Time += Step )
                Advance( Step );
            
            return TRUE;
        }            
    }
        
    return FALSE;
}

 
//==============================================================================
// Private initialization functions
//==============================================================================

// Returns index of particle if found
s32 cloth::FindParticle( const vector3& P )
{
    // See if the particle already exists
    for (s32 i = 0; i < m_Particles.GetCount(); i++)
    {
        // Compute distance between particle and position
        vector3 Delta   = m_Particles[i].m_BindPos - P;
        f32     DistSqr = Delta.LengthSquared();
        
        // If below threshold distance, particle has been found
        if( DistSqr < x_sqr( 0.01f ) )
            return i;        
    }

    // Not found
    return -1;
}

//==============================================================================

// Adds particle and returns index
s32 cloth::AddParticle( const vector3& P, const vector2& UV )
{
    // Already exist?
    s32 Index = FindParticle(P);
    if (Index != -1)
        return Index;

    // Create new particle
    Index = m_Particles.GetCount();
    cloth_particle& Particle = m_Particles.Append();

    // Setup particle
    Particle.m_BindPos = P;
    Particle.m_Pos     = Particle.m_LastPos = m_L2W * P;
    Particle.m_InvMass = 1.0f;
    Particle.m_UV      = UV;

    return Index;
}

//==============================================================================

// Returns index of connection, or -1 if not found
s32 cloth::FindConnection( s32 ParticleA, s32 ParticleB )
{
    // Does connection already exist?
    for (s32 i = 0; i < m_Connections.GetCount(); i++)
    {
        cloth_connection& Connection = m_Connections[i];

        // Found?
        if (        (Connection.m_ParticleA == ParticleA)
                &&  (Connection.m_ParticleB == ParticleB) )
            return i;

        // Found?
        if (        (Connection.m_ParticleB == ParticleA)
                &&  (Connection.m_ParticleA == ParticleB) )
            return i;
    }

    // Not found
    return -1;
}

//==============================================================================

// Adds connection between 2 particles and returns index
s32 cloth::AddConnection( s32 ParticleA, s32 ParticleB )
{
    // Does connection already exist?
    s32 Index = FindConnection(ParticleA, ParticleB);
    if (Index != -1)
        return Index;

    // Create new connection
    Index = m_Connections.GetCount();
    cloth_connection& Connection = m_Connections.Append();

    // Setup connection
    Connection.m_ParticleA   = ParticleA;
    Connection.m_ParticleB   = ParticleB;
    Connection.m_RestDistSqr = (m_Particles[ParticleA].m_BindPos - m_Particles[ParticleB].m_BindPos).LengthSquared();

    // Update the min distance squared (it's used for self intersection)
    if( Connection.m_RestDistSqr > 0.001f )
        m_MinDist = x_min( m_MinDist, x_sqrt( Connection.m_RestDistSqr ) );

    return Index;
}

//==============================================================================

s32 cloth::FindTriangleVert( s32 Triangle, s32 Vertex )
{
    // Check all verts
    for (s32 i = 0; i < 3; i++)
    {
        // Found?
        if (m_Triangles[Triangle].m_Particles[i] == Vertex)
            return i;
    }

    // Not found
    return -1;
}

//==============================================================================

// Returns index of triangle or -1 if not found
s32 cloth::FindTriangle( s32 Verts[3] )
{
    // Check all triangles
    for (s32 i = 0; i < m_Triangles.GetCount(); i++)
    {
        // Found all three verts?
        if (     (FindTriangleVert(i, Verts[0]) != -1)
              && (FindTriangleVert(i, Verts[1]) != -1)
              && (FindTriangleVert(i, Verts[2]) != -1) )
        {
            return i;
        }
    }

    // Not found
    return -1;
}

//==============================================================================

// Adds triangle particles and connections
void cloth::AddTriangle( const vector3& P0, const vector2& UV0,
                         const vector3& P1, const vector2& UV1,
                         const vector3& P2, const vector2& UV2 )
{
    s32 i;

    // Add particles
    s32 Verts[3];
    Verts[0] = AddParticle(P0, UV0);
    Verts[1] = AddParticle(P1, UV1);
    Verts[2] = AddParticle(P2, UV2);

    // Ran out of particles?
    if ( ( Verts[0] == -1 ) || ( Verts[1] == -1 ) || ( Verts[2] == -1 ) )
        return ;

    // Create new triangle?
    if (FindTriangle(Verts) == -1)
    {
        cloth_triangle& Triangle = m_Triangles.Append();
        Triangle.m_Particles[0] = Verts[0];
        Triangle.m_Particles[1] = Verts[1];
        Triangle.m_Particles[2] = Verts[2];

        // Find the diagonal edge of the triangle
        f32 DiagDist = 0;
        s32 DiagEdge = 0;
        for (i = 0; i < 3; i++)
        {
            // Longest edge so far?
            f32 Dist = (m_Particles[Verts[i]].m_BindPos - m_Particles[Verts[(i+1)%3]].m_BindPos).LengthSquared();
            if (Dist > DiagDist)
            {
                // Record
                DiagDist = Dist;
                DiagEdge = i;
            }
        }
        
        // Loop through all edges and create connections
        for (i = 0; i < 3; i++)
        {
            // Skip the diagonal edge - makes the cloth much more flexible
            if (i != DiagEdge)
                AddConnection(Verts[i], Verts[(i+1)%3]);
        }
    }
}

//==============================================================================
// Internal connection sort data/functions
//==============================================================================

static cloth_particle* s_pClothParticles = NULL;
static s32             s_nClothParticles = 0;

static
s32 ConnectionSortFn( const void* pItem0, const void* pItem1 )
{
    // Lookup connections
    cloth_connection* pCon0 = (cloth_connection*)pItem0;
    cloth_connection* pCon1 = (cloth_connection*)pItem1;
    
    // Make sure connections are valid
    ASSERT( s_nClothParticles );
    ASSERT( s_pClothParticles );
    ASSERT( pCon0 );
    ASSERT( pCon1 );
    ASSERT( pCon0->m_ParticleA >= 0 );
    ASSERT( pCon0->m_ParticleA < s_nClothParticles );
    ASSERT( pCon0->m_ParticleB >= 0 );
    ASSERT( pCon0->m_ParticleB < s_nClothParticles );
    ASSERT( pCon1->m_ParticleA >= 0 );
    ASSERT( pCon1->m_ParticleA < s_nClothParticles );
    ASSERT( pCon1->m_ParticleB >= 0 );
    ASSERT( pCon1->m_ParticleB < s_nClothParticles );
    
    // Lookup particles
    cloth_particle& Con0PartA = s_pClothParticles[ pCon0->m_ParticleA ];
    cloth_particle& Con0PartB = s_pClothParticles[ pCon0->m_ParticleB ];
    cloth_particle& Con1PartA = s_pClothParticles[ pCon1->m_ParticleA ];
    cloth_particle& Con1PartB = s_pClothParticles[ pCon1->m_ParticleB ];

    // Compute mid position of connections
    f32 Con0MidY = ( Con0PartA.m_BindPos.GetY() + Con0PartB.m_BindPos.GetY() ) * 0.5f;
    f32 Con1MidY = ( Con1PartA.m_BindPos.GetY() + Con1PartB.m_BindPos.GetY() ) * 0.5f;
    
    // Sort by Y component from top(bigger) -> lower(smaller)
    if( Con0MidY < Con1MidY )
        return 1;
    else if( Con0MidY > Con1MidY )
        return -1;
    else
        return 0;
}

//==============================================================================
// Public initialization functions
//==============================================================================

void cloth::Init( void )
{
#if defined(TARGET_PC) || defined(TARGET_PS2)  || defined TARGET_XBOX
    s32 i,j;
#else
    s32 i;
#endif

    // Lookup geometry
    rigid_geom* pGeom = m_RigidInst.GetRigidGeom();
    if (!pGeom)
        return;

    // Setup default collision bbox
    vector3 C = pGeom->m_BBox.GetCenter();
    f32     R = pGeom->m_BBox.GetRadius();
    m_LocalCollBBox.Set( C, R * 1.25f );
    
    // Clear lists
    m_Particles.SetCount( 0 );
    m_Connections.SetCount( 0 );
    m_Triangles.SetCount( 0 );

    texture* pTexture = NULL;

    // Loop through all submeshes
    for (i = 0; i < pGeom->m_nMeshes; i++)
    {
        // Skip if not "cloth" mesh
        if ( x_stristr(pGeom->GetMeshName( i ), "cloth") == NULL )
            continue;

        // Do not draw this part of the rigid instance
        m_RenderMask &= ~(1<<i);

        if ( pGeom->m_pMesh[i].nSubMeshes > 1 )
            x_throw( "Cloth meshes should only use one texture" );

        // Lookup submesh
        geom::submesh&  SubMesh = pGeom->m_pSubMesh[pGeom->m_pMesh[i].iSubMesh];

        // Lookup material
        geom::material& Material = pGeom->m_pMaterial[SubMesh.iMaterial];

        // Lookup diffuse texture
        const char* pTexName = pGeom->GetTextureName( Material.iTexture );

        // Skip if not "flag" or "cloth" texture
        if (        (x_stristr(pTexName, "flag") == NULL)
                &&  (x_stristr(pTexName, "cloth") == NULL) )
            continue;

        // Save out the material index so we can do texture lookups later on.
        m_MaterialIndex = SubMesh.iMaterial;

        // grab out pointer to the texture to initialize the damage bitmap later
        texture::handle TexHandle;
        TexHandle.SetName( pTexName );
        pTexture = TexHandle.GetPointer();

#ifdef TARGET_PC
        // Lookup the display list
        rigid_geom::dlist_pc& DList = pGeom->m_System.pPC[SubMesh.iDList];

        // Loop through all tri indices
        ASSERT((DList.nIndices % 3) == 0);
        for (j = 0; j < DList.nIndices; j += 3)
        {
            // Lookup tri indices
            s32 V0 = DList.pIndices[j+0];
            s32 V1 = DList.pIndices[j+1];
            s32 V2 = DList.pIndices[j+2];

            // Add tri
            AddTriangle(DList.pVert[V0].Pos, DList.pVert[V0].UV,
                DList.pVert[V1].Pos, DList.pVert[V1].UV,
                DList.pVert[V2].Pos, DList.pVert[V2].UV);
        }
#endif

#ifdef TARGET_XBOX
        // Lookup the display list
        rigid_geom::dlist_xbox& DList = pGeom->m_System.pXbox[SubMesh.iDList];

        // Loop through all tri indices
        ASSERT((DList.nIndices % 3) == 0);
        for (j = 0; j < DList.nIndices; j += 3)
        {
            // Lookup tri indices
            s32 V0 = DList.pIndices[j+0];
            s32 V1 = DList.pIndices[j+1];
            s32 V2 = DList.pIndices[j+2];

            // Add tri
            AddTriangle(DList.pVert[V0].Pos, DList.pVert[V0].UV,
                DList.pVert[V1].Pos, DList.pVert[V1].UV,
                DList.pVert[V2].Pos, DList.pVert[V2].UV);
        }
#endif

#ifdef TARGET_PS2
        // Lookup the display list
        rigid_geom::dlist_ps2& DList = pGeom->m_System.pPS2[SubMesh.iDList];

        // Loop through all verts
        s32 Winding = 1;
        for (j = 0; j < (DList.nVerts-2); j++)
        {
            // Add this tri from the strip?
            u32& ADC = *(u32*)&(DList.pPosition[j+2].GetW());
            if ((ADC & (1<<15)) == 0)
            {
                // Lookup 3 verts that make up tri
                vector3 P0,P1,P2 ;
                vector2 UV0, UV1, UV2 ;
                P0 = DList.pPosition[j+0];
                P1 = DList.pPosition[j+1];
                P2 = DList.pPosition[j+2];
                UV0.Set((f32)DList.pUV[((j+0)*2)+0] / (1<<12), (f32)DList.pUV[((j+0)*2)+1] / (1<<12) ); 
                UV1.Set((f32)DList.pUV[((j+1)*2)+0] / (1<<12), (f32)DList.pUV[((j+1)*2)+1] / (1<<12) ); 
                UV2.Set((f32)DList.pUV[((j+2)*2)+0] / (1<<12), (f32)DList.pUV[((j+2)*2)+1] / (1<<12) ); 

                // Add tri with correct winding (since it's used for computing normals)
                if (Winding)
                {
                    AddTriangle(P0, UV0, 
                                P1, UV1, 
                                P2, UV2);
                }
                else
                {
                    AddTriangle(P2, UV2, 
                                P1, UV1, 
                                P0, UV0);
                }

                // Flip winding
                Winding ^= 1;
            }
            else
            {
                // Start of new strip - reset winding
                Winding = 1;
            }
        }
#endif

    }

    // Sort connections from top -> bottom so cloth isn't as stretchy
    if( m_Connections.GetCount() )
    {
        // Sort
        s_pClothParticles = &m_Particles[0];
        s_nClothParticles = m_Particles.GetCount();
        x_qsort( &m_Connections[0], m_Connections.GetCount(), sizeof(cloth_connection), ConnectionSortFn );
        s_pClothParticles = NULL;
        s_nClothParticles = 0;
    }    

    // Default to pegging top particles
    PegParticles( 1, 1 );   // Y, +ve dir

    // Clear normals
    for (i = 0; i < m_Particles.GetCount(); i++)
    {
        // Lookup particle
        cloth_particle& Particle = m_Particles[i];

        // Set normal info to zero
        Particle.m_Normal.Zero();
    }

    // Loop through all triangles and accumulate normals into particles
    for (i = 0; i < m_Triangles.GetCount(); i++)
    {
        // Lookup triangle
        cloth_triangle& Triangle = m_Triangles[i];

        // Lookup particles
        cloth_particle& Particle0 = m_Particles[(s32)Triangle.m_Particles[0]];
        cloth_particle& Particle1 = m_Particles[(s32)Triangle.m_Particles[1]];
        cloth_particle& Particle2 = m_Particles[(s32)Triangle.m_Particles[2]];

        // Lookup particle positions
        const vector3& P0 = Particle0.m_BindPos;
        const vector3& P1 = Particle1.m_BindPos;
        const vector3& P2 = Particle2.m_BindPos;

        // Compute normal
        vector3 Normal = (P1-P0).Cross(P2-P0);

        // Accumulate into particles
        Particle0.m_Normal += Normal;
        Particle1.m_Normal += Normal;
        Particle2.m_Normal += Normal;
    }

#if defined TARGET_PS2 || defined TARGET_XBOX

    // Initialize damage bitmap
    if (pTexture)
    {
        // Get diffuse map info
        xbitmap& DiffuseBMP    = pTexture->m_Bitmap;

        // Setup damage bitmap
#ifdef TARGET_PS2

        s32      Width         = DiffuseBMP.GetWidth();/// 2;
        s32      Height        = DiffuseBMP.GetHeight();/// 2;
        s32      PixelDataSize = Width*Height/2;   // 4 bits per pixel

        // Allocate pixel data (clut data is static since it's the same for every flag)
        byte*    pData         = (byte*)x_malloc( PixelDataSize );
        byte*    pPixelData    = pData;
        byte*    pClutData     = m_DamageClut;

        xbitmap::format Format = xbitmap::FMT_P4_ABGR_8888;
#else
        s32      Width         = DiffuseBMP.GetWidth ();
        s32      Height        = DiffuseBMP.GetHeight();
        s32      PixelDataSize = Width*Height*4;

        // Allocate pixel data
        byte*    pData         = (byte*)x_malloc( PixelDataSize );
        byte*    pPixelData    = pData;
        byte*    pClutData     = NULL;

        xbitmap::format Format = xbitmap::FMT_32_ARGB_8888;
#endif
        m_DamageBMP.Setup( Format,      // Format
                           Width,       // Pixel width
                           Height,      // Pixel height
                           TRUE,        // DataOwned
                           pPixelData,  // pPixelData
                           FALSE,       // ClutOwned
                           pClutData);  // pClutData

        // Setup the palette data
#ifdef TARGET_XBOX
        // Setup the pixel data
        for (s32 y = 0; y < Height; y++)
        {
            for (s32 x = 0; x < Width; x++)
            {
                m_DamageBMP.SetPixelColor( xcolor( 255,255,255 ),x,y);    // Full bright, opaque
            }
        }
#else
        for (i = 0; i < 16; i++)
        {
            xcolor Color;
            Color.R = Color.G = Color.B = (u8)((f32)i * (255.0f / 15.0f));
            Color.A = (u8)((f32)i * (255.0f / 15.0f));
            m_DamageBMP.SetClutColor(Color, i);
        }

        // Setup the pixel data
        for (s32 y = 0; y < Height; y++)
        for (s32 x = 0; x < Width; x++)
            m_DamageBMP.SetPixelIndex(15, x,y);    // Full bright, opaque
#endif
        // Get ready for using
        vram_Register(m_DamageBMP);
    }
#endif

    // Reset motion
    Reset();
}

//===========================================================================

void cloth::PegParticles( s32 iAxis, s32 Dir )
{
    s32 i;

    ASSERT( iAxis >= 0 );
    ASSERT( iAxis <= 2 );
    ASSERT( Dir != 0 );
        
    // Compute bbox of particles
    bbox BBox;
    BBox.Clear();
    for ( i = 0; i < m_Particles.GetCount(); i++)
        BBox += m_Particles[i].m_BindPos;
    
    // Peg particles
    for( i = 0; i < m_Particles.GetCount(); i++ )
    {
        // Lookup particle
        cloth_particle& Particle = m_Particles[i];
        
        // Peg?
        xbool bPeg = FALSE;
        if( Dir > 0 )
            bPeg = ( Particle.m_BindPos[iAxis] >= ( BBox.Max[iAxis] - 10.0f ) );
        else
            bPeg = ( Particle.m_BindPos[iAxis] <= ( BBox.Min[iAxis] + 10.0f ) );

        // Set inverse mass to zero on particles that should be pegged
        if( bPeg )
            Particle.m_InvMass = 0.0f;
    }
}

//===========================================================================

void cloth::ClearAllInvMasses( void )
{
    // Everyone moves.
    for ( s32 i = 0; i < m_Particles.GetCount(); i++)
    {
        m_Particles[i].m_InvMass = 1.0f;
    }
}

//==============================================================================

void cloth::Reset( void )
{
    // Resets all particles to bind position
    for (s32 i = 0; i < m_Particles.GetCount(); i++)
        m_Particles[i].m_Pos = m_Particles[i].m_LastPos = m_L2W * m_Particles[i].m_BindPos;

    // Update bounds
    ComputeBounds();
    ComputeWorldCollBBox();

    // Apply constraints incase collision bbox is smaller than geometry
    ApplyConstraints();
}

//==============================================================================

void cloth::Kill( void )
{
#if defined TARGET_PS2 || defined TARGET_XBOX
    // Unregister if bitmap was registered
    if( m_DamageBMP.GetVRAMID() )
        vram_Unregister(m_DamageBMP);
    
    // Destroy damage bitmap
    m_DamageBMP.Kill();
#endif
}


//==============================================================================
// Physics functions
//==============================================================================

void cloth::ApplyCappedCylinderColl ( const vector3& Bottom, const vector3& Top, f32 Radius )
{
    CONTEXT("cloth::ApplyCappedCylinderColl");

    f32 RadiusSqr = Radius * Radius;

    // Loop through all particles
    for (s32 i = 0; i < m_Particles.GetCount(); i++)
    {
        // Lookup particle
        cloth_particle& Particle = m_Particles[i];

        // Skip if it cannot be moved
        if (Particle.m_InvMass == 0)
            continue;

        // Get vector and distance to line down middle of cylinder
        vector3 Delta   = Particle.m_Pos.GetClosestVToLSeg(Bottom, Top);
        f32     DistSqr = Delta.LengthSquared();

        // Project out of capped cylinder?
        if ((DistSqr > 0.00001f) && (DistSqr < RadiusSqr))
        {
            // Keep away from cylinder center line
            f32 Dist = x_sqrt(DistSqr);
            f32 Diff = (Dist - Radius) / Dist;

            // Scale and dampen
            Delta *= Diff;

            // Project particle out of cylinder
            Particle.m_Pos += Delta;
        }
    }
}

//==============================================================================

void cloth::OnColCheck( guid ObjectHitGuid, u32 Flags )
{
    CONTEXT( "cloth::OnColCheck" );

    g_CollisionMgr.StartApply( ObjectHitGuid );

    // Loop through all triangles
    for( s32 i = 0; i < m_Triangles.GetCount(); i++ )
    {
        // Lookup triangle
        cloth_triangle& Triangle = m_Triangles[i];

        // Lookup particle positions
        const vector3& P0 = m_Particles[(s32)Triangle.m_Particles[0]].m_Pos;
        const vector3& P1 = m_Particles[(s32)Triangle.m_Particles[1]].m_Pos;
        const vector3& P2 = m_Particles[(s32)Triangle.m_Particles[2]].m_Pos;

        // Apply both sides to collision manager - pass in triangle number | CLOTH_PRIM_KEY_TRI_FLAG
        // as the primitive key so that if a projectile hits the flag, we can identify the exact UV hit
        g_CollisionMgr.ApplyTriangle( P0, P1, P2, Flags, i | CLOTH_PRIM_KEY_TRI_FLAG );
        g_CollisionMgr.ApplyTriangle( P2, P1, P0, Flags, i | CLOTH_PRIM_KEY_TRI_FLAG );
    }

    g_CollisionMgr.EndApply();    
}

//==============================================================================

void cloth::ApplyDistConstraints( void )
{
    CONTEXT("cloth::ApplyDistConstraints");

    // Loop through all connections
    for (s32 i = 0; i < m_Connections.GetCount(); i++)
    {
        // Lookup connection
        cloth_connection& Connection = m_Connections[i];

        // Lookup particles and distance
        cloth_particle& ParticleA   = m_Particles[(s32)Connection.m_ParticleA];
        cloth_particle& ParticleB   = m_Particles[(s32)Connection.m_ParticleB];
        f32             RestDistSqr = Connection.m_RestDistSqr;

        // Lookup mass info and skip connection if both particles are rigid
        f32 InvMass0 = ParticleA.m_InvMass;
        f32 InvMass1 = ParticleB.m_InvMass;
        f32 TotalInvMass = InvMass0 + InvMass1;
        if (TotalInvMass < 0.000001f)
            continue;

        // Get distance between particles
        vector3 Delta   = ParticleB.m_Pos - ParticleA.m_Pos;
        f32     DistSqr = Delta.LengthSquared();

        // Move to target dist
        //f32 Dist = x_sqrt(DistSqr);
        //f32 Diff = (Dist - RestDist) / (Dist * TotalInvMass);

        // Using sqrt approx
        f32 Diff = -2.0f * ((RestDistSqr / (DistSqr + RestDistSqr)) - 0.5f) / TotalInvMass;

        // Scale and dampen
        Delta *= Diff * m_Stretch;

        // Apply deltas
        ParticleA.m_Pos += InvMass0 * Delta;
        ParticleB.m_Pos -= InvMass1 * Delta;
    }
}

//==============================================================================

void cloth::ApplyCollConstraints( void )
{
    CONTEXT("cloth::ApplyCollConstraints");

    s32 i, j;
    
    // Loop through all particles and keep them inside the collision bbox
    for (i = 0; i < m_Particles.GetCount(); i++)
    {
        // Lookup particle
        cloth_particle& Particle = m_Particles[i];

        // Skip if fixed
        if (Particle.m_InvMass == 0)
            continue;

        // Keep particle inside world collision bbox by projection
        Particle.m_Pos.Max( m_WorldCollBBox.Min );
        Particle.m_Pos.Min( m_WorldCollBBox.Max );
    }
    
    // Keep particles a set distance away from each other
    f32 RestDistSqr = x_sqr( m_MinDist );
    for (i = 0; i < m_Particles.GetCount(); i++)
    {
        cloth_particle& ParticleA = m_Particles[i];

        for (j = i+1; j < m_Particles.GetCount(); j++)
        {
            cloth_particle& ParticleB = m_Particles[j];

            // Lookup mass info and skip connection if both particles are rigid
            f32 InvMass0 = ParticleA.m_InvMass;
            f32 InvMass1 = ParticleB.m_InvMass;
            f32 TotalInvMass = InvMass0 + InvMass1;
            if (TotalInvMass < 0.000001f)
                continue;

            // Get distance between particles
            vector3 Delta   = ParticleB.m_Pos - ParticleA.m_Pos;
            f32     DistSqr = Delta.LengthSquared();
            if (DistSqr < RestDistSqr)
            {
                // Move to target dist
                //f32 Dist = x_sqrt(DistSqr);
                //f32 Diff = (Dist - RestDist) / (Dist * TotalInvMass);

                // Using sqrt approx
                f32 Diff = -2.0f * ((RestDistSqr / (DistSqr + RestDistSqr)) - 0.5f) / TotalInvMass;

                // Scale and dampen
                Delta *= Diff;//* m_Damp;

                // Apply deltas
                ParticleA.m_Pos += InvMass0 * Delta;
                ParticleB.m_Pos -= InvMass1 * Delta;
            }
        }
    }
}

//==============================================================================

void cloth::ApplyConstraints( void )
{
    CONTEXT("cloth::ApplyConstraints");

    s32 i;

    // Apply collision constraints
    ApplyCollConstraints();

    // Apply distance constraints - keeps the cloth held together
    for (i = 0; i < m_nIterations; i++)
        ApplyDistConstraints();
}

//==============================================================================

void cloth::Integrate( f32 DeltaTime )
{
    CONTEXT("cloth::Integrate");

    // Nothing to do?
    if (m_DeltaTime == 0)
        return;

    // Setup constants
    f32     DeltaTimeSquared = DeltaTime * DeltaTime;
    vector3 Accel            = m_Gravity * DeltaTimeSquared;

    // Apply verlet integration to all particles
    f32 Dampen = x_clamp( 1.0f - m_Dampen, 0.0f, 1.0f );
    for (s32 i = 0; i < m_Particles.GetCount(); i++)
    {
        // Lookup particle
        cloth_particle& Particle = m_Particles[i];

        // Compute movement
        vector3 Pos   = Particle.m_Pos;
        if (Particle.m_InvMass != 0)
        {
            // Compute velocity
            vector3 Vel = Dampen * ( Particle.m_Pos - Particle.m_LastPos );

            // Move!
            Particle.m_Pos += Vel + Accel;
        }

        // Update last position
        Particle.m_LastPos = Pos;
    }
}

//==============================================================================

void cloth::ComputeLighting( void )
{
    CONTEXT("cloth::ComputeLighting");

    s32 i;

    // Clear particle normals
    for (i = 0; i < m_Particles.GetCount(); i++)
        m_Particles[i].m_Normal.Zero();

    // Loop through all triangles and accumulate normals into particles
    for (i = 0; i < m_Triangles.GetCount(); i++)
    {
        // Lookup triangle
        cloth_triangle& Triangle = m_Triangles[i];

        // Lookup particles
        cloth_particle& Particle0 = m_Particles[(s32)Triangle.m_Particles[0]];
        cloth_particle& Particle1 = m_Particles[(s32)Triangle.m_Particles[1]];
        cloth_particle& Particle2 = m_Particles[(s32)Triangle.m_Particles[2]];

        // Lookup particle positions
        const vector3& P0 = Particle0.m_Pos;
        const vector3& P1 = Particle1.m_Pos;
        const vector3& P2 = Particle2.m_Pos;

        // Compute triangle normal
        vector3 Normal = (P1-P0).Cross(P2-P0);

        // Accumulate into particles normal
        Particle0.m_Normal += Normal;
        Particle1.m_Normal += Normal;
        Particle2.m_Normal += Normal;
    }

    // For correct brightness (since PS2 brightens x2 with damage texture on)
    f32 LightScale = 1.0f;
    f32 LightClamp = 255.0f;
    if (CLOTH_DAMAGE)
    {
        LightScale = 0.5f;
        LightClamp = 128.0f;
    }
    
    // Collect directional lights
    s32 nLights = g_LightMgr.CollectCharLights( m_L2W, m_LocalBBox, 3 );
    vector3 LightDirs[3];
    vector3 LightCols[3];
    xcolor  Col;
    i = 0;
    while( i < nLights )
    {
        g_LightMgr.GetCollectedCharLight( i, LightDirs[i], Col );
        LightDirs[i] *= -LightScale * m_LightDirAmount;
        LightCols[i].Set( (f32)Col.R, (f32)Col.G, (f32)Col.B );
        i++;
    }
    while( i < 3 )
    {
        LightDirs[i].Zero();
        LightCols[i].Zero();
        i++;
    }
        
    // Setup light direction and color matrices
    matrix4 LightDirsMat;
    matrix4 LightColsMat;
    LightDirsMat.Zero();
    LightColsMat.Zero();
    LightDirsMat.SetRows   ( LightDirs[0], LightDirs[1], LightDirs[2] );
    LightColsMat.SetColumns( LightCols[0], LightCols[1], LightCols[2] );

    // Setup ambient
    LightColsMat.SetTranslation( vector3( (f32)m_LightAmbColor.R * LightScale, 
                                          (f32)m_LightAmbColor.G * LightScale, 
                                          (f32)m_LightAmbColor.B * LightScale ) );

    // Normalize particle normals and compute color
    for (i = 0; i < m_Particles.GetCount(); i++)
    {
        // Lookup particle
        cloth_particle& Particle = m_Particles[i];

        // Compute final normal
        Particle.m_Normal.Normalize();

        // Compute intensity of each light on both sides of flag, keeping the brightest
        vector3 I = LightDirsMat * Particle.m_Normal;
        I.Max( -I );

        // Compute colors of each light + ambient       
        vector3 C = LightColsMat * I;
        C.Min( LightClamp );

        // Setup final color
        Particle.m_Color.R = (u8)C.GetX();
        Particle.m_Color.G = (u8)C.GetY();
        Particle.m_Color.B = (u8)C.GetZ();
        #ifdef TARGET_PC
            Particle.m_Color.A = 255;
        #else
            Particle.m_Color.A = 128;
        #endif            
    }
}

//==============================================================================

void cloth::ComputeBounds( void )
{
    CONTEXT("cloth::ComputeBounds");

    // Lookup geometry
    geom* pGeom = m_RigidInst.GetGeom();
    if (!pGeom)
    {
        m_LocalBBox.Set(vector3(0,0,0), 100);
        m_WorldBBox.Set(m_L2W.GetTranslation(), 100);
        return;
    }

    // Compute world bbox
    m_WorldBBox = pGeom->m_BBox;
    m_WorldBBox.Transform(m_L2W);
    for (s32 i = 0; i < m_Particles.GetCount(); i++)
        m_WorldBBox += m_Particles[i].m_Pos;

    // Always have thickness
    m_WorldBBox.Inflate(10,10,10);

    // Compute local bbox
    m_LocalBBox = m_WorldBBox;
    m_LocalBBox.Transform(m_W2L);
}

//==============================================================================

// Computes world space collision bbox
void cloth::ComputeWorldCollBBox( void )
{
    m_WorldCollBBox = m_LocalCollBBox;
    m_WorldCollBBox.Transform(m_L2W);
}

//==============================================================================

// Applies blast force to particles
void cloth::ApplyBlast( const vector3& Pos, f32 Radius, f32 Amount )
{
    // Compute radius squared just once ready for loop
    f32 RadiusSqr = Radius * Radius;
    
    // Bake in frame rate multiplier
    Amount *= CLOTH_TIME_STEP;
    
    // Loop through all particles
    for( s32 i = 0; i < m_Particles.GetCount(); i++ )
    {
        // Lookup particle
        cloth_particle& P = m_Particles[i];
        
        // Skip if particle is pinned
        if( P.m_InvMass == 0.0f )
            continue;
            
        // Compute distance squared from particles
        vector3 Delta   = P.m_Pos - Pos;
        f32     DistSqr = Delta.LengthSquared();
        
        // Within force radius and not right on top of particle?
        if( ( DistSqr < RadiusSqr ) && ( DistSqr > 0.001f ) )
        {
            // Compute distance from particle
            f32 Dist = x_sqrt( DistSqr );
                
            // Compute normalized direction to particle
            vector3 Dir = Delta * ( 1.0f / Dist );
            
            // Compute force ratio
            f32 Force = ( Radius - Dist ) / Radius;
            
            // Apply impulse to update particle velocity
            P.m_LastPos -= P.m_InvMass * Dir * Force * Amount;
        }
    }
}

//==============================================================================

// Applies directional blast force to particles
void cloth::ApplyBlast( const vector3& Pos, const vector3& Dir, f32 Radius, f32 Amount )
{
    // Compute radius squared just once ready for loop
    f32 RadiusSqr = Radius * Radius;

    // Bake in frame rate multiplier
    Amount *= CLOTH_TIME_STEP;

    // Loop through all particles
    for( s32 i = 0; i < m_Particles.GetCount(); i++ )
    {
        // Lookup particle
        cloth_particle& P = m_Particles[i];

        // Skip if particle is pinned
        if( P.m_InvMass == 0.0f )
            continue;

        // Compute distance squared from particles
        vector3 Delta   = P.m_Pos - Pos;
        f32     DistSqr = Delta.LengthSquared();

        // Within force radius?
        if( DistSqr < RadiusSqr )
        {
            // Compute distance from particle
            f32 Dist = 0.0f;
            if( DistSqr > 0.001f )
                Dist = x_sqrt( DistSqr );

            // Compute force ratio
            f32 Force = ( Radius - Dist ) / Radius;

            // Apply impulse to update particle velocity
            P.m_LastPos -= P.m_InvMass * Dir * Force * Amount;
        }
    }
}

//==============================================================================

// Applies pain blast to particles
void cloth::OnPain( const pain& Pain )
{
    // Pain come from melee?
    if( Pain.IsDirectHit() )
    {
        // Come from an actor's melee?
        object* pSource = g_ObjMgr.GetObjectByGuid( Pain.GetOriginGuid() );
        if( pSource->IsKindOf( actor::GetRTTI() ) )
        {
            // Apply melee force
            ApplyBlast( Pain.GetPosition(), 
                        Pain.GetDirection(), 
                        m_MinDist * 2.0f, 
                        Pain.GetForce() * CLOTH_PAIN_MELEE_FORCE_SCALE );
        }            
        return;
    }
            
    // Pain came from an explosion...
                
    // Lookup max force
    f32                        MaxForce           = 10.0f;
    pain_health_handle         hPainHealthHandle  = Pain.GetPainHealthHandle();
    const pain_health_profile* pPainHealthProfile = hPainHealthHandle.GetPainHealthProfile();
    if( pPainHealthProfile )
    {
        MaxForce = pPainHealthProfile->m_Force;
    }

    // Lookup force max radius
    f32                 ForceFarDist = 100.0f * 5.0f;
    pain_handle         hPain        = Pain.GetPainHandle();
    const pain_profile* pPainProfile = hPain.GetPainProfile();
    if( pPainProfile )
    {
        ForceFarDist = pPainProfile->m_ForceFarDist;
    }
        
    // Apply blast to particles
    ApplyBlast( Pain.GetPosition(), ForceFarDist, MaxForce * CLOTH_PAIN_EXPLOSION_FORCE_SCALE );
}

//==============================================================================

// Blend 4 bit color with destination color
inline void Blend4BitPixel( xbitmap& BMP, s32 X, s32 Y, s32 Color )
{
    // Lookup byte to write
    byte* pByte = (byte*)BMP.GetPixelData();
    pByte += X>>1;
    pByte += (BMP.GetWidth()>>1) * Y;

    // Which nibble?
    if (X & 1)
    {
        // Read pixel
        s32 Pixel = (((*pByte) & 0xF0) >> 4);

        // Blend
        Pixel = (Pixel * Color) >> 4;

        // Write pixel
        (*pByte) &= 0x0F; 
        (*pByte) |= (Pixel<<4);
    }
    else
    {
        // Read pixel
        s32 Pixel = ((*pByte) & 0x0F);

        // Blend
        Pixel = (Pixel * Color) >> 4;

        // Write pixel
        (*pByte) &= 0xF0; 
        (*pByte) |= Pixel;
    }
}

//==============================================================================

// Reads 4 bit color
inline s32 Read4BitPixel( xbitmap& BMP, s32 X, s32 Y )
{
    // Lookup byte to write
    byte* pByte = (byte*)BMP.GetPixelData();
    pByte += X>>1;
    pByte += (BMP.GetWidth()>>1) * Y;

    // Which nibble?
    s32 Pixel;
    if (X & 1)
    {
        // Read pixel
        Pixel = (((*pByte) & 0xF0) >> 4);
    }
    else
    {
        // Read pixel
        Pixel = ((*pByte) & 0x0F);
    }

    return Pixel;
}

//==============================================================================

// Punch a hole in the damage texture
// (Returns current status of given texture co-ord. 0=All punch thru, 1=All there)
s32 cloth::PunchDamage( f32 U, f32 V )
{
#ifdef TARGET_XBOX

    // Each sample in form (XOffset, YOffset, Color)
    static s32 Hole0[] =
    {
        // Punch thru star
        0,0,0,
        -1,0,0,
        +1,0,0,
        0,-1,0,
        0,+1,0,

        // Inner anti-aliasing
        -1,-1,7,
        +1,-1,7,
        +1,+1,7,
        -1,+1,7,

        // Outer anti-aliasing
        0,-2,13,
        0,+2,13,
        -2,0,13,
        +2,0,13,

        S32_MAX
    };

    static s32 Hole2[] =
    {
        0,0,0,
        1,0,0,
        1,1,0,
        0,1,0,

        0,-1,7,
        1,-1,7,
        -1,0,7,
        -1,1,7,
        2,0,7,
        2,1,7,
        0,2,7,
        1,2,7,

        -1,-1,13,
        -1,2,13,
        2,-1,13,
        2,2,13,

        S32_MAX
    };

    static s32 Hole1[] =
    {
        0,0,0,
        -1,0,7,
        +1,0,7,
        0,-1,7,
        0,+1,7,
        -1,-1,13,
        +1,-1,13,
        +1,+1,13,
        -1,+1,13,

        S32_MAX
    };

    static s32* Decals[] = {Hole0, Hole1, Hole2};

    // Lookup bitmap info
    s32 W = m_DamageBMP.GetWidth ();
    s32 H = m_DamageBMP.GetHeight();

    // Scale to pixel co-ords
    s32 X = (s32)(U * W);
    s32 Y = (s32)(V * H);

    // Select random decal
    s32* pDecal = Decals[x_irand(0, (sizeof(Decals) / sizeof(Decals[0]))-1)];

    // Read current pixel status

    xbitmap DamageBMP;
    DamageBMP.Setup( m_DamageBMP.GetFormat(),W,H,TRUE,NULL );

    texture_factory::handle Handle = vram_GetSurface( m_DamageBMP );
    void* Pixels = Handle->Lock(0);
    XGUnswizzleRect(
        Pixels,
        W,
        H,
        NULL,
        (void*)DamageBMP.GetPixelData(),
        W*4,
        NULL,
        4
    );
    xcolor C = DamageBMP.GetPixelColor( X,Y );
    s32 Status = C.A / 16;

    // Punch damage pixels
    while( pDecal[0] != S32_MAX )
    {
        // Compute bitmap pixel pos
        s32 DstX = X + pDecal[0];
        s32 DstY = Y + pDecal[1];

        // Only write if on bitmap
        if(( DstX >= 0 ) && ( DstX < W ) && ( DstY >= 0 ) && ( DstY < H ))
        {
            xcolor N = DamageBMP.GetPixelColor( DstX, DstY );

            u32 A = ((u32)N.A * (u32)pDecal[2])>>4;
            A = MINMAX( 0, A, 255 );
            N.Set( (u8)A, (u8)A, (u8)A, (u8)A );

            DamageBMP.SetPixelColor( N,DstX,DstY );
        }

        // Next pixel
        pDecal += 3;
    }
    XGSwizzleRect(
        DamageBMP.GetPixelData(),
        W*4,
        NULL,
        Pixels,
        W,
        H,
        NULL,
        4
    );
    Handle->Unlock();

    return Status;

#endif

#ifdef TARGET_PS2

    // Each sample in form (XOffset, YOffset, Color)
    static s32 Hole0[] =
    {
        // Punch thru star
        0,0,0,
        -1,0,0,
        +1,0,0,
        0,-1,0,
        0,+1,0,

        // Inner anti-aliasing
        -1,-1,7,
        +1,-1,7,
        +1,+1,7,
        -1,+1,7,

        // Outer anti-aliasing
        0,-2,13,
        0,+2,13,
        -2,0,13,
        +2,0,13,

        S32_MAX
    };

    static s32 Hole2[] =
    {
        0,0,0,
        1,0,0,
        1,1,0,
        0,1,0,

        0,-1,7,
        1,-1,7,
        -1,0,7,
        -1,1,7,
        2,0,7,
        2,1,7,
        0,2,7,
        1,2,7,

        -1,-1,13,
        -1,2,13,
        2,-1,13,
        2,2,13,
        
        S32_MAX
    };

    static s32 Hole1[] =
    {
        0,0,0,
        -1,0,7,
        +1,0,7,
        0,-1,7,
        0,+1,7,
        -1,-1,13,
        +1,-1,13,
        +1,+1,13,
        -1,+1,13,
        
        S32_MAX
    };

    static s32* Decals[] = {Hole0, Hole1, Hole2};

    // Lookup bitmap info
    s32 W = m_DamageBMP.GetWidth()-1;
    s32 H = m_DamageBMP.GetHeight()-1;

    // Scale to pixel co-ords
    s32 X = (s32)(U * W);
    s32 Y = (s32)(V * H);

    // Read current pixel status
    s32 Status = Read4BitPixel(m_DamageBMP, X,Y);

    // Select random decal
    s32* pDecal = Decals[x_irand(0, (sizeof(Decals) / sizeof(Decals[0]))-1)];

    // Punch damage pixels
    while(pDecal[0] != S32_MAX)
    {
        // Compute bitmap pixel pos
        s32 DstX = X + pDecal[0];
        s32 DstY = Y + pDecal[1];

        // Only write if on bitmap
        if ((DstX >= 0) && (DstX < W) && (DstY >= 0) && (DstY < H))
            Blend4BitPixel(m_DamageBMP, DstX,DstY, pDecal[2]);

        // Next pixel
        pDecal += 3;
    }

    return Status;
#else
    
    // PC - no damage bitmap present...
    (void)U;
    (void)V;
    return 15;

#endif
}

//==============================================================================

s32 cloth::CheckDamage( f32 U, f32 V )
{
#ifdef TARGET_PS2

    // Lookup bitmap info
    s32 W = m_DamageBMP.GetWidth ()-1;
    s32 H = m_DamageBMP.GetHeight()-1;

    // Scale to pixel co-ords
    s32 X = (s32)(U * W);
    s32 Y = (s32)(V * H);

    // Read current pixel status
    s32 Status = Read4BitPixel(m_DamageBMP, X,Y);

    return Status;

#elif defined TARGET_XBOX

    s32 W = m_DamageBMP.GetWidth ()-1;
    s32 H = m_DamageBMP.GetHeight()-1;

    // Scale to pixel co-ords
    s32 X = (s32)(U * W);
    s32 Y = (s32)(V * H);

    // Read current pixel status
    s32 Status = m_DamageBMP.GetPixelColor( X,Y );
    return Status/16;

#else
    
    // PC - no damage bitmap present...
    (void)U;
    (void)V;
    return 15;

#endif
}



//==============================================================================
//
//          P0         
//         /|\        
//        / | \       
//       /  |  \      
//      / 2 | 1 \     
//     /  _-TP-_ \
//    / _-      -_\
//   /_-    0     -\
//  P1--------------P2
//
void ComputeBarys( const vector3& P0, 
                   const vector3& P1, 
                   const vector3& P2, 
                   const vector3& TP, 
                         vector3& Bary )
{
    // Compute scaled normal
    vector3 Normal = v3_Cross(P1-P0, P2-P0);
    Normal *= 1.0f / Normal.LengthSquared();

    // Compute barycentric co-ords
    Bary.Set( v3_Cross(P2-P1, TP-P1).Dot(Normal),
              v3_Cross(P0-P2, TP-P2).Dot(Normal),
              v3_Cross(P1-P0, TP-P0).Dot(Normal) );
}

//==============================================================================

void cloth::OnProjectileImpact( const object&   ProjectileObject,
                                const vector3&  ProjectileVel,
                                      u32       CollPrimKey, 
                                const vector3&  CollPoint,
                                      xbool     PunchDamageHole,
                                      f32       ManualImpactForce )
{
    // Skip if not collision is not with cloth triangle
    if( ( CollPrimKey & CLOTH_PRIM_KEY_TRI_FLAG ) == 0 )
        return;
        
    // Lookup the tri that was hit - skip if not valid
    s32 HitTri = (s32)( CollPrimKey & ~CLOTH_PRIM_KEY_TRI_FLAG );
    if( ( HitTri < 0 ) || ( HitTri >= m_Triangles.GetCount() ) )
        return;

    // Lookup tri and tri particles
    cloth_triangle& Tri       = m_Triangles[HitTri];
    cloth_particle& ParticleA = m_Particles[(s32)Tri.m_Particles[0]];
    cloth_particle& ParticleB = m_Particles[(s32)Tri.m_Particles[1]];
    cloth_particle& ParticleC = m_Particles[(s32)Tri.m_Particles[2]];

    // Compute barycentric co-ords
    vector3 Bary;
    ComputeBarys(ParticleA.m_Pos,
                 ParticleB.m_Pos,
                 ParticleC.m_Pos, 
                 CollPoint, 
                 Bary);

    // Now compute UV's using barycentric co-ords
    vector2 UV = (Bary.GetX() * ParticleA.m_UV) +
                 (Bary.GetY() * ParticleB.m_UV) +
                 (Bary.GetZ() * ParticleC.m_UV);

    // Blit a hole into the damage bitmap, and use current pixel value as vel scaler
    f32 Scale = (1.0f / 15.0f) * 0.8f;
    
    if (PunchDamageHole)
        Scale *= PunchDamage(UV.X, UV.Y);
    else
        Scale *= CheckDamage(UV.X, UV.Y);

    Scale += 0.2f;

    // Compute impact vel
    vector3 ImpactVel = ProjectileVel;
    if( ManualImpactForce > 0 )
    {
        ImpactVel.NormalizeAndScale( ManualImpactForce );
    }
    else
    {
        ImpactVel *= m_ImpactScale;
    }
    ImpactVel *= Scale;

    // Apply to particles
    ParticleA.m_LastPos -= Bary.GetX() * ImpactVel;
    ParticleB.m_LastPos -= Bary.GetY() * ImpactVel;
    ParticleC.m_LastPos -= Bary.GetZ() * ImpactVel;

    voice_id VoiceID = g_AudioMgr.Play( "BulletImpactFabric",
                                        CollPoint, ProjectileObject.GetZone1(), TRUE );

    g_AudioManager.NewAudioAlert( VoiceID, 
                                  audio_manager::BULLET_IMPACTS, 
                                  CollPoint, 
                                  ProjectileObject.GetZone1(), 
                                  NULL_GUID );

}

//==============================================================================
// Logic functions
//==============================================================================

void cloth::Advance( f32 DeltaTime)
{
    CONTEXT("cloth::Advance");

    LOG_STAT(k_stats_Physics);
	
    // Add to total time
    m_DeltaTime += DeltaTime;

    // Apply delta time pool
    while(m_DeltaTime >= CLOTH_TIME_STEP)
    {
        // Integrate
        Integrate(CLOTH_TIME_STEP);

        // Iterate to solve constraints
        ApplyConstraints();

        // Next
        m_DeltaTime -= CLOTH_TIME_STEP;
    }
    
    // Update wind on/off state
    m_WindTimer -= DeltaTime;
    if( m_WindTimer <= 0.0f )
    {
        // Toggle state
        m_bWind ^= TRUE;
        
        // Setup timer
        if( m_bWind )
            m_WindTimer = x_frand( m_WindMinOnTime, m_WindMaxOnTime );
        else            
            m_WindTimer = x_frand( m_WindMinOffTime, m_WindMaxOffTime );
    }

    // Update wind direction
    m_WindDir.Rotate( m_WindRot * DeltaTime );
    m_WindDir.Normalize();
    
    // Apply wind?
    if( m_bWind )
    {
        // Compute world space wind dir 
        vector3 WorldWindDir = m_L2W.RotateVector( m_WindDir );
    
        // Update all particles
        for( s32 i = 0; i < m_Particles.GetCount(); i++ )
        {
            // Compute wind velocity
            f32     Strength = x_frand( m_WindMinStrength, m_WindMaxStrength );
            vector3 WindVel  = WorldWindDir * Strength;

            // Update particle velocity
            cloth_particle& Particle = m_Particles[i];
            Particle.m_LastPos -= WindVel;
        }
    }

    // Update bounds
    ComputeBounds();
}

//==============================================================================
// Set functions
//==============================================================================

void cloth::SetL2W( const matrix4& L2W, xbool bReset, f32 AirResistance )
{
    ASSERT( AirResistance >= 0.0f );
    ASSERT( AirResistance <= 1.0f );

    // Reset positions and bbox?
    if( bReset )
    {
        // Store new matrices
        m_L2W = L2W;
        m_W2L = m4_InvertRT( L2W );
    
        // Reset all particles    
        Reset();
    }
    else
    {
        // Compute old and new info
        vector3    OldPos = m_L2W.GetTranslation();
        quaternion OldRot = m_L2W.GetQuaternion();
        vector3    NewPos = L2W.GetTranslation();
        quaternion NewRot = L2W.GetQuaternion();
        quaternion InvOldRot = OldRot;
        InvOldRot.Invert();

        // Compute delta position/yaw for non-pinned particles
        vector3    DeltaPos = NewPos - OldPos;
        quaternion DeltaRot = NewRot * InvOldRot; // NOTE: Quats read left->right so this is equiv to NewRot - OldRot
        
        // Take air resistance into account
        DeltaPos *= ( 1.0f - AirResistance );                   // if AR=0 then DP=DeltaPos, if AR=1 then DP=(0,0,0)
        DeltaRot = BlendToIdentity( DeltaRot, AirResistance );  // if AR=0 then DR=DeltaRot, if AR=1 then DR=Identity
        
        // Compute delta transform matrix that will apply delta to non-pinned particles
        matrix4 DeltaTM;
        DeltaTM.Identity();
        DeltaTM.SetTranslation( -OldPos );          // Move to pivot
        DeltaTM.Rotate( DeltaRot );                 // Rotate around pivot
        DeltaTM.Translate( OldPos + DeltaPos );     // Put back into world
    
        // Store new matrices
        m_L2W = L2W;
        m_W2L = m4_InvertRT( L2W );
    
        // Update particle positions
        for (s32 i = 0; i < m_Particles.GetCount(); i++)
        {
            cloth_particle& Particle = m_Particles[i];
            
            // Pinned?
            if( Particle.m_InvMass == 0.0f )
            {
                // Compute position as if rigidly attached to the pole
                Particle.m_Pos = Particle.m_LastPos = m_L2W * Particle.m_BindPos;
            }
            else
            {
                // Update position and previous position so velocity isn't changed
                Particle.m_Pos     = DeltaTM * Particle.m_Pos;
                Particle.m_LastPos = DeltaTM * Particle.m_LastPos;
            }                
        }

        // Update bounds
        ComputeBounds();
        ComputeWorldCollBBox();
    }
}

//==============================================================================

void cloth::SetLocalCollBBox( const bbox& BBox )
{
    // Setup local bbox with validation incase min/max are backwards
    m_LocalCollBBox = BBox;
    m_LocalCollBBox.Min.Min( BBox.Max );
    m_LocalCollBBox.Max.Max( BBox.Min );
    
    // Update world bbox
    ComputeWorldCollBBox();

    // Reset positions and bbox
    Reset();
}

//==============================================================================

#ifdef TARGET_XBOX
static void cloth_SetColor( const xcolor& Color )
{
    D3DCOLOR C = D3DCOLOR_RGBA( Color.R>>1,
                                Color.G>>1,
                                Color.B>>1,
                                Color.A );

    g_pd3dDevice->SetVertexDataColor( D3DVSDE_DIFFUSE,C );
}
#endif

//==============================================================================

#ifdef TARGET_XBOX
static void cloth_SetVertex( const vector3& Vertex )
{
    g_pd3dDevice->SetVertexData4f(
        D3DVSDE_VERTEX,
        Vertex.GetX(),
        Vertex.GetY(),
        Vertex.GetZ(),
        1.0f
    );
}
#endif

//=============================================================================

#ifdef TARGET_XBOX

extern void xbox_SetPrimaryTarget( void );
extern void xbox_SetFogTarget    ( void );

///////////////////////////////////////////////////////////////////////////////

extern xcolor xbox_GetFogValue( const vector3& Position,s32 PaletteIndex );

///////////////////////////////////////////////////////////////////////////////

void cloth::RenderPrimingPass( texture* pTexture )
{
    xbox_SetFogTarget();

    // Render diffuse map blended with damage texture (ugly, ugly, ugly: massage draw)
    draw_Begin( DRAW_TRIANGLES, DRAW_TEXTURED | DRAW_CULL_NONE | DRAW_XBOX_NO_BEGIN );

    g_pd3dDevice->SetPixelShader(0);

    VERIFY( !g_pd3dDevice->SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 ));

    g_RenderState.Set( D3DRS_ALPHAFUNC, D3DCMP_GREATER );
    g_RenderState.Set( D3DRS_ALPHATESTENABLE,TRUE );
    g_RenderState.Set( D3DRS_ALPHAREF,64 );

    s32 iStg = 0;
    if( CLOTH_DAMAGE )
    {
        g_Texture.Set( iStg,vram_GetSurface( m_DamageBMP ));
        g_TextureStageState.Set( iStg, D3DTSS_ALPHAKILL    , D3DTALPHAKILL_ENABLE );
        g_TextureStageState.Set( iStg, D3DTSS_ADDRESSU     , D3DTADDRESS_CLAMP    );
        g_TextureStageState.Set( iStg, D3DTSS_ADDRESSV     , D3DTADDRESS_CLAMP    );
        g_TextureStageState.Set( iStg, D3DTSS_COLOROP      , D3DTOP_SELECTARG1    );
        g_TextureStageState.Set( iStg, D3DTSS_COLORARG1    , D3DTA_DIFFUSE        );
        g_TextureStageState.Set( iStg, D3DTSS_ALPHAOP      , D3DTOP_SELECTARG1    );
        g_TextureStageState.Set( iStg, D3DTSS_ALPHAARG1    , D3DTA_TEXTURE        );
        g_TextureStageState.Set( iStg, D3DTSS_RESULTARG    , D3DTA_CURRENT        );
        g_TextureStageState.Set( iStg, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU  );
        iStg ++;
    }

    g_TextureStageState.Set( iStg, D3DTSS_COLOROP, D3DTOP_DISABLE );
    g_TextureStageState.Set( iStg, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

    g_RenderState.Set( D3DRS_COLORWRITEENABLE,D3DCOLORWRITEENABLE_BLUE );

    // 2nd draw_Begin is necessary due to the GPU crash you'll get
    // if D3D->Begin() is called before setting all the above
    // render states. DRAW_KEEP_STATES mandatory.

    draw_EnableBilinear();
    draw_Begin( DRAW_TRIANGLES,DRAW_KEEP_STATES );
    {
        xcolor Temp = xbox_GetFogValue( m_L2W.GetTranslation(),g_RenderContext.LocalPlayerIndex );
        draw_Color( xcolor( 0,0,Temp.A,255 ));

        // Render tris
        for( s32 i=0;i<m_Triangles.GetCount();i++ )
        {
            // Lookup triangle
            cloth_triangle& Triangle = m_Triangles[i];

            // Loop through triangle verts
            for( s32 j = 0; j < 3; j++ )
            {
                // Lookup particle
                cloth_particle& Particle = m_Particles[(s32)Triangle.m_Particles[j]];

                // Draw particle vert
                g_pd3dDevice->SetVertexData2f( D3DVSDE_TEXCOORD0,Particle.m_UV.X,Particle.m_UV.Y );
                draw_Vertex(Particle.m_Pos);
            }
        }
    }
    draw_End();
}

///////////////////////////////////////////////////////////////////////////////

void cloth::RenderNormalPass( texture* pTexture )
{
    xbox_SetPrimaryTarget();

    // Render diffuse map blended with damage texture (ugly, ugly, ugly: massage draw)
    draw_Begin( DRAW_TRIANGLES, DRAW_TEXTURED | DRAW_CULL_NONE | DRAW_XBOX_NO_BEGIN );
    draw_SetTexture( pTexture->m_Bitmap );

    VERIFY( !g_pd3dDevice->SetVertexShader( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2 ));

    s32 iStg = 0;

    g_TextureStageState.Set( iStg, D3DTSS_ALPHAKILL    , D3DTALPHAKILL_DISABLE );
    g_TextureStageState.Set( iStg, D3DTSS_COLOROP      , D3DTOP_MODULATE       );
    g_TextureStageState.Set( iStg, D3DTSS_COLORARG1    , D3DTA_DIFFUSE         );
    g_TextureStageState.Set( iStg, D3DTSS_COLORARG2    , D3DTA_TEXTURE         );
    g_TextureStageState.Set( iStg, D3DTSS_ADDRESSU     , D3DTADDRESS_CLAMP     );
    g_TextureStageState.Set( iStg, D3DTSS_ADDRESSV     , D3DTADDRESS_CLAMP     );
    g_TextureStageState.Set( iStg, D3DTSS_ALPHAOP      , D3DTOP_SELECTARG1     );
    g_TextureStageState.Set( iStg, D3DTSS_ALPHAARG1    , D3DTA_DIFFUSE         );
    g_TextureStageState.Set( iStg, D3DTSS_RESULTARG    , D3DTA_CURRENT         );
    g_TextureStageState.Set( iStg, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU   );
    iStg ++;

    if( CLOTH_DAMAGE )
    {
        g_Texture.Set( iStg,vram_GetSurface( m_DamageBMP ));
        g_TextureStageState.Set( iStg, D3DTSS_ALPHAKILL    , D3DTALPHAKILL_ENABLE );
        g_TextureStageState.Set( iStg, D3DTSS_ADDRESSU     , D3DTADDRESS_CLAMP    );
        g_TextureStageState.Set( iStg, D3DTSS_ADDRESSV     , D3DTADDRESS_CLAMP    );
        g_TextureStageState.Set( iStg, D3DTSS_COLOROP      , D3DTOP_SELECTARG1    );
        g_TextureStageState.Set( iStg, D3DTSS_COLORARG1    , D3DTA_CURRENT        );
        g_TextureStageState.Set( iStg, D3DTSS_ALPHAOP      , D3DTOP_MODULATE      );
        g_TextureStageState.Set( iStg, D3DTSS_ALPHAARG1    , D3DTA_CURRENT        );
        g_TextureStageState.Set( iStg, D3DTSS_ALPHAARG2    , D3DTA_TEXTURE        );
        g_TextureStageState.Set( iStg, D3DTSS_RESULTARG    , D3DTA_CURRENT        );
        g_TextureStageState.Set( iStg, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU  );
        iStg ++;
    }

    g_TextureStageState.Set( iStg, D3DTSS_COLOROP, D3DTOP_DISABLE );
    g_TextureStageState.Set( iStg, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

    g_RenderState.Set( D3DRS_COLORWRITEENABLE,D3DCOLORWRITEENABLE_ALL );

    // 2nd draw_Begin is necessary due to the GPU crash you'll get
    // if D3D->Begin() is called before setting all the above
    // render states. DRAW_KEEP_STATES mandatory.

    draw_EnableBilinear();
    draw_Begin( DRAW_TRIANGLES,DRAW_KEEP_STATES );
    {
        // Render tris
        for( s32 i=0;i<m_Triangles.GetCount();i++ )
        {
            // Lookup triangle
            cloth_triangle& Triangle = m_Triangles[i];

            // Loop through triangle verts
            for( s32 j = 0; j < 3; j++ )
            {
                // Lookup particle
                cloth_particle& Particle = m_Particles[(s32)Triangle.m_Particles[j]];

                // Draw particle vert
                xcolor C( Particle.m_Color );
                C.A = 0;
                draw_Color( C );
                g_pd3dDevice->SetVertexData2f( D3DVSDE_TEXCOORD0,Particle.m_UV.X,Particle.m_UV.Y );
                g_pd3dDevice->SetVertexData2f( D3DVSDE_TEXCOORD1,Particle.m_UV.X,Particle.m_UV.Y );
                draw_Vertex(Particle.m_Pos);
            }
        }
    }
    draw_End();

    g_TextureStageState.Set( 0, D3DTSS_ALPHAKILL, D3DTALPHAKILL_DISABLE );
    g_TextureStageState.Set( 1, D3DTSS_ALPHAKILL, D3DTALPHAKILL_DISABLE );
}

#endif

//==============================================================================
// Render functions
//==============================================================================

void cloth::RenderClothGeometry( s32 VTexture /*= 0*/ )
{
    CONTEXT("cloth::RenderGeometry");

    s32 i,j;
                                            
    // Update lighting
    // TO DO: Only need to do this once for split screen!
    ComputeLighting();

    draw_ClearL2W();

    // Grab the texture that is currently being used by the cloth. Note
    // that we go through the render system to grab the texture out. Since
    // the geometry has been registered, the render system will have
    // registered the texture and have a nice way to access it rather than
    // using the resource system.
    texture* pTexture = NULL;
    const geom* pGeom = m_RigidInst.GetGeom();
    if( pGeom && (m_MaterialIndex != -1) )
    {
        pTexture = render::GetVTexture( pGeom, m_MaterialIndex, VTexture );
    }

    // Render dynamic cloth?
    if (pTexture)
    {
#ifdef TARGET_PS2
        // Draw damage?
        if (CLOTH_DAMAGE)
        {
            // Render damage texture using alpha
            draw_SetTexture(m_DamageBMP);
            draw_Begin(DRAW_TRIANGLES, DRAW_TEXTURED | DRAW_CULL_NONE);

            // Only write to alpha channel
            gsreg_Begin( 3 ) ;
            gsreg_SetFBMASK( 0x00FFFFFF ) ; // Write to Alpha only

            // Do not update Z so that things can be seen behind the punched holes
            gsreg_SetZBufferUpdate( FALSE );

            // Stencil into the alpha the damage texture
            gsreg_SetAlphaAndZBufferTests ( FALSE,                      // AlphaTest,
                                            ALPHA_TEST_GEQUAL,          // AlphaTestMethod,
                                            128,                        // AlphaRef,
                                            ALPHA_TEST_FAIL_RGB_ONLY,   // AlphaTestFail,
                                            FALSE,                      // DestAlphaTest,
                                            DEST_ALPHA_TEST_0,          // DestAlphaTestMethod,
                                            TRUE,                       // ZBufferTest,
                                            ZBUFFER_TEST_GEQUAL ) ;     // ZBufferTestMethod
            gsreg_End();

            for (i = 0; i < m_Triangles.GetCount(); i++)
            {
                // Lookup triangle
                cloth_triangle& Triangle = m_Triangles[i];

                // Loop through triangle verts
                for (j = 0; j < 3; j++)
                {
                    // Lookup particle
                    cloth_particle& Particle = m_Particles[Triangle.m_Particles[j]];

                    // Draw particle vert
                    draw_Color (XCOLOR_WHITE);
                    draw_UV    (Particle.m_UV);
                    draw_Vertex(Particle.m_Pos);
                }
            }
            draw_End();
        }

        // Render diffuse map blended with damage texture
        draw_SetTexture(pTexture->m_Bitmap);

        // Draw damage?
        if (CLOTH_DAMAGE)
        {
            draw_Begin(DRAW_TRIANGLES, DRAW_TEXTURED | DRAW_CULL_NONE | DRAW_USE_ALPHA);

            // Blend the flag diffuse towards black, using the alpha in the frame buffer
            // Perform an alpha dest test so we can have punch thru!
            // (Src - 0) * DstA + 0
            gsreg_Begin( 4 );
            gsreg_SetAlphaBlend( ALPHA_BLEND_MODE(C_SRC, C_ZERO, A_DST, C_ZERO) );
            gsreg_SetFBMASK( 0x00000000 );     // Write to ARGB
            
            // Turn on Z update
            gsreg_SetZBufferUpdate( TRUE );

            // Setup alpha dst test. Pixels with alpha top bit=one will pass!
            gsreg_SetAlphaAndZBufferTests ( FALSE,                  // AlphaTest,
                                            ALPHA_TEST_ALWAYS,      // AlphaTestMethod,
                                            64,                     // AlphaRef,
                                            ALPHA_TEST_FAIL_KEEP,   // AlphaTestFail,
                                            TRUE,                   // DestAlphaTest,
                                            DEST_ALPHA_TEST_1,      // DestAlphaTestMethod,
                                            TRUE,                   // ZBufferTest,
                                            ZBUFFER_TEST_GEQUAL ) ; // ZBufferTestMethod
            gsreg_End();
        }
        else
        {
            draw_Begin(DRAW_TRIANGLES, DRAW_TEXTURED | DRAW_CULL_NONE);
        }

        // Render tris
        for (i = 0; i < m_Triangles.GetCount(); i++)
        {
            // Lookup triangle
            cloth_triangle& Triangle = m_Triangles[i];

            // Loop through triangle verts
            for (j = 0; j < 3; j++)
            {
                // Lookup particle
                cloth_particle& Particle = m_Particles[Triangle.m_Particles[j]];

                // Draw particle vert
                draw_Color (Particle.m_Color);
                draw_UV    (Particle.m_UV);
                draw_Vertex(Particle.m_Pos);
            }
        }
        draw_End();

        // Turn off alpha dst test
        if (CLOTH_DAMAGE)
        {
            // Reset draw mode
            gsreg_Begin( 1 );
            gsreg_SetAlphaAndZBufferTests ( FALSE,                  // AlphaTest,
                                            ALPHA_TEST_ALWAYS,      // AlphaTestMethod,
                                            64,                     // AlphaRef,
                                            ALPHA_TEST_FAIL_KEEP,   // AlphaTestFail,
                                            FALSE,                  // DestAlphaTest,
                                            DEST_ALPHA_TEST_0,      // DestAlphaTestMethod,
                                            TRUE,                   // ZBufferTest,
                                            ZBUFFER_TEST_GEQUAL ); // ZBufferTestMethod
            gsreg_End();
        }

#elif defined TARGET_XBOX

    extern xbool m_SatCompensation;
    xbool bOld = m_SatCompensation;

    m_SatCompensation = FALSE;
    RenderPrimingPass( pTexture );
    RenderNormalPass ( pTexture );
    m_SatCompensation = bOld;

#else

        // PC VERSION - Just render diffuse map

        // Render diffuse map blended with damage texture
        draw_SetTexture(pTexture->m_Bitmap);
        draw_Begin(DRAW_TRIANGLES, DRAW_TEXTURED | DRAW_CULL_NONE);
        for (i = 0; i < m_Triangles.GetCount(); i++)
        {
            // Lookup triangle
            cloth_triangle& Triangle = m_Triangles[i];

            // Loop through triangle verts
            for (j = 0; j < 3; j++)
            {
                // Lookup particle
                cloth_particle& Particle = m_Particles[(s32)Triangle.m_Particles[j]];

                // Draw particle vert
                draw_Color (Particle.m_Color);
                draw_UV    (Particle.m_UV);
                draw_Vertex(Particle.m_Pos);
            }
        }
        draw_End();

#endif  // #ifdef TARGET_PS2


    }
}

//==============================================================================

void cloth::RenderRigidGeometry( u32 Flags )
{
    // Render rigid instance
    m_RigidInst.Render( &m_L2W, Flags, m_RenderMask );
}

//==============================================================================

#if !defined( CONFIG_RETAIL )

void draw_Arrow( const vector3& Start, const vector3& End, xcolor Color )
{
    // Compute axis of rotation for side lines
    vector3 Dir    = End - Start;
    vector3 Out    = Dir.Cross( vector3( 0, 1, 0 ) );
    if( Out.LengthSquared() < 0.1f )
        Out = Dir.Cross( vector3( 0, 1, 0 ) );
    vector3 Up     = Out.Cross( Dir );
    Up.Normalize();
    
    // Compute side line directions
    quaternion Rot0( Up, -R_25 );
    quaternion Rot1( Up,  R_25 );
    vector3    Dir0 = Rot0 * -Dir * 0.2f;
    vector3    Dir1 = Rot1 * -Dir * 0.2f;

    // Draw arrow lines    
    draw_Begin( DRAW_LINES );
    draw_Color( Color );
    draw_Vertex( Start ); draw_Vertex( End );           // Main
    draw_Vertex( End );   draw_Vertex( End + Dir0 );    // Left 
    draw_Vertex( End );   draw_Vertex( End + Dir1 );    // Right 
    draw_End();
}

void cloth::RenderSkeleton( void )
{
    CONTEXT("cloth::RenderSkeleton");

    s32 i;

    draw_ClearL2W();

    // Render world wind direction arrow
    f32     Radius = m_WorldBBox.GetRadius();
    vector3 Dir    = Radius * 0.5f * m_L2W.RotateVector( m_WindDir );
    vector3 Center = ( m_L2W * m_LocalBBox.GetCenter() ) - ( Dir * 2.0f );
    vector3 Start  = Center - Dir;
    vector3 End    = Center + Dir;
    xcolor  Color  = m_bWind ? XCOLOR_GREEN : XCOLOR_RED;

#ifdef X_EDITOR
    // Render white if not running the game
    if( g_game_running == FALSE )
        Color = XCOLOR_WHITE;
#endif
        
    draw_Arrow( Start, End, Color );
    
#ifdef X_EDITOR
    if( g_game_running )
        draw_Label( Center, Color, m_bWind ? "\n\nWindOn" : "\n\nWindOff" );
    else        
        draw_Label( Center, Color, "\n\nWind" );
#else
    draw_Label( Center, Color, m_bWind ? "\n\nWindOn" : "\n\nWindOff" );
#endif
    
    // Render all connections
    for (i = 0; i < m_Connections.GetCount(); i++)
    {
        cloth_connection& Connection = m_Connections[i];

        draw_Line( m_Particles[(s32)Connection.m_ParticleA].m_Pos,
                   m_Particles[(s32)Connection.m_ParticleB].m_Pos, XCOLOR_BLUE );
    }

    // Render all particles
    for (i = 0; i < m_Particles.GetCount(); i++)
    {
        // Lookup particle info
        cloth_particle& Particle = m_Particles[i];
        f32    Mass = Particle.GetMass();
        xcolor Color = (Mass == 0.0f) ? XCOLOR_RED : XCOLOR_GREEN;
        s32    Size  = (Mass == 0.0f) ? 5 : 2 ;

        // Draw particle and normal
        draw_Point(Particle.m_Pos, Color, Size );
        draw_Arrow(Particle.m_Pos, Particle.m_Pos + (Particle.m_Normal * 15.0f), Color );

        // Show mass        
        draw_Label( Particle.m_Pos, Color, "\n\n\n\nP[%d]\nMass(%.2f)", i, Mass );
    }

    // Render world collision bbox
    draw_BBox(m_WorldCollBBox, XCOLOR_YELLOW);
}

#endif  //#if !defined( CONFIG_RETAIL )

//==============================================================================

cloth_particle::cloth_particle() :
    m_BindPos       ( 0.0f, 0.0f, 0.0f ),
    m_Pos           ( 0.0f, 0.0f, 0.0f ),
    m_LastPos       ( 0.0f, 0.0f, 0.0f ),
    m_Normal        ( 0.0f, 0.0f, 1.0f ),
    m_UV            ( 0.0f, 0.0f ),
    m_InvMass       ( 1.0f ),
    m_Color         ( 128,128,128 )
{
}

//===========================================================================

f32 cloth_particle::GetMass( void ) const
{
    // Infinite mass?
    if( m_InvMass == 0.0f )
        return 0.0f;
    else        
        return 1.0f / m_InvMass;        
}

//===========================================================================

void cloth_particle::SetMass( f32 Mass )
{
    // Infinite mass?
    if( Mass == 0.0f )
        m_InvMass = 0.0f;
    else        
        m_InvMass = 1.0f / Mass;
}

//===========================================================================

