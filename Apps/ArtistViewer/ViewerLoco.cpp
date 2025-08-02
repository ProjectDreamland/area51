//==============================================================================
//
//  File:           ViewerLoco.hpp
//
//  Description:    Viewer locomotion class
//
//  Author:         Stephen Broumley
//
//  Date:           Started August 1st, 2003 (C) Inevitable Entertainment Inc.
//
//==============================================================================


//==============================================================================
// INCLUDES
//==============================================================================

#include "ViewerLoco.hpp"
#include "Main.hpp"
#include "ConfigFile.hpp"
#include "EventMgr\EventMgr.hpp"
#include "MeshUtil\RawSettings.hpp"


//==============================================================================
// FUNCTIONS
//==============================================================================

viewer_loco::viewer_loco( void ) :
    loco(),
    m_pConfigObject( NULL),
    m_PlayAnim( *this ),
    m_Idle    ( *this ),
    m_Move    ( *this ),
    m_bHasIdleAnims(FALSE),
    m_bHasMoveAnims(FALSE),
    m_bHasLipSyncAnims(FALSE),
    m_bWarnings(FALSE)
{
}

//==============================================================================

viewer_loco::~viewer_loco()
{
}

//=========================================================================
// STATE_PLAY ANIM
//=========================================================================

viewer_loco_play_anim::viewer_loco_play_anim( loco& Loco ) :
    loco_play_anim(Loco)
{
}

//=========================================================================
// STATE_IDLE
//=========================================================================

viewer_loco_idle::viewer_loco_idle( loco& Loco ) :
    loco_idle(Loco)
{
}

//=========================================================================
// STATE_MOVE
//=========================================================================

viewer_loco_move::viewer_loco_move( loco& Loco ) :
    loco_move(Loco)
{
}


//==============================================================================
// LOCOMOTION
//==============================================================================

void viewer_loco::FindAnim( loco::anim_type Type, const char* pNotThis, const char* pFindThis )
{
    // Already found?
    if (m_AnimLookupTable.m_Index[Type] != -1)
        return ;

    // Lookup anim group
    const anim_group* pAnimGroup = GetAnimGroupHandle().GetPointer() ;
    if (!pAnimGroup)
        return ;

    // Search all anims
    for (s32 i = 0 ; i < pAnimGroup->GetNAnims() ; i++)
    {
        // Lookup full name
        const char* pAnim = pAnimGroup->GetAnimInfo(i).GetName() ;
   
        // If not string is there, then skip
        if ((pNotThis) && (x_stristr(pAnim, pNotThis)))
            continue ;

        // Found?
        if (x_stristr(pAnim, pFindThis))
        {
            // Setup entry
            ASSERT(Type >= 0) ;
            ASSERT(Type < ANIM_TOTAL) ;
            m_AnimLookupTable.m_Index[Type] = i ;
            return ;
        }
    }
}


//==============================================================================

void viewer_loco::CheckForProperty ( const geom*                    pGeom,
                                     const char*                    pSectionName, 
                                     const char*                    pPropertyName,
                                           geom::property::type     PropertyType )
{
    xbool bFound = FALSE;
    
    // Search for section
    const geom::property_section* pSection = pGeom->FindPropertySection( pSectionName );
    if( pSection )
    {
        // Search for property
        const geom::property* pProperty = pGeom->FindProperty( pSection, pPropertyName, PropertyType );
        if( pProperty )
            bFound = TRUE;
    }
    
    // Missing?
    if( !bFound )
    {
        const char* pType = "";
        switch( PropertyType )
        {
        case geom::property::TYPE_FLOAT:    pType = "FLOAT";    break;        
        case geom::property::TYPE_INTEGER:  pType = "INTEGER";  break;        
        case geom::property::TYPE_ANGLE:    pType = "ANGLE";    break;        
        case geom::property::TYPE_STRING:   pType = "STRING";   break;        
        }   
        
        x_printf( "Missing %s %s %s\n", pSectionName, pPropertyName, pType );
        m_bWarnings = TRUE;
    }
}

//==============================================================================

