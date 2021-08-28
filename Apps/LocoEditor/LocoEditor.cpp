#include "stdafx.h"
#include "LocoEditor.hpp"
#include "Parsing\TextIn.hpp"
#include "Parsing\TextOut.hpp"
#include "..\Support\Animation\AnimData.hpp"

//=========================================================================
// LOCAL
//=========================================================================
DEFINE_RSC_TYPE( s_Desc, animation_desc, "anim", "Animation Files (*.anim)|*.anim", "A package containing a list of animations for a particular skeleton" );

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

animation_desc::animation_desc( void ) : rsc_desc( s_Desc ) 
{
    m_BindPose[0] = 0;
    m_bGenericAnim = FALSE;
    m_bDisableWarnings = FALSE;
}

//=========================================================================

void animation_desc::OnEnumProp( prop_enum& List )
{
    rsc_desc::OnEnumProp( List );

    List.PropEnumFileName( "ResDesc\\BindPose", 
                            "Exported 3DMax Files (*.matx)|*.matx|All Files (*.*)|*.*||", 
                            "This raw mesh needs to have the zero pose of the mesh", 0 );

    List.PropEnumBool     ( "ResDesc\\GenericAnim", 
                            "A generic animation is an animation which is not descriving an specific character, or object. This type of animations are use for things such camera paths, or moving instances around the world. So usually this should be off.", 0 );

    List.PropEnumButton   ( "ResDesc\\SortAnimations", 
                            "Press the button to sort the animation in alphabetical order (Uses the name field not the matx).",
                            PROP_TYPE_MUST_ENUM );

    List.PropEnumInt     ( "ResDesc\\AnimCount", 
                            "If you change the count you can add more animations.",
                            PROP_TYPE_MUST_ENUM );

    List.PropEnumButton  ( "ResDesc\\Add Anim", 
                            "Adds a new animation into the list.",
                            PROP_TYPE_MUST_ENUM );


    for( s32 i=0; i<m_lAnimInfo.GetCount(); i++ )
    {
        List.PropEnumString( xfs("ResDesc\\AnimList[%d]", i ), "Animation properties", PROP_TYPE_HEADER );

        s32 iHeader = List.PushPath( xfs("ResDesc\\AnimList[%d]\\", i) ) ;

        List.PropEnumFileName( "FileName", 
                                "Exported 3DMax Files (*.matx)|*.matx|All Files (*.*)|*.*||", 
                                "This is the actual file name for the animation", PROP_TYPE_MUST_ENUM );

        List.PropEnumString  ( "Name"                            , "This is the name for this animation. Unlike the matx name this name is used to link with the system.", 0 );
        List.PropEnumBool    ( "Loop"                            , "Can this animation loop.", 0 );
        List.PropEnumInt     ( "FPS"                             , "What frame rate should this animation be set at", 0 );
        List.PropEnumFloat   ( "Downsample"                      , "0.0 - no frames remaining, 1.0 - all frames remaining", 0 );
        
        List.PropEnumHeader  ( "Playback"                        , "Playback properties", 0 );
        List.PropEnumFloat   ( "Playback\\BlendTime"             , "Time in seconds to blend to this anim (-1 = use engine default).", 0 ) ;
        List.PropEnumFloat   ( "Playback\\SelectionWeight"       , "Influence weight when randomly choosing from anims with the same name.", 0 ) ;
        List.PropEnumInt     ( "Playback\\LoopFrame"             , "Loop frame to jump to (only applies if animation loop is set to true).", 0 );
        List.PropEnumInt     ( "Playback\\EndFrameOffset"        , "Offset (in frames) from end frame that flags the animation has finished.\nThis can be used to flag the anim has ended early so that non-looping\nanimations do not freeze and look static when blending to the next animation.", 0 );

        List.PropEnumString  ( "Playback\\ChainAnim"             , "Animation to chain to (leave blank for no-chaining).", 0 );
        List.PropEnumInt     ( "Playback\\ChainFrame"            , "Frame to start at in chained animation (-1 = use current).", 0 );
        List.PropEnumFloat   ( "Playback\\ChainCyclesMin"        , "Mininum # of cycles to play before switching to chained animation.", 0 );
        List.PropEnumFloat   ( "Playback\\ChainCyclesMax"        , "Maximum # of cycles to play before switching to chained animation.", 0 );
        List.PropEnumBool    ( "Playback\\ChainCyclesInteger"    , "Specifies that number of cycles to play between min and max is always integer", 0 );

        List.PropEnumBool    ( "Playback\\BlendFrames"           , "Should interpolation between frames be enabled?", 0 );
        List.PropEnumBool    ( "Playback\\BlendLoop"             , "Should interpolation between end of anim and loop frame be enabled?", 0 );
        
        List.PropEnumHeader  ( "Motion"                          , "Motion properties", 0 );
        List.PropEnumBool    ( "Motion\\AccumHoriz"              , "Anim moves owner horizontally on XZ. Set to TRUE for runs/walks/turns etc. Set to FALSE for idles.", 0 );
        List.PropEnumBool    ( "Motion\\AccumVert"               , "Anim moves owner vertically on Y. ONLY set to TRUE for climbing/cinema big jumps etc.", 0 );
        List.PropEnumBool    ( "Motion\\AccumYaw"                , "Anim rotates the owners Yaw. ONLY set to TRUE for turn animations.", 0 );
        List.PropEnumBool    ( "Motion\\Gravity"                 , "Controls whether gravity should also be applied to owner. ONLY set to FALSE for climbing/cinema big jumps etc.", 0 ) ;
        List.PropEnumBool    ( "Motion\\Collision"               , "Controls whether world collision should happen to owner. ONLY set to FALSE for cinema special case anims etc.", 0 ) ;
        List.PropEnumAngle   ( "Motion\\YawHandle"               , "Handle yaw of animation", 0 );

        List.PropEnumButton( "DeleteAnim", 
                                "Press this button to delete this entry",
                                PROP_TYPE_MUST_ENUM );

        List.PropEnumInt   ( "ReIndex",
                                "Enter the index where you want this animation to be in the list of animations.",
                                PROP_TYPE_MUST_ENUM | PROP_TYPE_DONT_SAVE );

        List.PopPath(iHeader) ;
    }
}

