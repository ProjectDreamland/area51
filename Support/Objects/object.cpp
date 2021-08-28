//==============================================================================
//
//  Object.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================
 

#include "Entropy.hpp"
#include "obj_mgr\Obj_Mgr.hpp"
#include "object.hpp"
#include "PainMgr\Pain.hpp"
#include "Event.hpp"
#include "Parsing\TextIn.hpp"
#include "e_Draw.hpp"

#ifdef X_EDITOR
#include "..\Apps\WorldEditor\WorldEditor.hpp"
#include "TriggerEx\Affecters\object_affecter.hpp"
#include "Dictionary\global_dictionary.hpp"
#include "Globals\Global_Variables_Manager.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "Templatemgr\TemplateMgr.hpp"
#endif

#include "Render\RenderInst.hpp"
#include "Gamelib\binLevel.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

//==============================================================================
//  OBJECT FUNCTIONS
//==============================================================================

object& object::operator = ( const object& Object )
{
    ASSERT( FALSE );
    return( (object&)Object );
}

//==============================================================================

object::object( void )
{
    // All initialization is done in OnInit
    m_Guid        = 0;
    m_ZoneInfo    = 0;
    m_AttrBits    = 0;
    m_FlagBits    = FLAG_DIRTY_TRANSFORM;
    m_WorldBBox.Clear();
    m_L2W.Identity();

#ifdef X_EDITOR
    m_bDrawBBox = false;
#endif // X_EDITOR

#ifndef TARGET_PC
    // Confirm that matrix is 16 byte aligned
    ASSERT( (((u32)(&m_L2W)) & 0xF) == 0 );
#endif

#if defined( USE_OBJECT_NAMES )
    m_Name[0] = 0;  //bjt
#endif

#if defined( USE_OBJECT_DEBUGINFO )
    m_DebugInfo.m_pDesc = NULL;
#endif

#ifdef X_EDITOR
    SetHidden( FALSE );
    SetSelectable( TRUE );
#endif
}

//==============================================================================

object::~object( void )
{
    //  if we are actually destructing the object then all that needs to be 
    //  done is deallocate any pointers that were allocated
//    if ( m_hCustomName.IsNull() == FALSE )
    {
        // TODO: Ask the string manager to nuke this string
        // m_hCustomName        
    }
}

//==============================================================================

inline void object::UpdateTransform( void )
{
    if( !(m_FlagBits & FLAG_DIRTY_TRANSFORM) )
        return;

    // Compute the new bbox
    m_WorldBBox = GetLocalBBox();
    m_WorldBBox.Transform( m_L2W );

    // Update the dirty bits
    m_FlagBits &= ~FLAG_DIRTY_TRANSFORM;
}

//==============================================================================

inline void object::OnSpacialUpdate( void )
{
    ASSERT( !(GetAttrBits() & ATTR_DESTROY) );
    if( IsLoading() == FALSE )
    {
        g_ObjMgr.RemoveFromSpatialDBase( m_SlotID );
        UpdateTransform();

        if( GetAttrBits() & ATTR_SPACIAL_ENTRY )
        {
            g_ObjMgr.AddToSpatialDBase     ( m_SlotID );
        }        
    }
}

//==============================================================================

void object::OnMove( const vector3& NewPos )
{       
    ASSERT( NewPos.IsValid() );

    ASSERT( x_abs(NewPos.GetX()) <= 1000000.0f );
    ASSERT( x_abs(NewPos.GetY()) <= 1000000.0f );
    ASSERT( x_abs(NewPos.GetZ()) <= 1000000.0f );

    m_FlagBits |= FLAG_DIRTY_TRANSLATION;
    m_L2W.SetTranslation( NewPos );
    UpdateTransform();              //    Being called in OnSpacialUpdate
    OnSpacialUpdate();
}

//==============================================================================

void object::OnMoveRel( const vector3& DeltaPos )
{
    OnMove( GetPosition() + DeltaPos );
}

//==============================================================================

void object::OnTransform( const matrix4& L2W )
{
    // Mark to recompute the W2L
    m_FlagBits |= FLAG_DIRTY_TRANSFORM;
    m_L2W = L2W;
    UpdateTransform();
    OnSpacialUpdate();
}

//===========================================================================

void object::OnTriggerTransform( const matrix4& L2W )
{
    OnTransform( L2W );
}

//==============================================================================

void object::OnInit( void ) 
{
    // Don't know what to do with this yet
}

//==============================================================================

void  object::OnActivate( xbool Flag )
{
    if( Flag ) 
    {
        ASSERTS( GetTypeDesc().GetAttrBits() & ATTR_NEEDS_LOGIC_TIME, "You can't set an object to have logic if the type desc doesn't have that bit" );
        m_AttrBits |= ATTR_NEEDS_LOGIC_TIME;
    }
    else
    {
        m_AttrBits &= ~ATTR_NEEDS_LOGIC_TIME;
    }
}
   
//==============================================================================
void object::OnKill( void )
{  
    // Don't know what to do with this yet
}

//==============================================================================