void  viewer_loco::OnInit( const geom*      pGeom, 
                           const char*      pAnimFileName, 
                           config_options::object&  Object )
{
    s32 i ;

    // Clear warnings
    m_bWarnings = FALSE;

    // Make sure properties exist for loco type only
    if( Object.m_Type == config_options::TYPE_LOCO )
    {
        // Make sure aimer properties are present
        CheckForProperty( pGeom, "AIMER", "BlendSpeed",         geom::property::TYPE_FLOAT );
        CheckForProperty( pGeom, "AIMER", "HorizMin",           geom::property::TYPE_ANGLE );
        CheckForProperty( pGeom, "AIMER", "HorizMax",           geom::property::TYPE_ANGLE );
        CheckForProperty( pGeom, "AIMER", "VertMin",            geom::property::TYPE_ANGLE );
        CheckForProperty( pGeom, "AIMER", "VertMax",            geom::property::TYPE_ANGLE );
        CheckForProperty( pGeom, "AIMER", "EyeBlendSpeed",      geom::property::TYPE_FLOAT );
        
        CheckForProperty( pGeom, "IDLE",  "DeltaYawMin",        geom::property::TYPE_ANGLE );
        CheckForProperty( pGeom, "IDLE",  "DeltaYawMax",        geom::property::TYPE_ANGLE );
        CheckForProperty( pGeom, "IDLE",  "TurnDeltaYawMin",    geom::property::TYPE_ANGLE );
        CheckForProperty( pGeom, "IDLE",  "TurnDeltaYawMax",    geom::property::TYPE_ANGLE );
        CheckForProperty( pGeom, "IDLE",  "Turn180DeltaYawMin", geom::property::TYPE_ANGLE );
        CheckForProperty( pGeom, "IDLE",  "Turn180DeltaYawMax", geom::property::TYPE_ANGLE );

        // Check for move style properties
        for( i = 0; i < loco::MOVE_STYLE_COUNT; i++ )
        {
            // Lookup info
            const char* pMoveStyle = GetMoveStyleName( i );

            // Setup move style defaults?
            CheckForProperty( pGeom, pMoveStyle, "IdleBlendTime",          geom::property::TYPE_FLOAT );
            CheckForProperty( pGeom, pMoveStyle, "MoveBlendTime",          geom::property::TYPE_FLOAT );
            CheckForProperty( pGeom, pMoveStyle, "FromPlayAnimBlendTime",  geom::property::TYPE_FLOAT );
            CheckForProperty( pGeom, pMoveStyle, "MoveTurnRate",           geom::property::TYPE_ANGLE );
        }
    }
    
    // Load settings?
    raw_settings RawSettings;
    const char* pSettingsFile = Object.m_Geoms[0].m_SettingsFile;
    if( pSettingsFile[0] )
        RawSettings.Load( pSettingsFile );

    // Override properties
    for( s32 iSection = 0; iSection < RawSettings.m_PropertySections.GetCount(); iSection++ )
    {
        const raw_settings::property_section& Section = RawSettings.m_PropertySections[ iSection ];

        for( s32 iProperty = 0; iProperty < Section.nProperties; iProperty++ )
        {
            const raw_settings::property& Property = RawSettings.m_Properties[ Section.iProperty + iProperty ];

            // Lookup type
            geom::property::type Type = geom::property::TYPE_TOTAL;
            if( Property.Type == "FLOAT" )
            {
                Type = geom::property::TYPE_FLOAT;
            }                                
            else if( Property.Type == "INTEGER" )
            {
                Type = geom::property::TYPE_INTEGER;
            }
            else if( Property.Type == "ANGLE"  )
            {
                Type = geom::property::TYPE_ANGLE;
            }
            else if( Property.Type == "STRING" ) 
            {
                Type = geom::property::TYPE_STRING;
            }

            // Error?
            if( Type == geom::property::TYPE_TOTAL )
            {
                x_printf( "Could't find prop type \"%s\"\n", (const char*)Property.Type );
                m_bWarnings = TRUE;
            }
            else
            {
                // Lookup property
                geom::property*         pProp    = NULL;
                geom::property_section* pSection = (geom::property_section*)pGeom->FindPropertySection( Section.Name );
                if( pSection )
                    pProp = (geom::property*)pGeom->FindProperty( pSection, Property.Name, Type );

                // Not found?
                if( pProp == NULL )
                {
                    x_printf( "Could't find \"%s\\%s\" - Recompile .skingeom!\n", (const char*)Section.Name, (const char*)Property.Name );
                    m_bWarnings = TRUE;
                }                                
                else                        
                {
                    switch( Type )
                    {
                    case geom::property::TYPE_FLOAT:
                        pProp->Value.Float = x_atof( Property.Value );
                        break;
                    case geom::property::TYPE_INTEGER:
                        pProp->Value.Integer = x_atoi( Property.Value );
                        break;
                    case geom::property::TYPE_ANGLE:
                        pProp->Value.Angle = DEG_TO_RAD( x_atof( Property.Value ) );
                        break;
                    }                                        
                }
            }
        }
    }

    // Call base class
    loco::OnInit( pGeom, pAnimFileName, NULL_GUID ) ;

    // Keep config object
    m_pConfigObject = &Object ;

    // Get anim group
    const anim_group* pAnimGroup = GetAnimGroupHandle().GetPointer() ;
    if (!pAnimGroup)
        return ;

    // Override bone masks using settings
    for( s32 iBoneMasks = 0; iBoneMasks < RawSettings.m_BoneMasks.GetCount(); iBoneMasks++ )
    {
        const raw_settings::bone_masks& BoneMasks = RawSettings.m_BoneMasks[ iBoneMasks ];
        geom::bone_masks* pBoneMasks = (geom::bone_masks*)pGeom->FindBoneMasks( BoneMasks.Name );
        if( pBoneMasks == NULL )
        {
            x_printf( "Couldn't find \"%s\" bone masks in .skingeom - rebuild!\n", (const char*)BoneMasks.Name );
            m_bWarnings = TRUE;
        }
        else                
        {
            // Clear all masks to zero ready for loading
            s32 iMask;
            for( iMask = 0; iMask < MAX_ANIM_BONES; iMask++ )
                pBoneMasks->Weights[ iMask ] = 0.0f;

            // Setup bone masks    
            for( iMask = 0; iMask < BoneMasks.Masks.GetCount(); iMask++ )
            {
                const raw_settings::bone_masks::mask& Mask = BoneMasks.Masks[iMask];

                // Lookup bone index
                s32 iBone = pAnimGroup->GetBoneIndex( Mask.BoneName, TRUE );                                                    
                if( iBone == -1 )
                {
                    //x_printf("Couldn't find bone \"%s\"\n", (const char*)Mask.BoneName );
                    //m_bWarnings = TRUE;
                }   
                else
                {
                    // Overwrite value
                    pBoneMasks->Weights[ iBone ] = Mask.Weight;
                }                         
            }
        }
    }

    // Loop through all styles
    for (i = 0 ; i < loco::MOVE_STYLE_COUNT ; i++)
    {
        // Lookup move style info
        const char* pStyle     = loco::GetMoveStyleName(i) ;
        s32         iAnimStyle = MOVE_STYLE_ANIM_COUNT * i ;

        // Lookup all anims for this style
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_IDLE),                   "turn",     xfs("%s_IDLE",  pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_IDLE),                   "turn",     xfs("%s_STAND", pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_IDLE),                   "turn",     xfs("%sIDLE",   pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_IDLE),                   "turn",     xfs("%sSTAND",  pStyle)) ;
                                                                
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_IDLE_FIDGET),            "turn",     xfs("%s_IDLE_FIDGET", pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_IDLE_FIDGET),            "turn",     xfs("%sIDLEFIDGET",   pStyle)) ;
                                                                
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_IDLE_TURN_LEFT),         NULL,       xfs("%s_IDLE_TURN_LEFT", pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_IDLE_TURN_LEFT),         NULL,       xfs("%s_TURN_LEFT",      pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_IDLE_TURN_LEFT),         NULL,       xfs("%sIDLETURNLEFT",    pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_IDLE_TURN_LEFT),         NULL,       xfs("%sTURNLEFT",        pStyle)) ;
                                                                
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_IDLE_TURN_RIGHT),        NULL,       xfs("%s_IDLE_TURN_RIGHT", pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_IDLE_TURN_RIGHT),        NULL,       xfs("%s_TURN_RIGHT",      pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_IDLE_TURN_RIGHT),        NULL,       xfs("%sIDLETURNRIGHT",    pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_IDLE_TURN_RIGHT),        NULL,       xfs("%sTURNRIGHT",        pStyle)) ;
   
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_IDLE_TURN_LEFT_180),     NULL,       xfs("%s_IDLE_TURN_LEFT_180", pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_IDLE_TURN_LEFT_180),     NULL,       xfs("%s_TURN_LEFT_180",      pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_IDLE_TURN_LEFT_180),     NULL,       xfs("%sIDLETURNLEFT180",     pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_IDLE_TURN_LEFT_180),     NULL,       xfs("%sTURNLEFT180",         pStyle)) ;

        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_IDLE_TURN_RIGHT_180),    NULL,       xfs("%s_IDLE_TURN_RIGHT_180",   pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_IDLE_TURN_RIGHT_180),    NULL,       xfs("%s_TURN_RIGHT_180",        pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_IDLE_TURN_RIGHT_180),    NULL,       xfs("%sIDLETURNRIGHT180",       pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_IDLE_TURN_RIGHT_180),    NULL,       xfs("%sTURNRIGHT180",           pStyle)) ;

        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_IDLE_TURN_180),          NULL,       xfs("%s_IDLE_TURN_180", pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_IDLE_TURN_180),          NULL,       xfs("%s_TURN_180",      pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_IDLE_TURN_180),          NULL,       xfs("%sIDLETURN180",    pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_IDLE_TURN_180),          NULL,       xfs("%sTURN180",        pStyle)) ;
                                                                
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_FRONT),             "turn",     xfs("%s_FORWARD",    pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_FRONT),             "turn",     xfs("%s_FRONT",      pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_FRONT),             "turn",     xfs("%sFORWARD",     pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_FRONT),             "turn",     xfs("%sFRONT",       pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_FRONT),             "turn",     xfs("%s_MOVE_FRONT", pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_FRONT),             "turn",     xfs("%sMOVE_FRONT",  pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_FRONT),             "turn",     xfs("%sMOVEFRONT",   pStyle)) ;
                                                                
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_BACK),              "turn",     xfs("%s_BACKWARD",   pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_BACK),              "turn",     xfs("%s_BACK",       pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_BACK),              "turn",     xfs("%sBACKWARD",    pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_BACK),              "turn",     xfs("%sBACK",        pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_BACK),              "turn",     xfs("%s_MOVE_BACK",  pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_BACK),              "turn",     xfs("%sMOVE_BACK",   pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_BACK),              "turn",     xfs("%sMOVEBACK",    pStyle)) ;
                                                                
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_LEFT),              "turn",     xfs("%s_STRAFE_L",   pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_LEFT),              "turn",     xfs("%s_LEFT",       pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_LEFT),              "turn",     xfs("%sSTRAFEL",     pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_LEFT),              "turn",     xfs("%sLEFT",        pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_LEFT),              "turn",     xfs("%s_MOVE_LEFT",  pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_LEFT),              "turn",     xfs("%sMOVE_LEFT",   pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_LEFT),              "turn",     xfs("%sMOVELEFT",    pStyle)) ;
                                                                
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_RIGHT),             "turn",     xfs("%s_STRAFE_R",   pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_RIGHT),             "turn",     xfs("%s_RIGHT",      pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_RIGHT),             "turn",     xfs("%sSTRAFER",     pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_RIGHT),             "turn",     xfs("%sRIGHT",       pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_RIGHT),             "turn",     xfs("%s_MOVE_RIGHT", pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_RIGHT),             "turn",     xfs("%sMOVE_RIGHT",  pStyle)) ;
        FindAnim((loco::anim_type)(iAnimStyle + MOVE_STYLE_ANIM_MOVE_RIGHT),             "turn",     xfs("%sMOVERIGHT",   pStyle)) ;
    }

    // Check for idle/move anims
    m_bHasIdleAnims = FALSE ;
    m_bHasMoveAnims = FALSE ;
    for (i = 0 ; i < MOVE_STYLE_COUNT ; i++)
    {
        move_style Style = (move_style)i ;
        m_bHasIdleAnims |= IsValidIdleStyle(Style) ;
        m_bHasMoveAnims |= IsValidMotionStyle(Style) ;
    }

    // Check for lip sync anims
    for (i = 0 ; i < pAnimGroup->GetNAnims() ; i++)
    {
        // Lookup anim name
        const char* pAnimName = pAnimGroup->GetAnimInfo(i).GetName() ;

        // Lip sync?
        if ( (x_stristr(pAnimName, "LIP_SYNC")) || (x_stristr(pAnimName, "LIPSYNC")) )
            m_bHasLipSyncAnims = TRUE ;
    }
        
    // Initialize aimer
    SetAimerBoneMasks( FALSE, 0.0f ) ;   // Use no aim bone bone masks

    // Put into idle state
    SetState(loco::STATE_IDLE) ;

    // Set start move style
    for (i = 0 ; i < MOVE_STYLE_COUNT ; i++)
    {
        move_style Style = (move_style)i ;
        if (IsValidMoveStyle(Style))
        {
            SetMoveStyle(Style) ;
            break ;
        }
    }
    
    // Turn off the aimer/eye tracking?
    if( Object.m_bUseAimer == FALSE )
    {
        m_Player.SetTrack( 6, NULL );   // Aim controller
        m_Player.SetTrack( 7, NULL );   // Additive eye controller
        SetFaceIdleEnabled( FALSE );
    }
}

