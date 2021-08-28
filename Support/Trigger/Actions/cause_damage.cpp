///////////////////////////////////////////////////////////////////////////
//
//  cause_damage.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\cause_damage.hpp"

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"
#include "render\LightMgr.hpp"

#include "Entropy.hpp"

//=========================================================================
// ENUMS
//=========================================================================

typedef enum_pair<pain::type> pain_type_enum_pair;

static pain_type_enum_pair s_PainTypeList[] = 
{
    pain_type_enum_pair("GENERIC",                  pain::TYPE_GENERIC),
    pain_type_enum_pair("EXPLOSION",                pain::TYPE_PROJECTILE_GRENADE),
    pain_type_enum_pair("SHOCK",                    pain::TYPE_ELECTROCUTION),
    pain_type_enum_pair( k_EnumEndStringConst,      pain::PAIN_UNDEFINED) //**MUST BE LAST**//
};

enum_table<pain::type>  cause_damage::m_PainTypeList( s_PainTypeList );              

//=========================================================================
// cause_damage
//=========================================================================

cause_damage::cause_damage ( guid ParentGuid ) : actions_base( ParentGuid )
{
    m_MaxDamage = 100.0f;
    m_MinDamage = 100.0f;
    m_Height    = 100.0f;
    m_Length    = 100.0f;
    m_Width     = 100.0f;
    m_MaxForce  =   0.0f;
    m_MinForce  =   0.0f;

    m_bCreateDecal  =   FALSE;
    m_DecalSize     =   200.0f;

    m_bCreateLight  =   FALSE;
    m_LightColor    =   xcolor(152, 152, 76, 255);
    m_LightRadius   =   300.0f;
    m_LightIntensity=   1.0f;
    m_LightFadeTime =   1.5f;
    
    m_PainType      =   pain::TYPE_GENERIC;
}

//=========================================================================

void cause_damage::Execute ( trigger_object* pParent )
{
    (void)pParent;

    TRIGGER_CONTEXT( "ACTION * cause_damage::Execute" );
    
    if (m_bCreateLight)
    {
        g_LightMgr.CreateLight( GetPositionOwner(), m_LightColor, 
            m_LightRadius, m_LightIntensity, m_LightFadeTime );
    }
    
    //collect all damagable objects in range (MAX 32)
    const s32 kMAX_CONTACTS = 128;
    slot_id idList[kMAX_CONTACTS];
    s32 contactCount = 1;

    g_ObjMgr.SelectBBox( object::ATTR_DAMAGEABLE , GetDamageBBox() ,object::TYPE_ALL_TYPES );    
    slot_id aID = g_ObjMgr.StartLoop();
    while( aID != SLOT_NULL )
    {
        if ( contactCount >= kMAX_CONTACTS )
            break ;

        idList[contactCount]= aID;
        ++contactCount;

        aID = g_ObjMgr.GetNextResult( aID );
    }
    g_ObjMgr.EndLoop();

    //now damage the collected objects
    while( --contactCount)
    {
        object* pObject = g_ObjMgr.GetObjectBySlot(idList[contactCount] );
        if (pObject)
        {
            pain PainEvent;
            PainEvent.Type      = m_PainType;
            PainEvent.Center    = GetPositionOwner();
            PainEvent.Origin    = m_ParentGuid ;
            PainEvent.PtOfImpact= GetDamageBBox().GetCenter();
            PainEvent.Direction = vector3(0.0f, 0.0f, 0.0f);
            PainEvent.DamageR0  = m_MaxDamage; 
            PainEvent.DamageR1  = m_MinDamage; 
            PainEvent.ForceR0   = m_MaxForce;
            PainEvent.ForceR1   = m_MinForce;
            PainEvent.RadiusR0  = 0;
            PainEvent.RadiusR1  = MAX( m_Height, MAX(m_Width, m_Length));
    
            pObject->OnPain( PainEvent ); 
        }
    }
}

//=========================================================================

void cause_damage::OnRender ( void )
{
    draw_BBox( GetDamageBBox(), XCOLOR_RED );
    draw_Label( GetPositionOwner(), XCOLOR_WHITE, GetTypeName() );
}

//=========================================================================

