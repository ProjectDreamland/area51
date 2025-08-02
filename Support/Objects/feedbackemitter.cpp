//==============================================================================
//
//  feedbackemitter.cpp
//
//  Copyright (c) 2004 Inevitable Entertainment Inc. All rights reserved.
//
//  define HERE
//
//==============================================================================

//==========================================================================
// INCLUDE
//==========================================================================
#include "feedbackemitter.hpp"
#include "Parsing\TextIn.hpp"
#include "Entropy.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "Render\Editor\editor_icons.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"

#include "Objects\Player.hpp"
#include "characters\character.hpp"
#include "Objects\propsurface.hpp"

//==========================================================================
// DEFINES
//==========================================================================

//==========================================================================
// GLOBAL
//==========================================================================

//==========================================================================
// FUNTIONS
//==========================================================================
//=========================================================================
// Spatial type table
typedef enum_pair<feedback_emitter::feedback_emitter_spatial_types> spatial_type_enum_pair;
static spatial_type_enum_pair s_SpatialTypeList[] = 
{
        spatial_type_enum_pair("Axis Cube",                feedback_emitter::SPATIAL_TYPE_AXIS_CUBE),
        spatial_type_enum_pair("Spherical",                feedback_emitter::SPATIAL_TYPE_SPHERICAL),
        spatial_type_enum_pair( k_EnumEndStringConst,      feedback_emitter::SPATIAL_TYPES_INVALID) //**MUST BE LAST**//
};
enum_table<feedback_emitter::feedback_emitter_spatial_types>  feedback_emitter::m_SpatialTypeList( s_SpatialTypeList );              

static struct feedback_emitter_desc : public object_desc
{
    feedback_emitter_desc( void ) : object_desc( 
        object::TYPE_DAMAGE_FIELD, 
        "FeedBack Emitter",
        "SCRIPT",

        object::ATTR_NEEDS_LOGIC_TIME    |
        object::ATTR_SPACIAL_ENTRY		 |
        object::ATTR_COLLIDABLE          |
        object::ATTR_BLOCKS_ALL_ACTORS   |
        object::ATTR_COLLISION_PERMEABLE,
        FLAGS_GENERIC_EDITOR_CREATE | 
        FLAGS_IS_DYNAMIC ) 
    {
        m_bRenderSpatial = TRUE;
    }

    //---------------------------------------------------------------------

    virtual object* Create( void ) { return new feedback_emitter; }

    //---------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32     OnEditorRender( object& Object ) const 
    { 
        if( Object.IsKindOf( feedback_emitter::GetRTTI() ) )
        {
            feedback_emitter& Field = feedback_emitter::GetSafeType( Object );   

            if ( m_bRenderSpatial || ( Field.GetAttrBits() & object::ATTR_EDITOR_SELECTED ))
            {
                Field.OnRenderSpatial();
            }

            if (Field.m_DrawActivationIcon)
            {
                EditorIcon_Draw( EDITOR_ICON_LOOP, Field.GetL2W(), FALSE, XCOLOR_PURPLE );
                EditorIcon_Draw( EDITOR_ICON_DAMAGE, Field.GetL2W(), FALSE, XCOLOR_PURPLE );
                Field.m_DrawActivationIcon = FALSE;
                return -1;
            }
        }
        return EDITOR_ICON_DAMAGE; 
    }

#endif // X_EDITOR


    //-------------------------------------------------------------------------

    virtual void OnEnumProp( prop_enum& List )
    {
        object_desc::OnEnumProp( List );
        List.PropEnumBool   ( "ObjectDesc\\Show Spatial",     "Show spacial fields always.", 0 );
    }

    //-------------------------------------------------------------------------

    virtual xbool OnProperty( prop_query&  I )
    {
        if( object_desc::OnProperty( I ) )
            return TRUE;

        if( I.VarBool( "ObjectDesc\\Show Spatial", m_bRenderSpatial ) )
            return TRUE;

        return FALSE;
    }