//==============================================================================

xbool viewer_loco::IsValidIdleStyle ( move_style Style )
{
    // Is idle present?
    s32 iIdle = MOVE_STYLE_ANIM_IDLE + (loco::MOVE_STYLE_ANIM_COUNT * Style) ;
    return (m_AnimLookupTable.m_Index[iIdle] != -1) ;
}

//==============================================================================

xbool viewer_loco::IsValidMotionStyle ( move_style Style )
{
    // Lookup indices
    s32 iFront = MOVE_STYLE_ANIM_MOVE_FRONT + (loco::MOVE_STYLE_ANIM_COUNT * Style) ;
    s32 iBack  = MOVE_STYLE_ANIM_MOVE_BACK  + (loco::MOVE_STYLE_ANIM_COUNT * Style) ;
    s32 iLeft  = MOVE_STYLE_ANIM_MOVE_LEFT  + (loco::MOVE_STYLE_ANIM_COUNT * Style) ;
    s32 iRight = MOVE_STYLE_ANIM_MOVE_RIGHT + (loco::MOVE_STYLE_ANIM_COUNT * Style) ;
    
    // If any direction is there, then there is motion!
    return (m_AnimLookupTable.m_Index[iFront] != -1) ||
           (m_AnimLookupTable.m_Index[iBack ] != -1) ||
           (m_AnimLookupTable.m_Index[iLeft ] != -1) ||
           (m_AnimLookupTable.m_Index[iRight] != -1) ;
}

