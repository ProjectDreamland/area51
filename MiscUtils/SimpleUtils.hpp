
#ifndef SIMPLE_UTILS_HPP
#define SIMPLE_UTILS_HPP

#include <x_types.hpp>
#include <x_stdio.hpp>

#include "Objects\Object.hpp"
#include "..\auxiliary\miscutils\property.hpp"
#include "..\auxiliary\miscutils\PropertyEnum.hpp"
#include "..\support\Characters\AlertPackage.hpp"
#include "..\Auxiliary\fx_RunTime\Fx_Mgr.hpp"

//Misc utils useful for the A51 project. There are project dependicies so this isn't a cross project
//util class..

//=========================================================================
//FORWARD DECLLERATIONS 

union   matrix4;
class   view;
class   god;
class   player;

struct  rigid_geom;
struct  rtti;
struct  guid;
#ifdef TARGET_PS2
struct  vector3;
#else
union   vector3;
#endif
struct  xcolor;

//==---------------------------------------------------------------------------
//  NOTES:
//
//  SMP_UTIL_GetInfoAllPlayers  : returns # of players found
//
//==---------------------------------------------------------------------------


void        SMP_UTIL_StrSafeCpy          ( char* Dest, const char* Src, s32 MaxLen );
xbool       SMP_UTIL_FileExist           ( const char* File );
vector3     SMP_UTIL_RandomVector        ( f32 Extent );
vector3     SMP_UTIL_RandomVector        ( f32 Extent, const vector3& Orientation );
void        SMP_UTIL_draw_MatrixAxis     ( const matrix4& rM );
void        SMP_UTIL_draw_Polygon        ( const vector3* pPoint, s32 NPoints, xcolor Color, xbool DoWire = TRUE );
void        SMP_UTIL_draw_Cube           ( const vector3& Halves, const matrix4& Matrix,  xcolor Color );
void        SMP_UTIL_PointObjectAt       ( object*  pObject, const vector3& rDir,const vector3& rPos, const f32 Roll );
xbool       SMP_UTIL_IsGuidOfType        ( object** ppObject, const guid& rGuid, const rtti& rRunTimeInfo );
f32         SMP_UTIL_GetVolume           ( const bbox& BBox );
void        SMP_UTIL_Broadcast_Alert     ( alert_package& Package ) ;
factions    SMP_UTIL_GetFactionForGuid   ( guid Guid );
xbool       SMP_UTIL_IsPlayerFaction     ( guid Guid );
bbox        SMP_UTIL_GetUntransformedBBox( const matrix4& M, const bbox& TransBBox );
god*        SMP_UTIL_Get_God             ( void ) ;
player*     SMP_UTIL_GetActivePlayer     ( void ) ;
guid        SMP_UTIL_GetActivePlayerGuid ( void ) ;
//xbool       SMP_UTIL_GetFloorProperties  ( guid CharGuid, xcolor& FloorColor, u32& FloorMat );
u16         SMP_UTIL_GetZoneForPosition  ( const vector3& vPosition, f32 DistToCheck = -200.f );
s32         SMP_UTIL_GetInfoAllPlayers   ( s32 MaxPlayersToGet, matrix4* pL2Ws, player** ppPlayers );
xbool       SMP_UTIL_InitFXFromString    ( const char* pFXOName, fx_handle& FXHandle );

//=========================================================================
//
//  Functions to simplify getting external resources loaded.
//  This sort of mimics the setup we had in The Hobbit resource system.
//  
//
//=========================================================================
enum smp_resource_type
{
    SMP_FXO,
    SMP_AUDIOPKG,
    SMP_XBMP,
    SMP_ANIM,
    SMP_SKINGEOM,
    SMP_RIGIDGEOM,
};

void        SMP_UTIL_EnumHiddenManualResource ( prop_enum&          PropEnum,
                                                const char*         pPropertyName,
                                                smp_resource_type   Type);
xbool       SMP_UTIL_IsHiddenManualResource   ( prop_query&         PropQuery,
                                                const char*         pPropertyName,
                                                const char*         pResourceName );

//=========================================================================

#ifdef TARGET_PC
void draw_Arc       ( const vector3& C, f32 R, radian Dir, radian FOV, xcolor Color = XCOLOR_WHITE, f32 PercentDraw = 0.005f ) ;
void draw_3DCircle  ( const vector3& C, f32 R, xcolor Color = XCOLOR_WHITE, const vector3& Up=vector3(0,1,0), f32 PercentDraw = 0.005f );
void draw_Cylinder  ( const vector3& Center, f32 Radius, f32 Height, s32 nSteps, xcolor Color, xbool bCapped = TRUE, const vector3& Up=vector3(0,1,0));