#ifndef X_RETAIL
void object::OnDebugRender( void )
{
    draw_Marker( m_L2W.GetTranslation(), XCOLOR_GREEN );
    draw_BBox  ( GetBBox() );
}
#endif

//==============================================================================

void object::OnEvent( const event& Event )
{
    (void)Event;
}

//==============================================================================

// Damage functions
void object::OnPain( const pain& Pain )
{
    (void)Pain ;
}

//==============================================================================

xbool object::OnChildPain( guid ChildGuid, const pain& Pain )
{
    (void)ChildGuid;
    (void)Pain;
    
    // Not processed
    return FALSE;
}

//==============================================================================

void object::OnLoad( text_in& TextIn )
{
    LoadStart();

    // Okay we can savefly start loading now
    prop_interface::OnLoad( TextIn );

    LoadEnd();
}
//==============================================================================

void object::OnPaste( const xarray<prop_container>& Container )
{
    LoadStart();

    // Okay we can savefly start loading now
    prop_interface::OnPaste( Container );

    LoadEnd();
}

//==============================================================================

object::type object::GetType( void ) const
{
    return GetTypeDesc().GetType();
}

//==============================================================================

#ifndef X_RETAIL
void object::OnColRender( xbool bRenderHigh ) 
{
    if( bRenderHigh )
    {
        draw_BBox( GetBBox(), XCOLOR_GREEN );
    }
    else
    {
        draw_BBox( GetBBox(), XCOLOR_BLUE );
    }
}   
#endif

//==============================================================================

void object::OnColCheck( void )
{
    if( g_CollisionMgr.IsUsingHighPoly() )
    {
        // This is because of the editor. In reality this should not be here
        g_CollisionMgr.StartApply( GetGuid() );
        g_CollisionMgr.ApplySphere( GetPosition(), 10 );
        g_CollisionMgr.EndApply();
    }
    else
    {
        g_CollisionMgr.StartApply( GetGuid() ) ;
        g_CollisionMgr.ApplyAABBox( GetBBox() ) ;
        g_CollisionMgr.EndApply() ;
    }
}

//==============================================================================

void object::OnRenderTransparent( void )
{
}

//==============================================================================

#if defined( USE_OBJECT_NAMES )

const char* object::GetName( void )
{
    return &m_Name[0];  
}

#endif

//==============================================================================

#if defined( USE_OBJECT_NAMES )

void object::SetName( const char *pNewObjectName)
{
    //if string is greater than max size truncate it
    if( x_strlen(pNewObjectName) >= MAX_OBJECT_NAME_LENGTH - 1 )
    {
        x_strncpy(&m_Name[0], pNewObjectName, MAX_OBJECT_NAME_LENGTH - 1);
        m_Name[MAX_OBJECT_NAME_LENGTH - 1] = 0;
    }
    else
        //else update object name
        x_strcpy(&m_Name[0], pNewObjectName);
}

#endif

//==============================================================================

