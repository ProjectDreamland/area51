//==============================================================================
//  VolumetricLight.cpp
//
//  Copyright (c) 2004 Inevitable Entertainment Inc. All rights reserved.
//
//  This class is for generating a "volumetric light". That is, it fakes a light
//  source projecting through dust, mist, or whatever that causes the light to
//  be scattered. Artists have been faking this with a sprite effect, but this
//  version will *hopefully* be a little bit less fill-rate intensive.
//==============================================================================

#include "VolumetricLight.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "Entropy.hpp"
#include "E_Draw.hpp"
#include "Render\Render.hpp"

//==============================================================================
// DEFINES
//==============================================================================

//==============================================================================
// OBJECT DESCRIPTION
//==============================================================================

static struct volumetric_light_obj_desc : public object_desc
{
    volumetric_light_obj_desc( void ) : object_desc( 
        object::TYPE_VOLUMETRIC_LIGHT, 
        "VolumetricLight", 
        "RENDER",

        object::ATTR_RENDERABLE    |
        object::ATTR_TRANSPARENT,

        FLAGS_GENERIC_EDITOR_CREATE | FLAGS_IS_DYNAMIC) {}

        //-------------------------------------------------------------------------

        virtual object* Create( void ) { return new volumetric_light_obj; }

        //-------------------------------------------------------------------------

#ifdef X_EDITOR
        virtual s32  OnEditorRender( object& Object ) const
        {
            object_desc::OnEditorRender( Object );
            return EDITOR_ICON_LIGHT_VOLUMETRIC;
        }
#endif // X_EDITOR

} s_VolumetricLightObj_Desc;

//==============================================================================

const object_desc& volumetric_light_obj::GetTypeDesc( void ) const
{
    return s_VolumetricLightObj_Desc;
}

//==============================================================================

const object_desc& volumetric_light_obj::GetObjectType( void )
{
    return s_VolumetricLightObj_Desc;
}

//==============================================================================
// FUNCTIONS
//==============================================================================

volumetric_light_obj::volumetric_light_obj( void ) :
    m_StartSize     ( 100.0f ),
    m_EndSize       ( 200.0f ),
    m_Length        ( 400.0f ),
    m_StartColor    ( 128, 128, 128, 255 ),
    m_EndColor      ( 128, 128, 128, 255 ),
    m_nSprites      ( 6 ),
    m_bRender       ( TRUE ),
    m_nBytesAlloced ( 0 ),
    m_pData         ( NULL )
{
}

//==============================================================================

volumetric_light_obj::~volumetric_light_obj( void )
{
    m_nBytesAlloced = 0;
    if( m_pData )
        x_free( m_pData );
}

//==============================================================================

#ifdef X_EDITOR
void volumetric_light_obj::OnDebugRender( void )
{
    f32 StartRadius = m_StartSize * 0.5f;
    f32 EndRadius   = m_EndSize * 0.5f;

    xcolor  Colors[8];
    vector3 Verts[8];
    Colors[0] = Colors[1] = Colors[2] = Colors[3] = m_StartColor;
    Colors[4] = Colors[5] = Colors[6] = Colors[7] = m_EndColor;
    Verts[0].Set( -StartRadius, -StartRadius, 0.0f );
    Verts[1].Set(  StartRadius, -StartRadius, 0.0f );
    Verts[2].Set(  StartRadius,  StartRadius, 0.0f );
    Verts[3].Set( -StartRadius,  StartRadius, 0.0f );
    Verts[4].Set( -EndRadius, -EndRadius, m_Length );
    Verts[5].Set(  EndRadius, -EndRadius, m_Length );
    Verts[6].Set(  EndRadius,  EndRadius, m_Length );
    Verts[7].Set( -EndRadius,  EndRadius, m_Length );

    static s16 Indices[] =
    {
        0, 1, 2, 3, 0, -1,
        4, 5, 6, 7, 4, -1,
        0, 4, -1, 1, 5, -1,
        2, 6, -1, 3, 7, -1
    };

    draw_SetL2W( GetL2W() );
    draw_Begin( DRAW_LINE_STRIPS );
    draw_Colors( Colors, 8 );
    draw_Verts( Verts, 8 );
    draw_Execute( Indices, 24 );
    draw_End();
}
#endif // X_EDITOR

//==============================================================================

void volumetric_light_obj::OnRender( void )
{
}

//==============================================================================