//==============================================================================

xbool viewer_loco::IsValidMoveStyle ( move_style Style )
{
    return (IsValidMotionStyle(Style)) || (IsValidIdleStyle(Style)) ;
}

//==============================================================================

loco::move_style viewer_loco::GetValidMoveStyle( loco::move_style Style )
{
    if ( (IsValidMotionStyle(Style)) || (IsValidIdleStyle(Style)) )
        return Style ;
    else
        return loco::MOVE_STYLE_NULL ;
}

//==============================================================================

// Sets move anims, turn rate etc
void viewer_loco::SetMoveStyle( move_style Style )
{
    if (Style == MOVE_STYLE_NULL)
        return ;

    // Call base class
    loco::SetMoveStyle(Style) ;

    // Set bone masks
    switch(Style)
    {
        case MOVE_STYLE_RUNAIM:
        case MOVE_STYLE_CROUCHAIM:
            SetAimerBoneMasks( TRUE, 0.5f ) ;   // Use aim bone bone masks
            break ;

        default:
            SetAimerBoneMasks( FALSE, 0.5f ) ;   // Use no aim bone bone masks
            break ;
    }
}

//==============================================================================

// Returns TRUE if idle anims are present
xbool viewer_loco::HasIdleAnims ( void ) const
{
    return m_bHasIdleAnims ;
}
//==============================================================================