    xbool m_bRenderSpatial;

} s_feedback_emitter_Desc;

//=========================================================================

const object_desc& feedback_emitter::GetTypeDesc( void ) const
{
    return s_feedback_emitter_Desc;
}

//=========================================================================

const object_desc& feedback_emitter::GetObjectType( void )
{
    return s_feedback_emitter_Desc;
}

//=========================================================================
feedback_emitter::feedback_emitter(void)
{
    //spatial dimensions
    for ( s32 i = 0; i < 3; i++)
        m_Dimensions[i] = 400.0f;      

    m_bActive = TRUE;
    m_SpatialType = SPATIAL_TYPE_SPHERICAL;
    m_TimeDelay = 0.25f;
    m_TimeSinceLastDamage = 0.0f;
    m_DrawActivationIcon = FALSE;
    m_RumbleStrangth = 0.5f;
}

//=========================================================================
feedback_emitter::~feedback_emitter(void)
{
}

//=========================================================================
void feedback_emitter::OnInit( void )
{
}

//=========================================================================
void feedback_emitter::OnAdvanceLogic( f32 DeltaTime )
{
    if (m_TimeSinceLastDamage < 0.0f)
        m_TimeSinceLastDamage = 0.0f;
    m_TimeSinceLastDamage += DeltaTime;

    object* pMover = m_FeedBackAnchorAffecter.GetObjectPtr();
    if (pMover)
    {
        //update position to match mover
        vector3 Pos = pMover->GetBBox().GetCenter();
        Pos.GetY() += m_YOffset;

        //must I move?
        if (Pos != GetPosition())
        {
            OnMove(Pos);
            m_bDoingFeedBack = TRUE;
        }
    } 


    if (!m_bActive)
        return;

    if (!m_bDoingFeedBack)
        return;

    //if (m_TimeSinceLastDamage > m_TimeDelay)
    {
        m_TimeSinceLastDamage = 0.0f;
        m_DrawActivationIcon = TRUE;

        xbool bFeedBackDone = FALSE;
        if ( QueryPlayerInVolume() )
        {
            bFeedBackDone = TRUE;
        }
        m_bDoingFeedBack = bFeedBackDone;
    }
}

//=========================================================================
void feedback_emitter::OnEnumProp( prop_enum& List )
{
    object::OnEnumProp( List );

    List.PropEnumHeader ( "FeedBack Emitter", "Information specific to the damage field", 0 );

    List.PropEnumBool   ( "FeedBack Emitter\\Start Active", "Is this field initially active and causing damage?", 0 );

    List.PropEnumFloat  ( "FeedBack Emitter\\Ruumble Strangth", "Strangth of the rumble",0 );

    switch ( m_SpatialType )
    {
    case SPATIAL_TYPE_AXIS_CUBE:       
        List.PropEnumFloat   ( "FeedBack Emitter\\Cube Width",    "The width of the cube.", 0 );
        List.PropEnumFloat   ( "FeedBack Emitter\\Cube Height",   "The height of the cube.", 0 );
        List.PropEnumFloat   ( "FeedBack Emitter\\Cube Length",   "The length of the cube.", 0 );
        break;

    case SPATIAL_TYPE_SPHERICAL:         
        List.PropEnumFloat   ( "FeedBack Emitter\\Sphere Radius",   "The radius of the sphere.", 0 );
        break;

    default:
        ASSERT(0);
        break;
    }

    List.PropEnumHeader     ( "MoverInfo", "This feedback emitter can optionally be attached to another guid. It will then update its position based on the objects position.", 0 );

    s32 iDamageHeader = List.PushPath( "MoverInfo\\" );        
    m_FeedBackAnchorAffecter.OnEnumProp(List,"Mover");
    List.PopPath( iDamageHeader );

    List.PropEnumFloat       ( "MoverInfo\\Y Offset",   "How much to adjust the Y Position of the object in relation to the mover object.", 0 );
}

//=============================================================================

