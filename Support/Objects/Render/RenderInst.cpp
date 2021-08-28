//=============================================================================
//
//  RenderInst.cpp
//
//=============================================================================

#include "Entropy.hpp"
#include "Objects\Render\RenderInst.hpp"

//=============================================================================
// DATA
//=============================================================================

#ifdef X_EDITOR
render_inst* render_inst::s_pHead = NULL;
#endif // X_EDITOR

//=============================================================================
// FUNCTIONS
//=============================================================================

render_inst::render_inst( void )
{
    m_hInst.Handle       = HNULL;
    m_Alpha              = 255;
    
    m_FadeTime        = 0.0f;
    m_FadeTimeElapsed = 0.0f;
    m_FadeDirection   = 0;

    #ifdef X_EDITOR
    m_pNext = NULL;
    m_pPrev = NULL;
    AddToList( this );
    #endif // X_EDITOR
}

//============================================================================+=

render_inst::~render_inst( void )
{
    #ifdef X_EDITOR
    RemoveFromList( this );
    m_pNext = NULL;
    m_pPrev = NULL;
    #endif // X_EDITOR
}

//=============================================================================

void render_inst::StartFade( s8 Direction, f32 TimeToFade )
{
    // handle fading in
    if ( Direction == 1 )
    {
        // full alpha right away?
        if ( (m_Alpha == 255) || (TimeToFade == 0.0f) )
        {
            m_Alpha         = 255;
            m_FadeDirection = 0;
            m_FadeTime      = 0.0f;
        }
        else
        {
            // calculate how far we should be faded already
            f32 T = (f32)m_Alpha / 255.0f;
            m_FadeTime        = TimeToFade;
            m_FadeTimeElapsed = T * TimeToFade;
            m_FadeDirection   = 1;
        }
    }
    // handle fading out
    else if ( Direction == -1 )
    {
        // zero alpha right away?
        if ( (m_Alpha == 0) || (TimeToFade == 0.0f) )
        {
            m_Alpha         = 0;
            m_FadeDirection = 0;
            m_FadeTime      = 0.0f;
        }
        else
        {
            // calculate how far we should be faded already
            f32 T = 1.0f - ((f32)m_Alpha / 255.0f);
            m_FadeTime        = TimeToFade;
            m_FadeTimeElapsed = T * TimeToFade;
            m_FadeDirection   = -1;
        }
    }
}

//=============================================================================

void render_inst::OnAdvanceLogic( f32 DeltaTime )
{
    if( m_FadeDirection == -1 )
    {
        m_FadeTimeElapsed += DeltaTime;
        if ( m_FadeTimeElapsed >= m_FadeTime )
        {
            m_Alpha           = 0;
            m_FadeTime        = 0.0f;
            m_FadeTimeElapsed = 0.0f;
            m_FadeDirection   = 0;
        }
        else
        {
            ASSERT( m_FadeTime > 0.0f );
            m_Alpha = (u8)(255.0f * (1.0f - (m_FadeTimeElapsed / m_FadeTime)));
        }
    }
    else
    if( m_FadeDirection == 1 )
    {
        m_FadeTimeElapsed += DeltaTime;
        if ( m_FadeTimeElapsed >= m_FadeTime )
        {
            m_Alpha           = 255;
            m_FadeTime        = 0.0f;
            m_FadeTimeElapsed = 0.0f;
            m_FadeDirection   = 0;
        }
        else
        {
            ASSERT( m_FadeTimeElapsed > 0.0f );
            m_Alpha = (u8)(255.0f * (m_FadeTimeElapsed / m_FadeTime));
        }
    }
}

//=============================================================================

bbox& render_inst::GetBBox( void ) const
{
    static bbox BBox;
    BBox.Clear();

    geom* pGeom = GetGeom();
    
    if( pGeom ) return( pGeom->m_BBox );
    else        return( BBox );
}

//=============================================================================