void object::OnEnumProp( prop_enum& List )
{
    List.PropEnumString  ( "Base",                      "This is the base class for all the objects.", PROP_TYPE_HEADER );
    List.PropEnumString  ( "Base\\Name",                "Name of object", 0 );
    List.PropEnumBool    ( "Base\\Hidden",              "TRUE - Object is not rendered in the editor",PROP_TYPE_DONT_SAVE);
    List.PropEnumBool    ( "Base\\Selectable",          "TRUE - Object is selectable in the editor",PROP_TYPE_DONT_SAVE);
    List.PropEnumVector3 ( "Base\\Position",            "Position of the object in world space", 0 );
    List.PropEnumRotation( "Base\\Rotation",            "Rotation of the object in world space", 0 );
    List.PropEnumBBox    ( "Base\\WorldBBox",           "BBox of the object in world space", PROP_TYPE_READ_ONLY );
    List.PropEnumBBox    ( "Base\\LocalBBox",           "BBox of the object in local space", PROP_TYPE_READ_ONLY );
    List.PropEnumGuid    ( "Base\\GUID",                "Unique Identifier of the object", PROP_TYPE_READ_ONLY );    
    List.PropEnumBool    ( "Base\\Permeable",           "Object is permeable", PROP_TYPE_READ_ONLY );
    List.PropEnumBool    ( "Base\\DisableProjShadows",  "Object cannot receive projector shadows (i.e. artist-placed shadows)", 0 );
    List.PropEnumBool    ( "Base\\CastShadows",         "Object can cast dynamic shadows", 0 );
    List.PropEnumBool    ( "Base\\ReceiveShadows",      "Object can receive dynamic shadows", 0 );
    List.PropEnumInt     ( "Base\\ZoneInfo",            "HIDDEN - 8 bits for zone1, 8 bits for zone2", PROP_TYPE_DONT_SHOW); 
    List.PropEnumInt     ( "Base\\Attrs",               "HIDDEN - attr bits for copying", PROP_TYPE_DONT_SHOW | PROP_TYPE_DONT_SAVE | PROP_TYPE_DONT_EXPORT);
    List.PropEnumInt     ( "Base\\Flags",               "HIDDEN - flag bits for copying", PROP_TYPE_DONT_SHOW | PROP_TYPE_DONT_SAVE | PROP_TYPE_DONT_EXPORT);

#ifdef X_EDITOR
    List.PropEnumString  ( "Base\\Zone1",     "Name of zone currently in", PROP_TYPE_READ_ONLY | PROP_TYPE_DONT_SAVE);
    List.PropEnumString  ( "Base\\Zone2",     "Name of zone currently in", PROP_TYPE_READ_ONLY | PROP_TYPE_DONT_SAVE);
#endif

    List.PropEnumBool( "Base\\Collision",                            "These are the collision flags for all objects.", PROP_TYPE_HEADER );
    List.PropEnumBool( "Base\\Collision\\Collision On",              "Turn ALL collision On/Off for object" , PROP_TYPE_MUST_ENUM );
    if( (GetAttrBits() & ATTR_COLLIDABLE) == ATTR_COLLIDABLE )
    {    
        List.PropEnumBool( "Base\\Collision\\Block Player",              "Block Player" , 0);
        List.PropEnumBool( "Base\\Collision\\Block Character",           "Block Character" , 0);
        List.PropEnumBool( "Base\\Collision\\Block Ragdoll",             "Block Ragdoll" , 0);
        List.PropEnumBool( "Base\\Collision\\Block Small Projectile",    "Block Small Projectile" , 0);
        List.PropEnumBool( "Base\\Collision\\Block Large Projectile",    "Block Large Projectile" , 0);
        List.PropEnumBool( "Base\\Collision\\Block Character LOS",       "Block Character LOS" , 0);
        List.PropEnumBool( "Base\\Collision\\Block Player LOS",          "Block Player LOS" , 0);
        List.PropEnumBool( "Base\\Collision\\Block Pain LOS",            "Block Pain LOS", 0 );
        List.PropEnumBool( "Base\\Collision\\Block Small Debris",        "Block Small Debris" , 0);
    }

/*
    // If the object can be activated/deactivated then expouse this variable
    if( GetTypeDesc().GetAttrBits() & ATTR_NEEDS_LOGIC_TIME )
    {
        List.PropEnumBool( "Base\\IsActive", "Tells whether an object is active or not. If you turn this off after loading the object will be not active. You can activate or deactivate an object any time.", PROP_TYPE_EXPOSE );
    }
    */
}

//==============================================================================

xbool object::HandleAttribute( prop_query& I, const char* PropName, u32 AttrBit )
{
    if( I.IsVar( PropName ) )
    {
        if( I.IsRead() )
        {
            I.SetVarBool( (GetAttrBits() & AttrBit) ? (TRUE):(FALSE)  );
        }
        else
        {
            if( I.GetVarBool() )
                TurnAttrBitsOn( AttrBit );
            else
                TurnAttrBitsOff( AttrBit );
        }

        return TRUE;
    }

    return FALSE;
}

//==============================================================================