xbool feedback_emitter::OnProperty( prop_query& I )
{
    if( object::OnProperty( I ) )
        return TRUE;

    if ( I.VarBool( "FeedBack Emitter\\Start Active", m_bActive))
        return TRUE;

    if( I.VarFloat ("FeedBack Emitter\\Ruumble Strangth", m_RumbleStrangth) )
        return TRUE;

    if ( I.VarFloat   ( "FeedBack Emitter\\Cube Width",             m_Dimensions[0] ) )
    {
        SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSLATION);
        UpdateTransform();              //    Being called in OnSpacialUpdate
        OnSpacialUpdate();
        return TRUE;
    }

    if ( I.VarFloat   ( "FeedBack Emitter\\Cube Height",            m_Dimensions[1] ) )
    {
        SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSLATION);
        UpdateTransform();              //    Being called in OnSpacialUpdate
        OnSpacialUpdate();
        return TRUE;
    }

    if ( I.VarFloat   ( "FeedBack Emitter\\Cube Length",            m_Dimensions[2] ) )
    {
        SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSLATION);
        UpdateTransform();              //    Being called in OnSpacialUpdate
        OnSpacialUpdate();
        return TRUE;
    }

    if ( I.VarFloat   ( "FeedBack Emitter\\Sphere Radius",          m_Dimensions[0] ) )
    {
        SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSLATION);
        UpdateTransform();              //    Being called in OnSpacialUpdate
        OnSpacialUpdate();
        return TRUE;
    }

    s32 iFeedbackHeader = I.PushPath( "MoverInfo\\" );        
    if ( m_FeedBackAnchorAffecter.OnProperty( I, "Mover" ))
        return TRUE;
    if ( I.VarFloat   ( "Y Offset",     m_YOffset ) )
        return TRUE;
    I.PopPath( iFeedbackHeader );

    return FALSE;
}

//=========================================================================

bbox feedback_emitter::GetLocalBBox( void ) const 
{
    bbox BBox;

    switch ( m_SpatialType )
    {
    case SPATIAL_TYPE_AXIS_CUBE:       
        {  
            f32 HalfWidth  = m_Dimensions[0]/2;
            f32 HalfHeight = m_Dimensions[1]/2;
            f32 HalfLength = m_Dimensions[2]/2;
            vector3 Min(    -HalfWidth,    -HalfHeight, -HalfLength );
            vector3 Max(     HalfWidth,     HalfHeight,  HalfLength );
            BBox.Set( Min, Max );
        }
        break;

    case SPATIAL_TYPE_SPHERICAL:    
        {
            vector3 Min(    -m_Dimensions[0],    -m_Dimensions[0], -m_Dimensions[0] );
            vector3 Max(     m_Dimensions[0],     m_Dimensions[0],  m_Dimensions[0] );
            BBox.Set( Min, Max );
        }
        break;

    default:
        ASSERT(0);
        break;
    }

    return BBox; 
}
//=========================================================================

#if !defined( CONFIG_RETAIL )

void feedback_emitter::OnRenderSpatial( void )
{
    //spatial trigger info
    xcolor DrawColor = xcolor(255, 128, 64);
    if( GetAttrBits() & ATTR_EDITOR_SELECTED )
    {
        DrawColor = xcolor(180,100,32);
    }

    switch ( m_SpatialType )
    {
    case SPATIAL_TYPE_AXIS_CUBE:       
        // Renders a volume given a BBox
        draw_Volume ( GetBBox(), xcolor(DrawColor.R, DrawColor.G, DrawColor.B, 90));
        draw_BBox   ( GetBBox(), DrawColor);  
        break;
    case SPATIAL_TYPE_SPHERICAL:         
        draw_Sphere( object::GetPosition(), m_Dimensions[0], DrawColor );
        break;
    }
}

#endif // !defined( CONFIG_RETAIL )
//=========================================================================