u64 render_inst::GetLODMask( const matrix4& L2W )
{
    // Compute screen size of object
    const view* pView = eng_GetView();
    geom* pGeom = GetGeom();
    f32 ScreenSize = pView->CalcScreenSize( L2W.GetTranslation(), pGeom->m_BBox.GetRadius() );

    // It is possible that when you get some cinematics putting weird camera angles
    // in place, the bbox position can technically be behind the camera. This is
    // because the view is at eye point, and the actor position is usually at
    // his feet. Get in close, tilt the camera up a bit, and voila. To correct
    // for this, fix up a screen size of zero returned from the view by
    // max'ing it out.
    if( ScreenSize <= 0.0f )
        ScreenSize = 65000.0f;

    // Get correct LOD Mask based on screen size
    return pGeom->GetLODMask( m_VMeshMask, (u16)ScreenSize );   // instead of -1, make a mask!!!!
}

//=============================================================================

s32 render_inst::GetNActiveBones( const u64& LODMask ) const
{
    // Lookup geom
    geom* pGeom = GetGeom();
    if( !pGeom )
        return 0;

    // Count bones used by geometry and tell the animation player
    //( bones in sorted into hierarchical order so we can just keep the max )
    s32 nActiveBones = 0;
    for( s32 i = 0; i < pGeom->m_nMeshes; i++ )
    {
        // Is this mesh being used?
        if( LODMask & ( (u64)1 << i ) )
        {
            // Update max count
            nActiveBones = MAX( nActiveBones, pGeom->m_pMesh[i].nBones );
        }
    }

    return nActiveBones;
}

//=============================================================================

void render_inst::OnEnumProp( prop_enum& List )
{
    // enumerate the vmesh list
    s32 HeaderId = List.PushPath( "RenderInst\\" );
    m_VMeshMask.OnEnumProp( List, GetGeom() );
    List.PopPath( HeaderId );
}

//=============================================================================

xbool render_inst::OnProperty( prop_query& I )
{
    geom* pGeom = GetGeom();

    if ( !I.IsBasePath( "RenderInst\\" ) )
    {
        return FALSE;
    }

    // handle the vmesh list
    s32 HeaderId = I.PushPath( "RenderInst\\" );
    if( m_VMeshMask.OnProperty( I, pGeom ) )
    {
    }
    else
    {
        I.PopPath( HeaderId );
        return FALSE;
    }

    I.PopPath( HeaderId );
    return( TRUE );
}

//=============================================================================

#ifdef X_EDITOR
void render_inst::UnregisterAll( void )
{
    render_inst* pCurr = s_pHead;
    while ( pCurr != NULL )
    {
        pCurr->UnregiserInst();
        pCurr = pCurr->m_pNext;
    }
}
#endif // X_EDITOR

//=============================================================================

#ifdef X_EDITOR
void render_inst::RegisterAll( void )
{
    render_inst* pCurr = s_pHead;
    while ( pCurr != NULL )
    {
        pCurr->RegisterInst();
        pCurr = pCurr->m_pNext;
    }
}
#endif // X_EDITOR

//=============================================================================

#ifdef X_EDITOR
void render_inst::AddToList( render_inst* pRenderInst )
{
    if ( s_pHead )
        s_pHead->m_pPrev = pRenderInst;

    pRenderInst->m_pPrev = NULL;
    pRenderInst->m_pNext = s_pHead;
    s_pHead = pRenderInst;
}
#endif // X_EDITOR

//=============================================================================

#ifdef X_EDITOR
void render_inst::RemoveFromList( render_inst* pRenderInst )
{
    // fix up the links
    if ( pRenderInst->m_pNext )
        pRenderInst->m_pNext->m_pPrev = pRenderInst->m_pPrev;
    if ( pRenderInst->m_pPrev )
        pRenderInst->m_pPrev->m_pNext = pRenderInst->m_pNext;

    // fix up the head?
    if ( pRenderInst == s_pHead )
        s_pHead = pRenderInst->m_pNext;

    // clear the pointers for safety
    pRenderInst->m_pNext = NULL;
    pRenderInst->m_pPrev = NULL;
}
#endif // X_EDITOR

//=============================================================================

// EOF