xbool object::OnProperty( prop_query& I )
{
#ifdef X_EDITOR
    s32 Zone1 = GetZone1();
    s32 Zone2 = GetZone2();
#endif // X_EDITOR

    //
    // All properties should begin with Base!
    //
    if( !I.IsBasePath("Base") )
        return FALSE;

    if( HandleAttribute( I, "Base\\DisableProjShadows", ATTR_DISABLE_PROJ_SHADOWS ) )
        return TRUE;

    if( HandleAttribute( I, "Base\\CastShadows", ATTR_CAST_SHADOWS ) )
        return TRUE;

    if( HandleAttribute( I, "Base\\ReceiveShadows", ATTR_RECEIVE_SHADOWS ) )
        return TRUE;

    if( I.IsBasePath("Base\\Collision") )
    {
        if( HandleAttribute( I, "Base\\Collision\\Collision On",           ATTR_COLLIDABLE             ) ) return TRUE;
        if( HandleAttribute( I, "Base\\Collision\\Block Player",           ATTR_BLOCKS_PLAYER           ) ) return TRUE;
        if( HandleAttribute( I, "Base\\Collision\\Block Character",        ATTR_BLOCKS_CHARACTER        ) ) return TRUE;
        if( HandleAttribute( I, "Base\\Collision\\Block Ragdoll",          ATTR_BLOCKS_RAGDOLL          ) ) return TRUE;
        if( HandleAttribute( I, "Base\\Collision\\Block Small Projectile", ATTR_BLOCKS_SMALL_PROJECTILES ) ) return TRUE;
        if( HandleAttribute( I, "Base\\Collision\\Block Large Projectile", ATTR_BLOCKS_LARGE_PROJECTILES ) ) return TRUE;
        if( HandleAttribute( I, "Base\\Collision\\Block Character LOS",    ATTR_BLOCKS_CHARACTER_LOS    ) ) return TRUE;
        if( HandleAttribute( I, "Base\\Collision\\Block Player LOS",       ATTR_BLOCKS_PLAYER_LOS       ) ) return TRUE;
        if( HandleAttribute( I, "Base\\Collision\\Block Pain LOS",         ATTR_BLOCKS_PAIN_LOS         ) ) return TRUE;
        if( HandleAttribute( I, "Base\\Collision\\Block Small Debris",     ATTR_BLOCKS_SMALL_DEBRIS     ) ) return TRUE;

        return FALSE;
    }


    if( I.IsVar( "Base" ) )
    {
        ASSERT( I.IsRead() );
        I.SetVarString( GetTypeDesc().GetTypeName(), 256 );
    }
    else if( I.IsVar( "Base\\Name" ) )
    {
       if( I.IsRead() )
       {
#ifdef USE_OBJECT_NAMES
            I.SetVarString(GetName(), MAX_OBJECT_NAME_LENGTH);
#else
            I.SetVarString(GetTypeDesc().GetTypeName(), MAX_OBJECT_NAME_LENGTH);
#endif
       }
       else
       {
#ifdef USE_OBJECT_NAMES
            SetName(I.GetVarString());
#endif // USE_OBJECT_NAMES
       }
    }
#ifdef X_EDITOR
    else if( I.IsVar( "Base\\Hidden" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarBool( IsHidden() );
        }
        else
        {
            SetHidden( I.GetVarBool() );
        }
    }
    else if( I.IsVar( "Base\\Selectable" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarBool( IsSelectable() );
        }
        else
        {
            SetSelectable( I.GetVarBool() );
        }
    }
#endif // X_EDITOR
    else if( I.IsVar( "Base\\Position" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarVector3( GetPosition() );
        }
        else
        {
            OnMove( I.GetVarVector3() );
        }
    }
    else if( I.IsVar( "Base\\Rotation" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarRotation( GetL2W().GetRotation() );
        }
        else
        {
            matrix4 L2W;
            L2W.Setup  ( vector3(1,1,1), I.GetVarRotation(), GetPosition() );
            OnTransform( L2W );
        }
    }
    else if( I.IsVar( "Base\\WorldBBox" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarBBox( GetBBox() );
        }
        else
        {
            ASSERT( 0 );
        }
    }
    else if( I.IsVar( "Base\\LocalBBox" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarBBox( GetLocalBBox() );
        }
        else
        {
            ASSERT( 0 );
        }
    }    
    else if( I.IsVar( "Base\\Permeable" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarBool( m_AttrBits & ATTR_COLLISION_PERMEABLE );
        }
        else
        {
            ASSERT( 0 );
        }
    }
    else if( I.VarGUID( "Base\\GUID", m_Guid ) ) 
    {
        // .. Nothing to do
    }
    else if( I.IsVar( "Base\\Attrs" ) ) 
    {
        if( I.IsRead() )
        {
            I.SetVarInt( (s32)GetAttrBits() );
        }
        else
        {
            SetAttrBits( (u32)I.GetVarInt() );
        }
    }
    else if( I.VarInt( "Base\\Flags", (s32&)m_FlagBits ) ) 
    {
        // .. Nothing to do
    }
    else if( I.IsVar( "Base\\ZoneInfo" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( m_ZoneInfo );
        }
        else
        {
            s32 Zone = I.GetVarInt();
            SetZone1( Zone & 0xff );
            SetZone2( (Zone>>8)&0xff );
        }
    }
#ifdef X_EDITOR
    else if( I.IsVar( "Base\\Zone1" ) )
    {
        if( I.IsRead() )
        {               
            I.SetVarString( xfs( "[%d] %s", (u8)GetZone1(), g_WorldEditor.GetZoneForId( (u8)GetZone1() ) ), 256 );
        }
    }
    else if( I.IsVar( "Base\\Zone2" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarString( xfs( "[%d] %s", (u8)GetZone2(), g_WorldEditor.GetZoneForId( (u8)GetZone2() ) ), 256 );
        }
    }
#endif
    /*
    else if( I.IsVar( "Base\\IsActive" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarBool( IsActive() );
        }
        else
        {
            OnActivate( I.GetVarBool() );
        }
    }
    */
    else
    {
        // could not find the property here
        return FALSE;
    }

    return TRUE;
}

//=============================================================================
// Property validation utility functions
//=============================================================================