void cause_damage::OnEnumProp ( prop_enum& List )
{   
    actions_base::OnEnumProp( List );

   List.AddFloat   ( "Width",       "The width of the damage area." );
   List.AddFloat   ( "Height",      "The height of the damage area." );
   List.AddFloat   ( "Length",      "The length of the damage area." );

   List.AddEnum    ( "PainType",    m_PainTypeList.BuildString(), "What type of pain this trigger causes.", PROP_TYPE_MUST_ENUM );

   List.AddFloat   ( "MaxDamage",   "Maximum amount of damage to apply (at center)." );
   List.AddFloat   ( "MinDamage",   "Minimum amount of damage to apply (at edge)." );

   List.AddFloat   ( "MaxForce",    "Maximum amount of force to apply (at center)." );
   List.AddFloat   ( "MinForce",    "Minimum amount of force to apply (at edge)." );

   List.AddBool    ( "CreateDecal", "Create a scorch decal at this location. (note: the trigger must be on the ground for this decal to appear correctly.)", PROP_TYPE_MUST_ENUM);

   if (m_bCreateDecal)
   {
       List.AddFloat("DecalSize",   "Size of the decal to show");
   }
       
   List.AddBool    ( "CreateLight", "Create a dynamic light at this location.", PROP_TYPE_MUST_ENUM);

   if (m_bCreateLight)
   {
       List.AddColor("LightColor",      "Color of dynamic light");
       List.AddFloat("LightRadius",     "Size of dynamic light");
       List.AddFloat("LightIntensity",  "Intensity of dynamic light");
       List.AddFloat("LightFade",       "How long before the light has faded");
   }
}

//=========================================================================

xbool cause_damage::OnProperty ( prop_query& I )
{
    if( actions_base::OnProperty( I ) )
        return TRUE;

    if ( I.IsVar( "PainType") )
    {
        if( I.IsRead() )
        {
            if ( cause_damage::m_PainTypeList.DoesValueExist( m_PainType ) )
            {
                I.SetVarEnum( cause_damage::m_PainTypeList.GetString( m_PainType ) );
            }
            else
            {
                I.SetVarEnum( "INVALID" );
            } 
        }
        else
        {
            pain::type PainType;

            if( cause_damage::m_PainTypeList.GetValue( I.GetVarEnum(), PainType ) )
            {
                m_PainType = PainType;
            }
        }
        
        return( TRUE );
    }

    if ( I.VarFloat   ( "Width",         m_Width ) )
        return TRUE;

    if ( I.VarFloat   ( "Height",        m_Height ) )
        return TRUE;
     
    if ( I.VarFloat   ( "Length",        m_Length ) )
        return TRUE;

    if ( I.VarFloat   ( "MaxDamage",     m_MaxDamage ) )
        return TRUE;

    if ( I.VarFloat   ( "MinDamage",     m_MinDamage ) )
        return TRUE;
    
    if ( I.VarFloat   ( "MaxForce",      m_MaxForce ) )
        return TRUE;

    if ( I.VarFloat   ( "MinForce",      m_MinForce ) )
        return TRUE;

    if ( I.VarBool    ( "CreateDecal",   m_bCreateDecal ) )
        return TRUE;

    if ( I.VarFloat   ( "DecalSize",     m_DecalSize ) )
        return TRUE;

    if ( I.VarBool    ( "CreateLight",   m_bCreateLight ) )
        return TRUE;

    if ( I.VarColor   ( "LightColor",    m_LightColor ) )
        return TRUE;

    if ( I.VarFloat   ( "LightRadius",   m_LightRadius ) )
        return TRUE;

    if ( I.VarFloat   ( "LightIntensity",m_LightIntensity ) )
        return TRUE;

    if ( I.VarFloat   ( "LightFade",     m_LightFadeTime ) )
        return TRUE;

    return FALSE;
}

//=========================================================================

bbox cause_damage::GetDamageBBox( void )
{
    bbox DamageArea;
    vector3 Min(-m_Width/2,    0,           -m_Length/2 );
    vector3 Max(m_Width/2,     m_Height,    m_Length/2 );

    DamageArea = bbox( Min + GetPositionOwner(), Max + GetPositionOwner() );

    return DamageArea;
}