// Returns TRUE if move anims are present
xbool viewer_loco::HasMoveAnims ( void ) const
{
    return m_bHasMoveAnims ;
}

//==============================================================================

// Returns TRUE if lip sync anims are present
xbool viewer_loco::HasLipSyncAnims ( void ) const
{
    return m_bHasLipSyncAnims ;
}

//==============================================================================

xbool viewer_loco::HasWarnings( void ) const
{
    return m_bWarnings;
}

//==============================================================================

void viewer_loco::SendEvents( object* pOwner )
{
    g_EventMgr.HandleSuperEvents( m_Player, pOwner );
    g_EventMgr.HandleSuperEvents( m_Player, m_MaskController,        pOwner );
    g_EventMgr.HandleSuperEvents( m_Player, m_AdditiveController[0], pOwner );
    g_EventMgr.HandleSuperEvents( m_Player, m_AdditiveController[1], pOwner );
    g_EventMgr.HandleSuperEvents( m_Player, m_LipSyncController,     pOwner );
}

//==============================================================================

void draw_Arc( const vector3& Center, f32 Radius, radian Angle0, radian Angle1, xcolor Color )
{
    s32     nSegs = 1 + ( (s32)( x_abs( Angle0 - Angle1 ) / R_5 ) );
    radian  Delta = (Angle1 - Angle0) / nSegs;
    radian  Angle = Angle0;

    draw_Begin( DRAW_TRIANGLES, DRAW_USE_ALPHA | DRAW_NO_ZBUFFER | DRAW_NO_ZWRITE | DRAW_CULL_NONE );
    draw_Color( Color );
    for( s32 i = 0; i < nSegs; i++ )
    {
        // Compute edge pts
        vector3 P0( 0,0,Radius );
        P0.RotateY( Angle );

        vector3 P1( 0,0,Radius );
        P1.RotateY( Angle + Delta );

        // Draw triangle
        draw_Vertex( Center );
        draw_Vertex( Center + P0 );
        draw_Vertex( Center + P1 );

        // Next
        Angle += Delta;
    }
    draw_End();
}