//=========================================================================
void animation_desc::OnLoad ( text_in& TextIn )
{
    m_bDisableWarnings = TRUE;
    rsc_desc::OnLoad( TextIn );
    m_bDisableWarnings = FALSE;
}

//=========================================================================
static
s32 CompareAnimInfo( const animation_desc::anim_info* A, const animation_desc::anim_info* B )
{
    return x_stricmp( A->Name, B->Name );
}

//=========================================================================

xbool animation_desc::OnProperty( prop_query& I )
{
    if( rsc_desc::OnProperty( I ) )
        return TRUE;
    
    // Sort animations
    if( I.IsVar( "ResDesc\\SortAnimations" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Sort Animations" );
        }
        else
        {
            if( m_lAnimInfo.GetCount() )
            {
                x_qsort( 
                    &m_lAnimInfo[0], 
                    m_lAnimInfo.GetCount(), 
                    sizeof(anim_info), 
                    (compare_fn*)CompareAnimInfo );
            }
        }
        return TRUE;
    }
    
    // Add anim
    if( I.IsVar( "ResDesc\\Add Anim" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Add" );
        }
        else
        {
            anim_info& AnimInfo = m_lAnimInfo.Append();
            AnimInfo.SetupDefaults();
        }
        return TRUE;
    }
    
    // GenericAnim
    if( I.VarBool( "ResDesc\\GenericAnim", m_bGenericAnim ) )
        return TRUE;
    
    // BindPose
    if( I.VarString( "ResDesc\\BindPose", m_BindPose, 256 ) )
        return TRUE;

    // AnimCount
    if( I.IsVar( "ResDesc\\AnimCount" ))
    {
        if( I.IsRead() )
        {
            I.SetVarInt( m_lAnimInfo.GetCount() );
        }
        else
        {
            s32 NewCount = I.GetVarInt();

            if( NewCount <= m_lAnimInfo.GetCount() )
            {
                for( s32 i=NewCount; i<m_lAnimInfo.GetCount(); i++ )
                {
                    if( x_strlen( m_lAnimInfo[i].FileName ) > 0 )
                        x_throw( "You must delete the animations by pressing the delete button" );
                }                

                m_lAnimInfo.Delete( NewCount, m_lAnimInfo.GetCount() - NewCount );
            }
            else
            {
                m_lAnimInfo.SetCount( NewCount );
            }
        }
        return TRUE;
    }

    // Anim header
    if( I.IsVar( "ResDesc\\AnimList[]" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarString( m_lAnimInfo[I.GetIndex(0)].Name, 32 );
        }
        else 
        {
            ASSERT( 0 );
        }
        return TRUE;
    }

    // Individual animations
    if( ( m_lAnimInfo.GetCount()  ) && ( I.IsSimilarPath("ResDesc\\AnimList[" ) ) )
    {
        // It's okay to push a path without popping it in the read/write,
        // since the path is reset for each property.
        I.PushPath("ResDesc\\AnimList[]\\") ; 

        // Lookup animation
        s32         Index    = I.GetIndex(0);
        anim_info&  AnimInfo = m_lAnimInfo[Index];

        // DeleteAnim
        if( I.IsVar( "DeleteAnim" ) )
        {
            if( I.IsRead() )
            {
                I.SetVarButton( "Delete" );
            }
            else
            {
                if( IDYES == MessageBox( NULL, "Are you sure you want to delete this entry?", "Warning",MB_ICONWARNING|MB_YESNO ) )
                {
                    m_lAnimInfo.Delete( I.GetIndex(0) );
                }
            }
            return TRUE;
        }

        // ReIndex
        if( I.IsVar( "ReIndex" ) )
        {
            if( I.IsRead() )
            {
                I.SetVarInt( Index );
            }
            else
            {
                s32 NewIndex = iMin( m_lAnimInfo.GetCount()-1, iMax( 0, I.GetVarInt() ) );

                anim_info AnimInfoTemp = m_lAnimInfo[Index];
                m_lAnimInfo.Delete( Index );
                m_lAnimInfo.Insert( NewIndex ) = AnimInfoTemp;
            }
            return TRUE;
        }

        // FileName
        if( I.IsVar( "FileName" ))
        {
            if( I.IsRead() )
            {
                I.SetVarFileName( AnimInfo.FileName, sizeof(AnimInfo.FileName) );
            }
            else
            {
                for( s32 i=0; i<m_lAnimInfo.GetCount(); i++ )
                {
                    if( i != I.GetIndex(0) && x_stricmp( I.GetVarFileName(), m_lAnimInfo[i].FileName ) == 0 )
                    {
                        if( m_bDisableWarnings == FALSE )
                        {
                            int ID = MessageBox( NULL, xfs("You already have this animation in the list.\n[%s]\nCheck #%d\nAre you sure you want to add this animation again?", 
                                I.GetVarFileName(), i), "Warning", MB_YESNO | MB_ICONWARNING );

                            if( ID == IDNO)
                                return TRUE;
                        }
                    }
                }

                x_strcpy( AnimInfo.FileName, I.GetVarFileName() );

                // If we don't have an animation name yet lets set the matx file as the default name
                if( AnimInfo.Name[0] == 0 )
                {
                    x_splitpath( I.GetVarFileName(), NULL, NULL, AnimInfo.Name, NULL );
                }
            }
            return TRUE;
        }

        // Name
        if( I.IsVar( "Name" ))
        {
            if( I.IsRead() )
                I.SetVarString( AnimInfo.Name, sizeof(AnimInfo.Name) );
            else
                x_strncpy( AnimInfo.Name, I.GetVarString(), sizeof(AnimInfo.Name) );

            return TRUE;
        }

        // SB:
        // NOTE: You'll see 2 searches for some properties - they are there for backwards
        //       compatibility because I updated the property names as per Aaron's request.

        // bLoop
        if(         ( I.VarBool( "bLoop", AnimInfo.bLoop ) )
                ||  ( I.VarBool( "Loop",  AnimInfo.bLoop ) ) )
            return TRUE;

        // FPS
        if( I.VarInt( "FPS", AnimInfo.FPS, 0, 60 ) )
            return TRUE;

        // Downsample
        if( I.VarFloat( "Downsample", AnimInfo.Downsample, 0, 1.0f ) )
            return TRUE;

        // BlendTime
        if( I.VarFloat( "Playback\\BlendTime", AnimInfo.BlendTime, -1.0f, 10.0f ) )
            return TRUE;

        // SelectionWeight
        if(         ( I.VarFloat( "Weight", AnimInfo.Weight, 0.0f, 1000.0f ) )
                ||  ( I.VarFloat( "Playback\\SelectionWeight", AnimInfo.Weight, 0.0f, 1000.0f ) ) )
            return TRUE;
        
        // LoopFrame
        if( I.VarInt( "Playback\\LoopFrame", AnimInfo.iLoopFrame, 0, 10000 ) )
            return TRUE;
        
        // EndFrameOffset
        if( I.VarInt( "Playback\\EndFrameOffset", AnimInfo.EndFrameOffset, 0, 10000 ) )
            return TRUE;


        // ChainAnim
        if( I.IsVar( "Playback\\ChainAnim" ))
        {
            if( I.IsRead() )
                I.SetVarString( AnimInfo.ChainAnim, sizeof(AnimInfo.ChainAnim) );
            else
                x_strncpy( AnimInfo.ChainAnim, I.GetVarString(), sizeof(AnimInfo.ChainAnim) );

            return TRUE;
        }

        // ChainFrame
        if( I.VarInt( "Playback\\ChainFrame", AnimInfo.iChainFrame, -1, 10000 ) )
            return TRUE;

        // ChainCyclesMin
        if( I.VarFloat( "Playback\\ChainCyclesMin", AnimInfo.ChainCyclesMin, 0.0f, 100.0f ) )
            return TRUE;

        // ChainCyclesMax
        if( I.VarFloat( "Playback\\ChainCyclesMax", AnimInfo.ChainCyclesMax, 0.0f, 100.0f ) )
            return TRUE;

        // ChainCyclesInteger
        if( I.VarBool( "Playback\\ChainCyclesInteger", AnimInfo.bChainCyclesInteger ) )
            return TRUE;

        // BlendFrames
        if( I.VarBool( "Playback\\BlendFrames", AnimInfo.bBlendFrames ) )
            return TRUE;

        // BlendLoop
        if( I.VarBool( "Playback\\BlendLoop", AnimInfo.bBlendLoop ) )
            return TRUE;

        // bAccumHorizMotion
        if(         ( I.VarBool( "bAccumHorizMotion",   AnimInfo.bAccumHorizMotion ) )
                ||  ( I.VarBool( "Motion\\AccumHoriz",  AnimInfo.bAccumHorizMotion ) ) )
            return TRUE;
        
        // bAccumVertMotion
        if(         ( I.VarBool( "bAccumVertMotion",  AnimInfo.bAccumVertMotion ) )
                ||  ( I.VarBool( "Motion\\AccumVert", AnimInfo.bAccumVertMotion ) ) )
            return TRUE;
        
        // bAccumYawMotion
        if(         ( I.VarBool( "bAccumYawMotion",     AnimInfo.bAccumYawMotion ) )
                ||  ( I.VarBool( "Motion\\AccumYaw",    AnimInfo.bAccumYawMotion ) ) )
            return TRUE;

        // bGravity
        if(         ( I.VarBool( "bGravity",        AnimInfo.bGravity ) )
                ||  ( I.VarBool( "Motion\\Gravity", AnimInfo.bGravity ) ) )
            return TRUE;
        
        // bWorldCollision
        if(         ( I.VarBool( "bWorldCollision",     AnimInfo.bWorldCollision ) )
                ||  ( I.VarBool( "Motion\\Collision",   AnimInfo.bWorldCollision ) ) )
            return TRUE;

        // Angle
        if(         ( I.VarAngle( "Angle",              AnimInfo.Angle, -360.0f, 360.0f ) )
                ||  ( I.VarAngle( "Motion\\YawHandle",  AnimInfo.Angle, -360.0f, 360.0f ) ) )
            return TRUE;

        ASSERTS(0, "TELL STEVE BROUMLEY - ANIM PROPERTIES ARE CORRUPT!!!");
    }

    return FALSE;
}