void feedback_emitter::OnColCheck	( void )
{
    if (!m_bActive)
        return;

    switch ( m_SpatialType )
    {
    case SPATIAL_TYPE_AXIS_CUBE:       
        {
            g_CollisionMgr.StartApply( GetGuid() );
            g_CollisionMgr.ApplyAABBox( GetBBox() );
            g_CollisionMgr.EndApply();
        }
        break;

    case SPATIAL_TYPE_SPHERICAL:         
        {
            g_CollisionMgr.StartApply( GetGuid() );
            g_CollisionMgr.ApplySphere( GetPosition(), m_Dimensions[0] );
            g_CollisionMgr.EndApply();
        }
        break;

    default:
        ASSERT(0);
        break;
    }

    object::OnColCheck();
}

//=============================================================================

void feedback_emitter::OnColNotify( object& Object )
{
    if (!m_bActive )
        return;

    m_TimeSinceLastDamage = m_TimeDelay + 1.0f;
    m_bDoingFeedBack = TRUE;

    object::OnColNotify(Object);
}

//=============================================================================

void feedback_emitter::OnActivate ( xbool Flag )
{
    m_bActive = Flag;    

    //make it instantly fire
    m_TimeSinceLastDamage += m_TimeDelay + 1.0f;

    if ( Flag == TRUE )
        LogicCheckOnActivate();
}
//=============================================================================

void feedback_emitter::LogicCheckOnActivate ( void )
{      
    if (!m_bActive)
        return;

    if ( QueryPlayerInVolume() )
        m_bDoingFeedBack = TRUE;

    if (m_bDoingFeedBack)
    {
        m_TimeSinceLastDamage = m_TimeDelay + 1.0f;
    }
}
//=============================================================================
// SPACIAL QUERIES
//=============================================================================

xbool feedback_emitter::QueryPlayerInVolume ( void )
{
    //Query for the player...  
    slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_PLAYER );

    xbool bVal = FALSE;    
    while( SlotID != SLOT_NULL )
    {
        object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );

        if( pObject != NULL )
            bVal |= QueryObjectInVolume(pObject);

        SlotID = g_ObjMgr.GetNext( SlotID );
    }

    return bVal;
}
f32 g_RUBMLE_TIME = 0.0f;
f32 g_RUMBLE_STR = 0.5f;
//=============================================================================
xbool feedback_emitter::QueryObjectInVolume( object* pObject )
{
    f32 Scalar = 1.0f;
    xbool bRval = FALSE;
    switch ( m_SpatialType )
    {
    case SPATIAL_TYPE_AXIS_CUBE:       
        {  
            bbox ObjectBBox = pObject->GetBBox();
            bbox FeedbackBBox = GetBBox();

            if( FeedbackBBox.Intersect(ObjectBBox) )
            {
                bRval = TRUE;
                vector3 Delta = ObjectBBox.GetCenter() - FeedbackBBox.GetCenter();
                f32 Dist = Delta.Length();
                Scalar = x_parametric( Dist, 0, MAX(m_Dimensions[0],MAX(m_Dimensions[1],m_Dimensions[2])), TRUE );
            }
        }
        break;

    case SPATIAL_TYPE_SPHERICAL:    
        { 
            bbox ObjectBBox = pObject->GetBBox();
            bbox FeedbackBBox = GetBBox();

            if( ObjectBBox.Intersect(GetPosition(), m_Dimensions[0]) )
            {
                bRval = TRUE;
                vector3 Delta = ObjectBBox.GetCenter() - FeedbackBBox.GetCenter();
                f32 Dist = Delta.Length();
                Scalar = x_parametric( Dist, 0, m_Dimensions[0], TRUE );
            }
        }
        break;

    default:
        ASSERT(0);
        break;
    }

    if (bRval)
    {
        // feedback
        player* pPlayerObj = SMP_UTIL_GetActivePlayer();
        if ( pPlayerObj )
        {
            pPlayerObj->DoFeedback(m_TimeDelay, m_RumbleStrangth/Scalar );
        }
    }

    return bRval;
}
