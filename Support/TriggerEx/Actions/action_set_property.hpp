///////////////////////////////////////////////////////////////////////////////
//
//  action_set_property.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_set_property_
#define _action_set_property_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"
#include "..\Affecters\object_affecter.hpp"

//=========================================================================
// action_set_property
//=========================================================================

class action_set_property : public actions_ex_base
{
public:
                    action_set_property                     ( guid ParentGuid );

    virtual         action_ex_types     GetType             ( void )    { return GetTypeStatic();}
    static          action_ex_types     GetTypeStatic       ( void )    { return TYPE_ACTION_AFFECT_PROPERTY;}
    virtual         const char*         GetTypeInfo         ( void )    { return "Change an exposed property on an object."; } 
    virtual         const char*         GetDescription      ( void );
                    s32                 GetCode             ( void )    { return m_Code; }
                    const char*         GetPropertyName     ( void );
                    s32                 GetPropertyType     ( void )    { return m_PropertyType; }
                    guid                GetPropertyGuid     ( void )    { return m_GuidVar.GetGuid(); }
                    guid                GetObjectAffecterGuid( void )   { return m_ObjectAffecter.GetGuid(); }   
                    
    virtual         xbool               Execute         ( f32 DeltaTime );    
    virtual			void	            OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	            OnProperty	    ( prop_query& rPropQuery );

#ifdef X_EDITOR
    virtual         object_affecter*    GetObjectRef0   ( xstring& Desc );
    virtual         s32*                GetPropertyRef  ( xstring& Desc, s32& PropertyType );
#endif

#ifndef X_RETAIL
    virtual         void                OnDebugRender   ( s32 Index );
#endif // X_RETAIL
    
protected:
    
    void            GetExternalPropTag  ( char* pTag );
    void            GetEnumForProperty  ( xstring& strEnum );
    void            SetPropertyType     ( void );
    xbool           RequiresObjectType  ( void );

    s32             BuildObjectList     ( object**& ppObjectArray );
    
public:    
    enum property_mod_codes
    { 
        INVALID_PMOD_CODES = -1,
        PMOD_CODE_ADD,
        PMOD_CODE_SUBTRACT,
        PMOD_CODE_SET,
        PMOD_CODES_END
    };
    
protected:    
    object_affecter m_ObjectAffecter;    
    
    union
    {
        xbool   m_Bool ;
        f32     m_Float ;
        s32     m_Integer ;
        // SH:Guid removed in favour of m_GuidVar
        //u64     m_Guid ;    // YUK - Guid has a constructor so I can't just use it!
    } m_VarRaw ;

    object_affecter m_GuidVar;
    s32             m_RawString;
    s32             m_Code;               //Action code
    s32             m_PropertyName;
    s32             m_PropertyType;
    s32             m_ObjectType;
};

#endif