#ifdef X_EDITOR
s32 object::OnValidateGeomAnim( xstring&            Desc, 
                                render_inst*        pRenderInst,
                                anim_group::handle* pAnimGroupHandle,
                                xstring&            ErrorMsg )
{
    // Lookup geometry info if present
    geom*   pGeom    = NULL;
    xstring GeomName = "";
    if( pRenderInst )
    {
        pGeom    = pRenderInst->GetGeom();
        GeomName = pRenderInst->GetGeomName();
    }

    // Lookup animation info if present
    anim_group* pAnimGroup = NULL;
    xstring     AnimName  = "";
    if( pAnimGroupHandle )
    {
        pAnimGroup = pAnimGroupHandle->GetPointer();
        AnimName   = pAnimGroupHandle->GetName();
    }

    // If geometry is present, make sure it's been assigned
    if( ( pRenderInst ) && ( pGeom == NULL ) )
    {
        if( GeomName.GetLength() == 0 )
            ErrorMsg += Desc + "No geometry assigned.\n";
        else
            ErrorMsg += Desc + "Geometry [" + GeomName + "] has no data. Make sure resources are built.\n";
        return 1;
    }

    // If animation is present, make sure it's been assigned
    if( ( pAnimGroupHandle ) && ( pAnimGroup == NULL ) )
    {
        if( AnimName.GetLength() == 0 )
            ErrorMsg += Desc + "No animation assigned.\n";
        else
            ErrorMsg += Desc + "Animation [" + AnimName + "] has no data. Make sure resources are built.\n";
        return 1;
    }

    // If geometry and animation is present, make sure the anim has at least as many bones as the geom
    if( ( pGeom ) && ( pAnimGroup ) && ( pAnimGroup->GetNBones() < pGeom->m_nBones ) )
    {
        xstring GeomBones;
        xstring AnimBones;
        GeomBones.Format( "%d", pGeom->m_nBones );
        AnimBones.Format( "%d", pAnimGroup->GetNBones() );

        ErrorMsg += Desc + "Geometry and animation have incompatible hierarchy!\n"
                    "Geometry [" + GeomName + "] has " + GeomBones + " bone(s).\n" +
                    "Animation [" + AnimName + "] has " + AnimBones + " bone(s).\n" +
                    "Animation must have at least as many bones as the geometry.\n";
        return 1;
    }

    // If only geometry is present, make sure there is no hierarchy
    if( ( pRenderInst ) && ( pGeom ) && ( !pAnimGroupHandle ) && ( pGeom->m_nBones != 1) )
    {
        xstring GeomBones;
        GeomBones.Format( "%d", pGeom->m_nBones );

        ErrorMsg += Desc + "Geometry [" + GeomName + "] has " + GeomBones + " bones. There should only be 1.\n";
        return 1;
    }

    // No errors
    return 0;
}

//=============================================================================

s32 object::OnValidateObject( xstring& Desc, guid* pGuid, xstring& ErrorMsg )
{
    // Not present?
    if( !pGuid )
        return 0;

    // Get guid value
    guid Guid = *pGuid;

    // Is guid defined?
    if( Guid == 0 )
    {
        ErrorMsg += Desc + "No guid specified.\n";
        return 1;
    }

    // Does object exist?
    if( g_ObjMgr.GetObjectByGuid( Guid ) == NULL )
    {
        ErrorMsg += Desc + "Object for guid [" + guid_ToString( Guid ) + "] does not exist.\n";
        return 1;
    }

    // No errors
    return 0;
}

//=============================================================================

s32 object::OnValidateObject( xstring& Desc, object_affecter* pAffecter, xstring& ErrorMsg )
{
    // Not present?
    if( !pAffecter )
        return 0;

    // Get affecter
    object_affecter& Affecter = *pAffecter;
    guid             Guid     = Affecter.GetGuid();

    // Which type?
    switch( Affecter.GetObjectCode() )
    {
        case object_affecter::OBJECT_CODE_CHECK_BY_STATIC_GUID:
            return OnValidateObject( Desc, &Guid, ErrorMsg );

        case object_affecter::OBJECT_CODE_CHECK_BY_GLOBAL_GUID:
        {
            // Guid var string specified?
            s32 iGuidVarName = Affecter.GetGuidVarName();
            if( iGuidVarName == -1 )
            {
                ErrorMsg += Desc + "Guid variable is not specified\n";
                return 1;
            }

            // Does guid var exist?
            xstring GuidVarName = g_StringMgr.GetString( iGuidVarName );
            xhandle GuidVarHandle;
            if( !g_VarMgr.GetGuidHandle( GuidVarName, &GuidVarHandle ) )
            {
                ErrorMsg += Desc + "Guid variable [" + GuidVarName + "] does not exist.\n";
                return 1;
            }
        }
        break;
    }

    // No errors
    return 0;
}

//=============================================================================

s32 object::OnValidateAnim( xstring& Desc, s32* pAnimGroupName, s32 iAnimName, xstring& ErrorMsg )
{
    // Not present?
    if( !pAnimGroupName )
        return 0;

    // Lookup anim group name string index
    s32 iAnimGroupName = *pAnimGroupName;

    // Anim group and anim name not specified?
    if( ( iAnimGroupName == -1 ) && ( iAnimName == -1 ) )
    {
        ErrorMsg += Desc + "Animation package and animation is not specified.\n";
        return 1;
    }

    // Anim group not specified?
    if( iAnimGroupName == -1 )
    {
        ErrorMsg += Desc + "Animation package is not specified.\n";
        return 1;
    }

    // Anim name not specified?
    if( iAnimName == -1 )
    {
        ErrorMsg += Desc + "Animation is not specified.\n";
        return 1;
    }

    // Lookup animation group and anim string
    xstring AnimGroupName = g_StringMgr.GetString( iAnimGroupName );
    xstring AnimName      = g_StringMgr.GetString( iAnimName );

    // Lookup animation group
    anim_group::handle hAnimGroup;
    hAnimGroup.SetName( AnimGroupName );
    anim_group* pAnimGroup = hAnimGroup.GetPointer();

    // Anim group not found?
    if( pAnimGroup == NULL )
    {
        ErrorMsg += Desc + "Animation package [" + AnimGroupName + "] does not exist.\n";
        return 1;
    }

    // Animation not found?
    if( pAnimGroup->GetAnimIndex( AnimName ) == -1 )
    {
        ErrorMsg += Desc + "Animation [" + AnimName + "] does not exist in the package [" + AnimGroupName + "].\n";
        return 1;
    }

    // No errors
    return 0;
}