//=========================================================================

void animation_desc::OnCheckIntegrity( void )
{
    rsc_desc::OnCheckIntegrity();

    if( m_BindPose[0] == 0 )
    {
        x_throw( "Animation package doesn't have a bindpose." );
    }


    for( s32 i=0; i<m_lAnimInfo.GetCount(); i++ )
    {
        if( m_lAnimInfo[i].FileName[0] == 0 )
        {
            x_throw( xfs( "Animation #%d doesn't have a matx file associated with it.", i) );
        }
    }
}

//=========================================================================

void animation_desc::OnGetCompilerDependencies( xarray<xstring>& List ) 
{
    OnCheckIntegrity();

    List.Append() = m_BindPose;

    for( s32 i=0; i<m_lAnimInfo.GetCount(); i++ )
    {
        List.Append() = m_lAnimInfo[i].FileName;
    }
}

//=========================================================================

void animation_desc::OnGetCompilerRules( xstring& CompilerRules ) 
{
    CompilerRules.Clear();
    
    CompilerRules += "AnimCompiler.exe ";
    
    // Verbose
    if( IsVerbose() )
        CompilerRules += "-LOG ";
    
    CompilerRules += "-BINDPOSE \"";
    CompilerRules += m_BindPose;
    CompilerRules += "\" ";

    if( m_bGenericAnim )
    {
        CompilerRules += "-KEEPBIND 1 ";
    }
    else
    {
        CompilerRules += "-KEEPBIND 0 ";
    }
    
    for( s32 i=0; i<m_lAnimInfo.GetCount(); i++ )
    {
        anim_info& Info = m_lAnimInfo[i];
        char Data[256];

        CompilerRules += ( "-NAME " );
        sprintf( Data, "\"%s\" ", Info.Name );
        CompilerRules += Data;

        CompilerRules += ( "-LOOP " );
        sprintf( Data, "%d ", Info.bLoop );
        CompilerRules += Data;

        CompilerRules += ( "-LOOP_FRAME " );
        sprintf( Data, "%d ", Info.iLoopFrame );
        CompilerRules += Data;

        CompilerRules += ( "-END_FRAME_OFFSET " );
        sprintf( Data, "%d ", Info.EndFrameOffset );
        CompilerRules += Data;

        CompilerRules += ( "-FPS " );
        sprintf( Data, "%d ", Info.FPS );
        CompilerRules += Data;
        
        CompilerRules += ( "-DOWNSAMPLE " );
        sprintf( Data, "%f ", Info.Downsample );
        CompilerRules += Data;
        
        CompilerRules += ( "-ACCUM_HORIZ_MOTION " );
        sprintf( Data, "%d ", Info.bAccumHorizMotion );
        CompilerRules += Data;

        CompilerRules += ( "-ACCUM_VERT_MOTION " );
        sprintf( Data, "%d ", Info.bAccumVertMotion );
        CompilerRules += Data;

        CompilerRules += ( "-ACCUM_YAW_MOTION " );
        sprintf( Data, "%d ", Info.bAccumYawMotion );
        CompilerRules += Data;

        CompilerRules += ( "-GRAVITY " );
        sprintf( Data, "%d ", Info.bGravity );
        CompilerRules += Data;

        CompilerRules += ( "-WORLD_COLLISION " );
        sprintf( Data, "%d ", Info.bWorldCollision );
        CompilerRules += Data;

        CompilerRules += ( "-WEIGHT " );
        sprintf( Data, "%f ", Info.Weight );
        CompilerRules += Data;

        CompilerRules += ( "-BLEND_TIME " );
        sprintf( Data, "%f ", Info.BlendTime );
        CompilerRules += Data;

        CompilerRules += ( "-HANDLE " );
        sprintf( Data, "%f ", RAD_TO_DEG(Info.Angle) );
        CompilerRules += Data;

        CompilerRules += ( "-CHAIN_ANIM " );
        sprintf( Data, "\"%s\" ", Info.ChainAnim );
        CompilerRules += Data;

        CompilerRules += ( "-CHAIN_FRAME " );
        sprintf( Data, "%d ", Info.iChainFrame );
        CompilerRules += Data;

        CompilerRules += ( "-CHAIN_CYCLES_MIN " );
        sprintf( Data, "%f ", Info.ChainCyclesMin );
        CompilerRules += Data;

        CompilerRules += ( "-CHAIN_CYCLES_MAX " );
        sprintf( Data, "%f ", Info.ChainCyclesMax );
        CompilerRules += Data;

        CompilerRules += ( "-CHAIN_CYCLES_INTEGER " );
        sprintf( Data, "%d ", Info.bChainCyclesInteger );
        CompilerRules += Data;
        
        CompilerRules += ( "-BLEND_FRAMES " );
        sprintf( Data, "%d ", Info.bBlendFrames );
        CompilerRules += Data;
        
        CompilerRules += ( "-BLEND_LOOP " );
        sprintf( Data, "%d ", Info.bBlendLoop );
        CompilerRules += Data;

        // NOTE - This must be the last rule since it triggers adding this anim
        //        to the compilers list.
        CompilerRules += ( "-ANIM \"" );
        CompilerRules += ( Info.FileName );
        CompilerRules += ( "\" " );
    }
}