void volumetric_light_obj::OnRenderTransparent( void )
{
    // Is the light on?
    if( !m_bRender )
    {
        return;
    }

    ////////////////////////////////////////////////////////////////////////
    // Use our pre-compiled data to render the volumetric light
    ////////////////////////////////////////////////////////////////////////

    // early returns if we don't have valid data
    if( !m_pData )
        return;
    
    texture* pProjTex = m_ProjTexHandle.GetPointer();
    if( !pProjTex )
        return;

    // grab out pointers to our compiled data
    byte* pData = m_pData;
    vector4* pPositions = (vector4*)pData;
    pData += ALIGN_16( sizeof(vector4) * m_nSprites );
    vector2* pRotScales = (vector2*)pData;
    pData += ALIGN_16( sizeof(vector2) * m_nSprites );
    u32* pColors = (u32*)pData;
    pData += ALIGN_16( sizeof(u32) * m_nSprites );
    ASSERT( pData == (m_pData + m_nBytesAlloced) );

    // render using our already compiled data
    render::SetDiffuseMaterial( pProjTex->m_Bitmap, render::BLEND_MODE_ADDITIVE, TRUE );
    render::Render3dSprites( m_nSprites, 1.0f, &GetL2W(), pPositions, pRotScales, pColors );
}

//==============================================================================

bbox volumetric_light_obj::GetLocalBBox( void ) const
{
    // The bbox width and height comes directly from the 
    // light's maximum radius, but then we need to expand
    // the ends to account for the sprite sizes there.
    f32 MaxRadius = 0.5f * MAX( m_StartSize, m_EndSize );
    return bbox( vector3( -MaxRadius, -MaxRadius, -m_StartSize * 0.5f ),
                 vector3( MaxRadius, MaxRadius, m_Length + m_EndSize * 0.5f ) );
}

//==============================================================================

void volumetric_light_obj::OnEnumProp( prop_enum& List )
{
    object::OnEnumProp( List );

    List.PropEnumHeader  ( "VolumetricLight",             "Volumetric light properties", 0 );
    List.PropEnumBool    ( "VolumetricLight\\IsTurnedOn", "Use this to turn the light on and off dynamically", PROP_TYPE_EXPOSE );
    List.PropEnumInt     ( "VolumetricLight\\NumSprites", "Number of sprites to use", 0 );
    List.PropEnumFloat   ( "VolumetricLight\\StartSize",  "The starting size of this light", 0 );
    List.PropEnumFloat   ( "VolumetricLight\\EndSize",    "The ending size of this light", 0 );
    List.PropEnumFloat   ( "VolumetricLight\\Length",     "The length this light volume will expand across", 0 );
    List.PropEnumColor   ( "VolumetricLight\\StartColor", "Starting color of this light", 0 );
    List.PropEnumColor   ( "VolumetricLight\\EndColor",   "Starting color of this light", 0 );
    List.PropEnumExternal( "VolumetricLight\\Texture",    "Resource\0xbmp\0", "Texture to project", 0 );
}

//==============================================================================

