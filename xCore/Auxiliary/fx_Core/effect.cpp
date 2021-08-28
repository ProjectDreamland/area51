//============================================================================
// INCLUDES
//============================================================================

#include "x_files.hpp"
#include "controller.hpp"
#include "element.hpp"
#include "element_spemitter.hpp"
#include "effect.hpp"
#include "x_context.hpp"

namespace fx_core
{

//============================================================================
// DATA
//============================================================================
#define EFFECT_VER  3


//============================================================================
//  effect
//============================================================================

effect::effect()
{
    m_IgnoreMeshes      = FALSE;
    m_Instanceable      = FALSE;
    m_pType             = "Effect";
}

//============================================================================

effect::~effect()
{
    s32 Count = m_Controllers.GetCount();
    s32 i;

    for ( i = 0; i < Count; i++ )
    {
        controller *pData = m_Controllers[i];
        delete pData;
    }
    
    m_Controllers.SetCount(0);

    Count = m_Elements.GetCount();

    // iterate through and delete each entry
    for ( i = 0; i < Count; i++ )
    {
        element *pData = m_Elements[i];
        delete pData;
    }

    m_Elements.SetCount(0);
}

//============================================================================

effect::effect( effect& Effect )
{
    ASSERT( FALSE );
}

//============================================================================
    
s32 effect::GetNumElements( void )
{
    return m_Elements.GetCount();
}

//============================================================================

element* effect::GetElement( s32 iElement )
{
    return m_Elements[iElement];
}

//============================================================================

void effect::AddElement( element* pElement )
{
    m_Elements.Append( pElement );
}

//============================================================================

void effect::RemoveElement( element* pElem )
{

    s32 Count, i;

    Count = m_Elements.GetCount();

    // look for the element in the list
    for ( i = 0; i < Count; i++ )
    {
        element* pElem2 = m_Elements[i];

        if ( pElem == pElem2 )
        {
            controller* pCont[5];
            pCont[0] = pElem->m_pScale;
            pCont[1] = pElem->m_pRotation;
            pCont[2] = pElem->m_pTranslation;
            pCont[3] = pElem->m_pColor;
            pCont[4] = pElem->m_pAlpha;

            delete pElem;
            m_Elements.Delete( i );

            // now delete the controllers
            for ( i = 0; i < 5; i++ )
            {
                s32 j;

                for ( j = 0; j < m_Controllers.GetCount(); j++ )
                {
                    if ( pCont[i] == m_Controllers[j] )
                    {
                        delete pCont[i];
                        m_Controllers.Delete(j);
                        break;
                    }
                }
            }
            return;
        }
    }
}

//============================================================================

void effect::MoveElement( s32 iElement, s32 iElementNew )
{
    ASSERT( iElement >= 0 );
    ASSERT( iElement < m_Elements.GetCount() );
    ASSERT( iElementNew >= 0 );
    ASSERT( iElementNew < m_Elements.GetCount() );

    element* pElement = m_Elements[iElement];
    m_Elements.Delete( iElement );
    m_Elements.Insert( iElementNew ) = pElement;
}

//============================================================================
    
s32 effect::GetNumControllers( void )
{
    s32         Count = 0;
    s32         i;
    s32         j;

    j = m_Controllers.GetCount();

    // Count the number of controllers with more than one key value
    for ( i = 0; i < j; i++ )
    {
        controller* pCont = m_Controllers[i];

        if ( strcmp(pCont->GetType(), LINEAR_KEY) == 0 )
        {
            ctrl_linear* pCtrl = (ctrl_linear*)pCont;

            if ( pCtrl->GetKeyCount() > 1 )
                Count++;
        }
    }

    return Count;
}

//============================================================================

s32 effect::GetOptimalNumControllers( void )
{
    s32         Count = 0;
    s32         i;
    s32         j;
    s32         Channels[30];

    j = m_Controllers.GetCount();

    // Count the number of controllers with more than one key value
    for ( i = 0; i < j; i++ )
    {
        controller* pCont = m_Controllers[i];

        if ( strcmp(pCont->GetType(), LINEAR_KEY) == 0 )
        {
            ctrl_linear* pCtrl = (ctrl_linear*)pCont;

            if ( pCtrl->GetOptimalKeyCount(Channels) )
                Count++;
        }
    }

    return Count;
}

//============================================================================
    
s32 effect::GetTotalNumControllers( void )
{
    return m_Controllers.GetCount();
}

//============================================================================

controller* effect::GetController( s32 iController )
{
    return m_Controllers[iController];
}

//============================================================================

void effect::AddController( controller* pController )
{
    m_Controllers.Append( pController );
}

//============================================================================

void effect::RenderBackground( f32 T )
{
    CONTEXT( "effect::RenderBackground" );

    // Background name?
    if( !m_BackgroundName.IsEmpty() )
    {
        // Start drawing
        draw_Begin( DRAW_SPRITES, DRAW_TEXTURED | DRAW_USE_ALPHA | DRAW_NO_ZBUFFER | DRAW_2D );
        if ( g_pTextureMgr->ActivateBitmap( m_BackgroundName ) == FALSE )
            m_BackgroundName.Clear();

        const view* pView = eng_GetView();
        rect r;
        pView->GetViewport( r );

        draw_Sprite( vector3(0,0,0), vector2(r.Max.X,r.Max.Y), XCOLOR_WHITE );

        draw_End();
    }
}

//============================================================================

void effect::Render( f32 T )
{
    CONTEXT( "effect::Render" );

    s32 i, Count;

    Count = m_Elements.GetCount();

    // Render all mesh elements first
    for ( i = 0; i < Count; i++ )
    {
        element* pElement = m_Elements[i];

        if( x_strcmp( pElement->GetType(), "Mesh" ) == 0 )
        {
            pElement->Render( T );
        }
    }

    // Render everything else
    for ( i = 0; i < Count; i++ )
    {
        element* pElement = m_Elements[i];

        if( x_strcmp( pElement->GetType(), "Mesh" ) != 0 )
        {
            pElement->Render( T );
        }
    }
}

//============================================================================

xbool effect::GetProperty( s32 Idx, s32 T, xstring& Name, xstring& Value, xbool& IsDisabled, base::prop_type& Type )
{
    switch( Idx ) 
    {
        case 0: 
            //============================================================================
            Name.Format( "Instanceable" );
            Value.Format( "%s", m_Instanceable == TRUE ? "true" : "false" );
            Type        = PROP_BOOL;
            IsDisabled  = FALSE;
            return TRUE;
        case 1:
            //============================================================================
            Name.Format( "Background" );
            Value       = m_BackgroundName;
            Type        = PROP_FILENAME;
            IsDisabled  = FALSE;
            return TRUE;
        case 2:
            //============================================================================
            Name.Format( "Export PC" );
            Value       = m_ExportPC;
            Type        = PROP_FILENAME;
            IsDisabled  = FALSE;
            return TRUE;
        case 3:
            //============================================================================
            Name.Format( "Export GCN" );
            Value       = m_ExportGCN;
            Type        = PROP_FILENAME;
            IsDisabled  = FALSE;
            return TRUE;
        case 4:
            //============================================================================
            Name.Format( "Export PS2" );
            Value       = m_ExportPS2;
            Type        = PROP_FILENAME;
            IsDisabled  = FALSE;
            return TRUE;
        case 5:
            //============================================================================
            Name.Format( "Export XBOX" );
            Value       = m_ExportXBOX;
            Type        = PROP_FILENAME;
            IsDisabled  = FALSE;
            return TRUE;
        default:
            return FALSE;
    }

    return FALSE;

}

//============================================================================

xbool effect::OnPropertyChanged( s32 T, xstring& Field, xstring& Value )
{
    xbool   Dirty = FALSE;

    if( x_strstr(Field, "Instanceable") )
    {
        m_Instanceable = x_strcmp( Value, "true") == 0 ? TRUE : FALSE;

        if( m_Instanceable )
        {
            for( int i = 0; i < m_Elements.GetCount(); i++ )
            {
                if( strcmp( m_Elements[i]->GetType(), "Spemitter" ) == 0 )
                {
                    ( (element_spemitter*)m_Elements[i] )->SetWorldSpace( false );
                }
            }
        }

        Dirty = TRUE;
    }
    else if( x_strstr(Field, "Background") )
    {
        m_BackgroundName = Value;
        Dirty = TRUE;
    }
    else if( x_strstr(Field, "Export PC") )
    {
        m_ExportPC = Value;
        Dirty = TRUE;
    }
    else if( x_strstr(Field, "Export GCN") )
    {
        m_ExportGCN = Value;
        Dirty = TRUE;
    }
    else if( x_strstr(Field, "Export PS2") )
    {
        m_ExportPS2 = Value;
        Dirty = TRUE;
    }
    else if( x_strstr(Field, "Export XBOX") )
    {
        m_ExportXBOX = Value;
        Dirty = TRUE;
    }

    return Dirty;
}

//============================================================================

void effect::Load( igfmgr& Igf )
{
    // Read the effect

    s32 Version;

    hfield VerField = Igf.Find( "Version" );
    if ( VerField )
        Version = Igf.GetS32();

    // ASSERT( Version == EFFECT_VER );

    hfield NewGrp;

    NewGrp = Igf.GetGroup( "Settings" );
    Igf.EnterGroup( NewGrp );
    {
        m_BackgroundName    = Igf.GetString( "Background"   );
        m_ExportPC          = Igf.GetString( "Export_PC"    );
        m_ExportGCN         = Igf.GetString( "Export_GCN"   );
        m_ExportPS2         = Igf.GetString( "Export_PS2"   );
        m_ExportXBOX        = Igf.GetString( "Export_XBOX"  );

        m_Instanceable      = ( Version > 2 ) ? Igf.GetBool( "Instanceable" ) : false;  // New in version 3

        Igf.ExitGroup();
    }


    // Read in the controllers
    NewGrp = Igf.GetGroup( "Controllers" );
    Igf.EnterGroup( NewGrp );
    {
        s32 Count = Igf.GetS32( "Count" );
        s32 i;

        // go to the first controller
        NewGrp = Igf.Find( "Controller" );

        for ( i = 0; i < Count; i++ )
        {
            // enter the group
            Igf.EnterGroup( NewGrp );

            // what kind of controller is it?
            const char* pType = Igf.GetString("Type");
            controller* pController = MakeController( pType );
            pController->LoadData( Igf );
            m_Controllers.Append( pController );
            Igf.ExitGroup();
            NewGrp = Igf.Next();
        }
        
        Igf.ExitGroup();
    }

    // Read the elements
    NewGrp = Igf.GetGroup( "Elements" );
    Igf.EnterGroup( NewGrp );
    {
        s32 Count = Igf.GetS32   ( "Count" );
        s32 i;

        // go to first element
        NewGrp = Igf.Find( "Element" );

        for ( i = 0; i < Count; i++ )
        {
            // enter the group
            Igf.EnterGroup( NewGrp );

            // what kind of controller is it?
            const char* pType = Igf.GetString( "Type" );

            if( !m_IgnoreMeshes || (x_stricmp( "MESH", pType ) != 0) )
            {
                element* pNewElem = MakeElement( pType );

                pNewElem->Load( Igf );

                // fixup controller pointers
                {
                    s32 j;

                    for ( j = 0; j < m_Controllers.GetCount(); j++ )
                    {
                        if ( m_Controllers[j]->GetSerNum() == (u32)pNewElem->m_pScale )
                            pNewElem->m_pScale = (ctrl_linear*)m_Controllers[j];
                        else
                        if ( m_Controllers[j]->GetSerNum() == (u32)pNewElem->m_pRotation )
                            pNewElem->m_pRotation = (ctrl_linear*)m_Controllers[j];
                        else
                        if ( m_Controllers[j]->GetSerNum() == (u32)pNewElem->m_pTranslation )
                            pNewElem->m_pTranslation = (ctrl_linear*)m_Controllers[j];
                        else
                        if ( m_Controllers[j]->GetSerNum() == (u32)pNewElem->m_pColor )
                            pNewElem->m_pColor = (ctrl_linear*)m_Controllers[j];
                        else
                        if ( m_Controllers[j]->GetSerNum() == (u32)pNewElem->m_pAlpha )
                            pNewElem->m_pAlpha = (ctrl_linear*)m_Controllers[j];
                    }
                }
                pNewElem->m_pEffect = this;
                m_Elements.Append( pNewElem );
            }

            Igf.ExitGroup();
            NewGrp = Igf.Next();
        }

        Igf.ExitGroup();
    }
}

//============================================================================
void effect::Save( igfmgr& Igf )
{
    // Write effect

    Igf.AddS32( "Version", EFFECT_VER );

    hfield NewGrp;
    
    NewGrp = Igf.AddGroup( "Settings", "Effect Settings" );
    Igf.EnterGroup( NewGrp );
    {
        Igf.AddString( "Background", (const char*)m_BackgroundName );
        Igf.AddString( "Export_PC",     m_ExportPC     );
        Igf.AddString( "Export_GCN",    m_ExportGCN    );
        Igf.AddString( "Export_PS2",    m_ExportPS2    );
        Igf.AddString( "Export_XBOX",   m_ExportXBOX   );
        Igf.AddBool  ( "Instanceable",  m_Instanceable );
        
        Igf.ExitGroup();
    }

    // Write the controllers
    NewGrp = Igf.AddGroup( "Controllers", "All the controllers" );
    Igf.EnterGroup( NewGrp );
    {
        Igf.AddS32( "Count", m_Controllers.GetCount() );
        
        s32 i, Count;
        Count = m_Controllers.GetCount();

        for ( i = 0; i < Count; i++ )
        {
            controller* pController = m_Controllers[i];
            pController->SaveData( Igf );
        }

        Igf.ExitGroup();
    }

    // Write the elements
    NewGrp = Igf.AddGroup( "Elements", "All of the elements" );
    Igf.EnterGroup( NewGrp );

    Igf.AddS32   ( "Count", m_Elements.GetCount() );

    s32 i, Count;
    Count = m_Elements.GetCount();
    for ( i = 0; i < Count; i++ )
    {
        element* pElement = m_Elements[i];
        pElement->Save( Igf );
    }
    
    Igf.ExitGroup();
}

//============================================================================

void effect::Destroy( void )
{
    s32 i, Count;
    
    // Destroy all elements
    Count = m_Elements.GetCount();
    
    for ( i = 0; i < Count; i++ )
    {
        element* pElement = m_Elements[i];
        delete pElement;
    }
    m_Elements.Clear();

    // Destroy all controllers
    Count = m_Controllers.GetCount();
    
    for ( i = 0; i < Count; i++ )
    {
        controller* pController = m_Controllers[i];
        delete pController;
    }

    m_Controllers.Clear();
}

//============================================================================

void effect::FlagAllTextures( void )
{
    s32 i, Count;
    
    Count = m_Elements.GetCount();

    for ( i = 0; i < Count; i++ )
    {
        element* pElement = m_Elements[i];
        pElement->FlagExportTextures();
    }
}

//============================================================================

void effect::ActivateAllTextures( void )
{
    s32 i, Count;
    
    Count = m_Elements.GetCount();

    for ( i = 0; i < Count; i++ )
    {
        element* pElement = m_Elements[i];
        pElement->ActivateTextures();
    }
}

//============================================================================

element* effect::FindElementByName( const char* pElemName )
{
    s32 i, Count;
    
    Count = m_Elements.GetCount();

    for ( i = 0; i < Count; i++ )
    {
        element* pElement = m_Elements[i];
        char ID[64];
        
        pElement->GetElementID( ID );
        
        if ( x_stricmp(ID, pElemName) == 0 )
        {
            return pElement;
        }
    }

    return NULL;
}

//============================================================================

void effect::SetAllElementsSelected( xbool State )
{
    for( s32 i=0 ; i<m_Elements.GetCount() ; i++ )
    {
        m_Elements[i]->SetSelected( State );
    }
}

//============================================================================

void effect::GetBookends( s32& FirstKey, s32& LastKey )
{
    s32 i, Count;
    s32 TmpFirst, TmpLast;
    s32 First = 999999, Last = -999999;
    
    Count = m_Elements.GetCount();

    for ( i = 0; i < Count; i++ )
    {
        element* pElement = m_Elements[i];
        pElement->GetBookends( TmpFirst, TmpLast );

        First = MIN( First, TmpFirst );
        Last =  MAX( Last, TmpLast );
    }

    FirstKey = First;
    LastKey = Last;
}

s32 effect::GetLifeSpan( void )
{
    s32 i, Count;
    s32 TmpFirst, TmpLast;
    s32 Last = -999999;
    f32 MaxPartLife = 0.0f;
    
    Count = m_Elements.GetCount();

    for ( i = 0; i < Count; i++ )
    {
        element* pElement = m_Elements[i];
        pElement->GetBookends( TmpFirst, TmpLast );

        Last =  MAX( Last, TmpLast );

        if ( x_strcmp(pElement->GetType(), "Spemitter") == 0 )
        {
            element_spemitter* pSpem = (element_spemitter*)pElement;
            MaxPartLife = MAX( pSpem->GetParticleLife(), MaxPartLife );
        }
    }

    return Last + (s32)( MaxPartLife * 30.0f ) + 10;
}

} // namespace fx_core