//=============================================================================

s32 object::OnValidateSound( xstring& Desc, rhandle<char>* phSoundPackage, s32 iSoundName, xstring& ErrorMsg )
{
    // Not present?
    if( !phSoundPackage )
        return 0;

    // Sound package and sound name not specified?
    if( ( phSoundPackage->GetName() == NULL ) && ( iSoundName == -1 ) )
    {
        ErrorMsg += Desc + "Audio package and sound is not specified.\n";
        return 1;
    }

    // Sound package not specified?
    if( phSoundPackage->GetName() == NULL )
    {
        ErrorMsg += Desc + "Audio package is not specified.\n";
        return 1;
    }

    // Sound name not specified?
    if( iSoundName == -1 )
    {
        ErrorMsg += Desc + "Sound is not specified.\n";
        return 1;
    }

    // Lookup sound
    xstring Sound = g_StringMgr.GetString( iSoundName );

    // Is sound valid?
    if( g_AudioMgr.IsValidDescriptor( Sound ) == FALSE )
    {
        ErrorMsg += Desc + "Sound [" + Sound + "] does not exist.\n";
        return 1;
    }

    // No errors
    return 0;
}

//=============================================================================

s32 object::OnValidateGlobal( xstring& Desc, s32* pGlobalName, xstring& ErrorMsg )
{
    // Not present?
    if( !pGlobalName )
        return 0;

    // Lookup global name
    s32 iGlobalName = *pGlobalName;

    // Is global specified?
    if( iGlobalName == -1 )
    {
        ErrorMsg += Desc + "Global variable is not specified.\n";
        return 1;
    }

    // Does global exist?
    xstring GlobalName = g_StringMgr.GetString( iGlobalName );
    xhandle GlobalHandle;
    if(         ( g_VarMgr.GetVarHandle  ( GlobalName, &GlobalHandle ) == FALSE )
            &&  ( g_VarMgr.GetGuidHandle ( GlobalName, &GlobalHandle ) == FALSE )
            &&  ( g_VarMgr.GetTimerHandle( GlobalName, &GlobalHandle ) == FALSE ) )
    {
        ErrorMsg += Desc + "Global variable [" + GlobalName + "] does not exist.\n";
        return 1;
    }

    // No errors
    return 0;
}

//=============================================================================

s32 object::OnValidateProperty( xstring& Desc, object_affecter* pAffecter, s32* pPropertyName, s32 PropertyType, xstring& ErrorMsg )
{
    // Not present?
    if( ( !pAffecter ) || ( !pPropertyName ) )
        return 0;

    // Object present?
    if ( OnValidateObject( Desc, pAffecter, ErrorMsg ) )
        return 1;

    // Lookup object (will only be available for static guid)
    object* pObject = pAffecter->GetObjectPtr();
    if( ( pObject == NULL ) && ( pAffecter->GetObjectCode() == object_affecter::OBJECT_CODE_CHECK_BY_STATIC_GUID ) )
    {
        ErrorMsg += Desc + "Object does not exist\n";
        return 1;
    }

    // Property present?
    s32 iPropName = *pPropertyName;
    if( iPropName == -1 )
    {
        ErrorMsg += Desc + "Property not specified.\n";
        return 1;
    }

    // Setup property query
    f32 F;
    s32 I;
    xbool B;
    guid G;
    char S[256] = { 0 } ;
    prop_query Query;
    xstring    PropName = g_StringMgr.GetString( iPropName );
    switch( PropertyType & PROP_TYPE_BASIC_MASK )
    {
        default:
            ErrorMsg += Desc + "Invalid property type.\n";
            return 1;

        case PROP_TYPE_ANGLE: 
            Query.RQueryAngle( PropName, F );
            break;

        case PROP_TYPE_FLOAT: 
            Query.RQueryFloat( PropName, F );
            break;

        case PROP_TYPE_INT: 
            Query.RQueryInt( PropName, I );
            break;

        case PROP_TYPE_BOOL: 
            Query.RQueryBool( PropName, B );
            break;

        case PROP_TYPE_GUID: 
            Query.RQueryGUID( PropName, G );
            break;

        case PROP_TYPE_ENUM: 
            Query.RQueryEnum( PropName, S );
            break;

        case PROP_TYPE_BUTTON:
            Query.RQueryButton( PropName, S );
            break;
    }   

    // Does property exist?
    if ( ( pObject ) && ( pObject->OnProperty( Query ) == FALSE ) )
    {
        ErrorMsg += Desc + "Property [" + PropName + "] does not exist.\n";
        return 1;
    }

    // No errors
    return 0;
}

//=============================================================================

