#include "DamageField.hpp"
#include "Parsing\TextIn.hpp"
#include "Entropy.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "Render\Editor\editor_icons.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"

#include "Objects\Player.hpp"
#include "characters\character.hpp"
#include "Objects\propsurface.hpp"

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

//=========================================================================
// Spatial type table
typedef enum_pair<damage_field::damage_field_spatial_types> spatial_type_enum_pair;
static spatial_type_enum_pair s_SpatialTypeList[] = 
{
    spatial_type_enum_pair("Axis Cube",                damage_field::SPATIAL_TYPE_AXIS_CUBE),
    spatial_type_enum_pair("Spherical",                damage_field::SPATIAL_TYPE_SPHERICAL),
    spatial_type_enum_pair( k_EnumEndStringConst,      damage_field::SPATIAL_TYPES_INVALID) //**MUST BE LAST**//
};
enum_table<damage_field::damage_field_spatial_types>  damage_field::m_SpatialTypeList( s_SpatialTypeList );              

/*
typedef enum_pair<pain::type> pain_type_enum_pair;
static pain_type_enum_pair s_PainTypeList[] = 
{
    pain_type_enum_pair("Generic",                  pain::TYPE_GENERIC),
    pain_type_enum_pair("Explosive",                pain::TYPE_PROJECTILE_GRENADE),
    pain_type_enum_pair("Shock",                    pain::TYPE_ELECTROCUTION),
    pain_type_enum_pair( k_EnumEndStringConst,      pain::PAIN_UNDEFINED) 
};
enum_table<pain::type>  damage_field::g_GenericPainTypeList( s_PainTypeList );              
*/
//=========================================================================