//=========================================================================

void animation_desc::OnGetFinalDependencies( xarray<xstring>& List, platform Platform, const char* pDirectory )
{
    anim_group AnimGroup;

    //
    // Load the anim group
    //
    if( AnimGroup.Load( xfs( "%s\\%s", pDirectory, GetName() ) ) == FALSE )
    {
        x_throw( xfs("Unable to open [%s] So I can't check for dependencies", xfs( "%s\\%s", pDirectory, GetName())) );
    }

    //
    // Loop throw all the event and check whether they have any external dependency
    //
    for( s32 i=0; i<AnimGroup.GetNAnims(); i++ )
    {
        const ::anim_info&  Info = AnimGroup.GetAnimInfo( i );

        for( s32 j=0; j<Info.GetNEvents(); j++ )
        {
            const anim_event&       AnimEvent = Info.GetEvent( j );
            const char*             EventType = AnimEvent.GetType();

            if(x_strcmp(EventType, "Particle") == 0)
            {
                List.Append() = xfs( "%s.fxo", AnimEvent.GetString( anim_event::STRING_IDX_PARTICLE_TYPE ));
            }
            else if( x_strcmp(EventType, "Rigid") == 0 )
            {
                List.Append() = xfs( "%s.rigidgeom", AnimEvent.GetString( anim_event::STRING_IDX_DEBRIS_TYPE ));
            }
            else if( x_strcmp(EventType, "Debris") == 0 )
            {
                List.Append() = xfs( "%s.rigidgeom", AnimEvent.GetString( anim_event::STRING_IDX_DEBRIS_TYPE ));
            }
        }
    }
}