xbool object:: OnValidateTemplate( xstring& Desc, s32* pTemplateName, xstring& ErrorMsg )
{
    // Not present?
    if( !pTemplateName )
        return 0;

    // Lookup template
    s32 iTemplate = *pTemplateName;

    // Specified?
    if( iTemplate == -1 )
    {
        ErrorMsg += Desc + "Template is not specified.\n";
        return 1;
    }

    // Does it exist?
    xstring Template = g_TemplateStringMgr.GetString( iTemplate );
    X_FILE* pFile = x_fopen( (const char*)Template, "rb" );
    if( pFile == NULL )
    {
        ErrorMsg += Desc + "Template [" + Template + "] does not exist.\n";
        return 1;
    }
    x_fclose( pFile );

    // No errors
    return 0;
}

//=============================================================================

s32 object::OnValidateProperties( xstring& ErrorMsg )
{
    s32 nErrors = 0;

    // Check default geometry and animation properties
    nErrors += OnValidateGeomAnim( xstring( "Error:\n" ), GetRenderInstPtr(), GetAnimGroupHandlePtr(), ErrorMsg );

    // Check bounds?
    switch( GetType() )
    {
    // Skip these
    case TYPE_GOD:
    case TYPE_PATH:
    case TYPE_SND_EMITTER:
        break;
    default:
        // If object is in spatial database check local bbox to make sure it's not huge
        // NOTE: Everything in the editor has this bit set which is annoying - it really shouldn't
        //       Also, since there are some huge pieces of geometry in the game, let's skip them.
        if( GetAttrBits() & object::ATTR_SPACIAL_ENTRY )
        {
            // Default limits
            f32 MaxW = 50000.0f;
            f32 MaxH = 50000.0f;
            f32 MaxL = 50000.0f;
       
            // Get bbox size
            bbox BBox = GetLocalBBox();
            f32 W = BBox.Max.GetX() - BBox.Min.GetX();
            f32 H = BBox.Max.GetY() - BBox.Min.GetY();
            f32 L = BBox.Max.GetZ() - BBox.Min.GetZ();

            // Is this geometry?
            if( GetGeomPtr() )
            {
                // Use big limits for geometry since the pieces can be big
                MaxW = 1000000.0f;
                MaxH = 1000000.0f;
                MaxL = 1000000.0f;
            
                // Count # of degenerate side
                s32 nDegen = 0;
                if( W <= 0 )
                    nDegen++;            
                if( H <= 0 )
                    nDegen++;            
                if( L <= 0 )
                    nDegen++;            
                    
                // There should be no more than 1 degenerate side
                if( nDegen > 1 )
                {
                    xstring Size;
                    Size.Format( "Width:%f Height:%f Length:%f", W, H, L );

                    nErrors++;
                    ErrorMsg += xstring("ERROR: Geometry has more than one degenerate dimension! Check your max file.\n") + Size + "\n";
                }
            }
            else
            {                
                // Negative or zero size?
                if( ( W <= 0 ) || ( H <= 0 ) || ( L <= 0 ) )
                {
                    xstring Size;
                    Size.Format( "Width:%f Height:%f Length:%f", W, H, L );

                    nErrors++;
                    ErrorMsg += xstring("ERROR: Invalid bounding box!\n") + Size + "\n";
                }
            }
            
            // Too big?
            if( ( W > MaxW ) || ( H > MaxH ) || ( L > MaxL ) )
            {
                xstring Size;
                Size.Format( "Width:%f Height:%f Length:%f", W, H, L );

                nErrors++;
                ErrorMsg += xstring("ERROR: Bounding box of object is huge!\n") + Size + "\n";
            }
        }
        break;
    }

    return nErrors;
}
#endif

//=============================================================================