xbool volumetric_light_obj::OnProperty( prop_query& I )
{
    if( object::OnProperty( I ) )
    {
    }
    else if( I.VarBool( "VolumetricLight\\IsTurnedOn", m_bRender ) )
    {
    }
    else if( I.VarInt( "VolumetricLight\\NumSprites", m_nSprites, 2, 64 ) )
    {
        // rebuild the data if necessary
        if( !I.IsRead() )
            SetupData();
    }
    else if( I.VarFloat( "VolumetricLight\\StartSize", m_StartSize, 1.0f, 1000.0f ) )
    {
        // rebuild the data if necessary
        if( !I.IsRead() )
        {
            OnMove( GetPosition() );    // this forces the bbox to be rebuilt
            SetupData();
        }
    }
    else if( I.VarFloat( "VolumetricLight\\EndSize", m_EndSize, 1.0f, 1000.0f ) )
    {
        // rebuild the data if necessary
        if( !I.IsRead() )
        {
            OnMove( GetPosition() );    // this forces the bbox to be rebuilt
            SetupData();
        }
    }
    else if( I.VarFloat( "VolumetricLight\\Length", m_Length, 1.0f, 3000.0f ) )
    {
        // rebuild the data if necessary
        if( !I.IsRead() )
        {
            OnMove( GetPosition() );    // this forces the bbox to be rebuilt
            SetupData();
        }
    }
    else if( I.VarColor( "VolumetricLight\\StartColor", m_StartColor ) )
    {
        // rebuild the data if necessary
        if( !I.IsRead() )
            SetupData();
    }
    else if( I.VarColor( "VolumetricLight\\EndColor", m_EndColor ) )
    {
        // rebuild the data if necessary
        if( !I.IsRead() )
            SetupData();
    }
    else if( I.IsVar( "VolumetricLight\\Texture" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_ProjTexHandle.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            m_ProjTexHandle.SetName( I.GetVarExternal() );
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

//==============================================================================

void volumetric_light_obj::SetupData( void )
{
    ////////////////////////////////////////////////////////////////////////////
    // Allocate space for the data and get some pointers to it
    ////////////////////////////////////////////////////////////////////////////

    // make sure we have the right amounted of allocated space for the new data
    s32 DataSize  = ALIGN_16( sizeof(vector4) * m_nSprites );
    DataSize     += ALIGN_16( sizeof(vector2) * m_nSprites );
    DataSize     += ALIGN_16( sizeof(u32) * m_nSprites );
    if( DataSize != m_nBytesAlloced )
    {
        m_nBytesAlloced = DataSize;
        if( m_pData )
            x_free( m_pData );
        m_pData = (byte*)x_malloc( DataSize );
    }

    // grab out some pointers to the new data
    byte* pData = m_pData;
    vector4* pVerts     = (vector4*)pData;
    pData += ALIGN_16( sizeof(vector4) * m_nSprites );
    vector2* pRotScales = (vector2*)pData;
    pData += ALIGN_16( sizeof(vector2) * m_nSprites );
    u32* pColors = (u32*)pData;
    pData += ALIGN_16( sizeof(u32) * m_nSprites );
    ASSERT( pData == (m_pData + m_nBytesAlloced) );

    ////////////////////////////////////////////////////////////////////////////
    // TODO: Use the function mtraub and I came up with to replace this stepping
    // algorithm. D[N] = (C^N-1)/(C-1)
    //
    // If we lay a bunch of sprites out in a line where each sprite's
    // intersection point is proportional to the next one, then we get a
    // situation where the distances look something like this:
    // D[i] = C*(X^(i-1) + X^(i-2) + ... + X^0), where D is the distance to
    // the next sprite, C is the distance between the first two sprites, and
    // X is the proportion between the sprites. Using the sigma notation, this
    // turns into:
    // D[N] = C * SIGMA(i=0, i<N-1, X^i)
    //      = C * ((X^(N-1))/(X-1)), by using polynomial long division
    //
    // We don't really care what X is since we can just scale the end result
    // into a nice range using C, so let's just choose X to be 2 and avoid
    // a divide-by-zero.
    ////////////////////////////////////////////////////////////////////////////
    

    ////////////////////////////////////////////////////////////////////////////
    // Do an approximation to figure out the correct constant multiplier
    // between sprites for the volumetric light.
    //
    // The idea is that the spacing between sprites should follow this pattern
    // and we will use Newton's method to approximate it.
    //
    // If R is our initial radius, then each subsequent sprite should be placed
    // with some ratio from r such that:
    // S[N] = R + C*R + C*C*R + C*C*C+R + ... + (C^N)*R = VolumeLength
    ////////////////////////////////////////////////////////////////////////////

    // Assume a constant of whatever it would take to get to the end of the
    // volume right away. This should be the complete max.
    static const f32 kEpsilon = 1.0f;
    f32 CurrGuess   = m_Length/(m_StartSize*0.5f);
    f32 StepConst   = CurrGuess;
    s32 SafetyCount = 0;
    while( 1 )
    {
        // figure out how far off we are if we were to add all the sprites
        f32 Radius = m_StartSize * 0.5f;
        s32 i;
        f32 Sum = 0.0f;
        for( i = 0; i < m_nSprites-1; i++ )
        {
            Radius *= StepConst;
            Sum    += Radius;
            if( Sum > (m_Length + kEpsilon) )
                break;
        }

        // bail out if we can't come to a decent solution...we'll just
        // use what we have thus far
        if( ++SafetyCount >= 100 )
            break;

        // have we approximated close enough?
        if( (i==m_nSprites-1) && (x_abs( m_Length-Sum ) <= kEpsilon) )
        {
            break;
        }

        // should we add or subtract to the constant?
        CurrGuess *= 0.5f;
        if( Sum < m_Length )
            StepConst += CurrGuess;
        else
            StepConst -= CurrGuess;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Okay, now that we have that, build the sprite data
    ////////////////////////////////////////////////////////////////////////////

    f32 D        = 0.0f;
    f32 StepSize = m_StartSize * 0.5f;
    s32 i;
    for( i = 0; i < m_nSprites; i++ )
    {
        // figure out the sprite properties
        f32 T = D / m_Length;
        T = MINMAX( 0.0f, T, 1.0f );
        f32 SpriteSize = m_StartSize + T * (m_EndSize - m_StartSize);
        u8 R = (u8)((f32)m_StartColor.R + T * (f32)(m_EndColor.R - m_StartColor.R));
        u8 G = (u8)((f32)m_StartColor.G + T * (f32)(m_EndColor.G - m_StartColor.G));
        u8 B = (u8)((f32)m_StartColor.B + T * (f32)(m_EndColor.B - m_StartColor.B));
        u8 A = (u8)((f32)m_StartColor.A + T * (f32)(m_EndColor.A - m_StartColor.A));

#ifdef TARGET_PS2
        R = (R==255) ? 0x80 : (R>>1);
        G = (R==255) ? 0x80 : (G>>1);
        B = (R==255) ? 0x80 : (B>>1);
        A = (R==255) ? 0x80 : (A>>1);
#endif

        // now fill the data into our arrays in the right format
        pVerts[i].Set( 0.0f, 0.0f, D, 0.0f );
        pRotScales[i].Set( R_0, SpriteSize );
        pColors[i] = (A<<24) | (B<<16) | (G<<8) | R;

        StepSize *= StepConst;
        D        += StepSize;
    }
}

//==============================================================================