//=========================================================================

locomotion_ed::locomotion_ed( void )
{
    m_pDesc = NULL;
}

//=========================================================================

locomotion_ed::~locomotion_ed( void )
{
    EndEdit();
}
    
//=========================================================================

void locomotion_ed::OnEnumProp( prop_enum& List )
{
    if( m_pDesc )
        m_pDesc->OnEnumProp( List );
}

//=========================================================================

xbool locomotion_ed::OnProperty( prop_query& I )
{
    if( m_pDesc )
        return m_pDesc->OnProperty( I );    

    return FALSE;
}

//=========================================================================

void locomotion_ed::Save( void )
{
    if( m_pDesc == NULL )
        x_throw ("There is not package open or created" );

    g_RescDescMGR.Save( *m_pDesc );
}

//=========================================================================
xbool locomotion_ed::NeedSave( void )
{
    return m_pDesc != NULL && m_pDesc->IsChanged();
}

//=========================================================================

void locomotion_ed::EndEdit( void )
{
    if( m_pDesc )
        m_pDesc->SetBeingEdited( FALSE );
    m_pDesc = NULL;
}

//=========================================================================

void locomotion_ed::New( void )
{
    EndEdit();
    BeginEdit( animation_desc::GetSafeType( g_RescDescMGR.CreateRscDesc( ".anim" ) ) );
}

//=========================================================================

void locomotion_ed::Load( const char* pFileName )
{
    EndEdit();
    BeginEdit( animation_desc::GetSafeType( g_RescDescMGR.Load( pFileName ) ) );
}

//=========================================================================

void locomotion_ed::BeginEdit( animation_desc& AnimDesc )
{
    AnimDesc.SetBeingEdited( TRUE );
    m_pDesc = &AnimDesc;
}