#endif
//=========================================================================
//enum_type is the enumeration type you want to pass in, it is the same type the table uses
//data_type is the data type of the Enum which may or may not be the same as the enum_Type
//=========================================================================

template <class data_type, class enum_type > 
xbool   SMP_UTIL_IsEnumVar           ( prop_query& rPropQuery, const char* pLabel, data_type& rEnumData, enum_table<enum_type>& rEnumTable )
{
    if ( rPropQuery.IsVar( pLabel ) )
    { 
        if( rPropQuery.IsRead() )
        {
            if (rEnumTable.DoesValueExist( (enum_type) rEnumData ))
            {
                rPropQuery.SetVarEnum( rEnumTable.GetString( ( (enum_type) rEnumData ) ) );
            }
            else
            {
                rPropQuery.SetVarEnum( "INVALID" );
            }
        }
        else
        {
            enum_type TmpVal;
            
            if( rEnumTable.GetValue( rPropQuery.GetVarEnum(), TmpVal ) )
            {
                rEnumData = (data_type) TmpVal;
            }
        }
        
        return TRUE ;
    } 
    
    return FALSE;
}

//=========================================================================
// templated string_type class must be one of the simple_string variants, or
// support the Get, Set and MaxLen functions...
//=========================================================================

template < class string_type > 
xbool   SMP_UTIL_IsExternalVar( prop_query& rPropQuery, const char* pLabel, string_type& rString, xbool& IsRead )
{
    IsRead = FALSE;

    if( rPropQuery.IsVar( pLabel ) )
    {
        IsRead =  rPropQuery.IsRead() ;

        if( IsRead )
        {
            rPropQuery.SetVarExternal( rString.Get(),  rString.MaxLen() );
        }
        else
        {
            if( rPropQuery.GetVarExternal()[0] )
            {
                rString.Set( rPropQuery.GetVarExternal() );
            }
        }

        return( TRUE );
    }

    return FALSE;
}

//=========================================================================

xbool SMP_UTIL_IsButtonVar      ( prop_query& rPropQuery, const char* pLabel, const char* pButtonTag, xbool& IsRead );
xbool SMP_UTIL_IsParticleFxVar  ( prop_query& I, const char* pName, rhandle<char>& hParticleFx );
xbool SMP_UTIL_IsBoneVar        ( prop_query& I, const char* pName, object& Object, s32& BoneName, s32& BoneIndex );
xbool SMP_UTIL_IsAudioVar       ( prop_query& I, const char* pName, rhandle<char>& hAudioPackage, s32& SoundID );

xbool SMP_UTIL_IsAnimVar        ( prop_query&               I, 
                                  const char*               pAnimGroupPropName, 
                                  const char*               pAnimPropName, 
                                        anim_group::handle& hAnimGroup, 
                                        s32&                AnimGroupName, 
                                        s32&                AnimName );

xbool SMP_UTIL_IsTemplateVar( prop_query& I, const char* pName, s32& TemplateID );

#ifdef USE_OBJECT_NAMES
xbool SMP_UTIL_IsObjectNameVar( prop_query& I, const char* pName, guid ObjectGuid );
#endif


//=========================================================================
// simple_string : Templated simple string class with some common funtionality
//=========================================================================

template <s32 MAX_STRING_SIZE> 
    class simple_string
{
public:

    simple_string   ( const guid& rGuid ){  Set(guid_ToString(rGuid)); }
    simple_string   ( void ){m_Buffer[0] = '\0';}
    simple_string   ( const char* pString ) {SMP_UTIL_StrSafeCpy( m_Buffer, pString, MAX_STRING_SIZE );}

           void         Set     ( const char* pString ) { SMP_UTIL_StrSafeCpy( m_Buffer, pString, MAX_STRING_SIZE );}
    inline char*        Get     ( void )        { return m_Buffer;}
    inline const char*  Get     ( void ) const  { return m_Buffer;}

    inline xbool        IsSame  ( const char* pString ) { ASSERT(pString); return x_strcmp( m_Buffer, pString ) == 0; }
    inline xbool        IsEmpty ( void ) { return m_Buffer[0] == '\0'; }
    inline s32          MaxLen  ( void ) { return MAX_STRING_SIZE; }
    
private:

    char m_Buffer[MAX_STRING_SIZE];
};

typedef  simple_string<255> big_string;
typedef  simple_string<64>  med_string;
typedef  simple_string<32>  sml_string;
   