static struct damage_field_desc : public object_desc
{
    damage_field_desc( void ) : object_desc( 
            object::TYPE_DAMAGE_FIELD, 
            "Damage Field",
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

    virtual object* Create( void ) { return new damage_field; }

    //---------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32     OnEditorRender( object& Object ) const 
    { 
        if( Object.IsKindOf( damage_field::GetRTTI() ) )
        {
            damage_field& Field = damage_field::GetSafeType( Object );   

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

} s_DamageField_Desc;

//=========================================================================

const object_desc& damage_field::GetTypeDesc( void ) const
{
    return s_DamageField_Desc;
}

//=========================================================================

const object_desc& damage_field::GetObjectType( void )
{
    return s_DamageField_Desc;
}

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

damage_field::damage_field( void ) :
m_DrawActivationIcon(FALSE),
m_SpatialType(SPATIAL_TYPE_AXIS_CUBE),
m_SpatialTargets( DF_TARGET_PLAYER | DF_TARGET_CHARACTERS ),
m_bActive(TRUE),
m_bDoingDamage(FALSE),
m_TimeDelay(0.25f),
m_TimeSinceLastDamage(0.0f),
m_YOffset(0.0f)
{
   //spatial dimensions
    for ( s32 i = 0; i < 3; i++)
          m_Dimensions[i] = 400.0f;      

    m_GenericPainType = TYPE_GENERIC_1;
    m_bDoAttenuate = FALSE;
}

//=========================================================================

void damage_field::OnEnumProp( prop_enum& List )
{
    object::OnEnumProp( List );

    List.PropEnumHeader  ( "DamageField", "Information specific to the damage field", 0 );
    
    List.PropEnumBool    ( "DamageField\\Start Active", "Is this field initially active and causing damage?", 0 );
    List.PropEnumBool    ( "DamageField\\Do Attenuation", "Should the damage fall off with distance?", 0 );
    List.PropEnumEnum    ( "DamageField\\Type", 
        m_SpatialTypeList.BuildString(), 
        "Types of spatial fields.  AxisCube is a box or Sphere is Radius.",  PROP_TYPE_MUST_ENUM );
    
    switch ( m_SpatialType )
    {
    case SPATIAL_TYPE_AXIS_CUBE:       
        List.PropEnumFloat   ( "DamageField\\Cube Width",    "The width of the cube.", 0 );
        List.PropEnumFloat   ( "DamageField\\Cube Height",   "The height of the cube.", 0 );
        List.PropEnumFloat   ( "DamageField\\Cube Length",   "The length of the cube.", 0 );
        break;

    case SPATIAL_TYPE_SPHERICAL:         
        List.PropEnumFloat   ( "DamageField\\Sphere Radius",   "The radius of the sphere.", 0 );
        break;

    default:
        ASSERT(0);
        break;
    }

    List.PropEnumBool        ( "DamageField\\Affect Player",     "Does this field affect the player?", 0 );
    List.PropEnumBool        ( "DamageField\\Affect AI",         "Does this field affect AI?", 0 );
    List.PropEnumBool        ( "DamageField\\Affect Props",      "Does this field affect prop surfaces?", 0 );

    List.PropEnumEnum        ( "DamageField\\PainType",    g_GenericPainTypeList.BuildString(), "What type of pain this trigger causes.", PROP_TYPE_MUST_ENUM );

    List.PropEnumFloat       ( "DamageField\\TimeDelay",   "How much time to delay between each firing.", 0 );

    List.PropEnumBool        ( "DamageField\\TriggerDamage", "Trigger the damage field to do damage", PROP_TYPE_DONT_SHOW | PROP_TYPE_READ_ONLY | PROP_TYPE_DONT_SAVE | PROP_TYPE_EXPOSE );

    List.PropEnumHeader     ( "MoverInfo", "This damage field can optionally be attached to another guid. It will then update its position based on the objects position.", 0 );

    s32 iDamageHeader = List.PushPath( "MoverInfo\\" );        
    m_DamageAnchorAffecter.OnEnumProp(List,"Mover");
    List.PopPath( iDamageHeader );

    List.PropEnumFloat       ( "MoverInfo\\Y Offset",   "How much to adjust the Y Position of the object in relation to the mover object.", 0 );
}

//=============================================================================

xbool damage_field::OnProperty( prop_query& I )
{
    if( object::OnProperty( I ) )
        return TRUE;

    s32 iDamageHeader = I.PushPath( "MoverInfo\\" );        
    if ( m_DamageAnchorAffecter.OnProperty( I, "Mover" ))
        return TRUE;
    if ( I.VarFloat   ( "Y Offset",     m_YOffset ) )
        return TRUE;
    I.PopPath( iDamageHeader );

    if ( I.VarBool( "DamageField\\Start Active", m_bActive))
        return TRUE;

    if ( I.VarBool( "DamageField\\Do Attenuation", m_bDoAttenuate))
        return TRUE;

    if ( I.IsVar( "DamageField\\PainType") )
    {
        if( I.IsRead() )
        {
            if ( g_GenericPainTypeList.DoesValueExist( m_GenericPainType ) )
            {
                I.SetVarEnum( g_GenericPainTypeList.GetString( m_GenericPainType ) );
            }
            else
            {
                I.SetVarEnum( "Generic_1" );
            } 
        }
        else
        {
            generic_pain_type Type;

            if( g_GenericPainTypeList.GetValue( I.GetVarEnum(), Type ) )
            {
                m_GenericPainType = Type;
            }
        }
        
        return TRUE;
    }

    if ( I.VarFloat   ( "DamageField\\TimeDelay",     m_TimeDelay ) )
        return TRUE;

    if ( SMP_UTIL_IsEnumVar<damage_field_spatial_types,damage_field_spatial_types>
        (I, "DamageField\\Type", 
        m_SpatialType, m_SpatialTypeList ) )
        return TRUE;

    if ( I.VarFloat   ( "DamageField\\Cube Width",             m_Dimensions[0] ) )
    {
        SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSLATION);
        UpdateTransform();              //    Being called in OnSpacialUpdate
        OnSpacialUpdate();
        return TRUE;
    }

    if ( I.VarFloat   ( "DamageField\\Cube Height",            m_Dimensions[1] ) )
    {
        SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSLATION);
        UpdateTransform();              //    Being called in OnSpacialUpdate
        OnSpacialUpdate();
        return TRUE;
    }
     
    if ( I.VarFloat   ( "DamageField\\Cube Length",            m_Dimensions[2] ) )
    {
        SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSLATION);
        UpdateTransform();              //    Being called in OnSpacialUpdate
        OnSpacialUpdate();
        return TRUE;
    }