//==============================================================================

// Render
void viewer_loco::Render( void )
{
    // Lookup info
    const vector3& Pos    = GetPosition() ;
    const vector3& LookAt = GetHeadLookAt() ;
    const vector3& MoveAt = GetMoveAt() ;
    radian  Yaw = GetYaw();

    // Draw limits                
    static xcolor C0( 128,128,0,255 ) ;
    static xcolor C1( 255,255,0,255 ) ;

    static xcolor C2( 0,200,0,255 ) ;
    static xcolor C3( 0,255,0,255 ) ;

    static xcolor C4( 0,128,128,255 ) ;
    static xcolor C5( 0,255,255,255 ) ;

    static xcolor C6( 128,0,0,255 ) ;
    static xcolor C7( 255,0,0,255 ) ;

    // Draw aimer limits
    draw_Arc( Pos, 120, Yaw, Yaw + m_AimController.GetHorizMinLimit(), C6 );
    draw_Arc( Pos, 120, Yaw, Yaw + m_AimController.GetHorizMaxLimit(), C7 );

    // Draw idle delta limits
    draw_Arc( Pos, 99, Yaw, Yaw + m_MoveStyleInfo.m_IdleDeltaYawMin, C0 );
    draw_Arc( Pos, 99, Yaw, Yaw + m_MoveStyleInfo.m_IdleDeltaYawMax, C1 );

    // Draw idle turn delta limits
    draw_Arc( Pos, 66, Yaw, Yaw + m_MoveStyleInfo.m_IdleTurnDeltaYawMin, C2 );
    draw_Arc( Pos, 66, Yaw, Yaw + m_MoveStyleInfo.m_IdleTurnDeltaYawMax, C3 );

    // Draw idle turn 180 delta limits
    draw_Arc( Pos, 33, Yaw, Yaw + m_MoveStyleInfo.m_IdleTurn180DeltaYawMin, C4 );
    draw_Arc( Pos, 33, Yaw, Yaw + m_MoveStyleInfo.m_IdleTurn180DeltaYawMax, C5 );
    
    // Draw move at
    Util_DrawMarker(GetMoveAt(), XCOLOR_BLUE) ;
    draw_Line(Pos, MoveAt, XCOLOR_BLUE) ;

    // Draw look at
    draw_Line(GetEyePosition(), LookAt, XCOLOR_YELLOW) ;
    Util_DrawMarker(LookAt, XCOLOR_GREEN) ;
    Util_DrawMarker(vector3(LookAt.GetX(), 0, LookAt.GetZ()), XCOLOR_GREEN) ;
    draw_Line(LookAt, vector3(LookAt.GetX(), 0, LookAt.GetZ()), XCOLOR_GREEN) ;
    draw_Line(GetPosition(), vector3(LookAt.GetX(), 0, LookAt.GetZ()), XCOLOR_GREEN) ;

    // Draw facing dir
    vector3 Dir(0,0,150) ;
    Dir.RotateY( Yaw );
    draw_Line( Pos, Pos+Dir, XCOLOR_PURPLE ) ;
    Util_DrawMarker(Pos, XCOLOR_PURPLE ) ;
    
    // Draw aimer dir
    Dir.Set(0,0,150) ;
    Dir.RotateY( Yaw + m_AimController.GetHorizAim() );
    draw_Line( Pos, Pos+Dir, XCOLOR_RED );
}

//==============================================================================