//=========================================================================
// select_slot_iterator : Helpful iterator object using for walking the slot_id lists returned from a 
// select call of the object manager..
//=========================================================================

struct select_slot_iterator
{
                select_slot_iterator   ( void );
               ~select_slot_iterator   ( void );


    object*     Get     ( void );

    void        Begin   ( void );
    void        End     ( void );
    
    void        Next    ( void );
    xbool       AtEnd   ( void );
    xbool       IsEmpty ( void );

protected:
    
    u16     m_SlotID;
    
};

//=========================================================================
// OBJECT_PTR ::templated wrapper class for automatic converison from GUID->ptr with type check and 
//  null checks..
//=========================================================================

template <class object_type>
    struct object_ptr
{
    object_ptr( object* pObject )
    {
        m_pObject = NULL;
        
        if (pObject != NULL)
        {
            if ( pObject->IsKindOf( object_type::GetRTTI() ) == TRUE )
            {
                m_pObject = (object_type*) pObject;
            }
        }
    }

    object_ptr( const guid& rGuid ) 
    { 
        m_pObject = NULL;
        
        if ( rGuid != NULL_GUID )
        {
            object* pObject = g_ObjMgr.GetObjectByGuid( rGuid );
            
            if (pObject != NULL)
            {
                if ( pObject->IsKindOf( object_type::GetRTTI() ) == TRUE )
                {
                    m_pObject = (object_type*) pObject;
                }
            }
        }
    }  
    
    inline xbool IsValid ( void ) { return ( m_pObject != NULL ); }
    
    inline object_type& operator *  ()  { return *m_pObject ; }
    inline object_type* operator -> ()  { return m_pObject ;  }
    inline xbool        operator == ( const object* pObject ) { return (m_pObject == pObject) ; }
    inline              operator xbool ( void ) { return (m_pObject != NULL) ; }

    object_type*  m_pObject;
};

//=========================================================================
//Explicit specilization of object_ptr<object> to avoid the RTTI check..
/*
struct object_ptr<object>
{
    object_ptr( const guid& rGuid ) ;
    
    inline xbool IsValid ( void ) { return ( m_pObject != NULL ); }

    object_type*  m_pObject;
};
*/

//=========================================================================
/*
template <class T>
    class ptr_list
{
public:
        
    enum
    {
        INVALID_KEY = 0
    };
      
    struct node
    {
        T           m_Val;
        node*       m_pNext;
    };
    
public:
    
    ptr_list ( void )
    {
        m_ValidationKey = INVALID_KEY+1;
        m_NodePool.SetCount(1);
    }
    
    ptr_list ( s32 Preallocate )
    { 
        m_ValidationKey = INVALID_KEY+1;
        m_NodePool.SetCount(Preallocate);
    }
    
    ~ptr_list ( void )
    {
        m_NodePool.Clear();
    }

    node           GetUsedHead  ( void )
    {
        return m_pUsed;
    }
    
private:

    void            Reset       ( void )
    {
        m_pFree     = m_NodePool[0];
        m_pUsed     = NULL;
        m_UsedNodes = 0;
    }   

    u32                 m_ValidationKey;

    node*               m_pFree;
    node*               m_pUsed;

    u32                 m_UsedNodes;

    xarray<node>        m_NodePool;
};

//=========================================================================

template <class T>
    class ptr_list_iterator
{
public:

    ptr_list_iterator ( ptr_list<T>& rList ) : m_rList( rList )
    { }
      
    T           Get     ( void )
    {
        ASSERT( m_pNode );

        if (m_pNode==NULL)
        {
            x_throw( "Invalid Iterator.\n" );
        }

        return m_pNode->m_Val;
    }

    void        Begin   ( void )
    { 
        m_pNode         = m_rList.GetUsedHead();
        m_ValidationKey = m_rList.GetValidationKey();
    }

    void        End     ( void )
    {
        m_pNode         = NULL;
        m_ValidationKey = ptr_list<T>::INVALID_KEY;
    }
    
    void        Next    ( void )
    {
        ASSERT( m_pNode );

        if (m_rList.ValidateKey(m_ValidationKey))
        {
            x_throw( "List has become invalidated.\n" );
        }
        
        m_pNode = m_pNode->m_pNext;
    }

    xbool       AtEnd   ( void )
    { 
        if (m_pNode == NULL)
            return TRUE;

        return FALSE;
    }

    xbool       IsEmpty ( void )
    {
        if (m_pNode == NULL)
            return TRUE;
    }

private:
    
    ptr_list<T>::node*          m_pNode;
    ptr_list<T>&                m_rList;
    u32                         m_ValidationKey;

};*/

#endif