    if ( I.VarFloat   ( "DamageField\\Sphere Radius",          m_Dimensions[0] ) )
    {
        SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSLATION);
        UpdateTransform();              //    Being called in OnSpacialUpdate
        OnSpacialUpdate();
        return TRUE;
    }

    if ( I.IsVar      ( "DamageField\\Affect Player" ) )
    {
        if (I.IsRead())
        {
            I.SetVarBool(m_SpatialTargets & DF_TARGET_PLAYER);
        }
        else
        {
            if (I.GetVarBool())
                m_SpatialTargets = m_SpatialTargets | DF_TARGET_PLAYER;
            else
                m_SpatialTargets = m_SpatialTargets & ~DF_TARGET_PLAYER;
        }
        return TRUE;
    }

    if ( I.IsVar      ( "DamageField\\Affect AI" ) )
    {
        if (I.IsRead())
        {
            I.SetVarBool(m_SpatialTargets & DF_TARGET_CHARACTERS);
        }
        else
        {
            if (I.GetVarBool())
                m_SpatialTargets = m_SpatialTargets | DF_TARGET_CHARACTERS;
            else
                m_SpatialTargets = m_SpatialTargets & ~DF_TARGET_CHARACTERS;
        }
        return TRUE;
    }

    if ( I.IsVar      ( "DamageField\\Affect Props" ) )
    {
        if (I.IsRead())
        {
            I.SetVarBool(m_SpatialTargets & DF_TARGET_PROPS);
        }
        else
        {
            if (I.GetVarBool())
                m_SpatialTargets = m_SpatialTargets | DF_TARGET_PROPS;
            else
                m_SpatialTargets = m_SpatialTargets & ~DF_TARGET_PROPS;
        }
        return TRUE;
    }

    if( I.IsVar("DamageField\\TriggerDamage") )
    {
        if( I.IsRead () )
        {
            I.SetVarBool(m_bDoingDamage);
        }
        else
        {
            m_bDoingDamage = I.GetVarBool();
            m_TimeSinceLastDamage = m_TimeDelay + 1.0f;
        }
        return TRUE;
    }

    return FALSE;
}

//=========================================================================

bbox damage_field::GetLocalBBox( void ) const 
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

void damage_field::OnRenderSpatial( void )
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

void damage_field::OnColCheck	( void )
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

void damage_field::OnColNotify( object& Object )
{
    if (!m_bActive )
        return;

    if (m_SpatialTargets & DF_TARGET_PLAYER)
    {
        if ( Object.IsKindOf( player::GetRTTI() ) == TRUE )
                m_bDoingDamage = TRUE;
    }

    if (m_SpatialTargets & DF_TARGET_CHARACTERS)
    {
        if ( Object.IsKindOf( character::GetRTTI() ) == TRUE )
                m_bDoingDamage = TRUE;
    }

    if (m_SpatialTargets & DF_TARGET_PROPS)
    {
        if ( Object.IsKindOf( prop_surface::GetRTTI() ) == TRUE )
                m_bDoingDamage = TRUE;
    }

    if (m_bDoingDamage)
    {
        m_TimeSinceLastDamage = m_TimeDelay + 1.0f;
    }

    object::OnColNotify(Object);
}

//=============================================================================

void damage_field::OnActivate ( xbool Flag )
{
    m_bActive = Flag;    
    
    //make it instantly fire
    m_TimeSinceLastDamage += m_TimeDelay + 1.0f;

    if ( Flag == TRUE )
        LogicCheckOnActivate();
}

//=============================================================================

void damage_field::LogicCheckOnActivate ( void )
{      
    if (!m_bActive)
        return;
    
    if (m_SpatialTargets & DF_TARGET_PLAYER)
    {
        if ( QueryPlayerInVolume(FALSE) )
                m_bDoingDamage = TRUE;
    }

    if (m_SpatialTargets & DF_TARGET_CHARACTERS)
    {
        if ( QueryNpcInVolume(FALSE) )
                m_bDoingDamage = TRUE;
    }

    if (m_SpatialTargets & DF_TARGET_PROPS)
    {
        if ( QueryPropsInVolume(FALSE) )
                m_bDoingDamage = TRUE;
    }

    if (m_bDoingDamage)
    {
        m_TimeSinceLastDamage = m_TimeDelay + 1.0f;
    }
}

//=============================================================================
// SPACIAL QUERIES
//=============================================================================

xbool damage_field::QueryPlayerInVolume ( xbool bDoDamage )
{
    //Query for the player...  
    slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_PLAYER );
     
    xbool bVal = FALSE;    
    while( SlotID != SLOT_NULL )
    {
        object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
        
        if( pObject != NULL )
            bVal |= QueryObjectInVolume(pObject, bDoDamage);
        
        SlotID = g_ObjMgr.GetNext( SlotID );
    }
    
    return bVal;
}

//=============================================================================

xbool damage_field::QueryNpcInVolume ( xbool bDoDamage )
{
    xbool bVal = FALSE;

    for( u32 objectTypeCount = 0; objectTypeCount < TYPE_END_OF_LIST; objectTypeCount++)
    {
        slot_id SlotID = g_ObjMgr.GetFirst((object::type)objectTypeCount);

        if( SlotID != SLOT_NULL )
        {
            object* tempObject = g_ObjMgr.GetObjectBySlot(SlotID);

            if(tempObject->GetAttrBits() & object::ATTR_CHARACTER_OBJECT )
            {
                while(SlotID != SLOT_NULL)
                {
                    object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
        
                    if( pObject != NULL )
                        bVal |= QueryObjectInVolume(pObject, bDoDamage);
    
                    SlotID = g_ObjMgr.GetNext( SlotID );
                }
            }
        }
    }

    return bVal;
}