bbox object::GetScreenBBox( const view& rView )
{
    bbox BBox   = GetColBBox();

    vector3& Max = BBox.Max;
    vector3& Min = BBox.Min;

    // screen positions of the top and bottom of the world bbox
    vector3 Top[4];
    vector3 Bot[4];

    Top[0] = rView.PointToScreen( Max );
    Top[1] = rView.PointToScreen( vector3( Max.GetX(), Max.GetY(), Min.GetZ() ) );
    Top[2] = rView.PointToScreen( vector3( Min.GetX(), Max.GetY(), Max.GetZ() ) );
    Top[3] = rView.PointToScreen( vector3( Min.GetX(), Max.GetY(), Min.GetZ() ) );

    Bot[0] = rView.PointToScreen( Min );
    Bot[1] = rView.PointToScreen( vector3( Max.GetX(), Min.GetY(), Min.GetZ() ) );
    Bot[2] = rView.PointToScreen( vector3( Min.GetX(), Min.GetY(), Max.GetZ() ) );
    Bot[3] = rView.PointToScreen( vector3( Max.GetX(), Min.GetY(), Max.GetZ() ) );

    f32 MinX = Top[0].GetX();
    f32 MinY = Top[0].GetY();
    f32 MaxX = Top[0].GetX();
    f32 MaxY = Top[0].GetY();

    MinX = MIN( MinX, Top[1].GetX() );
    MinX = MIN( MinX, Top[2].GetX() );
    MinX = MIN( MinX, Top[3].GetX() );
    MinX = MIN( MinX, Bot[0].GetX() );
    MinX = MIN( MinX, Bot[1].GetX() );
    MinX = MIN( MinX, Bot[2].GetX() );
    MinX = MIN( MinX, Bot[3].GetX() );

    MinY = MIN( MinY, Top[1].GetY() );
    MinY = MIN( MinY, Top[2].GetY() );
    MinY = MIN( MinY, Top[3].GetY() );
    MinY = MIN( MinY, Bot[0].GetY() );
    MinY = MIN( MinY, Bot[1].GetY() );
    MinY = MIN( MinY, Bot[2].GetY() );
    MinY = MIN( MinY, Bot[3].GetY() );


    MaxX = MAX( MaxX, Top[1].GetX() );
    MaxX = MAX( MaxX, Top[2].GetX() );
    MaxX = MAX( MaxX, Top[3].GetX() );
    MaxX = MAX( MaxX, Bot[0].GetX() );
    MaxX = MAX( MaxX, Bot[1].GetX() );
    MaxX = MAX( MaxX, Bot[2].GetX() );
    MaxX = MAX( MaxX, Bot[3].GetX() );

    MaxY = MAX( MaxY, Top[1].GetY() );
    MaxY = MAX( MaxY, Top[2].GetY() );
    MaxY = MAX( MaxY, Top[3].GetY() );
    MaxY = MAX( MaxY, Bot[0].GetY() );
    MaxY = MAX( MaxY, Bot[1].GetY() );
    MaxY = MAX( MaxY, Bot[2].GetY() );
    MaxY = MAX( MaxY, Bot[3].GetY() );

    return bbox( vector3( MinX, MinY, 0.0f ),
                 vector3( MaxX, MaxY, 0.0f ) );
}

//=============================================================================

void object::EnumAttachPoints( xstring& String ) const
{
    String = "BaseObject~"; 
    String[10] = 0;
}

//=============================================================================

s32 object::GetAttachPointIDByName( const char* pName ) const
{
    if (x_stricmp(pName,"BaseObject")==0)
        return 0;
    
    return -1;
}

//=============================================================================

xstring object::GetAttachPointNameByID( s32 iAttachPt ) const
{
    if (iAttachPt == 0)
        return xstring("BaseObject");

    return xstring("INVALID\0");
}

//=============================================================================

void object::OnAttachedMove(       s32      iAttachPt,
                             const matrix4& L2W )
{
    if (iAttachPt != 0)
        return;

    OnTransform( L2W );
}

//=============================================================================

xbool object::GetAttachPointData( s32      iAttachPt,
                                  matrix4& L2W,
                                  u32      Flags )
{
    (void)Flags;

    if (iAttachPt == 0)
    {
        L2W = GetL2W();
        return TRUE;
    }
    return FALSE;
}

//=============================================================================

geom* object::GetGeomPtr( void )
{
    render_inst* pRenderInst = GetRenderInstPtr();
    if( pRenderInst )
        return pRenderInst->GetGeom();
    else    
        return NULL;
}

//=============================================================================

const char* object::GetGeomName( void )
{
    render_inst* pRenderInst = GetRenderInstPtr();
    if( pRenderInst )
        return pRenderInst->GetGeomName();
    else    
        return "NULL";
}

//=============================================================================

anim_group* object::GetAnimGroupPtr( void )
{
    anim_group::handle* pAnimGroupHandle = GetAnimGroupHandlePtr();
    if( pAnimGroupHandle )
        return pAnimGroupHandle->GetPointer();
    else
        return NULL;
}

//=============================================================================

const char* object::GetAnimGroupName( void )
{
    anim_group::handle* pAnimGroupHandle = GetAnimGroupHandlePtr();
    if( pAnimGroupHandle )
        return pAnimGroupHandle->GetName();
    else
        return "NULL";
}

//=============================================================================

void object::SetNewZoneInfo( u16 ZoneInfo )
{
    // remove any current zone link information
    g_ObjMgr.RemoveSlotFromZone( m_SlotID );
    if( GetAttrBits() & object::ATTR_RENDERABLE )
        g_ObjMgr.RemoveSlotFromRenderable( m_SlotID );

    // set in the new zone and add the zone links back in
    m_ZoneInfo = ZoneInfo;
    g_ObjMgr.AddSlotToZone( m_SlotID );
    if( GetAttrBits() & object::ATTR_RENDERABLE )
        g_ObjMgr.AddSlotToRenderable( m_SlotID );
}

//=============================================================================

void object::UpdateRenderableLinks( void )
{
    g_ObjMgr.RemoveSlotFromRenderable( m_SlotID );
    if( GetAttrBits() & object::ATTR_RENDERABLE )
        g_ObjMgr.AddSlotToRenderable( m_SlotID );
}

//=============================================================================

const char* object::GetLogicalName( void )
{
    return this->GetTypeDesc().GetTypeName();
}

//=============================================================================

void object::OnAddedToGroup( guid gGroup )
{
    (void)gGroup;
}