//=============================================================================

xbool damage_field::QueryPropsInVolume ( xbool bDoDamage )
{
    //-- Query Prop Surfaces
    slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_PROP_SURFACE );

    xbool bVal = FALSE;
    while( SlotID != SLOT_NULL )
    {
        object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
        
        if( pObject != NULL )
            bVal |= QueryObjectInVolume(pObject, bDoDamage);
    
        SlotID = g_ObjMgr.GetNext( SlotID );
    }

    //-- Query Super Destructible Objects
    SlotID = g_ObjMgr.GetFirst( object::TYPE_SUPER_DESTRUCTIBLE_OBJ );

    while( SlotID != SLOT_NULL )
    {
        object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );

        if( pObject != NULL )
            bVal |= QueryObjectInVolume(pObject, bDoDamage);

        SlotID = g_ObjMgr.GetNext( SlotID );
    }
    
    return bVal;
}

//=============================================================================

xbool damage_field::QueryObjectInVolume( object* pObject, xbool bDoDamage )
{
    f32 Scalar = 1.0f;
    xbool bRval = FALSE;
    switch ( m_SpatialType )
    {
    case SPATIAL_TYPE_AXIS_CUBE:       
        {  
            bbox ObjectBBox = pObject->GetBBox();
            bbox DamageBBox = GetBBox();
            
            if( DamageBBox.Intersect(ObjectBBox) )
            {
                bRval = TRUE;
                vector3 Delta = ObjectBBox.GetCenter() - DamageBBox.GetCenter();
                f32 Dist = Delta.Length();
                Scalar = x_parametric( Dist, 0, MAX(m_Dimensions[0],MAX(m_Dimensions[1],m_Dimensions[2])), TRUE );
            }
        }
        break;
        
    case SPATIAL_TYPE_SPHERICAL:    
        { 
            bbox ObjectBBox = pObject->GetBBox();
            bbox DamageBBox = GetBBox();

            if( ObjectBBox.Intersect(GetPosition(), m_Dimensions[0]) )
            {
                bRval = TRUE;
                vector3 Delta = ObjectBBox.GetCenter() - DamageBBox.GetCenter();
                f32 Dist = Delta.Length();
                Scalar = x_parametric( Dist, 0, m_Dimensions[0], TRUE );
            }
        }
        break;
        
    default:
        ASSERT(0);
        break;
    }
    
    if (bRval && bDoDamage)
    {
        pain_handle PainHandle = GetPainHandleForGenericPain( m_GenericPainType );

        // Calculate custom scalar
        if( m_bDoAttenuate==FALSE )
            Scalar = 1.0f;

        //Do Damage
        pain Pain;
        Pain.Setup( PainHandle, GetGuid(), pObject->GetBBox().GetCenter() );
        Pain.SetDirectHitGuid( pObject->GetGuid() );
        Pain.ApplyToObject( pObject );
    }

    return bRval;
}

//=============================================================================

void damage_field::OnAdvanceLogic( f32 DeltaTime )
{
    CONTEXT( "damage_field::OnAdvanceLogic" );

    if (m_TimeSinceLastDamage < 0.0f)
        m_TimeSinceLastDamage = 0.0f;
    m_TimeSinceLastDamage += DeltaTime;

    object* pMover = m_DamageAnchorAffecter.GetObjectPtr();
    if (pMover)
    {
        //update position to match mover
        vector3 Pos = pMover->GetBBox().GetCenter();
        Pos.GetY() += m_YOffset;

        //must I move?
        if (Pos != GetPosition())
        {
            OnMove(Pos);
            m_bDoingDamage = TRUE;
        }
    } 

    if (!m_bActive)
        return;

    if (!m_bDoingDamage)
        return;

    if (m_TimeSinceLastDamage > m_TimeDelay)
    {
        m_TimeSinceLastDamage = 0.0f;
        m_DrawActivationIcon = TRUE;

        xbool bDamageDone = FALSE;
        if (m_SpatialTargets & DF_TARGET_PLAYER)
        {
            if ( QueryPlayerInVolume(TRUE) )
            {
                bDamageDone = TRUE;
            }
        }

        if (m_SpatialTargets & DF_TARGET_CHARACTERS)
        {
            if ( QueryNpcInVolume(TRUE) )
            {
                bDamageDone = TRUE;
            }
        }

        if (m_SpatialTargets & DF_TARGET_PROPS)
        {
            if ( QueryPropsInVolume(TRUE) )
            {
                bDamageDone = TRUE;
            }
        }

        m_bDoingDamage = bDamageDone;
    }
}
